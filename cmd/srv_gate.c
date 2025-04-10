#include <new.h>
#include <r9.h>
#include <dill/all.h>
#include <flux/time.h>
#include <assert.h>

char UN[] = "R";

struct R9tag tags[128];

R9tagAllocator all;

void demo_write(R9fid *f, C9tag tag, uint64_t offset, uint32_t size, uint8_t *data) {
	flux_r9tagFlushAll(f->file);

	// Claim we consumed everything so that clients don't think they need to resend the junk
	s9write(&f->s->c->ctx, tag, size);
}

void demo_read(R9fid *f, C9tag tag, uint64_t offset, uint32_t size) {
	struct R9tag *cur = NULL;

	cur = r9tagAllocate(f->file, tags);

	if (NULL == cur) {
		s9error(&f->s->c->ctx, tag, "All tags used");
		return;
	}

	flux_r9tagInsert(cur, f, tag, offset, size);
}

#define ST_DEFAULTS \
	.uid	= UN, \
	.gid	= UN, \
	.muid	= UN

R9file root = {
	.st = {
		.qid	= {.type = C9qtdir},
		.size	= 0,
		.name	= ".",
		.mode	= C9stdir | 0500,
		ST_DEFAULTS
	}
};

#define GATELIST(X) \
	X(one)\
	X(two)\
	X(three)\
	X(four)\
	X(five)\
	X(six)\
	X(seven)\
	X(eight)\
	X(nine)\
	X(demo)

#define FILE_ENUM(name) file_##name,
enum {
	GATELIST(FILE_ENUM)
	total_files
};

R9fileEv gateEv = {
	.on_read = demo_read
	,.on_write = demo_write
};

#define FILE_STRUCT(NAME) {\
	.st = {\
		.qid = {.path = file_##NAME},\
		.size	= 0,\
		.name	= #NAME,\
		.mode	= 0600,\
		ST_DEFAULTS\
	}\
	,.ev	= &gateEv\
},
R9file file_list[] = {
	GATELIST(FILE_STRUCT)
	{}
};

int r9list_tmp(R9fid *f, C9stat **st) {
	if (&root == f->file) {
		for (int i = 0; i < total_files; ++i) {
			st[i] = &file_list[i].st;
		}
		return total_files;
	}

	return -1;
}

R9file *r9seek_tmp(R9file *rf, R9session *s, const char *str) {
	if (NULL == rf && NULL == str) {
		return &root;
	}

	for (int i = 0; i < total_files; ++i) {
		if (&root == rf && 0 == strcmp(file_list[i].st.name, str)) {
			return file_list + i;
		}
	}

	return NULL;
}

/*
coroutine void pump() {
	forever {
		msleep(now() + 500);

		;
	}
} // */

int main(void) {
	handle srv, cli;
	struct ipaddr addr;
	R9client *c;
	int i;

	flux_r9tagAllocatorInit(&all);
	ipaddr_local(&addr, NULL, 5553, 0);

	srv	= tcp_listen(&addr, 10);
	R9srv * srv9 = flux_r9getMainSrv();
	flux_r9srvInit(srv9, r9seek_tmp, r9list_tmp);

	for (i = 0; i < total_files; ++i) {
		flux_r9fileInit(file_list + i, &all);
		file_list[i].st.atime	= flux_s();
		file_list[i].st.mtime	= flux_s();
	}

	forever {
		cli	= tcp_accept(srv, NULL, -1);

		c = allocClient(srv9, cli, tcp_close);

		if_slow (NULL == c) {
			tcp_close(cli, now() + 400);
			break;
		}

		go(run(c));
	}

	tcp_close(srv, now() + 400);

	return 0;
}
