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
#include "gui_data_snapshot.h"

namespace eez {
namespace psu {
namespace gui {
namespace data {

Snapshot currentSnapshot;
Snapshot previousSnapshot;

void Snapshot::takeSnapshot() {
    for (int i = 0; i < CH_NUM; ++i) {
        Channel &channel = Channel::get(i);

        channelSnapshots[i].flags.state = channel.isOutputEnabled() ? 1 : 0;

        char *mode_str = channel.getCvModeStr();
        channelSnapshots[i].flags.mode = 0;
        float uMon = channel.u.mon;
        float iMon = channel.i.mon;
        if (strcmp(mode_str, "CC") == 0) {
            channelSnapshots[i].mon_value = Value(uMon, UNIT_VOLT);
        } else if (strcmp(mode_str, "CV") == 0) {
            channelSnapshots[i].mon_value = Value(iMon, UNIT_AMPER);
        } else {
            channelSnapshots[i].flags.mode = 1;
            if (uMon < iMon) {
                channelSnapshots[i].mon_value = Value(uMon, UNIT_VOLT);
            } else {
                channelSnapshots[i].mon_value = Value(iMon, UNIT_AMPER);
            }
        }

        channelSnapshots[i].u_set = channel.u.set;
        channelSnapshots[i].i_set = channel.i.set;

        if (!channel.prot_conf.flags.u_state) channelSnapshots[i].flags.ovp = 1;
        else if (!channel.ovp.flags.tripped) channelSnapshots[i].flags.ovp = 2;
        else channelSnapshots[i].flags.ovp = 3;
        
        if (!channel.prot_conf.flags.i_state) channelSnapshots[i].flags.ocp = 1;
        else if (!channel.ocp.flags.tripped) channelSnapshots[i].flags.ocp = 2;
        else channelSnapshots[i].flags.ocp = 3;

        if (!channel.prot_conf.flags.p_state) channelSnapshots[i].flags.opp = 1;
        else if (!channel.opp.flags.tripped) channelSnapshots[i].flags.opp = 2;
        else channelSnapshots[i].flags.opp = 3;

        if (!temperature::prot_conf[temp_sensor::MAIN].state) channelSnapshots[i].flags.otp = 1;
        else if (!temperature::isSensorTripped(temp_sensor::MAIN)) channelSnapshots[i].flags.otp = 2;
        else channelSnapshots[i].flags.otp = 3;

        channelSnapshots[i].flags.dp = channel.flags.dp_on ? 1 : 2;
    }

    editModeSnapshot.takeSnapshot();

    alertMessage = alertMessage;
}

Value Snapshot::get(const Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_OUTPUT_STATE) {
        return Value(channelSnapshots[cursor.iChannel].flags.state);
    } else if (id == DATA_ID_OUTPUT_MODE) {
        return Value(channelSnapshots[cursor.iChannel].flags.mode);
    } else if (id == DATA_ID_MON_VALUE) {
        return channelSnapshots[cursor.iChannel].mon_value;
    } else if (id == DATA_ID_VOLT) {
        return Value(channelSnapshots[cursor.iChannel].u_set, UNIT_VOLT);
    } else if (id == DATA_ID_CURR) {
        return Value(channelSnapshots[cursor.iChannel].i_set, UNIT_AMPER);
    } else if (id == DATA_ID_OVP) {
        return Value(channelSnapshots[cursor.iChannel].flags.ovp);
    } else if (id == DATA_ID_OCP) {
        return Value(channelSnapshots[cursor.iChannel].flags.ocp);
    } else if (id == DATA_ID_OPP) {
        return Value(channelSnapshots[cursor.iChannel].flags.opp);
    } else if (id == DATA_ID_OTP) {
        return Value(channelSnapshots[cursor.iChannel].flags.otp);
    } else if (id == DATA_ID_DP) {
        return Value(channelSnapshots[cursor.iChannel].flags.dp);
    } else if (id == DATA_ID_ALERT_MESSAGE) {
        return alertMessage;
    } 
    
    Value value = editModeSnapshot.get(id);
    if (value.getUnit() != UNIT_NONE) {
        return value;
    }

    return Value();
}

bool Snapshot::isBlinking(const Cursor &cursor, uint8_t id) {
    bool result;
    if (editModeSnapshot.isBlinking(id, result)) {
        return result;
    }

    return false;
}

}
}
}
} // namespace eez::psu::ui::data
