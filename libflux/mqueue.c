#include <dill/all.h>
#include <dill/impl.h>
#include <flux/mq.h>
#include <flux/time.h>
#include <string.h>

#include <assert.h>
#include <stdio.h>

#define MSG_NUM (1 << 7)

typedef flux_mq mQ;
typedef flux_mq_message mQ_msg;

typedef struct {
	mQ mq;
	flux_mq_message msg[MSG_NUM];
	flux_mq_reader rdr[MSG_NUM];
	struct dill_list free_msg, free_rdr;
	uint16_t nmsg, nrdr;
} fatmq;

void flux_mq_init(mQ *mq) {
	// assume mq was zero'd
	dill_list_init(&mq->readers);
	dill_list_init(&mq->messages);
}

void flux_mq_zero(mQ *mq) {
	bzero(mq, sizeof(mQ));
}

mQ *flux_mq_alloc(size_t sz) {
	fatmq *mq;

	mq = calloc(1, sizeof(fatmq) + sz * sizeof(char));
	mq->mq.buf = (char *)(mq + 1);

	if_slow (NULL == mq) {
		goto end;
	}

	flux_mq_init(&mq->mq);
	dill_list_init(&mq->free_msg);
	dill_list_init(&mq->free_rdr);

	end:
	return &mq->mq;
}

static void add_msg(mQ *mq, mQ_msg *msg) {
	dill_list_init(&msg->node);
	dill_list_insert(&msg->node, &mq->messages);

	mq->nmsg++;
}

static flux_mq_message *alloc_msg(fatmq *mq) {
	struct dill_list * node;
	flux_mq_message *msg = NULL;

	if (dill_list_empty(&mq->free_msg)) {
		if (mq->nmsg < MSG_NUM) {
			msg = mq->msg + mq->nmsg;
			mq->nmsg++;
			node = &msg->node;
			add_msg(&mq->mq, msg);
		}
	} else {
		node = dill_list_next(&mq->free_msg);
		dill_list_erase(node);
		msg = dill_cont(node, flux_mq_message, node);
		add_msg(&mq->mq, msg);
	}

	return msg;
}

int flux_mq_write(mQ *mq, const char * const str, ssize_t sz) {
	fatmq *fmq = dill_cont(mq, fatmq, mq);
	mQ_msg *msg = alloc_msg(fmq);

	if_slow (NULL == msg) {
		return -1;
	}

	if (0 > sz) {
		sz = strlen(str);
	}

	msg->sz = sz;
	msg->tm = flux_us();
	msg->buf	= mq->buf + mq->pos;

	mq->pos += sz;
	mq->nmsg++;

	memcpy(msg->buf, str, sz);

	return 0;
}

static void safe_release_message(fatmq *fmq) {
	struct dill_list *it;
	uint64_t ts = ~0;
	flux_mq_reader *rdr;
	flux_mq_message *msg;

	dill_list_T_foreach(&fmq->mq.readers, flux_mq_reader, node, it, rdr) {
		ts = rdr->last_read > ts ? ts : rdr->last_read;
	}

	dill_list_T_foreach(&fmq->mq.messages, flux_mq_message, node, it, msg) {
		if (msg->tm >= ts) {
			return;
		}

		dill_list_erase(it);
		dill_list_insert(it, &fmq->free_msg);
		fmq->mq.nmsg--;

		// `it` gets invalidated, so we have to reset it to a proper value
		it = &fmq->mq.messages;
	}
}

ssize_t flux_mq_read(flux_mq_reader *rdr, char * buffer, size_t sz) {
	mQ_msg *msg;
	fatmq *fmq;
	ssize_t ssz;
	struct dill_list * node;

	fmq	= dill_cont(rdr->parent, fatmq, mq);

	// Get next message
	node = dill_list_next(rdr->pos);

	if (&fmq->mq.messages == node) {
		// Next message was the head
		return 0;
	}

	msg = dill_cont(node, flux_mq_message, node);

	// Copy message into buffer
	ssz = sz > msg->sz ? msg->sz : sz;

	memcpy(buffer, msg->buf, ssz);

	rdr->pos = node;
	rdr->last_read = msg->tm;

	// Free the messages in queue
	safe_release_message(fmq);

	// todo: check ts
	return ssz;
}

void flux_mq_reader_skip(flux_mq_reader *rdr) {
	// Check to see if `rdr->pos = &mq->messages`?

	rdr->pos = rdr->parent->messages.prev;
}

static void add_rdr(mQ *mq, flux_mq_reader *rdr) {
	dill_list_init(&rdr->node);
	dill_list_insert(&rdr->node, &mq->readers);

	mq->nreaders++;

	rdr->pos = &mq->messages;
	rdr->last_read = 0;
	rdr->parent = mq;
}

flux_mq_reader *flux_mq_get_reader(flux_mq *mq) {
	struct dill_list *node;
	flux_mq_reader *rdr = NULL;
	fatmq *fmq = dill_cont(mq, fatmq, mq);

	if (dill_list_empty(&fmq->free_rdr)) {
		if (fmq->nrdr < MSG_NUM) {
			rdr = fmq->rdr + fmq->nrdr;
			fmq->nrdr++;
			node = &rdr->node;
			add_rdr(mq, rdr);
		}
	} else {
		node = dill_list_next(&fmq->free_rdr);
		dill_list_erase(node);
		rdr = dill_cont(node, flux_mq_reader, node);
		add_rdr(mq, rdr);
	}

	return rdr;
}
