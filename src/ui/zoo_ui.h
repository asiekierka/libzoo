#ifndef __ZOO_UI_H__
#define __ZOO_UI_H__

#include "../libzoo/zoo.h"
#include "../drivers/zoo_io_path.h"

struct s_zoo_ui_state;

// UI primitives

void zoo_ui_filesel_call(struct s_zoo_ui_state *state, const char *title, const char *extension, zoo_func_callback cb);

// game operations

void zoo_ui_load_world(struct s_zoo_ui_state *state, bool as_save);
void zoo_ui_main_menu(struct s_zoo_ui_state *state);

// internal

void zoo_ui_init_select_window(struct s_zoo_ui_state *state, const char *title);

#ifdef ZOO_DEBUG_MENU
void zoo_ui_debug_printf(bool status, const char *format, ...);
void zoo_ui_debug_menu(struct s_zoo_ui_state *state);
#endif

// state definitions

typedef struct s_zoo_ui_state {
    zoo_state *zoo;
	zoo_text_window window;
	char filesel_extension[5];
} zoo_ui_state;

void zoo_ui_init(struct s_zoo_ui_state *state, zoo_state *zoo);

// inputs

#define ZOO_KEY_BACKSPACE 8
#define ZOO_KEY_TAB 9
#define ZOO_KEY_ENTER 13
#define ZOO_KEY_CTRL_Y 25
#define ZOO_KEY_ESCAPE 27
#define ZOO_KEY_ALT_P 153
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

#endif /* __ZOO_UI_H__ */