#pragma once

#include <r9.h>
#include <stdint.h>
#include <flux/tree.h>
#include <dill/conf.h>
#include <flux/time.h>

enum {
	IRC_TAGS // IRCv3 table
	,IRC_PREFIX
	,IRC_COMMAND
	,IRC_ARGS
};

enum {
	IRC_BUFFER_ROOT
	,IRC_BUFFER_IN
	,IRC_BUFFER_OUT
	,IRC_BUFFER_NAME
	,IRC_BUFFER_IDNAME
	,IRC_BUFFER_MAXOUT
	,IRC_BUFFER_LOG
	,IRC_BUFFER_RAW
	,IRC_BUFFER_CTL
	,IRC_BUFFER_COUNT
};

struct BufFile {
	R9file f;
	uint8_t *b, *be;
};

struct NickFile {
	R9file f;
	uint8_t nick[64], _nick[64];
};

struct ircBuffer {
	R9file files[IRC_BUFFER_COUNT];
	uint8_t name[64], idname[16]; // name is pretty name, idname is _pub_##### or _nick_#####
	List list;
	uint8_t id;
	struct ircClient *cli;

	Bit mem : 1;
	Bit chan : 1;
};

struct ircClient {
	handle h;
	struct BufFile fmotd, f001, f002, f003, f004, f005, fraw;
	struct NickFile nick;
	R9file root, fready, inf;
	Tree tree;
	List bufFree, bufActive;
	struct ircBuffer buffers[128];
	uint8_t bufCount;

	uint8_t motd[1 << 16], m1[512], m2[512], m3[512], m4[512], m5[1 << 12], logbuffer[1 << 18], UN[64];
};

typedef struct BufFile BufFile;
typedef struct NickFile NickFile;
typedef struct ircClient ircClient;
typedef struct ircBuffer ircBuffer;

void ircClientInit(ircClient *cl);
ircClient *getClientFromFile(R9file * rf);
ircClient *getClientById(uint16_t id);

uint8_t *bufFile_linewrite(BufFile * file, uint8_t *buf, uint8_t *be);
uint8_t *bufFile_write(BufFile * file, uint8_t *buf, uint8_t *be);
void bufFile_clear(BufFile * file);

#define bufFile_getpos(bf) ((bf)->b + (bf)->f.st.size)

ircBuffer *ircBufferAlloc(ircClient *cli);
void ircBufferInit(ircBuffer *buf);
void ircBufferFree(ircBuffer *buf);
ircClient *ircClientFromBuffer(ircBuffer *buf);
ircBuffer *ircBufferFromFile(R9file *);
int ircBufferFmt(ircBuffer *buf, const char * const fmt, ...);

#define chanfmt(c, fmt, ...) ircBufferFmt(c, "%lu " fmt "\n", flux_s(NULL), __VA_ARGS__)

#define clientMaskPath(clientid, val) (((val) & 0xFFFFFFFF) | ((~(clientid) & 0xFFFF) << 48))
#define clientUnmaskPath(path) (~(((path) & 0xFFFF000000000000) >> 48) & 0xFFFF)

void irctok(char * msg, struct anyconf *cf);
void irctokargs(struct anyconf *cf);
void tokprint(struct anyconf *cf);
