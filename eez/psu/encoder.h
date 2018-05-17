/*
 * EEZ PSU Firmware
 * Copyright (C) 2017-present, Envox d.o.o.
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
namespace encoder {

static const uint8_t MAX_MOVING_SPEED = 10;
static const uint8_t MIN_MOVING_SPEED = 1;
static const uint8_t DEFAULT_MOVING_DOWN_SPEED = 8;
static const uint8_t DEFAULT_MOVING_UP_SPEED = 6;

void init();
void read(int &counter, bool &clicked);

void enableAcceleration(bool enable);
void setMovingSpeed(uint8_t down, uint8_t up);

}
}
} // namespace eez::psu::encoder
