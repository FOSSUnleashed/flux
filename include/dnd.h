#pragma once

#include <r9.h>
#include <flux/list.h>

#define MAX_CLASSES 32
#define CRROOTF	files[CREATURE_ROOT].file
#define CRFNAME	CRROOTF.st.name

#define MOD(n) ((n) / 2 - 5)

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

	// 1/2 1/3 1/4 1/6 1/10 250+ 127 
	// negative hd and cr indicate 1/abs(n) eg: -2 == 1/2 ; -4 == 1/4
	int8_t ac, init, init_roll, team, hd, cr;

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

	uint8_t buffer[1 << 9], *b, *be;

	Bit down : 2; // 0 == alive ; 1 == unconcious/stable ; 2 == dying ; 3 == dead
};

enum DamageType {
	TYPE_SLASHING
	,TYPE_BLUDGEONING
	,TYPE_PIERCING
};

typedef enum DamageType DamageType;

typedef struct Creature Creature;
typedef struct AbScore AbScore;
typedef struct CreatureFile CreatureFile;
