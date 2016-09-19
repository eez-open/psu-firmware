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

#include "fan.h"
#include "temperature.h"

#include "gui_data_snapshot.h"
#include "gui_page_sys_info.h"

namespace eez {
namespace psu {
namespace gui {

void SysInfoPage::takeSnapshot(data::Snapshot *snapshot) {
	temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::MAIN];
	if (tempSensor.isInstalled()) {
		if (tempSensor.isTestOK()) {
			snapshot->flags.mainTemperatureStatus = 1;
			snapshot->mainTemperature = tempSensor.temperature;
		} else {
			snapshot->flags.mainTemperatureStatus = 0;
		}
	} else {
		snapshot->flags.mainTemperatureStatus = 2;
	}

	if (fan::test_result == TEST_FAILED || fan::test_result == TEST_WARNING) {
		snapshot->flags.fanStatus = 0;
	} else if (fan::test_result == TEST_OK) {
		snapshot->flags.fanStatus = 1;
		snapshot->fanSpeed = (float)fan::g_rpm;
	} else {
		snapshot->flags.fanStatus = 2;
	}

	snapshot->onTimeTotal = g_powerOnTimeCounter.getTotalTime();
	snapshot->onTimeLast = g_powerOnTimeCounter.getLastTime();
}

data::Value SysInfoPage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	if (id == DATA_ID_SYS_ON_TIME_TOTAL) {
		return data::Value(snapshot->onTimeTotal, data::VALUE_TYPE_ON_TIME_COUNTER);
	}

	if (id == DATA_ID_SYS_ON_TIME_LAST) {
		return data::Value(snapshot->onTimeLast, data::VALUE_TYPE_ON_TIME_COUNTER);
	}

	if (id == DATA_ID_SYS_INFO_FIRMWARE_VER) {
		return data::Value(FIRMWARE);
	}

	if (id == DATA_ID_SYS_INFO_SERIAL_NO) {
		return data::Value(PSU_SERIAL);
	}

	if (id == DATA_ID_SYS_TEMP_MAIN_STATUS) {
		return data::Value(snapshot->flags.mainTemperatureStatus);
	}

	if (id == DATA_ID_SYS_TEMP_MAIN && snapshot->flags.mainTemperatureStatus == 1) {
		return data::Value(snapshot->mainTemperature, data::VALUE_TYPE_FLOAT_CELSIUS);
	}

	if (id == DATA_ID_SYS_TEMP_AUX_STATUS) {
		return data::Value(2);
	}

	if (id == DATA_ID_SYS_INFO_CPU) {
		return data::Value(getCpuType());
	}

	if (cursor.iChannel >= 0) {
		int iChannel = cursor.iChannel >= 0 ? cursor.iChannel : g_channel->index - 1;

		if (id == DATA_ID_CHANNEL_BOARD_INFO_LABEL) {
			return data::Value(iChannel + 1, data::VALUE_TYPE_CHANNEL_BOARD_INFO_LABEL);
		}

		if (id == DATA_ID_CHANNEL_BOARD_INFO_REVISION) {
			if (iChannel < CH_NUM) {
				return data::Value(Channel::get(iChannel).getBoardRevisionName());
			}
		}
	}

	if (id == DATA_ID_SYS_INFO_SCPI_VER) {
		return data::Value(SCPI_STD_VERSION_REVISION);
	}

	if (id == DATA_ID_SYS_INFO_ETHERNET) {
		return data::Value(getCpuEthernetType());
	}

	if (id == DATA_ID_SYS_INFO_FAN_STATUS) {
		return data::Value(snapshot->flags.fanStatus);
	}

	if (id == DATA_ID_SYS_INFO_FAN_SPEED && snapshot->flags.fanStatus == 1) {
		return data::Value(snapshot->fanSpeed, data::VALUE_TYPE_FLOAT_RPM);
	}

	return data::Value();
}

}
}
} // namespace eez::psu::gui
