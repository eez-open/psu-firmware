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
namespace gui {
namespace font {

static const int GLYPH_HEADER_SIZE = 5;

struct Glyph {
    const uint8_t *data;

    int8_t dx;
    int8_t x;
    int8_t y;
    uint8_t width;
    uint8_t height;

    bool isFound() { return data != 0; }
};

struct Font {
    const uint8_t *fontData;

    Font();
    Font(const uint8_t *data);

    void getGlyph(uint8_t requested_encoding, Glyph &glyph);
    
    uint8_t getAscent();
    uint8_t getDescent();
    uint8_t getHeight();

private:
    uint8_t getByte(int offset);
    uint16_t getWord(int offset);

    uint8_t getEncodingStart();
    uint8_t getEncodingEnd();

    const uint8_t * findGlyphData(uint8_t requested_encoding);
    void fillGlyphParameters(Glyph &glyph);
};
}
}
}
} // namespace eez::psu::gui::font
