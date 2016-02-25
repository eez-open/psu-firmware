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

namespace eez {
namespace psu {
namespace util {

float remap(float x, float x1, float y1, float x2, float y2) {
    return y1 + (x - x1) * (y2 - y1) / (x2 - x1);
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

void strcatFloat(char *str, float value) {
    // mitigate "-0.00" case
    float min = 1.0f / pow(10, FLOAT_TO_STR_PREC);
    if (abs(value) < min) {
        value = 0;
    }

    str = str + strlen(str);

#if defined(_VARIANT_ARDUINO_DUE_X_) || defined(EEZ_PSU_SIMULATOR)
    sprintf(str, "%.*f", FLOAT_TO_STR_PREC, value);
#else
    dtostrf(value, 0, FLOAT_TO_STR_PREC, str);
#endif

    // remove trailing zeros
    /*
    str = str + strlen(str) - 1;
    while (*str == '0' && *(str - 1) != '.') {
        *str-- = 0;
    }
    */
}

void strcatVoltage(char *str, float value) {
    strcatFloat(str, value);
    strcat(str, " V");
}

void strcatCurrent(char *str, float value) {
    strcatFloat(str, value);
    strcat(str, " A");
}

void strcatDuration(char *str, float value) {
    if (value > 0.1) {
        strcatFloat(str, value);
        strcat(str, " s");
    }
    else {
        strcatFloat(str, value * 1000);
        strcat(str, " ms");
    }
}

void strcatLoad(char *str, float value) {
    if (value < 1000) {
        strcatFloat(str, value);
        strcat(str, " ohm");
    }
    else if (value < 1000000){
        strcatFloat(str, value / 1000);
        strcat(str, " Kohm");
    }
    else {
        strcatFloat(str, value / 1000000);
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
            uint32_t mask = -(crc & 1);
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

}
}
} // namespace eez::psu::util