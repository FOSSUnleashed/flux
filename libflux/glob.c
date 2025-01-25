#include <string.h>
#include <stdlib.h>

enum GlobType {
	GLOB_DEAD
	,GLOB_PREFIX // prefix*
	,GLOB_SUFFIX // *suffix
	,GLOB_MIXED // prefix*suffix
	,GLOB_MULTI // one*two*three
};

// TODO: handle multi

typedef struct glob Glob;
struct glob {
	char * pattern;
	int type, len, plen, slen, cnt;
};

int glob_init(Glob * g, const char * pat) {
	int len;

	g->pattern = strdup(pat);

	g->plen	= 0;
	g->slen	= 0;
	g->type	= GLOB_DEAD;

	for (len = 0, g->cnt = 0; *pat; ++len, ++pat) {
		if ('*' == *pat || '?' == *pat) {
			g->cnt++;
		} else {
			if (0 == g->cnt) {
				g->plen++;
			} else if (1 == g->cnt) {
				g->slen++;
			}
		}
	}

	g->len	= len;

	pat	= g->pattern;

	if (1 == g->cnt) {
		if ('*' == pat[0] || '?' == pat[0]) {
			g->type	= GLOB_SUFFIX;
		} else if ('*' == pat[len - 1] || '?' == pat[len - 1]) {
			g->type	= GLOB_PREFIX;
		} else {
			g->type	= GLOB_MIXED;
		}
	}

	return g->type;
}

void glob_free(Glob * g) {
	if (g->pattern) {
		free(g->pattern);
		g->pattern = NULL;
	}

	g->type = GLOB_DEAD;
}

int glob_match(Glob * g, const char * const str) {
	int len = strlen(str);

	// length sanity checks
	if (len < (g->len - g->cnt)) {
		return 0;
	}

	//fprintf(stderr, "DEBUG: %s %s (%d %d %d) %d\n", g->pattern, str, g->len, g->plen, g->slen, g->cnt);
	//fprintf(stderr, "DEBUG: %d\n", g->type);

	switch (g->type) {
	case GLOB_SUFFIX:
		return ('*' == *g->pattern || len == g->len) && !strncmp(str + (len - g->slen), g->pattern + 1, g->slen);
	case GLOB_PREFIX:
		return ('*' == g->pattern[g->plen] || len == g->len) && !strncmp(str, g->pattern, g->plen);
	case GLOB_MIXED:
		return ('*' == g->pattern[g->plen] || len == g->len) && !strncmp(str, g->pattern, g->plen) && !strncmp(str + (len - g->slen), g->pattern + 1 + g->plen, g->slen);
	}

	return 0;
}
