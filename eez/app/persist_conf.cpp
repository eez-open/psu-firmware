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

#include "eez/app/psu.h"
#include "eez/app/eeprom.h"
#include "eez/app/serial_psu.h"
#include "eez/app/event_queue.h"
#include "eez/app/profile.h"
#if OPTION_ENCODER
#include "eez/mw/encoder.h"
#endif
#if OPTION_DISPLAY
#include "eez/app/gui/psu.h"
#endif
#if OPTION_ETHERNET
#include "eez/app/ethernet.h"
#endif
#include "eez/app/datetime.h"

#define NUM_CHANNELS_VIEW_MODES 4

namespace eez {
namespace app {
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

static const uint16_t DEV_CONF_VERSION = 9;
static const uint16_t DEV_CONF2_VERSION = 10;
static const uint16_t CH_CAL_CONF_VERSION = 3;

static const uint16_t PERSIST_CONF_DEVICE_ADDRESS = 1024;
static const uint16_t PERSIST_CONF_DEVICE2_ADDRESS = 1536;

static const uint16_t PERSIST_CONF_CH_CAL_ADDRESS = 2048;
static const uint16_t PERSIST_CONF_CH_CAL_BLOCK_SIZE = 512;

static const uint16_t PERSIST_CONF_FIRST_PROFILE_ADDRESS = 5120;
static const uint16_t PERSIST_CONF_PROFILE_BLOCK_SIZE = 1024;

static const uint32_t ONTIME_MAGIC = 0xA7F31B3CL;

////////////////////////////////////////////////////////////////////////////////

DeviceConfiguration devConf;
DeviceConfiguration2 devConf2;

////////////////////////////////////////////////////////////////////////////////

uint32_t calc_checksum(const BlockHeader *block, uint16_t size) {
    return crc32(((const uint8_t *)block) + sizeof(uint32_t), size - sizeof(uint32_t));
}

bool check_block(const BlockHeader *block, uint16_t size, uint16_t version) {
    return block->checksum == calc_checksum(block, size) && block->version <= version;
}

bool save(BlockHeader *block, uint16_t size, uint16_t address, uint16_t version) {
    if (eeprom::g_testResult != TEST_OK) {
        return false;
    }

    block->version = version;
    block->checksum = calc_checksum(block, size);
    return eeprom::write((const uint8_t *)block, size, address);
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
    devConf.flags.isClickSoundEnabled = 1;

    devConf.flags.dateValid = 0;
    devConf.flags.timeValid = 0;
    devConf.flags.dst = 0;

	devConf.time_zone = 0;

    devConf.flags.profileAutoRecallEnabled = 0;
    devConf.profile_auto_recall_location = 0;

    devConf.touch_screen_cal_orientation = -1;
    devConf.touch_screen_cal_tlx = 0;
    devConf.touch_screen_cal_tly = 0;
    devConf.touch_screen_cal_brx = 0;
    devConf.touch_screen_cal_bry = 0;
    devConf.touch_screen_cal_trx = 0;
    devConf.touch_screen_cal_try = 0;

	devConf.flags.channelsViewMode = 0;

#ifdef EEZ_PLATFORM_SIMULATOR
    devConf.flags.ethernetEnabled = 1;
#else
    devConf.flags.ethernetEnabled = 0;
#endif // EEZ_PLATFORM_SIMULATOR

    devConf.flags.outputProtectionCouple = 0;
    devConf.flags.shutdownWhenProtectionTripped = 0;
    devConf.flags.forceDisablingAllOutputsOnPowerUp = 0;

    devConf.flags.ch1CalEnabled = 1;
    devConf.flags.ch2CalEnabled = 1;
}

void loadDevice() {
    if (eeprom::g_testResult == TEST_OK) {
        eeprom::read((uint8_t *)&devConf, sizeof(DeviceConfiguration), get_address(PERSIST_CONF_BLOCK_DEVICE));
        if (!check_block((BlockHeader *)&devConf, sizeof(DeviceConfiguration), DEV_CONF_VERSION)) {
            initDevice();
        } else {
			if (devConf.flags.channelsViewMode < 0 || devConf.flags.channelsViewMode >= NUM_CHANNELS_VIEW_MODES) {
				devConf.flags.channelsViewMode = 0;
			}

            if (devConf.header.version < 9) {
                devConf.flags.ch1CalEnabled = 1;
                devConf.flags.ch2CalEnabled = 1;
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

static void initEthernetSettings() {
    devConf2.flags.ethernetDhcpEnabled = 1;
    devConf2.ethernetIpAddress = getIpAddress(192, 168, 1, 100);
    devConf2.ethernetDns = getIpAddress(192, 168, 1, 1);
    devConf2.ethernetGateway = getIpAddress(192, 168, 1, 1);
    devConf2.ethernetSubnetMask = getIpAddress(255, 255, 255, 0);
    devConf2.ethernetScpiPort = TCP_PORT;
}

static void initDevice2() {
    memset(&devConf2, 0, sizeof(devConf2));
    devConf2.header.version = DEV_CONF2_VERSION;

    devConf2.flags.encoderConfirmationMode = 0;
    devConf2.flags.displayState = 1;

    devConf2.displayBrightness = DISPLAY_BRIGHTNESS_DEFAULT;

    devConf2.flags.serialEnabled = 1;
    devConf2.serialBaud = getIndexFromBaud(SERIAL_SPEED);
    devConf2.serialParity = serial::PARITY_NONE;

    initEthernetSettings();

    strcpy(devConf2.ntpServer, CONF_DEFAULT_NTP_SERVER);

    uint8_t macAddress[] = ETHERNET_MAC_ADDRESS;
    memcpy(devConf2.ethernetMacAddress, macAddress, 6);

	devConf2.dstRule = datetime::DST_RULE_OFF;

    devConf2.displayBackgroundLuminosityStep = DISPLAY_BACKGROUND_LUMINOSITY_STEP_DEFAULT;
#if OPTION_DISPLAY
    gui::lcd::onLuminocityChanged();
#endif
}

void loadDevice2() {
    if (eeprom::g_testResult == TEST_OK) {
        eeprom::read((uint8_t *)&devConf2, sizeof(DeviceConfiguration2), get_address(PERSIST_CONF_BLOCK_DEVICE2));
        if (!check_block((BlockHeader *)&devConf2, sizeof(DeviceConfiguration2), DEV_CONF2_VERSION)) {
            initDevice2();
        } else {
            if (devConf2.header.version < 9) {
                uint8_t macAddress[] = ETHERNET_MAC_ADDRESS;
                memcpy(devConf2.ethernetMacAddress, macAddress, 6);
            }

            if (devConf2.header.version < 10) {
                devConf2.displayBackgroundLuminosityStep = DISPLAY_BACKGROUND_LUMINOSITY_STEP_DEFAULT;
            }
#if OPTION_DISPLAY
            gui::lcd::onLuminocityChanged();
#endif
        }
    }
    else {
        initDevice2();
    }

    if (devConf2.serialBaud < 1 || devConf2.serialBaud > serial::g_baudsSize) {
        devConf2.serialBaud = getIndexFromBaud(SERIAL_SPEED);
    }

#if OPTION_ENCODER
    if (!devConf2.encoderMovingSpeedDown) {
        devConf2.encoderMovingSpeedDown = encoder::DEFAULT_MOVING_DOWN_SPEED;
    }
    if (!devConf2.encoderMovingSpeedUp) {
        devConf2.encoderMovingSpeedUp = encoder::DEFAULT_MOVING_UP_SPEED;
    }
    encoder::setMovingSpeed(devConf2.encoderMovingSpeedDown, devConf2.encoderMovingSpeedUp);
#endif
}

bool saveDevice2() {
    return save((BlockHeader *)&devConf2, sizeof(DeviceConfiguration2), get_address(PERSIST_CONF_BLOCK_DEVICE2), DEV_CONF2_VERSION);
}

bool isSystemPasswordValid(const char *new_password, size_t new_password_len, int16_t &err) {
    if (new_password_len < PASSWORD_MIN_LENGTH) {
		err = SCPI_ERROR_PASSWORD_TOO_SHORT;
		return false;
    }

    if (new_password_len > PASSWORD_MAX_LENGTH) {
		err = SCPI_ERROR_PASSWORD_TOO_LONG;
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
		err = SCPI_ERROR_PASSWORD_TOO_SHORT;
		return false;
    }

    if (new_password_len > PASSWORD_MAX_LENGTH) {
		err = SCPI_ERROR_PASSWORD_TOO_LONG;
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

bool readSystemDate(uint8_t &year, uint8_t &month, uint8_t &day) {
    if (devConf.flags.dateValid) {
        year = devConf.date_year;
        month = devConf.date_month;
        day = devConf.date_day;
        return true;
    }
    return false;
}

bool isDst() {
    return datetime::isDst(
        datetime::makeTime(
            2000 + devConf.date_year, devConf.date_month, devConf.date_day,
            devConf.time_hour, devConf.time_minute, devConf.time_second
        ),
        (datetime::DstRule)devConf2.dstRule
    );
}

void setDst(unsigned dst) {
    if (dst == 0) {
        devConf.flags.dst = 0;
    } else if (dst == 1) {
        devConf.flags.dst = 1;
    } else {
        devConf.flags.dst = isDst();
    }
}

void writeSystemDate(uint8_t year, uint8_t month, uint8_t day, unsigned dst) {
    devConf.date_year = year;
    devConf.date_month = month;
    devConf.date_day = day;

    devConf.flags.dateValid = 1;

    setDst(dst);

    saveDevice();
}

bool readSystemTime(uint8_t &hour, uint8_t &minute, uint8_t &second) {
    if (devConf.flags.timeValid) {
        hour = devConf.time_hour;
        minute = devConf.time_minute;
        second = devConf.time_second;
        return true;
    }
    return false;
}

void writeSystemTime(uint8_t hour, uint8_t minute, uint8_t second, unsigned dst) {
    devConf.time_hour = hour;
    devConf.time_minute = minute;
    devConf.time_second = second;

    devConf.flags.timeValid = 1;

    setDst(dst);

    saveDevice();
}

void writeSystemDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second, unsigned dst) {
    devConf.date_year = year;
    devConf.date_month = month;
    devConf.date_day = day;

    devConf.flags.dateValid = 1;

	devConf.time_hour = hour;
    devConf.time_minute = minute;
    devConf.time_second = second;

    devConf.flags.timeValid = 1;

    setDst(dst);

	saveDevice();
}

bool enableProfileAutoRecall(bool enable) {
    devConf.flags.profileAutoRecallEnabled = enable ? 1 : 0;
    return saveDevice();
}

bool isProfileAutoRecallEnabled() {
    return devConf.flags.profileAutoRecallEnabled ? true : false;
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

void setChannelsViewMode(unsigned int channelsViewMode) {
	devConf.flags.channelsViewMode = channelsViewMode;
	saveDevice();
}

void loadChannelCalibration(Channel &channel) {
    if (eeprom::g_testResult == TEST_OK) {
        eeprom::read((uint8_t *)&channel.cal_conf, sizeof(Channel::CalibrationConfiguration), get_address(PERSIST_CONF_BLOCK_CH_CAL, &channel));
        if (!check_block((BlockHeader *)&channel.cal_conf, sizeof(Channel::CalibrationConfiguration), CH_CAL_CONF_VERSION)) {
            channel.clearCalibrationConf();
        }
    }
    else {
        channel.clearCalibrationConf();
    }
}

bool saveChannelCalibration(Channel &channel) {
    return save((BlockHeader *)&channel.cal_conf, sizeof(Channel::CalibrationConfiguration), get_address(PERSIST_CONF_BLOCK_CH_CAL, &channel), CH_CAL_CONF_VERSION);
}

void saveCalibrationEnabledFlag(Channel &channel, bool enabled) {
    if (channel.index == 1) {
        devConf.flags.ch1CalEnabled = enabled ? 1 : 0;
    } else if (channel.index == 2) {
        devConf.flags.ch2CalEnabled = enabled ? 1 : 0;
    } else {
        return;
    }
    saveDevice();
}

bool loadProfile(int location, profile::Parameters *profile) {
    if (eeprom::g_testResult == TEST_OK) {
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

bool setEncoderSettings(uint8_t confirmationMode, uint8_t movingSpeedDown, uint8_t movingSpeedUp) {
    devConf2.flags.encoderConfirmationMode = confirmationMode;
    devConf2.encoderMovingSpeedDown = movingSpeedDown;
    devConf2.encoderMovingSpeedUp = movingSpeedUp;

#if OPTION_ENCODER
    encoder::setMovingSpeed(devConf2.encoderMovingSpeedDown, devConf2.encoderMovingSpeedUp);
#endif

    return saveDevice2();
}

bool setDisplayState(unsigned newState) {
    unsigned currentDisplayState = devConf2.flags.displayState;

    if (currentDisplayState != newState) {
        devConf2.flags.displayState = newState;
        if (!saveDevice2()) {
            devConf2.flags.displayState = currentDisplayState;
            return false;
        }
    }

    return true;
}

bool setDisplayBrightness(uint8_t displayBrightness) {
    devConf2.displayBrightness = displayBrightness;

#if OPTION_DISPLAY
    gui::lcd::updateBrightness();
#endif

    return saveDevice2();
}

bool setDisplayBackgroundLuminosityStep(uint8_t displayBackgroundLuminosityStep) {
    devConf2.displayBackgroundLuminosityStep = displayBackgroundLuminosityStep;

#if OPTION_DISPLAY
    gui::lcd::onLuminocityChanged();
    gui::refreshPage();
#endif

    return saveDevice2();
}

bool enableSerial(bool enable) {
    unsigned serialEnabled = enable ? 1 : 0;
    if (!devConf2.flags.skipSerialSetup || devConf2.flags.serialEnabled != serialEnabled) {
        devConf2.flags.serialEnabled = serialEnabled;
        devConf2.flags.skipSerialSetup = 1;
        saveDevice2();
        serial::update();
    }
    return true;
}

bool isSerialEnabled() {
    return devConf2.flags.serialEnabled ? true : false;
}

int getIndexFromBaud(long baud) {
    for (size_t i = 0; i < serial::g_baudsSize; ++i) {
        if (serial::g_bauds[i] == baud) {
            return i + 1;
        }
    }
    return 0;
}

long getBaudFromIndex(int index) {
    return serial::g_bauds[index - 1];
}

int getSerialBaudIndex() {
    return devConf2.serialBaud;
}

bool setSerialBaudIndex(int baudIndex) {
    uint8_t serialBaud = (uint8_t)baudIndex;
    if (!devConf2.flags.skipSerialSetup || devConf2.serialBaud != serialBaud) {
        devConf2.serialBaud = serialBaud;
        devConf2.flags.skipSerialSetup = 1;
        saveDevice2();
        serial::update();
    }
    return true;
}

int getSerialParity() {
    return devConf2.serialParity;
}

bool setSerialParity(int parity) {
    unsigned serialParity = (unsigned)parity;
    if (!devConf2.flags.skipSerialSetup || devConf2.serialParity != serialParity) {
        devConf2.serialParity = serialParity;
        devConf2.flags.skipSerialSetup = 1;
        saveDevice2();
        serial::update();
    }
    return true;
}

bool setSerialSettings(bool enabled, int baudIndex, int parity) {
    unsigned serialEnabled = enabled ? 1 : 0;
    uint8_t serialBaud = (uint8_t)baudIndex;
    unsigned serialParity = (unsigned)parity;
    if (
        !devConf2.flags.skipSerialSetup ||
		devConf2.flags.serialEnabled != serialEnabled ||
        devConf2.serialBaud != serialBaud ||
        devConf2.serialParity != serialParity
    ) {
        devConf2.flags.serialEnabled = enabled;
        devConf2.serialBaud = serialBaud;
        devConf2.serialParity = serialParity;
        devConf2.flags.skipSerialSetup = 1;
        saveDevice2();
        serial::update();
    }
    return true;
}

bool enableEthernet(bool enable) {
#if OPTION_ETHERNET
    unsigned ethernetEnabled = enable ? 1 : 0;
    if (!devConf2.flags.skipEthernetSetup || devConf.flags.ethernetEnabled != ethernetEnabled) {
        devConf.flags.ethernetEnabled = ethernetEnabled;
        saveDevice();
        devConf2.flags.skipEthernetSetup = 1;
        saveDevice2();
        event_queue::pushEvent(enable ? event_queue::EVENT_INFO_ETHERNET_ENABLED : event_queue::EVENT_INFO_ETHERNET_DISABLED);
        ethernet::update();
    }
    return true;
#else
    return false;
#endif
}

bool isEthernetEnabled() {
    return devConf.flags.ethernetEnabled ? true : false;
}

bool enableEthernetDhcp(bool enable) {
#if OPTION_ETHERNET
    unsigned ethernetDhcpEnabled = enable ? 1 : 0;
    if (!devConf2.flags.skipEthernetSetup || devConf2.flags.ethernetDhcpEnabled != ethernetDhcpEnabled) {
        devConf2.flags.ethernetDhcpEnabled = ethernetDhcpEnabled;
        devConf2.flags.skipEthernetSetup = 1;
        saveDevice2();
        ethernet::update();
    }
    return true;
#else
    return false;
#endif
}

bool isEthernetDhcpEnabled() {
    return devConf2.flags.ethernetDhcpEnabled ? true : false;
}

bool setEthernetMacAddress(uint8_t macAddress[]) {
#if OPTION_ETHERNET
    if (!devConf2.flags.skipEthernetSetup || memcmp(devConf2.ethernetMacAddress, macAddress, 6) != 0) {
        memcpy(devConf2.ethernetMacAddress, macAddress, 6);
        devConf2.flags.skipEthernetSetup = 1;
        saveDevice2();
        ethernet::update();
    }
    return true;
#else
    return false;
#endif
}

bool setEthernetIpAddress(uint32_t ipAddress) {
#if OPTION_ETHERNET
    if (!devConf2.flags.skipEthernetSetup || devConf2.ethernetIpAddress != ipAddress) {
        devConf2.ethernetIpAddress = ipAddress;
        devConf2.flags.skipEthernetSetup = 1;
        saveDevice2();
        ethernet::update();
    }
    return true;
#else
    return false;
#endif
}

bool setEthernetDns(uint32_t dns) {
#if OPTION_ETHERNET
	if (!devConf2.flags.skipEthernetSetup || devConf2.ethernetDns != dns) {
        devConf2.ethernetDns = dns;
        devConf2.flags.skipEthernetSetup = 1;
        saveDevice2();
        ethernet::update();
    }
    return true;
#else
    return false;
#endif
}

bool setEthernetGateway(uint32_t gateway) {
#if OPTION_ETHERNET
    if (!devConf2.flags.skipEthernetSetup || devConf2.ethernetGateway != gateway) {
        devConf2.ethernetGateway = gateway;
        devConf2.flags.skipEthernetSetup = 1;
        saveDevice2();
        ethernet::update();
    }
    return true;
#else
    return false;
#endif
}

bool setEthernetSubnetMask(uint32_t subnetMask) {
#if OPTION_ETHERNET
    if (!devConf2.flags.skipEthernetSetup || devConf2.ethernetSubnetMask != subnetMask) {
        devConf2.ethernetSubnetMask = subnetMask;
        devConf2.flags.skipEthernetSetup = 1;
        saveDevice2();
        ethernet::update();
    }
    return true;
#else
    return false;
#endif
}

bool setEthernetScpiPort(uint16_t scpiPort) {
#if OPTION_ETHERNET
    if (!devConf2.flags.skipEthernetSetup || devConf2.ethernetScpiPort != scpiPort) {
        devConf2.ethernetScpiPort = scpiPort;
        devConf2.flags.skipEthernetSetup = 1;
        saveDevice2();
        ethernet::update();
    }
    return true;
#else
    return false;
#endif
}

bool setEthernetSettings(bool enable, bool dhcpEnable, uint32_t ipAddress, uint32_t dns, uint32_t gateway, uint32_t subnetMask, uint16_t scpiPort, uint8_t *macAddress) {
#if OPTION_ETHERNET
    unsigned ethernetEnabled = enable ? 1 : 0;
    unsigned ethernetDhcpEnabled = dhcpEnable ? 1 : 0;

    if (
        !devConf2.flags.skipEthernetSetup ||
        devConf.flags.ethernetEnabled != ethernetEnabled ||
        devConf2.flags.ethernetDhcpEnabled != ethernetDhcpEnabled ||
        memcmp(devConf2.ethernetMacAddress, macAddress, 6) != 0 ||
        devConf2.ethernetIpAddress != ipAddress ||
        devConf2.ethernetDns != dns ||
        devConf2.ethernetGateway != gateway ||
        devConf2.ethernetSubnetMask != subnetMask ||
        devConf2.ethernetScpiPort != scpiPort
    ) {

        if (devConf.flags.ethernetEnabled != ethernetEnabled) {
            devConf.flags.ethernetEnabled = ethernetEnabled;
            saveDevice();
            event_queue::pushEvent(devConf.flags.ethernetEnabled ? event_queue::EVENT_INFO_ETHERNET_ENABLED : event_queue::EVENT_INFO_ETHERNET_DISABLED);
        }

        devConf2.flags.ethernetDhcpEnabled = ethernetDhcpEnabled;
        memcpy(devConf2.ethernetMacAddress, macAddress, 6);
        devConf2.ethernetIpAddress = ipAddress;
        devConf2.ethernetDns = dns;
        devConf2.ethernetGateway = gateway;
        devConf2.ethernetSubnetMask = subnetMask;
        devConf2.ethernetScpiPort = scpiPort;
        devConf2.flags.skipEthernetSetup = 1;
        saveDevice2();

        ethernet::update();
    }

    return true;
#else
    return false;
#endif
}

bool enableNtp(bool enable) {
    devConf2.flags.ntpEnabled = enable ? 1 : 0;
    return saveDevice2();
}

bool isNtpEnabled() {
    return devConf2.flags.ntpEnabled ? true : false;
}

bool setNtpServer(const char *ntpServer, size_t ntpServerLength) {
    strncpy(devConf2.ntpServer, ntpServer, ntpServerLength);
    devConf2.ntpServer[ntpServerLength] = 0;
    return saveDevice2();
}

bool setNtpSettings(bool enable, const char *ntpServer) {
    devConf2.flags.ntpEnabled = enable ? 1 : 0;
    strcpy(devConf2.ntpServer, ntpServer);
    return saveDevice2();
}

bool setSdLocked(bool sdLocked) {
    devConf2.flags.sdLocked = sdLocked ? 1 : 0;
    return saveDevice2();
}

bool isSdLocked() {
    return devConf2.flags.sdLocked ? true : false;
}

}
}
} // namespace eez::app::persist_conf
