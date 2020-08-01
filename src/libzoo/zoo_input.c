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
#include "zoo_internal.h"

void zoo_input_update(zoo_input_state *state) {
	int i, max_order = -1;
	int move_action = ZOO_ACTION_MAX;

	for (i = ZOO_ACTION_UP; i <= ZOO_ACTION_DOWN; i++) {
		if (state->actions_down[i] && state->actions_order[i] > max_order) {
			max_order = state->actions_order[i];
			move_action = i;
		}
	}

	switch (move_action) {
		case ZOO_ACTION_UP:
			state->delta_x = 0;
			state->delta_y = -1;
			break;
		case ZOO_ACTION_DOWN:
			state->delta_x = 0;
			state->delta_y = 1;
			break;
		case ZOO_ACTION_LEFT:
			state->delta_x = -1;
			state->delta_y = 0;
			break;
		case ZOO_ACTION_RIGHT:
			state->delta_x = 1;
			state->delta_y = 0;
			break;
		default:
			state->delta_x = 0;
			state->delta_y = 0;
			break;
	}
}

void zoo_input_clear(zoo_input_state *state) {
	int i;

	// update counts
	for (i = 0; i < ZOO_ACTION_MAX; i++) {
		state->actions_down[i] = false;
	}
}

void zoo_input_tick(zoo_input_state *state) {
	int i;

	// update counts
	for (i = 0; i < ZOO_ACTION_MAX; i++) {
		if (state->actions_held[i]) {
			state->actions_count[i]++;
			if (state->actions_count[i] >= state->repeat_end) {
				state->actions_count[i] = state->repeat_start;
				state->actions_down[i] = true;
			}
		}
	}
}

bool zoo_input_action_pressed(zoo_input_state *state, zoo_input_action action) {
	if (state->actions_down[action]) {
		state->actions_down[action] = false;
		return true;
	} else {
		return false;
	}
}

bool zoo_input_action_held(zoo_input_state *state, zoo_input_action action) {
	return state->actions_held[action];
}

void zoo_input_action_down(zoo_input_state *state, zoo_input_action action) {
	if (!state->actions_held[action]) {
		state->actions_down[action] = true;
		state->actions_held[action] = true;
		state->actions_count[action] = 0;
		state->actions_order[action] = state->pressed_count++;
	}
}

void zoo_input_action_up(zoo_input_state *state, zoo_input_action action) {
	uint8_t old_order;
	int i;

	if (state->actions_held[action]) {
		state->actions_held[action] = false;
		old_order = state->actions_order[action];
		state->actions_order[action] = 0;

		for (i = 0; i < ZOO_ACTION_MAX; i++) {
			if (state->actions_order[i] > old_order) {
				state->actions_order[i]--;
			}
		}

		state->pressed_count--;
	}
}

void zoo_input_action_set(zoo_input_state *state, zoo_input_action action, bool value) {
	if (value) {
		zoo_input_action_down(state, action);
	} else {
		zoo_input_action_up(state, action);
	}
}
