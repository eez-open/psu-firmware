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

#include "psu.h"

#if OPTION_DISPLAY

#include "gui_psu.h"
#include "gui_data.h"
#include "actions.h"
#include "app_gui_document.h"
#include "gui_password.h"
#include "gui_edit_mode.h"
#include "gui_edit_mode_slider.h"
#include "gui_edit_mode_step.h"
#include "gui_edit_mode_keypad.h"
#include "gui_numeric_keypad.h"
#include "gui_page_self_test_result.h"
#include "gui_page_event_queue.h"
#include "gui_page_ch_settings_protection.h"
#include "gui_page_ch_settings_trigger.h"
#include "gui_page_ch_settings_adv.h"
#include "gui_page_sys_settings.h"
#include "gui_page_user_profiles.h"
#include "lcd.h"
#include "mw_gui_lcd.h"
#include "idle.h"
#include "channel_dispatcher.h"
#include "bp.h"
#include "sound.h"
#include "event_queue.h"
#include "trigger.h"
#include "devices.h"
#include "temperature.h"
#include "calibration.h"
#include "touch.h"
#include "touch_calibration.h"
#include "mw_gui_gui.h"
#include "mw_gui_touch.h"

#if OPTION_ENCODER
#include "encoder.h"
#endif

#if OPTION_SD_CARD
#include "dlog.h"
#endif

#ifdef EEZ_PLATFORM_SIMULATOR
#include "platform/simulator/front_panel/control.h"
#endif

////////////////////////////////////////////////////////////////////////////////

#define CONF_GUI_ENTERING_STANDBY_PAGE_TIMEOUT 2000000L // 2s
#define CONF_GUI_STANDBY_PAGE_TIMEOUT 10000000L // 10s
#define CONF_GUI_DISPLAY_OFF_PAGE_TIMEOUT 2000000L // 2s
#define CONF_GUI_WELCOME_PAGE_TIMEOUT 2000000L // 2s
#define CONF_GUI_TOAST_DURATION_MS 2000L
#define CONF_DLOG_COLOR 62464

using namespace eez::psu::gui;

////////////////////////////////////////////////////////////////////////////////

namespace eez {
namespace psu {
namespace gui {

static persist_conf::DeviceFlags2 g_deviceFlags2;
static void(*g_checkAsyncOperationStatus)();
Channel *g_channel;
static char g_textMessage[32 + 1];
static uint8_t g_textMessageVersion;
static void(*g_errorMessageAction)();
static int g_errorMessageActionParam;
static void(*g_dialogYesCallback)();
static void(*g_dialogNoCallback)();
static void(*g_dialogCancelCallback)();
static void(*g_dialogLaterCallback)();
static WidgetCursor g_toggleOutputWidgetCursor;
static WidgetCursor g_foundTouchWidget;


void yesNoLater(const char *message, void(*yes_callback)(), void(*no_callback)(), void(*later_callback)() = 0) {
	data::set(data::Cursor(), DATA_ID_ALERT_MESSAGE, data::Value(message), 0);

	g_dialogYesCallback = yes_callback;
	g_dialogNoCallback = no_callback;
	g_dialogLaterCallback = later_callback;

	pushPage(PAGE_ID_YES_NO_LATER);
}

bool isChannelCalibrationsDone() {
	for (int i = 0; i < CH_NUM; ++i) {
		if (!Channel::get(i).isCalibrationExists()) {
			return false;
		}
	}
	return true;
}

bool isDateTimeSetupDone() {
	return persist_conf::devConf.flags.dateValid && persist_conf::devConf.flags.timeValid;
}

void channelCalibrationsYes() {
	executeAction(ACTION_ID_SHOW_SYS_SETTINGS_CAL);
}

void channelCalibrationsNo() {
	persist_conf::devConf2.flags.skipChannelCalibrations = 1;
	persist_conf::saveDevice2();
}

void dateTimeYes() {
	executeAction(ACTION_ID_SHOW_SYS_SETTINGS_DATE_TIME);
}

void dateTimeNo() {
	persist_conf::devConf2.flags.skipDateTimeSetup = 1;
	persist_conf::saveDevice2();
}

void serialYes() {
	executeAction(ACTION_ID_SHOW_SYS_SETTINGS_SERIAL);
}

void serialNo() {
	persist_conf::devConf2.flags.skipSerialSetup = 1;
	persist_conf::saveDevice2();
}

void ethernetYes() {
	executeAction(ACTION_ID_SHOW_SYS_SETTINGS_ETHERNET);
}

void ethernetNo() {
	persist_conf::devConf2.flags.skipEthernetSetup = 1;
	persist_conf::saveDevice2();
}

bool showSetupWizardQuestion() {
	if (!channel_dispatcher::isCoupled() && !channel_dispatcher::isTracked()) {
		if (!g_deviceFlags2.skipChannelCalibrations) {
			g_deviceFlags2.skipChannelCalibrations = 1;
			if (!isChannelCalibrationsDone()) {
				yesNoLater("Do you want to calibrate channels?", channelCalibrationsYes, channelCalibrationsNo);
				return true;
			}
		}
	}

	if (!g_deviceFlags2.skipSerialSetup) {
		g_deviceFlags2.skipSerialSetup = 1;
		if (!persist_conf::isSerialEnabled()) {
			yesNoLater("Do you want to setup serial port?", serialYes, serialNo);
			return true;
		}
	}

#if OPTION_ETHERNET
	if (!g_deviceFlags2.skipEthernetSetup) {
		g_deviceFlags2.skipEthernetSetup = 1;
		if (!persist_conf::isEthernetEnabled()) {
			yesNoLater("Do you want to setup ethernet?", ethernetYes, ethernetNo);
			return true;
		}
	}
#endif

	if (!g_deviceFlags2.skipDateTimeSetup) {
		g_deviceFlags2.skipDateTimeSetup = 1;
		if (!isDateTimeSetupDone()) {
			yesNoLater("Do you want to set date and time?", dateTimeYes, dateTimeNo);
			return true;
		}
	}

	return false;
}

void alertMessage(int alertPageId, data::Value message, void(*ok_callback)()) {
	data::set(data::Cursor(), DATA_ID_ALERT_MESSAGE, message, 0);

	g_dialogYesCallback = ok_callback;

	pushPage(alertPageId);

	if (alertPageId == PAGE_ID_ERROR_ALERT) {
		sound::playBeep();
	}
}

void longAlertMessage(int alertPageId, data::Value message, data::Value message2, void(*ok_callback)()) {
	data::set(data::Cursor(), DATA_ID_ALERT_MESSAGE_2, message2, 0);
	alertMessage(alertPageId, message, ok_callback);
}

void longErrorMessage(data::Value value1, data::Value value2, void(*ok_callback)()) {
	longAlertMessage(PAGE_ID_ERROR_LONG_ALERT, value1, value2, ok_callback);
}

void longErrorMessageP(const char *message1, const char *message2, void(*ok_callback)()) {
	longErrorMessage(data::Value(message1), data::Value(message2), ok_callback);
}

////////////////////////////////////////////////////////////////////////////////

data::Cursor g_focusCursor;
uint8_t g_focusDataId;
data::Value g_focusEditValue;

void setFocusCursor(const data::Cursor& cursor, uint8_t dataId) {
	g_focusCursor = cursor;
	g_focusDataId = dataId;
	g_focusEditValue = data::Value();
}

bool isFocusChanged() {
	return g_focusEditValue.getType() != VALUE_TYPE_NONE;
}

////////////////////////////////////////////////////////////////////////////////

#if OPTION_ENCODER

static bool g_isEncoderEnabledInActivePage;
uint32_t g_focusEditValueChangedTime;

bool isEncoderEnabledForWidget(const Widget *widget) {
	return widget->action == ACTION_ID_EDIT;
}

bool isEnabledFocusCursor(data::Cursor& cursor, uint8_t dataId) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	Channel &channel = Channel::get(iChannel);
	return channel.isOk() &&
		(channel_dispatcher::getVoltageTriggerMode(channel) == TRIGGER_MODE_FIXED || trigger::isIdle()) &&
		!(dataId == DATA_ID_CHANNEL_U_EDIT && channel.isRemoteProgrammingEnabled());
}

void isEncoderEnabledInActivePageCheckWidget(int pageId, const WidgetCursor &widgetCursor) {
	DECL_WIDGET(widget, widgetCursor.widgetOffset);
	if (isEncoderEnabledForWidget(widget)) {
		g_isEncoderEnabledInActivePage = true;
	}
}

bool isEncoderEnabledInActivePage() {
	// encoder is enabled if active page contains widget with "edit" action
	g_isEncoderEnabledInActivePage = false;
	enumWidgets(getActivePageId(), 0, 0, isEncoderEnabledInActivePageCheckWidget);
	return g_isEncoderEnabledInActivePage;
}

void moveToNextFocusCursor() {
	data::Cursor newCursor = g_focusCursor;
	uint8_t newDataId = g_focusDataId;

	for (int i = 0; i < CH_NUM * 2; ++i) {
		if (newDataId == DATA_ID_CHANNEL_U_EDIT) {
			newDataId = DATA_ID_CHANNEL_I_EDIT;
		} else {
			if (channel_dispatcher::isCoupled() || channel_dispatcher::isTracked()) {
				newCursor.i = 0;
			} else {
				newCursor.i = (newCursor.i + 1) % CH_NUM;
			}
			newDataId = DATA_ID_CHANNEL_U_EDIT;
		}

		if (isEnabledFocusCursor(newCursor, newDataId)) {
			setFocusCursor(newCursor, newDataId);
			if (edit_mode::isActive()) {
				edit_mode::update();
			}
			return;
		}
	}
}

bool onEncoderConfirmation() {
	if (edit_mode::isActive() && !edit_mode::isInteractiveMode() && edit_mode::getEditValue() != edit_mode::getCurrentValue()) {
		edit_mode::nonInteractiveSet();
		return true;
	}
	return false;
}

void onEncoder(int tickCount, int counter, bool clicked) {
	// wait for confirmation of changed value ...
	if (isFocusChanged() && tickCount - g_focusEditValueChangedTime >= ENCODER_CHANGE_TIMEOUT * 1000000L) {
		// ... on timeout discard changed value
		g_focusEditValue = data::Value();
	}

	if (!isEnabledFocusCursor(g_focusCursor, g_focusDataId)) {
		moveToNextFocusCursor();
	}

	if (counter != 0) {
		if (isFrontPanelLocked()) {
			return;
		}

		if (!isEnabledFocusCursor(g_focusCursor, g_focusDataId)) {
			moveToNextFocusCursor();
		}

		encoder::enableAcceleration(true);

		if (isEncoderEnabledInActivePage()) {
			data::Value value;
			if (persist_conf::devConf2.flags.encoderConfirmationMode && g_focusEditValue.getType() != VALUE_TYPE_NONE) {
				value = g_focusEditValue;
			} else {
				value = data::getEditValue(g_focusCursor, g_focusDataId);
			}

			float newValue = value.getFloat() + (value.getUnit() == UNIT_AMPER ? 0.001f : 0.01f) * counter;

			float min = data::getMin(g_focusCursor, g_focusDataId).getFloat();
			if (newValue < min) {
				newValue = min;
			}

			float max = data::getMax(g_focusCursor, g_focusDataId).getFloat();
			if (newValue > max) {
				newValue = max;
			}

			if (persist_conf::devConf2.flags.encoderConfirmationMode) {
				g_focusEditValue = MakeValue(newValue, value.getUnit(), g_focusCursor.i > 0 ? g_focusCursor.i : 0);
				g_focusEditValueChangedTime = micros();
			} else {
				int16_t error;
				if (!data::set(g_focusCursor, g_focusDataId, MakeValue(newValue, value.getUnit(), g_focusCursor.i), &error)) {
					errorMessage(g_focusCursor, data::MakeScpiErrorValue(error));
				}
			}
		}

		int activePageId = getActivePageId();

		if (activePageId == PAGE_ID_EDIT_MODE_KEYPAD || activePageId == PAGE_ID_NUMERIC_KEYPAD) {
			if (((NumericKeypad *)getActiveKeypad())->onEncoder(counter)) {
				return;
			}
		}

		if (activePageId == PAGE_ID_EDIT_MODE_STEP) {
			edit_mode_step::onEncoder(counter);
			return;
		}

		encoder::enableAcceleration(true);

		if (activePageId == PAGE_ID_EDIT_MODE_SLIDER) {
			edit_mode_slider::increment(counter);
			return;
		}
	}

	if (clicked) {
		if (isEncoderEnabledInActivePage()) {
			if (isFocusChanged()) {
				// confirmation
				int16_t error;
				if (!data::set(g_focusCursor, g_focusDataId, g_focusEditValue, &error)) {
					errorMessage(g_focusCursor, data::MakeScpiErrorValue(error));
				} else {
					g_focusEditValue = data::Value();
				}
			} else if (!onEncoderConfirmation()) {
				moveToNextFocusCursor();
			}

			playClickSound();
		}


		int activePageId = getActivePageId();
		if (activePageId == PAGE_ID_EDIT_MODE_KEYPAD || activePageId == PAGE_ID_NUMERIC_KEYPAD) {
			((NumericKeypad *)getActiveKeypad())->onEncoderClicked();
		}
	}

	Page *activePage = getActivePage();
	if (activePage) {
		if (counter) {
			activePage->onEncoder(counter);
		}

		if (clicked) {
			activePage->onEncoderClicked();
		}
	}
}

#endif

void turnScreenOff() {
	showPage(INTERNAL_PAGE_ID_NONE);
	lcd::turnOff();
}

void init() {
	g_deviceFlags2 = persist_conf::devConf2.flags;

	setFocusCursor(0, DATA_ID_CHANNEL_U_EDIT);

	lcd::init();

#ifdef EEZ_PLATFORM_SIMULATOR
	eez::psu::simulator::front_panel::open();
#endif

	touch::init();
	gui_init();
	touch_calibration::init();

#if OPTION_ENCODER
	encoder::init();
#endif
}

void tick(uint32_t tickCount) {
	if (touch_calibration::isCalibrating()) {
		return;
	}

#if OPTION_ENCODER
	int counter;
	bool clicked;
	encoder::read(counter, clicked);
	if (counter != 0 || clicked) {
		idle::noteEncoderActivity();
	}
	onEncoder(tickCount, counter, clicked);
#endif

	lcd::tick(tickCount);

	gui_tick(tickCount);

	int activePageId = getActivePageId();

	// wait some time for transitional pages
	if (activePageId == PAGE_ID_STANDBY) {
		if (int32_t(tickCount - g_showPageTime) < CONF_GUI_STANDBY_PAGE_TIMEOUT) {
			return;
		}
	} else if (activePageId == PAGE_ID_ENTERING_STANDBY) {
		if (int32_t(tickCount - g_showPageTime) >= CONF_GUI_ENTERING_STANDBY_PAGE_TIMEOUT) {
			uint32_t saved_showPageTime = g_showPageTime;
			showStandbyPage();
			g_showPageTime = saved_showPageTime;
		}
		return;
	}
	else if (activePageId == PAGE_ID_WELCOME) {
		if (int32_t(tickCount - g_showPageTime) < CONF_GUI_WELCOME_PAGE_TIMEOUT) {
			return;
		}
	}

	// turn the screen off if power is down
	if (!psu::isPowerUp()) {
		turnScreenOff();
		return;
	}

	// select page to go after transitional page
	if (activePageId == PAGE_ID_WELCOME || activePageId == PAGE_ID_STANDBY || activePageId == PAGE_ID_ENTERING_STANDBY) {
		if (!touch_calibration::isCalibrated()) {
			// touch screen is not calibrated
			showPage(PAGE_ID_SCREEN_CALIBRATION_INTRO);
		}
		else {
			showPage(getStartPageId());
		}
		return;
	}

	if (persist_conf::devConf2.flags.displayState == 0 && (activePageId != PAGE_ID_DISPLAY_OFF && activePageId != PAGE_ID_SELF_TEST_RESULT && touch_calibration::isCalibrated())) {
		showPage(PAGE_ID_DISPLAY_OFF);
		return;
	}
	else if (persist_conf::devConf2.flags.displayState == 1 && activePageId == PAGE_ID_DISPLAY_OFF) {
		lcd::turnOn();
		bp::switchStandby(false);
		showPage(PAGE_ID_MAIN);
		return;
	}

	//
	uint32_t inactivityPeriod = idle::getGuiAndEncoderInactivityPeriod();

#if GUI_BACK_TO_MAIN_ENABLED
	if (activePageId == PAGE_ID_EVENT_QUEUE ||
		activePageId == PAGE_ID_USER_PROFILES ||
		activePageId == PAGE_ID_USER_PROFILES2 ||
		activePageId == PAGE_ID_USER_PROFILE_0_SETTINGS ||
		activePageId == PAGE_ID_USER_PROFILE_SETTINGS) {
		if (inactivityPeriod >= GUI_BACK_TO_MAIN_DELAY * 1000UL) {
			showPage(PAGE_ID_MAIN);
		}
	}
#endif

	if (activePageId == PAGE_ID_SCREEN_CALIBRATION_INTRO) {
		if (inactivityPeriod >= 20 * 1000UL) {
			touch_calibration::enterCalibrationMode(PAGE_ID_SCREEN_CALIBRATION_YES_NO_CANCEL, getStartPageId());
			return;
		}
	}

	if (activePageId == PAGE_ID_TOAST3_ALERT || activePageId == PAGE_ID_ERROR_TOAST_ALERT) {
		if (inactivityPeriod >= CONF_GUI_TOAST_DURATION_MS) {
			dialogOk();
			return;
		}
	}

	if (activePageId == PAGE_ID_ASYNC_OPERATION_IN_PROGRESS) {
		data::set(data::Cursor(), DATA_ID_ASYNC_OPERATION_THROBBER, data::Value(g_throbber[(tickCount % 1000000) / 125000]), 0);
		if (g_checkAsyncOperationStatus) {
			g_checkAsyncOperationStatus();
		}
	}

	if (psu::g_rprogAlarm) {
		psu::g_rprogAlarm = false;
		longErrorMessage("Max. remote prog. voltage exceeded.", "Please remove it immediately!");
	}

	if (activePageId == PAGE_ID_DISPLAY_OFF) {
		if (lcd::isOn()) {
			if (int32_t(tickCount - g_showPageTime) >= CONF_GUI_DISPLAY_OFF_PAGE_TIMEOUT) {
				lcd::turnOff();
				g_showPageTime = tickCount;
			}
		}
		else {
			if (bp::isStandbyOn()) {
				if (int32_t(tickCount - g_showPageTime) >= 250000L) {
					bp::switchStandby(false);
					g_showPageTime = tickCount;
				}
			}
			else {
				if (int32_t(tickCount - g_showPageTime) >= 1300000L) {
					bp::switchStandby(true);
					g_showPageTime = tickCount;
				}
			}
		}
		return;
	}

	if (!isFrontPanelLocked() && activePageId == PAGE_ID_MAIN && int32_t(tickCount - g_showPageTime) >= 50000L) {
		if (showSetupWizardQuestion()) {
			return;
		}
	}
}

void touchHandling(uint32_t tickCount) {
	if (getActivePageId() == PAGE_ID_ENTERING_STANDBY) {
		return;
	}

	if (touch_calibration::isCalibrating()) {
		touch_calibration::tick(tickCount);
		return;
	}

	using namespace eez::mw::gui::touch;

	if (g_eventType != TOUCH_NONE) {
		idle::noteGuiActivity();
		gui_touchHandling(tickCount);
	}
}

void showWelcomePage() {
	showPage(PAGE_ID_WELCOME);
}

void showStandbyPage() {
	showPage(PAGE_ID_STANDBY);
}

void showEnteringStandbyPage() {
	showPage(PAGE_ID_ENTERING_STANDBY);
}

void showEthernetInit() {
	lcd::turnOn(true);
	if (persist_conf::isEthernetEnabled()) {
		showPage(PAGE_ID_ETHERNET_INIT);
	}
}

void showSelfTestResultPage() {
	showPage(PAGE_ID_SELF_TEST_RESULT);
}

void longInfoMessage(data::Value value1, data::Value value2, void(*ok_callback)()) {
	longAlertMessage(PAGE_ID_INFO_LONG_ALERT, value1, value2, ok_callback);
}

void toastMessageP(const char *message1, const char *message2, const char *message3, void(*ok_callback)()) {
	data::set(data::Cursor(), DATA_ID_ALERT_MESSAGE_3, message3, 0);
	longAlertMessage(PAGE_ID_TOAST3_ALERT, message1, message2, ok_callback);
}

void dialogYes() {
	popPage();

	if (g_dialogYesCallback) {
		g_dialogYesCallback();
	}
}

void dialogNo() {
	popPage();

	if (g_dialogNoCallback) {
		g_dialogNoCallback();
	}
}

void dialogCancel() {
	popPage();

	if (g_dialogCancelCallback) {
		g_dialogCancelCallback();
	}
}

void dialogOk() {
	dialogYes();
}

void dialogLater() {
	popPage();

	if (g_dialogLaterCallback) {
		g_dialogLaterCallback();
	}
}

void errorMessageAction() {
	popPage();

	if (g_dialogYesCallback) {
		g_dialogYesCallback();
	}

	g_errorMessageAction();
}

void infoMessage(data::Value value, void(*ok_callback)()) {
	alertMessage(PAGE_ID_INFO_ALERT, value, ok_callback);
}

void infoMessageP(const char *message, void(*ok_callback)()) {
	alertMessage(PAGE_ID_INFO_ALERT, data::Value(message), ok_callback);
}

void longInfoMessageP(const char *message1, const char *message2, void(*ok_callback)()) {
	longInfoMessage(data::Value(message1), data::Value(message2), ok_callback);
}

void setTextMessage(const char *message, unsigned int len) {
	strncpy(g_textMessage, message, len);
	g_textMessage[len] = 0;
	++g_textMessageVersion;
	if (getActivePageId() != PAGE_ID_TEXT_MESSAGE) {
		pushPage(PAGE_ID_TEXT_MESSAGE);
	}
}

void clearTextMessage() {
	if (getActivePageId() == PAGE_ID_TEXT_MESSAGE) {
		popPage();
	}
}

const char *getTextMessage() {
	return g_textMessage;
}

uint8_t getTextMessageVersion() {
	return g_textMessageVersion;
}

void changeLimit(Channel& channel, const data::Value& value, float minLimit, float maxLimit, float defLimit, void(*onSetLimit)(float)) {
	NumericKeypadOptions options;

	options.channelIndex = channel.index;

	options.editValueUnit = value.getUnit();

	options.min = minLimit;
	options.max = maxLimit;
	options.def = defLimit;

	options.enableMaxButton();
	options.enableDefButton();
	options.flags.signButtonEnabled = true;
	options.flags.dotButtonEnabled = true;

	NumericKeypad::start(0, value, options, onSetLimit);
}

void onSetVoltageLimit(float limit) {
	Channel& channel = Channel::get(g_errorMessageActionParam);
	channel_dispatcher::setVoltageLimit(channel, limit);
	infoMessageP("Voltage limit changed!", popPage);
}

void changeVoltageLimit() {
	Channel& channel = Channel::get(g_errorMessageActionParam);
	float minLimit = channel_dispatcher::getUMin(channel);
	float maxLimit = channel_dispatcher::getUMax(channel);
	float defLimit = channel_dispatcher::getUMax(channel);
	changeLimit(channel, MakeValue(channel_dispatcher::getULimit(channel), UNIT_VOLT, channel.index - 1), minLimit, maxLimit, defLimit, onSetVoltageLimit);
}

void onSetCurrentLimit(float limit) {
	Channel& channel = Channel::get(g_errorMessageActionParam);
	channel_dispatcher::setCurrentLimit(channel, limit);
	infoMessageP("Current limit changed!", popPage);
}

void changeCurrentLimit() {
	Channel& channel = Channel::get(g_errorMessageActionParam);
	float minLimit = channel_dispatcher::getIMin(channel);
	float maxLimit = channel_dispatcher::getIMax(channel);
	float defLimit = channel_dispatcher::getIMax(channel);
	changeLimit(channel, MakeValue(channel_dispatcher::getILimit(channel), UNIT_AMPER, channel.index - 1), minLimit, maxLimit, defLimit, onSetCurrentLimit);
}

void onSetPowerLimit(float limit) {
	Channel& channel = Channel::get(g_errorMessageActionParam);
	channel_dispatcher::setPowerLimit(channel, limit);
	infoMessageP("Power limit changed!", popPage);
}

void changePowerLimit() {
	Channel& channel = Channel::get(g_errorMessageActionParam);
	float minLimit = channel_dispatcher::getPowerMinLimit(channel);
	float maxLimit = channel_dispatcher::getPowerMaxLimit(channel);
	float defLimit = channel_dispatcher::getPowerDefaultLimit(channel);
	changeLimit(channel, MakeValue(channel_dispatcher::getPowerLimit(channel), UNIT_WATT, channel.index - 1), minLimit, maxLimit, defLimit, onSetPowerLimit);
}

void errorMessageP(const char *message, void(*ok_callback)()) {
	alertMessage(PAGE_ID_ERROR_ALERT, data::Value(message), ok_callback);
	sound::playBeep();
}

void yesNoDialog(int yesNoPageId, const char *message, void(*yes_callback)(), void(*no_callback)(), void(*cancel_callback)()) {
	data::set(data::Cursor(), DATA_ID_ALERT_MESSAGE, data::Value(message), 0);

	g_dialogYesCallback = yes_callback;
	g_dialogNoCallback = no_callback;
	g_dialogCancelCallback = cancel_callback;

	pushPage(yesNoPageId);
}

void areYouSure(void(*yes_callback)()) {
	yesNoDialog(PAGE_ID_YES_NO, "Are you sure?", yes_callback, 0, 0);
}

void areYouSureWithMessage(const char *message, void(*yes_callback)()) {
	yesNoDialog(PAGE_ID_ARE_YOU_SURE_WITH_MESSAGE, message, yes_callback, 0, 0);
}

void showAsyncOperationInProgress(const char *message, void(*checkStatus)()) {
	data::set(data::Cursor(), DATA_ID_ALERT_MESSAGE, data::Value(message), 0);
	g_checkAsyncOperationStatus = checkStatus;
	pushPage(PAGE_ID_ASYNC_OPERATION_IN_PROGRESS);
}

void hideAsyncOperationInProgress() {
	popPage();
}

void showProgressPage(const char *message, void(*abortCallback)()) {
	data::set(data::Cursor(), DATA_ID_ALERT_MESSAGE, data::Value(message), 0);
	g_dialogCancelCallback = abortCallback;
	pushPage(PAGE_ID_PROGRESS);
}

bool updateProgressPage(size_t processedSoFar, size_t totalSize) {
	if (getActivePageId() == PAGE_ID_PROGRESS) {
		if (totalSize > 0) {
			psu::gui::g_progress = data::Value((int)round((processedSoFar * 1.0f / totalSize) * 100.0f), VALUE_TYPE_PERCENTAGE);
		}
		else {
			psu::gui::g_progress = data::Value((uint32_t)processedSoFar, VALUE_TYPE_SIZE);
		}
		return true;
	}
	return false;
}

void hideProgressPage() {
	if (getActivePageId() == PAGE_ID_PROGRESS) {
		popPage();
	}
}

void channelReinitiateTrigger() {
	trigger::abort();
	channelInitiateTrigger();
}

void channelToggleOutput() {
	Channel& channel = Channel::get(g_foundWidgetAtDown.cursor.i >= 0 ? g_foundWidgetAtDown.cursor.i : 0);
	if (channel_dispatcher::isTripped(channel)) {
		errorMessageP("Channel is tripped!");
	}
	else {
		bool triggerModeEnabled = channel_dispatcher::getVoltageTriggerMode(channel) != TRIGGER_MODE_FIXED ||
			channel_dispatcher::getCurrentTriggerMode(channel) != TRIGGER_MODE_FIXED;

		if (channel.isOutputEnabled()) {
			if (triggerModeEnabled) {
				trigger::abort();
				for (int i = 0; i < CH_NUM; ++i) {
					Channel& channel = Channel::get(i);
					if (channel_dispatcher::getVoltageTriggerMode(channel) != TRIGGER_MODE_FIXED ||
						channel_dispatcher::getCurrentTriggerMode(channel) != TRIGGER_MODE_FIXED) {
						channel_dispatcher::outputEnable(Channel::get(i), false);
					}
				}
			}
			else {
				channel_dispatcher::outputEnable(channel, false);
			}
		}
		else {
			if (triggerModeEnabled) {
				if (trigger::isIdle()) {
					g_toggleOutputWidgetCursor = g_foundWidgetAtDown;
					pushPage(PAGE_ID_CH_START_LIST);
				}
				else if (trigger::isInitiated()) {
					trigger::abort();
					g_toggleOutputWidgetCursor = g_foundWidgetAtDown;
					pushPage(PAGE_ID_CH_START_LIST);
				}
				else {
					yesNoDialog(PAGE_ID_YES_NO, "Trigger is active. Re-initiate trigger?", channelReinitiateTrigger, 0, 0);
				}
			}
			else {
				channel_dispatcher::outputEnable(channel, true);
			}
		}
	}
}

void channelInitiateTrigger() {
	popPage();
	int err = trigger::initiate();
	if (err != SCPI_RES_OK) {
		mw::gui::errorMessage(g_toggleOutputWidgetCursor.cursor, data::MakeScpiErrorValue(err));
	}
}

void channelSetToFixed() {
	popPage();
	Channel& channel = Channel::get(g_toggleOutputWidgetCursor.cursor.i >= 0 ? g_toggleOutputWidgetCursor.cursor.i : 0);
	if (channel_dispatcher::getVoltageTriggerMode(channel) != TRIGGER_MODE_FIXED) {
		channel_dispatcher::setVoltageTriggerMode(channel, TRIGGER_MODE_FIXED);
	}
	if (channel_dispatcher::getCurrentTriggerMode(channel) != TRIGGER_MODE_FIXED) {
		channel_dispatcher::setCurrentTriggerMode(channel, TRIGGER_MODE_FIXED);
	}
	channel_dispatcher::outputEnable(channel, true);
}

void channelEnableOutput() {
	popPage();
	Channel& channel = Channel::get(g_toggleOutputWidgetCursor.cursor.i >= 0 ? g_toggleOutputWidgetCursor.cursor.i : 0);
	channel_dispatcher::outputEnable(channel, true);
}

void standBy() {
	changePowerState(false);
}

void turnDisplayOff() {
	persist_conf::setDisplayState(0);
}

void reset() {
	popPage();
	psu::reset();
}

void selectChannel() {
	if (g_foundWidgetAtDown.cursor.i >= 0) {
		g_channel = &Channel::get(g_foundWidgetAtDown.cursor.i);
	}
	else if (!g_channel || channel_dispatcher::isCoupled() || channel_dispatcher::isTracked()) {
		g_channel = &Channel::get(0);
	}
}

static void doUnlockFrontPanel() {
	popPage();

	if (persist_conf::lockFrontPanel(false)) {
		infoMessageP("Front panel is unlocked!");
	}
}

static void checkPasswordToUnlockFrontPanel() {
	checkPassword("Password: ", persist_conf::devConf2.systemPassword, doUnlockFrontPanel);
}

void lockFrontPanel() {
	if (persist_conf::lockFrontPanel(true)) {
		infoMessageP("Front panel is locked!");
	}
}

void unlockFrontPanel() {
	if (strlen(persist_conf::devConf2.systemPassword) > 0) {
		checkPasswordToUnlockFrontPanel();
	}
	else {
		if (persist_conf::lockFrontPanel(false)) {
			infoMessageP("Front panel is unlocked!");
		}
	}
}

static bool isChannelTripLastEvent(int i, event_queue::Event &lastEvent) {
	if (lastEvent.eventId == (event_queue::EVENT_ERROR_CH1_OVP_TRIPPED + i * 3) ||
		lastEvent.eventId == (event_queue::EVENT_ERROR_CH1_OCP_TRIPPED + i * 3) ||
		lastEvent.eventId == (event_queue::EVENT_ERROR_CH1_OPP_TRIPPED + i * 3) ||
		lastEvent.eventId == (event_queue::EVENT_ERROR_CH1_OTP_TRIPPED + i)) {
		return Channel::get(i).isTripped();
	}

	return false;
}

void onLastErrorEventAction() {
	event_queue::Event lastEvent;
	event_queue::getLastErrorEvent(&lastEvent);

	if (lastEvent.eventId == event_queue::EVENT_ERROR_AUX_OTP_TRIPPED && temperature::sensors[temp_sensor::AUX].isTripped()) {
		showPage(PAGE_ID_SYS_SETTINGS_AUX_OTP);
	}
	else if (isChannelTripLastEvent(0, lastEvent)) {
		g_channel = &Channel::get(0);
		showPage(PAGE_ID_CH_SETTINGS_PROT_CLEAR);
	}
	else if (isChannelTripLastEvent(1, lastEvent)) {
		g_channel = &Channel::get(1);
		showPage(PAGE_ID_CH_SETTINGS_PROT_CLEAR);
	}
	else {
		showPage(PAGE_ID_EVENT_QUEUE);
	}
}

void onTouchListGraph(const WidgetCursor &widgetCursor, int xTouch, int yTouch) {
	DECL_WIDGET(widget, widgetCursor.widgetOffset);
	DECL_WIDGET_SPECIFIC(ListGraphWidget, listGraphWidget, widget);

	if (xTouch < widgetCursor.x || xTouch >= widgetCursor.x + (int)widget->w) return;
	if (yTouch < widgetCursor.y || yTouch >= widgetCursor.y + (int)widget->h) return;

	int dwellListLength = data::getFloatListLength(listGraphWidget->dwellData);
	if (dwellListLength > 0) {
		int iCursor = -1;

		float *dwellList = data::getFloatList(listGraphWidget->dwellData);

		int maxListLength = data::getFloatListLength(widget->data);

		float dwellSum = 0;
		for (int i = 0; i < maxListLength; ++i) {
			if (i < dwellListLength) {
				dwellSum += dwellList[i];
			} else {
				dwellSum += dwellList[dwellListLength - 1];
			}
		}

		float currentDwellSum = 0;
		int xPrev = widgetCursor.x;
		for (int i = 0; i < maxListLength; ++i) {
			currentDwellSum += i < dwellListLength ? dwellList[i] : dwellList[dwellListLength - 1];
			int x1 = xPrev;
			int x2;
			if (i == maxListLength - 1) {
				x2 = widgetCursor.x + (int)widget->w - 1;
			} else {
				x2 = widgetCursor.x + int(currentDwellSum * (int)widget->w / dwellSum);
			}
			if (x2 < x1) x2 = x1;
			if (x2 >= widgetCursor.x + (int)widget->w) x2 = widgetCursor.x + (int)widget->w - 1;

			if (xTouch >= x1 && xTouch < x2) {
				int iCurrentCursor = data::get(widgetCursor.cursor, listGraphWidget->cursorData).getInt();
				iCursor = i * 3 + iCurrentCursor % 3;
				break;
			}
		}

		if (iCursor >= 0) {
			data::set(widgetCursor.cursor, listGraphWidget->cursorData, data::Value(iCursor), 0);
		}
	}
}

}
}
} // namespace eez::psu::gui

////////////////////////////////////////////////////////////////////////////////

using namespace eez::psu;

namespace eez {
namespace mw {
namespace gui {

bool isWidgetActionEnabledHook(const Widget *widget, bool &result) {
	if (isFrontPanelLocked()) {
		int activePageId = getActivePageId();
		if (activePageId == PAGE_ID_INFO_ALERT || activePageId == PAGE_ID_ERROR_ALERT || activePageId == PAGE_ID_KEYPAD ||
			activePageId == PAGE_ID_SCREEN_CALIBRATION_YES_NO || activePageId == PAGE_ID_SCREEN_CALIBRATION_YES_NO_CANCEL) {
			result = true;
			return true;
		}

		if (widget->action != ACTION_ID_SYS_FRONT_PANEL_UNLOCK) {
			result = false;
			return true;
		}
	}
	return false;
}

int transformStyleHook(const Widget *widget) {
	if (isFrontPanelLocked()) {
		if (widget->style == STYLE_ID_BOTTOM_BUTTON) {
			if (widget->action != ACTION_ID_SYS_FRONT_PANEL_UNLOCK) {
				return STYLE_ID_BOTTOM_BUTTON_DISABLED;
			}
		} else if (widget->style == STYLE_ID_EDIT_S) {
			return STYLE_ID_DEFAULT_S;
		} else if (widget->style == STYLE_ID_MON_VALUE) {
			return STYLE_ID_DEFAULT;
		} else if (widget->style == STYLE_ID_CHANNEL_OFF_LANDSCAPE) {
			return STYLE_ID_DEFAULT_L_LANDSCAPE;
		} else if (widget->style == STYLE_ID_EDIT_VALUE_ACTIVE_S_RIGHT) {
			return STYLE_ID_EDIT_VALUE_S_RIGHT;
		}
	}

	return widget->style;
}

bool isAutoRepeatActionHook(int action) {
	return
		action == ACTION_ID_KEYPAD_BACK ||
		action == ACTION_ID_UP_DOWN ||
		action == ACTION_ID_EVENT_QUEUE_PREVIOUS_PAGE ||
		action == ACTION_ID_EVENT_QUEUE_NEXT_PAGE ||
		action == ACTION_ID_CHANNEL_LISTS_PREVIOUS_PAGE ||
		action == ACTION_ID_CHANNEL_LISTS_NEXT_PAGE;
}

void onTouchDownHook(const WidgetCursor& foundWidget, int xTouch, int yTouch) {
	DECL_WIDGET(widget, foundWidget.widgetOffset);
	if (foundWidget && widget->type == WIDGET_TYPE_LIST_GRAPH) {
		g_foundTouchWidget = foundWidget;
		onTouchListGraph(g_foundTouchWidget, xTouch, yTouch);
		sound::playClick();
	} else {
		int activePageId = getActivePageId();
		if (activePageId == PAGE_ID_EDIT_MODE_SLIDER) {
			edit_mode_slider::onTouchDown();
		} else if (activePageId == PAGE_ID_EDIT_MODE_STEP) {
			edit_mode_step::onTouchDown();
		}
	}
}

void onTouchMoveHook(int xTouch, int yTouch) {
	int activePageId = getActivePageId();
	if (activePageId == PAGE_ID_SCREEN_CALIBRATION_YES_NO || activePageId == PAGE_ID_SCREEN_CALIBRATION_YES_NO_CANCEL) {
#ifdef CONF_DEBUG
		int x = xTouch;
		if (x < 1) x = 1;
		else if (x > lcd::getDisplayWidth() - 2) x = lcd::getDisplayWidth() - 2;

		int y = yTouch;
		if (y < 1) y = 1;
		else if (y > lcd::getDisplayHeight() - 2) y = lcd::getDisplayHeight() - 2;

		lcd::setColor(COLOR_WHITE);
		lcd::fillRect(x - 1, y - 1, x + 1, y + 1);
#endif
	} else {
		if (g_foundTouchWidget) {
			onTouchListGraph(g_foundTouchWidget, xTouch, yTouch);
		} else {
			int activePageId = getActivePageId();
			if (activePageId == PAGE_ID_EDIT_MODE_SLIDER) {
				edit_mode_slider::onTouchMove();
			} else if (activePageId == PAGE_ID_EDIT_MODE_STEP) {
				edit_mode_step::onTouchMove();
			}
		}
	}
}

bool onLongTouchHook() {
	int activePageId = getActivePageId();

	if (activePageId == INTERNAL_PAGE_ID_NONE || activePageId == PAGE_ID_STANDBY) {
		psu::changePowerState(true);
		return true;
	}

	if (activePageId == PAGE_ID_DISPLAY_OFF) {
		persist_conf::setDisplayState(1);
		return true;
	}

	return false;
}

bool onExtraLongTouchHook() {
	setPage(PAGE_ID_SCREEN_CALIBRATION_INTRO);
	return true;
}

bool onTouchUpHook() {
	int activePageId = getActivePageId();

	if (activePageId == PAGE_ID_SCREEN_CALIBRATION_INTRO) {
		touch_calibration::enterCalibrationMode(PAGE_ID_SCREEN_CALIBRATION_YES_NO_CANCEL, getStartPageId());
		return true;
	}

	if (activePageId == PAGE_ID_EDIT_MODE_SLIDER) {
		edit_mode_slider::onTouchUp();
		return true;
	}

	if (activePageId == PAGE_ID_EDIT_MODE_STEP) {
		edit_mode_step::onTouchUp();
		return true;
	}

	return false;
}

bool testExecuteActionOnTouchDownHook(int action) {
	return action == ACTION_ID_CHANNEL_TOGGLE_OUTPUT;
}

void executeUserActionHook(int actionId) {
	g_actionExecFunctions[actionId]();
}

void errorMessage(const data::Cursor& cursor, data::Value value, void(*ok_callback)()) {
	int errorPageId = PAGE_ID_ERROR_ALERT;

	if (value.getType() == VALUE_TYPE_SCPI_ERROR) {
		void(*action)() = 0;
		const char *actionLabel = 0;

		int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
		Channel& channel = Channel::get(iChannel);

		if (value.getScpiError() == SCPI_ERROR_VOLTAGE_LIMIT_EXCEEDED) {
			if (channel_dispatcher::getULimit(channel) < channel_dispatcher::getUMaxLimit(channel)) {
				action = changeVoltageLimit;
				actionLabel = "Change voltage limit";
			} else {
				errorPageId = PAGE_ID_ERROR_TOAST_ALERT;
			}
		} else if (value.getScpiError() == SCPI_ERROR_CURRENT_LIMIT_EXCEEDED) {
			if (channel_dispatcher::getILimit(channel) < channel_dispatcher::getIMaxLimit(channel)) {
				action = changeCurrentLimit;
				actionLabel = "Change current limit";
			} else {
				errorPageId = PAGE_ID_ERROR_TOAST_ALERT;
			}
		} else if (value.getScpiError() == SCPI_ERROR_POWER_LIMIT_EXCEEDED) {
			if (channel_dispatcher::getPowerLimit(channel) < channel_dispatcher::getPowerMaxLimit(channel)) {
				action = changePowerLimit;
				actionLabel = "Change power limit";
			} else {
				errorPageId = PAGE_ID_ERROR_TOAST_ALERT;
			}
		}

		if (action) {
			data::set(data::Cursor(), DATA_ID_ALERT_MESSAGE_2, actionLabel, 0);
			g_errorMessageAction = action;
			g_errorMessageActionParam = iChannel;
			errorPageId = PAGE_ID_ERROR_ALERT_WITH_ACTION;
		}
	}

	alertMessage(errorPageId, value, ok_callback);
	sound::playBeep();
}

Page *createPageFromId(int pageId) {
	switch (pageId) {
	case PAGE_ID_SELF_TEST_RESULT: return new SelfTestResultPage();
	case PAGE_ID_EVENT_QUEUE: return new EventQueuePage();
	case PAGE_ID_CH_SETTINGS_PROT: return new ChSettingsProtectionPage();
	case PAGE_ID_CH_SETTINGS_PROT_OVP: return new ChSettingsOvpProtectionPage();
	case PAGE_ID_CH_SETTINGS_PROT_OCP: return new ChSettingsOcpProtectionPage();
	case PAGE_ID_CH_SETTINGS_PROT_OPP: return new ChSettingsOppProtectionPage();
	case PAGE_ID_CH_SETTINGS_PROT_OTP: return new ChSettingsOtpProtectionPage();
	case PAGE_ID_CH_SETTINGS_ADV_LRIPPLE: return new ChSettingsAdvLRipplePage();
	case PAGE_ID_CH_SETTINGS_ADV_REMOTE: return new ChSettingsAdvRemotePage();
	case PAGE_ID_CH_SETTINGS_ADV_RANGES: return new ChSettingsAdvRangesPage();
	case PAGE_ID_CH_SETTINGS_ADV_TRACKING: return new ChSettingsAdvTrackingPage();
	case PAGE_ID_CH_SETTINGS_ADV_COUPLING:
	case PAGE_ID_CH_SETTINGS_ADV_COUPLING_INFO: return new ChSettingsAdvCouplingPage();
	case PAGE_ID_CH_SETTINGS_ADV_VIEW: return new ChSettingsAdvViewPage();
	case PAGE_ID_CH_SETTINGS_TRIGGER: return new ChSettingsTriggerPage();
	case PAGE_ID_CH_SETTINGS_LISTS: return new ChSettingsListsPage();
	case PAGE_ID_SYS_SETTINGS_DATE_TIME: return new SysSettingsDateTimePage();
#if OPTION_ETHERNET
	case PAGE_ID_SYS_SETTINGS_ETHERNET: return new SysSettingsEthernetPage();
	case PAGE_ID_SYS_SETTINGS_ETHERNET_STATIC: return new SysSettingsEthernetStaticPage();
#endif
	case PAGE_ID_SYS_SETTINGS_PROTECTIONS: return new SysSettingsProtectionsPage();
	case PAGE_ID_SYS_SETTINGS_TRIGGER: return new SysSettingsTriggerPage();
	case PAGE_ID_SYS_SETTINGS_IO: return new SysSettingsIOPinsPage();
	case PAGE_ID_SYS_SETTINGS_AUX_OTP: return new SysSettingsAuxOtpPage();
	case PAGE_ID_SYS_SETTINGS_SOUND: return new SysSettingsSoundPage();
#if OPTION_ENCODER
	case PAGE_ID_SYS_SETTINGS_ENCODER: return new SysSettingsEncoderPage();
#endif
	case PAGE_ID_SYS_SETTINGS_SERIAL: return new SysSettingsSerialPage();
	case PAGE_ID_USER_PROFILES:
	case PAGE_ID_USER_PROFILES2:
	case PAGE_ID_USER_PROFILE_0_SETTINGS:
	case PAGE_ID_USER_PROFILE_SETTINGS: return new UserProfilesPage();
	}

	return 0;
}

void onPageChanged() {
	psu::gui::lcd::turnOn();

	// clear text message if active page is not PAGE_ID_TEXT_MESSAGE
	if (getActivePageId() != PAGE_ID_TEXT_MESSAGE && g_textMessage[0]) {
		g_textMessage[0] = 0;
	}

	// clear
	g_focusEditValue = data::Value();

	idle::noteGuiActivity();
}

int getMainPageId() {
	return PAGE_ID_MAIN;
}

int getStartPageId() {
	if (devices::anyFailed()) {
		return PAGE_ID_SELF_TEST_RESULT;
	}
	return getMainPageId();
}

void onScaleUpdatedHook(int dataId, bool scaleIsVertical, int scaleWidth, float scaleHeight) {
	if (dataId == DATA_ID_EDIT_VALUE) {
		edit_mode_slider::scale_is_vertical = scaleIsVertical;
		edit_mode_slider::scale_width = scaleWidth;
		edit_mode_slider::scale_height = scaleHeight;
	}
}

bool executeCriticalTasks(int pageId) {
	return criticalTick(pageId);
}

uint16_t getWidgetBackgroundColorHook(const WidgetCursor& widgetCursor, const Style* style) {
#if OPTION_SD_CARD
	DECL_WIDGET(widget, widgetCursor.widgetOffset);
	int iChannel = widgetCursor.cursor.i >= 0 ? widgetCursor.cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	if (widget->data == DATA_ID_CHANNEL_U_EDIT || widget->data == DATA_ID_CHANNEL_U_MON_DAC) {
		if (dlog::g_logVoltage[iChannel]) {
			return CONF_DLOG_COLOR;
		}
	} else if (widget->data == DATA_ID_CHANNEL_I_EDIT) {
		if (dlog::g_logCurrent[iChannel]) {
			return CONF_DLOG_COLOR;
		}
	} else if (widget->data == DATA_ID_CHANNEL_P_MON) {
		if (dlog::g_logPower[iChannel]) {
			return CONF_DLOG_COLOR;
		}
	}
#endif
	return style->background_color;
}

bool isBlinkingHook(const data::Cursor &cursor, uint8_t id) {
	if ((g_focusCursor == cursor || channel_dispatcher::isCoupled()) && g_focusDataId == id && g_focusEditValue.getType() != VALUE_TYPE_NONE) {
		return true;
	}

	return false;
}

void playClickSound() {
	sound::playClick();
}

bool isFocusWidget(const WidgetCursor &widgetCursor) {
	if (isPageActiveOrOnStack(PAGE_ID_CH_SETTINGS_LISTS)) {
		return ((ChSettingsListsPage *)getActivePage())->isFocusWidget(widgetCursor);
	}

	int iChannel = widgetCursor.cursor.i >= 0 ? widgetCursor.cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	if (channel_dispatcher::getVoltageTriggerMode(Channel::get(iChannel)) != TRIGGER_MODE_FIXED && !trigger::isIdle()) {
		return false;
	}

	if (psu::calibration::isEnabled()) {
		return false;
	}

	DECL_WIDGET(widget, widgetCursor.widgetOffset);
	return (widgetCursor.cursor == -1 || widgetCursor.cursor == g_focusCursor) && widget->data == g_focusDataId;
}

const Style *getSelectFromEnumContainerStyle() {
	DECL_STYLE(style, STYLE_ID_SELECT_ENUM_ITEM_POPUP_CONTAINER);
	return style;
}

const Style *getSelectFromEnumItemStyle() {
	DECL_STYLE(style, STYLE_ID_SELECT_ENUM_ITEM_POPUP_ITEM);
	return style;
}

const Style *getSelectFromEnumDisabledItemStyle() {
	DECL_STYLE(style, STYLE_ID_SELECT_ENUM_ITEM_POPUP_DISABLED_ITEM);
	return style;
}

}
}
} // namespace eez::mw::gui

////////////////////////////////////////////////////////////////////////////////

namespace eez {
namespace mw {
namespace gui {
namespace data {

int getNumHistoryValues(uint8_t id) {
	return CHANNEL_HISTORY_SIZE;
}

int getCurrentHistoryValuePosition(const Cursor &cursor, uint8_t id) {
	int iChannel = cursor.i >= 0 ? cursor.i : (g_channel ? (g_channel->index - 1) : 0);
	return Channel::get(iChannel).getCurrentHistoryValuePosition();
}

Value getHistoryValue(const Cursor &cursor, uint8_t id, int position) {
	Value value = position;
	g_dataOperationsFunctions[id](data::DATA_OPERATION_GET_HISTORY_VALUE, (Cursor &)cursor, value);
	return value;
}

}
}
}
} // eez::mw::gui::data

#endif
