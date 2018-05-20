/*
* EEZ PSU Firmware
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

#include "mw_gui_gui.h"

namespace eez {
namespace app {

using namespace mw::gui;

namespace gui {

using namespace mw::gui;

void init();
void tick(uint32_t tickCount);
void touchHandling(uint32_t tickCount);

void showWelcomePage();
void showStandbyPage();
void showEnteringStandbyPage();
void showEthernetInit();

void showAsyncOperationInProgress(const char *message, void(*checkStatus)() = 0);
void hideAsyncOperationInProgress();

void showProgressPage(const char *message, void(*abortCallback)() = 0);
bool updateProgressPage(size_t processedSoFar, size_t totalSize);
void hideProgressPage();

void setTextMessage(const char *message, unsigned int len);
void clearTextMessage();
const char *getTextMessage();

void longErrorMessage(data::Value value1, data::Value value2, void(*ok_callback)() = 0);
void longErrorMessageP(const char *message1, const char *message2, void(*ok_callback)() = 0);

void infoMessage(data::Value value, void(*ok_callback)() = 0);
void infoMessageP(const char *message, void(*ok_callback)() = 0);

void longInfoMessage(data::Value value1, data::Value value2, void(*ok_callback)() = 0);
void longInfoMessageP(const char *message1, const char *message2, void(*ok_callback)() = 0);

void toastMessageP(const char *message1, const char *message2, const char *message3, void(*ok_callback)() = 0);

void errorMessageP(const char *message, void(*ok_callback)() = 0);

void yesNoDialog(int yesNoPageId, const char *message, void(*yes_callback)(), void(*no_callback)(), void(*cancel_callback)());
void areYouSure(void(*yes_callback)());
void areYouSureWithMessage(const char *message, void(*yes_callback)());

void dialogYes();
void dialogNo();
void dialogCancel();
void dialogOk();
void dialogLater();

void channelToggleOutput();
void channelInitiateTrigger();
void channelSetToFixed();
void channelEnableOutput();

void standBy();
void turnDisplayOff();
void reset();

void selectChannel();
extern Channel *g_channel;

void onLastErrorEventAction();

void errorMessageAction();
void lockFrontPanel();
void unlockFrontPanel();

uint8_t getTextMessageVersion();

extern data::Cursor g_focusCursor;
extern uint8_t g_focusDataId;
extern data::Value g_focusEditValue;
void setFocusCursor(const data::Cursor& cursor, uint8_t dataId);
bool isFocusChanged();

}
}
} // namespace eez::app::gui
