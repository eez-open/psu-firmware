/*
* EEZ PSU Firmware
* Copyright (C) 2017-present, Envox d.o.o.
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

#if OPTION_DISPLAY

#include "actions.h"
#include "event_queue.h"
#include "persist_conf.h"
#include "channel_dispatcher.h"
#include "gui_internal.h"
#include "gui_keypad.h"
#include "gui_edit_mode.h"
#include "gui_edit_mode_keypad.h"
#include "gui_calibration.h"
#include "gui_page_ch_settings_protection.h"
#include "gui_page_ch_settings_adv.h"
#include "gui_page_sys_settings.h"
#include "gui_page_user_profiles.h"
#include "gui_password.h"

namespace eez {
namespace psu {

using namespace gui;

void action_toggle_channel() {
    Channel& channel = Channel::get(g_foundWidgetAtDown.cursor.i);
    if (channel_dispatcher::isTripped(channel)) {
        errorMessageP(PSTR("Channel is tripped!"));
    } else {
        channel_dispatcher::outputEnable(channel, !channel.isOutputEnabled());
    }
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
    getActiveKeypad()->key();
}

void action_keypad_space() {
    getActiveKeypad()->space();
}

void action_keypad_back() {
    getActiveKeypad()->back();
}

void action_keypad_clear() {
    getActiveKeypad()->clear();
}

void action_keypad_caps() {
    getActiveKeypad()->caps();
}

void action_keypad_ok() {
    getActiveKeypad()->ok();
}

void action_keypad_cancel() {
    getActiveKeypad()->cancel();
}

void action_keypad_sign() {
    getActiveKeypad()->sign();
}

void action_keypad_unit() {
    getActiveKeypad()->unit();
}

void action_keypad_max() {
    getActiveKeypad()->setMaxValue();
}

void action_keypad_def() {
    getActiveKeypad()->setDefaultValue();
}

void action_touch_screen_calibration() {
    touch::calibration::enterCalibrationMode(PAGE_ID_SCREEN_CALIBRATION_YES_NO_CANCEL, PAGE_ID_SYS_SETTINGS2);
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

void action_turn_off() {
    // INTENTIONALLY BLANK IMPLEMENTATION
}

void action_show_previous_page() {
    popPage();
}

void action_show_main_page() {
    setPage(PAGE_ID_MAIN);
}

void action_show_event_queue() {
        setPage(PAGE_ID_EVENT_QUEUE);
}

void action_show_channel_settings() {
    gui::selectChannel();
    setPage(PAGE_ID_CH_SETTINGS_PROT);
}

void action_show_sys_settings() {
    setPage(PAGE_ID_SYS_SETTINGS);
}

void action_show_sys_settings2() {
    setPage(PAGE_ID_SYS_SETTINGS2);
}

void action_show_sys_settings_date_time() {
    pushPage(PAGE_ID_SYS_SETTINGS_DATE_TIME);
}

void action_show_sys_settings_cal() {
    pushPage(PAGE_ID_SYS_SETTINGS_CAL);
}

void action_show_sys_settings_cal_ch() {
    gui::selectChannel();
    pushPage(PAGE_ID_SYS_SETTINGS_CAL_CH);
}

void action_show_sys_settings_screen_calibration() {
    pushPage(PAGE_ID_SYS_SETTINGS_SCREEN_CALIBRATION);
}

void action_show_sys_settings_ethernet() {
    pushPage(PAGE_ID_SYS_SETTINGS_ETHERNET);
}

void action_show_sys_settings_protections() {
    pushPage(PAGE_ID_SYS_SETTINGS_PROTECTIONS);
}

void action_show_sys_settings_aux_otp() {
    pushPage(PAGE_ID_SYS_SETTINGS_AUX_OTP);
}

void action_show_sys_settings_sound() {
    pushPage(PAGE_ID_SYS_SETTINGS_SOUND);
}

void action_show_sys_settings_encoder() {
    pushPage(PAGE_ID_SYS_SETTINGS_ENCODER);
}

void action_show_sys_info() {
    setPage(PAGE_ID_SYS_INFO);
}

void action_show_sys_info2() {
    setPage(PAGE_ID_SYS_INFO2);
}

void action_show_main_help_page() {
    setPage(PAGE_ID_MAIN_HELP);
}

void action_show_edit_mode_step_help() {
    pushPage(PAGE_ID_EDIT_MODE_STEP_HELP);
}

void action_show_edit_mode_slider_help() {
    pushPage(PAGE_ID_EDIT_MODE_SLIDER_HELP);
}

void action_show_ch_settings_prot() {
    setPage(PAGE_ID_CH_SETTINGS_PROT);
}

void action_show_ch_settings_prot_clear() {
    setPage(PAGE_ID_CH_SETTINGS_PROT_CLEAR);
}

void action_show_ch_settings_prot_ocp() {
    pushPage(PAGE_ID_CH_SETTINGS_PROT_OCP);
}

void action_show_ch_settings_prot_ovp() {
    pushPage(PAGE_ID_CH_SETTINGS_PROT_OVP);
}

void action_show_ch_settings_prot_opp() {
    pushPage(PAGE_ID_CH_SETTINGS_PROT_OPP);
}

void action_show_ch_settings_prot_otp() {
    pushPage(PAGE_ID_CH_SETTINGS_PROT_OTP);
}

void action_show_ch_settings_adv() {
    setPage(PAGE_ID_CH_SETTINGS_ADV);
}

void action_show_ch_settings_adv_lripple() {
    pushPage(PAGE_ID_CH_SETTINGS_ADV_LRIPPLE);
}

void action_show_ch_settings_adv_rsense() {
    setPage(PAGE_ID_CH_SETTINGS_ADV_RSENSE);
}

void action_show_ch_settings_adv_rprog() {
    setPage(PAGE_ID_CH_SETTINGS_ADV_RPROG);
}

void action_show_ch_settings_adv_tracking() {
    setPage(PAGE_ID_CH_SETTINGS_ADV_TRACKING);
}

void action_show_ch_settings_adv_coupling() {
    setPage(PAGE_ID_CH_SETTINGS_ADV_COUPLING);
}

void action_show_ch_settings_info() {
    setPage(PAGE_ID_CH_SETTINGS_INFO);
}

void action_show_ch_settings_info_cal() {
    pushPage(PAGE_ID_SYS_SETTINGS_CAL_CH);
}

void action_sys_settings_cal_edit_password() {
    editCalibrationPassword();
}

void action_sys_settings_cal_ch_wiz_start() {
    gui::calibration::start();
}

void action_sys_settings_cal_ch_wiz_step_previous() {
    gui::calibration::previousStep();
}

void action_sys_settings_cal_ch_wiz_step_next() {
    gui::calibration::nextStep();
}

void action_sys_settings_cal_ch_wiz_stop_and_show_previous_page() {
    gui::calibration::stop();
    popPage();
}

void action_sys_settings_cal_ch_wiz_stop_and_show_main_page() {
    gui::calibration::stop();
    setPage(PAGE_ID_MAIN);
}

void action_sys_settings_cal_ch_wiz_step_set() {
    gui::calibration::set();
}

void action_sys_settings_cal_ch_wiz_save() {
    gui::calibration::save();
}

void action_sys_settings_cal_toggle_enable() {
    gui::calibration::toggleEnable();
}

void action_ch_settings_prot_clear() {
    ChSettingsProtectionPage::clear();
}

void action_ch_settings_prot_clear_and_disable() {
    ChSettingsProtectionPage::clearAndDisable();
}

void action_ch_settings_prot_toggle_state() {
    ((ChSettingsProtectionSetPage *)getActivePage())->toggleState();
}

void action_ch_settings_prot_edit_limit() {
    ((ChSettingsProtectionSetPage *)getActivePage())->editLimit();
}

void action_ch_settings_prot_edit_level() {
    ((ChSettingsProtectionSetPage *)getActivePage())->editLevel();
}

void action_ch_settings_prot_edit_delay() {
    ((ChSettingsProtectionSetPage *)getActivePage())->editDelay();
}

void action_set() {
    ((SetPage *)getActivePage())->set();
}

void action_discard() {
    ((SetPage *)getActivePage())->discard();
}

void action_edit_field() {
    ((SetPage *)getActivePage())->edit();
}

void action_event_queue_previous_page() {
    event_queue::moveToPreviousPage();
}

void action_event_queue_next_page() {
    event_queue::moveToNextPage();
}

void action_ch_settings_adv_lripple_toggle_status() {
    ((ChSettingsAdvLRipplePage *)getActivePage())->toggleStatus();
}

void action_ch_settings_adv_lripple_toggle_auto_mode() {
    ((ChSettingsAdvLRipplePage *)getActivePage())->toggleAutoMode();
}

void action_ch_settings_adv_rsense_toggle_status() {
    ((ChSettingsAdvRSensePage *)getActivePage())->toggleStatus();
}

void action_ch_settings_adv_rprog_toggle_status() {
    ((ChSettingsAdvRProgPage *)getActivePage())->toggleStatus();
}

void action_sys_settings_date_time_toggle_dst() {
    ((SysSettingsDateTimePage *)getActivePage())->toggleDst();
}

void action_show_user_profiles() {
    setPage(PAGE_ID_USER_PROFILES);
}

void action_show_user_profiles2() {
    setPage(PAGE_ID_USER_PROFILES2);
}

void action_show_user_profile_settings() {
    ((UserProfilesPage *)getActivePage())->showProfile();
}

void action_profiles_toggle_auto_recall() {
    ((UserProfilesPage *)getActivePage())->toggleAutoRecall();
}

void action_profile_toggle_is_auto_recall_location() {
    ((UserProfilesPage *)getActivePage())->toggleIsAutoRecallLocation();
}

void action_profile_recall() {
    ((UserProfilesPage *)getActivePage())->recall();
}

void action_profile_save() {
    ((UserProfilesPage *)getActivePage())->save();
}

void action_profile_delete() {
    ((UserProfilesPage *)getActivePage())->deleteProfile();
}

void action_profile_edit_remark() {
    ((UserProfilesPage *)getActivePage())->editRemark();
}

void action_toggle_channels_view_mode() {
    persist_conf::toggleChannelsViewMode();
}

void action_sys_settings_ethernet_enable() {
    SysSettingsEthernetPage::enable();
}

void action_sys_settings_ethernet_disable() {
    SysSettingsEthernetPage::disable();
}

void action_ch_settings_adv_coupling_uncouple() {
    ((ChSettingsAdvCouplingPage *)getActivePage())->uncouple();
}

void action_ch_settings_adv_coupling_set_parallel_info() {
    ((ChSettingsAdvCouplingPage *)getActivePage())->setParallelInfo();
}

void action_ch_settings_adv_coupling_set_series_info() {
    ((ChSettingsAdvCouplingPage *)getActivePage())->setSeriesInfo();
}

void action_ch_settings_adv_coupling_set_parallel() {
    ((ChSettingsAdvCouplingPage *)getActivePage())->setParallel();
}

void action_ch_settings_adv_coupling_set_series() {
    ((ChSettingsAdvCouplingPage *)getActivePage())->setSeries();
}

void action_ch_settings_adv_toggle_tracking_mode() {
    ((ChSettingsAdvTrackingPage *)getActivePage())->toggleTrackingMode();
}

void action_sys_settings_protections_toggle_output_protection_couple() {
    SysSettingsProtectionsPage::toggleOutputProtectionCouple();
}

void action_sys_settings_protections_toggle_shutdown_when_protection_tripped() {
    SysSettingsProtectionsPage::toggleShutdownWhenProtectionTripped();
}

void action_sys_settings_protections_toggle_force_disabling_all_outputs_on_power_up() {
    SysSettingsProtectionsPage::toggleForceDisablingAllOutputsOnPowerUp();
}

void action_sys_settings_protections_aux_otp_toggle_state() {
    ((SysSettingsAuxOtpPage *)getActivePage())->toggleState();
}

void action_sys_settings_protections_aux_otp_edit_level() {
    ((SysSettingsAuxOtpPage *)getActivePage())->editLevel();
}

void action_sys_settings_protections_aux_otp_edit_delay() {
    ((SysSettingsAuxOtpPage *)getActivePage())->editDelay();
}

void action_sys_settings_protections_aux_otp_clear() {
    SysSettingsAuxOtpPage::clear();
}

void action_on_last_error_event_action() {
    onLastErrorEventAction();
}

void action_edit_system_password() {
    editSystemPassword();
}

void action_sys_front_panel_lock() {
    // INTENTIONALLY BLANK IMPLEMENTATION
}

void action_sys_front_panel_unlock() {
    // INTENTIONALLY BLANK IMPLEMENTATION
}

void action_sys_settings_sound_toggle() {
    ((SysSettingsSoundPage *)getActivePage())->toggleSound();
}

void action_sys_settings_sound_toggle_click() {
    ((SysSettingsSoundPage *)getActivePage())->toggleClickSound();
}

void action_show_ch_settings_adv_view() {
    pushPage(PAGE_ID_CH_SETTINGS_ADV_VIEW);
}

void action_ch_settings_adv_view_edit_display_value1() {
    ((ChSettingsAdvViewPage *)getActivePage())->editDisplayValue1();
}

void action_ch_settings_adv_view_edit_display_value2() {
    ((ChSettingsAdvViewPage *)getActivePage())->editDisplayValue2();
}

void action_ch_settings_adv_view_swap_display_values() {
    ((ChSettingsAdvViewPage *)getActivePage())->swapDisplayValues();
}

void action_ch_settings_adv_view_edit_yt_view_rate() {
    ((ChSettingsAdvViewPage *)getActivePage())->editYTViewRate();
}

void action_select_enum_item() {
    ((SelectFromEnumPage *)getActivePage())->selectEnumItem();
}

void action_error_alert_action() {
    errorMessageAction();
}

void action_up_down() {
    upDown();
}

void action_sys_settings_encoder_toggle_confirmation_mode() {
    #if OPTION_ENCODER
    ((SysSettingsEncoderPage *)getActivePage())->toggleConfirmationMode();
    #endif
}


ACTION actions[] = {
    0,
    action_toggle_channel,
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
    action_turn_off,
    action_show_previous_page,
    action_show_main_page,
    action_show_event_queue,
    action_show_channel_settings,
    action_show_sys_settings,
    action_show_sys_settings2,
    action_show_sys_settings_date_time,
    action_show_sys_settings_cal,
    action_show_sys_settings_cal_ch,
    action_show_sys_settings_screen_calibration,
    action_show_sys_settings_ethernet,
    action_show_sys_settings_protections,
    action_show_sys_settings_aux_otp,
    action_show_sys_settings_sound,
    action_show_sys_settings_encoder,
    action_show_sys_info,
    action_show_sys_info2,
    action_show_main_help_page,
    action_show_edit_mode_step_help,
    action_show_edit_mode_slider_help,
    action_show_ch_settings_prot,
    action_show_ch_settings_prot_clear,
    action_show_ch_settings_prot_ocp,
    action_show_ch_settings_prot_ovp,
    action_show_ch_settings_prot_opp,
    action_show_ch_settings_prot_otp,
    action_show_ch_settings_adv,
    action_show_ch_settings_adv_lripple,
    action_show_ch_settings_adv_rsense,
    action_show_ch_settings_adv_rprog,
    action_show_ch_settings_adv_tracking,
    action_show_ch_settings_adv_coupling,
    action_show_ch_settings_info,
    action_show_ch_settings_info_cal,
    action_sys_settings_cal_edit_password,
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
    action_set,
    action_discard,
    action_edit_field,
    action_event_queue_previous_page,
    action_event_queue_next_page,
    action_ch_settings_adv_lripple_toggle_status,
    action_ch_settings_adv_lripple_toggle_auto_mode,
    action_ch_settings_adv_rsense_toggle_status,
    action_ch_settings_adv_rprog_toggle_status,
    action_sys_settings_date_time_toggle_dst,
    action_show_user_profiles,
    action_show_user_profiles2,
    action_show_user_profile_settings,
    action_profiles_toggle_auto_recall,
    action_profile_toggle_is_auto_recall_location,
    action_profile_recall,
    action_profile_save,
    action_profile_delete,
    action_profile_edit_remark,
    action_toggle_channels_view_mode,
    action_sys_settings_ethernet_enable,
    action_sys_settings_ethernet_disable,
    action_ch_settings_adv_coupling_uncouple,
    action_ch_settings_adv_coupling_set_parallel_info,
    action_ch_settings_adv_coupling_set_series_info,
    action_ch_settings_adv_coupling_set_parallel,
    action_ch_settings_adv_coupling_set_series,
    action_ch_settings_adv_toggle_tracking_mode,
    action_sys_settings_protections_toggle_output_protection_couple,
    action_sys_settings_protections_toggle_shutdown_when_protection_tripped,
    action_sys_settings_protections_toggle_force_disabling_all_outputs_on_power_up,
    action_sys_settings_protections_aux_otp_toggle_state,
    action_sys_settings_protections_aux_otp_edit_level,
    action_sys_settings_protections_aux_otp_edit_delay,
    action_sys_settings_protections_aux_otp_clear,
    action_on_last_error_event_action,
    action_edit_system_password,
    action_sys_front_panel_lock,
    action_sys_front_panel_unlock,
    action_sys_settings_sound_toggle,
    action_sys_settings_sound_toggle_click,
    action_show_ch_settings_adv_view,
    action_ch_settings_adv_view_edit_display_value1,
    action_ch_settings_adv_view_edit_display_value2,
    action_ch_settings_adv_view_swap_display_values,
    action_ch_settings_adv_view_edit_yt_view_rate,
    action_select_enum_item,
    action_error_alert_action,
    action_up_down,
    action_sys_settings_encoder_toggle_confirmation_mode
};

}
} // namespace eez::psu

#endif