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
#include "gui_page_event_queue.h"

namespace eez {
namespace psu {
namespace gui {

void EventQueuePage::pageWillAppear() {
	event_queue::moveToFirstPage();
	event_queue::markAsRead();
}

void EventQueuePage::takeSnapshot(data::Snapshot *snapshot) {
	int n = event_queue::getActivePageNumEvents();
	for (int i = 0; i < event_queue::EVENTS_PER_PAGE; ++i) {
		if (i < n) {
			event_queue::getActivePageEvent(i, snapshot->events + i);
		} else {
			snapshot->events[i].eventId = event_queue::EVENT_TYPE_NONE;
		}
	}
	snapshot->eventQueuePageInfo = data::Value::PageInfo(event_queue::getActivePageIndex(), event_queue::getNumPages());
}

data::Value EventQueuePage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	if (cursor.i >= 0) {
		if (id == DATA_ID_EVENT_QUEUE_EVENTS_TYPE) {
			return data::Value(event_queue::getEventType(snapshot->events + cursor.i));
		} 
		
		if (id == DATA_ID_EVENT_QUEUE_EVENTS_MESSAGE) {
			return data::Value(snapshot->events + cursor.i);
		}
	}

	if (id == DATA_ID_EVENT_QUEUE_MULTIPLE_PAGES) {
		return data::Value(snapshot->eventQueuePageInfo.getNumPages() > 1 ? 1 : 0);
	}

	if (id == DATA_ID_EVENT_QUEUE_PREVIOUS_PAGE_ENABLED) {
		return data::Value(snapshot->eventQueuePageInfo.getPageIndex() > 0 ? 1 : 0);
	}

	if (id == DATA_ID_EVENT_QUEUE_NEXT_PAGE_ENABLED) {
		return data::Value(snapshot->eventQueuePageInfo.getPageIndex() < snapshot->eventQueuePageInfo.getNumPages() - 1 ? 1 : 0);
	}

	if (id == DATA_ID_EVENT_QUEUE_PAGE_INFO) {
		return snapshot->eventQueuePageInfo;
	}

	return data::Value();
}

}
}
} // namespace eez::psu::gui
