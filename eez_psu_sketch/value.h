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

#pragma once

#define VOLTAGE_NUM_SIGNIFICANT_DECIMAL_DIGITS 2
#define CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS 3
#define POWER_NUM_SIGNIFICANT_DECIMAL_DIGITS 3
#define DURATION_NUM_SIGNIFICANT_DECIMAL_DIGITS 4
#define TEMP_NUM_SIGNIFICANT_DECIMAL_DIGITS 0
#define RPM_NUM_SIGNIFICANT_DECIMAL_DIGITS 0
#define LOAD_NUM_SIGNIFICANT_DECIMAL_DIGITS 2

namespace eez {
namespace psu {

class Channel;

enum ValueType {
    VALUE_TYPE_NONE,
    VALUE_TYPE_INT,
    VALUE_TYPE_FLOAT_FIRST,
	VALUE_TYPE_FLOAT,
    VALUE_TYPE_FLOAT_VOLT,
    VALUE_TYPE_FLOAT_MILLI_VOLT,
    VALUE_TYPE_FLOAT_AMPER,
    VALUE_TYPE_FLOAT_MILLI_AMPER,
	VALUE_TYPE_FLOAT_WATT,
	VALUE_TYPE_FLOAT_MILLI_WATT,
	VALUE_TYPE_FLOAT_SECOND,
    VALUE_TYPE_FLOAT_MILLI_SECOND,
	VALUE_TYPE_FLOAT_CELSIUS,
	VALUE_TYPE_FLOAT_RPM,
    VALUE_TYPE_FLOAT_OHM,
    VALUE_TYPE_FLOAT_KOHM,
    VALUE_TYPE_FLOAT_MOHM,
    VALUE_TYPE_FLOAT_LAST,
    VALUE_TYPE_LESS_THEN_MIN_FLOAT,
    VALUE_TYPE_GREATER_THEN_MAX_FLOAT = VALUE_TYPE_LESS_THEN_MIN_FLOAT + VALUE_TYPE_FLOAT_LAST - VALUE_TYPE_FLOAT_FIRST,
    VALUE_TYPE_CONST_STR = VALUE_TYPE_GREATER_THEN_MAX_FLOAT + VALUE_TYPE_FLOAT_LAST - VALUE_TYPE_FLOAT_FIRST,
    VALUE_TYPE_STR,
    VALUE_TYPE_CHANNEL_LABEL,
	VALUE_TYPE_CHANNEL_SHORT_LABEL,
	VALUE_TYPE_CHANNEL_BOARD_INFO_LABEL,
	VALUE_TYPE_LESS_THEN_MIN_INT,
	VALUE_TYPE_LESS_THEN_MIN_TIME_ZONE,
	VALUE_TYPE_GREATER_THEN_MAX_INT,
	VALUE_TYPE_GREATER_THEN_MAX_TIME_ZONE,
	VALUE_TYPE_EVENT,
	VALUE_TYPE_PAGE_INFO,
	VALUE_TYPE_ON_TIME_COUNTER,
    VALUE_TYPE_COUNTDOWN,
	VALUE_TYPE_SCPI_ERROR_TEXT,
	VALUE_TYPE_TIME_ZONE,
	VALUE_TYPE_YEAR,
	VALUE_TYPE_MONTH,
	VALUE_TYPE_DAY,
	VALUE_TYPE_HOUR,
	VALUE_TYPE_MINUTE,
	VALUE_TYPE_SECOND,
	VALUE_TYPE_USER_PROFILE_LABEL,
	VALUE_TYPE_USER_PROFILE_REMARK,
    VALUE_TYPE_EDIT_INFO,
    VALUE_TYPE_MAC_ADDRESS,
    VALUE_TYPE_IP_ADDRESS,
    VALUE_TYPE_PORT,
    VALUE_TYPE_ENUM,
    VALUE_TYPE_TEXT_MESSAGE,
    VALUE_TYPE_SERIAL_BAUD_INDEX
};

struct ValueTypeTraits {
    const char *unitStr;
    int numSignificantDecimalDigits;
    float precision;
};

extern ValueTypeTraits g_valueTypeTraits[];
extern float g_precisions[];

const char *getUnitStr(ValueType valueType);

inline int getNumSignificantDecimalDigits(ValueType valueType) { 
    return g_valueTypeTraits[valueType - VALUE_TYPE_FLOAT].numSignificantDecimalDigits; 
}

inline float getPrecision(ValueType valueType) { 
    return g_valueTypeTraits[valueType - VALUE_TYPE_FLOAT].precision; 
}

float getPrecision(float value, ValueType valueType, int channelIndex);

inline float getPrecisionFromNumSignificantDecimalDigits(int numSignificantDecimalDigits) {
    return g_precisions[numSignificantDecimalDigits]; 
}

}
} // namespace eez::psu