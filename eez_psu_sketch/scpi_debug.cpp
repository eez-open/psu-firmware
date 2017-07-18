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

#if OPTION_SD_CARD
#include "sd_card.h"
#endif

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
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif // CONF_DEBUG
}

scpi_result_t scpi_cmd_debugQ(scpi_t *context) {
#if CONF_DEBUG
    char buffer[2048];

    Channel::get(0).adcReadAll();
    Channel::get(1).adcReadAll();

    debug::dumpVariables(buffer);

    SCPI_ResultCharacters(context, buffer, strlen(buffer));

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif // CONF_DEBUG
}

scpi_result_t scpi_cmd_debugWdog(scpi_t * context) {
#if CONF_DEBUG
    if (!OPTION_WATCHDOG) {
        SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
        return SCPI_RES_ERR;
    }

    bool enable;
    if (!SCPI_ParamBool(context, &enable, TRUE)) {
        return SCPI_RES_ERR;
    }

	debug::g_debugWatchdog = enable;
    
    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif // CONF_DEBUG
}

scpi_result_t scpi_cmd_debugWdogQ(scpi_t * context) {
#if CONF_DEBUG
    if (!OPTION_WATCHDOG) {
        SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
        return SCPI_RES_ERR;
    }

    SCPI_ResultBool(context, debug::g_debugWatchdog);
    
    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
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
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif // CONF_DEBUG
}

scpi_result_t scpi_cmd_debugDirQ(scpi_t *context) {
#if CONF_DEBUG && OPTION_SD_CARD
    sd_card::dir();
    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_cmd_debugFileQ(scpi_t *context) {
#if CONF_DEBUG && OPTION_SD_CARD
    const char *param;
    size_t len;

    if (!SCPI_ParamCharacters(context, &param, &len, true)) {
        return SCPI_RES_ERR;
    }

    char path[MAX_PATH_LENGTH];
    strncpy(path, param, len);
    path[len] = 0;

    sd_card::dumpFile(path);

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif
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
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
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
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
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
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
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
    SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
    return SCPI_RES_ERR;
#endif // CONF_DEBUG
}


}
}
} // namespace eez::psu::scpi
