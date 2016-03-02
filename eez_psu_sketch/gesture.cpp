#include "psu.h"
#include "gesture.h"

#define MIN_SLIDE_DURATION 100 * 1000UL
#define MIN_SLIDE_DISTANCE 50

//#define MIN_TAP_DURATION 0 * 1000UL
//#define MAX_TAP_DURATION 250 * 1000UL
#define MAX_TAP_DISTANCE 150

namespace eez {
namespace psu {
namespace gui {
namespace gesture {

GestureType gesture = GESTURE_NONE;
int start_x;
int start_y;

static unsigned long last_tick_usec;
static bool last_is_down;
static int last_x;
static int last_y;

static long distance_x;
static long distance_y;
static unsigned long time;
static unsigned long count;

void recognize() {
    DebugTraceF("Distance X: %ld", distance_x);
    DebugTraceF("Distance Y: %ld", distance_y);
    DebugTraceF("Time: %lu", time);
    DebugTraceF("Count: %lu", count);

    // tap
    if (/*time >= MIN_TAP_DURATION && time <= MAX_TAP_DURATION && */abs(distance_x) < MAX_TAP_DISTANCE && abs(distance_y) < MAX_TAP_DISTANCE) {
        gesture = GESTURE_TAP;
        return;
    }

    // sliding
    if (time > MIN_SLIDE_DURATION) {
        if (distance_y < -MIN_SLIDE_DISTANCE && abs(distance_y) > abs(distance_x)) {
            gesture = GESTURE_SLIDE_UP;
            return;
        }
        if (distance_x > MIN_SLIDE_DISTANCE && abs(distance_x) > abs(distance_y)) {
            gesture = GESTURE_SLIDE_RIGHT;
            return;
        }
        if (distance_y > MIN_SLIDE_DISTANCE && abs(distance_y) > abs(distance_x)) {
            gesture = GESTURE_SLIDE_DOWN;
            return;
        }
        if (distance_x < -MIN_SLIDE_DISTANCE && abs(distance_x) > abs(distance_y)) {
            gesture = GESTURE_SLIDE_LEFT;
            return;
        }
    }
}

void push_pointer(unsigned long tick_usec, bool is_down, int x, int y) {
    gesture = GESTURE_NONE;

    if (is_down) {
        if (!last_is_down) {
            start_x = x;
            start_y = y;
            distance_x = 0;
            distance_y = 0;
            time = 0;
            count = 1;
        } else {
            distance_x += x - last_x;
            distance_y += y - last_y;
            time += tick_usec - last_tick_usec;
            count++;
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
