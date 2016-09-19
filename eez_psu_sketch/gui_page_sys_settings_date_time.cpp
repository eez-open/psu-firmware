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

#include "profile.h"

#include "gui_data_snapshot.h"
#include "gui_page_sys_settings_date_time.h"

namespace eez {
namespace psu {
namespace gui {

data::Value SysSettingsDateTimePage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	if (id == DATA_ID_SYS_INFO_DATE_TIME_YEAR) {
		return data::Value("2016");
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_MONTH) {
		return data::Value("09");
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_DAY) {
		return data::Value("16");
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_HOUR) {
		return data::Value("16");
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_MINUTE) {
		return data::Value("00");
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_SECOND) {
		return data::Value("00");
	}

	return data::Value();
}

void SysSettingsDateTimePage::edit() {
}

int SysSettingsDateTimePage::getDirty() {
	return 0;
}

void SysSettingsDateTimePage::set() {
}


}
}
} // namespace eez::psu::gui
