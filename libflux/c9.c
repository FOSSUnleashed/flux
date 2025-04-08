#include <dill/all.h>
#include <new.h>
#include <r9.h>
#include <flux/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

// 8k / 56bytes (estimated size of wire-form of C9stat) == ~146
C9stat *stbuf[256] = {};

/*
extern C9error s9auth(C9ctx *c, C9tag tag, const C9qid *aqid) __attribute__((nonnull(1, 3)));
extern C9error s9flush(C9ctx *c, C9tag tag) __attribute__((nonnull(1)));
*/

// TODO: file->on_set_size(..., uint64_t sz) ; from Create(..., OWRONLY | OTRUNC) and wstat(size)

struct {
	uint16_t pos;
	uint64_t time;
} _meta;

uint8_t deathClunk[] = {11, 0, 0, 0, 120, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

uint8_t *dill__c9endread(C9ctx *c, uint32_t sz, int *err) {
	*err = 0;

	if (4 == sz) {
		return deathClunk;
	}

	return deathClunk + 4;
}

/*
 * Should return a pointer to the data (exactly 'size' bytes) read.
 * Set 'err' to non-zero and return NULL in case of error.
 * 'err' set to zero (no error) should be used to return from c9process
 * early (timeout on read to do non-blocking operations, for example).
 */
uint8_t *dill__c9read(C9ctx *ctx, uint32_t size, int *err) {
	R9client * c = (R9client *)ctx;
	*err = 0;

	while (0 > brecv(c->h, c->rdbuf, size, -1)) {
		switch (errno) {
			case EBADF:
				printf("EBADF\n");
				break;
			case EBUSY:
				printf("EBUSY\n");
				break;
			case ECANCELED:
				printf("ECANCELED\n");
				break;
			case ECONNRESET:
				printf("ECONNRESETF\n");
				break;
			case EINVAL:
				printf("EINVAL\n");
				break;
			case ENOTSUP:
				printf("ENOTSUP\n");
				break;
			case EPIPE:
				if (4 == size) {
					ctx->read = dill__c9endread;
					c->disconnected = 1;
					return dill__c9endread(ctx, size, err);
				}
			case ETIMEDOUT:
				printf("ETIMEDOUT\n");
				break;
		}

		*err = -1;
		return NULL;
	}
	// TODO handle errno from recv

	return c->rdbuf;
}

uint8_t *dill__c9begin(C9ctx *ctx, uint32_t size) {
	R9client * c = (R9client *)ctx;
	uint8_t *b = c->wrbuf + c->wroff;

	// TODO: size check

	c->wroff += size;

	return b;
}

int dill__c9end(C9ctx *ctx) {
	R9client * c = (R9client *)ctx;
	int r;

	r = bsend(c->h, c->wrbuf, c->wroff, -1);
	// TODO: handle error from send, check for disconnections

	if (0 == r) {
		c->wroff = 0;
	}

	return r;
}

void dill__c9error(C9ctx *ctx, const char *fmt, ...) {
	R9client * c = (R9client *)ctx;
	va_list ap;

	if (ctx) {
		c->disconnected = 1;
	}

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
	fflush(stderr);
	va_end(ap);
}

void initfid(R9fid *f, C9mode mode) {
	f->isOpen	= 1;
	f->isRead	= (1 & ~mode);
	f->isWrite	= ((2 & mode) >> 1) ^ (1 & mode);
	f->isExec	= 3 == (3 & mode);
	f->isTrunk	= (mode & C9trunc) ? 1 : 0;

	if (0 == f->iounit) {
		f->iounit = f->s->c->ctx.msize >> 1;
	}

	if (f->file->st.qid.type & C9qtdir) {
		f->d.offset	= 0;
		f->d.total	= -1;
	} else {
		f->f.h = -1;
	}
}

void print_fids_debug(R9client * c) {
	R9fid * f;
	struct dill_rbtree_item *it;
	char * name, blank = 0;

	for (it = dill_rbtree_first(&c->fidmap); it; it = dill_rbtree_next(&c->fidmap, it)) {
		f = dill_cont(it, R9fid, node);

		name = (f->file) ? f->file->st.name : &blank;

		printf("FID:\t%d\t%016llx\t%s\n", f->id, f->file, name);
	}
}

void srv_proc(C9ctx * ctx, C9t * t) {
	R9rep r = R9repno;
	R9client * c = (R9client *)ctx;
	R9session * s = NULL;
	C9qid *noqids[1] = {NULL}, *qids[17];
	R9fid *f = NULL, *nf = NULL;
	R9file *rf = NULL;
	char **str;

	// End check

	// All sessions dead, can't network!
	if (Tclunk == t->type && 0xFFFF == t->tag && 0xFFFFFFFF == t->fid) {
		// Handle all the on_clunks
		struct dill_rbtree_item *it;
		struct dill_rbtree *tree = &c->fidmap;
		//printf ("DEBUG: did clunk cleanup\n");

		// loop through s->fidmap
		for (it = dill_rbtree_first(tree); it; it = dill_rbtree_next(tree, it)) {
			f = dill_cont(it, R9fid, node);

			if (f->file && f->file->ev && f->file->ev->on_clunk) {
				f->file->ev->on_clunk(f);
			}
		}

		c->disconnected = 1;
		return;
	}

	// Grabfid
	switch (t->type) {
	case Tversion:
	case Tauth:
	case Tattach:
	case Tflush:
		break;
	default:
		f = seekFid(c, t->fid);
	}

	if (f && !s) {
		s = f->s;
	}

	if (s && s->readonly) {
		switch (t->type) {
		case Topen:
			if ((2 & (t->open.mode + 1)) == 0) {
				break;
			}
		case Twrite:
		case Tremove:
		case Twstat:
		case Tcreate:
			s9error(ctx, t->tag, "Read Only");
			return;
		default: break; // To silence warnings
		}
	}

	// Prechecks

	switch (t->type) {
	case Tversion:
		s9version(ctx);
		break;
	case Tattach:
		// TODO: don't call this to setup main session
		s = c->srv->session(c, t->fid, t->attach.uname, t->attach.aname);

		rf = s->fid.file;

		if (NULL == rf) {
			s9error(ctx, t->tag, "Invalid ANAME");
		} else {
			s9attach(ctx, t->tag, &rf->st.qid);
		}

		break;
	case Twalk:
		str = t->walk.wname;
		R9file * parent;
		C9qid ** Q = qids;

		// fid == newfid (can happen)
		// fid must not be open
		// wname might contain ..
		// if (nwname > 1) fid MUST point to a directory, s9error otherwise
		// Permission check on each walk step
		// nwqid == 0 only if nwname == 0
		// nwqid >= 1 and nwqid <= nwname
		// newfid will only be valid if nwqid == nwname
		// Walk of .. from root == clone()

		// We are doing a clone
		if (0 == t->walk.wcount) {
			// clone a fid
			nf = allocFid(c, t->walk.newfid);
			nf->s = s;
			nf->file = f->file;

			s9walk(ctx, t->tag, noqids);
			break;
		}

		parent = f->file;

		printf("walk\t%d\t%d", t->fid, t->walk.newfid);
		while (*str) {
			printf("\t'%s'", *str);

			rf = c->srv->seek(parent, s, *str);

			if (NULL == rf) {
				break;
			}

			*Q = &rf->st.qid;
			Q++;

			parent = rf;

			++str;
		}
		printf("\t%016llx\t%d -\t%d\n", rf, t->walk.wcount, (Q - qids));
		*Q = NULL;

		if (NULL == rf) {
			if (Q == qids) {
				s9error(ctx, t->tag, "No such file");
			} else {
				s9walk(ctx, t->tag, qids);
			}
			break;
		}

		nf = allocFid(c, t->walk.newfid);

		if (NULL == nf) {
			s9error(ctx, t->tag, "Could not allocate fid");
			break;
		}

		nf->s = s;
		nf->file = parent;

		s9walk(ctx, t->tag, qids);

		break;
	case Tcreate:
		if (!f) {
			s9error(ctx, t->tag, "No such fid");
			return;
		}

		if (f->file) {
			if (NULL == f->file->ev || NULL == f->file->ev->on_create) {
				s9error(ctx, t->tag, "Cannot create a file here");
				return;
			}
		}

		assert(f->file);

		// TODO: is f->file a directory?
		// TODO: permission to write to f->file

		if (NULL != c->srv->seek(f->file, s, t->create.name)) { // overwrite Ev?
			s9error(ctx, t->tag, "File already exists");
			return;
		}

		f->iounit = 0;
		// TODO:
		/// perm & (~0666 | (dir.perm & 0666))
		//lar file is being created and
		// perm & (~0777 | (dir.perm & 0777))

		rf = f->file->ev->on_create(f, t->create.name, t->create.perm);

		if (NULL == rf) {
			s9error(ctx, t->tag, "File creation handle failed");
			return;
		}

		rf->parent = f->file;
		f->file	= rf;

		initfid(f, t->create.mode);

		s9create(ctx, t->tag, &rf->st.qid, f->iounit);
		break;
	case Topen:
		if (!f) {
			s9error(ctx, t->tag, "No such fid");
			return;
		}

		if (f->isOpen) {
			s9error(ctx, t->tag, "Fid already open");
			return;
		}

		f->iounit = 0;

		initfid(f, t->open.mode);

		if (f->isTrunk && f->isWrite) {
			if (f->file && f->file->ev && f->file->ev->on_truncate) {
				f->file->ev->on_truncate(f, 0);
			}
		}

		s9open(ctx, t->tag, &f->file->st.qid, f->iounit); // TODO Only send here if we opened the file ourselves instead of the user
		break;
	case Tread:
		if (!f->isOpen) {
			s9error(ctx, t->tag, "Fid not open");
			break;
		}

		if (!f->isRead && !f->isExec) {
			s9error(ctx, t->tag, "Fid not open for reading");
			break;
		}

		if (f->file->st.qid.type & C9qtdir) {
			int	cnt;
			C9stat **st = stbuf;

			if (-1 == f->d.total) {
				if (f->file->ev && f->file->ev->on_list) {
					f->d.total	= f->file->ev->on_list(f, st);
				} else {
					f->d.total	= c->srv->list(f, st);
				}
			} else if (f->d.total == f->d.offset) {
				if (0) {
					// Originally written to make 9pfuse happy, it makes 9pfuse stupid as it reread the directory endlessly
					f->d.total = -1;
					f->d.offset	= 0; // reset the read pointer
				}
				// EOF
				s9read(ctx, t->tag, NULL, 0);
				break;
			}
			// TODO: partial directory reads
			cnt	= f->d.total;

			if (-1 == cnt) {
				s9error(ctx, t->tag, "Directory not listable");
				break;
			}

			s9readdir(ctx, t->tag, st, &cnt, &t->read.offset, t->read.size);

			f->d.offset = cnt;
		} else if (f->file && f->file->ev && f->file->ev->on_shortread) {
			if (t->read.offset) {
				s9read(ctx, t->tag, NULL, 0);
			} else {
				f->file->ev->on_shortread(f, t->tag, 0, t->read.size);
			}
		} else if (f->file && f->file->ev && f->file->ev->on_read) {
			f->file->ev->on_read(f, t->tag, t->read.offset, t->read.size);
		} else {
			s9error(ctx, t->tag, "File cannot be read");
		}
		break;
	case Twrite:
		// .size .offset .data
		if (!f->isOpen) {
			s9error(ctx, t->tag, "Fid not open");
			break;
		}

		if (!f->isWrite) {
			s9error(ctx, t->tag, "Fid not open for writing");
			break;
		}

		// TODO: muid updates
		// TODO: all the mtime (Create | Truncate) & (isdir | remove | rename) and atime (Read etc...) updates

		if (f->file && f->file->ev && f->file->ev->on_write) {
			f->file->st.mtime = flux_s(); // TODO have a return value to check
			f->file->ev->on_write(f, t->tag, t->write.offset, t->write.size, t->write.data);
		} else if (f->file && f->file->ev && f->file->ev->on_linewrite) {
			// TODO: handle case where multiple writes split a line
			// TODO: handle errors from linewrite

			uint32_t left = t->write.size;
			uint8_t *data = t->write.data, *next;
			char *errstr = NULL;
			int rc = 0;

			while (left > 1 && !rc) {
				for (next = data; left && *next && '\n' != *next; ++next, --left);

				rc = f->file->ev->on_linewrite(f, next - data, data, &errstr);

				for (; left && *next && '\n' == *next; ++next, --left);

				data = next;
			}

			if (rc) {
				s9error(ctx, t->tag, errstr ? errstr : "Write error");
			} else {
				f->file->st.mtime = flux_s();
				s9write(ctx, t->tag, t->write.size);
			}
		} else {
			s9error(ctx, t->tag, "File not writable");
		}
		break;
	case Tstat:
		if (f) {
			s9stat(ctx, t->tag, &f->file->st);
		} else if (f->file) {
			s9error(ctx, t->tag, "No such file");
		}
		break;
	case Tclunk:
		if (0) {
			printf("CLUNK: %016llx %d\n", f, c->nextFid);
			print_fids_debug(c);
		}

		if (f) {
			releaseFid(c, f);
		}
		s9clunk(ctx, t->tag);
		break;
	case Tremove:
		assert(f->file);

		printf("REMOVE: %016llx %s\n", f->file, f->file->st.name);

		if (f && f->file) {
			bool success = false;

			// TODO: Check parent directory permissions to remove file
			// TODO: check this file permissions to remove file
			if (f->file->ev && f->file->ev->on_remove) {
				success = f->file->ev->on_remove(f);
			}

			if (success) {
				s9remove(ctx, t->tag);
			} else {
				s9error(ctx, t->tag, "Failed to remove file");
			}
		} else {
			s9error(ctx, t->tag, "No such file");
		}

		if (f) {
			printf("CLUNK: %016llx %d\n", f, c->nextFid);
			releaseFid(c, f);
		}
		break;
	case Twstat:
		//st.name (only if existing files do not have that name; requires write perms to parent directory)
		//st.size (requires write to file; cannot work on directories | allowed to reject change for other reasons)
		//st.mode|mtime (owner of file, or leader of group owning file) (directory bit cannot be changed)
		//st.gid (owner of file, only if also member of new group or leader of current group if also member of new group)

		// TODO: fulfill the following:
		//All wstat changes happen or none do.
		// C9t->wstat (C9stat)  t->wstat
		// s9proc

		C9stat *st = &t->wstat;
		uint16_t cnt = 0;
		uint32_t mode;

		if (NULL == f->file) {
			s9error(ctx, t->tag, "No such file");
			break;
		}

		if (st->name) {
			cnt++;
		}

		if (st->gid) {
			cnt++;
		}

		if (UINT64_MAX != st->size) {
			// fire truncate event
			cnt++;
		}

		if (UINT32_MAX != st->mode) {
			cnt++;
			mode = (f->file->st.qid.type & C9qtdir) ? st->mode | C9stdir : st->mode & ~C9stdir;
		}

		if (UINT32_MAX != st->mtime) {
			cnt++;
		}

		bool success = false;

		if (f->file->ev) {
			if (cnt) {
				if (UINT64_MAX != st->size && f->file->ev->on_truncate) {
					f->file->ev->on_truncate(f, st->size);
				}
				if (f->file->ev->on_wstat) {
					success = f->file->ev->on_wstat(f, cnt, st->name, st->gid, st->size, mode, st->mtime);
				}
			} else if (f->file->ev->on_sync) {
				// TODO: do this as a pending event
				f->file->ev->on_sync(f);
				success = true;
			}
		}

		if (success) {
			s9wstat(ctx, t->tag);
		} else {
			s9error(ctx, t->tag, "Not supported");
		}

		break;
	default:
		s9error(ctx, t->tag, "Not Implimented");
	}

	// User
	if (c->t) {
		r = c->t(c, s, f, t);
	}

	if (R9repyes == r) {
		// User replied
		return;
	}
}

coroutine void run(R9client * c) {
	c->ctx.t = srv_proc;
	c->ctx.read	= dill__c9read;
	c->ctx.end	= dill__c9end;
	c->ctx.begin	= dill__c9begin;
	c->ctx.error	= dill__c9error;

	c->ctx.msize	= MSZ;

	while (!c->disconnected) {
		if (0 != s9proc(&c->ctx)) {
			c->disconnected = 1;
		}
	}

	if (c->closeSock) {
		c->closeSock(c->h, -1);
	} else {
		hclose(c->h);
	}

	free(c);
}

coroutine void r9tcplisten(uint16_t port, R9srv * srv9) {
	handle srv, cli;
	struct ipaddr addr;
	R9client *c;

	if (-1 == ipaddr_local(&addr, NULL, port, 0)) {
		return;
	}

	srv	= tcp_listen(&addr, 10);

	if (-1 == srv) {
		return;
	}

	forever {
		cli	= tcp_accept(srv, NULL, -1);

		c = allocClient(srv9, cli, tcp_close);

		if_slow (NULL == c) {
			tcp_close(cli, now() + 400);
			continue;
		}

		go(run(c));
	}
}
