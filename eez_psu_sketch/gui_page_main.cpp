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

#if OPTION_DISPLAY

#include "gui_data_snapshot.h"
#include "gui_page_main.h"

namespace eez {
namespace psu {
namespace gui {

void MainPage::takeSnapshot(data::Snapshot *snapshot) {
	event_queue::getLastErrorEvent(&snapshot->lastEvent);
}

data::Value MainPage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	if (id == DATA_ID_EVENT_QUEUE_LAST_EVENT_TYPE) {
		return data::Value(event_queue::getEventType(&snapshot->lastEvent));
	}
		
	if (id == DATA_ID_EVENT_QUEUE_LAST_EVENT_MESSAGE) {
		if (event_queue::getEventType(&snapshot->lastEvent) != event_queue::EVENT_TYPE_NONE) {
			return data::Value(&snapshot->lastEvent);
		}
	}

	return data::Value();
}

}
}
} // namespace eez::psu::gui

#endif