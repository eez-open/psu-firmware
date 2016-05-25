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
/// PSU configuration profiles (save, recall, ...).
namespace profile {

/// Channel binary flags stored in profile.
struct ChannelFlags {
    unsigned output_enabled : 1;
    unsigned sense_enabled : 1;
    unsigned u_state : 1;
    unsigned i_state : 1;
    unsigned p_state : 1;
    unsigned cal_enabled : 1;
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
    unsigned reserverd6 : 1;
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R2B6
    unsigned rprog_enabled : 1;
#endif
    unsigned reserverd7 : 1;
    unsigned reserverd8 : 1;
    unsigned reserverd9 : 1;
    unsigned reserverd10 : 1;
    unsigned reserverd11 : 1;
    unsigned reserverd12 : 1;
    unsigned reserverd13 : 1;
    unsigned reserverd14 : 1;
    unsigned reserverd15 : 1;
};

/// Channel parameters stored in profile.
struct ChannelParameters {
    ChannelFlags flags;
    float u_set;
    float u_step;
    float i_set;
    float i_step;
    float u_delay;
    float i_delay;
    float p_delay;
    float p_level;
#ifdef EEZ_PSU_SIMULATOR
    bool load_enabled;
    float load;
#endif
};

/// Profile parameters.
struct Parameters {
    persist_conf::BlockHeader header;
    bool is_valid;
    char name[PROFILE_NAME_MAX_LENGTH + 1];
    bool power_is_up;
    ChannelParameters channels[CH_MAX];
    temperature::ProtectionConfiguration temp_prot[temp_sensor::COUNT];
};

void tick(unsigned long tick_usec);

void recallChannelsFromProfile(Parameters *profile);
bool recallFromProfile(Parameters *profile);
bool recall(int location);

bool load(int location, Parameters *profile);

bool enableSave(bool enable);
void save();
void saveImmediately();
bool saveAtLocation(int location);

bool deleteLocation(int location);
bool deleteAll();

bool isValid(int location);

bool setName(int location, const char *name, size_t name_len);
void getName(int location, char *name);

}
}
} // namespace eez::psu::profile
