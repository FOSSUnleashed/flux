#include <r9.h>
#include <time.h>
#include <dill/all.h>
#include <flux/time.h>
#include <flux/str.h>
#include <flux/list.h>
#include <assert.h>
#include <unistd.h>
#include <flux/isaac.h>
#include <flux/util.h>
#include <stdarg.h>
#include <dnd.h>

#include <stdio.h>

char UN[] = "R", buf[1 << 14];
uint8_t logbuf[1 << 14];

isaac32_ctx rng;
List turn, *T = &turn;

#define RAND() rand32(&rng)

size_t logbuffmt(const char * fmt, ...);

struct Creature *activeTurn();

/*
* **hp**: Int[2]:State Effective current and maximum HP, space delim'd
* **hp.current**: Int:State current real HP
* **hp.max**: Int:State maximum real HP
* **hp.tmp**: Int:State current temporary HP
* **ability**: Int[4]:Multi-state/RPC: Ability table, in order: Base, Modifier,
    Temporary, Effective Modifier.  Writes are accepted to apply level-up
    Ability Score bonuses only.
* **$ability**: Int:State Current effective ability score.
* **$ability_mod**: Int:State Current effective ability modifier.
* **skills**: Int[3]+Boolean:Multi-state/RPC: Skills table, in order: Score, Ranks,
    Modifiers, Class-skill
* **level**: Int:State Current creature level/max-HD.
* **classes/ **:
    * **level**: Int:State Current level in the specified class
    * **caster**: Int:State Effective caster level (if any) for the class
* **map**: State.  Creature's view of the map, affected by `binary_map` in `ctl`.
* **try**: Variable.  A valid action that the creature wishes to try.  This
    file and files that make use of it are AI-helpers.  Player clients are also
    allowed to make use of these files.
* **line.sight**: State, new-line delim'd list of creatures that are available
    for line-of-sight effects.  Affected by `try`
* **line.effect**: State, as `line.sight` but for line-of-effect effects.
* **spells/ **: Directory of spells available for casting, or other actions if
    any.
// */

Creature player, monster, mon2;

#define GET_CREATURE_FILE_TYPE(qid) ((qid).path & 0xFF)

uint8_t *flux_bufwriteint16(uint8_t *buf, uint8_t *bufend, int16_t val) {
	if (NULL == buf || NULL == bufend || buf > bufend) {
		errno = EINVAL;
		return NULL;
	}

	uint8_t tmp, *pos = bufend - 1;

	if (val < 0) {
		*buf++ = '-';
		val = -val;
	}

	while (pos >= buf) {
		tmp = '0' + val % 10;
		*pos-- = tmp;

		val /= 10;

		if (0 == val) {
			break;
		}
	}

	for (++pos; pos < bufend; ++pos, ++buf) {
		*buf = *pos;
	}

	return buf;
}

#define LF '\n'
#define LF1 LF, LF
#define LF2 LF1, LF1
#define LF3 LF2, LF2
#define LF4 LF3, LF3

void hp_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	uint8_t type = GET_CREATURE_FILE_TYPE(f->file->st.qid);
	uint8_t buf[] = {LF4, LF4}, *be = buf + sizeof(buf), *b;
	size_t dlen = 2;
	struct CreatureFile *cf = dill_cont(f->file, struct CreatureFile, file);
	struct Creature * cr = cf->cr;

	if (CREATURE_HP == type) {
		b = flux_bufwriteint16(buf, be, cr->hp.current);
		*b++ = ' ';
		b = flux_bufwriteint16(b, be, cr->hp.max);
		dlen	= 1 + (b - buf);
	} else if (CREATURE_HP_CURRENT == type) {
		b = flux_bufwriteint16(buf, be, cr->hp.current);
		dlen	= 1 + (b - buf);
	} else if (CREATURE_HP_MAX == type) {
		b = flux_bufwriteint16(buf, be, cr->hp.max);
		dlen	= 1 + (b - buf);
	} else {
	// Shouldn't happen
		dlen = 0;
	}

	s9read(&f->s->c->ctx, tag, buf, dlen);
}

void flushReady(struct Creature *cr) { // Make this part of the library, takes a R9file *rf;
	List * it;
	R9tag	* cur;

	R9file *rf = &(cr->files + CREATURE_READY)->file;

	r9tag_foreach(&cr->gate_tags, it, cur) {
		if (rf == cur->f->file) {
			s9read(&cur->f->s->c->ctx, cur->tag, NULL, 0);

			dill_list_erase(it);
			dill_list_insert(it, &cr->free_tags);

			// Need to do this, this will cause redundant seeks though
			it = &cr->gate_tags;
		}
	}
}

uint16_t die(uint16_t sides) {
	return 1 + (RAND() % sides);
}

uint16_t dice(uint16_t count, uint16_t sides) {
	uint16_t roll = 0;

	while (count) {
		roll += die(sides);
		--count;
	}

	return roll;
}

void rollInitiative() {
	int8_t roll, highroll = 0;
	Creature *cur, *high = NULL;
	List *it;

	flux_list_foreach(&turn, turn, it, cur) {
		roll = die(20) + cur->ab_dex.mod;

		cur->init_roll = roll;

		if (NULL == high || highroll < roll) {
			high = cur;
			highroll	= roll;
		}

		printf("%s rolled init of %d\n", cur->CRFNAME, roll);
	}

	if (NULL != high) {
		T = &high->turn;
		logbuffmt("First turn: %s\n", high->CRFNAME);
	}
}

void nextTurn() {
	T = dill_list_next(T);
	struct Creature *cr;

	if (&turn == T) {
		T = dill_list_next(T);
	}

	if (&turn == T) {
		// we have no-one to play
		return;
	}

	cr = dill_cont(T, struct Creature, turn);

	flushReady(cr);
}

struct Creature *activeTurn() {
	if (&turn == T) {
		nextTurn();

		if (&turn == T) {
			return NULL;
		}
	}

	return dill_cont(T, struct Creature, turn);
}

#define match(a, sz, b, p) (0 == flux_bufeq(a, a + sz, b, &p))

int act_write(R9fid* f, uint32_t size, uint8_t *buf, char ** errstr) {
	struct Creature *cr, *tg;
	struct CreatureFile *cf = dill_cont(f->file, struct CreatureFile, file);
	cr = cf->cr;
	uint16_t rolls[2];
	uint8_t crit = 0, *p, *crname, *tgname;

	if (activeTurn() != cr) {
		*errstr = "Not your turn";
		return -1;
	}

	if (cr == &player) {
		tg = &monster;
	} else {
		tg	 = &player;
	}

	crname	= cr->files->file.st.name;
	tgname	= tg->files->file.st.name;

	if (match(buf, size, "end", p)) {
		logbuffmt("%s ended their turn with no action\n", crname);
	} else if (match(buf, size, "heal", p)) {
		rolls[0] = dice(2, 12);

		cr->hp.current = min(cr->hp.current + rolls[0], cr->hp.max);

		logbuffmt("%s healed %d hit points\n", crname, rolls[0]);
	} else if (match(buf, size, "spell", p)) {
		rolls[0] = dice(2, 6) + 2;

		tg->hp.current = max(-10, tg->hp.current - rolls[0]);

		logbuffmt("%s cast a spell on %s for %d damage\n", crname, tgname, rolls[0]);
	} else if (match(buf, size, "attack", p)) {
		// TODO: BAB
		rolls[0] = die(20);

		if (20 == rolls[0]) {
			crit = 1;
			rolls[0] = die(20);
		}

		if (rolls[0] + cr->ab_str.mod > 10 + tg->ab_dex.mod + tg->ac) {
			// Hit
			crit++;
		}

		if (0 != crit) {
			rolls[1] = die(8) + cr->ab_str.mod;
			rolls[1] *= crit;

			tg->hp.current = max(-10, tg->hp.current - rolls[1]);
		}

		switch (crit) {
		case 0:
			logbuffmt("%s missed eir attack against %s\n", crname, tgname);
			break;
		case 1:
			logbuffmt("%s attacked %s for %d damage\n", crname, tgname, rolls[1]);
			break;
		case 2:
			logbuffmt("%s strongly attacked %s for %d damage\n", crname, tgname, rolls[1]);
			break;
		}
	} else {
		*errstr = "Invalid command";
		return -1;
	}

	nextTurn();

	return 0;
}

void ready_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	struct Creature *cr;
	struct CreatureFile *cf = dill_cont(f->file, struct CreatureFile, file);
	cr = cf->cr;
	R9tag *cur = NULL;
	List *it;

	if (cr == activeTurn()) {
		goto done;
	}

	if (dill_list_empty(&cr->free_tags)) {
		assert(cr->tag_cnt < (1 << 9));

		cur = cr->tags + cr->tag_cnt;
		cr->tag_cnt++;
	} else {
		it = dill_list_next(&cr->free_tags);
		dill_list_erase(it);

		cur = dill_cont(it, struct R9tag, list);
	}

	done:
	if (NULL == cur) {
		s9read(&f->s->c->ctx, tag, NULL, 0);
	} else {
		cur->f	= f;
		cur->size	= size;
		cur->offset	= offset;
		cur->tag	= tag;

		dill_list_insert(&cur->list, &cr->gate_tags);
	}
}

R9fileEv hpEv = {
	.on_shortread	= hp_read
}, readyEv = {
	.on_read	= ready_read
}, actEv = {
	.on_linewrite	= act_write
};

#define MOD(n) ((n) / 2 - 5)
#define ABS(n) ((n) > 0 ? (n) : -(n))

void abScore_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	struct CreatureFile *cf = dill_cont(f->file, struct CreatureFile, file);
	struct Creature * cr = cf->cr;
	uint8_t type = GET_CREATURE_FILE_TYPE(f->file->st.qid);
	uint8_t buf[]	= {LF4}, *be = buf + sizeof(buf), *b;

	switch (type) {
		case CREATURE_AB_STR: b = flux_bufwriteint16(buf, be, cr->ab_str.score); break;
		case CREATURE_AB_STR_MOD: buf[0] = (cr->ab_str.mod > 0) ? '+' : '-'; b = flux_bufwriteint16(buf + 1, be, ABS(cr->ab_str.mod)); break;
		case CREATURE_AB_DEX: b = flux_bufwriteint16(buf, be, cr->ab_dex.score); break;
		case CREATURE_AB_DEX_MOD: buf[0] = (cr->ab_dex.mod > 0) ? '+' : '-'; b = flux_bufwriteint16(buf + 1, be, ABS(cr->ab_dex.mod)); break;
		case CREATURE_AB_CON: b = flux_bufwriteint16(buf, be, cr->ab_con.score); break;
		case CREATURE_AB_CON_MOD: buf[0] = (cr->ab_con.mod > 0) ? '+' : '-'; b = flux_bufwriteint16(buf + 1, be, ABS(cr->ab_con.mod)); break;
		case CREATURE_AB_INT: b = flux_bufwriteint16(buf, be, cr->ab_int.score); break;
		case CREATURE_AB_INT_MOD: buf[0] = (cr->ab_int.mod > 0) ? '+' : '-'; b = flux_bufwriteint16(buf + 1, be, ABS(cr->ab_int.mod)); break;
		case CREATURE_AB_WIS: b = flux_bufwriteint16(buf, be, cr->ab_wis.score); break;
		case CREATURE_AB_WIS_MOD: buf[0] = (cr->ab_wis.mod > 0) ? '+' : '-'; b = flux_bufwriteint16(buf + 1, be, ABS(cr->ab_wis.mod)); break;
		case CREATURE_AB_CHA: b = flux_bufwriteint16(buf, be, cr->ab_cha.score); break;
		case CREATURE_AB_CHA_MOD: buf[0] = (cr->ab_cha.mod > 0) ? '+' : '-'; b = flux_bufwriteint16(buf + 1, be, ABS(cr->ab_cha.mod)); break;
		default:
			b = buf - 1;
	}

	s9read(&f->s->c->ctx, tag, buf, 1 + (b - buf));
}

R9fileEv abilityEv = {
}, abScoreEv = {
	.on_shortread	= abScore_read
};

void setScore(struct AbScore *sc, uint16_t val) {
	sc->score = val;
	sc->mod	= MOD(val);
}

void initCreature(struct Creature *cr, uint16_t id) {
	bufzero(cr, cr + 1);
	int i;
	R9file *f;
	struct CreatureFile *cf;
	Time Now = flux_s();

	for (i = 0; i < MAX_CREATURE_FILES; ++i) {
		cf = cr->files + i;

		cf->file.st.qid.path	= (id << 8) | i;
		cf->cr	= cr;
		cf->file.st.atime	= Now;
		cf->file.st.mtime	= Now;
		cf->file.st.uid	= UN;
		cf->file.st.gid	= UN;
		cf->file.st.muid	= UN;
	}

	// Initialize list objects

	dill_list_init(&cr->free_tags);
	dill_list_init(&cr->gate_tags);

	// TODO do properly:
	// Player HD: 2d8 + 2 (CON)
	cr->hp.current = 14;
	cr->hp.max	= 18;

	cr->ac = 4;

	setScore(&cr->ab_str, 18);
	setScore(&cr->ab_dex, 14);
	setScore(&cr->ab_con, 13);
	setScore(&cr->ab_int, 11);
	setScore(&cr->ab_wis, 9);
	setScore(&cr->ab_cha, 7);

	cf = cr->files;
	f	= &cf->file;

	f->st.name = "player";
	f->st.mode	= C9stdir | 0500;
	f->st.qid.type	= C9qtdir;

	cf++;
	f	= &cf->file;

	f->st.name	= "hp";
	f->st.mode	= 0400;
	f->ev	= &hpEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "hp.max";
	f->st.mode	= 0400;
	f->ev	= &hpEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "hp.current";
	f->st.mode	= 0400;
	f->ev	= &hpEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "abilities";
	f->st.mode	= 0600;
	f->ev	= &abilityEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "str";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "str_mod";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "dex";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "dex_mod";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "con";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "con_mod";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "int";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "int_mod";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "wis";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "wis_mod";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "cha";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "cha_mod";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "ready";
	f->st.mode	= 0400;
	f->ev	= &readyEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "act";
	f->st.mode	= 0200;
	f->ev	= &actEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "friends";
	f->st.mode	= C9stdir | 0500;
	f->st.qid.type	= C9qtdir;
//	f->ev	= &friendsEv; // R9fileEv


	cf++;
	f	= &cf->file;

	f->st.name	= "enemies";
	f->st.mode	= C9stdir | 0500;
	f->st.qid.type	= C9qtdir;
//	f->ev	= &enemiesEv;
}

void ctl_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	// Just EOF for testing
	s9read(&f->s->c->ctx, tag, NULL, 0);
}

int ctl_write(R9fid* f, uint32_t size, uint8_t *buf, char ** errstr) {
	printf("%.*s\n", size, buf);

	return 0;
}

#define ST_DEFAULTS \
	.uid	= UN, \
	.gid	= UN, \
	.muid	= UN

#define _STR(x) #x
#define STR(x) _STR(x)

const char build_ts_str[] = STR(BUILD_TS);

void pid_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	uint8_t buf[256];
	size_t sz;

	sz = sprintf(buf, "%d\n", getpid());

	if (sz > 0) {
		s9read(&f->s->c->ctx, tag, buf, sz);
	} else {
		s9read(&f->s->c->ctx, tag, NULL, 0);
	}
}

void build_ts_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	s9read(&f->s->c->ctx, tag, build_ts_str, sizeof(build_ts_str));
}

void log_read(R9fid *f, C9tag tag, uint64_t offset, uint32_t size) {
	s9read(&f->s->c->ctx, tag, logbuf + offset, flux_min(flux_max(0, f->file->st.size - offset), size));
}

R9fileEv ctlEv = {
	.on_shortread	= ctl_read
	,.on_linewrite	= ctl_write
}, buildEv = {
	.on_shortread	= build_ts_read
}, pidEv = {
	.on_shortread	= pid_read
}, logEv = {
	.on_shortread	= log_read
};

R9file root = {
	.st = {
		.qid	= {.type = C9qtdir, .path = 0xF000000000000000},
		.size	= 0,
		.name	= ".",
		.mode	= C9stdir | 0500,
		ST_DEFAULTS
	}
}, ctl = {
	.st = {
		.qid	= {.path	= 0xF000000000000000},
		.size	= 0,
		.name	= "ctl",
		.mode	= 0600,
		ST_DEFAULTS
	}
	,.ev	= &ctlEv
}, build_ts = {
	.st = {
		.qid	= {.path	= 0xF000000000000001},
		.size	= 0,
		.name	= "build_ts",
		.mode	= 0400,
		ST_DEFAULTS
	}
	,.ev	= &buildEv
}, pid = {
	.st = {
		.qid	= {.path	= 0xF000000000000002},
		.size	= 0,
		.name	= "pid",
		.mode	= 0400,
		ST_DEFAULTS
	}
	,.ev	= &pidEv
}, log_f = {
	.st = {
		.qid	= {.path	= 0xF000000000000003}
		,.size	= 0
		,.name	= "log"
		,.mode	= 0400
		,ST_DEFAULTS
	}
	,.ev	= &logEv
};

#define MAX_CREATURES	(1 << 16)
#define QID_TYPE_MASK	(0xF000000000000000LL)
#define QID_FILE_MASK	(0x00000000000000FFLL)
#define QID_ID_MASK	(0x0000000000FFFF00LL)

// N___ ____ __II IISS

struct Creature *getCreatureFromFile(R9file * rf, uint32_t *idout) {
	struct CreatureFile *cf;
	C9qid	* q = &rf->st.qid;
	uint32_t id = (~QID_ID_MASK & q->path);

	if (C9qtdir == q->type && (MAX_CREATURE_FILES > id)) {
		cf	= dill_cont(rf, struct CreatureFile, file);

		if (NULL != idout) {
			*idout = id;
		}

		return cf->cr;
	}

	return NULL;
}

int r9list_tmp(R9fid *f, C9stat **st) {
	int i = CREATURE_HP;
	struct Creature *cr, *cur;
	List *it;

	if (&root == f->file) {
		st[0] = &ctl.st;
		st[1]	= &build_ts.st;
		st[2]	= &pid.st;
		st[3]	= &log_f.st;

		i = 4;

		flux_list_foreach(&turn, turn, it, cur) {
			st[i++] = &cur->files[CREATURE_ROOT].file.st;
		}
		
		return i;
	}

	// TODO find out what kind of file this is correctly

	cr = getCreatureFromFile(f->file, NULL);

	if (NULL != cr) {
		if (&cr->files->file == f->file) {
			for (; i < MAX_CREATURE_FILES; ++i) {
				st[i - 1]	= &cr->files[i].file.st;
			}
			return MAX_CREATURE_FILES - 1;
		}

		if (&cr->files[CREATURE_FRIENDS].file == f->file) {
			i = 0;

			flux_list_foreach(&turn, turn, it, cur) {
				// TODO: use proper list of all creatures
				if (cr->team == cur->team) {
					st[i] = &cur->files[CREATURE_ROOT].file.st;
				}
			}
			return i;
		}

		if (&cr->files[CREATURE_ENEMIES].file == f->file) {
			i = 0;

			flux_list_foreach(&turn, turn, it, cur) {
				// TODO: use proper list of all creatures
				if (cr->team != cur->team) {
					st[i] = &cur->files[CREATURE_ROOT].file.st;
				}
			}
			return i;
		}
	}

	return -1;
}

#define streq(a, b) (0 == strcmp((a), (b)))

R9file *getCreatureByName(const uint8_t * str) {
	List *it;
	struct Creature *cur;

	flux_list_foreach(&turn, turn, it, cur) {
		// TODO: use proper list of all creatures
		if (streq(str, cur->files[CREATURE_ROOT].file.st.name)) {
			return &cur->files[CREATURE_ROOT].file;
		}
	}

	return NULL;
}

R9file *r9seek_tmp(R9file *rf, R9session *s, const char *str) {
	if (NULL == rf && NULL == str) {
		return &root;
	}

	R9file * crf;

	int i;

	if (&root == rf) {
		crf = getCreatureByName(str);
		if (NULL != crf) {
			return crf;
		} else if (streq(str, ctl.st.name)) {
			return &ctl;
		} else if (streq(str, build_ts.st.name)) {
			return &build_ts;
		} else if (streq(str, pid.st.name)) {
			return &pid;
		} else if (streq(str, log_f.st.name)) {
			return &log_f;
		}
	}

	uint32_t id;

	struct Creature * cr = getCreatureFromFile(rf, &id);

	if (cr) {
		if (CREATURE_ROOT == id) {
			for (i = CREATURE_HP; i < MAX_CREATURE_FILES; ++i) {
				if (streq(str, cr->files[i].file.st.name)) {
					return &cr->files[i].file;
				}
			}
		}

		if (CREATURE_FRIENDS == id || CREATURE_ENEMIES == id) {
			crf = getCreatureByName(str);
			return crf;
		}
	}

	return NULL;
}

uint32_t gen() {
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

	for (i = 0; i < FLUX_ISAAC_RANDSIZ; i++) {
		rng.randrsl[i] = gen();
	}

	isaac32_init(&rng, 1);

	R9srv * srv9 = flux_r9getMainSrv();
	flux_r9srvInit(srv9, r9seek_tmp, r9list_tmp);

	initCreature(&player, 0);
	initCreature(&monster, 1);
	initCreature(&mon2, 2);

	monster.files[0].file.st.name = "monster";
	mon2.CRFNAME	= "mon2";
	// HD 4d8
	monster.hp.current = 18;
	monster.hp.max	= 32;

	monster.ac = 1;

	setScore(&monster.ab_str, 24);
	setScore(&monster.ab_dex, 12);
	setScore(&monster.ab_con, 11);
	setScore(&monster.ab_int, 10);
	setScore(&monster.ab_wis, 9);
	setScore(&monster.ab_cha, 8);

	// HD 2d8
	monster.hp.current = 9;
	monster.hp.max	= 16;

	monster.ac = -1;

	setScore(&monster.ab_str, 24);
	setScore(&monster.ab_dex, 12);
	setScore(&monster.ab_con, 11);
	setScore(&monster.ab_int, 10);
	setScore(&monster.ab_wis, 9);
	setScore(&monster.ab_cha, 8);

	// turn stuff

	player.team = 1;

	dill_list_init(&turn);
	dill_list_insert(&player.turn, &turn);
	dill_list_insert(&monster.turn, &turn);
	dill_list_insert(&mon2.turn, &turn);

	rollInitiative();

	// IP stuff

	ipaddr_local(&addr, NULL, 5555, 0);

	srv	= tcp_listen(&addr, 10);

	assert(-1 != srv);

	forever {
		cli	= tcp_accept(srv, NULL, -1);

		if (-1 == cli) {
			printf("%d\n", errno);
			fflush(stdout);

			msleep(now() + 1500);
			continue;
		}

		fflush(stdout);

		c = allocClient(srv9, cli);

		if_slow (NULL == c) {
			tcp_close(cli, now() + 400);
			continue;
		}

		go(run(c));
	}

	//tcp_close(srv, now() + 400);

	return 0;
}

size_t logbuffmt(const char * fmt, ...) {
	va_list args;
	ssize_t sz;
	uint64_t *psz = &log_f.st.size;
	uint8_t *bs = logbuf + *psz, *be = logbuf + sizeof(logbuf);

	va_start(args, fmt);
	sz = vsnprintf(bs, be - bs, fmt, args);
	va_end(args);

	if (sz > 0) {
		*psz += sz;

		return sz;
	}

	return 0;
}
