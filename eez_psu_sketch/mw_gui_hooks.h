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

#include "mw_gui_page.h"

namespace eez {
namespace mw {
namespace gui {

bool executeCriticalTasks(int pageId);

Page *createPageFromId(int pageId);
void onPageChanged();
bool isFocusWidget(const WidgetCursor &widgetCursor);
int getStartPageId();
int getMainPageId();
void errorMessage(const data::Cursor& cursor, data::Value value, void(*ok_callback)() = 0);
bool isWidgetActionEnabledHook(const Widget *widget, bool &result);
int transformStyleHook(const Widget *widget);
bool isAutoRepeatActionHook(int action);

void flushGuiUpdate();

void onPageTouchHook(const WidgetCursor& foundWidget, Event& touchEvent);

bool testExecuteActionOnTouchDownHook(int action);
void executeUserActionHook(int actionId);

void onScaleUpdatedHook(int dataId, bool scaleIsVertical, int scaleWidth, float scaleHeight);

uint16_t getWidgetBackgroundColorHook(const WidgetCursor& widgetCursor, const Style* style);

bool isBlinkingHook(const data::Cursor &cursor, uint8_t id);

void playClickSound();

}
}
} // namespace eez::mw::gui
