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
#include "serial_psu.h"

#ifndef EEZ_PLATFORM_SIMULATOR
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

extern char _end;
extern "C" char *sbrk(int i);
char *ramstart = (char *)0x20070000;
char *ramend = (char *)0x20088000;
#endif

#if OPTION_SD_CARD
#include "sd_card.h"
#endif

#if CONF_DEBUG

#define AVG_LOOP_DURATION_N 100

namespace eez {
namespace psu {
namespace debug {

DebugValueVariable g_uDac[2]    = { DebugValueVariable("CH1 U_DAC"),     DebugValueVariable("CH2 U_DAC")     };
DebugValueVariable g_uMon[2]    = { DebugValueVariable("CH1 U_MON"),     DebugValueVariable("CH2 U_MON")     };
DebugValueVariable g_uMonDac[2] = { DebugValueVariable("CH1 U_MON_DAC"), DebugValueVariable("CH2 U_MON_DAC") };
DebugValueVariable g_iDac[2]    = { DebugValueVariable("CH1 I_DAC"),     DebugValueVariable("CH2 I_DAC")     };
DebugValueVariable g_iMon[2]    = { DebugValueVariable("CH1 I_MON"),     DebugValueVariable("CH2 I_MON")     };
DebugValueVariable g_iMonDac[2] = { DebugValueVariable("CH1 I_MON_DAC"), DebugValueVariable("CH2 I_MON_DAC") };
DebugValueVariable g_uTemp[3] = {
    DebugValueVariable("AUX TEMP"),
    DebugValueVariable("CH1 TEMP"),
    DebugValueVariable("CH2 TEMP")
};

DebugDurationVariable g_mainLoopDuration("MAIN_LOOP_DURATION");
#if CONF_DEBUG_VARIABLES
DebugDurationVariable g_listTickDuration("LIST_TICK_DURATION");
#endif
DebugCounterVariable g_adcCounter("ADC_COUNTER");

DebugVariable *g_variables[] = {
    &g_uDac[0],    &g_uDac[1],
    &g_uMon[0],    &g_uMon[1],
    &g_uMonDac[0], &g_uMonDac[1],
    &g_iDac[0],    &g_iDac[1],
    &g_iMon[0],    &g_iMon[1],
    &g_iMonDac[0], &g_iMonDac[1],

    &g_uTemp[0],
    &g_uTemp[1],
    &g_uTemp[2],

    &g_mainLoopDuration,
#if CONF_DEBUG_VARIABLES
    &g_listTickDuration,
#endif
    &g_adcCounter
};

bool g_debugWatchdog = true;

static uint32_t g_previousTickCount1sec;
static uint32_t g_previousTickCount10sec;

void dumpVariables(char *buffer) {
    buffer[0] = 0;

    for (unsigned i = 0; i < sizeof(g_variables) / sizeof(DebugVariable *); ++i) {
        strcat(buffer, g_variables[i]->name());
        strcat(buffer, " = ");
        g_variables[i]->dump(buffer);
        strcat(buffer, "\n");
	}

#ifndef EEZ_PLATFORM_SIMULATOR
	psu::criticalTick(-1);

	char *heapend = sbrk(0);
	register char * stack_ptr asm("sp");
	struct mallinfo mi = mallinfo();
	sprintf(buffer + strlen(buffer), "Dynamic ram used: %d\n", mi.uordblks);
	sprintf(buffer + strlen(buffer), "Program static ram used %d\n", &_end - ramstart);
	sprintf(buffer + strlen(buffer), "Stack ram used %d\n", ramend - stack_ptr);
	sprintf(buffer + strlen(buffer), "My guess at free mem: %d\n", stack_ptr - heapend + mi.fordblks);
#endif

#if OPTION_SD_CARD
	psu::criticalTick(-1);
	sd_card::dumpInfo(buffer);
#endif
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
    if (serial::g_testResult == TEST_OK) {
        SERIAL_PORT.print("**TRACE");

        char datetime_buffer[20] = { 0 };
        if (datetime::getDateTimeAsString(datetime_buffer)) {
            SERIAL_PORT.print(" [");
            SERIAL_PORT.print(datetime_buffer);
            SERIAL_PORT.print("]: ");
        } else {
            SERIAL_PORT.print(": ");
        }

        size_t len = strlen(traceBuffer);
        if (len > 64) {
            // dump trace buffer using chunks of 64 bytes
            const size_t CHUNK_SIZE = 64;
            const char *end = traceBuffer + len;
            const char *p = traceBuffer;
            const char *q = traceBuffer + CHUNK_SIZE;
            do {
                SERIAL_PORT.write(p, CHUNK_SIZE);
                p = q;
                q += CHUNK_SIZE;
            } while (q < end);
            SERIAL_PORT.println(p);
        } else {
            SERIAL_PORT.println(traceBuffer);
        }

        SERIAL_PORT.flush();
    }
}

void tick(uint32_t tickCount) {
#if CONF_DEBUG
    debug::g_mainLoopDuration.tick(tickCount);

    if (g_previousTickCount1sec != 0) {
        if (tickCount - g_previousTickCount1sec >= 1000000L) {
            for (unsigned i = 0; i < sizeof(g_variables) / sizeof(DebugVariable *); ++i) {
                g_variables[i]->tick1secPeriod();
            }
            g_previousTickCount1sec = tickCount;
        }
    } else {
        g_previousTickCount1sec = tickCount;
    }

    if (g_previousTickCount10sec != 0) {
        if (tickCount - g_previousTickCount10sec >= 10 * 1000000L) {
            for (unsigned i = 0; i < sizeof(g_variables) / sizeof(DebugVariable *); ++i) {
                g_variables[i]->tick10secPeriod();
            }
            g_previousTickCount10sec = tickCount;
        }
    } else {
        g_previousTickCount10sec = tickCount;
    }
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
    vsnprintf(traceBuffer, sizeof(traceBuffer), format, args);

    if (g_insideInterruptHandler) {
        dumpTraceBufferOnNextTick = true;
    } else {
        DumpTraceBuffer();
    }
}

////////////////////////////////////////////////////////////////////////////////

DebugVariable::DebugVariable(const char *name) : m_name(name) {
}

const char *DebugVariable::name() {
    return m_name;
}

////////////////////////////////////////////////////////////////////////////////

DebugValueVariable::DebugValueVariable(const char *name) : DebugVariable(name) {
}

void DebugValueVariable::tick1secPeriod() {
}

void DebugValueVariable::tick10secPeriod() {
}

void DebugValueVariable::dump(char *buffer) {
    strcatInt32(buffer, m_value);
}

////////////////////////////////////////////////////////////////////////////////

DebugDurationForPeriod::DebugDurationForPeriod()
    : m_min(4294967295UL)
    , m_max(0)
    , m_total(0)
    , m_count(0)
{
}

void DebugDurationForPeriod::tick(uint32_t duration) {
    if (duration < m_min) {
        m_min = duration;
    }

    if (duration > m_max) {
        m_max = duration;
    }

    m_total += duration;
    ++m_count;
}

void DebugDurationForPeriod::tickPeriod() {
    if (m_count > 0) {
        m_minLast = m_min;
        m_maxLast = m_max;
        m_avgLast = m_total / m_count;
    } else {
        m_minLast = 0;
        m_maxLast = 0;
        m_avgLast = 0;
    }

    m_min = 4294967295UL;
    m_max = 0;
    m_total = 0;
    m_count = 0;
}


void DebugDurationForPeriod::dump(char *buffer) {
    strcatUInt32(buffer, m_minLast);
    strcat(buffer, " ");
    strcatUInt32(buffer, m_avgLast);
    strcat(buffer, " ");
    strcatUInt32(buffer, m_maxLast);
}

////////////////////////////////////////////////////////////////////////////////

DebugDurationVariable::DebugDurationVariable(const char *name)
    : DebugVariable(name)
    , m_minTotal(4294967295UL)
    , m_maxTotal(0)
{
}

void DebugDurationVariable::start() {
    m_lastTickCount = micros();
}

void DebugDurationVariable::finish() {
    tick(micros());
}

void DebugDurationVariable::tick(uint32_t tickCount) {
    uint32_t duration = tickCount - m_lastTickCount;

    duration1sec.tick(duration);
    duration10sec.tick(duration);

    if (duration < m_minTotal) {
        m_minTotal = duration;
    }

    if (duration > m_maxTotal) {
        m_maxTotal = duration;
    }

    m_lastTickCount = tickCount;
}

void DebugDurationVariable::tick1secPeriod() {
    duration1sec.tickPeriod();
}

void DebugDurationVariable::tick10secPeriod() {
    duration10sec.tickPeriod();
}

void DebugDurationVariable::dump(char *buffer) {
    duration1sec.dump(buffer);

    strcat(buffer, " / ");

    duration10sec.dump(buffer);

    strcat(buffer, " / ");

    strcatUInt32(buffer, m_minTotal);
    strcat(buffer, " ");
    strcatUInt32(buffer, m_maxTotal);
}

////////////////////////////////////////////////////////////////////////////////

DebugCounterForPeriod::DebugCounterForPeriod() : m_counter(0) {
}

void DebugCounterForPeriod::inc() {
    ++m_counter;
}

void DebugCounterForPeriod::tickPeriod() {
    noInterrupts();
    m_lastCounter = m_counter;
    m_counter = 0;
    interrupts();
}

void DebugCounterForPeriod::dump(char *buffer) {
    strcatUInt32(buffer, m_lastCounter);
}

////////////////////////////////////////////////////////////////////////////////

DebugCounterVariable::DebugCounterVariable(const char *name) : DebugVariable(name)
{
}

void DebugCounterVariable::inc() {
    counter1sec.inc();
    counter10sec.inc();
    ++m_totalCounter;
}

void DebugCounterVariable::tick1secPeriod() {
    counter1sec.tickPeriod();
}

void DebugCounterVariable::tick10secPeriod() {
    counter10sec.tickPeriod();
}

void DebugCounterVariable::dump(char *buffer) {
    counter1sec.dump(buffer);

    strcat(buffer, " / ");

    counter10sec.dump(buffer);

    strcat(buffer, " / ");

    strcatUInt32(buffer, m_totalCounter);
}

}
}
} // namespace eez::psu::debug

#endif // CONF_DEBUG || CONF_DEBUG_LATEST
