#include <r9.h>
#include <flux/util.h>
#include <assert.h>

// tag allocate
// gate flush
// gate clear client function
// gate read ev helper -> insert gate tag
// gate clunk ev helper -> remove gate tag

void flux_r9fileInit(R9file *rf, R9tagAllocator *all) {
	dill_list_init(&rf->open);
	rf->all = all;
}

void flux_r9tagAllocatorInit(R9tagAllocator *alloc) {
	dill_list_init(&alloc->free);
	alloc->count = 0;
}

R9tag *flux_r9tagAllocate(R9tagAllocator *alloc, R9tag *tag, R9tag *tagend) {
	List *fr = &alloc->free, *it;
	R9tag *nx;

	if (dill_list_empty(fr)) {
		nx = tag + alloc->count;

		if (nx >= tagend) {
			// TODO: allocate more memory
			return NULL;
		}

		alloc->count++;
	} else {
		it = dill_list_next(fr);
		dill_list_erase(it);

		nx = flux_containerof(it, R9tag, list);
	}

	return nx;
}

void flux_r9tagInsert(R9tag *cur, R9fid *fid, C9tag tag, uint64_t offset, uint32_t size) {
	cur->tag = tag;
	cur->f	= fid;
	cur->size	= size;
	cur->offset	= offset;

	dill_list_insert(&cur->list, &fid->file->open);
}

void flux_r9tagFree(R9tagAllocator *all, List *it) {
	// pop out of list
	dill_list_erase(it);
	// insert into free list
	dill_list_insert(it, &all->free);
}

List *flux_r9tagFlush(R9tagAllocator *all, R9tag *cur) {
	List * prev = cur->list.prev;

	flux_r9tagFree(all, &cur->list);

	// send EOF
	s9read(&cur->f->s->c->ctx, cur->tag, NULL, 0);

	assert(prev->prev != &cur->list);
	assert(prev->next != &cur->list);
	assert(cur->list.next != prev);
	assert(cur->list.prev != prev);
	assert(cur->list.next != cur->f->file->open.next);
	assert(cur->list.prev != cur->f->file->open.next);
	assert(cur->list.next != cur->f->file->open.prev);
	assert(cur->list.prev != cur->f->file->open.prev);

	// `it` gets invalidated, so we need to reset `it` to a somewhat valid value
	return prev;
}

void flux_r9tagFlushAll(R9file *gf) {
	List *it;
	R9tag *cur;

	r9tag_foreach(&gf->open, it, cur) {
		it = flux_r9tagFlush(gf->all, cur);
	}
}

void flux_r9tagFlushClient(R9file *gf, R9client *c) {
	List *it;
	R9tag *cur;

	// TODO: no need to send EOF

	r9tag_foreach(&gf->open, it, cur) {
		if (c == cur->f->s->c) {
			it = flux_r9tagFlush(gf->all, cur);
		}
	}
}

void flux_r9gateClunkSession(R9fid *f) {
	; // TODO
}

void flux_r9gateClunk(R9fid *f) {
	List *head = &f->file->open, *it;
	R9tag *cur;

	r9tag_foreach(head, it, cur) {
		if (cur->f == f) {
			flux_r9tagFree(f->file->all, it);
			return;
		}
	}
}
