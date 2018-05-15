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
#include "buzzer.h"

#if defined(EEZ_PLATFORM_ARDUINO_DUE)
/*
Tone generator
v1  use timer, and toggle any digital pin in ISR
funky duration from arduino version
TODO use FindMckDivisor?
timer selected will preclude using associated pins for PWM etc.
could also do timer/pwm hardware toggle where caller controls duration
*/


// timers TC0 TC1 TC2   channels 0-2 ids 0-2  3-5  6-8     AB 0 1
// use TC1 channel 0
#define TONE_TIMER TC1
#define TONE_CHNL 0
#define TONE_IRQ TC3_IRQn

// TIMER_CLOCK4   84MHz/128 with 16 bit counter give 10 Hz to 656KHz
//  piano 27Hz to 4KHz

static uint8_t pinEnabled[PINS_COUNT];
static uint8_t TCChanEnabled = 0;
static boolean pin_state = false;
static Tc *chTC = TONE_TIMER;
static uint32_t chNo = TONE_CHNL;

volatile static int32_t toggle_count;
static uint32_t tone_pin;

// frequency (in hertz) and duration (in milliseconds).

void tone(uint32_t ulPin, uint32_t frequency, int32_t duration)
{
    const uint32_t rc = VARIANT_MCK / 256 / frequency;
    tone_pin = ulPin;
    toggle_count = 0;  // strange  wipe out previous duration
    if (duration > 0) toggle_count = 2 * frequency * duration / 1000;
    else toggle_count = -1;

    if (!TCChanEnabled) {
        pmc_set_writeprotect(false);
        pmc_enable_periph_clk((uint32_t)TONE_IRQ);
        TC_Configure(chTC, chNo,
            TC_CMR_TCCLKS_TIMER_CLOCK4 |
            TC_CMR_WAVE |         // Waveform mode
            TC_CMR_WAVSEL_UP_RC); // Counter running up and reset when equals to RC

        chTC->TC_CHANNEL[chNo].TC_IER = TC_IER_CPCS;  // RC compare interrupt
        chTC->TC_CHANNEL[chNo].TC_IDR = ~TC_IER_CPCS;
        NVIC_EnableIRQ(TONE_IRQ);
        TCChanEnabled = 1;
    }
    if (!pinEnabled[ulPin]) {
        pinMode(ulPin, OUTPUT);
        pinEnabled[ulPin] = 1;
    }
    TC_Stop(chTC, chNo);
    TC_SetRC(chTC, chNo, rc);    // set frequency
    TC_Start(chTC, chNo);
}

void noTone(uint32_t ulPin)
{
    TC_Stop(chTC, chNo);  // stop timer
    digitalWrite(ulPin, LOW);  // no signal on pin
}

// timer ISR  TC1 ch 0
void TC3_Handler(void) {
    TC_GetStatus(TC1, 0);
    if (toggle_count != 0) {
        // toggle pin  TODO  better
        digitalWrite(tone_pin, pin_state = !pin_state);
        if (toggle_count > 0) toggle_count--;
    }
    else {
        noTone(tone_pin);
    }
}
#endif

namespace eez {
namespace psu {
namespace buzzer {

void tone(uint32_t frequency, int32_t duration) {
    ::tone(BUZZER, frequency, duration);
}

}
}
} // namespace eez::psu::buzzer