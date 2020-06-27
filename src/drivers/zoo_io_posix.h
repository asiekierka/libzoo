#ifndef __ZOO_IO_POSIX_H__
#define __ZOO_IO_POSIX_H__

#include <stddef.h>
#include "../libzoo/zoo.h"
#include "zoo_io_path.h"

void zoo_io_create_posix_driver(zoo_io_path_driver *drv);

#endif /* __ZOO_IO_POSIX_H__ */