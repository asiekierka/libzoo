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
#include "../libzoo/zoo_internal.h" // zoo_element_defs
#include "zoo_sidebar.h"

static void zoo_sidebar_draw_string(zoo_state *state, uint8_t x, uint8_t y, uint8_t col, const char *text) {
	uint8_t i;

	for (i = 0; i < strlen(text); i++) {
		state->d_video->func_write(state->d_video, x + i, y, col, text[i]);
	}
}

static void zoo_sidebar_draw_value(zoo_state *state, uint8_t x, uint8_t y, uint8_t col, int16_t val, const char *pad) {
	char text[21];
	uint8_t i = 0;
	int16_t tval = val;

	// build value to string
	if (val < 0) {
		text[i++] = '-';
		val = -val;
	}
	// allocate characters
	if (val == 0) {
		text[i++] = '0';
		text[i++] = '\0';
	} else {
		for (tval = val; tval > 0; tval /= 10) {
			i++;
		}
		text[i--] = '\0';
		for (tval = val; tval > 0; tval /= 10) {
			text[i--] = '0' + (tval % 10);
		}
	}
	strncat(text, pad, sizeof(text) - 1);

	zoo_sidebar_draw_string(state, x, y, col, text);
}

static void zoo_sidebar_clear_line(zoo_state *state, uint8_t y) {
	uint8_t x;
	for (x = 60; x < 80; x++) {
		state->d_video->func_write(state->d_video, x, y, 0x11, ' ');
	}
}

void zoo_draw_sidebar_classic(zoo_state *state, uint16_t flags) {
	uint8_t y;

	if (flags & ZOO_SIDEBAR_UPDATE_REDRAW) {
		for (y = 0; y <= 24; y++) {
			zoo_sidebar_clear_line(state, y);
		}
		zoo_sidebar_draw_string(state, 61, 0, 0x1F, "    - - - - -      ");
		zoo_sidebar_draw_string(state, 62, 1, 0x70, "      Zoo      ");
		zoo_sidebar_draw_string(state, 61, 2, 0x1F, "    - - - - -      ");
		if (state->game_state == GS_PLAY) {
			zoo_sidebar_draw_string(state, 64,  7, 0x1E, " Health:");
			zoo_sidebar_draw_string(state, 64,  8, 0x1E, "   Ammo:");
			zoo_sidebar_draw_string(state, 64,  9, 0x1E, "Torches:");
			zoo_sidebar_draw_string(state, 64, 10, 0x1E, "   Gems:");
			zoo_sidebar_draw_string(state, 64, 11, 0x1E, "  Score:");
			zoo_sidebar_draw_string(state, 64, 12, 0x1E, "   Keys:");
			state->d_video->func_write(state->d_video, 62,  7, 0x1F, zoo_element_defs[ZOO_E_PLAYER].character);
			state->d_video->func_write(state->d_video, 62,  8, 0x1B, zoo_element_defs[ZOO_E_AMMO].character);
			state->d_video->func_write(state->d_video, 62,  9, 0x16, zoo_element_defs[ZOO_E_TORCH].character);
			state->d_video->func_write(state->d_video, 62, 10, 0x1B, zoo_element_defs[ZOO_E_GEM].character);
			state->d_video->func_write(state->d_video, 62, 12, 0x1F, zoo_element_defs[ZOO_E_KEY].character);
			zoo_sidebar_draw_string(state, 62, 14, 0x70, " T ");
			zoo_sidebar_draw_string(state, 65, 14, 0x1F, " Torch");
			zoo_sidebar_draw_string(state, 62, 15, 0x30, " B ");
			zoo_sidebar_draw_string(state, 62, 16, 0x70, " H ");
			zoo_sidebar_draw_string(state, 65, 16, 0x1F, " Help");
			zoo_sidebar_draw_string(state, 67, 18, 0x30, " \x18\x19\x1A\x1B ");
			zoo_sidebar_draw_string(state, 72, 18, 0x1F, " Move");
			zoo_sidebar_draw_string(state, 61, 19, 0x70, " Shift \x18\x19\x1A\x1B ");
			zoo_sidebar_draw_string(state, 72, 19, 0x1F, " Shoot");
			zoo_sidebar_draw_string(state, 62, 21, 0x70, " S ");
			zoo_sidebar_draw_string(state, 65, 21, 0x1F, " Save game");
			zoo_sidebar_draw_string(state, 62, 22, 0x30, " P ");
			zoo_sidebar_draw_string(state, 65, 22, 0x1F, " Pause");
			zoo_sidebar_draw_string(state, 62, 23, 0x70, " Q ");
			zoo_sidebar_draw_string(state, 65, 23, 0x1F, " Quit");
		} else {
			// TODO: game speed slider
			zoo_sidebar_draw_string(state, 62, 21, 0x70, " S ");
			zoo_sidebar_draw_string(state, 62, 7, 0x30, " W ");
			zoo_sidebar_draw_string(state, 65, 7, 0x1E, " World:");
			zoo_sidebar_draw_string(state, 69, 8, 0x1F, (strlen(state->world.info.name) != 0) ? state->world.info.name : "Untitled");
			zoo_sidebar_draw_string(state, 62, 11, 0x70, " P ");
			zoo_sidebar_draw_string(state, 65, 11, 0x1F, " Play");
			zoo_sidebar_draw_string(state, 62, 12, 0x30, " R ");
			zoo_sidebar_draw_string(state, 65, 12, 0x1E, " Restore game");
			zoo_sidebar_draw_string(state, 62, 13, 0x70, " Q ");
			zoo_sidebar_draw_string(state, 65, 13, 0x1E, " Quit");
			zoo_sidebar_draw_string(state, 62, 16, 0x30, " A ");
			zoo_sidebar_draw_string(state, 65, 16, 0x1F, " About ZZT!");
			zoo_sidebar_draw_string(state, 62, 17, 0x70, " H ");
			zoo_sidebar_draw_string(state, 65, 17, 0x1E, " High Scores");
			// TODO: editor enabled
		}
	}

	if (state->game_state == GS_PLAY) {
		if (flags & ZOO_SIDEBAR_UPDATE_PAUSED) {
			if (state->game_paused) {
				zoo_sidebar_draw_string(state, 64, 5, 0x1F, "Pausing...");
			} else {
				zoo_sidebar_clear_line(state, 5);
			}
		}

		if (state->board.info.time_limit_sec > 0) {
			zoo_sidebar_draw_string(state, 64, 6, 0x1E, "   Time:");
			zoo_sidebar_draw_value(state, 72, 6, 0x1E, state->board.info.time_limit_sec - state->world.info.board_time_sec, " ");
		} else {
			zoo_sidebar_clear_line(state, 6);
		}

		zoo_sidebar_draw_value(state, 72,  7, 0x1E, state->world.info.health, " ");
		zoo_sidebar_draw_value(state, 72,  8, 0x1E, state->world.info.ammo, "  ");
		zoo_sidebar_draw_value(state, 72,  9, 0x1E, state->world.info.torches, " ");
		zoo_sidebar_draw_value(state, 72, 10, 0x1E, state->world.info.gems, " ");
		zoo_sidebar_draw_value(state, 72, 11, 0x1E, state->world.info.score, " ");

		if (state->world.info.torch_ticks == 0) {
			zoo_sidebar_draw_string(state, 75, 9, 0x16, "    ");
		} else {
			for (y = 2; y <= 5; y++) {
				state->d_video->func_write(state->d_video, 73 + y, 9, 0x16, (y <= ((state->world.info.torch_ticks * 5) / ZOO_TORCH_DURATION)) ? '\xB1' : '\xB0');
			}
		}

		for (y = 0; y <= 6; y++) {
			state->d_video->func_write(state->d_video, 72 + y, 12, 0x19 + y, state->world.info.keys[y] ? zoo_element_defs[ZOO_E_KEY].character : ' ');
		}

		zoo_sidebar_draw_string(state, 65, 15, 0x1F, state->sound.enabled ? " Be quiet" : " Be noisy");
	}
}