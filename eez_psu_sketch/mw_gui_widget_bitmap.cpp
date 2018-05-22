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

#include "mw_mw.h"

#if OPTION_DISPLAY

#include "mw_util.h"
#include "mw_gui_gui.h"
#include "mw_gui_widget_bitmap.h"
#include "mw_gui_draw.h"

namespace eez {
namespace mw {
namespace gui {

void BitmapWidget_draw(int pageId, const WidgetCursor &widgetCursor) {
	widgetCursor.currentState->size = sizeof(WidgetState);

	bool refresh = !widgetCursor.previousState ||
		widgetCursor.previousState->flags.pressed != widgetCursor.currentState->flags.pressed;

	if (refresh) {
		DECL_WIDGET(widget, widgetCursor.widgetOffset);
		DECL_WIDGET_SPECIFIC(BitmapWidget, display_bitmap_widget, widget);
		DECL_WIDGET_STYLE(style, widget);
		drawBitmap(display_bitmap_widget->bitmap, widgetCursor.x, widgetCursor.y, (int)widget->w, (int)widget->h, style,
			widgetCursor.currentState->flags.pressed);
	}
}

}
}
} // namespace eez::mw::gui

#endif