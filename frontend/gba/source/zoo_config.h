#ifndef __ZOO_CONFIG_H__
#define __ZOO_CONFIG_H__

// GBA-specific defines

#define IWRAM_ARM_CODE __attribute__((section(".iwram"), long_call, target("arm")))

// Configuration

#undef ZOO_CONFIG_ENABLE_POSIX_FILE_IO

#define ZOO_CONFIG_ENABLE_UI_SLIM

#undef ZOO_CONFIG_USE_DOUBLE_FOR_MS

#endif /* __ZOO_CONFIG_H__ */
