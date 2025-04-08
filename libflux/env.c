#include <stdlib.h>
#include <flux/str.h>
#include <string.h>

Buffer flux_envbuf(const char * env) {
	Buffer b = {};

	b.start = getenv(env);

	if (NULL != b.start) {
		b.end	= b.start + strlen(b.start);
	}

	return b;
}
