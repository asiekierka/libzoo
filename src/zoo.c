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

zoo_call *zoo_call_push_callback(zoo_call_stack *stack, zoo_func_callback func, void *arg) {
	zoo_call *c = zoo_call_push(stack, CALLBACK, 0);

	c->args.cb.func = func;
	c->args.cb.arg = arg;

	return c;
}

void zoo_call_pop(zoo_call_stack *stack) {
	zoo_call *old_call;

	old_call = stack->call;
	stack->call = old_call->next;
	free(old_call);
}

static int16_t zoo_default_random(zoo_state *state, int16_t max) {
	state->random_seed = (state->random_seed * 134775813) + 1;
	return state->random_seed % max;
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

static void zoo_default_ui_draw_sidebar(zoo_state *state, uint16_t flags) {

}

void zoo_state_init(zoo_state *state) {
	memset(state, 0, sizeof(zoo_state));

	state->func_random = zoo_default_random;
	state->func_store_display = zoo_default_store_display;
	state->func_restore_display = zoo_default_restore_display;

	state->func_ui_draw_sidebar = zoo_default_ui_draw_sidebar;
	zoo_install_window_classic(state);

	state->tick_speed = 4;
	zoo_sound_state_init(&(state->sound));
	zoo_world_create(state);
}

bool zoo_has_hsecs_elapsed(zoo_state *state, int16_t *hsecs_counter, int16_t hsecs_value) {
	int16_t hsecs_total = (int16_t) (state->time_elapsed / 10);
	int16_t hsecs_diff = (hsecs_total - (*hsecs_counter) + 6000) % 6000;
	if (hsecs_diff >= hsecs_value) {
		*hsecs_counter = hsecs_total;
		return true;
	} else {
		return false;
	}
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
	state->func_ui_draw_sidebar(state, ZOO_SIDEBAR_UPDATE_ALL_REDRAW);
}
