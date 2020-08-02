#ifndef __ZOO_UI_INTERNAL_H__
#define __ZOO_UI_INTERNAL_H__

#include "zoo_ui.h"

struct s_zoo_ui_state;

// zoo_ui_file_select.c

void zoo_ui_filesel_call(struct s_zoo_ui_state *state, const char *title, const char *extension, zoo_func_callback cb);

// zoo_ui_osk.c

#ifdef ZOO_UI_OSK
typedef struct {
	int8_t x;
	int8_t y;
	uint8_t normal; // if < 32 || >= 128, shifted has visual char; if 0, done
	uint8_t shifted;
} zoo_osk_entry;

typedef struct {
	const zoo_osk_entry *entries;

	uint8_t x;
	uint8_t y;
	int16_t e_pos;

	bool active;
	bool shifted;
	void *screen_copy;
} zoo_osk_state;

void zoo_osk_open(zoo_ui_state *ui, zoo_osk_state *osk, uint8_t x, uint8_t y);
void zoo_osk_close(zoo_ui_state *ui, zoo_osk_state *osk);
void zoo_osk_tick(zoo_ui_state *ui, zoo_osk_state *osk);
#endif

// zoo_ui_prompt_string.c

typedef enum {
    ZOO_UI_PROMPT_NUMERIC,
    ZOO_UI_PROMPT_ALPHANUMERIC,
    ZOO_UI_PROMPT_ANY
} zoo_ui_prompt_mode;

typedef zoo_tick_retval (*zoo_ui_prompt_string_callback)(struct s_zoo_ui_state *state, const char *buffer, bool accepted);

void zoo_ui_popup_prompt_string(zoo_ui_state *state, zoo_ui_prompt_mode mode, uint16_t x, uint16_t y, uint16_t width,
    const char *question, const char *answer, zoo_ui_prompt_string_callback cb);

#define ZOO_UI_PROMPT_WIDTH 50

typedef struct {
    struct s_zoo_ui_state *ui;

    uint16_t x, y, width;
    uint8_t arrow_color, text_color;
    zoo_ui_prompt_mode mode;
    char orig_buffer[ZOO_UI_PROMPT_WIDTH + 1];
    char buffer[ZOO_UI_PROMPT_WIDTH + 1];

    bool first_key_press;
    zoo_ui_prompt_string_callback callback;

#ifdef ZOO_UI_OSK
    zoo_osk_state osk;
#endif

    void *screen_copy;
} zoo_ui_prompt_state;

// zoo_ui_util.c

void zoo_ui_init_select_window(struct s_zoo_ui_state *state, const char *title);

// zoo_ui_debug.c

#ifdef ZOO_DEBUG_MENU
void zoo_ui_debug_printf(bool status, const char *format, ...);
void zoo_ui_debug_menu(struct s_zoo_ui_state *state);
#endif

#endif /* __ZOO_UI_H__ */