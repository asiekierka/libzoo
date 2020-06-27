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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tonc.h>
#include "zoo.h"
#include "zoo_io_romfs.h"
#include "zoo_sidebar.h"
#include "zoo_ui.h"
#include "zoo_gba.h"
#include "sound_gba.h"
#include "video_gba.h"

#include "4x6_bin.h"

extern u32 __rom_end__;

static zoo_state state;
static zoo_ui_state ui_state;
static zoo_io_romfs_driver d_io;

extern uint16_t keys_down;
extern uint16_t keys_held;
volatile bool tick_requested = false;
volatile uint16_t ticks = 0;

IWRAM_ARM_CODE static void irq_timer_pit(void) {
	REG_IE |= IRQ_VCOUNT; // ensure vcount will still happen
	REG_IME = 1;

	ticks++;
	tick_requested = true;
	zoo_tick_advance_pit(&state);
	zoo_sound_tick(&(state.sound));
}

int main(void) {
	zoo_io_handle io_h;

	zoo_video_gba_hide();
	irq_init(NULL);

	// init game speed timer
	REG_TM0CNT_L = 65536 - 14398;
	REG_TM0CNT_H = TM_FREQ_64 | TM_IRQ | TM_ENABLE;

	zoo_state_init(&state);
	zoo_video_gba_install(&state, _4x6_bin);
	zoo_sound_gba_install(&state);
	state.func_draw_sidebar = zoo_draw_sidebar_slim;

	if (!zoo_io_create_romfs_driver(&d_io, (uint8_t *) &__rom_end__)) {
		return 0;
	}
	state.d_io = (zoo_io_driver *) &d_io;

	zoo_ui_init(&ui_state, &state);
	zoo_video_gba_show();

	irq_add(II_TIMER0, irq_timer_pit);

	bool tick_in_progress = false;
	bool tick_next_frame = false;

	while(true) {
		if (tick_requested) {
			tick_requested = false;

			if ((keys_down & KEY_START) && zoo_call_empty(&state.call_stack)) {
				zoo_ui_main_menu(&ui_state);
			}

			keys_held |= keys_down;

			zoo_input_action_set(&state.input, ZOO_ACTION_UP, keys_held & KEY_UP);
			zoo_input_action_set(&state.input, ZOO_ACTION_LEFT, keys_held & KEY_LEFT);
			zoo_input_action_set(&state.input, ZOO_ACTION_RIGHT, keys_held & KEY_RIGHT);
			zoo_input_action_set(&state.input, ZOO_ACTION_DOWN, keys_held & KEY_DOWN);
			zoo_input_action_set(&state.input, ZOO_ACTION_SHOOT, keys_held & KEY_A);
			zoo_input_action_set(&state.input, ZOO_ACTION_TORCH, keys_held & KEY_B);
			zoo_input_action_set(&state.input, ZOO_ACTION_OK, keys_held & KEY_A);
			zoo_input_action_set(&state.input, ZOO_ACTION_CANCEL, keys_held & KEY_B);

			keys_down = 0;

			tick_in_progress = true;
			while (tick_in_progress) {
				switch (zoo_tick(&state)) {
					case RETURN_IMMEDIATE:
						break;
					case RETURN_NEXT_FRAME:
						tick_next_frame = true;
						tick_in_progress = false;
						break;
					case RETURN_NEXT_CYCLE:
						tick_in_progress = false;
						break;
					case EXIT:
						break;
				}
			}
		}

		if (tick_next_frame) {
			VBlankIntrWait();
			tick_requested = true;
			tick_next_frame = false;
		}
	}
	return 0;
}
