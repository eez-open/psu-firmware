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
    EVENT_SCPI_ERROR(SCPI_ERROR_AUX_TEMP_SENSOR_TEST_FAILED, "AUX temp failed") \
    EVENT_SCPI_ERROR(SCPI_ERROR_CH1_TEMP_SENSOR_TEST_FAILED, "CH1 temp failed") \
    EVENT_SCPI_ERROR(SCPI_ERROR_CH2_TEMP_SENSOR_TEST_FAILED, "CH2 temp failed") \
    EVENT_SCPI_ERROR(SCPI_ERROR_CH1_DOWN_PROGRAMMER_SWITCHED_OFF, "DProg CH1 disabled") \
    EVENT_SCPI_ERROR(SCPI_ERROR_CH2_DOWN_PROGRAMMER_SWITCHED_OFF, "DProg CH2 disabled") \
    EVENT_SCPI_ERROR(SCPI_ERROR_CH1_OUTPUT_FAULT_DETECTED , "CH1 output fault") \
    EVENT_SCPI_ERROR(SCPI_ERROR_CH2_OUTPUT_FAULT_DETECTED , "CH2 output fault") \
    EVENT_ERROR(CH1_OVP_TRIPPED,  0, "Ch1 OVP tripped") \
    EVENT_ERROR(CH1_OCP_TRIPPED,  1, "Ch1 OCP tripped") \
    EVENT_ERROR(CH1_OPP_TRIPPED,  2, "Ch1 OPP tripped") \
    EVENT_ERROR(CH2_OVP_TRIPPED,  3, "Ch2 OVP tripped") \
    EVENT_ERROR(CH2_OCP_TRIPPED,  4, "Ch2 OCP tripped") \
    EVENT_ERROR(CH2_OPP_TRIPPED,  5, "Ch2 OPP tripped") \
    EVENT_ERROR(AUX_OTP_TRIPPED, 39, "AUX OTP tripped") \
    EVENT_ERROR(CH1_OTP_TRIPPED,  40, "CH1 OTP tripped") \
    EVENT_ERROR(CH2_OTP_TRIPPED,  41, "CH2 OTP tripped") \
    EVENT_ERROR(CH1_REMOTE_SENSE_REVERSE_POLARITY_DETECTED, 50, "CH1 rsense reverse polarity detected") \
    EVENT_ERROR(CH2_REMOTE_SENSE_REVERSE_POLARITY_DETECTED, 51, "CH2 rsense reverse polarity detected") \
    EVENT_WARNING(CH1_CALIBRATION_DISABLED, 0, "Ch1 calibration disabled") \
    EVENT_WARNING(CH2_CALIBRATION_DISABLED, 1, "Ch2 calibration disabled") \
    EVENT_WARNING(ETHERNET_NOT_CONNECTED, 2, "Ethernet not connected") \
    EVENT_WARNING(AUTO_RECALL_VALUES_MISMATCH, 3, "Auto-recall mismatch") \
    EVENT_INFO(WELCOME, 0, "Welcome!") \
    EVENT_INFO(POWER_UP, 1, "Power up") \
    EVENT_INFO(POWER_DOWN, 2, "Power down") \
    EVENT_INFO(CALIBRATION_PASSWORD_CHANGED, 3, "Calibration password changed") \
    EVENT_INFO(SOUND_ENABLED, 4, "Sound enabled") \
    EVENT_INFO(SOUND_DISABLED, 5, "Sound disabled") \
    EVENT_INFO(SYSTEM_DATE_TIME_CHANGED, 6, "Date/time changed") \
    EVENT_INFO(ETHERNET_ENABLED, 7, "Ethernet enabled") \
    EVENT_INFO(ETHERNET_DISABLED, 8, "Ethernet disabled") \
    EVENT_INFO(SYSTEM_PASSWORD_CHANGED, 9, "System password changed") \
    EVENT_INFO(CH1_OUTPUT_ENABLED, 10, "Ch1 output on") \
    EVENT_INFO(CH2_OUTPUT_ENABLED, 11, "Ch2 output on") \
    EVENT_INFO(CH1_OUTPUT_DISABLED, 20, "Ch1 output off") \
    EVENT_INFO(CH2_OUTPUT_DISABLED, 21, "Ch2 output off") \
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
    EVENT_INFO(DEFAULE_PROFILE_CHANGED_TO_9, 89, "Default profile changed to 9") \
    EVENT_INFO(CH1_CALIBRATION_ENABLED, 90, "Ch1 calibration enabled") \
    EVENT_INFO(CH2_CALIBRATION_ENABLED, 91, "Ch2 calibration enabled") \
    EVENT_INFO(COUPLED_IN_PARALLEL, 92, "Coupled in parallel") \
    EVENT_INFO(COUPLED_IN_SERIES, 93, "Coupled in series") \
    EVENT_INFO(CHANNELS_UNCOUPLED, 94, "Channels uncoupled") \
    EVENT_INFO(CHANNELS_TRACKED, 95, "Channels operates in track mode") \
    EVENT_INFO(OUTPUT_PROTECTION_COUPLED, 96, "Output protection coupled") \
    EVENT_INFO(OUTPUT_PROTECTION_DECOUPLED, 97, "Output protection decoupled") \
    EVENT_INFO(SHUTDOWN_WHEN_PROTECTION_TRIPPED_ENABLED, 98, "Shutdown when tripped enabled") \
    EVENT_INFO(SHUTDOWN_WHEN_PROTECTION_TRIPPED_DISABLED, 99, "Shutdown when tripped disabled") \
    EVENT_INFO(FORCE_DISABLING_ALL_OUTPUTS_ON_POWERUP_ENABLED, 100, "Force disabling outputs enabled") \
    EVENT_INFO(FORCE_DISABLING_ALL_OUTPUTS_ON_POWERUP_DISABLED, 101, "Force disabling outputs disabled") \
    EVENT_INFO(FRONT_PANEL_LOCKED, 102, "Front panel locked") \
    EVENT_INFO(FRONT_PANEL_UNLOCKED, 103, "Front panel unlocked") \
    

#define EVENT_ERROR_START_ID 10000
#define EVENT_WARNING_START_ID 12000
#define EVENT_INFO_START_ID 14000

#define EVENT_SCPI_ERROR(ID, TEXT)
#define EVENT_ERROR(NAME, ID, TEXT) EVENT_ERROR_##NAME = EVENT_ERROR_START_ID + ID,
#define EVENT_WARNING(NAME, ID, TEXT) EVENT_WARNING_##NAME = EVENT_WARNING_START_ID + ID,
#define EVENT_INFO(NAME, ID, TEXT) EVENT_INFO_##NAME = EVENT_INFO_START_ID + ID,
enum Events {
    LIST_OF_EVENTS
};
#undef EVENT_SCPI_ERROR
#undef EVENT_INFO
#undef EVENT_WARNING
#undef EVENT_ERROR

////////////////////////////////////////////////////////////////////////////////

#if DISPLAY_ORIENTATION == DISPLAY_ORIENTATION_PORTRAIT
static const int EVENTS_PER_PAGE = 10;
#elif DISPLAY_ORIENTATION == DISPLAY_ORIENTATION_LANDSCAPE
static const int EVENTS_PER_PAGE = 7;
#endif

////////////////////////////////////////////////////////////////////////////////

struct EventQueueHeader {
    uint32_t magicNumber;
    uint16_t version;
    uint16_t head;
    uint16_t size;
    uint16_t lastErrorEventIndex;
};

struct Event {
    uint32_t dateTime;
    int16_t eventId;
};

void init();
void tick(unsigned long tick_usec);

void getLastErrorEvent(Event *e);

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
