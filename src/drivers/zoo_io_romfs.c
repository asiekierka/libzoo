#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include "zoo_io_romfs.h"

// TODO: add support for directories

static enum {
	ROMFS_TYPE_HLINK = 0,
	ROMFS_TYPE_DIR = 1,
	ROMFS_TYPE_FILE = 2,
	ROMFS_TYPE_SLINK = 3,
	ROMFS_TYPE_BLOCK = 4,
	ROMFS_TYPE_CHAR = 5,
	ROMFS_TYPE_SOCKET = 6,
	ROMFS_TYPE_FIFO = 7
} romfs_type;

static ZOO_INLINE uint32_t romfs_read(uint8_t *ptr) {
	return (ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3];
}

static zoo_io_handle zoo_io_open_file_romfs(zoo_io_path_driver *drv, const char *name, zoo_io_mode mode) {
	FILE *file;
	zoo_io_handle h;
	uint8_t *curr_entry = ((zoo_io_romfs_driver *) drv)->root_ptr;
	uint8_t *file_ptr;
	uint8_t *base = ((zoo_io_romfs_driver *) drv)->fs;
	uint32_t offset;

	if (mode == MODE_WRITE) {
		return zoo_io_open_file_empty();
	}

	// skip root path
	if (name[0] == '/') {
		name++;
	}

	// search for file
	while (true) {
		offset = romfs_read(curr_entry);
		if ((offset & 7) == ROMFS_TYPE_FILE && !strcasecmp(name, (char *) (curr_entry + 16))) {
			// correct file
			file_ptr = curr_entry + ((16 + strlen((char*) (curr_entry + 16)) + 16) & (~15));
			h = zoo_io_open_file_mem(file_ptr, romfs_read(curr_entry + 8), MODE_READ);
			return h;
		}

		// advance
		curr_entry = base + (offset & (~15));
		if (curr_entry == base) {
			break;
		}
	}

	// did not find file
	h = zoo_io_open_file_empty();
	return h;
}

static bool zoo_io_scan_dir_romfs(zoo_io_path_driver *drv, const char *name, zoo_func_io_scan_dir_callback cb, void *cb_arg) {
	zoo_io_dirent ent;
	uint8_t *curr_entry = ((zoo_io_romfs_driver *) drv)->root_ptr;
	uint8_t *base = ((zoo_io_romfs_driver *) drv)->fs;
	uint32_t offset;
	
	while (true) {
		offset = romfs_read(curr_entry);
		if ((offset & 7) == ROMFS_TYPE_FILE) {
			ent.type = ((offset & 7) == ROMFS_TYPE_DIR) ? TYPE_DIR : TYPE_FILE;
			strncpy(ent.name, (char*) (curr_entry + 16), ZOO_PATH_MAX);

			if (!cb(drv, &ent, cb_arg)) {
				break;
			}
		}

		curr_entry = base + (offset & (~15));
		if (curr_entry == base) {
			break;
		}
	}

	return true;
}

bool zoo_io_create_romfs_driver(zoo_io_romfs_driver *drv, uint8_t *fs_ptr) {
	if (memcmp(fs_ptr, "-rom1fs-", 8)) {
		return false;
	}
	// TODO: add checksum validation

	zoo_io_internal_init_path_driver(&drv->parent);
	strncpy(drv->parent.path, "/", ZOO_PATH_MAX);

	drv->parent.parent.read_only = true;
	drv->parent.func_open_file_absolute = zoo_io_open_file_romfs;
	drv->parent.func_dir_scan = zoo_io_scan_dir_romfs;

	// set up filesystem pointers
	drv->fs = fs_ptr;
	drv->root_ptr = fs_ptr + ((16 + strlen((char*) (fs_ptr + 16)) + 16) & (~15));
	return true;
}