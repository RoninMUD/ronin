/*

		Specs for Red Dragon Volcano, by Solmyr
		Written by Solmyr for RoninMUD

*/

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
#include "enchant.h"

/* Room defines */

#define PLATEAU_ROOM     17506
#define MAGMA_PIT        17533
#define ADJ_HOLE_ROOM    17509
#define HOLE_ROOM        17510
#define HOLE_REPL_ROOM   17534
#define TUNNEL_ROOM      17511
#define ADJ_TUNNEL_ROOM  17512
#define ORG_TUNNEL_ROOM  17535
#define FIREWALL_ROOM    17519
#define DRAGON_LAIRSTART 17524
#define DRAGON_LAIREND   17531
#define BEHIND_FIREWALL  17536
#define CRATER_SHELF     17532
#define AROUND_VOLCANO   17537

/* Mobile defines */

#define YOUNG_DRAGON   17500
#define CHAMBER_ADULT  17501
#define TUNNEL_ADULT   17502
#define FIREWALL_ELDER 17503
#define LAIR_ELDER     17504
#define ANCIENT_DRAGON 17505

/* Object defines */

#define DRAGON_FEMUR   17500
#define DRAGON_WINGS   17501
#define GAUNTLETS      17507

/* External structs */

void stop_riding(struct char_data *ch,struct char_data *vict);
struct char_data *get_random_vict(struct char_data *ch);

/* Room specs */

/* Being inside the magma pit hurts */
int red_dragons_pit(int room, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict = NULL, *next_vict = NULL;

	if(!world[room].people) return FALSE;
	if(cmd != MSG_MOBACT) return FALSE;
	for(vict = world[room].people; vict; vict = next_vict)
	{
		if(!vict) return FALSE;
		if(IS_MORTAL(vict) && !IS_NPC(vict))
		{
			send_to_char("The boiling magma burns you as you sink into it!\n\r", vict);
			damage(vict, vict, 700, TYPE_UNDEFINED, DAM_FIRE);
		}
	}
	return FALSE;
}

/* use DRAGON_FEMUR in the room hole */
int red_dragons_hole(int room, CHAR *ch, int cmd, char *arg)
{
	char buf[MAX_INPUT_LENGTH];
	CHAR *next_ch = NULL;
	OBJ *obj = NULL;

	if(!ch) return FALSE;

	/* use the DRAGON_FEMUR with the hole and the tunnel will cave in */

	if(cmd != CMD_USE) return FALSE;
	one_argument(arg, buf);
	if(!*buf) return FALSE;
	if(strncmp(buf, "femur", 5)) return FALSE;
	if(!(obj = get_obj_in_list_num(real_object(DRAGON_FEMUR), ch->equipment[HOLD])))
			return FALSE;


	/* Changing the exit from ADJ_HOLE_ROOM */
	world[real_room(ADJ_HOLE_ROOM)].dir_option[WEST]->to_room_r = real_room(HOLE_REPL_ROOM);
	world[real_room(ADJ_TUNNEL_ROOM)].dir_option[WEST]->to_room_r = real_room(TUNNEL_ROOM);

	/* Transfering the people in the room to TUNNEL_ROOM */
	act("The floor of the tunnel breaks apart as $n wedges $p in the hole.\n\r",0,ch,obj,0, TO_ROOM);
	act("The floor of the tunnel breaks apart as you wedge $p in the hole.\n\r",0,ch,obj,0, TO_CHAR);
	extract_obj(unequip_char(ch, HOLD));

	for(ch = world[room].people; ch; ch = next_ch)
	{
		next_ch = ch->next_in_room;
		char_from_room(ch);
		send_to_char("You fall down into the darkness.\n\r", ch);
		char_to_room(ch, real_room(TUNNEL_ROOM));
		do_look(ch, "", CMD_LOOK);
	}
	for(obj = world[room].contents; obj; obj = obj->next_content)
	{
		obj_from_room(obj);
		obj_to_room(obj, real_room(TUNNEL_ROOM));
	}
	return TRUE;
}

/* If the tunnel has caved in, change back at zone reset
	 It is also possible to jump down into the cave below,
	 but this will hurt the character											 */
int red_dragons_hole_repl(int room, CHAR *ch, int cmd, char *arg)
{
	CHAR *next_ch = NULL;
	char buf[MAX_STRING_LENGTH];
	OBJ *obj = NULL;

	if(!ch) return FALSE;
	if(cmd == CMD_JUMP)
	{
		one_argument(arg, buf);
		if(!*buf)
			return FALSE;
		if(!strncmp(buf, "floor", 5))
		{
			if(GET_POS(ch) == POSITION_RIDING)
			{
				send_to_char("Dismount first.\n\r", ch);
				return TRUE;
			}
			act("You jump through the floor into the tunnel below.\n\r",0,ch,0,0, TO_CHAR);
			act("$n jumps through the floor.",0,ch,0,0, TO_ROOM);
			char_from_room(ch);
			char_to_room(ch, real_room(TUNNEL_ROOM));
			act("$n comes tumbling down from the darkness above.",0,ch,0,0, TO_ROOM);
			act("\n\rYou almost break your neck as you plunge head first into the rocky floor of the tunnel!\n\r",0,ch,0,0, TO_CHAR);
			if(IS_MORTAL(ch))
			{
				damage(ch, ch, GET_HIT(ch)/2, TYPE_UNDEFINED, DAM_NO_BLOCK);
				do_look(ch, "", CMD_LOOK);
			}
			return TRUE;
		}
	}
	if(cmd != MSG_ZONE_RESET) return FALSE;

	if(world[real_room(ADJ_HOLE_ROOM)].dir_option[WEST]->to_room_r == room)
	{
		world[real_room(ADJ_HOLE_ROOM)].dir_option[WEST]->to_room_r = real_room(HOLE_ROOM);

		for(ch = world[room].people; ch; ch = next_ch)
		{
			next_ch = ch->next_in_room;
			char_from_room(ch);
			char_to_room(ch, real_room(HOLE_ROOM));
		}
		for(obj = world[room].contents; obj; obj = obj->next_content)
		{
			obj_from_room(obj);
			obj_to_room(obj, real_room(HOLE_ROOM));
		}
	}
	return FALSE;
}

/* If the tunnel has caved in, change back below at zone reset */
int red_dragons_tunnel_repl(int room, CHAR *ch, int cmd, char *arg)
{
	CHAR *next_ch = NULL;
	OBJ *obj = NULL;

	if(cmd != MSG_ZONE_RESET) return FALSE;

	if(world[real_room(ADJ_TUNNEL_ROOM)].dir_option[WEST]->to_room_r == room)
	{
		world[real_room(ADJ_TUNNEL_ROOM)].dir_option[WEST]->to_room_r = real_room(ORG_TUNNEL_ROOM);

		for(ch = world[room].people; ch; ch = next_ch)
		{
			next_ch = ch->next_in_room;
			char_from_room(ch);
			char_to_room(ch, real_room(ORG_TUNNEL_ROOM));
		}
		for(obj = world[room].contents; obj; obj = obj->next_content)
		{
			obj_from_room(obj);
			obj_to_room(obj, real_room(ORG_TUNNEL_ROOM));
		}
	}
	return FALSE;
}

/* If there are people in the room, FIREWALL_ELDER will appear and attack */
int red_dragons_firewall(int room, CHAR *ch, int cmd, char *arg)
{
	CHAR *dragon = NULL, *vict = NULL, *next_vict = NULL, *next_ch = NULL;

	if(!world[room].people) return FALSE;
	if(cmd != MSG_MOBACT) return FALSE;

	if(chance(30))
	{
		for(ch = world[real_room(BEHIND_FIREWALL)].people; ch; ch = next_ch)
		{
			next_ch = ch->next_in_room;
			if(IS_NPC(ch) && (V_MOB(ch) == FIREWALL_ELDER))
				dragon = ch;
		}

		if(!dragon) return FALSE;

		send_to_room("A huge red dragon comes flying over the wall of fire and descends upon you!\n\r",real_room(FIREWALL_ROOM));
		char_from_room(dragon);
		char_to_room(dragon, room);
		vict = get_random_vict(dragon);
		if(!vict) return FALSE;
		set_fighting(dragon, vict);

		for(vict = world[room].people; vict; vict = next_vict)
		{
			next_vict = vict->next_in_room;
			if(IS_MORTAL(vict) && (vict != dragon))
			{
				send_to_char("The dragon sends a stream of fire to burn you!", vict);
				if(saves_spell(vict, SAVING_SPELL, GET_LEVEL(dragon)))
					damage(dragon, vict, number(300,450), TYPE_UNDEFINED, DAM_NO_BLOCK);
				else
					damage(dragon, vict, number(500,700), TYPE_UNDEFINED, DAM_NO_BLOCK);
			}
		}
	}
	return FALSE;
}

/* If people are inside the lair of the elder red dragon
	 he might pop out and deal damage to either one or all
	 PCs in the room, before leaving again.                */
int red_dragons_lair(int room, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict = NULL, *next_vict = NULL, *dragon = NULL, *next_ch = NULL;
	int i;

	if(!world[room].people) return FALSE;
	if(cmd != MSG_MOBACT) return FALSE;

	for(i = DRAGON_LAIRSTART; (i < DRAGON_LAIREND) && !dragon; i++)
	{
		for(ch = world[real_room(i)].people; ch; ch = next_ch)
		{
			next_ch = ch->next_in_room;
			if(IS_NPC(ch) && (V_MOB(ch) == LAIR_ELDER))
				dragon = ch;
		}
	}

	if(!dragon) return FALSE;

	switch(number(1,2))
	{
		/* Fly by and strike individual character */
		case 1:
		{
			ch = world[room].people;
			if(!ch) return FALSE;
			vict = get_random_vict(ch);
			if(!vict) vict = ch; /* If only 1char in room, get_random_vict won't find one */
			if(IS_MORTAL(vict))
			{
				act("A large red dragon appears out of the darkness and attacks you, before flying off.\n\r",0,vict,0,0, TO_CHAR);
				act("A large red dragon appears out of the darkness and attacks $n, before flying off again.\n\r",0,vict,0,0, TO_NOTVICT);
				damage(vict, vict, number(800,1200), TYPE_UNDEFINED, DAM_NO_BLOCK);
			}
			break;
		}

		case 2:
		{
			/* Fly by and spec everyone in the room */
			for(vict = world[room].people; vict; vict = next_vict)
			{
				next_vict = vict->next_in_room;
				if(!vict) break;
				if(IS_MORTAL(vict))
				{
					send_to_char("The sound of flapping wings breaks the silence.\n\r", vict);
					send_to_char("A large red dragon appears out of the darkness!\n\r", vict);
					send_to_char("The dragon opens its mouth and a sea of flames\n\r", vict);
					send_to_char("fall on you as the dragon glides over your head\n\r", vict);
					send_to_char("and back into the shadows of its lair.\n\r", vict);
					if(saves_spell(vict, SAVING_SPELL, GET_LEVEL(dragon)))
						damage(vict, vict, number(300,450), TYPE_UNDEFINED, DAM_NO_BLOCK);
					else
						damage(vict, vict, number(500,700), TYPE_UNDEFINED, DAM_NO_BLOCK);
				}
			}
			break;
		}
	}
	return FALSE;
}

/* After the people have been flown to this room, the dragon will gradually appear and attack */
int red_dragons_shelf(int room, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict = NULL, *next_vict = NULL, *dragon = NULL, *next_ch = NULL;
	OBJ *obj = NULL;
	static int dragontimer = 0;

	/* To be nice, PC corpses fall into the volcano below so they won't be as hard to retrieve */
	if(cmd == MSG_TICK)
	{
		if(world[room].contents)
		{
			for(obj = world[room].contents; obj; obj = obj->next_content)
			{
				if(obj->obj_flags.cost == PC_CORPSE)
				{
					send_to_room("\n\rA gust of wind pushes something over the edge of the shelf and into the volcano.\n\r", room);
					obj_from_room(obj);
					log_f("Red Dragons Log: %s is removed from 17532", OBJ_SHORT(obj));
					obj_to_room(obj, real_room(PLATEAU_ROOM));
					log_f("Red Dragons Log: %s is laid in 17506", OBJ_SHORT(obj));
					return FALSE;  /* Only one corpse per tick */
				}
			}
			return FALSE;
		}
		return FALSE;
	}

	/* Acts to room at certain times, then mob appearance and attack */
	if(!world[room].people) return FALSE;
	if(cmd != MSG_MOBACT) return FALSE;

	for(ch = world[real_room(AROUND_VOLCANO)].people; ch; ch = next_ch)
	{
		next_ch = ch->next_in_room;
		if(IS_NPC(ch) && (V_MOB(ch) == ANCIENT_DRAGON))
			dragon = ch;
	}
	if(!dragon) return FALSE;

	switch(dragontimer)
	{
		case 0:
		{
			send_to_room("A shadow rises on the other side of the volcano, ascending high into the sky.",real_room(CRATER_SHELF));
			dragontimer = 1;
			break;
		}
		case 1:
		{
			send_to_room("The distant shadow moves closer and engages in a sharp plunge towards you.",real_room(CRATER_SHELF));
			dragontimer = 2;
			break;
		}
		case 2:
		{
			char_from_room(dragon);
			char_to_room(dragon, room);
			vict = get_random_vict(dragon);
			if(!vict) break;
			set_fighting(dragon, vict);

			for(vict = world[room].people; vict; vict = next_vict)
			{
				next_vict = vict->next_in_room;
				if(!vict) break;
				if(IS_MORTAL(vict))
				{
					send_to_char("With a tremendous roar, an enormous dragon descends upon you, showering you in flames!\n\r", vict);
					damage(dragon, vict, 1000, TYPE_UNDEFINED, DAM_NO_BLOCK);
				}
			}
			dragontimer = 0;
			break;
		}
	}
	return FALSE;
}

/* Mobile specs */

int red_dragons_young(CHAR *dragon, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict = NULL;

	if(!dragon->specials.fighting) return FALSE;
	if(cmd != MSG_MOBACT) return FALSE;

	if((1*GET_MAX_HIT(dragon)/5) >= GET_HIT(dragon))
	{
		if(!IS_SET(dragon->specials.act, ACT_SHIELD)) SET_BIT(dragon->specials.act, ACT_SHIELD);
	}
	else
	{
		if(IS_SET(dragon->specials.act, ACT_SHIELD)) REMOVE_BIT(dragon->specials.act, ACT_SHIELD);
	}

	switch(number(1,2))
	{
		case 1: /* Claw attack, chance of dual */
		{
			if(chance(50))
			{
				vict = get_random_victim_fighting(dragon);
				if(!vict) break;
				act("You strike $N with your claws",1,dragon,0,vict, TO_CHAR);
				act("$n strikes you with $s massive claws!",1,dragon,0,vict, TO_VICT);
				act("$n strikes $N with $s massive claws!",1,dragon,0,vict, TO_NOTVICT);
				damage(dragon, vict, number(150,200), TYPE_UNDEFINED, DAM_NO_BLOCK);
			}
			else
			{
				vict = get_random_victim_fighting(dragon);
				if(!vict) break;
				act("You strike $N with your claws",1,dragon,0,vict, TO_CHAR);
				act("$n strikes you with $s massive claws!",1,dragon,0,vict, TO_VICT);
				act("$n strikes $N with $s massive claws!",1,dragon,0,vict, TO_NOTVICT);
				act("You strike $N with your claws",1,dragon,0,vict, TO_CHAR);
				act("$n strikes you with $s massive claws!",1,dragon,0,vict, TO_VICT);
				act("$n strikes $N with $s massive claws!",1,dragon,0,vict, TO_NOTVICT);
				damage(dragon, vict, number(250,350), TYPE_UNDEFINED, DAM_NO_BLOCK);
			}
			break;
		}

		case 2: /* bite attack */
		{
			vict = get_random_victim_fighting(dragon);
			if(!vict) break;
			act("You bite $N.",1,dragon,0,vict, TO_CHAR);
			act("$n stretches out his neck and bites you!",1,dragon,0,vict, TO_VICT);
			act("$n stretches out his neck and bites $N!",1,dragon,0,vict, TO_NOTVICT);
			damage(dragon, vict, number(300,400), TYPE_UNDEFINED, DAM_NO_BLOCK);
			break;
		}
	}
	return FALSE;
}

int red_dragons_chamber_adult(CHAR *dragon, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict = NULL, *next_vict = NULL;
	struct descriptor_data *d = NULL;

	if(!dragon->specials.fighting) return FALSE;
	if(cmd != MSG_MOBACT) return FALSE;

	if((2*GET_MAX_HIT(dragon)/5) >= GET_HIT(dragon))
	{
		if(!IS_SET(dragon->specials.act, ACT_SHIELD)) SET_BIT(dragon->specials.act, ACT_SHIELD);
	}
	else
	{
		if(IS_SET(dragon->specials.act, ACT_SHIELD)) REMOVE_BIT(dragon->specials.act, ACT_SHIELD);
	}

	switch(number(1,4))
	{
		case 1: /* Claw attack, chance of dual */
		{
			vict = get_random_victim_fighting(dragon);
			if(!vict) break;
			if(chance(50))
			{
				act("You strike $N with your claws",1,dragon,0,vict, TO_CHAR);
				act("$n strikes you with $s massive claws!",1,dragon,0,vict, TO_VICT);
				act("$n strikes $N with $s massive claws!",1,dragon,0,vict, TO_NOTVICT);
				damage(dragon, vict, number(200,300), TYPE_UNDEFINED, DAM_NO_BLOCK);
			}
			else
			{
				vict = get_random_victim_fighting(dragon);
				if(!vict) break;
				act("You strike $N with your claws",1,dragon,0,vict, TO_CHAR);
				act("$n strikes you with $s massive claws!",1,dragon,0,vict, TO_VICT);
				act("$n strikes $N with $s massive claws!",1,dragon,0,vict, TO_NOTVICT);
				act("You strike $N with your claws",1,dragon,0,vict, TO_CHAR);
				act("$n strikes you with $s massive claws!",1,dragon,0,vict, TO_VICT);
				act("$n strikes $N with $s massive claws!",1,dragon,0,vict, TO_NOTVICT);
				damage(dragon, vict, number(350,500), TYPE_UNDEFINED, DAM_NO_BLOCK);
			}
			break;
		}

		case 2: /* Bite attack */
		{
			vict = get_random_victim_fighting(dragon);
			if(!vict) break;
			act("You bite $N.",1,dragon,0,vict, TO_CHAR);
			act("$n stretches out his neck and bites you!",1,dragon,0,vict, TO_VICT);
			act("$n stretches out his neck and bites $N!",1,dragon,0,vict, TO_NOTVICT);
			damage(dragon, vict, number(400,500), TYPE_UNDEFINED, DAM_NO_BLOCK);
			break;
		}

		case 3: /* Breath weapon */
		{
			for(vict = world[CHAR_REAL_ROOM(dragon)].people; vict; vict = next_vict)
			{
				next_vict = vict->next_in_room;
				if(IS_MORTAL(vict))
				{
					act("$n opens $s mouth and expels a cone of fire, bathing the surroundings in flames!",1,dragon,0,vict, TO_VICT);
					if(saves_spell(vict, SAVING_SPELL, GET_LEVEL(dragon)))
						damage(dragon, vict, number(200,300), TYPE_UNDEFINED, DAM_NO_BLOCK);
					else
						damage(dragon, vict, number(300,400), TYPE_UNDEFINED, DAM_NO_BLOCK);
				}
			}
			break;
		}
		case 4: /* Throw people into the magma pit */
		{
			vict = get_random_victim_fighting(dragon);
			if(!vict) break;
			if((CHAR_REAL_ROOM(vict) != CHAR_REAL_ROOM(dragon)) || (CHAR_REAL_ROOM(dragon) != real_room(PLATEAU_ROOM)))
				break;
			d = vict->desc;

			act("With a swift motion of the tail, $n sends you tumbling into the pit!", 1, dragon, 0, vict, TO_VICT);
			act("With a swift motion of the tail, $n sends $N tumbling into the pit!", 1, dragon, 0, vict, TO_NOTVICT);
			act("You push $N into the pit.", 1, dragon, 0, vict, TO_CHAR);
			damage(dragon, vict, 300, TYPE_UNDEFINED, DAM_NO_BLOCK);

			if(!d)
				break;
			if(d->connected != CON_PLYNG)
				break;
			if(vict->specials.riding)
				stop_riding(vict, vict->specials.riding);

			char_from_room(vict);
			char_to_room(vict, real_room(MAGMA_PIT));
			do_look(vict, "", CMD_LOOK);
			WAIT_STATE(vict, number(2,4)*PULSE_VIOLENCE);
			break;
		}
	}
	return FALSE;
}

int red_dragons_tunnel_adult(CHAR *dragon, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict = NULL, *next_vict = NULL;

	if(!dragon->specials.fighting) return FALSE;
	if(cmd != MSG_MOBACT) return FALSE;

	if((2*GET_MAX_HIT(dragon)/5) >= GET_HIT(dragon))
	{
		if(!IS_SET(dragon->specials.act, ACT_SHIELD)) SET_BIT(dragon->specials.act, ACT_SHIELD);
	}
	else
	{
		if(IS_SET(dragon->specials.act, ACT_SHIELD)) REMOVE_BIT(dragon->specials.act, ACT_SHIELD);
	}

	switch(number(1,3))
	{
		case 1: /* Claw attack, chance of dual */
		{
			vict = get_random_victim_fighting(dragon);
			if(!vict) break;
			if(chance(50))
			{
				act("You strike $N with your claws",1,dragon,0,vict, TO_CHAR);
				act("$n strikes you with $s massive claws!",1,dragon,0,vict, TO_VICT);
				act("$n strikes $N with $s massive claws!",1,dragon,0,vict, TO_NOTVICT);
				damage(dragon, vict, number(300,400), TYPE_UNDEFINED, DAM_NO_BLOCK);
			}
			else
			{
				vict = get_random_victim_fighting(dragon);
				if(!vict) break;
				act("You strike $N with your claws",1,dragon,0,vict, TO_CHAR);
				act("$n strikes you with $s massive claws!",1,dragon,0,vict, TO_VICT);
				act("$n strikes $N with $s massive claws!",1,dragon,0,vict, TO_NOTVICT);
				act("You strike $N with your claws",1,dragon,0,vict, TO_CHAR);
				act("$n strikes you with $s massive claws!",1,dragon,0,vict, TO_VICT);
				act("$n strikes $N with $s massive claws!",1,dragon,0,vict, TO_NOTVICT);
				damage(dragon, vict, number(400,600), TYPE_UNDEFINED, DAM_NO_BLOCK);
			}
			break;
		}
		case 2: /* Bite attack */
		{
			vict = get_random_victim_fighting(dragon);
			if(!vict) break;
			act("You bite $N.",1,dragon,0,vict, TO_CHAR);
			act("$n stretches out his neck and bites you!",1,dragon,0,vict, TO_VICT);
			act("$n stretches out his neck and bites $N!",1,dragon,0,vict, TO_NOTVICT);
			damage(dragon, vict, number(500,600), TYPE_UNDEFINED, DAM_NO_BLOCK);
			break;
		}
		case 3: /* Breath weapon */
		{
			for(vict = world[CHAR_REAL_ROOM(dragon)].people; vict; vict = next_vict)
			{
				next_vict = vict->next_in_room;
				if(IS_MORTAL(vict))
				{
					act("$n opens $s mouth and expels a cone of fire, bathing the surroundings in flames!",1,dragon,0,vict, TO_VICT);
					if(saves_spell(vict, SAVING_SPELL, GET_LEVEL(dragon)))
						damage(dragon, vict, number(300,400), TYPE_UNDEFINED, DAM_NO_BLOCK);
					else
						damage(dragon, vict, number(425,550), TYPE_UNDEFINED, DAM_NO_BLOCK);
				}
			}
			break;
		}
	}
	return FALSE;
}


int red_dragons_firewall_elder(CHAR *dragon, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict = NULL, *next_vict = NULL;

	if(cmd != MSG_MOBACT) return FALSE;
	if(!dragon->specials.fighting && (world[CHAR_REAL_ROOM(dragon)].number == real_room(FIREWALL_ROOM)))
	{
		char_from_room(dragon);
		char_to_room(dragon, real_room(BEHIND_FIREWALL));
	}

	if((GET_MAX_HIT(dragon)/2) >= GET_HIT(dragon))
	{
		if(!IS_SET(dragon->specials.act, ACT_SHIELD)) SET_BIT(dragon->specials.act, ACT_SHIELD);
	}
	else
	{
		if(IS_SET(dragon->specials.act, ACT_SHIELD)) REMOVE_BIT(dragon->specials.act, ACT_SHIELD);
	}

	switch(number(1,4))
	{
		case 1: /* Claw attack, chance of dual */
		{
			vict = get_random_victim_fighting(dragon);
			if(!vict) break;
			if(chance(50))
			{
				act("You strike $N with your claws",1,dragon,0,vict, TO_CHAR);
				act("$n strikes you with $s massive claws!",1,dragon,0,vict, TO_VICT);
				act("$n strikes $N with $s massive claws!",1,dragon,0,vict, TO_NOTVICT);
				damage(dragon, vict, number(400,550), TYPE_UNDEFINED, DAM_NO_BLOCK);
			}
			else
			{
				vict = get_random_victim_fighting(dragon);
				if(!vict) break;
				act("You strike $N with your claws",1,dragon,0,vict, TO_CHAR);
				act("$n strikes you with $s massive claws!",1,dragon,0,vict, TO_VICT);
				act("$n strikes $N with $s massive claws!",1,dragon,0,vict, TO_NOTVICT);
				act("You strike $N with your claws",1,dragon,0,vict, TO_CHAR);
				act("$n strikes you with $s massive claws!",1,dragon,0,vict, TO_VICT);
				act("$n strikes $N with $s massive claws!",1,dragon,0,vict, TO_NOTVICT);
				damage(dragon, vict, number(600,800), TYPE_UNDEFINED, DAM_NO_BLOCK);
			}
			break;
		}
		case 2: /* Bite attack */
		{
			vict = get_random_victim_fighting(dragon);
			if(!vict) break;
			act("You bite $N.",1,dragon,0,vict, TO_CHAR);
			act("$n stretches out his neck and bites you!",1,dragon,0,vict, TO_VICT);
			act("$n stretches out his neck and bites $N!",1,dragon,0,vict, TO_NOTVICT);
			damage(dragon, vict, number(700,900), TYPE_UNDEFINED, DAM_NO_BLOCK);
			break;
		}
		case 3: /* Breath weapon */
		{
			for(vict = world[CHAR_REAL_ROOM(dragon)].people; vict; vict = next_vict)
			{
				next_vict = vict->next_in_room;
				if(IS_MORTAL(vict))
				{
					/* If less than 50% hps, the spec is harder */
					if(GET_HIT(dragon) < GET_MAX_HIT(dragon)/2)
					{
						act("$n lets out a loud roar, followed by a massive stream of crimson fire!",1,dragon,0,vict, TO_CHAR);				
						if(saves_spell(vict, SAVING_SPELL, GET_LEVEL(dragon)))
							damage(dragon, vict, number(550,700), TYPE_UNDEFINED, DAM_NO_BLOCK);
						else
							damage(dragon, vict, number(700,1000), TYPE_UNDEFINED, DAM_NO_BLOCK);
					}
					else
					{
						act("$n opens $s mouth and expels a cone of fire, bathing the surroundings in flames!",1,dragon,0,vict, TO_VICT);
						if(saves_spell(vict, SAVING_SPELL, GET_LEVEL(dragon)))
							damage(dragon, vict, number(400,500), TYPE_UNDEFINED, DAM_NO_BLOCK);
						else
							damage(dragon, vict, number(600,700), TYPE_UNDEFINED, DAM_NO_BLOCK);
					}
				}
			}
			break;
		}
		case 4: /* Tail strike */
		{
			vict = get_random_victim_fighting(dragon);
			if(!vict) break;
			act("You tail strike $N.",1,dragon,0,vict, TO_CHAR);
			act("$n's massive tail swings through the air and strikes you!",1,dragon,0,vict, TO_VICT);
			act("$n's massive tail swings through the air and strikes $N!",1,dragon,0,vict, TO_NOTVICT);
			damage(dragon, vict, number(1000,1350), TYPE_UNDEFINED, DAM_NO_BLOCK);
			break;
		}

	}
	return FALSE;
}

int red_dragons_lair_elder(CHAR *dragon, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict = NULL, *next_vict = NULL;
	int retreat_room = 0;

	if(!dragon->specials.fighting) return FALSE;
	if(cmd != MSG_MOBACT) return FALSE;

  /*
	if((GET_MAX_HIT(dragon)/2) >= GET_HIT(dragon))
	{
		if(!IS_SET(dragon->specials.act, ACT_SHIELD)) SET_BIT(dragon->specials.act, ACT_SHIELD);
	}
	else
	{
		if(IS_SET(dragon->specials.act, ACT_SHIELD)) REMOVE_BIT(dragon->specials.act, ACT_SHIELD);
	}
  */


	if(chance(10))
	{
		switch(number(24,31))
		{
			case 24:
				retreat_room = 17524;
				break;
			case 25:
				retreat_room = 17525;
				break;
			case 26:
				retreat_room = 17526;
				break;
			case 27:
				retreat_room = 17527;
				break;
			case 28:
				retreat_room = 17528;
				break;
			case 29:
				retreat_room = 17529;
				break;
			case 30:
				retreat_room = 17530;
				break;
			case 31:
				retreat_room = 17531;
				break;
		}
		act("You fly off and leave the fight.",0,dragon,0,0, TO_CHAR);
		act("$n spreads $s giant wings and flies off into the darkness.",0,dragon,0,0, TO_ROOM);
		stop_fighting(dragon);
		char_from_room(dragon);
		char_to_room(dragon, real_room(retreat_room));
	}

	if(!dragon->specials.fighting) return FALSE;
	switch(number(1,5))
	{
		case 1: /* Claw attack, chance of dual */
    case 2:
		{
			vict = get_random_victim_fighting(dragon);
			if(!vict) break;
			if(chance(50))
			{
				act("You strike $N with your claws",1,dragon,0,vict, TO_CHAR);
				act("$n strikes you with $s massive claws!",1,dragon,0,vict, TO_VICT);
				act("$n strikes $N with $s massive claws!",1,dragon,0,vict, TO_NOTVICT);
				damage(dragon, vict, number(350,450), TYPE_UNDEFINED, DAM_NO_BLOCK);
			}
			else
			{
				act("You strike $N with your claws",1,dragon,0,vict, TO_CHAR);
				act("$n strikes you with $s massive claws!",1,dragon,0,vict, TO_VICT);
				act("$n strikes $N with $s massive claws!",1,dragon,0,vict, TO_NOTVICT);
				act("You strike $N with your claws",1,dragon,0,vict, TO_CHAR);
				act("$n strikes you with $s massive claws!",1,dragon,0,vict, TO_VICT);
				act("$n strikes $N with $s massive claws!",1,dragon,0,vict, TO_NOTVICT);
				damage(dragon, vict, number(500,750), TYPE_UNDEFINED, DAM_NO_BLOCK);
			}
			break;
		}
		case 3: /* Bite attack */
		{
			vict = get_random_victim_fighting(dragon);
			if(!vict) break;
			act("You bite $N.",1,dragon,0,vict, TO_CHAR);
			act("$n stretches out his neck and bites you!",1,dragon,0,vict, TO_VICT);
			act("$n stretches out his neck and bites $N!",1,dragon,0,vict, TO_NOTVICT);
			damage(dragon, vict, number(600,750), TYPE_UNDEFINED, DAM_NO_BLOCK);
			break;
		}
		case 4: /* Breath weapon */
		{
			for(vict = world[CHAR_REAL_ROOM(dragon)].people; vict; vict = next_vict)
			{
				next_vict = vict->next_in_room;
				if(IS_MORTAL(vict))
				{
					/* More damage if dragon is below half hps */
					if(GET_HIT(dragon) < GET_MAX_HIT(dragon)/2)
					{
						act("$n lets out a loud roar, followed by a massive stream of crimson fire!",1,dragon,0,vict, TO_VICT);
						if(saves_spell(vict, SAVING_SPELL, GET_LEVEL(dragon)))
							damage(dragon, vict, number(450,600), TYPE_UNDEFINED, DAM_NO_BLOCK);
						else
							damage(dragon, vict, number(666,800), TYPE_UNDEFINED, DAM_NO_BLOCK);
					}
					else
					{
						act("$n opens $s mouth and expels a cone of fire, bathing the surroundings in flames!",1,dragon,0,vict, TO_VICT);
						if(saves_spell(vict, SAVING_SPELL, GET_LEVEL(dragon)))
							damage(dragon, vict, number(300,450), TYPE_UNDEFINED, DAM_NO_BLOCK);
						else
							damage(dragon, vict, number(500,666), TYPE_UNDEFINED, DAM_NO_BLOCK);
					}
				}
			}
			break;
		}
		case 5: /* Tail strike */
		{
			vict = get_random_victim_fighting(dragon);
			if(!vict) break;
			act("You tail strike $N.",1,dragon,0,vict, TO_CHAR);
			act("$n's massive tail swings through the air and strikes you!",1,dragon,0,vict, TO_VICT);
			act("$n's massive tail swings through the air and strikes $N!",1,dragon,0,vict, TO_NOTVICT);
			damage(dragon, vict, number(800,1000), TYPE_UNDEFINED, DAM_NO_BLOCK);
			break;
		}
	}
	return FALSE;
}

int frightful_presence(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg);

int red_dragons_ancient(CHAR *dragon, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict = NULL, *next_vict = NULL;
	ENCH *tmp_enchantment = NULL;

	if((cmd == MSG_ZONE_RESET) && !dragon->specials.fighting)
	{
		char_from_room(dragon);
		char_to_room(dragon, real_room(AROUND_VOLCANO));
	}

	if(cmd == MSG_TICK)
	{
		if (chance(50))
		{
			/* Frightful presence */
			act("$n flaps its wings and rises over you, showing $s awesome majesty!",1,dragon,0,0, TO_ROOM);
			for(vict = world[CHAR_REAL_ROOM(dragon)].people; vict; vict = next_vict)
			{
				next_vict = vict->next_in_room;
				if(IS_MORTAL(vict))
				{
					if(saves_spell(vict, SAVING_SPELL, GET_LEVEL(dragon)))
						continue;
					else
					{
						if(!enchanted_by(vict, "Frightful Presence"))
						{
        			CREATE(tmp_enchantment, ENCH, 1);
        			tmp_enchantment->name     = str_dup("Frightful Presence");
        			tmp_enchantment->duration = 10;
        			tmp_enchantment->func     = frightful_presence;
        			enchantment_to_char(vict, tmp_enchantment, FALSE);
							send_to_char("You are frightened.\n\r", vict);
						}
					}
				}
			}
		}
	}

	if(cmd == MSG_MOBACT)
	{
		if((3*GET_MAX_HIT(dragon)/5) >= GET_HIT(dragon))
		{
			if(!IS_SET(dragon->specials.act, ACT_SHIELD)) SET_BIT(dragon->specials.act, ACT_SHIELD);
		}
		else
		{
			if(IS_SET(dragon->specials.act, ACT_SHIELD)) REMOVE_BIT(dragon->specials.act, ACT_SHIELD);
		}

		switch(number(1,2))
		{
			case 1: /* Breath weapon */
			{
				for(vict = world[CHAR_REAL_ROOM(dragon)].people; vict; vict = next_vict)
				{
					next_vict = vict->next_in_room;
					if(IS_MORTAL(vict))
					{
						/* Half damage if dragon is below 50% hps */
						if(GET_HIT(dragon) < GET_MAX_HIT(dragon)/2)
						{
							act("$n opens $s mouth and expels a cone of fire, bathing the surroundings in flames!",1,dragon,0,vict, TO_VICT);
							if(saves_spell(vict, SAVING_SPELL, GET_LEVEL(dragon)))
								damage(dragon, vict, number(500,666), TYPE_UNDEFINED, DAM_NO_BLOCK);
							else
								damage(dragon, vict, number(666,999), TYPE_UNDEFINED, DAM_NO_BLOCK);
						}
						else
						{
							act("$n lets out a loud roar, followed by a massive stream of crimson fire!",1,dragon,0,vict, TO_VICT);
							if(saves_spell(vict, SAVING_SPELL, GET_LEVEL(dragon)))
								damage(dragon, vict, number(333,444), TYPE_UNDEFINED, DAM_NO_BLOCK);
							else
								damage(dragon, vict, number(500,666), TYPE_UNDEFINED, DAM_NO_BLOCK);
						}
					}
				}
				break;
			}
			case 2: /* Tail strike */
			{
				vict = get_random_victim_fighting(dragon);
				if(!vict) break;
				act("You tail strike $N.",1,dragon,0,vict, TO_CHAR);
				act("$n's massive tail swings through the air and strikes you!",1,dragon,0,vict, TO_VICT);
				act("$n's massive tail swings through the air and strikes $N!",1,dragon,0,vict, TO_NOTVICT);

				if(chance(70))
					damage(dragon, vict, number(500, 1000), TYPE_UNDEFINED, DAM_NO_BLOCK);
				else
				{
					if((GET_HIT(vict)<750) && (GET_HIT(vict)>250) && (vict != dragon->specials.fighting))
						GET_HIT(vict) = number(1, 111);
					else
						damage(dragon, vict, 2000, TYPE_UNDEFINED, DAM_NO_BLOCK);
					WAIT_STATE(vict, PULSE_VIOLENCE*2);
				}
				break;
			}
		}
	}

	if((cmd == MSG_VIOLENCE) && chance(50))
	{
		switch(number(1,3))
		{
			case 1: /* Claw attack, chance of dual */
			{
				vict = get_random_victim_fighting(dragon);
				if(!vict) break;
				if(chance(50))
				{
					act("You strike $N with your claws",1,dragon,0,vict, TO_CHAR);
					act("$n strikes you with $s massive claws!",1,dragon,0,vict, TO_VICT);
					act("$n strikes $N with $s massive claws!",1,dragon,0,vict, TO_NOTVICT);
					damage(dragon, vict, number(333,444), TYPE_UNDEFINED, DAM_NO_BLOCK);
				}
				else
				{
					act("You strike $N with your claws",1,dragon,0,vict, TO_CHAR);
					act("$n strikes you with $s massive claws!",1,dragon,0,vict, TO_VICT);
					act("$n strikes $N with $s massive claws!",1,dragon,0,vict, TO_NOTVICT);
					act("You strike $N with your claws",1,dragon,0,vict, TO_CHAR);
					act("$n strikes you with $s massive claws!",1,dragon,0,vict, TO_VICT);
					act("$n strikes $N with $s massive claws!",1,dragon,0,vict, TO_NOTVICT);
					damage(dragon, vict, number(475,575), TYPE_UNDEFINED, DAM_NO_BLOCK);
				}
				break;
			}
			case 2: /* Bite attack */
			{
				vict = get_random_victim_fighting(dragon);
				if(!vict) break;
				act("You bite $N.",1,dragon,0,vict, TO_CHAR);
				act("$n stretches out his neck and bites you!",1,dragon,0,vict, TO_VICT);
				act("$n stretches out his neck and bites $N!",1,dragon,0,vict, TO_NOTVICT);
				damage(dragon, vict, number(555,666), TYPE_UNDEFINED, DAM_NO_BLOCK);
				break;
			}
			case 3: /* Tail sweep */
			{
				act("In a sudden motion, $n swings $s tail in a massive sweep!",1,dragon,0,0, TO_ROOM);
				for(vict = world[CHAR_REAL_ROOM(dragon)].people; vict; vict = next_vict)
				{
					next_vict = vict->next_in_room;
					if(IS_MORTAL(vict))
					{
						act("$n's tail sweep knocks you off your feet!",1,dragon,0,vict, TO_VICT);
						GET_POS(ch) = POSITION_SITTING;
						damage(dragon, vict, number(100,500), TYPE_UNDEFINED, DAM_NO_BLOCK);
						WAIT_STATE(vict, number(2,5) RL_SEC);
					}
				}
				break;
			}
		}
	}

	return FALSE;
}

/* Object specs */

int red_dragons_wings(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
	char buf[MAX_INPUT_LENGTH];
	CHAR *ch2 = NULL, *next_char = NULL;

	ch = obj->equipped_by;
	if(!ch) return FALSE;
	if(cmd == CMD_USE && obj == EQ(ch,HOLD))
	{
		if(CHAR_VIRTUAL_ROOM(ch) != PLATEAU_ROOM) return FALSE;
		one_argument(arg, buf);
		if(strncmp(buf, "wings", 5)) return FALSE;

		act("You stretch out $p and take off through the hole in the ceiling.",0,ch,obj,0, TO_CHAR);
		act("$n stretches out $p and takes off through the hole in the ceiling.\n\r",0,ch,obj,0, TO_ROOM);

		for(ch2 = world[real_room(PLATEAU_ROOM)].people; ch2; ch2 = next_char)
		{
			next_char = ch2->next_in_room;
			send_to_char("The dragon wings take you on a trip through the hole in the ceiling.\n\r", ch2);
			send_to_char("You exit the hole and find yourself flying up the crater of a volcano\n\r", ch2);
			send_to_char("towards a ledge on the eastern wall.\n\r\n\r", ch2);
			char_from_room(ch2);
			char_to_room(ch2, real_room(CRATER_SHELF));
			do_look(ch2, "", CMD_LOOK);
		}

		send_to_char("\n\r\n\rHaving flown up to the volcano ledge, the heavy wings fall out\n\r", ch);
		send_to_char("of your reach and tumble into the volcano crater.\n\r", ch);
		extract_obj(unequip_char(ch, HOLD));
		return TRUE;
	}
	return FALSE;
}

int red_dragons_gauntlets(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
	CHAR *fodder = NULL, *next_ch = NULL;
	int amt = 0, totalamt = 0;

	if((cmd == MSG_TICK) && chance(7) && obj->equipped_by)
	{
		ch = obj->equipped_by;
		for(fodder = world[CHAR_REAL_ROOM(ch)].people; fodder; fodder = next_ch)
		{
			next_ch = fodder->next_in_room;
			if((fodder != ch) && IS_MORTAL(fodder) && !IS_NPC(fodder))
			{
				amt = GET_MANA(fodder)/100;
				totalamt += amt;
				GET_MANA(fodder) = GET_MANA(fodder) - amt;
			}
		}
		act("$n's $p glow for a fraction of a second.",0,ch,obj,0, TO_ROOM);
		act("Your $p glow for a fraction of a second.",0,ch,obj,0, TO_CHAR);
		if((GET_MANA(ch) + totalamt) > GET_MAX_MANA(ch))
			GET_MANA(ch) = GET_MAX_MANA(ch);
		else
			GET_MANA(ch) += totalamt;
	}
	return FALSE;
}

/* Enchantments */

int frightful_presence(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char *arg)
{
	CHAR *vict = NULL, *next_vict = NULL;
	int dragon = FALSE;

	if(cmd == MSG_REMOVE_ENCH)
	{
		send_to_char("You do not feel as intimidated.\n\r", ench_ch);
	}

	if(cmd == MSG_TICK)
	{
		for(vict = world[CHAR_REAL_ROOM(ench_ch)].people; vict; vict = next_vict)
		{
			next_vict = vict->next_in_room;

			if(V_MOB(vict) == ANCIENT_DRAGON)
				dragon = TRUE;
		}

		if((dragon == FALSE))
		{
			/* If the dragon is not present, set the duration to 0.
			   We can't remove the enchantment here, because on tick,
			   this function is called from enchantment_special which
			   will access the deallocated enchantment once we return.
			   This causes infinite badness */

			//frightful_presence(ench, ench_ch, ch, MSG_REMOVE_ENCH, NULL);
			//enchantment_remove(ench_ch, ench, 0);

			ench->duration = 0;
		}
	}

	return FALSE;
}
/* Spec assigns */

void assign_red_dragons()
{
	int i;

	assign_room(MAGMA_PIT        , red_dragons_pit);
	assign_room(HOLE_ROOM        , red_dragons_hole);
	assign_room(HOLE_REPL_ROOM   , red_dragons_hole_repl);
	assign_room(TUNNEL_ROOM      , red_dragons_tunnel_repl);
	assign_room(FIREWALL_ROOM    , red_dragons_firewall);
	for(i = DRAGON_LAIRSTART; i < DRAGON_LAIREND; i++)
		assign_room(i              , red_dragons_lair);
	assign_room(CRATER_SHELF     , red_dragons_shelf);

	assign_mob(YOUNG_DRAGON      , red_dragons_young);
	assign_mob(CHAMBER_ADULT     , red_dragons_chamber_adult);
	assign_mob(TUNNEL_ADULT      , red_dragons_tunnel_adult);
	assign_mob(FIREWALL_ELDER    , red_dragons_firewall_elder);
	assign_mob(LAIR_ELDER        , red_dragons_lair_elder);
	assign_mob(ANCIENT_DRAGON    , red_dragons_ancient);

	assign_obj(DRAGON_WINGS      , red_dragons_wings);
	assign_obj(GAUNTLETS         , red_dragons_gauntlets);
}
