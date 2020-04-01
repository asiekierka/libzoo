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
#include "zoo.h"

zoo_call *zoo_call_push(zoo_call_stack *stack, zoo_call_type type, uint8_t state) {
	// TODO: malloc check
	zoo_call *new_call;

	new_call = malloc(sizeof(zoo_call));
	new_call->next = stack->call;
	new_call->type = type;
	new_call->state = state;
	stack->call = new_call;

	return new_call;
}

void zoo_call_pop(zoo_call_stack *stack) {
	zoo_call *old_call;

	old_call = stack->call;
	stack->call = old_call->next;
	free(old_call);
}

static int16_t zoo_default_random(int16_t max) {
	return rand() % max;
}

static void* zoo_default_store_display(zoo_state *state) {
	int16_t ix, iy;
	uint8_t *data, *dp;

	if (state->func_read_char != NULL) {
		data = malloc(ZOO_BOARD_WIDTH * ZOO_BOARD_HEIGHT * 2);
		if (data != NULL) {
			// if null, assume nop route - this way out of memory isn't fatal
			dp = data;
			for (iy = 0; iy < ZOO_BOARD_HEIGHT; iy++) {
				for (ix = 0; ix < ZOO_BOARD_WIDTH; ix++, dp += 2) {
					state->func_read_char(ix, iy, dp, dp + 1);
				}
			}
		}

		return data;
	} else {
		// nop route
		return NULL;
	}
}

static void zoo_default_restore_display(zoo_state *state, void *data) {
	int16_t ix, iy;
	uint8_t *data8 = (uint8_t *)data;

	if (data != NULL && state->func_write_char != NULL) {
		for (iy = 0; iy < ZOO_BOARD_HEIGHT; iy++) {
			for (ix = 0; ix < ZOO_BOARD_WIDTH; ix++, data8 += 2) {
				state->func_write_char(ix, iy, data8[0], data8[1]);
			}
		}
	} else {
		// nop route
		zoo_board_draw(state);
	}
}

static void zoo_default_update_sidebar(zoo_state *state) {

}

void zoo_state_init(zoo_state *state) {
	memset(state, 0, sizeof(zoo_state));

	state->func_random = zoo_default_random;
	state->func_store_display = zoo_default_store_display;
	state->func_restore_display = zoo_default_restore_display;

	state->func_init_text_window = zoo_window_classic_init;
	state->func_update_sidebar = zoo_default_update_sidebar;

	state->tick_speed = 4;
	zoo_sound_state_init(&(state->sound));
	zoo_world_create(state);
}

#ifdef ZOO_CONFIG_USE_DOUBLE_FOR_MS
int16_t zoo_hsecs_to_pit_ticks(int16_t hsecs) {
	zoo_time_ms ms = hsecs * 10;
	return (int16_t) ceil((hsecs * 10) / ZOO_PIT_TICK_MS);
}

zoo_time_ms zoo_hsecs_to_pit_ms(int16_t hsecs) {
	return zoo_hsecs_to_pit_ticks(hsecs) * ZOO_PIT_TICK_MS;
}
#else
zoo_time_ms zoo_hsecs_to_pit_ms(int16_t hsecs) {
	zoo_time_ms ms = hsecs * 10 + ZOO_PIT_TICK_MS - 1;
	return ms - (ms % ZOO_PIT_TICK_MS);
}

int16_t zoo_hsecs_to_pit_ticks(int16_t hsecs) {
	zoo_time_ms ms = hsecs * 10 + ZOO_PIT_TICK_MS - 1;
	return ms / ZOO_PIT_TICK_MS;
}
#endif

void zoo_redraw(zoo_state *state) {
	zoo_board_draw(state);
	state->func_update_sidebar(state);
}
