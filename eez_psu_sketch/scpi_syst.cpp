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
#include "scpi_syst.h"

#include "datetime.h"
#include "sound.h"
#include "profile.h"

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_syst_CapabilityQ(scpi_t * context) {
    char text[sizeof(STR_SYST_CAP)];
    strcpy_P(text, PSTR(STR_SYST_CAP));
    SCPI_ResultText(context, text);
    
    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_ErrorNextQ(scpi_t * context) {
    return SCPI_SystemErrorNextQ(context);
}

scpi_result_t scpi_syst_ErrorCountQ(scpi_t * context) {
    return SCPI_SystemErrorCountQ(context);
}

scpi_result_t scpi_syst_VersionQ(scpi_t * context) {
    return SCPI_SystemVersionQ(context);
}

scpi_result_t scpi_syst_Power(scpi_t * context) {
    bool up;
    if (!SCPI_ParamBool(context, &up, TRUE)) {
        return SCPI_RES_ERR;
    }

#if OPTION_MAIN_TEMP_SENSOR
    if (temperature::isSensorTripped(temp_sensor::MAIN)) {
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

scpi_result_t scpi_syst_PowerQ(scpi_t * context) {
    SCPI_ResultBool(context, psu::isPowerUp());
    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_Date(scpi_t * context) {
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

scpi_result_t scpi_syst_DateQ(scpi_t * context) {
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

scpi_result_t scpi_syst_Time(scpi_t * context) {
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

scpi_result_t scpi_syst_TimeQ(scpi_t * context) {
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

scpi_result_t scpi_syst_Beeper(scpi_t * context) {
    sound::playBeep(true);
    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_BeeperState(scpi_t * context) {
    bool enable;
    if (!SCPI_ParamBool(context, &enable, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (enable != persist_conf::isBeepEnabled()) {
        persist_conf::enableBeep(enable);
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_BeeperStateQ(scpi_t * context) {
    SCPI_ResultBool(context, persist_conf::isBeepEnabled());
    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_TempProtectionClear(scpi_t * context) {
    int32_t sensor;
    if (!SCPI_ParamChoice(context, temp_sensor_choice, &sensor, false)) {
#if OPTION_MAIN_TEMP_SENSOR
		if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
        sensor = temp_sensor::MAIN;
#else
		return SCPI_RES_ERR;
#endif
	}

    temperature::clearProtection((temp_sensor::Type)sensor);

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_TempProtectionLevel(scpi_t * context) {
    float level;
    if (!get_temperature_param(context, level, OTP_MAIN_MIN_LEVEL, OTP_MAIN_MAX_LEVEL, OTP_MAIN_DEFAULT_LEVEL)) {
        return SCPI_RES_ERR;
    }

    int32_t sensor;
    if (!SCPI_ParamChoice(context, temp_sensor_choice, &sensor, false)) {
#if OPTION_MAIN_TEMP_SENSOR
		if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
        sensor = temp_sensor::MAIN;
#else
		return SCPI_RES_ERR;
#endif
    }

    temperature::prot_conf[sensor].level = level;
    profile::save();

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_TempProtectionLevelQ(scpi_t * context) {
    int32_t sensor;
    if (!SCPI_ParamChoice(context, temp_sensor_choice, &sensor, false)) {
#if OPTION_MAIN_TEMP_SENSOR
		if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
        sensor = temp_sensor::MAIN;
#else
		return SCPI_RES_ERR;
#endif
    }

    return result_float(context, temperature::prot_conf[sensor].level);
}

scpi_result_t scpi_syst_TempProtectionState(scpi_t * context) {
    bool state;
    if (!SCPI_ParamBool(context, &state, TRUE)) {
        return SCPI_RES_ERR;
    }

    int32_t sensor;
    if (!SCPI_ParamChoice(context, temp_sensor_choice, &sensor, false)) {
#if OPTION_MAIN_TEMP_SENSOR
		if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
        sensor = temp_sensor::MAIN;
#else
		return SCPI_RES_ERR;
#endif
    }

    temperature::prot_conf[sensor].state = state;
    profile::save();

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_TempProtectionStateQ(scpi_t * context) {
    int32_t sensor;
    if (!SCPI_ParamChoice(context, temp_sensor_choice, &sensor, false)) {
#if OPTION_MAIN_TEMP_SENSOR
		if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
        sensor = temp_sensor::MAIN;
#else
		return SCPI_RES_ERR;
#endif
    }

    SCPI_ResultBool(context, temperature::prot_conf[sensor].state);

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_TempProtectionDelay(scpi_t * context) {
    float delay;
    if (!get_duration_param(context, delay, OTP_MAIN_MIN_DELAY, OTP_MAIN_MAX_DELAY, OTP_MAIN_DEFAULT_DELAY)) {
        return SCPI_RES_ERR;
    }

    int32_t sensor;
    if (!SCPI_ParamChoice(context, temp_sensor_choice, &sensor, false)) {
#if OPTION_MAIN_TEMP_SENSOR
		if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
        sensor = temp_sensor::MAIN;
#else
		return SCPI_RES_ERR;
#endif
    }

    temperature::prot_conf[sensor].delay = delay;
    profile::save();

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_TempProtectionDelayQ(scpi_t * context) {
    int32_t sensor;
    if (!SCPI_ParamChoice(context, temp_sensor_choice, &sensor, false)) {
#if OPTION_MAIN_TEMP_SENSOR
		if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
        sensor = temp_sensor::MAIN;
#else
		return SCPI_RES_ERR;
#endif
    }

    SCPI_ResultFloat(context, temperature::prot_conf[sensor].delay);

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_TempProtectionTrippedQ(scpi_t * context) {
    int32_t sensor;
    if (!SCPI_ParamChoice(context, temp_sensor_choice, &sensor, false)) {
#if OPTION_MAIN_TEMP_SENSOR
		if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
        sensor = temp_sensor::MAIN;
#else
		return SCPI_RES_ERR;
#endif
    }

    SCPI_ResultBool(context, temperature::isSensorTripped((temp_sensor::Type)sensor));

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_ChannelCountQ(scpi_t * context) {
    SCPI_ResultInt(context, CH_NUM);

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_ChannelInformationCurrentQ(scpi_t * context) {
    Channel *channel = param_channel(context, false, true);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, channel->I_MAX);

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_ChannelInformationPowerQ(scpi_t * context) {
    Channel *channel = param_channel(context, false, true);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, channel->PTOT);

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_ChannelInformationProgramQ(scpi_t * context) {
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

    SCPI_ResultText(context, strFeatures);

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_ChannelInformationVoltageQ(scpi_t * context) {
    Channel *channel = param_channel(context, false, true);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    SCPI_ResultFloat(context, channel->U_MAX);

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_ChannelModelQ(scpi_t * context) {
    Channel *channel = param_channel(context, false, true);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    SCPI_ResultText(context, channel->getBoardRevisionName());

    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_CpuInformationEhternetTypeQ(scpi_t * context) {
    SCPI_ResultText(context, getCpuEthernetType());
    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_CpuInformationTypeQ(scpi_t * context) {
    SCPI_ResultText(context, getCpuType());
    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_CpuModelQ(scpi_t * context) {
    SCPI_ResultText(context, getCpuModel());
    return SCPI_RES_OK;
}

scpi_result_t scpi_syst_CpuOptionQ(scpi_t * context) {
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

    SCPI_ResultText(context, strFeatures);

    return SCPI_RES_OK;
}

}
}
} // namespace eez::psu::scpi