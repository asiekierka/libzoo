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

zoo_call *zoo_call_push(zoo_call_stack *stack, zoo_call_type type, uint8_t state) {
	zoo_call *new_call;

	new_call = malloc(sizeof(zoo_call));
	if (new_call == NULL) {
		return NULL;
	}
	new_call->next = stack->call;
	new_call->type = type;
	new_call->state = state;
	stack->call = new_call;

	return new_call;
}

zoo_call *zoo_call_push_callback(zoo_call_stack *stack, zoo_func_callback func, void *arg) {
	zoo_call *c = zoo_call_push(stack, CALLBACK, 0);

	if (c != NULL) {
		c->args.cb.func = func;
	c->args.cb.arg = arg;
	}

	return c;
}

void zoo_call_pop(zoo_call_stack *stack) {
	zoo_call *old_call;

	old_call = stack->call;
	stack->call = old_call->next;
	free(old_call);
}
