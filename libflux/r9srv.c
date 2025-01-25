#include <r9.h>
#include <assert.h>

static R9srv mainSrv = {};

R9srv *flux_r9getMainSrv() {
	return &mainSrv;
}

// need authEv (Fid *, uname, aname) -> File|stat (for qid)

void flux_r9srvInit(R9srv * srv, r9seekEv seek, r9listEv list) {
	srv->attach = flux_r9attach;

	srv->auth_read	= NULL;
	srv->auth_write	= NULL;
	srv->auth_clunk	= NULL;

	srv->session	= flux_r9sessionNew;

	srv->seek	= seek;
	srv->list	= list;

//	srv->flush	= flux_r9flush;
}

void flux_r9srvAttach(R9srv * srv, r9attachEv attach) {
	srv->attach = attach;
}

// Called after new session
void flux_r9attach(R9session * s) {
	s->fid.file = s->c->srv->seek(NULL, s, NULL);
}

void flux_r9sessionSetup(R9session * s, R9client * cli, C9fid fid, const char * uname, const char * aname) {
	s->c = cli;

	resetFid(&s->fid, fid);
	s->active = 1;

	s->fid.iounit = 0; // TODO
	s->fid.path	= NULL; // TODO
	s->fid.file = NULL;

	s->fid.s = s;
	s->fid.isSession = 1;

	dill_rbtree_insert(&cli->fidmap, s->fid.id, &s->fid.node);

	cli->srv->attach(s);
}

R9session *flux_r9sessionNew(R9client * cli, C9fid fid, const char * uname, const char * aname) {
	R9session * s;

	if (cli->main.active) {
		// TODO allocate a new session
		assert(0);
	} else {
		s = &cli->main;
		s->main = 1;
	}

	flux_r9sessionSetup(s, cli, fid, uname, aname);

	return s;
}
