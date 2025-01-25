#include <flux/str.h>
#include <string.h>

#define IS_SEP(c) (' ' == (c) || '\t' == (c) || '=' == (c))

int flux_str_parse_ctl(const char * data, char * buf, uint32_t sz, uint32_t bsz, char **V) {
	char *p = data, *bufend, *dataend;

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
