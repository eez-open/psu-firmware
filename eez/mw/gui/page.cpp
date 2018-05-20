/*
 * EEZ Middleware
 * Copyright (C) 2016-present, Envox d.o.o.
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

#include "eez/mw/mw.h"

#if OPTION_DISPLAY

#include "eez/mw/gui/gui.h"
#include "eez/mw/gui/page.h"
#include "eez/mw/gui/draw.h"

namespace eez {
namespace mw {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

void Page::pageWillAppear() {
}

bool Page::onEncoder(int counter) {
    return false;
}

bool Page::onEncoderClicked() {
    return false;
}

////////////////////////////////////////////////////////////////////////////////

void SetPage::edit() {
}

void SetPage::onSetValue(float value) {
	popPage();
	SetPage *page = (SetPage *)getActivePage();
	page->setValue(value);
}

void SetPage::setValue(float value) {
}

void SetPage::discard() {
	popPage();
}

////////////////////////////////////////////////////////////////////////////////

SelectFromEnumPage::SelectFromEnumPage(const data::EnumItem *enumDefinition_, uint8_t currentValue_, bool (*disabledCallback_)(uint8_t value), void (*onSet_)(uint8_t))
    : enumDefinition(enumDefinition_)
    , currentValue(currentValue_)
    , disabledCallback(disabledCallback_)
    , onSet(onSet_)
{
}

bool SelectFromEnumPage::isDisabled(int i) {
    return disabledCallback && disabledCallback(enumDefinition[i].value);
}

void SelectFromEnumPage::refresh() {
	const Style *containerStyle = getSelectFromEnumContainerStyle();
	const Style *itemStyle = getSelectFromEnumItemStyle();
	const Style *disabledItemStyle = getSelectFromEnumDisabledItemStyle();

    font::Font font = styleGetFont(itemStyle);

    // calculate geometry
    itemHeight = itemStyle->padding_vertical + font.getHeight() + itemStyle->padding_vertical;
    itemWidth = 0;

    int i;

    char text[64];

    for (i = 0; enumDefinition[i].menuLabel; ++i) {
        getItemLabel(i, text, sizeof(text));
        int width = lcd::measureStr(text, -1, font);
        if (width > itemWidth) {
            itemWidth = width;
        }
    }

    itemWidth = itemStyle->padding_horizontal + itemWidth + itemStyle->padding_horizontal;

    numItems = i;

    width = containerStyle->padding_horizontal + itemWidth + containerStyle->padding_horizontal;
    if (width > lcd::getDisplayWidth()) {
        width = lcd::getDisplayWidth();
    }

    height = containerStyle->padding_vertical + numItems * itemHeight + containerStyle->padding_vertical;
    if (height > lcd::getDisplayHeight()) {
        height = lcd::getDisplayHeight();
    }

    x = (lcd::getDisplayWidth() - width) / 2;
    y = (lcd::getDisplayHeight() - height) / 2;

    // draw background
    lcd::setColor(containerStyle->background_color);
    lcd::fillRect(x, y, x + width - 1, y + height - 1);

    // draw labels
    for (i = 0; enumDefinition[i].menuLabel; ++i) {
        int xItem, yItem;
        getItemPosition(i, xItem, yItem);

        getItemLabel(i, text, sizeof(text));
        drawText(getActivePageId(), text, -1, xItem, yItem, itemWidth, itemHeight, isDisabled(i) ? disabledItemStyle : itemStyle, false, false, false, NULL);
    }
}

bool SelectFromEnumPage::drawTick() {
    return false;
}

WidgetCursor SelectFromEnumPage::findWidget(int x, int y) {
    int i;

    for (i = 0; enumDefinition[i].menuLabel; ++i) {
        int xItem, yItem;
        getItemPosition(i, xItem, yItem);
        if (!isDisabled(i) && x >= xItem && x < xItem + itemWidth && y >= yItem && y < yItem + itemHeight) {
            break;
        }
    }

    return WidgetCursor(i + 1, x, y, -1, 0, 0);
}

void SelectFromEnumPage::drawWidget(const WidgetCursor &widgetCursor, bool selected) {
    int itemIndex = widgetCursor.widgetOffset - 1;
    if (itemIndex >= 0 && itemIndex < numItems) {
        int xItem, yItem;
        getItemPosition(widgetCursor.widgetOffset - 1, xItem, yItem);

        char text[64];
        getItemLabel(itemIndex, text, sizeof(text));

		const Style *itemStyle = getSelectFromEnumItemStyle();

        drawText(getActivePageId(), text, -1, xItem, yItem, itemWidth, itemHeight, itemStyle, selected, false, false, NULL);
    }
}

int SelectFromEnumPage::getAction(const WidgetCursor &widgetCursor) {
    int itemIndex = widgetCursor.widgetOffset - 1;
    if (itemIndex >= 0 && itemIndex < numItems) {
        return ACTION_ID_INTERNAL_SELECT_ENUM_ITEM;
    } else {
        return ACTION_ID_INTERNAL_SHOW_PREVIOUS_PAGE;
    }
}

void SelectFromEnumPage::selectEnumItem() {
    int itemIndex = g_foundWidgetAtDown.widgetOffset - 1;
    onSet(enumDefinition[itemIndex].value);
}

void SelectFromEnumPage::getItemPosition(int itemIndex, int &xItem, int &yItem) {
	const Style *containerStyle = getSelectFromEnumContainerStyle();

    xItem = x + containerStyle->padding_horizontal;
    yItem = y + containerStyle->padding_vertical + itemIndex * itemHeight;
}

void SelectFromEnumPage::getItemLabel(int itemIndex, char *text, int count) {
    if (enumDefinition[itemIndex].value == currentValue) {
        text[0] = (char)142;
    } else {
        text[0] = (char)141;
    }

    text[1] = ' ';

    strncpy(text + 2, enumDefinition[itemIndex].menuLabel, count - 3);

    text[count - 1] = 0;
}

}
}
} // namespace eez::mw::gui

#endif