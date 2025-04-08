#include <r9.h>
#include <time.h>
#include <dill/all.h>
#include <flux/time.h>
#include <flux/ulid.h>

flux_isaac32_ctx rng;

uint8_t UN[] = "R", buf[1 << 14];

#define ST_DEFAULTS \
	.uid	= UN, \
	.gid	= UN, \
	.muid	= UN, \
	.atime	= 0, \
	.mtime	= 0

void rf_beat_readx(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	uint32_t sit_ts = flux_sit();

	buf[7] = '\n';
	buf[6] = 0x30 + (sit_ts % 10);
	sit_ts /= 10;
	buf[5] = 0x30 + (sit_ts % 10);
	sit_ts /= 10;
	buf[4] = '.';
	buf[3] = 0x30 + (sit_ts % 10);
	sit_ts /= 10;
	buf[2] = 0x30 + (sit_ts % 10);
	sit_ts /= 10;
	buf[1] = 0x30 + (sit_ts % 10);
	buf[0] = '@';

	s9read(&f->s->c->ctx, tag, buf, 8);
}

void rf_beat_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	uint32_t sit_ts = flux_sit() / 100;

	buf[4] = '\n';
	buf[3] = 0x30 + (sit_ts % 10);
	sit_ts /= 10;
	buf[2] = 0x30 + (sit_ts % 10);
	sit_ts /= 10;
	buf[1] = 0x30 + (sit_ts % 10);
	buf[0] = '@';

	s9read(&f->s->c->ctx, tag, buf, 5);
}

void rf_ulid_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	uint8_t *p = flux_ulid32(buf, buf + 32, &rng);

	*p++ = '\n';

	s9read(&f->s->c->ctx, tag, buf, p - buf);
}

R9fileEv beatEv = {
	.on_shortread = rf_beat_read
}, beatxEv = {
	.on_shortread = rf_beat_readx
}, ulidEv = {
	.on_shortread = rf_ulid_read
};

R9file root = {
	.st = {
		.qid	= {.type = C9qtdir},
		.size	= 0,
		.name	= ".",
		.mode	= C9stdir | 0500,
		ST_DEFAULTS
	}
}, rf_beat = {
	.st = {
		.qid	= {},
		.size	= 5,
		.name	= "beat",
		.mode	= 0400,
		ST_DEFAULTS
	}
	,.ev	= &beatEv
}, rf_beatx = {
	.st = {
		.qid	= {.path = 1},
		.size	= 8,
		.name	= "beatx",
		.mode	= 0400,
		ST_DEFAULTS
	}
	,.ev	= &beatxEv
}, rf_ulid = {
	.st = {
		.qid	= {.path = 2},
		.size	= 27,
		.name	= "ulid",
		.mode	= 0400,
		ST_DEFAULTS
	}
	,.ev	= &ulidEv
};

int r9list_tmp(R9fid *f, C9stat **st) {
	if (&root == f->file) {
		st[0] = &rf_beat.st;
		st[1] = &rf_beatx.st;
		st[2]	= &rf_ulid.st;
		return 3;
	}

	return -1;
}

R9file *r9seek_tmp(R9file *rf, R9session *s, const char *str) {
	if (NULL == rf && NULL == str) {
		return &root;
	}

	if (&root == rf) {
		if (0 == strncmp("beatx", str, 5)) {
			return &rf_beatx;
		} else if (0 == strncmp("beat", str, 4)) {
			return &rf_beat;
		} else if (0 == strncmp("ulid", str, 4)) {
			return &rf_ulid;
		}
	}

	return NULL;
}

static uint32_t gen() {
	uint32_t r = 0, bits = 0, tmp;

	while (bits < 32) {
		tmp = (flux_us() == flux_us()) | ((flux_us() == flux_us()) << 1);

		switch (tmp) {
			case 0:
			case 3:
				break;
			case 1:
			case 2:
				r = (r << 1) | (tmp & 1);
				bits++;
		}
	}

	return r;
}

int main(void) {
	handle srv, cli;
	struct ipaddr addr;
	R9client *c;
	int i;
	R9srv * srv9 = flux_r9getMainSrv();
	flux_r9srvInit(srv9, r9seek_tmp, r9list_tmp);

	// RNG SEED

	for (i = 0; i < FLUX_ISAAC_RANDSIZ; i++) {
		rng.randrsl[i] = gen();
	}

	isaac32_init(&rng, 1);

	ipaddr_local(&addr, NULL, 1312, 0);

	srv	= tcp_listen(&addr, 10);

	for (i = 4; i > 0; --i) {
		cli	= tcp_accept(srv, NULL, -1);

		c = allocClient(srv9, cli, tcp_close);

		if_slow (NULL == c) {
			tcp_close(cli, now() + 400);
			continue;
		}

		if (1 == i) {
			run(c);
		} else {
			go(run(c));
		}
	}

	tcp_close(srv, now() + 400);

	return 0;
}
