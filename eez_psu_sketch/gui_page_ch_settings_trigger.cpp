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

ChSettingsListsPage::ChSettingsListsPage() {
}

data::Value ChSettingsListsPage::getData(const data::Cursor &cursor, uint8_t id) {
	data::Value value = Page::getData(cursor, id);
	if (value.getType() != data::VALUE_TYPE_NONE) {
		return value;
	}

	if (id == DATA_ID_CHANNEL_LIST_INDEX) {
		return data::Value(trigger::getVoltage(*g_channel), data::VALUE_TYPE_FLOAT_VOLT);
	}

    if (id == DATA_ID_CHANNEL_LIST_DWELL) {
		return data::Value(0.01f, data::VALUE_TYPE_FLOAT);
	}

	if (id == DATA_ID_CHANNEL_LIST_VOLTAGE) {
		return data::Value(0.1f * cursor.j, data::VALUE_TYPE_FLOAT_VOLT);
	}

	if (id == DATA_ID_CHANNEL_LIST_CURRENT) {
		return data::Value(0.01f * cursor.j, data::VALUE_TYPE_FLOAT_VOLT);
	}

	return data::Value();
}

}
}
} // namespace eez::psu::gui

#endif