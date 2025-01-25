#include <flux/dial.h>

// getenv()
#include <stdlib.h>
#include <string.h>

// getuid()
// getpwuid()
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

/*
       int getpwuid_r(uid_t uid, struct passwd *restrict pwd,
                      char buf[restrict .buflen], size_t buflen,
                      struct passwd **restrict result);
// */


static char * NAMESPACE[256] = {};
static char * DISPLAY[256] = {};

const char * getns() {
	char * env;

	if (!*NAMESPACE) {
		if (env = getenv("NAMESPACE")) {
			strcpy(NAMESPACE, env);
			goto end;
		}

		// WMII_ADDRESS actually
		if (env = getenv("WMII_NAMESPACE")) {
			strcpy(NAMESPACE, env);
			goto end;
		}

		// /tmp/ns.$USER.$DISPLAY
		return NULL;
	}

	end:
	return NAMESPACE;
}
