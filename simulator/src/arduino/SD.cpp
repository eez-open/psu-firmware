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
#include "SD.h"

#ifdef _WIN32

#undef INPUT
#undef OUTPUT

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <direct.h>

#else

#include <dirent.h>
#include <sys/stat.h> // mkdir

#endif

#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

namespace eez {
namespace psu {
namespace simulator {
namespace arduino {

SimulatorSD SD;

////////////////////////////////////////////////////////////////////////////////

bool SimulatorSD::begin(uint8_t cs) {
    const char *path = getConfFilePath("sd_card");
#ifdef _WIN32
    _mkdir(path);
#else
    mkdir(path, 0700);
#endif
    return File("/");
}

File SimulatorSD::open(const char *path, uint8_t mode) {
    return File(path, mode);
}

////////////////////////////////////////////////////////////////////////////////

File::File() {
    init();
}

File::File(const char *path, uint8_t mode)
    : m_path(path)
    , m_mode(mode)
{
    init();
}

void File::init() {
#ifdef _WIN32
    m_hFind = INVALID_HANDLE_VALUE;
#else
    m_dp = 0;
#endif
}

File::~File() {
    close();
}
std::string File::getRealPath() {
    std::string path;
    
    path = "sd_card";

    if (m_path[0] != '/') {
        path += "/";
    }

    path += m_path;

    // remove trailing slash
    if (path[path.size() - 1] == '/') {
        path = path.substr(0, path.size()-1);
    }

    return getConfFilePath(path.c_str());
}

File::operator bool() {
    if (m_path.length() == 0) {
        return false;
    }
    struct stat path_stat;   
    return stat(getRealPath().c_str(), &path_stat) == 0;
}

const char *File::name() {
    return strrchr(m_path.c_str(), '/');
}

uint32_t File::size() {
    FILE *fp = fopen(getRealPath().c_str(), "rb");
    if (!fp) {
        return 0;
    }
    
    fseek(fp, 0, SEEK_END);

    uint32_t size = ftell(fp);

    fclose(fp);

    return size;
}

bool File::isDirectory() {
    struct stat path_stat;
    if (stat(getRealPath().c_str(), &path_stat) != 0) {
        return false;
    }
    return !S_ISREG(path_stat.st_mode);
}

void File::close() {
#ifdef _WIN32
    if (m_hFind != INVALID_HANDLE_VALUE) {
        FindClose(m_hFind);
        m_hFind = INVALID_HANDLE_VALUE;
    }
#else
    if (m_dp) {
        closedir((DIR *)m_dp);
        m_dp = 0;
    }
#endif
    m_path = "";
}

void File::rewindDirectory() {
#ifdef _WIN32
    if (m_hFind != INVALID_HANDLE_VALUE) {
        FindClose(m_hFind);
        m_hFind = INVALID_HANDLE_VALUE;
    }
#else
    if (m_dp) {
        closedir((DIR *)m_dp);
    }
    m_dp = opendir(getRealPath().c_str());
#endif
}

File File::openNextFile(uint8_t mode) {
    const char *name;
#ifdef _WIN32
    WIN32_FIND_DATAA ffd;
    if (m_hFind == INVALID_HANDLE_VALUE) {
        std::string path = getRealPath() + "\\*.*";
        m_hFind = FindFirstFileA(path.c_str(), &ffd);
        if (m_hFind == INVALID_HANDLE_VALUE) {
            return File();
        }
    } else {
        if (!FindNextFileA(m_hFind, &ffd)) {
            FindClose(m_hFind);
            m_hFind = INVALID_HANDLE_VALUE;
            return File();
        }
    }
    name = ffd.cFileName;
#else
    if (!m_dp) {
        return File();
    }

    struct dirent *ep = readdir((DIR *)m_dp);
    if (!ep) {
        closedir((DIR *)m_dp);
        m_dp = 0;
        return File();
    }
    name = ep->d_name;
#endif

    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        return openNextFile(mode);
    }

    return File((m_path + '/' + name).c_str(), mode);
}

}
}
}
} // namespace eez::psu::simulator::arduino;
