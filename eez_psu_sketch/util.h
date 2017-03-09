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

#define clear_bit(reg, bitmask) *reg &= ~bitmask
#define set_bit(reg, bitmask) *reg |= bitmask

#define util_swap(type, i, j) {type t = i; i = j; j = t;}

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

#include "gui_data.h"

namespace eez {
namespace psu {

/// Various utility functions.
namespace util {

float remap(float x, float x1, float y1, float x2, float y2);
float clamp(float x, float min, float max);

void strcatInt(char *str, int value);
void strcatUInt32(char *str, uint32_t value);
void strcatFloat(char *str, float value, int numSignificantDecimalDigits = FLOAT_TO_STR_NUM_DECIMAL_DIGITS);
void strcatVoltage(char *str, float value, int numSignificantDecimalDigits = FLOAT_TO_STR_NUM_DECIMAL_DIGITS);
void strcatCurrent(char *str, float value, int numSignificantDecimalDigits = FLOAT_TO_STR_NUM_DECIMAL_DIGITS);
void strcatDuration(char *str, float value);
void strcatLoad(char *str, float value);

uint32_t crc32(const uint8_t *message, size_t size);

uint8_t toBCD(uint8_t bin);
uint8_t fromBCD(uint8_t bcd);

float floorPrec(float a, float prec);
float ceilPrec(float a, float prec);
float roundPrec(float a, float prec);

bool greater(float a, float b, float prec);
bool greaterOrEqual(float a, float b, float prec);
bool less(float a, float b, float prec);
bool lessOrEqual(float a, float b, float prec);
bool equal(float a, float b, float prec);

bool between(float x, float a, float b, float prec);

float multiply(float a, float b, float prec);

bool isNaN(float x);

bool isDigit(char ch);
bool isUperCaseLetter(char ch);

void removeTrailingZerosFromFloat(char *str);

bool pointInsideRect(int xPoint, int yPoint, int xRect, int yRect, int wRect, int hRect);

void getParentDir(const char *path, char *parentDirPath);

}
}
} // namespace eez::psu::util
