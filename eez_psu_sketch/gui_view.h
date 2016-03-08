/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
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

namespace eez {
namespace psu {
namespace gui {

#pragma pack(push, 1)

#define SMALL_FONT 1
#define MEDIUM_FONT 2
#define LARGE_FONT 3
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
#define WIDGET_TYPE_VERTICAL_LIST 2
#define WIDGET_TYPE_SELECT 3
#define WIDGET_TYPE_DISPLAY 4
#define WIDGET_TYPE_DISPLAY_STRING 5
#define WIDGET_TYPE_DISPLAY_STRING_SELECT 6
#define WIDGET_TYPE_THREE_STATE_INDICATOR 7
#define WIDGET_TYPE_VERTICAL_SLIDER 8
#define DISPLAY_POSITION_OR_SIZE_FIELD_MULTIPLIER 2
#define DATA_ID_CHANNELS 1
#define DATA_ID_OUTPUT_STATE 2
#define DATA_ID_OUTPUT_MODE 3
#define DATA_ID_MON_VALUE 4
#define DATA_ID_VOLT 5
#define DATA_ID_CURR 6
#define DATA_ID_OVP 7
#define DATA_ID_OCP 8
#define DATA_ID_OPP 9
#define DATA_ID_OTP 10
#define DATA_ID_DP 11
#define ACTION_ID_SLIDER 1
#define ACTION_ID_EXIT 2
#define ACTION_ID_TOGGLE_CHANNEL 3

typedef uint16_t OBJ_OFFSET;

struct List {
    uint8_t count;
    OBJ_OFFSET first;
};

struct VerticalListWidget {
    OBJ_OFFSET item_widget;
};

struct SelectWidget {
    List widgets;
};

struct Style {
    uint8_t font;
    uint16_t flags;
    uint16_t background_color;
    uint16_t color;
    uint16_t border_color;
    uint16_t padding_horizontal;
    uint16_t padding_vertical;
};

struct DisplayStringWidget {
    OBJ_OFFSET text;
};

struct Document {
    List styles;
    List pages;
};

struct ContainerWidget {
    List widgets;
};

struct ThreeStateIndicatorWidget {
    OBJ_OFFSET style1;
    OBJ_OFFSET style2;
    OBJ_OFFSET text;
};

struct Widget {
    uint8_t type;
    uint8_t data;
    uint8_t action;
    uint8_t x;
    uint8_t y;
    uint8_t w;
    uint8_t h;
    OBJ_OFFSET style;
    OBJ_OFFSET specific;
};

struct DisplayStringSelectWidget {
    OBJ_OFFSET style1;
    OBJ_OFFSET text1;
    OBJ_OFFSET style2;
    OBJ_OFFSET text2;
};

#pragma pack(pop)

extern uint8_t document[];

}
}
} // namespace eez::psu::gui
