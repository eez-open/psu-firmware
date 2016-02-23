/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
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
#include "gui_data.h"

#include "channel.h"

#define DATA_ID_CHANNELS 1
#define DATA_ID_OUTPUT_STATE 2
#define DATA_ID_OUTPUT_MODE 3
#define DATA_ID_MEAS_VOLT 4
#define DATA_ID_MEAS_CURR 5
#define DATA_ID_VOLT 6
#define DATA_ID_CURR 7
#define DATA_ID_OVP 8
#define DATA_ID_OCP 9
#define DATA_ID_OPP 10
#define DATA_ID_OTP 11

namespace eez {
namespace psu {
namespace gui {
namespace data {

static Channel *selected_channel;

struct ChannelState {
    char u_mon[8];
    char i_mon[8];
    char u_set[8];
    char i_set[8];
    uint8_t ovp;
    uint8_t ocp;
    uint8_t opp;
    uint8_t otp;
};


static ChannelState channel_last_state[CH_NUM];
static ChannelState *selected_channel_last_state;

int count(uint16_t id) {
    if (id == DATA_ID_CHANNELS) {
        return CH_NUM;
    }
    return 0;
}

void select(uint16_t id, int index) {
    if (id == DATA_ID_CHANNELS) {
        selected_channel = &Channel::get(index);
        selected_channel_last_state = channel_last_state + index;
    }
}

char *get(uint16_t id) {
    static char value[8];

    if (id == DATA_ID_OUTPUT_STATE) {
        if (selected_channel->isOutputEnabled()) return (char *)1;
        return (char *)0;
    } if (id == DATA_ID_OUTPUT_MODE) {
        // TODO improve
        char *mode_str = selected_channel->getCvModeStr();
        if (strcmp(mode_str, "CV") == 0) return (char *)0;
        if (strcmp(mode_str, "CC") == 0) return (char *)1;
        return (char *)2;
    } else if (id == DATA_ID_MEAS_VOLT) {
        value[0] = 0;
        util::strcatVoltage(value, selected_channel->u.mon);
        if (strcmp(selected_channel_last_state->u_mon, value) == 0) return 0;
        strcpy(selected_channel_last_state->u_mon, value);
        return value;
    } else if (id == DATA_ID_MEAS_CURR) {
        value[0] = 0;
        util::strcatCurrent(value, selected_channel->i.mon);
        if (strcmp(selected_channel_last_state->i_mon, value) == 0) return 0;
        strcpy(selected_channel_last_state->i_mon, value);
        return value;
    } else if (id == DATA_ID_VOLT) {
        value[0] = 0;
        util::strcatVoltage(value, selected_channel->u.set);
        if (strcmp(selected_channel_last_state->u_set, value) == 0) return 0;
        strcpy(selected_channel_last_state->u_set, value);
        return value;
    } else if (id == DATA_ID_CURR) {
        value[0] = 0;
        util::strcatCurrent(value, selected_channel->i.set);
        if (strcmp(selected_channel_last_state->i_set, value) == 0) return 0;
        strcpy(selected_channel_last_state->i_set, value);
        return value;
    } else if (id == DATA_ID_OVP) {
        uint8_t value;
        if (!selected_channel->prot_conf.flags.u_state) value = 1;
        else if (!selected_channel->ovp.flags.tripped) value = 2;
        else value = 3;
        if (value != selected_channel_last_state->ovp) {
            selected_channel_last_state->ovp = value;
            return (char *)value;
        }
    } else if (id == DATA_ID_OCP) {
        uint8_t value;
        if (!selected_channel->prot_conf.flags.i_state) value = 1;
        else if (!selected_channel->ocp.flags.tripped) value = 2;
        else value = 3;
        if (value != selected_channel_last_state->ocp) {
            selected_channel_last_state->ocp = value;
            return (char *)value;
        }
    } else if (id == DATA_ID_OPP) {
        uint8_t value;
        if (!selected_channel->prot_conf.flags.p_state) value = 1;
        else if (!selected_channel->opp.flags.tripped) value = 2;
        else value = 3;
        if (value != selected_channel_last_state->opp) {
            selected_channel_last_state->opp = value;
            return (char *)value;
        }
    } else if (id == DATA_ID_OTP) {
        uint8_t value;
        if (!temperature::prot_conf[temp_sensor::MAIN].state) value = 1;
        else if (!temperature::isSensorTripped(temp_sensor::MAIN)) value = 2;
        else value = 3;
        if (value != selected_channel_last_state->otp) {
            selected_channel_last_state->otp = value;
            return (char *)value;
        }
    }

    return 0;
}

void set(uint16_t id, const char *value) {
    if (id == DATA_ID_VOLT) {
    } else if (id == DATA_ID_CURR) {
    }
}

}
}
}
} // namespace eez::psu::ui::data
