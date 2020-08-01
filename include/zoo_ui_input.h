/**
 * Copyright (c) 2020 Adrian Siekierka
 *
 * Based on a reconstruction of code from ZZT,
 * Copyright 1991 Epic MegaGames, used with permission.
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

// zoo_ui_input.h - keyboard/joystick/mouse -> libzoo/ui adapter

#ifndef __ZOO_UI_INPUT_H__
#define __ZOO_UI_INPUT_H__

#include <stdint.h>
#include "zoo.h"

// TODO: joystick/mouse adapter

#define ZOO_UI_KBDBUF_SIZE 16

#define ZOO_KEY_RELEASED 0x8000

typedef struct {
    uint16_t ui_kbd_buffer[ZOO_UI_KBDBUF_SIZE];
    uint16_t action_to_kbd_button[ZOO_ACTION_MAX];
} zoo_ui_input_state;

void zoo_ui_input_init(zoo_ui_input_state *inp);

void zoo_ui_input_key(zoo_state *zoo, zoo_ui_input_state *inp, uint16_t key, bool pressed);
void zoo_ui_input_key_map(zoo_ui_input_state *inp, zoo_input_action action, uint16_t key);
uint16_t zoo_ui_input_key_pop(zoo_ui_input_state *inp);

// keybinds

#define ZOO_KEY_BACKSPACE 8
#define ZOO_KEY_TAB 9
#define ZOO_KEY_ENTER 13
#define ZOO_KEY_ESCAPE 27
#define ZOO_KEY_F1 187
#define ZOO_KEY_F2 188
#define ZOO_KEY_F3 189
#define ZOO_KEY_F4 190
#define ZOO_KEY_F5 191
#define ZOO_KEY_F6 192
#define ZOO_KEY_F7 193
#define ZOO_KEY_F8 194
#define ZOO_KEY_F9 195
#define ZOO_KEY_F10 196
#define ZOO_KEY_UP 200
#define ZOO_KEY_PAGE_UP 201
#define ZOO_KEY_LEFT 203
#define ZOO_KEY_RIGHT 205
#define ZOO_KEY_DOWN 208
#define ZOO_KEY_PAGE_DOWN 209
#define ZOO_KEY_INSERT 210
#define ZOO_KEY_DELETE 211
#define ZOO_KEY_HOME 212
#define ZOO_KEY_END 213

#define ZOO_KEY_SHIFT 256

#endif /* __ZOO_UI_INPUT_H__ */