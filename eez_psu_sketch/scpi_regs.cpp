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

namespace eez {
namespace app {
namespace scpi {

/**
* Update register value
* @param context
* @param name - register name
*/
static void psu_reg_update(scpi_t * context, scpi_psu_reg_name_t name) {
    reg_set(context, name, reg_get(context, name));
}

/**
* Update IEEE 488.2 register according to value and its mask register
* @param context
* @param val value of register
* @param mask name of mask register (enable register)
* @param bits bits to clear or set in STB
*/
static void psu_reg_update_ieee488_reg(scpi_t * context, scpi_reg_val_t val, scpi_psu_reg_name_t mask, scpi_reg_name_t ieee488RegName, scpi_reg_val_t bits) {
    if (val & reg_get(context, mask)) {
        SCPI_RegSetBits(context, ieee488RegName, bits);
    }
    else {
        SCPI_RegClearBits(context, ieee488RegName, bits);
    }
}

/**
* Set PSU register bits
* @param name - register name
* @param bits bit mask
*/
void psu_reg_set_bits(scpi_t * context, scpi_psu_reg_name_t name, scpi_reg_val_t bits) {
    reg_set(context, name, reg_get(context, name) | bits);
}

/**
* Clear PSU register bits
* @param name - register name
* @param bits bit mask
*/
void psu_reg_clear_bits(scpi_t * context, scpi_psu_reg_name_t name, scpi_reg_val_t bits) {
    reg_set(context, name, reg_get(context, name) & ~bits);
}

/**
* Update PSU register according to value and its mask register
* @param context
* @param val value of register
* @param mask name of mask register (enable register)
* @param bits bits to clear or set in STB
*/
static void psu_reg_update_psu_reg(scpi_t * context, scpi_reg_val_t val, scpi_psu_reg_name_t mask, scpi_psu_reg_name_t psuRegName, scpi_reg_val_t bits) {
    if (val & reg_get(context, mask)) {
        psu_reg_set_bits(context, psuRegName, bits);
    }
    else {
        psu_reg_clear_bits(context, psuRegName, bits);
    }
    psu_reg_update(context, psuRegName);
}

/**
* Update PSU register according to value
* @param context
* @param val value of register
* @param mask name of mask register (enable register)
* @param bits bits to clear or set in STB
*/
static void psu_reg_update_psu_reg(scpi_t * context, scpi_reg_val_t val, scpi_psu_reg_name_t psuRegName, scpi_reg_val_t bits) {
    if (val) {
        psu_reg_set_bits(context, psuRegName, bits);
    }
    else {
        psu_reg_clear_bits(context, psuRegName, bits);
    }
    psu_reg_update(context, psuRegName);
}

/**
* Get PSU specific register value
* @param name - register name
* @return register value
*/
scpi_reg_val_t reg_get(scpi_t * context, scpi_psu_reg_name_t name) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;
    if ((name < SCPI_PSU_REG_COUNT) && (psu_context->registers != NULL)) {
        return psu_context->registers[name];
    }
    else {
        return 0;
    }
}

/**
* Set PSU specific register value
* @param name - register name
* @param val - new value
*/
void reg_set(scpi_t *context, scpi_psu_reg_name_t name, scpi_reg_val_t val) {
    scpi_psu_t *psu_context = (scpi_psu_t *)context->user_context;

    if ((name >= SCPI_PSU_REG_COUNT) || (psu_context->registers == NULL)) {
        return;
    }

    /* set register value */
    psu_context->registers[name] = val;

    switch (name) {
    case SCPI_PSU_REG_QUES_INST_COND:
        psu_reg_update_psu_reg(context, val, SCPI_PSU_REG_QUES_COND, QUES_ISUM);
        break;
    case SCPI_PSU_REG_QUES_INST_EVENT:
        psu_reg_update_ieee488_reg(context, val, SCPI_PSU_REG_QUES_INST_ENABLE, SCPI_REG_QUES, QUES_ISUM);
        break;
    case SCPI_PSU_REG_QUES_INST_ENABLE:
        psu_reg_update(context, SCPI_PSU_REG_QUES_INST_EVENT);
        break;

    case SCPI_PSU_REG_OPER_INST_COND:
        psu_reg_update_psu_reg(context, val, SCPI_PSU_REG_OPER_COND, OPER_ISUM);
        break;
    case SCPI_PSU_REG_OPER_INST_EVENT:
        psu_reg_update_ieee488_reg(context, val, SCPI_PSU_REG_OPER_INST_ENABLE, SCPI_REG_OPER, OPER_ISUM);
        break;
    case SCPI_PSU_REG_OPER_INST_ENABLE:
        psu_reg_update(context, SCPI_PSU_REG_OPER_INST_EVENT);
        break;

    case SCPI_PSU_CH_REG_QUES_INST_ISUM_COND1:
        psu_reg_update_psu_reg(context, val, SCPI_PSU_REG_QUES_INST_COND, QUES_ISUM1);
        break;
    case SCPI_PSU_CH_REG_QUES_INST_ISUM_EVENT1:
        psu_reg_update_psu_reg(context, val, SCPI_PSU_CH_REG_QUES_INST_ISUM_ENABLE1, SCPI_PSU_REG_QUES_INST_EVENT, QUES_ISUM1);
        break;
    case SCPI_PSU_CH_REG_QUES_INST_ISUM_ENABLE1:
        psu_reg_update(context, SCPI_PSU_CH_REG_QUES_INST_ISUM_EVENT1);
        break;

    case SCPI_PSU_CH_REG_OPER_INST_ISUM_COND1:
        psu_reg_update_psu_reg(context, val, SCPI_PSU_REG_OPER_INST_COND, OPER_ISUM1);
        break;
    case SCPI_PSU_CH_REG_OPER_INST_ISUM_EVENT1:
        psu_reg_update_psu_reg(context, val, SCPI_PSU_CH_REG_OPER_INST_ISUM_ENABLE1, SCPI_PSU_REG_OPER_INST_EVENT, OPER_ISUM1);
        break;
    case SCPI_PSU_CH_REG_OPER_INST_ISUM_ENABLE1:
        psu_reg_update(context, SCPI_PSU_CH_REG_OPER_INST_ISUM_EVENT1);
        break;

    case SCPI_PSU_CH_REG_QUES_INST_ISUM_COND2:
        psu_reg_update_psu_reg(context, val, SCPI_PSU_REG_QUES_INST_COND, QUES_ISUM2);
        break;
    case SCPI_PSU_CH_REG_QUES_INST_ISUM_EVENT2:
        psu_reg_update_psu_reg(context, val, SCPI_PSU_CH_REG_QUES_INST_ISUM_ENABLE2, SCPI_PSU_REG_QUES_INST_EVENT, QUES_ISUM2);
        break;
    case SCPI_PSU_CH_REG_QUES_INST_ISUM_ENABLE2:
        psu_reg_update(context, SCPI_PSU_CH_REG_QUES_INST_ISUM_EVENT2);
        break;

    case SCPI_PSU_CH_REG_OPER_INST_ISUM_COND2:
        psu_reg_update_psu_reg(context, val, SCPI_PSU_REG_OPER_INST_COND, OPER_ISUM2);
        break;
    case SCPI_PSU_CH_REG_OPER_INST_ISUM_EVENT2:
        psu_reg_update_psu_reg(context, val, SCPI_PSU_CH_REG_OPER_INST_ISUM_ENABLE2, SCPI_PSU_REG_OPER_INST_EVENT, OPER_ISUM2);
        break;
    case SCPI_PSU_CH_REG_OPER_INST_ISUM_ENABLE2:
        psu_reg_update(context, SCPI_PSU_CH_REG_OPER_INST_ISUM_EVENT2);
        break;

    default:
        /* nothing to do */
        break;
    }
}

int reg_get_ques_isum_bit_mask_for_channel_protection_value(Channel *channel, Channel::ProtectionValue &cpv) {
    if (IS_OVP_VALUE(channel, cpv))
        return QUES_ISUM_OVP;
    if (IS_OCP_VALUE(channel, cpv))
        return QUES_ISUM_OCP;
    return QUES_ISUM_OPP;
}

void reg_set_ques_bit(scpi_t *context, int bit_mask, bool on) {
    scpi_reg_val_t val = reg_get(context, SCPI_PSU_REG_QUES_COND);
    if (on) {
        if (!(val & bit_mask)) {
            reg_set(context, SCPI_PSU_REG_QUES_COND, val | bit_mask);

            // set event on raising condition
            val = SCPI_RegGet(context, SCPI_REG_QUES);
            SCPI_RegSet(context, SCPI_REG_QUES, val | bit_mask);
        }
    }
    else {
        if (val & bit_mask) {
            reg_set(context, SCPI_PSU_REG_QUES_COND, val & ~bit_mask);
        }
    }
}

void reg_set_ques_isum_bit(scpi_t *context, Channel *channel, int bit_mask, bool on) {
    scpi_psu_reg_name_t reg_name = channel->index == 1 ? SCPI_PSU_CH_REG_QUES_INST_ISUM_COND1 : SCPI_PSU_CH_REG_QUES_INST_ISUM_COND2;
    scpi_reg_val_t val = reg_get(context, reg_name);
    if (on) {
        if (!(val & bit_mask)) {
            reg_set(context, reg_name, val | bit_mask);

            // set event on raising condition
            reg_name = channel->index == 1 ? SCPI_PSU_CH_REG_QUES_INST_ISUM_EVENT1 : SCPI_PSU_CH_REG_QUES_INST_ISUM_EVENT2;
            val = reg_get(context, reg_name);
            reg_set(context, reg_name, val | bit_mask);
        }
    }
    else {
        if (val & bit_mask) {
            reg_set(context, reg_name, val & ~bit_mask);
        }
    }
}

void reg_set_oper_bit(scpi_t *context, int bit_mask, bool on) {
    scpi_reg_val_t val = reg_get(context, SCPI_PSU_REG_OPER_COND);
    if (on) {
        if (!(val & bit_mask)) {
            reg_set(context, SCPI_PSU_REG_OPER_COND, val | bit_mask);

            // set event on raising condition
            val = SCPI_RegGet(context, SCPI_REG_OPER);
            SCPI_RegSet(context, SCPI_REG_OPER, val | bit_mask);
        }
    }
    else {
        if (val & bit_mask) {
            reg_set(context, SCPI_PSU_REG_OPER_COND, val & ~bit_mask);
        }
    }
}

void reg_set_oper_isum_bit(scpi_t *context, Channel *channel, int bit_mask, bool on) {
    scpi_psu_reg_name_t reg_name = channel->index == 1 ? SCPI_PSU_CH_REG_OPER_INST_ISUM_COND1 : SCPI_PSU_CH_REG_OPER_INST_ISUM_COND2;
    scpi_reg_val_t val = reg_get(context, reg_name);
    if (on) {
        if (!(val & bit_mask)) {
            reg_set(context, reg_name, val | bit_mask);

            // set event on raising condition
            reg_name = channel->index == 1 ? SCPI_PSU_CH_REG_OPER_INST_ISUM_EVENT1 : SCPI_PSU_CH_REG_OPER_INST_ISUM_EVENT2;
            val = reg_get(context, reg_name);
            reg_set(context, reg_name, val | bit_mask);
        }
    }
    else {
        if (val & bit_mask) {
            reg_set(context, reg_name, val & ~bit_mask);
        }
    }
}

}
}
} // namespace eez::app::scpi