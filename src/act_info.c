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
#define _XOPEN_SOURCE

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
#include <unistd.h>

char *const where_name[] =
	{
		"[Light         ] ",
		"[On Finger     ] ",
		"[On Finger     ] ",
		"[Around Neck   ] ",
		"[Around Neck   ] ",
		"[On Body       ] ",
		"[On Head       ] ",
		"[On Legs       ] ",
		"[On Feet       ] ",
		"[On Hands      ] ",
		"[On Arms       ] ",
		"[Off Hand      ] ",
		"[Around Body   ] ",
		"[Around Waist  ] ",
		"[Around Wrist  ] ",
		"[Around Wrist  ] ",
		"[Right Hand    ] ",
		"[Left Hand     ] ",
		"[On Face       ] ",
		"[Left Scabbard ] ",
		"[Right Scabbard] ",
};

/*
* Local functions.
*/
char *format_obj_to_char args((OBJ_DATA * obj, CHAR_DATA *ch,
							   bool fShort));
void show_char_to_char_0 args((CHAR_DATA * victim, CHAR_DATA *ch));
void show_char_to_char_1 args((CHAR_DATA * victim, CHAR_DATA *ch));
void show_char_to_char args((CHAR_DATA * list, CHAR_DATA *ch));
bool check_blind args((CHAR_DATA * ch));

void evil_eye args((CHAR_DATA * victim, CHAR_DATA *ch));
void check_left_arm args((CHAR_DATA * ch, CHAR_DATA *victim));
void check_right_arm args((CHAR_DATA * ch, CHAR_DATA *victim));
void check_left_leg args((CHAR_DATA * ch, CHAR_DATA *victim));
void check_right_leg args((CHAR_DATA * ch, CHAR_DATA *victim));

void obj_score args((CHAR_DATA * ch, OBJ_DATA *obj));

char *format_obj_to_char(OBJ_DATA *obj, CHAR_DATA *ch, bool fShort)
{
	static char buf[MAX_STRING_LENGTH];

	buf[0] = '\0';
	if (IS_HERO(ch) && obj->points > 0)
		strncat(buf, "(Legendary) ", MAX_STRING_LENGTH - strlen(buf));
	if (IS_OBJ_STAT(obj, ITEM_INVIS))
		strncat(buf, "#w(#yInvis#w)#e ", MAX_STRING_LENGTH - strlen(buf));
	if (IS_AFFECTED(ch, AFF_DETECT_EVIL) && !IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_OBJ_STAT(obj, ITEM_ANTI_EVIL))
		strncat(buf, "#w(#BBlue Aura#w)#e ", MAX_STRING_LENGTH - strlen(buf));
	else if (IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && !IS_OBJ_STAT(obj, ITEM_ANTI_EVIL))
		strncat(buf, "#w(#RRed Aura#w)#e ", MAX_STRING_LENGTH - strlen(buf));
	else if (IS_AFFECTED(ch, AFF_DETECT_EVIL) && IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && !IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_OBJ_STAT(obj, ITEM_ANTI_EVIL))
		strncat(buf, "(#cYellow Aura#e) ", MAX_STRING_LENGTH - strlen(buf));
	if (IS_AFFECTED(ch, AFF_DETECT_MAGIC) && IS_OBJ_STAT(obj, ITEM_MAGIC))
		strncat(buf, "#w(#MMagical#w)#e ", MAX_STRING_LENGTH - strlen(buf));
	if (IS_OBJ_STAT(obj, ITEM_GLOW))
		strncat(buf, "#w(#cGlowing#w) ", MAX_STRING_LENGTH - strlen(buf));
	if (IS_OBJ_STAT(obj, ITEM_HUM))
		strncat(buf, "#w(#CHumming#w) ", MAX_STRING_LENGTH - strlen(buf));
	if (IS_OBJ_STAT(obj, ITEM_SHADOWPLANE) &&
		obj->in_room != NULL &&
		!IS_AFFECTED(ch, AFF_SHADOWPLANE))
		strncat(buf, "#b(#yShadowplane#e#b)#e ", MAX_STRING_LENGTH - strlen(buf));
	if (!IS_OBJ_STAT(obj, ITEM_SHADOWPLANE) &&
		obj->in_room != NULL &&
		IS_AFFECTED(ch, AFF_SHADOWPLANE))
		strncat(buf, "(Normal plane) ", MAX_STRING_LENGTH - strlen(buf));

	if (fShort)
	{
		if (obj->short_descr != NULL)
			strncat(buf, obj->short_descr, MAX_STRING_LENGTH - strlen(buf));
	}
	else
	{
		if (obj->description != NULL)
			strncat(buf, obj->description, MAX_STRING_LENGTH - strlen(buf));
	}

	return buf;
}

/*
* Show a list to a character.
* Can coalesce duplicated items.
*/
void show_list_to_char(OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing)
{
	char buf[MAX_STRING_LENGTH];
	char **prgpstrShow;
	int *prgnShow;
	char *pstrShow;
	OBJ_DATA *obj;
	int nShow;
	int iShow;
	int count;
	bool fCombine;

	if (ch->desc == NULL)
		return;

	/*
	   * Alloc space for output lines.
    */
	count = 0;
	for (obj = list; obj != NULL; obj = obj->next_content)
		count++;
	prgpstrShow = alloc_mem(count * sizeof(char *));
	prgnShow = alloc_mem(count * sizeof(int));
	nShow = 0;

	/*
    * Format the list of objects.
    */
	for (obj = list; obj != NULL; obj = obj->next_content)
	{
		if (!IS_NPC(ch) && ch->pcdata->chobj != NULL && obj->chobj != NULL && obj->chobj == ch)
			continue;
		if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj))
		{
			pstrShow = format_obj_to_char(obj, ch, fShort);
			fCombine = FALSE;

			if (IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE))
			{
				/*
		  * Look for duplicates, case sensitive.
		  * Matches tend to be near end so run loop backwords.
			 */
				for (iShow = nShow - 1; iShow >= 0; iShow--)
				{
					if (!strcmp(prgpstrShow[iShow], pstrShow))
					{
						prgnShow[iShow]++;
						fCombine = TRUE;
						break;
					}
				}
			}

			/*
		  * Couldn't combine, or didn't want to.
		  */
			if (!fCombine)
			{
				prgpstrShow[nShow] = str_dup(pstrShow);
				prgnShow[nShow] = 1;
				nShow++;
			}
		}
	}

	/*
    * Output the formatted list.
    */
	for (iShow = 0; iShow < nShow; iShow++)
	{
		if (IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE))
		{
			if (prgnShow[iShow] != 1)
			{
				snprintf(buf, MAX_STRING_LENGTH, "(%2d) ", prgnShow[iShow]);
				send_to_char(buf, ch);
			}
			else
			{
				send_to_char("     ", ch);
			}
		}
		send_to_char(prgpstrShow[iShow], ch);
		send_to_char("\n\r", ch);
		free_string(prgpstrShow[iShow]);
	}

	if (fShowNothing && nShow == 0)
	{
		if (IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE))
			send_to_char("     ", ch);
		send_to_char("Nothing.\n\r", ch);
	}

	/*
    * Clean up.
    */
	free_mem(prgpstrShow, count * sizeof(char *));
	free_mem(prgnShow, count * sizeof(int));

	return;
}

void show_drop_to_char(OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing)
{
	char buf[MAX_STRING_LENGTH];
	char **prgpstrShow;
	int *prgnShow;
	char *pstrShow;
	OBJ_DATA *obj;
	int nShow;
	int iShow;
	int count;
	bool fCombine;

	if (ch->desc == NULL)
		return;

	/*
	   * Alloc space for output lines.
    */
	count = 0;
	for (obj = list; obj != NULL; obj = obj->next_content)
		count++;
	prgpstrShow = alloc_mem(count * sizeof(char *));
	prgnShow = alloc_mem(count * sizeof(int));
	nShow = 0;

	/*
    * Format the list of objects.
    */
	for (obj = list; obj != NULL; obj = obj->next_content)
	{
		if (!IS_NPC(ch) && ch->pcdata->chobj != NULL && obj->chobj != NULL && obj->chobj == ch)
			continue;
		if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj))
		{
			pstrShow = format_obj_to_char(obj, ch, fShort);
			fCombine = FALSE;

			if (IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE))
			{
				/*
		  * Look for duplicates, case sensitive.
		  * Matches tend to be near end so run loop backwords.
			 */
				for (iShow = nShow - 1; iShow >= 0; iShow--)
				{
					if (!strcmp(prgpstrShow[iShow], pstrShow))
					{
						prgnShow[iShow]++;
						fCombine = TRUE;
						break;
					}
				}
			}

			/*
		  * Couldn't combine, or didn't want to.
		  */
			if (!fCombine)
			{
				prgpstrShow[nShow] = str_dup(pstrShow);
				prgnShow[nShow] = 1;
				nShow++;
			}
		}
	}

	/*
    * Output the formatted list.
    */
	for (iShow = 0; iShow < nShow; iShow++)
	{
		if (IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE))
		{
			if (prgnShow[iShow] != 1)
			{
				snprintf(buf, MAX_STRING_LENGTH, "(%2d) ", prgnShow[iShow]);
				send_to_char(buf, ch);
			}
			else
			{
				send_to_char("     ", ch);
			}
		}
		send_to_char(prgpstrShow[iShow], ch);
		send_to_char("\n\r", ch);
		free_string(prgpstrShow[iShow]);
	}

	if (fShowNothing && nShow == 0)
	{
		if (IS_NPC(ch) || IS_SET(ch->act, PLR_COMBINE))
			send_to_char("     ", ch);
		send_to_char("Nothing.\n\r", ch);
	}

	/*
    * Clean up.
    */
	free_mem(prgpstrShow, count * sizeof(char *));
	free_mem(prgnShow, count * sizeof(int));

	return;
}

void show_char_to_char_0(CHAR_DATA *victim, CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	char mount[MAX_STRING_LENGTH];

	buf[0] = '\0';
	buf2[0] = '\0';
	buf3[0] = '\0';

	if (!IS_NPC(victim) && victim->pcdata->chobj != NULL)
		return;

	if (victim->mount != NULL && IS_SET(victim->mounted, IS_MOUNT))
		return;

	if (IS_HEAD(victim, LOST_HEAD) && IS_AFFECTED(victim, AFF_POLYMORPH))
		strncat(buf, "     ", MAX_STRING_LENGTH - strlen(buf));
	else
	{
		// TODO: Affects array should have their friendly name, so we only ever have to color shit once
		if (IS_AFFECTED(victim, AFF_INVISIBLE))
			strncat(buf, "#w(#yInvis#w)#e ", MAX_STRING_LENGTH - strlen(buf));
		if (IS_AFFECTED(victim, AFF_HIDE))
			strncat(buf, "#w(#gHide#w)#e ", MAX_STRING_LENGTH - strlen(buf));
		if (IS_AFFECTED(victim, AFF_CHARM))
			strncat(buf, "#R(#MC#mh#Ma#mr#Mm#me#Md#R)#e ", MAX_STRING_LENGTH - strlen(buf));
		if (IS_AFFECTED(victim, AFF_PASS_DOOR) ||
			IS_AFFECTED(victim, AFF_ETHEREAL))
			strncat(buf, "#w(#yTranslucent#w)#e ", MAX_STRING_LENGTH - strlen(buf));
		if (IS_AFFECTED(victim, AFF_FAERIE_FIRE))
			strncat(buf, "#m(#MPink Aura#m)#e ", MAX_STRING_LENGTH - strlen(buf));
		if (IS_EVIL(victim) && IS_AFFECTED(ch, AFF_DETECT_EVIL))
			strncat(buf, "#R(#rRed Aura#R)#e ", MAX_STRING_LENGTH - strlen(buf));
		if (IS_AFFECTED(victim, AFF_SANCTUARY))
			strncat(buf, "#W(#wWhite Aura#W)#e ", MAX_STRING_LENGTH - strlen(buf));
	}
	if (IS_AFFECTED(ch, AFF_SHADOWPLANE) && !IS_AFFECTED(victim, AFF_SHADOWPLANE))
		strncat(buf, "(Normal plane) ", MAX_STRING_LENGTH - strlen(buf));
	else if (!IS_AFFECTED(ch, AFF_SHADOWPLANE) && IS_AFFECTED(victim, AFF_SHADOWPLANE))
		strncat(buf, "#b(#yShadowplane#e#b)#e ", MAX_STRING_LENGTH - strlen(buf));
	/* Vampires can recognise each other - KaVir */
	if (IS_SET(victim->act, PLR_VAMPIRE) && IS_SET(ch->act, PLR_VAMPIRE))
	{
		if(DiscIsActive(GetPlayerDiscByTier(victim, ANIMALISM, ANIMALISM_SUBSUME_THE_SPIRIT)))
		{
			strncat(buf, "(Wolf) ", MAX_STRING_LENGTH - strlen(buf));
		}
		else
		{
			strncat(buf, "(Vampire) ", MAX_STRING_LENGTH - strlen(buf));
		}
	}

	if (!IS_NPC(ch) && IS_VAMPAFF(ch, VAM_AUSPEX) &&
		!IS_NPC(victim) && IS_VAMPAFF(victim, VAM_DISGUISED))
	{
		strncat(buf, "(", MAX_STRING_LENGTH - strlen(buf));
		strncat(buf, victim->name, MAX_STRING_LENGTH - strlen(buf));
		strncat(buf, ") ", MAX_STRING_LENGTH - strlen(buf));
	}

	if (IS_AFFECTED(victim, AFF_FLAMING))
	{
		if (IS_NPC(victim))
			snprintf(buf2, MAX_STRING_LENGTH, "\n\r...%s is engulfed in blazing flames!", victim->short_descr);
		else if (!IS_NPC(victim) && IS_AFFECTED(victim, AFF_POLYMORPH))
			snprintf(buf2, MAX_STRING_LENGTH, "\n\r...%s is engulfed in blazing flames!", victim->morph);
		else
			snprintf(buf2, MAX_STRING_LENGTH, "\n\r...%s is engulfed in blazing flames!", victim->name);
	}

	if (!IS_NPC(victim) && IS_HEAD(victim, LOST_HEAD) && IS_AFFECTED(victim, AFF_POLYMORPH))
	{
		if (IS_BODY(victim, GAGGED) && IS_BODY(victim, BLINDFOLDED))
			snprintf(buf3, MAX_STRING_LENGTH, "...%s is gagged and blindfolded!", victim->morph);
		else if (IS_BODY(victim, GAGGED))
			snprintf(buf3, MAX_STRING_LENGTH, "...%s is gagged!", victim->morph);
		else if (IS_BODY(victim, BLINDFOLDED))
			snprintf(buf3, MAX_STRING_LENGTH, "...%s is blindfolded!", victim->morph);
	}

	if (IS_HEAD(victim, LOST_HEAD) && IS_AFFECTED(victim, AFF_POLYMORPH))
	{
		strncat(buf, victim->morph, MAX_STRING_LENGTH - strlen(buf));
		strncat(buf, " is lying here.", MAX_STRING_LENGTH - strlen(buf));
		strncat(buf, buf2, MAX_STRING_LENGTH - strlen(buf));
		strncat(buf, buf3, MAX_STRING_LENGTH - strlen(buf));
		strncat(buf, "\n\r", MAX_STRING_LENGTH - strlen(buf));
		buf[5] = UPPER(buf[5]);
		send_to_char(buf, ch);
		return;
	}

	if (IS_BODY(victim, TIED_UP))
	{
		if (IS_NPC(victim))
			snprintf(buf3, MAX_STRING_LENGTH, "\n\r...%s is tied up", victim->short_descr);
		else if (!IS_NPC(victim) && IS_AFFECTED(victim, AFF_POLYMORPH))
			snprintf(buf3, MAX_STRING_LENGTH, "\n\r...%s is tied up", victim->morph);
		else
			snprintf(buf3, MAX_STRING_LENGTH, "\n\r...%s is tied up", victim->name);
		if (IS_BODY(victim, GAGGED) && IS_BODY(victim, BLINDFOLDED))
			strncat(buf3, ", gagged and blindfolded!", MAX_STRING_LENGTH - strlen(buf3));
		else if (IS_BODY(victim, GAGGED))
			strncat(buf3, " and gagged!", MAX_STRING_LENGTH - strlen(buf3));
		else if (IS_BODY(victim, BLINDFOLDED))
			strncat(buf3, " and blindfolded!", MAX_STRING_LENGTH - strlen(buf3));
		else
			strncat(buf3, "!", MAX_STRING_LENGTH - strlen(buf3));
	}

	if (!IS_NPC(victim) && IS_AFFECTED(victim, AFF_POLYMORPH))
		strncat(buf, victim->morph, MAX_STRING_LENGTH - strlen(buf));
	else if (victim->position == POS_STANDING && victim->long_descr[0] != '\0' && victim->mount == NULL)
	{
		strncat(buf, victim->long_descr, MAX_STRING_LENGTH - strlen(buf));
		send_to_char(buf, ch);
		if (IS_ITEMAFF(victim, ITEMA_SHOCKSHIELD))
			act("...$N is surrounded by a crackling shield of #ll#wi#lg#wh#lt#wn#li#wn#lg#e.", ch, NULL, victim, TO_CHAR);
		if (IS_ITEMAFF(victim, ITEMA_FIRESHIELD))
			act("...$N is surrounded by a burning shield of #rf#Ri#rr#Re#e.", ch, NULL, victim, TO_CHAR);
		if (IS_ITEMAFF(victim, ITEMA_ICESHIELD))
			act("...$N is surrounded by a shimmering shield of #ci#Cc#ce#e.", ch, NULL, victim, TO_CHAR);
		if (IS_ITEMAFF(victim, ITEMA_ACIDSHIELD))
			act("...$N is surrounded by a bubbling shield of #ga#Gc#gi#Gd#e.", ch, NULL, victim, TO_CHAR);
		return;
	}
	else
		strncat(buf, PERS(victim, ch), MAX_STRING_LENGTH - strlen(buf));

	if (victim->mount != NULL && victim->mounted == IS_RIDING)
	{
		if (IS_NPC(victim->mount))
			snprintf(mount, MAX_STRING_LENGTH, " is here riding %s", victim->mount->short_descr);
		else
			snprintf(mount, MAX_STRING_LENGTH, " is here riding %s", victim->mount->name);
		strncat(buf, mount, MAX_STRING_LENGTH - strlen(buf));
		if (victim->position == POS_FIGHTING)
		{
			strncat(buf, ", fighting ", MAX_STRING_LENGTH - strlen(buf));
			if (victim->fighting == NULL)
				strncat(buf, "thin air??", MAX_STRING_LENGTH - strlen(buf));
			else if (victim->fighting == ch)
				strncat(buf, "YOU!", MAX_STRING_LENGTH - strlen(buf));
			else if (victim->in_room == victim->fighting->in_room)
			{
				strncat(buf, PERS(victim->fighting, ch), MAX_STRING_LENGTH - strlen(buf));
				strncat(buf, ".", MAX_STRING_LENGTH - strlen(buf));
			}
			else
				strncat(buf, "somone who left??", MAX_STRING_LENGTH - strlen(buf));
		}
		else
			strncat(buf, ".", MAX_STRING_LENGTH - strlen(buf));
	}
	else if (victim->position == POS_STANDING && IS_AFFECTED(victim, AFF_FLYING))
		strncat(buf, " is hovering here", MAX_STRING_LENGTH - strlen(buf));
	else if (victim->position == POS_STANDING && (!IS_NPC(victim) && (IS_VAMPAFF(victim, VAM_FLYING))))
		strncat(buf, " is hovering here", MAX_STRING_LENGTH - strlen(buf));
	else
	{
		switch (victim->position)
		{
		case POS_DEAD:
			strncat(buf, " is DEAD!!", MAX_STRING_LENGTH - strlen(buf));
			break;
		case POS_MORTAL:
			strncat(buf, " is mortally wounded.", MAX_STRING_LENGTH - strlen(buf));
			break;
		case POS_INCAP:
			strncat(buf, " is incapacitated.", MAX_STRING_LENGTH - strlen(buf));
			break;
		case POS_STUNNED:
			strncat(buf, " is lying here stunned.", MAX_STRING_LENGTH - strlen(buf));
			break;
		case POS_SLEEPING:
			strncat(buf, " is sleeping here.", MAX_STRING_LENGTH - strlen(buf));
			break;
		case POS_RESTING:
			strncat(buf, " is resting here.", MAX_STRING_LENGTH - strlen(buf));
			break;
		case POS_STANDING:
			if (!IS_NPC(victim) && victim->stance[CURRENT_STANCE] == STANCE_NORMAL)
				strncat(buf, " is here, crouched in a fighting stance.", MAX_STRING_LENGTH - strlen(buf));
			else if (!IS_NPC(victim) && victim->stance[CURRENT_STANCE] == STANCE_VIPER)
				strncat(buf, " is here, crouched in a viper fighting stance.", MAX_STRING_LENGTH - strlen(buf));
			else if (!IS_NPC(victim) && victim->stance[CURRENT_STANCE] == STANCE_CRANE)
				strncat(buf, " is here, crouched in a crane fighting stance.", MAX_STRING_LENGTH - strlen(buf));
			else if (!IS_NPC(victim) && victim->stance[CURRENT_STANCE] == STANCE_FALCON)
				strncat(buf, " is here, crouched in an advanced fighting stance.", MAX_STRING_LENGTH - strlen(buf));
			else if (!IS_NPC(victim) && victim->stance[CURRENT_STANCE] == STANCE_MONGOOSE)
				strncat(buf, " is here, crouched in a mongoose fighting stance.", MAX_STRING_LENGTH - strlen(buf));
			else if (!IS_NPC(victim) && victim->stance[CURRENT_STANCE] == STANCE_BULL)
				strncat(buf, " is here, crouched in a bull fighting stance.", MAX_STRING_LENGTH - strlen(buf));
			else if (!IS_NPC(victim) && victim->stance[CURRENT_STANCE] == STANCE_SWALLOW)
				strncat(buf, " is here, crouched in an advanced fighting stance.", MAX_STRING_LENGTH - strlen(buf));
			else if (!IS_NPC(victim) && victim->stance[CURRENT_STANCE] == STANCE_COBRA)
				strncat(buf, " is here, crouched in an advanced fighting stance.", MAX_STRING_LENGTH - strlen(buf));
			else if (!IS_NPC(victim) && victim->stance[CURRENT_STANCE] == STANCE_LION)
				strncat(buf, " is here, crouched in an advanced fighting stance.", MAX_STRING_LENGTH - strlen(buf));
			else if (!IS_NPC(victim) && victim->stance[CURRENT_STANCE] == STANCE_GRIZZLIE)
				strncat(buf, " is here, crouched in an advanced fighting stance.", MAX_STRING_LENGTH - strlen(buf));
			else if (!IS_NPC(victim) && victim->stance[CURRENT_STANCE] == STANCE_PANTHER)
				strncat(buf, " is here, crouched in an advanced fighting stance.", MAX_STRING_LENGTH - strlen(buf));
			else
				strncat(buf, " is here.", MAX_STRING_LENGTH - strlen(buf));
			break;
		case POS_FIGHTING:
			strncat(buf, " is here, fighting ", MAX_STRING_LENGTH - strlen(buf));
			if (victim->fighting == NULL)
				strncat(buf, "thin air??", MAX_STRING_LENGTH - strlen(buf));
			else if (victim->fighting == ch)
				strncat(buf, "YOU!", MAX_STRING_LENGTH - strlen(buf));
			else if (victim->in_room == victim->fighting->in_room)
			{
				strncat(buf, PERS(victim->fighting, ch), MAX_STRING_LENGTH - strlen(buf));
				strncat(buf, ".", MAX_STRING_LENGTH - strlen(buf));
			}
			else
				strncat(buf, "somone who left??", MAX_STRING_LENGTH - strlen(buf));
			break;
		}
	}

	strncat(buf, buf2, MAX_STRING_LENGTH - strlen(buf));
	strncat(buf, buf3, MAX_STRING_LENGTH - strlen(buf));
	strncat(buf, "\n\r", MAX_STRING_LENGTH - strlen(buf));
	buf[0] = UPPER(buf[0]);
	send_to_char(buf, ch);

	if (IS_ITEMAFF(victim, ITEMA_SHOCKSHIELD))
		act("...$N is surrounded by a crackling shield of #ll#wi#lg#wh#lt#wn#li#wn#lg#e.", ch, NULL, victim, TO_CHAR);
	if (IS_ITEMAFF(victim, ITEMA_FIRESHIELD))
		act("...$N is surrounded by a burning shield of #rf#Ri#rr#Re#e.", ch, NULL, victim, TO_CHAR);
	if (IS_ITEMAFF(victim, ITEMA_ICESHIELD))
		act("...$N is surrounded by a shimmering shield of #ci#Cc#ce#e.", ch, NULL, victim, TO_CHAR);
	if (IS_ITEMAFF(victim, ITEMA_ACIDSHIELD))
		act("...$N is surrounded by a bubbling shield of #ga#Gc#gi#Gd#e.", ch, NULL, victim, TO_CHAR);
	return;
}

void evil_eye(CHAR_DATA *victim, CHAR_DATA *ch)
{
	char buf[MAX_INPUT_LENGTH];
	int tabletype;
	if (IS_NPC(victim))
		return;
	if (victim == ch)
		return;
	if (ch->level != 3 || victim->level != 3)
		return;
	if (victim->powertype != NULL)
	{
		snprintf(buf, MAX_INPUT_LENGTH, "\n\r%s\n\r", victim->powertype);
		send_to_char(buf, ch);
	}
	if (IS_SET(victim->spectype, EYE_SELFACTION) && victim->poweraction != NULL)
		interpret(victim, victim->poweraction);
	if (IS_SET(victim->spectype, EYE_ACTION) && victim->poweraction != NULL)
		interpret(ch, victim->poweraction);
	if (IS_SET(victim->spectype, EYE_SPELL) && victim->specpower > 0)
	{
		tabletype = skill_table[victim->specpower].target;
		(*skill_table[victim->specpower].spell_fun)(victim->specpower, victim->spl[tabletype], victim, ch);
	}
	return;
}

void show_char_to_char_1(CHAR_DATA *victim, CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	int iWear;
	int percent;
	bool found;

	if (can_see(victim, ch))
	{
		act("$n looks at you.", ch, NULL, victim, TO_VICT);
		act("$n looks at $N.", ch, NULL, victim, TO_NOTVICT);
	}

	if (!IS_NPC(ch) && IS_HEAD(victim, LOST_HEAD))
	{
		act("$N is lying here.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (victim->description[0] != '\0')
	{
		send_to_char(victim->description, ch);
	}
	else
	{
		act("You see nothing special about $M.", ch, NULL, victim, TO_CHAR);
	}

	if (victim->max_hit > 0)
		percent = (100 * victim->hit) / victim->max_hit;
	else
		percent = -1;

	strncpy(buf, PERS(victim, ch), MAX_STRING_LENGTH);

	if (percent >= 100)
		strncat(buf, " is in perfect health.\n\r", MAX_STRING_LENGTH - strlen(buf));
	else if (percent >= 90)
		strncat(buf, " is slightly scratched.\n\r", MAX_STRING_LENGTH - strlen(buf));
	else if (percent >= 80)
		strncat(buf, " has a few bruises.\n\r", MAX_STRING_LENGTH - strlen(buf));
	else if (percent >= 70)
		strncat(buf, " has some cuts.\n\r", MAX_STRING_LENGTH - strlen(buf));
	else if (percent >= 60)
		strncat(buf, " has several wounds.\n\r", MAX_STRING_LENGTH - strlen(buf));
	else if (percent >= 50)
		strncat(buf, " has many nasty wounds.\n\r", MAX_STRING_LENGTH - strlen(buf));
	else if (percent >= 40)
		strncat(buf, " is bleeding freely.\n\r", MAX_STRING_LENGTH - strlen(buf));
	else if (percent >= 30)
		strncat(buf, " is covered in blood.\n\r", MAX_STRING_LENGTH - strlen(buf));
	else if (percent >= 20)
		strncat(buf, " is leaking guts.\n\r", MAX_STRING_LENGTH - strlen(buf));
	else if (percent >= 10)
		strncat(buf, " is almost dead.\n\r", MAX_STRING_LENGTH - strlen(buf));
	else
		strncat(buf, " is DYING.\n\r", MAX_STRING_LENGTH - strlen(buf));

	buf[0] = UPPER(buf[0]);
	send_to_char(buf, ch);

	if (IS_AFFECTED(victim, AFF_INFRARED) || IS_VAMPAFF(victim, VAM_NIGHTSIGHT))
		act("$N's eyes are glowing bright red.", ch, NULL, victim, TO_CHAR);
	if (IS_AFFECTED(victim, AFF_FLYING))
		act("$N is hovering in the air.", ch, NULL, victim, TO_CHAR);
	if (IS_VAMPAFF(victim, VAM_FANGS))
		act("$N has a pair of long, pointed fangs.", ch, NULL, victim, TO_CHAR);

	found = FALSE;
	for (iWear = 0; iWear < MAX_WEAR; iWear++)
	{
		if ((obj = get_eq_char(victim, iWear)) != NULL && can_see_obj(ch, obj))
		{
			if (!found)
			{
				send_to_char("\n\r", ch);
				act("$N is using:", ch, NULL, victim, TO_CHAR);
				found = TRUE;
			}
			send_to_char(where_name[iWear], ch);
			if (IS_NPC(ch) || ch->pcdata->chobj == NULL || ch->pcdata->chobj != obj)
			{
				send_to_char(format_obj_to_char(obj, ch, TRUE), ch);
				send_to_char("\n\r", ch);
			}
			else
				send_to_char("you\n\r", ch);
		}
	}

	if (victim != ch && !IS_NPC(ch) && !IS_HEAD(victim, LOST_HEAD) && number_percent() < ch->pcdata->learned[gsn_peek])
	{
		send_to_char("\n\rYou peek at the inventory:\n\r", ch);
		show_list_to_char(victim->carrying, ch, TRUE, TRUE);
	}

	return;
}

void show_char_to_char(CHAR_DATA *list, CHAR_DATA *ch)
{
	CHAR_DATA *rch;

	for (rch = list; rch != NULL; rch = rch->next_in_room)
	{
		if (rch == ch)
			continue;

		if (!IS_NPC(rch) && IS_SET(rch->act, PLR_WIZINVIS) && get_trust(ch) < get_trust(rch))
			continue;

		if (!IS_NPC(rch) && IS_HEAD(rch, LOST_HEAD))
			continue;

		if (!IS_NPC(rch) && IS_EXTRA(rch, EXTRA_OSWITCH))
			continue;

		if (can_see(ch, rch))
			show_char_to_char_0(rch, ch);
		else if (room_is_dark(ch->in_room) && (IS_AFFECTED(rch, AFF_INFRARED) || IS_VAMPAFF(rch, VAM_NIGHTSIGHT)))
		{
			send_to_char("You see glowing red eyes watching YOU!\n\r", ch);
		}
	}

	return;
}

bool check_blind(CHAR_DATA *ch)
{
	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT))
		return TRUE;

    CLANDISC_DATA * disc;
    if( !IS_NPC(ch) && (disc = GetPlayerDiscByTier(ch, AUSPEX, AUSPEX_HEIGHTENED_SENSES)) != NULL) // PCs that have King of the Mountain active cannot attack
    {
        if(DiscIsActive(disc) && !str_cmp( disc->option, "Sight"))
        {
            return TRUE;
        }
    }

	if (IS_HEAD(ch, LOST_EYE_L) && IS_HEAD(ch, LOST_EYE_R))
	{
		send_to_char("You have no eyes to see with!\n\r", ch);
		return FALSE;
	}

	if (IS_BODY(ch, BLINDFOLDED))
	{
		send_to_char("You can't see a thing through the blindfold!\n\r", ch);
		return FALSE;
	}

	if (IS_AFFECTED(ch, AFF_BLIND))
	{
		send_to_char("You can't see a thing!\n\r", ch);
		return FALSE;
	}

	return TRUE;
}

void do_look(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	EXIT_DATA *pexit;
	CHAR_DATA *victim;
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	OBJ_DATA *obj;
	OBJ_DATA *portal;
	OBJ_DATA *portal_next;
	ROOM_INDEX_DATA *pRoomIndex;
	ROOM_INDEX_DATA *location;
	char *pdesc;
	int door;
	CLANDISC_DATA * disc;

	if (ch->desc == NULL)
		return;

	if (ch->position < POS_SLEEPING)
	{
		send_to_char("You can't see anything but stars!\n\r", ch);
		return;
	}

	if (ch->position == POS_SLEEPING)
	{
		send_to_char("You can't see anything, you're sleeping!\n\r", ch);
		return;
	}

	if (!check_blind(ch))
		return;

	if (!IS_NPC(ch) && !IS_SET(ch->act, PLR_HOLYLIGHT) && !IS_VAMPAFF(ch, VAM_NIGHTSIGHT) && !IS_AFFECTED(ch, AFF_SHADOWPLANE) && !(ch->in_room != NULL && ch->in_room->vnum == ROOM_VNUM_IN_OBJECT && !IS_NPC(ch) && ch->pcdata->chobj != NULL && ch->pcdata->chobj->in_obj != NULL) && room_is_dark(ch->in_room))
	{
        if((disc = GetPlayerDiscByTier(ch, AUSPEX, AUSPEX_HEIGHTENED_SENSES)) != NULL) {
            if(!DiscIsActive(disc) || str_cmp( disc->option, "Sight"))
            {
                send_to_char("It is pitch black ... \n\r", ch);
                show_char_to_char(ch->in_room->people, ch);
                return;
            }
        }
	}

	argument = one_argument(argument, arg1, MAX_INPUT_LENGTH);
	argument = one_argument(argument, arg2, MAX_INPUT_LENGTH);

	if (arg1[0] == '\0' || !str_cmp(arg1, "auto"))
	{
		/* 'look' or 'look auto' */
		if (ch->in_room != NULL && ch->in_room->vnum == ROOM_VNUM_IN_OBJECT && !IS_NPC(ch) && ch->pcdata->chobj != NULL && ch->pcdata->chobj->in_obj != NULL)
			act("$p", ch, ch->pcdata->chobj->in_obj, NULL, TO_CHAR);
		else if (IS_AFFECTED(ch, AFF_SHADOWPLANE))
			send_to_char("The shadow plane\n\r", ch);
		else
		{
			send_to_char_formatted(ch->in_room->name, ch);
			send_to_char_formatted("\n\r", ch);
		}

		if (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT))
			do_exits(ch, "auto");

		if (ch->in_room != NULL && ch->in_room->vnum == ROOM_VNUM_IN_OBJECT && !IS_NPC(ch) && ch->pcdata->chobj != NULL && ch->pcdata->chobj->in_obj != NULL)
		{
			act("You are inside $p.", ch, ch->pcdata->chobj->in_obj, NULL, TO_CHAR);
			show_list_to_char(ch->pcdata->chobj->in_obj->contains, ch, FALSE, FALSE);
		}
		else if ((arg1[0] == '\0' || !str_cmp(arg1, "auto")) && IS_AFFECTED(ch, AFF_SHADOWPLANE))
			send_to_char("You are standing in complete darkness.\n\r", ch);
		else if ((!IS_NPC(ch) && !IS_SET(ch->act, PLR_BRIEF)) &&
				 (arg1[0] == '\0' || !str_cmp(arg1, "auto")))
		{
			send_to_char_formatted(ch->in_room->description, ch);
			if (ch->in_room->blood == 1000)
				snprintf(buf, MAX_STRING_LENGTH, "You notice that the room is completely drenched in blood.\n\r");
			else if (ch->in_room->blood > 750)
				snprintf(buf, MAX_STRING_LENGTH, "You notice that there is a very large amount of blood around the room.\n\r");
			else if (ch->in_room->blood > 500)
				snprintf(buf, MAX_STRING_LENGTH, "You notice that there is a large quantity of blood around the room.\n\r");
			else if (ch->in_room->blood > 250)
				snprintf(buf, MAX_STRING_LENGTH, "You notice a fair amount of blood on the floor.\n\r");
			else if (ch->in_room->blood > 100)
				snprintf(buf, MAX_STRING_LENGTH, "You notice several blood stains on the floor.\n\r");
			else if (ch->in_room->blood > 50)
				snprintf(buf, MAX_STRING_LENGTH, "You notice a few blood stains on the floor.\n\r");
			else if (ch->in_room->blood > 25)
				snprintf(buf, MAX_STRING_LENGTH, "You notice a couple of blood stains on the floor.\n\r");
			else if (ch->in_room->blood > 0)
				snprintf(buf, MAX_STRING_LENGTH, "You notice a few drops of blood on the floor.\n\r");
			else
				snprintf(buf, MAX_STRING_LENGTH, "You notice nothing special in the room.\n\r");
			ADD_COLOUR(ch, buf, LIGHTRED, MAX_STRING_LENGTH);
			if (ch->in_room->blood > 0)
				send_to_char(buf, ch);
		}

		show_list_to_char(ch->in_room->contents, ch, FALSE, FALSE);
		show_char_to_char(ch->in_room->people, ch);
		return;
	}

	if (!str_cmp(arg1, "i") || !str_cmp(arg1, "in"))
	{
		/* 'look in' */
		if (arg2[0] == '\0')
		{
			send_to_char("Look in what?\n\r", ch);
			return;
		}

		if ((obj = get_obj_here(ch, arg2)) == NULL)
		{
			send_to_char("You do not see that here.\n\r", ch);
			return;
		}

		switch (obj->item_type)
		{
		default:
			send_to_char("That is not a container.\n\r", ch);
			break;

		case ITEM_PORTAL:
			pRoomIndex = get_room_index(obj->value[0]);
			location = get_room_index(ch->in_room->vnum);
			if (pRoomIndex == NULL)
			{
				send_to_char("It doesn't seem to lead anywhere.\n\r", ch);
				return;
			}
			if (obj->value[2] == 1 || obj->value[2] == 3)
			{
				send_to_char("It seems to be closed.\n\r", ch);
				return;
			}
			char_from_room(ch);
			char_to_room(ch, pRoomIndex);

			for (portal = ch->in_room->contents; portal != NULL; portal = portal_next)
			{
				portal_next = portal->next_content;
				if ((obj->value[0] == portal->value[3]) && (obj->value[3] == portal->value[0]))
				{
					if (IS_AFFECTED(ch, AFF_SHADOWPLANE) &&
						!IS_SET(portal->extra_flags, ITEM_SHADOWPLANE))
					{
						REMOVE_BIT(ch->affected_by, AFF_SHADOWPLANE);
						do_look(ch, "auto");
						SET_BIT(ch->affected_by, AFF_SHADOWPLANE);
						break;
					}
					else if (!IS_AFFECTED(ch, AFF_SHADOWPLANE) &&
							 IS_SET(portal->extra_flags, ITEM_SHADOWPLANE))
					{
						SET_BIT(ch->affected_by, AFF_SHADOWPLANE);
						do_look(ch, "auto");
						REMOVE_BIT(ch->affected_by, AFF_SHADOWPLANE);
						break;
					}
					else
					{
						do_look(ch, "auto");
						break;
					}
				}
			}
			char_from_room(ch);
			char_to_room(ch, location);
			break;

		case ITEM_DRINK_CON:
			if (obj->value[1] <= 0)
			{
				send_to_char("It is empty.\n\r", ch);
				break;
			}

			if (obj->value[1] < obj->value[0] / 5)
				snprintf(buf, MAX_STRING_LENGTH, "There is a little %s liquid left in it.\n\r", liq_table[obj->value[2]].liq_color);
			else if (obj->value[1] < obj->value[0] / 4)
				snprintf(buf, MAX_STRING_LENGTH, "It contains a small about of %s liquid.\n\r", liq_table[obj->value[2]].liq_color);
			else if (obj->value[1] < obj->value[0] / 3)
				snprintf(buf, MAX_STRING_LENGTH, "It's about a third full of %s liquid.\n\r", liq_table[obj->value[2]].liq_color);
			else if (obj->value[1] < obj->value[0] / 2)
				snprintf(buf, MAX_STRING_LENGTH, "It's about half full of %s liquid.\n\r", liq_table[obj->value[2]].liq_color);
			else if (obj->value[1] < obj->value[0])
				snprintf(buf, MAX_STRING_LENGTH, "It is almost full of %s liquid.\n\r", liq_table[obj->value[2]].liq_color);
			else if (obj->value[1] == obj->value[0])
				snprintf(buf, MAX_STRING_LENGTH, "It's completely full of %s liquid.\n\r", liq_table[obj->value[2]].liq_color);
			else
				snprintf(buf, MAX_STRING_LENGTH, "Somehow it is MORE than full of %s liquid.\n\r", liq_table[obj->value[2]].liq_color);
			send_to_char(buf, ch);
			break;

		case ITEM_CONTAINER:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			if (IS_SET(obj->value[1], CONT_CLOSED))
			{
				send_to_char("It is closed.\n\r", ch);
				break;
			}

			act("$p contains:", ch, obj, NULL, TO_CHAR);
			show_list_to_char(obj->contains, ch, TRUE, TRUE);
			break;
		}
		return;
	}

	if ((victim = get_char_room(ch, arg1)) != NULL)
	{
		show_char_to_char_1(victim, ch);
		evil_eye(victim, ch);
		return;
	}

	for (vch = char_list; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next;
		if (vch->in_room == NULL)
			continue;
		if (vch->in_room == ch->in_room)
		{
			if (!IS_NPC(vch) && !str_cmp(arg1, vch->morph))
			{
				show_char_to_char_1(vch, ch);
				evil_eye(vch, ch);
				return;
			}
			continue;
		}
	}

	if (!IS_NPC(ch) && ch->pcdata->chobj != NULL && ch->pcdata->chobj->in_obj != NULL)
	{
		obj = get_obj_in_obj(ch, arg1);
		if (obj != NULL)
		{
			send_to_char(obj->description, ch);
			send_to_char("\n\r", ch);
			return;
		}
	}

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content)
	{
		if (!IS_NPC(ch) && ch->pcdata->chobj != NULL && obj->chobj != NULL && obj->chobj == ch)
			continue;
		if (can_see_obj(ch, obj))
		{
			pdesc = get_extra_descr(arg1, obj->extra_descr);
			if (pdesc != NULL)
			{
				send_to_char(pdesc, ch);
				return;
			}

			pdesc = get_extra_descr(arg1, obj->pIndexData->extra_descr);
			if (pdesc != NULL)
			{
				send_to_char(pdesc, ch);
				return;
			}
		}

		if (is_name(arg1, obj->name))
		{
			send_to_char(obj->description, ch);
			send_to_char("\n\r", ch);
			return;
		}
	}

	for (obj = ch->in_room->contents; obj != NULL; obj = obj->next_content)
	{
		if (!IS_NPC(ch) && ch->pcdata->chobj != NULL && obj->chobj != NULL && obj->chobj == ch)
			continue;
		if (can_see_obj(ch, obj))
		{
			pdesc = get_extra_descr(arg1, obj->extra_descr);
			if (pdesc != NULL)
			{
				send_to_char(pdesc, ch);
				return;
			}

			pdesc = get_extra_descr(arg1, obj->pIndexData->extra_descr);
			if (pdesc != NULL)
			{
				send_to_char(pdesc, ch);
				return;
			}
		}

		if (is_name(arg1, obj->name))
		{
			send_to_char(obj->description, ch);
			send_to_char("\n\r", ch);
			return;
		}
	}

	pdesc = get_extra_descr(arg1, ch->in_room->extra_descr);
	if (pdesc != NULL)
	{
		send_to_char(pdesc, ch);
		return;
	}

	if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north"))
		door = 0;
	else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east"))
		door = 1;
	else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south"))
		door = 2;
	else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west"))
		door = 3;
	else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up"))
		door = 4;
	else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down"))
		door = 5;
	else
	{
		send_to_char("You do not see that here.\n\r", ch);
		return;
	}

	/* 'look direction' */
	if ((pexit = ch->in_room->exit[door]) == NULL)
	{
		send_to_char("Nothing special there.\n\r", ch);
		return;
	}

	if (pexit->description != NULL && pexit->description[0] != '\0')
		send_to_char(pexit->description, ch);
	else
		send_to_char("Nothing special there.\n\r", ch);

	if (pexit->keyword != NULL && pexit->keyword[0] != '\0' && pexit->keyword[0] != ' ')
	{
		if (IS_SET(pexit->exit_info, EX_CLOSED))
		{
			act("The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
		}
		else if (IS_SET(pexit->exit_info, EX_ISDOOR))
		{
			act("The $d is open.", ch, NULL, pexit->keyword, TO_CHAR);
			if ((pexit = ch->in_room->exit[door]) == NULL)
				return;
			if ((pRoomIndex = pexit->to_room) == NULL)
				return;
			location = get_room_index(ch->in_room->vnum);
			char_from_room(ch);
			char_to_room(ch, pRoomIndex);
			do_look(ch, "auto");
			char_from_room(ch);
			char_to_room(ch, location);
		}
		else
		{
			if ((pexit = ch->in_room->exit[door]) == NULL)
				return;
			if ((pRoomIndex = pexit->to_room) == NULL)
				return;
			location = get_room_index(ch->in_room->vnum);
			char_from_room(ch);
			char_to_room(ch, pRoomIndex);
			do_look(ch, "auto");
			char_from_room(ch);
			char_to_room(ch, location);
		}
	}
	else
	{
		if ((pexit = ch->in_room->exit[door]) == NULL)
			return;
		if ((pRoomIndex = pexit->to_room) == NULL)
			return;
		location = get_room_index(ch->in_room->vnum);
		char_from_room(ch);
		char_to_room(ch, pRoomIndex);
		do_look(ch, "auto");
		char_from_room(ch);
		char_to_room(ch, location);
	}

	return;
}

void do_examine(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (arg[0] == '\0')
	{
		send_to_char("Examine what?\n\r", ch);
		return;
	}

	do_look(ch, arg);

	if ((obj = get_obj_here(ch, arg)) != NULL)
	{
		if (obj->condition >= 100)
			snprintf(buf, MAX_STRING_LENGTH, "You notice that %s is in perfect condition.\n\r", obj->short_descr);
		else if (obj->condition >= 75)
			snprintf(buf, MAX_STRING_LENGTH, "You notice that %s is in good condition.\n\r", obj->short_descr);
		else if (obj->condition >= 50)
			snprintf(buf, MAX_STRING_LENGTH, "You notice that %s is in average condition.\n\r", obj->short_descr);
		else if (obj->condition >= 25)
			snprintf(buf, MAX_STRING_LENGTH, "You notice that %s is in poor condition.\n\r", obj->short_descr);
		else
			snprintf(buf, MAX_STRING_LENGTH, "You notice that %s is in awful condition.\n\r", obj->short_descr);
		send_to_char(buf, ch);
		switch (obj->item_type)
		{
		default:
			break;

		case ITEM_DRINK_CON:
		case ITEM_CONTAINER:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			send_to_char("When you look inside, you see:\n\r", ch);
			snprintf(buf, MAX_STRING_LENGTH, "in %s", arg);
			do_look(ch, buf);
		}
	}

	return;
}

/*
* Thanks to Zrin for auto-exit part.
*/
void do_exits(CHAR_DATA *ch, char *argument)
{
	extern char *const dir_name[];
	char buf[MAX_STRING_LENGTH];
	EXIT_DATA *pexit;
	bool found;
	bool fAuto;
	int door, n;

	buf[0] = '\0';
	fAuto = !str_cmp(argument, "auto");

	if (!check_blind(ch))
		return;

	strncpy(buf, fAuto ? "[Exits:" : "Obvious exits:\n\r", MAX_STRING_LENGTH);

	found = FALSE;
	for (door = 0; door <= 5; door++)
	{
		if ((pexit = ch->in_room->exit[door]) != NULL && pexit->to_room != NULL && !IS_SET(pexit->exit_info, EX_CLOSED))
		{
			found = TRUE;
			if (fAuto)
			{
				strncat(buf, " ", MAX_STRING_LENGTH - strlen(buf));
				strncat(buf, dir_name[door], MAX_STRING_LENGTH - strlen(buf));
			}
			else
			{
				n = strlen(buf);
				snprintf(buf + n, MAX_STRING_LENGTH - n, "%-5s - %s\n\r",
						 capitalize(dir_name[door]),
						 room_is_dark(pexit->to_room)
							 ? "Too dark to tell"
							 : pexit->to_room->name);
			}
		}
	}

	if (!found)
		strncat(buf, fAuto ? " none" : "None.\n\r", MAX_STRING_LENGTH - strlen(buf));

	if (fAuto)
		strncat(buf, "]\n\r", MAX_STRING_LENGTH - strlen(buf));

	send_to_char(buf, ch);
	return;
}

void do_score(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char ss1[MAX_STRING_LENGTH];
	char ss2[MAX_STRING_LENGTH];
	char ss3[MAX_STRING_LENGTH];

	if (!IS_NPC(ch) && (IS_EXTRA(ch, EXTRA_OSWITCH) || IS_HEAD(ch, LOST_HEAD)))
	{
		obj_score(ch, ch->pcdata->chobj);
		return;
	}
	snprintf(buf, MAX_STRING_LENGTH,
			 "You are %s%s, %d years old (%ld hours).\n\r",
			 ch->name,
			 IS_NPC(ch) ? "" : ch->pcdata->title,
			 get_age(ch),
			 ch->played / 3600);
	send_to_char(buf, ch);

	if (!IS_NPC(ch))
	{
		snprintf(buf, MAX_STRING_LENGTH, "This character was created %s\n\r", ch->createtime);
		send_to_char(buf, ch);
	}

	if (get_trust(ch) != ch->level)
	{
		snprintf(buf, MAX_STRING_LENGTH, "You are trusted at level %d.\n\r",
				 get_trust(ch));
		send_to_char(buf, ch);
	}

	snprintf(buf, MAX_STRING_LENGTH,
			 "You have %d/%d hit, %d/%d mana, %d/%d movement, %d primal energy.\n\r",
			 ch->hit, ch->max_hit,
			 ch->mana, ch->max_mana,
			 ch->move, ch->max_move,
			 ch->practice);
	send_to_char(buf, ch);

	snprintf(buf, MAX_STRING_LENGTH,
			 "You are carrying %d/%d items with weight %d/%d kg.\n\r",
			 ch->carry_number, can_carry_n(ch),
			 ch->carry_weight, can_carry_w(ch));
	send_to_char(buf, ch);

	snprintf(buf, MAX_STRING_LENGTH,
			 "Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d.\n\r",
			 get_curr_str(ch),
			 get_curr_int(ch),
			 get_curr_wis(ch),
			 get_curr_dex(ch),
			 get_curr_con(ch));
	send_to_char(buf, ch);

	/* added by Palmer  */
	snprintf(buf, MAX_STRING_LENGTH,
			 "Your beast is at %d.\n\r",
			 ch->beast);
	send_to_char(buf, ch);

	/* finished here */

	snprintf(buf, MAX_STRING_LENGTH,
			 "You have scored %ld exp, and have %ld gold coins in hand,\n\r and %ld in the bank.\n\r",
			 ch->exp, ch->gold, ch->bank);
	send_to_char(buf, ch);

	snprintf(buf, MAX_STRING_LENGTH,
			 "Autoexit: %s.  Autoloot: %s.  Autosac: %s.  Autogold: %s.\n\r",
			 (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT)) ? "yes" : "no",
			 (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOLOOT)) ? "yes" : "no",
			 (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOSAC)) ? "yes" : "no",
			 (!IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOGOLD)) ? "yes" : "no");
	send_to_char(buf, ch);

	snprintf(buf, MAX_STRING_LENGTH, "Wimpy set to %d hit points.\n\r", ch->wimpy);
	send_to_char(buf, ch);

	if (!IS_HERO(ch))
	{
		if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
			send_to_char("You are drunk.\n\r", ch);
		if (!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] == 0)
			send_to_char("You are thirsty.\n\r", ch);
		if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL] == 0)
			send_to_char("You are hungry.\n\r", ch);
	}

	switch (ch->position)
	{
	case POS_DEAD:
		send_to_char("You are DEAD!!\n\r", ch);
		break;
	case POS_MORTAL:
		send_to_char("You are mortally wounded.\n\r", ch);
		break;
	case POS_INCAP:
		send_to_char("You are incapacitated.\n\r", ch);
		break;
	case POS_STUNNED:
		send_to_char("You are stunned.\n\r", ch);
		break;
	case POS_SLEEPING:
		send_to_char("You are sleeping.\n\r", ch);
		break;
	case POS_RESTING:
		send_to_char("You are resting.\n\r", ch);
		break;
	case POS_STANDING:
		send_to_char("You are standing.\n\r", ch);
		break;
	case POS_FIGHTING:
		send_to_char("You are fighting.\n\r", ch);
		break;
	}

	if (ch->level >= 0)
	{
		snprintf(buf, MAX_STRING_LENGTH, "Armor: %d.  ", GET_ARMOR(ch));
		send_to_char(buf, ch);
	}

	send_to_char("You are armored.\n\r", ch);

	// This is all pretty stuff anyhow, but these numbers don't work anymore
	/*
	if (GET_ARMOR(ch) >= 101)
		send_to_char("naked!\n\r", ch);
	else if (GET_ARMOR(ch) >= 80)
		send_to_char("barely clothed.\n\r", ch);
	else if (GET_ARMOR(ch) >= 60)
		send_to_char("wearing clothes.\n\r", ch);
	else if (GET_ARMOR(ch) >= 40)
		send_to_char("slightly armored.\n\r", ch);
	else if (GET_ARMOR(ch) >= 20)
		send_to_char("somewhat armored.\n\r", ch);
	else if (GET_ARMOR(ch) >= 0)
		send_to_char("armored.\n\r", ch);
	else if (GET_ARMOR(ch) >= -50)
		send_to_char("well armored.\n\r", ch);
	else if (GET_ARMOR(ch) >= -100)
		send_to_char("strongly armored.\n\r", ch);
	else if (GET_ARMOR(ch) >= -150)
		send_to_char("heavily armored.\n\r", ch);
	else if (GET_ARMOR(ch) >= -200)
		send_to_char("superbly armored.\n\r", ch);
	else if (GET_ARMOR(ch) >= -249)
		send_to_char("divinely armored.\n\r", ch);
	else
		send_to_char("ultimately armored!\n\r", ch);
	*/

	if (ch->level >= 0)
	{
		snprintf(buf, MAX_STRING_LENGTH, "Hitroll: %d.  Damroll: %d.  ",
				 GET_HITROLL(ch), GET_DAMROLL(ch));
		send_to_char(buf, ch);
	}

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_VAMPIRE))
	{
		snprintf(buf, MAX_STRING_LENGTH, "Blood: %d.\n\r", ch->pcdata->condition[COND_THIRST]);
		send_to_char(buf, ch);
	}
	else if (ch->level >= 0)
		send_to_char("\n\r", ch);

	if (ch->level >= 0)
	{
		snprintf(buf, MAX_STRING_LENGTH, "Alignment: %d.  ", ch->alignment);
		send_to_char(buf, ch);
	}

	send_to_char("You are ", ch);
	if (ch->alignment > 900)
		send_to_char("angelic.\n\r", ch);
	else if (ch->alignment > 700)
		send_to_char("saintly.\n\r", ch);
	else if (ch->alignment > 350)
		send_to_char("good.\n\r", ch);
	else if (ch->alignment > 100)
		send_to_char("kind.\n\r", ch);
	else if (ch->alignment > -100)
		send_to_char("neutral.\n\r", ch);
	else if (ch->alignment > -350)
		send_to_char("mean.\n\r", ch);
	else if (ch->alignment > -700)
		send_to_char("evil.\n\r", ch);
	else if (ch->alignment > -900)
		send_to_char("demonic.\n\r", ch);
	else
		send_to_char("satanic.\n\r", ch);

	if (!IS_NPC(ch) && ch->level >= 0)
	{
		snprintf(buf, MAX_STRING_LENGTH, "Status: %d.  ", ch->race);
		send_to_char(buf, ch);
	}

	if (!IS_NPC(ch))
		send_to_char("You are ", ch);

	if (!IS_NPC(ch) && ch->level == 1)
		send_to_char("a Mortal.\n\r", ch);
	else if (!IS_NPC(ch) && ch->level == 2)
		send_to_char("a Mortal.\n\r", ch);
	else if (!IS_NPC(ch) && ch->level == 4)
		send_to_char("a Justicar.\n\r", ch);
	else if (!IS_NPC(ch) && ch->level == 5)
		send_to_char("an Elder.\n\r", ch);
	else if (!IS_NPC(ch) && ch->level == 6)
		send_to_char("a Seer.\n\r", ch);
	else if (!IS_NPC(ch) && ch->level == 7)
		send_to_char("an Oracle.\n\r", ch);
	else if (!IS_NPC(ch) && ch->level == 8)
		send_to_char("a God.\n\r", ch);
	else if (!IS_NPC(ch) && ch->level == 9)
		send_to_char("the Palmer!\n\r", ch);
	else if (!IS_NPC(ch) && ch->race <= 0)
		send_to_char("an Avatar.\n\r", ch);
	else if (!IS_NPC(ch) && ch->race <= 4)
		send_to_char("an Immortal.\n\r", ch);
	else if (!IS_NPC(ch) && ch->race <= 9)
		send_to_char("a Fighter.\n\r", ch);
	else if (!IS_NPC(ch) && ch->race <= 14)
		send_to_char("a Adventurer.\n\r", ch);
	else if (!IS_NPC(ch) && ch->race <= 19)
		send_to_char("a Champion.\n\r", ch);
	else if (!IS_NPC(ch) && ch->race <= 24)
		send_to_char("a Warrior.\n\r", ch);
	else if (!IS_NPC(ch) && ch->race >= 25)
		send_to_char("a Hero.\n\r", ch);
	else if (!IS_NPC(ch))
		send_to_char("a Bugged Character!\n\r", ch);

	if (!IS_NPC(ch))
	{
		if (ch->pkill == 0)
			snprintf(ss1, MAX_STRING_LENGTH, "no players");
		else if (ch->pkill == 1)
			snprintf(ss1, MAX_STRING_LENGTH, "%d player", ch->pkill);
		else
			snprintf(ss1, MAX_STRING_LENGTH, "%d players", ch->pkill);
		if (ch->pdeath == 0)
			snprintf(ss2, MAX_STRING_LENGTH, "no players");
		else if (ch->pdeath == 1)
			snprintf(ss2, MAX_STRING_LENGTH, "%d player", ch->pdeath);
		else
			snprintf(ss2, MAX_STRING_LENGTH, "%d players", ch->pdeath);
		snprintf(buf, MAX_STRING_LENGTH, "You have killed %s and have been killed by %s.\n\r", ss1, ss2);
		send_to_char(buf, ch);
		if (ch->mkill == 0)
			snprintf(ss1, MAX_STRING_LENGTH, "no mobs");
		else if (ch->mkill == 1)
			snprintf(ss1, MAX_STRING_LENGTH, "%ld mob", ch->mkill);
		else
			snprintf(ss1, MAX_STRING_LENGTH, "%ld mobs", ch->mkill);
		if (ch->mdeath == 0)
			snprintf(ss2, MAX_STRING_LENGTH, "no mobs");
		else if (ch->mdeath == 1)
			snprintf(ss2, MAX_STRING_LENGTH, "%d mob", ch->mdeath);
		else
			snprintf(ss2, MAX_STRING_LENGTH, "%d mobs", ch->mdeath);
		snprintf(buf, MAX_STRING_LENGTH, "You have killed %s and have been killed by %s.\n\r", ss1, ss2);
		send_to_char(buf, ch);
	}

	if (!IS_NPC(ch) && ch->pcdata->quest > 0)
	{
		if (ch->pcdata->quest == 1)
			snprintf(buf, MAX_STRING_LENGTH, "You have a single quest point.\n\r");
		else
			snprintf(buf, MAX_STRING_LENGTH, "You have %d quest points.\n\r", ch->pcdata->quest);
		send_to_char(buf, ch);
	}
	
	snprintf(buf, MAX_STRING_LENGTH, "You have %ld blood points.\n\r\n\r", ch->tierpoints);
	send_to_char(buf, ch);

	send_to_char("|   #w ----------------    ----------------    ----------------\n\r", ch);

	float hpPercent = (float)ch->hit/ch->max_hit;
	float manaPercent = (float)ch->mana/ch->max_mana;
	float movePercent = (float)ch->move/ch->max_move;
	
	snprintf(ss1, MAX_STRING_LENGTH, "%3d", (int)(hpPercent*100));
	snprintf(ss2, MAX_STRING_LENGTH, "%3d", (int)(manaPercent*100));
	snprintf(ss3, MAX_STRING_LENGTH, "%3d", (int)(movePercent*100));

	snprintf(buf, MAX_STRING_LENGTH, "#w|#0");

	for(int i = 0; i <16;i++)
	{
		

		if(i == 6) { strncat(buf, &ss1[0], 1);}
		else if(i == 7) { strncat(buf, &ss1[1], 1);}
		else if(i == 8) { strncat(buf, &ss1[2], 1);}
		else if(i == 9) { strcat(buf, "%");}
		else{ strcat(buf, " "); }

		if(i == ((int)(15*hpPercent)))
		{
			strcat(buf, "#e#w");
		}
	}

	strcat(buf, "#w|");

	strcat(buf, "  #w|#4");

	for(int i = 0; i <16;i++)
	{
		

		if(i == 6) { strncat(buf, &ss2[0], 1);}
		else if(i == 7) { strncat(buf, &ss2[1], 1);}
		else if(i == 8) { strncat(buf, &ss2[2], 1);}
		else if(i == 9) { strcat(buf, "%");}
		else{ strcat(buf, " "); }

		if(i == ((int)(15*manaPercent)))
		{
			strcat(buf, "#e#w");
		}
	}

	strcat(buf, "#w|");

	strcat(buf, "  #w|#2");

	for(int i = 0; i <16;i++)
	{
		

		if(i == 6) { strncat(buf, &ss3[0], 1);}
		else if(i == 7) { strncat(buf, &ss3[1], 1);}
		else if(i == 8) { strncat(buf, &ss3[2], 1);}
		else if(i == 9) { strcat(buf, "%");}
		else{ strcat(buf, " "); }

		if(i == ((int)(15*movePercent)))
		{
			strcat(buf, "#e#w");
		}
	}

	strcat(buf, "#w|#e");

	snprintf(buf2, MAX_STRING_LENGTH, "|   %s\n\r", buf);
	send_to_char(buf2,ch);

	send_to_char("|   #w ----------------    ----------------    ----------------\n\r", ch);
	
	return;
}

char *const day_name[] =
	{
		"the Moon", "the Bull", "Deception", "Thunder", "Freedom",
		"the Great Gods", "the Sun"};

char *const month_name[] =
	{
		"Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
		"the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
		"the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
		"the Long Shadows", "the Ancient Darkness", "the Great Evil"};

void do_time(CHAR_DATA *ch, char *argument)
{
	extern char str_boot_time[];
	char buf[MAX_STRING_LENGTH];
	char *suf;
	int day;

	day = time_info.day + 1;

	if (day > 4 && day < 20)
		suf = "th";
	else if (day % 10 == 1)
		suf = "st";
	else if (day % 10 == 2)
		suf = "nd";
	else if (day % 10 == 3)
		suf = "rd";
	else
		suf = "th";

	snprintf(buf, MAX_STRING_LENGTH,
			 "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n\rVampire Wars started up at %s\rThe system time is %s\r",

			 (time_info.hour % 12 == 0) ? 12 : time_info.hour % 12,
			 time_info.hour >= 12 ? "pm" : "am",
			 day_name[day % 7],
			 day, suf,
			 month_name[time_info.month],
			 str_boot_time,
			 (char *)ctime(&current_time));

	send_to_char(buf, ch);
	return;
}

void do_weather(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];

	static char *const sky_look[4] =
		{
			"cloudless",
			"cloudy",
			"rainy",
			"lit by flashes of lightning"};

	if (!IS_OUTSIDE(ch))
	{
		send_to_char("You can't see the weather indoors.\n\r", ch);
		return;
	}

	snprintf(buf, MAX_STRING_LENGTH, "The sky is %s and %s.\n\r",
			 sky_look[weather_info.sky],
			 weather_info.change >= 0
				 ? "a warm southerly breeze blows"
				 : "a cold northern gust blows");
	send_to_char(buf, ch);
	return;
}

void do_help(CHAR_DATA *ch, char *argument)
{
	char argall[MAX_INPUT_LENGTH];
	char argone[MAX_INPUT_LENGTH];
	HELP_DATA *pHelp;

	if (argument[0] == '\0')
		argument = "summary";

	/*
	   * Tricky argument handling so 'help a b' doesn't match a.
    */
	argall[0] = '\0';
	while (argument[0] != '\0')
	{
		argument = one_argument(argument, argone, MAX_INPUT_LENGTH);
		if (argall[0] != '\0')
			strncat(argall, " ", MAX_INPUT_LENGTH - strlen(argall));
		strncat(argall, argone, MAX_INPUT_LENGTH - strlen(argall));
	}

	for (pHelp = help_first; pHelp != NULL; pHelp = pHelp->next)
	{
		if (pHelp->level > get_trust(ch))
			continue;

		if (is_name(argall, pHelp->keyword))
		{
			if (pHelp->level >= 0 && str_cmp(argall, "imotd"))
			{
				send_to_char_formatted(pHelp->keyword, ch);
				send_to_char_formatted("\n\r", ch);
			}

			/*
		  * Strip leading '.' to allow initial blanks.
		  */
			if (pHelp->text[0] == '.')
				send_to_char_formatted(pHelp->text + 1, ch);
			else
				send_to_char_formatted(pHelp->text, ch);
			return;
		}
	}

	send_to_char_formatted("No help on that word.\n\r", ch);
	return;
}

void do_level(CHAR_DATA *ch, char *argument)
{
	char lev0[MAX_STRING_LENGTH];

	send_to_char_formatted("#G=========================#w[ #CSPELLS #w]#G==========================\n\r\n\r", ch);
	snprintf(lev0, MAX_STRING_LENGTH, "Purple: %3d   Red: %3d   Blue: %3d   Green: %3d   Yellow: %3d\n\r\n\r",ch->spl[SPELL_PURPLE],ch->spl[SPELL_RED],ch->spl[SPELL_BLUE],ch->spl[SPELL_GREEN],ch->spl[SPELL_YELLOW]);
	send_to_char_formatted( lev0, ch );

	send_to_char_formatted("#G=========================#w[ #CWEAPONS #w]#G=========================\n\r\n\r", ch);
	snprintf( lev0, MAX_STRING_LENGTH, "      Hit:   %3d   Slice: %3d   Stab:  %3d   Slash:  %3d    \n\r", ch->wpn[WEAPON_HIT], ch->wpn[WEAPON_SLICE], ch->wpn[WEAPON_STAB], ch->wpn[WEAPON_SLASH] );
	send_to_char_formatted( lev0, ch );
	snprintf( lev0, MAX_STRING_LENGTH, "      Whip:  %3d   Claw:  %3d   Blast: %3d   Pound:  %3d    \n\r", ch->wpn[WEAPON_WHIP], ch->wpn[WEAPON_CLAW], ch->wpn[WEAPON_BLAST], ch->wpn[WEAPON_POUND] );
	send_to_char_formatted( lev0, ch );
	snprintf( lev0, MAX_STRING_LENGTH, "      Crush: %3d   Bite:  %3d   Grep:  %3d   Pierce: %3d    \n\r", ch->wpn[WEAPON_CRUSH], ch->wpn[WEAPON_BITE], ch->wpn[WEAPON_GREP], ch->wpn[WEAPON_PIERCE] );
	send_to_char_formatted( lev0, ch );
	snprintf( lev0, MAX_STRING_LENGTH, "      Suck:  %3d    \n\r\n\r", ch->wpn[WEAPON_SUCK] );
	send_to_char_formatted( lev0, ch );

	send_to_char_formatted("#G======================#w[ #CBASIC STANCES #w]#G======================\n\r\n\r", ch);
	snprintf( lev0, MAX_STRING_LENGTH, "      Bull: %3d   Crane: %3d   Mongoose: %3d  Viper: %3d\n\r\n\r", ch->stance[STANCE_BULL], ch->stance[STANCE_CRANE], ch->stance[STANCE_MONGOOSE], ch->stance[STANCE_VIPER] );
	send_to_char_formatted( lev0, ch );


	send_to_char_formatted("#G====================#w[ #CADVANCED  STANCES #w]#G====================\n\r\n\r", ch);

	snprintf(lev0, MAX_STRING_LENGTH, "      Cobra: %3d    Falcon:  %3d   Grizzlie: %3d\n\r", ch->stance[STANCE_COBRA], ch->stance[STANCE_FALCON], ch->stance[STANCE_GRIZZLIE]);
	send_to_char_formatted (lev0, ch );
	snprintf(lev0, MAX_STRING_LENGTH, "      Lion:  %3d    Panther: %3d   Swallow:  %3d\n\r", ch->stance[STANCE_LION], ch->stance[STANCE_PANTHER], ch->stance[STANCE_SWALLOW]);
	send_to_char_formatted (lev0, ch );

	return;
}

void do_tierlist(CHAR_DATA *ch, char *argument)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char lev0[MAX_STRING_LENGTH];
	int tiercost;

    argument = one_argument(argument, arg1, MAX_INPUT_LENGTH);
    argument = one_argument(argument, arg2, MAX_INPUT_LENGTH);
	
	if( 
		arg1[0] == '\0' || (
		!str_cmp(arg1, "animalism") &&
		!str_cmp(arg1, "auspex") &&
		!str_cmp(arg1, "celerity") &&
		!str_cmp(arg1, "dominate") &&
		!str_cmp(arg1, "fortitude") &&
		!str_cmp(arg1, "obfuscate") &&
		!str_cmp(arg1, "obtenebration") &&
		!str_cmp(arg1, "potence") &&
		!str_cmp(arg1, "presence") &&
		!str_cmp(arg1, "quietus") &&
		!str_cmp(arg1, "thaumaturgy") &&
		!str_cmp(arg1, "vicissitude") 
		)
	 ) {
			
		snprintf( lev0, MAX_STRING_LENGTH, "[%2d] Animalism\n\r", ch->tier_clandisc[CLANDISC_ANIMALISM] );
		send_to_char( lev0, ch );

		snprintf( lev0, MAX_STRING_LENGTH, "[%2d] Auspex\n\r", ch->tier_clandisc[CLANDISC_AUSPEX] );
		send_to_char( lev0, ch );

		snprintf( lev0, MAX_STRING_LENGTH, "[%2d] Celerity\n\r", ch->tier_clandisc[CLANDISC_CELERITY] );
		send_to_char( lev0, ch );

		snprintf( lev0, MAX_STRING_LENGTH, "[%2d] Dominate\n\r", ch->tier_clandisc[CLANDISC_DOMINATE] );
        send_to_char( lev0, ch );

		snprintf( lev0, MAX_STRING_LENGTH, "[%2d] Fortitude\n\r", ch->tier_clandisc[CLANDISC_FORTITUDE] );
		send_to_char( lev0, ch );

		snprintf( lev0, MAX_STRING_LENGTH, "[%2d] Obfuscate\n\r", ch->tier_clandisc[CLANDISC_OBFUSCATE] );
		send_to_char( lev0, ch );

		snprintf( lev0, MAX_STRING_LENGTH, "[%2d] Obtenebration\n\r", ch->tier_clandisc[CLANDISC_OBTENEBRATION] );
		send_to_char( lev0, ch );

		snprintf( lev0, MAX_STRING_LENGTH, "[%2d] Potence\n\r", ch->tier_clandisc[CLANDISC_POTENCE] );
		send_to_char( lev0, ch );

		snprintf( lev0, MAX_STRING_LENGTH, "[%2d] Presence\n\r", ch->tier_clandisc[CLANDISC_PRESENCE] );
		send_to_char( lev0, ch );

		snprintf( lev0, MAX_STRING_LENGTH, "[%2d] Quietus\n\r", ch->tier_clandisc[CLANDISC_QUIETUS] );
		send_to_char( lev0, ch );

		snprintf( lev0, MAX_STRING_LENGTH, "[%2d] Thaumaturgy\n\r", ch->tier_clandisc[CLANDISC_THAUMATURGY] );
		send_to_char( lev0, ch );

		snprintf( lev0, MAX_STRING_LENGTH, "[%2d] Vicissitude\n\r", ch->tier_clandisc[CLANDISC_VICISSITUDE] );
		send_to_char( lev0, ch );
	}
	
	// They submitted a proper clandisc, so now let's make sure that they can actually tier it
	if( !str_cmp(arg1, "animalism")) {
		
		if( ch->vampgen + ch->tier_clandisc[CLANDISC_ANIMALISM] + 1 > 13) {
			send_to_char( "In order to train a higher tier of Animalism, you must lower you generation.\n\r", ch);
			return;
		}
		
		if(ch->tier_clandisc[CLANDISC_ANIMALISM] < 10) {
			tiercost = (ch->tier_clandisc[CLANDISC_ANIMALISM] + 1) * 10000;
			
			if( ch->tierpoints < tiercost ) {
				snprintf( lev0, MAX_STRING_LENGTH, "It costs %d blood points to achieve rank %d of Animalism.\n\r", tiercost, ch->tier_clandisc[CLANDISC_ANIMALISM] );
				send_to_char( lev0, ch );
			} else {
				ch->tierpoints -= tiercost;
				ch->tier_clandisc[CLANDISC_ANIMALISM] += 1;
				snprintf( lev0, MAX_STRING_LENGTH, "You have spent %d blood points to achieve tier %d of Animalism!\n\r", tiercost, ch->tier_clandisc[CLANDISC_ANIMALISM] );
				send_to_char( lev0, ch );
			}
		}
		
		if( ch->tier_clandisc[CLANDISC_ANIMALISM] > 10 ) {
			send_to_char("Cheating, eh? You've been set back to tier 10 of Animalism.", ch);
			ch->tier_clandisc[CLANDISC_ANIMALISM] = 10;
		}
	}
	
	if( !str_cmp(arg1, "auspex")) {
		
		if( ch->vampgen + ch->tier_clandisc[CLANDISC_AUSPEX] + 1 > 13) {
			send_to_char( "In order to train a higher tier of Auspex, you must lower you generation.\n\r", ch);
			return;
		}
		
		if(ch->tier_clandisc[CLANDISC_AUSPEX] < 10) {
			tiercost = (ch->tier_clandisc[CLANDISC_AUSPEX] + 1) * 10000;
			
			if( ch->tierpoints < tiercost ) {
				snprintf( lev0, MAX_STRING_LENGTH, "It costs %d blood points to achieve rank %d of Auspex.\n\r", tiercost, ch->tier_clandisc[CLANDISC_AUSPEX] );
				send_to_char( lev0, ch );
			} else {
				ch->tierpoints -= tiercost;
				ch->tier_clandisc[CLANDISC_AUSPEX] += 1;
				snprintf( lev0, MAX_STRING_LENGTH, "You have spent %d blood points to achieve tier %d of Auspex!\n\r", tiercost, ch->tier_clandisc[CLANDISC_AUSPEX] );
				send_to_char( lev0, ch );
			}
		}
		
		if( ch->tier_clandisc[CLANDISC_AUSPEX] > 10 ) {
			send_to_char("Cheating, eh? You've been set back to tier 10 of Auspex.", ch);
			ch->tier_clandisc[CLANDISC_AUSPEX] = 10;
		}
	}
	
	if( !str_cmp(arg1, "celerity")) {
		
		if( ch->vampgen + ch->tier_clandisc[CLANDISC_CELERITY] + 1 > 13) {
			send_to_char( "In order to train a higher tier of Celerity, you must lower you generation.\n\r", ch);
			return;
		}
		
		if(ch->tier_clandisc[CLANDISC_CELERITY] < 10) {
			tiercost = (ch->tier_clandisc[CLANDISC_CELERITY] + 1) * 10000;
			
			if( ch->tierpoints < tiercost ) {
				snprintf( lev0, MAX_STRING_LENGTH, "It costs %d blood points to achieve rank %d of Celerity.\n\r", tiercost, ch->tier_clandisc[CLANDISC_CELERITY] );
				send_to_char( lev0, ch );
			} else {
				ch->tierpoints -= tiercost;
				ch->tier_clandisc[CLANDISC_CELERITY] += 1;
				snprintf( lev0, MAX_STRING_LENGTH, "You have spent %d blood points to achieve tier %d of Celerity!\n\r", tiercost, ch->tier_clandisc[CLANDISC_CELERITY] );
				send_to_char( lev0, ch );
			}
		}
		
		if( ch->tier_clandisc[CLANDISC_CELERITY] > 10 ) {
			send_to_char("Cheating, eh? You've been set back to tier 10 of Celerity.", ch);
			ch->tier_clandisc[CLANDISC_CELERITY] = 10;
		}
	}

	if( !str_cmp(arg1, "dominate")) {

        if( ch->vampgen + ch->tier_clandisc[CLANDISC_DOMINATE] + 1 > 13) {
            send_to_char( "In order to train a higher tier of Dominate, you must lower you generation.\n\r", ch);
            return;
        }

        if(ch->tier_clandisc[CLANDISC_DOMINATE] < 10) {
            tiercost = (ch->tier_clandisc[CLANDISC_DOMINATE] + 1) * 10000;

            if( ch->tierpoints < tiercost ) {
                snprintf( lev0, MAX_STRING_LENGTH, "It costs %d blood points to achieve rank %d of Dominate.\n\r", tiercost, ch->tier_clandisc[CLANDISC_DOMINATE] );
                send_to_char( lev0, ch );
            } else {
                ch->tierpoints -= tiercost;
                ch->tier_clandisc[CLANDISC_CELERITY] += 1;
                snprintf( lev0, MAX_STRING_LENGTH, "You have spent %d blood points to achieve tier %d of Dominate!\n\r", tiercost, ch->tier_clandisc[CLANDISC_DOMINATE] );
                send_to_char( lev0, ch );
            }
        }

        if( ch->tier_clandisc[CLANDISC_DOMINATE] > 10 ) {
            send_to_char("Cheating, eh? You've been set back to tier 10 of Dominate.", ch);
            ch->tier_clandisc[CLANDISC_DOMINATE] = 10;
        }
    }
	
	if( !str_cmp(arg1, "fortitude")) {
		
		if( ch->vampgen + ch->tier_clandisc[CLANDISC_OBTENEBRATION] + 1 > 13) {
			send_to_char( "In order to train a higher tier of Obtenebration, you must lower you generation.\n\r", ch);
			return;
		}
		
		if(ch->tier_clandisc[CLANDISC_FORTITUDE] < 10) {
			tiercost = (ch->tier_clandisc[CLANDISC_FORTITUDE] + 1) * 10000;
			
			if( ch->tierpoints < tiercost ) {
				snprintf( lev0, MAX_STRING_LENGTH, "It costs %d blood points to achieve rank %d of Fortitude.\n\r", tiercost, ch->tier_clandisc[CLANDISC_FORTITUDE] );
				send_to_char( lev0, ch );
			} else {
				ch->tierpoints -= tiercost;
				ch->tier_clandisc[CLANDISC_FORTITUDE] += 1;
				snprintf( lev0, MAX_STRING_LENGTH, "You have spent %d blood points to achieve tier %d of Fortitude!\n\r", tiercost, ch->tier_clandisc[CLANDISC_FORTITUDE] );
				send_to_char( lev0, ch );
			}
		}
		
		if( ch->tier_clandisc[CLANDISC_FORTITUDE] > 10 ) {
			send_to_char("Cheating, eh? You've been set back to tier 10 of Fortitude.", ch);
			ch->tier_clandisc[CLANDISC_FORTITUDE] = 10;
		}
	}

	if( !str_cmp(arg1, "obtenebration")) {

		if( ch->vampgen + ch->tier_clandisc[CLANDISC_OBTENEBRATION] + 1 > 13) {
			send_to_char( "In order to train a higher tier of Obtenebration, you must lower you generation.\n\r", ch);
			return;
		}

		if(ch->tier_clandisc[CLANDISC_OBTENEBRATION] < 10) {
			tiercost = (ch->tier_clandisc[CLANDISC_OBTENEBRATION] + 1) * 10000;

			if( ch->tierpoints < tiercost ) {
				snprintf( lev0, MAX_STRING_LENGTH, "It costs %d blood points to achieve rank %d of Obtenebration.\n\r", tiercost, ch->tier_clandisc[CLANDISC_OBTENEBRATION] );
				send_to_char( lev0, ch );
			} else {
				ch->tierpoints -= tiercost;
				ch->tier_clandisc[CLANDISC_OBTENEBRATION] += 1;
				snprintf( lev0, MAX_STRING_LENGTH, "You have spent %d blood points to achieve tier %d of Obtenebration!\n\r", tiercost, ch->tier_clandisc[CLANDISC_OBTENEBRATION] );
				send_to_char( lev0, ch );
			}
		}

		if( ch->tier_clandisc[CLANDISC_OBTENEBRATION] > 10 ) {
			send_to_char("Cheating, eh? You've been set back to tier 10 of Obtenebration.", ch);
			ch->tier_clandisc[CLANDISC_OBTENEBRATION] = 10;
		}
	}

	if( !str_cmp(arg1, "obfuscate")) {
		
		if( ch->vampgen + ch->tier_clandisc[CLANDISC_OBFUSCATE] + 1 > 13) {
			send_to_char( "In order to train a higher tier of Obfuscate, you must lower you generation.\n\r", ch);
			return;
		}
		
		if(ch->tier_clandisc[CLANDISC_OBFUSCATE] < 10) {
			tiercost = (ch->tier_clandisc[CLANDISC_OBFUSCATE] + 1) * 10000;
			
			if( ch->tierpoints < tiercost ) {
				snprintf( lev0, MAX_STRING_LENGTH, "It costs %d blood points to achieve rank %d of Obfuscate.\n\r", tiercost, ch->tier_clandisc[CLANDISC_OBFUSCATE] );
				send_to_char( lev0, ch );
			} else {
				ch->tierpoints -= tiercost;
				ch->tier_clandisc[CLANDISC_OBFUSCATE] += 1;
				snprintf( lev0, MAX_STRING_LENGTH, "You have spent %d blood points to achieve tier %d of Obfuscate!\n\r", tiercost, ch->tier_clandisc[CLANDISC_OBFUSCATE] );
				send_to_char( lev0, ch );
			}
		}
		
		if( ch->tier_clandisc[CLANDISC_OBFUSCATE] > 10 ) {
			send_to_char("Cheating, eh? You've been set back to tier 10 of Obfuscate.", ch);
			ch->tier_clandisc[CLANDISC_OBFUSCATE] = 10;
		}
	}
	
	if( !str_cmp(arg1, "potence")) {
		
		// 12 + 1 + 1 = 14 -- FAIL
		if( ch->vampgen + ch->tier_clandisc[CLANDISC_POTENCE] + 1 > 13) {
			send_to_char( "In order to train a higher tier of Potence, you must lower you generation.\n\r", ch);
			return;
		}
		
		if(ch->tier_clandisc[CLANDISC_POTENCE] < 10) {
			tiercost = (ch->tier_clandisc[CLANDISC_POTENCE] + 1) * 10000;
			
			if( ch->tierpoints < tiercost ) {
				snprintf( lev0, MAX_STRING_LENGTH, "It costs %d blood points to achieve rank %d of Potence.\n\r", tiercost, ch->tier_clandisc[CLANDISC_POTENCE] );
				send_to_char( lev0, ch );
			} else {
				ch->tierpoints -= tiercost;
				ch->tier_clandisc[CLANDISC_POTENCE] += 1;
				snprintf( lev0, MAX_STRING_LENGTH, "You have spent %d blood points to achieve tier %d of Potence!\n\r", tiercost, ch->tier_clandisc[CLANDISC_POTENCE] );
				send_to_char( lev0, ch );
			}
		}
		
		if( ch->tier_clandisc[CLANDISC_POTENCE] > 10 ) {
			send_to_char("Cheating, eh? You've been set back to tier 10 of Potence.", ch);
			ch->tier_clandisc[CLANDISC_POTENCE] = 10;
		}
	}
	
	if( !str_cmp(arg1, "presence")) {
		
		if( ch->vampgen + ch->tier_clandisc[CLANDISC_PRESENCE] + 1 > 13) {
			send_to_char( "In order to train a higher tier of Presence, you must lower you generation.\n\r", ch);
			return;
		}
		
		if(ch->tier_clandisc[CLANDISC_PRESENCE] < 10) {
			tiercost = (ch->tier_clandisc[CLANDISC_PRESENCE] + 1) * 10000;
			
			if( ch->tierpoints < tiercost ) {
				snprintf( lev0, MAX_STRING_LENGTH, "It costs %d blood points to achieve rank %d of Presence.\n\r", tiercost, ch->tier_clandisc[CLANDISC_PRESENCE] );
				send_to_char( lev0, ch );
			} else {
				ch->tierpoints -= tiercost;
				ch->tier_clandisc[CLANDISC_PRESENCE] += 1;
				snprintf( lev0, MAX_STRING_LENGTH, "You have spent %d blood points to achieve tier %d of Presence!\n\r", tiercost, ch->tier_clandisc[CLANDISC_PRESENCE] );
				send_to_char( lev0, ch );
			}
		}
		
		if( ch->tier_clandisc[CLANDISC_PRESENCE] > 10 ) {
			send_to_char("Cheating, eh? You've been set back to tier 10 of Presence.", ch);
			ch->tier_clandisc[CLANDISC_PRESENCE] = 10;
		}
	}
	
	if( !str_cmp(arg1, "quietus")) {
		if( ch->vampgen + ch->tier_clandisc[CLANDISC_QUIETUS] + 1 > 13) {
			send_to_char( "In order to train a higher tier of Quietus, you must lower you generation.\n\r", ch);
			return;
		}
		
		if(ch->tier_clandisc[CLANDISC_QUIETUS] < 10) {
			tiercost = (ch->tier_clandisc[CLANDISC_QUIETUS] + 1) * 10000;
			
			if( ch->tierpoints < tiercost ) {
				snprintf( lev0, MAX_STRING_LENGTH, "It costs %d blood points to achieve rank %d of Quietus.\n\r", tiercost, ch->tier_clandisc[CLANDISC_QUIETUS] );
				send_to_char( lev0, ch );
			} else {
				ch->tierpoints -= tiercost;
				ch->tier_clandisc[CLANDISC_QUIETUS] += 1;
				snprintf( lev0, MAX_STRING_LENGTH, "You have spent %d blood points to achieve tier %d of Quietus!\n\r", tiercost, ch->tier_clandisc[CLANDISC_QUIETUS] );
				send_to_char( lev0, ch );
			}
		}
		
		if( ch->tier_clandisc[CLANDISC_QUIETUS] > 10 ) {
			send_to_char("Cheating, eh? You've been set back to tier 10 of Quietus.", ch);
			ch->tier_clandisc[CLANDISC_QUIETUS] = 10;
		}
	}
	
	if( !str_cmp(arg1, "thaumaturgy")) {
		if( ch->vampgen + ch->tier_clandisc[CLANDISC_THAUMATURGY] + 1 > 13) {
			send_to_char( "In order to train a higher tier of Thaumaturgy, you must lower you generation.\n\r", ch);
			return;
		}
		
		if(ch->tier_clandisc[CLANDISC_THAUMATURGY] < 10) {
			tiercost = (ch->tier_clandisc[CLANDISC_THAUMATURGY] + 1) * 10000;
			
			if( ch->tierpoints < tiercost ) {
				snprintf( lev0, MAX_STRING_LENGTH, "It costs %d blood points to achieve rank %d of Thaumaturgy.\n\r", tiercost, ch->tier_clandisc[CLANDISC_THAUMATURGY] );
				send_to_char( lev0, ch );
			} else {
				ch->tierpoints -= tiercost;
				ch->tier_clandisc[CLANDISC_THAUMATURGY] += 1;
				snprintf( lev0, MAX_STRING_LENGTH, "You have spent %d blood points to achieve tier %d of Thaumaturgy!\n\r", tiercost, ch->tier_clandisc[CLANDISC_THAUMATURGY] );
				send_to_char( lev0, ch );
			}
		}
		
		if( ch->tier_clandisc[CLANDISC_THAUMATURGY] > 10 ) {
			send_to_char("Cheating, eh? You've been set back to tier 10 of Thaumaturgy.", ch);
			ch->tier_clandisc[CLANDISC_THAUMATURGY] = 10;
		}
	}
	
	if( !str_cmp(arg1, "vicissitude")) {
		if( ch->vampgen + ch->tier_clandisc[CLANDISC_VICISSITUDE] + 1 > 13) {
			send_to_char( "In order to train a higher tier of Vicissitude, you must lower you generation.\n\r", ch);
			return;
		}
		
		if(ch->tier_clandisc[CLANDISC_VICISSITUDE] < 10) {
			tiercost = (ch->tier_clandisc[CLANDISC_VICISSITUDE] + 1) * 10000;
			
			if( ch->tierpoints < tiercost ) {
				snprintf( lev0, MAX_STRING_LENGTH, "It costs %d blood points to achieve rank %d of Vicissitude.\n\r", tiercost, ch->tier_clandisc[CLANDISC_VICISSITUDE] );
				send_to_char( lev0, ch );
			} else {
				ch->tierpoints -= tiercost;
				ch->tier_clandisc[CLANDISC_VICISSITUDE] += 1;
				snprintf( lev0, MAX_STRING_LENGTH, "You have spent %d blood points to achieve tier %d of Vicissitude!\n\r", tiercost, ch->tier_clandisc[CLANDISC_VICISSITUDE] );
				send_to_char( lev0, ch );
			}
		}
		
		if( ch->tier_clandisc[CLANDISC_VICISSITUDE] > 10 ) {
			send_to_char("Cheating, eh? You've been set back to tier 10 of Vicissitude.", ch);
			ch->tier_clandisc[CLANDISC_VICISSITUDE] = 10;
		}
	}
	
	

    return;
}

/*
* Palmer' version of who
*/
void do_who(CHAR_DATA *ch, char *argument)
{
	int n_vis, n_all;
	int to_see;
	int i;
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];

	const char *valid_args[] =
		{
			"mortal",
			"avatar",
			"immortal",
			"fighter",
			"adventurer",
			"champion",
			"warrior",
			"hero",
			"justicar",
			"god"};
	const int num_args = sizeof(valid_args) / sizeof(char *);

	/* find out what they want to see */
	to_see = 0;
	while (*argument)
	{
		argument = one_argument(argument, arg, MAX_STRING_LENGTH);
		arg[3] = '\0';
		for (i = 0; i < num_args; ++i)
		{
			if (!strncmp(valid_args[i], arg, 3))
			{
				to_see |= (1 << i);
				break;
			}
		}
		if (i == num_args)
		{
			/* invalid arg */
			send_to_char("Invalid argument, use one or more of: \n\r"
						 "mortal, avatar, immortal, fighter, adventurer, \n\r"
						 "champion, warrior, hero, justicar, god\n\r",
						 ch);
			return;
		}
	}
	if (to_see == 0)
	{
		/* no arguments, so show everything */
		to_see = 0xFFFFFFFF;
	}

	n_vis = 0;
	n_all = 0;
	send_to_char_formatted("--------------------------------------------------------------------------------\n\r", ch);
	for (d = descriptor_list; d != NULL; d = d->next)
	{
		CHAR_DATA *wch;
		char *prefix;
		char postfix[MAX_STRING_LENGTH];

		/* check for visibility */
		if (d->connected != CON_PLAYING || (!can_see(ch, d->character) && (!IS_SET(ch->act, PLR_WATCHER))))
		{
			if (d->connected == CON_PLAYING && d->character->level < LEVEL_SEER)
				++n_all;
			continue;
		}

		wch = (d->original != NULL) ? d->original : d->character;

		/* check for orgy membership */
		if (IS_SET(ch->extra, EXTRA_ORGYMEMBER) != IS_SET(wch->extra, EXTRA_ORGYMEMBER))
			continue;

		/* check if we're not interested in this person */
		if ((!IS_SET(to_see, (1 << 0)) && wch->level < 3)										   /* mortal */
			|| (!IS_SET(to_see, (1 << 1)) && wch->level == 3 && wch->race == 0)					   /* avatar */
			|| (!IS_SET(to_see, (1 << 2)) && wch->level == 3 && wch->race >= 1 && wch->race < 5)   /* immortal */
			|| (!IS_SET(to_see, (1 << 3)) && wch->level == 3 && wch->race >= 5 && wch->race < 10)  /* fighter */
			|| (!IS_SET(to_see, (1 << 4)) && wch->level == 3 && wch->race >= 10 && wch->race < 15) /* adventurer */
			|| (!IS_SET(to_see, (1 << 5)) && wch->level == 3 && wch->race >= 15 && wch->race < 20) /* champion */
			|| (!IS_SET(to_see, (1 << 6)) && wch->level == 3 && wch->race >= 20 && wch->race < 25) /* warrior */
			|| (!IS_SET(to_see, (1 << 7)) && wch->level == 3 && wch->race >= 25)				   /* hero */
			|| (!IS_SET(to_see, (1 << 8)) && wch->level == LEVEL_JUSTICAR)						   /* justicar */
			|| (!IS_SET(to_see, (1 << 9)) && wch->level > LEVEL_JUSTICAR)						   /* god */
		)
			continue;

		/* choose what to print in the left hand column */
		prefix = "[ BUGGED CHAR ] ";
		if (wch->pcdata->chobj && wch->pcdata->chobj->pIndexData->vnum == 12)
			prefix = "A Head        ";
		else if (wch->pcdata->chobj && wch->pcdata->chobj->pIndexData->vnum == 30005)
			prefix = "A Brain       ";
		else if (IS_EXTRA(wch, EXTRA_OSWITCH))
			prefix = "An Object     ";
		else if (wch->level < 3)
			prefix = "Mortal        ";
		else if (wch->level == 3)
		{
			if (wch->race < 1)
				prefix = "Avatar        ";
			else if (wch->race < 5)
				prefix = "Immortal      ";
			else if (wch->race < 10)
				prefix = "Fighter       ";
			else if (wch->race < 15)
				prefix = "Adventurer    ";
			else if (wch->race < 20)
				prefix = "Champion      ";
			else if (wch->race < 25)
				prefix = "Warrior       ";
			else
				prefix = "Hero          ";
		}
		else if (!str_cmp(wch->name, "Palmer"))
			prefix = "[ The Owner ] ";
		else if (wch->level == LEVEL_JUSTICAR)
		{
			if (ch->level < 3)
				prefix = "Justicar      ";
			else
			{
				if (0 == str_cmp(wch->clan, "Assamite"))
					prefix = "Assamite        ";
				else if (0 == str_cmp(wch->clan, "Caitiff"))
					prefix = "Sinner        ";
				else if (0 == str_cmp(wch->clan, "Cappadocian"))
					prefix = "Cappadocian   ";
				else if (0 == str_cmp(wch->clan, "Lasombra"))
					prefix = "Lasombra       ";
				else if (0 == str_cmp(wch->clan, "Tzimisce"))
					prefix = "Tzimisce     ";
				else if (0 == str_cmp(wch->clan, "Nosferatu"))
					prefix = "Nosferatu     ";
				else if (0 == str_cmp(wch->clan, "Toreador"))
					prefix = "Toreador      ";
				else if (0 == str_cmp(wch->clan, "Tremere"))
					prefix = "Tremere       ";
				else if (0 == str_cmp(wch->clan, "Ventrue"))
					prefix = "Ventrue       ";
			}
		}

		else if (wch->level > LEVEL_JUSTICAR)
		{
			if (ch->level >= LEVEL_SEER)
			{
				/* admin see this version */
				if (wch->level == LEVEL_SEER)
					prefix = "[ SEER ]      ";
				else if (wch->level == LEVEL_ORACLE)
					prefix = "[ ORACLE ]    ";
				else if (wch->level == LEVEL_GOD)
					prefix = "[ GOD ]       ";
				else if (wch->level >= LEVEL_CODER)
					prefix = "[ CODER ]     ";
			}
			else
			{
				/* players see this */
				if (wch->level >= LEVEL_SEER && wch->level <= LEVEL_CODER)
					prefix = "[ ADMIN ]     ";
			}
		}

		postfix[0] = '\0';
		if (IS_SET(wch->act, PLR_WIZINVIS))
		{
			strncat(postfix, "(WizInvis)", MAX_STRING_LENGTH - strlen(postfix));
		}
		if (str_cmp(wch->name, "Palmer") && IS_SET(wch->extra, EXTRA_ORGYMEMBER) && IS_SET(ch->extra, EXTRA_ORGYMEMBER))
		{
			strncat(postfix, "(o)", MAX_STRING_LENGTH - strlen(postfix));
		}

		++n_vis;
		++n_all;
		snprintf(buf, MAX_STRING_LENGTH, "%s%s%s %s\n\r", prefix, wch->name, wch->pcdata->title, postfix);
		send_to_char_formatted(buf, ch);
	}
	send_to_char_formatted("--------------------------------------------------------------------------------\n\r", ch);
	if (n_vis == 1)
		send_to_char_formatted("You are the only visible player connected!\n\r", ch);
	else
	{
		snprintf(buf, MAX_STRING_LENGTH, "There are %d visible players.\n\r", n_vis);
		send_to_char_formatted(buf, ch);
		snprintf(buf, MAX_STRING_LENGTH, "There are a total of %d players connected.\n\r", n_all);
		send_to_char_formatted(buf, ch);
	}
	send_to_char_formatted("--------------------------------------------------------------------------------\n\r", ch);
}

void do_inventory(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	OBJ_DATA *portal;
	OBJ_DATA *portal_next;
	ROOM_INDEX_DATA *pRoomIndex;
	ROOM_INDEX_DATA *location;

	if (!IS_NPC(ch) && IS_HEAD(ch, LOST_HEAD))
	{
		send_to_char("You are not a container.\n\r", ch);
		return;
	}
	else if (!IS_NPC(ch) && IS_EXTRA(ch, EXTRA_OSWITCH))
	{
		if (!IS_NPC(ch) && (obj = ch->pcdata->chobj) == NULL)
		{
			send_to_char("You are not a container.\n\r", ch);
			return;
		}
		switch (obj->item_type)
		{
		default:
			send_to_char("You are not a container.\n\r", ch);
			break;

		case ITEM_PORTAL:
			pRoomIndex = get_room_index(obj->value[0]);
			location = get_room_index(ch->in_room->vnum);
			if (pRoomIndex == NULL)
			{
				send_to_char("You don't seem to lead anywhere.\n\r", ch);
				return;
			}
			char_from_room(ch);
			char_to_room(ch, pRoomIndex);

			for (portal = ch->in_room->contents; portal != NULL; portal = portal_next)
			{
				portal_next = portal->next_content;
				if ((obj->value[0] == portal->value[3]) && (obj->value[3] == portal->value[0]))
				{
					if (IS_AFFECTED(ch, AFF_SHADOWPLANE) &&
						!IS_SET(portal->extra_flags, ITEM_SHADOWPLANE))
					{
						REMOVE_BIT(ch->affected_by, AFF_SHADOWPLANE);
						do_look(ch, "auto");
						SET_BIT(ch->affected_by, AFF_SHADOWPLANE);
						break;
					}
					else if (!IS_AFFECTED(ch, AFF_SHADOWPLANE) &&
							 IS_SET(portal->extra_flags, ITEM_SHADOWPLANE))
					{
						SET_BIT(ch->affected_by, AFF_SHADOWPLANE);
						do_look(ch, "auto");
						REMOVE_BIT(ch->affected_by, AFF_SHADOWPLANE);
						break;
					}
					else
					{
						do_look(ch, "auto");
						break;
					}
				}
			}
			char_from_room(ch);
			char_to_room(ch, location);
			break;

		case ITEM_DRINK_CON:
			if (obj->value[1] <= 0)
			{
				send_to_char("You are empty.\n\r", ch);
				break;
			}
			if (obj->value[1] < obj->value[0] / 5)
				snprintf(buf, MAX_INPUT_LENGTH, "There is a little %s liquid left in you.\n\r", liq_table[obj->value[2]].liq_color);
			else if (obj->value[1] < obj->value[0] / 4)
				snprintf(buf, MAX_INPUT_LENGTH, "You contain a small about of %s liquid.\n\r", liq_table[obj->value[2]].liq_color);
			else if (obj->value[1] < obj->value[0] / 3)
				snprintf(buf, MAX_INPUT_LENGTH, "You're about a third full of %s liquid.\n\r", liq_table[obj->value[2]].liq_color);
			else if (obj->value[1] < obj->value[0] / 2)
				snprintf(buf, MAX_INPUT_LENGTH, "You're about half full of %s liquid.\n\r", liq_table[obj->value[2]].liq_color);
			else if (obj->value[1] < obj->value[0])
				snprintf(buf, MAX_INPUT_LENGTH, "You are almost full of %s liquid.\n\r", liq_table[obj->value[2]].liq_color);
			else if (obj->value[1] == obj->value[0])
				snprintf(buf, MAX_INPUT_LENGTH, "You're completely full of %s liquid.\n\r", liq_table[obj->value[2]].liq_color);
			else
				snprintf(buf, MAX_INPUT_LENGTH, "Somehow you are MORE than full of %s liquid.\n\r", liq_table[obj->value[2]].liq_color);
			send_to_char(buf, ch);
			break;

		case ITEM_CONTAINER:
		case ITEM_CORPSE_NPC:
		case ITEM_CORPSE_PC:
			act("$p contain:", ch, obj, NULL, TO_CHAR);
			show_list_to_char(obj->contains, ch, TRUE, TRUE);
			break;
		}
		return;
	}
	send_to_char("You are carrying:\n\r", ch);
	show_list_to_char(ch->carrying, ch, TRUE, TRUE);
	return;
}

void do_equipment(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;
	int iWear;
	bool found;

	send_to_char_formatted("You are using:\n\r", ch);
	found = FALSE;
	for (iWear = 0; iWear < MAX_WEAR; iWear++)
	{

		
		if ((obj = get_eq_char(ch, iWear)) == NULL)
		{
			if(iWear == WEAR_LIGHT || iWear == WEAR_SHIELD ) continue;
			send_to_char_formatted(where_name[iWear], ch);
			send_to_char_formatted("Nothing\n\r", ch);
			continue;
		}

		send_to_char_formatted(where_name[iWear], ch);

		if (can_see_obj(ch, obj))
		{
			send_to_char_formatted(format_obj_to_char(obj, ch, TRUE), ch);
			send_to_char_formatted("\n\r", ch);
		}
		else
		{
			send_to_char_formatted("something.\n\r", ch);
		}
		found = TRUE;
	}

	if (!found)
		send_to_char_formatted("Nothing.\n\r", ch);

	return;
}

void do_compare(CHAR_DATA *ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj1;
	OBJ_DATA *obj2;
	int value1;
	int value2;
	char *msg;

	argument = one_argument(argument, arg1, MAX_INPUT_LENGTH);
	argument = one_argument(argument, arg2, MAX_INPUT_LENGTH);
	if (arg1[0] == '\0')
	{
		send_to_char("Compare what to what?\n\r", ch);
		return;
	}

	if ((obj1 = get_obj_carry(ch, arg1)) == NULL)
	{
		send_to_char("You do not have that item.\n\r", ch);
		return;
	}

	if (arg2[0] == '\0')
	{
		for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
		{
			if (obj2->wear_loc != WEAR_NONE && can_see_obj(ch, obj2) && obj1->item_type == obj2->item_type && (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0)
				break;
		}

		if (obj2 == NULL)
		{
			send_to_char("You aren't wearing anything comparable.\n\r", ch);
			return;
		}
	}
	else
	{
		if ((obj2 = get_obj_carry(ch, arg2)) == NULL)
		{
			send_to_char("You do not have that item.\n\r", ch);
			return;
		}
	}

	msg = NULL;
	value1 = 0;
	value2 = 0;

	if (obj1 == obj2)
	{
		msg = "You compare $p to itself.  It looks about the same.";
	}
	else if (obj1->item_type != obj2->item_type)
	{
		msg = "You can't compare $p and $P.";
	}
	else
	{
		switch (obj1->item_type)
		{
		default:
			msg = "You can't compare $p and $P.";
			break;

		case ITEM_ACCESSORY:
		case ITEM_LIGHT_ARMOR:
		case ITEM_MEDIUM_ARMOR:
		case ITEM_HEAVY_ARMOR:
			value1 = obj1->value[0];
			value2 = obj2->value[0];
			break;

		case ITEM_WEAPON:
		case ITEM_WEAPON_15HAND:
		case ITEM_WEAPON_2HAND:
			value1 = obj1->value[1] + obj1->value[2];
			value2 = obj2->value[1] + obj2->value[2];
			break;
		}
	}

	if (msg == NULL)
	{
		if (value1 == value2)
			msg = "$p and $P look about the same.";
		else if (value1 > value2)
			msg = "$p looks better than $P.";
		else
			msg = "$p looks worse than $P.";
	}

	act(msg, ch, obj1, obj2, TO_CHAR);
	return;
}

void do_credits(CHAR_DATA *ch, char *argument)
{
	do_help(ch, "diku");
	return;
}

void do_wizlist(CHAR_DATA *ch, char *argument)
{
	do_help(ch, "wizlist");
	return;
}

void do_where(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;
	bool found;

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (arg[0] == '\0')
	{
		send_to_char_formatted("Players near you:\n\r", ch);
		found = FALSE;
		for (d = descriptor_list; d; d = d->next)
		{
			if (d->connected == CON_PLAYING && (victim = d->character) != NULL && !IS_NPC(victim) && victim->in_room != NULL && victim->in_room->area == ch->in_room->area && victim->pcdata->chobj == NULL && can_see(ch, victim))
			{
				found = TRUE;
				snprintf(buf, MAX_STRING_LENGTH, "%-28s %s\n\r",
						 victim->name, victim->in_room->name);
				send_to_char_formatted(buf, ch);
			}
		}
		if (!found)
			send_to_char_formatted("None\n\r", ch);
	}
	else
	{
		found = FALSE;
		for (victim = char_list; victim != NULL; victim = victim->next)
		{
			if (victim->in_room != NULL && victim->in_room->area == ch->in_room->area && !IS_AFFECTED(victim, AFF_HIDE) && !IS_AFFECTED(victim, AFF_SNEAK) && can_see(ch, victim) && is_name(arg, victim->name))
			{
				found = TRUE;
				snprintf(buf, MAX_STRING_LENGTH, "%-28s %s\n\r",
						 PERS(victim, ch), victim->in_room->name);
				send_to_char(buf, ch);
				break;
			}
		}
		if (!found)
			act("You didn't find any $T.", ch, NULL, arg, TO_CHAR);
	}

	return;
}

void do_consider(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	char *msg;
	int diff;
	int overall;

	one_argument(argument, arg, MAX_INPUT_LENGTH);
	overall = 0;

	if (arg[0] == '\0')
	{
		send_to_char("Consider killing whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They're not here.\n\r", ch);
		return;
	}

	act("You examine $N closely, looking for $S weaknesses.", ch, NULL, victim, TO_CHAR);
	act("$n examine $N closely, looking for $S weaknesses.", ch, NULL, victim, TO_NOTVICT);
	act("$n examines you closely, looking for your weaknesses.", ch, NULL, victim, TO_VICT);

	if (!IS_NPC(victim))
		do_skill(ch, victim->name);

	diff = victim->level - ch->level + victim->hitroll - ch->hitroll;
	if (diff <= -35)
	{
		msg = "You are FAR more skilled than $M.";
		overall = overall + 3;
	}
	else if (diff <= -15)
	{
		msg = "$E is not as skilled as you are.";
		overall = overall + 2;
	}
	else if (diff <= -5)
	{
		msg = "$E doesn't seem quite as skilled as you.";
		overall = overall + 1;
	}
	else if (diff <= 5)
	{
		msg = "You are about as skilled as $M.";
	}
	else if (diff <= 15)
	{
		msg = "$E is slightly more skilled than you are.";
		overall = overall - 1;
	}
	else if (diff <= 35)
	{
		msg = "$E seems more skilled than you are.";
		overall = overall - 2;
	}
	else
	{
		msg = "$E is FAR more skilled than you.";
		overall = overall - 3;
	}
	act(msg, ch, NULL, victim, TO_CHAR);

	diff = victim->level - ch->level + victim->damroll - ch->damroll;
	if (diff <= -35)
	{
		msg = "You are FAR more powerful than $M.";
		overall = overall + 3;
	}
	else if (diff <= -15)
	{
		msg = "$E is not as powerful as you are.";
		overall = overall + 2;
	}
	else if (diff <= -5)
	{
		msg = "$E doesn't seem quite as powerful as you.";
		overall = overall + 1;
	}
	else if (diff <= 5)
	{
		msg = "You are about as powerful as $M.";
	}
	else if (diff <= 15)
	{
		msg = "$E is slightly more powerful than you are.";
		overall = overall - 1;
	}
	else if (diff <= 35)
	{
		msg = "$E seems more powerful than you are.";
		overall = overall - 2;
	}
	else
	{
		msg = "$E is FAR more powerful than you.";
		overall = overall - 3;
	}
	act(msg, ch, NULL, victim, TO_CHAR);

	if (victim->hit <= 0)
	{
		diff = ch->hit * 100 / 1;
	}
	else
	{
		diff = ch->hit * 100 / victim->hit;
	}
	if (diff <= 10)
	{
		msg = "$E is currently FAR healthier than you are.";
		overall = overall - 3;
	}
	else if (diff <= 50)
	{
		msg = "$E is currently much healthier than you are.";
		overall = overall - 2;
	}
	else if (diff <= 75)
	{
		msg = "$E is currently slightly healthier than you are.";
		overall = overall - 1;
	}
	else if (diff <= 125)
	{
		msg = "$E is currently about as healthy as you are.";
	}
	else if (diff <= 200)
	{
		msg = "You are currently slightly healthier than $M.";
		overall = overall + 1;
	}
	else if (diff <= 500)
	{
		msg = "You are currently much healthier than $M.";
		overall = overall + 2;
	}
	else
	{
		msg = "You are currently FAR healthier than $M.";
		overall = overall + 3;
	}
	act(msg, ch, NULL, victim, TO_CHAR);

	// TODO: Fix the verbage on this, the armor is all backwards because of AC previously being negative
	diff = GET_ARMOR(victim) - GET_ARMOR(ch);
	if (diff <= -100)
	{
		msg = "$E is FAR better armoured than you.";
		overall = overall - 3;
	}
	else if (diff <= -50)
	{
		msg = "$E looks much better armoured than you.";
		overall = overall - 2;
	}
	else if (diff <= -25)
	{
		msg = "$E looks better armoured than you.";
		overall = overall - 1;
	}
	else if (diff <= 25)
	{
		msg = "$E seems about as well armoured as you.";
	}
	else if (diff <= 50)
	{
		msg = "You are better armoured than $M.";
		overall = overall + 1;
	}
	else if (diff <= 100)
	{
		msg = "You are much better armoured than $M.";
		overall = overall + 2;
	}
	else
	{
		msg = "You are FAR better armoured than $M.";
		overall = overall + 3;
	}
	act(msg, ch, NULL, victim, TO_CHAR);

	diff = overall;
	if (diff <= -11)
		msg = "Conclusion: $E would kill you in seconds.";
	else if (diff <= -7)
		msg = "Conclusion: You would need a lot of luck to beat $M.";
	else if (diff <= -3)
		msg = "Conclusion: You would need some luck to beat $N.";
	else if (diff <= 2)
		msg = "Conclusion: It would be a very close fight.";
	else if (diff <= 6)
		msg = "Conclusion: You shouldn't have a lot of trouble defeating $M.";
	else if (diff <= 10)
		msg = "Conclusion: $N is no match for you.  You can easily beat $M.";
	else
		msg = "Conclusion: $E wouldn't last more than a few seconds against you.";
	act(msg, ch, NULL, victim, TO_CHAR);

	return;
}

void set_title(CHAR_DATA *ch, char *title)
{
	char buf[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
	{
		bug("Set_title: NPC.", 0);
		return;
	}

	if (isalpha(title[0]) || isdigit(title[0]))
	{
		buf[0] = ' ';
		strncpy(buf + 1, title, MAX_STRING_LENGTH - 1);
	}
	else
	{
		strncpy(buf, title, MAX_STRING_LENGTH);
	}

	free_string(ch->pcdata->title);
	ch->pcdata->title = str_dup(buf);
	return;
}

void do_title(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
		return;

	if (argument[0] == '\0')
	{
		send_to_char("Change your title to what?\n\r", ch);
		return;
	}

	if (strlen(argument) > 50)
		argument[50] = '\0';

	smash_tilde(argument);
	set_title(ch, argument);
	send_to_char("Ok.\n\r", ch);
}

void do_description(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];

	if (argument[0] != '\0')
	{
		buf[0] = '\0';
		smash_tilde(argument);
		if (argument[0] == '+')
		{
			if (ch->description != NULL)
				strncat(buf, ch->description, MAX_STRING_LENGTH - strlen(buf));
			argument++;
			while (isspace(*argument))
				argument++;
		}

		if (strlen(buf) + strlen(argument) >= 1601)
		{
			send_to_char_formatted("Description too long.\n\r", ch);
			return;
		}

		strncat(buf, argument, MAX_STRING_LENGTH - strlen(buf));
		strncat(buf, "\n\r", MAX_STRING_LENGTH - strlen(buf));
		free_string(ch->description);
		ch->description = str_dup(buf);
	}

	send_to_char_formatted("Your description is:\n\r", ch);
	send_to_char_formatted(ch->description ? ch->description : "(None).\n\r", ch);
	return;
}

void do_report(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	char buf[MAX_STRING_LENGTH];
	char hit_str[MAX_INPUT_LENGTH];
	char mana_str[MAX_INPUT_LENGTH];
	char move_str[MAX_INPUT_LENGTH];
	char mhit_str[MAX_INPUT_LENGTH];
	char mmana_str[MAX_INPUT_LENGTH];
	char mmove_str[MAX_INPUT_LENGTH];
	char exp_str[MAX_INPUT_LENGTH];
	snprintf(hit_str, MAX_INPUT_LENGTH, "%d", ch->hit);
	COL_SCALE(hit_str, ch, ch->hit, ch->max_hit, MAX_INPUT_LENGTH);
	snprintf(mana_str, MAX_INPUT_LENGTH, "%d", ch->mana);
	COL_SCALE(mana_str, ch, ch->mana, ch->max_mana, MAX_INPUT_LENGTH);
	snprintf(move_str, MAX_INPUT_LENGTH, "%d", ch->move);
	COL_SCALE(move_str, ch, ch->move, ch->max_move, MAX_INPUT_LENGTH);
	snprintf(exp_str, MAX_INPUT_LENGTH, "%ld", ch->exp);
	COL_SCALE(exp_str, ch, ch->exp, 1000, MAX_INPUT_LENGTH);
	snprintf(mhit_str, MAX_INPUT_LENGTH, "%d", ch->max_hit);
	ADD_COLOUR(ch, mhit_str, LIGHTCYAN, MAX_INPUT_LENGTH);
	snprintf(mmana_str, MAX_INPUT_LENGTH, "%d", ch->max_mana);
	ADD_COLOUR(ch, mmana_str, LIGHTCYAN, MAX_INPUT_LENGTH);
	snprintf(mmove_str, MAX_INPUT_LENGTH, "%d", ch->max_move);
	ADD_COLOUR(ch, mmove_str, LIGHTCYAN, MAX_INPUT_LENGTH);
	snprintf(buf, MAX_STRING_LENGTH,
			 "You report: %s/%s hp %s/%s mana %s/%s mv %s xp.\n\r",
			 hit_str, mhit_str,
			 mana_str, mmana_str,
			 move_str, mmove_str,
			 exp_str);

	send_to_char(buf, ch);

	for (vch = char_list; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next;
		if (vch == NULL)
			continue;
		if (vch == ch)
			continue;
		if (vch->in_room == NULL)
			continue;
		if (vch->in_room != ch->in_room)
			continue;
		snprintf(hit_str, MAX_INPUT_LENGTH, "%d", ch->hit);
		COL_SCALE(hit_str, vch, ch->hit, ch->max_hit, MAX_INPUT_LENGTH);
		snprintf(mana_str, MAX_INPUT_LENGTH, "%d", ch->mana);
		COL_SCALE(mana_str, vch, ch->mana, ch->max_mana, MAX_INPUT_LENGTH);
		snprintf(move_str, MAX_INPUT_LENGTH, "%d", ch->move);
		COL_SCALE(move_str, vch, ch->move, ch->max_move, MAX_INPUT_LENGTH);
		snprintf(exp_str, MAX_INPUT_LENGTH, "%ld", ch->exp);
		COL_SCALE(exp_str, vch, ch->exp, 1000, MAX_INPUT_LENGTH);
		snprintf(mhit_str, MAX_INPUT_LENGTH, "%d", ch->max_hit);
		ADD_COLOUR(vch, mhit_str, LIGHTCYAN, MAX_INPUT_LENGTH);
		snprintf(mmana_str, MAX_INPUT_LENGTH, "%d", ch->max_mana);
		ADD_COLOUR(vch, mmana_str, LIGHTCYAN, MAX_INPUT_LENGTH);
		snprintf(mmove_str, MAX_INPUT_LENGTH, "%d", ch->max_move);
		ADD_COLOUR(vch, mmove_str, LIGHTCYAN, MAX_INPUT_LENGTH);
		if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_POLYMORPH))
			snprintf(buf, MAX_STRING_LENGTH, "%s reports: %s/%s hp %s/%s mana %s/%s mv %s xp.\n\r",
					 ch->morph,
					 hit_str, mhit_str,
					 mana_str, mmana_str,
					 move_str, mmove_str,
					 exp_str);
		else
			snprintf(buf, MAX_STRING_LENGTH, "%s reports: %s/%s hp %s/%s mana %s/%s mv %s xp.\n\r",
					 IS_NPC(ch) ? capitalize(ch->short_descr) : ch->name,
					 hit_str, mhit_str,
					 mana_str, mmana_str,
					 move_str, mmove_str,
					 exp_str);
		buf[0] = UPPER(buf[0]);
		send_to_char(buf, vch);
	}
	return;
}

void do_practice(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	int sn;

	if (IS_NPC(ch))
		return;

	if (argument[0] == '\0')
	{
		int col;

		col = 0;
		for (sn = 0; sn < MAX_SKILL; sn++)
		{
			if (skill_table[sn].name == NULL)
				break;
			if (ch->level < skill_table[sn].skill_level[ch->class])
				continue;
			snprintf(buf, MAX_STRING_LENGTH, "%18s %3d%%  ",
					 skill_table[sn].name, ch->pcdata->learned[sn]);
			send_to_char_formatted(buf, ch);
			if (++col % 3 == 0)
				send_to_char_formatted("\n\r", ch);
		}

		if (col % 3 != 0)
			send_to_char("\n\r", ch);

		snprintf(buf, MAX_STRING_LENGTH, "You have %ld exp left.\n\r", ch->exp);
		send_to_char_formatted(buf, ch);
	}
	else
	{
		int adept;
		int rep_count;
		char arg[MAX_INPUT_LENGTH];

		rep_count = 1;
		one_argument(argument, arg, MAX_INPUT_LENGTH);
		if (is_number(arg))
		{
			/* look out for: prac # spell */
			argument = one_argument(argument, arg, MAX_INPUT_LENGTH);
			rep_count = atoi(arg);
			if (rep_count < 1)
			{
				send_to_char("Enter a positive number of times to practice.\n\r", ch);
				return;
			}
		}

		if (!IS_AWAKE(ch))
		{
			send_to_char("In your dreams, or what?\n\r", ch);
			return;
		}

		if (ch->exp <= 0)
		{
			send_to_char("You have no exp left.\n\r", ch);
			return;
		}

		if ((sn = skill_lookup(argument)) < 0 || (!IS_NPC(ch) && ch->level < skill_table[sn].skill_level[ch->class]))
		{
			send_to_char("You can't practice that.\n\r", ch);
			return;
		}

		adept = IS_NPC(ch) ? 100 : class_table[ch->class].skill_adept;

		if (ch->pcdata->learned[sn] >= adept)
		{
			snprintf(buf, MAX_STRING_LENGTH, "You are already an adept of %s.\n\r",
					 skill_table[sn].name);
			send_to_char(buf, ch);
		}
		else if (ch->pcdata->learned[sn] > 0 &&
				 (ch->pcdata->learned[sn] / 2) > ch->exp)
		{
			snprintf(buf, MAX_STRING_LENGTH, "You need %d exp to increase %s any more.\n\r",
					 (ch->pcdata->learned[sn] / 2), skill_table[sn].name);
			send_to_char(buf, ch);
		}
		else if (ch->pcdata->learned[sn] == 0 && ch->exp < 500)
		{
			snprintf(buf, MAX_STRING_LENGTH, "You need 500 exp to increase %s.\n\r",
					 skill_table[sn].name);
			send_to_char(buf, ch);
		}
		else
		{
			if (ch->pcdata->learned[sn] == 0)
			{
				ch->exp -= 500;
				ch->pcdata->learned[sn] += get_curr_int(ch);
				--rep_count;
			}
			while ((rep_count > 0) && (ch->pcdata->learned[sn] < adept) && (ch->exp >= ch->pcdata->learned[sn] / 2))
			{
				ch->exp -= (ch->pcdata->learned[sn] / 2);
				ch->pcdata->learned[sn] += get_curr_int(ch);
				--rep_count;
			}
			if (ch->pcdata->learned[sn] < adept)
			{
				act("You practice $T.",
					ch, NULL, skill_table[sn].name, TO_CHAR);
			}
			else
			{
				ch->pcdata->learned[sn] = adept;
				act("You are now an adept of $T.",
					ch, NULL, skill_table[sn].name, TO_CHAR);
			}
		}
	}
	return;
}

void do_prac1(CHAR_DATA *ch, char *argument)
{

	char buf[MAX_STRING_LENGTH];
	int sn;

	if (IS_NPC(ch))
		return;

	if (argument[0] == '\0')
	{
		int col;

		col = 0;
		for (sn = 0; sn < 61; sn++)
		{
			if (skill_table[sn].name == NULL)
				break;
			if (ch->level < skill_table[sn].skill_level[ch->class])
				continue;
			snprintf(buf, MAX_STRING_LENGTH, "%18s %3d%%  ",
					 skill_table[sn].name, ch->pcdata->learned[sn]);
			send_to_char_formatted(buf, ch);
			if (++col % 3 == 0)
				send_to_char_formatted("\n\r", ch);
		}

		if (col % 3 != 0)
			send_to_char("\n\r", ch);

		snprintf(buf, MAX_STRING_LENGTH, "You have %ld exp left.\n\r", ch->exp);
		send_to_char_formatted(buf, ch);
	}
	else
	{
	}
	return;
}

void do_prac2(CHAR_DATA *ch, char *argument)
{

	char buf[MAX_STRING_LENGTH];
	int sn;

	if (IS_NPC(ch))
		return;

	if (argument[0] == '\0')
	{
		int col;

		col = 0;
		for (sn = 61; sn < MAX_SKILL; sn++)
		{
			if (skill_table[sn].name == NULL)
				break;
			if (ch->level < skill_table[sn].skill_level[ch->class])
				continue;
			snprintf(buf, MAX_STRING_LENGTH, "%18s %3d%%  ",
					 skill_table[sn].name, ch->pcdata->learned[sn]);
			send_to_char_formatted(buf, ch);
			if (++col % 3 == 0)
				send_to_char_formatted("\n\r", ch);
		}

		if (col % 3 != 0)
			send_to_char("\n\r", ch);

		snprintf(buf, MAX_STRING_LENGTH, "You have %ld exp left.\n\r", ch->exp);
		send_to_char_formatted(buf, ch);
	}
	else
	{
	}
	return;
}

/*
* 'Wimpy' originally by Dionysos.
*/
void do_wimpy(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int wimpy;

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (arg[0] == '\0')
		wimpy = ch->max_hit / 5;
	else
		wimpy = atoi(arg);

	if (wimpy < 0)
	{
		send_to_char("Your courage exceeds your wisdom.\n\r", ch);
		return;
	}

	if (wimpy > ch->max_hit)
	{
		send_to_char("Such cowardice ill becomes you.\n\r", ch);
		return;
	}

	if (!IS_NPC(ch) && ch->race > 9)
	{
		send_to_char("Such cowardice is not befitting for one of your status !\n\r", ch);
		return;
	}

	if (!IS_NPC(ch) && ch->race > 4 && wimpy > 0)
	{
		send_to_char("Your cowardice shall earn you less experience.\n\r", ch);
	}

	ch->wimpy = wimpy;
	snprintf(buf, MAX_STRING_LENGTH, "Wimpy set to %d hit points.\n\r", wimpy);
	send_to_char(buf, ch);
	return;
}

void do_password(CHAR_DATA *ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char *pArg;
	char *pwdnew;
	char *p;
	char cEnd;
	int arglen;

	if (IS_NPC(ch))
		return;

	/*
	   * Can't use one_argument here because it smashes case.
	   * So we just steal all its code.  Bleagh.
    */
	pArg = arg1;
	arglen = MAX_INPUT_LENGTH;
	while (isspace(*argument))
		argument++;

	cEnd = ' ';
	if (*argument == '\'' || *argument == '"')
		cEnd = *argument++;

	while (*argument != '\0' && arglen > 1)
	{
		if (*argument == cEnd)
		{
			argument++;
			break;
		}
		*pArg++ = *argument++;
		--arglen;
	}
	*pArg = '\0';

	pArg = arg2;
	while (isspace(*argument))
		argument++;

	cEnd = ' ';
	if (*argument == '\'' || *argument == '"')
		cEnd = *argument++;

	while (*argument != '\0')
	{
		if (*argument == cEnd)
		{
			argument++;
			break;
		}
		*pArg++ = *argument++;
	}
	*pArg = '\0';

	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char("Syntax: password <old> <new>.\n\r", ch);
		return;
	}

	if (strcmp(arg1, ch->pcdata->pwd) &&
		strcmp(crypt(arg1, ch->pcdata->pwd), ch->pcdata->pwd))
	{
		WAIT_STATE(ch, 40);
		send_to_char("Wrong password.  Wait 10 seconds.\n\r", ch);
		return;
	}

	if (strlen(arg2) < 5)
	{
		send_to_char(
			"New password must be at least five characters long.\n\r", ch);
		return;
	}

	/*
    * No tilde allowed because of player file format.
    */
	pwdnew = crypt(arg2, ch->name);
	for (p = pwdnew; *p != '\0'; p++)
	{
		if (*p == '~')
		{
			send_to_char(
				"New password not acceptable, try again.\n\r", ch);
			return;
		}
	}

	free_string(ch->pcdata->pwd);
	ch->pcdata->pwd = str_dup(pwdnew);
	if (!IS_EXTRA(ch, EXTRA_NEWPASS))
		SET_BIT(ch->extra, EXTRA_NEWPASS);
	save_char_obj(ch);
	send_to_char("Ok.\n\r", ch);
	return;
}

void do_socials(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	int iSocial;
	int col;

	col = 0;

	for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
	{
		snprintf(buf, MAX_STRING_LENGTH, "%-12s", social_table[iSocial].name);
		send_to_char_formatted(buf, ch);
		if (++col % 6 == 0)
			send_to_char("\n\r", ch);
	}

	if (col % 6 != 0)
		send_to_char_formatted("\n\r", ch);

	return;
}
/*
void do_spells( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    int sn;
    int col;
    
    col = 0;
    for ( sn = 0; sn < MAX_SKILL && skill_table[sn].name != NULL; sn++ )
    {
	   snprintf( buf, MAX_STRING_LENGTH, "%-12s", skill_table[sn].name );
	   send_to_char( buf, ch );
	   if ( ++col % 6 == 0 )
		  send_to_char( "\n\r", ch );
    }
    
    if ( col % 6 != 0 )
	   send_to_char( "\n\r", ch );
    return;
}
*/
/*
* Contributed by Alander.
*/
void do_commands(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	int cmd;
	int col;

	col = 0;
	for (cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++)
	{
		if (cmd_table[cmd].level == 0 && cmd_table[cmd].level <= get_trust(ch))
		{
			snprintf(buf, MAX_STRING_LENGTH, "%-12s", cmd_table[cmd].name);
			send_to_char_formatted(buf, ch);
			if (++col % 6 == 0)
				send_to_char_formatted("\n\r", ch);
		}
	}

	if (col % 6 != 0)
		send_to_char_formatted("\n\r", ch);
	return;
}

void do_channels(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (arg[0] == '\0')
	{
		if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE))
		{
			send_to_char("You are silenced.\n\r", ch);
			return;
		}

		send_to_char("Channels:", ch);

		send_to_char(!IS_SET(ch->deaf, CHANNEL_AUCTION)
						 ? " +AUCTION"
						 : " -auction",
					 ch);

		send_to_char(!IS_SET(ch->deaf, CHANNEL_CHAT)
						 ? " +CHAT"
						 : " -chat",
					 ch);

		send_to_char(!IS_SET(ch->deaf, CHANNEL_NEWBIE)
						 ? " +NEWBIE"
						 : " -newbie",
					 ch);

		send_to_char(!IS_SET(ch->deaf, CHANNEL_MCHAT)
						 ? " +MCHAT"
						 : " -mchat",
					 ch);
#if 0
	   send_to_char( !IS_SET(ch->deaf, CHANNEL_HACKER)
		  ? " +HACKER"
		  : " -hacker",
		  ch );
#endif

		if (IS_IMMORTAL(ch))
		{
			send_to_char(!IS_SET(ch->deaf, CHANNEL_IMMTALK)
							 ? " +IMMTALK"
							 : " -immtalk",
						 ch);
			send_to_char(!IS_SET(ch->deaf, CHANNEL_IMMINFO)
							 ? " +IMMINFO"
							 : " -imminfo",
						 ch);
			/* gunna be changed to justicar channel-- midchange  VAL */
			send_to_char(!IS_SET(ch->deaf, CHANNEL_JUSTITALK)
							 ? " +JUSTITALK"
							 : " -justitalk",
						 ch);
		}

		send_to_char(!IS_SET(ch->deaf, CHANNEL_MUSIC)
						 ? " +MUSIC"
						 : " -music",
					 ch);

		send_to_char(!IS_SET(ch->deaf, CHANNEL_QUESTION)
						 ? " +QUESTION"
						 : " -question",
					 ch);

		send_to_char(!IS_SET(ch->deaf, CHANNEL_BID)
						 ? " +BID"
						 : " -bid",
					 ch);

		send_to_char(!IS_SET(ch->deaf, CHANNEL_VAMPTALK)
						 ? " +VAMPTALK"
						 : " -vamptalk",
					 ch);

		if (!IS_NPC(ch) && (!strncmp(ch->clan, "Lasombra", 4)))
		{
			send_to_char(!IS_SET(ch->deaf, CHANNEL_LASTALK)
							 ? " +LASTALK"
							 : " -lastalk",
						 ch);
		}

		if (!IS_NPC(ch) && (!strncmp(ch->clan, "Tremere", 4)))
		{
			send_to_char(!IS_SET(ch->deaf, CHANNEL_TREMTALK)
							 ? " +TREMTALK"
							 : " -tremtalk",
						 ch);
		}

		if (!IS_NPC(ch) && (!strncmp(ch->clan, "Toreador", 4)))
		{
			send_to_char(!IS_SET(ch->deaf, CHANNEL_TORTALK)
							 ? " +TORTALK"
							 : " -tortalk",
						 ch);
		}

		if (!IS_NPC(ch) && (!strncmp(ch->clan, "Tzimisce", 4)))
		{
			send_to_char(!IS_SET(ch->deaf, CHANNEL_TZITALK)
							 ? " +TZITALK"
							 : " -tzitalk",
						 ch);
		}

		if (!IS_NPC(ch) && (!strncmp(ch->clan, "Ventrue", 4)))
		{
			send_to_char(!IS_SET(ch->deaf, CHANNEL_VENTALK)
							 ? " +VENTALK"
							 : " -ventalk",
						 ch);
		}

		if (!IS_NPC(ch) && (!strncmp(ch->clan, "Assamite", 4)))
		{
			send_to_char(!IS_SET(ch->deaf, CHANNEL_ASSTALK)
							 ? " +ASSTALK"
							 : " -asstalk",
						 ch);
		}
		send_to_char(!IS_SET(ch->deaf, CHANNEL_SHOUT)
						 ? " +SHOUT"
						 : " -shout",
					 ch);

		send_to_char(!IS_SET(ch->deaf, CHANNEL_YELL)
						 ? " +YELL"
						 : " -yell",
					 ch);

		send_to_char(".\n\r", ch);
	}
	else
	{
		bool fClear;
		int bit;

		if (arg[0] == '+')
			fClear = TRUE;
		else if (arg[0] == '-')
			fClear = FALSE;
		else
		{
			send_to_char("Channels -channel or +channel?\n\r", ch);
			return;
		}

		if (!str_cmp(arg + 1, "auction"))
			bit = CHANNEL_AUCTION;
		else if (!str_cmp(arg + 1, "chat"))
			bit = CHANNEL_CHAT;
		else if (!str_cmp(arg + 1, "mchat"))
			bit = CHANNEL_MCHAT;
		else if (!str_cmp(arg + 1, "bid"))
			bit = CHANNEL_BID;
#if 0
	   else if ( !str_cmp( arg+1, "hacker"   ) ) bit = CHANNEL_HACKER;
#endif
		else if (!str_cmp(arg + 1, "immtalk"))
			bit = CHANNEL_IMMTALK;
		else if (!str_cmp(arg + 1, "music"))
			bit = CHANNEL_MUSIC;
		else if (!str_cmp(arg + 1, "question"))
			bit = CHANNEL_QUESTION;
		else if (!str_cmp(arg + 1, "shout"))
			bit = CHANNEL_SHOUT;
		else if (!str_cmp(arg + 1, "yell"))
			bit = CHANNEL_YELL;
		else if (!str_cmp(arg + 1, "imminfo"))
			bit = CHANNEL_IMMINFO;
		else if (!str_cmp(arg + 1, "justitalk"))
			bit = CHANNEL_JUSTITALK;
		else if (!str_cmp(arg + 1, "vamptalk"))
			bit = CHANNEL_VAMPTALK;
		else if (!str_cmp(arg + 1, "asstalk"))
			bit = CHANNEL_ASSTALK;
		else if (!str_cmp(arg + 1, "tzitalk"))
			bit = CHANNEL_TZITALK;
		else if (!str_cmp(arg + 1, "tremtalk"))
			bit = CHANNEL_TREMTALK;
		else if (!str_cmp(arg + 1, "tortalk"))
			bit = CHANNEL_TORTALK;
		else if (!str_cmp(arg + 1, "ventalk"))
			bit = CHANNEL_VENTALK;
		else if (!str_cmp(arg + 1, "lastalk"))
			bit = CHANNEL_LASTALK;
		else if (!str_cmp(arg + 1, "newbie"))
			bit = CHANNEL_NEWBIE;
		else
		{
			send_to_char("Set or clear which channel?\n\r", ch);
			return;
		}

		if (fClear)
			REMOVE_BIT(ch->deaf, bit);
		else
			SET_BIT(ch->deaf, bit);

		send_to_char("Ok.\n\r", ch);
	}

	return;
}

/*
* Contributed by Grodyn.
*/
void do_config(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	if (IS_NPC(ch))
		return;

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (arg[0] == '\0')
	{
		send_to_char_formatted("[ Keyword  ] Option\n\r", ch);

		send_to_char_formatted(IS_SET(ch->act, PLR_ANSI)
								   ? "[+ANSI     ] You have ansi colour on.\n\r"
								   : "[-ansi     ] You have ansi colour off.\n\r",
							   ch);

		send_to_char_formatted(IS_SET(ch->act, PLR_AUTOEXIT)
								   ? "[+AUTOEXIT ] You automatically see exits.\n\r"
								   : "[-autoexit ] You don't automatically see exits.\n\r",
							   ch);

		send_to_char_formatted(IS_SET(ch->act, PLR_AUTOLOOT)
								   ? "[+AUTOLOOT ] You automatically loot corpses.\n\r"
								   : "[-autoloot ] You don't automatically loot corpses.\n\r",
							   ch);

		send_to_char_formatted(IS_SET(ch->act, PLR_AUTOSAC)
								   ? "[+AUTOSAC  ] You automatically sacrifice corpses.\n\r"
								   : "[-autosac  ] You don't automatically sacrifice corpses.\n\r",
							   ch);

		send_to_char_formatted(IS_SET(ch->act, PLR_AUTOGOLD)
								   ? "[+AUTOGOLD  ] You automatically loot gold from corpses.\n\r"
								   : "[-autogold  ] You don't automatically loot gold from corpses.\n\r",
							   ch);

		send_to_char_formatted(IS_SET(ch->act, PLR_BLANK)
								   ? "[+BLANK    ] You have a blank line before your prompt.\n\r"
								   : "[-blank    ] You have no blank line before your prompt.\n\r",
							   ch);

		send_to_char_formatted(IS_SET(ch->act, PLR_BRIEF)
								   ? "[+BRIEF    ] You see brief descriptions.\n\r"
								   : "[-brief    ] You see long descriptions.\n\r",
							   ch);

		send_to_char_formatted(IS_SET(ch->act, PLR_COMBINE)
								   ? "[+COMBINE  ] You see object lists in combined format.\n\r"
								   : "[-combine  ] You see object lists in single format.\n\r",
							   ch);

		send_to_char_formatted(IS_SET(ch->act, PLR_FIGHT)
								   ? "[+FIGHT    ] You will not see parry/misses/dodges.\n\r"
								   : "[-fight    ] You will see parry/misses/dodges.\n\r",
							   ch);

		send_to_char_formatted(IS_SET(ch->act, PLR_FIGHT2)
								   ? "[+FIGHT2   ] You will not see shield attacks.\n\r"
								   : "[-fight2   ] You will see shield attacks.\n\r",
							   ch);

		send_to_char_formatted(IS_SET(ch->act, PLR_PROMPT)
								   ? "[+PROMPT   ] You have a prompt.\n\r"
								   : "[-prompt   ] You don't have a prompt.\n\r",
							   ch);

		send_to_char_formatted(IS_SET(ch->act, PLR_TELNET_GA)
								   ? "[+TELNETGA ] You receive a telnet GA sequence.\n\r"
								   : "[-telnetga ] You don't receive a telnet GA sequence.\n\r",
							   ch);

		send_to_char_formatted(IS_SET(ch->act, PLR_SILENCE)
								   ? "[+SILENCE  ] You are silenced.\n\r"
								   : "",
							   ch);

		send_to_char_formatted(!IS_SET(ch->act, PLR_NO_EMOTE)
								   ? ""
								   : "[-emote    ] You can't emote.\n\r",
							   ch);

		send_to_char_formatted(!IS_SET(ch->act, PLR_NO_TELL)
								   ? ""
								   : "[-tell     ] You can't use 'tell'.\n\r",
							   ch);
	}
	else
	{
		bool fSet;
		int bit;

		if (arg[0] == '+')
			fSet = TRUE;
		else if (arg[0] == '-')
			fSet = FALSE;
		else
		{
			send_to_char("Config -option or +option?\n\r", ch);
			return;
		}

		if (!str_cmp(arg + 1, "ansi"))
			bit = PLR_ANSI;
		else if (!str_cmp(arg + 1, "autoexit"))
			bit = PLR_AUTOEXIT;
		else if (!str_cmp(arg + 1, "autoloot"))
			bit = PLR_AUTOLOOT;
		else if (!str_cmp(arg + 1, "autosac"))
			bit = PLR_AUTOSAC;
		else if (!str_cmp(arg + 1, "autogold"))
			bit = PLR_AUTOGOLD;
		else if (!str_cmp(arg + 1, "blank"))
			bit = PLR_BLANK;
		else if (!str_cmp(arg + 1, "brief"))
			bit = PLR_BRIEF;
		else if (!str_cmp(arg + 1, "combine"))
			bit = PLR_COMBINE;
		else if (!str_cmp(arg + 1, "fight"))
			bit = PLR_FIGHT;
		else if (!str_cmp(arg + 1, "fight2"))
			bit = PLR_FIGHT2;
		else if (!str_cmp(arg + 1, "prompt"))
			bit = PLR_PROMPT;
		else if (!str_cmp(arg + 1, "telnetga"))
			bit = PLR_TELNET_GA;
		else
		{
			send_to_char("Config which option?\n\r", ch);
			return;
		}

		if (fSet)
			SET_BIT(ch->act, bit);
		else
			REMOVE_BIT(ch->act, bit);

		send_to_char("Ok.\n\r", ch);
	}

	return;
}

void do_ansi(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
		return;
	if (IS_SET(ch->act, PLR_ANSI))
		do_config(ch, "-ansi");
	else
		do_config(ch, "+ansi");
	return;
}

void do_autoexit(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
		return;
	if (IS_SET(ch->act, PLR_AUTOEXIT))
		do_config(ch, "-autoexit");
	else
		do_config(ch, "+autoexit");
	return;
}

void do_autoloot(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
		return;
	if (IS_SET(ch->act, PLR_AUTOLOOT))
		do_config(ch, "-autoloot");
	else
		do_config(ch, "+autoloot");
	return;
}

void do_autosac(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
		return;
	if (IS_SET(ch->act, PLR_AUTOSAC))
		do_config(ch, "-autosac");
	else
		do_config(ch, "+autosac");
	return;
}

void do_autogold(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
		return;
	if (IS_SET(ch->act, PLR_AUTOGOLD))
		do_config(ch, "-autogold");
	else
		do_config(ch, "+autogold");
	return;
}

void do_blank(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
		return;
	if (IS_SET(ch->act, PLR_BLANK))
		do_config(ch, "-blank");
	else
		do_config(ch, "+blank");
	return;
}

void do_fightconfig(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
		return;
	if (IS_SET(ch->act, PLR_FIGHT))
		do_config(ch, "-fight");
	else
		do_config(ch, "+fight");
	return;
}

void do_fight2config(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
		return;
	if (IS_SET(ch->act, PLR_FIGHT2))
		do_config(ch, "-fight2");
	else
		do_config(ch, "+fight2");
	return;
}

void do_brief(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
		return;
	if (IS_SET(ch->act, PLR_BRIEF))
		do_config(ch, "-brief");
	else
		do_config(ch, "+brief");
	return;
}

void do_prompt(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];

	if (argument[0] == '\0')
	{
		do_help(ch, "prompt");
		return;
	}

	if (!str_cmp(argument, "on"))
	{
		if (IS_SET(ch->act, PLR_PROMPT))
		{
			send_to_char("Your prompt is already on!\n\r", ch);
		}
		else
		{
			send_to_char("You will now see prompts.\n\r", ch);
			SET_BIT(ch->act, PLR_PROMPT);
		}
		return;
	}

	if (!str_cmp(argument, "off"))
	{
		if (!IS_SET(ch->act, PLR_PROMPT))
		{
			send_to_char("Your prompt is already off!\n\r", ch);
		}
		else
		{
			send_to_char("You will no longer see prompts.\n\r", ch);
			REMOVE_BIT(ch->act, PLR_PROMPT);
		}
		return;
	}

	if (!strcmp(argument, "reset"))
		strcpy(buf, "");
	else
	{
		if (strlen(argument) > 50)
			argument[50] = '\0';
		strcpy(buf, argument);
		smash_tilde(buf);
		if (str_suffix("%c", buf))
			strcat(buf, " ");
	}

	free_string(ch->prompt);
	ch->prompt = str_dup(buf);
	sprintf(buf, "Prompt set to %s\n\r", ch->prompt);
	send_to_char(buf, ch);
	return;
}

void do_diagnose(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int teeth = 0;
	int ribs = 0;
	CHAR_DATA *victim;

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (arg == '\0')
	{
		send_to_char("Who do you wish to diagnose?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("Nobody here by that name.\n\r", ch);
		return;
	}
	act("$n examines $N carefully, diagnosing $S injuries.", ch, NULL, victim, TO_NOTVICT);
	act("$n examines you carefully, diagnosing your injuries.", ch, NULL, victim, TO_VICT);
	act("Your diagnoses of $N reveals the following...", ch, NULL, victim, TO_CHAR);
	send_to_char("--------------------------------------------------------------------------------\n\r", ch);
	if ((victim->loc_hp[0] + victim->loc_hp[1] + victim->loc_hp[2] +
		 victim->loc_hp[3] + victim->loc_hp[4] + victim->loc_hp[5] +
		 victim->loc_hp[6]) == 0)
	{
		act("$N has no apparent injuries.", ch, NULL, victim, TO_CHAR);
		send_to_char("--------------------------------------------------------------------------------\n\r", ch);
		return;
	}
	/* Check head */
	if (IS_HEAD(victim, LOST_EYE_L) && IS_HEAD(victim, LOST_EYE_R))
		act("$N has lost both of $S eyes.", ch, NULL, victim, TO_CHAR);
	else if (IS_HEAD(victim, LOST_EYE_L))
		act("$N has lost $S left eye.", ch, NULL, victim, TO_CHAR);
	else if (IS_HEAD(victim, LOST_EYE_R))
		act("$N has lost $S right eye.", ch, NULL, victim, TO_CHAR);
	if (IS_HEAD(victim, LOST_EAR_L) && IS_HEAD(victim, LOST_EAR_R))
		act("$N has lost both of $S ears.", ch, NULL, victim, TO_CHAR);
	else if (IS_HEAD(victim, LOST_EAR_L))
		act("$N has lost $S left ear.", ch, NULL, victim, TO_CHAR);
	else if (IS_HEAD(victim, LOST_EAR_R))
		act("$N has lost $S right ear.", ch, NULL, victim, TO_CHAR);
	if (IS_HEAD(victim, LOST_NOSE))
		act("$N has lost $S nose.", ch, NULL, victim, TO_CHAR);
	else if (IS_HEAD(victim, BROKEN_NOSE))
		act("$N has got a broken nose.", ch, NULL, victim, TO_CHAR);
	if (IS_HEAD(victim, BROKEN_JAW))
		act("$N has got a broken jaw.", ch, NULL, victim, TO_CHAR);
	if (IS_HEAD(victim, LOST_HEAD))
	{
		act("$N has had $S head cut off.", ch, NULL, victim, TO_CHAR);
		if (IS_BLEEDING(victim, BLEEDING_HEAD))
			act("...Blood is spurting from the stump of $S neck.", ch, NULL, victim, TO_CHAR);
	}
	else
	{
		if (IS_BODY(victim, BROKEN_NECK))
			act("$N has got a broken neck.", ch, NULL, victim, TO_CHAR);
		if (IS_BODY(victim, CUT_THROAT))
		{
			act("$N has had $S throat cut open.", ch, NULL, victim, TO_CHAR);
			if (IS_BLEEDING(victim, BLEEDING_THROAT))
				act("...Blood is pouring from the wound.", ch, NULL, victim, TO_CHAR);
		}
	}
	if (IS_HEAD(victim, BROKEN_SKULL))
		act("$N has got a broken skull.", ch, NULL, victim, TO_CHAR);
	if (IS_HEAD(victim, LOST_TOOTH_1))
		teeth += 1;
	if (IS_HEAD(victim, LOST_TOOTH_2))
		teeth += 2;
	if (IS_HEAD(victim, LOST_TOOTH_4))
		teeth += 4;
	if (IS_HEAD(victim, LOST_TOOTH_8))
		teeth += 8;
	if (IS_HEAD(victim, LOST_TOOTH_16))
		teeth += 16;
	if (teeth > 0)
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has had %d teeth knocked out.", teeth);
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	if (IS_HEAD(victim, LOST_TONGUE))
		act("$N has had $S tongue ripped out.", ch, NULL, victim, TO_CHAR);
	if (IS_HEAD(victim, LOST_HEAD))
	{
		send_to_char("--------------------------------------------------------------------------------\n\r", ch);
		return;
	}
	/* Check body */
	if (IS_BODY(victim, BROKEN_RIBS_1))
		ribs += 1;
	if (IS_BODY(victim, BROKEN_RIBS_2))
		ribs += 2;
	if (IS_BODY(victim, BROKEN_RIBS_4))
		ribs += 4;
	if (IS_BODY(victim, BROKEN_RIBS_8))
		ribs += 8;
	if (IS_BODY(victim, BROKEN_RIBS_16))
		ribs += 16;
	if (ribs > 0)
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has got %d broken ribs.", ribs);
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	if (IS_BODY(victim, BROKEN_SPINE))
		act("$N has got a broken spine.", ch, NULL, victim, TO_CHAR);
	/* Check arms */
	check_left_arm(ch, victim);
	check_right_arm(ch, victim);
	check_left_leg(ch, victim);
	check_right_leg(ch, victim);
	send_to_char("--------------------------------------------------------------------------------\n\r", ch);
	return;
}

void check_left_arm(CHAR_DATA *ch, CHAR_DATA *victim)
{
	char buf[MAX_STRING_LENGTH];
	char finger[10];
	int fingers = 0;

	if (IS_ARM_L(victim, LOST_ARM) && IS_ARM_R(victim, LOST_ARM))
	{
		act("$N has lost both of $S arms.", ch, NULL, victim, TO_CHAR);
		if (IS_BLEEDING(victim, BLEEDING_ARM_L) && IS_BLEEDING(victim, BLEEDING_ARM_R))
			act("...Blood is spurting from both stumps.", ch, NULL, victim, TO_CHAR);
		else if (IS_BLEEDING(victim, BLEEDING_ARM_L))
			act("...Blood is spurting from the left stump.", ch, NULL, victim, TO_CHAR);
		else if (IS_BLEEDING(victim, BLEEDING_ARM_R))
			act("...Blood is spurting from the right stump.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (IS_ARM_L(victim, LOST_ARM))
	{
		act("$N has lost $S left arm.", ch, NULL, victim, TO_CHAR);
		if (IS_BLEEDING(victim, BLEEDING_ARM_L))
			act("...Blood is spurting from the stump.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (IS_ARM_L(victim, BROKEN_ARM) && IS_ARM_R(victim, BROKEN_ARM))
		act("$N arms are both broken.", ch, NULL, victim, TO_CHAR);
	else if (IS_ARM_L(victim, BROKEN_ARM))
		act("$N's left arm is broken.", ch, NULL, victim, TO_CHAR);
	if (IS_ARM_L(victim, LOST_HAND) && IS_ARM_R(victim, LOST_HAND))
	{
		act("$N has lost both of $S hands.", ch, NULL, victim, TO_CHAR);
		if (IS_BLEEDING(victim, BLEEDING_HAND_L) && IS_BLEEDING(victim, BLEEDING_HAND_R))
			act("...Blood is spurting from both stumps.", ch, NULL, victim, TO_CHAR);
		else if (IS_BLEEDING(victim, BLEEDING_HAND_L))
			act("...Blood is spurting from the left stump.", ch, NULL, victim, TO_CHAR);
		else if (IS_BLEEDING(victim, BLEEDING_HAND_R))
			act("...Blood is spurting from the right stump.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (IS_ARM_L(victim, LOST_HAND))
	{
		act("$N has lost $S left hand.", ch, NULL, victim, TO_CHAR);
		if (IS_BLEEDING(victim, BLEEDING_HAND_L))
			act("...Blood is spurting from the stump.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (IS_ARM_L(victim, LOST_FINGER_I))
		fingers += 1;
	if (IS_ARM_L(victim, LOST_FINGER_M))
		fingers += 1;
	if (IS_ARM_L(victim, LOST_FINGER_R))
		fingers += 1;
	if (IS_ARM_L(victim, LOST_FINGER_L))
		fingers += 1;
	if (fingers == 1)
		snprintf(finger, 10, "finger");
	else
		snprintf(finger, 10, "fingers");
	if (fingers > 0 && IS_ARM_L(victim, LOST_THUMB))
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has lost %d %s and $S thumb from $S left hand.", fingers, finger);
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	else if (fingers > 0)
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has lost %d %s from $S left hand.", fingers, finger);
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	else if (IS_ARM_L(victim, LOST_THUMB))
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has lost the thumb from $S left hand.");
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	fingers = 0;
	if (IS_ARM_L(victim, BROKEN_FINGER_I) && !IS_ARM_L(victim, LOST_FINGER_I))
		fingers += 1;
	if (IS_ARM_L(victim, BROKEN_FINGER_M) && !IS_ARM_L(victim, LOST_FINGER_M))
		fingers += 1;
	if (IS_ARM_L(victim, BROKEN_FINGER_R) && !IS_ARM_L(victim, LOST_FINGER_R))
		fingers += 1;
	if (IS_ARM_L(victim, BROKEN_FINGER_L) && !IS_ARM_L(victim, LOST_FINGER_L))
		fingers += 1;
	if (fingers == 1)
		snprintf(finger, 10, "finger");
	else
		snprintf(finger, 10, "fingers");
	if (fingers > 0 && IS_ARM_L(victim, BROKEN_THUMB) && !IS_ARM_L(victim, LOST_THUMB))
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has broken %d %s and $S thumb on $S left hand.", fingers, finger);
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	else if (fingers > 0)
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has broken %d %s on $S left hand.", fingers, finger);
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	else if (IS_ARM_L(victim, BROKEN_THUMB) && !IS_ARM_L(victim, LOST_THUMB))
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has broken the thumb on $S left hand.");
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	return;
}

void check_right_arm(CHAR_DATA *ch, CHAR_DATA *victim)
{
	char buf[MAX_STRING_LENGTH];
	char finger[MAX_STRING_LENGTH];
	int fingers = 0;

	if (IS_ARM_L(victim, LOST_ARM) && IS_ARM_R(victim, LOST_ARM))
		return;
	if (IS_ARM_R(victim, LOST_ARM))
	{
		act("$N has lost $S right arm.", ch, NULL, victim, TO_CHAR);
		if (IS_BLEEDING(victim, BLEEDING_ARM_R))
			act("...Blood is spurting from the stump.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (!IS_ARM_L(victim, BROKEN_ARM) && IS_ARM_R(victim, BROKEN_ARM))
		act("$N's right arm is broken.", ch, NULL, victim, TO_CHAR);
	if (IS_ARM_L(victim, LOST_HAND) && IS_ARM_R(victim, LOST_HAND))
		return;
	if (IS_ARM_R(victim, LOST_HAND))
	{
		act("$N has lost $S right hand.", ch, NULL, victim, TO_CHAR);
		if (IS_BLEEDING(victim, BLEEDING_HAND_R))
			act("...Blood is spurting from the stump.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (IS_ARM_R(victim, LOST_FINGER_I))
		fingers += 1;
	if (IS_ARM_R(victim, LOST_FINGER_M))
		fingers += 1;
	if (IS_ARM_R(victim, LOST_FINGER_R))
		fingers += 1;
	if (IS_ARM_R(victim, LOST_FINGER_L))
		fingers += 1;
	if (fingers == 1)
		snprintf(finger, 10, "finger");
	else
		snprintf(finger, 10, "fingers");
	if (fingers > 0 && IS_ARM_R(victim, LOST_THUMB))
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has lost %d %s and $S thumb from $S right hand.", fingers, finger);
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	else if (fingers > 0)
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has lost %d %s from $S right hand.", fingers, finger);
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	else if (IS_ARM_R(victim, LOST_THUMB))
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has lost the thumb from $S right hand.");
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	fingers = 0;
	if (IS_ARM_R(victim, BROKEN_FINGER_I) && !IS_ARM_R(victim, LOST_FINGER_I))
		fingers += 1;
	if (IS_ARM_R(victim, BROKEN_FINGER_M) && !IS_ARM_R(victim, LOST_FINGER_M))
		fingers += 1;
	if (IS_ARM_R(victim, BROKEN_FINGER_R) && !IS_ARM_R(victim, LOST_FINGER_R))
		fingers += 1;
	if (IS_ARM_R(victim, BROKEN_FINGER_L) && !IS_ARM_R(victim, LOST_FINGER_L))
		fingers += 1;
	if (fingers == 1)
		snprintf(finger, 10, "finger");
	else
		snprintf(finger, MAX_STRING_LENGTH, "fingers");
	if (fingers > 0 && IS_ARM_R(victim, BROKEN_THUMB) && !IS_ARM_R(victim, LOST_THUMB))
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has broken %d %s and $S thumb on $S right hand.", fingers, finger);
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	else if (fingers > 0)
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has broken %d %s on $S right hand.", fingers, finger);
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	else if (IS_ARM_R(victim, BROKEN_THUMB) && !IS_ARM_R(victim, LOST_THUMB))
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has broken the thumb on $S right hand.");
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	return;
}

void check_left_leg(CHAR_DATA *ch, CHAR_DATA *victim)
{
	char buf[MAX_STRING_LENGTH];
	char toe[10];
	int toes = 0;

	if (IS_LEG_L(victim, LOST_LEG) && IS_LEG_R(victim, LOST_LEG))
	{
		act("$N has lost both of $S legs.", ch, NULL, victim, TO_CHAR);
		if (IS_BLEEDING(victim, BLEEDING_LEG_L) && IS_BLEEDING(victim, BLEEDING_LEG_R))
			act("...Blood is spurting from both stumps.", ch, NULL, victim, TO_CHAR);
		else if (IS_BLEEDING(victim, BLEEDING_LEG_L))
			act("...Blood is spurting from the left stump.", ch, NULL, victim, TO_CHAR);
		else if (IS_BLEEDING(victim, BLEEDING_LEG_R))
			act("...Blood is spurting from the right stump.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (IS_LEG_L(victim, LOST_LEG))
	{
		act("$N has lost $S left leg.", ch, NULL, victim, TO_CHAR);
		if (IS_BLEEDING(victim, BLEEDING_LEG_L))
			act("...Blood is spurting from the stump.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (IS_LEG_L(victim, BROKEN_LEG) && IS_LEG_R(victim, BROKEN_LEG))
		act("$N legs are both broken.", ch, NULL, victim, TO_CHAR);
	else if (IS_LEG_L(victim, BROKEN_LEG))
		act("$N's left leg is broken.", ch, NULL, victim, TO_CHAR);
	if (IS_LEG_L(victim, LOST_FOOT) && IS_LEG_R(victim, LOST_FOOT))
	{
		act("$N has lost both of $S feet.", ch, NULL, victim, TO_CHAR);
		if (IS_BLEEDING(victim, BLEEDING_FOOT_L) && IS_BLEEDING(victim, BLEEDING_FOOT_R))
			act("...Blood is spurting from both stumps.", ch, NULL, victim, TO_CHAR);
		else if (IS_BLEEDING(victim, BLEEDING_FOOT_L))
			act("...Blood is spurting from the left stump.", ch, NULL, victim, TO_CHAR);
		else if (IS_BLEEDING(victim, BLEEDING_FOOT_R))
			act("...Blood is spurting from the right stump.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (IS_LEG_L(victim, LOST_FOOT))
	{
		act("$N has lost $S left foot.", ch, NULL, victim, TO_CHAR);
		if (IS_BLEEDING(victim, BLEEDING_FOOT_L))
			act("...Blood is spurting from the stump.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (IS_LEG_L(victim, LOST_TOE_A))
		toes += 1;
	if (IS_LEG_L(victim, LOST_TOE_B))
		toes += 1;
	if (IS_LEG_L(victim, LOST_TOE_C))
		toes += 1;
	if (IS_LEG_L(victim, LOST_TOE_D))
		toes += 1;
	if (toes == 1)
		snprintf(toe, 10, "toe");
	else
		snprintf(toe, 10, "toes");
	if (toes > 0 && IS_LEG_L(victim, LOST_TOE_BIG))
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has lost %d %s and $S big toe from $S left foot.", toes, toe);
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	else if (toes > 0)
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has lost %d %s from $S left foot.", toes, toe);
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	else if (IS_LEG_L(victim, LOST_TOE_BIG))
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has lost the big toe from $S left foot.");
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	toes = 0;
	if (IS_LEG_L(victim, BROKEN_TOE_A) && !IS_LEG_L(victim, LOST_TOE_A))
		toes += 1;
	if (IS_LEG_L(victim, BROKEN_TOE_B) && !IS_LEG_L(victim, LOST_TOE_B))
		toes += 1;
	if (IS_LEG_L(victim, BROKEN_TOE_C) && !IS_LEG_L(victim, LOST_TOE_C))
		toes += 1;
	if (IS_LEG_L(victim, BROKEN_TOE_D) && !IS_LEG_L(victim, LOST_TOE_D))
		toes += 1;
	if (toes == 1)
		snprintf(toe, 10, "toe");
	else
		snprintf(toe, 10, "toes");
	if (toes > 0 && IS_LEG_L(victim, BROKEN_TOE_BIG) && !IS_LEG_L(victim, LOST_TOE_BIG))
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has broken %d %s and $S big toe from $S left foot.", toes, toe);
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	else if (toes > 0)
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has broken %d %s on $S left foot.", toes, toe);
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	else if (IS_LEG_L(victim, BROKEN_TOE_BIG) && !IS_LEG_L(victim, LOST_TOE_BIG))
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has broken the big toe on $S left foot.");
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	return;
}

void check_right_leg(CHAR_DATA *ch, CHAR_DATA *victim)
{
	char buf[MAX_STRING_LENGTH];
	char toe[10];
	int toes = 0;

	if (IS_LEG_L(victim, LOST_LEG) && IS_LEG_R(victim, LOST_LEG))
		return;
	if (IS_LEG_R(victim, LOST_LEG))
	{
		act("$N has lost $S right leg.", ch, NULL, victim, TO_CHAR);
		if (IS_BLEEDING(victim, BLEEDING_LEG_R))
			act("...Blood is spurting from the stump.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (!IS_LEG_L(victim, BROKEN_LEG) && IS_LEG_R(victim, BROKEN_LEG))
		act("$N's right leg is broken.", ch, NULL, victim, TO_CHAR);
	if (IS_LEG_L(victim, LOST_FOOT) && IS_LEG_R(victim, LOST_FOOT))
		return;
	if (IS_LEG_R(victim, LOST_FOOT))
	{
		act("$N has lost $S right foot.", ch, NULL, victim, TO_CHAR);
		if (IS_BLEEDING(victim, BLEEDING_FOOT_R))
			act("...Blood is spurting from the stump.", ch, NULL, victim, TO_CHAR);
		return;
	}
	if (IS_LEG_R(victim, LOST_TOE_A))
		toes += 1;
	if (IS_LEG_R(victim, LOST_TOE_B))
		toes += 1;
	if (IS_LEG_R(victim, LOST_TOE_C))
		toes += 1;
	if (IS_LEG_R(victim, LOST_TOE_D))
		toes += 1;
	if (toes == 1)
		snprintf(toe, 10, "toe");
	else
		snprintf(toe, 10, "toes");
	if (toes > 0 && IS_LEG_R(victim, LOST_TOE_BIG))
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has lost %d %s and $S big toe from $S right foot.", toes, toe);
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	else if (toes > 0)
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has lost %d %s from $S right foot.", toes, toe);
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	else if (IS_LEG_R(victim, LOST_TOE_BIG))
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has lost the big toe from $S right foot.");
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	toes = 0;
	if (IS_LEG_R(victim, BROKEN_TOE_A) && !IS_LEG_R(victim, LOST_TOE_A))
		toes += 1;
	if (IS_LEG_R(victim, BROKEN_TOE_B) && !IS_LEG_R(victim, LOST_TOE_B))
		toes += 1;
	if (IS_LEG_R(victim, BROKEN_TOE_C) && !IS_LEG_R(victim, LOST_TOE_C))
		toes += 1;
	if (IS_LEG_R(victim, BROKEN_TOE_D) && !IS_LEG_R(victim, LOST_TOE_D))
		toes += 1;
	if (toes == 1)
		snprintf(toe, 10, "toe");
	else
		snprintf(toe, 10, "toes");
	if (toes > 0 && IS_LEG_R(victim, BROKEN_TOE_BIG) && !IS_LEG_R(victim, LOST_TOE_BIG))
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has broken %d %s and $S big toe on $S right foot.", toes, toe);
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	else if (toes > 0)
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has broken %d %s on $S right foot.", toes, toe);
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	else if (IS_LEG_R(victim, BROKEN_TOE_BIG) && !IS_LEG_R(victim, LOST_TOE_BIG))
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N has broken the big toe on $S right foot.");
		act(buf, ch, NULL, victim, TO_CHAR);
	}
	return;
}

void obj_score(CHAR_DATA *ch, OBJ_DATA *obj)
{
	char buf[MAX_STRING_LENGTH];
	AFFECT_DATA *paf;
	int itemtype;

	snprintf(buf, MAX_STRING_LENGTH, "You are %s.\n\r", obj->short_descr);
	send_to_char(buf, ch);

	snprintf(buf, MAX_STRING_LENGTH, "Type %s, Extra flags %s.\n\r", item_type_name(obj),
			 extra_bit_name(obj->extra_flags));
	send_to_char(buf, ch);

	snprintf(buf, MAX_STRING_LENGTH, "You weigh %ld pounds and are worth %ld gold coins.\n\r", obj->weight, obj->cost);
	send_to_char(buf, ch);

	if (obj->questmaker != NULL && strlen(obj->questmaker) > 1 &&
		obj->questowner != NULL && strlen(obj->questowner) > 1)
	{
		snprintf(buf, MAX_STRING_LENGTH, "You were created by %s, and are owned by %s.\n\r", obj->questmaker, obj->questowner);
		send_to_char(buf, ch);
	}
	else if (obj->questmaker != NULL && strlen(obj->questmaker) > 1)
	{
		snprintf(buf, MAX_STRING_LENGTH, "You were created by %s.\n\r", obj->questmaker);
		send_to_char(buf, ch);
	}
	else if (obj->questowner != NULL && strlen(obj->questowner) > 1)
	{
		snprintf(buf, MAX_STRING_LENGTH, "You are owned by %s.\n\r", obj->questowner);
		send_to_char(buf, ch);
	}

	switch (obj->item_type)
	{
	case ITEM_SCROLL:
	case ITEM_POTION:
		snprintf(buf, MAX_STRING_LENGTH, "You contain level %d spells of:", obj->value[0]);
		send_to_char(buf, ch);

		if (obj->value[1] >= 0 && obj->value[1] < MAX_SKILL)
		{
			send_to_char(" '", ch);
			send_to_char(skill_table[obj->value[1]].name, ch);
			send_to_char("'", ch);
		}

		if (obj->value[2] >= 0 && obj->value[2] < MAX_SKILL)
		{
			send_to_char(" '", ch);
			send_to_char(skill_table[obj->value[2]].name, ch);
			send_to_char("'", ch);
		}

		if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
		{
			send_to_char(" '", ch);
			send_to_char(skill_table[obj->value[3]].name, ch);
			send_to_char("'", ch);
		}

		send_to_char(".\n\r", ch);
		break;

	case ITEM_QUEST:
		snprintf(buf, MAX_STRING_LENGTH, "Your quest point value is %d.\n\r", obj->value[0]);
		send_to_char(buf, ch);
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		snprintf(buf, MAX_STRING_LENGTH, "You have %d(%d) charges of level %d",
				 obj->value[1], obj->value[2], obj->value[0]);
		send_to_char(buf, ch);

		if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
		{
			send_to_char(" '", ch);
			send_to_char(skill_table[obj->value[3]].name, ch);
			send_to_char("'", ch);
		}

		send_to_char(".\n\r", ch);
		break;

	case ITEM_WEAPON:
	case ITEM_WEAPON_15HAND:
	case ITEM_WEAPON_2HAND:
		snprintf(buf, MAX_STRING_LENGTH, "You inflict %d to %d damage in combat (average %d).\n\r",
				 obj->value[1], obj->value[2],
				 (obj->value[1] + obj->value[2]) / 2);
		send_to_char(buf, ch);

		if (obj->value[0] >= 1000)
			itemtype = obj->value[0] - ((obj->value[0] / 1000) * 1000);
		else
			itemtype = obj->value[0];

		if (itemtype > 0)
		{
			if (obj->level < 10)
				snprintf(buf, MAX_STRING_LENGTH, "You are a minor spell weapon.\n\r");
			else if (obj->level < 20)
				snprintf(buf, MAX_STRING_LENGTH, "You are a lesser spell weapon.\n\r");
			else if (obj->level < 30)
				snprintf(buf, MAX_STRING_LENGTH, "You are an average spell weapon.\n\r");
			else if (obj->level < 40)
				snprintf(buf, MAX_STRING_LENGTH, "You are a greater spell weapon.\n\r");
			else if (obj->level < 50)
				snprintf(buf, MAX_STRING_LENGTH, "You are a major spell weapon.\n\r");
			else
				snprintf(buf, MAX_STRING_LENGTH, "You are a supreme spell weapon.\n\r");
			send_to_char(buf, ch);
		}

		if (itemtype == 1)
			snprintf(buf, MAX_STRING_LENGTH, "You are dripping with corrosive #ga#Gc#gi#Gd#e.\n\r");
		else if (itemtype == 4)
			snprintf(buf, MAX_STRING_LENGTH, "You radiate an aura of darkness.\n\r");
		else if (itemtype == 30)
			snprintf(buf, MAX_STRING_LENGTH, "You are the bane of all evil.\n\r");
		else if (itemtype == 34)
			snprintf(buf, MAX_STRING_LENGTH, "You drink the souls of your victims.\n\r");
		else if (itemtype == 37)
			snprintf(buf, MAX_STRING_LENGTH, "You have been tempered in hellfire.\n\r");
		else if (itemtype == 48)
			snprintf(buf, MAX_STRING_LENGTH, "You crackle with sparks of #ll#wi#lg#wh#lt#wn#li#wn#lg#e.\n\r");
		else if (itemtype == 53)
			snprintf(buf, MAX_STRING_LENGTH, "You are dripping with a dark poison.\n\r");
		else if (itemtype > 0)
			snprintf(buf, MAX_STRING_LENGTH, "You have been imbued with the power of %s.\n\r", skill_table[itemtype].name);
		if (itemtype > 0)
			send_to_char(buf, ch);

		if (obj->value[0] >= 1000)
			itemtype = obj->value[0] / 1000;
		else
			break;

		if (itemtype == 4 || itemtype == 1)
			snprintf(buf, MAX_STRING_LENGTH, "You radiate an aura of darkness.\n\r");
		else if (itemtype == 27 || itemtype == 2)
			snprintf(buf, MAX_STRING_LENGTH, "You allow your wielder to see invisible things.\n\r");
		else if (itemtype == 39 || itemtype == 3)
			snprintf(buf, MAX_STRING_LENGTH, "You grant your wielder the power of flight.\n\r");
		else if (itemtype == 45 || itemtype == 4)
			snprintf(buf, MAX_STRING_LENGTH, "You allow your wielder to see in the dark.\n\r");
		else if (itemtype == 46 || itemtype == 5)
			snprintf(buf, MAX_STRING_LENGTH, "You render your wielder invisible to the human eye.\n\r");
		else if (itemtype == 52 || itemtype == 6)
			snprintf(buf, MAX_STRING_LENGTH, "You allow your wielder to walk through solid doors.\n\r");
		else if (itemtype == 54 || itemtype == 7)
			snprintf(buf, MAX_STRING_LENGTH, "You protect your wielder from evil.\n\r");
		else if (itemtype == 57 || itemtype == 8)
			snprintf(buf, MAX_STRING_LENGTH, "You protect your wielder in combat.\n\r");
		else if (itemtype == 9)
			snprintf(buf, MAX_STRING_LENGTH, "You allow your wielder to walk in complete silence.\n\r");
		else if (itemtype == 10)
			snprintf(buf, MAX_STRING_LENGTH, "You surround your wielder with a shield of #ll#wi#lg#wh#lt#wn#li#wn#lg#e.\n\r");
		else if (itemtype == 11)
			snprintf(buf, MAX_STRING_LENGTH, "You surround your wielder with a shield of #rf#Ri#rr#Re#e.\n\r");
		else if (itemtype == 12)
			snprintf(buf, MAX_STRING_LENGTH, "You surround your wielder with a shield of #ci#Cc#ce#e.\n\r");
		else if (itemtype == 13)
			snprintf(buf, MAX_STRING_LENGTH, "You surround your wielder with a shield of #ga#Gc#gi#Gd#e.\n\r");
		else
			snprintf(buf, MAX_STRING_LENGTH, "You are bugged...please report it.\n\r");
		if (itemtype > 0)
			send_to_char(buf, ch);
		break;

	case ITEM_ACCESSORY:
	case ITEM_LIGHT_ARMOR:
	case ITEM_MEDIUM_ARMOR:
	case ITEM_HEAVY_ARMOR:
		snprintf(buf, MAX_STRING_LENGTH, "Your armor is %d.\n\r", obj->value[0]);
		send_to_char(buf, ch);
		if (obj->value[3] < 1)
			break;
		if (obj->value[3] == 4 || obj->value[3] == 1)
			snprintf(buf, MAX_STRING_LENGTH, "You radiate an aura of darkness.\n\r");
		else if (obj->value[3] == 27 || obj->value[3] == 2)
			snprintf(buf, MAX_STRING_LENGTH, "You allow your wearer to see invisible things.\n\r");
		else if (obj->value[3] == 39 || obj->value[3] == 3)
			snprintf(buf, MAX_STRING_LENGTH, "You grant your wearer the power of flight.\n\r");
		else if (obj->value[3] == 45 || obj->value[3] == 4)
			snprintf(buf, MAX_STRING_LENGTH, "You allow your wearer to see in the dark.\n\r");
		else if (obj->value[3] == 46 || obj->value[3] == 5)
			snprintf(buf, MAX_STRING_LENGTH, "You render your wearer invisible to the human eye.\n\r");
		else if (obj->value[3] == 52 || obj->value[3] == 6)
			snprintf(buf, MAX_STRING_LENGTH, "You allow your wearer to walk through solid doors.\n\r");
		else if (obj->value[3] == 54 || obj->value[3] == 7)
			snprintf(buf, MAX_STRING_LENGTH, "You protect your wearer from evil.\n\r");
		else if (obj->value[3] == 57 || obj->value[3] == 8)
			snprintf(buf, MAX_STRING_LENGTH, "You protect your wearer in combat.\n\r");
		else if (obj->value[3] == 9)
			snprintf(buf, MAX_STRING_LENGTH, "You allow your wearer to walk in complete silence.\n\r");
		else if (obj->value[3] == 10)
			snprintf(buf, MAX_STRING_LENGTH, "You surround your wearer with a shield of #ll#wi#lg#wh#lt#wn#li#wn#lg#e.\n\r");
		else if (obj->value[3] == 11)
			snprintf(buf, MAX_STRING_LENGTH, "You surround your wearer with a shield of #rf#Ri#rr#Re#e.\n\r");
		else if (obj->value[3] == 12)
			snprintf(buf, MAX_STRING_LENGTH, "You surround your wearer with a shield of #ci#Cc#ce#e.\n\r");
		else if (obj->value[3] == 13)
			snprintf(buf, MAX_STRING_LENGTH, "You surround your wearer with a shield of #ga#Gc#gi#Gd#e.\n\r");
		else
			snprintf(buf, MAX_STRING_LENGTH, "You are bugged...please report it.\n\r");
		if (obj->value[3] > 0)
			send_to_char(buf, ch);
		break;
	}

	for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
	{
		if (paf->location != APPLY_NONE && paf->modifier != 0)
		{
			snprintf(buf, MAX_STRING_LENGTH, "You affect %s by %d.\n\r",
					 affect_loc_name(paf->location), paf->modifier);
			send_to_char(buf, ch);
		}
	}

	for (paf = obj->affected; paf != NULL; paf = paf->next)
	{
		if (paf->location != APPLY_NONE && paf->modifier != 0)
		{
			snprintf(buf, MAX_STRING_LENGTH, "You affect %s by %d.\n\r",
					 affect_loc_name(paf->location), paf->modifier);
			send_to_char(buf, ch);
		}
	}

	return;
}

void do_claninfo(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	one_argument(argument, arg, MAX_INPUT_LENGTH);
	/* int   iClan;
    int   ptotal=0;
    int   pktotal=0;
    int   temp;*/

	
	if (arg[0] == '\0')
	{
		send_to_char_formatted("Leader of Assamite............. None.\n\r", ch);
		send_to_char_formatted("Leader of Tremere............ None.\n\r", ch);
		send_to_char_formatted("Leader of Tzimisce.......... None.\n\r", ch);
		send_to_char_formatted("Leader of Ventrue............ None.\n\r", ch);
		send_to_char_formatted("Leader of Lasombra............ None.\n\r", ch);
		send_to_char_formatted("Leader of Toreador........... None.\n\r", ch);
		send_to_char_formatted("If you want anything, please contact your respective Leader.\n\r", ch);
		send_to_char_formatted("\n\r-------------------------------------\n\r", ch);

		send_to_char_formatted("CLAN               1   2   3   4   5   6   7 \n\r", ch);
		snprintf(buf, MAX_INPUT_LENGTH, "1 Assamite          --- %3d %3d %3d %3d %3d %3d \n\r", clan_infotable[1].pkills[2], clan_infotable[1].pkills[3], clan_infotable[1].pkills[4], clan_infotable[1].pkills[5], clan_infotable[1].pkills[6], clan_infotable[1].pkills[7]);
		send_to_char_formatted(buf, ch);
		snprintf(buf, MAX_INPUT_LENGTH, "2 Tzimisce       %3d --- %3d %3d %3d %3d %3d \n\r", clan_infotable[2].pkills[1], clan_infotable[2].pkills[3], clan_infotable[2].pkills[4], clan_infotable[2].pkills[5], clan_infotable[2].pkills[6], clan_infotable[2].pkills[7]);
		send_to_char_formatted(buf, ch);
		snprintf(buf, MAX_INPUT_LENGTH, "3 Ventrue         %3d %3d --- %3d %3d %3d %3d \n\r", clan_infotable[3].pkills[1], clan_infotable[3].pkills[2], clan_infotable[3].pkills[4], clan_infotable[3].pkills[5], clan_infotable[3].pkills[6], clan_infotable[3].pkills[7]);
		send_to_char_formatted(buf, ch);
		snprintf(buf, MAX_INPUT_LENGTH, "4 Tremere         %3d %3d %3d --- %3d %3d %3d \n\r", clan_infotable[4].pkills[1], clan_infotable[4].pkills[2], clan_infotable[4].pkills[3], clan_infotable[4].pkills[5], clan_infotable[4].pkills[6], clan_infotable[4].pkills[7]);
		send_to_char_formatted(buf, ch);
		snprintf(buf, MAX_INPUT_LENGTH, "5 Lasombra         %3d %3d %3d %3d --- %3d %3d \n\r", clan_infotable[5].pkills[1], clan_infotable[5].pkills[2], clan_infotable[5].pkills[3], clan_infotable[5].pkills[4], clan_infotable[5].pkills[6], clan_infotable[5].pkills[7]);
		send_to_char_formatted(buf, ch);
		snprintf(buf, MAX_INPUT_LENGTH, "6 Toreador        %3d %3d %3d %3d %3d --- %3d \n\r", clan_infotable[6].pkills[1], clan_infotable[6].pkills[2], clan_infotable[6].pkills[3], clan_infotable[6].pkills[4], clan_infotable[6].pkills[5], clan_infotable[6].pkills[7]);
		send_to_char_formatted(buf, ch);
		
	}
	if (!str_cmp(arg, "mobs"))
	{
		send_to_char_formatted("\n\r------------------------------------------\n\r", ch);
		send_to_char_formatted("CLAN	      Mobs Killed   Deaths by Mobs\n\r", ch);
		snprintf(buf, MAX_INPUT_LENGTH, "Assamite   %14ld %14d\n\r", clan_infotable[1].mkills, clan_infotable[1].mkilled);
		send_to_char_formatted(buf, ch);
		snprintf(buf, MAX_INPUT_LENGTH, "Tzimisce%14ld %14d\n\r", clan_infotable[2].mkills, clan_infotable[2].mkilled);
		send_to_char_formatted(buf, ch);
		snprintf(buf, MAX_INPUT_LENGTH, "Ventrue  %14ld %14d\n\r", clan_infotable[3].mkills, clan_infotable[3].mkilled);
		send_to_char_formatted(buf, ch);
		snprintf(buf, MAX_INPUT_LENGTH, "Tremere  %14ld %14d\n\r", clan_infotable[4].mkills, clan_infotable[4].mkilled);
		send_to_char_formatted(buf, ch);
		snprintf(buf, MAX_INPUT_LENGTH, "Lasombra  %14ld %14d\n\r", clan_infotable[5].mkills, clan_infotable[5].mkilled);
		send_to_char_formatted(buf, ch);
		snprintf(buf, MAX_INPUT_LENGTH, "Toreador %14ld %14d\n\r", clan_infotable[6].mkills, clan_infotable[6].mkilled);
		send_to_char_formatted(buf, ch);
	}
}

void do_affect(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	AFFECT_DATA *paf;

	if (!IS_NPC(ch) && (IS_EXTRA(ch, EXTRA_OSWITCH) || IS_HEAD(ch, LOST_HEAD)))
	{
		obj_score(ch, ch->pcdata->chobj);
		return;
	}

	send_to_char("                               -= AFFECTS =-\n\r", ch);
	send_to_char("\n\r", ch);
	send_to_char("-------------------------------------------------------------------------------\n\r", ch);
	if (ch->affected != NULL)
	{
		send_to_char("You are affected by:\n\r", ch);
		for (paf = ch->affected; paf != NULL; paf = paf->next)
		{
			snprintf(buf, MAX_STRING_LENGTH, "Spell: '%s'", skill_table[paf->type].name);
			send_to_char(buf, ch);

			if (ch->level >= 0)
			{
				snprintf(buf, MAX_STRING_LENGTH,
						 " modifies %s by %d for %d hours",
						 affect_loc_name(paf->location),
						 paf->modifier,
						 paf->duration);
				send_to_char(buf, ch);
			}

			send_to_char(".\n\r", ch);
		}
	}

	if (IS_AFFECTED(ch, AFF_HIDE))
		send_to_char("You are keeping yourself hidden from those around you.\n\r", ch);

	send_to_char("Shields :", ch);
	if (IS_ITEMAFF(ch, ITEMA_SHOCKSHIELD))
	{
		send_to_char(" #ll#wi#lg#wh#lt#wn#li#wn#lg#e", ch);
	}
	if (IS_ITEMAFF(ch, ITEMA_FIRESHIELD))
	{
		send_to_char(" #rf#Ri#rr#Re#e", ch);
	}
	if (IS_ITEMAFF(ch, ITEMA_ICESHIELD))
	{
		send_to_char(" #ci#Cc#ce#e", ch);
	}
	if (IS_ITEMAFF(ch, ITEMA_ACIDSHIELD))
	{
		send_to_char(" #ga#Gc#gi#Gd#e", ch);
	}
	send_to_char(".\n\r", ch);

	send_to_char("Item affects :", ch);
	if (IS_AFFECTED(ch, AFF_INFRARED))
		send_to_char(" Infravision", ch);
	if (IS_AFFECTED(ch, AFF_DETECT_INVIS))
		send_to_char(" SeeInvis", ch);
	if (IS_AFFECTED(ch, AFF_FLYING))
		send_to_char(" Fly", ch);
	if (IS_AFFECTED(ch, AFF_BLIND))
		send_to_char(" Blind", ch);
	if (IS_AFFECTED(ch, AFF_INVISIBLE))
		send_to_char(" Invis", ch);
	if (IS_AFFECTED(ch, AFF_PASS_DOOR))
		send_to_char(" Passdoor", ch);
	if (IS_AFFECTED(ch, AFF_PROTECT))
		send_to_char(" Protection", ch);
	if (IS_AFFECTED(ch, AFF_SANCTUARY))
		send_to_char(" #wSanct#e", ch);
	if (IS_AFFECTED(ch, AFF_SNEAK))
		send_to_char(" #ySneak#e", ch);
	if (IS_AFFECTED(ch, AFF_TRACKING))
		send_to_char(" Tracking", ch);
	send_to_char(".\n\r", ch);
	send_to_char("-------------------------------------------------------------------------------\n\r", ch);
}

bool canStance(CHAR_DATA *ch, int stance)
{
	switch (stance)
	{
        case STANCE_FALCON:
            return ch->stance[STANCE_BULL] >= 200 && ch->stance[STANCE_MONGOOSE] >= 200;
        case STANCE_SWALLOW:
            return ch->stance[STANCE_CRANE] >= 200 && ch->stance[STANCE_MONGOOSE] >= 200;
        case STANCE_COBRA:
            return ch->stance[STANCE_CRANE] >= 200 && ch->stance[STANCE_VIPER] >= 200;
        case STANCE_LION:
            return ch->stance[STANCE_BULL] >= 200 && ch->stance[STANCE_VIPER] >= 200;
        case STANCE_GRIZZLIE:
            return ch->stance[STANCE_BULL] >= 200 && ch->stance[STANCE_CRANE] >= 200;
        case STANCE_PANTHER:
            return ch->stance[STANCE_VIPER] >= 200 && ch->stance[STANCE_MONGOOSE] >= 200;
        default:
            return TRUE;
	}
}

int Get_Armor_Bonus(CHAR_DATA *ch)
{
	int armorBonus = 0;
	CLANDISC_DATA *disc;

	if(DiscIsActive(GetPlayerDiscByTier(ch, ANIMALISM, ANIMALISM_SUBSUME_THE_SPIRIT)))
	{
		armorBonus += (int)(ch->armor * 0.1);
	}

	if((disc = GetPlayerDiscByTier(ch, ANIMALISM, ANIMALISM_PACT_WITH_ANIMALS)) != NULL)
	{
		if(DiscIsActive(disc) && !str_cmp(disc->option, "Boar"))
		{
			armorBonus += (int)(ch->armor *0.1);
		}
	}


	return armorBonus;
}

int Get_Hitroll_Bonus(CHAR_DATA *ch)
{
    int hitrollBonus = 0;
    CLANDISC_DATA *disc;

    if((disc = GetPlayerDiscByTier(ch, CELERITY, CELERITY_PRECISION)) != NULL)
    {
        if(DiscIsActive(disc))
        {
            hitrollBonus += (int) (ch->hitroll * 0.25);
        }
    }

    if((disc = GetPlayerDiscByTier(ch, AUSPEX, AUSPEX_HEIGHTENED_SENSES)) != NULL)
    {
        if(DiscIsActive(disc) && !str_cmp( disc->option, "Touch"))
        {
            hitrollBonus += (int) (ch->hitroll * 0.1);
        }
    }

    return hitrollBonus;
}

int Get_Damroll_Bonus(CHAR_DATA *ch)
{
    int damrollBonus = 0;
    CLANDISC_DATA *disc;

    if((disc = GetPlayerDiscByTier(ch, AUSPEX, AUSPEX_HEIGHTENED_SENSES)) != NULL)
    {
        if(DiscIsActive(disc) && !str_cmp( disc->option, "Touch"))
        {
            damrollBonus += (int) (ch->damroll * 0.1);
        }
    }

    return damrollBonus ;
}

/**
 * Returns a string "str" centered in string of a length width "new_length".
 * Padding is done using the specified fill character "placeholder".
 */
char * str_center(char str[], unsigned int new_length, char placeholder)
{
	size_t str_length = strlen(str);

	// if a new length is less or equal length of the original string, returns the original string
	if (new_length <= str_length)
		return str;

	char *buffer;
	unsigned int i, total_rest_length;

	buffer = malloc(sizeof(char) * new_length);

	// length of a wrapper of the original string
	total_rest_length = new_length - str_length;

	// write a prefix to buffer
	i = 0;
	while (i < (total_rest_length / 2)) {
		buffer[i] = placeholder;
		++i;
	}
	buffer[i + 1] = '\0';

	// write the original string
	strcat(buffer, str);

	// write a postfix to the buffer
	i += str_length;
	while (i < new_length) {
		buffer[i] = placeholder;
		++i;
	}
	buffer[i + 1] = '\0';

	return buffer;
}
