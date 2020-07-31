#ifndef __ZOO_GBA_H__
#define __ZOO_GBA_H__

#ifdef ZOO_DEBUG_MENU
#define DEBUG_CONSOLE
#endif

#define ZOO_GBA_ENABLE_BLINKING

#define IWRAM_ARM_CODE __attribute__((section(".iwram"), long_call, target("arm")))
#define EWRAM_DATA __attribute__((section(".ewram")))
#define EWRAM_BSS __attribute__((section(".sbss")))

#endif /* __ZOO_GBA_H__ */
