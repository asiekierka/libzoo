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

const char zoo_line_chars[16] = "\xF9\xD0\xD2\xBA\xB5\xBC\xBB\xB9\xC6\xC8\xC9\xCC\xCD\xCA\xCB\xCE";
// FIX: preserve 1-indexing and fix messages for black keys/doors
const char zoo_color_names[8][8] = {
	"Black", "Blue", "Green", "Cyan", "Red", "Purple", "Yellow", "White"
};
const zoo_stat zoo_stat_template_default = {
	0, 0, 0, 0,
	0, 0, 0, 0,
	-1, -1,
	{ 0, 0 },
	NULL, 0, 0
};

const int16_t zoo_diagonal_delta_x[8] = {-1, 0, 1, 1, 1, 0, -1, -1};
const int16_t zoo_diagonal_delta_y[8] = {1, 1, 1, 0, -1, -1, -1, 0};
const int16_t zoo_neighbor_delta_x[4] = {0, 0, -1, 1};
const int16_t zoo_neighbor_delta_y[4] = {-1, 1, 0, 0};

static const zoo_tile zoo_board_tile_edge = {ZOO_E_BOARD_EDGE, 0x00};
static const zoo_tile zoo_board_tile_border = {ZOO_E_NORMAL, 0x0E};

void zoo_board_change(zoo_state *state, int16_t board_id) {
	state->board.tiles[state->board.stats[0].x][state->board.stats[0].y].element = ZOO_E_PLAYER;
	state->board.tiles[state->board.stats[0].x][state->board.stats[0].y].color
		= zoo_element_defs[ZOO_E_PLAYER].color;
	zoo_board_close(state);
	zoo_board_open(state, board_id);
}

void zoo_board_create(zoo_state *state) {
	int16_t i, ix, iy;

	state->board.name[0] = 0;
	state->board.info.message[0] = 0;
	state->board.info.max_shots = 255;
	state->board.info.is_dark = false;
	state->board.info.reenter_when_zapped = false;
	state->board.info.time_limit_sec = 0;
	for (i = 0; i < 4; i++)
		state->board.info.neighbor_boards[i] = 0;

	for (ix = 0; ix <= ZOO_BOARD_WIDTH + 1; ix++) {
		state->board.tiles[ix][0] = zoo_board_tile_edge;
		state->board.tiles[ix][ZOO_BOARD_HEIGHT + 1] = zoo_board_tile_edge;
	}

	for (iy = 0; iy <= ZOO_BOARD_HEIGHT + 1; iy++) {
		state->board.tiles[0][iy] = zoo_board_tile_edge;
		state->board.tiles[ZOO_BOARD_WIDTH + 1][iy] = zoo_board_tile_edge;
	}

	for (ix = 1; ix <= ZOO_BOARD_WIDTH; ix++) {
		for (iy = 1; iy <= ZOO_BOARD_HEIGHT; iy++) {
			state->board.tiles[ix][iy].element = ZOO_E_EMPTY;
			state->board.tiles[ix][iy].color = 0x00;
		}
	}

	for (ix = 1; ix <= ZOO_BOARD_WIDTH; ix++) {
		state->board.tiles[ix][1] = zoo_board_tile_border;
		state->board.tiles[ix][ZOO_BOARD_HEIGHT] = zoo_board_tile_border;
	}

	for (iy = 1; iy <= ZOO_BOARD_HEIGHT; iy++) {
		state->board.tiles[1][iy] = zoo_board_tile_border;
		state->board.tiles[ZOO_BOARD_WIDTH][iy] = zoo_board_tile_border;
	}

	state->board.tiles[ZOO_BOARD_WIDTH / 2][ZOO_BOARD_HEIGHT / 2].element = ZOO_E_PLAYER;
	state->board.tiles[ZOO_BOARD_WIDTH / 2][ZOO_BOARD_HEIGHT / 2].color
		= zoo_element_defs[ZOO_E_PLAYER].color;
	state->board.stat_count = 0;
	state->board.stats[0].x = ZOO_BOARD_WIDTH / 2;
	state->board.stats[0].y = ZOO_BOARD_HEIGHT / 2;
	state->board.stats[0].cycle = 1;
	state->board.stats[0].under.element = ZOO_E_EMPTY;
	state->board.stats[0].under.color = 0x00;
	state->board.stats[0].data = NULL;
	state->board.stats[0].data_len = 0;
}

void zoo_world_create(zoo_state *state) {
	int16_t i;

	state->world.board_count = 0;
	state->world.board_len[0] = 0;
	zoo_reset_message_flags(state);

	zoo_board_create(state);
	state->world.info.is_save = false;
	state->world.info.current_board = 0;
	state->world.info.ammo = 0;
	state->world.info.gems = 0;
	state->world.info.health = 100;
	state->world.info.energizer_ticks = 0;
	state->world.info.torches = 0;
	state->world.info.torch_ticks = 0;
	state->world.info.score = 0;
	state->world.info.board_time_sec = 0;
	state->world.info.board_time_hsec = 0;
	for (i = 0; i < 7; i++)
		state->world.info.keys[i] = false;
	for (i = 0; i < 10; i++)
		state->world.info.flags[i][0] = 0;

	zoo_board_change(state, 0);
	strncpy(state->board.name, "Title screen", sizeof(state->board.name) - 1);
	// TODO LoadedGameFileName = '';
	state->world.info.name[0] = 0;
}

void zoo_board_draw_tile(zoo_state *state, int16_t x, int16_t y) {
	uint8_t ch;
	zoo_tile *tile;

	if (state->func_write_char == NULL) {
		return;
	}

	// FIX: bounds check
	if (x >= 1 && y >= 1 && x <= ZOO_BOARD_WIDTH && y <= ZOO_BOARD_HEIGHT) {
		tile = &state->board.tiles[x][y];

		// darkness handling
		if (
			state->board.info.is_dark
			&& !zoo_element_defs[tile->element].visible_in_dark
			&& (
				(state->world.info.torch_ticks <= 0)
				|| (zoo_dist_sq2(state->board.stats[0].x - x, state->board.stats[0].y - y) >= ZOO_TORCH_DSQ)
			) && !state->force_darkness_off
		) {
			state->func_write_char(x - 1, y - 1, 0x07, '\xB0');
			return;
		}

		if (tile->element == ZOO_E_EMPTY) {
			state->func_write_char(x - 1, y - 1, 0x0F, ' ');
		} else if (zoo_element_defs[tile->element].has_draw_func) {
			zoo_element_defs[tile->element].draw_func(state, x, y, &ch);
			state->func_write_char(x - 1, y - 1, tile->color, ch);
		} else if (tile->element < ZOO_E_TEXT_MIN) {
			state->func_write_char(x - 1, y - 1, tile->color, zoo_element_defs[tile->element].character);
		} else {
			// text drawing
			// TODO: VideoMonochrome?
			if (tile->element == ZOO_E_TEXT_WHITE) {
				state->func_write_char(x - 1, y - 1, 0x0F, tile->color);
			} else {
				state->func_write_char(x - 1, y - 1,
					((tile->element - ZOO_E_TEXT_MIN + 1) << 4) | 0x0F, tile->color);
			}
		}
	}
}

void zoo_board_draw_border(zoo_state *state) {
	int i;

	for (i = 1; i <= ZOO_BOARD_WIDTH; i++) {
		zoo_board_draw_tile(state, i, 1);
		zoo_board_draw_tile(state, i, ZOO_BOARD_HEIGHT);
	}

	for (i = 1; i <= ZOO_BOARD_HEIGHT; i++) {
		zoo_board_draw_tile(state, 1, i);
		zoo_board_draw_tile(state, ZOO_BOARD_WIDTH, i);
	}
}

void zoo_board_draw(zoo_state *state) {
	int ix, iy;

	for (iy = 1; iy <= ZOO_BOARD_HEIGHT; iy++) {
		for (ix = 1; ix <= ZOO_BOARD_WIDTH; ix++) {
			zoo_board_draw_tile(state, ix, iy);
		}
	}
}

void zoo_stat_add(zoo_state *state, int16_t tx, int16_t ty, uint8_t element, int16_t color, int16_t tcycle, const zoo_stat *stat_template) {
	zoo_stat *stat;

	if (state->board.stat_count < ZOO_MAX_STAT) {
		state->board.stat_count++;
		stat = &(state->board.stats[state->board.stat_count]);

		memcpy(stat, stat_template, sizeof(zoo_stat));
		stat->x = tx;
		stat->y = ty;
		stat->cycle = tcycle;
		stat->under = state->board.tiles[tx][ty];
		stat->data_pos = 0;

		if (stat_template->data != NULL) {
			stat->data = malloc(stat->data_len);
			memcpy(stat->data, stat_template->data, stat->data_len);
		}

		if (zoo_element_defs[state->board.tiles[tx][ty].element].placeable_on_top) {
			state->board.tiles[tx][ty].color = (state->board.tiles[tx][ty].color & 0x70) | (color & 0x0F);
		} else {
			state->board.tiles[tx][ty].color = color;
		}
		state->board.tiles[tx][ty].element = element;

		if (ty > 0) {
			zoo_board_draw_tile(state, tx, ty);
		}
	}
}

void zoo_stat_remove(zoo_state *state, int16_t stat_id) {
	int i;
	zoo_stat *stat;

	// FIX: bounds check
	if (stat_id < 0 || stat_id > state->board.stat_count) return;
	stat = &(state->board.stats[stat_id]);

	if (stat->data_len != 0) {
		for (i = 1; i <= state->board.stat_count; i++) {
			if (i == stat_id) continue;
			if (state->board.stats[i].data == stat->data) goto StatDataInUse;
		}
	}
	free(stat->data);

StatDataInUse:
	if (stat_id < state->current_stat_tick) {
		state->current_stat_tick--;
	}

	state->board.tiles[stat->x][stat->y] = stat->under;
	if (stat->y > 0) {
		zoo_board_draw_tile(state, stat->x, stat->y);
	}

	for (i = 1; i <= state->board.stat_count; i++) {
		if (state->board.stats[i].follower >= stat_id) {
			if (state->board.stats[i].follower == stat_id) {
				state->board.stats[i].follower = -1;
			} else {
				state->board.stats[i].follower--;
			}
		}

		if (state->board.stats[i].leader >= stat_id) {
			if (state->board.stats[i].leader == stat_id) {
				state->board.stats[i].leader = -1;
			} else {
				state->board.stats[i].leader--;
			}
		}
	}

	for (i = stat_id + 1; i <= state->board.stat_count; i++) {
		state->board.stats[i - 1] = state->board.stats[i];
	}
	state->board.stat_count--;
}

int16_t zoo_stat_get_id(zoo_state *state, int16_t x, int16_t y) {
	int16_t i;
	zoo_stat *stat = state->board.stats;

	for (i = 0; i <= state->board.stat_count; i++, stat++) {
		if (x == stat->x && y == stat->y)
			return i;
	}

	return -1;
}

zoo_stat *zoo_stat_get(zoo_state *state, int16_t x, int16_t y, int16_t *stat_id) {
	int16_t i;
	zoo_stat *stat = state->board.stats;

	for (i = 0; i <= state->board.stat_count; i++, stat++) {
		if (x == stat->x && y == stat->y) {
			if (stat_id != NULL) {
				*stat_id = i;
			}
			return stat;
		}
	}

	if (stat_id != NULL) {
		*stat_id = -1;
	}
	return NULL;
}

void zoo_stat_move(zoo_state *state, int16_t stat_id, int16_t new_x, int16_t new_y) {
	zoo_stat *stat;
	zoo_tile under;
	int16_t old_x, old_y;
	int16_t ix, iy;

	// FIX: bounds check
	if (stat_id < 0 || stat_id > state->board.stat_count) return;

	stat = &(state->board.stats[stat_id]);

	under = stat->under;
	stat->under = state->board.tiles[new_x][new_y];

	if (state->board.tiles[stat->x][stat->y].element == ZOO_E_PLAYER) {
		state->board.tiles[new_x][new_y].color = state->board.tiles[stat->x][stat->y].color;
	} else if (state->board.tiles[new_x][new_y].element == ZOO_E_EMPTY) {
		state->board.tiles[new_x][new_y].color = state->board.tiles[stat->x][stat->y].color & 0x0F;
	} else {
		state->board.tiles[new_x][new_y].color = (state->board.tiles[stat->x][stat->y].color & 0x0F)
			+ (state->board.tiles[new_x][new_y].color & 0x70);
	}

	state->board.tiles[new_x][new_y].element = state->board.tiles[stat->x][stat->y].element;
	state->board.tiles[stat->x][stat->y] = under;

	old_x = stat->x;
	old_y = stat->y;
	stat->x = new_x;
	stat->y = new_y;

	zoo_board_draw_tile(state, stat->x, stat->y);
	zoo_board_draw_tile(state, old_x, old_y);

	if (stat_id == 0 && state->board.info.is_dark && state->world.info.torch_ticks > 0) {
		if (zoo_dist_sq(old_x - stat->x, old_y - stat->y) == 1) {
			for (ix = stat->x - ZOO_TORCH_DX - 3; ix <= stat->x + ZOO_TORCH_DX + 3; ix++)
			if (ix >= 1 && ix <= ZOO_BOARD_WIDTH)
			for (iy = stat->y - ZOO_TORCH_DY - 3; iy <= stat->y + ZOO_TORCH_DY + 3; iy++)
			if (iy >= 1 && iy <= ZOO_BOARD_HEIGHT) {
				if ((zoo_dist_sq2(ix - old_x, iy - old_y) < ZOO_TORCH_DSQ)
					^ (zoo_dist_sq2(ix - new_x, iy - new_y) < ZOO_TORCH_DSQ)) {
					zoo_board_draw_tile(state, ix, iy);
				}
			}
		} else {
			zoo_draw_player_surroundings(state, old_x, old_y, 0);
			zoo_draw_player_surroundings(state, stat->x, stat->y, 0);
		}
	}
}

void zoo_display_message(zoo_state *state, int16_t duration, char *message) {
	int i;

	i = zoo_stat_get_id(state, 0, 0);
	if (i != -1) {
		zoo_stat_remove(state, i);
		zoo_board_draw_border(state);
	}

	if (strlen(message) > 0) {
		zoo_stat_add(state, 0, 0, ZOO_E_MESSAGE_TIMER, 0, 1, &zoo_stat_template_default);
		state->board.stats[state->board.stat_count].p2 = duration / (state->tick_duration + 1);
		strncpy(state->board.info.message, message, sizeof(state->board.info.message) - 1);
	}
}

void zoo_board_damage_stat(zoo_state *state, int16_t stat_id) {
	int16_t old_x, old_y;
	zoo_stat *stat;

	// FIX: bounds check
	if (stat_id < 0 || stat_id > state->board.stat_count) return;

	stat = &(state->board.stats[stat_id]);
	if (stat_id == 0) {
		if (state->world.info.health > 0) {
			state->world.info.health -= 10;
			if (state->world.info.health < 0) {
				state->world.info.health = 0;
			}

			state->func_ui_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_HEALTH);
			zoo_display_message(state, 100, "Ouch!");

			state->board.tiles[stat->x][stat->y].color = 0x70 | (zoo_element_defs[ZOO_E_PLAYER].color & 0x0F);

			if (state->world.info.health > 0) {
				state->world.info.board_time_sec = 0;
				if (state->board.info.reenter_when_zapped) {
					zoo_sound_queue_const(&(state->sound), 4, "\x20\x01\x23\x01\x27\x01\x30\x01\x10\x01");

					// move player to start
					state->board.tiles[stat->x][stat->y].element = ZOO_E_EMPTY;
					zoo_board_draw_tile(state, stat->x, stat->y);
					old_x = stat->x;
					old_y = stat->y;
					stat->x = state->board.info.start_player_x;
					stat->y = state->board.info.start_player_y;
					zoo_draw_player_surroundings(state, old_x, old_y, 0);
					zoo_draw_player_surroundings(state, stat->x, stat->y, 0);

					state->game_paused = true;
				}
				zoo_sound_queue_const(&(state->sound), 4, "\x10\x01\x20\x01\x13\x01\x23\x01");
			} else {
				zoo_sound_queue_const(&(state->sound), 5,
					"\x20\x03\x23\x03\x27\x03\x30\x03"
					"\x27\x03\x2A\x03\x32\x03\x37\x03"
					"\x35\x03\x38\x03\x40\x03\x45\x03\x10\x0A"
				);
			}
		}
	} else {
		switch (state->board.tiles[stat->x][stat->y].element) {
			case ZOO_E_BULLET: zoo_sound_queue_const(&(state->sound), 3, "\x20\x01"); break;
			case ZOO_E_OBJECT: break;
			default: zoo_sound_queue_const(&(state->sound), 3, "\x40\x01\x10\x01\x50\x01\x30\x01"); break;
		}
		zoo_stat_remove(state, stat_id);
	}
}

void zoo_board_damage_tile(zoo_state *state, int16_t x, int16_t y) {
	int16_t stat_id = zoo_stat_get_id(state, x, y);

	if (stat_id != -1) {
		zoo_board_damage_stat(state, stat_id);
	} else {
		state->board.tiles[x][y].element = ZOO_E_EMPTY;
		zoo_board_draw_tile(state, x, y);
	}
}

void zoo_board_attack_tile(zoo_state *state, int16_t attacker_stat_id, int16_t x, int16_t y) {
	if ((attacker_stat_id == 0) && (state->world.info.energizer_ticks > 0)) {
		state->world.info.score += zoo_element_defs[state->board.tiles[x][y].element].score_value;
		state->func_ui_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_SCORE);
	} else {
		zoo_board_damage_stat(state, attacker_stat_id);
	}

	if ((attacker_stat_id > 0) && (attacker_stat_id <= state->current_stat_tick)) {
		state->current_stat_tick--;
	}

	if ((state->board.tiles[x][y].element == ZOO_E_PLAYER) && (state->world.info.energizer_ticks > 0)) {
		state->world.info.score += zoo_element_defs[state->board.tiles
			[state->board.stats[attacker_stat_id].x]
			[state->board.stats[attacker_stat_id].y]
		.element].score_value;
		state->func_ui_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_SCORE);
	} else {
		zoo_board_damage_tile(state, x, y);
		zoo_sound_queue_const(&(state->sound), 2, "\x10\x01");
	}
}

bool zoo_board_shoot(zoo_state *state, uint8_t element, int16_t x, int16_t y, int16_t dx, int16_t dy, int16_t param1) {
	if ((zoo_element_defs[state->board.tiles[x+dx][y+dy].element].walkable)
		|| (state->board.tiles[x+dx][y+dy].element == ZOO_E_WATER))
	{
		zoo_stat_add(state, x + dx, y + dy, element, zoo_element_defs[element].color, 1, &zoo_stat_template_default);
		state->board.stats[state->board.stat_count].p1 = param1;
		state->board.stats[state->board.stat_count].step_x = dx;
		state->board.stats[state->board.stat_count].step_y = dy;
		state->board.stats[state->board.stat_count].p2 = 100;
		return true;
	} else if (state->board.tiles[x+dx][y+dy].element == ZOO_E_BREAKABLE
		|| (
			zoo_element_defs[state->board.tiles[x+dx][y+dy].element].destructible
			&& ((state->board.tiles[x+dx][y+dy].element == ZOO_E_PLAYER) == (param1 != 0))
			&& (state->world.info.energizer_ticks <= 0)
		)
	) {
		zoo_board_damage_tile(state, x + dx, y + dy);
		zoo_sound_queue_const(&(state->sound), 2, "\x10\x01");
		return true;
	} else {
		return false;
	}
}

void zoo_calc_direction_rnd(zoo_state *state, int16_t *dx, int16_t *dy) {
	*dx = state->func_random(state, 3) - 1;
	*dy = (*dx == 0) ? (state->func_random(state, 2) * 2 - 1) : 0;
}

void zoo_calc_direction_seek(zoo_state *state, int16_t x, int16_t y, int16_t *dx, int16_t *dy) {
	*dx = 0;
	*dy = 0;

	if ((state->func_random(state, 2) < 1) || (state->board.stats[0].y == y)) {
		*dx = zoo_signum(state->board.stats[0].x - x);
	}

	if (*dx == 0) {
		*dy = zoo_signum(state->board.stats[0].y - y);
	}

	if (state->world.info.energizer_ticks > 0) {
		*dx = -(*dx);
		*dy = -(*dy);
	}
}

void zoo_board_enter(zoo_state *state) {
	state->board.info.start_player_x = state->board.stats[0].x;
	state->board.info.start_player_y = state->board.stats[0].y;

	if (state->board.info.is_dark) {
		if (state->msg_flags.hint_torch) {
			zoo_display_message(state, 200, "Room is dark - you need to light a torch!");
			state->msg_flags.hint_torch = false;
		}
	}

	state->world.info.board_time_sec = 0;
	state->func_ui_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_ALL);
}

void zoo_board_passage_teleport(zoo_state *state, int16_t x, int16_t y) {
	zoo_stat *stat;
	int16_t old_board;
	uint8_t color;
	int16_t ix, iy, new_x, new_y;

	stat = zoo_stat_get(state, x, y, NULL);
	color = state->board.tiles[x][y].color;
	old_board = state->world.info.current_board;
	zoo_board_change(state, stat != NULL ? stat->p3 : 0);

	new_x = 0;
	for (ix = 1; ix <= ZOO_BOARD_WIDTH; ix++) {
		for (iy = 1; iy <= ZOO_BOARD_HEIGHT; iy++) {
			if (
				state->board.tiles[ix][iy].element == ZOO_E_PASSAGE
				&& state->board.tiles[ix][iy].color == color
			) {
				new_x = ix;
				new_y = iy;
			}
		}
	}

	state->board.tiles[state->board.stats[0].x][state->board.stats[0].y].element = ZOO_E_EMPTY;
	state->board.tiles[state->board.stats[0].x][state->board.stats[0].y].color = 0x00;
	if (new_x != 0) {
		state->board.stats[0].x = new_x;
		state->board.stats[0].y = new_y;
	}

	state->game_paused = true;
	zoo_sound_queue_const(&(state->sound), 4,
		"\x30\x01\x34\x01\x37\x01"
		"\x31\x01\x35\x01\x38\x01"
		"\x32\x01\x36\x01\x39\x01"
		"\x33\x01\x37\x01\x3A\x01"
		"\x34\x01\x38\x01\x40\x01");
	// TODO transition
	zoo_board_draw(state); // not here
	zoo_board_enter(state);
}

void zoo_game_start(zoo_state *state, zoo_game_state game_state) {
	state->game_state = game_state;

	// call stack handling
	while (state->call_stack.call != NULL) {
		zoo_call_pop(&state->call_stack);
	}

	if (game_state == GS_NONE) {
		return;
	}

	// GS_TITLE/GS_PLAY
	state->game_play_exit_requested = false;
	state->current_tick = state->func_random(state, 100);
	state->current_stat_tick = state->board.stat_count + 1;
	state->tick_duration = state->tick_speed * 2;
	state->time_elapsed = 0;

	state->board.tiles[state->board.stats[0].x][state->board.stats[0].y].element
		= (state->game_state == GS_TITLE) ? ZOO_E_MONITOR : ZOO_E_PLAYER;
	state->board.tiles[state->board.stats[0].x][state->board.stats[0].y].color
		= zoo_element_defs[(state->game_state == GS_TITLE) ? ZOO_E_MONITOR : ZOO_E_PLAYER].color;

	zoo_board_enter(state);

	if (state->game_state == GS_TITLE) {
		zoo_display_message(state, 0, "");
		state->game_title_exit_requested = false;
		state->game_paused = false;
	} else if (state->game_state == GS_PLAY) {
		state->game_paused = true;
	}

	zoo_sound_clear_queue(&(state->sound));
	state->sound.block_queueing = false;
}

void zoo_game_stop(zoo_state *state) {
	state->game_state = GS_NONE;
	zoo_sound_clear_queue(&(state->sound));
}

static ZOO_INLINE zoo_tick_retval zoo_call_stack_tick(zoo_state *state) {
	zoo_call call;
	zoo_call *callptr;
	zoo_tick_retval ret;

	// call stack handling
	if (state->call_stack.call != NULL) {
		callptr = state->call_stack.call;
		switch (callptr->type) {
			case CALLBACK:
				// handle these a bit differently for performance
				ret = callptr->args.cb.func(
					state,
					callptr->args.cb.arg
				);
				if (ret == EXIT) {
					if (state->call_stack.call == callptr) {
						zoo_call_pop(&state->call_stack);
					}
					ret = RETURN_IMMEDIATE;
				}
				return ret;
			default:
				// handle these a bit slowly as they are not loops. should be fine
				memcpy(&call, state->call_stack.call, sizeof(zoo_call));
				zoo_call_pop(&state->call_stack);
				state->call_stack.state = call.state;
				state->call_stack.curr_call = &call;
				switch (call.type) {
					case TICK_FUNC:
						call.args.tick.func(state, call.args.tick.stat_id);
						break;
					case TOUCH_FUNC:
						call.args.touch.func(state, call.args.touch.x, call.args.touch.y,
							call.args.touch.source_stat_id, call.args.touch.dx, call.args.touch.dy);
						break;
					default:
						break;
				}
				state->call_stack.curr_call = NULL;
				return RETURN_IMMEDIATE;
		}
	}

	state->call_stack.state = 0;
	return EXIT; // never returned officially, so we repurpose it to mean "continue"
}

static zoo_tick_retval zoo_game_tick(zoo_state *state) {
	int i;
	uint8_t call_state;

	call_state = state->game_tick_state;
	state->game_tick_state = 0;

	switch (call_state) {
		case 1:
			goto GameTickState1;
		case 2:
			goto GameTickState2;
	}

	if (state->game_paused) {
		if (state->func_write_char != NULL) {
			if (zoo_has_hsecs_elapsed(state, &state->tick_counter, 25)) {
				state->game_paused_blink = !state->game_paused_blink;
			}

			if (state->game_paused_blink) {
				state->func_write_char(
					state->board.stats[0].x - 1,
					state->board.stats[0].y - 1,
					zoo_element_defs[ZOO_E_PLAYER].color,
					zoo_element_defs[ZOO_E_PLAYER].character
				);
			} else {
				if (state->board.tiles[state->board.stats[0].x][state->board.stats[0].y].element == ZOO_E_PLAYER) {
					state->func_write_char(
						state->board.stats[0].x - 1,
						state->board.stats[0].y - 1,
						0x0F, ' '
					);
				} else {
					zoo_board_draw_tile(state, state->board.stats[0].x, state->board.stats[0].y);
				}
			}
		}

		state->func_ui_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_PAUSED);
		zoo_input_update(&state->input);

		if (state->input.delta_x != 0 || state->input.delta_y != 0) {
			// push self
			state->game_tick_state = 1;
			// touch
			zoo_element_defs[state->board.tiles
				[state->board.stats[0].x + state->input.delta_x]
				[state->board.stats[0].y + state->input.delta_y]
			.element].touch_func(state,
				state->board.stats[0].x + state->input.delta_x,
				state->board.stats[0].y + state->input.delta_y,
				0, &state->input.delta_x,
				&state->input.delta_y
			);
			// return
			return RETURN_IMMEDIATE;
		}

GameTickState1:
		if (
			(state->input.delta_x != 0 || state->input.delta_y != 0)
			&& zoo_element_defs[state->board.tiles
				[state->board.stats[0].x + state->input.delta_x]
				[state->board.stats[0].y + state->input.delta_y]
			.element].walkable
		) {
			if (state->board.tiles[state->board.stats[0].x][state->board.stats[0].y].element == ZOO_E_PLAYER) {
				zoo_stat_move(state, 0,
					state->board.stats[0].x + state->input.delta_x,
					state->board.stats[0].y + state->input.delta_y
				);
			} else {
				zoo_board_draw_tile(state, state->board.stats[0].x, state->board.stats[0].y);
				state->board.stats[0].x += state->input.delta_x;
				state->board.stats[0].y += state->input.delta_y;
				state->board.tiles[state->board.stats[0].x][state->board.stats[0].y].element = ZOO_E_PLAYER;
				state->board.tiles[state->board.stats[0].x][state->board.stats[0].y].color
					= zoo_element_defs[ZOO_E_PLAYER].color;
				zoo_board_draw_tile(state, state->board.stats[0].x, state->board.stats[0].y);
				zoo_draw_player_surroundings(state, state->board.stats[0].x, state->board.stats[0].y, 0);
				zoo_draw_player_surroundings(state,
					state->board.stats[0].x - state->input.delta_x,
					state->board.stats[0].y - state->input.delta_y, 0
				);
			}

			// unpause
			state->game_paused = false;
			state->func_ui_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_PAUSED);
			state->current_tick = state->func_random(state, 100);
			state->current_stat_tick = state->board.stat_count + 1;
			state->world.info.is_save = true;

			return RETURN_NEXT_CYCLE;
		} else {
			return RETURN_NEXT_CYCLE;
		}
	} else {
		// not paused
		i = state->current_stat_tick;
		if (i <= state->board.stat_count) {
			if (state->board.stats[i].cycle != 0
				&& (state->current_tick % state->board.stats[i].cycle) == (i % state->board.stats[i].cycle)
			) {
				// tick self
				zoo_element_defs[
					state->board.tiles[state->board.stats[i].x][state->board.stats[i].y].element
				].tick_func(state, i);

				// if anything on stack...
				if (state->call_stack.call != NULL) {
					state->game_tick_state = 2;
					return RETURN_IMMEDIATE;
				}
			}
GameTickState2:
			state->current_stat_tick++;
		}
	}

	if (state->current_stat_tick > state->board.stat_count) {
		if (state->tick_duration == 0) {
			// workaround for fast game speeds to avoid high CPU usage
			state->current_tick++;
			if (state->current_tick > 420)
				state->current_tick = 1;
			state->current_stat_tick = 0;

			zoo_input_update(&state->input);
			return RETURN_NEXT_FRAME;
		} else {
			if (zoo_has_hsecs_elapsed(state, &state->tick_counter, state->tick_duration)) {
				state->current_tick++;
				if (state->current_tick > 420)
					state->current_tick = 1;
				state->current_stat_tick = 0;

				zoo_input_update(&state->input);
				return RETURN_IMMEDIATE;
			} else {
				return RETURN_NEXT_CYCLE;
			}
		}
	}

	if (state->game_play_exit_requested) {
		// TODO
		return RETURN_NEXT_CYCLE;
	}

	return RETURN_IMMEDIATE;
}

void zoo_tick_advance_pit(zoo_state *state) {
	state->time_elapsed += ZOO_PIT_TICK_MS;
	while (state->time_elapsed >= 60000) {
		state->time_elapsed -= 60000;
	}
}

static ZOO_INLINE zoo_tick_retval zoo_tick_inner(zoo_state *state) {
	zoo_tick_retval ret;

	ret = zoo_call_stack_tick(state);
	if (ret != EXIT) {
		return ret;
	}

	switch (state->game_state) {
		default: /* GS_NONE */
			return RETURN_NEXT_CYCLE;
		case GS_TITLE:
		case GS_PLAY:
			return zoo_game_tick(state);
	}
}

zoo_tick_retval zoo_tick(zoo_state *state) {
	zoo_tick_retval ret = zoo_tick_inner(state);
	if (ret != RETURN_IMMEDIATE) {
		zoo_input_clear_post_tick(&state->input);
	}
	return ret;
}
