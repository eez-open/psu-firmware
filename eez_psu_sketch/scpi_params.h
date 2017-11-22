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
namespace scpi {

extern scpi_choice_def_t temp_sensor_choice[];
extern scpi_choice_def_t internal_external_choice[];

Channel *param_channel(scpi_t *context, scpi_bool_t mandatory = FALSE, scpi_bool_t skip_channel_check = FALSE);
bool check_channel(scpi_t *context, int32_t ch);
Channel *set_channel_from_command_number(scpi_t *context);

bool param_temp_sensor(scpi_t *context, int32_t &sensor);

bool get_voltage_param(scpi_t *context, float &value, const Channel *channel, const Channel::Value *cv);
bool get_voltage_protection_level_param(scpi_t *context, float &value, float min, float max, float def);
bool get_current_param(scpi_t *context, float &value, const Channel *channel, const Channel::Value *cv);
bool get_power_param(scpi_t *context, float &value, float min, float max, float def);
bool get_temperature_param(scpi_t *context, float &value, float min, float max, float def);
bool get_duration_param(scpi_t *context, float &value, float min, float max, float def);

bool get_voltage_from_param(scpi_t *context, const scpi_number_t &param, float &value, const Channel *channel, const Channel::Value *cv);
bool get_voltage_protection_level_from_param(scpi_t *context, const scpi_number_t &param, float &value, float min, float max, float def);
bool get_current_from_param(scpi_t *context, const scpi_number_t &param, float &value, const Channel *channel, const Channel::Value *cv);
bool get_power_from_param(scpi_t *context, const scpi_number_t &param, float &value, float min, float max, float def);
bool get_temperature_from_param(scpi_t *context, const scpi_number_t &param, float &value, float min, float max, float def);
bool get_duration_from_param(scpi_t *context, const scpi_number_t &param, float &value, float min, float max, float def);

bool get_voltage_limit_param(scpi_t *context, float &value, const Channel *channel, const Channel::Value *cv);
bool get_current_limit_param(scpi_t *context, float &value, const Channel *channel, const Channel::Value *cv);
bool get_power_limit_param(scpi_t *context, float &value, const Channel *channel, const Channel::Value *cv);
bool get_voltage_limit_from_param(scpi_t *context, const scpi_number_t &param, float &value, const Channel *channel, const Channel::Value *cv);
bool get_current_limit_from_param(scpi_t *context, const scpi_number_t &param, float &value, const Channel *channel, const Channel::Value *cv);
bool get_power_limit_from_param(scpi_t *context, const scpi_number_t &param, float &value, const Channel *channel, const Channel::Value *cv);

scpi_result_t result_float(scpi_t *context, Channel *channel, float value, ValueType valueType);
bool get_profile_location_param(scpi_t *context, int &location, bool all_locations = false);

void outputOnTime(scpi_t* context, uint32_t time);
bool checkPassword(scpi_t *context, const char *againstPassword);

}
}
} // namespace eez::psu::scpi
