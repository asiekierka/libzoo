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
// EWRAM_BSS
static zoo_io_romfs_driver d_io;

extern volatile uint16_t keys_down;
extern volatile uint16_t keys_held;
volatile bool tick_requested = false;
volatile uint16_t ticks = 0;

#ifdef ZOO_DEBUG_MENU

#include <malloc.h>
#include <unistd.h>

/* https://devkitpro.org/viewtopic.php?f=6&t=3057 */

extern uint8_t *fake_heap_end;
extern uint8_t *fake_heap_start;

int platform_debug_free_memory(void) {
	struct mallinfo info = mallinfo();
	return info.fordblks + (fake_heap_end - (uint8_t*)sbrk(0));
}

#endif

bool platform_is_rom_ptr(void *ptr) {
	return (((u32) ptr) & 0x08000000) != 0;
}

IWRAM_ARM_CODE static void irq_timer_pit(void) {
	REG_IE |= (IRQ_VBLANK | IRQ_VCOUNT);
	REG_IME = 1;

	ticks++;
	tick_requested = true;
	zoo_tick_advance_pit(&state);
	zoo_sound_tick(&(state.sound));
}

#define dbg_ticks() (REG_TM2CNT_L | (REG_TM3CNT_L << 16))

#ifdef DEBUG_CONSOLE
#define DBG_TICK_TIME_LEN 16
static u32 dbg_tick_times[DBG_TICK_TIME_LEN];
static u16 dbg_tick_time_pos = 0;
#endif

int main(void) {
	zoo_io_handle io_h;

	zoo_video_gba_hide();
	irq_init(isr_master_nest);

	// init game speed timer
	REG_TM0CNT_L = 65536 - 14398;
	REG_TM0CNT_H = TM_FREQ_64 | TM_IRQ | TM_ENABLE;

#ifdef DEBUG_CONSOLE
	// init ticktime counter
	REG_TM2CNT_L = 0;
	REG_TM2CNT_H = TM_FREQ_1 | TM_ENABLE;
	REG_TM3CNT_L = 0;
	REG_TM3CNT_H = TM_FREQ_1 | TM_CASCADE | TM_ENABLE;
#endif

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
	bool used_start = false;

	while(true) {
		if (tick_requested) {
			tick_requested = false;
#ifdef DEBUG_CONSOLE
			u32 tick_time = dbg_ticks();
#endif
			keys_held = keys_down;

			zoo_input_action_set(&state.input, ZOO_ACTION_UP, keys_held & KEY_UP);
			zoo_input_action_set(&state.input, ZOO_ACTION_LEFT, keys_held & KEY_LEFT);
			zoo_input_action_set(&state.input, ZOO_ACTION_RIGHT, keys_held & KEY_RIGHT);
			zoo_input_action_set(&state.input, ZOO_ACTION_DOWN, keys_held & KEY_DOWN);
			zoo_input_action_set(&state.input, ZOO_ACTION_SHOOT, keys_held & KEY_A);
			zoo_input_action_set(&state.input, ZOO_ACTION_TORCH, keys_held & KEY_B);
			zoo_input_action_set(&state.input, ZOO_ACTION_OK, keys_held & KEY_A);
			zoo_input_action_set(&state.input, ZOO_ACTION_CANCEL, keys_held & (KEY_B | KEY_START | KEY_SELECT));

			if ((keys_down & KEY_START) && zoo_call_empty(&state.call_stack)) {
				zoo_input_action_pressed(&state.input, ZOO_ACTION_CANCEL);
				zoo_ui_main_menu(&ui_state);
			}

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

#ifdef DEBUG_CONSOLE
			if (!tick_next_frame) {
				dbg_tick_times[(dbg_tick_time_pos++) % DBG_TICK_TIME_LEN] = dbg_ticks() - tick_time;

				u32 avg_tick_time = 0;
				u32 max_tick_time = 0;
				for (int i = 0; i < DBG_TICK_TIME_LEN; i++) {
					avg_tick_time += dbg_tick_times[i];
					if (dbg_tick_times[i] > max_tick_time) {
						max_tick_time = dbg_tick_times[i];
					}
				}
				avg_tick_time = (avg_tick_time + (DBG_TICK_TIME_LEN >> 1)) / DBG_TICK_TIME_LEN;

				zoo_ui_debug_printf(true, "tick time: avg %d cy, max %d cy\n", avg_tick_time, max_tick_time);
				fflush(stdout);
			}
#endif
		}

		if (tick_next_frame) {
			VBlankIntrWait();
			tick_requested = true;
			tick_next_frame = false;
		} else if (!tick_requested) {
			Halt();
		}
	}
	return 0;
}
