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
#include "eeprom.h"
#include "event_queue.h"
#include "profile.h"

#define NUM_CHANNELS_VIEW_MODES 4

namespace eez {
namespace psu {
namespace persist_conf {

static const uint16_t PERSIST_CONF_START_ADDRESS = eeprom::EEPROM_START_ADDRESS;

////////////////////////////////////////////////////////////////////////////////

enum PersistConfSection {
    PERSIST_CONF_BLOCK_DEVICE,
    PERSIST_CONF_BLOCK_DEVICE2,
    PERSIST_CONF_BLOCK_CH_CAL,
    PERSIST_CONF_BLOCK_FIRST_PROFILE,
};

////////////////////////////////////////////////////////////////////////////////

static const uint16_t DEV_CONF_VERSION = 0x0008L;
static const uint16_t DEV_CONF2_VERSION = 0x0001L;
static const uint16_t CH_CAL_CONF_VERSION = 0x0003L;
static const uint16_t PROFILE_VERSION = 0x0005L;

static const uint16_t PERSIST_CONF_DEVICE_ADDRESS = 1024;
static const uint16_t PERSIST_CONF_DEVICE2_ADDRESS = 1536;

static const uint16_t PERSIST_CONF_CH_CAL_ADDRESS = 2048;
static const uint16_t PERSIST_CONF_CH_CAL_BLOCK_SIZE = 512;

static const uint16_t PERSIST_CONF_FIRST_PROFILE_ADDRESS = 4096;
static const uint16_t PERSIST_CONF_PROFILE_BLOCK_SIZE = 1024;

static const uint32_t ONTIME_MAGIC = 0xA7F31B3CL;

////////////////////////////////////////////////////////////////////////////////

DeviceConfiguration devConf;
DeviceConfiguration2 devConf2;

////////////////////////////////////////////////////////////////////////////////

uint32_t calc_checksum(const BlockHeader *block, uint16_t size) {
    return util::crc32(((const uint8_t *)block) + sizeof(uint32_t), size - sizeof(uint32_t));
}

bool check_block(const BlockHeader *block, uint16_t size, uint16_t version) {
    return block->checksum == calc_checksum(block, size) && block->version == version;
}

bool save(BlockHeader *block, uint16_t size, uint16_t address, uint16_t version) {
    if (eeprom::test_result == psu::TEST_OK) {
        block->version = version;
        block->checksum = calc_checksum(block, size);
        return eeprom::write((const uint8_t *)block, size, address);
    }
    return true;
}

uint16_t get_address(PersistConfSection section, Channel *channel = 0) {
    switch (section) {
    case PERSIST_CONF_BLOCK_DEVICE:  return PERSIST_CONF_DEVICE_ADDRESS;
    case PERSIST_CONF_BLOCK_DEVICE2:  return PERSIST_CONF_DEVICE2_ADDRESS;
    case PERSIST_CONF_BLOCK_CH_CAL:  return PERSIST_CONF_CH_CAL_ADDRESS + (channel->index - 1) * PERSIST_CONF_CH_CAL_BLOCK_SIZE;
    case PERSIST_CONF_BLOCK_FIRST_PROFILE: return PERSIST_CONF_FIRST_PROFILE_ADDRESS;
    }
    return -1;
}

uint16_t get_profile_address(int location) {
    return get_address(PERSIST_CONF_BLOCK_FIRST_PROFILE) + location * PERSIST_CONF_PROFILE_BLOCK_SIZE;
}

////////////////////////////////////////////////////////////////////////////////

static void initDevice() {
    memset(&devConf, 0, sizeof(devConf));

    devConf.header.checksum = 0;
    devConf.header.version = DEV_CONF_VERSION;

    strcpy(devConf.serialNumber, PSU_SERIAL);

    strcpy(devConf.calibration_password, CALIBRATION_PASSWORD_DEFAULT);

    devConf.flags.isSoundEnabled = 1;
    devConf.flags.isClickSoundEnabled = 0;

    devConf.flags.date_valid = 0;
    devConf.flags.time_valid = 0;
	devConf.flags.dst = 0;

	devConf.time_zone = 0;

    devConf.flags.profile_auto_recall = 1;
    devConf.profile_auto_recall_location = 0;

    devConf.touch_screen_cal_orientation = -1;
    devConf.touch_screen_cal_tlx = 0;
    devConf.touch_screen_cal_tly = 0;
    devConf.touch_screen_cal_brx = 0;
    devConf.touch_screen_cal_bry = 0;
    devConf.touch_screen_cal_trx = 0;
    devConf.touch_screen_cal_try = 0;

	devConf.flags.channelsViewMode = 0;

#ifdef EEZ_PSU_SIMULATOR
    devConf.gui_opened = true;
    devConf.flags.ethernetEnabled = 1;
#else
    devConf.flags.ethernetEnabled = 0;
#endif // EEZ_PSU_SIMULATOR

    devConf.flags.outputProtectionCouple = 0;
    devConf.flags.shutdownWhenProtectionTripped = 0;
    devConf.flags.forceDisablingAllOutputsOnPowerUp = 0;
}

void loadDevice() {
    if (eeprom::test_result == psu::TEST_OK) {
        eeprom::read((uint8_t *)&devConf, sizeof(DeviceConfiguration), get_address(PERSIST_CONF_BLOCK_DEVICE));
        if (!check_block((BlockHeader *)&devConf, sizeof(DeviceConfiguration), DEV_CONF_VERSION)) {
            initDevice();
        } else {
			if (devConf.flags.channelsViewMode < 0 || devConf.flags.channelsViewMode >= NUM_CHANNELS_VIEW_MODES) {
				devConf.flags.channelsViewMode = 0;
			}
		}
    }
    else {
        initDevice();
    }
}

bool saveDevice() {
    return save((BlockHeader *)&devConf, sizeof(DeviceConfiguration), get_address(PERSIST_CONF_BLOCK_DEVICE), DEV_CONF_VERSION);
}

static void initDevice2() {
    memset(&devConf2, 0, sizeof(devConf2));
    devConf2.header.version = DEV_CONF2_VERSION;
}

void loadDevice2() {
    if (eeprom::test_result == psu::TEST_OK) {
        eeprom::read((uint8_t *)&devConf2, sizeof(DeviceConfiguration2), get_address(PERSIST_CONF_BLOCK_DEVICE2));
        if (!check_block((BlockHeader *)&devConf2, sizeof(DeviceConfiguration2), DEV_CONF2_VERSION)) {
            initDevice2();
        }
    }
    else {
        initDevice2();
    }
}

bool saveDevice2() {
    return save((BlockHeader *)&devConf2, sizeof(DeviceConfiguration2), get_address(PERSIST_CONF_BLOCK_DEVICE2), DEV_CONF2_VERSION);
}

bool isSystemPasswordValid(const char *new_password, size_t new_password_len, int16_t &err) {
    if (new_password_len < PASSWORD_MIN_LENGTH) {
		err = SCPI_ERROR_SYS_PASSWORD_TOO_SHORT;
		return false;
    }

    if (new_password_len > PASSWORD_MAX_LENGTH) {
		err = SCPI_ERROR_SYS_PASSWORD_TOO_LONG;
        return false;
    }

	return true;
}

bool changeSystemPassword(const char *new_password, size_t new_password_len) {
    memset(&devConf2.systemPassword, 0, sizeof(devConf2.systemPassword));
    strncpy(devConf2.systemPassword, new_password, new_password_len);
    if (saveDevice2()) {
		event_queue::pushEvent(event_queue::EVENT_INFO_SYSTEM_PASSWORD_CHANGED);
		return true;
	}
	return false;
}

bool isCalibrationPasswordValid(const char *new_password, size_t new_password_len, int16_t &err) {
    if (new_password_len < PASSWORD_MIN_LENGTH) {
		err = SCPI_ERROR_CAL_PASSWORD_TOO_SHORT;
		return false;
    }

    if (new_password_len > PASSWORD_MAX_LENGTH) {
		err = SCPI_ERROR_CAL_PASSWORD_TOO_LONG;
        return false;
    }

	return true;
}

bool changeCalibrationPassword(const char *new_password, size_t new_password_len) {
    memset(&devConf.calibration_password, 0, sizeof(devConf.calibration_password));
    strncpy(devConf.calibration_password, new_password, new_password_len);
    if (saveDevice()) {
		event_queue::pushEvent(event_queue::EVENT_INFO_CALIBRATION_PASSWORD_CHANGED);
		return true;
	}
	return false;
}

bool changeSerial(const char *newSerialNumber, size_t newSerialNumberLength) {
    // copy up to 7 characters from newSerialNumber, fill the rest with zero's
    for (size_t i = 0; i < 7; ++i) {
        if (i < newSerialNumberLength) {
            devConf.serialNumber[i] = newSerialNumber[i];
        } else {
            devConf.serialNumber[i] = 0;
        }
    }
    devConf.serialNumber[7] = 0;

    return saveDevice();
}

bool enableSound(bool enable) {
    devConf.flags.isSoundEnabled = enable ? 1 : 0;
    if (saveDevice()) {
		event_queue::pushEvent(enable ? event_queue::EVENT_INFO_SOUND_ENABLED : event_queue::EVENT_INFO_SOUND_DISABLED);
		return true;
	}
	return false;
}

bool isSoundEnabled() {
    return devConf.flags.isSoundEnabled ? true : false;
}

bool enableClickSound(bool enable) {
    devConf.flags.isClickSoundEnabled = enable ? 1 : 0;
    if (saveDevice()) {
		return true;
	}
	return false;
}

bool isClickSoundEnabled() {
    return devConf.flags.isClickSoundEnabled ? true : false;
}

bool enableEthernet(bool enable) {
    devConf.flags.ethernetEnabled = enable ? 1 : 0;
    if (saveDevice()) {
		event_queue::pushEvent(enable ? event_queue::EVENT_INFO_ETHERNET_ENABLED : event_queue::EVENT_INFO_ETHERNET_DISABLED);
		return true;
	}
	return false;
}

bool isEthernetEnabled() {
    return devConf.flags.ethernetEnabled ? true : false;
}

bool readSystemDate(uint8_t &year, uint8_t &month, uint8_t &day) {
    if (devConf.flags.date_valid) {
        year = devConf.date_year;
        month = devConf.date_month;
        day = devConf.date_day;
        return true;
    }
    return false;
}

void writeSystemDate(uint8_t year, uint8_t month, uint8_t day) {
    devConf.date_year = year;
    devConf.date_month = month;
    devConf.date_day = day;
    devConf.flags.date_valid = 1;
    saveDevice();
}

bool readSystemTime(uint8_t &hour, uint8_t &minute, uint8_t &second) {
    if (devConf.flags.time_valid) {
        hour = devConf.time_hour;
        minute = devConf.time_minute;
        second = devConf.time_second;
        return true;
    }
    return false;
}

void writeSystemTime(uint8_t hour, uint8_t minute, uint8_t second) {
    devConf.time_hour = hour;
    devConf.time_minute = minute;
    devConf.time_second = second;
    devConf.flags.time_valid = 1;
    saveDevice();
}

void writeSystemDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
    devConf.date_year = year;
    devConf.date_month = month;
    devConf.date_day = day;
    devConf.flags.date_valid = 1;
    
	devConf.time_hour = hour;
    devConf.time_minute = minute;
    devConf.time_second = second;
    devConf.flags.time_valid = 1;

	saveDevice();
}

bool enableProfileAutoRecall(bool enable) {
    devConf.flags.profile_auto_recall = enable ? 1 : 0;
    return saveDevice();
}

bool isProfileAutoRecallEnabled() {
    return devConf.flags.profile_auto_recall ? true : false;
}

bool setProfileAutoRecallLocation(int location) {
    devConf.profile_auto_recall_location = (int8_t)location;
    if (saveDevice()) {
		event_queue::pushEvent(event_queue::EVENT_INFO_DEFAULE_PROFILE_CHANGED_TO_0 + location);

        if (location == 0) {
            profile::save();
        }

		return true;
	}
    return false;
}

int getProfileAutoRecallLocation() {
    return devConf.profile_auto_recall_location;
}

void toggleChannelsViewMode() {
	devConf.flags.channelsViewMode = (devConf.flags.channelsViewMode + 1) % NUM_CHANNELS_VIEW_MODES;
	saveDevice();
}

void loadChannelCalibration(Channel *channel) {
    if (eeprom::test_result == psu::TEST_OK) {
        eeprom::read((uint8_t *)&channel->cal_conf, sizeof(Channel::CalibrationConfiguration), get_address(PERSIST_CONF_BLOCK_CH_CAL, channel));
        if (!check_block((BlockHeader *)&channel->cal_conf, sizeof(Channel::CalibrationConfiguration), CH_CAL_CONF_VERSION)) {
            channel->clearCalibrationConf();
        }
    }
    else {
        channel->clearCalibrationConf();
    }
}

bool saveChannelCalibration(Channel *channel) {
    return save((BlockHeader *)&channel->cal_conf, sizeof(Channel::CalibrationConfiguration), get_address(PERSIST_CONF_BLOCK_CH_CAL, channel), CH_CAL_CONF_VERSION);
}

bool loadProfile(int location, profile::Parameters *profile) {
    if (eeprom::test_result == psu::TEST_OK) {
        eeprom::read((uint8_t *)profile, sizeof(profile::Parameters), get_profile_address(location));
        return check_block((BlockHeader *)profile, sizeof(profile::Parameters), PROFILE_VERSION);
    }
    return false;
}

bool saveProfile(int location, profile::Parameters *profile) {
    return save((BlockHeader *)profile, sizeof(profile::Parameters), get_profile_address(location), PROFILE_VERSION);
}

uint32_t readTotalOnTime(int type) {
	uint32_t buffer[6];

	eeprom::read((uint8_t *)buffer, sizeof(buffer),
		eeprom::EEPROM_ONTIME_START_ADDRESS + type * eeprom::EEPROM_ONTIME_SIZE);

	if (buffer[0] == ONTIME_MAGIC && buffer[1] == buffer[2]) {
		if (buffer[3] == ONTIME_MAGIC && buffer[4] == buffer[5]) {
			if (buffer[4] > buffer[1]) {
				return buffer[4];
			}
		}
		return buffer[1];
	}
	
	if (buffer[3] == ONTIME_MAGIC && buffer[4] == buffer[5]) {
		return buffer[4];
	}

	return 0;
}

bool writeTotalOnTime(int type, uint32_t time) {
	uint32_t buffer[6];

	buffer[0] = ONTIME_MAGIC;
	buffer[1] = time;
	buffer[2] = time;

	buffer[3] = ONTIME_MAGIC;
	buffer[4] = time;
	buffer[5] = time;

	return eeprom::write((uint8_t *)buffer, sizeof(buffer),
		eeprom::EEPROM_ONTIME_START_ADDRESS + type * eeprom::EEPROM_ONTIME_SIZE);
}

bool enableOutputProtectionCouple(bool enable) {
    int outputProtectionCouple = enable ? 1 : 0;

    if (devConf.flags.outputProtectionCouple == outputProtectionCouple) {
        return true;
    }

    devConf.flags.outputProtectionCouple = outputProtectionCouple;

    if (saveDevice()) {
        if (devConf.flags.outputProtectionCouple) {
		    event_queue::pushEvent(event_queue::EVENT_INFO_OUTPUT_PROTECTION_COUPLED);
        } else {
		    event_queue::pushEvent(event_queue::EVENT_INFO_OUTPUT_PROTECTION_DECOUPLED);
        }

        return true;
    }

    devConf.flags.outputProtectionCouple = 1 - outputProtectionCouple;

    return false;
}

bool isOutputProtectionCoupleEnabled() {
    return devConf.flags.outputProtectionCouple ? true : false;
}

bool enableShutdownWhenProtectionTripped(bool enable) {
    int shutdownWhenProtectionTripped = enable ? 1 : 0;

    if (devConf.flags.shutdownWhenProtectionTripped == shutdownWhenProtectionTripped) {
        return true;
    }

    devConf.flags.shutdownWhenProtectionTripped = shutdownWhenProtectionTripped;

    if (saveDevice()) {
        if (devConf.flags.shutdownWhenProtectionTripped) {
		    event_queue::pushEvent(event_queue::EVENT_INFO_SHUTDOWN_WHEN_PROTECTION_TRIPPED_ENABLED);
        } else {
		    event_queue::pushEvent(event_queue::EVENT_INFO_SHUTDOWN_WHEN_PROTECTION_TRIPPED_DISABLED);
        }

        return true;
    }

    devConf.flags.shutdownWhenProtectionTripped = 1 - shutdownWhenProtectionTripped;

    return false;
}

bool isShutdownWhenProtectionTrippedEnabled() {
    return devConf.flags.shutdownWhenProtectionTripped ? true : false;
}

bool enableForceDisablingAllOutputsOnPowerUp(bool enable) {
    int forceDisablingAllOutputsOnPowerUp = enable ? 1 : 0;

    if (devConf.flags.forceDisablingAllOutputsOnPowerUp == forceDisablingAllOutputsOnPowerUp) {
        return true;
    }

    devConf.flags.forceDisablingAllOutputsOnPowerUp = forceDisablingAllOutputsOnPowerUp;

    if (saveDevice()) {
        if (devConf.flags.forceDisablingAllOutputsOnPowerUp) {
		    event_queue::pushEvent(event_queue::EVENT_INFO_FORCE_DISABLING_ALL_OUTPUTS_ON_POWERUP_ENABLED);
        } else {
		    event_queue::pushEvent(event_queue::EVENT_INFO_FORCE_DISABLING_ALL_OUTPUTS_ON_POWERUP_DISABLED);
        }

        return true;
    }

    devConf.flags.forceDisablingAllOutputsOnPowerUp = 1 - forceDisablingAllOutputsOnPowerUp;

    return false;
}

bool isForceDisablingAllOutputsOnPowerUpEnabled() {
    return devConf.flags.forceDisablingAllOutputsOnPowerUp ? true : false;
}

bool lockFrontPanel(bool lock) {
    g_rlState = lock ? RL_STATE_REMOTE : RL_STATE_LOCAL;

    int isFrontPanelLocked = lock ? 1 : 0;

    if (devConf.flags.isFrontPanelLocked == isFrontPanelLocked) {
        return true;
    }

    devConf.flags.isFrontPanelLocked = isFrontPanelLocked;

    if (saveDevice()) {
        if (devConf.flags.isFrontPanelLocked) {
		    event_queue::pushEvent(event_queue::EVENT_INFO_FRONT_PANEL_LOCKED);
        } else {
		    event_queue::pushEvent(event_queue::EVENT_INFO_FRONT_PANEL_UNLOCKED);
        }

        return true;
    }

    devConf.flags.isFrontPanelLocked = 1 - isFrontPanelLocked;
    g_rlState = devConf.flags.isFrontPanelLocked ? RL_STATE_REMOTE : RL_STATE_LOCAL;

    return false;
}

}
}
} // namespace eez::psu::persist_conf