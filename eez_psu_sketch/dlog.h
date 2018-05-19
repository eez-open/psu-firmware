/*
* EEZ PSU Firmware
* Copyright (C) 2018-present, Envox d.o.o.
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

#include "trigger.h"

namespace eez {
namespace app {
namespace dlog {

extern bool g_logVoltage[CH_NUM];
extern bool g_logCurrent[CH_NUM];
extern bool g_logPower[CH_NUM];

static const float PERIOD_MIN = 0.02f;
static const float PERIOD_MAX = 120.0f;
static const float PERIOD_DEFAULT = 0.02f;
extern float g_period;

static const float TIME_MIN = 1.0f;
static const float TIME_MAX = 86400000.0f;
static const float TIME_DEFAULT = 60.0f;
extern float g_time;

extern trigger::Source g_triggerSource;

extern double g_currentTime;

bool isIdle();
bool isInitiated();
int initiate(const char *filePath);
void triggerGenerated(bool startImmediatelly = true);
int startImmediately();
void abort();

void tick(uint32_t tick_usec);
void reset();

}
}
} // namespace eez::app::dlog
