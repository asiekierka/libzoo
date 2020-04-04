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
#include "zoo.h"

#ifdef ZOO_CONFIG_ENABLE_SIDEBAR_SLIM

static void write_number(zoo_state *state, int16_t x, int16_t y, uint8_t col, int val) {
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
		state->func_write_char(x + i - pos, y, col, s[i]);
	}
}

static void write_number_torch_bg(zoo_state *state, int16_t x, int16_t y, uint8_t col, int val) {
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

	uint8_t torch_col = (col & 0x0F) | 0x60;
	int16_t torch_pos = state->world.info.torch_ticks > 0
		? x + (state->world.info.torch_ticks + 39) / 40
		: x;
	if (torch_pos > (x + 5)) torch_pos = (x + 5);

	for (i = pos; i < sizeof(s); i++) {
		int16_t nx = x + i - pos;
		state->func_write_char(nx, y, nx < torch_pos ? torch_col : col, s[i]);
	}

	for (i = x + sizeof(s) - pos; i < torch_pos; i++) {
		state->func_write_char(i, y, torch_col, ' ');
	}
}

static void zoo_draw_sidebar_slim(zoo_state *state, uint16_t flags) {
	if (state->game_state != GS_PLAY) {
		if (flags & ZOO_SIDEBAR_UPDATE_REDRAW) {
			for (int i = 0; i < 60; i++) {
				state->func_write_char(i, 25, 0x0F, ' ');
			}
		}
		return;
	}

	for (int i = 0; i < 60; i++) {
		state->func_write_char(i, 25, 0x1F, ' ');
	}

	int x = 1;

	// left-aligned

	state->func_write_char(x, 25, 0x1C, '\x03');
	write_number(state, x + 2, 25, 0x1F, state->world.info.health);
	x += 8;

	state->func_write_char(x, 25, 0x1B, '\x84');
	write_number(state, x + 2, 25, 0x1F, state->world.info.ammo);
	x += 8;

	state->func_write_char(x, 25, 0x1E, '\x9D');
	write_number_torch_bg(state, x + 2, 25, 0x1F, state->world.info.torches);
	x += 8;

	state->func_write_char(x, 25, 0x19, '\x04');
	write_number(state, x + 2, 25, 0x1F, state->world.info.gems);
	x += 8;

	state->func_write_char(x, 25, 0x17, '\x9E');
	write_number(state, x + 2, 25, 0x1F, state->world.info.score);
	x += 8;

	for (int i = 0; i < 7; i++) {
		if (state->world.info.keys[i])
			state->func_write_char(x + i, 25, 0x19 + i, '\x0C');
	}
	x += 8;

	if (state->board.info.time_limit_sec > 0) {
		state->func_write_char(x, 25, 0x1E, 'T');
		write_number(state, x + 2, 25, 0x1F, state->board.info.time_limit_sec - state->world.info.board_time_sec);
	}
	x += 8;
}

void zoo_install_sidebar_slim(zoo_state *state) {
	state->func_update_sidebar = zoo_draw_sidebar_slim;
}

#endif /* ZOO_CONFIG_ENABLE_SIDEBAR_SLIM */
