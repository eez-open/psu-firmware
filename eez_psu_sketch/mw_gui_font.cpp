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

#include "mw_mw.h"

#if OPTION_DISPLAY

#include "mw_gui_font.h"

namespace eez {
namespace mw {
namespace gui {
namespace font {

/*
Font header:

offset
0           ascent              unsigned
1           descent             unsigned
2           encoding start      unsigned
3           encoding end        unsigned
4           1st encoding offset unsigned word
6           2nd encoding offset unsigned word
...
*/

////////////////////////////////////////////////////////////////////////////////

Font::Font() : fontData(0) {
}

Font::Font(const uint8_t *data) : fontData(data) {
}

uint8_t Font::getByte(int offset) {
    return *(fontData + offset);
}

uint16_t Font::getWord(int offset) {
    const uint8_t *p = fontData + offset;
    uint8_t h = *p++;
    uint8_t l = *p;
    return (((uint16_t)h) << 8) + l;
}

uint8_t Font::getAscent() {
    return getByte(0);
}

uint8_t Font::getDescent() {
    return getByte(1);
}

uint8_t Font::getEncodingStart() {
    return getByte(2);
}

uint8_t Font::getEncodingEnd() {
    return getByte(3);
}

uint8_t Font::getHeight() {
    return getAscent() + getDescent();
}

const uint8_t *Font::findGlyphData(uint8_t requested_encoding) {
    const uint8_t *p = fontData;

    uint8_t start = getEncodingStart();
    uint8_t end = getEncodingEnd();

    if (requested_encoding < start || requested_encoding > end) {
        // Not found!
        return 0;
    }

    return p + getWord(4 + (requested_encoding - start) * 2);
}

void Font::fillGlyphParameters(Glyph &glyph) {
    /*
    Glyph header:

    offset
    0             DWIDTH                    signed
    1             BBX width                 unsigned
    2             BBX height                unsigned
    3             BBX xoffset               signed
    4             BBX yoffset               signed

    Note: byte 0 == 255 indicates empty glyph
    */

    glyph.dx = *((uint8_t *)(glyph.data + 0));
    glyph.width = *((uint8_t *)(glyph.data + 1));
    glyph.height = *((uint8_t *)(glyph.data + 2));
    glyph.x = *((uint8_t *)(glyph.data + 3));
    glyph.y = *((uint8_t *)(glyph.data + 4));
}

void Font::getGlyph(uint8_t requested_encoding, Glyph &glyph) {
    glyph.data = findGlyphData(requested_encoding);
    if (glyph.data) {
        fillGlyphParameters(glyph);
    }
}

}
}
}
} // namespace eez::mw::gui::font

#endif