#include <dill/tcp.h>
#include <dill/msock.h>
#include <sys/param.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <flux/str.h>
#include <newniine.h>
#include <flux/util.h>

ircClient *CLIENT_TEMP;

R9tagAllocator all;
R9tag tags[128];

TreeRoot clientTree;

#define ST_DEFAULTS \
	,.uid = cli->UN\
	,.gid	= cli->UN\
	,.muid	= cli->UN

void ready_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	R9tag *tg;

	tg = r9tagAllocate(f->file, tags);

	if (NULL == tg) {
		s9read(&f->s->c->ctx, tag, NULL, 0);
		return;
	}

	flux_r9tagInsert(tg, f, tag, offset, size);
}

int in_write(R9fid* f, uint32_t size, uint8_t *buf, char ** errstr) {
	struct iolist iol[2];
	int rc = 0;
	ircClient *cli = getClientFromFile(f->file);

	if (NULL == cli) {
		*errstr = "Could not locate client";
		return -1;
	}

	// TODO bug: 'join #fossuntest' lost a character

	// Send the input as is
	iol[0].iol_base = buf;
	iol[0].iol_len	= size;
	iol[0].iol_next	= iol + 1;
	iol[0].iol_rsvd	= 0;

	// Add a CR
	iol[1].iol_base = "\r";
	iol[1].iol_len	= 1;
	iol[1].iol_next	= NULL;
	iol[1].iol_rsvd	= 0;

	rc = msendl(cli->h, iol, iol + 1, -1);

	printf("%d %d\n", rc, errno);

	return 0;
}

R9fileEv infev = {
	.on_linewrite = in_write
}, readyev = {
	.on_read = ready_read
	,.on_clunk = flux_r9gateClunk
};

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
	ircClient *cli = getClientFromFile(f->file);

	if (NULL == cli) {
		s9error(&f->s->c->ctx, tag, "Could not find client");
	}

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

	msend(cli->h, msg, p - msg, -1);

	s9write(&f->s->c->ctx, tag, size);
}

R9fileEv nickev = {
	.on_read	= nick_read
	,.on_write	= nick_write
};

static void buf_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	BufFile *bf = dill_cont(f->file, BufFile, f);

	if (offset >= bf->f.st.size) {
		s9read(&f->s->c->ctx, tag, NULL, 0);
	}

	s9read(&f->s->c->ctx, tag, bf->b + offset, MIN(size, bf->f.st.size - offset));
}

static R9fileEv bufev = {
	.on_read	= buf_read
};

int r9list_tmp(R9fid * f, C9stat ** _st) {
	C9stat **st = _st;
	List * it;
	ircClient *cli = getClientFromFile(f->file);
	ircBuffer *buf = ircBufferFromFile(f->file);

	if (NULL == cli) {
		return -1;
	}

	if (&cli->root == f->file) {
		*st++ = &cli->fmotd.f.st;
		*st++ = &cli->f001.f.st;
		*st++ = &cli->f002.f.st;
		*st++ = &cli->f003.f.st;
		*st++ = &cli->f004.f.st;
		*st++ = &cli->f005.f.st;
		*st++ = &cli->nick.f.st;
		*st++ = &cli->inf.st;
		*st++ = &cli->fraw.f.st;
		*st++ = &cli->fready.st;

		// TODO: buffer listing
		flux_list_foreach(&cli->bufActive, list, it, buf) {
			*st++ = &buf->files[IRC_BUFFER_ROOT].st;
		}
	} else if (NULL != buf) {
		if (buf->files == f->file) { // buffer root
			*st++ = &buf->files[IRC_BUFFER_IN].st;
			*st++ = &buf->files[IRC_BUFFER_OUT].st;
		}
	}

	return (st == _st) ? -1 : st - _st;
}

R9file *r9seek_tmp(R9file * f, R9session * s, const char * name) {
	if (NULL == f && NULL == name) {
		// TODO: have sub directories;
		return &CLIENT_TEMP->root;
	}

	ircClient *cli = getClientFromFile(f);
	ircBuffer *buf;
	List *it;

	if (NULL == cli) {
		return NULL;
	}

	if (&cli->root == f) {
		if (0 == strcmp(cli->fmotd.f.st.name, name)) {
			return &cli->fmotd.f;
		} else if (0 == strcmp(cli->f001.f.st.name, name)) {
			return &cli->f001.f;
		} else if (0 == strcmp(cli->f002.f.st.name, name)) {
			return &cli->f002.f;
		} else if (0 == strcmp(cli->f003.f.st.name, name)) {
			return &cli->f003.f;
		} else if (0 == strcmp(cli->f004.f.st.name, name)) {
			return &cli->f004.f;
		} else if (0 == strcmp(cli->f005.f.st.name, name)) {
			return &cli->f005.f;
		} else if (0 == strcmp(cli->nick.f.st.name, name)) {
			return &cli->nick.f;
		} else if (0 == strcmp(cli->inf.st.name, name)) {
			return &cli->inf;
		} else if (0 == strcmp(cli->fraw.f.st.name, name)) {
			return &cli->fraw.f;
		} else if (0 == strcmp(cli->fready.st.name, name)) {
			return &cli->fready;
		}

		flux_list_foreach(&cli->bufActive, list, it, buf) {
			if (0 == strcmp(buf->files->st.name, name)) {
				return buf->files; // root
			}
		}
	} else if (buf = ircBufferFromFile(f)) {
		if ((f == buf->files)) { // root
			if (0 == strcmp(buf->files[IRC_BUFFER_IN].st.name, name)) {
				return buf->files + IRC_BUFFER_IN;
			} else if (0 == strcmp(buf->files[IRC_BUFFER_OUT].st.name, name)) {
				return buf->files + IRC_BUFFER_OUT;
			}
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

	assert(-1 != srv);

	forever {
		cli = tcp_accept(srv, NULL, -1);

		if (-1 == cli) {
			break;
		}

		c = allocClient(srv9, cli, tcp_close);

		if (NULL == c) {
			break;
		}

		go(run(c));
	}
}

#define bufFile_setup(bf, buf) do { (bf).b = buf; (bf).be = endof(buf); } while (0)

void fs_setup(ircClient *cli) {
	treeSafeInit(&clientTree);

	bufFile_setup(cli->fraw, cli->logbuffer);
	bufFile_setup(cli->f001, cli->m1);
	bufFile_setup(cli->f002, cli->m2);
	bufFile_setup(cli->f003, cli->m3);
	bufFile_setup(cli->f004, cli->m4);
	bufFile_setup(cli->f005, cli->m5);

	flux_r9tagAllocatorInit(&all);
	flux_r9fileInit(&cli->fready, &all);

	dill_rbtree_insert(&clientTree, 0, &cli->tree);
	dill_list_init(&cli->bufActive);
	dill_list_init(&cli->bufFree);
	cli->bufCount = 0;

	cli->root.st = (C9stat){
		.qid	= {.type = C9qtdir}
		,.mode = 0500 | C9stdir
		,.name = "."
		,.size = 0
		ST_DEFAULTS
	};
	cli->fmotd.f.st	= (C9stat){
		.qid = {}
		,.mode	= 0400
		,.name	= "motd"
		,.size	= 0
		ST_DEFAULTS
	};
	cli->fmotd.f.ev	= &bufev;
	cli->inf.st = (C9stat){
		.qid = {.type = C9qtappend}
		,.mode = 0200 | C9stappend
		,.name = "in"
		,.size = 0
		ST_DEFAULTS
	};
	cli->inf.ev = &infev;

	cli->f001.f.st	= (C9stat){
		.qid = {.path = 1}
		,.mode	= 0400
		,.name	= "001"
		,.size	= 0
		ST_DEFAULTS
	};
	cli->f001.f.ev	= &bufev;
	cli->f002.f.st	= (C9stat){
		.qid = {.path = 2}
		,.mode	= 0400
		,.name	= "002"
		,.size	= 0
		ST_DEFAULTS
	};
	cli->f002.f.ev	= &bufev;
	cli->f003.f.st	= (C9stat){
		.qid = {.path = 3}
		,.mode	= 0400
		,.name	= "003"
		,.size	= 0
		ST_DEFAULTS
	};
	cli->f003.f.ev	= &bufev;
	cli->f004.f.st	= (C9stat){
		.qid = {.path = 4}
		,.mode	= 0400
		,.name	= "004"
		,.size	= 0
		ST_DEFAULTS
	};
	cli->f004.f.ev	= &bufev;
	cli->f005.f.st	= (C9stat){
		.qid = {.path = 5}
		,.mode	= 0400
		,.name	= "005"
		,.size	= 0
		ST_DEFAULTS
	};
	cli->f005.f.ev	= &bufev;
	cli->nick.f.st	= (C9stat){
		.qid	= {.path = 6}
		,.mode	= 0600
		,.name	= "nick"
		,.size	= 0
		ST_DEFAULTS
	};
	cli->nick.f.ev	= &nickev;
	cli->fready.st = (C9stat){
		.qid	= {.path = 7}
		,.mode	= 0400
		,.name	= "ready"
		,.size	= 0
		ST_DEFAULTS
	};
	cli->fready.ev = &readyev;
	cli->fraw.f.st = (C9stat){
		.qid	= {.path = 8}
		,.mode	= 0400
		,.name	= "raw"
		,.size	= 0
		ST_DEFAULTS
	};
	cli->fraw.f.ev = &bufev;

	CLIENT_TEMP = cli;
}

ircClient *getClientFromFile(R9file * rf) {
	// TODO: calculate ID from file
	return getClientById(0);
}

ircClient *getClientById(uint16_t id) {
	Tree * trCli;

	trCli = dill_rbtree_seek(&clientTree, id);

	return containerof(trCli, ircClient, tree);
}
