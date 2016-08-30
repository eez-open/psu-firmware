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
#include "profile.h"

namespace eez {
namespace psu {
namespace persist_conf {

static const uint16_t PERSIST_CONF_START_ADDRESS = eeprom::EEPROM_START_ADDRESS;

////////////////////////////////////////////////////////////////////////////////

enum PersistConfSection {
    PERSIST_CONF_BLOCK_DEVICE,
    PERSIST_CONF_BLOCK_CH_CAL,
    PERSIST_CONF_BLOCK_FIRST_PROFILE,
};

////////////////////////////////////////////////////////////////////////////////

static const uint16_t DEV_CONF_VERSION = 0x0005L;
static const uint16_t CH_CAL_CONF_VERSION = 0x0001L;
static const uint16_t PROFILE_VERSION = 0x0004L;

static const uint16_t PERSIST_CONF_DEVICE_ADDRESS = 1024;

static const uint16_t PERSIST_CONF_CH_CAL_ADDRESS = 2048;
static const uint16_t PERSIST_CONF_CH_CAL_BLOCK_SIZE = 512;

static const uint16_t PERSIST_CONF_FIRST_PROFILE_ADDRESS = 4096;
static const uint16_t PERSIST_CONF_PROFILE_BLOCK_SIZE = 1024;

static const uint32_t ONTIME_MAGIC = 0xA7F11B3CL;

////////////////////////////////////////////////////////////////////////////////

DeviceConfiguration dev_conf;
uint8_t last_total_ontime_address_index;

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
    case PERSIST_CONF_BLOCK_CH_CAL:  return PERSIST_CONF_CH_CAL_ADDRESS + (channel->index - 1) * PERSIST_CONF_CH_CAL_BLOCK_SIZE;
    case PERSIST_CONF_BLOCK_FIRST_PROFILE: return PERSIST_CONF_FIRST_PROFILE_ADDRESS;
    }
    return -1;
}

uint16_t get_profile_address(int location) {
    return get_address(PERSIST_CONF_BLOCK_FIRST_PROFILE) + location * PERSIST_CONF_PROFILE_BLOCK_SIZE;
}

////////////////////////////////////////////////////////////////////////////////

void initDevice() {
    dev_conf.header.checksum = 0;
    dev_conf.header.version = DEV_CONF_VERSION;

    strcpy(dev_conf.calibration_password, CALIBRATION_PASSWORD_DEFAULT);

    dev_conf.flags.beep_enabled = 1;

    dev_conf.flags.date_valid = 0;
    dev_conf.flags.time_valid = 0;

    dev_conf.flags.profile_auto_recall = 1;
    dev_conf.profile_auto_recall_location = 0;

    dev_conf.touch_screen_cal_orientation = -1;
    dev_conf.touch_screen_cal_tlx = 0;
    dev_conf.touch_screen_cal_tly = 0;
    dev_conf.touch_screen_cal_brx = 0;
    dev_conf.touch_screen_cal_bry = 0;
    dev_conf.touch_screen_cal_trx = 0;
    dev_conf.touch_screen_cal_try = 0;

#ifdef EEZ_PSU_SIMULATOR
    dev_conf.gui_opened = false;
#endif // EEZ_PSU_SIMULATOR
}

void loadDevice() {
    if (eeprom::test_result == psu::TEST_OK) {
        eeprom::read((uint8_t *)&dev_conf, sizeof(DeviceConfiguration), get_address(PERSIST_CONF_BLOCK_DEVICE));
        if (!check_block((BlockHeader *)&dev_conf, sizeof(DeviceConfiguration), DEV_CONF_VERSION)) {
            initDevice();
        }
    }
    else {
        initDevice();
    }
}

bool saveDevice() {
    return save((BlockHeader *)&dev_conf, sizeof(DeviceConfiguration), get_address(PERSIST_CONF_BLOCK_DEVICE), DEV_CONF_VERSION);
}

bool isPasswordValid(const char *new_password, size_t new_password_len, int16_t &err) {
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

bool changePassword(const char *new_password, size_t new_password_len) {
    memset(&dev_conf.calibration_password, 0, sizeof(dev_conf.calibration_password));
    strncpy(dev_conf.calibration_password, new_password, new_password_len);
    return saveDevice();
}

void enableBeep(bool enable) {
    dev_conf.flags.beep_enabled = enable ? 1 : 0;
    saveDevice();
}

bool isBeepEnabled() {
    return dev_conf.flags.beep_enabled ? true : false;
}

bool readSystemDate(uint8_t &year, uint8_t &month, uint8_t &day) {
    if (dev_conf.flags.date_valid) {
        year = dev_conf.date_year;
        month = dev_conf.date_month;
        day = dev_conf.date_day;
        return true;
    }
    return false;
}

void writeSystemDate(uint8_t year, uint8_t month, uint8_t day) {
    dev_conf.date_year = year;
    dev_conf.date_month = month;
    dev_conf.date_day = day;
    dev_conf.flags.date_valid = 1;
    saveDevice();
}

bool readSystemTime(uint8_t &hour, uint8_t &minute, uint8_t &second) {
    if (dev_conf.flags.time_valid) {
        hour = dev_conf.time_hour;
        minute = dev_conf.time_minute;
        second = dev_conf.time_second;
        return true;
    }
    return false;
}

void writeSystemTime(uint8_t hour, uint8_t minute, uint8_t second) {
    dev_conf.time_hour = hour;
    dev_conf.time_minute = minute;
    dev_conf.time_second = second;
    dev_conf.flags.time_valid = 1;
    saveDevice();
}

bool enableProfileAutoRecall(bool enable) {
    dev_conf.flags.profile_auto_recall = enable ? 1 : 0;
    return saveDevice();
}

bool isProfileAutoRecallEnabled() {
    return dev_conf.flags.profile_auto_recall ? true : false;
}

bool setProfileAutoRecallLocation(int location) {
    dev_conf.profile_auto_recall_location = (int8_t)location;
    return saveDevice();
}

int getProfileAutoRecallLocation() {
    return dev_conf.profile_auto_recall_location;
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
		eeprom::EEPROM_ONTIME_START_ADDRESS +
		type * 6 * sizeof(uint32_t));

	if (buffer[0] == ONTIME_MAGIC && buffer[1] == buffer[2]) {
		if (buffer[3] == ONTIME_MAGIC && buffer[4] == buffer[5]) {
			if (buffer[4] > buffer[1]) {
				last_total_ontime_address_index = 1;
				return buffer[4];
			}
		}
		last_total_ontime_address_index = 0;
		return buffer[1];
	}
	
	if (buffer[3] == ONTIME_MAGIC && buffer[4] == buffer[5]) {
		last_total_ontime_address_index = 1;
		return buffer[4];
	}

	return 0;
}

bool writeTotalOnTime(int type, uint32_t time) {
	uint32_t buffer[3];

	buffer[0] = ONTIME_MAGIC;
	buffer[1] = time;
	buffer[2] = time;

	if (last_total_ontime_address_index == 0) last_total_ontime_address_index = 1;
	else last_total_ontime_address_index = 0;

	return eeprom::write((uint8_t *)buffer, sizeof(buffer),
		eeprom::EEPROM_ONTIME_START_ADDRESS +
		type * 6 * sizeof(uint32_t) +
		last_total_ontime_address_index * 3 * sizeof(uint32_t));
}

}
}
} // namespace eez::psu::persist_conf