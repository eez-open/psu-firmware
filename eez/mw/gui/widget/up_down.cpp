/*
 * EEZ Middleware
 * Copyright (C) 2018-present, Envox d.o.o.
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

#include "eez/mw/mw.h"

#if OPTION_DISPLAY

#include "eez/mw/util.h"
#include "eez/mw/gui/gui.h"
#include "eez/mw/gui/widget/up_down.h"
#include "eez/mw/gui/draw.h"
#include "eez/mw/gui/draw.h"

namespace eez {
namespace mw {
namespace gui {

enum UpDownWidgetSegment {
	UP_DOWN_WIDGET_SEGMENT_TEXT,
	UP_DOWN_WIDGET_SEGMENT_DOWN_BUTTON,
	UP_DOWN_WIDGET_SEGMENT_UP_BUTTON
};

UpDownWidgetSegment g_segment;
WidgetCursor g_selectedWidget;

void UpDownWidget_draw(int pageId, const WidgetCursor &widgetCursor) {
	DECL_WIDGET(widget, widgetCursor.widgetOffset);
	DECL_WIDGET_SPECIFIC(UpDownWidget, upDownWidget, widget);

	widgetCursor.currentState->size = sizeof(WidgetState);
	widgetCursor.currentState->data = data::get(widgetCursor.cursor, widget->data);
	widgetCursor.currentState->flags.pressed = g_selectedWidget == widgetCursor;

	bool refresh = !widgetCursor.previousState ||
		widgetCursor.previousState->flags.pressed != widgetCursor.currentState->flags.pressed ||
		widgetCursor.previousState->data != widgetCursor.currentState->data;

	if (refresh) {
		DECL_STRING(downButtonText, upDownWidget->downButtonText);
		DECL_STYLE(buttonsStyle, upDownWidget->buttonsStyle);

		font::Font buttonsFont = styleGetFont(buttonsStyle);
		int buttonWidth = buttonsFont.getHeight();

		drawText(pageId, downButtonText, -1, widgetCursor.x, widgetCursor.y, buttonWidth, (int)widget->h, buttonsStyle,
			widgetCursor.currentState->flags.pressed && g_segment == UP_DOWN_WIDGET_SEGMENT_DOWN_BUTTON,
			false, false, NULL);
		if (!isActivePage(pageId)) {
			return;
		}

		char text[64];
		widgetCursor.currentState->data.toText(text, sizeof(text));
		DECL_STYLE(style, widget->style);
		drawText(pageId, text, -1, widgetCursor.x + buttonWidth, widgetCursor.y, (int)(widget->w - 2 * buttonWidth), (int)widget->h, style, false, false, false, NULL);
		if (!isActivePage(pageId)) {
			return;
		}

		DECL_STRING(upButtonText, upDownWidget->upButtonText);
		drawText(pageId, upButtonText, -1, widgetCursor.x + widget->w - buttonWidth, widgetCursor.y, buttonWidth, (int)widget->h, buttonsStyle,
			widgetCursor.currentState->flags.pressed && g_segment == UP_DOWN_WIDGET_SEGMENT_UP_BUTTON, false, false, NULL);
	}
}

void upDown(const WidgetCursor &widgetCursor, UpDownWidgetSegment segment) {
	g_segment = segment;

	DECL_WIDGET(widget, widgetCursor.widgetOffset);

	int value = data::get(widgetCursor.cursor, widget->data).getInt();

	int newValue = value;

	if (g_segment == UP_DOWN_WIDGET_SEGMENT_DOWN_BUTTON) {
		--newValue;
	} else if (g_segment == UP_DOWN_WIDGET_SEGMENT_UP_BUTTON) {
		++newValue;
	}

	int min = data::getMin(widgetCursor.cursor, widget->data).getInt();
	if (newValue < min) {
		newValue = min;
	}

	int max = data::getMax(widgetCursor.cursor, widget->data).getInt();
	if (newValue > max) {
		newValue = max;
	}

	if (newValue != value) {
		data::set(widgetCursor.cursor, widget->data, newValue, 0);
	}
}

void UpDownWidget_onTouch(const WidgetCursor &widgetCursor, Event &touchEvent) {
	DECL_WIDGET(widget, widgetCursor.widgetOffset);

	if (touchEvent.type == EVENT_TYPE_TOUCH_DOWN || touchEvent.type == EVENT_TYPE_AUTO_REPEAT) {
		if (touchEvent.type == EVENT_TYPE_TOUCH_DOWN) {
			g_selectedWidget = widgetCursor;
		}

		if (touchEvent.x < widgetCursor.x + widget->w / 2) {
			upDown(widgetCursor, UP_DOWN_WIDGET_SEGMENT_DOWN_BUTTON);
		} else {
			upDown(widgetCursor, UP_DOWN_WIDGET_SEGMENT_UP_BUTTON);
		}
	} else if (touchEvent.type == EVENT_TYPE_TOUCH_UP) {
		g_selectedWidget = 0;
	}
}

}
}
} // namespace eez::mw::gui

#endif