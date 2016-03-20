#pragma once

namespace eez {
namespace psu {
namespace gui {
namespace data {

enum Unit {
    UNIT_NONE,
    UNIT_VOLT,
    UNIT_AMPER,
    UNIT_MILLI_VOLT,
    UNIT_MILLI_AMPER,
    UNIT_CONST_STR,
    UNIT_STR
};

struct Value {
    Value() {
        memset(this, 0, sizeof(Value));
    }

    Value(int value) : int_(value) {
    }
    
    Value(float value, Unit unit) {
        float_ = value;
        unit_ = unit;
    }

    Value(char *str) {
        str_ = str;
        unit_ = UNIT_STR;
    }

    static Value ConstStr(const char *pstr PROGMEM) {
        Value value;
        value.const_str_ = pstr;
        value.unit_ = UNIT_CONST_STR;
        return value;
    }

    bool operator ==(Value other) {
        return memcmp(this, &other, sizeof(Value)) == 0;
    }

    bool operator !=(Value other) {
        return memcmp(this, &other, sizeof(Value)) != 0;
    }

    float getFloat() { return float_; }
    Unit getUnit() { return (Unit)unit_; }
    uint8_t getInt() { return int_; }

    void toText(char *text, int count);

private:
    uint8_t unit_;
    union {
        int int_;
        const char *const_str_ PROGMEM;
        const char *str_;
        float float_;
    };
};

struct ChannelStateFlags {
    unsigned mode : 2;
    unsigned state : 2;
    unsigned ovp : 2;
    unsigned ocp : 2;
    unsigned opp : 2;
    unsigned otp : 2;
    unsigned dp : 2;
};

struct ChannelState {
    Value mon_value;
    float u_set;
    float i_set;
    ChannelStateFlags flags;
};

struct Cursor {
    int selected_channel_index;
    ChannelState channel_last_state[CH_NUM];

    bool operator != (const Cursor& rhs) const {
        return !(*this == rhs);
    }

    bool operator == (const Cursor& rhs) const {
        return selected_channel_index == rhs.selected_channel_index;
    }
};

Cursor getCursor();
void setCursor(Cursor cursor_);

int count(uint8_t id);
void select(uint8_t id, int index);
Value get(uint8_t id, bool &changed);
Value getMin(uint8_t id);
Value getMax(uint8_t id);
Unit getUnit(uint8_t id);
void set(uint8_t id, Value value);
void do_action(uint8_t id);

}
}
}
} // namespace eez::psu::ui
