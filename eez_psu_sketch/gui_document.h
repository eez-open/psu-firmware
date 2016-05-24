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
* along with this program.  If not, see http://www.gnu.org/licenses.
*/

#pragma once

namespace eez {
namespace psu {
namespace gui {

enum DataEnum {
    DATA_ID_NONE,
    DATA_ID_CHANNELS,
    DATA_ID_CHANNEL_OK,
    DATA_ID_OUTPUT_STATE,
    DATA_ID_OUTPUT_MODE,
    DATA_ID_MON_VALUE,
    DATA_ID_VOLT,
    DATA_ID_CURR,
    DATA_ID_OVP,
    DATA_ID_OCP,
    DATA_ID_OPP,
    DATA_ID_OTP,
    DATA_ID_DP,
    DATA_ID_ALERT_MESSAGE,
    DATA_ID_EDIT_VALUE,
    DATA_ID_EDIT_UNIT,
    DATA_ID_EDIT_INFO,
    DATA_ID_EDIT_MODE_INTERACTIVE_MODE_SELECTOR,
    DATA_ID_EDIT_STEPS,
    DATA_ID_MODEL_INFO,
    DATA_ID_FIRMWARE_INFO
};

enum ActionsEnum {
    ACTION_ID_NONE,
    ACTION_ID_EDIT,
    ACTION_ID_EDIT_MODE_SLIDER,
    ACTION_ID_EDIT_MODE_STEP,
    ACTION_ID_EDIT_MODE_KEYPAD,
    ACTION_ID_EXIT,
    ACTION_ID_TOGGLE_CHANNEL,
    ACTION_ID_TOUCH_SCREEN_CALIBRATION,
    ACTION_ID_YES,
    ACTION_ID_NO,
    ACTION_ID_CANCEL,
    ACTION_ID_KEY_0,
    ACTION_ID_KEY_1,
    ACTION_ID_KEY_2,
    ACTION_ID_KEY_3,
    ACTION_ID_KEY_4,
    ACTION_ID_KEY_5,
    ACTION_ID_KEY_6,
    ACTION_ID_KEY_7,
    ACTION_ID_KEY_8,
    ACTION_ID_KEY_9,
    ACTION_ID_KEY_DOT,
    ACTION_ID_KEY_SIGN,
    ACTION_ID_KEY_BACK,
    ACTION_ID_KEY_C,
    ACTION_ID_KEY_OK,
    ACTION_ID_KEY_UNIT,
    ACTION_ID_TOGGLE,
    ACTION_ID_NON_INTERACTIVE_ENTER,
    ACTION_ID_NON_INTERACTIVE_CANCEL,
    ACTION_ID_CHANNEL_SETTINGS,
    ACTION_ID_TURN_OFF
};

enum FontsEnum {
    FONT_ID_NONE,
    FONT_ID_SMALL,
    FONT_ID_MEDIUM,
    FONT_ID_LARGE,
    FONT_ID_ICONS
};

enum BitmapsEnum {
    BITMAP_ID_NONE,
    BITMAP_ID_LOGO
};

enum StylesEnum {
    STYLE_ID_DEFAULT,
    STYLE_ID_SMALL,
    STYLE_ID_LARGE,
    STYLE_ID_MON_VALUE,
    STYLE_ID_MON_VALUE_UR,
    STYLE_ID_MON_VALUE_MEDIUM,
    STYLE_ID_MON_VALUE_MEDIUM_UR,
    STYLE_ID_CHANNEL_OFF,
    STYLE_ID_CHANNEL_ERROR,
    STYLE_ID_CHANNEL_OFF_MEDIUM,
    STYLE_ID_PROT_INDICATOR,
    STYLE_ID_PROT_INDICATOR_SET,
    STYLE_ID_PROT_INDICATOR_TRIP,
    STYLE_ID_MENU,
    STYLE_ID_TAB_PAGE,
    STYLE_ID_TAB_PAGE_SELECTED,
    STYLE_ID_BOTTOM_BUTTON,
    STYLE_ID_BOTTOM_BUTTON_DISABLED,
    STYLE_ID_KEY,
    STYLE_ID_KEY_ICONS,
    STYLE_ID_EDIT_INFO,
    STYLE_ID_EDIT_VALUE_LARGE,
    STYLE_ID_EDIT_VALUE,
    STYLE_ID_EDIT_VALUE_UR,
    STYLE_ID_EDIT_VALUE_ACTIVE,
    STYLE_ID_EDIT_VALUE_SMALL,
    STYLE_ID_EDIT_VALUE_UR_SMALL,
    STYLE_ID_EDIT_VALUE_ACTIVE_SMALL,
    STYLE_ID_NON_INTERACTIVE_BUTTON,
    STYLE_ID_EDIT_MODE_SLIDER_SCALE,
    STYLE_ID_EDIT_MODE_STEP_VERTICAL_SLIDER,
    STYLE_ID_TOP_BAR
};

enum PagesEnum {
    PAGE_ID_WELCOME,
    PAGE_ID_ENTERING_STANDBY,
    PAGE_ID_STANDBY,
    PAGE_ID_MAIN,
    PAGE_ID_EDIT_MODE_SLIDER,
    PAGE_ID_EDIT_MODE_STEP,
    PAGE_ID_EDIT_MODE_KEYPAD,
    PAGE_ID_YES_NO
};

extern const uint8_t *fonts[];

struct Bitmap {
    uint16_t w;
    uint16_t h;
    const uint8_t *pixels PROGMEM;
};

extern Bitmap bitmaps[];

extern const uint8_t document[];

}
}
} // namespace eez::psu::gui
