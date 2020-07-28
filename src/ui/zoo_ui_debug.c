#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zoo_ui.h"

extern int platform_debug_free_memory(void);
extern void platform_debug_puts(const char *str, bool status);

static char debug_buffer[80 + 1];

void zoo_ui_debug_printf(bool status, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsniprintf(debug_buffer, sizeof(debug_buffer), format, args);
    va_end(args);
    platform_debug_puts(debug_buffer, status);
}

static ZOO_INLINE void zoo_ui_dbg_memfree(zoo_state *zoo) {
	zoo_ui_debug_printf(false, "free mem = %d bytes\n", platform_debug_free_memory());
}

static ZOO_INLINE void zoo_ui_dbg_memtest(zoo_state *zoo) {
	// Load every board.
	int curr_board = zoo->world.info.current_board;
	int i, j;

	zoo_board_close(zoo);

	for (i = 0; i <= zoo->world.board_count; i++) {
		zoo_board_open(zoo, i);
		zoo_ui_debug_printf(false, "opening board %d, free mem = %d bytes\n", i, platform_debug_free_memory());
#ifdef ZOO_USE_LABEL_CACHE
		for (j = 0; j <= zoo->board.stat_count; j++)
			zoo_oop_label_cache_build(zoo, j);
#endif
		zoo_board_close(zoo);
	}

	zoo_board_open(zoo, curr_board);
}


static zoo_tick_retval zoo_ui_debug_menu_cb(zoo_state *zoo, zoo_ui_state *cb_state) {
	char hyperlink[21];
	strncpy(hyperlink, cb_state->window.hyperlink, sizeof(hyperlink));

	zoo_window_close(&cb_state->window);
	zoo_call_pop(&zoo->call_stack);

	if (!strcmp(hyperlink, "memfree")) {
		zoo_ui_dbg_memfree(zoo);
	} else if (!strcmp(hyperlink, "bmemtest")) {
		zoo_ui_dbg_memtest(zoo);
	}

	return RETURN_IMMEDIATE;
}

void zoo_ui_debug_menu(struct s_zoo_ui_state *state) {
	zoo_ui_init_select_window(state, "Debug Menu");
	
	zoo_window_append(&state->window, "!memfree;Print free memory");
	if (state->zoo->game_state == GS_PLAY && !state->zoo->game_paused) {
		zoo_window_append(&state->window, "!bmemtest;Test board memory usage");
	}

	zoo_call_push_callback(&(state->zoo->call_stack), (zoo_func_callback) zoo_ui_debug_menu_cb, state);
	zoo_window_open(state->zoo, &state->window);
}
