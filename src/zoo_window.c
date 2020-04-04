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

void zoo_window_append(zoo_text_window *window, const char *text) {
	char *buffer;
	int16_t buflen = strlen(text);
	if (buflen > 50) buflen = 50;

	buffer = malloc(sizeof(char) * (buflen + 1));
	memcpy(buffer, text, buflen);
	buffer[buflen] = '\0';

	window->lines = realloc(window->lines, sizeof(char*) * (window->line_count + 1));
	window->lines[(window->line_count)++] = buffer;
}

void zoo_window_close(zoo_text_window *window) {
	int16_t i;

	for (i = 0; i < window->line_count; i++) {
		free(window->lines[i]);
	}
	free(window->lines);
}
