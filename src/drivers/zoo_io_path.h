#ifndef __ZOO_IO_H__
#define __ZOO_IO_H__

#include <stddef.h>
#include "../libzoo/zoo.h"

#ifndef ZOO_PATH_SEPARATOR
#ifdef _WIN32
#define ZOO_PATH_SEPARATOR '\\'
#define ZOO_PATH_SEPARATOR_STR "\\"
#else
#define ZOO_PATH_SEPARATOR '/'
#define ZOO_PATH_SEPARATOR_STR "/"
#endif
#endif

#ifndef ZOO_PATH_MAX
#ifdef _WIN32
#define ZOO_PATH_MAX 260
#else
// TODO: Linux uses 4096 by default
#define ZOO_PATH_MAX 512
#endif
#endif

struct s_zoo_io_path_driver;

typedef enum {
	TYPE_DIR,
	TYPE_FILE
} zoo_io_type;

typedef struct {
	zoo_io_type type;
	char name[ZOO_PATH_MAX + 1];
} zoo_io_dirent;

typedef bool (*zoo_func_io_scan_dir_callback)(struct s_zoo_io_path_driver *drv, zoo_io_dirent *e, void *arg);

typedef struct s_zoo_io_path_driver {
    zoo_io_driver parent;

	char path[ZOO_PATH_MAX + 1];
    
    // directory traversal
	zoo_io_handle (*func_open_file_absolute)(struct s_zoo_io_path_driver *drv, const char *filename, zoo_io_mode mode);
    bool (*func_dir_scan)(struct s_zoo_io_path_driver *drv, const char *dir, zoo_func_io_scan_dir_callback cb, void *cb_arg);
    bool (*func_dir_advance)(struct s_zoo_io_path_driver *drv, char *dest, const char *curr, const char *next, size_t len);
} zoo_io_path_driver;

void zoo_io_internal_init_path_driver(zoo_io_path_driver *drv);

#endif /* __ZOO_IO_H__ */