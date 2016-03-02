#include "psu.h"
#include "gesture.h"

#define MIN_SLIDE_DURATION 100 * 1000UL
#define MIN_SLIDE_DISTANCE 50

#define MIN_TAP_DURATION 0 * 1000UL
#define MAX_TAP_DURATION 250 * 1000UL
#define MAX_TAP_DISTANCE 50

namespace eez {
namespace psu {
namespace gui {
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
    // sliding
    if (time > MIN_SLIDE_DURATION) {
        if (distance_y < -MIN_SLIDE_DISTANCE && abs(distance_y) > abs(distance_x)) gesture = GESTURE_SLIDE_UP;
        if (distance_x > MIN_SLIDE_DISTANCE && abs(distance_x) > abs(distance_y)) gesture = GESTURE_SLIDE_RIGHT;
        if (distance_y > MIN_SLIDE_DISTANCE && abs(distance_y) > abs(distance_x)) gesture = GESTURE_SLIDE_DOWN;
        if (distance_x < -MIN_SLIDE_DISTANCE && abs(distance_x) > abs(distance_y)) gesture = GESTURE_SLIDE_LEFT;
    }

    // tap
    if (time >= MIN_TAP_DURATION && time <= MAX_TAP_DURATION && distance_x < MAX_TAP_DISTANCE && distance_y < MAX_TAP_DISTANCE) {
        gesture = GESTURE_TAP;
    }
}

void push_pointer(unsigned long tick_usec, bool is_down, int x, int y) {
    gesture = GESTURE_NONE;

    if (is_down) {
        if (!last_is_down) {
            distance_x = 0;
            distance_y = 0;
            time = 0;
            gesture = GESTURE_DOWN;
        } else {
            distance_x += x - last_x;
            distance_y += y - last_y;
            time += tick_usec - last_tick_usec;
        }
    } else {
        if (last_is_down) {
            gesture = GESTURE_UP;
            //recognize();
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
