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
#include "scpi_stat.h"

namespace eez {
namespace psu {
namespace scpi {

////////////////////////////////////////////////////////////////////////////////

scpi_result_t scpi_stat_QuestionableEventQ(scpi_t * context) {
    /* return value */
    SCPI_ResultInt32(context, SCPI_RegGet(context, SCPI_REG_QUES));

    /* clear register */
    SCPI_RegSet(context, SCPI_REG_QUES, 0);

    return SCPI_RES_OK;
}

scpi_result_t scpi_stat_QuestionableConditionQ(scpi_t * context) {
    /* return value */
    SCPI_ResultInt32(context, reg_get(context, SCPI_PSU_REG_QUES_COND));

    return SCPI_RES_OK;
}

scpi_result_t scpi_stat_QuestionableEnable(scpi_t * context) {
    int32_t new_QUESE;
    if (SCPI_ParamInt32(context, &new_QUESE, TRUE)) {
        SCPI_RegSet(context, SCPI_REG_QUESE, (scpi_reg_val_t)new_QUESE);
        return SCPI_RES_OK;
    }
    return SCPI_RES_ERR;
}

scpi_result_t scpi_stat_QuestionableEnableQ(scpi_t * context) {
    /* return value */
    SCPI_ResultInt32(context, SCPI_RegGet(context, SCPI_REG_QUESE));
 
    return SCPI_RES_OK;
}


scpi_result_t scpi_stat_OperationEventQ(scpi_t * context) {
    /* return value */
    SCPI_ResultInt32(context, SCPI_RegGet(context, SCPI_REG_OPER));

    /* clear register */
    SCPI_RegSet(context, SCPI_REG_OPER, 0);

    return SCPI_RES_OK;
}

scpi_result_t scpi_stat_OperationConditionQ(scpi_t * context) {
    /* return value */
    SCPI_ResultInt32(context, reg_get(context, SCPI_PSU_REG_OPER_COND));

    return SCPI_RES_OK;
}

scpi_result_t scpi_stat_OperationEnable(scpi_t * context) {
    int32_t new_OPERE;
    if (SCPI_ParamInt32(context, &new_OPERE, TRUE)) {
        SCPI_RegSet(context, SCPI_REG_OPERE, (scpi_reg_val_t)new_OPERE);
        return SCPI_RES_OK;
    }
    return SCPI_RES_ERR;
}

scpi_result_t scpi_stat_OperationEnableQ(scpi_t * context) {
    /* return value */
    SCPI_ResultInt32(context, SCPI_RegGet(context, SCPI_REG_OPERE));

    return SCPI_RES_OK;
}

scpi_result_t scpi_stat_QuestionableInstrumentEventQ(scpi_t * context) {
    /* return value */
    SCPI_ResultInt32(context, reg_get(context, SCPI_PSU_REG_QUES_INST_EVENT));

    /* clear register */
    reg_set(context, SCPI_PSU_REG_QUES_INST_EVENT, 0);

    return SCPI_RES_OK;
}

scpi_result_t scpi_stat_QuestionableInstrumentConditionQ(scpi_t * context) {
    /* return value */
    SCPI_ResultInt32(context, reg_get(context, SCPI_PSU_REG_QUES_INST_COND));

    return SCPI_RES_OK;
}

scpi_result_t scpi_stat_QuestionableInstrumentEnable(scpi_t * context) {
    int32_t newVal;
    if (SCPI_ParamInt32(context, &newVal, TRUE)) {
        reg_set(context, SCPI_PSU_REG_QUES_INST_ENABLE, (scpi_reg_val_t)newVal);
        return SCPI_RES_OK;
    }
    return SCPI_RES_ERR;
}

scpi_result_t scpi_stat_QuestionableInstrumentEnableQ(scpi_t * context) {
    /* return value */
    SCPI_ResultInt32(context, reg_get(context, SCPI_PSU_REG_QUES_INST_ENABLE));

    return SCPI_RES_OK;
}

scpi_result_t scpi_stat_OperationInstrumentEventQ(scpi_t * context) {
    /* return value */
    SCPI_ResultInt32(context, reg_get(context, SCPI_PSU_REG_OPER_INST_EVENT));

    /* clear register */
    reg_set(context, SCPI_PSU_REG_OPER_INST_EVENT, 0);

    return SCPI_RES_OK;
}

scpi_result_t scpi_stat_OperationInstrumentConditionQ(scpi_t * context) {
    /* return value */
    SCPI_ResultInt32(context, reg_get(context, SCPI_PSU_REG_OPER_INST_COND));

    return SCPI_RES_OK;
}

scpi_result_t scpi_stat_OperationInstrumentEnable(scpi_t * context) {
    int32_t newVal;
    if (SCPI_ParamInt32(context, &newVal, TRUE)) {
        reg_set(context, SCPI_PSU_REG_OPER_INST_ENABLE, (scpi_reg_val_t)newVal);
        return SCPI_RES_OK;
    }
    return SCPI_RES_ERR;
}

scpi_result_t scpi_stat_OperationInstrumentEnableQ(scpi_t * context) {
    /* return value */
    SCPI_ResultInt32(context, reg_get(context, SCPI_PSU_REG_OPER_INST_ENABLE));

    return SCPI_RES_OK;
}

scpi_result_t scpi_stat_QuestionableInstrumentISummaryEventQ(scpi_t * context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;

    int32_t ch;
    SCPI_CommandNumbers(context, &ch, 1, psu_context->selected_channel_index);
    if (ch < 1 || ch > min(CH_NUM, 2)) {
        SCPI_ErrorPush(context, SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE);
        return SCPI_RES_OK;
    }

    scpi_psu_reg_name_t isumReg = ch == 1 ? SCPI_PSU_CH_REG_QUES_INST_ISUM_EVENT1 : SCPI_PSU_CH_REG_QUES_INST_ISUM_EVENT2;

    /* return value */
    SCPI_ResultInt32(context, reg_get(context, isumReg));

    /* clear register */
    reg_set(context, isumReg, 0);

    return SCPI_RES_OK;
}

scpi_result_t scpi_stat_QuestionableInstrumentISummaryConditionQ(scpi_t * context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;

    int32_t ch;
    SCPI_CommandNumbers(context, &ch, 1, psu_context->selected_channel_index);
    if (ch < 1 || ch > min(CH_NUM, 2)) {
        SCPI_ErrorPush(context, SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE);
        return SCPI_RES_OK;
    }

    scpi_psu_reg_name_t isumReg = ch == 1 ? SCPI_PSU_CH_REG_QUES_INST_ISUM_COND1 : SCPI_PSU_CH_REG_QUES_INST_ISUM_COND2;

    /* return value */
    SCPI_ResultInt32(context, reg_get(context, isumReg));

    return SCPI_RES_OK;
}

scpi_result_t scpi_stat_QuestionableInstrumentISummaryEnable(scpi_t * context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;

    int32_t ch;
    SCPI_CommandNumbers(context, &ch, 1, psu_context->selected_channel_index);
    if (ch < 1 || ch > min(CH_NUM, 2)) {
        SCPI_ErrorPush(context, SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE);
        return SCPI_RES_OK;
    }

    scpi_psu_reg_name_t isumeReg = ch == 1 ? SCPI_PSU_CH_REG_QUES_INST_ISUM_ENABLE1 : SCPI_PSU_CH_REG_QUES_INST_ISUM_ENABLE2;

    int32_t newVal;
    if (SCPI_ParamInt32(context, &newVal, TRUE)) {
        reg_set(context, isumeReg, (scpi_reg_val_t)newVal);
        return SCPI_RES_OK;
    }
    return SCPI_RES_ERR;
}

scpi_result_t scpi_stat_QuestionableInstrumentISummaryEnableQ(scpi_t * context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;

    int32_t ch;
    SCPI_CommandNumbers(context, &ch, 1, psu_context->selected_channel_index);
    if (ch < 1 || ch > min(CH_NUM, 2)) {
        SCPI_ErrorPush(context, SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE);
        return SCPI_RES_OK;
    }

    scpi_psu_reg_name_t isumeReg = ch == 1 ? SCPI_PSU_CH_REG_QUES_INST_ISUM_ENABLE1 : SCPI_PSU_CH_REG_QUES_INST_ISUM_ENABLE2;

    /* return value */
    SCPI_ResultInt32(context, reg_get(context, isumeReg));

    return SCPI_RES_OK;
}

scpi_result_t scpi_stat_OperationInstrumentISummaryEventQ(scpi_t * context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;

    int32_t ch;
    SCPI_CommandNumbers(context, &ch, 1, psu_context->selected_channel_index);
    if (ch < 1 || ch > min(CH_NUM, 2)) {
        SCPI_ErrorPush(context, SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE);
        return SCPI_RES_OK;
    }

    scpi_psu_reg_name_t isumReg = ch == 1 ? SCPI_PSU_CH_REG_OPER_INST_ISUM_EVENT1 : SCPI_PSU_CH_REG_OPER_INST_ISUM_EVENT2;

    /* return value */
    SCPI_ResultInt32(context, reg_get(context, isumReg));

    /* clear register */
    reg_set(context, isumReg, 0);

    return SCPI_RES_OK;
}

scpi_result_t scpi_stat_OperationInstrumentISummaryConditionQ(scpi_t * context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;

    int32_t ch;
    SCPI_CommandNumbers(context, &ch, 1, psu_context->selected_channel_index);
    if (ch < 1 || ch > min(CH_NUM, 2)) {
        SCPI_ErrorPush(context, SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE);
        return SCPI_RES_OK;
    }

    scpi_psu_reg_name_t isumReg = ch == 1 ? SCPI_PSU_CH_REG_OPER_INST_ISUM_COND1 : SCPI_PSU_CH_REG_OPER_INST_ISUM_COND2;

    /* return value */
    SCPI_ResultInt32(context, reg_get(context, isumReg));

    return SCPI_RES_OK;
}

scpi_result_t scpi_stat_OperationInstrumentISummaryEnable(scpi_t * context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;

    int32_t ch;
    SCPI_CommandNumbers(context, &ch, 1, psu_context->selected_channel_index);
    if (ch < 1 || ch > min(CH_NUM, 2)) {
        SCPI_ErrorPush(context, SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE);
        return SCPI_RES_OK;
    }

    scpi_psu_reg_name_t isumeReg = ch == 1 ? SCPI_PSU_CH_REG_OPER_INST_ISUM_ENABLE1 : SCPI_PSU_CH_REG_OPER_INST_ISUM_ENABLE2;

    int32_t newVal;
    if (SCPI_ParamInt32(context, &newVal, TRUE)) {
        reg_set(context, isumeReg, (scpi_reg_val_t)newVal);
        return SCPI_RES_OK;
    }
    return SCPI_RES_ERR;
}

scpi_result_t scpi_stat_OperationInstrumentISummaryEnableQ(scpi_t * context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;

    int32_t ch;
    SCPI_CommandNumbers(context, &ch, 1, psu_context->selected_channel_index);
    if (ch < 1 || ch > min(CH_NUM, 2)) {
        SCPI_ErrorPush(context, SCPI_ERROR_HEADER_SUFFIX_OUTOFRANGE);
        return SCPI_RES_OK;
    }

    scpi_psu_reg_name_t isumeReg = ch == 1 ? SCPI_PSU_CH_REG_OPER_INST_ISUM_ENABLE1 : SCPI_PSU_CH_REG_OPER_INST_ISUM_ENABLE2;

    /* return value */
    SCPI_ResultInt32(context, reg_get(context, isumeReg));

    return SCPI_RES_OK;
}

scpi_result_t scpi_stat_Preset(scpi_t * context) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;

    SCPI_RegSet(context, SCPI_REG_ESE, 0);

    SCPI_RegSet(context, SCPI_REG_QUESE, 0);
    SCPI_RegSet(context, SCPI_REG_OPERE, 0);

    reg_set(context, SCPI_PSU_REG_QUES_INST_ENABLE, 0);
    reg_set(context, SCPI_PSU_REG_OPER_INST_ENABLE, 0);

    reg_set(context, SCPI_PSU_CH_REG_QUES_INST_ISUM_ENABLE1, 0);
    reg_set(context, SCPI_PSU_CH_REG_OPER_INST_ISUM_ENABLE1, 0);
    reg_set(context, SCPI_PSU_CH_REG_QUES_INST_ISUM_ENABLE2, 0);
    reg_set(context, SCPI_PSU_CH_REG_OPER_INST_ISUM_ENABLE2, 0);

    return SCPI_RES_OK;
}

}
}
} // namespace eez::psu::scpi