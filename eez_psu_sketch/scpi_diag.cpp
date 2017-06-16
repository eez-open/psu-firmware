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

#include "calibration.h"
#include "devices.h"
#include "temperature.h"
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
#include "fan.h"
#endif

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

static void printCalibrationValue(scpi_t *context, char *buffer, calibration::Value &value) {
    const char *prefix;
    void(*strcat_value)(char *str, float value, int numSignificantDecimalDigits, int channelIndex);
    int numSignificantDecimalDigits = 4;
    if (value.voltOrCurr) {
        prefix = PSTR("u");
        strcat_value = util::strcatVoltage;
    }
    else {
        prefix = PSTR("i");
        strcat_value = util::strcatCurrent;
    }

    if (value.min_set) { strcpy_P(buffer, prefix); strcat_P(buffer, PSTR("_min=")); strcat_value(buffer, value.min_val, numSignificantDecimalDigits, -1); SCPI_ResultText(context, buffer); }
    if (value.mid_set) { strcpy_P(buffer, prefix); strcat_P(buffer, PSTR("_mid=")); strcat_value(buffer, value.mid_val, numSignificantDecimalDigits, -1); SCPI_ResultText(context, buffer); }
    if (value.max_set) { strcpy_P(buffer, prefix); strcat_P(buffer, PSTR("_max=")); strcat_value(buffer, value.max_val, numSignificantDecimalDigits, -1); SCPI_ResultText(context, buffer); }

    strcpy_P(buffer, prefix); strcat_P(buffer, PSTR("_level="));
    switch (value.level) {
    case calibration::LEVEL_NONE: strcat_P(buffer, PSTR("none")); break;
    case calibration::LEVEL_MIN:  strcat_P(buffer, PSTR("min") ); break;
    case calibration::LEVEL_MID:  strcat_P(buffer, PSTR("mid") ); break;
    case calibration::LEVEL_MAX:  strcat_P(buffer, PSTR("max") ); break;
    }
    SCPI_ResultText(context, buffer);

    if (value.level != calibration::LEVEL_NONE) {
        strcpy_P(buffer, prefix); strcat_P(buffer, PSTR("_level_value=")); strcat_value(buffer, value.getLevelValue(), numSignificantDecimalDigits, -1); SCPI_ResultText(context, buffer);
        strcpy_P(buffer, prefix); strcat_P(buffer, PSTR("_adc="        )); strcat_value(buffer, value.getAdcValue()  , numSignificantDecimalDigits, -1); SCPI_ResultText(context, buffer);
    }
}

void printCalibrationParameters(scpi_t *context, ValueType valueType, uint8_t currentRange, bool calParamsExists, Channel::CalibrationValueConfiguration &calibrationValue, char *buffer) {
    const char *prefix;
    void(*strcat_value)(char *str, float value, int precision, int channelIndex);
    int numSignificantDecimalDigits = 4;
    if (valueType == VALUE_TYPE_FLOAT_VOLT) {
        prefix = PSTR("u");
        strcat_value = util::strcatVoltage;
    }
    else {
        if (currentRange == 0) {
            prefix = PSTR("i_5A");
        } else if (currentRange == 1) {
            prefix = PSTR("i_500mA");
        } else {
            prefix = PSTR("i");
        }
        strcat_value = util::strcatCurrent;
    }

    strcpy(buffer, prefix); strcat_P(buffer, PSTR("_cal_params_exists=")); util::strcatInt(buffer, calParamsExists);SCPI_ResultText(context, buffer);
    
    if (calParamsExists) {
        strcpy(buffer, prefix); strcat_P(buffer, PSTR("_min_level=")); strcat_value(buffer, calibrationValue.min.dac, numSignificantDecimalDigits, -1); SCPI_ResultText(context, buffer);
        strcpy(buffer, prefix); strcat_P(buffer, PSTR("_min_data=") ); strcat_value(buffer, calibrationValue.min.val, numSignificantDecimalDigits, -1); SCPI_ResultText(context, buffer);
        strcpy(buffer, prefix); strcat_P(buffer, PSTR("_min_adc=")  ); strcat_value(buffer, calibrationValue.min.adc, numSignificantDecimalDigits, -1); SCPI_ResultText(context, buffer);
        strcpy(buffer, prefix); strcat_P(buffer, PSTR("_mid_level=")); strcat_value(buffer, calibrationValue.mid.dac, numSignificantDecimalDigits, -1); SCPI_ResultText(context, buffer);
        strcpy(buffer, prefix); strcat_P(buffer, PSTR("_mid_data=") ); strcat_value(buffer, calibrationValue.mid.val, numSignificantDecimalDigits, -1); SCPI_ResultText(context, buffer);
        strcpy(buffer, prefix); strcat_P(buffer, PSTR("_mid_adc=")  ); strcat_value(buffer, calibrationValue.mid.adc, numSignificantDecimalDigits, -1); SCPI_ResultText(context, buffer);
        strcpy(buffer, prefix); strcat_P(buffer, PSTR("_max_level=")); strcat_value(buffer, calibrationValue.max.dac, numSignificantDecimalDigits, -1); SCPI_ResultText(context, buffer);
        strcpy(buffer, prefix); strcat_P(buffer, PSTR("_max_data=") ); strcat_value(buffer, calibrationValue.max.val, numSignificantDecimalDigits, -1); SCPI_ResultText(context, buffer);
        strcpy(buffer, prefix); strcat_P(buffer, PSTR("_max_adc=")  ); strcat_value(buffer, calibrationValue.max.adc, numSignificantDecimalDigits, -1); SCPI_ResultText(context, buffer);
	    strcpy(buffer, prefix); strcat_P(buffer, PSTR("_min_range=")  ); strcat_value(buffer, calibrationValue.minPossible, numSignificantDecimalDigits, -1); SCPI_ResultText(context, buffer);
	    strcpy(buffer, prefix); strcat_P(buffer, PSTR("_max_range=")  ); strcat_value(buffer, calibrationValue.maxPossible, numSignificantDecimalDigits, -1); SCPI_ResultText(context, buffer);
    }
}

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cmd_diagnosticInformationAdcQ(scpi_t * context) {
    Channel *channel = param_channel(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    channel->adcReadAll();

    char buffer[64] = { 0 };

    strcpy_P(buffer, PSTR("U_SET="));
    util::strcatVoltage(buffer, channel->u.mon_dac);
    SCPI_ResultText(context, buffer);

    strcpy_P(buffer, PSTR("U_MON="));
    util::strcatVoltage(buffer, channel->u.mon_last);
    SCPI_ResultText(context, buffer);

    strcpy_P(buffer, PSTR("I_SET="));
    util::strcatCurrent(buffer, channel->i.mon_dac, getNumSignificantDecimalDigits(VALUE_TYPE_FLOAT_AMPER), channel->index-1);
    SCPI_ResultText(context, buffer);

    strcpy_P(buffer, PSTR("I_MON="));
    util::strcatCurrent(buffer, channel->i.mon_last, getNumSignificantDecimalDigits(VALUE_TYPE_FLOAT_AMPER), channel->index-1);
    SCPI_ResultText(context, buffer);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_diagnosticInformationCalibrationQ(scpi_t *context) {
    Channel *channel = param_channel(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    char buffer[128] = { 0 };

    if (calibration::isEnabled()) {
        if (calibration::isRemarkSet()) {
            sprintf_P(buffer, PSTR("remark=%s"), calibration::getRemark());
            SCPI_ResultText(context, buffer);
        }
        printCalibrationValue(context, buffer, calibration::getVoltage());
        printCalibrationValue(context, buffer, calibration::getCurrent());
    }
    else {
        sprintf_P(buffer, PSTR("remark=%s %s"), channel->cal_conf.calibration_date, channel->cal_conf.calibration_remark);
        SCPI_ResultText(context, buffer);

        printCalibrationParameters(context, VALUE_TYPE_FLOAT_VOLT, -1, channel->cal_conf.flags.u_cal_params_exists, channel->cal_conf.u, buffer);
        if (channel->hasSupportForCurrentDualRange()) {
            printCalibrationParameters(context, VALUE_TYPE_FLOAT_AMPER, 0, channel->cal_conf.flags.i_cal_params_exists_range_high, channel->cal_conf.i[0], buffer);
            printCalibrationParameters(context, VALUE_TYPE_FLOAT_AMPER, 1, channel->cal_conf.flags.i_cal_params_exists_range_low, channel->cal_conf.i[1], buffer);
        } else {
            printCalibrationParameters(context, VALUE_TYPE_FLOAT_AMPER, -1, channel->cal_conf.flags.i_cal_params_exists_range_high, channel->cal_conf.i[0], buffer);
        }
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_diagnosticInformationProtectionQ(scpi_t * context) {
    char buffer[256] = { 0 };

    for (int i = 0; i < CH_NUM; ++i) {
        Channel *channel = &Channel::get(i);

        // voltage
        sprintf_P(buffer, PSTR("CH%d u_tripped=%d" ), channel->index, (int)channel->ovp.flags.tripped         ); SCPI_ResultText(context, buffer);
        sprintf_P(buffer, PSTR("CH%d u_state=%d"   ), channel->index, (int)channel->prot_conf.flags.u_state   ); SCPI_ResultText(context, buffer);
        sprintf_P(buffer, PSTR("CH%d u_delay="), channel->index);
        util::strcatDuration(buffer, channel->prot_conf.u_delay);
        SCPI_ResultText(context, buffer);

        sprintf_P(buffer, PSTR("CH%d u_level="), channel->index);
        util::strcatVoltage(buffer, channel->prot_conf.u_level);
        SCPI_ResultText(context, buffer);

        // current
        sprintf_P(buffer, PSTR("CH%d i_tripped=%d" ), channel->index, (int)channel->ocp.flags.tripped         ); SCPI_ResultText(context, buffer);
        sprintf_P(buffer, PSTR("CH%d i_state=%d"   ), channel->index, (int)channel->prot_conf.flags.i_state   ); SCPI_ResultText(context, buffer);
        sprintf_P(buffer, PSTR("CH%d i_delay="), channel->index);
        util::strcatDuration(buffer, channel->prot_conf.i_delay);
        SCPI_ResultText(context, buffer);

        // power
        sprintf_P(buffer, PSTR("CH%d p_tripped=%d"), channel->index, (int)channel->opp.flags.tripped         ); SCPI_ResultText(context, buffer);
        sprintf_P(buffer, PSTR("CH%d p_state=%d"  ), channel->index, (int)channel->prot_conf.flags.p_state   ); SCPI_ResultText(context, buffer);
        sprintf_P(buffer, PSTR("CH%d p_delay="), channel->index);
        util::strcatDuration(buffer, channel->prot_conf.p_delay);
        SCPI_ResultText(context, buffer);

        sprintf_P(buffer, PSTR("CH%d p_level="), channel->index);
        util::strcatPower(buffer, channel->prot_conf.p_level);
        SCPI_ResultText(context, buffer);
    }

	for (int i = 0; i < temp_sensor::NUM_TEMP_SENSORS; ++i) {
		temp_sensor::TempSensor &sensor = temp_sensor::sensors[i];
		temperature::TempSensorTemperature &sensorTemperature = temperature::sensors[i];

		sprintf_P(buffer, PSTR("temp_%s_tripped=%d"), sensor.name, (int)sensorTemperature.isTripped());
		SCPI_ResultText(context, buffer);

		sprintf_P(buffer, PSTR("temp_%s_state=%d"  ), sensor.name, (int)sensorTemperature.prot_conf.state);
		SCPI_ResultText(context, buffer);

		sprintf_P(buffer, PSTR("temp_%s_delay="), sensor.name);
		util::strcatDuration(buffer, sensorTemperature.prot_conf.delay);
		SCPI_ResultText(context, buffer);

		sprintf_P(buffer, PSTR("temp_%s_level="), sensor.name);
		util::strcatFloat(buffer, sensorTemperature.prot_conf.level, getNumSignificantDecimalDigits(VALUE_TYPE_FLOAT_CELSIUS));
		strcat_P(buffer, PSTR(" oC"));
		SCPI_ResultText(context, buffer);
	}

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_diagnosticInformationTestQ(scpi_t * context) {
    char buffer[128] = { 0 };

	for (int i = 0; i < devices::numDevices; ++i) {
		devices::Device &device = devices::devices[i];

		sprintf_P(buffer, PSTR("%d, %s, %s, %s"),
			device.testResult ? (int)*device.testResult : TEST_SKIPPED, device.deviceName, devices::getInstalledString(device.installed), devices::getTestResultString(*device.testResult));
		SCPI_ResultText(context, buffer);
	}

//    sprintf_P(buffer, PSTR("%d, EEPROM, %s, %s"),
//        eeprom::g_testResult, get_installed_str(OPTION_EXT_EEPROM), get_test_result_str(eeprom::g_testResult));
//    SCPI_ResultText(context, buffer);
//    
//    sprintf_P(buffer, PSTR("%d, Ethernet, %s, %s"),
//        ethernet::g_testResult, get_installed_str(OPTION_ETHERNET), get_test_result_str(ethernet::g_testResult));
//    SCPI_ResultText(context, buffer);
//
//    sprintf_P(buffer, PSTR("%d, RTC, %s, %s"),
//        rtc::g_testResult, get_installed_str(OPTION_EXT_RTC), get_test_result_str(rtc::g_testResult));
//    SCPI_ResultText(context, buffer);
//    
//    sprintf_P(buffer, PSTR("%d, DateTime, %s, %s"),
//        datetime::g_testResult, get_installed_str(true), get_test_result_str(datetime::g_testResult));
//    SCPI_ResultText(context, buffer);
//
//    sprintf_P(buffer, PSTR("%d, BP option, %s, %s"),
//        psu::TEST_SKIPPED, get_installed_str(OPTION_BP), get_test_result_str(psu::TEST_SKIPPED));
//    SCPI_ResultText(context, buffer);
//
//	for (int i = 0; i < temp_sensor::NUM_TEMP_SENSORS; ++i) {
//		temp_sensor::TempSensor &sensor = temp_sensor::sensors[i];
//		sprintf_P(buffer, PSTR("%d, %s temp, %s, %s"),
//			sensor.g_testResult, sensor.name, get_installed_str(sensor.installed ? true : false), get_test_result_str(sensor.g_testResult));
//		SCPI_ResultText(context, buffer);
//	}
//
//#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
//    sprintf_P(buffer, PSTR("%d, Fan, %s, %s"),
//        fan::g_testResult, get_installed_str(OPTION_FAN), get_test_result_str(fan::g_testResult));
//    SCPI_ResultText(context, buffer);
//#endif
//
//	if (psu::isPowerUp()) {
//        for (int i = 0; i < CH_NUM; ++i) {
//            Channel *channel = &Channel::get(i);
//
//            sprintf_P(buffer, PSTR("%d, CH%d IOEXP, installed, %s"),
//                channel->ioexp.g_testResult, channel->index, get_test_result_str((psu::TestResult)channel->ioexp.g_testResult));
//            SCPI_ResultText(context, buffer);
//
//            sprintf_P(buffer, PSTR("%d, CH%d DAC, installed, %s"),
//                channel->dac.g_testResult, channel->index, get_test_result_str((psu::TestResult)channel->dac.g_testResult));
//            SCPI_ResultText(context, buffer);
//
//            sprintf_P(buffer, PSTR("%d, CH%d ADC, installed, %s"),
//                channel->adc.g_testResult, channel->index, get_test_result_str((psu::TestResult)channel->adc.g_testResult));
//            SCPI_ResultText(context, buffer);
//        }
//    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_diagnosticInformationFanQ(scpi_t * context) {
#if (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12) && OPTION_FAN && FAN_OPTION_RPM_MEASUREMENT
	SCPI_ResultInt(context, fan::g_rpm);
#else
	SCPI_ResultInt(context, -1);
#endif

    return SCPI_RES_OK;
}

}
}
} // namespace eez::psu::scpi