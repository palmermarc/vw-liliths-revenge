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

/*
* Local functions.
*/

void add_to_history args((CHANNEL_DATA * channel_history, const char *information));
bool is_note_to args((CHAR_DATA * ch, NOTE_DATA *pnote));
void note_attach args((CHAR_DATA * ch));
void note_remove args((CHAR_DATA * ch, NOTE_DATA *pnote));
void talk_channel args((CHAR_DATA * ch, char *argument,
						int channel, const char *verb));
bool is_in args((char *, char *));
bool all_in args((char *, char *));

/* functions declared elsewhere that we are interested in */
extern void call_all args((CHAR_DATA * ch));

bool is_note_to(CHAR_DATA *ch, NOTE_DATA *pnote)
{
	if (!str_cmp(ch->name, pnote->sender))
		return TRUE;

	if (is_name("all", pnote->to_list))
		return TRUE;

	if (IS_IMMORTAL(ch) && is_name("imm", pnote->to_list))
		return TRUE;

	if (IS_IMMORTAL(ch) && is_name("immortal", pnote->to_list))
		return TRUE;

	if (is_name(ch->name, pnote->to_list))
		return TRUE;

	return FALSE;
}

void note_attach(CHAR_DATA *ch)
{
	NOTE_DATA *pnote;

	if (ch->pnote != NULL)
		return;

	if (note_free == NULL)
	{
		pnote = alloc_perm(sizeof(*ch->pnote));
	}
	else
	{
		pnote = note_free;
		note_free = note_free->next;
	}

	pnote->next = NULL;
	pnote->sender = str_dup(ch->name);
	pnote->date = str_dup("");
	pnote->to_list = str_dup("");
	pnote->subject = str_dup("");
	pnote->text = str_dup("");
	ch->pnote = pnote;
	return;
}

void note_remove(CHAR_DATA *ch, NOTE_DATA *pnote)
{
	char to_new[MAX_INPUT_LENGTH];
	char to_one[MAX_INPUT_LENGTH];
	FILE *fp;
	NOTE_DATA *prev;
	char *to_list;

	/*
    * Build a new to_list.
    * Strip out this recipient.
    */
	to_new[0] = '\0';
	to_list = pnote->to_list;
	while (*to_list != '\0')
	{
		to_list = one_argument(to_list, to_one, MAX_INPUT_LENGTH);
		if (to_one[0] != '\0' && str_cmp(ch->name, to_one))
		{
			strncat(to_new, " ", MAX_INPUT_LENGTH - strlen(to_new));
			strncat(to_new, to_one, MAX_INPUT_LENGTH - strlen(to_new));
		}
	}

	/*
    * Just a simple recipient removal?
    */
	if (str_cmp(ch->name, pnote->sender) && to_new[0] != '\0')
	{
		free_string(pnote->to_list);
		pnote->to_list = str_dup(to_new + 1);
		return;
	}

	/*
    * Remove note from linked list.
    */
	if (pnote == note_list)
	{
		note_list = pnote->next;
	}
	else
	{
		for (prev = note_list; prev != NULL; prev = prev->next)
		{
			if (prev->next == pnote)
				break;
		}

		if (prev == NULL)
		{
			bug("Note_remove: pnote not found.", 0);
			return;
		}

		prev->next = pnote->next;
	}

	free_string(pnote->text);
	free_string(pnote->subject);
	free_string(pnote->to_list);
	free_string(pnote->date);
	free_string(pnote->sender);
	pnote->next = note_free;
	note_free = pnote;

	/*
    * Rewrite entire list.
    */
	fclose(fpReserve);
	if ((fp = fopen(NOTE_FILE, "w")) == NULL)
	{
		perror(NOTE_FILE);
	}
	else
	{
		for (pnote = note_list; pnote != NULL; pnote = pnote->next)
		{
			fprintf(fp, "Sender  %s~\nDate    %s~\nTo      %s~\nSubject %s~\nText\n%s~\n\n",
					pnote->sender,
					pnote->date,
					pnote->to_list,
					pnote->subject,
					pnote->text);
		}
		fclose(fp);
	}
	fpReserve = fopen(NULL_FILE, "r");
	return;
}

void do_note(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	NOTE_DATA *pnote;
	int vnum;
	int anum;

	if (IS_NPC(ch))
		return;

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);
	smash_tilde(argument);

	if (!str_cmp(arg, "list"))
	{
		vnum = 0;
		for (pnote = note_list; pnote != NULL; pnote = pnote->next)
		{
			if (is_note_to(ch, pnote))
			{
				snprintf(buf, MAX_STRING_LENGTH, "[%3d] %s: %s\n\r",
						 vnum, pnote->sender, pnote->subject);
				send_to_char_formatted(buf, ch);
				vnum++;
			}
		}
		return;
	}

	if (!str_cmp(arg, "read"))
	{
		bool fAll;

		if (!str_cmp(argument, "all"))
		{
			fAll = TRUE;
			anum = 0;
		}
		else if (is_number(argument))
		{
			fAll = FALSE;
			anum = atoi(argument);
		}
		else
		{
			send_to_char_formatted("Note read which number?\n\r", ch);
			return;
		}

		vnum = 0;
		for (pnote = note_list; pnote != NULL; pnote = pnote->next)
		{
			if (is_note_to(ch, pnote) && (vnum++ == anum || fAll))
			{
				snprintf(buf, MAX_STRING_LENGTH, "[%3d] %s: %s\n\r%s\n\rTo: %s\n\r",
						 vnum - 1,
						 pnote->sender,
						 pnote->subject,
						 pnote->date,
						 pnote->to_list);
				send_to_char_formatted(buf, ch);
				send_to_char_formatted(pnote->text, ch);
				return;
			}
		}

		send_to_char("No such note.\n\r", ch);
		return;
	}

	if (!str_cmp(arg, "+"))
	{
		note_attach(ch);
		strncpy(buf, ch->pnote->text, MAX_STRING_LENGTH);
		if (strlen(buf) + strlen(argument) >= MAX_STRING_LENGTH - 4)
		{
			send_to_char_formatted("Note too long.\n\r", ch);
			return;
		}

		strncat(buf, argument, MAX_STRING_LENGTH - strlen(buf));
		strncat(buf, "\n\r", MAX_STRING_LENGTH - strlen(buf));
		free_string(ch->pnote->text);
		ch->pnote->text = str_dup(buf);
		send_to_char_formatted("Ok.\n\r", ch);
		return;
	}

	if (!str_cmp(arg, "subject"))
	{
		note_attach(ch);
		free_string(ch->pnote->subject);
		ch->pnote->subject = str_dup(argument);
		send_to_char_formatted("Ok.\n\r", ch);
		return;
	}

	if (!str_cmp(arg, "to"))
	{
		note_attach(ch);
		free_string(ch->pnote->to_list);
		ch->pnote->to_list = str_dup(argument);
		send_to_char_formatted("Ok.\n\r", ch);
		return;
	}

	if (!str_cmp(arg, "clear"))
	{
		if (ch->pnote != NULL)
		{
			free_string(ch->pnote->text);
			free_string(ch->pnote->subject);
			free_string(ch->pnote->to_list);
			free_string(ch->pnote->date);
			free_string(ch->pnote->sender);
			ch->pnote->next = note_free;
			note_free = ch->pnote;
			ch->pnote = NULL;
		}

		send_to_char_formatted("Ok.\n\r", ch);
		return;
	}

	if (!str_cmp(arg, "show"))
	{
		if (ch->pnote == NULL)
		{
			send_to_char_formatted("You have no note in progress.\n\r", ch);
			return;
		}

		snprintf(buf, MAX_STRING_LENGTH, "%s: %s\n\rTo: %s\n\r",
				 ch->pnote->sender,
				 ch->pnote->subject,
				 ch->pnote->to_list);
		send_to_char_formatted(buf, ch);
		send_to_char_formatted(ch->pnote->text, ch);
		return;
	}

	if (!str_cmp(arg, "post"))
	{
		FILE *fp;
		char *strtime;

		if (ch->pnote == NULL)
		{
			send_to_char_formatted("You have no note in progress.\n\r", ch);
			return;
		}

		ch->pnote->next = NULL;
		strtime = ctime(&current_time);
		strtime[strlen(strtime) - 1] = '\0';
		ch->pnote->date = str_dup(strtime);

		if (note_list == NULL)
		{
			note_list = ch->pnote;
		}
		else
		{
			for (pnote = note_list; pnote->next != NULL; pnote = pnote->next)
				;
			pnote->next = ch->pnote;
		}
		pnote = ch->pnote;
		ch->pnote = NULL;

		fclose(fpReserve);
		if ((fp = fopen(NOTE_FILE, "a")) == NULL)
		{
			perror(NOTE_FILE);
		}
		else
		{
			fprintf(fp, "Sender  %s~\nDate    %s~\nTo      %s~\nSubject %s~\nText\n%s~\n\n",
					pnote->sender,
					pnote->date,
					pnote->to_list,
					pnote->subject,
					pnote->text);
			fclose(fp);
		}
		fpReserve = fopen(NULL_FILE, "r");

		send_to_char_formatted("Ok.\n\r", ch);
		return;
	}

	if (!str_cmp(arg, "remove"))
	{
		if (!is_number(argument))
		{
			send_to_char_formatted("Note remove which number?\n\r", ch);
			return;
		}

		anum = atoi(argument);
		vnum = 0;
		for (pnote = note_list; pnote != NULL; pnote = pnote->next)
		{
			if (is_note_to(ch, pnote) && vnum++ == anum)
			{
				note_remove(ch, pnote);
				send_to_char_formatted("Ok.\n\r", ch);
				return;
			}
		}

		send_to_char_formatted("No such note.\n\r", ch);
		return;
	}

	send_to_char_formatted("Huh?  Type 'help note' for usage.\n\r", ch);
	return;
}

/*
* Generic channel function.
*/
void talk_channel(CHAR_DATA *ch, char *argument, int channel, const char *verb)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	int position;

	if (argument[0] == '\0')
	{
		snprintf(buf, MAX_STRING_LENGTH, "%s what?\n\r", verb);
		buf[0] = UPPER(buf[0]);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_CONE_OF_SILENCE) && ch->level < LEVEL_SEER)
	{
		snprintf(buf, MAX_STRING_LENGTH, "You are in a room that is silenced!\n\r");
		send_to_char(buf, ch);
		return;
	}

	if (IS_HEAD(ch, LOST_TONGUE))
	{
		snprintf(buf, MAX_STRING_LENGTH, "You can't %s without a tongue!\n\r", verb);
		send_to_char(buf, ch);
		return;
	}

	if (IS_BODY(ch, GAGGED))
	{
		snprintf(buf, MAX_STRING_LENGTH, "You can't %s with a gag on!\n\r", verb);
		send_to_char(buf, ch);
		return;
	}

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE))
	{
		snprintf(buf, MAX_STRING_LENGTH, "You can't %s.\n\r", verb);
		send_to_char(buf, ch);
		return;
	}

	REMOVE_BIT(ch->deaf, channel);

	switch (channel)
	{
	default:
		snprintf(buf, MAX_STRING_LENGTH, "You %s '%s'.\n\r", verb, argument);
		send_to_char(buf, ch);

		snprintf(buf, MAX_STRING_LENGTH, "$n %ss '$t'.", verb);
		snprintf(buf2, MAX_STRING_LENGTH, "$n %ss '$t'.", verb);
		break;
	case CHANNEL_CHAT:
		snprintf(buf, MAX_STRING_LENGTH, "#lYou %s '%s'.\n\r", verb, argument);
		send_to_char(buf, ch);
		if(ch->pcdata != NULL)
		{
			add_to_history(ch->pcdata->chat_history, buf);
		}

		snprintf(buf, MAX_STRING_LENGTH, "~C#l$n %ss '$t'.", verb);
		snprintf(buf2, MAX_STRING_LENGTH, "~C#l$n %ss '$t'.", verb);
		break;
	case CHANNEL_IMMTALK:
		snprintf(buf, MAX_STRING_LENGTH, "~i#w[Immortal] $n: $t.");
		snprintf(buf2, MAX_STRING_LENGTH, "~i#w[Immortal] $n: $t.");
		position = ch->position;
		ch->position = POS_STANDING;
		act(buf, ch, argument, NULL, TO_CHAR);
		ch->position = position;
		break;

	case CHANNEL_JUSTITALK:
		snprintf(buf, MAX_STRING_LENGTH, "[Justicar]:[$n] $t.");
		snprintf(buf2, MAX_STRING_LENGTH, "[Justicar]:[$n] $t.");
		position = ch->position;
		ch->position = POS_STANDING;
		act(buf, ch, argument, NULL, TO_CHAR);
		ch->position = position;
		break;

	case CHANNEL_NEWBIE:
		snprintf(buf, MAX_STRING_LENGTH, "~n[NEWBIE]:[$n] $t.");
		snprintf(buf2, MAX_STRING_LENGTH, "~n[NEWBIE]:[$n] $t.");
		position = ch->position;
		ch->position = POS_STANDING;
		act(buf, ch, argument, NULL, TO_CHAR);
		ch->position = position;
		break;

	case CHANNEL_PERSONAL:
		snprintf(buf, MAX_STRING_LENGTH, "$n--> $t.");
		snprintf(buf2, MAX_STRING_LENGTH, "$n--> $t.");
		position = ch->position;
		ch->position = POS_STANDING;
		act(buf, ch, argument, NULL, TO_CHAR);
		ch->position = position;
		break;

	case CHANNEL_NOSTALK:

		if (!IS_SET(ch->act, PLR_VAMPIRE))
			return;

		if (!IS_NPC(ch) && ch->vampgen == 1)
		{
			strncpy(buf, "~c<[$n]> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		else if (!IS_NPC(ch) && ch->vampgen == 2)
		{
			strncpy(buf, "~c<<$n>> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		else if (!IS_NPC(ch) && ch->vampgen >= 3)
		{
			strncpy(buf, "~c<$n> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		else
		{
			send_to_char("huh?\n\r", ch);
			return;
		}

		position = ch->position;
		ch->position = POS_STANDING;
		act(buf, ch, argument, NULL, TO_CHAR);
		ch->position = position;
		break;

	case CHANNEL_BRUTALK:
		if (!IS_NPC(ch) && ch->vampgen == 1)
		{
			strncpy(buf, "~c<[$n]> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		else if (!IS_NPC(ch) && ch->vampgen == 2)
		{
			strncpy(buf, "~c<<$n>> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		else
		{
			strncpy(buf, "~c<$n> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		position = ch->position;
		ch->position = POS_STANDING;
		act(buf, ch, argument, NULL, TO_CHAR);
		ch->position = position;
		break;

	case CHANNEL_MALKTALK:
		if (!IS_NPC(ch) && ch->vampgen == 1)
		{
			strncpy(buf, "~c<[$n]> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		else if (!IS_NPC(ch) && ch->vampgen == 2)
		{
			strncpy(buf, "~c<<$n>> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		else
		{
			strncpy(buf, "~c<$n> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		position = ch->position;
		ch->position = POS_STANDING;
		act(buf, ch, argument, NULL, TO_CHAR);
		ch->position = position;
		break;

	case CHANNEL_VENTALK:
		if (!IS_NPC(ch) && ch->vampgen == 1)
		{
			strncpy(buf, "~c<[$n]> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		else if (!IS_NPC(ch) && ch->vampgen == 2)
		{
			strncpy(buf, "~c<<$n>> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		else
		{
			strncpy(buf, "~c<$n> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		position = ch->position;
		ch->position = POS_STANDING;
		act(buf, ch, argument, NULL, TO_CHAR);
		ch->position = position;
		break;

	case CHANNEL_TREMTALK:
		if (!IS_NPC(ch) && ch->vampgen == 1)
		{
			strncpy(buf, "~c<[$n]> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		else if (!IS_NPC(ch) && ch->vampgen == 2)
		{
			strncpy(buf, "~c<<$n>> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		else
		{
			strncpy(buf, "~c<$n> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		position = ch->position;
		ch->position = POS_STANDING;
		act(buf, ch, argument, NULL, TO_CHAR);
		ch->position = position;
		break;

	case CHANNEL_LASTALK:
		if (!IS_NPC(ch) && ch->vampgen == 1)
		{
			strncpy(buf, "~c<[$n]> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		else if (!IS_NPC(ch) && ch->vampgen == 2)
		{
			strncpy(buf, "~c<<$n>> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		else
		{
			strncpy(buf, "~c<$n> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		position = ch->position;
		ch->position = POS_STANDING;
		act(buf, ch, argument, NULL, TO_CHAR);
		ch->position = position;
		break;

	case CHANNEL_TORTALK:
		if (!IS_NPC(ch) && ch->vampgen == 1)
		{
			strncpy(buf, "~c<[$n]> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		else if (!IS_NPC(ch) && ch->vampgen == 2)
		{
			strncpy(buf, "~c<<$n>> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		else
		{
			strncpy(buf, "~c<$n> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		position = ch->position;
		ch->position = POS_STANDING;
		act(buf, ch, argument, NULL, TO_CHAR);
		ch->position = position;
		break;

	case CHANNEL_CAPTALK:
		if (!IS_NPC(ch) && ch->vampgen == 1)
		{
			strncpy(buf, "~c<[$n]> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		else if (!IS_NPC(ch) && ch->vampgen == 2)
		{
			strncpy(buf, "~c<<$n>> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		else
		{
			strncpy(buf, "~c<$n> $t.", MAX_STRING_LENGTH);
			strncpy(buf2, "", MAX_STRING_LENGTH);
		}
		position = ch->position;
		ch->position = POS_STANDING;
		act(buf, ch, argument, NULL, TO_CHAR);
		ch->position = position;
		break;

	case CHANNEL_VAMPTALK:
		if (!IS_NPC(ch) && ch->vampgen == 1)
		{
			snprintf(buf, MAX_STRING_LENGTH, "[Kindred]:<[$n]> $t.");
			snprintf(buf2, MAX_STRING_LENGTH, "[Kindred]:<[$n]> $t.");
		}
		else if (!IS_NPC(ch) && ch->vampgen == 2)
		{
			snprintf(buf, MAX_STRING_LENGTH, "[Kindred]:<<$n>> $t.");
			snprintf(buf2, MAX_STRING_LENGTH, "[Kindred]:[[$n]] $t.");
		}
		else
		{
			snprintf(buf, MAX_STRING_LENGTH, "[Kindred]:<$n> $t.");
			snprintf(buf2, MAX_STRING_LENGTH, "[Kindred]:[$n] $t.");
		}
		position = ch->position;
		ch->position = POS_STANDING;
		act(buf, ch, argument, NULL, TO_CHAR);
		ch->position = position;
		break;
	}

	for (d = descriptor_list; d != NULL; d = d->next)
	{
		CHAR_DATA *och;
		CHAR_DATA *vch;

		och = d->original ? d->original : d->character;
		vch = d->character;

		if (d->connected == CON_PLAYING && vch != ch && !IS_SET(och->deaf, channel) && !IS_SET(och->in_room->room_flags, ROOM_QUIET))
		{
			if (channel == CHANNEL_NEWBIE)
				continue;
			if (channel == CHANNEL_PERSONAL && !IS_EXTRA(och, EXTRA_PERSONAL))
				continue;

			if (channel == CHANNEL_IMMTALK && !IS_IMMORTAL(och))
				continue;
			if (channel == CHANNEL_JUSTITALK && och->level < LEVEL_JUSTICAR)
				continue;
			if (channel == CHANNEL_VAMPTALK && (!IS_NPC(och) && !IS_SET(och->act, PLR_VAMPIRE)) && !IS_SET(och->in_room->room_flags, ROOM_QUIET))
				continue;

			if (channel == CHANNEL_NOSTALK && (!IS_NPC(och) && !IS_SET(och->act, PLR_VAMPIRE)) && !IS_SET(och->in_room->room_flags, ROOM_QUIET))
				continue;

			if (channel == CHANNEL_LASTALK && (!IS_NPC(och) && !IS_SET(och->act, PLR_VAMPIRE)) && !IS_SET(och->in_room->room_flags, ROOM_QUIET))
				continue;

			if (channel == CHANNEL_TORTALK && (!IS_NPC(och) && !IS_SET(och->act, PLR_VAMPIRE)) && !IS_SET(och->in_room->room_flags, ROOM_QUIET))
				continue;

			if (channel == CHANNEL_TREMTALK && (!IS_NPC(och) && !IS_SET(och->act, PLR_VAMPIRE)) && !IS_SET(och->in_room->room_flags, ROOM_QUIET))
				continue;
			if (channel == CHANNEL_MALKTALK && (!IS_NPC(och) && !IS_SET(och->act, PLR_VAMPIRE)) && !IS_SET(och->in_room->room_flags, ROOM_QUIET))
				continue;

			if (channel == CHANNEL_VENTALK && (!IS_NPC(och) && !IS_SET(och->act, PLR_VAMPIRE)) && !IS_SET(och->in_room->room_flags, ROOM_QUIET))
				continue;

			if (channel == CHANNEL_CAPTALK && (!IS_NPC(och) && !IS_SET(och->act, PLR_VAMPIRE)) && !IS_SET(och->in_room->room_flags, ROOM_QUIET))
				continue;

			if (channel == CHANNEL_BRUTALK && (!IS_NPC(och) && !IS_SET(och->act, PLR_VAMPIRE)) && !IS_SET(och->in_room->room_flags, ROOM_QUIET))
				continue;

			if (channel == CHANNEL_YELL && vch->in_room->area != ch->in_room->area)
				continue;

			if (str_cmp(ch->name, "Palmer") && IS_SET(ch->extra, EXTRA_ORGYMEMBER) && !IS_SET(och->extra, EXTRA_ORGYMEMBER))
				continue;

			if (str_cmp(och->name, "Palmer") && !IS_SET(ch->extra, EXTRA_ORGYMEMBER) && IS_SET(och->extra, EXTRA_ORGYMEMBER))
				continue;

			position = vch->position;
			if (channel != CHANNEL_SHOUT && channel != CHANNEL_YELL)
				vch->position = POS_STANDING;
			if (!IS_NPC(vch) && IS_SET(vch->act, PLR_VAMPIRE) && str_cmp(ch->clan, vch->clan))
			{
				act(buf2, ch, argument, vch, TO_VICT);
				vch->position = position;
				continue;
			}
			act(buf, ch, argument, vch, TO_VICT);
			vch->position = position;
		}
	}

	return;
}

void do_auction(CHAR_DATA *ch, char *argument)
{
	talk_channel(ch, argument, CHANNEL_AUCTION, "auction");
	return;
}

void do_chat(CHAR_DATA *ch, char *argument)
{
	talk_channel(ch, argument, CHANNEL_CHAT, "chat");
	return;
}

void do_mchat(CHAR_DATA *ch, char *argument)
{
	talk_channel(ch, argument, CHANNEL_MCHAT, "mchat");
	return;
}

void do_newbie(CHAR_DATA *ch, char *argument)
{
	talk_channel(ch, argument, CHANNEL_NEWBIE, "newbie");
	return;
}

void do_pers(CHAR_DATA *ch, char *argument)
{
	if (IS_EXTRA(ch, EXTRA_PERSONAL))
		talk_channel(ch, argument, CHANNEL_PERSONAL, "personal");
	else
		send_to_char("Huh?\n\r", ch);
	return;
}
/*
* Alander's new channels.
*/

void do_music(CHAR_DATA *ch, char *argument)
{
	talk_channel(ch, argument, CHANNEL_MUSIC, "music");
	return;
}

void do_justitalk(CHAR_DATA *ch, char *argument)
{
	/*   -- REMOVED FOR NOW, changed from vradio .. to be used soon -- code done */
	talk_channel(ch, argument, CHANNEL_JUSTITALK, "justitalk");

	return;
}

void do_question(CHAR_DATA *ch, char *argument)
{
	talk_channel(ch, argument, CHANNEL_QUESTION, "question");
	return;
}

void do_bid(CHAR_DATA *ch, char *argument)
{
	talk_channel(ch, argument, CHANNEL_BID, "bid");
	return;
}

void do_answer(CHAR_DATA *ch, char *argument)
{
	talk_channel(ch, argument, CHANNEL_QUESTION, "answer");
	return;
}

void do_shout(CHAR_DATA *ch, char *argument)
{
	talk_channel(ch, argument, CHANNEL_SHOUT, "shout");
	WAIT_STATE(ch, 12);
	return;
}

void do_yell(CHAR_DATA *ch, char *argument)
{
	talk_channel(ch, argument, CHANNEL_YELL, "yell");
	return;
}

void do_immtalk(CHAR_DATA *ch, char *argument)
{
	talk_channel(ch, argument, CHANNEL_IMMTALK, "immtalk");
	return;
}

void do_assamitetalk(CHAR_DATA *ch, char *argument)
{
	if (!IS_NPC(ch) && (!strncmp(ch->clan, "Assamite", 4)))
		talk_channel(ch, argument, CHANNEL_ASSTALK, "ASSTALK");
	else
		send_to_char("You are not a member of that clan.\n\r", ch);
	return;
}

void do_tzimiscetalk(CHAR_DATA *ch, char *argument)
{
	if (!IS_NPC(ch) && (!strncmp(ch->clan, "Tzimisce", 4)))
		talk_channel(ch, argument, CHANNEL_TZITALK, "TZITALK");
	else
		send_to_char("You are not a member of that clan.\n\r", ch);
	return;
}

void do_ventruetalk(CHAR_DATA *ch, char *argument)
{
	if (!IS_NPC(ch) && (!strncmp(ch->clan, "Ventrue", 4)))
		talk_channel(ch, argument, CHANNEL_VENTALK, "VENTALK");
	else
		send_to_char("You are not a member of that clan.\n\r", ch);
	return;
}

void do_lasombratalk(CHAR_DATA *ch, char *argument)
{
	if (!IS_NPC(ch) && (!strncmp(ch->clan, "Lasombra", 4)))
		talk_channel(ch, argument, CHANNEL_LASTALK, "LASTALK");
	else
		send_to_char("You are not a member of that clan.\n\r", ch);
	return;
}

void do_toreadortalk(CHAR_DATA *ch, char *argument)
{
	if (!IS_NPC(ch) && (!strncmp(ch->clan, "Toreador", 4)))
		talk_channel(ch, argument, CHANNEL_TORTALK, "TORTALK");
	else
		send_to_char("You are not a member of that clan.\n\r", ch);
	return;
}

void do_tremeretalk(CHAR_DATA *ch, char *argument)
{
	if (!IS_NPC(ch) && (!strncmp(ch->clan, "Tremere", 4)))
		talk_channel(ch, argument, CHANNEL_TRETALK, "TRETALK");
	else
		send_to_char("You are not a member of that clan.\n\r", ch);
	return;
}

void do_vamptalk(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch) || (!IS_NPC(ch) && !IS_SET(ch->act, PLR_VAMPIRE)))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
	talk_channel(ch, argument, CHANNEL_VAMPTALK, "vamptalk");
	return;
}

void do_say(CHAR_DATA *ch, char *argument)
{
	char name[80];
	char poly[MAX_INPUT_LENGTH];
	char speak[10];
	char speaks[10];
	CHAR_DATA *to;
	bool is_ok;

	if (IS_HEAD(ch, LOST_TONGUE))
	{
		send_to_char("You can't speak without a tongue!\n\r", ch);
		return;
	}
	if (IS_BODY(ch, GAGGED))
	{
		send_to_char("You can't speak with a gag on!\n\r", ch);
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char("Say what?\n\r", ch);
		return;
	}

	if (IS_BODY(ch, CUT_THROAT))
	{
		snprintf(speak, 10, "rasp");
		snprintf(speaks, 10, "rasps");
	}
	else
	{
		snprintf(speak, 10, "say");
		snprintf(speaks, 10, "says");
	}
	snprintf(poly, MAX_INPUT_LENGTH, "You %s '$T'.", speak);
	act(poly, ch, NULL, argument, TO_CHAR);

	snprintf(poly, MAX_INPUT_LENGTH, "$n %s '$T'.", speaks);

	if (ch->in_room->vnum != ROOM_VNUM_IN_OBJECT)
	{
		act(poly, ch, NULL, argument, TO_ROOM);
		room_text(ch, strlower(argument));
		return;
	}

	to = ch->in_room->people;
	for (; to != NULL; to = to->next_in_room)
	{
		is_ok = FALSE;

		if (to->desc == NULL || !IS_AWAKE(to))
			continue;

		if (ch == to)
			continue;

		if (!IS_NPC(ch) && ch->pcdata->chobj != NULL &&
			ch->pcdata->chobj->in_room != NULL &&
			!IS_NPC(to) && to->pcdata->chobj != NULL &&
			to->pcdata->chobj->in_room != NULL &&
			ch->in_room == to->in_room)
			is_ok = TRUE;
		else
			is_ok = FALSE;

		if (!IS_NPC(ch) && ch->pcdata->chobj != NULL &&
			ch->pcdata->chobj->in_obj != NULL &&
			!IS_NPC(to) && to->pcdata->chobj != NULL &&
			to->pcdata->chobj->in_obj != NULL &&
			ch->pcdata->chobj->in_obj == to->pcdata->chobj->in_obj)
			is_ok = TRUE;
		else
			is_ok = FALSE;

		if (!is_ok)
			continue;

		if (IS_NPC(ch))
			snprintf(name, 80, "%s", ch->short_descr);
		else if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_POLYMORPH))
			snprintf(name, 80, "%s", ch->morph);
		else
			snprintf(name, 80, "%s", ch->name);
		name[0] = UPPER(name[0]);
		snprintf(poly, MAX_INPUT_LENGTH, "%s %s '%s'.\n\r", name, speaks, argument);
		send_to_char(poly, to);
	}

	room_text(ch, strlower(argument));
	return;
}

void room_text(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	CHAR_DATA *mob;
	OBJ_DATA *obj;
	ROOMTEXT_DATA *rt;
	bool mobfound;
	bool hop;

	char arg[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	for (rt = ch->in_room->roomtext; rt != NULL; rt = rt->next)
	{
		if (!strcmp(argument, rt->input) || is_in(argument, rt->input) || all_in(argument, rt->input))
		{
			if (rt->name != NULL && str_cmp(rt->name, "\0") && str_cmp(rt->name, "all") && str_cmp(rt->name, "|all*"))
			{
				if (!is_in(ch->name, rt->name))
					continue;
			}

			mobfound = TRUE;
			if (rt->mob != 0)
			{
				mobfound = FALSE;
				for (vch = char_list; vch != NULL; vch = vch_next)
				{
					vch_next = vch->next;
					if (vch->in_room == NULL)
						continue;
					if (!IS_NPC(vch))
						continue;
					if (vch->in_room == ch->in_room && vch->pIndexData->vnum == rt->mob)
					{
						mobfound = TRUE;
						break;
					}
				}
			}
			if (!mobfound)
				continue;
			hop = FALSE;
			switch (rt->type % RT_RETURN)
			{
			case RT_SAY:
				break;
			case RT_LIGHTS:
				do_changelight(ch, "");
				break;
			case RT_LIGHT:
				REMOVE_BIT(ch->in_room->room_flags, ROOM_DARK);
				break;
			case RT_DARK:
				SET_BIT(ch->in_room->room_flags, ROOM_DARK);
				break;
			case RT_OBJECT:
				if (get_obj_index(rt->power) == NULL)
					return;
				obj = create_object(get_obj_index(rt->power), ch->level);
				if (IS_SET(rt->type, RT_TIMER))
					obj->timer = 1;
				if (CAN_WEAR(obj, ITEM_TAKE))
					obj_to_char(obj, ch);
				else
					obj_to_room(obj, ch->in_room);
				if (!str_cmp(rt->choutput, "copy"))
					act(rt->output, ch, obj, NULL, TO_CHAR);
				else
					act(rt->choutput, ch, obj, NULL, TO_CHAR);
				if (!IS_SET(rt->type, RT_PERSONAL))
					act(rt->output, ch, obj, NULL, TO_ROOM);
				hop = TRUE;
				break;
			case RT_MOBILE:
				if (get_mob_index(rt->power) == NULL)
					return;
				mob = create_mobile(get_mob_index(rt->power));
				char_to_room(mob, ch->in_room);
				if (!str_cmp(rt->choutput, "copy"))
					act(rt->output, ch, NULL, mob, TO_CHAR);
				else
					act(rt->choutput, ch, NULL, mob, TO_CHAR);
				if (!IS_SET(rt->type, RT_PERSONAL))
					act(rt->output, ch, NULL, mob, TO_ROOM);
				hop = TRUE;
				break;
			case RT_SPELL:
				(*skill_table[rt->power].spell_fun)(rt->power, number_range(20, 30), ch, ch);
				break;
			case RT_PORTAL:
				if (get_obj_index(OBJ_VNUM_PORTAL) == NULL)
					return;
				obj = create_object(get_obj_index(OBJ_VNUM_PORTAL), 0);
				obj->timer = 1;
				obj->value[0] = rt->power;
				obj->value[1] = 1;
				obj_to_room(obj, ch->in_room);
				break;
			case RT_TELEPORT:
				if (get_room_index(rt->power) == NULL)
					return;
				if (IS_SET(rt->type, RT_GODPASS) &&
					!IS_SET(ch->act, PLR_GODPASS))
				{
					send_to_char("The Gods Have Not Yet Deemed You Worthy!\n\r", ch);
					return;
				}
				if (!IS_NPC(ch) && ch->in_room->vnum == 3001 &&
					get_age(ch) >= 28)
				{
					send_to_char("You are NOT a newbie and therefore cannot access that area!\n\r", ch);
					return;
				}
				if (
					((ch->in_room->sector_type == SECT_VENTRUE) &&
					 (str_cmp(ch->clan, "Ventrue"))) ||

					((ch->in_room->sector_type == SECT_ASSASMITE) &&
					 (str_cmp(ch->clan, "Assamite"))) ||

					((ch->in_room->sector_type == SECT_TREMERE) &&
					 (str_cmp(ch->clan, "Tremere"))) ||

					((ch->in_room->sector_type == SECT_TOREADOR) &&
					 (str_cmp(ch->clan, "Toreador"))) ||

					((ch->in_room->sector_type == SECT_CAITIFF) &&
					 (str_cmp(ch->clan, "Caitiff"))) ||

					((ch->in_room->sector_type == SECT_LASOMBRA) &&
					 (str_cmp(ch->clan, "Lasombra"))) ||

					((ch->in_room->sector_type == SECT_TZIMISCE) &&
					 (str_cmp(ch->clan, "Tzimisce"))))

				{
					send_to_char("You are not of that clan !\n\r", ch);
					return;
				}
				if (!str_cmp(rt->choutput, "copy"))
					act(rt->output, ch, NULL, NULL, TO_CHAR);
				else
					act(rt->choutput, ch, NULL, NULL, TO_CHAR);
				if (!IS_SET(rt->type, RT_PERSONAL))
					act(rt->output, ch, NULL, NULL, TO_ROOM);
				if (ch->mount != NULL)
					do_dismount(ch, "");
				char_from_room(ch);
				char_to_room(ch, get_room_index(rt->power));
				act("$n appears in the room.", ch, NULL, NULL, TO_ROOM);
				do_look(ch, "auto");
				hop = TRUE;
				break;
			case RT_ACTION:
				snprintf(arg, MAX_INPUT_LENGTH, "%s", argument);
				argument = one_argument(arg, arg1, MAX_INPUT_LENGTH);
				argument = one_argument(arg, arg2, MAX_INPUT_LENGTH);
				if ((mob = get_char_room(ch, arg2)) == NULL)
					continue;
				interpret(mob, rt->output);
				break;
			case RT_OPEN_LIFT:
				open_lift(ch);
				break;
			case RT_CLOSE_LIFT:
				close_lift(ch);
				break;
			case RT_MOVE_LIFT:
				move_lift(ch, rt->power);
				break;
			default:
				break;
			}
			if (hop && IS_SET(rt->type, RT_RETURN))
				return;
			else if (hop)
				continue;
			if (!str_cmp(rt->choutput, "copy") && !IS_SET(rt->type, RT_ACTION))
				act(rt->output, ch, NULL, NULL, TO_CHAR);
			else if (!IS_SET(rt->type, RT_ACTION))
				act(rt->choutput, ch, NULL, NULL, TO_CHAR);
			if (!IS_SET(rt->type, RT_PERSONAL) && !IS_SET(rt->type, RT_ACTION))
				act(rt->output, ch, NULL, NULL, TO_ROOM);
			if (IS_SET(rt->type, RT_RETURN))
				return;
		}
	}
	return;
}

char *strlower(char *ip)
{
	static char buffer[MAX_INPUT_LENGTH];
	int pos;

	for (pos = 0; pos < (MAX_INPUT_LENGTH - 1) && ip[pos] != '\0'; pos++)
	{
		buffer[pos] = tolower(ip[pos]);
	}
	buffer[pos] = '\0';
	return buffer;
}

bool is_in(char *arg, char *ip)
{
	char *lo_arg;
	char cmp[MAX_INPUT_LENGTH];
	int fitted;

	if (ip[0] != '|')
		return FALSE;
	cmp[0] = '\0';
	lo_arg = strlower(arg);
	do
	{
		ip += strlen(cmp) + 1;
		fitted = sscanf(ip, "%[^*]", cmp);
		if (strstr(lo_arg, cmp) != NULL)
		{
			return TRUE;
		}
	} while (fitted > 0);
	return FALSE;
}

bool all_in(char *arg, char *ip)
{
	char *lo_arg;
	char cmp[MAX_INPUT_LENGTH];
	int fitted;

	if (ip[0] != '&')
		return FALSE;
	cmp[0] = '\0';
	lo_arg = strlower(arg);
	do
	{
		ip += strlen(cmp) + 1;
		fitted = sscanf(ip, "%[^*]", cmp);
		if (strstr(lo_arg, cmp) == NULL)
		{
			return FALSE;
		}
	} while (fitted > 0);
	return TRUE;
}

void tell_someone(CHAR_DATA *ch, CHAR_DATA *victim, char *argument)
{
	char poly[MAX_INPUT_LENGTH];
	int position;

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_SILENCE))
	{
		send_to_char("Your message didn't get through.\n\r", ch);
		return;
	}
	if (IS_BODY(ch, GAGGED))
	{
		send_to_char("Your message didn't get through.\n\r", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_CONE_OF_SILENCE))
	{
		send_to_char("You are in a room that is silenced!\n\r", ch);
		return;
	}
	if (!IS_IMMORTAL(ch) && !IS_AWAKE(victim))
	{
		act("$E can't hear you.", ch, 0, victim, TO_CHAR);
		return;
	}

	act("~tYou tell $N '$t'.", ch, argument, victim, TO_CHAR);

	position = victim->position;
	victim->position = POS_STANDING;

	snprintf(poly, MAX_INPUT_LENGTH, "~t$n tells you '$t'.");
	ADD_COLOUR(victim, poly, WHITE, MAX_INPUT_LENGTH);
	act(poly, ch, argument, victim, TO_VICT);

	victim->position = position;
	victim->reply = ch;
}

void add_to_history(CHANNEL_DATA *channel_history, const char * information)
{
	int hour, minute;
	char message[MAX_STRING_LENGTH];
	struct tm *tmtemp;

	hour = 0;
	minute = 0;
	if ((tmtemp = localtime(&current_time)) != NULL)
	{
		hour = tmtemp->tm_hour;
		minute = tmtemp->tm_min;
	}

	int pos = channel_history->position;
	channel_history->position = (channel_history->position + 1) % REVIEW_HISTORY_SIZE;

	snprintf(message, MAX_STRING_LENGTH, "%02d:%02d %s", hour, minute, information);
	free_string(channel_history->history[pos]);
	channel_history->history[pos] = str_dup(message);
}

void do_tell(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (arg[0] == '\0' || argument[0] == '\0')
	{
		send_to_char("Tell whom what?\n\r", ch);
		return;
	}

	/*
    * Can tell to PC's anywhere, but NPC's only in same room.
    * -- Furey
    */
	if ((victim = get_char_world(ch, arg)) == NULL || (IS_NPC(victim) && victim->in_room != ch->in_room))
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	tell_someone(ch, victim, argument);
	return;
}

void do_reply(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;

	if ((victim = ch->reply) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	tell_someone(ch, victim, argument);
	return;
}

void do_emote(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char *plast;

	char name[80];
	char poly[MAX_INPUT_LENGTH];
	CHAR_DATA *to;
	bool is_ok;

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_NO_EMOTE))
	{
		send_to_char("You can't show your emotions.\n\r", ch);
		return;
	}

	if (IS_HEAD(ch, LOST_TONGUE) || IS_HEAD(ch, LOST_HEAD) || IS_HEAD(ch, GAGGED))
	{
		send_to_char("You can't show your emotions.\n\r", ch);
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char("Emote what?\n\r", ch);
		return;
	}

	for (plast = argument; *plast != '\0'; plast++)
		;

	strncpy(buf, argument, MAX_STRING_LENGTH);
	if (isalpha(plast[-1]))
		strncat(buf, ".", MAX_STRING_LENGTH - strlen(buf));

	act("$n $T", ch, NULL, buf, TO_CHAR);

	if (ch->in_room->vnum != ROOM_VNUM_IN_OBJECT)
	{
		act("$n $T", ch, NULL, buf, TO_ROOM);
		return;
	}

	to = ch->in_room->people;
	for (; to != NULL; to = to->next_in_room)
	{
		is_ok = FALSE;

		if (to->desc == NULL || !IS_AWAKE(to))
			continue;

		if (ch == to)
			continue;

		if (!IS_NPC(ch) && ch->pcdata->chobj != NULL &&
			ch->pcdata->chobj->in_room != NULL &&
			!IS_NPC(to) && to->pcdata->chobj != NULL &&
			to->pcdata->chobj->in_room != NULL &&
			ch->in_room == to->in_room)
			is_ok = TRUE;
		else
			is_ok = FALSE;

		if (!IS_NPC(ch) && ch->pcdata->chobj != NULL &&
			ch->pcdata->chobj->in_obj != NULL &&
			!IS_NPC(to) && to->pcdata->chobj != NULL &&
			to->pcdata->chobj->in_obj != NULL &&
			ch->pcdata->chobj->in_obj == to->pcdata->chobj->in_obj)
			is_ok = TRUE;
		else
			is_ok = FALSE;

		if (!is_ok)
			continue;

		if (IS_NPC(ch))
			snprintf(name, 80, "%s", ch->short_descr);
		else if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_POLYMORPH))
			snprintf(name, 80, "%s", ch->morph);
		else
			snprintf(name, 80, "%s", ch->name);
		name[0] = UPPER(name[0]);
		snprintf(poly, MAX_INPUT_LENGTH, "%s %s\n\r", name, buf);
		send_to_char(poly, to);
	}
	return;
}

/*
* All the posing stuff.
*/
struct pose_table_type
{
	char *message[2 * MAX_CLASS];
};

const struct pose_table_type pose_table[] =
	{
		{{"You sizzle with energy.",
		  "$n sizzles with energy.",
		  "You feel very holy.",
		  "$n looks very holy.",
		  "You perform a small card trick.",
		  "$n performs a small card trick.",
		  "You show your bulging muscles.",
		  "$n shows $s bulging muscles."}},

		{{"You turn into a butterfly, then return to your normal shape.",
		  "$n turns into a butterfly, then returns to $s normal shape.",
		  "You nonchalantly turn wine into water.",
		  "$n nonchalantly turns wine into water.",
		  "You wiggle your ears alternately.",
		  "$n wiggles $s ears alternately.",
		  "You crack nuts between your fingers.",
		  "$n cracks nuts between $s fingers."}},

		{{"Blue sparks fly from your fingers.",
		  "Blue sparks fly from $n's fingers.",
		  "A halo appears over your head.",
		  "A halo appears over $n's head.",
		  "You nimbly tie yourself into a knot.",
		  "$n nimbly ties $mself into a knot.",
		  "You grizzle your teeth and look mean.",
		  "$n grizzles $s teeth and looks mean."}},

		{{"Little red lights dance in your eyes.",
		  "Little red lights dance in $n's eyes.",
		  "You recite words of wisdom.",
		  "$n recites words of wisdom.",
		  "You juggle with daggers, apples, and eyeballs.",
		  "$n juggles with daggers, apples, and eyeballs.",
		  "You hit your head, and your eyes roll.",
		  "$n hits $s head, and $s eyes roll."}},

		{{"A slimy green monster appears before you and bows.",
		  "A slimy green monster appears before $n and bows.",
		  "Deep in prayer, you levitate.",
		  "Deep in prayer, $n levitates.",
		  "You steal the underwear off every person in the room.",
		  "Your underwear is gone!  $n stole it!",
		  "Crunch, crunch -- you munch a bottle.",
		  "Crunch, crunch -- $n munches a bottle."}},

		{{"You turn everybody into a little pink elephant.",
		  "You are turned into a little pink elephant by $n.",
		  "An angel consults you.",
		  "An angel consults $n.",
		  "The dice roll ... and you win again.",
		  "The dice roll ... and $n wins again.",
		  "... 98, 99, 100 ... you do pushups.",
		  "... 98, 99, 100 ... $n does pushups."}},

		{{"A small ball of light dances on your fingertips.",
		  "A small ball of light dances on $n's fingertips.",
		  "Your body glows with an unearthly light.",
		  "$n's body glows with an unearthly light.",
		  "You count the money in everyone's pockets.",
		  "Check your money, $n is counting it.",
		  "Arnold Schwarzenegger admires your physique.",
		  "Arnold Schwarzenegger admires $n's physique."}},

		{{"Smoke and fumes leak from your nostrils.",
		  "Smoke and fumes leak from $n's nostrils.",
		  "A spot light hits you.",
		  "A spot light hits $n.",
		  "You balance a pocket knife on your tongue.",
		  "$n balances a pocket knife on your tongue.",
		  "Watch your feet, you are juggling granite boulders.",
		  "Watch your feet, $n is juggling granite boulders."}},

		{{"The light flickers as you rap in magical languages.",
		  "The light flickers as $n raps in magical languages.",
		  "Everyone levitates as you pray.",
		  "You levitate as $n prays.",
		  "You produce a coin from everyone's ear.",
		  "$n produces a coin from your ear.",
		  "Oomph!  You squeeze water out of a granite boulder.",
		  "Oomph!  $n squeezes water out of a granite boulder."}},

		{{"Your head disappears.",
		  "$n's head disappears.",
		  "A cool breeze refreshes you.",
		  "A cool breeze refreshes $n.",
		  "You step behind your shadow.",
		  "$n steps behind $s shadow.",
		  "You pick your teeth with a spear.",
		  "$n picks $s teeth with a spear."}},

		{{"A fire elemental singes your hair.",
		  "A fire elemental singes $n's hair.",
		  "The sun pierces through the clouds to illuminate you.",
		  "The sun pierces through the clouds to illuminate $n.",
		  "Your eyes dance with greed.",
		  "$n's eyes dance with greed.",
		  "Everyone is swept off their feet by your hug.",
		  "You are swept off your feet by $n's hug."}},

		{{"The sky changes color to match your eyes.",
		  "The sky changes color to match $n's eyes.",
		  "The ocean parts before you.",
		  "The ocean parts before $n.",
		  "You deftly steal everyone's weapon.",
		  "$n deftly steals your weapon.",
		  "Your karate chop splits a tree.",
		  "$n's karate chop splits a tree."}},

		{{"The stones dance to your command.",
		  "The stones dance to $n's command.",
		  "A thunder cloud kneels to you.",
		  "A thunder cloud kneels to $n.",
		  "The Grey Mouser buys you a beer.",
		  "The Grey Mouser buys $n a beer.",
		  "A strap of your armor breaks over your mighty thews.",
		  "A strap of $n's armor breaks over $s mighty thews."}},

		{{"The heavens and grass change colour as you smile.",
		  "The heavens and grass change colour as $n smiles.",
		  "The Burning Man speaks to you.",
		  "The Burning Man speaks to $n.",
		  "Everyone's pocket explodes with your fireworks.",
		  "Your pocket explodes with $n's fireworks.",
		  "A boulder cracks at your frown.",
		  "A boulder cracks at $n's frown."}},

		{{"Everyone's clothes are transparent, and you are laughing.",
		  "Your clothes are transparent, and $n is laughing.",
		  "An eye in a pyramid winks at you.",
		  "An eye in a pyramid winks at $n.",
		  "Everyone discovers your dagger a centimeter from their eye.",
		  "You discover $n's dagger a centimeter from your eye.",
		  "Mercenaries arrive to do your bidding.",
		  "Mercenaries arrive to do $n's bidding."}},

		{{"A black hole swallows you.",
		  "A black hole swallows $n.",
		  "Valentine Michael Smith offers you a glass of water.",
		  "Valentine Michael Smith offers $n a glass of water.",
		  "Where did you go?",
		  "Where did $n go?",
		  "Four matched Percherons bring in your chariot.",
		  "Four matched Percherons bring in $n's chariot."}},

		{{"The world shimmers in time with your whistling.",
		  "The world shimmers in time with $n's whistling.",
		  "The great god Mota gives you a staff.",
		  "The great god Mota gives $n a staff.",
		  "Click.",
		  "Click.",
		  "Atlas asks you to relieve him.",
		  "Atlas asks $n to relieve him."}}};

void do_pose(CHAR_DATA *ch, char *argument)
{
	int level;
	int pose;

	if (IS_NPC(ch))
		return;

	level = UMIN(ch->level, sizeof(pose_table) / sizeof(pose_table[0]) - 1);
	pose = number_range(0, level);

	act(pose_table[pose].message[2 * ch->class + 0], ch, NULL, NULL, TO_CHAR);
	act(pose_table[pose].message[2 * ch->class + 1], ch, NULL, NULL, TO_ROOM);

	return;
}

void do_bug(CHAR_DATA *ch, char *argument)
{
	char bugbuf[MAX_STRING_LENGTH];
	snprintf(bugbuf, MAX_STRING_LENGTH, "Error from %s: %s", ch->name, argument);
	append_file(ch, BUG_FILE, bugbuf);
	send_to_char("Bug submitted, thank you.\n\r", ch);
	return;
}

void do_idea(CHAR_DATA *ch, char *argument)
{
	char ideabuf[MAX_STRING_LENGTH];
	snprintf(ideabuf, MAX_STRING_LENGTH, "Idea from %s: %s", ch->name, argument);
	append_file(ch, IDEA_FILE, ideabuf);
	send_to_char("Idea submitted, thank you.\n\r", ch);
	return;
}

void do_typo(CHAR_DATA *ch, char *argument)
{
	char typobuf[MAX_STRING_LENGTH];
	snprintf(typobuf, MAX_STRING_LENGTH, "Typo from %s: %s", ch->name, argument);
	append_file(ch, TYPO_FILE, typobuf);
	send_to_char("Typo submitted, thank you.\n\r", ch);
	return;
}

void do_rent(CHAR_DATA *ch, char *argument)
{
	send_to_char("There is no rent here.  Just save and quit.\n\r", ch);
	return;
}

void do_qui(CHAR_DATA *ch, char *argument)
{
	send_to_char("If you want to QUIT, you have to spell it out.\n\r", ch);
	return;
}

void drop_others_stuff(CHAR_DATA *ch)
{
	OBJ_DATA *obj, *next;

	for (obj = ch->carrying; obj != NULL; obj = next)
	{
		next = obj->next_content;
		if ((obj->questowner != NULL) && str_cmp(obj->questowner, "") && str_cmp(ch->name, obj->questowner))
		{
			/* oi, you don't own this, drop it */
			obj_from_char(obj);
			obj_to_room(obj, ch->in_room);
			act("$n drops $p.", ch, obj, NULL, TO_ROOM);
			act("You drop $p.", ch, obj, NULL, TO_CHAR);

			/* Objects should only have a shadowplane flag when on the floor */
			if (IS_AFFECTED(ch, AFF_SHADOWPLANE) && (!IS_SET(obj->extra_flags, ITEM_SHADOWPLANE)))
				SET_BIT(obj->extra_flags, ITEM_SHADOWPLANE);
		}
	}
}

void do_quit(CHAR_DATA *ch, char *argument)
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];
	AFFECT_DATA *paf;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if (IS_NPC(ch))
		return;

	if (ch->position == POS_FIGHTING)
	{
		send_to_char("No way! You are fighting.\n\r", ch);
		return;
	}

	if (IS_SET(ch->act, PLR_NOQUIT))
	{
		send_to_char("You are unable to quit yet.\n\r", ch);
		return;
	}

	if ((ch->position < POS_SLEEPING) && (ch->in_room->vnum != ROOM_VNUM_LIMBO))
	{
		send_to_char("You're not DEAD yet.\n\r", ch);
		return;
	}

	if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_TRACKING))
	{
		affect_strip(ch, gsn_hunt);
	}

	if (ch->mount != NULL)
		do_dismount(ch, "");
	call_all(ch);
	if (ch->level < LEVEL_SEER)
		drop_others_stuff(ch);

	send_to_char("\n\r                         Speak not: whisper not:\n\r", ch);
	send_to_char("                      I know all that ye would tell,\n\r", ch);
	send_to_char("                    But to speak might break the spell\n\r", ch);
	send_to_char("                      Which must bend the invincible,\n\r", ch);
	send_to_char("                          The stern of thought;\n\r", ch);
	send_to_char("                 He yet defies the deepest power of Hell.\n\r\n\r", ch);
	snprintf(log_buf, MAX_INPUT_LENGTH * 2, "%s has quit.", ch->name);
	log_string(log_buf);
	/*
    * After extract_char the ch is no longer valid!
    */
	save_char_obj(ch);
	d = ch->desc;
	for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next_content;
		if (obj->wear_loc == WEAR_NONE)
			continue;
		if (!IS_NPC(ch) && ((obj->chobj != NULL && !IS_NPC(obj->chobj) &&
							 obj->chobj->pcdata->obj_vnum != 0) ||
							obj->item_type == ITEM_KEY))
		{
			for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
				affect_modify(ch, paf, FALSE);
			for (paf = obj->affected; paf != NULL; paf = paf->next)
				affect_modify(ch, paf, FALSE);
		}
	}
	if (ch->pcdata->obj_vnum != 0)
		act("$n slowly fades out of existance.", ch, NULL, NULL, TO_ROOM);
	else
		act("$n has left the game.", ch, NULL, NULL, TO_ROOM);

	if (ch->pcdata->obj_vnum == 0)
	{
		snprintf(buf, MAX_STRING_LENGTH, "%s has left the Vampire Wars.", ch->name);
		do_info(ch, buf);
	}
	if (ch->in_room != NULL)
		char_from_room(ch);
	char_to_room(ch, get_room_index(30002));

	if (d != NULL)
		close_socket(d);

	/*    if (ch->pcdata->obj_vnum == 0)
	   {
	   sprintf( buf, "%s has left the Vampire Wars.", ch->name );
	   do_info(ch,buf);
} */

	if (ch->pcdata->chobj != NULL)
		extract_obj(ch->pcdata->chobj);
	
	extract_char(ch, TRUE);
	return;
}

void do_save(CHAR_DATA *ch, char *argument)
{
	/*	char temp[MAX_INPUT_LENGTH]; */

	if (IS_NPC(ch))
		return;

	if (ch->level < 2)
	{
		send_to_char("You must kill at least 5 mobs before you can save.\n\r", ch);
		return;
	}

	save_char_obj(ch);
	save_claninfo();
	/*    sprintf( temp, "cp %s%s%s%s %s%s%s%s &\n\r", PLAYER_DIR, initial(ch->name), "/", capitalize(ch->name), BACKUP_DIR, initial(ch->name), "/", capitalize(ch->name));
    system( temp ); */
	send_to_char("Ok.\n\r", ch);
	return;
}

void do_follow(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (arg[0] == '\0')
	{
		send_to_char("Follow whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL)
	{
		act("But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR);
		return;
	}

	if (victim == ch)
	{
		if (ch->master == NULL)
		{
			send_to_char("You already follow yourself.\n\r", ch);
			return;
		}
		stop_follower(ch);
		return;
	}

	if ((ch->level - victim->level < -5 || ch->level - victim->level > 5) && !IS_HERO(ch))
	{
		send_to_char("You are not of the right caliber to follow.\n\r", ch);
		return;
	}

	if (ch->master != NULL)
		stop_follower(ch);

	add_follower(ch, victim);
	return;
}

void add_follower(CHAR_DATA *ch, CHAR_DATA *master)
{
	char poly[MAX_INPUT_LENGTH];
	if (ch->master != NULL)
	{
		bug("Add_follower: non-null master.", 0);
		return;
	}

	ch->master = master;
	ch->leader = NULL;

	if (can_see(master, ch))
	{
		snprintf(poly, MAX_INPUT_LENGTH, "$n now follows you.");
		act(poly, ch, NULL, master, TO_VICT);
	}
	snprintf(poly, MAX_INPUT_LENGTH, "You now follow $N.");
	act(poly, ch, NULL, master, TO_CHAR);

	return;
}

void stop_follower(CHAR_DATA *ch)
{
	char poly[MAX_INPUT_LENGTH];
	if (ch->master == NULL)
	{
		bug("Stop_follower: null master.", 0);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM))
	{
		REMOVE_BIT(ch->affected_by, AFF_CHARM);
		affect_strip(ch, gsn_charm_person);
	}

	if (can_see(ch->master, ch))
	{
		snprintf(poly, MAX_INPUT_LENGTH, "$n stops following you.");
		act(poly, ch, NULL, ch->master, TO_VICT);
	}
	snprintf(poly, MAX_INPUT_LENGTH, "You stop following $N.");
	act(poly, ch, NULL, ch->master, TO_CHAR);

	ch->master = NULL;
	ch->leader = NULL;
	return;
}

void die_follower(CHAR_DATA *ch)
{
	CHAR_DATA *fch;

	if (ch->master != NULL)
		stop_follower(ch);

	ch->leader = NULL;

	for (fch = char_list; fch != NULL; fch = fch->next)
	{
		if (fch->master == ch)
			stop_follower(fch);
		if (fch->leader == ch)
			fch->leader = fch;
	}

	return;
}

void do_order(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *och;
	CHAR_DATA *och_next;
	bool found;
	bool fAll;

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (arg[0] == '\0' || argument[0] == '\0')
	{
		send_to_char("Order whom to do what?\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM))
	{
		send_to_char("You feel like taking, not giving, orders.\n\r", ch);
		return;
	}

	if (!str_cmp(arg, "all"))
	{
		fAll = TRUE;
		victim = NULL;
	}
	else
	{
		fAll = FALSE;
		if ((victim = get_char_room(ch, arg)) == NULL)
		{
			send_to_char("They aren't here.\n\r", ch);
			return;
		}

		if (victim == ch)
		{
			send_to_char("Aye aye, right away!\n\r", ch);
			return;
		}

		if ((!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch) && !(IS_SET(ch->act, PLR_VAMPIRE) && IS_SET(victim->act, PLR_VAMPIRE)))
		{
			send_to_char("Do it yourself!\n\r", ch);
			return;
		}

		if (IS_SET(ch->act, PLR_VAMPIRE) && IS_SET(victim->act, PLR_VAMPIRE) && ((ch->vampgen > victim->vampgen) || str_cmp(ch->clan, victim->clan)))
		{
			act("$N ignores your order.", ch, NULL, victim, TO_CHAR);
			act("You ignore $n's order.", ch, NULL, victim, TO_VICT);
			return;
		}
	}

	found = FALSE;
	if (
		!strncmp(strlower(argument), ".", 1) ||
		!strncmp(strlower(argument), "/", 1) ||
		!strncmp(strlower(argument), ">", 1) ||
		!strncmp(strlower(argument), "arm", 3) ||
		!strncmp(strlower(argument), "au", 2) ||
		!strncmp(strlower(argument), "be", 2) ||
		!strncmp(strlower(argument), "bi", 2) ||
		!strncmp(strlower(argument), "bra", 3) ||
		!strncmp(strlower(argument), "c", 1) ||
		!strncmp(strlower(argument), "de", 2) ||
		!strncmp(strlower(argument), "dec", 3) ||
		!strncmp(strlower(argument), "dro", 3) ||
		!strncmp(strlower(argument), "ev", 2) ||
		!strncmp(strlower(argument), "exa", 3) ||
		!strncmp(strlower(argument), "gif", 3) ||
		!strncmp(strlower(argument), "l", 1) ||
		!strncmp(strlower(argument), "mu", 2) ||
		!strncmp(strlower(argument), "p", 1) ||
		!strncmp(strlower(argument), "q", 1) ||
		!strncmp(strlower(argument), "qua", 3) ||
		!strncmp(strlower(argument), "quit", 4) ||
		!strncmp(strlower(argument), "rec", 3) ||
		!strncmp(strlower(argument), "rem", 3) ||
		!strncmp(strlower(argument), "sac", 3) ||
		!strncmp(strlower(argument), "sho", 3) ||
		!strncmp(strlower(argument), "to", 2) ||
		!strncmp(strlower(argument), "trai", 4) ||
		!strncmp(strlower(argument), "unt", 3) ||
		!strncmp(strlower(argument), "v", 1) ||
		!strncmp(strlower(argument), "y", 1) ||
		!strncmp(strlower(argument), "z", 1))
	{
		send_to_char("That just wouldn't be cricket\n\r", ch);
		return;
	}

	for (och = ch->in_room->people; och != NULL; och = och_next)
	{
		och_next = och->next_in_room;

		if ((IS_AFFECTED(och, AFF_CHARM) && och->master == ch && (fAll || och == victim)) ||
			(!IS_NPC(ch) && !IS_NPC(och) &&
			 IS_SET(ch->act, PLR_VAMPIRE) && IS_SET(och->act, PLR_VAMPIRE) && ch->vampgen < och->vampgen && !str_cmp(ch->clan, och->clan)))
		{
			found = TRUE;
			act("$n orders you to '$t'.", ch, argument, och, TO_VICT);
			interpret(och, argument);
		}
	}

	if (found)
	{
		send_to_char("Ok.\n\r", ch);
		WAIT_STATE(ch, 12);
	}
	else
		send_to_char("You have no followers here.\n\r", ch);
	return;
}

void do_command(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;
	if (!IS_SET(ch->act, PLR_VAMPIRE))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
	if (!IS_VAMPAFF(ch, VAM_DOMINATE))
	{
		send_to_char("You are not trained in the dominate discipline.\n\r", ch);
		return;
	}
	if (arg[0] == '\0' || argument[0] == '\0')
	{
		send_to_char("Command whom to do what?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		send_to_char("How can you command yourself??\n\r", ch);
		return;
	}

	if (!IS_NPC(victim) && victim->level != 3)
	{
		send_to_char("You can only command other avatars.\n\r", ch);
		return;
	}

	if (ch->position == POS_FIGHTING)
	{
		send_to_char("You cannot do that while fighting.\n\r", ch);
		return;
	}

	if (!strncmp(strlower(argument), "arm", 3) ||
		!strncmp(strlower(argument), "to", 2) ||
		!strncmp(strlower(argument), "quit", 4) ||
		!strncmp(strlower(argument), "gif", 3) ||
		!strncmp(strlower(argument), "dro", 3) ||
		!strncmp(strlower(argument), "sac", 3) ||
		!strncmp(strlower(argument), "trai", 4) ||
		!strncmp(strlower(argument), "ev", 2) ||
		!strncmp(strlower(argument), "rem", 3) ||
		!strncmp(strlower(argument), "dec", 3) ||
		!strncmp(strlower(argument), "l", 1) ||
		!strncmp(strlower(argument), "exa", 3) ||
		!strncmp(strlower(argument), "/", 1) ||
		!strncmp(strlower(argument), "rec", 3) ||
		!strncmp(strlower(argument), "rep", 3) ||
		!strncmp(strlower(argument), "bra", 3) ||
		!strncmp(strlower(argument), "qua", 3) ||
		!strncmp(strlower(argument), "mc", 2) ||
		!strncmp(strlower(argument), "au", 2) ||
		!strncmp(strlower(argument), "mu", 2) ||
		!strncmp(strlower(argument), "gr", 2) ||
		!strncmp(strlower(argument), "z", 1) ||
		!strncmp(strlower(argument), "c", 1) ||
		!strncmp(strlower(argument), "q", 1) ||
		!strncmp(strlower(argument), "unt", 3) ||
		!strncmp(strlower(argument), "v", 1) ||
		!strncmp(strlower(argument), "y", 1) ||
		!strncmp(strlower(argument), "p", 1) ||
		!strncmp(strlower(argument), "sh", 2) ||
		!strncmp(strlower(argument), "de", 2) ||
		!strncmp(strlower(argument), ".", 1) ||
		!strncmp(strlower(argument), "bers", 4) ||
		!strncmp(strlower(argument), "backs", 5) ||
		!strncmp(strlower(argument), ">", 1))
	{
		send_to_char("Now why would you want to do that ?\n\r", ch);
		return;
	}

	/* Cheat catching bit - Archon */

	if ((!strncmp(strlower(argument), "de", 2)) && IS_NPC(victim))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "Log: **CHEAT**: %s just tried to crash the mud with command %s.", ch->name, argument);
		log_string(buf);
	}

	if (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
	{
		send_to_char("You cannot command this shopkeeper!\n\r", ch);
		return;
	}

	if (IS_NPC(victim))
		snprintf(buf, MAX_INPUT_LENGTH, "I think %s wants to %s", victim->short_descr, argument);
	else if (!IS_NPC(victim) && IS_AFFECTED(victim, AFF_POLYMORPH))
		snprintf(buf, MAX_INPUT_LENGTH, "I think %s wants to %s", victim->morph, argument);
	else
		snprintf(buf, MAX_INPUT_LENGTH, "I think %s wants to %s", victim->name, argument);
	do_say(ch, buf);

	if (IS_NPC(victim) &&
		(victim->level >= ((get_age(ch) / 100) * 20) ||
		 victim->level >= 100))

	{
		act("You shake off $N's suggestion.", victim, NULL, ch, TO_CHAR);
		act("$n shakes off $N's suggestion.", victim, NULL, ch, TO_NOTVICT);
		act("$n shakes off your suggestion.", victim, NULL, ch, TO_VICT);
		act("$s mind is too strong to overcome.", victim, NULL, ch, TO_VICT);
		WAIT_STATE(ch, 12); /* so they can't do loads of commands all at once in an alias */
		return;
	}

	else if (!IS_NPC(victim) && get_age(victim) >= get_age(ch))
	{
		if (number_percent() > 5)
		{
			act("You shake off $N's suggestion.", victim, NULL, ch, TO_CHAR);
			act("$n shakes off $N's suggestion.", victim, NULL, ch, TO_NOTVICT);
			act("$n shakes off your suggestion.", victim, NULL, ch, TO_VICT);
			act("$s mind is too strong to overcome.", victim, NULL, ch, TO_VICT);
			WAIT_STATE(ch, 12); /* so they can't do loads of commands all at once in an alias */
			return;
		}
	}

	else if (!IS_NPC(victim) && get_age(victim) < get_age(ch))
	{
		if ((number_percent() > (get_age(ch) - get_age(victim))) ||
			number_percent() < 5)
		{
			act("You shake off $N's suggestion.", victim, NULL, ch, TO_CHAR);
			act("$n shakes off $N's suggestion.", victim, NULL, ch, TO_NOTVICT);
			act("$n shakes off your suggestion.", victim, NULL, ch, TO_VICT);
			act("$s mind is too strong to overcome.", victim, NULL, ch, TO_VICT);
			WAIT_STATE(ch, 12); /* so they can't do loads of commands all at once in an alias */
			return;
		}
	}
	act("You blink in confusion.", victim, NULL, NULL, TO_CHAR);
	act("$n blinks in confusion.", victim, NULL, NULL, TO_ROOM);
	do_say(victim, "Yes, you're right, I do...");
	interpret(victim, argument);
	WAIT_STATE(ch, 12); /* so they can't do loads of commands all at once in an alias */
	return;
}

void do_group(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (arg[0] == '\0')
	{
		CHAR_DATA *gch;
		CHAR_DATA *leader;

		leader = (ch->leader != NULL) ? ch->leader : ch;
		snprintf(buf, MAX_STRING_LENGTH, "%s's group:\n\r", PERS(leader, ch));
		send_to_char(buf, ch);

		for (gch = char_list; gch != NULL; gch = gch->next)
		{
			if (is_same_group(gch, ch))
			{
				snprintf(buf, MAX_STRING_LENGTH,
						 "[%-16s] %4d/%4d hp %4d/%4d mana %4d/%4d mv %5ld xp\n\r",
						 capitalize(PERS(gch, ch)),
						 gch->hit, gch->max_hit,
						 gch->mana, gch->max_mana,
						 gch->move, gch->max_move,
						 gch->exp);
				send_to_char(buf, ch);
			}
		}
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (IS_NPC(ch) || IS_NPC(victim))
	{
		if (!IS_NPC(ch))
			send_to_char("You cannot group with a mob.\n\r", ch);
		return;
	}

	if (ch->master != NULL || (ch->leader != NULL && ch->leader != ch))
	{
		send_to_char("But you are following someone else!\n\r", ch);
		return;
	}

	if (victim->master != ch && ch != victim)
	{
		act("$N isn't following you.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_same_group(victim, ch) && ch != victim)
	{
		victim->leader = NULL;
		act("$n removes $N from $s group.", ch, NULL, victim, TO_NOTVICT);
		act("$n removes you from $s group.", ch, NULL, victim, TO_VICT);
		act("You remove $N from your group.", ch, NULL, victim, TO_CHAR);
		return;
	}

	victim->leader = ch;
	act("$N joins $n's group.", ch, NULL, victim, TO_NOTVICT);
	act("You join $n's group.", ch, NULL, victim, TO_VICT);
	act("$N joins your group.", ch, NULL, victim, TO_CHAR);
	return;
}

/*
* 'Split' originally by Gnort, God of Chaos.
*/
void do_split(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	int members;
	int amount;
	int share;
	int extra;

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (arg[0] == '\0')
	{
		send_to_char("Split how much?\n\r", ch);
		return;
	}

	amount = atoi(arg);

	if (amount < 0)
	{
		send_to_char("Your group wouldn't like that.\n\r", ch);
		return;
	}

	if (amount == 0)
	{
		send_to_char("You hand out zero coins, but no one notices.\n\r", ch);
		return;
	}

	if (ch->gold < amount)
	{
		send_to_char("You don't have that much gold.\n\r", ch);
		return;
	}

	members = 0;
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
	{
		if (is_same_group(gch, ch))
			members++;
	}

	if (members < 2)
	{
		send_to_char("Just keep it all.\n\r", ch);
		return;
	}

	share = amount / members;
	extra = amount % members;

	if (share == 0)
	{
		send_to_char("Don't even bother, cheapskate.\n\r", ch);
		return;
	}

	ch->gold -= amount;
	ch->gold += share + extra;

	snprintf(buf, MAX_STRING_LENGTH,
			 "You split %d gold coins.  Your share is %d gold coins.\n\r",
			 amount, share + extra);
	send_to_char(buf, ch);

	snprintf(buf, MAX_STRING_LENGTH, "$n splits %d gold coins.  Your share is %d gold coins.",
			 amount, share);

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
	{
		if (gch != ch && is_same_group(gch, ch))
		{
			act(buf, ch, NULL, gch, TO_VICT);
			gch->gold += share;
		}
	}

	return;
}

void do_gtell(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *gch;

	if (argument[0] == '\0')
	{
		send_to_char("Tell your group what?\n\r", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_CONE_OF_SILENCE))
	{
		snprintf(buf, MAX_STRING_LENGTH, "You are in a room that is silenced!\n\r");
		send_to_char(buf, ch);
		return;
	}

	if (IS_SET(ch->act, PLR_NO_TELL))
	{
		send_to_char("Your message didn't get through!\n\r", ch);
		return;
	}

	/*
    * Note use of send_to_char, so gtell works on sleepers.
    */
	snprintf(buf, MAX_STRING_LENGTH, "%s tells the group '%s'.\n\r", ch->name, argument);
	for (gch = char_list; gch != NULL; gch = gch->next)
	{
		if (is_same_group(gch, ch))
			send_to_char(buf, gch);
	}

	return;
}

/*
* It is very important that this be an equivalence relation:
* (1) A ~ A
* (2) if A ~ B then B ~ A
* (3) if A ~ B  and B ~ C, then A ~ C
*/
bool is_same_group(CHAR_DATA *ach, CHAR_DATA *bch)
{
	if (ach->leader != NULL)
		ach = ach->leader;
	if (bch->leader != NULL)
		bch = bch->leader;
	return ach == bch;
}

void do_changelight(CHAR_DATA *ch, char *argument)
{
	if (IS_SET(ch->in_room->room_flags, ROOM_DARK))
	{
		REMOVE_BIT(ch->in_room->room_flags, ROOM_DARK);
		act("The room is suddenly filled with light!", ch, NULL, NULL, TO_CHAR);
		act("The room is suddenly filled with light!", ch, NULL, NULL, TO_ROOM);
		return;
	}
	SET_BIT(ch->in_room->room_flags, ROOM_DARK);
	act("The lights in the room suddenly go out!", ch, NULL, NULL, TO_CHAR);
	act("The lights in the room suddenly go out!", ch, NULL, NULL, TO_ROOM);
	return;
}

void open_lift(CHAR_DATA *ch)
{
	ROOM_INDEX_DATA *location;
	int in_room;

	in_room = ch->in_room->vnum;
	location = get_room_index(in_room);

	if (is_open(ch))
		return;

	act("The doors open.", ch, NULL, NULL, TO_CHAR);
	act("The doors open.", ch, NULL, NULL, TO_ROOM);
	move_door(ch);
	if (is_open(ch))
		act("The doors close.", ch, NULL, NULL, TO_ROOM);
	if (!same_floor(ch, in_room))
		act("The lift judders suddenly.", ch, NULL, NULL, TO_ROOM);
	if (is_open(ch))
		act("The doors open.", ch, NULL, NULL, TO_ROOM);
	move_door(ch);
	open_door(ch, FALSE);
	char_from_room(ch);
	char_to_room(ch, location);
	open_door(ch, TRUE);
	move_door(ch);
	open_door(ch, TRUE);
	thru_door(ch, in_room);
	char_from_room(ch);
	char_to_room(ch, location);
	return;
}

void close_lift(CHAR_DATA *ch)
{
	ROOM_INDEX_DATA *location;
	int in_room;

	in_room = ch->in_room->vnum;
	location = get_room_index(in_room);

	if (!is_open(ch))
		return;
	act("The doors close.", ch, NULL, NULL, TO_CHAR);
	act("The doors close.", ch, NULL, NULL, TO_ROOM);
	open_door(ch, FALSE);
	move_door(ch);
	open_door(ch, FALSE);
	char_from_room(ch);
	char_to_room(ch, location);
	return;
}

void move_lift(CHAR_DATA *ch, int to_room)
{
	ROOM_INDEX_DATA *location;
	int in_room;

	in_room = ch->in_room->vnum;
	location = get_room_index(in_room);

	if (is_open(ch))
		act("The doors close.", ch, NULL, NULL, TO_CHAR);
	if (is_open(ch))
		act("The doors close.", ch, NULL, NULL, TO_ROOM);
	if (!same_floor(ch, to_room))
		act("The lift judders suddenly.", ch, NULL, NULL, TO_CHAR);
	if (!same_floor(ch, to_room))
		act("The lift judders suddenly.", ch, NULL, NULL, TO_ROOM);
	move_door(ch);
	open_door(ch, FALSE);
	char_from_room(ch);
	char_to_room(ch, location);
	open_door(ch, FALSE);
	thru_door(ch, to_room);
	return;
}

bool same_floor(CHAR_DATA *ch, int cmp_room)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next_content;
		if (obj->item_type != ITEM_PORTAL)
			continue;
		if (obj->pIndexData->vnum == 30001)
			continue;
		if (obj->value[0] == cmp_room)
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}

bool is_open(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next_content;
		if (obj->item_type != ITEM_PORTAL)
			continue;
		if (obj->pIndexData->vnum == 30001)
			continue;
		if (obj->value[2] == 0)
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}

void move_door(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	ROOM_INDEX_DATA *pRoomIndex;

	for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next_content;
		if (obj->item_type != ITEM_PORTAL)
			continue;
		if (obj->pIndexData->vnum == 30001)
			continue;
		pRoomIndex = get_room_index(obj->value[0]);
		char_from_room(ch);
		char_to_room(ch, pRoomIndex);
		return;
	}
	return;
}

void thru_door(CHAR_DATA *ch, int doorexit)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next_content;
		if (obj->item_type != ITEM_PORTAL)
			continue;
		if (obj->pIndexData->vnum == 30001)
			continue;
		obj->value[0] = doorexit;
		return;
	}
	return;
}

void open_door(CHAR_DATA *ch, bool be_open)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next_content;
		if (obj->item_type != ITEM_PORTAL)
			continue;
		if (obj->pIndexData->vnum == 30001)
			continue;
		if (obj->value[2] == 0 && !be_open)
			obj->value[2] = 3;
		else if (obj->value[2] == 3 && be_open)
			obj->value[2] = 0;
		return;
	}
	return;
}

void do_speak(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char poly[MAX_STRING_LENGTH];
	CHAR_DATA *rch;
	char *pName;
	int iSyl;
	int length;

	struct spk_type
	{
		char *old;
		char *new;
	};

	static const struct spk_type spk_table[] =
		{
			{" ", " "},
			{"are not", "ain't"},
			{"have", "'av"},
			{"hello", "oy"},
			{"hi ", "oy "},
			{"i am", "i'm"},
			{"it is", "tis"},
			{"the ", "da "},
			{" the", " da"},
			{"thank", "fank"},
			{"that", "dat"},
			{"they", "day"},
			{"this", "dis"},
			{"then", "den"},
			{"there", "ver"},
			{"their", "ver"},
			{"thing", "fing"},
			{"think", "fink"},
			{"was", "woz"},
			{"would", "wud"},
			{"what", "wot"},
			{"where", "weer"},
			{"when", "wen"},
			{"are", "is"},
			{"you", "ya"},
			{"your", "yer"},
			{"dead", "ded"},
			{"kill", "stomp"},
			{"food", "nosh"},
			{"blood", "blud"},
			{"vampire", "sucker"},
			{"fire", "hot"},
			{"dwarf", "stunty"},
			{"dwarves", "stunties"},
			{"human", "'uman"},
			{"goblin", "gobbo"},
			{"death", "def"},
			{"immune", "mune"},
			{"immunit", "munit"},
			{"kavir", "KaVir"},
			{"palmer", "Palmer"},
			{"a", "a"},
			{"b", "b"},
			{"c", "c"},
			{"d", "d"},
			{"e", "e"},
			{"f", "f"},
			{"g", "g"},
			{"h", "h"},
			{"i", "i"},
			{"j", "j"},
			{"k", "k"},
			{"l", "l"},
			{"m", "m"},
			{"n", "n"},
			{"o", "o"},
			{"p", "p"},
			{"q", "q"},
			{"r", "r"},
			{"s", "s"},
			{"t", "t"},
			{"u", "u"},
			{"v", "v"},
			{"w", "w"},
			{"x", "x"},
			{"y", "y"},
			{"z", "z"},
			{",", ","},
			{".", "."},
			{";", ";"},
			{":", ":"},
			{"(", "("},
			{")", ")"},
			{")", ")"},
			{"-", "-"},
			{"!", "!"},
			{"?", "?"},
			{"", ""}};
	buf[0] = '\0';

	if (argument[0] == '\0')
	{
		send_to_char("Say what?\n\r", ch);
		return;
	}

	for (pName = str_dup(argument); *pName != '\0'; pName += length)
	{
		for (iSyl = 0; (length = strlen(spk_table[iSyl].old)) != 0; iSyl++)
		{
			if (!str_prefix(spk_table[iSyl].old, pName))
			{
				strncat(buf, spk_table[iSyl].new, MAX_STRING_LENGTH - strlen(buf));
				break;
			}
		}

		if (length == 0)
			length = 1;
	}

	/* For polymorphed players - KaVir */
	if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_POLYMORPH))
		snprintf(poly, MAX_STRING_LENGTH, "%s", ch->morph);
	else if (IS_NPC(ch))
		snprintf(poly, MAX_STRING_LENGTH, "%s", ch->short_descr);
	else
		snprintf(poly, MAX_STRING_LENGTH, "%s", ch->name);

	snprintf(buf2, MAX_STRING_LENGTH, "$n says '%s'.", buf);
	snprintf(buf, MAX_STRING_LENGTH, "$n says '%s'.", argument);
	for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
	{
		if (rch != ch)
			act(ch->class == rch->class ? buf : buf2, ch, NULL, rch, TO_VICT);
	}
	snprintf(buf, MAX_STRING_LENGTH, "You say '%s'.", argument);
	act(buf, ch, NULL, NULL, TO_CHAR);
	room_text(ch, strlower(argument));
	return;
}

void do_cemote(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char *plast;
	DESCRIPTOR_DATA *d;
	char name[80];
	char poly[MAX_INPUT_LENGTH];
	bool is_ok;
	CHAR_DATA *och;
	CHAR_DATA *vch;
	/* Archrip - int position; */
	char palmer[MAX_INPUT_LENGTH];

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_NO_EMOTE))
	{
		send_to_char("You can't show your emotions.\n\r", ch);
		return;
	}

	if (IS_HEAD(ch, LOST_TONGUE) || IS_HEAD(ch, LOST_HEAD) || IS_HEAD(ch, GAGGED))
	{
		send_to_char("You can't show your emotions.\n\r", ch);
		return;
	}

	if (argument[0] == '\0')
	{
		send_to_char("CEmote what?\n\r", ch);
		return;
	}

	for (plast = argument; *plast != '\0'; plast++)
		;

	strncpy(buf, argument, MAX_STRING_LENGTH);
	if (isalpha(plast[-1]))
		strncat(buf, ".", MAX_STRING_LENGTH - strlen(buf));

	/*    if (ch->in_room->vnum != ROOM_VNUM_IN_OBJECT)
	   {
	   act( "$n $T", ch, NULL, buf, TO_ROOM );
	   return;
	   }
    */
	for (d = descriptor_list; d != NULL; d = d->next)
	{
		is_ok = TRUE;

		och = d->original ? d->original : d->character;
		vch = d->character;

		/*	if ( vch->desc == NULL || !IS_AWAKE(vch) )
	   continue;
	   */
		if (och == vch)
			is_ok = FALSE;

		if (!IS_NPC(och) && och->pcdata->chobj != NULL &&
			och->pcdata->chobj->in_room != NULL &&
			!IS_NPC(vch) && vch->pcdata->chobj != NULL &&
			vch->pcdata->chobj->in_room != NULL &&
			och->in_room == vch->in_room)
			is_ok = TRUE;
		else
			is_ok = FALSE;

		if (!IS_NPC(och) && och->pcdata->chobj != NULL &&
			och->pcdata->chobj->in_obj != NULL &&
			!IS_NPC(vch) && vch->pcdata->chobj != NULL &&
			vch->pcdata->chobj->in_obj != NULL &&
			och->pcdata->chobj->in_obj == vch->pcdata->chobj->in_obj)
			is_ok = TRUE;
		else
			is_ok = FALSE;

		snprintf(palmer, MAX_INPUT_LENGTH, "%s %s\n\r", ch->name, buf);
		send_to_char(palmer, vch);

		if (!is_ok)
			continue;
		if (IS_NPC(och))
			snprintf(name, 80, "%s", och->short_descr);
		else if (!IS_NPC(och) && IS_AFFECTED(och, AFF_POLYMORPH))
			snprintf(name, 80, "%s", och->morph);
		else
			snprintf(name, 80, "%s", och->name);
		name[0] = UPPER(name[0]);
		snprintf(poly, MAX_INPUT_LENGTH, "%s %s\n\r", name, buf);
		send_to_char(poly, vch);
	}
	return;
}

void review_history(CHAR_DATA *ch, CHANNEL_DATA *cd)
{
	int i;

	i = cd->position;

	do
	{
		send_to_char(cd->history[i], ch);
		i = (i + 1) % REVIEW_HISTORY_SIZE;
	} while (i != cd->position);

	return;
}

void do_review(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch) || (ch->pcdata == NULL))
	{
		send_to_char("Not for NPCs.\n\r", ch);
		return;
	}

	if (arg[0] == '\0')
	{
		send_to_char("Arguments: chat, tell, newbie, clan, immortal\n\r", ch);
		return;
	}

	if (!str_cmp(arg, "chat"))
	{
		review_history(ch, ch->pcdata->chat_history);
	}
	else if (!str_cmp(arg, "tell"))
	{
		review_history(ch, ch->pcdata->tell_history);
	}
	else if (!str_cmp(arg, "newbie"))
	{
		review_history(ch, ch->pcdata->newbie_history);
	}
	else if (!str_cmp(arg, "clan"))
	{
		review_history(ch, ch->pcdata->clan_history);
	}
	else if(!str_cmp(arg, "immortal"))
	{
		review_history(ch, ch->pcdata->immortal_history);
	}
	else
	{
		send_to_char("You cannot review that channel.\n\r", ch);
	}

	return;
}
