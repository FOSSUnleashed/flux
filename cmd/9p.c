#include <stdio.h>
#include <dill/all.h>
#include <flux/list.h>
#include <flux/t9.h>

int run = 2;
char *cloneWalk[] = {NULL};

/*
* [ ] tail -f /log
* [ ] check if creature exists
* [ ] ls
* [ ] read
* [ ] read1 (single read, optional offset?)
* [ ] write
* [ ] rw
* [ ] wr
* [ ] stat
* [ ] mkdir
* [ ] rmdir
* [ ] rm
* [ ] append
* [ ] mktmp
* [ ] settmp
*/

uint8_t *readStat(uint8_t * p, C9stat * st) {
	uint16_t *sz = p;
	uint8_t *end = p + *sz + 2;
	p += 2;

	p += 6; // type and dev

	st->qid.type	= *p; ++p;
	st->qid.version	= *(uint32_t *)p; p += 4;
	st->qid.path	= *(uint64_t *)p; p += 8;

	st->mode	= *(uint32_t *)p; p += 4;
	st->atime	= *(uint32_t *)p; p += 4;
	st->mtime	= *(uint32_t *)p; p += 4;
	st->size	= *(uint64_t *)p; p += 8;

	sz = p;

	st->name	= p + 2;

	p += *sz + 2;
	sz = p;

	st->uid = p + 2;

	p += *sz + 2;
	sz = p;

	st->gid	= p + 2;

	p += *sz + 2;
	sz = p;

	st->muid	= p + 2;

	return end;
}

void printStat(C9stat * st) {
	printf("(file %s\n", st->name);
	printf("\t(qid %02x %08x %016llx) ", st->qid.type, st->qid.version, st->qid.path);
	printf("(mode %08x)\n", st->mode);
	printf("\t(size: %d) ", st->size);
	printf("(time %d %d) ", st->atime, st->mtime);
	printf("(id %s %s %s)\n)\n", st->uid, st->gid, st->muid);
}

void T9R(C9ctx * ctx, C9r * R) {
	T9client * c = dill_cont(ctx, T9client, ctx);
	C9tag _tag, *tag = &_tag;
	union {uint32_t u32; uint8_t u8[4];} data;
	uint8_t *p, *end;
	uint32_t sz;
	C9stat st;

	switch (R->type) {
	case Rversion:
		printf("VERSION: %d\n", ctx->msize);
		break;
	case Rerror:
		printf("%s\n", R->error);
		break;
	case Rwalk:
		c9open(ctx, tag, 1, 0);
		break;
	case Ropen:
		c9read(ctx, tag, 1, 0, 1 << 12);
		break;
	case Rread:
		data.u32 = 0;
		p	= R->read.data;
		sz	= R->read.size;
		end = p + sz;

		while (end > p) {
			data.u8[0] = p[0];
			data.u8[1] = p[1];

			p = readStat(p, &st);
			printStat(&st);
		}
		c9clunk(ctx, tag, 1);
		c9clunk(ctx, tag, 0);
		break;
	case Rclunk:
		--run;
		break;
	}
}

void setup(T9client *c) {
	C9tag _tag, *tag = &_tag;
	C9ctx *ctx = &c->ctx;
	C9fid fid = 1;

	c9version(ctx, tag, 1 << 13);

	c9attach(ctx, tag, 0, C9nofid, "R", "");

	msleep(now() + 50);

	c9walk(ctx, tag, 0, fid, cloneWalk);
}

int main(int argc, char **argv) {
	T9client *c;
	handle h;
	struct ipaddr addr;

	ipaddr_remote(&addr, "127.1", 5555, IPADDR_IPV4, -1);

	h = tcp_connect(&addr, -1);

	c	= t9openClient(h);

	c->ctx.r = T9R;

	go(setup(c));

	while (run) {
		C9error e = c9proc(&c->ctx);

		if (e) {
			printf("C9e: %d\n", e);
			run = 0;
		}
	}

	return 0;
}
