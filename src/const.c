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

const   struct  clandisc_data clandisc_table    [MAX_CLANDISC]    =
{
    {"personal_armor", "fortitude", 1, do_personal_armor, "", "", "", "", 1, 5, FALSE }, //fortitude
    {"resilient_minds", "fortitude", 2, do_resilient_minds, "", "", "", "", 1, 5, FALSE }, //fortitude
    {"armor_of_kings", "fortitude", 3, do_armor_of_kings, "", "", "", "", 1, 5, FALSE }, //fortitude
    {"king_of_the_mountain", "fortitude", 4, do_king_of_the_mountain, "", "", "", "", 1, 5, FALSE }, //fortitude
    {"repair_undead_flesh", "fortitude", 5, do_repair_undead_flesh, "", "", "", "", 1, 5, FALSE }, //fortitude
    {"armored_flesh", "fortitude", 6, do_armored_flesh, "", "", "", "", 1, 5, FALSE }, //fortitude
    {"arm_of_prometheus", "fortitude", 7, do_arm_of_prometheus, "", "", "", "", 1, 5, FALSE }, //fortitude
    {"stand_against_all_foes", "fortitude", 8, do_stand_against_all_foes, "", "", "", "", 1, 5, FALSE }, //fortitude
    {"shared_strength", "fortitude", 9, do_shared_strength, "", "", "", "", 1, 5, FALSE }, //fortitude
    {"eternal_vigilance", "fortitude", 10, do_eternal_vigilance, "", "", "", "", 1, 5, FALSE }, //fortitude

    /* AUSPEX ABILITIES */
    {"heightened_senses", "auspex", 1, do_heightened_senses, "", "", "", "", 1, 5, FALSE }, //auspex
    {"aura_perception", "auspex", 2, do_aura_perception, "", "", "", "", 1, 5, FALSE }, //auspex
    {"prediction", "auspex", 3, do_prediction, "", "", "", "", 1, 5, FALSE }, //auspex
    {"clairvoyance", "auspex", 4, do_clairvoyance, "", "", "", "", 1, 5, FALSE }, //auspex
    {"spirit_travel", "auspex", 5, do_spirit_travel, "", "", "", "", 1, 5, FALSE }, //auspex
    {"the_mind_revealed", "auspex", 6, do_the_mind_revealed, "", "", "", "", 1, 5, FALSE }, //auspex
    {"anima_gathering", "auspex", 7, do_anima_gathering, "", "", "", "", 1, 5, FALSE }, //auspex
    {"ecstatic_agony", "auspex", 8, do_ecstatic_agony, "", "", "", "", 1, 5, FALSE }, //auspex
    {"psychic_assault", "auspex", 9, do_psychic_assault, "", "", "", "", 1, 5, FALSE }, //auspex
    {"master_of_the_dominion", "auspex", 10, do_master_of_the_dominion, "", "", "", "", 1, 5, FALSE }, //auspex

    /* ANIMALISM ABILITIES */
    {"pact_with_animals", "animalism", 1, do_pact_with_animals, "", "", "", "", 1, 5, FALSE }, //animalism
    {"beckoning", "animalism", 2, do_beckoning, "", "", "", "", 1, 5, FALSE }, //animalism
    {"quell_the_beast", "animalism", 3, do_quell_the_beast, "", "", "", "", 1, 5, FALSE }, //animalism
    {"subsume_the_spirit", "animalism", 4, do_subsume_the_spirit, "", "", "", "", 1, 5, FALSE }, //animalism
    {"drawing_out_the_beast", "animalism", 5, do_drawing_out_the_beast, "", "", "", "", 1, 5, FALSE }, //animalism
    {"tainted_oasis", "animalism", 6, do_tainted_oasis, "", "", "", "", 1, 5, FALSE }, //animalism
    {"conquer_the_beast", "animalism", 7, do_conquer_the_beast, "", "", "", "", 1, 5, FALSE }, //animalism
    {"taunt_the_caged_beast", "animalism", 8, do_taunt_the_caged_beast, "", "", "", "", 1, 5, FALSE }, //animalism
    {"unchain_the_ferocious_beast", "animalism", 9, do_unchain_the_ferocious_beast, "", "", "", "", 1, 5, FALSE }, //animalism
    {"free_the_beast_within", "animalism", 10, do_free_the_beast_within, "", "", "", "", 1, 5, FALSE }, //animalism

    /* CELERITY ABILITIES */
    {"quickness", "celerity", 1, do_quickness, "", "", "", "", 1, 5, FALSE }, //celerity
    {"precision", "celerity", 2, do_precision, "", "", "", "", 1, 5, FALSE }, //celerity
    {"momentum", "celerity", 3, do_momentum, "", "", "", "", 1, 5, FALSE }, //celerity
    {"flawless_parry", "celerity", 4, do_flawless_parry, "", "", "", "", 1, 5, FALSE }, //celerity
    {"stutter_step", "celerity", 5, do_stutter_step, "", "", "", "", 1, 5, FALSE }, //celerity
    {"flower_of_death", "celerity", 6, do_flower_of_death, "", "", "", "", 1, 5, FALSE }, //celerity
    {"zephyr", "celerity", 7, do_zephyr, "", "", "", "", 1, 5, FALSE }, //celerity
    {"paragon_of_motion", "celerity", 8, do_paragon_of_motion, "", "", "", "", 1, 5, FALSE }, //celerity
    {"the_unseen_storm", "celerity", 9, do_the_unseen_storm, "", "", "", "", 1, 5, FALSE }, //celerity
    {"between_the_ticks", "celerity", 10, do_between_the_ticks, "", "", "", "", 1, 5, FALSE }, //celerity

    /* OBTENEBRATION ABILITIES */
    {"shadow_play", "obtenebration", 1, do_shadow_play, "", "", "", "", 1, 5, FALSE }, //obtenebration
    {"shroud_of_night", "obtenebration", 2, do_shroud_of_night, "", "", "", "", 1, 5, FALSE }, //obtenebration
    {"arms_of_the_abyss", "obtenebration", 3, do_arms_of_the_abyss, "", "", "", "", 1, 5, FALSE }, //obtenebration
    {"black_metamorphosis", "obtenebration", 4, do_black_metamorphosis, "", "", "", "", 1, 5, FALSE }, //obtenebration
    {"shadowstep", "obtenebration", 5, do_shadowstep, "", "", "", "", 1, 5, FALSE }, //obtenebration
    {"the_darkness_within", "obtenebration", 6, do_the_darkness_within, "", "", "", "", 1, 5, FALSE }, //obtenebration
    {"inner_darkness", "obtenebration", 7, do_inner_darkness, "", "", "", "", 1, 5, FALSE }, //obtenebration
    {"tchernabog", "obtenebration", 8, do_tchernabog, "", "", "", "", 1, 5, FALSE }, //obtenebration
    {"ahrimans_demesne", "obtenebration", 9, do_ahrimans_demesne, "", "", "", "", 1, 5, FALSE }, //obtenebration
    {"banishment", "obtenebration", 10, do_banishment, "", "", "", "", 1, 5, FALSE }, //obtenebration

    /* POTENCE ABILITIES */
    {"crush", "potence", 1, do_crush, "", "", "", "", 1, 5, FALSE }, //potence
    {"the_fist_of_lillith", "potence", 2, do_the_fist_of_lillith, "", "", "", "", 1, 5, FALSE }, //potence
    {"earthshock", "potence", 3, do_earthshock, "", "", "", "", 1, 5, FALSE }, //potence
    {"aftershock", "potence", 4, do_aftershock, "", "", "", "", 1, 5, FALSE }, //potence
    {"the_forgers_hammer", "potence", 5, do_the_forgers_hammer, "", "", "", "", 1, 5, FALSE }, //potence
    {"fist_of_the_titans", "potence", 6, do_fist_of_the_titans, "", "", "", "", 1, 5, FALSE }, //potence
    {"brutality", "potence", 7, do_brutality, "", "", "", "", 1, 5, FALSE }, //potence
    {"lend_the_supernatural_vigor", "potence", 8, do_lend_the_supernatural_vigor, "", "", "", "", 1, 5, FALSE }, //potence
    {"might_of_the_heroes", "potence", 9, do_might_of_the_heroes, "", "", "", "", 1, 5, FALSE }, //potence
    {"touch_of_pain", "potence", 10, do_touch_of_pain, "", "", "", "", 1, 5, FALSE }, //potence

    /* PRESENCE ABILITIES */
    {"awe", "presence", 1, do_awe, "", "", "", "", 1, 5, FALSE }, //presence
    {"dread_gaze", "presence", 2, do_dread_gaze, "", "", "", "", 1, 5, FALSE }, //presence
    {"majesty", "presence", 3, do_majesty, "", "", "", "", 1, 5, FALSE }, //presence
    {"paralyzing_glance", "presence", 4, do_paralyzing_glance, "", "", "", "", 1, 5, FALSE }, //presence
    {"summon", "presence", 5, do_presence_summon, "", "", "", "", 1, 5, FALSE }, //presence
    {"bloodlust", "presence", 6, do_bloodlust, "", "", "", "", 1, 5, FALSE }, //presence
    {"phobia", "presence", 7, do_phobia, "", "", "", "", 1, 5, FALSE }, //presence
    {"stand_tall", "presence", 8, do_stand_tall, "", "", "", "", 1, 5, FALSE }, //presence
    {"dream_world", "presence", 9, do_dream_world, "", "", "", "", 1, 5, FALSE }, //presence
    {"pure_majesty", "presence", 10, do_pure_majesty, "", "", "", "", 1, 5, FALSE }, //presence

    /* QUIETUS ABILITIES */
    {"scorpions_touch", "quietus", 1, do_scorpions_touch, "", "", "", "", 1, 5, FALSE }, //quietus
    {"dagons_call", "quietus", 2, do_dagons_call, "", "", "", "", 1, 5, FALSE }, //quietus
    {"baals_caress", "quietus", 3, do_baals_caress, "", "", "", "", 1, 5, FALSE }, //quietus
    {"taste_of_death", "quietus", 4, do_taste_of_death, "", "", "", "", 1, 5, FALSE }, //quietus
    {"erosion", "quietus", 5, do_erosion, "", "", "", "", 1, 5, FALSE }, //quietus
    {"selective_silence", "quietus", 6, do_selective_silence, "", "", "", "", 1, 5, FALSE }, //quietus
    {"blood_sweat","quietus",  7, do_blood_sweat, "", "", "", "", 1, 5, FALSE }, //quietus
    {"thin_blood", "quietus", 8, do_thin_blood, "", "", "", "", 1, 5, FALSE }, //quietus
    {"blood_of_destruction", "quietus", 9, do_blood_of_destruction, "", "", "", "", 1, 5, FALSE }, //quietus
    {"weaken_the_blood_of_the_ancients", "quietus", 10, do_weaken_the_blood_of_the_ancients, "", "", "", "", 1, 5, FALSE }, //quietus

    /* THAUMATURGY ABILITIES */
    {"geomancy", "thaumaturgy", 1, do_geomancy, "", "", "", "", 1, 5, FALSE }, //thaum
    {"spark", "thaumaturgy", 2, do_spark, "", "", "", "", 1, 5, FALSE }, //thaum
    {"vertigo", "thaumaturgy", 3, do_vertigo, "", "", "", "", 1, 5, FALSE }, //thaum
    {"contortion", "thaumaturgy", 4, do_contortion, "", "", "", "", 1, 5, FALSE }, //thaum
    {"blood_boil", "thaumaturgy", 5, do_blood_boil, "", "", "", "", 1, 5, FALSE }, //thaum
    {"runes_of_power", "thaumaturgy", 6, do_runes_of_power, "", "", "", "", 1, 5, FALSE }, //thaum
    {"rout_the_charging_hordes", "thaumaturgy", 7, do_rout_the_charging_hordes, "", "", "", "", 1, 5, FALSE }, //thaum
    {"cognizant_construction", "thaumaturgy", 8, do_cognizant_construction, "", "", "", "", 1, 5, FALSE }, //thaum
    {"eye_of_the_storm", "thaumaturgy", 9, do_eye_of_the_storm, "", "", "", "", 1, 5, FALSE }, //thaum
    {"rego_ignem", "thaumaturgy", 10, do_rego_ignem, "", "", "", "", 1, 5, FALSE }, //thaum

    /* VICISSITUDE ABILITIES */
    {"malleable_visage", "vicissitude", 1, do_malleable_visage, "", "", "", "", 1, 5, FALSE }, //vicissitude
    {"fleshcraft", "vicissitude", 2, do_fleshcraft, "", "", "", "", 1, 5, FALSE }, //vicissitude
    {"bone_craft", "vicissitude", 3, do_bone_craft, "", "", "", "", 1, 5, FALSE }, //vicissitude
    {"flesh_rot", "vicissitude", 4, do_flesh_rot, "", "", "", "", 1, 5, FALSE }, //vicissitude
    {"breath_of_the_dragon", "vicissitude", 5, do_breath_of_the_dragon, "", "", "", "", 1, 5, FALSE }, //vicissitude
    {"body_arsenal", "vicissitude", 6, do_body_arsenal, "", "", "", "", 1, 5, FALSE }, //vicissitude
    {"impalers_fence", "vicissitude", 7, do_impalers_fence, "", "", "", "", 1, 5, FALSE }, //vicissitude
    {"will_over_form", "vicissitude", 8, do_will_over_form, "", "", "", "", 1, 5, FALSE }, //vicissitude
    {"skin_trap", "vicissitude", 9, do_skin_trap, "", "", "", "", 1, 5, FALSE }, //vicissitude
    {"the_last_dracul", "vicissitude", 10, do_the_last_dracul, "", "", "", "", 1, 5, FALSE }, //obfuscate

    /* OBFUSCATE ABILITIES */
    {"cloak_of_shadows", "obfuscate", 1, do_cloak_of_shadows, "", "", "", "", 1, 5, FALSE }, //obfuscate
    {"mask_of_a_thousand_faces", "obfuscate", 2, do_mask_of_a_thousand_faces, "", "", "", "", 1, 5, FALSE }, //obfuscate
    {"fade_from_the_minds_eye", "obfuscate", 3, do_fade_from_the_minds_eye, "", "", "", "", 1, 5, FALSE }, //obfuscate
    {"the_silence_of_death", "obfuscate", 4, do_the_silence_of_death, "", "", "", "", 1, 5, FALSE }, //obfuscate
    {"cloak_the_gathering", "obfuscate", 5, do_cloak_the_gathering, "", "", "", "", 1, 5, FALSE }, //obfuscate
    {"manifold_guise", "obfuscate", 6, do_manifold_guise, "", "", "", "", 1, 5, FALSE }, //obfuscate
    {"fortress_of_silence", "obfuscate", 7, do_fortress_of_silence, "", "", "", "", 1, 5, FALSE }, //obfuscate
    {"mental_maze", "obfuscate", 8, do_mental_maze, "", "", "", "", 1, 5, FALSE }, //obfuscate
    {"obscure_gods_creation", "obfuscate", 9, do_obscure_gods_creation, "", "", "", "", 1, 5, FALSE }, //obfuscate
    {"veil_of_blissful_ignorance", "obfuscate", 10, do_veil_of_blissful_ignorance, "", "", "", "", 1, 5, FALSE } //obfuscate
};
