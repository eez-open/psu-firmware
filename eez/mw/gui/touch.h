/*
 * EEZ Middleware
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

#include "eez/mw/gui/touch.h"

namespace eez {
namespace mw {
namespace gui {
namespace touch {

enum EventType {
    TOUCH_NONE,
    TOUCH_DOWN,
    TOUCH_MOVE,
    TOUCH_UP
};

extern EventType g_eventType;
extern int g_x;
extern int g_y;

void init();
void tick(uint32_t tickCount);

void read(bool &isPressed, int &x, int &y);

}
}
}
} // namespace eez::mw::gui::touch
