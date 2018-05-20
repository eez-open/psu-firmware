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

namespace eez {
namespace mw {
namespace gui {

bool styleIsBlink(const Style *style);

void drawText(int pageId, const char *text, int textLength, 
    int x, int y, int w, int h, 
    const Style *style, bool inverse, bool blink, bool ignoreLuminocity, uint16_t *overrideBackgroundColor);

void drawMultilineText(int pageId, const char *text, 
    int x, int y, int w, int h, 
    const Style *style, bool inverse);

void drawBitmap(uint8_t bitmapIndex, 
    int x, int y, int w, int h, 
    const Style *style, bool inverse);

void drawRectangle(int x, int y, int w, int h, 
    const Style *style, bool inverse, bool ignoreLuminocity);

}
}
} // namespace eez::mw::gui
