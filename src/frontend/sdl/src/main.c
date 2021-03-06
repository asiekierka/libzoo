/**
 * Copyright (c) 2018, 2019, 2020 Adrian Siekierka
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>

#include "zoo.h"
#include "zoo_io_posix.h"
#include "zoo_sidebar.h"
#include "zoo_sound_pcm.h"
#include "zoo_ui.h"
#include "types.h"
#include "render_software.h"

static const uint32_t default_palette[] = {
	0xff000000, 0xff0000aa, 0xff00aa00, 0xff00aaaa,
	0xffaa0000, 0xffaa00aa, 0xffaa5500, 0xffaaaaaa,
	0xff555555, 0xff5555ff, 0xff55ff55, 0xff55ffff,
	0xffff5555, 0xffff55ff, 0xffffff55, 0xffffffff
};
extern uint8_t res_8x14_bin[];

static zoo_state state;
static zoo_ui_state ui_state;
static zoo_io_path_driver io_driver;
static zoo_sound_pcm_driver pcm_driver;

static video_buffer video;
static render_options render_opts;
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *playfield;
static void *playfield_buffer;
static int playfield_pitch;
static SDL_mutex *playfield_mutex;
static bool playfield_changed;
static bool stop_tick_thread = false;
static SDL_TimerID tick_thread_game;
static SDL_TimerID tick_thread_pit;

#ifdef __BIG_ENDIAN__
#define SOFT_PIXEL_FORMAT SDL_PIXELFORMAT_ARGB32
#else
#define SOFT_PIXEL_FORMAT SDL_PIXELFORMAT_BGRA32
#endif

// audio logic

static SDL_AudioDeviceID audio_device;
static SDL_AudioSpec audio_spec;
static SDL_mutex *audio_mutex;

static void sdl_audio_callback(void *userdata, uint8_t *stream, int len) {
	SDL_LockMutex(audio_mutex);
	zoo_sound_pcm_generate(&pcm_driver, stream, len);
	SDL_UnlockMutex(audio_mutex);
}

static void init_audio(void) {
	SDL_AudioSpec requested_audio_spec;

	SDL_zero(requested_audio_spec);
	requested_audio_spec.freq = 48000;
	requested_audio_spec.format = AUDIO_S8;
	requested_audio_spec.channels = 1;
	requested_audio_spec.samples = 4096;
	requested_audio_spec.callback = sdl_audio_callback;

	audio_mutex = SDL_CreateMutex();

	// TODO: add SDL_AUDIO_ALLOW_CHANNELS_CHANGE
	audio_device = SDL_OpenAudioDevice(NULL, 0, &requested_audio_spec, &audio_spec,
		SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
	if (audio_device == 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not open audio device! %s", SDL_GetError());
	} else {
		pcm_driver.frequency = audio_spec.freq;
		pcm_driver.channels = audio_spec.channels;
		pcm_driver.volume = 200;
		pcm_driver.latency = 1;
		pcm_driver.format_signed = true;

		zoo_sound_pcm_init(&pcm_driver);
		state.sound.d_sound = &pcm_driver.parent;

		SDL_PauseAudioDevice(audio_device, 0);
	}
}

static void exit_audio(void) {
	if (audio_device != 0) {
		SDL_CloseAudioDevice(audio_device);
	}
}

// game logic

static uint32_t sdl_pit_tick(uint32_t interval, void *param) {
	if (stop_tick_thread) {
		// cease
		return 1000;
	}

	SDL_LockMutex(playfield_mutex);

	SDL_LockMutex(audio_mutex);
	zoo_tick_advance_pit(&state);
	zoo_sound_tick(&state.sound);
	zoo_sound_pcm_tick(&pcm_driver);
	SDL_UnlockMutex(audio_mutex);

	zoo_input_tick(&state.input);

	SDL_UnlockMutex(playfield_mutex);

	return ZOO_PIT_TICK_MS;
}

static uint32_t sdl_game_tick(uint32_t interval, void *param) {
	int tick_delay = ZOO_PIT_TICK_MS;
	bool ticking = true;

	if (stop_tick_thread) {
		// cease
		return 1000;
	}
	
	SDL_LockMutex(playfield_mutex);

	zoo_ui_tick(&ui_state);

	while (ticking) {
		switch (zoo_tick(&state)) {
			case RETURN_IMMEDIATE:
				break;
			case RETURN_NEXT_FRAME:
				tick_delay = 16;
				ticking = false;
				break;
			case RETURN_NEXT_CYCLE:
				ticking = false;
				break;
		}
	}
	SDL_UnlockMutex(playfield_mutex);

	return tick_delay;
}

void sdl_draw_char(zoo_video_driver *drv, int16_t x, int16_t y, uint8_t col, uint8_t chr) {
	software_draw_char(&render_opts, playfield_buffer, playfield_pitch, x, y, col, chr);
	playfield_changed = true;
}

void sdl_render(void) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_LockMutex(playfield_mutex);
	SDL_UnlockTexture(playfield);
	SDL_RenderCopy(renderer, playfield, NULL, NULL);
	SDL_LockTexture(playfield, NULL, &playfield_buffer, &playfield_pitch);
	SDL_UnlockMutex(playfield_mutex);

	SDL_RenderPresent(renderer);
}

static SDL_KeyCode as_shifted(SDL_KeyCode kcode) {
	if (kcode >= 'a' && kcode <= 'z') {
		return kcode - 32;
	} else switch(kcode) {
		case '1': return '!';
		case '2': return '@';
		case '3': return '#';
		case '4': return '$';
		case '5': return '%';
		case '6': return '^';
		case '7': return '&';
		case '8': return '*';
		case '9': return '(';
		case '0': return ')';
		case '-': return '_';
		case '=': return '+';
		case '[': return '{';
		case ']': return '}';
		case ';': return ':';
		case '\'': return '"';
		case '\\': return '|';
		case ',': return '<';
		case '.': return '>';
		case '/': return '?';
		case '`': return '~';
		default: return kcode;
	}
}

static uint16_t sdl_to_zoo_keycode(SDL_KeyCode code, bool shift) {
	if (code > 0 && code < 128) {
		return shift ? as_shifted(code) : code;
	} else if (code >= SDLK_F1 && code <= SDLK_F10) {
		return ((code - SDLK_F1) + ZOO_KEY_F1);
	} else switch (code) {
		case SDLK_UP: return ZOO_KEY_UP;
		case SDLK_LEFT: return ZOO_KEY_LEFT;
		case SDLK_RIGHT: return ZOO_KEY_RIGHT;
		case SDLK_DOWN: return ZOO_KEY_DOWN;
		case SDLK_PAGEUP: return ZOO_KEY_PAGE_UP;
		case SDLK_PAGEDOWN: return ZOO_KEY_PAGE_DOWN;
		case SDLK_INSERT: return ZOO_KEY_INSERT;
		case SDLK_DELETE: return ZOO_KEY_DELETE;
		case SDLK_HOME: return ZOO_KEY_HOME;
		case SDLK_END: return ZOO_KEY_END;
		default: return 0;
	}
}

// main

// TODO HACK
zoo_video_driver video_driver;

int main(int argc, char **argv) {
	bool use_slim_ui = true;
	SDL_Event event;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed! %s", SDL_GetError());
		return 1;
	}

	srand(time(NULL));

	zoo_state_init(&state);
	zoo_io_create_posix_driver(&io_driver);
	video_driver.func_write = sdl_draw_char;
	state.d_io = &io_driver.parent;
	state.d_video = &video_driver;
	state.random_seed = rand();

	if (use_slim_ui) {
		state.func_draw_sidebar = zoo_draw_sidebar_slim;
		video.width = 60;
		video.height = 26;
	} else {
		state.func_draw_sidebar = zoo_draw_sidebar_classic;
		video.width = 80;
		video.height = 25;
	}

	video.buffer = malloc(video.width * video.height * 2);

	render_opts.charset = res_8x14_bin;
	render_opts.palette = default_palette;
	render_opts.char_width = 8;
	render_opts.char_height = 14;
	render_opts.flags = 0;
	render_opts.bpp = 32;

	window = SDL_CreateWindow("libzoo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		video.width * render_opts.char_width,
		video.height * render_opts.char_height,
		SDL_WINDOW_ALLOW_HIGHDPI);
	// TODO: SDL_WINDOW_RESIZABLE

	if (window == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindow failed! %s", SDL_GetError());
		return 1;
	}

        SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateRenderer failed! %s", SDL_GetError());
		return 1;
	}

        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

	playfield = SDL_CreateTexture(renderer, SOFT_PIXEL_FORMAT,
		SDL_TEXTUREACCESS_STREAMING,
		video.width * render_opts.char_width,
		video.height * render_opts.char_height);
	SDL_LockTexture(playfield, NULL, &playfield_buffer, &playfield_pitch);

	playfield_mutex = SDL_CreateMutex();

	zoo_redraw(&state);
	zoo_ui_init(&ui_state, &state);

	init_audio();

	tick_thread_pit = SDL_AddTimer(ZOO_PIT_TICK_MS, sdl_pit_tick, NULL);
	tick_thread_game = SDL_AddTimer(1, sdl_game_tick, NULL);

	// run
	bool cont_loop = true;

	while (cont_loop) {
		SDL_LockMutex(playfield_mutex);
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYDOWN: {
					zoo_input_action_set(&(state.input), ZOO_ACTION_SHOOT, (event.key.keysym.mod & KMOD_SHIFT));
					uint16_t kcode = sdl_to_zoo_keycode(event.key.keysym.sym, event.key.keysym.mod & KMOD_SHIFT);
					if (kcode != 0) {
						zoo_ui_input_key(&state, &ui_state.input, kcode, true);
					}
				} break;
				case SDL_KEYUP: {
					zoo_input_action_set(&(state.input), ZOO_ACTION_SHOOT, (event.key.keysym.mod & KMOD_SHIFT));
					uint16_t kcode = sdl_to_zoo_keycode(event.key.keysym.sym, event.key.keysym.mod & KMOD_SHIFT);
					if (kcode != 0) {
						zoo_ui_input_key(&state, &ui_state.input, kcode, false);
					}
				} break;
				case SDL_QUIT:
					cont_loop = false;
					break;
			}
		}
		SDL_UnlockMutex(playfield_mutex);

		sdl_render();
	}

	SDL_LockMutex(playfield_mutex);
	stop_tick_thread = true;
	SDL_RemoveTimer(tick_thread_pit);
	SDL_RemoveTimer(tick_thread_game);
	SDL_UnlockMutex(playfield_mutex);

	exit_audio();

	SDL_DestroyTexture(playfield);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
