#include <stdio.h>
#include <snow.h>
#include <string.h>

typedef struct msg Msg;
struct msg {
	char *s, *v3, *cmd, *src, *dst, *txt;
};

#define IS_IRC_SPACE(c) (' ' == (c) || '\t' == (c))
#define ADVANCE for (; !IS_IRC_SPACE(s[i]); ++i); for(; IS_IRC_SPACE(s[i]); ++i) { m->s[i] = 0; }

void parse(Msg * m, const char * s) {
	m->s	= strdup(s);
	int i = 0;

	m->v3	=
	m->cmd	=
	m->src	=
	m->dst	=
	m->txt	= NULL;

	if ('@' == s[i]) {
		// We have an IRCv3 part
		m->v3 = m->s + 1;
		ADVANCE;
	}

	if (':' == s[i]) {
		// Prefix
		m->src = m->s + i + 1;
		ADVANCE;
	}

	m->cmd	= m->s + i;
	ADVANCE;
	m->dst	= m->s + i;
	ADVANCE;
	m->txt	= m->s + i;

	if (':' == *m->txt) {
		m->txt++;
	}
}

void msgfree(Msg * m) {
	free(m->s);
}

describe(msg) {
	Msg m;

	it("simple") {
		char * str = ":nick!user@host PRIVMSG #place :make it bigger";
		parse(&m, str);

		asserteq(m.v3, NULL);
		asserteq_str(m.cmd, "PRIVMSG");
		asserteq_str(m.src, "nick!user@host");
		asserteq_str(m.dst, "#place");
		asserteq_str(m.txt, "make it bigger");

		msgfree(&m);
	}

	it ("twitch") {
		char * str = "@badge-info=;badges=;color=;display-name=thankyouler;emotes=;first-msg=0;flags=;id=7b798216-020a-4bde-ab5b-17476fd7c4a1;mod=0;returning-chatter=0;room-id=59227136;subscriber=0;tmi-sent-ts=1722768351008;turbo=0;user-id=247057834;user-type= :nick!user@host PRIVMSG #abby :car soccar";
		parse(&m, str);

		assertneq(m.v3, NULL);
		asserteq_str(m.cmd, "PRIVMSG");
		asserteq_str(m.src, "nick!user@host");
		asserteq_str(m.dst, "#abby");
		asserteq_str(m.txt, "car soccar");

		msgfree(&m);
	}

	it ("005.1") {
		char * str = ":reflection.oftc.net 005 Xion CALLERID CASEMAPPING=rfc1459 DEAF=D KICKLEN=160 MODES=4 NICKLEN=30 PREFIX=(ov)@+ STATUSMSG=@+ TOPICLEN=391 NETWORK=OFTC MAXLIST=beI:100 MAXTARGETS=1 CHANTYPES=# :are supported by this server";
		parse(&m, str);

		asserteq_str(m.cmd, "005");
		asserteq_str(m.src, "reflection.oftc.net");

		msgfree(&m);
	}

	it ("005.2") {
		char * str = ":reflection.oftc.net 005 Xion CHANLIMIT=#:250 CHANNELLEN=50 CHANMODES=eIqb,k,l,cimnpstzMRS AWAYLEN=160 KNOCK ELIST=CMNTU SAFELIST EXCEPTS=e INVEX=I :are supported by this server";
		parse(&m, str);

		asserteq_str(m.cmd, "005");
		asserteq_str(m.src, "reflection.oftc.net");

		msgfree(&m);
	}
}

snow_main();
