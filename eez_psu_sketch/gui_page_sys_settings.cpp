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

#if OPTION_DISPLAY

#include "persist_conf.h"
#include "ethernet.h"
#include "temperature.h"
#include "profile.h"
#if OPTION_ENCODER
#include "encoder.h"
#endif

#include "gui_page_sys_settings.h"
#include "gui_numeric_keypad.h"

namespace eez {
namespace psu {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

SysSettingsDateTimePage::SysSettingsDateTimePage() {
	dateTime = origDateTime = datetime::DateTime::now();
	timeZone = origTimeZone = persist_conf::devConf.time_zone;
	dst = origDst = persist_conf::devConf.flags.dst;
}

data::Value SysSettingsDateTimePage::getData(const data::Cursor &cursor, uint8_t id) {
	data::Value value = SetPage::getData(cursor, id);
	if (value.getType() != data::VALUE_TYPE_NONE) {
		return value;
	}

	if (id == DATA_ID_SYS_INFO_DATE_TIME_YEAR) {
		return data::Value(dateTime.year, data::VALUE_TYPE_YEAR);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_MONTH) {
		return data::Value(dateTime.month, data::VALUE_TYPE_MONTH);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_DAY) {
		return data::Value(dateTime.day, data::VALUE_TYPE_DAY);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_HOUR) {
		return data::Value(dateTime.hour, data::VALUE_TYPE_HOUR);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_MINUTE) {
		return data::Value(dateTime.minute, data::VALUE_TYPE_MINUTE);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_SECOND) {
		return data::Value(dateTime.second, data::VALUE_TYPE_SECOND);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_TIME_ZONE) {
		return data::Value(timeZone, data::VALUE_TYPE_TIME_ZONE);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_DST) {
		return data::Value(dst ? 1 : 0);
	}

	return data::Value();
}

void SysSettingsDateTimePage::edit() {
	DECL_WIDGET(widget, g_foundWidgetAtDown.widgetOffset);
	int id = widget->data;

	NumericKeypadOptions options;

	options.editUnit = data::VALUE_TYPE_INT;

	options.flags.genericNumberKeypad = true;
	options.flags.maxButtonEnabled = false;
	options.flags.defButtonEnabled = false;
	options.flags.signButtonEnabled = false;
	options.flags.dotButtonEnabled = false;

	const char *label = 0;

    data::Value value;

    if (id == DATA_ID_SYS_INFO_DATE_TIME_YEAR) {
		label = "Year (2016-2036): ";
		options.min = 2017;
		options.max = 2036;
		options.def = 2017;
        value = data::Value((int)dateTime.year);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_MONTH) {
		label = "Month (1-12): ";
		options.min = 1;
		options.max = 12;
		options.def = 1;
        value = data::Value((int)dateTime.month);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_DAY) {
		label = "Day (1-31): ";
		options.min = 1;
		options.max = 31;
		options.def = 1;
        value = data::Value((int)dateTime.day);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_HOUR) {
		label = "Hour (0-23): ";
		options.min = 0;
		options.max = 23;
		options.def = 12;
        value = data::Value((int)dateTime.hour);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_MINUTE) {
		label = "Minute (0-59): ";
		options.min = 0;
		options.max = 59;
		options.def = 0;
        value = data::Value((int)dateTime.minute);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_SECOND) {
		label = "Second (0-59): ";
		options.min = 0;
		options.max = 59;
		options.def = 0;
        value = data::Value((int)dateTime.second);
	} else if (id == DATA_ID_SYS_INFO_DATE_TIME_TIME_ZONE) {
		label = "Time zone: ";
		options.min = -12.00;
		options.max = 14.00;
		options.def = 0;
		options.editUnit = data::VALUE_TYPE_TIME_ZONE;
		options.flags.dotButtonEnabled = true;
		options.flags.signButtonEnabled = true;
	}

	if (label) {
		editDataId = id;
		NumericKeypad::start(label, value, options, onSetValue);
	}
}

void SysSettingsDateTimePage::toggleDst() {
	dst = dst ? 0 : 1;
}

void SysSettingsDateTimePage::setValue(float value) {
	if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_YEAR) {
		dateTime.year = uint16_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_MONTH) {
		dateTime.month = uint8_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_DAY) {
		dateTime.day = uint8_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_HOUR) {
		dateTime.hour = uint8_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_MINUTE) {
		dateTime.minute = uint8_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_SECOND) {
		dateTime.second = uint8_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_SECOND) {
		dateTime.second = uint8_t(value);
	} else if (editDataId == DATA_ID_SYS_INFO_DATE_TIME_TIME_ZONE) {
		timeZone = int16_t(roundf(value * 100));
	}
}

int SysSettingsDateTimePage::getDirty() {
	return (dateTime != origDateTime || timeZone != origTimeZone || dst != origDst) ? 1 : 0;
}

void SysSettingsDateTimePage::set() {
    if (!datetime::isValidDate(uint8_t(dateTime.year - 2000), dateTime.month, dateTime.day)) {
        errorMessageP(PSTR("Invalid date!"));
        return;
    }

    if (!datetime::isValidTime(dateTime.hour, dateTime.minute, dateTime.second)) {
        errorMessageP(PSTR("Invalid time!"));
        popPage();
		return;
    }
	
	if (dateTime != origDateTime) {
		if (!datetime::setDateTime(uint8_t(dateTime.year - 2000), dateTime.month, dateTime.day, dateTime.hour, dateTime.minute, dateTime.second)) {
			errorMessageP(PSTR("Failed to set system date and time!"));
			return;
		}
	}

	if (timeZone != origTimeZone || dst != origDst) {
		persist_conf::devConf.time_zone = timeZone;
		persist_conf::devConf.flags.dst = dst;
		if (!persist_conf::saveDevice()) {
	        errorMessageP(PSTR("Failed to set time zone and DST!"));
			return;
		}
		
		if (dateTime == origDateTime) {
			event_queue::pushEvent(event_queue::EVENT_INFO_SYSTEM_DATE_TIME_CHANGED);
		}
	}

	infoMessageP(PSTR("Date and time settings saved!"), popPage);
	return;
}

////////////////////////////////////////////////////////////////////////////////

data::Value SysSettingsEthernetPage::getData(const data::Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_SYS_ETHERNET_ENABLED) {
        return data::Value(persist_conf::isEthernetEnabled() ? 1 : 0);
    }

#if OPTION_ETHERNET
    if (id == DATA_ID_SYS_ETHERNET_STATUS) {
        return data::Value(ethernet::g_testResult);
    }

    if (id == DATA_ID_SYS_ETHERNET_IP_ADDRESS) {
        return data::Value(ethernet::getIpAddress(), data::VALUE_TYPE_IP_ADDRESS);
    }

    if (id == DATA_ID_SYS_ETHERNET_SCPI_PORT) {
        return TCP_PORT;
    }
#endif

    return data::Value();
}

static void enableEthernet(bool enable) {
    persist_conf::enableEthernet(enable);
    longInfoMessageP(
        PSTR("Turn off and on power or"),
        PSTR("press reset to apply changes!"));
}

void SysSettingsEthernetPage::enable() {
    enableEthernet(true);
}

void SysSettingsEthernetPage::disable() {
    enableEthernet(false);
}

////////////////////////////////////////////////////////////////////////////////

data::Value SysSettingsProtectionsPage::getData(const data::Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_SYS_OUTPUT_PROTECTION_COUPLED) {
        return data::Value(persist_conf::isOutputProtectionCoupleEnabled() ? 1 : 0);
    }

    if (id == DATA_ID_SYS_SHUTDOWN_WHEN_PROTECTION_TRIPPED) {
        return data::Value(persist_conf::isShutdownWhenProtectionTrippedEnabled() ? 1 : 0);
    }

    if (id == DATA_ID_SYS_FORCE_DISABLING_ALL_OUTPUTS_ON_POWER_UP) {
        return data::Value(persist_conf::isForceDisablingAllOutputsOnPowerUpEnabled() ? 1 : 0);
    }

    return data::Value();
}

void SysSettingsProtectionsPage::toggleOutputProtectionCouple() {
    if (persist_conf::isOutputProtectionCoupleEnabled()) {
        if (persist_conf::enableOutputProtectionCouple(false)) {
            infoMessageP(PSTR("Output protection decoupled!"));
        }
    } else {
        if (persist_conf::enableOutputProtectionCouple(true)) {
            infoMessageP(PSTR("Output protection coupled!"));
        }
    }
}

void SysSettingsProtectionsPage::toggleShutdownWhenProtectionTripped() {
    if (persist_conf::isShutdownWhenProtectionTrippedEnabled()) {
        if (persist_conf::enableShutdownWhenProtectionTripped(false)) {
            infoMessageP(PSTR("Shutdown when tripped disabled!"));
        }
    } else {
        if (persist_conf::enableShutdownWhenProtectionTripped(true)) {
            infoMessageP(PSTR("Shutdown when tripped enabled!"));
        }
    }
}

void SysSettingsProtectionsPage::toggleForceDisablingAllOutputsOnPowerUp() {
    if (persist_conf::isForceDisablingAllOutputsOnPowerUpEnabled()) {
        if (persist_conf::enableForceDisablingAllOutputsOnPowerUp(false)) {
            infoMessageP(PSTR("Force disabling outputs disabled!"));
        }
    } else {
        if (persist_conf::enableForceDisablingAllOutputsOnPowerUp(true)) {
            infoMessageP(PSTR("Force disabling outputs enabled!"));
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

SysSettingsAuxOtpPage::SysSettingsAuxOtpPage() {
	origState = state = temperature::sensors[temp_sensor::AUX].prot_conf.state ? 1 : 0;

	origLevel = level = data::Value(temperature::sensors[temp_sensor::AUX].prot_conf.level, data::VALUE_TYPE_FLOAT_CELSIUS);
	minLevel = OTP_AUX_MIN_LEVEL;
	maxLevel = OTP_AUX_MAX_LEVEL;
	defLevel = OTP_AUX_DEFAULT_LEVEL;

	origDelay = delay = data::Value(temperature::sensors[temp_sensor::AUX].prot_conf.delay, data::VALUE_TYPE_FLOAT_SECOND);
	minDelay = OTP_AUX_MIN_DELAY;
	maxDelay = OTP_AUX_MAX_DELAY;
	defaultDelay = OTP_CH_DEFAULT_DELAY;
}

data::Value SysSettingsAuxOtpPage::getData(const data::Cursor &cursor, uint8_t id) {
	data::Value value = SetPage::getData(cursor, id);
	if (value.getType() != data::VALUE_TYPE_NONE) {
		return value;
	}

	if (id == DATA_ID_SYS_TEMP_AUX_OTP_STATE) {
		return state;
	}

    if (id == DATA_ID_SYS_TEMP_AUX_OTP_LEVEL) {
		return level;
	}

	if (id == DATA_ID_SYS_TEMP_AUX_OTP_DELAY) {
		return delay;
	}

	if (id == DATA_ID_SYS_TEMP_AUX_OTP_IS_TRIPPED) {
		return temperature::sensors[temp_sensor::AUX].isTripped() ? 1 : 0;
	}

	return data::Value();
}

int SysSettingsAuxOtpPage::getDirty() {
	return (origState != state || origLevel != level || origDelay != delay) ? 1 : 0;
}

void SysSettingsAuxOtpPage::set() {
	if (getDirty()) {
		setParams();
	}
}

void SysSettingsAuxOtpPage::toggleState() {
	state = state ? 0 : 1;
}


void SysSettingsAuxOtpPage::onLevelSet(float value) {
	popPage();
	SysSettingsAuxOtpPage *page = (SysSettingsAuxOtpPage *)getActivePage();
	page->level = data::Value(value, page->level.getType());
}

void SysSettingsAuxOtpPage::editLevel() {
	NumericKeypadOptions options;

	options.editUnit = level.getType();

	options.min = minLevel;
	options.max = maxLevel;
	options.def = defLevel;

	options.flags.genericNumberKeypad = true;
	options.flags.maxButtonEnabled = true;
	options.flags.defButtonEnabled = true;
	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	NumericKeypad::start(0, level, options, onLevelSet);
}

void SysSettingsAuxOtpPage::onDelaySet(float value) {
	popPage();
	SysSettingsAuxOtpPage *page = (SysSettingsAuxOtpPage *)getActivePage();
	page->delay = data::Value(value, page->delay.getType());
}

void SysSettingsAuxOtpPage::editDelay() {
	NumericKeypadOptions options;

	options.editUnit = delay.getType();

	options.min = minDelay;
	options.max = maxDelay;
	options.def = defaultDelay;

	options.flags.genericNumberKeypad = true;
	options.flags.maxButtonEnabled = true;
	options.flags.defButtonEnabled = true;
	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	NumericKeypad::start(0, delay, options, onDelaySet);
}

void SysSettingsAuxOtpPage::setParams() {
    temperature::sensors[temp_sensor::AUX].prot_conf.state = state ? true : false;
    temperature::sensors[temp_sensor::AUX].prot_conf.level = level.getFloat();
    temperature::sensors[temp_sensor::AUX].prot_conf.delay = delay.getFloat();

    profile::save();
    infoMessageP(PSTR("Aux temp. protection changed!"), popPage);
}

void SysSettingsAuxOtpPage::clear() {
    temperature::sensors[temp_sensor::AUX].clearProtection();
    infoMessageP(PSTR("Cleared!"), popPage);
}

////////////////////////////////////////////////////////////////////////////////

data::Value SysSettingsSoundPage::getData(const data::Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_SYS_SOUND_IS_ENABLED) {
        return data::Value(persist_conf::isSoundEnabled() ? 1 : 0);
    }

    if (id == DATA_ID_SYS_SOUND_IS_CLICK_ENABLED) {
        return data::Value(persist_conf::isClickSoundEnabled() ? 1 : 0);
    }

    return data::Value();
}

void SysSettingsSoundPage::toggleSound() {
    if (persist_conf::isSoundEnabled()) {
        if (persist_conf::enableSound(false)) {
            infoMessageP(PSTR("Sound disabled!"));
        }
    } else {
        if (persist_conf::enableSound(true)) {
            infoMessageP(PSTR("Sound enabled!"));
        }
    }
}

void SysSettingsSoundPage::toggleClickSound() {
    if (persist_conf::isClickSoundEnabled()) {
        if (persist_conf::enableClickSound(false)) {
            infoMessageP(PSTR("Click sound disabled!"));
        }
    } else {
        if (persist_conf::enableClickSound(true)) {
            infoMessageP(PSTR("Click sound enabled!"));
        }
    }
}

#if OPTION_ENCODER

////////////////////////////////////////////////////////////////////////////////

SysSettingsEncoderPage::SysSettingsEncoderPage() {
    origConfirmationMode = confirmationMode = persist_conf::devConf2.flags.encoderConfirmationMode;
    origMovingSpeedDown = movingSpeedDown = persist_conf::devConf2.encoderMovingSpeedDown;
    origMovingSpeedUp = movingSpeedUp = persist_conf::devConf2.encoderMovingSpeedUp;
}

data::Value SysSettingsEncoderPage::getData(const data::Cursor &cursor, uint8_t id) {
	data::Value value = SetPage::getData(cursor, id);
	if (value.getType() != data::VALUE_TYPE_NONE) {
		return value;
	}

    if (id == DATA_ID_SYS_ENCODER_CONFIRMATION_MODE) {
        return data::Value((int)confirmationMode);
    }

    if (id == DATA_ID_SYS_ENCODER_MOVING_DOWN_SPEED) {
        return data::Value((int)movingSpeedDown);
    }

    if (id == DATA_ID_SYS_ENCODER_MOVING_UP_SPEED) {
        return data::Value((int)movingSpeedUp);
    }

    return data::Value();
}

data::Value SysSettingsEncoderPage::getMin(const data::Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_SYS_ENCODER_MOVING_DOWN_SPEED || id == DATA_ID_SYS_ENCODER_MOVING_UP_SPEED) {
        return encoder::MIN_MOVING_SPEED;
    }

    return data::Value();
}

data::Value SysSettingsEncoderPage::getMax(const data::Cursor &cursor, uint8_t id) {
    if (id == DATA_ID_SYS_ENCODER_MOVING_DOWN_SPEED || id == DATA_ID_SYS_ENCODER_MOVING_UP_SPEED) {
        return encoder::MAX_MOVING_SPEED;
    }

    return data::Value();
}

bool SysSettingsEncoderPage::setData(const data::Cursor &cursor, uint8_t id, data::Value value) {
    if (id == DATA_ID_SYS_ENCODER_MOVING_DOWN_SPEED) {
        movingSpeedDown = (uint8_t)value.getInt();
        return true;
    }

    if (id == DATA_ID_SYS_ENCODER_MOVING_UP_SPEED) {
        movingSpeedUp = (uint8_t)value.getInt();
        return true;
    }

    return false;
}

void SysSettingsEncoderPage::toggleConfirmationMode() {
    confirmationMode = confirmationMode ? 0 : 1;
}

int SysSettingsEncoderPage::getDirty() {
    return origConfirmationMode != confirmationMode || origMovingSpeedDown != movingSpeedDown || origMovingSpeedUp != movingSpeedUp;
}

void SysSettingsEncoderPage::set() {
	if (getDirty()) {
        persist_conf::setEncoderSettings(confirmationMode, movingSpeedDown, movingSpeedUp);
		infoMessageP(PSTR("Encoder settings changed!"), popPage);
	}
}

#endif

////////////////////////////////////////////////////////////////////////////////

void SysSettingsDisplayPage::turnOff() {
    persist_conf::setDisplayState(0);
}

void SysSettingsDisplayPage::editBrightness() {
}

}
}
} // namespace eez::psu::gui

#endif
