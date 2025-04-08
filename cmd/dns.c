#include "../libdill/dns/dns.h"
#include <stdio.h>
#include <stdarg.h>
#include <err.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/un.h>
#include <ctype.h>
#include <dill/all.h>
#include <assert.h>
#include <args.h>

char *argv0;

struct {
	struct {
		const char *path[8];
		unsigned count;
	} resconf, nssconf, hosts, cache;

	const char *qname;
	enum dns_type qtype;

	int (*sort)();

	int verbose;
} MAIN = {
	.sort	= &dns_rr_i_packet,
};

static void panic(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);

#if _WIN32
	vfprintf(stderr, fmt, ap);

	exit(EXIT_FAILURE);
#else
	verrx(EXIT_FAILURE, fmt, ap);
#endif
} /* panic() */

#define panic_(fn, ln, fmt, ...)	\
	panic(fmt "%0s", (fn), (ln), __VA_ARGS__)
#define panic(...)			\
	panic_(__func__, __LINE__, "(%s:%d) " __VA_ARGS__, "")


static struct dns_resolv_conf *resconf(void) {
	static struct dns_resolv_conf *resconf;
	const char *path;
	unsigned i;
	int error;

	if (resconf)
		return resconf;

	if (!(resconf = dns_resconf_open(&error)))
		panic("dns_resconf_open: %s", dns_strerror(error));

	if (!MAIN.resconf.count)
		MAIN.resconf.path[MAIN.resconf.count++]	= "/etc/resolv.conf";

	for (i = 0; i < MAIN.resconf.count; i++) {
		path	= MAIN.resconf.path[i];

		if (0 == strcmp(path, "-"))
			error	= dns_resconf_loadfile(resconf, stdin);
		else
			error	= dns_resconf_loadpath(resconf, path);

		if (error)
			panic("%s: %s", path, dns_strerror(error));
	}

	for (i = 0; i < MAIN.nssconf.count; i++) {
		path	= MAIN.nssconf.path[i];

		if (0 == strcmp(path, "-"))
			error	= dns_nssconf_loadfile(resconf, stdin);
		else
			error	= dns_nssconf_loadpath(resconf, path);

		if (error)
			panic("%s: %s", path, dns_strerror(error));
	}

	if (!MAIN.nssconf.count) {
		path = "/etc/nsswitch.conf";

		if (!(error = dns_nssconf_loadpath(resconf, path)))
			MAIN.nssconf.path[MAIN.nssconf.count++] = path;
		else if (error != ENOENT)
			panic("%s: %s", path, dns_strerror(error));
	}

	return resconf;
} /* resconf() */


static size_t dns_af_len(int af) {
	static const size_t table[AF_MAX]	= {
		[AF_INET6]	= sizeof (struct sockaddr_in6),
		[AF_INET]	= sizeof (struct sockaddr_in),
#if (defined AF_UNIX && !defined _WIN32)
		[AF_UNIX]	= sizeof (struct sockaddr_un),
#endif
	};

	return table[af];
} /* dns_af_len() */

#define dns_sa_family(sa)	(((struct sockaddr *)(sa))->sa_family)

#define dns_sa_len(sa)		dns_af_len(dns_sa_family(sa))

void hexdump(const unsigned char *src, size_t len, FILE *fp) {
	static const unsigned char hex[]	= "0123456789abcdef";
	static const unsigned char tmpl[]	= "                                                    |                |\n";
	unsigned char ln[sizeof tmpl];
	const unsigned char *sp, *se;
	unsigned char *h, *g;
	unsigned i, n;

	sp	= src;
	se	= sp + len;

	while (sp < se) {
		memcpy(ln, tmpl, sizeof ln);

		h	= &ln[2];
		g	= &ln[53];

		for (n = 0; n < 2; n++) {
			for (i = 0; i < 8 && se - sp > 0; i++, sp++) {
				h[0]	= hex[0x0f & (*sp >> 4)];
				h[1]	= hex[0x0f & (*sp >> 0)];
				h	+= 3;

				*g++	= (isgraph(*sp))? *sp : '.';
			}

			h++;
		}

		fputs((char *)ln, fp);
	}

	return /* void */;
} /* hexdump() */

static void dns_p_dump3(struct dns_packet *P, struct dns_rr_i *I, FILE *fp) {
	enum dns_section section;
	struct dns_rr rr;
	int error;
	union dns_any any;
	char pretty[sizeof any * 2];
	size_t len;

	fputs(";; [HEADER]\n", fp);
	fprintf(fp, ";;     qr : %s(%d)\n", (dns_header(P)->qr)? "RESPONSE" : "QUERY", dns_header(P)->qr);
	fprintf(fp, ";; opcode : %s(%d)\n", dns_stropcode(dns_header(P)->opcode), dns_header(P)->opcode);
	fprintf(fp, ";;     aa : %s(%d)\n", (dns_header(P)->aa)? "AUTHORITATIVE" : "NON-AUTHORITATIVE", dns_header(P)->aa);
	fprintf(fp, ";;     tc : %s(%d)\n", (dns_header(P)->tc)? "TRUNCATED" : "NOT-TRUNCATED", dns_header(P)->tc);
	fprintf(fp, ";;     rd : %s(%d)\n", (dns_header(P)->rd)? "RECURSION-DESIRED" : "RECURSION-NOT-DESIRED", dns_header(P)->rd);
	fprintf(fp, ";;     ra : %s(%d)\n", (dns_header(P)->ra)? "RECURSION-ALLOWED" : "RECURSION-NOT-ALLOWED", dns_header(P)->ra);
	fprintf(fp, ";;  rcode : %s(%d)\n", dns_strrcode(dns_header(P)->rcode), dns_header(P)->rcode);

	section	= 0;

	while (dns_rr_grep(&rr, 1, I, P, &error)) {
		if (section != rr.section)
			fprintf(fp, "\n;; [%s:%d]\n", dns_strsection(rr.section), dns_p_count(P, rr.section));

		if ((len = dns_rr_print(pretty, sizeof pretty, &rr, P, &error)))
			fprintf(fp, "%s\n", pretty);

		section	= rr.section;
	}
} /* dns_p_dump3() */

static void print_packet(struct dns_packet *P, FILE *fp) {
	dns_p_dump3(P, dns_rr_i_new(P, .sort = MAIN.sort), fp);

	if (MAIN.verbose > 2)
		hexdump(P->data, P->end, fp);
} /* print_packet() */

void try(struct dns_packet *Q) {
	// Q->data - 2
	// Q->end
	// Q->size (size of allocated memory)

	struct ipaddr addr;
	handle sock;
	int rc, sz;
	struct dns_packet *A = dns_p_new(2024);
	uint8_t *buffer = A->data - 2, B[512];

	ipaddr_remote(&addr, "10.0.10.5", 53, IPADDR_IPV4, -1);
	sock = tcp_connect(&addr, -1);

	assert(-1 != sock);

	buffer[0] = 0;
	buffer[1] = Q->end;

	rc = bsend(sock, buffer, 2, -1);

	assert(-1 != rc);

	rc = bsend(sock, Q->data, Q->end, -1);

	assert(-1 != rc);

	rc = brecv(sock, buffer, 2, -1);

	assert(-1 != rc);

	sz = buffer[1] | (buffer[0] << 8);

	printf("SZ: %d\n", sz);

	rc = brecv(sock, buffer + 2, sz, -1);

	assert(-1 != rc);

	printf("QID: %d\n", Q->header.qid);
	hexdump(buffer, sz + 2, stdout);

	A->end = 12 + sz; // ???

	// dns_header
	struct dns_rr rr;
	union dns_any any;
	int pos = 12, count = 0; rc = 0;
	while (pos < A->end) {
		rc = dns_rr_parse(&rr, pos, A);

		if (rc) {
			break;
		}

		pos = dns_rr_skip(pos, A);

		printf("RC %d\nSECTION: %d\nPOS: %d P: %d LEN: %d END: %d\n", rc, rr.section, pos, rr.dn.p, rr.dn.len, A->end);
		printf("TYPE: %d CLASS: %d\nP: %d LEN: %d\n", rr.type, rr.class, rr.rd.p, rr.rd.len);
		hexdump(A->data + rr.dn.p, rr.dn.len, stdout);
		hexdump(A->data + rr.rd.p, rr.rd.len, stdout);

		dns_any_parse(&any, &rr, A);
		sz = dns_any_print(B, 512, &any, rr.type);

		printf("%.*s\n", sz, B);
		count++;
	}

	printf("Count: %d %04x %04x %04x %04x\n", count, A->header.qdcount, A->header.ancount, A->header.nscount, A->header.arcount);
}

void * setPort(struct sockaddr_storage *ss, uint16_t port) {
	port = htons(port);

	switch (ss->ss_family) {
	case AF_INET6:
		((struct sockaddr_in6*)ss)->sin6_port = port;
		return &((struct sockaddr_in6*)ss)->sin6_addr;
	case AF_INET:
		((struct sockaddr_in*)ss)->sin_port = port;
		return &((struct sockaddr_in*)ss)->sin_addr;
	}

	return NULL;
}

int main(int argc, char *argv[]) {
	struct dns_packet *A, *Q	= dns_p_new(512);
	char host[INET6_ADDRSTRLEN + 1];
	struct sockaddr_storage ss;
	struct dns_socket *so;
	int error, type;
	void *addrp;
	char *nshost = NULL;

	MAIN.qtype = DNS_T_A;
	MAIN.qname	= "fossunleashed.org";

	printf("%d\n", argc);

	ARGBEGIN {
	case 't':
		type = SOCK_STREAM;
		break;
	case 'u':
		type = SOCK_DGRAM;
		break;
	case 'z':
		switch (resconf()->options.tcp) {
		case DNS_RESCONF_TCP_ONLY:
			type = SOCK_STREAM;
			break;
		case DNS_RESCONF_TCP_DISABLE:
			type = SOCK_DGRAM;
			break;
		default:
			type = 0;
		}
	case 'n':
		MAIN.qname	= ARGF();
		break;
	case 'h':
		nshost = ARGF();
		break;
	} ARGEND;

	if (NULL != nshost) {
		ss.ss_family	= (strchr(nshost, ':'))? AF_INET6 : AF_INET;

		addrp = setPort(&ss, 53);

		if (1 != inet_pton(ss.ss_family, nshost, addrp))
			panic("%s: %s", nshost, dns_strerror(errno));
	} else {
		memcpy(&ss, &resconf()->nameserver[0], dns_sa_len(&resconf()->nameserver[0]));

		addrp = setPort(&ss, 53);
	}

	if (!inet_ntop(ss.ss_family, addrp, host, sizeof host))
		panic("bad host address, or none provided");

	if ((error = dns_p_push(Q, DNS_S_QD, MAIN.qname, strlen(MAIN.qname), MAIN.qtype, DNS_C_IN, 0, 0)))
		panic("dns_p_push: %s", dns_strerror(error));

	//dns_packet
	// dns_so_tcp_send
	// dns_so_tcp_recv

	Q->header.qid = 31337;

	printf("%d %d\n", Q->size, Q->end);
	hexdump(Q->data, Q->end, stdout);

	try(Q);

	dns_header(Q)->rd	= 1;

	fprintf(stderr, "querying %s for %s IN %s\n", host, MAIN.qname, dns_strtype(MAIN.qtype));

	if (!(so = dns_so_open((struct sockaddr *)&resconf()->iface, type, dns_opts(), &error)))
		panic("dns_so_open: %s", dns_strerror(error));

	while (!(A = dns_so_query(so, Q, (struct sockaddr *)&ss, &error))) {
		if (error != EAGAIN)
			panic("dns_so_query: %s (%d)", dns_strerror(error), error);
		if (dns_so_elapsed(so) > 10)
			panic("query timed-out");

		dns_so_poll(so, 1);
	}

	print_packet(A, stdout);

	dns_so_close(so);

	dns_resconf_dump(resconf(), stdout);

	return 0;
}
