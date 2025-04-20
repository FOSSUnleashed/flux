#include <flux/str.h>
#include <string.h>
#include <errno.h>

/*
	* [X] strlen() -> return pointer to NUL
	* [X] cmp
	* [X] indexof (memchr?)
	* [X] cpy
	* [X] chrnul (like indexof, but returns point to NUL terminator on failure)
	* [ ] strstr
	* [ ] glob()
	* [X] error() (use strerror_r()) (three error functions, two sig functions)
*/

#define NOTEND(b) (b.start < b.end)

const Buffer badbuffer = {NULL, NULL};

// trim() move end pointer -- trim to space or trim to NUL
// seek() move start pointer -- advance

// return NULL on:
// * in is NULL
// * end is NULL
// * end is not after in
uint8_t * flux_bufindexnull(Buffer data, uint8_t ch) {
	if (isbadbuffer(data)) {
		return NULL;
	}

	for (; NOTEND(data); ++data.start) {
		if (ch == *data.start) {
			return data.start;
		}
	}

	return data.end;
}

// Return NULL if we fail to find `ch`
uint8_t * flux_bufindex(Buffer data, uint8_t ch) {
	uint8_t *p = flux_bufindexnull(data, ch);

	return NULL != p && data.end != p && *p == ch ? p : NULL;
}

// Fails if buffers are different sizes
int flux_bufcmp(Buffer a, Buffer b, uint8_t ** out) {
	int res = 0;

	if (isbadbuffer(a) || isbadbuffer(b)) {
		errno = EINVAL;
		return -257;
	}

	if ((a.end - a.start) != (b.end - b.start)) {
		return -257;
	}

	// b size check infered
	for (; a.start < a.end && 0 == res; ++a.start, ++b.start) {
		res = *a.start - *b.start;
	}

	if (NULL != out) {
		*out = a.start;
	}

	return res;
}

int flux_bufcasecmp(Buffer a, Buffer b, uint8_t ** out) {
	int res = 0;

	if (isbadbuffer(a) || isbadbuffer(b)) {
		errno = EINVAL;
		return -257;
	}

	if ((a.end - a.start) != (b.end - b.start)) {
		return -257;
	}

	// b size check infered
	for (; a.start < a.end && 0 == res; ++a.start, ++b.start) {
		res = (*a.start & 0xDF) - (*b.start & 0xDF);
	}

	if (NULL != out) {
		*out = a.start;
	}

	return res;
}

// *out returns the last character matched successfully
int flux_bufeq(Buffer a, Buffer b, uint8_t ** out) {
	int res = 0;

	// Fail if either buffer is bad
	// Fail if b is not NUL-terminated
	if (isbadbuffer(a) || isbadbuffer(b) || *(b.end)) {
		errno = EINVAL;
		return -257;
	}

	// Fail if b is bigger than a
	if ((a.end - a.start) < (b.end - b.start)) {
		errno = ERANGE;
		return -257;
	}

	for (; NOTEND(a) && NOTEND(b) && 0 == res && *b.start; ++a.start, ++b.start) {
		res = *a.start - *b.start;
	}

	if (NULL != out) {
		*out = (uint8_t *)a.start - 1;
	}

	return res;
}

Buffer flux_bufcpy(Buffer dst, Buffer src) {
	if (isbadbuffer(dst) || isbadbuffer(src)) {
		errno = EINVAL;
		return badbuffer;
	}

	if ((src.end - src.start) > (dst.end - dst.start)) {
		errno = ERANGE;
		return badbuffer;
	}

	uint8_t *p = dst.start;

	// infer that dst will be valid from above check
	for (; NOTEND(src); ++src.start, ++p) {
		*p = *src.start;
	}

	dst.end = p;

	return dst;
}

// dst start end
// out start eod
Buffer flux_buflcpy(Buffer dst, Buffer src) {
	Buffer out = flux_bufcpy(dst, src);

	if (!isbadbuffer(out) && *out.end && out.end < dst.end) {
		*out.end = 0;
		out.end++;
	}

	return out;
}

FluxBuffer flux_bufextract(FluxBuffer haystack, uint8_t needle, FluxBuffer *remainder) {
	uint8_t *p = haystack.start;

	if (isbadbuffer(haystack)) {
		return badbuffer;
	}

	flux_bufadvance(p, haystack.end, needle != *p);

	if (NULL != remainder) {
		remainder->end	= haystack.end;
		remainder->start	= p;

		flux_bufadvance(remainder->start, remainder->end, needle == *remainder->start);
	}

	haystack.end = p;

	return haystack;
}

// Sets a buffer to be all of one value
void flux_bufset(Buffer dst, uint8_t val) {
	while (NOTEND(dst)) {
		*dst.start++ = val;
	}
}

// TODO
// [rw][su](b?)(8|16|32|64) -- 32 functions 

// uint8_t * flux_buf_r8(const uint8_t * in, const uint8_t * inend, uint8_t * out)
// flux_buf_r16
// flux_buf_r32
// flux_buf_r64
// uint8_t * flux_buf_w8(uint8_t * out, const uint8_t * outend, uint8_t in)
// flux_buf_w16
// flux_buf_w32
// flux_buf_w64


uint8_t *flux_bufdump(Buffer out, Buffer in) {
	uint8_t *b = out.start, *c = in.start, *p, chars[] = "0123456789ABCDEF";
	uintptr_t offset;

	if (isbadbuffer(in) || isbadbuffer(out)) {
		errno = EINVAL;
		return NULL;
	}

	// 67 characters per line of output
	while (c < in.end && b + 68 < out.end) {
		offset = c - in.start;

		// offset (9 characters)
		p = b + 8;

		*p-- = ':';

		while (p >= b) {
			*p-- = chars[offset & 0x0F];
			offset >>= 4;
		}

		b += 9;

		// hex columns (41 characters)

		*b++ = ' ';
		p = c;

		while (p - c < 16) {
			if (p < in.end) {
				*b++ = chars[(*p & 0xF0) >> 4];
				*b++ = chars[ *p & 0x0F];
			} else {
				*b++ = ' ';
				*b++ = ' ';
			}
			if ((p - c) & 1) {
				*b++ = ' ';
			}
			++p;
		}

		// ascii colums (17 characters)

		*b++ = ' ';
		p = c;

		while (p - c < 16) {
			if (p < in.end) {
				*b++ = (0x20 > *p || *p > 0x7F) ? '.' : *p;
			} else {
				*b++ = ' ';
			}
			++p;
		}

		c += 16;
		*b++ = '\n';
	}

	return b;
}





#define IS_SEP(c) (' ' == (c) || '\t' == (c) || '=' == (c))

int flux_str_parse_ctl(const char * data, char * buf, uint32_t sz, uint32_t bsz, char **V) {
	const char *p = data, *bufend, *dataend;

	dataend = data + sz;
	bufend = buf + bsz;

	// get the key
	for (; *p && buf < bufend && p < dataend && !IS_SEP(*p); ++p, ++buf) {
		*buf = *p;
	}

	if (p >= dataend || buf >= bufend) {
		goto error;
	}

	*buf = 0;
	++buf;
	*V = buf;

	for (; *p && buf < bufend && p < dataend && IS_SEP(*p); ++p);

	if (p >= dataend || buf >= bufend) {
		goto error;
	}

	for (; *p && buf < bufend && p < dataend; --sz, ++p, ++buf) {
		*buf = *p;
	}

	if (buf >= bufend) {
		goto error;
	}

	*buf = 0;

	return buf - *V; // return size of value
	error:
	*V = NULL;
	return -1;
}
