#ifndef __VIDEO_GBA__
#define __VIDEO_GBA__

#include <stdbool.h>
#include <stdint.h>
#include "zoo.h"

void zoo_video_gba_hide(void);
void zoo_video_gba_show(void);
void zoo_video_gba_install(zoo_state *state, const uint8_t *charset_bin);
void zoo_video_gba_set_blinking(bool val);

#endif /* __VIDEO_GBA__ */