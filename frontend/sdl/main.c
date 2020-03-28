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
#include <SDL2/SDL.h>

#include "zoo.h"
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
static video_buffer video;
static render_options render_opts;
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *playfield;
static void *playfield_buffer;
static int playfield_pitch;
static SDL_mutex *playfield_mutex;
static bool playfield_changed;

static zoo_input_state input;
static SDL_mutex *input_mutex;

#ifdef __BIG_ENDIAN__
#define SOFT_PIXEL_FORMAT SDL_PIXELFORMAT_ARGB32
#else
#define SOFT_PIXEL_FORMAT SDL_PIXELFORMAT_BGRA32
#endif

// audio logic

static SDL_AudioDeviceID audio_device;
static SDL_AudioSpec audio_spec;
static SDL_mutex *audio_mutex;
static zoo_pcm_state pcm_state;

static void sdl_audio_callback(void *userdata, uint8_t *stream, int len) {
	SDL_LockMutex(audio_mutex);
	zoo_sound_pcm_generate(&pcm_state, stream, len);
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
		pcm_state.frequency = audio_spec.freq;
		pcm_state.channels = audio_spec.channels;
		pcm_state.volume = 200;
		pcm_state.latency = 2;
		pcm_state.format_signed = true;
		zoo_sound_pcm_init(&state.sound, &pcm_state);

		SDL_PauseAudioDevice(audio_device, 0);
	}
}

static void exit_audio(void) {
	if (audio_device != 0) {
		SDL_CloseAudioDevice(audio_device);
	}
}

// game logic

static uint32_t ticks = 0;

static uint32_t sdl_timer_tick(uint32_t interval, void *param) {
	ticks++;

	SDL_LockMutex(audio_mutex);
	zoo_sound_tick(&(state.sound));
	zoo_sound_pcm_tick(&pcm_state);
	SDL_UnlockMutex(audio_mutex);

	if (ticks & 1) {
		SDL_LockMutex(input_mutex);
		memcpy(&state.input, &input, sizeof(zoo_input_state));
		SDL_UnlockMutex(input_mutex);

		SDL_LockMutex(playfield_mutex);
		while (true) {
			switch (zoo_tick(&state)) {
				case RETURN_IMMEDIATE:
					break;
				case RETURN_NEXT_FRAME:
					SDL_UnlockMutex(playfield_mutex);
					return 16; // TODO: more accurate
				case RETURN_NEXT_CYCLE:
					SDL_UnlockMutex(playfield_mutex);
					return 55; // TODO: more accurate
			}
		}
	}

	return 55;
}

void sdl_draw_char(int16_t x, int16_t y, uint8_t col, uint8_t chr) {
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

// input logic

typedef enum {
	IDIR_UP,
	IDIR_DOWN,
	IDIR_LEFT,
	IDIR_RIGHT,
	IDIR_MAX
} input_dir;

static int dir_pressed[IDIR_MAX];
static int dir_pressed_id;

static void input_dir_down(input_dir dir) {
	if (dir_pressed[dir] == 0) {
		dir_pressed_id++;
		dir_pressed[dir] = dir_pressed_id;
	}
}

static void input_dir_up(input_dir dir) {
	dir_pressed[dir] = 0;
}

static void input_dir_update_delta() {
	int m = 0;
	input.delta_x = 0;
	input.delta_y = 0;

	for (int i = 0; i < IDIR_MAX; i++) {
		if (dir_pressed[i] > m) {
			m = dir_pressed[i];
			switch (i) {
				case IDIR_UP:
					input.delta_x = 0;
					input.delta_y = -1;
					break;
				case IDIR_DOWN:
					input.delta_x = 0;
					input.delta_y = 1;
					break;
				case IDIR_LEFT:
					input.delta_x = -1;
					input.delta_y = 0;
					break;
				case IDIR_RIGHT:
					input.delta_x = 1;
					input.delta_y = 0;
					break;
			}
		}
	}
}

// main

int main(int argc, char **argv) {
	SDL_Event event;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed! %s", SDL_GetError());
		return 1;
	}

	zoo_state_init(&state);
	state.func_write_char = sdl_draw_char;
	zoo_install_sidebar_slim(&state);

	if (argc < 2) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "No file provided!");
		return 1;
	}

	FILE *f = fopen(argv[1], "rb");
	if (!zoo_world_load_file(&state, f, true)) {
		fclose(f);
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading file!");
		return 1;
	}
	fclose(f);

	video.width = 60;
	video.height = 26;
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
	input_mutex = SDL_CreateMutex();

	zoo_game_start(&state, GS_TITLE);
	zoo_redraw(&state);

	init_audio();

	SDL_AddTimer(55, sdl_timer_tick, NULL);

	// run
	bool cont_loop = true;

	while (cont_loop) {
		SDL_LockMutex(input_mutex);
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYDOWN:
					input.shoot = (event.key.keysym.mod & KMOD_SHIFT) ? true : false;
					switch (event.key.keysym.sym) {
						case SDLK_LEFT:
							input_dir_down(IDIR_LEFT);
							break;
						case SDLK_RIGHT:
							input_dir_down(IDIR_RIGHT);
							break;
						case SDLK_UP:
							input_dir_down(IDIR_UP);
							break;
						case SDLK_DOWN:
							input_dir_down(IDIR_DOWN);
							break;
						case SDLK_t:
							if (state.game_state == GS_PLAY) {
								input.torch = true;
							}
							break;
					}
					break;
				case SDL_KEYUP:
					input.shoot = (event.key.keysym.mod & KMOD_SHIFT) ? true : false;
					switch (event.key.keysym.sym) {
						case SDLK_LEFT:
							input_dir_up(IDIR_LEFT);
							break;
						case SDLK_RIGHT:
							input_dir_up(IDIR_RIGHT);
							break;
						case SDLK_UP:
							input_dir_up(IDIR_UP);
							break;
						case SDLK_DOWN:
							input_dir_up(IDIR_DOWN);
							break;
						case SDLK_t:
							if (state.game_state == GS_PLAY) {
								input.torch = false;
							}
							break;
						case SDLK_p:
							if (state.game_state == GS_TITLE) {
								zoo_game_stop(&state);

								f = fopen(argv[1], "rb");
								if (!zoo_world_load_file(&state, f, false)) {
									fclose(f);
									SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error loading file!");
									cont_loop = false;
									break;
								}
								fclose(f);

								zoo_game_start(&state, GS_PLAY);
								zoo_redraw(&state);
							}
							break;
						case SDLK_b:
							state.sound.enabled = !state.sound.enabled;
							break;
					}
					break;
				case SDL_QUIT:
					cont_loop = false;
					break;
			}
		}
		input_dir_update_delta();
		SDL_UnlockMutex(input_mutex);

		sdl_render();
	}

	exit_audio();

	SDL_DestroyTexture(playfield);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
