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

#include "channel.h"
#include "actions.h"
#include "devices.h"
#include "sound.h"
#include "event_queue.h"
#if OPTION_ENCODER
#include "encoder.h"
#endif

#include "gui.h"
#include "gui_internal.h"
#include "gui_password.h"
#include "gui_data_snapshot.h"
#include "gui_edit_mode.h"
#include "gui_edit_mode_slider.h"
#include "gui_edit_mode_step.h"
#include "gui_edit_mode_keypad.h"
#include "gui_widget_button_group.h"
#include "gui_page_self_test_result.h"
#include "gui_page_main.h"
#include "gui_page_event_queue.h"
#include "gui_page_ch_settings_protection.h"
#include "gui_page_ch_settings_adv.h"
#include "gui_page_ch_settings_info.h"
#include "gui_page_sys_settings.h"
#include "gui_page_sys_info.h"
#include "gui_page_user_profiles.h"

#ifdef EEZ_PSU_SIMULATOR
#include "front_panel/control.h"
#endif

#define CONF_GUI_PAGE_NAVIGATION_STACK_SIZE 5

#define CONF_GUI_STANDBY_PAGE_TIMEOUT 10000000UL // 10s
#define CONF_GUI_ENTERING_STANDBY_PAGE_TIMEOUT 5000000UL // 5s
#define CONF_GUI_WELCOME_PAGE_TIMEOUT 2000000UL // 2s
#define CONF_GUI_LONG_PRESS_TIMEOUT 1000000UL // 1s

#define CONF_GUI_KEYPAD_AUTO_REPEAT_DELAY 200000UL // 200ms

#define INTERNAL_PAGE_ID_NONE             -1
#define INTERNAL_PAGE_ID_SELECT_FROM_ENUM -2

namespace eez {
namespace psu {
namespace gui {

using namespace lcd;

////////////////////////////////////////////////////////////////////////////////

Styles *g_styles;
Document *g_document;

#if defined(EEZ_PSU_ARDUINO_MEGA)
static Styles g_stylesBuffer;
static Document g_documentBuffer;
#endif

static int g_activePageId;
static Page *g_activePage;

static int g_previousActivePageId;

static struct {
    int activePageId;
    Page *activePage;
} g_pageNavigationStack[CONF_GUI_PAGE_NAVIGATION_STACK_SIZE];
static int g_pageNavigationStackPointer = 0;

WidgetCursor g_foundWidgetAtDown;

static void (*g_dialogYesCallback)();
static void (*g_dialogNoCallback)();
static void (*g_dialogCancelCallback)();

static void (*g_errorMessageAction)();

static unsigned long g_showPageTime;
static unsigned long g_timeOfLastActivity;
static unsigned long g_touchDownTime;
static bool g_touchActionExecuted;

Channel *g_channel;

static data::Cursor g_wasFocusCursor;
static uint8_t g_wasFocusDataId;
data::Cursor g_focusCursor;
uint8_t g_focusDataId;

Page *createPageFromId(int pageId) {
    switch (pageId) {
    case PAGE_ID_SELF_TEST_RESULT: return new SelfTestResultPage();
    case PAGE_ID_MAIN: return new MainPage();
    case PAGE_ID_EVENT_QUEUE: return new EventQueuePage();
    case PAGE_ID_CH_SETTINGS_PROT: return new ChSettingsProtectionPage();
    case PAGE_ID_CH_SETTINGS_PROT_OVP: return new ChSettingsOvpProtectionPage();
    case PAGE_ID_CH_SETTINGS_PROT_OCP: return new ChSettingsOcpProtectionPage();
    case PAGE_ID_CH_SETTINGS_PROT_OPP: return new ChSettingsOppProtectionPage();
    case PAGE_ID_CH_SETTINGS_PROT_OTP: return new ChSettingsOtpProtectionPage();
    case PAGE_ID_CH_SETTINGS_ADV: return new ChSettingsAdvPage();
    case PAGE_ID_CH_SETTINGS_ADV_LRIPPLE: return new ChSettingsAdvLRipplePage();
    case PAGE_ID_CH_SETTINGS_ADV_RSENSE: return new ChSettingsAdvRSensePage();
    case PAGE_ID_CH_SETTINGS_ADV_RPROG: return new ChSettingsAdvRProgPage();
    case PAGE_ID_CH_SETTINGS_ADV_TRACKING: return new ChSettingsAdvTrackingPage();
    case PAGE_ID_CH_SETTINGS_ADV_COUPLING:
    case PAGE_ID_CH_SETTINGS_ADV_COUPLING_INFO: return new ChSettingsAdvCouplingPage();
    case PAGE_ID_CH_SETTINGS_ADV_VIEW: return new ChSettingsAdvViewPage();
    case PAGE_ID_CH_SETTINGS_INFO: return new ChSettingsInfoPage();
    case PAGE_ID_SYS_SETTINGS_DATE_TIME: return new SysSettingsDateTimePage();
    case PAGE_ID_SYS_SETTINGS_ETHERNET: return new SysSettingsEthernetPage();
    case PAGE_ID_SYS_SETTINGS_PROTECTIONS: return new SysSettingsProtectionsPage();
    case PAGE_ID_SYS_SETTINGS_AUX_OTP: return new SysSettingsAuxOtpPage();
    case PAGE_ID_SYS_SETTINGS_SOUND: return new SysSettingsSoundPage();
    case PAGE_ID_SYS_INFO:
    case PAGE_ID_SYS_INFO2: return new SysInfoPage();
    case PAGE_ID_USER_PROFILES:
    case PAGE_ID_USER_PROFILES2:
    case PAGE_ID_USER_PROFILE_0_SETTINGS:
    case PAGE_ID_USER_PROFILE_SETTINGS: return new UserProfilesPage();
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////

bool isActivePageInternal() {
    return g_activePageId < INTERNAL_PAGE_ID_NONE;
}

////////////////////////////////////////////////////////////////////////////////

int getActivePageId() {
    return g_activePageId;
}

Page *getActivePage() {
    return g_activePage;
}

int getPreviousActivePageId() {
    return g_previousActivePageId;
}

void doShowPage(int index, Page *page = 0) {
    lcd::turnOn();

    // delete current page
    if (g_activePage) {
        delete g_activePage;
    }

    g_previousActivePageId = g_activePageId;
    g_activePageId = index;

    if (page) {
        g_activePage = page;
    } else {
        g_activePage = createPageFromId(g_activePageId);
    }

    if (g_activePage) {
        g_activePage->pageWillAppear();
    }

    g_showPageTime = micros();
    g_timeOfLastActivity = millis();

    refreshPage();
}

void setPage(int index) {
    // delete stack
    for (int i = 0; i < g_pageNavigationStackPointer; ++i) {
        if (g_pageNavigationStack[i].activePage) {
            delete g_pageNavigationStack[i].activePage;
        }
    }
    g_pageNavigationStackPointer = 0;

    //
    doShowPage(index);
}

void replacePage(int index) {
    doShowPage(index);
}

void pushPage(int index, Page *page) {
    // push current page on stack
    if (g_activePageId != INTERNAL_PAGE_ID_NONE) {
        if (g_pageNavigationStackPointer == CONF_GUI_PAGE_NAVIGATION_STACK_SIZE) {
            // no more space on the stack

            // delete page on the bottom
            if (g_pageNavigationStack[0].activePage) {
                delete g_pageNavigationStack[0].activePage;
            }

            // move stack one down
            for (int i = 1; i < g_pageNavigationStackPointer; ++i) {
                g_pageNavigationStack[i-1].activePageId = g_pageNavigationStack[i].activePageId;
                g_pageNavigationStack[i-1].activePage = g_pageNavigationStack[i].activePage;
            }

            --g_pageNavigationStackPointer;
        }

        g_pageNavigationStack[g_pageNavigationStackPointer].activePageId = g_activePageId;
        g_pageNavigationStack[g_pageNavigationStackPointer].activePage = g_activePage;
        g_activePage = 0;
        ++g_pageNavigationStackPointer;
    }

    doShowPage(index, page);
}

void popPage() {
    if (g_pageNavigationStackPointer > 0) {
        --g_pageNavigationStackPointer;

        doShowPage(g_pageNavigationStack[g_pageNavigationStackPointer].activePageId,
            g_pageNavigationStack[g_pageNavigationStackPointer].activePage);
    } else {
        doShowPage(PAGE_ID_MAIN);
    }
}

void showWelcomePage() {
    setPage(PAGE_ID_WELCOME);
    flush();
}

void showSelfTestResultPage() {
    setPage(PAGE_ID_SELF_TEST_RESULT);
    flush();
}

void showStandbyPage() {
    setPage(PAGE_ID_STANDBY);
    flush();
}

void showEnteringStandbyPage() {
    setPage(PAGE_ID_ENTERING_STANDBY);
    flush();
}

void showEthernetInit() {
    doShowPage(PAGE_ID_ETHERNET_INIT);
    flush();
}

void pushSelectFromEnumPage(const data::EnumItem *enumDefinition, uint8_t currentValue, uint8_t disabledValue, void (*onSet)(uint8_t)) {
    pushPage(INTERNAL_PAGE_ID_SELECT_FROM_ENUM, new SelectFromEnumPage(enumDefinition, currentValue, disabledValue, onSet));
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

void errorMessageAction() {
    popPage();

    if (g_dialogYesCallback) {
        g_dialogYesCallback();
    }

    g_errorMessageAction();
}

void alertMessage(int alertPageId, data::Value message, void (*ok_callback)()) {
    data::set(data::Cursor(), DATA_ID_ALERT_MESSAGE, message, 0);

    g_dialogYesCallback = ok_callback;

    pushPage(alertPageId);

    if (alertPageId == PAGE_ID_ERROR_ALERT) {
        sound::playBeep();
    }
}

void longAlertMessage(int alertPageId, data::Value message, data::Value message2, void (*ok_callback)()) {
    data::set(data::Cursor(), DATA_ID_ALERT_MESSAGE_2, message2, 0);
    alertMessage(alertPageId, message, ok_callback);
}

void infoMessage(data::Value value, void (*ok_callback)()) {
    alertMessage(PAGE_ID_INFO_ALERT, value, ok_callback);
}

void infoMessageP(const char *message PROGMEM, void (*ok_callback)()) {
    alertMessage(PAGE_ID_INFO_ALERT, data::Value::ProgmemStr(message), ok_callback);
}

void longInfoMessage(data::Value value1, data::Value value2, void (*ok_callback)()) {
    longAlertMessage(PAGE_ID_INFO_LONG_ALERT, value1, value2, ok_callback);
}

void longInfoMessageP(const char *message1 PROGMEM, const char *message2 PROGMEM, void (*ok_callback)()) {
    longInfoMessage(data::Value::ProgmemStr(message1), data::Value::ProgmemStr(message2), ok_callback);
}

void toastMessageP(const char *message1 PROGMEM, const char *message2 PROGMEM, const char *message3 PROGMEM, void (*ok_callback)()) {
    data::set(data::Cursor(), DATA_ID_ALERT_MESSAGE_3, message3, 0);
    longAlertMessage(PAGE_ID_TOAST3_ALERT, message1, message2, ok_callback);
}

void errorMessage(data::Value value, void (*ok_callback)()) {
    int pageId = PAGE_ID_ERROR_ALERT;

    if (value.getType() == data::VALUE_TYPE_SCPI_ERROR_TEXT) {
        void (*action)();
        const char *actionLabel PROGMEM = 0;

        if (value.getScpiError() == SCPI_ERROR_VOLTAGE_LIMIT_EXCEEDED) {
            action = actions[ACTION_ID_SHOW_CH_SETTINGS_PROT_OVP];
            actionLabel = PSTR("Change voltage limit");
        } else if (value.getScpiError() == SCPI_ERROR_CURRENT_LIMIT_EXCEEDED) {
            action = actions[ACTION_ID_SHOW_CH_SETTINGS_PROT_OCP];
            actionLabel = PSTR("Change current limit");
        } else if (value.getScpiError() == SCPI_ERROR_POWER_LIMIT_EXCEEDED) {
            action = actions[ACTION_ID_SHOW_CH_SETTINGS_PROT_OPP];
            actionLabel = PSTR("Change power limit");
        }

        if (action) {
            data::set(data::Cursor(), DATA_ID_ALERT_MESSAGE_2, actionLabel, 0);
            g_errorMessageAction = action;
            alertMessage(PAGE_ID_ERROR_ALERT_WITH_ACTION, value, ok_callback);
            return;
        }
    }

    alertMessage(PAGE_ID_ERROR_ALERT, value, ok_callback);
}


void errorMessageP(const char *message PROGMEM, void (*ok_callback)()) {
    alertMessage(PAGE_ID_ERROR_ALERT, data::Value::ProgmemStr(message), ok_callback);
}

void yesNoDialog(int yesNoPageId, const char *message PROGMEM, void (*yes_callback)(), void (*no_callback)(), void (*cancel_callback)()) {
    data::set(data::Cursor(), DATA_ID_ALERT_MESSAGE, data::Value::ProgmemStr(message), 0);

    g_dialogYesCallback = yes_callback;
    g_dialogNoCallback = no_callback;
    g_dialogCancelCallback = cancel_callback;

    pushPage(yesNoPageId);
}

void areYouSure(void (*yes_callback)()) {
    yesNoDialog(PAGE_ID_YES_NO, PSTR("Are you sure?"), yes_callback, 0, 0);
}

void areYouSureWithMessage(const char *message PROGMEM, void (*yes_callback)()) {
    yesNoDialog(PAGE_ID_ARE_YOU_SURE_WITH_MESSAGE, message, yes_callback, 0, 0);
}

static bool isChannelTripLastEvent(int i, event_queue::Event &lastEvent) {
    if (lastEvent.eventId == (event_queue::EVENT_ERROR_CH1_OVP_TRIPPED + i * 3) ||
        lastEvent.eventId == (event_queue::EVENT_ERROR_CH1_OCP_TRIPPED + i * 3) ||
        lastEvent.eventId == (event_queue::EVENT_ERROR_CH1_OPP_TRIPPED + i * 3) ||
        lastEvent.eventId == (event_queue::EVENT_ERROR_CH1_OTP_TRIPPED + i)) 
    {
        return Channel::get(i).isTripped();
    }

    return false;
}

void onLastErrorEventAction() {
    event_queue::Event lastEvent;
    event_queue::getLastErrorEvent(&lastEvent);

    if (lastEvent.eventId == event_queue::EVENT_ERROR_AUX_OTP_TRIPPED && temperature::sensors[temp_sensor::AUX].isTripped()) {
        setPage(PAGE_ID_SYS_SETTINGS_AUX_OTP);
    } else if (isChannelTripLastEvent(0, lastEvent)) {
        g_channel = &Channel::get(0);
        setPage(PAGE_ID_CH_SETTINGS_PROT_CLEAR);
    } else if (isChannelTripLastEvent(1, lastEvent)) {
        g_channel = &Channel::get(1);
        setPage(PAGE_ID_CH_SETTINGS_PROT_CLEAR);
    } else {
        setPage(PAGE_ID_EVENT_QUEUE);
    }
}

static void doUnlockFrontPanel() {
    popPage();

    if (persist_conf::lockFrontPanel(false)) {
        infoMessageP(PSTR("Front panel is unlocked!"));
    }
}

static void checkPasswordToUnlockFrontPanel() {
    checkPassword(PSTR("Password: "), persist_conf::devConf2.systemPassword, doUnlockFrontPanel);
}

void lockFrontPanel() {
    if (persist_conf::lockFrontPanel(true)) {
        infoMessageP(PSTR("Front panel is locked!"));
    }
}

void unlockFrontPanel() {
    if (strlen(persist_conf::devConf2.systemPassword) > 0) {
        checkPasswordToUnlockFrontPanel();
    } else {
        if (persist_conf::lockFrontPanel(false)) {
            infoMessageP(PSTR("Front panel is unlocked!"));
        }
    }
}

bool isWidgetActionEnabled(const Widget *widget) {
    if (widget->action) {
        if (isFrontPanelLocked()) {
            if (g_activePageId == PAGE_ID_INFO_ALERT || g_activePageId == PAGE_ID_ERROR_ALERT || g_activePageId == PAGE_ID_KEYPAD) {
                return true;
            }

            if (widget->action != ACTION_ID_SYS_FRONT_PANEL_UNLOCK) {
                return false;
            }
        }

        return true;
    }

    return false;
}

int transformStyle(const Widget *widget) {
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

////////////////////////////////////////////////////////////////////////////////

void selectChannel() {
    if (g_foundWidgetAtDown.cursor.i >= 0) {
        g_channel = &Channel::get(g_foundWidgetAtDown.cursor.i);
    } else {
        g_channel = &Channel::get(0);
    }
}

////////////////////////////////////////////////////////////////////////////////

void setFocusCursor(const data::Cursor& cursor, uint8_t dataId) {
    g_focusCursor = cursor;
    g_focusDataId = dataId;
#if OPTION_ENCODER
    encoder::setSpeedMultiplier(g_focusDataId == DATA_ID_CHANNEL_U_SET ? 2 : 1);
#endif
}

bool wasFocusWidget(const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);
    return widgetCursor.cursor == g_wasFocusCursor && widget->data == g_wasFocusDataId;
}

bool isFocusWidget(const WidgetCursor &widgetCursor) {
    DECL_WIDGET(widget, widgetCursor.widgetOffset);
    return widgetCursor.cursor == g_focusCursor && widget->data == g_focusDataId;
}

////////////////////////////////////////////////////////////////////////////////

bool isLongPressAction(int actionId) {
    return actionId == ACTION_ID_TURN_OFF ||
        actionId == ACTION_ID_SYS_FRONT_PANEL_LOCK || 
        actionId == ACTION_ID_SYS_FRONT_PANEL_UNLOCK;
}

ActionType getAction(WidgetCursor &widgetCursor) {
    if (isActivePageInternal()) {
        return ((InternalPage *)g_activePage)->getAction(widgetCursor);
    } else {
        DECL_WIDGET(widget, widgetCursor.widgetOffset);
        return widget->action;
    }
}


void executeAction(int actionId) {
    sound::playClick();
    actions[actionId]();
}

////////////////////////////////////////////////////////////////////////////////

void standbyTouchHandling(unsigned long tick_usec) {
    // touch handling in power off:
    // wait for long press anywhere on the screen and then turn power on
    if (touch::event_type == touch::TOUCH_DOWN) {
        g_touchDownTime = tick_usec;
        g_touchActionExecuted = false;
    } else if (touch::event_type == touch::TOUCH_MOVE) {
        if (tick_usec - g_touchDownTime >= CONF_GUI_LONG_PRESS_TIMEOUT) {
            if (!g_touchActionExecuted) {
                g_touchActionExecuted = true;
                psu::changePowerState(true);
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void init() {
    setFocusCursor(0, DATA_ID_CHANNEL_U_SET);

    lcd::init();

#if OPTION_ENCODER
    encoder::init();
#endif

#ifdef EEZ_PSU_SIMULATOR
    if (true || persist_conf::devConf.gui_opened) {
        simulator::front_panel::open();
    }
#endif

    touch::init();

    g_activePageId = INTERNAL_PAGE_ID_NONE;
    g_activePage = 0;

#if defined(EEZ_PSU_ARDUINO_MEGA)
    arduino_util::prog_read_buffer(styles, (uint8_t *)&g_stylesBuffer, sizeof(Styles));
    g_styles = &g_stylesBuffer;

    arduino_util::prog_read_buffer(pages, (uint8_t *)&g_documentBuffer, sizeof(Document));
    g_document = &g_documentBuffer;
#else
    g_styles = (Styles *)styles;
    g_document = (Document *)document;
#endif
}

int getStartPageId() {
    return devices::anyFailed() ? PAGE_ID_SELF_TEST_RESULT: PAGE_ID_MAIN;
}

void tick(unsigned long tick_usec) {
#ifdef EEZ_PSU_SIMULATOR
    if (!simulator::front_panel::isOpened()) {
        return;
    }
#endif

    touch::tick(tick_usec);

#if OPTION_ENCODER
    int counter = encoder::readAndResetCounter();
    if (counter && g_activePage) {
        g_activePage->onEncoder(counter);
    }
#endif

    if (g_activePageId == INTERNAL_PAGE_ID_NONE) {
        standbyTouchHandling(tick_usec);
        return;
    }

    // wait some time for transitional pages
    if (g_activePageId == PAGE_ID_STANDBY && tick_usec - g_showPageTime < CONF_GUI_STANDBY_PAGE_TIMEOUT) {
        standbyTouchHandling(tick_usec);
        return;
    } else if (g_activePageId == PAGE_ID_ENTERING_STANDBY && tick_usec - g_showPageTime < CONF_GUI_ENTERING_STANDBY_PAGE_TIMEOUT) {
        if (!psu::isPowerUp()) {
            unsigned long saved_showPageTime = g_showPageTime;
            showStandbyPage();
            g_showPageTime = saved_showPageTime - (CONF_GUI_STANDBY_PAGE_TIMEOUT - CONF_GUI_ENTERING_STANDBY_PAGE_TIMEOUT);
        }
        return;
    } else if (g_activePageId == PAGE_ID_WELCOME && tick_usec - g_showPageTime < CONF_GUI_WELCOME_PAGE_TIMEOUT) {
        return;
    }

    // turn the screen off if power is down
    if (!psu::isPowerUp()) {
        standbyTouchHandling(tick_usec);
        g_activePageId = INTERNAL_PAGE_ID_NONE;
        g_activePage = 0;
        turnOff();
        return;
    }

    if (touch::calibration::isCalibrating()) {
        touch::calibration::tick(tick_usec);
        return;
    }

    // select page to go after transitional page
    if (g_activePageId == PAGE_ID_WELCOME || g_activePageId == PAGE_ID_STANDBY || g_activePageId == PAGE_ID_ENTERING_STANDBY) {
        if (!touch::calibration::isCalibrated()) {
            // touch screen is not calibrated
            setPage(PAGE_ID_SCREEN_CALIBRATION_INTRO);
        } else {
            setPage(getStartPageId());
        }
        return;
    }

    // touch handling
    if (touch::event_type != touch::TOUCH_NONE) {
        g_timeOfLastActivity = millis();

        if (touch::event_type == touch::TOUCH_DOWN) {
            if (g_activePageId == PAGE_ID_SCREEN_CALIBRATION_INTRO) {
                return;
            }

            g_touchDownTime = tick_usec;
            g_touchActionExecuted = false;
            WidgetCursor foundWidget = findWidget(touch::x, touch::y);
            g_foundWidgetAtDown = 0;
            if (foundWidget) {
                if (isActivePageInternal()) {
                    g_foundWidgetAtDown = foundWidget;
                } else {
                    DECL_WIDGET(widget, foundWidget.widgetOffset);
                    if (isWidgetActionEnabled(widget)) {
                        g_foundWidgetAtDown = foundWidget;
                    }
                }

                if (g_foundWidgetAtDown) {
                    selectWidget(g_foundWidgetAtDown);
                } else {
                    if (!isActivePageInternal()) {
                        DECL_WIDGET(widget, foundWidget.widgetOffset);
                        if (foundWidget && widget->type == WIDGET_TYPE_BUTTON_GROUP) {
                            widgetButtonGroup::onTouchDown(foundWidget);
                        } else if (g_activePageId == PAGE_ID_EDIT_MODE_SLIDER) {
                            edit_mode_slider::onTouchDown();
                        } else if (g_activePageId == PAGE_ID_EDIT_MODE_STEP) {
                            edit_mode_step::onTouchDown();
                        }
                    }
                }
            }
        } else if (touch::event_type == touch::TOUCH_MOVE) {
            if (g_activePageId == PAGE_ID_SCREEN_CALIBRATION_INTRO) {
                return;
            }

            if (g_foundWidgetAtDown) {
                ActionType action = getAction(g_foundWidgetAtDown);
                if (action == ACTION_ID_TURN_OFF) {
                    if (tick_usec - g_touchDownTime >= CONF_GUI_LONG_PRESS_TIMEOUT) {
                        if (!g_touchActionExecuted) {
                            deselectWidget();
                            g_foundWidgetAtDown = 0;
                            g_touchActionExecuted = true;
                            sound::playClick();
                            psu::changePowerState(false);
                        }
                    }
                } else if (action == ACTION_ID_SYS_FRONT_PANEL_LOCK || action == ACTION_ID_SYS_FRONT_PANEL_UNLOCK) {
                    if (tick_usec - g_touchDownTime >= CONF_GUI_LONG_PRESS_TIMEOUT) {
                        if (!g_touchActionExecuted) {
                            deselectWidget();
                            g_foundWidgetAtDown = 0;
                            g_touchActionExecuted = true;
                            sound::playClick();
                            if (isFrontPanelLocked()) {
                                unlockFrontPanel();
                            } else {
                                lockFrontPanel();
                            }
                        }
                    }
                } else if (action == ACTION_ID_KEYPAD_BACK) {
                    if (tick_usec - g_touchDownTime >= CONF_GUI_KEYPAD_AUTO_REPEAT_DELAY) {
                        g_touchDownTime = tick_usec;
                        g_touchActionExecuted = true;
                        executeAction(action);
                    }
                }
            } else {
                if (g_activePageId == PAGE_ID_EDIT_MODE_SLIDER) {
                    edit_mode_slider::onTouchMove();
                } else if (g_activePageId == PAGE_ID_EDIT_MODE_STEP) {
                    edit_mode_step::onTouchMove();
                } else if (g_activePageId == PAGE_ID_SCREEN_CALIBRATION_YES_NO || g_activePageId == PAGE_ID_SCREEN_CALIBRATION_YES_NO_CANCEL) {
    #ifdef CONF_DEBUG
                    int x = touch::x;
                    if (x < 1) x = 1;
                    else if (x > lcd::lcd.getDisplayXSize() - 2) x = lcd::lcd.getDisplayXSize() - 2;

                    int y = touch::y;
                    if (y < 1) y = 1;
                    else if (y > lcd::lcd.getDisplayYSize() - 2) y = lcd::lcd.getDisplayYSize() - 2;

                    lcd::lcd.setColor(VGA_WHITE);
                    lcd::lcd.fillRect(touch::x-1, touch::y-1, touch::x+1, touch::y+1);
    #endif
                }
            }
        } else if (touch::event_type == touch::TOUCH_UP) {
            if (g_activePageId == PAGE_ID_SCREEN_CALIBRATION_INTRO) {
                touch::calibration::enterCalibrationMode(PAGE_ID_SCREEN_CALIBRATION_YES_NO_CANCEL, getStartPageId());
                return;
            }

            if (g_foundWidgetAtDown) {
                deselectWidget();
                ActionType action = getAction(g_foundWidgetAtDown);
                if (!isLongPressAction(action)) {
                    executeAction(action);
                }
                g_foundWidgetAtDown = 0;
            } else {
                if (g_activePageId == PAGE_ID_EDIT_MODE_SLIDER) {
                    edit_mode_slider::onTouchUp();
                } else if (g_activePageId == PAGE_ID_EDIT_MODE_STEP) {
                    edit_mode_step::onTouchUp();
                }
            }
        }
    }

    //
    unsigned long inactivityPeriod = millis() - g_timeOfLastActivity;

#if GUI_BACK_TO_MAIN_ENABLED
    if (g_activePageId == PAGE_ID_EVENT_QUEUE ||
        g_activePageId == PAGE_ID_USER_PROFILES ||
        g_activePageId == PAGE_ID_USER_PROFILES2 ||
        g_activePageId == PAGE_ID_USER_PROFILE_0_SETTINGS ||
        g_activePageId == PAGE_ID_USER_PROFILE_SETTINGS)
    {
        if (inactivityPeriod >= GUI_BACK_TO_MAIN_DELAY * 1000UL) {
            setPage(PAGE_ID_MAIN);
        }
    }
#endif

    if (g_activePageId == PAGE_ID_SCREEN_CALIBRATION_INTRO) {
        if (inactivityPeriod >= 20 * 1000UL) {
            touch::calibration::enterCalibrationMode(PAGE_ID_SCREEN_CALIBRATION_YES_NO_CANCEL, getStartPageId());
            return;
        }
    }

    if (g_activePageId == PAGE_ID_TOAST3_ALERT) {
        if (inactivityPeriod >= 2 * 1000UL) {
            dialogOk();
            return;
        }
    }

    // update screen
    if (!touch::calibration::isCalibrating()) {
        drawTick();

        g_wasFocusCursor = g_focusCursor;
        g_wasFocusDataId = g_focusDataId;
    }
}

}
}
} // namespace eez::psu::gui
