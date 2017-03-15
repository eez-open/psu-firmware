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

static void print_calibration_value(scpi_t * context, char *buffer, calibration::Value &value) {
    const char *prefix;
    void(*strcat_value)(char *str, float value, int precision);
    if (value.voltOrCurr) {
        prefix = PSTR("u");
        strcat_value = util::strcatVoltage;
    }
    else {
        prefix = PSTR("i");
        strcat_value = util::strcatCurrent;
    }

    if (value.min_set) { strcpy_P(buffer, prefix); strcat_P(buffer, PSTR("_min=")); strcat_value(buffer, value.min_val, FLOAT_TO_STR_NUM_DECIMAL_DIGITS); SCPI_ResultText(context, buffer); }
    if (value.mid_set) { strcpy_P(buffer, prefix); strcat_P(buffer, PSTR("_mid=")); strcat_value(buffer, value.mid_val, FLOAT_TO_STR_NUM_DECIMAL_DIGITS); SCPI_ResultText(context, buffer); }
    if (value.max_set) { strcpy_P(buffer, prefix); strcat_P(buffer, PSTR("_max=")); strcat_value(buffer, value.max_val, FLOAT_TO_STR_NUM_DECIMAL_DIGITS); SCPI_ResultText(context, buffer); }

    strcpy_P(buffer, prefix); strcat_P(buffer, PSTR("_level="));
    switch (value.level) {
    case calibration::LEVEL_NONE: strcat_P(buffer, PSTR("none")); break;
    case calibration::LEVEL_MIN:  strcat_P(buffer, PSTR("min") ); break;
    case calibration::LEVEL_MID:  strcat_P(buffer, PSTR("mid") ); break;
    case calibration::LEVEL_MAX:  strcat_P(buffer, PSTR("max") ); break;
    }
    SCPI_ResultText(context, buffer);

    if (value.level != calibration::LEVEL_NONE) {
        strcpy_P(buffer, prefix); strcat_P(buffer, PSTR("_level_value=")); strcat_value(buffer, value.getLevelValue(), FLOAT_TO_STR_NUM_DECIMAL_DIGITS); SCPI_ResultText(context, buffer);
        strcpy_P(buffer, prefix); strcat_P(buffer, PSTR("_adc="        )); strcat_value(buffer, value.getAdcValue()  , FLOAT_TO_STR_NUM_DECIMAL_DIGITS); SCPI_ResultText(context, buffer);
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
    util::strcatVoltage(buffer, channel->u.mon);
    SCPI_ResultText(context, buffer);

    strcpy_P(buffer, PSTR("I_SET="));
    util::strcatCurrent(buffer, channel->i.mon_dac);
    SCPI_ResultText(context, buffer);

    strcpy_P(buffer, PSTR("I_MON="));
    util::strcatCurrent(buffer, channel->i.mon);
    SCPI_ResultText(context, buffer);

    return SCPI_RES_OK;
}

void printCalibrationParameters(scpi_t *context, const char *label, bool calParamsExists,  Channel::CalibrationValueConfiguration &calibrationValue, char *buffer) {
    strcpy(buffer, label); strcat_P(buffer, PSTR("_cal_params_exists=")); util::strcatInt(buffer, calParamsExists);SCPI_ResultText(context, buffer);
    
    if (calParamsExists) {
        strcpy(buffer, label); strcat_P(buffer, PSTR("_min_level=")); util::strcatVoltage(buffer, calibrationValue.min.dac, 6); SCPI_ResultText(context, buffer);
        strcpy(buffer, label); strcat_P(buffer, PSTR("_min_data=") ); util::strcatVoltage(buffer, calibrationValue.min.val, 6); SCPI_ResultText(context, buffer);
        strcpy(buffer, label); strcat_P(buffer, PSTR("_min_adc=")  ); util::strcatVoltage(buffer, calibrationValue.min.adc, 6); SCPI_ResultText(context, buffer);
        strcpy(buffer, label); strcat_P(buffer, PSTR("_mid_level=")); util::strcatVoltage(buffer, calibrationValue.mid.dac, 6); SCPI_ResultText(context, buffer);
        strcpy(buffer, label); strcat_P(buffer, PSTR("_mid_data=") ); util::strcatVoltage(buffer, calibrationValue.mid.val, 6); SCPI_ResultText(context, buffer);
        strcpy(buffer, label); strcat_P(buffer, PSTR("_mid_adc=")  ); util::strcatVoltage(buffer, calibrationValue.mid.adc, 6); SCPI_ResultText(context, buffer);
        strcpy(buffer, label); strcat_P(buffer, PSTR("_max_level=")); util::strcatVoltage(buffer, calibrationValue.max.dac, 6); SCPI_ResultText(context, buffer);
        strcpy(buffer, label); strcat_P(buffer, PSTR("_max_data=") ); util::strcatVoltage(buffer, calibrationValue.max.val, 6); SCPI_ResultText(context, buffer);
        strcpy(buffer, label); strcat_P(buffer, PSTR("_max_adc=")  ); util::strcatVoltage(buffer, calibrationValue.max.adc, 6); SCPI_ResultText(context, buffer);
	    strcpy(buffer, label); strcat_P(buffer, PSTR("_min_range=")  ); util::strcatVoltage(buffer, calibrationValue.minPossible, 6); SCPI_ResultText(context, buffer);
	    strcpy(buffer, label); strcat_P(buffer, PSTR("_max_range=")  ); util::strcatVoltage(buffer, calibrationValue.maxPossible, 6); SCPI_ResultText(context, buffer);
    }
}

scpi_result_t scpi_cmd_diagnosticInformationCalibrationQ(scpi_t * context) {
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
        print_calibration_value(context, buffer, calibration::getVoltage());
        print_calibration_value(context, buffer, calibration::getCurrent());
    }
    else {
        sprintf_P(buffer, PSTR("remark=%s %s"), channel->cal_conf.calibration_date, channel->cal_conf.calibration_remark);
        SCPI_ResultText(context, buffer);

        printCalibrationParameters(context, "u", channel->cal_conf.flags.u_cal_params_exists, channel->cal_conf.u, buffer);
        if (channel->boardRevision == CH_BOARD_REVISION_R5B12) {
            printCalibrationParameters(context, "i_5A", channel->cal_conf.flags.i_cal_params_exists_range0, channel->cal_conf.i[0], buffer);
            printCalibrationParameters(context, "i_500mA", channel->cal_conf.flags.i_cal_params_exists_range1, channel->cal_conf.i[1], buffer);
        } else {
            printCalibrationParameters(context, "i", channel->cal_conf.flags.i_cal_params_exists_range0, channel->cal_conf.i[0], buffer);
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
        util::strcatFloat(buffer, channel->prot_conf.u_level);
        strcat_P(buffer, PSTR(" V"));
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
        util::strcatFloat(buffer, channel->prot_conf.p_level);
        strcat_P(buffer, PSTR(" W"));
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
		util::strcatFloat(buffer, sensorTemperature.prot_conf.level);
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