/*
 * EEZ Middleware
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

#include "mw_gui_touch.h"

namespace eez {
namespace mw {
namespace gui {
namespace touch {

#define cbi(reg, bitmask) *reg &= ~bitmask
#define sbi(reg, bitmask) *reg |= bitmask
#define rbi(reg, bitmask) ((*reg) & bitmask)

#define pulse_high(reg, bitmask) sbi(reg, bitmask); cbi(reg, bitmask);
#define pulse_low(reg, bitmask) cbi(reg, bitmask); sbi(reg, bitmask);

volatile uint32_t *P_CLK, *P_CS, *P_DIN, *P_DOUT, *P_IRQ;
uint32_t B_CLK, B_CS, B_DIN, B_DOUT, B_IRQ;

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

void init() {
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

void read(bool &isPressed, int &x, int &y) {
	cbi(P_CS, B_CS);

    pinMode(TOUCH_IRQ,  INPUT);

    // read pressure
    isPressed = !rbi(P_IRQ, B_IRQ);
    if (isPressed) {
        // read X
        touch_WriteData(0x90);
        pulse_high(P_CLK, B_CLK);
        x = touch_ReadData();

        isPressed = !rbi(P_IRQ, B_IRQ);
        if (isPressed) {
            // read Y
            touch_WriteData(0xD0);
            pulse_high(P_CLK, B_CLK);
            y = touch_ReadData();

            if (x == -1) {
                isPressed = false;
                y = -1;
            } else if (y == -1) {
                isPressed = false;
                x = -1;
            } else {
                x = 4095 - x;
            }
        } else {
            x = -1;
            y = -1;
        }
    } else {
        x = -1;
        y = -1;
    }
    pinMode(TOUCH_IRQ, OUTPUT);

	sbi(P_CS, B_CS);
}

}
}
}
} // namespace eez::mw::gui::touch

#endif
