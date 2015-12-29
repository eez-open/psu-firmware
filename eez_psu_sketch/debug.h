/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
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

void Trace(char *format, ...);

}
}
} // namespace eez::psu::debug


#define DebugTrace(...) debug::Trace(__VA_ARGS__)

#else // NO DEBUG

#define DebugTrace(...) 0

#endif


#if CONF_DEBUG

#include <scpi-parser.h>

namespace eez {
namespace psu {
namespace debug {

extern uint16_t u_dac[2];
extern uint16_t i_dac[2];
extern int16_t u_mon[2];
extern int16_t u_mon_dac[2];
extern int16_t i_mon[2];
extern int16_t i_mon_dac[2];

extern unsigned long last_loop_duration;
extern unsigned long max_loop_duration;

extern unsigned long total_ioexp_int_counter;
extern unsigned long last_ioexp_int_counter;

void tick(unsigned long tick_usec);
void ioexpIntTick(unsigned long tick_usec);

}
}
} // namespace eez::psu::debug

#endif
