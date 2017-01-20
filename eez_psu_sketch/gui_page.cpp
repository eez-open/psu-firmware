/*
 * EEZ PSU Firmware
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

#include "psu.h"

#include "gui_internal.h"
#include "gui_data_snapshot.h"

namespace eez {
namespace psu {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

void Page::pageWillAppear() {
}

void Page::takeSnapshot(data::Snapshot *snapshot) {
}

data::Value Page::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	return data::Value();
}

void Page::onEncoder(int counter) {
}

bool Page::setFocusWidget(const WidgetCursor &widgetCursor) {
    return false;
}

////////////////////////////////////////////////////////////////////////////////

void SetPage::takeSnapshot(data::Snapshot *snapshot) {
	snapshot->flags.setPageDirty = getDirty();
}

data::Value SetPage::getData(const data::Cursor &cursor, uint8_t id, data::Snapshot *snapshot) {
	if (id == DATA_ID_SET_PAGE_DIRTY) {
		return data::Value(snapshot->flags.setPageDirty);
	}
	return data::Value();
}

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

SelectFromEnumPage::SelectFromEnumPage(const data::EnumItem *enumDefinition_, uint8_t currentValue_, uint8_t disabledValue_, void (*onSet_)(uint8_t))
    : enumDefinition(enumDefinition_)
    , currentValue(currentValue_)
    , disabledValue(disabledValue_)
    , onSet(onSet_)
{
}

void SelectFromEnumPage::refresh() {
    DECL_STYLE(containerStyle, STYLE_ID_SELECT_ENUM_ITEM_POPUP_CONTAINER);
    DECL_STYLE(itemStyle, STYLE_ID_SELECT_ENUM_ITEM_POPUP_ITEM);
    DECL_STYLE(disabledItemStyle, STYLE_ID_SELECT_ENUM_ITEM_POPUP_DISABLED_ITEM);

    font::Font font = styleGetFont(itemStyle);

    // calculate geometry
    itemHeight = itemStyle->padding_vertical + font.getHeight() + itemStyle->padding_vertical;
    itemWidth = 0;

    int i;

    char text[64];

    for (i = 0; enumDefinition[i].label; ++i) {
        getItemLabel(i, text, sizeof(text));
        int width = lcd::lcd.measureStr(text, -1, font);
        if (width > itemWidth) {
            itemWidth = width;
        }
    }

    itemWidth = itemStyle->padding_horizontal + itemWidth + itemStyle->padding_horizontal;

    numItems = i;

    width = containerStyle->padding_horizontal + itemWidth + containerStyle->padding_horizontal;
    if (width > lcd::lcd.getDisplayXSize()) {
        width = lcd::lcd.getDisplayXSize();
    }
    
    height = containerStyle->padding_vertical + numItems * itemHeight + containerStyle->padding_vertical;
    if (height > lcd::lcd.getDisplayYSize()) {
        height = lcd::lcd.getDisplayYSize();
    }

    x = (lcd::lcd.getDisplayXSize() - width) / 2;
    y = (lcd::lcd.getDisplayYSize() - height) / 2;

    // draw background
    lcd::lcd.setColor(containerStyle->background_color);
    lcd::lcd.fillRect(x, y, x + width - 1, y + height - 1);

    // draw labels
    for (i = 0; enumDefinition[i].label; ++i) {
        int xItem, yItem;
        getItemPosition(i, xItem, yItem);

        getItemLabel(i, text, sizeof(text));
        drawText(text, -1, xItem, yItem, itemWidth, itemHeight, i == disabledValue ? disabledItemStyle : itemStyle, false);
    }
}

bool SelectFromEnumPage::drawTick() {
    return false;
}

WidgetCursor SelectFromEnumPage::findWidget(int x, int y) {
    int i;

    for (i = 0; enumDefinition[i].label; ++i) {
        int xItem, yItem;
        getItemPosition(i, xItem, yItem);
        if (i != disabledValue && x >= xItem && x < xItem + itemWidth && y >= yItem && y < yItem + itemHeight) {
            break;
        }
    }

    return WidgetCursor(i + 1, x, y, -1);
}

void SelectFromEnumPage::drawWidget(const WidgetCursor &widgetCursor, bool selected) {
    int itemIndex = widgetCursor.widgetOffset - 1;
    if (itemIndex >= 0 && itemIndex < numItems) {
        int xItem, yItem;
        getItemPosition(widgetCursor.widgetOffset - 1, xItem, yItem);

        char text[64];
        getItemLabel(itemIndex, text, sizeof(text));

        DECL_STYLE(itemStyle, STYLE_ID_SELECT_ENUM_ITEM_POPUP_ITEM);

        drawText(text, -1, xItem, yItem, itemWidth, itemHeight, itemStyle, selected);
    }
}

ActionType SelectFromEnumPage::getAction(const WidgetCursor &widgetCursor) {
    int itemIndex = widgetCursor.widgetOffset - 1;
    if (itemIndex >= 0 && itemIndex < numItems) {
        return ACTION_ID_SELECT_ENUM_ITEM;
    } else {
        return ACTION_ID_SHOW_PREVIOUS_PAGE;
    }
}

void SelectFromEnumPage::selectEnumItem() {
    int itemIndex = g_foundWidgetAtDown.widgetOffset - 1;
    onSet(enumDefinition[itemIndex].value);
}

void SelectFromEnumPage::getItemPosition(int itemIndex, int &xItem, int &yItem) {
    DECL_STYLE(containerStyle, STYLE_ID_SELECT_ENUM_ITEM_POPUP_CONTAINER);

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

    strncpy_P(text + 2, enumDefinition[itemIndex].label, count - 3);
    
    text[count - 1] = 0;
}

}
}
} // namespace eez::psu::gui
