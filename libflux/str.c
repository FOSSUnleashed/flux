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


uint8_t * flux_bufindexnull(const uint8_t * in, const uint8_t * end, uint8_t ch) {
	if (end < in || NULL == in || NULL == end) {
		return NULL;
	}

	for (; in < end; ++in) {
		if (ch == *in) {
			return (uint8_t *)in;
		}
	}

	return (uint8_t *)end;
}

uint8_t * flux_bufindex(const uint8_t * in, const uint8_t * end, uint8_t ch) {
	const uint8_t *p = flux_bufindexnull(in, end, ch);

	return NULL != p && *p == ch ? (uint8_t *)p : NULL;
}

int flux_bufcmp(const uint8_t * a, const uint8_t * aend, const uint8_t * b, const uint8_t * bend) {
	int res = 0;

	if (aend < a || bend < b || NULL == a || NULL == b || NULL == aend || NULL == bend) {
		errno = EINVAL;
		return -257;
	}

	if ((aend - a) != (bend - b)) {
		return -257;
	}

	// b size check infered
	for (; a < aend && 0 == res; ++a, ++b) {
		res = *a - *b;
	}

	return res;
}

int flux_bufeq(const uint8_t * a, const uint8_t * aend, const uint8_t * str, uint8_t ** out) {
	int res = 0;

	if (aend < a || NULL == a || NULL == aend || NULL == str) {
		errno = EINVAL;
		return -257;
	}

	for (; a < aend && 0 == res && *str; ++a, ++str) {
		res = *a - *str;
	}

	if (NULL != out) {
		*out = (uint8_t *)a - 1;
	}

	return res;
}

uint8_t * flux_bufcpy(uint8_t * dst, const uint8_t * dstend, const uint8_t * src, const uint8_t * srcend) {
	if (NULL == dst || NULL == dstend || NULL == src || NULL == srcend) {
		errno = EINVAL;
		return NULL;
	}

	if ((srcend - src) > (dstend - dst)) {
		errno = ERANGE;
		return NULL;
	}

	// infer that dst will be valid from above check
	for (; src < srcend; ++src, ++dst) {
		*dst = *src;
	}

	return dst;
}

uint8_t * flux_buflcpy(uint8_t * dst, const uint8_t * dstend, const uint8_t * src, const uint8_t * srcend) {
	uint8_t * p = flux_bufcpy(dst, dstend, src, srcend);

	if (p) {
		*p = 0;
	}

	return p;
}

uint8_t * flux_bufwrite(uint8_t * dst, const uint8_t * dstend, const uint8_t * src, size_t sz) {
	uint8_t * end = flux_bufend(src, src + sz);

	return flux_bufcpy(dst, dstend, src, end);
}

uint8_t * flux_buflwrite(uint8_t * dst, const uint8_t * dstend, const uint8_t * src, size_t sz) {
	uint8_t * end = flux_bufend(src, src + sz);

	return flux_buflcpy(dst, dstend, src, end);
}

void flux_bufset(uint8_t * dst, const uint8_t * dstend, uint8_t val) {
	while (dst < dstend) {
		*dst++ = val;
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
