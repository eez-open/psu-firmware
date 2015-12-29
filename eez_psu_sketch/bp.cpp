/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
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
#include "bp.h"

namespace eez {
namespace psu {
namespace bp {

static uint16_t last_conf;

////////////////////////////////////////////////////////////////////////////////

void set(uint16_t conf) {
    if (OPTION_BP) {
        SPI.beginTransaction(TLC5925_SPI);
        digitalWrite(BP_OE, HIGH);
        digitalWrite(BP_SELECT, LOW);
        SPI.transfer(conf >> 8);
        SPI.transfer(conf & 0xFF);
        last_conf = conf;
        digitalWrite(BP_SELECT, HIGH);
        digitalWrite(BP_SELECT, LOW);
        digitalWrite(BP_OE, LOW);
        SPI.endTransaction();
    }
}

void bp_switch(uint16_t mask, bool on) {
    uint16_t conf = last_conf;

    if (on) {
        conf |= mask;
        if (mask & BP_LED_OUT1_PLUS) conf &= ~BP_LED_OUT1_PLUS_RED;
        if (mask & BP_LED_OUT1_MINUS) conf &= ~BP_LED_OUT1_MINUS_RED;
    }
    else {
        conf &= ~mask;
    }

    if (conf != last_conf) {
        set(conf);
    }
}

////////////////////////////////////////////////////////////////////////////////

void init() {
    switchStandby(true);
}

void switchStandby(bool on) {
    set(on ? BP_STANDBY : 0);
}

void switchOutput(Channel *channel, bool on) {
    bp_switch(channel->bp_led_out_plus |
        channel->bp_led_out_minus, on);
}

void switchSense(Channel *channel, bool on) {
    bp_switch(channel->bp_led_sense_plus |
        channel->bp_led_sense_minus |
        channel->bp_relay_sense, on);
}

}
}
} // namespace eez::psu::bp