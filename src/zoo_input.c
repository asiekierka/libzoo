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

static void zoo_input_movement_update(zoo_input_state *state) {
	zoo_input_action move_action = ZOO_ACTION_MAX;
	zoo_input_action i;

	// check for new arrow key presses
	if (move_action == ZOO_ACTION_MAX) {
		for (i = ZOO_ACTION_UP; i <= ZOO_ACTION_DOWN; i++) {
			if (state->actions_down[i]) {
				state->actions_down[i] = false;
				move_action = i;
			}
		}
	}

	// continue moving in existing direction
	if (move_action == ZOO_ACTION_MAX) {
	 	if (state->delta_x == 0) {
			if (state->delta_y < 0) { move_action = ZOO_ACTION_UP; }
			else if (state->delta_y > 0) { move_action = ZOO_ACTION_DOWN; }
		} else if (state->delta_y == 0) {
			if (state->delta_x < 0) { move_action = ZOO_ACTION_LEFT; }
			else if (state->delta_x > 0) { move_action = ZOO_ACTION_RIGHT; }
		}

		if (move_action != ZOO_ACTION_MAX && !state->actions_held[move_action]) {
			move_action = ZOO_ACTION_MAX;
		}
	}

	// check for old arrow key presses
	if (move_action == ZOO_ACTION_MAX) {
		for (i = ZOO_ACTION_UP; i <= ZOO_ACTION_DOWN; i++) {
			if (state->actions_held[i]) {
				move_action = i;
				break;
			}
		}
	}

	switch (move_action) {
		case ZOO_ACTION_MAX:
			state->delta_x = 0;
			state->delta_y = 0;
			break;
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
	}
}

void zoo_input_update(zoo_input_state *state) {
	zoo_input_movement_update(state);
	state->updated = true;
}

void zoo_input_clear_post_tick(zoo_input_state *state) {
	if (state->updated) {
		memset(state->actions_down, 0, sizeof(state->actions_down));
		state->updated = false;
	}
}

bool zoo_input_action_pressed(zoo_input_state *state, zoo_input_action action) {
	if (state->actions_down[action]) {
		state->actions_down[action] = false;
		return true;
	} else if (state->actions_held[action]) {
		return true;
	} else {
		return false;
	}
}

bool zoo_input_action_pressed_once(zoo_input_state *state, zoo_input_action action) {
	if (state->actions_down[action]) {
		state->actions_down[action] = false;
		return true;
	} else {
		return false;
	}
}

void zoo_input_action_down(zoo_input_state *state, zoo_input_action action) {
	state->actions_down[action] = true;
	state->actions_held[action] = true;
}

void zoo_input_action_once(zoo_input_state *state, zoo_input_action action) {
	state->actions_down[action] = true;
}

void zoo_input_action_up(zoo_input_state *state, zoo_input_action action) {
	state->actions_held[action] = false;
}

void zoo_input_action_set(zoo_input_state *state, zoo_input_action action, bool value) {
	if (value) {
		if (!state->actions_held[action]) {
			state->actions_down[action] = true;
		}
		state->actions_held[action] = true;
	} else {
		state->actions_held[action] = false;
	}
}
