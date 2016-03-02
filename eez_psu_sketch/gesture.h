#pragma once

#include "UTouch.h"

namespace eez {
namespace psu {
namespace gui {
namespace gesture {

enum GestureType {
    GESTURE_NONE,

    GESTURE_TAP,

    GESTURE_SLIDE_UP,
    GESTURE_SLIDE_RIGHT,
    GESTURE_SLIDE_DOWN,
    GESTURE_SLIDE_LEFT
};

extern GestureType gesture;
extern int start_x;
extern int start_y;

void push_pointer(unsigned long tick_usec, bool is_down, int x, int y);

}
}
}
} // namespace eez::psu::ui::touch
