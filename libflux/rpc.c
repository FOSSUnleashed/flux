#include <flux/rpc.h>
#include <assert.h>

struct rpcvfs {
	struct dill_hvfs vfs;
	TreeRoot tree;

	Bit mem : 1;
	handle pump;
};

typedef struct rpcvfs rpcvfs;

dill_unique_id(flux_rpc_type);

#define CONT(vfs) dill_cont(vfs, rpcvfs, vfs)
#define T(rpc) (&(rpc)->tree)

DILL_CHECK_STORAGE(rpcvfs, flux_rpcstorage);

static void *rpc_query(struct dill_hvfs *vfs, dill_query_type type) {
	if (flux_rpc_type == type) {
		return CONT(vfs);
	}

	errno = ENOTSUP;
	return NULL;
}

static void rpc_close(struct dill_hvfs *vfs) {
	rpcvfs *rpc = CONT(vfs);

	if (-1 != rpc->pump) {
		hclose(rpc->pump);
	}

	if (rpc->mem) {
		free(rpc);
	}
}


// TODO check storage size: rpcvfs vs flux_rpcstorage

handle flux_mrpc(void) {
	handle h;
	struct flux_rpcstorage *mem;
	rpcvfs *rpc;

	mem = calloc(sizeof(struct rpcvfs), 1);

	if (NULL == mem) {
		errno = ENOMEM;
		return -1;
	}

	h = flux_mrpc_mem(mem);

	if (-1 == h) {
		free(mem);
	}

	rpc = (rpcvfs*)mem;
	rpc->mem = 1;

	return h;
}

handle flux_mrpc_mem(struct flux_rpcstorage *mem) {
	rpcvfs *rpc = (rpcvfs*)mem;

	rpc->vfs.query = rpc_query;
	rpc->vfs.close	= rpc_close;

	dill_rbtree_init(T(rpc));
	rpc->pump = -1;

	return dill_hmake(&rpc->vfs);
}

void flux_rpc_setpump(handle h, handle pump) {
	rpcvfs *rpc = hquery(h, flux_rpc_type);

	if (NULL == rpc) {
		return;
	}

	rpc->pump = pump;
}

// callback for dill_waitfor
static void flux_rpc_clcancel(struct dill_clause *cl) {
	struct flux_rpcmsg *msg = dill_cont(cl, struct flux_rpcmsg, cl);
	struct rpcvfs *rpc = hquery(msg->rpc, flux_rpc_type);

	assert(NULL != rpc);

	// release msg
	dill_rbtree_erase(T(rpc), &msg->node);
	msg->busy = 0;
}

int flux_rpcwait(handle _rpc, uint64_t id, struct flux_rpcmsg * msg, int64_t deadline) {
	rpcvfs *rpc = hquery(_rpc, flux_rpc_type);
	int rc;

	// if busy fail
	if (msg->busy) {
		errno = EBUSY;
		return -1;
	}
	msg->busy = 1;
	msg->rpc	= _rpc;

	// setup clause
	dill_waitfor(&msg->cl, 0, flux_rpc_clcancel);

	// register message
	dill_rbtree_insert(T(rpc), id, &msg->node);

	// handle deadline (TODO)

	// block
	rc = dill_wait();

	// release msg
	dill_rbtree_erase(T(rpc), &msg->node);
	msg->busy = 0;

	return rc;
}

int flux_rpcemit(handle _rpc, struct flux_rpcmsg *msg) {
	if (!msg->busy) {
		errno = EAGAIN;
		return -1;
	}

	if (msg->rpc != _rpc) {
		errno = EINVAL;
		return -1;
	}

	// unblock clause
	dill_trigger(&msg->cl, 0);
}

struct flux_rpcmsg *flux_rpcget(handle _rpc, uint64_t id) {
	rpcvfs *rpc = hquery(_rpc, flux_rpc_type);
	Tree * node;

	if (NULL == rpc) {
		return NULL;
	}

	node = dill_rbtree_seek(T(rpc), id);
	return dill_cont(node, struct flux_rpcmsg, node);
}

// ECANCEL a single RPC call
int flux_rpccancel(handle, uint64_t id, int err) {
}

// ECANCEL all RPC calls associated with a handle
int flux_rpccrash(handle, int err) {
}

