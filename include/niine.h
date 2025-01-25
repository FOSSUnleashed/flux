/* See LICENSE file for license details. */
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <args.h>

#include <new.h>

#include <assert.h>

#include <dill/core.h>
#include <dill/msock.h>
#include <dill/bsock.h>
#include <dill/ip.h>
#include <dill/tcp.h>
#include <dill/util.h>
#include <dill/suffix.h>

#include <r9.h>

#define IRC_CHANNEL_MAX   200
#define IRC_NICK_MAX      200
#define IRC_MSG_MAX       (1 << 14) /* Big enough to contain ircv3 @-tables; libdill handles the PIPE_BUF code */
#define PING_TIMEOUT      300
#define UMODE_MAX          10
#define CMODE_MAX          50

enum { TOK_NICKSRV = 0, TOK_USER, TOK_CMD, TOK_CHAN, TOK_ARG, TOK_TEXT, TOK_LAST };

typedef struct Nick Nick;
struct Nick {
	char name[IRC_NICK_MAX];
	char prefix;
	Nick *next;
};

typedef struct Channel Channel;
struct Channel {
	handle fdin, cr, ch[2];
	char name[IRC_CHANNEL_MAX]; /* channel name (normalized) */
	char inpath[PATH_MAX];      /* input path */
	char outpath[PATH_MAX];     /* output path */
	Nick *nicks;
	Channel *next;
	R9file f;
};

// GPL Functions

enum {
	FILE_001,
	FILE_002,
	FILE_003,
	FILE_004,
	FILE_005,
	FILE_list,
	FILE_modes,
	FILE_motd
};

int chan_printf(Channel *, const char * const, ...);
#define chanfmt(c, fmt, ...) chan_printf(c, "%lu " fmt "\n", time(NULL), __VA_ARGS__)

#define TIMEOUT	-1
coroutine void      niine_run(int, int, const char * const);

int start(const char * const);

void channel_run(Channel * c);
void cleanquit();

// <9p>

int *ixp_start(int b);

// </9p>

// </GPL>

void      cap_parse(char *);
Channel * channel_add(const char *);
Channel * channel_find(const char *);
Channel * channel_join(const char *);
void      channel_leave(Channel *);
Channel * channel_new(const char *);
void      channel_normalize_name(char *);
void      channel_normalize_path(char *);
int       channel_open(Channel *);
void      channel_print(Channel *, const char *);
int       channel_reopen(Channel *);
void      channel_rm(Channel *);
void      create_dirtree(const char *);
void      create_filepath(char *, size_t, const char *, const char *, const char *);
void      ewritestr(int, const char *);
void      handle_channels_input(int, Channel *);
void      handle_server_output(int, int);
void      loginkey(int, const char *);
void      loginuser(int, const char *, const char *, const char *);
#define name_add(c, n) name_add3((c), (n), '\0')
void      name_add3(const char *, const char *, const char);
Nick *    name_find(Channel *, const char *);
void      name_menick(const char *, const char *);
void      name_mode(const char *, char *, char *);
void      name_nick(const char *, const char *);
void      name_quit(const char *, const char *, const char *);
int       name_rm(const char *, const char *);
int       name_rm3(Channel *, const char *, char *);
void      parse_cmodes(char *);
void      parse_prefix(char *);
void      proc_channels_input(int, Channel *, char *);
void      proc_channels_privmsg(int, Channel *, char *);
void      proc_names(const char *, char *);
void      proc_server_cmd(int, char *);
int       ptr_split(const char *, const char *, const char *, const char *);
int       read_line(int, char *, size_t);
void      setup(void);
void      sighandler(int);
int       tcpopen(const char *, const char *);
void      tokenize(char **, char *);
int       udsopen(const char *);
void      usage(void);

extern int      isrunning;
extern time_t   last_response;
extern Channel *channels;
extern Channel *channelmaster;
extern char     nick[32];          /* active nickname at runtime */
extern char     _nick[32];         /* nickname at startup */
extern char     ircpath[PATH_MAX]; /* irc dir (-i) */
extern char     msg[IRC_MSG_MAX];  /* message buf used for communication */
extern int      trackprefix;   /* flag to track user prefixes */
extern char     upref[UMODE_MAX];  /* user prefixes in use on this server */
extern char     umodes[UMODE_MAX]; /* modes corresponding to the prefixes */
extern char     cmodes[CMODE_MAX]; /* channel modes in use on this server */
extern char * argv0;

#define msgfmt(fmt, ...) snprintf(msg, IRC_CHANNEL_MAX, fmt, __VA_ARGS__)

#define IS_CHAN_PREFIX(c) (((c) == '#') || ((c) == '&') || ((c) == '+') || ((c) == '!'))

#define eprintf(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__)
#define die(fmt, ...) do { fprintf(stderr, fmt, __VA_ARGS__); exit(1); } while (0)
