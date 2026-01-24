/*spec.Ubers.c - Specs for Ubers by Fisher

     Written by Fisher for RoninMUD
     Last Modification Date: 3/31/2021

     Basic Specs for the mobs and rooms in the zone.
*/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "cmd.h"
#include "utility.h"
#include "act.h"
#include "fight.h"
#include "spec_assign.h"
#include "mob.spells.h"
#include "enchant.h"

/*Rooms */
#define UBER_MYSTIC_ROOM 12904
#define UBER_MYSTIC_HOLD_ROOM 12999
#define ZYCA_PRINCE_ROOM 10969
#define ZYCA_THROW_ROOM 10964

/*Objects */
#define UBER_SPIDERSILK_CLOAK 14600
#define UBER_THOUGHT_SLEEVES 14601
#define UBER_ENLIGHTENED_MANTLE 14602
#define UBER_UNSHAKEN_SANDALS 14603
#define UBER_INSIGHT_RING 14604




/*Mobs */
#define UBER_KING_SPIDER 14600
#define UBER_GREAT_MYSTIC 14601
#define UBER_ULT_MYSTIC 14602
#define UBER_ULT_MYSTIC_CLONE 14690
#define UBER_HUGE_WHIRLWIND 14603
#define UBER_ZYCA_PRINCE 14604
#define UBER_FIRE_NEWT_PHOENIX 14605
#define UBER_FIRE_NEWT_GOD 14606
#define UBER_BLUE_ROOK 14607
#define UBER_ZYCA_GUARD 14608




/*Miscellaneous strings */
//Generic States that are shifted to indicate different stages.
//Each Uber will use this differently.
#define STATE1 (1 << 0)	//1
#define STATE2 (1 << 1)	//2
#define STATE3 (1 << 2)	//4
#define STATE4 (1 << 3)	//8

#define GREAT_MYSTIC_ENCH_NAME  "Compressed Pressure Point"

#define MYSTIC_STIRRING_ENCH_NAME "Stirring of Enlightenment"
#define MYSTIC_PATH_ENCH_NAME "Path of Enlightenment"
#define MYSTIC_VISION_ENCH_NAME "Vision of Enlightenment"
#define MYSTIC_FULL_ENCH_NAME "State of Full Enlightenment"



/*======================================================================== */
/*===============================OBJECT SPECS============================= */
/*======================================================================== */

int check_mystic_set_pieces(CHAR *ch)
{
	int count = 0;
	OBJ *obj;

	if (!ch) return 0;
	
	for (int i = 0; i < MAX_WEAR; i++){
		obj = ch->equipment[i];
		
		if(!obj) continue;
		  
		switch (V_OBJ(obj))
		{
			case UBER_THOUGHT_SLEEVES:
			case UBER_ENLIGHTENED_MANTLE:
			case UBER_UNSHAKEN_SANDALS:
			case UBER_INSIGHT_RING:
				count++;
				break;
		}
		  
	}
	return count;
}

//Charm of the Snowbound Guardian	26344
int uber_thought_sleeves (OBJ *obj,CHAR *ch,int cmd,char *arg) {
  char buf[MIL];
  int i,j,set_pieces;
  CHAR *owner;

  if(cmd==MSG_TICK) {
    if(obj->obj_flags.value[3]>0) obj->obj_flags.value[3]--;
    return FALSE;
  }

  if(cmd==MSG_OBJ_ENTERING_GAME) {
    if(is_number(arg)) i=atoi(arg);
    else return FALSE;
    j=obj->obj_flags.value[3]*60; /* Secs left to recharge */
    if(i>j) obj->obj_flags.value[3]=0;
    return FALSE;
  }

	if (cmd == CMD_USE)
	{
		/* Don't spec if no ch. */
		if (!ch)
			return FALSE;
		/* Don't spec if ch is not awake. */
		if (!AWAKE(ch))
			return FALSE;
		/* Don't spec if obj is not equipped by the actor. */
		if (!(owner = obj->equipped_by))
			return FALSE;
		/* Don't spec if actor is not the owner. */
		if (ch != owner)
			return FALSE;

		one_argument(arg, buf);

		/* Return if no target. */
		if (!*buf)
			return FALSE;

    
	if (*buf && is_abbrev(buf, "sleeves")){
      if (!obj->obj_flags.value[3]) {
        
		set_pieces = check_mystic_set_pieces(ch);
		
		if(set_pieces == 4){			
			act("$n has gained full knowledge of the universe.",FALSE,ch,obj,0,TO_ROOM);        
			act("You have gained the full knowledge of the universe.",FALSE,ch,obj,0,TO_CHAR);
		}else if (set_pieces == 3){
			act("$n has mostly gained the knowledge of the universe.",FALSE,ch,obj,0,TO_ROOM);        
			act("You have mostly gained the knowledge of the universe.",FALSE,ch,obj,0,TO_CHAR);
		}else if (set_pieces == 2){
			act("$n has partially gained the knowledge of the universe.",FALSE,ch,obj,0,TO_ROOM);        
			act("You have partially gained the knowledge of the universe.",FALSE,ch,obj,0,TO_CHAR);
		}else if (set_pieces == 1){
			act("$n has started to gain the full knowledge of the universe.",FALSE,ch,obj,0,TO_ROOM);        
			act("You have started to gain the full knowledge of the universe.",FALSE,ch,obj,0,TO_CHAR);
		}
		
		switch (set_pieces)
		{
			case 4:
				spell_perceive(50, ch, ch, 0);
				/* fall through */
			case 3:
				spell_detect_invisibility(50, ch, ch, 0);
				/* fall through */
			case 2:
				spell_infravision(50, ch, ch, 0);
				/* fall through */
			case 1:
				spell_sense_life(50, ch, ch, 0);
				break;
			default:
				break;
		}
		
        obj->obj_flags.value[3]=30;
      } else {
        send_to_char ("The charm does nothing.\n",ch);
      }
      return TRUE;
    }
  }
  return FALSE;
}


int uber_enlightened_mantle(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  CHAR *owner,*tmp;
  char buf[MIL];
  int set_pieces;

  if(CHAOSMODE) return FALSE;

  if(cmd == MSG_DIE) {
    if(obj->equipped_by) {
      owner = obj->equipped_by;
      if(IS_MOB(owner)) return FALSE;
    }
    else{
      return FALSE;
	}
	
    if(IS_SET(world[CHAR_REAL_ROOM(owner)].room_flags, CHAOTIC)){
		return FALSE;
	}
	
	set_pieces = check_mystic_set_pieces(owner);
	
	act("$n's mantle flares with calm radiance, refusing to let death take hold.", TRUE, owner, 0, 0, TO_ROOM);
    send_to_char("Your mantle anchors you between moments, denying death.\n\r", owner);
	
	switch (set_pieces)
	{
		case 4: // If you have 4, retain more ability to keep fighting.
			GET_HIT(owner) = (GET_MAX_HIT(owner)/2);
			GET_MANA(owner) = (GET_MAX_MANA(owner)/2);
			GET_MOVE(owner) = (GET_MAX_MOVE(owner)/2);
			break;
		case 3: // If you have 3 - You can keep fighting
			GET_HIT(owner) = (GET_MAX_HIT(owner)/5);
			GET_MANA(owner) = 100;
			GET_MOVE(owner) = 10;
			spell_word_of_recall(GET_LEVEL(owner), owner, owner, 0);
			break;
		case 2:
			GET_HIT(owner) = (GET_MAX_HIT(owner)/10);
			GET_MANA(owner) = 10;
			GET_MOVE(owner) = 10;
			spell_word_of_recall(GET_LEVEL(owner), owner, owner, 0);
			break;
		case 1:
			GET_HIT(owner) = 1;
			GET_MANA(owner) = 0;
			GET_MOVE(owner) = 0;
			spell_word_of_recall(GET_LEVEL(owner), owner, owner, 0);
			break;
		default:
			break;
	}

    sprintf(buf,"%s rescued %s from %s [%d].", OBJ_SHORT(obj), GET_NAME(owner), world[CHAR_REAL_ROOM(owner)].name, CHAR_VIRTUAL_ROOM(owner));  wizlog(buf, LEVEL_WIZ, 6);
    log_s(buf);
	if(set_pieces < 4){
		for(tmp = world[CHAR_REAL_ROOM(owner)].people;tmp;tmp=tmp->next_in_room){
		  if(GET_OPPONENT(tmp) == owner){
			stop_fighting (tmp);
		  }
		}
		stop_fighting(owner);
	}
    unequip_char(owner, WEAR_ABOUT);
    obj_to_char(obj, owner);
    extract_obj(obj);

    
    return TRUE;
  }
  return FALSE;
}


int uber_unshaken_sandals(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
	CHAR *owner;
	int set_pieces;
	int restore;

	if (cmd == MSG_TICK){
		/* Don't spec if no ch. */
		ch = obj->equipped_by;
		
		if (!ch)
			return FALSE;
		
		/* Don't spec if ch is not awake. */
		if (!AWAKE(ch))
			return FALSE;
		/* Don't spec if obj is not equipped by the actor. */
		if (!(owner = obj->equipped_by))
			return FALSE;
		/* Don't spec if actor is not the owner. */
		if (ch != owner)
			return FALSE;
		if (IS_MOB(owner))
			return FALSE;
		
		
		/* Only help if movement is low */
		if (GET_MOVE(owner) > GET_MAX_MOVE(owner) / 2){
			return FALSE;
		}
		set_pieces = check_mystic_set_pieces(owner);

		switch (set_pieces)
		{
			case 4:
				restore = GET_MAX_MOVE(owner) / 4;
				break;
			case 3:
				restore = GET_MAX_MOVE(owner) / 6;
				break;
			case 2:
				restore = GET_MAX_MOVE(owner) / 8;
				break;
			case 1:
				restore = GET_MAX_MOVE(owner) / 10;
				break;
			default:
				return FALSE;
		}

		GET_MOVE(owner) = MIN(GET_MOVE(owner) + restore, GET_MAX_MOVE(owner));

		send_to_char("Your footing steadies as measured steps restore your strength.\n\r", owner);
		act("$n's stance steadies, each step deliberate and controlled.",TRUE, owner, 0, 0, TO_ROOM);

		return TRUE;
	}
	
	return FALSE;
}


void remove_other_mystic_enlightenment(CHAR *ch, const char *keep)
{
	static const char *enchants[] = {
		MYSTIC_STIRRING_ENCH_NAME,
		MYSTIC_PATH_ENCH_NAME,
		MYSTIC_VISION_ENCH_NAME,
		MYSTIC_FULL_ENCH_NAME
	};
	ENCH *current_enchant;
	int i;
	
	if (!ch)
		return;

	for (i = 0; i < NUMELEMS(enchants); i++)
	{
		current_enchant = get_enchantment_by_name(ch, enchants[i]);
		if (keep && !strcmp(enchants[i], keep))
			continue;
		
		if (enchanted_by(ch, enchants[i]))
			enchantment_remove(ch, current_enchant, FALSE);
	}
}

int mystic_enlightenment_enchantment(ENCH *ench, CHAR *ch, CHAR *signaler, int cmd, char *arg) {
  int set_pieces;
  set_pieces = check_mystic_set_pieces(ch);
  
  if (cmd == MSG_MOBACT) {
	  
	  switch (set_pieces)
		{
			case 4:
				remove_other_mystic_enlightenment(ch, MYSTIC_FULL_ENCH_NAME);

				if (!enchanted_by(ch, MYSTIC_FULL_ENCH_NAME))
				{
					enchantment_apply(ch, FALSE, MYSTIC_FULL_ENCH_NAME, 0, 60, ENCH_INTERVAL_TICK, 4, APPLY_DAMROLL, 0, 0, mystic_enlightenment_enchantment);

					send_to_char("Perfect clarity settles over you as enlightenment is fully realized.\n\r", ch);
				}
				break;

			case 3:
				remove_other_mystic_enlightenment(ch, MYSTIC_VISION_ENCH_NAME);

				if (!enchanted_by(ch, MYSTIC_VISION_ENCH_NAME))
				{
					enchantment_apply(ch, FALSE, MYSTIC_VISION_ENCH_NAME, 0, 60, ENCH_INTERVAL_TICK, 3, APPLY_DAMROLL, 0, 0, mystic_enlightenment_enchantment);

					send_to_char("Your vision sharpens as deeper truths reveal themselves.\n\r", ch);
				}
				break;

			case 2:
				remove_other_mystic_enlightenment(ch, MYSTIC_PATH_ENCH_NAME);

				if (!enchanted_by(ch, MYSTIC_PATH_ENCH_NAME))
				{
					enchantment_apply(ch, FALSE, MYSTIC_PATH_ENCH_NAME, 0, 60, ENCH_INTERVAL_TICK, 2, APPLY_DAMROLL, 0, 0, mystic_enlightenment_enchantment);

					send_to_char("You feel steadier as you continue along the path of enlightenment.\n\r", ch);
				}
				break;

			case 1:
				remove_other_mystic_enlightenment(ch, MYSTIC_STIRRING_ENCH_NAME);

				if (!enchanted_by(ch, MYSTIC_STIRRING_ENCH_NAME))
				{
					enchantment_apply(ch, FALSE, MYSTIC_STIRRING_ENCH_NAME, 0, 60, ENCH_INTERVAL_TICK, 1, APPLY_DAMROLL, 0, 0, mystic_enlightenment_enchantment);

					send_to_char("A faint awareness stirs within you.\n\r", ch);
				}
				break;
			default:
				remove_other_mystic_enlightenment(ch, NULL);
				send_to_char("You are no longer on the path to enlightenment.\n\r", ch);		
				break;
		}
    
    return FALSE;
  }

  return FALSE;
}


int uber_insight_ring(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
	CHAR *owner;
	int set_pieces;

	if (cmd == MSG_TICK){
		owner = obj->equipped_by;

		if (!owner)
			return FALSE;

		if (IS_MOB(owner))
			return FALSE;

		if (!AWAKE(owner))
			return FALSE;

		set_pieces = check_mystic_set_pieces(owner);

		switch (set_pieces)
		{
			case 4:
				remove_other_mystic_enlightenment(owner, MYSTIC_FULL_ENCH_NAME);

				if (!enchanted_by(owner, MYSTIC_FULL_ENCH_NAME))
				{
					enchantment_apply(owner, FALSE, MYSTIC_FULL_ENCH_NAME, 0, 60, ENCH_INTERVAL_TICK, 4, APPLY_DAMROLL, 0, 0, mystic_enlightenment_enchantment);

					send_to_char("Perfect clarity settles over you as enlightenment is fully realized.\n\r", owner);
				}
				break;

			case 3:
				remove_other_mystic_enlightenment(owner, MYSTIC_VISION_ENCH_NAME);

				if (!enchanted_by(owner, MYSTIC_VISION_ENCH_NAME))
				{
					enchantment_apply(owner, FALSE, MYSTIC_VISION_ENCH_NAME, 0, 60, ENCH_INTERVAL_TICK, 3, APPLY_DAMROLL, 0, 0, mystic_enlightenment_enchantment);

					send_to_char("Your vision sharpens as deeper truths reveal themselves.\n\r", owner);
				}
				break;

			case 2:
				remove_other_mystic_enlightenment(owner, MYSTIC_PATH_ENCH_NAME);

				if (!enchanted_by(owner, MYSTIC_PATH_ENCH_NAME))
				{
					enchantment_apply(owner, FALSE, MYSTIC_PATH_ENCH_NAME, 0, 60, ENCH_INTERVAL_TICK, 2, APPLY_DAMROLL, 0, 0, mystic_enlightenment_enchantment);

					send_to_char("You feel steadier as you continue along the path of enlightenment.\n\r", owner);
				}
				break;

			case 1:
				remove_other_mystic_enlightenment(owner, MYSTIC_STIRRING_ENCH_NAME);

				if (!enchanted_by(owner, MYSTIC_STIRRING_ENCH_NAME))
				{
					enchantment_apply(owner, FALSE, MYSTIC_STIRRING_ENCH_NAME, 0, 60, ENCH_INTERVAL_TICK, 1, APPLY_DAMROLL, 0, 0, mystic_enlightenment_enchantment);

					send_to_char("A faint awareness stirs within you.\n\r", owner);
				}
				break;			
		}

		return TRUE;
	}
	
	return FALSE;
}





/*======================================================================== */
/*================================ROOM SPECS============================== */
/*======================================================================== */

/*======================================================================== */
/*===============================MOBILE SPECS============================= */
/*======================================================================== */


/*  Utility Specs */

//Randomly Pick STR,DEX,INT,WIS or CON and lower the values
void apply_random_stat_down(CHAR *vict, char *enchant_name, int duration, int min, int max)
{
    int stat_to_adjust;
    int stat_adjustment;

    /* List of possible stats to reduce */
    const int stat_list[] = {
        APPLY_STR,
        APPLY_DEX,
        APPLY_INT,
        APPLY_WIS,
        APPLY_CON
    };

    /* Pick a random stat */
    stat_to_adjust = stat_list[number(0, NUMELEMS(stat_list) - 1)];

    /* Roll a random value between min and max */
    stat_adjustment = number(min, max);

    /* Make it negative */
    stat_adjustment = stat_adjustment * -1;

    /* Apply the enchantment */
    enchantment_apply(vict,TRUE,enchant_name,TYPE_UNDEFINED,duration,ENCH_INTERVAL_ROUND,stat_adjustment,stat_to_adjust,0, 0, 0);
}




//Mob Number 14600
//Load in Room 12094

int ub_uber_kingspider(CHAR *uber, CHAR *ch, int cmd, char *arg)
{
	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *vict, *next_vict;
	//You must declare the reward so you get AQP.
	int reward = 2;

	//Define any other variables
	OBJ *tmp;
	int hpdrain = 0;
	int manadrain = 0;

	switch (cmd)
	{
		case MSG_MOBACT:

			//if fighting - spec different attacks
			if (uber->specials.fighting)
			{
			 	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
				//Each Case statement that has an action needs to break out at the end.
				switch (number(0, 6))
				{
					case 0:
					case 1:
						vict = get_random_victim_fighting(uber);
						if (vict)
						{
							act("$n grabs $N and shoves a leg through their chest.", 0, uber, 0, vict, TO_NOTVICT);
							act("$n grabs you and shoves a leg through your chest.", 0, uber, 0, vict, TO_VICT);
							sprintf(buf, "%s is impaled by a giant leg.", GET_NAME(vict));
							act(buf, FALSE, uber, NULL, vict, TO_NOTVICT);
							sprintf(buf, "You are impaled by a giant leg.");
							act(buf, FALSE, uber, NULL, vict, TO_VICT);
							GET_HIT(vict) = GET_HIT(vict) - GET_HIT(vict) / 3;
						}

						break;
					case 2:
						vict = get_random_victim_fighting(uber);
						if (vict)
						{
							act("$n spins and hits $N with his abdomen, knocking them out.", 0, uber, 0, vict, TO_NOTVICT);
							act("$n spins and hits you with his abdomen, knocking you out.", 0, uber, 0, vict, TO_VICT);
							damage(uber, vict, 1200, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, PULSE_VIOLENCE *2);
						}

						break;
					case 3:
					case 4:

						act("$n hisses loudly and covers everyone in spidersilk.", 0, uber, 0, 0, TO_ROOM);
						for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(uber)); vict; vict = next_vict)
						{
							next_vict = CHAR_NEXT_IN_ROOM(vict);
							if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							damage(uber, vict, 900, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, PULSE_VIOLENCE *3);
						}
						break;
					case 5:
					case 6:
						vict = get_random_victim_fighting(uber);
						//This spec will heal the uber based on the random number. It will also drain stats from the vict (target character).  The Actions send different messages to the room.
						act("$n pierces you with long dripping fangs and sucks out some blood.", FALSE, uber, 0, vict, TO_VICT);
						act("$n pierces $N with long dripping fangs and sucks out some blood.", FALSE, uber, 0, vict, TO_NOTVICT);
						act("You pierce $N with long dripping fangs and suck out some blood.", FALSE, uber, 0, vict, TO_CHAR);
						send_to_char("You feel your blood be sucked from you.\n", vict);
						hpdrain = number(400, 600);
						manadrain = number(50, 200);
						//void drain_mana_hit_mv(struct char_data *ch, struct char_data *vict, int mana, int hit, int mv, bool add_m, bool add_hp, bool add_mv)
						if(vict){
							drain_mana_hit_mv(uber, vict, manadrain, hpdrain, 0, FALSE, FALSE, FALSE);
							GET_HIT(uber) = MIN(GET_MAX_HIT(uber), GET_HIT(uber) + (hpdrain *5));
						}
						break;
					default:
						break;
				}
			}
			else
			{
				if (chance(40))
				{
					do_say(uber, "Shakes its legs around the room and hisses.", CMD_SAY);
				}
			}

			break;
		case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain, the realm has been saved!\n\r", GET_SHORT(uber));
			send_to_room(buf, CHAR_REAL_ROOM(uber));
			mob_aq_reward(reward, uber);
			
			//55% chance to load the item.   Needed to remove Antis
			
			if(chance(55)){				
				tmp = read_object(UBER_SPIDERSILK_CLOAK, VIRTUAL);
				strip_object_antis(tmp,20,33);
				obj_to_room(tmp, CHAR_REAL_ROOM(uber));				
				
				sprintf(buf, "A %s flutters to the floor!\n\r", OBJ_SHORT(tmp));
				send_to_room(buf, CHAR_REAL_ROOM(uber));
			}
			
			break;
	}

	return FALSE;
}

//Mob Number 14601
//Load in Room 12931

//ub_uber_greatmystic
int ub_uber_greatmystic(CHAR *uber, CHAR *ch, int cmd, char *arg)
{
	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *tch, *vict, *next_vict;

	CHAR * wind;
	int uber_wind_nr;
	//You must declare the reward so you get AQP.
	int reward = 6;
	uber_wind_nr = real_mobile(UBER_HUGE_WHIRLWIND);
	//Define any other variables
	OBJ *obj = NULL;
	
	char *uber_gm_speak[4] = { "Let me show you the power of enlightenment.", "Have you come for my teachings?", "Respect your Elders!", "Humility awaits you in this chamber." };

	switch (cmd)
	{
		case MSG_TICK:
		
			if (cmd==MSG_TICK) {
				// equip gear if he doesn't have it
				
				obj=EQ(uber, WEAR_ARMS);
				if (obj && ( V_OBJ(obj) != UBER_THOUGHT_SLEEVES )){
					obj_to_char( unequip_char(uber, WEAR_ARMS), uber );
				}
				else if (!obj) {
				  obj=read_object(UBER_THOUGHT_SLEEVES, VIRTUAL);
				  strip_object_antis(obj,20,33);
				  equip_char(uber, obj, WEAR_ARMS);
				}
				obj=EQ(uber, WEAR_FEET);
				if (obj && ( V_OBJ(obj) != UBER_UNSHAKEN_SANDALS )){
					obj_to_char( unequip_char(uber, WEAR_FEET), uber );
				}
				else if (!obj) {
				  obj=read_object(UBER_UNSHAKEN_SANDALS, VIRTUAL);
				  strip_object_antis(obj,20,33);
				  equip_char(uber, obj, WEAR_FEET);
				}
				
				return FALSE;
			}
		
			break;
		case MSG_MOBACT:

			//Have him to chat to you.
			if (chance(40))
			{
				sprintf(buf, "%s", uber_gm_speak[number(0, NUMELEMS(uber_gm_speak) - 1)]);

				do_say(uber, buf, CMD_SAY);
			}

			//if fighting - spec different attacks
			if (uber->specials.fighting)
			{
			 	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
				//Each Case statement that has an action needs to break out at the end.

				switch (number(0, 6))
				{
					case 0:
						vict = get_random_victim_fighting(uber);
						if (vict)
						{
							act("$n calmly touches $N with his palm.  You don't need this, do you?", 0, uber, 0, vict, TO_NOTVICT);
							act("$n calmly touches you saying you don't need this, do you? ", 0, uber, 0, vict, TO_VICT);
							GET_HIT(vict) = GET_HIT(vict) *0.8;	//Remove 20% of Current HP
						}

						break;
					case 1:
						for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(uber)); vict; vict = next_vict)
						{
							next_vict = CHAR_NEXT_IN_ROOM(vict);
							if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							
							if (enchanted_by(vict, GREAT_MYSTIC_ENCH_NAME)){							
								damage(uber, vict, 1600, TYPE_UNDEFINED, DAM_PHYSICAL);
								act("$N is hit again in their arms.", 0, uber, 0, vict, TO_NOTVICT);
								act("You are hit again on your pressure points.", 0, uber, 0, vict, TO_VICT);
							}else if (chance(40)){
								
								act("$N looks weaker after being hit in the arms.", 0, uber, 0, vict, TO_NOTVICT);
								act("You feel weaker after getting hit in the arms.", 0, uber, 0, vict, TO_VICT);
								apply_random_stat_down(vict, GREAT_MYSTIC_ENCH_NAME, 10, 3, 6);
							}else{
								act("$n strikes $N in the arms with his fingers.", 0, uber, 0, vict, TO_NOTVICT);
								act("$n strikes you in the arm, hitting your pressure points. ", 0, uber, 0, vict, TO_VICT);
								damage(uber, vict, 1000, TYPE_UNDEFINED, DAM_PHYSICAL);
							}
							
						}
						
						break;

					case 2:
						vict = get_random_victim_fighting(uber);
						if (vict)
						{
							act("$n yells at $N saying do my paperwork as a stack of papers hits them in the face.", 0, uber, 0, vict, TO_NOTVICT);
							act("$n yells at you saying do my paperwork as a stack of papers hits you in the face.", 0, uber, 0, vict, TO_VICT);
							damage(uber, vict, 1500, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, PULSE_VIOLENCE *1);
						}

						break;
					case 3:
						// Spawn Uber Whirlwinds
						if ((tch = GET_OPPONENT(uber)) && mob_proto_table[uber_wind_nr].number < 1)
						{
							wind = read_mobile(uber_wind_nr, REAL);
							char_to_room(wind, CHAR_REAL_ROOM(uber));

							act("$n cackles gleefully and flashes a quick smirk at you.", FALSE, uber, 0, tch, TO_VICT);
							act("$n cackle at $N and flashes them a quick smirk.", FALSE, uber, 0, tch, TO_NOTVICT);
							act("$n appears in a huge gust of wind and water.", FALSE, wind, 0, 0, TO_ROOM);
							hit(wind, tch, TYPE_UNDEFINED);

							return FALSE;
						}

					case 4:
						break;
					case 5:
						//Group Stun
						act("$n screams 'Mystical Powers' and rants incoherently ", 0, uber, 0, 0, TO_ROOM);
						for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(uber)); vict; vict = next_vict)
						{
							next_vict = CHAR_NEXT_IN_ROOM(vict);
							if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							damage(uber, vict, 1200, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, PULSE_VIOLENCE *2);
						}

						break;
					case 6:
						act("$n punches himself in the leg.", FALSE, uber, 0, 0, TO_ROOM);
						do_say(uber, "Ahhh, feels so good.", CMD_SAY);
						GET_HIT(uber) = GET_HIT(uber) + 5000;
						break;
					default:
						break;
				}
			}

			//can add an else branch here if you want them to act but not in combat.

			break;

		case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain, the realm has been saved!\n\r", GET_SHORT(uber));
			send_to_room(buf, CHAR_REAL_ROOM(uber));
			mob_aq_reward(reward, uber);

			break;
	}

	return FALSE;
}

//Uber Ultimate Mystic Extra Functions

void uber_um_stun(CHAR *ch, CHAR *vict)
{
	//Final check for values.  Abort if they dont exist. 
	 if(!(vict) || !(ch)) return; 
	
	act("$n stuns you with a mighty blow on the head.", FALSE, ch, 0, vict, TO_VICT);
	act("$n stuns $N with a mighty blow on $S head.", FALSE, ch, 0, vict, TO_NOTVICT);
	act("You hit $N with a stunning blow. $E will be out for a while.", FALSE, ch, 0, vict, TO_CHAR);



	GET_POS(vict) = POSITION_STUNNED;

	if (GET_OPPONENT(vict))
	{
		stop_fighting(vict);
	}

	if (GET_OPPONENT(ch) && GET_OPPONENT(ch) == vict)
	{
		stop_fighting(ch);
		damage(ch, vict, 500, TYPE_UNDEFINED, DAM_PHYSICAL);
		WAIT_STATE(vict, 2 *PULSE_VIOLENCE);
	}
}

void uber_um_vanish(CHAR *ch)
{
	CHAR * t;

	act("$n vanishes into the shadows.", FALSE, ch, 0, 0, TO_ROOM);
	stop_fighting(ch);

	for (t = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)); t; t = CHAR_NEXT_IN_ROOM(t))
	{
		if (GET_OPPONENT(t) && GET_OPPONENT(t) == ch)
		{
			stop_fighting(t);
			send_to_char("You can't fight someone that vanished into thin air.\n", t);
		}
	}

	if (!(t = get_random_victim(ch)))
		return;

	act("$n materializes behind you. $e hits you in the head with a heavy punch.", FALSE, ch, 0, t, TO_VICT);
	act("$n materializes behind $N and hits them in the head with a heavy punch.", FALSE, ch, 0, t, TO_NOTVICT);
	act("You appear behind your next victim.", FALSE, ch, 0, t, TO_CHAR);
	hit(ch, t, TYPE_UNDEFINED);
}

//Mob Number 14602
//Load in Room 12904

//Uber Mystic Main Spec
//ub_uber_ultmystic
int ub_uber_ultmystic(CHAR *uber, CHAR *ch, int cmd, char *arg)
{
	//These are default declarations to give variables to characters.
	char buf[MAX_STRING_LENGTH];
	CHAR *vict, *next_vict, *vict2;

	CHAR *uber_mystic_clone;
	int uber_mystic_clone_nr,summon_room;

	uber_mystic_clone_nr = real_mobile(UBER_ULT_MYSTIC_CLONE);
	summon_room = UBER_MYSTIC_ROOM;

	//You must declare the reward so you get AQP.
	int reward = 6;
	int factor;

	//Define any other variables
	OBJ *obj = NULL;

	switch (cmd)
	{
		case MSG_TICK:
		
			if (cmd==MSG_TICK) {
				// equip gear if he doesn't have it
				
				obj=EQ(uber, WEAR_FINGER_R);
				if (obj && ( V_OBJ(obj) != UBER_INSIGHT_RING )){
					obj_to_char( unequip_char(uber, WEAR_FINGER_R), uber );
				}
				else if (!obj) {
				  obj=read_object(UBER_INSIGHT_RING, VIRTUAL);
				  strip_object_antis(obj,20,33);
				  equip_char(uber, obj, WEAR_FINGER_R);
				}
				obj=EQ(uber, WEAR_ABOUT);
				if (obj && ( V_OBJ(obj) != UBER_ENLIGHTENED_MANTLE )){
					obj_to_char( unequip_char(uber, WEAR_ABOUT), uber );
				}
				else if (!obj) {
				  obj=read_object(UBER_ENLIGHTENED_MANTLE, VIRTUAL);
				  strip_object_antis(obj,20,33);
				  equip_char(uber, obj, WEAR_ABOUT);
				}
				
				return FALSE;
			}
		
			break;
		
		case MSG_MOBACT:

			//if fighting - spec different attacks
			if (uber->specials.fighting)
			{
			 	//Go through different actions based on a switch case.   Adjust total number of actions to change percentages.
				//Each Case statement that has an action needs to break out at the end.

				factor = 10* GET_HIT(uber) / GET_MAX_HIT(uber);
				switch (factor)
				{
				 		//Stun a random victim.
					case 9:

						do_say(uber, "You do show some promise.", CMD_SAY);

						vict = get_random_victim(uber);
						
						if(vict){
							uber_um_stun(uber, vict);
						}
						
						
						break;
						//Vanish at 80% HP
					case 8:
						do_say(uber, "If your eyes cant keep up with me, how do you expect to win?", CMD_SAY);
						if(chance(50)){
							uber_um_vanish(uber);
						}else{
							vict = get_random_victim_fighting(uber);
							
							if(vict){
								uber_um_stun(uber, vict);
							}
						}

					case 7:
						
						//Summon a Single Clone to Fight.
						if (!IS_SET(GET_BANK(uber), STATE2))
						{
							do_say(uber, "Lets see how you handle true enlightenment.", CMD_SAY);
							SET_BIT(GET_BANK(uber), STATE2);
							//Summon a Clone and have them attack.
							
							uber_mystic_clone = read_mobile(uber_mystic_clone_nr, REAL);
							char_to_room(uber_mystic_clone, real_room(summon_room));
							
							vict = get_random_victim(uber_mystic_clone);
							if(vict){
								hit(uber_mystic_clone, vict, TYPE_UNDEFINED);
							}
							
							//Remove the Uber from the Room.							
							char_from_room(uber);
							char_to_room(uber, real_room(UBER_MYSTIC_HOLD_ROOM));
							
						}else {
						
							//Group Stun
							act("$n shouts Bow to the master of the mystical arts. ", 0, uber, 0, 0, TO_ROOM);
							for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(uber)); vict; vict = next_vict)
							{
								next_vict = CHAR_NEXT_IN_ROOM(vict);
								if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
								damage(uber, vict, 1000, TYPE_UNDEFINED, DAM_PHYSICAL);
								WAIT_STATE(vict, PULSE_VIOLENCE *1);
							}
						}

						break;
						//50/50 chance of single stun or double stun.
					case 6:
						do_say(uber, "How many shall I target today?", CMD_SAY);
						switch (number(0, 1))
						{
							case 0:
								if (!(vict = get_random_victim(uber)))
								{
									return FALSE;
								}

								uber_um_stun(uber, vict);
								break;

							case 1:

								vict = get_random_victim_fighting(uber);
								if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) return FALSE;
								uber_um_stun(uber, vict);

								//Check for more than 1 person in the room.
								if (count_mortals_room(uber, TRUE) > 1)
								{
								 						//get 2nd victim that is still fighting
									vict2 = get_random_victim_fighting(uber);
									if (!(vict) || IS_NPC(vict2) || !(IS_MORTAL(vict2))) return FALSE;
									uber_um_stun(uber, vict2);
								}
						}

						break;

						//Heal for 10% of max hp. Only do it once, else this would be infinite loop.
					case 5:

						//STATE1 is Healed.  We will set the bit and then be done healing.
						if (!IS_SET(GET_BANK(uber), STATE1))
						{
							do_say(uber, "May enlightenment one day also find you.", CMD_SAY);
							SET_BIT(GET_BANK(uber), STATE1);
							GET_HIT(uber) += (GET_MAX_HIT(uber) / 10);
						}

						break;

					case 4:
						do_say(uber, "Bathe in mystical flames.", CMD_SAY);
						vict = get_random_victim_fighting(uber);
						act("$n makes a complex hand gesture and engulfs $N in flames.", 0, uber, 0, vict, TO_NOTVICT);
						act("$n makes a complex hand gesture and engulfs you in flames.", 0, uber, 0, vict, TO_VICT);
						sprintf(buf, "%s is engulfed by %s!", GET_NAME(vict), "mystical flames.");
						act(buf, FALSE, uber, NULL, vict, TO_NOTVICT);
						sprintf(buf, "You are engulfed by mystical flames.");
						act(buf, FALSE, uber, NULL, vict, TO_VICT);
						GET_HIT(vict) = GET_HIT(vict) / 3;
						break;

						//Vanish Again.
					case 3:
						do_say(uber, "Once again you were too slow to keep up.", CMD_SAY);
						uber_um_vanish(uber);
						break;

					case 2:
						do_say(uber, "Time to end this charade. Stand there and die.", CMD_SAY);
						for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(uber)); vict; vict = next_vict)
						{
							next_vict = CHAR_NEXT_IN_ROOM(vict);
							if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) continue;
							damage(uber, vict, 1600, TYPE_UNDEFINED, DAM_PHYSICAL);
							WAIT_STATE(vict, PULSE_VIOLENCE *3);
						}

						break;

					case 1:
						do_say(uber, "Wonderful. Absolutely wonderful. Continue your development.", CMD_SAY);

						switch (number(0, 1))
						{
							case 0:
								if (!(vict = get_random_victim(uber)))
								{
									return FALSE;
								}

								uber_um_stun(uber, vict);
								break;

							case 1:

								vict = get_random_victim_fighting(uber);
								if (!(vict) || IS_NPC(vict) || !(IS_MORTAL(vict))) return FALSE;
								uber_um_stun(uber, vict);

								//Check for more than 1 person in the room.
								if (count_mortals_room(uber, TRUE) > 1)
								{
								 	//get 2nd victim that is still fighting
									vict2 = get_random_victim_fighting(uber);
									if (!(vict) || IS_NPC(vict2) || !(IS_MORTAL(vict2))) return FALSE;
									uber_um_stun(uber, vict2);
								}
						}

						break;
				}
			}
			else
			{
			 	//Remove Existing States if not fighting.

				/*If not fighting, remove the heal bit. */
				if (!GET_OPPONENT(uber) && IS_SET(GET_BANK(uber), STATE1))
				{
					REMOVE_BIT(GET_BANK(uber), STATE1);

					return FALSE;
				}
			}

			//can add an else branch here if you want them to act but not in combat.

			break;

		case MSG_DIE:	// on boss death reward AQP
			sprintf(buf, "%s has been slain, the realm has been saved!\n\r", GET_SHORT(uber));
			send_to_room(buf, CHAR_REAL_ROOM(uber));
			mob_aq_reward(reward, uber);

			break;
	}

	return FALSE;
}

int ub_uber_ultmystic_clone(CHAR *uber, CHAR *ch, int cmd, char *arg)
{
	char buf[MAX_STRING_LENGTH];
	CHAR *vict, *next_vict;
	int reward = 1;
	int factor;
	
	CHAR * uber_mystic;

	/* Don't waste CPU if no mortals */
	if (count_mortals_room(uber, TRUE) < 1)
		return FALSE;

	switch (cmd)
	{
		case MSG_MOBACT:

			if (uber->specials.fighting)
			{
				factor = 10 * GET_HIT(uber) / GET_MAX_HIT(uber);

				switch (factor)
				{
					/* High HP – Reflection lesson */
					case 9:
					case 8:
						do_say(uber, "Power without awareness returns to its source.", CMD_SAY);

						for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(uber)); vict; vict = next_vict)
						{
							next_vict = CHAR_NEXT_IN_ROOM(vict);
							if (!vict || IS_NPC(vict) || !IS_MORTAL(vict))
								continue;

							if (GET_OPPONENT(vict) == uber)
							{
								act("Mystic energy rebounds from $n back into you!", FALSE, uber, 0, vict, TO_VICT);
								damage(uber, vict, 1400, TYPE_UNDEFINED, DAM_PHYSICAL);
							}
							else{
								damage(uber, vict, 800, TYPE_UNDEFINED, DAM_PHYSICAL);
							}
						}
						break;

					/* Mid HP – Equalization */
					case 7:
					case 6:
						do_say(uber, "Balance must be restored.", CMD_SAY);

						for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(uber)); vict; vict = next_vict)
						{
							next_vict = CHAR_NEXT_IN_ROOM(vict);
							if (!vict || IS_NPC(vict) || !IS_MORTAL(vict))
								continue;

							if (GET_HIT(vict) > GET_MAX_HIT(vict) / 2)
								GET_HIT(vict) = GET_MAX_HIT(vict) / 2;
						}
						break;

					/* First Enlightenment Heal – once only */
					case 5:
						if (!IS_SET(GET_BANK(uber), STATE1))
						{
							do_say(uber, "Understanding precedes survival.", CMD_SAY);
							SET_BIT(GET_BANK(uber), STATE1);
							GET_HIT(uber) += GET_MAX_HIT(uber) / 5;
						}
						break;

					/* Focus punishment */
					case 4:
						do_say(uber, "You cling too tightly to a single path.", CMD_SAY);

						vict = get_random_victim_fighting(uber);
						if (vict && !IS_NPC(vict))
						{
							act("$n gestures calmly, and your strength falters.", FALSE, uber, 0, vict, TO_VICT);
							WAIT_STATE(vict, PULSE_VIOLENCE * 2);
						}
						break;

					/* Low HP – Test of restraint */
					case 3:
					case 2:
						do_say(uber, "Strike without thought, and be unmade.", CMD_SAY);

						for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(uber)); vict; vict = next_vict)
						{
							next_vict = CHAR_NEXT_IN_ROOM(vict);
							if (!vict || IS_NPC(vict) || !IS_MORTAL(vict))
								continue;

							damage(uber, vict, 900, TYPE_UNDEFINED, DAM_PHYSICAL);
						}
						break;

					/* Near death – calm acceptance */
					case 1:
						do_say(uber, "You have learned enough. Finish it.", CMD_SAY);
						break;
				}
			}
			else
			{
				/* Reset heal state when not fighting */
				if (!GET_OPPONENT(uber) && IS_SET(GET_BANK(uber), STATE1))
					REMOVE_BIT(GET_BANK(uber), STATE1);
			}
			break;

		case MSG_DIE:
			sprintf(buf, "%s fades, leaving behind only understanding.\n\r", GET_SHORT(uber));
			send_to_room(buf, CHAR_REAL_ROOM(uber));
			mob_aq_reward(reward, uber);
			
			//On Death, move the Uber back to the Room.
			uber_mystic = get_ch_world(UBER_ULT_MYSTIC);
			if(uber_mystic){
				char_from_room(uber_mystic);
				char_to_room(uber_mystic, real_room(UBER_MYSTIC_ROOM));
			}
			
			vict = get_random_victim(uber);
			if(vict){
			
				hit(uber_mystic, vict, TYPE_UNDEFINED);
			}
			
			break;
	}

	return FALSE;
}


//Mob Number 14604
//Load in Room 10969

int ub_uber_zycaprince(CHAR *uber, CHAR *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    CHAR *vict, *next_vict;
    int reward = 3;

	int summon_room = ZYCA_PRINCE_ROOM;
	
	CHAR *guard_summon;
	int guard_summon_nr;

    char *uber_zp_speak[] = {
        "Jeez Dad, leave me alone.",
        "You're not my real dad!",
        "The bachelor life is the life for me.",
        "Bah... I don't wanna do anything."
    };

    switch (cmd)
    {
        case MSG_MOBACT:

            if (chance(35)){
                do_say(uber, uber_zp_speak[number(0, 3)], CMD_SAY);
			}

            if (uber->specials.fighting)
            {
				
				//If Fighting, Summon Guards. Only Summon Once.
				
				if(!IS_SET(GET_BANK(uber), STATE1)){	
					
					SET_BIT(GET_BANK(uber), STATE1);
					guard_summon_nr = real_mobile(UBER_ZYCA_GUARD);
					do_say(uber, "Guards.  Protect me!", CMD_SAY);
					
					if(mob_proto_table[guard_summon_nr].number < 1){
						
						//Summon the Mob.
						guard_summon = read_mobile(guard_summon_nr, REAL);						
						char_to_room(guard_summon, real_room(summon_room));
						
					}
				}
				
				
                switch (number(0, 4))
                {
                    case 0: /* Pillow Toss */
                        vict = get_random_victim_fighting(uber);
                        if (vict)
                        {
                            act("$n screams and hurls a pillow at $N!", 0, uber, 0, vict, TO_ROOM);
                            damage(uber, vict, number(700, 1000), TYPE_UNDEFINED, DAM_PHYSICAL);
                        }
                        break;

                    case 1: /* Tantrum AoE */
                        act("$n throws himself into a wild tantrum!", 0, uber, 0, 0, TO_ROOM);
                        for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(uber)); vict; vict = next_vict)
                        {
                            next_vict = CHAR_NEXT_IN_ROOM(vict);
                            if (IS_NPC(vict) || !IS_MORTAL(vict)) continue;
                            damage(uber, vict, number(450, 1250), TYPE_UNDEFINED, DAM_PHYSICAL);
                        }
                        break;

                    case 2: /* Sulking Defense */
                        act("$n sulks and refuses to back down.", 0, uber, 0, 0, TO_ROOM);
                        affect_apply(uber, SPELL_ARMOR, 0, APPLY_ARMOR, -50, 0, 0);
                        break;

                    case 3: /* Shove Tank */
                        vict = get_random_victim_fighting(uber); 
						
                        if (vict && GET_OPPONENT(uber))
                        {
                            act("$n angrily shoves $N!", 0, uber, 0, vict, TO_ROOM);
                            damage(uber, vict, number(800, 1450), TYPE_UNDEFINED, DAM_PHYSICAL);
                            WAIT_STATE(vict, PULSE_VIOLENCE);
                        }else{
							act("$n angrily kicks $N!", 0, uber, 0, vict, TO_ROOM);
                            damage(uber, vict, number(600, 1150), TYPE_UNDEFINED, DAM_PHYSICAL);
						}
                        break;

                    default: /* Lazy Swipe */
                        vict = get_random_victim_fighting(uber);
                        if (vict)
							act("$n lazily swings his arm at $N!", 0, uber, 0, vict, TO_ROOM);
                            damage(uber, vict, number(500, 900), TYPE_UNDEFINED, DAM_PHYSICAL);
                        break;
                }
            }
            break;

        case MSG_DIE:
            sprintf(buf, "%s has been slain, the realm has been saved!\n\r", GET_SHORT(uber));
            send_to_room(buf, CHAR_REAL_ROOM(uber));
			mob_aq_reward(reward, uber);
			break;
    }

    return FALSE;
}




int ubers_zyca_guard_npc_rescue (CHAR *ch_hero, CHAR *ch_victim) {
  CHAR *ch_bad_guy ;

  for (ch_bad_guy=world[CHAR_REAL_ROOM(ch_hero)].people;
       ch_bad_guy && (ch_bad_guy->specials.fighting != ch_victim);
       ch_bad_guy=ch_bad_guy->next_in_room)  ;
  if (ch_bad_guy) {
    if (ch_bad_guy == ch_hero)
      return FALSE ; /* NO WAY I'll rescue the one I'm fighting! */
    act ("You bravely rescue $N.\n\r", FALSE, ch_hero, 0, ch_victim, TO_CHAR) ;
    act("You are rescued by $N.\n\r",
	FALSE, ch_victim, 0, ch_hero, TO_CHAR);
	act("How dare you touch $N.", FALSE, ch_hero, 0, ch_victim, TO_NOTVICT);
    act("With a violent cry, $n throws $mself in front of $N.", FALSE, ch_hero, 0, ch_victim, TO_NOTVICT);

    if (ch_bad_guy->specials.fighting)
      stop_fighting(ch_bad_guy);
    if (ch_hero->specials.fighting)
      stop_fighting(ch_hero);

    set_fighting(ch_hero, ch_bad_guy);
    set_fighting(ch_bad_guy, ch_hero);
    return TRUE ;
  }
  return FALSE ;
}

int ub_uber_zyca_covert_guard(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    
    CHAR *vict, *next_vict;
    CHAR *zyca_prince;
	int stun_delay;

    switch (cmd)
    {
    case MSG_MOBACT:
		
		//If the prince exists, then you need to rescue him if he is fighting.
		zyca_prince = get_ch_world(UBER_ZYCA_PRINCE);

		if(zyca_prince && GET_OPPONENT(zyca_prince)){
			ubers_zyca_guard_npc_rescue(mob,zyca_prince);				
		}


        if (mob->specials.fighting)
        {
			
			
            switch (number(0, 4))
            {
            case 0: /* Precision Strike */
                vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("$n strikes swiftly at $N's joints.", 0, mob, 0, vict, TO_NOTVICT);
                    act("A sharp blow numbs your body!", 0, mob, 0, vict, TO_VICT);
                    damage(mob, vict, number(500, 700), TYPE_UNDEFINED, DAM_PHYSICAL);
                }
                break;

            case 1: /* Nerve Disrupt – Short Stun */
                vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("$n jabs $N with brutal precision.", 0, mob, 0, vict, TO_NOTVICT);
                    act("Your muscles lock up as pain surges through you!", 0, mob, 0, vict, TO_VICT);
                    stun_delay = number(1, 2);
                    WAIT_STATE(vict, stun_delay * PULSE_VIOLENCE);
                }
                break;

            case 2: /* Smoke Rush – AoE Pressure */
                act("$n drops a small sphere that erupts in choking smoke!", 0, mob, 0, 0, TO_ROOM);
                for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
                {
                    next_vict = CHAR_NEXT_IN_ROOM(vict);
                    if (!vict || IS_NPC(vict) || !IS_MORTAL(vict))
                        continue;

                    if (chance(60))
                    {
                        act("You cough violently as smoke fills your lungs!", 0, mob, 0, vict, TO_VICT);
                        damage(mob, vict, number(600, 1000), TYPE_UNDEFINED, DAM_PHYSICAL);
                        WAIT_STATE(vict, 1 * PULSE_VIOLENCE);
                    }
                }
                break;

            case 3: /* Forced Extraction – Room Push */
                vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("$n grabs $N and hurls them bodily away!", 0, mob, 0, vict, TO_NOTVICT);
                    act("You are seized and violently thrown out of the area!", 0, mob, 0, vict, TO_VICT);

                    damage(mob, vict, number(600, 800), TYPE_UNDEFINED, DAM_PHYSICAL);

                    char_from_room(vict);
					char_to_room(vict, real_room(ZYCA_THROW_ROOM));
                    do_look(vict, "", CMD_LOOK);

                    act("$n stumbles in from another direction, clearly disoriented.", FALSE, vict, 0, 0, TO_ROOM);
                }
                break;

            case 4: /* Bodyguard Intercept – Tank Peel */
            default:
                vict = GET_OPPONENT(mob);
                if (vict)
                {
                    act("$n steps between the prince and $N, forcing them back!", 0, mob, 0, vict, TO_NOTVICT);
                    act("You are shoved back by a covert guard!", 0, mob, 0, vict, TO_VICT);
                    stop_fighting(vict);
                    WAIT_STATE(vict, 1 * PULSE_VIOLENCE);
                }
                break;
            }
        }
        break;
    }

    return FALSE;
}









//Mob Number 14605
//Load in Room 2933

int ub_uber_firenewtphoenix(CHAR *uber, CHAR *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    CHAR *vict, *next_vict;
    int reward = 4;

    if (count_mortals_room(uber, TRUE) < 1) return FALSE;

    switch (cmd)
    {
        case MSG_MOBACT:

            if (uber->specials.fighting)
            {
                switch (number(0, 4))
                {
                    case 0: /* Claw Frenzy */
                        vict = get_random_victim_fighting(uber);
                        if (vict)
                        {
                            act("$n rends $N with blazing claws!", 0, uber, 0, vict, TO_ROOM);
                            damage(uber, vict, number(1500, 1800), TYPE_UNDEFINED, DAM_PHYSICAL);
                        }
                        break;

                    case 1: /* Beak Pierce */
                        vict = get_random_victim_fighting(uber);
                        if (vict)
                        {
                            act("$n drives her burning beak into $N!", 0, uber, 0, vict, TO_ROOM);
                            damage(uber, vict, number(1300, 1600), TYPE_UNDEFINED, DAM_PHYSICAL);
                            WAIT_STATE(vict, 2 * PULSE_VIOLENCE);
                        }
                        break;

                    case 2: /* Wing Shockwave */
                        act("$n detonates a fiery wing shockwave!", 0, uber, 0, 0, TO_ROOM);
                        for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(uber)); vict; vict = next_vict)
                        {
                            next_vict = CHAR_NEXT_IN_ROOM(vict);
                            if (IS_NPC(vict) || !IS_MORTAL(vict)) continue;
                            damage(uber, vict, number(1200, 1500), TYPE_UNDEFINED, DAM_PHYSICAL);
                            WAIT_STATE(vict, 3 * PULSE_VIOLENCE);
                        }
                        break;

                    case 3: /* Immolation Burst */
                        vict = get_random_victim_fighting(uber);
                        if (vict)
                        {
                            act("$N is engulfed in phoenix fire!", 0, uber, 0, vict, TO_ROOM);
                            damage(uber, vict, number(1600, 1900), TYPE_UNDEFINED, DAM_PHYSICAL);
                        }
                        break;

                    default: /* Heat Pulse */
                        act("Waves of intense heat roll off $n.", 0, uber, 0, 0, TO_ROOM);
                        for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(uber)); vict; vict = next_vict)
                        {
                            next_vict = CHAR_NEXT_IN_ROOM(vict);
                            if (IS_NPC(vict) || !IS_MORTAL(vict)) continue;
                            damage(uber, vict, number(900, 1100), TYPE_UNDEFINED, DAM_PHYSICAL);
                        }
                        break;
                }
            }
            break;

        case MSG_DIE:
            sprintf(buf, "%s has been slain, the realm has been saved!\n\r", GET_SHORT(uber));
            send_to_room(buf, CHAR_REAL_ROOM(uber));
			mob_aq_reward(reward, uber);
            break;
    }

    return FALSE;
}





/* ---------------------------------------------------------
 * The True Fire Newt God (Elemental Variety Spec)
 * --------------------------------------------------------- */
int ub_uber_true_fire_newt_god(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    CHAR *vict, *next_vict;
    int reward = 4;

    if (count_mortals_room(mob, TRUE) < 1)
        return FALSE;

    switch (cmd)
    {
    case MSG_MOBACT:
        if (mob->specials.fighting)
        {
            switch (number(0, 3))
            {
            case 0: /* Flame Aspect */
                vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("$n cracks open its chest, releasing a living inferno!", 0, mob, 0, 0, TO_ROOM);
                    act("You are engulfed in godfire!", 0, mob, 0, vict, TO_VICT);
                    damage(mob, vict, number(1100, 1500), TYPE_UNDEFINED, DAM_FIRE);
                }
                break;

            case 1: /* Molten Blessing (Self Buff) */
                act("$n's scales glow white-hot as molten power surges.", 0, mob, 0, 0, TO_ROOM);
                affect_apply(mob, SPELL_BLESS, 0, APPLY_HITROLL, 10, 0, 0);
                affect_apply(mob, SPELL_BLESS, 0, APPLY_DAMROLL, 10, 0, 0);
                affect_apply(mob, SPELL_BLESS, 0, APPLY_ARMOR, -30, 0, 0);
                break;

            case 2: /* Lava Burst AoE */
                act("$n slams the ground, geysers of lava erupt everywhere!", 0, mob, 0, 0, TO_ROOM);
                for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
                {
                    next_vict = CHAR_NEXT_IN_ROOM(vict);
                    if (!vict || IS_NPC(vict) || !IS_MORTAL(vict))
                        continue;
                    damage(mob, vict, number(600, 900), TYPE_UNDEFINED, DAM_FIRE);
                }
                break;

            case 3: /* Ember Curse */
                vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("$n brands $N with a smoldering rune.", 0, mob, 0, vict, TO_NOTVICT);
                    act("A burning curse eats into your flesh!", 0, mob, 0, vict, TO_VICT);
                    damage(mob, vict, number(800, 1100), TYPE_UNDEFINED, DAM_FIRE);
                    affect_apply(vict, SPELL_CURSE, 0, APPLY_ARMOR, 150, 0, 0);
                }
                break;
            }
        }
        break;

    case MSG_DIE:
        sprintf(buf, "%s explodes into molten stone and ash.", GET_SHORT(mob));
        send_to_room(buf, CHAR_REAL_ROOM(mob));
        mob_aq_reward(reward, mob);
        break;
    }
    return FALSE;
}

/* ---------------------------------------------------------
 * The Uber Blue Rook (Tactical Control RNG Spec)
 * --------------------------------------------------------- */
int ub_uber_blue_rook(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    CHAR *vict, *next_vict, *current_tank;
    int reward = 3;

    if (count_mortals_room(mob, TRUE) < 1)
        return FALSE;

    switch (cmd)
    {
    case MSG_MOBACT:
        if (mob->specials.fighting)
        {
            switch (number(0, 4))
            {
            case 0: /* Tower Crush – Heavy Single Target */
                vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("$n descends like a falling tower!", 0, mob, 0, 0, TO_ROOM);
                    act("You are crushed beneath immense weight!", 0, mob, 0, vict, TO_VICT);
                    damage(mob, vict, number(1200, 1600), TYPE_UNDEFINED, DAM_PHYSICAL);
                }
                break;

            case 1: /* Board Sweep – Room Control */
                act("$n sweeps the battlefield with a massive stone wing!", 0, mob, 0, 0, TO_ROOM);
                for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
                {
                    next_vict = CHAR_NEXT_IN_ROOM(vict);
                    if (!vict || IS_NPC(vict) || !IS_MORTAL(vict))
                        continue;
                    damage(mob, vict, number(500, 800), TYPE_UNDEFINED, DAM_PHYSICAL);
                }
                break;

            case 2: /* Pin the Piece – Hard CC */
                vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("$n pins $N against an invisible wall.", 0, mob, 0, vict, TO_NOTVICT);
                    act("You are pinned in place, unable to move!", 0, mob, 0, vict, TO_VICT);
                    stop_fighting(vict);
                    WAIT_STATE(vict, 2 * PULSE_VIOLENCE);
                }
                break;

            case 3: /* Forced Reposition – Tank Disrupt */
                current_tank = GET_OPPONENT(mob);
                act("$n shifts position with calculated precision.", 0, mob, 0, 0, TO_ROOM);
                if (current_tank)
                {
                    act("The pressure suddenly lifts as your position is broken!", 0, mob, 0, current_tank, TO_VICT);
                    stop_fighting(current_tank);
                    damage(mob, current_tank, number(400, 600), TYPE_UNDEFINED, DAM_PHYSICAL);
                    WAIT_STATE(current_tank, 1 * PULSE_VIOLENCE);
                }
                vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("$n slams into $N with renewed force!", 0, mob, 0, vict, TO_NOTVICT);
                    damage(mob, vict, number(900, 1200), TYPE_UNDEFINED, DAM_PHYSICAL);
                }
                break;

            case 4: /* Fortify Position – Self Buff */
                act("$n locks into a fortified stance, stone grinding loudly.", 0, mob, 0, 0, TO_ROOM);
                affect_apply(mob, SPELL_ARMOR, 0, APPLY_ARMOR, -50, 0, 0);
                affect_apply(mob, SPELL_ARMOR, 0, APPLY_HITROLL, 8, 0, 0);
                affect_apply(mob, SPELL_ARMOR, 0, APPLY_DAMROLL, 8, 0, 0);
                break;
            }
        }
        break;

    case MSG_DIE:
        sprintf(buf, "%s fractures and collapses into blue stone rubble.", GET_SHORT(mob));
        send_to_room(buf, CHAR_REAL_ROOM(mob));
        mob_aq_reward(reward, mob);
        break;
    }
    return FALSE;
}

/* ---------------------------------------------------------
 * The Uber King Welmar (Command & Discipline RNG Spec)
 * --------------------------------------------------------- */
int ub_uber_king_welmar(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    CHAR *vict, *next_vict, *current_tank;
    int reward = 5;

    if (count_mortals_room(mob, TRUE) < 1)
        return FALSE;

    switch (cmd)
    {
    case MSG_MOBACT:
        if (mob->specials.fighting)
        {
            switch (number(0, 4))
            {
            case 0: /* Royal Decree – Focus Fire */
                vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("$n bellows a royal decree, pointing at $N!", 0, mob, 0, vict, TO_NOTVICT);
                    act("The king's command turns all eyes toward you!", 0, mob, 0, vict, TO_VICT);
                    damage(mob, vict, number(1000, 1400), TYPE_UNDEFINED, DAM_PHYSICAL);
                }
                break;

            case 1: /* Shield Wall – Defensive Command */
                act("$n raises his blade and shouts orders to hold the line!", 0, mob, 0, 0, TO_ROOM);
                affect_apply(mob, SPELL_PROTECT_FROM_EVIL, 0, APPLY_ARMOR, -30, 0, 0);
                affect_apply(mob, SPELL_PROTECT_FROM_EVIL, 0, APPLY_HITROLL, 6, 0, 0);
                affect_apply(mob, SPELL_PROTECT_FROM_EVIL, 0, APPLY_DAMROLL, 6, 0, 0);
                break;

            case 2: /* Break Formation – Tank Pressure */
                current_tank = GET_OPPONENT(mob);
                if (current_tank)
                {
                    act("$n smashes into $N, breaking their stance!", 0, mob, 0, current_tank, TO_NOTVICT);
                    act("Your stance is shattered by the king's assault!", 0, mob, 0, current_tank, TO_VICT);
                    stop_fighting(current_tank);
                    damage(mob, current_tank, number(500, 700), TYPE_UNDEFINED, DAM_PHYSICAL);
                    WAIT_STATE(current_tank, 1 * PULSE_VIOLENCE);
                }
                break;

            case 3: /* Coordinated Strike – Room Cleave */
                act("$n executes a perfectly timed sweeping strike!", 0, mob, 0, 0, TO_ROOM);
                for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
                {
                    next_vict = CHAR_NEXT_IN_ROOM(vict);
                    if (!vict || IS_NPC(vict) || !IS_MORTAL(vict))
                        continue;
                    if (chance(60))
                        damage(mob, vict, number(600, 900), TYPE_UNDEFINED, DAM_PHYSICAL);
                }
                break;

            case 4: /* Rally the Crown – Momentum Buff */
                act("$n roars and fights harder as the battle drags on!", 0, mob, 0, 0, TO_ROOM);
                affect_apply(mob, SPELL_BLESS, 0, APPLY_HITROLL, 10, 0, 0);
                affect_apply(mob, SPELL_BLESS, 0, APPLY_DAMROLL, 10, 0, 0);
                break;
            }
        }
        break;

    case MSG_DIE:
        sprintf(buf, "%s drops his sword and falls with grim dignity.", GET_SHORT(mob));
        send_to_room(buf, CHAR_REAL_ROOM(mob));
        mob_aq_reward(reward, mob);
        break;
    }
    return FALSE;
}

/* ---------------------------------------------------------
 * The Uber King Tutankhamen (Curse & Relic RNG Spec)
 * --------------------------------------------------------- */
int ub_uber_king_tutankhamen(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    char buf[MAX_STRING_LENGTH];
    CHAR *vict, *next_vict, *current_tank;
    int reward = 6;

    if (count_mortals_room(mob, TRUE) < 1)
        return FALSE;

    switch (cmd)
    {
    case MSG_MOBACT:
        if (mob->specials.fighting)
        {
            switch (number(0, 5))
            {
            case 0: /* Pharaoh's Mark – High Curse Damage */
                vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("$n raises a golden relic, marking $N with a pharaoh's curse!", 0, mob, 0, vict, TO_NOTVICT);
                    act("A burning sigil sears itself into your soul!", 0, mob, 0, vict, TO_VICT);
                    damage(mob, vict, number(1100, 1500), TYPE_UNDEFINED, DAM_MAGICAL);
                    affect_apply(vict, SPELL_CURSE, 0, APPLY_HITROLL, -10, 0, 0);
                    affect_apply(vict, SPELL_CURSE, 0, APPLY_DAMROLL, -10, 0, 0);
                }
                break;

            case 1: /* Tomb Wrath – Room AoE */
                act("$n calls upon the wrath of the sealed tombs!", 0, mob, 0, 0, TO_ROOM);
                for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
                {
                    next_vict = CHAR_NEXT_IN_ROOM(vict);
                    if (!vict || IS_NPC(vict) || !IS_MORTAL(vict))
                        continue;
                    damage(mob, vict, number(600, 900), TYPE_UNDEFINED, DAM_MAGICAL);
                }
                break;

            case 2: /* Mummy Bind – Hard CC */
                vict = get_random_victim_fighting(mob);
                if (vict)
                {
                    act("Ancient bandages unravel and coil around $N!", 0, mob, 0, vict, TO_NOTVICT);
                    act("You are bound tightly by ancient wrappings!", 0, mob, 0, vict, TO_VICT);
                    stop_fighting(vict);
                    WAIT_STATE(vict, 2 * PULSE_VIOLENCE);
                }
                break;

            case 3: /* Relic Drain – Tank Punish */
                current_tank = GET_OPPONENT(mob);
                if (current_tank)
                {
                    act("$n presses a cursed relic into $N's chest!", 0, mob, 0, current_tank, TO_NOTVICT);
                    act("Your strength is leeched away by the relic!", 0, mob, 0, current_tank, TO_VICT);
                    damage(mob, current_tank, number(700, 1000), TYPE_UNDEFINED, DAM_MAGICAL);
                    WAIT_STATE(current_tank, 1 * PULSE_VIOLENCE);
                }
                break;

            case 4: /* Sand of Ages – Attrition */
                act("$n scatters cursed sands that grind flesh and spirit alike.", 0, mob, 0, 0, TO_ROOM);
                for (vict = ROOM_PEOPLE(CHAR_REAL_ROOM(mob)); vict; vict = next_vict)
                {
                    next_vict = CHAR_NEXT_IN_ROOM(vict);
                    if (!vict || IS_NPC(vict) || !IS_MORTAL(vict))
                        continue;
                    if (chance(50))
                        damage(mob, vict, number(500, 700), TYPE_UNDEFINED, DAM_MAGICAL);
                }
                break;

            case 5: /* Undying Pharaoh – Self Reinforcement */
                act("$n chants in an ancient tongue as necrotic energy restores him.", 0, mob, 0, 0, TO_ROOM);
                affect_apply(mob, SPELL_ARMOR, 0, APPLY_ARMOR, -30, 0, 0);
                affect_apply(mob, SPELL_BLESS, 0, APPLY_DAMROLL, 10, 0, 0);
                break;
            }
        }
        break;

    case MSG_DIE:
        sprintf(buf, "%s collapses into dust, gold, and withered bandages.", GET_SHORT(mob));
        send_to_room(buf, CHAR_REAL_ROOM(mob));
        mob_aq_reward(reward, mob);
        break;
    }
    return FALSE;
}


//Assign Spec for the zone. Sets all other specs.

void assign_ubers(void)
{
	/*Objects */
	assign_obj(UBER_THOUGHT_SLEEVES,uber_thought_sleeves);
	assign_obj(UBER_ENLIGHTENED_MANTLE,uber_enlightened_mantle);
	assign_obj(UBER_UNSHAKEN_SANDALS,uber_unshaken_sandals);
	assign_obj(UBER_INSIGHT_RING,uber_insight_ring);
	
	

	/*Rooms */
	//assign_room(LIGHTWALL,             	cl_LightWallLink);

	/*Mobs */

	assign_mob(UBER_KING_SPIDER, ub_uber_kingspider);
	assign_mob(UBER_GREAT_MYSTIC, ub_uber_greatmystic);
	assign_mob(UBER_ULT_MYSTIC, ub_uber_ultmystic);	
	assign_mob(UBER_ULT_MYSTIC_CLONE, ub_uber_ultmystic_clone);
	
	
	#ifdef TEST_SITE
	
	assign_mob(UBER_FIRE_NEWT_PHOENIX, ub_uber_firenewtphoenix);	
	assign_mob(UBER_ZYCA_PRINCE, ub_uber_zycaprince);
	assign_mob(UBER_ZYCA_GUARD, ub_uber_zyca_covert_guard);
	#endif
	
	

}