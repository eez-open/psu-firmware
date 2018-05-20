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
#include "eez/mw/gui/widget/toggle_button.h"

namespace eez {
namespace mw {
namespace gui {

void ToggleButtonWidget_draw(int pageId, const WidgetCursor &widgetCursor) {
	DECL_WIDGET(widget, widgetCursor.widgetOffset);

	widgetCursor.currentState->size = sizeof(WidgetState);
	widgetCursor.currentState->flags.enabled = data::get(widgetCursor.cursor, widget->data).getInt() ? 1 : 0;

	bool refresh = !widgetCursor.previousState ||
		widgetCursor.previousState->flags.pressed != widgetCursor.currentState->flags.pressed ||
		widgetCursor.previousState->flags.enabled != widgetCursor.currentState->flags.enabled;

	if (refresh) {
		DECL_WIDGET_SPECIFIC(ToggleButtonWidget, toggle_button_widget, widget);
		DECL_STRING(text, widgetCursor.currentState->flags.enabled ? toggle_button_widget->text2 : toggle_button_widget->text1);
		DECL_WIDGET_STYLE(style, widget);
		drawText(pageId, text, -1, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
			widgetCursor.currentState->flags.pressed);
	}
}

}
}
} // namespace eez::mw::gui

#endif