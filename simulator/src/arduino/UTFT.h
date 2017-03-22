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
namespace simulator {
namespace arduino {

#define PORTRAIT 0
#define LANDSCAPE 1

#define ITDB32S         2   // SSD1289  (16bit)
#define ITDB32S_V2      33  // ILI9341  (16bit)
#define CTE32_R2        17

//*********************************
// COLORS
//*********************************
// VGA color palette
#define VGA_BLACK        0x0000
#define VGA_WHITE        0xFFFF
#define VGA_RED            0xF800
#define VGA_GREEN        0x0400
#define VGA_BLUE        0x001F
#define VGA_SILVER        0xC618
#define VGA_GRAY        0x8410
#define VGA_MAROON        0x8000
#define VGA_YELLOW        0xFFE0
#define VGA_OLIVE        0x8400
#define VGA_LIME        0x07E0
#define VGA_AQUA        0x07FF
#define VGA_TEAL        0x0410
#define VGA_NAVY        0x0010
#define VGA_FUCHSIA        0xF81F
#define VGA_PURPLE        0x8010

typedef uint16_t word;
typedef const unsigned short *bitmapdatatype PROGMEM;
typedef uint8_t regtype;
typedef uint8_t regsize;

class UTFT
{
public:
    UTFT(byte model, int RS, int WR, int CS, int RST, int SER=0);
    ~UTFT();

    void InitLCD(byte orientation=LANDSCAPE);
    void clrScr();
    void drawRect(int x1, int y1, int x2, int y2);
    void fillRect(int x1, int y1, int x2, int y2);
    void drawBitmap(int x, int y, int sx, int sy, bitmapdatatype data, int scale);
    void setColor(byte r, byte g, byte b);
    void setColor(word color);
    word getColor();
    void setBackColor(byte r, byte g, byte b);
    void setBackColor(word color);
    word getBackColor();
    void setContrast(char c);
    int  getDisplayXSize();
    int  getDisplayYSize();
    void setBrightness(byte br);

/*
    The functions and variables below should not normally be used.
    They have been left publicly available for use in add-on libraries
    that might need access to the lower level functions of UTFT.

    Please note that these functions and variables are not documented
    and I do not provide support on how to use them.
*/
    byte    fch, fcl, bch, bcl;
    byte    orient;
    int disp_x_size, disp_y_size;
    regtype CS;
    regtype *P_CS;
    regsize B_CS;
    word    x, y, x1, y1, x2, y2;
    word    *buffer;

    void setPixel(word color);
    void setXY(word x1_, word y1_, word x2_, word y2_);
    void clrXY();
    void drawHLine(int x, int y, int l);
    void drawVLine(int x, int y, int l);
    void drawPixel(int x, int y);
};

}
}
}
} // namespace eez::psu::simulator::arduino;

using namespace eez::psu::simulator::arduino;