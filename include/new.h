#pragma once

#include <stdint.h>
#include <dill/util.h>
#include <dill/ip.h>
#include <dill/util_rbtree.h>

int mkdirp(const char *dir);

// OpenBSD
#ifdef NEED_STRLCPY
size_t strlcpy(char *dst, const char *src, size_t siz);
#endif

// TEMP libdill stuff

int ipaddr_remote4(struct ipaddr* addr, const char* name, int port, int64_t deadline);

// </TEMP>

// TEMP IRC stuff
typedef struct {
	char * host, *name;
	uint16_t port;
	Bit ssl : 1;
} IRC_Connection;

// </TEMP>

// flux json
#define JSMN_HEADER
#include <jim.h>

#include <jsmn.h>
void json_buffer_copy(char *dst, char *src);
size_t jim_writer(const void *src, size_t size, size_t nmemb, Jim_Sink sink);

typedef struct {
	Jim j;
	size_t sz;
	char * buf;
} JumboJim;

void jim_setup(JumboJim *j, char * buf, size_t sz);

// </json>

#define c9readMax(msz) (msz - 11)
#define c9writeMax(msz) (msz - 23)

#define MSZ (1 << 13)

