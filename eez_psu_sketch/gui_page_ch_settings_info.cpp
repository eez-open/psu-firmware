/*
 * EEZ PSU Firmware
 * Copyright (C) 2016-present, Envox d.o.o.
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

#include "temperature.h"

#include "gui_data_snapshot.h"
#include "gui_page_ch_settings_info.h"

namespace eez {
namespace psu {
namespace gui {

void ChSettingsInfoPage::takeSnapshot(data::Snapshot *snapshot) {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
	channelSnapshot.flags.temperatureStatus = 2;
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4
	data::ChannelSnapshot &channelSnapshot = snapshot->channelSnapshots[g_channel->index - 1];

	temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::CH1 + g_channel->index - 1];
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

	channelSnapshot.onTimeTotal = g_channel->onTimeCounter.getTotalTime();
	channelSnapshot.onTimeLast = g_channel->onTimeCounter.getLastTime();
}

data::Value ChSettingsInfoPage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	data::ChannelSnapshot &channelSnapshot = snapshot->channelSnapshots[g_channel->index - 1];

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

	return data::Value();
}

}
}
} // namespace eez::psu::gui
