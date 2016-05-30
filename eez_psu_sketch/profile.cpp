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
 
#include "psu.h"
#include "profile.h"
#include "persist_conf.h"
#include "datetime.h"

namespace eez {
namespace psu {
namespace profile {

static bool g_save_enabled = true;
static bool g_save_profile = false;

////////////////////////////////////////////////////////////////////////////////

void tick(unsigned long tick_usec) {
    if (g_save_profile) {
        saveAtLocation(0);
        g_save_profile = false;
    }
}

void recallChannelsFromProfile(Parameters *profile) {
    bool last_save_enabled = enableSave(false);

    for (int i = 0; i < CH_MAX; ++i) {
        Channel::get(i).prot_conf.u_delay = profile->channels[i].u_delay;
        Channel::get(i).prot_conf.i_delay = profile->channels[i].i_delay;
        Channel::get(i).prot_conf.p_delay = profile->channels[i].p_delay;
        Channel::get(i).prot_conf.p_level = profile->channels[i].p_level;

        Channel::get(i).prot_conf.flags.u_state = profile->channels[i].flags.u_state;
        Channel::get(i).prot_conf.flags.i_state = profile->channels[i].flags.i_state;
        Channel::get(i).prot_conf.flags.p_state = profile->channels[i].flags.p_state;

        Channel::get(i).u.set = profile->channels[i].u_set;
        Channel::get(i).u.step = profile->channels[i].u_step;

        Channel::get(i).i.set = profile->channels[i].i_set;
        Channel::get(i).i.step = profile->channels[i].i_step;

#ifdef EEZ_PSU_SIMULATOR
        Channel::get(i).simulator.load_enabled = profile->channels[i].load_enabled;
        Channel::get(i).simulator.load = profile->channels[i].load;
#endif

        Channel::get(i).flags.cal_enabled = profile->channels[i].flags.cal_enabled && Channel::get(i).isCalibrationExists() ? 1 : 0;
        Channel::get(i).flags.output_enabled = profile->channels[i].flags.output_enabled;
        Channel::get(i).flags.sense_enabled = profile->channels[i].flags.sense_enabled;

        if (Channel::get(i).getFeatures() & CH_FEATURE_RPROG) {
            Channel::get(i).flags.rprog_enabled = profile->channels[i].flags.rprog_enabled;
        } else {
            Channel::get(i).flags.rprog_enabled = 0;
        }

        if (Channel::get(i).getFeatures() & CH_FEATURE_LRIPPLE) {
            Channel::get(i).flags.lripple_enabled = profile->channels[i].flags.lripple_enabled;
            Channel::get(i).flags.lripple_auto_enabled = profile->channels[i].flags.lripple_auto_enabled;
        } else {
            Channel::get(i).flags.lripple_enabled = 0;
            Channel::get(i).flags.lripple_auto_enabled = 0;
        }

        Channel::get(i).update();
    }

    enableSave(last_save_enabled);
}

bool recallFromProfile(Parameters *profile) {
    bool last_save_enabled = enableSave(false);

    bool result = true;

    memcpy(temperature::prot_conf, profile->temp_prot, sizeof(profile->temp_prot));

    if (profile->power_is_up) result &= psu::powerUp();
    else psu::powerDown();

    recallChannelsFromProfile(profile);

    enableSave(last_save_enabled);

    return result;
}

bool recall(int location) {
    if (location > 0 && location < NUM_PROFILE_LOCATIONS) {
        Parameters profile;
        if (persist_conf::loadProfile(location, &profile) && profile.is_valid) {
            if (persist_conf::saveProfile(0, &profile)) {
                return recallFromProfile(&profile);
            }
        }
    }
    return false;
}

bool load(int location, Parameters *profile) {
    if (location >= 0 && location < NUM_PROFILE_LOCATIONS) {
        return persist_conf::loadProfile(location, profile) && profile->is_valid;
    }
    return false;
}

bool enableSave(bool enable) {
    bool last_save_enabled = g_save_enabled;
    g_save_enabled = enable;
    return last_save_enabled;
}

void save() {
    if (!g_save_enabled) return;
    g_save_profile = true;
}

void saveImmediately() {
    saveAtLocation(0);
}

bool saveAtLocation(int location) {
    if (location >= 0 && location < NUM_PROFILE_LOCATIONS) {
        Parameters currentProfile;
        if (!persist_conf::loadProfile(location, &currentProfile)) {
            currentProfile.is_valid = false;
        }

        Parameters profile;

        profile.is_valid = true;

        // name
        memset(profile.name, 0, sizeof(profile.name));
        if (location > 0) {
            if (!currentProfile.is_valid || strncmp_P(currentProfile.name, PSTR("Saved at "), 9) == 0) {
                char datetime_buffer[20] = { 0 };
                if (datetime::getDateTimeAsString(datetime_buffer)) {
                    strcpy_P(profile.name, PSTR("Saved at "));
                    strcat(profile.name, datetime_buffer);
                }
            }
            else {
                strcpy(profile.name, currentProfile.name);
            }
        }

        noInterrupts();

        profile.power_is_up = psu::isPowerUp();

        for (int i = 0; i < CH_MAX; ++i) {
            profile.channels[i].flags.cal_enabled = Channel::get(i).flags.cal_enabled;
            profile.channels[i].flags.output_enabled = Channel::get(i).flags.output_enabled;
            profile.channels[i].flags.sense_enabled = Channel::get(i).flags.sense_enabled;

            if (Channel::get(i).getFeatures() & CH_FEATURE_RPROG) {
                profile.channels[i].flags.rprog_enabled = Channel::get(i).flags.rprog_enabled;
            } else {
                profile.channels[i].flags.rprog_enabled = 0;
            }

            if (Channel::get(i).getFeatures() & CH_FEATURE_LRIPPLE) {
                profile.channels[i].flags.lripple_enabled = Channel::get(i).flags.lripple_enabled;
                profile.channels[i].flags.lripple_auto_enabled = Channel::get(i).flags.lripple_auto_enabled;
            } else {
                profile.channels[i].flags.lripple_enabled = 0;
                profile.channels[i].flags.lripple_auto_enabled = 0;
            }

            profile.channels[i].flags.u_state = Channel::get(i).prot_conf.flags.u_state;
            profile.channels[i].flags.i_state = Channel::get(i).prot_conf.flags.i_state;
            profile.channels[i].flags.p_state = Channel::get(i).prot_conf.flags.p_state;

            profile.channels[i].u_set = Channel::get(i).u.set;
            profile.channels[i].u_step = Channel::get(i).u.step;

            profile.channels[i].i_set = Channel::get(i).i.set;
            profile.channels[i].i_step = Channel::get(i).i.step;

            profile.channels[i].u_delay = Channel::get(i).prot_conf.u_delay;
            profile.channels[i].i_delay = Channel::get(i).prot_conf.i_delay;
            profile.channels[i].p_delay = Channel::get(i).prot_conf.p_delay;
            profile.channels[i].p_level = Channel::get(i).prot_conf.p_level;

#ifdef EEZ_PSU_SIMULATOR
            profile.channels[i].load_enabled = Channel::get(i).simulator.load_enabled;
            profile.channels[i].load = Channel::get(i).simulator.load;
#endif
        }

        memcpy(profile.temp_prot, temperature::prot_conf, sizeof(profile.temp_prot));

        interrupts();

        return persist_conf::saveProfile(location, &profile);
    }

    return false;
}

bool deleteLocation(int location) {
    bool result = false;
    if (location > 0 && location < NUM_PROFILE_LOCATIONS) {
        Parameters profile;
        profile.is_valid = false;
        if (location == persist_conf::getProfileAutoRecallLocation()) {
            persist_conf::setProfileAutoRecallLocation(0);
        }
        result = persist_conf::saveProfile(location, &profile);
    }
    return result;
}

bool deleteAll() {
    for (int i = 1; i < NUM_PROFILE_LOCATIONS; ++i) {
        if (!deleteLocation(i)) {
            return false;
        }
    }
    return true;
}

bool isValid(int location) {
    if (location >= 0 && location < NUM_PROFILE_LOCATIONS) {
        Parameters profile;
        if (persist_conf::loadProfile(location, &profile)) {
            return profile.is_valid;
        }
    }
    return false;
}

bool setName(int location, const char *name, size_t name_len) {
    if (location > 0 && location < NUM_PROFILE_LOCATIONS) {
        Parameters profile;
        if (persist_conf::loadProfile(location, &profile) && profile.is_valid) {
            memset(profile.name, 0, sizeof(profile.name));
            strncpy(profile.name, name, name_len);
            return persist_conf::saveProfile(location, &profile);
        }
    }
    return false;
}

void getName(int location, char *name) {
    if (location >= 0 && location < NUM_PROFILE_LOCATIONS) {
        Parameters profile;
        if (persist_conf::loadProfile(location, &profile) && profile.is_valid) {
            strcpy(name, profile.name);
            return;
        }
    }
    strcpy(name, "--Not used--");
}

}
}
} // namespace eez::psu::profile