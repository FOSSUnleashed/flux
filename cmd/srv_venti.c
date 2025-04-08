#include <dill/tcp.h>
#include <stdio.h>
#include <assert.h>
#include <flux/str.h>
#include <dill/suffix.h>
#include <dill/prefix.h>
#include <dill/util.h>
#include <string.h>
#include <errno.h>
#include <flux/list.h>

#include <WjCryptLib_Sha1.h>

#define BSZ (1 << 16)

struct Entry {
	List node;
	union {
		SHA1_HASH score;
		uint32_t _score[5];
	};
	uint8_t type;
	uint32_t sz;
	uint8_t buffer[];
};

typedef struct Entry Entry;

List table;

Entry * allocEntry(uint32_t sz, uint8_t type) {
	Entry *e;

	e = calloc(1, sizeof(Entry) + sz);

	if fast(NULL != e) {
		dill_list_insert(&e->node, &table);

		e->type = type;
		e->sz	= sz;
	}

	return e;
}

void printScore(Entry * e) {
	for (int i = 0; i < 20; i += 2) {
		printf("%02x%02x ", e->score.bytes[i], e->score.bytes[i + 1]);
	}

	printf("%02x %u\n", e->type, e->sz);
}

Entry * lookupEntry(uint8_t * score, uint8_t type) {
	List * it;
	Entry * cur;

	printf("SEEKING: %02x %02x%02x%02x%02x%02x%02x\n", type, score[0], score[1], score[2], score[3], score[4], score[5], score[6]);

	flux_list_foreach(&table, node, it, cur) {
		printScore(cur);
		if (type == cur->type && 0 == memcmp(cur->score.bytes, score, 20)) {
			return cur;
		}
	}

	return NULL;
}

void Verror(handle cli, uint8_t *buffer) {
	buffer[0]	= 1; // Rerror
	buffer[2]	= 0;
	buffer[3]	= 3;
	buffer[4]	= 'W';
	buffer[5]	= 'T';
	buffer[6]	= 'F';

	int rc = msend(cli, buffer, 7, -1);

	assert(-1 != rc);
}

coroutine void process(handle cli) {
	SHA1_HASH sha1;
	int rc;
	char buffer[BSZ];
	Entry *e;
	uint32_t sz;
	Entry dummy;

	forever {
		rc = mrecv(cli, buffer, BSZ, -1);

		if (-1 == rc) {
			if (EPIPE == errno) {
				break;
			}
			printf("%d\n", errno);
			break;
		}

		printf("%d(%d) %d\n", rc, buffer[1], buffer[0]);

/*

013 003 004 005 006 007 008 009
002 003 004 005 006 007 008 009
001 255 255 255 255 255 255 255
255 255 255 255 255 255 255 255
255 255 255 255 255 255 255 255
255 255 255 255 255 255 255 255


255 016 008 009 010 011 012 013
014 015 255 255 255 000 255 255
255 255 255 255 255 255 255 255
255 255 255 255 255 255 255 255
255 255 255 255 255 255 255 255
255 255 255 255 255 255 255 255

// */

		if (14 == buffer[0]) { // Twrite
			Sha1Calculate(buffer + 6, rc - 6, &sha1);

			buffer[0] = 15; // Rwrite

			e = allocEntry(rc - 6, buffer[2]);

			if slow(NULL == e) {
				Verror(cli, buffer);
				continue;
			}

			memcpy(e->score.bytes, sha1.bytes, 20);
			memcpy(e->buffer, buffer + 6, e->sz);

			printScore(e);

			memcpy(buffer + 2, sha1.bytes, 20);

			msend(cli, buffer, 22, -1);
		} else if (12 == buffer[0]) { // Tread
			e = lookupEntry(buffer + 2, buffer[22]);
			sz = (buffer[24] << 8) + buffer[25];

			if (NULL == e) {
				memcpy(dummy.score.bytes, buffer + 2, 20);
				dummy.type = buffer[22];
				dummy.sz = sz;

				printf("DUMMY\n");

				printScore(&dummy);

				Verror(cli, buffer);
				continue;
			}

			printScore(e);

			if (sz > e->sz) {
				sz = e->sz;
			}

			buffer[0] = 13;
			memcpy(buffer + 2, e->buffer, sz);

			msend(cli, buffer, 2 + sz, -1);
		} else if (6 == buffer[0]) { // Tgoodbye
			break;
		} else if (2 == buffer[0] || 16 == buffer[0]) {
			buffer[0]++;

			msend(cli, buffer, 2, -1);
		} else {
			Verror(cli, buffer);
		}
	}
	cli = prefix_detach(cli);
	if (-1 == cli) {
		hclose(cli);
	} else {
		tcp_close(cli, -1);
	}
}

int main(int argc, char **argv) {
	handle srv, cli;
	struct ipaddr addr;
	char buffer[BSZ];
	int rc;

	dill_list_init(&table);

	ipaddr_local(&addr, NULL, 9999, IPADDR_IPV4);

	srv	= tcp_listen(&addr, 10);

	forever {
		cli	= tcp_accept(srv, NULL, -1);

		if slow(-1 == cli) {
			continue;
		}

		cli	= suffix_attach(cli, "\n", 1);

		rc	= mrecv(cli, buffer, BSZ, -1);

		assert(-1 != rc);

		buffer[rc] = 0;

		// TODO: process client negotiation
		// venti-02:04-libventi\n

		rc = msend(cli, "venti-02-libventi", 17, -1);

		assert(-1 != rc);

		cli	= suffix_detach(cli, -1);
		cli	= prefix_attach(cli, 2, 0);

		// <hello>

		rc = mrecv(cli, buffer, BSZ, -1);

		assert(-1 != rc);

		printf("%d(%d) %d\n", rc, buffer[1], buffer[0]);

		buffer[0]	= 5; // RHello
		// buffer[1] // tag
		buffer[2]	= 0;
		buffer[3]	= 2;
		buffer[4]	= 'H';
		buffer[5]	= 'I';
		buffer[6]	= 0;
		buffer[7]	= 0;

		msend(cli, buffer, 8, -1);

		// </hello>

		go(process(cli));
	}

	return 0;
}
