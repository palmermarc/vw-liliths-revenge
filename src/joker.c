
/**************************************************
*                                                 *
*   Some new functions for some pkilling fun!     *
*   Added by the Palmer with help                  *
*   & ideas from Malice,Kavir & Rotain            *
*                                                 *
***************************************************/
#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"


extern void	fwrite_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
extern void	fwrite_obj	args( ( CHAR_DATA *ch,  OBJ_DATA  *obj,
						FILE *fp, int iNest ) );

void do_trap(CHAR_DATA *ch, char *argument)
{
    /*	OBJ_DATA *trap; */
    OBJ_DATA *used;
    int      dammax;   
    int      dam;
    OBJ_DATA *bomb;
    /*	char buf[MAX_STRING_LENGTH];    */
    
    dammax = 0;
    dam = 0;
    
    bomb = get_eq_char(ch, WEAR_WIELD);
    
    if(ch->level < 2)
    {
	   send_to_char("Huh?\n\r",ch);
	   return;
    }
    
    if(ch->race == 0)
    {
	   send_to_char("You lack the ability to set a trap.\n\r",ch);
	   return;
    }
    
    
    if( bomb==NULL )
    {
	   send_to_char("You hold nothing in your right hand.\n\r",ch);
	   return;
    }   
    
    if( bomb->item_type != ITEM_BOMB) 
    {
	   send_to_char("You can only set a trap with a bomb!\n\r",ch);
	   return;
    }
    
    if( IS_SET(ch->in_room->room_flags, ROOM_SAFE))
    {
	   send_to_char("You cannot set a trap in a safe room.\n\r",ch);
	   return;
    }
    
    dammax = bomb->value[3];
    
    for(used=ch->carrying;used;used=used->next_content)
    {
	   if(used->item_type==ITEM_BOMB && used->value[3]==dammax
		  && IS_SET(used->wear_loc, WEAR_HOLD)) break;
    }
    
    if( !used )
    {
	   send_to_char("You lack the essential ingredients for the trap.\n\r", ch);
	   return;
    }
    
    obj_from_char(used);
    
    dam = (((ch->damroll + ch->hitroll)/2)*ch->race);
    
    if(dam >= dammax)
    {	
	   ch->in_room->bomb = ch->in_room->bomb + dammax;
    }
    else
    {
	   ch->in_room->bomb = ch->in_room->bomb + dam;
    }
    if(ch->in_room->bomb > 10000) ch->in_room->bomb = 10000;
    extract_obj(used);
    return;
    
    
}

void do_email(CHAR_DATA *ch, char *argument)
{
    if( !IS_NPC( ch ) )
    {
	   if( longstring( ch, argument) )
		  return;
	   
	   smash_tilde( argument );
	   free_string( ch->pcdata->email);
	   ch->pcdata->email = str_dup(argument);
	   send_to_char("Ok.\n\r",ch);
    }
    return;
}

bool longstring(CHAR_DATA *ch, char *argument)
{
    if( strlen(argument) > 60)
    {
	   send_to_char("No more than 60 characters please.\n\r",ch);
	   return TRUE;
    }
    else 
	   return FALSE;
}

void do_finger(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA       *victim;
    char		buf[MAX_INPUT_LENGTH];
    char		buf1[MAX_INPUT_LENGTH];
    bool            fOld=FALSE;
    bool            fPlay=FALSE;
    char*           status;
    static char *    const he_she [ ] = { "It", "He", "She" };
    
    strncpy(buf1, "", MAX_INPUT_LENGTH);
    if( argument[0] == '\0')
    {
	   send_to_char("Finger who?\n\r",ch);
	   return;
    }
    
    argument[0] = UPPER( argument[0] );
    
    if( !check_parse_name(argument) )
    {
	   send_to_char("Illegal name.\n\r",ch);
	   return;
    }
    
    if( !( victim = get_char_world(ch, argument) ))
    {
    /*		fOld = load_char_obj( &d, argument);
	   if (fOld) victim = d.character; */
    }
    else
    {
	   fPlay=TRUE;
	   fOld=TRUE;
    }
    
    if(!fOld)
    {
	   send_to_char("That person is not on Vampire Wars at the moment!\n\r",ch);
	   return;
    }
    
    if(IS_NPC(victim))
    {
	   send_to_char("That is a mob!\n\r", ch);
	   return;
    }
    
    send_to_char_formatted("----------------------------------------------------\n\r", ch);	
    snprintf(buf,  MAX_INPUT_LENGTH, "%s%s.\n\r" , victim->name, victim->pcdata->title);
    send_to_char_formatted(buf, ch);
    send_to_char_formatted(victim->description, ch);
    if( victim->sex < 0 || victim->sex > 2) victim->sex = 0;
    snprintf( buf, MAX_INPUT_LENGTH, "%s is %d years old.\n\r", he_she[victim->sex], get_age(victim));
    send_to_char( buf, ch);
    
    snprintf( buf, MAX_INPUT_LENGTH, "%s was created %s\n\r", he_she[victim->sex], victim->createtime);
    send_to_char( buf, ch);

	status = " bugged character";
	if ( victim->level < 3 )													status = " Mortal";
	else if ( victim->level == 3 )
	{
		if ( victim->race == 0 )												status = "n Avatar";
		else if ( victim->race >= 1 && victim->race < 5 )						status = "n Immortal";
		else if ( victim->race >= 5 && victim->race < 10 )						status = " Fighter";
		else if ( victim->race >= 10 && victim->race < 15 )						status = "n Adventurer";
		else if ( victim->race >= 15 && victim->race < 20 )						status = " Champion";
		else if ( victim->race >= 20 && victim->race < 25 )						status = " Warrior";
		else if ( victim->race >= 25 )											status = " Hero";
	}
	else
	{
		if ( victim->level == LEVEL_JUSTICAR && IS_SET( victim->act, PLR_VAMPIRE) )
		{
			if ( ch->level < 3 )												status = " clan leader";
			else
			{
				if ( !str_cmp( victim->clan, "Assamite" ) )							status = " Assamite clan leader";
				else if ( !str_cmp( victim->clan, "Caitiff" ) )						status = " sinner";
				else if ( !str_cmp( victim->clan, "Lasombra" ) )						status = " Lasombra clan leader";
				else if ( !str_cmp( victim->clan, "Tzimisce" ) )					status = " Tzimisce clan leader";
				else if ( !str_cmp( victim->clan, "Toreador" ) )					status = " Toreador clan leader";
				else if ( !str_cmp( victim->clan, "Tremere" ) )						status = " Tremere clan leader";
				else if ( !str_cmp( victim->clan, "Ventrue" ) )						status = " Ventrue clan leader";
			}
		}
		else
		{
			status = " member of the Admin team";
		}
	}
	snprintf( buf, MAX_INPUT_LENGTH, "%s is a%s.\n\r", he_she[victim->sex], status );
	send_to_char( buf, ch );

    if(victim->pcdata->email[0]!='\0')
	   snprintf(buf,  MAX_INPUT_LENGTH, "Email: %s\n\r",victim->pcdata->email);
    else strncpy( buf, "No email address submitted.\n\r", MAX_INPUT_LENGTH);
    send_to_char(buf, ch);
    
    send_to_char("----------------------------------------------------\n\r", ch);	
    
    if (fPlay && victim->level >= LEVEL_JUSTICAR)
	   act("$n just retrieved finger information about you.",ch,NULL,victim, TO_VICT);
    
    
    
    
    return;
    
    
}

void do_member(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mch;
    char buf[MAX_INPUT_LENGTH];
    
    
    if( argument[0] == '\0')
    {
	   send_to_char("Which clan?\n\r",ch);
	   return;
    }
    
    if(!str_cmp(ch->clan, "") && ch->level < LEVEL_GOD)
    {
	   send_to_char("You have to be a member of a clan to do that!\n\r", ch);
	   return;
    }
    
    
    
    if(!str_cmp(ch->clan, argument) || ch->level >= LEVEL_GOD)
    {
	   send_to_char("-----------= Clan Members =-----------\n\r",ch);
	   for(mch = char_list ; mch != NULL ; mch = mch->next)
	   { 
		  if (!IS_NPC(mch) && (!str_cmp(argument,mch->clan)))
		  {
			 if ((mch->vampgen == 2) && (!IS_SET(mch->act, PLR_WIZINVIS)))
			 {
				snprintf(buf,  MAX_INPUT_LENGTH, " %s <Leader of %s> \n\r",
				    mch->name, mch->clan);					
				send_to_char(buf, ch);
			 }
			 else if(IS_EXTRA(mch,EXTRA_PRINCE))
			 {
				snprintf(buf,  MAX_INPUT_LENGTH, " %s <Prince for %s> \n\r",
				    mch->name, mch->clan);
				send_to_char( buf, ch);
			 }
			 else if(IS_EXTRA(mch,EXTRA_SIRE))
			 {
				snprintf(buf,  MAX_INPUT_LENGTH, " %s <Sire for %s> \n\r",
				    mch->name, mch->clan);
				send_to_char( buf, ch);
			 }
			 else if (!IS_SET(mch->act, PLR_WIZINVIS))
			 {
				snprintf(buf,  MAX_INPUT_LENGTH, " %s \n\r", mch->name);
				send_to_char( buf, ch);
			 }
		  }
		  
	   }
	   
	   send_to_char("--------------------------------------\n\r",ch);
    }
    else 
    {
	   send_to_char("You cannot get information on that clan.\n\r", ch);
    }
    return;
    
}

void do_imminfo( char *argument )
{
    DESCRIPTOR_DATA *d;
    
    if( argument[0] == '\0')
    {
	   return;
    }
    
    for( d=descriptor_list; d!=NULL; d=d->next)
    {
	   if( d->connected == CON_PLAYING
		  && d->character->level >= LEVEL_GOD && !IS_SET(d->character->deaf, CHANNEL_IMMINFO))
	   {
		  send_to_char( "Imminfo -> ", d->character);
		  send_to_char( argument ,d->character);
		  send_to_char( "\n\r", d->character);
	   }
	   
    }
    
    
    
}

void do_deposit(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int money;
    
    argument = one_argument( argument, arg, MAX_INPUT_LENGTH );
    
    if( arg[0] == '\0')
    {
	   send_to_char( "Deposit how much gold?\n\r", ch);
	   return;
    }
    
    if( !is_number( arg ))
    {
	   send_to_char( "Er ?? You want to deposit what?? \n\r", ch);
	   return;
    }
    
    money = atoi( arg );
    
    if(!IS_SET(ch->in_room->room_flags, ROOM_BANK))
    {
	   send_to_char( "You must be in a bank to deposit gold!\n\r",ch);
	   return;
    }
    
    if( money < 1)
	   return;
    if( money > ch->gold )
    {
	   send_to_char( "You don't have that much gold!\n\r", ch);
	   return;
    }
    
    ch->gold = ch->gold - money;
    ch->bank = ch->bank + money * 9/10;
    
    act( "$n makes a transaction.", ch, NULL, NULL, TO_ROOM);
    
    snprintf( buf,  MAX_STRING_LENGTH, "You have %ld gold coins in hand, and %ld in the bank.\n\r", ch->gold, ch->bank);
    send_to_char( buf, ch);
    return;
    
}


void do_withdraw( CHAR_DATA *ch, char * argument)
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int money;
    
    argument = one_argument( argument, arg, MAX_INPUT_LENGTH );
    
    if( arg[0] == '\0')
    {
	   send_to_char( "Withdraw how much gold?\n\r",ch);
	   return;
    }
    
    if( !is_number( arg) )
    {
	   send_to_char( "You want to withdraw WHAT??\n\r", ch);
	   return;
    }
    
    money = atoi(arg);
    
    if(!IS_SET(ch->in_room->room_flags, ROOM_BANK))
    {
	   send_to_char( "You must be in a bank to withdraw gold\n\r", ch);
	   return;
    }
    
    if( money < 1)
	   return;
    if( money > ch->bank)
    {
	   send_to_char("You don't have that much money in the bank.\n\r",ch);
	   return;
    }
    
    ch->gold = ch->gold + money;
    ch->bank = ch->bank - money;
    
    act( "$n makes a transaction.\n\r", ch, NULL, NULL, TO_ROOM);
    
    snprintf( buf,  MAX_STRING_LENGTH, "You have %ld gold in hand, and %ld in the bank.\n\r", ch->gold, ch->bank);
    send_to_char( buf, ch);
    return;
    
}

void do_clanitem( CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    const int GOD_LEVEL = LEVEL_SEER;

    smash_tilde( argument );
    argument = one_argument( argument, arg, MAX_INPUT_LENGTH);
    argument = one_argument( argument, arg2, MAX_INPUT_LENGTH);

    
    if( ch->level < LEVEL_JUSTICAR && !IS_EXTRA(ch, EXTRA_TRUSTED))
    {
           send_to_char( "Only for Divine Beings or Clan Leaders! \n\r", ch);
	   return;
    }
    
    if( arg[0] == '\0')
    {
	   send_to_char( "Create which clan item (1,2 or 3)?\n\r", ch);
	   return;
    }

    if( arg2[0] == '\0' && ch->level >= LEVEL_SEER )
    {
         send_to_char( "Create item for which clan ?\n\r", ch);
         return;
    }
    
    if( arg[0] == '1' )
    {
          if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Assamite")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Assamite")) )
          {
              obj = create_object(get_obj_index( 24900 ), 0);
              obj_to_char(obj, ch); 
              act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
              act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
              return; 
          }
          if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Tzimisce")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Tzimisce")) )
          {
              obj = create_object(get_obj_index( 24903 ), 0);
              obj_to_char(obj, ch); 
              act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
              act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
              return; 
          }
          if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Tremere")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Tremere")) )
          {
		  obj = create_object(get_obj_index( 24906 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Toreador")) ||
               (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Toreador")) )
	   {
		  obj = create_object(get_obj_index( 24909 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Ventrue")) ||
               (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Ventrue")) )
	   {
		  obj = create_object(get_obj_index( 24912 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Nosferatu")) ||
               (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Nosferatu")) )
	   {
		  obj = create_object(get_obj_index( 24915 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Lasombra")) ||
               (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Lasombra")) )
	   {
		  obj = create_object(get_obj_index( 24918 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Cappadocian")) ||
               (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Cappadocian")) )
	   {
		  obj = create_object(get_obj_index( 24929 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
	   else 		
	   {
                  send_to_char( "There is no clan item 1 for that clan.\n\r",ch);
		  return;
	   }
	   return;
    }
    
    if( arg[0] == '2' )
    {
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Assamite")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Assamite")) )
	   {
		  obj = create_object(get_obj_index( 24901 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Tzimisce")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Tzimisce")) )
	   {
		  obj = create_object(get_obj_index( 24904 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Tremere")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Tremere")) )
	   {
		  obj = create_object(get_obj_index( 24907 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Toreador")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Toreador")) )
	   {
		  obj = create_object(get_obj_index( 24910 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Ventrue")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Ventrue")) )
	   {
		  obj = create_object(get_obj_index( 24913 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Nosferatu")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Nosferatu")) )
	   {
		  obj = create_object(get_obj_index( 24916 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Lasombra")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Lasombra")) )
	   {
		  obj = create_object(get_obj_index( 24919 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Cappadocian")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Cappadocian")) )
	   {
		  obj = create_object(get_obj_index( 24930 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
	   else 		
	   {
		  send_to_char( "There is no clan item 2 for your clan.\n\r",ch);
		  return;
	   }
	   return;
    }
    
    if( arg[0] == '3' )
    {
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Assamite")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Assamite")) )
	   {
		  obj = create_object(get_obj_index( 24902 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Tzimisce")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Tzimisce")) )
	   {
		  obj = create_object(get_obj_index( 24905 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Tremere")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Tremere")) )
	   {
		  obj = create_object(get_obj_index( 24908 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Toreador")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Toreador")) )
	   {
		  obj = create_object(get_obj_index( 24911 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Ventrue")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Ventrue")) )
	   {
		  obj = create_object(get_obj_index( 24914 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Nosferatu")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Nosferatu")) )
	   {
		  obj = create_object(get_obj_index( 24917 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Lasombra")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Lasombra")) )
	   {
		  obj = create_object(get_obj_index( 24920 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
           if( ((arg2[0]=='\0') && !str_cmp(ch->clan, "Cappadocian")) ||
              (ch->level >= GOD_LEVEL && !str_cmp(arg2, "Cappadocian")) )
	   {
		  obj = create_object(get_obj_index( 24931 ), 0);
		  obj_to_char(obj, ch); 
		  act( "$n mutters some words and $p appears.", ch, obj, NULL, TO_ROOM);
		  act( "You mutter some words and $p appears.", ch, obj, NULL, TO_CHAR);
		  return; 
	   }
	   else 		
	   {
		  send_to_char( "There is no clan item 3 for your clan.\n\r",ch);
		  return;
	   }
	   return;
    }
    
    
    return;
}

void do_godwho( CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    
    one_argument( argument, arg, MAX_INPUT_LENGTH);
    
    if(IS_NPC(ch)) return;
    
    send_to_char_formatted("[      Name      ] [ Sta ] [ Age  ] [ Hits  % ] [ Mana  % ] [ Move  % ] [   Exp    ]\n\r",ch);
    for( gch = char_list; gch != NULL; gch = gch->next)
    {
	   if( IS_NPC(gch)) continue;
	   if( !can_see(ch,gch)) continue;
	   
           snprintf( buf,  MAX_STRING_LENGTH, "[%-16s] [ %3d ] [ %4d ] [%-6d%3d] [%-6d%3d] [%-6d%3d] [ %7ld  ]\n\r",
		  capitalize( gch->name),
		  gch->race,
                  get_age(gch), 
		  gch->hit, (gch->hit * 100/ gch->max_hit),
		  gch->mana, (gch->mana * 100/ gch->max_mana),
		  gch->move, (gch->move * 100/ gch->max_move),
		  gch->exp);
	   send_to_char_formatted( buf, ch);
    }
    
    
    return;
    
    
}



void do_backup( CHAR_DATA *ch, char *argument )
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
    snprintf( temp,  MAX_INPUT_LENGTH, "%s%s%s%s", BACKUP_DIR, initial( ch->name ), "/", capitalize( ch->name) );
    snprintf( strsave,  MAX_INPUT_LENGTH, "tmp.file");
#else	
    snprintf( temp,  MAX_INPUT_LENGTH, "%s%s", BACKUP_DIR, capitalize( ch->name ) );
    snprintf( strsave,  MAX_INPUT_LENGTH, "tmp.file");
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
		  snprintf(buf, MAX_INPUT_LENGTH, "%s Last logged in on %s", chlevel, ch->lasttime);
	   else
		  snprintf(buf, MAX_INPUT_LENGTH, "%s New player logged in on %s", chlevel, ch->createtime);
	   fprintf( fp, "%s", buf);
    }
    fclose( fp );
    
    send_to_char("Backed up.\n\r", ch);
    
    fpReserve = fopen( NULL_FILE, "r" );
    rename( strsave, temp );
    return;
}




void do_delete( CHAR_DATA *ch, char *argument)
{
    char temp[MAX_INPUT_LENGTH];
    
    if ( IS_NPC(ch) )
    {
	   send_to_char("Mobs can't delete !\n\r", ch);
	   return;
    }
    
    if(argument[0] == '\0')
    {
	   send_to_char("Syntax: delete <password> \n\r",ch);
	   return;
    }
    
    if(strcmp(argument, ch->pcdata->pwd) && strcmp(crypt(argument, ch->pcdata->pwd), ch->pcdata->pwd))
    {
	   send_to_char("Wrong password!\n\r", ch);
	   return;
    }
    
    snprintf( temp,  MAX_INPUT_LENGTH, "mv %s%s%s%s %s%s &\n\r", 
	   PLAYER_DIR, initial(ch->name), "/", capitalize(ch->name),
	   DELETE_DIR, capitalize(ch->name));
    int systemReturn = system(temp);
    if(systemReturn == -1)
    {
        send_to_char("Could not delete...Please let an admin know.\n\r", ch);
        bug("Could not delete character", 0);
        return;
    }
    send_to_char( "Ok.\n\r", ch); 
    ch->level = 0;
    do_quit( ch, "");
    
    return;
}


void do_omember( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    bool fOld = FALSE; 
    
    
    if( ch->level != LEVEL_CODER && str_cmp(ch->name, "Palmer")) return;
    
    if( argument[0] == '\0')
    {
	   send_to_char("Finger who?\n\r",ch);
	   return;
    }
    
    argument[0] = UPPER( argument[0] );
    
    if( !check_parse_name(argument) )
    {
	   send_to_char("Illegal name.\n\r",ch);
	   return;
    }
    
    if(  (victim = get_char_world(ch, argument)) )
    {
	   fOld=TRUE;
    }
    
    if(!fOld)
    {
	   send_to_char("That person is not on Vampire Wars at the moment!\n\r",ch);
	   return;
    }
    
    if(IS_NPC(victim))
    {
	   send_to_char("That is a mob!\n\r", ch);
	   return;
    }
    
    if( !IS_EXTRA(victim, EXTRA_ORGYMEMBER)) 
    {
	   send_to_char("Member added. \n\r",ch);
	   SET_BIT(victim->extra, EXTRA_ORGYMEMBER);		
	   send_to_char("You have been accepted into the orgy!\n\r", victim);
    }
    else
    {
	   send_to_char("Member removed. \n\r",ch);
	   REMOVE_BIT(victim->extra, EXTRA_ORGYMEMBER);
	   send_to_char("You have been removed from the orgy!\n\r", victim);
    }
    
    return;
}

void do_ordain( CHAR_DATA *ch, char *argument)
{
    /* Archrip - CHAR_DATA *victim; */
    /* Archrip - bool fOld = FALSE; */
    
    send_to_char("Taken this out for now, did nothing anyway - Archon./n/r", ch);
    return;
    
    /*
    
	 if( ch->level != LEVEL_CODER && str_cmp(ch->name, "Palmer")) return;
	 
	   if( argument[0] == '\0')
	   {
	   send_to_char("Ordain who?\n\r",ch);
	   return;
	   }
	   
		argument[0] = UPPER( argument[0] );
		
		  if( !check_parse_name(argument) )
		  {
		  send_to_char("Illegal name.\n\r",ch);
		  return;
		  }
		  
		    if(  (victim = get_char_world(ch, argument)) )
		    {
		    fOld=TRUE;
		    }
		    
			 if(!fOld)
			 {
			 send_to_char("That person is not on Vampire Wars at the moment!\n\r",ch);
			 return;
			 }
			 
			   if(IS_NPC(victim))
			   {
			   send_to_char("That is a mob!\n\r", ch);
			   return;
			   }
			   
				if( !IS_SET(victim->act, PLR_KNIGHT)) 
				{
				send_to_char("Member added. \n\r",ch);
				SET_BIT(victim->act, PLR_KNIGHT);		
				send_to_char("You have been ordained into the order!\n\r", victim);
				}
				else
				{
				send_to_char("Member removed. \n\r",ch);
				REMOVE_BIT(victim->act, PLR_KNIGHT);
				send_to_char("You have been rejected from the order!\n\r", victim);
				}
				
				  return;
    */
}


void do_sabbat( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    bool fOld = FALSE; 
    
    
    if( ch->level != LEVEL_CODER && str_cmp(ch->name, "Palmer")) return;
    
    if( argument[0] == '\0')
    {
	   send_to_char("Induce who?\n\r",ch);
	   return;
    }
    
    argument[0] = UPPER( argument[0] );
    
    if( !check_parse_name(argument) )
    {
	   send_to_char("Illegal name.\n\r",ch);
	   return;
    }
    
    if(  (victim = get_char_world(ch, argument)) )
    {
	   fOld=TRUE;
    }
    
    if(!fOld)
    {
	   send_to_char("That person is not on Vampire Wars at the moment!\n\r",ch);
	   return;
    }
    
    if(IS_NPC(victim))
    {
	   send_to_char("That is a mob!\n\r", ch);
	   return;
    }
    
    if( !IS_SET(victim->act, PLR_SABBAT)) 
    {
	   send_to_char("Member added. \n\r",ch);
	   SET_BIT(victim->act, PLR_SABBAT);		
	   send_to_char("You are now a Sabbat member!\n\r", victim);
    }
    else
    {
	   send_to_char("Member removed. \n\r",ch);
	   REMOVE_BIT(victim->act, PLR_SABBAT);
	   send_to_char("You have been rejected from the Sabbat!\n\r", victim);
    }
    
    return;
}

void do_clandeposit(CHAR_DATA *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    long money;
    
    argument = one_argument( argument, arg, MAX_INPUT_LENGTH );
    
    if( arg[0] == '\0')
    {
	   send_to_char( "Deposit how much gold?\n\r", ch);
	   return;
    }
    
    if( !is_number( arg ))
    {
	   send_to_char( "Er ?? You want to deposit what?? \n\r", ch);
	   return;
    }
    
    if( !IS_SET(ch->act, PLR_VAMPIRE))
    {
	   send_to_char( "Only vampire clan members can do this.\n\r",ch);
	   return;
    }
    
    money = atoi( arg );
    
    if(!IS_SET(ch->in_room->room_flags, ROOM_BANK))
    {
	   send_to_char( "You must be in a bank to deposit gold!\n\r",ch);
	   return;
    }
    
    if( money < 1)
	   return;
    if( money > ch->gold )
    {
	   send_to_char( "You don't have that much gold!\n\r", ch);
	   return;
    }
    
    if( !str_cmp(ch->clan, "Assamite") )
    {
	   ch->gold = ch->gold - money;
	   clan_infotable[1].members = clan_infotable[1].members + (money * 9/10);
	   send_to_char( "You have just deposited money into the Assamite account.\n\r",ch);
    }
    else if( !str_cmp(ch->clan, "Tzimisce") ) 
    {
	   ch->gold = ch->gold - money;
	   clan_infotable[2].members = clan_infotable[2].members + (money * 9/10);
	   send_to_char( "You have just deposited money into the Tzimisce account.\n\r",ch);
    }
    else if( !str_cmp(ch->clan, "Ventrue") ) 
    {
	   ch->gold = ch->gold - money;
	   clan_infotable[3].members = clan_infotable[3].members + (money * 9/10);
	   send_to_char( "You have just deposited money into the Ventrue account.\n\r",ch);
    }
    else if( !str_cmp(ch->clan, "Tremere") ) 
    {
	   ch->gold = ch->gold - money;
	   clan_infotable[4].members = clan_infotable[4].members + (money * 9/10);
	   send_to_char( "You have just deposited money into the Tremere account.\n\r",ch);
    }
    else if( !str_cmp(ch->clan, "Lasombra") ) 
    {
	   ch->gold = ch->gold - money;
	   clan_infotable[5].members = clan_infotable[5].members + (money * 9/10);
	   send_to_char( "You have just deposited money into the Lasombra account.\n\r",ch);
    }
    else if( !str_cmp(ch->clan, "Toreador") ) 
    {
	   ch->gold = ch->gold - money;
	   clan_infotable[6].members = clan_infotable[6].members + (money * 9/10);
	   send_to_char( "You have just deposited money into the Toreador account.\n\r",ch);
    }
    else
    {
	   send_to_char( "You do not have a clan, so dont be silly!\n\r",ch);
	   return;
    } 
    
    act( "$n makes a transaction.", ch, NULL, NULL, TO_ROOM);
    
    return;
    
}

void do_clanwithdraw( CHAR_DATA *ch, char * argument)
{
    char arg[MAX_INPUT_LENGTH];
    long money;
    
    argument = one_argument( argument, arg, MAX_INPUT_LENGTH );
    
    if( arg[0] == '\0')
    {
	   send_to_char( "Withdraw how much gold?\n\r",ch);
	   return;
    }
    
    if( !is_number( arg) )
    {
	   send_to_char( "You want to withdraw WHAT??\n\r", ch);
	   return;
    }
    
    if( !IS_SET(ch->act, PLR_VAMPIRE))
    {
	   send_to_char( "Only vampire clan members can do this.\n\r",ch);
	   return;
    }
    
    money = atoi(arg);
    
    if(!IS_SET(ch->in_room->room_flags, ROOM_BANK))
    {
	   send_to_char( "You must be in a bank to withdraw gold\n\r", ch);
	   return;
    }
    
    if( money < 1)
	   return;
   
    if( !str_cmp(ch->clan, "Assamite") )
    {
	   if( money > clan_infotable[1].members)
	   {
		  send_to_char("The Assamite account does not have that much money\n\r",ch);
		  return;
	   }
	   else
		  clan_infotable[1].members = clan_infotable[1].members - money;
    }
    else if( !str_cmp(ch->clan, "Tzimisce") ) 
    {
	   if( money > clan_infotable[2].members)
	   {
		  send_to_char("The Tzimisce account does not have that much money\n\r",ch);
		  return;
	   }
	   else
		  clan_infotable[2].members = clan_infotable[2].members - money;
    }
    else if( !str_cmp(ch->clan, "Ventrue") ) 
    {
	   
	   if( money > clan_infotable[3].members)
	   {
		  send_to_char("The Ventrue account does not have that much money\n\r",ch);
		  return;
	   }
	   else
		  clan_infotable[3].members = clan_infotable[3].members - money;
    }
    else if( !str_cmp(ch->clan, "Tremere") ) 
    {
	   if( money > clan_infotable[4].members)
	   {
		  send_to_char("The Tremere account does not have that much money\n\r",ch);
		  return;
	   }
	   else
		  clan_infotable[4].members = clan_infotable[4].members - money;
    }
    else if( !str_cmp(ch->clan, "Lasombra") )
    {
	   if( money > clan_infotable[5].members)
	   {
		  send_to_char("The Lasombra account does not have that much money\n\r",ch);
		  return;
	   }
	   else
		  clan_infotable[5].members = clan_infotable[5].members - money;
    }
    else if( !str_cmp(ch->clan, "Toreador") )
    {
	   if( money > clan_infotable[6].members)
	   {
		  send_to_char("The Toreador account does not have that much money\n\r",ch);
		  return;
	   }
	   else
		  clan_infotable[6].members = clan_infotable[6].members - money;
    }
    else if( !str_cmp(ch->clan, "Caitiff")  )
    {
	   
	   if( money > clan_infotable[7].members)
	   {
		  send_to_char("The Caitiff account does not have that much money\n\r",ch);
		  return;
	   }
	   else
		  clan_infotable[7].members = clan_infotable[7].members - money;
    }
    else
    {
	   send_to_char( "You do not have a clan, so dont be silly!\n\r",ch);
	   return;
    } 
    
    ch->gold = ch->gold + money;
    act( "$n makes a transaction.\n\r", ch, NULL, NULL, TO_ROOM);
    return;
    
}

void do_clanbalance(CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    int clanno = 0;
    
    
    if( !IS_SET(ch->act, PLR_VAMPIRE))
    {
	   send_to_char( "Only vampire clan members can do this.\n\r",ch);
	   return;
    }
    
    
    if(!IS_SET(ch->in_room->room_flags, ROOM_BANK))
    {
	   send_to_char( "You must be in a bank to get a balance!\n\r",ch);
	   return;
    }
    
    if( ch->level > LEVEL_ORACLE)
    {
	   clanno=clan_infotable[1].members;
	   snprintf( buf, MAX_STRING_LENGTH, "Assamite = %d gold in the bank.\n\r", clanno);
	   send_to_char(buf,ch);
	   clanno=clan_infotable[2].members;
	   
	   snprintf( buf, MAX_STRING_LENGTH, "Tzimisce = %d gold in the bank.\n\r", clanno);
	   send_to_char(buf,ch);
	   clanno=clan_infotable[3].members;
	   
	   snprintf( buf, MAX_STRING_LENGTH, "Ventrue = %d gold in the bank.\n\r", clanno);
	   send_to_char(buf,ch);
	   clanno=clan_infotable[4].members;
	   
	   snprintf( buf, MAX_STRING_LENGTH, "Tremere = %d gold in the bank.\n\r", clanno);
	   send_to_char(buf,ch);
	   clanno=clan_infotable[5].members;
	   
	   snprintf( buf, MAX_STRING_LENGTH, "Lasombra = %d gold in the bank.\n\r", clanno);
	   send_to_char(buf,ch);
	   clanno=clan_infotable[6].members;
	   
	   snprintf( buf, MAX_STRING_LENGTH, "Toreador = %d gold in the bank.\n\r", clanno);
	   send_to_char(buf,ch);
	   clanno=clan_infotable[7].members;
	   
	   snprintf( buf, MAX_STRING_LENGTH, "Caitiff = %d gold in the bank.\n\r", clanno);
	   send_to_char(buf,ch);
	   return;
    }
	
    if( !str_cmp(ch->clan, "Assamite") )
    {
	   clanno = clan_infotable[1].members;
    }
    else if( !str_cmp(ch->clan, "Tzimisce") ) 
    {
	   clanno = clan_infotable[2].members;
    }
    else if( !str_cmp(ch->clan, "Ventrue") ) 
    {
	   clanno = clan_infotable[3].members;
    }
    else if( !str_cmp(ch->clan, "Tremere") ) 
    {
	   clanno = clan_infotable[4].members;
    }
    else if( !str_cmp(ch->clan, "Lasombra") ) 
    {
	   clanno = clan_infotable[5].members;
    }
    else if( !str_cmp(ch->clan, "Toreador") ) 
    {
	   clanno = clan_infotable[6].members;
    }
    else if( !str_cmp(ch->clan, "Caitiff")  ) 
    {
	   clanno =clan_infotable[7].members;
    }
    else
    {
	   send_to_char( "You do not have a clan, so dont be silly!\n\r",ch);
	   return;
    } 
    
    snprintf( buf, MAX_STRING_LENGTH, "Your clan currently has %d gold in the bank.\n\r", clanno);
    send_to_char(buf,ch);
    
    return;
    
}

void do_personal( CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *victim;
    bool fOld = FALSE; 
    
    
    if( ch->level != LEVEL_CODER && str_cmp(ch->name, "Palmer")) return;
    
    if( argument[0] == '\0')
    {
	   send_to_char("Finger who?\n\r",ch);
	   return;
    }
    
    argument[0] = UPPER( argument[0] );
    
    if( !check_parse_name(argument) )
    {
	   send_to_char("Illegal name.\n\r",ch);
	   return;
    }
    
    if(  (victim = get_char_world(ch, argument)) )
    {
	   fOld=TRUE;
    }
    
    if(!fOld)
    {
	   send_to_char("That person is not on Vampire Wars at the moment!\n\r",ch);
	   return;
    }
    
    if(IS_NPC(victim))
    {
	   send_to_char("That is a mob!\n\r", ch);
	   return;
    }
    
    if( !IS_EXTRA(victim, EXTRA_PERSONAL)) 
    {
	   send_to_char("Member added. \n\r",ch);
	   SET_BIT(victim->extra, EXTRA_PERSONAL);		
	   send_to_char("You are now on the personal channel.\n\r", victim);
    }
    else
    {
	   send_to_char("Member removed. \n\r",ch);
	   REMOVE_BIT(victim->extra, EXTRA_PERSONAL);
	   send_to_char("You are no longer on the personal channel.\n\r", victim);
    }
    
    return;
}

void do_outcast(CHAR_DATA *ch, char *argument)
{
    char arg [MAX_INPUT_LENGTH];
    CHAR_DATA *victim;
    
    one_argument( argument, arg, MAX_INPUT_LENGTH);

    if( arg[0] == '\0')
    {
	   send_to_char("Outcast who?\n\r",ch);
	   return;
    }
    
   
    if( NULL == (victim = get_char_world(ch, arg)) )
    {
	   send_to_char("That person is not on Vampire Wars at the moment!\n\r",ch);
	   return;
    }
    
    if(IS_NPC(victim))
    {
	   send_to_char("That is a mob!\n\r", ch);
	   return;
    }
    
    if( !strcmp(victim->clan, ""))
    {
	   send_to_char("That person is not in a clan\n\r", ch);
	   return;
    }
    
    if( !strcmp(victim->clan, "Caitiff"))
    {
	   send_to_char("That person is Caitiff already!\n\r", ch);
	   return;
    }
    
    if( !strcmp(ch->clan, victim->clan))
    {
	   send_to_char("You have been cast out of your clan!\n\r", victim);
	   send_to_char("Ok.\n\r", ch);
       free_string( victim->clan);
       victim->clan = str_dup( "Caitiff");
	   return;
    }
    
    
    return;
}

void do_addlag(CHAR_DATA *ch, char *argument)
{

  CHAR_DATA *victim;
  char arg1[MAX_STRING_LENGTH];
  int x;

  argument = one_argument(argument, arg1, MAX_STRING_LENGTH);

  if (arg1[0] == '\0')
    {
      send_to_char("addlag to who?", ch);
      return;
    }

  if ((victim = get_char_world(ch, arg1)) == NULL)
    {
      send_to_char("They're not here.", ch);
      return;
    }


  if ((x = atoi(argument)) < 0)
    {
      send_to_char("That makes a LOT of sense.", ch);
      return;
    }

  if (x > 100)
    {
      send_to_char("There's a limit to cruel and unusual punishment", ch);
      return;
    }

 if (IS_NPC(victim)) return;

  if( victim->level >= LEVEL_SEER)
  {
      send_to_char("No way! Not on admin", ch);
     return;
  }


/*  send_to_char("Somebody REALLY didn't like you", victim); - no warning 2 players */
  victim->lagpenalty = x;
  WAIT_STATE(victim, x);
  send_to_char("Adding lag now...", ch);
  return;
}

void do_sinbin( CHAR_DATA *ch, char *argument)
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    CHAR_DATA *gch;
    
    one_argument( argument, arg, MAX_INPUT_LENGTH);
    
    if(IS_NPC(ch)) return;
    
    send_to_char_formatted("[      Name      ] [ Lag ]   Misc\n\r",ch);
    for( gch = char_list; gch != NULL; gch = gch->next)
    {
	   if( IS_NPC(gch)) continue;
	   if( gch->lagpenalty > 0 || IS_SET(gch->act, PLR_SILENCE | PLR_FREEZE | PLR_NO_TELL | PLR_NO_EMOTE))
           {  
           snprintf( buf,  MAX_STRING_LENGTH, "[%-16s] [ %3d ]   %s%s%s%s\n\r",
		  capitalize( gch->name),
		  gch->lagpenalty, 
		  IS_SET(gch->act, PLR_SILENCE) ? "Silenced " : "",
		  IS_SET(gch->act, PLR_FREEZE) ? "Frozen " : "",
		  IS_SET(gch->act, PLR_NO_TELL) ? "No-Tells " : "",
		  IS_SET(gch->act, PLR_NO_EMOTE) ? "No-Emote " : "");
	   send_to_char_formatted( buf, ch);
           }
    }
    return;
    
    
}

void do_qlist( CHAR_DATA *ch, char *argument )
{

    send_to_char_formatted( "[Price] Item                          Short Name\n\r", ch );
    send_to_char_formatted( "[   10] A Blood Rod                      rod\n\r",ch);
    send_to_char_formatted( "[   15] A Bag                            bag\n\r",ch);
    send_to_char_formatted( "[   30] A Stake                          stake\n\r",ch); 
    send_to_char_formatted( "[   40] a special issue pair of boots    boots\n\r",ch);
    send_to_char_formatted( "[   40] a special issue ring             ring\n\r",ch);
    send_to_char_formatted( "[   40] a special issue pendant          pendant\n\r",ch);
    send_to_char_formatted( "[   40] a special issue breastplate      plate\n\r",ch);
    send_to_char_formatted( "[   40] a special issue helmet           helmet\n\r",ch);
    send_to_char_formatted( "[   40] a special issue pair of leggings leggings\n\r",ch);
    send_to_char_formatted( "[   40] a special issue pair of gloves   gloves\n\r",ch);
    send_to_char_formatted( "[   40] a special issue pair of sleeves  sleeves\n\r",ch);
    send_to_char_formatted( "[   40] a special issue cloak            cloak\n\r",ch);
    send_to_char_formatted( "[   40] a special issue girth            girth\n\r",ch);
    send_to_char_formatted( "[   40] a special issue bracer           bracer\n\r",ch);
    send_to_char_formatted( "[   40] a special issue face mask        mask\n\r",ch);
    send_to_char_formatted( "[   50] a special issue sword            sword\n\r",ch);

    return;
}

void do_qbuy( CHAR_DATA *ch, char *argument)
{
    char arg1 [MAX_INPUT_LENGTH];
    OBJ_DATA *obj;
    OBJ_INDEX_DATA *pObjIndex;
    int value;
    int cost;
    /* bool has_container */
    
    
    smash_tilde( argument );
    argument = one_argument( argument, arg1, MAX_INPUT_LENGTH );

    if( IS_NPC(ch))
    {
         send_to_char( "Not for mobs silly!\n\r", ch);
         return;
    }

    if( ch->pcdata->quest <=0 )
    {
        send_to_char("You don't have any quest points, so how do you propose to pay!\n\r",ch);
        return;
    }

    if( arg1[0] == '\0')
    {
       send_to_char("Type qlist to see what you can buy.\n\r",ch);
       return;
    }
  
    cost = 0; 
    value = 0;
    if( !str_cmp( arg1, "boots")){             value=1;cost=40;}
    else if( !str_cmp( arg1, "stake")){        value=2;cost=30;}
    else if( !str_cmp( arg1, "rod")){          value=3;cost=10;}
    else if( !str_cmp( arg1, "ring")){         value=4;cost=40;}
    else if( !str_cmp( arg1, "pendant")){      value=5;cost=40;}
    else if( !str_cmp( arg1, "plate")){        value=6;cost=40;}
    else if( !str_cmp( arg1, "helmet")){       value=7;cost=40;}
    else if( !str_cmp( arg1, "leggings")){     value=8;cost=40;}
    else if( !str_cmp( arg1, "gloves")){       value=9;cost=40;}
    else if( !str_cmp( arg1, "sleeves")){      value=10;cost=40;}
    else if( !str_cmp( arg1, "cloak")){        value=11;cost=40;}
    else if( !str_cmp( arg1, "girth")){        value=12;cost=40;}
    else if( !str_cmp( arg1, "bracer")){       value=13;cost=40;}
    else if( !str_cmp( arg1, "mask")){         value=14;cost=40;}
    else if( !str_cmp( arg1, "bag")){          value=15;cost=15;}
    else if( !str_cmp( arg1, "sword")){        value=16;cost=50;}
    else
    {
       value=0;
    }

    if(ch->pcdata->quest < cost)
    {
        send_to_char("You don't have enough quest points.\n\r",ch);
        return;
    }

    if ( ( pObjIndex = get_obj_index( OBJ_VNUM_PROTOPLASM ) ) == NULL )
    {
       send_to_char( "Error! Missing object, inform the Admin.\n\r", ch );
       return;
    }

    /* has_container = does_ch_have_a_container(ch);
    if ( value == WHATEVER_CONTAINER_VALUE && has_container)
    {
       send_to_char( "You cannot carry another container.\n\r", ch);
       return;
    }*/


    if(value>0)
    {
        switch(value)
        {
          default:
               send_to_char("Ooops problem! Tell Palmer!\n\r",ch);
               break;
          case 1:
               ch->pcdata->quest -= cost;
               obj = create_object( pObjIndex, 25 );
               obj->weight = 1;
               obj->cost   = 1000;
               obj->value[0] = 15;
               obj->item_type = ITEM_ACCESSORY;
               obj->wear_flags = ITEM_TAKE + ITEM_WEAR_FEET;
               obj->quest=QUEST_NAME+QUEST_SHORT+QUEST_LONG;
               free_string( obj->short_descr );
               free_string( obj->name );
               free_string( obj->description );
               obj->short_descr = str_dup("special issue boots");
               obj->name = str_dup("special issue boots");
               obj->description = str_dup("a pair of special issue boots lie here"); 

               if (obj->questmaker != NULL) free_string(obj->questmaker);
               obj->questmaker = str_dup(ch->name);
               obj_to_char(obj,ch);
               act( "You reach up into the air and draw out a pair of special issue boots.", ch, obj, NULL, TO_CHAR );
               act( "$n reaches up into the air and draws out a pair of special issue boots.", ch, obj, NULL, TO_ROOM );

               break;
          case 2:
               ch->pcdata->quest -= cost;
               obj = create_object( pObjIndex, 25 );
               obj->weight = 1;
               obj->cost   = 1000;
               obj->item_type = ITEM_STAKE;
               obj->wear_flags = ITEM_TAKE + ITEM_WIELD;
               obj->quest=QUEST_NAME+QUEST_SHORT+QUEST_LONG;
               free_string( obj->short_descr );
               obj->short_descr = str_dup("a stake");
               free_string( obj->name );
               obj->name = str_dup("a stake");
               free_string( obj->description );
               obj->description = str_dup("a stake lies here");               
               if (obj->questmaker != NULL) free_string(obj->questmaker);
               obj->questmaker = str_dup(ch->name);
               obj_to_char(obj,ch);
               act( "You reach up into the air and draw out a stake.", ch, obj, NULL, TO_CHAR );
               act( "$n reaches up into the air and draws out a stake.", ch, obj, NULL, TO_ROOM );
               break;
          case 3:
	       ch->pcdata->quest -= cost;
	       obj = create_object( pObjIndex, 25 );
	       obj->weight = 1;
	       obj->cost   = 1000;
	       obj->item_type = ITEM_FOUNTAIN;
               obj->value[0] = 1000;
               obj->value[1] = 1000;
               obj->value[2] = 13; 
               obj->quest=QUEST_NAME+QUEST_SHORT+QUEST_LONG;
               free_string( obj->short_descr );
               obj->short_descr = str_dup( "a blood rod");
               free_string( obj->name );
               obj->name = str_dup("a blood rod");
               free_string( obj->description );
               obj->description = str_dup("a blood rod lies here");
               if (obj->questmaker != NULL) free_string(obj->questmaker);
               obj->questmaker = str_dup(ch->name);
               obj_to_char(obj,ch);
               act( "You reach up into the air and draw out a blood rod.", ch, obj, NULL, TO_CHAR );
               act( "$n reaches up into the air and draws out a blood rod.", ch, obj, NULL, TO_ROOM );
               break;
          case 4:
               ch->pcdata->quest -= cost;
               obj = create_object( pObjIndex, 25 );
               obj->weight = 1;
               obj->cost   = 1000;
               obj->value[0] = 15;
               obj->item_type = ITEM_ACCESSORY;
               obj->wear_flags = ITEM_TAKE + ITEM_WEAR_FINGER;
               obj->quest=QUEST_NAME+QUEST_SHORT+QUEST_LONG;
               free_string( obj->short_descr );
               obj->short_descr = str_dup("special issue ring");
               free_string( obj->name );
               obj->name = str_dup("special issue ring");
               free_string( obj->description );
               obj->description = str_dup("a special issue ring lies here");
               if (obj->questmaker != NULL) free_string(obj->questmaker);
               obj->questmaker = str_dup(ch->name);
               obj_to_char(obj,ch);
               act( "You reach up into the air and draw out a special issue ring.", ch, obj, NULL, TO_CHAR );
               act( "$n reaches up into the air and draws out a special issue ring.", ch, obj, NULL, TO_ROOM );
               break;
          case 5:
               ch->pcdata->quest -= cost;
               obj = create_object( pObjIndex, 25 );
               obj->weight = 1;
               obj->cost   = 1000;
               obj->value[0] = 15;
               obj->item_type = ITEM_ACCESSORY;
               obj->wear_flags = ITEM_TAKE + ITEM_WEAR_NECK;
               obj->quest=QUEST_NAME+QUEST_SHORT+QUEST_LONG;
               free_string( obj->short_descr );
               obj->short_descr = str_dup("special issue pendant");
               free_string( obj->name );
               obj->name = str_dup("special issue pendant");
               free_string( obj->description );
               obj->description = str_dup("a special issue pendant lies here");
               if (obj->questmaker != NULL) free_string(obj->questmaker);
               obj->questmaker = str_dup(ch->name);
               obj_to_char(obj,ch);
               act( "You reach up into the air and draw out a special issue pendant.", ch, obj, NULL, TO_CHAR );
               act( "$n reaches up into the air and draws out a special issue pendant.", ch, obj, NULL, TO_ROOM );

               break;
          case 6:
               ch->pcdata->quest -= cost;
               obj = create_object( pObjIndex, 25 );
               obj->weight = 1;
               obj->cost   = 1000;
               obj->value[0] = 15;
               obj->item_type = ITEM_ACCESSORY;
               obj->wear_flags = ITEM_TAKE + ITEM_WEAR_BODY;
               obj->quest=QUEST_NAME+QUEST_SHORT+QUEST_LONG;
               free_string( obj->short_descr );
               obj->short_descr = str_dup("special issue plate");
               free_string( obj->name );
               obj->name = str_dup("special issue plate");
               free_string( obj->description );
               obj->description = str_dup("a special issue breast plate lies here");
               if (obj->questmaker != NULL) free_string(obj->questmaker);
               obj->questmaker = str_dup(ch->name);
               obj_to_char(obj,ch);
               act( "You reach up into the air and draw out a special issue breast plate.", ch, obj, NULL, TO_CHAR );
               act( "$n reaches up into the air and draws out a special issue breast plate.", ch, obj, NULL, TO_ROOM );

               break;
          case 7:
               ch->pcdata->quest -= cost;
               obj = create_object( pObjIndex, 25 );
               obj->weight = 1;
               obj->cost   = 1000;
               obj->value[0] = 15;
               obj->item_type = ITEM_ACCESSORY;
               obj->wear_flags = ITEM_TAKE + ITEM_WEAR_HEAD;
               obj->quest=QUEST_NAME+QUEST_SHORT+QUEST_LONG;
               free_string( obj->short_descr );
               obj->short_descr = str_dup("special issue helmet");
               free_string( obj->name );
               obj->name = str_dup("special issue helmet");
               free_string( obj->description );
               obj->description = str_dup("a special issue helmet lies here");
               if (obj->questmaker != NULL) free_string(obj->questmaker);
               obj->questmaker = str_dup(ch->name);
               obj_to_char(obj,ch);
               act( "You reach up into the air and draw out a special issue helmet.", ch, obj, NULL, TO_CHAR );
               act( "$n reaches up into the air and draws out a special issue helmet.", ch, obj, NULL, TO_ROOM );

               break;
          case 8:
               ch->pcdata->quest -= cost;
               obj = create_object( pObjIndex, 25 );
               obj->weight = 1;
               obj->cost   = 1000;
               obj->value[0] = 15;
               obj->item_type = ITEM_ACCESSORY;
               obj->wear_flags = ITEM_TAKE + ITEM_WEAR_LEGS;
               obj->quest=QUEST_NAME+QUEST_SHORT+QUEST_LONG;
               free_string( obj->short_descr );
               obj->short_descr = str_dup("special issue leggings");
               free_string( obj->name );
               obj->name = str_dup("special issue leggings");
               free_string( obj->description );
               obj->description = str_dup("a pair of special issue leggings lie here");
               if (obj->questmaker != NULL) free_string(obj->questmaker);
               obj->questmaker = str_dup(ch->name);
               obj_to_char(obj,ch);
               act( "You reach up into the air and draw out a pair of special issue leggings.", ch, obj, NULL, TO_CHAR );
               act( "$n reaches up into the air and draws out a pair of special issue leggings.", ch, obj, NULL, TO_ROOM );

               break;
          case 9:
               ch->pcdata->quest -= cost;
               obj = create_object( pObjIndex, 25 );
               obj->weight = 1;
               obj->cost   = 1000;
               obj->value[0] = 15;
               obj->item_type = ITEM_ACCESSORY;
               obj->wear_flags = ITEM_TAKE + ITEM_WEAR_HANDS;
               obj->quest=QUEST_NAME+QUEST_SHORT+QUEST_LONG;
               free_string( obj->short_descr );
               obj->short_descr = str_dup("special issue gloves");
               free_string( obj->name );
               obj->name = str_dup("special issue gloves");
               free_string( obj->description );
               obj->description = str_dup("a pair of special issue gloves lie here");
               if (obj->questmaker != NULL) free_string(obj->questmaker);
               obj->questmaker = str_dup(ch->name);
               obj_to_char(obj,ch);
               act( "You reach up into the air and draw out a pair of special issue gloves.", ch, obj, NULL, TO_CHAR );
               act( "$n reaches up into the air and draws out a pair of special issue gloves.", ch, obj, NULL, TO_ROOM );

               break;
          case 10:
               ch->pcdata->quest -= cost;
               obj = create_object( pObjIndex, 25 );
               obj->weight = 1;
               obj->cost   = 1000;
               obj->value[0] = 15;
               obj->item_type = ITEM_ACCESSORY;
               obj->wear_flags = ITEM_TAKE + ITEM_WEAR_ARMS;
               obj->quest=QUEST_NAME+QUEST_SHORT+QUEST_LONG;
               free_string( obj->short_descr );
               obj->short_descr = str_dup("special issue sleeves");
               free_string( obj->name );
               obj->name = str_dup("special issue sleeves");
               free_string( obj->description );
               obj->description = str_dup("a pair of special issue sleeves lie here");
               if (obj->questmaker != NULL) free_string(obj->questmaker);
               obj->questmaker = str_dup(ch->name);
               obj_to_char(obj,ch);
               act( "You reach up into the air and draw out a pair of special issue sleeves.", ch, obj, NULL, TO_CHAR );
               act( "$n reaches up into the air and draws out a pair of special issue sleeves.", ch, obj, NULL, TO_ROOM );
               break;
          case 11:
               ch->pcdata->quest -= cost;
               obj = create_object( pObjIndex, 25 );
               obj->weight = 1;
               obj->cost   = 1000;
               obj->value[0] = 15;
               obj->item_type = ITEM_ACCESSORY;
               obj->wear_flags = ITEM_TAKE + ITEM_WEAR_ABOUT;
               obj->quest=QUEST_NAME+QUEST_SHORT+QUEST_LONG;
               free_string( obj->short_descr );
               obj->short_descr = str_dup("special issue cloak");
               free_string( obj->name );
               obj->name = str_dup("special issue cloak");
               free_string( obj->description );
               obj->description = str_dup("a special issue cloak lies here");
               if (obj->questmaker != NULL) free_string(obj->questmaker);
               obj->questmaker = str_dup(ch->name);
               obj_to_char(obj,ch);
               act( "You reach up into the air and draw out a special issue cloak.", ch, obj, NULL, TO_CHAR );
               act( "$n reaches up into the air and draws out a special issue cloak.", ch, obj, NULL, TO_ROOM );
               break;
          case 12:
               ch->pcdata->quest -= cost;
               obj = create_object( pObjIndex, 25 );
               obj->weight = 1;
               obj->cost   = 1000;
               obj->value[0] = 15;
               obj->item_type = ITEM_ACCESSORY;
               obj->wear_flags = ITEM_TAKE + ITEM_WEAR_WAIST;
               obj->quest=QUEST_NAME+QUEST_SHORT+QUEST_LONG;
               free_string( obj->short_descr );
               obj->short_descr = str_dup("special issue girth");
               free_string( obj->name );
               obj->name = str_dup("special issue girth");
               free_string( obj->description );
               obj->description = str_dup("a special issue girth lies here");
               if (obj->questmaker != NULL) free_string(obj->questmaker);
               obj->questmaker = str_dup(ch->name);
               obj_to_char(obj,ch);
               act( "You reach up into the air and draw out a special issue girth.", ch, obj, NULL, TO_CHAR );
               act( "$n reaches up into the air and draws out a special issue girth.", ch, obj, NULL, TO_ROOM );
               break;
          case 13:
               ch->pcdata->quest -= cost;
               obj = create_object( pObjIndex, 25 );
               obj->weight = 1;
               obj->cost   = 1000;
               obj->value[0] = 15;
               obj->item_type = ITEM_ACCESSORY;
               obj->wear_flags = ITEM_TAKE + ITEM_WEAR_WRIST;
               obj->quest=QUEST_NAME+QUEST_SHORT+QUEST_LONG;
               free_string( obj->short_descr );
               obj->short_descr = str_dup("special issue bracer");
               free_string( obj->name );
               obj->name = str_dup("special issue bracer");
               free_string( obj->description );
               obj->description = str_dup("a special issue bracer lies here");
               if (obj->questmaker != NULL) free_string(obj->questmaker);
               obj->questmaker = str_dup(ch->name);
               obj_to_char(obj,ch);
               act( "You reach up into the air and draw out a special issue bracer.", ch, obj, NULL, TO_CHAR );
               act( "$n reaches up into the air and draws out a special issue bracer.", ch, obj, NULL, TO_ROOM );
               break;
          case 14:
               ch->pcdata->quest -= cost;
               obj = create_object( pObjIndex, 25 );
               obj->weight = 1;
               obj->cost   = 1000;
               obj->value[0] = 15;
               obj->item_type = ITEM_ACCESSORY;
               obj->wear_flags = ITEM_TAKE + ITEM_WEAR_FACE;
               obj->quest=QUEST_NAME+QUEST_SHORT+QUEST_LONG;
               free_string( obj->short_descr );
               obj->short_descr = str_dup("special issue mask");
               free_string( obj->name );
               obj->name = str_dup("special issue mask");
               free_string( obj->description );
               obj->description = str_dup("a special issue mask lies here");
               if (obj->questmaker != NULL) free_string(obj->questmaker);
               obj->questmaker = str_dup(ch->name);
               obj_to_char(obj,ch);
               act( "You reach up into the air and draw out a special issue mask.", ch, obj, NULL, TO_CHAR );
               act( "$n reaches up into the air and draws out a special issue mask.", ch, obj, NULL, TO_ROOM );
               break;
          case 15:
          /* container */
               ch->pcdata->quest -= cost;
               obj = create_object( pObjIndex, 25 );
               obj->weight = 1;
               obj->cost   = 1000;
               obj->value[0] = 999;
               obj->item_type = ITEM_CONTAINER;
               obj->wear_flags = ITEM_TAKE;
               obj->quest=QUEST_NAME+QUEST_SHORT+QUEST_LONG;
               free_string( obj->short_descr );
               obj->short_descr = str_dup("a bag");
               free_string( obj->name );
               obj->name = str_dup("bag");
               free_string( obj->description );
               obj->description = str_dup("a bag lies here");
               if (obj->questmaker != NULL) free_string(obj->questmaker);
               obj->questmaker = str_dup(ch->name);
               obj_to_char(obj,ch);
               act( "You reach up into the air and draw out a bag.", ch, obj, NULL, TO_CHAR );
               act( "$n reaches up into the air and draws out a bag.", ch, obj, NULL, TO_ROOM );
               break;
          case 16:
          /* sword */
               ch->pcdata->quest -= cost;
               obj = create_object( pObjIndex, 25 );
               obj->weight = 1;
               obj->cost   = 1000;
               obj->value[0] = 0;   /* spell */
               obj->value[1] = 10;  /* min */
               obj->value[2] = 20;  /* max */
               obj->value[3] = 3;   /* slash */
               obj->item_type = ITEM_WEAPON;
               obj->wear_flags = ITEM_TAKE + ITEM_WIELD;
               obj->quest=QUEST_NAME+QUEST_SHORT+QUEST_LONG;
               free_string( obj->short_descr );
               obj->short_descr = str_dup("special issue sword");
               free_string( obj->name );
               obj->name = str_dup("special issue sword");
               free_string( obj->description );
               obj->description = str_dup("a special issue sword lies here");
               if (obj->questmaker != NULL) free_string(obj->questmaker);
               obj->questmaker = str_dup(ch->name);
               obj_to_char(obj,ch);
               act( "You reach up into the air and draw out a special issue sword.", ch, obj, NULL, TO_CHAR );
               act( "$n reaches up into the air and draws out a special issue sword.", ch, obj, NULL, TO_ROOM );
               break;
               // TODO: Implement new armor types and new weapon types

         }        
    }
    else
    {
       send_to_char("There is no such item I am afraid\n\r",ch);
    }

}
