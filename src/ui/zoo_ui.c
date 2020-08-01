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

// game operations - SAVE WORLD

static zoo_tick_retval zoo_ui_save_world_cb(zoo_ui_state *state, const char *filename, bool accepted) {
	char full_filename[ZOO_PATH_MAX + 1];
	zoo_io_handle h;
	int ret;

	if (!accepted) return;

	strncpy(full_filename, filename, ZOO_PATH_MAX);
	strncat(full_filename, ".SAV", ZOO_PATH_MAX);

	// TODO: warn for long filenames (> 20 chars, minus extension);
	h = state->zoo->d_io->func_open_file(state->zoo->d_io, full_filename, MODE_WRITE);
	ret = zoo_world_save(state->zoo, &h);
	if (ret) {
		// TODO: I/O error message
	}
	h.func_close(&h);

	return RETURN_IMMEDIATE;
}

void zoo_ui_save_world(zoo_ui_state *state) {
	zoo_ui_popup_prompt_string(state, ZOO_UI_PROMPT_ANY, 3, 18, 50, "Save name? (.SAV)", state->zoo->world.info.name, zoo_ui_save_world_cb);
}

// main menu

static zoo_tick_retval zoo_ui_main_menu_cb(zoo_state *zoo, zoo_ui_state *cb_state) {
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
	} else if (!strcmp(hyperlink, "save")) {
		zoo_ui_save_world(cb_state);
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
	
	if (state->zoo->game_state != GS_PLAY) {
		zoo_window_append(&state->window, "!play;Play world");
	}

	if (state->zoo->d_io != NULL) {
		zoo_window_append(&state->window, "!load;Load world");
		if (!(state->zoo->d_io->read_only)) {
			if (state->zoo->game_state == GS_PLAY) {
				zoo_window_append(&state->window, "!save;Save world");
			}
			zoo_window_append(&state->window, "!restore;Restore world");
		}
	}

	if (state->zoo->game_state == GS_PLAY) {
		zoo_window_append(&state->window, "!quit;Quit world");
	}

#ifdef ZOO_DEBUG_MENU
	zoo_window_append(&state->window, "!zoo_debug;Debug options");
#endif

	zoo_call_push_callback(&(state->zoo->call_stack), (zoo_func_callback) zoo_ui_main_menu_cb, state);
	zoo_window_open(state->zoo, &state->window);
}

// state definitions

void zoo_ui_init(zoo_ui_state *state, zoo_state *zoo) {
	memset(state, 0, sizeof(zoo_ui_state));
	state->zoo = zoo;
	zoo_ui_input_init(&state->input);
}

void zoo_ui_tick(zoo_ui_state *state) {
	uint16_t key;
	bool in_game = state->zoo->game_state == GS_PLAY;

	if (!zoo_call_empty(&state->zoo->call_stack)) {
		return;
	}

	while ((key = zoo_ui_input_key_pop(&state->input)) != 0) {
		if (key & ZOO_KEY_RELEASED) continue;

		// TODO: temporary
		if (key == ZOO_KEY_F1) {
			zoo_ui_main_menu(state);
		}
	}
}