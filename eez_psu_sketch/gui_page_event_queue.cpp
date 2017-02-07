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

#include "gui_page_event_queue.h"

namespace eez {
namespace psu {
namespace gui {

event_queue::Event g_stateEvents[2][event_queue::EVENTS_PER_PAGE];

void EventQueuePage::pageWillAppear() {
	event_queue::moveToFirstPage();
	event_queue::markAsRead();
}

data::Value EventQueuePage::getData(const data::Cursor &cursor, uint8_t id) {
	if (cursor.i >= 0 && (id == DATA_ID_EVENT_QUEUE_EVENTS_TYPE || id == DATA_ID_EVENT_QUEUE_EVENTS_MESSAGE)) {
        event_queue::Event *event = &g_stateEvents[getCurrentStateBufferIndex()][cursor.i];

        int n = event_queue::getActivePageNumEvents();
        if (cursor.i < n) {
    		event_queue::getActivePageEvent(cursor.i, event);
		} else {
            event->eventId = event_queue::EVENT_TYPE_NONE;
		}

        if (id == DATA_ID_EVENT_QUEUE_EVENTS_TYPE) {
            return data::Value(event_queue::getEventType(event));
		} 
		
		if (id == DATA_ID_EVENT_QUEUE_EVENTS_MESSAGE) {
			return data::Value(event);
		}
	}

	if (id == DATA_ID_EVENT_QUEUE_MULTIPLE_PAGES) {
        data::Value eventQueuePageInfo = data::Value::PageInfo(event_queue::getActivePageIndex(), event_queue::getNumPages());
		return data::Value(eventQueuePageInfo.getNumPages() > 1 ? 1 : 0);
	}

	if (id == DATA_ID_EVENT_QUEUE_PREVIOUS_PAGE_ENABLED) {
        data::Value eventQueuePageInfo = data::Value::PageInfo(event_queue::getActivePageIndex(), event_queue::getNumPages());
		return data::Value(eventQueuePageInfo.getPageIndex() > 0 ? 1 : 0);
	}

	if (id == DATA_ID_EVENT_QUEUE_NEXT_PAGE_ENABLED) {
		data::Value eventQueuePageInfo = data::Value::PageInfo(event_queue::getActivePageIndex(), event_queue::getNumPages());
        return data::Value(eventQueuePageInfo.getPageIndex() < eventQueuePageInfo.getNumPages() - 1 ? 1 : 0);
	}

	if (id == DATA_ID_EVENT_QUEUE_PAGE_INFO) {
        data::Value eventQueuePageInfo = data::Value::PageInfo(event_queue::getActivePageIndex(), event_queue::getNumPages());
		return eventQueuePageInfo;
	}

	return data::Value();
}

}
}
} // namespace eez::psu::gui

#endif