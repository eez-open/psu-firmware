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
 
#pragma once

namespace eez {
namespace psu {

/// Various utility functions.
namespace util {

float remap(float x, float x1, float y1, float x2, float y2);
float clamp(float x, float min, float max);

void strcatInt(char *str, int value);
void strcatFloat(char *str, float value);
void strcatVoltage(char *str, float value);
void strcatCurrent(char *str, float value);
void strcatDuration(char *str, float value);
void strcatLoad(char *str, float value);

uint32_t crc32(const uint8_t *message, size_t size);

uint8_t toBCD(uint8_t bin);
uint8_t fromBCD(uint8_t bcd);

}
}
} // namespace eez::psu::util
