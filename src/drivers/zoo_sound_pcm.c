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
#include "zoo_sound_pcm.h"

void zoo_sound_pcm_generate(zoo_sound_pcm_driver *pcm, uint8_t *stream, size_t len) {
	uint8_t n_min = pcm->format_signed ? (-(pcm->volume >> 1)) : (0x80 - (pcm->volume >> 1));
	uint8_t n_ctr = pcm->format_signed ? 0 : 0x80;
	uint8_t n_max = pcm->format_signed ? (pcm->volume >> 1) : (0x80 + (pcm->volume >> 1));
	size_t pos = 0;
	double samples_per_tick = pcm->frequency * 55 / 1000.0;
	double samples_per_drum = pcm->frequency * 1 / 1000.0;
	double e_remains;
	double e_len;
	bool e_completes;
	uint32_t e_ticker, e_freq_id, i;
	double e_ticker_mod;
	zoo_pcm_entry *e_curr, *e_next;
	uint16_t e_next_pos;

	while (pos < len) {
		if ((pcm->ticks - pcm->buf_ticks) < pcm->latency) {
			pcm->buf_ticks = pcm->ticks - pcm->latency;
		}

			e_curr = NULL;
			e_next = NULL;
			if (pcm->buf_pos != pcm->buf_len) {
				e_curr = &(pcm->buffer[pcm->buf_pos]);
				if (e_curr->tick > (pcm->buf_ticks - pcm->latency)) {
					e_curr = NULL;
				}
				if (e_curr != NULL) {
					e_next_pos = (pcm->buf_pos + 1) % ZOO_CONFIG_SOUND_PCM_BUFFER_LEN;
					if (e_next_pos != pcm->buf_len) {
						e_next = &(pcm->buffer[e_next_pos]);
					}
					if (e_next != NULL && e_next->tick <= (pcm->buf_ticks - pcm->latency)) {
						// we have already played this... advance
						pcm->buf_pos = e_next_pos;
						continue;
					}
				}
			} else {
				// nothing in queue, use chance to force resync
				pcm->buf_ticks = pcm->ticks - pcm->latency;
			}


			e_remains = len - pos;
			e_len = samples_per_tick - pcm->sub_ticks;
			e_completes = true;
			if (e_remains < e_len) {
				e_len = e_remains;
				e_completes = false;
			}

			if (e_curr != NULL) {
				e_ticker = e_curr->emitted;
			}

			// not played yet!
			if (e_curr != NULL && e_curr->len > 0) {
				for (i = 0; i < e_len; i++, e_ticker++) {
					if (e_ticker < samples_per_drum) {
						stream[pos + i] = n_ctr;
					} else {
						e_freq_id = (e_ticker / samples_per_drum) - 1;
						if (e_freq_id < 0) e_freq_id = 0;
						if (e_freq_id >= e_curr->len) {
							if (!e_curr->clear) {
								e_freq_id = e_curr->len - 1;
							} else {
								// note finished
								stream[pos + i] = n_ctr;
								continue;
							}
						}

						e_ticker_mod = ((double) pcm->frequency / e_curr->freqs[e_freq_id]) / 2.0;
						stream[pos + i] = ((int) (e_ticker / e_ticker_mod) & 1) ? n_max : n_min;
					}
				}
			} else {
				// no sound
				memset(stream + pos, n_ctr, e_len);
			}

			if (e_curr != NULL) {
				e_curr->emitted = e_ticker;
			}

			pos += e_len;
			if (!e_completes) {
				// partial sample/end of processing
				pcm->sub_ticks += e_len;
			} else {
				// full new sample
				pcm->sub_ticks = 0;
				pcm->buf_ticks++;
			}
	}
}

static void zoo_sound_pcm_push(zoo_sound_pcm_driver *pcm, const uint16_t *freqs, uint16_t len, bool clear) {
	pcm->buffer[pcm->buf_len].tick = pcm->ticks;
	pcm->buffer[pcm->buf_len].emitted = 0;
	pcm->buffer[pcm->buf_len].freqs = freqs;
	pcm->buffer[pcm->buf_len].pos = 0;
	pcm->buffer[pcm->buf_len].len = len;
	pcm->buffer[pcm->buf_len].clear = clear;
	pcm->buf_len = (pcm->buf_len + 1) % ZOO_CONFIG_SOUND_PCM_BUFFER_LEN;
}

void zoo_sound_pcm_init(zoo_sound_pcm_driver *pcm) {
	pcm->parent.func_play_freqs = zoo_sound_pcm_push;

	pcm->buf_pos = 0;
	pcm->buf_len = 0;
	pcm->buf_ticks = 0;
	pcm->sub_ticks = 0;
	pcm->ticks = pcm->latency;
}

void zoo_sound_pcm_tick(zoo_sound_pcm_driver *pcm) {
	pcm->ticks++;
}