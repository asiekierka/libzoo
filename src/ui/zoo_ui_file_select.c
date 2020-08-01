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
#include "zoo_ui_internal.h"

static bool zoo_ui_filesel_dir_scan_cb(zoo_io_path_driver *drv, zoo_io_dirent *e, void *cb_arg) {
	zoo_ui_state *state = (zoo_ui_state *) cb_arg;
	char *dirext;

	if (e->type == TYPE_DIR) {
		// TODO: add directory support
		return true;
	}

	if (state->filesel_extension[0] != '\0') {
		dirext = strrchr(e->name, '.');
		if (dirext == NULL || strcasecmp(state->filesel_extension, dirext)) {
			return true;
		}
		zoo_window_append(&state->window, e->name);
	} else {
		zoo_window_append(&state->window, e->name);
	}
	return true;
}

void zoo_ui_filesel_call(zoo_ui_state *state, const char *title, const char *extension, zoo_func_callback cb) {
	// TODO: add directory support
	zoo_io_path_driver *d_io = (zoo_io_path_driver *) state->zoo->d_io;

	zoo_ui_init_select_window(state, title);
	strcpy(state->filesel_extension, extension != NULL ? extension : "");
	d_io->func_dir_scan(d_io, d_io->path, zoo_ui_filesel_dir_scan_cb, state);
	zoo_call_push_callback(&(state->zoo->call_stack), cb, state);
	zoo_window_open(state->zoo, &state->window);
}