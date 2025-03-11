#include <dill/all.h>
#include <assert.h>
#include <dill/conf.h>
#include <r9.h>
#include <newniine.h>

#include <stdio.h>

ircClient _cli;

extern BufFile fmotd, f001, f002, f003, f004, f005, fraw;
extern R9file fready;

coroutine void listen9();

void fs_setup(ircClient *cli);

handle irc_connect(ircClient *cli) {
	struct ipaddr addr;
	handle _h;
	
	ipaddr_remote(&addr, "irc.slashnet.org", 6667, IPADDR_IPV4, -1);

	_h = tcp_connect(&addr, -1);

	assert(-1 != _h);

	_h = suffix_attach(_h, "\n", 1);

	assert(-1 != _h);

	// TODO: move this outside, send correct nick on connect
	strcpy(cli->nick.nick, "TESTER");

	// TODO: check if this worked
	msend(_h, "NICK :TESTER\r", 13, -1);
	msend(_h, "USER test 0 * :I am a test client\r", 33, -1);

	bufFile_clear(&cli->fmotd);
	bufFile_clear(&cli->f005);
	// Maybe: bufFile_clear(&cli->fraw);

	return _h;
}

int main(int argc, char **argv) {
	int rc;

	struct anyconf cf = {};
	char buffer[1 << 14], outbuf[512], *out, *cmd;
	size_t sz;
	ircClient *cli = &_cli;

	cli->h = irc_connect(cli);

	fs_setup(cli);

	go(listen9());

	forever {
		sz = mrecv(cli->h, buffer, 1 << 14, -1);

		if (0 == sz) {
			continue;
		} else if (-1 == sz) {
			if (EPIPE == errno) {
				msleep(now() + 120); // TODO: exponential backoff

				cli->h = irc_connect(cli);

				if (-1 != cli->h) {
					continue;
				}
			}

			printf("ERROR %d\n", errno);
			return 1;
		}

		if ('\r' == buffer[sz - 1]) {
			sz--;
		}
		buffer[sz] = 0;

		bufFile_linewrite(&cli->fraw, buffer, buffer + sz);
		irctok(buffer, &cf);

		cmd	= cf.token[IRC_COMMAND];

		if ('0' == cmd[0] && '0' == cmd[1]) {
			switch (cmd[2]) {
			case '1':
				bufFile_linewrite(&cli->f001, cf.input, buffer + sz);
				continue;
			case '2':
				bufFile_linewrite(&cli->f002, cf.input, buffer + sz);
				continue;
			case '3':
				bufFile_linewrite(&cli->f003, cf.input, buffer + sz);
				continue;
			case '4':
				bufFile_linewrite(&cli->f004, cf.input, buffer + sz);
				continue;
			case '5':
				bufFile_linewrite(&cli->f005, cf.input, buffer + sz);
				continue;
			}
		} else {
			irctokargs(&cf);
		}

		if (0 == strcasecmp("PING", cmd)) {
			out = outbuf;
			out = stpcpy(out, "PONG :");
			if (cf.count >= IRC_ARGS) {
				out = stpcpy(out, cf.token[IRC_ARGS]);
			}
			*out++ = '\r';
			msend(cli->h, outbuf, out - outbuf, -1);
		} else if (0 == strcasecmp("PRIVMSG", cmd)) {
			flux_r9tagFlushAll(&cli->fready);
			tokprint(&cf);
		} else if (0 == strcasecmp("JOIN", cmd)) {
			ircBuffer *ibuf = ircBufferAlloc(cli);

			assert(NULL != ibuf);

			strcpy(ibuf->name, cf.token[IRC_ARGS]);
		} else if (0 == strcasecmp("372", cmd)) { // MOTD body
			bufFile_linewrite(&cli->fmotd, cf.input, buffer + sz);
			//motdp = stpcpy(motdp, cf.token[IRC_ARGS + 1]);
		} else if (0 == strcasecmp("376", cmd)) { // MOTD end
			;
		} else {
			tokprint(&cf);
		}
	}

	// BEGIN ????
	if (0) {
	FILE * fp;

	fp = fopen("tmptmp", "r");

	rc = fread(buffer, 1, 1 << 14, fp);

	buffer[rc] = 0;

	anyconf_readline(&cf, buffer);

	fclose(fp);
	}
	// END ????

	// @table :prefix command args :final_arg (channel/user)

	// PRIVMSG #channel :This a message that contains spaces
	// PRIVMSG #channel             Valid
	// PRIVMSG #channel Not Valid
	// :nick!~user@host PRIVMSG #channel :Something someone else said

	// peek for @, push null or until space
	// peek for : push null or until space

	return 0;
}
