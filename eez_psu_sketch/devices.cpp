/*
 * EEZ PSU Firmware
 * Copyright (C) 2016-present, Envox d.o.o.
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
#include "devices.h"

#include "eeprom.h"

#if OPTION_SD_CARD
#include "sd_card.h"
#endif

#if OPTION_ETHERNET
#include "ethernet.h"
#endif

#include "rtc.h"
#include "datetime.h"
#include "bp.h"
#include "temp_sensor.h"
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
#include "fan.h"
#endif

namespace eez {
namespace psu {
namespace devices {

#define TEMP_SENSOR(NAME, INSTALLED, PIN, CAL_POINTS, CH_NUM, QUES_REG_BIT, SCPI_ERROR) { #NAME" temp", INSTALLED, &temp_sensor::sensors[temp_sensor::NAME].g_testResult }

#define CHANNEL(INDEX, BOARD_REVISION, PINS, PARAMS) \
    { "CH"#INDEX" IOEXP", true, &Channel::get(INDEX-1).ioexp.g_testResult }, \
    { "CH"#INDEX" DAC", true, &Channel::get(INDEX-1).dac.g_testResult }, \
    { "CH"#INDEX" ADC", true, &Channel::get(INDEX-1).adc.g_testResult }

Device devices[] = {
    { "EEPROM", OPTION_EXT_EEPROM, &eeprom::g_testResult },
#if OPTION_SD_CARD
    { "SD card", OPTION_SD_CARD, &sd_card::g_testResult },
#else
    { "SD card", 0, 0 },
#endif
#if OPTION_ETHERNET
    { "Ethernet", 1, &ethernet::g_testResult },
#else
    { "Ethernet", 0, 0 },
#endif
    { "RTC", OPTION_EXT_RTC, &rtc::g_testResult },
    { "DateTime", true, &datetime::g_testResult },
    { "BP option", OPTION_BP, &bp::g_testResult },
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
    { "Fan", OPTION_FAN, &fan::g_testResult },
#endif
    TEMP_SENSORS,
    CHANNELS
};

#undef TEMP_SENSOR
#undef CHANNEL

int numDevices = sizeof(devices) / sizeof(Device);

bool anyFailed() {
    for (int i = 0; i < numDevices; ++i) {
        Device &device = devices[i];
        if (device.testResult != 0 && *device.testResult == TEST_FAILED) {
            return true;
        }
    }

    return false;
}

#define APPEND_CHAR(C) \
    nextIndex = index + 1; \
    if (nextIndex > MAX_LENGTH) break; \
    *(result + index) = C; \
    index = nextIndex;

#define APPEND_STRING(S) \
    str = S; \
    strLength = strlen(str); \
    nextIndex = index + strLength; \
    if (nextIndex > MAX_LENGTH) break; \
    strncpy(result + index, str, strLength); \
    index = nextIndex

char *getSelfTestResultString() {
    const int MAX_LENGTH = 255;
    char *result = (char *)malloc(MAX_LENGTH + 1);
    int index = 0;
    int nextIndex;
    const char* str;
    int strLength;

    for (int deviceIndex = 0; deviceIndex < numDevices; ++deviceIndex) {
        Device &device = devices[deviceIndex];
        if (device.testResult && *device.testResult == TEST_FAILED) {
            if (index > 0) {
                APPEND_CHAR('\n');
            }
            APPEND_CHAR('-');
            APPEND_CHAR(' ');
            APPEND_STRING(device.deviceName);
            APPEND_CHAR(' ');
            APPEND_STRING(getTestResultString(*device.testResult));
        }
    }

    *(result + index) = 0;

    return result;
}

const char *getInstalledString(bool installed) {
    if (installed)
        return "installed";
    return "not installed";
}

const char *getTestResultString(psu::TestResult g_testResult) {
    if (g_testResult == psu::TEST_OK)
        return "passed";
    if (g_testResult == psu::TEST_SKIPPED)
        return "skipped";
    if (g_testResult == psu::TEST_WARNING)
        return "warning";
    return "failed";
}


}
}
} // namespace eez::psu::devices
