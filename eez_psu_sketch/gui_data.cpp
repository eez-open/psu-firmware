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

namespace eez {
namespace psu {
namespace gui {
namespace data {

Channel *selected_channel;

struct ChannelState {
    char u_mon[8];
    char i_mon[8];
    char u_set[8];
    char i_set[8];
};

ChannelState channel_last_state[CH_NUM];
ChannelState *selected_channel_last_state;

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
