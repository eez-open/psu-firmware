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

#include "eez/mw/gui/hooks.h"
#include "eez/mw/gui/internal.h"

namespace eez {
namespace mw {

typedef void(*ActionExecFunc)();

/// GUI for local control using TFT with touch
namespace gui {

void gui_init();
void gui_tick(uint32_t tick_usec);
void gui_touchHandling(uint32_t tick_usec);

void refreshPage();

void showPage(int pageId);
void pushPage(int pageId, Page *page = 0);
void popPage();

extern uint32_t g_showPageTime;

int getActivePageId();
Page *getActivePage();
bool isActivePage(int pageId);

int getPreviousPageId();
Page *getPreviousPage();

void executeAction(int actionId);

void pushSelectFromEnumPage(const data::EnumItem *enumDefinition, uint8_t currentValue, bool(*disabledCallback)(uint8_t value), void(*onSet)(uint8_t));

extern WidgetCursor g_foundWidgetAtDown;

}
}
} // namespace eez::mw::gui
