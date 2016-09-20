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

void SysSettingsDateTimePage::edit() {
	DECL_WIDGET(widget, g_foundWidgetAtDown.widgetOffset);
	int id = widget->data;

	numeric_keypad::Options options;

	options.editUnit = data::VALUE_TYPE_INT;

	options.flags.genericNumberKeypad = true;
	options.flags.maxButtonEnabled = false;
	options.flags.defButtonEnabled = false;
	options.flags.signButtonEnabled = false;
	options.flags.dotButtonEnabled = false;

	const char *label = 0;

	if (id == DATA_ID_SYS_INFO_DATE_TIME_YEAR) {
		label = "Year (2016-2036): ";
		options.min = 2016;
		options.max = 2036;
		options.def = 2016;
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_MONTH) {
		label = "Month (1-12): ";
		options.min = 1;
		options.max = 12;
		options.def = 1;
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_DAY) {
		label = "Day (1-31): ";
		options.min = 1;
		options.max = 31;
		options.def = 1;
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_HOUR) {
		label = "Hour (0-23): ";
		options.min = 0;
		options.max = 23;
		options.def = 12;
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_MINUTE) {
		label = "Minute (0-59): ";
		options.min = 0;
		options.max = 59;
		options.def = 0;
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_SECOND) {
		label = "Second (0-59): ";
		options.min = 0;
		options.max = 59;
		options.def = 0;
	}

	if (label) {
		editDataId = id;
		numeric_keypad::start(label, options, onSetValue, showPreviousPage);
	}
}

void SysSettingsDateTimePage::setValue(float value) {
	if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_YEAR) {
		dateTime.year = uint16_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_MONTH) {
		dateTime.month = uint8_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_DAY) {
		dateTime.day = uint8_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_HOUR) {
		dateTime.hour = uint8_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_MINUTE) {
		dateTime.minute = uint8_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_SECOND) {
		dateTime.second = uint8_t(value);
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
