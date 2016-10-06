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

#include "psu.h"
#include "gui_widget_button_group.h"
#include "gui_data_snapshot.h"

namespace eez {
namespace psu {
namespace gui {
namespace widgetButtonGroup {

void drawButtons(const Widget* widget, int x, int y, const Style *style, int selectedButton, const data::Value *labels, int count) {
    if (widget->w > widget->h) {
        // horizontal orientation
		lcd::lcd.setColor(style->background_color);
		lcd::lcd.fillRect(x, y, x + widget->w - 1, y + widget->h - 1);

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

		int bottom = y + widget->h - 1;
		int topPadding = (widget->h - h * count) / 2;
		
		if (topPadding > 0) {
			lcd::lcd.setColor(style->background_color);
			lcd::lcd.fillRect(x, y, x + widget->w - 1, y + topPadding - 1);

			y += topPadding;
		}

		int labelHeight = min(w, h);
		int yOffset = (h - labelHeight) / 2;

		for (int i = 0; i < count; ++i) {
			if (yOffset > 0) {
				lcd::lcd.setColor(style->background_color);
				lcd::lcd.fillRect(x, y, x + widget->w - 1, y + yOffset - 1);
			}

			char text[32];
            labels[i].toText(text, 32);
            drawText(text, -1, x, y + yOffset, w, labelHeight , style, i == selectedButton);

			int b = y + yOffset + labelHeight;
			
			y += h;
        
			if (b < y) {
				lcd::lcd.setColor(style->background_color);
				lcd::lcd.fillRect(x, b, x + widget->w - 1, y - 1);
			}
		}

		if (y <= bottom) {
			lcd::lcd.setColor(style->background_color);
			lcd::lcd.fillRect(x, y, x + widget->w - 1, bottom);
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

        selectedButton = (touch::x - x) / w;
    } else {
        int h = widget->h / count;
        int y = widgetCursor.y + (widget->h - h * count) / 2;
        selectedButton = (touch::y - y) / h;
    }

    if (selectedButton >= 0 && selectedButton < count) {
        data::set(widgetCursor.cursor, widget->data, selectedButton, 0);
    }
}


}
}
}
} // namespace eez::psu::gui::widget_button_group
