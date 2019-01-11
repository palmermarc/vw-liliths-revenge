/***************************************************************************
*  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
*  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*                                                                         *
*  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
*  Chastain, Michael Quan, and Mitchell Tse.                              *
*                                                                         *
*  In order to use any part of this Merc Diku Mud, you must comply with   *
*  both the original Diku license in 'license.doc' as well the Merc       *
*  license in 'license.txt'.  In particular, you may not remove either of *
*  these copyright notices.                                               *
*                                                                         *
*  Much time and thought has gone into this software and you are          *
*  benefitting.  We hope that you share your changes too.  What goes      *
*  around, comes around.                                                  *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

bool	check_social	args( ( CHAR_DATA *ch, char *command, char *argument ) );

/*
* Command logging types.
*/
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2

/*
* Log-all switch.
*/
bool				fLogAll		= FALSE;

/*
* Command table.
*/
const	struct	cmd_type	cmd_table	[] =
{
	/*
	* Common movement commands.
    */
    { "north",			do_north,			POS_STANDING,	0,	LOG_NEVER	},
    { "east",			do_east,			POS_STANDING,	0,	LOG_NEVER	},
    { "south",			do_south,			POS_STANDING,	0,	LOG_NEVER	},
    { "west",			do_west,			POS_STANDING,	0,	LOG_NEVER	},
    { "up",				do_up,				POS_STANDING,	0,	LOG_NEVER	},
    { "down",			do_down,			POS_STANDING,	0,	LOG_NEVER	},
    /*
    * Common other commands.
    * Placed here so one and two letter abbreviations work.
    */
    { "backup",			do_backup,			POS_DEAD,		2,  LOG_ALWAYS	},
    { "buy",			do_buy,				POS_RESTING,	0,  LOG_NORMAL	},
    { "cast",			do_cast,			POS_FIGHTING,	0,  LOG_NORMAL	},
    { "command",		do_command,			POS_STANDING,	1,  LOG_ALWAYS	},
    { "crack",			do_crack,			POS_STANDING,	0,  LOG_NORMAL	},
    { "delete",			do_delete,			POS_DEAD,		1,  LOG_ALWAYS	},
    { "diagnose",		do_diagnose,		POS_FIGHTING,	0,  LOG_NORMAL	},
    { "dismount",		do_dismount,		POS_STANDING,	0,  LOG_NORMAL	},
    { "donate",			do_donate,			POS_STANDING,	0,  LOG_NORMAL	},
    { "email",			do_email,			POS_DEAD,		0,  LOG_NORMAL	},
    { "enter",			do_enter,			POS_STANDING,	0,  LOG_NORMAL	},
    { "exits",			do_exits,			POS_RESTING,	0,  LOG_NORMAL	},
    { "get",			do_get,				POS_RESTING,	0,  LOG_ALWAYS	},
    { "inventory",		do_inventory,		POS_DEAD,		0,  LOG_NORMAL	},
    { "kill",			do_kill,			POS_FIGHTING,	0,  LOG_NORMAL	},
	{ "engage",			do_engage,			POS_FIGHTING,	0,  LOG_NORMAL	},
    { "look",			do_look,			POS_RESTING,	0,  LOG_NORMAL	},
    { "mount",			do_mount,			POS_STANDING,	0,  LOG_NORMAL	},
    { "order",			do_order,			POS_RESTING,	0,  LOG_NORMAL	},
    { "rest",			do_rest,			POS_RESTING,	0,  LOG_NORMAL	},
    { "review",			do_review,			POS_DEAD,		0,  LOG_NORMAL	},
    { "stand",			do_stand,			POS_SLEEPING,	0,  LOG_NORMAL	},
    { "tell",			do_tell,			POS_RESTING,	0,  LOG_ALWAYS	},
    { "wield",			do_wear,			POS_RESTING,	0,  LOG_NORMAL	},
    { "wizhelp",		do_wizhelp,			POS_DEAD,		4,  LOG_ALWAYS	},
    /*
    * Informational commands.
    */
    { "areas",			do_areas,			POS_DEAD,		0,  LOG_NORMAL	},
    { "bug",			do_bug,				POS_DEAD,		0,  LOG_NORMAL	},
    { "commands",		do_commands,		POS_DEAD,		0,  LOG_NORMAL	},
    { "compare",		do_compare,			POS_RESTING,	0,  LOG_NORMAL	},
    { "consider",		do_consider,		POS_RESTING,	0,  LOG_NORMAL	},
    { "credits",		do_credits,			POS_DEAD,		0,  LOG_NORMAL	},
    { "deposit",		do_deposit,			POS_STANDING,	0,  LOG_NORMAL	},
    { "clanbalance",	do_clanbalance,		POS_STANDING,	0,  LOG_NORMAL	},
    { "clandeposit",	do_clandeposit,		POS_STANDING,	0,  LOG_NORMAL	},
    { "clanwithdraw",	do_clanwithdraw,	POS_STANDING,	0,  LOG_NORMAL	},
    { "equipment",		do_equipment,		POS_DEAD,		0,  LOG_NORMAL	},
    { "examine",		do_examine,			POS_RESTING,	0,  LOG_NORMAL	},
    { "help",			do_help,			POS_DEAD,		0,  LOG_NORMAL	},
    { "idea",			do_idea,			POS_DEAD,		0,  LOG_NORMAL	},
    { "report",			do_report,			POS_DEAD,		0,  LOG_NORMAL	},
    { "score",			do_score,			POS_DEAD,		0,  LOG_NORMAL	},
    { "affect",			do_affect,			POS_DEAD,		0,  LOG_NORMAL	},
    { "skill",			do_skill,			POS_RESTING,	0,  LOG_NORMAL	},
    { "spells",			do_level,			POS_RESTING,	0,  LOG_NORMAL	},
    { "socials",		do_socials,			POS_DEAD,		0,  LOG_NORMAL	},
    { "basicstances",	do_level,			POS_DEAD,		0,  LOG_NORMAL	},
    { "astances",		do_level,			POS_DEAD,		0,  LOG_NORMAL	},
    { "weapons",		do_level,			POS_DEAD,		0,  LOG_NORMAL	},
	{ "level",			do_level,			POS_DEAD,		0,  LOG_NORMAL	},
    { "time",			do_time,			POS_DEAD,		0,  LOG_NORMAL	},
    { "typo",			do_typo,			POS_DEAD,		0,  LOG_NORMAL	},
    { "weather",		do_weather,			POS_RESTING,	0,  LOG_NORMAL	},
    { "who",			do_who,				POS_DEAD,		0,  LOG_NORMAL	},
    { "withdraw",		do_withdraw,		POS_STANDING,	0,  LOG_NORMAL	},
    { "wizlist",		do_wizlist,			POS_DEAD,		0,  LOG_NORMAL	},
    { "introduce",		do_introduce,		POS_STANDING,	1,  LOG_NORMAL	},
    { "info",			do_claninfo,		POS_STANDING,	0,	LOG_NORMAL	},
    /*
    * Configuration commands.
    */
    { "alignment",		do_alignment,		POS_DEAD,		0,	LOG_NORMAL	},
    { "channels",		do_channels,		POS_DEAD,		1,	LOG_NORMAL	},
    { "config",			do_config,			POS_DEAD,		0,	LOG_NORMAL	},
	{ "prompt",			do_prompt,			POS_DEAD,		0,	LOG_NORMAL	},
    { "description",	do_description,		POS_DEAD,		0,	LOG_NORMAL	},
    { "password",		do_password,		POS_DEAD,		0,	LOG_ALWAYS	},
    { "title",			do_title,			POS_DEAD,		0,	LOG_NORMAL	},
    { "wimpy",			do_wimpy,			POS_DEAD,		0,	LOG_NORMAL	},
    { "ansi",			do_ansi,			POS_DEAD,		0,	LOG_NORMAL	},
    { "autoexit",		do_autoexit,		POS_DEAD,		0,	LOG_NORMAL	},
    { "autoloot",		do_autoloot,		POS_DEAD,		0,	LOG_NORMAL	},
    { "autosac",		do_autosac,			POS_DEAD,		0,	LOG_NORMAL	},
	{ "autogold",		do_autogold,		POS_DEAD,		0,	LOG_NORMAL	},
	{ "autostance",		do_autostance,		POS_DEAD,		0,	LOG_NORMAL	},
    { "blank",			do_blank,			POS_DEAD,		0,	LOG_NORMAL	},
    { "brief",			do_brief,			POS_DEAD,		0,	LOG_NORMAL	},
    { "fight",			do_fightconfig,		POS_DEAD,		0,	LOG_NORMAL	},
    { "fight2",			do_fight2config,	POS_DEAD,		0,	LOG_NORMAL	},
    /*
    * Communication commands.
    */
    { "answer",			do_answer,			POS_SLEEPING,	3,  LOG_NORMAL	},
    { "bid",			do_bid,				POS_RESTING,	3,  LOG_NORMAL	},
    { "auction",		do_auction,			POS_SLEEPING,	3,  LOG_NORMAL	},
    { "justitalk",		do_justitalk,		POS_DEAD,		4,  LOG_NORMAL	},
    { "jt",			 	do_justitalk,		POS_DEAD,		4,  LOG_NORMAL	},
    { "cemote",			do_cemote,			POS_RESTING,	9,  LOG_ALWAYS	},
    { "chat",			do_chat,			POS_DEAD,		3,  LOG_ALWAYS	},
	{ "newbie",			do_newbie,			POS_DEAD,		0,  LOG_ALWAYS },
    { ".",				do_chat,			POS_DEAD,		3,  LOG_ALWAYS	},
    { "mchat",			do_mchat,			POS_DEAD,		3,  LOG_ALWAYS	},
    { "-",				do_pers,			POS_DEAD,		8,  LOG_NORMAL	},
    { "emote",			do_emote,			POS_RESTING,	0,  LOG_NORMAL	},
    { ",",				do_emote,			POS_RESTING,	0,  LOG_NORMAL	},
    { "gtell",			do_gtell,			POS_DEAD,		0,  LOG_ALWAYS	},
    { ";",				do_gtell,			POS_DEAD,		0,  LOG_NORMAL	},
    { "music",			do_music,			POS_SLEEPING,	3,  LOG_NORMAL	},
    { "nostalk",		do_nostalk,			POS_SLEEPING,	1,  LOG_ALWAYS	},
    { "brutalk",		do_brutalk,			POS_SLEEPING,	1,  LOG_ALWAYS	},
    { "malktalk",		do_malktalk,		POS_SLEEPING,	1,  LOG_ALWAYS	},
    { "ventalk",		do_ventalk,			POS_SLEEPING,	1,  LOG_ALWAYS	},
    { "tremtalk",		do_tremtalk,		POS_SLEEPING,	1,  LOG_ALWAYS	},
    { "gangtalk",		do_gangtalk,		POS_SLEEPING,	1,  LOG_ALWAYS	},
    { "tortalk",		do_tortalk,			POS_SLEEPING,	1,  LOG_ALWAYS	},
    { "captalk",		do_captalk,			POS_SLEEPING,	1,  LOG_ALWAYS	},
    { "note",			do_note,			POS_RESTING,	5,  LOG_NORMAL	},
    { "quest",			do_quest,			POS_RESTING,	3,  LOG_NORMAL	},
    { "question",		do_question,		POS_SLEEPING,	3,  LOG_NORMAL	},
    { "reply",			do_reply,			POS_RESTING,	0,  LOG_ALWAYS	},
    { "say",			do_say,				POS_RESTING,	0,  LOG_ALWAYS	},
    { "speak",			do_speak,			POS_RESTING,	0,  LOG_ALWAYS	},
    { "'",				do_say,				POS_RESTING,	0,  LOG_ALWAYS	},
    { "shout",			do_shout,			POS_RESTING,	3,  LOG_ALWAYS	},
    { "yell",			do_yell,			POS_RESTING,	3,  LOG_ALWAYS	},
    /*
    * Object manipulation commands.
    */
    { "activate",		do_activate,		POS_RESTING,	0,  LOG_NORMAL	},
    { "brandish",		do_brandish,		POS_RESTING,	0,  LOG_NORMAL	},
    { "close",			do_close,			POS_RESTING,	0,  LOG_NORMAL	},
    { "draw",			do_draw,			POS_STANDING,	0,  LOG_NORMAL	},
    { "drink",			do_drink,			POS_RESTING,	0,  LOG_NEVER	},
    { "drop",			do_drop,			POS_RESTING,	0,  LOG_NORMAL	},
    { "eat",			do_eat,				POS_RESTING,	0,  LOG_NORMAL	},
    { "empty",			do_empty,			POS_RESTING,	0,  LOG_NORMAL	},
    { "fill",			do_fill,			POS_RESTING,	0,  LOG_NORMAL	},
    { "finger",			do_finger,			POS_DEAD,		0,  LOG_NORMAL	},
    { "give",			do_give,			POS_RESTING,	0,  LOG_NORMAL	},
    { "hold",			do_wear,			POS_RESTING,	0,  LOG_NORMAL	},
    { "list",			do_list,			POS_RESTING,	0,  LOG_NORMAL	},
    { "lock",			do_lock,			POS_RESTING,	0,  LOG_NORMAL	},
    { "morph",			do_morph,			POS_STANDING,	0,  LOG_NORMAL	},
    { "open",			do_open,			POS_RESTING,	0,  LOG_NORMAL	},
    { "pick",			do_pick,			POS_RESTING,	0,  LOG_NORMAL	},
    { "press",			do_press,			POS_RESTING,	0,  LOG_NORMAL	},
    { "pull",			do_pull,			POS_RESTING,	0,  LOG_NORMAL	},
    { "put",			do_put,				POS_RESTING,	0,  LOG_NORMAL	},
    { "quaff",			do_quaff,			POS_RESTING,	0,  LOG_NORMAL	},
    { "qbuy",			do_qbuy,			POS_RESTING,    3,  LOG_NORMAL	},
    { "qlist",			do_qlist,			POS_RESTING,    3,  LOG_NORMAL	},
    { "recite",			do_recite,			POS_RESTING,	0,  LOG_NORMAL	},
    { "remove",			do_remove,			POS_RESTING,	0,  LOG_NORMAL	},
    { "sell",			do_sell,			POS_RESTING,	0,  LOG_NORMAL	},
    { "sheath",			do_sheath,			POS_STANDING,	0,  LOG_NORMAL	},
    { "take",			do_get,				POS_RESTING,	0,  LOG_NORMAL	},
    { "reload",			do_reload,			POS_STANDING,	0,  LOG_NORMAL	},
    { "unload",			do_unload,			POS_STANDING,	0,	LOG_NORMAL	},
    { "shoot",			do_shoot,			POS_STANDING,	0,	LOG_NORMAL	},
    { "throw",			do_throw,			POS_STANDING,	0,	LOG_NORMAL	},
    { "twist",			do_twist,			POS_RESTING,	0,	LOG_NORMAL	},
    { "sacrifice",		do_sacrifice,		POS_RESTING,	0,	LOG_NORMAL	},
    { "unlock",			do_unlock,			POS_RESTING,	0,	LOG_NORMAL	},
    { "value",			do_value,			POS_RESTING,	0,	LOG_NORMAL	},
    { "wear",			do_wear,			POS_RESTING,	0,	LOG_NORMAL	},
    { "voodoo",			do_voodoo,			POS_RESTING,	0,	LOG_NORMAL	},
    { "zap",			do_zap,				POS_RESTING,	0,	LOG_NORMAL	},
    /*
    * Combat commands.
    */
    { "backstab",		do_backstab,		POS_STANDING,	0,  LOG_ALWAYS	},
    { "berserk",		do_berserk,			POS_FIGHTING,	3,  LOG_ALWAYS },
    { "bs",				do_backstab,		POS_STANDING,	0,  LOG_ALWAYS	},
    { "disarm",			do_disarm,			POS_FIGHTING,	0,  LOG_ALWAYS	},
    { "flee",			do_flee,			POS_FIGHTING,	0,  LOG_NORMAL	},
    { "fightstyle",		do_fightstyle,		POS_FIGHTING,	0,  LOG_ALWAYS	},
    { "hurl",			do_hurl,			POS_STANDING,	0,  LOG_ALWAYS	},
    { "kick",			do_kick,			POS_FIGHTING,	0,  LOG_NORMAL	},
    { "punch",			do_punch,			POS_STANDING,	0,  LOG_ALWAYS	},
    { "rescue",			do_rescue,			POS_FIGHTING,	3,  LOG_ALWAYS	},
    { "stance",			do_stance,			POS_FIGHTING,	0,  LOG_NORMAL	},
    /*
    * Miscellaneous commands.
    */
    { "blindfold",		do_blindfold,		POS_STANDING,	3,  LOG_ALWAYS	},
    { "call",			do_call,			POS_STANDING,	2,  LOG_ALWAYS	},
    { "claim",			do_claim,			POS_STANDING,	0,  LOG_ALWAYS	},
    { "complete",		do_complete,		POS_STANDING,	0,  LOG_ALWAYS	},
    { "follow",			do_follow,			POS_RESTING,	2,  LOG_ALWAYS	},
    { "gag",			do_gag,				POS_STANDING,	3,  LOG_ALWAYS	},
    { "gift",			do_gift,			POS_STANDING,	0,  LOG_ALWAYS	},
    { "godless",		do_godless,			POS_DEAD,		9,  LOG_ALWAYS	},
    { "group",			do_group,			POS_DEAD,		2,  LOG_ALWAYS	},
    { "hide",			do_hide,			POS_RESTING,	0,  LOG_NORMAL	},
    { "trap",			do_trap,			POS_STANDING,	3,  LOG_ALWAYS	},
    { "hunt",			do_hunt,			POS_STANDING,	3,  LOG_ALWAYS	},
    { "home",			do_home,			POS_STANDING,	0,  LOG_ALWAYS	},
    { "locate",			do_locate,			POS_STANDING,	0,  LOG_ALWAYS	},
    { "practice",		do_practice,		POS_SLEEPING,	0,  LOG_NORMAL	},
    { "prac1",			do_prac1,			POS_SLEEPING,	0,  LOG_ALWAYS	},
    { "prac2",			do_prac2,			POS_SLEEPING,	0,  LOG_ALWAYS	},
    { "qui",			do_qui,				POS_DEAD,		0,  LOG_ALWAYS	},
    { "quit",			do_quit,			POS_SLEEPING,	0,  LOG_ALWAYS	},
    { "recall",			do_recall,			POS_FIGHTING,	0,  LOG_ALWAYS	},
    { "escape",			do_escape,			POS_DEAD,		3,  LOG_ALWAYS	},
    { "/",				do_recall,			POS_FIGHTING,	0,  LOG_ALWAYS	},
    { "recharge",		do_recharge,		POS_STANDING,	0,  LOG_ALWAYS	},
    { "refresh",		do_refresh,			POS_STANDING,	0,  LOG_ALWAYS	},
    { "rent",			do_rent,			POS_DEAD,		0,  LOG_ALWAYS	},
    { "safe",			do_safe,			POS_STANDING,	0,  LOG_ALWAYS	},
    { "save",			do_save,			POS_DEAD,		0,  LOG_ALWAYS	},
    { "sleep",			do_sleep,			POS_SLEEPING,	0,  LOG_ALWAYS	},
    { "smother",		do_smother,			POS_STANDING,	0,  LOG_ALWAYS	},
    { "sneak",			do_sneak,			POS_STANDING,	0,  LOG_ALWAYS	},
    { "split",			do_split,			POS_RESTING,	0,  LOG_ALWAYS	},
    { "scan",			do_scan,			POS_RESTING,	0,  LOG_NORMAL	},
    { "spy",			do_spy,				POS_RESTING,	0,  LOG_ALWAYS	},
    { "steal",			do_steal,			POS_STANDING,	0,  LOG_ALWAYS	},
    { "summon",			do_summon,			POS_DEAD,		0,  LOG_ALWAYS	},
    { "struggle",		do_struggle,		POS_DEAD,		0,  LOG_ALWAYS	},
    { "tie",			do_tie,				POS_STANDING,	0,  LOG_ALWAYS	},
    { "token",			do_token,			POS_STANDING,	6,  LOG_ALWAYS	},
    { "train",			do_train,			POS_RESTING,	0,  LOG_NORMAL	},
    { "untie",			do_untie,			POS_STANDING,	3,  LOG_ALWAYS	},
    { "visible",		do_visible,			POS_SLEEPING,	0,  LOG_ALWAYS	},
    { "wake",			do_wake,			POS_SLEEPING,	0,  LOG_ALWAYS	},
	{ "where",			do_where,			POS_RESTING,	0,  LOG_NORMAL },
	{ "remort",			do_remort,			POS_STANDING,	3,  LOG_NORMAL },
    /*
    * Vampire commands.
    */
    { "bite",			do_bite,			POS_STANDING,	3,  LOG_ALWAYS	},
    { "change",			do_change,			POS_STANDING,	3,  LOG_ALWAYS	},
    { "clandisc",		do_clandisc,		POS_RESTING,	1,  LOG_ALWAYS	},
    { "clanitem",		do_clanitem,		POS_STANDING,	3,  LOG_ALWAYS	},
    { "clanname",		do_clanname,		POS_STANDING,	3,  LOG_ALWAYS	},
    { "claws",			do_claws,			POS_RESTING,	3,  LOG_ALWAYS	},
    { "darkheart",		do_darkheart,		POS_STANDING,	3,  LOG_ALWAYS	},
	{ "decapitate",		do_decapitate,		POS_STANDING,	3,	LOG_ALWAYS },
	{ "diablerize",		do_diablerize,		POS_STANDING,	3,	LOG_ALWAYS },
    { "evileye",		do_evileye,			POS_RESTING,	3,  LOG_ALWAYS	},
    { "fangs",			do_fangs,			POS_RESTING,	3,  LOG_ALWAYS	},
    { "favour",			do_favour,			POS_RESTING,	3,  LOG_ALWAYS	},
    { "feed",			do_feed,			POS_RESTING,	3,  LOG_ALWAYS	},
    { "mask",			do_mask,			POS_STANDING,	3,  LOG_ALWAYS	},
    { "member",			do_member,			POS_DEAD,		3,  LOG_ALWAYS	},
    { "mortal",			do_mortal,			POS_FIGHTING,	3,  LOG_ALWAYS	},
    { "nightsight",		do_nightsight,		POS_RESTING,	3,	LOG_ALWAYS	},
    { "poison",			do_poison,			POS_STANDING,	3,	LOG_ALWAYS	},
    { "readaura",		do_readaura,		POS_STANDING,	3,  LOG_ALWAYS	},
    { "regenerate",		do_regenerate,		POS_SLEEPING,	3,  LOG_NEVER	},
    { "stake",			do_stake,			POS_STANDING,	3,  LOG_ALWAYS	},
    { "scry",			do_scry,			POS_STANDING,	3,  LOG_NEVER	},
    { "serpent",		do_serpent,			POS_STANDING,	3,  LOG_ALWAYS	},
    { "shadowplane",	do_shadowplane,		POS_STANDING,	3,	LOG_ALWAYS	},
    { "shadowsight",	do_shadowsight,		POS_RESTING,	3,	LOG_ALWAYS	},
    { "shield",			do_shield,			POS_RESTING,	3,	LOG_ALWAYS	},
    { "truesight",		do_truesight,		POS_RESTING,	3,	LOG_ALWAYS	},
    { "upkeep",			do_upkeep,			POS_DEAD,		3,	LOG_ALWAYS	},
    { "vclan",			do_vclan,			POS_DEAD,		3,	LOG_ALWAYS	},
    { "vouch",			do_vouch,			POS_STANDING,	3,	LOG_ALWAYS	},
    /*
    * Immortal commands.
    */
	{ "doubleexp",		do_doublexp,		POS_DEAD,		9,	LOG_ALWAYS	},
    { "trust",			do_trust,			POS_DEAD,		9,	LOG_ALWAYS	},
    { "omember",		do_omember,			POS_DEAD,		9,	LOG_ALWAYS	},
    { "addlag",			do_addlag,			POS_DEAD,		6,	LOG_ALWAYS	},
    { "sinbin",			do_sinbin,			POS_DEAD,		5,	LOG_NEVER	},
	{ "allow",			do_allow,			POS_DEAD,		9,	LOG_ALWAYS	},
	{ "ban",			do_ban,				POS_DEAD,		9,	LOG_ALWAYS	},
	{ "bind",			do_bind,			POS_DEAD,		8,	LOG_ALWAYS	},
	{ "clearvam",		do_clearvam,		POS_STANDING,	1,	LOG_NORMAL	},
    { "clearvamp",		do_clearvamp,		POS_STANDING,	9,	LOG_ALWAYS	},
    { "create",			do_create,			POS_STANDING,	9,	LOG_NORMAL	},
    { "deny",			do_deny,			POS_DEAD,		9,	LOG_ALWAYS	},
    { "punt",			do_punt,			POS_DEAD,		7,	LOG_ALWAYS	},
    { "disconnect",		do_disconnect,		POS_DEAD,		7,	LOG_ALWAYS	},
    { "freeze",			do_freeze,			POS_DEAD,		7,	LOG_ALWAYS	},
    { "gwho",			do_godwho,			POS_DEAD,		7,	LOG_ALWAYS	},
    { "outcast",		do_outcast,			POS_DEAD,		4,	LOG_ALWAYS	},
    { "qset",			do_qset,			POS_DEAD,		8,	LOG_ALWAYS	},
    { "qstat",			do_qstat,			POS_DEAD,		8,	LOG_ALWAYS	},
    { "qtrust",			do_qtrust,			POS_DEAD,		9,	LOG_ALWAYS	},
    { "reboo",			do_reboo,			POS_DEAD,		9,	LOG_NORMAL	},
    { "reboot",			do_reboot,			POS_DEAD,		9,	LOG_ALWAYS	},
    { "shutdow",		do_shutdow,			POS_DEAD,		9,	LOG_NORMAL	},
    { "copyover",		do_copyover,		POS_DEAD,		9,	LOG_ALWAYS	},
    { "shutdown",		do_shutdown,		POS_DEAD,		9,	LOG_ALWAYS	},
    { "users",			do_users,			POS_DEAD,		6,	LOG_NORMAL	},
    { "connections",	do_connections,		POS_DEAD,		6,	LOG_NORMAL	},
    { "wizlock",		do_wizlock,			POS_DEAD,		9,	LOG_ALWAYS	},
    { "watcher",		do_watcher,			POS_DEAD,		2,	LOG_NEVER	},
    { "updateleague",	do_updateleague,	POS_DEAD,		10,	LOG_ALWAYS	},
    { "force",			do_force,			POS_DEAD,		7,	LOG_ALWAYS	},
    { "mclear",			do_mclear,			POS_DEAD,		6,	LOG_ALWAYS	},
    { "mload",			do_mload,			POS_DEAD,		8,  LOG_ALWAYS	},
    { "mset",			do_mset,			POS_DEAD,		8,  LOG_ALWAYS	},
    { "noemote",		do_noemote,			POS_DEAD,		6,  LOG_ALWAYS	},
    { "notell",			do_notell,			POS_DEAD,		6,  LOG_ALWAYS	},
    { "oclone",			do_oclone,			POS_DEAD,		8,  LOG_ALWAYS	},
    { "oload",			do_oload,			POS_DEAD,		8,  LOG_ALWAYS	},
    { "oset",			do_oset,			POS_DEAD,		7,  LOG_ALWAYS	},
    { "otransfer",		do_otransfer,		POS_DEAD,		8,  LOG_ALWAYS	},
    { "personal",		do_personal,		POS_DEAD,		9,  LOG_ALWAYS	},
    { "pload",			do_pload,			POS_DEAD,		9,  LOG_ALWAYS	},
    { "preturn",		do_preturn,			POS_DEAD,		2,  LOG_ALWAYS	},
    { "purge",			do_purge,			POS_DEAD,		7,  LOG_ALWAYS	},
    { "qmake",			do_qmake,			POS_DEAD,		9,  LOG_ALWAYS	},
    { "release",		do_release,			POS_DEAD,		8,  LOG_ALWAYS	},
    { "relevel",		do_relevel,			POS_DEAD,		1,	LOG_NORMAL	},
    { "armageddon",		do_muddie,			POS_DEAD,		0,	LOG_ALWAYS	},
    { "avatar",			do_avatar,			POS_DEAD,		9,  LOG_ALWAYS	},
    { "restore",		do_restore,			POS_STANDING,	6,  LOG_ALWAYS	},
    { "rset",			do_rset,			POS_DEAD,		7,  LOG_ALWAYS	},
    { "silence",		do_silence,			POS_DEAD,		6,  LOG_ALWAYS	},
    { "sla",			do_sla,				POS_DEAD,		7,  LOG_NORMAL	},
    { "slay",			do_slay,			POS_DEAD,		7,  LOG_ALWAYS	},
    { "sset",			do_sset,			POS_DEAD,		8,	LOG_ALWAYS	},
    { "transfer",		do_transfer,		POS_DEAD,		7,	LOG_ALWAYS	},
    { "at",				do_at,				POS_DEAD,		6,	LOG_ALWAYS	},
    { "bamfin",			do_bamfin,			POS_DEAD,		6,	LOG_NORMAL	},
    { "bamfout",		do_bamfout,			POS_DEAD,		6,	LOG_NORMAL	},
    { "echo",			do_echo,			POS_DEAD,		8,	LOG_ALWAYS	},
    { "goto",			do_goto,			POS_DEAD,		6,	LOG_NORMAL	},
    { "holylight",		do_holylight,		POS_DEAD,		4,	LOG_NORMAL	},
    { "invis",			do_invis,			POS_DEAD,		6,	LOG_NORMAL	},
    { "log",			do_log,				POS_DEAD,		7,	LOG_ALWAYS	},
    { "memory",			do_memory,			POS_DEAD,		6,	LOG_NORMAL	},
    { "mfind",			do_mfind,			POS_DEAD,		7,	LOG_NORMAL	},
    { "mstat",			do_mstat,			POS_DEAD,		7,	LOG_NORMAL	},
    { "mwhere",			do_mwhere,			POS_DEAD,		7,	LOG_NORMAL	},
    { "ofind",			do_ofind,			POS_DEAD,		7,	LOG_NORMAL	},
    { "ostat",			do_ostat,			POS_DEAD,		6,	LOG_NORMAL	},
    { "oswitch",		do_oswitch,			POS_DEAD,		7,	LOG_ALWAYS	},
    { "oreturn",		do_oreturn,			POS_DEAD,		7,	LOG_ALWAYS	},
    { "peace",			do_peace,			POS_DEAD,		6,	LOG_ALWAYS	},
    { "recho",			do_recho,			POS_DEAD,		6,	LOG_ALWAYS	},
    { "return",			do_return,			POS_DEAD,		7,	LOG_NORMAL	},
    { "rstat",			do_rstat,			POS_DEAD,		6,	LOG_NORMAL	},
	{ "aload",			do_aload,			POS_DEAD,		8,	LOG_ALWAYS	},
	{ "astat",			do_astat,			POS_DEAD,		8,	LOG_ALWAYS	},
	{ "hstat",			do_hstat,			POS_DEAD,		8,	LOG_ALWAYS	},
	{ "asave",			do_asave,			POS_DEAD,		8,	LOG_ALWAYS	},
	{ "hedit",			do_hedit,			POS_DEAD,		8,	LOG_ALWAYS	},
	{ "raz_custom",		do_customcommand,	POS_DEAD,		8,	LOG_ALWAYS	},
    { "slookup",		do_slookup,			POS_DEAD,		8,	LOG_NORMAL	},
    { "snoop",			do_snoop,			POS_DEAD,		6,	LOG_NORMAL	},
    { "switch",			do_switch,			POS_DEAD,		7,	LOG_ALWAYS	},
    { "ordain",			do_ordain,			POS_DEAD,		9,	LOG_ALWAYS	},
    { "sabbat",			do_sabbat,			POS_DEAD,		9,	LOG_ALWAYS	},
    { "vamptalk",		do_vamptalk,		POS_DEAD,		1,	LOG_NORMAL	},
    { "vtalk",			do_vamptalk,		POS_DEAD,		1,	LOG_NORMAL	},
    { ">",				do_vamptalk,		POS_DEAD,		1,	LOG_NORMAL	},
    { "vampire",		do_vampire,			POS_STANDING,	0,	LOG_NORMAL	},
    { "immune",			do_immune,			POS_DEAD,		0,	LOG_NORMAL	},
    { "immtalk",		do_immtalk,			POS_DEAD,		6,	LOG_NORMAL	},
    { ":",				do_immtalk,			POS_DEAD,		6,	LOG_NORMAL	},
    { "transport",		do_transport,		POS_DEAD,		0,	LOG_NORMAL	},
	{ "prefix",			do_prefix,			POS_DEAD,		6,  LOG_NORMAL  },
    /*
    * End of list.
    */
    { "",				0,					POS_DEAD,		0,	LOG_NORMAL	}
};



/*
* The social table.
* Add new socials here.
* Alphabetical order is not required.
*/
const	struct	social_type	social_table [] =
{
	{
		"accuse",
		"Accuse whom?",
		"$n is in an accusing mood.",
		"You look accusingly at $M.",
		"$n looks accusingly at $N.",
		"$n looks accusingly at you.",
		"You accuse yourself.",
		"$n seems to have a bad conscience."
	}, {
		"afk",
		"You announce that you are going away from keyboard.",
		"$n announces that $e is going away from keyboard.",
		"You announce that you are going away from keyboard.",
		"$n informs $N that $e is going away from keyboard.",
		"$n informs you that $e is going away from keyboard.",
		"You announce that you are going away from keyboard.",
		"$n announces that $e is going away from keyboard.",
	}, {
		"applaud",
		"Clap, clap, clap.",
		"$n gives a round of applause.",
		"You clap at $S actions.",
		"$n claps at $N's actions.",
		"$n gives you a round of applause.  You MUST'VE done something good!",
		"You applaud at yourself.  Boy, are we conceited!",
		"$n applauds at $mself.  Boy, are we conceited!"
	}, {
		"banzai",
		"You scream 'BANZAI!!!!' and charge into the fray.",
		"$n screams 'BANZAI!!!!' and charges into the fray.",
		"You scream 'BANZAI!!!!' and drag $M into the fray with you.",
		"$n screams 'BANZAI!!!!' and drags $N into the fray with $m!",
		"$n screams 'BANZAI!!!!' and drags you into the fray with $m!",
		"You scream 'BANZAI!!!!' and brandish your weapon for battle.",
		"$n screams 'BANZAI!!!!' and brandishes $s weapon for battle.",
	}, {
		"bark",
		"Woof!  Woof!",
		"$n barks like a dog.",
		"You bark at $M.",
		"$n barks at $N.",
		"$n barks at you.",
		"You bark at yourself.  Woof!  Woof!",
		"$n barks at $mself.  Woof!  Woof!"
	}, {
		"beer",
		"You down a cold, frosty beer.",
		"$n downs a cold, frosty beer.",
		"You draw a cold, frosty beer for $N.",
		"$n draws a cold, frosty beer for $N.",
		"$n draws a cold, frosty beer for you.",
		"You draw yourself a beer.",
		"$n draws $mself a beer."
	}, {
		"beg",
		"You beg the gods for mercy.",
		"The gods fall down laughing at $n's request for mercy.",
		"You desperately try to squeeze a few coins from $M.",
		"$n begs you for money.",
		"$n begs $N for a gold piece!",
		"Begging yourself for money doesn't help.",
		"$n begs himself for money."
	}, {
		"behead",
		"You look around for some heads to cut off.",
		"$n looks around for some heads to cut off.",
		"You grin evilly at $N and brandish your weapon.",
		"$n grins evilly at $N, while branding $s weapon!",
		"$n grins evilly at you, brandishing $s weapon.",
		"I really don't think you want to do that...",
		"$n is so desperate for exp that $e tries to decapitate $mself!"
	}, {
		"blownose",
		"You blow your nose loudly.",
		"$n blows $s nose loudly.",
		"You blow your nose on $S shirt.",
		"$n blows $s nose on $N's shirt.",
		"$n blows $s nose on your shirt.",
		"You blow your nose on your shirt.",
		"$n blows $s nose on $s shirt."
	}, {
		"blush",
		"Your cheeks are burning.",
		"$n blushes.",
		"You get all flustered up seeing $M.",
		"$n blushes as $e sees $N here.",
		"$n blushes as $e sees you here.  Such an effect on people!",
		"You blush at your own folly.",
		"$n blushes as $e notices $s boo-boo."
	}, {
		"boggle",
		"You boggle at all the loonies around you.",
		"$n boggles at all the loonies around $m.",
		"You boggle at $S ludicrous idea.",
		"$n boggles at $N's ludicrous idea.",
		"$n boggles at your ludicrous idea.",
		"BOGGLE.",
		"$n wonders what BOGGLE means."
	}, {
		"bonk",
		"BONK.",
		"$n spells 'potato' like Dan Quayle: 'B-O-N-K'.",
		"You bonk $M for being a numbskull.",
		"$n bonks $N.  What a numbskull.",
		"$n bonks you.  BONK BONK BONK!",
		"You bonk yourself.",
		"$n bonks $mself."
	}, {
		"bounce",
		"BOIINNNNNNGG!",
		"$n bounces around.",
		"You bounce onto $S lap.",
		"$n bounces onto $N's lap.",
		"$n bounces onto your lap.",
		"You bounce your head like a basketball.",
		"$n plays basketball with $s head."
	}, {
		"bow",
		"You bow deeply.",
		"$n bows deeply.",
		"You bow before $M.",
		"$n bows before $N.",
		"$n bows before you.",
		"You kiss your toes.",
		"$n folds up like a jack knife and kisses $s own toes."
	}, {
		"brb",
		"You announce that you will be right back.",
		"$n announces that $e will be right back.",
		"You announce that you will be right back.",
		"$n informs $N that $e will be right back.",
		"$n informs you that $e will be right back.",
		"You announce that you will be right back.",
		"$n announces that $e will be right back.",
	}, {
		"burp",
		"You burp loudly.",
		"$n burps loudly.",
		"You burp loudly to $M in response.",
		"$n burps loudly in response to $N's remark.",
		"$n burps loudly in response to your remark.",
		"You burp at yourself.",
		"$n burps at $mself.  What a sick sight."
	}, {
		"cackle",
		"You throw back your head and cackle with insane glee!",
		"$n throws back $s head and cackles with insane glee!",
		"You cackle gleefully at $N",
		"$n cackles gleefully at $N.",
		"$n cackles gleefully at you.  Better keep your distance from $m.",
		"You cackle at yourself.  Now, THAT'S strange!",
		"$n is really crazy now!  $e cackles at $mself."
	}, {
		"challenge",
		"Challenge who?",
		NULL,
		"You challenge $N to a fight to the death.",
		"$n challenges $N to a fight to the death.",
		"$n challenges you to a fight to the death.",
		"Challenge YOURSELF to a fight to the death?  I think not...",
		NULL
	}, {
		"chuckle",
		"You chuckle politely.",
		"$n chuckles politely.",
		"You chuckle at $S joke.",
		"$n chuckles at $N's joke.",
		"$n chuckles at your joke.",
		"You chuckle at your own joke, since no one else would.",
		"$n chuckles at $s own joke, since none of you would."
	}, {
		"clap",
		"You clap your hands together.",
		"$n shows $s approval by clapping $s hands together.",
		"You clap at $S performance.",
		"$n claps at $N's performance.",
		"$n claps at your performance.",
		"You clap at your own performance.",
		"$n claps at $s own performance."
	}, {
		"comb",
		"You comb your hair - perfect.",
		"$n combs $s hair, how dashing!",
		"You patiently untangle $N's hair - what a mess!",
		"$n tries patiently to untangle $N's hair.",
		"$n pulls your hair in an attempt to comb it.",
		"You pull your hair, but it will not be combed.",
		"$n tries to comb $s tangled hair."
	}, {
		"comfort",
		"Do you feel uncomfortable?",
		NULL,
		"You comfort $M.",
		"$n comforts $N.",
		"$n comforts you.",
		"You make a vain attempt to comfort yourself.",
		"$n has no one to comfort $m but $mself."
	}, {
		"cringe",
		"You cringe in terror.",
		"$n cringes in terror!",
		"You cringe away from $M.",
		"$n cringes away from $N in mortal terror.",
		"$n cringes away from you.",
		"I beg your pardon?",
		NULL
	}, {
		"cry",
		"Waaaaah ...",
		"$n bursts into tears.",
		"You cry on $S shoulder.",
		"$n cries on $N's shoulder.",
		"$n cries on your shoulder.",
		"You cry to yourself.",
		"$n sobs quietly to $mself."
	}, {
		"cuddle",
		"Whom do you feel like cuddling today?",
		NULL,
		"You cuddle $M.",
		"$n cuddles $N.",
		"$n cuddles you.",
		"You must feel very cuddly indeed ... :)",
		"$n cuddles up to $s shadow.  What a sorry sight."
	}, {
		"curse",
		"You swear loudly for a long time.",
		"$n swears: @*&^%@*&!",
		"You swear at $M.",
		"$n swears at $N.",
		"$n swears at you!  Where are $s manners?",
		"You swear at your own mistakes.",
		"$n starts swearing at $mself.  Why don't you help?"
	}, {
		"curtsey",
		"You curtsey to your audience.",
		"$n curtseys gracefully.",
		"You curtsey to $M.",
		"$n curtseys gracefully to $N.",
		"$n curtseys gracefully for you.",
		"You curtsey to your audience (yourself).",
		"$n curtseys to $mself, since no one is paying attention to $m."
	}, {
		"dance",
		"Feels silly, doesn't it?",
		"$n tries to break dance, but nearly breaks $s neck!",
		"You sweep $M into a romantic waltz.",
		"$n sweeps $N into a romantic waltz.",
		"$n sweeps you into a romantic waltz.",
		"You skip and dance around by yourself.",
		"$n dances a pas-de-une."
	}, {
		"drool",
		"You drool on yourself.",
		"$n drools on $mself.",
		"You drool all over $N.",
		"$n drools all over $N.",
		"$n drools all over you.",
		"You drool on yourself.",
		"$n drools on $mself."
	}, 	{
		"fart",
		"Where are your manners?",
		"$n lets off a real rip-roarer ... a greenish cloud envelops $n!",
		"You fart at $M.  Boy, you are sick.",
		"$n farts in $N's direction.  Better flee before $e turns to you!",
		"$n farts in your direction.  You gasp for air.",
		"You fart at yourself.  You deserve it.",
		"$n farts at $mself.  Better $m than you."
	}, {
		"flip",
		"You flip head over heels.",
		"$n flips head over heels.",
		"You flip $M over your shoulder.",
		"$n flips $N over $s shoulder.",
		"$n flips you over $s shoulder.  Hmmmm.",
		"You tumble all over the room.",
		"$n does some nice tumbling and gymnastics."
	}, {
		"fondle",
		"Who needs to be fondled?",
		NULL,
		"You fondly fondle $M.",
		"$n fondly fondles $N.",
		"$n fondly fondles you.",
		"You fondly fondle yourself, feels funny doesn't it ?",
		"$n fondly fondles $mself - this is going too far !!"
	}, {
		"french",
		"Kiss whom?",
		NULL,
		"You give $N a long and passionate kiss.",
		"$n kisses $N passionately.",
		"$n gives you a long and passionate kiss.",
		"You gather yourself in your arms and try to kiss yourself.",
		"$n makes an attempt at kissing $mself."
	}, {
		"frown",
		"What's bothering you ?",
		"$n frowns.",
		"You frown at what $E did.",
		"$n frowns at what $E did.",
		"$n frowns at what you did.",
		"You frown at yourself.  Poor baby.",
		"$n frowns at $mself.  Poor baby."
	}, {
		"fume",
		"You grit your teeth and fume with rage.",
		"$n grits $s teeth and fumes with rage.",
		"You stare at $M, fuming.",
		"$n stares at $N, fuming with rage.",
		"$n stares at you, fuming with rage!",
		"That's right - hate yourself!",
		"$n clenches $s fists and stomps his feet, fuming with anger."
	}, {
		"gasp",
		"You gasp in astonishment.",
		"$n gasps in astonishment.",
		"You gasp as you realize what $e did.",
		"$n gasps as $e realizes what $N did.",
		"$n gasps as $e realizes what you did.",
		"You look at yourself and gasp!",
		"$n takes one look at $mself and gasps in astonisment!"
	}, {
		"giggle",
		"You giggle.",
		"$n giggles.",
		"You giggle in $S's presence.",
		"$n giggles at $N's actions.",
		"$n giggles at you.  Hope it's not contagious!",
		"You giggle at yourself.  You must be nervous or something.",
		"$n giggles at $mself.  $e must be nervous or something."
	}, {
		"glare",
		"You glare at nothing in particular.",
		"$n glares around $m.",
		"You glare icily at $M.",
		"$n glares at $N.",
		"$n glares icily at you, you feel cold to your bones.",
		"You glare icily at your feet, they are suddenly very cold.",
		"$n glares at $s feet, what is bothering $m?"
	}, {
		"grin",
		"You grin evilly.",
		"$n grins evilly.",
		"You grin evilly at $M.",
		"$n grins evilly at $N.",
		"$n grins evilly at you.  Hmmm.  Better keep your distance.",
		"You grin at yourself.  You must be getting very bad thoughts.",
		"$n grins at $mself.  You must wonder what's in $s mind."
	}, {
		"groan",
		"You groan loudly.",
		"$n groans loudly.",
		"You groan at the sight of $M.",
		"$n groans at the sight of $N.",
		"$n groans at the sight of you.",
		"You groan as you realize what you have done.",
		"$n groans as $e realizes what $e has done."
	}, {
		"grope",
		"Whom do you wish to grope?",
		NULL,
		"Well, what sort of noise do you expect here?",
		"$n gropes $N.",
		"$n gropes you.",
		"You grope yourself - YUCK.",
		"$n gropes $mself - YUCK."
	}, {
		"grovel",
		"You grovel in the dirt.",
		"$n grovels in the dirt.",
		"You grovel before $M.",
		"$n grovels in the dirt before $N.",
		"$n grovels in the dirt before you.",
		"That seems a little silly to me.",
		NULL
	}, {
		"growl",
		"Grrrrrrrrrr ...",
		"$n growls.",
		"Grrrrrrrrrr ... take that, $N!",
		"$n growls at $N.  Better leave the room before the fighting starts.",
		"$n growls at you.  Hey, two can play it that way!",
		"You growl at yourself.  Boy, do you feel bitter!",
		"$n growls at $mself.  This could get interesting..."
	}, {
		"grumble",
		"You grumble.",
		"$n grumbles.",
		"You grumble to $M.",
		"$n grumbles to $N.",
		"$n grumbles to you.",
		"You grumble under your breath.",
		"$n grumbles under $s breath."
	}, {
		"grunt",
		"GRNNNHTTTT.",
		"$n grunts like a pig.",
		"GRNNNHTTTT.",
		"$n grunts to $N.  What a pig!",
		"$n grunts to you.  What a pig!",
		"GRNNNHTTTT.",
		"$n grunts to nobody in particular.  What a pig!"
	}, {
		"hand",
		"Kiss whose hand?",
		NULL,
		"You kiss $S hand.",
		"$n kisses $N's hand.  How continental!",
		"$n kisses your hand.  How continental!",
		"You kiss your own hand.",
		"$n kisses $s own hand."
	}, {
		"heal",
		"You start yelling for a heal!",
		"$n yells 'Hey, how about a heal? I'm DYING here!'",
		"You start yelling at $N for a heal!",
		"$n yells 'Hey $N, how about a heal? I'm DYING here!'",
		"$n yells 'Hey $N, how about a heal? I'm DYING here!'",
		"You start yelling for a heal!",
		"$n yells 'Hey, how about a heal? I'm DYING here!'"
	}, {
		"hop",
		"You hop around like a little kid.",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	}, {
		"hug",
		"Hug whom?",
		NULL,
		"You hug $M.",
		"$n hugs $N.",
		"$n hugs you.",
		"You hug yourself.",
		"$n hugs $mself in a vain attempt to get friendship."
	}, {
		"kiss",
		"Isn't there someone you want to kiss?",
		NULL,
		"You kiss $M.",
		"$n kisses $N.",
		"$n kisses you.",
		"All the lonely people :(",
		NULL
	}, {
		"laugh",
		"You laugh.",
		"$n laughs.",
		"You laugh at $N mercilessly.",
		"$n laughs at $N mercilessly.",
		"$n laughs at you mercilessly.  Hmmmmph.",
		"You laugh at yourself.  I would, too.",
		"$n laughs at $mself.  Let's all join in!!!"
	}, {
		"lag",
		"You complain about the terrible lag.",
		"$n starts complaining about the terrible lag.",
		"You complain to $N about the terrible lag.",
		"$n complains to $N about the terrible lag.",
		"$n complains to you about the terrible lag.",
		"You start muttering about the awful lag.",
		"$n starts muttering about the awful lag."
	}, {
		"lick",
		"You lick your lips and smile.",
		"$n licks $s lips and smiles.",
		"You lick $M.",
		"$n licks $N.",
		"$n licks you.",
		"You lick yourself.",
		"$n licks $mself - YUCK."
	}, {
		"love",
		"You love the whole world.",
		"$n loves everybody in the world.",
		"You tell your true feelings to $N.",
		"$n whispers softly to $N.",
		"$n whispers to you sweet words of love.",
		"Well, we already know you love yourself (lucky someone does!)",
		"$n loves $mself, can you believe it ?"
	}, {
		"lust",
		"You are getting lusty feelings!",
		"$n looks around lustily.",
		"You stare lustily at $N.",
		"$n stares lustily at $N.",
		"$n stares lustily at you.",
		"You stare lustily at...youself?",
		"$n looks $mself up and down lustily."
	}, {
		"massage",
		"Massage what?  Thin air?",
		NULL,
		"You gently massage $N's shoulders.",
		"$n massages $N's shoulders.",
		"$n gently massages your shoulders.  Ahhhhhhhhhh ...",
		"You practice yoga as you try to massage yourself.",
		"$n gives a show on yoga positions, trying to massage $mself."
	}, {
		"moan",
		"You start to moan.",
		"$n starts moaning.",
		"You moan for the loss of $m.",
		"$n moans for the loss of $N.",
		"$n moans at the sight of you.  Hmmmm.",
		"You moan at yourself.",
		"$n makes $mself moan."
	}, {
		"mmm",
		"You go mmMMmmMMmmMMmm.",
		"$n says 'mmMMmmMMmmMMmm.'",
		"You go mmMMmmMMmmMMmm.",
		"$n says 'mmMMmmMMmmMMmm.'",
		"$n thinks of you and says, 'mmMMmmMMmmMMmm.'",
		"You think of yourself and go mmMMmmMMmmMMmm.",
		"$n thinks of $mself and says 'mmMMmmMMmmMMmm.'",
	}, {
		"nibble",
		"Nibble on whom?",
		NULL,
		"You nibble on $N's ear.",
		"$n nibbles on $N's ear.",
		"$n nibbles on your ear.",
		"You nibble on your OWN ear.",
		"$n nibbles on $s OWN ear."
	}, {
		"nod",
		"You nod in agreement.",
		"$n nods in agreement.",
		"You nod in recognition to $M.",
		"$n nods in recognition to $N.",
		"$n nods in recognition to you.  You DO know $m, right?",
		"You nod at yourself.  Are you getting senile?",
		"$n nods at $mself.  $e must be getting senile."
	}, {
		"nose",
		"You wiggle your nose.",
		"$n wiggles $s nose.",
		"You tweek $S nose.",
		"$n tweeks $N's nose.",
		"$n tweeks your nose.",
		"You tweek your own nose!",
		"$n tweeks $s own nose!"
	}, {
		"nudge",
		"Nudge whom?",
		NULL,
		"You nudge $M.",
		"$n nudges $N.",
		"$n nudges you.",
		"You nudge yourself, for some strange reason.",
		"$n nudges $mself, to keep $mself awake."
	}, {
		"nuzzle",
		"Nuzzle whom?",
		NULL,
		"You nuzzle $S neck softly.",
		"$n softly nuzzles $N's neck.",
		"$n softly nuzzles your neck.",
		"I'm sorry, friend, but that's impossible.",
		NULL
	}, {
		"pat",
		"Pat whom?",
		NULL,
		"You pat $N on $S head.",
		"$n pats $N on $S head.",
		"$n pats you on your head.",
		"You pat yourself on your head.",
		"$n pats $mself on the head."
	}, {
		"point",
		"Point at whom?",
		NULL,
		"You point at $M accusingly.",
		"$n points at $N accusingly.",
		"$n points at you accusingly.",
		"You point proudly at yourself.",
		"$n points proudly at $mself."
	}, {
		"poke",
		"Poke whom?",
		NULL,
		"You poke $M in the ribs.",
		"$n pokes $N in the ribs.",
		"$n pokes you in the ribs.",
		"You poke yourself in the ribs, feeling very silly.",
		"$n pokes $mself in the ribs, looking very sheepish."
	}, {
		"ponder",
		"You ponder the question.",
		"$n sits down and thinks deeply.",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	}, {
		"pout",
		"Ah, don't take it so hard.",
		"$n pouts.",
		"You pout at $M.",
		"$n pouts at $N.",
		"$n pouts at you.",
		"Ah, don't take it so hard.",
		"$n pouts."
	}, {
		"pray",
		"You feel righteous, and maybe a little foolish.",
		"$n begs and grovels to the powers that be.",
		"You crawl in the dust before $M.",
		"$n falls down and grovels in the dirt before $N.",
		"$n kisses the dirt at your feet.",
		"Talk about narcissism ...",
		"$n mumbles a prayer to $mself."
	}, {
		"puke",
		"You puke ... chunks everywhere!",
		"$n pukes.",
		"You puke on $M.",
		"$n pukes on $N.",
		"$n spews vomit and pukes all over your clothing!",
		"You puke on yourself.",
		"$n pukes on $s clothes."
	}, {
		"purr",
		"MMMMEEEEEEEEOOOOOOOOOWWWWWWWWWWWW.",
		"$n purrs contentedly.",
		"You purr contentedly in $M lap.",
		"$n purrs contentedly in $N's lap.",
		"$n purrs contentedly in your lap.",
		"You purr at yourself.",
		"$n purrs at $mself.  Must be a cat thing."
	}, {
		"rofl",
		"You roll on the floor with laughter.",
		"$n rolls on the floor with laughter.",
		"You roll on the floor laughing at $N.",
		"$n rolls on the floor laughing at $N.",
		"$n rolls on the floor laughing at you.",
		"You roll on the floor with laughter.",
		"$n rolls on the floor with laughter."
	}, {
		"ruffle",
		"You've got to ruffle SOMEONE.",
		NULL,
		"You ruffle $N's hair playfully.",
		"$n ruffles $N's hair playfully.",
		"$n ruffles your hair playfully.",
		"You ruffle your hair.",
		"$n ruffles $s hair."
	}, {
		"runaway",
		"You scream 'RUN AWAY! RUN AWAY!'.",
		"$n screams 'RUN AWAY! RUN AWAY!'.",
		"You scream '$N, QUICK! RUN AWAY!'.",
		"$n screams '$N, QUICK! RUN AWAY!'.",
		"$n screams '$N, QUICK! RUN AWAY!'.",
		"You desperately look for somewhere to run to!",
		"$n looks like $e's about to run away.",
	}, {
		"scream",
		"ARRRRRRRRRRGH!!!!!",
		"$n screams loudly!",
		"ARRRRRRRRRRGH!!!!!  Yes, it MUST have been $S fault!!!",
		"$n screams loudly at $N.  Better leave before $n blames you, too!!!",
		"$n screams at you!  That's not nice!  *sniff*",
		"You scream at yourself.  Yes, that's ONE way of relieving tension!",
		"$n screams loudly at $mself!  Is there a full moon up?"
	}, {
		"shake",
		"You shake your head.",
		"$n shakes $s head.",
		"You shake $S hand.",
		"$n shakes $N's hand.",
		"$n shakes your hand.",
		"You are shaken by yourself.",
		"$n shakes and quivers like a bowl full of jelly."
	}, {
		"shiver",
		"Brrrrrrrrr.",
		"$n shivers uncomfortably.",
		"You shiver at the thought of fighting $M.",
		"$n shivers at the thought of fighting $N.",
		"$n shivers at the suicidal thought of fighting you.",
		"You shiver to yourself?",
		"$n scares $mself to shivers."
	}, {
		"shrug",
		"You shrug.",
		"$n shrugs helplessly.",
		"You shrug in response to $s question.",
		"$n shrugs in response to $N's question.",
		"$n shrugs in respopnse to your question.",
		"You shrug to yourself.",
		"$n shrugs to $mself.  What a strange person."
	}, {
		"sigh",
		"You sigh.",
		"$n sighs loudly.",
		"You sigh as you think of $M.",
		"$n sighs at the sight of $N.",
		"$n sighs as $e thinks of you.  Touching, huh?",
		"You sigh at yourself.  You MUST be lonely.",
		"$n sighs at $mself.  What a sorry sight."
	}, {
		"sing",
		"You raise your clear voice towards the sky.",
		"$n has begun to sing.",
		"You sing a ballad to $m.",
		"$n sings a ballad to $N.",
		"$n sings a ballad to you!  How sweet!",
		"You sing a little ditty to yourself.",
		"$n sings a little ditty to $mself."
	}, {
		"smile",
		"You smile happily.",
		"$n smiles happily.",
		"You smile at $M.",
		"$n beams a smile at $N.",
		"$n smiles at you.",
		"You smile at yourself.",
		"$n smiles at $mself."
	}, {
		"smirk",
		"You smirk.",
		"$n smirks.",
		"You smirk at $S saying.",
		"$n smirks at $N's saying.",
		"$n smirks at your saying.",
		"You smirk at yourself.  Okay ...",
		"$n smirks at $s own 'wisdom'."
	}, {
		"snap",
		"PRONTO ! You snap your fingers.",
		"$n snaps $s fingers.",
		"You snap back at $M.",
		"$n snaps back at $N.",
		"$n snaps back at you!",
		"You snap yourself to attention.",
		"$n snaps $mself to attention."
	}, {
		"snarl",
		"You grizzle your teeth and look mean.",
		"$n snarls angrily.",
		"You snarl at $M.",
		"$n snarls at $N.",
		"$n snarls at you, for some reason.",
		"You snarl at yourself.",
		"$n snarls at $mself."
	}, {
		"sneeze",
		"Gesundheit!",
		"$n sneezes.",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	}, {
		"snicker",
		"You snicker softly.",
		"$n snickers softly.",
		"You snicker with $M about your shared secret.",
		"$n snickers with $N about their shared secret.",
		"$n snickers with you about your shared secret.",
		"You snicker at your own evil thoughts.",
		"$n snickers at $s own evil thoughts."
	}, {
		"sniff",
		"You sniff sadly. *SNIFF*",
		"$n sniffs sadly.",
		"You sniff sadly at the way $E is treating you.",
		"$n sniffs sadly at the way $N is treating $m.",
		"$n sniffs sadly at the way you are treating $m.",
		"You sniff sadly at your lost opportunities.",
		"$n sniffs sadly at $mself.  Something MUST be bothering $m."
	}, {
		"snore",
		"Zzzzzzzzzzzzzzzzz.",
		"$n snores loudly.",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	}, {
		"snowball",
		"Whom do you want to throw a snowball at?",
		NULL,
		"You throw a snowball in $N's face.",
		"$n throws a snowball at $N.",
		"$n throws a snowball at you.",
		"You throw a snowball at yourself.",
		"$n throws a snowball at $mself."
	}, {
		"snuggle",
		"Who?",
		NULL,
		"you snuggle $M.",
		"$n snuggles up to $N.",
		"$n snuggles up to you.",
		"You snuggle up, getting ready to sleep.",
		"$n snuggles up, getting ready to sleep."
	}, {
		"spank",
		"Spank whom?",
		NULL,
		"You spank $M playfully.",
		"$n spanks $N playfully.",
		"$n spanks you playfully.  OUCH!",
		"You spank yourself.  Kinky!",
		"$n spanks $mself.  Kinky!"
	}, {
		"squeeze",
		"Where, what, how, whom?",
		NULL,
		"You squeeze $M fondly.",
		"$n squeezes $N fondly.",
		"$n squeezes you fondly.",
		"You squeeze yourself - try to relax a little!",
		"$n squeezes $mself."
	}, {
		"stare",
		"You stare at the sky.",
		"$n stares at the sky.",
		"You stare dreamily at $N, completely lost in $S eyes..",
		"$n stares dreamily at $N.",
		"$n stares dreamily at you, completely lost in your eyes.",
		"You stare dreamily at yourself - enough narcissism for now.",
		"$n stares dreamily at $mself - NARCISSIST!"
	}, {
		"strut",
		"Strut your stuff.",
		"$n struts proudly.",
		"You strut to get $S attention.",
		"$n struts, hoping to get $N's attention.",
		"$n struts, hoping to get your attention.",
		"You strut to yourself, lost in your own world.",
		"$n struts to $mself, lost in $s own world."
	}, {
		"sulk",
		"You sulk.",
		"$n sulks in the corner.",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	}, {
		"thank",
		"Thank you too.",
		NULL,
		"You thank $N heartily.",
		"$n thanks $N heartily.",
		"$n thanks you heartily.",
		"You thank yourself since nobody else wants to !",
		"$n thanks $mself since you won't."
	},	{
		"tickle",
		"Whom do you want to tickle?",
		NULL,
		"You tickle $N.",
		"$n tickles $N.",
		"$n tickles you - hee hee hee.",
		"You tickle yourself, how funny!",
		"$n tickles $mself."
	},	{
		"twiddle",
		"You patiently twiddle your thumbs.",
		"$n patiently twiddles $s thumbs.",
		"You twiddle $S ears.",
		"$n twiddles $N's ears.",
		"$n twiddles your ears.",
		"You twiddle your ears like Dumbo.",
		"$n twiddles $s own ears like Dumbo."
	}, {
		"wave",
		"You wave.",
		"$n waves happily.",
		"You wave goodbye to $N.",
		"$n waves goodbye to $N.",
		"$n waves goodbye to you.  Have a good journey.",
		"Are you going on adventures as well?",
		"$n waves goodbye to $mself."
	}, {
		"whistle",
		"You whistle appreciatively.",
		"$n whistles appreciatively.",
		"You whistle at the sight of $M.",
		"$n whistles at the sight of $N.",
		"$n whistles at the sight of you.",
		"You whistle a little tune to yourself.",
		"$n whistles a little tune to $mself."
	}, {
		"wiggle",
		"Your wiggle your bottom.",
		"$n wiggles $s bottom.",
		"You wiggle your bottom toward $M.",
		"$n wiggles $s bottom toward $N.",
		"$n wiggles $s bottom toward you.",
		"You wiggle about like a fish.",
		"$n wiggles about like a fish."
	}, 	{
		"wince",
		"You wince.  Ouch!",
		"$n winces.  Ouch!",
		"You wince at $M.",
		"$n winces at $N.",
		"$n winces at you.",
		"You wince at yourself.  Ouch!",
		"$n winces at $mself.  Ouch!"
	}, {
		"wink",
		"You wink suggestively.",
		"$n winks suggestively.",
		"You wink suggestively at $N.",
		"$n winks at $N.",
		"$n winks suggestively at you.",
		"You wink at yourself ?? - what are you up to ?",
		"$n winks at $mself - something strange is going on..."
	}, {
		"yawn",
		"You must be tired.",
		"$n yawns.",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	}, {
		"",
		NULL, NULL, NULL, NULL, NULL, NULL, NULL
	}
};



/*
* The main entry point for executing commands.
* Can be recursively called from 'at', 'order', 'force'.
*/
void interpret( CHAR_DATA *ch, char *argument )
{
/*  ROOMTEXT_DATA *rt;
    char kavirarg[MAX_INPUT_LENGTH]; */
    char arg[MAX_INPUT_LENGTH];
    char argu[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    char logline[MAX_INPUT_LENGTH];
    int cmd;
    int trust;
    bool found;

    snprintf(argu, MAX_INPUT_LENGTH, "%s %s",arg,one_argument( argument, arg, MAX_INPUT_LENGTH));

    /*
    * Strip leading spaces.
    */
    while ( isspace(*argument) )
	   argument++;
    if ( argument[0] == '\0' )
	   return;

	   /*
	   * No hiding.
	   REMOVE_BIT( ch->affected_by, AFF_HIDE );
    */

    /*
    * Implement freeze command.
    */
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE) )
    {
	   send_to_char( "You're totally frozen!\n\r", ch );
	   return;
    }

    /*
    * Grab the command word.
    * Special parsing so ' can be a command,
    *   also no spaces needed after punctuation.
    */
    strncpy( logline, argument, MAX_INPUT_LENGTH );
    if ( !isalpha(argument[0]) && !isdigit(argument[0]) )
    {
	   command[0] = argument[0];
	   command[1] = '\0';
	   argument++;
	   while ( isspace(*argument) )
		  argument++;
    }
    else
    {
	   argument = one_argument( argument, command, MAX_INPUT_LENGTH );
    }

/* Nasty LAG code here - Palmer */
   if(ch->lagpenalty > 0)
   {
	WAIT_STATE( ch, ch->lagpenalty);
   }





    /*
    * Look for command in command table.
    */
    found = FALSE;
    trust = get_trust( ch );
    for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ )
    {
	   if ( command[0] == cmd_table[cmd].name[0]
		  &&   !str_prefix( command, cmd_table[cmd].name )
		  &&   cmd_table[cmd].level <= trust )
	   {
		  if (IS_HEAD(ch,LOST_HEAD) || IS_EXTRA(ch,EXTRA_OSWITCH) )
		  {
			 if      (!str_cmp( cmd_table[cmd].name, "say"     )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "'"       )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "immtalk" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, ":"       )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "chat"    )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "."       )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "look"    )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "save"    )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "exits"   )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "emote"   )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "tell"    )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "order"   )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "who"     )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "weather" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "where"   )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "relevel" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "safe"    )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "scan"    )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "spy"     )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "score"   )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "save"    )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "inventory" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "oreturn" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "quit" ) &&
				!IS_NPC(ch) && ch->pcdata->obj_vnum != 0) found = TRUE;
			 else {
				send_to_char( "Not without a body!\n\r", ch ); return; }
		  }
		  else if (IS_BODY(ch,TIED_UP) )
		  {
			 if      (!str_cmp( cmd_table[cmd].name, "say"  )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "'"    )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "chat" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "."    )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "yell" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "bid" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "justitalk" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "shout")) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "look" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "save" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "exits")) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "inventory" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "tell" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "order" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "who" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "weather" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "where" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "introduce" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "relevel" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "safe" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "scan" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "spy"  )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "sleep" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "wake" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "fangs" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "claws" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "nightsight" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "shadowsight" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "shadowplane" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "regenerate" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "shield" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "vclan" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "upkeep" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "score" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "immune" )) found = TRUE;
else if (!str_cmp( cmd_table[cmd].name, "struggle" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "report" )) found = TRUE;
			 else if (!str_cmp( cmd_table[cmd].name, "goto" )) found = TRUE;
			 else {
				send_to_char( "Not while tied up.\n\r", ch );
				if (ch->position > POS_STUNNED)
				    act("$n strains against $s bonds.",ch,NULL,NULL,TO_ROOM);
				return; }
		  }
		  found = TRUE;
		  break;
	   }
    }

    /*
    * Log and snoop.
    */

    /* Only log it if it ISNT log never */
    if ( cmd_table[cmd].log != LOG_NEVER ) {
        if ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
	       ||   fLogAll
	       ||   cmd_table[cmd].log == LOG_ALWAYS )
        {
	       snprintf( log_buf,  MAX_INPUT_LENGTH*2, "%s: %s", ch->name, logline );
	       log_string( log_buf );
        }
    }

    if ( ch->desc != NULL && ch->desc->snoop_by != NULL )
    {
	   write_to_buffer( ch->desc->snoop_by, "% ",    2, 0 );
	   write_to_buffer( ch->desc->snoop_by, logline, 0, 0 );
	   write_to_buffer( ch->desc->snoop_by, "\n\r",  2, 0 );
    }

    if ( !found )
    {
    
	   if ( !check_social( ch, command, argument ) )
		  send_to_char( "Huh?\n\r", ch );
	   return;
    }

    /*
    * Character not in position for command?
    */
    if ( ch->position < cmd_table[cmd].position )
    {
	   switch( ch->position )
	   {
	   case POS_DEAD:
		  send_to_char( "Lie still; you are DEAD.\n\r", ch );
		  break;

	   case POS_MORTAL:
	   case POS_INCAP:
		  send_to_char( "You are hurt far too bad for that.\n\r", ch );
		  break;

	   case POS_STUNNED:
		  send_to_char( "You are too stunned to do that.\n\r", ch );
		  break;

	   case POS_SLEEPING:
		  send_to_char( "In your dreams, or what?\n\r", ch );
		  break;

	   case POS_RESTING:
		  send_to_char( "Nah... You feel too relaxed...\n\r", ch);
		  break;

	   case POS_FIGHTING:
		  send_to_char( "No way!  You are still fighting!\n\r", ch);
		  break;

	   }
	   return;
    }

    /*
    * Dispatch the command.
    */

    (*cmd_table[cmd].do_fun) ( ch, argument );
    tail_chain( );
    /*
    if (!str_cmp(arg,"say"))
    {
    sprintf(kavirarg,"%s",argu);
    room_text( ch, strlower(kavirarg) );
    }
    else
    {
    sprintf(kavirarg,">>>%s",argu);
    room_text( ch, kavirarg );
    }
    */
    return;
}



bool check_social( CHAR_DATA *ch, char *command, char *argument )
{
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    int cmd;
    bool found;

    found  = FALSE;
    for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ )
    {
	   if ( command[0] == social_table[cmd].name[0]
		  &&   !str_prefix( command, social_table[cmd].name ) )
	   {
		  found = TRUE;
		  break;
	   }
    }

    if ( !found )
	   return FALSE;

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_NO_EMOTE) )
    {
	   send_to_char( "You are anti-social!\n\r", ch );
	   return TRUE;
    }
    /*
    if ( IS_HEAD(ch,LOST_HEAD) || IS_EXTRA(ch,EXTRA_OSWITCH))
    {
    send_to_char( "You cannot socialise in this form!\n\r", ch );
    return TRUE;
    }
    */
    switch ( ch->position )
    {
    case POS_DEAD:
	   send_to_char( "Lie still; you are DEAD.\n\r", ch );
	   return TRUE;

    case POS_INCAP:
    case POS_MORTAL:
	   send_to_char( "You are hurt far too bad for that.\n\r", ch );
	   return TRUE;

    case POS_STUNNED:
	   send_to_char( "You are too stunned to do that.\n\r", ch );
	   return TRUE;

    case POS_SLEEPING:
    /*
    * I just know this is the path to a 12" 'if' statement.  :(
    * But two players asked for it already!  -- Furey
	   */
	   if ( !str_cmp( social_table[cmd].name, "snore" ) )
		  break;
	   send_to_char( "In your dreams, or what?\n\r", ch );
	   return TRUE;

    }

    one_argument( argument, arg, MAX_INPUT_LENGTH );
    victim = NULL;

    if ( arg[0] == '\0' )
    {
	   act( social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM    );
	   act( social_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR    );
    }
    else if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
	   send_to_char( "They aren't here.\n\r", ch );
    }
    else if ( victim == ch )
    {
	   act( social_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM    );
	   act( social_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR    );
    }
    else
    {
	   act( social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT );
	   act( social_table[cmd].char_found,    ch, NULL, victim, TO_CHAR    );
	   act( social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT    );

	   if ( !IS_NPC(ch) && IS_NPC(victim)
		  &&   !IS_AFFECTED(victim, AFF_CHARM)
		  &&   IS_AWAKE(victim) )
	   {
		  switch ( number_bits( 4 ) )
		  {
		  case 0:
			 multi_hit( victim, ch, TYPE_UNDEFINED );
			 break;

		  case 1: case 2: case 3: case 4:
		  case 5: case 6: case 7: case 8:
			 act( social_table[cmd].others_found,
				victim, NULL, ch, TO_NOTVICT );
			 act( social_table[cmd].char_found,
				victim, NULL, ch, TO_CHAR    );
			 act( social_table[cmd].vict_found,
				victim, NULL, ch, TO_VICT    );
			 break;

		  case 9: case 10: case 11: case 12:
			 act( "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
			 act( "You slap $N.",  victim, NULL, ch, TO_CHAR    );
			 act( "$n slaps you.", victim, NULL, ch, TO_VICT    );
			 break;
		  }
	   }
    }
    return TRUE;
}

/*
* Return true if an argument is completely numeric.
*/
bool is_number( char *arg )
{
    if ( *arg == '\0' )
	   return FALSE;

    for ( ; *arg != '\0'; arg++ )
    {
	   if ( !isdigit(*arg) )
		  return FALSE;
    }

    return TRUE;
}

/*
* Given a string like 14.foo, return 14 and 'foo'
*/
int number_argument( char *argument, char *arg, int arglen )
{
    char *pdot;
    int number;

    for ( pdot = argument; *pdot != '\0'; pdot++ )
    {
	   if ( *pdot == '.' )
	   {
		  *pdot = '\0';
		  number = atoi( argument );
		  *pdot = '.';
		  strncpy( arg, pdot+1, arglen );
		  return number;
	   }
    }

    strncpy( arg, argument, arglen );
    return 1;
}

/*
* Pick off one argument from a string and return the rest.
* Understands quotes.
*/
char *one_argument( char *argument, char *arg_first, int arglen )
{
    char cEnd;

    while ( isspace(*argument) )
	   argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
	   cEnd = *argument++;

    while ( *argument != '\0' && arglen > 1)
    {
	   if ( *argument == cEnd )
	   {
		  argument++;
		  break;
	   }
	   *arg_first = LOWER(*argument);
	   --arglen;
	   ++arg_first;
	   ++argument;
    }
    *arg_first = '\0';

    while ( isspace(*argument) )
	   argument++;

    return argument;
}