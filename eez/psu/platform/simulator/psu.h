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

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include <queue>

////////////////////////////////////////////////////////////////////////////////

static const uint8_t EXT_TRIG = 0;
static const uint8_t DOUT = 1;
static const uint8_t DOUT2 = 2;
static const uint8_t NUM_IO_PINS = 3;

static const uint8_t TEMP_ANALOG = 54;
static const uint8_t NTC1 = 59;
static const uint8_t NTC2 = 60;

#define ISOLATOR1_EN 0
#define IO_EXPANDER1 0
#define CONVEND1 0
#define ADC1_SELECT 0
#define DAC1_SELECT 0

#define ISOLATOR2_EN 1
#define IO_EXPANDER2 1
#define CONVEND2 1
#define ADC2_SELECT 1
#define DAC2_SELECT 1

////////////////////////////////////////////////////////////////////////////////

uint32_t millis();
uint32_t micros();
void delay(uint32_t millis);
void delayMicroseconds(uint32_t microseconds);

////////////////////////////////////////////////////////////////////////////////

/// Bare minimum implementation of the Arduino IPAddress class
class IPAddress {
	friend class UARTClass;

private:
	union {
		uint8_t bytes[4];  // IPv4 address
		uint32_t dword;
	} _address;

public:
	operator uint32_t() const { return _address.dword; };
};

////////////////////////////////////////////////////////////////////////////////

#include "eez/psu/platform/simulator/serial.h"

////////////////////////////////////////////////////////////////////////////////

char *getConfFilePath(const char *file_name);

////////////////////////////////////////////////////////////////////////////////

namespace eez {
namespace psu {
namespace simulator {

void init();
void tick();

void setTemperature(int sensor, float value);
float getTemperature(int sensor);

bool getPwrgood(int pin);
void setPwrgood(int pin, bool on);

bool getRPol(int pin);
void setRPol(int pin, bool on);

bool getCV(int pin);
void setCV(int pin, bool on);

bool getCC(int pin);
void setCC(int pin, bool on);

void exit();

}
}
} // namespace eez::psu::simulator
