/*
 * EEZ PSU Firmware
 * Copyright (C) 2015-present, Envox d.o.o.
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

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9

#define BP_LED_OUT1_PLUS      7
#define BP_LED_OUT1_PLUS_RED  6
#define BP_LED_SENSE1_PLUS    5
#define BP_LED_SENSE1_MINUS   4
#define BP_LED_OUT1_MINUS     3
#define BP_LED_OUT1_MINUS_RED 2

#define BP_LED_OUT2_PLUS      13
#define BP_LED_SENSE2_PLUS    12
#define BP_LED_SENSE2_MINUS   11
#define BP_LED_OUT2_MINUS     10

#define BP_RELAY_SENSE1       1
#define BP_RELAY_SENSE2       14

#define BP_K_PAR              9
#define BP_K_SER              8

#define BP_STANDBY            15

#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R2B6

#define BP_LED_OUT1           2
#define BP_LED_OUT1_RED       3
#define BP_LED_SENSE1         5

#define BP_LED_OUT2           13
#define BP_LED_SENSE2         12

#define BP_RELAY_SENSE1       1
#define BP_RELAY_SENSE2       14

#define BP_K_PAR              9
#define BP_K_SER              8

#define BP_STANDBY            15

#define BP_LED_CV1            7
#define BP_LED_CC1            6

#define BP_LED_CV2            0
#define BP_LED_CC2            10

#define BP_LED_PROG1          4
#define BP_LED_PROG2          11

#endif

namespace eez {
namespace psu {
namespace bp {

extern psu::TestResult test_result;

void init();

void switchStandby(bool on);
void switchOutput(Channel *channel, bool on);
void switchSense(Channel *channel, bool on);

void switchProg(Channel *channel, bool on);

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R2B6
void cvLedSwitch(Channel *channel, bool on);
void ccLedSwitch(Channel *channel, bool on);
#endif

}
}
} // namespace eez::psu::bp
