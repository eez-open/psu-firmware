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

#ifndef EEZ_PSU_R2B6_PINS_H
#define EEZ_PSU_R2B6_PINS_H

static const uint8_t TOUCH_IRQ = 2;
static const uint8_t TOUCH_DOUT = 3;
static const uint8_t WATCHDOG = 5;
static const uint8_t FAN_PWM = 6;
static const uint8_t LCD_BRIGHTNESS = 7;
static const uint8_t RTC_SELECT = 8;
static const uint8_t BP_SELECT = 9;
static const uint8_t BP_OE = 10;
static const uint8_t ETH_SELECT = 11;
static const uint8_t BUZZER = 12;
static const uint8_t ISOLATOR1_EN = 14;
static const uint8_t ADC1_SELECT = 15;
static const uint8_t DAC1_SELECT = 16;
static const uint8_t IO_EXPANDER1 = 17;
static const uint8_t ETH_IRQ = 18;
static const uint8_t RTC_IRQ = 19;
static const uint8_t CONVEND2 = 20;
static const uint8_t CONVEND1 = 21;
static const uint8_t LCD_DB8 = 22;
static const uint8_t LCD_DB9 = 23;
static const uint8_t LCD_DB10 = 24;
static const uint8_t LCD_DB11 = 25;
static const uint8_t LCD_DB12 = 26;
static const uint8_t LCD_DB13 = 27;
static const uint8_t LCD_DB14 = 28;
static const uint8_t LCD_DB15 = 29;
static const uint8_t LCD_DB7 = 30;
static const uint8_t LCD_DB6 = 31;
static const uint8_t LCD_DB5 = 32;
static const uint8_t LCD_DB4 = 33;
static const uint8_t LCD_DB3 = 34;
static const uint8_t LCD_DB2 = 35;
static const uint8_t LCD_DB1 = 36;
static const uint8_t LCD_DB0 = 37;
static const uint8_t LCD_RESET = 38;
static const uint8_t LCD_CS = 39;
static const uint8_t LCD_WR = 40;
static const uint8_t LCD_RS = 41;
static const uint8_t TOUCH_DIN = 42;
static const uint8_t TOUCH_CS = 43;
static const uint8_t TOUCH_SCLK = 44;
static const uint8_t ISOLATOR2_EN = 45;
static const uint8_t IO_EXPANDER2 = 46;
static const uint8_t DAC2_SELECT = 47;
static const uint8_t ADC2_SELECT = 48;
static const uint8_t EEPROM_SELECT = 49;
static const uint8_t LCDSD_CS = 53;
static const uint8_t TEMP_ANALOG = 54;
static const uint8_t PWR_DIRECT = 55;
static const uint8_t PWR_SSTART = 56;
static const uint8_t BATT_NTC = 58;
static const uint8_t NTC1 = 59;
static const uint8_t NTC2 = 60;
static const uint8_t EXT_TRIG = 61;
static const uint8_t PWD_RST = 67;

#endif // EEZ_PSU_R2B6_PINS_H