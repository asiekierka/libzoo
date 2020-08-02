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

#ifndef __ZOO_CONFIG_H__
#define __ZOO_CONFIG_H__

#ifndef ZOO_CONFIG_SOUND_PCM_BUFFER_LEN
#define ZOO_CONFIG_SOUND_PCM_BUFFER_LEN 32 // ~1.5 seconds of audio
#endif

// Feature flags
#ifdef ZOO_USE_ROM_POINTERS
// If we can't write to object code memory, we must enable some workarounds.
#ifndef ZOO_USE_LABEL_CACHE
#error Label cache required for ROM pointer support!
#endif
#define ZOO_STORE_LABEL_CACHE
#define ZOO_NO_OBJECT_CODE_WRITES
#endif

#endif /* __ZOO_CONFIG_H__ */
