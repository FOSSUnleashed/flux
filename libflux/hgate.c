// libdill gate handle

#include <flux/gate.h>
#include <dill/impl.h>
#include <dill/cr.h>
#include <dill/util.h>
#include <errno.h>

// hquery handle
// hclose handle

#define CONT(vfs) dill_cont(vfs, gatevfs, vfs)

dill_unique_id(flux_gate_type);

struct gatevfs {
	struct dill_hvfs vfs;
	struct dill_clause cl;
	Bit closed : 1;
	Bit mem : 1; // dunno if we need this
};

typedef struct gatevfs gatevfs;

static void *flux_gate_query(struct dill_hvfs *vfs, dill_query_type type) {
	gatevfs *gvfs = CONT(vfs);

	if (flux_gate_type == type) {
		return gvfs;
	}

	errno = ENOTSUP;
	return NULL;
}


static void flux_gate_close(struct dill_hvfs *vfs) {
	gatevfs *gvfs = CONT(vfs);

	free(gvfs);
}

static void flux_gate_cancel(struct dill_clause *cl) {
	// Do nothing?
}

handle flux_mgate(void) {
	gatevfs *gvfs = calloc(sizeof(gatevfs), 1);
	handle h;

	if (NULL == gvfs) {
		errno = ENOMEM;
		return -1;
	}

	gvfs->vfs.query = flux_gate_query;
	gvfs->vfs.close = flux_gate_close;

	h = dill_hmake(&gvfs->vfs);

	if (-1 == h) {
		free(gvfs);
	}

	return h;
}

int flux_gateisheld(handle gate) {
	gatevfs *gvfs = dill_hquery(gate, flux_gate_type);

	return !gvfs->closed;
}

int flux_gatehold(handle gate, int64_t deadline) {
	gatevfs *gvfs = dill_hquery(gate, flux_gate_type);
	int rc;

	if (gvfs->closed) {
		errno = EBUSY;
		return -1;
	}

	gvfs->closed = 1;

	dill_waitfor(&gvfs->cl, 0, flux_gate_cancel);

	rc = dill_wait();

	gvfs->closed = 0;

	return rc;
}


int flux_gateopen(handle gate) {
	gatevfs *gvfs = dill_hquery(gate, flux_gate_type);

	if (!gvfs->closed) {
		return 0;
	}

	dill_trigger(&gvfs->cl, 0);

	return 0;
}
