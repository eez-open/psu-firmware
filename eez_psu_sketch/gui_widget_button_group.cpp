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
#include "gui_widget_button_group.h"
#include "gui_data_snapshot.h"

namespace eez {
namespace psu {
namespace gui {
namespace widget_button_group {

void drawButtons(const Widget* widget, int x, int y, const Style *style, int selectedButton, const data::Value *labels, int count) {
    if (widget->w > widget->h) {
        // horizontal orientation
        int w = widget->w / count;
        x += (widget->w - w * count) / 2;
        int h = widget->h;
        for (int i = 0; i < count; ++i) {
            char text[32];
            labels[i].toText(text, 32);
            drawText(text, -1, x, y, w, h, style, i == selectedButton);
            x += w;
        }
    } else {
        // vertical orientation
        int w = widget->w;
        int h = widget->h / count;
        y += (widget->h - h * count) / 2;
        for (int i = 0; i < count; ++i) {
            char text[32];
            labels[i].toText(text, 32);
            drawText(text, -1, x, y + (h - min(w, h)) / 2, w, min(w, h), style, i == selectedButton);
            y += h;
        }

    }
}

bool draw(const WidgetCursor &widgetCursor, const Widget *widget, bool refresh, bool inverse) {
    int selectedButton = data::currentSnapshot.get(widgetCursor.cursor, widget->data).getInt();
    if (!refresh) {
        int previousSelectedButton = data::previousSnapshot.get(widgetCursor.cursor, widget->data).getInt();
        refresh = selectedButton != previousSelectedButton;
    }
    if (refresh) {
        const data::Value *labels;
        int count;
        data::getButtonLabels(widgetCursor.cursor, widget->data, &labels, count);
        DECL_WIDGET_STYLE(style, widget);
        drawButtons(widget, widgetCursor.x, widgetCursor.y, style, selectedButton, labels, count);
        return true;
    }
    return false;
}

void onTouchDown(const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);

    const data::Value *labels;
    int count;
    data::getButtonLabels(widgetCursor.cursor, widget->data, &labels, count);

    int selectedButton;
    if (widget->w > widget->h) {
        int w = widget->w / count;
        int x = widgetCursor.x + (widget->w - w * count) / 2;

        selectedButton = (touch::x / DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER - x) / w;
    } else {
        int h = widget->h / count;
        int y = widgetCursor.y + (widget->h - h * count) / 2;
        selectedButton = (touch::y / DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER - y) / h;
    }

    if (selectedButton >= 0 && selectedButton < count) {
        data::set(widgetCursor.cursor, widget->data, selectedButton);
    }
}


}
}
}
} // namespace eez::psu::gui::widget_button_group
