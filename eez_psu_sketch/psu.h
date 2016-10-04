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

#if !defined(EEZ_PSU_ARDUINO) && !defined(EEZ_PSU_SIMULATOR)
#define EEZ_PSU_ARDUINO
#endif

#ifdef EEZ_PSU_SIMULATOR
#include "simulator_psu.h"
#endif

#include "Arduino.h"
#include "SPI.h"

#include "eez_psu_rev.h"

#ifdef EEZ_PSU_ARDUINO
#include "arduino_psu.h"
#endif

#include "conf.h"

#ifdef EEZ_PSU_SIMULATOR
#include "simulator_conf.h"
#endif

#include "eez_psu.h"

#include <scpi-parser.h>

#include "ontime.h"

/// Namespace for the everything from the EEZ.
namespace eez {
/// PSU firmware.
namespace psu {

enum TestResult {
    TEST_FAILED = 0,
    TEST_OK = 1,
    TEST_SKIPPED = 2,
    TEST_WARNING = 3
};

void boot();

bool powerUp();
void powerDown();
bool isPowerUp();
bool changePowerState(bool up);
void powerDownBySensor();

bool reset();

bool test();

void tick();

void setEsrBits(int bit_mask);
void setQuesBits(int bit_mask, bool on);

void generateError(int16_t error);

const char *getModelName();
const char *getCpuModel();
const char *getCpuType();
const char *getCpuEthernetType();

void enterTimeCriticalMode();
bool isTimeCriticalMode();
void leaveTimeCriticalMode();

enum MaxCurrentLimitCause {
    MAX_CURRENT_LIMIT_CAUSE_NONE,
    MAX_CURRENT_LIMIT_CAUSE_FAN,
    MAX_CURRENT_LIMIT_CAUSE_TEMPERATURE
};
bool isMaxCurrentLimited();
MaxCurrentLimitCause getMaxCurrentLimitCause();
void limitMaxCurrent(MaxCurrentLimitCause cause);
void unlimitMaxCurrent();

extern ontime::Counter g_powerOnTimeCounter;
extern bool g_insideInterruptHandler;

}
} // namespace eez::psu

#include "debug.h"
#include "util.h"
#include "channel.h"

#define CHANNEL_VALUE_PRECISION powf(10.0f, FLOAT_TO_STR_PREC)
