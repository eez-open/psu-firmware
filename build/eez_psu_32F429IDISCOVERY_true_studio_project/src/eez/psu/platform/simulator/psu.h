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

#include "eez/psu/platform/simulator/arduino/Arduino.h"
#include "eez/psu/platform/simulator/arduino/SPI.h"

extern void eez_psu_init();

char *getConfFilePath(const char *file_name);

namespace eez {
namespace psu {
namespace simulator {

void init();
void tick();

void setTemperature(int sensor, float value);
float getTemperature(int sensor);

void exit();

}
}
} // namespace eez::psu::simulator
