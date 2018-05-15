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

#include "mw_util.h"
#include "mw_gui_gui.h"
#include "mw_gui_widget_button_group.h"

namespace eez {
namespace mw {
namespace gui {
namespace widgetButtonGroup {

void drawButtons(int pageId, const Widget* widget, int x, int y, const Style *style, int selectedButton, const data::Value *labels, int count) {
    if (widget->w > widget->h) {
        // horizontal orientation
		lcd::setColor(style->background_color);
		lcd::fillRect(x, y, x + widget->w - 1, y + widget->h - 1);

		int w = widget->w / count;
        x += (widget->w - w * count) / 2;
        int h = widget->h;
        for (int i = 0; i < count; ++i) {
            char text[32];
            labels[i].toText(text, 32);
            drawText(pageId, text, -1, x, y, w, h, style, i == selectedButton);
            if (!isActivePage(pageId)) {
                return;
            }
            x += w;
        }
    } else {
        // vertical orientation
        int w = widget->w;
        int h = widget->h / count;

		int bottom = y + widget->h - 1;
		int topPadding = (widget->h - h * count) / 2;

		if (topPadding > 0) {
			lcd::setColor(style->background_color);
			lcd::fillRect(x, y, x + widget->w - 1, y + topPadding - 1);

			y += topPadding;
		}

		int labelHeight = MIN(w, h);
		int yOffset = (h - labelHeight) / 2;

		for (int i = 0; i < count; ++i) {
			if (yOffset > 0) {
				lcd::setColor(style->background_color);
				lcd::fillRect(x, y, x + widget->w - 1, y + yOffset - 1);
			}

			char text[32];
            labels[i].toText(text, 32);
            drawText(pageId, text, -1, x, y + yOffset, w, labelHeight , style, i == selectedButton);
            if (!isActivePage(pageId)) {
                return;
            }

			int b = y + yOffset + labelHeight;

			y += h;

			if (b < y) {
				lcd::setColor(style->background_color);
				lcd::fillRect(x, b, x + widget->w - 1, y - 1);
			}
		}

		if (y <= bottom) {
			lcd::setColor(style->background_color);
			lcd::fillRect(x, y, x + widget->w - 1, bottom);
		}
	}
}

void draw(int pageId, const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);

    widgetCursor.currentState->size = sizeof(ButtonGroupWidgetState);
    widgetCursor.currentState->data = data::get(widgetCursor.cursor, widget->data);

    const data::Value *labels;
    int count;
    data::getList(widgetCursor.cursor, widget->data, &labels, count);

    ((ButtonGroupWidgetState *)widgetCursor.currentState)->labels = labels;

    bool refresh = !widgetCursor.previousState ||
        widgetCursor.previousState->flags.pressed != widgetCursor.currentState->flags.pressed ||
        widgetCursor.previousState->data != widgetCursor.currentState->data ||
        ((ButtonGroupWidgetState *)widgetCursor.previousState)->labels != ((ButtonGroupWidgetState *)widgetCursor.currentState)->labels;

    if (refresh) {
        DECL_WIDGET_STYLE(style, widget);
        drawButtons(pageId, widget, widgetCursor.x, widgetCursor.y, style, widgetCursor.currentState->data.getInt(), labels, count);
    }
}

void onTouchDown(const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);

    const data::Value *labels;
    int count;
    data::getList(widgetCursor.cursor, widget->data, &labels, count);

    int selectedButton;
    if (widget->w > widget->h) {
        int w = widget->w / count;
        int x = widgetCursor.x + (widget->w - w * count) / 2;

        selectedButton = (touch::g_x - x) / w;
    } else {
        int h = widget->h / count;
        int y = widgetCursor.y + (widget->h - h * count) / 2;
        selectedButton = (touch::g_y - y) / h;
    }

    if (selectedButton >= 0 && selectedButton < count) {
        data::set(widgetCursor.cursor, widget->data, selectedButton, 0);
        playClickSound();
    }
}


}
}
}
} // namespace eez::mw::gui::widget_button_group

#endif