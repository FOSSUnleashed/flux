#include <flux/str.h>
#include <errno.h>
#include <stdbool.h>
#include "str.h"

// [rw][su](b?)(8|16|32|64) -- 32 functions
// b == binary, read X bits from data stream

uint8_t *flux_buf_ws64(Buffer dst, int64_t val) {
	if (flux_isbadbuffer(dst)) {
		errno = EINVAL;
		return NULL;
	}

	uint8_t *pos = dst.end - 1;

	if (val < 0) {
		*dst.start++ = '-';
		val = -val;
	}

	while (pos >= dst.start) {
		*pos-- = '0' + val % 10;

		val /= 10;

		if (0 == val) {
			break;
		}
	}

	for (++pos; pos < dst.end; ++pos, ++dst.start) {
		*dst.start = *pos;
	}

	return dst.start;
}

int64_t flux_buf_rs64(Buffer in) {
	if (flux_isbadbuffer(in)) {
		errno = EINVAL;
		return 0;
	}

	int64_t val = 0;
	bool neg = false;

	if ('-' == *in.start) {
		in.start++;
		neg = true;
	}

	while (NOTEND(in) && '0' <= *in.start && '9' >= *in.start) {
		val = val * 10 + (*in.start++ - '0');
	}

	return neg ? -val : val;
}
