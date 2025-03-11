#include <newniine.h>
#include <stdio.h>

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

