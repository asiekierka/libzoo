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
# 8-byte 1-bit 8x8 bitmaps for each tile

from PIL import Image
import struct, sys

tiles = []

def write_tile(fp, tile):
	for i in range(0, 8):
		if i >= len(tile):
			fp.write(struct.pack("<B", 0))
		else:
			fp.write(struct.pack("<B", tile[i]))

def add_tile(im, x, y, w, h):
	global tile_ids, tile_by_id, tile_descs
	tile = [0, 0, 0, 0, 0, 0, 0, 0]
	for iy in range(0, h):
		ti = 0
		for ix in range(0, w):
			pxl = im.getpixel((x+ix, y+iy))
			ti = (ti >> 1)
			if pxl[0] > 128:
				ti = ti + 128
		tile[iy] = ti
	tile = tuple(tile)
	tiles.append(tile)

im = Image.open(sys.argv[1]).convert("RGBA")
for c in range(256):
	cx = int(c % 32) * 4
	cy = int(c / 32) * 6
	add_tile(im, cx, cy, 4, 6)

with open(sys.argv[2], "wb") as fp:
	for i in range(len(tiles)):
		write_tile(fp, tiles[i])
