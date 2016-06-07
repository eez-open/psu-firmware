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
#include "scpi_sour.h"

#include "profile.h"

#define I_STATE 1
#define P_STATE 2
#define U_STATE 3

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

static scpi_result_t set_step(scpi_t * context, Channel::Value *cv, float min_value, float max_value, float def, _scpi_unit_t unit) {
    scpi_number_t step_param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &step_param, true)) {
        return SCPI_RES_ERR;
    }

    float step;

    if (step_param.special) {
        if (step_param.tag == SCPI_NUM_DEF) {
            step = def;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return SCPI_RES_ERR;
        }
    }
    else {
        if (step_param.unit != SCPI_UNIT_NONE && step_param.unit != unit) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return SCPI_RES_ERR;
        }

        step = (float)step_param.value;
        if (step < min_value || step > max_value) {
            SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
            return SCPI_RES_ERR;
        }
    }

    cv->step = step;
    profile::save();

    return SCPI_RES_OK;

}

static scpi_result_t get_source_value(scpi_t * context, float value, float min, float max, float def) {
    int32_t spec;
    if (!SCPI_ParamChoice(context, scpi_special_numbers_def, &spec, false)) {
        if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
    }
    else {
        if (spec == SCPI_NUM_MIN) {
            value = min;
        }
        else if (spec == SCPI_NUM_MAX) {
            value = max;
        }
        else if (spec == SCPI_NUM_DEF) {
            value = def;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return SCPI_RES_ERR;
        }
    }

    return result_float(context, value);
}

static scpi_result_t get_source_value(scpi_t * context, float value, float def) {
    int32_t spec;
    if (!SCPI_ParamChoice(context, scpi_special_numbers_def, &spec, false)) {
        if (SCPI_ParamErrorOccurred(context)) {
            return SCPI_RES_ERR;
        }
    }
    else {
        if (spec == SCPI_NUM_DEF) {
            value = def;
        }
        else {
            SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
            return SCPI_RES_ERR;
        }
    }

    return result_float(context, value);
}

scpi_result_t set_delay(scpi_t *context, float &delay_var, float min, float max, float def) {
    float delay;
    if (!get_duration_param(context, delay, min, max, def)) {
        return SCPI_RES_ERR;
    }

    delay_var = delay;
    profile::save();

    return SCPI_RES_OK;
}

scpi_result_t get_delay(scpi_t *context, float delay) {
    SCPI_ResultFloat(context, delay);

    return SCPI_RES_OK;
}

scpi_result_t set_state(scpi_t *context, Channel *channel, int type) {
	bool state;
	if (!SCPI_ParamBool(context, &state, TRUE)) {
		return SCPI_RES_ERR;
	}

    switch (type) {
    case I_STATE: channel->prot_conf.flags.i_state = state; break;
    case P_STATE: channel->prot_conf.flags.p_state = state; break;
    default:      channel->prot_conf.flags.u_state = state; break;
	}

    profile::save();

	return SCPI_RES_OK;
}

scpi_result_t get_state(scpi_t *context, Channel *channel, int type) {
	switch (type) {
	case I_STATE: SCPI_ResultBool(context, channel->prot_conf.flags.i_state); break;
	case P_STATE: SCPI_ResultBool(context, channel->prot_conf.flags.p_state); break;
	default:      SCPI_ResultBool(context, channel->prot_conf.flags.u_state); break;
	}

	return SCPI_RES_OK;
}

scpi_result_t get_tripped(scpi_t *context, Channel::ProtectionValue &cpv) {
	SCPI_ResultBool(context, cpv.flags.tripped);

	return SCPI_RES_OK;
}

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_source_Current(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    float current;
    if (!get_current_param(context, current, channel, &channel->i)) {
        return SCPI_RES_ERR;
    }

    if (current * channel->u.set > channel->PTOT) {
        SCPI_ErrorPush(context, SCPI_ERROR_POWER_LIMIT_EXCEEDED);
        return SCPI_RES_ERR;
    }

    channel->setCurrent(current);

    return SCPI_RES_OK;
}

scpi_result_t scpi_source_CurrentQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_source_value(context, channel->i.set, channel->I_MIN, channel->I_MAX, channel->I_DEF);
}

scpi_result_t scpi_source_Voltage(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    float voltage;
    if (!get_voltage_param(context, voltage, channel, &channel->u)) {
        return SCPI_RES_ERR;
    }

    if (voltage * channel->i.set > channel->PTOT) {
        SCPI_ErrorPush(context, SCPI_ERROR_POWER_LIMIT_EXCEEDED);
        return SCPI_RES_ERR;
    }

    channel->setVoltage(voltage);

    return SCPI_RES_OK;
}

scpi_result_t scpi_source_VoltageQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_source_value(context, channel->u.set, channel->U_MIN, channel->U_MAX, channel->U_DEF);
}

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_source_CurrentStep(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return set_step(context, &channel->i, channel->I_MIN_STEP, channel->I_MAX_STEP, channel->I_DEF_STEP, SCPI_UNIT_AMPER);
}

scpi_result_t scpi_source_CurrentStepQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_source_value(context, channel->i.step, channel->I_DEF_STEP);
}

scpi_result_t scpi_source_VoltageStep(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return set_step(context, &channel->u, channel->U_MIN_STEP, channel->U_MAX_STEP, channel->U_DEF_STEP, SCPI_UNIT_VOLT);
}

scpi_result_t scpi_source_VoltageStepQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_source_value(context, channel->u.step, channel->U_DEF_STEP);
}

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_source_CurrentProtectionDelay(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return set_delay(context, channel->prot_conf.i_delay,
        channel->OCP_MIN_DELAY, channel->OCP_MAX_DELAY, channel->OCP_DEFAULT_DELAY);
}

scpi_result_t scpi_source_CurrentProtectionDelayQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_delay(context, channel->prot_conf.i_delay);
}

scpi_result_t scpi_source_CurrentProtectionState(scpi_t *context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }
    
    return set_state(context, channel, I_STATE);
}

scpi_result_t scpi_source_CurrentProtectionStateQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_state(context, channel, I_STATE);
}

scpi_result_t scpi_source_CurrentProtectionTrippedQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }
    
    return get_tripped(context, channel->ocp);
}

scpi_result_t scpi_source_PowerProtectionLevel(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
	}

	float power;
	if (!get_power_param(context, power, channel->OPP_MIN_LEVEL, channel->OPP_MAX_LEVEL, channel->OPP_DEFAULT_LEVEL)) {
		return SCPI_RES_ERR;
	}

    channel->prot_conf.p_level = power;
    profile::save();

	return SCPI_RES_OK;
}

scpi_result_t scpi_source_PowerProtectionLevelQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }
    
    return get_source_value(context, channel->prot_conf.p_level,
        channel->OPP_MIN_LEVEL, channel->OPP_MAX_LEVEL, channel->OPP_DEFAULT_LEVEL);
}

scpi_result_t scpi_source_PowerProtectionDelay(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return set_delay(context, channel->prot_conf.p_delay,
        channel->OCP_MIN_DELAY, channel->OCP_MAX_DELAY, channel->OPP_DEFAULT_DELAY);
}

scpi_result_t scpi_source_PowerProtectionDelayQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_delay(context, channel->prot_conf.p_delay);
}

scpi_result_t scpi_source_PowerProtectionState(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }
    
    return set_state(context, channel, P_STATE);
}

scpi_result_t scpi_source_PowerProtectionStateQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }
    
    return get_state(context, channel, P_STATE);
}

scpi_result_t scpi_source_PowerProtectionTrippedQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_tripped(context, channel->opp);
}

scpi_result_t scpi_source_VoltageProtectionLevel(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
	}

    float voltage;
    if (!get_voltage_protection_level_param(context, voltage, channel->u.set, channel->U_MAX, channel->U_MAX)) {
        return SCPI_RES_ERR;
    }

    channel->prot_conf.u_level = voltage;
    profile::save();

	return SCPI_RES_OK;
}

scpi_result_t scpi_source_VoltageProtectionLevelQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }
    
    return get_source_value(context, channel->prot_conf.u_level,
        channel->u.set, channel->U_MAX, channel->U_MAX);
}

scpi_result_t scpi_source_VoltageProtectionDelay(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return set_delay(context, channel->prot_conf.u_delay,
        channel->OVP_MIN_DELAY, channel->OVP_MAX_DELAY, channel->OVP_DEFAULT_DELAY);
}

scpi_result_t scpi_source_VoltageProtectionDelayQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_delay(context, channel->prot_conf.u_delay);
}

scpi_result_t scpi_source_VoltageProtectionState(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }
    
    return set_state(context, channel, U_STATE);
}

scpi_result_t scpi_source_VoltageProtectionStateQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }
    
    return get_state(context, channel, U_STATE);
}

scpi_result_t scpi_source_VoltageProtectionTrippedQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_tripped(context, channel->ovp);
}

scpi_result_t scpi_source_VoltageSenseSource(scpi_t * context) {
    if (!OPTION_BP) {
        SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
        return SCPI_RES_ERR;
    }

    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    int32_t choice;
    if (!SCPI_ParamChoice(context, internal_external_choice, &choice, TRUE)) {
        return SCPI_RES_ERR;
    }

    channel->remoteSensingEnable(choice == 0 ? false : true);

    return SCPI_RES_OK;
}

scpi_result_t scpi_source_VoltageSenseSourceQ(scpi_t * context) {
    if (!OPTION_BP) {
        SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
        return SCPI_RES_ERR;
    }

    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    SCPI_ResultBool(context, channel->isRemoteSensingEnabled());

    return SCPI_RES_OK;
}

scpi_result_t scpi_source_VoltageProgramSource(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    if (!(channel->getFeatures() & CH_FEATURE_RPROG)) {
        SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
        return SCPI_RES_ERR;
    }

    int32_t choice;
    if (!SCPI_ParamChoice(context, internal_external_choice, &choice, TRUE)) {
        return SCPI_RES_ERR;
    }

    channel->remoteProgrammingEnable(choice == 0 ? false : true);

    return SCPI_RES_OK;
}

scpi_result_t scpi_source_VoltageProgramSourceQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    if (!(channel->getFeatures() & CH_FEATURE_RPROG)) {
        SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
        return SCPI_RES_ERR;
    }

    SCPI_ResultBool(context, channel->isRemoteProgrammingEnabled());

    return SCPI_RES_OK;
}

scpi_result_t scpi_source_LRipple(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    if (!(channel->getFeatures() & CH_FEATURE_LRIPPLE)) {
        SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
        return SCPI_RES_ERR;
    }

	bool enable;
	if (!SCPI_ParamBool(context, &enable, TRUE)) {
		return SCPI_RES_ERR;
	}

    if (!channel->lowRippleEnable(enable)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_source_LRippleQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    if (!(channel->getFeatures() & CH_FEATURE_RPROG)) {
        SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
        return SCPI_RES_ERR;
    }

    SCPI_ResultBool(context, channel->isLowRippleEnabled());

    return SCPI_RES_OK;
}

scpi_result_t scpi_source_LRippleAuto(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    if (!(channel->getFeatures() & CH_FEATURE_RPROG)) {
        SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
        return SCPI_RES_ERR;
    }

	bool enable;
	if (!SCPI_ParamBool(context, &enable, TRUE)) {
		return SCPI_RES_ERR;
	}

    channel->lowRippleAutoEnable(enable);

    return SCPI_RES_OK;
}

scpi_result_t scpi_source_LRippleAutoQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    if (!(channel->getFeatures() & CH_FEATURE_RPROG)) {
        SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
        return SCPI_RES_ERR;
    }

    SCPI_ResultBool(context, channel->isLowRippleAutoEnabled());

    return SCPI_RES_OK;
}

}
}
} // namespace eez::psu::scpi