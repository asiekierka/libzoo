#ifndef __ZOO_CONFIG_H__
#define __ZOO_CONFIG_H__

// GBA-specific defines

#define IWRAM_ARM_CODE __attribute__((section(".iwram"), long_call, target("arm")))

// Configuration

#undef ZOO_CONFIG_FILE_IO

#endif /* __ZOO_CONFIG_H__ */
