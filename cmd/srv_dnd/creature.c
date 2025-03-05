#include <dnd.h>
#include <flux/str.h>
#include <string.h>

#define match(a, ae, str, p) (0 == flux_bufeq(a, ae, str, &p))
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

int creatureSpawn(uint8_t * name, uint8_t * nameEnd, int team, uint8_t * type, uint8_t * typeEnd) {
	uint8_t *p = NULL;
	Creature *cr = NULL;

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

	cr->b = cr->buffer;
	cr->be = cr->buffer + sizeof(cr->buffer);

	cr->CRFNAME = cr->b;
	cr->b = bufcpy(cr->b, cr->be, name, nameEnd);
	*cr->b++ = 0;

	cr->team = team;

	dill_list_insert(&cr->turn, T);

	if (match(type, typeEnd, "healer", p)) {
		setScore(&cr->ab_wis, 24);
		cr->ac += 4;
	} else if (match(type, typeEnd, "player", p)) {
		cr->ac += 8;
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
		// Trip
		// Weapon Focus (bite)
		// Speed 50
		// Bite +3 melee (1d6+1)
		// CR 1
	} else if (match(type, typeEnd, "direwolf", p)) {
		setScores(cr, 25, 15, 17, 2, 12, 10);

		// HD 6d8+16
		cr->hp.current = 3 * 9 + 16;
		cr->hp.max	= 64;

		cr->ac = 4;
		// LARGE
		// Trip
		// Weapon Focus (bite)
		// Speed 50
		// Bite +11 melee (1d8+10)
		// CR 3
	} else if (match(type, typeEnd, "zombiewolf", p)) {
		setScores(cr, 15, 13, 0, 0, 10, 1);

		// HD 4d12 + 3 (Toughness)
		cr->hp.current = 29;
		cr->hp.max	= 51;

		cr->ac = 5;
		// DR 5/slashing
		// Single action
		// Speed 50
		// Bite +4 melee (1d6+3) (slam same stats)
		// CR 1
	} else if (match(type, typeEnd, "skeletalwolf", p)) {
		setScores(cr, 25, 17, 0, 0, 10, 1);

		// HD 6d12
		cr->hp.current = 39;
		cr->hp.max	= 72;

		cr->ac = 7; // +2 Skeleton +3 Dex -1 Size +3 Direwolf
		// LARGE (based on DireWolf)
		// Improved Init
		// Cold Immunity
		// DR 5/bludgeoning
		// Speed 50
		// Bite +9 melee (1d8+10)
		// CR 2
	} else if (match(type, typeEnd, "snake", p)) {
		setScores(cr, 6, 17, 11, 1, 12, 2); // small viper

		// HD 1d8
		cr->hp.current = 5;
		cr->hp.max	= 8;

		cr->ac = 7; // +1 size +3 dex + 3 nac
		// SMALL
		// Speed 20
		// Improved Init
		// Weapon Finesse (Dex on attack, not damage)
		// Attack +0 BAB +1 Size +3 Dex
		// Bite +4 melee (1d2-2) plus poison (Fort 10, 1d6 Con, 1d6 Con)
		// CR 1/2
	} else if (match(type, typeEnd, "direrat", p)) {
		setScores(cr, 10, 17, 12, 1, 12, 4);

		// HD 1d8+1
		cr->hp.current = 6;
		cr->hp.max	= 9;

		cr->ac = 5; // +1 size +3 dex + 3 nac
		// SMALL
		// Speed 40
		// Weapon Finesse (Dex on attack, not damage)
		// Attack +0 BAB +1 Size +3 Dex
		// Bite +4 melee (1d4) plus disease (Fort 11, 1d3, 1d3 Dex 1d3 Con)
		// CR 1/3
	}

	return 0;
}

static int crSpawn(uint8_t * name, int team, uint8_t * type) {
	return creatureSpawn(name, flux_bufend(name, name + 256), team, type, flux_bufend(type, type + 256));
}

void setupCreatures(List * turn) {
	dill_list_init(turn);

	crSpawn("player", 1, "player");
	crSpawn("healer", 1, "healer");
	crSpawn("mage", 1, "mage");
	crSpawn("thief", 1, "thief");

	// turn stuff

	rollInitiative();
}
