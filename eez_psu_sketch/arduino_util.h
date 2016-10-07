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

namespace eez {
namespace psu {
/// Utility code (PROGMEM access) for Arduino platform
namespace arduino_util {

uint8_t prog_read_byte(const uint8_t *p PROGMEM);
uint16_t prog_read_word(const uint8_t *p PROGMEM);
void prog_read_buffer(const uint8_t *src PROGMEM, uint8_t *dest, int length);

}
}
} // namespace eez::psu::arduino_util
