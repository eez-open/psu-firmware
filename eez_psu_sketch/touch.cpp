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

#include "touch.h"
#include "touch_filter.h"

#define CONF_GUI_TOUCH_READ_FREQ_MS 10

namespace eez {
namespace psu {
namespace gui {
namespace touch {

////////////////////////////////////////////////////////////////////////////////

bool touch_is_pressed = false;
int touch_x = -1;
int touch_y = -1;

#ifdef EEZ_PSU_ARDUINO

#define cbi(reg, bitmask) *reg &= ~bitmask
#define sbi(reg, bitmask) *reg |= bitmask
#define rbi(reg, bitmask) ((*reg) & bitmask)

#define pulse_high(reg, bitmask) sbi(reg, bitmask); cbi(reg, bitmask);
#define pulse_low(reg, bitmask) cbi(reg, bitmask); sbi(reg, bitmask);

#if defined(__AVR__)
    #define regtype volatile uint8_t
    #define regsize uint8_t
#else
    #define regtype volatile uint32_t
    #define regsize uint32_t
#endif

regtype *P_CLK, *P_CS, *P_DIN, *P_DOUT, *P_IRQ;
regsize B_CLK, B_CS, B_DIN, B_DOUT, B_IRQ;

#if defined(__AVR__)
    void touch_WriteData(byte data) {
        byte temp = data;
        cbi(P_CLK, B_CLK);
        for (byte count = 0; count < 8; count++) {
            if (temp & 0x80)
                sbi(P_DIN, B_DIN);
            else
                cbi(P_DIN, B_DIN);
            temp = temp << 1; 
            cbi(P_CLK, B_CLK);                
            sbi(P_CLK, B_CLK);
        }
    }

    word touch_ReadData() {
        word data = 0;
        for (byte count = 0; count < 12; count++) {
            data <<= 1;
            sbi(P_CLK, B_CLK);
            cbi(P_CLK, B_CLK);                
            if (rbi(P_DOUT, B_DOUT))
                data++;
        }
        return(data);
    }
#else
    void touch_WriteData(byte data) {
        byte temp = data;
        cbi(P_CLK, B_CLK);
        for (byte count = 0; count < 8; count++) {
            if(temp & 0x80)
                digitalWrite(TOUCH_DIN, HIGH);
            else
                digitalWrite(TOUCH_DIN, LOW);
            temp = temp << 1; 
            digitalWrite(TOUCH_SCLK, LOW);
            digitalWrite(TOUCH_SCLK, HIGH);
        }
    }

    word touch_ReadData() {
        word data = 0;
        for (byte count = 0; count < 12; count++) {
            data <<= 1;
            digitalWrite(TOUCH_SCLK, HIGH);
            digitalWrite(TOUCH_SCLK, LOW);
            if (digitalRead(TOUCH_DOUT))
                data++;
        }
        return(data);
    }
#endif

void touch_init() {
	P_CLK	= portOutputRegister(digitalPinToPort(TOUCH_SCLK));
	B_CLK	= digitalPinToBitMask(TOUCH_SCLK);
	P_CS	= portOutputRegister(digitalPinToPort(TOUCH_CS));
	B_CS	= digitalPinToBitMask(TOUCH_CS);
	P_DIN	= portOutputRegister(digitalPinToPort(TOUCH_DIN));
	B_DIN	= digitalPinToBitMask(TOUCH_DIN);
	P_DOUT	= portInputRegister(digitalPinToPort(TOUCH_DOUT));
	B_DOUT	= digitalPinToBitMask(TOUCH_DOUT);
	P_IRQ	= portInputRegister(digitalPinToPort(TOUCH_IRQ));
	B_IRQ	= digitalPinToBitMask(TOUCH_IRQ);

	pinMode(TOUCH_SCLK,  OUTPUT);
    pinMode(TOUCH_CS,   OUTPUT);
    pinMode(TOUCH_DIN,  OUTPUT);
    pinMode(TOUCH_DOUT, INPUT);
    pinMode(TOUCH_IRQ,  OUTPUT);

	sbi(P_CS, B_CS);
	sbi(P_CLK, B_CLK);
	sbi(P_DIN, B_DIN);
	sbi(P_IRQ, B_IRQ);

    // Command for reading X position. This also sets bit 0 (PD0) and bit 1 (PD1) to zero,
    // which enables PENIRQ (see table 8. in XPT2046 datasheet).
    cbi(P_CS, B_CS);
    touch_WriteData(0x90);
    sbi(P_CS, B_CS);
}

void touch_read() {
	cbi(P_CS, B_CS);                    

    pinMode(TOUCH_IRQ,  INPUT);

    // read pressure
    touch_is_pressed = !rbi(P_IRQ, B_IRQ); 
    if (touch_is_pressed) {
        // read X
        touch_WriteData(0x90);        
        pulse_high(P_CLK, B_CLK);
        touch_x = touch_ReadData();

        touch_is_pressed = !rbi(P_IRQ, B_IRQ);
        if (touch_is_pressed) {
            // read Y
            touch_WriteData(0xD0);      
            pulse_high(P_CLK, B_CLK);
            touch_y = touch_ReadData();

            if (touch_x == -1) {
                touch_is_pressed = false;
                touch_y = -1;
            } else if (touch_y == -1) {
                touch_is_pressed = false;
                touch_x = -1;
            } else {
                touch_x = 4095 - touch_x;
            }
        } else {
            touch_x = -1;
            touch_y = -1;
        }
    } else {
        touch_x = -1;
        touch_y = -1;
    }
    pinMode(TOUCH_IRQ, OUTPUT);
    
	sbi(P_CS, B_CS);
}

#else

bool g_nextIsPressed = false;
int g_nextX = -1;
int g_nextY = -1;

void touch_init() {
}

void touch_read() {
    touch_is_pressed = g_nextIsPressed;
    touch_x = g_nextX;
    touch_y = g_nextY;
}

void touch_write(bool is_pressed, int x, int y) {
    g_nextIsPressed = is_pressed;
    g_nextX = x;
    g_nextY = y;
}

#endif

////////////////////////////////////////////////////////////////////////////////

EventType g_eventType = TOUCH_NONE;
int g_x = -1;
int g_y = -1;

uint32_t last_tick_usec = 0;

void init() {
    touch_init();
    calibration::init();
}

void tick(uint32_t tick_usec) {
    if (last_tick_usec == 0 || tick_usec - last_tick_usec > CONF_GUI_TOUCH_READ_FREQ_MS * 1000UL) {
    	touch_read();

        touch_is_pressed = filter(touch_is_pressed, touch_x, touch_y);
        if (touch_is_pressed) {
            transform(touch_x, touch_y);
        }

        last_tick_usec = tick_usec;
    }

    if (touch_is_pressed) {
        if (touch_x != -1 && touch_y != -1) {
            g_x = touch_x;
            g_y = touch_y;

            if (g_eventType == TOUCH_NONE || g_eventType == TOUCH_UP) {
                g_eventType = TOUCH_DOWN;
            } else {
                if (g_eventType == TOUCH_DOWN) {
                    g_eventType = TOUCH_MOVE;
                }
            }
            return;
        }
    }

    if (g_eventType == TOUCH_DOWN || g_eventType == TOUCH_MOVE) {
        g_eventType = TOUCH_UP;
    } else if (g_eventType == TOUCH_UP) {
        g_eventType = TOUCH_NONE;
        g_x = -1;
        g_y = -1;
    }
}

}
}
}
} // namespace eez::psu::ui::touch

#endif
