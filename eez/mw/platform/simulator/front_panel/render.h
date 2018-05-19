/*
 * EEZ Middleware
 * Copyright (C) 2018-present, Envox d.o.o.
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

#include "eez/mw/platform/simulator/front_panel/data.h"
#include "eez/mw/platform/simulator/imgui/window.h"

namespace eez {
namespace platform {
namespace simulator {
namespace front_panel {

imgui::WindowDefinition *getWindowDefinition(int w, int h);
void render(platform::simulator::imgui::Window *window, void *data);

}
}
}
} // namespace eez::platform::simulator::front_panel;
