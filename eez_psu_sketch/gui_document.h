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

#pragma once

#include "mw_gui_data.h"

namespace eez {
namespace app {
namespace gui {

enum DataEnum {
    DATA_ID_NONE,
    DATA_ID_EDIT_ENABLED,
    DATA_ID_CHANNELS,
    DATA_ID_CHANNEL_STATUS,
    DATA_ID_CHANNEL_OUTPUT_STATE,
    DATA_ID_CHANNEL_OUTPUT_MODE,
    DATA_ID_CHANNEL_MON_VALUE,
    DATA_ID_CHANNEL_U_SET,
    DATA_ID_CHANNEL_U_MON,
    DATA_ID_CHANNEL_U_MON_DAC,
    DATA_ID_CHANNEL_U_LIMIT,
    DATA_ID_CHANNEL_U_EDIT,
    DATA_ID_CHANNEL_I_SET,
    DATA_ID_CHANNEL_I_MON,
    DATA_ID_CHANNEL_I_MON_DAC,
    DATA_ID_CHANNEL_I_LIMIT,
    DATA_ID_CHANNEL_I_EDIT,
    DATA_ID_CHANNEL_P_MON,
    DATA_ID_CHANNELS_VIEW_MODE,
    DATA_ID_CHANNEL_DISPLAY_VALUE1,
    DATA_ID_CHANNEL_DISPLAY_VALUE2,
    DATA_ID_LRIP,
    DATA_ID_OVP,
    DATA_ID_OCP,
    DATA_ID_OPP,
    DATA_ID_OTP_CH,
    DATA_ID_OTP_AUX,
    DATA_ID_ALERT_MESSAGE,
    DATA_ID_ALERT_MESSAGE_2,
    DATA_ID_ALERT_MESSAGE_3,
    DATA_ID_EDIT_VALUE,
    DATA_ID_EDIT_UNIT,
    DATA_ID_EDIT_INFO,
    DATA_ID_EDIT_INFO1,
    DATA_ID_EDIT_INFO2,
    DATA_ID_EDIT_MODE_INTERACTIVE_MODE_SELECTOR,
    DATA_ID_EDIT_STEPS,
    DATA_ID_MODEL_INFO,
    DATA_ID_FIRMWARE_INFO,
    DATA_ID_SELF_TEST_RESULT,
    DATA_ID_KEYPAD_TEXT,
    DATA_ID_KEYPAD_CAPS,
    DATA_ID_KEYPAD_OPTION1_TEXT,
    DATA_ID_KEYPAD_OPTION1_ENABLED,
    DATA_ID_KEYPAD_OPTION2_TEXT,
    DATA_ID_KEYPAD_OPTION2_ENABLED,
    DATA_ID_KEYPAD_SIGN_ENABLED,
    DATA_ID_KEYPAD_DOT_ENABLED,
    DATA_ID_KEYPAD_UNIT_ENABLED,
    DATA_ID_CHANNEL_LABEL,
    DATA_ID_CHANNEL_SHORT_LABEL,
    DATA_ID_CHANNEL_TEMP_STATUS,
    DATA_ID_CHANNEL_TEMP,
    DATA_ID_CHANNEL_ON_TIME_TOTAL,
    DATA_ID_CHANNEL_ON_TIME_LAST,
    DATA_ID_CHANNEL_CALIBRATION_STATUS,
    DATA_ID_CHANNEL_CALIBRATION_STATE,
    DATA_ID_CHANNEL_CALIBRATION_DATE,
    DATA_ID_CHANNEL_CALIBRATION_REMARK,
    DATA_ID_CHANNEL_CALIBRATION_STEP_IS_SET_REMARK_STEP,
    DATA_ID_CHANNEL_CALIBRATION_STEP_NUM,
    DATA_ID_CHANNEL_CALIBRATION_STEP_STATUS,
    DATA_ID_CHANNEL_CALIBRATION_STEP_LEVEL_VALUE,
    DATA_ID_CHANNEL_CALIBRATION_STEP_VALUE,
    DATA_ID_CHANNEL_CALIBRATION_STEP_PREV_ENABLED,
    DATA_ID_CHANNEL_CALIBRATION_STEP_NEXT_ENABLED,
    DATA_ID_CAL_CH_U_MIN,
    DATA_ID_CAL_CH_U_MID,
    DATA_ID_CAL_CH_U_MAX,
    DATA_ID_CAL_CH_I0_MIN,
    DATA_ID_CAL_CH_I0_MID,
    DATA_ID_CAL_CH_I0_MAX,
    DATA_ID_CAL_CH_I1_MIN,
    DATA_ID_CAL_CH_I1_MID,
    DATA_ID_CAL_CH_I1_MAX,
    DATA_ID_CHANNEL_PROTECTION_OVP_STATE,
    DATA_ID_CHANNEL_PROTECTION_OVP_LEVEL,
    DATA_ID_CHANNEL_PROTECTION_OVP_DELAY,
    DATA_ID_CHANNEL_PROTECTION_OVP_LIMIT,
    DATA_ID_CHANNEL_PROTECTION_OCP_STATE,
    DATA_ID_CHANNEL_PROTECTION_OCP_DELAY,
    DATA_ID_CHANNEL_PROTECTION_OCP_LIMIT,
    DATA_ID_CHANNEL_PROTECTION_OCP_MAX_CURRENT_LIMIT_CAUSE,
    DATA_ID_CHANNEL_PROTECTION_OPP_STATE,
    DATA_ID_CHANNEL_PROTECTION_OPP_LEVEL,
    DATA_ID_CHANNEL_PROTECTION_OPP_DELAY,
    DATA_ID_CHANNEL_PROTECTION_OPP_LIMIT,
    DATA_ID_CHANNEL_PROTECTION_OTP_INSTALLED,
    DATA_ID_CHANNEL_PROTECTION_OTP_STATE,
    DATA_ID_CHANNEL_PROTECTION_OTP_LEVEL,
    DATA_ID_CHANNEL_PROTECTION_OTP_DELAY,
    DATA_ID_EVENT_QUEUE_LAST_EVENT_TYPE,
    DATA_ID_EVENT_QUEUE_LAST_EVENT_MESSAGE,
    DATA_ID_EVENT_QUEUE_EVENTS,
    DATA_ID_EVENT_QUEUE_EVENTS_TYPE,
    DATA_ID_EVENT_QUEUE_EVENTS_MESSAGE,
    DATA_ID_EVENT_QUEUE_MULTIPLE_PAGES,
    DATA_ID_EVENT_QUEUE_PREVIOUS_PAGE_ENABLED,
    DATA_ID_EVENT_QUEUE_NEXT_PAGE_ENABLED,
    DATA_ID_EVENT_QUEUE_PAGE_INFO,
    DATA_ID_CHANNEL_LRIPPLE_MAX_DISSIPATION,
    DATA_ID_CHANNEL_LRIPPLE_CALCULATED_DISSIPATION,
    DATA_ID_CHANNEL_LRIPPLE_AUTO_MODE,
    DATA_ID_CHANNEL_LRIPPLE_IS_ALLOWED,
    DATA_ID_CHANNEL_LRIPPLE_STATUS,
    DATA_ID_CHANNEL_RSENSE_STATUS,
    DATA_ID_CHANNEL_RPROG_INSTALLED,
    DATA_ID_CHANNEL_RPROG_STATUS,
    DATA_ID_CHANNEL_IS_COUPLED,
    DATA_ID_CHANNEL_IS_TRACKED,
    DATA_ID_CHANNEL_IS_COUPLED_OR_TRACKED,
    DATA_ID_CHANNEL_COUPLING_IS_ALLOWED,
    DATA_ID_CHANNEL_COUPLING_MODE,
    DATA_ID_CHANNEL_COUPLING_SELECTED_MODE,
    DATA_ID_CHANNEL_COUPLING_IS_SERIES,
    DATA_ID_SYS_ON_TIME_TOTAL,
    DATA_ID_SYS_ON_TIME_LAST,
    DATA_ID_SYS_TEMP_AUX_STATUS,
    DATA_ID_SYS_TEMP_AUX_OTP_STATE,
    DATA_ID_SYS_TEMP_AUX_OTP_LEVEL,
    DATA_ID_SYS_TEMP_AUX_OTP_DELAY,
    DATA_ID_SYS_TEMP_AUX_OTP_IS_TRIPPED,
    DATA_ID_SYS_TEMP_AUX,
    DATA_ID_SYS_INFO_FIRMWARE_VER,
    DATA_ID_SYS_INFO_SERIAL_NO,
    DATA_ID_SYS_INFO_SCPI_VER,
    DATA_ID_SYS_INFO_CPU,
    DATA_ID_SYS_INFO_ETHERNET,
    DATA_ID_SYS_INFO_FAN_STATUS,
    DATA_ID_SYS_INFO_FAN_SPEED,
    DATA_ID_CHANNEL_BOARD_INFO_LABEL,
    DATA_ID_CHANNEL_BOARD_INFO_REVISION,
    DATA_ID_DATE_TIME_DATE,
    DATA_ID_DATE_TIME_YEAR,
    DATA_ID_DATE_TIME_MONTH,
    DATA_ID_DATE_TIME_DAY,
    DATA_ID_DATE_TIME_TIME,
    DATA_ID_DATE_TIME_HOUR,
    DATA_ID_DATE_TIME_MINUTE,
    DATA_ID_DATE_TIME_SECOND,
    DATA_ID_DATE_TIME_TIME_ZONE,
    DATA_ID_DATE_TIME_DST,
    DATA_ID_SET_PAGE_DIRTY,
    DATA_ID_PROFILES_LIST1,
    DATA_ID_PROFILES_LIST2,
    DATA_ID_PROFILES_AUTO_RECALL_STATUS,
    DATA_ID_PROFILES_AUTO_RECALL_LOCATION,
    DATA_ID_PROFILE_STATUS,
    DATA_ID_PROFILE_LABEL,
    DATA_ID_PROFILE_REMARK,
    DATA_ID_PROFILE_IS_AUTO_RECALL_LOCATION,
    DATA_ID_PROFILE_CHANNEL_U_SET,
    DATA_ID_PROFILE_CHANNEL_I_SET,
    DATA_ID_PROFILE_CHANNEL_OUTPUT_STATE,
    DATA_ID_ETHERNET_INSTALLED,
    DATA_ID_ETHERNET_ENABLED,
    DATA_ID_ETHERNET_STATUS,
    DATA_ID_ETHERNET_IP_ADDRESS,
    DATA_ID_ETHERNET_DNS,
    DATA_ID_ETHERNET_GATEWAY,
    DATA_ID_ETHERNET_SUBNET_MASK,
    DATA_ID_ETHERNET_SCPI_PORT,
    DATA_ID_ETHERNET_IS_CONNECTED,
    DATA_ID_ETHERNET_DHCP,
    DATA_ID_ETHERNET_MAC,
    DATA_ID_CHANNEL_IS_VOLTAGE_BALANCED,
    DATA_ID_CHANNEL_IS_CURRENT_BALANCED,
    DATA_ID_SYS_OUTPUT_PROTECTION_COUPLED,
    DATA_ID_SYS_SHUTDOWN_WHEN_PROTECTION_TRIPPED,
    DATA_ID_SYS_FORCE_DISABLING_ALL_OUTPUTS_ON_POWER_UP,
    DATA_ID_SYS_PASSWORD_IS_SET,
    DATA_ID_SYS_RL_STATE,
    DATA_ID_SYS_SOUND_IS_ENABLED,
    DATA_ID_SYS_SOUND_IS_CLICK_ENABLED,
    DATA_ID_CHANNEL_DISPLAY_VIEW_SETTINGS_DISPLAY_VALUE1,
    DATA_ID_CHANNEL_DISPLAY_VIEW_SETTINGS_DISPLAY_VALUE2,
    DATA_ID_CHANNEL_DISPLAY_VIEW_SETTINGS_YT_VIEW_RATE,
    DATA_ID_SYS_ENCODER_CONFIRMATION_MODE,
    DATA_ID_SYS_ENCODER_MOVING_UP_SPEED,
    DATA_ID_SYS_ENCODER_MOVING_DOWN_SPEED,
    DATA_ID_SYS_ENCODER_INSTALLED,
    DATA_ID_SYS_DISPLAY_STATE,
    DATA_ID_SYS_DISPLAY_BRIGHTNESS,
    DATA_ID_CHANNEL_TRIGGER_MODE,
    DATA_ID_CHANNEL_TRIGGER_OUTPUT_STATE,
    DATA_ID_CHANNEL_TRIGGER_ON_LIST_STOP,
    DATA_ID_CHANNEL_U_TRIGGER_VALUE,
    DATA_ID_CHANNEL_I_TRIGGER_VALUE,
    DATA_ID_CHANNEL_LIST_COUNT,
    DATA_ID_CHANNEL_LISTS,
    DATA_ID_CHANNEL_LIST_INDEX,
    DATA_ID_CHANNEL_LIST_DWELL,
    DATA_ID_CHANNEL_LIST_DWELL_ENABLED,
    DATA_ID_CHANNEL_LIST_VOLTAGE,
    DATA_ID_CHANNEL_LIST_VOLTAGE_ENABLED,
    DATA_ID_CHANNEL_LIST_CURRENT,
    DATA_ID_CHANNEL_LIST_CURRENT_ENABLED,
    DATA_ID_CHANNEL_LISTS_PREVIOUS_PAGE_ENABLED,
    DATA_ID_CHANNEL_LISTS_NEXT_PAGE_ENABLED,
    DATA_ID_CHANNEL_LISTS_CURSOR,
    DATA_ID_CHANNEL_LISTS_INSERT_MENU_ENABLED,
    DATA_ID_CHANNEL_LISTS_DELETE_MENU_ENABLED,
    DATA_ID_CHANNEL_LISTS_DELETE_ROW_ENABLED,
    DATA_ID_CHANNEL_LISTS_CLEAR_COLUMN_ENABLED,
    DATA_ID_CHANNEL_LISTS_DELETE_ROWS_ENABLED,
    DATA_ID_TRIGGER_SOURCE,
    DATA_ID_TRIGGER_DELAY,
    DATA_ID_TRIGGER_INITIATE_CONTINUOUSLY,
    DATA_ID_TRIGGER_IS_INITIATED,
    DATA_ID_TRIGGER_IS_MANUAL,
    DATA_ID_CHANNEL_HAS_SUPPORT_FOR_CURRENT_DUAL_RANGE,
    DATA_ID_CHANNEL_RANGES_SUPPORTED,
    DATA_ID_CHANNEL_RANGES_MODE,
    DATA_ID_CHANNEL_RANGES_AUTO_RANGING,
    DATA_ID_CHANNEL_RANGES_CURRENTLY_SELECTED,
    DATA_ID_TEXT_MESSAGE,
    DATA_ID_SERIAL_STATUS,
    DATA_ID_SERIAL_ENABLED,
    DATA_ID_SERIAL_IS_CONNECTED,
    DATA_ID_SERIAL_BAUD,
    DATA_ID_SERIAL_PARITY,
    DATA_ID_CHANNEL_LIST_COUNTDOWN,
    DATA_ID_IO_PINS,
    DATA_ID_IO_PINS_INHIBIT_STATE,
    DATA_ID_IO_PIN_NUMBER,
    DATA_ID_IO_PIN_POLARITY,
    DATA_ID_IO_PIN_FUNCTION,
    DATA_ID_NTP_ENABLED,
    DATA_ID_NTP_SERVER,
    DATA_ID_ASYNC_OPERATION_THROBBER,
    DATA_ID_SYS_DISPLAY_BACKGROUND_LUMINOSITY_STEP,
    DATA_ID_PROGRESS,
    DATA_ID_VIEW_STATUS,
    DATA_ID_DLOG_STATUS
};

using eez::mw::gui::data::DataOperationEnum;
using eez::mw::gui::data::Cursor;
using eez::mw::gui::data::Value;

void data_edit_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channels(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_status(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_output_state(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_output_mode(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_mon_value(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_u_set(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_u_mon(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_u_mon_dac(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_u_limit(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_u_edit(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_i_set(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_i_mon(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_i_mon_dac(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_i_limit(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_i_edit(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_p_mon(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channels_view_mode(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_display_value1(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_display_value2(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_lrip(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_ovp(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_ocp(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_opp(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_otp_ch(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_otp_aux(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_alert_message(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_alert_message_2(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_alert_message_3(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_edit_value(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_edit_unit(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_edit_info(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_edit_info1(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_edit_info2(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_edit_mode_interactive_mode_selector(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_edit_steps(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_model_info(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_firmware_info(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_self_test_result(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_keypad_text(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_keypad_caps(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_keypad_option1_text(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_keypad_option1_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_keypad_option2_text(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_keypad_option2_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_keypad_sign_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_keypad_dot_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_keypad_unit_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_label(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_short_label(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_temp_status(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_temp(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_on_time_total(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_on_time_last(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_calibration_status(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_calibration_state(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_calibration_date(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_calibration_remark(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_calibration_step_is_set_remark_step(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_calibration_step_num(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_calibration_step_status(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_calibration_step_level_value(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_calibration_step_value(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_calibration_step_prev_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_calibration_step_next_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_cal_ch_u_min(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_cal_ch_u_mid(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_cal_ch_u_max(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_cal_ch_i0_min(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_cal_ch_i0_mid(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_cal_ch_i0_max(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_cal_ch_i1_min(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_cal_ch_i1_mid(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_cal_ch_i1_max(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_protection_ovp_state(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_protection_ovp_level(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_protection_ovp_delay(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_protection_ovp_limit(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_protection_ocp_state(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_protection_ocp_delay(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_protection_ocp_limit(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_protection_ocp_max_current_limit_cause(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_protection_opp_state(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_protection_opp_level(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_protection_opp_delay(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_protection_opp_limit(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_protection_otp_installed(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_protection_otp_state(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_protection_otp_level(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_protection_otp_delay(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_event_queue_last_event_type(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_event_queue_last_event_message(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_event_queue_events(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_event_queue_events_type(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_event_queue_events_message(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_event_queue_multiple_pages(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_event_queue_previous_page_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_event_queue_next_page_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_event_queue_page_info(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_lripple_max_dissipation(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_lripple_calculated_dissipation(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_lripple_auto_mode(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_lripple_is_allowed(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_lripple_status(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_rsense_status(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_rprog_installed(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_rprog_status(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_is_coupled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_is_tracked(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_is_coupled_or_tracked(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_coupling_is_allowed(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_coupling_mode(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_coupling_selected_mode(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_coupling_is_series(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_on_time_total(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_on_time_last(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_temp_aux_status(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_temp_aux_otp_state(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_temp_aux_otp_level(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_temp_aux_otp_delay(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_temp_aux_otp_is_tripped(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_temp_aux(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_info_firmware_ver(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_info_serial_no(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_info_scpi_ver(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_info_cpu(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_info_ethernet(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_info_fan_status(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_info_fan_speed(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_board_info_label(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_board_info_revision(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_date_time_date(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_date_time_year(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_date_time_month(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_date_time_day(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_date_time_time(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_date_time_hour(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_date_time_minute(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_date_time_second(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_date_time_time_zone(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_date_time_dst(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_set_page_dirty(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_profiles_list1(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_profiles_list2(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_profiles_auto_recall_status(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_profiles_auto_recall_location(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_profile_status(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_profile_label(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_profile_remark(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_profile_is_auto_recall_location(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_profile_channel_u_set(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_profile_channel_i_set(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_profile_channel_output_state(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_ethernet_installed(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_ethernet_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_ethernet_status(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_ethernet_ip_address(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_ethernet_dns(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_ethernet_gateway(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_ethernet_subnet_mask(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_ethernet_scpi_port(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_ethernet_is_connected(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_ethernet_dhcp(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_ethernet_mac(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_is_voltage_balanced(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_is_current_balanced(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_output_protection_coupled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_shutdown_when_protection_tripped(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_force_disabling_all_outputs_on_power_up(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_password_is_set(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_rl_state(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_sound_is_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_sound_is_click_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_display_view_settings_display_value1(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_display_view_settings_display_value2(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_display_view_settings_yt_view_rate(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_encoder_confirmation_mode(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_encoder_moving_up_speed(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_encoder_moving_down_speed(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_encoder_installed(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_display_state(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_display_brightness(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_trigger_mode(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_trigger_output_state(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_trigger_on_list_stop(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_u_trigger_value(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_i_trigger_value(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_list_count(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_lists(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_list_index(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_list_dwell(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_list_dwell_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_list_voltage(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_list_voltage_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_list_current(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_list_current_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_lists_previous_page_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_lists_next_page_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_lists_cursor(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_lists_insert_menu_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_lists_delete_menu_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_lists_delete_row_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_lists_clear_column_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_lists_delete_rows_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_trigger_source(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_trigger_delay(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_trigger_initiate_continuously(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_trigger_is_initiated(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_trigger_is_manual(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_has_support_for_current_dual_range(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_ranges_supported(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_ranges_mode(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_ranges_auto_ranging(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_ranges_currently_selected(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_text_message(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_serial_status(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_serial_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_serial_is_connected(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_serial_baud(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_serial_parity(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_channel_list_countdown(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_io_pins(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_io_pins_inhibit_state(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_io_pin_number(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_io_pin_polarity(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_io_pin_function(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_ntp_enabled(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_ntp_server(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_async_operation_throbber(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_sys_display_background_luminosity_step(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_progress(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_view_status(DataOperationEnum operation, Cursor &cursor, Value &value);
void data_dlog_status(DataOperationEnum operation, Cursor &cursor, Value &value);

typedef void (*DataOperationsFunction)(DataOperationEnum operation, Cursor &cursor, Value &value);

extern DataOperationsFunction g_dataOperationsFunctions[];

enum FontsEnum {
    FONT_ID_NONE,
    FONT_ID_SMALL,
    FONT_ID_MEDIUM,
    FONT_ID_LARGE,
    FONT_ID_ICONS
};

enum BitmapsEnum {
    BITMAP_ID_NONE,
    BITMAP_ID_LOGO,
    BITMAP_ID_BP_COUPLED
};

enum StylesEnum {
    STYLE_ID_NONE,
    STYLE_ID_BAR_GRAPH_I_DEFAULT,
    STYLE_ID_BAR_GRAPH_LIMIT_LINE,
    STYLE_ID_BAR_GRAPH_SET_LINE,
    STYLE_ID_BAR_GRAPH_TEXT,
    STYLE_ID_BAR_GRAPH_TEXT_VERTICAL,
    STYLE_ID_BAR_GRAPH_U_DEFUALT,
    STYLE_ID_BAR_GRAPH_UNREGULATED,
    STYLE_ID_BOTTOM_BUTTON,
    STYLE_ID_BOTTOM_BUTTON_BACKGROUND,
    STYLE_ID_BOTTOM_BUTTON_DISABLED,
    STYLE_ID_BOTTOM_BUTTON_TEXTUAL,
    STYLE_ID_BOTTOM_BUTTON_TEXTUAL_S,
    STYLE_ID_BOTTOM_BUTTON_TEXTUAL_S_LEFT,
    STYLE_ID_BOTTOM_BUTTON_TEXTUAL_S_DISABLED,
    STYLE_ID_CHANNEL_ERROR,
    STYLE_ID_CHANNEL_OFF,
    STYLE_ID_CHANNEL_OFF_DISABLED,
    STYLE_ID_COUPLED_INFO,
    STYLE_ID_COUPLED_INFO_S,
    STYLE_ID_DEFAULT,
    STYLE_ID_DEFAULT_DISABLED,
    STYLE_ID_DEFAULT_DISABLED_S_LEFT,
    STYLE_ID_DEFAULT_EDIT_INVERSE,
    STYLE_ID_DEFAULT_INVERSE,
    STYLE_ID_DEFAULT_S,
    STYLE_ID_DEFAULT_S_LEFT,
    STYLE_ID_EDIT_INFO_S,
    STYLE_ID_EDIT_S,
    STYLE_ID_EDIT_S_FOCUS,
    STYLE_ID_EDIT_VALUE,
    STYLE_ID_EDIT_VALUE_ACTIVE,
    STYLE_ID_EDIT_VALUE_ACTIVE_LEFT,
    STYLE_ID_EDIT_VALUE_ACTIVE_S_LEFT,
    STYLE_ID_EDIT_VALUE_ACTIVE_S_RIGHT,
    STYLE_ID_EDIT_VALUE_ACTIVE_S_CENTER,
    STYLE_ID_EDIT_VALUE_FOCUS_S_RIGHT,
    STYLE_ID_EDIT_VALUE_FOCUS_S_CENTER,
    STYLE_ID_EDIT_VALUE_L,
    STYLE_ID_EDIT_VALUE_LEFT,
    STYLE_ID_EDIT_VALUE_S_CENTERED,
    STYLE_ID_EDIT_VALUE_S_LEFT,
    STYLE_ID_EDIT_VALUE_S_RIGHT,
    STYLE_ID_ERROR_ALERT,
    STYLE_ID_ERROR_ALERT_BUTTON,
    STYLE_ID_EVENT_ERROR,
    STYLE_ID_EVENT_ERROR_ICON,
    STYLE_ID_EVENT_INFO,
    STYLE_ID_EVENT_INFO_ICON,
    STYLE_ID_EVENT_WARNING,
    STYLE_ID_EVENT_WARNING_ICON,
    STYLE_ID_INFO_ALERT,
    STYLE_ID_INFO_ALERT_BUTTON,
    STYLE_ID_YES_NO,
    STYLE_ID_YES_NO_BACKGROUND,
    STYLE_ID_YES_NO_MESSAGE,
    STYLE_ID_YES_NO_BUTTON,
    STYLE_ID_KEY,
    STYLE_ID_KEY_DISABLED,
    STYLE_ID_KEY_ICONS,
    STYLE_ID_KEY_SPEC,
    STYLE_ID_KEY_SPEC_ICONS,
    STYLE_ID_MAX_CURRENT_LIMIT_CAUSE,
    STYLE_ID_MENU_S,
    STYLE_ID_MON_DAC,
    STYLE_ID_MON_DAC_S,
    STYLE_ID_MON_VALUE,
    STYLE_ID_MON_VALUE_L_RIGHT,
    STYLE_ID_MON_VALUE_UR_L_RIGHT,
    STYLE_ID_MON_VALUE_FOCUS,
    STYLE_ID_NON_INTERACTIVE_BUTTON_S,
    STYLE_ID_NON_INTERACTIVE_BUTTON_S_DISABLED,
    STYLE_ID_PROT_INDICATOR_S,
    STYLE_ID_PROT_INDICATOR_SET_S,
    STYLE_ID_PROT_INDICATOR_TRIP_S,
    STYLE_ID_PROT_INDICATOR_INVALID_S,
    STYLE_ID_PROT_INDICATOR_BLINK_S,
    STYLE_ID_SET_VALUE_BALANCED,
    STYLE_ID_SET_VALUE_FOCUS_BALANCED,
    STYLE_ID_SET_VALUE_S_RIGHT_BALANCED,
    STYLE_ID_TAB_PAGE,
    STYLE_ID_TAB_PAGE_SELECTED,
    STYLE_ID_VALUE,
    STYLE_ID_VALUE_S,
    STYLE_ID_YELLOW_1,
    STYLE_ID_YELLOW_2,
    STYLE_ID_YELLOW_3,
    STYLE_ID_YELLOW_4,
    STYLE_ID_YELLOW_5,
    STYLE_ID_YELLOW_6,
    STYLE_ID_TOAST_ALERT,
    STYLE_ID_YT_GRAPH_U_DEFUALT,
    STYLE_ID_YT_GRAPH_U_DEFUALT_LABEL,
    STYLE_ID_YT_GRAPH_I_DEFAULT,
    STYLE_ID_YT_GRAPH_I_DEFAULT_LABEL,
    STYLE_ID_YT_GRAPH,
    STYLE_ID_YT_GRAPH_UNREGULATED,
    STYLE_ID_SELECT_ENUM_ITEM_POPUP_CONTAINER,
    STYLE_ID_SELECT_ENUM_ITEM_POPUP_ITEM,
    STYLE_ID_SELECT_ENUM_ITEM_POPUP_DISABLED_ITEM,
    STYLE_ID_DISPLAY_OFF,
    STYLE_ID_DISPLAY_OFF_S,
    STYLE_ID_LIST_GRAPH_CURSOR,
    STYLE_ID_DARK_LINE,
    STYLE_ID_TEXT_MESSAGE,
    STYLE_ID_BUTTON_INDICATOR_OFF,
    STYLE_ID_BUTTON_INDICATOR_ON,
    STYLE_ID_BUTTON_INDICATOR_ERROR,
    STYLE_ID_ASYNC_OPERATION,
    STYLE_ID_ASYNC_OPERATION_ACTION
};

enum PagesEnum {
    PAGE_ID_ETHERNET_INIT,
    PAGE_ID_SCREEN_CALIBRATION_INTRO,
    PAGE_ID_SCREEN_CALIBRATION_YES_NO,
    PAGE_ID_SCREEN_CALIBRATION_YES_NO_CANCEL,
    PAGE_ID_WELCOME,
    PAGE_ID_SELF_TEST_RESULT,
    PAGE_ID_MAIN,
    PAGE_ID_MAIN_HELP,
    PAGE_ID_EDIT_MODE_KEYPAD,
    PAGE_ID_EDIT_MODE_STEP,
    PAGE_ID_EDIT_MODE_STEP_HELP,
    PAGE_ID_EDIT_MODE_SLIDER,
    PAGE_ID_EDIT_MODE_SLIDER_HELP,
    PAGE_ID_INFO_ALERT,
    PAGE_ID_INFO_LONG_ALERT,
    PAGE_ID_TOAST3_ALERT,
    PAGE_ID_ERROR_ALERT,
    PAGE_ID_ERROR_LONG_ALERT,
    PAGE_ID_ERROR_ALERT_WITH_ACTION,
    PAGE_ID_ERROR_TOAST_ALERT,
    PAGE_ID_YES_NO,
    PAGE_ID_ARE_YOU_SURE_WITH_MESSAGE,
    PAGE_ID_YES_NO_LATER,
    PAGE_ID_TEXT_MESSAGE,
    PAGE_ID_ASYNC_OPERATION_IN_PROGRESS,
    PAGE_ID_PROGRESS,
    PAGE_ID_EVENT_QUEUE,
    PAGE_ID_KEYPAD,
    PAGE_ID_NUMERIC_KEYPAD,
    PAGE_ID_CH_SETTINGS_PROT,
    PAGE_ID_CH_SETTINGS_PROT_CLEAR,
    PAGE_ID_CH_SETTINGS_PROT_OVP,
    PAGE_ID_CH_SETTINGS_PROT_OCP,
    PAGE_ID_CH_SETTINGS_PROT_OPP,
    PAGE_ID_CH_SETTINGS_PROT_OTP,
    PAGE_ID_CH_SETTINGS_TRIGGER,
    PAGE_ID_CH_SETTINGS_LISTS,
    PAGE_ID_CH_SETTINGS_LISTS_INSERT_MENU,
    PAGE_ID_CH_SETTINGS_LISTS_DELETE_MENU,
    PAGE_ID_CH_START_LIST,
    PAGE_ID_CH_SETTINGS_ADV,
    PAGE_ID_CH_SETTINGS_ADV_LRIPPLE,
    PAGE_ID_CH_SETTINGS_ADV_REMOTE,
    PAGE_ID_CH_SETTINGS_ADV_RANGES,
    PAGE_ID_CH_SETTINGS_ADV_TRACKING,
    PAGE_ID_CH_SETTINGS_ADV_COUPLING,
    PAGE_ID_CH_SETTINGS_ADV_COUPLING_INFO,
    PAGE_ID_CH_SETTINGS_ADV_VIEW,
    PAGE_ID_CH_SETTINGS_INFO,
    PAGE_ID_SYS_SETTINGS,
    PAGE_ID_SYS_SETTINGS_AUX_OTP,
    PAGE_ID_SYS_SETTINGS_PROTECTIONS,
    PAGE_ID_SYS_SETTINGS_TRIGGER,
    PAGE_ID_SYS_SETTINGS_IO,
    PAGE_ID_SYS_SETTINGS_DATE_TIME,
    PAGE_ID_SYS_SETTINGS_ENCODER,
    PAGE_ID_SYS_SETTINGS2,
    PAGE_ID_SYS_SETTINGS_SERIAL,
    PAGE_ID_SYS_SETTINGS_ETHERNET,
    PAGE_ID_SYS_SETTINGS_ETHERNET_STATIC,
    PAGE_ID_SYS_SETTINGS_CAL,
    PAGE_ID_SYS_SETTINGS_CAL_CH,
    PAGE_ID_SYS_SETTINGS_CAL_CH_WIZ_STEP,
    PAGE_ID_SYS_SETTINGS_CAL_CH_WIZ_FINISH,
    PAGE_ID_SYS_SETTINGS_SCREEN_CALIBRATION,
    PAGE_ID_SYS_SETTINGS_DISPLAY,
    PAGE_ID_SYS_SETTINGS_SOUND,
    PAGE_ID_SYS_SETTINGS_DIAG,
    PAGE_ID_USER_PROFILES,
    PAGE_ID_USER_PROFILES2,
    PAGE_ID_USER_PROFILE_SETTINGS,
    PAGE_ID_USER_PROFILE_0_SETTINGS,
    PAGE_ID_SYS_INFO,
    PAGE_ID_SYS_INFO2,
    PAGE_ID_STAND_BY_MENU,
    PAGE_ID_ENTERING_STANDBY,
    PAGE_ID_STANDBY,
    PAGE_ID_DISPLAY_OFF
};

extern const uint8_t *fonts[];

struct Bitmap {
    uint16_t w;
    uint16_t h;
    const uint8_t *pixels;
};

extern Bitmap bitmaps[];

extern const uint8_t styles[];

extern const uint8_t document[];

}
}
} // namespace eez::app::gui
