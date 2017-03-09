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
 
#pragma once

#include "gui_internal.h"

namespace eez {
namespace psu {
namespace gui {

namespace data {
struct Snapshot;
}

class Page {
public:
    virtual ~Page() {}

	virtual void pageWillAppear();
	
    virtual int getListLength(uint8_t id);
    virtual float *getFloatList(uint8_t id);
    virtual data::Value getData(const data::Cursor &cursor, uint8_t id);
    virtual data::Value getMin(const data::Cursor &cursor, uint8_t id);
    virtual data::Value getMax(const data::Cursor &cursor, uint8_t id);
    virtual data::Value getDef(const data::Cursor &cursor, uint8_t id);
    virtual bool setData(const data::Cursor &cursor, uint8_t id, data::Value value);
    virtual bool onEncoder(int counter);
    virtual bool onEncoderClicked();
};

class SetPage : public Page {
public:
	data::Value getData(const data::Cursor &cursor, uint8_t id);

	virtual void edit();
	virtual int getDirty() = 0;
	virtual void set() = 0;
	virtual void discard();

protected:
	int editDataId;

	static void onSetValue(float value);
	virtual void setValue(float value);
};

class InternalPage : public Page {
public:
    virtual void refresh() = 0;
    virtual bool drawTick() = 0;
    virtual WidgetCursor findWidget(int x, int y) = 0;
    virtual void drawWidget(const WidgetCursor &widgetCursor, bool selected) = 0;
    virtual ActionType getAction(const WidgetCursor &widgetCursor) = 0;
};

class SelectFromEnumPage : public InternalPage {
public:
    SelectFromEnumPage(const data::EnumItem *enumDefinition_, uint8_t currentValue_, uint8_t disabledValue_, void (*onSet_)(uint8_t));

    void refresh();
    bool drawTick();
    WidgetCursor findWidget(int x, int y);
    void drawWidget(const WidgetCursor &widgetCursor, bool selected);
    ActionType getAction(const WidgetCursor &widgetCursor);

    void selectEnumItem();

private:
    const data::EnumItem *enumDefinition;
    uint8_t currentValue;
    uint8_t disabledValue;
    void (*onSet)(uint8_t);

    int numItems;
    int x;
    int y;
    int width;
    int height;
    int itemWidth;
    int itemHeight;

    void getItemPosition(int itemIndex, int &x, int &y);
    void getItemLabel(int itemIndex, char *text, int count);
};

}
}
} // namespace eez::psu::gui
