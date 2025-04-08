// TODO: Replace with GPL3
/*

  Copyright (c) 2017 Martin Sustrik

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.

*/

#ifndef LIBDILL_BYTES_H_INCLUDED
#define LIBDILL_BYTES_H_INCLUDED

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// TODO: endianess checks
// [rw][us]{8, 16, 32, 64} Read/Write (Un)Signed integers

// Read Unsigned Integers
#define dill_ru8(b)	(*((uint8_t *)(b)) & 0xFF)
#define dill_ru16(b) ((uint16_t)dill_ru8(b) | (uint16_t)dill_ru8(b + 1) << 8)
#define dill_ru32(b) ((uint32_t)dill_ru16(b) | (uint32_t)dill_ru16(b + 2) << 16)
#define dill_ru64(b) ((uint32_t)dill_ru32(b) | (uint64_t)dill_ru32(b + 4) << 32)

// Read Signed Integers
#define dill_rs8(b)	(*((int8_t *)(b)) & 0xFF)
#define dill_rs16(b) ((int16_t)dill_rs8(b + 1) | (int16_t)dill_rs8(b) << 8)
#define dill_rs32(b) ((int32_t)dill_rs16(b + 2) | (int32_t)dill_rs16(b) << 16)
#define dill_rs64(b) ((int32_t)dill_rs32(b + 4) | (int64_t)dill_rs32(b) << 32)

// Write unsigned integer
#define dill_wu8(b, v) do { *(b) = (uint8_t)(v) & 0xFF; } while (0)
#define dill_wu16(b, v) dill_wu8(b, (uint8_t)(v)); dill_wu8(b + 1, (uint8_t)((v) >> 8));
#define dill_wu32(b, v) dill_wu16(b, (uint16_t)(v)); dill_wu16(b + 2, (uint16_t)((v) >> 16));
#define dill_wu64(b, v) dill_wu32(b, (uint32_t)(v)); dill_wu32(b + 4, (uint32_t)((v) >> 32));

// Swap Bytes Around
#define dill_swap16(v) (((v) >> 8) & 0xFF | ((v) & 0xFF) << 8)
#define dill_swap32(v) (dill_swap16(v >> 16) & 0xFFFF | (dill_swap16(v) & 0xFFFF) << 16)

// Strings
static inline uint16_t dill_ws(unsigned char * buf, const char * str, uint16_t len) {
	uint16_t cnt = 2 + len;
	dill_wu16(buf, len);
	buf += 2;
	for (; len; --len) {
		*buf++ = *str++;
	}
	return cnt;
}

static inline uint16_t dill_rs(unsigned char * buf, unsigned char * out) {
	uint16_t sz, res;

	sz	= dill_ru16(buf);
	res	= sz + 2;
	buf	+= 2;

	for (; sz; --sz) {
		*out++ = *buf++;
	}

	return res;
}

#ifdef __cplusplus
}
#endif

#endif
