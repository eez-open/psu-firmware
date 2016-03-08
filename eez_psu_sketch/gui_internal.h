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

#include "lcd.h"
#include "font.h"
#include "touch.h"
#include "gesture.h"

#include "gui_data.h"
#include "gui_view.h"

namespace eez {
namespace psu {
namespace gui {

struct WidgetCursor {
    Widget *widget;
    int x;
    int y;
    data::Cursor cursor;

    WidgetCursor() : widget(0) {}

    WidgetCursor& operator=(int) {
        widget = 0;
        return *this;
    }

    bool operator != (const Widget* rhs) const {
        return widget != rhs || cursor != data::getCursor();
    }
    bool operator == (const Widget* rhs) const {
        return !(*this != rhs);
    }

    bool operator != (const WidgetCursor& rhs) const {
        return widget != rhs.widget || x != rhs.x || y != rhs.y || cursor != rhs.cursor;
    }

    operator bool() {
        return widget != 0;
    }
};

extern int page_index;

void refresh_page();
font::Font *styleGetFont(Style *style);
void drawText(char *text, int x, int y, int w, int h, Style *style, bool inverse);
void fill_rect(int x, int y, int w, int h);

}
}
} // namespace eez::psu::ui
