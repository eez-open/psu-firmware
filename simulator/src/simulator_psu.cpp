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

#include "psu.h"
#include "chips.h"
#if OPTION_DISPLAY
#include "front_panel/control.h"
#endif

#include "main_loop.h"

// for home directory (see getConfFilePath)
#ifdef _WIN32
#undef INPUT
#undef OUTPUT
#include <Windows.h>
#pragma warning(push)
#pragma warning(disable: 4091)
#include <Shlobj.h> 
#pragma warning(pop)
#include <direct.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#endif

namespace eez {
namespace psu {
namespace simulator {

float temperature[temp_sensor::TEMP_SENSORS_COUNT];

void init() {
    for (int i = 0; i < temp_sensor::TEMP_SENSORS_COUNT; ++i) {
        temperature[i] = 25.0f;
    }
}

void tick() {
    chips::tick();
    psu::tick();
#if OPTION_DISPLAY
    front_panel::tick();
#endif
}

void setTemperature(int sensor, float value) {
    temperature[sensor] = value;
}

float getTemperature(int sensor) {
    return temperature[sensor];
}

char *getConfFilePath(char *file_name) {
    static char file_path[1024];

    *file_path = '\0';

#ifdef _WIN32
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, file_path))) {
        strcat(file_path, "\\.eez_psu_sim");
        _mkdir(file_path);
        strcat(file_path, "\\");
    }
#else
    const char *home_dir = 0;
    if ((home_dir = getenv("HOME")) == NULL) {
        home_dir = getpwuid(getuid())->pw_dir;
    }
    if (home_dir) {
        strcat(file_path, home_dir);
        strcat(file_path, "/.eez_psu_sim");
        mkdir(file_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        strcat(file_path, "/");
    }
#endif

    strcat(file_path, file_name);
    return file_path;
}

void exit() {
    main_loop_exit();
}

} // namespace simulator

}
} // namespace eez::psu