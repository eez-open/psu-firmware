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
#include "io_pins.h"
#include "persist_conf.h"
#include "fan.h"

namespace eez {
namespace psu {
namespace io_pins {

static struct {
    unsigned outputFault: 2;
    unsigned outputEnabled: 2;
    unsigned toutputPulse: 1;
    unsigned inhibited : 1;
} g_lastState = {
    2,
    2,
    0,
    0
};

static uint32_t g_toutputPulseStartTickCount;

static bool g_digitalOutputPinState[2] = { false, false };

uint8_t isOutputFault() {
    if (psu::isPowerUp()) {
        if (fan::g_testResult == TEST_FAILED) {
            return 1;
        }
    }

    for (int i = 0; i < CH_NUM; ++i) {
        Channel& channel = Channel::get(i);
        if (channel.isTripped() || !channel.isTestOk()) {
            return 1;
        }
    }

    return 0;
}

uint8_t isOutputEnabled() {
    for (int i = 0; i < CH_NUM; ++i) {
        if (Channel::get(i).isOutputEnabled()) {
            return 1;
        }
    }
    return 0;
}

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
void updateFaultPin(int i) {
    persist_conf::IOPin &outputPin = persist_conf::devConf2.ioPins[i];
    int pin = i == 1 ? DOUT : DOUT2;
    int state = g_lastState.outputFault && outputPin.polarity == io_pins::POLARITY_POSITIVE ||  
        !g_lastState.outputFault && outputPin.polarity == io_pins::POLARITY_NEGATIVE
        ? 1 : 0;
    digitalWrite(pin, state);
    //DebugTraceF("FUNCTION_FAULT %d %d", pin, state);
}

void updateOnCouplePin(int i) {
    persist_conf::IOPin &outputPin = persist_conf::devConf2.ioPins[i];
    int pin = i == 1 ? DOUT : DOUT2;
    int state = g_lastState.outputEnabled && outputPin.polarity == io_pins::POLARITY_POSITIVE ||  
        !g_lastState.outputEnabled && outputPin.polarity == io_pins::POLARITY_NEGATIVE
        ? 1 : 0; 
    digitalWrite(pin, state);
    //DebugTraceF("FUNCTION_ON_COUPLE %d %d", pin, state);
}
#endif

void tick(uint32_t tickCount) {
    // execute input pins function
    unsigned inhibited = 0;
    persist_conf::IOPin &inputPin = persist_conf::devConf2.ioPins[0];
    if (inputPin.function == io_pins::FUNCTION_INHIBIT) {
        int value = digitalRead(EXT_TRIG);
        inhibited = value && inputPin.polarity == io_pins::POLARITY_POSITIVE || !value && inputPin.polarity == io_pins::POLARITY_NEGATIVE ? 1 : 0;
    }
    if (inhibited != g_lastState.inhibited) {
        g_lastState.inhibited = inhibited;
        for (int i = 0; i < CH_NUM; ++i) {
            Channel::get(i).onInhibitedChanged(inhibited ? true : false);
        }
    }

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
    // end trigger output pulse
    if (g_lastState.toutputPulse) {
        int32_t diff = tickCount - g_toutputPulseStartTickCount;
        if (diff > CONF_TOUTPUT_PULSE_WIDTH_MS * 1000L) {
            for (int i = 1; i < 3; ++i) {
                persist_conf::IOPin &outputPin = persist_conf::devConf2.ioPins[i];
                if (outputPin.function == io_pins::FUNCTION_TOUTPUT) {
                    digitalWrite(i == 1 ? DOUT : DOUT2, outputPin.polarity == io_pins::POLARITY_POSITIVE ? 0 : 1);
                }
            }

            g_lastState.toutputPulse = 0;
        }
    }

    enum {
        UNKNOWN,
        UNCHANGED,
        CHANGED
    } trippedState = UNKNOWN, outputEnabledState = UNKNOWN;

    // execute output pins function
    for (int i = 1; i < 3; ++i) {
        persist_conf::IOPin &outputPin = persist_conf::devConf2.ioPins[i];

        if (outputPin.function == io_pins::FUNCTION_FAULT) {
            if (trippedState == UNKNOWN) {
                uint8_t outputFault = isOutputFault();
                if (g_lastState.outputFault != outputFault) {
                    g_lastState.outputFault = outputFault;
                    trippedState = CHANGED;
                } else {
                    trippedState = UNCHANGED;
                }
            }

            if (trippedState == CHANGED) {
                updateFaultPin(i);
            }
        } else if (outputPin.function == io_pins::FUNCTION_ON_COUPLE) {
            if (outputEnabledState == UNKNOWN) {
                uint8_t outputEnabled = isOutputEnabled();
                if (g_lastState.outputEnabled != outputEnabled) {
                    g_lastState.outputEnabled = outputEnabled;
                    outputEnabledState = CHANGED;
                } else {
                    outputEnabledState = UNCHANGED;
                }
            }

            if (outputEnabledState == CHANGED) {
                updateOnCouplePin(i);
            }
        }
    }
#endif
}

void onTrigger() {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
    // start trigger output pulse
    for (int i = 1; i < 3; ++i) {
        persist_conf::IOPin &outputPin = persist_conf::devConf2.ioPins[i];
        if (outputPin.function == io_pins::FUNCTION_TOUTPUT) {
            digitalWrite(i == 1 ? DOUT : DOUT2, outputPin.polarity == io_pins::POLARITY_POSITIVE ? 1 : 0);
            g_lastState.toutputPulse = 1;
            g_toutputPulseStartTickCount = micros();
        }
    }
#endif
}

void refresh() {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
    // refresh output pins
    for (int i = 1; i < 3; ++i) {
        persist_conf::IOPin &outputPin = persist_conf::devConf2.ioPins[i];

        if (outputPin.function == io_pins::FUNCTION_NONE) {
            digitalWrite(i == 1 ? DOUT : DOUT2, 0);
        } else if (outputPin.function == io_pins::FUNCTION_FAULT) {
            updateFaultPin(i);
        } else if (outputPin.function == io_pins::FUNCTION_ON_COUPLE) {
            updateOnCouplePin(i);
        }
    }
#endif
}

bool isInhibited() {
    return g_lastState.inhibited ? true : false;
}

void setDigitalOutputPinState(int pin, bool state) {
	g_digitalOutputPinState[pin - 2] = state;

	if (persist_conf::devConf2.ioPins[pin - 1].polarity == io_pins::POLARITY_NEGATIVE) {
		state = !state;
	}

	if (pin == 2) {
		digitalWrite(DOUT, state);
	} else {
		digitalWrite(DOUT2, state);
	}
}

bool getDigitalOutputPinState(int pin) {
	return g_digitalOutputPinState[pin - 2];
}

}
}
} // namespace eez::psu::io_pins