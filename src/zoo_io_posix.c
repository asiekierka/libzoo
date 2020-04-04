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

#ifdef ZOO_CONFIG_ENABLE_POSIX_FILE_IO

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

zoo_io_handle zoo_io_open_file_posix(FILE *file) {
	zoo_io_handle h;
	h.p = file;
	h.func_getc = zoo_io_file_getc;
	h.func_putc = zoo_io_file_putc;
	h.func_read = zoo_io_file_read;
	h.func_write = zoo_io_file_write;
	h.func_skip = zoo_io_file_skip;
	h.func_tell = zoo_io_file_tell;
	return h;
}

#endif
