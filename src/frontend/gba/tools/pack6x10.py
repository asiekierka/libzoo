#!/usr/bin/python3

# Copyright (c) 2020 Adrian Siekierka
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# format:
# first 1024 bytes are a array of 256 uint16_t[2]s
# describing the two tiles encompassing a 6x10 character
# next, a uint16_t stores the total count of tiles
# next, 8-byte 1-bit 8x8 bitmaps follow for each tile

from PIL import Image
import struct, sys

tile_ids = 0
tile_id_dict = {}
tiles = []
tile_descs = []

def write_tile(fp, tile):
	for i in range(0, 8):
		if i >= len(tile):
			fp.write(struct.pack("<B", 0))
		else:
			fp.write(struct.pack("<B", tile[i]))

def add_tile(im, x, y, w, h):
	global tile_ids, tile_id_dict, tile_descs
	tile = []
	for iy in range(0, h):
		ti = 0
		for ix in range(0, w):
			pxl = im.getpixel((x+ix, y+iy))
			ti = (ti >> 1)
			if pxl[0] > 128:
				ti = ti + 128
		tile.append(ti)
	tile = tuple(tile)
	if tile in tile_id_dict:
		return tile_id_dict[tile]
	else:
		tile_id_dict[tile] = tile_ids
		tiles.append(tile)
		tile_ids = tile_ids + 1
		return tile_id_dict[tile]

im = Image.open(sys.argv[1]).convert("RGBA")
for c in range(256):
	cx = int(c % 32) * 6
	cy = int(c / 32) * 10
	tile_descs.append(add_tile(im, cx, cy, 6, 8))
	tile_descs.append(add_tile(im, cx, cy + 8, 6, 2))

with open(sys.argv[2], "wb") as fp:
	for i in range(0, 256 * 2):
		fp.write(struct.pack("<H", tile_descs[i]))
	fp.write(struct.pack("<H", tile_ids))
	for i in range(0, tile_ids):
		write_tile(fp, tiles[i])
