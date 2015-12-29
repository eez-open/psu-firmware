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
 
#ifndef EEZ_PSU_H
#define EEZ_PSU_H

////////////////////////////////////////////////////////////////////////////////

#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)

// MCU with ATmega32U4 and 128x64 Monochromatic LCD

static const uint8_t SCLK           = 15;
static const uint8_t DAC1_SELECT    = 8;
static const uint8_t LCD_CONTROL    = 9;
static const uint8_t BP_OE          = 10;
static const uint8_t LCD_BRIGHTNESS = 11;
static const uint8_t DAC2_SELECT    = 5;
static const uint8_t ADC2_SELECT    = 13;
static const uint8_t CONVEND1       = 3;
static const uint8_t CONVEND2       = 2;
static const uint8_t ETH_INT        = 0;
static const uint8_t ETH_SELECT     = 1;
static const uint8_t ISOLATOR1_EN   = 4;
static const uint8_t ISOLATOR2_EN   = 17;
static const uint8_t ADC1_SELECT    = 30;
static const uint8_t IO_EXPANDER1   = 12;
static const uint8_t IO_EXPANDER2   = 6;
static const uint8_t EEPROM_SELECT  = 31;
static const uint8_t RTC_INT        = 7;
static const uint8_t KEYPAD_ANALOG  = 23;
static const uint8_t TEMP_ANALOG    = 22;
static const uint8_t BP_SELECT      = 21;
static const uint8_t RTC_SELECT     = 20;
static const uint8_t BUZZER         = 19;
static const uint8_t LCD_SELECT     = 18;

#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

// MSU Shield with Arduino Mega and Touchscreen LCD

static const uint8_t TOUCH_IRQ = 2;
static const uint8_t TOUCH_DOUT = 3;
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
static const uint8_t TOUCH_DIN = 41;
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
static const uint8_t PWD_RST = 67;
static const uint8_t LED_CC1 = 4;
static const uint8_t LED_CV1 = 5;
static const uint8_t LED_CC2 = 68;
static const uint8_t LED_CV2 = 69;

#elif defined (_VARIANT_ARDUINO_DUE_X_)

// MSU Shield with Arduino Due and Touchscreen LCD

static const uint8_t TOUCH_IRQ = 2;
static const uint8_t TOUCH_DOUT = 3;
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
static const uint8_t TOUCH_DIN = 41;
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
static const uint8_t PWD_RST = 67;
static const uint8_t LED_CC1 = 4;
static const uint8_t LED_CV1 = 5;
static const uint8_t LED_CC2 = 68;
static const uint8_t LED_CV2 = 69;

#else

static const uint8_t TOUCH_IRQ = 2;
static const uint8_t TOUCH_DOUT = 3;
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
static const uint8_t TOUCH_DIN = 41;
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
static const uint8_t PWD_RST = 67;
static const uint8_t LED_CC1 = 4;
static const uint8_t LED_CV1 = 5;
static const uint8_t LED_CC2 = 68;
static const uint8_t LED_CV2 = 69;

#endif

////////////////////////////////////////////////////////////////////////////////

extern void eez_psu_init();

////////////////////////////////////////////////////////////////////////////////
// IO EXPANDER - MCP23S08

/*
MCP23S08

PSU 0-50V/3A I/O expander pinout:

Pin 0  In, ADC interrupt/DRDY
Pin 1  Out, DP enable (active low)
Pin 2  In, CV_ACTIVE (active low)
Pin 3  In, Temp sensor (V/F)
Pin 4  --, not in use
Pin 5  In, CC_ACTIVE (active low)
Pin 6  In, PWRGOOD
Pin 7  Out, OUTPUT_ENABLE
*/

extern SPISettings MCP23S08_SPI;

////////////////////////////////////////////////////////////////////////////////
// DAC - DAC8552

extern SPISettings DAC8552_SPI;

////////////////////////////////////////////////////////////////////////////////
// ADC - ADS1120

extern SPISettings ADS1120_SPI;

////////////////////////////////////////////////////////////////////////////////
// BP - TLC5925

extern SPISettings TLC5925_SPI;

////////////////////////////////////////////////////////////////////////////////
// RTC - PCA21125

extern SPISettings PCA21125_SPI;

////////////////////////////////////////////////////////////////////////////////
// EEPROM - AT25256B

extern SPISettings AT25256B_SPI;

////////////////////////////////////////////////////////////////////////////////
// ETHERNET - ENC28J60

extern SPISettings ENC28J60_SPI;

////////////////////////////////////////////////////////////////////////////////

#endif // EEZ_PSU_H