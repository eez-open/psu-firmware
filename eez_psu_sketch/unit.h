/*
* EEZ PSU Firmware
* Copyright (C) 2018-present, Envox d.o.o.
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

#define VOLTAGE_NUM_SIGNIFICANT_DECIMAL_DIGITS 2
#define CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS 3
#define POWER_NUM_SIGNIFICANT_DECIMAL_DIGITS 3
#define DURATION_NUM_SIGNIFICANT_DECIMAL_DIGITS 4
#define TEMP_NUM_SIGNIFICANT_DECIMAL_DIGITS 0
#define RPM_NUM_SIGNIFICANT_DECIMAL_DIGITS 0
#define LOAD_NUM_SIGNIFICANT_DECIMAL_DIGITS 2
#define PERCENT_NUM_SIGNIFICANT_DECIMAL_DIGITS 2

#include "mw_unit.h"

namespace eez {
namespace psu {

int getNumSignificantDecimalDigits(Unit unit);
float getPrecision(Unit unit);

}
} // eez::psu