/*
 * EEZ Middleware
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

#include "mw_gui_font.h"

namespace eez {
namespace mw {
namespace gui {
namespace lcd {

#define COLOR_BLACK	0x0000
#define COLOR_WHITE	0xFFFF
#define COLOR_RED	0xF800
#define COLOR_GREEN	0x0400
#define COLOR_BLUE	0x001F

int getDisplayWidth();
int getDisplayHeight();

#if defined(EEZ_PLATFORM_SIMULATOR) || defined(EEZ_PLATFORM_STM32)
uint16_t *getBuffer();
#endif

void setColor(uint8_t r, uint8_t g, uint8_t b);
void setColor(uint16_t color, bool ignoreLuminocity = false);
uint16_t getColor();

void setBackColor(uint8_t r, uint8_t g, uint8_t b);
void setBackColor(uint16_t color, bool ignoreLuminocity = false);
uint16_t getBackColor();

void drawPixel(int x, int y);
void drawRect(int x1, int y1, int x2, int y2);
void fillRect(int x1, int y1, int x2, int y2);
void drawHLine(int x, int y, int l);
void drawVLine(int x, int y, int l);
void drawBitmap(int x, int y, int sx, int sy, uint16_t* data);
void drawStr(int pageId, const char *text, int textLength, int x, int y, int clip_x1, int clip_y1, int clip_x2, int clip_y2, font::Font &font, bool fill_background);
int measureStr(const char *text, int textLength, font::Font &font, int max_width = 0);

}

}
}
} // namespace eez::mw::gui::lcd
