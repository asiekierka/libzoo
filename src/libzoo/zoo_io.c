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

#include <stdlib.h>
#include <string.h>
#include "zoo_internal.h"

static uint8_t zoo_io_mem_getc(zoo_io_handle *h) {
	uint8_t **ptr = (uint8_t **) &h->p;
	if (h->len <= 0) return 0;
	h->len--;
	return *((*ptr)++);
}

static size_t zoo_io_mem_putc(zoo_io_handle *h, uint8_t v) {
	uint8_t **ptr = (uint8_t **) &h->p;
	if (h->len <= 0) return 0;
	h->len--;
	*((*ptr)++) = v;
	return 1;
}

static size_t zoo_io_mem_read(zoo_io_handle *h, uint8_t *d_ptr, size_t len) {
	uint8_t **ptr = (uint8_t **) &h->p;
	if (len > h->len) len = h->len;
	if (len <= 0) return 0;
	h->len -= len;
	memcpy(d_ptr, *ptr, len);
	*ptr += len;
	return len;
}

static size_t zoo_io_mem_write(zoo_io_handle *h, const uint8_t *d_ptr, size_t len) {
	uint8_t **ptr = (uint8_t **) &h->p;
	if (len > h->len) len = h->len;
	if (len <= 0) return 0;
	h->len -= len;
	memcpy(*ptr, d_ptr, len);
	*ptr += len;
	return len;
}

static size_t zoo_io_mem_putc_ro(zoo_io_handle *h, uint8_t v) {
	return 0;
}

static size_t zoo_io_mem_write_ro(zoo_io_handle *h, const uint8_t *d_ptr, size_t len) {
	return 0;
}

static size_t zoo_io_mem_skip(zoo_io_handle *h, size_t len) {
	uint8_t **ptr = (uint8_t **) &h->p;
	if (len > h->len) len = h->len;
	h->len -= len;
	*ptr += len;
	return len;
}

static size_t zoo_io_mem_tell(zoo_io_handle *h) {
	return h->len_orig - h->len;
}

static void zoo_io_mem_close(zoo_io_handle *h) {

}

static uint8_t *zoo_io_mem_getptr(zoo_io_handle *h) {
	return h->p;
}

zoo_io_handle zoo_io_open_file_mem(uint8_t *ptr, size_t len, zoo_io_mode mode) {
	zoo_io_handle h;
	h.p = ptr;
	h.len = len;
	h.len_orig = len;
	h.func_getptr = zoo_io_mem_getptr;
	h.func_getc = zoo_io_mem_getc;
	h.func_putc = (mode == MODE_WRITE) ? zoo_io_mem_putc : zoo_io_mem_putc_ro;
	h.func_read = zoo_io_mem_read;
	h.func_write = (mode == MODE_WRITE) ? zoo_io_mem_write : zoo_io_mem_write_ro;
	h.func_skip = zoo_io_mem_skip;
	h.func_tell = zoo_io_mem_tell;
	h.func_close = zoo_io_mem_close;
	return h;
}