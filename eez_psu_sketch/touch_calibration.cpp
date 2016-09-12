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
#include "touch.h"
#include "touch_filter.h"
#include "lcd.h"
#include "gui_internal.h"

#define CONF_GUI_TOUCH_SCREEN_CALIBRATION_M 16

namespace eez {
namespace psu {
namespace gui {
namespace touch {
namespace calibration {

const int RECT_SIZE = 2 * CONF_GUI_TOUCH_SCREEN_CALIBRATION_M;

////////////////////////////////////////////////////////////////////////////////

static enum {
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

void draw_cross(int x, int y, word color) {
    last_cross_x = x;
    last_cross_y = y;

    lcd::lcd.setColor(color);
    lcd::lcd.drawVLine(x + RECT_SIZE / 2, y, RECT_SIZE - 1);
    lcd::lcd.drawVLine(x + RECT_SIZE / 2 + 1, y, RECT_SIZE - 1);
    lcd::lcd.drawHLine(x, y + RECT_SIZE / 2, RECT_SIZE - 1);
    lcd::lcd.drawHLine(x, y + RECT_SIZE / 2 + 1, RECT_SIZE - 1);

    lcd::lcd.setColor(VGA_BLACK);
    lcd::lcd.fillRect(
        x + RECT_SIZE / 2 - 2, y + RECT_SIZE / 2 - 2,
        x + RECT_SIZE / 2 + 3, y + RECT_SIZE / 2 + 3);
}

void draw_point(int x, int y) {
    if (x == lcd::lcd.getDisplayXSize() - 1) x -= RECT_SIZE;
    if (y == lcd::lcd.getDisplayYSize() - 1) y -= RECT_SIZE;
    
    lcd::lcd.setColor(VGA_BLACK);
    lcd::lcd.clrScr();
    lcd::lcd.fillRect(0, 0, lcd::lcd.getDisplayXSize() - 1, lcd::lcd.getDisplayYSize() - 1);
    
    draw_cross(x, y, VGA_WHITE);
}

bool read_point() {
    if (touch::event_type != touch::TOUCH_NONE)
        DebugTraceF("Calibration point: %d, %d", touch::x, touch::y);

    if (touch::event_type == touch::TOUCH_DOWN) {
        draw_cross(last_cross_x, last_cross_y, VGA_GREEN);
    } else if (touch::event_type == touch::TOUCH_UP) {
        *point_x = touch::x;
        *point_y = touch::y;
        return true;
    }

    return false;
}

void init() {
    bool success;
    
    if (persist_conf::dev_conf.touch_screen_cal_orientation == DISPLAY_ORIENTATION) {
        success = touch::calibrateTransform(
            persist_conf::dev_conf.touch_screen_cal_tlx,
            persist_conf::dev_conf.touch_screen_cal_tly, 
            persist_conf::dev_conf.touch_screen_cal_brx,
            persist_conf::dev_conf.touch_screen_cal_bry, 
            persist_conf::dev_conf.touch_screen_cal_trx,
            persist_conf::dev_conf.touch_screen_cal_try, 
            CONF_GUI_TOUCH_SCREEN_CALIBRATION_M,
            lcd::lcd.getDisplayXSize(), lcd::lcd.getDisplayYSize()
        );
    } else {
        success = false;
    }

    if (!success) {
        mode = MODE_START;
    } else {
        mode = MODE_FINISHED;
    }
}

void enterCalibrationMode() {
    touch::resetTransformCalibration();
    mode = MODE_START;
}

bool isCalibrated() {
    return mode == MODE_FINISHED;
}

void dialogYes() {
    persist_conf::dev_conf.touch_screen_cal_orientation = DISPLAY_ORIENTATION;
    persist_conf::dev_conf.touch_screen_cal_tlx = point_tlx;
    persist_conf::dev_conf.touch_screen_cal_tly = point_tly;
    persist_conf::dev_conf.touch_screen_cal_brx = point_brx;
    persist_conf::dev_conf.touch_screen_cal_bry = point_bry;
    persist_conf::dev_conf.touch_screen_cal_trx = point_trx;
    persist_conf::dev_conf.touch_screen_cal_try = point_try;

    persist_conf::saveDevice();

    showPage(PAGE_ID_MAIN);
}

void dialogNo() {
    enterCalibrationMode();
}

void dialogCancel() {
    showPage(PAGE_ID_MAIN);
}

void tick(unsigned long tick_usec) {
    if (mode != MODE_FINISHED) {
        if (mode == MODE_START) {
            point_x = &point_tlx;
            point_y = &point_tly;
            draw_point(0, 0);
            mode = MODE_POINT_TL;
        } else if (mode == MODE_POINT_TL) {
            if (read_point()) {
                point_x = &point_brx;
                point_y = &point_bry;
                draw_point(lcd::lcd.getDisplayXSize() - 1, lcd::lcd.getDisplayYSize() - 1);
                mode = MODE_POINT_BR;
            }
        } else if (mode == MODE_POINT_BR) {
            if (read_point()) {
                point_x = &point_trx;
                point_y = &point_try;
                draw_point(lcd::lcd.getDisplayXSize() - 1, 0);
                mode = MODE_POINT_TR;
            }
        } else if (mode == MODE_POINT_TR) {
            if (read_point()) {
                bool success = touch::calibrateTransform(
                    point_tlx, point_tly,  point_brx, point_bry,  point_trx, point_try,
                    CONF_GUI_TOUCH_SCREEN_CALIBRATION_M, lcd::lcd.getDisplayXSize(), lcd::lcd.getDisplayYSize());
                if (success) {
                    mode = MODE_FINISHED;
                    yesNoDialog(PSTR("Save changes?"), dialogYes, dialogNo, dialogCancel);
                } else {
                    mode = MODE_START;
                }
            }
        }
    }
}

}
}
}
}
} // namespace eez::psu::ui::touch::calibration