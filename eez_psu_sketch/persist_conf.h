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

class Channel;

namespace profile {

struct Parameters;

}
}
} // namespace eez::psu::profile

namespace eez {
namespace psu {
/// Store/restore of persistent configuration data (device configuration, calibration parameters, profiles) using external EEPROM.
namespace persist_conf {

/// Header of the every block stored in EEPROM. It contains checksum and version.
struct BlockHeader {
    uint32_t checksum;
    uint16_t version;
};

/// Device binary flags stored in DeviceConfiguration.
struct DeviceFlags {
    int beep_enabled : 1;
    int date_valid : 1;
    int time_valid : 1;
    int profile_auto_recall : 1;
    int dst : 1;
    int channelDisplayedValues : 3;
    int ethernetEnabled : 1;
    int outputProtectionCouple : 1;
    int shutdownWhenProtectionTripped : 1;
    int forceDisablingAllOutputsOnPowerUp : 1;
    int reserved1 : 1;
    int reserved2 : 1;
    int reserved3 : 1;
    int reserved4 : 1;
};

/// Device configuration block.
struct DeviceConfiguration {
    BlockHeader header;
    char serialNumber[7 + 1];
    char calibration_password[PASSWORD_MAX_LENGTH + 1];
    DeviceFlags flags;
    uint8_t date_year;
    uint8_t date_month;
    uint8_t date_day;
    uint8_t time_hour;
    uint8_t time_minute;
    uint8_t time_second;
	int16_t time_zone;
    int8_t profile_auto_recall_location;
    int8_t touch_screen_cal_orientation;
    int16_t touch_screen_cal_tlx;
    int16_t touch_screen_cal_tly;
    int16_t touch_screen_cal_brx;
    int16_t touch_screen_cal_bry;
    int16_t touch_screen_cal_trx;
    int16_t touch_screen_cal_try;
#ifdef EEZ_PSU_SIMULATOR
    bool gui_opened;
#endif // EEZ_PSU_SIMULATOR
};

extern DeviceConfiguration dev_conf;

void loadDevice();
bool saveDevice();

bool isPasswordValid(const char *new_password, size_t new_password_len, int16_t &err);
bool changePassword(const char *new_password, size_t new_password_len);

bool changeSerial(const char *newSerialNumber, size_t newSerialNumberLength);

bool enableBeep(bool enable);
bool isBeepEnabled();

bool enableEthernet(bool enable);
bool isEthernetEnabled();

bool readSystemDate(uint8_t &year, uint8_t &month, uint8_t &day);
void writeSystemDate(uint8_t year, uint8_t month, uint8_t day);

bool enableProfileAutoRecall(bool enable);
bool isProfileAutoRecallEnabled();
bool setProfileAutoRecallLocation(int location);
int getProfileAutoRecallLocation();

bool readSystemTime(uint8_t &hour, uint8_t &minute, uint8_t &second);
void writeSystemTime(uint8_t hour, uint8_t minute, uint8_t second);

void writeSystemDateTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);

void toggleChannelDisplayedValues();

void loadChannelCalibration(Channel *channel);
bool saveChannelCalibration(Channel *channel);

bool loadProfile(int location, profile::Parameters *profile);
bool saveProfile(int location, profile::Parameters *profile);

uint32_t readTotalOnTime(int type);
bool writeTotalOnTime(int type, uint32_t time);

bool enableOutputProtectionCouple(bool enable);
bool isOutputProtectionCoupleEnabled();

bool enableShutdownWhenProtectionTripped(bool enable);
bool isShutdownWhenProtectionTrippedEnabled();

bool enableForceDisablingAllOutputsOnPowerUp(bool enable);
bool isForceDisablingAllOutputsOnPowerUpEnabled();

}
}
} // namespace eez::psu::persist_conf
