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
#include "arduino_util.h"

namespace eez {
namespace psu {
namespace arduino_util {

uint8_t prog_read_byte(const uint8_t *p PROGMEM) {
    return pgm_read_byte_near(p);
}

uint16_t prog_read_word(const uint8_t *p PROGMEM) {
    return (((uint16_t)pgm_read_byte_near(p)) << 8) + pgm_read_byte_near(p + 1);
}

void prog_read_buffer(const uint8_t *src PROGMEM, uint8_t *dest, int length) {
    for (int i = 0; i < length; ++i) {
        *dest++ = pgm_read_byte_near(src);
        ++src;
    }
}

}
}
} // namespace eez::psu::arduino_util
