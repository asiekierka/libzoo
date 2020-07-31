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
#include <stdlib.h>
#include <string.h>
#include <tonc.h>
#include "zoo.h"
#include "zoo_gba.h"
#include "video_gba.h"

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

static zoo_state *inst_state;
static uint16_t disp_y_offset;
static bool blinking_enabled = false;
static uint8_t blink_ticks;

volatile uint16_t keys_down = 0;
volatile uint16_t keys_held = 0;

IWRAM_ARM_CODE
static void vram_update_bgcnt(void) {
#ifdef ZOO_GBA_ENABLE_BLINKING
	int fg_cbb = (blinking_enabled && (blink_ticks & 16)) ? 1 : 0;
#else
	int fg_cbb = 0;
#endif

	REG_BG0CNT = BG_PRIO(3) | BG_CBB(0) | BG_SBB((MAP_ADDR_OFFSET >> 11) + 0) | BG_4BPP | BG_SIZE0;
	REG_BG1CNT = BG_PRIO(2) | BG_CBB(0) | BG_SBB((MAP_ADDR_OFFSET >> 11) + 1) | BG_4BPP | BG_SIZE0;
	REG_BG2CNT = BG_PRIO(1) | BG_CBB(fg_cbb) | BG_SBB((MAP_ADDR_OFFSET >> 11) + 2) | BG_4BPP | BG_SIZE0;
	REG_BG3CNT = BG_PRIO(0) | BG_CBB(fg_cbb) | BG_SBB((MAP_ADDR_OFFSET >> 11) + 3) | BG_4BPP | BG_SIZE0;
}

IWRAM_ARM_CODE
static void vram_write_tile_1bpp(const uint8_t *data, uint32_t *vram_pos) {
	for (int iy = 0; iy < 8; iy++, data++) {
		uint32_t out = 0;
		uint8_t in = *data;
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

#define GET_VRAM_PTRS \
	u16* tile_bg_ptr = (u16*) (MEM_VRAM + MAP_ADDR_OFFSET + ((x&1) << 11) + ((x>>1) << 1) + ((y + MAP_Y_OFFSET) << 6)); \
	u16* tile_fg_ptr = &tile_bg_ptr[1 << 11]

IWRAM_ARM_CODE static void vram_write_char(zoo_video_driver *drv, int16_t x, int16_t y, uint8_t col, uint8_t chr) {
	GET_VRAM_PTRS;

#ifdef ZOO_GBA_ENABLE_BLINKING
	*tile_bg_ptr = '\xDB' | (((col >> 4) & 0x07) << 12);
	*tile_fg_ptr = chr | ((col & 0x80) << 1) | (col << 12);
#else
	*tile_bg_ptr = '\xDB' | (((col >> 4) & 0x07) << 12);
	*tile_fg_ptr = chr | (col << 12);
#endif
}

static void vram_read_char(zoo_video_driver *drv, int16_t x, int16_t y, uint8_t *col, uint8_t *chr) {
	GET_VRAM_PTRS;

#ifdef ZOO_GBA_ENABLE_BLINKING	
	*chr = (*tile_fg_ptr & 0xFF);
	*col = (*tile_fg_ptr >> 12) | ((*tile_bg_ptr >> 8) & 0x70) | ((*tile_fg_ptr >> 1) & 0x80);
#else
	*chr = (*tile_fg_ptr & 0xFF);
	*col = (*tile_fg_ptr >> 12) | ((*tile_bg_ptr >> 8) & 0x70);
#endif
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

IWRAM_ARM_CODE static void irq_vblank(void) {
	uint16_t ki = REG_KEYINPUT;

	disp_y_offset = (inst_state->game_state == GS_PLAY)
		? ((FONT_HEIGHT * MAP_Y_OFFSET) - ((SCREEN_HEIGHT - (FONT_HEIGHT * 26)) / 2))
		: ((FONT_HEIGHT * MAP_Y_OFFSET) - ((SCREEN_HEIGHT - (FONT_HEIGHT * 25)) / 2));
	REG_DISPSTAT = DSTAT_VBL_IRQ | DSTAT_VCT_IRQ | DSTAT_VCT(FONT_HEIGHT - disp_y_offset);

#ifdef DEBUG_CONSOLE
	if ((~ki) & KEY_R) {
		disp_y_offset = (8 * 31) - (8 * 26) + 2;
		REG_DISPSTAT = DSTAT_VBL_IRQ | DSTAT_VCT_IRQ | DSTAT_VCT(4);
	}
#endif

	REG_BG0VOFS = disp_y_offset;
	REG_BG1VOFS = disp_y_offset;
	REG_BG2VOFS = disp_y_offset;
	REG_BG3VOFS = disp_y_offset;

	keys_down |= ~ki;

#ifdef ZOO_GBA_ENABLE_BLINKING
	if (blinking_enabled) {
		if (((blink_ticks++) & 15) == 0) {
			vram_update_bgcnt();
		}
	}
#endif
}

static zoo_video_driver d_video_gba = {
	.func_write = vram_write_char,
	.func_read = vram_read_char
};

void zoo_video_gba_hide(void) {
	REG_DISPCNT = DCNT_BLANK;
}

void zoo_video_gba_show(void) {
	zoo_redraw(inst_state);

	VBlankIntrWait();
	REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_BG2 | DCNT_BG3;
}

void zoo_video_gba_set_blinking(bool val) {
	blinking_enabled = val;
	vram_update_bgcnt();
}

#ifdef DEBUG_CONSOLE
static u16 console_x = 0;
static u16 console_y = 0;

#define CONSOLE_WIDTH 60
#define CONSOLE_HEIGHT 3
#define CONSOLE_YOFFSET 27

void platform_debug_puts(const char *text, bool status) {
	if (status) {
		// clear line
		int x = 0;
		int y = CONSOLE_YOFFSET + CONSOLE_HEIGHT;
		GET_VRAM_PTRS;
		
		memset32(tile_fg_ptr, 0, 16);
		memset32(tile_fg_ptr + (1 << 10), 0, 16);

		while (*text != '\0') {
			char c = *(text++);
			if (c == '\n') continue;
			vram_write_char(NULL, x++, CONSOLE_YOFFSET + CONSOLE_HEIGHT, 0x0A, c);
		}
	} else {
		while (*text != '\0') {
			char c = *(text++);

			if (c == '\n') {
				console_x = 0;
				console_y += 1;
				continue;
			}

			while (console_y >= CONSOLE_HEIGHT) {
				// scroll one up
				int x = 0;
				int y = CONSOLE_YOFFSET;
				GET_VRAM_PTRS;

				memcpy32(tile_fg_ptr, tile_fg_ptr + 32, 16 * (CONSOLE_HEIGHT - 1));
				memcpy32(tile_fg_ptr + (1 << 10), tile_fg_ptr + 32 + (1 << 10), 16 * (CONSOLE_HEIGHT - 1));

				memset32(tile_fg_ptr + (32*(CONSOLE_HEIGHT-1)), 0, 16);
				memset32(tile_fg_ptr + (32*(CONSOLE_HEIGHT-1)) + (1 << 10), 0, 16);

				console_y--;
			}

			vram_write_char(NULL, console_x, console_y + CONSOLE_YOFFSET, 0x0F, c);
			console_x += 1;
			if (console_x >= CONSOLE_WIDTH) {
				console_x = 0;
				console_y += 1;
			}
		}
	}
}
#endif

void zoo_video_gba_install(zoo_state *state, const uint8_t *charset_bin) {
	// initialize state
	inst_state = state;
	state->d_video = &d_video_gba;

	// add interrupts
	irq_add(II_VCOUNT, irq_vcount);
	irq_add(II_VBLANK, irq_vblank);

	// load 4x6 charset
	for (int i = 0; i < 256; i++) {
		vram_write_tile_1bpp(charset_bin + i*8, ((uint32_t*) (MEM_VRAM + i*32)));
	}
#ifdef ZOO_GBA_ENABLE_BLINKING
	// 32KB is used to faciliate blinking:
	// chars 0-255: charset, not blinking
	// chars 256-511: charset, blinking, visible
	// chars 512-767: [blink] charset, not blinking
	// chars 768-1023: [blink] empty space, not visible
	memcpy32(((uint32_t*) (MEM_VRAM + (256*32))), ((uint32_t*) (MEM_VRAM)), 256 * 32);
	memcpy32(((uint32_t*) (MEM_VRAM + (512*32))), ((uint32_t*) (MEM_VRAM)), 256 * 32);
	memset32(((uint32_t*) (MEM_VRAM + (768*32))), 0x0000000, 256 * 32);
#endif

	// load palette
	for (int i = 0; i < 16; i++) {
		pal_bg_mem[(i<<4) | 0] = 0x0000;
		pal_bg_mem[(i<<4) | 1] = default_palette[i];
	}

	// initialize background registers
	vram_update_bgcnt();
	REG_BG0HOFS = 4;
	REG_BG0VOFS = 0;
	REG_BG1HOFS = 0;
	REG_BG1VOFS = 0;
	REG_BG2HOFS = 4;
	REG_BG2VOFS = 0;
	REG_BG3HOFS = 0;
	REG_BG3VOFS = 0;

	// clear display
	memset32((void*) (MEM_VRAM + MAP_ADDR_OFFSET), 0x00000000, 64 * 32 * 2);
}
