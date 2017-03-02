/*
 * EEZ PSU Firmware
 * Copyright (C) 2017-present, Envox d.o.o.
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

#include "gui_page.h"

#define LIST_ITEMS_PER_PAGE 4

namespace eez {
namespace psu {
namespace gui {

class ChSettingsTriggerPage : public Page {
public:
	ChSettingsTriggerPage();

	data::Value getData(const data::Cursor &cursor, uint8_t id);

    void editVoltageTriggerMode();
    void editVoltageTriggerValue();

    void editCurrentTriggerMode();
    void editCurrentTriggerValue();

private:
    static void onVoltageTriggerModeSet(uint8_t value);
    static void onVoltageTriggerValueSet(float value);

    static void onCurrentTriggerModeSet(uint8_t value);
    static void onCurrentTriggerValueSet(float value);
};

class ChSettingsListsPage : public SetPage {
public:
	ChSettingsListsPage();

    int getListSize(uint8_t id);
    float *getFloatList(uint8_t id);
    data::Value getMin(const data::Cursor &cursor, uint8_t id);
    data::Value getMax(const data::Cursor &cursor, uint8_t id);
    data::Value getDef(const data::Cursor &cursor, uint8_t id);
	data::Value getData(const data::Cursor &cursor, uint8_t id);

    void previousPage();
    void nextPage();

    void deleteRow();
    void insertRow();

    void edit();

    bool isFocusWidget(const WidgetCursor &widgetCursor);

	int getDirty();
	void set();
	void discard();

    bool onEncoder(int counter);
    bool onEncoderClicked();

private:
    int m_listVersion;

    float m_voltageList[MAX_LIST_SIZE];
    uint16_t m_voltageListSize;

    float m_currentList[MAX_LIST_SIZE];
    uint16_t m_currentListSize;

    float m_dwellList[MAX_LIST_SIZE];
    uint16_t m_dwellListSize;

    int m_iPage;
    int m_iFocused;
    uint8_t m_focusDataId;

    uint16_t getFocusedIndex();
    uint16_t getMaxListSize();
    uint16_t getNumPages();

    float getFocusedValue();
    void setFocusedValue(float value);
    static void onValueSet(float value);
};

}
}
} // namespace eez::psu::gui
