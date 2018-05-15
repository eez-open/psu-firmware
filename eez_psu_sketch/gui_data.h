#pragma once

#include "event_queue.h"
#include "mw_gui_data.h"

namespace eez {
namespace psu {
namespace gui {

enum EnumDefinition {
	ENUM_DEFINITION_CHANNEL_DISPLAY_VALUE,
	ENUM_DEFINITION_CHANNEL_TRIGGER_MODE,
	ENUM_DEFINITION_TRIGGER_SOURCE,
	ENUM_DEFINITION_CHANNEL_CURRENT_RANGE_SELECTION_MODE,
	ENUM_DEFINITION_CHANNEL_CURRENT_RANGE,
	ENUM_DEFINITION_CHANNEL_TRIGGER_ON_LIST_STOP,
	ENUM_DEFINITION_IO_PINS_POLARITY,
	ENUM_DEFINITION_IO_PINS_INPUT_FUNCTION,
	ENUM_DEFINITION_IO_PINS_OUTPUT_FUNCTION,
	ENUM_DEFINITION_SERIAL_PARITY,
	ENUM_DEFINITION_DST_RULE
};

using mw::gui::data::EnumItem;

extern EnumItem g_channelDisplayValueEnumDefinition[];
extern EnumItem g_channelTriggerModeEnumDefinition[];
extern EnumItem g_triggerSourceEnumDefinition[];
extern EnumItem g_channelCurrentRangeSelectionModeEnumDefinition[];
extern EnumItem g_channelCurrentRangeEnumDefinition[];
extern EnumItem g_channelTriggerOnListStopEnumDefinition[];
extern EnumItem g_ioPinsPolarityEnumDefinition[];
extern EnumItem g_ioPinsInputFunctionEnumDefinition[];
extern EnumItem g_ioPinsOutputFunctionEnumDefinition[];
extern EnumItem g_serialParityEnumDefinition[];
extern EnumItem g_dstRuleEnumDefinition[];

enum UserValueType {
	VALUE_TYPE_LESS_THEN_MIN_FLOAT = VALUE_TYPE_USER,
	VALUE_TYPE_GREATER_THEN_MAX_FLOAT,
	VALUE_TYPE_CHANNEL_LABEL,
	VALUE_TYPE_CHANNEL_SHORT_LABEL,
	VALUE_TYPE_CHANNEL_BOARD_INFO_LABEL,
	VALUE_TYPE_LESS_THEN_MIN_INT,
	VALUE_TYPE_LESS_THEN_MIN_TIME_ZONE,
	VALUE_TYPE_GREATER_THEN_MAX_INT,
	VALUE_TYPE_GREATER_THEN_MAX_TIME_ZONE,
	VALUE_TYPE_EVENT,
	VALUE_TYPE_PAGE_INFO,
	VALUE_TYPE_ON_TIME_COUNTER,
	VALUE_TYPE_COUNTDOWN,
	VALUE_TYPE_TIME_ZONE,
	VALUE_TYPE_DATE,
	VALUE_TYPE_YEAR,
	VALUE_TYPE_MONTH,
	VALUE_TYPE_DAY,
	VALUE_TYPE_TIME,
	VALUE_TYPE_HOUR,
	VALUE_TYPE_MINUTE,
	VALUE_TYPE_SECOND,
	VALUE_TYPE_USER_PROFILE_LABEL,
	VALUE_TYPE_USER_PROFILE_REMARK,
	VALUE_TYPE_EDIT_INFO,
	VALUE_TYPE_MAC_ADDRESS,
	VALUE_TYPE_IP_ADDRESS,
	VALUE_TYPE_PORT,
	VALUE_TYPE_TEXT_MESSAGE,
	VALUE_TYPE_SERIAL_BAUD_INDEX,
	VALUE_TYPE_PERCENTAGE,
	VALUE_TYPE_SIZE,
	VALUE_TYPE_DLOG_STATUS,
	VALUE_TYPE_VALUE_LIST,
	VALUE_TYPE_FLOAT_LIST
};

using mw::gui::data::Value;

Value MakeValue(float value, Unit unit, int channelIndex = -1, bool extendedPrecision = false);
Value MakeValueListValue(const Value *values);
Value MakeFloatListValue(float *pFloat);
Value MakeEventValue(psu::event_queue::Event *e);
Value MakePageInfoValue(uint8_t pageIndex, uint8_t numPages);
Value MakeLessThenMinMessageValue(float float_, const Value& value_);
Value MakeGreaterThenMaxMessageValue(float float_, const Value& value_);
Value MakeMacAddressValue(uint8_t* macAddress);

extern Value g_alertMessage;
extern Value g_alertMessage2;
extern Value g_alertMessage3;
extern Value g_progress;

extern char g_throbber[8];

}
}
} // eez::psu::gui