#pragma once

namespace eez {
namespace psu {
namespace gui {
namespace data {

enum Unit {
    UNIT_NONE,
    UNIT_VOLT,
    UNIT_AMPER
};

union Value {
public:
    Value() {
        memset(this, 0, sizeof(Value));
    }

    Value(int value) : int_(value) {
    }
    
    Value(float value, Unit unit) {
        float_ = value;
        unit_ = unit;
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

    void toText(char *text);
    void toTextNoUnit(char *text);

private:
    int int_;
    struct {
        float float_;
        uint8_t unit_;
    };
};

struct Cursor {
    Channel *selected_channel;

    bool operator != (const Cursor& rhs) const {
        return selected_channel != rhs.selected_channel;
    }
};

Cursor getCursor();
void setCursor(Cursor cursor_);

int count(uint16_t id);
void select(uint16_t id, int index);
Value get(uint16_t id, bool &changed);
Value getMin(uint16_t id);
Value getMax(uint16_t id);
void set(uint16_t id, Value value);


}
}
}
} // namespace eez::psu::ui
