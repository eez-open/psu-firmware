/*
* EEZ PSU Firmware
* Copyright (C) 2015 Envox d.o.o.
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
namespace gui {
namespace data {

////////////////////////////////////////////////////////////////////////////////

enum Unit {
    UNIT_NONE,

    UNIT_INT,

    UNIT_VOLT,
    UNIT_AMPER,
    UNIT_MILLI_VOLT,
    UNIT_MILLI_AMPER,

    UNIT_CONST_STR,
    UNIT_STR
};

struct Value {
    Value() : unit_(UNIT_NONE) { }
    Value(int value) : unit_(UNIT_INT), int_(value)  {}
    Value(float value, Unit unit) : unit_(unit), float_(value) {}
    Value(char *str) : unit_(UNIT_STR), str_(str) {}

    static Value ConstStr(const char *pstr PROGMEM) {
        Value value;
        value.const_str_ = pstr;
        value.unit_ = UNIT_CONST_STR;
        return value;
    }

    bool operator ==(const Value &other) {
        if (unit_ != other.unit_) {
            return false;
        }

        if (unit_ == UNIT_NONE) {
            return true;
        } else if (unit_ == UNIT_STR) {
            return strcmp(str_, other.str_) == 0;
        } else if (unit_ == UNIT_CONST_STR) {
            return strcmp_P(str_, other.str_) == 0;
        } else if (unit_ == UNIT_INT) {
            return int_ == other.int_;
        } else {
            return float_ == other.float_;
        }
    }

    bool operator !=(const Value &other) {
        return !(*this == other);
    }

    float getFloat() const { return float_; }
    
    Unit getUnit() const { return (Unit)unit_; }

    uint8_t getInt() const { return int_; }

    void toText(char *text, int count) const;

private:
    uint8_t unit_;
    union {
        int int_;
        float float_;
        const char *const_str_ PROGMEM;
        const char *str_;
    };
};

////////////////////////////////////////////////////////////////////////////////

struct Cursor {
    int iChannel;

    Cursor() {
        iChannel = -1;
    }

    Cursor(int i) {
        iChannel = i;
    }

    operator bool() {
        return iChannel != -1;
    }

    bool operator != (const Cursor& rhs) const {
        return !(*this == rhs);
    }

    bool operator == (const Cursor& rhs) const {
        return iChannel == rhs.iChannel;
    }
};

////////////////////////////////////////////////////////////////////////////////

int count(uint8_t id);
void select(Cursor &cursor, uint8_t id, int index);

Value getMin(const Cursor &cursor, uint8_t id);
Value getMax(const Cursor &cursor, uint8_t id);
Unit getUnit(const Cursor &cursor, uint8_t id);


void getButtonLabels(const Cursor &cursor, uint8_t id, const Value **labels, int &count);

void set(const Cursor &cursor, uint8_t id, Value value);
void toggle(uint8_t id);
void doAction(const Cursor &cursor, uint8_t id);

}
}
}
} // namespace eez::psu::ui::data
