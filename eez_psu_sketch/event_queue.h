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
 
#pragma once

namespace eez {
namespace psu {
namespace event_queue {

static const int EVENT_TYPE_NONE = 0;
static const int EVENT_TYPE_INFO = 1;
static const int EVENT_TYPE_WARNING = 2;
static const int EVENT_TYPE_ERROR = 3;

////////////////////////////////////////////////////////////////////////////////

#define LIST_OF_EVENTS \
	EVENT_ERROR(CH1_OVP_TRIPPED,  0, "Ch1 OVP tripped") \
	EVENT_ERROR(CH1_OCP_TRIPPED,  1, "Ch1 OCP tripped") \
	EVENT_ERROR(CH2_OVP_TRIPPED,  3, "Ch2 OVP tripped") \
	EVENT_ERROR(CH2_OCP_TRIPPED,  4, "Ch2 OCP tripped") \
	EVENT_ERROR(CH2_OPP_TRIPPED,  5, "Ch2 OPP tripped") \
	EVENT_ERROR(MAIN_OTP_TRIPPED, 30, "MAIN OTP tripped") \
	EVENT_ERROR(CH1_OTP_TRIPPED,  31, "CH1 OTP tripped") \
	EVENT_ERROR(CH2_OTP_TRIPPED,  32, "CH2 OTP tripped") \
	EVENT_INFO(WELCOME, 0, "Welcome!") \
	EVENT_INFO(POWER_UP, 1, "Power up") \
	EVENT_INFO(POWER_DOWN, 2, "Power down") \
	EVENT_INFO(CALIBRATION_PASSWORD_CHANGED, 3, "Calibration password changed") \
	EVENT_INFO(BEEPER_ENABLED, 4, "Beeper enabled") \
	EVENT_INFO(BEEPER_DISABLED, 5, "Beeper disabled") \
	EVENT_INFO(CH1_OUTPUT_ENABLED, 10, "Ch1 output enabled") \
	EVENT_INFO(CH2_OUTPUT_ENABLED, 11, "Ch2 output enabled") \
	EVENT_INFO(CH1_OUTPUT_DISABLED, 20, "Ch1 output disabled") \
	EVENT_INFO(CH2_OUTPUT_DISABLED, 21, "Ch2 output disabled") \
	EVENT_INFO(CH1_REMOTE_SENSE_ENABLED, 30, "Ch1 remote sense enabled") \
	EVENT_INFO(CH2_REMOTE_SENSE_ENABLED, 31, "Ch2 remote sense enabled") \
	EVENT_INFO(CH1_REMOTE_SENSE_DISABLED, 40, "Ch1 remote sense disabled") \
	EVENT_INFO(CH2_REMOTE_SENSE_DISABLED, 41, "Ch2 remote sense disabled") \
	EVENT_INFO(CH1_REMOTE_PROG_ENABLED, 50, "Ch1 remote prog enabled") \
	EVENT_INFO(CH2_REMOTE_PROG_ENABLED, 51, "Ch2 remote prog enabled") \
	EVENT_INFO(CH1_REMOTE_PROG_DISABLED, 60, "Ch1 remote prog disabled") \
	EVENT_INFO(CH2_REMOTE_PROG_DISABLED, 61, "Ch2 remote prog disabled") \
	EVENT_INFO(RECALL_FROM_PROFILE_0, 70, "Recall from profile 0") \
	EVENT_INFO(RECALL_FROM_PROFILE_1, 71, "Recall from profile 1") \
	EVENT_INFO(RECALL_FROM_PROFILE_2, 72, "Recall from profile 2") \
	EVENT_INFO(RECALL_FROM_PROFILE_3, 73, "Recall from profile 3") \
	EVENT_INFO(RECALL_FROM_PROFILE_4, 74, "Recall from profile 4") \
	EVENT_INFO(RECALL_FROM_PROFILE_5, 75, "Recall from profile 5") \
	EVENT_INFO(RECALL_FROM_PROFILE_6, 76, "Recall from profile 6") \
	EVENT_INFO(RECALL_FROM_PROFILE_7, 77, "Recall from profile 7") \
	EVENT_INFO(RECALL_FROM_PROFILE_8, 78, "Recall from profile 8") \
	EVENT_INFO(RECALL_FROM_PROFILE_9, 79, "Recall from profile 9") \
	EVENT_INFO(DEFAULE_PROFILE_CHANGED_TO_0, 80, "Default profile changed to 0") \
	EVENT_INFO(DEFAULE_PROFILE_CHANGED_TO_1, 81, "Default profile changed to 1") \
	EVENT_INFO(DEFAULE_PROFILE_CHANGED_TO_2, 82, "Default profile changed to 2") \
	EVENT_INFO(DEFAULE_PROFILE_CHANGED_TO_3, 83, "Default profile changed to 3") \
	EVENT_INFO(DEFAULE_PROFILE_CHANGED_TO_4, 84, "Default profile changed to 4") \
	EVENT_INFO(DEFAULE_PROFILE_CHANGED_TO_5, 85, "Default profile changed to 5") \
	EVENT_INFO(DEFAULE_PROFILE_CHANGED_TO_6, 86, "Default profile changed to 6") \
	EVENT_INFO(DEFAULE_PROFILE_CHANGED_TO_7, 87, "Default profile changed to 7") \
	EVENT_INFO(DEFAULE_PROFILE_CHANGED_TO_8, 88, "Default profile changed to 8") \
	EVENT_INFO(DEFAULE_PROFILE_CHANGED_TO_9, 89, "Default profile changed to 9")

#define EVENT_ERROR_START_ID 10000
#define EVENT_WARNING_START_ID 12000
#define EVENT_INFO_START_ID 14000

#define EVENT_ERROR(NAME, ID, TEXT) EVENT_ERROR_##NAME = EVENT_ERROR_START_ID + ID,
#define EVENT_WARNING(NAME, ID, TEXT) EVENT_WARNING_##NAME = EVENT_WARNING_START_ID + ID,
#define EVENT_INFO(NAME, ID, TEXT) EVENT_INFO_##NAME = EVENT_INFO_START_ID + ID,
enum Events {
	LIST_OF_EVENTS
};
#undef EVENT_INFO
#undef EVENT_WARNING
#undef EVENT_ERROR

////////////////////////////////////////////////////////////////////////////////

struct Event {
	uint32_t dateTime;
	int16_t eventId;
};

extern bool g_unread;

void init();
void tick(unsigned long tick_usec);

int getNumEvents();
void getEvent(int i, Event *e);

int getEventType(Event *e);
const char *getEventMessage(Event *e);

void pushEvent(int16_t eventId);

void markAsRead();

int getNumPages();
int getActivePageNumEvents();
void getActivePageEvent(int i, Event *e);
void moveToFirstPage();
void moveToNextPage();
void moveToPreviousPage();
int getActivePageIndex();

}
}
} // namespace eez::psu::event_queue
