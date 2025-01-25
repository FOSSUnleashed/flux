#include <dill/all.h>
#include <r9.h>

#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

struct dill_rbtree_item *dill_rbtree_seek(struct dill_rbtree *self, int64_t val) {
	struct dill_rbtree_item *p;
	p = self->root.left;

	while (p && &self->nil != p) {
		if (val < p->val) {
			p = p->left;
		} else if (val > p->val) {
			p = p->right;
		} else {
			return p;
		}
	}

	return NULL;
}

void resetFid(R9fid * f, C9fid fid) {
	f->id = fid;
	f->isSession	= 0;
	f->isAuth	= 0;

	f->isOpen	= 0;
	f->isRead	= 0;
	f->isWrite	= 0;
	f->isExec	= 0;
	f->isTrunk	= 0;
	f->isRClose	= 0;
	f->s = NULL;
}

void setupClient(R9client *c, handle h, R9srv * srv) {
	if (NULL == c) {
		return;
	}

	c->h	= h;
	c->srv	= srv;

	dill_rbtree_init(&c->fidmap);
	dill_rbtree_init(&c->fidfree);
}

R9client *allocClient(R9srv * srv, handle h) {
	R9client *c;

	if (-1 == h) {
		return NULL;
	}

	c = calloc(sizeof(R9client), 1);

	if_slow(NULL == c) {
		goto exit;
	}

	assert((void *)c == (void *)&c->ctx);
	assert(h > 0); // It really shouldn't be 0

	setupClient(c, h, srv);

	exit:
	return c;
}

R9fid *seekFid(R9client * c, C9fid fid) {
	struct dill_rbtree_item *n = dill_rbtree_seek(&c->fidmap, fid);

	if (NULL == n) {
		return NULL;
	}

	return dill_cont(n, R9fid, node);
}

R9fid *allocFid(R9client * c, C9fid fid) {
	R9fid *f = NULL;
	struct dill_rbtree_item *n;

	if (dill_rbtree_empty(&c->fidfree)) {
		// TODO range check
		assert(c->nextFid < 128);
		f = c->fids + c->nextFid;
		c->nextFid++;
	} else {
		n = dill_rbtree_first(&c->fidfree);
		dill_rbtree_erase(&c->fidfree, n);

		f = dill_cont(n, R9fid, node);
	}

	if (NULL == f) {
		return NULL;
	}

	resetFid(f, fid);

	dill_rbtree_insert(&c->fidmap, fid, &f->node);

	return f;
}

void releaseFid(R9client * c, R9fid * f) {
	struct dill_rbtree_item *n;
	int64_t id = 1;

	dill_rbtree_erase(&c->fidmap, &f->node);

	n = dill_rbtree_first(&c->fidfree);

	if (n) {
		id = n->val + 1;
	}

	if (f->isOpen && f->file && f->file->ev && f->file->ev->on_clunk) {
		f->file->ev->on_clunk(f);
	}

	f->file = NULL;

	dill_rbtree_insert(&c->fidfree, id, &f->node);
}
