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
#include "channel_dispatcher.h"

namespace eez {
namespace app {

void strcatFloatValue(char *str, float value, Unit unit, int channelIndex) {
    int numSignificantDecimalDigits = getNumSignificantDecimalDigits(unit);
    if (
		unit == UNIT_AMPER &&
		channelIndex != -1 &&
		channel_dispatcher::isCurrentLowRangeAllowed(Channel::get(channelIndex)) &&
		mw::lessOrEqual(value, 0.5, getPrecision(UNIT_AMPER)))
	{
        ++numSignificantDecimalDigits;
    }
	strcatFloat(str, value, numSignificantDecimalDigits);
}

void strcatVoltage(char *str, float value, int numSignificantDecimalDigits, int channelIndex) {
    if (numSignificantDecimalDigits == -1) {
        numSignificantDecimalDigits = getNumSignificantDecimalDigits(UNIT_VOLT);
    }
	strcatFloat(str, value, numSignificantDecimalDigits);
    strcat(str, "V");
}

void strcatCurrent(char *str, float value, int numSignificantDecimalDigits, int channelIndex) {
    if (numSignificantDecimalDigits == -1) {
        numSignificantDecimalDigits = getNumSignificantDecimalDigits(UNIT_AMPER);
    }
    if (channelIndex != -1 && channel_dispatcher::isCurrentLowRangeAllowed(Channel::get(channelIndex)) && mw::lessOrEqual(value, 0.5, getPrecision(UNIT_AMPER))) {
        ++numSignificantDecimalDigits;
    }
	strcatFloat(str, value, numSignificantDecimalDigits);
    strcat(str, "A");
}

void strcatPower(char *str, float value) {
	strcatFloat(str, value, getNumSignificantDecimalDigits(UNIT_WATT));
	strcat(str, "W");
}

void strcatDuration(char *str, float value) {
	int numSignificantDecimalDigits = getNumSignificantDecimalDigits(UNIT_SECOND);
	if (value > 0.1) {
		strcatFloat(str, value, numSignificantDecimalDigits);
		strcat(str, " s");
	} else {
		strcatFloat(str, value * 1000, numSignificantDecimalDigits - 3);
		strcat(str, " ms");
	}
}

void strcatLoad(char *str, float value) {
	int numSignificantDecimalDigits = getNumSignificantDecimalDigits(UNIT_OHM);
	if (value < 1000) {
		strcatFloat(str, value, numSignificantDecimalDigits);
		strcat(str, " ohm");
	} else if (value < 1000000) {
		strcatFloat(str, value / 1000, numSignificantDecimalDigits);
		strcat(str, " Kohm");
	} else {
		strcatFloat(str, value / 1000000, numSignificantDecimalDigits);
		strcat(str, " Mohm");
	}
}

float getPrecision(float value, Unit unit, int channelIndex) {
	int numSignificantDecimalDigits = getNumSignificantDecimalDigits(unit);

	if (mw::greater(value, 99.99f, getPrecisionFromNumSignificantDecimalDigits(2))) {
		if (numSignificantDecimalDigits > 1) {
			numSignificantDecimalDigits = 1;
		}
	} else if (mw::greater(value, 9.999f, getPrecisionFromNumSignificantDecimalDigits(2))) {
		if (numSignificantDecimalDigits > 2) {
			numSignificantDecimalDigits = 2;
		}
	} else {
		if (unit == UNIT_AMPER) {
			if (channelIndex != -1 && channel_dispatcher::isCurrentLowRangeAllowed(Channel::get(channelIndex)) && mw::lessOrEqual(value, 0.5, getPrecision(UNIT_AMPER))) {
				++numSignificantDecimalDigits;
			}
		}
	}

	return getPrecisionFromNumSignificantDecimalDigits(numSignificantDecimalDigits);
}

bool greater(float a, float b, Unit unit, int channelIndex) {
	return a > b && !mw::equal(a, b, getPrecision(b, unit, channelIndex));
}

bool greaterOrEqual(float a, float b, Unit unit, int channelIndex) {
	return a > b || equal(a, b, unit, channelIndex);
}

bool less(float a, float b, Unit unit, int channelIndex) {
	return a < b && !mw::equal(a, b, getPrecision(b, unit, channelIndex));
}

bool lessOrEqual(float a, float b, Unit unit, int channelIndex) {
	return a < b || equal(a, b, unit, channelIndex);
}

bool equal(float a, float b, Unit unit, int channelIndex) {
	float prec = getPrecision(b, unit, channelIndex);
	return roundf(a * prec) == roundf(b * prec);
}

bool between(float x, float a, float b, Unit unit, int channelIndex) {
	return greaterOrEqual(x, a, unit, channelIndex) && lessOrEqual(x, b, unit, channelIndex);
}

}
} // namespace eez::app::util
