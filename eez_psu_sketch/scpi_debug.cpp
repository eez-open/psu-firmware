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
#include "watchdog.h"
#include "temperature.h"
#include "fan.h"
#include "serial_psu.h"
#include "fan.h"

namespace eez {
namespace psu {

#if CONF_DEBUG
    using namespace debug;
#endif // CONF_DEBUG

namespace scpi {

scpi_result_t scpi_cmd_debug(scpi_t *context) {
#if CONF_DEBUG
    scpi_number_t param;
    if (SCPI_ParamNumber(context, 0, &param, false)) {
        delay((uint32_t) round(param.value * 1000));
    } else {
        delay(1000);
    }

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif // CONF_DEBUG
}

scpi_result_t scpi_cmd_debugQ(scpi_t *context) {
#if CONF_DEBUG
    char buffer[4096];

    Channel::get(0).adcReadAll();
    Channel::get(1).adcReadAll();

    debug::dumpVariables(buffer);

    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif // CONF_DEBUG
}

scpi_result_t scpi_cmd_debugWdog(scpi_t * context) {
#if CONF_DEBUG
    if (!OPTION_WATCHDOG) {
        SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
        return SCPI_RES_ERR;
    }

    bool enable;
    if (!SCPI_ParamBool(context, &enable, TRUE)) {
        return SCPI_RES_ERR;
    }

	debug::g_debugWatchdog = enable;
    
    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif // CONF_DEBUG
}

scpi_result_t scpi_cmd_debugWdogQ(scpi_t * context) {
#if CONF_DEBUG
    if (!OPTION_WATCHDOG) {
        SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
        return SCPI_RES_ERR;
    }

    SCPI_ResultBool(context, debug::g_debugWatchdog);
    
    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif // CONF_DEBUG
}

scpi_result_t scpi_cmd_debugOntimeQ(scpi_t *context) {
#if CONF_DEBUG
    char buffer[512] = { 0 };
    char *p = buffer;

    sprintf_P(p, PSTR("power active: %d\n"), int(g_powerOnTimeCounter.isActive() ? 1 : 0));
    p += strlen(p);

	for (int i = 0; i < CH_NUM; ++i) {
	    Channel& channel = Channel::get(i);

		sprintf_P(p, PSTR("CH%d active: %d\n"), channel.index, int(channel.onTimeCounter.isActive() ? 1 : 0));
		p += strlen(p);
	}

    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif // CONF_DEBUG
}

scpi_result_t scpi_cmd_debugVoltage(scpi_t *context) {
#if CONF_DEBUG
    Channel *channel = param_channel(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    uint32_t value;
    if (!SCPI_ParamUInt32(context, &value, true)) {
        return SCPI_RES_ERR;
    }

    channel->dac.set_voltage((uint16_t)value);

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif // CONF_DEBUG
}

scpi_result_t scpi_cmd_debugCurrent(scpi_t *context) {
#if CONF_DEBUG
    Channel *channel = param_channel(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    uint32_t value;
    if (!SCPI_ParamUInt32(context, &value, true)) {
        return SCPI_RES_ERR;
    }

    channel->dac.set_current((uint16_t)value);

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif // CONF_DEBUG
}

scpi_result_t scpi_cmd_debugMeasureVoltage(scpi_t *context) {
#if CONF_DEBUG
    if (serial::g_testResult != TEST_OK) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    Channel *channel = param_channel(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    while (true) {
        uint32_t tickCount = micros();
#if OPTION_WATCHDOG && (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12)
        watchdog::tick(tickCount);
#endif
	    temperature::tick(tickCount);
	    fan::tick(tickCount);

        channel->adc.start(AnalogDigitalConverter::ADC_REG0_READ_U_MON);
        delayMicroseconds(2000);
        int16_t adc_data = channel->adc.read();
        channel->eventAdcData(adc_data, false);

        SERIAL_PORT.print((int)debug::g_uMon[channel->index - 1].get());
        SERIAL_PORT.print(" ");
        SERIAL_PORT.print(channel->u.mon_last, 5);
        SERIAL_PORT.println("V");

        int32_t diff = micros() - tickCount;
        if (diff < 48000L) {
            delayMicroseconds(48000L - diff);
        }
    }

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif // CONF_DEBUG
}

scpi_result_t scpi_cmd_debugMeasureCurrent(scpi_t *context) {
#if CONF_DEBUG
    if (serial::g_testResult != TEST_OK) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    Channel *channel = param_channel(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    while (true) {
        uint32_t tickCount = micros();
#if OPTION_WATCHDOG && (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12)
        watchdog::tick(tickCount);
#endif
	    temperature::tick(tickCount);
	    fan::tick(tickCount);

        channel->adc.start(AnalogDigitalConverter::ADC_REG0_READ_I_MON);
        delayMicroseconds(2000);
        int16_t adc_data = channel->adc.read();
        channel->eventAdcData(adc_data, false);

        SERIAL_PORT.print((int)debug::g_iMon[channel->index - 1].get());
        SERIAL_PORT.print(" ");
        SERIAL_PORT.print(channel->i.mon_last, 5);
        SERIAL_PORT.println("A");

        int32_t diff = micros() - tickCount;
        if (diff < 48000L) {
            delayMicroseconds(48000L - diff);
        }
    }

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_HARDWARE_MISSING);
    return SCPI_RES_ERR;
#endif // CONF_DEBUG
}

scpi_result_t scpi_cmd_debugFan(scpi_t * context) {
	int32_t fanSpeed;
	if (!SCPI_ParamInt(context, &fanSpeed, TRUE)) {
		return SCPI_RES_ERR;
	}

	if (fanSpeed < 0) {
		fan::g_fanManualControl = false;
	}
	else {
		fan::g_fanManualControl = true;

		if (fanSpeed > 255) {
			fanSpeed = 255;
		}

		fan::g_fanSpeedPWM = fanSpeed;
		analogWrite(FAN_PWM, fan::g_fanSpeedPWM);
	}

	return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_debugFanQ(scpi_t * context) {
	SCPI_ResultInt(context, fan::g_fanSpeedPWM);

	return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_debugFanPid(scpi_t * context) {
	double Kp;
	if (!SCPI_ParamDouble(context, &Kp, TRUE)) {
		return SCPI_RES_ERR;
	}

	double Ki;
	if (!SCPI_ParamDouble(context, &Ki, TRUE)) {
		return SCPI_RES_ERR;
	}

	double Kd;
	if (!SCPI_ParamDouble(context, &Kd, TRUE)) {
		return SCPI_RES_ERR;
	}

	int POn;
	if (!SCPI_ParamInt(context, &POn, TRUE)) {
		return SCPI_RES_ERR;
	}

	fan::setPidTunings(Kp, Ki, Kd, POn);

	return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_debugFanPidQ(scpi_t * context) {
	double Kp[4] = { fan::g_Kp, fan::g_Ki, fan::g_Kd, fan::g_POn * 1.0f };

	SCPI_ResultArrayDouble(context, Kp, 4, SCPI_FORMAT_ASCII);

	return SCPI_RES_OK;
}


}
}
} // namespace eez::psu::scpi
