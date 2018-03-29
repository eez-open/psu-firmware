/*
 * EEZ PSU Firmware
 * Copyright (C) 2017-present, Envox d.o.o.
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

namespace eez {
namespace psu {
namespace list {

static const char *LIST_EXT = ".list";

void init();

void resetChannelList(Channel &channel);
void reset();

void setDwellList(Channel &channel, float *list, uint16_t listLength);
float *getDwellList(Channel &channel, uint16_t *listLength);

void setVoltageList(Channel &channel, float *list, uint16_t listLength);
float *getVoltageList(Channel &channel, uint16_t *listLength);

void setCurrentList(Channel &channel, float *list, uint16_t listLength);
float *getCurrentList(Channel &channel, uint16_t *listLength);

bool getListsChanged(Channel &channel);
void setListsChanged(Channel &channel, bool changed);

uint16_t getListCount(Channel &channel);
void setListCount(Channel &channel, uint16_t value);

bool isListEmpty(Channel &channel);

bool areListLengthsEquivalent(uint16_t size1, uint16_t size2);
bool areListLengthsEquivalent(uint16_t size1, uint16_t size2, uint16_t size3);
bool areListLengthsEquivalent(Channel &channel);
bool areVoltageAndDwellListLengthsEquivalent(Channel &channel);
bool areCurrentAndDwellListLengthsEquivalent(Channel &channel);
bool areVoltageAndCurrentListLengthsEquivalent(Channel &channel);

int checkLimits();

bool loadList(Channel &channel, const char *filePath, int *err);
bool saveList(Channel &channel, const char *filePath, int *err);

void executionStart(Channel &channel);

int maxListsSize(Channel &channel);

bool setListValue(Channel &channel, int16_t it, int *err);

void tick(uint32_t tick_usec);

bool isActive();

bool anyCounterVisible(uint32_t totalThreshold);
bool getCurrentDwellTime(Channel &channel, int32_t &remaining, uint32_t &total);

void abort();

}
}
} // namespace eez::psu::list