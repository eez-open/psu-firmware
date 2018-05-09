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
 
#pragma once

#if CONF_DEBUG || CONF_DEBUG_LATEST

#ifdef EEZ_PSU_ARDUINO
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

namespace eez {
namespace psu {
/// Everything used for the debugging purposes.
namespace debug {

void tick(uint32_t tickCount);

void Trace(const char *format, ...);

}
}
} // namespace eez::psu::debug


#define DebugTrace(message) debug::Trace(message)
#define DebugTraceF(format, ...) debug::Trace(format, __VA_ARGS__)

#else // NO DEBUG

#define DebugTrace(...) 0
#define DebugTraceF(...) 0

#endif


#if CONF_DEBUG

namespace eez {
namespace psu {
namespace debug {

class DebugVariable {
public:
    DebugVariable(const char *name);

    const char *name();

    virtual void tick1secPeriod() = 0;
    virtual void tick10secPeriod() = 0;
    virtual void dump(char *buffer) = 0;

private:
    const char *m_name;
};

class DebugValueVariable : public DebugVariable {
public:
    DebugValueVariable(const char *name);
   
    int32_t get() { return m_value; }
    void set(int32_t value) { m_value = value; }
    
    void tick1secPeriod();
    void tick10secPeriod();
    void dump(char *buffer);

private:
    int32_t m_value;
};

class DebugDurationForPeriod {
public:
    DebugDurationForPeriod();
    
    void tick(uint32_t duration);
    void tickPeriod();

    void dump(char *buffer);

private:
    uint32_t m_lastTickCount;

    uint32_t m_min;
    uint32_t m_max;
    uint32_t m_total;
    uint32_t m_count;

    uint32_t m_minLast;
    uint32_t m_maxLast;
    uint32_t m_avgLast;

};

class DebugDurationVariable : public DebugVariable {
public:
    DebugDurationVariable(const char *name);
    
    void start();
    void finish();
    void tick(uint32_t tickCount);

    void tick1secPeriod();
    void tick10secPeriod();
    void dump(char *buffer);

private:
    uint32_t m_lastTickCount;

    DebugDurationForPeriod duration1sec;
    DebugDurationForPeriod duration10sec;

    uint32_t m_minTotal;
    uint32_t m_maxTotal;
};

class DebugCounterForPeriod {
public:
    DebugCounterForPeriod();
    
    void inc();
    void tickPeriod();
    void dump(char *buffer);

private:
    uint32_t m_counter;
    uint32_t m_lastCounter;
};

class DebugCounterVariable : public DebugVariable {
public:
    DebugCounterVariable(const char *name);
    
    void inc();

    void tick1secPeriod();
    void tick10secPeriod();
    void dump(char *buffer);

private:
    DebugCounterForPeriod counter1sec;
    DebugCounterForPeriod counter10sec;

    uint32_t m_totalCounter;
};

extern DebugValueVariable g_uDac[2];
extern DebugValueVariable g_uMon[2];
extern DebugValueVariable g_uMonDac[2];
extern DebugValueVariable g_iDac[2];
extern DebugValueVariable g_iMon[2];
extern DebugValueVariable g_iMonDac[2];
extern DebugValueVariable g_uTemp[3];

extern DebugDurationVariable g_mainLoopDuration;
#if CONF_DEBUG_VARIABLES
extern DebugDurationVariable g_listTickDuration;
#endif
extern DebugCounterVariable g_adcCounter;

extern bool g_debugWatchdog;

void dumpVariables(char *buffer);

}
}
} // namespace eez::psu::debug

#endif
