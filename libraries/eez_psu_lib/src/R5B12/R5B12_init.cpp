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

#include <SPI.h>
#include "R5B12_pins.h"

void eez_psu_R5B12_init() {
    // set pinMode for all the pins
    pinMode(TOUCH_IRQ, INPUT);
    pinMode(TOUCH_DOUT, INPUT);

    // disable WATCHDOG
    pinMode(WATCHDOG, INPUT);

    //pinMode(WIFI_CE, OUTPUT);
    pinMode(SYNC_MASTER, INPUT);

    pinMode(FAN_PWM, OUTPUT);
    pinMode(LCD_BRIGHTNESS, OUTPUT);
    pinMode(RTC_SELECT, OUTPUT);
    pinMode(BP_SELECT, OUTPUT);
    pinMode(BP_OE, OUTPUT);
    pinMode(ETH_SELECT, OUTPUT);
    pinMode(BUZZER, OUTPUT);
    pinMode(FAN_SENSE, INPUT);
    pinMode(ISOLATOR1_EN, OUTPUT);
    pinMode(ADC1_SELECT, OUTPUT);
    pinMode(DAC1_SELECT, OUTPUT);
    pinMode(IO_EXPANDER1, OUTPUT);
    pinMode(ETH_IRQ, INPUT);
    pinMode(RTC_IRQ, INPUT);
    pinMode(CONVEND2, INPUT_PULLUP);
    pinMode(CONVEND1, INPUT_PULLUP);
    pinMode(LCD_DB8, OUTPUT);
    pinMode(LCD_DB9, OUTPUT);
    pinMode(LCD_DB10, OUTPUT);
    pinMode(LCD_DB11, OUTPUT);
    pinMode(LCD_DB12, OUTPUT);
    pinMode(LCD_DB13, OUTPUT);
    pinMode(LCD_DB14, OUTPUT);
    pinMode(LCD_DB15, OUTPUT);
    pinMode(LCD_DB7, OUTPUT);
    pinMode(LCD_DB6, OUTPUT);
    pinMode(LCD_DB5, OUTPUT);
    pinMode(LCD_DB4, OUTPUT);
    pinMode(LCD_DB3, OUTPUT);
    pinMode(LCD_DB2, OUTPUT);
    pinMode(LCD_DB1, OUTPUT);
    pinMode(LCD_DB0, OUTPUT);
    pinMode(LCD_RESET, OUTPUT);
    pinMode(LCD_CS, OUTPUT);
    pinMode(LCD_WR, OUTPUT);
    pinMode(LCD_RS, OUTPUT);
    pinMode(TOUCH_DIN, INPUT);
    pinMode(TOUCH_CS, OUTPUT);
    pinMode(TOUCH_SCLK, OUTPUT);
    pinMode(ISOLATOR2_EN, OUTPUT);
    pinMode(IO_EXPANDER2, OUTPUT);
    pinMode(DAC2_SELECT, OUTPUT);
    pinMode(ADC2_SELECT, OUTPUT);
    pinMode(EEPROM_SELECT, OUTPUT);
    pinMode(LCDSD_CS, OUTPUT);
    pinMode(TEMP_ANALOG, INPUT);
    pinMode(PWR_DIRECT, OUTPUT);
    pinMode(PWR_SSTART, OUTPUT);
    pinMode(BATT_NTC, INPUT);
    pinMode(NTC1, INPUT);
    pinMode(NTC2, INPUT);
    pinMode(EXT_TRIG, INPUT);
    pinMode(ENC_A, INPUT);
    pinMode(ENC_B, INPUT);
    pinMode(ENC_SW, INPUT_PULLUP);
    pinMode(PWD_RST, INPUT);

    //
    digitalWrite(PWR_DIRECT, LOW);
    digitalWrite(PWR_SSTART, LOW);

    // Deselect devices that share same SPI bus on the PSU post-regulator board
    digitalWrite(ADC1_SELECT, HIGH); // Deselect ADS1120
    digitalWrite(ADC2_SELECT, HIGH); // Deselect ADS1120

    digitalWrite(DAC1_SELECT, HIGH); // Deselect DAC8552
    digitalWrite(DAC2_SELECT, HIGH); // Deselect DAC8552

    digitalWrite(IO_EXPANDER1, HIGH); // Deselect MCP23S08
    digitalWrite(IO_EXPANDER2, HIGH); // Deselect MCP23S08

    digitalWrite(EEPROM_SELECT, HIGH); // Deselect EEPROM
    digitalWrite(RTC_SELECT, LOW); // Deselect PCA21125
    digitalWrite(ETH_SELECT, HIGH); // Deselect ETHERNET
    digitalWrite(BP_SELECT, HIGH); // Deselect TLC5925
    digitalWrite(BP_OE, HIGH); // Deselect TLC5925 OE

    digitalWrite(ISOLATOR1_EN, ISOLATOR_DISABLE); // Disable MISO on ch 1 isolators
    digitalWrite(ISOLATOR2_EN, ISOLATOR_DISABLE); // Disable MISO on ch 2 isolators

    digitalWrite(LCD_CS, LOW); // deselect LCD
    digitalWrite(TOUCH_CS, LOW); // deselect LCD
    analogWrite(LCD_BRIGHTNESS, 0);
    digitalWrite(LCDSD_CS, HIGH);

    // Address issue with lack of proper SPI connection on PCB r1B9
#if defined(EEZ_PSU_ARDUINO_DUE)
    pinMode(50, INPUT);
    pinMode(51, INPUT);
    pinMode(52, INPUT);
#endif

    SPI.begin(); // wake up the SPI bus
}