/*
		Specs for Cathedral, by Solmyr
		Written by Solmyr for RoninMUD
*/

/* Includes */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include "structs.h"
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

/* Room defines */

#define NORTHWEST_ARCADE 6702
#define SOUTHWEST_ARCADE 6704
#define HIGH_ALTAR       6705

/* Mob defines */

#define DEACON           6701
#define PRIEST           6702
#define HIGH_PRIEST      6703
#define ARCHBISHOP       6704
#define LARVANDAD        6705
#define GUSHNASAPH       6706
#define HORMISDAS        6707

/* Object defines */

#define DALMATIC_VIOLET  6701
#define CINTURE_VIOLET   6704
#define STOLE_VIOLET     6705
#define CHASUBLE_VIOLET  6706
#define DALMATIC_VIOLET_BISHOP  6707
#define CHASUBLE_VIOLET_BISHOP  6708
#define BISHOP_STAFF     6709
#define SARCOPHAGUS      6712
#define NEW_SARC_1       6713
#define NEW_SARC_2       6714
#define NEW_SARC_3       6715
#define LARVANDAD_BONES  6716
#define GUSHNASAPH_BONES 6717
#define HORMISDAS_BONES  6718
#define DALMATIC_WHITE   6721
#define CINTURE_WHITE    6722
#define STOLE_WHITE      6723
#define CHASUBLE_WHITE   6724
#define DALMATIC_WHITE_BISHOP  6725
#define CHASUBLE_WHITE_BISHOP  6726
#define DALMATIC_RED     6731
#define CINTURE_RED      6732
#define STOLE_RED        6733
#define CHASUBLE_RED     6734
#define DALMATIC_RED_BISHOP  6735
#define CHASUBLE_RED_BISHOP  6736
#define DALMATIC_BLACK   6741
#define CINTURE_BLACK    6742
#define STOLE_BLACK      6743
#define CHASUBLE_BLACK   6744
#define DALMATIC_BLACK_BISHOP  6745
#define CHASUBLE_BLACK_BISHOP  6746
#define DALMATIC_GREEN   6751
#define CINTURE_GREEN    6752
#define STOLE_GREEN      6753
#define CHASUBLE_GREEN   6754
#define DALMATIC_GREEN_BISHOP  6755
#define CHASUBLE_GREEN_BISHOP  6756

/* External structs */

struct char_data *get_random_vict(struct char_data *ch);

/* Gear functions */

void gear_change(CHAR *mob, int wear_pos, int violObj, int whiteObj, int redObj, int blackObj, int greenObj)
{
	OBJ *obj = NULL;
	bool violet = FALSE, white = FALSE, red = FALSE, black = FALSE, green = FALSE;
  if((obj = get_obj_in_list_num(real_object(violObj), mob->equipment[wear_pos])))
		violet = TRUE;
	if((obj = get_obj_in_list_num(real_object(whiteObj), mob->equipment[wear_pos])))
		white = TRUE;
	if((obj = get_obj_in_list_num(real_object(redObj), mob->equipment[wear_pos])))
		red = TRUE;
	if((obj = get_obj_in_list_num(real_object(blackObj), mob->equipment[wear_pos])))
		black = TRUE;
	if((obj = get_obj_in_list_num(real_object(greenObj), mob->equipment[wear_pos])))
		green = TRUE;
	
	if(!violet && !white && !red && !black && !green)
		return;
	
	if((((time_info.month == 10) && (time_info.day > 25)) ||
		 ((time_info.month == 11) && ((time_info.day < 7) ||
		 ((time_info.day > 7) && (time_info.day <24)))) ||
		 ((time_info.month == 1) && (time_info.day > 6)) ||
		 ((time_info.month == 2) && ((time_info.day < 19) ||
		 ((time_info.day > 19) && (time_info.day < 23)) ||
		 (time_info.day == 25)))))
	{
		if (white || red || black || green)
		{
		 extract_obj(unequip_char(mob, wear_pos));
		 obj = read_object(violObj, VIRTUAL);
		 if(obj) equip_char(mob, obj, wear_pos);
		}
		 return;
	}
	else if((((time_info.month == 11) && ((time_info.day > 23) ||
					(time_info.day == 7))) ||
					((time_info.month == 0) && (time_info.day < 13)) ||
					((time_info.month == 2) && ((time_info.day == 23) ||
					(time_info.day > 26))) || (time_info.month == 3) ||
					((time_info.month == 4) && (time_info.day < 20)) ||
					((time_info.month == 7) && (time_info.day == 14)) ||
					((time_info.month == 10) && (time_info.day == 0))))
	{
		if (violet || red || black || green)
		{
			extract_obj(unequip_char(mob, wear_pos));
			obj = read_object(whiteObj, VIRTUAL);
			if(obj) equip_char(mob, obj, wear_pos);
		}
		return;
	}
	else if((((time_info.month == 2) && ((time_info.day == 19) ||
					(time_info.day == 24) || (time_info.day == 26))) ||
					((time_info.month == 4) && (time_info.day == 20)) ||
					((time_info.month == 8) && (time_info.day == 13))))
	{
		if (violet || white || black || green)
		{
			extract_obj(unequip_char(mob, wear_pos));
			obj = read_object(redObj, VIRTUAL);
			if(obj) equip_char(mob, obj, wear_pos);
		}
		return;
	}
	else if(((time_info.month == 10) && (time_info.day == 1)))
	{
		if (violet || white || red || green)
		{
			extract_obj(unequip_char(mob, wear_pos));
			obj = read_object(blackObj, VIRTUAL);
			if(obj) equip_char(mob, obj, wear_pos);
		}
		return;
	}
	else
	{
		if (violet || white || red || black)
		{
			extract_obj(unequip_char(mob, wear_pos));
			obj = read_object(greenObj, VIRTUAL);
			if(obj) equip_char(mob, obj, wear_pos);
		}
		return;
	}	
	return;
}

/* Room specs */

/* Can't close the fence doors */
int cathedral_fence(int room, CHAR *ch, int cmd, char *arg)
{
	char buf[MAX_INPUT_LENGTH];
	
	if(!ch) return FALSE;
	if((cmd == CMD_CLOSE) && AWAKE(ch))
	{
		one_argument(arg, buf);
		if(!*buf) return FALSE;
		if(!strncmp(buf, "door", 4))
		{
			send_to_char("The door is chained and locked to the fence, preventing you from closing it", ch);
			return TRUE;
		}
	}
	return FALSE;
}

/* Specs for the high altar */
int cathedral_altar(int room, CHAR *ch, int cmd, char *arg)
{
	char buf[MAX_INPUT_LENGTH];
	CHAR *skeleton = NULL, *vict = NULL, *larvandad = NULL, *gushnasaph = NULL, *hormisdas = NULL, *next_ch = NULL;
	OBJ *sarcophagus = NULL, *loot = NULL, *new_sarc_1 = NULL, *new_sarc_2 = NULL, *new_sarc_3 = NULL, *obj = NULL;
	
	if(!ch) return FALSE;
	/* Open sarcophagus and the Three Kings appear */
	if((cmd == CMD_OPEN) && AWAKE(ch) && (sarcophagus = get_obj_in_list_num(real_object(SARCOPHAGUS), world[room].contents)))
	{
		one_argument(arg, buf);
		if(!*buf) return FALSE;
		if(!strncmp(buf, "sarcophagus", 11))
		{
			send_to_room("coffin opens and some ugly fucking skeletons appear", real_room(room));
			new_sarc_1 = read_object(NEW_SARC_1, VIRTUAL);
			obj_to_room(new_sarc_1, real_room(HIGH_ALTAR));
			new_sarc_2 = read_object(NEW_SARC_2, VIRTUAL);
			obj_to_room(new_sarc_2, real_room(HIGH_ALTAR));
			new_sarc_3 = read_object(NEW_SARC_3, VIRTUAL);
			obj_to_room(new_sarc_3, real_room(HIGH_ALTAR));

			for(loot = sarcophagus->contains; loot; loot = loot->next_content)
			{
				switch(number(1,3))
				{
					case 1:
					{
						obj_from_obj(loot);
						obj_to_obj(loot, new_sarc_1);
						break;
					}
					case 2:
					{
						obj_from_obj(loot);
						obj_to_obj(loot, new_sarc_2);
						break;
					}
					case 3:
					{
						obj_from_obj(loot);
						obj_to_obj(loot, new_sarc_3);
						break;
					}
				}
			}
			
			obj_from_room(sarcophagus);
			skeleton = read_mobile(LARVANDAD, VIRTUAL);
			char_to_room(skeleton, room);
			vict = get_random_vict(skeleton);
			if(vict)
				set_fighting(skeleton, vict);
			skeleton = read_mobile(GUSHNASAPH, VIRTUAL);
			char_to_room(skeleton, room);
			vict = get_random_vict(skeleton);
			if(vict)
				set_fighting(skeleton, vict);
			skeleton = read_mobile(HORMISDAS, VIRTUAL);
			char_to_room(skeleton, room);
			vict = get_random_vict(skeleton);
			if(vict)
				set_fighting(skeleton, vict);
		}
		return FALSE;
	}
	if(cmd != MSG_MOBACT) return FALSE;
	for(ch = world[room].people; ch; ch = next_ch)
	{
		next_ch = ch->next_in_room;
		if(IS_NPC(ch) && (V_MOB(ch) == LARVANDAD))
			larvandad = ch;
		if(IS_NPC(ch) && (V_MOB(ch) == GUSHNASAPH))
			gushnasaph = ch;
		if(IS_NPC(ch) && (V_MOB(ch) == HORMISDAS))
			hormisdas = ch;
	}
	if((!larvandad) && (!gushnasaph) && (!hormisdas)) return FALSE;
	if(chance(95)) return FALSE;
	if(world[room].contents)
	{
		for(obj = world[room].contents; obj; obj = obj->next_content)
		{
			if(V_OBJ(obj) == real_object(LARVANDAD_BONES))
			{
				send_to_room("By divine intervetion the pile of bones of a Skeletal Magi rise back up to life", room);
				larvandad = read_mobile(LARVANDAD, VIRTUAL);
				char_to_room(larvandad, room);
				vict = get_random_vict(larvandad);
				if(vict) set_fighting(larvandad, vict);
				extract_obj(obj);
			}
			if(V_OBJ(obj) == real_object(GUSHNASAPH_BONES))
			{
				send_to_room("By divine intervetion the pile of bones of a Skeletal Magi rise back up to life", room);
				gushnasaph = read_mobile(GUSHNASAPH, VIRTUAL);
				char_to_room(gushnasaph, room);
				vict = get_random_vict(gushnasaph);
				if(vict) set_fighting(gushnasaph, vict);
				extract_obj(obj);
			}
			if(V_OBJ(obj) == real_object(HORMISDAS_BONES))
			{
				send_to_room("By divine intervetion the pile of bones of a Skeletal Magi rise back up to life", room);
				hormisdas = read_mobile(HORMISDAS, VIRTUAL);
				char_to_room(hormisdas, room);
				vict = get_random_vict(hormisdas);
				if(vict) set_fighting(hormisdas, vict);
				extract_obj(obj);
			}
		}
	}
	return FALSE;
}

/* Mob specs */

/* Specs for Deacon */
int cathedral_deacon(CHAR *deacon, CHAR *ch, int cmd, char *arg)
{
	if((cmd == MSG_TICK) || (cmd == MSG_ZONE_RESET))
	{
		if(deacon->equipment[WEAR_NECK_1])
			gear_change(deacon, WEAR_NECK_1, STOLE_VIOLET, STOLE_WHITE, STOLE_RED, STOLE_BLACK, STOLE_GREEN);
		if(deacon->equipment[WEAR_ABOUT])
			gear_change(deacon, WEAR_ABOUT, DALMATIC_VIOLET, DALMATIC_WHITE, DALMATIC_RED, DALMATIC_BLACK, DALMATIC_GREEN);
		if(deacon->equipment[WEAR_WAIST])
			gear_change(deacon, WEAR_WAIST, CINTURE_VIOLET, CINTURE_WHITE, CINTURE_RED, CINTURE_BLACK, CINTURE_GREEN);
		return FALSE;
	}
	return FALSE;
}

/* Specs for Priest and High Priest */
int cathedral_priest(CHAR *priest, CHAR *ch, int cmd, char *arg)
{
	if((cmd == MSG_TICK) || (cmd == MSG_ZONE_RESET))
	{
		if(priest->equipment[WEAR_NECK_1])
			gear_change(priest, WEAR_NECK_1, STOLE_VIOLET, STOLE_WHITE, STOLE_RED, STOLE_BLACK, STOLE_GREEN);
		if(priest->equipment[WEAR_BODY])
			gear_change(priest, WEAR_BODY, CHASUBLE_VIOLET, CHASUBLE_WHITE, CHASUBLE_RED, CHASUBLE_BLACK, CHASUBLE_GREEN);
		if(priest->equipment[WEAR_WAIST])
			gear_change(priest, WEAR_WAIST, CINTURE_VIOLET, CINTURE_WHITE, CINTURE_RED, CINTURE_BLACK, CINTURE_GREEN);
		return FALSE;
	}
	return FALSE;
}

/* Specs for the Archbishop */
int cathedral_archbishop(CHAR *archbishop, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict = NULL;
	OBJ *staff = NULL;
	char buf[MAX_INPUT_LENGTH];

	if((cmd == MSG_TICK) || (cmd == MSG_ZONE_RESET))
	{
		if(archbishop->equipment[WEAR_BODY])
			gear_change(archbishop, WEAR_BODY, CHASUBLE_VIOLET_BISHOP, CHASUBLE_WHITE_BISHOP, CHASUBLE_RED_BISHOP, CHASUBLE_BLACK_BISHOP, CHASUBLE_GREEN_BISHOP);
		if(archbishop->equipment[WEAR_ABOUT])
			gear_change(archbishop, WEAR_ABOUT, DALMATIC_VIOLET_BISHOP, DALMATIC_WHITE_BISHOP, DALMATIC_RED_BISHOP, DALMATIC_BLACK_BISHOP, DALMATIC_GREEN_BISHOP);
		
		if((cmd == MSG_TICK) && chance(50) && world[CHAR_REAL_ROOM(archbishop)].people)
		{
			switch(number(0,3))
			{
				case 0:
				{
					act("You preach.",0,archbishop,0,0, TO_CHAR);
					act("The archbishop makes a solemn gesture and speaks:",1,archbishop,0,0, TO_ROOM);
					act("We as Catholic Christians do always have a reason for hope. We shall",1,archbishop,0,0, TO_ROOM);
					act("march with a straight back and head held high, self-confident and sure",1,archbishop,0,0, TO_ROOM);
					act("of victory in our modern times. For we have one God without rival, who",1,archbishop,0,0, TO_ROOM);
					act("revealed himself in Jesus Christ.",1,archbishop,0,0, TO_ROOM);
					break;
				}
				case 1:
				{
					vict = get_random_vict(archbishop);
					if(!vict || archbishop->specials.fighting) break;
					sprintf(buf, "Are you proud of who you are %s; do you follow the word of God?", GET_NAME(vict));
					do_say(archbishop, buf, CMD_SAY);
					break;
				}
				case 2:
				{
					if(!archbishop->specials.fighting) break;
					do_say(archbishop, "You will all regret this, mark my words, when the searing flames of Hell burn your flesh for all eternity!", CMD_SAY);
					break;
				}
			}
		}
		return FALSE;
	}

	/* Death or near death spec */	
	if((cmd == MSG_MOBACT) && archbishop->specials.fighting && ((vict = get_random_vict(archbishop)) && chance(15)))
	{
		act("It hurts you to do it, but $N's sacrilege cannot go unpunished, so you pray for God to smite $M.",0,archbishop,0,vict, TO_CHAR);
		act("$n bows down in prayer, and with a flashing light, the Hand of God smites $N down!",0,archbishop,0,vict, TO_NOTVICT);
		act("$n bows down in prayer, and with a flashing light, the Hand of God smites you down!",0,archbishop,0,vict, TO_VICT);
		if(chance(33) && (staff = get_obj_in_list_num(real_object(BISHOP_STAFF), ch->equipment[HOLD])))
			damage(archbishop, vict, number(500,5000), TYPE_UNDEFINED, DAM_NO_BLOCK);
		else
			damage(archbishop, vict, 2000, TYPE_UNDEFINED, DAM_NO_BLOCK);
		if((staff = get_obj_in_list_num(real_object(BISHOP_STAFF), ch->equipment[HOLD])))
		{
			act("\n\rThe affirmation of God's existence makes $n stronger.",0,archbishop,0,0, TO_ROOM);
			archbishop->points.damroll += 200;
		}
	}
	return FALSE;
}

int cathedral_larvandad(CHAR *larvandad, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict = NULL, *next_vict = NULL;
	OBJ *bones = NULL, *money = NULL, *obj = NULL;
	int i;
	
	if(cmd == MSG_CORPSE)
	{
		if(!ch) return FALSE;
		if(ch != larvandad) return FALSE;
		if(CHAR_VIRTUAL_ROOM(larvandad) != HIGH_ALTAR) return FALSE;
		if(ch->specials.fighting) stop_fighting(ch);
		death_cry(ch);
		for(vict = world[CHAR_REAL_ROOM(larvandad)].people; vict; vict = next_vict)
		{
			next_vict = vict->next_in_room;
			if(!IS_NPC(vict) && IS_MORTAL(vict) && (GET_ALIGNMENT(vict) > -1000))
			{				
				if(GET_ALIGNMENT(vict) > -800)
					GET_ALIGNMENT(vict) -= 200;
				else
					GET_ALIGNMENT(vict) = -1000;
			}
		}
		bones = read_object(LARVANDAD_BONES, VIRTUAL);
		if(GET_GOLD(ch)>0)
		{
			money = create_money(GET_GOLD(ch));
			GET_GOLD(ch) = 0;
			obj_to_room(money, CHAR_REAL_ROOM(ch));
		}
		for(i = 0; i<MAX_WEAR; i++)
			if(ch->equipment[i])
				obj_to_room(unequip_char(ch, i), CHAR_REAL_ROOM(ch));
		
		for(obj = ch->carrying; obj; obj = obj->next_content)
		{
			obj_from_char(obj);
			obj_to_room(obj, CHAR_REAL_ROOM(ch));
		}
		obj_to_room(bones, CHAR_REAL_ROOM(ch));
		send_to_room("The Skeletal Magi lets out a piercing scream as he is reduced to a pile of bones.\n\r", CHAR_REAL_ROOM(ch));
		extract_char(ch);
		return TRUE;
	}
	
	if((cmd == MSG_MOBACT) && larvandad->specials.fighting && chance(15))
	{
		act("$n kneels down in worship of the Messiah.",0,larvandad,0,0, TO_ROOM);
		act("A glowing orb appears in the hands of $n.",0,larvandad,0,0, TO_ROOM);
		for(vict = world[CHAR_REAL_ROOM(larvandad)].people; vict; vict = next_vict)
		{
			next_vict = vict->next_in_room;
			if(!IS_NPC(vict) && IS_MORTAL(vict))
			{
				send_to_char("Glowing spikes of gold shoot out from the glowing orb in $n's hands and strike everyone!", vict);
				damage(larvandad, vict, number(500,700), TYPE_UNDEFINED, DAM_NO_BLOCK);
			}
		}
	}
	return FALSE;
}

int cathedral_gushnasaph(CHAR *gushnasaph, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict = NULL, *next_vict = NULL;
	OBJ *bones = NULL, *money = NULL, *obj = NULL;
	int i;
	
	if(MSG_CORPSE)
	{
		if(!ch) return FALSE;
		if(ch != gushnasaph) return FALSE;
		if(CHAR_VIRTUAL_ROOM(gushnasaph) != HIGH_ALTAR) return FALSE;
		if(ch->specials.fighting) stop_fighting(ch);
		death_cry(ch);
		for(vict = world[CHAR_REAL_ROOM(gushnasaph)].people; vict; vict = next_vict)
		{
			next_vict = vict->next_in_room;
			if(!IS_NPC(vict) && IS_MORTAL(vict) && (GET_ALIGNMENT(vict) > -1000))
			{				
				if(GET_ALIGNMENT(vict) > -800)
					GET_ALIGNMENT(vict) -= 200;
				else
					GET_ALIGNMENT(vict) = -1000;
			}
		}
		bones = read_object(GUSHNASAPH_BONES, VIRTUAL);
		if(GET_GOLD(ch)>0)
		{
			money = create_money(GET_GOLD(ch));
			GET_GOLD(ch) = 0;
			obj_to_room(money, CHAR_REAL_ROOM(ch));
		}
		for(i = 0; i<MAX_WEAR; i++)
			if(ch->equipment[i])
				obj_to_room(unequip_char(ch, i), CHAR_REAL_ROOM(ch));
		
		for(obj = ch->carrying; obj; obj = obj->next_content)
		{
			obj_from_char(obj);
			obj_to_room(obj, CHAR_REAL_ROOM(ch));
		}
		obj_to_room(bones, CHAR_REAL_ROOM(ch));
		send_to_room("The Skeletal Magi lets out a piercing scream as he is reduced to a pile of bones.\n\r", CHAR_REAL_ROOM(ch));
		extract_char(ch);
		return TRUE;
	}
	
	if((cmd == MSG_MOBACT) && gushnasaph->specials.fighting && chance(15))
	{
		act("$n throws a small bag onto the floor that explodes and fills the area with an inpenetrable fog of dust.",0,gushnasaph,0,0, TO_ROOM);
		for(vict = world[CHAR_REAL_ROOM(gushnasaph)].people; vict; vict = next_vict)
		{
			next_vict = vict->next_in_room;
			if(!IS_NPC(vict) && IS_MORTAL(vict))
			{
				send_to_char("The dust gets into your eyes.", vict);
				damage(gushnasaph, vict, number(5,100), TYPE_UNDEFINED, DAM_NO_BLOCK);
				spell_blindness(GET_LEVEL(gushnasaph), gushnasaph, vict, 0);
			}
		}
	}
	return FALSE;
}

int cathedral_hormisdas(CHAR *hormisdas, CHAR *ch, int cmd, char *arg)
{
	OBJ *bones = NULL, *money = NULL, *obj = NULL;
	CHAR *vict = NULL, *next_vict = NULL;
	int i;
	
	if(MSG_CORPSE)
	{
		if(!ch) return FALSE;
		if(ch != hormisdas) return FALSE;
		if(CHAR_VIRTUAL_ROOM(hormisdas) != HIGH_ALTAR) return FALSE;
		if(ch->specials.fighting) stop_fighting(ch);
		death_cry(ch);
		for(vict = world[CHAR_REAL_ROOM(hormisdas)].people; vict; vict = next_vict)
		{
			next_vict = vict->next_in_room;
			if(!IS_NPC(vict) && IS_MORTAL(vict) && (GET_ALIGNMENT(vict) > -1000))
			{				
				if(GET_ALIGNMENT(vict) > -800)
					GET_ALIGNMENT(vict) -= 200;
				else
					GET_ALIGNMENT(vict) = -1000;
			}
		}
		bones = read_object(HORMISDAS_BONES, VIRTUAL);
		if(GET_GOLD(ch)>0)
		{
			money = create_money(GET_GOLD(ch));
			GET_GOLD(ch) = 0;
			obj_to_room(money, CHAR_REAL_ROOM(ch));
		}
		for(i = 0; i<MAX_WEAR; i++)
			if(ch->equipment[i])
				obj_to_room(unequip_char(ch, i), CHAR_REAL_ROOM(ch));
		
		for(obj = ch->carrying; obj; obj = obj->next_content)
		{
			obj_from_char(obj);
			obj_to_room(obj, CHAR_REAL_ROOM(ch));
		}
		obj_to_room(bones, CHAR_REAL_ROOM(ch));
		send_to_room("The Skeletal Magi lets out a piercing scream as he is reduced to a pile of bones.\n\r", CHAR_REAL_ROOM(ch));
		extract_char(ch);
		return TRUE;
	}
	
	if((cmd == MSG_MOBACT) && hormisdas->specials.fighting && chance(10) && (vict = get_random_vict(hormisdas)))
	{
		if((vict == hormisdas->specials.fighting) || IS_NPC(vict) || !IS_MORTAL(vict)) return FALSE;
		act("$n reaches $s arms towards the sky and is engulfed in brightness.",0,hormisdas,0,0, TO_ROOM);
		act("A stream of fire shoots from $n and into $N's chest, leaving $M convulsing in spasms on the ground!",0,hormisdas,0,vict, TO_NOTVICT);
		act("A stream of fire shoots from $n and strikes you in the chest, smiting you down where you convulse in pain!",0,hormisdas,0,vict, TO_VICT);
		damage(hormisdas, vict, number(1000,1500), TYPE_UNDEFINED, DAM_NO_BLOCK);
		GET_POS(vict) = POSITION_STUNNED;
		WAIT_STATE(vict, PULSE_VIOLENCE*3);
	}
	return FALSE;
}

/* Object specs */



/* Assigns */

void assign_cathedral(void)
{
	assign_room(NORTHWEST_ARCADE , cathedral_fence);
	assign_room(SOUTHWEST_ARCADE , cathedral_fence);
	assign_room(HIGH_ALTAR       , cathedral_altar);
	assign_mob(DEACON            , cathedral_deacon);
	assign_mob(PRIEST            , cathedral_priest);
	assign_mob(HIGH_PRIEST       , cathedral_priest);
	assign_mob(ARCHBISHOP        , cathedral_archbishop);
	assign_mob(LARVANDAD         , cathedral_larvandad);
	assign_mob(GUSHNASAPH        , cathedral_gushnasaph);
	assign_mob(HORMISDAS         , cathedral_hormisdas);
}
