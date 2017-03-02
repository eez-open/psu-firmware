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

namespace eez {
namespace psu {
namespace gui {

#pragma pack(push, 1)

#define STYLE_FLAGS_BORDER 1
#define STYLE_FLAGS_HORZ_ALIGN 6
#define STYLE_FLAGS_HORZ_ALIGN_LEFT 0
#define STYLE_FLAGS_HORZ_ALIGN_RIGHT 2
#define STYLE_FLAGS_HORZ_ALIGN_CENTER 4
#define STYLE_FLAGS_VERT_ALIGN 24
#define STYLE_FLAGS_VERT_ALIGN_TOP 0
#define STYLE_FLAGS_VERT_ALIGN_BOTTOM 8
#define STYLE_FLAGS_VERT_ALIGN_CENTER 16

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

#define LIST_TYPE_VERTICAL 1
#define LIST_TYPE_HORIZONTAL 2

#define SCALE_NEEDLE_POSITION_LEFT 1
#define SCALE_NEEDLE_POSITION_RIGHT 2
#define SCALE_NEEDLE_POSITION_TOP 3
#define SCALE_NEEDLE_POSITION_BOTTOM 4

#define BAR_GRAPH_ORIENTATION_LEFT_RIGHT 1
#define BAR_GRAPH_ORIENTATION_RIGHT_LEFT 2
#define BAR_GRAPH_ORIENTATION_TOP_BOTTOM 3
#define BAR_GRAPH_ORIENTATION_BOTTOM_TOP 4

typedef uint16_t OBJ_OFFSET;

////////////////////////////////////////////////////////////////////////////////

struct List {
    uint8_t count;
    OBJ_OFFSET first;
};

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

typedef uint8_t ActionType;

////////////////////////////////////////////////////////////////////////////////

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
};

struct ContainerWidget {
    List widgets;
};

struct ListWidget {
	uint8_t listType; // LIST_TYPE_VERTICAL or LIST_TYPE_HORIZONTAL
    OBJ_OFFSET item_widget;
};

struct SelectWidget {
    List widgets;
};

struct DisplayDataWidget {
    uint8_t activeStyle;
};

struct TextWidget {
    OBJ_OFFSET text;
};

struct MultilineTextWidget {
    OBJ_OFFSET text;
};

struct BitmapWidget {
    uint8_t bitmap;
};

struct ButtonWidget {
	OBJ_OFFSET text;
	uint8_t enabled;
	uint8_t disabledStyle;
};

struct ToggleButtonWidget {
    OBJ_OFFSET text1;
    OBJ_OFFSET text2;
};

struct ScaleWidget {
	uint8_t needle_position; // SCALE_NEEDLE_POSITION_...
	uint8_t needle_width;
    uint8_t needle_height;
};

struct BarGraphWidget {
	uint8_t orientation; // BAR_GRAPH_ORIENTATION_...
	uint8_t textStyle;
	uint8_t line1Data;
	uint8_t line1Style;
	uint8_t line2Data;
	uint8_t line2Style;
};

struct BarGraphWidgetState {
    WidgetState genericState;
    data::Value line1Data;
    data::Value line2Data;
};

struct YTGraphWidget {
	uint8_t y1Style;
	uint8_t y2Data;
	uint8_t y2Style;
};

struct YTGraphWidgetState {
    WidgetState genericState;
    data::Value y2Data;
};

enum UpDownWidgetSegment {
    UP_DOWN_WIDGET_SEGMENT_TEXT,
    UP_DOWN_WIDGET_SEGMENT_DOWN_BUTTON,
    UP_DOWN_WIDGET_SEGMENT_UP_BUTTON
};

struct UpDownWidget {
	uint8_t buttonsStyle;
	OBJ_OFFSET downButtonText;
	OBJ_OFFSET upButtonText;
};

struct ListGraphWidgetState {
    WidgetState genericState;
    data::Value cursorData;
};

struct ListGraphWidget {
    uint8_t dwellData;
    uint8_t y1Data;
	uint8_t y1Style;
    uint8_t y2Data;
	uint8_t y2Style;
    uint8_t cursorData;
    uint8_t cursorStyle;
};

struct CustomWidgetSpecific {
	uint8_t customWidget;
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
};

////////////////////////////////////////////////////////////////////////////////

struct CustomWidget {
    List widgets;
};

////////////////////////////////////////////////////////////////////////////////

struct Document {
    List customWidgets;
    List pages;
};

#pragma pack(pop)

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

    operator bool() {
        return widgetOffset != 0;
    }
};

////////////////////////////////////////////////////////////////////////////////

void drawWidget(const WidgetCursor &widgetCursor);
void refreshWidget(WidgetCursor widgetCursor);
void selectWidget(WidgetCursor &widgetCursor);
void deselectWidget();
void flush();

typedef void(*EnumWidgetsCallback)(const WidgetCursor &widgetCursor);
void enumWidgets(int pageIndex, WidgetState *previousState, WidgetState *currentState, EnumWidgetsCallback callback);

WidgetCursor findWidget(int x, int y);
void drawTick();

int getCurrentStateBufferIndex();

void onTouchListGraph(const WidgetCursor &widgetCursor);

}
}
} // namespace eez::psu::gui
