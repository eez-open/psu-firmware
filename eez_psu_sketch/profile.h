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

#include "temperature.h"

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
    unsigned reserved1 : 1;
    unsigned rprog_enabled : 1;
    unsigned reserverd10 : 1;
    unsigned lripple_auto_enabled : 1;
    unsigned parameters_are_valid : 1;
    unsigned displayValue1 : 2;
    unsigned displayValue2 : 2;
    unsigned u_triggerMode : 2;
    unsigned i_triggerMode : 2;
    unsigned currentRangeSelectionMode: 2;
    unsigned autoSelectCurrentRange: 1;
    unsigned listSaved: 1;
    unsigned triggerOutputState: 1;
    unsigned triggerOnListStop: 3;
    unsigned reserved2: 6;
};

/// Channel parameters stored in profile.
struct ChannelParameters {
    ChannelFlags flags;
    float u_set;
    float u_step;
	float u_limit;
    float u_delay;
    float u_level;
    float i_set;
    float i_step;
	float i_limit;
    float i_delay;
	float p_limit;
    float p_delay;
    float p_level;
    float ytViewRate;
    float u_triggerValue;
    float i_triggerValue;
    uint16_t listCount;
#ifdef EEZ_PSU_SIMULATOR
    bool load_enabled;
    float load;
	float voltProgExt;
#endif
};

/// Channel binary flags stored in profile.
struct ProfileFlags {
    unsigned isValid: 1;
    unsigned powerIsUp: 1;
    unsigned channelsCoupling : 2;
    unsigned reserverd : 12;
};

/// Profile parameters.
struct Parameters {
    persist_conf::BlockHeader header;
    ProfileFlags flags;
    char name[PROFILE_NAME_MAX_LENGTH + 1];
    ChannelParameters channels[CH_MAX];
    temperature::ProtectionConfiguration temp_prot[temp_sensor::MAX_NUM_TEMP_SENSORS];
};

void tick(uint32_t tick_usec);

void recallChannelsFromProfile(Parameters *profile, int location);
bool recallFromProfile(Parameters *profile, int location);
bool recall(int location);

bool load(int location, Parameters *profile);

void getSaveName(const Parameters *profile, char *name);

bool enableSave(bool enable);
void save();
void saveImmediately();
bool saveAtLocation(int location, char *name = 0);

bool deleteLocation(int location);
bool deleteAll();

bool isValid(int location);

bool setName(int location, const char *name, size_t nameLength);
void getName(int location, char *name, int count);

}
}
} // namespace eez::psu::profile
