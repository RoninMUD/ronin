/*
** File quest.c
**
** Procedures of use when running quests.  Commands are for
** IMM+ with a quest flag.
**
** Programmed for RoninMUD by Ranger - Original Date: June 20/96
**
** Last Modifacation date: Dec 2000 - added token - Ranger
**
** Using this code is not allowed without permission from originator.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "utility.h"
#include "fight.h"
#include "modify.h"
#include "utility.h"
#include "cmd.h"
#include "act.h"
#include "spec_assign.h"
#include "quest.h"
#include "enchant.h"
#include "remortv2.h"

/* global data */
struct scoreboard_data scores[101];
int number_of_kills=0;

/* declarations */
void write_board(int vnum,char *heading,char *message);

void do_quest(CHAR *ch, char *arg, int cmd) {
  if (!ch) return;

  if (!IS_NPC(ch) && IS_SET(GET_PFLAG(ch), PLR_NOSHOUT)) {
    printf_to_char(ch, "You have displeased the gods; you cannot quest.\n\r");

    return;
  }

  arg = skip_spaces(arg);

  if (!IS_NPC(ch) && is_abbrev(arg, "flag")) {
    if (!IS_SET(GET_PFLAG(ch), PLR_QUESTC)) {
      SET_BIT(GET_PFLAG(ch), PLR_QUESTC);

      printf_to_char(ch, "You turn ON the quest channel.\n\r");
    }

    TOGGLE_BIT(GET_PFLAG(ch), PLR_QUEST);

    printf_to_char(ch, "You turn %s your quest flag.\n\r", (IS_SET(GET_PFLAG(ch), PLR_QUEST) ? "ON" : "OFF"));

    wizlog_f(LEVEL_IMM, 7, "QSTINFO: %s turned %s %s quest flag.", GET_NAME(ch), HSHR(ch), (IS_SET(GET_PFLAG(ch), PLR_QUEST) ? "ON" : "OFF"));

    return;
  }

  communicate(ch, arg, COMM_QUEST);
}

#define FUNCT_WHERE      1
#define FUNCT_TRANSFER   2
#define FUNCT_GIVE       3
#define FUNCT_QUIETON    4
#define FUNCT_QUIETOFF   5
#define FUNCT_KICKOUT    6
#define FUNCT_QFLAG      7
#define FUNCT_CHAOTIC    8
#define FUNCT_LOG        9
#define FUNCT_TELEPORT   10
#define FUNCT_ZONE       11
#define FUNCT_SPREAD     12
#define FUNCT_SBRESET    13
#define FUNCT_TOKEN      14
#define FUNCT_QCARD      15
#define FUNCT_QINFO      16
#define FUNCT_QP         17
#define FUNCT_SCP        18
#define FUNCT_GOLD       19
#define FUNCT_EXP        20
#define FUNCT_REMORT_EXP 21

int check_god_access(CHAR *ch, int active);

extern struct obj_data  *object_list;
void do_qfunction(CHAR* ch, char *arg, int cmd)
{
  char arg1[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
  int function=0,target,num,vnum,i;
  struct string_block   sb;
  struct descriptor_data *d;
  struct char_data *victim;
  struct obj_data *obj=NULL,*next_obj;
  bool found;

  if (!check_god_access(ch,TRUE)) return;

  if(!IS_SET(GET_IMM_FLAGS(ch), WIZ_QUEST)) {
    send_to_char("You do not have the WIZ_QUEST flag.\n\r", ch);
    return;
  }

  arg = one_argument(arg, arg1);

  char field_name[MIL], friendly_name[MIL];

  if (*arg1) {
    if (is_abbrev(arg1, "where"))     function = FUNCT_WHERE;
    if (is_abbrev(arg1, "transfer"))  function = FUNCT_TRANSFER;
    if (is_abbrev(arg1, "give"))      function = FUNCT_GIVE;
    if (is_abbrev(arg1, "quieton"))   function = FUNCT_QUIETON;
    if (is_abbrev(arg1, "quietoff"))  function = FUNCT_QUIETOFF;
    if (is_abbrev(arg1, "kickout"))   function = FUNCT_KICKOUT;
    if (is_abbrev(arg1, "qflag"))     function = FUNCT_QFLAG;
    if (is_abbrev(arg1, "chaotic"))   function = FUNCT_CHAOTIC;
    if (is_abbrev(arg1, "log"))       function = FUNCT_LOG;
    if (is_abbrev(arg1, "teleport"))  function = FUNCT_TELEPORT;
    if (is_abbrev(arg1, "zone"))      function = FUNCT_ZONE;
    if (is_abbrev(arg1, "spread"))    function = FUNCT_SPREAD;
    if (is_abbrev(arg1, "sbreset"))   function = FUNCT_SBRESET;
    if (is_abbrev(arg1, "token"))     function = FUNCT_TOKEN;
    if (is_abbrev(arg1, "qcard"))     function = FUNCT_QCARD;
    if (is_abbrev(arg1, "award") ||
        is_abbrev(arg1, "qp") ||
        is_abbrev(arg1, "qpts")) {
      function = FUNCT_QP;
      snprintf(field_name, sizeof(field_name), "qp");
      snprintf(friendly_name, sizeof(friendly_name), "quest point");
    }
    if (is_abbrev(arg1, "scp") ||
        is_abbrev(arg1, "subpts")) {
      function = FUNCT_SCP;
      snprintf(field_name, sizeof(field_name), "scp");
      snprintf(friendly_name, sizeof(friendly_name), "subclass point");
    }
    if (is_abbrev(arg1, "gold")) {
      function = FUNCT_GOLD;
      snprintf(field_name, sizeof(field_name), "gold");
      snprintf(friendly_name, sizeof(friendly_name), "gold coin");
    }
    if (is_abbrev(arg1, "experience") ||
        is_abbrev(arg1, "xp")) {
      function = FUNCT_EXP;
      snprintf(field_name, sizeof(field_name), "exp");
      snprintf(friendly_name, sizeof(friendly_name), "experience point");
    }
    if (is_abbrev(arg1, "remort_experience") ||
        is_abbrev(arg1, "remort_xp")) {
      function = FUNCT_REMORT_EXP;
      snprintf(field_name, sizeof(field_name), "remort_exp");
      snprintf(friendly_name, sizeof(friendly_name), "remort experience point");
    }
  }

  arg = one_argument(arg, arg1);

  switch (function) {
    case FUNCT_WHERE:
      init_string_block(&sb);
      append_to_string_block(&sb, "Quest Players:\n\r--------\n\r");

      for (d = descriptor_list; d; d = d->next) {
        if (d->character && (d->connected == CON_PLYNG)
	    && (CHAR_REAL_ROOM(d->character) != NOWHERE)
	    && CAN_SEE(ch, d->character)
            && IS_SET(d->character->specials.pflag, PLR_QUEST) ) {
          if (d->original)   /* If switched */
            sprintf(buf, "%-20s - %s [%d] In body of %s\n\r",
                    GET_NAME(d->original),
                    world[CHAR_REAL_ROOM(d->character)].name,
                    world[CHAR_REAL_ROOM(d->character)].number, /* Ranger - June 96 */
                    fname(GET_NAME(d->character)));
          else
            sprintf(buf, "%-20s - %s [%d]\n\r",
                    GET_NAME(d->character),
                    world[CHAR_REAL_ROOM(d->character)].name,
                    world[CHAR_REAL_ROOM(d->character)].number);

          append_to_string_block(&sb, buf);
        }
      }
      page_string_block(&sb,ch);
      destroy_string_block(&sb);
      return;
      break;

    case FUNCT_TRANSFER:
      if(!*arg1) {
        send_to_char("\
Usage: qf transfer <char>/<all>\n\r\
       Transport all players with quest flags to your location.\n\r",ch);
        return;
      }
      if (!strcmp(arg1,"all")) {
        for (d = descriptor_list; d; d = d->next) {
          if (d->character != ch && !d->connected
              && IS_SET(d->character->specials.pflag, PLR_QUEST) ) {
            victim = d->character;
            if(GET_LEVEL(victim)<LEVEL_IMM) {
              act("$n disappears in a mushroom cloud.",
              FALSE, victim, 0, 0, TO_ROOM);

              target = CHAR_REAL_ROOM(ch);
	            char_from_room(victim);
	            char_to_room(victim,target);
              if(victim->specials.riding) {
                char_from_room(victim->specials.riding);
      	        char_to_room(victim->specials.riding,target);
              }
	      act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
	      act("$n has transferred you!",FALSE,ch,0,victim,TO_VICT);
	      do_look(victim,"",15);
            } else {
              if(IS_SET(victim->new.imm_flags, WIZ_LOG_FOUR)) {
                sprintf(buf,"%s requests you go to #%d for a quest.\n\r",
                     GET_NAME(ch), CHAR_VIRTUAL_ROOM(ch));
                send_to_char(buf,victim);
              }
            }
          }
        }

        sprintf (buf, "QSTINFO: %s transferred all to #%d for a quest.",
                 GET_NAME(ch), CHAR_VIRTUAL_ROOM(ch));
        wizlog(buf, GET_LEVEL(ch)+1, 4);
        log_s(buf);
        return;
      }

      if ((victim = get_char_vis(ch, arg1))) {
        if (IS_SET(victim->specials.pflag, PLR_QUEST)) {
          if(GET_LEVEL(victim)<LEVEL_IMM) {
            act("$n disappears in a mushroom cloud.",
              FALSE, victim, 0, 0, TO_ROOM);

            target = CHAR_REAL_ROOM(ch);
	          char_from_room(victim);
	          char_to_room(victim,target);
            if(victim->specials.riding) {
              char_from_room(victim->specials.riding);
      	      char_to_room(victim->specials.riding,target);
            }
	    act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
	    act("$n has transferred you!",FALSE,ch,0,victim,TO_VICT);
	    do_look(victim,"",15);
            sprintf (buf, "QSTINFO: %s transferred %s to #%d for a quest.",
                     GET_NAME(ch), GET_NAME(victim),CHAR_VIRTUAL_ROOM(ch));
            wizlog(buf, GET_LEVEL(ch)+1, 4);
            log_s(buf);
          } else {
            sprintf(buf,"%s requests you go to #%d for a quest.\n\r",
                   GET_NAME(ch), CHAR_VIRTUAL_ROOM(ch));
            send_to_char(buf,victim);
          }
        } else send_to_char("That player is not in the quest.\n\r",ch);
      } else send_to_char("That player cannot be found.\n\r",ch);
      return;
      break;

    case FUNCT_QUIETON:
      if(!*arg1) {
        send_to_char("\
Usage: qf quieton <char>/<all>\n\r\
       Turns on the victims quiet flag or the quiet flags of all\n\r\
       questers in the game. This prevents needless spam when\n\r\
       the quest runner is explaining the quest.\n\r",ch);
        return;
      }
      if (!strcmp(arg1,"all")) {
        for (d = descriptor_list; d; d = d->next) {
          if (d->character && (d->connected == CON_PLYNG)
             && (CHAR_REAL_ROOM(d->character) != NOWHERE)
	     && CAN_SEE(ch, d->character)
             && IS_SET(d->character->specials.pflag, PLR_QUEST)
             && !IS_SET(d->character->specials.pflag, PLR_QUIET)
             && (GET_LEVEL(d->character) < LEVEL_IMM) ) {
            SET_BIT(d->character->specials.pflag, PLR_QUIET);
            send_to_char("Quiet flag on.\n\r",d->character);
          }
        }
        send_to_char("You have set all mortal quiet flags.\n\r",ch);
        return;
      }

      if ((victim = get_char_vis(ch, arg1))) {
        if (IS_SET(victim->specials.pflag, PLR_QUEST)) {
          if (IS_SET(victim->specials.pflag, PLR_QUIET)) {
             send_to_char("Quiet flag already on.\n\r", ch);
          } else {
             SET_BIT(victim->specials.pflag, PLR_QUIET);
             send_to_char("You turn ON the quiet flag.\n\r", ch);
             send_to_char("Quiet flag on.\n\r",victim);
          }
        } else send_to_char("That player is not in the quest.\n\r",ch);
      } else send_to_char("That player cannot be found.\n\r",ch);
      return;
      break;

    case FUNCT_QUIETOFF:
      if(!*arg1) {
        send_to_char("\
Usage: qf quietoff <char>/<all>\n\r\
       Turns off the victims quiet flag or the quiet flags of all\n\r\
       questers in the game.\n\r",ch);
        return;
      }
      if (!strcmp(arg1,"all")) {
        for (d = descriptor_list; d; d = d->next) {
          if (d->character && (d->connected == CON_PLYNG)
             && (CHAR_REAL_ROOM(d->character) != NOWHERE)
	     && CAN_SEE(ch, d->character)
             && IS_SET(d->character->specials.pflag, PLR_QUIET)
             && (GET_LEVEL(d->character) < LEVEL_IMM) ) {
            REMOVE_BIT(d->character->specials.pflag, PLR_QUIET);
            send_to_char("Quiet flag off.\n\r",d->character);
          }
        }
        send_to_char("You have removed all mortal quiet flags.\n\r",ch);
        return;
      }

      if ((victim = get_char_vis(ch, arg1))) {
        if (IS_SET(victim->specials.pflag, PLR_QUIET)) {
           REMOVE_BIT(victim->specials.pflag, PLR_QUIET);
           send_to_char("Quiet flag off.\n\r",victim);
           send_to_char("You turn OFF the quiet flag.\n\r", ch);
        } else {
           send_to_char("Quiet flag already off.\n\r", ch);
        }
      } else send_to_char("That player cannot be found.\n\r",ch);
      return;
      break;

    case FUNCT_GIVE:
      if(!*arg1) {
        send_to_char("\
Usage: qf give <argument>\n\r\
       Useful when you need to give the same item to all questers.\n\r\
       For example: 'qf give 3 recall' would give 3 recall scrolls\n\r\
       to each quester in the room.  This assumes the scrolls are\n\r\
       already loaded.\n\r",ch);
        return;
      }
      for (d = descriptor_list; d; d = d->next) {
        if (d->character && (d->connected == CON_PLYNG)
	    && (CHAR_REAL_ROOM(d->character) != NOWHERE)
	    && CAN_SEE(ch, d->character) && (ch!=d->character)
            && IS_SET(d->character->specials.pflag, PLR_QUEST) ) {
           sprintf(buf, "%s %s", arg1,GET_NAME(d->character));
           do_give(ch,buf,CMD_GIVE);
        }
      }
      return;
      break;

    case FUNCT_KICKOUT:
      if(!*arg1) {
        send_to_char("\
Usage: qf kickout <char>/<all>\n\r\
       This will turn off the victims quest flag or the quest flag\n\r\
       of all the mortals in the game.  Used to disqualify a quester.\n\r",ch);
        return;
      }
      if (!strcmp(arg1,"all")) {
        for (d = descriptor_list; d; d = d->next) {
          if (d->character && (d->connected == CON_PLYNG)
             && (CHAR_REAL_ROOM(d->character) != NOWHERE)
             && CAN_SEE(ch, d->character)
             && IS_SET(d->character->specials.pflag, PLR_QUEST)
             && (GET_LEVEL(d->character) < LEVEL_IMM) ) {
            REMOVE_BIT(d->character->specials.pflag, PLR_QUEST);
            send_to_char("Quest flag off.\n\r",d->character);
          }
        }
        send_to_char("You have removed all mortal quest flags.\n\r",ch);
        return;
      }

      if ((victim = get_char_vis(ch, arg1))) {
        if (IS_SET(victim->specials.pflag, PLR_QUEST)) {
          REMOVE_BIT(victim->specials.pflag, PLR_QUEST);
          send_to_char("You turn OFF the quest flag.\n\r", ch);
          send_to_char("Quest flag off.\n\r",victim);
        } else send_to_char("That player is not in the quest.\n\r",ch);
      } else send_to_char("That player cannot be found.\n\r",ch);
      return;
      break;

    case FUNCT_QFLAG:
      if(!*arg1) {
        send_to_char("\
Usage: qf qflag <char>/<all>\n\r\
       This turns on the quest flag of the victim or of all the\n\r\
       characters in the same room.\n\r",ch);
        return;
      }
      if (!strcmp(arg1,"all")) {
        target = CHAR_REAL_ROOM(ch);
        for (d = descriptor_list; d; d = d->next) {
          if (d->character && (d->connected == CON_PLYNG)
             && (CHAR_REAL_ROOM(d->character) == target)
             && CAN_SEE(ch, d->character)
             && !IS_SET(d->character->specials.pflag, PLR_QUEST)) {
            SET_BIT(d->character->specials.pflag, PLR_QUEST);
            send_to_char("Quest flag on.\n\r",d->character);
          }
        }
        send_to_char("You have set all quest flags in this room.\n\r",ch);
        return;
      }

      if ((victim = get_char_vis(ch, arg1))) {
        if (!IS_SET(victim->specials.pflag, PLR_QUEST)) {
           SET_BIT(victim->specials.pflag, PLR_QUEST);
           send_to_char("You turn ON the quest flag.\n\r", ch);
           send_to_char("Quest flag on.\n\r",victim);
        } else send_to_char("Quest flag already on that player.\n\r",ch);
      } else send_to_char("That player cannot be found.\n\r",ch);

      return;
      break;

    case FUNCT_CHAOTIC:
      if(!*arg1) {
        send_to_char("\
Usage: qf chaotic <obj>/<all>\n\r\
       This turns on the chaotic flag to the object or all objects\n\r\
       you have in inventory.  Chaotic objects always remain in the\n\r\
       corpse of a victim in the chaos zone.\n\r",ch);
        return;
      }

      if (!strcmp(arg1,"all")) {
        for(obj = ch->carrying; obj; obj = obj->next_content) {
          if (obj) {
            found=TRUE;
            if (!IS_SET(obj->obj_flags.extra_flags, ITEM_CHAOTIC))
                 SET_BIT(obj->obj_flags.extra_flags, ITEM_CHAOTIC);
          }
        }

        if(found)
          send_to_char("You have set the CHAOTIC flag to all items in inventory.\n\r",ch);
        else
          send_to_char("You have nothing in inventory.\n\r",ch);
        return;
      }

      if (!(obj=get_obj_in_list_vis(ch, arg1, ch->carrying))) {
        send_to_char("You don't seem to have anything like that.\n\r", ch);
        return;
      }

      if (!IS_SET(obj->obj_flags.extra_flags, ITEM_CHAOTIC)) {
        SET_BIT(obj->obj_flags.extra_flags, ITEM_CHAOTIC);
        act("You add a CHAOTIC flag to $p.",0,ch,obj,0,TO_CHAR);
      }
      else
        act("$p already had a CHAOTIC flag.",0,ch,obj,0,TO_CHAR);

      return;
      break;

    case FUNCT_LOG:
      if(!*arg1) {
        send_to_char("\
Usage: qf log <obj>/<all>\n\r\
       This command is used to track items as they are picked up\n\r\
       dropped, etc.  An individual item, or all items in inventory\n\r\
       can be flagged for logging.\n\r",ch);
        return;
      }

      if (!strcmp(arg1,"all")) {
        for(obj = ch->carrying; obj; obj = obj->next_content) {
          if (obj) {
            found=TRUE;
            obj->log=2;
          }
        }

        if(found)
          send_to_char("All items in inventory will now be logged.\n\r",ch);
        else
          send_to_char("You have nothing in inventory.\n\r",ch);
        return;
      }

      if (!(obj=get_obj_in_list_vis(ch, arg1, ch->carrying))) {
        send_to_char("You don't seem to have anything like that.\n\r", ch);
        return;
      }

      obj->log=2;
      act("$p will now be logged.",0,ch,obj,0,TO_CHAR);
      return;
      break;

    case FUNCT_TELEPORT:
      do {
        target=number(0, top_of_world);
      }
      while (IS_SET(world[target].room_flags, PRIVATE) ||
             IS_SET(world[target].room_flags, LAWFUL) ||
             IS_SET(world[target].room_flags, DEATH) ||
             IS_SET(world[target].room_flags, LOCK));

      act("$n slowly fades out of existence.", FALSE, ch,0,0,TO_ROOM);
      char_from_room(ch);
      char_to_room(ch, target);
      act("$n slowly fades into existence.", FALSE, ch,0,0,TO_ROOM);
      do_look(ch, "", CMD_LOOK);
      break;

    case FUNCT_SBRESET:
      number_of_kills=0;
      send_to_char("Scoreboard reset.\n\r",ch);
      break;

    case FUNCT_SPREAD:
      if(!*arg1) {
        send_to_char("\
Usage: qf spread <num> <vnum>\n\r\
       This command is used to spread an item randomly around the\n\r\
       game.  The num is the number of items to spread, and the\n\r\
       vnum is the virtual number of the item.\n\r",ch);
        return;
      }

      if(!is_number(arg1)) {
        send_to_char("The first value needs to be a number.\n\r",ch);
        return;
      }
      else {
        num=atoi(arg1);
        if(num<1 || num>50) {
          send_to_char("The number should be between 1 and 50.\n\r",ch);
          return;
        }
      }

      arg=one_argument(arg,arg1);
      if(!*arg1) {
        send_to_char("What is the object's virtual number?\n\r",ch);
        return;
      }
      if(!is_number(arg1)) {
        send_to_char("The second value needs to be a number.\n\r",ch);
        return;
      }
      else {
        vnum=atoi(arg1);
        if(real_object(vnum)<0) {
          send_to_char("That object doesn't exist.\n\r",ch);
          return;
        }
      }

      if(top_of_zone_table<2) {
        send_to_char("There are not enough zones loaded.\n\r",ch);
        return;
      }

      sprintf(buf, "QSTINFO: %s spread %d of %d.",GET_NAME(ch),num,vnum);
      wizlog(buf, GET_LEVEL(ch)+1, 4);
      log_s(buf);

      for(i=0;i<num;i++) {
        do {
          target=number(0, top_of_world);
        }
        while (IS_SET(world[target].room_flags, PRIVATE) ||
               IS_SET(world[target].room_flags, DEATH) ||
               IS_SET(world[target].room_flags, LOCK) ||
               world[target].zone == world[real_room(0)].zone || /* Void & Limbo */
               world[target].zone == world[real_room(3014)].zone || /* Upper City */
               world[target].zone == world[real_room(1212)].zone || /* Immort */
               world[target].zone == world[real_room(3137)].zone || /* Lower City */
               world[target].zone == world[real_room(25300)].zone || /* Hell 1 */
               world[target].zone == world[real_room(25400)].zone || /* Hell 2 */
               world[target].zone == world[real_room(25500)].zone || /* Hell 3 */
               world[target].zone == world[real_room(27500)].zone || /* Clan Halls */
               world[target].zone == world[real_room(26100)].zone || /* Quest Zone Not connected */ 
               world[target].zone == world[real_room(3601)].zone || /* Cafe, by Jarldian */
               world[target].zone == world[real_room(6600)].zone || /* NewbieMist */
               world[target].zone == world[real_room(27800)].zone || /* ISAHall */
               world[target].zone == world[real_room(26000)].zone || /* Quest Zone - not connected */
               world[target].zone == world[real_room(26200)].zone || /* Quest Zone - not connected */
               world[target].zone == world[real_room(300)].zone || /* Salamandastron */
               world[target].zone == world[real_room(400)].zone); /* Salamandastron2 */
        obj=read_object(vnum,VIRTUAL);
        obj_to_room(obj, target);
        printf_to_char(ch,"%s placed in room %s (%d).\n\r",OBJ_SHORT(obj),world[target].name,world[target].number);
      }
      break;

    case FUNCT_ZONE:
      if(!*arg1) {
        send_to_char("\
Usage: qf zone <normal>/<never>/<doors>\n\r\
       This controls the zone reset mode of the chaos zone.\n\r\
       Normal is a repop of all mobs and closing of doors.\n\r\
       Never will prevent mobs from repoping and doors from auto closing.\n\r\
       Doors will allow doors to auto close but not allow mob repop.\n\r",ch);
        return;
      }

      if(is_abbrev(arg1, "normal")) {
        zone_table[real_zone(300)].reset_mode = 2;
        sprintf (buf, "QSTINFO: %s set Chaos Zone reset mode to normal.",GET_NAME(ch));
        wizlog(buf, GET_LEVEL(ch)+1, 4);
        send_to_char("Chaos Zone reset mode set to normal.\n\r", ch);
        return;
      } else if(is_abbrev(arg1, "never")) {
        zone_table[real_zone(300)].reset_mode = 0;
        sprintf (buf, "QSTINFO: %s set Chaos Zone reset mode to never.",GET_NAME(ch));
        wizlog(buf, GET_LEVEL(ch)+1, 4);
        send_to_char("Chaos Zone reset mode set to never.\n\r", ch);
        return;
      } else if(is_abbrev(arg1, "doors")) {
        zone_table[real_zone(300)].reset_mode = 5;
        sprintf (buf, "QSTINFO: %s set Chaos Zone reset mode to doors only.",GET_NAME(ch));
        wizlog(buf, GET_LEVEL(ch)+1, 4);
        send_to_char("Chaos Zone reset mode set to doors only.\n\r", ch);
        return;
      } else {
        send_to_char("Usage: qf zone <normal>/<never>/<doors>\n\r",ch);
        return;
      }

      break;

    case FUNCT_TOKEN:
      if(!*arg1) {
        send_to_char("\
Usage: qf token <char>\n\r\
       This command gives a subclass token to the indicated character.\n\r",ch);
        return;
      }

      if(real_object(5)<0) {
        send_to_char("The token doesn't exist.\n\r",ch);
      }

      if (!(victim = get_char_room_vis(ch, arg1))) {
        send_to_char("That player is not here.\n\r",ch);
        return;
      }
      if(IS_NPC(victim)) {
        send_to_char("You cannot token a mob.\n\r",ch);
        return;
      }

      sprintf(buf, "QSTINFO: %s gave a token to %s.",GET_NAME(ch),GET_NAME(victim));
      wizlog(buf, GET_LEVEL(ch)+1, 4);
      log_s(buf);
      act("You reward $N with a token.",0,ch,0,victim,TO_CHAR);
      act("$n rewards you with a token.",0,ch,0,victim,TO_VICT);
      act("$n rewards $N with a token.",0,ch,0,victim,TO_NOTVICT);
      obj=read_object(5,VIRTUAL);
      obj_to_char(obj, victim);
      return;
      break;
    case FUNCT_QCARD:
      if(!*arg1) {
        send_to_char("\
Usage: qf qc load <num> (Loads # of quest cards)\n\r\
       qf qc collect (Pulls all quest cards into inventory (skips room 1204)\n\r",ch);
        return;
      }

      if(real_object(35)<0) {
        send_to_char("The quest card doesn't exist.\n\r",ch);
      }

      if(is_abbrev(arg1,"load")) {
        arg=one_argument(arg,arg1);
        if(!*arg1) {
          send_to_char("How many quest cards?\n\r",ch);
          return;
        }
        if(!is_number(arg1)) {
          send_to_char("The amount of quest cards needs to be a number.\n\r",ch);
          return;
        }
        vnum=atoi(arg1);
        if(vnum<1 || vnum>100) {
          send_to_char("The number of cards should be between 0 and 100.\n\r",ch);
          return;
        }
        sprintf(buf, "QSTINFO: %s loads %d quest cards",GET_NAME(ch),vnum);
        wizlog(buf, GET_LEVEL(ch)+1, 4);
        log_s(buf);
        while(vnum) {
          obj=read_object(35,VIRTUAL);
          obj_to_char(obj, ch);
          vnum--;
        }
        send_to_char("Done.\n\r",ch);
        return;
      }
      else if(is_abbrev(arg1,"collect")) {
        for(obj=object_list; obj; obj = next_obj) {
          next_obj=obj->next;
          if(V_OBJ(obj)!=35) continue;
          if(OBJ_SPEC(obj)>0) continue;
          if(obj->carried_by) {
            obj_from_char(obj);
            obj_to_char(obj,ch);
            continue;
          }
          if(obj->in_obj) {
            if(obj->in_obj->in_room==real_room(1204)) continue;
            obj_from_obj(obj);
            obj_to_char(obj,ch);
            continue;
          }
          if(obj->in_room!=NOWHERE) {
            if(obj->in_room==real_room(1204)) continue;
            obj_from_room(obj);
            obj_to_char(obj,ch);
            continue;
          }
          if(obj->equipped_by) {
            for(i=0;i<MAX_WEAR;i++)
              if(obj->equipped_by->equipment[i]==obj) break;
            unequip_char(obj->equipped_by,i);
            obj_to_char(obj,ch);
            continue;
          }
        }
        send_to_char("Done.\n\r",ch);
      }
      else {
        send_to_char("\
Usage: qf qc load <num> (Loads # of quest cards)\n\r\
       qf qc collect (Pulls all quest cards into inventory (skips room 1204)\n\r",ch);
        return;
      }
      return;
      break;
    case FUNCT_QINFO:
      if (IS_SET(ch->new.imm_flags, WIZ_QUEST_INFO)) {
        send_to_char("OK, you have turned off quest info.\n\r", ch);
        REMOVE_BIT(ch->new.imm_flags, WIZ_QUEST_INFO);
      } else {
        send_to_char("OK, you have turned on quest info.\n\r", ch);
        SET_BIT(ch->new.imm_flags, WIZ_QUEST_INFO);
      }
      break;
    case FUNCT_QP:
    case FUNCT_SCP:
    case FUNCT_GOLD:
    case FUNCT_EXP:
    case FUNCT_REMORT_EXP:
      if (!*arg1) {
        printf_to_char(ch, "Usage: qf %s <char> <amount>\n\r", field_name);

        return;
      }

      if (!(victim = get_char_room_vis(ch, arg1))) {
        send_to_char("That player is not here.\n\r", ch);

        return;
      }

      if (IS_NPC(victim)) {
        send_to_char("You cannot award an NPC.\n\r", ch);

        return;
      }

      arg = one_argument(arg, arg1);

      if (!*arg1) {
        printf_to_char(ch, "How many %ss?\n\r", friendly_name);

        return;
      }

      int amount = 0;
      long long big_amount = 0;

      if (!is_number(arg1)) {
        send_to_char("Amount needs to be a non-zero number.\n\r", ch);

        return;
      }
      else {
        switch (function) {
          case FUNCT_QP:
          case FUNCT_SCP:
          case FUNCT_GOLD:
          case FUNCT_EXP:
            amount = atoi(arg1);
            break;
          case FUNCT_REMORT_EXP:
            errno = 0;

            big_amount = strtoll(arg1, NULL, 10);

            if (((errno == ERANGE) && ((big_amount == LONG_MAX) || (big_amount == LONG_MIN))) || ((errno != 0) && (big_amount == 0))) {
              send_to_char("Amount specified was out of range.  Try again.\n\r", ch);

              return;
            }
            break;
        }
      }

      bool bad_amount = FALSE;

      switch (function) {
        case FUNCT_QP:
          bad_amount = (((GET_QP(victim) + amount) < 0) || ((INT_MAX - GET_QP(victim)) < amount));
          break;
        case FUNCT_SCP:
          bad_amount = (((GET_SCP(victim) + amount) < 0) || ((INT_MAX - GET_SCP(victim)) < amount));
          break;
        case FUNCT_GOLD:
          bad_amount = (((GET_GOLD(victim) + amount) < 0) || ((INT_MAX - GET_GOLD(victim)) < amount));
          break;
        case FUNCT_EXP:
          bad_amount = (((GET_EXP(victim) + amount) < 0) || ((INT_MAX - GET_EXP(victim)) < amount));
          break;
        case FUNCT_REMORT_EXP:
          bad_amount = (((GET_REMORT_EXP(victim) + big_amount) < 0) || ((LONG_MAX - GET_REMORT_EXP(victim)) < big_amount));
          break;
      }

      if ((amount == 0) && (big_amount == 0)) {
        printf_to_char(ch, "You cannot award zero %ss.\n\r", friendly_name);
        return;
      }
      else if (bad_amount && ((amount < 0) || (big_amount < 0))) {
        printf_to_char(ch, "That amount would leave the charater with less than zero %ss.\n\r", friendly_name);

        return;
      }
      else if (bad_amount && ((amount > 0) || (big_amount > 0))) {
        printf_to_char(ch, "That amount would exceed the maximum limit for %ss.\n\r", friendly_name);

        return;
      }

      if (amount != 0) {
        snprintf(buf, sizeof(buf), "QSTINFO: %s awarded %s with %d %s(s).", GET_NAME(ch), GET_NAME(victim), amount, friendly_name);
      }
      else if (big_amount != 0) {
        snprintf(buf, sizeof(buf), "QSTINFO: %s awarded %s with %lld %s(s).", GET_NAME(ch), GET_NAME(victim), big_amount, friendly_name);
      }
      wizlog(buf, GET_LEVEL(ch) + 1, 4);
      log_s(buf);

      if (!IS_SUPREME(ch)) {
        snprintf(arg1, sizeof(arg1), "QF %s", field_name);
        write_board(3097, arg1, buf);
      }

      if (amount != 0) {
        snprintf(buf, sizeof(buf), "You award $N with %d %s(s).", amount, friendly_name);
        act(buf, 0, ch, 0, victim, TO_CHAR);
        snprintf(buf, sizeof(buf), "$n awards you with %d %s(s).", amount, friendly_name);
        act(buf, 0, ch, 0, victim, TO_VICT);
        snprintf(buf, sizeof(buf), "$n awards $N with %d %s(s).", amount, friendly_name);
        act(buf, 0, ch, 0, victim, TO_NOTVICT);
      }
      else if (big_amount != 0) {
        snprintf(buf, sizeof(buf), "You award $N with %lld %s(s).", big_amount, friendly_name);
        act(buf, 0, ch, 0, victim, TO_CHAR);
        snprintf(buf, sizeof(buf), "$n awards you with %lld %s(s).", big_amount, friendly_name);
        act(buf, 0, ch, 0, victim, TO_VICT);
        snprintf(buf, sizeof(buf), "$n awards $N with %lld %s(s).", big_amount, friendly_name);
        act(buf, 0, ch, 0, victim, TO_NOTVICT);
      }

      switch (function) {
        case FUNCT_QP:
          GET_QP(victim) += amount;
          break;
        case FUNCT_SCP:
          GET_SCP(victim) += amount;
          break;
        case FUNCT_GOLD:
          GET_GOLD(victim) += amount;
          break;
        case FUNCT_EXP:
          GET_EXP(victim) += amount;
          break;
        case FUNCT_REMORT_EXP:
          GET_REMORT_EXP(victim) += big_amount;

          if (GET_REMORT_EXP(victim) && !enchanted_by_type(victim, ENCHANT_REMORTV2)) {
            printf_to_char(ch, "%s's remort enchantment turned ON.\n\r", GET_NAME(victim));

            rv2_add_enchant(victim);
          }
          else {
            printf_to_char(ch, "%s's remort enchantment turned OFF.\n\r", GET_NAME(victim));

            rv2_remove_enchant(victim);
          }
          break;
      }
      break;
    default:
      send_to_char("\
Usage: qf where (location of questers)\n\r\
          transfer   <char>/<all> (relocate quester(s) to you)\n\r\
          give       <argument>   (give all questers items)\n\r\
          quieton    <char>/<all> (turn on quester quiet flag)\n\r\
          quietoff   <char>/<all> (turn off quester quiet flag)\n\r\
          kickout    <char>/<all> (turn off quester quest flag)\n\r\
          qflag      <char>/<all> (turn on quester quest flag)\n\r\
          chaotic    <obj>/<all>  (assign chaotic flag to obj)\n\r\
          log        <obj>/<all>  (turn on quest logging for obj)\n\r\
          teleport                (random transport around the world)\n\r\
          sbreset                 (resets scoreboard - for pkill quests)\n\r\
                     Use SCOREBOARD to see death list (limit 99 deaths)\n\r\
          spread     <num> <vnum> (random spreading of objs)\n\r\
          zone       <normal>/<never>/<doors> (chaos zone reset modes)\n\r\
          token      <char>       (reward the character with a subclass token)\n\r\
          qcard      load <amt>   (loads the # of quest cards)\n\r\
          qcard      collect      (pulls all qcards in the game to inventory)\n\r\
          qinfo                 (turns on/off info flag - see quester actions)\n\r\
          qp/award   <char> <amt> (award the character # of quest points)\n\r\
          scp        <char> <amt> (award the character # of subclass points)\n\r\
          gold       <char> <amt> (award the character # of gold coins)\n\r\
          exp        <char> <amt> (award the character # of experience points)\n\r\
          remort_exp <char> <amt> (award the character # of remort experience points)\n\r\n\r\
Using most of the commands without the arguments give more information.\n\r",ch);
      return;
      break;
  }
}


void do_scoreboard(CHAR *ch, char *argument, int cmd) {
  int i;

  if(number_of_kills<1) {
    send_to_char("There are no kills currently listed.\n\r",ch);
    return;
  }

  send_to_char("***** CHAOS KILLS *****\n\r\n\r",ch);
  for(i=1;i<=number_of_kills;i++) {
    printf_to_char(ch,"%d) %s killed by %s on %s in %s.\n\r",i,
                       scores[i].killed,scores[i].killer,
                       scores[i].time_txt,scores[i].location);
  }
}

