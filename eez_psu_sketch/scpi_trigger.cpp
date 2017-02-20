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

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

static scpi_choice_def_t sourceChoice[] = {
    { "BUS", trigger::SOURCE_BUS },
    { "IMMediate", trigger::SOURCE_IMMEDIATE },
    { "KEY", trigger::SOURCE_KEY },
    { "PIN1", trigger::SOURCE_PIN1 },
    SCPI_CHOICE_LIST_END
};

static scpi_choice_def_t polarityChoice[] = {
    { "POSitive", trigger::POLARITY_POSITIVE },
    { "NEGative", trigger::POLARITY_NEGATIVE },
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

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_triggerSequenceDelayQ(scpi_t * context) {
    SCPI_ResultFloat(context, trigger::getDelay());
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_triggerSequenceSlope(scpi_t * context) {
    int32_t polarity;
    if (!SCPI_ParamChoice(context, polarityChoice, &polarity, true)) {
        return SCPI_RES_ERR;
    }

    trigger::setPolarity((trigger::Polarity)polarity);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_triggerSequenceSlopeQ(scpi_t * context) {
    resultChoiceName(context, polarityChoice, trigger::getPolarity());
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_triggerSequenceSource(scpi_t * context) {
    int32_t source;
    if (!SCPI_ParamChoice(context, sourceChoice, &source, true)) {
        return SCPI_RES_ERR;
    }

    trigger::setSource((trigger::Source)source);

    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_triggerSequenceSourceQ(scpi_t * context) {
    resultChoiceName(context, sourceChoice, trigger::getSource());
    return SCPI_RES_OK;
}

scpi_result_t scpi_cmd_initiate(scpi_t * context) {
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