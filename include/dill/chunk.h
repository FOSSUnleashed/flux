#pragma once

#include <dill/core.h>
#include <dill/util.h>

#define DILL_CHUNK_IN	0x01
#define DILL_CHUNK_OUT	0x02

struct dill_chunk_storage {char _[1 << 11];} DILL_ALIGN; // 1280?

handle dill_chunk_attach_mem(handle bs, int flags, struct dill_chunk_storage *mem);
handle dill_chunk_attach(handle bs, int flags);
handle dill_chunk_detach(handle s, uint64_t deadline);

#if !defined DILL_DISABLE_RAW_NAMES
#define chunk_attach dill_chunk_attach
#define chunk_attach_mem dill_chunk_attach_mem
#define chunk_detach dill_chunk_detach
#endif
