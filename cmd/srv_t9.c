#include <new.h>
#include <r9.h>
#include <cfg.h>

#include <dill/all.h>
#include <flux/list.h>
#include <flux/str.h>

#include <flux/twitch.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <assert.h>

#define BSZ (1 << 19)

int dill_http_send(int s, const char *msg, size_t sz, int64_t deadline) ;

static void genUserAuthURL(char *cid, char *csid, char *uid, char *buff) {
	sprintf(buff, "/auth/authorize?client_id=%s&client_secret=%s&grant_type=user_token&user_id=%s&scope=channel:manage:broadcast", cid, csid, uid);
}

static void genAppAuthURL(char *cid, char *csid, char *buff) {
	sprintf(buff, "/auth/token?client_id=%s&client_secret=%s&grant_type=client_credentials", cid, csid);
}

typedef enum http_state http_state;
typedef enum http_method http_method;

List head, rq_headers, goal_head;

void t9_attach(R9session * s);
int t9_list(R9fid * f, C9stat **st);
R9file *t9_seek(R9file * f, R9session * s, const char *name);

enum http_state {
	HTTP_STATE_PREREQUEST, // before GET/POST/PASTE/etc
	HTTP_STATE_REQUEST_HEADERS,
	HTTP_STATE_USER_DATA,
	HTTP_STATE_STATUS,
	HTTP_STATE_SERVER_HEADERS,
	HTTP_STATE_DATA,
	HTTP_STATE_ERROR // unrecoverable error
};

enum http_method {
	HTTP_METHOD_NULL,
	HTTP_METHOD_GET,
	HTTP_METHOD_HEAD,
	HTTP_METHOD_POST,
	HTTP_METHOD_PATCH
};

#define HTTP_STATUS_SZ (1 << 10)
#define HTTP_HOST_SZ (1 << 10)
#define HTTP_PATH_SZ (1 << 10)

typedef struct {
	List node;
	char key[256], value[1 << 9];
} http_header;

LIST_POOL(http_header, node, 1 << 8);

http_headerPool headerPool;

http_header *allocHeader(char *k, char *v, List *head) {
	http_header *h = http_headerPoolAllocNode(&headerPool, head);

	assert(h && "Could not allocate header");

	if (k) {
		strcpy(h->key, k);
	}

	if (v) {
		strcpy(h->value, v);
	}

	return h;
}

typedef struct {
	handle h;
	List *rq_headers, rs_headers;
	char host[HTTP_HOST_SZ], status_str[HTTP_STATUS_SZ], path[HTTP_PATH_SZ];
	uint32_t content_length;
	uint16_t status; // 200 == ok, 404 == not found, etc
	http_state state;
	http_method method;
} http_req;

void http_init(http_req *rq, char * host) {
	rq->h = -1;
	rq->rq_headers = NULL;

	rq->path[0] = 0;
	rq->status_str[0] = 0;

	strcpy(rq->host, host);

	rq->content_length	= 0;
	rq->status	= 0;
	rq->state	= HTTP_STATE_PREREQUEST;
	rq->method	= HTTP_METHOD_NULL;

	dill_list_init(&rq->rs_headers);
}

int http_request(http_req *rq, http_method m, List * headers, char * pathfmt, ...) {
	if (NULL == headers || NULL == rq) {
		errno = EINVAL;
		goto error;
	}

	if (HTTP_STATE_PREREQUEST != rq->state) {
		errno = ENOTSUP;
		goto error;
	}

	rq->method = m;
	rq->rq_headers = headers;

	va_list ap;

	va_start(ap, pathfmt);
	vsnprintf(rq->path, HTTP_PATH_SZ, pathfmt, ap);
	va_end(ap);

//	printf("RQ: %s\n", rq->path);

	return 0;
	error:
	return -1;
}

// Thoughts?  Move header list to here as an argument, don't have in object.
int http_connect(Twitch *t, http_req *rq) {
	http_header *cur;
	List *it;

	if (-1 == (rq->h = tcp_connect(&t->addr, -1))) {
		goto error;
	}

	if (-1 == (rq->h = http_attach(rq->h))) {
		goto error;
	}

	// TODO check method
	if (HTTP_METHOD_GET == rq->method) {
		http_sendrequest(rq->h, "GET", rq->path, -1);
	} else if (HTTP_METHOD_PATCH == rq->method) {
		http_sendrequest(rq->h, "PATCH", rq->path, -1);
	} else {
		goto error;
	}

	http_sendfield(rq->h, "Host", rq->host, -1);

	dill_list_T_foreach(rq->rq_headers, http_header, node, it, cur) {
		http_sendfield(rq->h, cur->key, cur->value, -1);

		//printf("[%s] = %s\n", cur->key, cur->value);
	}

	http_done(rq->h, -1); // TODO? check return?

	return 0;
	error:
	return -1;
}

int http_get_headers(http_req *rq) {
	// reason must be big enough.  Server provides the data to fill in here.
	rq->status = http_recvstatus(rq->h, rq->status_str, HTTP_STATUS_SZ, -1);
	size_t len = 0;
	char *b;

	printf("STATUS: %03d - %s\n", rq->status, rq->status_str);

	while (1) {
		http_header *h = allocHeader(NULL, NULL, &rq->rs_headers);

		int rc = http_recvfield(rq->h, h->key, sizeof(h->key), h->value, sizeof(h->value), -1);

		if(rc == -1 && errno == EPIPE) break;
		printf("[%s] = %s\n", h->key, h->value);

		if (!strcasecmp("content-length", h->key)) {
			b = h->value;

			for (; *b; ++b) {
				len = len * 10 + *b - '0';
			}
		}
	}

	rq->content_length = len;

	return 0;
}

void writeChannel(Twitch *t, http_req *rq, char * uid, char * buff, size_t *len_out) {
	http_init(rq, t->host);
	http_request(rq, HTTP_METHOD_PATCH, &rq_headers, "%schannels?broadcaster_id=%s", t->host, uid);

	size_t len;
	char * b;
	http_header *h;

	len = strlen(buff);

	h = allocHeader("Content-Length", NULL, rq->rq_headers);

	sprintf(h->value, "%d", len);

	if (-1 == http_connect(t, rq)) {
		printf("Could not connect %s\n", errno);
		return;
	}

	http_headerPoolFreeNode(&headerPool, h);

	dill_http_send(rq->h, buff, strlen(buff), -1);

	http_get_headers(rq);
	len = rq->content_length;

	rq->h = http_detach(rq->h, -1); // TODO: return check

	if (len) {
		brecv(rq->h, buff, len, -1);
		buff[len] = 0;
	}
	*len_out = len;

	tcp_close(rq->h, -1);
	rq->h = -1;
}

int readGoals(Twitch *t, char * uid, char * buff, size_t *len_out) {
	http_req _rq, *rq = &_rq;

	http_init(&_rq, t->host);
	http_request(&_rq, HTTP_METHOD_GET, &rq_headers, "%sgoals?broadcaster_id=%s", t->base, uid);

	size_t len = 0;
	char * b;

	if (-1 == http_connect(t, rq)) {
		printf("Could not connect %s\n", errno);
		return -1;
	}

	http_get_headers(rq);
	len = rq->content_length;

	rq->h = http_detach(rq->h, -1); // TODO: return check

	if (len) {
		brecv(rq->h, buff, len, -1);
		buff[len] = 0;
	}
	*len_out = len;

	tcp_close(rq->h, -1);
	rq->h = -1;

	return rq->status;
}

int readChannel(Twitch *t, char * uid, char * buff, size_t *len_out) {
	http_req _rq, *rq = &_rq;

	http_init(&_rq, t->host);
	http_request(&_rq, HTTP_METHOD_GET, &rq_headers, "%schannels?broadcaster_id=%s", t->base, uid);

	size_t len = 0;
	char * b;

	if (-1 == http_connect(t, rq)) {
		printf("Could not connect %s\n", errno);
		return -1;
	}

	http_get_headers(rq);
	len = rq->content_length;

	rq->h = http_detach(rq->h, -1); // TODO: return check

	if (len) {
		brecv(rq->h, buff, len, -1);
		buff[len] = 0;
	}
	*len_out = len;

	tcp_close(rq->h, -1);
	rq->h = -1;

	return rq->status;
}

char buffer[BSZ], userBear[BSZ], appBear[BSZ];

typedef struct {
	List list;
	char key[128], value[128];
	int64_t ival;
} kv_node;

kv_node channel[32];

char UN[] = "R";

#define RBUFSZ (1 << 12)
#define WBUFSZ (1 << 9)

void channel_file_end(R9fid *f);
int channel_file_write(R9fid *f, uint32_t, uint8_t *, char**);
void channel_file_read(R9fid *f, C9tag, uint64_t, uint32_t);

typedef struct {
	R9file f;
	JumboJim j;
	char wbuf[WBUFSZ], rbuf[RBUFSZ];
	Twitch *t;
} R9buffer;

R9fileEv channelEv = {
	.on_read = channel_file_read
	,.on_linewrite = channel_file_write
	,.on_clunk = channel_file_end
};

R9buffer channel_f = {
	.f = {
		.st = {
			.uid = UN,
			.gid = UN,
			.muid = UN,
			.name	= "channel",
			.qid = {},
			.mode = 0600
		}
		,.ev	= &channelEv
	},
	.j = {
		.j = {
			.write = jim_writer
		}
	}
};

void json_parse(char *buf, size_t, List *H);

#include <cfg.h>

#define HOST "localhost"
#define PORT 8080

int main(void) {
	size_t len;
	Twitch *t;

	// CLIENTID=r45jxvepzpbugpxvqw7hmp2bdta2yq
	// CLIENTSECRET=

	/*

		// Mock API
	     "ID": "07abe8d773b7c797b67ebe28fc15b3",
      "Secret": "ae0c8b8c36069834e8deadc194751d",

		21673494

		8ca9302d4c2baf9 -- Auth

		// REAL DO NOT STREAM THIS KEY BELOW:

	// */

	if (0) {
		t = flux_twitch_create();

		configParseLine(&t->cfg, "client_id ");
	} else {
		t = flux_twitch_mock_create(HOST, PORT);

		configParseLine(&t->cfg, "client_id 07abe8d773b7c797b67ebe28fc15b3");
		configParseLine(&t->cfg, "client_secret ae0c8b8c36069834e8deadc194751d");
		configParseLine(&t->cfg, "user_id 21673494");
		configParseLine(&t->cfg, "user_token 8ca9302d4c2baf9");
	}

	channel_f.t = t;

	strcpy(userBear, "Bearer ");
	strcat(userBear, t->userToken);

	// List setup
	dill_list_init(&head);
	dill_list_init(&goal_head);
	dill_list_init(&rq_headers);

	http_headerPoolInit(&headerPool);

	for (int i = 0; i < 11; ++i) {
		dill_list_insert(&channel[i].list, &head);
		channel[i].ival = -1;
	}

	strcpy(channel[0].key, "broadcaster_id");
	strcpy(channel[1].key, "broadcaster_login");
	strcpy(channel[2].key, "broadcaster_name");
	strcpy(channel[3].key, "game_id");
	strcpy(channel[4].key, "game_name");
	strcpy(channel[5].key, "title");
	strcpy(channel[6].key, "broadcaster_language");
	strcpy(channel[7].key, "delay");
	strcpy(channel[8].key, "tags");
	strcpy(channel[9].key, "is_branded_content");
	strcpy(channel[10].key, "content_classification_labels");

/*	"id": "3cc9dd14-aa52-69a7-2e7d-7ce7b566f06b",
      "broadcaster_id": "74944423",
      "broadcaster_name": "FisherChief722",
      "broadcaster_login": "fisherchief722",
      "type": "new_subscription_count",
      "description": "Lets get to 857 subs!",
      "current_amount": 780,
      "target_amount": 857,
      "created_at": "2024-06-18T10:03:46-06:00" // */

	// goals
	strcpy(channel[11].key, "id");
	strcpy(channel[12].key, "broadcaster_id");
	strcpy(channel[13].key, "broadcaster_name");
	strcpy(channel[14].key, "broadcaster_login");
	strcpy(channel[15].key, "type");
	strcpy(channel[16].key, "description");
	strcpy(channel[17].key, "current_amount");
	strcpy(channel[18].key, "target_amount");
	strcpy(channel[19].key, "created_at");

	for (int i = 11; i < 20; ++i) {
		dill_list_insert(&channel[i].list, &goal_head);
	}

	if (0) {
		genUserAuthURL(t->clientId, t->clientSecret, t->userId, buffer);
		printf("%s\n", buffer);	

		genAppAuthURL(t->clientId, t->clientSecret, buffer);
		printf("%s\n", buffer);
	}

	// five header chains?  Auth | (user_tok app_tok)^(accept content-type)

	allocHeader("Client-Id", t->clientId, &rq_headers);
	allocHeader("Authorization", userBear, &rq_headers);
	allocHeader("Accept", "application/json", &rq_headers);
	allocHeader("Content-Type", "application/json", &rq_headers);

	int status;

	status = readChannel(t, t->userId, buffer, &len);
	if (status > 199 && 300 > status) {
		json_parse(buffer, len, &head);
	} else {
		printf("ERROR: %s\n", buffer);
	}

	status = readGoals(t, t->userId, buffer, &len);
	if (status > 199 && 300 > status) {
		json_parse(buffer, len, &goal_head);
	} else {
		printf("ERROR: %s\n", buffer);
	}

	handle srv, cli;
	struct ipaddr addr;
	R9client *c;
	R9srv * srv9 = flux_r9getMainSrv();

	flux_r9srvInit(srv9, t9_seek, t9_list);
	flux_r9srvAttach(srv9, t9_attach);

	ipaddr_local(&addr, NULL, 5533, 0);

	srv  = tcp_listen(&addr, 10);

	forever {
		cli  = tcp_accept(srv, NULL, -1);

		c = allocClient(srv9, cli);

		if_slow (NULL == c) {
			tcp_close(cli, now() + 400);
			continue;
		}

		go(run(c));
	}

	exit:
	tcp_close(srv, now() + 400);

	return 0;
}

		/*
			Keys:
				broadcaster_id
				broadcaster_login
				broadcaster_name
				game_id
				game_name
				title
				broadcaster_language
				delay
				tags
				is_branded_content
				content_classification_labels
		// */

void json_parse(char *buf, size_t len, List *H) {
	if (len) {
		jsmn_parser parser;
		jsmntok_t tokens[512];

		jsmn_init(&parser);

		len = jsmn_parse(&parser, buffer, len, tokens, 512);
		char subBuf[BSZ];

		kv_node *cur = NULL;
		List *it;

		// TODO: sanity check of JSON
		// token[0] = {...} OBJ
		// token[1] = {"data": ...} STRING == "data"
		// token[2] = {...: [...]} ARRAY
		// token[3] = {...: [{...}]} OBJ

		for (int i = 4; i < len; ++i) {
			jsmntok_t *T = tokens + i;
			size_t sublen = T->end - T->start;

			if (JSMN_ARRAY == T->type) {
				if (cur) {
					cur->value[0] = 0;
					cur = NULL;
				}
				i += T->size;
				continue;
			}

			if (JSMN_PRIMITIVE == T->type) {
				// number
				// boolean
				// null

				if (!cur) {
					printf("No key for primitive\n");
					continue;
				}

				char *s = buf + T->start, *e = buf + T->end, *p = s;

				if ('N' == *s || 'n' == *s) { // null
					cur->value[0] = 0;
					cur->ival = 0;
				} else if ('F' == *s || 'f' == *s) { // false
					cur->value[0] = '0';
					cur->value[1] = 0;
					cur->ival = 0;
				} else if ('T' == *s || 't' == *s) { // true
					cur->value[0] = '1';
					cur->value[1] = 0;
					cur->ival = 1;
				} else { // number
					int j;

					for (j = 0; p < e; ++p, ++j) {
						cur->value[j] = *p;
						cur->ival = cur->ival * 10 + *p - '0';
					}
					cur->value[j] = 0;
				}
			} else if (JSMN_STRING == T->type) {
				memcpy(subBuf, buf + T->start, sublen);
				subBuf[sublen] = 0;

				if (T->size) {
					dill_list_foreach(H, it) {
						cur = (kv_node *)it;
						if (!strcmp(cur->key, subBuf)) {
							it = NULL;
							cur->ival = 0;
							break;
						}
					}
					if (it) {
						printf("Could not find key\n");
						cur = NULL;
					}
					continue;
				}

				if (cur) {
					strcpy(cur->value, subBuf);
					cur = NULL;
				} else {
					printf("No kv_node found to assign!\n");
				}
			} else {
				if (cur) {
					cur->value[0] = 0;
					cur = NULL;
				}
				printf("Type: %d\tS: %d\tE: %d\tSZ: %d\n", T->type, T->start, T->end, T->size);
			}
		}
	}
}

void channel_file_end(R9fid *f) {
	R9buffer *b = dill_cont(f->file, R9buffer, f);
	size_t len;
	http_req rq;

	if (f->isWrite && b->j.buf) {
		jim_object_end(&b->j.j);
		b->j.buf[0] = 0;

		writeChannel(b->t, &rq, b->t->userId, b->wbuf, &len);

		printf("%d %s\n", len, b->wbuf);

		b->j.sz = 0;
		b->j.buf = NULL;
	}
}

int channel_file_write(R9fid *f, uint32_t count, uint8_t * data, char **err) {
	char buf[1 << 12], *val;
	R9buffer *b = dill_cont(f->file, R9buffer, f);

	if (5 > count) {
		printf("COUNT: %d\n", count);
		goto error;
	}

	if (0 > flux_str_parse_ctl(data, buf, count, 1 << 12, &val)) {
		goto error;
	}

	if (!b->j.buf) {
		b->j.buf = b->wbuf;
		b->j.sz	= WBUFSZ;
		b->j.j.sink = &b->j;

		jim_object_begin(&b->j.j);
	}

	jim_member_key(&b->j.j, buf);
	jim_string(&b->j.j, val);

	printf("[%s] = %s\n", buf, val);

	// do the update request

	return 0;
	error:
	return -1;
}

void goal_file_read(R9fid * f, C9tag tag, uint64_t offset, uint32_t count) {
	char buf[1 << 12];
	List * it;
	kv_node *cur;
	size_t sz = 0;

	if (offset) {
		// Send EOF
		s9read(&f->s->c->ctx, tag, NULL, 0);
	}

	dill_list_foreach(&goal_head, it) {
		cur = (kv_node *)it;

		sz += sprintf(buf + sz, "%s\t%s\n", cur->key, cur->value);
	}

	if (count >= sz) {
		s9read(&f->s->c->ctx, tag, buf, sz);
	} else {
		s9read(&f->s->c->ctx, tag, buf, count);
	}
}

void channel_file_read(R9fid * f, C9tag tag, uint64_t offset, uint32_t count) {
	char buf[1 << 12];
	List * it;
	kv_node *cur;
	size_t sz = 0;

	if (offset) {
		// Send EOF
		s9read(&f->s->c->ctx, tag, NULL, 0);
	}

	dill_list_foreach(&head, it) {
		cur = (kv_node *)it;

		sz += sprintf(buf + sz, "%s\t%s\n", cur->key, cur->value);
	}

	if (count >= sz) {
		s9read(&f->s->c->ctx, tag, buf, sz);
	} else {
		s9read(&f->s->c->ctx, tag, buf, count);
	}
}

R9file root = {
	.st = {
		.uid = UN,
		.gid = UN,
		.muid = UN,
		.name	= ".",
		.qid = {.type = C9qtdir},
		.mode = 0500 | C9stdir
	}
};

R9fileEv goalEv = {
	.on_read = goal_file_read
};

R9file goal_f = {
	.st = {
		.uid = UN,
		.gid = UN,
		.muid = UN,
		.name	= "goals",
		.qid = {.path = 1},
		.mode = 0400
	}
	,.ev	= &goalEv
};

void t9_attach(R9session * s) {
	s->fid.file = &root;
}

int t9_list(R9fid * f, C9stat **st) {
	if (&root == f->file) {
		st[0] = &channel_f.f.st;
		st[1] = &goal_f.st;

		return 2;
	}

	return 0;
}

R9file *t9_seek(R9file * f, R9session * s, const char *name) {
	if (&root == f) {
		if (!strcmp(channel_f.f.st.name, name)) {
			return &channel_f.f;
		}
		if (!strcmp(goal_f.st.name, name)) {
			return &goal_f;
		}
	}

	return NULL;
}
