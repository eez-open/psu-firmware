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

#pragma once

#include <time.h> 

#include "eeprom.h"
#include "rtc.h"
#include "ioexp.h"
#include "adc.h"
#include "dac.h"

namespace eez {
namespace psu {
namespace simulator {
/// Simulation of chips.
namespace chips {

/// Select/deselect chip on SPI bus depending.
/// \param pin Pin number
/// \param state HIGH or LOW
///
/// For example, if pin is EEPROM_SELECT and state is LOW then eeprom_chip will be selected_chip
/// and all subsequent SPI.transfer's will be redirect to that chip, until eeprom_chip is deselected
/// by calling this function with arguments EEPROM_SELECT and HIGH.
void select(int pin, int state);

/// Transfers data to currently selected chip.
/// \param pin Pin number
uint8_t transfer(uint8_t data);

/// This should be called periodically by the simulator main loop.
/// For the case if some of the chips need to do something in the background.
void tick();

////////////////////////////////////////////////////////////////////////////////

/// Abstract base class for all the chips.
class Chip {
public:
    virtual void select() = 0;
    virtual uint8_t transfer(uint8_t data) = 0;
};

////////////////////////////////////////////////////////////////////////////////

/// AT25256B chip simulation.
class EepromChip : public Chip {
    enum State {
        IDLE,
        READ_ADDR_MSB,
        READ_ADDR_LSB,
        READ,
        WRITE_ADDR_MSB,
        WRITE_ADDR_LSB,
        WRITE,
        RDSR
    };

public:
    EepromChip();
    ~EepromChip();

    void select();
    uint8_t transfer(uint8_t data);

private:
    FILE *fp;

    State state;
    uint16_t address;
    uint16_t address_index;

    uint8_t read_byte();
    void write_byte(uint8_t);
};

////////////////////////////////////////////////////////////////////////////////

/// PCA21125 chip simulation.
class RtcChip : public Chip {
    enum State {
        IDLE,
        RD_CONTROL_1,
        RD_CONTROL_2,

        WR_CONTROL_1,
        WR_CONTROL_2,

        RD_SECONDS,
        RD_MINUTES,
        RD_HOURS,
        RD_DAYS,
        RD_WEEKDAYS,
        RD_MONTHS,
        RD_YEARS,

        WR_SECONDS,
        WR_MINUTES,
        WR_HOURS,
        WR_DAYS,
        WR_WEEKDAYS,
        WR_MONTHS,
        WR_YEARS
    };

public:
    RtcChip();
    ~RtcChip();

    void select();
    uint8_t transfer(uint8_t data);

private:
    FILE *fp;

    time_t offset;

    tm tm_;

    State state;

    uint8_t ctrl1;
    uint8_t ctrl2;

    void setOffset(time_t offset_);

    tm *getTime();

    uint8_t getSeconds();
    void setSeconds(uint8_t seconds);

    uint8_t getMinutes();
    void setMinutes(uint8_t minutes);

    uint8_t getHours();
    void setHours(uint8_t hours);

    uint8_t getDays();
    void setDays(uint8_t days);

    uint8_t getWeekdays();
    void setWeekdays(uint8_t weekdays);

    uint8_t getMonths();
    void setMonths(uint8_t months);

    uint8_t getYears();
    void setYears(uint8_t years);
};

////////////////////////////////////////////////////////////////////////////////

/// TLC5925 chip simulation.
class BPChip : public Chip {
    enum State {
        IDLE,
        READ_MSB
    };

public:
    BPChip();

    void select();
    uint8_t transfer(uint8_t data);

    uint16_t getValue() { return value; }

private:
    State state;
    uint16_t value;
};

////////////////////////////////////////////////////////////////////////////////

/// MCP23S08 chip simulation.
class IOExpanderChip : public Chip {
    friend class AnalogDigitalConverterChip;

    enum State {
        IDLE,
        READ_REGISTER_INDEX,
        READ_REGISTER_VALUE,
        WRITE_REGISTER_INDEX,
        WRITE_REGISTER_VALUE
    };

public:
    IOExpanderChip();

    static bool getPwrgood(int pin);
    static void setPwrgood(int pin, bool on);

    static bool getRPol(int pin);
    static void setRPol(int pin, bool on);

	void select();
    uint8_t transfer(uint8_t data);

private:
    State state;
    uint8_t register_index;
    uint8_t register_values[IOExpander::NUM_REGISTERS];
    bool pwrgood;
	bool rpol;
    bool cc;
    bool cv;
};

////////////////////////////////////////////////////////////////////////////////

/// ADS1120 chip simulation.
class AnalogDigitalConverterChip : public Chip {
    friend class DigitalAnalogConverterChip;

    enum State {
        IDLE,
        READ_REG,
        WRITE_REG,
        WR1S0,
        RDATA_MSB,
        RDATA_LSB
    };

public:
    AnalogDigitalConverterChip(IOExpanderChip &ioexp_chip_, int convend_pin_);

    void tick();

    void select();
    uint8_t transfer(uint8_t data);

private:
    IOExpanderChip &ioexp_chip;
    int convend_pin;
    State state;
    uint8_t register_index;
    uint8_t register_values[4];
    uint16_t u_mon;
    uint16_t i_mon;
    uint16_t u_set;
    uint16_t i_set;
    int tick_counter;
    bool start;

    uint16_t getValue();
    void setDacValue(uint8_t data_buffer, uint16_t value);
    void updateValues();
};

////////////////////////////////////////////////////////////////////////////////

/// DAC8552 chip simulation.
class DigitalAnalogConverterChip : public Chip {
    enum State {
        IDLE,
        DATA_BUFFER_MSB,
        DATA_BUFFER_LSB,
    };

public:
    DigitalAnalogConverterChip(AnalogDigitalConverterChip &adc_chip_);

    void select();
    uint8_t transfer(uint8_t data);

private:
    AnalogDigitalConverterChip &adc_chip;
    State state;
    uint8_t data_buffer;
    uint16_t value;
};

////////////////////////////////////////////////////////////////////////////////

extern BPChip bp_chip;

////////////////////////////////////////////////////////////////////////////////

}
}
}
} // namespace eez::psu::simulator::hw
