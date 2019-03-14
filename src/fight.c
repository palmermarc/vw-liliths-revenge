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

int dam2;

/*
* Local functions.
*/
bool check_dodge args((CHAR_DATA * ch, CHAR_DATA *victim));
void check_killer args((CHAR_DATA * ch, CHAR_DATA *victim));
bool check_parry args((CHAR_DATA * ch, CHAR_DATA *victim, int dt));
bool check_block args((CHAR_DATA * ch, CHAR_DATA *victim, int dt));
void dam_message args((CHAR_DATA * ch, CHAR_DATA *victim, int dam, int dt));
void death_cry args((CHAR_DATA * ch));
void group_gain args((CHAR_DATA * ch, CHAR_DATA *victim));
int xp_compute args((CHAR_DATA * gch, CHAR_DATA *victim));
void make_corpse args((CHAR_DATA * ch));
void one_hit args((CHAR_DATA * ch, CHAR_DATA *victim, int dt, int handtype));
void raw_kill args((CHAR_DATA * victim));
void disarm args((CHAR_DATA * ch, CHAR_DATA *victim));
void trip args((CHAR_DATA * ch, CHAR_DATA *victim));
void bash args((CHAR_DATA * ch, CHAR_DATA *victim));
void improve_wpn args((CHAR_DATA * ch, int dtype, bool right_hand));
void improve_stance args((CHAR_DATA * ch));
void skillstance args((CHAR_DATA * ch, CHAR_DATA *victim));
void show_spell args((CHAR_DATA * ch, int dtype));
void fightaction args((CHAR_DATA * ch, CHAR_DATA *victim, int actype, int dtype, int wpntype));
void crack_head args((CHAR_DATA * ch, OBJ_DATA *obj, char *argument));
void critical_hit args((CHAR_DATA * ch, CHAR_DATA *victim, int dt, int dam));
void take_item args((CHAR_DATA * ch, OBJ_DATA *obj));
void clear_stats args((CHAR_DATA * ch));

/*
* Control the fights going on.
* Called periodically by update_handler.
*/
void violence_update(void)
{
	CHAR_DATA *ch;
	CHAR_DATA *victim;
	CHAR_DATA *rch;
	CHAR_DATA *rch_next;

	for (ch = char_list; ch != NULL; ch = ch->next)
	{
		if ((victim = ch->fighting) == NULL || ch->in_room == NULL)
		{
			continue;
		}

		if (IS_AWAKE(ch) && ch->in_room == victim->in_room)
		{
			multi_hit(ch, victim, TYPE_UNDEFINED);
		}
		else
		{
			stop_fighting(ch, FALSE);
		}

		if ((victim = ch->fighting) == NULL)
		{
			continue;
		}

		/* Fun for the whole family! */

		for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
		{
			rch_next = rch->next_in_room;

			if (IS_AWAKE(rch) && rch->fighting == NULL)
			{

				/* Mount's auto-assist their riders and vice versa. */

				if (rch->mount != NULL && rch->mount == ch)
				{
					one_hit(rch, victim, TYPE_UNDEFINED, 0);
					continue;
				}

				/* PC's auto-assist others in their group. */

				if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
				{
					if ((!IS_NPC(rch) || IS_AFFECTED(rch, AFF_CHARM)) && is_same_group(ch, rch) && !IS_AFFECTED(rch, AFF_SHADOWPLANE))
						one_hit(rch, victim, TYPE_UNDEFINED, 0);
					continue;
				}

				/* NPC's assist NPC's of same type or 12.5% chance regardless. */

				if (IS_NPC(rch) && !IS_AFFECTED(rch, AFF_CHARM))
				{
					if (rch->pIndexData == ch->pIndexData || (number_bits(3) == 0 && IS_NPC(ch)))
					{
						CHAR_DATA *vch;
						CHAR_DATA *target;
						int number;

						target = NULL;
						number = 0;
						for (vch = ch->in_room->people; vch; vch = vch->next)
						{
							if (can_see(rch, vch) && is_same_group(vch, victim) && number_range(0, number) == 0)
							{
								target = vch;
								number++;
							}
						}

						if (target != NULL)
						{
							multi_hit(rch, target, TYPE_UNDEFINED);
						}
					}
				}
			}
		}
	}

	return;
}

/*
* Do one group of attacks.
*/
void multi_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
	OBJ_DATA *wieldR;
	OBJ_DATA *wieldL;
	int sn, level, hand, mobatt, l, throw;
	char buf[MAX_STRING_LENGTH];

	wieldR = get_eq_char(ch, WEAR_WIELD);
	wieldL = get_eq_char(ch, WEAR_HOLD);
	throw = 0;

	// If the player is attacking an NPC, autodrop them into their preferred stance
	if (!IS_NPC(ch) && IS_NPC(victim))
	{
		autodrop(ch);
	}

	// If a NPC attacks the player, autodrop them into their preferred stance
	if (IS_NPC(ch) && !IS_NPC(victim))
	{
		autodrop(victim);
	}

    CLANDISC_DATA * disc;
	if( !IS_NPC(ch) && (disc = GetPlayerDiscByTier(ch, FORTITUDE, FORTITUDE_KING_OF_THE_MOUNTAIN)) != NULL) // PCs that have King of the Mountain active cannot attack
    {
       if(DiscIsActive(disc))
       {
            send_to_char("You are unable to attack with King of the Mountain active.\n\r", ch);
            return;
       }
    }

	/* First, the mob attacks */
	if (IS_NPC(ch))
	{
		mobatt = ch->level / 25; /* one attack every 25 levels */
		if (mobatt > 5)
			mobatt = 5; /* limit them to 5 max */
		if (mobatt < 1)
			mobatt = 1; /* gotta let em have at least 1 :> */
		for (l = 0; l < mobatt; ++l)
		{
			hand = number_range(1, 2);
			one_hit(ch, victim, -1, hand);
		}
	}

	if (!IS_NPC(ch))
	{
		if (IS_AFFECTED(ch, AFF_POLYMORPH))
		{
			act("Your concentration is lost in combat, you return to human form.", ch, NULL, NULL, TO_CHAR);
			act("$n loses concentration in combat and returns to human form.", ch, NULL, NULL, TO_ROOM);
			clear_stats(ch);
			while (ch->affected)
			{
				affect_remove(ch, ch->affected);
			}

			REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
			REMOVE_BIT(ch->vampaff, VAM_CHANGED);
			REMOVE_BIT(ch->polyaff, POLY_MIST);
			REMOVE_BIT(ch->affected_by, AFF_ETHEREAL);
			free_string(ch->morph);
			ch->morph = str_dup("");

			return;
		}

		// Checking if victim is stunned, we are only going to attack them once
		if (victim->position == POS_STUNNED)
		{
			// Try to swing with the right hand first
			if (wieldR != NULL && IS_WEAPON(wieldR))
			{
				one_hit(ch, victim, -1, 1);

				if (wieldR->value[0] >= 1)
				{

					if (wieldR->value[0] >= 1000)
						sn = wieldR->value[0] - ((wieldR->value[0] / 1000) * 1000);
					else
						sn = wieldR->value[0];

					if (sn != 0 && victim->position == POS_FIGHTING)
						(*skill_table[sn].spell_fun)(sn, wieldR->level, ch, victim);
				}
				return;
			}

			// Didn't have a weapon in right hand, try left
			if (wieldL != NULL && IS_WEAPON(wieldL))
			{
				one_hit(ch, victim, -1, 2);

				if (wieldL->value[0] >= 1)
				{

					if (wieldL->value[0] >= 1000)
						sn = wieldL->value[0] - ((wieldL->value[0] / 1000) * 1000);
					else
						sn = wieldL->value[0];

					if (sn != 0 && victim->position == POS_FIGHTING)
						(*skill_table[sn].spell_fun)(sn, wieldL->level, ch, victim);
				}
				return;
			}

			// No weapons wielded, throw them hands
			hand = number_range(1, 2);
			one_hit(ch, victim, -1, hand);

			return;
		}

		// We will randomly throw a spell from one of the two weapons being wielded
		if ((wieldR != NULL && IS_WEAPON(wieldR)) &&
			(wieldL != NULL && IS_WEAPON(wieldL)))
		{
			throw = number_range(1, 2);
		}

		// Swing with the right hand
		if (wieldR != NULL && IS_WEAPON(wieldR))
		{
			one_hit(ch, victim, -1, 1);

			/* DO THE SPELL FOR SPELL THROWING WEAPONS */

			if (throw == 0)
			{
				throw = 1;
			}

			if (wieldR->value[0] >= 1 && throw == 1)
			{

				if (wieldR->value[0] >= 1000)
					sn = wieldR->value[0] - ((wieldR->value[0] / 1000) * 1000);
				else
					sn = wieldR->value[0];

				if (sn != 0 && victim->position == POS_FIGHTING)
					(*skill_table[sn].spell_fun)(sn, wieldR->level, ch, victim);
			}
		}

		// Swing with the left hand
		if (wieldL != NULL && IS_WEAPON(wieldL))
		{
			one_hit(ch, victim, -1, 2);

			/* DO THE SPELL FOR SPELL THROWING WEAPONS */

			if (throw == 0)
			{
				throw = 2;
			}

			if (wieldL->value[0] >= 1 && throw == 2)
			{
				if (wieldL->value[0] >= 1000)
					sn = wieldL->value[0] - ((wieldL->value[0] / 1000) * 1000);
				else
					sn = wieldL->value[0];

				if (sn != 0 && victim->position == POS_FIGHTING)
					(*skill_table[sn].spell_fun)(sn, wieldL->level, ch, victim);
			}
		}

		// Didn't find a weapon, throw them hands
		if ((wieldR == NULL || !IS_WEAPON(wieldR)) &&
			(wieldL == NULL || !IS_WEAPON(wieldL)))
		{
			hand = number_range(1, 2);
			one_hit(ch, victim, -1, hand);
		}

		// Time to check for stances and swing accordingly
		// This still sucks dick, but works for now

		if (ch->stance[CURRENT_STANCE] == STANCE_VIPER && number_percent() <= (ch->stance[STANCE_VIPER] / 2))
		{
			// Swing with a random hand
			hand = number_range(1, 2);
			one_hit(ch, victim, -1, hand);
		}

		if (ch->stance[CURRENT_STANCE] == STANCE_COBRA && number_percent() <= (ch->stance[STANCE_COBRA] / 2))
		{
			// Swing with a random hand
			hand = number_range(1, 2);
			one_hit(ch, victim, -1, hand);
		}

		if (ch->stance[CURRENT_STANCE] == STANCE_PANTHER && number_percent() <= (ch->stance[STANCE_PANTHER] / 2))
		{
			// Swing with a random hand
			hand = number_range(1, 2);
			one_hit(ch, victim, -1, hand);

			// Swing with a random hand
			hand = number_range(1, 2);
			one_hit(ch, victim, -1, hand);
		}

		if (ch->stance[CURRENT_STANCE] == STANCE_LION && number_percent() <= (ch->stance[STANCE_LION] / 2))
		{
			// Swing with a random hand
			hand = number_range(1, 2);
			one_hit(ch, victim, -1, hand);

			// Swing with a random hand
			hand = number_range(1, 2);
			one_hit(ch, victim, -1, hand);

			// Swing with a random hand
			hand = number_range(1, 2);
			one_hit(ch, victim, -1, hand);
		}

		// Time for vampire checks

		// Checking for Animalism T4 - Giving one extra attack currently
		if(DiscIsActive(GetPlayerDiscByTier(ch, ANIMALISM, ANIMALISM_SUBSUME_THE_SPIRIT)))
		{
			hand = number_range(1, 2);
			one_hit(ch, victim, -1, hand);
		}

		CLANDISC_DATA *disc = NULL;

		disc = GetPlayerDiscByTier(ch, ANIMALISM, ANIMALISM_PACT_WITH_ANIMALS);
		
		if(DiscIsActive(disc))
		{
			if(!str_cmp(disc->option, "Wolf"))
			{
				// TODO: Make some define for weapon types
				// 5 Is Claw attack, we need DEFINE's for these
				one_hit(ch, victim, (TYPE_HIT + 5), 0);
			}
		}

        disc = GetPlayerDiscByTier(ch, CELERITY, CELERITY_QUICKNESS);

        if(DiscIsActive(disc)) // quickness grants two attacks, always
        {
            hand = number_range(1, 2);
            one_hit(ch, victim, -1, hand);

            hand = number_range(1, 2);
            one_hit(ch, victim, -1, hand);
        }

        disc = GetPlayerDiscByTier(ch, CELERITY, CELERITY_ZEPHYR);
        if(disc != NULL && DiscIsActive(disc)) // zephyr grants two attacks, always
        {
            hand = number_range(1, 2);
            one_hit(ch, victim, -1, hand);

            hand = number_range(1, 2);
            one_hit(ch, victim, -1, hand);

            if(DiscIsActive(disc) && disc->option > 0)
                disc->option -= 1;

            if( DiscIsActive(disc) && disc->option == 0 )
                disc->isActive = FALSE;
        }


		// Fang attack
		if (IS_VAMPAFF(ch, VAM_FANGS))
		{
			one_hit(ch, victim, (TYPE_HIT + 10), 0);
		}

		disc = GetPlayerDiscByTier(ch, POTENCE, 2);
        // Check if the attack has Fist of Lillith active - Potence T2
        if((disc = GetPlayerDiscByTier(ch, POTENCE, 2)) != NULL)
        {
            // If they have rounds remaining, then remove a round so that
            if(DiscIsActive(disc) && disc->option > 0)
                disc->option -= 1;

            if( DiscIsActive(disc) && disc->option == 0 )
                disc->isActive = FALSE;
        }

        // Check if the attack has Fist of the Titans active - Potence T2
        disc = GetPlayerDiscByTier(ch, POTENCE, 6);
        if((disc = GetPlayerDiscByTier(ch, POTENCE, 6)) != NULL)
        {
            // If they have rounds remaining, then remove a round so that
            if(DiscIsActive(disc) && disc->option > 0)
                disc->option -= 1;

            if( DiscIsActive(disc) && disc->option == 0 )
                disc->isActive = FALSE;
        }

	}

	if (victim->position < 4)
		return;

	/* SPELL SHIELDS */

	if (victim->itemaffect < 1)
		return;
	if (IS_NPC(victim) || victim->spl[1] < 4)
		level = victim->level;
	else
		level = (victim->spl[1] / 4);

	/* switch off the spell attack spam if they want it */
	if (IS_SET(victim->act, PLR_FIGHT2))
		victim->choke_dam_message = 1;
	if (IS_SET(ch->act, PLR_FIGHT2))
		ch->choke_dam_message = 1;

	if (ch->choke_dam_message)
	{
		snprintf(buf, MAX_STRING_LENGTH, "$N's magical shields attack you!");
		ADD_COLOUR(ch, buf, BLUE, MAX_STRING_LENGTH);
		act(buf, ch, NULL, victim, TO_CHAR);
	}

	if (victim->choke_dam_message)
	{
		snprintf(buf, MAX_STRING_LENGTH, "Your magical shields attack $n!");
		ADD_COLOUR(victim, buf, LIGHTBLUE, MAX_STRING_LENGTH);
		act(buf, ch, NULL, victim, TO_VICT);
	}

	if (IS_ITEMAFF(victim, ITEMA_SHOCKSHIELD))
		if ((sn = skill_lookup("lightning bolt")) > 0)
			(*skill_table[sn].spell_fun)(sn, level, victim, ch);
	if (IS_ITEMAFF(victim, ITEMA_FIRESHIELD))
		if ((sn = skill_lookup("fireball")) > 0)
			(*skill_table[sn].spell_fun)(sn, level, victim, ch);
	if (IS_ITEMAFF(victim, ITEMA_ICESHIELD))
		if ((sn = skill_lookup("chill touch")) > 0)
			(*skill_table[sn].spell_fun)(sn, level, victim, ch);
	if (IS_ITEMAFF(victim, ITEMA_ACIDSHIELD))
		if ((sn = skill_lookup("acid blast")) > 0)
			(*skill_table[sn].spell_fun)(sn, level, victim, ch);

    // End of round removal for Momentum
    if((disc = GetPlayerDiscByTier(ch, CELERITY, CELERITY_MOMENTUM)) != NULL && DiscIsActive(disc))
    {
        if(DiscIsActive(disc) && disc->option > 0)
            disc->option -= 1;

        if( DiscIsActive(disc) && disc->option == 0 )
            disc->isActive = FALSE;
    }

	victim->choke_dam_message = 0;
	ch->choke_dam_message = 0;
}

/*
* Hit one guy once.
*/
void one_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt, int handtype)
{
	OBJ_DATA *wield;
	int ammount;
	int dam, diceroll, level;
	bool right_hand;

	/* Can't beat a dead char! */
	/* Guard against weird room-leavings. */

	if (victim->position == POS_DEAD || ch->in_room != victim->in_room)
		return;

	/* Figure out the type of damage message. */

	if (handtype == 2)
	{
		wield = get_eq_char(ch, WEAR_HOLD);
		right_hand = FALSE;
	}
	else
	{
		wield = get_eq_char(ch, WEAR_WIELD);
		right_hand = TRUE;
	}

	if (dt == TYPE_UNDEFINED)
	{
		dt = TYPE_HIT;
		if (wield != NULL && IS_WEAPON(wield))
			dt += wield->value[3];
	}

	if (ch->wpn[dt - 1000] > 5)
		level = (ch->wpn[dt - 1000] / 5);
	else
		level = 1;
	if (level > 40)
		level = 40;

	while ((diceroll = number_bits(5)) >= 20)
		;

	if (diceroll == 0)
	{
		/* Miss. */
		damage(ch, victim, 0, dt);
		tail_chain();
		improve_wpn(ch, dt, right_hand);
		improve_stance(ch);
		return;
	}

	/* Hit. */
	/* Calc damage. */
	/* Base Damage First */

	if (IS_NPC(ch))
	{
		// Right now we take a mobs level/2 for damage
		dam = ch->level / 2;
		if ((wield != NULL) && (IS_WEAPON(wield)))
			dam += number_range(wield->value[1], wield->value[2]);
	}
	else
	{
		if ((wield != NULL) && (IS_WEAPON(wield)))
			dam = number_range(wield->value[1], wield->value[2]);
		else
			dam = number_range(1, 4);
	}

	/* Store that base damage before calculating bonuses */

	dam += GET_DAMROLL(ch);
	dam2 = 0;

	/* Now calculate bonuses, to add to base dam later */

	// This place doesn't quite make sense to have bonus dmg, but we'll fall in line for now
	if (dt == gsn_backstab)
		dam2 += dam * number_range(2, 4);

	if (!IS_NPC(ch) && ch->stance[CURRENT_STANCE] == STANCE_BULL)
		dam2 += (dam * (ch->stance[STANCE_BULL] / 100));

	if (!IS_NPC(ch) && ch->stance[CURRENT_STANCE] == STANCE_MONGOOSE)
		dam2 += (dam * (ch->stance[STANCE_MONGOOSE] / 166.66));

	if (!IS_NPC(ch) && ch->stance[CURRENT_STANCE] == STANCE_GRIZZLIE)
		dam2 += (dam * (ch->stance[STANCE_GRIZZLIE] / 80));

	if (!IS_NPC(ch) && ch->stance[CURRENT_STANCE] == STANCE_LION)
		dam2 += (dam * (ch->stance[STANCE_LION] / 50));

	if (!IS_NPC(ch) && ch->stance[CURRENT_STANCE] == STANCE_FALCON)
		dam2 += (dam * (ch->stance[STANCE_FALCON] / 66.66));

	if (!IS_NPC(ch) && ch->stance[CURRENT_STANCE] == STANCE_COBRA)
		dam2 += (dam * (ch->stance[STANCE_COBRA] / 133.33));

	/* CHECK FOR POTENCE - ARCHON */

	// TODO: Rework/remove this

	if (!IS_NPC(ch) && IS_VAMPAFF(ch, VAM_POTENCE))
	{
		ammount = (get_age(ch) / 100);
		if (IS_VAMPPASS(ch, VAM_POTENCE) && ammount >= 7)
			ammount = 6;
		else if (ammount >= 6)
			ammount = 5;
		ammount *= 5;
		ammount += (13 - ch->vampgen);
		ammount += 10;
		dam2 += ((dam / 100) * ammount);
	}

	/* Vampires should be tougher at night and weaker during the day. */

	if (IS_SET(ch->act, PLR_VAMPIRE))
	{
		if (weather_info.sunlight == SUN_LIGHT)
			dam /= 2;
		else if (weather_info.sunlight == SUN_DARK)
			dam2 += dam * 0.5;
	}
	else if (!IS_NPC(ch))
	{
		if (weather_info.sunlight == SUN_DARK)
			dam /= 2;
	}

	/* if ( !IS_NPC(ch) && dt >= TYPE_HIT)           */
	/* dam = dam + (dam * (ch->wpn[dt-1000] / 100)); */

	if (dam <= 0)
		dam = 1;

	damage(ch, victim, dam, dt);
	tail_chain();
	improve_wpn(ch, dt, right_hand);
	improve_stance(ch);
	return;
}

/*
* Inflict damage from a hit.
*/
void damage(CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt)
{
	int ammount;
	const float bottom_dam = 0.8f; /* the damage modifier for zero beast */
	const float top_dam = 1.5f;	/* the damage modifier for 100 beast */
	const float power_base = powf(top_dam - bottom_dam + 1.0f, 1.f / 100.f);
	char buf[MAX_INPUT_LENGTH];
	CLANDISC_DATA * disc;

	if (victim->position == POS_DEAD)
		return;

	/* Stop up any residual loopholes. Taken out for now. */

	/* if ((!IS_NPC(victim)) && (( dam > 1000 ))) dam = 1000; */
	/* if ((IS_NPC(victim)) && (( dam > 1500 ))) dam = 1500;  */

	if (victim != ch)
	{

		/* Certain attacks are forbidden. */
		/* Most other attacks are returned. */

		if (is_safe(ch, victim))
			return;

		check_killer(ch, victim);

		if (victim->position > POS_STUNNED)
		{
			if (victim->fighting == NULL)
				set_fighting(victim, ch);
			victim->position = POS_FIGHTING;
		}

		if (victim->position > POS_STUNNED)
		{
			if (ch->fighting == NULL)
				set_fighting(ch, victim);

			/* If victim is charmed, ch might attack victim's master. */

			if (IS_NPC(ch) && IS_NPC(victim) && IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL && victim->master->in_room == ch->in_room && number_bits(3) == 0)
			{
				stop_fighting(ch, FALSE);
				multi_hit(ch, victim->master, TYPE_UNDEFINED);
				return;
			}
		}

		/* More charm stuff. */

		if (victim->master == ch)
			stop_follower(victim);

		/* Damage modifiers. */
		/* Carried on from one_hit, these are the non-physical */
		/* damage modifiers, that are included in spell damage */

		if (!IS_AWAKE(victim))
			dam2 += dam * 2;

		if (IS_AFFECTED(ch, AFF_HIDE))
		{
			if (!can_see(victim, ch))
			{
				dam2 += dam * 0.5;
				send_to_char("You use your concealment to get a surprise attack!\n\r", ch);
			}
			REMOVE_BIT(ch->affected_by, AFF_HIDE);
			act("$n leaps from $s concealment.", ch, NULL, NULL, TO_ROOM);
		}

		/* Now, before we do any reductions to damage for magical */
		/* defences, or vampire powers, we need to add the modifiers */
		/* to the base damage we stored earlier in one_hit */

		dam += dam2;

		// TODO: Rework/Remove the vampire stuff below and maybe sanct, etc.
		if (IS_AFFECTED(victim, AFF_SANCTUARY))
			dam /= 2;

		if (IS_AFFECTED(victim, AFF_PROTECT) && IS_EVIL(ch))
			dam -= dam / 4;

		if (dam < 0)
			dam = 0;

		/* beast - exponential curve, 0 beast = 80% damage, 100 beast = 150% damage */
		dam *= powf(power_base, ch->beast) + bottom_dam - 1.0f;

		/* Check for disarm, trip, parry, and dodge. */
		if (dt >= TYPE_HIT)
		{
			if (IS_NPC(ch) && number_percent() < ch->level / 2)
				disarm(ch, victim);

			if (IS_NPC(ch) && number_percent() < ch->level / 2)
				trip(ch, victim);

			if (check_block(ch, victim, dt))
				return;

			if (check_parry(ch, victim, dt))
				return;

			if (check_dodge(ch, victim))
				return;
		}

		// Armor damage mitigation
		dam -= GET_ARMOR(victim)/10;
	
		if (dam < 0)
			dam = 0;
	
		// TODO: Remove damage reduction information, it's for debugging only
		int beforeReduction = dam;
		bool damagedReduced = FALSE;
		// Not sure how well this will play out without dam being a float
		// All of these are damage reduction
		if (victim->stance[CURRENT_STANCE] == STANCE_MONGOOSE)
		{
			damagedReduced = TRUE;
			dam *= (100 - ((float)victim->stance[STANCE_MONGOOSE] / 66.66)) / 100;
		}
	
		if (victim->stance[CURRENT_STANCE] == STANCE_FALCON)
		{
			damagedReduced = TRUE;
			dam *= (100 - ((float)victim->stance[STANCE_FALCON] / 40)) / 100;
		}
	
		if (victim->stance[CURRENT_STANCE] == STANCE_SWALLOW)
		{
			damagedReduced = TRUE;
			dam *= (100 - ((float)victim->stance[STANCE_SWALLOW] / 20)) / 100;
		}
	
		if (victim->stance[CURRENT_STANCE] == STANCE_PANTHER)
		{
			damagedReduced = TRUE;
			dam *= (100 - ((float)victim->stance[STANCE_PANTHER] / 40)) / 100;
		}
	
		if (damagedReduced)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "Damage before reduction: %d, Damage after: %d ", beforeReduction, dam);
			send_to_char(buf, ch);
		}
		// Except this guy
	
		if (victim->stance[CURRENT_STANCE] == STANCE_LION)
		{
			dam *= 1.1;
		}
	
		// Check if the attack has Subsume the spirit - Animalism T4
		if(DiscIsActive(GetPlayerDiscByTier(ch, ANIMALISM, ANIMALISM_SUBSUME_THE_SPIRIT)))
		{
			dam *= 1.1;
		}
	
		// Check if the attack has Crush active- Potence T1
    	if(DiscIsActive(GetPlayerDiscByTier(ch, POTENCE, 1)))
    	{
    	    dam *= 1.1;
    	}
	
    	// Check if the attack has Fist of Lillith active - Potence T2
    	if((disc = GetPlayerDiscByTier(ch, POTENCE, 2)) != NULL)
    	{
    	    if((DiscIsActive(disc) && disc->option > 0) || (ch->vampgen <= 7 && ch->tier_clandisc[CLANDISC_POTENCE] >= 6))
    	        dam *= 1.1;
    	}
	
    	// Check if the attack has Fist of the Titans active - Potence T2
    	if((disc = GetPlayerDiscByTier(ch, POTENCE, 6)) != NULL)
    	{
    	    if(DiscIsActive(disc) && disc->option > 0)
    	       dam *= 1.05;
    	}
	
    	disc = GetPlayerDiscByTier(ch, CELERITY, CELERITY_MOMENTUM);
    	if((disc = GetPlayerDiscByTier(ch, CELERITY, CELERITY_MOMENTUM)) != NULL && DiscIsActive(disc)) // Momentum gives a damage boost for two rounds, so drop this down at the bottom to make sure they get their full damage boost
    	{
    	    dam *= 1.10;
    	}
	
		// Animalism T1 - Snake
		// TODO: Add Posion for Pact here
	
	
    	// If the caster has Geomancy active, give them an additional 10% damage
		if(DiscIsActive(GetPlayerDiscByTier(ch, THAUMATURGY, 1)) && dt < 1000)
    	{
    	    dam *= 1.1;
    	}

        if(DiscIsActive(GetPlayerDiscByTier(victim, OBTENEBRATION, OBTENEBRATION_SHADOW_PLAY)) && IS_SET(ch->in_room->room_flags, ROOM_DARK))
        {
            dam *= 1.1;
        }



        /**
         * Now calculate the damage reductions from clandiscs
         */
	
    	// If the victim has Geomancy active, reduce the damage by 10%
    	if(DiscIsActive(GetPlayerDiscByTier(victim, THAUMATURGY, 1)) && dt < 1000)
    	{
    	    dam *= 0.9;
    	}

    	// Check to see if the victim has Aftershock - 15% damage reduction
    	if(DiscIsActive(GetPlayerDiscByTier(victim, POTENCE, 4)))
    	{
    	    dam *= 0.85;
    	}
	
    	// Check to see if the victim has Personal Armor - 10% damage reduction
    	if(DiscIsActive(GetPlayerDiscByTier(victim, FORTITUDE, 1)))
    	{
    	    dam *= 0.9;
    	}
    	else if(DiscIsActive(GetPlayerDiscByTier(victim, FORTITUDE, 9)))
    	{
    	    dam *= 0.8;
    	}
    	else if(DiscIsActive(GetPlayerDiscByTier(victim, FORTITUDE, 10)))
    	{
    	    dam *= 0.7;
    	}

		dam_message(ch, victim, dam, dt);

		// This is where Black Metamorphasis reflects the damage
		if(DiscIsActive(GetPlayerDiscByTier(victim, OBTENEBRATION, OBTENEBRATION_BLACK_METAMORPHOSIS)) && dt < 1000)
        {
            ch->hit -= dam/10;
            snprintf(buf, MAX_INPUT_LENGTH, "%s's Black Metamorphosis strikes you for %d Shadow Damage!", victim->name, dam/10 );
            send_to_char(buf, ch);

            snprintf(buf, MAX_INPUT_LENGTH, "Your Black Metamorphosis strikes %s for %d Shadow Damage!", ch->name, dam/10 );
            send_to_char(buf, victim);
        }
	}

	/* Hurt the victim. */
	/* Inform the victim of his new state. */

	victim->hit -= dam;

	/* SPECIAL STUFF, for immune mobs etc. */

	if (IS_NPC(victim) && (!str_cmp(victim->name, "Enkil") || !str_cmp(victim->name, "gatekeeper")) && victim->hit < victim->max_hit)
		victim->hit = victim->max_hit;

	if (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL && victim->hit < 1)
		victim->hit = 1;

	update_pos(victim);

	switch (victim->position)
	{
	case POS_MORTAL:
		act("$n is mortally wounded, and spraying blood everywhere.", victim, NULL, NULL, TO_ROOM);
		send_to_char("You are mortally wounded, and spraying blood everywhere.\n\r", victim);
		break;

	case POS_INCAP:
		act("$n is incapacitated, and bleeding badly.", victim, NULL, NULL, TO_ROOM);
		send_to_char("You are incapacitated, and bleeding badly.\n\r", victim);
		break;

	case POS_STUNNED:
		act("$n is stunned, but will soon recover.", victim, NULL, NULL, TO_ROOM);
		send_to_char("You are stunned, but will soon recover.\n\r", victim);
		break;

	case POS_DEAD:
		act("$n is DEAD!!", victim, 0, 0, TO_ROOM);
		send_to_char("You have been KILLED!!\n\r\n\r", victim);
		break;

	default:

		if (dam > victim->max_hit / 4)
			send_to_char("That really did HURT!\n\r", victim);
		if (victim->hit < victim->max_hit / 4 && dam > 0)
		{
			/* if (!IS_NPC(victim) && IS_SET(victim->act, PLR_VAMPIRE) */
			/* && number_percent() < victim->beast)                    */
			/* do_rage(victim,"");                                     */
			/* else                                                    */

			send_to_char("You sure are BLEEDING!\n\r", victim);
		}
		break;
	}

	/* Sleep spells and extremely wounded folks. */

	if (!IS_AWAKE(victim))
		stop_fighting(victim, FALSE);

	/* Payoff for killing things. */

	if (victim->position == POS_DEAD)
	{
		group_gain(ch, victim);

		if (!IS_NPC(victim))
		{
			snprintf(log_buf, MAX_INPUT_LENGTH * 2, "%s killed by %s at %ld",
					 victim->name,
					 (IS_NPC(ch) ? ch->short_descr : ch->name),
					 victim->in_room->vnum);
			log_string(log_buf);

			/* Dying penalty: */
			/* 1/2 your current exp. */

			if (victim->exp > 0)
				victim->exp = victim->exp / 2;

			/* Palmer add clan death here !! */

			if (!str_cmp(victim->clan, "Assamite"))
				clan_infotable[1].mkilled++;
			else if (!str_cmp(victim->clan, "Tzimisce"))
				clan_infotable[2].mkilled++;
			else if (!str_cmp(victim->clan, "Ventrue"))
				clan_infotable[3].mkilled++;
			else if (!str_cmp(victim->clan, "Tremere"))
				clan_infotable[4].mkilled++;
			else if (!str_cmp(victim->clan, "Lasombra"))
				clan_infotable[5].mkilled++;
			else if (!str_cmp(victim->clan, "Toreador"))
				clan_infotable[6].mkilled++;
		}

		if (IS_NPC(victim) && !IS_NPC(ch))
		{
			ch->mkill += 1;
			if (!str_cmp(ch->clan, "Assamite"))
				clan_infotable[1].mkills++;
			else if (!str_cmp(ch->clan, "Tzimisce"))
				clan_infotable[2].mkills++;
			else if (!str_cmp(ch->clan, "Ventrue"))
				clan_infotable[3].mkills++;
			else if (!str_cmp(ch->clan, "Tremere"))
				clan_infotable[4].mkills++;
			else if (!str_cmp(ch->clan, "Lasombra"))
				clan_infotable[5].mkills++;
			else if (!str_cmp(ch->clan, "Toreador"))
				clan_infotable[6].mkills++;

			if (ch->level == 1 && ch->mkill > 4)
			{
				ch->level = 2;
				do_save(ch, "");
			}
		}

		if (!IS_NPC(victim) && IS_NPC(ch))
			victim->mdeath = victim->mdeath + 1;

		raw_kill(victim);

		if (IS_SET(ch->act, PLR_AUTOGOLD))
		{
			do_get(ch, "gold corpse");
		}
		else if (IS_SET(ch->act, PLR_AUTOLOOT))
		{
			do_get(ch, "all corpse");
		}
		else
			do_look(ch, "in corpse");


		if (!IS_NPC(ch) && IS_NPC(victim))
		{
			if (IS_SET(ch->act, PLR_AUTOSAC))
				do_sacrifice(ch, "corpse");
		}

		return;
	}

	if (victim == ch)
		return;

	/* Take care of link dead people. */

	if (!IS_NPC(victim) && victim->desc == NULL)
	{
		if (number_range(0, victim->wait) == 0)
		{
			if (IS_BODY(victim, TIED_UP))
			{
				send_to_char("Not while tied up.\n\r", victim);
				if (victim->position > POS_STUNNED)
					act("$n strains against $s bonds.", victim, NULL, NULL, TO_ROOM);
			}
			else
				do_recall(victim, "");
			return;
		}
	}

	/* Wimp out? */

	if (IS_NPC(victim) && dam > 0)
	{
		if ((IS_SET(victim->act, ACT_WIMPY) && number_bits(1) == 0 && victim->hit < victim->max_hit / 2) || (IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL && victim->master->in_room != victim->in_room))
			do_flee(victim, "");
	}

	if (!IS_NPC(victim) && victim->race < 10 && victim->hit > 0 && victim->hit <= victim->wimpy && victim->wait == 0)
		do_flee(victim, "");

	tail_chain();
	return;
}

bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim)
{
	/* Ethereal people can only attack other ethereal people
if ( IS_AFFECTED(ch, AFF_ETHEREAL) && !IS_AFFECTED(victim, AFF_ETHEREAL) )
{
send_to_char( "You cannot while ethereal.\n\r", ch );
return TRUE;
}
if ( !IS_AFFECTED(ch, AFF_ETHEREAL) && IS_AFFECTED(victim, AFF_ETHEREAL) )
{
send_to_char( "You cannot fight an ethereal person.\n\r", ch );
return TRUE;
}
    */
	/* You cannot attack across planes */
	if (IS_AFFECTED(ch, AFF_SHADOWPLANE) && !IS_AFFECTED(victim, AFF_SHADOWPLANE))
	{
		act("You are too insubstantial!", ch, NULL, victim, TO_CHAR);
		return TRUE;
	}
	if (!IS_AFFECTED(ch, AFF_SHADOWPLANE) && IS_AFFECTED(victim, AFF_SHADOWPLANE))
	{
		act("$E is too insubstantial!", ch, NULL, victim, TO_CHAR);
		return TRUE;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char("You cannot fight in a safe room.\n\r", ch);
		return TRUE;
	}

	if (IS_HEAD(ch, LOST_HEAD) || IS_EXTRA(ch, EXTRA_OSWITCH))
	{
		send_to_char("Objects cannot fight!\n\r", ch);
		return TRUE;
	}
	else if (IS_HEAD(victim, LOST_HEAD) || IS_EXTRA(victim, EXTRA_OSWITCH))
	{
		send_to_char("You cannot attack an object.\n\r", ch);
		return TRUE;
	}

	/* Palmer added test */
	if (!IS_NPC(victim))
	{
		if (victim->fighting != NULL && !IS_NPC(victim->fighting) && victim->fighting != ch)
		{
			send_to_char("They are somewhat preoccupied!\n\r", ch);
			return TRUE;
		}
	}

	if (IS_NPC(victim) && (victim->pIndexData->pShop != NULL))
	{
		send_to_char("Didn't your mummy tell you never to steal from shops?\n\r", ch);
		return TRUE;
	}

	if (IS_NPC(ch) || IS_NPC(victim))
		return FALSE;

	/* Thx Josh! */
	if (victim->fighting == ch)
		return FALSE;

	if (victim->level != 3 || ch->level != 3)
	{
		send_to_char("Both players must be avatars to fight.\n\r", ch);
		return TRUE;
	}

	return FALSE;
}

bool no_attack(CHAR_DATA *ch, CHAR_DATA *victim)
{
	/* Ethereal people can only attack other ethereal people */
	if (IS_AFFECTED(ch, AFF_ETHEREAL) && !IS_AFFECTED(victim, AFF_ETHEREAL))
	{
		send_to_char("You cannot while ethereal.\n\r", ch);
		return TRUE;
	}
	if (!IS_AFFECTED(ch, AFF_ETHEREAL) && IS_AFFECTED(victim, AFF_ETHEREAL))
	{
		send_to_char("You cannot fight an ethereal person.\n\r", ch);
		return TRUE;
	}

	/* You cannot attack across planes */
	if (IS_AFFECTED(ch, AFF_SHADOWPLANE) && !IS_AFFECTED(victim, AFF_SHADOWPLANE))
	{
		act("You are too insubstantial!", ch, NULL, victim, TO_CHAR);
		return TRUE;
	}
	if (!IS_AFFECTED(ch, AFF_SHADOWPLANE) && IS_AFFECTED(victim, AFF_SHADOWPLANE))
	{
		act("$E is too insubstantial!", ch, NULL, victim, TO_CHAR);
		return TRUE;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char("You cannot fight in a safe room.\n\r", ch);
		return TRUE;
	}

	if (IS_HEAD(ch, LOST_HEAD) || IS_EXTRA(ch, EXTRA_OSWITCH))
	{
		send_to_char("Objects cannot fight!\n\r", ch);
		return TRUE;
	}
	else if (IS_HEAD(victim, LOST_HEAD) || IS_EXTRA(victim, EXTRA_OSWITCH))
	{
		send_to_char("You cannot attack objects.\n\r", ch);
		return TRUE;
	}

	return FALSE;
}

/*
* See if an attack justifies a KILLER flag.
*/
void check_killer(CHAR_DATA *ch, CHAR_DATA *victim)
{
	/*
* Follow charm thread to responsible character.
* Attacking someone's charmed char is hostile!
    */
	while (IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL)
		victim = victim->master;

	/*
	   * NPC's are fair game.
	   * So are killers and thieves.
    */
	if (IS_NPC(victim) || IS_SET(victim->act, PLR_KILLER) || IS_SET(victim->act, PLR_THIEF))
		return;

	/*
	   * Charm-o-rama.
    */
	if (IS_SET(ch->affected_by, AFF_CHARM))
	{
		if (ch->master == NULL)
		{
			/*
	   char buf[MAX_STRING_LENGTH];
	   sprintf( buf, "Check_killer: %s bad AFF_CHARM",
	   IS_NPC(ch) ? ch->short_descr : ch->name );
	   bug( buf, 0 );
		  */
			affect_strip(ch, gsn_charm_person);
			REMOVE_BIT(ch->affected_by, AFF_CHARM);
			return;
		}
		stop_follower(ch);
		return;
	}
	return;
}

bool check_block(CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
	return FALSE;
}

bool check_parry(CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
	OBJ_DATA *obj;
	int chance;
	char buf[MAX_INPUT_LENGTH];
	char buf1[MAX_INPUT_LENGTH];
	char buf7[MAX_INPUT_LENGTH];
	char buf8[MAX_INPUT_LENGTH];

	if (!IS_AWAKE(victim))
		return FALSE;

	if (IS_NPC(victim))
	{
		chance = victim->level;
		obj = NULL;
	}

	else
	{
		if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL && IS_WEAPON(obj))
		{
			chance = victim->wpn[obj->value[3]] / 8;
		}

		else if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL && IS_WEAPON(obj))
		{
			chance = victim->wpn[obj->value[3]] / 8;
		}

		else
			return FALSE;
	}

	if (!IS_NPC(victim) && (victim->stance[CURRENT_STANCE] == STANCE_CRANE) &&
		victim->stance[STANCE_CRANE] > 0)
	{
		chance += victim->stance[STANCE_CRANE] / 5;
	}

	if (!IS_NPC(victim) && (victim->stance[CURRENT_STANCE] == STANCE_SWALLOW) &&
		victim->stance[STANCE_SWALLOW] > 0)
	{
		chance += victim->stance[STANCE_SWALLOW] / 3;
	}

	if (!IS_NPC(victim) && (ch->stance[CURRENT_STANCE] == STANCE_COBRA) &&
		victim->stance[STANCE_COBRA] > 0)
	{
		chance += victim->stance[STANCE_COBRA] / 3;
	}

	if (!IS_NPC(victim) && (ch->stance[CURRENT_STANCE] == STANCE_GRIZZLIE) &&
		victim->stance[STANCE_GRIZZLIE] > 0)
	{
		chance += victim->stance[STANCE_GRIZZLIE] / 3;
	}

	if (!IS_NPC(victim) && (ch->stance[CURRENT_STANCE] == STANCE_LION))
	{
		chance -= 5;
	}

	if (!IS_NPC(victim) && (ch->stance[CURRENT_STANCE] == STANCE_PANTHER))
	{
		chance -= 5;
	}

	if (!IS_NPC(victim) && (ch->stance[CURRENT_STANCE] == STANCE_FALCON))
	{
		chance -= 5;
	}

	if (!IS_NPC(victim) &&
		victim->stance[STANCE_SWALLOW] == 200 &&
		victim->stance[STANCE_COBRA] == 200 &&
		victim->stance[STANCE_PANTHER] == 200 &&
		victim->stance[STANCE_GRIZZLIE] == 200 &&
		victim->stance[STANCE_LION] == 200 &&
		victim->stance[STANCE_FALCON] == 200)
		chance += 10;

	if (chance > 85)
		chance = 85;

	if (ch->max_move > 5000)
		chance = chance + (ch->max_move / 1000);

	if (chance > 95)
		chance = 95;

    CLANDISC_DATA * disc;
    if((disc = GetPlayerDiscByTier(ch, FORTITUDE, FORTITUDE_KING_OF_THE_MOUNTAIN)) != NULL)
    {
        if(DiscIsActive(disc))
        {
            chance = 95;
        }
    }

	if (IS_NPC(ch) && (ch->hitroll >= victim->hitroll))
		chance = chance - ((ch->hitroll - victim->hitroll) / 30);

	if (!IS_NPC(ch) && (ch->hitroll >= victim->hitroll))
		chance = chance - (((ch->hitroll - victim->hitroll) / 5) * 2);

	if (number_percent() >= chance)
		return FALSE;

	if (!IS_NPC(ch) && obj != NULL && IS_WEAPON(obj) &&
		obj->value[3] >= 0 && obj->value[3] <= 12)
	{
		if (!IS_SET(victim->act, PLR_FIGHT))
		{
			snprintf(buf, MAX_INPUT_LENGTH, "You parry $n's blow with $p.");
			ADD_COLOUR(victim, buf, LIGHTGREEN, MAX_INPUT_LENGTH);
			act(buf, ch, obj, victim, TO_VICT);
		}
		if (!IS_SET(ch->act, PLR_FIGHT))
		{
			snprintf(buf1, MAX_INPUT_LENGTH, "$N parries your blow with $p.");
			ADD_COLOUR(ch, buf1, LIGHTGREEN, MAX_INPUT_LENGTH);
			act(buf1, ch, obj, victim, TO_CHAR);
		}
		return TRUE;
	}
	if (!IS_SET(victim->act, PLR_FIGHT))
	{
		snprintf(buf7, MAX_INPUT_LENGTH, "You parry $n's attack.");
		ADD_COLOUR(victim, buf7, LIGHTGREEN, MAX_INPUT_LENGTH);
		act(buf7, ch, NULL, victim, TO_VICT);
	}
	if (!IS_SET(ch->act, PLR_FIGHT))
	{
		snprintf(buf8, MAX_INPUT_LENGTH, "$N parries your attack.");
		ADD_COLOUR(ch, buf8, LIGHTGREEN, MAX_INPUT_LENGTH);
		act(buf8, ch, NULL, victim, TO_CHAR);
	}
	return TRUE;
}

/*
* Check for dodge.
*/
bool check_dodge(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int chance;
	int dodge1;
	int dodge2;
	char buf2[MAX_INPUT_LENGTH];
	char buf3[MAX_INPUT_LENGTH];

	if (!IS_AWAKE(victim))
		return FALSE;

	if (IS_NPC(victim))
		chance = victim->level/10;
	else
		chance = (victim->wpn[0] / 8);

	dodge1 = victim->carry_weight;
	dodge2 = can_carry_w(victim);

	if (!IS_NPC(victim) && (victim->stance[CURRENT_STANCE] == STANCE_CRANE) &&
		victim->stance[STANCE_CRANE] > 0)
	{
		chance += victim->stance[STANCE_CRANE] / 8;
	}

	if (!IS_NPC(victim) && (victim->stance[CURRENT_STANCE] == STANCE_SWALLOW) &&
		victim->stance[STANCE_SWALLOW] > 0)
		chance += victim->stance[STANCE_SWALLOW] / 6;

	if (!IS_NPC(victim) && (victim->stance[CURRENT_STANCE] == STANCE_COBRA) &&
		victim->stance[STANCE_COBRA] > 0)
		chance += victim->stance[STANCE_COBRA] / 6;

	if (!IS_NPC(victim) && (victim->stance[CURRENT_STANCE] == STANCE_FALCON) &&
		victim->stance[STANCE_FALCON] > 0)
		chance += victim->stance[STANCE_FALCON] / 6;

	if (ch->stance[CURRENT_STANCE] == STANCE_LION)
	{
		chance -= 5;
	}

	if (!IS_NPC(victim) &&
		victim->stance[STANCE_SWALLOW] == 200 &&
		victim->stance[STANCE_COBRA] == 200 &&
		victim->stance[STANCE_PANTHER] == 200 &&
		victim->stance[STANCE_GRIZZLIE] == 200 &&
		victim->stance[STANCE_LION] == 200 &&
		victim->stance[STANCE_FALCON] == 200)
		chance += 10;

	if (chance > 85)
		chance = 85;

	if (ch->max_move > 5000)
		chance = chance + (ch->max_move / 1000);

	if (chance > 95)
		chance = 95;

	CLANDISC_DATA * disc;
	if((disc = GetPlayerDiscByTier(ch, FORTITUDE, FORTITUDE_KING_OF_THE_MOUNTAIN)) != NULL)
	{
	    if(DiscIsActive(disc))
	    {
	        chance = 95;
	    }
	}

	if (dodge2 < 1)
		return FALSE;
	else if ((dodge1 < 1) && (number_percent() >= chance))
		return FALSE;
	else if (dodge1 > 0 && number_percent() >= chance - (dodge1 * 140 / dodge2))
		return FALSE;

	if (!IS_SET(victim->act, PLR_FIGHT))
	{
		snprintf(buf2, MAX_INPUT_LENGTH, "You dodge $n's attack.");
		ADD_COLOUR(victim, buf2, GREEN, MAX_INPUT_LENGTH);
		act(buf2, ch, NULL, victim, TO_VICT);
	}
	if (!IS_SET(ch->act, PLR_FIGHT))
	{
		snprintf(buf3, MAX_INPUT_LENGTH, "$N dodges your attack.");
		ADD_COLOUR(ch, buf3, GREEN, MAX_INPUT_LENGTH);
		act(buf3, ch, NULL, victim, TO_CHAR);
	}
	return TRUE;
}

/*
* Set position of a victim.
*/
void update_pos(CHAR_DATA *victim)
{
	CHAR_DATA *mount;

	if (victim->hit > 0)
	{
		if (victim->position <= POS_STUNNED)
			victim->position = POS_STANDING;
		return;
	}
	else if ((mount = victim->mount) != NULL)
	{
		if (victim->mounted == IS_MOUNT)
		{
			act("$n rolls off $N.", mount, NULL, victim, TO_ROOM);
			act("You roll off $N.", mount, NULL, victim, TO_CHAR);
		}
		else if (victim->mounted == IS_RIDING)
		{
			act("$n falls off $N.", victim, NULL, mount, TO_ROOM);
			act("You fall off $N.", victim, NULL, mount, TO_CHAR);
		}
		mount->mount = NULL;
		victim->mount = NULL;
		mount->mounted = IS_ON_FOOT;
		victim->mounted = IS_ON_FOOT;
	}

	if (!IS_NPC(victim) && victim->hit <= -11 && IS_HERO(victim))
	{
		victim->hit = -10;
		stop_fighting(victim, TRUE);
		return;
	}

	if (IS_NPC(victim) || victim->hit <= -11)
	{
		victim->position = POS_DEAD;
		return;
	}

	if (victim->hit <= -6)
		victim->position = POS_MORTAL;
	else if (victim->hit <= -3)
		victim->position = POS_INCAP;
	else
		victim->position = POS_STUNNED;

	return;
}

/*
* Start fights.
*/
void set_fighting(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (ch->fighting != NULL)
	{
		bug("Set_fighting: already fighting", 0);
		return;
	}

	if (IS_AFFECTED(ch, AFF_SLEEP))
		affect_strip(ch, gsn_sleep);

	ch->fighting = victim;
	ch->position = POS_FIGHTING;

	return;
}

/*
* Stop fights.
*/
void stop_fighting(CHAR_DATA *ch, bool fBoth)
{
	CHAR_DATA *fch;

	for (fch = char_list; fch != NULL; fch = fch->next)
	{
		if (fch == ch || (fBoth && fch->fighting == ch))
		{
			fch->fighting = NULL;
			fch->position = POS_STANDING;
			update_pos(fch);
		}
	}

	return;
}

/*
* Make a corpse out of a character.
*/
void make_corpse(CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *corpse;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	char *name;

	if (IS_NPC(ch))
	{
		name = ch->short_descr;
		corpse = create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
		corpse->timer = number_range(4, 8);
		if (ch->gold > 0)
		{
			obj_to_obj(create_money(ch->gold), corpse);
			ch->gold = 0;
		}
	}
	else
	{
		name = ch->name;
		corpse = create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
		corpse->timer = number_range(25, 40);
		/* Why should players keep their gold? */
		if (ch->gold > 0)
		{
			obj = create_money(ch->gold);
			if (IS_AFFECTED(ch, AFF_SHADOWPLANE) &&
				(!IS_SET(obj->extra_flags, ITEM_SHADOWPLANE)))
				SET_BIT(obj->extra_flags, ITEM_SHADOWPLANE);
			obj_to_obj(obj, corpse);
			ch->gold = 0;
		}
	}

	snprintf(buf, MAX_STRING_LENGTH, corpse->short_descr, name);
	free_string(corpse->short_descr);
	corpse->short_descr = str_dup(buf);

	snprintf(buf, MAX_STRING_LENGTH, corpse->description, name);
	free_string(corpse->description);
	corpse->description = str_dup(buf);

	for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{

		obj_next = obj->next_content;

		obj_from_char(obj);

		if (IS_NPC(ch) && obj->questowner != NULL && strlen(obj->questowner) > 1)
		{
			obj_to_room(obj, ch->in_room);
		}
		else
		{
			if (IS_SET(obj->extra_flags, ITEM_VANISH))
				extract_obj(obj);
			else
			{
				if (IS_AFFECTED(ch, AFF_SHADOWPLANE) &&
					(!IS_SET(obj->extra_flags, ITEM_SHADOWPLANE)))
					SET_BIT(obj->extra_flags, ITEM_SHADOWPLANE);
				obj_to_obj(obj, corpse);
			}
		}
	}

	/* If you die in the shadowplane, your corpse should stay there */
	if (IS_AFFECTED(ch, AFF_SHADOWPLANE))
		SET_BIT(corpse->extra_flags, ITEM_SHADOWPLANE);

	obj_to_room(corpse, ch->in_room);
	return;
}

/*
* Improved Death_cry contributed by Diavolo.
*/
void death_cry(CHAR_DATA *ch)
{
	ROOM_INDEX_DATA *was_in_room;
	char *msg;
	int door;
	int vnum;

	vnum = 0;
	switch (number_bits(4))
	{
	/* Took out cut off bits, etc.  Will include as part of the final
    * Death moves.  KaVir.
	   */
	default:
		msg = "$n hits the ground ... DEAD.";
		break;
	}

	act(msg, ch, NULL, NULL, TO_ROOM);

	if (vnum != 0)
	{
		char buf[MAX_STRING_LENGTH];
		OBJ_DATA *obj;
		char *name;

		name = IS_NPC(ch) ? ch->short_descr : ch->name;
		obj = create_object(get_obj_index(vnum), 0);
		obj->timer = number_range(4, 7);

		snprintf(buf, MAX_STRING_LENGTH, obj->short_descr, name);
		free_string(obj->short_descr);
		obj->short_descr = str_dup(buf);

		snprintf(buf, MAX_STRING_LENGTH, obj->description, name);
		free_string(obj->description);
		obj->description = str_dup(buf);

		obj_to_room(obj, ch->in_room);
	}

	if (IS_NPC(ch))
		msg = "You hear something's death cry.";
	else
		msg = "You hear someone's death cry.";

	was_in_room = ch->in_room;
	for (door = 0; door <= 5; door++)
	{
		EXIT_DATA *pexit;

		if ((pexit = was_in_room->exit[door]) != NULL && pexit->to_room != NULL && pexit->to_room != was_in_room)
		{
			ch->in_room = pexit->to_room;
			act(msg, ch, NULL, NULL, TO_ROOM);
		}
	}
	ch->in_room = was_in_room;
	return;
}

void make_part(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int vnum;

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);
	vnum = 0;

	if (arg[0] == '\0')
		return;
	if (!str_cmp(arg, "head"))
		vnum = OBJ_VNUM_SEVERED_HEAD;
	else if (!str_cmp(arg, "arm"))
		vnum = OBJ_VNUM_SLICED_ARM;
	else if (!str_cmp(arg, "leg"))
		vnum = OBJ_VNUM_SLICED_LEG;
	else if (!str_cmp(arg, "heart"))
		vnum = OBJ_VNUM_TORN_HEART;
	else if (!str_cmp(arg, "turd"))
		vnum = OBJ_VNUM_TORN_HEART;
	else if (!str_cmp(arg, "entrails"))
		vnum = OBJ_VNUM_SPILLED_ENTRAILS;
	else if (!str_cmp(arg, "brain"))
		vnum = OBJ_VNUM_QUIVERING_BRAIN;
	else if (!str_cmp(arg, "eyeball"))
		vnum = OBJ_VNUM_SQUIDGY_EYEBALL;
	else if (!str_cmp(arg, "blood"))
		vnum = OBJ_VNUM_SPILT_BLOOD;
	else if (!str_cmp(arg, "face"))
		vnum = OBJ_VNUM_RIPPED_FACE;
	else if (!str_cmp(arg, "windpipe"))
		vnum = OBJ_VNUM_TORN_WINDPIPE;
	else if (!str_cmp(arg, "cracked_head"))
		vnum = OBJ_VNUM_CRACKED_HEAD;
	else if (!str_cmp(arg, "ear"))
		vnum = OBJ_VNUM_SLICED_EAR;
	else if (!str_cmp(arg, "nose"))
		vnum = OBJ_VNUM_SLICED_NOSE;
	else if (!str_cmp(arg, "tooth"))
		vnum = OBJ_VNUM_KNOCKED_TOOTH;
	else if (!str_cmp(arg, "tongue"))
		vnum = OBJ_VNUM_TORN_TONGUE;
	else if (!str_cmp(arg, "hand"))
		vnum = OBJ_VNUM_SEVERED_HAND;
	else if (!str_cmp(arg, "foot"))
		vnum = OBJ_VNUM_SEVERED_FOOT;
	else if (!str_cmp(arg, "thumb"))
		vnum = OBJ_VNUM_SEVERED_THUMB;
	else if (!str_cmp(arg, "index"))
		vnum = OBJ_VNUM_SEVERED_INDEX;
	else if (!str_cmp(arg, "middle"))
		vnum = OBJ_VNUM_SEVERED_MIDDLE;
	else if (!str_cmp(arg, "ring"))
		vnum = OBJ_VNUM_SEVERED_RING;
	else if (!str_cmp(arg, "little"))
		vnum = OBJ_VNUM_SEVERED_LITTLE;
	else if (!str_cmp(arg, "toe"))
		vnum = OBJ_VNUM_SEVERED_TOE;

	if (vnum != 0)
	{
		char buf[MAX_STRING_LENGTH];
		OBJ_DATA *obj;
		char *name;

		name = IS_NPC(ch) ? ch->short_descr : ch->name;
		obj = create_object(get_obj_index(vnum), 0);
		if (IS_NPC(ch))
			obj->timer = number_range(2, 5);
		else
			obj->timer = -1;
		if (!str_cmp(arg, "head") && IS_NPC(ch))
			obj->value[1] = ch->pIndexData->vnum;
		else if (!str_cmp(arg, "head") && !IS_NPC(ch))
		{
			ch->pcdata->chobj = obj;
			obj->chobj = ch;
			obj->timer = number_range(2, 3);
		}
		else if (!str_cmp(arg, "brain") && !IS_NPC(ch) &&
				 IS_AFFECTED(ch, AFF_POLYMORPH) && IS_HEAD(ch, LOST_HEAD))
		{
			if (ch->pcdata->chobj != NULL)
				ch->pcdata->chobj->chobj = NULL;
			ch->pcdata->chobj = obj;
			obj->chobj = ch;
			obj->timer = number_range(2, 3);
		}

		/* For blood :) KaVir */
		if (vnum == OBJ_VNUM_SPILT_BLOOD)
			obj->timer = 2;

		/* For voodoo dolls - KaVir */
		if (!IS_NPC(ch))
		{
			snprintf(buf, MAX_INPUT_LENGTH, obj->name, name);
			free_string(obj->name);
			obj->name = str_dup(buf);
		}
		else
		{
			snprintf(buf, MAX_INPUT_LENGTH, obj->name, "mob");
			free_string(obj->name);
			obj->name = str_dup(buf);
		}

		snprintf(buf, MAX_INPUT_LENGTH, obj->short_descr, name);
		free_string(obj->short_descr);
		obj->short_descr = str_dup(buf);

		snprintf(buf, MAX_INPUT_LENGTH, obj->description, name);
		free_string(obj->description);
		obj->description = str_dup(buf);

		if (IS_AFFECTED(ch, AFF_SHADOWPLANE))
			SET_BIT(obj->extra_flags, ITEM_SHADOWPLANE);

		obj_to_room(obj, ch->in_room);
	}
	return;
}

void raw_kill(CHAR_DATA *victim)
{
	CHAR_DATA *mount;
	stop_fighting(victim, TRUE);
	death_cry(victim);
	make_corpse(victim);

	if ((mount = victim->mount) != NULL)
	{
		if (victim->mounted == IS_MOUNT)
		{
			act("$n rolls off the corpse of $N.", mount, NULL, victim, TO_ROOM);
			act("You roll off the corpse of $N.", mount, NULL, victim, TO_CHAR);
		}
		else if (victim->mounted == IS_RIDING)
		{
			act("$n falls off $N.", victim, NULL, mount, TO_ROOM);
			act("You fall off $N.", victim, NULL, mount, TO_CHAR);
		}
		mount->mount = NULL;
		victim->mount = NULL;
		mount->mounted = IS_ON_FOOT;
		victim->mounted = IS_ON_FOOT;
	}

	if (IS_NPC(victim))
	{
		log_string(victim->pIndexData->player_name);
		victim->pIndexData->killed++;
		extract_char(victim, TRUE);
		return;
	}

	extract_char(victim, FALSE);
	while (victim->affected)
		affect_remove(victim, victim->affected);
	if (IS_AFFECTED(victim, AFF_POLYMORPH) && IS_AFFECTED(victim, AFF_ETHEREAL))
	{
		victim->affected_by = AFF_POLYMORPH + AFF_ETHEREAL;
	}
	else if (IS_AFFECTED(victim, AFF_POLYMORPH))
		victim->affected_by = AFF_POLYMORPH;
	else if (IS_AFFECTED(victim, AFF_ETHEREAL))
		victim->affected_by = AFF_ETHEREAL;
	else
		victim->affected_by = 0;
	if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_STAKE))
		REMOVE_BIT(victim->immune, IMM_STAKE);
	victim->itemaffect = 0;
	victim->loc_hp[0] = 0;
	victim->loc_hp[1] = 0;
	victim->loc_hp[2] = 0;
	victim->loc_hp[3] = 0;
	victim->loc_hp[4] = 0;
	victim->loc_hp[5] = 0;
	victim->loc_hp[6] = 0;
	victim->armor = 0;
	victim->position = POS_RESTING;
	victim->hit = UMAX(1, victim->hit);
	victim->mana = UMAX(1, victim->mana);
	victim->move = UMAX(1, victim->move);
	victim->hitroll = 0;
	victim->damroll = 0;
	victim->saving_throw = 0;
	victim->carry_weight = 0;
	victim->carry_number = 0;
	save_char_obj(victim);
	return;
}

void behead(CHAR_DATA *victim)
{
	char buf[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;

	if (IS_NPC(victim))
		return;

	location = get_room_index(victim->in_room->vnum);

	stop_fighting(victim, TRUE);

	make_part(victim, "head");

	make_corpse(victim);

	extract_char(victim, FALSE);
	while (victim->affected)
		affect_remove(victim, victim->affected);
	if (IS_AFFECTED(victim, AFF_POLYMORPH) && IS_AFFECTED(victim, AFF_ETHEREAL))
	{
		victim->affected_by = AFF_POLYMORPH + AFF_ETHEREAL;
	}
	else if (IS_AFFECTED(victim, AFF_POLYMORPH))
		victim->affected_by = AFF_POLYMORPH;
	else if (IS_AFFECTED(victim, AFF_ETHEREAL))
		victim->affected_by = AFF_ETHEREAL;
	else
		victim->affected_by = 0;
	if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_STAKE))
		REMOVE_BIT(victim->immune, IMM_STAKE);
	victim->itemaffect = 0;
	victim->loc_hp[0] = 0;
	victim->loc_hp[1] = 0;
	victim->loc_hp[2] = 0;
	victim->loc_hp[3] = 0;
	victim->loc_hp[4] = 0;
	victim->loc_hp[5] = 0;
	victim->loc_hp[6] = 0;
	victim->affected_by = 0;
	victim->armor = 0;
	victim->position = POS_STANDING;
	victim->hit = 1;
	victim->mana = UMAX(1, victim->mana);
	victim->move = UMAX(1, victim->move);
	victim->hitroll = 0;
	victim->damroll = 0;
	victim->saving_throw = 0;
	victim->carry_weight = 0;
	victim->carry_number = 0;

	char_from_room(victim);
	char_to_room(victim, location);
	SET_BIT(victim->loc_hp[0], LOST_HEAD);
	SET_BIT(victim->affected_by, AFF_POLYMORPH);
	snprintf(buf, MAX_INPUT_LENGTH, "the severed head of %s", victim->name);
	free_string(victim->morph);
	victim->morph = str_dup(buf);

	save_char_obj(victim);
	return;
}

void group_gain(CHAR_DATA *ch, CHAR_DATA *victim)
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *gch;
	int xp;
	int members;
    int tierpoints;

	/*
    * Monsters don't get kill xp's or alignment changes.
    * P-killing doesn't help either.
    * Dying of mortal wounds or poison doesn't give xp to anyone!
    */
	if ((IS_NPC(ch) && ch->mount == NULL) || victim == ch)
		return;

	/* The gatekeeper shouldn't ever die, but just in case... */
	/* crash caused by this, remming out for the time being */
	/*    if ( victim->pIndexData->vnum == 24001 )
    {
         snprintf(buf, MAX_STRING_LENGTH, "Log: **CHEAT** : %s just killed the gatekeeper!", ch->name);
         log_string( buf );
    } */
	members = 0;
	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
	{
		if (is_same_group(gch, ch))
			members++;
	}

	if (members == 0)
	{
		bug("Group_gain: members.", members);
		members = 1;
	}

	for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
	{
		OBJ_DATA *obj;
		OBJ_DATA *obj_next;

		if (!is_same_group(gch, ch))
			continue;

		xp = xp_compute(gch, victim) / members;
		snprintf(buf, MAX_STRING_LENGTH, "You receive %d experience points.\n\r", xp);
		send_to_char(buf, gch);
		
		
		
		if (gch->mount != NULL)
			send_to_char(buf, gch->mount);
		
		gain_exp(gch, xp);
        
        tierpoints = ch->max_hit / 1000;
        snprintf(buf, MAX_STRING_LENGTH, "#GYou receive %d blood points.\n\r", tierpoints);
        ch->tierpoints += 1;

		for (obj = ch->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			if (obj->wear_loc == WEAR_NONE)
				continue;

			if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch)) || (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch)) || (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch)))
			{
				act("You are zapped by $p.", ch, obj, NULL, TO_CHAR);
				act("$n is zapped by $p.", ch, obj, NULL, TO_ROOM);
				obj_from_char(obj);
				obj_to_room(obj, ch->in_room);
			}
		}

		for (obj = ch->carrying; obj != NULL; obj = obj_next)
		{
			obj_next = obj->next_content;
			if (obj->wear_loc == WEAR_NONE)
				continue;

			if ((strncmp(ch->clan, "Assamite", 4) && (obj->pIndexData->vnum == 24900 || obj->pIndexData->vnum == 24901 || obj->pIndexData->vnum == 24902)) || (strncmp(ch->clan, "Tzimisce", 4) && (obj->pIndexData->vnum == 24903 || obj->pIndexData->vnum == 24904 || obj->pIndexData->vnum == 24905)) || (strncmp(ch->clan, "Tremere", 4) && (obj->pIndexData->vnum == 24906 || obj->pIndexData->vnum == 24907 || obj->pIndexData->vnum == 24908)) || (strncmp(ch->clan, "Toreador", 4) && (obj->pIndexData->vnum == 24909 || obj->pIndexData->vnum == 24910 || obj->pIndexData->vnum == 24911)) || (strncmp(ch->clan, "Ventrue", 4) && (obj->pIndexData->vnum == 24912 || obj->pIndexData->vnum == 24913 || obj->pIndexData->vnum == 24914)) || (strncmp(ch->clan, "Lasombra", 4) && (obj->pIndexData->vnum == 24918 || obj->pIndexData->vnum == 24919 || obj->pIndexData->vnum == 24920)) || (strncmp(ch->clan, "Cappadocian", 4) && (obj->pIndexData->vnum == 24929 || obj->pIndexData->vnum == 24930 || obj->pIndexData->vnum == 24931)))

			{
				act("You are zapped by $p.", ch, obj, NULL, TO_CHAR);
				act("$n is zapped by $p.", ch, obj, NULL, TO_ROOM);
				obj_from_char(obj);
				obj_to_room(obj, ch->in_room);
			}
		}
	}

	return;
}

/*
* Compute xp for a kill.
* Also adjust alignment of killer.
* Edit this function to change xp computations.
*/
int xp_compute(CHAR_DATA *gch, CHAR_DATA *victim)
{
	int exp_spent;
	int exp;
	int level, lev;
	int bonus;
	int tierpoints;
	long cap;
	const float top = 1.0f;
	const float shift_up = 0.3f;
	const float std_dev = 350.0f;
	const float scale = top / (std_dev * 2.0f * 3.1415926f);
	char buf[MAX_STRING_LENGTH];

	if (gch->exp > 50000000)
	{
		send_to_char("Nobody likes a hoarder.. Spend some damn EXP!\n\r", gch);
		return 0;
	}

	/* calculate a millionth of how much exp they've spent so far */
	exp_spent = ((long)gch->max_hit * gch->max_hit + (long)gch->max_mana * gch->max_mana + (long)gch->max_move * gch->max_move) / 2000000;
	if (exp_spent > 1350)
		exp_spent = 1350;
	else if (exp_spent < 0)
		exp_spent = 0;

	/* randomise the victim level +- 2% and assign to exp  */
	level = victim->exp_level / 50;
	if (level)
		exp = number_range(victim->exp_level - level, victim->exp_level + level);
	else
		exp = victim->exp_level;

	/* gaussian curve, exp gets lower as the player gets bigger */
	exp = (int)((float)exp * powf(2.7182818f, -0.5f * (exp_spent * exp_spent) / (std_dev * std_dev)) * (std_dev * 2.0f * 3.1415926f) * scale + shift_up);

	/* 1% extra per status point*/
	exp *= 100 + gch->race;
	exp /= 100;
	
	tierpoints = gch->max_hit / 1000;

	if (gch->remortlevel > 0)
	{
		if (victim->remortlevel < gch->remortlevel)
		{
			exp -= ((gch->remortlevel - victim->remortlevel) * 0.2 * exp);
			tierpoints = tierpoints * 1.2 * (gch->remortlevel - victim->remortlevel);
			snprintf(buf, MAX_STRING_LENGTH, "#GYou receive %d blood points.\n\r", tierpoints);
			send_to_char(buf, gch);
			gch->tierpoints += tierpoints;
			send_to_char("#R[REMORT PENALTY!] #w", gch);
		}
		else
		{
			exp *= 1.25 * gch->remortlevel;
			tierpoints = tierpoints * 0.75 * (gch->remortlevel - victim->remortlevel);
			snprintf(buf, MAX_STRING_LENGTH, "#GYou receive %d blood points.\n\r", tierpoints);
			send_to_char(buf, gch);
			gch->tierpoints += tierpoints;

			send_to_char("#C[REMORT BONUS!!!] #w\n\r", gch);
		}
	}
	

	/* percentage modifier against wimpy people  */
	if (gch->wimpy)
	{
		exp -= exp * gch->wimpy / gch->max_hit;
	}

	/* encourage them to move along to a tougher zone */
	if (victim->exp_level < gch->max_hit)
	{
		lev = 100 - (gch->max_hit - victim->exp_level) / 40;
	}
	else
	{
		lev = 100;
	}
	lev = (lev >= 0) ? lev : 0;

	if (lev < 100)
	{
		exp *= lev;
		exp /= 100;
	}

	/* cap the exp based on their size if they're under 10k */
	cap = 5000 + (long)(gch->max_hit) + (long)(gch->max_mana) + (long)(gch->max_move);
	if ((exp > cap) && (gch->max_hit < 10000))
		exp = cap;

	bonus = exp;

	/* Adding it here to make it truly double EXP */
	if (double_exp)
	{
		exp += bonus;
		//info(victim, 0, "(DOUBLE EXP) You gain %d bonus exp points!\n\r", bonus);
	}

	return exp;
}

void dam_message(CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt)
{
	static char *const attack_table[] =
		{
			"hit",
			"slice", "stab", "slash", "whip", "claw",
			"blast", "pound", "crush", "grep", "bite",
			"pierce", "suck"};
	static char *const attack_table2[] =
		{
			"hits",
			"slices", "stabs", "slashes", "whips", "claws",
			"blasts", "pounds", "crushes", "greps", "bites",
			"pierces", "sucks"};
	char jokbug[256];
	char buf4[256], buf5[256], buf6[256];
	const char *vs;
	const char *vp;
	const char *attack;
	const char *attack2;
	int damp;
	int bodyloc;
	int recover;
	bool critical = FALSE;
	char punct;

	if (dam == 0)
	{
		vs = " miss";
		vp = " misses";
	}
	else if (dam <= 10)
	{
		vs = " very lightly";
		vp = " very lightly";
	}
	else if (dam <= 25)
	{
		vs = " lightly";
		vp = " lightly";
	}
	else if (dam <= 50)
	{
		vs = " hard";
		vp = " hard";
	}
	else if (dam <= 75)
	{
		vs = " very hard";
		vp = " very hard";
	}
	else if (dam <= 99)
	{
		vs = " extremely hard";
		vp = " extremely hard";
	}
	else if (dam <= 150)
	{
		vs = " incredably hard";
		vp = " incredably hard";
	}
	else if (dam <= 250)
	{
		vs = " Amazingly Hard";
		vp = " Amazingly Hard";
	}
	else if (dam <= 450)
	{
		vs = " Stupendously Hard!";
		vp = " Stupendously Hard!";
	}
	else if (dam <= 600)
	{
		vs = " Unfeasably Hard!";
		vp = " Unfeasably Hard!";
	}
	else if (dam <= 999)
	{
		vs = " Impossibly Hard!!";
		vp = " Impossibly Hard!!";
	}
	else
	{
		vs = " Supernaturally Hard!!!!";
		vp = " Supernaturally Hard!!!!";
	}

	/* If victim's hp are less/equal to 'damp', attacker gets a death blow */

	if (IS_NPC(victim))
		damp = 0;
	else
		damp = -1;
	if ((victim->hit - dam > damp) || (dt >= 0 && dt < MAX_SKILL))
	{
		punct = (dam <= 40) ? '.' : '!';

		if (dt == TYPE_HIT)
		{
			if (dam == 0)
			{
				snprintf(buf4, 256, "$n%s $N%c", vp, punct);
				snprintf(buf5, 256, "You%s $N%c (%d)", vs, punct, dam);
				snprintf(buf6, 256, "$n%s you%c (%d)", vp, punct, dam);
				
			}
			else
			{
				snprintf(buf4, 256, "$n hits $N%s%c", vp, punct);
				snprintf(buf5, 256, "You hit $N%s%c (%d)", vs, punct, dam);
				snprintf(buf6, 256, "$n hits you%s%c (%d) ", vp, punct, dam);
				
				critical = TRUE;
			}
		}
		else
		{
			if (dt >= 0 && dt < MAX_SKILL)
			{
				attack = skill_table[dt].noun_damage;
				attack2 = skill_table[dt].noun_damage;
			}
			else if (dt >= TYPE_HIT && dt < TYPE_HIT + sizeof(attack_table) / sizeof(attack_table[0]))
			{
				attack = attack_table[dt - TYPE_HIT];
				attack2 = attack_table2[dt - TYPE_HIT];
			}
			else
			{
				snprintf(jokbug, 256, "Dam_message(1): bad dt %d.%s", dt, ch->name);
				bug(jokbug, 0);
				/*		bug( "Dam_message(1): bad dt %d.", dt );*/
				dt = TYPE_HIT;
				attack = attack_table[0];
				attack2 = attack_table2[0];
			}

			/* Check for weapon resistance - KaVir */
			recover = 0;
			if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_SLASH) &&
				(strcmp(attack, "slash") == 0 || strcmp(attack, "slice") == 0) && number_percent() > 5)
				recover = 10;
			if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_STAB) &&
				(strcmp(attack, "stab") == 0 || strcmp(attack, "pierce") == 0) && number_percent() > 5)
				recover = 10;
			if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_SMASH) &&
				(strcmp(attack, "blast") == 0 || strcmp(attack, "pound") == 0 || strcmp(attack, "crush") == 0) && number_percent() > 5)
				recover = 10;
			if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_ANIMAL) &&
				(strcmp(attack, "bite") == 0 || strcmp(attack, "claw") == 0) && number_percent() > 5)
				recover = 10;
			if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_MISC) &&
				(strcmp(attack, "grep") == 0 || strcmp(attack, "suck") == 0 || strcmp(attack, "whip") == 0) && number_percent() > 5)
				recover = 10;
			victim->hit = victim->hit + recover;
			if (victim->hit > victim->max_hit)
				victim->hit = victim->max_hit;

			if (dam == 0)
			{
				snprintf(buf4, 256, "$n's %s%s $N%c", attack, vp, punct);
				snprintf(buf5, 256, "Your %s%s $N%c (%d)", attack, vp, punct, dam);
				snprintf(buf6, 256, "$n's %s%s you%c (%d)", attack, vp, punct, dam);
				
			}
			else
			{
				if (dt >= 0 && dt < MAX_SKILL)
				{
					snprintf(buf4, 256, "$n's %s strikes $N%s%c", attack2, vp, punct);
					snprintf(buf5, 256, "Your %s strikes $N%s%c (%d)", attack, vp, punct, dam);
					snprintf(buf6, 256, "$n's %s strikes you%s%c (%d)", attack2, vp, punct, dam);
					
				}
				else
				{
					snprintf(buf4, 256, "$n %s $N%s%c", attack2, vp, punct);
					snprintf(buf5, 256, "You %s $N%s%c (%d)", attack, vp, punct, dam);
					snprintf(buf6, 256, "$n %s you%s%c (%d)", attack2, vp, punct, dam);
					
					critical = TRUE;
				}
			}
		}

		ADD_COLOUR(ch, buf5, LIGHTRED, 256);
		ADD_COLOUR(victim, buf6, RED, 256);

		act(buf4, ch, NULL, victim, TO_NOTVICT);
		if (!ch->choke_dam_message)
		{
			act(buf5, ch, NULL, victim, TO_CHAR);
		}
		if (!victim->choke_dam_message)
		{
			act(buf6, ch, NULL, victim, TO_VICT);
		}
		if (critical)
			critical_hit(ch, victim, dt, dam);
		return;
	}
	/* maybe insert the drop all for npc's here?? */

	if (dt == TYPE_HIT && !IS_VAMPAFF(ch, VAM_FANGS))
	{
		damp = number_range(1, 4);
		if (damp == 1)
		{
			if (!ch->choke_dam_message)
			{
				act("You ram your fingers into $N's eye sockets and rip $S face off.", ch, NULL, victim, TO_CHAR);
				act("$n rams $s fingers into $N's eye sockets and rips $S face off.", ch, NULL, victim, TO_NOTVICT);
				act("$n rams $s fingers into your eye sockets and rips your face off.", ch, NULL, victim, TO_VICT);
			}
			make_part(victim, "face");
		}
		else if (damp == 2)
		{
			if (!ch->choke_dam_message)
			{
				act("You grab $N by the throat and tear $S windpipe out.", ch, NULL, victim, TO_CHAR);
				act("$n grabs $N by the throat and tears $S windpipe out.", ch, NULL, victim, TO_NOTVICT);
				act("$n grabs you by the throat and tears your windpipe out.", ch, NULL, victim, TO_VICT);
			}
			make_part(victim, "windpipe");
		}
		else if (damp == 3)
		{
			if (!ch->choke_dam_message)
			{
				act("You punch your fist through $N's stomach and rip out $S entrails.", ch, NULL, victim, TO_CHAR);
				act("$n punches $s fist through $N's stomach and rips out $S entrails.", ch, NULL, victim, TO_NOTVICT);
				act("$n punches $s fist through your stomach and rips out your entrails.", ch, NULL, victim, TO_VICT);
			}
			make_part(victim, "entrails");
		}
		else if (damp == 4)
		{
			if (!IS_BODY(victim, BROKEN_SPINE))
				SET_BIT(victim->loc_hp[1], BROKEN_SPINE);
			if (!ch->choke_dam_message)
			{
				act("You hoist $N above your head and slam $M down upon your knee.\n\rThere is a loud cracking sound as $N's spine snaps.", ch, NULL, victim, TO_CHAR);
				act("$n hoists $N above $s head and slams $M down upon $s knee.\n\rThere is a loud cracking sound as $N's spine snaps.", ch, NULL, victim, TO_NOTVICT);
				act("$n hoists you above $s head and slams you down upon $s knee.\n\rThere is a loud cracking sound as your spine snaps.", ch, NULL, victim, TO_VICT);
			}
		}
		return;
	}
	if (dt >= 0 && dt < MAX_SKILL)
		attack = skill_table[dt].noun_damage;
	else if (dt >= TYPE_HIT && dt < TYPE_HIT + sizeof(attack_table) / sizeof(attack_table[0]))
		attack = attack_table[dt - TYPE_HIT];
	else
	{
		bug("Dam_message(2): bad dt %d.", dt);
		dt = TYPE_HIT;
		attack = attack_table[0];
	}

	if (strcmp(attack, "slash") == 0 || strcmp(attack, "slice") == 0)
	{
		damp = number_range(1, 8);
		if (damp == 1)
		{
			if (!ch->choke_dam_message)
			{
				act("You swing your blade in a low arc, rupturing $N's abdominal cavity.\n\r$S entrails spray out over a wide area.", ch, NULL, victim, TO_CHAR);
				act("$n swings $s blade in a low arc, rupturing $N's abdominal cavity.\n\r$S entrails spray out over a wide area.", ch, NULL, victim, TO_NOTVICT);
				act("$n swings $s blade in a low arc, rupturing your abdominal cavity.\n\rYour entrails spray out over a wide area.", ch, NULL, victim, TO_VICT);
			}
			make_part(victim, "entrails");
		}
		else if (damp == 2)
		{
			if (!ch->choke_dam_message)
			{
				act("You thrust your blade into $N's mouth and twist it viciously.\n\rThe end of your blade bursts through the back of $S head.", ch, NULL, victim, TO_CHAR);
				act("$n thrusts $s blade into $N's mouth and twists it viciously.\n\rThe end of the blade bursts through the back of $N's head.", ch, NULL, victim, TO_NOTVICT);
				act("$n thrusts $s blade into your mouth and twists it viciously.\n\rYou feel the end of the blade burst through the back of your head.", ch, NULL, victim, TO_VICT);
			}
		}
		else if (damp == 3)
		{
			if (!IS_BODY(victim, CUT_THROAT))
				SET_BIT(victim->loc_hp[1], CUT_THROAT);
			if (!IS_BLEEDING(victim, BLEEDING_THROAT))
				SET_BIT(victim->loc_hp[6], BLEEDING_THROAT);
			if (!ch->choke_dam_message)
			{
				act("Your blow slices open $N's carotid artery, spraying blood everywhere.", ch, NULL, victim, TO_CHAR);
				act("$n's blow slices open $N's carotid artery, spraying blood everywhere.", ch, NULL, victim, TO_NOTVICT);
				act("$n's blow slices open your carotid artery, spraying blood everywhere.", ch, NULL, victim, TO_VICT);
			}
			make_part(victim, "blood");
		}
		else if (damp == 4)
		{
			if (!IS_BODY(victim, CUT_THROAT))
				SET_BIT(victim->loc_hp[1], CUT_THROAT);
			if (!IS_BLEEDING(victim, BLEEDING_THROAT))
				SET_BIT(victim->loc_hp[6], BLEEDING_THROAT);
			if (!ch->choke_dam_message)
			{
				act("You swing your blade across $N's throat, showering the area with blood.", ch, NULL, victim, TO_CHAR);
				act("$n swings $s blade across $N's throat, showering the area with blood.", ch, NULL, victim, TO_NOTVICT);
				act("$n swings $s blade across your throat, showering the area with blood.", ch, NULL, victim, TO_VICT);
			}
			make_part(victim, "blood");
		}
		else if (damp == 5)
		{
			if (!IS_HEAD(victim, BROKEN_SKULL))
			{
				if (!ch->choke_dam_message)
				{
					act("You swing your blade down upon $N's head, splitting it open.\n\r$N's brains pour out of $S forehead.", ch, NULL, victim, TO_CHAR);
					act("$n swings $s blade down upon $N's head, splitting it open.\n\r$N's brains pour out of $S forehead.", ch, NULL, victim, TO_NOTVICT);
					act("$n swings $s blade down upon your head, splitting it open.\n\rYour brains pour out of your forehead.", ch, NULL, victim, TO_VICT);
				}
				make_part(victim, "brain");
				SET_BIT(victim->loc_hp[0], BROKEN_SKULL);
			}
			else
			{
				if (!ch->choke_dam_message)
				{
					act("You plunge your blade deep into $N's chest.", ch, NULL, victim, TO_CHAR);
					act("$n plunges $s blade deep into $N's chest.", ch, NULL, victim, TO_NOTVICT);
					act("$n plunges $s blade deep into your chest.", ch, NULL, victim, TO_VICT);
				}
			}
		}
		else if (damp == 6)
		{
			if (!ch->choke_dam_message)
			{
				act("You swing your blade between $N's legs, nearly splitting $M in half.", ch, NULL, victim, TO_CHAR);
				act("$n swings $s blade between $N's legs, nearly splitting $M in half.", ch, NULL, victim, TO_NOTVICT);
				act("$n swings $s blade between your legs, nearly splitting you in half.", ch, NULL, victim, TO_VICT);
			}
		}
		else if (damp == 7)
		{
			if (!IS_ARM_L(victim, LOST_ARM))
			{
				if (!ch->choke_dam_message)
				{
					act("You swing your blade in a wide arc, slicing off $N's arm.", ch, NULL, victim, TO_CHAR);
					act("$n swings $s blade in a wide arc, slicing off $N's arm.", ch, NULL, victim, TO_NOTVICT);
					act("$n swings $s blade in a wide arc, slicing off your arm.", ch, NULL, victim, TO_VICT);
				}
				make_part(victim, "arm");
				SET_BIT(victim->loc_hp[2], LOST_ARM);
				if (!IS_BLEEDING(victim, BLEEDING_ARM_L))
					SET_BIT(victim->loc_hp[6], BLEEDING_ARM_L);
				if (IS_BLEEDING(victim, BLEEDING_HAND_L))
					REMOVE_BIT(victim->loc_hp[6], BLEEDING_HAND_L);
			}
			else if (!IS_ARM_R(victim, LOST_ARM))
			{
				if (!ch->choke_dam_message)
				{
					act("You swing your blade in a wide arc, slicing off $N's arm.", ch, NULL, victim, TO_CHAR);
					act("$n swings $s blade in a wide arc, slicing off $N's arm.", ch, NULL, victim, TO_NOTVICT);
					act("$n swings $s blade in a wide arc, slicing off your arm.", ch, NULL, victim, TO_VICT);
				}
				make_part(victim, "arm");
				SET_BIT(victim->loc_hp[3], LOST_ARM);
				if (!IS_BLEEDING(victim, BLEEDING_ARM_R))
					SET_BIT(victim->loc_hp[6], BLEEDING_ARM_R);
				if (IS_BLEEDING(victim, BLEEDING_HAND_R))
					REMOVE_BIT(victim->loc_hp[6], BLEEDING_HAND_R);
			}
			else
			{
				if (!ch->choke_dam_message)
				{
					act("You plunge your blade deep into $N's chest.", ch, NULL, victim, TO_CHAR);
					act("$n plunges $s blade deep into $N's chest.", ch, NULL, victim, TO_NOTVICT);
					act("$n plunges $s blade deep into your chest.", ch, NULL, victim, TO_VICT);
				}
			}
		}
		else if (damp == 8)
		{
			if (!IS_LEG_L(victim, LOST_LEG))
			{
				if (!ch->choke_dam_message)
				{
					act("You swing your blade in a low arc, slicing off $N's leg at the hip.", ch, NULL, victim, TO_CHAR);
					act("$n swings $s blade in a low arc, slicing off $N's leg at the hip.", ch, NULL, victim, TO_NOTVICT);
					act("$n swings $s blade in a wide arc, slicing off your leg at the hip.", ch, NULL, victim, TO_VICT);
				}
				make_part(victim, "leg");
				SET_BIT(victim->loc_hp[4], LOST_LEG);
				if (!IS_BLEEDING(victim, BLEEDING_LEG_L))
					SET_BIT(victim->loc_hp[6], BLEEDING_LEG_L);
				if (IS_BLEEDING(victim, BLEEDING_FOOT_L))
					REMOVE_BIT(victim->loc_hp[6], BLEEDING_FOOT_L);
			}
			else if (!IS_LEG_R(victim, LOST_LEG))
			{
				if (!ch->choke_dam_message)
				{
					act("You swing your blade in a low arc, slicing off $N's leg at the hip.", ch, NULL, victim, TO_CHAR);
					act("$n swings $s blade in a low arc, slicing off $N's leg at the hip.", ch, NULL, victim, TO_NOTVICT);
					act("$n swings $s blade in a wide arc, slicing off your leg at the hip.", ch, NULL, victim, TO_VICT);
				}
				make_part(victim, "leg");
				SET_BIT(victim->loc_hp[5], LOST_LEG);
				if (!IS_BLEEDING(victim, BLEEDING_LEG_R))
					SET_BIT(victim->loc_hp[6], BLEEDING_LEG_R);
				if (IS_BLEEDING(victim, BLEEDING_FOOT_R))
					REMOVE_BIT(victim->loc_hp[6], BLEEDING_FOOT_R);
			}
			else
			{
				if (!ch->choke_dam_message)
				{
					act("You plunge your blade deep into $N's chest.", ch, NULL, victim, TO_CHAR);
					act("$n plunges $s blade deep into $N's chest.", ch, NULL, victim, TO_NOTVICT);
					act("$n plunges $s blade deep into your chest.", ch, NULL, victim, TO_VICT);
				}
			}
		}
	}
	else if (strcmp(attack, "stab") == 0 || strcmp(attack, "pierce") == 0)
	{
		damp = number_range(1, 5);
		if (damp == 1)
		{
			if (!ch->choke_dam_message)
			{
				act("You defty invert your weapon and plunge it point first into $N's chest.\n\rA shower of blood sprays from the wound, showering the area.", ch, NULL, victim, TO_CHAR);
				act("$n defty inverts $s weapon and plunge it point first into $N's chest.\n\rA shower of blood sprays from the wound, showering the area.", ch, NULL, victim, TO_NOTVICT);
				act("$n defty inverts $s weapon and plunge it point first into your chest.\n\rA shower of blood sprays from the wound, showering the area.", ch, NULL, victim, TO_VICT);
			}
			make_part(victim, "blood");
		}
		else if (damp == 2)
		{
			if (!ch->choke_dam_message)
			{
				act("You thrust your blade into $N's mouth and twist it viciously.\n\rThe end of your blade bursts through the back of $S head.", ch, NULL, victim, TO_CHAR);
				act("$n thrusts $s blade into $N's mouth and twists it viciously.\n\rThe end of the blade bursts through the back of $N's head.", ch, NULL, victim, TO_NOTVICT);
				act("$n thrusts $s blade into your mouth and twists it viciously.\n\rYou feel the end of the blade burst through the back of your head.", ch, NULL, victim, TO_VICT);
			}
		}
		else if (damp == 3)
		{
			if (!ch->choke_dam_message)
			{
				act("You thrust your weapon up under $N's jaw and through $S head.", ch, NULL, victim, TO_CHAR);
				act("$n thrusts $s weapon up under $N's jaw and through $S head.", ch, NULL, victim, TO_NOTVICT);
				act("$n thrusts $s weapon up under your jaw and through your head.", ch, NULL, victim, TO_VICT);
			}
		}
		else if (damp == 4)
		{
			if (!ch->choke_dam_message)
			{
				act("You ram your weapon through $N's body, pinning $M to the ground.", ch, NULL, victim, TO_CHAR);
				act("$n rams $s weapon through $N's body, pinning $M to the ground.", ch, NULL, victim, TO_NOTVICT);
				act("$n rams $s weapon through your body, pinning you to the ground.", ch, NULL, victim, TO_VICT);
			}
		}
		else if (damp == 5)
		{
			if (!ch->choke_dam_message)
			{
				act("You stab your weapon into $N's eye and out the back of $S head.", ch, NULL, victim, TO_CHAR);
				act("$n stabs $s weapon into $N's eye and out the back of $S head.", ch, NULL, victim, TO_NOTVICT);
				act("$n stabs $s weapon into your eye and out the back of your head.", ch, NULL, victim, TO_VICT);
			}
			if (!IS_HEAD(victim, LOST_EYE_L) && number_percent() < 50)
				SET_BIT(victim->loc_hp[0], LOST_EYE_L);
			else if (!IS_HEAD(victim, LOST_EYE_R))
				SET_BIT(victim->loc_hp[0], LOST_EYE_R);
			else if (!IS_HEAD(victim, LOST_EYE_L))
				SET_BIT(victim->loc_hp[0], LOST_EYE_L);
		}
	}
	else if (strcmp(attack, "blast") == 0 || strcmp(attack, "pound") == 0 || strcmp(attack, "crush") == 0)
	{
		damp = number_range(1, 3);
		bodyloc = 0;
		if (damp == 1)
		{
			if (!ch->choke_dam_message)
			{
				act("Your blow smashes through $N's chest, caving in half $S ribcage.", ch, NULL, victim, TO_CHAR);
				act("$n's blow smashes through $N's chest, caving in half $S ribcage.", ch, NULL, victim, TO_NOTVICT);
				act("$n's blow smashes through your chest, caving in half your ribcage.", ch, NULL, victim, TO_VICT);
			}
			if (IS_BODY(victim, BROKEN_RIBS_1))
			{
				bodyloc += 1;
				REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_1);
			}
			if (IS_BODY(victim, BROKEN_RIBS_2))
			{
				bodyloc += 2;
				REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_2);
			}
			if (IS_BODY(victim, BROKEN_RIBS_4))
			{
				bodyloc += 4;
				REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_4);
			}
			if (IS_BODY(victim, BROKEN_RIBS_8))
			{
				bodyloc += 8;
				REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_8);
			}
			if (IS_BODY(victim, BROKEN_RIBS_16))
			{
				bodyloc += 16;
				REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_16);
			}
			bodyloc += number_range(1, 3);
			if (bodyloc > 24)
				bodyloc = 24;
			if (bodyloc >= 16)
			{
				bodyloc -= 16;
				SET_BIT(victim->loc_hp[1], BROKEN_RIBS_16);
			}
			if (bodyloc >= 8)
			{
				bodyloc -= 8;
				SET_BIT(victim->loc_hp[1], BROKEN_RIBS_8);
			}
			if (bodyloc >= 4)
			{
				bodyloc -= 4;
				SET_BIT(victim->loc_hp[1], BROKEN_RIBS_4);
			}
			if (bodyloc >= 2)
			{
				bodyloc -= 2;
				SET_BIT(victim->loc_hp[1], BROKEN_RIBS_2);
			}
			if (bodyloc >= 1)
			{
				bodyloc -= 1;
				SET_BIT(victim->loc_hp[1], BROKEN_RIBS_1);
			}
		}
		else if (damp == 2)
		{
			if (!ch->choke_dam_message)
			{
				act("Your blow smashes $N's spine, shattering it in several places.", ch, NULL, victim, TO_CHAR);
				act("$n's blow smashes $N's spine, shattering it in several places.", ch, NULL, victim, TO_NOTVICT);
				act("$n's blow smashes your spine, shattering it in several places.", ch, NULL, victim, TO_VICT);
			}
			if (!IS_BODY(victim, BROKEN_SPINE))
				SET_BIT(victim->loc_hp[1], BROKEN_SPINE);
		}
		else if (damp == 3)
		{
			if (!IS_HEAD(victim, BROKEN_SKULL))
			{
				if (!ch->choke_dam_message)
				{
					act("You swing your weapon down upon $N's head.\n\r$N's head cracks open like an overripe melon, leaking out brains.", ch, NULL, victim, TO_CHAR);
					act("$n swings $s weapon down upon $N's head.\n\r$N's head cracks open like an overripe melon, covering you with brains.", ch, NULL, victim, TO_NOTVICT);
					act("$n swings $s weapon down upon your head.\n\rYour head cracks open like an overripe melon, spilling your brains everywhere.", ch, NULL, victim, TO_VICT);
				}
				make_part(victim, "brain");
				SET_BIT(victim->loc_hp[0], BROKEN_SKULL);
			}
			else
			{
				if (!ch->choke_dam_message)
				{
					act("You hammer your weapon into $N's side, crushing bone.", ch, NULL, victim, TO_CHAR);
					act("$n hammers $s weapon into $N's side, crushing bone.", ch, NULL, victim, TO_NOTVICT);
					act("$n hammers $s weapon into your side, crushing bone.", ch, NULL, victim, TO_VICT);
				}
			}
		}
	}
	else if (strcmp(attack, "bite") == 0 || IS_VAMPAFF(ch, VAM_FANGS))
	{
		if (!ch->choke_dam_message)
		{
			act("You sink your teeth into $N's throat and tear out $S jugular vein.\n\rYou wipe the blood from your chin with one hand.", ch, NULL, victim, TO_CHAR);
			act("$n sink $s teeth into $N's throat and tears out $S jugular vein.\n\r$n wipes the blood from $s chin with one hand.", ch, NULL, victim, TO_NOTVICT);
			act("$n sink $s teeth into your throat and tears out your jugular vein.\n\r$n wipes the blood from $s chin with one hand.", ch, NULL, victim, TO_VICT);
		}
		make_part(victim, "blood");
		if (!IS_BODY(victim, CUT_THROAT))
			SET_BIT(victim->loc_hp[1], CUT_THROAT);
		if (!IS_BLEEDING(victim, BLEEDING_THROAT))
			SET_BIT(victim->loc_hp[6], BLEEDING_THROAT);
	}
	else if (strcmp(attack, "claw") == 0)
	{
		damp = number_range(1, 2);
		if (damp == 1)
		{
			if (!ch->choke_dam_message)
			{
				act("You tear out $N's throat, showering the area with blood.", ch, NULL, victim, TO_CHAR);
				act("$n tears out $N's throat, showering the area with blood.", ch, NULL, victim, TO_NOTVICT);
				act("$n tears out your throat, showering the area with blood.", ch, NULL, victim, TO_VICT);
			}
			make_part(victim, "blood");
			if (!IS_BODY(victim, CUT_THROAT))
				SET_BIT(victim->loc_hp[1], CUT_THROAT);
			if (!IS_BLEEDING(victim, BLEEDING_THROAT))
				SET_BIT(victim->loc_hp[6], BLEEDING_THROAT);
		}
		if (damp == 2)
		{
			if (!IS_HEAD(victim, LOST_EYE_L) && number_percent() < 50)
			{
				if (!ch->choke_dam_message)
				{
					act("You rip an eyeball from $N's face.", ch, NULL, victim, TO_CHAR);
					act("$n rips an eyeball from $N's face.", ch, NULL, victim, TO_NOTVICT);
					act("$n rips an eyeball from your face.", ch, NULL, victim, TO_VICT);
				}
				make_part(victim, "eyeball");
				SET_BIT(victim->loc_hp[0], LOST_EYE_L);
			}
			else if (!IS_HEAD(victim, LOST_EYE_R))
			{
				if (!ch->choke_dam_message)
				{
					act("You rip an eyeball from $N's face.", ch, NULL, victim, TO_CHAR);
					act("$n rips an eyeball from $N's face.", ch, NULL, victim, TO_NOTVICT);
					act("$n rips an eyeball from your face.", ch, NULL, victim, TO_VICT);
				}
				make_part(victim, "eyeball");
				SET_BIT(victim->loc_hp[0], LOST_EYE_R);
			}
			else if (!IS_HEAD(victim, LOST_EYE_L))
			{
				if (!ch->choke_dam_message)
				{
					act("You rip an eyeball from $N's face.", ch, NULL, victim, TO_CHAR);
					act("$n rips an eyeball from $N's face.", ch, NULL, victim, TO_NOTVICT);
					act("$n rips an eyeball from your face.", ch, NULL, victim, TO_VICT);
				}
				make_part(victim, "eyeball");
				SET_BIT(victim->loc_hp[0], LOST_EYE_L);
			}
			else
			{
				if (!ch->choke_dam_message)
				{
					act("You claw open $N's chest.", ch, NULL, victim, TO_CHAR);
					act("$n claws open $N's chest.", ch, NULL, victim, TO_NOTVICT);
					act("$n claws open $N's chest.", ch, NULL, victim, TO_VICT);
				}
			}
		}
	}
	else if (strcmp(attack, "whip") == 0)
	{
		if (!ch->choke_dam_message)
		{
			act("You entangle $N around the neck, and squeeze the life out of $S.", ch, NULL, victim, TO_CHAR);
			act("$n entangle $N around the neck, and squeezes the life out of $S.", ch, NULL, victim, TO_NOTVICT);
			act("$n entangles you around the neck, and squeezes the life out of you.", ch, NULL, victim, TO_VICT);
		}
		if (!IS_BODY(victim, BROKEN_NECK))
			SET_BIT(victim->loc_hp[1], BROKEN_NECK);
	}
	else if (strcmp(attack, "suck") == 0 || strcmp(attack, "grep") == 0)
	{
		if (!ch->choke_dam_message)
		{
			act("You place your weapon on $N's head and suck out $S brains.", ch, NULL, victim, TO_CHAR);
			act("$n places $s weapon on $N's head and suck out $S brains.", ch, NULL, victim, TO_NOTVICT);
			act("$n places $s weapon on your head and suck out your brains.", ch, NULL, victim, TO_VICT);
		}
	}
	else
	{
		bug("Dam_message(3): bad dt %d.", dt);
	}
	return;
}

/*
* Disarm a creature.
* Caller must check for successful attack.
*/
void disarm(CHAR_DATA *ch, CHAR_DATA *victim)
{
	OBJ_DATA *obj;
	char buf[MAX_INPUT_LENGTH];

	/* I'm fed up of being disarmed every 10 seconds - KaVir */
	if (IS_NPC(ch) && victim->level > 2 && number_percent() > 5)
		return;

	if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_DISARM) &&
		number_percent() > (ch->max_move / 1000))
		return;

	if (((obj = get_eq_char(victim, WEAR_WIELD)) == NULL) || !IS_WEAPON(obj))
	{
		if (((obj = get_eq_char(victim, WEAR_HOLD)) == NULL) || !IS_WEAPON(obj))
			return;
	}

	snprintf(buf, MAX_INPUT_LENGTH, "$n disarms you!");
	ADD_COLOUR(ch, buf, WHITE, MAX_INPUT_LENGTH);
	act(buf, ch, NULL, victim, TO_VICT);
	snprintf(buf, MAX_INPUT_LENGTH, "You disarm $N!");
	ADD_COLOUR(ch, buf, WHITE, MAX_INPUT_LENGTH);
	act(buf, ch, NULL, victim, TO_CHAR);
	snprintf(buf, MAX_INPUT_LENGTH, "$n disarms $N!");
	ADD_COLOUR(ch, buf, WHITE, MAX_INPUT_LENGTH);
	act(buf, ch, NULL, victim, TO_NOTVICT);

	obj_from_char(obj);

	/* Loyal weapons come back ;)  KaVir */
	if (IS_SET(obj->extra_flags, ITEM_LOYAL) && (!IS_NPC(victim)))
	{
		act("$p leaps back into your hand!", victim, obj, NULL, TO_CHAR);
		act("$p leaps back into $n's hand!", victim, obj, NULL, TO_ROOM);
		obj_to_char(obj, victim);
		do_wear(victim, obj->name);
	}
	else if (IS_NPC(victim))
		obj_to_char(obj, victim);
	else
		obj_to_room(obj, victim->in_room);

	return;
}

/*
* Trip a creature.
* Caller must check for successful attack.
*/
void trip(CHAR_DATA *ch, CHAR_DATA *victim)
{
	char buf[MAX_INPUT_LENGTH];
	if (IS_AFFECTED(victim, AFF_FLYING))
		return;
	if (!IS_NPC(victim) && IS_VAMPAFF(victim, VAM_FLYING))
		return;
	if (victim->wait == 0)
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$n trips you and you go down!");
		ADD_COLOUR(ch, buf, WHITE, MAX_INPUT_LENGTH);
		act(buf, ch, NULL, victim, TO_VICT);
		snprintf(buf, MAX_INPUT_LENGTH, "You trip $N and $E goes down!");
		ADD_COLOUR(ch, buf, WHITE, MAX_INPUT_LENGTH);
		act(buf, ch, NULL, victim, TO_CHAR);
		snprintf(buf, MAX_INPUT_LENGTH, "$n trips $N and $E goes down!");
		ADD_COLOUR(ch, buf, WHITE, MAX_INPUT_LENGTH);
		act(buf, ch, NULL, victim, TO_NOTVICT);

		WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
		WAIT_STATE(victim, 2 * PULSE_VIOLENCE);
		victim->position = POS_RESTING;
	}

	return;
}

void do_kill(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int wpntype = 0;
	int wpntype2 = 0;

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (arg[0] == '\0')
	{
		send_to_char("Kill whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		send_to_char("You cannot kill yourself!\n\r", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim)
	{
		act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
		return;
	}

	/*
    if( !IS_NPC(ch) && !IS_NPC(victim) && ch->race > 1 && victim->race > 1)
    {
    send_to_char("You cannot kill someone with no status.\n\r",ch);
    return;
    }
    */

	if (IS_NPC(ch) && !IS_NPC(victim) && IS_AFFECTED(ch, AFF_CHARM) && victim->level < 3)
	{
		return;
	}

	if (ch->position == POS_FIGHTING)
	{
		send_to_char("You do the best you can!\n\r", ch);
		return;
	}

	if (!IS_NPC(victim) && number_percent() <= victim->pcdata->learned[gsn_fastdraw])
	{
		if ((obj = get_eq_char(victim, WEAR_SCABBARD_R)) != NULL)
		{
			wpntype = obj->value[3];
			if (wpntype < 0 || wpntype > 12)
				wpntype = 0;
			if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
			{
				obj_from_char(obj);
				obj_to_room(obj, victim->in_room);
				act("You hurl $p aside.", victim, obj, NULL, TO_CHAR);
				act("$n hurls $p aside.", victim, obj, NULL, TO_ROOM);
			}
			do_draw(victim, "right");
		}
		if ((obj = get_eq_char(victim, WEAR_SCABBARD_L)) != NULL)
		{
			wpntype2 = obj->value[3];
			if (wpntype2 < 0 || wpntype2 > 12)
				wpntype2 = 0;
			if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
			{
				obj_from_char(obj);
				obj_to_room(obj, victim->in_room);
				act("You hurl $p aside.", victim, obj, NULL, TO_CHAR);
				act("$n hurls $p aside.", victim, obj, NULL, TO_ROOM);
			}
			do_draw(victim, "left");
		}

		// If the attacker has King of the Mountain active - don't allow them to attack
		CLANDISC_DATA * disc;
        if( !IS_NPC(ch))
        {
            disc = GetPlayerDiscByTier(ch, FORTITUDE, FORTITUDE_KING_OF_THE_MOUNTAIN); // PCs that have King of the Mountain active cannot attack
            if(disc == NULL || !DiscIsActive(disc))
            {
               if (wpntype > 0)
                    one_hit(victim, ch, TYPE_UNDEFINED, 1);
                if (wpntype2 > 0)
                    one_hit(victim, ch, TYPE_UNDEFINED, 2);
            } else {
                send_to_char("You are unable to attack with King of the Mountain active.\n\r", ch);
            }
        }

	}

	WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
	check_killer(ch, victim);
	multi_hit(ch, victim, TYPE_UNDEFINED);
	return;
}

void do_backstab(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (arg[0] == '\0')
	{
		send_to_char("Backstab whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		send_to_char("How can you sneak up on yourself?\n\r", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (((obj = get_eq_char(ch, WEAR_WIELD)) == NULL || obj->value[3] != 11) && ((obj = get_eq_char(ch, WEAR_HOLD)) == NULL || obj->value[3] != 11))
	{
		send_to_char("You need to wield a piercing weapon.\n\r", ch);
		return;
	}

	if (victim->fighting != NULL)
	{
		send_to_char("You can't backstab a fighting person.\n\r", ch);
		return;
	}

	if (victim->hit < victim->max_hit)
	{
		act("$N is hurt and suspicious ... you can't sneak up.",
			ch, NULL, victim, TO_CHAR);
		return;
	}

	check_killer(ch, victim);
	WAIT_STATE(ch, skill_table[gsn_backstab].beats);
	if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_BACKSTAB) && number_percent() > 5)
		damage(ch, victim, 0, gsn_backstab);
	else if (!IS_AWAKE(victim) || IS_NPC(ch) || number_percent() < ch->pcdata->learned[gsn_backstab])
		multi_hit(ch, victim, gsn_backstab);
	else
		damage(ch, victim, 0, gsn_backstab);

	return;
}

void do_flee(CHAR_DATA *ch, char *argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	CHAR_DATA *victim, *fch;
	int attempt;

	if ((victim = ch->fighting) == NULL)
	{
		if (ch->position == POS_FIGHTING)
			ch->position = POS_STANDING;
		send_to_char("You aren't fighting anyone.\n\r", ch);
		return;
	}

	if(IS_AFFECTED(ch, AFF_CURSE))
	{
		if (number_percent ( ) > 95)
		{
			send_to_char("You cannnot flee while cursed.\n\r", ch);
			return;
		}
	}

	was_in = ch->in_room;
	for (attempt = 0; attempt < 6; attempt++)
	{
		EXIT_DATA *pexit;
		int door;

		door = number_door();
		if ((pexit = was_in->exit[door]) == 0 || pexit->to_room == NULL || IS_SET(pexit->exit_info, EX_CLOSED) || (IS_NPC(ch) && IS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)))
			continue;

		/* following added by Palmer */
		if (!IS_NPC(ch))
		{
			int relaxed_stance = 0;
			for (fch = ch->in_room->people; fch != NULL; fch = fch->next_in_room)
			{
				/* players shouldn't follow fleeing players*/
				if (!IS_NPC(fch) && (fch->master == ch))
				{
					stop_follower(fch);
				}

				/* players should relax stance when fleeing from players */
				if (!relaxed_stance && !IS_NPC(fch) && (fch->fighting == ch))
				{
					do_stance(ch, "");
					relaxed_stance = 1;
				}
			}
		}

		move_char(ch, door);

		if ((now_in = ch->in_room) == was_in)
			continue;

		ch->in_room = was_in;
		act("$n has fled!", ch, NULL, NULL, TO_ROOM);
		ch->in_room = now_in;

		if (!IS_NPC(ch))
			send_to_char("You flee from combat!  Coward!\n\r", ch);

		stop_fighting(ch, TRUE);
		WAIT_STATE(ch, 2);

		return;
	}

	send_to_char("You were unable to escape!\n\r", ch);
	return;
}

void do_rescue(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *fch;

	one_argument(argument, arg, MAX_INPUT_LENGTH);
	if (arg[0] == '\0')
	{
		send_to_char("Rescue whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		send_to_char("What about fleeing instead?\n\r", ch);
		return;
	}

	if (ch->position == POS_FIGHTING)
	{
		send_to_char("You better finish your own fight first...\n\r", ch);
		return;
	}

	if (!IS_NPC(ch) && IS_NPC(victim))
	{
		send_to_char("Doesn't need your help!\n\r", ch);
		return;
	}

	if ((IS_AFFECTED(ch, AFF_SHADOWPLANE) &&
		 !IS_AFFECTED(victim, AFF_SHADOWPLANE)) ||
		(!IS_AFFECTED(ch, AFF_SHADOWPLANE) &&
		 IS_AFFECTED(victim, AFF_SHADOWPLANE)))
	{
		send_to_char("You can't rescue someone on a different plane!\n\r", ch);
		return;
	}

	if (ch->fighting == victim)
	{
		send_to_char("Too late.\n\r", ch);
		return;
	}

	if ((fch = victim->fighting) == NULL)
	{
		send_to_char("That person is not fighting right now.\n\r", ch);
		return;
	}

	WAIT_STATE(ch, skill_table[gsn_rescue].beats);
	if (!IS_NPC(ch) && number_percent() > ch->pcdata->learned[gsn_rescue])
	{
		send_to_char("You fail the rescue.\n\r", ch);
		return;
	}

	act("You rescue $N!", ch, NULL, victim, TO_CHAR);
	act("$n rescues you!", ch, NULL, victim, TO_VICT);
	act("$n rescues $N!", ch, NULL, victim, TO_NOTVICT);

	stop_fighting(fch, FALSE);
	stop_fighting(victim, FALSE);

	check_killer(ch, fch);
	set_fighting(ch, fch);
	set_fighting(fch, ch);

	do_humanity(ch, "");
	return;
}

void do_kick(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	int dam;
	int stance;

	if (!IS_NPC(ch) && ch->level < skill_table[gsn_kick].skill_level[ch->class])
	{
		send_to_char(
			"First you should learn to kick.\n\r", ch);
		return;
	}

	if ((victim = ch->fighting) == NULL)
	{
		send_to_char("You aren't fighting anyone.\n\r", ch);
		return;
	}

	WAIT_STATE(ch, skill_table[gsn_kick].beats);
	if (IS_NPC(ch) || number_percent() < ch->pcdata->learned[gsn_kick])
		dam = number_range(1, 4);
	else
	{
		dam = 0;
		damage(ch, victim, dam, gsn_kick);
		return;
	}

	dam += GET_DAMROLL(ch);
	if (dam == 0)
		dam = 1;
	if (!IS_AWAKE(victim))
		dam *= 2;
	if (!IS_NPC(ch) && IS_VAMPAFF(ch, VAM_POTENCE))
		dam *= 1.5;
	if (!IS_NPC(ch))
		stance = ch->stance[CURRENT_STANCE];
	if (!IS_NPC(ch) && IS_STANCE(ch, STANCE_NORMAL))
		dam *= 1.25;
	else if (!IS_NPC(ch) && IS_STANCE(ch, STANCE_BULL) && ch->stance[STANCE_BULL] > 100)
		dam = dam + (dam * ((ch->stance[STANCE_BULL] - 100) / 100));

	else if (!IS_NPC(ch) && IS_STANCE(ch, STANCE_LION) && ch->stance[STANCE_LION] > 100)
		dam = dam + (dam * ((ch->stance[STANCE_LION] - 100) / 100));
	else if (!IS_NPC(ch) && IS_STANCE(ch, STANCE_GRIZZLIE) && ch->stance[STANCE_GRIZZLIE] > 100)
		dam = dam + (dam * ((ch->stance[STANCE_GRIZZLIE] - 100) / 100));

	else if (!IS_NPC(ch) && ch->stance[stance] > 0 && ch->stance[stance] < 100)
		dam = dam * (ch->stance[stance] + 1) / 100;
	/* Vampires should be tougher at night and weaker during the day. */
	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_VAMPIRE))
	{
		if (weather_info.sunlight == SUN_LIGHT)
			dam *= 1.5;
		else if (weather_info.sunlight == SUN_DARK)
			dam /= 1.5;
	}
	if (!IS_NPC(ch))
		dam = dam + (dam * (ch->wpn[0] / 100));

	if (dam <= 0)
		dam = 1;

	if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_KICK) && number_percent() > 5)
		dam = 0;

	if (IS_NPC(victim) && IS_SET(victim->act, ACT_IMMKICK))
		dam = 0;

	if (dam > 50)
		dam = 50;

	damage(ch, victim, dam, gsn_kick);
	/*    if ( !IS_NPC(victim) && IS_IMMUNE(victim, IMM_KICK) )
    victim->hit += dam;     */
	return;
}

void do_punch(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int dam;

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	if (ch->level < skill_table[gsn_punch].skill_level[ch->class])
	{
		send_to_char("First you should learn to punch.\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("You aren't fighting anyone.\n\r", ch);
		return;
	}

	if (ch == victim)
	{
		send_to_char("You cannot punch yourself!\n\r", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (victim->hit < victim->max_hit)
	{
		send_to_char("They are hurt and suspicious.\n\r", ch);
		return;
	}

	if (victim->position < POS_FIGHTING)
	{
		send_to_char("You can only punch someone who is standing.\n\r", ch);
		return;
	}

	act("You draw your fist back and aim a punch at $N.", ch, NULL, victim, TO_CHAR);
	act("$n draws $s fist back and aims a punch at you.", ch, NULL, victim, TO_VICT);
	act("$n draws $s fist back and aims a punch at $N.", ch, NULL, victim, TO_NOTVICT);
	WAIT_STATE(ch, skill_table[gsn_punch].beats);
	if (IS_NPC(ch) || number_percent() < ch->pcdata->learned[gsn_punch])
		dam = number_range(1, 4);
	else
	{
		dam = 0;
		damage(ch, victim, dam, gsn_punch);
		return;
	}

	dam += GET_DAMROLL(ch);
	if (dam == 0)
		dam = 1;
	if (!IS_AWAKE(victim))
		dam *= 2;
	if (!IS_NPC(ch))
		dam = dam + (dam * (ch->wpn[0] / 100));

	if (dam <= 0)
		dam = 1;

	damage(ch, victim, dam, gsn_punch);
	if (victim == NULL || victim->position == POS_DEAD)
		return;
	stop_fighting(victim, TRUE);
	if (number_percent() <= 25 && !IS_HEAD(victim, BROKEN_NOSE) && !IS_HEAD(victim, LOST_NOSE))
	{
		act("Your nose shatters under the impact of the blow!", victim, NULL, NULL, TO_CHAR);
		act("$n's nose shatters under the impact of the blow!", victim, NULL, NULL, TO_ROOM);
		SET_BIT(victim->loc_hp[LOC_HEAD], BROKEN_NOSE);
	}
	else if (number_percent() <= 25 && !IS_HEAD(victim, BROKEN_JAW))
	{
		act("Your jaw shatters under the impact of the blow!", victim, NULL, NULL, TO_CHAR);
		act("$n's jaw shatters under the impact of the blow!", victim, NULL, NULL, TO_ROOM);
		SET_BIT(victim->loc_hp[LOC_HEAD], BROKEN_JAW);
	}
	act("You fall to the ground stunned!", victim, NULL, NULL, TO_CHAR);
	act("$n falls to the ground stunned!", victim, NULL, NULL, TO_ROOM);
	victim->position = POS_STUNNED;
	return;
}

void do_berserk(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int counter = 0;
	counter = 0;
	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	if (ch->position == POS_FIGHTING)
	{
		send_to_char("You are already fighting !", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE))
	{
		send_to_char("You cannot berserk in a safe room!\n\r", ch);
		return;
	}

	if (ch->level < skill_table[gsn_berserk].skill_level[ch->class])
	{
		send_to_char("You are not wild enough to go berserk.\n\r", ch);
		return;
	}

	WAIT_STATE(ch, skill_table[gsn_berserk].beats);
	if (number_percent() > ch->pcdata->learned[gsn_berserk])
	{
		act("You rant and rave, but nothing much happens.", ch, NULL, NULL, TO_CHAR);
		act("$n gets a wild look in $s eyes, but nothing much happens.", ch, NULL, NULL, TO_ROOM);
		return;
	}

	act("You go BERSERK!", ch, NULL, NULL, TO_CHAR);
	act("$n goes BERSERK!", ch, NULL, NULL, TO_ROOM);
	for (vch = char_list; vch != NULL; vch = vch_next)
	{
		if (counter > 8)
			break;

		vch_next = vch->next;
		if (vch->in_room == NULL)
			continue;
		if (ch == vch)
			continue;
		if (vch->in_room == ch->in_room)
		{
			if (can_see(ch, vch) && (vch->mount == NULL || vch->mount != ch))
				one_hit(ch, vch, -1, 1);
			counter++;
		}
	}

	do_beastlike(ch, "");
	return;
}

/* Hurl skill by KaVir */
void do_hurl(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	EXIT_DATA *pexit_rev;
	char buf[MAX_INPUT_LENGTH];
	char direction[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int door;
	int rev_dir;
	int dam;

	argument = one_argument(argument, arg1, MAX_INPUT_LENGTH);
	argument = one_argument(argument, arg2, MAX_INPUT_LENGTH);

	if (!IS_NPC(ch) && ch->pcdata->learned[gsn_hurl] < 1)
	{
		send_to_char("Maybe you should learn the skill first?\n\r", ch);
		return;
	}

	if (arg1[0] == '\0')
	{
		send_to_char("Who do you wish to hurl?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg1)) == NULL)
	{
		send_to_char("They are not here.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		send_to_char("How can you hurl yourself?\n\r", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	if (victim->mount != NULL && victim->mounted == IS_MOUNT)
	{
		send_to_char("But they have someone on their back!\n\r", ch);
		return;
	}
	else if (victim->mount != NULL && victim->mounted == IS_RIDING)
	{
		send_to_char("But they are riding!\n\r", ch);
		return;
	}

	if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_HURL) && number_percent() > 5)
	{
		send_to_char("You are unable to get their feet of the ground.\n\r", ch);
		return;
	}

	if ((victim->hit < victim->max_hit) || (victim->position == POS_FIGHTING && victim->fighting != ch))
	{
		act("$N is hurt and suspicious, and you are unable to approach $M.",
			ch, NULL, victim, TO_CHAR);
		return;
	}

	WAIT_STATE(ch, skill_table[gsn_hurl].beats);
	if (!IS_NPC(ch) && number_percent() > ch->pcdata->learned[gsn_hurl])
	{
		send_to_char("You are unable to get their feet of the ground.\n\r", ch);
		multi_hit(victim, ch, TYPE_UNDEFINED);
		return;
	}

	rev_dir = 0;

	if (arg2[0] == '\0')
		door = number_range(0, 3);
	else
	{
		if (!str_cmp(arg2, "n") || !str_cmp(arg2, "north"))
			door = 0;
		else if (!str_cmp(arg2, "e") || !str_cmp(arg2, "east"))
			door = 1;
		else if (!str_cmp(arg2, "s") || !str_cmp(arg2, "south"))
			door = 2;
		else if (!str_cmp(arg2, "w") || !str_cmp(arg2, "west"))
			door = 3;
		else
		{
			send_to_char("You can only hurl people north, south, east or west.\n\r", ch);
			return;
		}
	}

	if (door == 0)
	{
		snprintf(direction, MAX_INPUT_LENGTH, "north");
		rev_dir = 2;
	}
	if (door == 1)
	{
		snprintf(direction, MAX_INPUT_LENGTH, "east");
		rev_dir = 3;
	}
	if (door == 2)
	{
		snprintf(direction, MAX_INPUT_LENGTH, "south");
		rev_dir = 0;
	}
	if (door == 3)
	{
		snprintf(direction, MAX_INPUT_LENGTH, "west");
		rev_dir = 1;
	}

	if ((pexit = ch->in_room->exit[door]) == NULL || (to_room = pexit->to_room) == NULL)
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$n hurls $N into the %s wall.", direction);
		act(buf, ch, NULL, victim, TO_NOTVICT);
		snprintf(buf, MAX_INPUT_LENGTH, "You hurl $N into the %s wall.", direction);
		act(buf, ch, NULL, victim, TO_CHAR);
		snprintf(buf, MAX_INPUT_LENGTH, "$n hurls you into the %s wall.", direction);
		act(buf, ch, NULL, victim, TO_VICT);
		dam = number_range(ch->level, (ch->level * 4));
		if (dam < 0)
			dam = -dam;
		victim->hit = victim->hit - dam;
		update_pos(victim);
		if (victim->position == POS_DEAD)
		{

			if (IS_NPC(victim) && !IS_NPC(ch))
				ch->mkill = ch->mkill + 1;
			if (!IS_NPC(victim) && IS_NPC(ch))
				victim->mdeath = victim->mdeath + 1;
			raw_kill(victim);
			return;
		}
		return;
	}

	if (IS_SET(pexit->exit_info, EX_BASHPROOF))
	{
		act("That $d is hurlproof!", ch, NULL, pexit->keyword, TO_CHAR);
		return;
	}

	pexit = victim->in_room->exit[door];
	if (IS_SET(pexit->exit_info, EX_CLOSED) &&
		!IS_SET(pexit->exit_info, EX_BASHPROOF) &&
		!IS_AFFECTED(victim, AFF_PASS_DOOR) &&
		!IS_AFFECTED(victim, AFF_ETHEREAL))
	{
		if (IS_SET(pexit->exit_info, EX_LOCKED))
			REMOVE_BIT(pexit->exit_info, EX_LOCKED);
		if (IS_SET(pexit->exit_info, EX_CLOSED))
			REMOVE_BIT(pexit->exit_info, EX_CLOSED);
		snprintf(buf, MAX_INPUT_LENGTH, "$n hoists $N in the air and hurls $M %s.", direction);
		act(buf, ch, NULL, victim, TO_NOTVICT);
		snprintf(buf, MAX_INPUT_LENGTH, "You hoist $N in the air and hurl $M %s.", direction);
		act(buf, ch, NULL, victim, TO_CHAR);
		snprintf(buf, MAX_INPUT_LENGTH, "$n hurls you %s, smashing you through the $d.", direction);
		act(buf, ch, NULL, victim, TO_VICT);
		snprintf(buf, MAX_INPUT_LENGTH, "There is a loud crash as $n smashes through the $d.");
		act(buf, victim, NULL, pexit->keyword, TO_ROOM);

		if ((to_room = pexit->to_room) != NULL && (pexit_rev = to_room->exit[rev_dir]) != NULL && pexit_rev->to_room == ch->in_room && pexit_rev->keyword != NULL)
		{
			if (IS_SET(pexit_rev->exit_info, EX_LOCKED))
				REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
			if (IS_SET(pexit_rev->exit_info, EX_CLOSED))
				REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);
			if (door == 0)
				snprintf(direction, MAX_INPUT_LENGTH, "south");
			if (door == 1)
				snprintf(direction, MAX_INPUT_LENGTH, "west");
			if (door == 2)
				snprintf(direction, MAX_INPUT_LENGTH, "north");
			if (door == 3)
				snprintf(direction, MAX_INPUT_LENGTH, "east");
			char_from_room(victim);
			char_to_room(victim, to_room);
			snprintf(buf, MAX_INPUT_LENGTH, "$n comes smashing in through the %s $d.", direction);
			act(buf, victim, NULL, pexit->keyword, TO_ROOM);
			dam = number_range(ch->level, (ch->level * 6));
			if (dam < 0)
				dam = -dam;
			victim->hit = victim->hit - dam;
			update_pos(victim);
			if (victim->position == POS_DEAD)
			{

				if (IS_NPC(victim) && !IS_NPC(ch))
					ch->mkill = ch->mkill + 1;
				if (!IS_NPC(victim) && IS_NPC(ch))
					victim->mdeath = victim->mdeath + 1;
				raw_kill(victim);
				return;
			}
		}
	}
	else
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$n hurls $N %s.", direction);
		act(buf, ch, NULL, victim, TO_NOTVICT);
		snprintf(buf, MAX_INPUT_LENGTH, "You hurl $N %s.", direction);
		act(buf, ch, NULL, victim, TO_CHAR);
		snprintf(buf, MAX_INPUT_LENGTH, "$n hurls you %s.", direction);
		act(buf, ch, NULL, victim, TO_VICT);
		if (door == 0)
			snprintf(direction, MAX_INPUT_LENGTH, "south");
		if (door == 1)
			snprintf(direction, MAX_INPUT_LENGTH, "west");
		if (door == 2)
			snprintf(direction, MAX_INPUT_LENGTH, "north");
		if (door == 3)
			snprintf(direction, MAX_INPUT_LENGTH, "east");
		char_from_room(victim);
		char_to_room(victim, to_room);
		snprintf(buf, MAX_INPUT_LENGTH, "$n comes flying in from the %s.", direction);
		act(buf, victim, NULL, NULL, TO_ROOM);
		dam = number_range(ch->level, (ch->level * 2));
		if (dam < 0)
			dam = -dam;
		victim->hit = victim->hit - dam;
		update_pos(victim);
		if (victim->position == POS_DEAD)
		{
			if (IS_NPC(victim) && !IS_NPC(ch))
				ch->mkill = ch->mkill + 1;
			if (!IS_NPC(victim) && IS_NPC(ch))
				victim->mdeath = victim->mdeath + 1;
			raw_kill(victim);
			return;
		}
	}
	return;
}

void do_disarm(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int percent;

	if (!IS_NPC(ch) && ch->level < skill_table[gsn_disarm].skill_level[ch->class])
	{
		send_to_char("You don't know how to disarm opponents.\n\r", ch);
		return;
	}

	if ((get_eq_char(ch, WEAR_WIELD) == NULL) && (get_eq_char(ch, WEAR_HOLD) == NULL))
	{
		send_to_char("You must wield a weapon to disarm.\n\r", ch);
		return;
	}

	if ((victim = ch->fighting) == NULL)
	{
		send_to_char("You aren't fighting anyone.\n\r", ch);
		return;
	}

	if (((obj = get_eq_char(victim, WEAR_WIELD)) == NULL) && ((obj = get_eq_char(victim, WEAR_HOLD)) == NULL))
	{
		send_to_char("Your opponent is not wielding a weapon.\n\r", ch);
		return;
	}

	WAIT_STATE(ch, skill_table[gsn_disarm].beats);
	percent = number_percent() + victim->level - ch->level;
	if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_DISARM) && number_percent() > 5)
		send_to_char("You failed.\n\r", ch);
	else if (IS_NPC(ch) || percent < ch->pcdata->learned[gsn_disarm] * 2 / 3)
		disarm(ch, victim);
	else
		send_to_char("You failed.\n\r", ch);
	return;
}

void do_sla(CHAR_DATA *ch, char *argument)
{
	send_to_char("If you want to SLAY, spell it out.\n\r", ch);
	return;
}

void do_slay(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, MAX_INPUT_LENGTH);
	if (arg[0] == '\0')
	{
		send_to_char("Slay whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (ch == victim)
	{
		send_to_char("Suicide is a mortal sin.\n\r", ch);
		return;
	}

	if (IS_SET(victim->act, PLR_GODLESS) && ch->level != LEVEL_CODER)
	{
		send_to_char("You failed.\n\r", ch);
		return;
	}

	if (!IS_NPC(victim) && victim->level >= ch->level)
	{
		send_to_char("You failed.\n\r", ch);
		return;
	}

	act("You slay $M in cold blood!", ch, NULL, victim, TO_CHAR);
	act("$n slays you in cold blood!", ch, NULL, victim, TO_VICT);
	act("$n slays $N in cold blood!", ch, NULL, victim, TO_NOTVICT);
	raw_kill(victim);
	return;
}

/* Had problems with people not dying when POS_DEAD...KaVir */
void do_killperson(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, MAX_INPUT_LENGTH);
	if (arg[0] == '\0')
		return;
	if ((victim = get_char_room(ch, arg)) == NULL)
		return;
	send_to_char("You have been KILLED!!\n\r\n\r", victim);
	if (IS_NPC(victim) && !IS_NPC(ch))
		ch->mkill = ch->mkill + 1;
	if (!IS_NPC(victim) && IS_NPC(ch))
		victim->mdeath = victim->mdeath + 1;
	raw_kill(victim);
	return;
}

void do_diablerize(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_STRING_LENGTH];
	char buf[MAX_STRING_LENGTH];

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	if (arg[0] == '\0')
	{
		send_to_char("Decapitate whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (ch == victim)
	{
		send_to_char("That might be a bit tricky...\n\r", ch);
		return;
	}

	if (IS_NPC(victim))
	{
		send_to_char("You can only decapitate other players.\n\r", ch);
		return;
	}

	if (ch->level != 3)
	{
		send_to_char("You must be an avatar to decapitate someone.\n\r", ch);
		return;
	}

	if (victim->position > 1)
	{
		send_to_char("You can only do this to mortally wounded players.\n\r", ch);
		return;
	}

	if (!str_cmp(ch->clan, victim->clan) && !str_cmp(ch->clan, "\0") &&
		ch->clan != str_dup(""))
	{
		send_to_char("You cannot decapitate someone of your own clan.\n\r", ch);
		return;
	}

	if (ch->vampgen == 0)
	{
		send_to_char("You must be a vampire to diablerize other players.\n\r", ch);
		return;
	}

	if (victim->vampgen == 0)
	{
		send_to_char("You can only diablerize other vampires.\n\r", ch);
		return;
	}

	if (victim->vampgen < ch->vampgen)
	{
		send_to_char("You must be of a higher generation to diablerize your opponent.\n\r", ch);
		return;
	}

	if (ch->vampgen == 4)
	{
		send_to_char("Lilith has deemed you unworthy of generation 3.\n\r", ch);
		return;
	}

	act("You rip the heart from $N's chest and sink your teeth deep into!", ch, NULL, victim, TO_CHAR);
	send_to_char("Your heart has been ripped from your chest!\n\r", victim);
	act("$n rips out the heart from $N's chest!", ch, NULL, victim, TO_NOTVICT);

	ch->vampgen -= 1;
	victim->vampgen += 1;
	
	// Should they gain beast?
	//do_beastlike(ch, "");
	

	// Set the victim back to a mortal
	if (IS_SET(victim->act, PLR_VAMPIRE))
		do_mortalvamp(victim, "");

	snprintf(buf, MAX_INPUT_LENGTH, "%s has been diablerized and had their generation stolen by %s.", victim->name, ch->name);
	
	// Let the whole mud know the victim is a loser
	do_info(ch, buf);

}

/* For decapitating players - KaVir */
void do_decapitate(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	OBJ_DATA *obj2;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	int agg;
	int def;

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	obj2 = get_eq_char(ch, WEAR_HOLD);
	if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL)
	{
		if ((obj2 = get_eq_char(ch, WEAR_HOLD)) == NULL)
		{
			send_to_char("First you better get a weapon out!\n\r", ch);
			return;
		}
	}

	if ((obj != NULL && !IS_WEAPON(obj)))
	{
		if ((obj2 != NULL && !IS_WEAPON(obj2)))
		{
			send_to_char("But you are not wielding any weapons!\n\r", ch);
			return;
		}
	}

	if ((obj != NULL && obj->value[3] != 1 && obj->value[3] != 3) && (obj2 != NULL && obj2->value[3] != 1 && obj2->value[3] != 3))
	{
		send_to_char("You need to wield a slashing or slicing weapon to decapitate.\n\r", ch);
		return;
	}

	if (arg[0] == '\0')
	{
		send_to_char("Decapitate whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (ch == victim)
	{
		send_to_char("That might be a bit tricky...\n\r", ch);
		return;
	}

	if (IS_NPC(victim))
	{
		send_to_char("You can only decapitate other players.\n\r", ch);
		return;
	}

	if (ch->level != 3)
	{
		send_to_char("You must be an avatar to decapitate someone.\n\r", ch);
		return;
	}

	/*	 if ( victim->level != 3 )
    {
    send_to_char( "You can only decapitate other avatars.\n\r", ch );
    return;
}*/

	if (victim->position > 1)
	{
		send_to_char("You can only do this to mortally wounded players.\n\r", ch);
		return;
	}

	if (!str_cmp(ch->clan, victim->clan) && !str_cmp(ch->clan, "\0") &&
		ch->clan != str_dup(""))
	{
		send_to_char("You cannot decapitate someone of your own clan.\n\r", ch);
		return;
	}

	if (ch->race <= 0)
		agg = 0;
	else if (ch->race <= 4)
		agg = 1;
	else if (ch->race <= 9)
		agg = 2;
	else if (ch->race <= 14)
		agg = 3;
	else if (ch->race <= 19)
		agg = 4;
	else if (ch->race <= 24)
		agg = 5;
	else if (ch->race >= 25)
		agg = 6;

	if (victim->race <= 0)
		def = 0;
	else if (victim->race <= 4)
		def = 1;
	else if (victim->race <= 9)
		def = 2;
	else if (victim->race <= 14)
		def = 3;
	else if (victim->race <= 19)
		def = 4;
	else if (victim->race <= 24)
		def = 5;
	else if (victim->race >= 25)
		def = 6;

	if (agg > def)
	{
		send_to_char("You can only kill someone of the same level as you or higher!\n\r", ch);
		return;
	}

	/* if they have the same ip subnet mask, slap them on the wrist  */
	if (ch->desc->host_ip >> 8 == (victim->desc ? (victim->desc->host_ip >> 8) : (victim->host_ip >> 8)))
	{
		send_to_char("You're not allowed to kill someone from the same Internet subnet as you.\n\r", ch);
		return;
	}

	act("You bring your weapon down upon $N's neck!", ch, NULL, victim, TO_CHAR);
	send_to_char("Your head is sliced from your shoulders!\n\r", victim);
	act("$n swings $s weapon down towards $N's neck!", ch, NULL, victim, TO_NOTVICT);
	behead(victim);
	do_beastlike(ch, "");
	ch->pkill = ch->pkill + 1;
	victim->pdeath = victim->pdeath + 1;
	/*	 if ( victim->max_hit > 20 ) victim->max_hit = victim->max_hit - victim->max_hit / 10; */
	if (victim->pcdata->perm_str > 5)
		victim->pcdata->perm_str = victim->pcdata->perm_str - 1;
	if (victim->pcdata->perm_wis > 5)
		victim->pcdata->perm_wis = victim->pcdata->perm_wis - 1;
	if (victim->pcdata->perm_dex > 5)
		victim->pcdata->perm_dex = victim->pcdata->perm_dex - 1;
	if (victim->pcdata->perm_int > 5)
		victim->pcdata->perm_int = victim->pcdata->perm_int - 1;
	if (victim->pcdata->perm_con > 5)
		victim->pcdata->perm_con = victim->pcdata->perm_con - 1;
	if (victim->pcdata->perm_con > 20)
		victim->pcdata->perm_con = victim->pcdata->perm_con - 1;
	if (victim->pcdata->perm_str > 20)
		victim->pcdata->perm_con = victim->pcdata->perm_str - 5;
	if (victim->pcdata->perm_dex > 20)
		victim->pcdata->perm_con = victim->pcdata->perm_dex - 5;
	ch->exp = ch->exp + victim->exp / 2;
	snprintf(buf, MAX_INPUT_LENGTH, "You receive %ld experience points.\n\r", victim->exp / 2);
	send_to_char(buf, ch);
	victim->exp = 0;

	/* Valis*/

	if (IS_SET(victim->act, PLR_VAMPIRE) && (IS_IMMUNE(victim, IMM_HEAT)))
		REMOVE_BIT(victim->immune, IMM_HEAT);
	else if (IS_IMMUNE(victim, IMM_BACKSTAB))
		REMOVE_BIT(victim->immune, IMM_BACKSTAB);
	else if (IS_IMMUNE(victim, IMM_STEAL))
		REMOVE_BIT(victim->immune, IMM_STEAL);
	else if (IS_IMMUNE(victim, IMM_CHARM))
		REMOVE_BIT(victim->immune, IMM_CHARM);
	else if (IS_IMMUNE(victim, IMM_SUMMON))
		REMOVE_BIT(victim->immune, IMM_SUMMON);
	else if (IS_IMMUNE(victim, IMM_VOODOO))
		REMOVE_BIT(victim->immune, IMM_VOODOO);
	else if (IS_IMMUNE(victim, IMM_SLEEP))
		REMOVE_BIT(victim->immune, IMM_SLEEP);
	else if (IS_IMMUNE(victim, IMM_HURL))
		REMOVE_BIT(victim->immune, IMM_HURL);
	else if (IS_IMMUNE(victim, IMM_DISARM))
		REMOVE_BIT(victim->immune, IMM_DISARM);
	else if (IS_IMMUNE(victim, IMM_DRAIN))
		REMOVE_BIT(victim->immune, IMM_DRAIN);
	else if (IS_IMMUNE(victim, IMM_KICK))
		REMOVE_BIT(victim->immune, IMM_KICK);
	else if (IS_IMMUNE(victim, IMM_HEAT))
		REMOVE_BIT(victim->immune, IMM_HEAT);
	else if (IS_IMMUNE(victim, IMM_COLD))
		REMOVE_BIT(victim->immune, IMM_COLD);
	else if (IS_IMMUNE(victim, IMM_ACID))
		REMOVE_BIT(victim->immune, IMM_ACID);
	else if (IS_IMMUNE(victim, IMM_LIGHTNING))
		REMOVE_BIT(victim->immune, IMM_LIGHTNING);
	else if (IS_IMMUNE(victim, IMM_SLASH))
		REMOVE_BIT(victim->immune, IMM_SLASH);
	else if (IS_IMMUNE(victim, IMM_STAB))
		REMOVE_BIT(victim->immune, IMM_STAB);
	else if (IS_IMMUNE(victim, IMM_SMASH))
		REMOVE_BIT(victim->immune, IMM_SMASH);
	else if (IS_IMMUNE(victim, IMM_ANIMAL))
		REMOVE_BIT(victim->immune, IMM_ANIMAL);
	else if (IS_IMMUNE(victim, IMM_MISC))
		REMOVE_BIT(victim->immune, IMM_MISC);

	if (victim->race < 1 && ch->race > 0)
	{
		snprintf(buf, MAX_INPUT_LENGTH, "%s has been decapitated by %s.", victim->name, ch->name);
		do_info(ch, buf);
		return;
	}
	victim->level = victim->level - 1;

	if (ch->race - ((ch->race / 100) * 100) == 0)
	{
		ch->race = ch->race + 1;
	}
	else if (ch->race - ((ch->race / 100) * 100) < 25)
	{
		ch->race = ch->race + 1;
	}
	if (ch->race - ((ch->race / 100) * 100) == 0)
	{
		victim->race = victim->race;
	}
	else if (victim->race - ((victim->race / 100) * 100) > 0)
	{
		victim->race = victim->race - 1;
	}
	act("A misty white vapour pours from $N's corpse into your body.", ch, NULL, victim, TO_CHAR);
	act("A misty white vapour pours from $N's corpse into $n's body.", ch, NULL, victim, TO_NOTVICT);
	act("You double over in agony as raw energy pulses through your veins.", ch, NULL, NULL, TO_CHAR);
	act("$n doubles over in agony as sparks of energy crackle around $m.", ch, NULL, NULL, TO_NOTVICT);
	if (IS_SET(victim->act, PLR_VAMPIRE))
		do_mortalvamp(victim, "");
	snprintf(buf, MAX_INPUT_LENGTH, "%s has been decapitated by %s.", victim->name, ch->name);
	do_info(ch, buf);
	
	if ((!str_cmp(ch->clan, "Assamite")) && (!str_cmp(victim->clan, "Tzimisce")))
		clan_infotable[1].pkills[2]++;
	else if ((!str_cmp(ch->clan, "Assamite")) && (!str_cmp(victim->clan, "Ventrue")))
		clan_infotable[1].pkills[3]++;
	else if ((!str_cmp(ch->clan, "Assamite")) && (!str_cmp(victim->clan, "Tremere")))
		clan_infotable[1].pkills[4]++;
	else if ((!str_cmp(ch->clan, "Assamite")) && (!str_cmp(victim->clan, "Lasombra")))
		clan_infotable[1].pkills[5]++;
	else if ((!str_cmp(ch->clan, "Assamite")) && (!str_cmp(victim->clan, "Toreador")))
		clan_infotable[1].pkills[6]++;
	else if ((!str_cmp(ch->clan, "Tzimisce")) && (!str_cmp(victim->clan, "Assamite")))
		clan_infotable[2].pkills[1]++;
	else if ((!str_cmp(ch->clan, "Tzimisce")) && (!str_cmp(victim->clan, "Ventrue")))
		clan_infotable[2].pkills[3]++;
	else if ((!str_cmp(ch->clan, "Tzimisce")) && (!str_cmp(victim->clan, "Tremere")))
		clan_infotable[2].pkills[4]++;
	else if ((!str_cmp(ch->clan, "Tzimisce")) && (!str_cmp(victim->clan, "Lasombra")))
		clan_infotable[2].pkills[5]++;
	else if ((!str_cmp(ch->clan, "Tzimisce")) && (!str_cmp(victim->clan, "Toreador")))
		clan_infotable[2].pkills[6]++;
	else if ((!str_cmp(ch->clan, "Ventrue")) && (!str_cmp(victim->clan, "Assamite")))
		clan_infotable[3].pkills[1]++;
	else if ((!str_cmp(ch->clan, "Ventrue")) && (!str_cmp(victim->clan, "Tzimisce")))
		clan_infotable[3].pkills[2]++;
	else if ((!str_cmp(ch->clan, "Ventrue")) && (!str_cmp(victim->clan, "Tremere")))
		clan_infotable[3].pkills[4]++;
	else if ((!str_cmp(ch->clan, "Ventrue")) && (!str_cmp(victim->clan, "Lasombra")))
		clan_infotable[3].pkills[5]++;
	else if ((!str_cmp(ch->clan, "Ventrue")) && (!str_cmp(victim->clan, "Toreador")))
		clan_infotable[3].pkills[6]++;
	else if ((!str_cmp(ch->clan, "Tremere")) && (!str_cmp(victim->clan, "Assamite")))
		clan_infotable[4].pkills[1]++;
	else if ((!str_cmp(ch->clan, "Tremere")) && (!str_cmp(victim->clan, "Tzimisce")))
		clan_infotable[4].pkills[2]++;
	else if ((!str_cmp(ch->clan, "Tremere")) && (!str_cmp(victim->clan, "Ventrue")))
		clan_infotable[4].pkills[3]++;
	else if ((!str_cmp(ch->clan, "Tremere")) && (!str_cmp(victim->clan, "Lasombra")))
		clan_infotable[4].pkills[5]++;
	else if ((!str_cmp(ch->clan, "Tremere")) && (!str_cmp(victim->clan, "Toreador")))
		clan_infotable[4].pkills[6]++;
	else if ((!str_cmp(ch->clan, "Lasombra")) && (!str_cmp(victim->clan, "Assamite")))
		clan_infotable[5].pkills[1]++;
	else if ((!str_cmp(ch->clan, "Lasombra")) && (!str_cmp(victim->clan, "Tzimisce")))
		clan_infotable[5].pkills[2]++;
	else if ((!str_cmp(ch->clan, "Lasombra")) && (!str_cmp(victim->clan, "Ventrue")))
		clan_infotable[5].pkills[3]++;
	else if ((!str_cmp(ch->clan, "Lasombra")) && (!str_cmp(victim->clan, "Tremere")))
		clan_infotable[5].pkills[4]++;
	else if ((!str_cmp(ch->clan, "Lasombra")) && (!str_cmp(victim->clan, "Toreador")))
		clan_infotable[5].pkills[6]++;
	else if ((!str_cmp(ch->clan, "Toreador")) && (!str_cmp(victim->clan, "Assamite")))
		clan_infotable[6].pkills[1]++;
	else if ((!str_cmp(ch->clan, "Toreador")) && (!str_cmp(victim->clan, "Tzimisce")))
		clan_infotable[6].pkills[2]++;
	else if ((!str_cmp(ch->clan, "Toreador")) && (!str_cmp(victim->clan, "Ventrue")))
		clan_infotable[6].pkills[3]++;
	else if ((!str_cmp(ch->clan, "Toreador")) && (!str_cmp(victim->clan, "Tremere")))
		clan_infotable[6].pkills[4]++;
	else if ((!str_cmp(ch->clan, "Toreador")) && (!str_cmp(victim->clan, "Lasombra")))
		clan_infotable[6].pkills[5]++;

	save_claninfo();
	return;
}

void do_crack(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;
	OBJ_DATA *right;
	OBJ_DATA *left;

	right = get_eq_char(ch, WEAR_WIELD);
	left = get_eq_char(ch, WEAR_HOLD);
	if (right != NULL && right->pIndexData->vnum == 12)
		obj = right;
	else if (left != NULL && left->pIndexData->vnum == 12)
		obj = left;
	else
	{
		send_to_char("You are not holding any heads.\n\r", ch);
		return;
	}
	act("You hurl $p at the floor.", ch, obj, NULL, TO_CHAR);
	act("$n hurls $p at the floor.", ch, obj, NULL, TO_ROOM);
	act("$p cracks open, leaking brains out across the floor.", ch, obj, NULL, TO_CHAR);
	if (obj->chobj != NULL)
	{
		act("$p cracks open, leaking brains out across the floor.", ch, obj, obj->chobj, TO_NOTVICT);
		act("$p crack open, leaking brains out across the floor.", ch, obj, obj->chobj, TO_VICT);
	}
	else
	{
		act("$p cracks open, leaking brains out across the floor.", ch, obj, NULL, TO_ROOM);
	}
	crack_head(ch, obj, obj->name);
	obj_from_char(obj);
	extract_obj(obj);
}

void crack_head(CHAR_DATA *ch, OBJ_DATA *obj, char *argument)
{
	CHAR_DATA *victim;
	MOB_INDEX_DATA *pMobIndex;

	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg1, MAX_INPUT_LENGTH);
	argument = one_argument(argument, arg2, MAX_INPUT_LENGTH);
	if (str_cmp(arg2, "mob") && obj->chobj != NULL && !IS_NPC(obj->chobj) &&
		IS_AFFECTED(obj->chobj, AFF_POLYMORPH))
	{
		victim = obj->chobj;
		make_part(victim, "cracked_head");
		make_part(victim, "brain");
		snprintf(buf, MAX_INPUT_LENGTH, "the quivering brain of %s", victim->name);
		free_string(victim->morph);
		victim->morph = str_dup(buf);
		return;
	}
	else if (!str_cmp(arg2, "mob"))
	{
		if ((pMobIndex = get_mob_index(obj->value[1])) == NULL)
			return;
		victim = create_mobile(pMobIndex);
		char_to_room(victim, ch->in_room);
		make_part(victim, "cracked_head");
		make_part(victim, "brain");
		extract_char(victim, TRUE);
		return;
	}
	else
	{
		if ((pMobIndex = get_mob_index(30002)) == NULL)
			return;
		victim = create_mobile(pMobIndex);

		snprintf(buf, MAX_INPUT_LENGTH, "%s", capitalize(arg2));
		free_string(victim->short_descr);
		victim->short_descr = str_dup(buf);

		char_to_room(victim, ch->in_room);
		make_part(victim, "cracked_head");
		make_part(victim, "brain");
		extract_char(victim, TRUE);
		return;
	}
	return;
}

/* Voodoo skill by KaVir */

void do_voodoo(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	char part1[MAX_INPUT_LENGTH];
	char part2[MAX_INPUT_LENGTH];
	int dam;

	argument = one_argument(argument, arg1, MAX_INPUT_LENGTH);
	argument = one_argument(argument, arg2, MAX_INPUT_LENGTH);

	if (arg1[0] == '\0')
	{
		send_to_char("Who do you wish to use voodoo magic on?\n\r", ch);
		return;
	}

	if ((obj = get_eq_char(ch, WEAR_HOLD)) == NULL)
	{
		send_to_char("You are not holding a voodoo doll.\n\r", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL)
	{
		send_to_char("They are not here.\n\r", ch);
		return;
	}

	if (IS_NPC(victim))
	{
		send_to_char("Not on NPC's.\n\r", ch);
		return;
	}

	snprintf(part2, MAX_INPUT_LENGTH, "%s", obj->name);
	snprintf(part1, MAX_INPUT_LENGTH, "%s voodoo doll", victim->name);

	if (str_cmp(part1, part2))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "But you are holding %s, not %s!\n\r", obj->short_descr, victim->name);
		send_to_char(buf, ch);
		return;
	}

	if (arg2[0] == '\0')
	{
		send_to_char("You can 'stab', 'burn' or 'throw' the doll.\n\r", ch);
	}
	else if (!str_cmp(arg2, "stab"))
	{
		act("You stab a pin through $p.", ch, obj, NULL, TO_CHAR);
		act("$n stabs a pin through $p.", ch, obj, NULL, TO_ROOM);
		if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_VOODOO) && number_percent() > 5)
			return;
		act("You feel an agonising pain in your chest!", victim, NULL, NULL, TO_CHAR);
		act("$n clutches $s chest in agony!", victim, NULL, NULL, TO_ROOM);
	}
	else if (!str_cmp(arg2, "burn"))
	{
		act("You set fire to $p.", ch, obj, NULL, TO_CHAR);
		act("$n sets fire to $p.", ch, obj, NULL, TO_ROOM);
		act("$p burns to ashes.", ch, obj, NULL, TO_CHAR);
		act("$p burns to ashes.", ch, obj, NULL, TO_ROOM);
		extract_obj(obj);
		if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_VOODOO))
			return;
		if (IS_AFFECTED(victim, AFF_FLAMING))
			return;
		SET_BIT(victim->affected_by, AFF_FLAMING);
		act("You suddenly burst into flames!", victim, NULL, NULL, TO_CHAR);
		act("$n suddenly bursts into flames!", victim, NULL, NULL, TO_ROOM);
	}
	else if (!str_cmp(arg2, "throw"))
	{
		act("You throw $p to the ground.", ch, obj, NULL, TO_CHAR);
		act("$n throws $p to the ground.", ch, obj, NULL, TO_ROOM);
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
		if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_VOODOO))
			return;
		if (victim->position < POS_STANDING)
			return;
		if (victim->position == POS_FIGHTING)
			stop_fighting(victim, TRUE);
		act("A strange force picks you up and hurls you to the ground!", victim, NULL, NULL, TO_CHAR);
		act("$n is hurled to the ground by a strange force.", victim, NULL, NULL, TO_ROOM);
		victim->position = POS_RESTING;
		dam = number_range(ch->level, (5 * ch->level));
		if (dam < 0)
			dam = -dam;
		victim->hit = victim->hit - dam;
		update_pos(victim);
		if (victim->position == POS_DEAD && !IS_NPC(victim))
		{
			do_killperson(ch, victim->name);
			return;
		}
	}
	else
	{
		send_to_char("You can 'stab', 'burn' or 'throw' the doll.\n\r", ch);
	}
	return;
}

/* Need to get rid of those flames somehow - KaVir */
void do_smother(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, MAX_INPUT_LENGTH);
	if (arg[0] == '\0')
	{
		send_to_char("Smother whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (ch == victim)
	{
		send_to_char("You cannot smother yourself.\n\r", ch);
		return;
	}

	if (!IS_AFFECTED(victim, AFF_FLAMING))
	{
		send_to_char("But they are not on fire!\n\r", ch);
		return;
	}

	if (number_percent() > (ch->level * 2))
	{
		act("You try to smother the flames around $N but fail!", ch, NULL, victim, TO_CHAR);
		act("$n tries to smother the flames around you but fails!", ch, NULL, victim, TO_VICT);
		act("$n tries to smother the flames around $N but fails!", ch, NULL, victim, TO_NOTVICT);
		if (number_percent() > 98 && !IS_AFFECTED(ch, AFF_FLAMING))
		{
			act("A spark of flame from $N's body sets you on fire!", ch, NULL, victim, TO_CHAR);
			act("A spark of flame from your body sets $n on fire!", ch, NULL, victim, TO_VICT);
			act("A spark of flame from $N's body sets $n on fire!", ch, NULL, victim, TO_NOTVICT);
			SET_BIT(ch->affected_by, AFF_FLAMING);
			do_humanity(ch, "");
		}
		return;
	}

	act("You manage to smother the flames around $M!", ch, NULL, victim, TO_CHAR);
	act("$n manages to smother the flames around you!", ch, NULL, victim, TO_VICT);
	act("$n manages to smother the flames around $N!", ch, NULL, victim, TO_NOTVICT);
	REMOVE_BIT(victim->affected_by, AFF_FLAMING);
	do_humanity(ch, "");
	return;
}

/* Loads of Vampire powers follow.  KaVir */

void do_fangs(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	if (!IS_SET(ch->act, PLR_VAMPIRE))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
	if (IS_VAMPAFF(ch, VAM_FANGS))
	{
		send_to_char("Your fangs slide back into your gums.\n\r", ch);
		if (IS_AFFECTED(ch, AFF_POLYMORPH))
			snprintf(buf, MAX_INPUT_LENGTH, "%s's fangs slide back into $s gums.", ch->morph);
		else
			snprintf(buf, MAX_INPUT_LENGTH, "$n's fangs slide back into $s gums.");
		act(buf, ch, NULL, NULL, TO_ROOM);
		REMOVE_BIT(ch->vampaff, VAM_FANGS);
		return;
	}
	send_to_char("Your fangs extend out of your gums.\n\r", ch);
	if (IS_AFFECTED(ch, AFF_POLYMORPH))
		snprintf(buf, MAX_INPUT_LENGTH, "A pair of razor sharp fangs extend from %s's mouth.", ch->morph);
	else
		snprintf(buf, MAX_INPUT_LENGTH, "A pair of razor sharp fangs extend from $n's mouth.");
	act(buf, ch, NULL, NULL, TO_ROOM);
	SET_BIT(ch->vampaff, VAM_FANGS);
	return;
}

void do_nightsight(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	if (!IS_SET(ch->act, PLR_VAMPIRE))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
	if (!IS_VAMPAFF(ch, VAM_OBTENEBRATION))
	{
		send_to_char("You are not trained in the correct disciplines.\n\r", ch);
		return;
	}
	if (IS_VAMPAFF(ch, VAM_NIGHTSIGHT))
	{
		send_to_char("The red glow in your eyes fades.\n\r", ch);
		if (IS_AFFECTED(ch, AFF_POLYMORPH))
			snprintf(buf, MAX_INPUT_LENGTH, "The red glow in %s's eyes fades.", ch->morph);
		else
			snprintf(buf, MAX_INPUT_LENGTH, "The red glow in $n's eyes fades.");
		act(buf, ch, NULL, NULL, TO_ROOM);
		REMOVE_BIT(ch->vampaff, VAM_NIGHTSIGHT);
		return;
	}
	if (ch->pcdata->condition[COND_THIRST] < 10)
	{
		send_to_char("You have insufficient blood.\n\r", ch);
		return;
	}
	ch->pcdata->condition[COND_THIRST] -= number_range(5, 10);
	send_to_char("Your eyes start glowing red.\n\r", ch);
	if (IS_AFFECTED(ch, AFF_POLYMORPH))
		snprintf(buf, MAX_INPUT_LENGTH, "%s's eyes start glowing red.", ch->morph);
	else
		snprintf(buf, MAX_INPUT_LENGTH, "$n's eyes start glowing red.");
	act(buf, ch, NULL, NULL, TO_ROOM);
	SET_BIT(ch->vampaff, VAM_NIGHTSIGHT);
	return;
}

void do_shadowsight(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	if (!IS_SET(ch->act, PLR_VAMPIRE))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
	if (!IS_VAMPAFF(ch, VAM_OBTENEBRATION))
	{
		send_to_char("You are not trained in the Obtenebration discipline.\n\r", ch);
		return;
	}
	if (IS_AFFECTED(ch, AFF_SHADOWSIGHT))
	{
		send_to_char("You can no longer see between planes.\n\r", ch);
		REMOVE_BIT(ch->affected_by, AFF_SHADOWSIGHT);
		return;
	}
	if (ch->pcdata->condition[COND_THIRST] < 10)
	{
		send_to_char("You have insufficient blood.\n\r", ch);
		return;
	}
	ch->pcdata->condition[COND_THIRST] -= number_range(5, 10);
	send_to_char("You can now see between planes.\n\r", ch);
	SET_BIT(ch->affected_by, AFF_SHADOWSIGHT);
	return;
}

void do_bite(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	int clancount;
	bool can_sire = FALSE;

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	if (!IS_SET(ch->act, PLR_VAMPIRE) || (ch->vampgen < 1))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (ch->vamppass == -1)
		ch->vamppass = ch->vampaff;

	if (ch->vampgen == 1 || ch->vampgen == 2)
		can_sire = TRUE;
	if (IS_EXTRA(ch, EXTRA_PRINCE))
		can_sire = TRUE;
	if (IS_EXTRA(ch, EXTRA_SIRE))
		can_sire = TRUE;
	if (ch->vampgen > 6)
		can_sire = FALSE;

	if (!can_sire)
	{
		send_to_char("You are not able to create any childer.\n\r", ch);
		return;
	}

	if (!str_cmp(ch->clan, "") && (ch->vampgen != 1))
	{
		send_to_char("First you need to found a clan.\n\r", ch);
		return;
	}

	clancount = 0;
	if (IS_VAMPPASS(ch, VAM_CELERITY))
		clancount = clancount + 1;
	if (IS_VAMPPASS(ch, VAM_FORTITUDE))
		clancount = clancount + 1;
	if (IS_VAMPPASS(ch, VAM_POTENCE))
		clancount = clancount + 1;
	if (IS_VAMPPASS(ch, VAM_OBFUSCATE))
		clancount = clancount + 1;
	if (IS_VAMPPASS(ch, VAM_OBTENEBRATION))
		clancount = clancount + 1;
	if (IS_VAMPPASS(ch, VAM_AUSPEX))
		clancount = clancount + 1;
	if (IS_VAMPPASS(ch, VAM_DOMINATE))
		clancount = clancount + 1;

	if (clancount < 3)
	{
		send_to_char("First you need to master 3 disciplines.\n\r", ch);
		return;
	}

	if (arg[0] == '\0')
	{
		send_to_char("Bite whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (IS_NPC(victim))
	{
		send_to_char("Not on NPC's.\n\r", ch);
		return;
	}

	if (ch == victim)
	{
		send_to_char("You cannot bite yourself.\n\r", ch);
		return;
	}

	if (victim->level < 3)
	{
		send_to_char("You can only bite avatars.\n\r", ch);
		return;
	}

	if (IS_SET(victim->act, PLR_VAMPIRE) && ch->beast != 100)
	{
		send_to_char("But they are already a vampire!\n\r", ch);
		return;
	}

	if (!IS_IMMUNE(victim, IMM_VAMPIRE) && ch->beast != 100)
	{
		send_to_char("You cannot bite an unwilling person.\n\r", ch);
		return;
	}

	if (!IS_VAMPAFF(ch, VAM_FANGS) && ch->beast != 100)
	{
		send_to_char("First you better get your fangs out!\n\r", ch);
		return;
	}

	if (IS_VAMPAFF(ch, VAM_DISGUISED) && ch->beast != 100)
	{
		send_to_char("You must reveal your true nature to bite someone.\n\r", ch);
		return;
	}

	if (ch->exp < 1000 && ch->beast != 100 && ch->level <= LEVEL_JUSTICAR)
	{
		send_to_char("You cannot afford the 1000 exp to create a childe.\n\r", ch);
		return;
	}

	if (ch->beast == 100)
	{
		if (!IS_VAMPAFF(ch, VAM_FANGS))
			do_fangs(ch, "");
		act("Your jaw opens wide and you leap hungrily at $N.", ch, NULL, victim, TO_CHAR);
		act("$n's jaw opens wide and $e leaps hungrily at $N.", ch, NULL, victim, TO_NOTVICT);
		act("$n's jaw opens wide and $e leaps hungrily at you.", ch, NULL, victim, TO_VICT);
		one_hit(ch, victim, -1, 0);
		return;
	}

	/*	 if (ch->beast >= 0)
    if(ch->beast == 0 && IS_SET(ch->immune, IMM_SUNLIGHT)) REMOVE_BIT(ch->immune, IMM_SUNLIGHT);
    if(ch->level < 4) ch->beast += 1;*/
	if (ch->level <= LEVEL_JUSTICAR)
		ch->exp = ch->exp - 1000;
	if (IS_EXTRA(ch, EXTRA_SIRE))
		REMOVE_BIT(ch->extra, EXTRA_SIRE);
	act("You sink your teeth into $N.", ch, NULL, victim, TO_CHAR);
	act("$n sinks $s teeth into $N.", ch, NULL, victim, TO_NOTVICT);
	act("$n sinks $s teeth into your neck.", ch, NULL, victim, TO_VICT);
	SET_BIT(victim->act, PLR_VAMPIRE);
	if (victim->vampgen != 0 && (victim->vampgen <= (ch->vampgen + 1)))
	{
		send_to_char("Your vampiric status has been restored.\n\r", victim);
		return;
	}
	send_to_char("You are now a vampire.\n\r", victim);
    
    if(0 == victim->vampgen)
    {
        victim->vampgen = 13;
    }
    
	free_string(victim->lord);
	if (ch->vampgen == 1)
		victim->lord = str_dup(ch->name);
	else
	{
		snprintf(buf, MAX_INPUT_LENGTH, "%s %s", ch->lord, ch->name);
		victim->lord = str_dup(buf);
	}
	if (ch->vampgen != 1)
	{
		if (victim->vamppass == -1)
			victim->vamppass = victim->vampaff;

		/* Remove hp bonus from fortitude */
		if (IS_VAMPPASS(victim, VAM_FORTITUDE) && !IS_VAMPAFF(victim, VAM_FORTITUDE))
		{
			victim->max_hit = victim->max_hit - 0;
			victim->hit = victim->hit - 0;
			if (victim->hit < 1)
				victim->hit = 1;
		}

		/* Remove any old powers they might have */
		if (IS_VAMPPASS(victim, VAM_CELERITY))
		{
			REMOVE_BIT(victim->vamppass, VAM_CELERITY);
			REMOVE_BIT(victim->vampaff, VAM_CELERITY);
		}
		if (IS_VAMPPASS(victim, VAM_FORTITUDE))
		{
			REMOVE_BIT(victim->vamppass, VAM_FORTITUDE);
			REMOVE_BIT(victim->vampaff, VAM_FORTITUDE);
		}
		if (IS_VAMPPASS(victim, VAM_POTENCE))
		{
			REMOVE_BIT(victim->vamppass, VAM_POTENCE);
			REMOVE_BIT(victim->vampaff, VAM_POTENCE);
		}
		if (IS_VAMPPASS(victim, VAM_OBFUSCATE))
		{
			REMOVE_BIT(victim->vamppass, VAM_OBFUSCATE);
			REMOVE_BIT(victim->vampaff, VAM_OBFUSCATE);
		}
		if (IS_VAMPPASS(victim, VAM_OBTENEBRATION))
		{
			REMOVE_BIT(victim->vamppass, VAM_OBTENEBRATION);
			REMOVE_BIT(victim->vampaff, VAM_OBTENEBRATION);
		}
		if (IS_VAMPPASS(victim, VAM_AUSPEX))
		{
			REMOVE_BIT(victim->vamppass, VAM_AUSPEX);
			REMOVE_BIT(victim->vampaff, VAM_AUSPEX);
		}
		if (IS_VAMPPASS(victim, VAM_DOMINATE))
		{
			REMOVE_BIT(victim->vamppass, VAM_DOMINATE);
			REMOVE_BIT(victim->vampaff, VAM_DOMINATE);
		}
		free_string(victim->clan);
		victim->clan = str_dup(ch->clan);
		/* Give the vampire the base powers of their sire */
		if (IS_VAMPPASS(ch, VAM_FORTITUDE) && !IS_VAMPAFF(victim, VAM_FORTITUDE))
		{
			victim->max_hit = victim->max_hit + 0;
			victim->hit = victim->hit + 0;
		}
		if (IS_VAMPPASS(ch, VAM_CELERITY))
		{
			SET_BIT(victim->vamppass, VAM_CELERITY);
			SET_BIT(victim->vampaff, VAM_CELERITY);
		}
		if (IS_VAMPPASS(ch, VAM_FORTITUDE))
		{
			SET_BIT(victim->vamppass, VAM_FORTITUDE);
			SET_BIT(victim->vampaff, VAM_FORTITUDE);
		}
		if (IS_VAMPPASS(ch, VAM_POTENCE))
		{
			SET_BIT(victim->vamppass, VAM_POTENCE);
			SET_BIT(victim->vampaff, VAM_POTENCE);
		}
		if (IS_VAMPPASS(ch, VAM_OBFUSCATE))
		{
			SET_BIT(victim->vamppass, VAM_OBFUSCATE);
			SET_BIT(victim->vampaff, VAM_OBFUSCATE);
		}
		if (IS_VAMPPASS(ch, VAM_OBTENEBRATION))
		{
			SET_BIT(victim->vamppass, VAM_OBTENEBRATION);
			SET_BIT(victim->vampaff, VAM_OBTENEBRATION);
		}
		if (IS_VAMPPASS(ch, VAM_AUSPEX))
		{
			SET_BIT(victim->vamppass, VAM_AUSPEX);
			SET_BIT(victim->vampaff, VAM_AUSPEX);
		}
		if (IS_VAMPPASS(ch, VAM_DOMINATE))
		{
			SET_BIT(victim->vamppass, VAM_DOMINATE);
			SET_BIT(victim->vampaff, VAM_DOMINATE);
		}
	}
	return;
}

void do_clanname(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);
	if (IS_NPC(ch))
		return;

	if (!IS_SET(ch->act, PLR_VAMPIRE) || ch->vampgen != 1)
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (arg[0] == '\0')
	{
		send_to_char("Who's clan do you wish to name?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}
	if (IS_NPC(victim))
		return;
	if (victim->vampgen != 2)
	{
		send_to_char("Only the Antediluvians may have clans.\n\r", ch);
		return;
	}
	if (str_cmp(victim->clan, ""))
	{
		send_to_char("But they already have a clan!\n\r", ch);
		return;
	}
	smash_tilde(argument);
	free_string(victim->clan);
	victim->clan = str_dup(argument);
	send_to_char("Clan name set.\n\r", ch);
	return;
}

void do_stake(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *stake;
	DESCRIPTOR_DATA *d;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int blood;

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (arg[0] == '\0')
	{
		send_to_char("Stake whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	stake = get_eq_char(ch, WEAR_HOLD);
	if ((stake == NULL) || (stake->item_type != ITEM_STAKE))
	{
		send_to_char("How can you stake someone down without holding a stake?\n\r", ch);
		return;
	}

	if (IS_NPC(victim))
	{
		send_to_char("Not on NPC's.\n\r", ch);
		return;
	}

	if (ch == victim)
	{
		send_to_char("You cannot stake yourself.\n\r", ch);
		return;
	}

	if (!IS_SET(victim->act, PLR_VAMPIRE))
	{
		send_to_char("You can only stake vampires.\n\r", ch);
		return;
	}

	if (victim->position > POS_MORTAL)
	{
		send_to_char("You can only stake down a vampire who is mortally wounded.\n\r", ch);
		return;
	}

	act("You plunge $p into $N's heart.", ch, stake, victim, TO_CHAR);
	act("$n plunges $p into $N's heart.", ch, stake, victim, TO_NOTVICT);
	send_to_char("You feel a stake plunged through your heart.\n\r", victim);
	if (IS_IMMUNE(victim, IMM_STAKE))
		return;
	if (IS_EXTRA(victim, EXTRA_TRUSTED))
		return;

	/* tell the clan the bad news */
	snprintf(buf, MAX_STRING_LENGTH, "Info -> %s has been staked by %s.\n\r", victim->name, ch->name);
	for (d = descriptor_list; d != NULL; d = d->next)
	{
		CHAR_DATA *wch;

		/* check for validity */
		if (d->connected != CON_PLAYING)
		{
			continue;
		}
		wch = (d->original != NULL) ? d->original : d->character;

		/* check vampiric status and clan membership */
		if (!IS_SET(wch->act, PLR_VAMPIRE) || (0 != strcmp(wch->clan, victim->clan)))
		{
			continue;
		}

		send_to_char(buf, wch);
	}

	/* Have to make sure they have enough blood to change back */
	blood = victim->pcdata->condition[COND_THIRST];
	victim->pcdata->condition[COND_THIRST] = 666;

	/* To take care of vampires who have powers in affect. */
	if (IS_VAMPAFF(victim, VAM_DISGUISED))
		do_mask(victim, victim->name);
	if (IS_IMMUNE(victim, IMM_SHIELDED))
		do_shield(victim, "");
	if (IS_AFFECTED(victim, AFF_SHADOWPLANE))
		do_shadowplane(victim, "");
	if (IS_VAMPAFF(victim, VAM_FANGS))
		do_fangs(victim, "");
	if (IS_VAMPAFF(victim, VAM_NIGHTSIGHT))
		do_nightsight(victim, "");
	if (IS_AFFECTED(victim, AFF_SHADOWSIGHT))
		do_shadowsight(victim, "");
	if (IS_SET(victim->act, PLR_HOLYLIGHT))
		do_truesight(victim, "");
	if (IS_VAMPAFF(victim, VAM_CHANGED))
		do_change(victim, "human");
	victim->pcdata->condition[COND_THIRST] = blood;

	if (IS_EXTRA(victim, EXTRA_PRINCE))
		REMOVE_BIT(victim->extra, EXTRA_PRINCE);
	if (IS_EXTRA(victim, EXTRA_SIRE))
		REMOVE_BIT(victim->extra, EXTRA_SIRE);
	/* Palmer added here */
	/*	 victim->vampaff = 0;  */
	/*
    if (IS_VAMPPASS(victim, VAM_CELERITY))
    {REMOVE_BIT(victim->vamppass, VAM_CELERITY);
    REMOVE_BIT(victim->vampaff, VAM_CELERITY);}
    if (IS_VAMPPASS(victim, VAM_FORTITUDE))
    {REMOVE_BIT(victim->vamppass, VAM_FORTITUDE);
    REMOVE_BIT(victim->vampaff, VAM_FORTITUDE);}
    if (IS_VAMPPASS(victim, VAM_POTENCE))
    {REMOVE_BIT(victim->vamppass, VAM_POTENCE);
    REMOVE_BIT(victim->vampaff, VAM_POTENCE);}
    if (IS_VAMPPASS(victim, VAM_OBFUSCATE))
    {REMOVE_BIT(victim->vamppass, VAM_OBFUSCATE);
    REMOVE_BIT(victim->vampaff, VAM_OBFUSCATE);}
    if (IS_VAMPPASS(victim, VAM_AUSPEX))
    {REMOVE_BIT(victim->vamppass, VAM_AUSPEX);
    REMOVE_BIT(victim->vampaff, VAM_AUSPEX);}
    if (IS_VAMPPASS(victim, VAM_OBTENEBRATION))
    {REMOVE_BIT(victim->vamppass, VAM_OBTENEBRATION);
    REMOVE_BIT(victim->vampaff, VAM_OBTENEBRATION);}
    */

	if (IS_VAMPPASS(victim, VAM_FORTITUDE))
	{
		victim->max_hit = victim->max_hit - 0;
		victim->hit = victim->hit - 0;
	}
	if (IS_SET(victim->act, PLR_VAMPIRE))
		do_mortalvamp(victim, "");
	free_string(victim->lord);
	free_string(victim->clan);
	free_string(victim->powertype);
	free_string(victim->poweraction);
	victim->spectype = 0;
	victim->vamppass = 0;
	victim->vampaff = 0;
	victim->vampgen = 0;
	victim->lord = str_dup("");
	victim->clan = str_dup("");
	victim->powertype = str_dup("");
	victim->poweraction = str_dup("");
	/*	 REMOVE_BIT(victim->act, PLR_VAMPIRE);
    */

	/* stakes disappear after one use - Palmer */
	extract_obj(stake);
	ch->exp = ch->exp + 1000;
	victim->home = 3001;
	return;
}

void do_mask(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	if (!IS_SET(ch->act, PLR_VAMPIRE))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!IS_VAMPAFF(ch, VAM_OBFUSCATE))
	{
		send_to_char("You are not trained in the Obfuscate discipline.\n\r", ch);
		return;
	}

	if (arg[0] == '\0')
	{
		send_to_char("Change to look like whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (victim->level > ch->level)
	{
		send_to_char("Nope!\n\r", ch);
		return;
	}

	if (IS_NPC(victim))
	{
		send_to_char("Not on NPC's.\n\r", ch);
		return;
	}

	if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch))
	{
		send_to_char("You can only mask avatars or lower.\n\r", ch);
		return;
	}

	if (ch->pcdata->condition[COND_THIRST] < 40)
	{
		send_to_char("You have insufficient blood.\n\r", ch);
		return;
	}
	ch->pcdata->condition[COND_THIRST] -= number_range(30, 40);

	if (ch == victim)
	{
		if (!IS_AFFECTED(ch, AFF_POLYMORPH) && !IS_VAMPAFF(ch, VAM_DISGUISED))
		{
			send_to_char("You already look like yourself!\n\r", ch);
			return;
		}
		snprintf(buf, MAX_INPUT_LENGTH, "Your form shimmers and transforms into %s.", ch->name);
		act(buf, ch, NULL, victim, TO_CHAR);
		snprintf(buf, MAX_INPUT_LENGTH, "%s's form shimmers and transforms into %s.", ch->morph, ch->name);
		act(buf, ch, NULL, victim, TO_ROOM);
		REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
		REMOVE_BIT(ch->vampaff, VAM_DISGUISED);
		free_string(ch->morph);
		ch->morph = str_dup("");
		return;
	}
	if (IS_VAMPAFF(ch, VAM_DISGUISED))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "Your form shimmers and transforms into a clone of %s.", victim->name);
		act(buf, ch, NULL, victim, TO_CHAR);
		snprintf(buf, MAX_INPUT_LENGTH, "%s's form shimmers and transforms into a clone of %s.", ch->morph, victim->name);
		act(buf, ch, NULL, victim, TO_NOTVICT);
		snprintf(buf, MAX_INPUT_LENGTH, "%s's form shimmers and transforms into a clone of you!", ch->morph);
		act(buf, ch, NULL, victim, TO_VICT);
		free_string(ch->morph);
		ch->morph = str_dup(victim->name);
		return;
	}
	snprintf(buf, MAX_INPUT_LENGTH, "Your form shimmers and transforms into a clone of %s.", victim->name);
	act(buf, ch, NULL, victim, TO_CHAR);
	snprintf(buf, MAX_INPUT_LENGTH, "%s's form shimmers and transforms into a clone of %s.", ch->name, victim->name);
	act(buf, ch, NULL, victim, TO_NOTVICT);
	snprintf(buf, MAX_INPUT_LENGTH, "%s's form shimmers and transforms into a clone of you!", ch->name);
	act(buf, ch, NULL, victim, TO_VICT);
	SET_BIT(ch->affected_by, AFF_POLYMORPH);
	SET_BIT(ch->vampaff, VAM_DISGUISED);
	free_string(ch->morph);
	ch->morph = str_dup(victim->name);
	return;
}

void do_change(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	if (!IS_SET(ch->act, PLR_VAMPIRE))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (arg[0] == '\0')
	{
		send_to_char("You can change between 'human', 'bat', 'wolf' and 'mist' forms.\n\r", ch);
		return;
	}

	if (!str_cmp(arg, "bat"))
	{
		if (IS_AFFECTED(ch, AFF_POLYMORPH))
		{
			send_to_char("You can only polymorph from human form.\n\r", ch);
			return;
		}
		if (ch->pcdata->condition[COND_THIRST] < 50)
		{
			send_to_char("You have insufficient blood.\n\r", ch);
			return;
		}
		ch->pcdata->condition[COND_THIRST] -= number_range(40, 50);
		clear_stats(ch);
		act("You transform into bat form.", ch, NULL, NULL, TO_CHAR);
		act("$n transforms into a bat.", ch, NULL, NULL, TO_ROOM);
		SET_BIT(ch->vampaff, VAM_FLYING);
		SET_BIT(ch->vampaff, VAM_SONIC);
		SET_BIT(ch->polyaff, POLY_BAT);
		SET_BIT(ch->vampaff, VAM_CHANGED);
		SET_BIT(ch->affected_by, AFF_POLYMORPH);
		snprintf(buf, MAX_INPUT_LENGTH, "%s the vampire bat", ch->name);
		free_string(ch->morph);
		ch->morph = str_dup(buf);
		return;
	}
	else if (!str_cmp(arg, "wolf"))
	{
		if (IS_AFFECTED(ch, AFF_POLYMORPH))
		{
			send_to_char("You can only polymorph from human form.\n\r", ch);
			return;
		}
		if (ch->pcdata->condition[COND_THIRST] < 50)
		{
			send_to_char("You have insufficient blood.\n\r", ch);
			return;
		}
		ch->pcdata->condition[COND_THIRST] -= number_range(40, 50);
		act("You transform into wolf form.", ch, NULL, NULL, TO_CHAR);
		act("$n transforms into a dire wolf.", ch, NULL, NULL, TO_ROOM);
		clear_stats(ch);
		if (ch->wpn[0] > 0)
		{
			ch->hitroll += ch->wpn[0] / 4;
			ch->damroll += ch->wpn[0] / 4;
			ch->armor -= ch->wpn[0];
		}
		ch->pcdata->mod_str = 10;
		SET_BIT(ch->polyaff, POLY_WOLF);
		SET_BIT(ch->affected_by, AFF_POLYMORPH);
		SET_BIT(ch->vampaff, VAM_CHANGED);
		snprintf(buf, MAX_INPUT_LENGTH, "%s the dire wolf", ch->name);
		free_string(ch->morph);
		ch->morph = str_dup(buf);
		return;
	}
	else if (!str_cmp(arg, "mist"))
	{
		if (IS_AFFECTED(ch, AFF_POLYMORPH))
		{
			send_to_char("You can only polymorph from human form.\n\r", ch);
			return;
		}
		if (ch->pcdata->condition[COND_THIRST] < 50)
		{
			send_to_char("You have insufficient blood.\n\r", ch);
			return;
		}
		if (IS_SET(ch->in_room->room_flags, ROOM_NO_MIST))
		{
			send_to_char("You are unable to assume mist form in this room.\n\r", ch);
			return;
		}

		ch->pcdata->condition[COND_THIRST] -= number_range(40, 50);
		act("You transform into mist form.", ch, NULL, NULL, TO_CHAR);
		act("$n transforms into a white mist.", ch, NULL, NULL, TO_ROOM);
		clear_stats(ch);
		SET_BIT(ch->polyaff, POLY_MIST);
		SET_BIT(ch->vampaff, VAM_CHANGED);
		SET_BIT(ch->affected_by, AFF_POLYMORPH);
		/*SET_BIT(ch->affected_by, AFF_ETHEREAL);*/
		snprintf(buf, MAX_INPUT_LENGTH, "%s the white mist", ch->name);
		free_string(ch->morph);
		ch->morph = str_dup(buf);
		return;
	}
	else if (!str_cmp(arg, "human"))
	{
		if (!IS_AFFECTED(ch, AFF_POLYMORPH))
		{
			send_to_char("You are already in human form.\n\r", ch);
			return;
		}
		if (IS_VAMPAFF(ch, VAM_CHANGED) && IS_POLYAFF(ch, POLY_BAT))
		{
			REMOVE_BIT(ch->vampaff, VAM_FLYING);
			REMOVE_BIT(ch->vampaff, VAM_SONIC);
			REMOVE_BIT(ch->polyaff, POLY_BAT);
		}
		else if (IS_VAMPAFF(ch, VAM_CHANGED) && IS_POLYAFF(ch, POLY_WOLF))
		{
			REMOVE_BIT(ch->polyaff, POLY_WOLF);
			if (ch->hit < 1)
				ch->hit = 1;
		}
		else if (IS_VAMPAFF(ch, VAM_CHANGED) && IS_POLYAFF(ch, POLY_MIST))
		{
			REMOVE_BIT(ch->polyaff, POLY_MIST);
			REMOVE_BIT(ch->affected_by, AFF_ETHEREAL);
		}
		else
		{
			/* In case they try to change to human from a non-vamp form */
			send_to_char("You seem to be stuck in this form.\n\r", ch);
			return;
		}
		act("You transform into human form.", ch, NULL, NULL, TO_CHAR);
		act("$n transforms into human form.", ch, NULL, NULL, TO_ROOM);
		clear_stats(ch);
		while (ch->affected)
			affect_remove(ch, ch->affected);
		REMOVE_BIT(ch->affected_by, AFF_POLYMORPH);
		REMOVE_BIT(ch->vampaff, VAM_CHANGED);
		free_string(ch->morph);
		ch->morph = str_dup("");
		return;
	}
	else
		send_to_char("You can change between 'human', 'bat', 'wolf' and 'mist' forms.\n\r", ch);
	return;
}

void clear_stats(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	if (IS_NPC(ch))
		return;

	for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
		obj_next = obj->next_content;
		if (obj->wear_loc != WEAR_NONE)
			unequip_char(ch, obj);
	}
	ch->armor = 0;
	ch->hitroll = 0;
	ch->damroll = 0;
	ch->saving_throw = 0;
	ch->pcdata->mod_str = 0;
	ch->pcdata->mod_int = 0;
	ch->pcdata->mod_wis = 0;
	ch->pcdata->mod_dex = 0;
	ch->pcdata->mod_con = 0;
	save_char_obj(ch);
	return;
}

void do_clandisc(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	int clancount;
	int clanmax;
	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	if (!IS_SET(ch->act, PLR_VAMPIRE))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (ch->vamppass == -1)
		ch->vamppass = ch->vampaff;

	if (ch->vampgen == 1)
		clanmax = 8;
	else if (ch->vampgen == 2)
		clanmax = 6;
	else if (ch->vampgen <= 4)
		clanmax = 5;
	else
		clanmax = 4;

	clancount = 0;
	if (IS_VAMPAFF(ch, VAM_ANIMALISM) || IS_VAMPPASS(ch, VAM_ANIMALISM))
		clancount = clancount + 1;
	if (IS_VAMPAFF(ch, VAM_AUSPEX) || IS_VAMPPASS(ch, VAM_AUSPEX))
		clancount = clancount + 1;
	if (IS_VAMPAFF(ch, VAM_CELERITY) || IS_VAMPPASS(ch, VAM_CELERITY))
		clancount = clancount + 1;
	if (IS_VAMPAFF(ch, VAM_DOMINATE) || IS_VAMPPASS(ch, VAM_DOMINATE))
		clancount = clancount + 1;
	if (IS_VAMPAFF(ch, VAM_FORTITUDE) || IS_VAMPPASS(ch, VAM_FORTITUDE))
		clancount = clancount + 1;
	if (IS_VAMPAFF(ch, VAM_OBFUSCATE) || IS_VAMPPASS(ch, VAM_OBFUSCATE))
		clancount = clancount + 1;
	if (IS_VAMPAFF(ch, VAM_OBTENEBRATION) || IS_VAMPPASS(ch, VAM_OBTENEBRATION))
		clancount = clancount + 1;
	if (IS_VAMPAFF(ch, VAM_POTENCE) || IS_VAMPPASS(ch, VAM_POTENCE))
		clancount = clancount + 1;
	if (IS_VAMPAFF(ch, VAM_PRESENCE) || IS_VAMPPASS(ch, VAM_PRESENCE))
		clancount = clancount + 1;
	if (IS_VAMPAFF(ch, VAM_QUIETUS) || IS_VAMPPASS(ch, VAM_QUIETUS))
		clancount = clancount + 1;
	if (IS_VAMPAFF(ch, VAM_THAUMATURGY) || IS_VAMPPASS(ch, VAM_THAUMATURGY))
		clancount = clancount + 1;
	if (IS_VAMPAFF(ch, VAM_VICISSITUDE) || IS_VAMPPASS(ch, VAM_VICISSITUDE))
		clancount = clancount + 1;

	if (arg[0] == '\0')
	{
		send_to_char("Current powers:", ch);
		if (IS_VAMPAFF(ch, VAM_ANIMALISM) && !IS_VAMPPASS(ch, VAM_ANIMALISM))
			send_to_char(" Animalism", ch);
		else if (IS_VAMPAFF(ch, VAM_ANIMALISM))
			send_to_char(" ANIMALISM", ch);
        if (IS_VAMPAFF(ch, VAM_AUSPEX) && !IS_VAMPPASS(ch, VAM_AUSPEX))
			send_to_char(" Auspex", ch);
		else if (IS_VAMPAFF(ch, VAM_AUSPEX))
			send_to_char(" AUSPEX", ch);
		if (IS_VAMPAFF(ch, VAM_CELERITY) && !IS_VAMPPASS(ch, VAM_CELERITY))
			send_to_char(" Celerity", ch);
		else if (IS_VAMPAFF(ch, VAM_CELERITY))
			send_to_char(" CELERITY", ch);
        if (IS_VAMPAFF(ch, VAM_DOMINATE) && !IS_VAMPPASS(ch, VAM_DOMINATE))
			send_to_char(" Dominate", ch);
		else if (IS_VAMPAFF(ch, VAM_DOMINATE))
			send_to_char(" DOMINATE", ch);
        if (IS_VAMPAFF(ch, VAM_FORTITUDE) && !IS_VAMPPASS(ch, VAM_FORTITUDE))
            send_to_char(" Fortitude", ch);
        else if (IS_VAMPAFF(ch, VAM_FORTITUDE))
            send_to_char(" FORTITUDE", ch);
		if (IS_VAMPAFF(ch, VAM_OBFUSCATE) && !IS_VAMPPASS(ch, VAM_OBFUSCATE))
			send_to_char(" Obfuscate", ch);
		else if (IS_VAMPAFF(ch, VAM_OBFUSCATE))
			send_to_char(" OBFUSCATE", ch);
		if (IS_VAMPAFF(ch, VAM_OBTENEBRATION) && !IS_VAMPPASS(ch, VAM_OBTENEBRATION))
			send_to_char(" Obtenebration", ch);
		else if (IS_VAMPAFF(ch, VAM_OBTENEBRATION))
			send_to_char(" OBTENEBRATION", ch);
		if (IS_VAMPAFF(ch, VAM_POTENCE) && !IS_VAMPPASS(ch, VAM_POTENCE))
			send_to_char(" Potence", ch);
		else if (IS_VAMPAFF(ch, VAM_POTENCE))
			send_to_char(" POTENCE", ch);
		if (IS_VAMPAFF(ch, VAM_PRESENCE) && !IS_VAMPPASS(ch, VAM_PRESENCE))
			send_to_char(" Presence", ch);
		else if (IS_VAMPAFF(ch, VAM_PRESENCE))
			send_to_char(" PRESENCE", ch);
		if (IS_VAMPAFF(ch, VAM_QUIETUS) && !IS_VAMPPASS(ch, VAM_QUIETUS))
			send_to_char(" Quietus", ch);
		else if (IS_VAMPAFF(ch, VAM_QUIETUS))
			send_to_char(" QUIETUS", ch);
		if (IS_VAMPAFF(ch, VAM_THAUMATURGY) && !IS_VAMPPASS(ch, VAM_THAUMATURGY))
			send_to_char(" Thuamaturgy", ch);
		else if (IS_VAMPAFF(ch, VAM_THAUMATURGY))
			send_to_char(" THAUMATURGY", ch);
		if (IS_VAMPAFF(ch, VAM_VICISSITUDE) && !IS_VAMPPASS(ch, VAM_VICISSITUDE))
			send_to_char(" Vicissitude", ch);
		else if (IS_VAMPAFF(ch, VAM_VICISSITUDE))
			send_to_char(" VICISSITUDE", ch);

		if (!IS_VAMPAFF(ch, VAM_CELERITY) && !IS_VAMPPASS(ch, VAM_CELERITY) &&
			!IS_VAMPAFF(ch, VAM_FORTITUDE) && !IS_VAMPAFF(ch, VAM_POTENCE) &&
			!IS_VAMPAFF(ch, VAM_OBFUSCATE) && !IS_VAMPAFF(ch, VAM_AUSPEX) &&
			!IS_VAMPAFF(ch, VAM_OBTENEBRATION) &&
			!IS_VAMPPASS(ch, VAM_FORTITUDE) && !IS_VAMPPASS(ch, VAM_POTENCE) &&
			!IS_VAMPPASS(ch, VAM_OBFUSCATE) && !IS_VAMPPASS(ch, VAM_AUSPEX) &&
			!IS_VAMPPASS(ch, VAM_DOMINATE) && !IS_VAMPPASS(ch, VAM_OBTENEBRATION))
			send_to_char(" None", ch);
		send_to_char(".\n\r", ch);
		if (clancount < clanmax)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "Select %d from:", (clanmax - clancount));
			send_to_char(buf, ch);
		}
		else
			return;
		if (!IS_VAMPAFF(ch, VAM_AUSPEX))
			send_to_char(" Auspex", ch);
		if (!IS_VAMPAFF(ch, VAM_ANIMALISM))
			send_to_char(" Animalism", ch);
	    if (!IS_VAMPAFF(ch, VAM_DOMINATE))
            send_to_char(" Dominate", ch);
		if (!IS_VAMPAFF(ch, VAM_FORTITUDE))
			send_to_char(" Fortitude", ch);
		if (!IS_VAMPAFF(ch, VAM_OBFUSCATE))
			send_to_char(" Obfuscate", ch);
		if (!IS_VAMPAFF(ch, VAM_OBTENEBRATION))
			send_to_char(" Obtenebration", ch);
        if (!IS_VAMPAFF(ch, VAM_POTENCE))
			send_to_char(" Potence", ch);
        if (!IS_VAMPAFF(ch, VAM_PRESENCE))
			send_to_char(" Presence", ch);
        if (!IS_VAMPAFF(ch, VAM_QUIETUS))
			send_to_char(" Quietus", ch);
        if (!IS_VAMPAFF(ch, VAM_THAUMATURGY))
			send_to_char(" Thaumaturgy", ch);
        if (!IS_VAMPAFF(ch, VAM_VICISSITUDE))
			send_to_char(" Vicissitude", ch);
		send_to_char(".\n\r", ch);
		return;
	}

	if (clancount >= clanmax)
	{
	    if (!str_cmp(arg, "auspex") && (IS_VAMPAFF(ch, VAM_AUSPEX) || IS_VAMPPASS(ch, VAM_AUSPEX)))
            send_to_char("Powers: Heightened Senses, Aura Perception, Prediction, Clairvoyance, Spirit Travel.\n\r", ch);
        else if (!str_cmp(arg, "animalism") && (IS_VAMPAFF(ch, VAM_ANIMALISM) || IS_VAMPPASS(ch, VAM_ANIMALISM)))
            send_to_char("Powers: Pact with Animals, Beckoning, Quell the Beast , Subsume the Spirit, Drawing Out the Beast.\n\r", ch);
        else if (!str_cmp(arg, "dominate") && (IS_VAMPAFF(ch, VAM_DOMINATE) || IS_VAMPPASS(ch, VAM_DOMINATE)))
            send_to_char("Powers: Command, Mesmerize, Possession, Command Obedience, Tranquility.\n\r", ch);
        else if (!str_cmp(arg, "fortitude") && (IS_VAMPAFF(ch, VAM_FORTITUDE) || IS_VAMPPASS(ch, VAM_FORTITUDE)))
            send_to_char("Powers: Personal Armor, Resilient Minds , Armor of Kings, King of the Mountain, Repair the Undead Flesh.\n\r", ch);
        else if (!str_cmp(arg, "obfuscate") && (IS_VAMPAFF(ch, VAM_OBFUSCATE) || IS_VAMPPASS(ch, VAM_OBFUSCATE)))
            send_to_char("Powers: Cloak of Shadows, Mask of a Thousand Faces, Fade from the Mind's Eye, The Silence of Death, Cloak the Gathering.\n\r", ch);
        else if (!str_cmp(arg, "obtenebration") && (IS_VAMPAFF(ch, VAM_OBTENEBRATION) || IS_VAMPPASS(ch, VAM_OBTENEBRATION)))
            send_to_char("Powers: Shadow Play, Shroud of Night, Arms of the Abyss, Black Metamorphosis, Shadowstep.\n\r", ch);
        else if (!str_cmp(arg, "potence") && (IS_VAMPAFF(ch, VAM_POTENCE) || IS_VAMPPASS(ch, VAM_POTENCE)))
            send_to_char("Powers: Crush, The Fist of Lillith, Earthshock, Aftershock, The Forger's Hammer.\n\r", ch);
        else if (!str_cmp(arg, "presence") && (IS_VAMPAFF(ch, VAM_PRESENCE) || IS_VAMPPASS(ch, VAM_PRESENCE)))
            send_to_char("Powers: Awe, Dread Gaze, Majesty, Paralyzing Glance, Summon.\n\r", ch);
        else if (!str_cmp(arg, "quietus") && (IS_VAMPAFF(ch, VAM_QUIETUS) || IS_VAMPPASS(ch, VAM_QUIETUS)))
            send_to_char("Powers: Scorpion's Touch, Dagon's Call, Baal's Caress, Taste of Death, Erosion.\n\r", ch);
        else if (!str_cmp(arg, "thaumaturgy") && (IS_VAMPAFF(ch, VAM_THAUMATURGY) || IS_VAMPPASS(ch, VAM_THAUMATURGY)))
            send_to_char("Powers: Geomancy, Spark, Vertigo, Contortion, Blood Boil.\n\r", ch);
        else if (!str_cmp(arg, "vicissitude") && (IS_VAMPAFF(ch, VAM_VICISSITUDE) || IS_VAMPPASS(ch, VAM_VICISSITUDE)))
            send_to_char("Powers: Malleable Visage, Fleshcraft, Bone Craft, Flesh Rot, Breath of the Dragon.\n\r", ch);
		else
			send_to_char("You don't know any such Discipline.\n\r", ch);
		return;
	}
    else if (!str_cmp(arg, "auspex"))
    {
         if (IS_VAMPAFF(ch, VAM_AUSPEX) || IS_VAMPPASS(ch, VAM_AUSPEX))
         {
             send_to_char("Powers: Heightened Senses, Aura Perception, Prediction, Clairvoyance, Spirit Travel.\n\r", ch);
             return;
         }
         send_to_char("You master the discipline of Auspex.\n\r", ch);

         if (clancount < 3)
             SET_BIT(ch->vamppass, VAM_AUSPEX);
         SET_BIT(ch->vampaff, VAM_AUSPEX);
         return;
    }
    else if (!str_cmp(arg, "animalism"))
    {
         if (IS_VAMPAFF(ch, VAM_ANIMALISM) || IS_VAMPPASS(ch, VAM_ANIMALISM))
         {
             send_to_char("Powers: Pact with Animals, Beckoning, Quell the Beast , Subsume the Spirit, Drawing Out the Beast.\n\r", ch);
             return;
         }
         send_to_char("You master the discipline of Animalism.\n\r", ch);


         if (clancount < 3)
             SET_BIT(ch->vamppass, VAM_ANIMALISM);
         SET_BIT(ch->vampaff, VAM_ANIMALISM);
         return;
    }
    else if (!str_cmp(arg, "dominate"))
    {
         if (IS_VAMPAFF(ch, VAM_DOMINATE) || IS_VAMPPASS(ch, VAM_DOMINATE))
         {
             send_to_char("Powers: Command, Mesmerize, Possession, Command Obedience, Tranquility.\n\r", ch);
             return;
         }
         send_to_char("You master the discipline of Dominate.\n\r", ch);


         if (clancount < 3)
             SET_BIT(ch->vamppass, VAM_DOMINATE);
         SET_BIT(ch->vampaff, VAM_DOMINATE);
         return;
    }
    else if (!str_cmp(arg, "fortitude"))
    {
         if (IS_VAMPAFF(ch, VAM_FORTITUDE) || IS_VAMPPASS(ch, VAM_FORTITUDE))
         {
             send_to_char("Powers: Personal Armor, Resilient Minds , Armor of Kings, King of the Mountain, Repair the Undead Flesh.\n\r", ch);
             return;
         }
         send_to_char("You master the discipline of Fortitude.\n\r", ch);

         if (clancount < 3)
             SET_BIT(ch->vamppass, VAM_FORTITUDE);
         SET_BIT(ch->vampaff, VAM_FORTITUDE);
         return;
    }
    else if (!str_cmp(arg, "obfuscate"))
    {
         if (IS_VAMPAFF(ch, VAM_OBFUSCATE) || IS_VAMPPASS(ch, VAM_OBFUSCATE))
         {
             send_to_char("Powers: Cloak of Shadows, Mask of a Thousand Faces, Fade from the Mind's Eye, The Silence of Death, Cloak the Gathering.\n\r", ch);
             return;
         }
         send_to_char("You master the discipline of Obfuscate.\n\r", ch);

         if (clancount < 3)
             SET_BIT(ch->vamppass, VAM_OBFUSCATE);
         SET_BIT(ch->vampaff, VAM_OBFUSCATE);
         return;
    }
    else if (!str_cmp(arg, "obtenebration"))
    {
        if (IS_VAMPAFF(ch, VAM_OBTENEBRATION) || IS_VAMPPASS(ch, VAM_OBTENEBRATION))
        {
            send_to_char("Powers: Shadow Play, Shroud of Night, Arms of the Abyss, Black Metamorphosis, Shadowstep.\n\r", ch);
            return;
        }
        send_to_char("You master the discipline of Obtenebration.\n\r", ch);


        if (clancount < 3)
            SET_BIT(ch->vamppass, VAM_OBTENEBRATION);
        SET_BIT(ch->vampaff, VAM_OBTENEBRATION);
        return;
    }
	else if (!str_cmp(arg, "potence"))
    {
        if (IS_VAMPAFF(ch, VAM_POTENCE) || IS_VAMPPASS(ch, VAM_POTENCE))
        {
            send_to_char("Powers: Crush, The Fist of Lillith, Earthshock, Aftershock, The Forger's Hammer.\n\r", ch);
            return;
        }
        send_to_char("You master the discipline of Potence.\n\r", ch);


        if (clancount < 3)
            SET_BIT(ch->vamppass, VAM_POTENCE);
        SET_BIT(ch->vampaff, VAM_POTENCE);
        return;
    }
	else if (!str_cmp(arg, "presence"))
    {
        if (IS_VAMPAFF(ch, VAM_PRESENCE) || IS_VAMPPASS(ch, VAM_PRESENCE))
        {
            send_to_char("Powers: Awe, Dread Gaze, Majesty, Paralyzing Glance, Summon.\n\r", ch);
            return;
        }
        send_to_char("You master the discipline of Presence.\n\r", ch);

        if (clancount < 3)
            SET_BIT(ch->vamppass, VAM_PRESENCE);
        SET_BIT(ch->vampaff, VAM_PRESENCE);
        return;
    }
	else if (!str_cmp(arg, "quietus"))
    {
        if (IS_VAMPAFF(ch, VAM_QUIETUS) || IS_VAMPPASS(ch, VAM_QUIETUS))
        {
            send_to_char("Powers: Scorpion's Touch, Dagon's Call, Baal's Caress, Taste of Death, Erosion.\n\r", ch);
            return;
        }
        send_to_char("You master the discipline of Quietus.\n\r", ch);


        if (clancount < 3)
            SET_BIT(ch->vamppass, VAM_QUIETUS);
        SET_BIT(ch->vampaff, VAM_QUIETUS);
        return;
    }
    else if (!str_cmp(arg, "thaumaturgy"))
    {
        if (IS_VAMPAFF(ch, VAM_THAUMATURGY) || IS_VAMPPASS(ch, VAM_THAUMATURGY))
        {
            send_to_char("Powers: Geomancy, Spark, Vertigo, Contortion, Blood Boil.\n\r", ch);
            return;
        }
        send_to_char("You master the discipline of Thaumaturgy.\n\r", ch);

        if (clancount < 3)
            SET_BIT(ch->vamppass, VAM_THAUMATURGY);
        SET_BIT(ch->vampaff, VAM_THAUMATURGY);
        return;
    }
	else if (!str_cmp(arg, "vicissitude"))
	{
	    if( IS_VAMPAFF(ch, VAM_VICISSITUDE) || IS_VAMPPASS(ch, VAM_VICISSITUDE))
	    {
	        send_to_char("Powers: Malleable Visage, Fleshcraft, Bone Craft, Flesh Rot, Breath of the Dragon.\n\r", ch);
	        return;
	    }
	    send_to_char("You master the discipline of Vicissitude.\n\r", ch);
	    if( clancount < 3)
	        SET_BIT(ch->vamppass, VAM_VICISSITUDE);
	    SET_BIT(ch->vampaff, VAM_VICISSITUDE);
	    return;
	}
	else
		send_to_char("No such discipline.\n\r", ch);
	return;
}

void do_vampire(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	if (!IS_IMMUNE(ch, IMM_VAMPIRE))
	{
		send_to_char("You will now allow vampires to bite you.\n\r", ch);
		SET_BIT(ch->immune, IMM_VAMPIRE);
		return;
	}
	send_to_char("You will no longer allow vampires to bite you.\n\r", ch);
	REMOVE_BIT(ch->immune, IMM_VAMPIRE);
	return;
}

void do_shadowplane(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	if (!IS_SET(ch->act, PLR_VAMPIRE))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
	if (!IS_VAMPAFF(ch, VAM_OBTENEBRATION))
	{
		send_to_char("You are not trained in the Obtenebration discipline.\n\r", ch);
		return;
	}
	if (ch->pcdata->condition[COND_THIRST] < 75)
	{
		send_to_char("You have insufficient blood.\n\r", ch);
		return;
	}
	/* Palmer added here */
	if (IS_SET(ch->in_room->room_flags, ROOM_NO_SHADOWPLANE))
	{
		send_to_char("This room has no shadowplane counterpart.\n\r", ch);
		return;
	}
	ch->pcdata->condition[COND_THIRST] -= number_range(65, 75);
	if (arg[0] == '\0')
	{
		if (!IS_AFFECTED(ch, AFF_SHADOWPLANE))
		{
			send_to_char("You fade into the plane of shadows.\n\r", ch);
			act("The shadows flicker and swallow up $n.", ch, NULL, NULL, TO_ROOM);
			SET_BIT(ch->affected_by, AFF_SHADOWPLANE);
			do_look(ch, "auto");
			return;
		}
		REMOVE_BIT(ch->affected_by, AFF_SHADOWPLANE);
		send_to_char("You fade back into the real world.\n\r", ch);
		act("The shadows flicker and $n fades into existance.", ch, NULL, NULL, TO_ROOM);
		do_look(ch, "auto");
		return;
	}

	if ((obj = get_obj_here(ch, arg)) == NULL)
	{
		send_to_char("What do you wish to toss into the shadow plane?\n\r", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_SHADOWPLANE))
		send_to_char("You toss it to the ground and it vanishes.\n\r", ch);
	else
		send_to_char("You toss it into a shadow and it vanishes.\n\r", ch);
	return;
	/* Code for shadowplane equip */

	if (IS_OBJ_STAT(obj, ITEM_SHADOWPLANE) && !IS_AFFECTED(ch, AFF_SHADOWPLANE))
	{
		act("You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR);
		act("$n is zapped by $p and drops it.", ch, obj, NULL, TO_ROOM);
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
		return;
	}
}

void do_immune(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	send_to_char("--------------------------------------------------------------------------------\n\r", ch);
	send_to_char("                                -= Immunities =-\n\r", ch);
	send_to_char("--------------------------------------------------------------------------------\n\r", ch);
	/* Display weapon resistances */
	send_to_char("Weapons:", ch);
	if (!(!IS_IMMUNE(ch, IMM_SLASH) && !IS_IMMUNE(ch, IMM_STAB) &&
		  !IS_IMMUNE(ch, IMM_SMASH) && !IS_IMMUNE(ch, IMM_ANIMAL) &&
		  !IS_IMMUNE(ch, IMM_MISC)))
	{
		if (IS_IMMUNE(ch, IMM_SLASH))
			send_to_char(" Slash Slice", ch);
		if (IS_IMMUNE(ch, IMM_STAB))
			send_to_char(" Stab Pierce", ch);
		if (IS_IMMUNE(ch, IMM_SMASH))
			send_to_char(" Blast Crush Pound", ch);
		if (IS_IMMUNE(ch, IMM_ANIMAL))
			send_to_char(" Claw Bite", ch);
		if (IS_IMMUNE(ch, IMM_MISC))
			send_to_char(" Grep Whip Suck", ch);
	}
	else
		send_to_char(" None", ch);
	send_to_char(".\n\r", ch);

	/* Display spell immunities */
	send_to_char("Spells :", ch);
	if (!(!IS_IMMUNE(ch, IMM_CHARM) && !IS_IMMUNE(ch, IMM_HEAT) &&
		  !IS_IMMUNE(ch, IMM_COLD) && !IS_IMMUNE(ch, IMM_LIGHTNING) &&
		  !IS_IMMUNE(ch, IMM_ACID) && !IS_IMMUNE(ch, IMM_SUMMON) &&
		  !IS_IMMUNE(ch, IMM_VOODOO) && !IS_IMMUNE(ch, IMM_SLEEP) &&
		  !IS_IMMUNE(ch, IMM_DRAIN)))
	{
		if (IS_IMMUNE(ch, IMM_CHARM))
			send_to_char(" Charm", ch);
		if (IS_IMMUNE(ch, IMM_HEAT))
			send_to_char(" #rH#Re#ra#Rt#e", ch);
		if (IS_IMMUNE(ch, IMM_COLD))
			send_to_char(" #cC#Co#cl#Cd#e", ch);
		if (IS_IMMUNE(ch, IMM_LIGHTNING))
			send_to_char(" #lL#wi#lg#wh#lt#wn#li#wn#lg#e", ch);
		if (IS_IMMUNE(ch, IMM_ACID))
			send_to_char(" #gA#Gc#gi#Gd#e", ch);
		if (IS_IMMUNE(ch, IMM_SUMMON))
			send_to_char(" Summon", ch);
		if (IS_IMMUNE(ch, IMM_VOODOO))
			send_to_char(" Voodoo", ch);
		if (IS_IMMUNE(ch, IMM_SLEEP))
			send_to_char(" Sleep", ch);
		if (IS_IMMUNE(ch, IMM_DRAIN))
			send_to_char(" Drain", ch);
	}
	else
		send_to_char(" None", ch);
	send_to_char(".\n\r", ch);

	/* Display skill immunities */
	send_to_char("Skills :", ch);
	if (!(!IS_IMMUNE(ch, IMM_HURL) && !IS_IMMUNE(ch, IMM_BACKSTAB) &&
		  !IS_IMMUNE(ch, IMM_KICK) && !IS_IMMUNE(ch, IMM_DISARM) &&
		  !IS_IMMUNE(ch, IMM_STEAL)))
	{
		if (IS_IMMUNE(ch, IMM_HURL))
			send_to_char(" Hurl", ch);
		if (IS_IMMUNE(ch, IMM_BACKSTAB))
			send_to_char(" Backstab", ch);
		if (IS_IMMUNE(ch, IMM_KICK))
			send_to_char(" Kick", ch);
		if (IS_IMMUNE(ch, IMM_DISARM))
			send_to_char(" Disarm", ch);
		if (IS_IMMUNE(ch, IMM_STEAL))
			send_to_char(" Steal", ch);
	}
	else
		send_to_char(" None", ch);
	send_to_char(".\n\r", ch);

	/* Display vampire immunities */
	send_to_char("Vampire:", ch);
	if (!(IS_IMMUNE(ch, IMM_VAMPIRE) && !IS_IMMUNE(ch, IMM_STAKE) &&
		  !IS_IMMUNE(ch, IMM_SUNLIGHT)))
	{
		if (!IS_IMMUNE(ch, IMM_VAMPIRE))
			send_to_char(" Bite", ch);
		if (IS_IMMUNE(ch, IMM_STAKE) || IS_EXTRA(ch, EXTRA_TRUSTED))
			send_to_char(" Stake", ch);
		if (IS_IMMUNE(ch, IMM_SUNLIGHT))
			send_to_char(" Sunlight", ch);
	}
	else
		send_to_char(" None", ch);
	send_to_char(".\n\r", ch);
	send_to_char("--------------------------------------------------------------------------------\n\r", ch);
	return;
}

void do_introduce(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	if (!IS_SET(ch->act, PLR_VAMPIRE) || (ch->vampgen < 1) || (ch->vampgen > 7))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
	do_tradition(ch, ch->lord);
	return;
}

void do_tradition(CHAR_DATA *ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char arg4[MAX_INPUT_LENGTH];
	char arg5[MAX_INPUT_LENGTH];
	char arg6[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	char buf3[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg1, MAX_INPUT_LENGTH);
	argument = one_argument(argument, arg2, MAX_INPUT_LENGTH);
	argument = one_argument(argument, arg3, MAX_INPUT_LENGTH);
	argument = one_argument(argument, arg4, MAX_INPUT_LENGTH);
	argument = one_argument(argument, arg5, MAX_INPUT_LENGTH);
	argument = one_argument(argument, arg6, MAX_INPUT_LENGTH);

	/*
    if (arg1 != '\0') sprintf (arg1,"%s",capitalize(arg1));
    if (arg2 != '\0') sprintf (arg2,"%s",capitalize(arg2));
    if (arg3 != '\0') sprintf (arg3,"%s",capitalize(arg3));
    if (arg4 != '\0') sprintf (arg4,"%s",capitalize(arg4));
    if (arg5 != '\0') sprintf (arg5,"%s",capitalize(arg5));
    if (arg6 != '\0') sprintf (arg6,"%s",capitalize(arg6));
    */

	if (arg1 != '\0')
		arg1[0] = UPPER(arg1[0]);
	if (arg2 != '\0')
		arg2[0] = UPPER(arg2[0]);
	if (arg3 != '\0')
		arg3[0] = UPPER(arg3[0]);
	if (arg4 != '\0')
		arg4[0] = UPPER(arg4[0]);
	if (arg5 != '\0')
		arg5[0] = UPPER(arg5[0]);
	if (arg6 != '\0')
		arg6[0] = UPPER(arg6[0]);

	if (IS_NPC(ch))
		return;

	if (!IS_SET(ch->act, PLR_VAMPIRE) || (ch->vampgen < 1) || (ch->vampgen > 7))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
	if (!str_cmp(ch->clan, "") && ch->vampgen != 1)
	{
		send_to_char("Not until you've created your own clan!\n\r", ch);
		return;
	}

	if (ch->vampgen == 7)
	{
		snprintf(buf2, MAX_INPUT_LENGTH, "Seventh");
		snprintf(buf3, MAX_INPUT_LENGTH, "Fledgling");
	}
	else if (ch->vampgen == 6)
	{
		snprintf(buf2, MAX_INPUT_LENGTH, "Sixth");
		snprintf(buf3, MAX_INPUT_LENGTH, "Neonate");
	}
	else if (ch->vampgen == 5)
	{
		snprintf(buf2, MAX_INPUT_LENGTH, "Fifth");
		snprintf(buf3, MAX_INPUT_LENGTH, "Ancilla");
	}
	else if (ch->vampgen == 4)
	{
		snprintf(buf2, MAX_INPUT_LENGTH, "Fourth");
		snprintf(buf3, MAX_INPUT_LENGTH, "Elder");
	}
	else if (ch->vampgen == 3)
	{
		snprintf(buf2, MAX_INPUT_LENGTH, "Third");
		snprintf(buf3, MAX_INPUT_LENGTH, "Methuselah");
	}
	else if (ch->vampgen == 2)
	{
		snprintf(buf2, MAX_INPUT_LENGTH, "Second");
		snprintf(buf3, MAX_INPUT_LENGTH, "Antediluvian");
	}
	if (ch->vampgen == 1)
		snprintf(buf, MAX_INPUT_LENGTH, "As is the tradition, I recite the lineage of %s, Sire of all Kindred.", ch->name);
	else
		snprintf(buf, MAX_INPUT_LENGTH, "As is the tradition, I recite the lineage of %s, %s of the %s Generation.", ch->name, buf3, buf2);
	do_say(ch, buf);
	if (ch->vampgen != 1)
	{
		if (ch->vampgen == 7)
			snprintf(buf2, MAX_INPUT_LENGTH, "%s", arg6);
		else if (ch->vampgen == 6)
			snprintf(buf2, MAX_INPUT_LENGTH, "%s", arg5);
		else if (ch->vampgen == 5)
			snprintf(buf2, MAX_INPUT_LENGTH, "%s", arg4);
		else if (ch->vampgen == 4)
			snprintf(buf2, MAX_INPUT_LENGTH, "%s", arg3);
		else if (ch->vampgen == 3)
			snprintf(buf2, MAX_INPUT_LENGTH, "%s", arg2);
		else if (ch->vampgen == 2)
			snprintf(buf2, MAX_INPUT_LENGTH, "%s", arg1);

		if (ch->vampgen == 2)
			snprintf(buf, MAX_INPUT_LENGTH, "My name is %s.  I am the founder of %s.  My sire is %s.", ch->name, ch->clan, buf2);
		else
			snprintf(buf, MAX_INPUT_LENGTH, "My name is %s.  I am born of %s.  My sire is %s.", ch->name, ch->clan, buf2);
		do_say(ch, buf);
	}
	if (arg6[0] != '\0')
	{
		snprintf(buf, MAX_INPUT_LENGTH, "My name is %s.  I am born of %s.  My sire is %s.",
				 arg6, ch->clan, arg5);
		do_say(ch, buf);
	}
	if (arg5[0] != '\0')
	{
		snprintf(buf, MAX_INPUT_LENGTH, "My name is %s.  I am born of %s.  My sire is %s.",
				 arg5, ch->clan, arg4);
		do_say(ch, buf);
	}
	if (arg4[0] != '\0')
	{
		snprintf(buf, MAX_INPUT_LENGTH, "My name is %s.  I am born of %s.  My sire is %s.",
				 arg4, ch->clan, arg3);
		do_say(ch, buf);
	}
	if (arg3[0] != '\0')
	{
		snprintf(buf, MAX_INPUT_LENGTH, "My name is %s.  I am born of %s.  My sire is %s.",
				 arg3, ch->clan, arg2);
		do_say(ch, buf);
	}
	if (arg2[0] != '\0')
	{
		if (arg1[0] != '\0')
			snprintf(buf, MAX_INPUT_LENGTH, "My name is %s.  I am the founder of %s.  My sire is %s.", arg2, ch->clan, arg1);
		else
			snprintf(buf, MAX_INPUT_LENGTH, "My name is %s.  I am born of %s.  My sire is %s.", arg2, ch->clan, arg1);
		do_say(ch, buf);
	}
	if (ch->vampgen == 1)
		snprintf(buf, MAX_INPUT_LENGTH, "My name is %s.  All Kindred are my childer.", ch->name);
	else
		snprintf(buf, MAX_INPUT_LENGTH, "My name is %s.  All Kindred are my childer.", arg1);
	do_say(ch, buf);
	if (ch->vampgen == 7)
		snprintf(buf, MAX_INPUT_LENGTH, "My name is %s, childe of %s, childe of %s, childe of %s, childe of %s, childe of %s, childe of %s.  Recognize my lineage.", ch->name, arg6, arg5, arg4, arg3, arg2, arg1);
	if (ch->vampgen == 6)
		snprintf(buf, MAX_INPUT_LENGTH, "My name is %s, childe of %s, childe of %s, childe of %s, childe of %s, childe of %s.  Recognize my lineage.", ch->name, arg5, arg4, arg3, arg2, arg1);
	if (ch->vampgen == 5)
		snprintf(buf, MAX_INPUT_LENGTH, "My name is %s, childe of %s, childe of %s, childe of %s, childe of %s.  Recognize my lineage.", ch->name, arg4, arg3, arg2, arg1);
	if (ch->vampgen == 4)
		snprintf(buf, MAX_INPUT_LENGTH, "My name is %s, childe of %s, childe of %s, childe of %s.  Recognize my lineage.", ch->name, arg3, arg2, arg1);
	if (ch->vampgen == 3)
		snprintf(buf, MAX_INPUT_LENGTH, "My name is %s, childe of %s, childe of %s.  Recognize my lineage.", ch->name, arg2, arg1);
	if (ch->vampgen == 2)
		snprintf(buf, MAX_INPUT_LENGTH, "My name is %s, childe of %s.  Recognize my lineage.", ch->name, arg1);
	if (ch->vampgen == 1)
		snprintf(buf, MAX_INPUT_LENGTH, "My name is %s.  Recognize my lineage.", ch->name);
	do_say(ch, buf);
	return;
}

void do_truesight(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
		return;

	if (!IS_SET(ch->act, PLR_VAMPIRE))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
	if (!IS_VAMPAFF(ch, VAM_AUSPEX))
	{
		send_to_char("You are not trained in the Auspex discipline.\n\r", ch);
		return;
	}

	if (IS_SET(ch->act, PLR_HOLYLIGHT))
	{
		REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
		send_to_char("Your senses return to normal.\n\r", ch);
	}
	else
	{
		SET_BIT(ch->act, PLR_HOLYLIGHT);
		send_to_char("Your senses increase to incredable proportions.\n\r", ch);
	}

	return;
}

void do_scry(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	ROOM_INDEX_DATA *chroom;
	ROOM_INDEX_DATA *victimroom;
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	if (!IS_SET(ch->act, PLR_VAMPIRE))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!IS_VAMPAFF(ch, VAM_AUSPEX))
	{
		send_to_char("You are not trained in the Auspex discipline.\n\r", ch);
		return;
	}

	if (arg[0] == '\0')
	{
		send_to_char("Scry on whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (!IS_NPC(victim) && (victim->level > ch->level))
	{
		send_to_char("Nope.\n\r", ch);
		return;
	}

	if (ch->pcdata->condition[COND_THIRST] < 25)
	{
		send_to_char("You have insufficient blood.\n\r", ch);
		return;
	}
	ch->pcdata->condition[COND_THIRST] -= number_range(15, 25);

	if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_SHIELDED) && number_percent() > 5)
	{
		send_to_char("You are unable to locate them.\n\r", ch);
		return;
	}

	if (!IS_NPC(victim) && (victim->level > ch->level))
	{
		send_to_char("You cannot scry that person.\n\r", ch);
		return;
	}

	chroom = get_room_index(ch->in_room->vnum);
	victimroom = get_room_index(victim->in_room->vnum);

	if ((victimroom->vnum > 20999) && (victimroom->vnum < 21099) && (ch->level < 4))
	{
		send_to_char("You are unable to locate them.\n\r", ch);
		return;
	}

	if ((victimroom->vnum > 19999) && (victimroom->vnum < 20010) && (ch->level < 4))
	{
		send_to_char("You are unable to locate them.\n\r", ch);
		return;
	}

	char_from_room(ch);
	char_to_room(ch, victimroom);
	if (IS_AFFECTED(ch, AFF_SHADOWPLANE) && (!IS_AFFECTED(victim, AFF_SHADOWPLANE)))
	{
		REMOVE_BIT(ch->affected_by, AFF_SHADOWPLANE);
		do_look(ch, "auto");
		SET_BIT(ch->affected_by, AFF_SHADOWPLANE);
	}
	else if (!IS_AFFECTED(ch, AFF_SHADOWPLANE) && (IS_AFFECTED(victim, AFF_SHADOWPLANE)))
	{
		SET_BIT(ch->affected_by, AFF_SHADOWPLANE);
		do_look(ch, "auto");
		REMOVE_BIT(ch->affected_by, AFF_SHADOWPLANE);
	}
	else
		do_look(ch, "auto");
	char_from_room(ch);
	char_to_room(ch, chroom);

	return;
}

void do_readaura(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	if (!IS_SET(ch->act, PLR_VAMPIRE))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (!IS_VAMPAFF(ch, VAM_AUSPEX))
	{
		send_to_char("You are not trained in the Auspex discipline.\n\r", ch);
		return;
	}

	if (arg[0] == '\0')
	{
		send_to_char("Read the aura on what?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		if ((obj = get_obj_carry(ch, arg)) == NULL)
		{
			send_to_char("Read the aura on what?\n\r", ch);
			return;
		}
		if (ch->pcdata->condition[COND_THIRST] < 50)
		{
			send_to_char("You have insufficient blood.\n\r", ch);
			return;
		}
		ch->pcdata->condition[COND_THIRST] -= number_range(40, 50);
		act("$n examines $p intently.", ch, obj, NULL, TO_ROOM);
		spell_identify(skill_lookup("identify"), ch->level, ch, obj);
		return;
	}

	if (ch->pcdata->condition[COND_THIRST] < 50)
	{
		send_to_char("You have insufficient blood.\n\r", ch);
		return;
	}
	ch->pcdata->condition[COND_THIRST] -= number_range(40, 50);
	if (!IS_NPC(victim) && IS_IMMUNE(victim, IMM_SHIELDED))
	{
		send_to_char("You are unable to read their aura.\n\r", ch);
		return;
	}

	act("$n examines $N intently.", ch, NULL, victim, TO_NOTVICT);
	act("$n examines you intently.", ch, NULL, victim, TO_VICT);
	if (IS_NPC(victim))
		snprintf(buf, MAX_INPUT_LENGTH, "%s is an NPC.\n\r", victim->short_descr);
	else
	{
		if (victim->level == 9)
			snprintf(buf, MAX_INPUT_LENGTH, "%s is the Palmer!.\n\r", victim->name);
		if (victim->level == 8)
			snprintf(buf, MAX_INPUT_LENGTH, "%s is a God.\n\r", victim->name);
		if (victim->level == 7)
			snprintf(buf, MAX_INPUT_LENGTH, "%s is an Oracle.\n\r", victim->name);
		if (victim->level == 6)
			snprintf(buf, MAX_INPUT_LENGTH, "%s is a Seer.\n\r", victim->name);
		if (victim->level == 5)
			snprintf(buf, MAX_INPUT_LENGTH, "%s is an Elder.\n\r", victim->name);
		if (victim->level == 4)
			snprintf(buf, MAX_INPUT_LENGTH, "%s is a Justicar.\n\r", victim->name);
		if (victim->level == 3)
			snprintf(buf, MAX_INPUT_LENGTH, "%s is an Avatar.\n\r", victim->name);
		else
			snprintf(buf, MAX_INPUT_LENGTH, "%s is a Mortal.\n\r", victim->name);
	}
	send_to_char(buf, ch);
	if (!IS_NPC(victim))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "Str:%d, Int:%d, Wis:%d, Dex:%d, Con:%d.\n\r", get_curr_str(victim), get_curr_int(victim), get_curr_wis(victim), get_curr_dex(victim), get_curr_con(victim));
		send_to_char(buf, ch);
	}
	snprintf(buf, MAX_INPUT_LENGTH, "Hp:%d/%d, Mana:%d/%d, Move:%d/%d.\n\r", victim->hit, victim->max_hit, victim->mana, victim->max_mana, victim->move, victim->max_move);
	send_to_char(buf, ch);
	if (!IS_NPC(victim))
		snprintf(buf, MAX_INPUT_LENGTH, "Hitroll:%d, Damroll:%d, Armor:%d.\n\r", GET_HITROLL(victim), GET_DAMROLL(victim), GET_ARMOR(victim));
	else
		snprintf(buf, MAX_INPUT_LENGTH, "Armor:%d.\n\r", GET_ARMOR(victim));
	send_to_char(buf, ch);
	if (!IS_NPC(victim))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "Status:%d, ", victim->race);
		send_to_char(buf, ch);
		if (IS_SET(victim->act, PLR_VAMPIRE))
		{
			snprintf(buf, MAX_INPUT_LENGTH, "Blood:%d, ", victim->pcdata->condition[COND_THIRST]);
			send_to_char(buf, ch);
		}
	}
	snprintf(buf, MAX_INPUT_LENGTH, "Alignment:%d.\n\r", victim->alignment);
	send_to_char(buf, ch);
	if (!IS_NPC(victim) && IS_SET(victim->act, PLR_VAMPIRE))
	{
		send_to_char("Disciplines:", ch);
		if (IS_VAMPAFF(victim, VAM_CELERITY))
			send_to_char(" Celerity", ch);
		if (IS_VAMPAFF(victim, VAM_FORTITUDE))
			send_to_char(" Fortitude", ch);
		if (IS_VAMPAFF(victim, VAM_POTENCE))
			send_to_char(" Potence", ch);
		if (IS_VAMPAFF(victim, VAM_OBFUSCATE))
			send_to_char(" Obfuscate", ch);
		if (IS_VAMPAFF(victim, VAM_OBTENEBRATION))
			send_to_char(" Obtenebration", ch);
		if (IS_VAMPAFF(victim, VAM_AUSPEX))
			send_to_char(" Auspex", ch);
		send_to_char(".\n\r", ch);
	}
	return;
}

void do_mortal(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int blood;

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;
	if (!IS_SET(ch->act, PLR_VAMPIRE) && !IS_VAMPAFF(ch, VAM_MORTAL))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
	if (!IS_VAMPAFF(ch, VAM_OBFUSCATE))
	{
		send_to_char("You are not trained in the Obfuscate discipline.\n\r", ch);
		return;
	}
	if (IS_SET(ch->act, PLR_VAMPIRE))
	{
		if (ch->pcdata->condition[COND_THIRST] < 101)
		{
			send_to_char("Don't be silly, your a Vampire !\n\r", ch);
			return;
		}
		/* Have to make sure they have enough blood to change back */
		blood = ch->pcdata->condition[COND_THIRST];
		ch->pcdata->condition[COND_THIRST] = 666;

		/* Remove physical vampire attributes when you take mortal form */
		if (IS_VAMPAFF(ch, VAM_DISGUISED))
			do_mask(ch, ch->name);
		if (IS_IMMUNE(ch, IMM_SHIELDED))
			do_shield(ch, "");
		if (IS_AFFECTED(ch, AFF_SHADOWPLANE))
			do_shadowplane(ch, "");
		if (IS_VAMPAFF(ch, VAM_FANGS))
			do_fangs(ch, "");
		if (IS_VAMPAFF(ch, VAM_NIGHTSIGHT))
			do_nightsight(ch, "");
		if (IS_AFFECTED(ch, AFF_SHADOWSIGHT))
			do_shadowsight(ch, "");
		if (IS_SET(ch->act, PLR_HOLYLIGHT))
			do_truesight(ch, "");
		if (IS_VAMPAFF(ch, VAM_CHANGED))
			do_change(ch, "human");
		ch->pcdata->condition[COND_THIRST] = blood;

		send_to_char("Colour returns to your skin and you warm up a little.\n\r", ch);
		act("Colour returns to $n's skin.", ch, NULL, NULL, TO_ROOM);

		REMOVE_BIT(ch->act, PLR_VAMPIRE);
		SET_BIT(ch->vampaff, VAM_MORTAL);
		return;
	}
	send_to_char("You skin pales and cools.\n\r", ch);
	act("$n's skin pales slightly.", ch, NULL, NULL, TO_ROOM);
	SET_BIT(ch->act, PLR_VAMPIRE);
	REMOVE_BIT(ch->vampaff, VAM_MORTAL);
	return;
}

void do_mortalvamp(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int blood;

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;
	if (IS_SET(ch->act, PLR_VAMPIRE))
	{
		/* Have to make sure they have enough blood to change back */
		blood = ch->pcdata->condition[COND_THIRST];
		ch->pcdata->condition[COND_THIRST] = 666;

		/* Remove physical vampire attributes when you take mortal form */
		if (IS_VAMPAFF(ch, VAM_DISGUISED))
			do_mask(ch, ch->name);
		if (IS_IMMUNE(ch, IMM_SHIELDED))
			do_shield(ch, "");
		if (IS_AFFECTED(ch, AFF_SHADOWPLANE))
			do_shadowplane(ch, "");
		if (IS_VAMPAFF(ch, VAM_FANGS))
			do_fangs(ch, "");
		if (IS_VAMPAFF(ch, VAM_NIGHTSIGHT))
			do_nightsight(ch, "");
		if (IS_AFFECTED(ch, AFF_SHADOWSIGHT))
			do_shadowsight(ch, "");
		if (IS_SET(ch->act, PLR_HOLYLIGHT))
			do_truesight(ch, "");
		if (IS_VAMPAFF(ch, VAM_CHANGED))
			do_change(ch, "human");
		
		CLANDISC_DATA *disc;

		for(disc = ch->clandisc; disc != NULL; disc = disc->next)
		{
			if(disc->isActive)
			{
				(disc->do_ability) ( ch, disc, "" );
			}
		}

		ch->pcdata->condition[COND_THIRST] = blood;

		send_to_char("You lose your vampire powers.\n\r", ch);

		REMOVE_BIT(ch->act, PLR_VAMPIRE);
		SET_BIT(ch->vampaff, VAM_MORTAL);
		return;
	}
	send_to_char("You regain your vampire powers.\n\r", ch);
	SET_BIT(ch->act, PLR_VAMPIRE);
	REMOVE_BIT(ch->vampaff, VAM_MORTAL);
	return;
}

void do_shield(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;
	if (!IS_SET(ch->act, PLR_VAMPIRE))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
	if (!IS_VAMPAFF(ch, VAM_OBFUSCATE) && !IS_VAMPAFF(ch, VAM_DOMINATE))
	{
		send_to_char("You are not trained in the Obfuscate or Dominate disciplines.\n\r", ch);
		return;
	}
	if (ch->pcdata->condition[COND_THIRST] < 60)
	{
		send_to_char("You have insufficient blood.\n\r", ch);
		return;
	}
	ch->pcdata->condition[COND_THIRST] -= number_range(50, 60);
	if (!IS_IMMUNE(ch, IMM_SHIELDED))
	{
		send_to_char("You shield your aura from those around you.\n\r", ch);
		SET_BIT(ch->immune, IMM_SHIELDED);
		return;
	}
	send_to_char("You stop shielding your aura.\n\r", ch);
	REMOVE_BIT(ch->immune, IMM_SHIELDED);
	return;
}

void do_regenerate(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int ageadd;

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	ageadd = get_age(ch) / 20 + (ch->remortlevel * 5);

	if (!IS_SET(ch->act, PLR_VAMPIRE))
	{
		send_to_char("How exactly do you expect to do that?\n\r", ch);
		return;
	}

	if (ch->position == POS_FIGHTING)
	{
		send_to_char("You cannot regenerate while fighting.\n\r", ch);
		return;
	}

	if (ch->hit >= ch->max_hit && ch->mana >= ch->max_mana && ch->move >= ch->max_move)
	{
		send_to_char("But you are already completely regenerated!\n\r", ch);
		return;
	}

	if (ch->pcdata->condition[COND_THIRST] < 5)
	{
		send_to_char("You have insufficient blood.\n\r", ch);
		return;
	}

	ch->pcdata->condition[COND_THIRST] -= number_range(2, 5);
	if (ch->hit >= ch->max_hit && ch->mana >= ch->max_mana && ch->move >= ch->max_move)
	{
		send_to_char("Your body has completely regenerated.\n\r", ch);
		act("$n's body completely regenerates itself.", ch, NULL, NULL, TO_ROOM);
	}
	else
		send_to_char("Your body slowly regenerates itself.\n\r", ch);
	if (ch->hit < 1)
	{
		ch->hit = ch->hit + 1;
		update_pos(ch);
		WAIT_STATE(ch, 24);
	}
	else
	{ /* Palmer altered here */
		ch->hit += 13 + ageadd + ((13 - ch->vampgen) * 4);

		if (ch->hit > ch->max_hit)
			ch->hit = ch->max_hit;
		
		ch->mana += 13 + ageadd + ((13 - ch->vampgen) * 4);
		
		if (ch->mana > ch->max_mana)
			ch->mana = ch->max_mana;
		
		ch->move += 13 + ageadd + ((13 - ch->vampgen) * 4);
		
		if (ch->move > ch->max_move)
			ch->move = ch->max_move;

		update_pos(ch);
	}
	return;
}

void do_rage(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	send_to_char("You scream with rage as the beast within consumes you!\n\r", ch);
	if (IS_AFFECTED(ch, AFF_POLYMORPH))
		snprintf(buf, MAX_INPUT_LENGTH, "%s screams with rage as $s inner beast consumes $m!.", ch->morph);
	else
		snprintf(buf, MAX_INPUT_LENGTH, "$n screams with rage as $s inner beast consumes $m!.");
	act(buf, ch, NULL, NULL, TO_ROOM);
	for (vch = char_list; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next;
		if (vch->in_room == NULL)
			continue;
		if (ch == vch)
			continue;
		if (vch->in_room == ch->in_room)
		{
			if (can_see(ch, vch))
				multi_hit(ch, vch, TYPE_UNDEFINED);
		}
	}
	do_beastlike(ch, "");
	return;
}

/* you have a (1.0/last_point) chance of getting beast from 1 to 0, or from 99 to 100 */
const float last_point = 5000.0f;

void do_humanity(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int chance;
	const float power_base = powf(last_point, 1.f / 99.f);

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	/* exponential chance to lower, 100% at beast 100 */
	chance = powf(power_base, 100 - ch->beast);

	/*    chance = 101 - ch->beast;
    if (ch->beast < 25)
    {
        chance = (100-ch->beast) * LAST_POINT / 100;
    }*/

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_VAMPIRE) && ch->beast > 0 && number_range(1, chance) == 1)
	{
		if (ch->beast == 1)
		{
			send_to_char("You have reached Golconda!\n\r", ch);

			if (!IS_IMMUNE(ch, IMM_SUNLIGHT))
				SET_BIT(ch->immune, IMM_SUNLIGHT);
		}
		else
			send_to_char("You feel slightly more in control of your beast.\n\r", ch);
		ch->beast -= 1;
	}
	return;
}

void do_beastlike(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int blood;
	int chance;
	const float power_base = powf(last_point, 1.f / 99.f);

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	/* exponential chance to raise, 100% at beast 0 */
	chance = powf(power_base, ch->beast);

	/*    chance = ch->beast+1;
    if (ch->beast > 75)
    {
        chance = ch->beast * LAST_POINT / 100;
    }*/

	if (!IS_NPC(ch) && IS_SET(ch->act, PLR_VAMPIRE) && ch->beast < 100 && number_range(1, chance) == 1)
	{
		if (ch->beast < 99)
			send_to_char("You feel your beast take more control over your actions.\n\r", ch);
		else
			send_to_char("Your beast has fully taken over control of your actions!\n\r", ch);

		/* palmer added here */
		if (ch->beast == 0)
		{
			REMOVE_BIT(ch->immune, IMM_SUNLIGHT);
			send_to_char("Your beast goes up and you lose your immunity to sunlight.\n\r", ch);
		}
		ch->beast += 1;

		blood = ch->pcdata->condition[COND_THIRST];
		ch->pcdata->condition[COND_THIRST] = 666;
		if (IS_VAMPAFF(ch, VAM_OBTENEBRATION) && !IS_VAMPAFF(ch, VAM_NIGHTSIGHT))
			do_nightsight(ch, "");
		if (!IS_VAMPAFF(ch, VAM_FANGS))
			do_fangs(ch, "");
		ch->pcdata->condition[COND_THIRST] = blood;
	}
	return;
}

void do_feed(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	char bufch[MAX_INPUT_LENGTH];
	char bufvi[MAX_INPUT_LENGTH];
	int blood;

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;
	if (!IS_SET(ch->act, PLR_VAMPIRE))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}
	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}
	if (IS_NPC(victim))
	{
		send_to_char("Not on NPC's.\n\r", ch);
		return;
	}
	if (!IS_SET(victim->act, PLR_VAMPIRE))
	{
		send_to_char("Blood does them no good at all.\n\r", ch);
		return;
	}
	if (!IS_IMMUNE(victim, IMM_VAMPIRE))
	{
		send_to_char("They refuse to drink your blood.\n\r", ch);
		return;
	}
	if (ch->pcdata->condition[COND_THIRST] < 20)
	{
		send_to_char("You don't have enough blood.\n\r", ch);
		return;
	}

	blood = number_range(5, 10);
	ch->pcdata->condition[COND_THIRST] -= (blood * 2);
	victim->pcdata->condition[COND_THIRST] += blood;
	if (victim->pcdata->condition[COND_THIRST] > 100)
		victim->pcdata->condition[COND_THIRST] = 100;

	if (IS_AFFECTED(ch, AFF_POLYMORPH))
		snprintf(bufch, MAX_INPUT_LENGTH, "%s", ch->morph);
	else
		snprintf(bufch, MAX_INPUT_LENGTH, "%s", ch->name);
	if (IS_AFFECTED(victim, AFF_POLYMORPH))
		snprintf(bufvi, MAX_INPUT_LENGTH, "%s", victim->morph);
	else
		snprintf(bufvi, MAX_INPUT_LENGTH, "%s", victim->name);
	snprintf(buf, MAX_INPUT_LENGTH, "You cut open your wrist and feed some blood to %s.", bufvi);
	act(buf, ch, NULL, victim, TO_CHAR);
	snprintf(buf, MAX_INPUT_LENGTH, "%s cuts open $s wrist and feeds some blood to %s.", bufch, bufvi);
	act(buf, ch, NULL, victim, TO_NOTVICT);
	if (victim->position < 5)
		send_to_char("You feel some blood poured down your throat.\n\r", victim);
	else
	{
		snprintf(buf, MAX_INPUT_LENGTH, "%s cuts open $s wrist and feeds you some blood.", bufch);
		act(buf, ch, NULL, victim, TO_VICT);
	}
	return;
}

void do_upkeep(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	CLANDISC_DATA *disc;

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;
	if (!IS_SET(ch->act, PLR_VAMPIRE))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	send_to_char("--------------------------------------------------------------------------------\n\r", ch);
	send_to_char("                              -= Vampire upkeep =-\n\r", ch);
	send_to_char("--------------------------------------------------------------------------------\n\r", ch);
	send_to_char("Staying alive...upkeep 1.\n\r", ch);

	for(disc = ch->clandisc; disc != NULL; disc = disc->next)
	{
		if(DiscIsActive(disc))
		{
			send_to_char(disc->upkeepMessage, ch);
		}
		else if(!DiscIsActive(disc) && disc->isPassiveAbility)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "#rYou do not have %s active.#e\n\r", disc->name);
			send_to_char(buf, ch);
		}
		
	}

	if (IS_VAMPAFF(ch, VAM_DISGUISED))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "You are disguised as %s...upkeep 1.\n\r", ch->morph);
		send_to_char(buf, ch);
	}
	if (IS_IMMUNE(ch, IMM_SHIELDED))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "You are shielded...upkeep 1.\n\r");
		send_to_char(buf, ch);
	}
	if (IS_AFFECTED(ch, AFF_SHADOWPLANE))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "You are in the shadowplane...upkeep 1.\n\r");
		send_to_char(buf, ch);
	}
	if (IS_VAMPAFF(ch, VAM_FANGS))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "You have your fangs out...upkeep 1.\n\r");
		send_to_char(buf, ch);
	}
	if (IS_VAMPAFF(ch, VAM_NIGHTSIGHT))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "You have nightsight...upkeep 1.\n\r");
		send_to_char(buf, ch);
	}
	if (IS_AFFECTED(ch, AFF_SHADOWSIGHT))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "You have shadowsight...upkeep 1.\n\r");
		send_to_char(buf, ch);
	}
	if (IS_SET(ch->act, PLR_HOLYLIGHT))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "You have truesight...upkeep 1.\n\r");
		send_to_char(buf, ch);
	}
	if (IS_VAMPAFF(ch, VAM_CHANGED))
	{
		if (IS_POLYAFF(ch, POLY_BAT))
			snprintf(buf2, MAX_INPUT_LENGTH, "bat");
		else if (IS_POLYAFF(ch, POLY_WOLF))
			snprintf(buf2, MAX_INPUT_LENGTH, "wolf");
		else
			snprintf(buf2, MAX_INPUT_LENGTH, "mist");
		snprintf(buf, MAX_INPUT_LENGTH, "You have changed into %s form...upkeep 1.\n\r", buf2);
		send_to_char(buf, ch);
	}
	if (IS_POLYAFF(ch, POLY_SERPENT))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "You are in serpent form...upkeep 1.\n\r");
		send_to_char(buf, ch);
	}
	send_to_char("--------------------------------------------------------------------------------\n\r", ch);
	return;
}

void do_vclan(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;
	if (!IS_SET(ch->act, PLR_VAMPIRE))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	snprintf(buf, MAX_STRING_LENGTH, "The %s clan:\n\r", ch->clan);
	send_to_char_formatted(buf, ch);
	send_to_char_formatted("[      Name      ] [ Gen ] [ Hits  % ] [ Mana  % ] [ Move  % ] [  Exp  ] [Blood]\n\r", ch);
	for (gch = char_list; gch != NULL; gch = gch->next)
	{
		if (IS_NPC(gch))
			continue;
		if (!IS_SET(gch->act, PLR_VAMPIRE))
			continue;
		if (ch->level == LEVEL_CODER)
		{
			snprintf(buf, MAX_STRING_LENGTH,
					 "[%-16s] [  %d  ] [%-6d%3d] [%-6d%3d] [%-6d%3d] [%7ld] [ %3d ]\n\r",
					 capitalize(gch->name),
					 gch->vampgen,
					 gch->hit, (gch->hit * 100 / gch->max_hit),
					 gch->mana, (gch->mana * 100 / gch->max_mana),
					 gch->move, (gch->move * 100 / gch->max_move),
					 gch->exp, gch->pcdata->condition[COND_THIRST]);
			send_to_char_formatted(buf, ch);
		}
		else
		{
			if (!str_cmp(ch->clan, gch->clan) && (gch->vampgen >= ch->vampgen))
			{
				snprintf(buf, MAX_STRING_LENGTH,
						 "[%-16s] [  %d  ] [%-6d%3d] [%-6d%3d] [%-6d%3d] [%7ld] [ %3d ]\n\r",
						 capitalize(gch->name),
						 gch->vampgen,
						 gch->hit, (gch->hit * 100 / gch->max_hit),
						 gch->mana, (gch->mana * 100 / gch->max_mana),
						 gch->move, (gch->move * 100 / gch->max_move),
						 gch->exp, gch->pcdata->condition[COND_THIRST]);
				send_to_char_formatted(buf, ch);
			}
		}
	}
	return;
}

void improve_wpn(CHAR_DATA *ch, int dtype, bool right_hand)
{
	OBJ_DATA *wield;
	char bufskill[20];
	char buf[MAX_INPUT_LENGTH];
	int dice1;
	int dice2;
	int trapper;

	dice1 = number_percent();
	dice2 = number_percent();
	if (right_hand)
		wield = get_eq_char(ch, WEAR_WIELD);
	else
		wield = get_eq_char(ch, WEAR_HOLD);

	if (IS_NPC(ch))
		return;

	if (wield == NULL)
		dtype = TYPE_HIT;
	if (dtype == TYPE_UNDEFINED)
	{
		dtype = TYPE_HIT;
		if (wield != NULL && IS_WEAPON(wield))
			dtype += wield->value[3];
	}
	if (dtype < 1000 || dtype > 1012)
		return;
	dtype -= 1000;
	if (ch->wpn[dtype] >= 200)
		return;
	trapper = ch->wpn[dtype];
	if ((dice1 > ch->wpn[dtype] && dice2 > ch->wpn[dtype]) || (dice1 == 100 || dice2 == 100))
		ch->wpn[dtype] += 1;
	else
		return;
	if (trapper == ch->wpn[dtype])
		return;

	if (ch->wpn[dtype] == 1)
		snprintf(bufskill, 20, "slightly skilled");
	else if (ch->wpn[dtype] == 26)
		snprintf(bufskill, 20, "reasonable");
	else if (ch->wpn[dtype] == 51)
		snprintf(bufskill, 20, "fairly competent");
	else if (ch->wpn[dtype] == 76)
		snprintf(bufskill, 20, "highly skilled");
	else if (ch->wpn[dtype] == 101)
		snprintf(bufskill, 20, "very dangerous");
	else if (ch->wpn[dtype] == 126)
		snprintf(bufskill, 20, "extremely deadly");
	else if (ch->wpn[dtype] == 151)
		snprintf(bufskill, 20, "an expert");
	else if (ch->wpn[dtype] == 176)
		snprintf(bufskill, 20, "a master");
	else if (ch->wpn[dtype] == 200)
		snprintf(bufskill, 20, "a grand master");
	else
		return;
	if (wield == NULL || dtype == 0)
		snprintf(buf, MAX_INPUT_LENGTH, "You are now %s at unarmed combat.\n\r", bufskill);
	else
		snprintf(buf, MAX_INPUT_LENGTH, "You are now %s with %s.\n\r", bufskill, wield->short_descr);
	ADD_COLOUR(ch, buf, WHITE, MAX_INPUT_LENGTH);
	send_to_char(buf, ch);
	return;
}

void improve_stance(CHAR_DATA *ch)
{
	char buf[MAX_INPUT_LENGTH];
	char bufskill[35];
	int dice1;
	int dice2;
	int stance;

	dice1 = number_percent() * 2;
	dice2 = number_percent() * 2;

	if (IS_NPC(ch))
		return;

	stance = ch->stance[CURRENT_STANCE];
	if (stance < 1 || stance > 10)
		return;
	if (dice1 > ch->stance[stance] && dice2 > ch->stance[stance])
		ch->stance[stance] += 1;
	else
		return;
	if (stance == ch->stance[stance])
		return;

	if (ch->stance[stance] == 1)
		snprintf(bufskill, 35, "an apprentice of");
	else if (ch->stance[stance] == 26)
		snprintf(bufskill, 35, "a trainee of");
	else if (ch->stance[stance] == 51)
		snprintf(bufskill, 35, "a student of");
	else if (ch->stance[stance] == 76)
		snprintf(bufskill, 35, "fairly experienced in");
	else if (ch->stance[stance] == 101)
		snprintf(bufskill, 35, "well trained in");
	else if (ch->stance[stance] == 126)
		snprintf(bufskill, 35, "highly skilled in");
	else if (ch->stance[stance] == 151)
		snprintf(bufskill, 35, "an expert of");
	else if (ch->stance[stance] == 176)
		snprintf(bufskill, 35, "a master of");
	else if (ch->stance[stance] == 186)
		snprintf(bufskill, 35, "more masterful of");
	else if (ch->stance[stance] == 196)
		snprintf(bufskill, 35, "even more masterful of");
	else if (ch->stance[stance] == 199)
		snprintf(bufskill, 35, "on the verge of grand mastery of");
	else if (ch->stance[stance] == 200)
		snprintf(bufskill, 35, "a grand master of");
	else
		return;

	snprintf(buf, MAX_INPUT_LENGTH, "You are now %s the %s stance.\n\r", bufskill, stancenames[stance]);
	ADD_COLOUR(ch, buf, WHITE, MAX_INPUT_LENGTH);
	send_to_char(buf, ch);
	return;
}

void do_alignment(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;
	if (ch->level != 2)
	{
		send_to_char("Only mortals have the power to change alignment.\n\r", ch);
		return;
	}

	if (arg[0] == '\0')
	{
		send_to_char("Please enter 'good', 'neutral', or 'evil'.\n\r", ch);
		return;
	}
	if (!str_cmp(arg, "good"))
	{
		ch->alignment = 1000;
		send_to_char("Alignment is now angelic.\n\r", ch);
	}
	else if (!str_cmp(arg, "neutral"))
	{
		ch->alignment = 0;
		send_to_char("Alignment is now neutral.\n\r", ch);
	}
	else if (!str_cmp(arg, "evil"))
	{
		ch->alignment = -1000;
		send_to_char("Alignment is now satanic.\n\r", ch);
	}
	else
		send_to_char("Please enter 'good', 'neutral', or 'evil'.\n\r", ch);
	return;
}

void do_side(CHAR_DATA *ch, char *argument)
{
	char buf[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	if (arg[0] != '\0' && !str_cmp(arg, "renegade") &&
		(ch->class == 1 || ch->class == 2))
	{
		ch->class = 3;
		send_to_char("You are now a renegade.\n\r", ch);
		snprintf(buf, MAX_INPUT_LENGTH, "%s has turned renegade!", ch->name);
		do_info(ch, buf);
		return;
	}

	if (ch->class == 1)
	{
		send_to_char("You are a follower of Palmer.\n\r", ch);
		return;
	}
	else if (ch->class == 2)
	{
		send_to_char("You are a follower of Daile.\n\r", ch);
		return;
	}
	else if (ch->class == 3)
	{
		send_to_char("You are a renegade.\n\r", ch);
		return;
	}

	if (arg[0] == '\0')
	{
		send_to_char("You have not yet picked a side to fight on.\n\r", ch);
		send_to_char("Please enter 'Palmer' or 'Daile'.\n\r", ch);
		return;
	}
	if (!str_cmp(arg, "palmer"))
	{
		ch->class = 1;
		send_to_char("You are now a follower of Palmer.\n\r", ch);
		snprintf(buf, MAX_INPUT_LENGTH, "%s has pledged allegence to Palmer!", ch->name);
		do_info(ch, buf);
	}
	else if (!str_cmp(arg, "daile"))
	{
		ch->class = 2;
		send_to_char("You are now a follower of Daile.\n\r", ch);
		snprintf(buf, MAX_INPUT_LENGTH, "%s has pledged allegence to Daile!", ch->name);
		do_info(ch, buf);
	}
	else
		send_to_char("Please enter 'Palmer' or 'Daile'.\n\r", ch);
	return;
}

void do_skill(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char buf[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	char bufskill[25];
	char bufskill2[25];
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *wield;
	OBJ_DATA *wield2;
	int dtype;
	int dtype2;

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	if (arg[0] == '\0')
		snprintf(arg, MAX_INPUT_LENGTH, "%s", ch->name);

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (IS_NPC(victim))
	{
		send_to_char("Not on NPC's.\n\r", ch);
		return;
	}

	wield = get_eq_char(victim, WEAR_WIELD);
	wield2 = get_eq_char(victim, WEAR_HOLD);

	dtype = TYPE_HIT;
	dtype2 = TYPE_HIT;
	if (wield != NULL && IS_WEAPON(wield))
		dtype += wield->value[3];
	if (wield2 != NULL && IS_WEAPON(wield2))
		dtype2 += wield2->value[3];
	dtype -= 1000;
	dtype2 -= 1000;

	if (victim->wpn[dtype] == 00)
		snprintf(bufskill, 25, "totally unskilled");
	else if (victim->wpn[dtype] <= 25)
		snprintf(bufskill, 25, "slightly skilled");
	else if (victim->wpn[dtype] <= 50)
		snprintf(bufskill, 25, "reasonable");
	else if (victim->wpn[dtype] <= 75)
		snprintf(bufskill, 25, "fairly competent");
	else if (victim->wpn[dtype] <= 100)
		snprintf(bufskill, 25, "highly skilled");
	else if (victim->wpn[dtype] <= 125)
		snprintf(bufskill, 25, "very dangerous");
	else if (victim->wpn[dtype] <= 150)
		snprintf(bufskill, 25, "extremely deadly");
	else if (victim->wpn[dtype] <= 175)
		snprintf(bufskill, 25, "an expert");
	else if (victim->wpn[dtype] <= 199)
		snprintf(bufskill, 25, "a master");
	else if (victim->wpn[dtype] >= 200)
		snprintf(bufskill, 25, "a grand master");
	else
		return;

	if (victim->wpn[dtype2] == 00)
		snprintf(bufskill2, 25, "totally unskilled");
	else if (victim->wpn[dtype2] <= 25)
		snprintf(bufskill2, 25, "slightly skilled");
	else if (victim->wpn[dtype2] <= 50)
		snprintf(bufskill2, 25, "reasonable");
	else if (victim->wpn[dtype2] <= 75)
		snprintf(bufskill2, 25, "fairly competent");
	else if (victim->wpn[dtype2] <= 100)
		snprintf(bufskill2, 25, "highly skilled");
	else if (victim->wpn[dtype2] <= 125)
		snprintf(bufskill2, 25, "very dangerous");
	else if (victim->wpn[dtype2] <= 150)
		snprintf(bufskill2, 25, "extremely deadly");
	else if (victim->wpn[dtype2] <= 175)
		snprintf(bufskill2, 25, "an expert");
	else if (victim->wpn[dtype2] <= 199)
		snprintf(bufskill2, 25, "a master");
	else if (victim->wpn[dtype2] >= 200)
		snprintf(bufskill2, 25, "a grand master");
	else
		return;

	if (ch == victim)
	{
		if (dtype == 0 && dtype2 == 0)
			snprintf(buf, MAX_INPUT_LENGTH, "You are %s at unarmed combat.\n\r", bufskill);
		else
		{
			if (dtype != 0)
				snprintf(buf, MAX_INPUT_LENGTH, "You are %s with %s.\n\r", bufskill, wield->short_descr);
			if (dtype2 != 0)
				snprintf(buf2, MAX_INPUT_LENGTH, "You are %s with %s.\n\r", bufskill2, wield2->short_descr);
		}
	}
	else
	{
		if (dtype == 0 && dtype2 == 0)
			snprintf(buf, MAX_INPUT_LENGTH, "%s is %s at unarmed combat.\n\r", victim->name, bufskill);
		else
		{
			if (dtype != 0)
				snprintf(buf, MAX_INPUT_LENGTH, "%s is %s with %s.\n\r", victim->name, bufskill, wield->short_descr);
			if (dtype2 != 0)
				snprintf(buf2, MAX_INPUT_LENGTH, "%s is %s with %s.\n\r", victim->name, bufskill2, wield2->short_descr);
		}
	}
	if (!(dtype == 0 && dtype2 != 0))
		send_to_char(buf, ch);
	if (dtype2 != 0)
		send_to_char(buf2, ch);
	skillstance(ch, victim);
	return;
}

void skillstance(CHAR_DATA *ch, CHAR_DATA *victim)
{
	char buf[MAX_INPUT_LENGTH];
	char bufskill[25];
	int stance;

	if (IS_NPC(victim))
		return;

	stance = victim->stance[CURRENT_STANCE];
	if (stance < 1 || stance > 10)
		return;
	if (victim->stance[stance] <= 0)
		snprintf(bufskill, 25, "completely unskilled in");
	else if (victim->stance[stance] <= 25)
		snprintf(bufskill, 25, "an apprentice of");
	else if (victim->stance[stance] <= 50)
		snprintf(bufskill, 25, "a trainee of");
	else if (victim->stance[stance] <= 75)
		snprintf(bufskill, 25, "a student of");
	else if (victim->stance[stance] <= 100)
		snprintf(bufskill, 25, "fairly experienced in");
	else if (victim->stance[stance] <= 125)
		snprintf(bufskill, 25, "well trained in");
	else if (victim->stance[stance] <= 150)
		snprintf(bufskill, 25, "highly skilled in");
	else if (victim->stance[stance] <= 175)
		snprintf(bufskill, 25, "an expert of");
	else if (victim->stance[stance] <= 199)
		snprintf(bufskill, 25, "a master of");
	else if (victim->stance[stance] >= 200)
		snprintf(bufskill, 25, "a grand master of");
	else
		return;

	if (ch == victim)
		snprintf(buf, MAX_INPUT_LENGTH, "You are %s the %s stance.", bufskill, stancenames[stance]);
	else
		snprintf(buf, MAX_INPUT_LENGTH, "$N is %s the %s stance.", bufskill, stancenames[stance]);
	act(buf, ch, NULL, victim, TO_CHAR);
	return;
}

void do_spell(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (arg[0] == '\0')
	{
		show_spell(ch, 0);
		show_spell(ch, 1);
		show_spell(ch, 2);
		show_spell(ch, 3);
		show_spell(ch, 4);
	}
	else if (!str_cmp(arg, "purple"))
		show_spell(ch, 0);
	else if (!str_cmp(arg, "red"))
		show_spell(ch, 1);
	else if (!str_cmp(arg, "blue"))
		show_spell(ch, 2);
	else if (!str_cmp(arg, "green"))
		show_spell(ch, 3);
	else if (!str_cmp(arg, "yellow"))
		show_spell(ch, 4);
	else
		send_to_char("You know of no such magic.\n\r", ch);
	return;
}

void show_spell(CHAR_DATA *ch, int dtype)
{
	char buf[MAX_INPUT_LENGTH];
	char bufskill[MAX_INPUT_LENGTH];

	if (IS_NPC(ch))
		return;

	if (ch->spl[dtype] == 00)
		snprintf(bufskill, MAX_INPUT_LENGTH, "untrained");
	else if (ch->spl[dtype] <= 25)
		snprintf(bufskill, MAX_INPUT_LENGTH, "an apprentice");
	else if (ch->spl[dtype] <= 50)
		snprintf(bufskill, MAX_INPUT_LENGTH, "a student");
	else if (ch->spl[dtype] <= 75)
		snprintf(bufskill, MAX_INPUT_LENGTH, "a scholar");
	else if (ch->spl[dtype] <= 100)
		snprintf(bufskill, MAX_INPUT_LENGTH, "a magus");
	else if (ch->spl[dtype] <= 125)
		snprintf(bufskill, MAX_INPUT_LENGTH, "an adept ");
	else if (ch->spl[dtype] <= 150)
		snprintf(bufskill, MAX_INPUT_LENGTH, "a mage");
	else if (ch->spl[dtype] <= 175)
		snprintf(bufskill, MAX_INPUT_LENGTH, "a warlock");
	else if (ch->spl[dtype] <= 199)
		snprintf(bufskill, MAX_INPUT_LENGTH, "a master wizard");
	else if (ch->spl[dtype] >= 200)
		snprintf(bufskill, MAX_INPUT_LENGTH, "a grand sorcerer");
	else
		return;
	if (dtype == 0)
		snprintf(buf, MAX_INPUT_LENGTH, "You are %s at purple magic.\n\r", bufskill);
	else if (dtype == 1)
		snprintf(buf, MAX_INPUT_LENGTH, "You are %s at red magic.\n\r", bufskill);
	else if (dtype == 2)
		snprintf(buf, MAX_INPUT_LENGTH, "You are %s at blue magic.\n\r", bufskill);
	else if (dtype == 3)
		snprintf(buf, MAX_INPUT_LENGTH, "You are %s at green magic.\n\r", bufskill);
	else if (dtype == 4)
		snprintf(buf, MAX_INPUT_LENGTH, "You are %s at yellow magic.\n\r", bufskill);
	else
		return;
	send_to_char(buf, ch);
	return;
}

/* In case we need to remove our pfiles, or wanna turn mortal for a bit */
void do_relevel(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
		return;
	if (!str_cmp(ch->name, "Palmer") || !str_cmp(ch->name, "Palmer") || !str_cmp(ch->name, "Dirge"))
	{
		ch->level = MAX_LEVEL;
		ch->trust = MAX_LEVEL;
		send_to_char("Done.\n\r", ch);
	}
	else
		send_to_char("Huh?\n\r", ch);
	return;
}

/* Added by Palmer */
void do_avatar(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
		return;
	if (!strcmp(ch->name, "Palmer") || !strcmp(ch->name, "Palmer") || !strcmp(ch->name, "Dirge"))
	{
		ch->level = 3;
		ch->trust = 0;
		send_to_char("Done.\n\r", ch);
	}
	else
		send_to_char("Huh?\n\r", ch);

	return;
}
void do_scan(CHAR_DATA *ch, char *argument)
{
	ROOM_INDEX_DATA *location;

	location = get_room_index(ch->in_room->vnum);

	send_to_char("[North]\n\r", ch);
	do_spydirection(ch, "n");
	char_from_room(ch);
	char_to_room(ch, location);

	send_to_char("[East]\n\r", ch);
	do_spydirection(ch, "e");
	char_from_room(ch);
	char_to_room(ch, location);

	send_to_char("[South]\n\r", ch);
	do_spydirection(ch, "s");
	char_from_room(ch);
	char_to_room(ch, location);

	send_to_char("[West]\n\r", ch);
	do_spydirection(ch, "w");
	char_from_room(ch);
	char_to_room(ch, location);

	send_to_char("[Up]\n\r", ch);
	do_spydirection(ch, "u");
	char_from_room(ch);
	char_to_room(ch, location);

	send_to_char("[Down]\n\r", ch);
	do_spydirection(ch, "d");
	char_from_room(ch);
	char_to_room(ch, location);
	return;
}

void do_spy(CHAR_DATA *ch, char *argument)
{
	ROOM_INDEX_DATA *location;
	char arg1[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg1, MAX_INPUT_LENGTH);

	if (arg1[0] == '\0')
	{
		send_to_char("Which direction do you wish to spy?\n\r", ch);
		return;
	}

	if (!(!str_cmp(arg1, "n") || !str_cmp(arg1, "north") ||
		  !str_cmp(arg1, "e") || !str_cmp(arg1, "east") ||
		  !str_cmp(arg1, "s") || !str_cmp(arg1, "south") ||
		  !str_cmp(arg1, "w") || !str_cmp(arg1, "west") ||
		  !str_cmp(arg1, "u") || !str_cmp(arg1, "up") ||
		  !str_cmp(arg1, "d") || !str_cmp(arg1, "down")))
	{
		send_to_char("You can only spy people north, south, east, west, up or down.\n\r", ch);
		return;
	}

	location = get_room_index(ch->in_room->vnum);

	send_to_char("[Short Range]\n\r", ch);
	do_spydirection(ch, arg1);
	send_to_char("\n\r", ch);
	send_to_char("[Medium Range]\n\r", ch);
	do_spydirection(ch, arg1);
	send_to_char("\n\r", ch);
	send_to_char("[Long Range]\n\r", ch);
	do_spydirection(ch, arg1);
	send_to_char("\n\r", ch);

	/* Move them back */
	char_from_room(ch);
	char_to_room(ch, location);
	return;
}

void do_spydirection(CHAR_DATA *ch, char *argument)
{
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	int door;
	bool is_empty;

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	is_empty = TRUE;

	if (arg[0] == '\0')
		return;

	if (!str_cmp(arg, "n") || !str_cmp(arg, "north"))
		door = 0;
	else if (!str_cmp(arg, "e") || !str_cmp(arg, "east"))
		door = 1;
	else if (!str_cmp(arg, "s") || !str_cmp(arg, "south"))
		door = 2;
	else if (!str_cmp(arg, "w") || !str_cmp(arg, "west"))
		door = 3;
	else if (!str_cmp(arg, "u") || !str_cmp(arg, "up"))
		door = 4;
	else if (!str_cmp(arg, "d") || !str_cmp(arg, "down"))
		door = 5;
	else
		return;

	if ((pexit = ch->in_room->exit[door]) == NULL || (to_room = pexit->to_room) == NULL)
	{
		send_to_char("   No exit.\n\r", ch);
		return;
	}
	pexit = ch->in_room->exit[door];
	if (IS_SET(pexit->exit_info, EX_CLOSED))
	{
		send_to_char("   Closed door.\n\r", ch);
		return;
	}
	char_from_room(ch);
	char_to_room(ch, to_room);
	for (vch = char_list; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next;
		if (vch->in_room == NULL)
			continue;
		if (vch == ch)
			continue;
		if (!can_see(ch, vch))
			continue;
		if (vch->in_room == ch->in_room)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "   %s%s\n\r",
					 IS_NPC(vch) ? capitalize(vch->short_descr) : vch->name,
					 IS_NPC(vch) ? "" : " (Player)");
			send_to_char(buf, ch);
			is_empty = FALSE;
			continue;
		}
	}
	if (is_empty)
	{
		send_to_char("   Nobody here.\n\r", ch);
		return;
	}
	return;
}

void throw_to(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, ROOM_INDEX_DATA *location, char *revdoor)
{
	char buf[MAX_INPUT_LENGTH];
	bool has_container;

	has_container = does_ch_have_a_container(victim);
	if (obj->item_type == ITEM_CONTAINER && has_container)
	{
		/* only one container per player */
		snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and drops at your feet.", revdoor);
		act(buf, ch, obj, NULL, TO_ROOM);
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
		char_from_room(ch);
		char_to_room(ch, location);
	}
	else
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and lands in $N's hands.", revdoor);
		act(buf, ch, obj, victim, TO_NOTVICT);
		snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and lands in your hands.", revdoor);
		act(buf, ch, obj, victim, TO_VICT);
		obj_from_char(obj);
		obj_to_char(obj, victim);
		char_from_room(ch);
		char_to_room(ch, location);
	}
}

void do_throw(CHAR_DATA *ch, char *argument)
{
	ROOM_INDEX_DATA *location;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	char revdoor[MAX_INPUT_LENGTH];
	int door;

	argument = one_argument(argument, arg1, MAX_INPUT_LENGTH);
	argument = one_argument(argument, arg2, MAX_INPUT_LENGTH);

	if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL)
	{
		if ((obj = get_eq_char(ch, WEAR_HOLD)) == NULL)
		{
			send_to_char("You are not holding anything to throw.\n\r", ch);
			return;
		}
	}

	if (arg1[0] == '\0')
	{
		send_to_char("Which direction do you wish to throw?\n\r", ch);
		return;
	}

	if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north"))
	{
		door = 0;
		snprintf(arg1, MAX_INPUT_LENGTH, "north");
	}
	else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east"))
	{
		door = 1;
		snprintf(arg1, MAX_INPUT_LENGTH, "east");
	}
	else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south"))
	{
		door = 2;
		snprintf(arg1, MAX_INPUT_LENGTH, "south");
	}
	else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west"))
	{
		door = 3;
		snprintf(arg1, MAX_INPUT_LENGTH, "west");
	}
	else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up"))
	{
		door = 4;
		snprintf(arg1, MAX_INPUT_LENGTH, "up");
	}
	else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down"))
	{
		door = 5;
		snprintf(arg1, MAX_INPUT_LENGTH, "down");
	}
	else
	{
		send_to_char("You can only throw north, south, east, west, up or down.\n\r", ch);
		return;
	}

	if (!str_cmp(arg1, "north"))
	{
		door = 0;
		snprintf(revdoor, MAX_INPUT_LENGTH, "south");
	}
	else if (!str_cmp(arg1, "east"))
	{
		door = 1;
		snprintf(revdoor, MAX_INPUT_LENGTH, "west");
	}
	else if (!str_cmp(arg1, "south"))
	{
		door = 2;
		snprintf(revdoor, MAX_INPUT_LENGTH, "north");
	}
	else if (!str_cmp(arg1, "west"))
	{
		door = 3;
		snprintf(revdoor, MAX_INPUT_LENGTH, "east");
	}
	else if (!str_cmp(arg1, "up"))
	{
		door = 4;
		snprintf(revdoor, MAX_INPUT_LENGTH, "down");
	}
	else if (!str_cmp(arg1, "down"))
	{
		door = 5;
		snprintf(revdoor, MAX_INPUT_LENGTH, "up");
	}
	else
		return;

	location = get_room_index(ch->in_room->vnum);

	snprintf(buf, MAX_INPUT_LENGTH, "You hurl $p %s.", arg1);
	act(buf, ch, obj, NULL, TO_CHAR);
	snprintf(buf, MAX_INPUT_LENGTH, "$n hurls $p %s.", arg1);
	act(buf, ch, obj, NULL, TO_ROOM);
	/* First room */
	if ((pexit = ch->in_room->exit[door]) == NULL || (to_room = pexit->to_room) == NULL)
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$p bounces off the %s wall.", arg1);
		act(buf, ch, obj, NULL, TO_ROOM);
		act(buf, ch, obj, NULL, TO_CHAR);
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
		return;
	}
	pexit = ch->in_room->exit[door];
	if (IS_SET(pexit->exit_info, EX_CLOSED))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$p bounces off the %s door.", arg1);
		act(buf, ch, obj, NULL, TO_ROOM);
		act(buf, ch, obj, NULL, TO_CHAR);
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
		return;
	}
	char_from_room(ch);
	char_to_room(ch, to_room);
	if ((victim = get_char_room(ch, arg2)) != NULL)
	{
		throw_to(ch, victim, obj, location, revdoor);
		return;
	}

	/* Second room */
	if ((pexit = ch->in_room->exit[door]) == NULL || (to_room = pexit->to_room) == NULL)
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and strikes %s wall.", revdoor, arg1);
		act(buf, ch, obj, NULL, TO_ROOM);
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
		char_from_room(ch);
		char_to_room(ch, location);
		return;
	}
	pexit = ch->in_room->exit[door];
	if (IS_SET(pexit->exit_info, EX_CLOSED))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and strikes the %s door.", revdoor, arg1);
		act(buf, ch, obj, NULL, TO_ROOM);
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
		char_from_room(ch);
		char_to_room(ch, location);
		return;
	}
	snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and carries on %s.", revdoor, arg1);
	act(buf, ch, obj, NULL, TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, to_room);
	if ((victim = get_char_room(ch, arg2)) != NULL)
	{
		throw_to(ch, victim, obj, location, revdoor);
		return;
	}

	/* Third room */
	if ((pexit = ch->in_room->exit[door]) == NULL || (to_room = pexit->to_room) == NULL)
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and strikes %s wall.", revdoor, arg1);
		act(buf, ch, obj, NULL, TO_ROOM);
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
		char_from_room(ch);
		char_to_room(ch, location);
		return;
	}
	pexit = ch->in_room->exit[door];
	if (IS_SET(pexit->exit_info, EX_CLOSED))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and strikes the %s door.", revdoor, arg1);
		act(buf, ch, obj, NULL, TO_ROOM);
		obj_from_char(obj);
		obj_to_room(obj, ch->in_room);
		char_from_room(ch);
		char_to_room(ch, location);
		return;
	}
	snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and carries on %s.", revdoor, arg1);
	act(buf, ch, obj, NULL, TO_ROOM);
	char_from_room(ch);
	char_to_room(ch, to_room);
	if ((victim = get_char_room(ch, arg2)) != NULL)
	{
		throw_to(ch, victim, obj, location, revdoor);
		return;
	}

	snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and drops at your feet.", revdoor);
	act(buf, ch, obj, NULL, TO_ROOM);
	obj_from_char(obj);
	obj_to_room(obj, ch->in_room);

	/* Move them back */
	char_from_room(ch);
	char_to_room(ch, location);
	return;
}

void do_shoot(CHAR_DATA *ch, char *argument)
{
	ROOM_INDEX_DATA *location;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	OBJ_DATA *ammo;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	char revdoor[MAX_INPUT_LENGTH];
	int door;
	bool missed;
	int dam;

	argument = one_argument(argument, arg1, MAX_INPUT_LENGTH);
	argument = one_argument(argument, arg2, MAX_INPUT_LENGTH);

	if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL || obj->item_type != ITEM_MISSILE)
	{
		if ((obj = get_eq_char(ch, WEAR_HOLD)) == NULL || obj->item_type != ITEM_MISSILE)
		{
			send_to_char("You are not holding anything to shoot.\n\r", ch);
			return;
		}
	}

	if (obj->value[1] < 1)
	{
		send_to_char("First you need to load it.\n\r", ch);
		return;
	}

	if (arg1[0] == '\0')
	{
		send_to_char("Please specify a direction to shoot.\n\r", ch);
		return;
	}

	if (arg2[0] == '\0')
	{
		send_to_char("Please specify a target to shoot at.\n\r", ch);
		return;
	}

	if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north"))
	{
		door = 0;
		snprintf(arg1, MAX_INPUT_LENGTH, "north");
	}
	else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east"))
	{
		door = 1;
		snprintf(arg1, MAX_INPUT_LENGTH, "east");
	}
	else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south"))
	{
		door = 2;
		snprintf(arg1, MAX_INPUT_LENGTH, "south");
	}
	else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west"))
	{
		door = 3;
		snprintf(arg1, MAX_INPUT_LENGTH, "west");
	}
	else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up"))
	{
		door = 4;
		snprintf(arg1, MAX_INPUT_LENGTH, "up");
	}
	else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down"))
	{
		door = 5;
		snprintf(arg1, MAX_INPUT_LENGTH, "down");
	}
	else
	{
		send_to_char("You can only shoot north, south, east, west, up or down.\n\r", ch);
		return;
	}

	if (!str_cmp(arg1, "north"))
	{
		door = 0;
		snprintf(revdoor, MAX_INPUT_LENGTH, "south");
	}
	else if (!str_cmp(arg1, "east"))
	{
		door = 1;
		snprintf(revdoor, MAX_INPUT_LENGTH, "west");
	}
	else if (!str_cmp(arg1, "south"))
	{
		door = 2;
		snprintf(revdoor, MAX_INPUT_LENGTH, "north");
	}
	else if (!str_cmp(arg1, "west"))
	{
		door = 3;
		snprintf(revdoor, MAX_INPUT_LENGTH, "east");
	}
	else if (!str_cmp(arg1, "up"))
	{
		door = 4;
		snprintf(revdoor, MAX_INPUT_LENGTH, "down");
	}
	else if (!str_cmp(arg1, "down"))
	{
		door = 5;
		snprintf(revdoor, MAX_INPUT_LENGTH, "up");
	}
	else
		return;

	if (get_obj_index(obj->value[0]) == NULL)
		return;

	location = get_room_index(ch->in_room->vnum);
	ammo = create_object(get_obj_index(obj->value[0]), 0);

	if (number_percent() <= ammo->value[0])
		missed = FALSE;
	else
		missed = TRUE;
	obj->value[1] -= 1;

	snprintf(buf, MAX_INPUT_LENGTH, "You point $p %s and shoot.", arg1);
	act(buf, ch, obj, NULL, TO_CHAR);
	snprintf(buf, MAX_INPUT_LENGTH, "$n points $p %s and shoots.", arg1);
	act(buf, ch, obj, NULL, TO_ROOM);
	/* First room */
	if ((pexit = ch->in_room->exit[door]) == NULL || (to_room = pexit->to_room) == NULL)
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$p bounces off the %s wall.", arg1);
		act(buf, ch, ammo, NULL, TO_ROOM);
		act(buf, ch, ammo, NULL, TO_CHAR);
		obj_to_room(ammo, ch->in_room);
		return;
	}
	pexit = ch->in_room->exit[door];
	if (IS_SET(pexit->exit_info, EX_CLOSED))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$p bounces off the %s door.", arg1);
		act(buf, ch, ammo, NULL, TO_ROOM);
		act(buf, ch, ammo, NULL, TO_CHAR);
		obj_to_room(ammo, ch->in_room);
		return;
	}
	char_from_room(ch);
	char_to_room(ch, to_room);
	if ((victim = get_char_room(ch, arg2)) != NULL)
	{
		if (missed)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "$p flies in from the %s, just missing $N, and carrying on %s.", revdoor, arg1);
			act(buf, ch, ammo, victim, TO_NOTVICT);
			snprintf(buf, MAX_INPUT_LENGTH, "$p flies in from the %s, just missing you, and carrying on %s.", revdoor, arg1);
			act(buf, ch, ammo, victim, TO_VICT);
		}
		else
		{
			snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and strikes $N.", revdoor);
			act(buf, ch, ammo, victim, TO_NOTVICT);
			snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and strikes you.", revdoor);
			act(buf, ch, ammo, victim, TO_VICT);
			obj_to_char(ammo, victim);
			char_from_room(ch);
			char_to_room(ch, location);
			dam = number_range(ammo->value[1], ammo->value[2]);
			if (dam < 0)
				dam = -dam;
			if (IS_SET(victim->act, PLR_VAMPIRE) && ammo->pIndexData->vnum == 20001)
			{
				victim->hit -= dam;
			}
			else if (ammo->pIndexData->vnum != 20001)
			{
				victim->hit -= dam;
			}
			update_pos(victim);
			if (victim->position == POS_DEAD && !IS_HERO(victim))
			{
				send_to_char("You have been KILLED!!\n\r\n\r", victim);
				raw_kill(victim);
			}
			return;
		}
	}

	/* Second room */
	if ((pexit = ch->in_room->exit[door]) == NULL || (to_room = pexit->to_room) == NULL)
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and strikes %s wall.", revdoor, arg1);
		act(buf, ch, ammo, NULL, TO_ROOM);
		obj_to_room(ammo, ch->in_room);
		char_from_room(ch);
		char_to_room(ch, location);
		return;
	}
	pexit = ch->in_room->exit[door];
	if (IS_SET(pexit->exit_info, EX_CLOSED))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and strikes the %s door.", revdoor, arg1);
		act(buf, ch, ammo, NULL, TO_ROOM);
		obj_to_room(ammo, ch->in_room);
		char_from_room(ch);
		char_to_room(ch, location);
		return;
	}
	if (!missed)
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and carries on %s.", revdoor, arg1);
		act(buf, ch, ammo, NULL, TO_ROOM);
	}
	char_from_room(ch);
	char_to_room(ch, to_room);
	if ((victim = get_char_room(ch, arg2)) != NULL)
	{
		if (missed)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "$p flies in from the %s, just missing $N, and carrying on %s.", revdoor, arg1);
			act(buf, ch, ammo, victim, TO_NOTVICT);
			snprintf(buf, MAX_INPUT_LENGTH, "$p flies in from the %s, just missing you, and carrying on %s.", revdoor, arg1);
			act(buf, ch, ammo, victim, TO_VICT);
		}
		else
		{
			snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and strikes $N.", revdoor);
			act(buf, ch, ammo, victim, TO_NOTVICT);
			snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and strikes you.", revdoor);
			act(buf, ch, ammo, victim, TO_VICT);
			obj_to_char(ammo, victim);
			char_from_room(ch);
			char_to_room(ch, location);
			dam = number_range(ammo->value[1], ammo->value[2]);
			if (dam < 0)
				dam = -dam;
			if (IS_SET(victim->act, PLR_VAMPIRE) && ammo->pIndexData->vnum == 20001)
			{
				victim->hit -= dam;
			}
			else if (ammo->pIndexData->vnum != 20001)
			{
				victim->hit -= dam;
			}
			update_pos(victim);
			if (victim->position == POS_DEAD && !IS_HERO(victim))
			{
				send_to_char("You have been KILLED!!\n\r\n\r", victim);
				raw_kill(victim);
			}
			return;
		}
	}

	/* Third room */
	if ((pexit = ch->in_room->exit[door]) == NULL || (to_room = pexit->to_room) == NULL)
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and strikes %s wall.", revdoor, arg1);
		act(buf, ch, ammo, NULL, TO_ROOM);
		obj_to_room(ammo, ch->in_room);
		char_from_room(ch);
		char_to_room(ch, location);
		return;
	}
	pexit = ch->in_room->exit[door];
	if (IS_SET(pexit->exit_info, EX_CLOSED))
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and strikes the %s door.", revdoor, arg1);
		act(buf, ch, ammo, NULL, TO_ROOM);
		obj_to_room(ammo, ch->in_room);
		char_from_room(ch);
		char_to_room(ch, location);
		return;
	}
	if (!missed)
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and carries on %s.", revdoor, arg1);
		act(buf, ch, ammo, NULL, TO_ROOM);
	}
	char_from_room(ch);
	char_to_room(ch, to_room);
	if ((victim = get_char_room(ch, arg2)) != NULL)
	{
		if (missed)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "$p flies in from the %s, just missing $N, and carrying on %s.", revdoor, arg1);
			act(buf, ch, ammo, victim, TO_NOTVICT);
			snprintf(buf, MAX_INPUT_LENGTH, "$p flies in from the %s, just missing you, and carrying on %s.", revdoor, arg1);
			act(buf, ch, ammo, victim, TO_VICT);
		}
		else
		{
			snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and strikes $N.", revdoor);
			act(buf, ch, ammo, victim, TO_NOTVICT);
			snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and strikes you.", revdoor);
			act(buf, ch, ammo, victim, TO_VICT);
			obj_to_char(ammo, victim);
			char_from_room(ch);
			char_to_room(ch, location);
			dam = number_range(ammo->value[1], ammo->value[2]);
			if (dam < 0)
				dam = -dam;
			if (IS_SET(victim->act, PLR_VAMPIRE) && ammo->pIndexData->vnum == 20001)
			{
				victim->hit -= dam;
			}
			else if (ammo->pIndexData->vnum != 20001)
			{
				victim->hit -= dam;
			}
			update_pos(victim);
			if (victim->position == POS_DEAD && !IS_HERO(victim))
			{
				send_to_char("You have been KILLED!!\n\r\n\r", victim);
				raw_kill(victim);
			}
			return;
		}
	}

	snprintf(buf, MAX_INPUT_LENGTH, "$p comes flying in from the %s and drops at your feet.", revdoor);
	act(buf, ch, ammo, NULL, TO_ROOM);
	obj_to_room(ammo, ch->in_room);

	/* Move them back */
	char_from_room(ch);
	char_to_room(ch, location);
	return;
}

void do_reload(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj2;
	OBJ_DATA *ammo;
	OBJ_DATA *missile;

	obj = get_eq_char(ch, WEAR_WIELD);
	obj2 = get_eq_char(ch, WEAR_HOLD);

	if (obj == NULL || obj2 == NULL)
	{
		send_to_char("You must hold the weapon to reload in one hand and the ammo in the other.\n\r", ch);
		return;
	}

	if (!(obj->item_type == ITEM_MISSILE && obj2->item_type == ITEM_AMMO) && !(obj2->item_type == ITEM_MISSILE && obj->item_type == ITEM_AMMO))
	{
		send_to_char("You must hold the weapon to reload in one hand and the ammo in the other.\n\r", ch);
		return;
	}

	if (obj->item_type == ITEM_MISSILE)
	{
		missile = obj;
		ammo = obj2;
	}
	else
	{
		missile = obj2;
		ammo = obj;
	}

	if (missile->value[1] >= missile->value[2])
	{
		send_to_char("It is already fully loaded.\n\r", ch);
		return;
	}

	if (missile->value[3] != ammo->value[3])
	{
		send_to_char("That weapon doesn't take that sort of ammo.\n\r", ch);
		return;
	}

	if (missile->value[0] != 0 && missile->value[1] != 0 &&
		missile->value[0] != ammo->pIndexData->vnum)
	{
		send_to_char("You cannot mix ammo types.\n\r", ch);
		return;
	}

	act("You load $p into $P.", ch, ammo, missile, TO_CHAR);
	act("$n loads $p into $P.", ch, ammo, missile, TO_ROOM);
	missile->value[1] += 1;
	missile->value[0] = ammo->pIndexData->vnum;
	obj_from_char(ammo);
	extract_obj(ammo);
	return;
}

void do_unload(CHAR_DATA *ch, char *argument)
{
	OBJ_DATA *missile;
	OBJ_DATA *ammo;

	if ((missile = get_eq_char(ch, WEAR_WIELD)) == NULL || missile->item_type != ITEM_MISSILE)
	{
		if ((missile = get_eq_char(ch, WEAR_HOLD)) == NULL || missile->item_type != ITEM_MISSILE)
		{
			send_to_char("You are not holding anything to unload.\n\r", ch);
			return;
		}
	}

	if (missile->value[1] <= 0)
	{
		send_to_char("But it is already empty!\n\r", ch);
		return;
	}

	if (get_obj_index(missile->value[0]) == NULL)
		return;

	ammo = create_object(get_obj_index(missile->value[0]), 0);
	act("You remove $p from $P.", ch, ammo, missile, TO_CHAR);
	act("$n removes $p from $P.", ch, ammo, missile, TO_ROOM);
	missile->value[1] -= 1;
	obj_to_char(ammo, ch);
	return;
}

void do_autostance(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char autostancename[MAX_INPUT_LENGTH];
	int current_auto_stance;
	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (IS_NPC(ch))
		return;

	current_auto_stance = ch->stance[AUTODROP];
	if (!str_cmp(arg, ""))
	{
		snprintf(autostancename, MAX_INPUT_LENGTH, "Current Autostance: %s\n\r", stancenames[current_auto_stance]);
		send_to_char(autostancename, ch);
		return;
	}

	if (!str_cmp(arg, "none"))
	{
		send_to_char("Autostance set to None.\n\r", ch);
		ch->stance[AUTODROP] = STANCE_NONE;
		return;
	}

	// Not a fan of the "11", should be a "MAX_STANCE" or something
	for (int i = 1; i < 11; i++)
	{

		if (!str_cmp(arg, stancenames[i]))
		{
			if (!canStance(ch, i))
			{
				send_to_char("No such Stance.\n\r", ch);
				return;
			}

			snprintf(autostancename, MAX_INPUT_LENGTH, "Autostance set to %s.\n\r", stancenames[i]);
			send_to_char(autostancename, ch);
			ch->stance[AUTODROP] = i;
			return;
		}
	}
}

void autodrop(CHAR_DATA *ch)
{
	char buf[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH];
	int auto_drop_stance;

	if (IS_NPC(ch))
		return;

	if (ch->stance[AUTODROP] == STANCE_NORMAL)
		return;

	if (ch->stance[CURRENT_STANCE] < 1)
	{
		ch->stance[CURRENT_STANCE] = ch->stance[AUTODROP];

		auto_drop_stance = ch->stance[AUTODROP];
		sprintf(buf, "You fall into the %s stance.", stancenames[auto_drop_stance]);
		act(buf, ch, NULL, NULL, TO_CHAR);
		sprintf(buf2, "$n falls into the %s stance.", stancenames[auto_drop_stance]);
		act(buf2, ch, NULL, NULL, TO_ROOM);
	}
}

void do_stance(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, MAX_INPUT_LENGTH);

	char *const prettyStanceTextForMe[11] = {
		"wooo i'm dumb",
		"You arch your body into the viper fighting stance.\n\r",
		"You swing your body into the crane fighting stance.\n\r",
		"You flow into the falcon fighting stance.\n\r",
		"You twist into the mongoose fighting stance.\n\r",
		"You hunch down into the bull fighting stance.\n\r",
		"You spread yourself into the swallow fighting stance.\n\r",
		"You twist into the cobra fighting stance.\n\r",
		"You fall into the lion fighting stance.\n\r",
		"You swell into the grizzlie fighting stance.\n\r",
		"You leap into the panther fighting stance.\n\r"};

	// These astance messages are fucking dumb, I'll update them later
	char *const prettyStanceTextForYou[11] = {
		"wooo $n is dumb",
		"$n arches $s body into the viper fighting stance.",
		"$n swings $s body into the crane fighting stance.",
		"$n gets into an advanced stance, the likes of which you have never seen!",
		"$n twists into the mongoose fighting stance.",
		"$n hunches down into the bull fighting stance.",
		"$n gets into an advanced stance, the likes of which you have never seen!",
		"$n gets into an advanced stance, the likes of which you have never seen!",
		"$n gets into an advanced stance, the likes of which you have never seen!",
		"$n gets into an advanced stance, the likes of which you have never seen!",
		"$n gets into an advanced stance, the likes of which you have never seen!"};

	if (IS_NPC(ch))
		return;

	if (arg[0] == '\0')
	{
		if (ch->stance[CURRENT_STANCE] == -1)
		{
			ch->stance[CURRENT_STANCE] = 0;
			send_to_char("You drop into a fighting stance.\n\r", ch);
			act("$n drops into a fighting stance.", ch, NULL, NULL, TO_ROOM);
		}
		else
		{
			ch->stance[CURRENT_STANCE] = -1;
			send_to_char("You relax from your fighting stance.\n\r", ch);
			act("$n relaxes from $s fighting stance.", ch, NULL, NULL, TO_ROOM);
		}
		return;
	}

	if (ch->stance[CURRENT_STANCE] > 0)
	{
		send_to_char("You cannot change stances until you come up from the one you are currently in.\n\r", ch);
		return;
	}

	if (!str_cmp(arg, "none"))
	{
		ch->stance[CURRENT_STANCE] = STANCE_NONE;
		send_to_char("You drop into a general fighting stance.\n\r", ch);
		act("$n drops into a general fighting stance.", ch, NULL, NULL, TO_ROOM);
	}

	// Not a fan of the "11", should be a "MAX_STANCE" or something
	for (int i = 1; i < 11; i++)
	{
		if (!str_cmp(arg, stancenames[i]))
		{
			if (!canStance(ch, i))
			{
				send_to_char("Syntax is: stance <style>.\n\r", ch);
				send_to_char("Stance being one of: None, Viper, Crane, Mongoose, Bull.\n\rAdvanced stances being: Swallow, Cobra, Lion, Grizzlie, Panther.\n\r", ch);
				return;
			}

			send_to_char(prettyStanceTextForMe[i], ch);
			act(prettyStanceTextForYou[i], ch, NULL, NULL, TO_ROOM);
			ch->stance[CURRENT_STANCE] = i;
			WAIT_STATE(ch, 12);
			return;
		}
	}
	send_to_char("Syntax is: stance <style>.\n\r", ch);
	send_to_char("Stance being one of: None, Viper, Crane, Mongoose, Bull.\n\rAdvanced stances being: Swallow, Cobra, Lion, Grizzlie, Panther.\n\r", ch);
	return;
}

void do_fightstyle(CHAR_DATA *ch, char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	int selection;
	int value;

	argument = one_argument(argument, arg1, MAX_INPUT_LENGTH);
	argument = one_argument(argument, arg2, MAX_INPUT_LENGTH);

	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char_formatted("Syntax is: fightstyle <number> <style>.\n\r", ch);
		send_to_char_formatted("Style can be selected from the following (enter style in text form):\n\r", ch);
		send_to_char_formatted("[ 1]*Trip      [ 2]*Kick      [ 3] Bash      [ 4] Elbow     [ 5] Knee\n\r", ch);
		send_to_char_formatted("[ 6] Headbutt  [ 7]*Disarm    [ 8] Bite      [ 9]*Dirt      [10] Grapple\n\r", ch);
		send_to_char_formatted("[11] Punch     [12]*Gouge     [13] Rip       [14]*Stamp     [15] Backfist\n\r", ch);
		send_to_char_formatted("[16] Jumpkick  [17] Spinkick  [18] Hurl      [19] Sweep     [20] Charge\n\r", ch);
		snprintf(buf, MAX_INPUT_LENGTH, "Selected options: 1:[%d] 2:[%d] 3:[%d] 4:[%d] 5:[%d] 6:[%d] 7:[%d] 8:[%d].\n\r", ch->cmbt[0], ch->cmbt[1],
				 ch->cmbt[2], ch->cmbt[3], ch->cmbt[4], ch->cmbt[5], ch->cmbt[6], ch->cmbt[7]);
		send_to_char_formatted(buf, ch);
		send_to_char_formatted("\n\r* This has been coded (others are not yet in).\n\r", ch);
		send_to_char_formatted("Use 'clear' to clear a fightstyle to 0. eg fightstyle 1 clear \n\r", ch);
		return;
	}
	value = is_number(arg1) ? atoi(arg1) : -1;
	if (value < 1 || value > 8)
	{
		send_to_char("Please enter a value between 1 and 8.\n\r", ch);
		return;
	}
	if (!str_cmp(arg2, "clear"))
		selection = 0;
	else if (!str_cmp(arg2, "trip"))
		selection = 1;
	else if (!str_cmp(arg2, "kick"))
		selection = 2;
	else if (!str_cmp(arg2, "bash"))
		selection = 3;
	else if (!str_cmp(arg2, "elbow"))
		selection = 4;
	else if (!str_cmp(arg2, "knee"))
		selection = 5;
	else if (!str_cmp(arg2, "headbutt"))
		selection = 6;
	else if (!str_cmp(arg2, "disarm"))
		selection = 7;
	else if (!str_cmp(arg2, "bite"))
		selection = 8;
	else if (!str_cmp(arg2, "dirt"))
		selection = 9;
	else if (!str_cmp(arg2, "grapple"))
		selection = 10;
	else if (!str_cmp(arg2, "punch"))
		selection = 11;
	else if (!str_cmp(arg2, "gouge"))
		selection = 12;
	else if (!str_cmp(arg2, "rip"))
		selection = 13;
	else if (!str_cmp(arg2, "stamp"))
		selection = 14;
	else if (!str_cmp(arg2, "backfist"))
		selection = 15;
	else if (!str_cmp(arg2, "jumpkick"))
		selection = 16;
	else if (!str_cmp(arg2, "spinkick"))
		selection = 17;
	else if (!str_cmp(arg2, "hurl"))
		selection = 18;
	else if (!str_cmp(arg2, "sweep"))
		selection = 19;
	else if (!str_cmp(arg2, "charge"))
		selection = 20;
	else
	{
		send_to_char_formatted("Syntax is: fightstyle <number> <style>.\n\r", ch);
		send_to_char_formatted("Style can be selected from the following (enter style in text form):\n\r", ch);
		send_to_char_formatted("[ 1]*Trip      [ 2]*Kick      [ 3] Bash      [ 4] Elbow     [ 5] Knee\n\r", ch);
		send_to_char_formatted("[ 6] Headbutt  [ 7]*Disarm    [ 8] Bite      [ 9]*Dirt      [10] Grapple\n\r", ch);
		send_to_char_formatted("[11] Punch     [12]*Gouge     [13] Rip       [14]*Stamp     [15] Backfist\n\r", ch);
		send_to_char_formatted("[16] Jumpkick  [17] Spinkick  [18] Hurl      [19] Sweep     [20] Charge\n\r", ch);
		snprintf(buf, MAX_INPUT_LENGTH, "Selected options: 1:[%d] 2:[%d] 3:[%d] 4:[%d] 5:[%d] 6:[%d] 7:[%d] 8:[%d].\n\r", ch->cmbt[0], ch->cmbt[1],
				 ch->cmbt[2], ch->cmbt[3], ch->cmbt[4], ch->cmbt[5], ch->cmbt[6], ch->cmbt[7]);
		send_to_char_formatted(buf, ch);
		send_to_char_formatted("\n\r* This has been coded (others are not yet in).\n\r", ch);
		send_to_char_formatted("Use 'clear' to clear a fightstyle to 0. eg fightstyle 1 clear \n\r", ch);
		return;
	}
	ch->cmbt[(value - 1)] = selection;
	snprintf(buf, MAX_INPUT_LENGTH, "Combat option %d now set to %s (%d).\n\r", value, arg2, ch->cmbt[0]);
	send_to_char(buf, ch);
	return;
}

void fightaction(CHAR_DATA *ch, CHAR_DATA *victim, int actype, int dtype, int wpntype)
{
	AFFECT_DATA af;
	char buf[MAX_INPUT_LENGTH];
	char buf1[MAX_INPUT_LENGTH];
	/*
    char            buf      [MAX_INPUT_LENGTH]; */

	if (IS_NPC(ch))
		return;
	/*  if ( IS_AFFECTED(ch,AFF_POLYMORPH) ) return; *** Just in case...KaVir */
	if (actype < 1 || actype > 20)
		return;
	if (victim == NULL)
		return;

	/* Trip */
	if (actype == 1 && (victim->position == POS_STANDING || victim->position == POS_FIGHTING))
	{
		if (number_percent() <= ch->wpn[0])
			trip(ch, victim);
		one_hit(ch, victim, dtype, wpntype);
		return;
	}
	else if (actype == 2 && number_percent() < 75)
	{
		do_kick(ch, "");
		return;
	}
	else if (actype == 3 && number_percent() < 75)
	{
		if (number_percent() <= ch->wpn[0])
		{
			bash(ch, victim);
			one_hit(ch, victim, dtype, wpntype);
		}

		return;
	}
	else if (actype == 7)
	{
		if (number_percent() < 25)
			disarm(ch, victim);
		return;
	}
	else if (actype == 9)
	{
		snprintf(buf, MAX_INPUT_LENGTH, "You kick a spray of dirt into $N's face.");
		ADD_COLOUR(ch, buf, CYAN, MAX_INPUT_LENGTH);
		act(buf, ch, NULL, victim, TO_CHAR);
		act("$n kicks a spray of dirt into your face.", ch, NULL, victim, TO_VICT);
		act("$n kicks a spray of dirt into $N's face.", ch, NULL, victim, TO_NOTVICT);
		if (IS_AFFECTED(victim, AFF_BLIND) || number_percent() < 25)
			return;

		if (IS_NPC(victim) && IS_SET(victim->act, ACT_IMMBLIND))
			return;

		af.type = 4;
		af.location = APPLY_HITROLL;
		af.modifier = -4;
		af.duration = 1;
		af.bitvector = AFF_BLIND;
		affect_to_char(victim, &af);
		snprintf(buf1, MAX_INPUT_LENGTH, "$N is blinded!");
		ADD_COLOUR(ch, buf1, WHITE, MAX_INPUT_LENGTH);
		act(buf1, ch, NULL, victim, TO_CHAR);
		act("$N is blinded!", ch, NULL, victim, TO_NOTVICT);
		send_to_char("You are blinded!\n\r", victim);
		return;
	}
	else if (actype == 12)
	{
		snprintf(buf, MAX_INPUT_LENGTH, "You gouge your fingers into $N's eyes.");
		ADD_COLOUR(ch, buf, CYAN, MAX_INPUT_LENGTH);
		act(buf, ch, NULL, victim, TO_CHAR);
		act("$n gouges $s fingers into your eyes.", ch, NULL, victim, TO_VICT);
		act("$n gouges $s fingers into $N's eyes.", ch, NULL, victim, TO_NOTVICT);
		if (IS_AFFECTED(victim, AFF_BLIND) || number_percent() < 75)
		{
			one_hit(ch, victim, dtype, wpntype);
			return;
		}

		if (IS_NPC(victim) && IS_SET(victim->act, ACT_IMMBLIND))
		{
			one_hit(ch, victim, dtype, wpntype);
			return;
		}

		af.type = 4;
		af.location = APPLY_HITROLL;
		af.modifier = -4;
		af.duration = 1;
		af.bitvector = AFF_BLIND;
		affect_to_char(victim, &af);
		snprintf(buf1, MAX_INPUT_LENGTH, "$N is blinded!");
		ADD_COLOUR(ch, buf1, WHITE, MAX_INPUT_LENGTH);
		act(buf1, ch, NULL, victim, TO_CHAR);
		act("$N is blinded!", ch, NULL, victim, TO_NOTVICT);
		send_to_char("You are blinded!\n\r", victim);
		return;
	}
	else if (actype == 14)
	{
		if (victim->move < 1)
			return;
		snprintf(buf, MAX_INPUT_LENGTH, "You leap in the air and stamp on $N's feet.");
		ADD_COLOUR(ch, buf, CYAN, MAX_INPUT_LENGTH);
		act(buf, ch, NULL, victim, TO_CHAR);
		act("$n leaps in the air and stamps on your feet.", ch, NULL, victim, TO_VICT);
		act("$n leaps in the air and stamps on $N's feet.", ch, NULL, victim, TO_NOTVICT);
		victim->move -= number_range(25, 50);
		if (victim->move < 0)
			victim->move = 0;
		return;
	}
	return;
}

void critical_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt, int dam)
{
	OBJ_DATA *obj;
	OBJ_DATA *damaged;
	char buf[MAX_INPUT_LENGTH];
	char buf2[MAX_INPUT_LENGTH]; // This was set to 20, I could either fix it here or make the rest 20
	int dtype;
	int critical = 0;
	int wpn = 0;
	int wpn2 = 0;
	int count;
	int count2;

	dtype = dt - 1000;
	if (dtype < 0 || dtype > 12)
		return;
	if (IS_NPC(ch))
		critical += ((ch->level + 1) / 5);
	if (!IS_NPC(ch))
		critical += ((ch->wpn[dtype] + 1) / 10);
	if (IS_NPC(victim))
		critical -= ((victim->level + 1) / 5);
	if (!IS_NPC(victim))
	{
		obj = get_eq_char(victim, WEAR_WIELD);
		dtype = TYPE_HIT;
		if (obj != NULL && IS_WEAPON(obj))
			dtype += obj->value[3];
		wpn = dtype - 1000;
		if (wpn < 0 || wpn > 12)
			wpn = 0;
		obj = get_eq_char(victim, WEAR_HOLD);
		dtype = TYPE_HIT;
		if (obj != NULL && IS_WEAPON(obj))
			dtype += obj->value[3];
		wpn2 = dtype - 1000;
		if (wpn2 < 0 || wpn2 > 12)
			wpn2 = 0;

		if (victim->wpn[wpn] > victim->wpn[wpn2])
			critical -= ((victim->wpn[wpn] + 1) / 10);
		else
			critical -= ((victim->wpn[wpn2] + 1) / 10);
	}
	if (critical < 1)
		critical = 1;
	else if (critical > 20)
		critical = 20;
	if (number_percent() > critical)
		return;
	critical = number_range(1, 23);
	if (critical == 1)
	{
		if (IS_HEAD(victim, LOST_EYE_L) && IS_HEAD(victim, LOST_EYE_R))
			return;
		if ((damaged = get_eq_char(victim, WEAR_FACE)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevents you from loosing an eye.", victim, damaged, NULL, TO_CHAR);
			act("$p prevents $n from loosing an eye.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p falls broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p falls broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		if (!IS_HEAD(victim, LOST_EYE_L) && number_percent() < 50)
			SET_BIT(victim->loc_hp[0], LOST_EYE_L);
		else if (!IS_HEAD(victim, LOST_EYE_R))
			SET_BIT(victim->loc_hp[0], LOST_EYE_R);
		else if (!IS_HEAD(victim, LOST_EYE_L))
			SET_BIT(victim->loc_hp[0], LOST_EYE_L);
		else
			return;
		act("Your skillful blow takes out $N's eye!", ch, NULL, victim, TO_CHAR);
		act("$n's skillful blow takes out $N's eye!", ch, NULL, victim, TO_NOTVICT);
		act("$n's skillful blow takes out your eye!", ch, NULL, victim, TO_VICT);
		make_part(victim, "eyeball");
		return;
	}
	else if (critical == 2)
	{
		if (IS_HEAD(victim, LOST_EAR_L) && IS_HEAD(victim, LOST_EAR_R))
			return;
		if ((damaged = get_eq_char(victim, WEAR_HEAD)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevents you from loosing an ear.", victim, damaged, NULL, TO_CHAR);
			act("$p prevents $n from loosing an ear.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p falls broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p falls broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		if (!IS_HEAD(victim, LOST_EAR_L) && number_percent() < 50)
			SET_BIT(victim->loc_hp[0], LOST_EAR_L);
		else if (!IS_HEAD(victim, LOST_EAR_R))
			SET_BIT(victim->loc_hp[0], LOST_EAR_R);
		else if (!IS_HEAD(victim, LOST_EAR_L))
			SET_BIT(victim->loc_hp[0], LOST_EAR_L);
		else
			return;
		act("Your skillful blow cuts off $N's ear!", ch, NULL, victim, TO_CHAR);
		act("$n's skillful blow cuts off $N's ear!", ch, NULL, victim, TO_NOTVICT);
		act("$n's skillful blow cuts off your ear!", ch, NULL, victim, TO_VICT);
		make_part(victim, "ear");
		return;
	}
	else if (critical == 3)
	{
		if (IS_HEAD(victim, LOST_NOSE))
			return;
		if ((damaged = get_eq_char(victim, WEAR_FACE)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevents you from loosing your nose.", victim, damaged, NULL, TO_CHAR);
			act("$p prevents $n from loosing $s nose.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p falls broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p falls broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		SET_BIT(victim->loc_hp[0], LOST_NOSE);
		act("Your skillful blow cuts off $N's nose!", ch, NULL, victim, TO_CHAR);
		act("$n's skillful blow cuts off $N's nose!", ch, NULL, victim, TO_NOTVICT);
		act("$n's skillful blow cuts off your nose!", ch, NULL, victim, TO_VICT);
		make_part(victim, "nose");
		return;
	}
	else if (critical == 4)
	{
		if (IS_HEAD(victim, LOST_NOSE) || IS_HEAD(victim, BROKEN_NOSE))
			return;
		if ((damaged = get_eq_char(victim, WEAR_FACE)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevents you from breaking your nose.", victim, damaged, NULL, TO_CHAR);
			act("$p prevents $n from breaking $s nose.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p falls broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p falls broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		if (!IS_HEAD(victim, LOST_NOSE) && !IS_HEAD(victim, BROKEN_NOSE))
			SET_BIT(victim->loc_hp[0], BROKEN_NOSE);
		else
			return;
		act("Your skillful blow breaks $N's nose!", ch, NULL, victim, TO_CHAR);
		act("$n's skillful blow breaks $N's nose!", ch, NULL, victim, TO_NOTVICT);
		act("$n's skillful blow breaks your nose!", ch, NULL, victim, TO_VICT);
		return;
	}
	else if (critical == 5)
	{
		if (IS_HEAD(victim, BROKEN_JAW))
			return;
		if ((damaged = get_eq_char(victim, WEAR_FACE)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevents you from breaking your jaw.", victim, damaged, NULL, TO_CHAR);
			act("$p prevents $n from breaking $s jaw.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p falls broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p falls broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		if (!IS_HEAD(victim, BROKEN_JAW))
			SET_BIT(victim->loc_hp[0], BROKEN_JAW);
		else
			return;
		act("Your skillful blow breaks $N's jaw!", ch, NULL, victim, TO_CHAR);
		act("$n's skillful blow breaks $N's jaw!", ch, NULL, victim, TO_NOTVICT);
		act("$n's skillful blow breaks your jaw!", ch, NULL, victim, TO_VICT);
		return;
	}
	else if (critical == 6)
	{
		if (IS_ARM_L(victim, LOST_ARM))
			return;
		if ((damaged = get_eq_char(victim, WEAR_ARMS)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevent you from loosing your left arm.", victim, damaged, NULL, TO_CHAR);
			act("$p prevent $n from loosing $s left arm.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		if (!IS_ARM_L(victim, LOST_ARM))
			SET_BIT(victim->loc_hp[2], LOST_ARM);
		else
			return;
		if (!IS_BLEEDING(victim, BLEEDING_ARM_L))
			SET_BIT(victim->loc_hp[6], BLEEDING_ARM_L);
		if (IS_BLEEDING(victim, BLEEDING_HAND_L))
			REMOVE_BIT(victim->loc_hp[6], BLEEDING_HAND_L);
		act("Your skillful blow cuts off $N's left arm!", ch, NULL, victim, TO_CHAR);
		act("$n's skillful blow cuts off $N's left arm!", ch, NULL, victim, TO_NOTVICT);
		act("$n's skillful blow cuts off your left arm!", ch, NULL, victim, TO_VICT);
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
		return;
	}
	else if (critical == 7)
	{
		if (IS_ARM_R(victim, LOST_ARM))
			return;
		if ((damaged = get_eq_char(victim, WEAR_ARMS)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevent you from loosing your right arm.", victim, damaged, NULL, TO_CHAR);
			act("$p prevent $n from loosing $s right arm.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		if (!IS_ARM_R(victim, LOST_ARM))
			SET_BIT(victim->loc_hp[3], LOST_ARM);
		else
			return;
		if (!IS_BLEEDING(victim, BLEEDING_ARM_R))
			SET_BIT(victim->loc_hp[6], BLEEDING_ARM_R);
		if (IS_BLEEDING(victim, BLEEDING_HAND_R))
			REMOVE_BIT(victim->loc_hp[6], BLEEDING_HAND_R);
		act("Your skillful blow cuts off $N's right arm!", ch, NULL, victim, TO_CHAR);
		act("$n's skillful blow cuts off $N's right arm!", ch, NULL, victim, TO_NOTVICT);
		act("$n's skillful blow cuts off your right arm!", ch, NULL, victim, TO_VICT);
		make_part(victim, "arm");
		if (IS_ARM_L(victim, LOST_ARM) && IS_ARM_R(victim, LOST_ARM))
		{
			if ((obj = get_eq_char(victim, WEAR_ARMS)) != NULL)
				take_item(victim, obj);
		}
		if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
			take_item(victim, obj);
		if ((obj = get_eq_char(victim, WEAR_HANDS)) != NULL)
			take_item(victim, obj);
		if ((obj = get_eq_char(victim, WEAR_WRIST_R)) != NULL)
			take_item(victim, obj);
		if ((obj = get_eq_char(victim, WEAR_FINGER_R)) != NULL)
			take_item(victim, obj);
		return;
	}
	else if (critical == 8)
	{
		if (IS_ARM_L(victim, LOST_ARM) || IS_ARM_L(victim, BROKEN_ARM))
			return;
		if ((damaged = get_eq_char(victim, WEAR_ARMS)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevent you from breaking your left arm.", victim, damaged, NULL, TO_CHAR);
			act("$p prevent $n from breaking $s left arm.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		if (!IS_ARM_L(victim, BROKEN_ARM) && !IS_ARM_L(victim, LOST_ARM))
			SET_BIT(victim->loc_hp[2], BROKEN_ARM);
		else
			return;
		act("Your skillful blow breaks $N's left arm!", ch, NULL, victim, TO_CHAR);
		act("$n's skillful blow breaks $N's left arm!", ch, NULL, victim, TO_NOTVICT);
		act("$n's skillful blow breaks your left arm!", ch, NULL, victim, TO_VICT);
		if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
			take_item(victim, obj);
		return;
	}
	else if (critical == 9)
	{
		if (IS_ARM_R(victim, LOST_ARM) || IS_ARM_R(victim, BROKEN_ARM))
			return;
		if ((damaged = get_eq_char(victim, WEAR_ARMS)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevent you from breaking your right arm.", victim, damaged, NULL, TO_CHAR);
			act("$p prevent $n from breaking $s right arm.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		if (!IS_ARM_R(victim, BROKEN_ARM) && !IS_ARM_R(victim, LOST_ARM))
			SET_BIT(victim->loc_hp[3], BROKEN_ARM);
		else
			return;
		act("Your skillful blow breaks $N's right arm!", ch, NULL, victim, TO_CHAR);
		act("$n's skillful blow breaks $N's right arm!", ch, NULL, victim, TO_NOTVICT);
		act("$n's skillful blow breaks your right arm!", ch, NULL, victim, TO_VICT);
		if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
			take_item(victim, obj);
		return;
	}
	else if (critical == 10)
	{
		if (IS_ARM_L(victim, LOST_HAND) || IS_ARM_L(victim, LOST_ARM))
			return;
		if ((damaged = get_eq_char(victim, WEAR_HANDS)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevent you from loosing your left hand.", victim, damaged, NULL, TO_CHAR);
			act("$p prevent $n from loosing $s left hand.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		if (!IS_ARM_L(victim, LOST_HAND) && !IS_ARM_L(victim, LOST_ARM))
			SET_BIT(victim->loc_hp[2], LOST_HAND);
		else
			return;
		if (IS_BLEEDING(victim, BLEEDING_ARM_L))
			REMOVE_BIT(victim->loc_hp[6], BLEEDING_ARM_L);
		if (!IS_BLEEDING(victim, BLEEDING_HAND_L))
			SET_BIT(victim->loc_hp[6], BLEEDING_HAND_L);
		act("Your skillful blow cuts off $N's left hand!", ch, NULL, victim, TO_CHAR);
		act("$n's skillful blow cuts off $N's left hand!", ch, NULL, victim, TO_NOTVICT);
		act("$n's skillful blow cuts off your left hand!", ch, NULL, victim, TO_VICT);
		make_part(victim, "hand");
		if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
			take_item(victim, obj);
		if ((obj = get_eq_char(victim, WEAR_HANDS)) != NULL)
			take_item(victim, obj);
		if ((obj = get_eq_char(victim, WEAR_WRIST_L)) != NULL)
			take_item(victim, obj);
		if ((obj = get_eq_char(victim, WEAR_FINGER_L)) != NULL)
			take_item(victim, obj);
		return;
	}
	else if (critical == 11)
	{
		if (IS_ARM_R(victim, LOST_HAND) || IS_ARM_R(victim, LOST_ARM))
			return;
		if ((damaged = get_eq_char(victim, WEAR_HANDS)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevent you from loosing your right hand.", victim, damaged, NULL, TO_CHAR);
			act("$p prevent $n from loosing $s right hand.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		if (!IS_ARM_R(victim, LOST_HAND) && !IS_ARM_R(victim, LOST_ARM))
			SET_BIT(victim->loc_hp[3], LOST_HAND);
		else
			return;
		if (IS_BLEEDING(victim, BLEEDING_ARM_R))
			REMOVE_BIT(victim->loc_hp[6], BLEEDING_ARM_R);
		if (!IS_BLEEDING(victim, BLEEDING_HAND_R))
			SET_BIT(victim->loc_hp[6], BLEEDING_HAND_R);
		act("Your skillful blow cuts off $N's right hand!", ch, NULL, victim, TO_CHAR);
		act("$n's skillful blow cuts off $N's right hand!", ch, NULL, victim, TO_NOTVICT);
		act("$n's skillful blow cuts off your right hand!", ch, NULL, victim, TO_VICT);
		make_part(victim, "hand");
		if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
			take_item(victim, obj);
		if ((obj = get_eq_char(victim, WEAR_HANDS)) != NULL)
			take_item(victim, obj);
		if ((obj = get_eq_char(victim, WEAR_WRIST_R)) != NULL)
			take_item(victim, obj);
		if ((obj = get_eq_char(victim, WEAR_FINGER_R)) != NULL)
			take_item(victim, obj);
		return;
	}
	else if (critical == 12)
	{
		if (IS_ARM_L(victim, LOST_ARM))
			return;
		if (IS_ARM_L(victim, LOST_HAND))
			return;
		if (IS_ARM_L(victim, LOST_THUMB) && IS_ARM_L(victim, LOST_FINGER_I) &&
			IS_ARM_L(victim, LOST_FINGER_M) && IS_ARM_L(victim, LOST_FINGER_R) &&
			IS_ARM_L(victim, LOST_FINGER_L))
			return;
		if ((damaged = get_eq_char(victim, WEAR_HANDS)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevent you from loosing some fingers from your left hand.", victim, damaged, NULL, TO_CHAR);
			act("$p prevent $n from loosing some fingers from $s left hand.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		count = 0;
		count2 = 0;
		if (!IS_ARM_L(victim, LOST_THUMB) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[2], LOST_THUMB);
			count2 += 1;
			make_part(victim, "thumb");
		}
		if (!IS_ARM_L(victim, LOST_FINGER_I) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[2], LOST_FINGER_I);
			count += 1;
			make_part(victim, "index");
		}
		if (!IS_ARM_L(victim, LOST_FINGER_M) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[2], LOST_FINGER_M);
			count += 1;
			make_part(victim, "middle");
		}
		if (!IS_ARM_L(victim, LOST_FINGER_R) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[2], LOST_FINGER_R);
			count += 1;
			make_part(victim, "ring");
			if ((obj = get_eq_char(victim, WEAR_FINGER_L)) != NULL)
				take_item(victim, obj);
		}
		if (!IS_ARM_L(victim, LOST_FINGER_L) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[2], LOST_FINGER_L);
			count += 1;
			make_part(victim, "little");
		}
		if (count == 1)
			snprintf(buf2, MAX_INPUT_LENGTH, "finger");
		else
			snprintf(buf2, MAX_INPUT_LENGTH, "fingers");
		if (count > 0 && count2 > 0)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "Your skillful blow cuts off %d %s and the thumb from $N's left hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_CHAR);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow cuts off %d %s and the thumb from $N's left hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_NOTVICT);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow cuts off %d %s and the thumb from your left hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_VICT);
			if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
				take_item(victim, obj);
			return;
		}
		else if (count > 0)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "Your skillful blow cuts off %d %s from $N's left hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_CHAR);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow cuts off %d %s from $N's left hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_NOTVICT);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow cuts off %d %s from your left hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_VICT);
			if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
				take_item(victim, obj);
			return;
		}
		else if (count2 > 0)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "Your skillful blow cuts off the thumb from $N's left hand.");
			act(buf, ch, NULL, victim, TO_CHAR);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow cuts off the thumb from $N's left hand.");
			act(buf, ch, NULL, victim, TO_NOTVICT);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow cuts off the thumb from your left hand.");
			act(buf, ch, NULL, victim, TO_VICT);
			if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
				take_item(victim, obj);
			return;
		}
		return;
	}
	else if (critical == 13)
	{
		if (IS_ARM_L(victim, LOST_ARM))
			return;
		if (IS_ARM_L(victim, LOST_HAND))
			return;
		if ((IS_ARM_L(victim, LOST_THUMB) || IS_ARM_L(victim, BROKEN_THUMB)) &&
			(IS_ARM_L(victim, LOST_FINGER_I) || IS_ARM_L(victim, BROKEN_FINGER_I)) &&
			(IS_ARM_L(victim, LOST_FINGER_M) || IS_ARM_L(victim, BROKEN_FINGER_M)) &&
			(IS_ARM_L(victim, LOST_FINGER_R) || IS_ARM_L(victim, BROKEN_FINGER_R)) &&
			(IS_ARM_L(victim, LOST_FINGER_L) || IS_ARM_L(victim, BROKEN_FINGER_L)))
			return;
		if ((damaged = get_eq_char(victim, WEAR_HANDS)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevent you from breaking some fingers on your left hand.", victim, damaged, NULL, TO_CHAR);
			act("$p prevent $n from breaking some fingers on $s left hand.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		count = 0;
		count2 = 0;
		if (IS_ARM_L(victim, LOST_ARM))
			return;
		if (IS_ARM_L(victim, LOST_HAND))
			return;

		if (!IS_ARM_L(victim, BROKEN_THUMB) && !IS_ARM_L(victim, LOST_THUMB) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[2], BROKEN_THUMB);
			count2 += 1;
		}
		if (!IS_ARM_L(victim, BROKEN_FINGER_I) && !IS_ARM_L(victim, LOST_FINGER_I) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[2], BROKEN_FINGER_I);
			count += 1;
		}
		if (!IS_ARM_L(victim, BROKEN_FINGER_M) && !IS_ARM_L(victim, LOST_FINGER_M) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[2], BROKEN_FINGER_M);
			count += 1;
		}
		if (!IS_ARM_L(victim, BROKEN_FINGER_R) && !IS_ARM_L(victim, LOST_FINGER_R) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[2], BROKEN_FINGER_R);
			count += 1;
		}
		if (!IS_ARM_L(victim, BROKEN_FINGER_L) && !IS_ARM_L(victim, LOST_FINGER_L) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[2], BROKEN_FINGER_L);
			count += 1;
		}
		if (count == 1)
			snprintf(buf2, MAX_INPUT_LENGTH, "finger");
		else
			snprintf(buf2, MAX_INPUT_LENGTH, "fingers");
		if (count > 0 && count2 > 0)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "Your skillful breaks %d %s and the thumb on $N's left hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_CHAR);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow breaks %d %s and the thumb on $N's left hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_NOTVICT);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow breaks %d %s and the thumb on your left hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_VICT);
			if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
				take_item(victim, obj);
			return;
		}
		else if (count > 0)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "Your skillful blow breaks %d %s on $N's left hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_CHAR);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow breaks %d %s on $N's left hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_NOTVICT);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow breaks %d %s on your left hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_VICT);
			if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
				take_item(victim, obj);
			return;
		}
		else if (count2 > 0)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "Your skillful blow breaks the thumb on $N's left hand.");
			act(buf, ch, NULL, victim, TO_CHAR);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow breaks the thumb on $N's left hand.");
			act(buf, ch, NULL, victim, TO_NOTVICT);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow breaks the thumb on your left hand.");
			act(buf, ch, NULL, victim, TO_VICT);
			if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
				take_item(victim, obj);
			return;
		}
		return;
	}
	else if (critical == 14)
	{
		if (IS_ARM_R(victim, LOST_ARM))
			return;
		if (IS_ARM_R(victim, LOST_HAND))
			return;
		if (IS_ARM_R(victim, LOST_THUMB) && IS_ARM_R(victim, LOST_FINGER_I) &&
			IS_ARM_R(victim, LOST_FINGER_M) && IS_ARM_R(victim, LOST_FINGER_R) &&
			IS_ARM_R(victim, LOST_FINGER_L))
			return;
		if ((damaged = get_eq_char(victim, WEAR_HANDS)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevent you from loosing some fingers from your right hand.", victim, damaged, NULL, TO_CHAR);
			act("$p prevent $n from loosing some fingers from $s right hand.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		count = 0;
		count2 = 0;
		if (IS_ARM_R(victim, LOST_ARM))
			return;
		if (IS_ARM_R(victim, LOST_HAND))
			return;

		if (!IS_ARM_R(victim, LOST_THUMB) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[3], LOST_THUMB);
			count2 += 1;
			make_part(victim, "thumb");
		}
		if (!IS_ARM_R(victim, LOST_FINGER_I) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[3], LOST_FINGER_I);
			count += 1;
			make_part(victim, "index");
		}
		if (!IS_ARM_R(victim, LOST_FINGER_M) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[3], LOST_FINGER_M);
			count += 1;
			make_part(victim, "middle");
		}
		if (!IS_ARM_R(victim, LOST_FINGER_R) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[3], LOST_FINGER_R);
			count += 1;
			make_part(victim, "ring");
			if ((obj = get_eq_char(victim, WEAR_FINGER_R)) != NULL)
				take_item(victim, obj);
		}
		if (!IS_ARM_R(victim, LOST_FINGER_L) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[3], LOST_FINGER_L);
			count += 1;
			make_part(victim, "little");
		}
		if (count == 1)
			snprintf(buf2, MAX_INPUT_LENGTH, "finger");
		else
			snprintf(buf2, MAX_INPUT_LENGTH, "fingers");
		if (count > 0 && count2 > 0)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "Your skillful blow cuts off %d %s and the thumb from $N's right hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_CHAR);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow cuts off %d %s and the thumb from $N's right hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_NOTVICT);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow cuts off %d %s and the thumb from your right hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_VICT);
			if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
				take_item(victim, obj);
			return;
		}
		else if (count > 0)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "Your skillful blow cuts off %d %s from $N's right hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_CHAR);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow cuts off %d %s from $N's right hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_NOTVICT);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow cuts off %d %s from your right hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_VICT);
			if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
				take_item(victim, obj);
			return;
		}
		else if (count2 > 0)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "Your skillful blow cuts off the thumb from $N's right hand.");
			act(buf, ch, NULL, victim, TO_CHAR);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow cuts off the thumb from $N's right hand.");
			act(buf, ch, NULL, victim, TO_NOTVICT);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow cuts off the thumb from your right hand.");
			act(buf, ch, NULL, victim, TO_VICT);
			if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
				take_item(victim, obj);
			return;
		}
		return;
	}
	else if (critical == 15)
	{
		if (IS_ARM_R(victim, LOST_ARM))
			return;
		if (IS_ARM_R(victim, LOST_HAND))
			return;
		if ((IS_ARM_R(victim, LOST_THUMB) || IS_ARM_R(victim, BROKEN_THUMB)) &&
			(IS_ARM_R(victim, LOST_FINGER_I) || IS_ARM_R(victim, BROKEN_FINGER_I)) &&
			(IS_ARM_R(victim, LOST_FINGER_M) || IS_ARM_R(victim, BROKEN_FINGER_M)) &&
			(IS_ARM_R(victim, LOST_FINGER_R) || IS_ARM_R(victim, BROKEN_FINGER_R)) &&
			(IS_ARM_R(victim, LOST_FINGER_L) || IS_ARM_R(victim, BROKEN_FINGER_L)))
			return;
		if ((damaged = get_eq_char(victim, WEAR_HANDS)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevent you from breaking some fingers on your right hand.", victim, damaged, NULL, TO_CHAR);
			act("$p prevent $n from breaking some fingers on $s right hand.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		count = 0;
		count2 = 0;
		if (IS_ARM_R(victim, LOST_ARM))
			return;
		if (IS_ARM_R(victim, LOST_HAND))
			return;

		if (!IS_ARM_R(victim, BROKEN_THUMB) && !IS_ARM_R(victim, LOST_THUMB) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[3], BROKEN_THUMB);
			count2 += 1;
		}
		if (!IS_ARM_R(victim, BROKEN_FINGER_I) && !IS_ARM_R(victim, LOST_FINGER_I) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[3], BROKEN_FINGER_I);
			count += 1;
		}
		if (!IS_ARM_R(victim, BROKEN_FINGER_M) && !IS_ARM_R(victim, LOST_FINGER_M) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[3], BROKEN_FINGER_M);
			count += 1;
		}
		if (!IS_ARM_R(victim, BROKEN_FINGER_R) && !IS_ARM_R(victim, LOST_FINGER_R) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[3], BROKEN_FINGER_R);
			count += 1;
		}
		if (!IS_ARM_R(victim, BROKEN_FINGER_L) && !IS_ARM_R(victim, LOST_FINGER_L) && number_percent() < 25)
		{
			SET_BIT(victim->loc_hp[3], BROKEN_FINGER_L);
			count += 1;
		}
		if (count == 1)
			snprintf(buf2, MAX_INPUT_LENGTH, "finger");
		else
			snprintf(buf2, MAX_INPUT_LENGTH, "fingers");
		if (count > 0 && count2 > 0)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "Your skillful breaks %d %s and the thumb on $N's right hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_CHAR);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow breaks %d %s and the thumb on $N's right hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_NOTVICT);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow breaks %d %s and the thumb on your right hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_VICT);
			if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
				take_item(victim, obj);
			return;
		}
		else if (count > 0)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "Your skillful blow breaks %d %s on $N's right hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_CHAR);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow breaks %d %s on $N's right hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_NOTVICT);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow breaks %d %s on your right hand.", count, buf2);
			act(buf, ch, NULL, victim, TO_VICT);
			if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
				take_item(victim, obj);
			return;
		}
		else if (count2 > 0)
		{
			snprintf(buf, MAX_INPUT_LENGTH, "Your skillful blow breaks the thumb on $N's right hand.");
			act(buf, ch, NULL, victim, TO_CHAR);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow breaks the thumb on $N's right hand.");
			act(buf, ch, NULL, victim, TO_NOTVICT);
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow breaks the thumb on your right hand.");
			act(buf, ch, NULL, victim, TO_VICT);
			if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
				take_item(victim, obj);
			return;
		}
		return;
	}
	else if (critical == 16)
	{
		if (IS_LEG_L(victim, LOST_LEG))
			return;
		if ((damaged = get_eq_char(victim, WEAR_LEGS)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevent you from loosing your left leg.", victim, damaged, NULL, TO_CHAR);
			act("$p prevent $n from loosing $s left leg.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		if (!IS_LEG_L(victim, LOST_LEG))
			SET_BIT(victim->loc_hp[4], LOST_LEG);
		else
			return;
		if (!IS_BLEEDING(victim, BLEEDING_LEG_L))
			SET_BIT(victim->loc_hp[6], BLEEDING_LEG_L);
		if (IS_BLEEDING(victim, BLEEDING_FOOT_L))
			REMOVE_BIT(victim->loc_hp[6], BLEEDING_FOOT_L);
		act("Your skillful blow cuts off $N's left leg!", ch, NULL, victim, TO_CHAR);
		act("$n's skillful blow cuts off $N's left leg!", ch, NULL, victim, TO_NOTVICT);
		act("$n's skillful blow cuts off your left leg!", ch, NULL, victim, TO_VICT);
		make_part(victim, "leg");
		if (IS_LEG_L(victim, LOST_LEG) && IS_LEG_R(victim, LOST_LEG))
		{
			if ((obj = get_eq_char(victim, WEAR_LEGS)) != NULL)
				take_item(victim, obj);
		}
		if ((obj = get_eq_char(victim, WEAR_FEET)) != NULL)
			take_item(victim, obj);
		return;
	}
	else if (critical == 17)
	{
		if (IS_LEG_R(victim, LOST_LEG))
			return;
		if ((damaged = get_eq_char(victim, WEAR_LEGS)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevent you from loosing your right leg.", victim, damaged, NULL, TO_CHAR);
			act("$p prevent $n from loosing $s right leg.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		if (!IS_LEG_R(victim, LOST_LEG))
			SET_BIT(victim->loc_hp[5], LOST_LEG);
		else
			return;
		if (!IS_BLEEDING(victim, BLEEDING_LEG_R))
			SET_BIT(victim->loc_hp[6], BLEEDING_LEG_R);
		if (IS_BLEEDING(victim, BLEEDING_FOOT_R))
			REMOVE_BIT(victim->loc_hp[6], BLEEDING_FOOT_R);
		act("Your skillful blow cuts off $N's right leg!", ch, NULL, victim, TO_CHAR);
		act("$n's skillful blow cuts off $N's right leg!", ch, NULL, victim, TO_NOTVICT);
		act("$n's skillful blow cuts off your right leg!", ch, NULL, victim, TO_VICT);
		make_part(victim, "leg");
		if (IS_LEG_L(victim, LOST_LEG) && IS_LEG_R(victim, LOST_LEG))
		{
			if ((obj = get_eq_char(victim, WEAR_LEGS)) != NULL)
				take_item(victim, obj);
		}
		if ((obj = get_eq_char(victim, WEAR_FEET)) != NULL)
			take_item(victim, obj);
		return;
	}
	else if (critical == 18)
	{
		if (IS_LEG_L(victim, BROKEN_LEG) || IS_LEG_L(victim, LOST_LEG))
			return;
		if ((damaged = get_eq_char(victim, WEAR_LEGS)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevent you from breaking your left leg.", victim, damaged, NULL, TO_CHAR);
			act("$p prevent $n from breaking $s left leg.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		if (!IS_LEG_L(victim, BROKEN_LEG) && !IS_LEG_L(victim, LOST_LEG))
			SET_BIT(victim->loc_hp[4], BROKEN_LEG);
		else
			return;
		act("Your skillful blow breaks $N's left leg!", ch, NULL, victim, TO_CHAR);
		act("$n's skillful blow breaks $N's left leg!", ch, NULL, victim, TO_NOTVICT);
		act("$n's skillful blow breaks your left leg!", ch, NULL, victim, TO_VICT);
		return;
	}
	else if (critical == 19)
	{
		if (IS_LEG_R(victim, BROKEN_LEG) || IS_LEG_R(victim, LOST_LEG))
			return;
		if ((damaged = get_eq_char(victim, WEAR_LEGS)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevent you from breaking your right leg.", victim, damaged, NULL, TO_CHAR);
			act("$p prevent $n from breaking $s right leg.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		if (!IS_LEG_R(victim, BROKEN_LEG) && !IS_LEG_R(victim, LOST_LEG))
			SET_BIT(victim->loc_hp[5], BROKEN_LEG);
		else
			return;
		act("Your skillful blow breaks $N's right leg!", ch, NULL, victim, TO_CHAR);
		act("$n's skillful blow breaks $N's right leg!", ch, NULL, victim, TO_NOTVICT);
		act("$n's skillful blow breaks your right leg!", ch, NULL, victim, TO_VICT);
		return;
	}
	else if (critical == 20)
	{
		if (IS_LEG_L(victim, LOST_LEG) || IS_LEG_L(victim, LOST_FOOT))
			return;
		if ((damaged = get_eq_char(victim, WEAR_FEET)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevent you from loosing your left foot.", victim, damaged, NULL, TO_CHAR);
			act("$p prevent $n from loosing $s left foot.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		if (!IS_LEG_L(victim, LOST_LEG) && !IS_LEG_L(victim, LOST_FOOT))
			SET_BIT(victim->loc_hp[4], LOST_FOOT);
		else
			return;
		if (IS_BLEEDING(victim, BLEEDING_LEG_L))
			REMOVE_BIT(victim->loc_hp[6], BLEEDING_LEG_L);
		if (!IS_BLEEDING(victim, BLEEDING_FOOT_L))
			SET_BIT(victim->loc_hp[6], BLEEDING_FOOT_L);
		act("Your skillful blow cuts off $N's left foot!", ch, NULL, victim, TO_CHAR);
		act("$n's skillful blow cuts off $N's left foot!", ch, NULL, victim, TO_NOTVICT);
		act("$n's skillful blow cuts off your left foot!", ch, NULL, victim, TO_VICT);
		make_part(victim, "foot");
		if ((obj = get_eq_char(victim, WEAR_FEET)) != NULL)
			take_item(victim, obj);
		return;
	}
	else if (critical == 21)
	{
		if (IS_LEG_R(victim, LOST_LEG) || IS_LEG_R(victim, LOST_FOOT))
			return;
		if ((damaged = get_eq_char(victim, WEAR_FEET)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevent you from loosing your right foot.", victim, damaged, NULL, TO_CHAR);
			act("$p prevent $n from loosing $s right foot.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		if (!IS_LEG_R(victim, LOST_LEG) && !IS_LEG_R(victim, LOST_FOOT))
			SET_BIT(victim->loc_hp[5], LOST_FOOT);
		else
			return;
		if (IS_BLEEDING(victim, BLEEDING_LEG_R))
			REMOVE_BIT(victim->loc_hp[6], BLEEDING_LEG_R);
		if (!IS_BLEEDING(victim, BLEEDING_FOOT_R))
			SET_BIT(victim->loc_hp[6], BLEEDING_FOOT_R);
		act("Your skillful blow cuts off $N's right foot!", ch, NULL, victim, TO_CHAR);
		act("$n's skillful blow cuts off $N's right foot!", ch, NULL, victim, TO_NOTVICT);
		act("$n's skillful blow cuts off your right foot!", ch, NULL, victim, TO_VICT);
		make_part(victim, "foot");
		if ((obj = get_eq_char(victim, WEAR_FEET)) != NULL)
			take_item(victim, obj);
		return;
	}
	else if (critical == 22)
	{
		int bodyloc = 0;
		int broken = number_range(1, 3);
		if (IS_BODY(victim, BROKEN_RIBS_1))
			bodyloc += 1;
		if (IS_BODY(victim, BROKEN_RIBS_2))
			bodyloc += 2;
		if (IS_BODY(victim, BROKEN_RIBS_4))
			bodyloc += 4;
		if (IS_BODY(victim, BROKEN_RIBS_8))
			bodyloc += 8;
		if (IS_BODY(victim, BROKEN_RIBS_16))
			bodyloc += 16;
		if (bodyloc >= 24)
			return;

		if ((damaged = get_eq_char(victim, WEAR_BODY)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevents you from breaking some ribs.", victim, damaged, NULL, TO_CHAR);
			act("$p prevents $n from breaking some ribs.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		if (IS_BODY(victim, BROKEN_RIBS_1))
			REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_1);
		if (IS_BODY(victim, BROKEN_RIBS_2))
			REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_2);
		if (IS_BODY(victim, BROKEN_RIBS_4))
			REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_4);
		if (IS_BODY(victim, BROKEN_RIBS_8))
			REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_8);
		if (IS_BODY(victim, BROKEN_RIBS_16))
			REMOVE_BIT(victim->loc_hp[1], BROKEN_RIBS_16);
		if (bodyloc + broken > 24)
			broken -= 1;
		if (bodyloc + broken > 24)
			broken -= 1;
		bodyloc += broken;
		if (bodyloc >= 16)
		{
			bodyloc -= 16;
			SET_BIT(victim->loc_hp[1], BROKEN_RIBS_16);
		}
		if (bodyloc >= 8)
		{
			bodyloc -= 8;
			SET_BIT(victim->loc_hp[1], BROKEN_RIBS_8);
		}
		if (bodyloc >= 4)
		{
			bodyloc -= 4;
			SET_BIT(victim->loc_hp[1], BROKEN_RIBS_4);
		}
		if (bodyloc >= 2)
		{
			bodyloc -= 2;
			SET_BIT(victim->loc_hp[1], BROKEN_RIBS_2);
		}
		if (bodyloc >= 1)
		{
			bodyloc -= 1;
			SET_BIT(victim->loc_hp[1], BROKEN_RIBS_1);
		}
		snprintf(buf, MAX_INPUT_LENGTH, "Your skillful blow breaks %d of $N's ribs!", broken);
		act(buf, ch, NULL, victim, TO_CHAR);
		snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow breaks %d of $N's ribs!", broken);
		act(buf, ch, NULL, victim, TO_NOTVICT);
		snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow breaks %d of your ribs!", broken);
		act(buf, ch, NULL, victim, TO_VICT);
		return;
	}
	else if (critical == 23)
	{
		int bodyloc = 0;
		int broken = number_range(1, 3);
		if (IS_HEAD(victim, LOST_TOOTH_1))
			bodyloc += 1;
		if (IS_HEAD(victim, LOST_TOOTH_2))
			bodyloc += 2;
		if (IS_HEAD(victim, LOST_TOOTH_4))
			bodyloc += 4;
		if (IS_HEAD(victim, LOST_TOOTH_8))
			bodyloc += 8;
		if (IS_HEAD(victim, LOST_TOOTH_16))
			bodyloc += 16;
		if (bodyloc >= 28)
			return;

		if ((damaged = get_eq_char(victim, WEAR_FACE)) != NULL &&
			damaged->toughness > 0)
		{
			act("$p prevents you from loosing some teeth.", victim, damaged, NULL, TO_CHAR);
			act("$p prevents $n from loosing some teeth.", victim, damaged, NULL, TO_ROOM);
			if (IS_SET(damaged->quest, QUEST_INDEST) && victim->race >= 10)
				return;
			if (dam - damaged->toughness < 0)
				return;
			if ((dam - damaged->toughness > damaged->resistance))
				damaged->condition -= damaged->resistance;
			else
				damaged->condition -= (dam - damaged->toughness);
			if (damaged->condition < 1)
			{
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_CHAR);
				act("$p fall broken to the ground.", ch, damaged, NULL, TO_ROOM);
				obj_from_char(damaged);
				extract_obj(damaged);
			}
			return;
		}
		if (IS_HEAD(victim, LOST_TOOTH_1))
			REMOVE_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_1);
		if (IS_HEAD(victim, LOST_TOOTH_2))
			REMOVE_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_2);
		if (IS_HEAD(victim, LOST_TOOTH_4))
			REMOVE_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_4);
		if (IS_HEAD(victim, LOST_TOOTH_8))
			REMOVE_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_8);
		if (IS_HEAD(victim, LOST_TOOTH_16))
			REMOVE_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_16);
		if (bodyloc + broken > 28)
			broken -= 1;
		if (bodyloc + broken > 28)
			broken -= 1;
		bodyloc += broken;
		if (bodyloc >= 16)
		{
			bodyloc -= 16;
			SET_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_16);
		}
		if (bodyloc >= 8)
		{
			bodyloc -= 8;
			SET_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_8);
		}
		if (bodyloc >= 4)
		{
			bodyloc -= 4;
			SET_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_4);
		}
		if (bodyloc >= 2)
		{
			bodyloc -= 2;
			SET_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_2);
		}
		if (bodyloc >= 1)
		{
			bodyloc -= 1;
			SET_BIT(victim->loc_hp[LOC_HEAD], LOST_TOOTH_1);
		}
		snprintf(buf, MAX_INPUT_LENGTH, "Your skillful blow knocks out %d of $N's teeth!", broken);
		act(buf, ch, NULL, victim, TO_CHAR);
		snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow knocks out %d of $N's teeth!", broken);
		act(buf, ch, NULL, victim, TO_NOTVICT);
		snprintf(buf, MAX_INPUT_LENGTH, "$n's skillful blow knocks out %d of your teeth!", broken);
		act(buf, ch, NULL, victim, TO_VICT);
		if (broken >= 1)
			make_part(victim, "tooth");
		if (broken >= 2)
			make_part(victim, "tooth");
		if (broken >= 3)
			make_part(victim, "tooth");
		return;
	}
	return;
}

void take_item(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (obj == NULL)
		return;
	obj_from_char(obj);
	obj_to_room(obj, ch->in_room);
	act("You wince in pain and $p falls to the ground.", ch, obj, NULL, TO_CHAR);
	act("$n winces in pain and $p falls to the ground.", ch, obj, NULL, TO_ROOM);
	return;
}

void do_favour(CHAR_DATA *ch, char *argument)
{
	CHAR_DATA *victim;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	argument = one_argument(argument, arg1, MAX_INPUT_LENGTH);
	argument = one_argument(argument, arg2, MAX_INPUT_LENGTH);
	if (IS_NPC(ch))
		return;
	if (!IS_SET(ch->act, PLR_VAMPIRE) && (IS_EXTRA(ch, EXTRA_PRINCE) || ch->vampgen != 2))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

	if (arg1[0] == '\0' || arg2[0] == '\0')
	{
		send_to_char("Syntax is: favour <target> <prince/sire>\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg1)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (IS_NPC(victim))
	{
		send_to_char("Not on NPC's.\n\r", ch);
		return;
	}

	if (ch == victim)
	{
		send_to_char("Not on yourself!\n\r", ch);
		return;
	}

	if (!IS_SET(victim->act, PLR_VAMPIRE))
	{
		send_to_char("But they are not a vampire!\n\r", ch);
		return;
	}

	if (str_cmp(victim->clan, ch->clan))
	{
		send_to_char("You can only grant your favour to someone in your clan.\n\r", ch);
		return;
	}

	if (ch->vampgen >= victim->vampgen)
	{
		send_to_char("You can only grant your favour to someone of a lower generation.\n\r", ch);
		return;
	}

	if (victim->vampgen > 6)
	{
		send_to_char("There is no point in doing that, as they are unable to sire any childer.\n\r", ch);
		return;
	}

	if (!str_cmp(arg2, "prince") && ch->vampgen == 2)
	{
		if (IS_EXTRA(victim, EXTRA_PRINCE))
		{
			act("You remove $N's prince privilages!", ch, NULL, victim, TO_CHAR);
			act("$n removes $N's prince privilages!", ch, NULL, victim, TO_NOTVICT);
			act("$n removes your prince privilages!", ch, NULL, victim, TO_VICT);
			if (IS_EXTRA(victim, EXTRA_SIRE))
				REMOVE_BIT(victim->extra, EXTRA_SIRE);
			REMOVE_BIT(victim->extra, EXTRA_PRINCE);
			return;
		}
		act("You make $N a prince!", ch, NULL, victim, TO_CHAR);
		act("$n has made $N a prince!", ch, NULL, victim, TO_NOTVICT);
		act("$n has made you a prince!", ch, NULL, victim, TO_VICT);
		SET_BIT(victim->extra, EXTRA_PRINCE);
		if (IS_EXTRA(victim, EXTRA_SIRE))
			REMOVE_BIT(victim->extra, EXTRA_SIRE);
		return;
	}
	else if (!str_cmp(arg2, "sire") && (ch->vampgen == 2 || IS_EXTRA(ch, EXTRA_PRINCE)))
	{
		if (IS_EXTRA(victim, EXTRA_SIRE))
		{
			act("You remove $N's permission to sire a childe!", ch, NULL, victim, TO_CHAR);
			act("$n has removed $N's permission to sire a childe!", ch, NULL, victim, TO_NOTVICT);
			act("$n has remove your permission to sire a childe!", ch, NULL, victim, TO_VICT);
			REMOVE_BIT(victim->extra, EXTRA_SIRE);
			return;
		}
		act("You grant $N permission to sire a childe!", ch, NULL, victim, TO_CHAR);
		act("$n has granted $N permission to sire a childe!", ch, NULL, victim, TO_NOTVICT);
		act("$n has granted you permission to sire a childe!", ch, NULL, victim, TO_VICT);
		SET_BIT(victim->extra, EXTRA_SIRE);
		return;
	}
	else
		send_to_char("You are unable to grant that sort of favour.\n\r", ch);
	return;
}

void bash(CHAR_DATA *ch, CHAR_DATA *victim)
{
	char buf[MAX_INPUT_LENGTH];
	if (IS_AFFECTED(victim, AFF_FLYING))
		return;
	if (!IS_NPC(victim) && IS_VAMPAFF(victim, VAM_FLYING))
		return;
	if (victim->wait == 0)
	{
		snprintf(buf, MAX_INPUT_LENGTH, "$n bashes you and you go down!");
		ADD_COLOUR(ch, buf, WHITE, MAX_INPUT_LENGTH);
		act(buf, ch, NULL, victim, TO_VICT);
		snprintf(buf, MAX_INPUT_LENGTH, "You bash $N and $E goes down!");
		ADD_COLOUR(ch, buf, WHITE, MAX_INPUT_LENGTH);
		act(buf, ch, NULL, victim, TO_CHAR);
		snprintf(buf, MAX_INPUT_LENGTH, "$n bashes $N and $E goes down!");
		ADD_COLOUR(ch, buf, WHITE, MAX_INPUT_LENGTH);
		act(buf, ch, NULL, victim, TO_NOTVICT);

		WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
		WAIT_STATE(victim, 2 * PULSE_VIOLENCE);
		victim->position = POS_RESTING;
	}

	return;
}

void do_engage(CHAR_DATA *ch, char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, MAX_INPUT_LENGTH);

	if (arg[0] == '\0')
	{
		send_to_char("Engage whom?\n\r", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		send_to_char("You cannot engage yourself!\n\r", ch);
		return;
	}

	if(victim == ch->fighting)
	{
		send_to_char("You already fighting them!\n\r", ch);
		return;
	}

	if (!IS_NPC(victim) && (victim->level != 3 || ch->level != 3))
	{
		send_to_char("Both players must be avatars to fight.\n\r", ch);
		return;
	}

	if(victim->position != POS_FIGHTING)
	{
		send_to_char("They are not currently fighting you or anyone else.\n\r", ch);
		return;
	}

	ch->fighting = victim;

	snprintf(buf, MAX_STRING_LENGTH, "You engage %s.\n\r", victim->name);
	send_to_char(buf,ch);

	return;
}
