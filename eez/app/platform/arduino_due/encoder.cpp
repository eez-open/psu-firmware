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

#include "eez/app/psu.h"

#if OPTION_ENCODER

#include "eez/mw/encoder.h"

#define CONF_ENCODER_ACCELERATION_DECREMENT_PER_MS 1
#define CONF_ENCODER_ACCELERATION_INCREMENT_FACTOR 2

#define CONF_ENCODER_SWITCH_DURATION_MIN 5000
#define CONF_ENCODER_SWITCH_DURATION_MAX 1000000

namespace eez {
namespace mw {
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
	{ 0x3 , 0x2, 0x1,  0x0 },{ 0x23, 0x0, 0x1,  0x0 },
{ 0x13, 0x2, 0x0,  0x0 },{ 0x3 , 0x5, 0x4,  0x0 },
{ 0x3 , 0x3, 0x4, 0x10 },{ 0x3 , 0x5, 0x3, 0x20 }
};
#else
// Use the full-step state table (emits a code at 00 only)
const uint8_t ttable[7][4] = {
	{ 0x0, 0x2, 0x4,  0x0 },{ 0x3, 0x0, 0x1, 0x10 },
{ 0x3, 0x2, 0x0,  0x0 },{ 0x3, 0x2, 0x1,  0x0 },
{ 0x6, 0x0, 0x4,  0x0 },{ 0x6, 0x5, 0x0, 0x10 },
{ 0x6, 0x5, 0x4,  0x0 },
};
#endif

bool g_accelerationEnabled = true;
int g_speedDown;
int g_speedUp;

volatile uint8_t g_rotationState = 0;
volatile int g_rotationCounter = 0;
volatile uint32_t g_rotationLastTime = 0;
volatile int g_acceleration;

volatile uint32_t g_switchLastTime = 0;
volatile int g_switchCounter = 0;

void abInterruptHandler() {
	uint8_t pinState = (digitalRead(ENC_B) << 1) | digitalRead(ENC_A);

	g_rotationState = ttable[g_rotationState & 0xf][pinState];
	uint8_t result = g_rotationState/* & 0x30*/;
	if (result == DIR_CW || result == DIR_CCW) {
		uint32_t time = micros();
		int32_t diff = time - g_rotationLastTime;

		if (g_accelerationEnabled) {
			g_acceleration += -CONF_ENCODER_ACCELERATION_DECREMENT_PER_MS * (diff / 1000) +
				CONF_ENCODER_ACCELERATION_INCREMENT_FACTOR * (result == DIR_CW ? g_speedUp : g_speedDown);

			if (g_acceleration < 0) g_acceleration = 0;
			if (g_acceleration > 99) g_acceleration = 99;
		} else {
			g_acceleration = 0;
		}

		int amount = 1 + g_acceleration;

		if (result == DIR_CW) {
			g_rotationCounter += amount;
		} else if (result == DIR_CCW) {
			g_rotationCounter -= amount;
		}

		g_rotationLastTime = time;
	}
}

void swInterruptHandler() {
	uint32_t time = micros();
	if (digitalRead(ENC_SW)) {
		if (g_switchLastTime) {
			int32_t diff = time - g_switchLastTime;
			if (diff > CONF_ENCODER_SWITCH_DURATION_MIN && diff < CONF_ENCODER_SWITCH_DURATION_MAX) {
				++g_switchCounter;
			}
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

void enableAcceleration(bool enable) {
	if (g_accelerationEnabled != enable) {
		g_accelerationEnabled = enable;
		g_acceleration = 0;
	}
}

void setMovingSpeed(uint8_t down, uint8_t up) {
	g_speedDown = down;
	g_speedUp = up;
}

}
}
} // namespace eez::mw::encoder

#endif
