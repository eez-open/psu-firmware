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

#include "profile.h"
#include "channel_dispatcher.h"
#include "trigger.h"
#include "list.h"

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

scpi_result_t get_delay(scpi_t *context, float delay) {
    SCPI_ResultFloat(context, delay);

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

scpi_result_t scpi_cmd_sourceCurrentLevelImmediateAmplitude(scpi_t * context) {
	Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    float current;
    if (!get_current_param(context, current, channel, &channel->i)) {
        return SCPI_RES_ERR;
    }

	if (current > channel_dispatcher::getILimit(*channel)) {
        SCPI_ErrorPush(context, SCPI_ERROR_CURRENT_LIMIT_EXCEEDED);
        return SCPI_RES_ERR;
	}

    if (current * channel_dispatcher::getUSet(*channel) > channel_dispatcher::getPowerLimit(*channel)) {
        SCPI_ErrorPush(context, SCPI_ERROR_POWER_LIMIT_EXCEEDED);
        return SCPI_RES_ERR;
    }

    channel_dispatcher::setCurrent(*channel, current);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceCurrentLevelImmediateAmplitudeQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_source_value(context, 
        channel_dispatcher::getISet(*channel),
        channel_dispatcher::getIMin(*channel),
        channel_dispatcher::getIMax(*channel),
        channel_dispatcher::getIDef(*channel));
}

scpi_result_t scpi_cmd_sourceVoltageLevelImmediateAmplitude(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    float voltage;
    if (!get_voltage_param(context, voltage, channel, &channel->u)) {
        return SCPI_RES_ERR;
    }

	if (channel->isRemoteProgrammingEnabled()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
	}

	if (voltage > channel_dispatcher::getULimit(*channel)) {
        SCPI_ErrorPush(context, SCPI_ERROR_VOLTAGE_LIMIT_EXCEEDED);
        return SCPI_RES_ERR;
	}

	if (voltage * channel_dispatcher::getISet(*channel) > channel_dispatcher::getPowerLimit(*channel)) {
        SCPI_ErrorPush(context, SCPI_ERROR_POWER_LIMIT_EXCEEDED);
        return SCPI_RES_ERR;
    }

    channel_dispatcher::setVoltage(*channel, voltage);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceVoltageLevelImmediateAmplitudeQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

	float u;
	if (channel->isRemoteProgrammingEnabled()) {
		u = channel->u.mon_dac;
	} else {
		u = channel_dispatcher::getUSet(*channel);
	}

    return get_source_value(context, u,
        channel_dispatcher::getUMin(*channel),
        channel_dispatcher::getUMax(*channel),
        channel_dispatcher::getUDef(*channel));
}

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cmd_sourceCurrentLevelImmediateStepIncrement(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return set_step(context, &channel->i, channel->I_MIN_STEP, channel->I_MAX_STEP, channel->I_DEF_STEP, SCPI_UNIT_AMPER);
}

scpi_result_t scpi_cmd_sourceCurrentLevelImmediateStepIncrementQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_source_value(context, channel->i.step, channel->I_DEF_STEP);
}

scpi_result_t scpi_cmd_sourceVoltageLevelImmediateStepIncrement(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return set_step(context, &channel->u, channel->U_MIN_STEP, channel->U_MAX_STEP, channel->U_DEF_STEP, SCPI_UNIT_VOLT);
}

scpi_result_t scpi_cmd_sourceVoltageLevelImmediateStepIncrementQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_source_value(context, channel->u.step, channel->U_DEF_STEP);
}

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cmd_sourceCurrentProtectionDelayTime(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    float delay;
    if (!get_duration_param(context, delay, channel->OCP_MIN_DELAY, channel->OCP_MAX_DELAY, channel->OCP_DEFAULT_DELAY)) {
        return SCPI_RES_ERR;
    }

    channel_dispatcher::setOcpDelay(*channel, delay);
    
    profile::save();
    
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceCurrentProtectionDelayTimeQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_delay(context, channel->prot_conf.i_delay);
}

scpi_result_t scpi_cmd_sourceCurrentProtectionState(scpi_t *context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }
    
	bool state;
	if (!SCPI_ParamBool(context, &state, TRUE)) {
		return SCPI_RES_ERR;
	}

    channel_dispatcher::setOcpState(*channel, state);

	return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceCurrentProtectionStateQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_state(context, channel, I_STATE);
}

scpi_result_t scpi_cmd_sourceCurrentProtectionTrippedQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }
    
    return get_tripped(context, channel->ocp);
}

scpi_result_t scpi_cmd_sourcePowerProtectionLevel(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
	}

	float power;
	if (!get_power_param(context, power, channel_dispatcher::getOppMinLevel(*channel), channel_dispatcher::getOppMaxLevel(*channel), channel_dispatcher::getOppDefaultLevel(*channel))) {
		return SCPI_RES_ERR;
	}

    channel_dispatcher::setOppLevel(*channel, power);
    profile::save();

	return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourcePowerProtectionLevelQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }
    
    return get_source_value(context, channel_dispatcher::getPowerProtectionLevel(*channel),
        channel_dispatcher::getOppMinLevel(*channel),
        channel_dispatcher::getOppMaxLevel(*channel),
        channel_dispatcher::getOppDefaultLevel(*channel));
}

scpi_result_t scpi_cmd_sourcePowerProtectionDelayTime(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    float delay;
    if (!get_duration_param(context, delay, channel->OPP_MIN_DELAY, channel->OPP_MAX_DELAY, channel->OPP_DEFAULT_DELAY)) {
        return SCPI_RES_ERR;
    }

    channel_dispatcher::setOppDelay(*channel, delay);
    
    profile::save();
    
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourcePowerProtectionDelayTimeQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_delay(context, channel->prot_conf.p_delay);
}

scpi_result_t scpi_cmd_sourcePowerProtectionState(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }
    
	bool state;
	if (!SCPI_ParamBool(context, &state, TRUE)) {
		return SCPI_RES_ERR;
	}

    channel_dispatcher::setOppState(*channel, state);

	return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourcePowerProtectionStateQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }
    
    return get_state(context, channel, P_STATE);
}

scpi_result_t scpi_cmd_sourcePowerProtectionTrippedQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_tripped(context, channel->opp);
}

scpi_result_t scpi_cmd_sourceVoltageProtectionLevel(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
	}

    float voltage;
    if (!get_voltage_protection_level_param(context, voltage, channel_dispatcher::getUSet(*channel), channel_dispatcher::getUMax(*channel), channel_dispatcher::getUMax(*channel))) {
        return SCPI_RES_ERR;
    }

    channel_dispatcher::setOvpLevel(*channel, voltage);
    profile::save();

	return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceVoltageProtectionLevelQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }
    
    return get_source_value(context, channel_dispatcher::getUProtectionLevel(*channel),
        channel_dispatcher::getUSet(*channel),
        channel_dispatcher::getUMax(*channel),
        channel_dispatcher::getUMax(*channel));
}

scpi_result_t scpi_cmd_sourceVoltageProtectionDelayTime(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    float delay;
    if (!get_duration_param(context, delay, channel->OVP_MIN_DELAY, channel->OVP_MAX_DELAY, channel->OVP_DEFAULT_DELAY)) {
        return SCPI_RES_ERR;
    }

    channel_dispatcher::setOvpDelay(*channel, delay);
    
    profile::save();
    
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceVoltageProtectionDelayTimeQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_delay(context, channel->prot_conf.u_delay);
}

scpi_result_t scpi_cmd_sourceVoltageProtectionState(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }
    
	bool state;
	if (!SCPI_ParamBool(context, &state, TRUE)) {
		return SCPI_RES_ERR;
	}

    channel_dispatcher::setOvpState(*channel, state);

	return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceVoltageProtectionStateQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }
    
    return get_state(context, channel, U_STATE);
}

scpi_result_t scpi_cmd_sourceVoltageProtectionTrippedQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_tripped(context, channel->ovp);
}

scpi_result_t scpi_cmd_sourceVoltageSenseSource(scpi_t * context) {
    if (channel_dispatcher::isSeries()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTE_ERROR_CHANNELS_ARE_COUPLED);
        return SCPI_RES_ERR;
    }

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

    channel_dispatcher::remoteSensingEnable(*channel, choice == 0 ? false : true);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceVoltageSenseSourceQ(scpi_t * context) {
    if (channel_dispatcher::isSeries()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTE_ERROR_CHANNELS_ARE_COUPLED);
        return SCPI_RES_ERR;
    }

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

scpi_result_t scpi_cmd_sourceVoltageProgramSource(scpi_t * context) {
    if (channel_dispatcher::isCoupled()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTE_ERROR_CHANNELS_ARE_COUPLED);
        return SCPI_RES_ERR;
    }

    if (channel_dispatcher::isTracked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTE_ERROR_IN_TRACKING_MODE);
        return SCPI_RES_ERR;
    }

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

scpi_result_t scpi_cmd_sourceVoltageProgramSourceQ(scpi_t * context) {
    if (channel_dispatcher::isCoupled()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTE_ERROR_CHANNELS_ARE_COUPLED);
        return SCPI_RES_ERR;
    }

    if (channel_dispatcher::isTracked()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTE_ERROR_IN_TRACKING_MODE);
        return SCPI_RES_ERR;
    }

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

scpi_result_t scpi_cmd_sourceLripple(scpi_t * context) {
    if (channel_dispatcher::isSeries()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTE_ERROR_CHANNELS_ARE_COUPLED);
        return SCPI_RES_ERR;
    }

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

    if (!channel_dispatcher::lowRippleEnable(*channel, enable)) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTION_ERROR);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceLrippleQ(scpi_t * context) {
    if (channel_dispatcher::isSeries()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTE_ERROR_CHANNELS_ARE_COUPLED);
        return SCPI_RES_ERR;
    }

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

scpi_result_t scpi_cmd_sourceLrippleAuto(scpi_t * context) {
    if (channel_dispatcher::isSeries()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTE_ERROR_CHANNELS_ARE_COUPLED);
        return SCPI_RES_ERR;
    }

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

    channel_dispatcher::lowRippleAutoEnable(*channel, enable);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceLrippleAutoQ(scpi_t * context) {
    if (channel_dispatcher::isSeries()) {
        SCPI_ErrorPush(context, SCPI_ERROR_EXECUTE_ERROR_CHANNELS_ARE_COUPLED);
        return SCPI_RES_ERR;
    }

    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    if (!(channel->getFeatures() & CH_FEATURE_LRIPPLE)) {
        SCPI_ErrorPush(context, SCPI_ERROR_OPTION_NOT_INSTALLED);
        return SCPI_RES_ERR;
    }

    SCPI_ResultBool(context, channel->isLowRippleAutoEnabled());

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceCurrentLimitPositiveImmediateAmplitude(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    float limit;
    if (!get_current_limit_param(context, limit, channel, &channel->i)) {
        return SCPI_RES_ERR;
    }

    channel_dispatcher::setCurrentLimit(*channel, limit);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceCurrentLimitPositiveImmediateAmplitudeQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_source_value(context,
        channel_dispatcher::getILimit(*channel),
        0,
        channel_dispatcher::getIMaxLimit(*channel),
        channel_dispatcher::getIMaxLimit(*channel));
}

scpi_result_t scpi_cmd_sourceVoltageLimitPositiveImmediateAmplitude(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    float limit;
    if (!get_voltage_limit_param(context, limit, channel, &channel->i)) {
        return SCPI_RES_ERR;
    }

    channel_dispatcher::setVoltageLimit(*channel, limit);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceVoltageLimitPositiveImmediateAmplitudeQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_source_value(context,
        channel_dispatcher::getULimit(*channel),
        0,
        channel_dispatcher::getUMaxLimit(*channel),
        channel_dispatcher::getUMaxLimit(*channel));
}

scpi_result_t scpi_cmd_sourcePowerLimit(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    float limit;
    if (!get_power_limit_param(context, limit, channel, &channel->i)) {
        return SCPI_RES_ERR;
    }

    channel_dispatcher::setPowerLimit(*channel, limit);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourcePowerLimitQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_source_value(context,
        channel_dispatcher::getPowerLimit(*channel),
        channel_dispatcher::getPowerMinLimit(*channel),
        channel_dispatcher::getPowerMaxLimit(*channel),
        channel_dispatcher::getPowerDefaultLimit(*channel));
}

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cmd_sourceCurrentLevelTriggeredAmplitude(scpi_t * context) {
	Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    float current;
    if (!get_current_param(context, current, channel, &channel->i)) {
        return SCPI_RES_ERR;
    }

    trigger::setCurrent(*channel, current);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceCurrentLevelTriggeredAmplitudeQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_source_value(context, 
        trigger::getCurrent(*channel),
        channel_dispatcher::getIMin(*channel),
        channel_dispatcher::getIMax(*channel),
        channel_dispatcher::getIDef(*channel));
}

scpi_result_t scpi_cmd_sourceVoltageLevelTriggeredAmplitude(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    float voltage;
    if (!get_voltage_param(context, voltage, channel, &channel->u)) {
        return SCPI_RES_ERR;
    }

    trigger::setVoltage(*channel, voltage);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceVoltageLevelTriggeredAmplitudeQ(scpi_t * context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    return get_source_value(context,
        trigger::getVoltage(*channel),
        channel_dispatcher::getUMin(*channel),
        channel_dispatcher::getUMax(*channel),
        channel_dispatcher::getUDef(*channel));
}

static scpi_choice_def_t triggerModeChoice[] = {
    { "FIXed", TRIGGER_MODE_FIXED },
    { "LIST", TRIGGER_MODE_LIST },
    { "STEP", TRIGGER_MODE_STEP },
    SCPI_CHOICE_LIST_END /* termination of option list */
};

scpi_result_t scpi_cmd_sourceCurrentMode(scpi_t *context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    int32_t triggerMode;
    if (!SCPI_ParamChoice(context, triggerModeChoice, &triggerMode, true)) {
        return SCPI_RES_ERR;
    }

    if (trigger::isExecuting()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CANNOT_CHANGE_TRANSIENT_TRIGGER);
        return SCPI_RES_ERR;
    }

    channel_dispatcher::setCurrentTriggerMode(*channel, (TriggerMode)triggerMode);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceCurrentModeQ(scpi_t *context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    resultChoiceName(context, triggerModeChoice, channel_dispatcher::getCurrentTriggerMode(*channel));

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceVoltageMode(scpi_t *context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    int32_t triggerMode;
    if (!SCPI_ParamChoice(context, triggerModeChoice, &triggerMode, true)) {
        return SCPI_RES_ERR;
    }

    if (trigger::isExecuting()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CANNOT_CHANGE_TRANSIENT_TRIGGER);
        return SCPI_RES_ERR;
    }

    channel_dispatcher::setVoltageTriggerMode(*channel, (TriggerMode)triggerMode);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceVoltageModeQ(scpi_t *context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    resultChoiceName(context, triggerModeChoice, channel_dispatcher::getVoltageTriggerMode(*channel));

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceListCount(scpi_t *context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    scpi_number_t param;
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param, true)) {
        return SCPI_RES_ERR;
    }

    uint8_t count;

    if (param.special) {
        if (param.tag == SCPI_NUM_INF) {
            count = 0;
        } else {
			SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
			return SCPI_RES_ERR;
        }
    } else {
        if (param.unit != SCPI_UNIT_NONE) {
            SCPI_ErrorPush(context, SCPI_ERROR_INVALID_SUFFIX);
            return SCPI_RES_ERR;
        }

        int value = (int)param.value;
        if (value < 0 || value > 255) {
			SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
			return SCPI_RES_ERR;
        }

        count = value;
    }

    if (trigger::isExecuting()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CANNOT_CHANGE_TRANSIENT_TRIGGER);
        return SCPI_RES_ERR;
    }

    list::setListCount(*channel, count);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceListCountQ(scpi_t *context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    SCPI_ResultInt(context, list::getListCount(*channel));

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceListCurrentLevel(scpi_t *context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    float list[MAX_LIST_SIZE];
    uint16_t listSize = 0;

    while (true) {
        float voltage;
        if (!SCPI_ParamFloat(context, &voltage, false)) {
            break;
        }

        if (listSize >= MAX_LIST_SIZE) {
            SCPI_ErrorPush(context, SCPI_ERROR_TOO_MANY_LIST_POINTS);
            return SCPI_RES_ERR;
        }

        list[listSize++] = voltage;
    }

    if (listSize == 0) {
        SCPI_ErrorPush(context, SCPI_ERROR_TOO_MANY_LIST_POINTS);
        return SCPI_RES_ERR;
    }

    if (trigger::isExecuting()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CANNOT_CHANGE_TRANSIENT_TRIGGER);
        return SCPI_RES_ERR;
    }

    list::setCurrentList(*channel, list, listSize);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceListCurrentLevelQ(scpi_t *context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    uint16_t listSize;
    float *list = list::getCurrentList(*channel, &listSize);
    SCPI_ResultArrayFloat(context, list, listSize, SCPI_FORMAT_ASCII);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceListDwell(scpi_t *context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    float list[MAX_LIST_SIZE];
    uint16_t listSize = 0;

    while (true) {
        float voltage;
        if (!SCPI_ParamFloat(context, &voltage, false)) {
            break;
        }

        if (listSize >= MAX_LIST_SIZE) {
            SCPI_ErrorPush(context, SCPI_ERROR_TOO_MANY_LIST_POINTS);
            return SCPI_RES_ERR;
        }

        list[listSize++] = voltage;
    }

    if (listSize == 0) {
        SCPI_ErrorPush(context, SCPI_ERROR_TOO_MANY_LIST_POINTS);
        return SCPI_RES_ERR;
    }

    if (trigger::isExecuting()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CANNOT_CHANGE_TRANSIENT_TRIGGER);
        return SCPI_RES_ERR;
    }

    list::setDwellList(*channel, list, listSize);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceListDwellQ(scpi_t *context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    uint16_t listSize;
    float *list = list::getDwellList(*channel, &listSize);
    SCPI_ResultArrayFloat(context, list, listSize, SCPI_FORMAT_ASCII);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceListVoltageLevel(scpi_t *context) {
    DebugTraceF("vi %ul", micros());

    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    float list[MAX_LIST_SIZE];
    uint16_t listSize = 0;

    while (true) {
        float voltage;
        if (!SCPI_ParamFloat(context, &voltage, false)) {
            break;
        }

        if (listSize >= MAX_LIST_SIZE) {
            SCPI_ErrorPush(context, SCPI_ERROR_TOO_MANY_LIST_POINTS);
            return SCPI_RES_ERR;
        }

        list[listSize++] = voltage;
    }

    if (listSize == 0) {
        SCPI_ErrorPush(context, SCPI_ERROR_TOO_MANY_LIST_POINTS);
        return SCPI_RES_ERR;
    }

    if (trigger::isExecuting()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CANNOT_CHANGE_TRANSIENT_TRIGGER);
        return SCPI_RES_ERR;
    }

    list::setVoltageList(*channel, list, listSize);

    DebugTraceF("vo %ul", micros());

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_sourceListVoltageLevelQ(scpi_t *context) {
    Channel *channel = set_channel_from_command_number(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    uint16_t listSize;
    float *list = list::getVoltageList(*channel, &listSize);
    SCPI_ResultArrayFloat(context, list, listSize, SCPI_FORMAT_ASCII);

    return SCPI_RES_OK;
}

}
}
} // namespace eez::psu::scpi