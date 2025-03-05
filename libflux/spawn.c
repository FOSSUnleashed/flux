#include <flux/spawn.h>
#include <unistd.h>
#include <poll.h>
#include <sys/resource.h>
#include <errno.h>

#if 0

static int spawn(const char ** argv, const char ** env, int *fds_out);

int flux_mspawn(const char ** argv, const char ** env, dill_handle *hout) {
// TODO
	// spawn()
	// convert all FDs to handles
	// return
}

int flux_bspawn(const char ** argv, const char ** env, dill_handle *hout) {
// TODO
}

#define NFD (64)

static int cleanChild(int fd0, int fd1, int fd2) {
	struct pollfd f[NFD];
	int fdmax, pos = 3, i, nfd = NFD, rc;

	// move FDs to correct positions
	if (-1 == dup2(fd0, 0)) {
		return -1;
	}
	if (-1 == dup2(fd1, 1)) {
		return -1;
	}
	if (-1 == dup2(fd2, 2)) {
		return -1;
	}
	close(fd0);
	close(fd1);
	close(fd2);

	getrlimit(RLIMIT_NOFILE, (struct rlimit *)&fdmax);

	while (pos < fdmax) {
		for (i = 0; i < nfd; ++i) {
			f[i].fd = pos + i;
			f[i].events = 0;
			f[i].revents = 0;
		}

		rc = poll(f, nfd, 0);

		if (rc != nfd) { // no open FDs
			for (i = 0; i < nfd; ++i) {
				if (POLLNVAL != f[i].revents) {
					close(f[i].fd);
				}
			}
		}

		pos += NFD;
		nfd = fdmax - pos;

		if (nfd > NFD) {
			nfd = NFD;
		}
	}

	return 0;
}

static int spawn(const char ** argv, const char ** env, int *fds_out) {
	int fds[6] = {-1,-1,-1,-1,-1,-1}, i, err, rc;

	if (NULL == argv) {
		errno = EINVAL;
		return -1;
	}

	if (NULL == fds_out) {
		errno = EINVAL;
		return -1;
	}

	if (pipe2(fds, O_NONBLOCK)) {
		err = errno;
		goto error;
	}
	if (pipe2(fds + 2, O_NONBLOCK)) {
		err = errno;
		goto error;
	}
	if (pipe2(fds + 4, O_NONBLOCK)) {
		err = errno;
		goto error;
	}

	rc = fork();
	if (-1 == rc) {
		err = errno;
		goto error;
	}

	if (rc) { // PARENT
		close(fds[0]);
		close(fds[3]);
		close(fds[5]);

		fds_out[0] = fds[1];
		fds_out[1] = fds[2];
		fds_out[2] = fds[4];

		return rc; // return PID
	} else { // CHILD
		close (fds[1]);
		close (fds[2]);
		close (fds[4]);

		if (-1 == cleanChild(fds[0], fds[3], fds[5])) {
			goto die;
		}

		if (NULL == env) {
			execvp(argv[0], argv);
		} else {
			execve(argv[0], argv, env);
		}
		die:
		exit(1); // We can't do anything, just die TODO -- can we do something better to inform parent?
	}

	error:
	for (i = 0; i < 6; ++i) {
		if (-1 != fds[i])
			close(fds[i]);
	}
	errno = err;
	return -1;
}

#endif
