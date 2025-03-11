#include <newniine.h>

#define ST_DEFAULTS \
		,.uid= cli->UN \
		,.gid= cli->UN \
		,.muid= cli->UN

static void buf_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	// TODO: detect file

	s9read(&f->s->c->ctx, tag, NULL, 0);
}

static void buf_write(R9fid* f, C9tag tag, uint64_t offset, uint32_t size, uint8_t *buf) {
	// TODO: detect file

	s9write(&f->s->c->ctx, tag, size);
}

static R9fileEv bufev = {
	.on_read = buf_read
	,.on_write = buf_write
};

ircBuffer *ircBufferAlloc(ircClient *cli) {
	ircBuffer *buf = NULL;
	List *it;

	if (dill_list_empty(&cli->bufFree)) {
		buf = cli->buffers + cli->bufCount;

		if (buf > flux_endof(cli->buffers)) {
			buf = NULL;
		} else {
			buf->id = cli->bufCount++;
		}
	} else {
		buf = flux_list_unfree(&cli->bufFree, it, ircBuffer, list);
	}

	if (NULL != buf) {
		buf->cli = cli;
		ircBufferInit(buf);
		dill_list_insert(&buf->list, &cli->bufActive);
	}

	return buf;
}

void ircBufferInit(ircBuffer *buf) {
	uint32_t id = buf->id;
	ircClient *cli = buf->cli;

	//flux_bufzero(buf, buf + 1);

	buf->id = id;
	id = 0x10000 | (id << 8);

	// TODO: set idname
	// Caller sets name

	buf->files[IRC_BUFFER_ROOT].st = (C9stat){
		.qid	= {.type = C9qtdir, .path = clientMaskPath(cli->tree.val, id)}
		,.mode	= 0500 | C9stdir
		,.size	= 0
		,.name	= buf->name // TODO: idname
		ST_DEFAULTS
	};

	buf->files[IRC_BUFFER_IN].st = (C9stat){
		.qid	= {.path = clientMaskPath(cli->tree.val, id)}
		,.mode	= 0200
		,.size	= 0
		,.name	= "in"
		ST_DEFAULTS
	};
	buf->files[IRC_BUFFER_IN].ev = &bufev;

	buf->files[IRC_BUFFER_OUT].st = (C9stat){
		.qid	= {.path = clientMaskPath(cli->tree.val, id | 1)}
		,.mode	= 0400
		,.size	= 0
		,.name	= "out"
		ST_DEFAULTS
	};
	buf->files[IRC_BUFFER_OUT].ev = &bufev;
}

void ircBufferFree(ircBuffer *buf);

ircBuffer *ircBufferFromFile(R9file * rf) {
	ircBuffer *buf = NULL;
	ircClient *cli;
	uint64_t path = rf->st.qid.path;
	uint32_t client_id, path_id;
	uint8_t buffer_id, file_id;

	client_id = clientUnmaskPath(path);
	path_id	= path & 0xFFFFFFFF;

	if (0x10000 == path_id & 0xFFFF0000) {
		buffer_id = (path_id & 0x0000FF00) >> 8;
		file_id = path_id & 0xFF;

		cli = getClientById(client_id);

		if (NULL != cli) { // TODO: handle dynamically allocated buffers
			buf = cli->buffers + buffer_id;

			if (buf->id != buffer_id) { // bad buffer
				buf = NULL;
			}
		}
	}

	return buf;
}

int ircBufferFmt(ircBuffer *buf, const char * const fmt, ...);
