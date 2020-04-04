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

#ifdef ZOO_CONFIG_ENABLE_POSIX_FILE_IO

static uint8_t zoo_io_file_getc(zoo_io_handle *h) {
	FILE *f = (FILE*) h->p;
	// TODO: EOF check
	return fgetc(f);
}

static size_t zoo_io_file_putc(zoo_io_handle *h, uint8_t v) {
	FILE *f = (FILE*) h->p;
	// TODO: EOF check
	fputc(v, f);
	return 1;
}

static size_t zoo_io_file_read(zoo_io_handle *h, uint8_t *d_ptr, size_t len) {
	FILE *f = (FILE*) h->p;
	return fread(d_ptr, 1, len, f);
}

static size_t zoo_io_file_write(zoo_io_handle *h, const uint8_t *d_ptr, size_t len) {
	FILE *f = (FILE*) h->p;
	return fwrite(d_ptr, 1, len, f);
}

static size_t zoo_io_file_skip(zoo_io_handle *h, size_t len) {
	FILE *f = (FILE*) h->p;
	// TODO: check if works on writes
	fseek(f, len, SEEK_CUR);
	return len;
}

static size_t zoo_io_file_tell(zoo_io_handle *h) {
	FILE *f = (FILE*) h->p;
	return ftell(f);
}

static zoo_io_handle zoo_io_file_open(FILE *file) {
	zoo_io_handle h;
	h.p = file;
	h.func_getc = zoo_io_file_getc;
	h.func_putc = zoo_io_file_putc;
	h.func_read = zoo_io_file_read;
	h.func_write = zoo_io_file_write;
	h.func_skip = zoo_io_file_skip;
	h.func_tell = zoo_io_file_tell;
	return h;
}

#endif

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
	int i;

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
	stat->data = NULL; h->func_skip(h, 4);
	stat->data_pos = zoo_io_read_short(h);
	stat->data_len = zoo_io_read_short(h);
	h->func_skip(h, 8);
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
	h->func_skip(h, 4); // stat->data
	zoo_io_write_short(h, stat->data_pos);
	zoo_io_write_short(h, stat->data_len);
	h->func_skip(h, 8);
}

static size_t zoo_board_io_max_len(zoo_state *state) {
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
	size += 33 * state->board.stat_count;
	for (ix = 0; ix <= state->board.stat_count; ix++)
		if (state->board.stats[ix].data_len > 0)
			size += state->board.stats[ix].data_len;

	return size;
}

static void zoo_board_io_close(zoo_state *state, zoo_io_handle *h) {
	int ix, iy;
	zoo_rle_tile rle;
	zoo_stat *stat;

	zoo_io_write_pstring(h, 50, state->board.name, sizeof(state->board.name) - 1);

	ix = 1;
	iy = 1;
	rle.count = 1;
	rle.tile = state->board.tiles[ix][iy];
	do {
		ix++;
		if (ix > ZOO_BOARD_WIDTH) {
			ix = 1;
			iy++;
		}

		if (
			(state->board.tiles[ix][iy].color == rle.tile.color)
			&& (state->board.tiles[ix][iy].element == rle.tile.element)
			&& (rle.count < 255)
			&& (iy <= ZOO_BOARD_HEIGHT)
		) {
			rle.count++;
		} else {
			zoo_io_write_byte(h, rle.count);
			zoo_io_write_tile(h, rle.tile);
			rle.tile = state->board.tiles[ix][iy];
			rle.count = 1;
		}
	} while (iy <= ZOO_BOARD_HEIGHT);

	zoo_io_write_byte(h, state->board.info.max_shots);
	zoo_io_write_byte(h, state->board.info.is_dark);
	for (ix = 0; ix < 4; ix++)
		zoo_io_write_byte(h, state->board.info.neighbor_boards[ix]);
	zoo_io_write_byte(h, state->board.info.reenter_when_zapped);
	zoo_io_write_pstring(h, 58, state->board.info.message, ZOO_LEN_MESSAGE);
	zoo_io_write_byte(h, state->board.info.start_player_x);
	zoo_io_write_byte(h, state->board.info.start_player_y);
	zoo_io_write_short(h, state->board.info.time_limit_sec);
	h->func_skip(h, 16);

	zoo_io_write_short(h, state->board.stat_count);
	stat = &state->board.stats[0];
	for (ix = 0; ix <= state->board.stat_count; ix++, stat++) {
		if (stat->data_len > 0) {
			for (iy = 1; iy < ix; iy++) {
				if (state->board.stats[iy].data == stat->data) {
					stat->data_len = -iy;
				}
			}
		}
		zoo_stat_io_write(h, stat);
		if (stat->data_len > 0) {
			h->func_write(h, (uint8_t *) stat->data, stat->data_len);
			free(stat->data);
		}
	}
}

static void zoo_board_io_open(zoo_state *state, zoo_io_handle *h) {
	int ix, iy;
	zoo_rle_tile rle;
	zoo_stat *stat;

	zoo_io_read_pstring(h, 50, state->board.name, sizeof(state->board.name) - 1);

	ix = 1;
	iy = 1;
	rle.count = 0;
	do {
		if (rle.count <= 0) {
			rle.count = zoo_io_read_byte(h);
			rle.tile = zoo_io_read_tile(h);
		}
		state->board.tiles[ix][iy] = rle.tile;
		ix++;
		if (ix > ZOO_BOARD_WIDTH) {
			ix = 1;
			iy++;
		}
		rle.count--;
	} while (iy <= ZOO_BOARD_HEIGHT);

	state->board.info.max_shots = zoo_io_read_byte(h);
	state->board.info.is_dark = zoo_io_read_byte(h);
	for (ix = 0; ix < 4; ix++)
		state->board.info.neighbor_boards[ix] = zoo_io_read_byte(h);
	state->board.info.reenter_when_zapped = zoo_io_read_byte(h);
	zoo_io_read_pstring(h, 58, state->board.info.message, ZOO_LEN_MESSAGE);
	state->board.info.start_player_x = zoo_io_read_byte(h);
	state->board.info.start_player_y = zoo_io_read_byte(h);
	state->board.info.time_limit_sec = zoo_io_read_short(h);
	h->func_skip(h, 16);

	state->board.stat_count = zoo_io_read_short(h);
	stat = &state->board.stats[0];

	for (ix = 0; ix <= state->board.stat_count; ix++, stat++) {
		zoo_stat_io_read(h, stat);
		if (stat->data_len > 0) {
			// TODO: malloc check
			stat->data = malloc(stat->data_len);
			h->func_read(h, (uint8_t *) stat->data, stat->data_len);
		} else if (stat->data_len < 0) {
			// TODO: bounds check
			stat->data = state->board.stats[-stat->data_len].data;
			stat->data_len = state->board.stats[-stat->data_len].data_len;
		}
	}
}

void zoo_board_close(zoo_state *state) {
	zoo_io_handle handle;
	int16_t board_id;
	size_t buf_len;

	board_id = state->world.info.current_board;
	if (state->world.board_data[board_id] != NULL) {
		free(state->world.board_data[board_id]);
		state->world.board_data[board_id] = NULL;
	}

	buf_len = zoo_board_io_max_len(state);
	// TODO: malloc check
	state->world.board_data[board_id] = malloc(buf_len);

	handle = zoo_io_open_file_mem(state->world.board_data[board_id], buf_len, true);

	zoo_board_io_close(state, &handle);
	if (handle.func_tell(&handle) != buf_len) {
		state->world.board_len[board_id] = handle.func_tell(&handle);
		// TODO: realloc check?
		state->world.board_data[board_id] =
			realloc(state->world.board_data[board_id], state->world.board_len[board_id]);
	}
}

void zoo_board_open(zoo_state *state, int16_t board_id) {
	zoo_io_handle handle;
	if (board_id > state->world.board_count) {
		board_id = state->world.info.current_board;
	}

	handle = zoo_io_open_file_mem(
		state->world.board_data[board_id],
		state->world.board_len[board_id],
		false
	);

	zoo_board_io_open(state, &handle);
	state->world.info.current_board = board_id;
}

//
void zoo_world_close(zoo_state *state) {
	int i;

	zoo_board_close(state);
	for (i = 0; i <= state->world.board_count; i++) {
		free(state->world.board_data[i]);
	}
}

bool zoo_world_load(zoo_state *state, zoo_io_handle *h, bool title_only) {
	int i;
	zoo_world_close(state);

	state->world.board_count = zoo_io_read_short(h);
	if (state->world.board_count < 0) {
		if (state->world.board_count != -1) {
			return false;
		} else {
			state->world.board_count = zoo_io_read_short(h);
		}
	}

	state->world.info.ammo = zoo_io_read_short(h);
	state->world.info.gems = zoo_io_read_short(h);
	for (i = 0; i < 7; i++)
		state->world.info.keys[i] = zoo_io_read_byte(h);
	state->world.info.health = zoo_io_read_short(h);
	state->world.info.current_board = zoo_io_read_short(h);
	state->world.info.torches = zoo_io_read_short(h);
	state->world.info.torch_ticks = zoo_io_read_short(h);
	state->world.info.energizer_ticks = zoo_io_read_short(h);
	h->func_skip(h, 2);
	state->world.info.score = zoo_io_read_short(h);
	zoo_io_read_pstring(h, 20, state->world.info.name, sizeof(state->world.info.name) - 1);
	for (i = 0; i < 10; i++)
		zoo_io_read_pstring(h, 20, state->world.info.flags[i], sizeof(state->world.info.flags[i]) - 1);
	state->world.info.board_time_sec = zoo_io_read_short(h);
	state->world.info.board_time_hsec = zoo_io_read_short(h);
	state->world.info.is_save = zoo_io_read_byte(h);
	h->func_skip(h, 247);

	if (title_only) {
		state->world.board_count = 0;
		state->world.info.current_board = 0;
		state->world.info.is_save = true;
	}

	for (i = 0; i <= state->world.board_count; i++) {
		state->world.board_len[i] = zoo_io_read_short(h);
		state->world.board_data[i] = malloc(state->world.board_len[i]);
		h->func_read(h, state->world.board_data[i], state->world.board_len[i]);
	}

	zoo_board_open(state, state->world.info.current_board);

	return true;
}
