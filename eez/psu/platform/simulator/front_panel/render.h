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

#include "eez/psu/platform/simulator/front_panel/data.h"
#include "eez/platform/simulator/imgui/window.h"

namespace eez {
namespace psu {
namespace simulator {
namespace front_panel {

platform::simulator::imgui::WindowDefinition *getWindowDefinition(int w, int h);
void render(platform::simulator::imgui::Window *window, Data *data);

}
}
}
} // namespace eez::psu::simulator::front_panel;
