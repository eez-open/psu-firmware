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

#include "eez/app/psu.h"
#include "eez/app/platform/simulator/SdFat.h"
#include "eez/app/util.h"

#include <time.h>

#ifdef _WIN32

#undef INPUT
#undef OUTPUT

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <direct.h>
#include <io.h>

#else

#include <dirent.h>
#include <sys/stat.h> // mkdir
#include <errno.h>
#include <unistd.h>
#include <sys/statvfs.h>

#endif

#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif

////////////////////////////////////////////////////////////////////////////////

std::string getRealPath(const char *path) {
    std::string realPath;

    realPath = "sd_card";

    if (path[0]) {
        if (path[0] != '/') {
            realPath += "/";
        }

        realPath += path;
    }

    // remove trailing slash
    if (realPath[realPath.size() - 1] == '/') {
        realPath = realPath.substr(0, realPath.size()-1);
    }

    return getConfFilePath(realPath.c_str());
}

bool pathExists(const char *path) {
    std::string realPath = getRealPath(path);
    struct stat path_stat;
    int result = stat(realPath.c_str(), &path_stat);
    return result == 0;
}

////////////////////////////////////////////////////////////////////////////////

class FileImpl {
    friend class File;
    friend class SdFat;

public:
    FileImpl();
    FileImpl(const char *path, uint8_t mode = READ_ONLY);
    ~FileImpl();

    operator bool();
    bool getName(char *name, size_t size);
    uint32_t size();
    bool isDirectory();

    void close();

    bool dirEntry(dir_t* dir);

    void rewindDirectory();
    File openNextFile(uint8_t mode = READ_ONLY);

    bool truncate(uint32_t length);

    bool available();
    bool seek(uint32_t pos);
    int peek();
    int read();
    int read(void *buf, uint16_t nbyte);
    size_t write(const uint8_t *buf, size_t size);
	void sync();

    void print(float value, int numDecimalDigits);
    void print(char value);

private:
    int m_refCount;
    std::string m_path;
    uint8_t m_mode;
    FILE *m_fp;

#ifdef _WIN32
    void *m_hFind;
#else
    void *m_dp;
#endif

    void ref();
    void unref();

    void init();
    void open();
    std::string getRealPath();
};

////////////////////////////////////////////////////////////////////////////////

FileImpl::FileImpl() {
    init();
}

FileImpl::FileImpl(const char *path, uint8_t mode)
    : m_path(path)
    , m_mode(mode)
{
    init();
}

void FileImpl::init() {
    m_refCount = 0;
    m_fp = NULL;
#ifdef _WIN32
    m_hFind = INVALID_HANDLE_VALUE;
#else
    m_dp = 0;
#endif
}

void FileImpl::open() {
    if (m_mode == FILE_WRITE) {
        m_fp = fopen(getRealPath().c_str(), "ab");
    } else {
        m_fp = fopen(getRealPath().c_str(), "rb");
    }
}

void FileImpl::ref() {
    ++m_refCount;
}

void FileImpl::unref() {
    --m_refCount;
    if (m_refCount == 0) {
        delete this;
    }
}

FileImpl::~FileImpl() {
    close();
}

std::string FileImpl::getRealPath() {
    return ::getRealPath(m_path.c_str());
}

FileImpl::operator bool() {
    if (m_fp) {
        return true;
    }

    if (m_path.length() == 0) {
        return false;
    }

    return pathExists(m_path.c_str());
}

bool FileImpl::getName(char *name, size_t size) {
    strncpy(name, strrchr(m_path.c_str(), '/'), size);
    name[size] = 0;
    return true;
}

uint32_t FileImpl::size() {
    FILE *fp = fopen(getRealPath().c_str(), "rb");
    if (!fp) {
        return 0;
    }

    fseek(fp, 0, SEEK_END);

    uint32_t size = ftell(fp);

    fclose(fp);

    return size;
}

bool FileImpl::isDirectory() {
    struct stat path_stat;
    if (stat(getRealPath().c_str(), &path_stat) != 0) {
        return false;
    }
    return !S_ISREG(path_stat.st_mode);
}

void FileImpl::close() {
    if (m_fp) {
        fclose(m_fp);
        m_fp = NULL;
    }

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

bool FileImpl::dirEntry(dir_t* dir) {
    time_t mtime;

#ifdef _WIN32
    HANDLE hFile = CreateFileA(getRealPath().c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    FILETIME lastWriteTime;
    BOOL result = GetFileTime(hFile, NULL, NULL, &lastWriteTime);

    CloseHandle(hFile);

    if (!result) {
        return false;
    }

    ULARGE_INTEGER ull;

    ull.LowPart = lastWriteTime.dwLowDateTime;
    ull.HighPart = lastWriteTime.dwHighDateTime;

    mtime = ull.QuadPart / 10000000ULL - 11644473600ULL;
#else
    struct stat st = {0};
    int ret = lstat(getRealPath().c_str(), &st);
    if (ret == -1) {
        return false;
    }

    mtime = st.st_mtime;
#endif

    struct tm *tmmtime = gmtime(&mtime);

    dir->lastWriteDate = FAT_DATE(tmmtime->tm_year + 1900, tmmtime->tm_mon + 1, tmmtime->tm_mday);
    dir->lastWriteTime = FAT_TIME(tmmtime->tm_hour, tmmtime->tm_min, tmmtime->tm_sec);

    return true;
}

void FileImpl::rewindDirectory() {
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

File FileImpl::openNextFile(uint8_t mode) {
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

bool FileImpl::seek(uint32_t pos) {
    return fseek(m_fp, pos, SEEK_SET) == 0;
}

bool FileImpl::truncate(uint32_t length) {
#ifdef _WIN32
    return _chsize(_fileno(m_fp), length) == 0;
#else
    return ftruncate(fileno(m_fp), length) == 0;
#endif
}

bool FileImpl::available() {
    return peek() != EOF;
}

int FileImpl::peek() {
    int c = getc(m_fp);

    if (c == EOF) {
        return EOF;
    }

    ungetc(c, m_fp);

    return c;
}

int FileImpl::read() {
    return getc(m_fp);
}

int FileImpl::read(void *buf, uint16_t nbyte) {
    return fread(buf, 1, nbyte, m_fp);
}

size_t FileImpl::write(const uint8_t *buf, size_t size) {
    return fwrite(buf, 1, size, m_fp);
}

void FileImpl::sync() {
	fflush(m_fp);
}


void FileImpl::print(float value, int numDecimalDigits) {
    fprintf(m_fp, "%.*f", numDecimalDigits, value);
}

void FileImpl::print(char value) {
    fputc(value, m_fp);
}

////////////////////////////////////////////////////////////////////////////////

File::File() {
    m_impl = new FileImpl();
    m_impl->ref();
}

File::File(const char *path, uint8_t mode) {
    m_impl = new FileImpl(path, mode);
    m_impl->ref();
}

File::File(const File &file) {
    m_impl = file.m_impl;
    m_impl->ref();
}

File &File::operator =(const File &file) {
    if (m_impl) m_impl->unref();
    m_impl = file.m_impl;
    m_impl->ref();
    return *this;
}

File::~File() {
    m_impl->unref();
}

void File::close() {
    return m_impl->close();
}

void File::dateTimeCallback(void (*dateTime)(uint16_t* date, uint16_t* time)) {
}

bool File::dirEntry(dir_t* dir) {
    return m_impl->dirEntry(dir);
}

File::operator bool() {
    return m_impl->operator bool();
}

bool File::getName(char *name, size_t size) {
    return m_impl->getName(name, size);
}

uint32_t File::size() {
    return m_impl->size();
}

bool File::isDirectory() {
    return m_impl->isDirectory();
}

void File::rewindDirectory() {
    m_impl->rewindDirectory();
}

File File::openNextFile(uint8_t mode) {
    return m_impl->openNextFile(mode);
}

bool File::truncate(uint32_t length) {
    return m_impl->truncate(length);
}

bool File::available() {
    return m_impl->available();
}

bool File::seek(uint32_t pos) {
    return m_impl->seek(pos);
}

int File::peek() {
    return m_impl->peek();
}

int File::read() {
    return m_impl->read();
}

int File::read(void *buf, uint16_t nbyte) {
    return m_impl->read(buf, nbyte);
}

size_t File::write(const uint8_t *buf, size_t size) {
    return m_impl->write(buf, size);
}

void File::sync() {
	return m_impl->sync();
}

void File::print(float value, int numDecimalDigits) {
    m_impl->print(value, numDecimalDigits);
}

void File::print(char value) {
    m_impl->print(value);
}

////////////////////////////////////////////////////////////////////////////////

bool SdFat::begin() {
    // make sure SD card root path exists
    mkdir("/");
    return File("/");
}

File SdFat::open(const char *path, uint8_t mode) {
    File file(path, mode);
    file.m_impl->open();
    return file;
}

bool SdFat::exists(const char *path) {
    return pathExists(path);
}

bool SdFat::rename(const char *sourcePath, const char *destinationPath) {
    std::string realSourcePath = getRealPath(sourcePath);
    std::string realDestinationPath = getRealPath(destinationPath);
    return ::rename(realSourcePath.c_str(), realDestinationPath.c_str()) == 0;
}

bool SdFat::remove(const char *path) {
    std::string realPath = getRealPath(path);
    return ::remove(realPath.c_str()) == 0;
}

bool SdFat::mkdir(const char *path) {
    if (pathExists(path)) {
        return true;
    }

    char parentDir[205];
    getParentDir(path, parentDir);

    if (parentDir[0]) {
        if (!mkdir(parentDir)) {
            return false;
        }
    }

    std::string realPath = getRealPath(path);

    int result;
#ifdef _WIN32
    result = ::_mkdir(realPath.c_str());
#else
    result = ::mkdir(realPath.c_str(), 0700);
#endif
    return result == 0 || result == EEXIST;
}

bool SdFat::rmdir(const char *path) {
    std::string realPath = getRealPath(path);
#ifdef _WIN32
    int result = ::_rmdir(realPath.c_str());
#else
    int result = ::rmdir(realPath.c_str());
#endif
    return result == 0;
}

bool SdFat::getInfo(uint64_t &usedSpace, uint64_t &freeSpace) {
#ifdef _WIN32
    DWORD sectorsPerCluster, bytesPerSector, numberOfFreeClusters, totalNumberOfClusters;
    GetDiskFreeSpaceA(getRealPath("").c_str(), &sectorsPerCluster, &bytesPerSector, &numberOfFreeClusters, &totalNumberOfClusters);
    usedSpace = uint64_t(bytesPerSector) * sectorsPerCluster * (totalNumberOfClusters - numberOfFreeClusters);
    freeSpace = uint64_t(bytesPerSector) * sectorsPerCluster * numberOfFreeClusters;
    return true;
#else
    struct statvfs buf;
    statvfs(getRealPath("").c_str(), &buf);
    usedSpace = uint64_t(buf.f_bsize) * (buf.f_blocks - buf.f_bfree);
    freeSpace = uint64_t(buf.f_bsize) * buf.f_bfree;
    return true;
#endif
}
