#include <r9.h>
#include <dill/all.h>
#include <flux/time.h>
#include <string.h>
#include <flux/str.h>
#include <flux/list.h>
#include <flux/mq.h>

#include <stdio.h>

#include <assert.h>

char UN[] = "R", buf[1 << 14];

#define ST_DEFAULTS \
	.uid	= UN, \
	.gid	= UN, \
	.muid	= UN

List vars, gates;
uint32_t varcnt = 2;

// [ ] delete variables
// [ ] events on deletion
// [ ] ram files (entire directory?)
// [ ] ev files
// [x] timer files
// [ ] number files (+ - operations)
// [ ] Queue file (for work queue, eg: work/break/work/break/work/long-break/work/break/work/end)

//
// Variable type directories
//

R9file var_d = {
	.st = {
		.qid	= {.type = C9qtdir, .path = 1}
		,.mode	= C9stdir | 0500
		,.name	= "vars"
		,ST_DEFAULTS
	}
}, gate_d = {
	.st = {
		.qid	= {.type = C9qtdir, .path = 2}
		,.mode	= C9stdir | 0500
		,.name	= "gates"
		,ST_DEFAULTS
	}
}, time_d = {
	.st = {
		.qid	= {.type = C9qtdir, .path = 3}
		,.mode	= C9stdir | 0500
		,.name	= "time"
		,ST_DEFAULTS
	}
}, time_gate_d = {
	.st = {
		.qid	= {.type = C9qtdir, .path = 4}
		,.mode	= C9stdir | 0500
		,.name	= "gates"
		,ST_DEFAULTS
	}
}, time_ev_d = {
	.st = {
		.qid	= {.type = C9qtdir, .path = 5}
		,.mode	= C9stdir | 0500
		,.name	= "ev"
		,ST_DEFAULTS
	}
}, time_start_d = {
	.st = {
		.qid	= {.type = C9qtdir, .path = 6}
		,.mode	= C9stdir | 0500
		,.name	= "start"
		,ST_DEFAULTS
	}
}, time_end_d = {
	.st = {
		.qid	= {.type = C9qtdir, .path = 7}
		,.mode	= C9stdir | 0500
		,.name	= "end"
		,ST_DEFAULTS
	}
}, time_single_d = {
	.st = {
		.qid	= {.type = C9qtdir, .path = 8}
		,.mode	= C9stdir | 0500
		,.name	= "single"
		,ST_DEFAULTS
	}
};

typedef struct {
	R9file gate, ev, start, end, single;
	List node, open_ev, open_gate;
	uint64_t expire, next, ctime;
	uint32_t period, duration;
	uint8_t mem[];
} TimeFile;

//
// Root event file
//

struct R9tag tags[128];
List free_tags, events_head, time_head;
uint32_t tag_cnt = 0;

#define free_tag(tag) dill_list_erase(tag); dill_list_insert(tag, &free_tags)

flux_mq *events_mq;

struct R9tag* allocTag() {
	List *it;
	struct R9tag *cur;

	// grab a new tag structure
	if (dill_list_empty(&free_tags)) {
		assert(tag_cnt < 128);
		cur = tags + tag_cnt;
		tag_cnt++;
	} else {
		it = dill_list_next(&free_tags);
		dill_list_erase(it);

		cur = dill_cont(it, struct R9tag, list);
	}

	return cur;
}

void events_read(R9fid *f, C9tag tag, uint64_t offset, uint32_t size) {
	struct R9tag *cur;

	cur = allocTag();

	cur->f = f;
	cur->offset = offset;
	cur->size	= size;
	cur->tag	= tag;

	dill_list_insert(&cur->list, &events_head);
}

void events_clunk(R9fid *f) {
	List *head = &events_head, *it;
	struct R9tag *cur;

	r9tag_foreach(head, it, cur) {
		if (cur->f == f) {
			free_tag(it);
			return;
		}
	}
}

coroutine void pump() {
	List *it, *jt;
	struct R9tag *cur;
	flux_mq_reader *rdr;
	char pump_buf[1 << 9];
	ssize_t sz;
	uint64_t nowt;

	TimeFile *tf;

	dill_list_init(&events_head);

	events_mq = flux_mq_alloc(1 << 14);

	if_slow (NULL == events_mq) {
		return;
	}

	rdr = flux_mq_get_reader(events_mq);

	if_slow (NULL == rdr) {
		return;
	}

	forever {
		msleep(now() + 500);

		sz = flux_mq_read(rdr, pump_buf, 1 << 9);

		// send main events if any
		if (0 < sz) {
			r9tag_foreach(&events_head, it, cur) {
				s9read(&cur->f->s->c->ctx, cur->tag, pump_buf, sz);

				free_tag(it);

				// reset head so loop works properly
				it = &events_head;
			}
		}

		// send time events and gate closes
		flux_list_foreach(&time_head, node, it, tf) {
			nowt = flux_s();

			// period expire next
			if (tf->next > nowt || 0 == tf->expire) {
				continue;
			}

			tf->next = nowt + tf->period;
			if (tf->next > tf->expire) {
				tf->next = tf->expire;
			}

			if (tf->expire < nowt) {
				nowt = tf->expire;
			}

			sz = sprintf(pump_buf, "%llu\n", tf->expire - nowt);

			r9tag_foreach(&tf->open_ev, jt, cur) {
				s9read(&cur->f->s->c->ctx, cur->tag, pump_buf, sz);

				free_tag(jt);
				jt = &tf->open_ev;
			}

			if (tf->expire > nowt) {
				continue;
			}

			r9tag_foreach(&tf->open_gate, jt, cur) {
				s9read(&cur->f->s->c->ctx, cur->tag, NULL, 0);

				free_tag(jt);
				jt = &tf->open_gate;
			}

			tf->expire = 0;
		}
	}
}

//
// Timer 
//

void time_ev_read(R9fid *f, C9tag tag, uint64_t offset, uint32_t size) {
	TimeFile * tf = dill_cont(f->file, TimeFile, ev);
	R9tag *t;

	if (!tf->expire || flux_s() > tf->expire) {
		s9read(&f->s->c->ctx, tag, NULL, 0);
		return;
	}

	t = allocTag();

	t->f	= f;
	t->tag = tag;
	t->offset	= offset;
	t->size	= size;

	dill_list_insert(&t->list, &tf->open_ev);
}

void time_ev_clunk(R9fid *f) {
	TimeFile * tf = dill_cont(f->file, TimeFile, ev);
	List *head = &tf->open_ev, *it;
	struct R9tag *cur;

	r9tag_foreach(head, it, cur) {
		if (cur->f == f) {
			free_tag(it);
			return;
		}
	}
}

void time_gate_read(R9fid *f, C9tag tag, uint64_t offset, uint32_t size) {
	TimeFile * tf = dill_cont(f->file, TimeFile, gate);
	R9tag *t;

	t = allocTag();

	t->f	= f;
	t->tag = tag;
	t->offset	= offset;
	t->size	= size;

	dill_list_insert(&t->list, &tf->open_gate);
}

void time_gate_clunk(R9fid *f) {
	TimeFile * tf = dill_cont(f->file, TimeFile, gate);
	List *head = &tf->open_gate, *it;
	struct R9tag *cur;

	r9tag_foreach(head, it, cur) {
		if (cur->f == f) {
			free_tag(it);
			return;
		}
	}
}

static void time_start_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	TimeFile * tf = dill_cont(f->file, TimeFile, start);
	char buf[1 << 6];
	ssize_t sz;
	
	sz = sprintf(buf, "%llu\n", tf->ctime);

	if (sz > 0) {
		s9read(&f->s->c->ctx, tag, buf, sz);
	} else {
		s9read(&f->s->c->ctx, tag, NULL, 0);
	}
}

static void time_end_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	TimeFile * tf = dill_cont(f->file, TimeFile, end);
	char buf[1 << 6];
	ssize_t sz;
	
	sz = sprintf(buf, "%llu\n", tf->expire);

	if (sz > 0) {
		s9read(&f->s->c->ctx, tag, buf, sz);
	} else {
		s9read(&f->s->c->ctx, tag, NULL, 0);
	}
}

static void time_single_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	TimeFile * tf = dill_cont(f->file, TimeFile, single);
	char buf[1 << 6];
	ssize_t sz;
	uint64_t nowt = flux_s();

	if (nowt > tf->expire) {
		sz = 2;
		buf[0]	= '0';
		buf[1]	= '\n';
	} else {
		sz = sprintf(buf, "%llu\n", tf->expire - flux_s());	
	}

	if (sz > 0) {
		s9read(&f->s->c->ctx, tag, buf, sz);
	} else {
		s9read(&f->s->c->ctx, tag, NULL, 0);
	}
}

R9fileEv time_ev_ev = {
	.on_read	= time_ev_read
	,.on_clunk	= time_ev_clunk
}, time_gate_ev = {
	.on_read	= time_gate_read
	,.on_clunk	= time_gate_clunk
}, time_start_ev = {
	.on_shortread	= time_start_read
}, time_end_ev = {
	.on_shortread	= time_end_read
}, time_single_ev = {
	.on_shortread	= time_single_read
};

int newtime(const char * key, const char * due, const char * period) {
	TimeFile * time;
	C9stat *st;
	int64_t duet, periodt = 1;

	if (*period) {
		periodt = flux_parsetime(period);
	}

	duet = flux_parsetime(due);

	if (0 > duet || 0 > periodt) {
		return -1;
	}

	time = calloc(1, sizeof(TimeFile) + strlen(key) + 1);

	if_slow (NULL == time) {
		return -1;
	}

	time->ctime	= flux_s();
	time->expire = time->ctime + duet;
	time->next	= time->ctime + periodt;
	time->duration	= duet;
	time->period	= periodt;

	strcpy(time->mem, key);

	st = &time->start.st;

	st->uid = UN;
	st->gid = UN;
	st->muid = UN;
	st->name = time->mem;
	st->mode = 0400;
	st->qid.path = varcnt | (2LL << 31);
	st->atime = flux_s();
	st->mtime = flux_s();

	st = &time->end.st;

	st->uid = UN;
	st->gid = UN;
	st->muid = UN;
	st->name = time->mem;
	st->mode = 0400;
	st->qid.path = varcnt | (3LL << 31);
	st->atime = flux_s();
	st->mtime = flux_s();

	st = &time->single.st;

	st->uid = UN;
	st->gid = UN;
	st->muid = UN;
	st->name = time->mem;
	st->mode = 0400;
	st->qid.path = varcnt | (4LL << 31);
	st->atime = flux_s();
	st->mtime = flux_s();

	st = &time->gate.st;

	st->uid = UN;
	st->gid = UN;
	st->muid = UN;
	st->name = time->mem;
	st->mode = 0400;
	st->qid.path = varcnt | (1 << 31);
	st->atime = flux_s();
	st->mtime = flux_s();

	st = &time->ev.st;

	st->uid = UN;
	st->gid = UN;
	st->muid = UN;
	st->name = time->mem;
	st->mode = 0400;
	st->qid.path = varcnt++;
	st->atime = flux_s();
	st->mtime = flux_s();

	time->gate.ev	= &time_gate_ev;
	time->ev.ev	= &time_ev_ev;
	time->start.ev	= &time_start_ev;
	time->end.ev	= &time_end_ev;
	time->single.ev	= &time_single_ev;

	dill_list_init(&time->open_ev);
	dill_list_init(&time->open_gate);

	dill_list_insert(&time->node, &time_head);

	return 0;
}

//
// Gate files
//

typedef struct {
	R9file f;
	List node, open;
	uint8_t mem[];
} GateFile;

void gate_write(R9fid *f, C9tag tag, uint64_t offset, uint32_t size, uint8_t *data) {
	struct R9tag *cur;
	GateFile *gf = dill_cont(f->file, GateFile, f);
	struct dill_list *it, *head = &gf->open;
	char gate_buf[1 << 8];
	size_t sz;

	r9tag_foreach(head, it, cur) {
		// send
		s9read(&cur->f->s->c->ctx, cur->tag, NULL, 0);
		free_tag(it);

		// `it` gets invalidated, so we need to reset `it` to a somewhat valid value
		it = head;
	}

	// TODO: flux_mq_write_fmt
	sz = stpcpy(stpcpy(gate_buf, "gate_close "), f->file->st.name) - gate_buf;
	gate_buf[sz] = '\n';
	sz++;

	flux_mq_write(events_mq, gate_buf, sz);

	// Claim we consumed everything so that clients don't think they need to resend the junk
	s9write(&f->s->c->ctx, tag, size);
}

void gate_read(R9fid *f, C9tag tag, uint64_t offset, uint32_t size) {
	struct R9tag *cur = NULL;
	GateFile *gf = dill_cont(f->file, GateFile, f);
	List *head = &gf->open;

	cur = allocTag();

	// set the tag struct
	cur->tag	= tag;
	cur->size	= size;
	cur->offset	= offset;
	cur->f	= f;

	// insert into file tag list
	dill_list_insert(&cur->list, head);
}

void gate_clunk(R9fid *f) {
	GateFile *gf = dill_cont(f->file, GateFile, f);
	List *head = &gf->open, *it;
	struct R9tag *cur;

	r9tag_foreach(head, it, cur) {
		if (cur->f == f) {
			free_tag(it);
			return;
		}
	}
}

R9fileEv gateEv = {
	.on_read	= gate_read
	,.on_write	= gate_write
	,.on_clunk	= gate_clunk
};

int newgate(const char * key) {
	GateFile * gate;
	C9stat *st;

	gate = calloc(1, sizeof(GateFile) + 128);

	if_slow (NULL == gate) {
		return -1;
	}

	// We are assuming that the keyname fits in our 128 byte buffer
	strcpy(gate->mem, key);

	st = &gate->f.st;

	st->uid = UN;
	st->gid = UN;
	st->muid = UN;
	st->name = gate->mem;
	st->mode = 0600;
	st->qid.path = varcnt++;
	st->atime = flux_s();
	st->mtime = flux_s();

	gate->f.ev = &gateEv;

	dill_list_init(&gate->open);

	dill_list_insert(&gate->node, &gates);

	return 0;
}

//
// Var files
//

typedef struct {
	R9file f;
	List node;
	char *key, *val;
	uint8_t mem[];
} MemFile;

void mem_write(R9fid *f, C9tag tag, uint64_t offset, uint32_t size, uint8_t *buf) {
	MemFile *mem = dill_cont(f->file, MemFile, f);
	size_t sz;

	// Be stupid and use mem->val as a scratch buffer since we're going to wipe it anyways
	sz = stpcpy(stpcpy(mem->val, "var_write "), f->file->st.name) - mem->val;
	mem->val[sz] = '\n';
	sz++;
	flux_mq_write(events_mq, mem->val, sz);

	mem->f.st.size = size;
	memcpy(mem->val, buf, size);

	s9write(&f->s->c->ctx, tag, size);
}

void mem_read(R9fid *f, C9tag tag, uint64_t offset, uint32_t size) {
	MemFile *mem = dill_cont(f->file, MemFile, f);

	if (offset >= mem->f.st.size) {
		size = 0;
	}

	if (size > mem->f.st.size) {
		size = mem->f.st.size;
	}

	s9read(&f->s->c->ctx, tag, mem->val + offset, size ? size - offset : 0);
}

R9fileEv memEv = {
	.on_read	= mem_read
	,.on_write = mem_write
};

int newvar(const char * key) {
	MemFile *mem;
	C9stat *st;

	mem = calloc(1, sizeof(MemFile) + 8192 + 128);

	if_slow (NULL == mem) {
		return -1;
	}

	mem->val = stpcpy(mem->mem, key) + 1;
	mem->key = mem->mem;

	st = &mem->f.st;

	st->uid = UN;
	st->gid = UN;
	st->muid = UN;
	st->name = mem->key;
	st->mode = 0600;
	st->qid.path = varcnt++;
	st->atime = flux_s();
	st->mtime = flux_s();

	mem->f.ev = &memEv;

	dill_list_insert(&mem->node, &vars);

	return 0;
}

#define match(b, str) (0 == flux_bufeq((b), BUFLIT(str), NULL))
#define argcheck(n) do {\
	if (pp - bb <= n) {\
		*errstr = "Insufficient arguments";\
		return -1;\
	}\
} while (0)

int ctl_write(R9fid* f, uint32_t size, uint8_t *data, char ** errstr) {
	uint8_t *b, *end;
	Buffer bb[16], *pp;
	TimeFile *tf;
	R9file *rf;

	pp = bb;

	end	= data + size;

	for (b = data; pp < bb + 16 && b < end; ++pp) {
		pp->start = b;
		flux_bufadvance(b, end, ' ' != *b);
		if (' ' != *b) {
			pp->end = end;
			*end = 0; // TODO: FIXME BAD BAD BAD BAD (over-run of *data)
		} else {
			pp->end	= b;
			flux_bufadvance(b, end, ' ' == *b);
			pp->end[0] = 0;
		}
	}

	if (b != end) {
		*errstr	= "To many tokens in ctl string";
		return -1;
	}

	if (match(bb[0], "var")) {
		argcheck(1);

		// check for colision
		if (r9seek_tmp(&var_d, f->s, bb[1].start)) {
			return 0;
		}

		if (newvar(bb[1].start)) {
			*errstr = "Could not allocate memory";
			return -1;
		}
	} else if (match(bb[0], "reset")) {
		if (NULL == (rf = r9seek_tmp(&time_ev_d, f->s, bb[1].start))) {
			*errstr = "No such timer";
			return -1;
		}
		tf = dill_cont(rf, TimeFile, ev);

		if (tf->expire) {
			*errstr = "Timer still active"; // Do we actually want this behavior though?
			return -1;
		}

		if (!dill_list_empty(&tf->open_ev) || !dill_list_empty(&tf->open_gate)) {
			*errstr = "Gate and event files not flushed";
			return -1;
		}

		// TODO: period

		tf->ctime	= flux_s();
		tf->expire	= tf->duration + tf->ctime;
	} else if (match(bb[0], "time")) {
		argcheck(2);
		// check for colision
		if (r9seek_tmp(&time_ev_d, f->s, bb[1].start)) {
			return 0;
		}

		// time name Xt (offset)
		// time name Xt Yt (offset period)
		// time name due (due)
		// time name due Yt (due period)

		if (newtime(bb[1].start, bb[2].start, (pp - bb > 3) ? bb[3].start : "")) {
			*errstr = "Could not allocate memory";
			return -1;
		}
	} else if (match(bb[0], "gate")) {
		argcheck(1);
		// check for colision
		if (r9seek_tmp(&gate_d, f->s, bb[1].start)) {
			return 0;
		}

		if (newgate(bb[1].start)) {
			*errstr = "Could not allocate memory";
			return -1;
		}
	} else {
		*errstr = "Unknown command";
		return -1;
	}

	while (pp > bb) {
		--pp;
		pp->end[0] = ' ';
	}

	flux_mq_write(events_mq, data, size);

	return 0;
}

//
// Root Directory
//

R9fileEv ctlEv = {
	.on_linewrite = ctl_write
}, eventEv = {
	.on_read	= events_read
	,.on_clunk	= events_clunk
};

R9file root = {
	.st = {
		.qid	= {.type = C9qtdir},
		.size	= 0,
		.name	= ".",
		.mode	= C9stdir | 0500,
		ST_DEFAULTS
	}
}, rf_ctl = {
	.st = {
		.qid	= {},
		.size	= 0,
		.name	= "ctl",
		.mode	= 0200,
		ST_DEFAULTS
	}
	,.ev	= &ctlEv
}, rf_event = {
	.st = {
		.qid	= {.path = 1},
		.size	= 0,
		.name	= "event",
		.mode	= 0400,
		ST_DEFAULTS
	}
	,.ev	= &eventEv
};

int r9list_tmp(R9fid *f, C9stat **st) {
	int ret = 0;
	List *it;

	if (&root == f->file) {
		st[0] = &rf_ctl.st;
		st[1] = &rf_event.st;
		st[2] = &var_d.st;
		st[3] = &gate_d.st;
		st[4] = &time_d.st;

		return 5;
	} else if (&time_d == f->file) {
		st[0] = &time_gate_d.st;
		st[1] = &time_ev_d.st;
		st[2] = &time_start_d.st;
		st[3] = &time_end_d.st;
		st[4] = &time_single_d.st;

		return 5;
	} else if (&time_ev_d == f->file) {
		TimeFile *cur;

		flux_list_foreach(&time_head, node, it, cur) {
			st[ret] = &cur->ev.st;
			++ret;
		}

		return ret;
	} else if (&time_gate_d == f->file) {
		TimeFile *cur;

		flux_list_foreach(&time_head, node, it, cur) {
			st[ret] = &cur->gate.st;
			++ret;
		}

		return ret;
	} else if (&time_start_d == f->file) {
		TimeFile *cur;

		flux_list_foreach(&time_head, node, it, cur) {
			st[ret] = &cur->start.st;
			++ret;
		}

		return ret;
	} else if (&time_end_d == f->file) {
		TimeFile *cur;

		flux_list_foreach(&time_head, node, it, cur) {
			st[ret] = &cur->end.st;
			++ret;
		}

		return ret;
	} else if (&time_single_d == f->file) {
		TimeFile *cur;

		flux_list_foreach(&time_head, node, it, cur) {
			st[ret] = &cur->single.st;
			++ret;
		}

		return ret;
	} else if (&gate_d == f->file) {
		GateFile *cur;
		flux_list_foreach(&gates, node, it, cur) {
			st[ret] = &cur->f.st;
			++ret;
		}

		return ret;
	} else if (&var_d == f->file) {
		MemFile *cur;
		flux_list_foreach(&vars, node, it, cur) {
			st[ret] = &cur->f.st;
			++ret;
		}

		return ret;
	}

	return -1;
}

R9file *r9seek_tmp(R9file *rf, R9session *s, const char *str) {
	List *it;

	if (NULL == rf && NULL == str) {
		return &root;
	}

	if (&root == rf) {
		if (0 == strncmp("event", str, 5)) {
			return &rf_event;
		} else if (0 == strncmp("ctl", str, 3)) {
			return &rf_ctl;
		} else if (0 == strcmp(var_d.st.name, str)) {
			return &var_d;
		} else if (0 == strcmp(gate_d.st.name, str)) {
			return &gate_d;
		} else if (0 == strcmp(time_d.st.name, str)) {
			return &time_d;
		}
	} else if (&time_d == rf) {
		if (0 == strcmp(time_ev_d.st.name, str)) {
			return &time_ev_d;
		} else if (0 == strcmp(time_gate_d.st.name, str)) {
			return &time_gate_d;
		} else if (0 == strcmp(time_single_d.st.name, str)) {
			return &time_single_d;
		} else if (0 == strcmp(time_start_d.st.name, str)) {
			return &time_start_d;
		} else if (0 == strcmp(time_end_d.st.name, str)) {
			return &time_end_d;
		}
	} else if (&time_end_d == rf) {
		TimeFile *cur;

		flux_list_foreach(&time_head, node, it, cur) {
			if (!strcmp(str, cur->mem)) {
				return &cur->end;
			}
		}
	} else if (&time_start_d == rf) {
		TimeFile *cur;

		flux_list_foreach(&time_head, node, it, cur) {
			if (!strcmp(str, cur->mem)) {
				return &cur->start;
			}
		}
	} else if (&time_single_d == rf) {
		TimeFile *cur;

		flux_list_foreach(&time_head, node, it, cur) {
			if (!strcmp(str, cur->mem)) {
				return &cur->single;
			}
		}
	} else if (&time_gate_d == rf) {
		TimeFile *cur;

		flux_list_foreach(&time_head, node, it, cur) {
			if (!strcmp(str, cur->mem)) {
				return &cur->gate;
			}
		}
	} else if (&time_ev_d == rf) {
		TimeFile *cur;

		flux_list_foreach(&time_head, node, it, cur) {
			if (!strcmp(str, cur->mem)) {
				return &cur->ev;
			}
		}
	} else if (&gate_d == rf) {
		GateFile *cur;

		flux_list_foreach(&gates, node, it, cur) {
			if (!strcmp(str, cur->mem)) {
				return &cur->f;
			}
		}
	} else if (&var_d == rf) {
		MemFile *cur;

		flux_list_foreach(&vars, node, it, cur) {
			if (!strcmp(str, cur->key)) {
				return &cur->f;
			}
		}
	}

	return NULL;
}

int main(void) {
	handle srv, cli;
	struct ipaddr addr;
	R9client *c;

	dill_list_init(&vars);
	dill_list_init(&gates);
	dill_list_init(&time_head);
	dill_list_init(&free_tags);

	ipaddr_local(&addr, NULL, 1125, 0);

	srv	= tcp_listen(&addr, 10);
	R9srv * srv9 = flux_r9getMainSrv();
	flux_r9srvInit(srv9, r9seek_tmp, r9list_tmp);

	go(pump());

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
