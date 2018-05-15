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

namespace eez {
namespace psu {

void strcatFloatValue(char *str, float value, Unit unit, int channelIndex);
void strcatVoltage(char *str, float value, int numSignificantDecimalDigits = -1, int channelIndex = -1);
void strcatCurrent(char *str, float value, int numSignificantDecimalDigits = -1, int channelIndex = -1);
void strcatPower(char *str, float value);
void strcatDuration(char *str, float value);
void strcatLoad(char *str, float value);

bool greater(float a, float b, Unit unit, int channelIndex = -1);
bool greaterOrEqual(float a, float b, Unit unit, int channelIndex = -1);
bool less(float a, float b, Unit unit, int channelIndex = -1);
bool lessOrEqual(float a, float b, Unit unit, int channelIndex = -1);
bool equal(float a, float b, Unit unit, int channelIndex = -1);
bool between(float x, float a, float b, Unit unit, int channelIndex = -1);

}
} // namespace eez::psu::util
