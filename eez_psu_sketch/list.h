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

void init();

void resetChannelList(Channel &channel);
void reset();

void setVoltageList(Channel &channel, float *list, uint16_t listSize);
float *getVoltageList(Channel &channel, uint16_t *listSize);

void setCurrentList(Channel &channel, float *list, uint16_t listSize);
float *getCurrentList(Channel &channel, uint16_t *listSize);

void setDwellList(Channel &channel, float *list, uint16_t listSize);
float *getDwellList(Channel &channel, uint16_t *listSize);

uint16_t getListCount(Channel &channel);
void setListCount(Channel &channel, uint16_t value);

bool areListSizesEquivalent(uint16_t size1, uint16_t size2);
bool areVoltageAndDwellListSizesEquivalent(Channel &channel);
bool areCurrentAndDwellListSizesEquivalent(Channel &channel);
bool areVoltageAndCurrentListSizesEquivalent(Channel &channel);

bool loadList(Channel &channel, int *err);
bool saveList(Channel &channel, int *err);

void executionReset(Channel &channel);
void executionSetVoltage(Channel &channel);
void executionSetCurrent(Channel &channel);
void executionStart(Channel &channel);

void tick(uint32_t tick_usec);

bool isActive();

void abort();

}
}
} // namespace eez::psu::list