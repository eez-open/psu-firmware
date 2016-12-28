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
#include "persist_conf.h"

#include "gui_internal.h"
#include "gui_password.h"
#include "gui_keypad.h"

namespace eez {
namespace psu {
namespace gui {

static void (*g_checkPasswordOkCallback)();
static const char *g_oldPassword;
static char g_newPassword[PASSWORD_MAX_LENGTH];

////////////////////////////////////////////////////////////////////////////////

static void checkPasswordOkCallback(char *text) {
    int nPassword = strlen(g_oldPassword);
    int nText = strlen(text);
    if (nPassword == nText && strncmp(g_oldPassword, text, nText) == 0) {
        g_checkPasswordOkCallback();
    } else {
        // entered password doesn't match, 
        errorMessageP(PSTR("Invalid password!"), popPage);
    }
}

static void checkPassword(const char *label, void (*ok)()) {
    g_checkPasswordOkCallback = ok;
    keypad::startPush(label, 0, PASSWORD_MAX_LENGTH, true, checkPasswordOkCallback, popPage);
}

////////////////////////////////////////////////////////////////////////////////

static void onRetypeNewPasswordOk(char *text) {
    size_t textLen = strlen(text);
    if (strlen(g_newPassword) != textLen || strncmp(g_newPassword, text, textLen) != 0) {
        // retyped new password doesn't match
        errorMessageP(PSTR("Password doesn't match!"), popPage);
        return;
    }
    
    bool isChanged;

    if (g_oldPassword == persist_conf::devConf2.systemPassword) {
        isChanged = persist_conf::changeSystemPassword(g_newPassword, strlen(g_newPassword));
    } else {
        isChanged = persist_conf::changeCalibrationPassword(g_newPassword, strlen(g_newPassword));
    }

    if (!isChanged) {
        // failed to save changed password
        errorMessageP(PSTR("Failed to change password!"), popPage);
        return;
    }

    // success
    infoMessageP(PSTR("Password changed!"), popPage);
}

static void onNewPasswordOk(char *text) {
    int textLength = strlen(text);

    bool isOk;
    int16_t err;
    if (g_oldPassword == persist_conf::devConf2.systemPassword) {
        isOk = persist_conf::isSystemPasswordValid(text, textLength, err);
    } else {
        isOk = persist_conf::isCalibrationPasswordValid(text, textLength, err);
    }
    
    if (!isOk) {
        // invalid password, return to keypad
        if (err == SCPI_ERROR_SYS_PASSWORD_TOO_SHORT || err == SCPI_ERROR_CAL_PASSWORD_TOO_SHORT) {
            errorMessageP(PSTR("Password too short!"));
        } else {
            errorMessageP(PSTR("Password too long!"));
        }
        return;
    }

    strcpy(g_newPassword, text);
    keypad::startReplace(PSTR("Retype new password: "), 0, PASSWORD_MAX_LENGTH, true, onRetypeNewPasswordOk, popPage);
}

static void onOldPasswordOk() {
    keypad::startReplace(PSTR("New password: "), 0, PASSWORD_MAX_LENGTH, true, onNewPasswordOk, popPage);
}

static void editPassword(const char *oldPassword) {
    g_oldPassword = oldPassword;

    if (strlen(g_oldPassword)) {
        checkPassword(PSTR("Current password: "), onOldPasswordOk);
    } else {
        keypad::startPush(PSTR("New password: "), 0, PASSWORD_MAX_LENGTH, true, onNewPasswordOk, popPage);
    }
}

////////////////////////////////////////////////////////////////////////////////

void checkPassword(const char *label PROGMEM, const char *password, void (*ok)()) {
    if (strlen(password) == 0) {
        ok();
    } else {
        g_oldPassword = password;
        checkPassword(label, ok);
    }
}

void editSystemPassword() {
    editPassword(persist_conf::devConf2.systemPassword);
}

void editCalibrationPassword() {
    editPassword(persist_conf::devConf.calibration_password);
}

}
}
} // namespace eez::psu::gui
