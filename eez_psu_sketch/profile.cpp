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
#include "trigger.h"
#include "list_program.h"
#include "calibration.h"
#include "idle.h"
#include "scpi_psu.h"
#if OPTION_SD_CARD
#include "sd_card.h"
#endif

namespace eez {
namespace psu {
namespace profile {

#define AUTO_NAME_PREFIX "Saved at "

static bool g_saveEnabled = true;
static bool g_saveProfile = false;

////////////////////////////////////////////////////////////////////////////////

void tick(uint32_t tickCount) {
    if (persist_conf::devConf.flags.profileAutoRecallEnabled) {
        if (g_saveProfile && !list::isActive() && !calibration::isEnabled() && idle::isIdle()) {
            DebugTrace("Profile 0 saved!");
            saveAtLocation(0);
            g_saveProfile = false;
        }
    }
}

void getChannelProfileListFilePath(Channel &channel, int location, char *filePath) {
    strcpy(filePath, PROFILES_DIR);
    strcat(filePath, PATH_SEPARATOR);
    strcat(filePath, "LST_");
    strcatInt(filePath, channel.index);
    strcat(filePath, "_");
    strcatInt(filePath, location);
    strcat(filePath, LIST_FILE_EXTENSION);
}

void recallChannelsFromProfile(Parameters *profile, int location) {
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

#ifdef EEZ_PLATFORM_SIMULATOR
			channel.simulator.load_enabled = profile->channels[i].load_enabled;
			channel.simulator.load = profile->channels[i].load;
			channel.simulator.voltProgExt = profile->channels[i].voltProgExt;
#endif

			channel.flags.outputEnabled = channel.isTripped() ? 0 : profile->channels[i].flags.output_enabled;
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

            channel.flags.voltageTriggerMode = (TriggerMode)profile->channels[i].flags.u_triggerMode;
            channel.flags.currentTriggerMode = (TriggerMode)profile->channels[i].flags.i_triggerMode;
            channel.flags.triggerOutputState = profile->channels[i].flags.triggerOutputState;
            channel.flags.triggerOnListStop = profile->channels[i].flags.triggerOnListStop;
            trigger::setVoltage(channel, profile->channels[i].u_triggerValue);
            trigger::setCurrent(channel, profile->channels[i].i_triggerValue);
            list::setListCount(channel, profile->channels[i].listCount);

            channel.flags.currentRangeSelectionMode = profile->channels[i].flags.currentRangeSelectionMode;
            channel.flags.autoSelectCurrentRange = profile->channels[i].flags.autoSelectCurrentRange;

#if OPTION_SD_CARD
            if (profile->channels[i].flags.listSaved) {
                char filePath[MAX_PATH_LENGTH];
                getChannelProfileListFilePath(channel, location, filePath);
                int err;
                if (list::loadList(channel, filePath, &err)) {
                    if (location == 0) {
                        list::setListsChanged(channel, false);
                    }
                } else {
                    psu::generateError(err);
                }
            }
#endif
		}

		channel.update();
	}

    enableSave(last_save_enabled);
}

bool recallFromProfile(Parameters *profile, int location) {
    bool last_save_enabled = enableSave(false);

    bool result = true;

	for (int i = 0; i < temp_sensor::NUM_TEMP_SENSORS; ++i) {
		memcpy(&temperature::sensors[i].prot_conf, profile->temp_prot + i, sizeof(temperature::ProtectionConfiguration));
	}

    if (profile->flags.powerIsUp) result &= psu::powerUp();
    else psu::powerDown();

    recallChannelsFromProfile(profile, location);

    enableSave(last_save_enabled);

    return result;
}

bool recall(int location) {
    if (location > 0 && location < NUM_PROFILE_LOCATIONS) {
        Parameters profile;
        if (persist_conf::loadProfile(location, &profile) && profile.flags.isValid) {
            if (persist_conf::saveProfile(0, &profile)) {
                if (recallFromProfile(&profile, location)) {
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

bool recallFromFile(const char *filePath, int *err) {
#if OPTION_SD_CARD
    if (sd_card::g_testResult != TEST_OK) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    if (!sd_card::exists(filePath, NULL)) {
        if (err) *err = SCPI_ERROR_LIST_NOT_FOUND;
        return false;
    }

    File file = SD.open(filePath, FILE_READ);

    if (!file) {
        if (err) *err = SCPI_ERROR_EXECUTION_ERROR;
        return false;
    }

    Parameters profile;
    int size = file.read(&profile, sizeof(profile));
    file.close();

    if (size != sizeof(profile) || !persist_conf::check_block((const persist_conf::BlockHeader *)&profile, sizeof(profile), persist_conf::PROFILE_VERSION)) {
        // TODO more specific error
        if (err) *err = SCPI_ERROR_EXECUTION_ERROR;
        return false;
    }


    if (!persist_conf::saveProfile(0, &profile)) {
        // TODO more specific error
        if (err) *err = SCPI_ERROR_EXECUTION_ERROR;
        return false;
    }

    if (!recallFromProfile(&profile, 0)) {
        // TODO more specific error
        if (err) *err = SCPI_ERROR_EXECUTION_ERROR;
		return false;
    }

    event_queue::pushEvent(event_queue::EVENT_INFO_RECALL_FROM_FILE);
    return true;
#else
    if (err) *err = SCPI_ERROR_HARDWARE_MISSING;
    return false;
#endif
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
	if (!profile->flags.isValid || strncmp(profile->name, AUTO_NAME_PREFIX, strlen(AUTO_NAME_PREFIX)) == 0) {
		strcpy(name, AUTO_NAME_PREFIX);
		datetime::getDateTimeAsString(name + strlen(AUTO_NAME_PREFIX));
	} else {
		strcpy(name, profile->name);
	}
}

bool enableSave(bool enable) {
    bool last_save_enabled = g_saveEnabled;
    g_saveEnabled = enable;
    return last_save_enabled;
}

void save() {
    if (!g_saveEnabled) return;
    g_saveProfile = true;
}

void saveImmediately() {
    if (persist_conf::devConf.flags.profileAutoRecallEnabled) {
        DebugTrace("Profile 0 saved!");
        saveAtLocation(0);
        g_saveProfile = false;
    }
}

void fillProfile(Parameters *pProfile, int location, const char *name, const Parameters *pCurrentProfile) {
    Parameters &profile = *pProfile;
    const Parameters& currentProfile = *pCurrentProfile;

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

#ifdef EEZ_PLATFORM_SIMULATOR
			profile.channels[i].load_enabled = channel.simulator.load_enabled;
			profile.channels[i].load = channel.simulator.load;
			profile.channels[i].voltProgExt = channel.simulator.voltProgExt;
#endif

            profile.channels[i].flags.u_triggerMode = channel.flags.voltageTriggerMode;
            profile.channels[i].flags.i_triggerMode = channel.flags.currentTriggerMode;
            profile.channels[i].flags.triggerOutputState = channel.flags.triggerOutputState;
            profile.channels[i].flags.triggerOnListStop = channel.flags.triggerOnListStop;
            profile.channels[i].u_triggerValue = trigger::getVoltage(channel);
            profile.channels[i].i_triggerValue = trigger::getCurrent(channel);
            profile.channels[i].listCount = list::getListCount(channel);

            profile.channels[i].flags.currentRangeSelectionMode = channel.flags.currentRangeSelectionMode;
            profile.channels[i].flags.autoSelectCurrentRange = channel.flags.autoSelectCurrentRange;

#if OPTION_SD_CARD
            if (location != 0 || list::getListsChanged(channel)) {
                char filePath[MAX_PATH_LENGTH];
                getChannelProfileListFilePath(channel, location, filePath);
                if (list::areListLengthsEquivalent(channel)) {
                    list::saveList(channel, filePath, NULL);
                    profile.channels[i].flags.listSaved = 1;
                } else {
                    sd_card::deleteFile(filePath, NULL);
                }
            } else {
                if (pCurrentProfile && currentProfile.flags.isValid) {
                    profile.channels[i].flags.listSaved = currentProfile.channels[i].flags.listSaved;
                } else {
                    profile.channels[i].flags.listSaved = 0;
                }
            }
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
}

bool saveAtLocation(int location, const char *name) {
    if (location >= 0 && location < NUM_PROFILE_LOCATIONS) {
        Parameters currentProfile;

        if (!persist_conf::loadProfile(location, &currentProfile)) {
            currentProfile.flags.isValid = false;
        }

        Parameters profile;
        fillProfile(&profile, location, name, &currentProfile);

        return persist_conf::saveProfile(location, &profile);
    }

    return false;
}

bool saveToFile(const char *filePath, int *err) {
#if OPTION_SD_CARD
    if (sd_card::g_testResult != TEST_OK) {
        if (err) *err = SCPI_ERROR_MASS_STORAGE_ERROR;
        return false;
    }

    sd_card::makeParentDir(filePath);

    sd_card::deleteFile(filePath, NULL);

    File file = SD.open(filePath, FILE_WRITE);

    if (!file) {
        // TODO more specific error
        if (err)*err = SCPI_ERROR_EXECUTION_ERROR;
        return false;
    }

    Parameters profile;
    fillProfile(&profile, 0, NULL, NULL);

    profile.header.version = persist_conf::PROFILE_VERSION;
    profile.header.checksum = persist_conf::calc_checksum((const persist_conf::BlockHeader *)&profile, sizeof(profile));

    size_t size = file.write((const uint8_t *)&profile, sizeof(profile));
    file.close();

    if (size != sizeof(profile)) {
        // TODO more specific error
        *err = SCPI_ERROR_EXECUTION_ERROR;
        return false;
    }

    return true;
#else
    if (err) {
        *err = SCPI_ERROR_HARDWARE_MISSING;
    }
    return false;
#endif
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