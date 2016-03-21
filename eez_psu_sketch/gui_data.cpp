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
#include "gui_internal.h"
#include "gui_keypad.h"
#include "channel.h"

namespace eez {
namespace psu {
namespace gui {
namespace data {

static Value alertMessage;

////////////////////////////////////////////////////////////////////////////////

void Value::toText(char *text, int count) {
    text[0] = 0;

    util::strcatFloat(text, float_);

    switch (unit_) {
    case UNIT_VOLT:
        strcat(text, " V");
        break;
    
    case UNIT_AMPER:
        strcat(text, " A");
        break;

    case UNIT_CONST_STR:
        strncpy_P(text, const_str_, count - 1);
        text[count - 1] = 0;
        break;

    case UNIT_STR:
        strncpy(text, str_, count - 1);
        text[count - 1] = 0;
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////

void Snapshot::takeSnapshot() {
    for (int i = 0; i < CH_NUM; ++i) {
        Channel &channel = Channel::get(i);

        channelStates[i].flags.state = channel.isOutputEnabled() ? 1 : 0;

        char *mode_str = channel.getCvModeStr();
        channelStates[i].flags.mode = 0;
        float uMon = channel.u.mon;
        float iMon = channel.i.mon;
        if (strcmp(mode_str, "CC") == 0) {
            channelStates[i].mon_value = Value(uMon, UNIT_VOLT);
        } else if (strcmp(mode_str, "CV") == 0) {
            channelStates[i].mon_value = Value(iMon, UNIT_AMPER);
        } else {
            channelStates[i].flags.mode = 1;
            if (uMon < iMon) {
                channelStates[i].mon_value = Value(uMon, UNIT_VOLT);
            } else {
                channelStates[i].mon_value = Value(iMon, UNIT_AMPER);
            }
        }

        channelStates[i].u_set = channel.u.set;
        channelStates[i].i_set = channel.i.set;

        if (!channel.prot_conf.flags.u_state) channelStates[i].flags.ovp = 1;
        else if (!channel.ovp.flags.tripped) channelStates[i].flags.ovp = 2;
        else channelStates[i].flags.ovp = 3;
        
        if (!channel.prot_conf.flags.i_state) channelStates[i].flags.ocp = 1;
        else if (!channel.ocp.flags.tripped) channelStates[i].flags.ocp = 2;
        else channelStates[i].flags.ocp = 3;

        if (!channel.prot_conf.flags.p_state) channelStates[i].flags.opp = 1;
        else if (!channel.opp.flags.tripped) channelStates[i].flags.opp = 2;
        else channelStates[i].flags.opp = 3;

        if (!temperature::prot_conf[temp_sensor::MAIN].state) channelStates[i].flags.otp = 1;
        else if (!temperature::isSensorTripped(temp_sensor::MAIN)) channelStates[i].flags.otp = 2;
        else channelStates[i].flags.otp = 3;

        channelStates[i].flags.dp = channel.flags.dp_on ? 1 : 2;
    }

    alertMessage = alertMessage;

    if (edit_data_cursor) {
        editValue = edit_value;

        Channel &channel = Channel::get(edit_data_cursor.iChannel);
        if (edit_data_id == DATA_ID_VOLT) {
            sprintf_P(editInfo, PSTR("Set Ch%d voltage [%d-%d V]"), channel.index, (int)channel.U_MIN, (int)channel.U_MAX);
        } else {
            sprintf_P(editInfo, PSTR("Set Ch%d current [%d-%d A]"), channel.index, (int)channel.I_MIN, (int)channel.I_MAX);
        }
    
        switch (keypad::get_edit_unit()) {
        case UNIT_VOLT: editUnit = Value::ConstStr("mV"); break;
        case UNIT_MILLI_VOLT: editUnit = Value::ConstStr("V"); break;
        case UNIT_AMPER: editUnit = Value::ConstStr("mA"); break;
        default: editUnit = Value::ConstStr("A");
        }

        this->editInteractiveMode = isEditInteractiveMode ? 0 : 1;
    }

    keypad::get_text(keypadText);
}

Value Snapshot::get(const Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_OUTPUT_STATE) {
        return Value(channelStates[cursor.iChannel].flags.state);
    } else if (id == DATA_ID_OUTPUT_MODE) {
        return Value(channelStates[cursor.iChannel].flags.mode);
    } else if (id == DATA_ID_MON_VALUE) {
        return channelStates[cursor.iChannel].mon_value;
    } else if (id == DATA_ID_VOLT) {
        return Value(channelStates[cursor.iChannel].u_set, UNIT_VOLT);
    } else if (id == DATA_ID_CURR) {
        return Value(channelStates[cursor.iChannel].i_set, UNIT_AMPER);
    } else if (id == DATA_ID_OVP) {
        return Value(channelStates[cursor.iChannel].flags.ovp);
    } else if (id == DATA_ID_OCP) {
        return Value(channelStates[cursor.iChannel].flags.ocp);
    } else if (id == DATA_ID_OPP) {
        return Value(channelStates[cursor.iChannel].flags.opp);
    } else if (id == DATA_ID_OTP) {
        return Value(channelStates[cursor.iChannel].flags.otp);
    } else if (id == DATA_ID_DP) {
        return Value(channelStates[cursor.iChannel].flags.dp);
    } else if (id == DATA_ID_ALERT_MESSAGE) {
        return alertMessage;
    } else if (id == DATA_ID_EDIT_VALUE) {
       return editValue;
    } else if (id == DATA_ID_EDIT_UNIT) {
        return editUnit;
    } else if (id == DATA_ID_EDIT_INFO) {
        return editInfo;
    } else if (id == DATA_ID_EDIT_INTERACTIVE_MODE) {
        return editInteractiveMode;
    }
    return Value();
}

bool Snapshot::isBlinking(const Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_EDIT_VALUE) {
        return !isEditInteractiveMode && editValue != get(edit_data_cursor, edit_data_id);
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

int count(uint8_t id) {
    if (id == DATA_ID_CHANNELS) {
        return CH_NUM;
    }
    return 0;
}

void select(Cursor &cursor, uint8_t id, int index) {
    if (id == DATA_ID_CHANNELS) {
        cursor.iChannel = index;
    }
}

Value getMin(const Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_VOLT) {
        return Value(Channel::get(cursor.iChannel).U_MIN, UNIT_VOLT);
    } else if (id == DATA_ID_CURR) {
        return Value(Channel::get(cursor.iChannel).I_MIN, UNIT_AMPER);
    }
    return Value();
}

Value getMax(const Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_VOLT)
        return Value(Channel::get(cursor.iChannel).U_MAX, UNIT_VOLT);
    else if (id == DATA_ID_CURR)
        return Value(Channel::get(cursor.iChannel).I_MAX, UNIT_AMPER);
    return Value();
}

Unit getUnit(const Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_VOLT)
        return UNIT_VOLT;
    else if (id == DATA_ID_CURR)
        return UNIT_AMPER;
    return UNIT_NONE;
}

void set(const Cursor &cursor, uint8_t id, Value value) {
    if (id == DATA_ID_VOLT)
        Channel::get(cursor.iChannel).setVoltage(value.getFloat());
    else if (id == DATA_ID_CURR)
        Channel::get(cursor.iChannel).setCurrent(value.getFloat());
    else if (id == DATA_ID_ALERT_MESSAGE)
        alertMessage = value;
}

void toggle(uint8_t id) {
    if (id == DATA_ID_EDIT_INTERACTIVE_MODE) {
        isEditInteractiveMode = !isEditInteractiveMode;
        if (!isEditInteractiveMode) {
            edit_value_saved = edit_value;
        }
    }
}

void doAction(const Cursor &cursor, uint8_t id) {
    if (id == ACTION_ID_TOGGLE_CHANNEL) {
        Channel::get(cursor.iChannel).outputEnable(!Channel::get(cursor.iChannel).isOutputEnabled());
    }
}

}
}
}
} // namespace eez::psu::ui::data
