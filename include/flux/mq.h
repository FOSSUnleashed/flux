#pragma once

#include <dill/util_list.h>

// mqueue

typedef struct {
	Bit lossless : 1;
	Bit singletonRespond : 1;
	Bit advanced : 1; // ???

	// keep - How many messages to keep in a queue for new readers?
	// nreaders - number of readers
	// nmsg - number of messages current stored
	uint16_t keep, nreaders, nmsg;
	uint32_t pos;

	struct dill_list readers, messages;

	// pad - message padding between messages when not singleton (only in multi-reads)
	// tail - message padding after a message (always after each read)
	char *pad, *tail, *buf;
} flux_mq;

typedef struct {
	struct dill_list node;
	uint16_t sz;
	uint64_t tm;
	char *buf;
} flux_mq_message;

typedef struct {
	struct dill_list node, *pos;
	flux_mq * parent;

	uint64_t last_read;
} flux_mq_reader;

// PagingQueue - provide a vfs to recover earlier messages instead erroring when reader is behind

// getQueuerReaderFromIndex - get a queue reader from a certain negative index (how does this interact with keep)

void flux_mq_init(flux_mq *mq);
void flux_mq_zero(flux_mq *mq);
flux_mq *flux_mq_alloc(size_t sz);

// TODO: flux_mq_write_fmt
int flux_mq_write(flux_mq *mq, const char * const, ssize_t);
ssize_t flux_mq_read(flux_mq_reader *, char *, size_t);

flux_mq_reader *flux_mq_get_reader(flux_mq *);
void flux_mq_reader_skip(flux_mq_reader *rdr);
