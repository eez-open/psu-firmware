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

#include "ethernet.h"

namespace eez {
namespace psu {
/// GUI for local control using TFT with touch
namespace gui {

void init();
void tick(uint32_t tick_usec);

void refreshPage();

void showWelcomePage();
void showStandbyPage();
void showEnteringStandbyPage();
void showEthernetInit();


}
}
} // namespace eez::psu::gui
