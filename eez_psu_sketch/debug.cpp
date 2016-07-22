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

uint16_t u_dac[2];
uint16_t i_dac[2];
int16_t u_mon[2];
int16_t u_mon_dac[2];
int16_t i_mon[2];
int16_t i_mon_dac[2];

static unsigned long previous_tick_count = 0;
unsigned long last_loop_duration = 0;
unsigned long max_loop_duration = 0;

static unsigned long avg_loop_duration_counter = 0;
static unsigned long avg_loop_duration_total = 0;
unsigned long avg_loop_duration = 0;

static unsigned long ioexp_previous_tick_count = 0;
static unsigned long current_ioexp_int_counter = 0;
unsigned long total_ioexp_int_counter = 0;
unsigned long last_ioexp_int_counter = 0;

unsigned long g_set_voltage_or_current_time_start = 0;

bool g_debug_watchdog = true;

void ioexpIntTick(unsigned long tick_usec) {
    ++current_ioexp_int_counter;
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
static bool insideInterruptHandler = false;

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
}

void tick(unsigned long tick_usec) {
#if CONF_DEBUG
    if (previous_tick_count != 0) {
        last_loop_duration = tick_usec - previous_tick_count;
        if (last_loop_duration > max_loop_duration) {
            max_loop_duration = last_loop_duration;
        }

        if (avg_loop_duration_counter++ < AVG_LOOP_DURATION_N) {
            avg_loop_duration_total += last_loop_duration;
        } else {
            avg_loop_duration = avg_loop_duration_total / AVG_LOOP_DURATION_N;
            avg_loop_duration_total = 0;
            avg_loop_duration_counter = 0;
        }
    }

    if (ioexp_previous_tick_count != 0) {
        unsigned long duration = tick_usec - ioexp_previous_tick_count;
        if (duration > 1000000) {
            noInterrupts();
            unsigned long int_counter = current_ioexp_int_counter;
            current_ioexp_int_counter = 0;
            interrupts();

            last_ioexp_int_counter = int_counter;
            total_ioexp_int_counter += int_counter;

            ioexp_previous_tick_count = tick_usec;
        }
    }
    else {
        ioexp_previous_tick_count = tick_usec;
    }

    previous_tick_count = tick_usec;
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

    if (insideInterruptHandler) {
        dumpTraceBufferOnNextTick = true;
    } else {
        DumpTraceBuffer();
    }
}

void interruptHandlerStarted() {
    insideInterruptHandler = true;
}

void interruptHandlerFinished() {
    insideInterruptHandler = false;
}

}
}
} // namespace eez::psu::debug

#endif // CONF_DEBUG || CONF_DEBUG_LATEST