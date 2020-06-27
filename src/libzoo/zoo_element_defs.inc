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

// Auto-generated file. Please do not edit directly.

const zoo_element_def zoo_element_defs[ZOO_MAX_ELEMENT + 1] = {
	{ // 0 - Empty
		' ', // character
		0x70, // color
		false, // destructible
		true, // pushable
		false, // visible in dark
		false, // placeable on top
		true, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		0, // editor category
		'\0', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Empty", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"EMPTY", // stripped name (libzoo)
	},
	{ // 1
		' ', // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_e_board_edge_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		0, // editor category
		'\0', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"", // stripped name (libzoo)
	},
	{ // 2
		' ', // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_e_message_timer_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		0, // editor category
		'\0', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"", // stripped name (libzoo)
	},
	{ // 3 - Monitor
		' ', // character
		0x07, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		1, // cycle
		zoo_e_monitor_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		0, // editor category
		'\0', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Monitor", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"MONITOR", // stripped name (libzoo)
	},
	{ // 4 - Player
		0x02, // character
		0x1F, // color
		true, // destructible
		true, // pushable
		true, // visible in dark
		false, // placeable on top
		false, // walkable
		true, // has draw func
		zoo_e_player_draw, // draw func
		1, // cycle
		zoo_e_player_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_ITEM, // editor category
		'Z', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Player", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"Items:", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"PLAYER", // stripped name (libzoo)
	},
	{ // 5 - Ammo
		0x84, // character
		0x03, // color
		false, // destructible
		true, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_e_ammo_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_ITEM, // editor category
		'A', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Ammo", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"AMMO", // stripped name (libzoo)
	},
	{ // 6 - Torch
		0x9D, // character
		0x06, // color
		false, // destructible
		false, // pushable
		true, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_e_torch_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_ITEM, // editor category
		'T', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Torch", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"TORCH", // stripped name (libzoo)
	},
	{ // 7 - Gem
		0x04, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		true, // destructible
		true, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_e_gem_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_ITEM, // editor category
		'G', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Gem", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"GEM", // stripped name (libzoo)
	},
	{ // 8 - Key
		0x0C, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		true, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_e_key_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_ITEM, // editor category
		'K', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Key", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"KEY", // stripped name (libzoo)
	},
	{ // 9 - Door
		0x0A, // character
		ZOO_COLOR_WHITE_ON_CHOICE, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_e_door_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_ITEM, // editor category
		'D', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Door", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"DOOR", // stripped name (libzoo)
	},
	{ // 10 - Scroll
		0xE8, // character
		0x0F, // color
		false, // destructible
		true, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		1, // cycle
		zoo_e_scroll_tick, // tick func
		zoo_e_scroll_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_ITEM, // editor category
		'S', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Scroll", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"Edit text of scroll", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		true, // has text (libzoo)
		"SCROLL", // stripped name (libzoo)
	},
	{ // 11 - Passage
		0xF0, // character
		ZOO_COLOR_WHITE_ON_CHOICE, // color
		false, // destructible
		false, // pushable
		true, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		0, // cycle
		zoo_default_tick, // tick func
		zoo_e_passage_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_ITEM, // editor category
		'P', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Passage", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"Room thru passage?", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"PASSAGE", // stripped name (libzoo)
	},
	{ // 12 - Duplicator
		0xFA, // character
		0x0F, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		true, // has draw func
		zoo_e_duplicator_draw, // draw func
		2, // cycle
		zoo_e_duplicator_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_ITEM, // editor category
		'U', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Duplicator", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"Duplication rate?;SF", // param 2 name
		"", // param bullet type name
		"", // param board name
		"Source direction?", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"DUPLICATOR", // stripped name (libzoo)
	},
	{ // 13 - Bomb
		0x0B, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		true, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		true, // has draw func
		zoo_e_bomb_draw, // draw func
		6, // cycle
		zoo_e_bomb_tick, // tick func
		zoo_e_bomb_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_ITEM, // editor category
		'B', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Bomb", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"BOMB", // stripped name (libzoo)
	},
	{ // 14 - Energizer
		0x7F, // character
		0x05, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_e_energizer_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_ITEM, // editor category
		'E', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Energizer", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"ENERGIZER", // stripped name (libzoo)
	},
	{ // 15 - Star
		'S', // character
		0x0F, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		true, // has draw func
		zoo_e_star_draw, // draw func
		1, // cycle
		zoo_e_star_tick, // tick func
		zoo_e_damaging_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		0, // editor category
		'\0', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Star", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"STAR", // stripped name (libzoo)
	},
	{ // 16 - Clockwise
		'/', // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		true, // has draw func
		zoo_e_conveyor_cw_draw, // draw func
		3, // cycle
		zoo_e_conveyor_cw_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_ITEM, // editor category
		'1', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Clockwise", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"Conveyors:", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"CLOCKWISE", // stripped name (libzoo)
	},
	{ // 17 - Counter
		0x5C, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		true, // has draw func
		zoo_e_conveyor_ccw_draw, // draw func
		2, // cycle
		zoo_e_conveyor_ccw_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_ITEM, // editor category
		'2', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Counter", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"COUNTER", // stripped name (libzoo)
	},
	{ // 18 - Bullet
		0xF8, // character
		0x0F, // color
		true, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		1, // cycle
		zoo_e_bullet_tick, // tick func
		zoo_e_damaging_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		0, // editor category
		'\0', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Bullet", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"BULLET", // stripped name (libzoo)
	},
	{ // 19 - Water
		0xB0, // character
		0xF9, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		true, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_e_water_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_TERRAIN, // editor category
		'W', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Water", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"Terrains:", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"WATER", // stripped name (libzoo)
	},
	{ // 20 - Forest
		0xB0, // character
		0x20, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_e_forest_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_TERRAIN, // editor category
		'F', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Forest", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"FOREST", // stripped name (libzoo)
	},
	{ // 21 - Solid
		0xDB, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_TERRAIN, // editor category
		'S', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Solid", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"Walls:", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"SOLID", // stripped name (libzoo)
	},
	{ // 22 - Normal
		0xB2, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_TERRAIN, // editor category
		'N', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Normal", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"NORMAL", // stripped name (libzoo)
	},
	{ // 23 - Breakable
		0xB1, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_TERRAIN, // editor category
		'B', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Breakable", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"BREAKABLE", // stripped name (libzoo)
	},
	{ // 24 - Boulder
		0xFE, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		true, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_e_pushable_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_TERRAIN, // editor category
		'O', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Boulder", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"BOULDER", // stripped name (libzoo)
	},
	{ // 25 - Slider (NS)
		0x12, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_e_pushable_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_TERRAIN, // editor category
		'1', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Slider (NS)", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"SLIDERNS", // stripped name (libzoo)
	},
	{ // 26 - Slider (EW)
		0x1D, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_e_pushable_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_TERRAIN, // editor category
		'2', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Slider (EW)", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"SLIDEREW", // stripped name (libzoo)
	},
	{ // 27 - Fake
		0xB2, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		true, // placeable on top
		true, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_e_fake_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_TERRAIN, // editor category
		'A', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Fake", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"FAKE", // stripped name (libzoo)
	},
	{ // 28 - Invisible
		' ', // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_e_invisible_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_TERRAIN, // editor category
		'I', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Invisible", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"INVISIBLE", // stripped name (libzoo)
	},
	{ // 29 - Blink wall
		0xCE, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		true, // has draw func
		zoo_e_blink_wall_draw, // draw func
		1, // cycle
		zoo_e_blink_wall_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_TERRAIN, // editor category
		'L', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Blink wall", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"Starting time", // param 1 name
		"Period", // param 2 name
		"", // param bullet type name
		"", // param board name
		"Wall direction", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"BLINKWALL", // stripped name (libzoo)
	},
	{ // 30 - Transporter
		0xC5, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		true, // has draw func
		zoo_e_transporter_draw, // draw func
		2, // cycle
		zoo_e_transporter_tick, // tick func
		zoo_e_transporter_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_TERRAIN, // editor category
		'T', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Transporter", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"Direction?", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"TRANSPORTER", // stripped name (libzoo)
	},
	{ // 31 - Line
		0xCE, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		true, // has draw func
		zoo_e_line_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		0, // editor category
		'\0', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Line", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"LINE", // stripped name (libzoo)
	},
	{ // 32 - Ricochet
		'*', // character
		0x0A, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_TERRAIN, // editor category
		'R', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Ricochet", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"RICOCHET", // stripped name (libzoo)
	},
	{ // 33
		0xCD, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		0, // editor category
		'\0', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"", // stripped name (libzoo)
	},
	{ // 34 - Bear
		0x99, // character
		0x06, // color
		true, // destructible
		true, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		3, // cycle
		zoo_e_bear_tick, // tick func
		zoo_e_damaging_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_CREATURE, // editor category
		'B', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Bear", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"Creatures:", // category name
		"Sensitivity?", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		1, // score value
		false, // has text (libzoo)
		"BEAR", // stripped name (libzoo)
	},
	{ // 35 - Ruffian
		0x05, // character
		0x0D, // color
		true, // destructible
		true, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		1, // cycle
		zoo_e_ruffian_tick, // tick func
		zoo_e_damaging_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_CREATURE, // editor category
		'R', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Ruffian", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"Intelligence?", // param 1 name
		"Resting time?", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		2, // score value
		false, // has text (libzoo)
		"RUFFIAN", // stripped name (libzoo)
	},
	{ // 36 - Object
		0x02, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		true, // has draw func
		zoo_e_object_draw, // draw func
		3, // cycle
		zoo_e_object_tick, // tick func
		zoo_e_object_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_CREATURE, // editor category
		'O', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Object", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"Character?", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"Edit Program", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		true, // has text (libzoo)
		"OBJECT", // stripped name (libzoo)
	},
	{ // 37 - Slime
		'*', // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		3, // cycle
		zoo_e_slime_tick, // tick func
		zoo_e_slime_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_CREATURE, // editor category
		'V', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Slime", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"Movement speed?;FS", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"SLIME", // stripped name (libzoo)
	},
	{ // 38 - Shark
		'^', // character
		0x07, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		3, // cycle
		zoo_e_shark_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_CREATURE, // editor category
		'Y', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Shark", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"Intelligence?", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"SHARK", // stripped name (libzoo)
	},
	{ // 39 - Spinning gun
		0x18, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		true, // has draw func
		zoo_e_spinning_gun_draw, // draw func
		2, // cycle
		zoo_e_spinning_gun_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_CREATURE, // editor category
		'G', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Spinning gun", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"Intelligence?", // param 1 name
		"Firing rate?", // param 2 name
		"Firing type?", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"SPINNINGGUN", // stripped name (libzoo)
	},
	{ // 40 - Pusher
		0x10, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		true, // has draw func
		zoo_e_pusher_draw, // draw func
		4, // cycle
		zoo_e_pusher_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_CREATURE, // editor category
		'P', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Pusher", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"Push direction?", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"PUSHER", // stripped name (libzoo)
	},
	{ // 41 - Lion
		0xEA, // character
		0x0C, // color
		true, // destructible
		true, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		2, // cycle
		zoo_e_lion_tick, // tick func
		zoo_e_damaging_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_CREATURE, // editor category
		'L', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Lion", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"Beasts:", // category name
		"Intelligence?", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		1, // score value
		false, // has text (libzoo)
		"LION", // stripped name (libzoo)
	},
	{ // 42 - Tiger
		0xE3, // character
		0x0B, // color
		true, // destructible
		true, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		2, // cycle
		zoo_e_tiger_tick, // tick func
		zoo_e_damaging_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_CREATURE, // editor category
		'T', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Tiger", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"Intelligence?", // param 1 name
		"Firing rate?", // param 2 name
		"Firing type?", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		2, // score value
		false, // has text (libzoo)
		"TIGER", // stripped name (libzoo)
	},
	{ // 43
		0xBA, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		0, // editor category
		'\0', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"", // stripped name (libzoo)
	},
	{ // 44 - Head
		0xE9, // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		true, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		2, // cycle
		zoo_e_centipede_head_tick, // tick func
		zoo_e_damaging_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_CREATURE, // editor category
		'H', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Head", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"Centipedes", // category name
		"Intelligence?", // param 1 name
		"Deviance?", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		1, // score value
		false, // has text (libzoo)
		"HEAD", // stripped name (libzoo)
	},
	{ // 45 - Segment
		'O', // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		true, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		2, // cycle
		zoo_e_centipede_segment_tick, // tick func
		zoo_e_damaging_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		ZOO_CATEGORY_CREATURE, // editor category
		'S', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"Segment", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		3, // score value
		false, // has text (libzoo)
		"SEGMENT", // stripped name (libzoo)
	},
	{ // 46
		' ', // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		0, // editor category
		'\0', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"", // stripped name (libzoo)
	},
	{ // 47
		' ', // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		0, // editor category
		'\0', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"", // stripped name (libzoo)
	},
	{ // 48
		' ', // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		0, // editor category
		'\0', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"", // stripped name (libzoo)
	},
	{ // 49
		' ', // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		0, // editor category
		'\0', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"", // stripped name (libzoo)
	},
	{ // 50
		' ', // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		0, // editor category
		'\0', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"", // stripped name (libzoo)
	},
	{ // 51
		' ', // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		0, // editor category
		'\0', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"", // stripped name (libzoo)
	},
	{ // 52
		' ', // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		0, // editor category
		'\0', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"", // stripped name (libzoo)
	},
	{ // 53
		' ', // character
		ZOO_COLOR_CHOICE_ON_BLACK, // color
		false, // destructible
		false, // pushable
		false, // visible in dark
		false, // placeable on top
		false, // walkable
		false, // has draw func
		zoo_default_draw, // draw func
		-1, // cycle
		zoo_default_tick, // tick func
		zoo_default_touch, // touch func
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		0, // editor category
		'\0', // editor shortcut
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		"", // name
#ifdef ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS
		"", // category name
		"", // param 1 name
		"", // param 2 name
		"", // param bullet type name
		"", // param board name
		"", // param dir name
		"", // param text name
#endif /* ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS */
		0, // score value
		false, // has text (libzoo)
		"", // stripped name (libzoo)
	}
};
