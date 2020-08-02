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
#include "zoo_ui_internal.h"
#include "zoo_ui_input.h"

#define ZOO_OSK_SHIFT 128

#define ZOO_OSK_WIDTH 33
#define ZOO_OSK_HEIGHT 8
#define ZOO_OSK_OX 4
#define ZOO_OSK_OY 2

static const zoo_osk_entry osk_entries_default[] = {
	{0,  0, '1', '!'},
	{2,  0, '2', '@'},
	{4,  0, '3', '#'},
	{6,  0, '4', '$'},
	{8,  0, '5', '%'},
	{10, 0, '6', '^'},
	{12, 0, '7', '&'},
	{14, 0, '8', '*'},
	{16, 0, '9', '('},
	{18, 0, '0', ')'},
	{20, 0, '-', '_'},
	{22, 0, '=', '+'},
	{24, 0, 8,   27},
	{1,  1, 'q', 'Q'},
	{3,  1, 'w', 'W'},
	{5,  1, 'e', 'E'},
	{7,  1, 'r', 'R'},
	{9,  1, 't', 'T'},
	{11, 1, 'y', 'Y'},
	{13, 1, 'u', 'U'},
	{15, 1, 'i', 'I'},
	{17, 1, 'o', 'O'},
	{19, 1, 'p', 'P'},
	{21, 1, '[', '{'},
	{23, 1, ']', '}'},
	{0, 2, ZOO_OSK_SHIFT, 24},
	{2,  2, 'a', 'A'},
	{4,  2, 's', 'S'},
	{6,  2, 'd', 'D'},
	{8,  2, 'f', 'F'},
	{10, 2, 'g', 'G'},
	{12, 2, 'h', 'H'},
	{14, 2, 'j', 'J'},
	{16, 2, 'k', 'K'},
	{18, 2, 'l', 'L'},
	{20, 2, ';', ':'},
	{22, 2, '\'', '"'},
	{24, 2, 13,  26},
	{3,  3, 'z', 'Z'},
	{5,  3, 'x', 'X'},
	{7,  3, 'c', 'C'},
	{9,  3, 'v', 'V'},
	{11, 3, 'b', 'B'},
	{13, 3, 'n', 'N'},
	{15, 3, 'm', 'M'},
	{17, 3, ',', '<'},
	{19, 3, '.', '>'},
	{21, 3, '/', '?'},
	{0, 0, 0, 0}
};

static void zoo_osk_draw(zoo_state *state, zoo_osk_state *osk, bool redraw) {
	int i, ix, iy;
	const zoo_osk_entry *entry = osk->entries;
	zoo_video_driver *d_video = state->d_video;

	if (redraw) {
		zoo_window_draw_pattern(state, osk->x, osk->y, ZOO_OSK_WIDTH, 0x1F, ZOO_WINDOW_PATTERN_TOP);
		for (iy = 1; iy < ZOO_OSK_HEIGHT - 1; iy++) {
			zoo_window_draw_pattern(state, osk->x, osk->y + iy, ZOO_OSK_WIDTH, 0x1F, ZOO_WINDOW_PATTERN_INNER);
		}
		zoo_window_draw_pattern(state, osk->x, osk->y + ZOO_OSK_HEIGHT - 1, ZOO_OSK_WIDTH, 0x1F, ZOO_WINDOW_PATTERN_BOTTOM);
	}

	i = 0;
	for (; entry->normal != '\0'; entry++, i++) {
		ix = osk->x + ZOO_OSK_OX + entry->x;
		iy = osk->y + ZOO_OSK_OY + entry->y;
		d_video->func_write(d_video, ix, iy, osk->e_pos == i ? 0x71 : 0x1F,
			(entry->normal >= 32 && entry->normal < 128) ? (osk->shifted ? entry->shifted : entry->normal) : entry->shifted);
	}	
}

void zoo_osk_tick(zoo_ui_state *ui, zoo_osk_state *osk) {
	int delta_x = 0;
	int delta_y = 0;
	int i, dist, min_dist, min_pos;
	int old_e_pos = osk->e_pos;
	const zoo_osk_entry *i_entry;
	const zoo_osk_entry *entry;

	if (!osk->active) return;

	if (zoo_input_action_pressed(&ui->zoo->input, ZOO_ACTION_CANCEL)) {
		// ESCAPE
		zoo_ui_input_key(ui->zoo, &ui->input, ZOO_KEY_ESCAPE, true);
		zoo_ui_input_key(ui->zoo, &ui->input, ZOO_KEY_ESCAPE, false);
		return;
	}

	entry = &osk->entries[osk->e_pos];

	if (zoo_input_action_pressed(&ui->zoo->input, ZOO_ACTION_OK)) {
		if (entry->normal == ZOO_OSK_SHIFT) {
			osk->shifted = !osk->shifted;
			zoo_ui_input_key(ui->zoo, &ui->input, ZOO_KEY_SHIFT, osk->shifted);
			old_e_pos = -1; // force draw
		} else if (entry->normal >= 32 && entry->normal < 128) {
			zoo_ui_input_key(ui->zoo, &ui->input, osk->shifted ? entry->shifted : entry->normal, true);
			zoo_ui_input_key(ui->zoo, &ui->input, osk->shifted ? entry->shifted : entry->normal, false);
		} else {
			zoo_ui_input_key(ui->zoo, &ui->input, entry->normal, true);
			zoo_ui_input_key(ui->zoo, &ui->input, entry->normal, false);
		}
	}

	if (zoo_input_action_pressed(&ui->zoo->input, ZOO_ACTION_LEFT)) delta_x--;
	if (zoo_input_action_pressed(&ui->zoo->input, ZOO_ACTION_RIGHT)) delta_x++;
	if (zoo_input_action_pressed(&ui->zoo->input, ZOO_ACTION_UP)) delta_y--;
	if (zoo_input_action_pressed(&ui->zoo->input, ZOO_ACTION_DOWN)) delta_y++;

	if (delta_y != 0) {
		min_dist = (1 << 30);
		min_pos = -1;
		for (i = 0, i_entry = osk->entries; i_entry->normal != '\0'; i++, i_entry++) {
			if (i == osk->e_pos) {
				continue;
			}
			if (zoo_signum(i_entry->y - entry->y) == zoo_signum(delta_y)
				&& ((zoo_signum(i_entry->x - entry->x) == zoo_signum(delta_y) || entry->x == i_entry->x)))
			{
				dist = (zoo_abs(i_entry->y - entry->y) << 2) + zoo_abs(i_entry->x - entry->x);
				if (dist < min_dist) {
					min_dist = dist;
					min_pos = i;
				}
			}
		}
		if (min_pos >= 0) {
			osk->e_pos = min_pos;
		} 
	}
	
	if (delta_x != 0) {
		min_dist = (1 << 30);
		min_pos = -1;
		for (i = 0, i_entry = osk->entries; i_entry->normal != '\0'; i++, i_entry++) {
			if (i == osk->e_pos) {
				continue;
			}
			if (i_entry->y == entry->y && zoo_signum(i_entry->x - entry->x) == zoo_signum(delta_x)) {
				dist = zoo_abs(i_entry->x - entry->x);
				if (dist < min_dist) {
					min_dist = dist;
					min_pos = i;
				}
			}
		}
		if (min_pos >= 0) {
			osk->e_pos = min_pos;
		} 
	}

	if (osk->e_pos != old_e_pos) {
		zoo_osk_draw(ui->zoo, osk, false);
	}
}

void zoo_osk_open(zoo_ui_state *ui, zoo_osk_state *osk, uint8_t x, uint8_t y) {
	memset(osk, 0, sizeof(zoo_osk_state));
	osk->x = x;
	osk->y = y;
	osk->entries = osk_entries_default;
	osk->active = true;
	
	zoo_ui_input_key(ui->zoo, &ui->input, ZOO_KEY_SHIFT, false);
	
	osk->screen_copy = zoo_store_display(ui->zoo, x, y, ZOO_OSK_WIDTH, ZOO_OSK_HEIGHT);
	zoo_osk_draw(ui->zoo, osk, true);
}

void zoo_osk_close(zoo_ui_state *ui, zoo_osk_state *osk) {
	zoo_ui_input_key(ui->zoo, &ui->input, ZOO_KEY_SHIFT, false);

	zoo_restore_display(ui->zoo, osk->screen_copy, ZOO_OSK_WIDTH, ZOO_OSK_HEIGHT,
		0, 0, ZOO_OSK_WIDTH, ZOO_OSK_HEIGHT,
		osk->x, osk->y);
	zoo_free_display(ui->zoo, osk->screen_copy);

	osk->active = false;
}