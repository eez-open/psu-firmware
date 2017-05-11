/*
 * EEZ PSU Firmware
 * Copyright (C) 2017-present, Envox d.o.o.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#include "psu.h"
#include "trigger.h"
#include "channel_dispatcher.h"
#include "list.h"
#include "profile.h"
#include "persist_conf.h"
#include "io_pins.h"

namespace eez {
namespace psu {
namespace trigger {

static struct {
    float u;
    float i;
} g_levels[CH_MAX];

enum State {
    STATE_IDLE,
    STATE_INITIATED,
    STATE_TRIGGERED,
    STATE_EXECUTING
};
static State g_state;
static uint32_t g_triggeredTime;
static uint8_t g_extTrigLastState;

bool g_triggerInProgress[CH_MAX];

void reset() {
    persist_conf::devConf2.triggerDelay = DELAY_DEFAULT;
    persist_conf::devConf2.triggerSource = SOURCE_IMMEDIATE;
    persist_conf::devConf2.flags.triggerContinuousInitializationEnabled = false;

    persist_conf::saveDevice2();

    g_state = STATE_IDLE;
}

void extTrigInterruptHandler() {
    uint8_t state = digitalRead(EXT_TRIG);
    if (state == 1 && g_extTrigLastState == 0 && persist_conf::devConf2.ioPins[0].polarity == io_pins::POLARITY_POSITIVE ||
        state == 0 && g_extTrigLastState == 1 && persist_conf::devConf2.ioPins[0].polarity == io_pins::POLARITY_NEGATIVE) {
        generateTrigger(SOURCE_PIN1, false);
    }
    g_extTrigLastState = state;
}

void init() {
    g_state = STATE_IDLE;

    noInterrupts();
    g_extTrigLastState = digitalRead(EXT_TRIG);
    attachInterrupt(digitalPinToInterrupt(EXT_TRIG), extTrigInterruptHandler, CHANGE);
    interrupts();

    if (isContinuousInitializationEnabled()) {
        initiate();
    }
}

void setDelay(float delay) {
    persist_conf::devConf2.triggerDelay = delay;
}

float getDelay() {
    return persist_conf::devConf2.triggerDelay;
}

void setSource(Source source) {
    persist_conf::devConf2.triggerSource = source;
}

Source getSource() {
    return (Source)persist_conf::devConf2.triggerSource;
}

void setVoltage(Channel &channel, float value) {
    g_levels[channel.index - 1].u = value;
}

float getVoltage(Channel &channel) {
    return g_levels[channel.index - 1].u;
}

void setCurrent(Channel &channel, float value) {
    g_levels[channel.index - 1].i = value;
}

float getCurrent(Channel &channel) {
    return g_levels[channel.index - 1].i;
}

void check(uint32_t currentTime) {
    if (currentTime - g_triggeredTime > persist_conf::devConf2.triggerDelay * 1000L) {
        startImmediately();
    }
}

int generateTrigger(Source source, bool checkImmediatelly) {
    if (persist_conf::devConf2.triggerSource != source) {
        return SCPI_ERROR_TRIGGER_IGNORED;
    }
    
    if (g_state != STATE_INITIATED) {
        return SCPI_ERROR_TRIGGER_IGNORED;
    }

    g_state = STATE_TRIGGERED;

    g_triggeredTime = micros() / 1000;

    if (checkImmediatelly) {
        check(g_triggeredTime);
    }

    return SCPI_RES_OK;
}

bool isTriggerFinished() {
    for (int i = 0; i < CH_NUM; ++i) {
        if (g_triggerInProgress[i]) {
            return false;
        }
    }
    return true;
}

void triggerFinished() {
    if (persist_conf::devConf2.flags.triggerContinuousInitializationEnabled) {
        g_state = STATE_INITIATED;
    } else {
        g_state = STATE_IDLE;
    }
}

void onTriggerFinished(Channel &channel) {
    if (channel.getVoltageTriggerMode() == TRIGGER_MODE_LIST) {
        int err;

        switch (channel.getTriggerOnListStop()) {
        case TRIGGER_ON_LIST_STOP_OUTPUT_OFF:
            channel_dispatcher::setVoltage(channel, 0);
            channel_dispatcher::setCurrent(channel, 0);
            channel_dispatcher::outputEnable(channel, false);
            break;
        case TRIGGER_ON_LIST_STOP_SET_TO_FIRST_STEP:
            if (!list::setListValue(channel, 0, &err)) {
                generateError(err);
            }
            break;
        case TRIGGER_ON_LIST_STOP_SET_TO_LAST_STEP:
            if (!list::setListValue(channel, list::maxListsSize(channel) - 1, &err)) {
                generateError(err);
            }
            break;
        case TRIGGER_ON_LIST_STOP_STANDBY:
            channel_dispatcher::setVoltage(channel, 0);
            channel_dispatcher::setCurrent(channel, 0);
            channel_dispatcher::outputEnable(channel, false);
            schedulePowerDown();
            break;
        }
    }
}

void setTriggerFinished(Channel &channel) {
    if (channel_dispatcher::isCoupled() || channel_dispatcher::isTracked()) {
        for (int i = 0; i < CH_NUM; ++i) {
            g_triggerInProgress[i] = false;
        }
    } else {
        g_triggerInProgress[channel.index - 1] = false;
    }

    onTriggerFinished(channel);

    if (isTriggerFinished()) {
        triggerFinished();
    }
}

int checkTrigger() {
    bool onlyFixed = true;

    for (int i = 0; i < CH_NUM; ++i) {
        Channel& channel = Channel::get(i);

        if (i == 0 || !(channel_dispatcher::isCoupled() || channel_dispatcher::isTracked())) {
            if (channel.getVoltageTriggerMode() != channel.getCurrentTriggerMode()) {
                return SCPI_ERROR_INCOMPATIBLE_TRANSIENT_MODES;
            }

            if (channel.getVoltageTriggerMode() != TRIGGER_MODE_FIXED) {
	            if (channel.isRemoteProgrammingEnabled()) {
                    return SCPI_ERROR_EXECUTION_ERROR;
	            }

                if (channel.getVoltageTriggerMode() == TRIGGER_MODE_LIST) {
                    if (list::isListEmpty(channel)) {
                        return SCPI_ERROR_LIST_IS_EMPTY;
                    }

                    if (!list::areListLengthsEquivalent(channel)) {
                        return SCPI_ERROR_LIST_LENGTHS_NOT_EQUIVALENT;
                    }

                    int err = list::checkLimits();
                    if (err) {
                        return err;
                    }
                } else {
	                if (util::greater(g_levels[i].u, channel_dispatcher::getULimit(channel), getPrecision(VALUE_TYPE_FLOAT_VOLT))) {
                        return SCPI_ERROR_VOLTAGE_LIMIT_EXCEEDED;
	                }

                    if (util::greater(g_levels[i].i, channel_dispatcher::getILimit(channel), getPrecision(VALUE_TYPE_FLOAT_AMPER))) {
                        return SCPI_ERROR_CURRENT_LIMIT_EXCEEDED;
	                }

	                if (util::greater(g_levels[i].u * g_levels[i].i, channel_dispatcher::getPowerLimit(channel), getPrecision(VALUE_TYPE_FLOAT_WATT))) {
                        return SCPI_ERROR_POWER_LIMIT_EXCEEDED;
                    }
                }

                onlyFixed = false;
            }
        }
    }

    if (onlyFixed) {
        return SCPI_ERROR_CANNOT_INITIATE_WHILE_IN_FIXED_MODE;
    }

    return 0;
}

int startImmediately() {
    int err = checkTrigger();
    if (err) {
        return err;
    }

    g_state = STATE_EXECUTING;
    for (int i = 0; i < CH_NUM; ++i) {
        g_triggerInProgress[i] = true;
    }

    io_pins::onTrigger();

    for (int i = 0; i < CH_NUM; ++i) {
        Channel& channel = Channel::get(i);

        if (i == 0 || !(channel_dispatcher::isCoupled() || channel_dispatcher::isTracked())) {
            if (channel.getVoltageTriggerMode() == TRIGGER_MODE_LIST) {
                channel_dispatcher::setVoltage(channel, 0);
                channel_dispatcher::setCurrent(channel, 0);

                channel_dispatcher::outputEnable(channel, channel_dispatcher::getTriggerOutputState(channel));

                list::executionStart(channel);
            } else {
                if (channel.getVoltageTriggerMode() == TRIGGER_MODE_STEP) {
                    channel_dispatcher::setVoltage(channel, g_levels[i].u);
                    channel_dispatcher::setCurrent(channel, g_levels[i].i);

                    channel_dispatcher::outputEnable(channel, channel_dispatcher::getTriggerOutputState(channel));
                }

                setTriggerFinished(channel);
            }
        }
    }

    return SCPI_RES_OK;
}

int initiate() {
    if (persist_conf::devConf2.triggerSource == SOURCE_IMMEDIATE) {
        return startImmediately();
    } else {
        int err = checkTrigger();
        if (err) {
            return err;
        }
        g_state = STATE_INITIATED;
    }
    return SCPI_RES_OK;
}

int enableInitiateContinuous(bool enable) {
    persist_conf::devConf2.flags.triggerContinuousInitializationEnabled = enable;
    if (enable) {
        return initiate();
    } else {
        return SCPI_RES_OK;
    }
}

bool isContinuousInitializationEnabled() {
    return persist_conf::devConf2.flags.triggerContinuousInitializationEnabled;
}

bool isIdle() {
    return g_state == STATE_IDLE;
}

bool isInitiated() {
    return g_state == STATE_INITIATED;
}

void abort() {
    list::abort();
    g_state = STATE_IDLE;
}

void tick(uint32_t tick_usec) {
    if (g_state == STATE_TRIGGERED) {
        check(tick_usec / 1000);
    }
}

}
}
} // namespace eez::psu::trigger