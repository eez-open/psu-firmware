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

#pragma once

#include "mw_gui_lcd.h"
#include "mw_gui_font.h"
#include "mw_gui_touch.h"

#include "mw_gui_data.h"
#include "mw_gui_view.h"

#include "gui_document.h"
using namespace eez::app::gui;


namespace eez {
namespace mw {
namespace gui {

enum InternalActionsEnum {
	ACTION_ID_INTERNAL_SELECT_ENUM_ITEM = -1,
	ACTION_ID_INTERNAL_SHOW_PREVIOUS_PAGE = -2
};

#define INTERNAL_PAGE_ID_NONE             -1
#define INTERNAL_PAGE_ID_SELECT_FROM_ENUM -2

class Page;

bool isActivePageInternal();

void setPage(int pageId);
void replacePage(int pageId, Page *page = 0);
bool isPageActiveOrOnStack(int pageId);

font::Font styleGetFont(const Style *style);
void drawText(int pageId, const char *text, int textLength, int x, int y, int w, int h, const Style *style, bool inverse, bool dimmed = false, bool ignoreLuminocity = false, uint16_t *overrideBackgroundColor = 0);

void upDown();

////////////////////////////////////////////////////////////////////////////////
// GUI definition document accessor functions

extern Styles *g_styles;
extern Document *g_document;

inline OBJ_OFFSET getListItemOffset(const List &list, int index, int listItemSize) {
    return list.first + index * listItemSize;
}

inline OBJ_OFFSET getStyleOffset(int styleID) {
    if (styleID == 0) {
        return 0;
    }
    return getListItemOffset(*g_styles, styleID - 1, sizeof(Style));
}

inline OBJ_OFFSET getCustomWidgetOffset(int customWidgetID) {
    if (customWidgetID == 0) {
        return 0;
    }
    return getListItemOffset(g_document->customWidgets, customWidgetID - 1, sizeof(CustomWidget));
}

inline OBJ_OFFSET getPageOffset(int pageID) {
    return getListItemOffset(g_document->pages, pageID, sizeof(Widget));
}

#define DECL_WIDGET_STYLE(var, widget) DECL_STYLE(var, transformStyleHook(widget))
#define DECL_STYLE(var, styleID) const Style *var = (const Style *)(styles + getStyleOffset(styleID))

#define DECL_CUSTOM_WIDGET(var, customWidgetID) const CustomWidget *var = (const CustomWidget *)(document + getCustomWidgetOffset(customWidgetID))

#define DECL_WIDGET(var, widgetOffset) const Widget *var = (const Widget *)(document + (widgetOffset))
#define DECL_WIDGET_SPECIFIC(type, var, widget) const type *var = (const type *)(document + (widget)->specific)
#define DECL_STRING(var, offset) const char *var = (const char *)(document + (offset))
#define DECL_BITMAP(var, offset) const Bitmap *var = (const Bitmap *)(document + (offset))
#define DECL_STRUCT_WITH_OFFSET(Struct, var, offset) const Struct *var = (const Struct *)(document + (offset))

////////////////////////////////////////////////////////////////////////////////

}
}
} // namespace eez::mw::ui
