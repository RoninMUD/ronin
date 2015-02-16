/*
  spec.mapmaker.c - Specs for Midgaard Map Maker & Cartographer, by BobaNight

  Written by Alan K. Miles for RoninMUD
  Based on Original Specs by Robert Upshall for RoninMUD
  Last Modification Date: 12/10/2011
*/

/* Includes */
/* System Includes */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Ronin Includes */
#include "structs.h"
#include "utils.h"
#include "db.h"
#include "cmd.h"
#include "comm.h"
#include "fight.h"
#include "handler.h"
#include "interpreter.h"
#include "limits.h"
#include "mob.spells.h"
#include "spec_assign.h"
#include "spells.h"
#include "act.h"
#include "utility.h"

/* Externs */
struct char_data *character_list;
struct obj_data *object_list;

extern int exp_table[58];
extern struct str_app_type str_app[];

extern char *skip_spaces(char * string);
void number_argument_interpreter(char *argument, int *number, char *first_arg, char *second_arg);

/* Defines */
/* Mobiles */
#define MM_MOB_MAPMAKER      3050
#define MM_MOB_CARTOGRAPHER  3051

/* Objects */
#define MM_OBJ_SCROLL        3150
#define MM_OBJ_SCROLL_CASE   3151
#define MM_OBJ_MAP_MIDGAARD  3012
#define MM_OBJ_MAP_FIRST     3152
#define MM_OBJ_MAP_LAST      3175
#define MM_OBJ_QUESTCARD     3179

/* Rooms */
#define MM_ROOM_MAPMAKER     3096
#define MM_ROOM_CARTOGRAPHER 3097

/* Misc */
#define MM_QUEST_MOB_NUM_CHOICES 4
#define MM_QUEST_MOB_LEVEL_UP    5
#define MM_QUEST_MOB_LEVEL_DOWN  3

/* Map Maker Quest Mobs */
struct mm_mob_t
{
	int quest_num;
	int mob_vnum;
};

#define MM_QUEST_MOB_COUNT   45
const struct mm_mob_t mm_moblist[MM_QUEST_MOB_COUNT] =
{
	{ 1, 110 },    /* chief sprite */
	{ 2, 3503 },   /* Porcupine */
	{ 3, 7334 },   /* fido dog (A scruffy Fido) */
	{ 4, 2613 },   /* spider */
	{ 5, 6000 },   /* john lumberjack */
	{ 6, 7324 },   /* nomad male */
	{ 7, 7401 },   /* guard beast */
	{ 8, 7315 },   /* bandit leader */
	{ 9, 6115 },   /* shargugh brownie */
	{ 10, 1303 },  /* gargon */
	{ 11, 1100 },  /* elven wizard */
	{ 12, 3730 },  /* sister (Mayor's Sister) */
	{ 13, 5303 },  /* captain (Griffon Captain) */
	{ 14, 6114 },  /* queen spider */
	{ 15, 8321 },  /* boy cabin john thomas */
	{ 16, 1000 },  /* gargoyle */
	{ 17, 11310 }, /* master burgonmaster */
	{ 18, 20805 }, /* Osquip */
	{ 19, 2645 },  /* alligator gator crocodile creature swamp */
	{ 20, 1116 },  /* elven warrior */
	{ 21, 10022 }, /* Enfan pastry chef */
	{ 22, 1117 },  /* dwarven prince */
	{ 23, 2912 },  /* fire newt leader */
	{ 24, 2634 },  /* zoo keeper zookeeper head */
	{ 25, 8611 },  /* prince */
	{ 26, 9619 },  /* unicorn */
	{ 27, 2306 },  /* white king */
	{ 28, 2315 },  /* black king */
	{ 29, 4809 },  /* bright wizard */
	{ 30, 11607 }, /* sailor elven */
	{ 31, 4811 },  /* grey wizard */
	{ 32, 2807 },  /* negress */
	{ 33, 2916 },  /* lizard (an enormous lizard) */
	{ 34, 7009 },  /* gnoll chieftain */
	{ 35, 8206 },  /* whale */
	{ 36, 2915 },  /* phoenix */
	{ 37, 20811 }, /* Repairman Jocko */
	{ 38, 4704 },  /* Halfling Zombie */
	{ 39, 20821 }, /* Fungus King */
	{ 40, 4517 },  /* tilik small pixie */
	{ 41, 4515 },  /* troll large crab herder */
	{ 42, 8901 },  /* king welmar */
	{ 43, 4517 },  /* tilik small pixie */
	{ 44, 4704 },  /* Halfling Zombie */
	{ 45, 20821 }  /* Fungus King */
};

/* Misc Functions*/
static int mm_rand_int(int n)
{
	int limit = RAND_MAX - RAND_MAX % n;
	int rnd = 0;

	do
	{
		rnd = rand();
	} while (rnd >= limit);

	return rnd % n;
}

void mm_shuffle_moblist(struct mm_mob_t *moblist, int n)
{
	int i = 0;
	int j = 0;
	struct mm_mob_t tmp;

	for (i = n - 1; i > 0; i--)
	{
		j = mm_rand_int(i + 1);
		tmp = moblist[j];
		moblist[j] = moblist[i];
		moblist[i] = tmp;
	}
}

/* Mobile Specs */
int mm_mapmaker_mob_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	char buf[MSL];
	char buf2[MIL];
	char name[MIL];
	int i = 0;
	int num = 0;
	int found = 0;
	int num_choices = 0;
	int quest_num = 0;
	int quest_mob_vnum = 0;
	struct mm_mob_t rnd_moblist[MM_QUEST_MOB_COUNT];
	struct mm_mob_t tmp_mob_t;
	CHAR *quest_mob = 0;
	OBJ *quest_obj = 0;

	if (IS_NPC(ch)) return FALSE;
	if (CHAR_VIRTUAL_ROOM(mob) != MM_ROOM_MAPMAKER) return FALSE;

	if (cmd == CMD_LIST)
	{
		if (!IS_MORTAL(ch)) return FALSE;

		for (i = 0; i < MM_QUEST_MOB_COUNT; i++)
		{
			rnd_moblist[i] = mm_moblist[i];
		}

		mm_shuffle_moblist(rnd_moblist, MM_QUEST_MOB_COUNT);

		sprintf(buf, "$N tells you 'The following quests are available to you...'");
		act(buf, FALSE, ch, 0, mob, TO_CHAR);

		for (i = 0, num_choices = 0; i < MM_QUEST_MOB_COUNT && num_choices < MM_QUEST_MOB_NUM_CHOICES; i++)
		{
			quest_mob_vnum = rnd_moblist[i].mob_vnum;

			found = FALSE;
			for (quest_mob = character_list; quest_mob; quest_mob = quest_mob->next)
			{
				if (!IS_MOB(quest_mob) || CHAR_REAL_ROOM(quest_mob) == NOWHERE) continue;

				if (V_MOB(quest_mob) == quest_mob_vnum)
				{
					quest_num = rnd_moblist[i].quest_num;
					found = TRUE;

					break;
				}
			}

			if (found && quest_mob)
			{
				if (GET_LEVEL(quest_mob) < (GET_LEVEL(ch) - MM_QUEST_MOB_LEVEL_DOWN) ||
					GET_LEVEL(quest_mob) > (GET_LEVEL(ch) + MM_QUEST_MOB_LEVEL_UP))
				{
					found = FALSE;
				}
			}

			if (found && quest_mob)
			{
				for (quest_obj = object_list; quest_obj; quest_obj = quest_obj->next)
				{
					if (V_OBJ(quest_obj) != MM_OBJ_SCROLL) continue;

					if (quest_obj->obj_flags.value[1] == ch->ver3.id &&
						quest_obj->obj_flags.value[2] == quest_num &&
						quest_obj->obj_flags.value[3] == quest_mob_vnum)
					{
						found = FALSE;

						break;
					}
					else
					if (quest_obj->obj_flags.value[2] == quest_num &&
						quest_obj->obj_flags.value[3] == quest_mob_vnum &&
						(((quest_obj->carried_by && (V_MOB(quest_obj->carried_by) != MM_MOB_MAPMAKER)) ||
						((quest_obj->in_obj && (V_OBJ(quest_obj->in_obj) != MM_OBJ_SCROLL_CASE)))) ||
						(!quest_obj->carried_by && !quest_obj->in_obj)))
					{
						found = FALSE;

						break;
					}
				}
			}

			if (found && quest_mob)
			{
				sprintf(buf, "Quest %d: %s.\n\r", quest_num, GET_SHORT(quest_mob));
				send_to_char(buf, ch);
			}

			if (found && quest_mob) num_choices++;
		}

		if (!num_choices)
		{
			sprintf(buf, "$N tells you 'There are no valid quest choices for you at this time. Please try back later.'");
			act(buf, FALSE, ch, 0, mob, TO_CHAR);
		}

		return TRUE;
	}
	else
	if (cmd == CMD_UNKNOWN)
	{
		if (!ch) return FALSE;
		if (!AWAKE(ch)) return FALSE;
		if (!IS_MORTAL(ch)) return FALSE;

		arg = skip_spaces(arg);
		arg = one_argument(arg, buf);

		if (strcmp(string_to_lower(buf), "choose")) return FALSE;

		arg = skip_spaces(arg);
		arg = one_argument(arg, buf);

		if (!is_number(buf))
		{
			sprintf(buf, "$N tells you 'You must choose a quest by its designated quest number. Please try again.'");
			act(buf, FALSE, ch, 0, mob, TO_CHAR);

			return TRUE;
		}

		quest_num = atoi(buf);

		found = FALSE;
		for (i = 0; i < MM_QUEST_MOB_COUNT; i++)
		{
			tmp_mob_t = mm_moblist[i];

			if (tmp_mob_t.quest_num == quest_num)
			{
				quest_mob_vnum = tmp_mob_t.mob_vnum;
				found = TRUE;

				break;
			}
		}

		if (!found)
		{
			sprintf(buf, "$N tells you 'I'm not offering a quest with that number. Please use 'list' to see the available quests.'\r\n");
			act(buf, FALSE, ch, 0, mob, TO_CHAR);

			return TRUE;
		}

		found = FALSE;
		for (quest_mob = character_list; quest_mob; quest_mob = quest_mob->next)
		{
			if (!IS_MOB(quest_mob) || CHAR_REAL_ROOM(quest_mob) == NOWHERE) continue;

			if (V_MOB(quest_mob) == quest_mob_vnum)
			{
				found = TRUE;

				break;
			}
		}

		if (found && quest_mob)
		{
			if (GET_LEVEL(quest_mob) < (GET_LEVEL(ch) - MM_QUEST_MOB_LEVEL_DOWN) ||
				GET_LEVEL(quest_mob) > (GET_LEVEL(ch) + MM_QUEST_MOB_LEVEL_UP))
			{
				sprintf(buf, "$N tells you 'You aren't eligible for that quest.'");
				act(buf, FALSE, ch, 0, mob, TO_CHAR);

				return TRUE;
			}
		}

		if (!found)
		{
			sprintf(buf, "$N tells you 'The target of that quest has been killed already.'");
			act(buf, FALSE, ch, 0, mob, TO_CHAR);

			return TRUE;
		}

		for (quest_obj = object_list; quest_obj; quest_obj = quest_obj->next)
		{
			if (V_OBJ(quest_obj) != MM_OBJ_SCROLL) continue;

			if (quest_obj->obj_flags.value[1] == ch->ver3.id &&
				quest_obj->obj_flags.value[2] == quest_num &&
				quest_obj->obj_flags.value[3] == quest_mob_vnum &&
				((quest_obj->carried_by && (V_MOB(quest_obj->carried_by) == MM_MOB_MAPMAKER)) ||
				((quest_obj->in_obj && (V_OBJ(quest_obj->in_obj) == MM_OBJ_SCROLL_CASE)))))
			{
				// the scroll has already been turned in to the map maker
				sprintf(buf, "$N tells you 'You have already completed that quest for me. Try again another time.'");
				act(buf, FALSE, ch, 0, mob, TO_CHAR);

				return TRUE;
			}
			else
			if (quest_obj->obj_flags.value[1] == ch->ver3.id &&
				quest_obj->obj_flags.value[2] == quest_num &&
				quest_obj->obj_flags.value[3] == quest_mob_vnum &&
				(quest_obj->carried_by && (quest_obj->carried_by == ch)))
			{
				// ch is carrying the scroll
				sprintf(buf, "$N tells you 'You've already found my lost scroll! Give it to me and I'll grant you a reward!'");
				act(buf, FALSE, ch, 0, mob, TO_CHAR);

				return TRUE;
			}
			else
			if (quest_obj->obj_flags.value[1] == ch->ver3.id &&
				quest_obj->obj_flags.value[2] == quest_num &&
				quest_obj->obj_flags.value[3] == quest_mob_vnum &&
				(quest_obj->carried_by && (V_MOB(quest_obj->carried_by) == quest_mob_vnum)))
			{
				// the quest mob has the scroll
				sprintf(buf2, "You are already hunting %s. Bring back my lost scroll and I'll give you a reward!", GET_SHORT(quest_mob));
				sprintf(buf, "$N tells you '%s'", buf2);
				act(buf, FALSE, ch, 0, mob, TO_CHAR);

				return TRUE;
			}
			else
			if (quest_obj->obj_flags.value[1] == ch->ver3.id &&
				quest_obj->obj_flags.value[2] == quest_num &&
				quest_obj->obj_flags.value[3] == quest_mob_vnum)
			{
				// the scroll is "somewhere else" (ground, someone has it, etc.)
				sprintf(buf, "$N tells you 'My lost scroll is somewhere out there... Bring it back and give it to me and I'll give you a reward!'");
				act(buf, FALSE, ch, 0, mob, TO_CHAR);

				return TRUE;
			}
			else
			if (quest_obj->obj_flags.value[2] == quest_num &&
				quest_obj->obj_flags.value[3] == quest_mob_vnum &&
				(((quest_obj->carried_by && V_MOB(quest_obj->carried_by) != MM_MOB_MAPMAKER) ||
				((quest_obj->in_obj && (V_OBJ(quest_obj->in_obj) != MM_OBJ_SCROLL_CASE)))) ||
				(!quest_obj->carried_by && !quest_obj->in_obj)))
			{
				// the scroll is out there somewhere, but ch is not the one on the quest
				sprintf(buf2, "%s is already being hunted. Pick another quest or try back later.", GET_SHORT(quest_mob));
				sprintf(buf, "$N tells you '%s'", buf2);
				act(buf, FALSE, ch, 0, mob, TO_CHAR);

				return TRUE;
			}
		}

		if (!quest_mob) return TRUE;

		sprintf(buf2, "Please, hunt down %s and find my lost scroll! Bring it back for a reward!", GET_SHORT(quest_mob));
		sprintf(buf, "$N tells you '%s'", buf2);
		act(buf, FALSE, ch, 0, mob, TO_CHAR);

		/* Load the lost scroll. */
		quest_obj = read_object(MM_OBJ_SCROLL, VIRTUAL);

		if (!quest_obj)
		{
			sprintf(buf, "Unable to load quest object with vnum: %d\r\n", MM_OBJ_SCROLL);
			send_to_char(buf, ch);

			wizlog(buf, LEVEL_IMM, WIZ_LOG_SIX);
			log_f("%s", buf);

			return TRUE;
		}

		quest_obj->obj_flags.value[1] = ch->ver3.id;
		quest_obj->obj_flags.value[2] = quest_num;
		quest_obj->obj_flags.value[3] = quest_mob_vnum;

		obj_to_char(quest_obj, quest_mob);

		/* Load a postcard with the name of the quest mob. */
		quest_obj = read_object(MM_OBJ_QUESTCARD, VIRTUAL);

		if (!quest_obj)
		{
			sprintf(buf, "Unable to load quest object with vnum: %d\r\n", MM_OBJ_QUESTCARD);
			send_to_char(buf, ch);

			wizlog(buf, LEVEL_IMM, WIZ_LOG_SIX);
			log_f("%s", buf);

			return TRUE;
		}

		sprintf(buf, "The Map Maker has asked you to retrieve his lost scroll from %s.\n\r", GET_SHORT(quest_mob));
		DESTROY(quest_obj->action_description);
		quest_obj->action_description = str_dup(buf);

		quest_obj->obj_flags.value[1] = ch->ver3.id;
		quest_obj->obj_flags.value[2] = quest_num;
		quest_obj->obj_flags.value[3] = quest_mob_vnum;

		obj_to_char(quest_obj, ch);

		act("$N hands you an $p to remind you of your quest.", FALSE, ch, quest_obj, mob, TO_CHAR);

		return TRUE;
	}
	else if (cmd == CMD_GIVE)
	{
		if (!ch) return FALSE;
		if (IS_NPC(ch)) return FALSE;

		number_argument_interpreter(arg, &num, buf, name);

		if (!*arg || !*name) return FALSE;
		else if (get_char_room_vis(ch, name) != mob) return FALSE;

		if (num != 1)
		{
			if (num > 1)
			{
				sprintf(buf, "$N tells you 'Please turn in your items one at a time.'");
				act(buf, 0, ch, 0, mob, TO_CHAR);
			}

			return TRUE;
		}

		quest_obj = get_obj_in_list_vis(ch, buf, ch->carrying);

		if (!quest_obj) return FALSE;

		if (V_OBJ(quest_obj) != MM_OBJ_SCROLL ||
			((V_OBJ(quest_obj) == MM_OBJ_SCROLL) && (quest_obj->obj_flags.value[1] != ch->ver3.id)))
		{
			sprintf(buf, "$N tells you 'I don't want that!'");
			act(buf, 0, ch, 0, mob, TO_CHAR);

			return TRUE;
		}

		quest_mob_vnum = quest_obj->obj_flags.value[3];

		sprintf(buf, "$N tells you 'Thanks! Here's your reward!'");
		act(buf, 0, ch, 0, mob, TO_CHAR);

		num = ((GET_EXP(ch) + exp_table[GET_LEVEL(ch) + 1] -  GET_EXP(ch)) / GET_LEVEL(ch)) * 0.1;

		sprintf(buf, "You gain %d experience for turning in the lost scroll.\r\n", num);
		send_to_char(buf, ch);

		gain_exp(ch, num);

		obj_from_char(quest_obj);
		obj_to_char(quest_obj, mob);

		REMOVE_BIT(quest_obj->obj_flags.extra_flags2, ITEM_ALL_DECAY);

		sprintf(buf, "put map case");
		command_interpreter(mob, buf);

		DESTROY(quest_obj->name);
		quest_obj->name = str_dup("mmmapcompleted");

		for (quest_obj = ch->carrying; quest_obj; quest_obj = quest_obj->next_content)
		{
			if (V_OBJ(quest_obj) == MM_OBJ_QUESTCARD &&
				quest_obj->obj_flags.value[3] == quest_mob_vnum)
			{
				obj_from_char(quest_obj);
				extract_obj(quest_obj);

				break;
			}
		}

		return TRUE;
	}

	return FALSE;
}


int mm_cartographer_mob_spec(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
	char buf[MSL];
	char name[MIL];
	int i = 0;
	int num = 0;
	int cost = 0;
	OBJ *tmp_obj = 0;

	if (cmd == CMD_LIST)
	{
		if (!ch) return FALSE;
		if (IS_NPC(ch)) return FALSE;
		if (CHAR_VIRTUAL_ROOM(mob) != MM_ROOM_CARTOGRAPHER) return FALSE;

		send_to_char("You can buy:\r\n", ch);

		for (i = 0, tmp_obj = mob->carrying; tmp_obj; tmp_obj = tmp_obj->next_content, i++)
		{
			if (V_OBJ(tmp_obj) == MM_OBJ_MAP_MIDGAARD ||
				(V_OBJ(tmp_obj) >= MM_OBJ_MAP_FIRST && V_OBJ(tmp_obj) <= MM_OBJ_MAP_LAST))
			{
				strcpy(name, OBJ_SHORT(tmp_obj));

				sprintf(buf, "%s for %d gold coins.\r\n", CAP(name), tmp_obj->obj_flags.cost);
				send_to_char(buf, ch);
			}
		}

		if (!i)
		{
			send_to_char("Nothing.\r\n", ch);
		}

		return TRUE;
	}
	else
	if (cmd == CMD_BUY)
	{
		if (!ch) return FALSE;
		if (IS_NPC(ch)) return FALSE;
		if (CHAR_VIRTUAL_ROOM(mob) != MM_ROOM_CARTOGRAPHER) return FALSE;

		number_argument_interpreter(arg, &num, buf, name);

		if (!*arg ||
			!*buf)
		{
			sprintf(buf, "$N asks you 'What would you like to buy?'");
			act(buf, 0, ch, 0, mob, TO_CHAR);

			return TRUE;
		}

		tmp_obj = get_obj_in_list_vis(ch, buf, mob->carrying);

		if (!tmp_obj)
		{
			sprintf(buf, "$N tells you 'I'm not selling anything like that right now.'");
			act(buf, 0, ch, 0, mob, TO_CHAR);

			return TRUE;
		}

		if (CAN_CARRY_N(ch) < (IS_CARRYING_N(ch) + num))
		{
			if (num > 1)
			{
				sprintf(buf, "%d*%s: You can't carry that many items.\n\r", num, OBJ_NAME(tmp_obj));
			}
			else
			{
				sprintf(buf, "%s: You can't carry that many items.\n\r", OBJ_NAME(tmp_obj));
			}
			send_to_char(buf, ch);
			
			return TRUE;
		}

		if (CAN_CARRY_W(ch) < (IS_CARRYING_W(ch) + (tmp_obj->obj_flags.weight * num)))
		{
			if (num > 1)
			{
				sprintf(buf, "%d*%s: You can't carry that much weight.\n\r", num, OBJ_NAME(tmp_obj));
			}
			else
			{
				sprintf(buf, "%s: You can't carry that much weight.\n\r", OBJ_NAME(tmp_obj));
			}
			send_to_char(buf, ch);

			return TRUE;
		}

		if (GET_LEVEL(ch) < LEVEL_IMM)
		{
			for (i = 0, cost = 0; i < num; i++, cost += tmp_obj->obj_flags.cost);

			sprintf(buf, "$N tells you 'That'll be %d coins.'", cost);
			act(buf, 0, ch, 0, mob, TO_CHAR);

			if (GET_GOLD(ch) < cost)
			{
				sprintf(buf, "$N tells you 'Which I see you can't afford.'");
				act(buf, 0, ch, 0, mob, TO_CHAR);

				return TRUE;
			}

			GET_GOLD(ch) -= cost;
			GET_GOLD(mob) += cost;
		}

		if (num > 1)
		{
			sprintf(buf, "$n buys %d*$p.", num);
		}
		else
		{
			sprintf(buf, "$n buys $p.");
		}
		act(buf, FALSE, ch, tmp_obj, 0, TO_ROOM);

		if (num > 1)
		{
			sprintf(buf, "You now have %d*%s.\n\r", num, OBJ_SHORT(tmp_obj));
		}
		else
		{
			sprintf(buf, "You now have %s.\n\r", OBJ_SHORT(tmp_obj));
		}
		send_to_char(buf, ch);

		for (i = 0; i < num; i++)
		{
			tmp_obj = read_object(tmp_obj->item_number, REAL);
			obj_to_char(tmp_obj, ch);
		}

		save_char(ch, NOWHERE);

		return TRUE;
	}
	else
	if (cmd == CMD_SELL || cmd == CMD_GIVE)
	{
		if (IS_NPC(ch)) return FALSE;
		if (CHAR_VIRTUAL_ROOM(mob) != MM_ROOM_CARTOGRAPHER) return FALSE;

		number_argument_interpreter(arg, &num, buf, name);

		if (!*arg || !*name) return FALSE;
		else if (get_char_room_vis(ch, name) != mob) return FALSE;

		sprintf(buf, "$N tells you 'Thanks, but no thanks.'");
		act(buf, 0, ch, 0, mob, TO_CHAR);

		return TRUE;
	}

	return FALSE;
}

/* Object Specs */
int mm_map_obj_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
	CHAR *owner = 0;

	if (cmd == CMD_GET)
	{
		if (!ch) return FALSE;

		if (obj->obj_flags.value[1] == ch->ver3.id)
		{
			if (!IS_SET(obj->obj_flags.wear_flags, ITEM_TAKE))
			{
				SET_BIT(obj->obj_flags.wear_flags, ITEM_TAKE);
			}
		}
		
		if (!CAN_GET_OBJ(ch, obj))
		{
			if (IS_SET(obj->obj_flags.wear_flags, ITEM_TAKE))
			{
				REMOVE_BIT(obj->obj_flags.wear_flags, ITEM_TAKE);
			}
		}

		return FALSE;
	}
	else
	if (cmd == MSG_DIE)
	{
		if (!ch) return FALSE;
		if (!(owner = obj->carried_by)) return FALSE;

		act("`bAs $N dies, $p drops to the floor!", FALSE, ch, obj, owner, TO_ROOM);
		act("`bAs $N dies, $p drops to the floor!", FALSE, ch, obj, owner, TO_CHAR);

		obj_from_char(obj);
		obj_to_room(obj, CHAR_REAL_ROOM(owner));

		if (IS_SET(obj->obj_flags.wear_flags, ITEM_TAKE))
		{
			REMOVE_BIT(obj->obj_flags.wear_flags, ITEM_TAKE);
		}

		return FALSE;
	}

	return FALSE;
}

/*

int mm_questcard_obj_spec(OBJ *obj, CHAR *ch, int cmd, char *arg)
{
	char buf[MSL];
	CHAR *owner = NULL;
	CHAR *quest_mob = NULL;
	OBJ *tmp_obj = NULL;
	int quest_mob_vnum = 0;
	int found = FALSE;

	if (cmd == CMD_LOOK ||
		cmd == CMD_EXAMINE)
	{
		if (!ch) return FALSE;
		if (!(owner = obj->carried_by)) return FALSE;

		arg = one_argument(arg, buf);

		if (!*buf) return FALSE;
		if (!isname(buf, OBJ_NAME(obj))) return FALSE;

		tmp_obj = get_obj_in_list_vis(ch, buf, ch->carrying);

		if (!tmp_obj || tmp_obj != obj) return FALSE;

		quest_mob_vnum = obj->obj_flags.value[3];

		for (quest_mob = character_list; quest_mob; quest_mob = quest_mob->next)
		{
			if (!IS_MOB(quest_mob) || CHAR_REAL_ROOM(quest_mob) == NOWHERE) continue;

			if (V_MOB(quest_mob) == quest_mob_vnum)
			{
				sprintf(buf, "The Map Maker has asked you to retrieve his lost scroll from %s.\n\r", GET_SHORT(quest_mob));
				send_to_char(buf, ch);

				found = TRUE;

				break;
			}
		}

		if (found) return TRUE;
	}

	return FALSE;
}
*/


/* Assign Specs */
void assign_mapmaker(void)
{
	assign_mob(MM_MOB_MAPMAKER, mm_mapmaker_mob_spec);
	assign_mob(MM_MOB_CARTOGRAPHER, mm_cartographer_mob_spec);

	assign_obj(MM_OBJ_SCROLL, mm_map_obj_spec);
/*	assign_obj(MM_OBJ_QUESTCARD, mm_questcard_obj_spec); */
}
