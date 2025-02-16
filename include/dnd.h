#pragma once

#include <r9.h>

#define MAX_CLASSES 32
#define CRROOTF	files[CREATURE_ROOT].file
#define CRFNAME	CRROOTF.st.name

enum {
	CREATURE_ROOT
	,CREATURE_HP
	,CREATURE_HP_MAX
	,CREATURE_HP_CURRENT
	,CREATURE_ABILITY
	,CREATURE_AB_STR
	,CREATURE_AB_STR_MOD
	,CREATURE_AB_DEX
	,CREATURE_AB_DEX_MOD
	,CREATURE_AB_CON
	,CREATURE_AB_CON_MOD
	,CREATURE_AB_INT
	,CREATURE_AB_INT_MOD
	,CREATURE_AB_WIS
	,CREATURE_AB_WIS_MOD
	,CREATURE_AB_CHA
	,CREATURE_AB_CHA_MOD
	,CREATURE_READY
	,CREATURE_ACT
	,CREATURE_FRIENDS
	,CREATURE_ENEMIES
	,MAX_CREATURE_FILES
};

struct CreatureFile {
	R9file file;
	struct Creature *cr;
};

struct AbScore {
	uint16_t score;
	int8_t mod;
};

struct Creature {
	struct {
		int16_t current, max;
	} hp;

	struct AbScore ab_str, ab_dex, ab_con, ab_int, ab_wis, ab_cha;

	struct {
		uint8_t ranks;
	} skills;

	uint8_t hd;
	int8_t ac, init, init_roll, team;

	struct {   
		uint8_t level;
		uint8_t type;
	} classes[MAX_CLASSES];

	// TODO Maybe CreatureDir struct?
	struct CreatureFile files[MAX_CREATURE_FILES];

	R9tag tags[1 << 9];

	List free_tags, gate_tags;
	uint16_t tag_cnt;

	List turn;
};

typedef struct Creature Creature;
typedef struct AbScore AbScore;
typedef struct CreatureFile CreatureFile;
