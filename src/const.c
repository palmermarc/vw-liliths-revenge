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

const   struct  clandisc_type clandisc_table    [WHAT_GOES_HERE]    =
{
    {"personal_armor", 1, do_personal_armor, HAS_FORTITUDE, 1, TRUE }, //fortitude
    {"resilient_minds", 2, do_resilient_minds, HAS_FORTITUDE, 1, TRUE }, //fortitude
    {"armor_of_kings", 3, do_armor_of_kings, HAS_FORTITUDE, 1, TRUE }, //fortitude
    {"king_of_the_mountain", 4, do_king_of_the_mountain, HAS_FORTITUDE, 1, TRUE }, //fortitude
    {"repair_undead_flesh", 5, do_repair_undead_flesh, HAS_FORTITUDE, 1, TRUE }, //fortitude
    {"armored_flesh", 6, do_armored_flesh, HAS_FORTITUDE, 1, TRUE }, //fortitude
    {"arm_of_prometheus", 7, do_arm_of_prometheus, HAS_FORTITUDE, 1, TRUE }, //fortitude
    {"stand_against_all_foes", 8, do_stand_against_all_foes, HAS_FORTITUDE, 1, TRUE }, //fortitude
    {"shared_strength", 9, do_shared_strength, HAS_FORTITUDE, 1, TRUE }, //fortitude
    {"eternal_vigilance", 10, do_eternal_vigilance, HAS_FORTITUDE, 1, TRUE }, //fortitude
    {"heightened_senses", 1, do_heightened_senses, HAS_FORTITUDE, 1, TRUE }, //auspex
    {"aura_perception", 2, do_aura_perception, HAS_FORTITUDE, 1, TRUE }, //auspex
    {"prediction", 3, do_prediction, HAS_FORTITUDE, 1, TRUE }, //auspex
    {"clairvoyance", 4, do_clairvoyance, HAS_FORTITUDE, 1, TRUE }, //auspex
    {"spirit_travel", 5, do_spirit_travel, HAS_FORTITUDE, 1, TRUE }, //auspex
    {"the_mind_revealed", 6, do_the_mind_revealed, HAS_FORTITUDE, 1, TRUE }, //auspex
    {"anima_gathering", 7, do_anima_gathering, HAS_FORTITUDE, 1, TRUE }, //auspex
    {"ecstatic_agony", 8, do_ecstatic_agony, HAS_FORTITUDE, 1, TRUE }, //auspex
    {"psychic_assault", 9, do_psychic_assault, HAS_FORTITUDE, 1, TRUE }, //auspex
    {"master_of_the_dominion", 10, do_master_of_the_dominion, HAS_FORTITUDE, 1, TRUE }, //auspex
    {"pact_with_animals", 1, do_pact_with_animals, HAS_FORTITUDE, 1, TRUE }, //animalism
    {"beckoning", 2, do_beckoning, HAS_FORTITUDE, 1, TRUE }, //animalism
    {"quell_the_beast", 3, do_quell_the_beast, HAS_FORTITUDE, 1, TRUE }, //animalism
    {"subsume_the_spirit", 4, do_subsume_the_spirit, HAS_FORTITUDE, 1, TRUE }, //animalism
    {"drawing_out_the_beast", 5, do_drawing_out_the_beast, HAS_FORTITUDE, 1, TRUE }, //animalism
    {"tainted_oasis", 6, do_tainted_oasis, HAS_FORTITUDE, 1, TRUE }, //animalism
    {"conquer_the_beast", 7, do_conquer_the_beast, HAS_FORTITUDE, 1, TRUE }, //animalism
    {"taunt_the_caged_beast", 8, do_taunt_the_caged_beast, HAS_FORTITUDE, 1, TRUE }, //animalism
    {"unchain_the_ferocious_beast", 9, do_unchain_the_ferocious_beast, HAS_FORTITUDE, 1, TRUE }, //animalism
    {"free_the_beast_within", 10, do_free_the_beast_within, HAS_FORTITUDE, 1, TRUE }, //animalism
    {"quickness", 1, do_quickness, HAS_FORTITUDE, 1, TRUE }, //celerity
    {"precision", 2, do_precision, HAS_FORTITUDE, 1, TRUE }, //celerity
    {"momentum", 3, do_momentum, HAS_FORTITUDE, 1, TRUE }, //celerity
    {"flawless_parry", 4, do_flawless_parry, HAS_FORTITUDE, 1, TRUE }, //celerity
    {"stutter_step", 5, do_stutter_step, HAS_FORTITUDE, 1, TRUE }, //celerity
    {"flower_of_death", 6, do_flower_of_death, HAS_FORTITUDE, 1, TRUE }, //celerity
    {"zephyr", 7, do_zephyr, HAS_FORTITUDE, 1, TRUE }, //celerity
    {"paragon_of_motion", 8, do_paragon_of_motion, HAS_FORTITUDE, 1, TRUE }, //celerity
    {"the_unseen_storm", 9, do_the_unseen_storm, HAS_FORTITUDE, 1, TRUE }, //celerity
    {"between_the_ticks", 10, do_between_the_ticks, HAS_FORTITUDE, 1, TRUE }, //celerity
    {"shadow_play", 1, do_shadow_play, HAS_FORTITUDE, 1, TRUE }, //obtenebration
    {"shroud_of_night", 2, do_shroud_of_night, HAS_FORTITUDE, 1, TRUE }, //obtenebration
    {"arms_of_the_abyss", 3, do_arms_of_the_abyss, HAS_FORTITUDE, 1, TRUE }, //obtenebration
    {"black_metamorphosis", 4, do_black_metamorphosis, HAS_FORTITUDE, 1, TRUE }, //obtenebration
    {"shadowstep", 5, do_shadowstep, HAS_FORTITUDE, 1, TRUE }, //obtenebration
    {"the_darkness_within", 6, do_the_darkness_within, HAS_FORTITUDE, 1, TRUE }, //obtenebration
    {"inner_darkness", 7, do_inner_darkness, HAS_FORTITUDE, 1, TRUE }, //obtenebration
    {"tchernabog", 8, do_tchernabog, HAS_FORTITUDE, 1, TRUE }, //obtenebration
    {"ahrimans_demesne", 9, do_ahrimans_demesne, HAS_FORTITUDE, 1, TRUE }, //obtenebration
    {"banishment", 10, do_banishment, HAS_FORTITUDE, 1, TRUE }, //obtenebration
    {"crush", 1, do_crush, HAS_FORTITUDE, 1, TRUE }, //potence
    {"the_fist_of_lillith", 2, do_the_fist_of_lillith, HAS_FORTITUDE, 1, TRUE }, //potence
    {"earthshock", 3, do_earthshock, HAS_FORTITUDE, 1, TRUE }, //potence
    {"aftershock", 4, do_aftershock, HAS_FORTITUDE, 1, TRUE }, //potence
    {"the_forgers_hammer", 5, do_the_forgers_hammer, HAS_FORTITUDE, 1, TRUE }, //potence
    {"fist_of_the_titans", 6, do_fist_of_the_titans, HAS_FORTITUDE, 1, TRUE }, //potence
    {"brutality", 7, do_brutality, HAS_FORTITUDE, 1, TRUE }, //potence
    {"lend_the_supernatural_vigor", 8, do_lend_the_supernatural_vigor, HAS_FORTITUDE, 1, TRUE }, //potence
    {"might_of_the_heroes", 9, do_might_of_the_heroes, HAS_FORTITUDE, 1, TRUE }, //potence
    {"touch_of_pain", 10, do_touch_of_pain, HAS_FORTITUDE, 1, TRUE }, //potence
    {"awe", 1, do_awe, HAS_FORTITUDE, 1, TRUE }, //presence
    {"dread_gaze", 2, do_dread_gaze, HAS_FORTITUDE, 1, TRUE }, //presence
    {"majesty", 3, do_majesty, HAS_FORTITUDE, 1, TRUE }, //presence
    {"paralyzing_glance", 4, do_paralyzing_glance, HAS_FORTITUDE, 1, TRUE }, //presence
    {"summon", 5, do_presence_summon, HAS_FORTITUDE, 1, TRUE }, //presence
    {"bloodlust", 6, do_bloodlust, HAS_FORTITUDE, 1, TRUE }, //presence
    {"phobia", 7, do_phobia, HAS_FORTITUDE, 1, TRUE }, //presence
    {"stand_tall", 8, do_stand_tall, HAS_FORTITUDE, 1, TRUE }, //presence
    {"dream_world", 9, do_dream_world, HAS_FORTITUDE, 1, TRUE }, //presence
    {"pure_majesty", 10, do_pure_majesty, HAS_FORTITUDE, 1, TRUE }, //presence
    {"scorpions_touch", 1, do_scorpions_touch, HAS_FORTITUDE, 1, TRUE }, //quietus
    {"dagons_call", 2, do_dagons_call, HAS_FORTITUDE, 1, TRUE }, //quietus
    {"baals_caress", 3, do_baals_caress, HAS_FORTITUDE, 1, TRUE }, //quietus
    {"taste_of_death", 4, do_taste_of_death, HAS_FORTITUDE, 1, TRUE }, //quietus
    {"erosion", 5, do_erosion, HAS_FORTITUDE, 1, TRUE }, //quietus
    {"selective_silence", 6, do_selective_silence, HAS_FORTITUDE, 1, TRUE }, //quietus
    {"blood_sweat", 7, do_blood_sweat, HAS_FORTITUDE, 1, TRUE }, //quietus
    {"thin_blood", 8, do_thin_blood, HAS_FORTITUDE, 1, TRUE }, //quietus
    {"blood_of_destruction", 9, do_blood_of_destruction, HAS_FORTITUDE, 1, TRUE }, //quietus
    {"weaken_the_blood_of_the_ancients", 10, do_weaken_the_blood_of_the_ancients, HAS_FORTITUDE, 1, TRUE }, //quietus
    {"geomancy", 1, do_geomancy, HAS_FORTITUDE, 1, TRUE }, //thaum
    {"spark", 2, do_spark, HAS_FORTITUDE, 1, TRUE }, //thaum
    {"vertigo", 3, do_vertigo, HAS_FORTITUDE, 1, TRUE }, //thaum
    {"contortion", 4, do_contortion, HAS_FORTITUDE, 1, TRUE }, //thaum
    {"blood_boil", 5, do_blood_boil, HAS_FORTITUDE, 1, TRUE }, //thaum
    {"runes_of_power", 6, do_runes_of_power, HAS_FORTITUDE, 1, TRUE }, //thaum
    {"rout_the_charging_hordes", 7, do_rout_the_charging_hordes, HAS_FORTITUDE, 1, TRUE }, //thaum
    {"cognizant_construction", 8, do_cognizant_construction, HAS_FORTITUDE, 1, TRUE }, //thaum
    {"eye_of_the_storm", 9, do_eye_of_the_storm, HAS_FORTITUDE, 1, TRUE }, //thaum
    {"rego_ignem", 10, do_rego_ignem, HAS_FORTITUDE, 1, TRUE }, //thaum
    {"malleable_visage", 1, do_malleable_visage, HAS_FORTITUDE, 1, TRUE }, //vicissitude
    {"fleshcraft", 2, do_fleshcraft, HAS_FORTITUDE, 1, TRUE }, //vicissitude
    {"bone_craft", 3, do_bone_craft, HAS_FORTITUDE, 1, TRUE }, //vicissitude
    {"flesh_rot", 4, do_flesh_rot, HAS_FORTITUDE, 1, TRUE }, //vicissitude
    {"breath_of_the_dragon", 5, do_breath_of_the_dragon, HAS_FORTITUDE, 1, TRUE }, //vicissitude
    {"body_arsenal", 6, do_body_arsenal, HAS_FORTITUDE, 1, TRUE }, //vicissitude
    {"impalers_fence", 7, do_impalers_fence, HAS_FORTITUDE, 1, TRUE }, //vicissitude
    {"will_over_form", 8, do_will_over_form, HAS_FORTITUDE, 1, TRUE }, //vicissitude
    {"skin_trap", 9, do_skin_trap, HAS_FORTITUDE, 1, TRUE }, //vicissitude
    {"the_last_dracul", 10, do_the_last_dracul, HAS_FORTITUDE, 1, TRUE }, //vicissitude
    {"cloak_of_shadows", 1, do_cloak_of_shadows, HAS_FORTITUDE, 1, TRUE }, //obfuscate
    {"mask_of_a_thousand_faces", 2, do_mask_of_a_thousand_faces, HAS_FORTITUDE, 1, TRUE }, //obfuscate
    {"fade_from_the_minds_eye", 3, do_fade_from_the_minds_eye, HAS_FORTITUDE, 1, TRUE }, //obfuscate
    {"the_silence_of_death", 4, do_the_silence_of_death, HAS_FORTITUDE, 1, TRUE }, //obfuscate
    {"cloak_the_gathering", 5, do_cloak_the_gathering, HAS_FORTITUDE, 1, TRUE }, //obfuscate
    {"manifold_guise", 6, do_manifold_guise, HAS_FORTITUDE, 1, TRUE }, //obfuscate
    {"fortress_of_silence", 7, do_fortress_of_silence, HAS_FORTITUDE, 1, TRUE }, //obfuscate
    {"mental_maze", 8, do_mental_maze, HAS_FORTITUDE, 1, TRUE }, //obfuscate
    {"obscure_gods_creation", 9, do_obscure_gods_creation, HAS_FORTITUDE, 1, TRUE }, //obfuscate
    {"veil_of_blissful_ignorance", 10, do_veil_of_blissful_ignorance, HAS_FORTITUDE, 1, TRUE }, //obfuscate
}
