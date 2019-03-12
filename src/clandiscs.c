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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "merc.h"

void  do_crush args( ( CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument ) );
void  do_brutality args( ( CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument ) );
void  do_might args( ( CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument ) );

void do_clandisc_message args((CHAR_DATA *ch, CHAR_DATA *victim, CLANDISC_DATA *disc));

/*
* Fortitude, Rank 5 - Repair the Undead Flesh - Heal yourself greatly (30%)
*/
void do_repair_undead_flesh(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
    char buf[MAX_INPUT_LENGTH];

    snprintf(buf, MAX_INPUT_LENGTH, "You are attempting to use the %s ability.\n\r", disc->name);
    send_to_char(buf, ch);
    return;

    if (IS_NPC(ch))
        return;

    if (!IS_SET(ch->act, PLR_VAMPIRE) || disc == NULL)
    {
        send_to_char("Only vampires who have reached rank 5 of Fortitude can use this ability.\n\r", ch);
        return;
    }

    if (ch->pcdata->condition[COND_THIRST] < 75)
    {
        send_to_char("You have insufficient blood.\n\r", ch);
        return;
    }

    // Let everyone know what's happening
    send_to_char( "You consume blood an excellerated rate to repair your undead flesh.\n\r", ch);
    snprintf(buf, MAX_INPUT_LENGTH, "$n's begins to repair their undead flesh.");
    act( buf, ch, NULL, NULL, TO_ROOM );

    // Maybe they'll get lucky and it'll cost less!
    ch->pcdata->condition[COND_THIRST] -= number_range(50, 75);

    // Heal them
    ch->hit += ch->max_hit * .3;

    return;
}

 /*
 * Fortitude, Rank 6 - Armored Flesh - Enemies that hit you deal damage to themselves (10%) --- Personal Armor - Damage resistance now 20% --- Resilient Minds now has additional resist (20%)
 */
void do_armored_flesh(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

 /*
 * Fortitude, Rank 7 - Arm of Prometheus - Limbs, eyes can no longer be removed.
 */
void do_arm_of_prometheus(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

 /*
 * Fortitude, Rank 8 - Stand Against All Foes - Gain increased dodge/parry/armor (5% for each enemy attacking you) Cannot exceed 80% dodge/parry with this skill.
 */
void do_stand_against_all_foes(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

 /*
 * Fortitude, Rank 9 - Shared Strength - Grant another your supernatural resistance (15% damage reduction) If they already have fortitude they get your full bonus.
 */
void do_shared_strength(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

/*
* Fortitude, Rank 10 - Eternal Vigilance - Take no damage from sunlight. Gain additional damage resistance (30% total)
*/
void do_eternal_vigilance(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_heightened_senses(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_aura_perception(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_prediction(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_clairvoyance(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_spirit_travel(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_the_mind_revealed(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_anima_gathering(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_ecstatic_agony(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument)
{

}

void do_psychic_assault(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_master_of_the_dominion(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_pact_with_animals(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    char *option = NULL;

    // TODO: Clean this up, I think the on/off message can have $n, $t etc for the act stuff

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

    if(!str_cmp(arg, ""))
    {
        send_to_char("Usage: pact <Wolf|Snake|Bird|Boar|Disolve>\n\r", ch);
        return;
    }

    if(!str_cmp(arg, "Disolve"))
    {
        if(disc->isActive)
        {
            snprintf(buf, MAX_INPUT_LENGTH, "You dissolve your pact with the %s.\n\r", disc->option);
            disc->personal_message_off = str_dup(buf);
            disc->option = "";
            do_clandisc_message(ch, NULL, disc);
            return;

        }
        else
        {
            send_to_char("You have no pact to disolve.\n\r", ch);
            return;
        }
        
    }

    if(str_cmp(arg, "Wolf") && str_cmp(arg, "Snake") && str_cmp(arg, "Bird") && str_cmp(arg, "Boar"))
    {
        send_to_char("Usage: pact <Wolf|Snake|Bird|Boar|Disolve>\n\r", ch);
        return;
    }

    if(str_cmp(disc->option, ""))
    {
        option = str_dup(disc->option);
    }

    if(!str_cmp(arg, "Wolf"))
    {
        disc->option = "Wolf";
    }

    if(!str_cmp(arg, "Snake"))
    {
        disc->option = "Snake";
    }

    if(!str_cmp(arg, "Bird"))
    {
        disc->option = "Bird";
    }

    if(!str_cmp(arg, "Boar"))
    {
        disc->option = "Boar";
    }

    if(option != NULL)
    {
        snprintf(buf, MAX_INPUT_LENGTH, "You dissolve your pact with the %s\n\r", option);
        disc->personal_message_off = str_dup(buf);
        do_clandisc_message(ch, NULL, disc);
    }

    snprintf(buf, MAX_INPUT_LENGTH, "You make a pact with the %s\n\r", disc->option);
    disc->personal_message_on = str_dup(buf);  

    snprintf(buf, MAX_INPUT_LENGTH, "You have a pact with the %s...upkeep %d.\n\r", disc->option, disc->bloodcost);
    disc->upkeepMessage = str_dup(buf);

    do_clandisc_message(ch, NULL, disc);

    return;
}

void do_beckoning(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_quell_the_beast(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];

    CHAR_DATA *victim;

    argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

    if(arg[0] == '\0')
    {
        send_to_char("Usage: quell <target>\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

    if(IS_NPC(victim))
    {
        if(victim->position != POS_FIGHTING)
        {
            send_to_char("They are not currently fighting!\n\r", ch);
            return;
        }

        if(number_percent() >= 40)
        {
            snprintf(buf, MAX_INPUT_LENGTH, "You quell %s.\n\r", victim->name);
            disc->personal_message_on = str_dup(buf);

            do_clandisc_message(ch, NULL, disc);
            
            do_flee(victim, "");

            WAIT_STATE(ch, 12);
            return;
        }
        else
        {
            snprintf(buf, MAX_INPUT_LENGTH, "You have failed to quell %s.\n\r", victim->name);
            disc->personal_message_on = str_dup(buf);

            do_clandisc_message(ch, NULL, disc);
            WAIT_STATE(ch, 12);
            return;
        }
    }
    else
    {
        if(!IS_SET(victim->act, PLR_VAMPIRE))
        {
            send_to_char("You can only do that to vampires!\n\r", ch);
            return;
        }

        snprintf(buf, MAX_INPUT_LENGTH, "You quell %s and lower their beast.\n\r", victim->name);
        disc->personal_message_on = str_dup(buf);

        snprintf(buf, MAX_INPUT_LENGTH, "$n quell's you and lowers your beast!\n\r");
        disc->victim_message = str_dup(buf);

        snprintf(buf, MAX_INPUT_LENGTH, "$n quell's $N and lowers their beast!\n\r");
        disc->room_message_on = str_dup(buf);

        victim->beast = victim->beast / 2;

        do_clandisc_message(ch, victim, disc);
        WAIT_STATE(ch, 12);
    }

    return;
}

void do_subsume_the_spirit(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument)
{
    char buf[MAX_INPUT_LENGTH];

    if (!IS_SET(ch->act, PLR_VAMPIRE) || disc == NULL)
    {
        send_to_char("You are unable to perform that action.\n\r", ch);
        return;
    }

    do_clandisc_message(ch, NULL, disc);

    snprintf(buf, MAX_INPUT_LENGTH, "You are in the form of a Wolf...upkeep %d.\n\r", disc->bloodcost);
    disc->upkeepMessage = str_dup(buf);

    return;
}

void do_drawing_out_the_beast(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_tainted_oasis(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_conquer_the_beast(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_taunt_the_caged_beast(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_unchain_the_ferocious_beast(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_free_the_beast_within(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_quickness(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_precision(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument)
{

}

void do_momentum(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_flawless_parry(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_stutter_step(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_flower_of_death(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_zephyr(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_paragon_of_motion(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_the_unseen_storm(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_between_the_ticks(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_shadow_play(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_shroud_of_night(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_arms_of_the_abyss(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_black_metamorphosis(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_shadowstep(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_the_darkness_within(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_inner_darkness(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_tchernabog(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_ahrimans_demesne(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_banishment(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_crush(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
    char buf[MAX_INPUT_LENGTH];

    if (!IS_SET(ch->act, PLR_VAMPIRE) || disc == NULL)
    {
        send_to_char("You are unable to perform that action.\n\r", ch);
        return;
    }

    // they have crush, because that's rank 1 and this is rank 7
    if((pdisc = GetPlayerDiscByTier(ch, POTENCE, 7)) != NULL && DiscIsActive(pdisc))
    {
        do_brutality(ch, pdisc, NULL);
    }

    // Check if the attack has Fist of Might of Heroes active, and it so, disable it
    if((pdisc = GetPlayerDiscByTier(ch, POTENCE, 9)) != NULL && DiscIsActive(pdisc))
    {
        do_might(ch, pdisc, NULL);
    }

    do_clandisc_message(ch, NULL, disc);

    snprintf(buf, MAX_INPUT_LENGTH, "Your strength causes you to crush your enemies...upkeep %d.\n\r", disc->bloodcost);
    disc->upkeepMessage = str_dup(buf);

    return;
}

void do_the_fist_of_lillith(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
    if (!IS_SET(ch->act, PLR_VAMPIRE) || disc == NULL)
    {
        send_to_char("You are unable to perform that action.\n\r", ch);
        return;
    }

    if( ch->vampgen <= 7 && ch->tier_clandisc[CLANDISC_POTENCE] >= 6) {
        disc->isPassiveAbility = TRUE;
        disc->bloodcost = 10;

        snprintf(buf, MAX_INPUT_LENGTH, "You are channeling the power of Lillith...upkeep %d.\n\r", disc->bloodcost);
        disc->upkeepMessage = str_dup(buf);
    }

    do_clandisc_message(ch, NULL, disc);

    return;
}

void do_earthshock(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_aftershock(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
    char buf[MAX_INPUT_LENGTH];

    if (!IS_SET(ch->act, PLR_VAMPIRE) || disc == NULL)
    {
        send_to_char("You are unable to perform that action.\n\r", ch);
        return;
    }

    do_clandisc_message(ch, NULL, disc);

    snprintf(buf, MAX_INPUT_LENGTH, "Your atershock provides damage absorption...upkeep %d.\n\r", disc->bloodcost);
    disc->upkeepMessage = str_dup(buf);

    return;
}

void do_the_forgers_hammer(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_fist_of_the_titans(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
    if (!IS_SET(ch->act, PLR_VAMPIRE) || disc == NULL)
    {
        send_to_char("You are unable to perform that action.\n\r", ch);
        return;
    }

    do_clandisc_message(ch, NULL, disc);

    return;
}

void do_brutality(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
    char buf[MAX_INPUT_LENGTH];
    CLANDISC_DATA * pdisc;

    if (!IS_SET(ch->act, PLR_VAMPIRE) || disc == NULL)
    {
        send_to_char("You are unable to perform that action.\n\r", ch);
        return;
    }

    // they have crush, because that's rank 1 and this is rank 7
    if((pdisc = GetPlayerDiscByTier(ch, POTENCE, 1)) != NULL && DiscIsActive(pdisc))
    {
        do_crush(ch, pdisc, NULL);
    }

    // Check if the attack has Fist of Might of Heroes active, and it so, disable it
    if((pdisc = GetPlayerDiscByTier(ch, POTENCE, 9)) != NULL && DiscIsActive(pdisc))
    {
        do_might(ch, pdisc, NULL);
    }

    do_clandisc_message(ch, NULL, disc);

    snprintf(buf, MAX_INPUT_LENGTH, "Your strength causes you to crush your enemies...upkeep %d.\n\r", disc->bloodcost);
    disc->upkeepMessage = str_dup(buf);

    return;
}

void do_lend_the_supernatural_vigor(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_might_of_the_heroes(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
    char buf[MAX_INPUT_LENGTH];
    CLANDISC_DATA * pdisc;

    if (!IS_SET(ch->act, PLR_VAMPIRE) || disc == NULL)
    {
        send_to_char("You are unable to perform that action.\n\r", ch);
        return;
    }

    // they have crush, because that's rank 1 and this is rank 7
    if((pdisc = GetPlayerDiscByTier(ch, POTENCE, 1)) != NULL && DiscIsActive(pdisc))
    {
        do_crush(ch, pdisc, NULL);
    }


    // Check if the attack has Fist of the Titans active, and it so, disable it
    if((pdisc = GetPlayerDiscByTier(ch, POTENCE, 7)) != NULL && DiscIsActive(pdisc))
    {
        do_brutality(ch, pdisc, NULL);
    }

    do_clandisc_message(ch, NULL, disc);

    snprintf(buf, MAX_INPUT_LENGTH, "Your strength causes you to crush your enemies...upkeep %d.\n\r", disc->bloodcost);
    disc->upkeepMessage = str_dup(buf);

    return;
}

void do_touch_of_pain(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_awe(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_dread_gaze(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_majesty(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_paralyzing_glance(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_presence_summon(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_bloodlust(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_phobia(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_stand_tall(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument)
{

}

void do_dream_world(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_pure_majesty(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_scorpions_touch(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_dagons_call(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_baals_caress(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_taste_of_death(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_erosion(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_selective_silence(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_blood_sweat(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_thin_blood(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_blood_of_destruction(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument)
{

}

void do_weaken_the_blood_of_the_ancients(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_geomancy(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
    char buf[MAX_INPUT_LENGTH];

    snprintf(buf, MAX_INPUT_LENGTH, "Your spin sparks with magical energy...upkeep %d.\n\r", disc->bloodcost);
    disc->upkeepMessage = str_dup(buf);

    do_clandisc_message(ch, NULL, disc);

    return;
}

void do_spark(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *vch;

	for(vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
		if( vch->fighting != ch) // yay they are fighting me
			return;
			
		if(number_percent() >= 80)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "Your skin sparks, connecting with %s and stunning them.\n\r", vch->name);
			disc->personal_message_on = str_dup(buf);
			
			if(!IS_NPC(vch))
			{
				snprintf(buf, MAX_INPUT_LENGTH, "$n's skin sparks, connecting with and stunning you.\n\r");
				disc->victim_message = str_dup(buf);
			}
			
			do_clandisc_message(ch, NULL, disc);
			vch->position = POS_STUNNED;
		} 
        else 
        {
		    snprintf(buf, MAX_INPUT_LENGTH, "Your spark fails to land on %s.\n\r", vch->name);
		    disc->personal_message_on = str_dup(buf);
			
			if(!IS_NPC(vch)) 
            {
    			snprintf(buf, MAX_INPUT_LENGTH, "$n skin sparks, but it does not connect with you.\n\r");
    			disc->victim_message = str_dup(buf);
			}
			
			do_clandisc_message(ch, NULL, disc);	
		}
	}
	
	WAIT_STATE(ch, 12);
	return;
}

void do_vertigo(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];

    CHAR_DATA *victim;

    argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

    if(arg[0] == '\0')
    {
        send_to_char("Usage: vertigo <target>\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if(IS_NPC(victim)) {
        send_to_char("Vertigo can only be used on other players.\n\r", ch);
        return;
    }

    // it landed
    if(number_percent() >= 80)
    {
        snprintf(buf, MAX_INPUT_LENGTH, "Your vertigo disorients %s.\n\r", victim->name);
        disc->personal_message_on = str_dup(buf);

        snprintf(buf, MAX_INPUT_LENGTH, "You feel disoriented. Maybe you need to rest.\n\r");
        disc->victim_message = str_dup(buf);

        do_clandisc_message(ch, NULL, disc);

        // force the fight to stop
        stop_fighting(victim, TRUE);

        // stun the victim
        victim->position = POS_STUNNED;

        // add lag to the caster
        WAIT_STATE(ch, 14);
        return;
    }
    else
    {
        snprintf(buf, MAX_INPUT_LENGTH, "Your vertigo has failed to influence %s.\n\r", victim->name);
        disc->personal_message_on = str_dup(buf);

        snprintf(buf, MAX_INPUT_LENGTH, "$n has tried to affect your mind, but you have resisted.\n\r");
        disc->victim_message = str_dup(buf);

        do_clandisc_message(ch, NULL, disc);
        WAIT_STATE(ch, 14);
        return;
    }
}

void do_contortion(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    int location;
    OBJ_DATA *obj;
    CHAR_DATA *victim;

    argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

    if(arg[0] == '\0')
    {
        send_to_char("Usage: contort <target>\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if(IS_NPC(victim)) {
        send_to_char("Contortion can only be used on other players.\n\r", ch);
        return;
    }

    // it landed
    if(number_percent() >= 66)
    {
        location = number_range(1, 4);

        // Modified the following code from fight.c to jack their limbs/gear

        if(location == 1)
        {
            // unlucky... they already lost that limb
            if (IS_ARM_L(victim, LOST_ARM)) {
                snprintf(buf, MAX_INPUT_LENGTH, "Your contortion was successful, but %s has already lost their left arm.\n\r", victim->name);
                send_to_char(buf, ch);
                return;
            }

            SET_BIT(victim->loc_hp[2], LOST_ARM);
            if (!IS_BLEEDING(victim, BLEEDING_ARM_L))
                SET_BIT(victim->loc_hp[6], BLEEDING_ARM_L);
            if (IS_BLEEDING(victim, BLEEDING_HAND_L))
                REMOVE_BIT(victim->loc_hp[6], BLEEDING_HAND_L);
            make_part(victim, "arm");
            if (IS_ARM_L(victim, LOST_ARM) && IS_ARM_R(victim, LOST_ARM))
            {
                if ((obj = get_eq_char(victim, WEAR_ARMS)) != NULL)
                    take_item(victim, obj);
            }
            if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
                take_item(victim, obj);
            if ((obj = get_eq_char(victim, WEAR_HANDS)) != NULL)
                take_item(victim, obj);
            if ((obj = get_eq_char(victim, WEAR_WRIST_L)) != NULL)
                take_item(victim, obj);
            if ((obj = get_eq_char(victim, WEAR_FINGER_L)) != NULL)
                take_item(victim, obj);

            snprintf(buf, MAX_INPUT_LENGTH, "You contort %s's limbs and rot away their left arm.\n\r", victim->name);
            disc->personal_message_on = str_dup(buf);

            snprintf(buf, MAX_INPUT_LENGTH, "$n contorts your body and it rots away your left arm.\n\r");
            disc->victim_message = str_dup(buf);

        }
        else if(location == 2)
        {
           // unlucky... they already lost that limb
           if (IS_ARM_R(victim, LOST_ARM)) {
               snprintf(buf, MAX_INPUT_LENGTH, "Your contortion was successful, but %s has already lost their right arm.\n\r", victim->name);
               send_to_char(buf, ch);
               return;
           }

           SET_BIT(victim->loc_hp[3], LOST_ARM);
           if (!IS_BLEEDING(victim, BLEEDING_ARM_R))
               SET_BIT(victim->loc_hp[6], BLEEDING_ARM_R);
           if (IS_BLEEDING(victim, BLEEDING_HAND_L))
               REMOVE_BIT(victim->loc_hp[6], BLEEDING_HAND_R);
           make_part(victim, "arm");
           if (IS_ARM_L(victim, LOST_ARM) && IS_ARM_R(victim, LOST_ARM))
           {
               if ((obj = get_eq_char(victim, WEAR_ARMS)) != NULL)
                   take_item(victim, obj);
           }
           if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
               take_item(victim, obj);
           if ((obj = get_eq_char(victim, WEAR_HANDS)) != NULL)
               take_item(victim, obj);
           if ((obj = get_eq_char(victim, WEAR_WRIST_R)) != NULL)
               take_item(victim, obj);
           if ((obj = get_eq_char(victim, WEAR_FINGER_R)) != NULL)
               take_item(victim, obj);

            snprintf(buf, MAX_INPUT_LENGTH, "You contort %s's limbs and rot away their right arm.", victim->name);
            disc->personal_message_on = str_dup(buf);

            snprintf(buf, MAX_INPUT_LENGTH, "$n contorts your body and it rots away your right arm.\n\r");
            disc->victim_message = str_dup(buf);
        }
        else if(location == 3)
        {
           // unlucky... they already lost that limb
           if (IS_LEG_R(victim, LOST_LEG)) {
               snprintf(buf, MAX_INPUT_LENGTH, "Your contortion was successful, but %s has already lost their right leg.\n\r", victim->name);
               send_to_char(buf, ch);
               return;
           }

            SET_BIT(victim->loc_hp[5], LOST_LEG);
            if (!IS_BLEEDING(victim, BLEEDING_LEG_R))
                SET_BIT(victim->loc_hp[6], BLEEDING_LEG_R);
            if (IS_BLEEDING(victim, BLEEDING_FOOT_R))
                REMOVE_BIT(victim->loc_hp[6], BLEEDING_FOOT_R);
            make_part(victim, "leg");
            if (IS_LEG_L(victim, LOST_LEG) && IS_LEG_R(victim, LOST_LEG))
            {
                if ((obj = get_eq_char(victim, WEAR_LEGS)) != NULL)
                    take_item(victim, obj);
            }
            if ((obj = get_eq_char(victim, WEAR_FEET)) != NULL)
                take_item(victim, obj);

            snprintf(buf, MAX_INPUT_LENGTH, "You contort %s's limbs and rot away their right leg.\n\r", victim->name);
            disc->personal_message_on = str_dup(buf);

            snprintf(buf, MAX_INPUT_LENGTH, "$n contorts your body and it rots away your right leg.\n\r");
            disc->victim_message = str_dup(buf);
        }
        else
        {
           // unlucky... they already lost that limb
           if (IS_ARM_R(victim, LOST_ARM)) {
               snprintf(buf, MAX_INPUT_LENGTH, "Your contortion was successful, but %s has already lost their left leg.\n\r", victim->name);
               send_to_char(buf, ch);
               return;
           }

            SET_BIT(victim->loc_hp[4], LOST_LEG);
            if (!IS_BLEEDING(victim, BLEEDING_LEG_L))
                SET_BIT(victim->loc_hp[6], BLEEDING_LEG_L);
            if (IS_BLEEDING(victim, BLEEDING_FOOT_L))
                REMOVE_BIT(victim->loc_hp[6], BLEEDING_FOOT_L);
            make_part(victim, "leg");
            if (IS_LEG_L(victim, LOST_LEG) && IS_LEG_R(victim, LOST_LEG))
            {
                if ((obj = get_eq_char(victim, WEAR_LEGS)) != NULL)
                    take_item(victim, obj);
            }
            if ((obj = get_eq_char(victim, WEAR_FEET)) != NULL)
                take_item(victim, obj);

            snprintf(buf, MAX_INPUT_LENGTH, "You contort %s's limbs and rot away their left leg.\n\r", victim->name);
            disc->personal_message_on = str_dup(buf);

            snprintf(buf, MAX_INPUT_LENGTH, "$n contorts your body and it rots away your left leg.\n\r");
            disc->victim_message = str_dup(buf);
        }

        do_clandisc_message(ch, NULL, disc);

        // force the fight to stop
        stop_fighting(victim, TRUE);

        // stun the victim
        victim->position = POS_STUNNED;

        // add lag to the caster
        WAIT_STATE(ch, 14);
        return;
    }
    else
    {
        snprintf(buf, MAX_INPUT_LENGTH, "Your contortion attempt has failed.\n\r");
        disc->personal_message_on = str_dup(buf);

        snprintf(buf, MAX_INPUT_LENGTH, "$n has tried to sever your limbs and failed.\n\r");
        disc->victim_message = str_dup(buf);

        do_clandisc_message(ch, NULL, disc);
        WAIT_STATE(ch, 12);
        return;
    }
}

void do_blood_boil(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
	char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
	int dmg;
	CHAR_DATA *victim;

    argument = one_argument(argument, arg, MAX_INPUT_LENGTH);
	
    if(arg[0] == '\0')
    {
        send_to_char("Usage: bloodboil <target>\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if(IS_NPC(victim)) {
        send_to_char("Bloodboil can only be used on other players.\n\r", ch);
        return;
    }
	
	if(is_safe(ch, victim)){
		return;
	}
	
	// Round 1 - FIGHT!
	set_fighting(ch, victim);
	set_fighting(victim, ch);
	
	// Set the damage right off the bat because for some reason this is always 10% no matter what
	dmg = victim->max_hit/10;
	
    // it landed
    if(number_percent() >= 60)
    {
		snprintf(buf, MAX_INPUT_LENGTH, "Your Bloodboil hits %s for %d damage!\n\r", victim->name, dmg);
        disc->personal_message_on = str_dup(buf);

        snprintf(buf, MAX_INPUT_LENGTH, "$n's Bloodboil hits you for %d damage!\n\r", dmg);
        disc->victim_message = str_dup(buf);
		
		victim->hit -= dmg;
    }
    else
    {
        snprintf(buf, MAX_INPUT_LENGTH, "Your Bloodboil attempt has failed.\n\r");
        disc->personal_message_on = str_dup(buf);

        snprintf(buf, MAX_INPUT_LENGTH, "$n has tried to boil your blood, but you resisted.\n\r");
        disc->victim_message = str_dup(buf);
	}

	do_clandisc_message(ch, NULL, disc);
	update_pos(victim);
	WAIT_STATE(ch, 12);
	return;
}

void do_runes_of_power(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_rout_the_charging_hordes(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_cognizant_construction(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_eye_of_the_storm(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_rego_ignem(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_malleable_visage(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    char *option = NULL;

    CHAR_DATA *victim;

    // TODO: Clean this up, I think the on/off message can have $n, $t etc for the act stuff

    argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

    if(!str_cmp(arg, ""))
    {
        send_to_char("Usage: visage <PlayerName|Disolve>\n\r", ch);
        return;
    }

    if(!str_cmp(arg, "Disolve"))
    {
        if(disc->isActive)
        {
            snprintf(buf, MAX_INPUT_LENGTH, "You dissolve your malleable vissage with the %s.\n\r", disc->option);
            disc->personal_message_off = str_dup(buf);
            disc->option = "";
            do_clandisc_message(ch, NULL, disc);
            return;

        }
        else
        {
            send_to_char("You have no malleable vissage to disolve.\n\r", ch);
            return;
        }
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if(IS_NPC(victim)) {
        send_to_char("You cannot use malleable vissage on a NPC.\n\r", ch);
        return;
    }

    if(str_cmp(disc->option, ""))
    {
        option = str_dup(disc->option);
    }

    if(victim != NULL)
    {
        disc->option = victim->name;
    }

    if(option != NULL)
    {
        snprintf(buf, MAX_INPUT_LENGTH, "You dissolve your malleable vissage with the %s\n\r", option);
        disc->personal_message_off = str_dup(buf);
        do_clandisc_message(ch, NULL, disc);
    }

    snprintf(buf, MAX_INPUT_LENGTH, "Your malleable visage turns you in to %s\n\r", disc->option);
    disc->personal_message_on = str_dup(buf);

    snprintf(buf, MAX_INPUT_LENGTH, "You have malleable visage has turned you in to %s...upkeep %d.\n\r", disc->option, disc->bloodcost);
    disc->upkeepMessage = str_dup(buf);

    do_clandisc_message(ch, NULL, disc);

    return;
}

void do_fleshcraft(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
    char buf[MAX_INPUT_LENGTH];

    snprintf(buf, MAX_INPUT_LENGTH, "Your flesh is crafted to grant you an additional arm...upkeep %d.\n\r", disc->bloodcost);
    disc->upkeepMessage = str_dup(buf);

    do_clandisc_message(ch, NULL, disc);

    return;
}

void do_bone_craft(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
    char buf[MAX_INPUT_LENGTH];

    snprintf(buf, MAX_INPUT_LENGTH, "You wrap your body in bone armor...upkeep %d.\n\r", disc->bloodcost);
    disc->upkeepMessage = str_dup(buf);

    do_clandisc_message(ch, NULL, disc);

    return;
}

void do_flesh_rot(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_INPUT_LENGTH];
    int location;
    int chance;
    OBJ_DATA *obj;
    CHAR_DATA *victim;

    argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

    if(arg[0] == '\0')
    {
        send_to_char("Usage: contort <target>\n\r", ch);
        return;
    }

    if ((victim = get_char_world(ch, arg)) == NULL)
    {
        send_to_char("They aren't here.\n\r", ch);
        return;
    }

    if(IS_NPC(victim)) {
        send_to_char("Flesh Rot can only be used on other players.\n\r", ch);
        return;
    }

    chance = 90;
    if(ch->vampgen > victim->vampgen) {
        chance -= (ch->vampgen - victim->vampgen) * 5;
    }

    // it landed
    if(number_percent() >= chance)
    {
        location = number_range(1, 4);

        // Modified the following code from fight.c to jack their limbs/gear

        if(location == 1)
        {
            // unlucky... they already lost that limb
            if (IS_ARM_L(victim, LOST_ARM)) {
                snprintf(buf, MAX_INPUT_LENGTH, "Your flesh rot was unable to remove the left arm of %s.\n\r", victim->name);
                send_to_char(buf, ch);
                return;
            }

            SET_BIT(victim->loc_hp[2], LOST_ARM);
            if (!IS_BLEEDING(victim, BLEEDING_ARM_L))
                SET_BIT(victim->loc_hp[6], BLEEDING_ARM_L);
            if (IS_BLEEDING(victim, BLEEDING_HAND_L))
                REMOVE_BIT(victim->loc_hp[6], BLEEDING_HAND_L);
            make_part(victim, "arm");
            if (IS_ARM_L(victim, LOST_ARM) && IS_ARM_R(victim, LOST_ARM))
            {
                if ((obj = get_eq_char(victim, WEAR_ARMS)) != NULL)
                    take_item(victim, obj);
            }
            if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
                take_item(victim, obj);
            if ((obj = get_eq_char(victim, WEAR_HANDS)) != NULL)
                take_item(victim, obj);
            if ((obj = get_eq_char(victim, WEAR_WRIST_L)) != NULL)
                take_item(victim, obj);
            if ((obj = get_eq_char(victim, WEAR_FINGER_L)) != NULL)
                take_item(victim, obj);

            snprintf(buf, MAX_INPUT_LENGTH, "You rot %s's flesh and they lose their left arms.\n\r", victim->name);
            disc->personal_message_on = str_dup(buf);

            snprintf(buf, MAX_INPUT_LENGTH, "$n rots your flesh and you lost your left arm.\n\r");
            disc->victim_message = str_dup(buf);

        }
        else if(location == 2)
        {
           // unlucky... they already lost that limb
           if (IS_ARM_R(victim, LOST_ARM)) {
               snprintf(buf, MAX_INPUT_LENGTH, "Your flesh rot was unable to remove the right arm of %s.\n\r", victim->name);
               send_to_char(buf, ch);
               return;
           }

           SET_BIT(victim->loc_hp[3], LOST_ARM);
           if (!IS_BLEEDING(victim, BLEEDING_ARM_R))
               SET_BIT(victim->loc_hp[6], BLEEDING_ARM_R);
           if (IS_BLEEDING(victim, BLEEDING_HAND_L))
               REMOVE_BIT(victim->loc_hp[6], BLEEDING_HAND_R);
           make_part(victim, "arm");
           if (IS_ARM_L(victim, LOST_ARM) && IS_ARM_R(victim, LOST_ARM))
           {
               if ((obj = get_eq_char(victim, WEAR_ARMS)) != NULL)
                   take_item(victim, obj);
           }
           if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
               take_item(victim, obj);
           if ((obj = get_eq_char(victim, WEAR_HANDS)) != NULL)
               take_item(victim, obj);
           if ((obj = get_eq_char(victim, WEAR_WRIST_R)) != NULL)
               take_item(victim, obj);
           if ((obj = get_eq_char(victim, WEAR_FINGER_R)) != NULL)
               take_item(victim, obj);

            snprintf(buf, MAX_INPUT_LENGTH, "You rot %s's flesh and they lose their right arms.", victim->name);
            disc->personal_message_on = str_dup(buf);

            snprintf(buf, MAX_INPUT_LENGTH, "$n rots your flesh and you lost your right arm.\n\r");
            disc->victim_message = str_dup(buf);
        }
        else if(location == 3)
        {
           // unlucky... they already lost that limb
           if (IS_LEG_R(victim, LOST_LEG)) {
               snprintf(buf, MAX_INPUT_LENGTH, "Your flesh rot was unable to remove the right leg of %s.\n\r", victim->name);
               send_to_char(buf, ch);
               return;
           }

            SET_BIT(victim->loc_hp[5], LOST_LEG);
            if (!IS_BLEEDING(victim, BLEEDING_LEG_R))
                SET_BIT(victim->loc_hp[6], BLEEDING_LEG_R);
            if (IS_BLEEDING(victim, BLEEDING_FOOT_R))
                REMOVE_BIT(victim->loc_hp[6], BLEEDING_FOOT_R);
            make_part(victim, "leg");
            if (IS_LEG_L(victim, LOST_LEG) && IS_LEG_R(victim, LOST_LEG))
            {
                if ((obj = get_eq_char(victim, WEAR_LEGS)) != NULL)
                    take_item(victim, obj);
            }
            if ((obj = get_eq_char(victim, WEAR_FEET)) != NULL)
                take_item(victim, obj);

            snprintf(buf, MAX_INPUT_LENGTH, "You rot %s's flesh and they lose their right leg.\n\r", victim->name);
            disc->personal_message_on = str_dup(buf);

            snprintf(buf, MAX_INPUT_LENGTH, "$n rots your flesh and you lost your right leg.\n\r");
            disc->victim_message = str_dup(buf);
        }
        else
        {
           // unlucky... they already lost that limb
           if (IS_ARM_R(victim, LOST_ARM)) {
               snprintf(buf, MAX_INPUT_LENGTH, "Your flesh rot was unable to remove the left leg of %s.\n\r", victim->name);
               send_to_char(buf, ch);
               return;
           }

            SET_BIT(victim->loc_hp[4], LOST_LEG);
            if (!IS_BLEEDING(victim, BLEEDING_LEG_L))
                SET_BIT(victim->loc_hp[6], BLEEDING_LEG_L);
            if (IS_BLEEDING(victim, BLEEDING_FOOT_L))
                REMOVE_BIT(victim->loc_hp[6], BLEEDING_FOOT_L);
            make_part(victim, "leg");
            if (IS_LEG_L(victim, LOST_LEG) && IS_LEG_R(victim, LOST_LEG))
            {
                if ((obj = get_eq_char(victim, WEAR_LEGS)) != NULL)
                    take_item(victim, obj);
            }
            if ((obj = get_eq_char(victim, WEAR_FEET)) != NULL)
                take_item(victim, obj);

            snprintf(buf, MAX_INPUT_LENGTH, "You rot %s's flesh and they lose their left leg.\n\r", victim->name);
            disc->personal_message_on = str_dup(buf);

            snprintf(buf, MAX_INPUT_LENGTH, "$n rots your flesh and you lost your left leg.\n\r");
            disc->victim_message = str_dup(buf);
        }

        do_clandisc_message(ch, NULL, disc);

        // force the fight to stop
        stop_fighting(victim, TRUE);

        // stun the victim
        victim->position = POS_STUNNED;

        // add lag to the caster
        WAIT_STATE(ch, 14);
        return;
    }
    else
    {
        snprintf(buf, MAX_INPUT_LENGTH, "Your flesh rot attempt has failed.\n\r");
        disc->personal_message_on = str_dup(buf);

        snprintf(buf, MAX_INPUT_LENGTH, "$n has tried to rot your flesh and failed.\n\r");
        disc->victim_message = str_dup(buf);

        do_clandisc_message(ch, NULL, disc);
        WAIT_STATE(ch, 12);
        return;
    }
}

void do_breath_of_the_dragon(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{
    int dmg;
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *vch;

	for(vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
		if( vch->fighting != ch) // yay they are fighting me
			return;

        if(is_safe(ch, vch)) // they are safe so don't attack
            return;

        // Round 1 - FIGHT!
        set_fighting(ch, vch);
        set_fighting(vch, ch);

        // Make sure this doesn't hit someone in their group ... because that's just fucking retarded if it does

        dmg = vch->max_hit * 0.15; // damage is 30% of their HP

        if(number_percent() >= 25) // 75% chance to hit... woot
        {
            snprintf(buf, MAX_INPUT_LENGTH, "Your dragon breath strikes %s for %d damage!.\n\r", vch->name, dmg);
            disc->personal_message_on = str_dup(buf);

            if(!IS_NPC(vch))
            {
                snprintf(buf, MAX_INPUT_LENGTH, "$n's dragon breath strikes you for %d damage!\n\r", dmg);
                disc->victim_message = str_dup(buf);
            }
        } else {
            snprintf(buf, MAX_INPUT_LENGTH, "Your dragon breath does not effect %s.\n\r", vch->name);
            disc->personal_message_on = str_dup(buf);

            if(!IS_NPC(vch)) {
                snprintf(buf, MAX_INPUT_LENGTH, "$n's dragon breath does not effect you.\n\r");
                disc->victim_message = str_dup(buf);
            }
		}

		do_clandisc_message(ch, NULL, disc);
	}

	WAIT_STATE(ch, 12);
	return;
}

void do_body_arsenal(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_impalers_fence(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_will_over_form(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_skin_trap(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_the_last_dracul(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_cloak_of_shadows(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_mask_of_a_thousand_faces(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_fade_from_the_minds_eye(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_the_silence_of_death(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_cloak_the_gathering(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_manifold_guise(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_fortress_of_silence(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_mental_maze(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_obscure_gods_creation(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_veil_of_blissful_ignorance(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_king_of_the_mountain(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_armor_of_kings(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_resilient_minds(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_personal_armor(CHAR_DATA *ch, CLANDISC_DATA *disc, char *argument) 
{

}

void do_clandisc_message(CHAR_DATA *ch, CHAR_DATA *victim, CLANDISC_DATA *disc) 
{
    char buf[MAX_INPUT_LENGTH];

    if (IS_NPC(ch))
        return;

    if (disc->isActive )
    {
        act(disc->personal_message_off, ch, NULL, NULL, TO_CHAR);
        
        if( str_cmp(disc->room_message_off, "" ))
        {
            snprintf(buf, MAX_INPUT_LENGTH, "%s", disc->room_message_off);
            act(buf, ch, NULL, NULL, TO_ROOM);
        }

        if(disc->isPassiveAbility) disc->isActive = FALSE;
        return;
    }
    
    act(disc->personal_message_on, ch, NULL, NULL, TO_CHAR);

    if( str_cmp(disc->room_message_on, "" ))
    {
        snprintf(buf, MAX_INPUT_LENGTH, "%s", disc->room_message_on);

        if(victim != NULL)
        {
            act(buf, ch, NULL, victim, TO_NOTVICT);
        }
        else
        {
            act(buf, ch, NULL, NULL, TO_ROOM);
        }
        
    }

    if( str_cmp(disc->victim_message, "" ))
    {
        if(victim != NULL)
        {
            snprintf(buf, MAX_INPUT_LENGTH, "%s", disc->victim_message);
            act(buf, ch, NULL, victim, TO_VICT);
        }
    }

    if(disc->isPassiveAbility) disc->isActive = TRUE;
    return;
}


CLANDISC_DATA *GetPlayerDiscByName(CHAR_DATA * ch, char * name) 
{
    CLANDISC_DATA *disc;

    for(disc = ch->clandisc; disc != NULL; disc = disc->next)
    {
        if( disc->name == name )
        {
	    
            return disc;
        
        }
    }

	return NULL;
}

CLANDISC_DATA *GetPlayerDiscByTier(CHAR_DATA *ch, char *clandisc, int tier)
{
    CLANDISC_DATA *disc;

    if(ch->clandisc == NULL) return NULL;

    for( disc = ch->clandisc; disc != NULL; disc = disc->next)
    {
        if(!str_cmp(disc->clandisc, clandisc) && disc->tier == tier)
        {
            return disc;
        }
    }

    return NULL;
}

bool DiscIsActive(CLANDISC_DATA *disc)
{
    if(disc == NULL) return false;

    return disc->isActive;
}

CLANDISC_DATA *get_disc_by_name(char * name) 
{
    int cmd;
    CLANDISC_DATA *disc;

    for ( cmd = 0; clandisc_table[cmd].name[0] != '\0'; cmd++ )
    {
        if ( name[0] == clandisc_table[cmd].name[0]
		  &&   !str_prefix( name, clandisc_table[cmd].name ) )
          {
              disc = alloc_perm(sizeof(*disc));
              disc->name = clandisc_table[cmd].name;
              disc->clandisc = clandisc_table[cmd].clandisc;
              disc->tier = clandisc_table[cmd].tier;
              disc->do_ability = clandisc_table[cmd].do_ability;
              disc->personal_message_on = clandisc_table[cmd].personal_message_on;
              disc->personal_message_off = clandisc_table[cmd].personal_message_off;
              disc->room_message_on = clandisc_table[cmd].room_message_on;
              disc->room_message_off = clandisc_table[cmd].room_message_off;
              disc->victim_message = clandisc_table[cmd].victim_message;
              disc->upkeepMessage = clandisc_table[cmd].upkeepMessage;
              disc->option = clandisc_table[cmd].option;
              disc->timeLeft = clandisc_table[cmd].timeLeft;
              disc->cooldown = clandisc_table[cmd].cooldown;
              disc->bloodcost = clandisc_table[cmd].bloodcost;
              disc->isActive = clandisc_table[cmd].isActive;
              disc->isPassiveAbility = clandisc_table[cmd].isPassiveAbility;
              disc->next = NULL;

              return disc;
          }
    }

	return NULL;
}