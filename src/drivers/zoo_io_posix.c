#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include "zoo_io_posix.h"

#include <sys/stat.h>

static uint8_t zoo_io_file_getc(zoo_io_handle *h) {
	FILE *f = (FILE*) h->p;
	int result = fgetc(f);
	return (result == EOF) ? 0 : result;
}

static size_t zoo_io_file_putc(zoo_io_handle *h, uint8_t v) {
	FILE *f = (FILE*) h->p;
	return (fputc(v, f) == v) ? 1 : 0;
}

static size_t zoo_io_file_read(zoo_io_handle *h, uint8_t *d_ptr, size_t len) {
	FILE *f = (FILE*) h->p;
	return fread(d_ptr, 1, len, f);
}

static size_t zoo_io_file_write(zoo_io_handle *h, const uint8_t *d_ptr, size_t len) {
	FILE *f = (FILE*) h->p;
	return fwrite(d_ptr, 1, len, f);
}

static size_t zoo_io_file_skip(zoo_io_handle *h, size_t len) {
	FILE *f = (FILE*) h->p;
	// TODO: check if works on writes
	fseek(f, len, SEEK_CUR);
	return len;
}

static size_t zoo_io_file_tell(zoo_io_handle *h) {
	FILE *f = (FILE*) h->p;
	return ftell(f);
}

static void zoo_io_file_close(zoo_io_handle *h) {
	FILE *f = (FILE*) h->p;
	fclose(f);
}

static zoo_io_handle zoo_io_open_file_posix(zoo_io_path_driver *drv, const char *name, zoo_io_mode mode) {
	FILE *file;
	zoo_io_handle h;

	file = fopen(name, mode == MODE_WRITE ? "wb" : "rb");
	if (file == NULL) {
		return zoo_io_open_file_empty();
	}

	h.p = file;
	h.func_getptr = NULL;
	h.func_getc = zoo_io_file_getc;
	h.func_putc = zoo_io_file_putc;
	h.func_read = zoo_io_file_read;
	h.func_write = zoo_io_file_write;
	h.func_skip = zoo_io_file_skip;
	h.func_tell = zoo_io_file_tell;
	h.func_close = zoo_io_file_close;
	return h;
}

static inline int64_t zoo_io_get_mtime(const char *basename, const char *name) {
	struct stat statinfo;
	char path[ZOO_PATH_MAX + 1];

	strncpy(path, basename, ZOO_PATH_MAX);
	zoo_path_cat(path, basename, ZOO_PATH_MAX);
	stat(path, &statinfo);
	return statinfo.st_mtime;
}

static bool zoo_io_scan_dir_posix(zoo_io_path_driver *drv, const char *name, zoo_func_io_scan_dir_callback cb, void *cb_arg) {
	DIR *dir;
	struct dirent *dent;
	zoo_io_dirent ent;

	dir = opendir(name);
	if (dir == NULL) {
		return false;
	}

	while ((dent = readdir(dir)) != NULL) {
		strncpy(ent.name, dent->d_name, ZOO_PATH_MAX);
#if !defined(_DIRENT_HAVE_D_TYPE)
		// TODO: unhackify
		FILE *test_file = fopen(dent->d_name, "rb");
		if (test_file != NULL) {
			ent.type = TYPE_FILE;
			fclose(test_file);
		} else {
			ent.type = TYPE_DIR;
		}
#elif defined(_PSP_FW_VERSION)
		// TODO: unhackify
		ent.type = FIO_SO_ISDIR(dent->d_stat.st_mode) ? TYPE_DIR : TYPE_FILE;
#else
		ent.type = dent->d_type == DT_DIR ? TYPE_DIR : TYPE_FILE;
#endif
		if (ent.type == TYPE_FILE) {
			ent.mtime = zoo_io_get_mtime(name, ent.name);
		}

		if (!cb(drv, &ent, cb_arg)) {
			break;
		}
	}

	closedir(dir);
	return true;
}

void zoo_io_create_posix_driver(zoo_io_path_driver *drv) {
	zoo_io_internal_init_path_driver(drv);

	if (getcwd(drv->path, ZOO_PATH_MAX) == NULL) {
		strncpy(drv->path, "/", ZOO_PATH_MAX);
	}

	drv->func_open_file_absolute = zoo_io_open_file_posix;
	drv->func_dir_scan = zoo_io_scan_dir_posix;
}
