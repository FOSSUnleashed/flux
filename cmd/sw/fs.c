#include <dill/all.h>
#include <r9.h>
#include <string.h>
#include <dill/util_rbtree.h>
#include <stdio.h>
#include <assert.h>
#include <flux/str.h>
#include <flux/list.h>

int sw_list(R9fid *, C9stat **);

R9file *sw_on_create(R9fid *, const char * name, uint32_t perms);

void sw_on_clunk(R9fid * f);
bool sw_on_remove(R9fid * f);
void sw_on_truncate(R9fid * f, uint64_t sz);
bool sw_on_wstat(R9fid * f, uint16_t, const uint8_t *, const uint8_t *, uint64_t, uint32_t, uint32_t);
void sw_on_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size);
void sw_on_write(R9fid* f, C9tag tag, uint64_t offset, uint32_t size, uint8_t *buf);

char UN[] = "R";

typedef struct dill_rbtree_item rbtree_node;

R9fileEv ramDirEv = {
	.on_create	= sw_on_create
	,.on_remove	= sw_on_remove
}, ramEv = {
	.on_truncate	= sw_on_truncate
	,.on_write	= sw_on_write
	,.on_read	= sw_on_read
	,.on_clunk	= sw_on_clunk
	,.on_wstat	= sw_on_wstat
	,.on_remove	= sw_on_remove
};

struct sw_job_history {
	uint64_t ts;
	List node;
	// Who
	// done_option
	// Task
};

struct sw_job {
	List history;
	// input_file_tree

	// Initial Input
};

enum {
	SW_WORKSPACE_ROOT
	,SW_WORKSPACE_OUTPUT
	,SW_WORKSPACE_INPUT
	,SW_WORKSPACE_CTL
	,SW_WORKSPACE_CTL_READY
	,SW_WORKSPACE_CTL_CONFIG
	,SW_WORKSPACE_CTL_HISTORY
	,SW_WORKSPACE_CTL_SPACE // prints the workspace ID
	,SW_WORKSPACE_CTL_DONE
	,SW_WORKSPACE_CTL_OPTIONS // What options are available for done
	,SW_WORKSPACE_FILES
};

struct sw_workspace {
	rbtree_node node; // val is sint64

	R9file files[SW_WORKSPACE_FILES];

	R9fileEv root_ev; // used for validation only

	// Task *task; // Contains the /input file tree and history information
}; // R9session

typedef struct sw_workspace sw_workspace;

sw_workspace main_workspace;

sw_workspace *sw_wsFromSession(R9session * s) {
	sw_workspace *ws;

	ws = dill_cont(s->fid.file, sw_workspace, files[SW_WORKSPACE_ROOT]);

	assert(&main_workspace == ws);

	// Validation that we have a workspace pointer
	if (&ws->root_ev == s->fid.file->ev) {
		return ws;
	}

	return NULL;
}

void initWorkspace(sw_workspace *ws) {
	flux_bufzero(ws, ws + 1);

	R9file *rf;

	ws->node.val = 0; // TODO: create ID

	rf = ws->files + SW_WORKSPACE_ROOT;

	rf->st.qid	= (C9qid){.type = C9qtdir};
	rf->st.uid	= UN;
	rf->st.gid	= UN;
	rf->st.muid	= UN;
	rf->st.mode	= 0500 | C9stdir;
	rf->st.name	= ".";

	rf->ev	= &ws->root_ev;

	rf = ws->files + SW_WORKSPACE_OUTPUT;

	rf->st.qid	= (C9qid){.type = C9qtdir, .path = SW_WORKSPACE_OUTPUT};
	rf->st.uid	= UN;
	rf->st.gid	= UN;
	rf->st.muid	= UN;
	rf->st.mode	= 0700 | C9stdir;
	rf->st.name	= "output";

	rf->ev	= &ramDirEv;

	rf = ws->files + SW_WORKSPACE_INPUT;

	rf->st.qid	= (C9qid){.type = C9qtdir, .path = SW_WORKSPACE_INPUT};
	rf->st.uid	= UN;
	rf->st.gid	= UN;
	rf->st.muid	= UN;
	rf->st.mode	= 0500 | C9stdir;
	rf->st.name	= "input";

	rf = ws->files + SW_WORKSPACE_CTL;

	rf->st.qid	= (C9qid){.type = C9qtdir, .path = SW_WORKSPACE_CTL};
	rf->st.uid	= UN;
	rf->st.gid	= UN;
	rf->st.muid	= UN;
	rf->st.mode	= 0500 | C9stdir;
	rf->st.name	= "ctl";

	rf = ws->files + SW_WORKSPACE_CTL_READY;

	rf->st.qid	= (C9qid){.path = SW_WORKSPACE_CTL_READY};
	rf->st.uid	= UN;
	rf->st.gid	= UN;
	rf->st.muid	= UN;
	rf->st.mode	= 0400;
	rf->st.name	= "ready";

	rf = ws->files + SW_WORKSPACE_CTL_CONFIG;

	rf->st.qid	= (C9qid){.path = SW_WORKSPACE_CTL_CONFIG};
	rf->st.uid	= UN;
	rf->st.gid	= UN;
	rf->st.muid	= UN;
	rf->st.mode	= 0600;
	rf->st.name	= "config";

	rf = ws->files + SW_WORKSPACE_CTL_OPTIONS;

	rf->st.qid	= (C9qid){.path = SW_WORKSPACE_CTL_OPTIONS};
	rf->st.uid	= UN;
	rf->st.gid	= UN;
	rf->st.muid	= UN;
	rf->st.mode	= 0400;
	rf->st.name	= "options";

	rf = ws->files + SW_WORKSPACE_CTL_DONE;

	rf->st.qid	= (C9qid){.path = SW_WORKSPACE_CTL_DONE};
	rf->st.uid	= UN;
	rf->st.gid	= UN;
	rf->st.muid	= UN;
	rf->st.mode	= 0200;
	rf->st.name	= "done";

	rf = ws->files + SW_WORKSPACE_CTL_SPACE;

	rf->st.qid	= (C9qid){.path = SW_WORKSPACE_CTL_SPACE};
	rf->st.uid	= UN;
	rf->st.gid	= UN;
	rf->st.muid	= UN;
	rf->st.mode	= 0400;
	rf->st.name	= "space";

	rf = ws->files + SW_WORKSPACE_CTL_HISTORY;

	rf->st.qid	= (C9qid){.path = SW_WORKSPACE_CTL_HISTORY};
	rf->st.uid	= UN;
	rf->st.gid	= UN;
	rf->st.muid	= UN;
	rf->st.mode	= 0400;
	rf->st.name	= "history";
}

struct sw_ramfile {
	rbtree_node node;

	R9file file;
	R9file *parent;

	char buffer[256];
	uint8_t *content;
	uint16_t sz;
};

#define RAMFILECOUNT (1 << 10)

typedef struct sw_ramfile sw_ramfile;

sw_ramfile files[RAMFILECOUNT];

struct dill_rbtree ramfs, free_ramfs;
uint16_t file_nodes = 0;

sw_ramfile *getRamFile(R9file *rf) {
	sw_ramfile *end = files + RAMFILECOUNT, *ram = NULL;

	if ((sw_ramfile *)rf > files && end > (sw_ramfile *)rf) {
		ram = dill_cont(rf, sw_ramfile, file);

		// TODO sanity check index
	}

	return ram;
}

void sw_debug(char c) {
	rbtree_node *it;
	sw_ramfile *file;
	printf("FILE_NODE_COUNT: %c %d\n", c, file_nodes);

	for (it = dill_rbtree_first(&ramfs); it; it = dill_rbtree_next(&ramfs, it)) {
		file = dill_cont(it, sw_ramfile, node);

		printf("\t[%d] %s %d\n", (file - files)/sizeof(sw_ramfile), file->buffer, file->sz);
	}
}

void sw_init(void) {
	dill_rbtree_init(&ramfs);
	dill_rbtree_init(&free_ramfs);
}

static void sw_init_ramfile(sw_ramfile * file, uint32_t mode) {
	file->file.st.name	= file->buffer;
	file->file.st.mode	= mode;
	file->file.st.uid	= UN;
	file->file.st.gid	= UN;
	file->file.st.muid	= UN;

	file->file.ev	= (mode & C9stdir) ? &ramDirEv : &ramEv;

	file->file.st.qid.type = (mode >> 24) & 0xc0;

	file->content = NULL;
	file->file.st.size = 0;
	file->sz = 0;
}

void sw_on_truncate(R9fid * f, uint64_t sz) {
	sw_ramfile * file = dill_cont(f->file, sw_ramfile, file);

	sw_debug('T');

	if (sz > f->file->st.size) {
		// need alloc?
		if (sz > file->sz) {
			// TODO:
			return;
		}
	}
	f->file->st.size = sz;
}

bool sw_on_wstat(R9fid *f, uint16_t cnt, const uint8_t * name, const uint8_t * gid, uint64_t sz, uint32_t mode, uint32_t mtime) {
	if (f->file->st.size != sz && UINT64_MAX != sz) {
		return false;
	}

	sw_debug('s');

	if (name) {
		strcpy(f->file->st.name, name);
		// TODO: handle capping the buffer
	}

	if (gid) {
		// TODO
	}

	if (UINT32_MAX != mode) {
		f->file->st.mode = mode;
	}

	if (UINT32_MAX != mtime) {
		f->file->st.mtime = mtime;
	}

	return true;
}

sw_ramfile *sw_new_ramfile(uint32_t mode) {
	sw_ramfile *file = NULL;
	rbtree_node *it;

	if (dill_rbtree_empty(&free_ramfs)) {
		if (file_nodes == 256) {
			return NULL;
		}

		file = files + file_nodes;

		file->file.st.qid.path = file->node.val = 0xFFFFF + file_nodes;

		file_nodes++;
	} else {
		it = dill_rbtree_first(&free_ramfs);
		dill_rbtree_erase(&free_ramfs, it);

		file = dill_cont(it, sw_ramfile, node);
	}

	if (NULL == file) {
		return NULL;
	}

	sw_init_ramfile(file, mode);

	dill_rbtree_insert(&ramfs, file->node.val, &file->node);

	return file;
}

void sw_free_ramfile(const sw_ramfile * file) {
	rbtree_node *it = (rbtree_node *)&file->node; // remove const

	dill_rbtree_erase(&ramfs, it);
	dill_rbtree_insert(&free_ramfs, file->node.val, it);
}

void sw_on_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	sw_ramfile * file = dill_cont(f->file, sw_ramfile, file);

	if (offset >= file->file.st.size || NULL == file->content) {
		s9read(&f->s->c->ctx, tag, NULL, 0);
		return;
	}

	uint16_t sz = file->file.st.size - offset;

	if (sz > size) {
		sz = size;
	}
	
	s9read(&f->s->c->ctx, tag, file->content + offset, sz);
}

#define BSZ (1 << 12)
#define MAX_BSZ (1 << 15)

void sw_on_write(R9fid* f, C9tag tag, uint64_t offset, uint32_t size, uint8_t *buf) { // TODO: handle truncation
	sw_ramfile * file = dill_cont(f->file, sw_ramfile, file);
	uint16_t sz;
	uint64_t end = offset + size;
	uint8_t *p;

	sw_debug('W');

	printf("rwx(%d %d %d) T:%d %s\n", f->isRead, f->isWrite, f->isExec, f->isTrunk, file->buffer);

	if (offset > MAX_BSZ || end > MAX_BSZ) {
		s9error(&f->s->c->ctx, tag, "Not enough space");
		return;
	}

	if (NULL == file->content) {
		sz = BSZ;

		while (size > sz) {
			sz += BSZ;
		}

		file->content = malloc(sz);

		if (NULL == file->content) {
			s9error(&f->s->c->ctx, tag, "Could not write to file");
			return;
		}

		file->sz = sz;
	}

	// file->sz current buffer size
	// offset index to write to
	// size amount of data being written

	if (end > file->sz) {
		for (sz = file->sz; sz < MAX_BSZ; sz += BSZ);

		p = realloc(file->content, sz);

		if (NULL == p) {
			s9error(&f->s->c->ctx, tag, "Could not write to file");
			return;
		}

		file->content = p;
	}

	if (end > file->file.st.size) {
		file->file.st.size = end;
	}

	memcpy(file->content + offset, buf, size);

	s9write(&f->s->c->ctx, tag, size);
}

R9file *sw_on_create(R9fid * f, const char * name, uint32_t mode) {
	sw_ramfile * file;

	printf("CREATE: %s\n", name);

	file = sw_new_ramfile(mode);

	if (NULL == file) {
		return NULL;
	}

	assert(file >= files && files + 1024 > file);

	sw_debug('C');

	strncpy(file->buffer, name, 256);
	file->buffer[255] = 0;

	file->file.st.mode = mode;

	file->parent = f->file;

	return &file->file;
}

void sw_on_clunk(R9fid * f) {
}

bool sw_on_remove(R9fid * f) {
	sw_ramfile * file = dill_cont(f->file, sw_ramfile, file);

	if (NULL == file) {
		return false;
	}

	assert(file >= files && files + 1024 > file);

	sw_debug('R');

	if (file->file.st.qid.type & C9qtdir) {
		int children = sw_list(f, NULL);

		if (children) {
			return false;
		}
	}

	file->file.st.mode	= 0;
	file->file.st.qid.type	= 0;

	file->buffer[0] = 0;

	if (NULL != file->content) {
		free(file->content);
	}

	file->content = NULL;
	file->file.st.size = 0;
	file->sz = 0;

	sw_free_ramfile(file);
	f->file = NULL;

	return true;
}

int sw_list(R9fid * f, C9stat ** st) {
	int i = 0;
	rbtree_node *it;
	sw_ramfile *ram = getRamFile(f->file);
	sw_workspace *ws = sw_wsFromSession(f->s);

	assert(&main_workspace == ws);

	if (ws->files + SW_WORKSPACE_ROOT == f->file) {
		st[0] = &ws->files[SW_WORKSPACE_OUTPUT].st;
		st[1] = &ws->files[SW_WORKSPACE_INPUT].st;
		st[2] = &ws->files[SW_WORKSPACE_CTL].st;
		
		return 3;
	} else if (ws->files + SW_WORKSPACE_CTL == f->file) {
		st[0] = &ws->files[SW_WORKSPACE_CTL_READY].st;
		st[1] = &ws->files[SW_WORKSPACE_CTL_DONE].st;
		st[2] = &ws->files[SW_WORKSPACE_CTL_OPTIONS].st;
		st[3] = &ws->files[SW_WORKSPACE_CTL_CONFIG].st;
		st[4] = &ws->files[SW_WORKSPACE_CTL_SPACE].st;
		st[5] = &ws->files[SW_WORKSPACE_CTL_HISTORY].st;
		
		return 6;
	} else if ((ws->files + SW_WORKSPACE_OUTPUT) == f->file || &ram->file == f->file) {
		it = dill_rbtree_first(&ramfs);

		for (; it; it = dill_rbtree_next(&ramfs, it)) {
			sw_ramfile * file = dill_cont(it, sw_ramfile, node);

			if (file->parent == f->file) {
				if (NULL != st) {
					st[i] = &file->file.st;
				}
				i++;
			}
		}

		return i;
	} else if (C9qtdir & f->file->st.qid.type) {
		return 0;
	}

	return -1;
}

sw_ramfile *sw_subSeek(sw_ramfile *parent, const char * name, sw_workspace *ws) {
	sw_ramfile * file = NULL;
	rbtree_node *it;
	R9file *pf	= (NULL == parent) ? ws->files + SW_WORKSPACE_OUTPUT : &parent->file;

	it = dill_rbtree_first(&ramfs);

	for (; it; it = dill_rbtree_next(&ramfs, it)) {
		file = dill_cont(it, sw_ramfile, node);

		if (file->parent == pf && 0 == strcmp(file->file.st.name, name)) {
			break;
		}

		file = NULL;
	}

	return file;
}

R9file *sw_seek(R9file * f, R9session * s, const char * name) {
	sw_ramfile *ram = getRamFile(f);
	sw_workspace *ws = sw_wsFromSession(s);

	assert(&main_workspace == ws);

	if (NULL != ram || (ws->files + SW_WORKSPACE_OUTPUT) == f) {
		ram = sw_subSeek(ram, name, ws);

		return (NULL == ram) ? NULL : &ram->file;
	} else if ((ws->files + SW_WORKSPACE_ROOT) == f) {
		if (0 == strcmp(ws->files[SW_WORKSPACE_OUTPUT].st.name, name)) {
			return ws->files + SW_WORKSPACE_OUTPUT;
		} else if (0 == strcmp(ws->files[SW_WORKSPACE_INPUT].st.name, name)) {
			return ws->files + SW_WORKSPACE_INPUT;
		} else if (0 == strcmp(ws->files[SW_WORKSPACE_CTL].st.name, name)) {
			return ws->files + SW_WORKSPACE_CTL;
		}
	} else if ((ws->files + SW_WORKSPACE_CTL) == f) {
		if (0 == strcmp(ws->files[SW_WORKSPACE_CTL_OPTIONS].st.name, name)) {
			return ws->files + SW_WORKSPACE_CTL_OPTIONS;
		} else if (0 == strcmp(ws->files[SW_WORKSPACE_CTL_READY].st.name, name)) {
			return ws->files + SW_WORKSPACE_CTL_READY;
		} else if (0 == strcmp(ws->files[SW_WORKSPACE_CTL_CONFIG].st.name, name)) {
			return ws->files + SW_WORKSPACE_CTL_CONFIG;
		} else if (0 == strcmp(ws->files[SW_WORKSPACE_CTL_SPACE].st.name, name)) {
			return ws->files + SW_WORKSPACE_CTL_SPACE;
		} else if (0 == strcmp(ws->files[SW_WORKSPACE_CTL_DONE].st.name, name)) {
			return ws->files + SW_WORKSPACE_CTL_DONE;
		} else if (0 == strcmp(ws->files[SW_WORKSPACE_CTL_HISTORY].st.name, name)) {
			return ws->files + SW_WORKSPACE_CTL_HISTORY;
		}
	}

	return NULL;
}

int init = 0;

void sw_attach(R9session *s) {
	if (!init) {
		initWorkspace(&main_workspace);
		init = 1;
	}

	s->fid.file = main_workspace.files + SW_WORKSPACE_ROOT;
}
