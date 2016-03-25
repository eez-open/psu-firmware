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
#define WIDGET_TYPE_DISPLAY_MULTILINE_STRING 7
#define WIDGET_TYPE_SCALE 8
#define WIDGET_TYPE_TOGGLE_BUTTON 9
#define WIDGET_TYPE_BUTTON_GROUP 10
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
#define DATA_ID_EDIT_VALUE 13
#define DATA_ID_EDIT_UNIT 14
#define DATA_ID_EDIT_INFO 15
#define DATA_ID_EDIT_MODE_INTERACTIVE_MODE_SELECTOR 16
#define DATA_ID_EDIT_STEPS 17
#define DATA_ID_MODEL_INFO 18
#define DATA_ID_FIRMWARE_INFO 19
#define ACTION_ID_EDIT 1
#define ACTION_ID_EDIT_MODE_SLIDER 2
#define ACTION_ID_EDIT_MODE_STEP 3
#define ACTION_ID_EDIT_MODE_KEYPAD 4
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
#define ACTION_ID_TOGGLE 27
#define ACTION_ID_NON_INTERACTIVE_ENTER 28
#define ACTION_ID_NON_INTERACTIVE_CANCEL 29
#define ACTION_ID_CHANNEL_SETTINGS 30
#define ACTION_ID_TURN_OFF 31
#define PAGE_ID_WELCOME 0
#define PAGE_ID_ENTERING_STANDBY 1
#define PAGE_ID_STANDBY 2
#define PAGE_ID_MAIN 3
#define PAGE_ID_EDIT_MODE_SLIDER 4
#define PAGE_ID_EDIT_MODE_STEP 5
#define PAGE_ID_EDIT_MODE_KEYPAD 6
#define PAGE_ID_YES_NO 7
#define STYLE_ID_DEFAULT 0
#define STYLE_ID_SMALL 1
#define STYLE_ID_LARGE 2
#define STYLE_ID_MON_VALUE 3
#define STYLE_ID_MON_VALUE_UR 4
#define STYLE_ID_MON_VALUE_MEDIUM 5
#define STYLE_ID_MON_VALUE_MEDIUM_UR 6
#define STYLE_ID_CHANNEL_OFF 7
#define STYLE_ID_CHANNEL_OFF_MEDIUM 8
#define STYLE_ID_PROT_INDICATOR 9
#define STYLE_ID_PROT_INDICATOR_SET 10
#define STYLE_ID_PROT_INDICATOR_TRIP 11
#define STYLE_ID_MENU 12
#define STYLE_ID_TAB_PAGE 13
#define STYLE_ID_TAB_PAGE_SELECTED 14
#define STYLE_ID_BOTTOM_BUTTON 15
#define STYLE_ID_KEY 16
#define STYLE_ID_KEY_ICONS 17
#define STYLE_ID_EDIT_INFO 18
#define STYLE_ID_EDIT_VALUE_LARGE 19
#define STYLE_ID_EDIT_VALUE 20
#define STYLE_ID_EDIT_VALUE_UR 21
#define STYLE_ID_EDIT_VALUE_ACTIVE 22
#define STYLE_ID_EDIT_VALUE_SMALL 23
#define STYLE_ID_EDIT_VALUE_UR_SMALL 24
#define STYLE_ID_EDIT_VALUE_ACTIVE_SMALL 25
#define STYLE_ID_NON_INTERACTIVE_BUTTON 26
#define STYLE_ID_EDIT_MODE_SLIDER_SCALE 27
#define STYLE_ID_EDIT_MODE_STEP_VERTICAL_SLIDER 28
#define STYLE_ID_TOP_BAR 29

typedef uint16_t OBJ_OFFSET;

struct List {
    uint8_t count;
    OBJ_OFFSET first;
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

struct ScaleWidget {
    uint16_t ticks_color;
    uint8_t needle_height;
};

struct ListWidget {
    OBJ_OFFSET item_widget;
};

struct ToggleButtonWidget {
    OBJ_OFFSET text1;
    OBJ_OFFSET text2;
};

struct SelectWidget {
    List widgets;
};

struct DisplayStringWidget {
    OBJ_OFFSET text;
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

struct Document {
    List styles;
    List pages;
};

struct ContainerWidget {
    List widgets;
};

#pragma pack(pop)

extern const uint8_t document[] PROGMEM;

}
}
} // namespace eez::psu::gui
