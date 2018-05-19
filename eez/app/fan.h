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
 
#pragma once

namespace eez {
namespace app {
namespace fan {

extern TestResult g_testResult;
extern volatile int g_rpm;

void init();
void test_start();
bool test();
void tick(uint32_t tick_usec);

extern bool g_fanManualControl;
extern int g_fanSpeedPWM;

extern double g_Kp;
extern double g_Ki;
extern double g_Kd;
extern int g_POn;

void setPidTunings(double Kp, double Ki, double Kd, int POn);

void setFanPwm(int pwm);

}
}
} // namespace eez::app::fan
