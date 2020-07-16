#include <string.h>
#include "zoo_io_path.h"

void zoo_path_cat(char *dest, const char *src, size_t n) {
	size_t len = strlen(dest);
	if (len < n && dest[len - 1] != ZOO_PATH_SEPARATOR) {
		dest[len++] = ZOO_PATH_SEPARATOR;
		dest[len] = '\0';
	}
	strncpy(dest + len, src, n - len);
}

typedef struct {
	char fn_cmp[ZOO_PATH_MAX + 1];
	char fn_found[ZOO_PATH_MAX + 1];
} zoo_io_translate_state;

static bool zoo_io_translate_compare(zoo_io_path_driver *drv, zoo_io_dirent *e, void *cb_arg) {
	zoo_io_translate_state *ts = (zoo_io_translate_state *) cb_arg;

	if (e->type == TYPE_FILE && !strcasecmp(e->name, ts->fn_cmp)) {
		strncpy(ts->fn_found, e->name, ZOO_PATH_MAX);
		return false;
	} else {
		return true;
	}
}

static void zoo_io_translate(zoo_io_path_driver *drv, const char *filename, char *buffer, size_t buflen) {
	zoo_io_translate_state ts;

    // fn_cmp - searched name, fn_found - found actual name
    strncpy(ts.fn_cmp, filename, ZOO_PATH_MAX);
    ts.fn_found[0] = '\0';

    drv->func_dir_scan(drv, drv->path, zoo_io_translate_compare, &ts);

    strncpy(buffer, drv->path, buflen);
    zoo_path_cat(buffer, (ts.fn_found[0] != '\0') ? ts.fn_found : ts.fn_cmp, buflen);
}

static zoo_io_handle zoo_io_open_relative(zoo_io_driver *p_drv, const char *filename, zoo_io_mode mode) {
    zoo_io_path_driver *drv = (zoo_io_path_driver*) p_drv;
    char buffer[ZOO_PATH_MAX + 1];

    // only support files, not directories, here
    if (strchr(filename, ZOO_PATH_SEPARATOR) != NULL) {
        return zoo_io_open_file_empty();
    }

    if (drv->func_dir_scan != NULL) {
        zoo_io_translate(drv, filename, buffer, ZOO_PATH_MAX);
    } else {
        strncpy(buffer, drv->path, ZOO_PATH_MAX);
        zoo_path_cat(buffer, filename, ZOO_PATH_MAX);
    }

    // try opening file
    return drv->func_open_file_absolute(drv, buffer, mode);
}

static bool zoo_io_path_advance(zoo_io_path_driver *drv, char *dest, const char *curr, const char *next, size_t len) {
    char *pathsep_pos;
    int pathsep_len;

    if (curr == NULL) {
        return false;
    } else if (next == NULL || !strcmp(next, ".")) {
        strncpy(dest, curr, len);
        return true;
    } else if (!strcmp(next, "..")) {
        pathsep_pos = strrchr(curr, ZOO_PATH_SEPARATOR);
        if (pathsep_pos == NULL) {
            return false;
        }
        pathsep_len = pathsep_pos - curr;
        if (pathsep_len >= len) {
            return false;
        }
        memcpy(dest, curr, pathsep_len);
        dest[pathsep_len] = '\0';
        return true;
    } else {
        strncpy(dest, curr, len);
        zoo_path_cat(dest, next, len);
        return true;
    }
}

void zoo_io_internal_init_path_driver(zoo_io_path_driver *drv) {
	memset(drv, 0, sizeof(zoo_io_path_driver));
    drv->parent.func_open_file = zoo_io_open_relative;
    drv->func_dir_advance = zoo_io_path_advance;
}
