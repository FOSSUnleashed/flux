#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h> // system()
#include <sys/stat.h>
#include <args.h>
#include <flux/dial.h>

// stat() poll version
// read event file, update on event item (pubsub)

char buffer[1 << 12];
size_t sz;

char *argv0;

int treset() {
	return system("reset");
}

int render() {
	treset();
	write(1, buffer, sz);

	return 0;
}

int load(char *filename) {
	int fd = open(filename, 0);

	assert(-1 != fd);

	sz = read(fd, buffer, sizeof(buffer));

	return sz;
}

int timeEq(struct timespec *a, struct timespec *b) {
	return (a->tv_sec == b->tv_sec && a->tv_nsec == b->tv_nsec);
}

void usage() {
	printf("Usage: %s [-f file]\n", argv0);
	exit(0);
}

int main(int argc, char **argv) {
	char *fn = "test.file";
	struct stat st;
	typeof(st.st_mtim) mtime;

	ARGBEGIN {
	case 'f':
		fn = EARGF(usage());
		break;
	} ARGEND;

	// read file
	stat(fn, &st);
	mtime = st.st_mtim;
	load(fn);
	render();

	printf("NAMESPACE: %s\n", flux_getns());

	for (;;) {
		stat(fn, &st);

		if (timeEq(&mtime, &st.st_mtim)) {
			sleep(3);
		} else {
			load(fn);
			render();
			mtime = st.st_mtim;
		}
	}

	return 0;
}

#if 0

// do handshake (no auth)
c = setup(addr, eaddr, uname, euname, aname, eaname, deadline)

h == tcp_handle | ipc_handle
c == rpc_handle

error checking

fid = t9open(c, "pane", 0);

t9read(fid, offset, buffer, buffer_end);
plumb_parse(pmp, buffer, buffer_end);

#endif
