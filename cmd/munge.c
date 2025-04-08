#include <dill/core.h>
#include <dill/ipc.h>
#include <flux/str.h>
#include <stdio.h>
#include <assert.h>
#include <dill/fastbytes.h>

/*





// Rdecode
n += sizeof (m->error_num) u8
n += sizeof (m->error_len) u8
n += m->error_len;
n += sizeof (m->cipher); u8
n += sizeof (m->mac); u8
n += sizeof (m->zip); u8
n += sizeof (m->realm_len) u8
n += m->realm_len;
n += sizeof (m->ttl); u8
n += sizeof (m->addr_len); u8?/u32?
n += m->addr_len;
n += sizeof (m->time0); u32
n += sizeof (m->time1); u32
n += sizeof (m->cred_uid); u32
n += sizeof (m->cred_gid); u32
n += sizeof (m->auth_uid); u32
n += sizeof (m->auth_gid); u32
n += sizeof (m->data_len); u32
n += m->data_len;

*/

#define BSZ (1 << 13)

struct MungeMsg {
	uint8_t status;
	Buffer data, err;
};

typedef struct MungeMsg MungeMsg;

Buffer munge_recv(handle munge, Buffer b, int64_t deadline) {
	uint32_t sz;
	int rc;
	uint8_t _c[BSZ], *p;
	Buffer c = BUFLIT(_c);

	// TODO check b.end against sz and 11

	rc = brecv(munge, b.start, 11, deadline);

	if (-1 == rc) {
		goto error;
	}

	sz = ru32(b.start + 7);
	sz = swap32(sz);

	rc = brecv(munge, b.start + 11, sz, deadline);

	if (-1 == rc) {
		goto error;
	}

	p = flux_bufdump(c.start, c.end, b.start, b.start + 11 + sz);

	printf("%.*s", p - c.start, c.start);

	b.end = b.start + 11 + sz;

	return b;
	error:
	b.start = NULL;
	b.end	= NULL;
	return b;
}

void munge_recv_enc(handle munge, Buffer b, MungeMsg *m, int64_t deadline) {
/*
// Rencode
u8: err num
u8: err len

u32: size
str: message
*/

	uint8_t *p;
	uint32_t sz;
	Buffer e;

	e = munge_recv(munge, b, deadline);

	if (NULL == e.start) {
		return;
	}

	m->status = b.start[11];

	p = b.start + 12; // err len
	if (*p) { // no error
		m->err.start	= NULL;
		m->err.end	= NULL;
	} else {
		m->err.start	= p + 1;
		p += *p + 1; // size[4]
		m->err.end	= p;
	}

	sz = ru32(p);
	sz = swap32(sz);

	p += 4;

	m->data.start	= p;
	m->data.end	= e.end;
}

#define ADDR "/run/munge/munge.socket.2"

int main(int argc, char **argv) {
	handle munge;
	uint8_t _b[BSZ], *p, _c[BSZ];
	Buffer b = BUFLIT(_b), c = BUFLIT(_c), s;
	size_t sz;
	int rc;
	MungeMsg msg;

	munge = ipc_connect(ADDR, -1);

	assert(-1 != munge);

	// Magic[4] Version[1]
	_b[0] = 0;
	_b[1] = 0x60;
	_b[2] = 0x6d;
	_b[3] = 0x4b;
	_b[4] = 0x04;

	// Type[1] 2 -> 3, 4 -> 5
	_b[5] = 2;

	// Attempts[1]
	_b[6] = 0;

	p = _b + 11;

	//
	// Tencode
	//
	/*

// Tencode
u8: cypher (01)
u8: mac (01)
u8: zip (01)
u32: realm length (0000 0000)
u8: TTL (00 == Default)

u32:	-1 == any uid
u32:	-1 == any gid

u32:	size of message

str:	message

*/
	// Cyper[1] 1 == use server defaults
	p[0] = 1;
	// Mac[1] 1 == use server defaults
	p[1] = 1;
	// Zip[1] 1== use server defaults, 0 == no compression
	p[2] = 1;
	// Realm[4 + s] Not used
	wu32(p + 3, 0);

	// TTL[1] 0 == 300 | otherwise TTS is the number of seconds
	p[7] = 10;

	// uid[4]
	wu32(p + 8, swap32(10000));
	// gid[4]
	wu32(p + 12, 0xFFFFFFFF);

	// msg[4 + s]
	wu32(p + 16, swap32(10));
	p += 20;
	*p++ = 'f';
	*p++ = 'o';
	*p++ = 'o';
	*p++ = '\n';
	*p++	= 0;
	*p++	= 'l';
	*p++ = 'o';
	*p++ = 'l';
	*p++ = '\n';
	*p++	= 0;

	// size[4]
	sz = p - _b - 11;
	wu32(_b + 7, swap32(sz));

	rc = bsend(munge, _b, p - _b, -1);

	assert(-1 != rc);

	munge_recv_enc(munge, b, &msg, -1);

	printf("%s\n", msg.data);

	c = flux_bufcpy(c, msg.data);

	// type[1]
	_b[5]	= 4;

	// attempts[1]
	_b[6] = 0;

/*

// Tdecode
u32: size
str: message
*/

	p = _b + 11;
	sz = msg.data.end - msg.data.start - 1;
	wu32(p, swap32(sz));
	s.start	= b.start + 15;
	s.end	= b.end;
	s = flux_bufcpy(s, msg.data);
	p = s.end;

	// size[4]
	sz = p - _b - 12;
	wu32(_b + 7, swap32(sz));

	ipc_close(munge, -1);
	munge = ipc_connect(ADDR, -1);

	rc = bsend(munge, _b, p - _b, -1);

	printf("%d\n", errno);

	assert(-1 != rc);

	munge_recv(munge, b, -1);

	return 0;
}
