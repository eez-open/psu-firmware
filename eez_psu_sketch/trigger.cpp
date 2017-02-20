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
    STATE_TRIGGERED
};
static State g_state;
static bool g_continuousInitializationEnabled;
static uint32_t g_triggeredTime;
static uint8_t g_extTrigLastState;

void reset() {
    g_delay = DELAY_DEFAULT;
    g_source = SOURCE_IMMEDIATE;
    g_polarity = POLARITY_POSITIVE;

    for (int i = 0; i < CH_NUM; ++i) {
        g_levels[i].u = NAN;
        g_levels[i].i = NAN;
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

void setVoltage(int iChannel, float value) {
    g_levels[iChannel].u = value;
}

float getVoltage(int iChannel) {
    return g_levels[iChannel].u;
}

void setCurrent(int iChannel, float value) {
    g_levels[iChannel].i = value;
}

float getCurrent(int iChannel) {
    return g_levels[iChannel].i;
}

void check(uint32_t currentTime) {
    if (currentTime - g_triggeredTime > g_delay * 1000L) {
        startImmediately();
        if (g_continuousInitializationEnabled) {
            g_state = STATE_INITIATED;
        } else {
            g_state = STATE_IDLE;
        }
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

int startImmediately() {
    for (int i = 0; i < CH_NUM; ++i) {
        Channel& channel = Channel::get(i);

        if (i == 0 || !(channel_dispatcher::isCoupled() || channel_dispatcher::isTracked())) {
            if (!util::isNaN(g_levels[i].u)) {
	            if (channel.isRemoteProgrammingEnabled()) {
                    return SCPI_ERROR_EXECUTION_ERROR;
	            }

	            if (g_levels[i].u > channel_dispatcher::getULimit(channel)) {
                    return SCPI_ERROR_VOLTAGE_LIMIT_EXCEEDED;
	            }

	            if (g_levels[i].u * channel_dispatcher::getISet(channel) > channel_dispatcher::getPowerLimit(channel)) {
                    return SCPI_ERROR_POWER_LIMIT_EXCEEDED;
                }
            }

            if (!util::isNaN(g_levels[i].i)) {
                if (g_levels[i].i > channel_dispatcher::getILimit(channel)) {
                    return SCPI_ERROR_CURRENT_LIMIT_EXCEEDED;
	            }

                if (g_levels[i].i * channel_dispatcher::getUSet(channel) > channel_dispatcher::getPowerLimit(channel)) {
                    return SCPI_ERROR_POWER_LIMIT_EXCEEDED;
                }
            }
        }
    }

    for (int i = 0; i < CH_NUM; ++i) {
        Channel& channel = Channel::get(i);

        if (i == 0 || !(channel_dispatcher::isCoupled() || channel_dispatcher::isTracked())) {
            if (!util::isNaN(g_levels[i].u)) {
                channel_dispatcher::setVoltage(channel, g_levels[i].u);
            }

            if (!util::isNaN(g_levels[i].i)) {
                channel_dispatcher::setCurrent(channel, g_levels[i].i);
            }
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

void abort() {
    reset();
}

void tick(uint32_t tick_usec) {
    if (g_state == STATE_TRIGGERED) {
        check(tick_usec / 1000);
    }
}

}
}
} // namespace eez::psu::trigger