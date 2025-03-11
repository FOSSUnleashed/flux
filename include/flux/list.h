#pragma once

#include <dill/util_list.h>

typedef struct dill_list List;

// free (node)
// init (pool)
// alloc (node)

// TODO: figgure out how to make this work
#define LIST_FOREACH(obj, member) List_##obj##_foreach(head, it, cur) dill_list_T_foreach(head, obj, member, it, cur)

#define flux_list_foreach(head, M, it, cur) dill_list_T_foreach(head, typeof(*cur), M, it, cur)

#define flux_list_unfree(free, it, type, member) (it = dill_list_next((free)), dill_list_erase(it), dill_cont(it, type, member))

/*
	void xPoolInit(xPool*) -- initialize a zero'd pool
	void xPoolZero(xPool*) -- zero out a dirty pool (should not be used on a pool that has been used)

	x *xPoolAllocNode(xPool*, List* head) -- get a new node and optionally add it to head
	xPoolFreeNode(xPool*, x*) -- free a allocated node
// */

#define LIST_POOL(obj, member, sz) \
	typedef struct {\
		List free; \
		uint16_t cnt;\
		obj set[sz];\
	} obj##Pool; \
\
	static inline void obj##PoolInit(obj##Pool *p) {\
		dill_list_init(&p->free);\
	}\
\
	static inline void obj##PoolZero(obj##Pool *p) {\
		p->cnt = 0;\
		obj##PoolInit(p);\
	}\
\
	static inline obj* obj##PoolAllocNode(obj##Pool *p, List *head) {\
		obj *n = NULL;\
		List *it;\
		if (dill_list_empty(&p->free)) {\
			if (sz > p->cnt) {\
				n = p->set + p->cnt;\
				p->cnt++;\
			}\
		} else { \
			it = dill_list_next(&p->free);\
			dill_list_erase(it);\
			n = dill_cont(it, obj, member); \
		}\
		if (!n) { return NULL; }\
		if (head) { dill_list_insert(&n->member, head); }\
		return n;\
	}\
\
	static inline void obj##PoolFreeNode(obj##Pool *p, obj *n) {\
		dill_list_erase(&n->member);\
		dill_list_insert(&n->member, &p->free);\
	}\

