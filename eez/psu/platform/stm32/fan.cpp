/*
 * EEZ PSU Firmware
 * Copyright (C) 2016-present, Envox d.o.o.
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

#include "eez/psu/psu.h"
#include "eez/psu/temperature.h"
#include "eez/psu/scpi/psu.h"
#include "eez/psu/pid.h"

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12

#include "eez/psu/fan.h"

namespace eez {
namespace psu {
namespace fan {

////////////////////////////////////////////////////////////////////////////////

enum RpmMeasureState {
    RPM_MEASURE_STATE_START,
    RPM_MEASURE_STATE_MEASURE_T1,
    RPM_MEASURE_STATE_MEASURE_T2,
    RPM_MEASURE_STATE_MEASURED,
    RPM_MEASURE_STATE_FINISHED
};

////////////////////////////////////////////////////////////////////////////////

TestResult g_testResult = psu::TEST_FAILED;

bool g_fanManualControl = false;
int g_fanSpeedPWM = 0;

double g_Kp = FAN_PID_KP;
double g_Ki = FAN_PID_KI;
double g_Kd = FAN_PID_KD;
int g_POn = FAN_PID_POn;

volatile int g_rpm = 0;

////////////////////////////////////////////////////////////////////////////////

void init() {
}

void test_start() {
    if (OPTION_FAN) {
    }
}

bool test() {
    if (OPTION_FAN) {
        g_testResult = psu::TEST_OK;
    } else {
        g_testResult = psu::TEST_SKIPPED;
    }
    return true;
}

void tick(uint32_t tick_usec) {
}

void setPidTunings(double Kp, double Ki, double Kd, int POn) {
	g_Kp = Kp;
	g_Ki = Ki;
	g_Kd = Kd;
	g_POn = POn;
}

void setFanPwm(int pwm) {
}

}
}
} // namespace eez::psu::fan

#endif