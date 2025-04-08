#pragma once

void dnsfs_setup();
void dnsfs_srv();

#include <r9.h>
#include <dill/_dns.h>
#include <dill/all.h>

// root
// -> Requests

// Requests
// raw (raw packet)
// host (server that will respond/has responded)
// -> RRs
// header info
// ctl?
// ready?

// RRs
// rd
// dn
// ipv4 (if A or likewise)
// ipv6 (if AAAA or likewise)
// host (if needed)
// txt (for TXT/SPF)
// raw

// to have a new packet, create a new request directory in root
// then either write to host + ctl? or write to raw

enum {
	RQD_ROOT
	,RQD_RAW
	,RQD_HOST
	,RQD_CTL
	,RQD_READY
	,RQD_HEADER
	,RQD_MAX
};

struct RequestDirectory { // must be dynamically allocated!
	R9file rf[RQD_MAX];
	uint8_t names[512]; // Buffer for names
	uint16_t namesz; // How much of the buffer above is used
	// ^ host file value | root_directory_name | rr_names?

	struct ResourceRecordDirectory *rrd;

	struct dns_packet p; // must be last member
};

struct ResourceRecordDirectory {
	R9file rf[1];
};

typedef struct RequestDirectory Reqdir;
typedef struct ResourceRecordDirectory RRdir;
