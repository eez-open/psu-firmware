#pragma once

#include "UTouch.h"

namespace eez {
namespace psu {
namespace gui {
namespace gesture {

enum GestureType {
    GESTURE_NONE = 0,

    GESTURE_SLIDE_UP = 1,
    GESTURE_SLIDE_RIGHT = 2,
    GESTURE_SLIDE_DOWN = 3,
    GESTURE_SLIDE_LEFT = 4,
};

extern GestureType gesture;

void push_pointer(unsigned long tick_usec, bool is_down, int x, int y);

}
}
}
} // namespace eez::psu::ui::touch
