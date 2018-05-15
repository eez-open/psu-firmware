/*
* EEZ Middleware
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

#include "mw_unit.h"

#define clear_bit(reg, bitmask) *reg &= ~bitmask
#define set_bit(reg, bitmask) *reg |= bitmask
#define util_swap(type, i, j) {type t = i; i = j; j = t;}
#define MIN(a,b) ((a)<(b)?(a):(b))
#define MAX(a,b) ((a)>(b)?(a):(b))

#define PATH_SEPARATOR "/"

namespace eez {
namespace mw {

float remap(float x, float x1, float y1, float x2, float y2);
float remapQuad(float x, float x1, float y1, float x2, float y2);
float remapCubic(float x, float x1, float y1, float x2, float y2);
float remapExp(float x, float x1, float y1, float x2, float y2);
float clamp(float x, float min, float max);

void strcatInt(char *str, int value);
void strcatInt32(char *str, int32_t value);
void strcatUInt32(char *str, uint32_t value);
void strcatFloat(char *str, float value, int numSignificantDecimalDigits);

uint32_t crc32(const uint8_t *message, size_t size);

uint8_t toBCD(uint8_t bin);
uint8_t fromBCD(uint8_t bcd);

extern float g_precisions[];

inline float getPrecisionFromNumSignificantDecimalDigits(int numSignificantDecimalDigits) {
	return g_precisions[numSignificantDecimalDigits];
}

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
bool isHexDigit(char ch);
bool isUperCaseLetter(char ch);

char toHexDigit(int num);
int fromHexDigit(char ch);

void removeTrailingZerosFromFloat(char *str);

bool pointInsideRect(int xPoint, int yPoint, int xRect, int yRect, int wRect, int hRect);

void getParentDir(const char *path, char *parentDirPath);

bool parseMacAddress(const char *macAddressStr, size_t macAddressStrLength, uint8_t *macAddress);

int getIpAddressPartA(uint32_t ipAddress);
void setIpAddressPartA(uint32_t *ipAddress, uint8_t value);

int getIpAddressPartB(uint32_t ipAddress);
void setIpAddressPartB(uint32_t *ipAddress, uint8_t value);

int getIpAddressPartC(uint32_t ipAddress);
void setIpAddressPartC(uint32_t *ipAddress, uint8_t value);

int getIpAddressPartD(uint32_t ipAddress);
void setIpAddressPartD(uint32_t *ipAddress, uint8_t value);

void ipAddressToArray(uint32_t ipAddress, uint8_t *ipAddressArray);
uint32_t arrayToIpAddress(uint8_t *ipAddressArray);

uint32_t getIpAddress(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

bool parseIpAddress(const char *ipAddressStr, size_t ipAddressStrLength, uint32_t &ipAddress);
void ipAddressToString(uint32_t ipAddress, char *ipAddressStr);

void macAddressToString(uint8_t *macAddress, char *macAddressStr);

void formatTimeZone(int16_t timeZone, char *text, int count);
bool parseTimeZone(const char *timeZoneStr, size_t timeZoneLength, int16_t &timeZone);

void replaceCharacter(char *str, char ch, char repl);

bool endsWith(const char *str, const char *suffix);

}
} // namespace eez::mw
