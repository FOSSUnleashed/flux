#include <assert.h>
#include <dill/all.h>
#include <dill/fastbytes.h>
#include <flux/rpc.h>
#include <flux/dial.h>
#include <flux/str.h>
#include <flux/plumb.h>
#include <stdio.h>

#define MSZ (1 << 15)

struct npState {
	uint64_t msz;
	handle sock, rpc;
	uint8_t *ibuf;
	struct flux_rpcstorage sto;
};

struct npMsg {
	struct flux_rpcmsg msg;
	uint32_t sz;
	uint8_t type;
	uint16_t tag;
	struct npState *state;
	uint8_t *data, *edata;
};

struct npQid {
	uint8_t type;
	uint32_t ver;
	uint64_t path;
};

typedef struct npState npState;
typedef struct npMsg npMsg;

npMsg _msg[16];
int hexdump = 1;

uint8_t hexbuf[MSZ], *hexe = hexbuf + 1024;
Buffer hex = BUFLIT(hexbuf);

static npState *getState(handle rpc) {
	struct flux_rpcstorage *sto = dill_hquery(rpc, flux_rpc_type);

	return dill_cont(sto, npState, sto);
}

static void copyQid(const uint8_t *b, struct npQid *qid) {
	if (NULL == qid) return;

	qid->type = *b;
	qid->ver	= ru32(b + 1);
	qid->path	= ru64(b + 5);
}

npMsg *allocMsg() {
	int i;

	if (!_msg->tag) { // we haven't setup the messages
		for (i = 0; i < 16; i++) {
			_msg[i].tag	= i ? i : 0xFFFF;
			_msg[i].type	= 0;
		}
	}

	for (i = 0; i < 16; ++i) {
		if (!_msg[i].type) {
			return _msg + i;
		}
	}

	return NULL;
}

void freeMsg(npMsg *msg) {
	msg->type = 0;
}

static uint8_t *writeHeader(uint8_t *buf, uint32_t sz, uint8_t type, uint16_t tag) {
	wu32(buf, sz);
	wu8(buf + 4, type);
	wu16(buf + 5, tag);

	return buf + 7;
}

int npRead(npState *st, npMsg **pmsg, uint8_t *buf) {
	int rc;
	npMsg *msg;
	struct flux_rpcmsg *rmsg;
	uint8_t *he;

	rc = brecv(st->sock, buf, 7, -1);

//	he = flux_bufdump(hexbuf, hexe, buf, buf + 7);
//	printf("%.*sREAD: %d %d\n", he - hexbuf, hexbuf, rc, errno);

	if (-1 == rc) {
		return -1;
	}

	rmsg	= flux_rpcget(st->rpc, ru16(buf + 5));
	msg	= dill_cont(rmsg, npMsg, msg);

	if (NULL == msg) {
		errno = EINVAL;
		return -1;
	}

	msg->sz	= ru32(buf);
	msg->type	= buf[4];
	msg->tag	= ru16(buf + 5);
	msg->data	= buf + 7;
	msg->edata	= buf + msg->sz;

	*pmsg = msg;

	rc = brecv(st->sock, buf + 7, msg->sz - 7, -1);

	if (hexdump) {
		Buffer b = {buf, buf + msg->sz};
		he = flux_bufdump(hex, b);
		printf("%.*sREAD: %d %d\n", he - hexbuf, hexbuf, rc, errno);
	}

	return rc;
}

static int rpcsend(npState *st, npMsg **_msg, uint8_t *buf, uint8_t *be, uint8_t type) {
	npMsg *msg = allocMsg();
	int rc;
	uint8_t *he;

	if (NULL == msg) {
		errno = ENOMEM;
		return -1;
	}

	*_msg = msg;

	writeHeader(buf, be - buf, type, msg->tag);

	if (hexdump) {
		Buffer b = {buf, be};
		he = flux_bufdump(hex, b);
		printf("%.*s", he - hexbuf, hexbuf);
	}

	rc = bsend(st->sock, buf, be - buf, -1);

	if (-1 == rc) {
		return -1;
	}

	// wait
	return flux_rpcwait(st->rpc, msg->tag, &msg->msg, -1);
}

// version
// auth
// attach

int rpcattach(handle rpc, uint32_t fid, uint32_t afid, uint8_t *uid, uint8_t *aname, struct npQid *qid) {
	npState *st = getState(rpc);
	npMsg *msg;
	uint8_t *b;

	if (NULL == st) {
		errno = EINVAL;
		return -1;
	}

	b = st->ibuf + 7;
	// hdr[7] fid[4] afid[4] uid[s] aname[s]
	wu32(b, fid);
	wu32(b + 4, afid);
	b += 8;
	b += ws(b, "R", 1); // uid
	wu16(b, 0); // aname
	b += 2;

	rpcsend(st, &msg, st->ibuf, b, 104); // blocks

	copyQid(msg->data, qid);
	printf("QID: %02x %08x %016llx\n", b[0], ru32(b + 1), ru64(b + 5));

	// TODO handle Rerror

	return 0;
}

int rpcwalk(handle rpc, uint32_t ofid, uint32_t nfid, uint8_t **path, struct npQid **qids) {
	npState *st = getState(rpc);
	npMsg *msg;
	uint8_t *b, *p;
	int isclone = NULL == path || NULL == *path, counter;

	if (NULL == st) {
		errno = EINVAL;
		return -1;
	}

	b = st->ibuf + 7;
	// hdr[7] ofid[4] nfid[4] n[2] path[n*s]
	wu32(b, ofid);
	wu32(b + 4, nfid);
	b += 8;

	if (isclone) {
		wu16(b, 0);
		b += 2;
	} else {
		p = b;
		counter = 0;

		b += 2;

		while (*path) {
			b += ws(b, *path, strlen(*path));
			path++;
			++counter;
		}

		wu16(p, counter);
	}

	rpcsend(st, &msg, st->ibuf, b, 110); // blocks

	if (111 != msg->type) {
		return -1;
	}

	if (NULL != qids) {
		b	= msg->data + 2;

		while (b < msg->edata && *qids) {
			copyQid(b, *qids);
			b += 13;
			qids++;
		}
	}

	return 0;
}

int rpcwalk1(handle rpc, uint32_t ofid, uint32_t nfid, uint8_t *file, struct npQid *qid) {
	uint8_t *path[] = {file, NULL};

	return rpcwalk(rpc, ofid, nfid, path, &qid);
}

int rpcopen(handle rpc, uint32_t fid, uint8_t mode, struct npQid *qid, uint32_t *iounit) {
	npState *st = getState(rpc);
	npMsg *msg;
	uint8_t *b;

	if (NULL == st) {
		errno = EINVAL;
		return -1;
	}

	b = st->ibuf + 7;
	// hdr[7] fid[4] mode[1]
	wu32(b, fid);
	b[4] = mode;
	b += 5;

	rpcsend(st, &msg, st->ibuf, b, 112); // blocks

	if (113 != msg->type) {
		return -1;
	}

	copyQid(msg->data, qid);

	if (NULL != iounit) {
		*iounit = ru32(msg->data + 13);
	}

	return 0;
}

int rpcread(handle rpc, uint32_t fid, uint64_t offset, uint32_t sz, uint8_t **data, uint8_t **edata) {
	npState *st = getState(rpc);
	npMsg *msg;
	uint8_t *b;

	if (NULL == st) {
		errno = EINVAL;
		return -1;
	}

	b = st->ibuf + 7;
	// size[4] Tread tag[2] fid[4] offset[8] count[4]
	// size[4] Rread tag[2] count[4] data[count]
	wu32(b, fid);
	wu64(b + 4, offset);
	wu32(b + 12, sz);

	rpcsend(st, &msg, st->ibuf, b + 16, 116); // blocks

	if (117 != msg->type) {
		return -1;
	}

	if (NULL != data) {
		*data = msg->data + 4;
	}

	if (NULL != edata) {
		*edata = msg->edata;
	}

	return 0;
}

int rpcreadcopy(handle rpc, uint32_t fid, uint64_t offset, uint8_t *data, uint8_t *edata) {
	Buffer b;
	int rc;

	if (data >= edata) {
		errno = EINVAL;
		return -1;
	}

	rc = rpcread(rpc, fid, offset, edata - data, &b.start, &b.end);

	if (-1 == rc) {
		return -1;
	}

	flux_bufcpy((Buffer){data, edata}, b);

	return rc;
}

// error
// read
// write
// clunk
// remove
// flush
// stat
// wstat
// walk
// open
// create

#include <stdlib.h> // system()
#include <time.h>

#define BG256(x)	("\x1b[48;5;" #x "m")
#define FG256(x)	("\x1b[38;5;" #x "m")
#define CLEARCODES	"\x1b[0m"

#define SMALLBUF	(1 << 6)


coroutine void run(handle rpc) {
	int rc;
	struct npQid qid;
	uint32_t ofid = 0, nfid = 1;
	uint8_t *b, *be;
	pmsg p;
	char timeline[SMALLBUF];

	time_t now = time(NULL);
	struct tm* timeinfo = localtime(&now);

	rc = rpcwalk1(rpc, ofid, nfid, "pane", &qid);

	if (-1 == rc) {
		return;
	}

	printf("QID: %02x %08x %016llx\n", qid.type, qid.ver, qid.path);

	rc = rpcopen(rpc, nfid, 0, &qid, NULL);

	if (-1 == rc) {
		return;
	}

	printf("QID: %02x %08x %016llx\n", qid.type, qid.ver, qid.path);

	hexdump = 0;

	forever {
		rc = rpcread(rpc, nfid, 0, MSZ - 32, &b, &be);

		if (-1 == rc) {
			return;
		}

		rc = parse_pmsg(&p, b, be);

		if (-1 == rc) {
			printf("parse PMG ERRNO %d\n", errno);
			return;
		}

		system("reset");
		strftime(timeline, SMALLBUF, "%Y-%m-%d %H:%M:%S", timeinfo);
		printf("%s%s %-77s%s\n%s", FG256("255"), BG256("240"), timeline, CLEARCODES, CLEARCODES);
		printf("%.*s\n", p.sz, p.data);
	}
}

coroutine void pump(handle rpc, npState *st) {
	npMsg *msg;
	int rc;

	forever {
		rc = npRead(st, &msg, st->ibuf);

		if (-1 == rc) {
			assert(ECANCELED == errno);
			return;
		}

		flux_rpcemit(st->rpc, &msg->msg);
	}
}

npState *nextState; // TODO

handle setup(
	uint8_t *addr
	,uint8_t *eaddr
	,uint8_t *uname
	,uint8_t *euname
	,uint8_t *aname
	,uint8_t *eaname
	,int64_t deadline
) {
	handle rpc = -1, sock = dial(addr, eaddr, NULL, deadline), cr;
	uint8_t *ib;
	npMsg *msg;

	if (-1 == sock) {
		return -1;
	}

	rpc = flux_mrpc_mem(&nextState->sto);

	if (-1 == rpc) {
		// TODO: use correct close function
		ipc_close(sock, deadline);
		return -1;
	}

	nextState->sock	= sock;
	nextState->rpc	= rpc;

	cr = go(pump(rpc, nextState));

	flux_rpc_setpump(rpc, cr);

	// VERSION
	ib = nextState->ibuf + 7;

	wu32(ib, MSZ); ib += 4;
	ib += ws(ib, "9P2000", 6);

	rpcsend(nextState, &msg, nextState->ibuf, ib, 100); // Tversion

	if (19 != msg->sz || 101 != msg->type) {
		goto teardown;
	}

	// ATTACH

	ib = nextState->ibuf + 7;

	wu32(ib, 0); // fid == 0
	wu32(ib + 4, 0xFFFFFFFF); // afid == NOFID
	ib += 8;

	ib += ws(ib, "R", 1);
	wu16(ib, 0); // Empty aname
	ib += 2;

	rpcsend(nextState, &msg, nextState->ibuf, ib, 104); // Tattach

	if (105 != msg->type) {
		goto teardown;
	}

	return rpc;

	teardown:
	ipc_close(sock, deadline);
	hclose(rpc);
	nextState->sock = -1;
	return -1;
}

int main(int argc, char **argv) {
	uint8_t ibuf[MSZ];
	npState state;
	uint8_t *dialaddr = "ns!plumb";

	nextState = &state;
	state.ibuf	= ibuf;

	state.rpc = setup(dialaddr, dialaddr + 8, NULL, NULL, NULL, NULL, -1);

	assert(-1 != state.rpc);

	run(state.rpc);

	if (-1 != state.sock) {
		hclose(state.rpc);
		ipc_close(state.sock, -1);
	}

	return 0;
}
