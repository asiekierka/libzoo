/**
 * Copyright (c) 2020 Adrian Siekierka
 *
 * Based on a reconstruction of code from ZZT,
 * Copyright 1991 Epic MegaGames, used with permission.
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

#include <stdlib.h>
#include <string.h>
#include "zoo.h"

#ifdef ZOO_CONFIG_ENABLE_FILE_IO_POSIX
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>

static uint8_t zoo_io_file_getc(zoo_io_handle *h) {
	FILE *f = (FILE*) h->p;
	// TODO: EOF check
	return fgetc(f);
}

static size_t zoo_io_file_putc(zoo_io_handle *h, uint8_t v) {
	FILE *f = (FILE*) h->p;
	// TODO: EOF check
	fputc(v, f);
	return 1;
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

static zoo_io_handle zoo_io_open_file_posix(const char *name, zoo_io_mode mode) {
	FILE *file;
	zoo_io_handle h;

	file = fopen(name, mode == MODE_WRITE ? "wb" : "rb");
	if (file == NULL) {
		return zoo_io_open_file_mem(NULL, 0, false);
	}

	h.p = file;
	h.func_getc = zoo_io_file_getc;
	h.func_putc = zoo_io_file_putc;
	h.func_read = zoo_io_file_read;
	h.func_write = zoo_io_file_write;
	h.func_skip = zoo_io_file_skip;
	h.func_tell = zoo_io_file_tell;
	h.func_close = zoo_io_file_close;
	return h;
}

static bool zoo_io_scan_dir_posix(const char *name, zoo_func_io_scan_dir_callback cb, void *cb_arg) {
	DIR *dir;
	struct dirent *dent;
	zoo_io_dirent ent;

	dir = opendir(name);
	if (dir == NULL) {
		return false;
	}

	while ((dent = readdir(dir)) != NULL) {
		strncpy(ent.name, dent->d_name, ZOO_PATH_MAX);
		ent.type = dent->d_type == DT_DIR ? TYPE_DIR : TYPE_FILE;
		if (!cb(&ent, cb_arg)) {
			break;
		}
	}

	closedir(dir);
	return true;
}

void zoo_io_install_posix(zoo_io_state *state) {
	if (getcwd(state->path, ZOO_PATH_MAX) == NULL) {
		strncpy(state->path, "/", ZOO_PATH_MAX);
	}

	state->func_io_open_file = zoo_io_open_file_posix;
	state->func_io_scan_dir = zoo_io_scan_dir_posix;
}
#endif
