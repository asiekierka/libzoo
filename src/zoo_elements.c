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

#define TICK_GET_SELF zoo_stat *stat = &state->board.stats[stat_id]
#define CALL_STATE_GET_SELF uint8_t call_state = state->call_stack.state; state->call_stack.state = 0
#define CALL_SET_TOUCH_ARGS(c) \
	(c)->args.touch.x = x; \
	(c)->args.touch.y = y; \
	(c)->args.touch.source_stat_id = source_stat_id; \
	(c)->args.touch.dx = dx; \
	(c)->args.touch.dy = dy

static void zoo_default_tick(zoo_state *state, int16_t stat_id) {

}

static void zoo_default_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {

}

static void zoo_default_draw(zoo_state *state, int16_t x, int16_t y, uint8_t *ch) {
	*ch = '?';
}

static void zoo_e_message_timer_tick(zoo_state *state, int16_t stat_id) {
	int i, ix;
	TICK_GET_SELF;

	if (stat->x == 0) {
		if (state->func_write_message != NULL) {
			state->func_write_message(state, stat->p2, state->board.info.message);
		} else if (state->func_write_char != NULL) {
			ix = strnlen(state->board.info.message, ZOO_LEN_MESSAGE);
			state->func_write_char(((60 - ix) >> 1), 24, 9 + (stat->p2 % 7), ' ');
			for (i = 0; i < ix; i++) {
				state->func_write_char(((60 - ix) >> 1) + 1 + i, 24, 9 + (stat->p2 % 7), state->board.info.message[i]);
			}
			state->func_write_char(((60 - ix) >> 1) + 1 + ix, 24, 9 + (stat->p2 % 7), ' ');
		}
		stat->p2--;
		if (stat->p2 <= 0) {
			zoo_stat_remove(state, stat_id);
			state->current_stat_tick--;
			if (state->func_write_message != NULL) {
				state->func_write_message(state, 0, NULL);
			}
			zoo_board_draw_border(state);
			state->board.info.message[0] = 0;
		}
	}
}

static void zoo_e_damaging_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	zoo_board_attack_tile(state, source_stat_id, x, y);
}

static void zoo_e_lion_tick(zoo_state *state, int16_t stat_id) {
	int16_t dx, dy;
	TICK_GET_SELF;

	if (stat->p1 < state->func_random(10)) {
		zoo_calc_direction_rnd(state, &dx, &dy);
	} else {
		zoo_calc_direction_seek(state, stat->x, stat->y, &dx, &dy);
	}

	if (zoo_element_defs[state->board.tiles[stat->x + dx][stat->y + dy].element].walkable) {
		zoo_stat_move(state, stat_id, stat->x + dx, stat->y + dy);
	} else if (state->board.tiles[stat->x + dx][stat->y + dy].element == ZOO_E_PLAYER) {
		zoo_board_attack_tile(state, stat_id, stat->x + dx, stat->y + dy);
	}
}

static void zoo_e_tiger_tick(zoo_state *state, int16_t stat_id) {
	bool shot;
	uint8_t element;
	TICK_GET_SELF;

	element = (stat->p2 & 0x80) ? ZOO_E_STAR : ZOO_E_BULLET;
	if ((state->func_random(10) * 3) <= (stat->p2 & 0x7F)) {
		if (zoo_difference(stat->x, state->board.stats[0].x) <= 2) {
			shot = zoo_board_shoot(state, element, stat->x, stat->y, 0, zoo_signum(state->board.stats[0].y - stat->y), 1);
		} else {
			shot = false;
		}

		if (!shot) {
			if (zoo_difference(stat->y, state->board.stats[0].y) <= 2) {
				shot = zoo_board_shoot(state, element, stat->x, stat->y, zoo_signum(state->board.stats[0].x - stat->x), 0, 1);
			}
		}
	}

	zoo_e_lion_tick(state, stat_id);
}

static void zoo_e_ruffian_tick(zoo_state *state, int16_t stat_id) {
	uint8_t *d_elem;
	TICK_GET_SELF;

	if ((stat->step_x == 0) && (stat->step_y == 0)) {
		if ((stat->p2 + 8) <= state->func_random(17)) {
			if (stat->p1 >= state->func_random(9)) {
				zoo_calc_direction_seek(state, stat->x, stat->y, &stat->step_x, &stat->step_y);
			} else {
				zoo_calc_direction_rnd(state, &stat->step_x, &stat->step_y);
			}
		}
	} else {
		if (((stat->y == state->board.stats[0].y) || (stat->x == state->board.stats[0].x)) && (state->func_random(9) <= stat->p1)) {
			zoo_calc_direction_seek(state, stat->x, stat->y, &stat->step_x, &stat->step_y);
		}

		d_elem = &state->board.tiles[stat->x + stat->step_x][stat->y + stat->step_y].element;
		if (*d_elem == ZOO_E_PLAYER) {
			zoo_board_attack_tile(state, stat_id, stat->x + stat->step_x, stat->y + stat->step_y);
		} else if (zoo_element_defs[*d_elem].walkable) {
			zoo_stat_move(state, stat_id, stat->x + stat->step_x, stat->y + stat->step_y);

			if ((stat->p2 + 8) <= state->func_random(17)) {
				stat->step_x = 0;
				stat->step_y = 0;
			}
		} else {
			stat->step_x = 0;
			stat->step_y = 0;
		}
	}
}

static void zoo_e_bear_tick(zoo_state *state, int16_t stat_id) {
	uint8_t *d_elem;
	int16_t dx, dy;
	TICK_GET_SELF;

	if (stat->x != state->board.stats[0].x) {
		if (zoo_difference(stat->y, state->board.stats[0].y) <= (8 - stat->p1)) {
			dx = zoo_signum(state->board.stats[0].x - stat->x);
			dy = 0;
			goto Movement;
		}
	}

	if (zoo_difference(stat->x, state->board.stats[0].x) <= (8 - stat->p1)) {
		dx = 0;
		dy = zoo_signum(state->board.stats[0].y - stat->y);
		goto Movement;
	}

	dx = 0;
	dy = 0;

Movement:
	d_elem = &state->board.tiles[stat->x + dx][stat->y + dy].element;
	if (zoo_element_defs[*d_elem].walkable) {
		zoo_stat_move(state, stat_id, stat->x + dx, stat->y + dy);
	} else if (*d_elem == ZOO_E_PLAYER || *d_elem == ZOO_E_BREAKABLE) {
		zoo_board_attack_tile(state, stat_id, stat->x + dx, stat->y + dy);
	}
}

static void zoo_e_centipede_head_tick(zoo_state *state, int16_t stat_id) {
	int16_t ix, iy;
	int16_t tx, ty;
	int16_t tmp;
	zoo_stat *stat2;
	TICK_GET_SELF;

	if ((stat->x == state->board.stats[0].x) && (state->func_random(10) < stat->p1)) {
		stat->step_y = zoo_signum(state->board.stats[0].y - stat->y);
		stat->step_x = 0;
	} else if ((stat->y == state->board.stats[0].y) && (state->func_random(10) < stat->p1)) {
		stat->step_x = zoo_signum(state->board.stats[0].x - stat->x);
		stat->step_y = 0;
	} else if (((state->func_random(10) * 4) < stat->p2) || ((stat->step_x == 0) && (stat->step_y == 0))) {
		zoo_calc_direction_rnd(state, &stat->step_x, &stat->step_y);
	}

	if (!zoo_element_defs[state->board.tiles[stat->x + stat->step_x][stat->y + stat->step_y].element].walkable
		&& (state->board.tiles[stat->x + stat->step_x][stat->y + stat->step_y].element != ZOO_E_PLAYER))
	{
		ix = stat->step_x;
		iy = stat->step_y;

		tmp = ((state->func_random(2) * 2) - 1) * stat->step_y;
		stat->step_y = ((state->func_random(2) * 2) - 1) * stat->step_x;
		stat->step_x = tmp;

		if (!zoo_element_defs[state->board.tiles[stat->x + stat->step_x][stat->y + stat->step_y].element].walkable
			&& (state->board.tiles[stat->x + stat->step_x][stat->y + stat->step_y].element != ZOO_E_PLAYER))
		{
			stat->step_x = -stat->step_x;
			stat->step_y = -stat->step_y;

			if (!zoo_element_defs[state->board.tiles[stat->x + stat->step_x][stat->y + stat->step_y].element].walkable
				&& (state->board.tiles[stat->x + stat->step_x][stat->y + stat->step_y].element != ZOO_E_PLAYER))
			{
				if (zoo_element_defs[state->board.tiles[stat->x - ix][stat->y - iy].element].walkable
					|| (state->board.tiles[stat->x - ix][stat->y - iy].element == ZOO_E_PLAYER))
				{
					stat->step_x = -ix;
					stat->step_y = -iy;
				} else {
					stat->step_x = 0;
					stat->step_y = 0;
				}
			}
		}
	}

	if ((stat->step_x == 0) && (stat->step_y == 0)) {
		// flip centipede
		state->board.tiles[stat->x][stat->y].element = ZOO_E_CENTIPEDE_SEGMENT;
		stat->leader = -1;
		while (stat->follower > 0) {
			tmp = stat->follower;
			stat->follower = stat->leader;
			stat->leader = tmp;

			stat_id = tmp;
			stat = &(state->board.stats[stat_id]);
		}
		stat->follower = stat->leader;
		state->board.tiles[stat->x][stat->y].element = ZOO_E_CENTIPEDE_HEAD;
	} else if (state->board.tiles[stat->x + stat->step_x][stat->y + stat->step_y].element == ZOO_E_PLAYER) {
		// attack player
		if (stat->follower != -1) {
			stat2 = &(state->board.stats[stat->follower]);
			state->board.tiles[stat2->x][stat2->y].element = ZOO_E_CENTIPEDE_HEAD;
			stat2->step_x = stat->step_x;
			stat2->step_y = stat->step_y;
			zoo_board_draw_tile(state, stat2->x, stat2->y);
		}
		zoo_board_attack_tile(state, stat_id, stat->x + stat->step_x, stat->y + stat->step_y);
	} else {
		zoo_stat_move(state, stat_id, stat->x + stat->step_x, stat->y + stat->step_y);

		do {
			stat = &(state->board.stats[stat_id]);
			tx = stat->x - stat->step_x;
			ty = stat->y - stat->step_y;
			ix = stat->step_x;
			iy = stat->step_y;

			if (stat->follower < 0) {
				if (state->board.tiles[tx - ix][ty - iy].element == ZOO_E_CENTIPEDE_SEGMENT) {
					tmp = zoo_stat_get_id(state, tx - ix, ty - iy);
					if (tmp >= 0 && state->board.stats[tmp].leader < 0) {
						stat->follower = tmp;
					}
				} else if (state->board.tiles[tx - iy][ty - ix].element == ZOO_E_CENTIPEDE_SEGMENT) {
					tmp = zoo_stat_get_id(state, tx - iy, ty - ix);
					if (tmp >= 0 && state->board.stats[tmp].leader < 0) {
						stat->follower = tmp;
					}
				} else if (state->board.tiles[tx + iy][ty + ix].element == ZOO_E_CENTIPEDE_SEGMENT) {
					tmp = zoo_stat_get_id(state, tx + iy, ty + ix);
					if (tmp >= 0 && state->board.stats[tmp].leader < 0) {
						stat->follower = tmp;
					}
				}
			}

			if (stat->follower > 0) {
				stat2 = &(state->board.stats[stat->follower]);
				stat2->leader = stat_id;
				stat2->p1 = stat->p1;
				stat2->p2 = stat->p2;
				stat2->step_x = tx - stat2->x;
				stat2->step_y = ty - stat2->y;
				zoo_stat_move(state, stat->follower, tx, ty);
			}
			stat_id = stat->follower;
		} while (stat_id != -1);
	}
}

static void zoo_e_centipede_segment_tick(zoo_state *state, int16_t stat_id) {
	TICK_GET_SELF;

	if (stat->leader < 0) {
		if (stat->leader < -1) {
			state->board.tiles[stat->x][stat->y].element = ZOO_E_CENTIPEDE_HEAD;
		} else {
			stat->leader -= 1;
		}
	}
}

static void zoo_e_bullet_tick(zoo_state *state, int16_t stat_id) {
	int16_t ix, iy;
	int16_t i_stat;
	uint8_t i_elem;
	bool first_try = true;
	TICK_GET_SELF;

TryMove:
	ix = stat->x + stat->step_x;
	iy = stat->y + stat->step_y;
	i_elem = state->board.tiles[ix][iy].element;

	if (zoo_element_defs[i_elem].walkable || i_elem == ZOO_E_WATER) {
		zoo_stat_move(state, stat_id, ix, iy);
		return;
	}

	if (i_elem == ZOO_E_RICOCHET && first_try) {
		stat->step_x = -stat->step_x;
		stat->step_y = -stat->step_y;
		zoo_sound_queue_const(&(state->sound), 1, "\xF9\x01");
		first_try = false;
		goto TryMove;
	}

	if (i_elem == ZOO_E_BREAKABLE || (zoo_element_defs[i_elem].destructible && (i_elem == ZOO_E_PLAYER || stat->p1 == 0))) {
		if (zoo_element_defs[i_elem].score_value != 0) {
			state->world.info.score += zoo_element_defs[i_elem].score_value;
			state->func_ui_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_SCORE);
		}
		zoo_board_attack_tile(state, stat_id, ix, iy);
		return;
	}

	if (state->board.tiles[stat->x + stat->step_y][stat->y + stat->step_x].element == ZOO_E_RICOCHET && first_try) {
		ix = stat->step_x;
		stat->step_x = -stat->step_y;
		stat->step_y = -ix;
		zoo_sound_queue_const(&(state->sound), 1, "\xF9\x01");
		first_try = false;
		goto TryMove;
	}

	if (state->board.tiles[stat->x - stat->step_y][stat->y - stat->step_x].element == ZOO_E_RICOCHET && first_try) {
		ix = stat->step_x;
		stat->step_x = stat->step_y;
		stat->step_y = ix;
		zoo_sound_queue_const(&(state->sound), 1, "\xF9\x01");
		first_try = false;
		goto TryMove;
	}

	zoo_stat_remove(state, stat_id);
	state->current_stat_tick--;
	if (i_elem == ZOO_E_OBJECT || i_elem == ZOO_E_SCROLL) {
		i_stat = zoo_stat_get_id(state, ix, iy);
		zoo_oop_send(state, -i_stat, "SHOT", false);
	}
}

static void zoo_e_spinning_gun_draw(zoo_state *state, int16_t x, int16_t y, uint8_t *ch) {
	switch (state->current_tick % 8) {
		case 0: case 1: *ch = '\x18'; break;
		case 2: case 3: *ch = '\x1A'; break;
		case 4: case 5: *ch = '\x19'; break;
		default: *ch = '\x1B'; break;
	}
}

static void zoo_e_line_draw(zoo_state *state, int16_t x, int16_t y, uint8_t *ch) {
	int16_t i, v = 0, shift = 1;

	for (i = 0; i < 4; i++, shift <<= 1) {
		switch (state->board.tiles[x + zoo_neighbor_delta_x[i]][y + zoo_neighbor_delta_y[i]].element) {
			case ZOO_E_LINE:
			case ZOO_E_BOARD_EDGE:
				v |= shift;
				break;
		}
	}

	*ch = zoo_line_chars[v];
}

static void zoo_e_spinning_gun_tick(zoo_state *state, int16_t stat_id) {
	bool shot;
	int16_t dx, dy;
	uint8_t element;
	TICK_GET_SELF;

	zoo_board_draw_tile(state, stat->x, stat->y);
	element = (stat->p2 & 0x80) ? ZOO_E_STAR : ZOO_E_BULLET;

	if (state->func_random(9) < (stat->p2 & 0x7F)) {
		if (state->func_random(9) <= stat->p1) {
			if (zoo_difference(stat->x, state->board.stats[0].x) <= 2) {
				shot = zoo_board_shoot(state, element, stat->x, stat->y,
					0, zoo_signum(state->board.stats[0].y - stat->y), 1);
			} else {
				shot = false;
			}

			if (!shot) {
				if (zoo_difference(stat->x, state->board.stats[0].x) <= 2) {
					shot = zoo_board_shoot(state, element, stat->x, stat->y,
						zoo_signum(state->board.stats[0].x - stat->x), 0, 1);
				}
			}
		} else {
			zoo_calc_direction_rnd(state, &dx, &dy);
			shot = zoo_board_shoot(state, element, stat->x, stat->y, dx, dy, 1);
		}
	}
}
static void zoo_conveyor_tick(zoo_state *state, int16_t x, int16_t y, int16_t dir) {
	int16_t i;
	int16_t i_stat;
	int16_t ix, iy;
	bool can_move;
	zoo_tile tiles[8];
	int16_t i_min, i_max;
	zoo_tile tmp_tile;

	i_min = (dir == 1) ? 0 : 7;
	i_max = (dir == 1) ? 8 : -1;

	can_move = true;
	for (i = i_min; i != i_max; i += dir) {
		tiles[i] = state->board.tiles[x + zoo_diagonal_delta_x[i]][y + zoo_diagonal_delta_y[i]];
		if (tiles[i].element == ZOO_E_EMPTY) {
			can_move = true;
		} else if (!zoo_element_defs[tiles[i].element].pushable) {
			can_move = false;
		}
	}

	for (i = i_min; i != i_max; i += dir) {
		if (can_move) {
			if (zoo_element_defs[tiles[i].element].pushable) {
				ix = x + zoo_diagonal_delta_x[(i - dir) & 7];
				iy = y + zoo_diagonal_delta_y[(i - dir) & 7];
				if (zoo_element_defs[tiles[i].element].cycle >= 0) {
					tmp_tile = state->board.tiles[x + zoo_diagonal_delta_x[i]][y + zoo_diagonal_delta_y[i]];
					i_stat = zoo_stat_get_id(state, x + zoo_diagonal_delta_x[i], y + zoo_diagonal_delta_y[i]);
					state->board.tiles[x + zoo_diagonal_delta_x[i]][y + zoo_diagonal_delta_y[i]] = tiles[i];
					state->board.tiles[ix][iy].element = ZOO_E_EMPTY;
					zoo_stat_move(state, i_stat, ix, iy);
					state->board.tiles[x + zoo_diagonal_delta_x[i]][y + zoo_diagonal_delta_y[i]] = tmp_tile;
				} else {
					state->board.tiles[ix][iy] = tiles[i];
					zoo_board_draw_tile(state, ix, iy);
				}

				if (!zoo_element_defs[tiles[(i + dir) & 7].element].pushable) {
					state->board.tiles[x + zoo_diagonal_delta_x[i]][y + zoo_diagonal_delta_y[i]].element = ZOO_E_EMPTY;
					zoo_board_draw_tile(state, x + zoo_diagonal_delta_x[i], y + zoo_diagonal_delta_y[i]);
				}
			} else {
				can_move = false;
			}
		} else if (tiles[i].element == ZOO_E_EMPTY) {
			can_move = true;
		} else if (!zoo_element_defs[tiles[i].element].pushable) {
			can_move = false;
		}
	}
}

static void zoo_e_conveyor_cw_draw(zoo_state *state, int16_t x, int16_t y, uint8_t *ch) {
	switch ((state->current_tick / zoo_element_defs[ZOO_E_CONVEYOR_CW].cycle) & 3) {
		case 0: *ch = '\xB3'; break;
		case 1: *ch = '\x2F'; break;
		case 2: *ch = '\xC4'; break;
		default: *ch = '\x5C'; break;
	}
}

static void zoo_e_conveyor_cw_tick(zoo_state *state, int16_t stat_id) {
	TICK_GET_SELF;
	zoo_board_draw_tile(state, stat->x, stat->y);
	zoo_conveyor_tick(state, stat->x, stat->y, 1);
}

static void zoo_e_conveyor_ccw_draw(zoo_state *state, int16_t x, int16_t y, uint8_t *ch) {
	switch ((state->current_tick / zoo_element_defs[ZOO_E_CONVEYOR_CW].cycle) & 3) {
		case 3: *ch = '\xB3'; break;
		case 2: *ch = '\x2F'; break;
		case 1: *ch = '\xC4'; break;
		default: *ch = '\x5C'; break;
	}
}

static void zoo_e_conveyor_ccw_tick(zoo_state *state, int16_t stat_id) {
	TICK_GET_SELF;
	zoo_board_draw_tile(state, stat->x, stat->y);
	zoo_conveyor_tick(state, stat->x, stat->y, -1);
}

static void zoo_e_bomb_draw(zoo_state *state, int16_t x, int16_t y, uint8_t *ch) {
	zoo_stat *stat = zoo_stat_get(state, x, y, NULL);

	// FIX: bounds check
	if (stat == NULL || stat->p1 <= 1) {
		*ch = 11;
	} else {
		*ch = 48 + stat->p1;
	}
}

static void zoo_e_bomb_tick(zoo_state *state, int16_t stat_id) {
	int16_t old_x, old_y;
	TICK_GET_SELF;

	if (stat->p1 > 0) {
		stat->p1--;
		zoo_board_draw_tile(state, stat->x, stat->y);

		if (stat->p1 == 1) {
			zoo_sound_queue_const(&(state->sound), 1, "\x60\x01\x50\x01\x40\x01\x30\x01\x20\x01\x10\x01");
			zoo_draw_player_surroundings(state, stat->x, stat->y, 1);
		} else if (stat->p1 == 0) {
			old_x = stat->x;
			old_y = stat->y;
			zoo_stat_remove(state, stat_id);
			zoo_draw_player_surroundings(state, old_x, old_y, 2);
		} else {
			zoo_sound_queue_const(&(state->sound), 1, ((stat->p1 & 1) == 0) ? "\xF8\x01" : "\xF5\x01");
		}
	}
}

static void zoo_e_bomb_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	zoo_stat *stat = zoo_stat_get(state, x, y, NULL);
	// FIX: bounds check
	if (stat != NULL) {
		if (stat->p1 == 0) {
			stat->p1 = 9;
			zoo_board_draw_tile(state, stat->x, stat->y);
			zoo_display_message(state, 200, "Bomb activated!");
			zoo_sound_queue_const(&(state->sound), 4, "\x30\x01\x35\x01\x40\x01\x45\x01\x50\x01");
		} else {
			zoo_element_push(state, x, y, *dx, *dy);
		}
	}
}

static void zoo_e_transporter_move(zoo_state *state, int16_t x, int16_t y, int16_t dx, int16_t dy) {
	int16_t ix, iy;
	int16_t new_x, new_y;
	uint8_t *ielem;
	bool finish_search;
	bool is_valid_dest;
	zoo_stat *istat;
	zoo_stat *stat = zoo_stat_get(state, x + dx, y + dy, NULL);

	// FIX: bounds check
	if (stat != NULL && stat->step_x == dx && stat->step_y == dy) {
		ix = stat->x;
		iy = stat->y;
		new_x = -1;
		finish_search = false;
		is_valid_dest = true;
		do {
			ix += dx;
			iy += dy;
			ielem = &state->board.tiles[ix][iy].element;
			if (*ielem == ZOO_E_BOARD_EDGE) {
				finish_search = true;
			} else if (is_valid_dest) {
				is_valid_dest = false;

				if (!zoo_element_defs[*ielem].walkable) {
					zoo_element_push(state, ix, iy, dx, dy);
				}

				if (zoo_element_defs[*ielem].walkable) {
					finish_search = true;
					new_x = ix;
					new_y = iy;
				} else {
					new_x = -1;
				}
			}

			if (*ielem == ZOO_E_TRANSPORTER) {
				istat = zoo_stat_get(state, ix, iy, NULL);
				// FIX: bounds check
				if (istat != NULL && istat->step_x == -dx && istat->step_y == -dy) {
					is_valid_dest = true;
				}
			}
		} while (!finish_search);
		if (new_x != -1) {
			zoo_element_move(state, stat->x - dx, stat->y - dy, new_x, new_y);
			zoo_sound_queue_const(&(state->sound), 3, "\x30\x01\x42\x01\x34\x01\x46\x01\x38\x01\x4A\x01\x40\x01\x52\x01");
		}
	}
}

static const char *zoo_e_transporter_ns_chars = "^~^-v_v-";
static const char *zoo_e_transporter_ew_chars = "(<(\xB3)>)\xB3";

static void zoo_e_transporter_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	zoo_e_transporter_move(state, x - *dx, y - *dy, *dx, *dy);
	*dx = 0;
	*dy = 0;
}

static void zoo_e_transporter_tick(zoo_state *state, int16_t stat_id) {
	TICK_GET_SELF;
	zoo_board_draw_tile(state, stat->x, stat->y);
}

static void zoo_e_transporter_draw(zoo_state *state, int16_t x, int16_t y, uint8_t *ch) {
	zoo_stat *stat = zoo_stat_get(state, x, y, NULL);

	// FIX: bounds check (zoo_signum changes behaviour slightly in edge cases)
	if (stat != NULL) {
		if (stat->step_x == 0) {
			*ch = zoo_e_transporter_ns_chars[(zoo_signum(stat->step_y) + 1) * 2 + ((state->current_tick / stat->cycle) & 3)];
		} else {
			*ch = zoo_e_transporter_ew_chars[(zoo_signum(stat->step_x) + 1) * 2 + ((state->current_tick / stat->cycle) & 3)];
		}
	}
}

static const char *zoo_e_star_chars = "\xB3\x2F\xC4\x5C";

static void zoo_e_star_draw(zoo_state *state, int16_t x, int16_t y, uint8_t *ch) {
	uint8_t *col = &state->board.tiles[x][y].color;

	*ch = zoo_e_star_chars[state->current_tick & 3];
	*col += 1;
	if (*col > 15) *col = 9;
}

static void zoo_e_star_tick(zoo_state *state, int16_t stat_id) {
	uint8_t *i_elem;
	TICK_GET_SELF;

	stat->p2--;
	if (stat->p2 <= 0) {
		zoo_stat_remove(state, stat_id);
	} else {
		if ((stat->p2 & 1) == 0) {
			zoo_calc_direction_seek(state, stat->x, stat->y, &stat->step_x, &stat->step_y);
			i_elem = &state->board.tiles[stat->x + stat->step_x][stat->y + stat->step_y].element;
			if (*i_elem == ZOO_E_PLAYER || *i_elem == ZOO_E_BREAKABLE) {
				zoo_board_attack_tile(state, stat_id, stat->x + stat->step_x, stat->y + stat->step_y);
			} else {
				if (!zoo_element_defs[*i_elem].walkable) {
					zoo_element_push(state, stat->x + stat->step_x, stat->y + stat->step_y, stat->step_x, stat->step_y);
				}

				if (zoo_element_defs[*i_elem].walkable || *i_elem == ZOO_E_WATER) {
					zoo_stat_move(state, stat_id, stat->x + stat->step_x, stat->y + stat->step_y);
				}
			}
		} else {
			zoo_board_draw_tile(state, stat->x, stat->y);
		}
	}
}

static void zoo_e_energizer_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	zoo_sound_queue_const(&(state->sound), 9,
		"\x20\x03\x23\x03\x24\x03\x25\x03\x35\x03\x25\x03\x23\x03\x20\x03"
		"\x30\x03\x23\x03\x24\x03\x25\x03\x35\x03\x25\x03\x23\x03\x20\x03"
		"\x30\x03\x23\x03\x24\x03\x25\x03\x35\x03\x25\x03\x23\x03\x20\x03"
		"\x30\x03\x23\x03\x24\x03\x25\x03\x35\x03\x25\x03\x23\x03\x20\x03"
		"\x30\x03\x23\x03\x24\x03\x25\x03\x35\x03\x25\x03\x23\x03\x20\x03"
		"\x30\x03\x23\x03\x24\x03\x25\x03\x35\x03\x25\x03\x23\x03\x20\x03"
		"\x30\x03\x23\x03\x24\x03\x25\x03\x35\x03\x25\x03\x23\x03\x20\x03");

	state->board.tiles[x][y].element = ZOO_E_EMPTY;
	zoo_board_draw_tile(state, x, y);

	state->world.info.energizer_ticks = 75;
	state->func_ui_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_ALL);

	if (state->msg_flags.energizer) {
		zoo_display_message(state, 200, "Energizer - You are invincible");
		state->msg_flags.energizer = false;
	}

	zoo_oop_send(state, 0, "ALL:ENERGIZE", false);
}

static void zoo_e_slime_tick(zoo_state *state, int16_t stat_id) {
	int16_t dir, color, changed_tiles;
	int16_t start_x, start_y;
	TICK_GET_SELF;

	if (stat->p1 < stat->p2) {
		stat->p1++;
	} else {
		color = state->board.tiles[stat->x][stat->y].color;
		stat->p1 = 0;
		start_x = stat->x;
		start_y = stat->y;
		changed_tiles = 0;

		for (dir = 0; dir < 4; dir++) {
			if (zoo_element_defs[state->board.tiles
				[start_x + zoo_neighbor_delta_x[dir]]
				[start_y + zoo_neighbor_delta_y[dir]]
			.element].walkable) {
				if (changed_tiles == 0) {
					zoo_stat_move(state, stat_id, start_x + zoo_neighbor_delta_x[dir], start_y + zoo_neighbor_delta_y[dir]);
					state->board.tiles[start_x][start_y].element = ZOO_E_BREAKABLE;
					state->board.tiles[start_x][start_y].color = color;
					zoo_board_draw_tile(state, start_x, start_y);
				} else {
					zoo_stat_add(state, start_x + zoo_neighbor_delta_x[dir], start_y + zoo_neighbor_delta_y[dir],
						ZOO_E_SLIME, color, zoo_element_defs[ZOO_E_SLIME].cycle, &zoo_stat_template_default);
				}

				changed_tiles++;
			}
		}

		if (changed_tiles == 0) {
			zoo_stat_remove(state, stat_id);
			state->board.tiles[start_x][start_y].element = ZOO_E_BREAKABLE;
			state->board.tiles[start_x][start_y].color = color;
			zoo_board_draw_tile(state, start_x, start_y);
		}
	}
}

static void zoo_e_slime_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	uint8_t color = state->board.tiles[x][y].color;
	zoo_board_damage_stat(state, zoo_stat_get_id(state, x, y));
	state->board.tiles[x][y].element = ZOO_E_BREAKABLE;
	state->board.tiles[x][y].color = color;
	zoo_board_draw_tile(state, x, y);
	zoo_sound_queue_const(&(state->sound), 2, "\x20\x01\x23\x01");
}

static void zoo_e_shark_tick(zoo_state *state, int16_t stat_id) {
	int16_t dx, dy;
	TICK_GET_SELF;

	if (stat->p1 < state->func_random(10)) {
		zoo_calc_direction_rnd(state, &dx, &dy);
	} else {
		zoo_calc_direction_seek(state, stat->x, stat->y, &dx, &dy);
	}

	switch (state->board.tiles[stat->x + dx][stat->y + dy].element) {
		case ZOO_E_WATER: {
			zoo_stat_move(state, stat_id, stat->x + dx, stat->y + dy);
		} break;
		case ZOO_E_PLAYER: {
			zoo_board_attack_tile(state, stat_id, stat->x + dx, stat->y + dy);
		} break;
	}
}

static void zoo_e_blink_wall_draw(zoo_state *state, int16_t x, int16_t y, uint8_t *ch) {
	*ch = '\xCE';
}

static void zoo_e_blink_wall_tick(zoo_state *state, int16_t stat_id) {
	int16_t ix, iy;
	bool hit_boundary;
	int16_t player_stat_id;
	int16_t el;
	uint8_t *i_elem;
	TICK_GET_SELF;

	if (stat->p3 == 0) {
		stat->p3 = stat->p1 + 1;
	}

	if (stat->p3 == 1) {
		ix = stat->x + stat->step_x;
		iy = stat->y + stat->step_y;

		el = (stat->step_x != 0) ? ZOO_E_BLINK_RAY_EW : ZOO_E_BLINK_RAY_NS;

		while (state->board.tiles[ix][iy].element == el
			&& state->board.tiles[ix][iy].color == state->board.tiles[stat->x][stat->y].color)
		{
			state->board.tiles[ix][iy].element = ZOO_E_EMPTY;
			zoo_board_draw_tile(state, ix, iy);
			ix += stat->step_x;
			iy += stat->step_y;
			stat->p3 = stat->p2 * 2 + 1;
		}

		if ((stat->x + stat->step_x) == ix && (stat->y + stat->step_y) == iy) {
			hit_boundary = false;
			do {
				i_elem = &(state->board.tiles[ix][iy].element);
				if (*i_elem != ZOO_E_EMPTY && zoo_element_defs[*i_elem].destructible) {
					zoo_board_damage_tile(state, ix, iy);
				}

				if (*i_elem == ZOO_E_PLAYER) {
					player_stat_id = zoo_stat_get_id(state, ix, iy);

					if (stat->step_x != 0) {
						if (state->board.tiles[ix][iy - 1].element == ZOO_E_EMPTY) {
							zoo_stat_move(state, player_stat_id, ix, iy - 1);
						} else if (state->board.tiles[ix][iy + 1].element == ZOO_E_EMPTY) {
							zoo_stat_move(state, player_stat_id, ix, iy + 1);
						}
					} else {
						if (state->board.tiles[ix + 1][iy].element == ZOO_E_EMPTY) {
							zoo_stat_move(state, player_stat_id, ix + 1, iy);
						} else if (state->board.tiles[ix - 1][iy].element == ZOO_E_EMPTY) {
							zoo_stat_move(state, player_stat_id, ix + 1, iy);
						}
					}

					if (*i_elem == ZOO_E_PLAYER) {
						while (state->world.info.health > 0) {
							zoo_board_damage_stat(state, player_stat_id);
						}
						hit_boundary = true;
					}
				}

				if (*i_elem == ZOO_E_EMPTY) {
					state->board.tiles[ix][iy].element = el;
					state->board.tiles[ix][iy].color = state->board.tiles[stat->x][stat->y].color;
					zoo_board_draw_tile(state, ix, iy);
				} else {
					hit_boundary = true;
				}

				ix += stat->step_x;
				iy += stat->step_y;
			} while (!hit_boundary);

			stat->p3 = (stat->p2 * 2) + 1;
		}
	} else {
		stat->p3--;
	}
}

void zoo_element_move(zoo_state *state, int16_t old_x, int16_t old_y, int16_t new_x, int16_t new_y) {
	int16_t stat_id = zoo_stat_get_id(state, old_x, old_y);
	if (stat_id >= 0) {
		zoo_stat_move(state, stat_id, new_x, new_y);
	} else {
		state->board.tiles[new_x][new_y] = state->board.tiles[old_x][old_y];
		zoo_board_draw_tile(state, new_x, new_y);
		state->board.tiles[old_x][old_y].element = ZOO_E_EMPTY;
		zoo_board_draw_tile(state, old_x, old_y);
	}
}

void zoo_element_push(zoo_state *state, int16_t x, int16_t y, int16_t dx, int16_t dy) {
	uint8_t *e, *de;

	e = &state->board.tiles[x][y].element;
	if ((*e == ZOO_E_SLIDER_NS && dx == 0) || (*e == ZOO_E_SLIDER_EW && dy == 0)
		|| zoo_element_defs[*e].pushable)
	{
		de = &state->board.tiles[x+dx][y+dy].element;
		if (*de == ZOO_E_TRANSPORTER) {
			zoo_e_transporter_move(state, x, y, dx, dy);
		} else if (*de != ZOO_E_EMPTY) {
			zoo_element_push(state, x + dx, y + dy, dx, dy);
		}

		if (
			!zoo_element_defs[*de].walkable
			&& zoo_element_defs[*de].destructible
			&& (*de != ZOO_E_PLAYER)
		) {
			zoo_board_damage_tile(state, x + dx, y + dy);
		}

		if (zoo_element_defs[*de].walkable) {
			zoo_element_move(state, x, y, x + dx, y + dy);
		}
	}
}

static void zoo_e_duplicator_draw(zoo_state *state, int16_t x, int16_t y, uint8_t *ch) {
	zoo_stat *stat = zoo_stat_get(state, x, y, NULL);

	// FIX: bounds check
	switch (stat != NULL ? stat->p1 : 0) {
		default:
		case 1: *ch = '\xFA'; break;
		case 2: *ch = '\xF9'; break;
		case 3: *ch = '\xF8'; break;
		case 4: *ch = '\x6F'; break;
		case 5: *ch = '\x4F'; break;
	}
}

static void zoo_e_object_tick(zoo_state *state, int16_t stat_id) {
	TICK_GET_SELF;
	zoo_call *call;

	zoo_oop_execute(state, stat_id, &stat->data_pos, "Interaction");
	if (state->object_window_request) {
		// push self
		call = zoo_call_push(&state->call_stack, TICK_FUNC, 1);
		call->args.tick.func = zoo_e_object_tick;
		call->args.tick.stat_id = stat_id;
		// push text window
		state->func_ui_open_window(state, &state->object_window);
		// return
		state->object_window_request = false;
		return;
	}

	if (stat->step_x != 0 || stat->step_y != 0) {
		if (zoo_element_defs[state->board.tiles[stat->x + stat->step_x][stat->y + stat->step_y].element].walkable) {
			zoo_stat_move(state, stat_id, stat->x + stat->step_x, stat->y + stat->step_y);
		} else {
			zoo_oop_send(state, -stat_id, "THUD", false);
		}
	}
}

static void zoo_e_object_draw(zoo_state *state, int16_t x, int16_t y, uint8_t *ch) {
	zoo_stat *stat = zoo_stat_get(state, x, y, NULL);
	*ch = stat != NULL ? stat->p1 : 0;
}

static void zoo_e_object_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	zoo_oop_send(state, -zoo_stat_get_id(state, x, y), "TOUCH", false);
}

static void zoo_e_duplicator_tick(zoo_state *state, int16_t stat_id) {
	zoo_call *call;
	zoo_tile *target_tile;
	int16_t source_stat_id;
	zoo_stat *source_stat;
	TICK_GET_SELF;
	CALL_STATE_GET_SELF;
	switch (call_state) {
		case 1:
			goto DuplicatorTickState1;
	}

	if (stat->p1 <= 4) {
		stat->p1++;
	} else {
		stat->p1 = 0;
		target_tile = &state->board.tiles[stat->x - stat->step_x][stat->y - stat->step_y];
		if (target_tile->element == ZOO_E_PLAYER) {
			// push self
			call = zoo_call_push(&state->call_stack, TICK_FUNC, 1);
			call->args.tick.func = zoo_e_duplicator_tick;
			call->args.tick.stat_id = stat_id;
			// touch
			zoo_element_defs[state->board.tiles[stat->x + stat->step_x][stat->y + stat->step_y].element]
				.touch_func(state, stat->x + stat->step_x, stat->y + stat->step_y,
					0, &state->input.delta_x, &state->input.delta_y);
			// return
			return;
		} else {
			if (target_tile->element != ZOO_E_EMPTY) {
				zoo_element_push(state, stat->x - stat->step_x, stat->y - stat->step_y,
					-stat->step_x, -stat->step_y);
			}

			if (target_tile->element == ZOO_E_EMPTY) {
				source_stat = zoo_stat_get(state, stat->x + stat->step_x, stat->y + stat->step_y, &source_stat_id);

				// FIX: allow max_stat > 174
				if (source_stat_id > 0) {
					if (state->board.stat_count < ((ZOO_MAX_STAT > 174) ? ZOO_MAX_STAT : 174)) {
						zoo_stat_add(state, stat->x - stat->step_x, stat->y - stat->step_y,
							state->board.tiles[stat->x + stat->step_x][stat->y + stat->step_y].element,
							state->board.tiles[stat->x + stat->step_x][stat->y + stat->step_y].color,
							source_stat->cycle, source_stat);

						zoo_board_draw_tile(state, stat->x - stat->step_x, stat->y - stat->step_y);
					}
				} else if (source_stat_id != 0) {
					*target_tile = state->board.tiles[stat->x + stat->step_x][stat->y + stat->step_y];
					zoo_board_draw_tile(state, stat->x - stat->step_x, stat->y - stat->step_y);
				}

				zoo_sound_queue_const(&(state->sound), 3, "\x30\x02\x32\x02\x34\x02\x35\x02\x37\x02");
			} else {
				zoo_sound_queue_const(&(state->sound), 3, "\x18\x01\x16\x01");
			}
		}

DuplicatorTickState1:
		stat->p1 = 0;
	}

	zoo_board_draw_tile(state, stat->x, stat->y);
	stat->cycle = (9 - stat->p2) * 3;
}

static void zoo_e_scroll_tick(zoo_state *state, int16_t stat_id) {
	TICK_GET_SELF;
	uint8_t *col = &state->board.tiles[stat->x][stat->y].color;

	*col += 1;
	if (*col > 15) *col = 9;
	zoo_board_draw_tile(state, stat->x, stat->y);
}

static void zoo_e_scroll_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	// TODO: pre-calculate buffer
	uint8_t buf[64];
	zoo_call *call;
	int16_t stat_id;
	zoo_stat *stat = zoo_stat_get(state, x, y, &stat_id);
	CALL_STATE_GET_SELF;

	if (stat == NULL) {
		return;
	}
	switch (call_state) {
		case 1:
			goto ScrollTouchState1;
	}

	zoo_sound_queue(&(state->sound), 2, buf, zoo_sound_parse("c-c+d-d+e-e+f-f+g-g", buf, sizeof(buf) - 1));

	stat->data_pos = 0;
	zoo_oop_execute(state, stat_id, &stat->data_pos, "Interaction");

	if (state->object_window_request) {
		// push self
		call = zoo_call_push(&state->call_stack, TOUCH_FUNC, 1);
		call->args.touch.func = zoo_e_scroll_touch;
		CALL_SET_TOUCH_ARGS(call);
		// push text window
		state->func_ui_open_window(state, &state->object_window);
		// return
		state->object_window_request = false;
		return;
	}

ScrollTouchState1:
	zoo_stat_remove(state, stat_id);
}

// FIX: macros to portably handle black key (= 256 gems)

#define ZOO_IS_KEY_SET(k) ((k) > 0 ? state->world.info.keys[(k) - 1] : (((state->world.info.gems >> 8) & 0xFF) != 0))
#define ZOO_SET_KEY(k) if ((k) > 0) { state->world.info.keys[(k) - 1] = true; } \
	else { state->world.info.gems = (state->world.info.gems & 0xFF) | 0x100; }
#define ZOO_CLEAR_KEY(k) if ((k) > 0) { state->world.info.keys[(k) - 1] = false; } \
	else { state->world.info.gems &= 0xFF; }

static void zoo_e_key_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	char msg[ZOO_LEN_MESSAGE + 1];
	int16_t key = state->board.tiles[x][y].color & 7;

	if (ZOO_IS_KEY_SET(key)) {
		strncpy(msg, "You already have a ", ZOO_LEN_MESSAGE);
		strncat(msg, zoo_color_names[key], ZOO_LEN_MESSAGE);
		strncat(msg, " key!", ZOO_LEN_MESSAGE);
		zoo_display_message(state, 200, msg);
		zoo_sound_queue_const(&(state->sound), 2, "\x30\x02\x20\x02");
	} else {
		ZOO_SET_KEY(key);
		state->board.tiles[x][y].element = ZOO_E_EMPTY;
		state->func_ui_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_KEYS);
		strncpy(msg, "You now have the ", ZOO_LEN_MESSAGE);
		strncat(msg, zoo_color_names[key], ZOO_LEN_MESSAGE);
		strncat(msg, " key.", ZOO_LEN_MESSAGE);
		zoo_display_message(state, 200, msg);
		zoo_sound_queue_const(&(state->sound), 2, "\x40\x01\x44\x01\x47\x01\x40\x01\x44\x01\x47\x01\x40\x01\x44\x01\x47\x01\x50\x02");
	}
}

static void zoo_e_ammo_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	state->world.info.ammo += 5;

	state->board.tiles[x][y].element = ZOO_E_EMPTY;
	state->func_ui_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_AMMO);
	zoo_sound_queue_const(&(state->sound), 2, "\x30\x01\x31\x01\x32\x01");

	if (state->msg_flags.ammo) {
		state->msg_flags.ammo = false;
		zoo_display_message(state, 200, "Ammunition - 5 shots per container.");
	}
}

static void zoo_e_gem_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	state->world.info.gems += 1;
	state->world.info.health += 1;
	state->world.info.score += 10;

	state->board.tiles[x][y].element = ZOO_E_EMPTY;
	if (state->world.info.health < 0) {
		state->world.info.health = 0;
	}
	state->func_ui_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_GEMS | ZOO_SIDEBAR_UPDATE_HEALTH | ZOO_SIDEBAR_UPDATE_SCORE);
	zoo_sound_queue_const(&(state->sound), 2, "\x40\x01\x37\x01\x34\x01\x30\x01");

	if (state->msg_flags.gem) {
		state->msg_flags.gem = false;
		zoo_display_message(state, 200, "Gems give you Health!");
	}
}

static void zoo_e_passage_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	zoo_board_passage_teleport(state, x, y);
	*dx = 0;
	*dy = 0;
}

static void zoo_e_door_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	char msg[ZOO_LEN_MESSAGE + 1];
	int16_t key = (state->board.tiles[x][y].color >> 4) & 7;

	if (ZOO_IS_KEY_SET(key)) {
		state->board.tiles[x][y].element = ZOO_E_EMPTY;
		zoo_board_draw_tile(state, x, y);

		ZOO_CLEAR_KEY(key);
		state->func_ui_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_KEYS);

		strncpy(msg, "The ", ZOO_LEN_MESSAGE);
		strncat(msg, zoo_color_names[key], ZOO_LEN_MESSAGE);
		strncat(msg, " door is now open.", ZOO_LEN_MESSAGE);
		zoo_display_message(state, 200, msg);
		zoo_sound_queue_const(&(state->sound), 3, "\x30\x01\x37\x01\x3B\x01\x30\x01\x37\x01\x3B\x01\x40\x04");
	} else {
		strncpy(msg, "The ", ZOO_LEN_MESSAGE);
		strncat(msg, zoo_color_names[key], ZOO_LEN_MESSAGE);
		strncat(msg, " door is locked!", ZOO_LEN_MESSAGE);
		zoo_display_message(state, 200, msg);
		zoo_sound_queue_const(&(state->sound), 3, "\x17\x01\x10\x01");
	}
}

static void zoo_e_pushable_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	zoo_element_push(state, x, y, *dx, *dy);
	zoo_sound_queue_const(&(state->sound), 2, "\x15\x01");
}

static void zoo_e_pusher_draw(zoo_state *state, int16_t x, int16_t y, uint8_t *ch) {
	zoo_stat *stat = zoo_stat_get(state, x, y, NULL);

	// FIX: bounds check
	if (stat == NULL) {
		*ch = 31;
	} else if (stat->step_x == 1) {
		*ch = 16;
	} else if (stat->step_x == -1) {
		*ch = 17;
	} else if (stat->step_y == -1) {
		*ch = 30;
	} else {
		*ch = 31;
	}
}

static void zoo_e_pusher_tick(zoo_state *state, int16_t stat_id) {
	int16_t start_x, start_y, n_stat_id;
	zoo_stat *n_stat;
	zoo_stat *stat = &state->board.stats[stat_id];

	start_x = stat->x;
	start_y = stat->y;
	if (!zoo_element_defs[state->board.tiles[stat->x + stat->step_x][stat->y + stat->step_y].element].walkable) {
		zoo_element_push(state, stat->x + stat->step_x, stat->y + stat->step_y, stat->step_x, stat->step_y);
	}

	stat = zoo_stat_get(state, start_x, start_y, &stat_id);
	// FIX: bounds check
	if (stat != NULL) {
		if (zoo_element_defs[state->board.tiles[stat->x + stat->step_x][stat->y + stat->step_y].element].walkable) {
			zoo_stat_move(state, stat_id, stat->x + stat->step_x, stat->y + stat->step_y);
			zoo_sound_queue_const(&(state->sound), 2, "\x15\x01");

			if (state->board.tiles[stat->x - (stat->step_x * 2)][stat->y - (stat->step_y * 2)].element == ZOO_E_PUSHER) {
				n_stat = zoo_stat_get(state, stat->x - (stat->step_x * 2), stat->y - (stat->step_y * 2), &n_stat_id);
				// FIX: bounds check
				if (n_stat != NULL && n_stat->step_x == stat->step_x && n_stat->step_y == stat->step_y) {
					zoo_element_defs[ZOO_E_PUSHER].tick_func(state, n_stat_id);
				}
			}
		}
	}
}

static void zoo_e_torch_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	state->world.info.torches += 1;
	state->board.tiles[x][y].element = ZOO_E_EMPTY;

	zoo_board_draw_tile(state, x, y);
	state->func_ui_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_TORCHES);

	if (state->msg_flags.torch) {
		zoo_display_message(state, 200, "Torch - used for lighting in the underground.");
	}
	state->msg_flags.torch = false;

	zoo_sound_queue_const(&(state->sound), 3, "\x30\x01\x39\x01\x34\x02");
}

static void zoo_e_invisible_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	state->board.tiles[x][y].element = ZOO_E_EMPTY;
	zoo_board_draw_tile(state, x, y);

	zoo_sound_queue_const(&(state->sound), 3, "\x12\x01\x10\x01");
	zoo_display_message(state, 100, "You are blocked by an invisible wall.");
}

static void zoo_e_forest_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	state->board.tiles[x][y].element = ZOO_E_EMPTY;
	zoo_board_draw_tile(state, x, y);

	zoo_sound_queue_const(&(state->sound), 3, "\x39\x01");

	if (state->msg_flags.forest) {
		zoo_display_message(state, 200, "A path is cleared through the forest.");
	}
	state->msg_flags.forest = false;
}

static void zoo_e_fake_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	if (state->msg_flags.fake) {
		zoo_display_message(state, 200, "A fake wall - secret passage!");
	}
	state->msg_flags.fake = false;
}

static void zoo_e_board_edge_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	zoo_call *call;
	int16_t neighbor_id, board_id;
	int16_t entry_x, entry_y;
	CALL_STATE_GET_SELF;

	if (call_state > 0) {
		call = state->call_stack.curr_call;
		neighbor_id = call->args.touch.extra.board_edge.neighbor_id;
		board_id = call->args.touch.extra.board_edge.board_id;
		entry_x = call->args.touch.extra.board_edge.entry_x;
		entry_y = call->args.touch.extra.board_edge.entry_y;
		switch (call_state) {
			case 1:
				goto BoardEdgeState1;
		}
	}

	entry_x = state->board.stats[0].x;
	entry_y = state->board.stats[0].y;
	if (*dy == -1) {
		neighbor_id = 0;
		entry_y = ZOO_BOARD_HEIGHT;
	} else if (*dy == 1) {
		neighbor_id = 1;
		entry_y = 1;
	} else if (*dx == -1) {
		neighbor_id = 2;
		entry_x = ZOO_BOARD_WIDTH;
	} else {
		neighbor_id = 3;
		entry_x = 1;
	}

	if (state->board.info.neighbor_boards[neighbor_id] != 0) {
		board_id = state->world.info.current_board;
		zoo_board_change(state, state->board.info.neighbor_boards[neighbor_id]);
		if (state->board.tiles[entry_x][entry_y].element != ZOO_E_PLAYER) {
			// push self
			call = zoo_call_push(&state->call_stack, TOUCH_FUNC, 1);
			call->args.touch.func = zoo_e_board_edge_touch;
			CALL_SET_TOUCH_ARGS(call);
			call->args.touch.extra.board_edge.neighbor_id = neighbor_id;
			call->args.touch.extra.board_edge.board_id = board_id;
			call->args.touch.extra.board_edge.entry_x = entry_x;
			call->args.touch.extra.board_edge.entry_y = entry_y;
			// touch
			zoo_element_defs[state->board.tiles[entry_x][entry_y].element]
				.touch_func(state, entry_x, entry_y, source_stat_id,
					&state->input.delta_x, &state->input.delta_y);
			// return
			return;
		}

BoardEdgeState1:
		if (
			zoo_element_defs[state->board.tiles[entry_x][entry_y].element].walkable
			|| (state->board.tiles[entry_x][entry_y].element == ZOO_E_PLAYER)
		) {
			if (state->board.tiles[entry_x][entry_y].element != ZOO_E_PLAYER) {
				zoo_stat_move(state, 0, entry_x, entry_y);
			}

			// TODO Transition
			zoo_board_draw(state);
			*dx = 0;
			*dy = 0;
			zoo_board_enter(state);
		} else {
			// return
			zoo_board_change(state, board_id);
		}
	}
}

static void zoo_e_water_touch(zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy) {
	zoo_sound_queue_const(&(state->sound), 3, "\x40\x01\x50\x01");
	zoo_display_message(state, 100, "Your way is blocked by water.");
}

void zoo_draw_player_surroundings(zoo_state *state, int16_t x, int16_t y, int16_t bomb_phase) {
	int16_t ix, iy, istat;
	zoo_tile *tile;

	for (ix = x - ZOO_TORCH_DX - 1; ix <= x + ZOO_TORCH_DX + 1; ix++)
	if (ix >= 1 && ix <= ZOO_BOARD_WIDTH)
	for (iy = y - ZOO_TORCH_DY - 1; iy <= y + ZOO_TORCH_DY + 1; iy++)
	if (iy >= 1 && iy <= ZOO_BOARD_HEIGHT) {
		if (bomb_phase > 0) {
			if (zoo_dist_sq2(ix - x, iy - y) < ZOO_TORCH_DSQ) {
				tile = &state->board.tiles[ix][iy];
				if (bomb_phase == 1) {
					// if (zoo_element_defs[tile->element].param_text_name[0] != '\0') {
					if (zoo_element_defs[tile->element].has_text) {
						istat = zoo_stat_get_id(state, ix, iy);
						if (istat > 0) {
							zoo_oop_send(state, -istat, "BOMBED", false);
						}
					}

					if (zoo_element_defs[tile->element].destructible) {
						zoo_board_damage_tile(state, ix, iy);
					}

					if (tile->element == ZOO_E_EMPTY || tile->element == ZOO_E_BREAKABLE) {
						tile->element = ZOO_E_BREAKABLE;
						tile->color = 9 + state->func_random(7);
						zoo_board_draw_tile(state, ix, iy);
					}
				} else /* usually bomb_phase == 2 */ {
					if (tile->element == ZOO_E_BREAKABLE) {
						tile->element = ZOO_E_EMPTY;
					}
				}
			}
		}
		zoo_board_draw_tile(state, ix, iy);
	}
}

static void zoo_e_player_draw(zoo_state *state, int16_t x, int16_t y, uint8_t *ch) {
	// CHANGE: energizer blinking logic is moved here to keep zoo_element_defs a const
	if (state->world.info.energizer_ticks > 0) {
		*ch = (state->current_tick & 1) ? '\x02' : '\x01';
	} else {
		*ch = '\x02';
	}
}

static void zoo_e_player_tick(zoo_state *state, int16_t stat_id) {
	zoo_call *call;
	int16_t i;
	int16_t bullet_count;
	TICK_GET_SELF;
	CALL_STATE_GET_SELF;
	switch (call_state) {
		case 1:
			goto PlayerTickState1;
	}

	if (state->world.info.energizer_ticks > 0) {
		if (state->current_tick & 1) {
			state->board.tiles[stat->x][stat->y].color = 0x0F;
		} else {
			state->board.tiles[stat->x][stat->y].color = (((state->current_tick % 7) + 1) << 4) | 0x0F;
		}

		zoo_board_draw_tile(state, stat->x, stat->y);
	} else if (
		state->board.tiles[stat->x][stat->y].color != 0x1F
	) {
		state->board.tiles[stat->x][stat->y].color = 0x1F;
		zoo_board_draw_tile(state, stat->x, stat->y);
	}

	if (state->world.info.health <= 0) {
		state->input.delta_x = 0;
		state->input.delta_y = 0;
		// TODO input refactor
		// state->input.shoot = false;

		if (zoo_stat_get_id(state, 0, 0) == -1) {
			zoo_display_message(state, 32000, " Game over  -  Press ESCAPE");
		}

		state->tick_duration = 0;
		state->sound.block_queueing = true;
	}

	if (state->input.delta_x != 0 || state->input.delta_y != 0) {
		if (zoo_input_action_pressed(&state->input, ZOO_ACTION_SHOOT)) {
			if (state->board.info.max_shots == 0) {
				if (state->msg_flags.no_shooting) {
					zoo_display_message(state, 200, "Can't shoot in this place!");
				}
				state->msg_flags.no_shooting = false;
			} else if (state->world.info.ammo == 0) {
				if (state->msg_flags.out_of_ammo) {
					zoo_display_message(state, 200, "You don't have any ammo!");
				}
				state->msg_flags.out_of_ammo = false;
			} else {
				bullet_count = 0;
				for (i = 0; i <= state->board.stat_count; i++) {
					if (state->board.tiles
						[state->board.stats[i].x]
						[state->board.stats[i].y]
						.element == ZOO_E_BULLET && state->board.stats[i].p1 == 0
					) {
						bullet_count++;
					}
				}
				if (bullet_count < state->board.info.max_shots) {
					if (zoo_board_shoot(state, ZOO_E_BULLET, stat->x, stat->y,
						state->input.delta_x, state->input.delta_y, 0)
					) {
						state->world.info.ammo--;
						state->func_ui_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_AMMO);

						zoo_sound_queue_const(&(state->sound), 2, "\x40\x01\x30\x01\x20\x01");

						state->input.delta_x = 0;
						state->input.delta_y = 0;
					}
				}
			}
		} else {
			// push self
			call = zoo_call_push(&(state->call_stack), TICK_FUNC, 1);
			call->args.tick.func = zoo_e_player_tick;
			call->args.tick.stat_id = stat_id;
			// touch
			zoo_element_defs[state->board.tiles
				[stat->x + state->input.delta_x]
				[stat->y + state->input.delta_y]
			.element].touch_func(state, stat->x + state->input.delta_x, stat->y + state->input.delta_y,
				0, &state->input.delta_x, &state->input.delta_y);
			// return
			// -> player move - part 2
			return;
		}
	}

	if (call_state == 1) {
		// player move - part 2
PlayerTickState1:
		if (state->input.delta_x != 0 || state->input.delta_y != 0) {
			// TODO player move sound
			if (
				zoo_element_defs[state->board.tiles
					[stat->x + state->input.delta_x]
					[stat->y + state->input.delta_y]
				.element].walkable
			) {
				zoo_stat_move(state, 0,
					stat->x + state->input.delta_x,
					stat->y + state->input.delta_y
				);
			}
		}
	}

	if (zoo_input_action_pressed_once(&state->input, ZOO_ACTION_TORCH)) {
		if (state->world.info.torch_ticks <= 0) {
			if (state->world.info.torches > 0) {
				if (state->board.info.is_dark) {
					state->world.info.torches--;
					state->world.info.torch_ticks = ZOO_TORCH_DURATION;

					zoo_draw_player_surroundings(state, stat->x, stat->y, 0);
					state->func_ui_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_TORCHES);
				} else {
					if (state->msg_flags.room_not_dark) {
						zoo_display_message(state, 200, "Don't need torch - room is not dark!");
						state->msg_flags.room_not_dark = false;
					}
				}
			} else {
				if (state->msg_flags.out_of_torches) {
					zoo_display_message(state, 200, "You don't have any torches!");
					state->msg_flags.out_of_torches = false;
				}
			}
		}
	}

	if (state->world.info.torch_ticks > 0) {
		state->world.info.torch_ticks--;
		if (state->world.info.torch_ticks <= 0) {
			zoo_draw_player_surroundings(state, stat->x, stat->y, 0);
			zoo_sound_queue_const(&(state->sound), 3, "\x30\x01\x20\x01\x10\x01");
		}

		// TODO: ZZT normally does this every 40 ticks; maybe another function to check?
		state->func_ui_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_TORCHES);
	}

	if (state->world.info.energizer_ticks > 0) {
		state->world.info.energizer_ticks--;
		if (state->world.info.energizer_ticks == 10) {
			zoo_sound_queue_const(&(state->sound), 9, "\x20\x03\x1A\x03\x17\x03\x16\x03\x15\x03\x13\x03\x10\x03");
		} else if (state->world.info.energizer_ticks <= 0) {
			state->board.tiles[stat->x][stat->y].color = zoo_element_defs[ZOO_E_PLAYER].color;
			zoo_board_draw_tile(state, stat->x, stat->y);
		}
	}

	if ((state->board.info.time_limit_sec > 0) && (state->world.info.health > 0)) {
		if (zoo_has_hsecs_elapsed(state, &state->world.info.board_time_hsec, 100)) {
			state->world.info.board_time_sec++;

			if ((state->board.info.time_limit_sec - 10) == state->world.info.board_time_sec) {
				zoo_display_message(state, 200, "Running out of time!");
				zoo_sound_queue_const(&(state->sound), 3, "\x40\x06\x45\x06\x40\x06\x35\x06\x40\x06\x45\x06\x40\x0A");
			} else if (state->world.info.board_time_sec > state->board.info.time_limit_sec) {
				zoo_board_damage_stat(state, 0);
			}

			state->func_ui_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_TIME);
		}
	}
}

static void zoo_e_monitor_tick(zoo_state *state, int16_t stat_id) {
	// TODO
}

void zoo_reset_message_flags(zoo_state *state) {
	memset(&(state->msg_flags), 1, sizeof(zoo_state_message));
}

#include "zoo_element_defs.inc"
