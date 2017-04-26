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
#include "sound.h"
#include "event_queue.h"

namespace eez {
namespace psu {
namespace event_queue {

static const uint32_t MAGIC = 0xD8152FC3L;
static const uint16_t VERSION = 4;

static const uint16_t MAX_EVENTS = 100;
static const uint16_t NULL_INDEX = MAX_EVENTS;

static const uint16_t EVENT_HEADER_SIZE = 16;
static const uint16_t EVENT_SIZE = 16;

static EventQueueHeader eventQueue;

static int16_t g_eventsToPush[6];
static uint8_t g_eventsToPushHead = 0;
static const int MAX_EVENTS_TO_PUSH = sizeof(g_eventsToPush) / sizeof(int16_t);

static uint8_t g_pageIndex = 0;

static Event g_lastErrorEvent;
static bool g_lastErrorEventChanged;

void readHeader() {
    eeprom::read((uint8_t *)&eventQueue, sizeof(EventQueueHeader), eeprom::EEPROM_EVENT_QUEUE_START_ADDRESS);
}

void writeHeader() {
    if (eeprom::g_testResult == psu::TEST_OK) {
        eeprom::write((uint8_t *)&eventQueue, sizeof(EventQueueHeader), eeprom::EEPROM_EVENT_QUEUE_START_ADDRESS);
    }
}

void readEvent(uint16_t eventIndex, Event *e) {
    eeprom::read((uint8_t *)e, sizeof(Event), eeprom::EEPROM_EVENT_QUEUE_START_ADDRESS + EVENT_HEADER_SIZE + eventIndex * EVENT_SIZE);
}

void writeEvent(uint16_t eventIndex, Event *e) {
    if (eeprom::g_testResult == psu::TEST_OK) {
        eeprom::write((uint8_t *)e, sizeof(Event), eeprom::EEPROM_EVENT_QUEUE_START_ADDRESS + EVENT_HEADER_SIZE + eventIndex * EVENT_SIZE);
    }
}

void init() {
    readHeader();
    g_lastErrorEventChanged = true;

    if (eventQueue.magicNumber != MAGIC || eventQueue.version != VERSION || eventQueue.head >= MAX_EVENTS || eventQueue.size > MAX_EVENTS) {
        eventQueue.magicNumber = MAGIC;
        eventQueue.version = VERSION;
        eventQueue.head = 0;
        eventQueue.size = 0;
        eventQueue.lastErrorEventIndex = NULL_INDEX;

        pushEvent(EVENT_INFO_WELCOME);
    }
}

void doPushEvent(int16_t eventId) {
    Event e;

    e.dateTime = datetime::now();
    e.eventId = eventId;

    writeEvent(eventQueue.head, &e);

    if (eventQueue.lastErrorEventIndex == eventQueue.head) {
        // this event overwrote last error event, therefore:
        eventQueue.lastErrorEventIndex = NULL_INDEX;
        g_lastErrorEventChanged = true;
    }

    int eventType = getEventType(&e);
    if (eventType == EVENT_TYPE_ERROR || eventType == EVENT_TYPE_WARNING && eventQueue.lastErrorEventIndex == NULL_INDEX) {
        eventQueue.lastErrorEventIndex = eventQueue.head;
        g_lastErrorEventChanged = true;
    }

    eventQueue.head = (eventQueue.head + 1) % MAX_EVENTS;
    if (eventQueue.size < MAX_EVENTS) {
        ++eventQueue.size;
    }

    writeHeader();

    if (getEventType(&e) == EVENT_TYPE_ERROR) {
        sound::playBeep();
    }
}

void tick(uint32_t tick_usec) {
    for (int i = 0; i < g_eventsToPushHead; ++i) {
        doPushEvent(g_eventsToPush[i]);
    }
    g_eventsToPushHead = 0;
}

int getNumEvents() {
    return eventQueue.size;
}

void getEvent(uint16_t index, Event *e) {
    uint16_t eventIndex = (eventQueue.head - (index + 1) + MAX_EVENTS) % MAX_EVENTS;
    readEvent(eventIndex, e);
}

void getLastErrorEvent(Event *e) {
    if (g_lastErrorEventChanged) {
        if (eventQueue.lastErrorEventIndex != NULL_INDEX) {
            readEvent(eventQueue.lastErrorEventIndex, &g_lastErrorEvent);
        } else {
            g_lastErrorEvent.eventId = EVENT_TYPE_NONE;
        }
        g_lastErrorEventChanged = false;
    }

    memcpy(e, &g_lastErrorEvent, sizeof(Event));
}

int getEventType(Event *e) {
    if (e->eventId >= EVENT_INFO_START_ID) {
        return EVENT_TYPE_INFO;
    } else if (e->eventId >= EVENT_WARNING_START_ID) {
        return EVENT_TYPE_WARNING;
    } else if (e->eventId != EVENT_TYPE_NONE) {
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
#define EVENT_SCPI_ERROR(ID, TEXT)
#define EVENT_ERROR(NAME, ID, TEXT)
#define EVENT_WARNING(NAME, ID, TEXT)
#define EVENT_INFO(NAME, ID, TEXT) case EVENT_INFO_START_ID + ID: p_message = PSTR(TEXT); break;
            LIST_OF_EVENTS
#undef EVENT_SCPI_ERROR
#undef EVENT_INFO
#undef EVENT_WARNING
#undef EVENT_ERROR
        }
    } else if (e->eventId >= EVENT_WARNING_START_ID) {
        switch (e->eventId) {
#define EVENT_SCPI_ERROR(ID, TEXT)
#define EVENT_ERROR(NAME, ID, TEXT)
#define EVENT_WARNING(NAME, ID, TEXT) case EVENT_WARNING_START_ID + ID: p_message = PSTR(TEXT); break;
#define EVENT_INFO(NAME, ID, TEXT)
            LIST_OF_EVENTS
#undef EVENT_SCPI_ERROR
#undef EVENT_INFO
#undef EVENT_WARNING
#undef EVENT_ERROR
        default:
            p_message = 0;
        }
    } else {
        switch (e->eventId) {
#define EVENT_SCPI_ERROR(ID, TEXT) case ID: p_message = PSTR(TEXT); break;
#define EVENT_INFO(NAME, ID, TEXT)
#define EVENT_WARNING(NAME, ID, TEXT)
#define EVENT_ERROR(NAME, ID, TEXT) case EVENT_ERROR_START_ID + ID: p_message = PSTR(TEXT); break;
            LIST_OF_EVENTS
#undef EVENT_SCPI_ERROR
#undef EVENT_INFO
#undef EVENT_WARNING
#undef EVENT_ERROR
        default:
            return SCPI_ErrorTranslate(e->eventId);
        }
    }

    if (p_message) {
        strncpy_P(message, p_message, sizeof(message) - 1);
        message[sizeof(message) - 1] = 0;
        return message;
    }

    return 0;
}

void pushEvent(int16_t eventId) {
    if (g_eventsToPushHead < MAX_EVENTS_TO_PUSH) {
        g_eventsToPush[g_eventsToPushHead] = eventId;
        ++g_eventsToPushHead;
    } else {
        DebugTrace("MAX_EVENTS_TO_PUSH exceeded");
    }
}

void markAsRead() {
    if (eventQueue.lastErrorEventIndex != NULL_INDEX) {
        eventQueue.lastErrorEventIndex = NULL_INDEX;
        g_lastErrorEventChanged = true;
        writeHeader();
    }
}

int getNumPages() {
    return (getNumEvents() + EVENTS_PER_PAGE - 1) / EVENTS_PER_PAGE;
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
    }
}

void moveToPreviousPage() {
    if (g_pageIndex > 0) {
        --g_pageIndex;
    }
}

int getActivePageIndex() {
    return g_pageIndex;
}

}
}
} // namespace eez::psu::event_queue
