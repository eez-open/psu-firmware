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
 
#include "psu.h"
#include "datetime.h"

#if CONF_DEBUG

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

static unsigned long ioexp_previous_tick_count = 0;
static unsigned long current_ioexp_int_counter = 0;
unsigned long total_ioexp_int_counter = 0;
unsigned long last_ioexp_int_counter = 0;

void tick(unsigned long tick_usec) {
    if (previous_tick_count != 0) {
        last_loop_duration = tick_usec - previous_tick_count;
        if (last_loop_duration > max_loop_duration) {
            max_loop_duration = last_loop_duration;
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
}

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

void Trace(char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);

    Serial.print("**TRACE");
    
    char datetime_buffer[20] = { 0 };
    if (datetime::getDateTimeAsString(datetime_buffer)) {
        Serial.print(" [");
        Serial.print(datetime_buffer);
        Serial.print("]: ");
    } else {
        Serial.print(": ");
    }

    Serial.println(buffer);
}

}
}
} // namespace eez::psu::debug

#endif // CONF_DEBUG || CONF_DEBUG_LATEST