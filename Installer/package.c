#include "package.h"

#include <stdio.h>
#include <string.h>

#define FW_620 0x06020010u
#define FW_639 0x06030910u
#define FW_660 0x06060010u
#define FW_661 0x06060110u
#define MAX_PACKAGE_FILES 64
#define MAX_FILE_SIZE (8u * 1024u * 1024u)

static int read_exact(const ProReader *io, int fd, void *buffer, unsigned size)
{
    unsigned char *p = buffer;
    while (size) {
        int n = io->read(fd, p, size);
        if (n <= 0 || (unsigned)n > size)
            return -1;
        p += n;
        size -= (unsigned)n;
    }
    return 0;
}

int pro_select_files(uint32_t firmware, int model, ProFile files[PRO_FILE_COUNT])
{
    static const char *common[] = {
        "/kd/_systemctrl.prx", "/kd/_vshctrl.prx", "/kd/_stargate.prx",
        "/kd/_popcorn.prx", "/kd/_inferno.prx", "/kd/_galaxy.prx",
        "/vsh/module/_recovery.prx", "/vsh/module/_satelite.prx"
    };
    if (firmware != FW_620 && firmware != FW_639 &&
        firmware != FW_660 && firmware != FW_661)
        return -1;
    if (model < 0 || model > 10)
        return -1;
    if ((firmware == FW_620 && model > 4) ||
        (firmware == FW_639 && model == 10))
        return -1;
    memset(files, 0, sizeof(*files) * PRO_FILE_COUNT);
    strcpy(files[0].name, "/kd/_march33.prx");
    for (unsigned i = 0; i < sizeof(common) / sizeof(common[0]); ++i)
        strcpy(files[i + 1].name, common[i]);
    strcpy(files[9].name, "/kd/_usbdevice.prx");
    return 0;
}

int pro_validate_package(const ProReader *io, int fd, ProFile files[PRO_FILE_COUNT])
{
    unsigned char raw[5], buffer[1024];
    unsigned count;
    if (io->seek(fd, 0, 0) < 0 || read_exact(io, fd, raw, 4) < 0)
        return -1;
    count = (unsigned)raw[0] | ((unsigned)raw[1] << 8) |
            ((unsigned)raw[2] << 16) | ((unsigned)raw[3] << 24);
    if (!count || count > MAX_PACKAGE_FILES)
        return -1;
    for (unsigned i = 0; i < count; ++i) {
        char name[256];
        unsigned size, remaining;
        int offset;
        if (read_exact(io, fd, raw, 5) < 0)
            return -1;
        size = (unsigned)raw[0] | ((unsigned)raw[1] << 8) |
               ((unsigned)raw[2] << 16) | ((unsigned)raw[3] << 24);
        if (!size || size > MAX_FILE_SIZE || !raw[4] ||
            read_exact(io, fd, name, raw[4]) < 0)
            return -1;
        name[raw[4]] = 0;
        if (memchr(name, 0, raw[4]))
            return -1;
        offset = io->seek(fd, 0, 1);
        if (offset < 0)
            return -1;
        for (unsigned j = 0; j < PRO_FILE_COUNT; ++j) {
            if (strcmp(name, files[j].name) == 0) {
                if (files[j].size)
                    return -1;
                files[j].size = size;
                files[j].offset = offset;
            }
        }
        remaining = size;
        while (remaining) {
            unsigned n = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
            if (read_exact(io, fd, buffer, n) < 0)
                return -1;
            remaining -= n;
        }
    }
    if (io->read(fd, buffer, 1) != 0)
        return -1;
    for (unsigned i = 0; i < PRO_FILE_COUNT; ++i)
        if (!files[i].size)
            return -1;
    return 0;
}
