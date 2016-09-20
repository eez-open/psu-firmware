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

#include "gui_data_snapshot.h"
#include "gui_page_sys_settings_date_time.h"
#include "gui_numeric_keypad.h"

namespace eez {
namespace psu {
namespace gui {

void SysSettingsDateTimePage::pageWillAppear() {
	origDateTime = datetime::DateTime::now();
	dateTime = origDateTime;
}

data::Value SysSettingsDateTimePage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	data::Value value = SetPage::getData(cursor, id, snapshot);
	if (value.getType() != data::VALUE_TYPE_NONE) {
		return value;
	}

	if (id == DATA_ID_SYS_INFO_DATE_TIME_YEAR) {
		return data::Value(dateTime.year);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_MONTH) {
		return data::Value(dateTime.month);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_DAY) {
		return data::Value(dateTime.day);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_HOUR) {
		return data::Value(dateTime.hour);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_MINUTE) {
		return data::Value(dateTime.minute);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_SECOND) {
		return data::Value(dateTime.second);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_DST) {
		return data::Value(0);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_TIME_ZONE) {
		return data::Value("Not supported");
	}

	return data::Value();
}

void onYearSet(float value) {
}

void SysSettingsDateTimePage::edit() {
	DECL_WIDGET(widget, g_foundWidgetAtDown.widgetOffset);
	int id = widget->data;
	if (id == DATA_ID_SYS_INFO_DATE_TIME_YEAR) {
		//numeric_keypad::start("Year: ", 2016, 3016, dateTime.year);
	}
}

int SysSettingsDateTimePage::getDirty() {
	return dateTime != origDateTime;
}

void SysSettingsDateTimePage::set() {
}


}
}
} // namespace eez::psu::gui
