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

#pragma once

#include "touch_calibration.h"

namespace eez {
namespace psu {
namespace gui {
namespace touch {

extern void init();
extern void tick(uint32_t tick_usec);

enum EventType {
    TOUCH_NONE,
    TOUCH_DOWN,
    TOUCH_MOVE,
    TOUCH_UP
};

extern EventType event_type;
extern int x;
extern int y;

extern bool directIsPressed();

#ifdef EEZ_PSU_SIMULATOR
void touch_write(bool is_pressed, int x, int y);
#endif

}
}
}
} // namespace eez::psu::ui::touch
