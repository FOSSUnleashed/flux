#include <flux/t9.h>
#include <stdio.h>
#include <dill/ipc.h>
#include <string.h>

int dead = 0, flags = 0;

typedef struct {
	char *src, *dst, *cwd, *type, *attr, *size, *data;
	uint32_t sz;
} pmsg;

#define NEXT(B) for (; *b && '\n' != *b; ++b) {B;}; *b = 0; ++b;

void parse_pmsg(pmsg *p, char * data) {
	char * b = data;

	p->sz = 0;

	p->src	= b; NEXT()
	p->dst	= b; NEXT()
	p->cwd	= b; NEXT()
	p->type	= b; NEXT()
	p->attr	= b; NEXT()
	p->size	= b; NEXT(p->sz = p->sz * 10 + *b - '0')
	p->data	= b;
}

void print_pmsg(pmsg *p) {
	printf("src: %s\n", p->src);
	printf("dst: %s\n", p->dst);
	printf("cwd: %s\n", p->cwd);
	printf("type: %s\n", p->type);
	printf("attr: %s\n", p->attr);
	printf("data[%d]: %s\n", p->sz, p->data);
}

void T9R(C9ctx *ctx, C9r *r) {
	T9client * c = dill_cont(ctx, T9client, ctx);
	pmsg m;

	if (Rread == r->type) {
		parse_pmsg(&m, r->read.data);

		// TODO: xs serialization mode

		if (1 == flags) {
			print_pmsg(&m);
		} else if (0 == flags || 3 == flags) {
			printf("%s\n", m.data);
			if (flags) dead = 1;
		} else if (2 == flags) {
			write(1, m.data, m.sz);
		}
	} else if (Rerror == r->type) {
		dead = 1;
		printf("ERROR: %s\n", r->error);
	}
}

int main(int argc, char **argv) {
	T9client *c;
	handle h;
	struct ipaddr addr;
	C9tag tag;
	char * path[] = {"image", NULL};

	if (3 == argc) {
		if (!strcmp("-d", argv[1])) {
			// TODO: be less stupid
			// 0 == infinite newline
			// 1 == debug?
			// 2 == binary stream
			// 3 == newline then die
			flags = argv[2][0] - '0';
		}
	}

	if (3 < flags) {
		printf("0 == infinite newline\n");
		printf("1 == debug?\n");
		printf("2 == binary stream\n");
		printf("3 == newline then die\n");
		return 1;
	}

//	ipaddr_local($);

	h = ipc_connect("/tmp/ns.R.:1/plumb", -1);

	c = t9openClient(h);

	if (-1 == h || NULL == c) {
		printf("Bad client or handle, guess which\n");
		return 1;
	}

	c->ctx.r = T9R;

	c9version(&c->ctx, &tag, T9_MSZ);
	c9proc(&c->ctx);

	c9attach(&c->ctx, &tag, 0, C9nofid, "R", "");
	c9proc(&c->ctx);

	c9walk(&c->ctx, &tag, 0, 1, path);
	c9proc(&c->ctx);

	c9open(&c->ctx, &tag, 1, 0);
	c9proc(&c->ctx);

	while (!dead) {
		c9read(&c->ctx, &tag, 1, 0, 512);
		c9proc(&c->ctx);
	}

	return 0;
}
