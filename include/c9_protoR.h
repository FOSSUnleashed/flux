#pragma once
#include <c9_proto.h>

#ifndef C9_NO_SERVER

extern C9error s9version(C9ctx *c) __attribute__((nonnull(1)));
extern C9error s9auth(C9ctx *c, C9tag tag, const C9qid *aqid) __attribute__((nonnull(1, 3)));
extern C9error s9error(C9ctx *c, C9tag tag, const char *err) __attribute__((nonnull(1)));
extern C9error s9attach(C9ctx *c, C9tag tag, const C9qid *qid) __attribute__((nonnull(1, 3)));
extern C9error s9flush(C9ctx *c, C9tag tag) __attribute__((nonnull(1)));
extern C9error s9walk(C9ctx *c, C9tag tag, C9qid *qids[]) __attribute__((nonnull(1, 3)));
extern C9error s9open(C9ctx *c, C9tag tag, const C9qid *qid, uint32_t iounit) __attribute__((nonnull(1, 3)));
extern C9error s9create(C9ctx *c, C9tag tag, const C9qid *qid, uint32_t iounit) __attribute__((nonnull(1, 3)));
extern C9error s9read(C9ctx *c, C9tag tag, const void *data, uint32_t size) __attribute__((nonnull(1, 3)));
extern C9error s9readdir(C9ctx *c, C9tag tag, C9stat *st[], int *num, uint64_t *offset, uint32_t size) __attribute__((nonnull(1, 3, 4)));
extern C9error s9write(C9ctx *c, C9tag tag, uint32_t size) __attribute__((nonnull(1)));
extern C9error s9clunk(C9ctx *c, C9tag tag) __attribute__((nonnull(1)));
extern C9error s9remove(C9ctx *c, C9tag tag) __attribute__((nonnull(1)));
extern C9error s9stat(C9ctx *c, C9tag tag, const C9stat *s) __attribute__((nonnull(1, 3)));
extern C9error s9wstat(C9ctx *c, C9tag tag) __attribute__((nonnull(1)));

extern C9error s9proc(C9ctx *c) __attribute__((nonnull(1)));

uint8_t *dill_c9read(C9ctx *ctx, uint32_t size, int *err);
uint8_t *dill_c9begin(C9ctx *ctx, uint32_t size);
int dill_c9end(C9ctx *ctx);
void dill_c9error(C9ctx *ctx, const char *fmt, ...);

#endif /* C9_NO_SERVER */
