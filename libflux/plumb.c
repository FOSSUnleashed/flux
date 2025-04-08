#include <flux/str.h>
#include <flux/plumb.h>
#include <errno.h>

#define NEXT(B) do {\
	for (; *b && '\n' != *b && b < be; ++b) {B;}\
	if (b >= be) {\
		goto error_smallbuf;\
	}\
	*b = 0;\
} while (0)

int parse_pmsg(pmsg *p, uint8_t *data, uint8_t *be) {
	uint8_t *b = data;

	if (be <= b || NULL == p) {
		errno = EINVAL;
		return -1;
	}

	p->sz = 0;

	p->src	= b; NEXT();
	p->esrc	= b++;
	p->dst	= b; NEXT();
	p->edst	= b++;
	p->cwd	= b; NEXT();
	p->ecwd	= b++;
	p->type	= b; NEXT();
	p->etype	= b++;
	p->attr	= b; NEXT();
	p->eattr	= b++;
	p->size	= b; NEXT(p->sz = p->sz * 10 + *b - '0');
	p->data	= ++b;
	p->edata	= b + p->sz;

	if (p->edata > be) {
		goto error_smallbuf;
	}

	// We have more than just this message in the buffer
	if (p->edata < be) {
		return 1;
	}

	return 0;

	// Buffer did not contain a full message
	error_smallbuf:
	errno = EFAULT; // "Bad Address" pretty sure not the correct one to use
	return -1;
}
