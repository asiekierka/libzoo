/**
 * Copyright (c) 2020 Adrian Siekierka
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __ZOO_CONFIG_DEFAULTS_H__
#define __ZOO_CONFIG_DEFAULTS_H__

// Features

// Enable file I/O, state tracking, related UI methods, etc.
#define ZOO_CONFIG_ENABLE_FILE_IO

// Enable POSIX implementation of file/dir I/O.
#define ZOO_CONFIG_ENABLE_FILE_IO_POSIX

// Enable editor-specific constants in element definitions.
// #define ZOO_CONFIG_ENABLE_EDITOR_CONSTANTS

// Enable PCM sound logic.
// #define ZOO_CONFIG_ENABLE_SOUND_PCM
#define ZOO_CONFIG_SOUND_PCM_BUFFER_LEN 32 // ~1.5 seconds of audio

// Various sidebar styles.
// - classic (80x25 ZZT-like)
#define ZOO_CONFIG_ENABLE_UI_CLASSIC
// - slim (60x26 bottom bar)
#define ZOO_CONFIG_ENABLE_UI_SLIM

// Options

// Use doubles instead of ints for passing millisecond values.
// Provides increased timer accuracy.
#define ZOO_CONFIG_USE_DOUBLE_FOR_MS

// Compiler options

#ifndef ZOO_PATH_SEPARATOR
#ifdef _WIN32
#define ZOO_PATH_SEPARATOR '\\'
#define ZOO_PATH_SEPARATOR_STR "\\"
#else
#define ZOO_PATH_SEPARATOR '/'
#define ZOO_PATH_SEPARATOR_STR "/"
#endif
#endif

#ifndef ZOO_PATH_MAX
#ifdef _WIN32
#define ZOO_PATH_MAX 260
#else
// TODO: Linux uses 4096 by default
#define ZOO_PATH_MAX 512
#endif
#endif

#if __STDC_VERSION__ >= 199901L
#define ZOO_INLINE inline
#else
#define ZOO_INLINE
#endif

#endif /* __ZOO_CONFIG_DEFAULTS_H__ */
