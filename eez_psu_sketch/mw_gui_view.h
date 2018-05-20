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

#include "mw_gui_data.h"

namespace eez {
namespace mw {
namespace gui {

#define STYLE_FLAGS_BORDER 1
#define STYLE_FLAGS_HORZ_ALIGN 6
#define STYLE_FLAGS_HORZ_ALIGN_LEFT 0
#define STYLE_FLAGS_HORZ_ALIGN_RIGHT 2
#define STYLE_FLAGS_HORZ_ALIGN_CENTER 4
#define STYLE_FLAGS_VERT_ALIGN 24
#define STYLE_FLAGS_VERT_ALIGN_TOP 0
#define STYLE_FLAGS_VERT_ALIGN_BOTTOM 8
#define STYLE_FLAGS_VERT_ALIGN_CENTER 16
#define STYLE_FLAGS_BLINK 32

#define WIDGET_TYPE_NONE 0
#define WIDGET_TYPE_CONTAINER 1
#define WIDGET_TYPE_LIST 2
#define WIDGET_TYPE_SELECT 3
#define WIDGET_TYPE_DISPLAY_DATA 4
#define WIDGET_TYPE_TEXT 5
#define WIDGET_TYPE_MULTILINE_TEXT 6
#define WIDGET_TYPE_RECTANGLE 7
#define WIDGET_TYPE_BITMAP 8
#define WIDGET_TYPE_BUTTON 9
#define WIDGET_TYPE_TOGGLE_BUTTON 10
#define WIDGET_TYPE_BUTTON_GROUP 11
#define WIDGET_TYPE_SCALE 12
#define WIDGET_TYPE_BAR_GRAPH 13
#define WIDGET_TYPE_CUSTOM 14
#define WIDGET_TYPE_YT_GRAPH 15
#define WIDGET_TYPE_UP_DOWN 16
#define WIDGET_TYPE_LIST_GRAPH 17

typedef uint16_t OBJ_OFFSET;

////////////////////////////////////////////////////////////////////////////////

#pragma pack(push, 1)

struct List {
    uint8_t count;
    OBJ_OFFSET first;
};

struct Style {
    uint8_t font;
    uint16_t flags;
    uint16_t background_color;
    uint16_t color;
    uint16_t border_color;
    uint8_t padding_horizontal;
    uint8_t padding_vertical;
};

typedef List Styles;

typedef uint8_t ActionType;

struct Widget {
    uint8_t type;
    uint8_t data;
    ActionType action;
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;
    uint8_t style;
    OBJ_OFFSET specific;
};

struct Rect {
    int16_t x;
    int16_t y;
    uint16_t w;
    uint16_t h;
};

struct PageWidget {
    List widgets;
	List transparentRectangles;
    uint8_t closePageIfTouchedOutside;
};

struct CustomWidget {
    List widgets;
};

struct Document {
    List customWidgets;
    List pages;
};

#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////

#ifdef EEZ_PLATFORM_ARDUINO_DUE
#pragma pack(push, 1)
#endif

struct WidgetStateFlags {
    unsigned pressed: 1;
    unsigned focused: 1;
    unsigned blinking: 1;
    unsigned enabled: 1;
};

struct WidgetState {
    uint16_t size;
    WidgetStateFlags flags;
    data::Value data;
	uint16_t backgroundColor;
};

#ifdef EEZ_PLATFORM_ARDUINO_DUE
#pragma pack(pop)
#endif

////////////////////////////////////////////////////////////////////////////////

struct WidgetCursor {
    OBJ_OFFSET widgetOffset;
    int x;
    int y;
    data::Cursor cursor;
    int segment;
    WidgetState *previousState;
    WidgetState *currentState;

    WidgetCursor() : widgetOffset(0) {}

    WidgetCursor(OBJ_OFFSET widgetOffset_, int x_, int y_, const data::Cursor &cursor_, WidgetState *previousState_, WidgetState *currentState_)
        : widgetOffset(widgetOffset_), x(x_), y(y_), cursor(cursor_), previousState(previousState_), currentState(currentState_)
    {
    }

    WidgetCursor& operator=(int) {
        widgetOffset = 0;
		cursor.i = -1;
        return *this;
    }

    bool operator != (const WidgetCursor& rhs) const {
        return widgetOffset != rhs.widgetOffset || x != rhs.x || y != rhs.y || cursor != rhs.cursor;
    }

    bool operator == (const WidgetCursor& rhs) const {
        return !(*this != rhs);
    }

    operator bool() const {
        return widgetOffset != 0;
    }
};

////////////////////////////////////////////////////////////////////////////////

WidgetCursor& getSelectedWidget();
void selectWidget(WidgetCursor &widgetCursor);
void deselectWidget();

bool isBlinkTime();

typedef void(*EnumWidgetsCallback)(int pageId, const WidgetCursor &widgetCursor);
void enumWidgets(int pageId, EnumWidgetsCallback callback);

WidgetCursor findWidget(int x, int y);
void drawTick();

int getCurrentStateBufferIndex();

typedef void (*OnTouchDownFunctionType)(const WidgetCursor &widgetCursor, int xTouch, int yTouch);
extern OnTouchDownFunctionType g_onTouchDownFunctions[];

typedef void (*OnTouchMoveFunctionType)(const WidgetCursor &widgetCursor, int xTouch, int yTouch);
extern OnTouchMoveFunctionType g_onTouchMoveFunctions[];

}
}
} // namespace eez::mw::gui
