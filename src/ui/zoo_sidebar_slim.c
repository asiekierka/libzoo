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
#include "zoo_sidebar.h"

static void write_number(zoo_video_driver *d_video, int16_t x, int16_t y, uint8_t col, int val, int len) {
	char s[8];
	int16_t pos = sizeof(s);
	int16_t i = val < 0 ? -val : val;

	while (i >= 10) {
		s[--pos] = '0' + (i % 10);
		i /= 10;
	}
	s[--pos] = '0' + (i % 10);
	if (val < 0) {
		s[--pos] = '-';
	}

	for (i = pos; i < sizeof(s); i++) {
		d_video->func_write(d_video, x + i - pos, y, col, s[i]);
	}
	for (i = sizeof(s) - pos; i < len; i++) {
		d_video->func_write(d_video, x + i, y, col, ' ');
	}
}

static void write_number_torch_bg(zoo_state *state, int16_t x, int16_t y, uint8_t col, int val, int len) {
	char s[8];
	int16_t pos = sizeof(s);
	int16_t i = val < 0 ? -val : val;
	uint8_t torch_col;
	int16_t torch_pos;
	zoo_video_driver *d_video = state->d_video;

	while (i >= 10) {
		s[--pos] = '0' + (i % 10);
		i /= 10;
	}
	s[--pos] = '0' + (i % 10);
	if (val < 0) {
		s[--pos] = '-';
	}

	torch_col = (col & 0x0F) | 0x60;
	torch_pos = state->world.info.torch_ticks > 0
		? (state->world.info.torch_ticks + 39) / 40
		: 0;
	if (torch_pos > 5) torch_pos = 5;

	for (i = pos; i < sizeof(s); i++) {
		int16_t nx = x + i - pos;
		d_video->func_write(d_video, nx, y, nx < torch_pos ? torch_col : col, s[i]);
	}

	for (i = sizeof(s) - pos; i < torch_pos; i++) {
		d_video->func_write(d_video, x + i, y, torch_col, ' ');
	}
	for (; i < len; i++) {
		d_video->func_write(d_video, x + i, y, col, ' ');
	}
}

void zoo_draw_sidebar_slim(zoo_state *state, uint16_t flags) {
	int i;
	int x = 1;
	zoo_video_driver *d_video = state->d_video;

	if (state->game_state != GS_PLAY) {
		if (flags & ZOO_SIDEBAR_UPDATE_REDRAW) {
			for (i = 0; i < 60; i++) {
				d_video->func_write(d_video, i, 25, 0x0F, ' ');
			}
		}
		return;
	}

	// left-aligned

	if (flags & ZOO_SIDEBAR_UPDATE_HEALTH) {
		d_video->func_write(d_video, x, 25, 0x1C, '\x03');
		d_video->func_write(d_video, x + 1, 25, 0x1C, ' ');
		write_number(d_video, x + 2, 25, 0x1F, state->world.info.health, 6);
	}
	x += 8;

	if (flags & ZOO_SIDEBAR_UPDATE_AMMO) {
		d_video->func_write(d_video, x, 25, 0x1B, '\x84');
		d_video->func_write(d_video, x + 1, 25, 0x1B, ' ');
		write_number(d_video, x + 2, 25, 0x1F, state->world.info.ammo, 6);
	}
	x += 8;

	if (flags & ZOO_SIDEBAR_UPDATE_TORCHES) {
		d_video->func_write(d_video, x, 25, 0x1E, '\x9D');
		d_video->func_write(d_video, x + 1, 25, 0x1E, ' ');
		write_number_torch_bg(state, x + 2, 25, 0x1F, state->world.info.torches, 6);
	}
	x += 8;

	if (flags & ZOO_SIDEBAR_UPDATE_GEMS) {
		d_video->func_write(d_video, x, 25, 0x19, '\x04');
		d_video->func_write(d_video, x + 1, 25, 0x19, ' ');
		write_number(d_video, x + 2, 25, 0x1F, state->world.info.gems, 6);
	}
	x += 8;

	if (flags & ZOO_SIDEBAR_UPDATE_SCORE) {
		d_video->func_write(d_video, x, 25, 0x17, '\x9E');
		d_video->func_write(d_video, x + 1, 25, 0x17, ' ');
		write_number(d_video, x + 2, 25, 0x1F, state->world.info.score, 6);
	}
	x += 8;

	if (flags & ZOO_SIDEBAR_UPDATE_KEYS) {
		for (i = 0; i < 7; i++) {
			if (state->world.info.keys[i])
				d_video->func_write(d_video, x + i, 25, 0x19 + i, '\x0C');
			else
				d_video->func_write(d_video, x + i, 25, 0x1F, ' ');
		}
		d_video->func_write(d_video, x + 7, 25, 0x1F, ' ');
	}
	x += 8;

	if (flags & ZOO_SIDEBAR_UPDATE_TIME) {
		if (state->board.info.time_limit_sec > 0) {
			d_video->func_write(d_video, x, 25, 0x1E, 'T');
			d_video->func_write(d_video, x + 1, 25, 0x1E, ' ');
			write_number(d_video, x + 2, 25, 0x1F, state->board.info.time_limit_sec - state->world.info.board_time_sec, 6);
		} else {
			for (i = 0; i < 8; i++) {
				d_video->func_write(d_video, x + i, 25, 0x1E, ' ');
			}
		}
	}
	x += 8;

	if (flags & ZOO_SIDEBAR_UPDATE_REDRAW) {
		d_video->func_write(d_video, 0, 25, 0x1F, ' ');
		for (i = x; i < 60; i++) {
			d_video->func_write(d_video, i, 25, 0x1F, ' ');
		}
	}
}
