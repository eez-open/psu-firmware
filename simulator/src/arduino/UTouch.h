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

#define PORTRAIT			0
#define LANDSCAPE			1

#define PREC_LOW			1
#define PREC_MEDIUM			2
#define PREC_HI				3
#define PREC_EXTREME		4

namespace eez {
namespace psu {
namespace simulator {
namespace arduino {

typedef uint16_t word;
typedef uint8_t regtype;
typedef uint8_t regsize;

class UTouch {
public:
    UTouch(byte tclk, byte tcs, byte tdin, byte dout, byte irq);

    void	InitTouch(byte orientation = LANDSCAPE);
    void	read();
    bool	dataAvailable();
    int16_t	getX();
    int16_t	getY();
    void	setPrecision(byte precision);

    static void setData(bool is_down, int x_, int y_);

private:
    static bool is_down;
    static int x;
    static int y;
};

}
}
}
} // namespace eez::psu::simulator::arduino;

using namespace eez::psu::simulator::arduino;