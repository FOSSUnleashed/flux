#include <snow.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define JIM_IMPLEMENTATION
#include <jsmn.h>

#include <new.h>

void json_buffer_copy(char *dst, char *src) {
	for (; *src; ++dst, ++src) {
		*dst = *src;
		if ('\\' == *src) {
			if ('n' == src[1]) {
				++src;
				*dst = '\n';
			}
		}
	}
	*dst = 0;
}

size_t jim_writer(const void *src, size_t size, size_t nmemb, Jim_Sink sink) {
	uint64_t sz = size * nmemb;
	JumboJim *j = sink;

	if (sz > j->sz) {
		return 0;
	}

	memcpy(j->buf, src, sz);
	j->sz -= sz;
	j->buf += sz;

	return sz;
}

void jim_setup(JumboJim *j, char * buf, size_t sz) {
	memset(j, 0, sizeof(JumboJim));

	j->buf = buf;
	j->sz	= sz;
	j->j.sink = j;
	j->j.write = jim_writer;
}
