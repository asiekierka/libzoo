/**
 * Copyright (c) 2020 Adrian Siekierka
 *
 * Based on a reconstruction of code from ZZT,
 * Copyright 1991 Epic MegaGames, used with permission.
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

#ifndef __ZOO_H__
#define __ZOO_H__

#include <stdbool.h>
#include <stdint.h>
#include "zoo_config.h"


#ifdef ZOO_CONFIG_USE_DOUBLE_FOR_MS
#include <math.h>
#endif

// early defs

struct s_zoo_input_state;
struct s_zoo_state;
struct s_zoo_text_window;

typedef struct s_zoo_video_driver {
	// must implement
	void (*func_write)(struct s_zoo_video_driver *drv, int16_t x, int16_t y, uint8_t col, uint8_t chr);

	// optional to implement
	void (*func_read)(struct s_zoo_video_driver *drv, int16_t x, int16_t y, uint8_t *col, uint8_t *chr);
	void *(*func_store_display)(struct s_zoo_video_driver *drv, int16_t x, int16_t y, int16_t width, int16_t height);
	void (*func_restore_display)(struct s_zoo_video_driver *drv, void *data, int16_t width, int16_t height, int16_t srcx, int16_t srcy, int16_t srcwidth, int16_t srcheight, int16_t dstx, int16_t dsty);
} zoo_video_driver;

// PIT timing

#ifdef ZOO_CONFIG_USE_DOUBLE_FOR_MS
// 3.579545 MHz - NTSC dotclock
// dotclock * 4 = 14.31818
// 14.31818 / 12 ~= 1.19318166 - PIT frequency
// 65535 - maximum PIT cycle count before reload
// (65535 / 1193181.66) = SYS_TIMER_TIME (seconds)
typedef double zoo_time_ms;
#define ZOO_PIT_TICK_MS 54.92457840312107
#else
typedef uint32_t zoo_time_ms;
#define ZOO_PIT_TICK_MS 55
#endif /* ZOO_CONFIG_USE_DOUBLE_FOR_MS */

// call stack

typedef enum {
	// cycle in progress - return immediately
	RETURN_IMMEDIATE,
	// cycle in progress, awaiting i/o - return next frame
	RETURN_NEXT_FRAME,
	// cycle complete - return next cycle
	RETURN_NEXT_CYCLE,
	// exit the current mode
	EXIT
} zoo_tick_retval;

typedef void (*zoo_func_element_draw)(struct s_zoo_state *state, int16_t x, int16_t y, uint8_t *ch);
typedef void (*zoo_func_element_tick)(struct s_zoo_state *state, int16_t stat_id);
typedef void (*zoo_func_element_touch)(struct s_zoo_state *state, int16_t x, int16_t y, int16_t source_stat_id, int16_t *dx, int16_t *dy);
typedef zoo_tick_retval (*zoo_func_callback)(struct s_zoo_state *state, void *arg);

typedef enum {
	TICK_FUNC,
	TOUCH_FUNC,
	CALLBACK
} zoo_call_type;

typedef struct s_zoo_call {
	struct s_zoo_call *next;
	zoo_call_type type;
	uint8_t state;
	union {
		struct {
			zoo_func_element_tick func;
			int16_t stat_id;
		} tick;
		struct {
			zoo_func_element_touch func;
			int16_t x, y;
			int16_t source_stat_id;
			int16_t *dx, *dy;
			union {
				struct {
					int16_t neighbor_id;
					int16_t board_id;
					int16_t entry_x;
					int16_t entry_y;
				} board_edge;
			} extra;
		} touch;
		struct {
			zoo_func_callback func;
			void *arg;
		} cb;
	} args;
} zoo_call;

typedef struct {
	zoo_call *call;
 	uint8_t state;
	zoo_call *curr_call;
} zoo_call_stack;

#define zoo_call_empty(stack) ((stack)->call == NULL)
zoo_call *zoo_call_push(zoo_call_stack *stack, zoo_call_type type, uint8_t state);
zoo_call *zoo_call_push_callback(zoo_call_stack *stack, zoo_func_callback func, void *arg);
void zoo_call_pop(zoo_call_stack *stack);

// maths

#define zoo_sq(val) ((val)*(val))
#define zoo_dist_sq(a, b) (zoo_sq((a)) + zoo_sq((b)))
#define zoo_dist_sq2(a, b) (zoo_sq((a)) + 2*zoo_sq((b)))
#define zoo_signum(val) ( ((val) > 0) ? 1 : ( ((val) < 0) ? -1 : 0 ) )
#define zoo_difference(a, b) ( ((a) >= (b)) ? ((a) - (b)) : ((b) - (a)) )
#define zoo_toupper(c) (((c) >= 'a' && (c) <= 'z') ? ((c) - 0x20) : (c))

// sound

// zoo_sound.c

typedef struct s_zoo_sound_driver {
	void (*func_play_freqs)(struct s_zoo_sound_driver *drv, const uint16_t *freqs, uint16_t len, bool clear);
} zoo_sound_driver;

typedef struct s_zoo_sound_state {
	bool enabled;
	bool block_queueing;
	int16_t current_priority;
	uint8_t duration_multiplier;
	uint8_t duration_counter;
	uint8_t buffer[256];
	int16_t buffer_pos;
	int16_t buffer_len;
	bool is_playing;

	zoo_sound_driver *d_sound;
} zoo_sound_state;

// subtract 1 as those are usually strings and contain a leading \0
#define zoo_sound_queue_const(state, priority, data) zoo_sound_queue((state), (priority), (uint8_t*)(data), (sizeof((data))-1))

void zoo_sound_queue(zoo_sound_state *state, int16_t priority, const uint8_t *data, int16_t len);
void zoo_sound_clear_queue(zoo_sound_state *state);
void zoo_sound_tick(zoo_sound_state *state);
int16_t zoo_sound_parse(const char *input, uint8_t *output, int16_t out_max);
void zoo_sound_state_init(zoo_sound_state *state);

// zoo_io.c

typedef enum {
	MODE_READ,
	MODE_WRITE
} zoo_io_mode;

typedef struct s_zoo_io_handle {
	void *p;
	int len;
	int len_orig;
	uint8_t (*func_getc)(struct s_zoo_io_handle *h);
	size_t (*func_read)(struct s_zoo_io_handle *h, uint8_t *ptr, size_t len);
	size_t (*func_putc)(struct s_zoo_io_handle *h, uint8_t v);
	size_t (*func_write)(struct s_zoo_io_handle *h, const uint8_t *ptr, size_t len);
	size_t (*func_skip)(struct s_zoo_io_handle *h, size_t len);
	size_t (*func_tell)(struct s_zoo_io_handle *h);
	void (*func_close)(struct s_zoo_io_handle *h);
} zoo_io_handle;

typedef struct s_zoo_io_driver {
	bool read_only;

	// must implement
	zoo_io_handle (*func_open_file)(struct s_zoo_io_driver *drv, const char *filename, zoo_io_mode mode);
} zoo_io_driver;

#define zoo_io_open_file_empty() zoo_io_open_file_mem(NULL, 0, MODE_READ)
zoo_io_handle zoo_io_open_file_mem(uint8_t *ptr, size_t len, zoo_io_mode mode);

// game

#define ZOO_MAX_BOARD_LEN 20000

#define ZOO_BOARD_WIDTH 60
#define ZOO_BOARD_HEIGHT 25
#define ZOO_MAX_STAT 150
#define ZOO_MAX_BOARD 100
#define ZOO_MAX_ELEMENT 53
#define ZOO_MAX_FLAG 10
#define ZOO_LEN_FLAG 20
#define ZOO_LEN_MESSAGE 58
#define ZOO_MAX_KEY_BUFFER 31

#define ZOO_TORCH_DURATION 200
#define ZOO_TORCH_DX 8
#define ZOO_TORCH_DY 5
#define ZOO_TORCH_DSQ 50

typedef struct {
	uint8_t element;
	uint8_t color;
} zoo_tile;

typedef struct {
	uint8_t x, y;
	int16_t step_x, step_y;
	int16_t cycle;
	uint8_t p1, p2, p3;
	int16_t follower, leader;
	zoo_tile under;
	char *data;
	int16_t data_pos;
	int16_t data_len;
} zoo_stat;

typedef struct {
	uint8_t count;
	zoo_tile tile;
} zoo_rle_tile;

typedef struct {
	uint8_t max_shots;
	bool is_dark;
	uint8_t neighbor_boards[4];
	bool reenter_when_zapped;
	char message[ZOO_LEN_MESSAGE + 1];
	uint8_t start_player_x, start_player_y;
	int16_t time_limit_sec;
} zoo_board_info;

typedef struct {
	int16_t ammo;
	int16_t gems;
	bool keys[7];
	int16_t health;
	int16_t current_board;
	int16_t torches;
	int16_t torch_ticks;
	int16_t energizer_ticks;
	int16_t score;
	char name[21];
	char flags[ZOO_MAX_FLAG][ZOO_LEN_FLAG + 1];
	int16_t board_time_sec;
	int16_t board_time_hsec;
	bool is_save;
} zoo_world_info;

typedef struct {
	char name[51];
	zoo_tile tiles[ZOO_BOARD_WIDTH + 2][ZOO_BOARD_HEIGHT + 2];
	int16_t stat_count;
	zoo_stat stats[ZOO_MAX_STAT + 2];
	zoo_board_info info;
} zoo_board;

typedef struct {
	int16_t board_count;
	void *board_data[ZOO_MAX_BOARD + 2];
	int16_t board_len[ZOO_MAX_BOARD + 2];
	zoo_world_info info;
} zoo_world;

typedef struct {
	bool ammo;
	bool out_of_ammo;
	bool no_shooting;
	bool torch;
	bool out_of_torches;
	bool room_not_dark;
	bool hint_torch;
	bool forest;
	bool fake;
	bool gem;
	bool energizer;
} zoo_state_message;

typedef enum {
	// universal - movement
	ZOO_ACTION_UP,
	ZOO_ACTION_LEFT,
	ZOO_ACTION_RIGHT,
	ZOO_ACTION_DOWN,

	// universal - windows/popups
	ZOO_ACTION_OK,
	ZOO_ACTION_CANCEL,

	// gameplay
	ZOO_ACTION_SHOOT,
	ZOO_ACTION_TORCH,

	ZOO_ACTION_MAX
} zoo_input_action;

typedef struct s_zoo_input_state {
	int16_t delta_x;
	int16_t delta_y;
	bool updated;

	bool actions_down[ZOO_ACTION_MAX];
	bool actions_held[ZOO_ACTION_MAX];
} zoo_input_state;

typedef struct s_zoo_text_window {
	int16_t line_count;
	int16_t line_pos;
	char **lines;
	char hyperlink[21];
	char title[51];
	void *screen_copy;

	bool accepted;
	bool hyperlink_as_select;
	bool viewing_file;
	bool manual_close;
	bool disable_transitions;

	uint8_t state;
	int8_t counter;
} zoo_text_window;

typedef enum {
	GS_NONE,
	GS_TITLE,
	GS_PLAY
} zoo_game_state;

typedef struct s_zoo_state {
	zoo_board board;
	zoo_world world;

	zoo_state_message msg_flags;

	bool game_title_exit_requested;
	bool game_play_exit_requested;
	int16_t return_board_id;

	zoo_game_state game_state;

	int16_t current_tick;
	int16_t current_stat_tick;
	int16_t tick_counter;
	int16_t tick_speed;
	int16_t tick_duration;
	zoo_time_ms time_elapsed;
	bool game_paused;
	bool game_paused_blink;
	bool force_darkness_off;

	char oop_char;
	char oop_word[21];
	int16_t oop_value;

	zoo_input_state input;
	zoo_sound_state sound;

	bool object_replace_stat;
	zoo_tile object_replace_tile;
	zoo_text_window object_window;
	bool object_window_request;
	zoo_call_stack call_stack;
	uint8_t game_tick_state; // not in call_stack to save performance

	uint32_t random_seed;
	// TODO: does this need to be overrideable?
	int16_t (*func_random)(struct s_zoo_state *state, int16_t max);

	zoo_video_driver *d_video;
	zoo_io_driver *d_io;

	// - high-level engine hooks (optional, have default implementations)
	void (*func_draw_sidebar)(struct s_zoo_state *state, uint16_t flags);
	void (*func_write_message)(struct s_zoo_state *state, uint8_t p2, const char *message);
} zoo_state;

typedef struct {
	uint8_t character;
	uint8_t color;
	bool destructible;
	bool pushable;
	bool visible_in_dark;
	bool placeable_on_top;
	bool walkable;
	bool has_draw_func;
	zoo_func_element_draw draw_func;
	int16_t cycle;
	zoo_func_element_tick tick_func;
	zoo_func_element_touch touch_func;
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
	int16_t editor_category;
	char editor_shortcut;
#endif
	char name[21];
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
	char category_name[21];
	char param1_name[21];
	char param2_name[21];
	char param_bullet_type_name[21];
	char param_board_name[21];
	char param_dir_name[21];
	char param_text_name[21];
#endif
	int16_t score_value;
// NOTE: libzoo additions follow
	bool has_text;
	char oop_strip_name[21];
} zoo_element_def;

// zoo.c

void zoo_state_init(zoo_state *state);
void zoo_redraw(zoo_state *state);
bool zoo_world_reload(zoo_state *state);
bool zoo_world_play(zoo_state *state);
bool zoo_world_return_title(zoo_state *state);

void* zoo_store_display(zoo_state *state, int16_t x, int16_t y, int16_t width, int16_t height);
void zoo_restore_display(zoo_state *state, void *data, int16_t width, int16_t height, int16_t srcx, int16_t srcy, int16_t srcwidth, int16_t srcheight, int16_t dstx, int16_t dsty);

bool zoo_check_hsecs_elapsed(zoo_state *state, int16_t *hsecs_counter, int16_t hsecs_value);
bool zoo_has_hsecs_elapsed(zoo_state *state, int16_t *hsecs_counter, int16_t hsecs_value);
zoo_time_ms zoo_hsecs_to_pit_ms(int16_t hsecs);
int16_t zoo_hsecs_to_pit_ticks(int16_t hsecs);

// zoo_element.c

extern const zoo_element_def zoo_element_defs[ZOO_MAX_ELEMENT + 1];

void zoo_element_move(zoo_state *state, int16_t old_x, int16_t old_y, int16_t new_x, int16_t new_y);
void zoo_element_push(zoo_state *state, int16_t x, int16_t y, int16_t dx, int16_t dy);

void zoo_draw_player_surroundings(zoo_state *state, int16_t x, int16_t y, int16_t bomb_phase);

void zoo_reset_message_flags(zoo_state *state);

// zoo_game.c

extern const char zoo_line_chars[16];
extern const char zoo_color_names[8][8];
extern const zoo_stat zoo_stat_template_default;

extern const int16_t zoo_diagonal_delta_x[8];
extern const int16_t zoo_diagonal_delta_y[8];
extern const int16_t zoo_neighbor_delta_x[4];
extern const int16_t zoo_neighbor_delta_y[4];

void zoo_board_change(zoo_state *state, int16_t board_id);
void zoo_board_create(zoo_state *state);
void zoo_world_create(zoo_state *state);

void zoo_board_draw_tile(zoo_state *state, int16_t x, int16_t y);
void zoo_board_draw_border(zoo_state *state);
void zoo_board_draw(zoo_state *state);

void zoo_stat_add(zoo_state *state, int16_t tx, int16_t ty, uint8_t element, int16_t color, int16_t tcycle, const zoo_stat *stat_template);
void zoo_stat_remove(zoo_state *state, int16_t stat_id);
int16_t zoo_stat_get_id(zoo_state *state, int16_t x, int16_t y);
zoo_stat *zoo_stat_get(zoo_state *state, int16_t x, int16_t y, int16_t *stat_id);
void zoo_stat_move(zoo_state *state, int16_t stat_id, int16_t new_x, int16_t new_y);

void zoo_board_damage_stat(zoo_state *state, int16_t stat_id);
void zoo_board_damage_tile(zoo_state *state, int16_t x, int16_t y);
void zoo_board_attack_tile(zoo_state *state, int16_t attacker_stat_id, int16_t x, int16_t y);
bool zoo_board_shoot(zoo_state *state, uint8_t element, int16_t x, int16_t y, int16_t dx, int16_t dy, int16_t param1);

void zoo_display_message(zoo_state *state, int16_t duration, char *message);

void zoo_calc_direction_rnd(zoo_state *state, int16_t *dx, int16_t *dy);
void zoo_calc_direction_seek(zoo_state *state, int16_t x, int16_t y, int16_t *dx, int16_t *dy);

void zoo_board_enter(zoo_state *state);
void zoo_board_passage_teleport(zoo_state *state, int16_t x, int16_t y);

void zoo_game_start(zoo_state *state, zoo_game_state game_state);
void zoo_game_stop(zoo_state *state);

void zoo_tick_advance_pit(zoo_state *state);
zoo_tick_retval zoo_tick(zoo_state *state);

// zoo_game_io.c

void zoo_board_close(zoo_state *state);
void zoo_board_open(zoo_state *state, int16_t board_id);

void zoo_world_close(zoo_state *state);
bool zoo_world_load(zoo_state *state, zoo_io_handle *h, bool title_only);
bool zoo_world_save(zoo_state *state, zoo_io_handle *h);

// zoo_input.c

bool zoo_input_action_pressed(zoo_input_state *state, zoo_input_action action);
bool zoo_input_action_pressed_once(zoo_input_state *state, zoo_input_action action);
void zoo_input_update(zoo_input_state *state);
void zoo_input_clear_post_tick(zoo_input_state *state);

void zoo_input_action_down(zoo_input_state *state, zoo_input_action action);
void zoo_input_action_once(zoo_input_state *state, zoo_input_action action);
void zoo_input_action_up(zoo_input_state *state, zoo_input_action action);
void zoo_input_action_set(zoo_input_state *state, zoo_input_action action, bool value);

// zoo_oop.c

int16_t zoo_flag_get_id(zoo_state *state, const char *name);
void zoo_flag_set(zoo_state *state, const char *name);
void zoo_flag_clear(zoo_state *state, const char *name);

bool zoo_oop_send(zoo_state *state, int16_t stat_id, const char *send_label, bool ignore_lock);
void zoo_oop_execute(zoo_state *state, int16_t stat_id, int16_t *position, const char *default_name);

// zoo_window.c

char *zoo_window_line_at(zoo_text_window *window, int pos);
char *zoo_window_line_selected(zoo_text_window *window);
void zoo_window_append(zoo_text_window *window, const char *text);
void zoo_window_close(zoo_text_window *window);

void zoo_window_append_file(zoo_text_window *window, zoo_io_handle *h);
bool zoo_window_open_file(zoo_io_driver *io, zoo_text_window *window, const char *filename);

// zoo_window_classic.c

void zoo_window_open(zoo_state *state, zoo_text_window *window);
void zoo_window_set_position(int16_t x, int16_t y, int16_t width, int16_t height);

// zoo_ui_*

#define ZOO_SIDEBAR_UPDATE_TIME 0x0001
#define ZOO_SIDEBAR_UPDATE_HEALTH 0x0002
#define ZOO_SIDEBAR_UPDATE_AMMO 0x0004
#define ZOO_SIDEBAR_UPDATE_TORCHES 0x0008 /* + torch ticks */
#define ZOO_SIDEBAR_UPDATE_GEMS 0x0010
#define ZOO_SIDEBAR_UPDATE_SCORE 0x0020
#define ZOO_SIDEBAR_UPDATE_KEYS 0x0040

#define ZOO_SIDEBAR_UPDATE_PAUSED 0x0080

#define ZOO_SIDEBAR_UPDATE_OTHER 0x4000
#define ZOO_SIDEBAR_UPDATE_ALL 0x7FFF
#define ZOO_SIDEBAR_UPDATE_REDRAW 0x8000
#define ZOO_SIDEBAR_UPDATE_ALL_REDRAW 0xFFFF

// defines

#define ZOO_E_EMPTY 0
#define ZOO_E_BOARD_EDGE 1
#define ZOO_E_MESSAGE_TIMER 2
#define ZOO_E_MONITOR 3 // state - Title screen
#define ZOO_E_PLAYER 4 // state - Playing
#define ZOO_E_AMMO 5
#define ZOO_E_TORCH 6
#define ZOO_E_GEM 7
#define ZOO_E_KEY 8
#define ZOO_E_DOOR 9
#define ZOO_E_SCROLL 10
#define ZOO_E_PASSAGE 11
#define ZOO_E_DUPLICATOR 12
#define ZOO_E_BOMB 13
#define ZOO_E_ENERGIZER 14
#define ZOO_E_STAR 15
#define ZOO_E_CONVEYOR_CW 16
#define ZOO_E_CONVEYOR_CCW 17
#define ZOO_E_BULLET 18
#define ZOO_E_WATER 19
#define ZOO_E_FOREST 20
#define ZOO_E_SOLID 21
#define ZOO_E_NORMAL 22
#define ZOO_E_BREAKABLE 23
#define ZOO_E_BOULDER 24
#define ZOO_E_SLIDER_NS 25
#define ZOO_E_SLIDER_EW 26
#define ZOO_E_FAKE 27
#define ZOO_E_INVISIBLE 28
#define ZOO_E_BLINK_WALL 29
#define ZOO_E_TRANSPORTER 30
#define ZOO_E_LINE 31
#define ZOO_E_RICOCHET 32
#define ZOO_E_BLINK_RAY_EW 33
#define ZOO_E_BEAR 34
#define ZOO_E_RUFFIAN 35
#define ZOO_E_OBJECT 36
#define ZOO_E_SLIME 37
#define ZOO_E_SHARK 38
#define ZOO_E_SPINNING_GUN 39
#define ZOO_E_PUSHER 40
#define ZOO_E_LION 41
#define ZOO_E_TIGER 42
#define ZOO_E_BLINK_RAY_NS 43
#define ZOO_E_CENTIPEDE_HEAD 44
#define ZOO_E_CENTIPEDE_SEGMENT 45
#define ZOO_E_TEXT_BLUE 47
#define ZOO_E_TEXT_GREEN 48
#define ZOO_E_TEXT_CYAN 49
#define ZOO_E_TEXT_RED 50
#define ZOO_E_TEXT_PURPLE 51
#define ZOO_E_TEXT_YELLOW 52
#define ZOO_E_TEXT_WHITE 53
#define ZOO_E_TEXT_MIN ZOO_E_TEXT_BLUE

#define ZOO_CATEGORY_ITEM 1
#define ZOO_CATEGORY_CREATURE 2
#define ZOO_CATEGORY_TERRAIN 3
#define ZOO_COLOR_SPECIAL_MIN 0xF0
#define ZOO_COLOR_CHOICE_ON_BLACK 0xFF
#define ZOO_COLOR_WHITE_ON_CHOICE 0xFE
#define ZOO_COLOR_CHOICE_ON_CHOICE 0xFD

#endif /* __ZOO_H__ */
