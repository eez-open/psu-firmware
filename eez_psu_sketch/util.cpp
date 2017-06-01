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
namespace psu {
namespace util {

float remap(float x, float x1, float y1, float x2, float y2) {
    return y1 + (x - x1) * (y2 - y1) / (x2 - x1);
}

float remapQuad(float x, float x1, float y1, float x2, float y2) {
    float t = remap(x, x1, 0, x2, 1);
    t = t * t;
    x = remap(t, 0, x1, 1, x2);
    return remap(x, x1, y1, x2, y2);
}

float remapCubic(float x, float x1, float y1, float x2, float y2) {
    float t = remap(x, x1, 0, x2, 1);
    t = t * t * t;
    x = remap(t, 0, x1, 1, x2);
    return remap(x, x1, y1, x2, y2);
}

float remapExp(float x, float x1, float y1, float x2, float y2) {
    float t = remap(x, x1, 0, x2, 1);
    t = t == 0 ? 0 : float(pow(2, 10 * (t - 1)));
    x = remap(t, 0, x1, 1, x2);
    return remap(x, x1, y1, x2, y2);
}

float clamp(float x, float min, float max) {
    if (x <= min) {
        return min;
    }
    if (x >= max) {
        return max;
    }
    return x;
}

void strcatInt(char *str, int value) {
    str = str + strlen(str);
    sprintf(str, "%d", value);
}

void strcatInt32(char *str, int32_t value) {
    str = str + strlen(str);
    sprintf(str, "%ld", (long)value);
}

void strcatUInt32(char *str, uint32_t value) {
    str = str + strlen(str);
    sprintf(str, "%lu", (unsigned long)value);
}

void strcatFloat(char *str, float value, int numSignificantDecimalDigits) {
    // mitigate "-0.00" case
    float min = (float) (1.0f / pow(10, numSignificantDecimalDigits)) / 2;
    if (fabs(value) < min) {
        value = 0;
    }

    str = str + strlen(str);

#if defined(_VARIANT_ARDUINO_DUE_X_) || defined(EEZ_PSU_SIMULATOR)
    sprintf(str, "%.*f", numSignificantDecimalDigits, value);
#else
    dtostrf(value, 0, precision, str);
#endif

    // remove trailing zeros
    /*
    str = str + strlen(str) - 1;
    while (*str == '0' && *(str - 1) != '.') {
        *str-- = 0;
    }
    */
}

void strcatFloat(char *str, float value, ValueType valueType, int channelIndex) {
    int numSignificantDecimalDigits = getNumSignificantDecimalDigits(valueType);
    if (valueType == VALUE_TYPE_FLOAT_AMPER && channelIndex != -1 && channel_dispatcher::isCurrentLowRangeAllowed(Channel::get(channelIndex)) && util::lessOrEqual(value, 0.5, getPrecision(VALUE_TYPE_FLOAT_AMPER))) {
        ++numSignificantDecimalDigits;
    }
    strcatFloat(str, value, numSignificantDecimalDigits);
}

void strcatVoltage(char *str, float value, int numSignificantDecimalDigits, int channelIndex) {
    if (numSignificantDecimalDigits == -1) {
        numSignificantDecimalDigits = getNumSignificantDecimalDigits(VALUE_TYPE_FLOAT_VOLT);
    }
    strcatFloat(str, value, numSignificantDecimalDigits);
    strcat(str, "V");
}

void strcatCurrent(char *str, float value, int numSignificantDecimalDigits, int channelIndex) {
    if (numSignificantDecimalDigits == -1) {
        numSignificantDecimalDigits = getNumSignificantDecimalDigits(VALUE_TYPE_FLOAT_AMPER);
    }
    if (channelIndex != -1 && channel_dispatcher::isCurrentLowRangeAllowed(Channel::get(channelIndex)) && util::lessOrEqual(value, 0.5, getPrecision(VALUE_TYPE_FLOAT_AMPER))) {
        ++numSignificantDecimalDigits;
    }
    strcatFloat(str, value, numSignificantDecimalDigits);
    strcat(str, "A");
}

void strcatPower(char *str, float value) {
    strcatFloat(str, value, getNumSignificantDecimalDigits(VALUE_TYPE_FLOAT_WATT));
    strcat(str, "W");
}

void strcatDuration(char *str, float value) {
    int numSignificantDecimalDigits = getNumSignificantDecimalDigits(VALUE_TYPE_FLOAT_SECOND);
    if (value > 0.1) {
        strcatFloat(str, value, numSignificantDecimalDigits);
        strcat(str, " s");
    }
    else {
        strcatFloat(str, value * 1000, numSignificantDecimalDigits - 3);
        strcat(str, " ms");
    }
}

void strcatLoad(char *str, float value) {
    int numSignificantDecimalDigits = getNumSignificantDecimalDigits(VALUE_TYPE_FLOAT_OHM);
    if (value < 1000) {
        strcatFloat(str, value, numSignificantDecimalDigits);
        strcat(str, " ohm");
    }
    else if (value < 1000000){
        strcatFloat(str, value / 1000, numSignificantDecimalDigits);
        strcat(str, " Kohm");
    }
    else {
        strcatFloat(str, value / 1000000, numSignificantDecimalDigits);
        strcat(str, " Mohm");
    }
}

/*
From http://www.hackersdelight.org/hdcodetxt/crc.c.txt:

This is the basic CRC-32 calculation with some optimization but no
table lookup. The the byte reversal is avoided by shifting the crc reg
right instead of left and by using a reversed 32-bit word to represent
the polynomial.
When compiled to Cyclops with GCC, this function executes in 8 + 72n
instructions, where n is the number of bytes in the input message. It
should be doable in 4 + 61n instructions.
If the inner loop is strung out (approx. 5*8 = 40 instructions),
it would take about 6 + 46n instructions.
*/

uint32_t crc32(const uint8_t *mem_block, size_t block_size) {
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < block_size; ++i) {
        uint32_t byte = mem_block[i];            // Get next byte.
        crc = crc ^ byte;
        for (int j = 0; j < 8; ++j) {    // Do eight times.
            uint32_t mask = -((int32_t)crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }
    return ~crc;
}

uint8_t toBCD(uint8_t bin) {
    return ((bin / 10) << 4) | (bin % 10);
}

uint8_t fromBCD(uint8_t bcd) {
    return ((bcd >> 4) & 0xF) * 10 + (bcd & 0xF);
}

float floorPrec(float a, float prec) {
    return floorf(a * prec) / prec;
}

float ceilPrec(float a, float prec) {
    return ceilf(a * prec) / prec;
}

float roundPrec(float a, float prec) {
    return roundf(a * prec) / prec;
}

bool greater(float a, float b, float prec) {
	return a > b && !equal(a, b, prec);
}

bool greater(float a, float b, ValueType valueType, int channelIndex) {
    return a > b && !equal(a, b, getPrecision(b, valueType, channelIndex));
}

bool greaterOrEqual(float a, float b, float prec) {
	return a > b || equal(a, b, prec);
}

bool greaterOrEqual(float a, float b, ValueType valueType, int channelIndex) {
	return a > b || equal(a, b, valueType, channelIndex);
}

bool less(float a, float b, float prec) {
    return a < b && !equal(a, b, prec);
}

bool less(float a, float b, ValueType valueType, int channelIndex) {
    return a < b && !equal(a, b, getPrecision(b, valueType, channelIndex));
}

bool lessOrEqual(float a, float b, float prec) {
	return a < b || equal(a, b, prec);
}

bool lessOrEqual(float a, float b, ValueType valueType, int channelIndex) {
	return a < b || equal(a, b, valueType, channelIndex);
}

bool equal(float a, float b, float prec) {
	return roundf(a * prec) == roundf(b * prec);
}

bool equal(float a, float b, ValueType valueType, int channelIndex) {
    float prec = getPrecision(b, valueType, channelIndex);
	return roundf(a * prec) == roundf(b * prec);
}

bool between(float x, float a, float b, float prec) {
    return greaterOrEqual(x, a, prec) && lessOrEqual(x, b, prec);
}

bool between(float x, float a, float b, ValueType valueType, int channelIndex) {
    return greaterOrEqual(x, a, valueType, channelIndex) && lessOrEqual(x, b, valueType, channelIndex);
}

float multiply(float a, float b, float prec) {
	return roundPrec(a, prec) * roundPrec(b, prec);
}

bool isNaN(float x) {
	return x != x;
}

bool isDigit(char ch) {
    return ch >= '0' && ch <= '9';
}

bool isUperCaseLetter(char ch) {
    return ch >= 'A' && ch <= 'Z';
}

void removeTrailingZerosFromFloat(char *str) {
    int n = strlen(str);
    for (int i = strlen(str) - 1; i >= 0; --i) {
        if (str[i] == '0') {
            str[i] = 0;
        } else {
            if (str[i] == '.') {
                str[i] = 0;
            }
            break;
        }
    }
}

bool pointInsideRect(int xPoint, int yPoint, int xRect, int yRect, int wRect, int hRect) {
    return xPoint >= xRect && xPoint < xRect + wRect &&
        yPoint >= yRect && yPoint < yRect + hRect;
}

void getParentDir(const char *path, char *parentDirPath) {
    int lastPathSeparatorIndex;

    for (lastPathSeparatorIndex = strlen(path) - 1;
        lastPathSeparatorIndex >= 0 && path[lastPathSeparatorIndex] != PATH_SEPARATOR[0];
        --lastPathSeparatorIndex);

    int i;
    for (i = 0; i < lastPathSeparatorIndex; ++i) {
        parentDirPath[i] = path[i];
    }
    parentDirPath[i] = 0;
}

bool parseIpAddress(const char *ipAddressStr, size_t ipAddressStrLength, uint32_t &ipAddress) {
    const char *p = ipAddressStr;
    const char *q = ipAddressStr + ipAddressStrLength;

    uint8_t ipAddressArray[4];

    for (int i = 0; i < 4; ++i) {
        if (p == q) {
            return false;
        }

        uint32_t part = 0;
        for (int j = 0; j < 3; ++j) {
            if (p == q) {
                if (j > 0 && i == 3) {
                    break;
                } else {
                    return false;
                }
            } else if (isDigit(*p)) {
                part = part * 10 + (*p++ - '0');
            } else if (j > 0 && *p == '.') {
                break;
            } else {
                return false;
            }
        }

        if (part > 255) {
            return false;
        }

        if (i < 3 && *p++ != '.' || i == 3 && p != q) {
            return false;
        }

        ipAddressArray[i] = part;
    }

    ipAddress = arrayToIpAddress(ipAddressArray);

    return true;
}

int getIpAddressPartA(uint32_t ipAddress) {
    return ((uint8_t *)&ipAddress)[0];
}

void setIpAddressPartA(uint32_t *ipAddress, uint8_t value) {
    ((uint8_t *)ipAddress)[0] = value;
}

int getIpAddressPartB(uint32_t ipAddress) {
    return ((uint8_t *)&ipAddress)[1];
}

void setIpAddressPartB(uint32_t *ipAddress, uint8_t value) {
    ((uint8_t *)ipAddress)[1] = value;
}

int getIpAddressPartC(uint32_t ipAddress) {
    return ((uint8_t *)&ipAddress)[2];
}

void setIpAddressPartC(uint32_t *ipAddress, uint8_t value) {
    ((uint8_t *)ipAddress)[2] = value;
}

int getIpAddressPartD(uint32_t ipAddress) {
    return ((uint8_t *)&ipAddress)[3];
}

void setIpAddressPartD(uint32_t *ipAddress, uint8_t value) {
    ((uint8_t *)ipAddress)[3] = value;
}

void ipAddressToArray(uint32_t ipAddress, uint8_t *ipAddressArray) {
    ipAddressArray[0] = getIpAddressPartA(ipAddress);
    ipAddressArray[1] = getIpAddressPartB(ipAddress);
    ipAddressArray[2] = getIpAddressPartC(ipAddress);
    ipAddressArray[3] = getIpAddressPartD(ipAddress);
}

uint32_t arrayToIpAddress(uint8_t *ipAddressArray) {
    return getIpAddress(ipAddressArray[0], ipAddressArray[1], ipAddressArray[2], ipAddressArray[3]);
}

uint32_t getIpAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d) {
    uint32_t ipAddress;

    setIpAddressPartA(&ipAddress, a);
    setIpAddressPartB(&ipAddress, b);
    setIpAddressPartC(&ipAddress, c);
    setIpAddressPartD(&ipAddress, d);

    return ipAddress;
}

void ipAddressToString(uint32_t ipAddress, char *ipAddressStr) {
    sprintf(ipAddressStr, "%d.%d.%d.%d",
        getIpAddressPartA(ipAddress),
        getIpAddressPartB(ipAddress),
        getIpAddressPartC(ipAddress),
        getIpAddressPartD(ipAddress));
}

void macAddressToString(uint8_t *macAddress, char *macAddressStr) {
    for (int i = 0; i < 6; ++i) {
        macAddressStr[3*i] = util::hexDigit((macAddress[i] & 0xF0) >> 4);
        macAddressStr[3*i+1] = util::hexDigit(macAddress[i] & 0xF);
        macAddressStr[3*i+2] = i < 5 ? '-' : 0;
    }
}

char hexDigit(int num) {
    if (num >= 0 && num <= 9) {
        return '0' + num;
    } else {
        return 'A' + (num - 10);
    }
}

void formatTimeZone(int16_t timeZone, char *text, int count) {
    if (timeZone == 0) {
        strncpy_P(text, PSTR("GMT"), count - 1);
    } else {
        char sign;
        int16_t value;
        if (timeZone > 0) {
            sign = '+';
            value = timeZone;
        } else {
            sign = '-';
            value = -timeZone;
        }
        snprintf_P(text, count-1, PSTR("%c%02d:%02d GMT"), sign, value / 100, value % 100);
    }
    text[count - 1] = 0;
}

bool parseTimeZone(const char *timeZoneStr, size_t timeZoneLength, int16_t &timeZone) {
    int state = 0;

    int sign = 1;
    int integerPart = 0;
    int fractionPart = 0;

    const char *end = timeZoneStr + timeZoneLength;
    for (const char *p = timeZoneStr; p < end; ++p) {
        if (*p == ' ') {
            continue;
        }

        if (state == 0) {
            if (*p == '+') {
                state = 1;
            } else if (*p == '-') {
                sign = -1;
                state = 1;
            } else if (isDigit(*p)) {
                integerPart = *p - '0';
                state = 2;
            } else {
                return false;
            }
        } else if (state == 1) {
            if (isDigit(*p)) {
                integerPart = (*p - '0');
                state = 2;
            } else {
                return false;
            }
        } else if (state == 2) {
            if (*p == ':') {
                state = 4;
            } else if (isDigit(*p)) {
                integerPart = integerPart * 10 + (*p - '0');
                state = 3;
            } else {
                return false;
            }
        } else if (state == 3) {
            if (*p == ':') {
                state = 4;
            } else {
                return false;
            }
        } else if (state == 4) {
            if (isDigit(*p)) {
                fractionPart = (*p - '0');
                state = 5;
            } else {
                return false;
            }
        } else if (state == 5) {
            if (isDigit(*p)) {
                fractionPart = fractionPart * 10 + (*p - '0');
                state = 6;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    if (state != 2 && state != 3 && state != 6) {
        return false;
    }

    int value= sign * (integerPart * 100 + fractionPart);

    if (value < -1200 || value > 1400) {
        return false;
    }

    timeZone = (int16_t)value;

    return true;
}

}
}
} // namespace eez::psu::util