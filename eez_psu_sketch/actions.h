/*
* EEZ PSU Firmware
* Copyright (C) 2016-present, Envox d.o.o.
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

enum ActionsEnum {
    ACTION_ID_NONE,
    ACTION_ID_TOGGLE_CHANNEL,
    ACTION_ID_SHOW_CHANNEL_SETTINGS,
    ACTION_ID_SHOW_MAIN_PAGE,
    ACTION_ID_EDIT,
    ACTION_ID_EDIT_MODE_SLIDER,
    ACTION_ID_EDIT_MODE_STEP,
    ACTION_ID_EDIT_MODE_KEYPAD,
    ACTION_ID_EXIT_EDIT_MODE,
    ACTION_ID_TOGGLE_INTERACTIVE_MODE,
    ACTION_ID_NON_INTERACTIVE_ENTER,
    ACTION_ID_NON_INTERACTIVE_DISCARD,
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
    ACTION_ID_TOUCH_SCREEN_CALIBRATION,
    ACTION_ID_YES,
    ACTION_ID_NO,
    ACTION_ID_CANCEL,
    ACTION_ID_TURN_OFF,
    ACTION_ID_SHOW_SYS_SETTINGS,
    ACTION_ID_SHOW_MAIN_HELP_PAGE,
    ACTION_ID_SHOW_CH_SETTINGS,
    ACTION_ID_SHOW_CH_SETTINGS_PROT,
    ACTION_ID_SHOW_CH_SETTINGS_PROT_CLEAR,
    ACTION_ID_SHOW_CH_SETTINGS_PROT_OCP,
    ACTION_ID_SHOW_CH_SETTINGS_PROT_OVP,
    ACTION_ID_SHOW_CH_SETTINGS_PROT_OPP,
    ACTION_ID_SHOW_CH_SETTINGS_PROT_OTP,
    ACTION_ID_SHOW_CH_SETTINGS_ADV,
    ACTION_ID_SHOW_CH_SETTINGS_ADV_LRIPPLE,
    ACTION_ID_SHOW_CH_SETTINGS_ADV_LIMITS,
    ACTION_ID_SHOW_CH_SETTINGS_ADV_RSENSE,
    ACTION_ID_SHOW_CH_SETTINGS_ADV_RPROG,
    ACTION_ID_SHOW_CH_SETTINGS_DISP,
    ACTION_ID_SHOW_CH_SETTINGS_INFO
};

typedef void (*ACTION)();

extern ACTION actions[];

}
} // namespace eez::psu::gui
