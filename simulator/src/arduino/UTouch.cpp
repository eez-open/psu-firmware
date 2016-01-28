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
#include "UTouch.h"

namespace eez {
namespace psu {
namespace simulator {
namespace arduino {

bool UTouch::is_down;
int UTouch::x;
int UTouch::y;

UTouch::UTouch(byte tclk, byte tcs, byte tdin, byte dout, byte irq)
{
}

void UTouch::InitTouch(byte orientation) {
}

void UTouch::read() {
}

bool UTouch::dataAvailable() {
    return is_down;
}

int16_t	UTouch::getX() {
    return x;
}

int16_t	UTouch::getY() {
    return y;
}

void UTouch::setPrecision(byte precision) {
}

void UTouch::setData(bool is_down_, int x_, int y_) {
    is_down = is_down_;
    x = x_;
    y = y_;
}

}
}
}
} // namespace eez::psu::simulator::arduino;
