/*
 * EEZ PSU Firmware
 * Copyright (C) 2017-present, Envox d.o.o.
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

#include "trigger.h"
#include "io_pins.h"
#include "channel_dispatcher.h"
#include "profile.h"

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

static scpi_choice_def_t sourceChoice[] = {
    { "BUS", trigger::SOURCE_BUS },
    { "IMMediate", trigger::SOURCE_IMMEDIATE },
    { "MANual", trigger::SOURCE_MANUAL },
    { "PIN1", trigger::SOURCE_PIN1 },
    SCPI_CHOICE_LIST_END
};

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_cmd_triggerSequenceImmediate(scpi_t * context) {
    int result = trigger::startImmediately();
    if (result != SCPI_RES_OK) {
        SCPI_ErrorPush(context, result);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_triggerSequenceDelay(scpi_t * context) {
    float delay;
    if (!get_duration_param(context, delay, trigger::DELAY_MIN, trigger::DELAY_MAX, trigger::DELAY_DEFAULT)) {
        return SCPI_RES_ERR;
    }

    trigger::setDelay(delay);
    persist_conf::saveDevice2();

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_triggerSequenceDelayQ(scpi_t * context) {
    SCPI_ResultFloat(context, trigger::getDelay());
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_triggerSequenceSource(scpi_t * context) {
    int32_t source;
    if (!SCPI_ParamChoice(context, sourceChoice, &source, true)) {
        return SCPI_RES_ERR;
    }

    trigger::setSource((trigger::Source)source);

    if (source == trigger::SOURCE_PIN1) {
        persist_conf::devConf2.ioPins[0].function = io_pins::FUNCTION_TINPUT;
    }

    persist_conf::saveDevice2();

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_triggerSequenceSourceQ(scpi_t * context) {
    resultChoiceName(context, sourceChoice, trigger::getSource());
    return SCPI_RES_OK;
}

static scpi_choice_def_t triggerOnListStopChoice[] = {
    { "OFF", TRIGGER_ON_LIST_STOP_OUTPUT_OFF },
    { "FIRSt", TRIGGER_ON_LIST_STOP_SET_TO_FIRST_STEP },
    { "LAST", TRIGGER_ON_LIST_STOP_SET_TO_LAST_STEP },
    { "STANdbay", TRIGGER_ON_LIST_STOP_STANDBY },
    SCPI_CHOICE_LIST_END /* termination of option list */
};

scpi_result_t scpi_cmd_triggerSequenceExitCondition(scpi_t * context) {
    int32_t triggerOnListStop;
    if (!SCPI_ParamChoice(context, triggerOnListStopChoice, &triggerOnListStop, true)) {
        return SCPI_RES_ERR;
    }

    Channel *channel = param_channel(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    if (!trigger::isIdle()) {
        SCPI_ErrorPush(context, SCPI_ERROR_CANNOT_CHANGE_TRANSIENT_TRIGGER);
        return SCPI_RES_ERR;
    }

    channel_dispatcher::setTriggerOnListStop(*channel, (TriggerOnListStop)triggerOnListStop);
    profile::save();

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_triggerSequenceExitConditionQ(scpi_t * context) {
    Channel *channel = param_channel(context);
    if (!channel) {
        return SCPI_RES_ERR;
    }

    resultChoiceName(context, triggerOnListStopChoice, channel_dispatcher::getTriggerOnListStop(*channel));

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_initiateImmediate(scpi_t * context) {
    int result = trigger::initiate();
    if (result != SCPI_RES_OK) {
        SCPI_ErrorPush(context, result);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_initiateContinuous(scpi_t * context) {
    bool enable;
    if (!SCPI_ParamBool(context, &enable, TRUE)) {
        return SCPI_RES_ERR;
    }

    int result = trigger::enableInitiateContinuous(enable);

    persist_conf::saveDevice2();

    if (result != SCPI_RES_OK) {
        SCPI_ErrorPush(context, result);
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_initiateContinuousQ(scpi_t * context) {
    SCPI_ResultBool(context, trigger::isContinuousInitializationEnabled() ? 1 : 0);
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_abort(scpi_t * context) {
    trigger::abort();

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_coreTrg(scpi_t * context) {
    int result = trigger::generateTrigger(trigger::SOURCE_BUS);
    if (result != SCPI_RES_OK) {
        SCPI_ErrorPush(context, result);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

}
}
} // namespace eez::psu::scpi