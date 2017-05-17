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

namespace eez {
namespace psu {
namespace io_pins {

static struct {
    unsigned tripped: 2;
    unsigned outputEnabled: 2;
    unsigned toutputPulse: 1;
} g_lastState = {
    2,
    2
};

static uint32_t g_toutputPulseStartTickCount;

uint8_t isTripped() {
    for (int i = 0; i < CH_NUM; ++i) {
        if (Channel::get(i).isTripped()) {
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

void tick(uint32_t tickCount) {
    // execute input pins function
    persist_conf::IOPin &inputPin = persist_conf::devConf2.ioPins[0];
    if (inputPin.function == io_pins::FUNCTION_INHIBIT) {
        int value = digitalRead(EXT_TRIG);
        if (value && inputPin.polarity == io_pins::POLARITY_POSITIVE || !value && inputPin.polarity == io_pins::POLARITY_NEGATIVE) {
            for (int i = 0; i < CH_NUM; ++i) {
                if (Channel::get(i).isOutputEnabled()) {
                    Channel::get(i).outputEnable(false);
                }
            }
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

    // execute output pins function
    for (int i = 1; i < 3; ++i) {
        persist_conf::IOPin &outputPin = persist_conf::devConf2.ioPins[i];

        if (outputPin.function == io_pins::FUNCTION_FAULT) {
            uint8_t tripped = isTripped();
            if (g_lastState.tripped != tripped) {
                g_lastState.tripped = tripped;
                int pin = i == 1 ? DOUT : DOUT2;
                int state = tripped && outputPin.polarity == io_pins::POLARITY_POSITIVE ||  
                    !tripped && outputPin.polarity == io_pins::POLARITY_NEGATIVE
                    ? 1 : 0;
                digitalWrite(pin, state);
                DebugTraceF("FUNCTION_FAULT %d %d", pin, state);
            }
        } else if (outputPin.function == io_pins::FUNCTION_ON_COUPLE) {
            uint8_t outputEnabled = isOutputEnabled();
            if (g_lastState.outputEnabled != outputEnabled) {
                g_lastState.outputEnabled = outputEnabled;
                int pin = i == 1 ? DOUT : DOUT2;
                int state = outputEnabled && outputPin.polarity == io_pins::POLARITY_POSITIVE ||  
                    !outputEnabled && outputPin.polarity == io_pins::POLARITY_NEGATIVE
                    ? 1 : 0; 
                digitalWrite(pin, state);
                DebugTraceF("FUNCTION_ON_COUPLE %d %d", pin, state);
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

}
}
} // namespace eez::psu::io_pins