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
#include "datetime.h"
#include "eeprom.h"
#include "event_queue.h"

#include "gui_internal.h"

namespace eez {
namespace psu {
namespace event_queue {

static const uint32_t MAGIC = 0xD8152FC3L;
static const uint16_t VERSION = 2;

static const uint16_t MAX_EVENTS = 100;

struct EventQueueHeader {
	uint32_t magicNumber;
	uint16_t version;
	uint16_t head;
	uint16_t size;
};

static EventQueueHeader eventQueue;

bool g_unread = false;
bool g_refreshGUI = false;

static int16_t g_eventsDuringInterruptHandling[6];
static uint8_t g_eventsDuringInterruptHandlingHead = 0;
static const int MAX_EVENTS_DURING_INTERRUPT_HANDLING = sizeof(g_eventsDuringInterruptHandling) / sizeof(int16_t);

static const int EVENTS_PER_PAGE = 7;
static uint8_t g_pageIndex = 0;

void init() {
	eeprom::read((uint8_t *)&eventQueue, sizeof(EventQueueHeader), eeprom::EEPROM_EVENT_QUEUE_START_ADDRESS);

	if (eventQueue.magicNumber != MAGIC || eventQueue.version != VERSION || eventQueue.head >= MAX_EVENTS || eventQueue.size > MAX_EVENTS) {
		eventQueue = {
			MAGIC,
			VERSION,
			0,
			0
		};

		pushEvent(EVENT_INFO_WELCOME);
	}
}

void tick(unsigned long tick_usec) {
	for (int i = 0; i < g_eventsDuringInterruptHandlingHead; ++i) {
		pushEvent(g_eventsDuringInterruptHandling[i]);
	}
	g_eventsDuringInterruptHandlingHead = 0;

	if (g_refreshGUI) {
		if (gui::getActivePage() == gui::PAGE_ID_EVENT_QUEUE) {
			gui::refreshPage();
		}
		g_refreshGUI = false;
	}
}

int getNumEvents() {
	return eventQueue.size;
}

void getEvent(int i, Event *e) {
	i = (eventQueue.head - (i + 1) + MAX_EVENTS) % MAX_EVENTS;

	eeprom::read((uint8_t *)e, sizeof(Event), eeprom::EEPROM_EVENT_QUEUE_START_ADDRESS + sizeof(EventQueueHeader) + i * sizeof(Event));
}

int getEventType(Event *e) {
	if (e->eventId >= EVENT_INFO_START_ID) {
		return EVENT_TYPE_INFO;
	} else if (e->eventId >= EVENT_WARNING_START_ID) {
		return EVENT_TYPE_WARNING;
	} else if (e->eventId > 0) {
		return EVENT_TYPE_ERROR;
	} else {
		return EVENT_TYPE_NONE;
	}
}

const char *getEventMessage(Event *e) {
	static char message[35];

	const char *p_message = 0;

	if (e->eventId >= EVENT_INFO_START_ID) {
		switch (e->eventId) {
#define EVENT_ERROR(NAME, ID, TEXT)
#define EVENT_WARNING(NAME, ID, TEXT)
#define EVENT_INFO(NAME, ID, TEXT) case EVENT_INFO_START_ID + ID: p_message = PSTR(TEXT); break;
			LIST_OF_EVENTS
#undef EVENT_INFO
#undef EVENT_WARNING
#undef EVENT_ERROR
		}
	} else if (e->eventId >= EVENT_WARNING_START_ID) {
		switch (e->eventId) {
#define EVENT_ERROR(NAME, ID, TEXT)
#define EVENT_WARNING(NAME, ID, TEXT) case EVENT_WARNING_START_ID + ID: p_message = PSTR(TEXT); break;
#define EVENT_INFO(NAME, ID, TEXT)
			LIST_OF_EVENTS
#undef EVENT_INFO
#undef EVENT_WARNING
#undef EVENT_ERROR
		default:
			p_message = 0;
		}
	} else if (e->eventId >= EVENT_ERROR_START_ID) {
		switch (e->eventId) {
#define EVENT_INFO(NAME, ID, TEXT)
#define EVENT_WARNING(NAME, ID, TEXT)
#define EVENT_ERROR(NAME, ID, TEXT) case EVENT_ERROR_START_ID + ID: p_message = PSTR(TEXT); break;
			LIST_OF_EVENTS
#undef EVENT_INFO
#undef EVENT_WARNING
#undef EVENT_ERROR
		}
	} else if (e->eventId > 0) {
		return SCPI_ErrorTranslate(e->eventId);
	}

	if (p_message) {
		strncpy_P(message, p_message, sizeof(message) - 1);
		message[sizeof(message) - 1] = 0;
		return message;
	}

	return 0;
}

void pushEvent(int16_t eventId) {
	if (g_insideInterruptHandler) {
		if (g_eventsDuringInterruptHandlingHead < MAX_EVENTS_DURING_INTERRUPT_HANDLING) {
			g_eventsDuringInterruptHandling[g_eventsDuringInterruptHandlingHead] = eventId;
			++g_eventsDuringInterruptHandlingHead;
		} else {
			DebugTrace("MAX_EVENTS_DURING_INTERRUPT_HANDLING exceeded");
		}
	} else {
		Event e;

		e.dateTime = datetime::now();
		e.eventId = eventId;

		eeprom::write((uint8_t *)&e, sizeof(Event), eeprom::EEPROM_EVENT_QUEUE_START_ADDRESS + sizeof(EventQueueHeader) + eventQueue.head * sizeof(Event));

		eventQueue.head = (eventQueue.head + 1) % MAX_EVENTS;
		if (eventQueue.size < MAX_EVENTS) {
			++eventQueue.size;
		}

		eeprom::write((uint8_t *)&eventQueue, sizeof(EventQueueHeader), eeprom::EEPROM_EVENT_QUEUE_START_ADDRESS);

		g_unread = true;
		g_refreshGUI = true;
	}
}

void markAsRead() {
	g_unread = false;
}

int getNumPages() {
	return getNumEvents() / EVENTS_PER_PAGE + 1;
}

int getActivePageNumEvents() {
	if (g_pageIndex < getNumPages() - 1) {
		return EVENTS_PER_PAGE;
	} else {
		return getNumEvents() - (getNumPages() - 1) * EVENTS_PER_PAGE;
	}
}

void getActivePageEvent(int i, Event *e) {
	getEvent(g_pageIndex * EVENTS_PER_PAGE + i, e);
}

void moveToFirstPage() {
	g_pageIndex = 0;
}

void moveToNextPage() {
	if (g_pageIndex < getNumPages() - 1) {
		++g_pageIndex;
		g_refreshGUI = true;
	}
}

void moveToPreviousPage() {
	if (g_pageIndex > 0) {
		--g_pageIndex;
		g_refreshGUI = true;
	}
}

int getActivePageIndex() {
	return g_pageIndex;
}

}
}
} // namespace eez::psu::event_queue
