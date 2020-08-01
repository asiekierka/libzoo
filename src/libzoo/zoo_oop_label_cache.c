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

/**
 * Label cache implementation.
 * Method: Keep a simple, pre-calculated, pre-allocated list of
 * all areas in object code which look like labels.
 * 
 * Goals:
 * - improve performance of common commands, like #SEND, #ZAP and #RESTORE
 * - allow blocking writes to object code (on ROM-based platforms)
 */

void zoo_oop_label_cache_build(zoo_state *state, int16_t stat_id) {
	zoo_stat *stat = &state->board.stats[stat_id];
	int16_t label_count = 0;
	int16_t pos, label_pos, last_label_pos;

	if (stat->data != NULL && stat->data_len > 0) {
		if (stat->label_cache_size > 0) {
			return;
		}

		// check existing stats
		for (pos = 1; pos <= state->board.stat_count; pos++) {
			if (state->board.stats[pos].data == stat->data && state->board.stats[pos].label_cache_size > 0) {
				stat->label_cache = state->board.stats[pos].label_cache;
				stat->label_cache_size = state->board.stats[pos].label_cache_size;
				return;
			}
		}

		// count labels
		for (pos = 0; pos < (stat->data_len-1); pos++) {
			if (stat->data[pos] == '\r' && (stat->data[pos+1] == ':' || stat->data[pos+1] == '\'')) {
				label_count++;
				last_label_pos = pos;
				pos++;
			}
		}

		// create cache
		stat->label_cache_size = label_count + 1;
		if (label_count > 0) {
			stat->label_cache = malloc(sizeof(zoo_stat_label) * label_count);

			pos = 0;
			label_pos = 0;
			for (pos = 0; pos <= last_label_pos; pos++) {
				if (stat->data[pos] == '\r' && (stat->data[pos+1] == ':' || stat->data[pos+1] == '\'')) {
					stat->label_cache[label_pos].pos = pos;
					stat->label_cache[label_pos].zapped = stat->data[pos+1] == '\'';	
					pos++;
					label_pos++;
				}
			}

			// assert(label_pos == label_count);
		}
	} else {
		stat->label_cache_size = 0;
	}
}

static void zoo_oop_label_cache_free(zoo_state *state, int16_t stat_id) {
	zoo_stat *stat = &state->board.stats[stat_id];
	void *ptr;
	int pos;

	if (stat->label_cache_size > 0) {
		ptr = stat->label_cache;
		free(ptr);

		for (pos = 1; pos <= state->board.stat_count; pos++) {
			stat = &state->board.stats[pos];
			if (stat->label_cache_size > 0 && stat->label_cache == ptr) {
				free(stat->label_cache);
				stat->label_cache_size = 0;
			}
		}
	}
}

// FIXME: exposes internal
int16_t zoo_oop_find_string_from(zoo_state *state, int16_t stat_id, const char *str, int16_t start_pos, int16_t end_pos);

GBA_FAST_CODE
int16_t zoo_oop_label_cache_search(zoo_state *state, int16_t stat_id, const char *object_message, bool zapped) {
	int i;
	int label_cache_size;
	zoo_stat_label *label_cache;
	int16_t pos;

	zoo_oop_label_cache_build(state, stat_id);
	label_cache = state->board.stats[stat_id].label_cache;
	label_cache_size = state->board.stats[stat_id].label_cache_size - 1;

	for (i = 0; i < label_cache_size; i++) {
		// printf("id %d, entry %d: pos %d, %s\n", stat_id, i, label_cache[i].pos, label_cache[i].zapped ? "zapped" : "not zapped");
		if (zapped == label_cache[i].zapped) {
			pos = zoo_oop_find_string_from(state, stat_id, object_message,
				label_cache[i].pos + 2,
				label_cache[i].pos + 2
			);
			if (pos >= 2) {
				return pos - 2;
			}
		}
	}

	return -1;
}

GBA_FAST_CODE
void zoo_oop_label_cache_zap(zoo_state *state, int16_t stat_id, int16_t label_data_pos, bool zapped, bool recurse, const char *label) {
	zoo_stat *stat = &state->board.stats[stat_id];
	int ix;
	int16_t pos;

#ifdef ZOO_NO_OBJECT_CODE_WRITES
	// Emulate #ZAP/RESTORE restart. (writes)
	if (label_data_pos == 0) {
		stat->label_cache_chr2 = zapped ? '\'' : ':';
		zoo_oop_label_cache_free(state, stat_id);
	}
#endif

	zoo_oop_label_cache_build(state, stat_id);
	for (ix = 0; ix < stat->label_cache_size-1; ix++) {
		if (stat->label_cache[ix].pos == label_data_pos) {
			stat->label_cache[ix].zapped = zapped;
#ifndef ZOO_NO_OBJECT_CODE_WRITES
			stat->data[label_data_pos + 1] = zapped ? '\'' : ':';
#endif
			break;
		}
	}

	if (recurse) {
		// Find remaining positions
		for (ix++; ix < stat->label_cache_size-1; ix++) {
			if (stat->label_cache[ix].zapped != zapped) {
				pos = stat->label_cache[ix].pos;
				if (zoo_oop_find_string_from(state, stat_id, label, pos + 2, pos + 2) >= 0) {
					stat->label_cache[ix].zapped = zapped;
#ifndef ZOO_NO_OBJECT_CODE_WRITES
					stat->data[stat->label_cache[ix].pos + 1] = zapped ? '\'' : ':';
#endif
				}
			}
		}
	}
}
