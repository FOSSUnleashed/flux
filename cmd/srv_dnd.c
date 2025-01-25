#include <r9.h>
#include <time.h>
#include <dill/all.h>
#include <flux/time.h>

#include <stdio.h>

char UN[] = "R", buf[1 << 14];

void player_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	// Just EOF for testing
	s9read(&f->s->c->ctx, tag, NULL, 0);
}

int player_write(R9fid* f, uint32_t size, uint8_t *buf, char ** errstr) {
	printf("%.*s\n", size, buf);

	return 0;
}

#define ST_DEFAULTS \
	.uid	= UN, \
	.gid	= UN, \
	.muid	= UN

R9fileEv playerEv = {
	.on_shortread	= player_read
	,.on_linewrite	= player_write
};

R9file root = {
	.st = {
		.qid	= {.type = C9qtdir},
		.size	= 0,
		.name	= ".",
		.mode	= C9stdir | 0500,
		ST_DEFAULTS
	}
}, player = {
	.st = {
		.qid	= {},
		.size	= 0,
		.name	= "player",
		.mode	= 0600,
		ST_DEFAULTS
	}
	,.ev	= &playerEv
};

int r9list_tmp(R9fid *f, C9stat **st) {
	if (&root == f->file) {
		st[0] = &player.st;
		return 1;
	}

	return -1;
}

R9file *r9seek_tmp(R9file *rf, R9session *s, const char *str) {
	if (NULL == rf && NULL == str) {
		return &root;
	}

	if (&root == rf) {
		if (0 == strcmp(str, player.st.name)) {
			return &player;
		}
	}

	return NULL;
}

int main(void) {
	handle srv, cli;
	struct ipaddr addr;
	R9client *c;
	int i;

	R9srv * srv9 = flux_r9getMainSrv();
	flux_r9srvInit(srv9, r9seek_tmp, r9list_tmp);

	ipaddr_local(&addr, NULL, 5555, 0);

	srv	= tcp_listen(&addr, 10);

	forever {
		cli	= tcp_accept(srv, NULL, -1);

		c = allocClient(srv9, cli);

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
