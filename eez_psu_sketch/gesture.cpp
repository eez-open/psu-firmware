/*
 * EEZ PSU Firmware
 * Copyright (C) 2015-present, Envox d.o.o.
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

#include "psu.h"

#if OPTION_DISPLAY

#include "gesture.h"
#include "touch.h"

#define MIN_SLIDE_DURATION 100 * 1000UL
#define MIN_SLIDE_DISTANCE 50

//#define MIN_TAP_DURATION 0 * 1000UL
//#define MAX_TAP_DURATION 250 * 1000UL
#define MAX_TAP_DISTANCE 150

namespace eez {
namespace psu {
namespace gui {
namespace gesture {

GestureType gesture_type = GESTURE_NONE;
int start_x;
int start_y;

static unsigned long last_tick_usec;
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
        gesture_type = GESTURE_TAP;
        return;
    }

    // sliding
    if (time > MIN_SLIDE_DURATION) {
        if (distance_y < -MIN_SLIDE_DISTANCE && abs(distance_y) > abs(distance_x)) {
            gesture_type = GESTURE_SLIDE_UP;
            return;
        }
        if (distance_x > MIN_SLIDE_DISTANCE && abs(distance_x) > abs(distance_y)) {
            gesture_type = GESTURE_SLIDE_RIGHT;
            return;
        }
        if (distance_y > MIN_SLIDE_DISTANCE && abs(distance_y) > abs(distance_x)) {
            gesture_type = GESTURE_SLIDE_DOWN;
            return;
        }
        if (distance_x < -MIN_SLIDE_DISTANCE && abs(distance_x) > abs(distance_y)) {
            gesture_type = GESTURE_SLIDE_LEFT;
            return;
        }
    }
}

void tick(unsigned long tick_usec) {
    gesture_type = GESTURE_NONE;

    if (touch::event_type == touch::TOUCH_DOWN) {
        start_x = touch::x;
        start_y = touch::y;
        distance_x = 0;
        distance_y = 0;
        time = 0;
        count = 1;
    } else if (touch::event_type == touch::TOUCH_MOVE) {
        distance_x += touch::x - last_x;
        distance_y += touch::y - last_y;
        time += tick_usec - last_tick_usec;
        count++;
    } else if (touch::event_type == touch::TOUCH_UP) {
        recognize();
    }

    last_tick_usec = tick_usec;
    last_x = touch::x;
    last_y = touch::y;
}

}
}
}
} // namespace eez::psu::ui::gesture

#endif