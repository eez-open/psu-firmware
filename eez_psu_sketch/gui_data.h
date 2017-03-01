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

namespace eez {
namespace psu {
namespace gui {
namespace data {

////////////////////////////////////////////////////////////////////////////////

enum EnumDefinition {
    ENUM_DEFINITION_CHANNEL_DISPLAY_VALUE,
    ENUM_DEFINITION_CHANNEL_TRIGGER_MODE
};

struct EnumItem {
    uint8_t value;
    const char *label PROGMEM;
};

extern data::EnumItem g_channelDisplayValueEnumDefinition[];
extern data::EnumItem g_channelTriggerModeEnumDefinition[];

////////////////////////////////////////////////////////////////////////////////

enum ValueType {
    VALUE_TYPE_NONE,
    VALUE_TYPE_INT,
    VALUE_TYPE_FLOAT_FIRST,
	VALUE_TYPE_FLOAT,
    VALUE_TYPE_FLOAT_VOLT,
    VALUE_TYPE_FLOAT_AMPER,
    VALUE_TYPE_FLOAT_MILLI_VOLT,
    VALUE_TYPE_FLOAT_MILLI_AMPER,
	VALUE_TYPE_FLOAT_WATT,
	VALUE_TYPE_FLOAT_SECOND,
	VALUE_TYPE_FLOAT_CELSIUS,
	VALUE_TYPE_FLOAT_RPM,
    VALUE_TYPE_FLOAT_LAST,
    VALUE_TYPE_CONST_STR,
    VALUE_TYPE_STR,
    VALUE_TYPE_CHANNEL_LABEL,
	VALUE_TYPE_CHANNEL_SHORT_LABEL,
	VALUE_TYPE_CHANNEL_BOARD_INFO_LABEL,
	VALUE_TYPE_LESS_THEN_MIN_FLOAT,
	VALUE_TYPE_LESS_THEN_MIN_INT,
	VALUE_TYPE_LESS_THEN_MIN_TIME_ZONE,
	VALUE_TYPE_GREATER_THEN_MAX_FLOAT,
	VALUE_TYPE_GREATER_THEN_MAX_INT,
	VALUE_TYPE_GREATER_THEN_MAX_TIME_ZONE,
	VALUE_TYPE_EVENT,
	VALUE_TYPE_PAGE_INFO,
	VALUE_TYPE_ON_TIME_COUNTER,
	VALUE_TYPE_SCPI_ERROR_TEXT,
	VALUE_TYPE_TIME_ZONE,
	VALUE_TYPE_YEAR,
	VALUE_TYPE_MONTH,
	VALUE_TYPE_DAY,
	VALUE_TYPE_HOUR,
	VALUE_TYPE_MINUTE,
	VALUE_TYPE_SECOND,
	VALUE_TYPE_USER_PROFILE_LABEL,
	VALUE_TYPE_USER_PROFILE_REMARK,
    VALUE_TYPE_EDIT_INFO,
    VALUE_TYPE_IP_ADDRESS,
    VALUE_TYPE_ENUM
};

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
    Value(float value, ValueType type) : type_(type), float_(value) {}
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

    int getInt() const { return int_; }

    void toText(char *text, int count) const;

	bool isString() { return type_ == VALUE_TYPE_STR; }
	const char *asString() { return str_; }

	uint8_t getPageIndex() { return pageInfo_.pageIndex; }
	uint8_t getNumPages() { return pageInfo_.numPages; }

    int16_t getScpiError() { return int16_; }

private:
    uint8_t type_;
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
};

////////////////////////////////////////////////////////////////////////////////

struct Cursor {
    int i;
    int j;

    Cursor() {
        i = -1;
        j = -1;
    }

    Cursor(int i, int j = -1) {
        this->i = i;
        this->j = j;
    }

    operator bool() {
        return i != -1;
    }

    bool operator != (const Cursor& rhs) const {
        return !(*this == rhs);
    }

    bool operator == (const Cursor& rhs) const {
        return i == rhs.i && j == rhs.j;
    }

	void reset() {
		i = -1;
        j = -1;
	}
};

////////////////////////////////////////////////////////////////////////////////

extern Value g_alertMessage;
extern Value g_alertMessage2;
extern Value g_alertMessage3;

int count(uint8_t id);
void select(Cursor &cursor, uint8_t id, int index);

Value getMin(const Cursor &cursor, uint8_t id);
Value getMax(const Cursor &cursor, uint8_t id);
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
