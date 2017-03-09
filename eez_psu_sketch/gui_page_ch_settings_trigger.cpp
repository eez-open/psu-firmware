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

#include "psu.h"

#if OPTION_DISPLAY

#include "profile.h"
#include "channel_dispatcher.h"
#include "trigger.h"
#include "list.h"
#include "encoder.h"

#include "gui_page_ch_settings_trigger.h"
#include "gui_numeric_keypad.h"

#define INF_TEXT PSTR("\x91")

namespace eez {
namespace psu {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

ChSettingsTriggerPage::ChSettingsTriggerPage() {
}

data::Value ChSettingsTriggerPage::getData(const data::Cursor &cursor, uint8_t id) {
	data::Value value = Page::getData(cursor, id);
	if (value.getType() != data::VALUE_TYPE_NONE) {
		return value;
	}

	if (id == DATA_ID_CHANNEL_TRIGGER_MODE) {
		return data::Value(channel_dispatcher::getVoltageTriggerMode(*g_channel), data::ENUM_DEFINITION_CHANNEL_TRIGGER_MODE);
	}

	if (id == DATA_ID_CHANNEL_U_TRIGGER_VALUE) {
		return data::Value(channel_dispatcher::getTriggerVoltage(*g_channel), data::VALUE_TYPE_FLOAT_VOLT);
	}

	if (id == DATA_ID_CHANNEL_I_TRIGGER_VALUE) {
		return data::Value(channel_dispatcher::getTriggerCurrent(*g_channel), data::VALUE_TYPE_FLOAT_AMPER);
	}

	if (id == DATA_ID_CHANNEL_LIST_COUNT) {
        uint16_t count = list::getListCount(*g_channel);
        if (count > 0) {
    		return data::Value(count);
        } else {
            return data::Value(INF_TEXT);
        }
	}

    return data::Value();
}

void ChSettingsTriggerPage::onTriggerModeSet(uint8_t value) {
	popPage();
    channel_dispatcher::setVoltageTriggerMode(*g_channel, (TriggerMode)value);
    channel_dispatcher::setCurrentTriggerMode(*g_channel, (TriggerMode)value);
    profile::save();
}

void ChSettingsTriggerPage::editTriggerMode() {
    pushSelectFromEnumPage(data::g_channelTriggerModeEnumDefinition, channel_dispatcher::getVoltageTriggerMode(*g_channel), -1, onTriggerModeSet);
}

void ChSettingsTriggerPage::onVoltageTriggerValueSet(float value) {
	popPage();
    channel_dispatcher::setTriggerVoltage(*g_channel, value);
    profile::save();
}

void ChSettingsTriggerPage::editVoltageTriggerValue() {
	NumericKeypadOptions options;

	options.editUnit = data::VALUE_TYPE_FLOAT_VOLT;

	options.min = channel_dispatcher::getUMin(*g_channel);
	options.max = channel_dispatcher::getUMax(*g_channel);
	options.def = channel_dispatcher::getUMax(*g_channel);

	options.flags.genericNumberKeypad = true;
	options.enableMaxButton();
	options.enableDefButton();
	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	NumericKeypad::start(0, data::Value(trigger::getVoltage(*g_channel), data::VALUE_TYPE_FLOAT_VOLT), options, onVoltageTriggerValueSet);
}

void ChSettingsTriggerPage::onCurrentTriggerValueSet(float value) {
	popPage();
    channel_dispatcher::setTriggerCurrent(*g_channel, value);
    profile::save();
}

void ChSettingsTriggerPage::editCurrentTriggerValue() {
	NumericKeypadOptions options;

	options.editUnit = data::VALUE_TYPE_FLOAT_AMPER;

	options.min = channel_dispatcher::getIMin(*g_channel);
	options.max = channel_dispatcher::getIMax(*g_channel);
	options.def = channel_dispatcher::getIMax(*g_channel);

	options.flags.genericNumberKeypad = true;
	options.enableMaxButton();
	options.enableDefButton();
	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	NumericKeypad::start(0, data::Value(trigger::getCurrent(*g_channel), data::VALUE_TYPE_FLOAT_AMPER), options, onCurrentTriggerValueSet);
}

void ChSettingsTriggerPage::onListCountSet(float value) {
	popPage();
    list::setListCount(*g_channel, (uint16_t)value);
    profile::save();
}

void ChSettingsTriggerPage::onListCountSetToInfinity() {
	popPage();
    list::setListCount(*g_channel, 0);
    profile::save();
}


void ChSettingsTriggerPage::editListCount() {
	NumericKeypadOptions options;

	options.editUnit = data::VALUE_TYPE_INT;

	options.min = 0;
	options.max = MAX_LIST_COUNT;
	options.def = 0;

	options.flags.genericNumberKeypad = true;
    options.flags.option1ButtonEnabled = true;
    options.option1ButtonText = INF_TEXT;
    options.option1 = onListCountSetToInfinity;

	NumericKeypad::start(0, data::Value((uint16_t)list::getListCount(*g_channel)), options, onListCountSet);
}

////////////////////////////////////////////////////////////////////////////////

ChSettingsListsPage::ChSettingsListsPage()
    : m_iCursor(0)
    , m_listVersion(0)
{
    float *dwellList = list::getDwellList(*g_channel, &m_dwellListLength);
    memcpy(m_dwellList, dwellList, m_dwellListLength * sizeof(float));

    float *voltageList = list::getVoltageList(*g_channel, &m_voltageListLength);
    memcpy(m_voltageList, voltageList, m_voltageListLength * sizeof(float));

    float *currentList = list::getCurrentList(*g_channel, &m_currentListLength);
    memcpy(m_currentList, currentList, m_currentListLength * sizeof(float));

}

int ChSettingsListsPage::getListLength(uint8_t id) {
	if (id == DATA_ID_CHANNEL_LISTS) {
		return getMaxListLength();
	}
    
    if (id == DATA_ID_CHANNEL_LIST_DWELL) {
        return m_dwellListLength;
    }

    if (id == DATA_ID_CHANNEL_LIST_VOLTAGE) {
        return m_voltageListLength;
    }

    if (id == DATA_ID_CHANNEL_LIST_CURRENT) {
        return m_currentListLength;
    }

    return 0;
}

float *ChSettingsListsPage::getFloatList(uint8_t id) {
    if (id == DATA_ID_CHANNEL_LIST_DWELL) {
        return m_dwellList;
    }

    if (id == DATA_ID_CHANNEL_LIST_VOLTAGE) {
        return m_voltageList;
    }

    if (id == DATA_ID_CHANNEL_LIST_CURRENT) {
        return m_currentList;
    }

    return 0;
}

data::Value ChSettingsListsPage::getMin(const data::Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_CHANNEL_LIST_VOLTAGE) {
        return data::Value(channel_dispatcher::getUMin(*g_channel), data::VALUE_TYPE_FLOAT_VOLT);
    }

    if (id == DATA_ID_CHANNEL_LIST_CURRENT) {
        return data::Value(channel_dispatcher::getIMin(*g_channel), data::VALUE_TYPE_FLOAT_AMPER);
    }

    if (id == DATA_ID_CHANNEL_LIST_DWELL) {
        return data::Value(LIST_DWELL_MIN, data::VALUE_TYPE_FLOAT_SECOND);
    }

    return data::Value();
}

data::Value ChSettingsListsPage::getMax(const data::Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_CHANNEL_LIST_VOLTAGE) {
        return data::Value(channel_dispatcher::getUMax(*g_channel), data::VALUE_TYPE_FLOAT_VOLT);
    }

    if (id == DATA_ID_CHANNEL_LIST_CURRENT) {
        return data::Value(channel_dispatcher::getIMax(*g_channel), data::VALUE_TYPE_FLOAT_AMPER);
    }

    if (id == DATA_ID_CHANNEL_LIST_DWELL) {
        return data::Value(LIST_DWELL_MAX, data::VALUE_TYPE_FLOAT_SECOND);
    }

    return data::Value();
}

data::Value ChSettingsListsPage::getDef(const data::Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_CHANNEL_LIST_VOLTAGE) {
        return data::Value(g_channel->u.def, data::VALUE_TYPE_FLOAT_VOLT);
    }

    if (id == DATA_ID_CHANNEL_LIST_CURRENT) {
        return data::Value(g_channel->i.def, data::VALUE_TYPE_FLOAT_AMPER);
    }

    if (id == DATA_ID_CHANNEL_LIST_DWELL) {
        return data::Value(LIST_DWELL_DEF, data::VALUE_TYPE_FLOAT_SECOND);
    }

    return data::Value();
}

data::Value ChSettingsListsPage::getData(const data::Cursor &cursor, uint8_t id) {
	data::Value value = SetPage::getData(cursor, id);
	if (value.getType() != data::VALUE_TYPE_NONE) {
		return value;
	}

    int iPage = getPageIndex();
    int iRow = iPage * LIST_ITEMS_PER_PAGE + cursor.i;

	if (id == DATA_ID_CHANNEL_LISTS) {
		return data::Value(m_listVersion);
	}

    if (id == DATA_ID_CHANNEL_LIST_INDEX) {
		return data::Value(iRow + 1);
	}

    if (id == DATA_ID_CHANNEL_LIST_DWELL_ENABLED) {
        return data::Value(iRow <= m_dwellListLength ? 1 : 0);
    }

    if (id == DATA_ID_CHANNEL_LIST_DWELL) {
        if (iRow < m_dwellListLength) {
		    return data::Value(m_dwellList[iRow], data::VALUE_TYPE_FLOAT_SECOND);
        } else {
            return data::Value(PSTR("-"));
        }
	}

    if (id == DATA_ID_CHANNEL_LIST_VOLTAGE_ENABLED) {
        return data::Value(iRow <= m_voltageListLength ? 1 : 0);
    }

    if (id == DATA_ID_CHANNEL_LIST_VOLTAGE) {
        if (iRow < m_voltageListLength) {
		    return data::Value(m_voltageList[iRow], data::VALUE_TYPE_FLOAT_VOLT);
        } else {
            return data::Value(PSTR("-"));
        }
	}

    if (id == DATA_ID_CHANNEL_LIST_CURRENT_ENABLED) {
        return data::Value(iRow <= m_currentListLength ? 1 : 0);
    }

    if (id == DATA_ID_CHANNEL_LIST_CURRENT) {
        if (iRow < m_currentListLength) {
		    return data::Value(m_currentList[iRow], data::VALUE_TYPE_FLOAT_AMPER);
        } else {
            return data::Value(PSTR("-"));
        }
	}

	if (id == DATA_ID_CHANNEL_LISTS_PREVIOUS_PAGE_ENABLED) {
		return data::Value(iPage > 0 ? 1 : 0);
	}

	if (id == DATA_ID_CHANNEL_LISTS_NEXT_PAGE_ENABLED) {
        return data::Value((iPage < getNumPages() - 1) ? 1 : 0);
	}

    if (id == DATA_ID_CHANNEL_LISTS_CURSOR) {
        return data::Value(m_iCursor);
    }

    if (id == DATA_ID_CHANNEL_LISTS_INSERT_MENU_ENABLED) {
        return data::Value(getRowIndex() < getMaxListLength() ? 1 : 0);
    }

    if (id == DATA_ID_CHANNEL_LISTS_DELETE_MENU_ENABLED) {
        return data::Value(getMaxListLength() ? 1 : 0);
    }

    if (id == DATA_ID_CHANNEL_LISTS_DELETE_ROW_ENABLED) {
        return data::Value(getRowIndex() < getMaxListLength() ? 1 : 0);
    }

    if (id == DATA_ID_CHANNEL_LISTS_CLEAR_COLUMN_ENABLED) {
        return data::Value(getRowIndex() < getMaxListLength() ? 1 : 0);
    }

    if (id == DATA_ID_CHANNEL_LISTS_DELETE_ROWS_ENABLED) {
        return data::Value(getRowIndex() < getMaxListLength() ? 1 : 0);
    }

    return data::Value();
}

bool ChSettingsListsPage::setData(const data::Cursor &cursor, uint8_t id, data::Value value) {
    if (id == DATA_ID_CHANNEL_LISTS_CURSOR) {
        m_iCursor = value.getInt();
        moveCursorToFirstAvailableCell();
        return true;
    }

    return false;
}

void ChSettingsListsPage::previousPage() {
    int iPage = getPageIndex();
    if (iPage > 0) {
        --iPage;
        m_iCursor = iPage * LIST_ITEMS_PER_PAGE * 3;
        moveCursorToFirstAvailableCell();
    }
}

void ChSettingsListsPage::nextPage() {
    int iPage = getPageIndex();
    if (iPage < getNumPages() - 1) {
        ++iPage;
        m_iCursor = iPage * LIST_ITEMS_PER_PAGE * 3;
        moveCursorToFirstAvailableCell();
    }
}

bool ChSettingsListsPage::isFocusedValueEmpty() {
    data::Cursor cursor(getCursorIndexWithinPage());
    data::Value value = data::get(cursor, getDataIdAtCursor());
    return value.getType() == data::VALUE_TYPE_STR;
}

float ChSettingsListsPage::getFocusedValue() {
    data::Cursor cursor(getCursorIndexWithinPage());

    data::Value value = data::get(cursor, getDataIdAtCursor());
    
    if (value.getType() == data::VALUE_TYPE_STR) {
        value = data::getDef(cursor, getDataIdAtCursor());
    }
        
    return value.getFloat();
}

void ChSettingsListsPage::setFocusedValue(float value) {
    data::Cursor cursor(getCursorIndexWithinPage());

    uint8_t dataId = getDataIdAtCursor();

	data::Value min = data::getMin(cursor, dataId);
	data::Value max = data::getMax(cursor, dataId);

    if (util::greaterOrEqual(value, min.getFloat(), data::getPrecision(min.getType())) &&
        util::lessOrEqual(value, max.getFloat(), data::getPrecision(max.getType())))
    {
        int iRow = getRowIndex();
    
        if (dataId == DATA_ID_CHANNEL_LIST_DWELL) {
            m_dwellList[iRow] = value;
            if (iRow >= m_dwellListLength) {
                m_dwellListLength = iRow + 1;
            }
        } else if (dataId == DATA_ID_CHANNEL_LIST_VOLTAGE) {
            m_voltageList[iRow] = value;
            if (iRow >= m_voltageListLength) {
                m_voltageListLength = iRow + 1;
            }
        } else {
            m_currentList[iRow] = value;
            if (iRow >= m_currentListLength) {
                m_currentListLength = iRow + 1;
            }
        }
        
        ++m_listVersion;
    }
}

void ChSettingsListsPage::onValueSet(float value) {
    ChSettingsListsPage *page = (ChSettingsListsPage *)getPreviousPage();
    page->doValueSet(value);
}

void ChSettingsListsPage::doValueSet(float value) {
    uint8_t dataId = getDataIdAtCursor();

    if (dataId != DATA_ID_CHANNEL_LIST_DWELL) {
        float power = value;

        int iRow = getRowIndex();

        if (dataId == DATA_ID_CHANNEL_LIST_VOLTAGE) {
            if (iRow < m_currentListLength) {
                power *= m_currentList[iRow];
            } else if (m_currentListLength > 0) {
                power *= m_currentList[0];
            } else {
                power = 0;
            }
        } else {
            if (iRow < m_voltageListLength) {
                power *= m_voltageList[iRow];
            } else if (m_voltageListLength > 0) {
                power *= m_voltageList[0];
            } else {
                power = 0;
            }
        }

        if (util::greater(power, g_channel->getPowerMaxLimit(), data::getPrecision(data::VALUE_TYPE_FLOAT_WATT))) {
            errorMessageP(PSTR("Power limit exceeded"));
            return;
        }
    }

    popPage();
    setFocusedValue(value);
}

void ChSettingsListsPage::edit() {
    DECL_WIDGET(widget, g_foundWidgetAtDown.widgetOffset);

    if (isFocusWidget(g_foundWidgetAtDown)) {
        NumericKeypadOptions options;

        data::Cursor cursor(getCursorIndexWithinPage());

        uint8_t dataId = getDataIdAtCursor();

        data::Value value = data::get(cursor, dataId);

	    data::Value def = data::getDef(cursor, dataId);

        if (value.getType() == data::VALUE_TYPE_STR) {
            value = data::Value();
            options.editUnit = def.getType();
        } else {
            options.editUnit = value.getType();
        }

        data::Value min = data::getMin(cursor, dataId);
        data::Value max = data::getMax(cursor, dataId);

        options.def = def.getFloat();
	    options.min = min.getFloat();
	    options.max = max.getFloat();

	    options.flags.genericNumberKeypad = true;
	    options.flags.signButtonEnabled = true;
	    options.flags.dotButtonEnabled = true;

        char label[64];
        strcpy_P(label, PSTR("["));
        if (dataId == DATA_ID_CHANNEL_LIST_DWELL) {
            char dwell[64];
            min.toText(dwell, sizeof(dwell));
            strcat(label, dwell);
        } else {
		    util::strcatFloat(label, options.min);
        }
		strcat_P(label, PSTR("-"));
        if (dataId == DATA_ID_CHANNEL_LIST_DWELL) {
            char dwell[64];
            max.toText(dwell, sizeof(dwell));
            strcat(label, dwell);
        } else if (dataId == DATA_ID_CHANNEL_LIST_VOLTAGE) {
    		util::strcatVoltage(label, options.max);
        } else {
    		util::strcatCurrent(label, options.max);
        }
		strcat_P(label, PSTR("]: "));

	    NumericKeypad::start(label, value, options, onValueSet);
    } else {
        m_iCursor = getCursorIndex(g_foundWidgetAtDown.cursor, widget->data);

        if (isFocusedValueEmpty()) {
            edit();
        }
    }
}

bool ChSettingsListsPage::isFocusWidget(const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);
    return m_iCursor == getCursorIndex(widgetCursor.cursor, widget->data);
}

int ChSettingsListsPage::getRowIndex() {
    return m_iCursor / 3;
}

int ChSettingsListsPage::getColumnIndex() {
    return m_iCursor % 3;
}

int ChSettingsListsPage::getPageIndex() {
    return getRowIndex() / LIST_ITEMS_PER_PAGE;
}

uint16_t ChSettingsListsPage::getMaxListLength() {
    uint16_t size = m_dwellListLength;

    if (m_voltageListLength > size) {
        size = m_voltageListLength;
    }
    
    if (m_currentListLength > size) {
        size = m_currentListLength;
    }

    return size;
}

uint16_t ChSettingsListsPage::getNumPages() {
    return getMaxListLength() / LIST_ITEMS_PER_PAGE + 1;
}

int ChSettingsListsPage::getCursorIndexWithinPage() {
    return getRowIndex() % LIST_ITEMS_PER_PAGE;
}

uint8_t ChSettingsListsPage::getDataIdAtCursor() {
    int iColumn = getColumnIndex();
    if (iColumn == 0) {
        return DATA_ID_CHANNEL_LIST_DWELL;
    } else if (iColumn == 1) {
        return DATA_ID_CHANNEL_LIST_VOLTAGE;
    } else {
        return DATA_ID_CHANNEL_LIST_CURRENT;
    }
}

int ChSettingsListsPage::getCursorIndex(const data::Cursor &cursor, uint8_t id) {
    int iCursor = (getPageIndex() * LIST_ITEMS_PER_PAGE + cursor.i) * 3;
    if (id == DATA_ID_CHANNEL_LIST_DWELL) {
        return iCursor;
    } else if (id == DATA_ID_CHANNEL_LIST_VOLTAGE) {
        return iCursor + 1;
    } else if (id == DATA_ID_CHANNEL_LIST_CURRENT) {
        return iCursor + 2;
    } else {
        return -1;
    }
}

void ChSettingsListsPage::moveCursorToFirstAvailableCell() {
    int iRow = getRowIndex();
    int iColumn = getColumnIndex();
    if (iColumn == 0) {
        if (iRow > m_dwellListLength) {
            if (iRow > m_voltageListLength) {
                if (iRow > m_currentListLength) {
                    m_iCursor = 0;
                } else {
                    m_iCursor += 2;
                }
            } else {
                m_iCursor += 1;
            }
        }
    } else if (iColumn == 1) {
        if (iRow > m_voltageListLength) {
            if (iRow > m_currentListLength) {
                m_iCursor += 2;
                moveCursorToFirstAvailableCell();
            } else {
                m_iCursor += 1;
            }
        }
    } else {
        if (iRow > m_currentListLength) {
            m_iCursor += 1;
            moveCursorToFirstAvailableCell();
        }
    }
}

int ChSettingsListsPage::getDirty() {
    return m_listVersion > 0;
}

void ChSettingsListsPage::set() {
    if (getDirty()) {
        if (list::areListLengthsEquivalent(m_dwellListLength, m_voltageListLength, m_currentListLength)) {
            trigger::abort();

            list::setDwellList(*g_channel, m_dwellList, m_dwellListLength);
            list::setVoltageList(*g_channel, m_voltageList, m_voltageListLength);
            list::setCurrentList(*g_channel, m_currentList, m_currentListLength);

            profile::saveImmediately();

            infoMessageP(PSTR("Lists changed!"), popPage);
        } else {
            errorMessageP(PSTR("List lengths are not equivalent!"));
        }
    }
}

void ChSettingsListsPage::discard() {
    if (getDirty()) {
        areYouSureWithMessage(PSTR("You have unsaved changes!"), popPage);
    } else {
        popPage();
    }
}

bool ChSettingsListsPage::onEncoder(int counter) {
    encoder::enableAcceleration(true);
    uint8_t dataId = getDataIdAtCursor();
    if (dataId == DATA_ID_CHANNEL_LIST_DWELL || dataId == DATA_ID_CHANNEL_LIST_VOLTAGE) {
        encoder::setMovingSpeedMultiplier(1.0f);
    } else {
        encoder::setMovingSpeedMultiplier(g_channel->i.max / g_channel->u.max);
    }

    float step = dataId == DATA_ID_CHANNEL_LIST_DWELL ? 0.001f : 0.01f;

    setFocusedValue(getFocusedValue() + step * counter);

    return true;
}

bool ChSettingsListsPage::onEncoderClicked() {
    uint8_t dataId = getDataIdAtCursor();
    int iRow = getRowIndex();

    if (dataId == DATA_ID_CHANNEL_LIST_DWELL) {
        if (iRow <= m_voltageListLength) {
            m_iCursor += 1;
            return true;
        }
            
        if (iRow <= m_currentListLength) {
            m_iCursor += 2;
            return true;
        }

        m_iCursor += 3;
    } else if (dataId == DATA_ID_CHANNEL_LIST_VOLTAGE) {
        if (iRow <= m_currentListLength) {
            m_iCursor += 1;
            return true;
        }

        m_iCursor += 2;
    } else {
        m_iCursor += 1;
    }

    moveCursorToFirstAvailableCell();

    return true;
}

void ChSettingsListsPage::showInsertMenu() {
    if (getRowIndex() < getMaxListLength()) {
        pushPage(PAGE_ID_CH_SETTINGS_LISTS_INSERT_MENU);
    }
}

void ChSettingsListsPage::showDeleteMenu() {
    if (getMaxListLength()) {
        pushPage(PAGE_ID_CH_SETTINGS_LISTS_DELETE_MENU);
    }
}

void ChSettingsListsPage::insertRow(int iRow, int iCopyRow) {
    if (getMaxListLength() < MAX_LIST_LENGTH) {
        int iRow = getRowIndex();
        for (int i = MAX_LIST_LENGTH - 2; i >= iRow; --i) {
            m_dwellList[i+1] = m_dwellList[i];
            m_voltageList[i+1] = m_voltageList[i];
            m_currentList[i+1] = m_currentList[i];
        }
    
        data::Cursor cursor(getCursorIndexWithinPage());

        if (iRow <= m_dwellListLength) {
            m_dwellList[iRow] = m_dwellList[iCopyRow];
            ++m_dwellListLength;
        }
        
        if (iRow <= m_voltageListLength) {
            m_voltageList[iRow] = m_voltageList[iCopyRow];
            ++m_voltageListLength;
        }
        
        if (iRow <= m_currentListLength) {
            m_currentList[iRow] = m_currentList[iCopyRow];
            ++m_currentListLength;
        }
    }
}

void ChSettingsListsPage::insertRowAbove() {
    int iRow = getRowIndex();
    insertRow(iRow, iRow);
}

void ChSettingsListsPage::insertRowBelow() {
    int iRow = getRowIndex();
    if (iRow < getMaxListLength()) {
        m_iCursor += 3;
        insertRow(getRowIndex(), iRow);
    }
}

void ChSettingsListsPage::deleteRow() {
    int iRow = getRowIndex();
    int maxListLength = getMaxListLength();
    if (iRow < getMaxListLength()) {
        for (int i = iRow+1; i < MAX_LIST_LENGTH; ++i) {
            m_dwellList[i-1] = m_dwellList[i];
            m_voltageList[i-1] = m_voltageList[i];
            m_currentList[i-1] = m_currentList[i];
        }
    
        if (iRow < m_dwellListLength) {
            --m_dwellListLength;
        }
        
        if (iRow < m_voltageListLength) {
            --m_voltageListLength;
        }

        if (iRow < m_currentListLength) {
            --m_currentListLength;
        }
    }
}

void ChSettingsListsPage::onClearColumn() {
    ((ChSettingsListsPage *)getActivePage())->doClearColumn();
}

void ChSettingsListsPage::doClearColumn() {
    int iRow = getRowIndex();
    int iColumn = getColumnIndex();
    if (iColumn == 0) {
        m_dwellListLength = iRow;
    } else if (iColumn == 1) {
        m_voltageListLength = iRow;
    } else {
        m_currentListLength = iRow;
    }
}

void ChSettingsListsPage::clearColumn() {
    yesNoDialog(PAGE_ID_YES_NO, PSTR("Are you sure?"), onClearColumn, 0, 0);
}

void ChSettingsListsPage::onDeleteRows() {
    ((ChSettingsListsPage *)getActivePage())->doDeleteRows();
}

void ChSettingsListsPage::doDeleteRows() {
    int iRow = getRowIndex();
    if (iRow < m_dwellListLength) m_dwellListLength = iRow;
    if (iRow < m_voltageListLength) m_voltageListLength = iRow;
    if (iRow < m_currentListLength) m_currentListLength = iRow;
}

void ChSettingsListsPage::deleteRows() {
    yesNoDialog(PAGE_ID_YES_NO, PSTR("Are you sure?"), onDeleteRows, 0, 0);
}

void ChSettingsListsPage::onDeleteAll() {
    ((ChSettingsListsPage *)getActivePage())->doDeleteAll();
}

void ChSettingsListsPage::doDeleteAll() {
    m_dwellListLength = 0;
    m_voltageListLength = 0;
    m_currentListLength = 0;
    m_iCursor = 0;
}

void ChSettingsListsPage::deleteAll() {
    yesNoDialog(PAGE_ID_YES_NO, PSTR("Are you sure?"), onDeleteAll, 0, 0);
}

}
}
} // namespace eez::psu::gui

#endif