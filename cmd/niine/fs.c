#include <dill/tcp.h>
#include <dill/msock.h>
#include <r9.h>
#include <sys/param.h>
#include <string.h>
#include <stdio.h>

char UN[] = "R";

extern handle h;

#define ST_DEFAULTS \
	,.uid = UN\
	,.gid	= UN\
	,.muid	= UN

int in_write(R9fid* f, uint32_t size, uint8_t *buf, char ** errstr) {
	struct iolist iol[2];
	int rc = 0;

	iol[0].iol_base = buf;
	iol[0].iol_len	= size;
	iol[0].iol_next	= iol + 1;
	iol[0].iol_rsvd	= 0;

	iol[1].iol_base = "\r";
	iol[1].iol_len	= 1;
	iol[1].iol_next	= NULL;
	iol[1].iol_rsvd	= 0;

	rc = msendl(h, iol, iol + 1, -1);

	printf("%d %d\n", rc, errno);

	return 0;
}

R9fileEv infev = {
	.on_linewrite = in_write
};

R9file root = {
	.st = {
		.qid	= {.type = C9qtdir}
		,.mode = 0500 | C9stdir
		,.name = "."
		,.size = 0
		ST_DEFAULTS
	}
}, inf = {
	.st = {
		.qid = {.type = C9qtappend}
		,.mode = 0200 | C9stappend
		,.name = "in"
		,.size = 0
		ST_DEFAULTS
	}
	,.ev	= &infev
};

typedef struct {
	R9file f;
	char nick[64], _nick[64];
} NickFile;

void nick_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	NickFile *nf = dill_cont(f->file, NickFile, f);

	if (offset >= nf->f.st.size) {
		s9read(&f->s->c->ctx, tag, NULL, 0);
	}

	s9read(&f->s->c->ctx, tag, nf->nick + offset, MIN(size, nf->f.st.size - offset));
}

void nick_write(R9fid* f, C9tag tag, uint64_t offset, uint32_t size, uint8_t *buf) {
	char msg[64], *p;
	uint32_t sz = size;

	if (size >= 32) {
		s9error(&f->s->c->ctx, tag, "Write too big");
	}

	// TODO: actually validate we didn't get any invalid characters

	p = stpcpy(msg, "NICK :");

	while (0 != *buf && '\n' != *buf && sz > 0) {
		*p++ = *buf++;
		sz--;
	}

	*p++ = '\r';

	msend(h, msg, p - msg, -1);

	s9write(&f->s->c->ctx, tag, size);
}

R9fileEv nickev = {
	.on_read	= nick_read
	,.on_write	= nick_write
};

NickFile nick = {
	.f = {
		.st = {
			.qid	= {.path = 6}
			,.mode	= 0600
			,.name	= "nick"
			,.size	= 0
			ST_DEFAULTS
		}
		,.ev	= &nickev
	}
};

typedef struct {
	R9file f;
	char * b;
} BufFile;

void buf_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	BufFile *bf = dill_cont(f->file, BufFile, f);

	if (offset >= bf->f.st.size) {
		s9read(&f->s->c->ctx, tag, NULL, 0);
	}

	s9read(&f->s->c->ctx, tag, bf->b + offset, MIN(size, bf->f.st.size - offset));
}

R9fileEv bufev = {
	.on_read	= buf_read
};

BufFile f001 = {
	.f = {
		.st = {
			.qid = {.path = 1}
			,.mode	= 0400
			,.name	= "001"
			,.size	= 0
			ST_DEFAULTS
		}
		,.ev	= &bufev
	}
}, f002 = {
	.f = {
		.st = {
			.qid = {.path = 2}
			,.mode	= 0400
			,.name	= "002"
			,.size	= 0
			ST_DEFAULTS
		}
		,.ev	= &bufev
	}
}, f003 = {
	.f = {
		.st = {
			.qid = {.path = 3}
			,.mode	= 0400
			,.name	= "003"
			,.size	= 0
			ST_DEFAULTS
		}
		,.ev	= &bufev
	}
}, f004 = {
	.f = {
		.st = {
			.qid = {.path = 4}
			,.mode	= 0400
			,.name	= "004"
			,.size	= 0
			ST_DEFAULTS
		}
		,.ev	= &bufev
	}
}, f005 = {
	.f = {
		.st = {
			.qid = {.path = 5}
			,.mode	= 0400
			,.name	= "005"
			,.size	= 0
			ST_DEFAULTS
		}
		,.ev	= &bufev
	}
}, fmotd = {
	.f = {
		.st = {
			.qid = {}
			,.mode	= 0400
			,.name	= "motd"
			,.size	= 0
			ST_DEFAULTS
		}
		,.ev	= &bufev
	}
};

int r9list_tmp(R9fid * f, C9stat ** st) {
	if (&root == f->file) {
		*st++ = &fmotd.f.st;
		*st++ = &f001.f.st;
		*st++ = &f002.f.st;
		*st++ = &f003.f.st;
		*st++ = &f004.f.st;
		*st++ = &f005.f.st;
		*st++ = &nick.f.st;
		*st++ = &inf.st;

		return 8;
	}

	return -1;
}

R9file *r9seek_tmp(R9file * f, R9session * s, const char * name) {
	if (NULL == f && NULL == name) {
		return &root;
	}

	if (&root == f) {
		if (0 == strcmp(fmotd.f.st.name, name)) {
			return &fmotd.f;
		} else if (0 == strcmp(f001.f.st.name, name)) {
			return &f001.f;
		} else if (0 == strcmp(f002.f.st.name, name)) {
			return &f002.f;
		} else if (0 == strcmp(f003.f.st.name, name)) {
			return &f003.f;
		} else if (0 == strcmp(f004.f.st.name, name)) {
			return &f004.f;
		} else if (0 == strcmp(f005.f.st.name, name)) {
			return &f005.f;
		} else if (0 == strcmp(nick.f.st.name, name)) {
			return &nick.f;
		} else if (0 == strcmp(inf.st.name, name)) {
			return &inf;
		}
	}

	return NULL;
}

coroutine void listen9() {
	struct ipaddr addr;
	handle srv, cli;
	R9client *c;

	R9srv * srv9 = flux_r9getMainSrv();
	flux_r9srvInit(srv9, r9seek_tmp, r9list_tmp);

	ipaddr_local(&addr, "127.0.0.1", 4444, IPADDR_IPV4);
	srv = tcp_listen(&addr, 10);

	printf("SRV: %d\n", srv);

	forever {
		cli = tcp_accept(srv, NULL, -1);

		if (-1 == cli) {
			break;
		}

		c = allocClient(srv9, cli);

		if (NULL == c) {
			break;
		}

		go(run(c));
	}
}
