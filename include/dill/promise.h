#pragma once

typedef struct {
	int ch[2];
	void *success, *failure;
	uint8_t x[2], ref;
} Promise;

static inline int promise_init(Promise *p) {
	p->success = p->failure = NULL;

	p->ref = 0;

	return chmake(p->ch);
}

static inline int promise_pass(Promise *p, void *val, uint64_t deadline) {
	if (NULL != p->success || NULL != p->failure) {
		errno = EINVAL;

		return -1;
	}

	p->success = val;

	// If we have no listeners, we don't want to block on the off-chance that no listeners ever happen
	return p->ref ? chsend(p->ch[0], NULL, 0, deadline) : 0;
}

static inline int promise_fail(Promise *p, void *val, uint64_t deadline) {
	if (NULL != p->success || NULL != p->failure) {
		errno = EINVAL;

		return -1;
	}

	p->failure = val;

	// If we have no listeners, we don't want to block on the off-chance that no listeners ever happen
	return p->ref ? chsend(p->ch[0], NULL, 0, deadline) : 0;
}
