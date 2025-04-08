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
#include <stdbool.h>

#include <stdio.h>

#define BSZ (1 << 15)

char UN[] = "R", buf[BSZ];
uint8_t logbuf[BSZ];

isaac32_ctx rng;
List turn, *T = &turn;
bool Victory = false;

#define RAND() rand32(&rng)

struct Creature *activeTurn();

const uint8_t *bufstrlen(const uint8_t *buf) {
	for (; *buf; ++buf);

	return buf;
}

/*
* **hp**: Int[2]:State Effective current and maximum HP, space delim'd
* **hp.current**: Int:State current real HP
* **hp.max**: Int:State maximum real HP
* **hp.tmp**: Int:State current temporary HP
* **ability**: Int[4]:Multi-state/RPC: Ability table, in order: Base, Modifier,
    Temporary, Effective Modifier.  Writes are accepted to apply level-up
    Ability Score bonuses only.
* **$ability**: Int:State Current effective ability score.
* **$ability.mod**: Int:State Current effective ability modifier.
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

#define GET_CREATURE_FILE_TYPE(qid) ((qid).path & 0xFF)
#define streq(a, b) (0 == strcmp((a), (b)))

#define MAX_CREATURES	(1 << 16)
#define QID_TYPE_MASK	(0xF000000000000000LL)
#define QID_FILE_MASK	(0x00000000000000FFLL)
#define QID_ID_MASK	(0x0000000000FFFF00LL)

// N___ ____ __II IISS

Creature *getCreatureByName(const uint8_t * _str, const uint8_t * strend) {
	List *it;
	struct Creature *cur;
	Buffer str = {(uint8_t *)_str, (uint8_t *)strend}, name;


	flux_list_foreach(&turn, turn, it, cur) {
		// TODO: use proper list of all creatures
		name.start = cur->files[CREATURE_ROOT].file.st.name;
		name.end	= name.start + 512;
		name.end	= bufend(name);

		if (0 == flux_bufcmp(str, name, NULL)) {
			return cur;
		}
	}

	return NULL;
}

R9file *getCreatureRootFileByName(const uint8_t * str) {
	Creature * cr = getCreatureByName(str, bufstrlen(str) - 1);

	return (NULL == cr) ? NULL : &cr->files[CREATURE_ROOT].file;
}

Creature *getCreatureFromFile(R9file * rf, uint32_t *idout) {
	CreatureFile *cf;
	C9qid	* q = &rf->st.qid;
	uint64_t id = (~QID_ID_MASK & q->path);

	if (C9qtdir == q->type && (MAX_CREATURE_FILES > id)) {
		cf	= dill_cont(rf, struct CreatureFile, file);

		if (NULL != idout) {
			*idout = id;
		}

		return cf->cr;
	}

	return NULL;
}

void takeDamage(Creature *cr, DamageType dt, uint16_t amt) {
	(void)dt;
	cr->hp.current = max(cr->hp.current - amt, -10);

	if (0 >= cr->hp.current && 0 == cr->down) {
		cr->down = 2;
		logbuffmt("%s has fallen down!\n", cr->CRFNAME);
	}
}

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

	if (NULL == cr) { // true when Victory, occasionally
		return;
	}

	R9file *rf = &(cr->files + CREATURE_READY)->file;

	r9tag_foreach(&cr->gate_tags, it, cur) {
		if (rf == cur->f->file) {
			if (Victory) {
				s9error(&cur->f->s->c->ctx, cur->tag, "Victory");
			} else {
				s9read(&cur->f->s->c->ctx, cur->tag, NULL, 0);
			}

			dill_list_erase(it);
			dill_list_insert(it, &cr->free_tags);

			// Need to do this, this will cause redundant seeks though
			it = &cr->gate_tags;
		}
	}

	if (!Victory) {
		logbuffmt("It is now %s's turn\n", cr->CRFNAME);
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
		logbuffmt("New game started!\n\tFirst turn: %s\n", high->CRFNAME);
	}

	flushReady(activeTurn());
}

bool creatureAbleToAct(Creature *cr) {
	return cr->hp.current > 0;
}

void nextTurn() {
	struct Creature *cr = dill_cont(T, Creature, turn);

	// Note: we might accidentally disconnect T from &turn, make sure we don't

	do {
		T = dill_list_next(T);

		if (&turn == T) {
			T = dill_list_next(T);
		}

		if (creatureAbleToAct(dill_cont(T, Creature, turn))) {
			break;
		}
	} while (&cr->turn != T);

	if (&turn == T || Victory) {
		// we have no-one to play
		return;
	}

	cr = dill_cont(T, struct Creature, turn);

	flushReady(cr);
}

struct Creature *activeTurn() {
	if (Victory) {
		return NULL;
	}

	if (&turn == T) {
		nextTurn();

		if (&turn == T) {
			return NULL;
		}
	}

	return dill_cont(T, struct Creature, turn);
}

#define match(a, sz, b, p) (0 == flux_bufeq(a, BUFLIT(b), &p))

Weapon nulloWeapon = {
	.dice_size	= 4
	,.dice_count	= 1
	,.crit_mult	= 2
	,.crit_range	= 20
};

int act_write(R9fid* f, uint32_t size, uint8_t *data, char ** errstr) {
	struct Creature *cr, *tg = NULL;
	struct CreatureFile *cf = dill_cont(f->file, struct CreatureFile, file);
	cr = cf->cr;
	uint16_t rolls[2];
	uint8_t crit = 0, *p, *be = data + size;
	Buffer buf = {data, data + size};

	if (activeTurn() != cr) {
		*errstr = "Not your turn";
		return -1;
	}

	if (match(buf, size, "end", p)) {
		logbuffmt("%s ended their turn with no action\n", cr->CRFNAME);
	} else if (match(buf, size, "heal", p)) {
		rolls[0] = dice(2, 12);

		if (p != be) {
			for (++p; p <= be && ' ' == *p; ++p);

			tg = getCreatureByName(p, be);
		}

		if (NULL == tg) {
			goto no_target;
		}

		logbuffmt("%s healed %s %d hit points %d %02x(%c)\n", cr->CRFNAME, tg->CRFNAME, rolls[0], be - p, *be, *be);

		tg->hp.current = min(tg->hp.current + rolls[0], tg->hp.max);
	} else if (match(buf, size, "spell", p)) {
		rolls[0] = dice(2, 6) + 2;

		if (p != be) {
			for (++p; p <= be && ' ' == *p; ++p);

			tg = getCreatureByName(p, be);
		}

		if (NULL == tg) {
			goto no_target;
		}

		logbuffmt("%s cast a spell on %s for %d damage\n", cr->CRFNAME, tg->CRFNAME, rolls[0]);

		takeDamage(tg, 0, rolls[0]);
	} else if (match(buf, size, "attack", p)) {
		rolls[0] = die(20);
		Weapon *wp = &nulloWeapon;

		if (cr->weapon < MAX_WEAPONS && cr->weapons[cr->weapon].dice_count) {
			wp = cr->weapons + cr->weapon;
		}

		if (p != be) {
			for (++p; p <= be && ' ' == *p; ++p);

			tg = getCreatureByName(p, be);
		}

		if (NULL == tg) {
			goto no_target;
		}

		if (wp->crit_range <= rolls[0]) {
			crit = 1;
			rolls[0] = die(20);
		}

		// TODO: size mod
		// size mod | bab | focus | str/dex
		rolls[0] += wp->focus + (wp->finesse ? cr->ab_dex.mod : cr->ab_str.mod) + cr->bab;

		if (rolls[0] > 10 + tg->ab_dex.mod + tg->ac) {
			// Hit
			crit++;
		}

		if (0 != crit) {
			rolls[1] = die(8) + (wp->primary ? cr->ab_str.mod * 3 / 2 : cr->ab_str.mod) + wp->specialization * 2;
			if (crit > 1) {
				rolls[1] *= (wp->crit_mult > 1 && wp->crit_mult < 5) ? wp->crit_mult : 2;
			}
		}

		switch (crit) {
		case 0:
			logbuffmt("%s missed eir attack against %s\n", cr->CRFNAME, tg->CRFNAME);
			break;
		case 1:
			logbuffmt("%s attacked %s for %d damage\n", cr->CRFNAME, tg->CRFNAME, rolls[1]);
			break;
		case 2:
			logbuffmt("%s strongly attacked %s for %d damage\n", cr->CRFNAME, tg->CRFNAME, rolls[1]);
			break;
		}

		if (0 != crit) {
			takeDamage(tg, 0, rolls[1]);
		}
	} else {
		*errstr = "Invalid command";
		return -1;
	}

	if (!Victory) {
		List *it;
		int32_t victoriousTeam = -1;

		flux_list_foreach(&turn, turn, it, cr) {
			if (cr->down) {
				continue;
			}

			if (-1 == victoriousTeam) {
				victoriousTeam = cr->team;
			} else if (cr->team != victoriousTeam) {
				victoriousTeam = -1;
				break;
			}
		}

		if (-1 != victoriousTeam) {
			logbuffmt("Victory to team %d\n", victoriousTeam);
			Victory = true;
		}
	}

	nextTurn();

	return 0;
	no_target:
	*errstr = "No valid target";
	return -1;
}

void ready_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	struct Creature *cr;
	struct CreatureFile *cf = dill_cont(f->file, struct CreatureFile, file);
	cr = cf->cr;
	R9tag *cur = NULL;
	List *it;

	if (Victory) {
		s9error(&f->s->c->ctx, tag, "Victory");
		return;
	}

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

// Char Creation:
// Select race
// Ability points assignment
// Alignment

// General
// Select weapon
// Select spell
// Use item?

// Level up:
// Select feat
// Select ability level up choice
// Select skill points
// Select class to level up
// Class ability selection
// Select spells to learn

static int crctl_write(R9fid* f, uint32_t size, uint8_t *buf, char ** errstr) {
	

	return 0;
}

static void crctl_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	// What choices need to be made for the character

	s9read(&f->s->c->ctx, tag, NULL, 0);
}

R9fileEv abilityEv = {
}, abScoreEv = {
	.on_shortread	= abScore_read
}, crctlEv = {
	.on_shortread	= crctl_read
	,.on_linewrite	= crctl_write
};

void setScore(struct AbScore *sc, uint16_t val);
void setupCreatures(List * turn);

void initCreature(struct Creature *cr, uint16_t id) {
	bufzeromem(cr, cr + 1);
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

	f->st.name	= "str.mod";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "dex";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "dex.mod";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "con";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "con.mod";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "int";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "int.mod";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "wis";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "wis.mod";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "cha";
	f->st.mode	= 0400;
	f->ev	= &abScoreEv;

	cf++;
	f	= &cf->file;

	f->st.name	= "cha.mod";
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

	cf++;
	f	= &cf->file;

	f->st.name	= "ctl";
	f->st.mode	= 0600;
	f->ev	= &crctlEv;
}

void ctl_read(R9fid* f, C9tag tag, uint64_t offset, uint32_t size) {
	// Just EOF for testing
	s9read(&f->s->c->ctx, tag, NULL, 0);
}

int creatureSpawn(uint8_t * name, uint8_t * nameEnd, int team, uint8_t * type, uint8_t * typeEnd);

int16_t flux_bufreadint16(uint8_t *buf, uint8_t *be, uint8_t **p) {
	int sign = 1;
	uint8_t *b = buf;
	int16_t num = 0;

	if (be == b) goto read_error;

	if ('-' == *b) {
		sign = -1;
		++b;
	}

	if (be == b) goto read_error;

	flux_bufadvance(b, be, ('0' <= *b && '9' >= *b)) {
		num = num * 10 + (*b - '0');
	}

	if (p) {
		*p = b;
	}

	return sign * num;
	read_error:
	if (p) {
		*p = NULL;
	}
	return 0;
}

int ctl_write(R9fid* f, uint32_t size, uint8_t *data, char ** errstr) {
	uint8_t *p, *be = data + size;
	List *it;
	Creature *cr;
	Buffer buf = {data, data + size};

	if (match(buf, size, "start", p)) {
		rollInitiative();
	} else if (match(buf, size, "reset", p)) {
		flux_list_foreach(T, turn, it, cr) {
			cr->hp.current = cr->hp.max;
			cr->down = 0;
		}
		Victory = false;
		rollInitiative();
	} else if (match(buf, size, "spawn", p)) { // spawn <name> <team> <type>
		if (p == be) goto bad_spawn;

		logbuffmt("Attempt Spawn\n");

		uint8_t *name, *nameEnd, *team, *teamEnd;
		++p;

		flux_bufadvance(p, be, (' ' == *p)); // skip space
		if (p == be) goto bad_spawn;

		name = p;

		flux_bufadvance(p, be, (' ' != *p));
		if (p == be) goto bad_spawn;

		nameEnd = p;

		flux_bufadvance(p, be, (' ' == *p)); // skip space
		if (p == be) goto bad_spawn;

		team = p;

		flux_bufadvance(p, be, (' ' != *p));
		if (p == be) goto bad_spawn;

		teamEnd = p;

		flux_bufadvance(p, be, (' ' == *p)); // skip space
		if (p == be) goto bad_spawn;

		logbuffmt("spawn test\n");
		creatureSpawn(name, nameEnd, flux_bufreadint16(team, teamEnd, NULL), p, be);

		return 0;
		bad_spawn:
		*errstr = "Invalid spawn command: spawn <name> <team> <type>";
		return -1;
	} else {
		printf("%.*s\n", size, buf);
	}

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
	.on_read	= log_read
};

R9file root = {
	.st = {
		.qid	= {.type = C9qtdir, .path = 0xF000000000000000LL},
		.size	= 0,
		.name	= ".",
		.mode	= C9stdir | 0500,
		ST_DEFAULTS
	}
}, ctl = {
	.st = {
		.qid	= {.path	= 0xF000000000000000LL},
		.size	= 0,
		.name	= "ctl",
		.mode	= 0600,
		ST_DEFAULTS
	}
	,.ev	= &ctlEv
}, build_ts = {
	.st = {
		.qid	= {.path	= 0xF000000000000001LL},
		.size	= 0,
		.name	= "build_ts",
		.mode	= 0400,
		ST_DEFAULTS
	}
	,.ev	= &buildEv
}, pid = {
	.st = {
		.qid	= {.path	= 0xF000000000000002LL},
		.size	= 0,
		.name	= "pid",
		.mode	= 0400,
		ST_DEFAULTS
	}
	,.ev	= &pidEv
}, log_f = {
	.st = {
		.qid	= {.path	= 0xF000000000000003LL}
		,.size	= 0
		,.name	= "log"
		,.mode	= 0400
		,ST_DEFAULTS
	}
	,.ev	= &logEv
};

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
				if (cr->team == cur->team && cr != cur) {
					st[i] = &cur->files[CREATURE_ROOT].file.st;
					++i;
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
					++i;
				}
			}
			return i;
		}
	}

	return -1;
}

R9file *r9seek_tmp(R9file *rf, R9session *s, const char *str) {
	if (NULL == rf && NULL == str) {
		return &root;
	}

	R9file * crf;

	int i;

	if (&root == rf) {
		crf = getCreatureRootFileByName(str);
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
			crf = getCreatureRootFileByName(str);
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

int main(int argc, char **argv) {
	handle srv, cli;
	struct ipaddr addr;
	R9client *c;
	int i;

	// RNG SEED

	for (i = 0; i < FLUX_ISAAC_RANDSIZ; i++) {
		rng.randrsl[i] = gen();
	}

	isaac32_init(&rng, 1);

	// Creature

	setupCreatures(&turn);

	// SRV

	R9srv * srv9 = flux_r9getMainSrv();
	flux_r9srvInit(srv9, r9seek_tmp, r9list_tmp);

	// IP stuff

	ipaddr_local(&addr, NULL, argc > 1 ? 1129 : 5555, 0);

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

		c = allocClient(srv9, cli, tcp_close);

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

		log_f.st.mtime	= flux_s();

		return sz;
	}

	return 0;
}
