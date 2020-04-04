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

extern zoo_state state;

const static uint16_t *sound_freqs;
static uint16_t sound_len;
static bool sound_clear_flag;

static inline void gba_play_sound(uint16_t freq) {
	if (freq < 64) {
		REG_SOUND2CNT_L = SSQR_DUTY1_2 | SSQR_IVOL(0);
		REG_SOUND2CNT_H = SFREQ_RESET;
	} else {
		REG_SOUND2CNT_L = SSQR_DUTY1_2 | SSQR_IVOL(12);
		REG_SOUND2CNT_H = (2048 - (131072 / (int)freq)) | SFREQ_RESET;
	}
}

IWRAM_ARM_CODE static void irq_timer_sound(void) {
	REG_IE |= IRQ_VCOUNT; // ensure vcount will still happen
	REG_IME = 1;

	if (sound_len == 0) {
		gba_play_sound(0);
	} else {
		gba_play_sound(*(sound_freqs++));
		sound_len--;
		if (sound_len > 0 || sound_clear_flag) {
			// set timer
			REG_TM1CNT_L = 65536 - 262;
			REG_TM1CNT_H = TM_FREQ_64 | TM_IRQ | TM_ENABLE;
			return;
		}
	}

	// reset timer
	REG_TM1CNT_H = 0;
}

IWRAM_ARM_CODE static void gba_play_freqs(zoo_sound_state *state, const uint16_t *freqs, uint16_t len, bool clear) {
	sound_freqs = freqs;
	sound_len = len;
	sound_clear_flag = clear;
	irq_timer_sound();
}

void sound_clear(void) {
	gba_play_sound(0);
}

void sound_install(void) {
	irq_add(II_TIMER1, irq_timer_sound);

	// init sound
	REG_SOUNDCNT_X = SSTAT_ENABLE;
	REG_SOUNDCNT_L = SDMG_LVOL(7) | SDMG_RVOL(7) | SDMG_LSQR2 | SDMG_RSQR2;
	REG_SOUNDCNT_H = SDS_DMG100;

	state.sound.func_play_freqs = gba_play_freqs;
}
