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
|1024   |  32|[Device configuration](#device)           |
|2048   | 121|CH1 [calibration parameters](#calibration)|
|2560   | 121|CH2 [calibration parameters](#calibration)|
|4096   | 164|[Profile](#profile) 0                     |
|5120   | 164|[Profile](#profile) 1                     |
|6144   | 164|[Profile](#profile) 2                     |
|7168   | 164|[Profile](#profile) 3                     |
|8192   | 164|[Profile](#profile) 4                     |
|9216   | 164|[Profile](#profile) 5                     |
|10240  | 164|[Profile](#profile) 6                     |
|11264  | 164|[Profile](#profile) 7                     |
|12288  | 164|[Profile](#profile) 8                     |
|13312  | 164|[Profile](#profile) 9                     |
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
|6     |17  |string                   |Calibration password         |
|23    |2   |[bitarray](#device-flags)|[Device Flags](#device-flags)|
|25    |1   |int                      |Year                         |
|26    |1   |int                      |Month                        |
|27    |1   |int                      |Day                          |
|28    |1   |int                      |Hour                         |
|29    |1   |int                      |Minute                       |
|30    |1   |int                      |Second                       |
|31    |1   |int                      |Auto profile location        |

#### <a name="device-flags">Device flags</a>

|Bit|Description        |
|---|-------------------|
|0  |Beep enabled       |
|1  |Date set           |
|2  |Time set           |
|3  |Auto recall profile|
|4  |Reserved           |
|5  |Reserved           |
|6  |Reserved           |
|7  |Reserved           |
|8  |Reserved           |
|9  |Reserved           |
|10 |Reserved           |
|11 |Reserved           |
|12 |Reserved           |
|13 |Reserved           |
|14 |Reserved           |
|15 |Reserved           |

## <a name="calibration">Calibration parameters</a>

|Offset|Size|Type                   |Description                  |
|------|----|-----------------------|-----------------------------|
|0     |6   |[struct](#block-header)|[Block header](#block-header)|
|6     |1   |[bitarray](#cal-flags) |[Flags](#cal-flags)          |
|7     |36  |[struct](#cal-points)  |Voltage [points](#cal-points)|
|43    |36  |[struct](#cal-points)  |Current [points](#cal-points)|
|79    |8   |string                 |Date                         |
|88    |33  |string                 |Remark                       |

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
|6     |1   |boolean                |Is valid?                      |
|7     |33  |string                 |Name                           |
|40    |1   |boolean                |Is power up?                   |
|41    |34  |[struct](#ch-params)   |CH1 [parameters](#ch-params)   |
|75    |34  |[struct](#ch-params)   |CH2 [parameters](#ch-params)   |
|109   |55  |[struct](#otp-conf)[5] |[OTP configurations](#otp-conf)|

#### <a name="ch-params">Channel parameters</a>

|Offset|Size|Type                 |Description               |
|------|----|---------------------|--------------------------|
|0     |2   |[bitarray](#ch-flags)|[Channel Flags](#ch-flags)|
|2     |4   |float                |Voltage set               |
|6     |4   |float                |Voltage step              |
|10    |4   |float                |Current set               |
|14    |4   |float                |Current step              |
|18    |4   |float                |OVP delay                 |
|22    |4   |float                |OCP delay                 |
|26    |4   |float                |OPP delay                 |
|30    |4   |float                |OPP level                 |

#### <a name="ch-flags">Channel flags</a>

|Bit|Description        |
|---|-------------------|
|0  |Output enabled     |
|1  |Sense enabled      |
|2  |OVP state          |
|3  |OCP state          |
|3  |OPP state          |
|5  |Calibration enabled|
|6  |Reserved           |
|7  |Reserved           |
|8  |Reserved           |
|9  |Reserved           |
|10 |Reserved           |
|11 |Reserved           |
|12 |Reserved           |
|13 |Reserved           |
|14 |Reserved           |
|15 |Reserved           |

#### <a name="otp-conf">OTP configuration</a>

|Offset|Size|Type              |Description                     |
|------|----|------------------|--------------------------------|
|0     |2   |[enum](#temp-sens)|[Temperature sensor](#temp-sens)|
|2     |4   |float             |OTP delay                       |
|6     |4   |float             |OTP level                       |
|10    |1   |boolean           |OTP state                       |


##### <a name="temp-sens">Temperature sensor</a>

|Value|Name|
|-----|----|
|0    |MAIN|
|1    |S1  |
|2    |S2  |
|3    |BAT1|
|4    |BAT2|

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

bool init();
bool test();

extern TestResult test_result;

void read(uint8_t *buffer, uint16_t buffer_size, uint16_t address);
bool write(const uint8_t *buffer, uint16_t buffer_size, uint16_t address);

}
}
} // namespace eez::psu::eeprom
