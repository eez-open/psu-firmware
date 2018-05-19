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
#include "mw_encoder.h"
#endif
#include "util.h"
#if OPTION_ETHERNET
#include "ntp.h"
#endif

#include "gui_psu.h"
#include "gui_data.h"
#include "mw_gui_gui.h"
#include "gui_page_sys_settings.h"
#include "gui_numeric_keypad.h"

using namespace eez::app::gui;

namespace eez {
namespace psu {
namespace gui {

////////////////////////////////////////////////////////////////////////////////

SysSettingsDateTimePage::SysSettingsDateTimePage() {
#if OPTION_ETHERNET
    ntpEnabled = origNtpEnabled = persist_conf::isNtpEnabled();
    strcpy(ntpServer, persist_conf::devConf2.ntpServer);
    strcpy(origNtpServer, persist_conf::devConf2.ntpServer);
#else
    ntpEnabled = origNtpEnabled = false;
    strcpy(ntpServer, "");
    strcpy(origNtpServer, "");
#endif
    dateTimeModified = false;
	timeZone = origTimeZone = persist_conf::devConf.time_zone;
	dstRule = origDstRule = (datetime::DstRule)persist_conf::devConf2.dstRule;
}

void SysSettingsDateTimePage::toggleNtp() {
#if OPTION_ETHERNET
    ntpEnabled = !ntpEnabled;
#endif
}

void SysSettingsDateTimePage::editNtpServer() {
#if OPTION_ETHERNET
    Keypad::startPush(0, ntpServer, 32, false, onSetNtpServer, popPage);
#endif
}

#if OPTION_ETHERNET
void SysSettingsDateTimePage::onSetNtpServer(char *value) {
	SysSettingsDateTimePage *page = (SysSettingsDateTimePage*)getPreviousPage();
    strcpy(page->ntpServer, value);

    popPage();
}
#endif

void SysSettingsDateTimePage::edit() {
	DECL_WIDGET(widget, g_foundWidgetAtDown.widgetOffset);
	int id = widget->data;

	NumericKeypadOptions options;

	const char *label = 0;

    data::Value value;

    if (id == DATA_ID_DATE_TIME_YEAR) {
		label = "Year (2016-2036): ";
		options.min = 2017;
		options.max = 2036;
		options.def = 2017;
        value = data::Value((int)dateTime.year);
	} else if (id == DATA_ID_DATE_TIME_MONTH) {
		label = "Month (1-12): ";
		options.min = 1;
		options.max = 12;
		options.def = 1;
        value = data::Value((int)dateTime.month);
	} else if (id == DATA_ID_DATE_TIME_DAY) {
		label = "Day (1-31): ";
		options.min = 1;
		options.max = 31;
		options.def = 1;
        value = data::Value((int)dateTime.day);
	} else if (id == DATA_ID_DATE_TIME_HOUR) {
		label = "Hour (0-23): ";
		options.min = 0;
		options.max = 23;
		options.def = 12;
        value = data::Value((int)dateTime.hour);
	} else if (id == DATA_ID_DATE_TIME_MINUTE) {
		label = "Minute (0-59): ";
		options.min = 0;
		options.max = 59;
		options.def = 0;
        value = data::Value((int)dateTime.minute);
	} else if (id == DATA_ID_DATE_TIME_SECOND) {
		label = "Second (0-59): ";
		options.min = 0;
		options.max = 59;
		options.def = 0;
        value = data::Value((int)dateTime.second);
	} else if (id == DATA_ID_DATE_TIME_TIME_ZONE) {
		label = "Time zone: ";
		options.min = -12.00;
		options.max = 14.00;
		options.def = 0;
		options.flags.dotButtonEnabled = true;
		options.flags.signButtonEnabled = true;
		value = data::Value(timeZone, VALUE_TYPE_TIME_ZONE);
	}

	if (label) {
		editDataId = id;
		NumericKeypad::start(label, value, options, onSetValue);
	}
}

void SysSettingsDateTimePage::onDstRuleSet(uint8_t value) {
    popPage();
	SysSettingsDateTimePage *page = (SysSettingsDateTimePage*)getActivePage();
    page->dstRule = (datetime::DstRule)value;
}

void SysSettingsDateTimePage::selectDstRule() {
    pushSelectFromEnumPage(g_dstRuleEnumDefinition, dstRule, 0, onDstRuleSet);
}

void SysSettingsDateTimePage::setValue(float value) {
	if (editDataId == DATA_ID_DATE_TIME_YEAR) {
		dateTime.year = uint16_t(value);
        dateTimeModified = true;
	} else if (editDataId == DATA_ID_DATE_TIME_MONTH) {
		dateTime.month = uint8_t(value);
        dateTimeModified = true;
	} else if (editDataId == DATA_ID_DATE_TIME_DAY) {
		dateTime.day = uint8_t(value);
        dateTimeModified = true;
	} else if (editDataId == DATA_ID_DATE_TIME_HOUR) {
		dateTime.hour = uint8_t(value);
        dateTimeModified = true;
	} else if (editDataId == DATA_ID_DATE_TIME_MINUTE) {
		dateTime.minute = uint8_t(value);
        dateTimeModified = true;
	} else if (editDataId == DATA_ID_DATE_TIME_SECOND) {
		dateTime.second = uint8_t(value);
        dateTimeModified = true;
	} else if (editDataId == DATA_ID_DATE_TIME_TIME_ZONE) {
		timeZone = int16_t(roundf(value * 100));
	}
}

int SysSettingsDateTimePage::getDirty() {
    if (ntpEnabled && !ntpServer[0]) {
        return 0;
    }

	if (ntpEnabled != origNtpEnabled) {
        return 1;
    }

    if (ntpEnabled) {
        if (ntpServer[0] && strcmp(ntpServer, origNtpServer)) {
            return 1;
        }
    } else {
        if (dateTimeModified) {
            return 1;
        }
    }

    return (timeZone != origTimeZone || dstRule != origDstRule) ? 1 : 0;
}

#if OPTION_ETHERNET

void SysSettingsDateTimePage::checkTestNtpServerStatus() {
    bool testResult;
    if (ntp::isTestNtpServerDone(testResult)) {
        popPage();

        if (testResult) {
            SysSettingsDateTimePage *page = (SysSettingsDateTimePage*)getActivePage();
            page->doSet();
        } else {
            errorMessageP("Unable to connect to NTP server!");
        }
    }
}

void SysSettingsDateTimePage::testNtpServer() {
    ntp::testNtpServer(ntpServer);
    showAsyncOperationInProgress("Testing NTP server...", checkTestNtpServerStatus);
}
#endif


void SysSettingsDateTimePage::set() {
    if (getDirty()) {
#if OPTION_ETHERNET
        if (ntpEnabled && strcmp(ntpServer, origNtpServer)) {
            testNtpServer();
            return;
        }
#endif
        doSet();
    }
}

void SysSettingsDateTimePage::doSet() {
    if (!ntpEnabled) {
        if (!datetime::isValidDate(uint8_t(dateTime.year - 2000), dateTime.month, dateTime.day)) {
            errorMessageP("Invalid date!");
            return;
        }

        if (!datetime::isValidTime(dateTime.hour, dateTime.minute, dateTime.second)) {
            errorMessageP("Invalid time!");
            popPage();
		    return;
        }
    }

#if OPTION_ETHERNET
    if (ntpEnabled != origNtpEnabled || strcmp(ntpServer, origNtpServer)) {
        persist_conf::setNtpSettings(ntpEnabled, ntpServer);
    }
#endif

    if (dstRule != origDstRule) {
		persist_conf::devConf2.dstRule = dstRule;
		persist_conf::saveDevice2();
    }

    if (!ntpEnabled && dateTimeModified) {
		datetime::setDateTime(uint8_t(dateTime.year - 2000), dateTime.month, dateTime.day, dateTime.hour, dateTime.minute, dateTime.second, true, 2);
    }

	if (timeZone != origTimeZone) {
		persist_conf::devConf.time_zone = timeZone;
		persist_conf::saveDevice();
	}

#if OPTION_ETHERNET
    ntp::reset();
#endif

    if (ntpEnabled || !dateTimeModified) {
        event_queue::pushEvent(event_queue::EVENT_INFO_SYSTEM_DATE_TIME_CHANGED);
    }

    infoMessageP("Date and time settings saved!", popPage);
	return;
}

////////////////////////////////////////////////////////////////////////////////

#if OPTION_ETHERNET

SysSettingsEthernetPage::SysSettingsEthernetPage() {
    m_enabledOrig = m_enabled = persist_conf::isEthernetEnabled();
    m_dhcpEnabledOrig = m_dhcpEnabled = persist_conf::isEthernetDhcpEnabled();
    m_ipAddressOrig = m_ipAddress = persist_conf::devConf2.ethernetIpAddress;
    m_dnsOrig = m_dns = persist_conf::devConf2.ethernetDns;
    m_gatewayOrig = m_gateway = persist_conf::devConf2.ethernetGateway;
    m_subnetMaskOrig = m_subnetMask = persist_conf::devConf2.ethernetSubnetMask;
    m_scpiPortOrig = m_scpiPort = persist_conf::devConf2.ethernetScpiPort;
    memcpy(m_macAddressOrig, persist_conf::devConf2.ethernetMacAddress, 6);
    memcpy(m_macAddress, persist_conf::devConf2.ethernetMacAddress, 6);
}

void SysSettingsEthernetPage::toggle() {
    m_enabled = !m_enabled;
}

void SysSettingsEthernetPage::toggleDhcp() {
    m_dhcpEnabled = !m_dhcpEnabled;
}

void SysSettingsEthernetPage::editStaticAddress() {
    pushPage(PAGE_ID_SYS_SETTINGS_ETHERNET_STATIC);
}

void SysSettingsEthernetPage::onSetScpiPort(float value) {
    popPage();
    SysSettingsEthernetPage *page = (SysSettingsEthernetPage *)getActivePage();
    page->m_scpiPort = (uint16_t)value;
}

void SysSettingsEthernetPage::editScpiPort() {
	NumericKeypadOptions options;

	options.min = 0;
	options.max = 65535;
	options.def = TCP_PORT;

    options.enableDefButton();

	NumericKeypad::start(0, data::Value((int)m_scpiPort, VALUE_TYPE_PORT), options, (void (*)(float))onSetScpiPort);
}

void SysSettingsEthernetPage::onSetMacAddress(char *value) {
    uint8_t macAddress[6];
    if (!parseMacAddress(value, strlen(value), macAddress)) {
        errorMessageP("Invalid MAC address!");
        return;
    }

	SysSettingsEthernetPage *page = (SysSettingsEthernetPage*)getPreviousPage();
    memcpy(page->m_macAddress, macAddress, 6);

    popPage();
}

void SysSettingsEthernetPage::editMacAddress() {
    char macAddressStr[18];
    macAddressToString(m_macAddress, macAddressStr);
    Keypad::startPush(0, macAddressStr, 32, false, onSetMacAddress, popPage);
}

int SysSettingsEthernetPage::getDirty() {
    return
        m_enabledOrig != m_enabled ||
        m_dhcpEnabledOrig != m_dhcpEnabled ||
        m_ipAddressOrig != m_ipAddress ||
        m_dnsOrig != m_dns ||
        m_gatewayOrig != m_gateway ||
        m_subnetMaskOrig != m_subnetMask ||
        m_scpiPortOrig != m_scpiPort ||
        memcmp(m_macAddress, m_macAddressOrig, 6) != 0;
}

void SysSettingsEthernetPage::set() {
    if (getDirty()) {
        if (persist_conf::setEthernetSettings(m_enabled, m_dhcpEnabled, m_ipAddress, m_dns, m_gateway, m_subnetMask, m_scpiPort, m_macAddress)) {
            longInfoMessageP("Turn off and on power or", "press reset to apply changes!", popPage);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

SysSettingsEthernetStaticPage::SysSettingsEthernetStaticPage() {
    SysSettingsEthernetPage *page = (SysSettingsEthernetPage *)getPreviousPage();

    m_ipAddressOrig = m_ipAddress = page->m_ipAddress;
    m_dnsOrig = m_dns = page->m_dns;
    m_gatewayOrig = m_gateway = page->m_gateway;
    m_subnetMaskOrig = m_subnetMask = page->m_subnetMask;
}

void SysSettingsEthernetStaticPage::onAddressSet(uint32_t address) {
    popPage();
    SysSettingsEthernetStaticPage *page = (SysSettingsEthernetStaticPage *)getActivePage();
    *page->m_editAddress = address;
}

void SysSettingsEthernetStaticPage::editAddress(uint32_t &address) {
    m_editAddress = &address;

	NumericKeypadOptions options;

	NumericKeypad::start(0, data::Value((uint32_t)address, VALUE_TYPE_IP_ADDRESS), options, (void (*)(float))onAddressSet);
}

void SysSettingsEthernetStaticPage::editIpAddress() {
    editAddress(m_ipAddress);
}

void SysSettingsEthernetStaticPage::editDns() {
    editAddress(m_dns);
}

void SysSettingsEthernetStaticPage::editGateway() {
    editAddress(m_gateway);
}

void SysSettingsEthernetStaticPage::editSubnetMask() {
    editAddress(m_subnetMask);
}

int SysSettingsEthernetStaticPage::getDirty() {
    return
        m_ipAddressOrig != m_ipAddress ||
        m_dnsOrig != m_dns ||
        m_gatewayOrig != m_gateway ||
        m_subnetMaskOrig != m_subnetMask;
}

void SysSettingsEthernetStaticPage::set() {
    if (getDirty()) {
        SysSettingsEthernetPage *page = (SysSettingsEthernetPage *)getPreviousPage();

        page->m_ipAddress = m_ipAddress;
        page->m_dns = m_dns;
        page->m_gateway = m_gateway;
        page->m_subnetMask = m_subnetMask;

        popPage();
    }
}

#endif


////////////////////////////////////////////////////////////////////////////////

void SysSettingsProtectionsPage::toggleOutputProtectionCouple() {
    if (persist_conf::isOutputProtectionCoupleEnabled()) {
        if (persist_conf::enableOutputProtectionCouple(false)) {
            infoMessageP("Output protection decoupled!");
        }
    } else {
        if (persist_conf::enableOutputProtectionCouple(true)) {
            infoMessageP("Output protection coupled!");
        }
    }
}

void SysSettingsProtectionsPage::toggleShutdownWhenProtectionTripped() {
    if (persist_conf::isShutdownWhenProtectionTrippedEnabled()) {
        if (persist_conf::enableShutdownWhenProtectionTripped(false)) {
            infoMessageP("Shutdown when tripped disabled!");
        }
    } else {
        if (persist_conf::enableShutdownWhenProtectionTripped(true)) {
            infoMessageP("Shutdown when tripped enabled!");
        }
    }
}

void SysSettingsProtectionsPage::toggleForceDisablingAllOutputsOnPowerUp() {
    if (persist_conf::isForceDisablingAllOutputsOnPowerUpEnabled()) {
        if (persist_conf::enableForceDisablingAllOutputsOnPowerUp(false)) {
            infoMessageP("Force disabling outputs disabled!");
        }
    } else {
        if (persist_conf::enableForceDisablingAllOutputsOnPowerUp(true)) {
            infoMessageP("Force disabling outputs enabled!");
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

SysSettingsAuxOtpPage::SysSettingsAuxOtpPage() {
	origState = state = temperature::sensors[temp_sensor::AUX].prot_conf.state ? 1 : 0;

	origLevel = level = MakeValue(temperature::sensors[temp_sensor::AUX].prot_conf.level, UNIT_CELSIUS);
	minLevel = OTP_AUX_MIN_LEVEL;
	maxLevel = OTP_AUX_MAX_LEVEL;
	defLevel = OTP_AUX_DEFAULT_LEVEL;

	origDelay = delay = MakeValue(temperature::sensors[temp_sensor::AUX].prot_conf.delay, UNIT_SECOND);
	minDelay = OTP_AUX_MIN_DELAY;
	maxDelay = OTP_AUX_MAX_DELAY;
	defaultDelay = OTP_CH_DEFAULT_DELAY;
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
	page->level = MakeValue(value, page->level.getUnit());
}

void SysSettingsAuxOtpPage::editLevel() {
	NumericKeypadOptions options;

	options.editValueUnit = level.getUnit();

	options.min = minLevel;
	options.max = maxLevel;
	options.def = defLevel;

	options.enableMaxButton();
	options.enableDefButton();
	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	NumericKeypad::start(0, level, options, onLevelSet);
}

void SysSettingsAuxOtpPage::onDelaySet(float value) {
	popPage();
	SysSettingsAuxOtpPage *page = (SysSettingsAuxOtpPage *)getActivePage();
	page->delay = MakeValue(value, page->delay.getUnit());
}

void SysSettingsAuxOtpPage::editDelay() {
	NumericKeypadOptions options;

	options.editValueUnit = delay.getUnit();

	options.min = minDelay;
	options.max = maxDelay;
	options.def = defaultDelay;

	options.enableMaxButton();
	options.enableDefButton();
	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	NumericKeypad::start(0, delay, options, onDelaySet);
}

void SysSettingsAuxOtpPage::setParams() {
    temperature::sensors[temp_sensor::AUX].prot_conf.state = state ? true : false;
    temperature::sensors[temp_sensor::AUX].prot_conf.level = level.getFloat();
    temperature::sensors[temp_sensor::AUX].prot_conf.delay = delay.getFloat();

    profile::save();
    infoMessageP("Aux temp. protection changed!", popPage);
}

void SysSettingsAuxOtpPage::clear() {
    temperature::sensors[temp_sensor::AUX].clearProtection();
    infoMessageP("Cleared!", popPage);
}

////////////////////////////////////////////////////////////////////////////////

void SysSettingsSoundPage::toggleSound() {
    if (persist_conf::isSoundEnabled()) {
        if (persist_conf::enableSound(false)) {
            infoMessageP("Sound disabled!");
        }
    } else {
        if (persist_conf::enableSound(true)) {
            infoMessageP("Sound enabled!");
        }
    }
}

void SysSettingsSoundPage::toggleClickSound() {
    if (persist_conf::isClickSoundEnabled()) {
        if (persist_conf::enableClickSound(false)) {
            infoMessageP("Click sound disabled!");
        }
    } else {
        if (persist_conf::enableClickSound(true)) {
            infoMessageP("Click sound enabled!");
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

void SysSettingsEncoderPage::toggleConfirmationMode() {
    confirmationMode = confirmationMode ? 0 : 1;
}

int SysSettingsEncoderPage::getDirty() {
    return origConfirmationMode != confirmationMode || origMovingSpeedDown != movingSpeedDown || origMovingSpeedUp != movingSpeedUp;
}

void SysSettingsEncoderPage::set() {
	if (getDirty()) {
        persist_conf::setEncoderSettings(confirmationMode, movingSpeedDown, movingSpeedUp);
		infoMessageP("Encoder settings changed!", popPage);
	}
}

#endif

////////////////////////////////////////////////////////////////////////////////

SysSettingsTriggerPage::SysSettingsTriggerPage() {
    m_sourceOrig = m_source = trigger::getSource();
    m_delayOrig = m_delay = trigger::getDelay();
    m_initiateContinuouslyOrig = m_initiateContinuously = trigger::isContinuousInitializationEnabled();
}

void SysSettingsTriggerPage::onTriggerSourceSet(uint8_t value) {
    popPage();
	SysSettingsTriggerPage *page = (SysSettingsTriggerPage*)getActivePage();
    page->m_source = (trigger::Source)value;
}

void SysSettingsTriggerPage::selectSource() {
    pushSelectFromEnumPage(g_triggerSourceEnumDefinition, m_source, 0, onTriggerSourceSet);
}

void SysSettingsTriggerPage::onDelaySet(float value) {
    popPage();
	SysSettingsTriggerPage *page = (SysSettingsTriggerPage*)getActivePage();
    page->m_delay = value;
}

void SysSettingsTriggerPage::editDelay() {
    NumericKeypadOptions options;

    options.editValueUnit = UNIT_SECOND;

    options.def = 0;
	options.min = 0;
	options.max = 3600;

	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	NumericKeypad::start(0, MakeValue(trigger::getDelay(), UNIT_SECOND), options, onDelaySet);
}

void SysSettingsTriggerPage::toggleInitiateContinuously() {
    m_initiateContinuously = !m_initiateContinuously;
}

int SysSettingsTriggerPage::getDirty() {
    return m_sourceOrig != m_source || m_delayOrig != m_delay || m_initiateContinuouslyOrig != m_initiateContinuously;
}

void SysSettingsTriggerPage::set() {
    if (getDirty()) {
        trigger::setSource(m_source);
        trigger::setDelay(m_delay);
        trigger::enableInitiateContinuous(m_initiateContinuously);

        if (m_source == trigger::SOURCE_PIN1) {
            persist_conf::devConf2.ioPins[0].function = io_pins::FUNCTION_TINPUT;
        }

        persist_conf::saveDevice2();

        infoMessageP("Trigger settings saved!", popPage);
    }
}


////////////////////////////////////////////////////////////////////////////////

SysSettingsIOPinsPage::SysSettingsIOPinsPage() {
    for (int i = 0; i < 3; ++i) {
        m_polarityOrig[i] = m_polarity[i] = (io_pins::Polarity) persist_conf::devConf2.ioPins[i].polarity;
        m_functionOrig[i] = m_function[i] = (io_pins::Function) persist_conf::devConf2.ioPins[i].function;
    }
}

void SysSettingsIOPinsPage::togglePolarity() {
    int i = g_foundWidgetAtDown.cursor.i;
    m_polarity[i] = m_polarity[i] == io_pins::POLARITY_NEGATIVE ? io_pins::POLARITY_POSITIVE : io_pins::POLARITY_NEGATIVE;
}

void SysSettingsIOPinsPage::onFunctionSet(uint8_t value) {
    popPage();
	SysSettingsIOPinsPage *page = (SysSettingsIOPinsPage*)getActivePage();
    page->m_function[page->pinNumber] = (io_pins::Function)value;
}

void SysSettingsIOPinsPage::selectFunction() {
    pinNumber = g_foundWidgetAtDown.cursor.i;
    pushSelectFromEnumPage(pinNumber == 0 ? g_ioPinsInputFunctionEnumDefinition : g_ioPinsOutputFunctionEnumDefinition,
        m_function[pinNumber], 0, onFunctionSet);
}

int SysSettingsIOPinsPage::getDirty() {
    for (int i = 0; i < 3; ++i) {
        if (m_polarityOrig[i] != m_polarity[i] || m_functionOrig[i] != m_function[i]) {
            return true;
        }
    }
    return false;
}

void SysSettingsIOPinsPage::set() {
    if (getDirty()) {
        for (int i = 0; i < 3; ++i) {
            persist_conf::devConf2.ioPins[i].polarity = m_polarity[i];
            persist_conf::devConf2.ioPins[i].function = m_function[i];
        }

        if (persist_conf::saveDevice2()) {
            infoMessageP("Digital I/O pin settings saved!", popPage);
        }

        io_pins::refresh();
    }
}


////////////////////////////////////////////////////////////////////////////////

SysSettingsSerialPage::SysSettingsSerialPage() {
    m_enabledOrig = m_enabled = persist_conf::isSerialEnabled();
    m_baudIndexOrig = m_baudIndex = persist_conf::getSerialBaudIndex();
    m_parityOrig = m_parity = (serial::Parity)persist_conf::getSerialParity();
}

void SysSettingsSerialPage::toggle() {
    m_enabled = !m_enabled;
}

void SysSettingsSerialPage::onParitySet(uint8_t value) {
    popPage();
	SysSettingsSerialPage *page = (SysSettingsSerialPage*)getActivePage();
    page->m_parity = (serial::Parity)value;
}

void SysSettingsSerialPage::selectParity() {
    pushSelectFromEnumPage(g_serialParityEnumDefinition, m_parity, 0, onParitySet);
}

int SysSettingsSerialPage::getDirty() {
    return m_enabledOrig != m_enabled || m_baudIndexOrig != m_baudIndex || m_parityOrig != m_parity;
}

void SysSettingsSerialPage::set() {
    if (getDirty()) {
        if (persist_conf::setSerialSettings(m_enabled, m_baudIndex, m_parity)) {
            infoMessageP("Serial settings saved!", popPage);
        }
    }
}

}
}
} // namespace eez::psu::gui

#endif
