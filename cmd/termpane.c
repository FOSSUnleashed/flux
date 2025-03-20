#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define RESET "\x1b\x5b\x48\x1b\x5b\x32\x4a"
#define BG256(x)	("\x1b[48;5;" #x "m")
#define FG256(x)	("\x1b[38;5;" #x "m")
#define CLEARCODES	"\x1b[0m"

#define SMALLBUF	(1 << 8)

char	headline[SMALLBUF]
	,timeline[SMALLBUF]
	,linkcur[SMALLBUF]
	,linklast[SMALLBUF]
	,*filebuf;

size_t lastsz;

void header(const char * str) {
	system("reset");
	printf("%s%s %-77s%s\n%s", FG256("255"), BG256("240"), str, CLEARCODES, CLEARCODES);
}

void writeTime() {
	time_t now = time(NULL);

	struct tm* timeinfo = localtime(&now);

	strftime(timeline, SMALLBUF, "%Y-%m-%d %H:%M:%S", timeinfo);
}

char * load(const char *filename, size_t *sz) {
	struct stat st;
	int fd = open(filename, O_RDONLY);
	if (fd < 0)
		return NULL;

	if (fstat(fd, &st) < 0)
		return NULL;

	*sz = st.st_size;

	void *buffer = mmap(0, *sz, PROT_READ, MAP_PRIVATE, fd, 0);
	if (buffer == MAP_FAILED)
		return NULL;
	close(fd);

	return buffer;
}

void show(const char *fn) {
	size_t sz;

	if (NULL != filebuf) {
		munmap(filebuf, lastsz);
	}

	filebuf = load(fn, &sz);

	writeTime();
	snprintf(headline, SMALLBUF - 1, "%s  %s", timeline, fn);

	header(headline);
	write(1, filebuf, sz);
	lastsz = sz;
}

/*
	TODO: args
	-f file (for varfs)
	-n pipe (fifo)
	-p plumb (default)
	-s symlink (secondary default)
*/

int main(int argc, char ** argv) {
	char name[SMALLBUF];
	struct stat st;

	filebuf = NULL;

	if (2 != argc) {
		printf("Usage: termpane <link>\n");
		return 1;
	}

	strcpy(name, argv[1]);

	if (-1 == stat(name, &st)) {
		goto error;
	}

	if (st.st_mode & S_IFLNK) {
		linklast[0] = 0;

		for (;;) {
			if (-1 == readlink(name, linkcur, SMALLBUF - 1)) {
				goto error;
			}

			if (strncmp(linkcur, linklast, SMALLBUF - 1)) {
				show(name);
				strncpy(linklast, linkcur, SMALLBUF - 1);
			}

			usleep(300000);
		}
	}

error:
	printf("ERROR: '%s' is not a symlink!\n", name);
	return 1;
}
