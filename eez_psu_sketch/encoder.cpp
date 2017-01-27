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

#if OPTION_ENCODER

#include "encoder.h"

#define CONF_ENCODER_SPEED_PT1_X 5.0f
#define CONF_ENCODER_SPEED_PT1_Y 100.0f

#define CONF_ENCODER_SPEED_PT2_X 250.0f
#define CONF_ENCODER_SPEED_PT2_Y 1.0f

namespace eez {
namespace psu {
namespace encoder {

// Half-step mode?
#define HALF_STEP

// define to enable weak pullups.
#define ENABLE_PULLUPS

#define DIR_CCW 0x10
#define DIR_CW 0x20

#ifdef HALF_STEP
// Use the half-step state table (emits a code at 00 and 11)
const uint8_t ttable[6][4] = {
  {0x3 , 0x2, 0x1,  0x0}, {0x23, 0x0, 0x1,  0x0},
  {0x13, 0x2, 0x0,  0x0}, {0x3 , 0x5, 0x4,  0x0},
  {0x3 , 0x3, 0x4, 0x10}, {0x3 , 0x5, 0x3, 0x20}
};
#else
// Use the full-step state table (emits a code at 00 only)
const uint8_t ttable[7][4] = {
  {0x0, 0x2, 0x4,  0x0}, {0x3, 0x0, 0x1, 0x10},
  {0x3, 0x2, 0x0,  0x0}, {0x3, 0x2, 0x1,  0x0},
  {0x6, 0x0, 0x4,  0x0}, {0x6, 0x5, 0x0, 0x10},
  {0x6, 0x5, 0x4,  0x0},
};
#endif

bool g_variableSpeed = true;
int g_speedDown;
int g_speedUp;
float g_speedMultiplier = 1.0f;

volatile uint8_t g_rotationState = 0;
volatile int g_rotationCounter = 0;
volatile unsigned long g_rotationLastTime = 0;

volatile unsigned long g_switchLastTime = 0;
volatile int g_switchCounter = 0;

void abInterruptHandler() {
    uint8_t pinState = (digitalRead(ENC_B) << 1) | digitalRead(ENC_A);

    g_rotationState = ttable[g_rotationState & 0xf][pinState];
    uint8_t result = g_rotationState/* & 0x30*/;
    if (result == DIR_CW || result == DIR_CCW) {
        unsigned long time = micros();
        unsigned long diff = time - g_rotationLastTime;

        int amount = 1;
        
        if (g_variableSpeed) {
            amount = (int) roundf(
                util::clamp(
                    g_speedMultiplier * 
                    (
                        result == DIR_CW ?

                        util::remap(diff / 1000.0f, 
                            CONF_ENCODER_SPEED_PT1_X, CONF_ENCODER_SPEED_PT1_Y * g_speedUp / MAX_MOVING_SPEED,
                            CONF_ENCODER_SPEED_PT2_X, CONF_ENCODER_SPEED_PT2_Y) :

                        util::remap(diff / 1000.0f, 
                            CONF_ENCODER_SPEED_PT1_X, CONF_ENCODER_SPEED_PT1_Y * g_speedDown / MAX_MOVING_SPEED,
                            CONF_ENCODER_SPEED_PT2_X, CONF_ENCODER_SPEED_PT2_Y)
                    ),
                    CONF_ENCODER_SPEED_PT2_Y,
                    CONF_ENCODER_SPEED_PT1_Y
                )
            );
        }
        
        if (result == DIR_CW) {
            g_rotationCounter += amount;
        } else if (result == DIR_CCW) {
            g_rotationCounter -= amount;
        }

        g_rotationLastTime = time;
    }
}

void swInterruptHandler() {
    unsigned long time = micros();
    if (digitalRead(ENC_SW)) {
        unsigned long diff = time - g_switchLastTime;
        if (diff > 10000 && diff < 250000) {
            ++g_switchCounter;
        }
    } else {
        g_switchLastTime = time;
    }
}

void init() {
#ifdef ENABLE_PULLUPS
    digitalWrite(ENC_A, HIGH);
    digitalWrite(ENC_B, HIGH);

    digitalWrite(ENC_SW, HIGH);
#endif

    attachInterrupt(digitalPinToInterrupt(ENC_A), abInterruptHandler, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENC_B), abInterruptHandler, CHANGE);

    attachInterrupt(digitalPinToInterrupt(ENC_SW), swInterruptHandler, CHANGE);
}

void read(int &counter, bool &clicked) {
    noInterrupts();

    counter = g_rotationCounter;
    g_rotationCounter = 0;

    clicked = g_switchCounter > 0;
    g_switchCounter = 0;

    interrupts();
}

void enableVariableSpeed(bool enable) {
    g_variableSpeed = enable;
}

void setMovingSpeed(uint8_t down, uint8_t up) {
    g_speedDown = down;
    g_speedUp = up;
}

void setMovingSpeedMultiplier(float speedMultiplier) {
    g_speedMultiplier = speedMultiplier;
}

#ifdef EEZ_PSU_SIMULATOR
void write(int counter, bool clicked) {
    if (counter != 0) {
        g_rotationCounter += counter;
    }

    if (clicked) {
        ++g_switchCounter;
    }
}
#endif

}
}
} // namespace eez::psu::encoder

#endif
