#pragma once

#include <dill/impl.h>
#include <dill/cr.h>
#include <flux/tree.h>

struct flux_rpcmsg {
	Tree node;
	struct dill_clause cl;
	handle rpc;
	Bit busy : 1;
};

struct flux_rpcstorage { uint8_t _mem[13 * 8]; }; // Tree == 10, VFS == 2

handle flux_mrpc(void);
handle flux_mrpc_mem(struct flux_rpcstorage *);

int flux_rpcwait(handle, uint64_t id, struct flux_rpcmsg *, int64_t deadline);

int flux_rpcemit(handle, struct flux_rpcmsg *);

struct flux_rpcmsg *flux_rpcget(handle, uint64_t id);

int flux_rpccancel(handle, uint64_t id, int err);
int flux_rpccrash(handle, int err);

void flux_rpc_setpump(handle h, handle pump);

dill_unique_id_pub(flux_rpc_type);
