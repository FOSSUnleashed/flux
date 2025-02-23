#include <dill/impl.h>
#include <flux/fd.h>

#if 0

struct fd_bsock_vfs {
	struct dill_hvfs hvfs;
	struct dill_bsock_vfs bvfs;

	int fd;

	Bit busy : 1;
};

struct fd_msock_vfs {
	struct dill_hvfs hvfs;
	struct dill_msock_vfs mvfs;

	int fd;

	Bit busy : 1;
};

static int flux_fd_bsendl(struct dill_bsock_vfs *mvfs, struct dill_iolist *first, struct dill_iolist *last, int64_t deadline);
static ssize_t flux_fd_brecvl(struct dill_bsock_vfs *mvfs, struct dill_iolist *first, struct dill_iolist *last, int64_t deadline);
static int flux_fd_msendl(struct dill_msock_vfs *mvfs, struct dill_iolist *first, struct dill_iolist *last, int64_t deadline);
static ssize_t flux_fd_mrecvl(struct dill_msock_vfs *mvfs, struct dill_iolist *first, struct dill_iolist *last, int64_t deadline);

DILL_CHECK_STORAGE(fd_msock_vfs, flux_fd_storage);
DILL_CHECK_STORAGE(fd_msock_vfs, flux_fd_storage);

dill_handle dill_fd2bsock(int fd) {
	int nfd = dup(fd);
	dill_handle h;

	if (-1 == nfd) {
		return -1;
	}

	h = dill__fd2bsock(nfd);

	if (-1 == h) {
		close(nfd);
		return -1;
	}

	close(fd);

	return h;
}

dill_handle dill_fd2msock(int fd) {
	int nfd = dup(fd);
	dill_handle h;

	if (-1 == nfd) {
		return -1;
	}

	h = dill__fd2msock(nfd);

	if (-1 == h) {
		close(nfd);
		return -1;
	}

	close(fd);

	return h;
}

static void *flux_fd_m_hquery(struct dill_hvfs *hvfs, const void *type) {
	struct fd_msock_vfs *obj = dill_cont(hvfs, struct fd_msock_vfs, hvfs);
	if (type == dill_msock_type) return &obj->mvfs;

	errno = ENOTSUP;
	return NULL;
}

static void *flux_fd_b_hquery(struct dill_hvfs *hvfs, const void *type) {
	struct fd_bsock_vfs *obj = dill_cont(hvfs, struct fd_bsock_vfs, hvfs);
	if (type == dill_bsock_type) return &obj->bvfs;

	errno = ENOTSUP;
	return NULL;
}

static void flux_fd_m_hclose(struct dill_hvfs *hvfs) {
	struct fd_msock_vfs *obj = dill_cont(hvfs, struct fd_msock_vfs, hvfs);
	dill_fd_close(obj->fd);
	free(obj);
}

static void flux_fd_b_hclose(struct dill_hvfs *hvfs) {
	struct fd_bsock_vfs *obj = dill_cont(hvfs, struct fd_bsock_vfs, hvfs);
	dill_fd_close(obj->fd);
	free(obj);
}


dill_handle dill__fd2bsock(int fd) {
	struct dill_fd_bsock_vfs *obj = malloc(sizeof(struct fd_bsock_vfs));

	if dill_slow(NULL == obj) {
		errno = ENOMEM;
		return -1;
	}

	obj->hvfs.query	= flux_fd_m_query;
	obj->hvfs.close	= flux_fd_m_close;

	obj->bvfs.msendl	= flux_fd_msendl;
	obj->bvfs.mrecvl	= flux_fd_mrecvl;

	obj->fd	= fd;

	handle h = dill_hmake(&obj->hvfs);

	if (-1 == h) {
		free(obj);
		return -1;
	}

	return h;
}

dill_handle dill__fd2msock(int fd) {
	struct dill_fd_bsock_vfs *obj = malloc(sizeof(struct dill_fd_msock_vfs));

	if dill_slow(NULL == obj) {
		errno = ENOMEM;
		return -1;
	}
}

static int flux_fd_bsendl(struct dill_bsock_vfs *mvfs, struct dill_iolist *first, struct dill_iolist *last, int64_t deadline) {
}
static ssize_t flux_fd_brecvl(struct dill_bsock_vfs *mvfs, struct dill_iolist *first, struct dill_iolist *last, int64_t deadline) {
}

static int flux_fd_msendl(struct dill_msock_vfs *mvfs, struct dill_iolist *first, struct dill_iolist *last, int64_t deadline) {
	size_t iol_num, iol_sz, sz = 0, rc, rsz;
	struct fd_msock_vfs *obj = dill_cont(mvfs, struct fd_msock_vfs, mvfs);
	struct dill_iolist *cur	= first;
	char *buf;

	rc = dill_iolcheck(first, last, &iol_num, &iol_sz);

	if dill_slow(rc < 0) {
		return -1;
	}

	buf	= cur->iol_base;
	rsz	= cur->iol_len;

	while (cur) {
		rc = dill_fdout(obj->fd, deadline);

		if dill_slow(0 > rc) {
			// let errno bleed through
			return -1;
		}

		// TODO: handle null iol_base
		obj->busy = 1;
		rc = write(obj->fd, buf, rsz);
		obj->busy = 0;

		if (0 == rc) {
			break;
		}

		if dill_slow(0 > rc) {
			return -1;
		}

		sz += rc;

		if (rc == rsz) {
			cur	= cur->iol_next;

			if (cur) {
				rsz	= cur->iol_len;
				buf	= cur->iol_base;
			}
		} else {
			rsz	-= rc;
			buf	+= rc;
		}
	}

	return sz;
}

static ssize_t flux_fd_mrecvl(struct dill_msock_vfs *mvfs, struct dill_iolist *first, struct dill_iolist *last, int64_t deadline) {
	struct fd_msock_vfs *obj = dill_cont(mvfs, struct fd_msock_vfs, mvfs);
	struct dill_iolist *cur	= first;
	char *buf;

	rc = dill_iolcheck(first, last, &iol_num, &iol_sz);

	if dill_slow(rc < 0) {
		return -1;
	}

	buf	= cur->iol_base;
	rsz	= cur->iol_len;

	while (cur) {
		rc = dill_fdin(obj->fd, deadline);

		if dill_slow(0 > rc) {
			// Let errno bleed through
			return -1;
		}

		// TODO: handle null iol_base
		obj->busy = 1;
		rc = read(obj->fd, buf, rsz);
		obj->busy = 0;

		if (0 == rc) {
			break;
		}

		if (dill_slow(0 > rc)) {
			return -1;
		}

		sz += rc;

		if (rc == rsz) {
			cur	= cur->iol_next;

			if (cur) {
				rsz	= cur->iol_len;
				buf	= cur->iol_base;
			}
		} else {
			rsz	-= rc;
			buf	+= rc;
		}
	}

	return sz;
}

#endif
