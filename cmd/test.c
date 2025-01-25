#include <stdio.h>
#include <dill/all.h>
#include <dill/util_rbtree.h>

typedef Tree dill_rbtree_item;
typedef TreeRoot dill_rbtree;

TreeRoot fids, tags, freefids, freetags;

#define LOL (1 << 9)

// status of connection

typedef struct {
	Tree node;
	uint8_t type;
	union {
		uint32_t clunkfid;
		uint16_t flushtag;
	}
} Tag;

typedef struct {
	Tree node;
	Bit create : 1;
	Bit open : 1;

	uint8_t mode;
	uint32_t perm;

	// path and parent fid
} Fid;

Tag T[LOL], *tp = T, *te = T + LOL;
Fid F[LOL], *fp = F, *fe = F + LOL;

void free_tag(Tag *t) {
	dill_rbtree_erase(&t->node);
	dill_rbtree_insert(&freetags, t->val, &t->node);
}

void free_fid(Fid *f) {
	dill_rbtree_erase(&f->node);
	dill_rbtree_insert(&freefids, t->val, &f->node);
}

Tag * new_tag() {
	Tag * t;

	if (dill_rbtree_empty(&tags)) {
		t = tp++;
		assert(tp < te);
	} else {
		;
	}

	return t;
}

handle dial() {
	struct ipaddr addr;

	ipaddr_local(&addr, "127.0.0.1", 5555, IPADDR_IPV4);

	return tcp_connect(&addr, -1);
}

// VERSION - H
// ATTACH - 
// AUTH - F

// WALK - err while disc
// OEPN - err while disc
// CREAT - err while disc

// READ - EOF while disconnected
// WRITE - 0 write while disconnected || error? xs might choke

// STAT - err while disc?
// WSTAT - err while disc

// REMOVE - err while disc

// CLUNK - handle while disc
// FLUSH - handle while disc

coroutine srvlisten() {
	handle srv, client;
	struct ipaddr addr;
	int rc;

	ipaddr_local(&addr, "127.0.0.1", 3131, IPADDR_IPV4);
	srv = tcp_listen(&addr, 1);

	client = tcp_accept(srv, NULL, -1);

	while (1) {
		;
	}
}

int main(int argc, char **argv) {
	handle h;
	int rc;
	struct ipaddr addr;
	char buffer[1024];

	go(srvlisten());

	h = dial();

	rc = brecv(h, buffer, 512, -1);

	printf("%d %d\n", rc, errno);

	return 0;
}
