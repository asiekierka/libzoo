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

static void zoo_ui_io_error(void) {
	// TODO
}

#ifdef ZOO_CONFIG_ENABLE_FILE_IO
typedef struct {
	zoo_text_window window;
	bool as_save;
} zoo_ui_file_select_state;

static zoo_ui_file_select_state filesel_state;

static bool zoo_ui_populate_file(zoo_io_dirent *e, void *cb_arg) {
	zoo_ui_file_select_state *state = (zoo_ui_file_select_state *) cb_arg;
	const char *dirext;
	const char *dirext_cmp = state->as_save ? ".SAV" : ".ZZT";

	if (e->type == TYPE_DIR) {
		// TODO: add directory support
		return true;
	}

	if (dirext_cmp != NULL) {
		dirext = strrchr(e->name, '.');
		if (dirext == NULL || strcasecmp(dirext_cmp, dirext)) {
			return true;
		}
	}

	zoo_window_append(&(state->window), e->name);
	return true;
}

static zoo_tick_retval zoo_ui_load_world_cb(zoo_state *state, zoo_ui_file_select_state *cb_state) {
	char path[ZOO_PATH_MAX + 1];
	char *name = zoo_window_line_selected(&(cb_state->window));
	zoo_io_handle h;

	if (cb_state->window.accepted && name != NULL) {
		// TODO: warn for long filenames (> 20 chars, minus extension);
		strncpy(path, state->io.path, ZOO_PATH_MAX);
		zoo_path_cat(path, name, ZOO_PATH_MAX);
		h = state->io.func_io_open_file(path, MODE_READ);
		if (zoo_world_load(state, &h, false)) {
			if (filesel_state.as_save) {
				zoo_game_start(state, GS_PLAY);
			} else {
				zoo_board_change(state, 0);
				zoo_game_start(state, GS_TITLE);
			}
			zoo_redraw(state);
		} else {
			zoo_ui_io_error();
		}
		h.func_close(&h);
	}

	zoo_window_close(&cb_state->window);
	return EXIT;
}

void zoo_ui_load_world(zoo_state *state, bool as_save) {
	// TODO: add directory support
	memset(&filesel_state, 0, sizeof(filesel_state));
	strcpy(filesel_state.window.title, as_save ? "Saved Games" : "ZZT Worlds");
	filesel_state.window.manual_close = true;
	filesel_state.window.hyperlink_as_select = true;
	filesel_state.as_save = as_save;
	state->io.func_io_scan_dir(state->io.path, zoo_ui_populate_file, &filesel_state);
	zoo_call_push_callback(&(state->call_stack), (zoo_func_callback) zoo_ui_load_world_cb, &filesel_state);
	state->func_ui_open_window(state, &filesel_state.window);
}
#endif

void zoo_ui_play(zoo_state *state) {
#ifdef ZOO_CONFIG_ENABLE_FILE_IO
	char path[ZOO_PATH_MAX + 1];
	zoo_io_handle h;
#endif
	bool ret = true;

#ifdef ZOO_CONFIG_ENABLE_FILE_IO
	if (state->world.info.is_save) {
		zoo_io_translate(&state->io, state->world.info.name, ".ZZT", path, ZOO_PATH_MAX);
		h = state->io.func_io_open_file(path, MODE_READ);
		ret = zoo_world_load(state, &h, false);
		state->return_board_id = state->world.info.current_board;
	}
#endif

	if (ret) {
		zoo_game_start(state, GS_PLAY);

		zoo_board_change(state, state->return_board_id);
		zoo_board_enter(state);

		zoo_redraw(state);
	} else {
		zoo_ui_io_error();
	}
}
