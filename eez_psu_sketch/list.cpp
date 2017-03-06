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
 
#include "psu.h"
#include "list.h"
#include "trigger.h"
#include "channel_dispatcher.h"

namespace eez {
namespace psu {
namespace list {

static struct {
    float voltageList[MAX_LIST_SIZE];
    uint16_t voltageListSize;

    float currentList[MAX_LIST_SIZE];
    uint16_t currentListSize;

    float dwellList[MAX_LIST_SIZE];
    uint16_t dwellListSize;

    uint16_t count;
} g_channelsLists[CH_NUM];

static struct {
    struct {
        unsigned setVoltage: 1;
        unsigned setCurrent: 1;
    } flags; 
    int counter;
    int it;
    uint32_t nextPointTime;
} g_execution[CH_NUM];

////////////////////////////////////////////////////////////////////////////////

void init() {
    reset();
}

void resetChannelList(Channel &channel) {
    int i = channel.index - 1;

    g_channelsLists[i].voltageListSize = 0;
    g_channelsLists[i].currentListSize = 0;
    g_channelsLists[i].dwellListSize = 0;

    g_channelsLists[i].count = 1;

    g_execution[i].counter = -1;
}

void reset() {
    for (int i = 0; i < CH_NUM; ++i) {
        resetChannelList(Channel::get(i));
    }
}

void setVoltageList(Channel &channel, float *list, uint16_t listSize) {
    memcpy(g_channelsLists[channel.index - 1].voltageList, list, listSize * sizeof(float));
    g_channelsLists[channel.index - 1].voltageListSize = listSize;
}

float *getVoltageList(Channel &channel, uint16_t *listSize) {
    *listSize = g_channelsLists[channel.index - 1].voltageListSize;
    return g_channelsLists[channel.index - 1].voltageList;
}

void setCurrentList(Channel &channel, float *list, uint16_t listSize) {
    memcpy(g_channelsLists[channel.index - 1].currentList, list, listSize * sizeof(float));
    g_channelsLists[channel.index - 1].currentListSize = listSize;
}

float *getCurrentList(Channel &channel, uint16_t *listSize) {
    *listSize = g_channelsLists[channel.index - 1].currentListSize;
    return g_channelsLists[channel.index - 1].currentList;
}

void setDwellList(Channel &channel, float *list, uint16_t listSize) {
    memcpy(g_channelsLists[channel.index - 1].dwellList, list, listSize * sizeof(float));
    g_channelsLists[channel.index - 1].dwellListSize = listSize;
}

float *getDwellList(Channel &channel, uint16_t *listSize) {
    *listSize = g_channelsLists[channel.index - 1].dwellListSize;
    return g_channelsLists[channel.index - 1].dwellList;
}

uint16_t getListCount(Channel &channel) {
    return g_channelsLists[channel.index - 1].count;
}

void setListCount(Channel &channel, uint16_t value) {
    g_channelsLists[channel.index - 1].count = value;
}

bool areListSizesEquivalent(uint16_t size1, uint16_t size2) {
    return size1 != 0 && size2 != 0 && (size1 == 1 || size2 == 1 || size1 == size2);
}

bool areVoltageAndDwellListSizesEquivalent(Channel &channel) {
    return areListSizesEquivalent(g_channelsLists[channel.index - 1].voltageListSize, g_channelsLists[channel.index - 1].dwellListSize);
}

bool areCurrentAndDwellListSizesEquivalent(Channel &channel) {
    return areListSizesEquivalent(g_channelsLists[channel.index - 1].currentListSize, g_channelsLists[channel.index - 1].dwellListSize);
}

bool areVoltageAndCurrentListSizesEquivalent(Channel &channel) {
    return areListSizesEquivalent(g_channelsLists[channel.index - 1].voltageListSize, g_channelsLists[channel.index - 1].currentListSize);
}

bool loadList(Channel &channel, int *err) {
    return true;
}

bool saveList(Channel &channel, int *err) {
    return true;
}

void executionReset(Channel &channel) {
    g_execution[channel.index - 1].flags.setVoltage = 0;
    g_execution[channel.index - 1].flags.setCurrent = 0;
}

void executionSetVoltage(Channel &channel) {
    g_execution[channel.index - 1].flags.setVoltage = 1;
}

void executionSetCurrent(Channel &channel) {
    g_execution[channel.index - 1].flags.setCurrent = 1;
}

void executionStart(Channel &channel) {
    g_execution[channel.index - 1].it = -1;
    g_execution[channel.index - 1].counter = g_channelsLists[channel.index - 1].count;
}

int maxListsSize(Channel &channel) {
    uint16_t maxSize = 0;

    if (g_channelsLists[channel.index - 1].voltageListSize > maxSize) {
        maxSize = g_channelsLists[channel.index - 1].voltageListSize;
    }

    if (g_channelsLists[channel.index - 1].currentListSize > maxSize) {
        maxSize = g_channelsLists[channel.index - 1].currentListSize;
    }

    if (g_channelsLists[channel.index - 1].dwellListSize > maxSize) {
        maxSize = g_channelsLists[channel.index - 1].dwellListSize;
    }

    return maxSize;
}

static bool g_active = false;
static int g_min;
static int g_last;
static int g_max;

void tick(uint32_t tick_usec) {
#if CONF_DEBUG_VARIABLES
    debug::g_listTickDuration.tick(tick_usec);
#endif

    bool active = false;

    for (int i = 0; i < CH_NUM; ++i) {
        Channel &channel = Channel::get(i);
        if (g_execution[i].counter >= 0) {
            active = true;
            break;
        }
    }

    if (g_active != active) {
        if (g_active) {
            DebugTraceF("Min: %d", g_min);
            DebugTraceF("Max: %d", g_max);
        } else {
            g_min = 1000000;
            g_max = 1;
            g_last = tick_usec;
        }

        g_active = active;
    } else {
        if (g_active) {
            int time = tick_usec - g_last;
            if (time < g_min) {
                g_min = time;
            }
            if (time > g_max) {
                g_max = time;
            }
            g_last = tick_usec;
        }
    }

    for (int i = 0; i < CH_NUM; ++i) {
        Channel &channel = Channel::get(i);
        if (g_execution[i].counter >= 0) {
            bool set = false;

            if (g_execution[i].it == -1) {
                set = true;
            } else {
                int32_t diff = g_execution[i].nextPointTime - tick_usec;
                if (diff <= 0) {
                    set = true;
                }
            }

            if (set) {
                if (++g_execution[i].it == maxListsSize(channel)) {
                    if (g_execution[i].counter > 0) {
                        if (--g_execution[i].counter == 0) {
                            g_execution[i].counter = -1;

                            if (g_execution[i].flags.setVoltage) {
                                trigger::setVoltageTriggerFinished(channel);
                            }

                            if (g_execution[i].flags.setCurrent) {
                                trigger::setCurrentTriggerFinished(channel);
                            }

                            return;
                        }
                    }

                    g_execution[i].it = 0;
                }

                if (g_execution[i].flags.setVoltage) {
                    float voltage = g_channelsLists[i].voltageList[g_execution[i].it % g_channelsLists[i].voltageListSize];

	                if (voltage > channel_dispatcher::getULimit(channel)) {
                        generateError(SCPI_ERROR_VOLTAGE_LIMIT_EXCEEDED);
                        abort();
                        return;
	                }

	                if (voltage * channel_dispatcher::getISet(channel) > channel_dispatcher::getPowerLimit(channel)) {
                        generateError(SCPI_ERROR_POWER_LIMIT_EXCEEDED);
                        abort();
                        return;
                    }

                    channel_dispatcher::setVoltage(channel, voltage);
                }

                if (g_execution[i].flags.setCurrent) {
                    float current = g_channelsLists[i].currentList[g_execution[i].it % g_channelsLists[i].currentListSize];

                    if (current > channel_dispatcher::getILimit(channel)) {
                        generateError(SCPI_ERROR_CURRENT_LIMIT_EXCEEDED);
                        abort();
                        return;
	                }

                    if (current * channel_dispatcher::getUSet(channel) > channel_dispatcher::getPowerLimit(channel)) {
                        generateError(SCPI_ERROR_POWER_LIMIT_EXCEEDED);
                        abort();
                        return;
                    }

                    channel_dispatcher::setCurrent(channel, current);
                }

                uint32_t dwell = (uint32_t)round(g_channelsLists[i].dwellList[g_execution[i].it % g_channelsLists[i].dwellListSize] * 1000000L);
                g_execution[i].nextPointTime = tick_usec + dwell;
            }
        }
    }
}

bool isActive() {
    return g_active;
}

void abort() {
    for (int i = 0; i < CH_NUM; ++i) {
        g_execution[i].counter = -1;
    }
}

}
}
} // namespace eez::psu::list