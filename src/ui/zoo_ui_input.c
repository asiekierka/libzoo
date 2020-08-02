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

void zoo_ui_input_init(zoo_ui_input_state *inp) {
	memset(inp, 0, sizeof(zoo_ui_input_state));

	zoo_ui_input_key_map(inp, ZOO_ACTION_UP, ZOO_KEY_UP);
	zoo_ui_input_key_map(inp, ZOO_ACTION_DOWN, ZOO_KEY_DOWN);
	zoo_ui_input_key_map(inp, ZOO_ACTION_LEFT, ZOO_KEY_LEFT);
	zoo_ui_input_key_map(inp, ZOO_ACTION_RIGHT, ZOO_KEY_RIGHT);
	zoo_ui_input_key_map(inp, ZOO_ACTION_TORCH, 't');
	zoo_ui_input_key_map(inp, ZOO_ACTION_OK, ZOO_KEY_ENTER);
	zoo_ui_input_key_map(inp, ZOO_ACTION_CANCEL, ZOO_KEY_ESCAPE);
}

void zoo_ui_input_key(zoo_state *zoo, zoo_ui_input_state *inp, uint16_t key, bool pressed) {
	int i;
	if (key == 0) return;

	for (i = 0; i < ZOO_ACTION_MAX; i++) {
		if (inp->action_to_kbd_button[i] == key) {
			zoo_input_action_set(&zoo->input, i, pressed);
		}
	}

	for (i = 0; i < ZOO_UI_KBDBUF_SIZE; i++) {
		if (inp->ui_kbd_buffer[i] == 0) {
			inp->ui_kbd_buffer[i] = key | (pressed ? 0 : ZOO_KEY_RELEASED);
			break;
		}
	}
}

void zoo_ui_input_key_map(zoo_ui_input_state *inp, zoo_input_action action, uint16_t key) {
	inp->action_to_kbd_button[action] = key;
}

uint16_t zoo_ui_input_key_pop(zoo_ui_input_state *inp) {
	uint16_t key = inp->ui_kbd_buffer[0];

	if (key != 0) {
		memmove(inp->ui_kbd_buffer, inp->ui_kbd_buffer + 1, sizeof(uint16_t) * (ZOO_UI_KBDBUF_SIZE - 1));
		inp->ui_kbd_buffer[ZOO_UI_KBDBUF_SIZE - 1] = 0;
	}
	return key;
}