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

#include "mw_gui_touch.h"

namespace eez {
namespace psu {
namespace gui {
namespace touch {

void init();
void tick(uint32_t tick_usec);

#if defined(EEZ_PLATFORM_SIMULATOR) || defined(EEZ_PLATFORM_STM32)
void touch_write(bool is_pressed, int x, int y);
#endif

}
}
}
} // namespace eez::psu::gui::touch
