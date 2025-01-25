#include <dill/all.h>
#include <assert.h>
#include <dill/conf.h>
#include <r9.h>

#include <stdio.h>

char motd[1 << 16], m1[512], m2[512], m3[512], m4[512], m5[1 << 12], *m5p, *motdp;
handle h;

typedef struct {
	R9file f;
	char * b;
} BufFile;

extern struct {
	R9file f;
	char nick[64], _nick[64];
} nick;

extern BufFile fmotd, f001, f002, f003, f004, f005;

enum {
	IRC_TAGS // IRCv3 table
	,IRC_PREFIX
	,IRC_COMMAND
	,IRC_ARGS
};

void irctok(char * msg, struct anyconf *cf) {
	anyconf_readline(cf, msg);

	char nx = anyconf_nextchar(cf);

	if ('@' == nx) { // IRCv3 TABLE
		anyconf_skip(cf, "@");
		anyconf_scan(cf, "^%s");
		anyconf_skip(cf, "%s");
	} else {
		anyconf_pushnull(cf);
	}

	nx = anyconf_nextchar(cf);

	if (':' == nx) { // PREFIX
		anyconf_skip(cf, ":");
		anyconf_scan(cf, "^%s");
		anyconf_skip(cf, "%s");
	} else {
		anyconf_pushnull(cf);
	}

	anyconf_scan(cf, "^%s");
	anyconf_skip(cf, "%s");
}

void irctokargs(struct anyconf *cf) {
	char nx;

	while (!anyconf_endofline(cf)) {
		nx = anyconf_nextchar(cf);

		if (':' == nx) {
			// Use skipone instead of skip, messages with a leading : would appear like:
			// PRIVMSG #channel ::Hi!
			// Message should be ":Hi!"
			anyconf_skipone(cf, ":");
			anyconf_scan(cf, "^\r\n"); // Message?
		} else {
			anyconf_scan(cf, "^%s");
			anyconf_skip(cf, "%s");
		}
	}
}

void tokprint(struct anyconf *cf) {
	for (int i = 0; i < cf->count; ++i) {
		if (cf->token[i]) {
			printf("%d %s\n", i, cf->token[i]);
		} else {
			printf("%d NULL\n", i);
		}
	}
}

coroutine void listen9();

int main(int argc, char **argv) {
	struct ipaddr addr;
	int rc;

	ipaddr_remote(&addr, "irc.slashnet.org", 6667, IPADDR_IPV4, -1);

	h = tcp_connect(&addr, -1);

	assert(-1 != h);

	h = suffix_attach(h, "\n", 1);

	assert(-1 != h);

	// NICK TESTER
	// USER test 0 * :I am a testclient

	struct anyconf cf = {};
	char buffer[1 << 14], outbuf[512], *out, *cmd, *args;
	size_t sz;

	m5p = m5;
	motdp = motd;
	*motd = 0;

	strcpy(nick.nick, "TESTER");

	msend(h, "NICK :TESTER\n", 13, -1);
	msend(h, "USER test 0 * :I am a test client\n", 33, -1);

	go(listen9());

	forever {
		sz = mrecv(h, buffer, 1 << 14, -1);

		if (0 == sz) {
			continue;
		} else if (-1 == sz) {
			printf("ERROR %d\n", errno);
			return 1;
		}

		if ('\r' == buffer[sz - 1]) {
			buffer[sz - 1] = 0;
		} else {
			buffer[sz] = 0;
		}

		irctok(buffer, &cf);

		cmd	= cf.token[IRC_COMMAND];
		args	= cf.input;

		if ('0' == cmd[0] && '0' == cmd[1]) {
			switch (cmd[2]) {
			case '1':
				args = stpcpy(m1, cf.input);
				*args++ = '\n';
				*args = 0;
				f001.f.st.size = args - m1;
				f001.b = m1;
				continue;
			case '2':
				args = stpcpy(m2, cf.input);
				*args++ = '\n';
				*args = 0;
				f002.f.st.size = args - m2;
				f002.b = m2;
				continue;
			case '3':
				args = stpcpy(m3, cf.input);
				*args++ = '\n';
				*args = 0;
				f003.f.st.size = args - m3;
				f003.b = m3;
				continue;
			case '4':
				args = stpcpy(m4, cf.input);
				*args++ = '\n';
				*args = 0;
				f004.f.st.size = args - m4;
				f004.b = m4;
				continue;
			case '5':
				m5p = stpcpy(m5p, cf.input);
				*m5p++ = '\n';
				f005.f.st.size = m5p - m5;
				f005.b = m5;
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
			msend(h, outbuf, out - outbuf, -1);
		} else if (0 == strcasecmp("PRIVMSG", cmd)) {
			tokprint(&cf);
		} else if (0 == strcasecmp("372", cmd)) { // MOTD body
			motdp = stpcpy(motdp, cf.token[IRC_ARGS + 1]);
			*motdp++ = '\n';
		} else if (0 == strcasecmp("376", cmd)) { // MOTD end
			fmotd.b = motd;
			fmotd.f.st.size = motdp - motd;
			motdp = motd;

			if (m5p != m5) {
				m5p = m5;

				printf("001: %s\n", m1);
				printf("002: %s\n", m2);
				printf("003: %s\n", m3);
				printf("004: %s\n", m4);
				printf("005: %s\n", m5);
			}
		} else {
			tokprint(&cf);
		}
	}

	FILE * fp;

	fp = fopen("tmptmp", "r");

	rc = fread(buffer, 1, 1 << 14, fp);

	buffer[rc] = 0;

	anyconf_readline(&cf, buffer);

	// @table :prefix command args :final_arg (channel/user)

	// PRIVMSG #channel :This a message that contains spaces
	// PRIVMSG #channel             Valid
	// PRIVMSG #channel Not Valid
	// :nick!~user@host PRIVMSG #channel :Something someone else said

	// peek for @, push null or until space
	// peek for : push null or until space

	fclose(fp);

	return 0;
}
