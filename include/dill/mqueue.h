#pragma once

#include <dill/util_list.h>

// Pending
// - write
// - pushfid
// - clunk
// - flush
// - respond

struct dill_mqueue {
	struct dill_hvfs hvfs;
	struct dill_msock_vfs mvfs;

	Bit lossless : 1;
	Bit singleResponse : 1;
	Bit advanced : 1;

	uint16_t sz;
	uint16_t keep, readers;
	char *pad, *tail;

	dill_slist start, last, set[];
};

struct dill_mqueue_reader {
	struct dill_hvfs hvfs;
	struct dill_msock_vfs mvfs;

	struct dill_mqueue *source;
	dill_slist *pos;
	uint16_t sz;
};

// MQUEUE
handle mqueue_new();

int mqueue_has_readers();

// READER
handle mqueue_new_reader(handle);
