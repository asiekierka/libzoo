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

static const uint16_t zoo_sound_freqs[96] = {
	64,	67,	71,	76,	80,	85,	90,	95,	101,	107,	114,	120,	0,	0,	0,	0,
	128,	135,	143,	152,	161,	170,	181,	191,	203,	215,	228,	241,	0,	0,	0,	0,
	256,	271,	287,	304,	322,	341,	362,	383,	406,	430,	456,	483,	0,	0,	0,	0,
	512,	542,	574,	608,	645,	683,	724,	767,	812,	861,	912,	966,	0,	0,	0,	0,
	1024,	1084,	1149,	1217,	1290,	1366,	1448,	1534,	1625,	1722,	1824,	1933,	0,	0,	0,	0,
	2048,	2169,	2298,	2435,	2580,	2733,	2896,	3068,	3250,	3444,	3649,	3866,	0,	0,	0,	0
};

typedef struct {
	uint16_t len;
	uint16_t data[15];
} zoo_sound_drum;

static const zoo_sound_drum zoo_sound_drums[10] = {
	{ 1, {3200,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0}},
	{14, {1100,	1200,	1300,	1400,	1500,	1600,	1700,	1800,	1900,	2000,	2100,	2200,	2300,	2400,	0}},
	{14, {4800,	4800,	8000,	1600,	4800,	4800,	8000,	1600,	4800,	4800,	8000,	1600,	4800,	4800,	8000}},
	{14, {0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0}},
	{14, {500,	2556,	1929,	3776,	3386,	4517,	1385,	1103,	4895,	3396,	874,	1616,	5124,	606,	0}},
	{14, {1600,	1514,	1600,	821,	1600,	1715,	1600,	911,	1600,	1968,	1600,	1490,	1600,	1722,	1600}},
	{14, {2200,	1760,	1760,	1320,	2640,	880,	2200,	1760,	1760,	1320,	2640,	880,	2200,	1760,	0}},
	{14, {688,	676,	664,	652,	640,	628,	616,	604,	592,	580,	568,	556,	544,	532,	0}},
	{14, {1207,	1224,	1163,	1127,	1159,	1236,	1269,	1314,	1127,	1224,	1320,	1332,	1257,	1327,	0}},
	{14, {378,	331,	316,	230,	224,	384,	480,	320,	358,	412,	376,	621,	554,	426,	0}}
};

static ZOO_INLINE void zoo_nosound(zoo_sound_state *state) {
	if (state->d_sound != NULL && state->d_sound->func_play_freqs != NULL) {
		state->d_sound->func_play_freqs(state->d_sound, NULL, 0, false);
	}
}

static void zoo_default_play_note(zoo_sound_state *state, uint8_t note, uint8_t duration) {
	if (state->d_sound != NULL && state->d_sound->func_play_freqs != NULL) {
		state->d_sound->func_play_freqs(state->d_sound, NULL, 0, false);
		if (note >= 16 && note < 112) {
			state->d_sound->func_play_freqs(state->d_sound, &zoo_sound_freqs[note - 16], 1, false);
		} else if (note >= 240 && note < 250) {
			state->d_sound->func_play_freqs(state->d_sound, zoo_sound_drums[note - 240].data, zoo_sound_drums[note - 240].len, true);
		} else {
			// nosound - already called
		}
	}
}

void zoo_sound_queue(zoo_sound_state *state, int16_t priority, const uint8_t *data, int16_t len) {
	if (!state->block_queueing && (!state->is_playing || (
		((priority >= state->current_priority) && (state->current_priority != -1))
		|| (priority == -1)
	))) {
		if (priority >= 0 || !state->is_playing) {
			state->current_priority = priority;
			memcpy(state->buffer, data, len);
			state->buffer_pos = 0;
			state->buffer_len = len;
			state->duration_counter = 1;
		} else {
			// trim buffer
			if (state->buffer_pos > 0) {
				state->buffer_len -= state->buffer_pos;
				memmove(state->buffer, state->buffer + state->buffer_pos, state->buffer_len);
				state->buffer_pos = 0;
			}
			// append buffer
			if ((state->buffer_len + len) < 255) {
				memcpy(state->buffer + state->buffer_len, data, len);
				state->buffer_len += len;
			}
		}
		state->is_playing = true;
	}
}

void zoo_sound_clear_queue(zoo_sound_state *state) {
	state->buffer_len = 0;
	state->is_playing = false;
	zoo_nosound(state);
}

void zoo_sound_tick(zoo_sound_state *state) {
	uint8_t note, duration;

	if (!state->enabled) {
		state->is_playing = false;
		zoo_nosound(state);
	} else if (state->is_playing) {
		if (--(state->duration_counter) <= 0) {
			if (state->buffer_pos >= state->buffer_len) {
				zoo_nosound(state);
				state->is_playing = false;
			} else {
				note = state->buffer[(state->buffer_pos)++];
				duration = state->buffer[(state->buffer_pos)++];
				// TODO
				/* if (state->func_play_note != NULL) {
					state->func_play_note(state, note, duration);
				} */
				zoo_default_play_note(state, note, duration);
				state->duration_counter = state->duration_multiplier * duration;
			}
		}
	}
}

static const char letter_to_tone[7] = {9, 11, 0, 2, 4, 5, 7};

int16_t zoo_sound_parse(const char *input, uint8_t *output, int16_t out_max) {
	uint8_t note_octave = 3;
	uint8_t note_duration = 1;
	uint8_t note_tone;
	int16_t inpos;
	int16_t outpos = 0;

	for (inpos = 0; inpos < strlen(input); inpos++) {
		// check for overflow
		if ((outpos + 2) >= out_max) break;

		note_tone = -1;
		switch (zoo_toupper(input[inpos])) {
			case 'T': note_duration = 1; break;
			case 'S': note_duration = 2; break;
			case 'I': note_duration = 4; break;
			case 'Q': note_duration = 8; break;
			case 'H': note_duration = 16; break;
			case 'W': note_duration = 32; break;
			case '.': note_duration = note_duration * 3 / 2; break;
			case '3': note_duration = note_duration / 3; break;
			case '+': if (note_octave < 6) note_octave += 1; break;
			case '-': if (note_octave > 1) note_octave -= 1; break;

			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G': {
				note_tone = letter_to_tone[(input[inpos] - 'A') & 0x07];

				switch (input[inpos + 1]) {
					case '!': note_tone -= 1; inpos++; break;
					case '#': note_tone += 1; inpos++; break;
				}

				output[outpos++] = (note_octave << 4) | note_tone;
				output[outpos++] = note_duration;
			} break;

			case 'X': {
				output[outpos++] = 0;
				output[outpos++] = note_duration;
			} break;

			case '0':
			case '1':
			case '2':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9': {
				output[outpos++] = input[inpos] + 0xF0 - '0';
				output[outpos++] = note_duration;
			} break;
		}
	}

	return outpos;
}

void zoo_sound_state_init(zoo_sound_state *state) {
	memset(state, 0, sizeof(*state));
	state->enabled = true;
	state->block_queueing = false;
	state->duration_multiplier = 1;
	zoo_sound_clear_queue(state);
}
