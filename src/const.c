/****************************************************************************
* Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,           *
* Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.      *
*                                                                           *
* Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael             *
* Chastain, Michael Quan, and Mitchell Tse.                                 *
*                                                                           *
* In order to use any part of this Merc Diku Mud, you must comply with      *
* both the original Diku license in 'license.doc' as well the Merc          *
* license in 'license.txt'. In particular, you may not remove either of     *
* these copyright notices.                                                  *
*                                                                           *
* Much time and thought has gone into this software and you are             *
* benefitting. We hope that you share your changes too. What goes           *
* around, comes around.                                                     *
****************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"

/*
* Class table - what the fuck is this even used for?
*/
const	struct	class_type	class_table	[MAX_CLASS]	=
{
    {
        "Mag", APPLY_INT, OBJ_VNUM_SCHOOL_DAGGER,
        3018, 100, 18, 6, 11, 15, TRUE
    }, {
        "Cle", APPLY_WIS, OBJ_VNUM_SCHOOL_MACE,
        3003, 100, 18, 6, 11, 15, TRUE
    }, {
        "Thi", APPLY_DEX, OBJ_VNUM_SCHOOL_DAGGER,
        3028, 100, 18, 6, 11, 15, TRUE
    }, {
        "War", APPLY_STR, OBJ_VNUM_SCHOOL_SWORD,
        3022, 100, 18, 6, 11, 15, TRUE
    }
};

char * const stancenames[11] = {
	"unstanced", "viper", "crane", "falcon", "mongoose",
	"bull", "swallow", "cobra", "lion", "grizzlie",
	"panther"
};

/*
* Titles - what the fuck is this even used for?
*/
char *	const title_table [MAX_CLASS][MAX_LEVEL+1][2] =
{
    {
        { "Sorcerer", "Sorceress" },
        { "Sorcerer", "Sorceress" },
        { "Sorcerer", "Sorceress" },
        { "Sorcerer", "Sorceress" },
        { "Sorcerer", "Sorceress" },
        { "Sorcerer", "Sorceress" },
        { "Sorcerer", "Sorceress" },
        { "Sorcerer", "Sorceress" },
        { "Sorcerer", "Sorceress" },
        { "Sorcerer", "Sorceress" },
    }, {
        { "Priest", "Priestess"	},
        { "Priest", "Priestess"	},
        { "Priest", "Priestess"	},
        { "Priest", "Priestess" },
        { "Priest", "Priestess" },
        { "Priest", "Priestess" },
        { "Priest", "Priestess" },
        { "Priest", "Priestess" },
        { "Priest", "Priestess" },
        { "Priest", "Priestess" },
    }, {
        { "Pilferer", "Pilferess" },
        { "Pilferer", "Pilferess" },
        { "Pilferer", "Pilferess" },
        { "Pilferer", "Pilferess" },
        { "Pilferer", "Pilferess" },
        { "Pilferer", "Pilferess" },
        { "Pilferer", "Pilferess" },
        { "Pilferer", "Pilferess" },
        { "Pilferer", "Pilferess" },
        { "Pilferer", "Pilferess" },
    }, {
        { "Swordsman", "Swordswoman" },
        { "Swordsman", "Swordswoman" },
        { "Swordsman", "Swordswoman" },
        { "Swordsman", "Swordswoman" },
        { "Swordsman", "Swordswoman" },
        { "Swordsman", "Swordswoman" },
        { "Swordsman", "Swordswoman" },
        { "Swordsman", "Swordswoman" },
        { "Swordsman", "Swordswoman" },
        { "Swordsman", "Swordswoman" },
    }
};

/*
 * Liquid properties.
 * Used in world.obj.
 */
const	struct	liq_type	liq_table	[LIQ_MAX]	=
{
    { "water", "clear",	{ 0, 1, 10 }	}, /* 0 */
    { "beer", "amber",	{ 3, 2, 5 }	},
    { "wine", "rose", { 5, 2, 5 }	},
    { "ale", "brown",	{ 2, 2, 5 }	},
    { "dark ale", "dark", { 1, 2, 5 }	},
    { "whisky", "golden",	{ 6, 1, 4 }	}, /* 5 */
    { "lemonade", "pink", { 0, 1, 8 }	},
    { "firebreather", "boiling",	{ 10, 0, 0 }	},
    { "local specialty", "everclear",	{ 3, 3, 3 }	},
    { "slime mold juice", "green",	{ 0, 4, -8 }	},
    { "milk", "white",	{ 0, 3, 6 }	}, /* 10 */
    { "tea", "tan", { 0, 1, 6 }	},
    { "coffee", "black",	{ 0, 1, 6 }	},
    { "blood", "red", { 0, 0, 5 }	},
    { "salt water", "clear",	{ 0, 1, -2 }	},
    { "cola", "cherry",	{ 0, 1, 5 }	} /* 15 */
};

/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n
const	struct	skill_type	skill_table	[MAX_SKILL]	=
{
    {
        "reserved", { 99, 99, 99, 99 },
        0, TAR_IGNORE, POS_STANDING,
        NULL, SLOT( 0),	 0,	 0,
        "", ""
    }, {
        "acid blast", { 1, 1, 1, 1 },
        spell_acid_blast, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(70),	20,	12,
        "acid blast", "!Acid Blast!"
    }, {
        "armor", { 1, 1, 1, 1 },
        spell_armor, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT( 1), 5, 12,
        "", "You feel less protected."
    }, {
        "bless", { 1, 1, 1, 1 },
        spell_bless, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT( 3), 5, 12,
        "", "You feel less righteous."
    }, {
        "blindness", { 1, 1, 1, 1 },
        spell_blindness, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        &gsn_blindness, SLOT( 4), 5, 12,
        "", "You can see again."
    }, {
        "burning hands", { 1, 1, 1, 1 },
        spell_burning_hands, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT( 5),	15,	12,
        "burning hands", "!Burning Hands!"
    }, {
        "call lightning", { 1, 1, 1, 1 },
        spell_call_lightning, TAR_IGNORE, POS_FIGHTING,
        NULL, SLOT( 6),	15,	12,
        "lightning bolt", "!Call Lightning!"
    }, {
        "cause critical", { 1, 1, 1, 1 },
        spell_cause_critical, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(63),	20,	12,
        "spell", "!Cause Critical!"
    }, {
        "cause light", { 1, 1, 1, 1 },
        spell_cause_light, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(62),	15,	12,
        "spell", "!Cause Light!"
    }, {
        "cause serious", { 1, 1, 1, 1 },
        spell_cause_serious, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(64),	17,	12,
        "spell", "!Cause Serious!"
    }, {
        "change sex", { 1, 1, 1, 1 },
        spell_change_sex, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
        NULL, SLOT(82),	15,	12,
        "", "Your body feels familiar again."
    }, {
        "charm person", { 1, 1, 1, 1 },
        spell_charm_person,	TAR_CHAR_OFFENSIVE,	POS_STANDING,
        &gsn_charm_person, SLOT( 7), 5,	12,
        "", "You feel more self-confident."
    }, {
        "chill touch", { 1, 1, 1, 1 },
        spell_chill_touch, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT( 8),	15,	12,
        "chilling touch", "You feel less cold."
    }, {
        "colour spray", { 1, 1, 1, 1 },
        spell_colour_spray,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
        NULL, SLOT(10),	15,	12,
        "colour spray", "!Colour Spray!"
    }, {
        "continual light", { 1, 1, 1, 1 },
        spell_continual_light, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(57), 7, 12,
        "", "!Continual Light!"
    }, {
        "control weather", { 1, 1, 1, 1 },
        spell_control_weather, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(11),	25,	12,
        "", "!Control Weather!"
    }, {
        "create food", { 1, 1, 1, 1 },
        spell_create_food, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(12), 5, 12,
        "", "!Create Food!"
    }, {
        "create spring", { 1, 1, 1, 1 },
        spell_create_spring, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(80),	20,	12,
        "", "!Create Spring!"
    }, {
        "create water", { 1, 1, 1, 1 },
        spell_create_water,	TAR_OBJ_INV, POS_STANDING,
        NULL, SLOT(13), 5, 12,
        "", "!Create Water!"
    }, {
        "cure blindness", { 1, 1, 1, 1 },
        spell_cure_blindness, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
        NULL, SLOT(14), 5,	12,
        "", "!Cure Blindness!"
    }, {
        "cure critical", { 1, 1, 1, 1 },
        spell_cure_critical, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
        NULL, SLOT(15),	20,	12,
        "", "!Cure Critical!"
    }, {
        "cure light", { 1, 1, 1, 1 },
        spell_cure_light, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
        NULL, SLOT(16),	10,	12,
        "", "!Cure Light!"
    }, {
        "cure poison", { 1, 1, 1, 1 },
        spell_cure_poison, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(43), 5, 12,
        "", "!Cure Poison!"
    }, {
        "cure serious", { 1, 1, 1, 1 },
        spell_cure_serious,	TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
        NULL, SLOT(61),	15,	12,
        "", "!Cure Serious!"
    }, {
        "curse", { 1, 1, 1, 1 },
        spell_curse, TAR_CHAR_OFFENSIVE, POS_STANDING,
        &gsn_curse, SLOT(17), 20, 12,
        "curse", "The curse wears off."
    }, {
        "detect evil", { 1, 1, 1, 1 },
        spell_detect_evil, TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT(18), 5,	12,
        "", "The red in your vision disappears."
    }, {
        "detect hidden", { 1, 1, 1, 1 },
        spell_detect_hidden, TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT(44), 5,	12,
        "", "You feel less aware of your suroundings."
    }, {
        "detect invis", { 1, 1, 1, 1 },
        spell_detect_invis,	TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT(19), 5, 12,
        "", "You no longer see invisible objects."
    }, {
        "detect magic", { 1, 1, 1, 1 },
        spell_detect_magic,	TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT(20), 5, 12,
        "", "The detect magic wears off."
    }, {
        "detect poison", { 1, 1, 1, 1 },
        spell_detect_poison, TAR_OBJ_INV, POS_STANDING,
        NULL, SLOT(21), 5, 12,
        "", "!Detect Poison!"
    }, {
        "dispel evil", { 1, 1, 1, 1 },
        spell_dispel_evil, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(22),	15,	12,
        "dispel evil", "!Dispel Evil!"
    }, {
        "dispel magic", { 1, 1, 1, 1 },
        spell_dispel_magic,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
        NULL, SLOT(59),	15,	12,
        "", "!Dispel Magic!"
    }, {
        "earthquake", { 1, 1, 1, 1 },
        spell_earthquake, TAR_IGNORE, POS_FIGHTING,
        NULL, SLOT(23),	15,	12,
        "earthquake", "!Earthquake!"
    }, {
        "enchant weapon", { 1, 1, 1, 1 },
        spell_enchant_weapon, TAR_OBJ_INV, POS_STANDING,
        NULL, SLOT(24),	100, 16,
        "", "!Enchant Weapon!"
    }, {
        "energy drain", { 1, 1, 1, 1 },
        spell_energy_drain,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
        NULL, SLOT(25),	35,	12,
        "energy drain", "!Energy Drain!"
    }, {
        "faerie fire", { 1, 1, 1, 1 },
        spell_faerie_fire, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(72), 5, 8,
        "faerie fire", "The pink aura around you fades away."
    }, {
        "faerie fog", { 1, 1, 1, 1 },
        spell_faerie_fog, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(73),	12,	12,
        "faerie fog", "!Faerie Fog!"
    }, {
        "fireball", { 1, 1, 1, 1 },
        spell_fireball, TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
        NULL, SLOT(26),	15,	16,
        "fireball", "!Fireball!"
    }, {
        "flamestrike", { 1, 1, 1, 1 },
        spell_flamestrike, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(65),	20,	12,
        "flamestrike", "!Flamestrike!"
    }, {
        "fly", { 1, 1, 1, 1 },
        spell_fly, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(56),	10,	18,
        "", "You slowly float to the ground."
    }, {
        "gate", { 4, 4, 4, 4 },
        spell_gate, TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
        NULL, SLOT(83),	500, 12,
        "", "!Gate!"
    }, {
        "giant strength", { 1, 1, 1, 1 },
        spell_giant_strength, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(39),	20,	12,
        "", "You feel weaker."
    }, {
        "harm", { 1, 1, 1, 1 },
        spell_harm, TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
        NULL, SLOT(27),	35,	12,
        "harm spell", "!Harm!"
    }, {
        "heal", { 1, 1, 1, 1 },
        spell_heal, TAR_CHAR_DEFENSIVE,	POS_FIGHTING,
        NULL, SLOT(28),	50,	12,
        "", "!Heal!"
    }, {
        "identify", { 1, 1, 1, 1 },
        spell_identify, TAR_OBJ_INV, POS_STANDING,
        NULL, SLOT(53),	12,	20,
        "", "!Identify!"
    }, {
        "infravision", { 1, 1, 1, 1 },
        spell_infravision, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(77), 5, 18,
        "", "You no longer see in the dark."
    }, {
        "invis", { 1, 1, 1, 1 },
        spell_invis, TAR_CHAR_DEFENSIVE, POS_STANDING,
        &gsn_invis, SLOT(29), 5, 12,
        "", "You are no longer invisible."
    }, {
        "know alignment", { 1, 1, 1, 1 },
        spell_know_alignment, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(58), 9,	12,
        "", "!Know Alignment!"
    }, {
        "lightning bolt", { 1, 1, 1, 1 },
        spell_lightning_bolt, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(30),	15,	12,
        "lightning bolt", "!Lightning Bolt!"
    }, {
        "locate object", { 1, 1, 1, 1 },
        spell_locate_object, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(31),	20,	18,
        "", "!Locate Object!"
    }, {
        "magic missile", { 1, 1, 1, 1 },
        spell_magic_missile, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(32),	15,	8,
        "magic missile", "!Magic Missile!"
    }, {
        "mass invis", { 1, 1, 1, 1 },
        spell_mass_invis, TAR_IGNORE, POS_STANDING,
        &gsn_mass_invis, SLOT(69), 20,	24,
        "", "!Mass Invis!"
    }, {
        "pass door", { 1, 1, 1, 1 },
        spell_pass_door, TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT(74),	20,	12,
        "", "You feel solid again."
    }, {
        "poison", { 1, 1, 1, 1 },
        spell_poison, TAR_CHAR_OFFENSIVE, POS_STANDING,
        &gsn_poison, SLOT(33), 10, 12,
        "poison", "You feel less sick."
    }, {
        "protection", { 1, 1, 1, 1 },
        spell_protection, TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT(34), 5, 12,
        "", "You feel less protected."
    }, {
        "refresh", { 1, 1, 1, 1 },
        spell_refresh, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(81),	12,	18,
        "refresh", "!Refresh!"
    }, {
        "remove curse", { 1, 1, 1, 1 },
        spell_remove_curse,	TAR_IGNORE, POS_STANDING,
        NULL, SLOT(35),	 5,	12,
        "", "!Remove Curse!"
    }, {
        "sanctuary", { 1, 1, 1, 1 },
        spell_sanctuary, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(36),	75,	12,
        "", "The white aura around your body fades."
    }, {
        "shield", { 1, 1, 1, 1 },
        spell_shield, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(67),	12,	18,
        "", "Your force shield shimmers then fades away."
    }, {
        "shocking grasp", { 1, 1, 1, 1 },
        spell_shocking_grasp, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(37),	15,	12,
        "shocking grasp", "!Shocking Grasp!"
    }, {
        "sleep", { 1, 1, 1, 1 },
        spell_sleep, TAR_CHAR_OFFENSIVE, POS_STANDING,
        &gsn_sleep, SLOT(38), 15, 12,
        "", "You feel less tired."
    }, {
        "stone skin", { 1, 1, 1, 1 },
        spell_stone_skin, TAR_CHAR_SELF, POS_STANDING,
        NULL, SLOT(66),	12,	18,
        "", "Your skin feels soft again."
    }, {
        "summon", { 1, 1, 1, 1 },
        spell_summon, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(40),	50,	12,
        "", "!Summon!"
    }, {
		"teleport", { 1, 1, 1, 1 },
		spell_teleport, TAR_IGNORE, POS_FIGHTING,
		NULL, SLOT(2),	35,	12,
		"", "!Teleport!"
	}, {
		"treasurehunter", { 1, 1, 1, 1 },
		spell_treasurehunter, TAR_IGNORE, POS_FIGHTING,
		NULL, SLOT(2),	35,	12,
		"", "!Treasurehunter!"
	}, {
        "ventriloquate", { 1, 1, 1, 1 },
        spell_ventriloquate, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(41), 5, 12,
        "", "!Ventriloquate!"
    }, {
        "weaken", { 1, 1, 1, 1 },
        spell_weaken, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(68),	20,	12,
        "spell", "You feel stronger."
    }, {
        "word of recall", { 99, 99, 99, 99 },
        spell_word_of_recall, TAR_CHAR_SELF, POS_RESTING,
        NULL, SLOT(42), 5, 12,
        "", "!Word of Recall!"
    }, {
        "acid breath", { 1, 1, 1, 1 },
        spell_acid_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
        NULL, SLOT(200), 50, 12,
        "blast of acid", "!Acid Breath!"
    }, {
        "fire breath", { 1, 1, 1, 1 },
        spell_fire_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(201), 50, 12,
        "blast of flame", "!Fire Breath!"
    }, {
        "frost breath", { 1, 1, 1, 1 },
        spell_frost_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
        NULL, SLOT(202), 50, 12,
        "blast of frost", "!Frost Breath!"
    }, {
        "gas breath", { 1, 1, 1, 1 },
        spell_gas_breath, TAR_IGNORE, POS_FIGHTING,
        NULL, SLOT(203), 50, 12,
        "blast of gas", "!Gas Breath!"
    }, {
        "lightning breath",	{ 1, 1, 1, 1 },
        spell_lightning_breath,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
        NULL, SLOT(204), 50, 12,
        "blast of lightning", "!Lightning Breath!"
    }, {
        "backstab", { 1, 1, 1, 1 },
        spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_backstab, SLOT( 0), 0,	24,
        "backstab", "!Backstab!"
    }, {
        "disarm", { 1, 1, 1, 1 },
        spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_disarm, SLOT( 0), 0, 24,
        "", "!Disarm!"
    }, {
        "hide", { 1, 1, 1, 1 },
        spell_null, TAR_IGNORE, POS_RESTING,
        &gsn_hide, SLOT( 0), 0,	12,
        "", "!Hide!"
    }, {
        "hurl", { 1, 1, 1, 1 },
        spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_hurl, SLOT( 0), 0,	24,
        "", "!Hurl!"
    }, {
        "kick", { 1, 1, 1, 1 },
        spell_null, TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
        &gsn_kick, SLOT( 0), 0, 8,
        "kick", "!Kick!"
    }, {
        "peek", { 1, 1, 1, 1 },
        spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_peek, SLOT( 0), 0,	 8,
        "", "!Peek!"
    }, {
        "pick lock", { 1, 1, 1, 1 },
        spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_pick_lock, SLOT( 0), 0, 12,
        "", "!Pick!"
    }, {
        "rescue", { 1, 1, 1, 1 },
        spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_rescue, SLOT( 0), 0, 12,
        "", "!Rescue!"
    }, {
        "sneak", { 1, 1, 1, 1 },
        spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_sneak, SLOT( 0), 0, 12,
        "", NULL
    }, {
        "steal", { 1, 1, 1, 1 },
        spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_steal, SLOT( 0), 0, 24,
        "", "!Steal!"
    }, {
        "general purpose", { 4, 4, 4, 4 },
        spell_general_purpose,	TAR_CHAR_OFFENSIVE,	POS_FIGHTING,
        NULL, SLOT(501), 0,	12,
        "general purpose ammo",	"!General Purpose Ammo!"
    }, {
        "high explosive", { 4, 4, 4, 4 },
        spell_high_explosive, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
        NULL, SLOT(502), 0,	12,
        "high explosive ammo",	"!High Explosive Ammo!"
    }, {
        "guardian", { 1, 1, 1, 1 },
        spell_guardian, TAR_CHAR_DEFENSIVE,	POS_STANDING,
        NULL, SLOT(600), 500, 12,
        "", "!Guardian!"
    }, {
        "soulblade", { 1, 1, 1, 1 },
        spell_soulblade, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(601), 100, 12,
        "", "!Soulblade!"
    }, {
        "mana", { 1, 1, 1, 1 },
        spell_mana, TAR_CHAR_DEFENSIVE,	POS_STANDING,
        NULL, SLOT(602), 0,	12,
        "", "!Mana!"
    }, {
        "frenzy", { 1, 1, 1, 1 },
        spell_frenzy, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(603), 20, 12,
        "", "Your bloodlust subsides."
    }, {
        "darkblessing", { 1, 1, 1, 1 },
        spell_darkblessing,	TAR_CHAR_DEFENSIVE,	POS_STANDING,
        NULL, SLOT(604), 20, 12,
        "", "You feel less wicked."
    }, {
        "portal", { 1, 1, 1, 1 },
        spell_portal, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(604), 50, 12,
        "", "!Portal!"
    }, {
        "energyflux", { 99, 99, 99, 99 },
        spell_energyflux, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(605), 0,	12,
        "", "!EnergyFlux!"
    }, {
        "voodoo", { 1, 1, 1, 1 },
        spell_voodoo, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(606), 100, 12,
        "", "!Voodoo!"
    }, {
        "transport", { 1, 1, 1, 1 },
        spell_transport, TAR_OBJ_INV, POS_STANDING,
        NULL, SLOT(607), 12, 24,
        "", "!Transport!"
    }, {
        "regenerate", { 1, 1, 1, 1 },
        spell_regenerate, TAR_OBJ_INV, POS_STANDING,
        NULL, SLOT(608), 100, 12,
        "", "!Regenerate!"
    }, {
        "clot", { 1, 1, 1, 1 },
        spell_clot, TAR_CHAR_DEFENSIVE,	POS_STANDING,
        NULL, SLOT(609), 50, 12,
        "", "!Clot!"
    }, {
        "mend", { 1, 1, 1, 1 },
        spell_mend, TAR_CHAR_DEFENSIVE,	POS_STANDING,
        NULL, SLOT(610), 50, 12,
        "", "!Mend!"
    }, {
        "punch", { 1, 1, 1, 1 },
        spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_punch, SLOT( 0), 0, 24,
        "punch", "!Punch!"
    }, {
        "quest", { 1, 1, 1, 1 },
        spell_quest, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(611), 500, 12,
        "", "!Quest!"
    }, {
        "minor creation", { 1, 1, 1, 1 },
        spell_minor_creation, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(612), 500, 12,
        "", "!MinorCreation!"
    }, {
        "brew", { 1, 1, 1, 1 },
        spell_brew, TAR_OBJ_INV, POS_STANDING,
        NULL, SLOT(613), 100, 12,
        "", "!Brew!"
    }, {
        "scribe", { 1, 1, 1, 1 },
        spell_scribe, TAR_OBJ_INV, POS_STANDING,
        NULL, SLOT(614), 100, 12,
        "", "!Scribe!"
    }, {
        "carve", { 1, 1, 1, 1 },
        spell_carve, TAR_OBJ_INV, POS_STANDING,
        NULL, SLOT(615), 200, 12,
        "", "!Carve!"
    }, {
        "engrave", { 1, 1, 1, 1 },
        spell_engrave, TAR_OBJ_INV, POS_STANDING,
        NULL, SLOT(616), 300, 12,
        "", "!Engrave!"
    }, {
        "bake", { 1, 1, 1, 1 },
        spell_bake, TAR_OBJ_INV, POS_STANDING,
        NULL, SLOT(617), 100, 12,
        "", "!Bake!"
    }, {
        "mount", { 1, 1, 1, 1 },
        spell_mount, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(618), 500, 12,
        "", "!Mount!"
    }, {
        "berserk", { 1, 1, 1, 1 },
        spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_berserk, SLOT( 0), 0, 8,
        "", "!Berserk!"
    }, {
        "fastdraw", { 1, 1, 1, 1 },
        spell_null, TAR_IGNORE, POS_FIGHTING,
        &gsn_fastdraw, SLOT( 0), 0,	0,
        "", "!Fastdraw!"
    }, {
        "hunt", { 1, 1, 1, 1 },
        spell_null, TAR_IGNORE, POS_STANDING,
        &gsn_hunt, SLOT( 0), 0, 12,
        "", "!Hunt!"
    }, {
        "scan", { 1, 1, 1, 1 },
        spell_scan, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(619), 6,	24,
        "", "!Scan!"
    }, {
        "repair", { 1, 1, 1, 1 },
        spell_repair, TAR_IGNORE, POS_STANDING,
        NULL, SLOT(620), 100, 24,
        "", "!Repair!"
    }, {
        "spellproof", { 1, 1, 1, 1 },
        spell_spellproof, TAR_OBJ_INV, POS_STANDING,
        NULL, SLOT(621), 50, 12,
        "", "!Spellproof!"
    }, {
        "clone", { 1, 1, 1, 1 },
        spell_clone, TAR_CHAR_DEFENSIVE,	POS_STANDING,
        NULL, SLOT(622), 100, 12,
        "", "!Clone!"
    }, {
        "reveal", { 1, 1, 1, 1 },
        spell_reveal, TAR_CHAR_DEFENSIVE, POS_STANDING,
        NULL, SLOT(623), 750, 12,
        "", "!Reveal!"
    }
};

const   struct  clandisc_data clandisc_table    []    =
{
    {"personalarmor", FORTITUDE, 1, do_personal_armor, "You protect yourself with personal armor.\n\r", "You are no longer protected by personal armor.\n\r", "$n is protected by personal armor.\n\r", "$n is no longer protected by personal armor.\n\r", "", "", "", 0, 1, 5, FALSE, TRUE }, //fortitude
    {"resilientminds", FORTITUDE, 2, do_resilient_minds, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //fortitude
    {"armorofkings", FORTITUDE, 3, do_armor_of_kings, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //fortitude
    {"kingmountain", FORTITUDE, 4, do_king_of_the_mountain, "You forsake all attacks in order to focus on defense.\n\r", "You are no longer supernaturally protecting yourself.\n\r", "$n forsakes all attacks to make them unhittable.\n\r", "$n is no longer unhittable... happy hunting!\n\r", "", "", "", 0, 1, 15, FALSE, TRUE }, //fortitude
    {"repairflesh", FORTITUDE, 5, do_repair_undead_flesh, "You heal yourself by repairing your undead flesh.\n\r", "", "$n consumes blood and repairs their undead flesh.\n\r", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //fortitude
    {"armoredflesh", FORTITUDE, 6, do_armored_flesh, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //fortitude
    {"armprometheus", FORTITUDE, 7, do_arm_of_prometheus, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //fortitude
    {"standagainstallfoes", FORTITUDE, 8, do_stand_against_all_foes, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //fortitude
    {"sharedstrength", FORTITUDE, 9, do_shared_strength, "Your shared strength protects you from all foes.\n\r", "Your shared strength no longer protects you.\n\r", "$n's shared strength protects them.\n\r", "$n's shared strength no longer protects them.\n\r", "", "", "", 0, 1, 25, FALSE, TRUE }, //fortitude
    {"eternalvigilance", FORTITUDE, 10, do_eternal_vigilance, "Your eternal vigilance knows no bounds.\n\r", "Eternal Vigilance no longer protects you.\n\r", "$n is protected by their eternal vigilance.\n\r", "$n is no longer protected by their eternal vigilance.\n\r", "", "", "", 0, 1, 35, FALSE, TRUE }, //fortitude

    // AUSPEX ABILITIES 
    {"senses", AUSPEX, 1, do_heightened_senses, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //auspex
    {"perception", AUSPEX, 2, do_aura_perception, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //auspex
    {"prediction", AUSPEX, 3, do_prediction, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //auspex
    {"clairvoyance", AUSPEX, 4, do_clairvoyance, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //auspex
    {"spirittravel", AUSPEX, 5, do_spirit_travel, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //auspex
    {"mindrevealed", AUSPEX, 6, do_the_mind_revealed, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //auspex
    {"animagathering", AUSPEX, 7, do_anima_gathering, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //auspex
    {"ecstaticagony", AUSPEX, 8, do_ecstatic_agony, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //auspex
    {"psychicassault", AUSPEX, 9, do_psychic_assault, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //auspex
    {"masterdominion", AUSPEX, 10, do_master_of_the_dominion, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //auspex

    // ANIMALISM ABILITIES 
    {"pact", ANIMALISM, ANIMALISM_PACT_WITH_ANIMALS, do_pact_with_animals, "", "", "", "", "", "", "", 0, 60, 5, FALSE, TRUE }, //animalism
    {"beckoning", ANIMALISM, ANIMALISM_BECKONING, do_beckoning, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //animalism
    {"quell", ANIMALISM, ANIMALISM_QUELL_THE_BEAST, do_quell_the_beast, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //animalism
    {"subsume", ANIMALISM, ANIMALISM_SUBSUME_THE_SPIRIT, do_subsume_the_spirit, 
        "You contort and growl, as your body changes into the form of a Wolf.", 
        "You scream out as your body reverts back to a normal form.",
        "$n's body contorts and they let out a loud howl.",
        "$n screams in agony as their body reverts back to a human form.",
        "",
        "",
        "",
        0, 1, 5, FALSE, TRUE }, //animalism
    {"drawbeast", ANIMALISM, 5, do_drawing_out_the_beast, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //animalism
    {"taintedoasis", ANIMALISM, 6, do_tainted_oasis, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //animalism
    {"conquerbeast", ANIMALISM, 7, do_conquer_the_beast, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //animalism
    {"tauntcagedbeast", ANIMALISM, 8, do_taunt_the_caged_beast, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //animalism
    {"unchain", ANIMALISM, 9, do_unchain_the_ferocious_beast, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //animalism
    {"beastwithin", ANIMALISM, 10, do_free_the_beast_within, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //animalism

    // CELERITY ABILITIES
    {"quickness", CELERITY, 1, do_quickness, "Your quickness knows no bounds.\n\r", "Turns out, your quickness had bounds...\n\r", "", "", "", "", "", 0, 1, 3, FALSE, TRUE }, //celerity
    {"precision", CELERITY, 2, do_precision, "Your precision at high speeds allows you to see your victim's moves before they happen.\n\r", "Your precision dissipates.\n\r", "", "", "", "2", "", 0, 1, 6, FALSE, FALSE }, //celerity
    {"momentum", CELERITY, 3, do_momentum, "Your speed sharpens, allowing you to strike harder than before.\n\r", "Your speed returns back to normal.\n\r", "", "", "", "2", "", 0, 1, 9, FALSE, FALSE }, //celerity
    {"flawlessparry", CELERITY, 4, do_flawless_parry, "You speed up to unspeakable limits, making you impossible to hit.\n\r", "You slow down your defence, allowing you to focus on attacks once again.\n\r", "$n sacrifices their offense to become impossible to hit.\n\r", "$n can be hit again.\n\r", "", "", "", 0, 1, 12, FALSE, TRUE }, //celerity
    {"stutterstep", CELERITY, 5, do_stutter_step, "You move so quickly that you stutter step, allowing for faster blocking without sacrificing offense.\n\r", "You no longer stutter-step.\n\r", "", "", "", "", "", 0, 1, 15, FALSE, TRUE }, //celerity
    {"flowerdeath", CELERITY, 6, do_flower_of_death, "", "", "", "", "", "", "", 0, 1, 18, FALSE, FALSE }, //celerity
    {"zephyr", CELERITY, 7, do_zephyr, "You move so fast that you rip your body apart.\n\r", "Your speed returns to normal.\n\r", "$n moves so fast that they begin to destroy their own body.\n\r", "$n's speed returns to normal.\n\r", "", "", "", 0, 1, 21, FALSE, FALSE }, //celerity
    {"paragonmotion", CELERITY, 8, do_paragon_of_motion, "", "", "", "", "", "", "", 0, 1, 24, FALSE, FALSE }, //celerity
    {"unseenstorm", CELERITY, 9, do_the_unseen_storm, "", "", "", "", "", "", "", 0, 1, 27, FALSE, FALSE }, //celerity
    {"betweenticks", CELERITY, 10, do_between_the_ticks, "", "", "", "", "", "", "", 0, 1, 30, FALSE, FALSE }, //celerity

    // OBTENEBRATION ABILITIES
    {"shadowplay", OBTENEBRATION, 1, do_shadow_play, "Your shadow play makes you stronger.\n\r", "You stop using shadow play.\n\r", "", "", "", "", "", 0, 1, 5, FALSE, TRUE }, //obtenebration
    {"shroudnight", OBTENEBRATION, 2, do_shroud_of_night, "You blanket the room in darkness.", "", "$n blankets the room in darkness.\n\r", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obtenebration
    {"armsabyss", OBTENEBRATION, 3, do_arms_of_the_abyss, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obtenebration
    {"blackmetamorphosis", OBTENEBRATION, 4, do_black_metamorphosis, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obtenebration
    {"shadowstep", OBTENEBRATION, 5, do_shadowstep, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obtenebration
    {"darknesswithin", OBTENEBRATION, 6, do_the_darkness_within, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obtenebration
    {"innerdarkness", OBTENEBRATION, 7, do_inner_darkness, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obtenebration
    {"tchernabog", OBTENEBRATION, 8, do_tchernabog, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obtenebration
    {"ahrimansdemesne", OBTENEBRATION, 9, do_ahrimans_demesne, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obtenebration
    {"banishment", OBTENEBRATION, 10, do_banishment, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obtenebration

    // POTENCE ABILITIES
    {"crush", POTENCE, 1, do_crush, "Your strength allows you to crush your foes.\n\r", "Your strength returns to normal.\n\r", "$n's grows strong enough to crush their enemies.\n\r", "$n's strength returns to normal.\n\r", "", "", "", 0, 1, 5, FALSE, TRUE }, //potence
    {"fistoflillith", POTENCE, 2, do_the_fist_of_lillith, "You channel the power of Lillith.\n\r", "Lillith's power fades from you.\n\r", "$n channels the power of Lillith.\n\r", "$n is no longer empowered by Lillith.\n\r", "", "", "", 3, 1, 25, FALSE, FALSE }, //potence
    {"earthshock", POTENCE, 3, do_earthshock, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //potence
    {"aftershock", POTENCE, 4, do_aftershock, "", "", "", "", "", "", "", 0, 1, 5, FALSE, TRUE }, //potence
    {"forgers", POTENCE, 5, do_the_forgers_hammer, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //potence
    {"titansfist", POTENCE, 6, do_fist_of_the_titans, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //potence
    {"brutality", POTENCE, 7, do_brutality, "", "", "", "", "", "", "", 0, 1, 5, FALSE, TRUE }, //potence
    {"lendvigor", POTENCE, 8, do_lend_the_supernatural_vigor, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //potence
    {"might", POTENCE, 9, do_might_of_the_heroes, "", "", "", "", "", "", "", 0, 1, 5, FALSE, TRUE }, //potence
    {"touch", POTENCE, 10, do_touch_of_pain, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //potence

    // PRESENCE ABILITIES
    {"awe", PRESENCE, 1, do_awe, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //presence
    {"dreadgaze", PRESENCE, 2, do_dread_gaze, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //presence
    {"majesty", PRESENCE, 3, do_majesty, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //presence
    {"paralyzingglance", PRESENCE, 4, do_paralyzing_glance, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //presence
    {"psummon", PRESENCE, 5, do_presence_summon, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //presence
    {"bloodlust", PRESENCE, 6, do_bloodlust, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //presence
    {"phobia", PRESENCE, 7, do_phobia, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //presence
    {"standtall", PRESENCE, 8, do_stand_tall, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //presence
    {"dreamworld", PRESENCE, 9, do_dream_world, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //presence
    {"puremajesty", PRESENCE, 10, do_pure_majesty, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //presence

    // QUIETUS ABILITIES
    {"scorpionstouch", QUIETUS, 1, do_scorpions_touch, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //quietus
    {"dagonscall", QUIETUS, 2, do_dagons_call, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //quietus
    {"baalscaress", QUIETUS, 3, do_baals_caress, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //quietus
    {"tastedeath", QUIETUS, 4, do_taste_of_death, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //quietus
    {"erosion", QUIETUS, 5, do_erosion, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //quietus
    {"selectivesilence", QUIETUS, 6, do_selective_silence, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //quietus
    {"bloodsweat",QUIETUS,  7, do_blood_sweat, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //quietus
    {"thinblood", QUIETUS, 8, do_thin_blood, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //quietus
    {"blooddestruction", QUIETUS, 9, do_blood_of_destruction, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //quietus
    {"weakentheblood", QUIETUS, 10, do_weaken_the_blood_of_the_ancients, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //quietus

    // THAUMATURGY ABILITIES
    {"geomancy", THAUMATURGY, 1, do_geomancy, "Your skin sparks with magical energy.", "The magic fades as your skin stops sparking.", "$n's skin sparks with magical energy.", "$n's skin stops sparking.", "", "", "", 0, 1, 5, FALSE, TRUE }, //thaum
    {"spark", THAUMATURGY, 2, do_spark, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //thaum
    {"vertigo", THAUMATURGY, 3, do_vertigo, "", "", "", "", "", "", "", 0, 1, 35, FALSE, FALSE }, //thaum
    {"contortion", THAUMATURGY, 4, do_contortion, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //thaum
    {"bloodboil", THAUMATURGY, 5, do_blood_boil, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //thaum
    {"runesofpower", THAUMATURGY, 6, do_runes_of_power, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //thaum
    {"routthecharginghordes", THAUMATURGY, 7, do_rout_the_charging_hordes, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //thaum
    {"cognizantconstruction", THAUMATURGY, 8, do_cognizant_construction, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //thaum
    {"eyeofthestorm", THAUMATURGY, 9, do_eye_of_the_storm, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //thaum
    {"regoignem", THAUMATURGY, 10, do_rego_ignem, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //thaum

    // VICISSITUDE ABILITIES 
    {"visage", VICISSITUDE, 1, do_malleable_visage, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //vicissitude
    {"fleshcraft", VICISSITUDE, 2, do_fleshcraft, "", "", "", "", "", "", "", 0, 1, 5, FALSE, TRUE }, //vicissitude
    {"bonecraft", VICISSITUDE, 3, do_bone_craft, "", "", "", "", "", "", "", 0, 1, 5, FALSE, TRUE }, //vicissitude
    {"fleshrot", VICISSITUDE, 4, do_flesh_rot, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //vicissitude
    {"dragonbreath", VICISSITUDE, 5, do_breath_of_the_dragon, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //vicissitude
    {"bodyarsenal", VICISSITUDE, 6, do_body_arsenal, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //vicissitude
    {"impalersfence", VICISSITUDE, 7, do_impalers_fence, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //vicissitude
    {"willoverform", VICISSITUDE, 8, do_will_over_form, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //vicissitude
    {"skintrap", VICISSITUDE, 9, do_skin_trap, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //vicissitude
    {"thelastdracul", VICISSITUDE, 10, do_the_last_dracul, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obfuscate

    // OBFUSCATE ABILITIES
    {"cloakofshadows", OBFUSCATE, 1, do_cloak_of_shadows, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obfuscate
    {"maskfaces", OBFUSCATE, 2, do_mask_of_a_thousand_faces, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obfuscate
    {"fade", OBFUSCATE, 3, do_fade_from_the_minds_eye, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obfuscate
    {"silenceofdeath", OBFUSCATE, 4, do_the_silence_of_death, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obfuscate
    {"cloakthegathering", OBFUSCATE, 5, do_cloak_the_gathering, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obfuscate
    {"manifoldguise", OBFUSCATE, 6, do_manifold_guise, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obfuscate
    {"fortressofsilence", OBFUSCATE, 7, do_fortress_of_silence, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obfuscate
    {"mentalmaze", OBFUSCATE, 8, do_mental_maze, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obfuscate
    {"obscurecreation", OBFUSCATE, 9, do_obscure_gods_creation, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obfuscate
    {"veil", OBFUSCATE, 10, do_veil_of_blissful_ignorance, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE }, //obfuscate

    {"command", DOMINATE, 1, do_command, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE },
    {"mesmerize", DOMINATE, 2, do_mesmerize, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE },
    {"posession", DOMINATE, 3, do_posession, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE },
    {"obedience", DOMINATE, 4, do_obedience, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE },
    {"tranquility", DOMINATE, 5, do_tranquility, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE },
    {"massmanipulation", DOMINATE, 6, do_massmanipulation, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE },
    {"chainpsyche", DOMINATE, 7, do_chainpsyche, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE },
    {"mindattack", DOMINATE, 8, do_mindattack, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE },
    {"bestintentions", DOMINATE, 9, do_bestintentions, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE },
    {"massmesmerize", DOMINATE, 10, do_massmesmerize, "", "", "", "", "", "", "", 0, 1, 5, FALSE, FALSE },

    
    {""}
};
