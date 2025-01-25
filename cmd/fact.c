#include <flux/t9.h>
#include <dill/ipc.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

int dead = 0;

enum F9read {
	F9rdok,
	F9rderror,
	F9rddone
};

coroutine void pump(C9ctx *ctx) {
	forever {
		c9proc(ctx);
		msleep(now() + 100);
	}
}

void T9R(C9ctx *ctx, C9r *r) {
	C9tag t;

	if (Rread == r->type) {
		if (0 == r->read.size) {
			dead = 1;
			return;
		}

		if (4 == r->read.size && !memcmp(r->read.data, "done", 4)) {
			dead = 1;
		}

		printf("[%d] %.*s\n", r->read.size, r->read.size, r->read.data);

		if (!dead) {
			c9write(ctx, &t, 1, 0, "read", 4);
		}
	} else if (Rwrite == r->type) {
		c9read(ctx, &t, 1, 0, 512);
	}
}

int main(int argc, char **argv) {
	handle h;
	T9client *c;
	C9tag tag = 0xffff;
	char * ctl_path[] = {"ctl", NULL}, *rpc_path[] = {"rpc", NULL};

	h = ipc_connect("/tmp/ns.R.:1/factotum", -1);

	c = t9openClient(h);

	if_slow (NULL == c || -1 == h) {
		return 1;
	}

	// C9ctx
	c->ctx.r = T9R;

	go(pump(&c->ctx));

	c9version(&c->ctx, &tag, T9_MSZ);
	msleep(now() + 500);

	c9attach(&c->ctx, &tag, 0, C9nofid, "R", "");

	c9walk(&c->ctx, &tag, 0, 1, rpc_path);

	c9open(&c->ctx, &tag, 1, 2); // open read/wr

	c9write(&c->ctx, &tag, 1, 0, "start proto=pass role=client user=testing", 42);

	while (!dead) {
		//c9read(&c->ctx, &tag, 1, 0, 512);
		msleep(now() + 50);
	}

	ipc_close(h, -1);

	return 0;
}
