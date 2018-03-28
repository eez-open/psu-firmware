/*
* EEZ PSU Firmware
* Copyright (C) 2018-present, Envox d.o.o.
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
#include "scpi_psu.h"

#if OPTION_SD_CARD

#include "sd_card.h"
#include "datetime.h"
#include "channel_dispatcher.h"
#include "dlog.h"

namespace eez {
namespace psu {
namespace dlog {

bool g_logVoltage[CH_NUM];
bool g_logCurrent[CH_NUM];
float g_period = PERIOD_DEFAULT;
float g_time = TIME_DEFAULT;
trigger::Source g_triggerSource = trigger::SOURCE_IMMEDIATE;
char g_filePath[MAX_PATH_LENGTH + 1];

enum State {
	STATE_IDLE,
	STATE_INITIATED,
	STATE_TRIGGERED,
	STATE_EXECUTING
};
static State g_state = STATE_IDLE;

File g_file;
uint32_t g_nextTickCount;

void setState(State newState) {
	if (g_state != newState) {
		g_state = newState;
	}
}

int checkDlogParameters() {
	bool somethingToLog;
	for (int i = 0; i < CH_NUM; ++i) {
		if (g_logVoltage[i] || g_logCurrent[i]) {
			somethingToLog = true;
			break;
		}
	}

	if (!somethingToLog) {
		return SCPI_ERROR_EXECUTION_ERROR; // @todo find better SCPI error code
	}

	if (!*g_filePath) {
		return SCPI_ERROR_EXECUTION_ERROR; // @todo find better SCPI error code
	}

	return 0;
}

bool isIdle() {
	return g_state == STATE_IDLE;
}

int initiate(const char *filePath) {
	int error = SCPI_RES_OK;

	strcpy(g_filePath, filePath);

	if (g_triggerSource == trigger::SOURCE_IMMEDIATE) {
		error = startImmediately();
	} else {
		error = checkDlogParameters();
		if (!error) {
			setState(STATE_INITIATED);
		}
	}

	if (error) {
		g_filePath[0] = 0;
	}

	return error;
}

#define MAGIC1  0x3A92C491L
#define MAGIC2  0x1C5CBAE8L
#define VERSION 0x00000001L

void writeUint8(uint8_t value) {
	g_file.write((const uint8_t *)&value, 1);
}

void writeUint32(uint32_t value) {
	writeUint8(value & 0xFF);
	writeUint8((value >> 8) & 0xFF);
	writeUint8((value >> 16) & 0xFF);
	writeUint8(value >> 24);
}

void writeFloat(float value) {
	writeUint32(*((uint32_t *)&value));
}

int startImmediately() {
	int err = checkDlogParameters();
	if (err) {
		return err;
	}

	g_file = SD.open(g_filePath, FILE_WRITE);
	if (!g_file) {
		return SCPI_ERROR_EXECUTION_ERROR; // @todo find better SCPI error code
	}

	if (!g_file.truncate(0)) {
		return SCPI_ERROR_MASS_STORAGE_ERROR;
	}

	setState(STATE_EXECUTING);

	writeUint32(MAGIC1);
	writeUint32(MAGIC2);
	writeUint32(VERSION);
	uint32_t flag = 0;
	for (int i = 0; i < CH_NUM; ++i) {
		if (g_logVoltage[i]) {
			flag |= 0x01 << (i * 2);
		}
		if (g_logCurrent[i]){
			flag |= 0x02 << (i * 2);
		}
	}
	writeUint32(flag);
	writeFloat(g_period);
	writeFloat(g_time);
	writeUint32(datetime::nowUtc());

	g_nextTickCount = micros();
	log(g_nextTickCount);

	return SCPI_RES_OK;
}

void abort() {
	if (g_state == STATE_EXECUTING) {
		setState(STATE_IDLE);
		g_file.close();
	}
}

void log(uint32_t tickCount) {
	writeUint32(tickCount - g_nextTickCount);

	for (int i = 0; i < CH_NUM; ++i) {
		Channel &channel = Channel::get(i);
		if (g_logVoltage[i]) {
			writeFloat(channel_dispatcher::getUMon(channel));
		}
		if (g_logCurrent[i]) {
			writeFloat(channel_dispatcher::getIMon(channel));
		}
	}

	// @todo we need more precision here
	g_nextTickCount += floor(g_period * 1000000L);
}

void tick(uint32_t tickCount) {
	if (g_state == STATE_EXECUTING) {
		if (tickCount > g_nextTickCount) {
			log(tickCount);
		}
	}
}

}
}
} // namespace eez::psu::dlog

#endif // OPTION_SD_CARD