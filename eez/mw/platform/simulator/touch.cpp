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

#include "eez/mw/mw.h"

#if OPTION_DISPLAY

#include "eez/mw/gui/touch.h"

namespace eez {
namespace mw {
namespace gui {
namespace touch {

static bool g_isPressedState = false;
static int g_xState = -1;
static int g_yState = -1;

void init() {
}

void read(bool &isPressed, int &x, int &y) {
    isPressed = g_isPressedState;
    x = g_xState;
    y = g_yState;
}

void write(bool isPressed, int x, int y) {
    g_isPressedState = isPressed;
    g_xState = x;
    g_yState = y;
}

}
}
}
} // namespace eez::mw::gui::touch

#endif
