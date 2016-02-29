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
 
#include <SPI.h>
#include "eez_psu.h"

SPISettings MCP23S08_SPI(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE0);
SPISettings DAC8552_SPI(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE1);
SPISettings ADS1120_SPI(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE1);
SPISettings TLC5925_SPI(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE0);
SPISettings PCA21125_SPI(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE0);
SPISettings AT25256B_SPI(SPI_CLOCK_DIV4, MSBFIRST, SPI_MODE0);
#if defined(_VARIANT_ARDUINO_DUE_X_)
SPISettings ENC28J60_SPI(10, MSBFIRST, SPI_MODE0);
#else
SPISettings ENC28J60_SPI(SPI_CLOCK_DIV2, MSBFIRST, SPI_MODE0);
#endif

void eez_psu_init() {
    pinMode(PWR_DIRECT, OUTPUT);
    digitalWrite(PWR_DIRECT, LOW);

    pinMode(PWR_SSTART, OUTPUT);
    digitalWrite(PWR_SSTART, LOW);

    pinMode(SS, OUTPUT);

    // Deselect devices that share same SPI bus on the PSU post-regulator board
    pinMode(ADC1_SELECT, OUTPUT);
    digitalWrite(ADC1_SELECT, HIGH); // Deselect ADS1120
    pinMode(ADC2_SELECT, OUTPUT);
    digitalWrite(ADC2_SELECT, HIGH); // Deselect ADS1120

    pinMode(DAC1_SELECT, OUTPUT);
    digitalWrite(DAC1_SELECT, HIGH); // Deselect DAC8552
    pinMode(DAC2_SELECT, OUTPUT);
    digitalWrite(DAC2_SELECT, HIGH); // Deselect DAC8552

    pinMode(IO_EXPANDER1, OUTPUT);
    digitalWrite(IO_EXPANDER1, HIGH); // Deselect MCP23S08
    pinMode(IO_EXPANDER2, OUTPUT);
    digitalWrite(IO_EXPANDER2, HIGH); // Deselect MCP23S08

    pinMode(EEPROM_SELECT, OUTPUT);
    digitalWrite(EEPROM_SELECT, HIGH); // Deselect EEPROM

    pinMode(RTC_SELECT, OUTPUT);
    digitalWrite(RTC_SELECT, LOW); // Deselect PCA21125

    pinMode(ETH_SELECT, OUTPUT);
    digitalWrite(ETH_SELECT, HIGH); // Deselect ENC28J60

    pinMode(BP_SELECT, OUTPUT);
    digitalWrite(BP_SELECT, HIGH); // Deselect TLC5925

    pinMode(BP_OE, OUTPUT);
    digitalWrite(BP_OE, HIGH); // Deselect TLC5925 OE

    pinMode(ISOLATOR1_EN, OUTPUT);
    digitalWrite(ISOLATOR1_EN, HIGH); // Disable MISO on ch 1 isolators
    pinMode(ISOLATOR2_EN, OUTPUT);
    digitalWrite(ISOLATOR2_EN, HIGH); // Disable MISO on ch 2 isolators

    pinMode(CONVEND1, INPUT_PULLUP); // ADC DRDY for ch1
    pinMode(CONVEND2, INPUT_PULLUP); // ADC DRDY for ch2

#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
    pinMode(LCD_SELECT, OUTPUT);  
    digitalWrite(LCD_SELECT, LOW); // deselect LCD
#else
    pinMode(LCD_CS, OUTPUT);
    digitalWrite(LCD_CS, LOW); // deselect LCD

    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, LOW); // deselect LCD
#endif

    pinMode(LCD_BRIGHTNESS, OUTPUT); // Arduino Pin 11 is PB7 (pin 12) assigned to LCD_BRIGHTNESS
    analogWrite(LCD_BRIGHTNESS, 200); // (value from 0 to 255, 0=dark)

    pinMode(LED_CC1, OUTPUT);
    digitalWrite(LED_CC1, LOW);

    pinMode(LED_CV1, OUTPUT);
    digitalWrite(LED_CV1, LOW);

    pinMode(LED_CC2, OUTPUT);
    digitalWrite(LED_CC2, LOW);

    pinMode(LED_CV2, OUTPUT);
    digitalWrite(LED_CV2, LOW);
    
    pinMode(LCDSD_CS, OUTPUT);
    digitalWrite(LCDSD_CS, HIGH);
    
    // Address issue with lack of proper SPI connection on PCB r1B9
#if defined(_VARIANT_ARDUINO_DUE_X_)
    pinMode(50, INPUT);
    pinMode(51, INPUT);
    pinMode(52, INPUT);
#endif

    SPI.begin(); // wake up the SPI bus
}    