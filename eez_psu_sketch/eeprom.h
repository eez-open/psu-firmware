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

/*! \page eeprom_map EEPROM map

# Overview

|Address|Size|Description                               |
|-------|----|------------------------------------------|
|0      |  64|Not used                                  |
|64     |  24|[Total ON-time counter](#ontime-counter)  |
|128    |  24|[CH1 ON-time counter](#ontime-counter)    |
|192    |  24|[CH2 ON-time counter](#ontime-counter)    |
|1024   |  64|[Device configuration](#device)           |
|1536   | 128|[Device configuration 2](#device2)           |
|2048   | 144|CH1 [calibration parameters](#calibration)|
|2560   | 144|CH2 [calibration parameters](#calibration)|
|5120   | 232|[Profile](#profile) 0                     |
|6144   | 232|[Profile](#profile) 1                     |
|7168   | 232|[Profile](#profile) 2                     |
|8192   | 232|[Profile](#profile) 3                     |
|9216   | 232|[Profile](#profile) 4                     |
|10240  | 232|[Profile](#profile) 5                     |
|11264  | 232|[Profile](#profile) 6                     |
|12288  | 232|[Profile](#profile) 7                     |
|13312  | 232|[Profile](#profile) 8                     |
|14336  | 232|[Profile](#profile) 9                     |
|16384  | 610|[Event Queue](#event-queue)               |

## <a name="ontime-counter">ON-time counter</a>

|Offset|Size|Type                     |Description                  |
|------|----|-------------------------|-----------------------------|
|0     |4   |int                      |1st magic number             |
|4     |4   |int                      |1st counter                  |
|8     |4   |int                      |1st counter (copy)           |
|12    |4   |int                      |2nd magic number             |
|16    |4   |int                      |2nd counter                  |
|20    |4   |int                      |2bd counter (copy)           |

## <a name="device">Device configuration</a>

|Offset|Size|Type                     |Description                  |
|------|----|-------------------------|-----------------------------|
|0     |6   |[struct](#block-header)  |[Block header](#block-header)|
|8     |8   |string                   |Serial number                |
|16    |17  |string                   |Calibration password         |
|36    |4   |[bitarray](#device-flags)|[Device Flags](#device-flags)|
|40    |1   |int                      |Year                         |
|41    |1   |int                      |Month                        |
|42    |1   |int                      |Day                          |
|43    |1   |int                      |Hour                         |
|44    |1   |int                      |Minute                       |
|45    |1   |int                      |Second                       |
|46    |2   |int                      |Time zone                    |
|48    |1   |int                      |Auto profile location        |
|49    |1   |int                      |Touch screen cal. orientation|
|50    |2   |int                      |Touch screen cal. TLX        |
|52    |2   |int                      |Touch screen cal. TLY        |
|54    |2   |int                      |Touch screen cal. BRX        |
|56    |2   |int                      |Touch screen cal. BRY        |
|58    |2   |int                      |Touch screen cal. TRX        |
|60    |2   |int                      |Touch screen cal. TRY        |

## <a name="device">Device configuration 2</a>

|Offset|Size|Type                     |Description                  |
|------|----|-------------------------|-----------------------------|
|0     |6   |[struct](#block-header)  |[Block header](#block-header)|
|8     |17  |string                   |System password              |

#### <a name="device-flags">Device flags</a>

|Bit|Description         |
|---|--------------------|
|0  |Sound enabled       |
|1  |Date set            |
|2  |Time set            |
|3  |Auto recall profile |
|4  |DST                 |
|5  |Channel display mode|
|6  |Channel display mode|
|7  |Channel display mode|
|8  |Ethernet enabled    |
|9  |Switch off all outputs when protection tripped |
|10 |Shutdown when protection tripped               |
|11 |Force disabling of all outputs on power up     |
|12 |Click sound enabled |

## <a name="calibration">Calibration parameters</a>

|Offset|Size|Type                   |Description                  |
|------|----|-----------------------|-----------------------------|
|0     |8   |[struct](#block-header)|[Block header](#block-header)|
|8     |4   |[bitarray](#cal-flags) |[Flags](#cal-flags)          |
|12    |44  |[struct](#cal-points)  |Voltage [points](#cal-points)|
|56    |44  |[struct](#cal-points)  |Current [points](#cal-points)|
|100   |9   |string                 |Date                         |
|109   |33  |string                 |Remark                       |

#### <a name="cal-flags">Calibration flags</a>

|Bit|Description        |
|---|-------------------|
|0  |Voltage calibrated?|
|1  |Current calibrated?|

#### <a name="cal-points">Value points</a>

|Offset|Size|Type                |Description            |
|------|----|--------------------|-----------------------|
|0     |12  |[struct](#cal-point)|Min [point](#cal-point)|
|12    |12  |[struct](#cal-point)|Mid [point](#cal-point)|
|24    |12  |[struct](#cal-point)|Max [point](#cal-point)|
|36    |4   |[struct](#cal-point)|Min. set value possible|
|40    |4   |[struct](#cal-point)|Max. set value possible|

#### <a name="cal-point">Value point</a>

|Offset|Size|Type |Description|
|------|----|-----|-----------|
|0     |4   |float|DAC value  |
|4     |4   |float|Real value |
|8     |4   |float|ADC value  |

## <a name="profile">Profile</a>

|Offset|Size|Type                   |Description                    |
|------|----|-----------------------|-------------------------------|
|0     |6   |[struct](#block-header)|[Block header](#block-header)  |
|8     |4   |[bitarray](#prof-flags)|[Flags](#prof-flags)           |
|12    |33  |string                 |Name                           |
|48    |52  |[struct](#ch-params)   |CH1 [parameters](#ch-params)   |
|100   |52  |[struct](#ch-params)   |CH2 [parameters](#ch-params)   |
|152   |80  |[struct](#otp-conf)[5] |[OTP configurations](#otp-conf)|

#### <a name="prof-flags">Profile flags</a>

|Bit|Description        |
|---|-------------------|
|0  |Is valid?          |
|1  |Power is up?       |
|2  |Channels coupling: 0 - None, 1 - Parallel, 2 - Series, 3 - Tracking mode |

#### <a name="ch-params">Channel parameters</a>

|Offset|Size|Type                 |Description               |
|------|----|---------------------|--------------------------|
|0     |4   |[bitarray](#ch-flags)|[Channel Flags](#ch-flags)|
|4     |4   |float                |U set                     |
|8     |4   |float                |U step                    |
|12    |4   |float                |U limit                   |
|16    |4   |float                |OVP delay                 |
|20    |4   |float                |OVP level                 |
|24    |4   |float                |I set                     |
|28    |4   |float                |I step                    |
|32    |4   |float                |I limit                   |
|36    |4   |float                |OCP delay                 |
|40    |4   |float                |P step                    |
|44    |4   |float                |OVP delay                 |
|48    |4   |float                |OVP level                 |

#### <a name="ch-flags">Channel flags</a>

|Bit|Description        |
|---|-------------------|
|0  |Output enabled     |
|1  |Sense enabled      |
|2  |OVP enabled        |
|3  |OCP enabled        |
|3  |OPP enabled        |
|5  |Calibration enabled|
|6  |RPROG enabled      |
|7  |Reserved           |
|8  |LRIPPLE enabled    |
|9  |Params. valid      |

#### <a name="otp-conf">OTP configuration</a>

|Offset|Size|Type              |Description                     |
|------|----|------------------|--------------------------------|
|0     |4   |[enum](#temp-sens)|[Temperature sensor](#temp-sens)|
|4     |4   |float             |OTP delay                       |
|8     |4   |float             |OTP level                       |
|12    |1   |boolean           |OTP state                       |


##### <a name="temp-sens">Temperature sensor</a>

|Value|Name|
|-----|----|
|0    |AUX |
|1    |CH1 |
|2    |CH2 |
|3    |reserved|
|4    |reserved|

## <a name="block-header">Block header</a>

|Offset|Size|Type|Description|
|------|----|----|-----------|
|0     |4   |int |Checksum   |
|4     |2   |int |Version    |

## <a name="event-queue">Event queue</a>

|Offset|Size |Type                     |Description                  |
|------|-----|-------------------------|-----------------------------|
|0     |4    |int                      |Magic number                 |
|4     |2    |int                      |Version                      |
|6     |2    |int                      |Queue head                   |
|8     |2    |int                      |Queue size                   |
|10    |2    |int                      |Last error event index       |
|16    |1600 |[struct](#event)         |Max. 100 events              |

## <a name="event">Event</a>

|Offset|Size|Type                     |Description                  |
|------|----|-------------------------|-----------------------------|
|0     |4   |datetime                 |Event date and time          |
|4     |2   |int                      |Event ID                     |

*/


namespace eez {
namespace psu {
namespace eeprom {

static const uint16_t EEPROM_TEST_ADDRESS = 0;
static const uint16_t EEPROM_TEST_BUFFER_SIZE = 64;

// opcodes
static const uint8_t WREN = 6;
static const uint8_t WRDI = 4;
static const uint8_t RDSR = 5;
static const uint8_t WRSR = 1;
static const uint8_t READ = 3;
static const uint8_t WRITE = 2;

static const uint16_t EEPROM_ONTIME_START_ADDRESS = 64;
static const uint16_t EEPROM_ONTIME_SIZE = 64;

static const uint16_t EEPROM_START_ADDRESS = 1024;

static const uint16_t EEPROM_EVENT_QUEUE_START_ADDRESS = 16384;

void init();
bool test();

extern TestResult g_testResult;

void read(uint8_t *buffer, uint16_t buffer_size, uint16_t address);
bool write(const uint8_t *buffer, uint16_t buffer_size, uint16_t address);

}
}
} // namespace eez::psu::eeprom
