#pragma once

#include <new.h>

#include <c9_protoR.h>
#include <dill/util_list.h>
#include <dill/util_rbtree.h>
#include <stdbool.h>
// Following niine/spec

typedef struct R9file R9file;
typedef struct R9fid R9fid;
typedef struct R9client R9client;
typedef struct R9session R9session;
typedef struct R9srv R9srv;
typedef struct R9fileEv R9fileEv;

typedef enum {
	R9reperr = -1
	,R9repyes
	,R9repno
	,R9repdone
} R9rep;

typedef struct {
	char * name;
	C9qt qtype;
	uint32_t type; // user-defined (enum)
	uint32_t perm; // mode
	uint32_t flags; // user-defined
} R9filetab;

typedef R9rep (*R9handle)(R9client *, R9session *, R9fid *, C9t *);

typedef void (*r9writeEv)(R9fid*, C9tag, uint64_t offset, uint32_t size, uint8_t *buf);
typedef int (*r9linewriteEv)(R9fid*, uint32_t size, uint8_t *buf, char ** errstr);
typedef void (*r9readEv)(R9fid*, C9tag, uint64_t offset, uint32_t size);
typedef void (*r9clunkEv)(R9fid*);
typedef bool (*r9removeEv)(R9fid*);

typedef bool (*r9wstatEv)(R9fid*, uint16_t cnt, const uint8_t *name, const uint8_t *gid, uint64_t sz, uint32_t mode, uint32_t mtime);
typedef void (*r9syncEv)(R9fid*);
typedef void (*r9truncateEv)(R9fid*, uint64_t sz);

// name mode
typedef R9file *(*r9createEv)(R9fid*, const char *, uint32_t);

// Called to assign root file
typedef void (*r9attachEv)(R9session *);

typedef R9file *(*r9seekEv)(R9file *, R9session *, const char *);
typedef int (*r9listEv)(R9fid *, C9stat **);

// Deprecated
R9file *r9seek_tmp(R9file *, R9session *, const char *);
int r9list_tmp(R9fid *, C9stat **);


coroutine void r9tcplisten(uint16_t port, R9srv *);

struct R9srv {
	void (*attach)(R9session *);
// - allocate/deallocate
// -- Fid
// -- Session
// -- Tags?
// -- Files
	void * auth_read, * auth_write, * auth_clunk;

	R9session *(*session)(R9client *, C9fid, const char * uname, const char * aname);
	void (*sessionClose)(R9session *);

	R9fid *(*fid)(C9fid, char ** path);
	void (*fidClose)(R9fid *);

	r9seekEv seek;
	r9listEv list;

// flush

// - new tag (Session/Client?)
// - close tag (Tag)
};

// Directory events
// - seek
// - list
// - new file
// - new fid? (create)

// Client
// -> ctx
// -> session
// --> fid

// Returns the handle stored in a R9Fid or attached R9file
#define r9getHandle(fid) \
	(-1 == (fid)->h ? ( \
		(fid)->st ? (fid)->st->h : -1\
	: (fid)->h))

struct R9fileEv {
	r9readEv on_read, on_shortread;
	r9writeEv on_write;
	r9linewriteEv on_linewrite;
	r9clunkEv on_clunk;
	r9createEv on_create;
	r9removeEv on_remove;

	r9syncEv on_sync;
	r9truncateEv on_truncate;
	r9wstatEv on_wstat;
};

struct R9file {
	C9stat st;

	Bit pending : 1;
	Bit ephemeral : 1;
	Bit mem : 1; // needs to be free()'d
	handle h;

	R9fileEv * ev;

	R9file * parent;
};

struct R9fid {
	struct dill_rbtree_item node;
	Bit isSession : 1;
	Bit isAuth : 1;
	Bit isOpen : 1;
	Bit isRead : 1;
	Bit isWrite : 1;
	Bit isExec : 1;
	Bit isTrunk : 1;
	Bit isRClose : 1;

	R9session *s;
	R9file *file;

	C9fid id;
	char ** path;
	uint32_t iounit;

	union {
		struct {
			handle h;
		} f;

		struct {
			int32_t offset;
			int32_t total;
		} d;
	};
};

struct R9session {
	R9fid fid;
	R9client *c;
	char * auth; // whoami
	char ** path; // attach path, maybe have it in R9fid?
	// fid list
	Bit readonly : 1;
	Bit main : 1;
	Bit active : 1;
	Bit allEvents : 1;
};

struct R9client {
	C9ctx ctx;
	R9handle t;
	R9session main;
	handle h;
	uint8_t rdbuf[MSZ], wrbuf[MSZ];
	uint32_t wroff, flags;
	C9fid     base, auth;
	struct dill_rbtree fidmap, fidfree;
	R9fid fids[128];
	size_t nextFid;
	R9srv * srv;

	Bit disconnected : 1;
	Bit attached : 1;

	// lookup_fn
	// seek_fn
	// tag list
	// session list
	// version
	// msz is in ctx
};

R9rep t_prefix(C9ctx *c, C9t *t);

void resetFid(R9fid * f, C9fid fid);
void registerSession(R9client * c, R9session * s);
void releaseFid(R9client * c, R9fid * f);

void setupClient(R9client *, handle, R9srv *);
R9client *allocClient(R9srv *, handle);

R9fid *seekFid(R9client * c, C9fid fid);
R9fid *allocFid(R9client * c, C9fid fid);

coroutine void run(R9client *);

// <R9srv>
R9srv *flux_r9getMainSrv();
void flux_r9srvInit(R9srv * srv, r9seekEv seek, r9listEv list);
void flux_r9srvAttach(R9srv *, r9attachEv);

void flux_r9attach(R9session * s);
void flux_r9sessionSetup(R9session * s, R9client * cli, C9fid fid, const char * uname, const char * aname);
R9session *flux_r9sessionNew(R9client * cli, C9fid fid, const char * uname, const char * aname);
// </R9srv>

struct R9tag {
	struct dill_list list;
	C9tag tag;
	uint64_t offset;
	uint32_t size;
	R9fid *f;
};

struct R9gateFile {
	R9file f;
	struct dill_list open;
};

typedef struct R9gateFile R9gateFile;
typedef struct R9tag R9tag;

#define r9tag_foreach(head, it, cur) dill_list_T_foreach(head, R9tag, list, it, cur)
