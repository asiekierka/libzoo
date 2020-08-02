#ifndef __ZOO_UI_H__
#define __ZOO_UI_H__

#include "zoo.h"
#include "zoo_io_path.h"
#include "zoo_ui_input.h"

#ifdef ZOO_UI_OSK
#define ZOO_UI_CHEAT_HISTORY
#endif

struct s_zoo_ui_state;

// game operations

void zoo_ui_load_world(struct s_zoo_ui_state *state, bool as_save);
void zoo_ui_main_menu(struct s_zoo_ui_state *state);

// state definitions

#define ZOO_UI_CHEAT_HISTORY_SIZE 8

typedef struct s_zoo_ui_state {
    zoo_state *zoo;
	zoo_ui_input_state input;

	zoo_text_window window;
	char filesel_extension[5];

#ifdef ZOO_UI_CHEAT_HISTORY
	char *cheat_history[ZOO_UI_CHEAT_HISTORY_SIZE];
#endif
} zoo_ui_state;

void zoo_ui_init(zoo_ui_state *state, zoo_state *zoo);
void zoo_ui_tick(zoo_ui_state *state);

#endif /* __ZOO_UI_H__ */