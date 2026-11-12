#ifndef PRO_INSTALLER_PACKAGE_H
#define PRO_INSTALLER_PACKAGE_H

#include <stdint.h>

#define PRO_FILE_COUNT 9
#define PRO_PATH_SIZE 64

typedef struct {
    char name[PRO_PATH_SIZE];
    uint32_t size;
    int offset;
} ProFile;

typedef struct {
    int (*read)(int fd, void *buffer, unsigned size);
    int (*seek)(int fd, int offset, int whence);
} ProReader;

int pro_select_files(uint32_t firmware, int model, ProFile files[PRO_FILE_COUNT]);
int pro_validate_package(const ProReader *io, int fd, ProFile files[PRO_FILE_COUNT]);

#endif
