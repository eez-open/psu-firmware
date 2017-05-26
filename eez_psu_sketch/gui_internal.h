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
#include "gesture.h"

#include "gui_data.h"
#include "gui_view.h"
#include "gui_page.h"

#include "actions.h"
#include "gui_document.h"

#if defined(EEZ_PSU_ARDUINO_MEGA)
#include "arduino_util.h"
#endif

namespace eez {
namespace psu {
namespace gui {

class Page;

int getActivePageId();
Page *getActivePage();

int getPreviousActivePageId();
Page *getPreviousPage();

bool isActivePageInternal();

void setPage(int pageId);
void replacePage(int pageId, Page *page = 0);
void pushPage(int pageId, Page *page = 0);
void popPage();
bool isPageActiveOrOnStack(int pageId);

font::Font styleGetFont(const Style *style);
void drawText(const char *text, int textLength, int x, int y, int w, int h, const Style *style, bool inverse, bool dimmed = false);
void fillRect(int x, int y, int w, int h);

void pushSelectFromEnumPage(const data::EnumItem *enumDefinition, uint8_t currentValue, bool (*disabledCallback)(uint8_t value), void (*onSet)(uint8_t));

void infoMessage(data::Value value, void (*ok_callback)() = 0);
void infoMessageP(const char *message PROGMEM, void (*ok_callback)() = 0);

void longInfoMessage(data::Value value1, data::Value value2, void (*ok_callback)() = 0);
void longInfoMessageP(const char *message1 PROGMEM, const char *message2 PROGMEM, void (*ok_callback)() = 0);

void toastMessageP(const char *message1 PROGMEM, const char *message2 PROGMEM, const char *message3 PROGMEM, void (*ok_callback)() = 0);

void errorMessage(const data::Cursor& cursor, data::Value value, void (*ok_callback)() = 0);
void errorMessageP(const char *message PROGMEM, void (*ok_callback)() = 0);

void longErrorMessage(data::Value value1, data::Value value2, void (*ok_callback)() = 0);
void longErrorMessageP(const char *message1 PROGMEM, const char *message2 PROGMEM, void (*ok_callback)() = 0);

void yesNoDialog(int yesNoPageId, const char *message PROGMEM, void (*yes_callback)(), void (*no_callback)(), void (*cancel_callback)());
void areYouSure(void (*yes_callback)());
void areYouSureWithMessage(const char *message PROGMEM, void (*yes_callback)());

void showAsyncOperationInProgress(const char *message, void (*checkStatus)());

void dialogYes();
void dialogNo();
void dialogCancel();
void dialogOk();
void dialogLater();

void errorMessageAction();

void onLastErrorEventAction();

void lockFrontPanel();

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

#if defined(EEZ_PSU_ARDUINO_MEGA)

#define DECL_WIDGET_STYLE(var, widget) DECL_STYLE(var, (widget)->style)

#define DECL_STYLE(var, styleId) \
    Style var##_buffer; \
    arduino_util::prog_read_buffer(styles + getStyleOffset(styleId), (uint8_t *)&var##_buffer, sizeof(Style)); \
    const Style *var = &var##_buffer

#define DECL_WIDGET(var, widgetOffset) \
    Widget var##_buffer; \
    arduino_util::prog_read_buffer(document + (widgetOffset), (uint8_t *)&var##_buffer, sizeof(Widget)); \
    const Widget *var = &var##_buffer

#define DECL_WIDGET_SPECIFIC(type, var, widget) \
    type var##_buffer; \
    arduino_util::prog_read_buffer(document + (widget)->specific, (uint8_t *)&var##_buffer, sizeof(type)); \
    const type *var = &var##_buffer

#define DECL_STRING(var, offset) \
    char var##_buffer[128]; \
    strncpy_P(var##_buffer, (const char *)(document + (offset)), sizeof(var##_buffer) - 1); \
    var##_buffer[sizeof(var##_buffer) - 1] = 0; \
    const char *var = var##_buffer

#define DECL_BITMAP(var, offset) \
    Bitmap var##_buffer; \
    arduino_util::prog_read_buffer(document + (offset), (uint8_t *)&var##_buffer, sizeof(Bitmap)); \
    const Bitmap *var = &var##_buffer

#define DECL_STRUCT_WITH_OFFSET(Struct, var, offset) \
    Struct var##_buffer; \
    arduino_util::prog_read_buffer(document + (offset), (uint8_t *)&var##_buffer, sizeof(Struct)); \
    const Struct *var = &var##_buffer

#else

#define DECL_WIDGET_STYLE(var, widget) DECL_STYLE(var, transformStyle(widget))
#define DECL_STYLE(var, styleID) const Style *var = (const Style *)(styles + getStyleOffset(styleID))

#define DECL_CUSTOM_WIDGET(var, customWidgetID) const CustomWidget *var = (const CustomWidget *)(document + getCustomWidgetOffset(customWidgetID))

#define DECL_WIDGET(var, widgetOffset) const Widget *var = (const Widget *)(document + (widgetOffset))
#define DECL_WIDGET_SPECIFIC(type, var, widget) const type *var = (const type *)(document + (widget)->specific)
#define DECL_STRING(var, offset) const char *var = (const char *)(document + (offset))
#define DECL_BITMAP(var, offset) const Bitmap *var = (const Bitmap *)(document + (offset))
#define DECL_STRUCT_WITH_OFFSET(Struct, var, offset) const Struct *var = (const Struct *)(document + (offset))

#endif

////////////////////////////////////////////////////////////////////////////////

}
}
} // namespace eez::psu::ui
