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
#include <unistd.h>
#include "merc.h"

#if !defined(macintosh)
extern int _filbuf args((FILE *));
#endif

/*
* Globals.
*/
HELP_DATA *help_first;
HELP_DATA *help_last;

SHOP_DATA *shop_first;
SHOP_DATA *shop_last;

SPEC_DATA *spec_first;
SPEC_DATA *spec_last;

CHAR_DATA *char_free;
EXTRA_DESCR_DATA *extra_descr_free;
NOTE_DATA *note_free;
OBJ_DATA *obj_free;
PC_DATA *pcdata_free;

char bug_buf[2 * MAX_INPUT_LENGTH];
CHAR_DATA *char_list;
char *help_greeting;
char log_buf[2 * MAX_INPUT_LENGTH];
KILL_DATA kill_table[MAX_LEVEL];
NOTE_DATA *note_list;
OBJ_DATA *object_list;
TIME_INFO_DATA time_info;
WEATHER_DATA weather_info;

sh_int gsn_backstab;
sh_int gsn_hide;
sh_int gsn_peek;
sh_int gsn_pick_lock;
sh_int gsn_sneak;
sh_int gsn_steal;

sh_int gsn_disarm;
sh_int gsn_fastdraw;
sh_int gsn_berserk;
sh_int gsn_punch;
sh_int gsn_kick;
sh_int gsn_hurl;
sh_int gsn_rescue;

sh_int gsn_blindness;
sh_int gsn_charm_person;
sh_int gsn_curse;
sh_int gsn_invis;
sh_int gsn_mass_invis;
sh_int gsn_poison;
sh_int gsn_sleep;
sh_int gsn_hunt;

/*
* Locals.
*/
MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
char *string_hash[MAX_KEY_HASH];

AREA_DATA *area_first;
AREA_DATA *area_last;

char *string_space;
char *top_string;
char str_empty[1];

int top_affect;
int top_area;
int top_rt;
int top_ed;
int top_exit;
int top_help;
int top_mob_index;
int top_obj_index;
int top_reset;
int top_room;
int top_shop;

/*
* Memory management.
* Increase MAX_STRING if you have too.
* Tune the others only if you understand what you're doing.
*/
/* #define        MAX_STRING  2096576 */
#define MAX_STRING 4193152
#define MAX_PERM_BLOCK 131072
/* #define        MAX_PERM_BLOCK 262144 */
#define MAX_MEM_LIST 11

void *rgFreeList[MAX_MEM_LIST];
const int rgSizeList[MAX_MEM_LIST] =
	{
		16, 32, 64, 128, 256, 1024, 2048, 4096, 8192, 16384, 32768 - 64};

int nAllocString;
int sAllocString;
int nAllocPerm;
int sAllocPerm;

/*
* Semi-locals.
*/
bool fBootDb;
bool initialBoot;
FILE *fpArea;
char strArea[MAX_INPUT_LENGTH];

/*
* Local booting procedures.
*/
void init_mm args((void));

AREA_DATA *load_area args((FILE * fp, char *fileName));
void load_helps args((FILE * fp, AREA_DATA *area));
void load_mobiles args((FILE * fp, AREA_DATA *area));
void load_objects args((FILE * fp, AREA_DATA *area));
void load_resets args((FILE * fp, AREA_DATA *area));
void load_rooms args((FILE * fp, AREA_DATA *area));
void load_shops args((FILE * fp, AREA_DATA *area));
void load_specials args((FILE * fp, AREA_DATA *area));
void load_notes args((void));

void fix_exits args((void));

void reset_area args((AREA_DATA * pArea));

void copyover_recover args(());
bool write_to_descriptor args((int desc, char *txt, int length));

/*
* Big mama top level function.
*/
void boot_db(bool fCopyOver)
{
	/*
* Init some data space stuff.
    */
	{
		if ((string_space = calloc(1, MAX_STRING)) == NULL)
		{
			bug("Boot_db: can't alloc %d string space.", MAX_STRING);
			exit(1);
		}
		top_string = string_space;
		fBootDb = TRUE;
		initialBoot = TRUE;
	}

	/*
    * Init random number generator.
    */
	{
		init_mm();
	}

	/*
    * Set time and weather.
    */
	{
		long lhour, lday, lmonth;

		lhour = (current_time - 650336715) / (PULSE_TICK / PULSE_PER_SECOND);
		time_info.hour = lhour % 24;
		lday = lhour / 24;
		time_info.day = lday % 35;
		lmonth = lday / 35;
		time_info.month = lmonth % 17;
		time_info.year = lmonth / 17;

		if (time_info.hour < 5)
			weather_info.sunlight = SUN_DARK;
		else if (time_info.hour < 6)
			weather_info.sunlight = SUN_RISE;
		else if (time_info.hour < 19)
			weather_info.sunlight = SUN_LIGHT;
		else if (time_info.hour < 20)
			weather_info.sunlight = SUN_SET;
		else
			weather_info.sunlight = SUN_DARK;

		weather_info.change = 0;
		weather_info.mmhg = 960;
		if (time_info.month >= 7 && time_info.month <= 12)
			weather_info.mmhg += number_range(1, 50);
		else
			weather_info.mmhg += number_range(1, 80);

		if (weather_info.mmhg <= 980)
			weather_info.sky = SKY_LIGHTNING;
		else if (weather_info.mmhg <= 1000)
			weather_info.sky = SKY_RAINING;
		else if (weather_info.mmhg <= 1020)
			weather_info.sky = SKY_CLOUDY;
		else
			weather_info.sky = SKY_CLOUDLESS;
	}

	/* read in the clan stuff. */

	read_claninfo();
	read_siteban_info();

	/*
    * Assign gsn's for skills which have them.
    */
	{
		int sn;

		for (sn = 0; sn < MAX_SKILL; sn++)
		{
			if (skill_table[sn].pgsn != NULL)
				*skill_table[sn].pgsn = sn;
		}
	}

	// Load up the area files
	load_areas();

	/*
    * Fix up exits.
    * Declare db booting over.
    * Reset all areas once.
    * Load up the notes file.
    */
	{
		fix_exits();
		fBootDb = FALSE;
		initialBoot = FALSE;
		area_update();
		load_donrooms();
		load_notes();
	}

	if (fCopyOver)
		copyover_recover();

	return;
}

/*
* Snarf an 'area' header line.
*/

void load_areas(void)
{
	FILE *fpList;

	if ((fpList = fopen(AREA_LIST, "r")) == NULL)
	{
		perror(AREA_LIST);
		exit(1);
	}

	char tempArea[MAX_INPUT_LENGTH];

	for (;;)
	{
		strncpy(tempArea, fread_word(fpList), MAX_INPUT_LENGTH);
		if (tempArea[0] == '$')
			break;

		if (tempArea[0] == '-')
		{
			fpArea = stdin;
		}
		else
		{
			strncpy(strArea, AREA_DIR, MAX_INPUT_LENGTH);
			strncat(strArea, tempArea, MAX_INPUT_LENGTH);

			log_string(strArea);
			
				load_area_file(strArea);
			

			//load_area_file(strArea);
		}
	}
	fclose(fpList);
}

void load_area_file(char *areaFile)
{
	AREA_DATA *area = NULL;
	if ((fpArea = fopen(areaFile, "r")) == NULL)
	{
		perror(areaFile);
		exit(1);
	}

	for (;;)
	{
		char *word;

		if (fread_letter(fpArea) != '#')
		{
			bug("Boot_db: # not found.", 0);
			exit(1);
		}

		word = fread_word(fpArea);

		if (word[0] == '$')
			break;
		else if (!str_cmp(word, "AREA"))
		{
			area = load_area(fpArea, areaFile);
		}
		else if (!str_cmp(word, "HELPS"))
			load_helps(fpArea, area);
		else if (!str_cmp(word, "MOBILES"))
			load_mobiles(fpArea, area);
		else if (!str_cmp(word, "OBJECTS"))
			load_objects(fpArea, area);
		else if (!str_cmp(word, "RESETS"))
			load_resets(fpArea, area);
		else if (!str_cmp(word, "ROOMS"))
			load_rooms(fpArea, area);
		else if (!str_cmp(word, "SHOPS"))
			load_shops(fpArea, area);
		else if (!str_cmp(word, "SPECIALS"))
			load_specials(fpArea, area);
		else
		{
			bug("Boot_db: bad section name.", 0);
			exit(1);
		}
	}

	if (fpArea != stdin)
	{
		fclose(fpArea);
	}
	strncpy(strArea, areaFile, MAX_INPUT_LENGTH);
	fpArea = NULL;
	fBootDb = FALSE;
}

AREA_DATA *load_area(FILE *fp, char *fileName)
{
	AREA_DATA *pArea;
	AREA_DATA *pAreaCheck;
	char creator[MAX_INPUT_LENGTH];

	pArea = alloc_perm(sizeof(*pArea));
	pArea->reset_first = NULL;
	pArea->reset_last = NULL;
	pArea->creator = alloc_perm(sizeof(creator));
	pArea->file = str_dup(fileName);
	pArea->name = one_argument(fread_string(fp), pArea->creator, MAX_INPUT_LENGTH);
	pArea->wasModified = FALSE;
	pArea->age = 15;
	pArea->nplayer = 0;
	pArea->helps = 0;
	pArea->mobiles = 0;
	pArea->rooms = 0;
	pArea->resets = 0;
	pArea->shops = 0;
	pArea->objects = 0;
	pArea->specials = 0;
	pArea->prevent_respawn = FALSE;

	for (pAreaCheck = area_first; pAreaCheck != NULL; pAreaCheck = pAreaCheck->next)
	{
		if (!str_cmp(pArea->name, pAreaCheck->name))
		{
			pAreaCheck->name = pArea->name;
			pAreaCheck->creator = pArea->creator;
			pAreaCheck->reset_first = pArea->reset_first;
			pAreaCheck->reset_last = pArea->reset_last;
			return pAreaCheck;
		}
	}

	if (area_first == NULL)
		area_first = pArea;

	if (area_last != NULL)
	{
		area_last->next = pArea;
	}

	area_last = pArea;
	pArea->next = NULL;

	top_area++;

	return pArea;
}

/*
* Snarf a help section.
*/
void load_helps(FILE *fp, AREA_DATA *area)
{
	HELP_DATA *pHelp;
	HELP_DATA *pHelpCheck;
	bool alreadyExists;

	for (;;)
	{
		alreadyExists = FALSE;
		pHelp = alloc_perm(sizeof(*pHelp));
		pHelp->level = fread_number(fp, -999);
		pHelp->keyword = fread_string(fp);
		pHelp->area = area;
		if (pHelp->keyword[0] == '$')
			break;
		pHelp->text = fread_string(fp);

		if (!str_cmp(pHelp->keyword, "greeting"))
			help_greeting = pHelp->text;

		// Lets check and see if this help structure already exists
		for (pHelpCheck = help_first; pHelpCheck != NULL; pHelpCheck = pHelpCheck->next)
		{
			if (is_name(pHelp->keyword, pHelpCheck->keyword))
			{
				pHelpCheck->text = pHelp->text;
				pHelpCheck->level = pHelp->level;
				pHelpCheck->keyword = pHelp->keyword;
				alreadyExists = TRUE;
				break;
			}
		}

		area->helps++;

		if (alreadyExists)
		{
			continue;
		}

		if (help_first == NULL)
			help_first = pHelp;
		if (help_last != NULL)
		{

			help_last->next = pHelp;
		}

		help_last = pHelp;
		pHelp->next = NULL;
		top_help++;
	}

	return;
}

/*
* Snarf a mob section.
*/
void load_mobiles(FILE *fp, AREA_DATA *area)
{
	MOB_INDEX_DATA *pMobIndex;
	MOB_INDEX_DATA *pMobExists;
	bool alreadyExists;

	for (;;)
	{
		alreadyExists = FALSE;
		sh_int vnum;
		char letter;
		int iHash;

		letter = fread_letter(fp);
		if (letter != '#')
		{
			bug("Load_mobiles: # not found.", 0);
			exit(1);
		}

		vnum = fread_number(fp, -999);
		if (vnum == 0)
			break;

		fBootDb = FALSE;
		if ((pMobExists = get_mob_index(vnum)) != NULL)
		{

			if (str_cmp(pMobExists->area->name, area->name))
			{
				bug("Load_mobiles: vnum %d duplicated.", vnum);
				exit(1); // Exit 1 may be too harsh unless we're on initial load
			}
			alreadyExists = TRUE;
		}
		fBootDb = TRUE;

		pMobIndex = alloc_perm(sizeof(*pMobIndex));
		pMobIndex->vnum = vnum;
		pMobIndex->player_name = fread_string(fp);
		pMobIndex->short_descr = fread_string(fp);
		pMobIndex->long_descr = fread_string(fp);
		pMobIndex->description = fread_string(fp);

		pMobIndex->long_descr[0] = UPPER(pMobIndex->long_descr[0]);
		pMobIndex->description[0] = UPPER(pMobIndex->description[0]);

		pMobIndex->act = fread_number(fp, -999) | ACT_IS_NPC;
		pMobIndex->affected_by = fread_number(fp, -999);
		pMobIndex->itemaffect = 0;
		pMobIndex->pShop = NULL;
		pMobIndex->alignment = fread_number(fp, -9999);
		letter = fread_letter(fp);
		pMobIndex->exp_level = number_fuzzy(fread_number(fp, -999));

		switch (letter)
		{
		case 'S':
			pMobIndex->level = pMobIndex->exp_level / 75;
			break;

		case 'C':
			pMobIndex->level = number_fuzzy(fread_number(fp, -999));
			break;

		default:
			bug("Load_mobiles: vnum %d unrecognised type.", vnum);
			exit(1);
		}

		/*
	   * The unused stuff is for imps who want to use the old-style
	   * stats-in-files method.
	   */

		pMobIndex->hitroll = fread_number(fp, -999);
		pMobIndex->ac = fread_number(fp, -9999);
		pMobIndex->hitnodice = fread_number(fp, -999);
		/* 'd' */ fread_letter(fp); /* Unused */
		pMobIndex->hitsizedice = fread_number(fp, -999);
		/* '+' */ fread_letter(fp); /* Unused */
		pMobIndex->hitplus = fread_number(fp, -999);
		pMobIndex->damnodice = fread_number(fp, -999);
		/* 'd' */ fread_letter(fp);						 /* Unused */
		pMobIndex->damsizedice = fread_number(fp, -999); /* Unused */
		/* '+' */ fread_letter(fp);						 /* Unused */
		pMobIndex->damplus = fread_number(fp, -999);	 /* Unused */
		pMobIndex->gold = fread_number(fp, -999);		 /* Unused */
		/* xp can't be used! */ fread_number(fp, -999);  /* Unused */
		/* position */ fread_number(fp, -999);			 /* Unused */
		/* start pos */ fread_number(fp, -999);			 /* Unused */

		/*
												* Back to meaningful values.
	   */
		pMobIndex->sex = fread_number(fp, -999);
		pMobIndex->area = area;

		iHash = vnum % MAX_KEY_HASH;
		pMobIndex->next = mob_index_hash[iHash];
		mob_index_hash[iHash] = pMobIndex;
		area->mobiles++;

		if (!alreadyExists)
		{
			top_mob_index++;
		}
	}

	return;
}

/*
* Snarf an obj section.
*/
void load_objects(FILE *fp, AREA_DATA *area)
{
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_INDEX_DATA *pObjExists;
	bool alreadyExists;

	for (;;)
	{
		alreadyExists = FALSE;
		sh_int vnum;
		char letter;
		int iHash;

		letter = fread_letter(fp);
		if (letter != '#')
		{
			bug("Load_objects: # not found.", 0);
			exit(1);
		}

		vnum = fread_number(fp, -999);
		if (vnum == 0)
			break;

		fBootDb = FALSE;

		if ((pObjExists = get_obj_index(vnum)) != NULL)
		{
			if (str_cmp(pObjExists->area->name, area->name))
			{
				bug("Load_objects: vnum %d duplicated.", vnum);
				exit(1); // Exit 1 may be too harsh unless we're on initial load
			}
			alreadyExists = TRUE;
		}
		fBootDb = TRUE;

		pObjIndex = alloc_perm(sizeof(*pObjIndex));
		pObjIndex->vnum = vnum;
		pObjIndex->name = fread_string(fp);
		pObjIndex->area = area;
		pObjIndex->short_descr = fread_string(fp);
		pObjIndex->description = fread_string(fp);
		/* Action description */ fread_string(fp);

		pObjIndex->short_descr[0] = LOWER(pObjIndex->short_descr[0]);
		pObjIndex->description[0] = UPPER(pObjIndex->description[0]);

		pObjIndex->item_type = fread_number(fp, -999);
		pObjIndex->extra_flags = fread_number(fp, -999);
		pObjIndex->wear_flags = fread_number(fp, -999);
		pObjIndex->value[0] = fread_number(fp, -999);
		pObjIndex->value[1] = fread_number(fp, -999);
		pObjIndex->value[2] = fread_number(fp, -999);
		pObjIndex->value[3] = fread_number(fp, -999);
		pObjIndex->weight = fread_number(fp, -999);
		pObjIndex->cost = fread_number(fp, -999); /* Unused */
		/* Cost per day */ fread_number(fp, -999);
		/*
	   if ( pObjIndex->item_type == ITEM_POTION )
	   SET_BIT(pObjIndex->extra_flags, ITEM_NODROP);
	   */
		pObjIndex->chpoweron = str_dup("");
		pObjIndex->chpoweroff = str_dup("");
		pObjIndex->chpoweruse = str_dup("");
		pObjIndex->victpoweron = str_dup("");
		pObjIndex->victpoweroff = str_dup("");
		pObjIndex->victpoweruse = str_dup("");
		pObjIndex->spectype = 0;
		pObjIndex->specpower = 0;

		for (;;)
		{
			char letter;

			letter = fread_letter(fp);

			if (letter == 'A')
			{
				AFFECT_DATA *paf;

				paf = alloc_perm(sizeof(*paf));
				paf->type = -1;
				paf->duration = -1;
				paf->location = fread_number(fp, -999);
				paf->modifier = fread_number(fp, -999);
				paf->bitvector = 0;
				paf->next = pObjIndex->affected;
				pObjIndex->affected = paf;
				top_affect++;
			}

			else if (letter == 'E')
			{
				EXTRA_DESCR_DATA *ed;

				ed = alloc_perm(sizeof(*ed));
				ed->keyword = fread_string(fp);
				ed->description = fread_string(fp);
				ed->next = pObjIndex->extra_descr;
				pObjIndex->extra_descr = ed;
				top_ed++;
			}

			else if (letter == 'Q')
			{
				pObjIndex->chpoweron = fread_string(fp);
				pObjIndex->chpoweroff = fread_string(fp);
				pObjIndex->chpoweruse = fread_string(fp);
				pObjIndex->victpoweron = fread_string(fp);
				pObjIndex->victpoweroff = fread_string(fp);
				pObjIndex->victpoweruse = fread_string(fp);
				pObjIndex->spectype = fread_number(fp, -999);
				pObjIndex->specpower = fread_number(fp, -999);
			}

			else
			{
				ungetc(letter, fp);
				break;
			}
		}

		/*
	   * Translate spell "slot numbers" to internal "skill numbers."
	   */
		switch (pObjIndex->item_type)
		{
		case ITEM_PILL:
		case ITEM_POTION:
		case ITEM_SCROLL:
			pObjIndex->value[1] = slot_lookup(pObjIndex->value[1]);
			pObjIndex->value[2] = slot_lookup(pObjIndex->value[2]);
			pObjIndex->value[3] = slot_lookup(pObjIndex->value[3]);
			break;

		case ITEM_BOMB:
			pObjIndex->value[3] = pObjIndex->value[3];
			break;

		case ITEM_STAFF:
		case ITEM_WAND:
			pObjIndex->value[3] = slot_lookup(pObjIndex->value[3]);
			break;
		}

		iHash = vnum % MAX_KEY_HASH;
		pObjIndex->next = obj_index_hash[iHash];
		obj_index_hash[iHash] = pObjIndex;

		area->objects++;

		if (!alreadyExists)
		{
			top_obj_index++;
		}
	}

	return;
}

/*
* Snarf a reset section.
*/
void load_resets(FILE *fp, AREA_DATA *area)
{
	RESET_DATA *pReset;

	if (area_last == NULL)
	{
		bug("Load_resets: no #AREA seen yet.", 0);
		exit(1);
	}

	for (;;)
	{
		ROOM_INDEX_DATA *pRoomIndex;
		EXIT_DATA *pexit;
		char letter;

		if ((letter = fread_letter(fp)) == 'S')
			break;

		if (letter == '*')
		{
			fread_to_eol(fp);
			continue;
		}

		pReset = alloc_perm(sizeof(*pReset));
		pReset->command = letter;
		/* if_flag */ fread_number(fp, -999);
		pReset->arg1 = fread_number(fp, -999);
		pReset->arg2 = fread_number(fp, -999);
		pReset->arg3 = (letter == 'G' || letter == 'R')
						   ? 0
						   : fread_number(fp, -999);
		fread_to_eol(fp);

		/*
	   * Validate parameters.
	   * We're calling the index functions for the side effect.
	   */
		switch (letter)
		{
		default:
			bug("Load_resets: bad command '%c'.", letter);
			exit(1);
			break;

		case 'M':
			get_mob_index(pReset->arg1);
			get_room_index(pReset->arg3);
			break;

		case 'O':
			get_obj_index(pReset->arg1);
			get_room_index(pReset->arg3);
			break;

		case 'P':
			get_obj_index(pReset->arg1);
			get_obj_index(pReset->arg3);
			break;

		case 'G':
		case 'E':
			get_obj_index(pReset->arg1);
			break;

		case 'D':
			pRoomIndex = get_room_index(pReset->arg1);

			if (pReset->arg2 < 0 || pReset->arg2 > 5 || (pexit = pRoomIndex->exit[pReset->arg2]) == NULL || !IS_SET(pexit->exit_info, EX_ISDOOR))
			{
				bug("Load_resets: 'D': exit %d not door.", pReset->arg2);
				exit(1);
			}

			if (pReset->arg3 < 0 || pReset->arg3 > 2)
			{
				bug("Load_resets: 'D': bad 'locks': %d.", pReset->arg3);
				exit(1);
			}

			break;

		case 'R':
			pRoomIndex = get_room_index(pReset->arg1);

			if (pReset->arg2 < 0 || pReset->arg2 > 6)
			{
				bug("Load_resets: 'R': bad exit %d.", pReset->arg2);
				exit(1);
			}

			break;
		}

		if (area->reset_first == NULL)
		{
			area->reset_first = pReset;
		}

		if (area->reset_last != NULL)
		{
			area->reset_last->next = pReset;
		}

		area->reset_last = pReset;
		area->resets++;
		pReset->next = NULL;
		top_reset++;
	}

	return;
}

/*
* Snarf a room section.
*/
void load_rooms(FILE *fp, AREA_DATA *area)
{
	ROOM_INDEX_DATA *pRoomIndex;
	ROOM_INDEX_DATA *pRoomExists;
	bool alreadyExists;

	if (area_last == NULL)
	{
		bug("Load_resets: no #AREA seen yet.", 0);
		exit(1);
	}

	for (;;)
	{
		alreadyExists = FALSE;
		sh_int vnum;
		char letter;
		int door;
		int iHash;

		letter = fread_letter(fp);
		if (letter != '#')
		{
			bug("Load_rooms: # not found.", 0);
			exit(1);
		}

		vnum = fread_number(fp, -999);
		if (vnum == 0)
			break;

		fBootDb = FALSE;
		if ((pRoomExists = get_room_index(vnum)) != NULL)
		{
			if (str_cmp(pRoomExists->area->name, area->name))
			{
				bug("Load_rooms: vnum %d duplicated.", vnum);
				exit(1); // Exit 1 may be too harsh unless we're on initial load
			}
			alreadyExists = TRUE;
		}
		fBootDb = TRUE;

		pRoomIndex = alloc_perm(sizeof(*pRoomIndex));
		pRoomIndex->people = NULL;
		pRoomIndex->contents = NULL;
		pRoomIndex->extra_descr = NULL;
		pRoomIndex->area = area;
		pRoomIndex->vnum = vnum;
		pRoomIndex->name = fread_string(fp);
		pRoomIndex->description = fread_string(fp);
		pRoomIndex->area_number = fread_number(fp, -999); /* Added by Palmer */
		pRoomIndex->room_flags = fread_number(fp, -999);
		pRoomIndex->sector_type = fread_number(fp, -999);
		pRoomIndex->light = 0;
		pRoomIndex->blood = 0;
		pRoomIndex->roomtext = NULL;
		pRoomIndex->bomb = 0;
		for (door = 0; door <= 5; door++)
			pRoomIndex->exit[door] = NULL;

		for (;;)
		{
			letter = fread_letter(fp);

			if (letter == 'S')
				break;

			if (letter == 'D')
			{
				EXIT_DATA *pexit;
				int locks;

				door = fread_number(fp, -999);
				if (door < 0 || door > 5)
				{
					bug("Fread_rooms: vnum %d has bad door number.", vnum);
					exit(1);
				}

				pexit = alloc_perm(sizeof(*pexit));
				pexit->description = fread_string(fp);
				pexit->keyword = fread_string(fp);
				pexit->exit_info = 0;
				locks = fread_number(fp, -999);
				pexit->key = fread_number(fp, -999);
				pexit->vnum = fread_number(fp, -999);

				switch (locks)
				{
				case 1:
					pexit->exit_info = EX_ISDOOR;
					break;
				case 2:
					pexit->exit_info = EX_ISDOOR | EX_PICKPROOF;
					break;
				case 3:
					pexit->exit_info = EX_ISDOOR | EX_BASHPROOF;
					break;
				case 4:
					pexit->exit_info = EX_ISDOOR | EX_PICKPROOF | EX_BASHPROOF;
					break;
				case 5:
					pexit->exit_info = EX_ISDOOR | EX_PASSPROOF;
					break;
				case 6:
					pexit->exit_info = EX_ISDOOR | EX_PICKPROOF | EX_PASSPROOF;
					break;
				case 7:
					pexit->exit_info = EX_ISDOOR | EX_BASHPROOF | EX_PASSPROOF;
					break;
				case 8:
					pexit->exit_info = EX_ISDOOR | EX_PICKPROOF | EX_BASHPROOF | EX_PASSPROOF;
					break;
				}

				pRoomIndex->exit[door] = pexit;
				top_exit++;
			}
			else if (letter == 'E')
			{
				EXTRA_DESCR_DATA *ed;

				ed = alloc_perm(sizeof(*ed));
				ed->keyword = fread_string(fp);
				ed->description = fread_string(fp);
				ed->next = pRoomIndex->extra_descr;
				pRoomIndex->extra_descr = ed;
				top_ed++;
			}
			else if (letter == 'T')
			{
				ROOMTEXT_DATA *rt;

				rt = alloc_perm(sizeof(*rt));
				rt->input = fread_string(fp);
				rt->output = fread_string(fp);
				rt->choutput = fread_string(fp);
				rt->name = fread_string(fp);
				rt->type = fread_number(fp, -999);
				rt->power = fread_number(fp, -999);
				rt->mob = fread_number(fp, -999);
				rt->next = pRoomIndex->roomtext;
				pRoomIndex->roomtext = rt;
				top_rt++;
			}
			else
			{
				bug("Load_rooms: vnum %d has flag not 'DES'.", vnum);
				exit(1);
			}
		}

		iHash = vnum % MAX_KEY_HASH;
		pRoomIndex->next = room_index_hash[iHash];
		room_index_hash[iHash] = pRoomIndex;

		area->rooms++;

		if (!alreadyExists)
		{
			top_room++;
		}
	}

	return;
}

/*
* Snarf a shop section.
*/
void load_shops(FILE *fp, AREA_DATA *area)
{
	SHOP_DATA *pShop;

	for (;;)
	{
		MOB_INDEX_DATA *pMobIndex;
		int iTrade;

		pShop = alloc_perm(sizeof(*pShop));
		pShop->keeper = fread_number(fp, -999);
		if (pShop->keeper == 0)
			break;
		for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
			pShop->buy_type[iTrade] = fread_number(fp, -999);
		pShop->profit_buy = fread_number(fp, -999);
		pShop->profit_sell = fread_number(fp, -999);
		pShop->open_hour = fread_number(fp, -999);
		pShop->close_hour = fread_number(fp, -999);
		pShop->area = area;
		pShop->comment = fread_string_eol(fp);
		pMobIndex = get_mob_index(pShop->keeper);
		pMobIndex->pShop = pShop;

		if (shop_first == NULL)
			shop_first = pShop;
		if (shop_last != NULL)
			shop_last->next = pShop;

		area->shops++;
		shop_last = pShop;
		pShop->next = NULL;
		top_shop++;
	}

	return;
}

/*
* Snarf spec proc declarations.
*/
void load_specials(FILE *fp, AREA_DATA *area)
{
	SPEC_DATA *pSpec;
	for (;;)
	{
		MOB_INDEX_DATA *pMobIndex;
		char letter;

		switch (letter = fread_letter(fp))
		{
		default:
			bug("Load_specials: letter '%c' not *MS.", letter);
			exit(1);

		case 'S':
			return;

		case '*':
			break;

		case 'M':
			pSpec = alloc_perm(sizeof(*pSpec));
			pSpec->command = letter;
			pSpec->vnum = fread_number(fp, -999);
			pSpec->spec = str_dup(fread_word(fp));
			pSpec->comment = fread_string_eol(fp);

			pSpec->area = area;
			pMobIndex = get_mob_index(pSpec->vnum);
			pMobIndex->spec_fun = spec_lookup(pSpec->spec);
			pMobIndex->spec = pSpec;

			if (pMobIndex->spec_fun == 0)
			{
				bug("Load_specials: 'M': vnum %d.", pMobIndex->vnum);
				exit(1);
			}

			area->specials++;

			if (spec_first == NULL)
				spec_first = pSpec;
			if (spec_last != NULL)
				spec_last->next = pSpec;

			spec_last = pSpec;
			pSpec->next = NULL;
			break;
		}

		fread_to_eol(fp);
	}
}

/*
* Snarf notes file.
*/
void load_notes(void)
{
	FILE *fp;
	NOTE_DATA *pnotelast;

	if ((fp = fopen(NOTE_FILE, "r")) == NULL)
		return;

	pnotelast = NULL;
	for (;;)
	{
		NOTE_DATA *pnote;
		char letter;

		do
		{
			letter = getc(fp);
			if (feof(fp))
			{
				fclose(fp);
				return;
			}
		} while (isspace(letter));
		ungetc(letter, fp);

		pnote = alloc_perm(sizeof(*pnote));

		if (str_cmp(fread_word(fp), "sender"))
			break;
		pnote->sender = fread_string(fp);

		if (str_cmp(fread_word(fp), "date"))
			break;
		pnote->date = fread_string(fp);

		if (str_cmp(fread_word(fp), "to"))
			break;
		pnote->to_list = fread_string(fp);

		if (str_cmp(fread_word(fp), "subject"))
			break;
		pnote->subject = fread_string(fp);

		if (str_cmp(fread_word(fp), "text"))
			break;
		pnote->text = fread_string(fp);

		if (note_list == NULL)
			note_list = pnote;
		else
			pnotelast->next = pnote;

		pnotelast = pnote;
	}

	strncpy(strArea, NOTE_FILE, MAX_INPUT_LENGTH);
	fpArea = fp;
	bug("Load_notes: bad key word.", 0);
	exit(1);
	return;
}

/*
* Translate all room exits from virtual to real.
* Has to be done after all rooms are read in.
* Check for bad reverse exits.
*/
void fix_exits(void)
{
	extern const sh_int rev_dir[];
	char buf[MAX_STRING_LENGTH];
	ROOM_INDEX_DATA *pRoomIndex;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;
	int iHash;
	int door;

	for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
	{
		for (pRoomIndex = room_index_hash[iHash];
			 pRoomIndex != NULL;
			 pRoomIndex = pRoomIndex->next)
		{
			bool fexit;

			fexit = FALSE;
			for (door = 0; door <= 5; door++)
			{
				if ((pexit = pRoomIndex->exit[door]) != NULL)
				{
					fexit = TRUE;
					if (pexit->vnum <= 0)
						pexit->to_room = NULL;
					else
						pexit->to_room = get_room_index(pexit->vnum);
				}
			}

			if (!fexit)
				SET_BIT(pRoomIndex->room_flags, ROOM_NO_MOB);
		}
	}

	for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
	{
		for (pRoomIndex = room_index_hash[iHash];
			 pRoomIndex != NULL;
			 pRoomIndex = pRoomIndex->next)
		{
			for (door = 0; door <= 5; door++)
			{
				if ((pexit = pRoomIndex->exit[door]) != NULL && (to_room = pexit->to_room) != NULL && (pexit_rev = to_room->exit[rev_dir[door]]) != NULL && pexit_rev->to_room != pRoomIndex)
				{
					snprintf(buf, MAX_STRING_LENGTH, "Fix_exits: %ld:%d -> %ld:%d -> %ld.",
							 pRoomIndex->vnum, door,
							 to_room->vnum, rev_dir[door],
							 (pexit_rev->to_room == NULL)
								 ? 0
								 : pexit_rev->to_room->vnum);
					/*        bug( buf, 0 ); */
				}
			}
		}
	}

	return;
}

/*
* Repopulate areas periodically.
*/
void area_update(void)
{
	AREA_DATA *pArea;

	for (pArea = area_first; pArea != NULL; pArea = pArea->next)
	{
		CHAR_DATA *pch;

		if (++pArea->age < 3)
			continue;

		/*
		  * Check for PC's.
	   */
		if (pArea->nplayer > 0 && pArea->age == 15 - 1)
		{
			for (pch = char_list; pch != NULL; pch = pch->next)
			{
				if (!IS_NPC(pch) && IS_AWAKE(pch) && pch->in_room != NULL && pch->in_room->area == pArea)
				{
					send_to_char("You hear an agonised scream in the distance.\n\r", pch);
				}
			}
		}

		/*
	   * Check age and reset.
	   * Note: Mud School resets every 3 minutes (not 15).
	   */
		if (pArea->nplayer == 0 || pArea->age >= 15 && pArea->prevent_respawn == FALSE)
		{
			ROOM_INDEX_DATA *pRoomIndex;

			reset_area(pArea);
			pArea->age = number_range(0, 3);
			pRoomIndex = get_room_index(ROOM_VNUM_SCHOOL);
			if (pRoomIndex != NULL && pArea == pRoomIndex->area)
				pArea->age = 15 - 3;
		}
	}

	return;
}

/*
* Reset one area.
*/
void reset_area(AREA_DATA *pArea)
{
	RESET_DATA *pReset;
	CHAR_DATA *mob;
	bool last;

	mob = NULL;
	last = TRUE;
	for (pReset = pArea->reset_first; pReset != NULL; pReset = pReset->next)
	{
		ROOM_INDEX_DATA *pRoomIndex;
		MOB_INDEX_DATA *pMobIndex;
		OBJ_INDEX_DATA *pObjIndex;
		OBJ_INDEX_DATA *pObjToIndex;
		EXIT_DATA *pexit;
		OBJ_DATA *obj;
		OBJ_DATA *obj_to;

		switch (pReset->command)
		{
		default:
			bug("Reset_area: bad command %c.", pReset->command);
			break;

		case 'M':
			if ((pMobIndex = get_mob_index(pReset->arg1)) == NULL)
			{
				bug("Reset_area: 'M': bad vnum %d.", pReset->arg1);
				continue;
			}

			if ((pRoomIndex = get_room_index(pReset->arg3)) == NULL)
			{
				bug("Reset_area: 'R': bad vnum %d.", pReset->arg3);
				continue;
			}

			if (pMobIndex->count >= pReset->arg2)
			{
				last = FALSE;
				break;
			}

			mob = create_mobile(pMobIndex);

			/*
		  * Check for pet shop.
		  */
			{
				ROOM_INDEX_DATA *pRoomIndexPrev;
				pRoomIndexPrev = get_room_index(pRoomIndex->vnum - 1);
				if (pRoomIndexPrev != NULL && IS_SET(pRoomIndexPrev->room_flags, ROOM_PET_SHOP))
					SET_BIT(mob->act, ACT_PET);
			}

			if (room_is_dark(pRoomIndex))
				SET_BIT(mob->affected_by, AFF_INFRARED);

			char_to_room(mob, pRoomIndex);
			last = TRUE;
			break;

		case 'O':
			if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL)
			{
				bug("Reset_area: 'O': bad vnum %d.", pReset->arg1);
				continue;
			}

			if ((pRoomIndex = get_room_index(pReset->arg3)) == NULL)
			{
				bug("Reset_area: 'R': bad vnum %d.", pReset->arg3);
				continue;
			}

			if (pArea->nplayer > 0 || count_obj_list(pObjIndex, pRoomIndex->contents) > 0)
			{
				last = FALSE;
				break;
			}

			obj = create_object(pObjIndex, number_range(1, 50));
			obj->cost = 0;
			obj_to_room(obj, pRoomIndex);
			last = TRUE;
			break;

		case 'P':
			if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL)
			{
				bug("Reset_area: 'P': bad vnum %d.", pReset->arg1);
				continue;
			}

			if ((pObjToIndex = get_obj_index(pReset->arg3)) == NULL)
			{
				bug("Reset_area: 'P': bad vnum %d.", pReset->arg3);
				continue;
			}

			if (pArea->nplayer > 0 || (obj_to = get_obj_type(pObjToIndex)) == NULL || obj_to->in_room == NULL || count_obj_list(pObjIndex, obj_to->contains) > 0)
			{
				last = FALSE;
				break;
			}

			obj = create_object(pObjIndex, number_range(1, 50));
			obj_to_obj(obj, obj_to);
			last = TRUE;
			break;

		case 'G':
		case 'E':
			if ((pObjIndex = get_obj_index(pReset->arg1)) == NULL)
			{
				bug("Reset_area: 'E' or 'G': bad vnum %d.", pReset->arg1);
				continue;
			}

			if (!last)
				break;

			if (mob == NULL)
			{
				bug("Reset_area: 'E' or 'G': null mob for vnum %d.",
					pReset->arg1);
				last = FALSE;
				break;
			}

			if (mob->pIndexData->pShop != NULL)
			{
				int olevel;

				switch (pObjIndex->item_type)
				{
				default:
					olevel = 0;
					break;
				case ITEM_PILL:
					olevel = number_range(0, 10);
					break;
				case ITEM_POTION:
					olevel = number_range(0, 10);
					break;
				case ITEM_SCROLL:
					olevel = number_range(5, 15);
					break;
				case ITEM_WAND:
					olevel = number_range(10, 20);
					break;
				case ITEM_STAFF:
					olevel = number_range(15, 25);
					break;
				case ITEM_BOMB:
					olevel = number_range(5, 100);
					break;
				case ITEM_ARMOR:
					olevel = number_range(5, 15);
					break;
				case ITEM_WEAPON:
					olevel = number_range(5, 15);
					break;
				}

				obj = create_object(pObjIndex, olevel);
				SET_BIT(obj->extra_flags, ITEM_INVENTORY);
			}
			else
			{
				obj = create_object(pObjIndex, number_range(1, 50));
			}
			obj_to_char(obj, mob);
			if (pReset->command == 'E')
				equip_char(mob, obj, pReset->arg3);
			last = TRUE;
			break;

		case 'D':
			if ((pRoomIndex = get_room_index(pReset->arg1)) == NULL)
			{
				bug("Reset_area: 'D': bad vnum %d.", pReset->arg1);
				continue;
			}

			if ((pexit = pRoomIndex->exit[pReset->arg2]) == NULL)
				break;

			switch (pReset->arg3)
			{
			case 0:
				REMOVE_BIT(pexit->exit_info, EX_CLOSED);
				REMOVE_BIT(pexit->exit_info, EX_LOCKED);
				break;

			case 1:
				SET_BIT(pexit->exit_info, EX_CLOSED);
				REMOVE_BIT(pexit->exit_info, EX_LOCKED);
				break;

			case 2:
				SET_BIT(pexit->exit_info, EX_CLOSED);
				SET_BIT(pexit->exit_info, EX_LOCKED);
				break;
			}

			last = TRUE;
			break;

		case 'R':
			if ((pRoomIndex = get_room_index(pReset->arg1)) == NULL)
			{
				bug("Reset_area: 'R': bad vnum %d.", pReset->arg1);
				continue;
			}

			{
				int d0;
				int d1;

				for (d0 = 0; d0 < pReset->arg2 - 1; d0++)
				{
					d1 = number_range(d0, pReset->arg2 - 1);
					pexit = pRoomIndex->exit[d0];
					pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
					pRoomIndex->exit[d1] = pexit;
				}
			}
			break;
		}
	}

	return;
}

/*
* Create an instance of a mobile.
*/
CHAR_DATA *create_mobile(MOB_INDEX_DATA *pMobIndex)
{
	CHAR_DATA *mob;

	if (pMobIndex == NULL)
	{
		bug("Create_mobile: NULL pMobIndex.", 0);
		exit(1);
	}

	if (char_free == NULL)
	{
		mob = alloc_perm(sizeof(*mob));
	}
	else
	{
		mob = char_free;
		char_free = char_free->next;
	}

	clear_char(mob);
	mob->pIndexData = pMobIndex;

	mob->name = pMobIndex->player_name;
	mob->short_descr = pMobIndex->short_descr;
	mob->long_descr = pMobIndex->long_descr;
	mob->description = pMobIndex->description;

	mob->spec_fun = pMobIndex->spec_fun;

	mob->exp_level = number_fuzzy(pMobIndex->exp_level);
	mob->level = pMobIndex->level;
	mob->home = 3001;
	mob->act = pMobIndex->act;
	mob->affected_by = pMobIndex->affected_by;
	mob->alignment = pMobIndex->alignment;
	mob->sex = pMobIndex->sex;

	if (pMobIndex->ac <= 0)
	{
		mob->armor = interpolate(mob->level, 100, -100);
	}
	else
	{
		mob->armor = pMobIndex->ac;
	}
	if (mob->armor < -800)
		mob->armor = -800;

	/*    if(pMobIndex->hitroll <= 0)
    {    
    mob->max_hit  = (mob->level * 8 + number_range(
				mob->level * mob->level / 4,
				mob->level * mob->level ));
				}
				else
				{
				mob->max_hit = pMobIndex->hitroll;
}    */

	if (pMobIndex->hitroll <= 0)
	{
		mob->hitroll = mob->level;
	}
	else
	{
		mob->hitroll = pMobIndex->hitroll;
	}

	/* Palmer added here  */
	if (pMobIndex->hitnodice <= 0 && pMobIndex->hitsizedice <= 0 && pMobIndex->hitplus <= 0)
	{
		mob->max_hit = mob->level;
	}
	else
	{
		mob->max_hit = (pMobIndex->hitnodice * number_range(0, pMobIndex->hitsizedice)) + pMobIndex->hitplus;
	}

	if (mob->level > 160)
		mob->max_hit = 50000;
	mob->hit = mob->max_hit;
	/* Palmer added here */
	if (pMobIndex->damnodice <= 0 && pMobIndex->damsizedice <= 0 && pMobIndex->damplus <= 0)
	{
		mob->damroll = mob->level;
	}
	else
	{
		mob->damroll = (pMobIndex->damnodice * number_range(0, pMobIndex->damsizedice)) + pMobIndex->damplus;
	}
	/*
    * Insert in list.
    */
	mob->next = char_list;
	char_list = mob;
	pMobIndex->count++;
	return mob;
}

/*
* Create an instance of an object.
*/
OBJ_DATA *create_object(OBJ_INDEX_DATA *pObjIndex, int level)
{
	static OBJ_DATA obj_zero;
	OBJ_DATA *obj;

	if (pObjIndex == NULL)
	{
		bug("Create_object: NULL pObjIndex.", 0);
		exit(1);
	}

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
	obj->pIndexData = pObjIndex;
	obj->in_room = NULL;
	obj->level = level;
	obj->wear_loc = -1;

	obj->name = pObjIndex->name;
	obj->short_descr = pObjIndex->short_descr;
	obj->description = pObjIndex->description;

	if (pObjIndex->chpoweron != NULL)
	{
		obj->chpoweron = pObjIndex->chpoweron;
		obj->chpoweroff = pObjIndex->chpoweroff;
		obj->chpoweruse = pObjIndex->chpoweruse;
		obj->victpoweron = pObjIndex->victpoweron;
		obj->victpoweroff = pObjIndex->victpoweroff;
		obj->victpoweruse = pObjIndex->victpoweruse;
		obj->spectype = pObjIndex->spectype;
		obj->specpower = pObjIndex->specpower;
	}
	else
	{
		obj->chpoweron = str_dup("(NULL)");
		obj->chpoweroff = str_dup("(NULL)");
		obj->chpoweruse = str_dup("(NULL)");
		obj->victpoweron = str_dup("(NULL)");
		obj->victpoweroff = str_dup("(NULL)");
		obj->victpoweruse = str_dup("(NULL)");
		obj->spectype = 0;
		obj->specpower = 0;
	}
	obj->item_type = pObjIndex->item_type;
	obj->extra_flags = pObjIndex->extra_flags;
	obj->wear_flags = pObjIndex->wear_flags;
	obj->value[0] = pObjIndex->value[0];
	obj->value[1] = pObjIndex->value[1];
	obj->value[2] = pObjIndex->value[2];
	obj->value[3] = pObjIndex->value[3];
	obj->weight = pObjIndex->weight;
	obj->cost = pObjIndex->cost;
	/*	 obj->cost     = number_fuzzy( 10 )
    * number_fuzzy( level ) * number_fuzzy( level );
    
    */
	obj->condition = 100;
	obj->toughness = 5;
	obj->resistance = 25;

	/*
    * Mess with object properties.
    */
	switch (obj->item_type)
	{
	default:
		bug("Read_object: vnum %d bad type.", pObjIndex->vnum);
		break;

	case ITEM_LIGHT:
	case ITEM_TREASURE:
	case ITEM_FURNITURE:
	case ITEM_TRASH:
	case ITEM_CONTAINER:
	case ITEM_DRINK_CON:
	case ITEM_KEY:
	case ITEM_FOOD:
	case ITEM_BOAT:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	case ITEM_FOUNTAIN:
	case ITEM_PORTAL:
	case ITEM_EGG:
	case ITEM_VOODOO:
	case ITEM_STAKE:
	case ITEM_MISSILE:
	case ITEM_AMMO:
	case ITEM_QUEST:
	case ITEM_QUESTCARD:
	case ITEM_QUESTMACHINE:
		break;

	case ITEM_SCROLL:
		obj->value[0] = number_fuzzy(obj->value[0]);
		break;

	case ITEM_BOMB:
	case ITEM_WAND:
	case ITEM_STAFF:
		obj->value[0] = number_fuzzy(obj->value[0]);
		obj->value[1] = number_fuzzy(obj->value[1]);
		obj->value[2] = obj->value[1];
		break;

	case ITEM_WEAPON:
		/*
    obj->value[1]  = number_fuzzy( number_fuzzy( 1 * level / 4 + 2 ) );
    obj->value[2]  = number_fuzzy( number_fuzzy( 3 * level / 4 + 6 ) );
	   */
		obj->value[1] = number_range(1, 10);
		obj->value[2] = number_range((obj->value[1] + 1), (obj->value[1] * 2));
		break;

	case ITEM_ARMOR:
		/*
    obj->value[0]  = number_fuzzy( level / 4 + 2 );
	   */
		obj->value[0] = number_range(5, 15);
		break;

	case ITEM_POTION:
	case ITEM_PILL:
		obj->value[0] = number_fuzzy(number_fuzzy(obj->value[0]));
		break;

	case ITEM_MONEY:
		obj->value[0] = obj->cost;
		break;
	}

	obj->next = object_list;
	object_list = obj;
	pObjIndex->count++;

	return obj;
}

/*
* Clear a new character.
*/
void clear_char(CHAR_DATA *ch)
{
	static CHAR_DATA ch_zero;

	*ch = ch_zero;
	ch->name = &str_empty[0];
	ch->short_descr = &str_empty[0];
	ch->long_descr = &str_empty[0];
	ch->description = &str_empty[0];
	ch->prompt = &str_empty[0];
	ch->prefix = &str_empty[0];
	ch->lord = &str_empty[0];
	ch->clan = &str_empty[0];
	ch->morph = &str_empty[0];
	ch->createtime = &str_empty[0];
	ch->lasthost = &str_empty[0];
	ch->powertype = &str_empty[0];
	ch->poweraction = &str_empty[0];
	ch->pload = &str_empty[0];

	ch->logon = current_time;
	ch->armor = 100;
	ch->position = POS_STANDING;
	ch->practice = 0;
	ch->hit = 20;
	ch->max_hit = 20;
	ch->mana = 100;
	ch->max_mana = 100;
	ch->move = 100;
	ch->max_move = 100;
	return;
}

/*
* Free a character.
*/
void free_char(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next_content;
		extract_obj(obj);
	}

	for (paf = ch->affected; paf != NULL; paf = paf_next)
	{
		paf_next = paf->next;
		affect_remove(ch, paf);
	}

	free_string(ch->name);
	free_string(ch->short_descr);
	free_string(ch->long_descr);
	free_string(ch->description);
	free_string(ch->prompt);
	free_string(ch->prefix);
	free_string(ch->lord);
	free_string(ch->clan);
	free_string(ch->morph);
	free_string(ch->createtime);
	free_string(ch->lasthost);
	free_string(ch->powertype);
	free_string(ch->poweraction);
	free_string(ch->pload);

	if (ch->pcdata != NULL)
	{
		free_string(ch->pcdata->pwd);
		free_string(ch->pcdata->email);
		ch->pcdata->hunting = char_free;
		free_string(ch->pcdata->bamfin);
		free_string(ch->pcdata->bamfout);
		free_string(ch->pcdata->title);
		ch->pcdata->next = pcdata_free;
		pcdata_free = ch->pcdata;
	}

	ch->next = char_free;
	char_free = ch;
	return;
}

/*
* Get an extra description from a list.
*/
char *get_extra_descr(const char *name, EXTRA_DESCR_DATA *ed)
{
	for (; ed != NULL; ed = ed->next)
	{
		if (is_name(name, ed->keyword))
			return ed->description;
	}
	return NULL;
}

/*
* Translates mob virtual number to its mob index struct.
* Hash table lookup.
*/
MOB_INDEX_DATA *get_mob_index(int vnum)
{
	MOB_INDEX_DATA *pMobIndex;

	for (pMobIndex = mob_index_hash[vnum % MAX_KEY_HASH];
		 pMobIndex != NULL;
		 pMobIndex = pMobIndex->next)
	{
		if (pMobIndex->vnum == vnum)
			return pMobIndex;
	}

	if (fBootDb)
	{
		bug("Get_mob_index: bad vnum %d.", vnum);
		exit(1);
	}

	return NULL;
}

/*
* Translates mob virtual number to its obj index struct.
* Hash table lookup.
*/
OBJ_INDEX_DATA *get_obj_index(int vnum)
{
	OBJ_INDEX_DATA *pObjIndex;

	for (pObjIndex = obj_index_hash[vnum % MAX_KEY_HASH];
		 pObjIndex != NULL;
		 pObjIndex = pObjIndex->next)
	{
		if (pObjIndex->vnum == vnum)
			return pObjIndex;
	}

	if (fBootDb)
	{
		bug("Get_obj_index: bad vnum %d.", vnum);
		exit(1);
	}

	return NULL;
}

/*
* Translates mob virtual number to its room index struct.
* Hash table lookup.
*/
ROOM_INDEX_DATA *get_room_index(int vnum)
{
	ROOM_INDEX_DATA *pRoomIndex;

	for (pRoomIndex = room_index_hash[vnum % MAX_KEY_HASH];
		 pRoomIndex != NULL;
		 pRoomIndex = pRoomIndex->next)
	{
		if (pRoomIndex->vnum == vnum)
			return pRoomIndex;
	}

	if (fBootDb)
	{
		bug("Get_room_index: bad vnum %d.", vnum);
		exit(1);
	}

	return NULL;
}

/*
* Read a letter from a file.
*/
char fread_letter(FILE *fp)
{
	char c;

	do
	{
		c = getc(fp);
	} while (isspace(c));

	return c;
}

/*
* Read a number from a file.
*/
int fread_number(FILE *fp, int ROE)
{
	int number;
	bool sign;
	char c;

	do
	{
		c = getc(fp);
	} while (isspace(c));

	number = 0;

	sign = FALSE;
	if (c == '+')
	{
		c = getc(fp);
	}
	else if (c == '-')
	{
		sign = TRUE;
		c = getc(fp);
	}

	if (!isdigit(c))
	{
		bug("Fread_number: bad format.", 0);
		return ROE;
		/* exit( 1 ); */
	}

	while (isdigit(c))
	{
		number = number * 10 + c - '0';
		c = getc(fp);
	}

	if (sign)
		number = 0 - number;

	if (c == '|')
		number += fread_number(fp, ROE);
	else if (c != ' ')
		ungetc(c, fp);

	return number;
}

/*
* Read and allocate space for a string from a file.
* These strings are read-only and shared.
* Strings are hashed:
*   each string prepended with hash pointer to prev string,
*   hash code is simply the string length.
* This function takes 40% to 50% of boot-up time.
*/
char *fread_string(FILE *fp)
{
	char *plast;
	char c;

	plast = top_string + sizeof(char *);
	if (plast > &string_space[MAX_STRING - MAX_STRING_LENGTH])
	{
		bug("Fread_string: MAX_STRING %d exceeded.", MAX_STRING);
		return str_dup("ERROR!! MAX_STRING EXCEEDED");
		/*exit( 1 );*/
	}

	/*
    * Skip blanks.
    * Read first char.
    */
	do
	{
		c = getc(fp);
	} while (isspace(c));

	if ((*plast++ = c) == '~')
		return &str_empty[0];

	for (;;)
	{
		/*
    * Back off the char type lookup,
    *   it was too dirty for portability.
    *   -- Furey
	   */
		switch (*plast = getc(fp))
		{
		default:
			plast++;
			break;

		case EOF:
			bug("Fread_string: EOF", 0);
			/*		 exit( 1 ); */
			/* maybe ???  return NULL ??? */
			return str_dup("ERROR!! EOF_REACHED");
			break;

		case '\n':
			plast++;
			*plast++ = '\r';
			break;

		case '\r':
			break;

		case '~':
			plast++;
			{
				union {
					char *pc;
					char rgc[sizeof(char *)];
				} u1;
				int ic;
				int iHash;
				char *pHash;
				char *pHashPrev;
				char *pString;

				plast[-1] = '\0';
				iHash = UMIN(MAX_KEY_HASH - 1, plast - 1 - top_string);
				for (pHash = string_hash[iHash]; pHash; pHash = pHashPrev)
				{
					for (ic = 0; ic < sizeof(char *); ic++)
						u1.rgc[ic] = pHash[ic];
					pHashPrev = u1.pc;
					pHash += sizeof(char *);

					if (top_string[sizeof(char *)] == pHash[0] && !strcmp(top_string + sizeof(char *) + 1, pHash + 1))
						return pHash;
				}

				if (fBootDb)
				{
					pString = top_string;
					top_string = plast;
					u1.pc = string_hash[iHash];
					for (ic = 0; ic < sizeof(char *); ic++)
						pString[ic] = u1.rgc[ic];
					string_hash[iHash] = pString;

					nAllocString += 1;
					sAllocString += top_string - pString;
					return pString + sizeof(char *);
				}
				else
				{
					return str_dup(top_string + sizeof(char *));
				}
			}
		}
	}
}

char *fread_string_eol( FILE *fp )
{
    static bool char_special[256-EOF];
    char *plast;
    char c;
 
    if ( char_special[EOF-EOF] != TRUE )
    {
        char_special[EOF -  EOF] = TRUE;
        char_special['\n' - EOF] = TRUE;
        char_special['\r' - EOF] = TRUE;
    }
 
    plast = top_string + sizeof(char *);
    if ( plast > &string_space[MAX_STRING - MAX_STRING_LENGTH] )
    {
        bug( "Fread_string: MAX_STRING %d exceeded.", MAX_STRING );
        exit( 1 );
    }
 
    /*
     * Skip blanks.
     * Read first char.
     */
    do
    {
        c = getc( fp );
    }
    while ( isspace(c) );
 
    if ( ( *plast++ = c ) == '\n')
        return &str_empty[0];
 
    for ( ;; )
    {
        if ( !char_special[ ( *plast++ = getc( fp ) ) - EOF ] )
            continue;
 
        switch ( plast[-1] )
        {
        default:
            break;
 
        case EOF:
            bug( "Fread_string_eol  EOF", 0 );
            exit( 1 );
            break;
 
        case '\n':  case '\r':
            {
                union
                {
                    char *      pc;
                    char        rgc[sizeof(char *)];
                } u1;
                int ic;
                int iHash;
                char *pHash;
                char *pHashPrev;
                char *pString;
 
                plast[-1] = '\0';
                iHash     = UMIN( MAX_KEY_HASH - 1, plast - 1 - top_string );
                for ( pHash = string_hash[iHash]; pHash; pHash = pHashPrev )
                {
                    for ( ic = 0; ic < sizeof(char *); ic++ )
                        u1.rgc[ic] = pHash[ic];
                    pHashPrev = u1.pc;
                    pHash    += sizeof(char *);
 
                    if ( top_string[sizeof(char *)] == pHash[0]
                    &&   !strcmp( top_string+sizeof(char *)+1, pHash+1 ) )
                        return pHash;
                }
 
                if ( fBootDb )
                {
                    pString             = top_string;
                    top_string          = plast;
                    u1.pc               = string_hash[iHash];
                    for ( ic = 0; ic < sizeof(char *); ic++ )
                        pString[ic] = u1.rgc[ic];
                    string_hash[iHash]  = pString;
 
                    nAllocString += 1;
                    sAllocString += top_string - pString;
                    return pString + sizeof(char *);
                }
                else
                {
                    return str_dup( top_string + sizeof(char *) );
                }
            }
        }
    }
}

/*
* Read to end of line (for comments).
*/
void fread_to_eol(FILE *fp)
{
	char c;

	do
	{
		c = getc(fp);
	} while (c != '\n' && c != '\r');

	do
	{
		c = getc(fp);
	} while (c == '\n' || c == '\r');

	ungetc(c, fp);
	return;
}

/*
* Read one word (into static buffer).
*/
char *fread_word(FILE *fp)
{
	static char word[MAX_INPUT_LENGTH];
	char *pword;
	char cEnd;

	do
	{
		cEnd = getc(fp);
	} while (isspace(cEnd));

	if (cEnd == '\'' || cEnd == '"')
	{
		pword = word;
	}
	else
	{
		word[0] = cEnd;
		pword = word + 1;
		cEnd = ' ';
	}

	for (; pword < word + MAX_INPUT_LENGTH; pword++)
	{
		*pword = getc(fp);
		if (cEnd == ' ' ? isspace(*pword) : *pword == cEnd)
		{
			if (cEnd == ' ')
				ungetc(*pword, fp);
			*pword = '\0';
			return word;
		}
	}

	bug("Fread_word: word too long.", 0);
	/*	 exit( 1 ); JOKER CHANGED HERE */
	/*       return NULL;              */
	word[0] = 'S';
	word[1] = 'H';
	word[2] = 'A';
	word[3] = 'G';
	word[4] = 'G';
	word[5] = 'E';
	word[6] = 'D';
	word[7] = '\0';
	return word;
}

/*
* Allocate some ordinary memory,
*   with the expectation of freeing it someday.
*/
void *alloc_mem(int sMem)
{
	void *pMem;
	int iList;

	for (iList = 0; iList < MAX_MEM_LIST; iList++)
	{
		if (sMem <= rgSizeList[iList])
			break;
	}

	if (iList == MAX_MEM_LIST)
	{
		bug("Alloc_mem: size %d too large.", sMem);

		/*	exit( 1 ); */
	}

	if (rgFreeList[iList] == NULL)
	{
		pMem = alloc_perm(rgSizeList[iList]);
	}
	else
	{
		pMem = rgFreeList[iList];
		rgFreeList[iList] = *((void **)rgFreeList[iList]);
	}

	return pMem;
}

/*
* Free some memory.
* Recycle it back onto the free list for blocks of that size.
*/
void free_mem(void *pMem, int sMem)
{
	int iList;
	char buf[MAX_INPUT_LENGTH];

	for (iList = 0; iList < MAX_MEM_LIST; iList++)
	{
		if (sMem <= rgSizeList[iList])
			break;
	}

	if (iList == MAX_MEM_LIST)
	{
		bug("Free_mem: size %d too large.", sMem);
		snprintf(buf, MAX_INPUT_LENGTH, "Alloc_mem: size %d too large.", sMem);
		/*   do_imminfo( buf );   */

		/*	exit( 1 ); */
	}

	*((void **)pMem) = rgFreeList[iList];
	rgFreeList[iList] = pMem;

	return;
}

/*
* Allocate some permanent memory.
* Permanent memory is never freed,
*   pointers into it may be copied safely.
*/
void *alloc_perm(int sMem)
{
	static char *pMemPerm;
	static int iMemPerm;
	void *pMem;

	while (sMem % sizeof(long) != 0)
		sMem++;
	if (sMem > MAX_PERM_BLOCK)
	{
		bug("Alloc_perm: %d too large.", sMem);
		exit(1);
	}

	if (pMemPerm == NULL || iMemPerm + sMem > MAX_PERM_BLOCK)
	{
		iMemPerm = 0;
		if ((pMemPerm = calloc(1, MAX_PERM_BLOCK)) == NULL)
		{
			perror("Alloc_perm");
			exit(1);
		}
	}

	pMem = pMemPerm + iMemPerm;
	iMemPerm += sMem;
	nAllocPerm += 1;
	sAllocPerm += sMem;
	return pMem;
}

/*
* Duplicate a string into dynamic memory.
* Fread_strings are read-only and shared.
*/
char *str_dup(const char *str)
{
	char *str_new;

	if (str[0] == '\0')
		return &str_empty[0];

	if (str >= string_space && str < top_string)
		return (char *)str;

	str_new = alloc_mem(strlen(str) + 1);
	strcpy(str_new, str);
	return str_new;
}

/*
* Free a string.
* Null is legal here to simplify callers.
* Read-only shared strings are not touched.
*/
void free_string(char *pstr)
{
	if (pstr == NULL || pstr == &str_empty[0] || (pstr >= string_space && pstr < top_string))
		return;

	free_mem(pstr, strlen(pstr) + 1);
	return;
}

void do_areas(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	AREA_DATA *pArea1;
	AREA_DATA *pArea2;
	int iArea;
	int iAreaHalf;

	iAreaHalf = (top_area + 1) / 2;
	pArea1 = area_first;
	pArea2 = area_first;
	for (iArea = 0; iArea < iAreaHalf; iArea++)
		pArea2 = pArea2->next;

	for (iArea = 0; iArea < iAreaHalf; iArea++)
	{
		snprintf(buf, MAX_STRING_LENGTH, "%-39s%-39s\n\r",
				 pArea1->name, (pArea2 != NULL) ? pArea2->name : "");
		send_to_char_formatted(buf, ch);
		pArea1 = pArea1->next;
		if (pArea2 != NULL)
			pArea2 = pArea2->next;
	}

	return;
}

void do_memory(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];

	snprintf(buf, MAX_STRING_LENGTH, "Affects %5d\n\r", top_affect);
	send_to_char_formatted(buf, ch);
	snprintf(buf, MAX_STRING_LENGTH, "Areas   %5d\n\r", top_area);
	send_to_char_formatted(buf, ch);
	snprintf(buf, MAX_STRING_LENGTH, "RmTxt   %5d\n\r", top_rt);
	send_to_char_formatted(buf, ch);
	snprintf(buf, MAX_STRING_LENGTH, "ExDes   %5d\n\r", top_ed);
	send_to_char_formatted(buf, ch);
	snprintf(buf, MAX_STRING_LENGTH, "Exits   %5d\n\r", top_exit);
	send_to_char_formatted(buf, ch);
	snprintf(buf, MAX_STRING_LENGTH, "Helps   %5d\n\r", top_help);
	send_to_char_formatted(buf, ch);
	snprintf(buf, MAX_STRING_LENGTH, "Mobs    %5d\n\r", top_mob_index);
	send_to_char_formatted(buf, ch);
	snprintf(buf, MAX_STRING_LENGTH, "Objs    %5d\n\r", top_obj_index);
	send_to_char_formatted(buf, ch);
	snprintf(buf, MAX_STRING_LENGTH, "Resets  %5d\n\r", top_reset);
	send_to_char_formatted(buf, ch);
	snprintf(buf, MAX_STRING_LENGTH, "Rooms   %5d\n\r", top_room);
	send_to_char_formatted(buf, ch);
	snprintf(buf, MAX_STRING_LENGTH, "Shops   %5d\n\r", top_shop);
	send_to_char_formatted(buf, ch);

	snprintf(buf, MAX_STRING_LENGTH, "Strings %5d strings of %7d bytes (max %d).\n\r",
			 nAllocString, sAllocString, MAX_STRING);
	send_to_char_formatted(buf, ch);

	snprintf(buf, MAX_STRING_LENGTH, "Perms   %5d blocks  of %7d bytes.\n\r",
			 nAllocPerm, sAllocPerm);
	send_to_char_formatted(buf, ch);

	return;
}

/*
* Stick a little fuzz on a number.
*/
int number_fuzzy(int number)
{
	switch (number_bits(2))
	{
	case 0:
		number -= 1;
		break;
	case 3:
		number += 1;
		break;
	}

	return UMAX(1, number);
}

/*
* Generate a random number.
*/
int number_range(int from, int to)
{
	int power;
	int number;

	if ((to = to - from + 1) <= 1)
		return from;

	for (power = 2; power < to; power <<= 1)
		;

	while ((number = number_mm() & (power - 1)) >= to)
		;

	return from + number;
}

/*
* Generate a percentile roll.
*/
int number_percent(void)
{
	int percent;

	while ((percent = number_mm() & (128 - 1)) > 99)
		;

	return 1 + percent;
}

/*
* Generate a random door.
*/
int number_door(void)
{
	int door;

	while ((door = number_mm() & (8 - 1)) > 5)
		;

	return door;
}

int number_bits(int width)
{
	return number_mm() & ((1 << width) - 1);
}

/*
* I've gotten too many bad reports on OS-supplied random number generators.
* This is the Mitchell-Moore algorithm from Knuth Volume II.
* Best to leave the constants alone unless you've read Knuth.
* -- Furey
*/
static int rgiState[2 + 55];

void init_mm()
{
	int *piState;
	int iState;

	piState = &rgiState[2];

	piState[-2] = 55 - 55;
	piState[-1] = 55 - 24;

	piState[0] = ((int)current_time) & ((1 << 30) - 1);
	piState[1] = 1;
	for (iState = 2; iState < 55; iState++)
	{
		piState[iState] = (piState[iState - 1] + piState[iState - 2]) & ((1 << 30) - 1);
	}
	return;
}

int number_mm(void)
{
	int *piState;
	int iState1;
	int iState2;
	int iRand;

	piState = &rgiState[2];
	iState1 = piState[-2];
	iState2 = piState[-1];
	iRand = (piState[iState1] + piState[iState2]) & ((1 << 30) - 1);
	piState[iState1] = iRand;
	if (++iState1 == 55)
		iState1 = 0;
	if (++iState2 == 55)
		iState2 = 0;
	piState[-2] = iState1;
	piState[-1] = iState2;
	return iRand >> 6;
}

/*
* Roll some dice.
*/
int dice(int number, int size)
{
	int idice;
	int sum;

	switch (size)
	{
	case 0:
		return 0;
	case 1:
		return number;
	}

	for (idice = 0, sum = 0; idice < number; idice++)
		sum += number_range(1, size);

	return sum;
}

/*
* Simple linear interpolation.
*/
int interpolate(int level, int value_00, int value_32)
{
	return value_00 + level * (value_32 - value_00) / 32;
}

/*
* Removes the tildes from a string.
* Used for player-entered strings that go into disk files.
*/
void smash_tilde(char *str)
{
	for (; *str != '\0'; str++)
	{
		if (*str == '~')
			*str = '-';
	}

	return;
}

/*
* Compare strings, case insensitive.
* Return TRUE if different
*   (compatibility with historical functions).
*/
bool str_cmp(const char *astr, const char *bstr)
{
	char buf[MAX_STRING_LENGTH];
	if (astr == NULL)
	{
		bug("Str_cmp: null astr.", 0);
		return TRUE;
	}

	if (bstr == NULL)
	{
		snprintf(buf, MAX_STRING_LENGTH, "Str_cmp: null bstr. Trying to compare: %s.", astr);
		bug(buf, 0);
		return TRUE;
	}

	for (; *astr || *bstr; astr++, bstr++)
	{
		if (LOWER(*astr) != LOWER(*bstr))
			return TRUE;
	}

	return FALSE;
}

/*
* Compare strings, case insensitive, for prefix matching.
* Return TRUE if astr not a prefix of bstr
*   (compatibility with historical functions).
*/
bool str_prefix(const char *astr, const char *bstr)
{
	if (astr == NULL)
	{
		bug("Strn_cmp: null astr.", 0);
		return TRUE;
	}

	if (bstr == NULL)
	{
		bug("Strn_cmp: null bstr.", 0);
		return TRUE;
	}

	for (; *astr; astr++, bstr++)
	{
		if (LOWER(*astr) != LOWER(*bstr))
			return TRUE;
	}

	return FALSE;
}

/*
* Compare strings, case insensitive, for match anywhere.
* Returns TRUE is astr not part of bstr.
*   (compatibility with historical functions).
*/
bool str_infix(const char *astr, const char *bstr)
{
	int sstr1;
	int sstr2;
	int ichar;
	char c0;

	if ((c0 = LOWER(astr[0])) == '\0')
		return FALSE;

	sstr1 = strlen(astr);
	sstr2 = strlen(bstr);

	for (ichar = 0; ichar <= sstr2 - sstr1; ichar++)
	{
		if (c0 == LOWER(bstr[ichar]) && !str_prefix(astr, bstr + ichar))
			return FALSE;
	}

	return TRUE;
}

/*
* Compare strings, case insensitive, for suffix matching.
* Return TRUE if astr not a suffix of bstr
*   (compatibility with historical functions).
*/
bool str_suffix(const char *astr, const char *bstr)
{
	int sstr1;
	int sstr2;

	sstr1 = strlen(astr);
	sstr2 = strlen(bstr);
	if (sstr1 <= sstr2 && !str_cmp(astr, bstr + sstr2 - sstr1))
		return FALSE;
	else
		return TRUE;
}

/*
* Returns an initial-capped string.
*/
char *capitalize(const char *str)
{
	static char strcap[MAX_STRING_LENGTH];
	int i;

	for (i = 0; str[i] != '\0'; i++)
		strcap[i] = LOWER(str[i]);
	strcap[i] = '\0';
	strcap[0] = UPPER(strcap[0]);
	return strcap;
}

/*
* Append a string to a file.
*/
void append_file(CHAR_DATA *ch, char *file, char *str)
{
	FILE *fp;

	if (IS_NPC(ch) || str[0] == '\0')
		return;

	fclose(fpReserve);
	if ((fp = fopen(file, "a")) == NULL)
	{
		perror(file);
		send_to_char("Could not open the file!\n\r", ch);
	}
	else
	{
		fprintf(fp, "%s\n", str);
		fclose(fp);
	}

	fpReserve = fopen(NULL_FILE, "r");
	return;
}

/*
* Reports a bug.
*/
void bug(const char *str, int param)
{
	char buf[MAX_STRING_LENGTH];
	FILE *fp;
	int n;

	if (fpArea != NULL)
	{
		int iLine;
		int iChar;

		if (fpArea == stdin)
		{
			iLine = 0;
		}
		else
		{
			iChar = ftell(fpArea);
			fseek(fpArea, 0, 0);
			for (iLine = 0; ftell(fpArea) < iChar; iLine++)
			{
				while (getc(fpArea) != '\n')
					;
			}
			fseek(fpArea, iChar, 0);
		}

		snprintf(buf, MAX_STRING_LENGTH, "[*****] FILE: %s LINE: %d ", strArea, iLine);
		log_string(buf);

		if ((fp = fopen("shutdown.txt", "a")) != NULL)
		{
			fprintf(fp, "[*****] %s\n", buf);
			fclose(fp);
		}
	}

	strncpy(buf, "[*****] BUG: ", MAX_STRING_LENGTH);
	n = strlen(buf);
	snprintf(buf + n, MAX_STRING_LENGTH - n, str, param);
	log_string(buf);

	fclose(fpReserve);
	if ((fp = fopen(BUG_FILE, "a")) != NULL)
	{
		fprintf(fp, "%s\n", buf);
		fclose(fp);
	}
	fpReserve = fopen(NULL_FILE, "r");

	return;
}

/*
* Writes a string to the log.
*/
void log_string(const char *str)
{
	struct tm *tm_time;
	static int prev_min = 0;

	tm_time = localtime(&current_time);
	if (tm_time->tm_min != prev_min)
	{
		char buf[32];

		fprintf(stderr, "**************************************************\n");
		strftime(buf, 32, "%d %b %Y %H:%M\n", tm_time);
		fprintf(stderr, "%s", buf);
		fprintf(stderr, "**************************************************\n");

		prev_min = tm_time->tm_min;
	}

	fprintf(stderr, "%02d : %s\n", tm_time->tm_sec, str);
	return;
}

/*
* This function is here to aid in debugging.
* If the last expression in a function is another function call,
*   gcc likes to generate a JMP instead of a CALL.
* This is called "tail chaining."
* It hoses the debugger call stack for that call.
* So I make this the last call in certain critical functions,
*   where I really need the call stack to be right for debugging!
*
* If you don't understand this, then LEAVE IT ALONE.
* Don't remove any calls to tail_chain anywhere.
*
* -- Furey
*/
void tail_chain(void)
{
	return;
}

char *fread_jword(FILE *fp)
{
	static char word[MAX_INPUT_LENGTH];
	char *pword;
	char cEnd;

	do
	{
		cEnd = getc(fp);
	} while (isspace(cEnd));

	if (cEnd == '\'' || cEnd == '"')
	{
		pword = word;
	}
	else
	{
		word[0] = cEnd;
		pword = word + 1;
		cEnd = ':';
	}

	for (; pword < word + MAX_INPUT_LENGTH; pword++)
	{
		*pword = getc(fp);
		if (cEnd == ':' ? ispunct(*pword) : *pword == cEnd)
		{
			if (cEnd == ':')
				ungetc(*pword, fp);
			*pword = '\0';
			return word;
		}
	}

	bug("Fread_jword: word too long.", 0);
	exit(1);
	return NULL;
}

extern int port, control; /* db.c */

void do_copyover(CHAR_DATA *ch, char *argument)
{
	FILE *fp;
	DESCRIPTOR_DATA *d, *d_next;
	char buf[100], buf2[100];

	fp = fopen(COPYOVER_FILE, "w");

	if (!fp)
	{
		send_to_char("Copyover file not writeable, aborted.\n\r", ch);
		log_string("Could not write to copyover file.");
		perror("do_copyover:fopen");
		return;
	}

	sprintf(buf, "Made it passed opening %s", COPYOVER_FILE);
	/* Consider changing all saved areas here, if you use OLC */

	/* do_asave (NULL, ""); - autosave changed areas */

	sprintf(buf, "\n\r *** COPYOVER by %s - please remain seated!\n\r", ch->name);

	/* For each playing descriptor, save its state */
	for (d = descriptor_list; d; d = d_next)
	{
		CHAR_DATA *och = d->original ? d->original : d->character;
		d_next = d->next; /* We delete from the list , so need to save this */

		if (!d->character || d->connected > CON_PLAYING) /* drop those logging on */
		{
			write_to_descriptor(d->descriptor, "\n\rIn the middle of a copyover. You picked the 3 seconds that you can't log in ... Go ahead and hit that retry button.\n\r", 0);
			close_socket(d); /* throw'em out */
		}
		else
		{
			fprintf(fp, "%d %s %s\n", d->descriptor, och->name, d->host);
			if (och->level == 1)
			{
				write_to_descriptor(d->descriptor, "Good news! Even though you haven't had 5mkills, we got you covered!\n\r", 0);
				advance_level(och);
				och->level++; /* Advance_level doesn't do that */
			}
			save_char_obj(och);
			write_to_descriptor(d->descriptor, buf, 0);
		}
	}

	fprintf(fp, "-1\n");
	fclose(fp);

	/* Close reserve and other always-open files and release other resources */

	fclose(fpReserve);

	/* exec - descriptors are inherited */

	sprintf(buf, "%d", port);
	sprintf(buf2, "%d", control);
	save_donrooms();
	send_to_char(EXE_FILE, ch);
	execl(EXE_FILE, "4000", buf, "copyover", buf2, (char *)NULL);

	/* Failed - sucessful exec will not return */

	perror("do_copyover: execl");
	send_to_char("Copyover FAILED!\n\r", ch);

	/* Here you might want to reopen fpReserve */
}

/* Recover from a copyover - load players */
void copyover_recover()
{
	DESCRIPTOR_DATA *d;
	FILE *fp;
	char name[100];
	char host[MAX_INPUT_LENGTH];
	int desc;
	bool fOld;

	log_string("Copyover recovery initiated");

	fp = fopen(COPYOVER_FILE, "r");

	if (!fp) /* there are some descriptors open which will hang forever then ? */
	{
		perror("copyover_recover:fopen");
		log_string("Copyover file not found. Exitting.\n\r");
		exit(1);
	}

	unlink(COPYOVER_FILE); /* In case something crashes - doesn't prevent reading    */

	for (;;)
	{
		if (fscanf(fp, "%d %s %s\n", &desc, name, host) < 3)
		{
			break;
		}
		if (desc == -1)
			break;

		/* Write something, and check if it goes error-free */
		if (!write_to_descriptor(desc, "\n\rRestoring from copyover...\n\r", 0))
		{
			close(desc); /* nope */
			continue;
		}

		d = alloc_perm(sizeof(DESCRIPTOR_DATA));
		init_descriptor(d, desc); /* set up various stuff */

		d->host = str_dup(host);
		d->next = descriptor_list;
		descriptor_list = d;
		d->connected = CON_COPYOVER_RECOVER; /* -15, so close_socket frees the char */

		/* Now, find the pfile */

		fOld = load_char_obj(d, name);

		if (!fOld) /* Player file not found?! */
		{
			write_to_descriptor(desc, "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r", 0);
			close_socket(d);
		}
		else /* ok! */
		{
			write_to_descriptor(desc, "\n\rCopyover recovery complete.\n\r", 0);

			/* Just In Case */
			if (!d->character->in_room)
				d->character->in_room = get_room_index(ROOM_VNUM_TEMPLE);

			/* Insert in the char_list */
			d->character->next = char_list;
			char_list = d->character;

			char_to_room(d->character, d->character->in_room);
			do_look(d->character, "");
			act("$n materializes!", d->character, NULL, NULL, TO_ROOM);
			d->connected = CON_PLAYING;
		}
	}

	fclose(fp);
}
