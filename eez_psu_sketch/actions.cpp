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
#include "event_queue.h"
#include "gui_internal.h"
#include "gui_keypad.h"
#include "gui_edit_mode.h"
#include "gui_edit_mode_keypad.h"
#include "gui_calibration.h"
#include "gui_protection.h"

namespace eez {
namespace psu {

using namespace gui;

void action_toggle_channel() {
    Channel& channel = Channel::get(g_foundWidgetAtDown.cursor.iChannel);
    if (channel.isTripped()) {
        errorMessageP(PSTR("Channel is tripped!"), 0);
    } else {
        channel.outputEnable(!channel.isOutputEnabled());
    }
}

void action_show_channel_settings() {
    gui::selectChannel();
    showPage(PAGE_ID_CH_SETTINGS_PROT);
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

void action_keypad_key() {
    keypad::key();
}

void action_keypad_space() {
    keypad::space();
}

void action_keypad_back() {
    keypad::back();
}

void action_keypad_clear() {
    keypad::clear();
}

void action_keypad_caps() {
    keypad::caps();
}

void action_keypad_ok() {
    keypad::ok();
}

void action_keypad_cancel() {
    keypad::cancel();
}

void action_keypad_sign() {
    keypad::sign();
}

void action_keypad_unit() {
    keypad::unit();
}

void action_keypad_max() {
    keypad::setMaxValue();
}

void action_keypad_def() {
    keypad::setDefaultValue();
}

void action_touch_screen_calibration() {
    touch::calibration::enterCalibrationMode();
}

void action_yes() {
    dialogYes();
}

void action_no() {
    dialogNo();
}

void action_ok() {
    dialogOk();
}

void action_cancel() {
    dialogCancel();
}

void action_show_previous_page() {
    showPreviousPage();
}

void action_turn_off() {

}

void action_show_sys_settings() {
    showPage(PAGE_ID_SYS_SETTINGS);
}

void action_show_main_help_page() {
    showPage(PAGE_ID_MAIN_HELP);
}

void action_show_ch_settings_prot() {
    showPage(PAGE_ID_CH_SETTINGS_PROT);
}

void action_show_ch_settings_prot_clear() {
    showPage(PAGE_ID_CH_SETTINGS_PROT_CLEAR);
}

void action_show_ch_settings_prot_ocp() {
    protection::editOCP();
}

void action_show_ch_settings_prot_ovp() {
    protection::editOVP();
}

void action_show_ch_settings_prot_opp() {
    protection::editOPP();
}

void action_show_ch_settings_prot_otp() {
    protection::editOTP();
}

void action_show_ch_settings_adv() {
    showPage(PAGE_ID_CH_SETTINGS_ADV);
}

void action_show_ch_settings_adv_lripple() {
    showPage(PAGE_ID_CH_SETTINGS_ADV_LRIPPLE);
}

void action_show_ch_settings_adv_limits() {
    showPage(PAGE_ID_CH_SETTINGS_ADV_LIMITS);
}

void action_show_ch_settings_adv_rsense() {
    showPage(PAGE_ID_CH_SETTINGS_ADV_RSENSE);
}

void action_show_ch_settings_adv_rprog() {
    showPage(PAGE_ID_CH_SETTINGS_ADV_RPROG);
}

void action_show_ch_settings_disp() {
    showPage(PAGE_ID_CH_SETTINGS_DISP);
}

void action_show_ch_settings_info() {
    showPage(PAGE_ID_CH_SETTINGS_INFO);
}

void action_show_sys_settings_cal() {
    showPage(PAGE_ID_SYS_SETTINGS_CAL);
}

void action_show_sys_settings_cal_ch() {
    gui::selectChannel();
    showPage(PAGE_ID_SYS_SETTINGS_CAL_CH);
}

void action_sys_settings_cal_edit_password() {
    calibration::editPassword();
}

void action_sys_settings_cal_ch_params_enabled() {

}

void action_sys_settings_cal_ch_wiz_start() {
    calibration::start();
}

void action_sys_settings_cal_ch_wiz_step_previous() {
    calibration::previousStep();
}

void action_sys_settings_cal_ch_wiz_step_next() {
    calibration::nextStep();
}

void action_sys_settings_cal_ch_wiz_stop_and_show_previous_page() {
    calibration::stop();
    showPreviousPage();
}

void action_sys_settings_cal_ch_wiz_stop_and_show_main_page() {
    calibration::stop();
    showPage(PAGE_ID_MAIN);
}

void action_sys_settings_cal_ch_wiz_step_set() {
    calibration::set();
}

void action_sys_settings_cal_ch_wiz_save() {
    calibration::save();
}

void action_sys_settings_cal_toggle_enable() {
    calibration::toggleEnable();
}

void action_ch_settings_prot_clear() {
    protection::clear();
}

void action_ch_settings_prot_clear_and_disable() {
    protection::clearAndDisable();
}

void action_ch_settings_prot_toggle_state() {
    protection::toggleState();
}

void action_ch_settings_prot_edit_limit() {
    protection::editLimit();
}

void action_ch_settings_prot_edit_level() {
    protection::editLevel();
}

void action_ch_settings_prot_edit_delay() {
    protection::editDelay();
}

void action_ch_settings_prot_set() {
    protection::set();
}

void action_ch_settings_prot_discard() {
    protection::discard();
}

void action_show_event_queue() {
    showPage(PAGE_ID_EVENT_QUEUE);
}

void action_event_queue_previous_page() {
    event_queue::moveToPreviousPage();
}

void action_event_queue_next_page() {
    event_queue::moveToNextPage();
}


ACTION actions[] = {
    0,
    action_toggle_channel,
    action_show_channel_settings,
    action_show_main_page,
    action_edit,
    action_edit_mode_slider,
    action_edit_mode_step,
    action_edit_mode_keypad,
    action_exit_edit_mode,
    action_toggle_interactive_mode,
    action_non_interactive_enter,
    action_non_interactive_discard,
    action_keypad_key,
    action_keypad_space,
    action_keypad_back,
    action_keypad_clear,
    action_keypad_caps,
    action_keypad_ok,
    action_keypad_cancel,
    action_keypad_sign,
    action_keypad_unit,
    action_keypad_max,
    action_keypad_def,
    action_touch_screen_calibration,
    action_yes,
    action_no,
    action_ok,
    action_cancel,
    action_show_previous_page,
    action_turn_off,
    action_show_sys_settings,
    action_show_main_help_page,
    action_show_ch_settings_prot,
    action_show_ch_settings_prot_clear,
    action_show_ch_settings_prot_ocp,
    action_show_ch_settings_prot_ovp,
    action_show_ch_settings_prot_opp,
    action_show_ch_settings_prot_otp,
    action_show_ch_settings_adv,
    action_show_ch_settings_adv_lripple,
    action_show_ch_settings_adv_limits,
    action_show_ch_settings_adv_rsense,
    action_show_ch_settings_adv_rprog,
    action_show_ch_settings_disp,
    action_show_ch_settings_info,
    action_show_sys_settings_cal,
    action_show_sys_settings_cal_ch,
    action_sys_settings_cal_edit_password,
    action_sys_settings_cal_ch_params_enabled,
    action_sys_settings_cal_ch_wiz_start,
    action_sys_settings_cal_ch_wiz_step_previous,
    action_sys_settings_cal_ch_wiz_step_next,
    action_sys_settings_cal_ch_wiz_stop_and_show_previous_page,
    action_sys_settings_cal_ch_wiz_stop_and_show_main_page,
    action_sys_settings_cal_ch_wiz_step_set,
    action_sys_settings_cal_ch_wiz_save,
    action_sys_settings_cal_toggle_enable,
    action_ch_settings_prot_clear,
    action_ch_settings_prot_clear_and_disable,
    action_ch_settings_prot_toggle_state,
    action_ch_settings_prot_edit_limit,
    action_ch_settings_prot_edit_level,
    action_ch_settings_prot_edit_delay,
    action_ch_settings_prot_set,
    action_ch_settings_prot_discard,
    action_show_event_queue,
    action_event_queue_previous_page,
    action_event_queue_next_page
};

}
} // namespace eez::psu::gui
