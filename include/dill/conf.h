#pragma once

#include <stdbool.h>
#include <sys/types.h>

#ifndef DILL_CONF_MAX_TOKEN
#define DILL_CONF_MAX_TOKEN 16
#endif

struct dill_anyconf {
	char *token[DILL_CONF_MAX_TOKEN];
	unsigned count;
	char buffer[1024], *tp, *cp, *input, *ie;
};

// Check to see if `mc` matches the pattern
bool dill_anyconf_match(const char *pat, char mc);
// Removes the last topen
void dill_anyconf_pop(struct dill_anyconf *cf);
// Adds the current loaded string as a token
int dill_anyconf_push(struct dill_anyconf *cf);
// Zero out cp/tp/count to read a new set of tokens
void dill_anyconf_reset(struct dill_anyconf *cf);
// Move the pointer forward over a matchPattern
size_t dill_anyconf_skip(struct dill_anyconf *cf, const char *pat);
size_t dill_anyconf_skipone(struct dill_anyconf *cf, const char *pat);
// Mark a set of characters for inclusion in push()
ssize_t dill_anyconf_scan(struct dill_anyconf *cf, const char *pat);

// Move to one character after the next \n, return false if we hit the end of input
bool dill_anyconf_nextline(struct dill_anyconf *cf);
// Push a null token -1 on failure, 0 on success
int dill_anyconf_pushnull(struct dill_anyconf *cf);
// Read buffer in, return true if we have data to read
bool dill_anyconf_readline(struct dill_anyconf *cf, char * line);

#define dill_anyconf_nextchar(cf) ((cf)->ie == (cf)->input ? '\0' : *(cf)->input)
#define dill_anyconf_endofline(cf) ((cf)->ie == (cf)->input)


#if !defined DILL_DISABLE_RAW_NAMES
#define anyconf dill_anyconf

#define anyconf_match dill_anyconf_match
#define anyconf_pop dill_anyconf_pop
#define anyconf_push dill_anyconf_push
#define anyconf_reset dill_anyconf_reset
#define anyconf_skip dill_anyconf_skip
#define anyconf_scan dill_anyconf_scan

#define anyconf_nextline dill_anyconf_nextline
#define anyconf_readline dill_anyconf_readline
#define anyconf_nextchar dill_anyconf_nextchar
#define anyconf_pushnull dill_anyconf_pushnull
#define anyconf_skipone dill_anyconf_skipone
#define anyconf_endofline dill_anyconf_endofline

#endif
