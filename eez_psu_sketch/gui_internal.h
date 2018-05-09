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

#include "lcd.h"
#include "font.h"
#include "touch.h"

#include "gui_data.h"
#include "gui_view.h"
#include "gui_page.h"

#include "actions.h"
#include "gui_document.h"

namespace eez {
namespace psu {
namespace gui {

#define INTERNAL_PAGE_ID_NONE             -1
#define INTERNAL_PAGE_ID_SELECT_FROM_ENUM -2

class Page;

int getActivePageId();
Page *getActivePage();

bool isActivePage(int pageId);

int getPreviousActivePageId();
Page *getPreviousPage();

bool isActivePageInternal();

void setPage(int pageId);
void replacePage(int pageId, Page *page = 0);
void pushPage(int pageId, Page *page = 0);
void popPage();
bool isPageActiveOrOnStack(int pageId);

font::Font styleGetFont(const Style *style);
void drawText(int pageId, const char *text, int textLength, int x, int y, int w, int h, const Style *style, bool inverse, bool dimmed = false, bool ignoreLuminocity = false, uint16_t *overrideBackgroundColor = 0);
void fillRect(int x, int y, int w, int h);

void pushSelectFromEnumPage(const data::EnumItem *enumDefinition, uint8_t currentValue, bool (*disabledCallback)(uint8_t value), void (*onSet)(uint8_t));

void infoMessage(data::Value value, void (*ok_callback)() = 0);
void infoMessageP(const char *message, void (*ok_callback)() = 0);

void longInfoMessage(data::Value value1, data::Value value2, void (*ok_callback)() = 0);
void longInfoMessageP(const char *message1, const char *message2, void (*ok_callback)() = 0);

void toastMessageP(const char *message1, const char *message2, const char *message3, void (*ok_callback)() = 0);

void errorMessage(const data::Cursor& cursor, data::Value value, void (*ok_callback)() = 0);
void errorMessageP(const char *message, void (*ok_callback)() = 0);

void longErrorMessage(data::Value value1, data::Value value2, void (*ok_callback)() = 0);
void longErrorMessageP(const char *message1, const char *message2, void (*ok_callback)() = 0);

void yesNoDialog(int yesNoPageId, const char *message, void (*yes_callback)(), void (*no_callback)(), void (*cancel_callback)());
void areYouSure(void (*yes_callback)());
void areYouSureWithMessage(const char *message, void (*yes_callback)());

void dialogYes();
void dialogNo();
void dialogCancel();
void dialogOk();
void dialogLater();

void errorMessageAction();

void onLastErrorEventAction();

void lockFrontPanel();
void unlockFrontPanel();

int transformStyle(const Widget *widget);

extern WidgetCursor g_foundWidgetAtDown;

void selectChannel();
extern Channel *g_channel;

void setFocusCursor(const data::Cursor& cursor, uint8_t dataId);

extern data::Cursor g_focusCursor;
extern uint8_t g_focusDataId;
extern data::Value g_focusEditValue;
bool wasFocusWidget(const WidgetCursor &widgetCursor);
bool isFocusWidget(const WidgetCursor &widgetCursor);
bool isFocusChanged();

void upDown();

void channelToggleOutput();
void channelInitiateTrigger();
void channelSetToFixed();
void channelEnableOutput();

void standBy();
void turnDisplayOff();
void reset();

uint8_t getTextMessageVersion();

////////////////////////////////////////////////////////////////////////////////
// GUI definition document accessor functions

extern Styles *g_styles;
extern Document *g_document;

inline OBJ_OFFSET getListItemOffset(const List &list, int index, int listItemSize) {
    return list.first + index * listItemSize;
}

inline OBJ_OFFSET getStyleOffset(int styleID) {
    if (styleID == 0) {
        return 0;
    }
    return getListItemOffset(*g_styles, styleID - 1, sizeof(Style));
}

inline OBJ_OFFSET getCustomWidgetOffset(int customWidgetID) {
    if (customWidgetID == 0) {
        return 0;
    }
    return getListItemOffset(g_document->customWidgets, customWidgetID - 1, sizeof(CustomWidget));
}

inline OBJ_OFFSET getPageOffset(int pageID) {
    return getListItemOffset(g_document->pages, pageID, sizeof(Widget));
}

#define DECL_WIDGET_STYLE(var, widget) DECL_STYLE(var, transformStyle(widget))
#define DECL_STYLE(var, styleID) const Style *var = (const Style *)(styles + getStyleOffset(styleID))

#define DECL_CUSTOM_WIDGET(var, customWidgetID) const CustomWidget *var = (const CustomWidget *)(document + getCustomWidgetOffset(customWidgetID))

#define DECL_WIDGET(var, widgetOffset) const Widget *var = (const Widget *)(document + (widgetOffset))
#define DECL_WIDGET_SPECIFIC(type, var, widget) const type *var = (const type *)(document + (widget)->specific)
#define DECL_STRING(var, offset) const char *var = (const char *)(document + (offset))
#define DECL_BITMAP(var, offset) const Bitmap *var = (const Bitmap *)(document + (offset))
#define DECL_STRUCT_WITH_OFFSET(Struct, var, offset) const Struct *var = (const Struct *)(document + (offset))

////////////////////////////////////////////////////////////////////////////////

}
}
} // namespace eez::psu::ui
