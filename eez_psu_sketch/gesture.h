#pragma once

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

extern GestureType gesture_type;
extern int start_x;
extern int start_y;

void tick(unsigned long tick_usec);

}
}
}
} // namespace eez::psu::ui::touch
