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
#include "zoo_ui.h"

// utility methods

void zoo_ui_init_select_window(zoo_ui_state *state, const char *title) {
	memset(&state->window, 0, sizeof(zoo_text_window));
	strcpy(state->window.title, title);
	state->window.manual_close = true;
	state->window.hyperlink_as_select = true;
}

// UI primitives - FILE SELECT

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

// game operations - LOAD WORLD

static zoo_tick_retval zoo_ui_load_world_cb(zoo_state *zoo, zoo_ui_state *cb_state) {
	char *name = zoo_window_line_selected(&(cb_state->window));
	bool as_save = !strcmp(cb_state->filesel_extension, ".SAV");
	zoo_io_handle h;
	int ret;

	if (cb_state->window.accepted && name != NULL) {
		// TODO: warn for long filenames (> 20 chars, minus extension);
		h = zoo->d_io->func_open_file(zoo->d_io, name, MODE_READ);
		ret = zoo_world_load(zoo, &h, false);
		if (!ret) {
			if (as_save) {
				zoo_game_start(zoo, GS_PLAY);
			} else {
				zoo_board_change(zoo, 0);
				zoo_game_start(zoo, GS_TITLE);
			}
			zoo_redraw(zoo);
		} else {
			// TODO: I/O error message
		}
		h.func_close(&h);
	}

	zoo_window_close(&cb_state->window);
	return EXIT;
}

void zoo_ui_load_world(zoo_ui_state *state, bool as_save) {
	zoo_ui_filesel_call(state, as_save ? "Saved Games" : "ZZT Worlds", as_save ? ".SAV" : ".ZZT", (zoo_func_callback) zoo_ui_load_world_cb);
}

#ifdef ZOO_DEBUG_MENU
#endif

static zoo_tick_retval zoo_ui_debug_menu_cb(zoo_state *zoo, zoo_ui_state *cb_state) {
	char hyperlink[21];
	strncpy(hyperlink, cb_state->window.hyperlink, sizeof(hyperlink));

	zoo_window_close(&cb_state->window);
	zoo_call_pop(&zoo->call_stack);

	if (!strcmp(hyperlink, "play")) {
		if (cb_state->zoo->game_state == GS_TITLE)  {
			zoo_world_play(zoo);
		}
	} else if (!strcmp(hyperlink, "load")) {
		zoo_ui_load_world(cb_state, false);
	} else if (!strcmp(hyperlink, "restore")) {
		zoo_ui_load_world(cb_state, true);
	} else if (!strcmp(hyperlink, "quit"))  {
		if (cb_state->zoo->game_state == GS_PLAY)  {
			zoo_world_return_title(zoo);
		}
#ifdef ZOO_DEBUG_MENU
	} else if (!strcmp(hyperlink, "zoo_debug")) {
		zoo_ui_debug_menu(cb_state);
#endif
	}

	return RETURN_IMMEDIATE;
}

void zoo_ui_main_menu(struct s_zoo_ui_state *state) {
	zoo_ui_init_select_window(state, "Main Menu");
	
#ifdef ZOO_DEBUG_MENU
	zoo_window_append(&state->window, "!zoo_debug;Debug options");
#endif

	if (state->zoo->game_state != GS_PLAY) {
		zoo_window_append(&state->window, "!play;Play world");
	}

	if (state->zoo->d_io != NULL) {
		zoo_window_append(&state->window, "!load;Load world");
		if (!(state->zoo->d_io->read_only)) {
			zoo_window_append(&state->window, "!restore;Restore world");
		}
	}

	if (state->zoo->game_state == GS_PLAY) {
		zoo_window_append(&state->window, "!quit;Quit world");
	}

	zoo_call_push_callback(&(state->zoo->call_stack), (zoo_func_callback) zoo_ui_debug_menu_cb, state);
	zoo_window_open(state->zoo, &state->window);
}

// state definitions

void zoo_ui_init(zoo_ui_state *state, zoo_state *zoo) {
	memset(state, 0, sizeof(zoo_ui_state));
	state->zoo = zoo;
}
