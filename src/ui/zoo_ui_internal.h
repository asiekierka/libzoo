#ifndef __ZOO_UI_INTERNAL_H__
#define __ZOO_UI_INTERNAL_H__

#include "zoo_ui.h"

struct s_zoo_ui_state;

void zoo_ui_filesel_call(struct s_zoo_ui_state *state, const char *title, const char *extension, zoo_func_callback cb);

void zoo_ui_init_select_window(struct s_zoo_ui_state *state, const char *title);

// zoo_ui_debug.c

#ifdef ZOO_DEBUG_MENU
void zoo_ui_debug_printf(bool status, const char *format, ...);
void zoo_ui_debug_menu(struct s_zoo_ui_state *state);
#endif

#endif /* __ZOO_UI_H__ */