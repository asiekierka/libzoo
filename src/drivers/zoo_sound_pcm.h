#ifndef __ZOO_SOUND_PCM__
#define __ZOO_SOUND_PCM__

#include <stddef.h>
#include <stdint.h>
#include "../libzoo/zoo.h"

typedef struct {
	uint32_t tick;
	uint32_t emitted;
	const uint16_t *freqs;
	uint16_t pos, len;
	bool clear;
} zoo_pcm_entry;

typedef struct {
    zoo_sound_driver parent;

	// user-configurable
	uint32_t frequency;
	uint8_t channels;
	uint8_t volume;
	bool format_signed;
	uint8_t latency; // in ticks

	// generated
	zoo_pcm_entry buffer[ZOO_CONFIG_SOUND_PCM_BUFFER_LEN];
	uint16_t buf_pos;
	uint16_t buf_len;
	int32_t buf_ticks;
	double sub_ticks;
	int32_t ticks;
} zoo_sound_pcm_driver;

void zoo_sound_pcm_init(zoo_sound_pcm_driver *drv);
void zoo_sound_pcm_generate(zoo_sound_pcm_driver *drv, uint8_t *stream, size_t len);
void zoo_sound_pcm_tick(zoo_sound_pcm_driver *drv);

#endif /* __ZOO_SOUND_PCM__ */