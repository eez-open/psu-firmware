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
#include "channel_dispatcher.h"
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
	    data::ChannelSnapshot &channelSnapshot = channelSnapshots[i];

        if (i >= CH_NUM) {
			channelSnapshot.flags.status = 0;
			continue;
		}

		Channel &channel = Channel::get(i);

        channelSnapshot.flags.status = channel.isOk() ? 1 : 2;

        channelSnapshot.flags.state = channel.isOutputEnabled() ? 1 : 0;

        if (timeout) {
            char *mode_str = channel.getCvModeStr();
            channelSnapshot.flags.mode = 0;
            float uMon = channel_dispatcher::getUMon(channel);
            float iMon = channel_dispatcher::getIMon(channel);
            if (strcmp(mode_str, "CC") == 0) {
                channelSnapshot.mon_value = Value(uMon, VALUE_TYPE_FLOAT_VOLT);
            } else if (strcmp(mode_str, "CV") == 0) {
                channelSnapshot.mon_value = Value(iMon, VALUE_TYPE_FLOAT_AMPER);
            } else {
                channelSnapshot.flags.mode = 1;
                if (uMon < iMon) {
                    channelSnapshot.mon_value = Value(uMon, VALUE_TYPE_FLOAT_VOLT);
                } else {
                    channelSnapshot.mon_value = Value(iMon, VALUE_TYPE_FLOAT_AMPER);
                }
            }

			channelSnapshot.p_mon = util::multiply(channel_dispatcher::getUMon(channel), channel_dispatcher::getIMon(channel), CHANNEL_VALUE_PRECISION);
        }

        channelSnapshot.u_set = channel_dispatcher::getUSet(channel);
		channelSnapshot.u_mon = channel_dispatcher::getUMon(channel);
		channelSnapshot.u_monDac = channel_dispatcher::getUMonDac(channel);
		channelSnapshot.u_limit = channel_dispatcher::getULimit(channel);
        channelSnapshot.i_set = channel_dispatcher::getISet(channel);
		channelSnapshot.i_mon = channel_dispatcher::getIMon(channel);
		channelSnapshot.i_monDac = channel_dispatcher::getIMonDac(channel);
		channelSnapshot.i_limit = channel_dispatcher::getILimit(channel);

		channelSnapshot.flags.lrip = channel.flags.lrippleEnabled ? 1 : 0;
		channelSnapshot.flags.rprog = channel.flags.rprogEnabled ? 1 : 0;

		if (!channel.prot_conf.flags.i_state) channelSnapshot.flags.ocp = 0;
        else if (!channel.ocp.flags.tripped) channelSnapshot.flags.ocp = 1;
        else channelSnapshot.flags.ocp = 2;

		if (!channel.prot_conf.flags.u_state) channelSnapshot.flags.ovp = 0;
        else if (!channel.ovp.flags.tripped) channelSnapshot.flags.ovp = 1;
        else channelSnapshot.flags.ovp = 2;
        
        if (!channel.prot_conf.flags.p_state) channelSnapshot.flags.opp = 0;
        else if (!channel.opp.flags.tripped) channelSnapshot.flags.opp = 1;
        else channelSnapshot.flags.opp = 2;

#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
		channelSnapshot.flags.otp_ch = 0;
        channelSnapshot.flags.temperatureStatus = 2;
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
		temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::CH1 + i];
        if (!tempSensor.isInstalled() || !tempSensor.isTestOK() || !tempSensor.prot_conf.state) channelSnapshot.flags.otp_ch = 0;
        else if (!tempSensor.isTripped()) channelSnapshot.flags.otp_ch = 1;
        else channelSnapshot.flags.otp_ch = 2;

	    if (tempSensor.isInstalled()) {
		    if (tempSensor.isTestOK()) {
			    channelSnapshot.flags.temperatureStatus = 1;
			    channelSnapshot.temperature = tempSensor.temperature;
		    } else {
			    channelSnapshot.flags.temperatureStatus = 0;
		    }
	    } else {
		    channelSnapshot.flags.temperatureStatus = 2;
	    }
#endif

		channelSnapshot.flags.dp = channel.flags.dpOn ? 1 : 0;
		channelSnapshot.flags.cal_enabled = channel.isCalibrationEnabled() ? 1 : 0;

        channelSnapshot.onTimeTotal = channel.onTimeCounter.getTotalTime();
	    channelSnapshot.onTimeLast = channel.onTimeCounter.getLastTime();
    }

	flags.channelDisplayedValues = persist_conf::dev_conf.flags.channelDisplayedValues;
    flags.channelCouplingMode = channel_dispatcher::getType();

    if (channel_dispatcher::isSeries()) {
        flags.isVoltageBalanced = Channel::get(0).isVoltageBalanced() || Channel::get(1).isVoltageBalanced() ? 1 : 0;
    } else {
        flags.isVoltageBalanced = 0;
    }

    if (channel_dispatcher::isParallel()) {
        flags.isCurrentBalanced = Channel::get(0).isCurrentBalanced() || Channel::get(1).isCurrentBalanced() ? 1 : 0;
    } else {
        flags.isCurrentBalanced = 0;
    }

    temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::MAIN];
	if (!tempSensor.prot_conf.state) flags.otp = 0;
    else if (!tempSensor.isTripped()) flags.otp = 1;
    else flags.otp = 2;

	keypadSnapshot.takeSnapshot(this);
    editModeSnapshot.takeSnapshot(this);

    alertMessage = g_alertMessage;
    alertMessage2 = g_alertMessage2;

	Page *activePage = getActivePage();
	if (activePage) {
		activePage->takeSnapshot(this);
	}
}

Value Snapshot::get(const Cursor &cursor, uint8_t id) {
	if (id == DATA_ID_CHANNEL_DISPLAYED_VALUES) {
		return Value(flags.channelDisplayedValues);
	}

	if (id == DATA_ID_CHANNEL_COUPLING_MODE) {
		return data::Value(flags.channelCouplingMode);
	}

	if (id == DATA_ID_CHANNEL_IS_COUPLED) {
		return data::Value(flags.channelCouplingMode == channel_dispatcher::TYPE_SERIES || flags.channelCouplingMode == channel_dispatcher::TYPE_PARALLEL ? 1 : 0);
	}

    if (id == DATA_ID_CHANNEL_IS_TRACKED) {
		return data::Value(flags.channelCouplingMode == channel_dispatcher::TYPE_TRACKED ? 1 : 0);
	}

	if (id == DATA_ID_CHANNEL_IS_COUPLED_OR_TRACKED) {
		return data::Value(flags.channelCouplingMode != channel_dispatcher::TYPE_NONE ? 1 : 0);
	}

    int iChannel;
    if (cursor.i >= 0) {
		iChannel = cursor.i;
    } else if (g_channel) {
        iChannel = g_channel->index - 1;
    } else {
        iChannel = 0;
    }

    data::ChannelSnapshot &channelSnapshot = channelSnapshots[iChannel];

	if (id == DATA_ID_CHANNEL_STATUS) {
		return Value(channelSnapshot.flags.status);
	}

	if (channelSnapshot.flags.status == 1) {
		if (id == DATA_ID_CHANNEL_OUTPUT_STATE) {
			return Value(channelSnapshot.flags.state);
		}
		
		if (id == DATA_ID_CHANNEL_OUTPUT_MODE) {
			return Value(channelSnapshot.flags.mode);
		}
		
		if (id == DATA_ID_CHANNEL_MON_VALUE) {
			return channelSnapshot.mon_value;
		}
		
		if (id == DATA_ID_CHANNEL_U_SET) {
			return Value(channelSnapshot.u_set, VALUE_TYPE_FLOAT_VOLT);
		}
		
		if (id == DATA_ID_CHANNEL_U_MON) {
			return Value(channelSnapshot.u_mon, VALUE_TYPE_FLOAT_VOLT);
		}

		if (id == DATA_ID_CHANNEL_U_MON_DAC) {
			return Value(channelSnapshot.u_monDac, VALUE_TYPE_FLOAT_VOLT);
		}

		if (id == DATA_ID_CHANNEL_U_LIMIT) {
			return Value(channelSnapshot.u_limit, VALUE_TYPE_FLOAT_VOLT);
		}

		if (id == DATA_ID_CHANNEL_I_SET) {
			return Value(channelSnapshot.i_set, VALUE_TYPE_FLOAT_AMPER);
		}
		
		if (id == DATA_ID_CHANNEL_I_MON) {
			return Value(channelSnapshot.i_mon, VALUE_TYPE_FLOAT_AMPER);
		}

		if (id == DATA_ID_CHANNEL_I_MON_DAC) {
			return Value(channelSnapshot.i_monDac, VALUE_TYPE_FLOAT_AMPER);
		}

		if (id == DATA_ID_CHANNEL_I_LIMIT) {
			return Value(channelSnapshot.i_limit, VALUE_TYPE_FLOAT_VOLT);
		}

		if (id == DATA_ID_CHANNEL_P_MON) {
			return Value(channelSnapshot.p_mon, VALUE_TYPE_FLOAT_WATT);
		}

		if (id == DATA_ID_LRIP) {
			return Value(channelSnapshot.flags.lrip);
		}

		if (id == DATA_ID_CHANNEL_RPROG_STATUS) {
			return Value(channelSnapshot.flags.rprog);
		}

		if (id == DATA_ID_OVP) {
			return Value(channelSnapshot.flags.ovp);
		}
		
		if (id == DATA_ID_OCP) {
			return Value(channelSnapshot.flags.ocp);
		}
		
		if (id == DATA_ID_OPP) {
			return Value(channelSnapshot.flags.opp);
		}
		
		if (id == DATA_ID_OTP) {
			return Value(channelSnapshot.flags.otp_ch);
		}
		
		if (id == DATA_ID_CHANNEL_LABEL) {
			return data::Value(iChannel + 1, data::VALUE_TYPE_CHANNEL_LABEL);
		}
		
		if (id == DATA_ID_CHANNEL_SHORT_LABEL) {
			return data::Value(iChannel + 1, data::VALUE_TYPE_CHANNEL_SHORT_LABEL);
		}

        if (id == DATA_ID_CHANNEL_TEMP_STATUS) {
	    	return data::Value(channelSnapshot.flags.temperatureStatus);
	    }

	    if (id == DATA_ID_CHANNEL_TEMP && channelSnapshot.flags.temperatureStatus == 1) {
		    return data::Value(channelSnapshot.temperature, data::VALUE_TYPE_FLOAT_CELSIUS);
	    }

	    if (id == DATA_ID_CHANNEL_ON_TIME_TOTAL) {
		    return data::Value(channelSnapshot.onTimeTotal, data::VALUE_TYPE_ON_TIME_COUNTER);
	    }

	    if (id == DATA_ID_CHANNEL_ON_TIME_LAST) {
		    return data::Value(channelSnapshot.onTimeLast, data::VALUE_TYPE_ON_TIME_COUNTER);
	    }
    }
	
    if (id == DATA_ID_CHANNEL_IS_VOLTAGE_BALANCED) {
        return data::Value(flags.isVoltageBalanced);
    }

    if (id == DATA_ID_CHANNEL_IS_CURRENT_BALANCED) {
        return data::Value(flags.isCurrentBalanced);
    }

    if (id == DATA_ID_OTP) {
        return Value(flags.otp);
    }
	
	if (id == DATA_ID_ALERT_MESSAGE) {
        return alertMessage;
    }
	
	if (id == DATA_ID_ALERT_MESSAGE_2) {
        return alertMessage2;
    }

    if (id == DATA_ID_MODEL_INFO) {
        return Value(getModelInfo());
    }
	
	if (id == DATA_ID_FIRMWARE_INFO) {
        return Value(getFirmwareInfo());
    }

    if (id == DATA_ID_SYS_ETHERNET_INSTALLED) {
        return data::Value(OPTION_ETHERNET);
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
