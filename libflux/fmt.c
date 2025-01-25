#include <stdio.h>
#include <flux/fmt.h>
#include <dill/bsock.h>
#include <dill/msock.h>
#include <stdarg.h>

int iol_fmt(struct iolist *iol, const char * const fmt, ...) {
	va_list ap;
	size_t sz;

	va_start(ap, fmt);
	sz = vsnprintf(iol->iol_base, iol->iol_len, fmt, ap);
	va_end(ap);

	if (0 > sz) {
		return -1;
	}

	iol->iol_len = sz;

	return 0;
}
