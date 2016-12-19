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
#include "datetime.h"

#if CONF_DEBUG

#define AVG_LOOP_DURATION_N 10

namespace eez {
namespace psu {
namespace debug {

uint16_t uDac[2];
uint16_t iDac[2];
int16_t uMon[2];
int16_t uMonDac[2];
int16_t iMon[2];
int16_t iMonDac[2];

static unsigned long previousTickCount = 0;
unsigned long lastLoopDuration = 0;
unsigned long maxLoopDuration = 0;

static unsigned long avgLoopDurationCounter = 0;
static unsigned long avgLoopDurationTotal = 0;
unsigned long avgLoopDuration = 0;

static unsigned long previousAdcReadCount = 0;
static unsigned long currentAdcReadCounter = 0;
unsigned long totalAdcReadCounter = 0;
unsigned long lastAdcReadCounter = 0;

unsigned long g_setVoltageOrCurrentTimeStart = 0;

bool g_debugWatchdog = true;

void adcReadTick(unsigned long tick_usec) {
    ++currentAdcReadCounter;
}

}
}
} // namespace eez::psu::debug

#endif // CONF_DEBUG


#if CONF_DEBUG || CONF_DEBUG_LATEST

namespace eez {
namespace psu {
namespace debug {

static char traceBuffer[256];
static bool dumpTraceBufferOnNextTick = false;

void DumpTraceBuffer() {
    Serial.print("**TRACE");
    
    char datetime_buffer[20] = { 0 };
    if (datetime::getDateTimeAsString(datetime_buffer)) {
        Serial.print(" [");
        Serial.print(datetime_buffer);
        Serial.print("]: ");
    } else {
        Serial.print(": ");
    }

    Serial.println(traceBuffer);

    Serial.flush();
}

void tick(unsigned long tick_usec) {
#if CONF_DEBUG
    if (previousTickCount != 0) {
        lastLoopDuration = tick_usec - previousTickCount;
        if (lastLoopDuration > maxLoopDuration) {
            maxLoopDuration = lastLoopDuration;
        }

        if (avgLoopDurationCounter++ < AVG_LOOP_DURATION_N) {
            avgLoopDurationTotal += lastLoopDuration;
        } else {
            avgLoopDuration = avgLoopDurationTotal / AVG_LOOP_DURATION_N;
            avgLoopDurationTotal = 0;
            avgLoopDurationCounter = 0;
        }
    }

    if (previousAdcReadCount != 0) {
        unsigned long duration = tick_usec - previousAdcReadCount;
        if (duration > 1000000) {
            noInterrupts();
            unsigned long int_counter = currentAdcReadCounter;
            currentAdcReadCounter = 0;
            interrupts();

            lastAdcReadCounter = int_counter;
            totalAdcReadCounter += int_counter;

            previousAdcReadCount = tick_usec;
        }
    }
    else {
        previousAdcReadCount = tick_usec;
    }

    previousTickCount = tick_usec;
#endif

    if (dumpTraceBufferOnNextTick) {
        DumpTraceBuffer();
        dumpTraceBufferOnNextTick = false;
    }
}

void Trace(const char *format, ...) {
	if (dumpTraceBufferOnNextTick) return;

    va_list args;
    va_start(args, format);
    vsnprintf_P(traceBuffer, sizeof(traceBuffer), format, args);

    if (g_insideInterruptHandler) {
        dumpTraceBufferOnNextTick = true;
    } else {
        DumpTraceBuffer();
    }
}

}
}
} // namespace eez::psu::debug

#endif // CONF_DEBUG || CONF_DEBUG_LATEST