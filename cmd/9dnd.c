#include <flux/str.h>
#include <flux/t9.h>
#include <unistd.h>
#include <assert.h>
#include <dill/tcp.h>
#include <dill/all.h>
#include <stdio.h>
#include <flux/util.h>

uint64_t offset = 0;

void T9R(C9ctx *ctx, C9r *r) {
	T9client *c = dill_cont(ctx, T9client, ctx);

	switch (r->type) {
	case Rread:
		if (0 == r->read.size) {
			msleep(now() + 500);
			return;
		}
		write(1, r->read.data, r->read.size);
		offset += r->read.size;
		break;
	case Rerror:
		fprintf(stderr, "ERROR: %s\n", r->error);
		break;
	default:
		printf("Message: %d\n", r->type);
	}
}

coroutine void setup(T9client *c) {
	C9tag tag;
	C9fid fidroot = 0, fidlog = 1;
	uint8_t *walkPath[] = {"log", NULL};

	c9version(&c->ctx, &tag, 1 << 13);

	c9attach(&c->ctx, &tag, fidroot, C9nofid, "R", "");

	c9walk(&c->ctx, &tag, fidroot, fidlog, walkPath);
	msleep(now() + 500);

	c9open(&c->ctx, &tag, fidlog, 0);
	yield();

	forever {
		c9read(&c->ctx, &tag, fidlog, offset, 1 << 12);
		msleep(now() + 500);
	}
}

int main(int argc, char **argv) {
	handle cli;
	T9client *c;
	struct ipaddr addr;

	ipaddr_remote(&addr, "127.0.0.1", 5555, IPADDR_IPV4, -1);

	cli	= tcp_connect(&addr, -1);

	c = t9openClient(cli);

	assert(c);

	c->ctx.r = T9R;

	go(setup(c));

	while (c) {
		C9error e = c9proc(&c->ctx);
	}

	return 0;
}
