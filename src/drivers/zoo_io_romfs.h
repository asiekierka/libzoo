#ifndef __ZOO_IO_POSIX_H__
#define __ZOO_IO_POSIX_H__

#include <stddef.h>
#include "../libzoo/zoo.h"
#include "zoo_io_path.h"

typedef struct {
    zoo_io_path_driver parent;
    uint8_t *fs;
    uint8_t *root_ptr;
} zoo_io_romfs_driver;

bool zoo_io_create_romfs_driver(zoo_io_romfs_driver *drv, uint8_t *fs_ptr);

#endif /* __ZOO_IO_POSIX_H__ */