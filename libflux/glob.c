#include <string.h>
#include <stdlib.h>
#include <flux/str.h>
#define FLUX_HAVE_GLOB_TYPE
#include <flux/glob.h>

enum GlobType {
	GLOB_DEAD
	,GLOB_LITERAL // No wildcard characters to match
	,GLOB_WILD	// No literal characters, 100% wildcard
	,GLOB_PREFIX // prefix*
	,GLOB_SUFFIX // *suffix
	,GLOB_MIXED // prefix*suffix
	,GLOB_MULTI // one*two*three
};

// TODO: handle multi

typedef struct glob Glob;
struct glob {
	Buffer *pattern;
	uint16_t plen, slen, cnt; // Prefix Length | Suffix Length | Wildcard Count
	uint16_t min, max; // max = 0 if glob contains *
	enum GlobType type;
};

Buffer *flux_bufdup(Buffer b) {
	if (isbadbuffer(b)) {
		return NULL;
	}
	size_t sz = b.end - b.start + sizeof(Buffer);

	Buffer *p = malloc(sz);

	if (NULL != p) {
		p->start	= (uint8_t *)(p + 1);
		p->end	= ((uint8_t *)(p)) + sz;

		bufcpy(*p, b);
	}

	return p;
}

Glob * flux_globAlloc(Buffer pat) {
	return NULL;
}

int glob_init(Glob * g, Buffer pat) {
	int len;

	g->pattern = flux_bufdup(pat);

	if (NULL == g->pattern) {
		return GLOB_DEAD;
	}

	g->plen	= 0;
	g->cnt	= 0;
	g->min	= 0;
	g->type	= GLOB_DEAD;

	for (len = 0, g->cnt = 0; pat.start < pat.end; ++len, ++pat.start) {
		if ('*' == *pat.start || '?' == *pat.start) {
			g->cnt++;
			g->slen	= 0;
		} else {
			if (0 == g->cnt) {
				g->plen++;
			} else {
				g->slen++;
			}
		}
		if ('*' != *pat.start) {
			g->min++;
		}
	}

	if (len == g->min) { // We have a static length
		g->max = len;
	} else {
		g->max = 0;
	}

	pat	= *g->pattern;

	if (len == g->cnt) {
		g->type = GLOB_WILD;
	} else if (1 == g->cnt) {
		if ('*' == pat.start[0] || '?' == pat.start[0]) {
			g->type	= GLOB_SUFFIX;
		} else if ('*' == pat.start[len - 1] || '?' == pat.start[len - 1]) {
			g->type	= GLOB_PREFIX;
		} else {
			g->type	= GLOB_MIXED;
		}
	} else if (0 == g->cnt) {
		g->type	= GLOB_LITERAL;
	} else {
		// TODO
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

int glob_match(Glob * g, Buffer str) {
	if (NULL == str.start || NULL == str.end || str.start > str.end) { // isbadbuffer fails on BUFLIT1("")
		return 0;
	}

	int len = str.end - str.start;
	Buffer pat = *g->pattern;

	// length sanity checks
	if (len < g->min || (0 != g->max && len != g->max)) {
		return 0;
	}

	//fprintf(stderr, "DEBUG: %s %s (%d %d %d) %d\n", g->pattern, str, g->len, g->plen, g->slen, g->cnt);
	//fprintf(stderr, "DEBUG: %d\n", g->type);

	switch (g->type) {
	case GLOB_WILD:
		return 1;
	case GLOB_SUFFIX:
		str.start = str.end - g->slen;
		pat.start++;
	case GLOB_LITERAL:
		return 0 == flux_bufcmp(pat, str, NULL);

	case GLOB_PREFIX:
		str.end = str.start + g->plen;
		pat.end--;
		return 0 == flux_bufcmp(pat, str, NULL);

	case GLOB_MIXED:
		Buffer start = {.start = str.start, .end = str.start + g->plen}
			,end = {.start = str.end - g->slen, .end = str.end}
			,prefix = {.start = pat.start, .end = pat.start + g->plen}
			,suffix = {.start = pat.end - g->slen, .end = pat.end};
		return 0 == flux_bufcmp(start, prefix, NULL) && 0 == flux_bufcmp(end, suffix, NULL);

	default:
		return 0;
	}
}
