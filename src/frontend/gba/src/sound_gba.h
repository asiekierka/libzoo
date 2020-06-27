#ifndef __SOUND_GBA__
#define __SOUND_GBA__

#include <stdbool.h>
#include <stdint.h>
#include "zoo.h"

typedef struct {
	zoo_sound_driver parent;

    const uint16_t *sound_freqs;
    uint16_t sound_len;
    bool sound_clear_flag;
} zoo_sound_gba_driver;

void zoo_sound_gba_clear(void);
void zoo_sound_gba_install(zoo_state *state);

#endif /* __SOUND_GBA__ */