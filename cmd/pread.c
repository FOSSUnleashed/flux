#include <flux/t9.h>
#include <stdio.h>
#include <dill/ipc.h>
#include <string.h>
#include <flux/plumb.h>
#include <errno.h>
#include <args.h>
#include <flux/dial.h>
#include <flux/str.h>

int dead = 0, flags = 0;
char *argv0;

void print_pmsg(pmsg *p) {
	printf("src: %s\n", p->src);
	printf("dst: %s\n", p->dst);
	printf("cwd: %s\n", p->cwd);
	printf("type: %s\n", p->type);
	printf("attr: %s\n", p->attr);
	printf("data[%d]: %.*s\n", p->sz, p->sz, p->data);
}

void T9R(C9ctx *ctx, C9r *r) {
	T9client * c = dill_cont(ctx, T9client, ctx);
	pmsg m;
	int rc;

	if (Rread == r->type) {
		rc = parse_pmsg(&m, r->read.data, r->read.data + r->read.size);

		// TODO: xs serialization mode
		if (-1 == rc) {
			return;
		}

#if 0
		printf("DATA: %016llx %d\n", r->read.data, r->read.size);
		printf("SRC: %016llx %016llx\n", m.src, m.esrc);
		printf("DST: %016llx %016llx\n", m.dst, m.edst);
		printf("CWD: %016llx %016llx\n", m.cwd, m.ecwd);
		printf("TYPE: %016llx %016llx\n", m.type, m.etype);
		printf("ATTR: %016llx %016llx\n", m.attr, m.eattr);
		printf("SIZE: %016llx %d\n", m.size, m.sz);
		printf("DATA: %016llx %016llx\n", m.data, m.edata);
#endif

		if (1 == flags) {
			print_pmsg(&m);
		} else if (0 == flags || 3 == flags) {
			printf("%.*s\n", m.sz, m.data);
			if (flags) dead = 1;
		} else if (2 == flags) {
			write(1, m.data, m.sz);
		}
	} else if (Rerror == r->type) {
		dead = 1;
		printf("ERROR: %s\n", r->error);
	}
}

void usage() {
	dead = 1;

	printf("Usage: %s [-d MODE] [-p PORT] [-u UNIX]\n", argv0);
	printf("Mode values:\n");
	printf("\t0 == infinite newline\n");
	printf("\t1 == debug?\n");
	printf("\t2 == binary stream\n");
	printf("\t3 == newline then die\n");
}

#undef unix

int main(int argc, char **argv) {
	T9client *c;
	handle h;
	struct ipaddr addr;
	C9tag tag;
	char * path[] = {"image", NULL}, *tmp, *unix = NULL;
	Buffer bdial = {};

	ARG({
	case 'd':
		tmp = EARGF(usage());
		flags = *tmp - '0';
		break;
	case 'a':
		tmp	= EARGF(usage());
		bdial	= string2buffer(tmp, 512);
		break;
	case 'p':
		path[0] = EARGF(usage());
		break;
	case 'u':
		unix	= EARGF(usage());
		break;
	default:
		usage();
	});

	if (3 < flags) {
		usage();
		return 1;
	}

	if (dead) {
		return 1;
	}

//	ipaddr_local($);

	if (unix) {
		h = ipc_connect(unix, -1);
	} else {
		if (NULL == bdial.start) {
			bdial	= BUFLIT("ns!plumb");
		}

		h = flux_dial(bdial.start, bdial.end, NULL, -1);
	}

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
