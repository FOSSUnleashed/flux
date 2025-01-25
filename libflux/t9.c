#include <flux/t9.h>
#include <dill/util.h>
#include <dill/bsock.h>
#include <stdlib.h>

// actual read
uint8_t *dill_t9read(C9ctx *ctx, uint32_t size, int *err) {
	T9client * c = dill_cont(ctx, T9client, ctx);

	if (0 == brecv(c->h, c->rbuf, size, -1)) {
		return c->rbuf;
	}

	*err = C9Ebuf;

	return NULL;
}

// "allocation"
uint8_t *dill_t9begin(C9ctx *ctx, uint32_t size) {
	T9client * c = dill_cont(ctx, T9client, ctx);
	uint8_t *b = c->wbuf + c->woff;

	// TODO: size check

	c->woff =+ size;

	return b;
}

// ...
int dill_t9end(C9ctx *ctx) {
	T9client * c = dill_cont(ctx, T9client, ctx);

	if (0 == bsend(c->h, c->wbuf, c->woff, -1)) {
		c->woff = 0;
		return 0;
	}

	return C9Ebuf;
}

T9client *t9openClient(handle h) {
	T9client * c = calloc(1, sizeof(T9client));

	if_slow (NULL == c) {
		return NULL;
	}

	c->ctx.read = dill_t9read;
	c->ctx.begin = dill_t9begin;
	c->ctx.end = dill_t9end;

	return c;
}
