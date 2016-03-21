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

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#else
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#define PSTR(U) U
#define strcpy_P strcpy
#define strncpy_P strncpy
#define strcat_P strcat
#define sprintf_P sprintf
#define vsnprintf_P vsnprintf
#define strcmp_P strcmp
#define strncmp_P strncmp

extern void eez_psu_init();

#define interrupts() 0
#define noInterrupts() 0

#include "temp_sensor.h"

namespace eez {
namespace psu {
/// Firmware simulator.
namespace simulator {

void init();
void tick();

void setTemperature(temp_sensor::Type sensor, float value);
float getTemperature(temp_sensor::Type sensor);

char *getConfFilePath(char *file_name);

void exit();

}
}
} // namespace eez::psu::simulator
