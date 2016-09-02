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

namespace eez {
namespace psu {
namespace event_queue {

static const uint32_t MAGIC = 0xD8152FC3L;
static const uint16_t VERSION = 1;

static const uint16_t MAX_EVENTS = 100;

struct EventQueueHeader {
	uint32_t magicNumber;
	uint16_t version;
	uint16_t head;
	uint16_t size;
};

static EventQueueHeader eventQueue;

void init() {
	eeprom::read((uint8_t *)&eventQueue, sizeof(EventQueueHeader), eeprom::EEPROM_EVENT_QUEUE_START_ADDRESS);

	if (eventQueue.magicNumber != MAGIC || eventQueue.version != VERSION || eventQueue.head >= MAX_EVENTS || eventQueue.size > MAX_EVENTS) {
		eventQueue = {
			MAGIC,
			VERSION,
			0,
			0
		};

		pushEvent(EVENT_TYPE_INFO, "Welcome!");
	}
}

int getNumEvents() {
	return eventQueue.size;
}

void getEvent(int i, Event *e) {
	i = (eventQueue.head - (i + 1) + MAX_EVENTS) % MAX_EVENTS;

	eeprom::read((uint8_t *)e, sizeof(Event), eeprom::EEPROM_EVENT_QUEUE_START_ADDRESS + sizeof(EventQueueHeader) + i * sizeof(Event));
}

void pushEvent(uint8_t type,  const char *message) {
	Event e;

	e.dateTime = datetime::now();
	e.type = type;
	strncpy(e.message, message, sizeof(e.message));
	e.message[sizeof(e.message) - 1] = 0;

	eeprom::write((uint8_t *)&e, sizeof(Event), eeprom::EEPROM_EVENT_QUEUE_START_ADDRESS + sizeof(EventQueueHeader) + eventQueue.head * sizeof(Event));

	eventQueue.head = (eventQueue.head + 1) % MAX_EVENTS;
	if (eventQueue.size < MAX_EVENTS) {
		++eventQueue.size;
	}

	eeprom::write((uint8_t *)&eventQueue, sizeof(EventQueueHeader), eeprom::EEPROM_EVENT_QUEUE_START_ADDRESS);
}

void clear() {
	eventQueue.head = 0;
	eventQueue.size = 0;
	eeprom::write((uint8_t *)&eventQueue, sizeof(EventQueueHeader), eeprom::EEPROM_EVENT_QUEUE_START_ADDRESS);
}

}
}
} // namespace eez::psu::event_queue
