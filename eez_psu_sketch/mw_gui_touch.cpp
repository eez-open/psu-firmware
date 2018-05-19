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

#include "mw_mw.h"

#if OPTION_DISPLAY

#include "mw_gui_touch.h"
#include "mw_gui_touch_filter.h"

#define CONF_GUI_TOUCH_READ_FREQ_MS 10

 ////////////////////////////////////////////////////////////////////////////////

namespace eez {
namespace mw {
namespace gui {
namespace touch {

EventType g_eventType = TOUCH_NONE;
int g_x = -1;
int g_y = -1;

bool g_touchIsPressed = false;
int g_touchX = -1;
int g_touchY = -1;

uint32_t g_lastTickCount = 0;

////////////////////////////////////////////////////////////////////////////////

void tick(uint32_t tickCount) {
    if (g_lastTickCount == 0 || tickCount - g_lastTickCount > CONF_GUI_TOUCH_READ_FREQ_MS * 1000UL) {
    	read(g_touchIsPressed, g_touchX, g_touchY);

        g_touchIsPressed = filter(g_touchIsPressed, g_touchX, g_touchY);
        if (g_touchIsPressed) {
            transform(g_touchX, g_touchY);
        }

        g_lastTickCount = tickCount;
    }

    if (g_touchIsPressed) {
        if (g_touchX != -1 && g_touchY != -1) {
            g_x = g_touchX;
			g_y = g_touchY;

            if (g_eventType == TOUCH_NONE || g_eventType == TOUCH_UP) {
                g_eventType = TOUCH_DOWN;
            } else {
                if (g_eventType == TOUCH_DOWN) {
                    g_eventType = TOUCH_MOVE;
                }
            }
            return;
        }
    }

    if (g_eventType == TOUCH_DOWN || g_eventType == TOUCH_MOVE) {
        g_eventType = TOUCH_UP;
    } else if (g_eventType == TOUCH_UP) {
        g_eventType = TOUCH_NONE;
        g_x = -1;
        g_y = -1;
    }
}

}
}
}
} // namespace eez::mw::gui::touch

#endif
