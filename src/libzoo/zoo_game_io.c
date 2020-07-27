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
#include "zoo.h"

#define zoo_io_read_byte(h) (h)->func_getc((h))

static int16_t zoo_io_read_short(zoo_io_handle *h) {
	uint8_t v = h->func_getc(h);
	return v | ((uint16_t) h->func_getc(h) << 8);
}

static zoo_tile zoo_io_read_tile(zoo_io_handle *h) {
	zoo_tile tile;
	tile.element = h->func_getc(h);
	tile.color = h->func_getc(h);
	return tile;
}

static void zoo_io_read_pstring(zoo_io_handle *h, int p_len, char *str, int str_len) {
	int len = h->func_getc(h);
	if (len > p_len) len = p_len;
	if (len > str_len) len = str_len;
	int slen = h->func_read(h, (uint8_t*) str, len);
	h->func_skip(h, p_len - len);
	str[slen] = 0;
}

#define zoo_io_write_byte(h, v) (h)->func_putc((h), (v))

static void zoo_io_write_short(zoo_io_handle *h, int16_t v) {
	h->func_putc(h, (v & 0xFF));
	h->func_putc(h, (v >> 8));
}

static void zoo_io_write_tile(zoo_io_handle *h, zoo_tile tile) {
	h->func_putc(h, tile.element);
	h->func_putc(h, tile.color);
}

static void zoo_io_write_pstring(zoo_io_handle *h, int p_len, const char *str, int str_len) {
	str_len = strnlen(str, str_len);
	if (str_len > p_len) str_len = p_len;

	h->func_putc(h, str_len);
	h->func_write(h, (uint8_t *) str, str_len);
	h->func_skip(h, p_len - str_len);
}

static void zoo_stat_io_read(zoo_io_handle *h, zoo_stat *stat) {
	stat->x = zoo_io_read_byte(h);
	stat->y = zoo_io_read_byte(h);
	stat->step_x = zoo_io_read_short(h);
	stat->step_y = zoo_io_read_short(h);
	stat->cycle = zoo_io_read_short(h);
	stat->p1 = zoo_io_read_byte(h);
	stat->p2 = zoo_io_read_byte(h);
	stat->p3 = zoo_io_read_byte(h);
	stat->follower = zoo_io_read_short(h);
	stat->leader = zoo_io_read_short(h);
	stat->under = zoo_io_read_tile(h);
#ifdef ZOO_USE_ROM_POINTERS
	h->func_read(h, &stat->data, 4);
#else
	stat->data = NULL; h->func_skip(h, 4);
#endif
	stat->data_pos = zoo_io_read_short(h);
	stat->data_len = zoo_io_read_short(h);
	h->func_skip(h, 8);
#ifdef ZOO_USE_LABEL_CACHE
	stat->label_cache = NULL;
	stat->label_cache_size = 0;
#endif
}

static void zoo_stat_io_write(zoo_io_handle *h, zoo_stat *stat) {
	zoo_io_write_byte(h, stat->x);
	zoo_io_write_byte(h, stat->y);
	zoo_io_write_short(h, stat->step_x);
	zoo_io_write_short(h, stat->step_y);
	zoo_io_write_short(h, stat->cycle);
	zoo_io_write_byte(h, stat->p1);
	zoo_io_write_byte(h, stat->p2);
	zoo_io_write_byte(h, stat->p3);
	zoo_io_write_short(h, stat->follower);
	zoo_io_write_short(h, stat->leader);
	zoo_io_write_tile(h, stat->under);
#ifdef ZOO_USE_ROM_POINTERS
	h->func_write(h, &stat->data, 4);
#else
	h->func_skip(h, 4); // stat->data
#endif
	zoo_io_write_short(h, stat->data_pos);
	zoo_io_write_short(h, stat->data_len);
	h->func_skip(h, 8);
}

size_t zoo_io_board_max_length(zoo_board *board) {
	int ix;
	size_t size = 0;

	// board name
	size += 51;
	// max buffer size (TODO: RLE pass)
	size += ZOO_BOARD_WIDTH * ZOO_BOARD_HEIGHT * 3;
	// board info
	size += 86;
	// stat count
	size += 2;
	// stats
	for (ix = 0; ix <= board->stat_count; ix++) {
		size += 33;
#ifndef ZOO_USE_ROM_POINTERS
		if (board->stats[ix].data_len > 0)
			size += board->stats[ix].data_len;
#endif
	}

	return size;
}

int zoo_io_board_write(zoo_io_handle *h, zoo_board *board) {
	int ix, iy;
	zoo_rle_tile rle;
	zoo_stat *stat;

	zoo_io_write_pstring(h, 50, board->name, sizeof(board->name) - 1);

	ix = 1;
	iy = 1;
	rle.count = 1;
	rle.tile = board->tiles[ix][iy];
	do {
		ix++;
		if (ix > ZOO_BOARD_WIDTH) {
			ix = 1;
			iy++;
		}

		if (
			(board->tiles[ix][iy].color == rle.tile.color)
			&& (board->tiles[ix][iy].element == rle.tile.element)
			&& (rle.count < 255)
			&& (iy <= ZOO_BOARD_HEIGHT)
		) {
			rle.count++;
		} else {
			zoo_io_write_byte(h, rle.count);
			zoo_io_write_tile(h, rle.tile);
			rle.tile = board->tiles[ix][iy];
			rle.count = 1;
		}
	} while (iy <= ZOO_BOARD_HEIGHT);

	zoo_io_write_byte(h, board->info.max_shots);
	zoo_io_write_byte(h, board->info.is_dark);
	for (ix = 0; ix < 4; ix++)
		zoo_io_write_byte(h, board->info.neighbor_boards[ix]);
	zoo_io_write_byte(h, board->info.reenter_when_zapped);
	zoo_io_write_pstring(h, 58, board->info.message, ZOO_LEN_MESSAGE);
	zoo_io_write_byte(h, board->info.start_player_x);
	zoo_io_write_byte(h, board->info.start_player_y);
	zoo_io_write_short(h, board->info.time_limit_sec);
	h->func_skip(h, 16);

	zoo_io_write_short(h, board->stat_count);
	stat = &board->stats[0];
	for (ix = 0; ix <= board->stat_count; ix++, stat++) {
		if (stat->data_len > 0) {
			for (iy = 1; iy < ix; iy++) {
				if (board->stats[iy].data == stat->data) {
					stat->data_len = -iy;
				}
			}
		}
		zoo_stat_io_write(h, stat);
		if (stat->data_len > 0) {
#ifndef ZOO_USE_ROM_POINTERS
			h->func_write(h, (uint8_t *) stat->data, stat->data_len);
#endif
			zoo_stat_free(stat);
		}
	}

	return 0;
}

int zoo_io_board_read(zoo_io_handle *h, zoo_board *board) {
	int ix, iy;
	zoo_rle_tile rle;
	zoo_stat *stat;
#ifdef ZOO_USE_ROM_POINTERS
	bool is_rom = h->func_getptr != NULL && platform_is_rom_ptr(h->func_getptr(h));
#endif

	zoo_io_read_pstring(h, 50, board->name, sizeof(board->name) - 1);

	ix = 1;
	iy = 1;
	rle.count = 0;
	do {
		if (rle.count <= 0) {
			rle.count = zoo_io_read_byte(h);
			rle.tile = zoo_io_read_tile(h);
		}
		board->tiles[ix][iy] = rle.tile;
		ix++;
		if (ix > ZOO_BOARD_WIDTH) {
			ix = 1;
			iy++;
		}
		rle.count--;
	} while (iy <= ZOO_BOARD_HEIGHT);

	board->info.max_shots = zoo_io_read_byte(h);
	board->info.is_dark = zoo_io_read_byte(h);
	for (ix = 0; ix < 4; ix++)
		board->info.neighbor_boards[ix] = zoo_io_read_byte(h);
	board->info.reenter_when_zapped = zoo_io_read_byte(h);
	zoo_io_read_pstring(h, 58, board->info.message, ZOO_LEN_MESSAGE);
	board->info.start_player_x = zoo_io_read_byte(h);
	board->info.start_player_y = zoo_io_read_byte(h);
	board->info.time_limit_sec = zoo_io_read_short(h);
	h->func_skip(h, 16);

	board->stat_count = zoo_io_read_short(h);
	stat = &board->stats[0];

	if (board->stat_count > (ZOO_MAX_STAT + 1)) {
		// more stats than we can load
		return ZOO_ERROR_INVAL;
	}

	for (ix = 0; ix <= board->stat_count; ix++, stat++) {
		zoo_stat_io_read(h, stat);
		if (stat->data_len > 0) {
#ifdef ZOO_USE_ROM_POINTERS
			if (is_rom) {
				stat->data = (char*) h->func_getptr(h);
				h->func_skip(h, stat->data_len);
			}
			// If not from ROM, stat->data should be correct,
			// and there should be no text following.
#else
			stat->data = malloc(stat->data_len);
			if (stat->data == NULL)
				return ZOO_ERROR_NOMEM;
			h->func_read(h, (uint8_t *) stat->data, stat->data_len);
#endif
		} else if (stat->data_len < 0) {
			// TODO: bounds check
			stat->data = board->stats[-stat->data_len].data;
			stat->data_len = board->stats[-stat->data_len].data_len;
		}
	}

	return 0;
}

int zoo_board_close(zoo_state *state) {
	zoo_io_handle handle;
	int16_t board_id;
	size_t buf_len;
	int ret;
	uint8_t *new_ptr;

	board_id = state->world.info.current_board;
	if (state->world.board_data[board_id] != NULL) {
		if (!platform_is_rom_ptr(state->world.board_data[board_id]))
			free(state->world.board_data[board_id]);
		state->world.board_data[board_id] = NULL;
	}

	buf_len = zoo_io_board_max_length(&state->board);
	state->world.board_data[board_id] = malloc(buf_len);
	if (state->world.board_data[board_id] == NULL)
		return ZOO_ERROR_NOMEM;

	handle = zoo_io_open_file_mem(state->world.board_data[board_id], buf_len, true);

	ret = zoo_io_board_write(&handle, &state->board);
	if (ret) return ret;

	if (handle.func_tell(&handle) != buf_len) {
		state->world.board_len[board_id] = handle.func_tell(&handle);
		new_ptr = realloc(state->world.board_data[board_id], state->world.board_len[board_id]);
		if (new_ptr != NULL)
			state->world.board_data[board_id] = new_ptr;
	}

	return 0;
}

int zoo_board_open(zoo_state *state, int16_t board_id) {
	zoo_io_handle handle;
	int ret;

	if (board_id > state->world.board_count) {
		board_id = state->world.info.current_board;
	}

	handle = zoo_io_open_file_mem(
		state->world.board_data[board_id],
		state->world.board_len[board_id],
		false
	);

	ret = zoo_io_board_read(&handle, &state->board);
	if (ret) return ret;

	state->world.info.current_board = board_id;
	return 0;
}

int zoo_world_close(zoo_state *state) {
	int i;

	i = zoo_board_close(state);
	if (i) return i;

	for (i = 0; i <= state->world.board_count; i++) {
		if (!platform_is_rom_ptr(state->world.board_data[i]))
			free(state->world.board_data[i]);
		state->world.board_data[i] = NULL;
	}

	return 0;
}

int zoo_io_world_read(zoo_io_handle *h, zoo_world *world, bool title_only) {
	int i;
#ifdef ZOO_USE_ROM_POINTERS
	bool is_rom = h->func_getptr != NULL && platform_is_rom_ptr(h->func_getptr(h));
#endif

	world->board_count = zoo_io_read_short(h);
	if (world->board_count < 0) {
		if (world->board_count != -1) {
			return ZOO_ERROR_WRONGVER;
		} else {
			world->board_count = zoo_io_read_short(h);
			if (world->board_count > ZOO_MAX_BOARD)
				return ZOO_ERROR_INVAL;
		}
	}

	world->info.ammo = zoo_io_read_short(h);
	world->info.gems = zoo_io_read_short(h);
	for (i = 0; i < 7; i++)
		world->info.keys[i] = zoo_io_read_byte(h);
	world->info.health = zoo_io_read_short(h);
	world->info.current_board = zoo_io_read_short(h);
	world->info.torches = zoo_io_read_short(h);
	world->info.torch_ticks = zoo_io_read_short(h);
	world->info.energizer_ticks = zoo_io_read_short(h);
	h->func_skip(h, 2);
	world->info.score = zoo_io_read_short(h);
	zoo_io_read_pstring(h, 20, world->info.name, sizeof(world->info.name) - 1);
	for (i = 0; i < 10; i++)
		zoo_io_read_pstring(h, 20, world->info.flags[i], sizeof(world->info.flags[i]) - 1);
	world->info.board_time_sec = zoo_io_read_short(h);
	world->info.board_time_hsec = zoo_io_read_short(h);
	world->info.is_save = zoo_io_read_byte(h);
	h->func_skip(h, 247);

	if (title_only) {
		world->board_count = 0;
		world->info.current_board = 0;
		world->info.is_save = true;
	}

	for (i = 0; i <= world->board_count; i++) {
		world->board_len[i] = zoo_io_read_short(h);
#ifdef ZOO_USE_ROM_POINTERS
		if (is_rom) {
			world->board_data[i] = h->func_getptr(h);
			h->func_skip(h, world->board_len[i]);
			continue;
		}
#endif
		world->board_data[i] = malloc(world->board_len[i]);
		if (world->board_data[i] == NULL)
			return ZOO_ERROR_NOMEM;
		h->func_read(h, world->board_data[i], world->board_len[i]);
	}

	return 0;
}

int zoo_io_world_write(zoo_io_handle *h, zoo_world *world) {
	int i;

	zoo_io_write_short(h, -1);
	zoo_io_write_short(h, world->board_count);

	zoo_io_write_short(h, world->info.ammo);
	zoo_io_write_short(h, world->info.gems);
	for (i = 0; i < 7; i++)
		zoo_io_write_byte(h, world->info.keys[i] ? 1 : 0);
	zoo_io_write_short(h, world->info.health);
	zoo_io_write_short(h, world->info.current_board);
	zoo_io_write_short(h, world->info.torches);
	zoo_io_write_short(h, world->info.torch_ticks);
	zoo_io_write_short(h, world->info.energizer_ticks);
	zoo_io_write_short(h, 0);
	zoo_io_write_short(h, world->info.score);
	zoo_io_write_pstring(h, 20, world->info.name, sizeof(world->info.name) - 1);
	for (i = 0; i < 10; i++)
		zoo_io_write_pstring(h, 20, world->info.flags[i], sizeof(world->info.flags[i]) - 1);
	zoo_io_write_short(h, world->info.board_time_sec);
	zoo_io_write_short(h, world->info.board_time_hsec);
	zoo_io_write_byte(h, world->info.is_save ? 1 : 0);
	h->func_skip(h, 247);

	for (i = 0; i <= world->board_count; i++) {
		zoo_io_write_short(h, world->board_len[i]);
		h->func_write(h, world->board_data[i], world->board_len[i]);
	}

	return 0;
}

int zoo_world_load(zoo_state *state, zoo_io_handle *h, bool title_only) {
	int ret;

	ret = zoo_world_close(state);
	if (ret) return ret;

	ret = zoo_io_world_read(h, &state->world, title_only);
	if (ret) return ret;
	state->return_board_id = state->world.info.current_board;

	ret = zoo_board_open(state, state->return_board_id);
	if (ret) return ret;

	return 0;
}

int zoo_world_save(zoo_state *state, zoo_io_handle *h) {
	int ret;

	ret = zoo_board_close(state);
	if (ret) return ret;

	ret = zoo_io_world_write(h, &state->world);
	if (ret) return ret;

	ret = zoo_board_open(state, state->world.info.current_board);
	if (ret) return ret;

	return 0;
}
