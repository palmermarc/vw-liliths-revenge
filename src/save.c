/***************************************************************************
*  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
*  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
*
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
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "merc.h"
#include "cJSON.h"

struct clan_info clan_infotable[MAX_CLAN];
struct league_info league_infotable[MAX_PLAYERS];

#if !defined(macintosh)
extern int _filbuf args((FILE *));
#endif

/*
* Array of containers read for proper re-nesting of objects.
*/
#define MAX_NEST 100
static OBJ_DATA *rgObjNest[MAX_NEST];

/*
* Local functions.
*/
void fwrite_char args((CHAR_DATA * ch, FILE *fp));
void fwrite_obj args((CHAR_DATA * ch, OBJ_DATA *obj, cJSON *objects, int iNest));
void fread_char args((CHAR_DATA * ch, FILE *fp));
void fread_obj args((CHAR_DATA * ch, FILE *fp));
void fread_clandisc args((CHAR_DATA *ch, FILE *fp));

void load_char_spells_json args((cJSON *spells, CHAR_DATA *ch));
void load_char_weapons_json args((cJSON *weapons, CHAR_DATA *ch));
void load_char_stances_json args((cJSON *stances, CHAR_DATA *ch));
void load_char_skills_json args((cJSON *skills, CHAR_DATA *ch));
void load_char_affects_json args((cJSON *affect_datas, CHAR_DATA *ch));
void load_char_clandiscs_json args((cJSON *clandiscs, CHAR_DATA *ch));
void load_char_objects_json args((cJSON *objects, CHAR_DATA *ch));
extern int top_affect;

char *initial(const char *str)
{
	static char strint[MAX_STRING_LENGTH];
	strint[0] = UPPER(str[0]);
	return strint;
}

/*
* Save a character and inventory.
* Would be cool to save NPC's too for quest purposes,
*   some of the infrastructure is provided.
*/
void save_char_obj(CHAR_DATA *ch)
{
	char strsave[MAX_INPUT_LENGTH];
	char temp[MAX_INPUT_LENGTH];
	FILE *fp;

	if (IS_NPC(ch) || ch->level < 2)
		return;

	if (ch->desc != NULL && ch->desc->original != NULL)
		ch = ch->desc->original;

	ch->save_time = current_time;
	fclose(fpReserve);

#if !defined(macintosh) && !defined(MSDOS)
	snprintf(temp, MAX_INPUT_LENGTH, "%s%s%s%s.json", PLAYER_DIR, initial(ch->name), "/", capitalize(ch->name));
	snprintf(strsave, MAX_INPUT_LENGTH, "tmp.file");
#else
	snprintf(temp, MAX_INPUT_LENGTH, "%s%s.json", PLAYER_DIR, capitalize(ch->name));
	snprintf(strsave, MAX_INPUT_LENGTH, "tmp.file");
#endif

	if ((fp = fopen(strsave, "w")) == NULL)
	{
		bug("Save_char_obj: fopen", 0);
		perror(strsave);
	}
	else
	{
		fwrite_char(ch, fp);
	}
	fclose(fp);

	fpReserve = fopen(NULL_FILE, "r");
	/*    sprintf(temp, "cp %s%s%s%s %s%s%s%s\n\r" ,PLAYER_DIR, initial(ch->name),"/",  capitalize(ch->name), BACKUP_DIR,initial(ch->name), "/", capitalize(ch->name));
    system( temp );
    send_to_char( temp, ch);*/
	rename(strsave, temp);
	return;
}

/*
* Write the char.
*/
void fwrite_char(CHAR_DATA *ch, FILE *fp)
{
	AFFECT_DATA *paf;
	CLANDISC_DATA *disc;
    int iHash;

    cJSON *charData = cJSON_CreateObject();
    cJSON *weapons = NULL;
    cJSON *weapon = NULL;
    cJSON *spells = NULL;
    cJSON *spell = NULL;
    cJSON *skills = NULL;
    cJSON *stances = NULL;
    cJSON *stance = NULL;
    cJSON *stats = NULL;
    cJSON *locationHp = NULL;
    cJSON *condition = NULL;
    cJSON *affect_datas = NULL;
    cJSON *affect_data = NULL;
    cJSON *clandiscs = NULL;
	cJSON *clandisc = NULL;
	cJSON *objects = NULL;

	cJSON_AddItemToObject(charData, "Name", cJSON_CreateString(ch->name));
	cJSON_AddItemToObject(charData, "ShortDescr", cJSON_CreateString(ch->short_descr));
	cJSON_AddItemToObject(charData, "LongDescr", cJSON_CreateString(ch->long_descr));
	cJSON_AddItemToObject(charData, "Description", cJSON_CreateString(ch->description));

	if (ch->prompt != NULL || !str_cmp(ch->prompt, "<%hhp %mm %vmv> "))
		cJSON_AddItemToObject(charData, "Prompt", cJSON_CreateString(ch->prompt));

	cJSON_AddItemToObject(charData, "Lord", cJSON_CreateString(ch->lord));
	cJSON_AddItemToObject(charData, "Clan", cJSON_CreateString(ch->clan));
	cJSON_AddItemToObject(charData, "Morph", cJSON_CreateString(ch->morph));
	cJSON_AddItemToObject(charData, "Createtime", cJSON_CreateString(ch->createtime));
	cJSON_AddItemToObject(charData, "Lasttime", cJSON_CreateString(ch->lasttime));
	cJSON_AddItemToObject(charData, "Lasthost", cJSON_CreateString(ch->lasthost));
	cJSON_AddItemToObject(charData, "Poweraction", cJSON_CreateString(ch->poweraction));
	cJSON_AddItemToObject(charData, "Powertype", cJSON_CreateString(ch->powertype));
	cJSON_AddItemToObject(charData, "Sex", cJSON_CreateNumber(ch->sex));
	cJSON_AddItemToObject(charData, "Class", cJSON_CreateNumber(ch->class));
	cJSON_AddItemToObject(charData, "Race", cJSON_CreateNumber(ch->race));
	cJSON_AddItemToObject(charData, "Status", cJSON_CreateNumber(ch->status));
	cJSON_AddItemToObject(charData, "PKEnabled", cJSON_CreateNumber(ch->pk_enabled));
	cJSON_AddItemToObject(charData, "RemortLevel", cJSON_CreateNumber(ch->remortlevel));
	cJSON_AddItemToObject(charData, "Immune", cJSON_CreateNumber(ch->immune));
	cJSON_AddItemToObject(charData, "Polyaff", cJSON_CreateNumber(ch->polyaff));
	cJSON_AddItemToObject(charData, "Itemaffect", cJSON_CreateNumber(ch->itemaffect));
	cJSON_AddItemToObject(charData, "Vampaff", cJSON_CreateNumber(ch->vampaff));
	cJSON_AddItemToObject(charData, "Vamppass", cJSON_CreateNumber(ch->vamppass));
	cJSON_AddItemToObject(charData, "Form", cJSON_CreateNumber(ch->form));
	cJSON_AddItemToObject(charData, "Beast", cJSON_CreateNumber(ch->beast));
	cJSON_AddItemToObject(charData, "Vampgen", cJSON_CreateNumber(ch->vampgen));
	cJSON_AddItemToObject(charData, "Spectype", cJSON_CreateNumber(ch->spectype));
	cJSON_AddItemToObject(charData, "Specpower", cJSON_CreateNumber(ch->specpower));
	cJSON_AddItemToObject(charData, "Home", cJSON_CreateNumber(ch->home));
	cJSON_AddItemToObject(charData, "Level", cJSON_CreateNumber(ch->level));
	cJSON_AddItemToObject(charData, "Trust", cJSON_CreateNumber(ch->trust));
	cJSON_AddItemToObject(charData, "Played", cJSON_CreateNumber(ch->played + (int)(current_time - ch->logon)));

	cJSON_AddItemToObject(charData, "Room", cJSON_CreateNumber(
			(ch->in_room == get_room_index(ROOM_VNUM_LIMBO) && ch->was_in_room != NULL)
				? ch->was_in_room->vnum
				: ch->in_room->vnum));

	weapons = cJSON_CreateObject();
    cJSON_AddItemToObject(charData, "weapons", weapons);

    for( iHash = 0; iHash <= WEAPON_MAX; iHash++ )
    {
        weapon = cJSON_CreateObject();
        cJSON_AddItemToObject(weapons, attack_table[iHash], weapon);
        cJSON_AddItemToObject(weapon, "level", cJSON_CreateNumber(ch->wpn[iHash]));
        cJSON_AddItemToObject(weapon, "tier", cJSON_CreateNumber(ch->tier_wpn[iHash]));
    }

	spells = cJSON_CreateObject();
    cJSON_AddItemToObject(charData, "spells", spells);

    for( iHash = 0; iHash <= SPELL_MAX; iHash++ )
    {
        spell = cJSON_CreateObject();
        cJSON_AddItemToObject(spells, colornames[iHash], spell);
        cJSON_AddItemToObject(spell, "level", cJSON_CreateNumber(ch->spl[iHash]));
        cJSON_AddItemToObject(spell, "tier", cJSON_CreateNumber(ch->tier_spl[iHash]));
    }

    /**
    COMMENTED OUT ON 6/18 BECAUSE I HAVE NO IDEA WHAT THIS EVEN DOES
	fprintf(fp, "Combat       %d %d %d %d %d %d %d %d\n",
			ch->cmbt[0], ch->cmbt[1], ch->cmbt[2], ch->cmbt[3],
			ch->cmbt[4], ch->cmbt[5], ch->cmbt[6], ch->cmbt[7]);
    */

	stances = cJSON_CreateObject();
    cJSON_AddItemToObject(charData, "stances", stances);

    for( iHash = 0; iHash <= MAX_STANCE; iHash++ )
    {
        if( iHash == 0)
        {
            cJSON_AddItemToObject(stances, "current_stance", cJSON_CreateNumber(ch->stance[CURRENT_STANCE]));
            continue;
        }

		if( iHash == MAX_STANCE)
        {
            cJSON_AddItemToObject(stances, "autodrop", cJSON_CreateNumber(ch->stance[AUTODROP]));
            continue;
        }

        stance = cJSON_CreateObject();
        cJSON_AddItemToObject(stances, stancenames[iHash], stance);
        cJSON_AddItemToObject(stance, "level", cJSON_CreateNumber(ch->stance[iHash]));
        cJSON_AddItemToObject(stance, "tier", cJSON_CreateNumber(ch->tier_stance[iHash]));
    }

    locationHp = cJSON_CreateArray();
    cJSON_AddItemToObject(charData, "Locationhp", locationHp);
    cJSON_AddItemToArray(locationHp, cJSON_CreateNumber(ch->loc_hp[0]));
    cJSON_AddItemToArray(locationHp, cJSON_CreateNumber(ch->loc_hp[1]));
    cJSON_AddItemToArray(locationHp, cJSON_CreateNumber(ch->loc_hp[2]));
    cJSON_AddItemToArray(locationHp, cJSON_CreateNumber(ch->loc_hp[3]));
    cJSON_AddItemToArray(locationHp, cJSON_CreateNumber(ch->loc_hp[4]));
    cJSON_AddItemToArray(locationHp, cJSON_CreateNumber(ch->loc_hp[5]));
    cJSON_AddItemToArray(locationHp, cJSON_CreateNumber(ch->loc_hp[6]));

    cJSON_AddItemToObject(charData, "gold", cJSON_CreateNumber(ch->gold));
	cJSON_AddItemToObject(charData, "Bank", cJSON_CreateNumber(ch->bank));
	cJSON_AddItemToObject(charData, "Exp", cJSON_CreateNumber(ch->exp));
	cJSON_AddItemToObject(charData, "TierPoints", cJSON_CreateNumber(ch->tierpoints));
	cJSON_AddItemToObject(charData, "BloodPoints", cJSON_CreateNumber(ch->bloodpoints));
	cJSON_AddItemToObject(charData, "Act", cJSON_CreateNumber(ch->act));
	cJSON_AddItemToObject(charData, "Extra", cJSON_CreateNumber(ch->extra));
	cJSON_AddItemToObject(charData, "AffectedBy", cJSON_CreateNumber(ch->affected_by));
	cJSON_AddItemToObject(charData, "Position", cJSON_CreateNumber(ch->position == POS_FIGHTING ? POS_STANDING : ch->position));
	cJSON_AddItemToObject(charData, "Primal", cJSON_CreateNumber(ch->primal));
	cJSON_AddItemToObject(charData, "SavingThrow", cJSON_CreateNumber(ch->saving_throw));
	cJSON_AddItemToObject(charData, "Alignment", cJSON_CreateNumber(ch->alignment));
	cJSON_AddItemToObject(charData, "Hitroll", cJSON_CreateNumber(ch->hitroll));
	cJSON_AddItemToObject(charData, "Damroll", cJSON_CreateNumber(ch->damroll));
	cJSON_AddItemToObject(charData, "Dodge", cJSON_CreateNumber(ch->dodge));
	cJSON_AddItemToObject(charData, "Parry", cJSON_CreateNumber(ch->parry));
	cJSON_AddItemToObject(charData, "Block", cJSON_CreateNumber(ch->block));
	cJSON_AddItemToObject(charData, "Armor", cJSON_CreateNumber(ch->armor));
	cJSON_AddItemToObject(charData, "Wimpy", cJSON_CreateNumber(ch->wimpy));
	cJSON_AddItemToObject(charData, "Deaf", cJSON_CreateNumber(ch->deaf));
	cJSON_AddItemToObject(charData, "LagPenalty", cJSON_CreateNumber(ch->lagpenalty));

	if (IS_NPC(ch))
	{
		cJSON_AddItemToObject(charData, "Vnum", cJSON_CreateNumber(ch->pIndexData->vnum));
	}
	else
	{
		cJSON_AddItemToObject(charData, "Password", cJSON_CreateString(ch->pcdata->pwd));
		cJSON_AddItemToObject(charData, "Email", cJSON_CreateString(ch->pcdata->email));
		cJSON_AddItemToObject(charData, "Bamfin", cJSON_CreateString(ch->pcdata->bamfin));
		cJSON_AddItemToObject(charData, "Bamfout", cJSON_CreateString(ch->pcdata->bamfout));
		cJSON_AddItemToObject(charData, "Title", cJSON_CreateString(ch->pcdata->title));

		stats = cJSON_CreateObject();
		cJSON_AddItemToObject(charData, "stats", stats);
		cJSON_AddItemToObject(stats, "perm_str", cJSON_CreateNumber(ch->pcdata->perm_str));
		cJSON_AddItemToObject(stats, "perm_int", cJSON_CreateNumber(ch->pcdata->perm_int));
		cJSON_AddItemToObject(stats, "perm_wis", cJSON_CreateNumber(ch->pcdata->perm_wis));
		cJSON_AddItemToObject(stats, "perm_dex", cJSON_CreateNumber(ch->pcdata->perm_dex));
		cJSON_AddItemToObject(stats, "perm_con", cJSON_CreateNumber(ch->pcdata->perm_con));
		cJSON_AddItemToObject(stats, "mod_str", cJSON_CreateNumber(ch->pcdata->mod_str));
        cJSON_AddItemToObject(stats, "mod_int", cJSON_CreateNumber(ch->pcdata->mod_int));
        cJSON_AddItemToObject(stats, "mod_wis", cJSON_CreateNumber(ch->pcdata->mod_wis));
        cJSON_AddItemToObject(stats, "mod_dex", cJSON_CreateNumber(ch->pcdata->mod_dex));
        cJSON_AddItemToObject(stats, "mod_con", cJSON_CreateNumber(ch->pcdata->mod_con));

        cJSON_AddItemToObject(stats, "current_hp", cJSON_CreateNumber(ch->hit));
        cJSON_AddItemToObject(stats, "max_hp", cJSON_CreateNumber(ch->max_hit));
        cJSON_AddItemToObject(stats, "current_mana", cJSON_CreateNumber(ch->mana));
        cJSON_AddItemToObject(stats, "max_mana", cJSON_CreateNumber(ch->max_mana));
        cJSON_AddItemToObject(stats, "current_move", cJSON_CreateNumber(ch->move));
        cJSON_AddItemToObject(stats, "max_move", cJSON_CreateNumber(ch->max_move));

        cJSON_AddItemToObject(stats, "pkills", cJSON_CreateNumber(ch->pkill));
        cJSON_AddItemToObject(stats, "pdeaths", cJSON_CreateNumber(ch->pdeath));
        cJSON_AddItemToObject(stats, "mkills", cJSON_CreateNumber(ch->mkill));
        cJSON_AddItemToObject(stats, "mdeaths", cJSON_CreateNumber(ch->mdeath));

        cJSON_AddItemToObject(charData, "Quest", cJSON_CreateNumber(ch->pcdata->quest));

		if (ch->pcdata->obj_vnum != 0)
			cJSON_AddItemToObject(charData, "Objvnum", cJSON_CreateNumber(ch->pcdata->obj_vnum));

        condition = cJSON_CreateArray();
        cJSON_AddItemToObject(charData, "Condition", condition);
        cJSON_AddItemToArray(condition, cJSON_CreateNumber(ch->pcdata->condition[0]));
        cJSON_AddItemToArray(condition, cJSON_CreateNumber(ch->pcdata->condition[1]));
        cJSON_AddItemToArray(condition, cJSON_CreateNumber(ch->pcdata->condition[2]));

		skills = cJSON_CreateObject();
        cJSON_AddItemToObject(charData, "skills", skills);

        // Drop all of the single skills into the skills object
        for (iHash = 0; iHash < MAX_SKILL; iHash++)
            cJSON_AddItemToObject(skills, skill_table[iHash].name, cJSON_CreateNumber(ch->pcdata->learned[iHash]));
	}

	affect_datas = cJSON_CreateArray();
    cJSON_AddItemToObject(charData, "affect_data", affect_datas);

    for (paf = ch->affected; paf != NULL; paf = paf->next)
    {
        affect_data = cJSON_CreateObject();
        cJSON_AddItemToArray(affect_datas, affect_data);
        cJSON_AddItemToObject(affect_data, "name", cJSON_CreateString(skill_table[paf->type].name));
        cJSON_AddItemToObject(affect_data, "type", cJSON_CreateNumber(paf->type));
        cJSON_AddItemToObject(affect_data, "duration", cJSON_CreateNumber(paf->duration));
        cJSON_AddItemToObject(affect_data, "modifier", cJSON_CreateNumber(paf->modifier));
        cJSON_AddItemToObject(affect_data, "location", cJSON_CreateNumber(paf->location));
        cJSON_AddItemToObject(affect_data, "bitvector", cJSON_CreateNumber(paf->bitvector));
    }

	// Define the clandiscs array
	clandiscs = cJSON_CreateArray();

	// add the array to the character data
	cJSON_AddItemToObject(charData, "clandiscs", clandiscs);

	for(disc = ch->clandisc; disc != NULL; disc = disc->next)
	{
		clandisc = cJSON_CreateObject();
		cJSON_AddItemToArray(clandiscs, clandisc);

		cJSON_AddItemToObject(clandisc, "Name", cJSON_CreateString(disc->name));
		cJSON_AddItemToObject(clandisc, "Clandisc", cJSON_CreateString(disc->clandisc));
		cJSON_AddItemToObject(clandisc, "Tier", cJSON_CreateNumber(disc->tier));
		cJSON_AddItemToObject(clandisc, "PersonalMessageOn", cJSON_CreateString(disc->personal_message_on));
		cJSON_AddItemToObject(clandisc, "PersonalMessageOff", cJSON_CreateString(disc->personal_message_off));
		cJSON_AddItemToObject(clandisc, "RoomMessageOn", cJSON_CreateString(disc->room_message_on));
		cJSON_AddItemToObject(clandisc, "RoomMessageOff", cJSON_CreateString(disc->room_message_off));
		cJSON_AddItemToObject(clandisc, "VictimMessage", cJSON_CreateString(disc->victim_message));
		cJSON_AddItemToObject(clandisc, "Option", cJSON_CreateString(disc->option));
		cJSON_AddItemToObject(clandisc, "UpkeepMessage", cJSON_CreateString(disc->upkeepMessage));
		cJSON_AddItemToObject(clandisc, "Timeleft", cJSON_CreateNumber(disc->timeLeft));
		cJSON_AddItemToObject(clandisc, "IsActive", cJSON_CreateNumber(disc->isActive));
	}

	objects = cJSON_CreateArray();
	cJSON_AddItemToObject(charData, "objects", objects);

	if (ch->carrying != NULL)
		fwrite_obj(ch, ch->carrying, objects, 0);

    fprintf(fp, "%s", cJSON_Print(charData));
    cJSON_Delete(charData);
	return;
}

/*
* Write an object and its contents.
*/
void fwrite_obj(CHAR_DATA *ch, OBJ_DATA *obj, cJSON *objects , int iNest)
{
	EXTRA_DESCR_DATA *ed;
	AFFECT_DATA *paf;
	IMBUE_DATA *id;

	cJSON *object = NULL;
    cJSON *affect_datas = NULL;
    cJSON *affect_data = NULL;
    cJSON *imbue_datas = NULL;
    cJSON *imbue_data = NULL;
    cJSON *extra_descriptions = NULL;
    cJSON *extra_description = NULL;

	/*
    * Slick recursion to write lists backwards,
    *   so loading them will load in forwards order.
    */
	if (obj->next_content != NULL)
		fwrite_obj(ch, obj->next_content, objects, iNest);

	/*
	 * Castrate storage characters.
     */
	if ((obj->chobj != NULL && !IS_NPC(obj->chobj) &&
		 obj->chobj->pcdata->obj_vnum != 0) ||
		obj->item_type == ITEM_KEY
		/*  || obj->item_type == ITEM_PILL
           || obj->item_type == ITEM_SCROLL
           || obj->item_type == ITEM_POTION
           || obj->item_type == ITEM_WAND
           || obj->item_type == ITEM_STAFF*/
	)
		return;

	object = cJSON_CreateObject();
	cJSON_AddItemToArray(objects, object);
    cJSON_AddItemToObject(object, "Nest", cJSON_CreateNumber(iNest));
    cJSON_AddItemToObject(object, "Name", cJSON_CreateString(obj->name));
    cJSON_AddItemToObject(object, "ShortDescr", cJSON_CreateString(obj->short_descr));
    cJSON_AddItemToObject(object, "Description", cJSON_CreateString(obj->description));

    if (obj->chpoweron != NULL && str_cmp(obj->chpoweron, "(null)") && str_cmp(obj->chpoweron, ""))
        cJSON_AddItemToObject(object, "Poweronch", cJSON_CreateString(obj->chpoweron));

    if (obj->chpoweroff != NULL && str_cmp(obj->chpoweroff, "(null)") && str_cmp(obj->chpoweroff, ""))
        cJSON_AddItemToObject(object, "Poweroffch", cJSON_CreateString(obj->chpoweroff));

    if (obj->chpoweruse != NULL && str_cmp(obj->chpoweruse, "(null)") && str_cmp(obj->chpoweruse, ""))
        cJSON_AddItemToObject(object, "Powerusech", cJSON_CreateString(obj->chpoweruse));

    if (obj->victpoweron != NULL && str_cmp(obj->victpoweron, "(null)") && str_cmp(obj->victpoweron, ""))
        cJSON_AddItemToObject(object, "Poweronvict", cJSON_CreateString(obj->victpoweron));

    if (obj->victpoweroff != NULL && str_cmp(obj->victpoweroff, "(null)") && str_cmp(obj->victpoweroff, ""))
        cJSON_AddItemToObject(object, "Poweroffvict", cJSON_CreateString(obj->victpoweroff));

    if (obj->victpoweruse != NULL && str_cmp(obj->victpoweruse, "(null)") && str_cmp(obj->victpoweruse, ""))
        cJSON_AddItemToObject(object, "Powerusevict", cJSON_CreateString(obj->victpoweruse));

    if (obj->questmaker != NULL && strlen(obj->questmaker) > 1)
    {
    	cJSON_AddItemToObject(object, "Questmaker", cJSON_CreateString(obj->questmaker));
    }

	if (obj->questowner != NULL && strlen(obj->questowner) > 1)
	{
		cJSON_AddItemToObject(object, "Questowner", cJSON_CreateString(obj->questowner));
	}

    cJSON_AddItemToObject(object, "Vnum", cJSON_CreateNumber(obj->pIndexData->vnum));
    cJSON_AddItemToObject(object, "ExtraFlags", cJSON_CreateNumber(obj->extra_flags));
    cJSON_AddItemToObject(object, "WearFlags", cJSON_CreateNumber(obj->wear_flags));
    cJSON_AddItemToObject(object, "WearLoc", cJSON_CreateNumber(obj->wear_loc));
    cJSON_AddItemToObject(object, "ItemType", cJSON_CreateNumber(obj->item_type));
    cJSON_AddItemToObject(object, "Weight", cJSON_CreateNumber(obj->weight));

    if (obj->specpower != 0)
        cJSON_AddItemToObject(object, "Specpower", cJSON_CreateNumber(obj->specpower));

    cJSON_AddItemToObject(object, "Condition", cJSON_CreateNumber(obj->condition));
    cJSON_AddItemToObject(object, "Toughness", cJSON_CreateNumber(obj->toughness));
    cJSON_AddItemToObject(object, "Resistance", cJSON_CreateNumber(obj->resistance));
    cJSON_AddItemToObject(object, "Quest", cJSON_CreateNumber(obj->quest));
    cJSON_AddItemToObject(object, "Points", cJSON_CreateNumber(obj->points));

    cJSON_AddItemToObject(object, "Level", cJSON_CreateNumber(obj->level));
    cJSON_AddItemToObject(object, "Timer", cJSON_CreateNumber(obj->timer));
    cJSON_AddItemToObject(object, "Cost", cJSON_CreateNumber(obj->cost));

    cJSON_AddItemToObject(object, "value0", cJSON_CreateNumber(obj->value[0]));
    cJSON_AddItemToObject(object, "value1", cJSON_CreateNumber(obj->value[1]));
    cJSON_AddItemToObject(object, "value2", cJSON_CreateNumber(obj->value[2]));
    cJSON_AddItemToObject(object, "value3", cJSON_CreateNumber(obj->value[3]));

	if (obj->spectype != 0)
		cJSON_AddItemToObject(object, "Spectype", cJSON_CreateNumber(obj->spectype));

	switch (obj->item_type)
	{
        case ITEM_POTION:
        case ITEM_SCROLL:
            if (obj->value[1] > 0)
                cJSON_AddItemToObject(object, "Spell 1", cJSON_CreateString(skill_table[obj->value[1]].name));

            if (obj->value[2] > 0)
                cJSON_AddItemToObject(object, "Spell ", cJSON_CreateString(skill_table[obj->value[2]].name));

            if (obj->value[3] > 0)
                cJSON_AddItemToObject(object, "Spell 3", cJSON_CreateString(skill_table[obj->value[3]].name));

            break;

        case ITEM_PILL:
        case ITEM_STAFF:
        case ITEM_WAND:
            if (obj->value[3] > 0)
                cJSON_AddItemToObject(object, "Spell 3", cJSON_CreateString(skill_table[obj->value[3]].name));

            break;
	}

    affect_datas = cJSON_CreateArray();
    cJSON_AddItemToObject(object, "affects", affect_datas);

	for (paf = obj->affected; paf != NULL; paf = paf->next)
	{
	    affect_data = cJSON_CreateObject();
	    cJSON_AddItemToArray(affect_datas, affect_data);
	    cJSON_AddItemToObject(affect_data, "type", cJSON_CreateString(skill_table[paf->type].name));
	    cJSON_AddItemToObject(affect_data, "duration", cJSON_CreateNumber(paf->duration));
	    cJSON_AddItemToObject(affect_data, "modifier", cJSON_CreateNumber(paf->modifier));
	    cJSON_AddItemToObject(affect_data, "location", cJSON_CreateNumber(paf->location));
	    cJSON_AddItemToObject(affect_data, "bitvector", cJSON_CreateNumber(paf->bitvector));
	}

	extra_descriptions = cJSON_CreateArray();
    cJSON_AddItemToObject(object, "extra_descriptions", extra_descriptions);
	for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
	{
	    extra_description = cJSON_CreateObject();
        cJSON_AddItemToArray(extra_descriptions, extra_description);
		cJSON_AddItemToObject(extra_description, "keyword", cJSON_CreateString(ed->keyword));
		cJSON_AddItemToObject(extra_description, "description", cJSON_CreateString(ed->description));
	}


    imbue_datas = cJSON_CreateArray();
    cJSON_AddItemToObject(object, "imbues", imbue_datas);
	for (id = obj->imbue; id != NULL; id = id->next)
	{
	    imbue_data = cJSON_CreateObject();
	    cJSON_AddItemToArray(imbue_datas, imbue_data);
	    cJSON_AddItemToObject(imbue_data, "name", cJSON_CreateString(id->name));
	    cJSON_AddItemToObject(imbue_data, "item_type", cJSON_CreateString(id->item_type));
	    cJSON_AddItemToObject(imbue_data, "affect_number", cJSON_CreateNumber(id->affect_number));
	}


	if (obj->contains != NULL)
		fwrite_obj(ch, obj->contains, objects, iNest + 1);

	return;
}

/*
* Load a char and inventory into a new ch structure.
*/
bool load_char_obj(DESCRIPTOR_DATA *d, char *name)
{
	static PC_DATA pcdata_zero;
	char strsave[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	FILE *fp;
	bool found;
	char buf[MAX_STRING_LENGTH];
	int i;

	if (char_free == NULL)
	{
		ch = alloc_perm(sizeof(*ch));
	}
	else
	{
		ch = char_free;
		char_free = char_free->next;
	}
	clear_char(ch);

	if (pcdata_free == NULL)
	{
		ch->pcdata = alloc_perm(sizeof(*ch->pcdata));
	}
	else
	{
		ch->pcdata = pcdata_free;
		pcdata_free = pcdata_free->next;
	}
	*ch->pcdata = pcdata_zero;

	d->character = ch;
	ch->desc = d;
	ch->host_ip = d->host_ip;
	ch->name = str_dup(name);
	ch->act = PLR_BLANK | PLR_COMBINE | PLR_PROMPT;
	ch->prompt = str_dup("<%hhp %mm %vmv> ");
	ch->extra = 0;
	ch->pcdata->pwd = str_dup("");
	ch->pcdata->email = str_dup("");
	ch->pcdata->hunting = char_free;
	ch->pcdata->bamfin = str_dup("");
	ch->pcdata->bamfout = str_dup("");
	ch->pcdata->title = str_dup("");
	ch->pcdata->tell_history = alloc_perm(sizeof(*ch->pcdata->tell_history));
	ch->pcdata->chat_history = alloc_perm(sizeof(*ch->pcdata->chat_history));
	ch->pcdata->newbie_history = alloc_perm(sizeof(*ch->pcdata->newbie_history));
	ch->pcdata->clan_history = alloc_perm(sizeof(*ch->pcdata->clan_history));
	ch->pcdata->immortal_history = alloc_perm(sizeof(*ch->pcdata->immortal_history));

	for (i = 0; i < REVIEW_HISTORY_SIZE; ++i)
	{
		ch->pcdata->tell_history->history[i] = str_dup("");
		ch->pcdata->chat_history->history[i] = str_dup("");
		ch->pcdata->newbie_history->history[i] = str_dup("");
		ch->pcdata->clan_history->history[i] = str_dup("");
		ch->pcdata->immortal_history->history[i] = str_dup("");
	}

	ch->pcdata->tell_history->position = 0;
	ch->pcdata->chat_history->position = 0;
	ch->pcdata->newbie_history->position = 0;
	ch->pcdata->clan_history->position = 0;
	ch->pcdata->immortal_history->position = 0;
	ch->lord = str_dup("");
	ch->clan = str_dup("");
	ch->morph = str_dup("");
	ch->pload = str_dup("");
	ch->createtime = str_dup(ctime(&current_time));
	ch->lasttime = str_dup("");
	ch->lasthost = str_dup("");
	ch->poweraction = str_dup("");
	ch->powertype = str_dup("");
	ch->remortlevel = 0;
	ch->spectype = 0;
	ch->specpower = 0;
	ch->mounted = 0;
	ch->home = 3001;
	ch->vampgen = 0;
	ch->vampaff = 0;
	ch->itemaffect = 0;
	ch->vamppass = -1;
	ch->polyaff = 0;
	ch->immune = 0;
	ch->form = 32767;
	ch->beast = 50;
	ch->loc_hp[0] = 0;
	ch->loc_hp[1] = 0;
	ch->loc_hp[2] = 0;
	ch->loc_hp[3] = 0;
	ch->loc_hp[4] = 0;
	ch->loc_hp[5] = 0;
	ch->loc_hp[6] = 0;
	ch->wpn[WEAPON_HIT] = 0;
	ch->wpn[WEAPON_SLICE] = 0;
	ch->wpn[WEAPON_STAB] = 0;
	ch->wpn[WEAPON_SLASH] = 0;
	ch->wpn[WEAPON_WHIP] = 0;
	ch->wpn[WEAPON_CLAW] = 0;
	ch->wpn[WEAPON_BLAST] = 0;
	ch->wpn[WEAPON_POUND] = 0;
	ch->wpn[WEAPON_CRUSH] = 0;
	ch->wpn[WEAPON_BITE] = 0;
	ch->wpn[WEAPON_GREP] = 0;
	ch->wpn[WEAPON_PIERCE] = 0;
	ch->wpn[WEAPON_SUCK] = 0;
	ch->spl[SPELL_PURPLE] = 4;
	ch->spl[SPELL_RED] = 4;
	ch->spl[SPELL_BLUE] = 4;
	ch->spl[SPELL_GREEN] = 4;
	ch->spl[SPELL_YELLOW] = 4;
	ch->cmbt[0] = 0;
	ch->cmbt[1] = 0;
	ch->cmbt[2] = 0;
	ch->cmbt[3] = 0;
	ch->cmbt[4] = 0;
	ch->cmbt[5] = 0;
	ch->cmbt[6] = 0;
	ch->cmbt[7] = 0;
	ch->tier_clandisc[0] = 0;
	ch->tier_clandisc[1] = 0;
	ch->tier_clandisc[2] = 0;
	ch->tier_clandisc[3] = 0;
	ch->tier_clandisc[4] = 0;
	ch->tier_clandisc[5] = 0;
	ch->tier_clandisc[6] = 0;
	ch->tier_clandisc[7] = 0;
	ch->tier_clandisc[8] = 0;
	ch->tier_clandisc[9] = 0;
	ch->tier_clandisc[10] = 0;
	ch->tier_clandisc[11] = 0;
	ch->tier_stance[STANCE_VIPER] = 0;
	ch->tier_stance[STANCE_CRANE] = 0;
	ch->tier_stance[STANCE_FALCON] = 0;
	ch->tier_stance[STANCE_MONGOOSE] = 0;
	ch->tier_stance[STANCE_BULL] = 0;
	ch->tier_stance[STANCE_SWALLOW] = 0;
	ch->tier_stance[STANCE_COBRA] = 0;
	ch->tier_stance[STANCE_LION] = 0;
	ch->tier_stance[STANCE_GRIZZLIE] = 0;
	ch->tier_stance[STANCE_PANTHER] = 0;
	ch->tier_stance[AUTODROP] = 0;
	ch->tier_wpn[WEAPON_HIT] = 0;
	ch->tier_wpn[WEAPON_SLICE] = 0;
	ch->tier_wpn[WEAPON_STAB] = 0;
	ch->tier_wpn[WEAPON_SLASH] = 0;
	ch->tier_wpn[WEAPON_WHIP] = 0;
	ch->tier_wpn[WEAPON_CLAW] = 0;
	ch->tier_wpn[WEAPON_BLAST] = 0;
	ch->tier_wpn[WEAPON_POUND] = 0;
	ch->tier_wpn[WEAPON_CRUSH] = 0;
	ch->tier_wpn[WEAPON_BITE] = 0;
	ch->tier_wpn[WEAPON_GREP] = 0;
	ch->tier_wpn[WEAPON_PIERCE] = 0;
	ch->tier_wpn[WEAPON_SUCK] = 0;
	ch->tier_spl[SPELL_PURPLE] = 4;
	ch->tier_spl[SPELL_RED] = 4;
	ch->tier_spl[SPELL_BLUE] = 4;
	ch->tier_spl[SPELL_GREEN] = 4;
	ch->tier_spl[SPELL_YELLOW] = 4;
	ch->stance[CURRENT_STANCE] = 0;
	ch->stance[STANCE_VIPER] = 0;
	ch->stance[STANCE_CRANE] = 0;
	ch->stance[STANCE_FALCON] = 0;
	ch->stance[STANCE_MONGOOSE] = 0;
	ch->stance[STANCE_BULL] = 0;
	ch->stance[STANCE_SWALLOW] = 0;
	ch->stance[STANCE_COBRA] = 0;
	ch->stance[STANCE_LION] = 0;
	ch->stance[STANCE_GRIZZLIE] = 0;
	ch->stance[STANCE_PANTHER] = 0;
	ch->stance[AUTODROP] = 0;
	ch->pkill = 0;
	ch->pdeath = 0;
	ch->mkill = 0;
	ch->mdeath = 0;
	ch->hitroll = 0;
	ch->damroll = 0;
	ch->dodge = 0;
	ch->parry = 0;
	ch->block = 0;
	ch->pcdata->perm_str = 13;
	ch->pcdata->perm_int = 13;
	ch->pcdata->perm_wis = 13;
	ch->pcdata->perm_dex = 13;
	ch->pcdata->perm_con = 13;
	ch->pcdata->quest = 0;
	ch->pcdata->obj_vnum = 0;
	ch->pcdata->condition[COND_THIRST] = 48;
	ch->pcdata->condition[COND_FULL] = 48;
	ch->gold = 0;
	ch->bank = 0;
	ch->choke_dam_message = 0;

	found = FALSE;
	fclose(fpReserve);
#if !defined(macintosh) && !defined(MSDOS)
	snprintf(strsave, MAX_INPUT_LENGTH, "%s%s%s%s%s", PLAYER_DIR, initial(ch->name),
			 "/", capitalize(ch->name), ".gz");
	if ((fp = fopen(strsave, "r")))
	{
		fclose(fp);
		snprintf(buf, MAX_INPUT_LENGTH, "gzip -dfq %s", strsave);
		int systemReturn = system(buf);
		if (systemReturn == -1)
		{
			bug("Could not load character", 0);
			return FALSE;
		}
	}
#endif


/**
 * Hijacking the function to first try to load JSON
 * and if that isn't available, fall back to the old
 * Merc player files
 */

    snprintf(strsave, MAX_INPUT_LENGTH, "%s%s%s%s.json", PLAYER_DIR, initial(ch->name), "/", capitalize(ch->name));
    if ((fp = fopen(strsave, "r")) != NULL)
    {
        fseek(fp, 0, SEEK_END);
        long fsize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char *data = malloc(fsize + 1);
        int result = fread(data, fsize, 1, fp);


        data[fsize] = 0;

        cJSON *jChar = cJSON_Parse(data);

        if (jChar == NULL)
        {
            const char *error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != NULL)
            {
                fprintf(stderr, "Error before: %s and result %d\n", error_ptr, result);
            }
            log_string("Error in jChar");
            cJSON_Delete(jChar);
            exit(1);
        }

        ch->name = cJSON_GetObjectItemCaseSensitive(jChar, "Name")->valuestring;
        ch->short_descr = cJSON_GetObjectItemCaseSensitive(jChar, "ShortDescr")->valuestring;
        ch->long_descr = cJSON_GetObjectItemCaseSensitive(jChar, "LongDescr")->valuestring;
        ch->description = cJSON_GetObjectItemCaseSensitive(jChar, "Description")->valuestring;

        // TODO: This should only be grabbed if it's in the JSON
        ch->prompt = cJSON_GetObjectItemCaseSensitive(jChar, "Prompt")->valuestring;

        ch->lord = cJSON_GetObjectItemCaseSensitive(jChar, "Lord")->valuestring;
        ch->clan = cJSON_GetObjectItemCaseSensitive(jChar, "Clan")->valuestring;
        ch->morph = cJSON_GetObjectItemCaseSensitive(jChar, "Morph")->valuestring;
        ch->createtime = cJSON_GetObjectItemCaseSensitive(jChar, "Createtime")->valuestring;
        ch->lasttime = cJSON_GetObjectItemCaseSensitive(jChar, "Lasttime")->valuestring;
        ch->lasthost = cJSON_GetObjectItemCaseSensitive(jChar, "Lasthost")->valuestring;
        ch->poweraction = cJSON_GetObjectItemCaseSensitive(jChar, "Poweraction")->valuestring;
        ch->powertype = cJSON_GetObjectItemCaseSensitive(jChar, "Powertype")->valuestring;
        ch->sex = cJSON_GetObjectItemCaseSensitive(jChar, "Sex")->valuedouble;
        ch->class = cJSON_GetObjectItemCaseSensitive(jChar, "Class")->valuedouble;
        ch->status = cJSON_GetObjectItemCaseSensitive(jChar, "Status")->valuedouble;
        ch->pk_enabled = cJSON_GetObjectItemCaseSensitive(jChar, "PKEnabled")->valuedouble;
        ch->remortlevel = cJSON_GetObjectItemCaseSensitive(jChar, "RemortLevel")->valuedouble;
        ch->immune = cJSON_GetObjectItemCaseSensitive(jChar, "Immune")->valuedouble;
        ch->polyaff = cJSON_GetObjectItemCaseSensitive(jChar, "Polyaff")->valuedouble;
        ch->itemaffect = cJSON_GetObjectItemCaseSensitive(jChar, "Itemaffect")->valuedouble;
        ch->vampaff = cJSON_GetObjectItemCaseSensitive(jChar, "Vampaff")->valuedouble;
        ch->vamppass = cJSON_GetObjectItemCaseSensitive(jChar, "Vamppass")->valuedouble;
        ch->form = cJSON_GetObjectItemCaseSensitive(jChar, "Form")->valuedouble;
        ch->beast = cJSON_GetObjectItemCaseSensitive(jChar, "Beast")->valuedouble;
        ch->vampgen = cJSON_GetObjectItemCaseSensitive(jChar, "Vampgen")->valuedouble;
        ch->spectype = cJSON_GetObjectItemCaseSensitive(jChar, "Spectype")->valuedouble;
        ch->specpower = cJSON_GetObjectItemCaseSensitive(jChar, "Specpower")->valuedouble;
        ch->home = cJSON_GetObjectItemCaseSensitive(jChar, "Home")->valuedouble;
        ch->level = cJSON_GetObjectItemCaseSensitive(jChar, "Level")->valuedouble;
        ch->trust = cJSON_GetObjectItemCaseSensitive(jChar, "Trust")->valuedouble;
        ch->played = cJSON_GetObjectItemCaseSensitive(jChar, "Played")->valuedouble;

        ch->in_room = cJSON_GetObjectItemCaseSensitive(jChar, "Room")->valueint;
		if (ch->in_room == NULL)
			ch->in_room = get_room_index(ROOM_VNUM_LIMBO);

        /**
        COMMENTED OUT ON 6/18 BECAUSE I HAVE NO IDEA WHAT THIS EVEN DOES
        fprintf(fp, "Combat       %d %d %d %d %d %d %d %d\n",
                ch->cmbt[0], ch->cmbt[1], ch->cmbt[2], ch->cmbt[3],
                ch->cmbt[4], ch->cmbt[5], ch->cmbt[6], ch->cmbt[7]);
        */

        // TODO: Add in the Stances section here
        /*
        stances = cJSON_CreateObject();
        cJSON_AddItemToObject(charData, "stances", stances);

        for( iHash = 0; iHash < MAX_STANCE; iHash++ )
        {
            if( iHash == 0)
            {
                cJSON_AddItemToObject(stances, "current_stance", cJSON_CreateNumber(ch->stance[CURRENT_STANCE]));
                break;
            }


            if( iHash == MAX_STANCE)
            {
                cJSON_AddItemToObject(stances, "autodrop", cJSON_CreateNumber(ch->stance[AUTODROP]));
                break;
            }

            stance = cJSON_CreateObject();
            cJSON_AddItemToObject(stances, stancenames[iHash], stance);
            cJSON_AddItemToObject(stance, "level", cJSON_CreateNumber(ch->stance[iHash]));
            cJSON_AddItemToObject(stance, "tier", cJSON_CreateNumber(ch->tier_stance[iHash]));
        }
        */
        // TODO: Add in the locationHp section here
        /*
        locationHp = cJSON_CreateArray();
        cJSON_AddItemToObject(charData, "Locationhp", locationHp);
        cJSON_AddItemToArray(locationHp, cJSON_CreateNumber(ch->loc_hp[0]));
        cJSON_AddItemToArray(locationHp, cJSON_CreateNumber(ch->loc_hp[1]));
        cJSON_AddItemToArray(locationHp, cJSON_CreateNumber(ch->loc_hp[2]));
        cJSON_AddItemToArray(locationHp, cJSON_CreateNumber(ch->loc_hp[3]));
        cJSON_AddItemToArray(locationHp, cJSON_CreateNumber(ch->loc_hp[4]));
        cJSON_AddItemToArray(locationHp, cJSON_CreateNumber(ch->loc_hp[5]));
        cJSON_AddItemToArray(locationHp, cJSON_CreateNumber(ch->loc_hp[6]));
        */


        ch->gold = cJSON_GetObjectItemCaseSensitive(jChar, "gold")->valuedouble;
        ch->bank = cJSON_GetObjectItemCaseSensitive(jChar, "Bank")->valuedouble;
        ch->exp = cJSON_GetObjectItemCaseSensitive(jChar, "Exp")->valuedouble;
        ch->tierpoints = cJSON_GetObjectItemCaseSensitive(jChar, "TierPoints")->valuedouble;
        ch->bloodpoints = cJSON_GetObjectItemCaseSensitive(jChar, "BloodPoints")->valuedouble;
        ch->act = cJSON_GetObjectItemCaseSensitive(jChar, "Act")->valuedouble;
        ch->extra = cJSON_GetObjectItemCaseSensitive(jChar, "Extra")->valuedouble;
        ch->affected_by = cJSON_GetObjectItemCaseSensitive(jChar, "AffectedBy")->valuedouble;
        ch->position = cJSON_GetObjectItemCaseSensitive(jChar, "Position")->valuedouble;
        ch->primal = cJSON_GetObjectItemCaseSensitive(jChar, "Primal")->valuedouble;
        ch->saving_throw = cJSON_GetObjectItemCaseSensitive(jChar, "SavingThrow")->valuedouble;
        ch->alignment = cJSON_GetObjectItemCaseSensitive(jChar, "Alignment")->valuedouble;
        ch->hitroll = cJSON_GetObjectItemCaseSensitive(jChar, "Hitroll")->valuedouble;
        ch->damroll = cJSON_GetObjectItemCaseSensitive(jChar, "Damroll")->valuedouble;
        ch->dodge = cJSON_GetObjectItemCaseSensitive(jChar, "Dodge")->valuedouble;
        ch->parry = cJSON_GetObjectItemCaseSensitive(jChar, "Parry")->valuedouble;
        ch->block = cJSON_GetObjectItemCaseSensitive(jChar, "Block")->valuedouble;
        ch->armor = cJSON_GetObjectItemCaseSensitive(jChar, "Armor")->valuedouble;
        ch->wimpy = cJSON_GetObjectItemCaseSensitive(jChar, "Wimpy")->valuedouble;
        ch->deaf = cJSON_GetObjectItemCaseSensitive(jChar, "Deaf")->valuedouble;
        ch->lagpenalty = cJSON_GetObjectItemCaseSensitive(jChar, "LagPenalty")->valuedouble;

        /**
         * Handle all of the PC data
         */
        ch->pcdata->pwd = cJSON_GetObjectItemCaseSensitive(jChar, "Password")->valuestring;
        ch->pcdata->email = cJSON_GetObjectItemCaseSensitive(jChar, "Email")->valuestring;
        ch->pcdata->bamfin = cJSON_GetObjectItemCaseSensitive(jChar, "Bamfin")->valuestring;
        ch->pcdata->bamfout = cJSON_GetObjectItemCaseSensitive(jChar, "Bamfout")->valuestring;
        ch->pcdata->title = cJSON_GetObjectItemCaseSensitive(jChar, "Title")->valuestring;


        cJSON *stats;
        stats = cJSON_GetObjectItemCaseSensitive(jChar, "stats");

        ch->pcdata->perm_str = cJSON_GetObjectItemCaseSensitive(stats, "perm_str")->valuedouble;
        ch->pcdata->perm_int = cJSON_GetObjectItemCaseSensitive(stats, "perm_int")->valuedouble;
        ch->pcdata->perm_wis = cJSON_GetObjectItemCaseSensitive(stats, "perm_wis")->valuedouble;
        ch->pcdata->perm_dex = cJSON_GetObjectItemCaseSensitive(stats, "perm_dex")->valuedouble;
        ch->pcdata->perm_con = cJSON_GetObjectItemCaseSensitive(stats, "perm_con")->valuedouble;
        ch->pcdata->mod_str = cJSON_GetObjectItemCaseSensitive(stats, "mod_str")->valuedouble;
        ch->pcdata->mod_int = cJSON_GetObjectItemCaseSensitive(stats, "mod_int")->valuedouble;
        ch->pcdata->mod_wis = cJSON_GetObjectItemCaseSensitive(stats, "mod_wis")->valuedouble;
        ch->pcdata->mod_dex = cJSON_GetObjectItemCaseSensitive(stats, "mod_dex")->valuedouble;
        ch->pcdata->mod_con = cJSON_GetObjectItemCaseSensitive(stats, "mod_con")->valuedouble;

        ch->hit = cJSON_GetObjectItemCaseSensitive(stats, "current_hp")->valuedouble;
        ch->max_hit = cJSON_GetObjectItemCaseSensitive(stats, "max_hp")->valuedouble;
        ch->mana = cJSON_GetObjectItemCaseSensitive(stats, "current_mana")->valuedouble;
        ch->max_mana = cJSON_GetObjectItemCaseSensitive(stats, "max_mana")->valuedouble;
        ch->move = cJSON_GetObjectItemCaseSensitive(stats, "current_move")->valuedouble;
        ch->max_move = cJSON_GetObjectItemCaseSensitive(stats, "max_move")->valuedouble;

        ch->pkill = cJSON_GetObjectItemCaseSensitive(stats, "pkills")->valuedouble;
        ch->pdeath = cJSON_GetObjectItemCaseSensitive(stats, "pdeaths")->valuedouble;
        ch->mkill = cJSON_GetObjectItemCaseSensitive(stats, "mkills")->valuedouble;
        ch->mdeath = cJSON_GetObjectItemCaseSensitive(stats, "mdeaths")->valuedouble;

        ch->pcdata->quest = cJSON_GetObjectItemCaseSensitive(jChar, "Quest")->valuedouble;

        load_char_clandiscs_json(cJSON_GetObjectItemCaseSensitive(jChar, "clandiscs"), ch);

		load_char_objects_json(cJSON_GetObjectItemCaseSensitive(jChar, "objects"), ch);

        load_char_affects_json(cJSON_GetObjectItemCaseSensitive(jChar, "affect_data"), ch);

        load_char_stances_json(cJSON_GetObjectItemCaseSensitive(jChar, "stances"), ch);

        load_char_weapons_json(cJSON_GetObjectItemCaseSensitive(jChar, "weapons"), ch);

        load_char_spells_json(cJSON_GetObjectItemCaseSensitive(jChar, "spells"), ch);

		load_char_skills_json(cJSON_GetObjectItemCaseSensitive(jChar, "skills"), ch);

        fclose(fp);
        fpReserve = fopen(NULL_FILE, "r");

        return TRUE;
    }

#if !defined(macintosh) && !defined(MSDOS)
	snprintf(strsave, MAX_INPUT_LENGTH, "%s%s%s%s", PLAYER_DIR, initial(ch->name), "/", capitalize(ch->name));
#else
	snprintf(strsave, MAX_INPUT_LENGTH, "%s%s", PLAYER_DIR, capitalize(name));
#endif

	if ((fp = fopen(strsave, "r")) != NULL)
	{
		int iNest;

		for (iNest = 0; iNest < MAX_NEST; iNest++)
			rgObjNest[iNest] = NULL;

		found = TRUE;
		for (;;)
		{
			char letter;
			char *word;

			letter = fread_letter(fp);
			if (letter == '*')
			{
				fread_to_eol(fp);
				continue;
			}

			if (letter != '#')
			{
				bug("Load_char_obj: # not found.", 0);
				return FALSE;
				break;
			}

			word = fread_word(fp);
			if (!str_cmp(word, "PLAYER"))
				fread_char(ch, fp);
			else if (!str_cmp(word, "OBJECT"))
				fread_obj(ch, fp);
			else if (!str_cmp(word, "CLANDISC"))
				fread_clandisc(ch, fp);
			else if (!str_cmp(word, "END"))
				break;
			else
			{
				bug("Load_char_obj: bad section.", 0);
				return FALSE;
				break;
			}
		}
		fclose(fp);
	}

	fpReserve = fopen(NULL_FILE, "r");
	return found;
}

/*
* Read in a char.
*/

#if defined(KEY)
#undef KEY
#endif
/* 
#define KEY( literal, field, value, ROE, default )   	\				
if ( !str_cmp( word, literal ) )	       	\	
{			 				 	\	
fMatch = TRUE;						\		
field  = value;						\
break;							\
	} */

#define KEY(literal, field, value, ROE, default) \
	if (!str_cmp(word, literal))                 \
	{                                            \
		field = value;                           \
		fMatch = TRUE;                           \
		break;                                   \
	}
/*
	int validate_number(int value,int ROE,int default)
	{
	
	  if( value == ROE)
	  {
	  errordetect = TRUE;
	  return default;
	  }
	  fMatch = TRUE;
	  return value;
	  }
	  */
#define KEYS(literal, field, value) \
	if (!str_cmp(word, literal))    \
	{                               \
		field = value;              \
		fMatch = TRUE;              \
		break;                      \
	}

void fread_char(CHAR_DATA *ch, FILE *fp)
{
	char buf[MAX_STRING_LENGTH];
	char errormess[MAX_STRING_LENGTH];
	char *word;
	bool fMatch;
	bool errordetect = FALSE;

	for (;;)
	{
		word = feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;

		switch (UPPER(word[0]))
		{
		case '*':
			fMatch = TRUE;
			fread_to_eol(fp);
			break;

		case 'A':
			if (!str_cmp(word, "Act"))
			{
				ch->act = fread_number(fp, -999);
				if (ch->act == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Act \n\r");
					ch->act = PLR_BLANK | PLR_COMBINE | PLR_PROMPT;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "AffectedBy"))
			{
				ch->affected_by = fread_number(fp, -999);
				if (ch->affected_by == -999)
				{
					errordetect = TRUE;
					ch->affected_by = 0;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Act \n\r");
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Alignment"))
			{
				ch->alignment = fread_number(fp, -9999);
				if (ch->alignment < -1000 || ch->alignment > 1000)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Alignment \n\r");
					ch->alignment = 0;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Armor"))
			{
				ch->armor = fread_number(fp, -9999);
				if (ch->armor == -9999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Armor \n\r");
					ch->armor = 0;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Affect") || !str_cmp(word, "AffectData"))
			{
				AFFECT_DATA *paf;

				if (affect_free == NULL)
				{
					paf = alloc_perm(sizeof(*paf));
				}
				else
				{
					paf = affect_free;
					affect_free = affect_free->next;
				}

				if (!str_cmp(word, "Affect"))
				{
					/* Obsolete 2.0 form. */
					paf->type = fread_number(fp, -9999);
					if (paf->type == -9999)
						errordetect = TRUE;
				}
				else
				{
					int sn;

					sn = skill_lookup(fread_word(fp));
					if (sn < 0)
						bug("Fread_char: unknown skill.", 0);
					else
						paf->type = sn;
				}

				paf->duration = fread_number(fp, -9999);
				if (paf->duration == -9999)
				{
					errordetect = TRUE;
					paf->duration = 1;
				}
				paf->modifier = fread_number(fp, -9999);
				if (paf->modifier == -9999)
				{
					errordetect = TRUE;
					paf->modifier = 0;
				}
				paf->location = fread_number(fp, -9999);
				if (paf->location == -9999)
				{
					errordetect = TRUE;
					paf->location = 1;
				}
				paf->bitvector = fread_number(fp, -9999);
				if (paf->bitvector == -9999)
				{
					errordetect = TRUE;
					paf->bitvector = 0;
				}
				paf->next = ch->affected;
				ch->affected = paf;
				if (errordetect)
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Affect(Data) \n\r");
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "AttrMod"))
			{
				ch->pcdata->mod_str = fread_number(fp, -999);
				if (ch->pcdata->mod_str == -999)
				{
					errordetect = TRUE;
					ch->pcdata->mod_str = 0;
				}
				ch->pcdata->mod_int = fread_number(fp, -999);
				if (ch->pcdata->mod_int == -999)
				{
					errordetect = TRUE;
					ch->pcdata->mod_int = 0;
				}
				ch->pcdata->mod_wis = fread_number(fp, -999);
				if (ch->pcdata->mod_wis == -999)
				{
					errordetect = TRUE;
					ch->pcdata->mod_wis = 0;
				}
				ch->pcdata->mod_dex = fread_number(fp, -999);
				if (ch->pcdata->mod_dex == -999)
				{
					errordetect = TRUE;
					ch->pcdata->mod_dex = 0;
				}
				ch->pcdata->mod_con = fread_number(fp, -999);
				if (ch->pcdata->mod_con == -999)
				{
					errordetect = TRUE;
					ch->pcdata->mod_con = 0;
				}
				if (errordetect)
					snprintf(errormess, MAX_STRING_LENGTH, "Error in AttrMod \n\r");
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "AttrPerm"))
			{
				ch->pcdata->perm_str = fread_number(fp, -999);
				if (ch->pcdata->perm_str == -999)
				{
					errordetect = TRUE;
					ch->pcdata->perm_str = 13;
				}
				ch->pcdata->perm_int = fread_number(fp, -999);
				if (ch->pcdata->perm_int == -999)
				{
					errordetect = TRUE;
					ch->pcdata->perm_int = 13;
				}
				ch->pcdata->perm_wis = fread_number(fp, -999);
				if (ch->pcdata->perm_wis == -999)
				{
					errordetect = TRUE;
					ch->pcdata->perm_wis = 13;
				}
				ch->pcdata->perm_dex = fread_number(fp, -999);
				if (ch->pcdata->perm_dex == -999)
				{
					errordetect = TRUE;
					ch->pcdata->perm_dex = 13;
				}
				ch->pcdata->perm_con = fread_number(fp, -999);
				if (ch->pcdata->perm_con == -999)
				{
					errordetect = TRUE;
					ch->pcdata->perm_con = 13;
				}
				if (errordetect)
					snprintf(errormess, MAX_STRING_LENGTH, "Error in AttrPerm \n\r");
				fMatch = TRUE;
				break;
			}
			break;

		case 'B':
			KEYS("Bamfin", ch->pcdata->bamfin, fread_string(fp));
			KEYS("Bamfout", ch->pcdata->bamfout, fread_string(fp));
			if (!str_cmp(word, "Bank"))
			{
				ch->bank = fread_number(fp, -999);
				if (ch->bank == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Bank \n\r");
					ch->bank = 0;
				}
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Beast"))
			{
				ch->beast = fread_number(fp, -999);
				if (ch->beast > 100 || ch->beast < 0)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Beast \n\r");
					bug(errormess, 0);
					ch->beast = 50;
				}
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Block"))
			{
				ch->block = fread_number(fp, -999);
				if (ch->block == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Block \n\r");
					ch->block = 0;
				}
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "BloodPoints"))
            {
                ch->bloodpoints = fread_number(fp, -999);
                if (ch->bloodpoints == -999)
                {
                    errordetect = TRUE;
                    snprintf(errormess, MAX_STRING_LENGTH, "Error in BloodPoints \n\r");
                    ch->bloodpoints = 0;
                }
                fMatch = TRUE;
                break;
            }
			break;

		case 'C':
			KEYS("Clan", ch->clan, fread_string(fp));

			if (!str_cmp(word, "Class"))
			{
				ch->class = fread_number(fp, -999);
				if (ch->class == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Class \n\r");
					ch->class = 0;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Combat"))
			{
				ch->cmbt[0] = fread_number(fp, -999);
				if (ch->cmbt[0] < 0 || ch->cmbt[0] > 200)
				{
					errordetect = TRUE;
					ch->cmbt[0] = 0;
				}
				ch->cmbt[1] = fread_number(fp, -999);
				if (ch->cmbt[1] < 0 || ch->cmbt[1] > 200)
				{
					errordetect = TRUE;
					ch->cmbt[1] = 0;
				}
				ch->cmbt[2] = fread_number(fp, -999);
				if (ch->cmbt[2] < 0 || ch->cmbt[2] > 200)
				{
					errordetect = TRUE;
					ch->cmbt[2] = 0;
				}
				ch->cmbt[3] = fread_number(fp, -999);
				if (ch->cmbt[3] < 0 || ch->cmbt[3] > 200)
				{
					errordetect = TRUE;
					ch->cmbt[3] = 0;
				}
				ch->cmbt[4] = fread_number(fp, -999);
				if (ch->cmbt[4] < 0 || ch->cmbt[4] > 200)
				{
					errordetect = TRUE;
					ch->cmbt[4] = 0;
				}
				ch->cmbt[5] = fread_number(fp, -999);
				if (ch->cmbt[5] < 0 || ch->cmbt[5] > 200)
				{
					errordetect = TRUE;
					ch->cmbt[5] = 0;
				}
				ch->cmbt[6] = fread_number(fp, -999);
				if (ch->cmbt[6] < 0 || ch->cmbt[6] > 200)
				{
					errordetect = TRUE;
					ch->cmbt[6] = 0;
				}
				ch->cmbt[7] = fread_number(fp, -999);
				if (ch->cmbt[7] < 0 || ch->cmbt[7] > 200)
				{
					errordetect = TRUE;
					ch->cmbt[7] = 0;
				}
				if (errordetect)
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Combat \n\r");
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Condition"))
			{
				ch->pcdata->condition[0] = fread_number(fp, -999);
				if (ch->pcdata->condition[0] > 100 || ch->pcdata->condition[0] < 0)
				{
					errordetect = TRUE;
					ch->pcdata->condition[0] = 48;
				}
				ch->pcdata->condition[1] = fread_number(fp, -999);
				if (ch->pcdata->condition[1] > 100 || ch->pcdata->condition[1] < 0)
				{
					errordetect = TRUE;
					ch->pcdata->condition[1] = 48;
				}
				ch->pcdata->condition[2] = fread_number(fp, -999);
				if (ch->pcdata->condition[2] > 100 || ch->pcdata->condition[2] < 0)
				{
					errordetect = TRUE;
					ch->pcdata->condition[2] = 48;
				}
				if (errordetect)
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Condition \n\r");
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Createtime"))
			{
				char *str_old;
				int len;

				ch->createtime = fread_string(fp);

				/* strip off all carriage returns from the creation time */
				len = strlen(ch->createtime);
				str_old = ch->createtime;
				strtok(str_old, "\n\r");
				ch->createtime = str_dup(str_old);
				free_mem(str_old, len + 1);

				/* needed to use free_mem() directly because strtok() overwrites 
	          the original string with nulls, making strlen() and therefore 
	          also free_string() screw up.
	       */

				fMatch = TRUE;
				break;
			}
			break;

		case 'D':
			if (!str_cmp(word, "Damroll"))
			{
				ch->damroll = fread_number(fp, -999);
				if (ch->damroll == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Damroll \n\r");
					ch->damroll = 0;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Deaf"))
			{
				ch->deaf = fread_number(fp, -999);

				if (ch->deaf == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Deaf \n\r");
					ch->deaf = 0;
				}
				if (ch->deaf < 0)
					ch->deaf = 0;
				fMatch = TRUE;
				break;
			}
			KEYS("Description", ch->description, fread_string(fp));

			if (!str_cmp(word, "Dodge"))
			{
				ch->dodge = fread_number(fp, -999);
				if (ch->dodge == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Dodge \n\r");
					ch->dodge = 0;
				}
				fMatch = TRUE;
				break;
			}
			break;

		case 'E':
			if (!str_cmp(word, "End"))
				return;
			if (!str_cmp(word, "Exp"))
			{
				ch->exp = fread_number(fp, -999);
				if (ch->exp == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Exp \n\r");
					ch->exp = 0;
				}
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Extra"))
			{
				ch->extra = fread_number(fp, -999);
				if (ch->extra == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Extra \n\r");
					ch->extra = 0;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Email"))
			{
				ch->pcdata->email = fread_string(fp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'F':
			if (!str_cmp(word, "Form"))
			{
				ch->form = fread_number(fp, -999);
				if (ch->form == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Form \n\r");
					ch->form = 32767;
				}
				fMatch = TRUE;
				break;
			}
			break;

		case 'G':
			if (!str_cmp(word, "Gold"))
			{
				ch->gold = fread_number(fp, -999);
				if (ch->gold == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Gold \n\r");
					ch->gold = 0;
				}
				fMatch = TRUE;
				break;
			}
			break;

		case 'H':
			if (!str_cmp(word, "Hitroll"))
			{
				ch->hitroll = fread_number(fp, -999);
				if (ch->hitroll == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Hitroll \n\r");
					ch->hitroll = 0;
				}
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Home"))
			{
				ch->home = fread_number(fp, -999);
				if (ch->home == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Home \n\r");
					ch->home = 3001;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "HpManaMove"))
			{
				ch->hit = fread_number(fp, -999);
				if (ch->hit == -999)
				{
					errordetect = TRUE;
					ch->hit = 20;
				}
				ch->max_hit = fread_number(fp, -999);
				if (ch->max_hit == -999)
				{
					errordetect = TRUE;
					ch->max_hit = 20;
				}
				ch->mana = fread_number(fp, -999);
				if (ch->mana == -999)
				{
					errordetect = TRUE;
					ch->mana = 100;
				}
				ch->max_mana = fread_number(fp, -999);
				if (ch->max_mana == -999)
				{
					errordetect = TRUE;
					ch->max_mana = 100;
				}
				ch->move = fread_number(fp, -999);
				if (ch->move == -999)
				{
					errordetect = TRUE;
					ch->move = 100;
				}
				ch->max_move = fread_number(fp, -999);
				if (ch->max_move == -999)
				{
					errordetect = TRUE;
					ch->max_move = 100;
				}
				if (errordetect)
					snprintf(errormess, MAX_STRING_LENGTH, "Error in HpManaMove \n\r");
				fMatch = TRUE;
				break;
			}
			break;

		case 'I':
			if (!str_cmp(word, "Immune"))
			{
				ch->immune = fread_number(fp, -999);
				if (ch->immune == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Immune \n\r");
					ch->immune = 0;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Itemaffect"))
			{
				ch->itemaffect = fread_number(fp, -999);
				if (ch->itemaffect == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Itemaffect \n\r");
					ch->itemaffect = 3001;
				}
				fMatch = TRUE;
				break;
			}
			break;

		case 'L':
			KEYS("Lasthost", ch->lasthost, fread_string(fp));
			KEYS("Lasttime", ch->lasttime, fread_string(fp));
			if (!str_cmp(word, "Level"))
			{
				ch->level = fread_number(fp, -999);
				if (ch->level == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Level \n\r");
					ch->level = 1;
				}
				/*		   if ( ch->level == 9 &&
			  ( (str_cmp( ch->name, "Palmer" )) &&
			  (str_cmp( ch->name, "Archon")) &&
			  (str_cmp( ch->name, "Palmer")) &&
			  (str_cmp( ch->name, "Dirge"))))
		   {
			  ch->level = 1;
		   }*/
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Locationhp"))
			{
				ch->loc_hp[0] = fread_number(fp, -999);
				if (ch->loc_hp[0] == -999)
				{
					errordetect = TRUE;
					ch->loc_hp[0] = 0;
				}
				ch->loc_hp[1] = fread_number(fp, -999);
				if (ch->loc_hp[1] == -999)
				{
					errordetect = TRUE;
					ch->loc_hp[1] = 0;
				}
				ch->loc_hp[2] = fread_number(fp, -999);
				if (ch->loc_hp[2] == -999)
				{
					errordetect = TRUE;
					ch->loc_hp[2] = 0;
				}
				ch->loc_hp[3] = fread_number(fp, -999);
				if (ch->loc_hp[3] == -999)
				{
					errordetect = TRUE;
					ch->loc_hp[3] = 0;
				}
				ch->loc_hp[4] = fread_number(fp, -999);
				if (ch->loc_hp[4] == -999)
				{
					errordetect = TRUE;
					ch->loc_hp[4] = 0;
				}
				ch->loc_hp[5] = fread_number(fp, -999);
				if (ch->loc_hp[5] == -999)
				{
					errordetect = TRUE;
					ch->loc_hp[5] = 0;
				}
				ch->loc_hp[6] = fread_number(fp, -999);
				if (ch->loc_hp[6] == -999)
				{
					errordetect = TRUE;
					ch->loc_hp[6] = 0;
				}
				if (errordetect)
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Locationhp \n\r");
				fMatch = TRUE;
				break;
			}
			KEYS("LongDescr", ch->long_descr, fread_string(fp));
			KEYS("Lord", ch->lord, fread_string(fp));
			KEYS("LagPenalty", ch->lagpenalty, fread_number(fp, -999));
			if (ch->lagpenalty == -999)
			{
				errordetect = TRUE;
				ch->lagpenalty = 0;
			}
			break;

		case 'M':
			KEYS("Morph", ch->morph, fread_string(fp));
			break;

		case 'N':
			if (!str_cmp(word, "Name"))
			{
				/*
	    * Name already set externally.
		   */
				fread_to_eol(fp);
				fMatch = TRUE;
				break;
			}

			break;

		case 'O':
			if (!str_cmp(word, "Objvnum"))
			{
				ch->pcdata->obj_vnum = fread_number(fp, -999);
				if (ch->pcdata->obj_vnum == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Objvnum \n\r");
					ch->pcdata->obj_vnum = 24000;
				}
				fMatch = TRUE;
				break;
			}
			break;

		case 'P':
			if (!str_cmp(word, "Parry"))
			{
				ch->parry = fread_number(fp, -999);
				if (ch->parry == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Parry \n\r");
					ch->parry = 0;
				}
				fMatch = TRUE;
				break;
			}
			if(!str_cmp(word, "PKEnabled"))
			{
			    ch->pk_enabled = fread_number(fp, -999);
			    if(ch->pk_enabled == -999)
			    {
			        errordetect = TRUE;
			        snprintf(errormess, MAX_STRING_LENGTH, "Error in PKEnabled\n\r");
			        ch->pk_enabled = 0;
			    }

			    fMatch = TRUE;
			    break;
			}
			KEYS("Password", ch->pcdata->pwd, fread_string(fp));
			if (!str_cmp(word, "Played"))
			{
				ch->played = fread_number(fp, -999);
				if (ch->played == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Played \n\r");
					ch->played = 0;
				}
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Polyaff"))
			{
				ch->polyaff = fread_number(fp, -999);
				if (ch->polyaff == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Polyaff \n\r");
					ch->polyaff = 0;
				}
				fMatch = TRUE;
				break;
			}
			KEYS("Poweraction", ch->poweraction, fread_string(fp));
			KEYS("Powertype", ch->powertype, fread_string(fp));
			if (!str_cmp(word, "Position"))
			{
				ch->position = fread_number(fp, -999);
				if (ch->position == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Position \n\r");
					ch->position = 7;
				}
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Primal"))
			{
				ch->primal = fread_number(fp, -999);
				if (ch->primal == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Primal\n\r");
					ch->primal = 0;
				}
				fMatch = TRUE;
				break;
			}
			KEYS("Prompt", ch->prompt, fread_string(fp));
			if (!str_cmp(word, "PkPdMkMd"))
			{
				ch->pkill = fread_number(fp, -999);
				if (ch->pkill == -999)
				{
					errordetect = TRUE;
					ch->pkill = 0;
				}
				ch->pdeath = fread_number(fp, -999);
				if (ch->pdeath == -999)
				{
					errordetect = TRUE;
					ch->pdeath = 0;
				}
				ch->mkill = fread_number(fp, -999);
				if (ch->mkill == -999)
				{
					errordetect = TRUE;
					ch->mkill = 0;
				}
				ch->mdeath = fread_number(fp, -999);
				if (ch->mdeath == -999)
				{
					errordetect = TRUE;
					ch->mdeath = 0;
				}
				if (errordetect)
					snprintf(errormess, MAX_STRING_LENGTH, "Error in PkPdMkMd \n\r");
				fMatch = TRUE;
				break;
			}
			break;

		case 'Q':
			if (!str_cmp(word, "Quest"))
			{
				ch->pcdata->quest = fread_number(fp, -999);
				if (ch->pcdata->quest == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Quest \n\r");
					ch->pcdata->quest = 0;
				}
				fMatch = TRUE;
				break;
			}
			break;

		case 'R':

		    if (!str_cmp(word, "Race"))
            {
                fMatch = TRUE;
                break;
            }
			if (!str_cmp(word, "RemortLevel"))
			{
				ch->remortlevel = fread_number(fp, -999);
				if (ch->remortlevel == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Remort Level \n\r");
					ch->remortlevel = 0;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Room"))
			{
				/*	int jkrtemp=0;
	    jkrtemp = fread_number( fp, -999);
		   if( jkrtemp == -999) {errordetect = TRUE; jkrtemp = 3001;} */
				ch->in_room = get_room_index(fread_number(fp, -999));
				if (ch->in_room == NULL)
					ch->in_room = get_room_index(ROOM_VNUM_LIMBO);
				fMatch = TRUE;
				break;
			}

			break;

		case 'S':
			if (!str_cmp(word, "SavingThrow"))
			{
				ch->saving_throw = fread_number(fp, -999);
				if (ch->saving_throw == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in SavingThrow \n\r");
					ch->saving_throw = 0;
				}
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Sex"))
			{
				ch->sex = fread_number(fp, -999);
				if (ch->sex > 2 || ch->sex < 0)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Sex \n\r");
					ch->sex = 0;
				}
				fMatch = TRUE;
				break;
			}
			KEYS("ShortDescr", ch->short_descr, fread_string(fp));

			if (!str_cmp(word, "Skill"))
			{
				int sn;
				int value;

				value = fread_number(fp, -999);
				if (value == -999)
				{
					errordetect = TRUE;
					value = 0;
				}
				sn = skill_lookup(fread_word(fp));
				/*
		   if ( sn < 0 )
		   bug( "Fread_char: unknown skill.", 0 );
		   else
		   ch->pcdata->learned[sn] = value;
		   */
				if (sn >= 0)
					ch->pcdata->learned[sn] = value;
				fMatch = TRUE;
			}

			if (!str_cmp(word, "Specpower"))
			{
				ch->specpower = fread_number(fp, -999);
				if (ch->specpower == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Specpower \n\r");
					ch->specpower = 0;
				}
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Spectype"))
			{
				ch->spectype = fread_number(fp, -999);
				if (ch->spectype == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Spectype \n\r");
					ch->spectype = 0;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Spells"))
			{
				ch->spl[SPELL_PURPLE] = fread_number(fp, -999);
				if (ch->spl[SPELL_PURPLE] == -999)
				{
					errordetect = TRUE;
					ch->spl[SPELL_PURPLE] = 0;
				}
				ch->spl[SPELL_RED] = fread_number(fp, -999);
				if (ch->spl[SPELL_RED] == -999)
				{
					errordetect = TRUE;
					ch->spl[SPELL_RED] = 0;
				}
				ch->spl[SPELL_BLUE] = fread_number(fp, -999);
				if (ch->spl[SPELL_BLUE] == -999)
				{
					errordetect = TRUE;
					ch->spl[SPELL_BLUE] = 0;
				}
				ch->spl[SPELL_GREEN] = fread_number(fp, -999);
				if (ch->spl[SPELL_GREEN] == -999)
				{
					errordetect = TRUE;
					ch->spl[SPELL_GREEN] = 0;
				}
				ch->spl[SPELL_YELLOW] = fread_number(fp, -999);
				if (ch->spl[SPELL_YELLOW] == -999)
				{
					errordetect = TRUE;
					ch->spl[SPELL_YELLOW] = 0;
				}
				if (errordetect)
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Spells \n\r");
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Stance"))
			{
				ch->stance[CURRENT_STANCE] = fread_number(fp, -999);
				if (ch->stance[CURRENT_STANCE] == -999)
					errordetect = TRUE;
				if (ch->stance[CURRENT_STANCE] < -1)
				{
					ch->stance[CURRENT_STANCE] = 200;
				}
				ch->stance[STANCE_VIPER] = fread_number(fp, -999);
				if (ch->stance[STANCE_VIPER] == -999)
					errordetect = TRUE;
				if ( ch->stance[STANCE_VIPER] < 0)
				{
					ch->stance[STANCE_VIPER] = 200;
				}
				ch->stance[STANCE_CRANE] = fread_number(fp, -999);
				if (ch->stance[STANCE_CRANE] == -999)
					errordetect = TRUE;
				if (ch->stance[STANCE_CRANE] < 0)
				{
					ch->stance[STANCE_CRANE] = 200;
				}
				ch->stance[STANCE_FALCON] = fread_number(fp, -999);
				if (ch->stance[STANCE_FALCON] == -999)
					errordetect = TRUE;
				if ( ch->stance[STANCE_FALCON] < 0)
				{
					ch->stance[STANCE_FALCON] = 200;
				}
				ch->stance[STANCE_MONGOOSE] = fread_number(fp, -999);
				if (ch->stance[STANCE_MONGOOSE] == -999)
					errordetect = TRUE;
				if (ch->stance[STANCE_MONGOOSE] < 0)
				{
					ch->stance[STANCE_MONGOOSE] = 200;
				}
				ch->stance[STANCE_BULL] = fread_number(fp, -999);
				if (ch->stance[STANCE_BULL] == -999)
					errordetect = TRUE;
				if (ch->stance[STANCE_BULL] < 0)
				{
					ch->stance[STANCE_BULL] = 200;
				}
				ch->stance[STANCE_SWALLOW] = fread_number(fp, -999);
				if (ch->stance[STANCE_SWALLOW] == -999)
					errordetect = TRUE;
				if (ch->stance[STANCE_SWALLOW] < 0)
				{
					ch->stance[STANCE_SWALLOW] = 200;
				}
				ch->stance[STANCE_COBRA] = fread_number(fp, -999);
				if (ch->stance[STANCE_COBRA] == -999)
					errordetect = TRUE;
				if (ch->stance[STANCE_COBRA] < 0)
				{
					ch->stance[STANCE_COBRA] = 200;
				}
				ch->stance[STANCE_LION] = fread_number(fp, -999);
				if (ch->stance[STANCE_LION] == -999)
					errordetect = TRUE;
				if (ch->stance[STANCE_LION] < 0)
				{
					ch->stance[STANCE_LION] = 200;
				}
				ch->stance[STANCE_GRIZZLIE] = fread_number(fp, -999);
				if (ch->stance[STANCE_GRIZZLIE] == -999)
					errordetect = TRUE;
				if (ch->stance[STANCE_GRIZZLIE] < 0)
				{
					ch->stance[STANCE_GRIZZLIE] = 200;
				}
				ch->stance[STANCE_PANTHER] = fread_number(fp, -999);
				if (ch->stance[STANCE_PANTHER] == -999)
					errordetect = TRUE;
				if (ch->stance[STANCE_PANTHER] < 0)
				{
					ch->stance[STANCE_PANTHER] = 200;
				}
				ch->stance[AUTODROP] = fread_number(fp, -999);
				if (ch->stance[AUTODROP] == -999)
					errordetect = TRUE;
				if (ch->stance[AUTODROP] > 11 || ch->stance[AUTODROP] < 0)
				{
					ch->stance[AUTODROP] = 0;
				}
				if (errordetect)
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Stance \n\r");
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Status"))
            {
                ch->status = fread_number(fp, -999);
                if (ch->status == -999)
                {
                    errordetect = TRUE;
                    snprintf(errormess, MAX_STRING_LENGTH, "Error in status \n\r");
                    ch->status = 0;
                }
                fMatch = TRUE;
                break;
            }

			break;

		case 'T':
			if (!str_cmp(word, "TierPoints"))
			{
				ch->tierpoints = fread_number(fp, -999);
				if (ch->tierpoints == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in TierPoints \n\r");
					ch->tierpoints = 0;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "TierClandiscs"))
			{
				ch->tier_clandisc[0] = fread_number(fp, -999);
				if (ch->tier_clandisc[0] == -999)
				{
					errordetect = TRUE;
					ch->tier_clandisc[0] = 0;
				}
				ch->tier_clandisc[1] = fread_number(fp, -999);
				if (ch->tier_clandisc[1] == -999)
				{
					errordetect = TRUE;
					ch->tier_clandisc[1] = 0;
				}
				ch->tier_clandisc[2] = fread_number(fp, -999);
				if (ch->tier_clandisc[2] == -999)
				{
					errordetect = TRUE;
					ch->tier_clandisc[2] = 0;
				}
				ch->tier_clandisc[3] = fread_number(fp, -999);
				if (ch->tier_clandisc[3] == -999)
				{
					errordetect = TRUE;
					ch->tier_clandisc[3] = 0;
				}
				ch->tier_clandisc[4] = fread_number(fp, -999);
				if (ch->tier_clandisc[4] == -999)
				{
					errordetect = TRUE;
					ch->tier_clandisc[4] = 0;
				}
				ch->tier_clandisc[5] = fread_number(fp, -999);
				if (ch->tier_clandisc[5] == -999)
				{
					errordetect = TRUE;
					ch->tier_clandisc[5] = 0;
				}
				ch->tier_clandisc[6] = fread_number(fp, -999);
				if (ch->tier_clandisc[6] == -999)
				{
					errordetect = TRUE;
					ch->tier_clandisc[6] = 0;
				}
				ch->tier_clandisc[7] = fread_number(fp, -999);
				if (ch->tier_clandisc[7] == -999)
				{
					errordetect = TRUE;
					ch->tier_clandisc[7] = 0;
				}
				ch->tier_clandisc[8] = fread_number(fp, -999);
				if (ch->tier_clandisc[8] == -999)
				{
					errordetect = TRUE;
					ch->tier_clandisc[8] = 0;
				}
				ch->tier_clandisc[9] = fread_number(fp, -999);
				if (ch->tier_clandisc[9] == -999)
				{
					errordetect = TRUE;
					ch->tier_clandisc[9] = 0;
				}
				ch->tier_clandisc[10] = fread_number(fp, -999);
				if (ch->tier_clandisc[10] == -999)
				{
					errordetect = TRUE;
					ch->tier_clandisc[10] = 0;
				}
				ch->tier_clandisc[11] = fread_number(fp, -999);
				if (ch->tier_clandisc[11] == -999)
				{
					ch->tier_clandisc[11] = 0;
				}

				if (errordetect)
					snprintf(errormess, MAX_STRING_LENGTH, "Error in TierClandiscs \n\r");
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "TierSpells"))
			{
				ch->tier_spl[SPELL_PURPLE] = fread_number(fp, -999);
				if (ch->tier_spl[SPELL_PURPLE] == -999)
				{
					errordetect = TRUE;
					ch->tier_spl[SPELL_PURPLE] = 0;
				}
				ch->tier_spl[SPELL_RED] = fread_number(fp, -999);
				if (ch->tier_spl[SPELL_RED] == -999)
				{
					errordetect = TRUE;
					ch->tier_spl[SPELL_RED] = 0;
				}
				ch->tier_spl[SPELL_BLUE] = fread_number(fp, -999);
				if (ch->tier_spl[SPELL_BLUE] == -999)
				{
					errordetect = TRUE;
					ch->tier_spl[SPELL_BLUE] = 0;
				}
				ch->tier_spl[SPELL_GREEN] = fread_number(fp, -999);
				if (ch->tier_spl[SPELL_GREEN] == -999)
				{
					errordetect = TRUE;
					ch->tier_spl[SPELL_GREEN] = 0;
				}
				ch->tier_spl[SPELL_YELLOW] = fread_number(fp, -999);
				if (ch->tier_spl[SPELL_YELLOW] == -999)
				{
					errordetect = TRUE;
					ch->tier_spl[SPELL_YELLOW] = 0;
				}

				if (errordetect)
					snprintf(errormess, MAX_STRING_LENGTH, "Error in TierSpells \n\r");
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "TierStances"))
			{
				ch->tier_stance[CURRENT_STANCE] = fread_number(fp, -999);
				if (ch->tier_stance[CURRENT_STANCE] == -999)
				{
					errordetect = TRUE;
					ch->tier_stance[CURRENT_STANCE] = 0;
				}
				ch->tier_stance[STANCE_VIPER] = fread_number(fp, -999);
				if (ch->tier_stance[STANCE_VIPER] == -999)
				{
					errordetect = TRUE;
					ch->tier_stance[STANCE_VIPER] = 0;
				}
				ch->tier_stance[STANCE_CRANE] = fread_number(fp, -999);
				if (ch->tier_stance[STANCE_CRANE] == -999)
				{
					errordetect = TRUE;
					ch->tier_stance[STANCE_CRANE] = 0;
				}
				ch->tier_stance[STANCE_FALCON] = fread_number(fp, -999);
				if (ch->tier_stance[STANCE_FALCON] == -999)
				{
					errordetect = TRUE;
					ch->tier_stance[STANCE_FALCON] = 0;
				}
				ch->tier_stance[STANCE_MONGOOSE] = fread_number(fp, -999);
				if (ch->tier_stance[STANCE_MONGOOSE] == -999)
				{
					errordetect = TRUE;
					ch->tier_stance[STANCE_MONGOOSE] = 0;
				}
				ch->tier_stance[STANCE_BULL] = fread_number(fp, -999);
				if (ch->tier_stance[STANCE_BULL] == -999)
				{
					errordetect = TRUE;
					ch->tier_stance[STANCE_BULL] = 0;
				}
				ch->tier_stance[STANCE_SWALLOW] = fread_number(fp, -999);
				if (ch->tier_stance[STANCE_SWALLOW] == -999)
				{
					errordetect = TRUE;
					ch->tier_stance[STANCE_SWALLOW] = 0;
				}
				ch->tier_stance[STANCE_COBRA] = fread_number(fp, -999);
				if (ch->tier_stance[STANCE_COBRA] == -999)
				{
					errordetect = TRUE;
					ch->tier_stance[STANCE_COBRA] = 0;
				}
				ch->tier_stance[STANCE_LION] = fread_number(fp, -999);
				if (ch->tier_stance[STANCE_LION] == -999)
				{
					errordetect = TRUE;
					ch->tier_stance[STANCE_LION] = 0;
				}
				ch->tier_stance[STANCE_GRIZZLIE] = fread_number(fp, -999);
				if (ch->tier_stance[STANCE_GRIZZLIE] == -999)
				{
					errordetect = TRUE;
					ch->tier_stance[STANCE_GRIZZLIE] = 0;
				}
				ch->tier_stance[STANCE_PANTHER] = fread_number(fp, -999);
				if (ch->tier_stance[STANCE_PANTHER] == -999)
				{
					errordetect = TRUE;
					ch->tier_stance[STANCE_PANTHER] = 0;
				}
				ch->tier_stance[AUTODROP] = fread_number(fp, -999);
				if (ch->tier_stance[AUTODROP] == -999)
				{
					errordetect = TRUE;
					ch->tier_stance[AUTODROP] = 0;
				}

				if (errordetect)
					snprintf(errormess, MAX_STRING_LENGTH, "Error in TierStances \n\r");
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "TierWeapons"))
			{
				ch->tier_wpn[WEAPON_HIT] = fread_number(fp, -999);
				if (ch->tier_wpn[WEAPON_HIT] == -999)
				{
					errordetect = TRUE;
					ch->tier_wpn[WEAPON_HIT] = 0;
				}
				ch->tier_wpn[WEAPON_SLICE] = fread_number(fp, -999);
				if (ch->tier_wpn[WEAPON_SLICE] == -999)
				{
					errordetect = TRUE;
					ch->tier_wpn[WEAPON_SLICE] = 0;
				}
				ch->tier_wpn[WEAPON_STAB] = fread_number(fp, -999);
				if (ch->tier_wpn[WEAPON_STAB] == -999)
				{
					errordetect = TRUE;
					ch->tier_wpn[WEAPON_STAB] = 0;
				}
				ch->tier_wpn[WEAPON_SLASH] = fread_number(fp, -999);
				if (ch->tier_wpn[WEAPON_SLASH] == -999)
				{
					errordetect = TRUE;
					ch->tier_wpn[WEAPON_SLASH] = 0;
				}
				ch->tier_wpn[WEAPON_WHIP] = fread_number(fp, -999);
				if (ch->tier_wpn[WEAPON_WHIP] == -999)
				{
					errordetect = TRUE;
					ch->tier_wpn[WEAPON_WHIP] = 0;
				}
				ch->tier_wpn[WEAPON_CLAW] = fread_number(fp, -999);
				if (ch->tier_wpn[WEAPON_CLAW] == -999)
				{
					errordetect = TRUE;
					ch->tier_wpn[WEAPON_CLAW] = 0;
				}
				ch->tier_wpn[WEAPON_BLAST] = fread_number(fp, -999);
				if (ch->tier_wpn[WEAPON_BLAST] == -999)
				{
					errordetect = TRUE;
					ch->tier_wpn[WEAPON_BLAST] = 0;
				}
				ch->tier_wpn[WEAPON_POUND] = fread_number(fp, -999);
				if (ch->tier_wpn[WEAPON_POUND] == -999)
				{
					errordetect = TRUE;
					ch->tier_wpn[WEAPON_POUND] = 0;
				}
				ch->tier_wpn[WEAPON_CRUSH] = fread_number(fp, -999);
				if (ch->tier_wpn[WEAPON_CRUSH] == -999)
				{
					errordetect = TRUE;
					ch->tier_wpn[WEAPON_CRUSH] = 0;
				}
				ch->tier_wpn[WEAPON_BITE] = fread_number(fp, -999);
				if (ch->tier_wpn[WEAPON_BITE] == -999)
				{
					errordetect = TRUE;
					ch->tier_wpn[WEAPON_BITE] = 0;
				}
				ch->tier_wpn[WEAPON_GREP] = fread_number(fp, -999);
				if (ch->tier_wpn[WEAPON_GREP] == -999)
				{
					errordetect = TRUE;
					ch->tier_wpn[WEAPON_GREP] = 0;
				}
				ch->tier_wpn[WEAPON_PIERCE] = fread_number(fp, -999);
				if (ch->tier_wpn[WEAPON_PIERCE] == -999)
				{
					errordetect = TRUE;
					ch->tier_wpn[WEAPON_PIERCE] = 0;
				}
				ch->tier_wpn[WEAPON_SUCK] = fread_number(fp, -999);
				if (ch->tier_wpn[WEAPON_SUCK] == -999)
				{
					errordetect = TRUE;
					ch->tier_wpn[WEAPON_SUCK] = 0;
				}

				if (errordetect)
					snprintf(errormess, MAX_STRING_LENGTH, "Error in TierWeapons \n\r");
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Trust"))
			{
				ch->trust = fread_number(fp, -999);
				/*		   if( ch->trust == 9 &&
			  ( str_cmp( ch->name, "Palmer") &&
			  str_cmp( ch->name, "Archon") &&
			  str_cmp( ch->name, "Palmer") &&
			  str_cmp( ch->name, "Dirge") ) )
		   { 
			  ch->trust = 0;
		   }*/
				if (ch->trust == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Trust \n\r");
					ch->trust = 0;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Title"))
			{
				ch->pcdata->title = fread_string(fp);
				if (isalpha(ch->pcdata->title[0]) || isdigit(ch->pcdata->title[0]))
				{
					snprintf(buf, MAX_STRING_LENGTH, " %s", ch->pcdata->title);
					free_string(ch->pcdata->title);
					ch->pcdata->title = str_dup(buf);
				}
				fMatch = TRUE;
				break;
			}

			break;

		case 'V':
			if (!str_cmp(word, "Vampaff"))
			{
				ch->vampaff = fread_number(fp, -999);
				if (ch->vampaff == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Vampaff \n\r");
					ch->vampaff = 0;
				}
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Vampgen"))
			{
				ch->vampgen = fread_number(fp, -999);
				if (ch->vampgen == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Vampgen \n\r");
					ch->vampgen = 0;
				}
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Vamppass"))
			{
				ch->vamppass = fread_number(fp, -999);
				if (ch->vamppass == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Vamppass \n\r");
					ch->vamppass = 0;
				}
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Vnum"))
			{
				int jkrtemp = 0;
				jkrtemp = fread_number(fp, -999);
				if (jkrtemp == -999)
				{
					errordetect = TRUE;
					jkrtemp = 24000;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Vnum \n\r");
				}
				ch->pIndexData = get_mob_index(jkrtemp);
				fMatch = TRUE;
				break;
			}
			break;

		case 'W':
			if (!str_cmp(word, "Weapons"))
			{
				ch->wpn[WEAPON_HIT] = fread_number(fp, -999);
				if (ch->wpn[WEAPON_HIT] < 0)
				{
					errordetect = TRUE;
					ch->wpn[WEAPON_HIT] = 0;
				}
				ch->wpn[WEAPON_SLICE] = fread_number(fp, -999);
				if (ch->wpn[WEAPON_SLICE] < 0)
				{
					errordetect = TRUE;
					ch->wpn[WEAPON_SLICE] = 0;
				}
				ch->wpn[WEAPON_STAB] = fread_number(fp, -999);
				if (ch->wpn[WEAPON_STAB] < 0)
				{
					errordetect = TRUE;
					ch->wpn[WEAPON_STAB] = 0;
				}
				ch->wpn[WEAPON_SLASH] = fread_number(fp, -999);
				if (ch->wpn[WEAPON_SLASH] < 0)
				{
					errordetect = TRUE;
					ch->wpn[WEAPON_SLASH] = 0;
				}
				ch->wpn[WEAPON_WHIP] = fread_number(fp, -999);
				if (ch->wpn[WEAPON_WHIP] < 0)
				{
					errordetect = TRUE;
					ch->wpn[WEAPON_WHIP] = 0;
				}
				ch->wpn[WEAPON_CLAW] = fread_number(fp, -999);
				if (ch->wpn[WEAPON_CLAW] < 0)
				{
					errordetect = TRUE;
					ch->wpn[WEAPON_CLAW] = 0;
				}
				ch->wpn[WEAPON_BLAST] = fread_number(fp, -999);
				if (ch->wpn[WEAPON_BLAST] < 0)
				{
					errordetect = TRUE;
					ch->wpn[WEAPON_BLAST] = 0;
				}
				ch->wpn[WEAPON_POUND] = fread_number(fp, -999);
				if (ch->wpn[WEAPON_POUND] < 0)
				{
					errordetect = TRUE;
					ch->wpn[WEAPON_POUND] = 0;
				}
				ch->wpn[WEAPON_CRUSH] = fread_number(fp, -999);
				if (ch->wpn[WEAPON_CRUSH] < 0)
				{
					errordetect = TRUE;
					ch->wpn[WEAPON_CRUSH] = 0;
				}
				ch->wpn[WEAPON_BITE] = fread_number(fp, -999);
				if (ch->wpn[WEAPON_BITE] < 0)
				{
					errordetect = TRUE;
					ch->wpn[WEAPON_BITE] = 0;
				}
				ch->wpn[WEAPON_GREP] = fread_number(fp, -999);
				if (ch->wpn[WEAPON_GREP] < 0)
				{
					errordetect = TRUE;
					ch->wpn[WEAPON_GREP] = 0;
				}
				ch->wpn[WEAPON_PIERCE] = fread_number(fp, -999);
				if (ch->wpn[WEAPON_PIERCE] < 0)
				{
					errordetect = TRUE;
					ch->wpn[WEAPON_PIERCE] = 0;
				}
				ch->wpn[WEAPON_SUCK] = fread_number(fp, -999);
				if (ch->wpn[WEAPON_SUCK] < 0)
				{
					errordetect = TRUE;
					ch->wpn[WEAPON_SUCK] = 0;
				}
				if (errordetect)
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Weapon\n\r");
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Wimpy"))
			{
				ch->wimpy = fread_number(fp, -999);
				if (ch->wimpy == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Wimpy \n\r");
					ch->wimpy = 0;
				}
				fMatch = TRUE;
				break;
			}
			break;
		}

		if (errordetect == TRUE)
		{

			char palmer[MAX_INPUT_LENGTH];
			send_to_char("ERROR DETECTED! \n\r Your pfile is buggered please contact a CODER and do NOT log this char in again untill told to do so.\n\r", ch);
			bug("ERROR DETECTED! Shagged pfile!! during fread_char load.", 0);
			snprintf(palmer, MAX_INPUT_LENGTH, "%s has a shagged pfile(ERROR DETECTED!), please inform a CODER!\n\r", ch->name);
			do_info(ch, palmer);
			bug(palmer, 0);
			bug(errormess, 0);
			do_info(ch, errormess);
			close_socket(ch->desc);
			errordetect = FALSE;
			/*    	    strcpy( errormess, "");	*/
			return;
		}

		if (!fMatch)
		{
			snprintf(errormess, MAX_STRING_LENGTH, "Fread_char: no match (!fMatch) %s field error", word);
			bug(errormess, 0);

			snprintf(errormess, MAX_STRING_LENGTH, "Error detected trying to read the '%s' field.", word);
			bug(errormess, 0);

			if (!strcmp(word, "SHAGGED"))
			{
				char palmer[MAX_INPUT_LENGTH];
				send_to_char("Your pfile is buggered please contact a CODER and do NOT use this char again until told to do so. \n\r", ch);
				bug("Shagged pfile!! during fread_char load.", 0);
				snprintf(palmer, MAX_INPUT_LENGTH, "%s has a shagged pfile, please inform a CODER!\n\r", ch->name);
				do_info(ch, palmer);
				bug(palmer, 0);
				bug(errormess, 0);
				close_socket(ch->desc);
				return;
			}
			else
				fread_to_eol(fp);
		}
	}
}

void fread_clandisc(CHAR_DATA *ch, FILE *fp)
{
	CLANDISC_DATA *disc;
	CLANDISC_DATA *discLookup;
	static CLANDISC_DATA disc_zero;
	char *word;
	bool fMatch;
	bool errordetect = FALSE;
	char errormess[MAX_STRING_LENGTH];

	if(clandisc_free == NULL)
	{
		disc = alloc_perm(sizeof(*disc));
	}
	else
	{
		disc = clandisc_free;
		clandisc_free = clandisc_free->next;
	}

	*disc = disc_zero;
	disc->name = str_dup("");
	disc->clandisc = str_dup("");
	disc->tier = 0;
	disc->personal_message_on = str_dup("");
	disc->personal_message_off = str_dup("");
	disc->room_message_on = str_dup("");
	disc->room_message_off = str_dup("");
	disc->victim_message = str_dup("");
	disc->option = str_dup("");
	disc->upkeepMessage = str_dup("");
	disc->timeLeft = 0;
	disc->cooldown = 0;
	disc->bloodcost = 0;
	disc->isActive = FALSE;
	disc->isPassiveAbility = FALSE;

	for (;;)
	{
		word = feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;

		switch (UPPER(word[0]))
		{
		case '*':
			fMatch = TRUE;
			fread_to_eol(fp);
			break;

		case 'C':
			KEYS("Clandisc", disc->clandisc, fread_string(fp));
			break;
		case 'E':
			if (!str_cmp(word, "End"))
			{
				discLookup = get_disc_by_name(disc->name);

				discLookup->isActive = disc->isActive;
				discLookup->option = disc->option;
				discLookup->timeLeft = disc->timeLeft;
				discLookup->personal_message_on = disc->personal_message_on;
				discLookup->personal_message_off = disc->personal_message_off;
				discLookup->room_message_on = disc->room_message_on;
				discLookup->room_message_off = disc->room_message_off;
				discLookup->upkeepMessage = disc->upkeepMessage;
				discLookup->victim_message = disc->victim_message;
				discLookup->cooldown = disc->cooldown;

				SetPlayerDisc(ch, discLookup);
				return;
			}
			break;

		case 'I':
			if (!str_cmp(word, "IsActive"))
			{
				if(fread_number(fp, -999) == TRUE)
				{
					disc->isActive = TRUE;
				}
				else
				{
					disc->isActive = FALSE;
				}
				
				fMatch = TRUE;
				break;
			}
			break;

		case 'N':
			KEYS("Name", disc->name, fread_string(fp));
			break;

		case 'O':
			KEYS("Option", disc->option, fread_string(fp));
			break;

		case 'P':
			KEYS("PersonalMessageOn", disc->personal_message_on, fread_string(fp));
			KEYS("PersonalMessageOff", disc->personal_message_off, fread_string(fp));
            if(!str_cmp(word, "Practice"))
            {
                fMatch = TRUE;
                break;
            }
			break;

		case 'R':
			KEYS("RoomMessageOn", disc->room_message_on, fread_string(fp));
			KEYS("RoomMessageOff", disc->room_message_off, fread_string(fp));
			break;

		case 'T':
			if (!str_cmp(word, "Tier"))
			{
				disc->tier = fread_number(fp, -999);
				if(disc->tier == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Tier \n\r");
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Timeleft"))
			{
				disc->timeLeft = fread_number(fp, -999);
				if(disc->timeLeft == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in TimeLeft \n\r");
				}
				fMatch = TRUE;
				break;
			}

			break;

		case 'U':
			KEYS("UpkeepMessage", disc->upkeepMessage, fread_string(fp));
			break;

		case 'V':
			KEYS("VictimMessage", disc->victim_message, fread_string(fp));
			break;

		}

		if (errordetect == TRUE)
		{
			char palmer[MAX_INPUT_LENGTH];
			send_to_char("ERROR DETECTED! Your pfile is buggered please contact a CODER and do NOT use this char again until told to do so.\n\r", ch);
			bug("ERROR DETECTED! Shagged pfile!! during fread_clandisc load.", 0);
			bug(errormess, 0);
			snprintf(palmer, MAX_INPUT_LENGTH, "%s has a shagged pfile(ERROR DETECTED!), please inform a CODER!\n\r", ch->name);
			do_info(ch, palmer);
			do_info(ch, errormess);
			bug(palmer, 0);
			bug(errormess, 0);
			close_socket(ch->desc);
			errordetect = FALSE;
			return;
		}

		if (!fMatch)
		{

			char palmer[MAX_INPUT_LENGTH];

			bug("Fread_clandisc: no match.error 2 (valis)", 0);
			bug(palmer, 0);
			if (!strcmp(word, "SHAGGED"))
			{
				char palmer[MAX_INPUT_LENGTH];
				send_to_char("Your pfile is buggered please contact a CODER and do NOT use this char again until told to do so\n\r", ch);
				bug("Shagged pfile!! during fread_clandisc load.", 0);
				snprintf(palmer, MAX_INPUT_LENGTH, "%s has a shagged pfile, please inform a CODER!\n\r", ch->name);
				do_info(ch, palmer);
				bug(palmer, 0);
				bug(errormess, 0);
				close_socket(ch->desc);
				return;
			}
			else
				fread_to_eol(fp);
		}
	}
}

void fread_obj(CHAR_DATA *ch, FILE *fp)
{
	static OBJ_DATA obj_zero;
	OBJ_DATA *obj;
	char *word;
	int iNest;
	bool fMatch;
	bool fNest;
	bool fVnum;
	bool errordetect = FALSE;
	char errormess[MAX_STRING_LENGTH];

	if (obj_free == NULL)
	{
		obj = alloc_perm(sizeof(*obj));
	}
	else
	{
		obj = obj_free;
		obj_free = obj_free->next;
	}

	*obj = obj_zero;
	obj->name = str_dup("");
	obj->short_descr = str_dup("");
	obj->description = str_dup("");
	obj->chpoweron = str_dup("(null)");
	obj->chpoweroff = str_dup("(null)");
	obj->chpoweruse = str_dup("(null)");
	obj->victpoweron = str_dup("(null)");
	obj->victpoweroff = str_dup("(null)");
	obj->victpoweruse = str_dup("(null)");
	obj->questmaker = str_dup("");
	obj->questowner = str_dup("");
	obj->spectype = 0;
	obj->specpower = 0;
	obj->condition = 100;
	obj->toughness = 0;
	obj->resistance = 100;
	obj->quest = 0;
	obj->points = 0;

	fNest = FALSE;
	fVnum = TRUE;
	iNest = 0;

	for (;;)
	{
		word = feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;

		switch (UPPER(word[0]))
		{
		case '*':
			fMatch = TRUE;
			fread_to_eol(fp);
			break;

		case 'A':
			if (!str_cmp(word, "Affect") || !str_cmp(word, "AffectData"))
			{
				AFFECT_DATA *paf;

				if (affect_free == NULL)
				{
					paf = alloc_perm(sizeof(*paf));
				}
				else
				{
					paf = affect_free;
					affect_free = affect_free->next;
				}

				paf->duration = fread_number(fp, -999);
				paf->modifier = fread_number(fp, -999);
				paf->location = fread_number(fp, -999);
				paf->next = obj->affected;
				obj->affected = paf;
				fMatch = TRUE;
				break;
			}
			break;

		case 'C':
			if (!str_cmp(word, "Condition"))
			{
				obj->condition = fread_number(fp, -999);
				if (obj->condition == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Condition \n\r");
					obj->condition = 100;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Cost"))
			{
				obj->cost = fread_number(fp, -999);
				if (obj->cost == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Cost \n\r");
					obj->cost = 0;
				}
				fMatch = TRUE;
				break;
			}
			break;

		case 'D':
			KEYS("Description", obj->description, fread_string(fp));
			break;

		case 'E':
			if (!str_cmp(word, "ExtraFlags"))
			{
				obj->extra_flags = fread_number(fp, -999);
				if (obj->extra_flags == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in ExtraFlags \n\r");
					obj->extra_flags = 0;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "ExtraDescr"))
			{
				EXTRA_DESCR_DATA *ed;

				if (extra_descr_free == NULL)
				{
					ed = alloc_perm(sizeof(*ed));
				}
				else
				{
					ed = extra_descr_free;
					extra_descr_free = extra_descr_free->next;
				}

				ed->keyword = fread_string(fp);
				ed->description = fread_string(fp);
				ed->next = obj->extra_descr;
				obj->extra_descr = ed;
				fMatch = TRUE;
			}

			if (!str_cmp(word, "End"))
			{
				if (!fNest || !fVnum)
				{
					bug("Fread_obj: incomplete object.", 0);
					free_string(obj->name);
					free_string(obj->description);
					free_string(obj->short_descr);
					obj->next = obj_free;
					obj_free = obj;
					return;
				}
				else
				{
					obj->next = object_list;
					object_list = obj;
					obj->pIndexData->count++;
					if (iNest == 0 || rgObjNest[iNest] == NULL)
						obj_to_char(obj, ch);
					else
						obj_to_obj(obj, rgObjNest[iNest - 1]);
					return;
				}
			}
			break;

		case 'I':
			if (!str_cmp(word, "ImbueData"))
			{
				IMBUE_DATA *id;

				if (imbue_free == NULL)
				{
					id = alloc_perm(sizeof(*id));
				}
				else
				{
					id = imbue_free;
					imbue_free = imbue_free->next;
				}

				id->name = fread_string(fp);
				id->item_type = fread_string(fp);
				id->affect_number = fread_number(fp, -999);

				id->next = obj->imbue;
				obj->imbue = id;
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "ItemType"))
			{
				obj->item_type = fread_number(fp, -999);
				if (obj->item_type == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in ItemType \n\r");
					obj->item_type = 0;
				}
				fMatch = TRUE;
				break;
			}
			break;

		case 'L':
			if (!str_cmp(word, "Level"))
			{
				obj->level = fread_number(fp, -999);
				if (obj->level == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Level \n\r");
					obj->level = 0;
				}
				fMatch = TRUE;
				break;
			}
			break;

		case 'N':
			KEYS("Name", obj->name, fread_string(fp));

			if (!str_cmp(word, "Nest"))
			{
				iNest = fread_number(fp, -999);
				if (iNest < 0 || iNest >= MAX_NEST)
				{
					bug("Fread_obj: bad nest %d.", iNest);
				}
				else
				{
					rgObjNest[iNest] = obj;
					fNest = TRUE;
				}
				fMatch = TRUE;
			}
			break;

		case 'P':
			if (!str_cmp(word, "Points"))
			{
				obj->points = fread_number(fp, -999);
				if (obj->points == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Points \n\r");
					obj->points = 0;
				}
				fMatch = TRUE;
				break;
			}
			KEYS("Poweronch", obj->chpoweron, fread_string(fp));
			KEYS("Poweroffch", obj->chpoweroff, fread_string(fp));
			KEYS("Powerusech", obj->chpoweruse, fread_string(fp));
			KEYS("Poweronvict", obj->victpoweron, fread_string(fp));
			KEYS("Poweroffvict", obj->victpoweroff, fread_string(fp));
			KEYS("Powerusevict", obj->victpoweruse, fread_string(fp));
			break;

		case 'Q':
			if (!str_cmp(word, "Quest"))
			{
				obj->quest = fread_number(fp, -999);
				if (obj->quest == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Quest \n\r");
					obj->quest = 0;
				}
				fMatch = TRUE;
				break;
			}
			KEYS("Questmaker", obj->questmaker, fread_string(fp));
			KEYS("Questowner", obj->questowner, fread_string(fp));
			break;

		case 'R':
			if (!str_cmp(word, "Resistance"))
			{
				obj->resistance = fread_number(fp, -999);
				if (obj->resistance == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Resistance \n\r");
					obj->resistance = 0;
				}
				fMatch = TRUE;
				break;
			}
			break;

		case 'S':
			KEYS("ShortDescr", obj->short_descr, fread_string(fp));
			if (!str_cmp(word, "Spectype"))
			{
				obj->spectype = fread_number(fp, -999);
				if (obj->spectype == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Spectype \n\r");
					obj->spectype = 0;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Specpower"))
			{
				obj->specpower = fread_number(fp, -999);
				if (obj->specpower == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Specpower \n\r");
					obj->specpower = 0;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Spell"))
			{
				int iValue;
				int sn;

				iValue = fread_number(fp, -999);
				sn = skill_lookup(fread_word(fp));
				if (iValue < 0 || iValue > 3)
				{
					bug("Fread_obj: bad iValue %d.", iValue);
				}
				else if (sn < 0)
				{
					bug("Fread_obj: unknown skill.", 0);
				}
				else
				{
					obj->value[iValue] = sn;
				}
				fMatch = TRUE;
				break;
			}
			break;

		case 'T':

			if (!str_cmp(word, "Timer"))
			{
				obj->timer = fread_number(fp, -999);
				if (obj->timer == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Timer \n\r");
					obj->timer = 0;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Toughness"))
			{
				obj->toughness = fread_number(fp, -999);
				if (obj->toughness == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Toughness \n\r");
					obj->toughness = 0;
				}
				fMatch = TRUE;
				break;
			}
			break;

		case 'V':
			if (!str_cmp(word, "Values"))
			{
				obj->value[0] = 0;
				obj->value[1] = 0;
				obj->value[2] = 0;
				obj->value[3] = 0;
				obj->value[0] = fread_number(fp, -999);
				obj->value[1] = fread_number(fp, -999);
				obj->value[2] = fread_number(fp, -999);
				obj->value[3] = fread_number(fp, -999);
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Vnum"))
			{
				int vnum;

				vnum = fread_number(fp, -999);
				if ((obj->pIndexData = get_obj_index(vnum)) == NULL)
					bug("Fread_obj: bad vnum %d.", vnum);
				else
					fVnum = TRUE;
				fMatch = TRUE;
				break;
			}
			break;

		case 'W':
			if (!str_cmp(word, "WearFlags"))
			{
				obj->wear_flags = fread_number(fp, -999);
				if (obj->wear_flags == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in WearFlags \n\r");
					obj->wear_flags = 0;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "WearLoc"))
			{
				obj->wear_loc = fread_number(fp, -999);
				if (obj->wear_loc == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in WearLoc \n\r");
					obj->wear_loc = 0;
				}
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "Weight"))
			{
				obj->weight = fread_number(fp, -999);
				if (obj->weight == -999)
				{
					errordetect = TRUE;
					snprintf(errormess, MAX_STRING_LENGTH, "Error in Weight \n\r");
					obj->weight = 0;
				}
				fMatch = TRUE;
				break;
			}
			break;
		}

		if (errordetect == TRUE)
		{

			char palmer[MAX_INPUT_LENGTH];
			send_to_char("ERROR DETECTED! Your pfile is buggered please contact a CODER and do NOT use this char again until told to do so.\n\r", ch);
			bug("ERROR DETECTED! Shagged pfile!! during fread_char load.", 0);
			bug(errormess, 0);
			snprintf(palmer, MAX_INPUT_LENGTH, "%s has a shagged pfile(ERROR DETECTED!), please inform a CODER!\n\r", ch->name);
			do_info(ch, palmer);
			do_info(ch, errormess);
			bug(palmer, 0);
			bug(errormess, 0);
			close_socket(ch->desc);
			errordetect = FALSE;
			return;
		}

		if (!fMatch)
		{

			char palmer[MAX_INPUT_LENGTH];

			bug("Fread_obj: no match.error 2 (valis)", 0);
			bug(palmer, 0);
			if (!strcmp(word, "SHAGGED"))
			{
				char palmer[MAX_INPUT_LENGTH];
				send_to_char("Your pfile is buggered please contact a CODER and do NOT use this char again until told to do so\n\r", ch);
				bug("Shagged pfile!! during fread_obj load.", 0);
				snprintf(palmer, MAX_INPUT_LENGTH, "%s has a shagged pfile, please inform a CODER!\n\r", ch->name);
				do_info(ch, palmer);
				bug(palmer, 0);
				bug(errormess, 0);
				close_socket(ch->desc);
				return;
			}
			else
				fread_to_eol(fp);
		}
	}
}
BAN_DATA *ban_list;

void save_siteban_info(void)
{
	BAN_DATA *pban;
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	char strsave[MAX_INPUT_LENGTH];
	int num;

	num = 0;
	fclose(fpReserve);
	strncpy(strsave, SITEBAN_FILE, strlen(SITEBAN_FILE));

	if (!(fp = fopen(strsave, "w")))
	{
		snprintf(buf, MAX_STRING_LENGTH, "Save_siteban:fopen %s:", strsave);
		bug(buf, 0);
		perror(strsave);
		return;
	}
	else
	{
		for (pban = ban_list; pban != NULL; pban = pban->next)
		{
			num++;
		}
		fprintf(fp, "%d\n", num);

		for (pban = ban_list; pban != NULL; pban = pban->next)
		{
			fprintf(fp, "%s\n", pban->name);
		}

		fprintf(fp, "#END");
		fclose(fp);
		fpReserve = fopen(NULL_FILE, "r");
		return;
	}
}

void read_siteban_info(void)
{
	BAN_DATA *pban;
	FILE *fp;
	int j, num;
	char strsave[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];

	fclose(fpReserve);
	strncpy(strsave, SITEBAN_FILE, MAX_INPUT_LENGTH);

	if (!(fp = fopen(strsave, "r")))
	{
		snprintf(buf, MAX_STRING_LENGTH, "Save_siteban:fopen %s:", strsave);
		bug(buf, 0);
		perror(strsave);
		return;
	}

	num = fread_number(fp, -999);

	for (j = 1; j <= num; j++)
	{
		if (ban_free == NULL)
		{
			pban = alloc_perm(sizeof(*pban));
		}
		else
		{
			pban = ban_free;
			ban_free = ban_free->next;
		}

		pban->name = str_dup(fread_word(fp));
		pban->next = ban_list;
		ban_list = pban;
	}
	fpReserve = fopen(NULL_FILE, "r");
}

void save_claninfo(void)
{
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	char strsave[MAX_INPUT_LENGTH];

	fclose(fpReserve);
	strncpy(strsave, CLAN_FILE, MAX_INPUT_LENGTH);
	if (!(fp = fopen(strsave, "w")))
	{
		snprintf(buf, MAX_STRING_LENGTH, "Save_claninfo: fopen %s: ", strsave);
		bug(buf, 0);
		perror(strsave);
		return;
	}
	else
	{
		int temp;
		int temp2;
		fprintf(fp, "%d\n", MAX_CLAN);
		for (temp = 1; temp < MAX_CLAN; temp++)
		{
			fprintf(fp, "%ld\n", clan_infotable[temp].members);
			for (temp2 = 1; temp2 < MAX_CLAN; temp2++)
				fprintf(fp, "%d\n", clan_infotable[temp].pkills[temp2]);

			for (temp2 = 1; temp2 < MAX_CLAN; temp2++)
				fprintf(fp, "%d\n", clan_infotable[temp].pkilled[temp2]);

			fprintf(fp, "%ld\n", clan_infotable[temp].mkills);
			fprintf(fp, "%d\n", clan_infotable[temp].mkilled);
		}
	}
	fclose(fp);
	fpReserve = fopen(NULL_FILE, "r");
	return;
}

void read_claninfo(void)
{
	int iClan = 0;
	int temp2;
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	char strsave[MAX_INPUT_LENGTH];
	int num = MAX_CLAN - 1;

	fclose(fpReserve);
	strncpy(strsave, CLAN_FILE, MAX_INPUT_LENGTH);
	if (!(fp = fopen(strsave, "r")))
	{
		snprintf(buf, MAX_STRING_LENGTH, "Read_claninfo: fopen %s: ", strsave);
		bug(buf, 0);
		perror(strsave);

		for (iClan = 1; iClan < num; iClan++)
		{
			clan_infotable[iClan].members = 0;
			clan_infotable[iClan].mkills = 0;
			clan_infotable[iClan].mkilled = 0;
			for (temp2 = 1; temp2 < num; temp2++)
			{
				(clan_infotable[iClan].pkills[temp2] = 0);
				(clan_infotable[iClan].pkilled[temp2] = 0);
			}
		}
		fpReserve = fopen(NULL_FILE, "r");
		save_claninfo();
		return;
	}
	else
		num = fread_number(fp, -999);
	for (iClan = 1; iClan < num; iClan++)
	{
		clan_infotable[iClan].members = fread_number(fp, -999);
		for (temp2 = 1; temp2 < num; temp2++)
			clan_infotable[iClan].pkills[temp2] = fread_number(fp, -999);
		for (temp2 = 1; temp2 < num; temp2++)
			clan_infotable[iClan].pkilled[temp2] = fread_number(fp, -999);
		clan_infotable[iClan].mkills = fread_number(fp, -999);
		clan_infotable[iClan].mkilled = fread_number(fp, -999);
	}

	fclose(fp);
	fpReserve = fopen(NULL_FILE, "r");
}

void do_updateleague(CHAR_DATA *ch, char *argument)
{
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	char strsave[MAX_INPUT_LENGTH];

	int n = 0;
	int m = 0;
	char *tester = 0;
	int maxplayers = 0;

	fclose(fpReserve);
	strncpy(strsave, LEAGUE_FILE, MAX_INPUT_LENGTH);
	if (!(fp = fopen(strsave, "r")))
	{
		snprintf(buf, MAX_STRING_LENGTH, "Read_leagueinfo: fopen %s: ", strsave);
		bug(buf, 0);
		perror(strsave);
		return;
	}
	else
	{
		for (n = 1; !(EOF == (atoi(tester))); n++)
		{
			ungetc((atoi(tester)), fp);
			league_infotable[n].name = fread_jword(fp);
			fread_word(fp);
			league_infotable[n].Pk = fread_number(fp, -999);
			league_infotable[n].Pd = fread_number(fp, -999);
			league_infotable[n].Mk = fread_number(fp, -999);
			league_infotable[n].Md = fread_number(fp, -999);
			fread_to_eol(fp);
			tester = (void *)(intptr_t)getc(fp);
			snprintf(buf, MAX_STRING_LENGTH, "name %s \n\r", league_infotable[n].name);
			send_to_char(buf, ch);
		}
		maxplayers = n;
	}

	fclose(fp);

	fpReserve = fopen(NULL_FILE, "r");

	for (m = 1; m < maxplayers; m++)
	{
		snprintf(buf, MAX_STRING_LENGTH, "%d => %s Pk:%d Pd:%d Mk:%ld Md:%d\n\r", m,
				 league_infotable[m].name, league_infotable[m].Pk,
				 league_infotable[m].Pd, league_infotable[m].Mk,
				 league_infotable[m].Md);
		send_to_char(buf, ch);
	}
}

void load_char_stances_json(cJSON *stances, CHAR_DATA *ch)
{
	cJSON *viper;
	cJSON *crane;
	cJSON *falcon;
	cJSON *mongoose;
	cJSON *bull;
	cJSON *swallow;
	cJSON *cobra;
	cJSON *lion;
	cJSON *grizzlie;
	cJSON *panther;

	ch->stance[CURRENT_STANCE] = cJSON_GetObjectItemCaseSensitive(stances, "current_stance")->valuedouble;
	ch->stance[AUTODROP] = cJSON_GetObjectItemCaseSensitive(stances, "autodrop")->valuedouble;
	viper = cJSON_GetObjectItemCaseSensitive(stances, "viper");
	crane = cJSON_GetObjectItemCaseSensitive(stances, "crane");
	falcon = cJSON_GetObjectItemCaseSensitive(stances, "falcon");
	mongoose = cJSON_GetObjectItemCaseSensitive(stances, "mongoose");
	bull = cJSON_GetObjectItemCaseSensitive(stances, "bull");
	swallow = cJSON_GetObjectItemCaseSensitive(stances, "swallow");
	cobra = cJSON_GetObjectItemCaseSensitive(stances, "cobra");
	lion = cJSON_GetObjectItemCaseSensitive(stances, "lion");
	grizzlie = cJSON_GetObjectItemCaseSensitive(stances, "grizzlie");
	panther = cJSON_GetObjectItemCaseSensitive(stances, "panther");

	ch->stance[STANCE_VIPER] = cJSON_GetObjectItemCaseSensitive(viper, "level")->valuedouble;
    ch->stance[STANCE_CRANE] = cJSON_GetObjectItemCaseSensitive(crane, "level")->valuedouble;
    ch->stance[STANCE_FALCON] = cJSON_GetObjectItemCaseSensitive(falcon, "level")->valuedouble;
    ch->stance[STANCE_MONGOOSE] = cJSON_GetObjectItemCaseSensitive(mongoose, "level")->valuedouble;
    ch->stance[STANCE_BULL] = cJSON_GetObjectItemCaseSensitive(bull, "level")->valuedouble;
    ch->stance[STANCE_SWALLOW] = cJSON_GetObjectItemCaseSensitive(swallow, "level")->valuedouble;
    ch->stance[STANCE_COBRA] = cJSON_GetObjectItemCaseSensitive(cobra, "level")->valuedouble;
    ch->stance[STANCE_LION] = cJSON_GetObjectItemCaseSensitive(lion, "level")->valuedouble;
    ch->stance[STANCE_GRIZZLIE] = cJSON_GetObjectItemCaseSensitive(grizzlie, "level")->valuedouble;
    ch->stance[STANCE_PANTHER] = cJSON_GetObjectItemCaseSensitive(panther, "level")->valuedouble;

    ch->tier_stance[STANCE_VIPER] = cJSON_GetObjectItemCaseSensitive(viper, "tier")->valuedouble;
	ch->tier_stance[STANCE_CRANE] = cJSON_GetObjectItemCaseSensitive(crane, "tier")->valuedouble;
	ch->tier_stance[STANCE_FALCON] = cJSON_GetObjectItemCaseSensitive(falcon, "tier")->valuedouble;
	ch->tier_stance[STANCE_MONGOOSE] = cJSON_GetObjectItemCaseSensitive(mongoose, "tier")->valuedouble;
	ch->tier_stance[STANCE_BULL] = cJSON_GetObjectItemCaseSensitive(bull, "tier")->valuedouble;
	ch->tier_stance[STANCE_SWALLOW] = cJSON_GetObjectItemCaseSensitive(swallow, "tier")->valuedouble;
	ch->tier_stance[STANCE_COBRA] = cJSON_GetObjectItemCaseSensitive(cobra, "tier")->valuedouble;
	ch->tier_stance[STANCE_LION] = cJSON_GetObjectItemCaseSensitive(lion, "tier")->valuedouble;
	ch->tier_stance[STANCE_GRIZZLIE] = cJSON_GetObjectItemCaseSensitive(grizzlie, "tier")->valuedouble;
	ch->tier_stance[STANCE_PANTHER] = cJSON_GetObjectItemCaseSensitive(panther, "tier")->valuedouble;

	return;
}

void load_char_spells_json(cJSON *spells, CHAR_DATA *ch)
{
	cJSON *green;
	cJSON *yellow;
	cJSON *red;
	cJSON *purple;
	cJSON *blue;

	green = cJSON_GetObjectItemCaseSensitive(spells, "green");
	yellow = cJSON_GetObjectItemCaseSensitive(spells, "yellow");
	red = cJSON_GetObjectItemCaseSensitive(spells, "red");
	purple = cJSON_GetObjectItemCaseSensitive(spells, "purple");
	blue = cJSON_GetObjectItemCaseSensitive(spells, "blue");

	ch->spl[SPELL_PURPLE] = cJSON_GetObjectItemCaseSensitive(purple, "level")->valuedouble;
	ch->spl[SPELL_RED] = cJSON_GetObjectItemCaseSensitive(red, "level")->valuedouble;
	ch->spl[SPELL_BLUE] = cJSON_GetObjectItemCaseSensitive(blue, "level")->valuedouble;
	ch->spl[SPELL_GREEN] = cJSON_GetObjectItemCaseSensitive(green, "level")->valuedouble;
	ch->spl[SPELL_YELLOW] = cJSON_GetObjectItemCaseSensitive(yellow, "level")->valuedouble;

	ch->tier_spl[SPELL_PURPLE] = cJSON_GetObjectItemCaseSensitive(purple, "tier")->valuedouble;
	ch->tier_spl[SPELL_RED] = cJSON_GetObjectItemCaseSensitive(red, "tier")->valuedouble;
	ch->tier_spl[SPELL_BLUE] = cJSON_GetObjectItemCaseSensitive(blue, "tier")->valuedouble;
	ch->tier_spl[SPELL_GREEN] = cJSON_GetObjectItemCaseSensitive(green, "tier")->valuedouble;
	ch->tier_spl[SPELL_YELLOW] = cJSON_GetObjectItemCaseSensitive(yellow, "tier")->valuedouble;

	return;
}

void load_char_weapons_json(cJSON *weapons, CHAR_DATA *ch)
{
	cJSON *hit;
	cJSON *slice;
	cJSON *stab;
	cJSON *slash;
	cJSON *whip;
	cJSON *claw;
	cJSON *blast;
	cJSON *pound;
	cJSON *crush;
	cJSON *bite;
	cJSON *grep;
	cJSON *pierce;
	cJSON *suck;

	hit = cJSON_GetObjectItemCaseSensitive(weapons, "hit");
	slice = cJSON_GetObjectItemCaseSensitive(weapons, "slice");
	stab = cJSON_GetObjectItemCaseSensitive(weapons, "stab");
	slash = cJSON_GetObjectItemCaseSensitive(weapons, "slash");
	whip = cJSON_GetObjectItemCaseSensitive(weapons, "whip");
	claw = cJSON_GetObjectItemCaseSensitive(weapons, "claw");
	blast = cJSON_GetObjectItemCaseSensitive(weapons, "blast");
	pound = cJSON_GetObjectItemCaseSensitive(weapons, "pound");
	crush = cJSON_GetObjectItemCaseSensitive(weapons, "crush");
	bite = cJSON_GetObjectItemCaseSensitive(weapons, "bite");
	grep = cJSON_GetObjectItemCaseSensitive(weapons, "grep");
	pierce = cJSON_GetObjectItemCaseSensitive(weapons, "pierce");
	suck = cJSON_GetObjectItemCaseSensitive(weapons, "suck");

	ch->wpn[WEAPON_HIT] = cJSON_GetObjectItemCaseSensitive(hit, "level")->valuedouble;
	ch->wpn[WEAPON_SLICE] = cJSON_GetObjectItemCaseSensitive(slice, "level")->valuedouble;
    ch->wpn[WEAPON_STAB] = cJSON_GetObjectItemCaseSensitive(stab, "level")->valuedouble;
    ch->wpn[WEAPON_SLASH] = cJSON_GetObjectItemCaseSensitive(slash, "level")->valuedouble;
    ch->wpn[WEAPON_WHIP] = cJSON_GetObjectItemCaseSensitive(whip, "level")->valuedouble;
    ch->wpn[WEAPON_CLAW] = cJSON_GetObjectItemCaseSensitive(claw, "level")->valuedouble;
    ch->wpn[WEAPON_BLAST] = cJSON_GetObjectItemCaseSensitive(blast, "level")->valuedouble;
    ch->wpn[WEAPON_POUND] = cJSON_GetObjectItemCaseSensitive(pound, "level")->valuedouble;
    ch->wpn[WEAPON_CRUSH] = cJSON_GetObjectItemCaseSensitive(crush, "level")->valuedouble;
    ch->wpn[WEAPON_BITE] = cJSON_GetObjectItemCaseSensitive(bite, "level")->valuedouble;
    ch->wpn[WEAPON_GREP] = cJSON_GetObjectItemCaseSensitive(grep, "level")->valuedouble;
    ch->wpn[WEAPON_PIERCE] = cJSON_GetObjectItemCaseSensitive(pierce, "level")->valuedouble;
    ch->wpn[WEAPON_SUCK] = cJSON_GetObjectItemCaseSensitive(suck, "level")->valuedouble;

    ch->tier_wpn[WEAPON_HIT] = cJSON_GetObjectItemCaseSensitive(hit, "tier")->valuedouble;
	ch->tier_wpn[WEAPON_SLICE] = cJSON_GetObjectItemCaseSensitive(slice, "tier")->valuedouble;
	ch->tier_wpn[WEAPON_STAB] = cJSON_GetObjectItemCaseSensitive(stab, "tier")->valuedouble;
	ch->tier_wpn[WEAPON_SLASH] = cJSON_GetObjectItemCaseSensitive(slash, "tier")->valuedouble;
	ch->tier_wpn[WEAPON_WHIP] = cJSON_GetObjectItemCaseSensitive(whip, "tier")->valuedouble;
	ch->tier_wpn[WEAPON_CLAW] = cJSON_GetObjectItemCaseSensitive(claw, "tier")->valuedouble;
	ch->tier_wpn[WEAPON_BLAST] = cJSON_GetObjectItemCaseSensitive(blast, "tier")->valuedouble;
	ch->tier_wpn[WEAPON_POUND] = cJSON_GetObjectItemCaseSensitive(pound, "tier")->valuedouble;
	ch->tier_wpn[WEAPON_CRUSH] = cJSON_GetObjectItemCaseSensitive(crush, "tier")->valuedouble;
	ch->tier_wpn[WEAPON_BITE] = cJSON_GetObjectItemCaseSensitive(bite, "tier")->valuedouble;
	ch->tier_wpn[WEAPON_GREP] = cJSON_GetObjectItemCaseSensitive(grep, "tier")->valuedouble;
	ch->tier_wpn[WEAPON_PIERCE] = cJSON_GetObjectItemCaseSensitive(pierce, "tier")->valuedouble;
	ch->tier_wpn[WEAPON_SUCK] = cJSON_GetObjectItemCaseSensitive(suck, "tier")->valuedouble;

	return;
}

void load_char_clandiscs_json(cJSON *clandiscs, CHAR_DATA *ch)
{
    CLANDISC_DATA *disc;
    cJSON *clandisc = NULL;
    char discError[MAX_STRING_LENGTH];

    cJSON_ArrayForEach(clandisc, clandiscs)
    {
        disc = NULL;
        disc = alloc_perm(sizeof(*disc));

        if( (disc = get_disc_by_name(cJSON_GetObjectItemCaseSensitive(clandisc, "Name")->valuestring)) == NULL)
        {
            snprintf(discError, MAX_STRING_LENGTH, "Error retrieving the discipline '%s'", cJSON_GetObjectItemCaseSensitive(clandisc, "Name")->valuestring);
            log_string(discError);
        }

        disc->option = cJSON_GetObjectItemCaseSensitive(clandisc, "Option")->valuestring;
        disc->timeLeft = cJSON_GetObjectItemCaseSensitive(clandisc, "Timeleft")->valuedouble;
        disc->isActive = cJSON_GetObjectItemCaseSensitive(clandisc, "IsActive")->valuedouble;

        // Set the player disc based on what we just loaded from the pfile
        SetPlayerDisc(ch, disc);
    }
}

void load_char_skills_json(cJSON *skills, CHAR_DATA *ch)
{
	int iHash;

	// Loop through all of the skills and drop it into the character data
	for (iHash = 0; iHash < MAX_SKILL; iHash++)
	{
		if(skill_table[iHash].name != NULL)
			ch->pcdata->learned[iHash] = cJSON_GetObjectItemCaseSensitive(skills, skill_table[iHash].name)->valuedouble;
	}


	return;
}

void load_char_affects_json(cJSON *affect_datas, CHAR_DATA *ch)
{
	AFFECT_DATA *paf;
	const cJSON *affect_data = NULL;

	cJSON_ArrayForEach(affect_data, affect_datas)
	{
		paf = alloc_perm(sizeof(*paf));
		paf->type = cJSON_GetObjectItemCaseSensitive(affect_data, "type")->valuedouble;
		paf->duration = cJSON_GetObjectItemCaseSensitive(affect_data, "duration")->valuedouble;
		paf->modifier = cJSON_GetObjectItemCaseSensitive(affect_data, "modifier")->valuedouble;
		paf->location = cJSON_GetObjectItemCaseSensitive(affect_data, "location")->valuedouble;
		paf->bitvector = cJSON_GetObjectItemCaseSensitive(affect_data, "bitvector")->valuedouble;

		paf->next = ch->affected;
		ch->affected = paf;
		top_affect++;
	}
}

void load_char_objects_json(cJSON *objects, CHAR_DATA *ch)
{
    OBJ_DATA *obj;
    IMBUE_DATA *id;
    static OBJ_DATA obj_zero;
    int iNest;
    bool fNest;
    bool fVnum;
    char errormess[MAX_STRING_LENGTH];
    cJSON *object;
    cJSON *imbue_data;
    cJSON *imbues;
    cJSON *descriptions;
    cJSON *description;
    cJSON *affect_datas;
    cJSON *affect;

    if (obj_free == NULL)
    {
        obj = alloc_perm(sizeof(*obj));
    }
    else
    {
        obj = obj_free;
        obj_free = obj_free->next;
    }

    *obj = obj_zero;
    obj->name = str_dup("");
    obj->short_descr = str_dup("");
    obj->description = str_dup("");
    obj->chpoweron = str_dup("(null)");
    obj->chpoweroff = str_dup("(null)");
    obj->chpoweruse = str_dup("(null)");
    obj->victpoweron = str_dup("(null)");
    obj->victpoweroff = str_dup("(null)");
    obj->victpoweruse = str_dup("(null)");
    obj->questmaker = str_dup("");
    obj->questowner = str_dup("");
    obj->spectype = 0;
    obj->specpower = 0;
    obj->condition = 100;
    obj->toughness = 0;
    obj->resistance = 100;
    obj->quest = 0;
    obj->points = 0;
    obj->value[0] = 0;
    obj->value[1] = 0;
    obj->value[2] = 0;
    obj->value[3] = 0;
    obj->wear_loc = 0;

    fNest = FALSE;
    fVnum = TRUE;
    iNest = 0;

    cJSON_ArrayForEach(object, objects)
    {
        log_string("Loading the next item!");
        log_string(cJSON_Print(object));

        // Fist, let's grab all of the easy data
        obj->condition = cJSON_GetObjectItemCaseSensitive(object, "Condition")->valuedouble;
        obj->cost = cJSON_GetObjectItemCaseSensitive(object, "Cost")->valuedouble;
        obj->description = cJSON_GetObjectItemCaseSensitive(object, "Description")->valuestring;
        obj->extra_flags = cJSON_GetObjectItemCaseSensitive(object, "ExtraFlags")->valuedouble;
        obj->item_type = cJSON_GetObjectItemCaseSensitive(object, "ItemType")->valuedouble;
        obj->level = cJSON_GetObjectItemCaseSensitive(object, "Level")->valuedouble;
        obj->name = cJSON_GetObjectItemCaseSensitive(object, "Name")->valuestring;

        cJSON *points = cJSON_GetObjectItemCaseSensitive(object, "Points");
        if(points)
            obj->points = points->valuedouble;

        cJSON *poweron = cJSON_GetObjectItemCaseSensitive(object, "Poweronch");
        if(poweron)
            obj->chpoweron = poweron->valuestring;

        cJSON *poweroff = cJSON_GetObjectItemCaseSensitive(object, "Poweroffch");
        if(poweroff)
            obj->chpoweroff = poweroff->valuestring;

        cJSON *poweruse = cJSON_GetObjectItemCaseSensitive(object, "Powerusech");
        if(poweruse)
            obj->chpoweruse = poweruse->valuestring;

        cJSON *victpoweron = cJSON_GetObjectItemCaseSensitive(object, "Poweronvict");
        if(victpoweron)
            obj->victpoweron = victpoweron->valuestring;

        cJSON *victpoweroff = cJSON_GetObjectItemCaseSensitive(object, "Poweroffvict");
        if(victpoweroff)
            obj->victpoweroff = cJSON_GetObjectItemCaseSensitive(object, "Poweroffvict")->valuestring;

        cJSON *victpoweruse = cJSON_GetObjectItemCaseSensitive(object, "Powerusevict");
        if(victpoweruse)
            obj->victpoweruse = victpoweruse->valuestring;

        cJSON *quest = cJSON_GetObjectItemCaseSensitive(object, "Quest");
        if(quest)
            obj->quest = quest->valuedouble;

        cJSON *questmaker =cJSON_GetObjectItemCaseSensitive(object, "Questmaker");
        if(questmaker)
            obj->questmaker = questmaker->valuestring;

        cJSON *questowner = cJSON_GetObjectItemCaseSensitive(object, "Questowner");
        if(questowner)
            obj->questowner = questowner->valuestring;

        obj->resistance = cJSON_GetObjectItemCaseSensitive(object, "Resistance")->valuedouble;
        obj->short_descr = cJSON_GetObjectItemCaseSensitive(object, "ShortDescr")->valuestring;

        cJSON *spectype = cJSON_GetObjectItemCaseSensitive(object, "Spectype");
        if(spectype)
            obj->spectype = spectype->valuedouble;

        cJSON *specpower = cJSON_GetObjectItemCaseSensitive(object, "Specpower");
        if(specpower)
            obj->specpower = specpower->valuedouble;

        obj->value[0] = cJSON_GetObjectItemCaseSensitive(object, "value0")->valuedouble;
        obj->value[1] = cJSON_GetObjectItemCaseSensitive(object, "value1")->valuedouble;
        obj->value[2] = cJSON_GetObjectItemCaseSensitive(object, "value2")->valuedouble;
        obj->value[3] = cJSON_GetObjectItemCaseSensitive(object, "value3")->valuedouble;
        obj->wear_flags = cJSON_GetObjectItemCaseSensitive(object, "WearFlags")->valuedouble;
        obj->wear_loc = cJSON_GetObjectItemCaseSensitive(object, "WearLoc")->valuedouble;
        obj->weight = cJSON_GetObjectItemCaseSensitive(object, "Weight")->valuedouble;

        // Now, let's grab the imbue data
        imbues = cJSON_GetObjectItemCaseSensitive(object, "imbues");

        cJSON_ArrayForEach(imbue_data, imbues)
        {
            if (imbue_free == NULL)
            {
                id = alloc_perm(sizeof(*id));
            }
            else
            {
                id = imbue_free;
                imbue_free = imbue_free->next;
            }

            id->name = cJSON_GetObjectItemCaseSensitive(imbue_data, "name")->valuestring;
            id->item_type = cJSON_GetObjectItemCaseSensitive(imbue_data, "item_type")->valuestring;
            id->affect_number = cJSON_GetObjectItemCaseSensitive(imbue_data, "affect_number")->valuedouble;

            id->next = obj->imbue;
            obj->imbue = id;
        }

        int vnum = cJSON_GetObjectItemCaseSensitive(object, "Vnum")->valuedouble;
        if ((obj->pIndexData = get_obj_index(vnum)) == NULL)
            bug("Fread_obj: bad vnum %d.", vnum);
        else
            fVnum = TRUE;

        descriptions = cJSON_GetObjectItemCaseSensitive(object, "extra_descriptions");
        cJSON_ArrayForEach(description, descriptions)
        {
            EXTRA_DESCR_DATA *ed;

            if (extra_descr_free == NULL)
            {
                ed = alloc_perm(sizeof(*ed));
            }
            else
            {
                ed = extra_descr_free;
                extra_descr_free = extra_descr_free->next;
            }

            ed->keyword = cJSON_GetObjectItemCaseSensitive(description, "keyword")->valuestring;
            ed->description = cJSON_GetObjectItemCaseSensitive(description, "description")->valuestring;
            ed->next = obj->extra_descr;
            obj->extra_descr = ed;
        }

        affect_datas = cJSON_GetObjectItemCaseSensitive(object, "affects");
        cJSON_ArrayForEach(affect, affect_datas)
        {
			AFFECT_DATA *paf;

			if ( affect_free == NULL )
			{
				paf = alloc_perm( sizeof(*paf) );
			}
			else
			{
				paf = affect_free;
				affect_free	= affect_free->next;
			}

			int *sn;

			sn = skill_lookup( cJSON_GetObjectItemCaseSensitive(affect, "type")->valuestring );
			if ( sn < 0 )
				bug( "Fread_char: unknown skill.", 0 );
			else
				paf->type = sn;

			paf->duration = cJSON_GetObjectItemCaseSensitive(affect, "duration")->valueint;
			paf->modifier = cJSON_GetObjectItemCaseSensitive(affect, "modifier")->valueint;
			paf->location = cJSON_GetObjectItemCaseSensitive(affect, "location")->valueint;
			paf->next = obj->affected;
			obj->affected = paf;
        }

        // Check to see if the item is nested in another item
        iNest = cJSON_GetObjectItemCaseSensitive(object, "Nest")->valuedouble;
        if (iNest < 0 || iNest >= MAX_NEST)
        {
            bug("Fread_obj: bad nest %d.", iNest);
        }
        else
        {
            rgObjNest[iNest] = obj;
            fNest = TRUE;
        }

        if (!fNest || !fVnum)
        {
            bug("Fread_obj: incomplete object.", 0);
            free_string(obj->name);
            free_string(obj->description);
            free_string(obj->short_descr);
            obj->next = obj_free;
            obj_free = obj;
            continue;
        }
        else
        {
            obj->next = object_list;
            object_list = obj;
            obj->pIndexData->count++;
            if (iNest == 0 || rgObjNest[iNest] == NULL)
                obj_to_char(obj, ch);
            else
                obj_to_obj(obj, rgObjNest[iNest - 1]);
            continue;
        }
        log_string("----------------------------------");
    }

    /**
    if (errordetect == TRUE)
    {

        char palmer[MAX_INPUT_LENGTH];
        send_to_char("ERROR DETECTED! Your pfile is buggered please contact a CODER and do NOT use this char again until told to do so.\n\r", ch);
        bug("ERROR DETECTED! Shagged pfile!! during fread_char load.", 0);
        bug(errormess, 0);
        snprintf(palmer, MAX_INPUT_LENGTH, "%s has a shagged pfile(ERROR DETECTED!), please inform a CODER!\n\r", ch->name);
        do_info(ch, palmer);
        do_info(ch, errormess);
        bug(palmer, 0);
        bug(errormess, 0);
        close_socket(ch->desc);
        errordetect = FALSE;
        return;
    }
    */

	return;
}