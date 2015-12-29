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
 
#pragma once

#define BP_LED_OUT1_PLUS      (1 <<  7)
#define BP_LED_OUT1_PLUS_RED  (1 <<  6)
#define BP_LED_SENSE1_PLUS    (1 <<  5)
#define BP_LED_SENSE1_MINUS   (1 <<  4)
#define BP_LED_OUT1_MINUS     (1 <<  3)
#define BP_LED_OUT1_MINUS_RED (1 <<  2)

#define BP_LED_OUT2_PLUS      (1 << 13)
#define BP_LED_SENSE2_PLUS    (1 << 12)
#define BP_LED_SENSE2_MINUS   (1 << 11)
#define BP_LED_OUT2_MINUS     (1 << 10)

#define BP_RELAY_SENSE1       (1 <<  1)
#define BP_RELAY_SENSE2       (1 << 14)

#define BP_K_PAR              (1 <<  9)
#define BP_K_SER              (1 <<  8)

#define BP_STANDBY            (1 << 15)

namespace eez {
namespace psu {
namespace bp {

void init();

void switchStandby(bool on);
void switchOutput(Channel *channel, bool on);
void switchSense(Channel *channel, bool on);

}
}
} // namespace eez::psu::bp
