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
#include "zoo.h"

static int16_t window_x = 5;
static int16_t window_y = 3;
static int16_t window_width = 49;
static int16_t window_height = 19;

void zoo_window_classic_set_position(int16_t x, int16_t y, int16_t width, int16_t height) {
	window_x = x;
	window_y = y;
	window_width = width;
	window_height = height;
}

static void zoo_window_classic_append(zoo_text_window *window, const char *text) {
	char *buffer;
	int16_t buflen = strlen(text);
	if (buflen > 50) buflen = 50;

	buffer = malloc(sizeof(char) * (buflen + 1));
	memcpy(buffer, text, buflen);
	buffer[buflen] = '\0';

	window->lines = realloc(window->lines, sizeof(char*) * (window->line_count + 1));
	window->lines[(window->line_count)++] = buffer;
}

static void zoo_window_classic_close(zoo_text_window *window) {
	int16_t i;

	for (i = 0; i < window->line_count; i++) {
		free(window->lines[i]);
	}
	free(window->lines);
}

static void zoo_window_draw_title(zoo_text_window *window, zoo_state *state, uint8_t color, const char *title) {
	int16_t i;
	int16_t il = strlen(title);
	int16_t is = window_x + ((window_width - il) >> 1);
	for (i = window_x + 2; i < (window_x + window_width - 2); i++) {
		if (i >= is && i < (is+il)) {
			state->func_write_char(i, window_y + 1, color, title[i - is]);
		} else {
			state->func_write_char(i, window_y + 1, color, ' ');
		}
	}
}

static void zoo_window_draw_open(zoo_text_window *window, zoo_state *state) {
	int16_t ix, iy;

	// TODO: transitions
	window->screen_copy = state->func_store_display(state);

	// inner box
	for (iy = 1; iy < window_height - 1; iy++) {
		if (iy == 2) continue;
		for (ix = 2; ix < window_width - 2; ix++) {
			state->func_write_char(window_x + ix, window_y + iy, 0x1F, ' ');
		}
	}

	// outline
	for (ix = 2; ix < window_width - 2; ix++) {
		state->func_write_char(window_x + ix, window_y, 0x0F, '\xCD');
		state->func_write_char(window_x + ix, window_y + 2, 0x0F, '\xCD');
		state->func_write_char(window_x + ix, window_y + window_height - 1, 0x0F, '\xCD');
	}

	for (iy = 1; iy < window_height - 1; iy++) {
		state->func_write_char(window_x, window_y + iy, 0x0F, ' ');
		if (iy == 2) {
			state->func_write_char(window_x + 1, window_y + iy, 0x0F, '\xC6');
			state->func_write_char(window_x + window_width - 2, window_y + iy, 0x0F, '\xB5');
		} else {
			state->func_write_char(window_x + 1, window_y + iy, 0x0F, '\xB3');
			state->func_write_char(window_x + window_width - 2, window_y + iy, 0x0F, '\xB3');
		}
		state->func_write_char(window_x + window_width - 1, window_y + iy, 0x0F, ' ');
	}

	state->func_write_char(window_x, window_y, 0x0F, '\xC6');
	state->func_write_char(window_x, window_y + window_height - 1, 0x0F, '\xC6');
	state->func_write_char(window_x + window_width - 1, window_y, 0x0F, '\xB5');
	state->func_write_char(window_x + window_width - 1, window_y + window_height - 1, 0x0F, '\xB5');
	state->func_write_char(window_x + 1, window_y, 0x0F, '\xD1');
	state->func_write_char(window_x + window_width - 2, window_y, 0x0F, '\xD1');
	state->func_write_char(window_x + 1, window_y + window_height - 1, 0x0F, '\xCF');
	state->func_write_char(window_x + window_width - 2, window_y + window_height - 1, 0x0F, '\xCF');

	// called in zoo_window_draw_text
	// zoo_window_draw_title(window, state, 0x1E, window->title);
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
	state->func_write_char(window_x + 2, line_y, 0x1C, same_line ? '\xAF' : ' ');
	state->func_write_char(window_x + window_width - 3, line_y, 0x1C, same_line ? '\xAE' : ' ');

	text_offset = 0;
	text_color = 0x1E;
	text_x = 0;
	text_width = (window_width - 7);

	if (line_pos >= 0 && line_pos < window->line_count) {
		str = window->lines[line_pos];

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
				text_x = (text_width - strlen(str)) / 2;
				break;
		}
	} else if (window->viewing_file) {
		// TODO: zoo_draw_string refactor?
	}

	if (str != NULL) {
		for (i = -text_x; i < (text_width - text_x); i++) {
			if (draw_arrow && i == -3) {
 				state->func_write_char(window_x + 4 + i + text_x, line_y, 0x1D, '\x10');
			} else {
				state->func_write_char(window_x + 4 + i + text_x, line_y, text_color,
					(i >= 0 && i < strlen(str)) ? str[i] : ' ');
			}
		}
	} else {
		is_boundary = line_pos == -1 || line_pos == window->line_count;
		for (i = 0; i < (window_width - 4); i++) {
			state->func_write_char(window_x + 2 + i, line_y, text_color,
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
	// TODO: transitions
	state->func_restore_display(state, window->screen_copy);
}

static void zoo_window_hyperlink(zoo_text_window *window, zoo_state *state) {
	char *str;
	char pointer_str[21];
	char pointer_label[21];
	int16_t i;

	str = window->lines[window->line_pos];
	for (i = 0; i < sizeof(pointer_str); i++) {
		pointer_str[i] = str[i + 1];
		if (pointer_str[i] == '\0' || pointer_str[i] == ';' || i == (sizeof(pointer_str) - 1)) {
			pointer_str[i] = '\0';
			break;
		}
	}

	if (pointer_str[0] == '-') {
		// TODO: files
	} else {
		if (window->hyperlink_as_select) {
			strncpy(window->hyperlink, pointer_str, sizeof(window->hyperlink));
		} else {
			pointer_label[0] = ':';
			strncpy(pointer_label + 1, pointer_str, sizeof(pointer_label) - 2);
			// TODO: in-document label jumps
		}
	}
}

static zoo_tick_retval zoo_window_classic_tick(zoo_text_window *window, zoo_state *state) {
	int16_t old_line_pos = window->line_pos;

	if (window->state == 0) {
		zoo_window_draw_open(window, state);
		zoo_window_draw_text(window, state, false);
		window->state = 1;
	} else if (window->state == 1) {
		window->line_pos += state->input.delta_y;
		window->line_pos += state->input.delta_x * 4;
		if (window->line_pos < 0) window->line_pos = 0;
		if (window->line_pos >= window->line_count) window->line_pos = window->line_count - 1;

		if (old_line_pos != window->line_pos) {
			zoo_window_draw_text(window, state, false);
		}

		if (state->input.shoot) {
			if (window->lines[window->line_pos][0] == '!') {
				zoo_window_hyperlink(window, state);
			}
			zoo_window_draw_close(window, state);
			return EXIT;
		}
	}

	return RETURN_NEXT_CYCLE;
}

void zoo_window_classic_init(zoo_text_window *window) {
	memset(window, 0, sizeof(zoo_text_window));

	window->func_append = zoo_window_classic_append;
	window->func_close = zoo_window_classic_close;
	window->func_tick = zoo_window_classic_tick;
}
