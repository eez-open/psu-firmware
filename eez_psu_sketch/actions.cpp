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

#include "psu.h"
#include "actions.h"
#include "gui_internal.h"
#include "gui_edit_mode.h"
#include "gui_edit_mode_keypad.h"

namespace eez {
namespace psu {

using namespace gui;

void action_toggle_channel() {
    Channel& channel = Channel::get(found_widget_at_down.cursor.iChannel);
    channel.outputEnable(!channel .isOutputEnabled());
}

void action_channel_settings() {
    showPage(PAGE_ID_CHANNEL_SETTINGS);
}

void action_show_main_page() {
    showPage(PAGE_ID_MAIN);
}

void action_edit() {
    edit_mode::enter();
}

void action_edit_mode_slider() {
    edit_mode::enter(PAGE_ID_EDIT_MODE_SLIDER);
}

void action_edit_mode_step() {
    edit_mode::enter(PAGE_ID_EDIT_MODE_STEP);
}

void action_edit_mode_keypad() {
    edit_mode::enter(PAGE_ID_EDIT_MODE_KEYPAD);
}

void action_exit_edit_mode() {
    if (edit_mode::isActive()) {
        edit_mode::exit();
    }
}

void action_toggle_interactive_mode() {
    edit_mode::toggleInteractiveMode();
}

void action_non_interactive_enter() {
    edit_mode::nonInteractiveSet();
}

void action_non_interactive_discard() {
    edit_mode::nonInteractiveDiscard();
}

void action_key_0() {
    edit_mode_keypad::digit(0);
}

void action_key_1() {
    edit_mode_keypad::digit(1);
}

void action_key_2() {
    edit_mode_keypad::digit(2);
}

void action_key_3() {
    edit_mode_keypad::digit(3);
}

void action_key_4() {
    edit_mode_keypad::digit(4);
}

void action_key_5() {
    edit_mode_keypad::digit(5);
}

void action_key_6() {
    edit_mode_keypad::digit(6);
}

void action_key_7() {
    edit_mode_keypad::digit(7);
}

void action_key_8() {
    edit_mode_keypad::digit(8);
}

void action_key_9() {
    edit_mode_keypad::digit(9);
}

void action_key_dot() {
    edit_mode_keypad::dot();
}

void action_key_sign() {
    edit_mode_keypad::sign();
}

void action_key_back() {
    edit_mode_keypad::back();
}

void action_key_c() {
    edit_mode_keypad::clear();
}

void action_key_ok() {
    edit_mode_keypad::ok();
}

void action_key_unit() {
    edit_mode_keypad::unit();
}

void action_touch_screen_calibration() {
    touch::calibration::enterCalibrationMode();
}

void action_yes() {
    dialog_yes_callback();
}

void action_no() {
    dialog_no_callback();
}

void action_cancel() {
    dialog_cancel_callback();
}

void action_turn_off() {

}


ACTION actions[] = {
    0,
    action_toggle_channel,
    action_channel_settings,
    action_show_main_page,
    action_edit,
    action_edit_mode_slider,
    action_edit_mode_step,
    action_edit_mode_keypad,
    action_exit_edit_mode,
    action_toggle_interactive_mode,
    action_non_interactive_enter,
    action_non_interactive_discard,
    action_key_0,
    action_key_1,
    action_key_2,
    action_key_3,
    action_key_4,
    action_key_5,
    action_key_6,
    action_key_7,
    action_key_8,
    action_key_9,
    action_key_dot,
    action_key_sign,
    action_key_back,
    action_key_c,
    action_key_ok,
    action_key_unit,
    action_touch_screen_calibration,
    action_yes,
    action_no,
    action_cancel,
    action_turn_off
};

}
} // namespace eez::psu::gui
