#include <stdio.h>
#include <dill/all.h>
#include <assert.h>
#include <r9.h>

R9file *sw_seek(R9file * f, R9session * s, const char * name);
int sw_list(R9fid * f, C9stat ** st);
void sw_attach(R9session * s);

void sw_init(void);

int main(int argc, char **argv) {
	handle srv, cli;
	struct ipaddr addr;
	R9client *c;
	R9srv	* n9srv = flux_r9getMainSrv();

	flux_r9srvInit(n9srv, sw_seek, sw_list);
	flux_r9srvAttach(n9srv, sw_attach);

	sw_init();

	ipaddr_local(&addr, NULL, 1221, 0);

	srv	= tcp_listen(&addr, 10);

	forever {
		cli	= tcp_accept(srv, NULL, -1);

		c = allocClient(n9srv, cli);

		if_slow (NULL == c) {
			tcp_close(cli, now() + 400);
			continue;
		}

		go(run(c));
	}

	return 0;
}
