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

#include "psu.h"
#include "UTFT.h"
#include "arduino_util.h"

namespace eez {
namespace psu {
namespace simulator {
namespace arduino {

UTFT::UTFT(byte , int , int , int , int , int) {
	P_CS = &CS;
    disp_x_size = 239;
    disp_y_size = 319;
    buffer = new word[getDisplayXSize() * getDisplayYSize()];
}

UTFT::~UTFT() {
    delete [] buffer;
}

void UTFT::InitLCD(byte orientation) {
    orient = orientation;
    clrXY();
}

void UTFT::setColor(byte r, byte g, byte b) {
    // rrrrrggggggbbbbb
	fch=((r&248)|g>>5);
	fcl=((g&28)<<3|b>>3);
}

void UTFT::setColor(word color) {
	fch=byte(color>>8);
	fcl=byte(color & 0xFF);
}

word UTFT::getColor() {
	return (fch<<8) | fcl;
}

void UTFT::setBackColor(byte r, byte g, byte b) {
	bch=((r&248)|g>>5);
	bcl=((g&28)<<3|b>>3);
}

void UTFT::setBackColor(word color) {
	bch = byte(color>>8);
	bcl = byte(color & 0xFF);
}

word UTFT::getBackColor() {
	return (bch<<8) | bcl;
}

void UTFT::setContrast(char c) {
}

int UTFT::getDisplayXSize() {
    return orient == PORTRAIT ? disp_x_size + 1 : disp_y_size + 1;
}

int UTFT::getDisplayYSize() {
    return orient == PORTRAIT ? disp_y_size + 1 : disp_x_size + 1;
}

void UTFT::setBrightness(byte br) {
}

void UTFT::setPixel(word color) {
    if (orient == PORTRAIT) {
        if (x >= 0 && x < getDisplayXSize() && y >= 0 && y < getDisplayYSize()) {
            *(buffer + y * getDisplayXSize() + x) = color;
        }
        if (++x > x2) {
            x = x1;
            if (++y > y2) {
                y = y1;
            }
        }
    } else {
        if (x >= 0 && x < getDisplayXSize() && y >= 0 && y < getDisplayYSize()) {
            *(buffer + (disp_y_size - x) * getDisplayYSize() + y) = color;
        }
        if (--x < x1) {
            x = x2;
        }
    }
}

void UTFT::setXY(word x1_, word y1_, word x2_, word y2_) {
    x1 = x1_;
    y1 = y1_;
    x2 = x2_;
    y2 = y2_;

    if (orient == PORTRAIT) {
        x = x1;
        y = y1;
    } else {
        x = x2;
        y = y1;
    }
}

void UTFT::clrXY() {
	if (orient == PORTRAIT) {
		setXY(0, 0, disp_x_size, disp_y_size);
    }
    else {
		setXY(0, 0, disp_y_size, disp_x_size);
    }
}

void UTFT::clrScr() {
    word *p = buffer;
    word *end = buffer + getDisplayXSize() * getDisplayYSize();
    while (p < end) {
        *p++ = 0;
    }
}

void UTFT::drawRect(int x1, int y1, int x2, int y2) {
    drawHLine(x1, y1, x2 - x1 + 1);
    drawHLine(x1, y2, x2 - x1 + 1);
	drawVLine(x1, y1, y2 - y1 + 1);
	drawVLine(x2, y1, y2 - y1 + 1);
}

void UTFT::fillRect(int x1, int y1, int x2, int y2) {
    if (orient == PORTRAIT) {
        setXY(x1, y1, x2, y2);
        for (int i = 0; i < (x2 - x1 + 1) * (y2 - y1 + 1); ++i) {
            setPixel((fch << 8) | fcl);
        }
    } else {
        for (int iy = y1; iy <= y2; ++iy) {
            setXY(x1, iy, x2, iy);
            for (int ix = x2; ix >= x1; --ix) {
                setPixel((fch << 8) | fcl);
            }
        }
    }
}

void UTFT::drawHLine(int x, int y, int l) {
    setXY(x, y, x + l, y);
    for (int i = 0; i < l; ++i) {
        setPixel((fch << 8) | fcl);
    }
}

void UTFT::drawVLine(int x, int y, int l) {
    if (orient == PORTRAIT) {
        setXY(x, y, x, y + l);
        for (int i = 0; i < l; ++i) {
            setPixel((fch << 8) | fcl);
        }
    } else {
        for (int i = 0; i < l; ++i) {
            setXY(x, y + i, x, y + i);
            setPixel((fch << 8) | fcl);
        }
    }
}

void UTFT::drawBitmap(int x, int y, int sx, int sy, bitmapdatatype data, int scale) {
    if (orient == PORTRAIT) {
        setXY(x, y, x + sx - 1, y + sy - 1);
        for (int i = 0; i < sx * sy; ++i) {
            unsigned char l = arduino_util::prog_read_byte(((uint8_t *)data) + 2 * i + 0);
            unsigned char h = arduino_util::prog_read_byte(((uint8_t *)data) + 2 * i + 1);
            setPixel((h << 8) + l);
        }
    } else {
        for (int iy = 0; iy < sy; ++iy) {
            setXY(x, y + iy, x + sx - 1, y + iy);
            for (int ix = sx - 1; ix >= 0; --ix) {
                unsigned char l = arduino_util::prog_read_byte(((uint8_t *)data) + 2 * (iy * sx + ix) + 0);
                unsigned char h = arduino_util::prog_read_byte(((uint8_t *)data) + 2 * (iy * sx + ix) + 1);
                setPixel((h << 8) + l);
            }
        }
    }
}

}
}
}
} // namespace eez::psu::simulator::arduino;
