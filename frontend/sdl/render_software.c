/**
 * Copyright (c) 2018, 2019, 2020 Adrian Siekierka
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

#include <stdint.h>
#include <stdlib.h>
#include "render_software.h"

static void software_draw_char_32bpp(render_options *opts, uint32_t *buffer, int pitch, int x, int y, uint8_t col, uint8_t chr) {
	int pos_mul = (opts->flags & RENDER_40COL) ? 2 : 1;

	uint8_t bg = col >> 4;
	uint8_t fg = col & 0xF;
	const uint8_t *co = opts->charset + (chr * opts->char_height);

	for (int cy = 0; cy < opts->char_height; cy++, co++) {
		int line = *co;
		int bpos = ((y * opts->char_height + cy) * (pitch >> 2)) + ((x * opts->char_width) * pos_mul);
		for (int cx = 0; cx < opts->char_width; cx++, line <<= 1, bpos += pos_mul) {
			uint32_t bcol = opts->palette[(line & 0x80) ? fg : bg];
			buffer[bpos] = bcol;
			if (pos_mul == 2) buffer[bpos+1] = bcol;
		}
	}
}

static void software_draw_char_8bpp(render_options *opts, uint8_t *buffer, int pitch, int x, int y, uint8_t col, uint8_t chr) {
	int pos_mul = (opts->flags & RENDER_40COL) ? 2 : 1;

	uint8_t bg = col >> 4;
	uint8_t fg = col & 0xF;
	const uint8_t *co = opts->charset + (chr * opts->char_height);

	for (int cy = 0; cy < opts->char_height; cy++, co++) {
		int line = *co;
		int bpos = ((y * opts->char_height + cy) * (pitch >> 2)) + ((x * opts->char_width) * pos_mul);
		for (int cx = 0; cx < opts->char_width; cx++, line <<= 1, bpos += pos_mul) {
			uint8_t bcol = (line & 0x80) ? fg : bg;
			buffer[bpos] = bcol;
			if (pos_mul == 2) buffer[bpos+1] = bcol;
		}
	}
}

void software_draw_char(render_options *opts, void *buffer, int pitch, int x, int y, uint8_t col, uint8_t chr) {
	if (col >= 0x80 && !(opts->flags & RENDER_BLINK_OFF)) {
		col &= 0x7F;
		if (opts->flags & RENDER_BLINK_PHASE) {
			col = (col >> 4) * 0x11;
		}
	}

	switch (opts->bpp) {
		case 8:
			software_draw_char_8bpp(opts, buffer, pitch, x, y, col, chr);
			break;
		case 32:
			software_draw_char_32bpp(opts, buffer, pitch, x, y, col, chr);
			break;
	}
}

void software_draw_screen(render_options *opts, void *buffer, int pitch, video_buffer *video) {
	int pos = 0;

	for (int y = 0; y < video->height; y++) {
		for (int x = 0; x < video->width; x++, pos += 2) {
			uint8_t chr = video->buffer[pos];
			uint8_t col = video->buffer[pos + 1];
			software_draw_char(opts, buffer, pitch, x, y, col, chr);
		}
	}
}
