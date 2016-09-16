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

#pragma once

#include "lcd.h"
#include "font.h"
#include "touch.h"
#include "gesture.h"

#include "gui_data.h"
#include "gui_view.h"
#include "gui_page.h"

#include "actions.h"
#include "gui_document.h"

#if defined(EEZ_PSU_ARDUINO_MEGA)
#include "arduino_util.h"
#endif

namespace eez {
namespace psu {
namespace gui {

struct WidgetCursor {
    OBJ_OFFSET widgetOffset;
    int x;
    int y;
    data::Cursor cursor;

    WidgetCursor() : widgetOffset(0) {}

    WidgetCursor(OBJ_OFFSET widgetOffset_, int x_, int y_, const data::Cursor &cursor_)
        : widgetOffset(widgetOffset_), x(x_), y(y_), cursor(cursor_) 
    {
    }

    WidgetCursor& operator=(int) {
        widgetOffset = 0;
		cursor.iChannel = -1;
        return *this;
    }

    bool operator != (const WidgetCursor& rhs) const {
        return widgetOffset != rhs.widgetOffset || x != rhs.x || y != rhs.y || cursor != rhs.cursor;
    }

    bool operator == (const WidgetCursor& rhs) const {
        return !(*this != rhs);
    }

    operator bool() {
        return widgetOffset != 0;
    }

    //const Widget* getWidget() const;
};

class Page;

int getActivePageId();
Page *getActivePage();
void showPage(int index, bool pushOnStack = true);
void showAuxPage(int index);
void showPreviousPage();
void refreshPage();

font::Font styleGetFont(const Style *style);
void drawText(const char *text, int textLength, int x, int y, int w, int h, const Style *style, bool inverse);
void fillRect(int x, int y, int w, int h);

void infoMessage(data::Value value, void (*ok_callback)() = 0);
void infoMessageP(const char *message PROGMEM, void (*ok_callback)() = 0);

void errorMessage(data::Value value, void (*ok_callback)() = 0);
void errorMessageP(const char *message PROGMEM, void (*ok_callback)() = 0);

void yesNoDialog(int yesNoPageId, const char *message PROGMEM, void (*yes_callback)(), void (*no_callback)(), void (*cancel_callback)());

void dialogYes();
void dialogNo();
void dialogCancel();
void dialogOk();

extern WidgetCursor g_foundWidgetAtDown;

void selectChannel();
extern Channel *g_channel;

////////////////////////////////////////////////////////////////////////////////
// GUI definition document accessor functions

extern Document *g_doc;

inline OBJ_OFFSET getListItemOffset(const List &list, int index, int listItemSize) {
    return list.first + index * listItemSize;
}

inline OBJ_OFFSET getPageOffset(int pageID) {
    return getListItemOffset(g_doc->pages, pageID, sizeof(Widget));
}

inline const int getWidgetStyleId(const Widget *widget) {
    return (widget->style - g_doc->styles.first) / sizeof(Style);
}

#if defined(EEZ_PSU_ARDUINO_MEGA)

#define DECL_WIDGET(var, widgetOffset) \
    Widget var##_buffer; \
    arduino_util::prog_read_buffer(document + (widgetOffset), (uint8_t *)&var##_buffer, sizeof(Widget)); \
    const Widget *var = &var##_buffer

#define DECL_WIDGET_STYLE(var, widget) \
    Style var##_buffer; \
    arduino_util::prog_read_buffer(document + (widget)->style, (uint8_t *)&var##_buffer, sizeof(Style)); \
    const Style *var = &var##_buffer

#define DECL_STYLE_WITH_OFFSET(var, styleOffset) \
    Style var##_buffer; \
    arduino_util::prog_read_buffer(document + (styleOffset), (uint8_t *)&var##_buffer, sizeof(Style)); \
    const Style *var = &var##_buffer

#define DECL_STYLE(var, styleId) \
    Style var##_buffer; \
    arduino_util::prog_read_buffer(document + g_doc->styles.first + (styleId) * sizeof(Style), (uint8_t *)&var##_buffer, sizeof(Style)); \
    const Style *var = &var##_buffer

#define DECL_WIDGET_SPECIFIC(type, var, widget) \
    type var##_buffer; \
    arduino_util::prog_read_buffer(document + (widget)->specific, (uint8_t *)&var##_buffer, sizeof(type)); \
    const type *var = &var##_buffer

#define DECL_STRING(var, offset) \
    char var##_buffer[128]; \
    strncpy_P(var##_buffer, (const char *)(document + (offset)), sizeof(var##_buffer) - 1); \
    var##_buffer[sizeof(var##_buffer) - 1] = 0; \
    const char *var = var##_buffer

#define DECL_BITMAP(var, offset) \
    Bitmap var##_buffer; \
    arduino_util::prog_read_buffer(document + (offset), (uint8_t *)&var##_buffer, sizeof(Bitmap)); \
    const Bitmap *var = &var##_buffer

#define DECL_STRUCT_WITH_OFFSET(Struct, var, offset) \
    Struct var##_buffer; \
    arduino_util::prog_read_buffer(document + (offset), (uint8_t *)&var##_buffer, sizeof(Struct)); \
    const Struct *var = &var##_buffer

#else

#define DECL_WIDGET(var, widgetOffset) const Widget *var = (const Widget *)(document + (widgetOffset))
#define DECL_WIDGET_STYLE(var, widget) const Style *var = (const Style *)(document + (widget)->style)
#define DECL_STYLE_WITH_OFFSET(var, styleOffset) const Style *var = (const Style *)(document + (styleOffset))
#define DECL_WIDGET_SPECIFIC(type, var, widget) const type *var = (const type *)(document + (widget)->specific)
#define DECL_STYLE(var, styleId) const Style *var = (const Style *)(document + g_doc->styles.first + (styleId) * sizeof(Style))
#define DECL_STRING(var, offset) const char *var = (const char *)(document + (offset))
#define DECL_BITMAP(var, offset) const Bitmap *var = (const Bitmap *)(document + (offset))
#define DECL_STRUCT_WITH_OFFSET(Struct, var, offset) const Struct *var = (const Struct *)(document + (offset))
#endif

////////////////////////////////////////////////////////////////////////////////

}
}
} // namespace eez::psu::ui
