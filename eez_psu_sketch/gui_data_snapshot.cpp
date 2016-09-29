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
#include "calibration.h"
#include "temperature.h"
#include "persist_conf.h"
#include "gui_data_snapshot.h"
#include "gui_internal.h"
#include "gui_keypad.h"
#include "gui_calibration.h"

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
        char *p = Channel::getChannelsInfoShort(model_info);
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

    for (int i = 0; i < CH_MAX; ++i) {
		if (i >= CH_NUM) {
			channelSnapshots[i].flags.status = 0;
			continue;
		}

		Channel &channel = Channel::get(i);

        channelSnapshots[i].flags.status = channel.isOk() ? 1 : 2;

        channelSnapshots[i].flags.state = channel.isOutputEnabled() ? 1 : 0;

        if (timeout) {
            char *mode_str = channel.getCvModeStr();
            channelSnapshots[i].flags.mode = 0;
            float uMon = channel.u.mon;
            float iMon = channel.i.mon;
            if (strcmp(mode_str, "CC") == 0) {
                channelSnapshots[i].mon_value = Value(uMon, VALUE_TYPE_FLOAT_VOLT);
            } else if (strcmp(mode_str, "CV") == 0) {
                channelSnapshots[i].mon_value = Value(iMon, VALUE_TYPE_FLOAT_AMPER);
            } else {
                channelSnapshots[i].flags.mode = 1;
                if (uMon < iMon) {
                    channelSnapshots[i].mon_value = Value(uMon, VALUE_TYPE_FLOAT_VOLT);
                } else {
                    channelSnapshots[i].mon_value = Value(iMon, VALUE_TYPE_FLOAT_AMPER);
                }
            }

			channelSnapshots[i].p_mon = util::multiply(channel.u.mon, channel.i.mon, CHANNEL_VALUE_PRECISION);
        }

        channelSnapshots[i].u_set = channel.u.set;
		channelSnapshots[i].u_mon = channel.u.mon;
		channelSnapshots[i].u_monDac = channel.u.mon_dac;
		channelSnapshots[i].u_limit = channel.getVoltageLimit();
        channelSnapshots[i].i_set = channel.i.set;
		channelSnapshots[i].i_mon = channel.i.mon;
		channelSnapshots[i].i_monDac = channel.i.mon_dac;
		channelSnapshots[i].i_limit = channel.getCurrentLimit();

		channelSnapshots[i].flags.lrip = channel.flags.lrippleEnabled ? 1 : 0;
		channelSnapshots[i].flags.rprog = channel.flags.rprogEnabled ? 1 : 0;

		if (!channel.prot_conf.flags.i_state) channelSnapshots[i].flags.ocp = 0;
        else if (!channel.ocp.flags.tripped) channelSnapshots[i].flags.ocp = 1;
        else channelSnapshots[i].flags.ocp = 2;

		if (!channel.prot_conf.flags.u_state) channelSnapshots[i].flags.ovp = 0;
        else if (!channel.ovp.flags.tripped) channelSnapshots[i].flags.ovp = 1;
        else channelSnapshots[i].flags.ovp = 2;
        
        if (!channel.prot_conf.flags.p_state) channelSnapshots[i].flags.opp = 0;
        else if (!channel.opp.flags.tripped) channelSnapshots[i].flags.opp = 1;
        else channelSnapshots[i].flags.opp = 2;

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
		channelSnapshots[i].flags.otp_ch = 0;
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
		temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::CH1 + i];
        if (!tempSensor.isInstalled() || !tempSensor.isTestOK() || !tempSensor.prot_conf.state) channelSnapshots[i].flags.otp_ch = 0;
        else if (!tempSensor.isTripped()) channelSnapshots[i].flags.otp_ch = 1;
        else channelSnapshots[i].flags.otp_ch = 2;
#endif

		channelSnapshots[i].flags.dp = channel.flags.dpOn ? 1 : 0;
		
		channelSnapshots[i].flags.cal_enabled = channel.isCalibrationEnabled() ? 1 : 0;
    }

	flags.channelDisplayedValues = persist_conf::dev_conf.flags.channelDisplayedValues;

	temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::MAIN];
	if (!tempSensor.prot_conf.state) flags.otp = 0;
    else if (!tempSensor.isTripped()) flags.otp = 1;
    else flags.otp = 2;

	keypadSnapshot.takeSnapshot(this);
    editModeSnapshot.takeSnapshot(this);

    alertMessage = g_alertMessage;

	Page *activePage = getActivePage();
	if (activePage) {
		activePage->takeSnapshot(this);
	}
}

Value Snapshot::get(const Cursor &cursor, uint8_t id) {
	if (id == DATA_ID_CHANNEL_DISPLAYED_VALUES) {
		return Value(flags.channelDisplayedValues);
	}

	if (cursor.i >= 0 || g_channel != 0) {
		int iChannel = cursor.i >= 0 ? cursor.i : g_channel->index - 1;

		if (id == DATA_ID_CHANNEL_STATUS) {
			return Value(channelSnapshots[iChannel].flags.status);
		}

		if (channelSnapshots[iChannel].flags.status == 1) {
			if (id == DATA_ID_CHANNEL_OUTPUT_STATE) {
				return Value(channelSnapshots[iChannel].flags.state);
			}
		
			if (id == DATA_ID_CHANNEL_OUTPUT_MODE) {
				return Value(channelSnapshots[iChannel].flags.mode);
			}
		
			if (id == DATA_ID_CHANNEL_MON_VALUE) {
				return channelSnapshots[iChannel].mon_value;
			}
		
			if (id == DATA_ID_CHANNEL_U_SET) {
				return Value(channelSnapshots[iChannel].u_set, VALUE_TYPE_FLOAT_VOLT);
			}
		
			if (id == DATA_ID_CHANNEL_U_MON) {
				return Value(channelSnapshots[iChannel].u_mon, VALUE_TYPE_FLOAT_VOLT);
			}

			if (id == DATA_ID_CHANNEL_U_MON_DAC) {
				return Value(channelSnapshots[iChannel].u_monDac, VALUE_TYPE_FLOAT_VOLT);
			}

			if (id == DATA_ID_CHANNEL_U_LIMIT) {
				return Value(channelSnapshots[iChannel].u_limit, VALUE_TYPE_FLOAT_VOLT);
			}

			if (id == DATA_ID_CHANNEL_I_SET) {
				return Value(channelSnapshots[iChannel].i_set, VALUE_TYPE_FLOAT_AMPER);
			}
		
			if (id == DATA_ID_CHANNEL_I_MON) {
				return Value(channelSnapshots[iChannel].i_mon, VALUE_TYPE_FLOAT_AMPER);
			}

			if (id == DATA_ID_CHANNEL_I_MON_DAC) {
				return Value(channelSnapshots[iChannel].i_monDac, VALUE_TYPE_FLOAT_AMPER);
			}

			if (id == DATA_ID_CHANNEL_I_LIMIT) {
				return Value(channelSnapshots[iChannel].i_limit, VALUE_TYPE_FLOAT_VOLT);
			}

			if (id == DATA_ID_CHANNEL_P_MON) {
				return Value(channelSnapshots[iChannel].p_mon, VALUE_TYPE_FLOAT_WATT);
			}

			if (id == DATA_ID_LRIP) {
				return Value(channelSnapshots[iChannel].flags.lrip);
			}

			if (id == DATA_ID_CHANNEL_RPROG_STATUS) {
				return Value(channelSnapshots[iChannel].flags.rprog);
			}

			if (id == DATA_ID_OVP) {
				return Value(channelSnapshots[iChannel].flags.ovp);
			}
		
			if (id == DATA_ID_OCP) {
				return Value(channelSnapshots[iChannel].flags.ocp);
			}
		
			if (id == DATA_ID_OPP) {
				return Value(channelSnapshots[iChannel].flags.opp);
			}
		
			if (id == DATA_ID_OTP_CH) {
				return Value(channelSnapshots[iChannel].flags.otp_ch);
			}
		
			if (id == DATA_ID_DP) {
				return Value(channelSnapshots[iChannel].flags.dp);
			}
		
			if (id == DATA_ID_CHANNEL_LABEL) {
				return data::Value(iChannel + 1, data::VALUE_TYPE_CHANNEL_LABEL);
			}
		
			if (id == DATA_ID_CHANNEL_SHORT_LABEL) {
				return data::Value(iChannel + 1, data::VALUE_TYPE_CHANNEL_SHORT_LABEL);
			}
		}
	}
	
	if (id == DATA_ID_OTP) {
        return Value(flags.otp);
    }
	
	if (id == DATA_ID_ALERT_MESSAGE) {
        return alertMessage;
    }
	
	if (id == DATA_ID_MODEL_INFO) {
        return Value(getModelInfo());
    }
	
	if (id == DATA_ID_FIRMWARE_INFO) {
        return Value(getFirmwareInfo());
    }

	Page *page = getActivePage();
	if (page) {
		Value value = page->getData(cursor, id, this);
		if (value.getType() != VALUE_TYPE_NONE) {
			return value;
		}
	}

	Value value = keypadSnapshot.get(id);
    if (value.getType() != VALUE_TYPE_NONE) {
        return value;
    }

    value = editModeSnapshot.getData(id);
    if (value.getType() != VALUE_TYPE_NONE) {
        return value;
    }

	value = gui::calibration::getData(cursor, id, this);
    if (value.getType() != VALUE_TYPE_NONE) {
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
