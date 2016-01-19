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
#include "lcd.h"
#include "font.h"

#include "channel.h"

namespace eez {
namespace psu {
namespace ui {

#define HORZ_ALIGN_LEFT   1
#define HORZ_ALIGN_RIGHT  2
#define HORZ_ALIGN_CENTER 3

using namespace lcd;

void init() {
    lcd::init();

    lcd::lcd.setBackColor(255, 255, 255);
    
    lcd::lcd.setColor(255, 255, 255);
    lcd::lcd.fillRect(0, 0, lcd::lcd.getDisplayXSize() - 1, lcd::lcd.getDisplayYSize() - 1);
    
    //lcd::lcd.setColor(0, 0, 255);
    //lcd::lcd.drawRect(0, 0, lcd::lcd.getDisplayXSize() - 1, lcd::lcd.getDisplayYSize() - 1);

    //lcd::lcd.setColor(0, 0, 0);
    //lcd::lcd.drawStr(10, 150, "Hello, world!", font::MEDIUM_FONT);
}

void tick(unsigned long tick_usec) {
    char str[16] = { 0 };
    util::strcatVoltage(str, Channel::get(0).u.mon);

    font::Font &font = font::SMALL_FONT;

    int vertPadding = 4;
    int horzPadding = 8;
    int align = HORZ_ALIGN_CENTER;

    int x1 = 0;
    int x2 = lcd::lcd.getDisplayXSize() - 1;

    int y1 = 0;
    int yBaseLine = y1 + vertPadding + font.getAscent();
    int y2 = yBaseLine + font.getDescent() + vertPadding;
    
    lcd::lcd.setColor(255, 255, 255);
    lcd::lcd.fillRect(x1, y1, x2, y2);

    lcd::lcd.setColor(0, 255, 0);
    lcd::lcd.drawRect(x1, y1, x2, y2);
    lcd::lcd.drawRect(x1 + horzPadding, y1 + vertPadding, x2 - horzPadding, y2 - vertPadding);

    int width = lcd::lcd.measureStr(str, font);

    int xOffset;
    if (align == HORZ_ALIGN_LEFT) xOffset = horzPadding;
    else if (align == HORZ_ALIGN_RIGHT) xOffset = x2 - horzPadding - width;
    else xOffset = x1 + ((x2 - x1) - width) / 2;

    lcd::lcd.setColor(255, 0, 0);
    lcd::lcd.drawHLine(xOffset, yBaseLine, width);

    lcd::lcd.setColor(0, 0, 0);
    lcd::lcd.drawStr(xOffset, yBaseLine, str, font);

    strcpy(str, str);
}

}
}
} // namespace eez::psu::ui
