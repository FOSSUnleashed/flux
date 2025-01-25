#pragma once

#include <dill/core.h>
#include <X11/Xlib.h>
#include <X11/Xfuncproto.h>

static inline int Xwait(Display *d, XEvent *xe, uint64_t deadline) {
	int rc = XPending(d);

	while (0 == rc) {
		fdin(d->fd, -1);

		rc	= XPending(d);
	}

	XNextEvent(d, xe);

	return rc;
}
