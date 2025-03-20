#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h> // system()
#include <sys/stat.h>
#include <args.h>

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
