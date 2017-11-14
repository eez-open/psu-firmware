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

#include <string>

namespace eez {
namespace psu {
namespace simulator {
namespace arduino {

#define FILE_READ  1 // O_READ
#define FILE_WRITE 2 // O_READ | O_WRITE | O_CREAT
#define READ_ONLY  3 // O_RDONLY

class FileImpl;

class File {
    friend class SimulatorSD;

public:
    File();
    File(const File &file);
    File(const char *path, uint8_t mode = READ_ONLY);
    File &operator=(const File &file);

    ~File();
    void close();

    operator bool();
    const char *name();
    uint32_t size();
    bool isDirectory();

    void rewindDirectory();
    File openNextFile(uint8_t mode = READ_ONLY);

    bool available();
    bool seek(uint32_t pos);
    int peek();
    int read();
    int read(void *buf, uint16_t nbyte);

    void print(float value, int numDecimalDigits);
    void print(char value);

private:
    FileImpl *m_impl;
};

class SimulatorSD {
public:
    bool begin(uint8_t cs);
    File open(const char *path, uint8_t mode = FILE_READ);
    bool exists(const char *path);
    bool mkdir(const char *path);
    bool remove(const char *path);
};

extern SimulatorSD SD;

}
}
}
} // namespace eez::psu::simulator::arduino;
