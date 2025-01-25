#pragma once

#include <dill/core.h>
#include <stdbool.h>

typedef enum {
	DILL_DNS_RESOLV	= 1 << 0
	,DILL_DNS_NSS	= 1 << 1
	,DILL_DNS_HOSTS	= 1 << 2
} dill_dns_cfgfile; 

handle dill_dns_main(void);
handle dill_dns_getcr(void);
int dill_dns_setcr(handle); // -1 if not a dns handle, give -1 to use main

handle dill_dns_connect(const char * ip);

int dill_dns_lookup4(handle, const char * host);
int dill_dns_lookup6(handle, const char * host);
int dill_dns_lookupp4(handle, const char * host);
int dill_dns_lookupp6(handle, const char * host);

#define dill_dns_lookup dill_dns_lookupp4

int dill_dns_query(handle, const char * host, int qType);

// Configuration

int dill_dns_loadsystem(handle, dill_dns_cfgfile); // RESOLV | NSS | HOSTS
int dill_dns_addserver(handle, const char * ip); // timeout?  tries?  node does 4.
handle dill_dns_clone(handle);

int dill_dns_loadconfigline(handle, dill_dns_cfgfile, const char * line);

#define dill_dns_isip(ip) (dill_dns_isipv4(ip) || dill_dns_isipv6(ip))

bool dill_dns_isipv4(const char * ip);
bool dill_dns_isipv6(const char * ip);
