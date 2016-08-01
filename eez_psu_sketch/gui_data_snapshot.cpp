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
#include "gui_data_snapshot.h"
#include "gui_view.h"
#include "gui_document.h"
#include "temperature.h"

#define CONF_GUI_REFRESH_EVERY_MS 250

namespace eez {
namespace psu {
namespace gui {
namespace data {

/*
We are auto generating model name from the channels definition:

<cnt>/<volt>/<curr>[-<cnt2>/<volt2>/<curr2>] (<platform>)

Where is:

<cnt>      - number of the equivalent channels
<volt>     - max. voltage
<curr>     - max. curr
<platform> - Mega, Due, Simulator or Unknown
*/
const char *getModelInfo() {
    static char model_info[CH_NUM * (sizeof("XX V / XX A") - 1) + (CH_NUM - 1) * (sizeof(" - ") - 1) + 1];

    if (*model_info == 0) {
        char *p = model_info;

        bool ch_used[CH_NUM];

        for (int i = 0; i < CH_NUM; ++i) {
            ch_used[i] = false;
        }

        bool first_channel = true;

        for (int i = 0; i < CH_NUM; ++i) {
            if (!ch_used[i]) {
                int count = 1;
                for (int j = i + 1; j < CH_NUM; ++j) {
                    if (Channel::get(i).U_MAX == Channel::get(j).U_MAX && Channel::get(i).I_MAX == Channel::get(j).I_MAX) {
                        ch_used[j] = true;
                        ++count;
                    }
                }

                if (first_channel) {
                    first_channel = false;
                }
                else {
                    *p++ += ' ';
                    *p++ += '-';
                    *p++ += ' ';
                }

                p += sprintf_P(p, PSTR("%d V / %d A"), (int)floor(Channel::get(i).U_MAX), (int)floor(Channel::get(i).I_MAX));
            }
        }

        *p = 0;
    }

    return model_info;
}

const char *getFirmwareInfo() {
    static const char FIRMWARE_LABEL[] PROGMEM = "Firmware: ";
    static char firmware_info[sizeof(FIRMWARE_LABEL) - 1 + sizeof(FIRMWARE) - 1 + 1];

    if (*firmware_info == 0) {
        strcat_P(firmware_info, FIRMWARE_LABEL);
        strcat_P(firmware_info, PSTR(FIRMWARE));
    }

    return firmware_info;
}


Snapshot currentSnapshot;
Snapshot previousSnapshot;

void Snapshot::takeSnapshot() {
    bool timeout = false;
    unsigned long currentTime = micros();
    if (currentTime - lastSnapshotTime >= CONF_GUI_REFRESH_EVERY_MS * 1000UL) {
        timeout = true;
        lastSnapshotTime = currentTime;
    }

    for (int i = 0; i < CH_NUM; ++i) {
        Channel &channel = Channel::get(i);

        channelSnapshots[i].flags.ok = channel.isOk() ? 1 : 0;

        channelSnapshots[i].flags.state = channel.isOutputEnabled() ? 1 : 0;

        if (timeout) {
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
        }

        channelSnapshots[i].u_set = channel.u.set;
        channelSnapshots[i].i_set = channel.i.set;

		channelSnapshots[i].flags.lrip = channel.flags.lripple_enabled ? 1 : 0;

		if (!channel.prot_conf.flags.i_state) channelSnapshots[i].flags.ocp = 0;
        else if (!channel.ocp.flags.tripped) channelSnapshots[i].flags.ocp = 1;
        else channelSnapshots[i].flags.ocp = 2;

		if (!channel.prot_conf.flags.u_state) channelSnapshots[i].flags.ovp = 0;
        else if (!channel.ovp.flags.tripped) channelSnapshots[i].flags.ovp = 1;
        else channelSnapshots[i].flags.ovp = 2;
        
        if (!channel.prot_conf.flags.p_state) channelSnapshots[i].flags.opp = 0;
        else if (!channel.opp.flags.tripped) channelSnapshots[i].flags.opp = 1;
        else channelSnapshots[i].flags.opp = 2;

		temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::CH1 + i];
        if (!tempSensor.isInstalled() || !tempSensor.isTestOK() || !tempSensor.prot_conf.state) channelSnapshots[i].flags.otp_ch = 0;
        else if (!tempSensor.isTripped()) channelSnapshots[i].flags.otp_ch = 1;
        else channelSnapshots[i].flags.otp_ch = 2;

		channelSnapshots[i].flags.dp = channel.flags.dp_on ? 1 : 0;
    }

	temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::MAIN];
	if (!tempSensor.prot_conf.state) flags.otp = 0;
    else if (!tempSensor.isTripped()) flags.otp = 1;
    else flags.otp = 2;

    editModeSnapshot.takeSnapshot();

    alertMessage = g_alertMessage;
}

Value Snapshot::get(const Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_CHANNEL_OK) {
        return Value(channelSnapshots[cursor.iChannel].flags.ok);
    } else if (id == DATA_ID_OUTPUT_STATE) {
        return Value(channelSnapshots[cursor.iChannel].flags.state);
    } else if (id == DATA_ID_OUTPUT_MODE) {
        return Value(channelSnapshots[cursor.iChannel].flags.mode);
    } else if (id == DATA_ID_MON_VALUE) {
        return channelSnapshots[cursor.iChannel].mon_value;
    } else if (id == DATA_ID_VOLT) {
        return Value(channelSnapshots[cursor.iChannel].u_set, UNIT_VOLT);
    } else if (id == DATA_ID_CURR) {
        return Value(channelSnapshots[cursor.iChannel].i_set, UNIT_AMPER);
    } else if (id == DATA_ID_LRIP) {
        return Value(channelSnapshots[cursor.iChannel].flags.lrip);
    } else if (id == DATA_ID_OVP) {
        return Value(channelSnapshots[cursor.iChannel].flags.ovp);
    } else if (id == DATA_ID_OCP) {
        return Value(channelSnapshots[cursor.iChannel].flags.ocp);
    } else if (id == DATA_ID_OPP) {
        return Value(channelSnapshots[cursor.iChannel].flags.opp);
    } else if (id == DATA_ID_OTP) {
        return Value(flags.otp);
    } else if (id == DATA_ID_OTP_CH) {
        return Value(channelSnapshots[cursor.iChannel].flags.otp_ch);
    } else if (id == DATA_ID_DP) {
        return Value(channelSnapshots[cursor.iChannel].flags.dp);
    } else if (id == DATA_ID_ALERT_MESSAGE) {
        return alertMessage;
    } else if (id == DATA_ID_MODEL_INFO) {
        return Value(getModelInfo());
    } else if (id == DATA_ID_FIRMWARE_INFO) {
        return Value(getFirmwareInfo());
    }
    
    Value value = editModeSnapshot.get(id);
    if (value.getUnit() != UNIT_NONE) {
        return value;
    }

    return Value();
}

bool Snapshot::isBlinking(const Cursor &cursor, uint8_t id) {
    bool result;
    if (editModeSnapshot.isBlinking(*this, id, result)) {
        return result;
    }

    return false;
}

}
}
}
} // namespace eez::psu::ui::data
