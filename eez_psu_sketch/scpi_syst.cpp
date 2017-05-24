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
#include "scpi_psu.h"

#include "serial_psu.h"
#include "ethernet.h"
#include "datetime.h"
#include "sound.h"
#include "profile.h"
#include "channel_dispatcher.h"
#if OPTION_DISPLAY
#include "gui.h"
#endif

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cmd_systemCapabilityQ(scpi_t *context) {
    char text[sizeof(STR_SYST_CAP)];
    strcpy_P(text, PSTR(STR_SYST_CAP));
    SCPI_ResultText(context, text);
    
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemErrorNextQ(scpi_t *context) {
    return SCPI_SystemErrorNextQ(context);
}

scpi_result_t scpi_cmd_systemErrorCountQ(scpi_t *context) {
    return SCPI_SystemErrorCountQ(context);
}

scpi_result_t scpi_cmd_systemVersionQ(scpi_t *context) {
    return SCPI_SystemVersionQ(context);
}

scpi_result_t scpi_cmd_systemPower(scpi_t *context) {
    bool up;
    if (!SCPI_ParamBool(context, &up, TRUE)) {
        return SCPI_RES_ERR;
    }

#if OPTION_AUX_TEMP_SENSOR
    if (temperature::sensors[temp_sensor::AUX].isTripped()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CANNOT_EXECUTE_BEFORE_CLEARING_PROTECTION);
        return SCPI_RES_ERR;
    }
#endif

    if (!psu::changePowerState(up)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemPowerQ(scpi_t *context) {
    SCPI_ResultBool(context, psu::isPowerUp());
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemDate(scpi_t *context) {
    int32_t year;
    if (!SCPI_ParamInt(context, &year, TRUE)) {
        return SCPI_RES_ERR;
    }

    int32_t month;
    if (!SCPI_ParamInt(context, &month, TRUE)) {
        return SCPI_RES_ERR;
    }

    int32_t day;
    if (!SCPI_ParamInt(context, &day, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (year < 2000 || year > 2099) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    year = year - 2000;

    if (!datetime::isValidDate((uint8_t)year, (uint8_t)month, (uint8_t)day)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }

    if (!datetime::setDate((uint8_t)year, (uint8_t)month, (uint8_t)day)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemDateClear(scpi_t *context) {
    persist_conf::devConf.flags.dateValid = 0;
    persist_conf::saveDevice();
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemDateQ(scpi_t *context) {
    uint8_t year, month, day;
    if (!datetime::getDate(year, month, day)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    char buffer[16] = { 0 };
    sprintf_P(buffer, PSTR("%d, %d, %d"), (int)(year + 2000), (int)month, (int)day);
    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemTime(scpi_t *context) {
    int32_t hour;
    if (!SCPI_ParamInt(context, &hour, TRUE)) {
        return SCPI_RES_ERR;
    }

    int32_t minute;
    if (!SCPI_ParamInt(context, &minute, TRUE)) {
        return SCPI_RES_ERR;
    }

    int32_t second;
    if (!SCPI_ParamInt(context, &second, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (!datetime::isValidTime((uint8_t)hour, (uint8_t)minute, (uint8_t)second)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }

    if (!datetime::setTime((uint8_t)hour, (uint8_t)minute, (uint8_t)second)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemTimeClear(scpi_t *context) {
    persist_conf::devConf.flags.timeValid = 0;
    persist_conf::saveDevice();
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemTimeQ(scpi_t *context) {
    uint8_t hour, minute, second;
    if (!datetime::getTime(hour, minute, second)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    char buffer[16] = { 0 };
    sprintf_P(buffer, PSTR("%d, %d, %d"), (int)hour, (int)minute, (int)second);
    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemBeeperImmediate(scpi_t *context) {
    sound::playBeep(true);
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemBeeperState(scpi_t *context) {
    bool enable;
    if (!SCPI_ParamBool(context, &enable, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (enable != persist_conf::isSoundEnabled()) {
		if (!persist_conf::enableSound(enable)) {
			SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
			return SCPI_RES_ERR;
		}
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemBeeperStateQ(scpi_t *context) {
    SCPI_ResultBool(context, persist_conf::isSoundEnabled());
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemBeeperKeyState(scpi_t *context) {
    bool enable;
    if (!SCPI_ParamBool(context, &enable, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (enable != persist_conf::isClickSoundEnabled()) {
		if (!persist_conf::enableClickSound(enable)) {
			SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
			return SCPI_RES_ERR;
		}
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemBeeperKeyStateQ(scpi_t *context) {
    SCPI_ResultBool(context, persist_conf::isClickSoundEnabled());
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemTemperatureProtectionHighClear(scpi_t *context) {
    int32_t sensor;
    if (!param_temp_sensor(context, sensor)) {
		return SCPI_RES_ERR;
    }

    channel_dispatcher::clearOtpProtection(sensor);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemTemperatureProtectionHighLevel(scpi_t *context) {
    float level;
    if (!get_temperature_param(context, level, OTP_AUX_MIN_LEVEL, OTP_AUX_MAX_LEVEL, OTP_AUX_DEFAULT_LEVEL)) {
        return SCPI_RES_ERR;
    }

    int32_t sensor;
    if (!param_temp_sensor(context, sensor)) {
		return SCPI_RES_ERR;
    }

    channel_dispatcher::setOtpLevel(sensor, level);
    profile::save();

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemTemperatureProtectionHighLevelQ(scpi_t *context) {
    int32_t sensor;
    if (!param_temp_sensor(context, sensor)) {
		return SCPI_RES_ERR;
    }

    return result_float(context, 0, temperature::sensors[sensor].prot_conf.level, VALUE_TYPE_FLOAT_CELSIUS);
}

scpi_result_t scpi_cmd_systemTemperatureProtectionHighState(scpi_t *context) {
    bool state;
    if (!SCPI_ParamBool(context, &state, TRUE)) {
        return SCPI_RES_ERR;
    }

    int32_t sensor;
    if (!param_temp_sensor(context, sensor)) {
		return SCPI_RES_ERR;
    }

    channel_dispatcher::setOtpState(sensor, state);
    profile::save();

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemTemperatureProtectionHighStateQ(scpi_t *context) {
    int32_t sensor;
    if (!param_temp_sensor(context, sensor)) {
		return SCPI_RES_ERR;
    }

    SCPI_ResultBool(context, temperature::sensors[sensor].prot_conf.state);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemTemperatureProtectionHighDelayTime(scpi_t *context) {
    float delay;
    if (!get_duration_param(context, delay, OTP_AUX_MIN_DELAY, OTP_AUX_MAX_DELAY, OTP_AUX_DEFAULT_DELAY)) {
        return SCPI_RES_ERR;
    }

    int32_t sensor;
    if (!param_temp_sensor(context, sensor)) {
		return SCPI_RES_ERR;
    }

    channel_dispatcher::setOtpDelay(sensor, delay);
    profile::save();

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemTemperatureProtectionHighDelayTimeQ(scpi_t *context) {
    int32_t sensor;
    if (!param_temp_sensor(context, sensor)) {
		return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, temperature::sensors[sensor].prot_conf.delay);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemTemperatureProtectionHighTrippedQ(scpi_t *context) {
    int32_t sensor;
    if (!param_temp_sensor(context, sensor)) {
		return SCPI_RES_ERR;
    }

    SCPI_ResultBool(context, temperature::sensors[sensor].isTripped());

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemChannelCountQ(scpi_t *context) {
    SCPI_ResultInt(context, CH_NUM);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemChannelInformationCurrentQ(scpi_t *context) {
    Channel *channel = param_channel(context, false, true);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, channel_dispatcher::getIMax(*channel));

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemChannelInformationPowerQ(scpi_t *context) {
    Channel *channel = param_channel(context, false, true);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, channel->PTOT);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemChannelInformationProgramQ(scpi_t *context) {
    Channel *channel = param_channel(context, false, true);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    uint16_t features = channel->getFeatures();
    
    char strFeatures[64] = {0};

    if (features & CH_FEATURE_VOLT) {
        strcat(strFeatures, "Volt");
    }

    if (features & CH_FEATURE_CURRENT) {
        if (strFeatures[0]) {
            strcat(strFeatures, ", ");
        }
        strcat(strFeatures, "Current");
    }

    if (features & CH_FEATURE_POWER) {
        if (strFeatures[0]) {
            strcat(strFeatures, ", ");
        }
        strcat(strFeatures, "Power");
    }

    if (features & CH_FEATURE_OE) {
        if (strFeatures[0]) {
            strcat(strFeatures, ", ");
        }
        strcat(strFeatures, "OE");
    }

    if (features & CH_FEATURE_DPROG) {
        if (strFeatures[0]) {
            strcat(strFeatures, ", ");
        }
        strcat(strFeatures, "DProg");
    }

    if (features & CH_FEATURE_LRIPPLE) {
        if (strFeatures[0]) {
            strcat(strFeatures, ", ");
        }
        strcat(strFeatures, "LRipple");
    }

    if (features & CH_FEATURE_RPROG) {
        if (strFeatures[0]) {
            strcat(strFeatures, ", ");
        }
        strcat(strFeatures, "Rprog");
    }

    if (features & CH_FEATURE_RPOL) {
        if (strFeatures[0]) {
            strcat(strFeatures, ", ");
        }
        strcat(strFeatures, "RPol");
    }

    SCPI_ResultText(context, strFeatures);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemChannelInformationVoltageQ(scpi_t *context) {
    Channel *channel = param_channel(context, false, true);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, channel_dispatcher::getUMax(*channel));

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemChannelInformationOntimeTotalQ(scpi_t *context) {
    Channel *channel = param_channel(context, false, true);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    outputOnTime(context, channel->onTimeCounter.getTotalTime());

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemChannelInformationOntimeLastQ(scpi_t *context) {
    Channel *channel = param_channel(context, false, true);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    outputOnTime(context, channel->onTimeCounter.getLastTime());

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemChannelModelQ(scpi_t *context) {
    Channel *channel = param_channel(context, false, true);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    SCPI_ResultText(context, channel->getBoardRevisionName());

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCpuInformationEthernetTypeQ(scpi_t *context) {
    SCPI_ResultText(context, getCpuEthernetType());
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCpuInformationTypeQ(scpi_t *context) {
    SCPI_ResultText(context, getCpuType());
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCpuInformationOntimeTotalQ(scpi_t *context) {
	outputOnTime(context, g_powerOnTimeCounter.getTotalTime());
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCpuInformationOntimeLastQ(scpi_t *context) {
	outputOnTime(context, g_powerOnTimeCounter.getLastTime());
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCpuModelQ(scpi_t *context) {
    SCPI_ResultText(context, getCpuModel());
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCpuOptionQ(scpi_t *context) {
    char strFeatures[128] = {0};

#if OPTION_BP
    if (strFeatures[0]) {
        strcat(strFeatures, ", ");
    }
    strcat(strFeatures, "BPost");
#endif

#if OPTION_EXT_EEPROM
    if (strFeatures[0]) {
        strcat(strFeatures, ", ");
    }
    strcat(strFeatures, "EEPROM");
#endif

#if OPTION_EXT_RTC
    if (strFeatures[0]) {
        strcat(strFeatures, ", ");
    }
    strcat(strFeatures, "RTC");
#endif

#if OPTION_SD_CARD
    if (strFeatures[0]) {
        strcat(strFeatures, ", ");
    }
    strcat(strFeatures, "SDcard");
#endif

#if OPTION_ETHERNET
    if (strFeatures[0]) {
        strcat(strFeatures, ", ");
    }
    strcat(strFeatures, "Ethernet");
#endif

#if OPTION_DISPLAY
    if (strFeatures[0]) {
        strcat(strFeatures, ", ");
    }
    strcat(strFeatures, "Display");
#endif

#if OPTION_WATCHDOG
    if (strFeatures[0]) {
        strcat(strFeatures, ", ");
    }
    strcat(strFeatures, "Watchdog");
#endif

    SCPI_ResultText(context, strFeatures);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemSerial(scpi_t *context) {
    const char *serial;
    size_t serialLength;

    if (!SCPI_ParamCharacters(context, &serial, &serialLength, true)) {
        return SCPI_RES_ERR;
    }

    if (serialLength > 7) {
        SCPI_ErrorPush(context, SCPI_ERROR_CHARACTER_DATA_TOO_LONG);
        return SCPI_RES_ERR;
    }

    if (!persist_conf::changeSerial(serial, serialLength)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}


scpi_result_t scpi_cmd_systemSerialQ(scpi_t *context) {
    SCPI_ResultText(context, persist_conf::devConf.serialNumber);
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemPowerProtectionTrip(scpi_t *context) {
    bool enable;
    if (!SCPI_ParamBool(context, &enable, TRUE)) {
        return SCPI_RES_ERR;
    }

	if (!persist_conf::enableShutdownWhenProtectionTripped(enable)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
	}

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemPowerProtectionTripQ(scpi_t *context) {
    SCPI_ResultBool(context, persist_conf::isShutdownWhenProtectionTrippedEnabled());

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemPonOutputDisable(scpi_t *context) {
    bool enable;
    if (!SCPI_ParamBool(context, &enable, TRUE)) {
        return SCPI_RES_ERR;
    }

	if (!persist_conf::enableForceDisablingAllOutputsOnPowerUp(enable)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
	}

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemPonOutputDisableQ(scpi_t *context) {
    SCPI_ResultBool(context, persist_conf::isForceDisablingAllOutputsOnPowerUpEnabled());

    return SCPI_RES_OK;
}

static bool check_password(scpi_t *context) {
    const char *password;
    size_t len;

    if (!SCPI_ParamCharacters(context, &password, &len, true)) {
        return false;
    }

	size_t nPassword = strlen(persist_conf::devConf2.systemPassword);
    if (nPassword != len || strncmp(persist_conf::devConf2.systemPassword, password, len) != 0) {
        SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SYS_PASSWORD);
        return false;
    }

    return true;
}

scpi_result_t scpi_cmd_systemPasswordNew(scpi_t *context) {
    if (!check_password(context)) {
        return SCPI_RES_ERR;
    }

    const char *new_password;
    size_t new_password_len;

    if (!SCPI_ParamCharacters(context, &new_password, &new_password_len, true)) {
        return SCPI_RES_ERR;
    }

	int16_t err;
	if (!persist_conf::isSystemPasswordValid(new_password, new_password_len, err)) {
        SCPI_ErrorPush(context, err);
        return SCPI_RES_ERR;
	}

    if (!persist_conf::changeSystemPassword(new_password, new_password_len)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemPasswordFpanelReset(scpi_t *context) {
    if (!persist_conf::changeSystemPassword("", 0)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemPasswordCalibrateReset(scpi_t *context) {
    if (!persist_conf::changeCalibrationPassword(CALIBRATION_PASSWORD_DEFAULT, strlen(CALIBRATION_PASSWORD_DEFAULT))) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemKlock(scpi_t *context) {
    if (!persist_conf::lockFrontPanel(true)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

#if OPTION_DISPLAY
    gui::refreshPage();
#endif

    return SCPI_RES_OK;
}

scpi_choice_def_t rlStateChoice[] = {
    { "LOCal", RL_STATE_LOCAL },
    { "REMote", RL_STATE_REMOTE },
    { "RWLock", RL_STATE_RW_LOCK },
    SCPI_CHOICE_LIST_END /* termination of option list */
};


scpi_result_t scpi_cmd_systemCommunicateRlstate(scpi_t *context) {
    int32_t rlState;
    if (!SCPI_ParamChoice(context, rlStateChoice, &rlState, true)) {
        return SCPI_RES_ERR;
    }

    g_rlState = (RLState)rlState;

#if OPTION_DISPLAY
    gui::refreshPage();
#endif

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemLocal(scpi_t *context) {
    g_rlState = RL_STATE_LOCAL;

#if OPTION_DISPLAY
    gui::refreshPage();
#endif

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemRemote(scpi_t *context) {
    g_rlState = RL_STATE_REMOTE;

#if OPTION_DISPLAY
    gui::refreshPage();
#endif

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemRwlock(scpi_t *context) {
    g_rlState = RL_STATE_RW_LOCK;

#if OPTION_DISPLAY
    gui::refreshPage();
#endif

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCommunicateSerialBaud(scpi_t *context) {
    int32_t baud;
    if (!SCPI_ParamInt(context, &baud, TRUE)) {
        return SCPI_RES_ERR;
    }

    int baudIndex = persist_conf::getIndexFromBaud(baud);
    if (baudIndex == 0) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }

    if (!persist_conf::setSerialBaudIndex(baudIndex)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCommunicateSerialBaudQ(scpi_t *context) {
    SCPI_ResultInt(context, persist_conf::getBaudFromIndex(persist_conf::getSerialBaudIndex()));
    return SCPI_RES_OK;
}

// NONE|ODD|EVEN
static scpi_choice_def_t parityChoice[] = {
    { "NONE",  serial::PARITY_NONE },
    { "EVEN",  serial::PARITY_EVEN },
    { "ODD",   serial::PARITY_ODD },
    { "MARK",  serial::PARITY_MARK },
    { "SPACE", serial::PARITY_SPACE },
    SCPI_CHOICE_LIST_END
};

scpi_result_t scpi_cmd_systemCommunicateSerialParity(scpi_t *context) {
    int32_t parity;
    if (!SCPI_ParamChoice(context, parityChoice, &parity, true)) {
        return SCPI_RES_ERR;
    }

    if (!persist_conf::setSerialParity(parity)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCommunicateSerialParityQ(scpi_t *context) {
    resultChoiceName(context, parityChoice, persist_conf::getSerialParity());
    return SCPI_RES_OK;
}

// NONE|ODD|EVEN
static scpi_choice_def_t commInterfaceChoice[] = {
    { "SERial", 1 },
    { "ETHernet", 2 },
    { "NTP", 3 },
    { "SOCKets", 4 },
    SCPI_CHOICE_LIST_END
};


scpi_result_t scpi_cmd_systemCommunicateEnable(scpi_t *context) {
    bool enable;
    if (!SCPI_ParamBool(context, &enable, TRUE)) {
        return SCPI_RES_ERR;
    }

    int32_t commInterface;
    if (!SCPI_ParamChoice(context, commInterfaceChoice, &commInterface, true)) {
        return SCPI_RES_ERR;
    }

    if (commInterface == 1) {
        persist_conf::enableSerial(enable);
    } else if (commInterface == 2) {
        persist_conf::enableEthernet(enable);
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCommunicateEnableQ(scpi_t *context) {
    int32_t commInterface;
    if (!SCPI_ParamChoice(context, commInterfaceChoice, &commInterface, true)) {
        return SCPI_RES_ERR;
    }

    if (commInterface == 1) {
        SCPI_ResultBool(context, persist_conf::isSerialEnabled());
    } else if (commInterface == 2) {
        SCPI_ResultBool(context, persist_conf::isEthernetEnabled());
    } else {
        SCPI_ResultBool(context, false);
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCommunicateEthernetDhcp(scpi_t *context) {
    bool enable;
    if (!SCPI_ParamBool(context, &enable, TRUE)) {
        return SCPI_RES_ERR;
    }

    persist_conf::enableEthernetDhcp(enable);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCommunicateEthernetDhcpQ(scpi_t *context) {
    SCPI_ResultBool(context, persist_conf::isEthernetDhcpEnabled());
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCommunicateEthernetAddress(scpi_t *context) {
    const char *ipAddressStr;
    size_t ipAddressStrLength;

    if (!SCPI_ParamCharacters(context, &ipAddressStr, &ipAddressStrLength, true)) {
        return SCPI_RES_ERR;
    }

    uint32_t ipAddress;
    if (!util::parseIpAddress(ipAddressStr, ipAddressStrLength, ipAddress)) {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }

    persist_conf::setEthernetIpAddress(ipAddress);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCommunicateEthernetAddressQ(scpi_t *context) {
    char ipAddressStr[16];
    if (persist_conf::devConf2.flags.ethernetDhcpEnabled) {
        util::ipAddressToString(ethernet::getIpAddress(), ipAddressStr);
    } else {
        util::ipAddressToString(persist_conf::devConf2.ethernetIpAddress, ipAddressStr);
    }
    SCPI_ResultText(context, ipAddressStr);
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCommunicateEthernetDns(scpi_t *context) {
    const char *ipAddressStr;
    size_t ipAddressStrLength;

    if (!SCPI_ParamCharacters(context, &ipAddressStr, &ipAddressStrLength, true)) {
        return SCPI_RES_ERR;
    }

    uint32_t ipAddress;
    if (!util::parseIpAddress(ipAddressStr, ipAddressStrLength, ipAddress)) {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }

    persist_conf::setEthernetDns(ipAddress);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCommunicateEthernetDnsQ(scpi_t *context) {
    char ipAddressStr[16];
    util::ipAddressToString(persist_conf::devConf2.ethernetDns, ipAddressStr);
    SCPI_ResultText(context, ipAddressStr);
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCommunicateEthernetGateway(scpi_t *context) {
    const char *ipAddressStr;
    size_t ipAddressStrLength;

    if (!SCPI_ParamCharacters(context, &ipAddressStr, &ipAddressStrLength, true)) {
        return SCPI_RES_ERR;
    }

    uint32_t ipAddress;
    if (!util::parseIpAddress(ipAddressStr, ipAddressStrLength, ipAddress)) {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }

    persist_conf::setEthernetGateway(ipAddress);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCommunicateEthernetGatewayQ(scpi_t *context) {
    char ipAddressStr[16];
    util::ipAddressToString(persist_conf::devConf2.ethernetGateway, ipAddressStr);
    SCPI_ResultText(context, ipAddressStr);
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCommunicateEthernetSmask(scpi_t *context) {
    const char *ipAddressStr;
    size_t ipAddressStrLength;

    if (!SCPI_ParamCharacters(context, &ipAddressStr, &ipAddressStrLength, true)) {
        return SCPI_RES_ERR;
    }

    uint32_t ipAddress;
    if (!util::parseIpAddress(ipAddressStr, ipAddressStrLength, ipAddress)) {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }

    persist_conf::setEthernetSubnetMask(ipAddress);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCommunicateEthernetSmaskQ(scpi_t *context) {
    char ipAddressStr[16];
    util::ipAddressToString(persist_conf::devConf2.ethernetSubnetMask, ipAddressStr);
    SCPI_ResultText(context, ipAddressStr);
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCommunicateEthernetPort(scpi_t *context) {
    int32_t port;
    if (!SCPI_ParamInt(context, &port, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (port < 0 && port > 65535) {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }

    persist_conf::setEthernetScpiPort((uint16_t)port);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCommunicateEthernetPortQ(scpi_t *context) {
    SCPI_ResultInt(context, persist_conf::devConf2.ethernetScpiPort);
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_systemCommunicateEthernetMacQ(scpi_t *context) {
#if OPTION_ETHERNET
    char macAddressStr[18];
    util::macAddressToString(ethernet::g_mac, macAddressStr);
    SCPI_ResultText(context, macAddressStr);
    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
}

}
}
} // namespace eez::psu::scpi