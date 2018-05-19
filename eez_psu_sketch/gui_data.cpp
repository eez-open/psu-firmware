/*
* EEZ PSU Firmware
* Copyright (C) 2018-present, Envox d.o.o.
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
* along with this program.  If not, see http://www.gnu.org/licenses.
*/

#include "psu.h"

#if OPTION_DISPLAY

#include "calibration.h"
#include "channel_dispatcher.h"
#include "dlog.h"
#if OPTION_ENCODER
#include "mw_encoder.h"
#endif
#if OPTION_ETHERNET
#include "ethernet.h"
#endif
#include "event_queue.h"
#include "fan.h"
#include "list_program.h"
#include "serial_psu.h"
#include "temperature.h"
#include "trigger.h"

#include "gui_psu.h"
#include "gui_calibration.h"
#include "gui_data.h"
#include "gui_edit_mode.h"
#include "gui_edit_mode_keypad.h"
#include "gui_edit_mode_step.h"
#include "gui_numeric_keypad.h"
#include "gui_page_ch_settings_protection.h"
#include "gui_page_ch_settings_adv.h"
#include "gui_page_ch_settings_trigger.h"
#include "gui_page_self_test_result.h"
#include "gui_page_sys_settings.h"
#include "gui_page_user_profiles.h"

#define CONF_GUI_REFRESH_EVERY_MS 250
#define CONF_LIST_COUNDOWN_DISPLAY_THRESHOLD 5 // 5 seconds

using namespace eez::app::gui;

namespace eez {
namespace mw {
namespace gui {
namespace data {

const EnumItem *g_enumDefinitions[] = {
	g_channelDisplayValueEnumDefinition,
	g_channelTriggerModeEnumDefinition,
	g_triggerSourceEnumDefinition,
	g_channelCurrentRangeSelectionModeEnumDefinition,
	g_channelCurrentRangeEnumDefinition,
	g_channelTriggerOnListStopEnumDefinition,
	g_ioPinsPolarityEnumDefinition,
	g_ioPinsInputFunctionEnumDefinition,
	g_ioPinsOutputFunctionEnumDefinition,
	g_serialParityEnumDefinition,
	g_dstRuleEnumDefinition
};

}
}
}
} // eez::mw::gui::data

namespace eez {
namespace app {
namespace gui {

using namespace eez::mw::gui;
using namespace eez::mw::gui::data;
using data::EnumItem;

EnumItem g_channelDisplayValueEnumDefinition[] = {
	{ DISPLAY_VALUE_VOLTAGE, "Voltage (V)" },
	{ DISPLAY_VALUE_CURRENT, "Current (A)" },
	{ DISPLAY_VALUE_POWER, "Power (W)" },
	{ 0, 0 }
};

EnumItem g_channelTriggerModeEnumDefinition[] = {
	{ TRIGGER_MODE_FIXED, "Fixed" },
	{ TRIGGER_MODE_LIST, "List" },
	{ TRIGGER_MODE_STEP, "Step" },
	{ 0, 0 }
};

EnumItem g_triggerSourceEnumDefinition[] = {
	{ trigger::SOURCE_BUS, "Bus" },
	{ trigger::SOURCE_IMMEDIATE, "Immediate" },
	{ trigger::SOURCE_MANUAL, "Manual" },
	{ trigger::SOURCE_PIN1, "Pin1" },
	{ 0, 0 }
};

EnumItem g_channelCurrentRangeSelectionModeEnumDefinition[] = {
	{ CURRENT_RANGE_SELECTION_USE_BOTH, "Best (default)" },
	{ CURRENT_RANGE_SELECTION_ALWAYS_HIGH, "5A" },
	{ CURRENT_RANGE_SELECTION_ALWAYS_LOW, "0.5A" },
	{ 0, 0 }
};

EnumItem g_channelCurrentRangeEnumDefinition[] = {
	{ CURRENT_RANGE_HIGH, "5A" },
	{ CURRENT_RANGE_LOW, "0.5A" },
	{ 0, 0 }
};

EnumItem g_channelTriggerOnListStopEnumDefinition[] = {
	{ TRIGGER_ON_LIST_STOP_OUTPUT_OFF, "Output OFF" },
	{ TRIGGER_ON_LIST_STOP_SET_TO_FIRST_STEP, "Set to first step" },
	{ TRIGGER_ON_LIST_STOP_SET_TO_LAST_STEP, "Set to last step" },
	{ TRIGGER_ON_LIST_STOP_STANDBY, "Standby" },
	{ 0, 0 }
};

EnumItem g_ioPinsPolarityEnumDefinition[] = {
	{ io_pins::POLARITY_NEGATIVE, "Negative" },
	{ io_pins::POLARITY_POSITIVE, "Positive" },
	{ 0, 0 }
};

EnumItem g_ioPinsInputFunctionEnumDefinition[] = {
	{ io_pins::FUNCTION_NONE, "None" },
	{ io_pins::FUNCTION_INPUT, "Input" },
	{ io_pins::FUNCTION_INHIBIT, "Inhibit" },
	{ io_pins::FUNCTION_TINPUT, "Trigger input", "Tinput" },
	{ 0, 0 }
};

EnumItem g_ioPinsOutputFunctionEnumDefinition[] = {
	{ io_pins::FUNCTION_NONE, "None" },
	{ io_pins::FUNCTION_OUTPUT, "Output" },
	{ io_pins::FUNCTION_FAULT, "Fault" },
	{ io_pins::FUNCTION_ON_COUPLE, "Channel ON couple", "ONcoup" },
	{ io_pins::FUNCTION_TOUTPUT, "Trigger output", "Toutput" },
	{ 0, 0 }
};

EnumItem g_serialParityEnumDefinition[] = {
	{ serial::PARITY_NONE, "None" },
	{ serial::PARITY_EVEN, "Even" },
	{ serial::PARITY_ODD, "Odd" },
	{ serial::PARITY_MARK, "Mark" },
	{ serial::PARITY_SPACE, "Space" },
	{ 0, 0 }
};

EnumItem g_dstRuleEnumDefinition[] = {
	{ datetime::DST_RULE_OFF, "Off" },
	{ datetime::DST_RULE_EUROPE, "Europe" },
	{ datetime::DST_RULE_USA, "USA" },
	{ datetime::DST_RULE_AUSTRALIA, "Australia" },
	{ 0, 0 }
};

////////////////////////////////////////////////////////////////////////////////

Value MakeValue(float value, Unit unit, int channelIndex, bool extendedPrecision) {
	int numSignificantDecimalDigits = getNumSignificantDecimalDigits(unit);
	if (extendedPrecision) {
		++numSignificantDecimalDigits;
	} else {
		if (
			channelIndex != -1 &&
			unit == UNIT_AMPER &&
			channel_dispatcher::isCurrentLowRangeAllowed(Channel::get(channelIndex)) &&
			mw::lessOrEqual(value, 0.5, getPrecision(UNIT_AMPER))
			)
		{
			++numSignificantDecimalDigits;
		}
	}
	return Value(value, unit, numSignificantDecimalDigits, extendedPrecision);
}

Value MakeValueListValue(const Value *values) {
	Value value;
	value.type_ = VALUE_TYPE_VALUE_LIST;
	value.options_ = 0;
	value.unit_ = UNIT_UNKNOWN;
	value.pValue_ = values;
	return value;
}

Value MakeFloatListValue(float *pFloat) {
	Value value;
	value.type_ = VALUE_TYPE_FLOAT_LIST;
	value.options_ = 0;
	value.unit_ = UNIT_UNKNOWN;
	value.pFloat_ = pFloat;
	return value;
}

Value MakeEventValue(event_queue::Event *e) {
	Value value;
	value.type_ = VALUE_TYPE_EVENT;
	value.options_ = 0;
	value.unit_ = UNIT_UNKNOWN;
	value.pVoid_ = e;
	return value;
}

Value MakePageInfoValue(uint8_t pageIndex, uint8_t numPages) {
	Value value;
	value.pairOfUint8_.first = pageIndex;
	value.pairOfUint8_.second = numPages;
	value.type_ = VALUE_TYPE_PAGE_INFO;
	return value;
}

Value MakeLessThenMinMessageValue(float float_, const Value& value_) {
	Value value;
	if (value_.getType() == VALUE_TYPE_INT) {
		value.int_ = int(float_);
		value.type_ = VALUE_TYPE_LESS_THEN_MIN_INT;
	} else if (value_.getType() == VALUE_TYPE_TIME_ZONE) {
		value.type_ = VALUE_TYPE_LESS_THEN_MIN_TIME_ZONE;
	} else {
		value.float_ = float_;
		value.unit_ = value_.getUnit();
		value.type_ = VALUE_TYPE_LESS_THEN_MIN_FLOAT;
	}
	return value;
}

Value MakeGreaterThenMaxMessageValue(float float_, const Value& value_) {
	Value value;
	if (value_.getType() == VALUE_TYPE_INT) {
		value.int_ = int(float_);
		value.type_ = VALUE_TYPE_GREATER_THEN_MAX_INT;
	} else if (value_.getType() == VALUE_TYPE_TIME_ZONE) {
		value.type_ = VALUE_TYPE_GREATER_THEN_MAX_TIME_ZONE;
	} else {
		value.float_ = float_;
		value.unit_ = value_.getUnit();
		value.type_ = VALUE_TYPE_GREATER_THEN_MAX_FLOAT;
	}
	return value;
}

Value MakeMacAddressValue(uint8_t* macAddress) {
	Value value;
	value.type_ = VALUE_TYPE_MAC_ADDRESS;
	value.puint8_ = macAddress;
	return value;
}

////////////////////////////////////////////////////////////////////////////////

void printTime(uint32_t time, char *text, int count) {
	int h = time / 3600;
	int r = time - h * 3600;
	int m = r / 60;
	int s = r - m * 60;

	if (h > 0) {
		snprintf(text, count - 1, "%dh %dm", h, m);
	} else if (m > 0) {
		snprintf(text, count - 1, "%dm %ds", m, s);
	} else {
		snprintf(text, count - 1, "%ds", s);
	}

	text[count - 1] = 0;
}

////////////////////////////////////////////////////////////////////////////////

event_queue::Event *getEventFromValue(const Value& value) {
	return (event_queue::Event *)value.getVoidPointer();
}

uint8_t getPageIndexFromValue(const Value& value) {
	return value.getFirstUInt8();
}

uint8_t getNumPagesFromValue(const Value& value) {
	return value.getSecondUInt8();
}

////////////////////////////////////////////////////////////////////////////////

bool compare_LESS_THEN_MIN_FLOAT_value(const Value& a, const Value&b) {
	return a.getUnit() == b.getUnit() && a.getFloat() == b.getFloat();
}

void LESS_THEN_MIN_FLOAT_value_to_text(const Value& value, char *text, int count) {
	char valueText[64];
	MakeValue(value.getFloat(), (Unit)value.getUnit()).toText(valueText, sizeof(text));
	snprintf(text, count - 1, "Value is less then %s", valueText);
	text[count - 1] = 0;
}

bool compare_GREATER_THEN_MAX_FLOAT_value(const Value& a, const Value&b) {
	return a.getUnit() == b.getUnit() && a.getFloat() == b.getFloat();
}

void GREATER_THEN_MAX_FLOAT_value_to_text(const Value& value, char *text, int count) {
	char valueText[64];
	MakeValue(value.getFloat(), (Unit)value.getUnit()).toText(valueText, sizeof(text));
	snprintf(text, count - 1, "Value is greater then %s", valueText);
	text[count - 1] = 0;
}

bool compare_CHANNEL_LABEL_value(const Value& a, const Value&b) {
	return a.getUInt8() == b.getUInt8();
}

void CHANNEL_LABEL_value_to_text(const Value& value, char *text, int count) {
	snprintf(text, count - 1, "Channel %d:", value.getUInt8());
	text[count - 1] = 0;
}

bool compare_CHANNEL_SHORT_LABEL_value(const Value& a, const Value&b) {
	return a.getUInt8() == b.getUInt8();
}

void CHANNEL_SHORT_LABEL_value_to_text(const Value& value, char *text, int count) {
	snprintf(text, count - 1, "Ch%d:", value.getUInt8());
	text[count - 1] = 0;
}

bool compare_CHANNEL_BOARD_INFO_LABEL_value(const Value& a, const Value&b) {
	return a.getInt() == b.getInt();
}

void CHANNEL_BOARD_INFO_LABEL_value_to_text(const Value& value, char *text, int count) {
	snprintf(text, count - 1, "CH%d board:", value.getInt());
	text[count - 1] = 0;
}

bool compare_LESS_THEN_MIN_INT_value(const Value& a, const Value&b) {
	return a.getInt() == b.getInt();
}

void LESS_THEN_MIN_INT_value_to_text(const Value& value, char *text, int count) {
	snprintf(text, count - 1, "Value is less then %d", value.getInt());
	text[count - 1] = 0;
}

bool compare_LESS_THEN_MIN_TIME_ZONE_value(const Value& a, const Value&b) {
	return true;
}

void LESS_THEN_MIN_TIME_ZONE_value_to_text(const Value& value, char *text, int count) {
	strncpy(text, "Value is less then -12:00", count - 1);
	text[count - 1] = 0;
}

bool compare_GREATER_THEN_MAX_INT_value(const Value& a, const Value&b) {
	return a.getInt() == b.getInt();
}

void GREATER_THEN_MAX_INT_value_to_text(const Value& value, char *text, int count) {
	snprintf(text, count - 1, "Value is greater then %d", value.getInt());
	text[count - 1] = 0;
}

bool compare_GREATER_THEN_MAX_TIME_ZONE_value(const Value& a, const Value&b) {
	return true;
}

void GREATER_THEN_MAX_TIME_ZONE_value_to_text(const Value& value, char *text, int count) {
	strncpy(text, "Value is greater then +14:00", count - 1);
	text[count - 1] = 0;
}

bool compare_EVENT_value(const Value& a, const Value&b) {
	return getEventFromValue(a)->dateTime == getEventFromValue(b)->dateTime &&
		getEventFromValue(a)->eventId == getEventFromValue(b)->eventId;
}

void EVENT_value_to_text(const Value& value, char *text, int count) {
	int year, month, day, hour, minute, second;
	datetime::breakTime(getEventFromValue(value)->dateTime, year, month, day, hour, minute, second);

	int yearNow, monthNow, dayNow, hourNow, minuteNow, secondNow;
	datetime::breakTime(datetime::now(), yearNow, monthNow, dayNow, hourNow, minuteNow, secondNow);

	if (yearNow == year && monthNow == month && dayNow == day) {
		snprintf(text, count - 1, "%c [%02d:%02d:%02d] %s",
			127 + event_queue::getEventType(getEventFromValue(value)),
			hour, minute, second,
			event_queue::getEventMessage(getEventFromValue(value)));
	} else {
		snprintf(text, count - 1, "%c [%02d-%02d-%02d] %s",
			127 + event_queue::getEventType(getEventFromValue(value)),
			day, month, year % 100,
			event_queue::getEventMessage(getEventFromValue(value)));
	}

	text[count - 1] = 0;
}

bool compare_PAGE_INFO_value(const Value& a, const Value&b) {
	return getPageIndexFromValue(a) == getPageIndexFromValue(b) && getNumPagesFromValue(a) == getNumPagesFromValue(b);
}

void PAGE_INFO_value_to_text(const Value& value, char *text, int count) {
	snprintf(text, count - 1, "Page #%d of %d", getPageIndexFromValue(value) + 1, getNumPagesFromValue(value));
	text[count - 1] = 0;
}

bool compare_ON_TIME_COUNTER_value(const Value& a, const Value&b) {
	return a.getUInt32() == b.getUInt32();
}

void ON_TIME_COUNTER_value_to_text(const Value& value, char *text, int count) {
	ontime::counterToString(text, count, value.getUInt32());
}

bool compare_COUNTDOWN_value(const Value& a, const Value&b) {
	return a.getUInt32() == b.getUInt32();
}

void COUNTDOWN_value_to_text(const Value& value, char *text, int count) {
	printTime(value.getUInt32(), text, count);
}

bool compare_TIME_ZONE_value(const Value& a, const Value&b) {
	return a.getInt16() == b.getInt16();
}

void TIME_ZONE_value_to_text(const Value& value, char *text, int count) {
	formatTimeZone(value.getInt16(), text, count);
}

bool compare_DATE_value(const Value& a, const Value&b) {
	return a.getUInt32() == b.getUInt32();
}

void DATE_value_to_text(const Value& value, char *text, int count) {
	int year, month, day, hour, minute, second;
	datetime::breakTime(value.getUInt32(), year, month, day, hour, minute, second);
	snprintf(text, count - 1, "%d - %02d - %02d", year, month, day);
	text[count - 1] = 0;
}

bool compare_YEAR_value(const Value& a, const Value&b) {
	return a.getUInt16() == b.getUInt16();
}

void YEAR_value_to_text(const Value& value, char *text, int count) {
	snprintf(text, count - 1, "%d", value.getUInt16());
	text[count - 1] = 0;
}

bool compare_MONTH_value(const Value& a, const Value&b) {
	return a.getUInt8() == b.getUInt8();
}

void MONTH_value_to_text(const Value& value, char *text, int count) {
	snprintf(text, count - 1, "%02d", value.getUInt8());
	text[count - 1] = 0;
}

bool compare_DAY_value(const Value& a, const Value&b) {
	return a.getUInt8() == b.getUInt8();
}

void DAY_value_to_text(const Value& value, char *text, int count) {
	snprintf(text, count - 1, "%02d", value.getUInt8());
	text[count - 1] = 0;
}

bool compare_TIME_value(const Value& a, const Value&b) {
	return a.getUInt32() == b.getUInt32();
}

void TIME_value_to_text(const Value& value, char *text, int count) {
	int year, month, day, hour, minute, second;
	datetime::breakTime(value.getUInt32(), year, month, day, hour, minute, second);
	snprintf(text, count - 1, "%02d : %02d : %02d", hour, minute, second);
	text[count - 1] = 0;
}

bool compare_HOUR_value(const Value& a, const Value&b) {
	return a.getUInt8() == b.getUInt8();
}

void HOUR_value_to_text(const Value& value, char *text, int count) {
	snprintf(text, count - 1, "%02d", value.getUInt8());
	text[count - 1] = 0;
}

bool compare_MINUTE_value(const Value& a, const Value&b) {
	return a.getUInt8() == b.getUInt8();
}

void MINUTE_value_to_text(const Value& value, char *text, int count) {
	snprintf(text, count - 1, "%02d", value.getUInt8());
	text[count - 1] = 0;
}

bool compare_SECOND_value(const Value& a, const Value&b) {
	return a.getUInt8() == b.getUInt8();
}

void SECOND_value_to_text(const Value& value, char *text, int count) {
	snprintf(text, count - 1, "%02d", value.getUInt8());
	text[count - 1] = 0;
}

bool compare_USER_PROFILE_LABEL_value(const Value& a, const Value&b) {
	return a.getInt() == b.getInt();
}

void USER_PROFILE_LABEL_value_to_text(const Value& value, char *text, int count) {
	snprintf(text, count - 1, "[ %d ]", value.getInt());
	text[count - 1] = 0;
}

bool compare_USER_PROFILE_REMARK_value(const Value& a, const Value&b) {
	return a.getInt() == b.getInt();
}

void USER_PROFILE_REMARK_value_to_text(const Value& value, char *text, int count) {
	profile::getName(value.getInt(), text, count);
}

bool compare_EDIT_INFO_value(const Value& a, const Value&b) {
	return a.getInt() == b.getInt();
}

void EDIT_INFO_value_to_text(const Value& value, char *text, int count) {
	edit_mode::getInfoText(value.getInt(), text);
}

bool compare_MAC_ADDRESS_value(const Value& a, const Value&b) {
	return memcmp(a.getPUint8(), b.getPUint8(), 6) == 0;
}

void MAC_ADDRESS_value_to_text(const Value& value, char *text, int count) {
	macAddressToString(value.getPUint8(), text);
}

bool compare_IP_ADDRESS_value(const Value& a, const Value&b) {
	return a.getUInt32() == b.getUInt32();
}

void IP_ADDRESS_value_to_text(const Value& value, char *text, int count) {
	ipAddressToString(value.getUInt32(), text);
}

bool compare_PORT_value(const Value& a, const Value&b) {
	return a.getUInt16() == b.getUInt16();
}

void PORT_value_to_text(const Value& value, char *text, int count) {
	snprintf(text, count - 1, "%d", value.getUInt16());
	text[count - 1] = 0;
}

bool compare_TEXT_MESSAGE_value(const Value& a, const Value&b) {
	return a.getUInt8() == b.getUInt8();
}

void TEXT_MESSAGE_value_to_text(const Value& value, char *text, int count) {
	strncpy(text, getTextMessage(), count - 1);
	text[count - 1] = 0;
}

bool compare_SERIAL_BAUD_INDEX_value(const Value& a, const Value&b) {
	return a.getInt() == b.getInt();
}

void SERIAL_BAUD_INDEX_value_to_text(const Value& value, char *text, int count) {
	snprintf(text, count - 1, "%ld", serial::g_bauds[value.getInt() - 1]);
	text[count - 1] = 0;
}

bool compare_PERCENTAGE_value(const Value& a, const Value&b) {
	return a.getInt() == b.getInt();
}

void PERCENTAGE_value_to_text(const Value& value, char *text, int count) {
	snprintf(text, count - 1, "%d%%", value.getInt());
	text[count - 1] = 0;
}

bool compare_SIZE_value(const Value& a, const Value&b) {
	return a.getUInt32() == b.getUInt32();
}

void SIZE_value_to_text(const Value& value, char *text, int count) {
	snprintf(text, count - 1, "%u", value.getUInt32());
	text[count - 1] = 0;
}

bool compare_DLOG_STATUS_value(const Value& a, const Value&b) {
	return a.getUInt32() == b.getUInt32();
}

void DLOG_STATUS_value_to_text(const Value& value, char *text, int count) {
	strcpy(text, "Dlog: ");
	printTime(value.getUInt32(), text + 6, count - 6);
}

bool compare_VALUE_LIST_value(const Value& a, const Value&b) {
	return a.getValueList() == b.getValueList();
}

void VALUE_LIST_value_to_text(const Value& value, char *text, int count) {
}

bool compare_FLOAT_LIST_value(const Value& a, const Value&b) {
	return a.getFloatList() == b.getFloatList();
}

void FLOAT_LIST_value_to_text(const Value& value, char *text, int count) {
}

////////////////////////////////////////////////////////////////////////////////

}
}
} // eez::app::gui

int g_x;

namespace eez {
namespace mw {
namespace gui {
namespace data {

CompareValueFunction g_compareUserValueFunctions[] = {
	compare_LESS_THEN_MIN_FLOAT_value,
	compare_GREATER_THEN_MAX_FLOAT_value,
	compare_CHANNEL_LABEL_value,
	compare_CHANNEL_SHORT_LABEL_value,
	compare_CHANNEL_BOARD_INFO_LABEL_value,
	compare_LESS_THEN_MIN_INT_value,
	compare_LESS_THEN_MIN_TIME_ZONE_value,
	compare_GREATER_THEN_MAX_INT_value,
	compare_GREATER_THEN_MAX_TIME_ZONE_value,
	compare_EVENT_value,
	compare_PAGE_INFO_value,
	compare_ON_TIME_COUNTER_value,
	compare_COUNTDOWN_value,
	compare_TIME_ZONE_value,
	compare_DATE_value,
	compare_YEAR_value,
	compare_MONTH_value,
	compare_DAY_value,
	compare_TIME_value,
	compare_HOUR_value,
	compare_MINUTE_value,
	compare_SECOND_value,
	compare_USER_PROFILE_LABEL_value,
	compare_USER_PROFILE_REMARK_value,
	compare_EDIT_INFO_value,
	compare_MAC_ADDRESS_value,
	compare_IP_ADDRESS_value,
	compare_PORT_value,
	compare_TEXT_MESSAGE_value,
	compare_SERIAL_BAUD_INDEX_value,
	compare_PERCENTAGE_value,
	compare_SIZE_value,
	compare_DLOG_STATUS_value,
	compare_VALUE_LIST_value,
	compare_FLOAT_LIST_value
};

ValueToTextFunction g_userValueToTextFunctions[] = {
	LESS_THEN_MIN_FLOAT_value_to_text,
	GREATER_THEN_MAX_FLOAT_value_to_text,
	CHANNEL_LABEL_value_to_text,
	CHANNEL_SHORT_LABEL_value_to_text,
	CHANNEL_BOARD_INFO_LABEL_value_to_text,
	LESS_THEN_MIN_INT_value_to_text,
	LESS_THEN_MIN_TIME_ZONE_value_to_text,
	GREATER_THEN_MAX_INT_value_to_text,
	GREATER_THEN_MAX_TIME_ZONE_value_to_text,
	EVENT_value_to_text,
	PAGE_INFO_value_to_text,
	ON_TIME_COUNTER_value_to_text,
	COUNTDOWN_value_to_text,
	TIME_ZONE_value_to_text,
	DATE_value_to_text,
	YEAR_value_to_text,
	MONTH_value_to_text,
	DAY_value_to_text,
	TIME_value_to_text,
	HOUR_value_to_text,
	MINUTE_value_to_text,
	SECOND_value_to_text,
	USER_PROFILE_LABEL_value_to_text,
	USER_PROFILE_REMARK_value_to_text,
	EDIT_INFO_value_to_text,
	MAC_ADDRESS_value_to_text,
	IP_ADDRESS_value_to_text,
	PORT_value_to_text,
	TEXT_MESSAGE_value_to_text,
	SERIAL_BAUD_INDEX_value_to_text,
	PERCENTAGE_value_to_text,
	SIZE_value_to_text,
	DLOG_STATUS_value_to_text,
	VALUE_LIST_value_to_text,
	FLOAT_LIST_value_to_text
};

}
}
}
} // eez::mw::gui::data

namespace eez {
namespace app {
namespace gui {

Value g_alertMessage;
Value g_alertMessage2;
Value g_alertMessage3;
Value g_progress;

char g_throbber[8] = { '|', '/', '-', '\\', '|', '/', '-', '\\' };

////////////////////////////////////////////////////////////////////////////////

static struct ChannelSnapshot {
	unsigned int mode;
	data::Value monValue;
	float pMon;
	uint32_t lastSnapshotTime;
} g_channelSnapshot[CH_MAX];

ChannelSnapshot& getChannelSnapshot(Channel& channel) {
	ChannelSnapshot &channelSnapshot = g_channelSnapshot[channel.index - 1];

	uint32_t currentTime = micros();
	if (!channelSnapshot.lastSnapshotTime || currentTime - channelSnapshot.lastSnapshotTime >= CONF_GUI_REFRESH_EVERY_MS * 1000UL) {
		const char *mode_str = channel.getCvModeStr();
		channelSnapshot.mode = 0;
		float uMon = channel_dispatcher::getUMon(channel);
		float iMon = channel_dispatcher::getIMon(channel);
		if (strcmp(mode_str, "CC") == 0) {
			channelSnapshot.monValue = MakeValue(uMon, UNIT_VOLT, channel.index - 1);
		} else if (strcmp(mode_str, "CV") == 0) {
			channelSnapshot.monValue = MakeValue(iMon, UNIT_AMPER, channel.index - 1);
		} else {
			channelSnapshot.mode = 1;
			if (uMon < iMon) {
				channelSnapshot.monValue = MakeValue(uMon, UNIT_VOLT, channel.index - 1);
			} else {
				channelSnapshot.monValue = MakeValue(iMon, UNIT_AMPER, channel.index - 1);
			}
		}
		channelSnapshot.pMon = multiply(uMon, iMon, getPrecision(UNIT_WATT));
		channelSnapshot.lastSnapshotTime = currentTime;
	}

	return channelSnapshot;
}

////////////////////////////////////////////////////////////////////////////////

Page *getPage(int pageId) {
	if (getActivePageId() == pageId) {
		return getActivePage();
	}
	if (getPreviousPageId() == pageId) {
		return getPreviousPage();
	}
	return NULL;
}

Page *getUserProfilesPage() {
	Page *page = getPage(PAGE_ID_USER_PROFILES);
	if (!page) {
		page = getPage(PAGE_ID_USER_PROFILES2);
	}
	if (!page) {
		page = getPage(PAGE_ID_USER_PROFILE_0_SETTINGS);
	}
	if (!page) {
		page = getPage(PAGE_ID_USER_PROFILE_SETTINGS);
	}
	return page;
}

}
}
} // eez::app::gui

////////////////////////////////////////////////////////////////////////////////

using namespace eez::app;

namespace eez {
namespace app {
namespace gui {

void data_edit_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);

		if ((channel_dispatcher::getVoltageTriggerMode(channel) != TRIGGER_MODE_FIXED && !trigger::isIdle()) || isPageActiveOrOnStack(PAGE_ID_CH_SETTINGS_LISTS)) {
			value = 0;
		}
		if (app::calibration::isEnabled()) {
			value = 0;
		}
		value = 1;
	}
}

void data_channels(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_COUNT) {
		value = CH_MAX;
	}
}

void data_channel_status(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		int channelStatus = channel.index > CH_NUM ? 0 : (channel.isOk() ? 1 : 2);
		value = channelStatus;
	}
}

void data_channel_output_state(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		value = channel.isOutputEnabled();
	}
}

void data_channel_output_mode(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		ChannelSnapshot &channelSnapshot = getChannelSnapshot(channel);
		value = (int)channelSnapshot.mode;
	}
}

void data_channel_mon_value(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		ChannelSnapshot &channelSnapshot = getChannelSnapshot(channel);
		value = channelSnapshot.monValue;
	}
}

void data_channel_u_set(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		value = MakeValue(channel_dispatcher::getUSet(channel), UNIT_VOLT, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_EDIT_VALUE) {
		value = MakeValue(channel_dispatcher::getUSetUnbalanced(channel), UNIT_VOLT, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_MIN) {
		value = MakeValue(channel_dispatcher::getUMin(channel), UNIT_VOLT, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_MAX) {
		value = MakeValue(channel_dispatcher::getUMax(channel), UNIT_VOLT, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_LIMIT) {
		value = MakeValue(channel_dispatcher::getULimit(channel), UNIT_VOLT, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_UNIT) {
		value = UNIT_VOLT;
	} else if (operation == data::DATA_OPERATION_SET) {
		if (!between(value.getFloat(), channel_dispatcher::getUMin(channel), channel_dispatcher::getUMax(channel), UNIT_VOLT, iChannel)) {
			value = MakeScpiErrorValue(SCPI_ERROR_DATA_OUT_OF_RANGE);
		} else if (greater(value.getFloat(), channel_dispatcher::getULimit(channel), UNIT_VOLT, iChannel)) {
			value = MakeScpiErrorValue(SCPI_ERROR_VOLTAGE_LIMIT_EXCEEDED);
		} else if (greater(value.getFloat() * channel_dispatcher::getISetUnbalanced(channel), channel_dispatcher::getPowerLimit(channel), UNIT_WATT, iChannel)) {
			value = MakeScpiErrorValue(SCPI_ERROR_POWER_LIMIT_EXCEEDED);
		} else {
			channel_dispatcher::setVoltage(channel, value.getFloat());
		}
	}
}

void data_channel_u_mon(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		value = MakeValue(channel_dispatcher::getUMon(channel), UNIT_VOLT, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_MIN) {
		value = MakeValue(channel_dispatcher::getUMin(channel), UNIT_VOLT, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_MAX) {
		value = MakeValue(channel_dispatcher::getUMax(channel), UNIT_VOLT, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_LIMIT) {
		value = MakeValue(channel_dispatcher::getULimit(channel), UNIT_VOLT, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_HISTORY_VALUE) {
		int position = value.getInt();
		value = MakeValue(channel_dispatcher::getUMonHistory(channel, position), UNIT_VOLT, iChannel);
	}
}

void data_channel_u_mon_dac(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		value = MakeValue(channel_dispatcher::getUMonDac(channel), UNIT_VOLT, iChannel);
	}
}

void data_channel_u_limit(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		value = MakeValue(channel_dispatcher::getULimit(channel), UNIT_VOLT, iChannel);
	}
}

void data_channel_u_edit(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		bool focused = (g_focusCursor == cursor || channel_dispatcher::isCoupled()) && g_focusDataId == DATA_ID_CHANNEL_U_EDIT;
		if (focused && g_focusEditValue.getType() != VALUE_TYPE_NONE) {
			value = g_focusEditValue;
		} else if (focused && getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD && edit_mode_keypad::g_keypad->isEditing()) {
			data_keypad_text(operation, cursor, value);
		} else {
			value = MakeValue(channel_dispatcher::getUSet(channel), UNIT_VOLT, iChannel);
		}
	} else if (operation == data::DATA_OPERATION_GET_EDIT_VALUE) {
		value = MakeValue(channel_dispatcher::getUSetUnbalanced(channel), UNIT_VOLT, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_MIN) {
		value = MakeValue(channel_dispatcher::getUMin(channel), UNIT_VOLT, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_MAX) {
		value = MakeValue(channel_dispatcher::getUMax(channel), UNIT_VOLT, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_LIMIT) {
		value = MakeValue(channel_dispatcher::getULimit(channel), UNIT_VOLT, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_UNIT) {
		value = UNIT_VOLT;
	} else if (operation == data::DATA_OPERATION_SET) {
		if (!between(value.getFloat(), channel_dispatcher::getUMin(channel), channel_dispatcher::getUMax(channel), UNIT_VOLT, iChannel)) {
			value = MakeScpiErrorValue(SCPI_ERROR_DATA_OUT_OF_RANGE);
		} else if (greater(value.getFloat(), channel_dispatcher::getULimit(channel), UNIT_VOLT, iChannel)) {
			value = MakeScpiErrorValue(SCPI_ERROR_VOLTAGE_LIMIT_EXCEEDED);
		} else if (greater(value.getFloat() * channel_dispatcher::getISetUnbalanced(channel), channel_dispatcher::getPowerLimit(channel), UNIT_WATT, iChannel)) {
			value = MakeScpiErrorValue(SCPI_ERROR_POWER_LIMIT_EXCEEDED);
		} else {
			channel_dispatcher::setVoltage(channel, value.getFloat());
		}
	}
}

void data_channel_i_set(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		value = MakeValue(channel_dispatcher::getISet(channel), UNIT_AMPER, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_EDIT_VALUE) {
		value = MakeValue(channel_dispatcher::getISetUnbalanced(channel), UNIT_AMPER, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_MIN) {
		value = MakeValue(channel_dispatcher::getIMin(channel), UNIT_AMPER, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_MAX) {
		value = MakeValue(channel_dispatcher::getIMax(channel), UNIT_AMPER, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_LIMIT) {
		value = MakeValue(channel_dispatcher::getILimit(channel), UNIT_AMPER, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_UNIT) {
		value = UNIT_AMPER;
	} else if (operation == data::DATA_OPERATION_SET) {
		if (!between(value.getFloat(), channel_dispatcher::getIMin(channel), channel_dispatcher::getIMax(channel), UNIT_AMPER, iChannel)) {
			value = MakeScpiErrorValue(SCPI_ERROR_DATA_OUT_OF_RANGE);
		} else if (greater(value.getFloat(), channel_dispatcher::getILimit(channel), UNIT_AMPER, iChannel)) {
			value = MakeScpiErrorValue(SCPI_ERROR_CURRENT_LIMIT_EXCEEDED);
		} else if (greater(value.getFloat() * channel_dispatcher::getUSetUnbalanced(channel), channel_dispatcher::getPowerLimit(channel), UNIT_AMPER, iChannel)) {
			value = MakeScpiErrorValue(SCPI_ERROR_POWER_LIMIT_EXCEEDED);
		} else {
			channel_dispatcher::setCurrent(channel, value.getFloat());
		}
	}
}

void data_channel_i_mon(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		value = MakeValue(channel_dispatcher::getIMon(channel), UNIT_AMPER, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_MIN) {
		value = MakeValue(channel_dispatcher::getIMin(channel), UNIT_AMPER, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_MAX) {
		value = MakeValue(channel_dispatcher::getIMax(channel), UNIT_AMPER, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_LIMIT) {
		value = MakeValue(channel_dispatcher::getILimit(channel), UNIT_AMPER, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_HISTORY_VALUE) {
		int position = value.getInt();
		value = MakeValue(channel_dispatcher::getIMonHistory(channel, position), UNIT_AMPER, iChannel);
	}
}

void data_channel_i_mon_dac(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		value = MakeValue(channel_dispatcher::getIMonDac(channel), UNIT_AMPER, iChannel);
	}
}

void data_channel_i_limit(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		value = MakeValue(channel_dispatcher::getILimit(channel), UNIT_AMPER, iChannel);
	}
}

void data_channel_i_edit(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		bool focused = (g_focusCursor == cursor || channel_dispatcher::isCoupled()) && g_focusDataId == DATA_ID_CHANNEL_I_EDIT;
		if (focused && g_focusEditValue.getType() != VALUE_TYPE_NONE) {
			value = g_focusEditValue;
		} else if (focused && getActivePageId() == PAGE_ID_EDIT_MODE_KEYPAD && edit_mode_keypad::g_keypad->isEditing()) {
			data_keypad_text(operation, cursor, value);
		} else {
			value = MakeValue(channel_dispatcher::getISet(channel), UNIT_AMPER, iChannel);
		}
	} else if (operation == data::DATA_OPERATION_GET_EDIT_VALUE) {
		value = MakeValue(channel_dispatcher::getISetUnbalanced(channel), UNIT_AMPER, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_MIN) {
		value = MakeValue(channel_dispatcher::getIMin(channel), UNIT_AMPER, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_MAX) {
		value = MakeValue(channel_dispatcher::getIMax(channel), UNIT_AMPER, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_LIMIT) {
		value = MakeValue(channel_dispatcher::getILimit(channel), UNIT_AMPER, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_UNIT) {
		value = UNIT_AMPER;
	} else if (operation == data::DATA_OPERATION_SET) {
		if (!between(value.getFloat(), channel_dispatcher::getIMin(channel), channel_dispatcher::getIMax(channel), UNIT_AMPER, iChannel)) {
			value = MakeScpiErrorValue(SCPI_ERROR_DATA_OUT_OF_RANGE);
		} else if (greater(value.getFloat(), channel_dispatcher::getILimit(channel), UNIT_AMPER, iChannel)) {
			value = MakeScpiErrorValue(SCPI_ERROR_CURRENT_LIMIT_EXCEEDED);
		} else if (greater(value.getFloat() * channel_dispatcher::getUSetUnbalanced(channel), channel_dispatcher::getPowerLimit(channel), UNIT_AMPER, iChannel)) {
			value = MakeScpiErrorValue(SCPI_ERROR_POWER_LIMIT_EXCEEDED);
		} else {
			channel_dispatcher::setCurrent(channel, value.getFloat());
		}
	}
}

void data_channel_p_mon(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		value = MakeValue(getChannelSnapshot(channel).pMon, UNIT_WATT, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_EDIT_VALUE) {
	} else if (operation == data::DATA_OPERATION_GET_MIN) {
		value = MakeValue(channel_dispatcher::getPowerMinLimit(channel), UNIT_WATT, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_MAX) {
		value = MakeValue(channel_dispatcher::getPowerMaxLimit(channel), UNIT_WATT, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_LIMIT) {
		value = MakeValue(channel_dispatcher::getPowerLimit(channel), UNIT_WATT, iChannel);
	} else if (operation == data::DATA_OPERATION_GET_UNIT) {
	} else if (operation == data::DATA_OPERATION_GET_HISTORY_VALUE) {
		float pMon = multiply(
			channel_dispatcher::getUMonHistory(channel, value.getInt()),
			channel_dispatcher::getIMonHistory(channel, value.getInt()),
			getPrecision(UNIT_WATT));
		value = MakeValue(pMon, UNIT_WATT, iChannel);
	}
}

void data_channels_view_mode(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = (int)persist_conf::devConf.flags.channelsViewMode;
	}
}

void data_channel_display_value1(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (channel.flags.displayValue1 == DISPLAY_VALUE_VOLTAGE) {
		data_channel_u_mon(operation, cursor, value);
	} else if (channel.flags.displayValue1 == DISPLAY_VALUE_CURRENT) {
		data_channel_i_mon(operation, cursor, value);
	} else if (channel.flags.displayValue1 == DISPLAY_VALUE_POWER) {
		data_channel_p_mon(operation, cursor, value);
	}
}

void data_channel_display_value2(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (channel.flags.displayValue2 == DISPLAY_VALUE_VOLTAGE) {
		data_channel_u_mon(operation, cursor, value);
	} else if (channel.flags.displayValue2 == DISPLAY_VALUE_CURRENT) {
		data_channel_i_mon(operation, cursor, value);
	} else if (channel.flags.displayValue2 == DISPLAY_VALUE_POWER) {
		data_channel_p_mon(operation, cursor, value);
	}
}

void data_lrip(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		value = (int)channel.flags.lrippleEnabled;
	}
}

void data_ovp(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		if (!channel.prot_conf.flags.u_state) {
			value = 0;
		} else if (!channel_dispatcher::isOvpTripped(channel)) {
			value = 1;
		} else {
			value = 2;
		}
	}
}

void data_ocp(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		if (!channel.prot_conf.flags.i_state) {
			value = 0;
		} else if (!channel_dispatcher::isOcpTripped(channel)) {
			value = 1;
		} else {
			value = 2;
		}
	}
}

void data_opp(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		if (!channel.prot_conf.flags.p_state) {
			value = 0;
		} else if (!channel_dispatcher::isOppTripped(channel)) {
			value = 1;
		} else {
			value = 2;
		}
	}
}

void data_otp_ch(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::CH1 + iChannel];
		if (!tempSensor.isInstalled() || !tempSensor.isTestOK() || !tempSensor.prot_conf.state) {
			value = 0;
		} else if (!channel_dispatcher::isOtpTripped(channel)) {
			value = 1;
		} else {
			value = 2;
		}
	}
#endif
}

void data_otp_aux(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::AUX];
		if (!tempSensor.prot_conf.state) {
			value = 0;
		} else if (!tempSensor.isTripped()) {
			value = 1;
		} else {
			value = 2;
		}
	}
}

void data_alert_message(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = gui::g_alertMessage;
	} else if (operation == data::DATA_OPERATION_SET) {
		gui::g_alertMessage = value;
	}
}

void data_alert_message_2(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = gui::g_alertMessage2;
	} else if (operation == data::DATA_OPERATION_SET) {
		gui::g_alertMessage2 = value;
	}
}

void data_alert_message_3(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = gui::g_alertMessage3;
	} else if (operation == data::DATA_OPERATION_SET) {
		gui::g_alertMessage3 = value;
	}
}

void data_edit_value(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = edit_mode::getEditValue();
	} else if (operation == data::DATA_OPERATION_GET_MIN) {
		value = edit_mode::getMin();
	} else if (operation == data::DATA_OPERATION_GET_MAX) {
		value = edit_mode::getMax();
	} else if (operation == data::DATA_OPERATION_IS_BLINKING) {
		value = !edit_mode::isInteractiveMode() && (edit_mode::getEditValue() != edit_mode::getCurrentValue());
	}
}

void data_edit_unit(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		Keypad *keypad = getActiveKeypad();
		if (keypad) {
			value = getUnitName(keypad->getSwitchToUnit());
		}
	}
}

void data_edit_info(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int editInfoPartOffset = g_focusCursor.i * 6 + (g_focusDataId == DATA_ID_CHANNEL_U_EDIT ? 0 : 1) * 3;
		value = data::Value(0 + editInfoPartOffset, VALUE_TYPE_EDIT_INFO);
	}
}

void data_edit_info1(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int editInfoPartOffset = g_focusCursor.i * 6 + (g_focusDataId == DATA_ID_CHANNEL_U_EDIT ? 0 : 1) * 3;
		value = data::Value(1 + editInfoPartOffset, VALUE_TYPE_EDIT_INFO);
	}
}

void data_edit_info2(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int editInfoPartOffset = g_focusCursor.i * 6 + (g_focusDataId == DATA_ID_CHANNEL_U_EDIT ? 0 : 1) * 3;
		value = data::Value(2 + editInfoPartOffset, VALUE_TYPE_EDIT_INFO);
	}
}

void data_edit_mode_interactive_mode_selector(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = edit_mode::isInteractiveMode() ? 0 : 1;
	}
}

void data_edit_steps(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = edit_mode_step::getStepIndex();
	} else if (operation == data::DATA_OPERATION_GET_VALUE_LIST) {
		value = MakeValueListValue(edit_mode_step::getStepValues());
	} else if (operation == data::DATA_OPERATION_COUNT) {
		value = edit_mode_step::getStepValuesCount();
	} else if (operation == data::DATA_OPERATION_SET) {
		edit_mode_step::setStepIndex(value.getInt());
	}
}

void data_model_info(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		/*
		We are auto generating model name from the channels definition:

		<cnt>/<volt>/<curr>[-<cnt2>/<volt2>/<curr2>] (<platform>)

		Where is:

		<cnt>      - number of the equivalent channels
		<volt>     - max. voltage
		<curr>     - max. curr
		<platform> - Mega, Due, Simulator or Unknown
		*/
		static char model_info[CH_NUM * (sizeof("XX V / XX A") - 1) + CH_NUM * (sizeof(" - ") - 1) + 1];

		if (*model_info == 0) {
			char *p = Channel::getChannelsInfoShort(model_info);
			*p = 0;
		}

		value = model_info;
	}
}

void data_firmware_info(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		static const char FIRMWARE_LABEL[] = "Firmware: ";
		static char firmware_info[sizeof(FIRMWARE_LABEL) - 1 + sizeof(FIRMWARE) - 1 + 1];

		if (*firmware_info == 0) {
			strcat(firmware_info, FIRMWARE_LABEL);
			strcat(firmware_info, FIRMWARE);
		}

		value = firmware_info;
	}
}

void data_self_test_result(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SelfTestResultPage *page = (SelfTestResultPage *)getPage(PAGE_ID_SELF_TEST_RESULT);
		if (page) {
			value = page->m_selfTestResult;
		}
	}
}

void data_keypad_text(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		Keypad *keypad = getActiveKeypad();
		if (keypad) {
			value = keypad->getKeypadTextValue();
		}
	}
}

void data_keypad_caps(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		Keypad *keypad = getActiveKeypad();
		if (keypad) {
			value = keypad->m_isUpperCase;
		}
	}
}

void data_keypad_option1_text(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		NumericKeypad *page = (NumericKeypad *)getPage(PAGE_ID_NUMERIC_KEYPAD);
		if (page) {
			value = data::Value(page->m_options.flags.option1ButtonEnabled ? page->m_options.option1ButtonText : "");
		}
	}
}

void data_keypad_option1_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		NumericKeypad *page = (NumericKeypad *)getPage(PAGE_ID_NUMERIC_KEYPAD);
		if (page) {
			value = (int)page->m_options.flags.option1ButtonEnabled;
		}
	}
}

void data_keypad_option2_text(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		NumericKeypad *page = (NumericKeypad *)getPage(PAGE_ID_NUMERIC_KEYPAD);
		if (page) {
			value = data::Value(page->m_options.flags.option2ButtonEnabled ? page->m_options.option2ButtonText : "");
		}
	}
}

void data_keypad_option2_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		NumericKeypad *page = (NumericKeypad *)getPage(PAGE_ID_NUMERIC_KEYPAD);
		if (page) {
			value = (int)page->m_options.flags.option2ButtonEnabled;
		}
	}
}

void data_keypad_sign_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		NumericKeypad *page = (NumericKeypad *)getPage(PAGE_ID_NUMERIC_KEYPAD);
		if (page) {
			value = (int)page->m_options.flags.signButtonEnabled;
		}
	}
}

void data_keypad_dot_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		NumericKeypad *page = (NumericKeypad *)getPage(PAGE_ID_NUMERIC_KEYPAD);
		if (page) {
			value = (int)page->m_options.flags.dotButtonEnabled;
		}
	}
}

void data_keypad_unit_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		NumericKeypad *page = (NumericKeypad *)getPage(PAGE_ID_NUMERIC_KEYPAD);
		if (page) {
			value =
				page->m_options.editValueUnit == UNIT_VOLT ||
				page->m_options.editValueUnit == UNIT_MILLI_VOLT ||
				page->m_options.editValueUnit == UNIT_AMPER ||
				page->m_options.editValueUnit == UNIT_MILLI_AMPER ||
				page->m_options.editValueUnit == UNIT_WATT ||
				page->m_options.editValueUnit == UNIT_MILLI_WATT ||
				page->m_options.editValueUnit == UNIT_SECOND ||
				page->m_options.editValueUnit == UNIT_MILLI_SECOND;
		}
	}
}

void data_channel_label(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		value = data::Value(iChannel + 1, VALUE_TYPE_CHANNEL_LABEL);
	}
}

void data_channel_short_label(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		value =  data::Value(iChannel + 1, VALUE_TYPE_CHANNEL_SHORT_LABEL);
	}
}

void data_channel_temp_status(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9
		value = 2;
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::CH1 + iChannel];
		if (tempSensor.isInstalled()) {
			if (tempSensor.isTestOK()) {
				value = 1;
			} else {
				value = 0;
			}
		} else {
			value = 2;
		}
#endif
	}
}

void data_channel_temp(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		float temperature = 0;
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::CH1 + iChannel];
		if (tempSensor.isInstalled() && tempSensor.isTestOK()) {
			temperature = tempSensor.temperature;
		}
#endif
		value = MakeValue(temperature, UNIT_CELSIUS);
	}
}

void data_channel_on_time_total(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value((uint32_t)channel.onTimeCounter.getTotalTime(), VALUE_TYPE_ON_TIME_COUNTER);
	}
}

void data_channel_on_time_last(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value((uint32_t)channel.onTimeCounter.getLastTime(), VALUE_TYPE_ON_TIME_COUNTER);
	}
}

void data_channel_calibration_status(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		value = channel.isCalibrationExists();
	}
}

void data_channel_calibration_state(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		value = channel.isCalibrationEnabled();
	}
}

void data_channel_calibration_date(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		value = data::Value(channel.cal_conf.calibration_date);
	}
}

void data_channel_calibration_remark(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		value = data::Value(channel.cal_conf.calibration_remark);
	}
}

void data_channel_calibration_step_is_set_remark_step(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = gui::calibration_wizard::g_stepNum == gui::calibration_wizard::MAX_STEP_NUM - 1;
	}
}

void data_channel_calibration_step_num(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(gui::calibration_wizard::g_stepNum);
	}
}

void data_channel_calibration_step_status(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		switch (gui::calibration_wizard::g_stepNum) {
		case 0: value = app::calibration::getVoltage().min_set; break;
		case 1: value = app::calibration::getVoltage().mid_set; break;
		case 2: value = app::calibration::getVoltage().max_set; break;
		case 3: case 6: value = app::calibration::getCurrent().min_set; break;
		case 4: case 7: value = app::calibration::getCurrent().mid_set; break;
		case 5: case 8: value = app::calibration::getCurrent().max_set; break;
		case 9: value = app::calibration::isRemarkSet(); break;
		}
	}
}

void data_channel_calibration_step_level_value(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = gui::calibration_wizard::getLevelValue();
	}
}

void data_channel_calibration_step_value(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		switch (gui::calibration_wizard::g_stepNum) {
		case 0: value = MakeValue(app::calibration::getVoltage().min_val, UNIT_VOLT, iChannel, true); break;
		case 1: value = MakeValue(app::calibration::getVoltage().mid_val, UNIT_VOLT, iChannel, true); break;
		case 2: value = MakeValue(app::calibration::getVoltage().max_val, UNIT_VOLT, iChannel, true); break;
		case 3: value = MakeValue(app::calibration::getCurrent().min_val, UNIT_AMPER, iChannel, true); break;
		case 4: value = MakeValue(app::calibration::getCurrent().mid_val, UNIT_AMPER, iChannel, true); break;
		case 5: value = MakeValue(app::calibration::getCurrent().max_val, UNIT_AMPER, iChannel, true); break;
		case 6: value = MakeValue(app::calibration::getCurrent().min_val, UNIT_AMPER, iChannel, true); break;
		case 7: value = MakeValue(app::calibration::getCurrent().mid_val, UNIT_AMPER, iChannel, true); break;
		case 8: value = MakeValue(app::calibration::getCurrent().max_val, UNIT_AMPER, iChannel, true); break;
		case 9: value = data::Value(app::calibration::getRemark()); break;
		}
	}
}

void data_channel_calibration_step_prev_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = gui::calibration_wizard::g_stepNum > 0;
	}
}

void data_channel_calibration_step_next_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = gui::calibration_wizard::g_stepNum < gui::calibration_wizard::MAX_STEP_NUM;
	}
}

void data_cal_ch_u_min(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		value = MakeValue(channel.cal_conf.u.min.val, UNIT_VOLT, channel.index - 1);
	}
}

void data_cal_ch_u_mid(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		value = MakeValue(channel.cal_conf.u.mid.val, UNIT_VOLT, channel.index - 1);
	}
}

void data_cal_ch_u_max(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		value = MakeValue(channel.cal_conf.u.max.val, UNIT_VOLT, channel.index - 1);
	}
}

void data_cal_ch_i0_min(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		value = MakeValue(channel.cal_conf.i[0].min.val, UNIT_AMPER, channel.index - 1);
	}
}

void data_cal_ch_i0_mid(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		value = MakeValue(channel.cal_conf.i[0].mid.val, UNIT_AMPER, channel.index - 1);
	}
}

void data_cal_ch_i0_max(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		value = MakeValue(channel.cal_conf.i[0].max.val, UNIT_AMPER, channel.index - 1);
	}
}

void data_cal_ch_i1_min(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		if (channel.hasSupportForCurrentDualRange()) {
			value = MakeValue(channel.cal_conf.i[1].min.val, UNIT_AMPER, channel.index - 1);
		} else {
			value = data::Value("");
		}
	}
}


void data_cal_ch_i1_mid(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		if (channel.hasSupportForCurrentDualRange()) {
			value = MakeValue(channel.cal_conf.i[1].mid.val, UNIT_AMPER, channel.index - 1);
		} else {
			value = data::Value("");
		}
	}
}

void data_cal_ch_i1_max(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		if (channel.hasSupportForCurrentDualRange()) {
			value = MakeValue(channel.cal_conf.i[1].max.val, UNIT_AMPER, channel.index - 1);
		} else {
			value = data::Value("");
		}
	}
}

void data_channel_protection_ovp_state(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getPage(PAGE_ID_CH_SETTINGS_PROT_OVP);
		if (page) {
			value = page->state;
		}
	}
}

void data_channel_protection_ovp_level(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getPage(PAGE_ID_CH_SETTINGS_PROT_OVP);
		if (page) {
			value = page->level;
		}
	}
}

void data_channel_protection_ovp_delay(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getPage(PAGE_ID_CH_SETTINGS_PROT_OVP);
		if (page) {
			value = page->delay;
		}
	}
}

void data_channel_protection_ovp_limit(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getPage(PAGE_ID_CH_SETTINGS_PROT_OVP);
		if (page) {
			value = page->limit;
		}
	}
}

void data_channel_protection_ocp_state(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getPage(PAGE_ID_CH_SETTINGS_PROT_OCP);
		if (page) {
			value = page->state;
		}
	}
}

void data_channel_protection_ocp_delay(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getPage(PAGE_ID_CH_SETTINGS_PROT_OCP);
		if (page) {
			value = page->delay;
		}
	}
}

void data_channel_protection_ocp_limit(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getPage(PAGE_ID_CH_SETTINGS_PROT_OCP);
		if (page) {
			value = page->limit;
		}
	}
}

void data_channel_protection_ocp_max_current_limit_cause(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(g_channel->getMaxCurrentLimitCause());
	}
}

void data_channel_protection_opp_state(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getPage(PAGE_ID_CH_SETTINGS_PROT_OPP);
		if (page) {
			value = page->state;
		}
	}
}

void data_channel_protection_opp_level(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getPage(PAGE_ID_CH_SETTINGS_PROT_OPP);
		if (page) {
			value = page->level;
		}
	}
}

void data_channel_protection_opp_delay(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getPage(PAGE_ID_CH_SETTINGS_PROT_OPP);
		if (page) {
			value = page->delay;
		}
	}
}

void data_channel_protection_opp_limit(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getPage(PAGE_ID_CH_SETTINGS_PROT_OPP);
		if (page) {
			value = page->limit;
		}
	}
}

void data_channel_protection_otp_installed(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = temperature::isChannelSensorInstalled(g_channel);
	}
}

void data_channel_protection_otp_state(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getPage(PAGE_ID_CH_SETTINGS_PROT_OTP);
		if (page) {
			value = page->state;
		}
	}
}

void data_channel_protection_otp_level(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getPage(PAGE_ID_CH_SETTINGS_PROT_OTP);
		if (page) {
			value = page->level;
		}
	}
}

void data_channel_protection_otp_delay(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsProtectionSetPage *page = (ChSettingsProtectionSetPage *)getPage(PAGE_ID_CH_SETTINGS_PROT_OTP);
		if (page) {
			value = page->delay;
		}
	}
}

static event_queue::Event g_lastEvent[2];

void data_event_queue_last_event_type(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		event_queue::Event *lastEvent = &g_lastEvent[getCurrentStateBufferIndex()];
		event_queue::getLastErrorEvent(lastEvent);
		value = data::Value(event_queue::getEventType(lastEvent));
	}
}

void data_event_queue_last_event_message(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		event_queue::Event *lastEvent = &g_lastEvent[getCurrentStateBufferIndex()];
		event_queue::getLastErrorEvent(lastEvent);
		if (event_queue::getEventType(lastEvent) != event_queue::EVENT_TYPE_NONE) {
			value = MakeEventValue(lastEvent);
		}
	}
}

void data_event_queue_events(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_COUNT) {
		value = event_queue::EVENTS_PER_PAGE;
	}
}

event_queue::Event g_stateEvents[2][event_queue::EVENTS_PER_PAGE];

void data_event_queue_events_type(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		if (cursor.i >= 0) {
			event_queue::Event *event = &g_stateEvents[getCurrentStateBufferIndex()][cursor.i];

			int n = event_queue::getActivePageNumEvents();
			if (cursor.i < n) {
				event_queue::getActivePageEvent(cursor.i, event);
			} else {
				event->eventId = event_queue::EVENT_TYPE_NONE;
			}

			value = data::Value(event_queue::getEventType(event));
		}
	}
}

void data_event_queue_events_message(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		if (cursor.i >= 0) {
			event_queue::Event *event = &g_stateEvents[getCurrentStateBufferIndex()][cursor.i];

			int n = event_queue::getActivePageNumEvents();
			if (cursor.i < n) {
				event_queue::getActivePageEvent(cursor.i, event);
			} else {
				event->eventId = event_queue::EVENT_TYPE_NONE;
			}

			value = MakeEventValue(event);
		}
	}
}

void data_event_queue_multiple_pages(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		data::Value eventQueuePageInfo = MakePageInfoValue(event_queue::getActivePageIndex(), event_queue::getNumPages());
		value = getNumPagesFromValue(eventQueuePageInfo) > 1;
	}

}

void data_event_queue_previous_page_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		data::Value eventQueuePageInfo = MakePageInfoValue(event_queue::getActivePageIndex(), event_queue::getNumPages());
		value = getPageIndexFromValue(eventQueuePageInfo) > 0;
	}

}

void data_event_queue_next_page_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		data::Value eventQueuePageInfo = MakePageInfoValue(event_queue::getActivePageIndex(), event_queue::getNumPages());
		value = getPageIndexFromValue(eventQueuePageInfo) < getNumPagesFromValue(eventQueuePageInfo) - 1;
	}

}

void data_event_queue_page_info(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = MakePageInfoValue(event_queue::getActivePageIndex(), event_queue::getNumPages());
	}
}

void data_channel_lripple_max_dissipation(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = MakeValue(g_channel->SOA_POSTREG_PTOT, UNIT_WATT, g_channel->index - 1);
	}

}

void data_channel_lripple_calculated_dissipation(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		Channel &channel = Channel::get(g_channel->index - 1);
		value = MakeValue(channel_dispatcher::getIMon(channel) * (g_channel->SOA_VIN - channel_dispatcher::getUMon(channel)), UNIT_WATT, g_channel->index - 1);
	}

}

void data_channel_lripple_auto_mode(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsAdvLRipplePage *page = (ChSettingsAdvLRipplePage *)getPage(PAGE_ID_CH_SETTINGS_ADV_LRIPPLE);
		if (page) {
			value = page->autoMode;
		}
	}

}

void data_channel_lripple_is_allowed(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = g_channel->isLowRippleAllowed(micros());
	}

}

void data_channel_lripple_status(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsAdvLRipplePage *page = (ChSettingsAdvLRipplePage *)getPage(PAGE_ID_CH_SETTINGS_ADV_LRIPPLE);
		if (page) {
			value = g_channel->isLowRippleAllowed(micros()) ? page->status : false;
		}
	}
}

void data_channel_rsense_status(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = g_channel->isRemoteSensingEnabled();
	}
}

void data_channel_rprog_installed(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = g_channel->getFeatures() & CH_FEATURE_RPROG ? 1 : 0;
	}
}

void data_channel_rprog_status(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	if (operation == data::DATA_OPERATION_GET) {
		value = (int)channel.flags.rprogEnabled;
	}
}

void data_channel_is_coupled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = channel_dispatcher::isCoupled();
	}
}

void data_channel_is_tracked(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = channel_dispatcher::isTracked();
	}
}

void data_channel_is_coupled_or_tracked(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = channel_dispatcher::isCoupled() || channel_dispatcher::isTracked();
	}
}

void data_channel_coupling_is_allowed(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = channel_dispatcher::isCouplingOrTrackingAllowed();
	}
}

void data_channel_coupling_mode(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(channel_dispatcher::getType());
	} else if (operation == data::DATA_OPERATION_SELECT) {
		cursor.i = 0;
	}
}

void data_channel_coupling_selected_mode(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = ChSettingsAdvCouplingPage::selectedMode;
	}
}

void data_channel_coupling_is_series(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = channel_dispatcher::isSeries();
	}
}

void data_sys_on_time_total(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value((uint32_t)g_powerOnTimeCounter.getTotalTime(), VALUE_TYPE_ON_TIME_COUNTER);
	}
}

void data_sys_on_time_last(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value((uint32_t)g_powerOnTimeCounter.getLastTime(), VALUE_TYPE_ON_TIME_COUNTER);
	}
}

void data_sys_temp_aux_status(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::AUX];
		if (tempSensor.isInstalled()) {
			if (tempSensor.isTestOK()) {
				value = 1;
			} else {
				value = 0;
			}
		} else {
			value = 2;
		}
	}
}

void data_sys_temp_aux_otp_state(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsAuxOtpPage *page = (SysSettingsAuxOtpPage *)getPage(PAGE_ID_SYS_SETTINGS_AUX_OTP);
		if (page) {
			value = page->state;
		}
	}
}

void data_sys_temp_aux_otp_level(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsAuxOtpPage *page = (SysSettingsAuxOtpPage *)getPage(PAGE_ID_SYS_SETTINGS_AUX_OTP);
		if (page) {
			value = page->level;
		}
	}
}

void data_sys_temp_aux_otp_delay(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsAuxOtpPage *page = (SysSettingsAuxOtpPage *)getPage(PAGE_ID_SYS_SETTINGS_AUX_OTP);
		if (page) {
			value = page->delay;
		}
	}
}

void data_sys_temp_aux_otp_is_tripped(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = temperature::sensors[temp_sensor::AUX].isTripped();
	}
}

void data_sys_temp_aux(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		float auxTemperature = 0;
		temperature::TempSensorTemperature &tempSensor = temperature::sensors[temp_sensor::AUX];
		if (tempSensor.isInstalled() && tempSensor.isTestOK()) {
			auxTemperature = tempSensor.temperature;
		}
		value = MakeValue(auxTemperature, UNIT_CELSIUS);
	}
}

void data_sys_info_firmware_ver(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(FIRMWARE);
	}
}

void data_sys_info_serial_no(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(persist_conf::devConf.serialNumber);
	}
}

void data_sys_info_scpi_ver(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(SCPI_STD_VERSION_REVISION);
	}
}

void data_sys_info_cpu(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(getCpuModel());
	}
}

void data_sys_info_ethernet(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(getCpuEthernetType());
	}
}

void data_sys_info_fan_status(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R1B9 || !OPTION_FAN
		value = 3;
#elif EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
		if (fan::g_testResult == TEST_FAILED || fan::g_testResult == TEST_WARNING) {
			value = 0;
		} else if (fan::g_testResult == TEST_OK) {
#if FAN_OPTION_RPM_MEASUREMENT
			value = 1;
#else
			value = 2;
#endif
		} else {
			value = 3;
		}
#endif
	}
}

void data_sys_info_fan_speed(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
#if (EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R3B4 || EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12) && FAN_OPTION_RPM_MEASUREMENT
	if (operation == data::DATA_OPERATION_GET) {
		if (fan::g_testResult == TEST_OK) {
			value = MakeValue((float)fan::g_rpm, UNIT_RPM);
		}
	}
#endif
}

void data_channel_board_info_label(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		if (cursor.i >= 0 && cursor.i < CH_NUM) {
			value = data::Value(cursor.i + 1, VALUE_TYPE_CHANNEL_BOARD_INFO_LABEL);
		}
	}
}

void data_channel_board_info_revision(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		if (cursor.i >= 0 && cursor.i < CH_NUM) {
			value = data::Value(Channel::get(cursor.i).getBoardRevisionName());
		}
	}
}

void data_date_time_date(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsDateTimePage *page = (SysSettingsDateTimePage *)getPage(PAGE_ID_SYS_SETTINGS_DATE_TIME);
		if (page && page->ntpEnabled) {
			uint32_t nowUtc = datetime::nowUtc();
			uint32_t nowLocal = datetime::utcToLocal(nowUtc, page->timeZone, page->dstRule);
			value = data::Value(nowLocal, VALUE_TYPE_DATE);
		}
	}
}

void data_date_time_year(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsDateTimePage *page = (SysSettingsDateTimePage *)getPage(PAGE_ID_SYS_SETTINGS_DATE_TIME);
		if (page && !page->ntpEnabled) {
			if (!page->dateTimeModified) {
				page->dateTime = datetime::DateTime::now();
			}
			value = data::Value(page->dateTime.year, VALUE_TYPE_YEAR);
		}
	}
}

void data_date_time_month(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsDateTimePage *page = (SysSettingsDateTimePage *)getPage(PAGE_ID_SYS_SETTINGS_DATE_TIME);
		if (page && !page->ntpEnabled) {
			if (!page->dateTimeModified) {
				page->dateTime = datetime::DateTime::now();
			}
			value = data::Value(page->dateTime.month, VALUE_TYPE_MONTH);
		}
	}
}

void data_date_time_day(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsDateTimePage *page = (SysSettingsDateTimePage *)getPage(PAGE_ID_SYS_SETTINGS_DATE_TIME);
		if (page && !page->ntpEnabled) {
			if (!page->dateTimeModified) {
				page->dateTime = datetime::DateTime::now();
			}
			value = data::Value(page->dateTime.day, VALUE_TYPE_DAY);
		}
	}
}

void data_date_time_time(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsDateTimePage *page = (SysSettingsDateTimePage *)getPage(PAGE_ID_SYS_SETTINGS_DATE_TIME);
		if (page && page->ntpEnabled) {
			uint32_t nowUtc = datetime::nowUtc();
			uint32_t nowLocal = datetime::utcToLocal(nowUtc, page->timeZone, page->dstRule);
			value = data::Value(nowLocal, VALUE_TYPE_TIME);
		}
	}
}

void data_date_time_hour(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsDateTimePage *page = (SysSettingsDateTimePage *)getPage(PAGE_ID_SYS_SETTINGS_DATE_TIME);
		if (page && !page->ntpEnabled) {
			if (!page->dateTimeModified) {
				page->dateTime = datetime::DateTime::now();
			}
			value = data::Value(page->dateTime.hour, VALUE_TYPE_HOUR);
		}
	}
}

void data_date_time_minute(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsDateTimePage *page = (SysSettingsDateTimePage *)getPage(PAGE_ID_SYS_SETTINGS_DATE_TIME);
		if (page && !page->ntpEnabled) {
			if (!page->dateTimeModified) {
				page->dateTime = datetime::DateTime::now();
			}
			value = data::Value(page->dateTime.minute, VALUE_TYPE_MINUTE);
		}
	}
}

void data_date_time_second(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsDateTimePage *page = (SysSettingsDateTimePage *)getPage(PAGE_ID_SYS_SETTINGS_DATE_TIME);
		if (page && !page->ntpEnabled) {
			if (!page->dateTimeModified) {
				page->dateTime = datetime::DateTime::now();
			}
			value = data::Value(page->dateTime.second, VALUE_TYPE_SECOND);
		}
	}
}

void data_date_time_time_zone(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsDateTimePage *page = (SysSettingsDateTimePage *)getPage(PAGE_ID_SYS_SETTINGS_DATE_TIME);
		if (page) {
			value = data::Value(page->timeZone, VALUE_TYPE_TIME_ZONE);
		}
	}
}

void data_date_time_dst(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsDateTimePage *page = (SysSettingsDateTimePage *)getPage(PAGE_ID_SYS_SETTINGS_DATE_TIME);
		if (page) {
			value = data::Value(page->dstRule, VALUE_TYPE_TIME_ZONE);
		}
	}
}

void data_set_page_dirty(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SetPage* page = (SetPage *)getActivePage();
		if (page) {
			value = data::Value(page->getDirty());
		}
	}
}

void data_profiles_list1(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_COUNT) {
		value = 4;
	}
}

void data_profiles_list2(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_COUNT) {
		value = 6;
	} else if (operation == data::DATA_OPERATION_SELECT) {
		cursor.i = 4 + value.getInt();
	}
}

void data_profiles_auto_recall_status(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(persist_conf::isProfileAutoRecallEnabled());
	}
}

void data_profiles_auto_recall_location(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(persist_conf::getProfileAutoRecallLocation());
	}
}

void data_profile_status(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		if (g_selectedProfileLocation != -1) {
			value = profile::isValid(g_selectedProfileLocation);
		}
	}
}

void data_profile_label(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		if (g_selectedProfileLocation != -1) {
			value = data::Value(g_selectedProfileLocation, VALUE_TYPE_USER_PROFILE_LABEL);
		} else if (cursor.i >= 0) {
			value = data::Value(cursor.i, VALUE_TYPE_USER_PROFILE_LABEL);
		}
	}
}

void data_profile_remark(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		if (g_selectedProfileLocation != -1) {
			UserProfilesPage *page = (UserProfilesPage *)getUserProfilesPage();
			if (page) {
				value = data::Value(page->profile.name);
			}
		} else if (cursor.i >= 0) {
			value = data::Value(cursor.i, VALUE_TYPE_USER_PROFILE_REMARK);
		}
	}
}

void data_profile_is_auto_recall_location(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		if (g_selectedProfileLocation != -1) {
			value = persist_conf::getProfileAutoRecallLocation() == g_selectedProfileLocation;
		}
	}
}

void data_profile_channel_u_set(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		if (g_selectedProfileLocation != -1 && (cursor.i >= 0 && cursor.i < CH_MAX)) {
			UserProfilesPage *page = (UserProfilesPage *)getUserProfilesPage();
			if (page) {
				value = MakeValue(page->profile.channels[cursor.i].u_set, UNIT_VOLT, cursor.i);
			}
		}
	}
}

void data_profile_channel_i_set(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		if (g_selectedProfileLocation != -1 && (cursor.i >= 0 && cursor.i < CH_MAX)) {
			UserProfilesPage *page = (UserProfilesPage *)getUserProfilesPage();
			if (page) {
				value = MakeValue(page->profile.channels[cursor.i].i_set, UNIT_AMPER, cursor.i);
			}
		}
	}
}

void data_profile_channel_output_state(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		if (g_selectedProfileLocation != -1 && (cursor.i >= 0 && cursor.i < CH_MAX)) {
			UserProfilesPage *page = (UserProfilesPage *)getUserProfilesPage();
			if (page) {
				value = (int)page->profile.channels[cursor.i].flags.output_enabled;
			}
		}
	}
}

void data_ethernet_installed(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(OPTION_ETHERNET);
	}
}

void data_ethernet_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
#if OPTION_ETHERNET
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsEthernetPage *page = (SysSettingsEthernetPage *)getPage(PAGE_ID_SYS_SETTINGS_ETHERNET);
		if (page) {
			value = page->m_enabled;
		}
	}
#endif
}

void data_ethernet_status(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
#if OPTION_ETHERNET
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(ethernet::g_testResult);
	}
#endif
}

void data_ethernet_ip_address(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
#if OPTION_ETHERNET
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsEthernetStaticPage *page = (SysSettingsEthernetStaticPage *)getPage(PAGE_ID_SYS_SETTINGS_ETHERNET_STATIC);
		if (page) {
			value = data::Value(page->m_ipAddress, VALUE_TYPE_IP_ADDRESS);
		} else {
			SysSettingsEthernetPage *page = (SysSettingsEthernetPage *)getPage(PAGE_ID_SYS_SETTINGS_ETHERNET);
			if (page) {
				if (page->m_dhcpEnabled) {
					value = data::Value(ethernet::getIpAddress(), VALUE_TYPE_IP_ADDRESS);
				}
			}
		}
 	}
#endif
}

void data_ethernet_dns(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
#if OPTION_ETHERNET
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsEthernetStaticPage *page = (SysSettingsEthernetStaticPage *)getPage(PAGE_ID_SYS_SETTINGS_ETHERNET_STATIC);
		if (page) {
			value = data::Value(page->m_dns, VALUE_TYPE_IP_ADDRESS);
		}
	}
#endif
}

void data_ethernet_gateway(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
#if OPTION_ETHERNET
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsEthernetStaticPage *page = (SysSettingsEthernetStaticPage *)getPage(PAGE_ID_SYS_SETTINGS_ETHERNET_STATIC);
		if (page) {
			value = data::Value(page->m_gateway, VALUE_TYPE_IP_ADDRESS);
		}
	}
#endif
}

void data_ethernet_subnet_mask(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
#if OPTION_ETHERNET
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsEthernetStaticPage *page = (SysSettingsEthernetStaticPage *)getPage(PAGE_ID_SYS_SETTINGS_ETHERNET_STATIC);
		if (page) {
			value = data::Value(page->m_subnetMask, VALUE_TYPE_IP_ADDRESS);
		}
	}
#endif
}

void data_ethernet_scpi_port(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
#if OPTION_ETHERNET
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsEthernetPage *page = (SysSettingsEthernetPage *)getPage(PAGE_ID_SYS_SETTINGS_ETHERNET);
		if (page) {
			value = data::Value((uint16_t)page->m_scpiPort, VALUE_TYPE_PORT);
		}
	}
#endif
}

void data_ethernet_is_connected(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
#if OPTION_ETHERNET
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(ethernet::isConnected());
	}
#endif
}

void data_ethernet_dhcp(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
#if OPTION_ETHERNET
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsEthernetPage *page = (SysSettingsEthernetPage *)getPage(PAGE_ID_SYS_SETTINGS_ETHERNET);
		if (page) {
			value = page->m_dhcpEnabled;
		}
	}
#endif
}

void data_ethernet_mac(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
#if OPTION_ETHERNET
	static uint8_t s_macAddressData[2][6];

	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsEthernetPage *page = (SysSettingsEthernetPage *)getPage(PAGE_ID_SYS_SETTINGS_ETHERNET);
		if (page) {
			uint8_t *macAddress = &s_macAddressData[getCurrentStateBufferIndex()][0];
			memcpy(macAddress, page->m_macAddress, 6);
			value = MakeMacAddressValue(macAddress);
		}
	}
#endif
}

void data_channel_is_voltage_balanced(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		if (channel_dispatcher::isSeries()) {
			value = Channel::get(0).isVoltageBalanced() || Channel::get(1).isVoltageBalanced();
		} else {
			value = 0;
		}
	}
}

void data_channel_is_current_balanced(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		if (channel_dispatcher::isParallel()) {
			value = Channel::get(0).isCurrentBalanced() || Channel::get(1).isCurrentBalanced();
		} else {
			value = 0;
		}
	}
}

void data_sys_output_protection_coupled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = persist_conf::isOutputProtectionCoupleEnabled();
	}
}

void data_sys_shutdown_when_protection_tripped(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = persist_conf::isShutdownWhenProtectionTrippedEnabled();
	}
}

void data_sys_force_disabling_all_outputs_on_power_up(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = persist_conf::isForceDisablingAllOutputsOnPowerUpEnabled();
	}
}

void data_sys_password_is_set(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = strlen(persist_conf::devConf2.systemPassword) > 0;
	}
}

void data_sys_rl_state(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(g_rlState);
	}
}

void data_sys_sound_is_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = persist_conf::isSoundEnabled();
	}
}

void data_sys_sound_is_click_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = persist_conf::isClickSoundEnabled();
	}
}

void data_channel_display_view_settings_display_value1(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsAdvViewPage *page = (ChSettingsAdvViewPage *)getPage(PAGE_ID_CH_SETTINGS_ADV_VIEW);
		if (page) {
			value = MakeEnumDefinitionValue(page->displayValue1, ENUM_DEFINITION_CHANNEL_DISPLAY_VALUE);
		}
	}
}

void data_channel_display_view_settings_display_value2(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsAdvViewPage *page = (ChSettingsAdvViewPage *)getPage(PAGE_ID_CH_SETTINGS_ADV_VIEW);
		if (page) {
			value = MakeEnumDefinitionValue(page->displayValue2, ENUM_DEFINITION_CHANNEL_DISPLAY_VALUE);
		}
	}
}

void data_channel_display_view_settings_yt_view_rate(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsAdvViewPage *page = (ChSettingsAdvViewPage *)getPage(PAGE_ID_CH_SETTINGS_ADV_VIEW);
		if (page) {
			value = MakeValue(page->ytViewRate, UNIT_SECOND);
		}
	}
}

void data_sys_encoder_confirmation_mode(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
#if OPTION_ENCODER
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsEncoderPage *page = (SysSettingsEncoderPage *)getPage(PAGE_ID_SYS_SETTINGS_ENCODER);
		if (page) {
			value = data::Value((int)page->confirmationMode);
		}
	}
#endif
}

void data_sys_encoder_moving_up_speed(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
#if OPTION_ENCODER
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsEncoderPage *page = (SysSettingsEncoderPage *)getPage(PAGE_ID_SYS_SETTINGS_ENCODER);
		if (page) {
			value = data::Value((int)page->movingSpeedUp);
		}
	} else if (operation == data::DATA_OPERATION_GET_MIN) {
		value = encoder::MIN_MOVING_SPEED;
	} else if (operation == data::DATA_OPERATION_GET_MAX) {
		value = encoder::MAX_MOVING_SPEED;
	} else if (operation == data::DATA_OPERATION_SET) {
		SysSettingsEncoderPage *page = (SysSettingsEncoderPage *)getPage(PAGE_ID_SYS_SETTINGS_ENCODER);
		if (page) {
			page->movingSpeedUp = (uint8_t)value.getInt();
		}
	}
#endif
}

void data_sys_encoder_moving_down_speed(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
#if OPTION_ENCODER
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsEncoderPage *page = (SysSettingsEncoderPage *)getPage(PAGE_ID_SYS_SETTINGS_ENCODER);
		if (page) {
			value = data::Value((int)page->movingSpeedDown);
		}
	} else if (operation == data::DATA_OPERATION_GET_MIN) {
		value = encoder::MIN_MOVING_SPEED;
	} else if (operation == data::DATA_OPERATION_GET_MAX) {
		value = encoder::MAX_MOVING_SPEED;
	} else if (operation == data::DATA_OPERATION_SET) {
		SysSettingsEncoderPage *page = (SysSettingsEncoderPage *)getPage(PAGE_ID_SYS_SETTINGS_ENCODER);
		if (page) {
			page->movingSpeedDown = (uint8_t)value.getInt();
		}
	}
#endif
}

void data_sys_encoder_installed(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(OPTION_ENCODER);
	}
}

void data_sys_display_state(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = (int)persist_conf::devConf2.flags.displayState;
	}
}

void data_sys_display_brightness(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(persist_conf::devConf2.displayBrightness);
	} else if (operation == data::DATA_OPERATION_GET_MIN) {
		value = DISPLAY_BRIGHTNESS_MIN;
	} else if (operation == data::DATA_OPERATION_GET_MAX) {
		value = DISPLAY_BRIGHTNESS_MAX;
	} else if (operation == data::DATA_OPERATION_SET) {
		persist_conf::setDisplayBrightness((uint8_t)value.getInt());
	}
}

void data_channel_trigger_mode(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = MakeEnumDefinitionValue(channel_dispatcher::getVoltageTriggerMode(*g_channel), ENUM_DEFINITION_CHANNEL_TRIGGER_MODE);
	}
}

void data_channel_trigger_output_state(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = channel_dispatcher::getTriggerOutputState(*g_channel);
	}
}

void data_channel_trigger_on_list_stop(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = MakeEnumDefinitionValue(channel_dispatcher::getTriggerOnListStop(*g_channel), ENUM_DEFINITION_CHANNEL_TRIGGER_ON_LIST_STOP);
	}
}

void data_channel_u_trigger_value(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = MakeValue(channel_dispatcher::getTriggerVoltage(*g_channel), UNIT_VOLT, g_channel->index - 1);
	}
}

void data_channel_i_trigger_value(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = MakeValue(channel_dispatcher::getTriggerCurrent(*g_channel), UNIT_AMPER, g_channel->index - 1);
	}
}

void data_channel_list_count(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		uint16_t count = list::getListCount(*g_channel);
		if (count > 0) {
			value = data::Value(count);
		} else {
			value = data::Value(INF_TEXT);
		}
	}
}

void data_channel_lists(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			value = data::Value(page->m_listVersion);
		}
	} else if (operation == data::DATA_OPERATION_COUNT) {
		value = LIST_ITEMS_PER_PAGE;
	} else if (operation == data::DATA_OPERATION_GET_FLOAT_LIST_LENGTH) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			value = page->getMaxListLength();
		}
	}
}

void data_channel_list_index(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			int iPage = page->getPageIndex();
			int iRow = iPage * LIST_ITEMS_PER_PAGE + cursor.i;
			value = iRow + 1;
		}
	}
}

void data_channel_list_dwell(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			int iPage = page->getPageIndex();
			int iRow = iPage * LIST_ITEMS_PER_PAGE + cursor.i;
			if (iRow < page->m_dwellListLength) {
				value = MakeValue(page->m_dwellList[iRow], UNIT_SECOND);
			} else {
				value = data::Value(EMPTY_VALUE);
			}
		}
	} else if (operation == data::DATA_OPERATION_GET_MIN) {
		value = MakeValue(LIST_DWELL_MIN, UNIT_SECOND);
	} else if (operation == data::DATA_OPERATION_GET_MAX) {
		value = MakeValue(LIST_DWELL_MAX, UNIT_SECOND);
	} else if (operation == data::DATA_OPERATION_GET_DEF) {
		value = MakeValue(LIST_DWELL_DEF, UNIT_SECOND);
	} else if (operation == data::DATA_OPERATION_GET_FLOAT_LIST_LENGTH) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			value = page->m_dwellListLength;
		}
	} else if (operation == data::DATA_OPERATION_GET_FLOAT_LIST) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			value = MakeFloatListValue(page->m_dwellList);
		}
	}
}

void data_channel_list_dwell_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			int iPage = page->getPageIndex();
			int iRow = iPage * LIST_ITEMS_PER_PAGE + cursor.i;
			value = iRow <= page->m_dwellListLength;
		}
	}
}

void data_channel_list_voltage(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			int iPage = page->getPageIndex();
			int iRow = iPage * LIST_ITEMS_PER_PAGE + cursor.i;
			if (iRow < page->m_voltageListLength) {
				value = MakeValue(page->m_voltageList[iRow], UNIT_VOLT, g_channel->index - 1);
			} else {
				value = data::Value(EMPTY_VALUE);
			}
		}
	} else if (operation == data::DATA_OPERATION_GET_MIN) {
		value = MakeValue(channel_dispatcher::getUMin(*g_channel), UNIT_VOLT, g_channel->index - 1);
	} else if (operation == data::DATA_OPERATION_GET_MAX) {
		value = MakeValue(channel_dispatcher::getUMax(*g_channel), UNIT_VOLT, g_channel->index - 1);
	} else if (operation == data::DATA_OPERATION_GET_DEF) {
		value = MakeValue(g_channel->u.def, UNIT_VOLT, g_channel->index - 1);
	} else if (operation == data::DATA_OPERATION_GET_FLOAT_LIST_LENGTH) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			value = page->m_voltageListLength;
		}
	} else if (operation == data::DATA_OPERATION_GET_FLOAT_LIST) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			value = MakeFloatListValue(page->m_voltageList);
		}
	}
}

void data_channel_list_voltage_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			int iPage = page->getPageIndex();
			int iRow = iPage * LIST_ITEMS_PER_PAGE + cursor.i;
			value = iRow <= page->m_voltageListLength;
		}
	}
}

void data_channel_list_current(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			int iPage = page->getPageIndex();
			int iRow = iPage * LIST_ITEMS_PER_PAGE + cursor.i;
			if (iRow < page->m_currentListLength) {
				value = MakeValue(page->m_currentList[iRow], UNIT_AMPER, g_channel->index - 1);
			} else {
				value = data::Value(EMPTY_VALUE);
			}
		}
	} else if (operation == data::DATA_OPERATION_GET_MIN) {
		value = MakeValue(channel_dispatcher::getIMin(*g_channel), UNIT_AMPER, g_channel->index - 1);
	} else if (operation == data::DATA_OPERATION_GET_MAX) {
		value = MakeValue(channel_dispatcher::getIMax(*g_channel), UNIT_AMPER, g_channel->index - 1);
	} else if (operation == data::DATA_OPERATION_GET_DEF) {
		value = MakeValue(g_channel->i.def, UNIT_AMPER, g_channel->index - 1);
	} else if (operation == data::DATA_OPERATION_GET_FLOAT_LIST_LENGTH) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			value = page->m_currentListLength;
		}
	} else if (operation == data::DATA_OPERATION_GET_FLOAT_LIST) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			ChSettingsListsPage *page = (ChSettingsListsPage*)getActivePage();
			value = MakeFloatListValue(page->m_currentList);
		}
	}
}

void data_channel_list_current_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			int iPage = page->getPageIndex();
			int iRow = iPage * LIST_ITEMS_PER_PAGE + cursor.i;
			value = iRow <= page->m_currentListLength;
		}
	}
}

void data_channel_lists_previous_page_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			int iPage = page->getPageIndex();
			value = iPage > 0;
		}
	}
}

void data_channel_lists_next_page_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			int iPage = page->getPageIndex();
			value = (iPage < page->getNumPages() - 1);
		}
	}
}

void data_channel_lists_cursor(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
	if (page) {
		if (operation == data::DATA_OPERATION_GET) {
			value = data::Value(page->m_iCursor);
		} else if (operation == data::DATA_OPERATION_SET) {
			page->m_iCursor = value.getInt();
			page->moveCursorToFirstAvailableCell();
		}
	}
}

void data_channel_lists_insert_menu_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			value = page->getRowIndex() < page->getMaxListLength();
		}
	}
}

void data_channel_lists_delete_menu_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			value = page->getMaxListLength();
		}
	}
}

void data_channel_lists_delete_row_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			value = page->getRowIndex() < page->getMaxListLength();
		}
	}
}

void data_channel_lists_clear_column_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			value = page->getRowIndex() < page->getMaxListLength();
		}
	}
}

void data_channel_lists_delete_rows_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		ChSettingsListsPage *page = (ChSettingsListsPage *)getPage(PAGE_ID_CH_SETTINGS_LISTS);
		if (page) {
			value = page->getRowIndex() < page->getMaxListLength();
		}
	}
}

void data_trigger_source(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsTriggerPage *page = (SysSettingsTriggerPage *)getPage(PAGE_ID_SYS_SETTINGS_TRIGGER);
		if (page) {
			value = MakeEnumDefinitionValue(page->m_source, ENUM_DEFINITION_TRIGGER_SOURCE);
		}
	}
}

void data_trigger_delay(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsTriggerPage *page = (SysSettingsTriggerPage *)getPage(PAGE_ID_SYS_SETTINGS_TRIGGER);
		if (page) {
			value = MakeValue(page->m_delay, UNIT_SECOND);
		}
	}
}

void data_trigger_initiate_continuously(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsTriggerPage *page = (SysSettingsTriggerPage *)getPage(PAGE_ID_SYS_SETTINGS_TRIGGER);
		if (page) {
			value = page->m_initiateContinuously;
		}
	}
}

void data_trigger_is_initiated(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		bool isInitiated = trigger::isInitiated();
#if OPTION_SD_CARD
		if (!isInitiated && dlog::isInitiated()) {
			isInitiated = true;
		}
#endif
		value = isInitiated;
	} else if (operation == data::DATA_OPERATION_IS_BLINKING) {
		value = trigger::isInitiated();
	}
}

void data_trigger_is_manual(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		bool isManual = trigger::getSource() == trigger::SOURCE_MANUAL;
#if OPTION_SD_CARD
		if (!isManual && dlog::g_triggerSource == trigger::SOURCE_MANUAL) {
			isManual = true;
		}
#endif
		value = isManual;
	}
}

void data_channel_has_support_for_current_dual_range(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		value = channel.hasSupportForCurrentDualRange();
	}
}

void data_channel_ranges_supported(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = g_channel->hasSupportForCurrentDualRange();
	}
}

void data_channel_ranges_mode(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = MakeEnumDefinitionValue(g_channel->getCurrentRangeSelectionMode(),
			ENUM_DEFINITION_CHANNEL_CURRENT_RANGE_SELECTION_MODE);
	}
}

void data_channel_ranges_auto_ranging(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = g_channel->isAutoSelectCurrentRangeEnabled();
	}
}

void data_channel_ranges_currently_selected(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = MakeEnumDefinitionValue(g_channel->flags.currentCurrentRange,
			ENUM_DEFINITION_CHANNEL_CURRENT_RANGE);
	}
}

void data_text_message(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(getTextMessageVersion(), VALUE_TYPE_TEXT_MESSAGE);
	}
}

void data_serial_status(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(serial::g_testResult);
	}
}

void data_serial_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsSerialPage *page = (SysSettingsSerialPage *)getPage(PAGE_ID_SYS_SETTINGS_SERIAL);
		if (page) {
			value = page->m_enabled;
		}
	}
}

void data_serial_is_connected(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(serial::isConnected());
	}
}

void data_serial_baud(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsSerialPage *page = (SysSettingsSerialPage *)getPage(PAGE_ID_SYS_SETTINGS_SERIAL);
		if (page) {
			value = data::Value((int)page->m_baudIndex, VALUE_TYPE_SERIAL_BAUD_INDEX);
		}
	} else if (operation == data::DATA_OPERATION_GET_MIN) {
		value = 1;
	} else if (operation == data::DATA_OPERATION_GET_MAX) {
		value = (int)serial::g_baudsSize;
	} else if (operation == data::DATA_OPERATION_SET) {
		SysSettingsSerialPage *page = (SysSettingsSerialPage *)getPage(PAGE_ID_SYS_SETTINGS_SERIAL);
		if (page) {
			page->m_baudIndex = (uint8_t)value.getInt();
		}
	}
}

void data_serial_parity(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsSerialPage *page = (SysSettingsSerialPage *)getPage(PAGE_ID_SYS_SETTINGS_SERIAL);
		if (page) {
			value = MakeEnumDefinitionValue(page->m_parity, ENUM_DEFINITION_SERIAL_PARITY);
		}
	}
}

void data_channel_list_countdown(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel &channel = Channel::get(iChannel);
		int32_t remaining;
		uint32_t total;
		if (list::getCurrentDwellTime(channel, remaining, total) && total >= CONF_LIST_COUNDOWN_DISPLAY_THRESHOLD) {
			value = data::Value((uint32_t)remaining, VALUE_TYPE_COUNTDOWN);
		}
	}
}

void data_io_pins(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_COUNT) {
#if EEZ_PSU_SELECTED_REVISION == EEZ_PSU_REVISION_R5B12
		value = 3;
#else
		value = 1;
#endif
	}
}

void data_io_pins_inhibit_state(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		persist_conf::IOPin &inputPin = persist_conf::devConf2.ioPins[0];
		if (inputPin.function == io_pins::FUNCTION_INHIBIT) {
			value = io_pins::isInhibited();
		} else {
			value = 2;
		}
	} else if (operation == data::DATA_OPERATION_IS_BLINKING) {
		persist_conf::IOPin &inputPin = persist_conf::devConf2.ioPins[0];
		value = inputPin.function == io_pins::FUNCTION_INHIBIT && io_pins::isInhibited();
	}

}

void data_io_pin_number(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(cursor.i + 1);
	}
}

void data_io_pin_polarity(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsIOPinsPage *page = (SysSettingsIOPinsPage *)getPage(PAGE_ID_SYS_SETTINGS_IO);
		if (page) {
			value = MakeEnumDefinitionValue(page->m_polarity[cursor.i], ENUM_DEFINITION_IO_PINS_POLARITY);
		}
	}
}

void data_io_pin_function(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsIOPinsPage *page = (SysSettingsIOPinsPage *)getPage(PAGE_ID_SYS_SETTINGS_IO);
		if (page) {
			if (cursor.i == 0) {
				value = MakeEnumDefinitionValue(page->m_function[cursor.i], ENUM_DEFINITION_IO_PINS_INPUT_FUNCTION);
			} else {
				value = MakeEnumDefinitionValue(page->m_function[cursor.i], ENUM_DEFINITION_IO_PINS_OUTPUT_FUNCTION);
			}
		}
	}
}

void data_ntp_enabled(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsDateTimePage *page = (SysSettingsDateTimePage *)getPage(PAGE_ID_SYS_SETTINGS_DATE_TIME);
		if (page) {
			value = page->ntpEnabled;
		}
	}
}

void data_ntp_server(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		SysSettingsDateTimePage *page = (SysSettingsDateTimePage *)getPage(PAGE_ID_SYS_SETTINGS_DATE_TIME);
		if (page) {
			value = page->ntpServer[0] ? page->ntpServer : "<not specified>";
		}
	}

}

void data_async_operation_throbber(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(gui::g_throbber[(millis() % 1000) / 125]);
	}
}

void data_sys_display_background_luminosity_step(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = data::Value(persist_conf::devConf2.displayBackgroundLuminosityStep);
	} else if (operation == data::DATA_OPERATION_GET_MIN) {
		value = DISPLAY_BACKGROUND_LUMINOSITY_STEP_MIN;
	} else if (operation == data::DATA_OPERATION_GET_MAX) {
		value = DISPLAY_BACKGROUND_LUMINOSITY_STEP_MAX;
	} else if (operation == data::DATA_OPERATION_SET) {
		persist_conf::setDisplayBackgroundLuminosityStep((uint8_t)value.getInt());
	}
}

void data_progress(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
		value = gui::g_progress;
	}
}

void data_view_status(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
	if (operation == data::DATA_OPERATION_GET) {
#if OPTION_SD_CARD
		bool listStatusVisible = list::anyCounterVisible(CONF_LIST_COUNDOWN_DISPLAY_THRESHOLD);
		bool dlogStatusVisible = !dlog::isIdle();
		if (listStatusVisible && dlogStatusVisible) {
			value = micros() % (2 * 1000000UL) < 1000000UL ? 1 : 2;
		} else if (listStatusVisible) {
			value = 1;
		} else if (dlogStatusVisible) {
			value = 2;
		}
#else
		if (list::anyCounterVisible(CONF_LIST_COUNDOWN_DISPLAY_THRESHOLD)) {
			value = 1;
		}
#endif
		value = 0;
	}
}

void data_dlog_status(data::DataOperationEnum operation, data::Cursor &cursor, data::Value &value) {
#if OPTION_SD_CARD
	if (operation == data::DATA_OPERATION_GET) {
		if (dlog::isInitiated()) {
			value = "Dlog trigger waiting";
		} else if (!dlog::isIdle()) {
			value = data::Value((uint32_t)floor(dlog::g_currentTime), VALUE_TYPE_DLOG_STATUS);
		}
	}
#endif
}

}
}
} // namespace eez::app::gui

#endif
