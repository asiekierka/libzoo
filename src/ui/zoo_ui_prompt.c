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

#include <stdlib.h>
#include <string.h>
#include "../libzoo/zoo_internal.h"
#include "zoo_ui_internal.h"

static void zoo_ui_prompt_string_draw(zoo_state *zoo, zoo_ui_prompt_state *state) {
    int i, buf_len;
    zoo_video_driver *d_video = zoo->d_video;

    buf_len = strlen(state->buffer);
    for (i = 0; i < state->width; i++) {
        d_video->func_write(d_video, state->x + i, state->y, state->text_color, (i < buf_len) ? state->buffer[i] : ' ');
        d_video->func_write(d_video, state->x + i, state->y - 1, state->arrow_color, ' ');
    }
    d_video->func_write(d_video, state->x + state->width, state->y - 1, state->arrow_color, ' ');
    d_video->func_write(d_video, state->x + buf_len, state->y - 1, (state->arrow_color & 0xF0) | 0x0F, 31);
}

static zoo_tick_retval zoo_ui_prompt_string_cb(zoo_state *zoo, zoo_ui_prompt_state *state) {
    uint16_t key;
    int buf_len;
    bool accepted;
    bool changed = false;
    zoo_tick_retval ret_cb;

	while ((key = zoo_ui_input_key_pop(&state->ui->input)) != 0) {
        if (key & ZOO_KEY_RELEASED) continue;

        if (key >= 32 && key < 128) {
            if (state->first_key_press) {
                state->buffer[0] = '\0';
                state->first_key_press = false;
            }

            accepted = false;
            switch (state->mode) {
            case ZOO_UI_PROMPT_NUMERIC:
                accepted = (key >= '0') && (key <= '9');
                break;
            case ZOO_UI_PROMPT_ALPHANUMERIC:
                if (
                    ((zoo_toupper(key) >= 'A') && (zoo_toupper(key) <= 'Z'))
                    || ((key >= '0') && key <= '9') || (key == '-')
                ) {
                    accepted = true;
                    key = zoo_toupper(key);
                }
                break;
            case ZOO_UI_PROMPT_ANY:
                accepted = true;
                break;
            }

            if (accepted) {
                buf_len = strlen(state->buffer);
                if (buf_len < state->width) {
                    state->buffer[buf_len] = key;
                    state->buffer[buf_len + 1] = '\0';
                }
                changed = true;
            }
        } else if (key == ZOO_KEY_LEFT || key == ZOO_KEY_BACKSPACE) {
            buf_len = strlen(state->buffer);
            if (buf_len > 0) {
                state->buffer[buf_len - 1] = '\0';
                changed = true;
            }
        } else if (key == ZOO_KEY_ENTER || key == ZOO_KEY_ESCAPE) {
            ret_cb = state->callback(state->ui, key == ZOO_KEY_ESCAPE ? state->orig_buffer : state->buffer, key != ZOO_KEY_ESCAPE);
            zoo_restore_display(state->ui->zoo, state->screen_copy, 60, 25, 0, 0, 60, 25, 0, 0);
            zoo_free_display(state->ui->zoo, state->screen_copy);
            free(state);
            return EXIT;
        }

        state->first_key_press = false;
    }

    if (changed) {
        zoo_ui_prompt_string_draw(zoo, state);
    }

    return RETURN_NEXT_FRAME;
}

static void zoo_ui_prompt_state_init(zoo_ui_state *state, zoo_ui_prompt_state *prompt, const char *answer, uint16_t x, uint16_t y, uint16_t width) {
    memset(prompt, 0, sizeof(zoo_ui_prompt_state));
    prompt->x = x;
    prompt->y = y;
    prompt->width = width;
    prompt->mode = ZOO_UI_PROMPT_ANY;
    prompt->ui = state;
    prompt->screen_copy = zoo_store_display(state->zoo, 0, 0, 60, 25);
    strncpy(prompt->orig_buffer, answer, width);
    prompt->orig_buffer[width] = '\0';
    strcpy(prompt->buffer, prompt->orig_buffer);
}

void zoo_ui_popup_prompt_string(zoo_ui_state *state, zoo_ui_prompt_mode mode, uint16_t x, uint16_t y, uint16_t width,
    const char *question, const char *answer, zoo_ui_prompt_string_callback cb)
{
    uint16_t inner_x = x + 7;
    uint16_t inner_y = y + 4;
    uint16_t inner_width = width - 14;
    uint8_t color = 0x4F;
    zoo_ui_prompt_state *prompt = malloc(sizeof(zoo_ui_prompt_state));
    if (prompt == NULL) {
        cb(state, answer, false);
        return;
    }

    if (width > ZOO_UI_PROMPT_WIDTH) width = ZOO_UI_PROMPT_WIDTH;
    width = inner_width + 14;

    zoo_window_draw_pattern(state->zoo, x, y    , width, color, ZOO_WINDOW_PATTERN_TOP);
    zoo_window_draw_pattern(state->zoo, x, y + 1, width, color, ZOO_WINDOW_PATTERN_INNER);
    zoo_window_draw_pattern(state->zoo, x, y + 2, width, color, ZOO_WINDOW_PATTERN_SEPARATOR);
    zoo_window_draw_pattern(state->zoo, x, y + 3, width, color, ZOO_WINDOW_PATTERN_INNER);
    zoo_window_draw_pattern(state->zoo, x, y + 4, width, color, ZOO_WINDOW_PATTERN_INNER);
    zoo_window_draw_pattern(state->zoo, x, y + 5, width, color, ZOO_WINDOW_PATTERN_BOTTOM);

    zoo_ui_prompt_state_init(state, prompt, answer, inner_x, inner_y, inner_width);
    prompt->arrow_color = prompt->text_color = color;
    prompt->mode = mode;
    prompt->callback = cb;

    // TODO: move elsewhere (general key buffer reset)
    while (zoo_ui_input_key_pop(&state->input) != 0);
    zoo_call_push_callback(&state->zoo->call_stack, (zoo_func_callback) zoo_ui_prompt_string_cb, prompt);

    zoo_ui_prompt_string_draw(state->zoo, prompt);
}