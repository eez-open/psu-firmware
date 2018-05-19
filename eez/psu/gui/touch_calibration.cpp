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

#include "eez/psu/psu.h"

#if OPTION_DISPLAY

#include "eez/mw/gui/touch.h"
#include "eez/mw/gui/touch_filter.h"
#include "eez/mw/gui/lcd.h"
#include "eez/psu/gui/psu.h"
#include "eez/psu/sound.h"

#define CONF_GUI_TOUCH_SCREEN_CALIBRATION_M 16

////////////////////////////////////////////////////////////////////////////////

using namespace eez::mw::gui::touch;

namespace eez {
namespace psu {
namespace gui {
namespace touch_calibration {

const int RECT_SIZE = 2 * CONF_GUI_TOUCH_SCREEN_CALIBRATION_M;

static int g_yesNoPageId;
static int g_nextPageId;

static enum {
	MODE_NOT_CALIBRATED,
    MODE_CALIBRATED,
    MODE_START,
    MODE_POINT_TL,
    MODE_POINT_BR,
    MODE_POINT_TR,
	MODE_FINISHED
} mode;

static int point_tlx;
static int point_tly;

static int point_brx;
static int point_bry;

static int point_trx;
static int point_try;

static int *point_x;
static int *point_y;

static int last_cross_x;
static int last_cross_y;

static bool g_wasDown = false;

void draw_cross(int x, int y, uint16_t color) {
    last_cross_x = x;
    last_cross_y = y;

    lcd::setColor(color);
    lcd::drawVLine(x + RECT_SIZE / 2, y, RECT_SIZE - 1);
    lcd::drawVLine(x + RECT_SIZE / 2 + 1, y, RECT_SIZE - 1);
    lcd::drawHLine(x, y + RECT_SIZE / 2, RECT_SIZE - 1);
    lcd::drawHLine(x, y + RECT_SIZE / 2 + 1, RECT_SIZE - 1);

    lcd::setColor(COLOR_BLACK);
    lcd::fillRect(
        x + RECT_SIZE / 2 - 2, y + RECT_SIZE / 2 - 2,
        x + RECT_SIZE / 2 + 3, y + RECT_SIZE / 2 + 3);
}

void draw_point(int x, int y) {
    if (x == lcd::getDisplayWidth() - 1) x -= RECT_SIZE;
    if (y == lcd::getDisplayHeight() - 1) y -= RECT_SIZE;

    lcd::setColor(COLOR_BLACK);
    lcd::fillRect(0, 0, lcd::getDisplayWidth() - 1, lcd::getDisplayHeight() - 1);

    draw_cross(x, y, COLOR_WHITE);
}

bool read_point() {
    if (g_eventType == TOUCH_DOWN) {
		g_wasDown = true;
        draw_cross(last_cross_x, last_cross_y, COLOR_GREEN);
    } else if (g_wasDown && g_eventType == TOUCH_UP) {
		g_wasDown = false;
        *point_x = g_x;
        *point_y = g_y;
        return true;
    }

    return false;
}

void init() {
    bool success;

    if (persist_conf::devConf.touch_screen_cal_orientation == DISPLAY_ORIENTATION) {
        success = touch::calibrateTransform(
            persist_conf::devConf.touch_screen_cal_tlx,
            persist_conf::devConf.touch_screen_cal_tly,
            persist_conf::devConf.touch_screen_cal_brx,
            persist_conf::devConf.touch_screen_cal_bry,
            persist_conf::devConf.touch_screen_cal_trx,
            persist_conf::devConf.touch_screen_cal_try,
            CONF_GUI_TOUCH_SCREEN_CALIBRATION_M,
            lcd::getDisplayWidth(), lcd::getDisplayHeight()
        );
    } else {
        success = false;
    }

    if (success) {
        mode = MODE_CALIBRATED;
    } else {
        mode = MODE_NOT_CALIBRATED;
    }
}

void startCalibration() {
	touch::resetTransformCalibration();
    mode = MODE_START;
}

void enterCalibrationMode(int yesNoPageId, int nextPageId) {
	g_yesNoPageId = yesNoPageId;
	g_nextPageId = nextPageId;

	startCalibration();

	Channel::saveAndDisableOE();
}

void leaveCalibrationMode() {
	mode = MODE_CALIBRATED;
    if (g_nextPageId == -1) {
        popPage();
    } else {
        showPage(g_nextPageId);
    }

	Channel::restoreOE();
}

bool isCalibrated() {
    return mode == MODE_CALIBRATED;
}

bool isCalibrating() {
    return mode != MODE_CALIBRATED && mode != MODE_NOT_CALIBRATED && mode != MODE_FINISHED;
}

void dialogYes() {
	persist_conf::devConf.touch_screen_cal_orientation = DISPLAY_ORIENTATION;
    persist_conf::devConf.touch_screen_cal_tlx = point_tlx;
    persist_conf::devConf.touch_screen_cal_tly = point_tly;
    persist_conf::devConf.touch_screen_cal_brx = point_brx;
    persist_conf::devConf.touch_screen_cal_bry = point_bry;
    persist_conf::devConf.touch_screen_cal_trx = point_trx;
    persist_conf::devConf.touch_screen_cal_try = point_try;

    persist_conf::saveDevice();

	leaveCalibrationMode();
}

void dialogNo() {
    startCalibration();
}

void dialogCancel() {
	leaveCalibrationMode();
}

void startAgain() {
    mode = MODE_START;
}

void tick(uint32_t tick_usec) {
    if (mode == MODE_START) {
        point_x = &point_tlx;
        point_y = &point_tly;
        draw_point(0, 0);
        mode = MODE_POINT_TL;
    } else if (mode == MODE_POINT_TL) {
        if (read_point()) {
            sound::playClick();
            point_x = &point_brx;
            point_y = &point_bry;
            draw_point(lcd::getDisplayWidth() - 1, lcd::getDisplayHeight() - 1);
            mode = MODE_POINT_BR;
        }
    } else if (mode == MODE_POINT_BR) {
        if (read_point()) {
            sound::playClick();
            point_x = &point_trx;
            point_y = &point_try;
            draw_point(lcd::getDisplayWidth() - 1, 0);
            mode = MODE_POINT_TR;
        }
    } else if (mode == MODE_POINT_TR) {
        if (read_point()) {
            sound::playClick();
            bool success = touch::calibrateTransform(
                point_tlx, point_tly,  point_brx, point_bry,  point_trx, point_try,
                CONF_GUI_TOUCH_SCREEN_CALIBRATION_M, lcd::getDisplayWidth(), lcd::getDisplayHeight());
            if (success) {
				mode = MODE_FINISHED;
                yesNoDialog(g_yesNoPageId, "Save changes?", dialogYes, dialogNo, dialogCancel);
            } else {
                mode = MODE_FINISHED;
                toastMessageP("Received data is invalid due to", "imprecise pointing or", "communication problem!", startAgain);
            }
        }
    }
}

}
}
}
} // namespace eez::psu::gui::touch_calibration

#endif