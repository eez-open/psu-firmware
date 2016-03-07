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

#include "gui_view.h"

namespace eez {
namespace psu {
namespace gui {
namespace data {

void Value::toText(char *text) {
    text[0] = 0;

    util::strcatFloat(text, float_);

    switch (unit_) {
    case UNIT_VOLT:
        strcat(text, " V");
        break;
    
    case UNIT_AMPER:
        strcat(text, " A");
        break;
    }
}

void Value::toTextNoUnit(char *text) {
    text[0] = 0;

    util::strcatFloat(text, float_);
}

static Cursor cursor;

struct ChannelStateFlags {
    unsigned mode : 2;
    unsigned state : 2;
    unsigned ovp : 2;
    unsigned ocp : 2;
    unsigned opp : 2;
    unsigned otp : 2;
    unsigned dp : 2;
};

struct ChannelState {
    Value mon_value;
    float u_set;
    float i_set;
    ChannelStateFlags flags;
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
        cursor.selected_channel = &Channel::get(index);
        selected_channel_last_state = channel_last_state + index;
    }
}

Value get(uint16_t id, bool &changed) {
    Value value;

    changed = false;

    if (id == DATA_ID_OUTPUT_STATE) {
        uint8_t state = cursor.selected_channel->isOutputEnabled() ? 1 : 0;
        if (state != selected_channel_last_state->flags.state) {
            selected_channel_last_state->flags.state = state;
            changed = true;
        }
        value = Value(state);
    } else if (id == DATA_ID_OUTPUT_MODE) {
        char *mode_str = cursor.selected_channel->getCvModeStr();
        uint8_t mode = strcmp(mode_str, "UR") == 0 ? 1 : 0;
        if (mode != selected_channel_last_state->flags.mode) {
            selected_channel_last_state->flags.mode = mode;
            changed = true;
        }
        value = Value(mode);
    } else if (id == DATA_ID_MON_VALUE) {
        float mon_value;
        Unit unit;
        char *mode_str = cursor.selected_channel->getCvModeStr();
        if (strcmp(mode_str, "CC") != 0) {
            // CC -> volt
            mon_value = cursor.selected_channel->u.mon;
            unit = UNIT_VOLT;
        } else if (strcmp(mode_str, "CV") != 0) {
            // CV -> curr
            mon_value = cursor.selected_channel->i.mon;
            unit = UNIT_AMPER;
        } else {
            // UR ->
            if (cursor.selected_channel->u.mon < cursor.selected_channel->i.mon) {
                // min(volt, curr)
                mon_value = cursor.selected_channel->u.mon;
                unit = UNIT_VOLT;
            } else {
                // or curr if equal
                mon_value = cursor.selected_channel->i.mon;
                unit = UNIT_AMPER;
            }
        }
        value = Value(mon_value, unit);
        if (selected_channel_last_state->mon_value != value) {
            selected_channel_last_state->mon_value = value;
            changed = true;
        }
    } else if (id == DATA_ID_VOLT) {
        float u_set = cursor.selected_channel->u.set;
        if (selected_channel_last_state->u_set != u_set) {
            selected_channel_last_state->u_set = u_set;
            changed = true;
        }
        value = Value(u_set, UNIT_VOLT);
    } else if (id == DATA_ID_CURR) {
        float i_set = cursor.selected_channel->i.set;
        if (selected_channel_last_state->i_set != cursor.selected_channel->i.set) {
            selected_channel_last_state->i_set = cursor.selected_channel->i.set;
            changed = true;
        }
        value = Value(i_set, UNIT_AMPER);
    } else if (id == DATA_ID_OVP) {
        uint8_t ovp;
        if (!cursor.selected_channel->prot_conf.flags.u_state) ovp = 1;
        else if (!cursor.selected_channel->ovp.flags.tripped) ovp = 2;
        else ovp = 3;
        if (selected_channel_last_state->flags.ovp != ovp) {
            selected_channel_last_state->flags.ovp = ovp;
            changed = true;
        }
        value = Value(ovp);
    } else if (id == DATA_ID_OCP) {
        uint8_t ocp;
        if (!cursor.selected_channel->prot_conf.flags.i_state) ocp = 1;
        else if (!cursor.selected_channel->ocp.flags.tripped) ocp = 2;
        else ocp = 3;
        if (selected_channel_last_state->flags.ocp != ocp) {
            selected_channel_last_state->flags.ocp = ocp;
            changed = true;
        }
        value = Value(ocp);
    } else if (id == DATA_ID_OPP) {
        uint8_t opp;
        if (!cursor.selected_channel->prot_conf.flags.p_state) opp = 1;
        else if (!cursor.selected_channel->opp.flags.tripped) opp = 2;
        else opp = 3;
        if (selected_channel_last_state->flags.opp != opp) {
            selected_channel_last_state->flags.opp = opp;
            changed = true;
        }
        value = Value(opp);
    } else if (id == DATA_ID_OTP) {
        uint8_t otp;
        if (!temperature::prot_conf[temp_sensor::MAIN].state) otp = 1;
        else if (!temperature::isSensorTripped(temp_sensor::MAIN)) otp = 2;
        else otp = 3;
        if (selected_channel_last_state->flags.otp != otp) {
            selected_channel_last_state->flags.otp = otp;
            changed = true;
        }
        value = Value(otp);
    } else if (id == DATA_ID_DP) {
        uint8_t dp = cursor.selected_channel->flags.dp_on ? 1 : 2;
        if (selected_channel_last_state->flags.dp != dp) {
            selected_channel_last_state->flags.dp = dp;
            changed = true;
        }
        value = Value(dp);
    }

    return value;
}

Value getMin(uint16_t id) {
    Value value;
    if (id == DATA_ID_VOLT) {
        value = Value(cursor.selected_channel->U_MIN, UNIT_VOLT);
    } else if (id == DATA_ID_CURR) {
        value = Value(cursor.selected_channel->I_MIN, UNIT_AMPER);
    }
    return value;
}

Value getMax(uint16_t id) {
    Value value;
    if (id == DATA_ID_VOLT) {
        value = Value(cursor.selected_channel->U_MAX, UNIT_VOLT);
    } else if (id == DATA_ID_CURR) {
        value = Value(cursor.selected_channel->I_MAX, UNIT_AMPER);
    }
    return value;
}

void set(uint16_t id, Value value) {
    if (id == DATA_ID_VOLT) {
        cursor.selected_channel->setVoltage(value.getFloat());
    } else if (id == DATA_ID_CURR) {
        cursor.selected_channel->setCurrent(value.getFloat());
    }
}

Cursor getCursor() {
    return cursor;
}

void setCursor(Cursor cursor_) {
    cursor = cursor_;
}

}
}
}
} // namespace eez::psu::ui::data
