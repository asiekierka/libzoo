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

#define _GNU_SOURCE

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "zoo_internal.h"
#include "libzoo/zoo_oop_token.c"

#define oop_word_cmp(c) strncmp(state->oop_word, (c), sizeof(state->oop_word) - 1)

void zoo_stat_free(zoo_stat *stat) {
	if (!platform_is_rom_ptr(stat->data))
		free(stat->data);

#ifdef ZOO_USE_LABEL_CACHE
	if (stat->label_cache_size > 0) {
		free(stat->label_cache);
		stat->label_cache = NULL;
		stat->label_cache_size = 0;
	}
#endif
}

void zoo_stat_clear(zoo_stat *stat) {
	memset(stat, 0, sizeof(zoo_stat));
	stat->follower = -1;
	stat->leader = -1;
}

static void zoo_oop_error(zoo_state *state, int16_t stat_id, const char *message) {
	char msg_full[ZOO_LEN_MESSAGE + 1];

	strncpy(msg_full, "ERR: ", sizeof(msg_full) - 1);
	strncat(msg_full, message, sizeof(msg_full) - 1);
	zoo_display_message(state, 200, msg_full);

	zoo_sound_queue_const(&(state->sound), 5, "\x50\x0A");
	state->board.stats[stat_id].data_pos = -1;
}

static ZOO_INLINE void zoo_oop_read_char(zoo_state *state, int16_t stat_id, int16_t *position) {
	zoo_stat *stat = &(state->board.stats[stat_id]);

	if (*position >= 0 && *position < stat->data_len) {
#ifdef ZOO_NO_OBJECT_CODE_WRITES
		// Emulate #ZAP/RESTORE RESTART behaviour (reads).
		if (*position == 1 && stat->label_cache_chr2 != 0)
			state->oop_char = stat->label_cache_chr2;
		else
#endif
		state->oop_char = stat->data[*position];
		*position += 1;
	} else {
		state->oop_char = 0;
	}
}

GBA_FAST_CODE
static void zoo_oop_read_word(zoo_state *state, int16_t stat_id, int16_t *position) {
	int word_pos = 0;

	do {
		zoo_oop_read_char(state, stat_id, position);
	} while (state->oop_char == ' ');

	state->oop_char = zoo_toupper(state->oop_char);
	if (state->oop_char < '0' || state->oop_char > '9') {
		while (
			(state->oop_char >= 'A' && state->oop_char <= 'Z')
			|| (state->oop_char == ':')
			|| (state->oop_char >= '0' && state->oop_char <= '9')
			|| (state->oop_char == '_')
		) {
			// TODO: handle overflow
			state->oop_word[word_pos++] = state->oop_char;

			zoo_oop_read_char(state, stat_id, position);
			state->oop_char = zoo_toupper(state->oop_char);
		}

		if (*position > 0) {
			*position -= 1;
		}
	}
	state->oop_word[word_pos] = 0;
}

static void zoo_oop_read_value(zoo_state *state, int16_t stat_id, int16_t *position) {
	char word[21];
	int word_pos = 0;

	do {
		zoo_oop_read_char(state, stat_id, position);
	} while (state->oop_char == ' ');

	// libzoo fix: toupper is not necessary here
	while (state->oop_char >= '0' && state->oop_char <= '9') {
		word[word_pos++] = state->oop_char;
		zoo_oop_read_char(state, stat_id, position);
	}
	word[word_pos] = 0;

	if (*position > 0) {
		*position -= 1;
	}

	state->oop_value = (word_pos > 0) ? atoi(word) : -1;
}

static void zoo_oop_skip_line(zoo_state *state, int16_t stat_id, int16_t *position) {
	do {
		zoo_oop_read_char(state, stat_id, position);
	} while (state->oop_char != '\0' && state->oop_char != '\r');
}

static bool zoo_oop_parse_direction(zoo_state *state, int16_t stat_id, int16_t *position, int16_t *dx, int16_t *dy) {
	bool result = true;

	switch (zoo_oop_token_search(tok_zoo_oop_token_dir, state->oop_word)) {
	case TOK_DIR_NORTH: {
		*dx = 0;
		*dy = -1;
	} break;
    case TOK_DIR_SOUTH: {
		*dx = 0;
		*dy = 1;
	} break;
    case TOK_DIR_EAST: {
		*dx = 1;
		*dy = 0;
	} break;
    case TOK_DIR_WEST: {
		*dx = -1;
		*dy = 0;
	} break;
    case TOK_DIR_IDLE: {
		*dx = 0;
		*dy = 0;
	} break;
    case TOK_DIR_SEEK: {
		zoo_calc_direction_seek(state, state->board.stats[stat_id].x, state->board.stats[stat_id].y, dx, dy);
	} break;
    case TOK_DIR_FLOW: {
		*dx = state->board.stats[stat_id].step_x;
		*dy = state->board.stats[stat_id].step_y;
	} break;
    case TOK_DIR_RND: {
		zoo_calc_direction_rnd(state, dx, dy);
	} break;
    case TOK_DIR_RNDNS: {
		*dx = 0;
		*dy = state->func_random(state, 2) * 2 - 1;
	} break;
    case TOK_DIR_RNDNE: {
		*dx = state->func_random(state, 2);
		*dy = (*dx == 0) ? -1 : 0;
	} break;
    case TOK_DIR_CW: {
		zoo_oop_read_word(state, stat_id, position);
		result = zoo_oop_parse_direction(state, stat_id, position, dy, dx);
		*dx = -(*dx);
	} break;
    case TOK_DIR_CCW: {
		zoo_oop_read_word(state, stat_id, position);
		result = zoo_oop_parse_direction(state, stat_id, position, dy, dx);
		*dy = -(*dy);
	} break;
    case TOK_DIR_RNDP: {
		zoo_oop_read_word(state, stat_id, position);
		result = zoo_oop_parse_direction(state, stat_id, position, dy, dx);
		if (state->func_random(state, 2) == 0) {
			*dx = -(*dx);
		} else {
			*dy = -(*dy);
		}
	} break;
    case TOK_DIR_OPP: {
		zoo_oop_read_word(state, stat_id, position);
		result = zoo_oop_parse_direction(state, stat_id, position, dx, dy);
		*dx = -(*dx);
		*dy = -(*dy);
	} break;
	default: {
		*dx = 0;
		*dy = 0;
		result = false;
	}
	}

	return result;
}

static void zoo_oop_read_direction(zoo_state *state, int16_t stat_id, int16_t *position, int16_t *dx, int16_t *dy) {
	zoo_oop_read_word(state, stat_id, position);
	if (!zoo_oop_parse_direction(state, stat_id, position, dx, dy)) {
		zoo_oop_error(state, stat_id, "Bad direction");
	}
}

GBA_FAST_CODE
int16_t zoo_oop_find_string_from(zoo_state *state, int16_t stat_id, const char *str, int16_t start_pos, int16_t end_pos) {
	int16_t pos, word_pos, cmp_pos;
	int16_t data_len = end_pos >= 0 ? (end_pos+1) : state->board.stats[stat_id].data_len;
	const char *ptr_str;

	pos = start_pos;

	while (pos < data_len) {
		cmp_pos = pos;
		ptr_str = str;

		while (*ptr_str != '\0') {
			zoo_oop_read_char(state, stat_id, &cmp_pos);
			if (zoo_toupper(*ptr_str) != zoo_toupper(state->oop_char)) {
				goto NoMatch;
			}
			ptr_str++;
		}

		zoo_oop_read_char(state, stat_id, &cmp_pos);
		state->oop_char = zoo_toupper(state->oop_char);
		if ((state->oop_char >= 'A' && state->oop_char <= 'Z') || (state->oop_char == '_')) {
			// match invalid - word incomplete
			goto NoMatch;
		} else {
			// match valid
			return pos;
		}

NoMatch:
		// no match
		pos++;
	}

	return -1;
}

static ZOO_INLINE int16_t zoo_oop_find_string(zoo_state *state, int16_t stat_id, const char *str) {
	return zoo_oop_find_string_from(state, stat_id, str, 0, -1);
}

GBA_FAST_CODE
static bool zoo_oop_iterate_stat(zoo_state *state, int16_t stat_id, int16_t *i_stat, const char *lookup) {
	int16_t pos;
	bool found = false;

	*i_stat += 1;

	if (!strcmp(lookup, "ALL")) {
		found = *i_stat <= state->board.stat_count;
	} else if (!strcmp(lookup, "OTHERS")) {
		if (*i_stat <= state->board.stat_count) {
			if (*i_stat != stat_id) {
				found = true;
			} else {
				*i_stat += 1;
				found = *i_stat <= state->board.stat_count;
			}
		}
	} else if (!strcmp(lookup, "SELF")) {
		if (stat_id > 0 && (*i_stat <= stat_id)) {
			*i_stat = stat_id;
			found = true;
		}
	} else {
		while (*i_stat <= state->board.stat_count) {
			if (state->board.stats[*i_stat].data != NULL) {
				pos = 0;
				zoo_oop_read_char(state, *i_stat, &pos);
				if (state->oop_char == '@') {
					zoo_oop_read_word(state, *i_stat, &pos);
					if (!oop_word_cmp(lookup)) {
						found = true;
					}
				}
			}

			if (found) break;
			*i_stat += 1;
		}
	}

	return found;
}

static bool zoo_oop_find_label(zoo_state *state, int16_t stat_id, const char *send_label, int16_t *i_stat, int16_t *i_data_pos, const char *label_prefix) {
	int i;
	char *target_split_pos;
	// libzoo note: In regular ZZT, this is limited to 20 characters.
	// However, as OOP words are *also* limited to 20 characters,
	// this should hopefully not pose a practical problem.
	const char *object_message;
	char target_lookup[21];
#ifndef ZOO_USE_LABEL_CACHE
	char prefixed_object_message[21+2];
#endif
	bool built_pom = false;
	bool found_stat = false;

	target_split_pos = strchr(send_label, ':');
	if (target_split_pos == NULL) {
		// if there is no target, we only check stat_id
		if (*i_stat < stat_id) {
			object_message = send_label;
			*i_stat = stat_id;
			found_stat = true;
		}
	} else {
		i = target_split_pos - send_label;
		if (i >= sizeof(target_lookup)) i = (sizeof(target_lookup) - 1);
		memcpy(target_lookup, send_label, i);
		target_lookup[i] = 0;

		object_message = target_split_pos + 1;
FindNextStat:
		found_stat = zoo_oop_iterate_stat(state, stat_id, i_stat, target_lookup);
	}

	if (found_stat) {
		if (!strcmp(object_message, "RESTART")) {
			*i_data_pos = 0;
		} else {
#ifdef ZOO_USE_LABEL_CACHE
			*i_data_pos = zoo_oop_label_cache_search(state, *i_stat, object_message, label_prefix[1] == '\'');
#else
			if (!built_pom) {
				strncpy(prefixed_object_message, label_prefix, sizeof(prefixed_object_message) - 1);
				strncat(prefixed_object_message, object_message, sizeof(prefixed_object_message) - 1);
				built_pom = true;
			}

			*i_data_pos = zoo_oop_find_string(state, *i_stat, prefixed_object_message);
#endif

			// if lookup target exists, there may be more stats
			if ((*i_data_pos < 0) && (target_split_pos != NULL)) {
				goto FindNextStat;
			}
		}
		found_stat = *i_data_pos >= 0;
	}

	return found_stat;
}


int16_t zoo_flag_get_id(zoo_state *state, const char *name) {
	int i;

	for (i = 0; i < ZOO_MAX_FLAG; i++) {
		if (!strncmp(name, state->world.info.flags[i], ZOO_LEN_FLAG)) {
			return i;
		}
	}

	return -1;
}

void zoo_flag_set(zoo_state *state, const char *name) {
	int i;

	if (zoo_flag_get_id(state, name) < 0) {
		i = 0;
		while ((i < (ZOO_MAX_FLAG-1)) && state->world.info.flags[i][0] != '\0') {
			i++;
		}
		// free or last flag
		strncpy(state->world.info.flags[i], name, ZOO_LEN_FLAG);
	}
}

void zoo_flag_clear(zoo_state *state, const char *name) {
	int i = zoo_flag_get_id(state, name);
	if (i >= 0) {
		state->world.info.flags[i][0] = 0;
	}
}

static void zoo_oop_strip_chars(char *output, const char *input, int outlen) {
	int ii, io = 0;

	for (ii = 0; ii < strlen(input) && io < outlen; ii++) {
		if (((input[ii] >= 'A') && (input[ii] <= 'Z')) || ((input[ii] >= '0') && (input[ii] <= '9'))) {
			output[io++] = input[ii];
		} else if ((input[ii] >= 'a') && (input[ii] <= 'z')) {
			output[io++] = input[ii] - 0x20;
		}
	}
	output[io] = 0;
}

static bool zoo_oop_parse_tile(zoo_state *state, int16_t stat_id, int16_t *position, zoo_tile *tile) {
	// char name[21];
	int i;

	tile->color = 0x00;

	zoo_oop_read_word(state, stat_id, position);
	i = zoo_oop_token_search(tok_zoo_oop_token_color, state->oop_word);
	if (i != TOK_COLOR_INVALID) {
		tile->color = i + 9;
		zoo_oop_read_word(state, stat_id, position);			
	}
	/*
	for (i = 1; i <= 7; i++) {
		zoo_oop_strip_chars(name, zoo_color_names[i], sizeof(name) - 1);
		if (!strncmp(name, state->oop_word, 20)) {
		if (!strncmp(zoo_oop_color_names[i], state->oop_word, 20)) {
			tile->color = i + 8;
			zoo_oop_read_word(state, stat_id, position);
			break;
		}
	}
	*/

	for (i = 0; i <= ZOO_MAX_ELEMENT; i++) {
		// zoo_oop_strip_chars(name, zoo_element_defs[i].name, sizeof(name) - 1);
		// if (!strncmp(name, state->oop_word, 20)) {
		if (!strncmp(zoo_element_defs[i].oop_strip_name, state->oop_word, 20)) {
			tile->element = i;
			return true;
		}
	}

	return false;
}

static ZOO_INLINE uint8_t zoo_get_color_for_tile_match(zoo_state *state, zoo_tile tile) {
	uint8_t c = zoo_element_defs[tile.element].color;
	if (c < ZOO_COLOR_SPECIAL_MIN) {
		return c & 0x07;
	} else if (c == ZOO_COLOR_WHITE_ON_CHOICE) {
		return ((tile.color >> 4) & 0x0F) + 8;
	} else {
		return tile.color & 0x0F;
	}
}

GBA_FAST_CODE
static bool zoo_find_tile_on_board(zoo_state *state, int16_t *x, int16_t *y, zoo_tile tile) {
	int16_t lx = *x;
	int16_t ly = *y;

	while (true) {
		lx += 1;
		if (lx > ZOO_BOARD_WIDTH) {
			lx = 1;
			ly += 1;
			if (ly > ZOO_BOARD_HEIGHT) {
				*x = lx;
				*y = ly;
				return false;
			}
		}

		if (state->board.tiles[lx][ly].element == tile.element) {
			if ((tile.color == 0) || (zoo_get_color_for_tile_match(state, state->board.tiles[lx][ly]) == tile.color)) {
				*x = lx;
				*y = ly;
				return true;
			}
		}
	}
}

static void zoo_oop_place_tile(zoo_state *state, int16_t x, int16_t y, zoo_tile tile) {
	uint8_t color;

	if (state->board.tiles[x][y].element == ZOO_E_PLAYER) {
		return;
	}

	color = tile.color;
	if (zoo_element_defs[tile.element].color < ZOO_COLOR_SPECIAL_MIN) {
		color = zoo_element_defs[tile.element].color;
	} else {
		if (color == 0) color = state->board.tiles[x][y].color;
		if (color == 0) color = 0x0F;
		if (zoo_element_defs[tile.element].color == ZOO_COLOR_WHITE_ON_CHOICE) {
			color = ((color - 8) << 4) | 0x0F;
		}
	}

	if (state->board.tiles[x][y].element == tile.element) {
		state->board.tiles[x][y].color = color;
	} else {
		zoo_board_damage_tile(state, x, y);
		if (zoo_element_defs[tile.element].cycle >= 0) {
			zoo_stat_add(state, x, y, tile.element, color, zoo_element_defs[tile.element].cycle, &zoo_stat_template_default);
		} else {
			state->board.tiles[x][y].element = tile.element;
			state->board.tiles[x][y].color = color;
		}
	}

	zoo_board_draw_tile(state, x, y);
}

static bool zoo_oop_check_condition(zoo_state *state, int16_t stat_id, int16_t *position) {
	int16_t ix, iy;
	// libzoo fix: consistent initalization
	// otherwise a zoo_oop_error in "ANY" could lead to invalid data
	zoo_tile tile = {0, 0};

	switch (zoo_oop_token_search(tok_zoo_oop_token_cond, state->oop_word)) {
	case TOK_COND_NOT:
		zoo_oop_read_word(state, stat_id, position);
		return !zoo_oop_check_condition(state, stat_id, position);
	case TOK_COND_ALLIGNED:
		return (state->board.stats[stat_id].x == state->board.stats[0].x)
		|| (state->board.stats[stat_id].y == state->board.stats[0].y);
	case TOK_COND_CONTACT:
		return zoo_dist_sq(state->board.stats[stat_id].x - state->board.stats[0].x,
			state->board.stats[stat_id].y - state->board.stats[0].y) == 1;
	case TOK_COND_BLOCKED:
		zoo_oop_read_direction(state, stat_id, position, &ix, &iy);
		return !zoo_element_defs[state->board.tiles
			[state->board.stats[stat_id].x + ix]
			[state->board.stats[stat_id].y + iy]
		.element].walkable;
	case TOK_COND_ENERGIZED:
		return state->world.info.energizer_ticks > 0;
	case TOK_COND_ANY:
		if (!zoo_oop_parse_tile(state, stat_id, position, &tile)) {
			zoo_oop_error(state, stat_id, "Bad object kind");
			return false;
		}

		ix = 0;
		iy = 1;
		return zoo_find_tile_on_board(state, &ix, &iy, tile);
	default:
		return zoo_flag_get_id(state, state->oop_word) >= 0;
	}
}

static void zoo_oop_read_line_to_end(zoo_state *state, int16_t stat_id, int16_t *position, char *buf, int buflen) {
	int16_t bufpos = 0;

	zoo_oop_read_char(state, stat_id, position);
	while ((state->oop_char != '\0') && (state->oop_char != '\r')) {
		if (bufpos < buflen) {
			buf[bufpos++] = state->oop_char;
		}
		zoo_oop_read_char(state, stat_id, position);
	}
	buf[bufpos] = 0;
}

bool zoo_oop_send(zoo_state *state, int16_t stat_id, const char *send_label, bool ignore_lock) {
	int16_t i_data_pos, i_stat;
	bool ignore_self_lock, result;

	if (stat_id < 0) {
		// if statId is negative, label send will always succeed on self
		// this is used for in-game events (f.e. TOUCH, SHOT)
		stat_id = -stat_id;
		ignore_self_lock = true;
	} else {
		ignore_self_lock = false;
	}

	result = false;
	i_stat = 0;

	while (zoo_oop_find_label(state, stat_id, send_label, &i_stat, &i_data_pos, "\r:")) {
		if (
			((state->board.stats[i_stat].p2 == 0) || (ignore_lock))
			|| ((stat_id == i_stat) && !ignore_self_lock)
		) {
			if (i_stat == stat_id) {
				result = true;
			}

			state->board.stats[i_stat].data_pos = i_data_pos;
		}
	}

	return result;
}

#define INIT_TEXT_WINDOW() \
	if (text_window.line_count < 0) { \
		memset(&text_window, 0, sizeof(text_window)); \
	}

void zoo_oop_execute(zoo_state *state, int16_t stat_id, int16_t *position, const char *default_name) {
	char buf[256];
	char buf2[256];
	int16_t buf2_len;
	char name[51];
	zoo_stat *stat;
	int16_t dx, dy;
	int16_t ix, iy;
	bool stop_running;
	bool replace_stat;
	bool end_of_program;
	zoo_tile replace_tile;
	int16_t name_position;
	int16_t last_position;
	bool repeat_ins_next_tick;
	bool line_finished;
	int16_t label_data_pos;
	int16_t label_stat_id;
	int16_t *counter_ptr;
	bool counter_subtract;
	int16_t bind_stat_id;
	int16_t ins_count;
	zoo_tile arg_tile, arg_tile2;
	//
	zoo_text_window text_window;

	// libzoo fix: done on demand
	// strncpy(name, default_name, sizeof(name) - 1);
	
	stat = &state->board.stats[stat_id];

	if (state->call_stack.state == 1) {
		// returning from object window
		if (state->object_window.hyperlink[0] != '\0') {
			if (zoo_oop_send(state, stat_id, state->object_window.hyperlink, false)) {
				// continue
				goto StartParsing;
			}
		}
		
		// go back to where we left off
		replace_stat = state->object_replace_stat;
		replace_tile = state->object_replace_tile;
		goto AfterTextWindow;
	}

	if (*position < 0) {
		return;
	}

StartParsing:
	text_window.line_count = -1;
	stop_running = false;
	repeat_ins_next_tick = false;
	replace_stat = false;
	end_of_program = false;
	ins_count = 0;
	do {
ReadInstruction:
		line_finished = true;
		last_position = *position;
		zoo_oop_read_char(state, stat_id, position);

		// skip labels
		while (state->oop_char == ':') {
			do {
				zoo_oop_read_char(state, stat_id, position);
			} while (state->oop_char != '\0' && state->oop_char != '\r');
			zoo_oop_read_char(state, stat_id, position);
		}

		switch (state->oop_char) {
		case '\'':	// zapped label
		case '@': {	// name
			zoo_oop_skip_line(state, stat_id, position);
		} break;
		case '/':	// direction
			repeat_ins_next_tick = true;
		case '?': {	// try direction
			zoo_oop_read_word(state, stat_id, position);
			if (zoo_oop_parse_direction(state, stat_id, position, &dx, &dy)) {
				if (dx != 0 || dy != 0) {
					if (!zoo_element_defs[state->board.tiles[stat->x + dx][stat->y + dy].element].walkable) {
						zoo_element_push(state, stat->x + dx, stat->y + dy, dx, dy);
					}

					if (zoo_element_defs[state->board.tiles[stat->x + dx][stat->y + dy].element].walkable) {
						zoo_stat_move(state, stat_id, stat->x + dx, stat->y + dy);
						repeat_ins_next_tick = false;
					}
				} else { // idle always succeeds
					repeat_ins_next_tick = false;
				}

				// check for newline
				zoo_oop_read_char(state, stat_id, position);
				if (state->oop_char != '\r') *position -= 1;

				stop_running = true;
			} else {
				zoo_oop_error(state, stat_id, "Bad direction");
			}
		} break;
		case '#': {	// instruction
ReadCommand:
			zoo_oop_read_word(state, stat_id, position);
			if (!oop_word_cmp("THEN")) {
				zoo_oop_read_word(state, stat_id, position);
			}
			if (state->oop_word[0] == '\0') {
				goto ReadInstruction;
			} else {
				ins_count++;
				switch (zoo_oop_token_search(tok_zoo_oop_token_ins, state->oop_word)) {
				case TOK_INS_GO: {
					zoo_oop_read_direction(state, stat_id, position, &dx, &dy);

					if (!zoo_element_defs[state->board.tiles[stat->x + dx][stat->y + dy].element].walkable) {
						zoo_element_push(state, stat->x + dx, stat->y + dy, dx, dy);
					}

					if (zoo_element_defs[state->board.tiles[stat->x + dx][stat->y + dy].element].walkable) {
						zoo_stat_move(state, stat_id, stat->x + dx, stat->y + dy);
					} else {
						repeat_ins_next_tick = true;
					}

					stop_running = true;
				} break;
                case TOK_INS_TRY: {
					zoo_oop_read_direction(state, stat_id, position, &dx, &dy);

					if (!zoo_element_defs[state->board.tiles[stat->x + dx][stat->y + dy].element].walkable) {
						zoo_element_push(state, stat->x + dx, stat->y + dy, dx, dy);
					}

					if (zoo_element_defs[state->board.tiles[stat->x + dx][stat->y + dy].element].walkable) {
						zoo_stat_move(state, stat_id, stat->x + dx, stat->y + dy);
						stop_running = true;
					} else {
						goto ReadCommand;
					}

					stop_running = true;
				} break;
                case TOK_INS_WALK: {
					zoo_oop_read_direction(state, stat_id, position, &dx, &dy);
					stat->step_x = dx;
					stat->step_y = dy;
				} break;
                case TOK_INS_SET: {
					zoo_oop_read_word(state, stat_id, position);
					zoo_flag_set(state, state->oop_word);
				} break;
                case TOK_INS_CLEAR: {
					zoo_oop_read_word(state, stat_id, position);
					zoo_flag_clear(state, state->oop_word);
				} break;
                case TOK_INS_IF: {
					zoo_oop_read_word(state, stat_id, position);
					if (zoo_oop_check_condition(state, stat_id, position)) {
						goto ReadCommand;
					}
				} break;
                case TOK_INS_SHOOT: {
					zoo_oop_read_direction(state, stat_id, position, &dx, &dy);
					if (zoo_board_shoot(state, ZOO_E_BULLET, stat->x, stat->y, dx, dy, 1)) {
						zoo_sound_queue_const(&(state->sound), 2, "\x30\x01\x26\x01");
					}
					stop_running = true;
				} break;
                case TOK_INS_THROWSTAR: {
					zoo_oop_read_direction(state, stat_id, position, &dx, &dy);
					if (zoo_board_shoot(state, ZOO_E_STAR, stat->x, stat->y, dx, dy, 1)) {
					}
					stop_running = true;
				} break;
				case TOK_INS_GIVE:
                case TOK_INS_TAKE: {
					counter_subtract = !oop_word_cmp("TAKE");

					zoo_oop_read_word(state, stat_id, position);
		
					switch (zoo_oop_token_search(tok_zoo_oop_token_give, state->oop_word)) {
					case TOK_GIVE_HEALTH: {
						counter_ptr = &(state->world.info.health);
					} break;
               		case TOK_GIVE_AMMO: {
						counter_ptr = &(state->world.info.ammo);
					} break;
                	case TOK_GIVE_GEMS: {
						counter_ptr = &(state->world.info.gems);
					} break;
               		case TOK_GIVE_TORCHES: {
						counter_ptr = &(state->world.info.torches);
					} break;
                	case TOK_GIVE_SCORE: {
						counter_ptr = &(state->world.info.score);
					} break;
                	case TOK_GIVE_TIME: {
						counter_ptr = &(state->world.info.board_time_sec);
					} break;
					default: {
						counter_ptr = NULL;
					} break;
					}

					if (counter_ptr != NULL) {
						zoo_oop_read_value(state, stat_id, position);
						if (state->oop_value > 0) {
							if (counter_subtract) {
								state->oop_value = -state->oop_value;
							}
							if ((*counter_ptr + state->oop_value) >= 0) {
								*counter_ptr += state->oop_value;
							} else {
								goto ReadCommand;
							}
						}
					}

					state->func_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_ALL);
				} break;
                case TOK_INS_END: {
					*position = -1;
					state->oop_char = '\0';
				} break;
                case TOK_INS_ENDGAME: {
					state->world.info.health = 0;
				} break;
                case TOK_INS_IDLE: {
					stop_running = true;
				} break;
                case TOK_INS_RESTART: {
					*position = 0;
					line_finished = false;
				} break;
                case TOK_INS_ZAP: {
					zoo_oop_read_word(state, stat_id, position);
					// state->oop_word is used by zoo_oop_iterate_stat
					strncpy(buf2, state->oop_word, sizeof(buf2));
					label_stat_id = 0;
					while (
						zoo_oop_find_label(state, stat_id, buf2,
						&label_stat_id, &label_data_pos, "\r:")
					) {
#ifdef ZOO_USE_LABEL_CACHE
						zoo_oop_label_cache_zap(state, label_stat_id, label_data_pos, true, false, buf2);
#else
						state->board.stats[label_stat_id].data[label_data_pos + 1] = '\'';
#endif
					}
				} break;
                case TOK_INS_RESTORE: {
					zoo_oop_read_word(state, stat_id, position);
					strncpy(buf, "\r'", sizeof(buf) - 1);
					strncat(buf, state->oop_word, sizeof(buf) - 1);
					strncat(buf, "\r", sizeof(buf) - 1);
					// state->oop_word is used by zoo_oop_iterate_stat
					strncpy(buf2, state->oop_word, sizeof(buf2));

					label_stat_id = 0;
					while (
						zoo_oop_find_label(state, stat_id, buf2,
						&label_stat_id, &label_data_pos, "\r'")
					) {
#ifdef ZOO_USE_LABEL_CACHE
						zoo_oop_label_cache_zap(state, label_stat_id, label_data_pos, false, true, buf + 2);
#else
						do {
							state->board.stats[label_stat_id].data[label_data_pos + 1] = ':';
							// libzoo fix: optimization - no need to check already checked parts of the code
							// label_data_pos = zoo_oop_find_string(state, label_stat_id, buf);
							label_data_pos = zoo_oop_find_string_from(state, label_stat_id, buf, label_data_pos + 2, -1);
						} while (label_data_pos > 0);
#endif
					}
				} break;
                case TOK_INS_LOCK: {
					stat->p2 = 1;
				} break;
                case TOK_INS_UNLOCK: {
					stat->p2 = 0;
				} break;
                case TOK_INS_SEND: {
					zoo_oop_read_word(state, stat_id, position);
					// state->oop_word is used by zoo_oop_iterate_stat
					strncpy(buf2, state->oop_word, sizeof(buf2));
					if (zoo_oop_send(state, stat_id, buf2, false)) {
						line_finished = false;
					}
				} break;
                case TOK_INS_BECOME: {
					if (zoo_oop_parse_tile(state, stat_id, position, &arg_tile)) {
						replace_stat = true;
						replace_tile.element = arg_tile.element;
						replace_tile.color = arg_tile.color;
					} else {
						zoo_oop_error(state, stat_id, "Bad #BECOME");
					}
				} break;
                case TOK_INS_PUT: {
					zoo_oop_read_direction(state, stat_id, position, &dx, &dy);
					if (dx == 0 && dy == 0) {
						zoo_oop_error(state, stat_id, "Bad #PUT");
					} else if (!zoo_oop_parse_tile(state, stat_id, position, &arg_tile)) {
						zoo_oop_error(state, stat_id, "Bad #PUT");
					} else if (
						((stat->x + dx) > 0)
						&& ((stat->x + dx) <= ZOO_BOARD_WIDTH)
						&& ((stat->y + dy) > 0)
						&& ((stat->y + dy) < ZOO_BOARD_HEIGHT)
					) {
						if (!zoo_element_defs[state->board.tiles[stat->x + dx][stat->y + dy]
							.element].walkable
						) {
							zoo_element_push(state, stat->x + dx, stat->y + dy, dx, dy);
						}

						zoo_oop_place_tile(state, stat->x + dx, stat->y + dy, arg_tile);
					}
				} break;
                case TOK_INS_CHANGE: {
					if (!zoo_oop_parse_tile(state, stat_id, position, &arg_tile)) {
						zoo_oop_error(state, stat_id, "Bad #CHANGE");
					}
					if (!zoo_oop_parse_tile(state, stat_id, position, &arg_tile2)) {
						zoo_oop_error(state, stat_id, "Bad #CHANGE");
					}

					ix = 0;
					iy = 1;
					if (arg_tile2.color == 0 && zoo_element_defs[arg_tile2.element].color < ZOO_COLOR_SPECIAL_MIN) {
						arg_tile2.color = zoo_element_defs[arg_tile2.element].color;
					}

					while (zoo_find_tile_on_board(state, &ix, &iy, arg_tile)) {
						zoo_oop_place_tile(state, ix, iy, arg_tile2);
					}
				} break;
                case TOK_INS_PLAY: {
					zoo_oop_read_line_to_end(state, stat_id, position, buf, sizeof(buf) - 1);
					buf2_len = zoo_sound_parse(buf, (uint8_t*) buf2, sizeof(buf2));
					if (buf2_len > 0) {
						zoo_sound_queue(&(state->sound), -1, (uint8_t*) buf2, buf2_len);
					}
					line_finished = false;
				} break;
                case TOK_INS_CYCLE: {
					zoo_oop_read_value(state, stat_id, position);
					if (state->oop_value > 0) {
						stat->cycle = state->oop_value;
					}
				} break;
                case TOK_INS_CHAR: {
					zoo_oop_read_value(state, stat_id, position);
					if (state->oop_value > 0 && state->oop_value <= 255) {
						stat->p1 = state->oop_value;
						zoo_board_draw_tile(state, stat->x, stat->y);
					}
				} break;
                case TOK_INS_DIE: {
					replace_stat = true;
					replace_tile.element = ZOO_E_EMPTY;
					replace_tile.color = 0x0F;
				} break;
                case TOK_INS_BIND: {
					zoo_oop_read_word(state, stat_id, position);
					bind_stat_id = 0;
					// state->oop_word is used by zoo_oop_iterate_stat
					strncpy(buf, state->oop_word, sizeof(buf));
					if (zoo_oop_iterate_stat(state, stat_id, &bind_stat_id, buf)) {
						// libzoo fix: add Bind_StatDataInUse check
						for (ix = 1; ix <= state->board.stat_count; ix++) {
							if (ix == stat_id) continue;
							if (state->board.stats[ix].data == stat->data) goto Bind_StatDataInUse;
						}
						zoo_stat_free(stat);
					Bind_StatDataInUse:
						stat->data = state->board.stats[bind_stat_id].data;
						stat->data_len = state->board.stats[bind_stat_id].data_len;
#ifdef ZOO_USE_LABEL_CACHE
						stat->label_cache = state->board.stats[bind_stat_id].label_cache;
						stat->label_cache_size = state->board.stats[bind_stat_id].label_cache_size;
#endif
						*position = 0;
					}
				} break;
				default: {
					strncpy(buf, state->oop_word, sizeof(buf) - 1);
					if (zoo_oop_send(state, stat_id, buf, false)) {
						line_finished = false;
					} else if (strchr(buf, ':') == NULL) {
						strncpy(buf2, "Bad command ", sizeof(buf2) - 1);
						strncat(buf2, buf, sizeof(buf2) - 1);
						zoo_oop_error(state, stat_id, buf2);
					}
				} break;
				}
			}

			if (line_finished) {
				zoo_oop_skip_line(state, stat_id, position);
			}
		} break;
		case '\r': {	// newline
			if (text_window.line_count > 0) {
				zoo_window_append(&text_window, "");
			}
		} break;
		case '\0': {	// end
			end_of_program = true;
		} break;
		default: {	// text
			buf[0] = state->oop_char;
			zoo_oop_read_line_to_end(state, stat_id, position, buf + 1, sizeof(buf) - 2);
			INIT_TEXT_WINDOW();
			zoo_window_append(&text_window, buf);
		} break;
		}
	} while (!end_of_program && !stop_running && !repeat_ins_next_tick && !replace_stat && ins_count <= 32);

	if (repeat_ins_next_tick) {
		*position = last_position;
	}

	if (state->oop_char == 0) {
		*position = -1;
	}

	if (text_window.line_count > 1) {
		name_position = 0;
		zoo_oop_read_char(state, stat_id, &name_position);
		if (state->oop_char == '@') {
			zoo_oop_read_line_to_end(state, stat_id, &name_position, name, sizeof(name) - 1);
		}
		if (name[0] == '\0') {
			strncpy(name, default_name != NULL ? default_name : "Interaction", sizeof(name) - 1);
		}

		strncpy(text_window.title, name, sizeof(text_window.title) - 1);
		text_window.hyperlink_as_select = true;
		text_window.viewing_file = false;

		state->object_replace_stat = replace_stat;
		state->object_replace_tile = replace_tile;
		memcpy(&(state->object_window), &text_window, sizeof(zoo_text_window));
		state->object_window_request = true;
		return;
	} else if (text_window.line_count == 1) {
		zoo_display_message(state, 200, text_window.lines[0]);
		zoo_window_close(&text_window);
	}

	AfterTextWindow:
	if (replace_stat) {
		ix = stat->x;
		iy = stat->y;
		zoo_board_damage_stat(state, stat_id);
		zoo_oop_place_tile(state, ix, iy, replace_tile);
	}
}
