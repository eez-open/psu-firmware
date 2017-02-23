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
#include "event_queue.h"
#include "channel_dispatcher.h"

namespace eez {
namespace psu {
namespace profile {

#define AUTO_NAME_PREFIX PSTR("Saved at ")

static bool g_save_enabled = true;
static bool g_save_profile = false;

////////////////////////////////////////////////////////////////////////////////

void tick(uint32_t tickCount) {
    if (g_save_profile) {
        saveAtLocation(0);
        g_save_profile = false;
    }
}

void recallChannelsFromProfile(Parameters *profile) {
    bool last_save_enabled = enableSave(false);

    channel_dispatcher::setType((channel_dispatcher::Type)profile->flags.channelsCoupling);

    for (int i = 0; i < CH_MAX; ++i) {
		Channel &channel = Channel::get(i);

		if (profile->channels[i].flags.parameters_are_valid) {
			channel.prot_conf.u_delay = profile->channels[i].u_delay;
			channel.prot_conf.u_level = profile->channels[i].u_level;
			channel.prot_conf.i_delay = profile->channels[i].i_delay;
			channel.prot_conf.p_delay = profile->channels[i].p_delay;
			channel.prot_conf.p_level = profile->channels[i].p_level;

			channel.prot_conf.flags.u_state = profile->channels[i].flags.u_state;
			channel.prot_conf.flags.i_state = profile->channels[i].flags.i_state;
			channel.prot_conf.flags.p_state = profile->channels[i].flags.p_state;

			channel.u.set = profile->channels[i].u_set;
			channel.u.step = profile->channels[i].u_step;
			channel.u.limit = profile->channels[i].u_limit;

			channel.i.set = profile->channels[i].i_set;
			channel.i.step = profile->channels[i].i_step;
			channel.setCurrentLimit(profile->channels[i].i_limit);

			channel.p_limit = profile->channels[i].p_limit;

#ifdef EEZ_PSU_SIMULATOR
			channel.simulator.load_enabled = profile->channels[i].load_enabled;
			channel.simulator.load = profile->channels[i].load;
			channel.simulator.voltProgExt = profile->channels[i].voltProgExt;
#endif

			channel.calibrationEnable(profile->channels[i].flags.cal_enabled && channel.isCalibrationExists() ? 1 : 0);
			channel.flags.outputEnabled = profile->channels[i].flags.output_enabled;
			channel.flags.senseEnabled = profile->channels[i].flags.sense_enabled;

			if (channel.getFeatures() & CH_FEATURE_RPROG) {
				channel.flags.rprogEnabled = profile->channels[i].flags.rprog_enabled;
			} else {
				channel.flags.rprogEnabled = 0;
			}

			if (channel.getFeatures() & CH_FEATURE_LRIPPLE) {
				channel.flags.lrippleAutoEnabled = profile->channels[i].flags.lripple_auto_enabled;
			} else {
				channel.flags.lrippleEnabled = 0;
				channel.flags.lrippleAutoEnabled = 0;
			}

            channel.flags.displayValue1 = profile->channels[i].flags.displayValue1;
            channel.flags.displayValue2 = profile->channels[i].flags.displayValue2;
            channel.ytViewRate = profile->channels[i].ytViewRate;

            if (channel.flags.displayValue1 == 0 && channel.flags.displayValue2 == 0) {
                channel.flags.displayValue1 = DISPLAY_VALUE_VOLTAGE;
                channel.flags.displayValue2 = DISPLAY_VALUE_CURRENT;
            }
            if (channel.ytViewRate == 0) {
                channel.ytViewRate = GUI_YT_VIEW_RATE_DEFAULT;
            }
		}

		channel.update();
	}

    enableSave(last_save_enabled);
}

bool recallFromProfile(Parameters *profile) {
    bool last_save_enabled = enableSave(false);

    bool result = true;

	for (int i = 0; i < temp_sensor::NUM_TEMP_SENSORS; ++i) {
		memcpy(&temperature::sensors[i].prot_conf, profile->temp_prot + i, sizeof(temperature::ProtectionConfiguration));
	}

    if (profile->flags.powerIsUp) result &= psu::powerUp();
    else psu::powerDown();

    recallChannelsFromProfile(profile);

    enableSave(last_save_enabled);

    return result;
}

bool recall(int location) {
    if (location > 0 && location < NUM_PROFILE_LOCATIONS) {
        Parameters profile;
        if (persist_conf::loadProfile(location, &profile) && profile.flags.isValid) {
            if (persist_conf::saveProfile(0, &profile)) {
                if (recallFromProfile(&profile)) {
					event_queue::pushEvent(event_queue::EVENT_INFO_RECALL_FROM_PROFILE_0 + location);
					return true;
				} else {
					return false;
				}
            }
        }
    }
    return false;
}

bool load(int location, Parameters *profile) {
    if (location >= 0 && location < NUM_PROFILE_LOCATIONS) {
        if (persist_conf::loadProfile(location, profile)) {
            return profile->flags.isValid;
        }
    }
    profile->flags.isValid = 0;
    return false;
}

void getSaveName(const Parameters *profile, char *name) {
	if (!profile->flags.isValid || strncmp_P(profile->name, AUTO_NAME_PREFIX, strlen(AUTO_NAME_PREFIX)) == 0) {
		strcpy_P(name, AUTO_NAME_PREFIX);
		datetime::getDateTimeAsString(name + strlen(AUTO_NAME_PREFIX));
	} else {
		strcpy(name, profile->name);
	}
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
    g_save_profile = false;
}

bool saveAtLocation(int location, char *name) {
    if (location >= 0 && location < NUM_PROFILE_LOCATIONS) {
        Parameters currentProfile;

        if (!persist_conf::loadProfile(location, &currentProfile)) {
            currentProfile.flags.isValid = false;
        }

        Parameters profile;
        memset(&profile, 0, sizeof(Parameters));

        profile.flags.isValid = true;

        profile.flags.channelsCoupling = channel_dispatcher::getType();

        // name
        if (location > 0) {
			if (name) {
				strcpy(profile.name, name);
			} else {
				getSaveName(&currentProfile, profile.name);
			}
        }

        noInterrupts();

        profile.flags.powerIsUp = psu::isPowerUp();

        for (int i = 0; i < CH_MAX; ++i) {
			if (i < CH_NUM) {
				Channel &channel = Channel::get(i);

				profile.channels[i].flags.parameters_are_valid = 1;

				profile.channels[i].flags.cal_enabled = channel.isCalibrationEnabled();
				profile.channels[i].flags.output_enabled = channel.flags.outputEnabled;
				profile.channels[i].flags.sense_enabled = channel.flags.senseEnabled;

				if (channel.getFeatures() & CH_FEATURE_RPROG) {
					profile.channels[i].flags.rprog_enabled = channel.flags.rprogEnabled;
				} else {
					profile.channels[i].flags.rprog_enabled = 0;
				}

				if (channel.getFeatures() & CH_FEATURE_LRIPPLE) {
					profile.channels[i].flags.lripple_auto_enabled = Channel::get(i).flags.lrippleAutoEnabled;
				} else {
					profile.channels[i].flags.lripple_auto_enabled = 0;
				}

				profile.channels[i].flags.u_state = channel.prot_conf.flags.u_state;
				profile.channels[i].flags.i_state = channel.prot_conf.flags.i_state;
				profile.channels[i].flags.p_state = channel.prot_conf.flags.p_state;

				profile.channels[i].u_set = channel.getUSetUnbalanced();
				profile.channels[i].u_step = channel.u.step;
				profile.channels[i].u_limit = channel.u.limit;

				profile.channels[i].i_set = channel.getISetUnbalanced();
				profile.channels[i].i_step = channel.i.step;
				profile.channels[i].i_limit = channel.i.limit;

				profile.channels[i].p_limit = channel.p_limit;

				profile.channels[i].u_delay = channel.prot_conf.u_delay;
				profile.channels[i].u_level = channel.prot_conf.u_level;
				profile.channels[i].i_delay = channel.prot_conf.i_delay;
				profile.channels[i].p_delay = channel.prot_conf.p_delay;
				profile.channels[i].p_level = channel.prot_conf.p_level;

                profile.channels[i].flags.displayValue1 = channel.flags.displayValue1;
                profile.channels[i].flags.displayValue2 = channel.flags.displayValue2;
                profile.channels[i].ytViewRate = channel.ytViewRate;

#ifdef EEZ_PSU_SIMULATOR
				profile.channels[i].load_enabled = channel.simulator.load_enabled;
				profile.channels[i].load = channel.simulator.load;
				profile.channels[i].voltProgExt = channel.simulator.voltProgExt;
#endif
			} else {
				profile.channels[i].flags.parameters_are_valid = 0;
			}
        }

		for (int i = 0; i < temp_sensor::MAX_NUM_TEMP_SENSORS; ++i) {
			if (i < temp_sensor::NUM_TEMP_SENSORS) {
				memcpy(profile.temp_prot + i, &temperature::sensors[i].prot_conf, sizeof(temperature::ProtectionConfiguration));
			} else {
				profile.temp_prot[i].sensor = i;
                if (profile.temp_prot[i].sensor == temp_sensor::AUX) {
				    profile.temp_prot[i].delay = OTP_AUX_DEFAULT_DELAY;
				    profile.temp_prot[i].level = OTP_AUX_DEFAULT_LEVEL;
				    profile.temp_prot[i].state = OTP_AUX_DEFAULT_STATE;
                } else {
				    profile.temp_prot[i].delay = OTP_CH_DEFAULT_DELAY;
				    profile.temp_prot[i].level = OTP_CH_DEFAULT_LEVEL;
				    profile.temp_prot[i].state = OTP_CH_DEFAULT_STATE;
                }
			}
		}

        interrupts();

        return persist_conf::saveProfile(location, &profile);
    }

    return false;
}

bool deleteLocation(int location) {
    bool result = false;
    if (location > 0 && location < NUM_PROFILE_LOCATIONS) {
        Parameters profile;
        profile.flags.isValid = false;
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
            return profile.flags.isValid;
        }
    }
    return false;
}

bool setName(int location, const char *name, size_t name_len) {
    if (location > 0 && location < NUM_PROFILE_LOCATIONS) {
        Parameters profile;
        if (persist_conf::loadProfile(location, &profile) && profile.flags.isValid) {
            memset(profile.name, 0, sizeof(profile.name));
            strncpy(profile.name, name, name_len);
            return persist_conf::saveProfile(location, &profile);
        }
    }
    return false;
}

void getName(int location, char *name, int count) {
	if (location >= 0 && location < NUM_PROFILE_LOCATIONS) {
        Parameters profile;
        if (persist_conf::loadProfile(location, &profile) && profile.flags.isValid) {
            strncpy(name, profile.name, count - 1);
			name[count - 1] = 0;
            return;
        }
    }
    strncpy(name, "--Not used--", count - 1);
	name[count - 1] = 0;
}

}
}
} // namespace eez::psu::profile