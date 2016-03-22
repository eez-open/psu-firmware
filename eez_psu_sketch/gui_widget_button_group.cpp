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

void drawButtons(const WidgetCursor &widgetCursor, Style *style, int selectedButton, const data::Value *labels, int count) {
    int x = widgetCursor.x;
    int y = widgetCursor.y;
    if (widgetCursor.widget->w > widgetCursor.widget->h) {
        // horizontal orientation
        int w = widgetCursor.widget->w / count;
        x += (widgetCursor.widget->w - w * count) / 2;
        int h = widgetCursor.widget->h;
        for (int i = 0; i < count; ++i) {
            char text[32];
            labels[i].toText(text, 32);
            drawText(text, x, y, w, h, style, i == selectedButton);
            x += w;
        }
    } else {
        // vertical orientation
        int w = widgetCursor.widget->w;
        int h = widgetCursor.widget->h / count;
        y += (widgetCursor.widget->h - h * count) / 2;
        for (int i = 0; i < count; ++i) {
            char text[32];
            labels[i].toText(text, 32);
            drawText(text, x, y + (h - min(w, h)) / 2, w, min(w, h), style, i == selectedButton);
            y += h;
        }

    }
}

bool draw(uint8_t *document, const WidgetCursor &widgetCursor, bool refresh, bool inverse) {
    int selectedButton = data::currentSnapshot.get(widgetCursor.cursor, widgetCursor.widget->data).getInt();
    if (!refresh) {
        int previousSelectedButton = data::previousSnapshot.get(widgetCursor.cursor, widgetCursor.widget->data).getInt();
        refresh = selectedButton != previousSelectedButton;
    }
    if (refresh) {
        const data::Value *labels;
        int count;
        data::getButtonLabels(widgetCursor.cursor, widgetCursor.widget->data, &labels, count);
        drawButtons(widgetCursor, (Style *)(document + widgetCursor.widget->style), selectedButton, labels, count);
        return true;
    }
    return false;
}

void onTouchDown(const WidgetCursor &widgetCursor) {
    const data::Value *labels;
    int count;
    data::getButtonLabels(widgetCursor.cursor, widgetCursor.widget->data, &labels, count);

    int selectedButton;
    if (widgetCursor.widget->w > widgetCursor.widget->h) {
        int w = widgetCursor.widget->w / count;
        int x = widgetCursor.x + (widgetCursor.widget->w - w * count) / 2;

        selectedButton = (touch::x / DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER - x) / w;
    } else {
        int h = widgetCursor.widget->h / count;
        int y = widgetCursor.y + (widgetCursor.widget->h - h * count) / 2;
        selectedButton = (touch::y / DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER - y) / h;
    }

    if (selectedButton >= 0 && selectedButton < count) {
        data::set(widgetCursor.cursor, widgetCursor.widget->data, selectedButton);
    }
}


}
}
}
} // namespace eez::psu::gui::widget_button_group
