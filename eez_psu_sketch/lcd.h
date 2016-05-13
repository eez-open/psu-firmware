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

#include "UTFT.h"
#include "font.h"

namespace eez {
namespace psu {
namespace gui {
namespace lcd {

class EEZ_UTFT : public UTFT {
public:
    EEZ_UTFT(byte model, int RS, int WR, int CS, int RST, int SER = 0);

    void drawStr(const char *text, int textLength, int x, int y, int clip_x1, int clip_y1, int clip_x2, int clip_y2, font::Font &font, bool fill_background);
    int measureStr(const char *text, int textLength, font::Font &font, int max_width = 0);

private:
    font::Font font;

    int8_t drawGlyph(int x1, int y1, int clip_x1, int clip_y1, int clip_x2, int clip_y2, uint8_t encoding, bool fill_background);
    int8_t measureGlyph(uint8_t encoding);
};

extern EEZ_UTFT lcd;

void init();
void turnOn();
void turnOff();
   
}
}
}
} // namespace eez::psu::ui::lcd
