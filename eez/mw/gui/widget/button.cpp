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
#include "eez/mw/gui/widget/button.h"

namespace eez {
namespace mw {
namespace gui {

void ButtonWidget_draw(int pageId, const WidgetCursor &widgetCursor) {
	DECL_WIDGET(widget, widgetCursor.widgetOffset);
	DECL_WIDGET_SPECIFIC(ButtonWidget, button_widget, widget);

	widgetCursor.currentState->size = sizeof(WidgetState);
	widgetCursor.currentState->flags.enabled = data::get(widgetCursor.cursor, button_widget->enabled).getInt() ? 1 : 0;
	widgetCursor.currentState->flags.blinking = isBlinkTime() && data::isBlinking(widgetCursor.cursor, widget->data);
	widgetCursor.currentState->data = widget->data ? data::get(widgetCursor.cursor, widget->data) : 0;

	bool refresh = !widgetCursor.previousState ||
		widgetCursor.previousState->flags.pressed != widgetCursor.currentState->flags.pressed ||
		widgetCursor.previousState->flags.enabled != widgetCursor.currentState->flags.enabled ||
		widgetCursor.previousState->flags.blinking != widgetCursor.currentState->flags.blinking ||
		widgetCursor.previousState->data != widgetCursor.currentState->data;

	if (refresh) {
		if (widget->data) {
			DECL_STYLE(style, widgetCursor.currentState->flags.enabled ? widget->style : button_widget->disabledStyle);

			if (widgetCursor.currentState->data.isString()) {
				drawText(pageId, widgetCursor.currentState->data.getString(), -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
					widgetCursor.currentState->flags.pressed,
					widgetCursor.currentState->flags.blinking);
			} else {
				DECL_STRING(text, button_widget->text);
				drawText(pageId, text, -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
					widgetCursor.currentState->flags.pressed,
					widgetCursor.currentState->flags.blinking);
			}
		} else {
			DECL_STRING(text, button_widget->text);
			DECL_STYLE(style, widgetCursor.currentState->flags.enabled ? widget->style : button_widget->disabledStyle);
			drawText(pageId, text, -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
				widgetCursor.currentState->flags.pressed,
				widgetCursor.currentState->flags.blinking);
		}
	}
}

}
}
} // namespace eez::mw::gui

#endif