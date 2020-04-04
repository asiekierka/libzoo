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

#include "4x6_bin.h"

#define FONT_HEIGHT 6
#define MAP_Y_OFFSET 1
#define MAP_ADDR_OFFSET 0x8000

static const u16 default_palette[] = {
	0x0000,
	0x5000,
	0x0280,
	0x5280,
	0x0014,
	0x5014,
	0x0154,
	0x5294,
	0x294A,
	0x7D4A,
	0x2BEA,
	0x7FEA,
	0x295F,
	0x7D5F,
	0x2BFF,
	0x7FFF
};
extern u32 __rom_end__;

static zoo_state state;
volatile uint16_t disp_y_offset;
volatile int keys_down = 0;
volatile int keys_held = 0;
volatile bool tick_requested = false;
volatile uint16_t ticks = 0;

static void vram_write_tile_1bpp(const uint8_t *data, uint32_t *vram_pos, uint8_t invert_mask, uint8_t shift) {
	for (int iy = 0; iy < 8; iy++, data++) {
		uint32_t out = 0;
		uint8_t in = ((*data) ^ invert_mask) >> shift;
		out |= ((in >> 7) & 1) << 28;
		out |= ((in >> 6) & 1) << 24;
		out |= ((in >> 5) & 1) << 20;
		out |= ((in >> 4) & 1) << 16;
		out |= ((in >> 3) & 1) << 12;
		out |= ((in >> 2) & 1) << 8;
		out |= ((in >> 1) & 1) << 4;
		out |= ((in) & 1);
		*(vram_pos++) = out;
	}
}

IWRAM_ARM_CODE static void vram_write_char(int16_t x, int16_t y, uint8_t col, uint8_t chr) {
	u16* tile_bg_ptr = (u16*) (MEM_VRAM + MAP_ADDR_OFFSET + ((x&1) << 11) + ((x>>1) << 1) + ((y + MAP_Y_OFFSET) << 6));
	u16* tile_fg_ptr = &tile_bg_ptr[1 << 11];
	*tile_bg_ptr = '\xDB' | (((col >> 4) & 0x07) << 12);
	*tile_fg_ptr = chr | (col << 12);
}

IWRAM_ARM_CODE static void irq_vcount(void) {
	uint16_t next_vcount;
	disp_y_offset += (8 - FONT_HEIGHT);
	next_vcount = REG_VCOUNT + FONT_HEIGHT;
	REG_BG0VOFS = disp_y_offset;
	REG_BG1VOFS = disp_y_offset;
	REG_BG2VOFS = disp_y_offset;
	REG_BG3VOFS = disp_y_offset;
	REG_DISPSTAT = DSTAT_VBL_IRQ | DSTAT_VCT_IRQ | DSTAT_VCT(next_vcount);
}

static inline void gba_clear_sound(void) {
	REG_SOUND2CNT_L = SSQR_DUTY1_2 | SSQR_IVOL(0);
	REG_SOUND2CNT_H = SFREQ_RESET;
}

IWRAM_ARM_CODE static void gba_play_freqs(zoo_sound_state *state, const uint16_t *freqs, uint16_t len, bool clear) {
	// TODO: support drums
	if (len != 1 || clear) {
		gba_clear_sound();
	} else {
		uint16_t freq = freqs[0];
		if (freq < 64) freq = 64;
		REG_SOUND2CNT_L = SSQR_DUTY1_2 | SSQR_IVOL(12);
		REG_SOUND2CNT_H = (2048 - (131072 / (int)freq)) | SFREQ_RESET;
	}
}

IWRAM_ARM_CODE static void irq_vblank(void) {
	int ki = REG_KEYINPUT;

	disp_y_offset = (state.game_state == GS_PLAY)
		? ((FONT_HEIGHT * MAP_Y_OFFSET) - ((SCREEN_HEIGHT - (FONT_HEIGHT * 26)) / 2))
		: ((FONT_HEIGHT * MAP_Y_OFFSET) - ((SCREEN_HEIGHT - (FONT_HEIGHT * 25)) / 2));
	REG_DISPSTAT = DSTAT_VBL_IRQ | DSTAT_VCT_IRQ | DSTAT_VCT(FONT_HEIGHT - disp_y_offset);
	REG_BG0VOFS = disp_y_offset;
	REG_BG1VOFS = disp_y_offset;
	REG_BG2VOFS = disp_y_offset;
	REG_BG3VOFS = disp_y_offset;

	keys_held &= ~ki;
	keys_down |= (~ki) & (~keys_held);
}

IWRAM_ARM_CODE static void irq_timer_pit(void) {
	ticks++;
	tick_requested = true;
	REG_IE |= IRQ_VCOUNT; // ensure vcount will still happen
	REG_IME = 1;
	zoo_tick_advance_pit(&state);
	zoo_sound_tick(&(state.sound));
}

int main(void) {
	// set forced blank until display data is loaded
	REG_DISPCNT = DCNT_BLANK;

	irq_init(NULL);
	irq_add(II_VCOUNT, irq_vcount);
	irq_add(II_VBLANK, irq_vblank);
	irq_add(II_TIMER0, irq_timer_pit);

	// load 4x6 charset
	for (int i = 0; i < 256; i++) {
		vram_write_tile_1bpp(_4x6_bin + i*8, ((u32*) (MEM_VRAM + i*32)), 0, 0);
	}

	// load palette
	for (int i = 0; i < 16; i++) {
		pal_bg_mem[(i<<4) | 0] = 0x0000;
		pal_bg_mem[(i<<4) | 1] = default_palette[i];
	}

	REG_BG0CNT = BG_PRIO(3) | BG_CBB(0) | BG_SBB((MAP_ADDR_OFFSET >> 11) + 0) | BG_4BPP | BG_SIZE0;
	REG_BG1CNT = BG_PRIO(2) | BG_CBB(0) | BG_SBB((MAP_ADDR_OFFSET >> 11) + 1) | BG_4BPP | BG_SIZE0;
	REG_BG2CNT = BG_PRIO(1) | BG_CBB(0) | BG_SBB((MAP_ADDR_OFFSET >> 11) + 2) | BG_4BPP | BG_SIZE0;
	REG_BG3CNT = BG_PRIO(0) | BG_CBB(0) | BG_SBB((MAP_ADDR_OFFSET >> 11) + 3) | BG_4BPP | BG_SIZE0;
	REG_BG0HOFS = 4;
	REG_BG0VOFS = 0;
	REG_BG1HOFS = 0;
	REG_BG1VOFS = 0;
	REG_BG2HOFS = 4;
	REG_BG2VOFS = 0;
	REG_BG3HOFS = 0;
	REG_BG3VOFS = 0;

	// clear display
	for (int iy = 0; iy < 32; iy++) {
		for (int ix = 0; ix < 60; ix++) {
			vram_write_char(ix, iy, 0x0F, ' ');
		}
	}

	// disable forced blanking
	VBlankIntrWait();
	REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3;

	// init game speed timer
	REG_TM0CNT_L = 65536 - 14398;
	REG_TM0CNT_H = TM_FREQ_64 | TM_IRQ | TM_ENABLE;

	zoo_state_init(&state);
	state.func_write_char = vram_write_char;
	state.sound.func_play_freqs = gba_play_freqs;
	zoo_install_sidebar_slim(&state);

	// init sound
	REG_SOUNDCNT_X = SSTAT_ENABLE;
	REG_SOUNDCNT_L = SDMG_LVOL(7) | SDMG_RVOL(7) | SDMG_LSQR2 | SDMG_RSQR2;
	REG_SOUNDCNT_H = SDS_DMG100;

	if (!zoo_world_load(&state, &__rom_end__, (1 << 25), true)) {
		return 0;
	}

	zoo_game_start(&state, GS_TITLE);
	zoo_redraw(&state);

	int j = 0;
	bool tick_in_progress = false;
	bool tick_next_frame = false;

	while(true) {
		if (tick_requested) {
			tick_requested = false;

			if (state.game_state == GS_TITLE) {
				if (keys_down & KEY_START) {
					zoo_game_stop(&state);
					gba_clear_sound();

					if (!zoo_world_load(&state, &__rom_end__, (1 << 25), false)) {
						return 0;
					}

					zoo_game_start(&state, GS_PLAY);
					zoo_redraw(&state);
				}
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
