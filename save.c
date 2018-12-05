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

struct clan_info clan_infotable[MAX_CLAN];
struct league_info league_infotable[MAX_PLAYERS];

#if !defined(macintosh)
extern	int	_filbuf		args( (FILE *) );
#endif



/*
* Array of containers read for proper re-nesting of objects.
*/
#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];



/*
* Local functions.
*/
void	fwrite_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fwrite_obj	args( ( CHAR_DATA *ch,  OBJ_DATA  *obj,
					FILE *fp, int iNest ) );
void	fread_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fread_obj	args( ( CHAR_DATA *ch,  FILE *fp ) );


char *initial( const char *str )
{
    static char strint [MAX_STRING_LENGTH];
    strint[0] = UPPER( str[0] );
    return strint;
}

/*
* Save a character and inventory.
* Would be cool to save NPC's too for quest purposes,
*   some of the infrastructure is provided.
*/
void save_char_obj( CHAR_DATA *ch )
{
    char chlevel [15];
    char buf [MAX_INPUT_LENGTH];
    char strsave[MAX_INPUT_LENGTH];
    char temp[MAX_INPUT_LENGTH]; 
    FILE *fp;
    
    if ( IS_NPC(ch) || ch->level < 2 )
	   return;
    
    if ( ch->desc != NULL && ch->desc->original != NULL )
	   ch = ch->desc->original;
    
    ch->save_time = current_time;
    fclose( fpReserve );
    
#if !defined( macintosh) && !defined(MSDOS)
    snprintf( temp, MAX_INPUT_LENGTH, "%s%s%s%s", PLAYER_DIR, initial( ch->name ), "/", capitalize( ch->name) );
    snprintf( strsave, MAX_INPUT_LENGTH, "tmp.file");
#else	
    snprintf( temp, MAX_INPUT_LENGTH, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
    snprintf( strsave, MAX_INPUT_LENGTH, "tmp.file");
#endif
    
    
    if ( ( fp = fopen( strsave, "w" ) ) == NULL )
    {
	   bug( "Save_char_obj: fopen", 0 );
	   perror( strsave );
    }
    else
    {
	   fwrite_char( ch, fp );
	   if ( ch->carrying != NULL )
		  fwrite_obj( ch, ch->carrying, fp, 0 );
	   fprintf( fp, "#END\n" );
	   if      (ch->level >= 9) snprintf(chlevel,15,"<CODER>");
	   else if (ch->level == 8) snprintf(chlevel,15,"<GOD>");
	   else if (ch->level == 7) snprintf(chlevel,15,"<DEMIGOD>");
	   else if (ch->level == 6) snprintf(chlevel,15,"<DEITY>");
	   else if (ch->level == 5) snprintf(chlevel,15,"<Elder>");
	   else if (ch->level == 4) snprintf(chlevel,15,"<Builder>");
	   else if (ch->level == 3) snprintf(chlevel,15,"<Avatar>");
	   else                     snprintf(chlevel,15,"<Mortal>");
	   if (strlen(ch->lasttime) > 1)
		  snprintf(buf,MAX_INPUT_LENGTH, "%s Last logged in on %s", chlevel, ch->lasttime);
	   else
		  snprintf(buf,MAX_INPUT_LENGTH, "%s New player logged in on %s", chlevel, ch->createtime);
	   fprintf( fp, "%s", buf);
    }
    fclose( fp );
    
    fpReserve = fopen( NULL_FILE, "r" );
    /*    sprintf(temp, "cp %s%s%s%s %s%s%s%s\n\r" ,PLAYER_DIR, initial(ch->name),"/",  capitalize(ch->name), BACKUP_DIR,initial(ch->name), "/", capitalize(ch->name));
    system( temp );
    send_to_char( temp, ch);*/
    rename( strsave, temp );
    return;
}



/*
* Write the char.
*/
void fwrite_char( CHAR_DATA *ch, FILE *fp )
{
    AFFECT_DATA *paf;
    int sn;
    
    fprintf( fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER"		);
    
    fprintf( fp, "Name         %s~\n",	ch->name		);
    fprintf( fp, "ShortDescr   %s~\n",	ch->short_descr		);
    fprintf( fp, "LongDescr    %s~\n",	ch->long_descr		);
    fprintf( fp, "Description  %s~\n",	ch->description		);
	if (ch->prompt != NULL || !str_cmp(ch->prompt,"<%hhp %mm %vmv> "))
	{
		fprintf( fp, "Prompt %s~\n",      ch->prompt  	);
	}
    fprintf( fp, "Lord         %s~\n",	ch->lord		);
    fprintf( fp, "Clan         %s~\n",	ch->clan		);
    fprintf( fp, "Morph        %s~\n",	ch->morph		);
    fprintf( fp, "Createtime   %s~\n",	ch->createtime		);
    fprintf( fp, "Lasttime     %s~\n",	ch->lasttime		);
    fprintf( fp, "Lasthost     %s~\n",	ch->lasthost		);
    fprintf( fp, "Poweraction  %s~\n",	ch->poweraction		);
    fprintf( fp, "Powertype    %s~\n",	ch->powertype		);
    fprintf( fp, "Sex          %d\n",	ch->sex			);
    fprintf( fp, "Class        %d\n",	ch->class		);
    fprintf( fp, "Race         %d\n",	ch->race		);
	fprintf( fp, "RemortLevel  %d\n",	ch->remortlevel);
    fprintf( fp, "Immune       %ld\n",   ch->immune              );
    fprintf( fp, "Polyaff      %d\n",	ch->polyaff		);
    fprintf( fp, "Itemaffect   %d\n",	ch->itemaffect		);
    fprintf( fp, "Vampaff      %ld\n",   ch->vampaff             );
    fprintf( fp, "Vamppass     %ld\n",   ch->vamppass            );
    fprintf( fp, "Form         %d\n",	ch->form		);
    fprintf( fp, "Beast        %d\n",	ch->beast		);
    fprintf( fp, "Vampgen      %d\n",	ch->vampgen		);
    fprintf( fp, "Spectype     %d\n",	ch->spectype		);
    fprintf( fp, "Specpower    %d\n",	ch->specpower		);
    fprintf( fp, "Home         %ld\n",   ch->home                );
    fprintf( fp, "Level        %d\n",	ch->level		);
    fprintf( fp, "Trust        %d\n",	ch->trust		);
           fprintf( fp, "Played       %ld\n",
	   ch->played + (int) (current_time - ch->logon)		);
    fprintf( fp, "Room         %ld\n",
	   (  ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
	   && ch->was_in_room != NULL )
	   ? ch->was_in_room->vnum
	   : ch->in_room->vnum );
    
    fprintf( fp, "PkPdMkMd     %d %d %ld %d\n",
	   ch->pkill, ch->pdeath, ch->mkill, ch->mdeath );
    
    fprintf( fp, "Weapons      %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
	   ch->wpn[0], ch->wpn[1], ch->wpn[2], ch->wpn[3], ch->wpn[4], 
	   ch->wpn[5], ch->wpn[6], ch->wpn[7], ch->wpn[8], ch->wpn[9], 
	   ch->wpn[10], ch->wpn[11], ch->wpn[12] );
    fprintf( fp, "Spells       %d %d %d %d %d\n",
	   ch->spl[0], ch->spl[1], ch->spl[2], ch->spl[3], ch->spl[4] );
    fprintf( fp, "Combat       %d %d %d %d %d %d %d %d\n",
	   ch->cmbt[0], ch->cmbt[1], ch->cmbt[2], ch->cmbt[3],
	   ch->cmbt[4], ch->cmbt[5], ch->cmbt[6], ch->cmbt[7] );
    fprintf( fp, "Stance       %d %d %d %d %d %d %d %d %d %d %d %d\n",
	   ch->stance[0], ch->stance[1], ch->stance[2], ch->stance[3],
	   ch->stance[4], ch->stance[5], ch->stance[6], ch->stance[7],
	   ch->stance[8], ch->stance[9], ch->stance[10], ch->stance[11] );
    fprintf( fp, "Locationhp   %d %d %d %d %d %d %d\n",
	   ch->loc_hp[0], ch->loc_hp[1], ch->loc_hp[2], ch->loc_hp[3],
	   ch->loc_hp[4], ch->loc_hp[5], ch->loc_hp[6] );
    fprintf( fp, "HpManaMove   %d %d %d %d %d %d\n",
	   ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
    fprintf( fp, "Gold         %ld\n",   ch->gold                );
    fprintf( fp, "Bank         %ld\n",   ch->bank                );
    fprintf( fp, "Exp          %ld\n",   ch->exp                 );
    fprintf( fp, "Act          %ld\n",   ch->act                 );
    fprintf( fp, "Extra        %d\n",   ch->extra		);
    fprintf( fp, "AffectedBy   %ld\n",   ch->affected_by         );
    /* Bug fix from Alander */
    fprintf( fp, "Position     %d\n",
        ch->position == POS_FIGHTING ? POS_STANDING : ch->position );
    
    fprintf( fp, "Practice     %d\n",	ch->practice		);
    fprintf( fp, "SavingThrow  %d\n",	ch->saving_throw	);
    fprintf( fp, "Alignment    %d\n",	ch->alignment		);
    fprintf( fp, "Hitroll      %d\n",	ch->hitroll		);
    fprintf( fp, "Damroll      %d\n",	ch->damroll		);
    fprintf( fp, "Armor        %d\n",	ch->armor		);
    fprintf( fp, "Wimpy        %d\n",	ch->wimpy		);
    fprintf( fp, "Deaf         %ld\n",	ch->deaf		);
    fprintf( fp, "LagPenalty   %d\n",   ch->lagpenalty          );    
 
    if ( IS_NPC(ch) )
    {
           fprintf( fp, "Vnum         %ld\n",    ch->pIndexData->vnum    );
    }
    else
    {
	   fprintf( fp, "Password     %s~\n",	ch->pcdata->pwd		);
	   fprintf( fp, "Email        %s~\n",      ch->pcdata->email       );
	   fprintf( fp, "Bamfin       %s~\n",	ch->pcdata->bamfin	);
	   fprintf( fp, "Bamfout      %s~\n",	ch->pcdata->bamfout	);
	   fprintf( fp, "Title        %s~\n",	ch->pcdata->title	);
	   fprintf( fp, "AttrPerm     %d %d %d %d %d\n",
		  ch->pcdata->perm_str,
		  ch->pcdata->perm_int,
		  ch->pcdata->perm_wis,
		  ch->pcdata->perm_dex,
		  ch->pcdata->perm_con );
	   
	   fprintf( fp, "AttrMod      %d %d %d %d %d\n",
		  ch->pcdata->mod_str, 
		  ch->pcdata->mod_int, 
		  ch->pcdata->mod_wis,
		  ch->pcdata->mod_dex, 
		  ch->pcdata->mod_con );
	   
	   fprintf( fp, "Quest        %d\n", ch->pcdata->quest );
	   
	   if (ch->pcdata->obj_vnum != 0)
		  fprintf( fp, "Objvnum      %d\n", ch->pcdata->obj_vnum );
	   
	   fprintf( fp, "Condition    %d %d %d\n",
		  ch->pcdata->condition[0],
		  ch->pcdata->condition[1],
		  ch->pcdata->condition[2] );
	   
	   for ( sn = 0; sn < MAX_SKILL; sn++ )
	   {
		  if ( skill_table[sn].name != NULL && ch->pcdata->learned[sn] > 0 )
		  {
			 fprintf( fp, "Skill        %d '%s'\n",
				ch->pcdata->learned[sn], skill_table[sn].name );
		  }
	   }
    }
    
    for ( paf = ch->affected; paf != NULL; paf = paf->next )
    {
	   /* Thx Alander */
	   if ( paf->type < 0 || paf->type >= MAX_SKILL )
		  continue;
	   
	   fprintf( fp, "AffectData   '%s' %3d %3d %3d %10d\n",
		  skill_table[paf->type].name,
		  paf->duration,
		  paf->modifier,
		  paf->location,
		  paf->bitvector
		  );
    }
    
    fprintf( fp, "End\n\n" );
    return;
}



/*
* Write an object and its contents.
*/
void fwrite_obj( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest )
{
    EXTRA_DESCR_DATA *ed;
    AFFECT_DATA *paf;
    
    /*
    * Slick recursion to write lists backwards,
    *   so loading them will load in forwards order.
    */
    if ( obj->next_content != NULL )
	   fwrite_obj( ch, obj->next_content, fp, iNest );
    
	   /*
	   * Castrate storage characters.
    */
    if ( (obj->chobj != NULL && !IS_NPC(obj->chobj) &&
	   obj->chobj->pcdata->obj_vnum != 0) || obj->item_type == ITEM_KEY 
         /*  || obj->item_type == ITEM_PILL
           || obj->item_type == ITEM_SCROLL
           || obj->item_type == ITEM_POTION
           || obj->item_type == ITEM_WAND
           || obj->item_type == ITEM_STAFF*/)
	   return;
    fprintf( fp, "#OBJECT\n" );
    fprintf( fp, "Nest         %d\n",	iNest			     );
    fprintf( fp, "Name         %s~\n",	obj->name		     );
    fprintf( fp, "ShortDescr   %s~\n",	obj->short_descr	     );
    fprintf( fp, "Description  %s~\n",	obj->description	     );
    if (obj->chpoweron != NULL && str_cmp(obj->chpoweron,"(null)") && str_cmp(obj->chpoweron,""))
	   fprintf( fp, "Poweronch    %s~\n",	obj->chpoweron	     );
    if (obj->chpoweroff != NULL && str_cmp(obj->chpoweroff,"(null)") && str_cmp(obj->chpoweroff,""))
	   fprintf( fp, "Poweroffch   %s~\n",	obj->chpoweroff      );
    if (obj->chpoweruse != NULL && str_cmp(obj->chpoweruse,"(null)") && str_cmp(obj->chpoweruse,""))
	   fprintf( fp, "Powerusech   %s~\n",	obj->chpoweruse      );
    if (obj->victpoweron != NULL && str_cmp(obj->victpoweron,"(null)") && str_cmp(obj->victpoweron,""))
	   fprintf( fp, "Poweronvict  %s~\n",	obj->victpoweron     );
    if (obj->victpoweroff != NULL && str_cmp(obj->victpoweroff,"(null)") && str_cmp(obj->victpoweroff,""))
	   fprintf( fp, "Poweroffvict %s~\n",	obj->victpoweroff    );
    if (obj->victpoweruse != NULL && str_cmp(obj->victpoweruse,"(null)") && str_cmp(obj->victpoweruse,""))
	   fprintf( fp, "Powerusevict %s~\n",	obj->victpoweruse    );
    if (obj->questmaker != NULL && strlen(obj->questmaker) > 1)
	   fprintf( fp, "Questmaker   %s~\n",	obj->questmaker      );
    if (obj->questowner != NULL && strlen(obj->questowner) > 1)
	   fprintf( fp, "Questowner   %s~\n",	obj->questowner      );
    fprintf( fp, "Vnum         %ld\n",   obj->pIndexData->vnum        );
    fprintf( fp, "ExtraFlags   %ld\n",   obj->extra_flags             );
    fprintf( fp, "WearFlags    %ld\n",   obj->wear_flags              );
    fprintf( fp, "WearLoc      %d\n",	obj->wear_loc		     );
    fprintf( fp, "ItemType     %d\n",	obj->item_type		     );
    fprintf( fp, "Weight       %ld\n",   obj->weight                  );
    if (obj->spectype != 0)
	   fprintf( fp, "Spectype     %d\n",	obj->spectype	     );
    if (obj->specpower != 0)
	   fprintf( fp, "Specpower    %d\n",	obj->specpower	     );
    fprintf( fp, "Condition    %d\n",	obj->condition		     );
    fprintf( fp, "Toughness    %d\n",	obj->toughness		     );
    fprintf( fp, "Resistance   %d\n",	obj->resistance		     );
    if (obj->quest != 0)
           fprintf( fp, "Quest        %ld\n",    obj->quest           );
    if (obj->points != 0)
           fprintf( fp, "Points       %ld\n",    obj->points          );
    fprintf( fp, "Level        %d\n",	obj->level		     );
    fprintf( fp, "Timer        %d\n",	obj->timer		     );
    fprintf( fp, "Cost         %ld\n",   obj->cost                    );
    fprintf( fp, "Values       %d %d %d %d\n",
	   obj->value[0], obj->value[1], obj->value[2], obj->value[3]	     );
    
    switch ( obj->item_type )
    {
    case ITEM_POTION:
	   if ( obj->value[1] > 0 )
	   {
		  fprintf( fp, "Spell 1      '%s'\n", 
			 skill_table[obj->value[1]].name );
	   }
	   
	   if ( obj->value[2] > 0 )
	   {
		  fprintf( fp, "Spell 2      '%s'\n", 
			 skill_table[obj->value[2]].name );
	   }
	   
	   if ( obj->value[3] > 0 )
	   {
		  fprintf( fp, "Spell 3      '%s'\n", 
			 skill_table[obj->value[3]].name );
	   }
	   
	   break;
	   
    case ITEM_SCROLL:
	   if ( obj->value[1] > 0 )
	   {
		  fprintf( fp, "Spell 1      '%s'\n", 
			 skill_table[obj->value[1]].name );
	   }
	   
	   if ( obj->value[2] > 0 )
	   {
		  fprintf( fp, "Spell 2      '%s'\n", 
			 skill_table[obj->value[2]].name );
	   }
	   
	   if ( obj->value[3] > 0 )
	   {
		  fprintf( fp, "Spell 3      '%s'\n", 
			 skill_table[obj->value[3]].name );
	   }
	   
	   break;
	   
    case ITEM_PILL:
    case ITEM_STAFF:
    case ITEM_WAND:
	   if ( obj->value[3] > 0 )
	   {
		  fprintf( fp, "Spell 3      '%s'\n", 
			 skill_table[obj->value[3]].name );
	   }
	   
	   break;
    }
    
    for ( paf = obj->affected; paf != NULL; paf = paf->next )
    {
	   fprintf( fp, "AffectData   %d %d %d\n",
		  paf->duration,
		  paf->modifier,
		  paf->location
		  );
    }
    
    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next )
    {
	   fprintf( fp, "ExtraDescr   %s~ %s~\n",
		  ed->keyword, ed->description );
    }
    
    fprintf( fp, "End\n\n" );
    
    if ( obj->contains != NULL )
	   fwrite_obj( ch, obj->contains, fp, iNest + 1 );
    
    return;
}



/*
* Load a char and inventory into a new ch structure.
*/
bool load_char_obj( DESCRIPTOR_DATA *d, char *name )
{
    static PC_DATA pcdata_zero;
    char strsave[MAX_INPUT_LENGTH];
    CHAR_DATA *ch;
    FILE *fp;
    bool found;
    char buf[MAX_STRING_LENGTH];
    int i;
    
    
    
    if ( char_free == NULL )
    {
	   ch				= alloc_perm( sizeof(*ch) );
    }
    else
    {
	   ch				= char_free;
	   char_free			= char_free->next;
    }
    clear_char( ch );
    
    if ( pcdata_free == NULL )
    {
	   ch->pcdata			= alloc_perm( sizeof(*ch->pcdata) );
    }
    else
    {
	   ch->pcdata			= pcdata_free;
	   pcdata_free			= pcdata_free->next;
    }
    *ch->pcdata				= pcdata_zero;
    
    d->character			= ch;
    ch->desc				= d;
    ch->host_ip				= d->host_ip;
    ch->name				= str_dup( name );
    ch->act				= PLR_BLANK
	   | PLR_COMBINE
	   | PLR_PROMPT;
	ch->prompt 				= str_dup("<%hhp %mm %vmv> ");
    ch->extra				= 0;
    ch->pcdata->pwd			= str_dup( "" );
    ch->pcdata->email			= str_dup( "" );
    ch->pcdata->hunting                 = char_free;
    ch->pcdata->bamfin			= str_dup( "" );
    ch->pcdata->bamfout			= str_dup( "" );
    ch->pcdata->title			= str_dup( "" );
	ch->pcdata->tell_history	= alloc_perm(sizeof(*ch->pcdata->tell_history));
	ch->pcdata->chat_history	= alloc_perm(sizeof(*ch->pcdata->chat_history));
	ch->pcdata->newbie_history	= alloc_perm(sizeof(*ch->pcdata->newbie_history));
	ch->pcdata->clan_history	= alloc_perm(sizeof(*ch->pcdata->clan_history));
	ch->pcdata->immortal_history	= alloc_perm(sizeof(*ch->pcdata->immortal_history));
    for ( i=0; i < REVIEW_HISTORY_SIZE; ++i )
    {
    	ch->pcdata->tell_history->history[i] = str_dup( "" );
    	ch->pcdata->chat_history->history[i] = str_dup( "" );
    	ch->pcdata->newbie_history->history[i] = str_dup( "" );
    	ch->pcdata->clan_history->history[i] = str_dup( "" );
    	ch->pcdata->immortal_history->history[i] = str_dup( "" );
    }
    ch->pcdata->tell_history->position = 0;
    ch->pcdata->chat_history->position = 0;
    ch->pcdata->newbie_history->position = 0;
    ch->pcdata->clan_history->position = 0;
    ch->pcdata->immortal_history->position = 0;
    ch->lord				= str_dup( "" );
    ch->clan				= str_dup( "" );
    ch->morph				= str_dup( "" );
    ch->pload				= str_dup( "" );
    ch->createtime			= str_dup( ctime( &current_time ) );
    ch->lasttime			= str_dup( "" );
    ch->lasthost			= str_dup( "" );
    ch->poweraction			= str_dup( "" );
    ch->powertype			= str_dup( "" );
	ch->remortlevel			= 0;
    ch->spectype			= 0;
    ch->specpower			= 0;
    ch->mounted				= 0;
    ch->home				= 3001;
    ch->vampgen				= 0;
    ch->vampaff				= 0;
    ch->itemaffect			= 0;
    ch->vamppass			= -1;
    ch->polyaff				= 0;
    ch->immune				= 0;
    ch->form				= 32767;
    ch->beast				= 50;
    ch->loc_hp[0]			= 0;
    ch->loc_hp[1]			= 0;
    ch->loc_hp[2]			= 0;
    ch->loc_hp[3]			= 0;
    ch->loc_hp[4]			= 0;
    ch->loc_hp[5]			= 0;
    ch->loc_hp[6]			= 0;
    ch->wpn[0]				= 0;
    ch->wpn[1]				= 0;
    ch->wpn[2]				= 0;
    ch->wpn[3]				= 0;
    ch->wpn[4]				= 0;
    ch->wpn[5]				= 0;
    ch->wpn[6]				= 0;
    ch->wpn[7]				= 0;
    ch->wpn[8]				= 0;
    ch->wpn[9]				= 0;
    ch->wpn[10]				= 0;
    ch->wpn[11]				= 0;
    ch->wpn[12]				= 0;
    ch->spl[0]				= 4;
    ch->spl[1]				= 4;
    ch->spl[2]				= 4;
    ch->spl[3]				= 4;
    ch->spl[4]				= 4;
    ch->cmbt[0]				= 0;
    ch->cmbt[1]				= 0;
    ch->cmbt[2]				= 0;
    ch->cmbt[3]				= 0;
    ch->cmbt[4]				= 0;
    ch->cmbt[5]				= 0;
    ch->cmbt[6]				= 0;
    ch->cmbt[7]				= 0;
    ch->stance[0]			= 0;
    ch->stance[1]			= 0;
    ch->stance[2]			= 0;
    ch->stance[3]			= 0;
    ch->stance[4]			= 0;
    ch->stance[5]			= 0;
    ch->stance[6]			= 0;
    ch->stance[7]			= 0;
    ch->stance[8]			= 0;
    ch->stance[9]			= 0;
    ch->stance[10]			= 0;
    ch->stance[11]          = 0;
    ch->pkill				= 0;
    ch->pdeath				= 0;
    ch->mkill				= 0;
    ch->mdeath				= 0;
    ch->pcdata->perm_str		= 13;
    ch->pcdata->perm_int		= 13; 
    ch->pcdata->perm_wis		= 13;
    ch->pcdata->perm_dex		= 13;
    ch->pcdata->perm_con		= 13;
    ch->pcdata->quest			= 0;
    ch->pcdata->obj_vnum		= 0;
    ch->pcdata->condition[COND_THIRST]	= 48;
    ch->pcdata->condition[COND_FULL]	= 48;
    ch->gold				= 0;
    ch->bank				= 0;
    ch->choke_dam_message               = 0;
    
    found = FALSE;
    fclose( fpReserve );
#if !defined( macintosh) && !defined( MSDOS ) 
    snprintf( strsave, MAX_INPUT_LENGTH, "%s%s%s%s%s", PLAYER_DIR, initial( ch->name ), 
	   "/", capitalize( ch->name ), ".gz");
    if( (fp = fopen( strsave, "r") ) )
    {
	   fclose( fp );
	   snprintf( buf, MAX_INPUT_LENGTH, "gzip -dfq %s", strsave);
	   int systemReturn = system(buf);
    	if(systemReturn == -1)
    	{
        	bug("Could not load character", 0);
        	return FALSE;
    	}
    }
#endif
    
#if !defined( macintosh ) && !defined( MSDOS )
    snprintf( strsave, MAX_INPUT_LENGTH, "%s%s%s%s", PLAYER_DIR, initial( ch->name ), "/", capitalize( ch->name) );
#else
    snprintf( strsave, MAX_INPUT_LENGTH, "%s%s", PLAYER_DIR, capitalize( name ) );
#endif
    
    if ( ( fp = fopen( strsave, "r" ) ) != NULL )
    {
	   int iNest;
	   
	   for ( iNest = 0; iNest < MAX_NEST; iNest++ )
		  rgObjNest[iNest] = NULL;
	   
	   found = TRUE;
	   for ( ; ; )
	   {
		  char letter;
		  char *word;
		  
		  letter = fread_letter( fp );
		  if ( letter == '*' )
		  {
			 fread_to_eol( fp );
			 continue;
		  }
		  
		  if ( letter != '#' )
		  {
			 bug( "Load_char_obj: # not found.", 0 );
			 return FALSE;
			 break;
		  }
		  
		  word = fread_word( fp );
		  if      ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp );
		  else if ( !str_cmp( word, "OBJECT" ) ) fread_obj  ( ch, fp );
		  else if ( !str_cmp( word, "END"    ) ) break;
		  else
		  {
			 bug( "Load_char_obj: bad section.", 0 );
			 return FALSE;
			 break;
		  }
	   }
	   fclose( fp );
    }
    
    fpReserve = fopen( NULL_FILE, "r" );
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
	
#define KEY( literal, field, value, ROE, default )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				field  = value;			\
				fMatch = TRUE;                      \
				break;				\
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
#define KEYS( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				field  = value;			\
				fMatch = TRUE;                      \
				break;				\
}
	  
	  void fread_char( CHAR_DATA *ch, FILE *fp )
	  {
		 char buf[MAX_STRING_LENGTH];
		 char errormess[MAX_STRING_LENGTH];
		 char *word;
		 bool fMatch;
		 bool errordetect=FALSE;
		 
		 for ( ; ; )
		 {
			word   = feof( fp ) ? "End" : fread_word( fp );
			fMatch = FALSE;
			
			
			switch ( UPPER(word[0]) )
			{
			case '*':
			    fMatch = TRUE;
			    fread_to_eol( fp );
			    break;
			    
			case 'A':
			    if ( !str_cmp( word, "Act"))
			    {
				   ch->act = fread_number( fp, -999);
				   if(ch->act == -999) 
				   {
					  errordetect = TRUE;
					  snprintf( errormess, MAX_STRING_LENGTH, "Error in Act \n\r");
					  ch->act = PLR_BLANK|PLR_COMBINE|PLR_PROMPT;
				   }
				   fMatch = TRUE;
				   break;
			    }	
			    
			    if( !str_cmp( word, "AffectedBy"))
			    {
				   ch->affected_by = fread_number( fp, -999);
				   if( ch->affected_by == -999)
				   {
					  errordetect = TRUE;
					  ch->affected_by = 0;
					  snprintf( errormess, MAX_STRING_LENGTH, "Error in Act \n\r");
				   }
				   fMatch = TRUE;
				   break;
			    }
			    
			    if( !str_cmp( word, "Alignment"))
			    {
				   ch->alignment = fread_number(fp, -9999);
				   if( ch->alignment < -1000 || ch->alignment > 1000)
				   {
					  errordetect = TRUE;
					  snprintf( errormess, MAX_STRING_LENGTH, "Error in Alignment \n\r");
					  ch->alignment = 0;
				   }
				   fMatch = TRUE;
				   break;
			    }		
			    
			    
			    if( !str_cmp( word, "Armor"))
			    {	
				   ch->armor = fread_number( fp, -9999);
				   if( ch->armor == -9999)
				   {
					  errordetect = TRUE;
					  snprintf( errormess, MAX_STRING_LENGTH, "Error in Armor \n\r");
					  ch->armor = 100;
				   }
				   fMatch = TRUE;
				   break;
			    }
			    
			    if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
			    {
				   AFFECT_DATA *paf;
				   
				   if ( affect_free == NULL )
				   {
					  paf		= alloc_perm( sizeof(*paf) );
				   }
				   else
				   {
					  paf		= affect_free;
					  affect_free	= affect_free->next;
				   }
				   
				   if ( !str_cmp( word, "Affect" ) )
				   {
					  /* Obsolete 2.0 form. */
					  paf->type	= fread_number( fp, -9999 );
					  if( paf->type == -9999) errordetect = TRUE;
				   }
				   else
				   {
					  int sn;
					  
					  sn = skill_lookup( fread_word( fp ) );
					  if ( sn < 0 )
						 bug( "Fread_char: unknown skill.", 0 );
					  else
						 paf->type = sn;
				   }
				   
				   paf->duration	= fread_number( fp, -9999 );
				   if( paf->duration == -9999)
				   { errordetect = TRUE; paf->duration = 1; }
				   paf->modifier	= fread_number( fp, -9999 );
				   if( paf->modifier == -9999)
				   { errordetect = TRUE; paf->modifier = 0; } 
				   paf->location	= fread_number( fp, -9999 );
				   if( paf->location == -9999)
				   { errordetect = TRUE; paf->location = 1; }
				   paf->bitvector	= fread_number( fp, -9999 );
				   if( paf->bitvector == -9999)
				   { errordetect = TRUE; paf->bitvector = 0; }
				   paf->next	= ch->affected;
				   ch->affected	= paf;
				   if (errordetect)
						snprintf( errormess,  MAX_STRING_LENGTH, "Error in Affect(Data) \n\r");
				   fMatch = TRUE;
				   break;
			    }
			    
			    if ( !str_cmp( word, "AttrMod"  ) )
			    {
				   ch->pcdata->mod_str  = fread_number( fp, -999 );
				   if( ch->pcdata->mod_str == -999) 
				   { errordetect = TRUE; ch->pcdata->mod_str = 0; }
				   ch->pcdata->mod_int  = fread_number( fp, -999 );
				   if( ch->pcdata->mod_int == -999) 
				   { errordetect = TRUE; ch->pcdata->mod_int = 0; }
				   ch->pcdata->mod_wis  = fread_number( fp, -999 );
				   if( ch->pcdata->mod_wis == -999)
				   { errordetect = TRUE; ch->pcdata->mod_wis = 0; }
				   ch->pcdata->mod_dex  = fread_number( fp, -999 );
				   if( ch->pcdata->mod_dex == -999)
				   { errordetect = TRUE; ch->pcdata->mod_dex = 0; }
				   ch->pcdata->mod_con  = fread_number( fp, -999 );
				   if( ch->pcdata->mod_con == -999) 
				   { errordetect = TRUE; ch->pcdata->mod_con = 0; }
				   if (errordetect)
						snprintf( errormess,  MAX_STRING_LENGTH, "Error in AttrMod \n\r");
				   fMatch = TRUE;
				   break;
			    }
			    
			    if ( !str_cmp( word, "AttrPerm" ) )
			    {
				   ch->pcdata->perm_str = fread_number( fp, -999 );
				   if( ch->pcdata->perm_str == -999)
				   { errordetect = TRUE; ch->pcdata->perm_str = 13; }
				   ch->pcdata->perm_int = fread_number( fp, -999 );
				   if( ch->pcdata->perm_int == -999) 
				   { errordetect = TRUE; ch->pcdata->perm_int = 13; }
				   ch->pcdata->perm_wis = fread_number( fp, -999 );
				   if( ch->pcdata->perm_wis == -999) 
				   { errordetect = TRUE; ch->pcdata->perm_wis = 13; }
				   ch->pcdata->perm_dex = fread_number( fp, -999 );
				   if( ch->pcdata->perm_dex == -999)
				   { errordetect = TRUE; ch->pcdata->perm_dex = 13; }
				   ch->pcdata->perm_con = fread_number( fp, -999 );
				   if( ch->pcdata->perm_con == -999)
				   { errordetect = TRUE; ch->pcdata->perm_con = 13; }
				   if (errordetect)
						snprintf( errormess,  MAX_STRING_LENGTH, "Error in AttrPerm \n\r");
				   fMatch = TRUE;
				   break;
			    }
			    break;
			    
	case 'B':
	    KEYS( "Bamfin",	ch->pcdata->bamfin,	fread_string( fp ) );
	    KEYS( "Bamfout",	ch->pcdata->bamfout,	fread_string( fp ) );
	    if( !str_cmp( word, "Bank"))
	    {
		   ch->bank = fread_number( fp, -999);
		   if(ch->bank == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess, MAX_STRING_LENGTH, "Error in Bank \n\r");
			  ch->bank = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    if( !str_cmp( word, "Beast"))
	    {
		   ch->beast = fread_number(fp, -999);
		   if(ch->beast > 100 || ch->beast < 0)
		   {
			  errordetect = TRUE;
			  snprintf( errormess, MAX_STRING_LENGTH, "Error in Beast \n\r");
			  bug( errormess , 0 );
			  ch->beast = 50;
		   }
		   fMatch = TRUE;
		   break;
	    }	
	    break;
	    
	case 'C':
	    KEYS( "Clan",	ch->clan,		fread_string( fp ) );
	    
	    if( !str_cmp( word, "Class"))
	    {
		   ch->class = fread_number( fp, -999);
		   if( ch->class == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess, MAX_STRING_LENGTH, "Error in Class \n\r");
			  ch->class = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }	
	    
	    if ( !str_cmp( word, "Combat" ) )
	    {
		   ch->cmbt[0]	= fread_number( fp, -999 );
		   if( ch->cmbt[0] < 0 || ch->cmbt[0] > 200) 
		   { errordetect = TRUE; ch->cmbt[0] = 0; }
		   ch->cmbt[1]	= fread_number( fp, -999 );
		   if( ch->cmbt[1] < 0 || ch->cmbt[1] > 200) 
		   { errordetect = TRUE; ch->cmbt[1] = 0; }
		   ch->cmbt[2]	= fread_number( fp, -999 );
		   if( ch->cmbt[2] < 0 || ch->cmbt[2] > 200) 
		   { errordetect = TRUE; ch->cmbt[2] = 0; }
		   ch->cmbt[3]	= fread_number( fp, -999 );
		   if( ch->cmbt[3] < 0 || ch->cmbt[3] > 200) 
		   { errordetect = TRUE; ch->cmbt[3] = 0; }
		   ch->cmbt[4]	= fread_number( fp, -999 );
		   if( ch->cmbt[4] < 0 || ch->cmbt[4] > 200) 
		   { errordetect = TRUE; ch->cmbt[4] = 0; }
		   ch->cmbt[5]	= fread_number( fp, -999 );
		   if( ch->cmbt[5] < 0 || ch->cmbt[5] > 200) 
		   { errordetect = TRUE; ch->cmbt[5] = 0; }
		   ch->cmbt[6]	= fread_number( fp, -999 );
		   if( ch->cmbt[6] < 0 || ch->cmbt[6] > 200) 
		   { errordetect = TRUE; ch->cmbt[6] = 0; }
		   ch->cmbt[7]	= fread_number( fp, -999 );
		   if( ch->cmbt[7] < 0 || ch->cmbt[7] > 200) 
		   { errordetect = TRUE; ch->cmbt[7] = 0; }
		   if (errordetect)
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in Combat \n\r");
		   fMatch = TRUE;
		   break;
	    }
	    
	    if ( !str_cmp( word, "Condition" ) )
	    {
		   ch->pcdata->condition[0] = fread_number( fp, -999 );
		   if( ch->pcdata->condition[0] > 100 || ch->pcdata->condition[0] < 0)
		   { errordetect = TRUE; ch->pcdata->condition[0] = 48; }
		   ch->pcdata->condition[1] = fread_number( fp, -999 );
		   if( ch->pcdata->condition[1] > 100 || ch->pcdata->condition[1] < 0)
		   { errordetect = TRUE; ch->pcdata->condition[1] = 48; }
		   ch->pcdata->condition[2] = fread_number( fp, -999 );
		   if( ch->pcdata->condition[2] > 100 || ch->pcdata->condition[2] < 0)
		   { errordetect = TRUE; ch->pcdata->condition[2] = 48; }
		   if (errordetect)
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in Condition \n\r");
		   fMatch = TRUE;
		   break;
	    }
	    if ( !str_cmp( word, "Createtime"))
	    {
	       char *str_old;
	       int len;

	       ch->createtime = fread_string( fp );

	       /* strip off all carriage returns from the creation time */
	       len = strlen( ch->createtime);
           str_old = ch->createtime;
           strtok( str_old, "\n\r");
           ch->createtime = str_dup( str_old);
           free_mem( str_old, len + 1);

	       /* needed to use free_mem() directly because strtok() overwrites 
	          the original string with nulls, making strlen() and therefore 
	          also free_string() screw up.
	       */

	       fMatch = TRUE;
	       break;
	    }
	    break;
	    
	case 'D':
	    if( !str_cmp( word, "Damroll"))
	    {
		   ch->damroll = fread_number( fp, -999);
		   if(ch->damroll == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess, MAX_STRING_LENGTH, "Error in Damroll \n\r");
			  ch->damroll = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    
	    if( !str_cmp( word, "Deaf"))
	    {
		   ch->deaf = fread_number( fp, -999);
	               
     	           if(ch->deaf == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess, MAX_STRING_LENGTH, "Error in Deaf \n\r");
			  ch->deaf = 0;
		   }
		   if(ch->deaf < 0) ch->deaf=0;
                   fMatch = TRUE;
		   break;
	    }
	    KEYS( "Description",	ch->description,	fread_string( fp ) );
	    break;
	    
	case 'E':
	    if ( !str_cmp( word, "End" ) )
		   return;
	    if( !str_cmp( word, "Exp"))
	    {
		   ch->exp = fread_number( fp, -999);
		   if(ch->exp == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess, MAX_STRING_LENGTH, "Error in Exp \n\r");
			  ch->exp = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    if( !str_cmp( word, "Extra"))
	    {
		   ch->extra = fread_number( fp, -999);
		   if(ch->extra == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess, MAX_STRING_LENGTH, "Error in Extra \n\r");
			  ch->extra = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    
	    if( !str_cmp(word, "Email" ) ) 
	    {	
		   ch->pcdata->email = fread_string( fp ); 	
		   fMatch = TRUE;
		   break;
	    }
	    break;
	    
	case 'F':
	    if( !str_cmp( word, "Form"))
	    {
		   ch->form = fread_number( fp, -999);
		   if(ch->form == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess, MAX_STRING_LENGTH, "Error in Form \n\r");
			  ch->form = 32767;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    break;
	    
	case 'G':
	    if( !str_cmp( word, "Gold"))
	    {
		   ch->gold = fread_number( fp, -999);
		   if(ch->gold == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess, MAX_STRING_LENGTH, "Error in Gold \n\r");
			  ch->gold = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    break;
	    
	case 'H':
	    if( !str_cmp( word, "Hitroll"))
	    {
		   ch->hitroll = fread_number( fp, -999);
		   if(ch->hitroll == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess, MAX_STRING_LENGTH, "Error in Hitroll \n\r");
			  ch->hitroll = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    if( !str_cmp( word, "Home"))
	    {
		   ch->home = fread_number( fp, -999);
		   if(ch->home == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess, MAX_STRING_LENGTH, "Error in Home \n\r");
			  ch->home = 3001;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    
	    if ( !str_cmp( word, "HpManaMove" ) )
	    {
		   ch->hit		= fread_number( fp, -999 );
		   if( ch->hit == -999 ) { errordetect = TRUE; ch->hit = 20;}
		   ch->max_hit	= fread_number( fp, -999 );
		   if( ch->max_hit == -999) { errordetect = TRUE; ch->max_hit = 20;}
		   ch->mana	= fread_number( fp, -999 );
		   if( ch->mana == -999) { errordetect = TRUE; ch->mana = 100;}
		   ch->max_mana	= fread_number( fp, -999 );
		   if( ch->max_mana == -999) { errordetect = TRUE; ch->max_mana = 100;}
		   ch->move	= fread_number( fp, -999 );
		   if( ch->move == -999) { errordetect = TRUE; ch->move = 100;}
		   ch->max_move	= fread_number( fp, -999 );
		   if( ch->max_move == -999) { errordetect = TRUE; ch->max_move = 100;}
		   if(errordetect)
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in HpManaMove \n\r");
		   fMatch = TRUE;
		   break;
	    }
	    break;
	    
	case 'I':
	    if( !str_cmp( word, "Immune"))
	    {
		   ch->immune = fread_number( fp, -999);
		   if(ch->immune == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess, MAX_STRING_LENGTH, "Error in Immune \n\r");
			  ch->immune = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    
	    if( !str_cmp( word, "Itemaffect"))
	    {
		   ch->itemaffect = fread_number( fp, -999);
		   if(ch->itemaffect == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess, MAX_STRING_LENGTH, "Error in Itemaffect \n\r");
			  ch->itemaffect = 3001;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    break;
	    
	case 'L':
	    KEYS( "Lasthost",	ch->lasthost,		fread_string( fp ) );
	    KEYS( "Lasttime",	ch->lasttime,		fread_string( fp ) );
	    if( !str_cmp( word, "Level"))
	    {
		   ch->level = fread_number( fp, -999);
		   if(ch->level == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess, MAX_STRING_LENGTH, "Error in Level \n\r");
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
	    if ( !str_cmp( word, "Locationhp" ) )
	    {
		   ch->loc_hp[0]	= fread_number( fp, -999 );
		   if( ch->loc_hp[0] == -999) { errordetect = TRUE; ch->loc_hp[0] = 0; }
		   ch->loc_hp[1]	= fread_number( fp, -999 );
		   if( ch->loc_hp[1] == -999) { errordetect = TRUE; ch->loc_hp[1] = 0; }
		   ch->loc_hp[2]	= fread_number( fp, -999 );
		   if( ch->loc_hp[2] == -999) { errordetect = TRUE; ch->loc_hp[2] = 0; }
		   ch->loc_hp[3]	= fread_number( fp, -999 );
		   if( ch->loc_hp[3] == -999) { errordetect = TRUE; ch->loc_hp[3] = 0; }
		   ch->loc_hp[4]	= fread_number( fp, -999 );
		   if( ch->loc_hp[4] == -999) { errordetect = TRUE; ch->loc_hp[4] = 0; }
		   ch->loc_hp[5]	= fread_number( fp, -999 );
		   if( ch->loc_hp[5] == -999) { errordetect = TRUE; ch->loc_hp[5] = 0; }
		   ch->loc_hp[6]	= fread_number( fp, -999 );
		   if( ch->loc_hp[6] == -999) { errordetect = TRUE; ch->loc_hp[6] = 0; }
		   if (errordetect)
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in Locationhp \n\r");
		   fMatch = TRUE;
		   break;
	    }
	    KEYS( "LongDescr",	ch->long_descr,		fread_string( fp )  );
	    KEYS( "Lord",	ch->lord,		fread_string( fp )  );
            KEYS( "LagPenalty", ch->lagpenalty,         fread_number( fp, -999));
            if( ch->lagpenalty == -999) { errordetect = TRUE; ch->lagpenalty = 0; }
	    break;
	    
	case 'M':
	    KEYS( "Morph",	ch->morph,		fread_string( fp ) );
	    break;
	    
	case 'N':
	    if ( !str_cmp( word, "Name" ) )
	    {
	    /*
	    * Name already set externally.
		   */
		   fread_to_eol( fp );
		   fMatch = TRUE;
		   break;
	    }
	    
	    break;
	    
	case 'O':
	    if( !str_cmp( word, "Objvnum"))
	    {
		   ch->pcdata->obj_vnum = fread_number( fp, -999);
		   if(ch->pcdata->obj_vnum == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess,  MAX_STRING_LENGTH, "Error in Objvnum \n\r");
			  ch->pcdata->obj_vnum = 24000;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    break;
	    
	case 'P':
	    KEYS( "Password",	ch->pcdata->pwd,	fread_string( fp ) );
	    if( !str_cmp( word, "Played"))
	    {
		   ch->played = fread_number( fp, -999);
		   if(ch->played == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess,  MAX_STRING_LENGTH, "Error in Played \n\r");
			  ch->played = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    if( !str_cmp( word, "Polyaff"))
	    {
		   ch->polyaff = fread_number( fp, -999);
		   if(ch->polyaff == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess,  MAX_STRING_LENGTH, "Error in Polyaff \n\r");
			  ch->polyaff = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    KEYS( "Poweraction",	ch->poweraction,	fread_string( fp ) );
	    KEYS( "Powertype",	ch->powertype,		fread_string( fp ) );
	    if( !str_cmp( word, "Position"))
	    {
		   ch->position = fread_number( fp, -999);
		   if(ch->position == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess,  MAX_STRING_LENGTH, "Error in Position \n\r");
			  ch->position = 7;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    if( !str_cmp( word, "Practice"))
	    {
		   ch->practice = fread_number( fp, -999);
		   if(ch->practice == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess,  MAX_STRING_LENGTH, "Error in Practice \n\r");
			  ch->practice = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
		KEYS( "Prompt",      ch->prompt,             fread_string( fp ) );
	    if ( !str_cmp( word, "PkPdMkMd" ) )
	    {
		   ch->pkill	= fread_number( fp, -999 );
		   if( ch->pkill == -999) { errordetect = TRUE; ch->pkill = 0; }
		   ch->pdeath	= fread_number( fp, -999 );
		   if( ch->pdeath == -999) { errordetect = TRUE; ch->pdeath = 0; }
		   ch->mkill	= fread_number( fp, -999 );
		   if( ch->mkill == -999) { errordetect = TRUE; ch->mkill = 0; }
		   ch->mdeath	= fread_number( fp, -999 );
		   if( ch->mdeath == -999) { errordetect = TRUE; ch->mdeath = 0; }
		   if (errordetect)
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in PkPdMkMd \n\r");
		   fMatch = TRUE;
		   break;
	    }
	    break;
	    
	case 'Q':
	    if( !str_cmp( word, "Quest"))
	    {
		   ch->pcdata->quest = fread_number( fp, -999);
		   if(ch->pcdata->quest == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess,  MAX_STRING_LENGTH, "Error in Quest \n\r");
			  ch->pcdata->quest = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    break;
	    
	case 'R':
	    if( !str_cmp( word, "Race"))
	    {
		   ch->race = fread_number( fp, -999);
		   if(ch->race == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess,  MAX_STRING_LENGTH, "Error in Race \n\r");
			  ch->race = 0;
		   }
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
	    
	    if ( !str_cmp( word, "Room" ) )
	    {
	    /*	int jkrtemp=0;
	    jkrtemp = fread_number( fp, -999);
		   if( jkrtemp == -999) {errordetect = TRUE; jkrtemp = 3001;} */
		   ch->in_room = get_room_index( fread_number( fp, -999 ) );
		   if ( ch->in_room == NULL )
			  ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
		   fMatch = TRUE;
		   break;
	    }
	    
	    break;
	    
	case 'S':
	    if( !str_cmp( word, "SavingThrow"))
	    {
		   ch->saving_throw = fread_number( fp, -999);
		   if(ch->saving_throw == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess,  MAX_STRING_LENGTH, "Error in SavingThrow \n\r");
			  ch->saving_throw = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    if( !str_cmp( word, "Sex"))
	    {
		   ch->sex = fread_number( fp, -999);
		   if(ch->sex > 2 || ch->sex < 0)
		   {
			  errordetect = TRUE;
			  snprintf( errormess,  MAX_STRING_LENGTH, "Error in Sex \n\r");
			  ch->sex = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    KEYS( "ShortDescr",	ch->short_descr,	fread_string( fp ) );
	    
	    if ( !str_cmp( word, "Skill" ) )
	    {
		   int sn;
		   int value;
		   
		   value = fread_number( fp, -999 );
		   if( value == -999) { errordetect = TRUE; value = 0; }
		   sn    = skill_lookup( fread_word( fp ) );
		   /*
		   if ( sn < 0 )
		   bug( "Fread_char: unknown skill.", 0 );
		   else
		   ch->pcdata->learned[sn] = value;
		   */
		   if ( sn >= 0 )
			  ch->pcdata->learned[sn] = value;
		   fMatch = TRUE;
	    }
	    
	    if( !str_cmp( word, "Specpower"))
	    {
		   ch->specpower = fread_number( fp, -999);
		   if(ch->specpower == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess,  MAX_STRING_LENGTH, "Error in Specpower \n\r");
			  ch->specpower = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    if( !str_cmp( word, "Spectype"))
	    {
		   ch->spectype = fread_number( fp, -999);
		   if(ch->spectype == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess,  MAX_STRING_LENGTH, "Error in Spectype \n\r");
			  ch->spectype = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    
	    if ( !str_cmp( word, "Spells" ) )
	    {
		   ch->spl[0]	= fread_number( fp, -999 );
		   if( ch->spl[0] == -999) {errordetect = TRUE; ch->spl[0] = 0;}
		   ch->spl[1]	= fread_number( fp, -999 );
		   if( ch->spl[1] == -999) {errordetect = TRUE; ch->spl[1] = 0;}
		   ch->spl[2]	= fread_number( fp, -999 );
		   if( ch->spl[2] == -999) {errordetect = TRUE; ch->spl[2] = 0;}
		   ch->spl[3]	= fread_number( fp, -999 );
		   if( ch->spl[3] == -999) {errordetect = TRUE; ch->spl[3] = 0;}
		   ch->spl[4]	= fread_number( fp, -999 );
		   if( ch->spl[4] == -999) {errordetect = TRUE; ch->spl[4] = 0;}
		   if (errordetect)
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in Spells \n\r");
		   fMatch = TRUE;
		   break;
	    }
	    
	    if ( !str_cmp( word, "Stance" ) )
	    {
		   ch->stance[0]	= fread_number( fp, -999 );
		   if(ch->stance[0] == -999) errordetect = TRUE;
		   if(ch->stance[0] > 200 || ch->stance[0] < -1) 
		   { ch->stance[0] = 200; }
		   ch->stance[1]	= fread_number( fp, -999 );
		   if(ch->stance[1] == -999) errordetect = TRUE;
		   if(ch->stance[1] > 200 || ch->stance[1] < 0) 
		   { ch->stance[1] = 200; }
		   ch->stance[2]	= fread_number( fp, -999 );
		   if(ch->stance[2] == -999) errordetect = TRUE;
		   if(ch->stance[2] > 200 || ch->stance[2] < 0) 
		   { ch->stance[2] = 200; }
		   ch->stance[3]	= fread_number( fp, -999 );
		   if(ch->stance[3] == -999) errordetect = TRUE;
		   if(ch->stance[3] > 200 || ch->stance[3] < 0) 
		   { ch->stance[3] = 200; }
		   ch->stance[4]	= fread_number( fp, -999 );
		   if(ch->stance[4] == -999) errordetect = TRUE;
		   if(ch->stance[4] > 200 || ch->stance[4] < 0) 
		   { ch->stance[4] = 200; }
		   ch->stance[5]	= fread_number( fp, -999 );
		   if(ch->stance[5] == -999) errordetect = TRUE;
		   if(ch->stance[5] > 200 || ch->stance[5] < 0) 
		   { ch->stance[5] = 200; }
		   ch->stance[6]	= fread_number( fp, -999 );
		   if(ch->stance[6] == -999) errordetect = TRUE;
		   if(ch->stance[6] > 200 || ch->stance[6] < 0) 
		   { ch->stance[6] = 200; }
		   ch->stance[7]	= fread_number( fp, -999 );
		   if(ch->stance[7] == -999) errordetect = TRUE;
		   if(ch->stance[7] > 200 || ch->stance[7] < 0) 
		   { ch->stance[7] = 200; }
		   ch->stance[8]	= fread_number( fp, -999 );
		   if(ch->stance[8] == -999) errordetect = TRUE;
		   if(ch->stance[8] > 200 || ch->stance[8] < 0) 
		   { ch->stance[8] = 200; }
		   ch->stance[9]	= fread_number( fp, -999 );
		   if(ch->stance[9] == -999) errordetect = TRUE;
		   if(ch->stance[9] > 200 || ch->stance[9] < 0) 
		   { ch->stance[9] = 200; }
		   ch->stance[10]	= fread_number( fp, -999 );
		   if(ch->stance[10] == -999) errordetect = TRUE;
		   if(ch->stance[10] > 200 || ch->stance[10] < 0) 
		   { ch->stance[10] = 200; }
            ch->stance[11]    = fread_number( fp, -999 );
           if(ch->stance[11] == -999) errordetect = TRUE;
           if(ch->stance[11] > 11 || ch->stance[11] < 0)
            { ch->stance[11] = 0; }
		   if (errordetect)
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in Stance \n\r");
		   fMatch = TRUE;
		   break;
	    }
	    
	    break;
	    
	case 'T':
	    if( !str_cmp( word, "Trust"))
	    {
		   ch->trust = fread_number( fp, -999);
/*		   if( ch->trust == 9 &&
			  ( str_cmp( ch->name, "Palmer") &&
			  str_cmp( ch->name, "Archon") &&
			  str_cmp( ch->name, "Palmer") &&
			  str_cmp( ch->name, "Dirge") ) )
		   { 
			  ch->trust = 0;
		   }*/
		   if(ch->trust == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess,  MAX_STRING_LENGTH, "Error in Trust \n\r");
			  ch->trust = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    
	    if ( !str_cmp( word, "Title" ) )
	    {
		   ch->pcdata->title = fread_string( fp );
		   if ( isalpha(ch->pcdata->title[0])
			  ||   isdigit(ch->pcdata->title[0]) )
		   {
			  snprintf( buf,  MAX_STRING_LENGTH, " %s", ch->pcdata->title );
			  free_string( ch->pcdata->title );
			  ch->pcdata->title = str_dup( buf );
		   }
		   fMatch = TRUE;
		   break;
	    }
	    
	    break;
	    
	case 'V':
	    if( !str_cmp( word, "Vampaff"))
	    {
		   ch->vampaff = fread_number( fp, -999);
		   if(ch->vampaff == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess,  MAX_STRING_LENGTH, "Error in Vampaff \n\r");
			  ch->vampaff = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    if( !str_cmp( word, "Vampgen"))
	    {
		   ch->vampgen = fread_number( fp, -999);
		   if(ch->vampgen == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess,  MAX_STRING_LENGTH, "Error in Vampgen \n\r");
			  ch->vampgen = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    if( !str_cmp( word, "Vamppass"))
	    {
		   ch->vamppass = fread_number( fp, -999);
		   if(ch->vamppass == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess,  MAX_STRING_LENGTH, "Error in Vamppass \n\r");
			  ch->vamppass = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    if ( !str_cmp( word, "Vnum" ) )
	    {
		   int jkrtemp=0;
		   jkrtemp = fread_number( fp, -999);
		   if( jkrtemp == -999) {errordetect = TRUE; jkrtemp = 24000; snprintf( errormess,  MAX_STRING_LENGTH, "Error in Vnum \n\r");}
		   ch->pIndexData = get_mob_index( jkrtemp );
		   fMatch = TRUE;
		   break;
	    }
	    break;
	    
	case 'W':
	    if ( !str_cmp( word, "Weapons" ) )
	    {
		   ch->wpn[0]	= fread_number( fp, -999 );
		   if( ch->wpn[0] < 0 || ch->wpn[0] > 200 ) {errordetect=TRUE;ch->wpn[0]=0;}
		   ch->wpn[1]	= fread_number( fp, -999 );
		   if( ch->wpn[1] < 0 || ch->wpn[1] > 200 ) {errordetect=TRUE;ch->wpn[1]=0;}
		   ch->wpn[2]	= fread_number( fp, -999 );
		   if( ch->wpn[2] < 0 || ch->wpn[2] > 200 ) {errordetect=TRUE;ch->wpn[2]=0;}
		   ch->wpn[3]	= fread_number( fp, -999 );
		   if( ch->wpn[3] < 0 || ch->wpn[3] > 200 ) {errordetect=TRUE;ch->wpn[3]=0;}
		   ch->wpn[4]	= fread_number( fp, -999 );
		   if( ch->wpn[4] < 0 || ch->wpn[4] > 200 ) {errordetect=TRUE;ch->wpn[4]=0;}
		   ch->wpn[5]	= fread_number( fp, -999 );
		   if( ch->wpn[5] < 0 || ch->wpn[5] > 200 ) {errordetect=TRUE;ch->wpn[5]=0;}
		   ch->wpn[6]	= fread_number( fp, -999 );
		   if( ch->wpn[6] < 0 || ch->wpn[6] > 200 ) {errordetect=TRUE;ch->wpn[6]=0;}
		   ch->wpn[7]	= fread_number( fp, -999 );
		   if( ch->wpn[7] < 0 || ch->wpn[7] > 200 ) {errordetect=TRUE;ch->wpn[7]=0;}
		   ch->wpn[8]	= fread_number( fp, -999 );
		   if( ch->wpn[8] < 0 || ch->wpn[8] > 200 ) {errordetect=TRUE;ch->wpn[8]=0;}
		   ch->wpn[9]	= fread_number( fp, -999 );
		   if( ch->wpn[9] < 0 || ch->wpn[9] > 200 ) {errordetect=TRUE;ch->wpn[9]=0;}
		   ch->wpn[10]	= fread_number( fp, -999 );
		   if( ch->wpn[10] < 0 || ch->wpn[10] > 200 ) {errordetect=TRUE;ch->wpn[10]=0;}
		   ch->wpn[11]	= fread_number( fp, -999 );
		   if( ch->wpn[11] < 0 || ch->wpn[11] > 200 ) {errordetect=TRUE;ch->wpn[11]=0;}
		   ch->wpn[12]	= fread_number( fp, -999 );
		   if( ch->wpn[12] < 0 || ch->wpn[12] > 200 ) {errordetect=TRUE;ch->wpn[12]=0;}
			if (errordetect)
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in Weapon\n\r");
		   fMatch = TRUE;
		   break;
	    }
	    if( !str_cmp( word, "Wimpy"))
	    {
		   ch->wimpy = fread_number( fp, -999);
		   if(ch->wimpy == -999)
		   {
			  errordetect = TRUE;
			  snprintf( errormess,  MAX_STRING_LENGTH, "Error in Wimpy \n\r");
			  ch->wimpy = 0;
		   }
		   fMatch = TRUE;
		   break;
	    }
	    break;
	}
	
	if( errordetect == TRUE )
	{
	    
	    char palmer[MAX_INPUT_LENGTH];
	    send_to_char( "ERROR DETECTED! \n\r Your pfile is buggered please contact a CODER and do NOT log this char in again untill told to do so.\n\r", ch); 	
	    bug( "ERROR DETECTED! Shagged pfile!! during fread_char load.", 0 );
	    snprintf( palmer,  MAX_INPUT_LENGTH, "%s has a shagged pfile(ERROR DETECTED!), please inform a CODER!\n\r", ch->name);
	    do_info(ch, palmer);
	    bug( palmer , 0);
	    bug( errormess, 0);
	    do_info(ch, errormess);
	    close_socket( ch->desc ); 
	    errordetect = FALSE;
	    /*    	    strcpy( errormess, "");	*/
	    return;
	}	
	
	if ( !fMatch )
	{
		bug(word, 0);
	    bug( "Fread_char: no match (!fMatch)Char field error.", 0 );
	    if( !strcmp( word, "SHAGGED"))
	    {
		   char palmer[MAX_INPUT_LENGTH];
		   send_to_char( "Your pfile is buggered please contact a CODER and do NOT use this char again until told to do so. \n\r",ch); 	
		   bug( "Shagged pfile!! during fread_char load.", 0 );
		   snprintf( palmer,  MAX_INPUT_LENGTH, "%s has a shagged pfile, please inform a CODER!\n\r", ch->name);
		   do_info(ch, palmer);
		   bug( palmer , 0);
		   bug( errormess , 0);
		   close_socket( ch->desc );
		   return;
	    }
	    else 
		   fread_to_eol( fp );	
	}
    }
}

void fread_obj( CHAR_DATA *ch, FILE *fp )
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
    
    if ( obj_free == NULL )
    {
	   obj		= alloc_perm( sizeof(*obj) );
    }
    else
    {
	   obj		= obj_free;
	   obj_free	= obj_free->next;
    }
    
    *obj		= obj_zero;
    obj->name		= str_dup( "" );
    obj->short_descr	= str_dup( "" );
    obj->description	= str_dup( "" );
    obj->chpoweron	= str_dup( "(null)" );
    obj->chpoweroff	= str_dup( "(null)" );
    obj->chpoweruse	= str_dup( "(null)" );
    obj->victpoweron	= str_dup( "(null)" );
    obj->victpoweroff	= str_dup( "(null)" );
    obj->victpoweruse	= str_dup( "(null)" );
    obj->questmaker	= str_dup( "" );
    obj->questowner	= str_dup( "" );
    obj->spectype	= 0;
    obj->specpower	= 0;
    obj->condition	= 100;
    obj->toughness	= 0;
    obj->resistance	= 100;
    obj->quest		= 0;
    obj->points		= 0;
    
    fNest		= FALSE;
    fVnum		= TRUE;
    iNest		= 0;
    
    for ( ; ; )
    {
	   word   = feof( fp ) ? "End" : fread_word( fp );
	   fMatch = FALSE;
	   
	   switch ( UPPER(word[0]) )
	   {
	   case '*':
		  fMatch = TRUE;
		  fread_to_eol( fp );
		  break;
		  
	   case 'A':
		  if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
		  {
			 AFFECT_DATA *paf;
			 
			 if ( affect_free == NULL )
			 {
				paf		= alloc_perm( sizeof(*paf) );
			 }
			 else
			 {
				paf		= affect_free;
				affect_free	= affect_free->next;
			 }
			 
			 paf->duration	= fread_number( fp, -999 );
			 paf->modifier	= fread_number( fp, -999 );
			 paf->location	= fread_number( fp, -999 );
			 paf->next	= obj->affected;
			 obj->affected	= paf;
			 fMatch		= TRUE;
			 break;
		  }
		  break;
		  
	   case 'C':
		  if( !str_cmp( word, "Condition"))
		  {
			 obj->condition = fread_number( fp, -999);
			 if(obj->condition == -999)
			 {
				errordetect = TRUE;
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in Condition \n\r");
				obj->condition = 100;
			 }
			 fMatch = TRUE;
			 break;
		  }
		  
		  if( !str_cmp( word, "Cost"))
		  {
			 obj->cost = fread_number( fp, -999);
			 if(obj->cost == -999)
			 {
				errordetect = TRUE;
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in Cost \n\r");
				obj->cost = 0;
			 }
			 fMatch = TRUE;
			 break;
		  }
		  break;
		  
	   case 'D':
		  KEYS( "Description",	obj->description,	fread_string( fp ) );
		  break;
		  
	   case 'E':
		  if( !str_cmp( word, "ExtraFlags"))
		  {
			 obj->extra_flags = fread_number( fp, -999);
			 if(obj->extra_flags == -999)
			 {
				errordetect = TRUE;
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in ExtraFlags \n\r");
				obj->extra_flags = 0;
			 }
			 fMatch = TRUE;
			 break;
		  }
		  
		  if ( !str_cmp( word, "ExtraDescr" ) )
		  {
			 EXTRA_DESCR_DATA *ed;
			 
			 if ( extra_descr_free == NULL )
			 {
				ed			= alloc_perm( sizeof(*ed) );
			 }
			 else
			 {
				ed			= extra_descr_free;
				extra_descr_free	= extra_descr_free->next;
			 }
			 
			 ed->keyword		= fread_string( fp );
			 ed->description		= fread_string( fp );
			 ed->next		= obj->extra_descr;
			 obj->extra_descr	= ed;
			 fMatch = TRUE;
		  }
		  
		  if ( !str_cmp( word, "End" ) )
		  {
			 if ( !fNest || !fVnum )
			 {
				bug( "Fread_obj: incomplete object.", 0 );
				free_string( obj->name        );
				free_string( obj->description );
				free_string( obj->short_descr );
				obj->next = obj_free;
				obj_free  = obj;
				return;
			 }
			 else
			 {
				obj->next	= object_list;
				object_list	= obj;
				obj->pIndexData->count++;
				if ( iNest == 0 || rgObjNest[iNest] == NULL )
				    obj_to_char( obj, ch );
				else
				    obj_to_obj( obj, rgObjNest[iNest-1] );
				return;
			 }
		  }
		  break;
		  
	   case 'I':
		  if( !str_cmp( word, "ItemType"))
		  {
			 obj->item_type = fread_number( fp, -999);
			 if(obj->item_type == -999)
			 {
				errordetect = TRUE;
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in ItemType \n\r");
				obj->item_type = 0;
			 }
			 fMatch = TRUE;
			 break;
		  }
		  break;
		  
	   case 'L':
		  if( !str_cmp( word, "Level"))
		  {
			 obj->level = fread_number( fp, -999);
			 if(obj->level == -999)
			 {
				errordetect = TRUE;
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in Level \n\r");
				obj->level = 0;
			 }
			 fMatch = TRUE;
			 break;
		  }
		  break;
		  
	   case 'N':
		  KEYS( "Name",	obj->name,		fread_string( fp ) );
		  
		  if ( !str_cmp( word, "Nest" ) )
		  {
			 iNest = fread_number( fp, -999 );
			 if ( iNest < 0 || iNest >= MAX_NEST )
			 {
				bug( "Fread_obj: bad nest %d.", iNest );
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
		  if( !str_cmp( word, "Points"))
		  {
			 obj->points = fread_number( fp, -999);
			 if(obj->points == -999)
			 {
				errordetect = TRUE;
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in Points \n\r");
				obj->points = 0;
			 }
			 fMatch = TRUE;
			 break;
		  }
		  KEYS( "Poweronch",	obj->chpoweron,		fread_string( fp ) );
		  KEYS( "Poweroffch",	obj->chpoweroff,	fread_string( fp ) );
		  KEYS( "Powerusech",	obj->chpoweruse,	fread_string( fp ) );
		  KEYS( "Poweronvict",	obj->victpoweron,	fread_string( fp ) );
		  KEYS( "Poweroffvict",obj->victpoweroff,	fread_string( fp ) );
		  KEYS( "Powerusevict",obj->victpoweruse,	fread_string( fp ) );
		  break;
		  
	   case 'Q':
		  if( !str_cmp( word, "Quest"))
		  {
			 obj->quest = fread_number( fp, -999);
			 if(obj->quest == -999)
			 {
				errordetect = TRUE;
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in Quest \n\r");
				obj->quest = 0;
			 }
			 fMatch = TRUE;
			 break;
		  }
		  KEYS( "Questmaker",	obj->questmaker,	fread_string( fp ) );
		  KEYS( "Questowner",	obj->questowner,	fread_string( fp ) );
		  break;
		  
	   case 'R':
		  if( !str_cmp( word, "Resistance"))
		  {
			 obj->resistance = fread_number( fp, -999);
			 if(obj->resistance == -999)
			 {
				errordetect = TRUE;
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in Resistance \n\r");
				obj->resistance = 0;
			 }
			 fMatch = TRUE;
			 break;
		  }
		  break;
		  
	   case 'S':
		  KEYS( "ShortDescr",	obj->short_descr,	fread_string( fp ) );
		  if( !str_cmp( word, "Spectype"))
		  {
			 obj->spectype = fread_number( fp, -999);
			 if(obj->spectype == -999)
			 {
				errordetect = TRUE;
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in Spectype \n\r");
				obj->spectype = 0;
			 }
			 fMatch = TRUE;
			 break;
		  }
		  
		  if( !str_cmp( word, "Specpower"))
		  {
			 obj->specpower = fread_number( fp, -999);
			 if(obj->specpower == -999)
			 {
				errordetect = TRUE;
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in Specpower \n\r");
				obj->specpower = 0;
			 }
			 fMatch = TRUE;
			 break;
		  }
		  
		  if ( !str_cmp( word, "Spell" ) )
		  {
			 int iValue;
			 int sn;
			 
			 iValue = fread_number( fp, -999 );
			 sn     = skill_lookup( fread_word( fp ) );
			 if ( iValue < 0 || iValue > 3 )
			 {
				bug( "Fread_obj: bad iValue %d.", iValue );
			 }
			 else if ( sn < 0 )
			 {
				bug( "Fread_obj: unknown skill.", 0 );
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
		  if( !str_cmp( word, "Timer"))
		  {
			 obj->timer = fread_number( fp, -999);
			 if(obj->timer == -999)
			 {
				errordetect = TRUE;
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in Timer \n\r");
				obj->timer = 0;
			 }
			 fMatch = TRUE;
			 break;
		  }
		  
		  if( !str_cmp( word, "Toughness"))
		  {
			 obj->toughness = fread_number( fp, -999);
			 if(obj->toughness == -999)
			 {
				errordetect = TRUE;
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in Toughness \n\r");
				obj->toughness = 0;
			 }
			 fMatch = TRUE;
			 break;
		  }	
		  break;
		  
	   case 'V':
		  if ( !str_cmp( word, "Values" ) )
		  {
			 obj->value[0] = 0;
			 obj->value[1] = 0;
			 obj->value[2] = 0;
			 obj->value[3] = 0;
			 obj->value[0]	= fread_number( fp, -999 );
			 obj->value[1]	= fread_number( fp, -999 );
			 obj->value[2]	= fread_number( fp, -999 );
			 obj->value[3]	= fread_number( fp, -999 );
			 fMatch		= TRUE;
			 break;
		  }
		  
		  if ( !str_cmp( word, "Vnum" ) )
		  {
			 int vnum;
			 
			 vnum = fread_number( fp, -999 );
			 if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
				bug( "Fread_obj: bad vnum %d.", vnum );
			 else
				fVnum = TRUE;
			 fMatch = TRUE;
			 break;
		  }
		  break;
		  
	   case 'W':
		  if( !str_cmp( word, "WearFlags"))
		  {
			 obj->wear_flags = fread_number( fp, -999);
			 if(obj->wear_flags == -999)
			 {
				errordetect = TRUE;
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in WearFlags \n\r");
				obj->wear_flags = 0;
			 }
			 fMatch = TRUE;
			 break;
		  }
		  
		  if( !str_cmp( word, "WearLoc"))
		  {
			 obj->wear_loc = fread_number( fp, -999);
			 if(obj->wear_loc == -999)
			 {
				errordetect = TRUE;
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in WearLoc \n\r");
				obj->wear_loc = 0;
			 }
			 fMatch = TRUE;
			 break;
		  }
		  
		  if( !str_cmp( word, "Weight"))
		  {
			 obj->weight = fread_number( fp, -999);
			 if(obj->weight == -999)
			 {
				errordetect = TRUE;
				snprintf( errormess,  MAX_STRING_LENGTH, "Error in Weight \n\r");
				obj->weight = 0;
			 }
			 fMatch = TRUE;
			 break;
		  }	
		  break;
		  
	}
	
	if( errordetect == TRUE )
	{
	    
	    char palmer[MAX_INPUT_LENGTH];
	    send_to_char( "ERROR DETECTED! Your pfile is buggered please contact a CODER and do NOT use this char again until told to do so.\n\r",ch); 	
	    bug( "ERROR DETECTED! Shagged pfile!! during fread_char load.", 0 );
	    bug( errormess, 0);
	    snprintf( palmer,  MAX_INPUT_LENGTH, "%s has a shagged pfile(ERROR DETECTED!), please inform a CODER!\n\r", ch->name);
	    do_info(ch, palmer);
	    do_info(ch, errormess);
	    bug( palmer , 0);
	    bug( errormess , 0);
	    close_socket( ch->desc );
	    errordetect = FALSE;
	    return;
	}	
	
	if ( !fMatch )
	{
	    
	    char palmer[MAX_INPUT_LENGTH];
	    
	    bug( "Fread_obj: no match.error 2 (valis)", 0 );
	    bug (palmer, 0);
	    if( !strcmp(word, "SHAGGED")) 
	    {
		   char palmer[MAX_INPUT_LENGTH];
		   send_to_char( "Your pfile is buggered please contact a CODER and do NOT use this char again until told to do so\n\r",ch); 	
		   bug( "Shagged pfile!! during fread_obj load.", 0 );
		   snprintf( palmer,  MAX_INPUT_LENGTH, "%s has a shagged pfile, please inform a CODER!\n\r", ch->name);
		   do_info(ch, palmer);
		   bug( palmer , 0);
		   bug( errormess , 0);
		   close_socket( ch->desc );
		   return;
	    }	
	    else	
		   fread_to_eol(fp);
	}
    }
}
BAN_DATA *     ban_list;

void save_siteban_info( void )
{
    BAN_DATA *pban;
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    char strsave[MAX_INPUT_LENGTH];
    int num;
    
    num = 0;
    fclose(fpReserve);
    strncpy(strsave, SITEBAN_FILE,strlen(SITEBAN_FILE));
    
    if ( !( fp=fopen( strsave,"w")))
    {
	   snprintf(buf,  MAX_STRING_LENGTH, "Save_siteban:fopen %s:", strsave);
	   bug( buf, 0 );
	   perror(strsave);
	   return;
    }
    else
    {
	   for ( pban = ban_list; pban != NULL; pban = pban->next )
	   {
		  num++;
	   }
	   fprintf(fp, "%d\n", num);
	   
	   for ( pban = ban_list; pban != NULL; pban = pban->next )
	   {
		  fprintf(fp, "%s\n", pban->name);
	   }
	   
	   fprintf(fp, "#END");
	   fclose(fp);
	   fpReserve = fopen(NULL_FILE, "r");
	   return;
    }
}

void read_siteban_info( void )
{
    BAN_DATA *pban;
    FILE *fp;
    int j,num;
    char strsave[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    
    fclose(fpReserve);
    strncpy(strsave, SITEBAN_FILE, MAX_INPUT_LENGTH);
    
    if ( !( fp=fopen( strsave,"r")))
    {
	   snprintf(buf,  MAX_STRING_LENGTH, "Save_siteban:fopen %s:", strsave);
	   bug( buf, 0 );
	   perror(strsave);
	   return;
    }
    
    num = fread_number(fp, -999);
    
    for(j=1; j<=num; j++)
    {
	   if ( ban_free == NULL )
	   {
		  pban     = alloc_perm( sizeof(*pban) );
	   }
	   else
	   {
		  pban     = ban_free;
		  ban_free = ban_free->next;
	   }
	   
	   pban->name = str_dup( fread_word( fp ) );
	   pban->next = ban_list;
	   ban_list   = pban;
    }
    
    
}

void save_claninfo( void )
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    char strsave[MAX_INPUT_LENGTH];
    
    fclose(fpReserve);
    strncpy(strsave, CLAN_FILE, MAX_INPUT_LENGTH);
    if( !( fp=fopen( strsave, "w")))
    {
	   snprintf(buf,  MAX_STRING_LENGTH, "Save_claninfo: fopen %s: ", strsave);
	   bug( buf, 0);
	   perror(strsave);
	   return;
    }
    else
    {
	   int temp;
	   int temp2;
	   fprintf(fp, "%d\n", MAX_CLAN);
	   for(temp=1;temp<MAX_CLAN;temp++)
	   {
		  fprintf(fp,"%ld\n", clan_infotable[temp].members);
		  for(temp2=1;temp2<MAX_CLAN;temp2++)
			 fprintf(fp,"%d\n",clan_infotable[temp].pkills[temp2]);
		  
		  for(temp2=1;temp2<MAX_CLAN;temp2++)
			 fprintf(fp,"%d\n",clan_infotable[temp].pkilled[temp2]);
		  
		  fprintf(fp, "%ld\n", clan_infotable[temp].mkills);
		  fprintf(fp, "%d\n", clan_infotable[temp].mkilled);
	   }
    }
    fclose(fp);
    fpReserve = fopen(NULL_FILE, "r");
    return;
}

void read_claninfo( void )
{
    int iClan=0;
    int temp2;
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    char strsave[MAX_INPUT_LENGTH];
    int num = MAX_CLAN-1;
    
    fclose(fpReserve);
    strncpy( strsave, CLAN_FILE, MAX_INPUT_LENGTH);
    if(!(fp = fopen(strsave, "r")))
    {
	   snprintf(buf,  MAX_STRING_LENGTH, "Read_claninfo: fopen %s: ", strsave);
	   bug(buf, 0);
	   perror(strsave);
	   
	   for(iClan=1; iClan<num ; iClan++)
	   {
		  clan_infotable[iClan].members=0; 
		  clan_infotable[iClan].mkills=0;
		  clan_infotable[iClan].mkilled=0;
		  for(temp2=1; temp2<num; temp2++)
		  {
			 (clan_infotable[iClan].pkills[temp2]=0);
			 (clan_infotable[iClan].pkilled[temp2]=0);
		  }
	   }
	   save_claninfo();
	   return;
    }
    else
	   num = fread_number(fp, -999);
    for(iClan=1; iClan<num; iClan++)
    {
	   clan_infotable[iClan].members = fread_number(fp, -999);
	   for(temp2=1; temp2<num; temp2++)
		  clan_infotable[iClan].pkills[temp2] = fread_number(fp, -999);
	   for(temp2=1; temp2<num; temp2++)
		  clan_infotable[iClan].pkilled[temp2] = fread_number(fp, -999);
	   clan_infotable[iClan].mkills = fread_number(fp, -999);
	   clan_infotable[iClan].mkilled = fread_number(fp, -999);
    }
}

void do_updateleague( CHAR_DATA *ch, char *argument)
{
    FILE *fp;
    char buf[MAX_STRING_LENGTH];
    char strsave[MAX_INPUT_LENGTH];
    
    int n=0;
    int m=0; 
    char *tester=0;
    int maxplayers=0;
    
    fclose(fpReserve);
    strncpy( strsave, LEAGUE_FILE, MAX_INPUT_LENGTH);
    if(!(fp = fopen(strsave, "r")))
    {
	   snprintf(buf,  MAX_STRING_LENGTH, "Read_leagueinfo: fopen %s: ", strsave);
	   bug(buf, 0);
	   perror(strsave);
	   return;
    }
    else
    {  
	   for( n=1; !(EOF==(atoi(tester))) ; n++)
	   {
		  ungetc((atoi(tester)),fp);	
		  league_infotable[n].name = fread_jword(fp);
		  fread_word(fp);
		  league_infotable[n].Pk   = fread_number(fp, -999);
		  league_infotable[n].Pd   = fread_number(fp, -999);
		  league_infotable[n].Mk   = fread_number(fp, -999);
		  league_infotable[n].Md   = fread_number(fp, -999);
		  fread_to_eol(fp);
		  tester = (void*)(intptr_t)getc(fp);
		  snprintf(buf,  MAX_STRING_LENGTH, "name %s \n\r", league_infotable[n].name);
		  send_to_char(buf,ch); 
	   }
	   maxplayers = n;		
    }
    
    
    for (m=1; m<maxplayers; m++)
    {
           snprintf( buf,  MAX_STRING_LENGTH, "%d => %s Pk:%d Pd:%d Mk:%ld Md:%d\n\r", m,
		  league_infotable[m].name,league_infotable[m].Pk,
		  league_infotable[m].Pd,league_infotable[m].Mk,
		  league_infotable[m].Md);
	   send_to_char(buf,ch);
    }
    
    
}

