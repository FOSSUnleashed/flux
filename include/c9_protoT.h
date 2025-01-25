#pragma once
#include <c9_proto.h>

#ifndef C9_NO_CLIENT

extern C9error c9version(C9ctx *c, C9tag *tag, uint32_t msize) __attribute__((nonnull(1, 2)));
extern C9error c9auth(C9ctx *c, C9tag *tag, C9fid afid, const char *uname, const char *aname) __attribute__((nonnull(1, 2)));
extern C9error c9flush(C9ctx *c, C9tag *tag, C9tag oldtag) __attribute__((nonnull(1, 2)));
extern C9error c9attach(C9ctx *c, C9tag *tag, C9fid fid, C9fid afid, const char *uname, const char *aname) __attribute__((nonnull(1, 2)));
extern C9error c9walk(C9ctx *c, C9tag *tag, C9fid fid, C9fid newfid, const char *path[]) __attribute__((nonnull(1, 2, 5)));
extern C9error c9open(C9ctx *c, C9tag *tag, C9fid fid, C9mode mode) __attribute__((nonnull(1, 2)));
extern C9error c9create(C9ctx *c, C9tag *tag, C9fid fid, const char *name, uint32_t perm, C9mode mode) __attribute__((nonnull(1, 2, 4)));
extern C9error c9read(C9ctx *c, C9tag *tag, C9fid fid, uint64_t offset, uint32_t count) __attribute__((nonnull(1, 2)));
extern C9error c9write(C9ctx *c, C9tag *tag, C9fid fid, uint64_t offset, const void *in, uint32_t count) __attribute__((nonnull(1, 2, 5)));
extern C9error c9wrstr(C9ctx *c, C9tag *tag, C9fid fid, const char *s) __attribute__((nonnull(1, 2, 4)));
extern C9error c9clunk(C9ctx *c, C9tag *tag, C9fid fid) __attribute__((nonnull(1, 2)));
extern C9error c9remove(C9ctx *c, C9tag *tag, C9fid fid) __attribute__((nonnull(1, 2)));
extern C9error c9stat(C9ctx *c, C9tag *tag, C9fid fid) __attribute__((nonnull(1, 2)));
extern C9error c9wstat(C9ctx *c, C9tag *tag, C9fid fid, const C9stat *s) __attribute__((nonnull(1, 2, 4)));

/*
 * Wait until a response comes and process it. If the function returns
 * any error, context must be treated as 'broken' and no subsequent calls
 * should be made without reinitialization (c9version).
 */
extern C9error c9proc(C9ctx *c) __attribute__((nonnull(1)));

#endif /* C9_NO_CLIENT */
