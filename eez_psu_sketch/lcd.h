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

#include "font.h"

namespace eez {
namespace psu {
namespace gui {
namespace lcd {

#define COLOR_BLACK	0x0000
#define COLOR_WHITE	0xFFFF
#define COLOR_RED	0xF800
#define COLOR_GREEN	0x0400
#define COLOR_BLUE	0x001F

class LCD {
public:
    LCD(uint8_t model, uint8_t RS, uint8_t WR, uint8_t CS, uint8_t RST);
    ~LCD();

#ifdef EEZ_PSU_SIMULATOR
    uint16_t *buffer;
#endif

    void init(uint8_t orientation);

    int getDisplayWidth();
    int getDisplayHeight();

    void setColor(uint8_t r, uint8_t g, uint8_t b);
    void setColor(uint16_t color);
    uint16_t getColor();

    void setBackColor(uint8_t r, uint8_t g, uint8_t b);
    void setBackColor(uint16_t color);
    uint16_t getBackColor();

    void drawPixel(int x, int y);
    void drawRect(int x1, int y1, int x2, int y2);
    void fillRect(int x1, int y1, int x2, int y2);
    void drawHLine(int x, int y, int l);
    void drawVLine(int x, int y, int l);
    void drawBitmap(int x, int y, int sx, int sy, uint16_t* data);
    void drawStr(const char *text, int textLength, int x, int y, int clip_x1, int clip_y1, int clip_x2, int clip_y2, font::Font &font, bool fill_background);
    int measureStr(const char *text, int textLength, font::Font &font, int max_width = 0);

private:
    uint8_t orientation;

    uint16_t displayWidth;
    uint16_t displayHeight;

    uint8_t fch, fcl, bch, bcl;

    font::Font font;

#ifdef EEZ_PSU_SIMULATOR
    uint16_t x, y, x1, y1, x2, y2;
#else
    uint8_t display_model;

    uint8_t RS, WR, CS, RST;
    volatile uint32_t *P_RS, *P_WR, *P_CS, *P_RST;
    uint16_t B_RS, B_WR, B_CS, B_RST;

    void write(char VH, char VL);
    void writeCom(char VL);
    void writeData(char VH, char VL);
    void writeData(char VL);
    void writeComData(char com1, int dat1);
    void setDirectionRegisters();
    void fastFill16(int ch, int cl, long numPixels);
#endif

    void setPixel(uint16_t color);
    void setXY(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    int8_t drawGlyph(int x1, int y1, int clip_x1, int clip_y1, int clip_x2, int clip_y2, uint8_t encoding, bool fill_background);
    int8_t measureGlyph(uint8_t encoding);
};

extern LCD lcd;

void init();
void turnOn();
void turnOff();

void updateBrightness();
   
}
}
}
} // namespace eez::psu::ui::lcd
