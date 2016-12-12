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

void tick(unsigned long tick_usec);

void Trace(const char *format, ...);

}
}
} // namespace eez::psu::debug


#define DebugTrace(message) debug::Trace(PSTR(message))
#define DebugTraceF(format, ...) debug::Trace(PSTR(format), __VA_ARGS__)

#else // NO DEBUG

#define DebugTrace(...) 0
#define DebugTraceF(...) 0

#endif


#if CONF_DEBUG

namespace eez {
namespace psu {
namespace debug {

extern uint16_t uDac[2];
extern uint16_t iDac[2];
extern int16_t uMon[2];
extern int16_t uMonDac[2];
extern int16_t iMon[2];
extern int16_t iMonDac[2];

extern unsigned long g_setVoltageOrCurrentTimeStart;

extern unsigned long lastLoopDuration;
extern unsigned long maxLoopDuration;
extern unsigned long avgLoopDuration;

extern unsigned long totalAdcReadCounter;
extern unsigned long lastAdcReadCounter;

void adcReadTick(unsigned long tick_usec);

extern bool g_debugWatchdog;

}
}
} // namespace eez::psu::debug

#endif
