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

#include "psu.h"
#include "unit.h"

namespace eez {
namespace app {

int getNumSignificantDecimalDigits(Unit unit) {
	static int g_numSignificantDecimalDigits[] = {
		2,
		VOLTAGE_NUM_SIGNIFICANT_DECIMAL_DIGITS,
		VOLTAGE_NUM_SIGNIFICANT_DECIMAL_DIGITS - 3,
		CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS,
		CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS - 3,
		POWER_NUM_SIGNIFICANT_DECIMAL_DIGITS,
		POWER_NUM_SIGNIFICANT_DECIMAL_DIGITS - 3,
		DURATION_NUM_SIGNIFICANT_DECIMAL_DIGITS,
		DURATION_NUM_SIGNIFICANT_DECIMAL_DIGITS - 3,
		TEMP_NUM_SIGNIFICANT_DECIMAL_DIGITS,
		RPM_NUM_SIGNIFICANT_DECIMAL_DIGITS,
		LOAD_NUM_SIGNIFICANT_DECIMAL_DIGITS,
		LOAD_NUM_SIGNIFICANT_DECIMAL_DIGITS,
		LOAD_NUM_SIGNIFICANT_DECIMAL_DIGITS,
		PERCENT_NUM_SIGNIFICANT_DECIMAL_DIGITS
	};

	return g_numSignificantDecimalDigits[unit];
}

float getPrecision(Unit unit) {
	static float g_precisions[] = {
		powf(10.0f, 2.0f),
		powf(10.0f, (float)VOLTAGE_NUM_SIGNIFICANT_DECIMAL_DIGITS),
		powf(10.0f, (float)(VOLTAGE_NUM_SIGNIFICANT_DECIMAL_DIGITS - 3)),
		powf(10.0f, (float)CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS),
		powf(10.0f, (float)(CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS - 3)),
		powf(10.0f, (float)POWER_NUM_SIGNIFICANT_DECIMAL_DIGITS),
		powf(10.0f, (float)(POWER_NUM_SIGNIFICANT_DECIMAL_DIGITS - 3)),
		powf(10.0f, (float)DURATION_NUM_SIGNIFICANT_DECIMAL_DIGITS),
		powf(10.0f, (float)(DURATION_NUM_SIGNIFICANT_DECIMAL_DIGITS - 3)),
		powf(10.0f, (float)TEMP_NUM_SIGNIFICANT_DECIMAL_DIGITS),
		powf(10.0f, (float)RPM_NUM_SIGNIFICANT_DECIMAL_DIGITS),
		powf(10.0f, (float)LOAD_NUM_SIGNIFICANT_DECIMAL_DIGITS),
		powf(10.0f, (float)LOAD_NUM_SIGNIFICANT_DECIMAL_DIGITS),
		powf(10.0f, (float)LOAD_NUM_SIGNIFICANT_DECIMAL_DIGITS),
		powf(10.0f, (float)PERCENT_NUM_SIGNIFICANT_DECIMAL_DIGITS)
	};

	return g_precisions[unit];
}


}
} // namespace eez::app::util
