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

namespace eez {
namespace psu {
namespace ui {

void init() {
    lcd::init();

    lcd::lcd.setBackColor(255, 255, 255);
    
    lcd::lcd.setColor(255, 255, 255);
    lcd::lcd.fillRect(0, 0, lcd::lcd.getDisplayXSize() - 1, lcd::lcd.getDisplayYSize() - 1);
    
    lcd::lcd.setColor(0, 0, 255);
    lcd::lcd.drawRect(0, 0, lcd::lcd.getDisplayXSize() - 1, lcd::lcd.getDisplayYSize() - 1);

    lcd::lcd.setColor(0, 0, 0);
    lcd::lcd.drawStr(10, 150, "Hello, world!", font::MEDIUM_FONT);
}

}
}
} // namespace eez::psu::ui
