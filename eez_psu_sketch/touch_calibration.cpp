/*
 * EEZ PSU Firmware
 * Copyright (C) 2015 Envox d.o.o.
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

#define CONF_TOUCH_SCREEN_CALIBRATION_M 16

namespace eez {
namespace psu {
namespace gui {
namespace touch {
namespace calibration {

////////////////////////////////////////////////////////////////////////////////

static enum {
    TOUCH_CAL_START,
    TOUCH_CAL_POINT_TL,
    TOUCH_CAL_POINT_BR,
    TOUCH_CAL_POINT_TR,
    TOUCH_CAL_FINISHED

} touch_cal_mode;

int touch_cal_point_tlx;
int touch_cal_point_tly;

int touch_cal_point_brx;
int touch_cal_point_bry;

int touch_cal_point_trx;
int touch_cal_point_try;

int *touch_cal_point_x;
int *touch_cal_point_y;

void touch_cal_draw_point(int x, int y) {
    const int RECT_SIZE = 2 * CONF_TOUCH_SCREEN_CALIBRATION_M;
    
    if (x == lcd::lcd.getDisplayXSize()) x -= RECT_SIZE;
    if (y == lcd::lcd.getDisplayYSize()) y -= RECT_SIZE;
    
    lcd::lcd.setColor(VGA_BLACK);
    lcd::lcd.clrScr();
    lcd::lcd.fillRect(0, 0, lcd::lcd.getDisplayXSize() - 1, lcd::lcd.getDisplayYSize() - 1);
    
    lcd::lcd.setColor(VGA_WHITE);
    lcd::lcd.drawVLine(x + RECT_SIZE / 2, y, RECT_SIZE);
    lcd::lcd.drawVLine(x + RECT_SIZE / 2 + 1, y, RECT_SIZE);
    lcd::lcd.drawHLine(x, y + RECT_SIZE / 2, RECT_SIZE);
    lcd::lcd.drawHLine(x, y + RECT_SIZE / 2 + 1, RECT_SIZE);

    lcd::lcd.setColor(VGA_BLACK);
    lcd::lcd.fillRect(x + RECT_SIZE / 2 - 2, y + RECT_SIZE / 2 - 2, x + RECT_SIZE / 2 + 3, y + RECT_SIZE / 2 + 3);
}

bool touch_call_read_point() {
    if (touch::event_type != touch::TOUCH_NONE)
        DebugTraceF("Calibration point: %d, %d", touch::x, touch::y);

    if (touch::event_type == touch::TOUCH_UP) {
        *touch_cal_point_x = touch::x;
        *touch_cal_point_y = touch::y;
        return true;
    } else {
        return false;
    }
}

void init() {
    bool touch_cal_success = touch::calibrate_transform(
        persist_conf::dev_conf.touch_screen_cal_tlx,
        persist_conf::dev_conf.touch_screen_cal_tly, 
        persist_conf::dev_conf.touch_screen_cal_brx,
        persist_conf::dev_conf.touch_screen_cal_bry, 
        persist_conf::dev_conf.touch_screen_cal_trx,
        persist_conf::dev_conf.touch_screen_cal_try, 
        CONF_TOUCH_SCREEN_CALIBRATION_M
    );

    if (!touch_cal_success) {
        touch_cal_mode = TOUCH_CAL_START;
    } else {
        touch_cal_mode = TOUCH_CAL_FINISHED;
    }
}

void enter_calibration_mode() {
    touch::reset_transform_calibration();
    touch_cal_mode = TOUCH_CAL_START;
}

bool is_calibrated() {
    return touch_cal_mode == TOUCH_CAL_FINISHED;
}

void onYes() {
    persist_conf::dev_conf.touch_screen_cal_tlx = touch_cal_point_tlx;
    persist_conf::dev_conf.touch_screen_cal_tly = touch_cal_point_tly;
    persist_conf::dev_conf.touch_screen_cal_brx = touch_cal_point_brx;
    persist_conf::dev_conf.touch_screen_cal_bry = touch_cal_point_bry;
    persist_conf::dev_conf.touch_screen_cal_trx = touch_cal_point_trx;
    persist_conf::dev_conf.touch_screen_cal_try = touch_cal_point_try;

    persist_conf::saveDevice();

    page_index = PAGE_MAIN;
    refresh_page();
}

void onNo() {
    enter_calibration_mode();
}

void tick(unsigned long tick_usec) {
    if (touch_cal_mode != TOUCH_CAL_FINISHED) {
        if (touch_cal_mode == TOUCH_CAL_START) {
            touch_cal_point_x = &touch_cal_point_tlx;
            touch_cal_point_y = &touch_cal_point_tly;
            touch_cal_draw_point(0, 0);
            touch_cal_mode = TOUCH_CAL_POINT_TL;
        } else if (touch_cal_mode == TOUCH_CAL_POINT_TL) {
            if (touch_call_read_point()) {
                touch_cal_point_x = &touch_cal_point_brx;
                touch_cal_point_y = &touch_cal_point_bry;
                touch_cal_draw_point(lcd::lcd.getDisplayXSize(), lcd::lcd.getDisplayYSize());
                touch_cal_mode = TOUCH_CAL_POINT_BR;
            }
        } else if (touch_cal_mode == TOUCH_CAL_POINT_BR) {
            if (touch_call_read_point()) {
                touch_cal_point_x = &touch_cal_point_trx;
                touch_cal_point_y = &touch_cal_point_try;
                touch_cal_draw_point(lcd::lcd.getDisplayXSize(), 0);
                touch_cal_mode = TOUCH_CAL_POINT_TR;
            }
        } else if (touch_cal_mode == TOUCH_CAL_POINT_TR) {
            if (touch_call_read_point()) {
                bool touch_cal_success = touch::calibrate_transform(
                    touch_cal_point_tlx,
                    touch_cal_point_tly, 
                    touch_cal_point_brx,
                    touch_cal_point_bry, 
                    touch_cal_point_trx,
                    touch_cal_point_try,
                    CONF_TOUCH_SCREEN_CALIBRATION_M
                );
                if (touch_cal_success) {
                    touch_cal_mode = TOUCH_CAL_FINISHED;
                    alert(PSTR("Save changes?"), onYes, onNo);
                } else {
                    touch_cal_mode = TOUCH_CAL_START;
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