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

	if (id == DATA_ID_CHANNEL_U_TRIGGER_MODE) {
		return data::Value(channel_dispatcher::getVoltageTriggerMode(*g_channel), data::ENUM_DEFINITION_CHANNEL_TRIGGER_MODE);
	}

	if (id == DATA_ID_CHANNEL_U_TRIGGER_VALUE) {
		return data::Value(trigger::getVoltage(*g_channel), data::VALUE_TYPE_FLOAT_VOLT);
	}

	if (id == DATA_ID_CHANNEL_I_TRIGGER_MODE) {
		return data::Value(channel_dispatcher::getCurrentTriggerMode(*g_channel), data::ENUM_DEFINITION_CHANNEL_TRIGGER_MODE);
	}

	if (id == DATA_ID_CHANNEL_I_TRIGGER_VALUE) {
		return data::Value(trigger::getCurrent(*g_channel), data::VALUE_TYPE_FLOAT_AMPER);
	}

	if (id == DATA_ID_CHANNEL_LIST_COUNT) {
		return data::Value(1);
	}

    return data::Value();
}

void ChSettingsTriggerPage::onVoltageTriggerModeSet(uint8_t value) {
	popPage();
    channel_dispatcher::setVoltageTriggerMode(*g_channel, (TriggerMode)value);
    profile::save();
}

void ChSettingsTriggerPage::editVoltageTriggerMode() {
    pushSelectFromEnumPage(data::g_channelTriggerModeEnumDefinition, channel_dispatcher::getVoltageTriggerMode(*g_channel), -1, onVoltageTriggerModeSet);
}

void ChSettingsTriggerPage::onVoltageTriggerValueSet(float value) {
	popPage();
    trigger::setVoltage(*g_channel, value);
    profile::save();
}

void ChSettingsTriggerPage::editVoltageTriggerValue() {
	NumericKeypadOptions options;

	options.editUnit = data::VALUE_TYPE_FLOAT_VOLT;

	options.min = channel_dispatcher::getUMin(*g_channel);
	options.max = channel_dispatcher::getUMax(*g_channel);
	options.def = channel_dispatcher::getUMax(*g_channel);

	options.flags.genericNumberKeypad = true;
	options.flags.maxButtonEnabled = true;
	options.flags.defButtonEnabled = true;
	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	NumericKeypad::start(0, data::Value(trigger::getVoltage(*g_channel), data::VALUE_TYPE_FLOAT_VOLT), options, onVoltageTriggerValueSet);
}

void ChSettingsTriggerPage::onCurrentTriggerModeSet(uint8_t value) {
	popPage();
    channel_dispatcher::setCurrentTriggerMode(*g_channel, (TriggerMode)value);
    profile::save();
}

void ChSettingsTriggerPage::editCurrentTriggerMode() {
    pushSelectFromEnumPage(data::g_channelTriggerModeEnumDefinition, channel_dispatcher::getCurrentTriggerMode(*g_channel), -1, onCurrentTriggerModeSet);
}

void ChSettingsTriggerPage::onCurrentTriggerValueSet(float value) {
	popPage();
    trigger::setCurrent(*g_channel, value);
    profile::save();
}

void ChSettingsTriggerPage::editCurrentTriggerValue() {
	NumericKeypadOptions options;

	options.editUnit = data::VALUE_TYPE_FLOAT_AMPER;

	options.min = channel_dispatcher::getIMin(*g_channel);
	options.max = channel_dispatcher::getIMax(*g_channel);
	options.def = channel_dispatcher::getIMax(*g_channel);

	options.flags.genericNumberKeypad = true;
	options.flags.maxButtonEnabled = true;
	options.flags.defButtonEnabled = true;
	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	NumericKeypad::start(0, data::Value(trigger::getCurrent(*g_channel), data::VALUE_TYPE_FLOAT_AMPER), options, onCurrentTriggerValueSet);
}

////////////////////////////////////////////////////////////////////////////////

ChSettingsListsPage::ChSettingsListsPage()
    : m_iPage(0)
    , m_iFocused(-1)
    , m_listVersion(0)
{
    float *voltageList = list::getVoltageList(*g_channel, &m_voltageListSize);
    memcpy(m_voltageList, voltageList, m_voltageListSize * sizeof(float));

    float *currentList = list::getCurrentList(*g_channel, &m_currentListSize);
    memcpy(m_currentList, currentList, m_currentListSize * sizeof(float));

    float *dwellList = list::getDwellList(*g_channel, &m_dwellListSize);
    memcpy(m_dwellList, dwellList, m_dwellListSize * sizeof(float));
}

int ChSettingsListsPage::getListSize(uint8_t id) {
	if (id == DATA_ID_CHANNEL_LISTS) {
		return getMaxListSize();
	}
    
    if (id == DATA_ID_CHANNEL_LIST_DWELL) {
        return m_dwellListSize;
    }

    if (id == DATA_ID_CHANNEL_LIST_VOLTAGE) {
        return m_voltageListSize;
    }

    if (id == DATA_ID_CHANNEL_LIST_CURRENT) {
        return m_currentListSize;
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
        return data::Value(g_channel->u.min, data::VALUE_TYPE_FLOAT_VOLT);
    }

    if (id == DATA_ID_CHANNEL_LIST_CURRENT) {
        return data::Value(g_channel->i.min, data::VALUE_TYPE_FLOAT_AMPER);
    }

    if (id == DATA_ID_CHANNEL_LIST_DWELL) {
        return data::Value(LIST_DWELL_MIN, data::VALUE_TYPE_FLOAT_SECOND);
    }

    return data::Value();
}

data::Value ChSettingsListsPage::getMax(const data::Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_CHANNEL_LIST_VOLTAGE) {
        return data::Value(g_channel->u.max, data::VALUE_TYPE_FLOAT_VOLT);
    }

    if (id == DATA_ID_CHANNEL_LIST_CURRENT) {
        return data::Value(g_channel->i.max, data::VALUE_TYPE_FLOAT_AMPER);
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

    int i = LIST_ITEMS_PER_PAGE * m_iPage + cursor.i;

	if (id == DATA_ID_CHANNEL_LISTS) {
		return data::Value(m_listVersion);
	}

    if (id == DATA_ID_CHANNEL_LIST_INDEX) {
		return data::Value(i + 1);
	}

    if (id == DATA_ID_CHANNEL_LIST_DWELL_ENABLED) {
        return data::Value(i <= m_dwellListSize ? 1 : 0);
    }

    if (id == DATA_ID_CHANNEL_LIST_DWELL) {
        if (i < m_dwellListSize) {
		    return data::Value(m_dwellList[i], data::VALUE_TYPE_FLOAT_SECOND);
        } else {
            return data::Value(PSTR("-"));
        }
	}

    if (id == DATA_ID_CHANNEL_LIST_VOLTAGE_ENABLED) {
        return data::Value(i <= m_voltageListSize ? 1 : 0);
    }

    if (id == DATA_ID_CHANNEL_LIST_VOLTAGE) {
        if (i < m_voltageListSize) {
		    return data::Value(m_voltageList[i], data::VALUE_TYPE_FLOAT_VOLT);
        } else {
            return data::Value(PSTR("-"));
        }
	}

    if (id == DATA_ID_CHANNEL_LIST_CURRENT_ENABLED) {
        return data::Value(i <= m_currentListSize ? 1 : 0);
    }

    if (id == DATA_ID_CHANNEL_LIST_CURRENT) {
        if (i < m_currentListSize) {
		    return data::Value(m_currentList[i], data::VALUE_TYPE_FLOAT_AMPER);
        } else {
            return data::Value(PSTR("-"));
        }
	}

	if (id == DATA_ID_CHANNEL_LISTS_PREVIOUS_PAGE_ENABLED) {
		return data::Value(m_iPage > 0 ? 1 : 0);
	}

	if (id == DATA_ID_CHANNEL_LISTS_NEXT_PAGE_ENABLED) {
        return data::Value((m_iPage < getNumPages() - 1) ? 1 : 0);
	}

	if (id == DATA_ID_CHANNEL_LISTS_DELETE_ROW_ENABLED) {
        return data::Value((m_iFocused != -1 && getFocusedIndex() < getMaxListSize()) ? 1 : 0);
	}

	if (id == DATA_ID_CHANNEL_LISTS_INSERT_ROW_ENABLED) {
        return data::Value((m_iFocused != -1 && getFocusedIndex() < getMaxListSize()) ? 1 : 0);
	}

    return data::Value();
}

void ChSettingsListsPage::previousPage() {
    if (m_iPage > 0) {
        --m_iPage;
        m_iFocused = -1;
    }
}

void ChSettingsListsPage::nextPage() {
    if (m_iPage < getNumPages() - 1) {
        ++m_iPage;
        m_iFocused = -1;
    }
}

void ChSettingsListsPage::deleteRow() {
}

void ChSettingsListsPage::insertRow() {
}

float ChSettingsListsPage::getFocusedValue() {
    data::Cursor cursor(m_iFocused);
	return data::get(cursor, m_focusDataId).getFloat();
}

void ChSettingsListsPage::setFocusedValue(float value) {
    data::Cursor cursor(m_iFocused);

	float min = data::getMin(cursor, m_focusDataId).getFloat();
	float max = data::getMax(cursor, m_focusDataId).getFloat();

    if (value >= min && value <= max) {
        int i = getFocusedIndex();
    
        if (m_focusDataId == DATA_ID_CHANNEL_LIST_DWELL) {
            m_dwellList[i] = value;
            if (i >= m_dwellListSize) {
                m_dwellListSize = i + 1;
            }
        } else if (m_focusDataId == DATA_ID_CHANNEL_LIST_VOLTAGE) {
            m_voltageList[i] = value;
            if (i >= m_voltageListSize) {
                m_voltageListSize = i + 1;
            }
        } else {
            m_currentList[i] = value;
            if (i >= m_currentListSize) {
                m_currentListSize = i + 1;
            }
        }
        
        ++m_listVersion;
    }
}

void ChSettingsListsPage::onValueSet(float value) {
    popPage();
    ChSettingsListsPage *page = (ChSettingsListsPage *)getActivePage();
    page->setFocusedValue(value);
}

void ChSettingsListsPage::edit() {
    DECL_WIDGET(widget, g_foundWidgetAtDown.widgetOffset);

    if (isFocusWidget(g_foundWidgetAtDown)) {
        NumericKeypadOptions options;

        data::Cursor cursor(m_iFocused);

        data::Value value = data::get(cursor, m_focusDataId);

	    options.editUnit = value.getType();

	    options.min = data::getMin(cursor, m_focusDataId).getFloat();
	    options.max = data::getMax(cursor, m_focusDataId).getFloat();
	    options.def = data::getDef(cursor, m_focusDataId).getFloat();

	    options.flags.genericNumberKeypad = true;
	    options.flags.maxButtonEnabled = true;
	    options.flags.defButtonEnabled = true;
	    options.flags.signButtonEnabled = true;
	    options.flags.dotButtonEnabled = true;

	    NumericKeypad::start(0, value, options, onValueSet);
    }
    m_iFocused = g_foundWidgetAtDown.cursor.i;
    m_focusDataId = widget->data;
}

bool ChSettingsListsPage::isFocusWidget(const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);
    return widgetCursor.cursor.i == m_iFocused && 
        widget->data == m_focusDataId;
}

uint16_t ChSettingsListsPage::getFocusedIndex() {
    return m_iPage * LIST_ITEMS_PER_PAGE + m_iFocused;
}

uint16_t ChSettingsListsPage::getMaxListSize() {
    uint16_t size = m_dwellListSize;

    if (m_voltageListSize > size) {
        size = m_voltageListSize;
    }
    
    if (m_currentListSize > size) {
        size = m_currentListSize;
    }

    return size;
}

uint16_t ChSettingsListsPage::getNumPages() {
    return getMaxListSize() / LIST_ITEMS_PER_PAGE + 1;
}

int ChSettingsListsPage::getDirty() {
    return m_listVersion > 0;
}

void ChSettingsListsPage::set() {
    if (!list::areListSizesEquivalent(m_dwellListSize, m_voltageListSize) || 
        !list::areListSizesEquivalent(m_dwellListSize, m_currentListSize) ||
        !list::areListSizesEquivalent(m_voltageListSize, m_currentListSize))
    {
        errorMessageP(PSTR("List lengths are not equivalent!"));
    } else {
        list::setDwellList(*g_channel, m_dwellList, m_dwellListSize);
        list::setVoltageList(*g_channel, m_voltageList, m_voltageListSize);
        list::setCurrentList(*g_channel, m_currentList, m_currentListSize);

        infoMessageP(PSTR("Lists changed!"), popPage);
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
    if (m_iFocused != -1) {
        encoder::enableAcceleration(true);
        if (m_focusDataId == DATA_ID_CHANNEL_LIST_DWELL || m_focusDataId == DATA_ID_CHANNEL_LIST_VOLTAGE) {
            encoder::setMovingSpeedMultiplier(1.0f);
        } else {
            encoder::setMovingSpeedMultiplier(g_channel->i.max / g_channel->u.max);
        }

        float step = m_focusDataId == DATA_ID_CHANNEL_LIST_DWELL ? 0.001f : 0.01f;

        setFocusedValue(getFocusedValue() + step * counter);
    }

    return true;
}

bool ChSettingsListsPage::onEncoderClicked() {
    if (m_iFocused == -1) {
        m_iFocused = 0;
        m_focusDataId = DATA_ID_CHANNEL_LIST_DWELL;
    } else {
        if (m_focusDataId == DATA_ID_CHANNEL_LIST_DWELL) {
            if (getFocusedIndex() <= m_voltageListSize) {
                m_focusDataId = DATA_ID_CHANNEL_LIST_VOLTAGE;
                return true;
            }
            
            if (getFocusedIndex() <= m_currentListSize) {
                m_focusDataId = DATA_ID_CHANNEL_LIST_CURRENT;
                return true;
            }
            
            ++m_iFocused;
        } else if (m_focusDataId == DATA_ID_CHANNEL_LIST_VOLTAGE) {
            if (getFocusedIndex() <= m_currentListSize) {
                m_focusDataId = DATA_ID_CHANNEL_LIST_CURRENT;
                return true;
            }
            
            ++m_iFocused;
        } else {
            ++m_iFocused;
        }

        if (m_iFocused == LIST_ITEMS_PER_PAGE) {
            m_iFocused = 0;
            ++m_iPage;
        }

        if (getFocusedIndex() > getMaxListSize()) {
            m_iFocused = 0;
            m_iPage = 0;
        }

        if (getFocusedIndex() <= m_dwellListSize) {
            m_focusDataId = DATA_ID_CHANNEL_LIST_DWELL;
        } else if (getFocusedIndex() <= m_voltageListSize) {
            m_focusDataId = DATA_ID_CHANNEL_LIST_VOLTAGE;
        } else {
            m_focusDataId = DATA_ID_CHANNEL_LIST_CURRENT;
        }
    }

    return true;
}


}
}
} // namespace eez::psu::gui

#endif