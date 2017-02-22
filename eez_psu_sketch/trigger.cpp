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

namespace eez {
namespace psu {
namespace trigger {

static struct {
    float u;
    float i;
} g_levels[CH_NUM];

static float g_delay;
static Source g_source;
static Polarity g_polarity;

enum State {
    STATE_IDLE,
    STATE_INITIATED,
    STATE_TRIGGERED,
    STATE_EXECUTING
};
static State g_state;
static bool g_continuousInitializationEnabled;
static uint32_t g_triggeredTime;
static uint8_t g_extTrigLastState;

static const int VOLTAGE_TRIGGER_IN_PROGRESS = 1;
static const int CURRENT_TRIGGER_IN_PROGRESS = 2;
uint8_t g_triggerInProgress[CH_NUM];

void reset() {
    g_delay = DELAY_DEFAULT;
    g_source = SOURCE_IMMEDIATE;
    g_polarity = POLARITY_POSITIVE;

    for (int i = 0; i < CH_NUM; ++i) {
        g_levels[i].u = 0;
        g_levels[i].i = 0;
    }

    g_state = STATE_IDLE;
    g_continuousInitializationEnabled = false;
}

void extTrigInterruptHandler() {
    uint8_t state = digitalRead(EXT_TRIG);
    if (state == 1 && g_extTrigLastState == 0 && g_polarity == POLARITY_POSITIVE || state == 0 && g_extTrigLastState == 1 && g_polarity == POLARITY_NEGATIVE) {
        generateTrigger(SOURCE_PIN1, false);
    }
    g_extTrigLastState = state;
}

void init() {
    reset();

    noInterrupts();
    g_extTrigLastState = digitalRead(EXT_TRIG);
    attachInterrupt(digitalPinToInterrupt(EXT_TRIG), extTrigInterruptHandler, CHANGE);
    interrupts();
}

void setDelay(float delay) {
    g_delay = delay;
}

float getDelay() {
    return g_delay;
}

void setSource(Source source) {
    g_source = source;
}

Source getSource() {
    return g_source;
}

void setPolarity(Polarity polarity) {
    g_polarity = polarity;
}

Polarity getPolarity() {
    return g_polarity;
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
    if (currentTime - g_triggeredTime > g_delay * 1000L) {
        startImmediately();
    }
}

int generateTrigger(Source source, bool checkImmediatelly) {
    if (g_source != source) {
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
    if (g_continuousInitializationEnabled) {
        g_state = STATE_INITIATED;
    } else {
        g_state = STATE_IDLE;
    }

    profile::enableSave(true);
}

void setVoltageTriggerFinished(Channel &channel) {
    if (channel_dispatcher::isCoupled() || channel_dispatcher::isTracked()) {
        for (int i = 0; i < CH_NUM; ++i) {
            g_triggerInProgress[i] &= ~VOLTAGE_TRIGGER_IN_PROGRESS;
        }
    } else {
        g_triggerInProgress[channel.index - 1] &= ~VOLTAGE_TRIGGER_IN_PROGRESS;
    }

    if (isTriggerFinished()) {
        triggerFinished();
    }
}

void setCurrentTriggerFinished(Channel &channel) {
    if (channel_dispatcher::isCoupled() || channel_dispatcher::isTracked()) {
        for (int i = 0; i < CH_NUM; ++i) {
            g_triggerInProgress[i] &= ~CURRENT_TRIGGER_IN_PROGRESS;
        }
    } else {
        g_triggerInProgress[channel.index - 1] &= ~CURRENT_TRIGGER_IN_PROGRESS;
    }

    if (isTriggerFinished()) {
        triggerFinished();
    }
}

int startImmediately() {
    g_state = STATE_EXECUTING;

    profile::enableSave(false);

    for (int i = 0; i < CH_NUM; ++i) {
        g_triggerInProgress[i] = VOLTAGE_TRIGGER_IN_PROGRESS | CURRENT_TRIGGER_IN_PROGRESS;
    }

    for (int i = 0; i < CH_NUM; ++i) {
        Channel& channel = Channel::get(i);

        if (i == 0 || !(channel_dispatcher::isCoupled() || channel_dispatcher::isTracked())) {
            if (channel.getVoltageTriggerMode() == TRIGGER_MODE_FIXED) {
            } else{
	            if (channel.isRemoteProgrammingEnabled()) {
                    return SCPI_ERROR_EXECUTION_ERROR;
	            }

                if (channel.getVoltageTriggerMode() == TRIGGER_MODE_LIST) {
                    if (!list::areVoltageAndDwellListSizesEquivalent(channel)) {
                        return SCPI_ERROR_LIST_LENGTHS_NOT_EQUIVALENT;
                    }
                } else {
	                if (g_levels[i].u > channel_dispatcher::getULimit(channel)) {
                        return SCPI_ERROR_VOLTAGE_LIMIT_EXCEEDED;
	                }

	                if (g_levels[i].u * channel_dispatcher::getISet(channel) > channel_dispatcher::getPowerLimit(channel)) {
                        return SCPI_ERROR_POWER_LIMIT_EXCEEDED;
                    }
                }
            }

            if (channel.getCurrentTriggerMode() == TRIGGER_MODE_FIXED) {
            } else if (channel.getCurrentTriggerMode() == TRIGGER_MODE_LIST) {
                if (!list::areCurrentAndDwellListSizesEquivalent(channel)) {
                    return SCPI_ERROR_LIST_LENGTHS_NOT_EQUIVALENT;
                }
            } else {
                if (g_levels[i].i > channel_dispatcher::getILimit(channel)) {
                    return SCPI_ERROR_CURRENT_LIMIT_EXCEEDED;
	            }

                if (g_levels[i].i * channel_dispatcher::getUSet(channel) > channel_dispatcher::getPowerLimit(channel)) {
                    return SCPI_ERROR_POWER_LIMIT_EXCEEDED;
                }
            }

            if (channel.getVoltageTriggerMode() == TRIGGER_MODE_LIST && channel.getCurrentTriggerMode() == TRIGGER_MODE_LIST) {
                if (!list::areVoltageAndCurrentListSizesEquivalent(channel)) {
                    return SCPI_ERROR_LIST_LENGTHS_NOT_EQUIVALENT;
                }
            }
        }
    }

    for (int i = 0; i < CH_NUM; ++i) {
        Channel& channel = Channel::get(i);

        if (i == 0 || !(channel_dispatcher::isCoupled() || channel_dispatcher::isTracked())) {
            list::executionReset(channel);

            if (channel.getVoltageTriggerMode() == TRIGGER_MODE_FIXED) {
                setVoltageTriggerFinished(channel);
            } else if (channel.getVoltageTriggerMode() == TRIGGER_MODE_LIST) {
                list::executionSetVoltage(channel);
            } else {
                channel_dispatcher::setVoltage(channel, g_levels[i].u);
                setVoltageTriggerFinished(channel);
            }

            if (channel.getCurrentTriggerMode() == TRIGGER_MODE_FIXED) {
                setCurrentTriggerFinished(channel);
            } else if (channel.getCurrentTriggerMode() == TRIGGER_MODE_LIST) {
                list::executionSetCurrent(channel);
            } else {
                channel_dispatcher::setCurrent(channel, g_levels[i].i);
                setCurrentTriggerFinished(channel);
            }
        
            list::executionStart(channel);
        }
    }

    return SCPI_RES_OK;
}

int initiate() {
    if (g_source == SOURCE_IMMEDIATE) {
        return startImmediately();
    } else {
        g_state = STATE_INITIATED;
    }
    return SCPI_RES_OK;
}

int enableInitiateContinuous(bool enable) {
    g_continuousInitializationEnabled = enable;
    return initiate();
}

bool isContinuousInitializationEnabled() {
    return g_continuousInitializationEnabled;
}

bool isExecuting() {
    return g_state == STATE_EXECUTING;
}

void abort() {
    list::abort();
    g_state = STATE_IDLE;
    profile::enableSave(true);
}

void tick(uint32_t tick_usec) {
    if (g_state == STATE_TRIGGERED) {
        check(tick_usec / 1000);
    }
}

}
}
} // namespace eez::psu::trigger