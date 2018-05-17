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

#include "eez/psu/psu.h"

#if OPTION_ENCODER

#include "eez/psu/encoder.h"

namespace eez {
namespace psu {
namespace encoder {

int g_rotationCounter = 0;
int g_switchCounter = 0;

void init() {
}

void read(int &counter, bool &clicked) {
	counter = g_rotationCounter;
	g_rotationCounter = 0;

	clicked = g_switchCounter > 0;
	g_switchCounter = 0;
}

void enableAcceleration(bool enable) {
}

void setMovingSpeed(uint8_t down, uint8_t up) {
}

void write(int counter, bool clicked) {
    if (counter != 0) {
        g_rotationCounter += counter;
    }

    if (clicked) {
        ++g_switchCounter;
    }
}

}
}
} // namespace eez::psu::encoder

#endif
