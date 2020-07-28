/**
 * Copyright (c) 2020 Adrian Siekierka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ZOO_IO_PATH_H__
#define __ZOO_IO_PATH_H__

#include <stddef.h>
#include "zoo.h"

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
	int64_t mtime;
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

void zoo_path_cat(char *dest, const char *src, size_t n);
void zoo_io_internal_init_path_driver(zoo_io_path_driver *drv);

#endif /* __ZOO_IO_PATH_H__ */
