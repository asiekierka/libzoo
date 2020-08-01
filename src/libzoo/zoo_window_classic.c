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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "zoo_internal.h"

#define WINDOW_STATE_START 0
#define WINDOW_STATE_ANIM_OPEN 1
#define WINDOW_STATE_TICK 2
#define WINDOW_STATE_ANIM_CLOSE 3

static int16_t window_x = 5;
static int16_t window_y = 3;
static int16_t window_width = 49;
static int16_t window_height = 19;

#define WINDOW_ANIM_MAX (window_height >> 1)

void zoo_window_set_position(int16_t x, int16_t y, int16_t width, int16_t height) {
	window_x = x;
	window_y = y;
	window_width = width;
	window_height = height;
}

static void zoo_window_draw_title(zoo_text_window *window, zoo_state *state, uint8_t color, const char *title) {
	int16_t i;
	int16_t il = strlen(title);
	int16_t is = window_x + ((window_width + 1 - il) >> 1);

	for (i = window_x + 2; i < (window_x + window_width - 2); i++) {
		if (i >= is && i < (is+il)) {
			state->d_video->func_write(state->d_video, i, window_y + 1, color, title[i - is]);
		} else {
			state->d_video->func_write(state->d_video, i, window_y + 1, color, ' ');
		}
	}
}

static const char draw_patterns[4][5] = {
	{'\xC6', '\xD1', '\xCD', '\xD1', '\xB5'}, // top
	{'\xC6', '\xCF', '\xCD', '\xCF', '\xB5'}, // bottom
	{' ', '\xB3', ' ', '\xB3', ' '}, // inner
	{' ', '\xC6', '\xCD', '\xB5', ' '} // separator
};

#define PAT_TOP 0
#define PAT_BOTTOM 1
#define PAT_INNER 2
#define PAT_SEP 3

static void zoo_window_draw_border(zoo_text_window *window, zoo_state *state, int16_t y, const char *pattern) {
	int16_t ix;

	state->d_video->func_write(state->d_video, window_x, y, 0x0F, pattern[0]);
	state->d_video->func_write(state->d_video, window_x + 1, y, 0x0F, pattern[1]);
	for (ix = 2; ix < window_width - 2; ix++) {
		state->d_video->func_write(state->d_video, window_x + ix, y, 0x0F, pattern[2]);
	}
	state->d_video->func_write(state->d_video, window_x + window_width - 2, y, 0x0F, pattern[3]);
	state->d_video->func_write(state->d_video, window_x + window_width - 1, y, 0x0F, pattern[4]);
}

static void zoo_window_draw_open(zoo_text_window *window, zoo_state *state) {
	int16_t ix;
	int16_t y0 = window_y + window->counter;
	int16_t y1 = window_y + window->counter + 1;
	int16_t y2 = window_y + window_height - window->counter - 2;
	int16_t y3 = window_y + window_height - window->counter - 1;

	zoo_window_draw_border(window, state, y0, draw_patterns[PAT_TOP]);
	zoo_window_draw_border(window, state, y1, draw_patterns[PAT_INNER]);
	zoo_window_draw_border(window, state, y2, draw_patterns[PAT_INNER]);
	zoo_window_draw_border(window, state, y3, draw_patterns[PAT_BOTTOM]);
}

static void zoo_window_draw_open_finish(zoo_text_window *window, zoo_state *state) {
	zoo_window_draw_border(window, state, window_y + 2, draw_patterns[PAT_SEP]);
	zoo_window_draw_title(window, state, 0x1E, window->title);
}

static void zoo_window_draw_open_all(zoo_text_window *window, zoo_state *state) {
	int16_t iy;

	zoo_window_draw_border(window, state, window_y, draw_patterns[PAT_TOP]);
	for (iy = 1; iy < window_height - 1; iy++) {
		zoo_window_draw_border(window, state, window_y + iy, draw_patterns[iy == 2 ? PAT_SEP : PAT_INNER]);	
	}
	zoo_window_draw_border(window, state, window_y + window_height - 1, draw_patterns[PAT_BOTTOM]);
}

static void zoo_window_draw_line(zoo_text_window *window, zoo_state *state, int16_t line_pos, bool no_formatting) {
	int line_y;
	int text_x, text_width;
	int text_offset, text_color;
	int i;
	bool same_line;
	bool is_boundary, draw_arrow = false;
	char *str = NULL;
	char *tmp = NULL;

	line_y = (window_y + line_pos - window->line_pos) + (window_height >> 1) + 1;
	same_line = line_pos == window->line_pos;
	state->d_video->func_write(state->d_video, window_x + 2, line_y, 0x1C, same_line ? '\xAF' : ' ');
	state->d_video->func_write(state->d_video, window_x + window_width - 3, line_y, 0x1C, same_line ? '\xAE' : ' ');

	text_offset = 0;
	text_color = 0x1E;
	text_x = 0;
	text_width = (window_width - 7);

	str = zoo_window_line_at(window, line_pos);
	if (str != NULL) {
		switch (str[0]) {
			case '!':
				tmp = strchr(str, ';');
				if (tmp != NULL) str = tmp + 1;
				draw_arrow = true;
				text_x += 5;
				text_color = 0x1F;
				break;
			case ':':
				tmp = strchr(str, ';');
				if (tmp != NULL) str = tmp + 1;
				text_color = 0x1F;
				break;
			case '$':
				str++;
				text_color = 0x1F;
				// (window_width - 8 - strlen(str)) / 2
				text_x = (text_width - 1 - strlen(str)) >> 1;
				break;
		}
	} else if (window->viewing_file) {
		// TODO: zoo_draw_string refactor?
	}

	if (str != NULL) {
		for (i = -text_x - 1; i < (text_width - text_x); i++) {
			if (draw_arrow && i == -3) {
 				state->d_video->func_write(state->d_video, window_x + 4 + i + text_x, line_y, 0x1D, '\x10');
			} else {
				state->d_video->func_write(state->d_video, window_x + 4 + i + text_x, line_y, text_color,
					(i >= 0 && i < strlen(str)) ? str[i] : ' ');
			}
		}
	} else {
		is_boundary = line_pos == -1 || line_pos == window->line_count;
		for (i = 0; i < (window_width - 4); i++) {
			state->d_video->func_write(state->d_video, window_x + 2 + i, line_y, text_color,
				(is_boundary && ((i % 5) == 4)) ? '\x07' : ' ');
		}
	}

}

static void zoo_window_draw_text(zoo_text_window *window, zoo_state *state, bool no_formatting) {
	int i;
	for (i = 0; i < window_height - 4; i++) {
		zoo_window_draw_line(window, state, window->line_pos - (window_height >> 1) + i + 2, no_formatting);
	}
	zoo_window_draw_title(window, state, 0x1E, window->title);
}

static void zoo_window_draw_close(zoo_text_window *window, zoo_state *state) {
	int16_t ix, iy;
	int16_t y0, y1;

	// draw line at counter + 1
	iy = window->counter + 1;
	if (iy <= WINDOW_ANIM_MAX) {
		y0 = window_y + iy;
		y1 = window_y + window_height - iy - 1;

		zoo_window_draw_border(window, state, y0, draw_patterns[PAT_TOP]);
		zoo_window_draw_border(window, state, y1, draw_patterns[PAT_BOTTOM]);
	}

	// restore line at counter
	iy = window->counter;
	zoo_restore_display(state, window->screen_copy, window_width, window_height,
		0, iy, window_width, 1, window_x, window_y + iy);
	zoo_restore_display(state, window->screen_copy, window_width, window_height,
		0, window_height - iy - 1, window_width, 1, window_x, window_y + window_height - iy - 1);
}

// if TRUE, close the window
static bool zoo_window_hyperlink(zoo_text_window *window, zoo_state *state, char *str) {
	char pointer_str[21];
	char pointer_label[21];
	int16_t i;

	for (i = 0; i < sizeof(pointer_str); i++) {
		pointer_str[i] = str[i + 1];
		if (pointer_str[i] == '\0' || pointer_str[i] == ';' || i == (sizeof(pointer_str) - 1)) {
			pointer_str[i] = '\0';
			break;
		}
	}

	if (pointer_str[0] == '-') {
		zoo_window_close(window);
		if (state->d_io != NULL) {
			if (zoo_window_open_file(state->d_io, window, pointer_str + 1)) {
				window->viewing_file = true;
				zoo_window_draw_text(window, state, false);
				return false;
			}
		}
	} else {
		if (window->hyperlink_as_select) {
			strncpy(window->hyperlink, pointer_str, sizeof(window->hyperlink));
		} else {
			pointer_label[0] = ':';
			strncpy(pointer_label + 1, pointer_str, sizeof(pointer_label) - 2);
			// TODO: in-document label jumps
		}
	}
	return true;
}

static zoo_tick_retval zoo_window_classic_tick(zoo_state *state, zoo_text_window *window) {
	int16_t old_line_pos = window->line_pos;
	bool act_ok, act_cancel, should_close;
	char *curr_str;

	switch (window->state) {
		case WINDOW_STATE_START:
			window->screen_copy = zoo_store_display(state, window_x, window_y, window_width, window_height);
			window->counter = WINDOW_ANIM_MAX;
			window->state = WINDOW_STATE_ANIM_OPEN;
			if (window->disable_transitions) {
				zoo_window_draw_open_all(window, state);
				goto FinishOpenWindow;
			}
			// fall through
		case WINDOW_STATE_ANIM_OPEN:
			zoo_window_draw_open(window, state);
			if ((--window->counter) < 0) {
				zoo_window_draw_open_finish(window, state);
			FinishOpenWindow:
				zoo_window_draw_text(window, state, false);
				window->state = WINDOW_STATE_TICK;
			} else {
				return RETURN_NEXT_FRAME;
			}
			break;
		case WINDOW_STATE_TICK:
			zoo_input_update(&state->input);

			if (zoo_input_action_pressed(&state->input, ZOO_ACTION_LEFT)) window->line_pos -= 4;
			else if (zoo_input_action_pressed(&state->input, ZOO_ACTION_UP)) window->line_pos--;

			if (zoo_input_action_pressed(&state->input, ZOO_ACTION_RIGHT)) window->line_pos += 4;
			else if (zoo_input_action_pressed(&state->input, ZOO_ACTION_DOWN)) window->line_pos++;

			if (window->line_pos < 0) window->line_pos = 0;
			if (window->line_pos >= window->line_count) window->line_pos = window->line_count - 1;

			if (old_line_pos != window->line_pos) {
				zoo_window_draw_text(window, state, false);
			}

			act_ok = zoo_input_action_pressed(&state->input, ZOO_ACTION_OK);
			act_cancel = zoo_input_action_pressed(&state->input, ZOO_ACTION_CANCEL);
			if (act_ok || act_cancel) {
				window->state = WINDOW_STATE_ANIM_CLOSE;
				window->accepted = act_ok;
				if (window->disable_transitions) {
					zoo_restore_display(state, window->screen_copy, window_width, window_height, 0, 0, window_width, window_height, window_x, window_y);
					goto CloseWindow;
				} else {
					window->counter = 0;
				}
			}
			break;
		case WINDOW_STATE_ANIM_CLOSE:
			zoo_window_draw_close(window, state);
			if ((++window->counter) > WINDOW_ANIM_MAX) {
				CloseWindow:
				should_close = true;
				if (window->accepted) {
					curr_str = zoo_window_line_selected(window);
					if (curr_str != NULL && curr_str[0] == '!') {
						should_close = zoo_window_hyperlink(window, state, curr_str);
					}
				}
				if (should_close) {
					if (!window->manual_close) {
						zoo_window_close(window);
					}
					return EXIT;
				} else {
					window->state = WINDOW_STATE_START;
					zoo_input_clear(&state->input);
					return RETURN_IMMEDIATE;
				}
			} else {
				return RETURN_NEXT_FRAME;
			} 
			break;
	}

	return RETURN_NEXT_FRAME;
}

void zoo_window_open(zoo_state *state, zoo_text_window *window) {
	window->state = 0;
	window->counter = 0;

	zoo_call_push_callback(&(state->call_stack), (zoo_func_callback) zoo_window_classic_tick, window);
}
