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
#include "idle.h"
#include "rtc.h"
#include "event_queue.h"
#include "scpi_psu.h"

namespace eez {
namespace app {
namespace idle {

enum ActivityType {
    ACTIVITY_TYPE_NONE,
    ACTIVITY_TYPE_SCPI,
    ACTIVITY_TYPE_GUI,
    ACTIVITY_TYPE_ENCODER
};
static uint32_t g_idleTimeout[3] = {SCPI_IDLE_TIMEOUT, GUI_IDLE_TIMEOUT, ENCODER_IDLE_TIMEOUT};
static ActivityType g_lastActivityType = ACTIVITY_TYPE_NONE;
static uint32_t g_timeOfLastActivity;

#define MAX_GUI_OR_ENCODER_INACTIVITY_TIME 60 * 1000
static bool g_guiOrEncoderInactivityTimeMaxed = true;
static uint32_t g_timeOfLastGuiOrEncoderActivity;

void tick() {
    uint32_t tickCount = micros();

    if (g_lastActivityType != ACTIVITY_TYPE_NONE) {
        if (tickCount - g_timeOfLastActivity >= g_idleTimeout[g_lastActivityType - 1] * 1000 * 1000) {
            g_lastActivityType = ACTIVITY_TYPE_NONE;
        }
    }

    if (!g_guiOrEncoderInactivityTimeMaxed) {
        uint32_t guiOrEncoderInactivityPeriod = getGuiAndEncoderInactivityPeriod();
        if (guiOrEncoderInactivityPeriod >= MAX_GUI_OR_ENCODER_INACTIVITY_TIME) {
            g_guiOrEncoderInactivityTimeMaxed = true;
        }
    }
}

void noteScpiActivity() {
    g_lastActivityType = ACTIVITY_TYPE_SCPI;
    g_timeOfLastActivity = micros();
}

void noteGuiActivity() {
    g_lastActivityType = ACTIVITY_TYPE_GUI;
    g_timeOfLastActivity = micros();
    g_guiOrEncoderInactivityTimeMaxed = false;
    g_timeOfLastGuiOrEncoderActivity = g_timeOfLastActivity;
}

void noteEncoderActivity() {
    g_lastActivityType = ACTIVITY_TYPE_ENCODER;
    g_timeOfLastActivity = micros();
    g_guiOrEncoderInactivityTimeMaxed = false;
    g_timeOfLastGuiOrEncoderActivity = g_timeOfLastActivity;
}

uint32_t getGuiAndEncoderInactivityPeriod() {
    if (g_guiOrEncoderInactivityTimeMaxed) {
        return MAX_GUI_OR_ENCODER_INACTIVITY_TIME;
    } else {
        return (micros() - g_timeOfLastGuiOrEncoderActivity) / 1000;
    }
}

bool isIdle() {
    return g_lastActivityType == ACTIVITY_TYPE_NONE;
}

}
}
} // namespace eez::app::idle