#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>

size_t strlcpy(char *dst, const char *src, size_t siz);

/* creates directories bottom-up, if necessary (code from ii) */
int mkdirp(const char *dir) {
	char tmp[PATH_MAX], *p;
	struct stat st;
	size_t len;

	strlcpy(tmp, dir, sizeof(tmp));
	len = strlen(tmp);
	if (len > 0 && tmp[len - 1] == '/')
		tmp[len - 1] = '\0';

	if ((stat(tmp, &st) != -1) && S_ISDIR(st.st_mode))
		return 0; /* dir exists */

	for (p = tmp + 1; *p; p++) {
		if (*p != '/')
			continue;
		*p = '\0';
		mkdir(tmp, S_IRWXU);
		*p = '/';
	}
	mkdir(tmp, S_IRWXU);

	return -!(0 == stat(tmp, &st) && S_ISDIR(st.st_mode));
}
