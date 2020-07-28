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

#ifndef __ZOO_SOUND_PCM__
#define __ZOO_SOUND_PCM__

#include <stddef.h>
#include <stdint.h>
#include "zoo.h"

typedef struct {
	uint32_t tick;
	uint32_t emitted;
	const uint16_t *freqs;
	uint16_t pos, len;
	bool clear;
} zoo_pcm_entry;

typedef struct {
    zoo_sound_driver parent;

	// user-configurable
	uint32_t frequency;
	uint8_t channels;
	uint8_t volume;
	bool format_signed;
	uint8_t latency; // in ticks

	// generated
	zoo_pcm_entry buffer[ZOO_CONFIG_SOUND_PCM_BUFFER_LEN];
	uint16_t buf_pos;
	uint16_t buf_len;
	int32_t buf_ticks;
	double sub_ticks;
	int32_t ticks;
} zoo_sound_pcm_driver;

void zoo_sound_pcm_init(zoo_sound_pcm_driver *drv);
void zoo_sound_pcm_generate(zoo_sound_pcm_driver *drv, uint8_t *stream, size_t len);
void zoo_sound_pcm_tick(zoo_sound_pcm_driver *drv);

#endif /* __ZOO_SOUND_PCM__ */