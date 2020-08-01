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

#include <gem.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "zoo.h"
#include "zoo_io_posix.h"
#include "zoo_sidebar.h"
#include "zoo_sound_pcm.h"
#include "zoo_ui.h"

enum graphics_mode {
	GFX_MODE_MONO,
	GFX_MODE_COLOR16
};

extern uint8_t _4x8_bin[];
static MFDB font_4x8_m;
static MFDB screen_m;

static short wk_work_in[11], wk_work_out[57];
static short wk_handle;

static enum graphics_mode gfx_mode;
static short x_offset, y_offset;
static short old_pal_colors[16 * 3];
static short pal_colors[16 * 3] = {
	0, 0, 0,
	0, 0, 667,
	0, 667, 0,
	0, 667, 667,
	667, 0, 0,
	667, 0, 667,
	667, 333, 0,
	667, 667, 667,
	333, 333, 333,
	333, 333, 1000,
	333, 1000, 333,
	333, 1000, 1000,
	1000, 333, 333,
	1000, 333, 1000,
	1000, 1000, 333,
	1000, 1000, 1000
};

static void init_gfx(void) {
	memset(&font_4x8_m, 0, sizeof(MFDB));
	memset(&screen_m, 0, sizeof(MFDB));

	font_4x8_m.fd_addr = _4x8_bin;
	font_4x8_m.fd_w = 4 * 32;
	font_4x8_m.fd_h = 8 * 8;
	font_4x8_m.fd_wdwidth = font_4x8_m.fd_w >> 4;
	font_4x8_m.fd_nplanes = 1;

	if (gfx_mode == GFX_MODE_COLOR16) {
		for (short i = 0; i < 16; i++) {
			vq_color(wk_handle, i, 0, old_pal_colors + (i * 3));
			vs_color(wk_handle, i, pal_colors + (i * 3));
		}
	}
}

static void deinit_gfx(void) {
	if (gfx_mode == GFX_MODE_COLOR16) {
		for (short i = 0; i < 16; i++) {
			vs_color(wk_handle, i, old_pal_colors + (i * 3));
		}
	}
}

static zoo_state state;
static zoo_ui_state ui_state;
static zoo_video_driver video_driver;
static zoo_io_path_driver io_driver;

static void vdi_write_char(zoo_video_driver *drv, int16_t x, int16_t y, uint8_t col, uint8_t chr) {
	short coords[8];
	short colors[2];

	if (gfx_mode == GFX_MODE_COLOR16) {
		colors[0] = col & 15;
		colors[1] = (col >> 4) & 7;
	} else {
		uint8_t fg = col & 15;
		uint8_t bg = (col >> 4) & 7;

		colors[0] = fg != 0 ? 0 : 1;
		colors[1] = (fg == 0 && bg != 0) ? 0 : 1;
	}

	coords[4] = (x << 2) + x_offset;
	coords[5] = (y << 3) + y_offset;
	coords[6] = coords[4] + 3;
	coords[7] = coords[5] + 7;

	coords[0] = (chr & 31) << 2;
	coords[1] = (chr >> 5) << 3;
	coords[2] = coords[0] + 3;
	coords[3] = coords[1] + 7;

	vrt_cpyfm(wk_handle, 1, coords, &font_4x8_m, &screen_m, colors);
}

int main(int argc, char **argv) {
	srand(time(NULL));
	appl_init();

	wk_handle = graf_handle(NULL, NULL, NULL, NULL);
	for (int i = 0; i < 10; i++)
		wk_work_in[i] = 1;
	wk_work_in[10] = 2;

	v_opnvwk(wk_work_in, &wk_handle, wk_work_out);

	// TODO: crashes
	/* if (wk_work_out[0] < 319 || wk_work_out[1] < 199) {
		form_alert(1, "[3][Cannot launch:|Resolution too low!][ OK ]");
		goto FinishWk;
	} */

	x_offset = (wk_work_out[0] + 1 - 320) >> 1;
	y_offset = (wk_work_out[1] + 1 - 200) >> 1;
	gfx_mode = wk_work_out[13] >= 16 ? GFX_MODE_COLOR16 : GFX_MODE_MONO;

	v_exit_cur(wk_handle);
	init_gfx();

	zoo_state_init(&state);
	zoo_io_create_posix_driver(&io_driver);
	video_driver.func_write = vdi_write_char;
	state.d_io = &io_driver.parent;
	state.d_video = &video_driver;
	state.random_seed = rand();

	state.func_draw_sidebar = zoo_draw_sidebar_classic;

	zoo_redraw(&state);
	zoo_ui_init(&ui_state, &state);

	bool running = true;
	short tdelay = 55;
	while (running) {
		short kmeta;
		short kreturn;
		short recv_events = evnt_multi(
			MU_KEYBD | MU_TIMER,
			0, 0, 0,
			0, 0, 0, 0, 0,
			0, 0, 0, 0, 0,
			NULL,
			tdelay,
			NULL, NULL, NULL,
			NULL, &kreturn,
			NULL
		);

		if (recv_events & MU_KEYBD) {
			int action = ZOO_ACTION_MAX;
			switch (kreturn >> 8) { /* scancode */
			case 1: /* ESC */
				action = ZOO_ACTION_CANCEL;
				break;
			case 28: /* Enter */
				action = ZOO_ACTION_OK;
				break;
			case 20: /* T */
				action = ZOO_ACTION_TORCH;
				break;
			case 72: /* Up */
				action = ZOO_ACTION_UP;
				break;
			case 75: /* Left */
				action = ZOO_ACTION_LEFT;
				break;
			case 77: /* Right */
				action = ZOO_ACTION_RIGHT;
				break;
			case 80: /* Down */
				action = ZOO_ACTION_DOWN;
				break;
			}
			if (action < ZOO_ACTION_MAX) {
				zoo_input_action_down(&state.input, action);
				zoo_input_action_up(&state.input, action);
			} else if ((kreturn >> 8) == 59) {
				/* F1 */
				if (zoo_call_empty(&state.call_stack)) {
					zoo_ui_main_menu(&ui_state);
				}
			} else if ((kreturn & 0xDF) == 'Q') {
				running = false;
			}
		}
		if (recv_events & MU_TIMER) {	
			graf_mkstate(NULL, NULL, NULL, &kmeta);
			zoo_input_action_set(&state.input, ZOO_ACTION_SHOOT, (kmeta & 0x03) != 0);

			zoo_tick_advance_pit(&state);
			zoo_sound_tick(&state.sound);
			zoo_input_tick(&state.input);

			bool ticking = true;
			while (ticking) {
				switch (zoo_tick(&state)) {
					case RETURN_IMMEDIATE:
						break;
					case RETURN_NEXT_FRAME:
						tdelay = 16;
						ticking = false;
						break;
					case RETURN_NEXT_CYCLE:
						tdelay = 55;
						ticking = false;
						break;
				}
			}
		}
	}

	deinit_gfx();

FinishWk:
	v_clsvwk(wk_handle);

	appl_exit();

	return 0;
}
