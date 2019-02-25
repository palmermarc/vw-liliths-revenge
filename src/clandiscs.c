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

/**
 * Fortitude, Rank 1 - Personal Armor - Take less damage (10%)
 */
void do_personal_armor(CHAR_DATA *ch, CLANDISC_DATA *disc) {
	char buf[MAX_INPUT_LENGTH];

	if (IS_NPC(ch))
		return;

	if (!IS_SET(ch->act, PLR_VAMPIRE))
	{
		send_to_char("Huh?\n\r", ch);
		return;
	}

    if (disc->isActive )
	{
		send_to_char("Your skin becomes weaker.\n\r", ch);
		if (IS_AFFECTED(ch, AFF_POLYMORPH))
			snprintf(buf, MAX_INPUT_LENGTH, "%s's skin becomes weaker.", ch->morph);
		else
			snprintf(buf, MAX_INPUT_LENGTH, "$n's skin becomes weaker.");
		act(buf, ch, NULL, NULL, TO_ROOM);
		disc->isActive = FALSE;
		return;
	}
	
	send_to_char("Your skin becomes hard enough to break weapons.\n\r", ch);
	
	if (IS_AFFECTED(ch, AFF_POLYMORPH))
		snprintf(buf, MAX_INPUT_LENGTH, "%s's skin becomes hard enough to break weapons.", ch->morph);
	else
		snprintf(buf, MAX_INPUT_LENGTH, "$n's skin becomes hard enough to break weapons.");

	act(buf, ch, NULL, NULL, TO_ROOM);
	disc->isActive = TRUE;
	return;
}

/*
 * Fortitude, Rank 2 - Resilient Minds - Mind altering powers succeed less on you (10% extra resist)
 */
void do_resilient_minds(CHAR_DATA *ch) {

}

 /*
 * Fortitude, Rank 3 - Armor of Kings - Weapons that hit you may break, gain a significant amount of armor.
 */
void do_armor_of_kings(CHAR_DATA *ch) {

}

 /*
 * Fortitude, Rank 4 - King of the Mountain - Focus purely on defense blocking all attacks, at the expense of all else. (100% block, no attacks)
 */
void do_king_of_the_mountain(CHAR_DATA *ch) {

}

/*
* Fortitude, Rank 5 - Repair the Undead Flesh - Heal yourself greatly (30%)
*/
void do_repair_undead_flesh(CHAR_DATA *ch) {

}

 /*
 * Fortitude, Rank 6 - Armored Flesh - Enemies that hit you deal damage to themselves (10%) --- Personal Armor - Damage resistance now 20% --- Resilient Minds now has additional resist (20%)
 */
void do_armored_flesh(CHAR_DATA *ch) {

}

 /*
 * Fortitude, Rank 7 - Arm of Prometheus - Limbs, eyes can no longer be removed.
 */
void do_arm_of_prometheus(CHAR_DATA *ch) {

}

 /*
 * Fortitude, Rank 8 - Stand Against All Foes - Gain increased dodge/parry/armor (5% for each enemy attacking you) Cannot exceed 80% dodge/parry with this skill.
 */
void do_stand_against_all_foes(CHAR_DATA *ch) {

}

 /*
 * Fortitude, Rank 9 - Shared Strength - Grant another your supernatural resistance (15% damage reduction) If they already have fortitude they get your full bonus.
 */
void do_shared_strength(CHAR_DATA *ch) {

}

/*
* Fortitude, Rank 10 - Eternal Vigilance - Take no damage from sunlight. Gain additional damage resistance (30% total)
*/
void do_eternal_vigilance(CHAR_DATA *ch) {

}

void do_heightened_senses(CHAR_DATA *ch) {

}

void do_aura_perception(CHAR_DATA *ch) {

}

void do_prediction(CHAR_DATA *ch) {

}

void do_clairvoyance(CHAR_DATA *ch) {

}

void do_spirit_travel(CHAR_DATA *ch) {

}

void do_the_mind_revealed(CHAR_DATA *ch) {

}

void do_anima_gathering(CHAR_DATA *ch) {

}

void do_ecstatic_agony(CHAR_DATA *ch) {

}

void do_psychic_assault(CHAR_DATA *ch) {

}

void do_master_of_the_dominion(CHAR_DATA *ch) {

}

void do_pact_with_animals(CHAR_DATA *ch) {

}

void do_beckoning(CHAR_DATA *ch) {

}

void do_quell_the_beast(CHAR_DATA *ch) {

}

void do_subsume_the_spirit(CHAR_DATA *ch) {

}

void do_drawing_out_the_beast(CHAR_DATA *ch) {

}

void do_tainted_oasis(CHAR_DATA *ch) {

}

void do_conquer_the_beast(CHAR_DATA *ch) {

}

void do_taunt_the_caged_beast(CHAR_DATA *ch) {

}

void do_unchain_the_ferocious_beast(CHAR_DATA *ch) {

}

void do_free_the_beast_within(CHAR_DATA *ch) {

}

void do_quickness(CHAR_DATA *ch) {

}

void do_precision(CHAR_DATA *ch) {

}

void do_momentum(CHAR_DATA *ch) {

}

void do_flawless_parry(CHAR_DATA *ch) {

}

void do_stutter_step(CHAR_DATA *ch) {

}

void do_flower_of_death(CHAR_DATA *ch) {

}

void do_zephyr(CHAR_DATA *ch) {

}

void do_paragon_of_motion(CHAR_DATA *ch) {

}

void do_the_unseen_storm(CHAR_DATA *ch) {

}

void do_between_the_ticks(CHAR_DATA *ch) {

}

void do_shadow_play(CHAR_DATA *ch) {

}

void do_shroud_of_night(CHAR_DATA *ch) {

}

void do_arms_of_the_abyss(CHAR_DATA *ch) {

}

void do_black_metamorphosis(CHAR_DATA *ch) {

}

void do_shadowstep(CHAR_DATA *ch) {

}

void do_the_darkness_within(CHAR_DATA *ch) {

}

void do_inner_darkness(CHAR_DATA *ch) {

}

void do_tchernabog(CHAR_DATA *ch) {

}

void do_ahrimans_demesne(CHAR_DATA *ch) {

}

void do_banishment(CHAR_DATA *ch) {

}

void do_crush(CHAR_DATA *ch) {

}

void do_the_fist_of_lillith(CHAR_DATA *ch) {

}

void do_earthshock(CHAR_DATA *ch) {

}

void do_aftershock(CHAR_DATA *ch) {

}

void do_the_forgers_hammer(CHAR_DATA *ch) {

}

void do_fist_of_the_titans(CHAR_DATA *ch) {

}

void do_brutality(CHAR_DATA *ch) {

}

void do_lend_the_supernatural_vigor(CHAR_DATA *ch) {

}

void do_might_of_the_heroes(CHAR_DATA *ch) {

}

void do_touch_of_pain(CHAR_DATA *ch) {

}

void do_awe(CHAR_DATA *ch) {

}

void do_dread_gaze(CHAR_DATA *ch) {

}

void do_majesty(CHAR_DATA *ch) {

}

void do_paralyzing_glance(CHAR_DATA *ch) {

}

void do_presence_summon(CHAR_DATA *ch) {

}

void do_bloodlust(CHAR_DATA *ch) {

}

void do_phobia(CHAR_DATA *ch) {

}

void do_stand_tall(CHAR_DATA *ch) {

}

void do_dream_world(CHAR_DATA *ch) {

}

void do_pure_majesty(CHAR_DATA *ch) {

}

void do_scorpions_touch(CHAR_DATA *ch) {

}

void do_dagons_call(CHAR_DATA *ch) {

}

void do_baals_caress(CHAR_DATA *ch) {

}

void do_taste_of_death(CHAR_DATA *ch) {

}

void do_erosion(CHAR_DATA *ch) {

}

void do_selective_silence(CHAR_DATA *ch) {

}

void do_blood_sweat(CHAR_DATA *ch) {

}

void do_thin_blood(CHAR_DATA *ch) {

}

void do_blood_of_destruction(CHAR_DATA *ch) {

}

void do_weaken_the_blood_of_the_ancients(CHAR_DATA *ch) {

}

void do_geomancy(CHAR_DATA *ch) {

}

void do_spark(CHAR_DATA *ch) {

}

void do_vertigo(CHAR_DATA *ch) {

}

void do_contortion(CHAR_DATA *ch) {

}

void do_blood_boil(CHAR_DATA *ch) {

}

void do_runes_of_power(CHAR_DATA *ch) {

}

void do_rout_the_charging_hordes(CHAR_DATA *ch) {

}

void do_cognizant_construction(CHAR_DATA *ch) {

}

void do_eye_of_the_storm(CHAR_DATA *ch) {

}

void do_rego_ignem(CHAR_DATA *ch) {

}

void do_malleable_visage(CHAR_DATA *ch) {

}

void do_fleshcraft(CHAR_DATA *ch) {

}

void do_bone_craft(CHAR_DATA *ch) {

}

void do_flesh_rot(CHAR_DATA *ch) {

}

void do_breath_of_the_dragon(CHAR_DATA *ch) {

}

void do_body_arsenal(CHAR_DATA *ch) {

}

void do_impalers_fence(CHAR_DATA *ch) {

}

void do_will_over_form(CHAR_DATA *ch) {

}

void do_skin_trap(CHAR_DATA *ch) {

}

void do_the_last_dracul(CHAR_DATA *ch) {

}

void do_cloak_of_shadows(CHAR_DATA *ch) {

}

void do_mask_of_a_thousand_faces(CHAR_DATA *ch) {

}

void do_fade_from_the_minds_eye(CHAR_DATA *ch) {

}

void do_the_silence_of_death(CHAR_DATA *ch) {

}

void do_cloak_the_gathering(CHAR_DATA *ch) {

}

void do_manifold_guise(CHAR_DATA *ch) {

}

void do_fortress_of_silence(CHAR_DATA *ch) {

}

void do_mental_maze(CHAR_DATA *ch) {

}

void do_obscure_gods_creation(CHAR_DATA *ch) {

}

void do_veil_of_blissful_ignorance(CHAR_DATA *ch) {

}
