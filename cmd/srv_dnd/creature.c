#include <dnd.h>
#include <flux/str.h>
#include <string.h>

#define match(a, ae, str, p) (0 == flux_bufeq(a, BUFLIT(str), &p))
#define setScores(cr, str, dex, con, int, wis, cha) \
		setScore(&(cr)->ab_str, str); \
		setScore(&(cr)->ab_dex, dex); \
		setScore(&(cr)->ab_con, con); \
		setScore(&(cr)->ab_int, int); \
		setScore(&(cr)->ab_wis, wis); \
		setScore(&(cr)->ab_cha, cha);

Creature creatures[128], *player = creatures;
static int creatureCount = 0;
extern List *T;

void setScore(struct AbScore *sc, uint16_t val) {
	sc->score = val;
	sc->mod	= val ? MOD(val) : 0;

	// TODO: score == 0 for non-existant scores is a blegh hack
}

void rollInitiative();
void initCreature(struct Creature *cr, uint16_t id);

int creatureSpawn(uint8_t * _name, uint8_t * nameEnd, int team, uint8_t * _type, uint8_t * typeEnd) {
	uint8_t *p = NULL;
	Creature *cr = NULL;
	Buffer name = {_name, nameEnd}, type = {_type, typeEnd};

	// TODO remove when no longer hard-coding
	if (match(name, nameEnd, "player", p)) {
		cr = player;
	} else if (match(name, nameEnd, "healer", p)) {
		cr = player + 1;
	} else if (match(name, nameEnd, "mage", p)) {
		cr = player + 2;
	} else if (match(name, nameEnd, "thief", p)) {
		cr = player + 3;
	} else if (creatureCount < 130) {
		cr = creatures + creatureCount;
	}

	if (NULL == cr) {
		return -1;
	}

	initCreature(cr, creatureCount++);

	Buffer crb = BUFLIT(cr->buffer), x;

	cr->b = crb.start;
	cr->be = crb.end;

	// WTF am I doing here?
	cr->CRFNAME = cr->b;
	x = bufcpy(crb, name);
	cr->b	= x.end;
	*cr->b++ = 0;

	cr->team = team;

	dill_list_insert(&cr->turn, T);

	if (match(type, typeEnd, "healer", p)) {
		setScore(&cr->ab_wis, 24);
		cr->ac += 4;
	} else if (match(type, typeEnd, "thief", p)) {
		setScore(&cr->ab_dex, 24);
		cr->ac += 8;

		cr->weapon = 0;
		cr->weapons[0].dice_count	= 3;
		cr->weapons[0].dice_size	= 4;
		cr->weapons[0].crit_range	= 16;
		cr->weapons[0].crit_mult	= 4;
		cr->weapons[0].specialization	= 3;
	} else if (match(type, typeEnd, "player", p)) {
		cr->ac += 8;

		cr->weapon	= 0;

		cr->weapons[0].dice_count	= 3;
		cr->weapons[0].dice_size	= 6;
		cr->weapons[0].crit_range	= 20;
		cr->weapons[0].crit_mult	= 2;
		cr->weapons[0].focus	= 3;
	} else if (match(type, typeEnd, "monster", p)) {
		// HD 4d8
		cr->hp.current = 18;
		cr->hp.max	= 32;

		cr->ac = 1;

		setScores(cr, 24, 12, 11, 10, 9, 8);
	} else if (match(type, typeEnd, "mon2", p)) {
		// HD 2d8
		cr->hp.current = 9;
		cr->hp.max= 16;

		cr->ac = -1;

		setScores(cr, 24, 12, 11, 10, 9, 8);
	} else if (match(type, typeEnd, "wolf", p)) {
		setScores(cr, 13, 15, 15, 2, 12, 6);

		// HD 2d8+4
		cr->hp.current = 13;
		cr->hp.max	= 20;

		cr->ac = 4;

		cr->bab = 1;

		cr->weapon = 0;

		// Bite +3 melee (1d6+1)
		cr->weapons[0].focus = 1;
		cr->weapons[0].dice_count	= 1;
		cr->weapons[0].dice_size	= 6;
		cr->weapons[0].crit_range	= 20;
		cr->weapons[0].crit_mult	= 2;
		cr->weapons[0].primary	= 1;

		// Trip
		// Speed 50

		cr->cr = 1;
	} else if (match(type, typeEnd, "direwolf", p)) {
		setScores(cr, 25, 15, 17, 2, 12, 10);

		// HD 6d8+16
		cr->hp.current = 3 * 9 + 16;
		cr->hp.max	= 64;

		cr->ac = 4;
		cr->bab	= 4;

		cr->weapon = 0;

		// Bite +11 melee (1d8+10)
		cr->weapons[0].focus = 1;
		cr->weapons[0].dice_count	= 1;
		cr->weapons[0].dice_size	= 8;
		cr->weapons[0].crit_range	= 20;
		cr->weapons[0].crit_mult	= 2;
		cr->weapons[0].primary	= 1;

		// LARGE
		// Trip
		// Speed 50
		cr->cr	= 3;
	} else if (match(type, typeEnd, "zombiewolf", p)) {
		setScores(cr, 15, 13, 0, 0, 10, 1);

		// HD 4d12 + 3 (Toughness)
		cr->hp.current = 29;
		cr->hp.max	= 51;

		cr->ac = 5;
		cr->bab	= 2;

		cr->weapon	= 1;

		// Bite +4 (1d6+3)
		cr->weapons[0].dice_count	= 1;
		cr->weapons[0].dice_size	= 6;
		cr->weapons[0].crit_range	= 20;
		cr->weapons[0].crit_mult	= 2;
		cr->weapons[0].primary	= 1;

		// Slam +4 (1d6+3)
		cr->weapons[1].dice_count	= 1;
		cr->weapons[1].dice_size	= 6;
		cr->weapons[1].crit_range	= 20;
		cr->weapons[1].crit_mult	= 2;
		cr->weapons[1].primary	= 1;

		// DR 5/slashing
		// Single action
		// Speed 50
		cr->cr = 1;
	} else if (match(type, typeEnd, "skeletalwolf", p)) {
		setScores(cr, 25, 17, 0, 0, 10, 1);

		// HD 6d12
		cr->hp.current = 39;
		cr->hp.max	= 72;

		cr->ac	= 7; // +2 Skeleton +3 Dex -1 Size +3 Direwolf
		cr->bab	= 3;

		cr->weapon	= 0;

		// Bite +9 (1d8+10)
		cr->weapons[0].dice_count	= 1;
		cr->weapons[0].dice_size	= 8;
		cr->weapons[0].crit_range	= 20;
		cr->weapons[0].crit_mult	= 2;
		cr->weapons[0].primary	= 1;

		// LARGE (based on DireWolf)
		// Improved Init
		// Cold Immunity
		// DR 5/bludgeoning
		// Speed 50
		cr->cr = 2;
	} else if (match(type, typeEnd, "snake", p)) {
		setScores(cr, 6, 17, 11, 1, 12, 2); // small viper

		// HD 1d8
		cr->hp.current = 5;
		cr->hp.max	= 8;

		cr->ac = 7; // +1 size +3 dex + 3 nac
		cr->bab	= 0;

		cr->weapon	= 0;

		// Bite +4 (1d2-2)
		// Bite +4 melee (1d2-2) plus poison (Fort 10, 1d6 Con, 1d6 Con)
		cr->weapons[0].dice_count	= 1;
		cr->weapons[0].dice_size	= 2;
		cr->weapons[0].crit_range	= 20;
		cr->weapons[0].crit_mult	= 2;
		cr->weapons[0].finesse	= 1; // Weapon Finesse (Dex on attack, not damage)

		// SMALL
		// Speed 20
		// Improved Init
		// Attack +0 BAB +1 Size +3 Dex
		cr->cr = -2;
	} else if (match(type, typeEnd, "direrat", p)) {
		setScores(cr, 10, 17, 12, 1, 12, 4);

		// HD 1d8+1
		cr->hp.current = 6;
		cr->hp.max	= 9;

		cr->ac = 5; // +1 size +3 dex + 3 nac
		cr->bab	= 0;

		cr->weapon	= 0;

		// Bite +4 melee (1d4) plus disease (Fort 11, 1d3, 1d3 Dex 1d3 Con)
		cr->weapons[0].dice_count	= 1;
		cr->weapons[0].dice_size	= 4;
		cr->weapons[0].crit_range	= 20;
		cr->weapons[0].crit_mult	= 2;
		cr->weapons[0].finesse	= 1; // Weapon Finesse (Dex on attack, not damage)

		// SMALL
		// Speed 40
		// Attack +0 BAB +1 Size +3 Dex
		cr->cr	= -3;
	}

	logbuffmt("Spawned a %.*s named %.*s for team %d\n", (type.end - type.start), type.start, (name.end - name.start), name.start, team);

	return 0;
}

void setupCreatures(List * turn) {
	dill_list_init(turn);

	Buffer x;

	x = BUFLIT("player");
	creatureSpawn(x.start, x.end, 1, x.start, x.end);
	x = BUFLIT("healer");
	creatureSpawn(x.start, x.end, 1, x.start, x.end);
	x = BUFLIT("mage");
	creatureSpawn(x.start, x.end, 1, x.start, x.end);
	x = BUFLIT("thief");
	creatureSpawn(x.start, x.end, 1, x.start, x.end);
}
