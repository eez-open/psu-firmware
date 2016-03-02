#pragma once

#include "UTouch.h"

namespace eez {
namespace psu {
namespace gui {
namespace gesture {

enum GestureType {
    GESTURE_NONE,

    GESTURE_DOWN,
    GESTURE_UP,

    GESTURE_SLIDE_UP,
    GESTURE_SLIDE_RIGHT,
    GESTURE_SLIDE_DOWN,
    GESTURE_SLIDE_LEFT,

    GESTURE_TAP,
};

extern GestureType gesture;

void push_pointer(unsigned long tick_usec, bool is_down, int x, int y);

}
}
}
} // namespace eez::psu::ui::touch
