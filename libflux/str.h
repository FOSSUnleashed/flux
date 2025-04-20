#pragma once
#include <errno.h>
#include <flux/str.h>

#define NOTEND(b) (b.start < b.end)
#define BUFSZ(b) ((b).end - (b).start)

extern const Buffer badbuffer;

#define NIBBLE(hc) ((hc) > 9 ? 'a' + (hc) - 10 : '0' + (hc))

static inline uint8_t * writehexchar(Buffer dst, uint8_t c) {
	*dst.start++ = NIBBLE((c & 0xF0) >> 4);
	*dst.start++ = NIBBLE(c & 0x0F);
	return dst.start;
}
