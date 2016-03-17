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

#pragma once

namespace eez {
namespace psu {
namespace gui {

#pragma pack(push, 1)

#define SMALL_FONT 1
#define MEDIUM_FONT 2
#define LARGE_FONT 3
#define ICONS_FONT 4
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
#define WIDGET_TYPE_HORIZONTAL_LIST 3
#define WIDGET_TYPE_SELECT 4
#define WIDGET_TYPE_DISPLAY 5
#define WIDGET_TYPE_DISPLAY_STRING 6
#define WIDGET_TYPE_DISPLAY_STRING_SELECT 7
#define WIDGET_TYPE_THREE_STATE_INDICATOR 8
#define WIDGET_TYPE_VERTICAL_SLIDER 9
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
#define DATA_ID_ALERT_MESSAGE 12
#define DATA_ID_EDIT_UNIT 13
#define ACTION_ID_EDIT 1
#define ACTION_ID_EDIT_WITH_SLIDER 2
#define ACTION_ID_EDIT_WITH_STEP 3
#define ACTION_ID_EDIT_WITH_KEYPAD 4
#define ACTION_ID_EXIT 5
#define ACTION_ID_TOGGLE_CHANNEL 6
#define ACTION_ID_TOUCH_SCREEN_CALIBRATION 7
#define ACTION_ID_YES 8
#define ACTION_ID_NO 9
#define ACTION_ID_CANCEL 10
#define ACTION_ID_KEY_0 11
#define ACTION_ID_KEY_1 12
#define ACTION_ID_KEY_2 13
#define ACTION_ID_KEY_3 14
#define ACTION_ID_KEY_4 15
#define ACTION_ID_KEY_5 16
#define ACTION_ID_KEY_6 17
#define ACTION_ID_KEY_7 18
#define ACTION_ID_KEY_8 19
#define ACTION_ID_KEY_9 20
#define ACTION_ID_KEY_DOT 21
#define ACTION_ID_KEY_SIGN 22
#define ACTION_ID_KEY_BACK 23
#define ACTION_ID_KEY_C 24
#define ACTION_ID_KEY_OK 25
#define ACTION_ID_KEY_UNIT 26
#define PAGE_ID_MAIN 0
#define PAGE_ID_EDIT_WITH_SLIDER 1
#define PAGE_ID_EDIT_WITH_STEP 2
#define PAGE_ID_EDIT_WITH_KEYPAD 3
#define PAGE_ID_YES_NO 4
#define STYLE_ID_DEFAULT 0
#define STYLE_ID_SMALL 1
#define STYLE_ID_MON_VALUE 2
#define STYLE_ID_MON_VALUE_UR 3
#define STYLE_ID_MON_VALUE_MEDIUM 4
#define STYLE_ID_MON_VALUE_MEDIUM_UR 5
#define STYLE_ID_CHANNEL_OFF 6
#define STYLE_ID_CHANNEL_OFF_MEDIUM 7
#define STYLE_ID_PROT_INDICATOR 8
#define STYLE_ID_PROT_INDICATOR_SET 9
#define STYLE_ID_PROT_INDICATOR_TRIP 10
#define STYLE_ID_MENU 11
#define STYLE_ID_TAB_PAGE 12
#define STYLE_ID_TAB_PAGE_SELECTED 13
#define STYLE_ID_EXIT 14
#define STYLE_ID_KEY 15
#define STYLE_ID_KEY_ICONS 16
#define STYLE_ID_EDIT_INFO 17
#define STYLE_ID_EDIT_VALUE 18
#define STYLE_ID_EDIT_VALUE_UR 19
#define STYLE_ID_EDIT_VALUE_ACTIVE 20

typedef uint16_t OBJ_OFFSET;

struct List {
    uint8_t count;
    OBJ_OFFSET first;
};

struct ContainerWidget {
    List widgets;
};

struct ThreeStateIndicatorWidget {
    OBJ_OFFSET style1;
    OBJ_OFFSET style2;
    OBJ_OFFSET text;
};

struct SelectWidget {
    List widgets;
};

struct Document {
    List styles;
    List pages;
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

struct ListWidget {
    OBJ_OFFSET item_widget;
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

struct DisplayStringSelectWidget {
    OBJ_OFFSET style1;
    OBJ_OFFSET text1;
    OBJ_OFFSET style2;
    OBJ_OFFSET text2;
};

struct DisplayStringWidget {
    OBJ_OFFSET text;
};

#pragma pack(pop)

extern uint8_t document[];

}
}
} // namespace eez::psu::gui
