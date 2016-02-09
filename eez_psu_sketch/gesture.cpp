#include "psu.h"
#include "gesture.h"

#define MIN_SLIDE_DURATION 100 * 1000UL
#define MIN_SLIDE_DISTANCE 50

namespace eez {
namespace psu {
namespace ui {
namespace gesture {

unsigned long last_tick_usec;
bool last_is_down;
int last_x;
int last_y;

int distance_x;
int distance_y;
unsigned long time;

GestureType gesture = GESTURE_NONE;

void recognize() {
    DebugTrace("Start gesture recognize %ul", time);
    
    if (time > MIN_SLIDE_DURATION) {
        DebugTrace("Gesture recognize time passed");

        if (distance_y < -MIN_SLIDE_DISTANCE && abs(distance_y) > abs(distance_x)) gesture = GESTURE_SLIDE_UP;
        if (distance_x > MIN_SLIDE_DISTANCE && abs(distance_x) > abs(distance_y)) gesture = GESTURE_SLIDE_RIGHT;
        if (distance_y > MIN_SLIDE_DISTANCE && abs(distance_y) > abs(distance_x)) gesture = GESTURE_SLIDE_DOWN;
        if (distance_x < -MIN_SLIDE_DISTANCE && abs(distance_x) > abs(distance_y)) gesture = GESTURE_SLIDE_LEFT;
    }

    if (gesture != GESTURE_NONE) {
        DebugTrace("Gesture: %d", gesture);
    }
    
    DebugTrace("End gesture recognize");
}

void push_pointer(unsigned long tick_usec, bool is_down, int x, int y) {
    gesture = GESTURE_NONE;

    if (is_down) {
        DebugTrace("%d, %d", x, y);

        if (!last_is_down) {
            distance_x = 0;
            distance_y = 0;
            time = 0;
        } else {
            distance_x += x - last_x;
            distance_y += y - last_y;
            time += tick_usec - last_tick_usec;
        }
    } else {
        if (last_is_down) {
            recognize();
        }
    }

    last_tick_usec = tick_usec;
    last_is_down = is_down;
    last_x = x;
    last_y = y;
}

}
}
}
} // namespace eez::psu::ui::gesture
