#ifndef __ZOO_GBA_H__
#define __ZOO_GBA_H__

#ifdef ZOO_DEBUG_MENU
#define DEBUG_CONSOLE
#endif

#define IWRAM_ARM_CODE __attribute__((section(".iwram"), long_call, target("arm")))

#endif /* __ZOO_GBA_H__ */
