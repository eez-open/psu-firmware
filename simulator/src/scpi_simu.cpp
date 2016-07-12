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
#include "scpi_simu.h"

#include "simulator_psu.h"
#include "chips.h"
#if OPTION_DISPLAY
#include "front_panel/control.h"
#endif

namespace eez {
namespace psu {

using namespace simulator;

namespace scpi {

////////////////////////////////////////////////////////////////////////////////

bool get_resistance_from_param(scpi_t *context, const scpi_number_t &param, float &value) {
    if (param.special) {
        if (param.tag == SCPI_NUM_MAX) {
            value = SIM_LOAD_MAX;
        }
        else if (param.tag == SCPI_NUM_MIN) {
            value = SIM_LOAD_MIN;
        }
        else if (param.tag == SCPI_NUM_DEF) {
            value = SIM_LOAD_DEF;
        }
        else if (param.tag == SCPI_NUM_INF) {
            value = INFINITY;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return false;
        }
    }
    else {
        if (param.unit != SCPI_UNIT_NONE && param.unit != SCPI_UNIT_OHM) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return false;
        }

        value = (float)param.value;
        if (value < SIM_LOAD_MIN || value > SIM_LOAD_MAX) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return false;
        }
    }
    return true;
}

static bool get_resistance_param(scpi_t *context, float &value) {
    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return false;
    }

    return get_resistance_from_param(context, param, value);
}

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_simu_LoadState(scpi_t *context) {
    bool state;
    if (!SCPI_ParamBool(context, &state, TRUE)) {
        return SCPI_RES_ERR;
    }

    Channel *channel = param_channel(context, FALSE, TRUE);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    channel->simulator.setLoadEnabled(state);

    return SCPI_RES_OK;
}

scpi_result_t scpi_simu_LoadStateQ(scpi_t * context) {
    Channel *channel = param_channel(context, FALSE, TRUE);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    SCPI_ResultBool(context, channel->simulator.getLoadEnabled());

    return SCPI_RES_OK;
}

scpi_result_t scpi_simu_Load(scpi_t *context) {
	float value;
	if (!get_resistance_param(context, value)) {
		return SCPI_RES_ERR;
	}
	
    Channel *channel = param_channel(context, FALSE, TRUE);
	if (!channel) {
        return SCPI_RES_ERR;
    }

    channel->simulator.setLoad(value);

    return SCPI_RES_OK;
}

scpi_result_t scpi_simu_LoadQ(scpi_t *context) {
    Channel *channel = param_channel(context, FALSE, TRUE);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    float value;

    int32_t spec;
    if (!SCPI_ParamChoice(context, scpi_special_numbers_def, &spec, false)) {
        if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }

        value = channel->simulator.getLoad();
    }
    else {
        if (spec == SCPI_NUM_MIN) {
            value = SIM_LOAD_MIN;
        }
        else if (spec == SCPI_NUM_MAX) {
            value = SIM_LOAD_MAX;
        }
        else if (spec == SCPI_NUM_DEF) {
            value = SIM_LOAD_DEF;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return SCPI_RES_ERR;
        }
    }

    return result_float(context, value);
}

scpi_result_t scpi_simu_VoltageProgramExternal(scpi_t *context) {
	float value;
	if (!get_voltage_param(context, value, 0, 0)) {
		return SCPI_RES_ERR;
	}
	
    Channel *channel = param_channel(context, FALSE, TRUE);
	if (!channel) {
        return SCPI_RES_ERR;
    }

    if (!(channel->getFeatures() & CH_FEATURE_RPROG)) {
        SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
        return SCPI_RES_ERR;
    }

    channel->simulator.setVoltProgExt(value);

    return SCPI_RES_OK;
}

scpi_result_t scpi_simu_VoltageProgramExternalQ(scpi_t *context) {
    Channel *channel = param_channel(context, FALSE, TRUE);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    if (!(channel->getFeatures() & CH_FEATURE_RPROG)) {
        SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
        return SCPI_RES_ERR;
    }

    return result_float(context, channel->simulator.getVoltProgExt());
}

scpi_result_t scpi_simu_Pwrgood(scpi_t *context) {
    bool on;
    if (!SCPI_ParamBool(context, &on, TRUE)) {
        return SCPI_RES_ERR;
    }

    Channel *channel = param_channel(context, FALSE, TRUE);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    chips::IOExpanderChip::setPwrgood(channel->ioexp_pin, on);

    return SCPI_RES_OK;
}

scpi_result_t scpi_simu_PwrgoodQ(scpi_t *context) {
    Channel *channel = param_channel(context, FALSE, TRUE);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    SCPI_ResultBool(context, chips::IOExpanderChip::getPwrgood(channel->ioexp_pin));

    return SCPI_RES_OK;
}

scpi_result_t scpi_simu_Temperature(scpi_t *context) {
    float value;
    if (!get_temperature_param(context, value, 0.0f, 100.0f, 25.0f)) {
        return SCPI_RES_ERR;
    }

    int32_t sensor;
    if (!param_temp_sensor(context, sensor)) {
		return SCPI_RES_ERR;
    }

    simulator::setTemperature(sensor, value);

    return SCPI_RES_OK;
}

scpi_result_t scpi_simu_TemperatureQ(scpi_t *context) {
    int32_t sensor;
    if (!param_temp_sensor(context, sensor)) {
		return SCPI_RES_ERR;
    }

    float value;

    int32_t spec;
    if (!SCPI_ParamChoice(context, scpi_special_numbers_def, &spec, false)) {
        if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }

        value = simulator::getTemperature(sensor);
    }
    else {
        if (spec == SCPI_NUM_MIN) {
            value = SIM_TEMP_MIN;
        }
        else if (spec == SCPI_NUM_MAX) {
            value = SIM_TEMP_MAX;
        }
        else if (spec == SCPI_NUM_DEF) {
            value = SIM_TEMP_DEF;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return SCPI_RES_ERR;
        }
    }

    return result_float(context, value);
}

scpi_result_t scpi_simu_GUI(scpi_t *context) {
#if OPTION_DISPLAY
    if (!simulator::front_panel::open()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
#else
    SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
    return SCPI_RES_ERR;
#endif
}

scpi_result_t scpi_simu_Exit(scpi_t *context) {
    simulator::exit();

    return SCPI_RES_OK;
}

}
}
} // namespace eez::psu::scpi