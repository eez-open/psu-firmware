/*
* EEZ PSU Firmware
* Copyright (C) 2015-present, Envox d.o.o.
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

#include "event_queue.h"
#include "value.h"

namespace eez {
namespace psu {
namespace gui {
namespace data {

////////////////////////////////////////////////////////////////////////////////

enum EnumDefinition {
    ENUM_DEFINITION_CHANNEL_DISPLAY_VALUE,
    ENUM_DEFINITION_CHANNEL_TRIGGER_MODE,
    ENUM_DEFINITION_TRIGGER_SOURCE,
    ENUM_DEFINITION_CHANNEL_CURRENT_RANGE_SELECTION_MODE,
    ENUM_DEFINITION_CHANNEL_CURRENT_RANGE,
    ENUM_DEFINITION_CHANNEL_TRIGGER_ON_LIST_STOP,
    ENUM_DEFINITION_IO_PINS_POLARITY,
    ENUM_DEFINITION_IO_PINS_INPUT_FUNCTION,
    ENUM_DEFINITION_IO_PINS_OUTPUT_FUNCTION
};

struct EnumItem {
    uint8_t value;
    const char *menuLabel PROGMEM;
    const char *widgetLabel PROGMEM;
};

extern data::EnumItem g_channelDisplayValueEnumDefinition[];
extern data::EnumItem g_channelTriggerModeEnumDefinition[];
extern data::EnumItem g_triggerSourceEnumDefinition[];
extern data::EnumItem g_channelCurrentRangeSelectionModeEnumDefinition[];
extern data::EnumItem g_channelCurrentRangeEnumDefinition[];
extern data::EnumItem g_channelTriggerOnListStopEnumDefinition[];
extern data::EnumItem g_ioPinsPolarityEnumDefinition[];
extern data::EnumItem g_ioPinsInputFunctionEnumDefinition[];
extern data::EnumItem g_ioPinsOutputFunctionEnumDefinition[];

enum ValueOptions {
    VALUE_OPTIONS_CH1 = 0x00,
    VALUE_OPTIONS_CH2 = 0x01,
    VALUE_OPTIONS_CH_MASK = 0x03,

    VALUE_OPTIONS_EXTENDED_PRECISION = 0x04
};

////////////////////////////////////////////////////////////////////////////////

inline bool isFloatType(ValueType valueType) { 
    return valueType >= VALUE_TYPE_FLOAT_FIRST && valueType <= VALUE_TYPE_FLOAT_LAST;
}

struct Value {
    Value() : type_(VALUE_TYPE_NONE) { }
    Value(int value) : type_(VALUE_TYPE_INT), int_(value)  {}
	Value(int value, ValueType type) : type_(type), int_(value)  {}
	Value(uint8_t value, ValueType type) : type_(type), uint8_(value)  {}
	Value(uint16_t value, ValueType type) : type_(type), uint16_(value)  {}
	Value(uint32_t value, ValueType type) : type_(type), uint32_(value)  {}
    Value(float value, ValueType type);
    Value(float value, ValueType type, int channelIndex);
    Value(float value, ValueType type, int channelIndex, bool extendedPrecision);
    Value(const char *str) : type_(VALUE_TYPE_STR), str_(str) {}
	Value(event_queue::Event *e) : type_(VALUE_TYPE_EVENT), event_(e) {}
    Value(uint8_t value, EnumDefinition enumDefinition) : type_(VALUE_TYPE_ENUM) {
        enum_.value = value;
        enum_.enumDefinition = enumDefinition;
    }

    static Value ProgmemStr(const char *pstr PROGMEM);
	static Value PageInfo(uint8_t pageIndex, uint8_t numPages);
	static Value ScpiErrorText(int16_t errorCode);
	static Value LessThenMinMessage(float float_, ValueType type);
	static Value GreaterThenMaxMessage(float float_, ValueType type);

	bool operator ==(const Value &other) const;

    bool operator !=(const Value &other) const {
        return !(*this == other);
    }

    float getFloat() const { return float_; }
    
    ValueType getType() const { return (ValueType)type_; }
    bool isFloat() const { return isFloatType((ValueType)type_); }

    int getInt() const;

    void toText(char *text, int count) const;

	bool isString() { return type_ == VALUE_TYPE_STR; }
	const char *asString() { return str_; }

    bool isConstString() { return type_ == VALUE_TYPE_CONST_STR; }

	uint8_t getPageIndex() { return pageInfo_.pageIndex; }
	uint8_t getNumPages() { return pageInfo_.numPages; }

    int16_t getScpiError() { return int16_; }

    bool isMilli() const;

private:
    uint8_t type_;
    uint8_t options_;
    union {
        int int_;
		int16_t int16_;
		uint8_t uint8_;
		uint16_t uint16_;
		uint32_t uint32_;

		float float_;
        
		const char *const_str_ PROGMEM;
        const char *str_;
		
		event_queue::Event *event_;
		
		struct {
			uint8_t pageIndex;
			uint8_t numPages;
		} pageInfo_;

		struct {
			uint8_t value;
			uint8_t enumDefinition;
		} enum_;
    };

    void formatFloatValue(float &value, ValueType &valueType, int &numSignificantDecimalDigits) const;
};

////////////////////////////////////////////////////////////////////////////////

struct Cursor {
    int i;

    Cursor() {
        i = -1;
    }

    Cursor(int i, int j = -1) {
        this->i = i;
    }

    operator bool() {
        return i != -1;
    }

    bool operator != (const Cursor& rhs) const {
        return !(*this == rhs);
    }

    bool operator == (const Cursor& rhs) const {
        return i == rhs.i;
    }

	void reset() {
		i = -1;
	}
};

////////////////////////////////////////////////////////////////////////////////

extern Value g_alertMessage;
extern Value g_alertMessage2;
extern Value g_alertMessage3;

int count(uint8_t id);
void select(Cursor &cursor, uint8_t id, int index);

int getListLength(uint8_t id);
float *getFloatList(uint8_t id);

Value getMin(const Cursor &cursor, uint8_t id);
Value getMax(const Cursor &cursor, uint8_t id);
Value getDef(const Cursor &cursor, uint8_t id);
Value getLimit(const Cursor &cursor, uint8_t id);
ValueType getUnit(const Cursor &cursor, uint8_t id);

void getList(const Cursor &cursor, uint8_t id, const Value **labels, int &count);

Value get(const Cursor &cursor, uint8_t id);
bool set(const Cursor &cursor, uint8_t id, Value value, int16_t *error);

int getNumHistoryValues(uint8_t id);
int getCurrentHistoryValuePosition(const Cursor &cursor, uint8_t id);
Value getHistoryValue(const Cursor &cursor, uint8_t id, int position);

bool isBlinking(const Cursor &cursor, uint8_t id);
Value getEditValue(const Cursor &cursor, uint8_t id);

}
}
}
} // namespace eez::psu::ui::data
