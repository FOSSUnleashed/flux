#pragma once

struct C9aux;

typedef struct C9r C9r;
typedef struct C9t C9t;
typedef struct C9stat C9stat;
typedef struct C9ctx C9ctx;
typedef struct C9qid C9qid;
typedef uint32_t C9fid;
typedef uint16_t C9tag;

/* Stat field is not changed if it's set to this value when calling c9wstat. */
#define C9nochange (~0)

/* Special fid used with auth/attach to basically avoid authentication. */
#define C9nofid ((C9fid)~0)

/* C9modes for opening a file. */
typedef enum
{
	C9read = 0,
	C9write = 1,
	C9rdwr = 2,
	C9exec = 3,
	C9trunc = 0x10,
	C9rclose = 0x40,
	C9excl = 0x1000,
}C9mode;

typedef enum
{
	/* User/owner. */
	C9permur = 1<<8, /* Readable. */
	C9permuw = 1<<7, /* Writable. */
	C9permux = 1<<6, /* Executable. */

	/* Group. */
	C9permgr = 1<<5,
	C9permgw = 1<<4,
	C9permgx = 1<<3,

	/* Other. */
	C9permor = 1<<2,
	C9permow = 1<<1,
	C9permox = 1<<0,
}C9perm;

/* Directory. */
#define C9permdir 0x80000000

/* Bitmask of stat.mode. */
#define C9stdir 0x80000000
#define C9stappend 0x40000000
#define C9stexcl 0x20000000
#define C9sttmp 0x04000000

/* Limits. */
enum
{
	C9maxtags = 64,    /* Maximal number of outstanding requests. [1-65535] */
	C9maxflush = 8,    /* Maximal number of outstanding flushes. [1-65535] */
	C9maxstr = 0xffff, /* Maximal string length. [1-65535] */
	C9minmsize = 4096, /* Minimal sane msize. [4096-...] */
	C9maxpathel = 16,  /* Maximal number of elements in a path. Do not change. */
};

/* Errors. */
typedef enum
{
	C9Einit = -1,  /* Initialization failed. */
	C9Ever = -2,   /* Protocol version doesn't match. */
	C9Epkt = -3,   /* Incoming packet error. */
	C9Etag = -4,   /* No free tags or bad tag. */
	C9Ebuf = -5,   /* No buffer space enough for a message. */
	C9Epath = -6,  /* Path is too long or just invalid. */
	C9Eflush = -7, /* Limit of outstanding flushes reached. */
	C9Esize = -8,  /* Can't fit data in one message. */
	C9Estr = -9    /* Bad string. */
}C9error;

/* Request types. */
typedef enum
{
	Tversion = 100,
	Tauth = 102,
	Tattach = 104,
	Tflush = 108,
	Twalk = 110,
	Topen = 112,
	Tcreate = 114,
	Tread = 116,
	Twrite = 118,
	Tclunk = 120,
	Tremove = 122,
	Tstat = 124,
	Twstat = 126
}C9ttype;

/* Response types. */
typedef enum
{
	Rversion = 101,
	Rauth = 103,
	Rattach = 105,
	Rerror = 107,
	Rflush = 109,
	Rwalk = 111,
	Ropen = 113,
	Rcreate = 115,
	Rread = 117,
	Rwrite = 119,
	Rclunk = 121,
	Rremove = 123,
	Rstat = 125,
	Rwstat = 127
}C9rtype;

/* Unique file id type. */
typedef enum
{
	C9qtdir = 1<<7,
	C9qtappend = 1<<6,
	C9qtexcl = 1<<5,
	C9qtauth = 1<<3,
	C9qttmp = 1<<2,
	C9qtfile = 0
}C9qt;

/* Unique file id. */
struct C9qid
{
	uint64_t path;
	uint32_t version;
	C9qt type;
};

/*
 * File stats. Version and muid are ignored on wstat. Dmdir bit
 * change in mode won't work on wstat. Set any integer field to
 * C9nochange to keep it unchanged on wstat. Set any string to NULL to
 * keep it unchanged. Strings can be empty (""), but never NULL after
 * stat call.
 */
struct C9stat
{
	C9qid qid;   /* Same as qid[0]. */
	uint64_t size; /* Size of the file (in bytes). */
	char *name;  /* Name of the file. */
	char *uid;   /* Owner of the file. */
	char *gid;   /* Group of the file. */
	char *muid;  /* The user who modified the file last. */
	uint32_t mode;   /* Permissions. See C9st* and C9perm. */
	uint32_t atime;  /* Last access time. */
	uint32_t mtime;  /* Last modification time. */
};

/* Response data. */
struct C9r
{
	union
	{
		char *error;

		struct
		{
			uint8_t *data;
			uint32_t size;
		}read;

		struct
		{
			uint32_t size;
		}write;

		/* File stats (only valid if type is Rstat). */
		C9stat stat;

		/*
		 * Qid(s). qid[0] is valid for auth/attach/create/stat/open.
		 * More ids may be a result of a walk, see numqid.
		 */
		C9qid qid[C9maxpathel];
	};
	C9rtype type; /* Response type. */

	/*
	 * If not zero, is the maximum number of bytes that are guaranteed
	 * to be read or written atomically, without breaking into multiple
	 * messages.
	 */
	uint32_t iounit;

	int numqid; /* Number of valid unique ids in qid array. */
	C9tag tag;  /* Tag number. */
};

/* Request data. */
struct C9t
{
	C9ttype type;
	C9tag tag;
	union
	{
		struct
		{
			char *uname;
			char *aname;
			C9fid afid;
		}attach;

		struct
		{
			char *uname;
			char *aname;
			C9fid afid;
		}auth;

		struct
		{
			char *name;
			uint32_t perm;
			C9mode mode;
		}create;

		struct
		{
			C9tag oldtag;
		}flush;

		struct
		{
			C9mode mode;
		}open;

		struct
		{
			uint64_t offset;
			uint32_t size;
		}read;

		struct
		{
			char *wname[C9maxpathel+1]; /* wname[16] is always NULL */
			uint8_t wcount;
			C9fid newfid;
		}walk;

		struct
		{
			uint64_t offset;
			uint8_t *data;
			uint32_t size;
		}write;

		C9stat wstat;
	};
	C9fid fid;
};

enum
{
	C9tagsbits = sizeof(uint32_t) * 8,
};

struct C9ctx
{
	/*
	 * Should return a pointer to the data (exactly 'size' bytes) read.
	 * Set 'err' to non-zero and return NULL in case of error.
	 * 'err' set to zero (no error) should be used to return from c9process
	 * early (timeout on read to do non-blocking operations, for example).
	 */
	uint8_t *(*read)(C9ctx *ctx, uint32_t size, int *err) __attribute__((nonnull(1, 3)));

	/* Should return a buffer to store 'size' bytes. Nil means no memory. */
	uint8_t *(*begin)(C9ctx *ctx, uint32_t size) __attribute__((nonnull(1)));

	/*
	 * Marks the end of a message. Callback may decide if any accumulated
	 * messages should be sent to the server/client.
	 */
	int (*end)(C9ctx *ctx) __attribute__((nonnull(1)));

	/* Callback called every time a new R-message is received. */
	void (*r)(C9ctx *ctx, C9r *r) __attribute__((nonnull(1, 2)));

	/* Callback called every time a new T-message is received. */
	void (*t)(C9ctx *ctx, C9t *t) __attribute__((nonnull(1, 2)));

	/* Callback for error messages. */
	void (*error)(C9ctx *ctx, const char *fmt, ...) __attribute__((nonnull(1), format(printf, 2, 3)));

	/* Auxiliary data, can be used by any of above callbacks. */
	struct C9aux *aux;

	/* private stuff */
	void *paux;
	uint32_t msize;
#ifndef C9_NO_CLIENT
	uint32_t flush[C9maxflush];
	uint32_t tags[C9maxtags/C9tagsbits];
#endif
	union
	{
#ifndef C9_NO_CLIENT
		C9tag lowfreetag;
#endif
#ifndef C9_NO_SERVER
		uint16_t svflags;
#endif
	};
};

/* Parse one directory entry. */
extern C9error c9parsedir(C9ctx *c, C9stat *stat, uint8_t **data, uint32_t *size) __attribute__((nonnull(1, 2, 3)));
