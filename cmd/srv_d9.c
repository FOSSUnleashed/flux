#include <new.h>
#include <r9.h>
#include <dill/all.h>
#include <flux/list.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>

// includes newline
#define UUID_SIZE 37

char UN[] = "R", buf[1 << 8];

List new_files;

#define ST_DEFAULTS \
	.uid	= UN, \
	.muid	= UN, \
	.gid	= UN

typedef struct {
	R9file f;
	List node;
	char name[];
} R9new_file;

void new_file_read(R9fid *f, C9tag tag, uint64_t offset, uint32_t sz) {
	s9read(&f->s->c->ctx, tag, "TODO\n", 5);
}

R9fileEv new_file_ev = {
	.on_shortread	= new_file_read
	// on_write
	// on_clunk
};

R9file *new_file(R9fid *f, const char * name, uint32_t mode) {
	int sz = strlen(name);
	C9stat *st;
	R9new_file *nf;

	nf = calloc(1, sizeof(R9new_file) + 1 + sz);

	if (NULL == nf) {
		return NULL;
	}

	f->file = &nf->f;

	st = &f->file->st;

	f->file->mem = 1;

	st->name = nf->name;
	strcpy(st->name, name);
	st->uid	= UN;
	st->gid	= UN;
	st->muid	= UN;
	st->mode	= 0400;
	st->qid.path = 3;

	printf("[%016llx] %s\n", nf->name, st->name);

	nf->f.ev	= &new_file_ev;

	dill_list_insert(&nf->node, &new_files);

	return &nf->f;
}

// /
// /uuid

R9fileEv newEv = {
	.on_create = new_file
};

R9file root = {
	.st = {
		.name	= ".",
		.qid	= {.type = C9qtdir},
		.mode	= C9stdir | 0500,
		ST_DEFAULTS
	}
}, new_d = {
	.st = {
		.name = "new",
		.qid	= {.type = C9qtdir, .path = 1},
		.mode	= C9stdir | 0700,
		ST_DEFAULTS
	}
	,.ev = &newEv
}, storage_d = {
	.st = {
		.name = "storage",
		.qid	= {.type = C9qtdir, .path = 2},
		.mode	= C9stdir | 0500,
		ST_DEFAULTS
	}
};

void uuid_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	int fd;

	// TODO: generate in process without blocking badly
	fd = open("/proc/sys/kernel/random/uuid", 0);

	if (-1 != fd) {
		read(fd, buf, UUID_SIZE);
		close(fd);
	} else {
		s9error(&f->s->c->ctx, tag, "Could not generate UUID");
		return;
	}

	s9read(&f->s->c->ctx, tag, buf, UUID_SIZE);
}

R9fileEv uniqEv = {
	.on_shortread = uuid_read
};

R9file uniq_id = {
	.st = {
		.name = "uuid",
		.mode	= 0400,
		.size	= UUID_SIZE,
		ST_DEFAULTS
	}
	,.ev	= &uniqEv
};

R9file *r9seek_tmp(R9file * f, R9session * s, const char * name) {
	if (NULL == name && NULL == f) {
		return &root;
	}

	if (&root == f) {
		if (!strcmp(uniq_id.st.name, name)) {
			return &uniq_id;
		} else if (!strcmp(new_d.st.name, name)) {
			return &new_d;
		} else if (!strcmp(storage_d.st.name, name)) {
			return &storage_d;
		}
	}

	if (&new_d == f) {
		List * it;
		R9new_file *cur;

		dill_list_T_foreach(&new_files, typeof(*cur), node, it, cur) {
			if (!strcmp(cur->f.st.name, name)) {
				return &cur->f;
			}
		}
	}

	return NULL;
}

int r9list_tmp(R9fid * f, C9stat ** st) {
	if (&root == f->file) {
		st[0] = &new_d.st;
		st[1] = &storage_d.st;
		st[2] = &uniq_id.st;

		return 3;
	}

	if (&new_d == f->file) {
		List * it;
		R9new_file *cur;
		int i = 0;

		dill_list_T_foreach(&new_files, typeof(*cur), node, it, cur) {
			st[i] = &cur->f.st;
			++i;
		}

		return i;
	}

	return 0;
}

int main(void) {
	int rc = 0;
	rc += mkdirp("/home/R/dream/storage");
	rc += mkdirp("/home/R/dream/staging");
	rc += mkdirp("/home/R/dream/meta");

	printf("RC: %d\n", rc);

	dill_list_init(&new_files);

	handle srv, cli;
	struct ipaddr addr;
	R9client *c;

	R9srv * srv9 = flux_r9getMainSrv();
	flux_r9srvInit(srv9, r9seek_tmp, r9list_tmp);

	ipaddr_local(&addr, NULL, 5555, 0);

	srv	= tcp_listen(&addr, 10);

	forever {
		cli	= tcp_accept(srv, NULL, -1);

		c = allocClient(srv9, cli, tcp_close);

		if_slow (NULL == c) {
			tcp_close(cli, now() + 400);
			continue;
		}

		go(run(c));
	}

	exit:
	tcp_close(srv, now() + 400);

	return 0;
}
