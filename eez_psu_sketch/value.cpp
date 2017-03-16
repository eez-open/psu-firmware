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
#include "channel.h"
#include "value.h"

namespace eez {
namespace psu {

const char *getUnitStr(ValueType valueType) {
    switch (valueType) {
    case VALUE_TYPE_FLOAT_VOLT:
        return "V";
    case VALUE_TYPE_FLOAT_MILLI_VOLT:
        return "mV";
    case VALUE_TYPE_FLOAT_AMPER:
        return "A";
    case VALUE_TYPE_FLOAT_MILLI_AMPER:
        return "mA";
    case VALUE_TYPE_FLOAT_WATT:
        return "W";
    case VALUE_TYPE_FLOAT_MILLI_WATT:
        return "mW";
    case VALUE_TYPE_FLOAT_SECOND:
        return "s";
    case VALUE_TYPE_FLOAT_MILLI_SECOND:
        return "ms";
    case VALUE_TYPE_FLOAT_CELSIUS:
        return "oC";
    case VALUE_TYPE_FLOAT_RPM:
        return "rpm";
    case VALUE_TYPE_FLOAT_OHM:
        return "ohm";
    case VALUE_TYPE_FLOAT_KOHM:
        return "Kohm";
    case VALUE_TYPE_FLOAT_MOHM:
        return "Mohm";
    }
    return "";
}

int getNumSignificantDecimalDigits(ValueType valueType) {
    if (valueType == VALUE_TYPE_FLOAT_VOLT) return VOLTAGE_NUM_SIGNIFICANT_DECIMAL_DIGITS;
    if (valueType == VALUE_TYPE_FLOAT_AMPER) return CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS;
    if (valueType == VALUE_TYPE_FLOAT_WATT) return POWER_NUM_SIGNIFICANT_DECIMAL_DIGITS;
    if (valueType == VALUE_TYPE_FLOAT_SECOND) return DURATION_NUM_SIGNIFICANT_DECIMAL_DIGITS;
    if (valueType == VALUE_TYPE_FLOAT_CELSIUS) return TEMP_NUM_SIGNIFICANT_DECIMAL_DIGITS;
    if (valueType == VALUE_TYPE_FLOAT_RPM) return RPM_NUM_SIGNIFICANT_DECIMAL_DIGITS;
    if (valueType == VALUE_TYPE_FLOAT_OHM) return LOAD_NUM_SIGNIFICANT_DECIMAL_DIGITS;
    return 2;
}

int getNumSignificantDecimalDigitsForCurrent(uint8_t currentRange) {
    if (currentRange == 1) {
        return CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS_R5B12;
    }
    return CURRENT_NUM_SIGNIFICANT_DECIMAL_DIGITS;
}

float getPrecisionFromNumSignificantDecimalDigits(int numSignificantDecimalDigits) {
    return powf(10.0f, (float)numSignificantDecimalDigits);
}

float getPrecision(ValueType valueType) {
    return powf(10.0f, (float)getNumSignificantDecimalDigits(valueType));
}

}
} // namespace eez::psu