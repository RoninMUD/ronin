/*
** aquest.c
**   Routines for autoquester
**
** Written for RoninMUD by Ranger
**
** Do not distribute without permission.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

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
#include "subclass.h"
#include "aquest.h"

extern void mob_do(CHAR* mob, char *arg);

int check_guildmaster(CHAR *ch, CHAR *mob) {
  if(!ch) return TRUE;
  if(!IS_NPC(ch) && ((GET_CLASS(mob)==CLASS_AVATAR) || (GET_CLASS(ch)==GET_CLASS(mob)))) return TRUE;
  act("$N tells you 'Go Away! I don't deal with your type!'",FALSE,ch,0,mob,TO_CHAR);
  return FALSE;
}

#define QUEST_QUIT     1
#define QUEST_REQUEST  2
#define QUEST_COMPLETE 3
#define QUEST_STATUS   4
#define QUEST_LIST     5
#define QUEST_BUY      6
#define QUEST_CARD     7
#define QUEST_ORDER    8

#define AQCARDS_SPREAD 25

const int aq_card[] = {//should this be in constants.c?
 2, /* 1 aq point */
 4, /* 2 aq point */
 6, /* 3 aq point */
 8, /* 4 aq point */
 10, /* 5 aq point */
};

int generate_id(void);

void aqcard_cleanup(int id)
{
  OBJ *tmp_obj;
  for(tmp_obj = object_list; tmp_obj; tmp_obj = tmp_obj->next)
  {
    if(V_OBJ(tmp_obj) != 35) continue; //not a questcard? skip
    if(OBJ_SPEC(tmp_obj) != id) continue; //not the char's questcard? skip
    extract_obj(tmp_obj);
  }
  return;
}

void do_aquest(CHAR *ch, char *argument, int cmd) {
  char arg[MAX_INPUT_LENGTH];
  char usage[]="\
This command is used to handle automatic questing from guildmasters.\n\r\n\r\
  Usage: aquest request  - request a quest (solo/low/mid/high optional)\n\r\
                card <#> - request a questcard hunt (# options: 1/2/3/4/5)\n\r\
                complete - get credit for the quest\n\r\
                info     - get info on current quest status\n\r\
                quit     - quit your current quest\n\r\
                order    - request an order (solo/low/mid/high optional)\n\r\
                list     - list things to buy\n\r\
                buy      - buy things with quest points\n\r";

  int option = 0;

  argument = one_argument(argument, arg);

  if (!*arg) {
    send_to_char(usage, ch);
    return;
  }

  if (is_abbrev(arg, "quit")) option = QUEST_QUIT;
  if (is_abbrev(arg, "request")) option = QUEST_REQUEST;
  if (is_abbrev(arg, "complete")) option = QUEST_COMPLETE;
  if (is_abbrev(arg, "info")) option = QUEST_STATUS;
  if (is_abbrev(arg, "list")) option = QUEST_LIST;
  if (is_abbrev(arg, "buy")) option = QUEST_BUY;
  if (is_abbrev(arg, "card")) option = QUEST_CARD;
  if (is_abbrev(arg, "order")) option = QUEST_ORDER;

  switch (option) {
    case QUEST_STATUS:
      if (ch->quest_status == QUEST_NONE) {
        send_to_char("You are not currently on any quest.\n\r", ch);
        if (ch->ver3.time_to_quest)
          printf_to_char(ch, "You must wait %d ticks to start another.\n\r", ch->ver3.time_to_quest);
        else
          printf_to_char(ch, "You can start another at any time.\n\r");
        return;
      }

      if (ch->quest_status == QUEST_RUNNING) {
        if (ch->questmob) {
          printf_to_char(ch, "You are on a quest to kill %s, for %d point(s).  You have %d ticks left.\n\r", GET_SHORT(ch->questmob), ch->quest_level, ch->ver3.time_to_quest);
          printf_to_char(ch, "%s can be found around %s.\n\r", GET_SHORT(ch->questmob), real_room(ch->quest_room_v) > 0 ? world[real_room(ch->quest_room_v)].name : "Unknown");
        }
        else if (ch->questobj) {
          if (V_OBJ(ch->questobj) == 35)
            printf_to_char(ch, "You are on a quest to recover %d %s, for %d point(s). You have %d ticks left.\n\r", aq_card[ch->quest_level - 1], OBJ_SHORT(ch->questobj), ch->quest_level, ch->ver3.time_to_quest);
          else
            printf_to_char(ch, "You are on a quest to recover %s, for %d point(s). You have %d ticks left.\n\r", OBJ_SHORT(ch->questobj), ch->quest_level, ch->ver3.time_to_quest);
        }
        else
          printf_to_char(ch, "You are on a quest to do something, dunno what it is.\n\r");
        return;
      }

      if (ch->quest_status == QUEST_COMPLETED) {
        send_to_char("You have completed your quest.  Return to the quest giver for credit.\n\r", ch);
        return;
      }

      if (ch->quest_status == QUEST_FAILED) {
        send_to_char("You have failed your quest for some reason.\n\r", ch);
        if (ch->ver3.time_to_quest)
          printf_to_char(ch, "You must wait %d ticks to start another.\n\r", ch->ver3.time_to_quest);
        else
          printf_to_char(ch, "You can start another at any time.\n\r");
        return;
      }
      break;
    case QUEST_QUIT:
      if (ch->quest_status == QUEST_NONE)
        send_to_char("You aren't on any quest.\n\r", ch);
      else if (ch->quest_status == QUEST_FAILED) {
        ch->questgiver = 0;
        if (ch->questobj)
          ch->questobj->owned_by = 0;
        ch->questobj = 0;
        if (ch->questmob)
          ch->questmob->questowner = 0;
        ch->questmob = 0;
        ch->quest_status = QUEST_NONE;
        ch->quest_level = 0;
        printf_to_char(ch, "You have failed your quest, you can start another in %d ticks.\n\r", ch->ver3.time_to_quest);
      }
      else {
        ch->questgiver = 0;
        if (ch->questobj) {
          if (V_OBJ(ch->questobj) == 35)
            aqcard_cleanup(ch->ver3.id);
          else
            ch->questobj->owned_by = 0;
        }
        ch->questobj = 0;
        if (ch->questmob)
          ch->questmob->questowner = 0;
        ch->questmob = 0;
        ch->quest_status = QUEST_NONE;
        ch->quest_level = 0;
#ifndef TESTSITE
        if (GET_LEVEL(ch) < LEVEL_IMM)
          ch->ver3.time_to_quest = MAX(ch->ver3.time_to_quest - 40, 5);
        else
          ch->ver3.time_to_quest = 0;
#else
        ch->ver3.time_to_quest = 0;
#endif
        printf_to_char(ch, "There, you have quit your quest, you can start another in %d ticks.\n\r", ch->ver3.time_to_quest);
      }
      return;
      break;
    case QUEST_COMPLETE:
    case QUEST_REQUEST:
    case QUEST_LIST:
    case QUEST_BUY:
    case QUEST_CARD:
      send_to_char("You must return to your guildmaster.\n\r", ch);
      return;
      break;
    case QUEST_ORDER:
      send_to_char("You'll have to find someone who has those.\n\r", ch);
      return;
      break;
    default:
      send_to_char(usage, ch);
      return;
      break;
  }
}

int stat_cost(CHAR *ch, int type) {
  int current;

  switch(type) {
    case APPLY_STR:
      if(GET_OSTR(ch)<18) return(0);
      if(GET_OADD(ch)<100) return(0);
      current=(GET_OSTR(ch)-18+1)*200;
      if(GET_OSTR(ch)>24) current=0; /* real limit to 25 */
      return(current);
      break;
    case APPLY_INT:
      if(GET_OINT(ch)<18) return(0);
      current=(GET_OINT(ch)-18+1)*200;
      if(GET_OINT(ch)>24) current=0; /* real limit to 25 */
      return(current);
      break;
    case APPLY_WIS:
      if(GET_OWIS(ch)<18) return(0);
      current=(GET_OWIS(ch)-18+1)*200;
      if(GET_OWIS(ch)>24) current=0; /* real limit to 25 */
      return(current);
      break;
    case APPLY_CON:
      if(GET_OCON(ch)<18) return(0);
      current=(GET_OCON(ch)-18+1)*200;
      if(GET_OCON(ch)>24) current=0; /* real limit to 25 */
      return(current);
      break;
    case APPLY_DEX:
      if(GET_ODEX(ch)<18) return(0);
      current=(GET_ODEX(ch)-18+1)*200;
      if(GET_ODEX(ch)>24) current=0; /* real limit to 25 */
      return(current);
      break;
    default:
      return(0);
  }
}

int generate_quest(CHAR *ch, CHAR *mob, int lh_opt);

int quest_giver(CHAR *mob,CHAR *ch, int cmd, char *argument) {
  char arg[MAX_INPUT_LENGTH],buf[MSL];
  int option=0,choice,lh_opt=0,cost;
  int i = 0, qc_count, target, qc_double = FALSE;
  OBJ *tmp_obj, *qcard;

  if(cmd!=CMD_AQUEST) return FALSE;
  if(!ch) return FALSE;

  argument=one_argument(argument,arg);
  if(!*arg) return FALSE;

  if(is_abbrev(arg,"request")) option=QUEST_REQUEST;
  if(is_abbrev(arg,"complete")) option=QUEST_COMPLETE;
  if(is_abbrev(arg,"list")) option=QUEST_LIST;
  if(is_abbrev(arg,"buy")) option=QUEST_BUY;
  if(is_abbrev(arg,"card")) option=QUEST_CARD;

  if(option && mob->specials.fighting) {
    send_to_char("Wait for the fighting to stop.\n\r",ch);
    return TRUE;
  }

  switch(option) {
    case QUEST_COMPLETE:
      if(!check_guildmaster(ch,mob)) return TRUE;
      if(ch->questgiver!=mob) {
        act("$N tells you, 'I didn't send you on any quest, try someone else.'",0,ch,0,mob,TO_CHAR);
        return TRUE;
      }
      if(ch->quest_status==QUEST_NONE) {
        act("$N tells you, 'You aren't on any quest.'",0,ch,0,mob,TO_CHAR);
        return TRUE;
      }
      if(ch->quest_status==QUEST_RUNNING && ch->questmob) {
        act("$N tells you, 'You're still working on a quest.'",0,ch,0,mob,TO_CHAR);
        return TRUE;
      }
      if(ch->quest_status==QUEST_FAILED) {
        act("$N tells you, 'You failed your quest for some reason. Perhaps you could try another soon.'",0,ch,0,mob,TO_CHAR);
        ch->questgiver=0;
        if(ch->questobj)
        {
          if(V_OBJ(ch->questobj) == 35)
            aqcard_cleanup(ch->ver3.id);
          else
            ch->questobj->owned_by=0;
        }
        ch->questobj=0;
        if(ch->questmob) ch->questmob->questowner=0;
        ch->questmob=0;
        ch->quest_status=QUEST_NONE;
        ch->quest_level=0;
        return TRUE;
      }
      if(ch->quest_status!=QUEST_COMPLETED && ch->questmob) {
        act("$N tells you, 'You haven't completed any quest.'",0,ch,0,mob,TO_CHAR);
        return TRUE;
      }
      act( "$n informs $N $e has completed $s quest.",0, ch, 0, mob, TO_NOTVICT);
      act("You inform $N you have completed $s quest.",0,ch, 0, mob, TO_CHAR);
      if(ch->questobj)
      {
        if(V_OBJ(ch->questobj) != 35)
        {/* for as of yet implemented single object questing */
          if(ch->questobj->equipped_by==ch) {
            act("$N tells you, 'Remove the $q and try again.'",0,ch,ch->questobj,mob,TO_CHAR);
            return TRUE;
          }
          if(ch->questobj->carried_by!=ch) {
            act("$N tells you, 'I don't seem to see the $q.'",0,ch,ch->questobj,mob,TO_CHAR);
            return TRUE;
          }
          if(ch->quest_level==1)
            sprintf(buf,"$N tells you, 'Thanks for the $q, you get %d quest point.'",ch->quest_level);
          else
            sprintf(buf,"$N tells you, 'Thanks for the $q, you get %d quest points.'",ch->quest_level);
          act(buf,0,ch,ch->questobj,mob,TO_CHAR);
          if(ch->quest_level==1)
            sprintf(buf,"$N tells $n, 'Thanks for the $q, you get %d quest point.'",ch->quest_level);
          else
            sprintf(buf,"$N tells $n, 'Thanks for the $q, you get %d quest points.'",ch->quest_level);
          act(buf,0,ch,ch->questobj,mob,TO_NOTVICT);
          extract_obj(ch->questobj);
        }
        else
        {/* for auto-questcard hunting */
          qc_count = 0;
          for(tmp_obj = ch->carrying; tmp_obj; tmp_obj = tmp_obj->next_content)
          {
            if(V_OBJ(ch->questobj) == V_OBJ(tmp_obj) && OBJ_SPEC(tmp_obj) == ch->ver3.id)
              qc_count++;
          }

          if(qc_count < aq_card[ch->quest_level-1])
          {
            sprintf(buf,"$N tells you, 'Hrm, you still need %d more of your questcards, keep looking!'", aq_card[ch->quest_level-1] - qc_count);
            act(buf, 0, ch, 0, mob, TO_CHAR);
            return TRUE;
          }
          else
          {
            if(chance(3))
            {
              ch->quest_level *= 2; /* double value of the quest */
              qc_double = TRUE;
            }

            if(ch->quest_level==1)
              sprintf(buf, "$N tells you, 'Splendid! I'll take those off your hands, you get %d quest point.'", ch->quest_level);
            else
            {
              if(qc_double)
                sprintf(buf, "$N tells you, 'I'm feeling generous, double points for you! You get %d quest points.'", ch->quest_level);
              else
                sprintf(buf, "$N tells you, 'Splendid! I'll take those off your hands, you get %d quest points.'", ch->quest_level);
            }
            act(buf, 0, ch, ch->questobj, mob, TO_CHAR);
            act("$n dexterously snags the questcards from your inventory.", 0, mob, 0, ch, TO_VICT);
            act("$n dexterously snags the questcards from $N's inventory.", 0, mob, 0, ch, TO_NOTVICT);
            aqcard_cleanup(ch->ver3.id);
          }
        }
      }
      else
      {/* for mob aqs */
        if(ch->quest_level==1)
          sprintf(buf,"$N tells you, 'Congratulations, you get %d quest point.'",ch->quest_level);
        else
          sprintf(buf,"$N tells you, 'Congratulations, you get %d quest points.'",ch->quest_level);
        act(buf,0,ch,0,mob,TO_CHAR);
        if(ch->quest_level==1)
          sprintf(buf,"$N tells $n, 'Congratulations, you get %d quest point.'",ch->quest_level);
        else
          sprintf(buf,"$N tells $n, 'Congratulations, you get %d quest points.'",ch->quest_level);
        act(buf,0,ch,0,mob,TO_NOTVICT);
      }
      sprintf(buf,"%s has completed %s quest!",GET_NAME(ch),HSHR(ch));
      do_quest(mob,buf,CMD_QUEST);

      ch->ver3.quest_points+=ch->quest_level;
      ch->questgiver=0;
      if(ch->questobj)
        ch->questobj->owned_by=0;
      ch->questobj=0;
      if(ch->questmob) ch->questmob->questowner=0;
      ch->questmob=0;
      ch->quest_status=QUEST_NONE;
      ch->quest_level=0;
      ch->ver3.time_to_quest= MAX(ch->ver3.time_to_quest - 40, 5);
      return TRUE;
      break;
    case QUEST_REQUEST:
      if(!check_guildmaster(ch,mob)) return TRUE;
      if(ch->quest_status==QUEST_COMPLETED) {
        act("$N tells you, 'You should get credit for your completed quest first.'",0,ch,0,mob,TO_CHAR);
        return TRUE;
      }
      if(ch->questgiver || ch->questobj || ch->questmob || ch->quest_status==QUEST_RUNNING) {
        act("$N tells you, 'You are already on a quest.'",0,ch,0,mob,TO_CHAR);
        return TRUE;
      }
      if(ch->quest_status!=QUEST_NONE && ch->quest_status!=QUEST_FAILED) {
        act("$N tells you, 'For some reason I can't give you a quest right now.'",0,ch,0,mob,TO_CHAR);
        return TRUE;
      }
      if(ch->ver3.time_to_quest) {
        sprintf(buf,"$N tells you, 'You must wait %d ticks before starting another quest.'",ch->ver3.time_to_quest);
        act(buf,0,ch,0,mob,TO_CHAR);
        return TRUE;
      }

      argument = one_argument(argument, arg);

      if (!*arg || is_abbrev(arg, "newbie")) lh_opt = 0;
      else if (is_abbrev(arg, "solo")) lh_opt = 1;
      else if (is_abbrev(arg, "low")) lh_opt = 2;
      else if (is_abbrev(arg, "high")) lh_opt = 3;
      else if (is_abbrev(arg, "mid")) lh_opt = 4;
      else {
        act("$N tells you, 'Sorry, I didn't quite get that.  Please use: aquest request newbie, solo, low, mid, or high.'", 0, ch, 0, mob, TO_CHAR);

        return TRUE;
      }

      if (!generate_quest(ch, mob, lh_opt)) {
        act("$N tells you, 'Sorry, I failed to find a quest for you.'", 0, ch, 0, mob, TO_CHAR);

        return TRUE;
      }

      return TRUE;
      break;

    case QUEST_LIST:
      if(!check_guildmaster(ch,mob)) return TRUE;
      act("$n asks $N for a list of quest items.",0,ch,0,mob,TO_ROOM);
      *buf = '\0';
      sprintf(buf + strlen(buf),"\
$N tells you, 'Current Quest Items available for Purchase:'\n\r\
     1) 1 subclass point - 5 quest points\n\r");
     if(!(cost=stat_cost(ch,APPLY_STR)))
       sprintf(buf + strlen(buf),"\
     2) Strength         - Unavailable\n\r");
     else
       sprintf(buf + strlen(buf),"\
     2) %d Strength      - %d quest points\n\r",GET_OSTR(ch)+1,cost);
     if(!(cost=stat_cost(ch,APPLY_INT)))
       sprintf(buf + strlen(buf),"\
     3) Intelligence     - Unavailable\n\r");
     else
       sprintf(buf + strlen(buf),"\
     3) %d Intelligence  - %d quest points\n\r",GET_OINT(ch)+1,cost);
     if(!(cost=stat_cost(ch,APPLY_WIS)))
       sprintf(buf + strlen(buf),"\
     4) Wisdom           - Unavailable\n\r");
     else
       sprintf(buf + strlen(buf),"\
     4) %d Wisdom        - %d quest points\n\r",GET_OWIS(ch)+1,cost);
     if(!(cost=stat_cost(ch,APPLY_DEX)))
       sprintf(buf + strlen(buf),"\
     5) Dexterity        - Unavailable\n\r");
     else
       sprintf(buf + strlen(buf),"\
     5) %d Dexterity     - %d quest points\n\r",GET_ODEX(ch)+1,cost);
     if(!(cost=stat_cost(ch,APPLY_CON)))
       sprintf(buf + strlen(buf),"\
     6) Constitution     - Unavailable\n\r");
     else
       sprintf(buf + strlen(buf),"\
     6) %d Constitution  - %d quest points\n\r",GET_OCON(ch)+1,cost);

      strcat(buf,"To buy an item, type AQUEST BUY <#>.");
      act(buf,0,ch,0,mob,TO_CHAR);
      return TRUE;
      break;

    case QUEST_BUY:
      if(!check_guildmaster(ch,mob)) return TRUE;
      one_argument(argument,arg);
      if(!*arg) {
        act("$N tells you, 'To buy an item type AQUEST BUY <#>'",0,ch,0,mob,TO_CHAR);
        return TRUE;
      }
      if(is_number(arg)) choice=atoi(arg);
      else {
        act("$N tells you, 'To buy an item type AQUEST BUY <#>'",0,ch,0,mob,TO_CHAR);
        return TRUE;
      }

      switch(choice) {
        case 1: /* 5 quest points = 1 subclass point */
          if(ch->ver3.quest_points<5) {
            act("$N tells you, 'You do not have enough quest points for that item.'",0,ch,0,mob,TO_CHAR);
            return TRUE;
          }
          ch->ver3.quest_points-=5;
          ch->ver3.subclass_points+=1;
          act("$N tells you, 'You just bought a subclass point.'",0,ch,0,mob,TO_CHAR);
          save_char(ch, NOWHERE);
          return TRUE;
          break;
        case 2: /* str */
          if(!(cost=stat_cost(ch,APPLY_STR))) {
            act("$N tells you, 'Increased strength is not available to you at this time.'",0,ch,0,mob,TO_CHAR);
            return TRUE;
          }
          if(ch->ver3.quest_points<cost) {
            act("$N tells you, 'You do not have enough quest points for that item.'",0,ch,0,mob,TO_CHAR);
            return TRUE;
          }
          ch->ver3.quest_points-=cost;
          GET_OSTR(ch)+=1;
          affect_total(ch);
          act("$N tells you, 'You just bought increased strength.'",0,ch,0,mob,TO_CHAR);
          save_char(ch, NOWHERE);
          return TRUE;
          break;
        case 3: /* int */
          if(!(cost=stat_cost(ch,APPLY_INT))) {
            act("$N tells you, 'Increased intelligence is not available to you at this time.'",0,ch,0,mob,TO_CHAR);
            return TRUE;
          }
          if(ch->ver3.quest_points<cost) {
            act("$N tells you, 'You do not have enough quest points for that item.'",0,ch,0,mob,TO_CHAR);
            return TRUE;
          }
          ch->ver3.quest_points-=cost;
          GET_OINT(ch)+=1;
          affect_total(ch);
          act("$N tells you, 'You just bought increased intelligence.'",0,ch,0,mob,TO_CHAR);
          save_char(ch, NOWHERE);
          return TRUE;
          break;
        case 4: /* wis */
          if(!(cost=stat_cost(ch,APPLY_WIS))) {
            act("$N tells you, 'Increased wisdom is not available to you at this time.'",0,ch,0,mob,TO_CHAR);
            return TRUE;
          }
          if(ch->ver3.quest_points<cost) {
            act("$N tells you, 'You do not have enough quest points for that item.'",0,ch,0,mob,TO_CHAR);
            return TRUE;
          }
          ch->ver3.quest_points-=cost;
          GET_OWIS(ch)+=1;
          affect_total(ch);
          act("$N tells you, 'You just bought increased wisdom.'",0,ch,0,mob,TO_CHAR);
          save_char(ch, NOWHERE);
          return TRUE;
          break;
        case 5: /* dex */
          if(!(cost=stat_cost(ch,APPLY_DEX))) {
            act("$N tells you, 'Increased dexterity is not available to you at this time.'",0,ch,0,mob,TO_CHAR);
            return TRUE;
          }
          if(ch->ver3.quest_points<cost) {
            act("$N tells you, 'You do not have enough quest points for that item.'",0,ch,0,mob,TO_CHAR);
            return TRUE;
          }
          ch->ver3.quest_points-=cost;
          GET_ODEX(ch)+=1;
          affect_total(ch);
          act("$N tells you, 'You just bought increased dexterity.'",0,ch,0,mob,TO_CHAR);
          save_char(ch, NOWHERE);
          return TRUE;
          break;
        case 6: /* con */
          if(!(cost=stat_cost(ch,APPLY_CON))) {
            act("$N tells you, 'Increased constitution is not available to you at this time.'",0,ch,0,mob,TO_CHAR);
            return TRUE;
          }
          if(ch->ver3.quest_points<cost) {
            act("$N tells you, 'You do not have enough quest points for that item.'",0,ch,0,mob,TO_CHAR);
            return TRUE;
          }
          ch->ver3.quest_points-=cost;
          GET_OCON(ch)+=1;
          affect_total(ch);
          act("$N tells you, 'You just bought increased constitution.'",0,ch,0,mob,TO_CHAR);
          save_char(ch, NOWHERE);
          return TRUE;
          break;
        default:
          act("$N tells you, 'That isn't a valid item number.'",0,ch,0,mob,TO_CHAR);
          return TRUE;
          break;
      }
      return TRUE;
      break;

    case QUEST_CARD:
      if(!check_guildmaster(ch,mob)) return TRUE;
      if(ch->quest_status==QUEST_COMPLETED) {
        act("$N tells you, 'You should get credit for your completed quest first.'", 0, ch, 0, mob, TO_CHAR);
        return TRUE;
      }
      if(ch->questgiver || ch->questobj || ch->questmob || ch->quest_status==QUEST_RUNNING) {
        act("$N tells you, 'You are already on a quest.'", 0, ch, 0, mob, TO_CHAR);
        return TRUE;
      }
      if(ch->quest_status!=QUEST_NONE && ch->quest_status!=QUEST_FAILED) {
        act("$N tells you, 'For some reason I can't give you a quest right now.'", 0, ch, 0, mob, TO_CHAR);
        return TRUE;
      }
      if(ch->ver3.time_to_quest) {
        sprintf(buf,"$N tells you, 'You must wait %d ticks before starting another quest.'", ch->ver3.time_to_quest);
        act(buf, 0, ch, 0, mob, TO_CHAR);
        return TRUE;
      }

      argument=one_argument(argument,arg);
      if(!*arg)
      {
        act("$N tells you, 'To request a questcard hunt type AQUEST CARD <#>'", 0, ch, 0, mob, TO_CHAR);
        return TRUE;
      }
      if(is_number(arg))
      {
        choice=atoi(arg);
        if(choice < 1 || choice > 5)
        {
          act("$N tells you, 'The value of the quest must be between 1 and 5.'", 0, ch, 0, mob, TO_CHAR);
          return TRUE;
        }
      }
      else
      {
        act("$N tells you, 'To request a questcard hunt type AQUEST CARD <#>'", 0, ch, 0, mob, TO_CHAR);
        return TRUE;
      }
      for(i = 0; i < AQCARDS_SPREAD; i++)
      {
        do
        {
          target = number(0, top_of_world);
        }
        while (IS_SET(world[target].room_flags, PRIVATE) ||
               IS_SET(world[target].room_flags, DEATH) ||
               IS_SET(world[target].room_flags, LOCK) ||
               IS_SET(world[target].room_flags, HAZARD) ||
               world[target].zone == world[real_room(3000)].zone ||  /* Northern Midgaard Update */
               world[target].zone == world[real_room(0)].zone || /* Void & Limbo */
               world[target].zone == world[real_room(1212)].zone || /* Immort */
               world[target].zone == world[real_room(17500)].zone || /* Red Dragon Volcano */
               world[target].zone == world[real_room(25300)].zone || /* Hell 1 */
               world[target].zone == world[real_room(25400)].zone || /* Hell 2 */
               world[target].zone == world[real_room(25500)].zone || /* Hell 3 */
               world[target].zone == world[real_room(27500)].zone || /* Clan Halls */
               world[target].zone == world[real_room(26100)].zone || /* Quest Zone Not connected */
               world[target].zone == world[real_room(3601)].zone || /* Cafe, by Jarldian */
               world[target].zone == world[real_room(6600)].zone || /* NewbieMist */
               world[target].zone == world[real_room(27800)].zone || /* ISAHall */
               world[target].zone == world[real_room(28700)].zone || /* Immortal Village, by Nosferatu */
               world[target].zone == world[real_room(26000)].zone || /* Quest Zone - not connected */
               world[target].zone == world[real_room(26200)].zone); /* Quest Zone - not connected */

        qcard = read_object(35, VIRTUAL);
        if(ch->ver3.id <= 0)
          ch->ver3.id = generate_id();
        OBJ_SPEC(qcard) = ch->ver3.id;      //tag the item with the char's ID
        sprintf(buf, "questcard %s", GET_NAME(ch));
        qcard->name = str_dup(buf);        //tag the item with the char's name as a keyword
        sprintf(buf, "a questcard for %s", GET_NAME(ch));
        qcard->short_description = str_dup(buf); //change the short desc to include char's name
        sprintf(buf, "A questcard for %s is lying on the ground.", GET_NAME(ch));
        qcard->description = str_dup(buf);    //change the long desc to include char's name
        qcard->log = 1;
        obj_to_room(qcard, target);
     }
     ch->quest_level = choice;
     ch->questmob = 0;
     ch->questobj = qcard;
     ch->quest_status=QUEST_RUNNING;
     ch->questgiver = mob;
     ch->ver3.time_to_quest = 60;
     sprintf(buf,"$N tells you, 'Collect %d personalized questcards for %d quest point(s), in %d ticks.'", aq_card[ch->quest_level-1], ch->quest_level, ch->ver3.time_to_quest);
     act(buf, 0, ch, 0, mob, TO_CHAR);
     return TRUE;
     break;
  }

  return FALSE;
}


const int aq_mob_master_list[][2] = {
  // { VNum, Level } // <name>
  { 110, 0 }, /* chief sprite */
  { 1100, 0 }, /* elven wizard */
  { 1116, 0 }, /* elven warrior */
  { 1117, 0 }, /* dwarven prince */
  { 1303, 0 }, /* gargon */
  { 2306, 0 }, /* white king */
  { 2315, 0 }, /* black king */
  { 2634, 0 }, /* zoo keeper zookeeper head */
  { 2645, 0 }, /* alligator gator crocodile creature swamp */
  { 2807, 0 }, /* negress */
  { 2912, 0 }, /* fire newt leader */
  { 2915, 0 }, /* phoenix */
  { 2916, 0 }, /* lizard  */
  { 3730, 0 }, /* sister */
  { 4515, 0 }, /* troll large crab herder */
  { 4517, 0 }, /* tilik small pixie */
  { 4704, 0 }, /* Halfling Zombie */
  { 4809, 0 }, /* bright wizard */
  { 4811, 0 }, /* grey wizard */
  { 5303, 0 }, /* captain */
  { 6000, 0 }, /* john lumberjack */
  { 6114, 0 }, /* queen spider */
  { 6115, 0 }, /* shargugh brownie */
  { 7009, 0 }, /* gnoll chieftain */
  { 7315, 0 }, /* bandit leader */
  { 7324, 0 }, /* nomad male */
  { 7334, 0 }, /* fido dog */
  { 7401, 0 }, /* guard beast */
  { 8206, 0 }, /* whale */
  { 8321, 0 }, /* boy cabin john thomas */
  { 8611, 0 }, /* prince */
  { 8901, 0 }, /* king welmar */
  { 9619, 0 }, /* unicorn */
  { 10008, 0 }, /* gargoyle */
  { 11310, 0 }, /* master burgonmaster */
  { 11607, 0 }, /* sailor elven */
  { 20805, 0 }, /* Osquip */
  { 20811, 0 }, /* Repairman Jocko */
  { 20821, 0 }, /* Fungus King */
  { 506, 1 }, /* Farmer */
  { 508, 1 }, /* Tea Master */
  { 542, 1 }, /* tengu leaflet */
  { 702, 1 }, /* small fairy */
  { 805, 1 }, /* Mummy */
  { 806, 1 }, /* mummy */
  { 807, 1 }, /* mummy */
  { 1307, 1 }, /* overlord */
  { 1602, 1 }, /* turtle giant sea */
  { 1604, 1 }, /* chimp chmipanzee */
  { 1605, 1 }, /* ape giant */
  { 1909, 1 }, /* orak */
  { 1926, 1 }, /* dragon black */
  { 1933, 1 }, /* zyekian warder lorin */
  { 2023, 1 }, /* scorpio */
  { 2027, 1 }, /* pisces mermaid */
  { 2103, 1 }, /* merchant */
  { 2510, 1 }, /* rabbit (cute rabbit) */
  { 3405, 1 }, /* priest */
  { 4060, 1 }, /* captain orc orcish */
  { 4108, 1 }, /* mage orc arch orcish wizard */
  { 4109, 1 }, /* nim orc orcish statue */
  { 4410, 1 }, /* Gatekeeper Guard */
  { 4435, 1 }, /* milhouse star roach */
  { 4437, 1 }, /* Rizzo the cook */
  { 4513, 1 }, /* goblin grug pale creature */
  { 4806, 1 }, /* averland */
  { 4807, 1 }, /* ostermark */
  { 4808, 1 }, /* middenheim */
  { 4826, 1 }, /* queen cow matriarch bovine statue */
  { 5158, 1 }, /* rock elemental */
  { 5407, 1 }, /* stheno gorgon */
  { 5408, 1 }, /* euryale gorgon */
  { 5409, 1 }, /* medusa gorgon */
  { 6110, 1 }, /* tree ancient */
  { 7002, 1 }, /* ripper statue */
  { 7336, 1 }, /* master light */
  { 7402, 1 }, /* beast (The Beast) */
  { 7503, 1 }, /* pirate observer */
  { 8203, 1 }, /* neptune */
  { 8317, 1 }, /* pirate captain lost */
  { 8335, 1 }, /* Grand Poobah */
  { 8401, 1 }, /* aldor warlock */
  { 8506, 1 }, /* worm sand */
  { 8508, 1 }, /* lost fisherman */
  { 8509, 1 }, /* kingfisher bird */
  { 8607, 1 }, /* knight */
  { 8615, 1 }, /* rabbit */
  { 9010, 1 }, /* elf chief elven guard */
  { 9012, 1 }, /* elf elven champion master */
  { 9505, 1 }, /* wolf queen */
  { 10318, 1 }, /* monk grand master human */
  { 10412, 1 }, /* tephonal son prince */
  { 11327, 1 }, /* jander vampire */
  { 11617, 1 }, /* elven mystic */
  { 11706, 1 }, /* ninja guardian */
  { 12022, 1 }, /* oak stoned huge */
  { 14001, 1 }, /* apparition girl marianne */
  { 14002, 1 }, /* apparition girl susanne */
  { 14003, 1 }, /* apparition girl annette */
  { 14518, 1 }, /* pixie glittering dust */
  { 16569, 1 }, /* werner curator */
  { 16690, 1 }, /* Aerial servant */
  { 16904, 1 }, /* spectre spirit trembling */
  { 20857, 1 }, /* Supreme Slug */
  { 26475, 1 }, /* protector */
  { 27113, 1 }, /* cannibal warrior */
  { 27651, 1 }, /* king monkey monkeys */
  { 27662, 1 }, /* blood goddess */
  { 204, 2 },  /* bugbear chief humanoid */
  { 403, 2 }, /* kobold supervisor giant */
  { 555, 2 }, /* Oni lesser */
  { 596, 2 }, /* monk priest sohei */
  { 713, 2 }, /* troll mother */
  { 714, 2 }, /* troll mother */
  { 715, 2 }, /* troll mother */
  { 716, 2 }, /* troll mother */
  { 804, 2 }, /* Tut */
  { 922, 2 }, /* leader rebel zan */
  { 1915, 2 }, /* thrag beastman master */
  { 2029, 2 }, /* bear ursa major */
  { 2030, 2 }, /* cassiopeia queen */
  { 2031, 2 }, /* cepheus king */
  { 2032, 2 }, /* polaris polar star */
  { 2809, 2 }, /* doctor */
  { 4110, 2 }, /* morian protector guard guardian */
  { 4484, 2 }, /* first lieutenant jobs */
  { 4516, 2 }, /* toxic young black dragon */
  { 5414, 2 }, /* aeacus */
  { 5415, 2 }, /* radamanthus */
  { 5416, 2 }, /* minos */
  { 5418, 2 }, /* hecate */
  { 5420, 2 }, /* hades king */
  { 5425, 2 }, /* mass blob */
  { 5593, 2 }, /* black skeleton */
  { 5705, 2 }, /* golem statue stone akinra */
  { 7501, 2 }, /* pirate first mate */
  { 8360, 2 }, /* alchemist mad yuri madman */
  { 8406, 2 }, /* dragon */
  { 9504, 2 }, /* wolf king */
  { 10017, 2 }, /* Calcifer */
  { 10417, 2 }, /* genschleng were crocodile werecrocodile */
  { 11324, 2 }, /* soth lord */
  { 11504, 2 }, /* fei lien */
  { 11507, 2 }, /* shang-ti */
  { 12003, 2 }, /* ant gatemaster guard */
  { 12008, 2 }, /* dragon guard */
  { 12014, 2 }, /* cobra king */
  { 12023, 2 }, /* stone big */
  { 12026, 2 }, /* ancient crocodile cro */
  { 12209, 2 }, /* gigantic mushroom mus */
  { 12905, 2 }, /* mystic great */
  { 12911, 2 }, /* kansatsu mystic brother elder */
  { 12914, 2 }, /* genshi mystic brother elder */
  { 12915, 2 }, /* kyoki mystic brother elder */
  { 12916, 2 }, /* yagyu mystic brother elder */
  { 13011, 2 }, /* owlbear */
  { 14000, 2 }, /* ghost father man figure */
  { 14502, 2 }, /* Yorla sayer truth hag old wretched */
  { 16803, 2 }, /* revenant form */
  { 16804, 2 }, /* hunter arctic man squat */
  { 17300, 2 }, /* crystal golem statue warrior */
  { 17301, 2 }, /* phasteus cat tiger */
  { 19011, 2 }, /* man-eater bear slagbjoern */
  { 20163, 2 }, /* priest zen */
  { 20183, 2 }, /* Sensei Ryo */
  { 20840, 2 }, /* Cloaker Lord */
  { 20901, 2 }, /* blarf Hero */
  { 20923, 2 }, /* tweef king Vandimar */
  { 20924, 2 }, /* tweef queen Larienne */
  { 21201, 2 }, /* cave ogre monster */
  { 21209, 2 }, /* terral castle guard */
  { 21218, 2 }, /* grydon guard gates castle */
  { 24005, 2 }, /* guardian tomb ancient statue */
  { 24900, 2 }, /* Dark Druid Guardian */
  { 26405, 2 }, /* statue demon */
  { 26476, 2 }, /* adept */
  { 26479, 2 }, /* devotee */
  { 27100, 2 }, /* Centurion */
  { 27109, 2 }, /* tyrannosaurus rex */
  { 27112, 2 }, /* quazit */
  { 27699, 2 }, /* El Diablo */
  { 27700, 2 }, /* Mountain Yeti */
  { 28502, 2 }, /* minotaur bull man */
  { 701, 3 }, /* racti troll hermit */
  { 706, 3 }, /* juktoa troll foreman */
  //{ 1261, 3 }, /* salamander sal */
  //{ 2702, 3 }, /* marikith elder */
  { 3919, 3 }, /* celestial dragon */
  { 4447, 3 }, /* Gentle ben filthy inmate */
  { 4463, 3 }, /* doctor jacobs */
  { 4464, 3 }, /* gypsy alice chains prisoner */
  { 4465, 3 }, /* Gaarn were badger beast */
  { 4466, 3 }, /* drow drider */
  { 4483, 3 }, /* Captain Guard */
  { 4601, 3 }, /* gigantic eye */
  { 4605, 3 }, /* worm heart */
  { 4608, 3 }, /* nose hair */
  //{ 4706, 3 }, /* Garbage Golem */
  { 6201, 3 }, /* the Unholy Deacon */
  { 8361, 3 }, /* wind dust elemental */
  { 11712, 3 }, /* master pagoda */
  { 12005, 3 }, /* dragon ancient huge dra anc */
  { 12009, 3 }, /* lost adventurer */
  { 12021, 3 }, /* ettin wizard mage ett */
  { 12025, 3 }, /* huge purple worm */
  { 12201, 3 }, /* cleric ettin et cl */
  { 13017, 3 }, /* tree ant treeant */
  { 14509, 3 }, /* priest arak */
  { 16507, 3 }, /* ancient wax knight faze */
  { 16508, 3 }, /* wax knight bill janitor */  
  { 16513, 3 }, /* proto-horse proto horse wax */
  { 16526, 3 }, /* jenny consort girl */
  { 17001, 3 }, /* atropos doctor agent */
  { 17003, 3 }, /* sand beast pile */
  { 21109, 3 }, /* root tree large */
  { 21207, 3 }, /* aldrene bard lady singer */
  { 21223, 3 }, /* thief morian thug */
  { 21319, 3 }, /* ogre magi celil-gandil */
  { 21320, 3 }, /* ogre magi fumbor */
  { 21321, 3 }, /* ogre magi meldur */
  { 21322, 3 }, /* ogre magi anawyn */
  { 21334, 3 }, /* bugbear bug bear */
  //{ 23007, 3 }, /* hydra cryohydra snake */
  { 23009, 3 }, /* polar bear glacial */
  { 24903, 3 }, /* druid protector */
  { 26403, 3 }, /* guide */
  { 26404, 3 }, /* seeress mistress */
  { 26702, 3 }, /* ghost prisoner */
  { 27101, 3 }, /* roman tribune marcus aurelius */
  { 27102, 3 }, /* Damoclese the Gladiator */
  { 27106, 3 }, /* lord bundolo */
  { 27715, 3 }, /* gelugon guardian guard demon */
  { 20923, 3 }, /* tweef king vandimar */
  { 20924, 3 }, /* tweef queen larienne */
  { 209, 4 }, /* lookout vagabond scout leader */
  { 501, 4 }, /* Oni Greater */
  { 540, 4 }, /* Tanoshi Wrestler */
  { 552, 4 }, /* Sojobo tengu king */
  { 598, 4 }, /* shukenja grand priest */
  { 703, 4 }, /* neyuv lizard assassin reptile */
  { 4469, 4 }, /* animate skeleton */
  { 4472, 4 }, /* Voldra Sage */
  { 4612, 4 }, /* Kitzanti Captain Dark Purple */
  //{ 4703, 4 }, /* voodoo doll */
  //{ 4707, 4 }, /* Cleric Werra Garbage */
  { 5801, 4 }, /* slaphoff kender captain */
  { 6273, 4 }, /* ant lion */
  { 6298, 4 }, /* cave bear */
  { 7500, 4 }, /* pirate captain */
  { 7507, 4 }, /* cook old chinese */
  { 10907, 4 }, /* zyca */
  { 11702, 4 }, /* Chun Lui gong */
  { 12012, 4 }, /* king spider spi */
  { 12202, 4 }, /* thief ettin th et */
  { 12203, 4 }, /* rat giant */
  //{ 12811, 4 }, /* mankey pet */
  { 12904, 4 }, /* mystic ultimate */
  { 13501, 4 }, /* oglozt greater */
  { 14205, 4 }, /* fred gatekeeper */
  //{ 14508, 4 }, /* eduard magistrate wererat grotesque rat */
  { 16515, 4 }, /* gorgo fur beast */
  { 17005, 4 }, /* marten man */
  { 17006, 4 }, /* maerlyn sorcerer wizard */
  { 17007, 4 }, /* oracle spirit */
  { 17330, 4 }, /* dwarf smith prisoner ragar */
  { 20108, 4 }, /* Miyamoto Musashi */
  { 20129, 4 }, /* Spectral Warlord */
  { 20130, 4 }, /* warhorse skeletal */
  { 20165, 4 }, /* black panther */
  { 21203, 4 }, /* king morian moria ruler mandrial */
  { 25018, 4 }, /* elemental water prince */
  { 25019, 4 }, /* earth elemental king */
  { 25020, 4 }, /* air elemental lord */
  { 25021, 4 }, /* fire elemental sultan */
  { 25035, 4 }, /* demon balor */
  { 25040, 4 }, /* death lord */
  { 26401, 4 }, /* vizier */
  { 26481, 4 }, /* healer */
  { 27105, 4 }, /* cannibal witchdoctor */
  { 27720, 4 }, /* myrdon thief rogue master */
  { 27721, 4 }, /* shadowraith ninja assassin jal pur */
  { 28708, 4 }, /* achilles hero immortal */
  { 28709, 4 }, /* odysseus king hero immortal */
  { 28710, 4 }, /* perseus hero immortal */
  { 28711, 4 }, /* oedipus hero immortal */
  { 28712, 4 }, /* theseus king hero immortal */
  { 28714, 4 }, /* prometheus hero immortal */
  { 28716, 4 }, /* agamemnon king hero immortal */
  { 28717, 4 }, /* jason hero immortal */
  { 28718, 4 }, /* pandora immortal */
  { 700, 5 }, /* sakdul large troll gypsy */
  { 4600, 5 }, /* Neuron Beast Strands */
  { 5105, 5 }, /* drow apprentice */
  { 5106, 5 }, /* drow weaponmaster zarc */
  { 5107, 5 }, /* drow matron mother lower */
  { 5109, 5 }, /* illithid */
  { 5140, 5 }, /* spider sentry first */
  { 5177, 5 }, /* Dgarzah Drow rogue Leader */
  { 5184, 5 }, /* spider sentry third */
  { 5189, 5 }, /* spider sentry fourth */
  { 5191, 5 }, /* spider sentry second */
  { 5596, 5 }, /* myconid king mushroom */
  { 11326, 5 }, /* vampire strahd count */
  { 17004, 5 }, /* twixt bard man master */
  { 17010, 5 }, /* minion lesser */
  { 17308, 5 }, /* marcus wizard mage */
  { 20145, 5 }, /* Shogun Warlord Samurai */
  { 21204, 5 }, /* adrel sage magic */
  { 21205, 5 }, /* ulric advisor */
  { 21210, 5 }, /* priest high dark man */
  { 21323, 5 }, /* ogre sorcerer eowadriendir */
  { 21332, 5 }, /* otyugh stench garbage pile vines */
  { 25000, 5 }, /* demi lich */
  { 25002, 5 }, /* death crimson */
  { 25010, 5 }, /* kraken */
  { 25013, 5 }, /* kalas */
  { 26402, 5 }, /* emir malik */
  { 26482, 5 }, /* magus */
  { 27712, 5 }, /* bebilith stalker purple spider insect */
  { 19403, 5 }, /* mountaninous grolem */
//  { 19508, 5 }, /* difficult world elemental earth ruler*/
//  { 19509, 5 }, /* difficult world elemental fire ruler */
//  { 19510, 5 }, /* difficult world elemental air ruler */
//  { 19511, 5 }, /* difficult world elemental lightning ruler */
//  { 19512, 5 }, /* difficult world elemental water ruler */
//  { 10502, 6 }, /*tarion cavalier*/
  { 5103, 6 }, /* drow arch-mage mage */
  { 5125, 6 }, /* drow matron mother third */
  { 5126, 6 }, /* drow matron mother fourth */
  { 5127, 6 }, /* drow matron mother second */
  { 5132, 6 }, /* drow leader varrn */
  { 5553, 6 }, /* Garaek drow drider overseer */
  { 5574, 6 }, /* colossal wyrm */
  { 5901, 6 }, /* drow leader rezik */
  { 7703, 6 }, /* typik lizard shaman reptile */
  { 13502, 6 }, /* demon reptilian reptile */
  //{ 14501, 6 }, /* keira banshee ghost */
  { 14503, 6 }, /* ardaan inquisitor warrior */
  //{ 11514, 6 }, /* wyvern */
  { 13019, 6 }, /* elf elven master beastmaster */
  { 17002, 6 }, /* vermilion king */
  { 17306, 6 }, /* pit fiend */
  { 17342, 6 }, /* troll cook chef */
  { 20107, 6 }, /* Raiden */
  //{ 23001, 6 }, /* remorhaz ice burrower */
  { 25001, 6 }, /* keftab */
  { 26583, 6 }, /* guru */
  { 26706, 6 }, /* creature large hideous mutated rat yeti human */
  { 26707, 6 }, /* observer tower mage old man */
  { 28700, 6 }, /* icarus immortal */
  //{ 27722, 6 }, /* shomed nomad hero tarion desert */
};

static bool aq_calc_skip = FALSE;

int generate_quest(CHAR *ch, CHAR *mob, int lh_opt) {
  static int skip_aq2 = 0, skip_aq4 = 0;

  /* For "mid", we want a roughly even number of 2, 3 and 4 point mobs, so one time per boot, calculate the percentage of 2 and/or 4 point mobs to skip and store these numbers in some static variables. */
  if (!aq_calc_skip) {
    int aq2 = 0, aq3 = 0, aq4 = 0;

    for (int i = 0; i < NUMELEMS(aq_mob_master_list); i++) {
      int aq_mob_quest_level = aq_mob_master_list[i][1];

      if (aq_mob_quest_level == 2) aq2++;
      else if (aq_mob_quest_level == 3) aq3++;
      else if (aq_mob_quest_level == 4) aq4++;
    }

    if ((aq2 > 0) && (aq2 > aq3)) skip_aq2 = ((1.0 - ((double)aq3 / (double)aq2)) * 100);
    if ((aq4 > 0) && (aq4 > aq3)) skip_aq4 = ((1.0 - ((double)aq3 / (double)aq4)) * 100);

    aq_calc_skip = TRUE;
  }

  int aq_mob_temp_list[NUMELEMS(aq_mob_master_list)][2];
  int aq_mob_num = 0;

  /* Construct a list of all eligible mob VNums based on the chosen quest level. */
  for (int i = 0; i < NUMELEMS(aq_mob_master_list); i++) {
    int aq_mob_vnum = aq_mob_master_list[i][0];
    int aq_mob_quest_level = aq_mob_master_list[i][1];

    if ((lh_opt == 0) && (GET_LEVEL(ch) < 25) && (aq_mob_quest_level > 0)) continue;  // newbie
    if ((lh_opt == 0) && (GET_LEVEL(ch) >= 25) && (aq_mob_quest_level < 1)) continue; // non-newbie
    if ((lh_opt == 1) && (aq_mob_quest_level > 1)) continue;                          // solo
    if ((lh_opt == 2) && (aq_mob_quest_level > 3)) continue;                          // low
    if ((lh_opt == 3) && (aq_mob_quest_level < 3)) continue;                          // high
    if ((lh_opt == 4) && ((aq_mob_quest_level < 2) ||
                          (aq_mob_quest_level > 4) ||
                          ((aq_mob_quest_level == 2 && chance(skip_aq2)) ||
                           (aq_mob_quest_level == 4 && chance(skip_aq4))))) continue; // mid

    aq_mob_temp_list[aq_mob_num][0] = aq_mob_vnum;
    aq_mob_temp_list[aq_mob_num][1] = aq_mob_quest_level;

    aq_mob_num++;
  }

  /* Shuffle the 2D array. We'll start picking from the head of the list below for our AQ mob. */
  shuffle_2d_int_array(aq_mob_temp_list, aq_mob_num);

  CHAR *quest_mob = NULL;
  int quest_level = -1;

  /* Search our list of eligibile VNums for an AQ mob, starting at the head of the shuffled 2D array. */
  for (int i = 0; i < aq_mob_num; i++) {
    int temp_aq_mob_vnum = aq_mob_temp_list[i][0];
    int temp_aq_mob_quest_level = aq_mob_temp_list[i][1];

    int temp_aq_mob_proto_index = real_mobile(temp_aq_mob_vnum);
    if (temp_aq_mob_proto_index < 0) continue; // This mob is not loaded in the world.

    int temp_aq_mob_num_in_game = mob_proto_table[temp_aq_mob_proto_index].number;
    if (temp_aq_mob_num_in_game < 1) continue; // Sanity check.

    CHAR *temp_aq_mob = NULL;

    if (temp_aq_mob_num_in_game == 1) {
      /* If there's only one in the game, we can just grab it from the list with get_ch_world(). */
      temp_aq_mob = get_ch_world(temp_aq_mob_vnum);

      /* This VNum was not eligible (it's already assigned to somebody, or this mob is already fighting someone), so continue the main loop. */
      if (!temp_aq_mob || temp_aq_mob->questowner || (GET_OPPONENT(temp_aq_mob) && !IS_NPC(GET_OPPONENT(temp_aq_mob)))) continue;
    }
    else {
      /* More than one AQ mob of this VNum exists. We'll need to see if the VNum is eligible and if so, get one of the mobs. */
      int temp_aq_mob_eligible_num = 0;

      /* Search the list of all characters and see if this VNum is eligible (nobody else is on a quest for it). */
      for (CHAR *temp_mob = character_list; temp_mob && (temp_aq_mob_eligible_num < temp_aq_mob_num_in_game); temp_mob = temp_mob->next) {
        if (temp_mob->nr_v != temp_aq_mob_vnum) continue; // Not the right VNum.
        if (GET_OPPONENT(temp_aq_mob) && !IS_NPC(GET_OPPONENT(temp_aq_mob))) continue; // This mob is already fighting someone.
        if (temp_mob->questmob_ineligible) continue; // This mob is not eligible to be a quest mob.

        /* This VNum was not eligible (it's already assigned to somebody, or the mob is already fighting a mortal), so break out of the loop. */
        if (temp_mob->questowner) {
          temp_aq_mob_eligible_num = 0;
          break;
        }

        temp_aq_mob_eligible_num++;
      }

      /* This VNum was not eligible (it's already assigned to somebody), so continue the main loop. */
      if (temp_aq_mob_eligible_num < 1) continue;

      int pick = number(1, temp_aq_mob_eligible_num);
      int temp_aq_mob_num = 0;

      /* Search the list of all characters and get the actual mob that was picked among those eligible. */
      for (CHAR *temp_mob = character_list; temp_mob && (temp_aq_mob_num < pick); temp_mob = temp_mob->next) {
        if (temp_mob->nr_v != temp_aq_mob_vnum) continue;

        temp_aq_mob_num++;

        /* We found the mob we're looking for, so assign it to temp_aq_mob and break out of the loop. */
        if (temp_aq_mob_num == pick) {
          temp_aq_mob = temp_mob;
          break;
        }
      }
    }

    /* If we found what we're looking for, assign the mob and it's quest level, then break out of the main loop. */
    if (temp_aq_mob) {
      quest_mob = temp_aq_mob;
      quest_level = temp_aq_mob_quest_level;
      break;
    }
  }

  /* We found something and we're ready to give the player a quest. */
  if (quest_mob && quest_level >= 0) {
    quest_mob->questowner = ch;

    ch->questgiver = mob;
    ch->questobj = 0;
    ch->questmob = quest_mob;
    ch->quest_level = (quest_level == 0 ? 1 : quest_level);
    ch->quest_status = QUEST_RUNNING;
    ch->quest_room_v = CHAR_VIRTUAL_ROOM(quest_mob);
    ch->ver3.time_to_quest = 60;

    char buf[MSL];

    snprintf(buf, sizeof(buf), "$N tells you, 'Kill %s for %d quest point(s), in 60 ticks.'", GET_SHORT(quest_mob), ch->quest_level);
    act(buf, 0, ch, 0, mob, TO_CHAR);

    snprintf(buf, sizeof(buf), "$N tells you, '%s can be found around %s.'", GET_SHORT(quest_mob), world[real_room(ch->quest_room_v)].name);
    act(buf, 0, ch, 0, mob, TO_CHAR);

    int double_point_chance = 2;

    // Prestige Perk 19
    if (GET_PRESTIGE_PERK(ch) >= 19) {
      double_point_chance += 2;
    }

    if (chance(double_point_chance)) {
      act("$N tells you, 'Its your lucky day!  I'm going to double your quest point reward!'", 0, ch, 0, mob, TO_CHAR);

      ch->quest_level *= 2;
    }

    return TRUE;
  }

  return FALSE;
}

/* Object questing implemented through mob other than guildmaster in this case */
const int aq_obj_master_list[][2] = {
  // {VNum, Value} // <short desc> <repop>
  {101, 1}, // A pair of glasses 50
  {103, 1}, // a small hammer 50
  {104, 1}, // A small stethoscope 50
  {105, 1}, // A pink potion 50
  {106, 1}, // A small wand 50
  {200, 1}, // A curved rapier 15
  {508, 1}, // Porcelain Teacup 25
  {1101, 1}, // Sting 50
  {1106, 1}, // elven bow 50
  {1108, 1}, // the thain girth 50
  {1110, 1}, // an egg 50
  {1301, 1}, // An ivory headband 50
  {1302, 1}, // A ruby wand 50
  {1907, 1}, // a blackthorn shillelagh 50
  {1914, 1}, // a light sphere 50
  {2005, 1}, // An ugly skull of Draco 50
  {2020, 1}, // the Ring of Dignity 50
  {2305, 1}, // a white crosier 50
  {2310, 1}, // a black crosier 50
  {2502, 1}, // wooden spear 50
  {2506, 1}, // a black mask 50
  {2601, 1}, // giant wooden shield 20
  {2606, 1}, // A pair of alligator hide sleeves 19
  {2700, 1}, // a ragged cotton shirt 50
  {2702, 1}, // a small sharp knife 30
  {2806, 1}, // a book of songs 50
  {2809, 1}, // a book of maths 50
  {2811, 1}, // a book of jokes 50
  {2902, 1}, // A Goat's Milk Bladder 15
  {2913, 1}, // A Large Spiked Chain Mace 15
  {3414, 1}, // a whisper thin rapier 50
  {3425, 1}, // An auburn-colored orb 50
  {3701, 1}, // A Silver Necklace 20
  {4000, 1}, // A Yellow Ring 30
  {4051, 1}, // a metal helmet 35
  {4100, 1}, // A Black Demon's Blade 50
  {4516, 1}, // A Rusty Chain Shirt 50
  {4812, 1}, // Averland greatsword 50
  {4813, 1}, // Ostermark greathammer 50
  {4814, 1}, // Middenheim greatdagger 50
  {5302, 1}, // a small iron ball 50
  {5601, 1}, // a bearskin 50
  {5809, 1}, // bratwurst 50
  {6001, 1}, // a chequered shirt 50
  {6112, 1}, // an iron crown 50
  {6113, 1}, // an iron sceptre 50
  {6511, 1}, // A golden dwarven ring 50
  {7003, 1}, // An evil sabre 50
  {7310, 1}, // A Curved sword 50
  {8011, 1}, // sparkling lightening sword 50
  {8202, 1}, // a sealskin jacket 50
  {8323, 1}, // the Cabin Boy's Sponge 50
  {8333, 1}, // a seaweed loincloth 50
  {8407, 1}, // a steel bracelet 50
  {8503, 1}, // Feathers of the Kingfisher 50
  {8510, 1}, // Ragged Sleeves 50
  {8605, 1}, // a Helmet of the Black Knight 50
  {8613, 1}, // a Golden Harp 40
  {9414, 1}, // some platinum leg bands 50
  {9421, 1}, // a lodestone 50
  {9515, 1}, // A crystal staff 50
  {10302, 1}, // a string of prayer beads 50
  {10903, 1}, // a steel ring 50
  {11062, 1}, // a devil's trident 50
  {11068, 1}, // a black cape 50
  {11313, 1}, // a crystal ball 50
  {16620, 1}, // a flaming whip 40
  {16681, 1}, // Boots of Water Walking 50
  {19009, 1}, // a wool sweater 11
  {20820, 1}, // Stick of Fungus Powers 25
  {27621, 1}, // The necklace of shrunken heads 35
  {28506, 1}, // A Jeweled Tiara 60
  {709, 2}, // A Heavy Stone Club 46
  {804, 2}, // Tattered Gloves 30
  {904, 2}, // A Black Pair of Boots 80
  {908, 2}, // A Blood Red Cape 60
  {911, 2}, // The Whip of Oppression 75
  {1104, 2}, // a silvery cloak 10
  {1601, 2}, // a Turtle's Shell 15
  {1604, 2}, // a metal hoop 30
  {1605, 2}, // a Ring of Vines 15
  {1608, 2}, // a woven vine sash 20
  {2503, 2}, // a golden belt 10
  {2803, 2}, // a shinning necklace 10
  {3906, 2}, // Feather Sleeves 45
  {4053, 2}, // An Orcish Whip 20
  {4105, 2}, // Silver Gauntlets 30
  {4107, 2}, // A Marble Pendant 40
  {4400, 2}, // a long silk skirt 11
  {4454, 2}, // An Engraved Eagle Breastplate 30
  {5200, 2}, // A ringmail vest 50
  {5203, 2}, // Some iron sleeves 50
  {5208, 2}, // A sturdy iron shield 50
  {5427, 2}, // The shield of Pallas 50
  {5435, 2}, // a jeweled ring 50
  {5436, 2}, // a crown of dreams 50
  {5449, 2}, // a pair of sandals 50
  {5454, 2}, // the brassard of the hydra 50
  {5455, 2}, // A heavy metal wrist band 50
  {5713, 2}, // a dagger of slicing 50
  {5802, 2}, // Utility belt 25
  {7334, 2}, // An Obsidian shard. 10
  {8201, 2}, // golden trident 10
  {8321, 2}, // A Parrot's Skeleton 33
  {8322, 2}, // the Pirate Captain's Hat 15
  {8335, 2}, // A Rusted Iron Gauntlet 50
  {8403, 2}, // a chef's hat 25
  {8406, 2}, // a Dragon's Scale 50
  {8408, 2}, // a steel girdle 20
  {8513, 2}, // Fishing Line 50
  {8616, 2}, // a Goat's Skull 50
  {8624, 2}, // a Cat-O-Nine Tails 35
  {9004, 2}, // a Two-handed Mithril Sword 10
  {9307, 2}, // Gold Dragon Shield 50
  {9308, 2}, // A black leather belt with dangling skulls 50
  {9309, 2}, // A staff with arcane symbols 50
  {9321, 2}, // An Arcane Runed Robe 50
  {9521, 2}, // a broken ring 50
  {10000, 2}, // Pelt of Matted Fury 11
  {10008, 2}, // a silver crest 13
  {10405, 2}, // a Magical Eyeglass 65
  {10410, 2}, // Belt of the Class Hero 30
  {11069, 2}, // a pair of black pants 50
  {11072, 2}, // a black belt 40
  {11144, 2}, // a stiletto 35
  {11315, 2}, // cloak of sadness 10
  {11616, 2}, // a galvorn plate 50
  {12911, 2}, // a blue silk shirt 50
  {12932, 2}, // a human skull on a chain 18
  {13019, 2}, // a Heart-Shaped Amulet 20
  {14008, 2}, // a rosette bracelet 50
  {16637, 2}, // Berbalang's cloak 30
  //{19007, 2}, // a bear-tooth amulet 0
  {20851, 2}, // A toupe 50
  {20856, 2}, // a bracelet of slug entrails 30
  {26411, 2}, // lucky ivory die 30
  {27651, 2}, // the jade monkey 25
  {27682, 2}, // A purple robe 20
  //{27700, 2}, // An Ice Opal 25
  {517, 3}, // Shuriken 45
  {801, 3}, // Pharaoh's Death Mask 30
  {803, 3}, // Tattered Leggings 30
  {907, 3}, // A Set of Golden Bangles 40
  {910, 3}, // The Rebel's Breastplate 17
  {4109, 3}, // A Belt of the Undergrounds 9
  {4448, 3}, // a shield made from solid oak 18
  {4464, 3}, // iron shackles and chains 20
  {4465, 3}, // A Were-Badger's Mane 21
  {4477, 3}, // An Adamanite Scimitar 30
  {4508, 3}, // Boots of Striding 10
  {4510, 3}, // A Mushroom-Top Loincloth 10
  {4604, 3}, // the Finger of Thievery 10
  {6802, 3}, // leg plates of devotion 50
  {6803, 3}, // buckler of obeisance 50
  {7502, 3}, // a dueling pistol 50
  {7506, 3}, // a silk scarf 15
  {8337, 3}, // a long white coat 50
  {8402, 3}, // The Dagger of Aldor 17
  {8603, 3}, // The Holy Grail 10
  {8922, 3}, // a common recipe 3
  {10018, 3}, // lime-green jerkin 3
  {10021, 3}, // Wreath of Fire 13
  {10409, 3}, // Colossal Claw of the Greater Lycanthrope 10
  {10908, 3}, // a Zyca Orb 50
  {11316, 3}, // breast plate of Black rose 10
  {11507, 3}, // the Hammer of Lei Kung 10
  {11524, 3}, // The Plate Mail of Fei Lien 10
  {11533, 3}, // the Staff of Shang-Ti 10
  {11618, 3}, // The robe of Aman 10
  {12026, 3}, // Ring of Wizardry 50
  {12033, 3}, // Stone Shield 50
  {12035, 3}, // dragon skin leggings 50
  {12036, 3}, // dragon skin sleeves 50
  {12045, 3}, // glowing gem 50
  {12201, 3}, // Crystal Lantern 50
  {12202, 3}, // Crystal Shield 50
  {12206, 3}, // Ettin Leg Plates 50
  {12207, 3}, // Ettin Arm Plates 50
  {12209, 3}, // small black stone 50
  {12929, 3}, // a black silk hood 20
  {13018, 3}, // a wooden seed 20
  {13025, 3}, // An Oak-Leaf Net 50
  {13503, 3}, // crimson mask 30
  {13504, 3}, // Black bracelet of Kgozt 20
  {14027, 3}, // Spiked Hammer of Koraths 20
  {14028, 3}, // Shield of Ancient Powers 25
  {16507, 3}, // a bloody staff 35
  {16508, 3}, // An emerald muslin sash 43
  {16651, 3}, // a green potion 10
  {16660, 3}, // A huge stone hammer 10
  {16808, 3}, // an obsidian arrowhead 25
  {16809, 3}, // an atlatl 20
  {16807, 3}, // a tattered shroud 22
  {16902, 3}, // A Web-strewn Shield 7
  {16903, 3}, // A Web-strewn Plate 8
  {17021, 3}, // A jagged scalpel 30
  {17023, 3}, // Dune Boots 40
  {17300, 3}, // Frosted Bracelet 55
  {20147, 3}, // a Ninjato 100
  {20825, 3}, // Wings of the Cloaker Lord 20
  {21215, 3}, // Golden Dagger 29
  {21305, 3}, // A broken sword 30
  {24904, 3}, // Bracelet made from roots 25
  {25016, 3}, // a Silver Trident 50
  {25027, 3}, // The Silver Circlet of Concentration 50
  {26408, 3}, // A Crackled Black Urn 10
  {26409, 3}, // A Flying Carpet 10
  {27108, 3}, // a Staff of the Magi 78
  {27113, 3}, // the thick hide of a dinosaur 23
  {27116, 3}, // The Shadowblade 32
  {28503, 3}, // Pan's Flute 15
  {204, 4}, // A gleaming spyglass 7
  {3907, 4}, // The tail of the Celestial Dragon 10
  {5709, 4}, // The medallion of Akinra 10
  //{5711, 4}, // mercury's boots 10
  {5804, 4}, // Pair of red lederhosen 20
  {6208, 4}, // a chitonous exoskeleton 10
  {6210, 4}, // a priest's skirt 10
  {6215, 4}, // a pair of bearskin sleeves 10
  {7520, 4}, // a butchering knife 30
  {7509, 4}, // a pair of silk gloves 10
  {7510, 4}, // a diamond ring 10
  {7511, 4}, // a diamond bracelet 10
  {7522, 4}, // a pair of white, flowing silk pants 10
  {8362, 4}, // dusty pants 20
  {10015, 4}, // an Ethereal Sash 3
  {10022, 4}, // Obsidian Ring 30
  {10028, 4}, // worn hickory cane 32
  {10905, 4}, // a pair of Zyca arm plates 10
  {10907, 4}, // a Zyca ring 10
  {11506, 4}, // the Wyvernspur 50
  {12028, 4}, // Snakeskin Belt 10
  {12023, 4}, // Dark Banded Mail 25
  {12032, 4}, // Adventurer's Cloak 33
  {12040, 4}, // skin of salamander 33
  {12915, 4}, // a blue silk garb 20
  {13101, 4}, // a living flame 15
  {13201, 4}, // a shard of frozen water 15
  {13301, 4}, // a heart of stone 15
  {13401, 4}, // a wispy cloud 15
  {13506, 4}, // Glowing armbands of Creaz 20
  {13507, 4}, // Ornate platinum belt 20
  {14503, 4}, // a Blood-Red Amulet 12
  {16512, 4}, // A Rusted Bucket 15
  {16513, 4}, // A rotting bridle 15
  {16514, 4}, // A hardened wax breast plate 20
  {16530, 4}, // Filthy Slippers 75
  {17024, 4}, // A pointy wizard's hat 20
  {17080, 4}, // a Silver Harmonica 20
  {17321, 4}, // Cube of Awareness 50
  {20840, 4}, // a shield of mottled flesh 12
  {21108, 4}, // A giant leaf shield 10
  {21205, 4}, // Boots of Stealth 10
  {21217, 4}, // the Mallet of the Underworld 10
  {23008, 4}, // Pelt of the Glacial Polar Bear 5
  {24900, 4}, // druidic battle wrap 14
  {24901, 4}, // Dark Druid's buckler 15
  {24905, 4}, // Druid's Leafy Green Cloak 12
  {25007, 4}, // Silver Shield 10
  {25010, 4}, // A Pair of Jeweled Gauntlets 10
  {25011, 4}, // A Pitch-Black Cloak 20
  {26475, 4}, // The Gloves Of Siva 10
  {26476, 4}, // A Jade Ring 20
  {26710, 4}, // A Transparent Cutlass 10
  {27100, 4}, // a Centurion's helmet 13
  {27104, 4}, // Cestus 23
  {27110, 4}, // a hunting knife 12
  {27123, 4}, // a wreath of true laurel 10
  {27721, 4}, // Wristband of the Assassin 10
  {518, 5}, // a shinobigawa 14
  {802, 5}, // Sword of Healing 10
  {4608, 5}, // a Parasite's Fang 5
  {4701, 5}, // the Gloves of Leoric 9
  {5109, 5}, // Drow Battle Gauntlets 50
  {5111, 5}, // a polished marble ring 13
  {5116, 5}, // a piwafwi cloak 13
  {5175, 5}, // an eyepatch 19
  {5920, 5}, // Myconid blood 15
  {6217, 5}, // a flute made of bone 8
  {11327, 5}, // a band of twisted veins 7
  {12208, 5}, // Amulet of Ettins 10
  {12211, 5}, // Ettins Cape 10
  //{12822, 5}, // the Scent of a Mankey 10
  {12827, 5}, // A bag of MankeyBits 10
  {12928, 5}, // a silver katana 8
  {13001, 5}, // an Elven Cloak 15
  {14002, 5}, // The Shadow Cloak 15
  {14004, 5}, // The Shadow Plate 15
  {16528, 5}, // A Stained Apron 20
  {17309, 5}, // Fiend's Girdle 30
  {20107, 5}, // Drums of Panic 35
  {21110, 5}, // A sword made from bone 10
  {21200, 5}, // Darkened Bone Ring 10
  {21201, 5}, // Darkened Bone Plate 8
  {21203, 5}, // The Mask of Concentration 18
  {21314, 5}, // An Ornamental Belt 1
  {21322, 5}, // A Suit of Red Scale Mail 1
  {24906, 5}, // Evil Avenger 11
  {25018, 5}, // A Crystal Ring 10
  {25026, 5}, // the Soul Amulet 10
  {27102, 5}, // A bone mace 12
  {27106, 5}, // Gladius 9
  {27712, 5}, // Basilisk Blood Potion 10
  {27720, 5}, // Banded Mail of the Bandit 5
  {704, 6}, // Mask of Lizard Powers 10
  {707, 6}, // The Ring of Eternal Life 7
  {4704, 6}, // Werra's Belt of Garbage 5
  {5105, 6}, // a jeweled scimitar 12
  {5197, 6}, // Ball of Faerie Fire 10
  {12001, 6}, // golden breastplate 10
  {12018, 6}, // golden leggings 10
  {12020, 6}, // golden shield 10
  {12901, 6}, // the Fung-Tai dagger 8
  {12909, 6}, // a band of holy spirits 5
  {13509, 6}, // Black cloak of Darkness 15
  {14502, 6}, // The Hell Stick 10
  {16529, 6}, // Wax Ring 12
  {17002, 6}, // Vermilion's Blade 16
  {17005, 6}, // Hood of Darkness 10
  {17371, 6}, // Dragon Horn 15
  {17396, 6}, // A mithril hammer 10
  {20145, 6}, // War Fan 17
  {21206, 6}, // Armlinks of Fire 15
  {21210, 6}, // a Dragon Scale Belt 17
  {21321, 6}, // A Pair of Gleaming Gauntlets 2
  {26406, 6}, // A Holey Majestic Cloak 12
  {26477, 6}, // The Mantle of Devotion 8
  {700, 7}, // a dusty deck of Tarot Cards 10
  {702, 7}, // a pair of baggy pants 10
  {4700, 7}, // a small voodoo doll 3
  {5546, 7}, // Long Spiked Whip 10
  {7700, 7}, // Typik's Bloody Entrails 17
  {11702, 7}, // A Silver Full Plate 11
  {17322, 7}, // Dragon Highlord's Shield 10
  {21207, 7}, // Black Demon's Talon 10
  {21212, 7}, // The Stone Dragon's Shield 10
  {26478, 7}, // The Signet Ring of the Sultan 6
  {26578, 7}, // A Tortoise Shell Shield 8
  {27722, 7}, // Talisman of the Desert Moon 5
  {28739, 7}, // A pair of melted Wax Wings 15
  {28740, 7}, // The Shield of All Times 15
  {543, 8}, // a Tengu Cloak of Straw 7
  {5110, 8}, // A tome of forbidden magic 8
  {5582, 8}, // a silken cloak 11
  {11720, 8}, // a cracked dragon's tooth 13
  {13500, 8}, // a tooth-and-skull necklace 7
  {14507, 8}, // Sandals of Sorrow 10
  {17306, 8}, // Fiend's Necklace 8
  {17372, 8}, // The Seal of the Drake 7
  {20146, 8}, // shimmering band of Mokume Gane 5
  {26713, 8}, // A Clean Red Cloak 5
  {26714, 8}, // A Glowing Staff of Gnarled Wood 5
  {28741, 8}, // The Coin of Fate 12
  {5198, 10}, // Flaming Mask 10
  {14212, 10}, // Boundary Warden Belt 45
  //{7201, 10}, // a soul shard fragment 10
  {16503, 10}, // the Dagger of Segretto 5
  {16805, 10}, // Scythe of Execution 8
  {16806, 10}, // A Bladed Lash 8
  {21326, 10}, // The Left Gauntlet of Calaphas 10
  {26402, 10}, // The Wand of Watoomb 18
  {26579, 10}, // The Loincloth of Good Omens 8
  {26711, 10}, // The Greatsword of the Guard 8
  {27711, 10}, // A Carapace Shield 7
  {4478, 15}, // A Pretty Noose 8
  {4488, 15}, // Axe of Justice 10
  //{5807, 15}, // Silk Suit 20
  {11712, 15}, // Armor of Dark Angels 8
  {19400, 15}, // An Extraordinarily Large Grolem Beak 10
  {26404, 15}, // The Dark Sphere of Ul 8
  {571, 20}, // Ebon Armguards 5
  {585, 20}, // Small Portal of Evil 7
  {599, 20}, // The Gong of Unholy Wrath 8
  {5580, 20}, // Scorched Abdomen 8
  {5581, 20}, // Some Fine Elastic Cartilage 9
  {20132, 20}, // A Gleaming Jade Battlesuit 6
  {20138, 20}, // Helm of Lacquered Wood 8
  {21329, 20}, // a swarm of angry beetles 4
  {21330, 20}, // a mass of black, glistening thorns 5
  {25300, 20}, // Tail of Minos 8
  {26403, 20}, // The Mallet of Orcus 6
  {26709, 20}, // The Grand Cape of Emithoynarthe 15
  {26712, 20}, // Heavy Red Gloves 15
  {26715, 20}, // Shimmering Metallic Greaves 15
  //{27724, 20}, // A Pair of Fine Leather Boots 3
  //{27726, 20}, // The Circlet of Devotion 4
  //{27727, 20}, // A Pair of Tactical Zyca Leg Plates 4
  {28602, 20}, // Olympian Leg Plates 12
  {28720, 20}, // The Staff of Dark Influence 8
  //{300, 25}, // The Talon of a Red Dragon 6
  //{301, 25}, // a pair of red dragon scale boots 6
  //{572, 25}, // An oaken root wand 3
  //{2716, 25}, // The black sting of the Queen 10
  //{6806, 25}, // Erishkigal's lash 9
  {14208, 25}, // Councilor's Plate 10
  {21309, 25}, // A Rotting Otyugh Skin 2
};

#define STORAGE_ROOM            5807
#define CENTRAL_PROCESSING      5812
#define COLLECTOR               5812
#define TEMPLATE_AQORDER        40
#define AQ_ORDER_QUIT_TIME      150
#define AQ_ORDER_WAIT_TIME      30

const char *kendernames[] = {
  "Karl",
  "Dieter",
  "Hans",
  "Jurgen",
  "Hilda",
  "Erwin",
  "Herman",
  "Eva",
  "Marlene"
};

int generate_aq_order(CHAR *requester, CHAR *ordergiver, int lh_opt) {
  if (requester->ver3.id <= 0) {
    requester->ver3.id = generate_id();
  }

  OBJ *aq_order = read_object(TEMPLATE_AQORDER, VIRTUAL);

  // cleanup first
  DESTROY(aq_order->description);
  DESTROY(aq_order->short_description);

  aq_order->ownerid[0] = requester->ver3.id;   // tag the order with the requester's ID

  char buf[MAX_STRING_LENGTH];

  sprintf(buf, "An infamous acquisition order, forgotten here by %s.", GET_NAME(requester));
  aq_order->description = str_dup(buf);        // change the long desc to include requester's name

  sprintf(buf, "%s's acquisition order", GET_NAME(requester));
  aq_order->short_description = str_dup(buf);  // change the short desc to include requester's name

  aq_order->log = 1;

  int aq_obj_temp_list[NUMELEMS(aq_obj_master_list)][2];
  int aq_obj_num = 0;

  for (int i = 0; i < NUMELEMS(aq_obj_master_list); i++) {
    int aq_obj_temp_vnum = aq_obj_master_list[i][0];
    int aq_obj_temp_value = aq_obj_master_list[i][1];

    if ((lh_opt == 0) && (aq_obj_temp_value > 8)) continue;   // other (non-uber)
    if ((lh_opt == 1) && (aq_obj_temp_value != 1)) continue;  // newbie
    if ((lh_opt == 2) && (aq_obj_temp_value != 2)) continue;  // low
    if ((lh_opt == 3) && (aq_obj_temp_value != 3)) continue;  // mid
    if ((lh_opt == 4) && (aq_obj_temp_value != 4)) continue;  // high
    if ((lh_opt == 5) && ((aq_obj_temp_value < 4) ||
                          (aq_obj_temp_value > 8))) continue; // veteran
    if ((lh_opt == 6) && (aq_obj_temp_value < 10)) continue;  // uber

    aq_obj_temp_list[aq_obj_num][0] = aq_obj_temp_vnum;
    aq_obj_temp_list[aq_obj_num][1] = aq_obj_temp_value;

    aq_obj_num++;
  }

  /* We need at least 4 eligible objects and not more than the max number of objects (sanity check). */
  if (aq_obj_num >= 4 || aq_obj_num <= NUMELEMS(aq_obj_temp_list)) {
    /* Shuffle the array of eligible objects. We'll pick the first 4 below. */
    shuffle_2d_int_array(aq_obj_temp_list, aq_obj_num);

    int quest_value = 0;

    for (int i = 0; i < 4; i++) {
      int order_obj_vnum = aq_obj_temp_list[i][0];
      int order_obj_value = aq_obj_temp_list[i][1];

      aq_order->obj_flags.value[i] = order_obj_vnum;
      quest_value += order_obj_value;
    }

    // tag order with questvalue - visible in magic.c "identify"
    OBJ_SPEC(aq_order) = quest_value;

    // change the extra desc to include required objects
    struct extra_descr_data *tmp_descr;
    CREATE(tmp_descr, struct extra_descr_data, 1);
    tmp_descr->keyword = str_dup("order");
    sprintf(buf, "A well-worn, faded canvas bag has a large blue patch sewn\n\r\
to it. The patch has been sewn over half-a-hundred times and\n\r\
had those stitches ripped out just as many. There are four\n\r\
hastily sewn lines of text detailing the kenders' wish list.\n\r\
\n\r   %s\n\r   %s\n\r   %s\n\r   %s\n\r",
      aq_order->obj_flags.value[0] >= 0 ? real_object(aq_order->obj_flags.value[0]) >= 0 ? obj_proto_table[real_object(aq_order->obj_flags.value[0])].short_description : "something" : "nothing",
      aq_order->obj_flags.value[1] >= 0 ? real_object(aq_order->obj_flags.value[1]) >= 0 ? obj_proto_table[real_object(aq_order->obj_flags.value[1])].short_description : "something" : "nothing",
      aq_order->obj_flags.value[2] >= 0 ? real_object(aq_order->obj_flags.value[2]) >= 0 ? obj_proto_table[real_object(aq_order->obj_flags.value[2])].short_description : "something" : "nothing",
      aq_order->obj_flags.value[3] >= 0 ? real_object(aq_order->obj_flags.value[3]) >= 0 ? obj_proto_table[real_object(aq_order->obj_flags.value[3])].short_description : "something" : "nothing");
    tmp_descr->description = str_dup(buf);
    aq_order->ex_description = tmp_descr;
    tmp_descr = NULL;

    sprintf(buf, "Good luck %s, try to do a better job than %s, it'll be worth %d points if you can.",
      GET_NAME(requester), kendernames[number(0, NUMELEMS(kendernames)-1)], quest_value);
    do_say(ordergiver, buf, CMD_SAY);

    obj_to_char(aq_order, requester);

    sprintf(buf, "%s gives %s to %s.", GET_SHORT(ordergiver),
      OBJ_SHORT(aq_order), GET_NAME(requester));
    act(buf, 0, ordergiver, 0, requester, TO_NOTVICT);

    sprintf(buf, "%s gives %s to you.", GET_SHORT(ordergiver),
      OBJ_SHORT(aq_order));
    act(buf, 0, ordergiver, 0, requester, TO_VICT);

    return TRUE;
  }
  else {
    wizlog("WIZINFO: AQ Order object assignment error.", LEVEL_IMP, 5);
    log_f("WIZINFO: AQ Order object assignment error.");

    obj_to_room(aq_order, real_room(CENTRAL_PROCESSING));

    return FALSE;
  }

  return FALSE;
}

static void get_owner_name(OBJ *order, char *buf) {
  if (order->ownerid[0] < 1) {
    strncpy(buf, "no-one", MAX_DNAME_LENGTH);
    return;
  }

  strncpy(buf, idname[order->ownerid[0]].name, MAX_DNAME_LENGTH);

  if (buf[0] == '\0') {
    strncpy(buf, "someone", MAX_DNAME_LENGTH);
    return;
  }

  (void)CAP(buf);
}

int aq_order_obj(OBJ *order, CHAR *ch, int cmd, char *arg) {
  char argument[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char name[MAX_DNAME_LENGTH];
  struct extra_descr_data *tmp_descr;
  CHAR *collector = NULL;
  char *collectorinsult[] = {
    "wimp",
    "quitter",
    "lame-o",
    "goldbricker",
    "pansy",
    "slacker",
    "chump",
    "loser"
  };

  // block methods of getting order out of game and back again on non-owner
  // since this would allow multiple orders for a character at once
  // "junk" only real method to quit an order
  // Dump/Scrapyard workaround is in int dump()

  if (ch != order->carried_by) return FALSE;

  switch(cmd) {
    case MSG_OBJ_ENTERING_GAME:
      if (V_OBJ(order) != TEMPLATE_AQORDER) return FALSE;

      // redo the short/long desc strings based on Owner
      if (order->ownerid[0] > 0) {
        get_owner_name(order, name);

        // cleanup first
        DESTROY(order->description);
        DESTROY(order->short_description);
        sprintf(buf, "An infamous acquisition order, forgotten here by %s.", name);
        order->description = str_dup(buf);        // change the long desc to include requester's name
        sprintf(buf, "%s's acquisition order", name);
        order->short_description = str_dup(buf);  // change the short desc to include requester's name
      }

      // change the extra desc to include required objects
      CREATE(tmp_descr, struct extra_descr_data, 1);
      tmp_descr->keyword = str_dup("order");
      sprintf(buf, "A well-worn, faded canvas bag has a large blue patch sewn\n\r\
to it. The patch has been sewn over half-a-hundred times and\n\r\
had those stitches ripped out just as many. There are four\n\r\
hastily sewn lines of text detailing the kenders' wish list.\n\r\
\n\r   %s\n\r   %s\n\r   %s\n\r   %s\n\r",
        order->obj_flags.value[0] >= 0 ? real_object(order->obj_flags.value[0]) >= 0 ? obj_proto_table[real_object(order->obj_flags.value[0])].short_description : "something" : "nothing",
        order->obj_flags.value[1] >= 0 ? real_object(order->obj_flags.value[1]) >= 0 ? obj_proto_table[real_object(order->obj_flags.value[1])].short_description : "something" : "nothing",
        order->obj_flags.value[2] >= 0 ? real_object(order->obj_flags.value[2]) >= 0 ? obj_proto_table[real_object(order->obj_flags.value[2])].short_description : "something" : "nothing",
        order->obj_flags.value[3] >= 0 ? real_object(order->obj_flags.value[3]) >= 0 ? obj_proto_table[real_object(order->obj_flags.value[3])].short_description : "something" : "nothing");

      tmp_descr->description = str_dup(buf);
      order->ex_description = tmp_descr;
      tmp_descr = NULL;
      break;

    case MSG_CORPSE:
      // pop orders out when a corpse is made with a message
      if (ch && (V_OBJ(order) == TEMPLATE_AQORDER)) {
        get_owner_name(order, name);

        obj_from_char(order);
        obj_to_room(order, CHAR_REAL_ROOM(ch));
        sprintf(buf, "Just as %s expires, %s drops %s.\n\r", GET_DISP_NAME(ch), HSSH(ch), OBJ_SHORT(order));
        send_to_room(buf, CHAR_REAL_ROOM(ch));
        log_f("QSTINFO: AQ Order for %s dropped from %s's corpse in room %d", name, GET_DISP_NAME(ch), CHAR_VIRTUAL_ROOM(ch));
      }
      break;

    case MSG_AUTORENT:
      if (V_OBJ(order) != TEMPLATE_AQORDER) return FALSE;

      if ((order->ownerid[0] != ch->ver3.id) && (order->ownerid[0] > 0)) {
        get_owner_name(order, name);

        obj_from_char(order);
        obj_to_room(order, real_room(STORAGE_ROOM));

        // want to have COLLECTOR send a quest message when he intercepts an
        //   autorent like this, so we find him or load him
        if (mob_proto_table[real_mobile(COLLECTOR)].number < 1) {
          collector = read_mobile(COLLECTOR, VIRTUAL);
          char_to_room(collector, real_room(number(3000,3072)));
        }
        else {
          collector = get_ch_world(COLLECTOR);
          if (!collector) return FALSE; // shouldn't be possible, but for safety
        }

        sprintf(buf, "Oops, %s nearly auto-rented with %s, but I moved it to our main storage facility for safekeeping.", GET_DISP_NAME(ch), OBJ_SHORT(order));
        do_quest(collector, buf, CMD_QUEST);
        log_f("QSTINFO: AQ Order for %s placed in %d on autorent of %s", name, STORAGE_ROOM, GET_DISP_NAME(ch));
      }
      break;

    case CMD_RENT:
      if (V_OBJ(order) != TEMPLATE_AQORDER) return FALSE;
      if(!IS_NPC(ch) && GET_LEVEL(ch) >= LEVEL_IMM) return FALSE;

      if ((order->ownerid[0] != ch->ver3.id) && (order->ownerid[0] > 0)) {
        sprintf(buf, "Something prevents you from renting with %s.\n\r", OBJ_SHORT(order));
        send_to_char(buf, ch);
        return TRUE;
      }
      break;

    case CMD_STORE:
      if(!IS_NPC(ch) && GET_LEVEL(ch) >= LEVEL_IMM) return FALSE;
      // technically this will trigger anywhere and not just at vault
      //   but due to potential for clan vaults, I'm just leaving it
      //   as a broad case
      arg = one_argument(arg, argument);
      if (!*argument) return FALSE; // no argument after "store"
      if (!(order = get_obj_in_list_vis(ch, argument, ch->carrying))) return FALSE;
      if (V_OBJ(order) != TEMPLATE_AQORDER) return FALSE;

      sprintf(buf, "Something prevents you from storing %s.\n\r", OBJ_SHORT(order));
      send_to_char(buf, ch);
      return TRUE;
      break;

    case CMD_JUNK:
      if(!IS_NPC(ch) && GET_LEVEL(ch) >= LEVEL_IMM) return FALSE;
      arg = one_argument(arg, argument);
      if (!*argument) return FALSE; // no argument after "junk"
      if (!(order = get_obj_in_list_vis(ch, argument, ch->carrying))) return FALSE;
      if (V_OBJ(order) != TEMPLATE_AQORDER) return FALSE;

      // want to have COLLECTOR send a quest message when he intercepts a
      // junk like this, so we find him or load him
      if (mob_proto_table[real_mobile(COLLECTOR)].number < 1) {
      collector = read_mobile(COLLECTOR, VIRTUAL);
      }
      else {
        collector = get_ch_world(COLLECTOR);
        if (!collector) return FALSE; // shouldn't be possible, but for safety
        sprintf(buf, "%s disappears in a beam of bright light.\n\r", GET_SHORT(collector));
        send_to_room(buf, CHAR_REAL_ROOM(collector));
        char_from_room(collector);
      }
      char_to_room(collector, CHAR_REAL_ROOM(ch));
      sprintf(buf, "%s appears in a corona of bright light and points a strange pistol at %s, which disappears.\n\r", GET_SHORT(collector), OBJ_SHORT(order));
      send_to_room(buf, CHAR_REAL_ROOM(collector));

      if (order->ownerid[0] > 0 && strcmp(idname[ch->ver3.id].name, idname[order->ownerid[0]].name)) {
        // someone other than owner is junking
        get_owner_name(order, name);

        sprintf(buf,
          "Tsk tsk %s, doing %s's dirty work. "
          "If %s can't handle the order I'll just send it back to Central Processing to be requeued, that %s.",
          GET_DISP_NAME(ch), name, name, collectorinsult[number(0, NUMELEMS(collectorinsult)-1)]);
      }
      else {
        sprintf(buf,
          "I saw that %s, if you don't think you can handle the order I'll just send it back to Central Processing to be requeued... you %s.",
          GET_DISP_NAME(ch), collectorinsult[number(0, NUMELEMS(collectorinsult)-1)]);
      }

      do_quest(collector, buf, CMD_QUEST);

      // Prevent crash if CENTRAL_PROCESSING is not loaded in the world (usually just in test/dev scenarios).
      if (real_room(CENTRAL_PROCESSING) == NOWHERE) {
        log_s("Order junked but CENTRAL_PROCESSING is not loaded.");
        return TRUE;
      }

      // this is where it gets moved to another room to "hack" a wait timer
      // --- boots and crashes will cancel this, so be it
      obj_from_char(order);
      obj_to_room(order, real_room(CENTRAL_PROCESSING));
      order->obj_flags.timer = AQ_ORDER_QUIT_TIME; // setup "hack" wait timer
      SET_BIT(order->obj_flags.extra_flags2, ITEM_ALL_DECAY);
      return TRUE;
      break;

    case MSG_OBJ_DROPPED:
      get_owner_name(order, name);
      log_f("QSTINFO: AQ Order for %s dropped by %s in room %d", name, GET_DISP_NAME(ch), CHAR_VIRTUAL_ROOM(ch));
      break;

    default:
      break;
  }

  return FALSE;
}

// list of VNUMs of Uber bosses, and associated room VNUM
int ubers[][2] = {
  // {mob VNUM, room VNUM} // <spec file>
  {12027, 12063}, // Queen Ant - <none>
  {14014, 14065}, // Dream Shadow - spec.house.c
  {21220, 21275}, // Huge Stone Dragon - spec.moria.c
  {26585, 26582}, // Enlightened One - spec.pagoda.c
  {20109, 20109}, // Musashi - spec.daimyo.c
  {11300, 11400}, // Strahd - spec.barovia.c
#ifdef TEST_SITE
  {14600, 12094}, //King Spider - spec.ubers.c
  {14601, 12931}, //Great Mystic - spec.ubers.c
  {14602, 12904} //Ultimate Mystic - spec.ubers.c
#endif
};


int aq_order_mob (CHAR *collector, CHAR *ch, int cmd, char *arg) {
  OBJ *order = NULL;
  OBJ *obj = NULL, *next_obj  = NULL;
  char buf[MAX_STRING_LENGTH];
  char argument[MAX_INPUT_LENGTH];
  char name[MAX_DNAME_LENGTH];
  int i, j, k, num_objects = 4, tmp_value, lh_opt = 0, questvalue = 0;
  int requirements[4] = {-1, -1, -1, -1};
  bool value_exists = FALSE;
  bool found[4] = {FALSE, FALSE, FALSE, FALSE};
  char *collectoraction[8] = {"groan","frustration","cod","fume",
                              "blorf","roll","sneor","mumble"};
  char *kenderinsults[10] = {"fool","moron","idiot","bonehead",
                            "nitwit","nincompoop","imbecile","dullard",
                            "cotton-headed ninnymuggins","peabrain"};
  CHAR *uber_mob = NULL;
  char *collectorexclamation[7] = {"Attention","Zounds","Great Scott!","The Realm is doomed",
                              "Blame Sane","Damn you Lovecraft","It is the end times"}; 
  int uber_choice = 0, uber_room = 0;

  if (cmd == CMD_AQUEST) {
    // process order request
    arg = one_argument(arg, argument);
    if (*argument) {
      if (is_abbrev(argument,"order")) {
        // check if already an order in game for that player
        for (obj = object_list; obj; obj = obj->next) {
          if ((TEMPLATE_AQORDER == V_OBJ(obj)) && (obj->ownerid[0] == ch->ver3.id)) {
            if (obj->in_room == real_room(CENTRAL_PROCESSING)) {
              mob_do(collector, "shrug");
              sprintf(buf, "Your previous order is still being processed %s, it'll be another %d minutes or so. Longer if %s is working on it I suppose.",
                  GET_NAME(ch), obj->obj_flags.timer, kendernames[number(0, NUMELEMS(kendernames)-1 )]);
            } else {
              sprintf(buf, "You've already got an order %s, I doubt you could handle more.", GET_NAME(ch));
            }
            do_say(collector, buf, CMD_SAY);
            return TRUE;
          }
        }

        arg = one_argument(arg, argument);

        if (*argument) {
          if (is_abbrev(argument,"newbie"))  lh_opt=1;
          if (is_abbrev(argument,"low"))     lh_opt=2;
          if (is_abbrev(argument,"mid"))     lh_opt=3;
          if (is_abbrev(argument,"high"))    lh_opt=4;
          if (is_abbrev(argument,"veteran")) lh_opt=5;
          if (is_abbrev(argument,"uber"))    lh_opt=6;
        }

        if (lh_opt == 1 && GET_LEVEL(ch) >= 40) {
          mob_do(collector, "bah");
          sprintf(buf, "I've been surrounded by slackers for years, I'm not interested in another one. \
You're too experienced for that kind of order %s, and you know it.", GET_NAME(ch));
          do_say(collector, buf, CMD_SAY);
        } else if (lh_opt == 6 && GET_LEVEL(ch) < 45) {
          mob_do(collector, "rofl");
          do_say(collector, "Not bloody likely. You couldn't handle that level of order pipsqueak.", CMD_SAY);
        } else if (!generate_aq_order(ch, collector, lh_opt)) {
          mob_do(collector, "shrug");
          sprintf(buf, "Sorry %s, guess I couldn't find an order for you.",
              GET_NAME(ch));
          do_say(collector, buf, CMD_SAY);
        }
      return TRUE;
      }
    }
  } // end "request order"

  if (cmd == MSG_MOBACT && count_mortals_room(collector, TRUE) > 0) {
    // generic flavor
    switch(number(0,49)) {
    case 0:
      mob_do(collector, "grumble");
      break;
    case 1:
      sprintf(buf, "%s screams into a small device on %s arm, 'Quit goofing around!'\n\r",
          GET_SHORT(collector),HSHR(collector));
      send_to_room(buf, CHAR_REAL_ROOM(collector));
      mob_do(collector, "steam");
      break;
    case 2:
      do_say(collector, "Looking for some quest points?", CMD_SAY);
      break;
    case 3:
      do_say(collector, "I could always use the help of a moderately intelligent individual.", CMD_SAY);
      break;
    case 4:
      sprintf(buf, "Ugh, %s is completely useless!", kendernames[number(0, NUMELEMS(kendernames)-1 )]);
      do_say(collector, buf, CMD_SAY);
      break;
    case 5:
      do_say(collector, "Argh, these orders are piling up.", CMD_SAY);
      break;
    case 6:
      sprintf(buf, "You hear faint voices coming from a small device on %s's arm.\n\r",
          GET_SHORT(collector));
      send_to_room(buf, CHAR_REAL_ROOM(collector));
      mob_do(collector, chance(50) ? "roll" : "ignore");
      break;
    default:
      break;
    }

    if(collector->specials.fighting) {
      do_say(collector, "What the heck are you doing? Scotty get me outta' here!", CMD_SAY);
      sprintf(buf, "%s disappears in a beam of bright light.\n\r", GET_SHORT(collector));
      send_to_room(buf, CHAR_REAL_ROOM(collector));
      GET_HIT(collector) = GET_MAX_HIT(collector);
      char_from_room(collector);
      char_to_room(collector, real_room(number(3000,3072)));
    }
  }

  if (cmd == MSG_OBJ_GIVEN) {
    // process order delivery
    order = get_obj_in_list_vis(collector, "order", collector->carrying);

    if (order && (OBJ_TYPE(order) == ITEM_AQ_ORDER)) {
      get_owner_name(order, name);

      if ((order->ownerid[0] != ch->ver3.id) && (order->ownerid[0] > 0)) {
        // handed in by non-owner
        sprintf(buf, "Where'd you get this? Did you kill 'em? Give it back to %s.", name);
        do_say(collector, buf, CMD_SAY);
        obj_from_char(order);
        obj_to_char(order, ch);
        sprintf(buf, "%s hands %s back to %s, eyeing %s warily.", GET_SHORT(collector), OBJ_SHORT(order), GET_NAME(ch), HMHR(ch));
        act(buf,0,collector,0,ch,TO_NOTVICT);
        sprintf(buf, "%s hands %s back to you warily.", GET_SHORT(collector), OBJ_SHORT(order));
        act(buf,0,collector,0,ch,TO_VICT);
        return TRUE;
      }

      for (i = 0; i < 4; i++) {
        requirements[i] = order->obj_flags.value[i];
        if(requirements[i] < 0) { // null (-1) requirement object
          found[i] = TRUE;
          num_objects--;
        } else {
          for (obj = order->contains; obj; obj = next_obj) {
            next_obj = obj->next_content;

            if (V_OBJ(obj) == requirements[i]) {
              // required object found, but is it new enough?
              if (obj->obj_flags.popped < (order->obj_flags.popped - 1)) {
                // object was popped before order received : see aq_obj_date_popped()
                do_say(collector, "No, no, no - this won't do at all.", CMD_SAY);
                sprintf(buf, "%s was popped too long ago, I need a new one!", OBJ_SHORT(obj));
                do_say(collector, buf, CMD_SAY);
                obj_from_char(order);
                obj_to_char(order, ch);
                sprintf(buf, "%s gives %s back to %s disappointedly.", GET_SHORT(collector), OBJ_SHORT(order), GET_NAME(ch));
                act(buf,0,collector,0,ch,TO_NOTVICT);
                sprintf(buf, "%s gives %s back to you disappointedly.", GET_SHORT(collector), OBJ_SHORT(order));
                act(buf,0,collector,0,ch,TO_VICT);
                return TRUE;
              }
              else if (obj->obj_flags.extra_flags & ITEM_CLONE) {
                do_say(collector, "No, no, no - this won't do at all.", CMD_SAY);
                sprintf(buf, "%s is a clone, and I don't take cheap knock-offs!", OBJ_SHORT(obj));
                do_say(collector, buf, CMD_SAY);
                obj_from_char(order);
                obj_to_char(order, ch);
                sprintf(buf, "%s gives %s back to %s disappointedly.", GET_SHORT(collector), OBJ_SHORT(order), GET_NAME(ch));
                act(buf,0,collector,0,ch,TO_NOTVICT);
                sprintf(buf, "%s gives %s back to you disappointedly.", GET_SHORT(collector), OBJ_SHORT(order));
                act(buf,0,collector,0,ch,TO_VICT);
                return TRUE;
              }
              else {
                found[i] = TRUE;
                value_exists = FALSE;
                // OBJ_SPEC(order) is set initially by aq_generate_order() to be
                //   the order's AQP value, for magic.c identify output;
                //   but we'll count here anyway with aq_objs[][] current values
                //   also gives us a chance to award a double point bonus
                for (k = 0; k < NUMELEMS(aq_obj_master_list); k++) {
                  if (aq_obj_master_list[k][0] == V_OBJ(obj)) {
                    value_exists = TRUE;
                    tmp_value = aq_obj_master_list[k][1];
                    if (chance( MIN(5, tmp_value) )) {
                      // higher value has higher chance for doubling - reward trying harder
                      tmp_value *= 2;
                    }
                    questvalue += tmp_value;
                  }
                }
                if(!value_exists) {
                  // order included an object VNUM not in aq_objs[]
                  sprintf(buf, "WIZINFO: AQ Order included object #: %d which is not in aq_objs point value table.", V_OBJ(obj));
                  wizlog(buf,LEVEL_SUP,5);
                  log_s(buf);
                }
              }
            }
          }
        }
      }

      if (found[0] && found[1] && found[2] && found[3] && (COUNT_CONTENTS(order) == num_objects)) {
        // fulfilled requirement objects!
        switch(number(0,9)) {
        case 0:
          do_say(collector, "Splendid, truly marvelous! I'll send this on to Central Processing.", CMD_SAY);
          break;
        case 1:
          do_say(collector, "And there was much rejoicing.", CMD_SAY);
          break;
        case 2:
          do_say(collector, "Wow, you did it.", CMD_SAY);
          break;
        case 3:
          do_say(collector, "I'm so used to failure, this is a welcome surprise.", CMD_SAY);
          break;
        case 4:
          do_say(collector, "I won't ask where any of this came from.", CMD_SAY);
          mob_do(collector, "wink");
          break;
        default:
          break;
        }

        if (questvalue != OBJ_SPEC(order)) {
          sprintf(buf, "The market is always in flux my %s. This is worth %d points to me now.", GET_SEX(ch) == SEX_MALE ? "boy" : "dear", questvalue);
          do_say(collector, buf, CMD_SAY);
        }
        else {
          sprintf(buf, "Well done! Your %d points are well deserved %s.", questvalue, GET_SEX(ch) == SEX_MALE ? "sir" : "madam");
          do_say(collector, buf, CMD_SAY);
        }

        sprintf(buf, "%s casually points a strange pistol at the order\n\r...and it disappears before your very eyes!\n\r", GET_SHORT(collector));
        send_to_room(buf, CHAR_REAL_ROOM(collector));

        sprintf(buf, "%s completed an order for me, what a %s!", GET_NAME(ch), GET_SEX(ch) == SEX_MALE ? "guy" : "gal");
        do_quest(collector, buf, CMD_QUEST);
        ch->ver3.quest_points += questvalue;

        // this is where it gets moved to another room to "hack" a wait timer
        //   --- boots and crashes will cancel this, so be it
        obj_from_char(order);
        obj_to_room(order, real_room(CENTRAL_PROCESSING));
        order->obj_flags.timer = AQ_ORDER_WAIT_TIME; // setup "hack" wait timer
        SET_BIT(order->obj_flags.extra_flags2, ITEM_ALL_DECAY);
        return FALSE;
      }
      else {
        if (!found[0] || !found[1] || !found[2] || !found[3]) { // missing at least one object
          mob_do(collector, collectoraction[number(0, NUMELEMS(collectoraction)-1)]);
          for (j = 0; j < 4; j++) {
            if(!found[j] && requirements[j] >= 0) {
              // generate annoyed response
              sprintf(buf, "You're even %s than that %s %s. You didn't bring me %s!",
                chance(50) ? "dumber" : "stupider",
                kenderinsults[number(0, NUMELEMS(kenderinsults)-1 )],
                kendernames[number(0, NUMELEMS(kendernames)-1 )],
                real_object(requirements[j]) >= 0 ? obj_proto_table[real_object(requirements[j])].short_description : "something");
              do_say(collector, buf, CMD_SAY);
              break;
            }
          }
          sprintf(buf,"Read the order more carefully, I don't have time for your %s %s.",
              chance(50) ? "tomfoolery" : "nonsense", GET_NAME(ch));
          do_say(collector, buf, CMD_SAY);
        }
        else if (COUNT_CONTENTS(order) != num_objects) { // more than 4 objects in order
          sprintf(buf, "thumbsdown %s", GET_NAME(ch));
          mob_do(collector, buf);
          sprintf(buf, "%s, are you trying to be as foolish as %s, or does it just come naturally? The number of the counting shall be 4!",
              GET_NAME(ch), kendernames[number(0, NUMELEMS(kendernames)-1 )]);
          do_say(collector, buf, CMD_SAY);
        }

        obj_from_char(order);
        obj_to_char(order, ch);
        sprintf(buf, "%s tosses %s back to %s tiredly.", GET_SHORT(collector), OBJ_SHORT(order), GET_NAME(ch));
        act(buf,0,collector,0,ch,TO_NOTVICT);
        sprintf(buf, "%s tosses %s back to you tiredly.", GET_SHORT(collector), OBJ_SHORT(order));
        act(buf,0,collector,0,ch,TO_VICT);
        return TRUE;
      }
    }
    else { // given non-AQ_ORDER
      do_say(collector, "Thanks, I guess?", CMD_SAY);
    }
  }

  CHAR *vict, *next_vict;
  bool spawn_uber = TRUE;
  // dawn of the Ubers - Hemp 2017-07-07
  if (cmd == MSG_ZONE_RESET) {
    if (chance(10)) {
      // pick an Uber to load
      uber_choice = number(0, NUMELEMS(ubers)-1);
      uber_room = ubers[uber_choice][1];

      for( vict = world[real_room(uber_room)].people; vict; vict = next_vict ) {
        // don't load if there already is one or a PC in room
        next_vict = vict->next_in_room;
        if( V_MOB(vict) == ubers[uber_choice][0] || !IS_NPC(vict) ) {
          spawn_uber = FALSE;
          break;
        }
      }
      if (spawn_uber) {
        uber_mob = read_mobile(ubers[uber_choice][0], VIRTUAL);
        char_to_room(uber_mob, real_room(uber_room));
        sprintf(buf, "A dazzling light and ear-splitting sound warp the space around you as %s emerges from a Planar Gate.\n", GET_SHORT(uber_mob));
        send_to_room(buf, real_room(uber_room));
        sprintf(buf, "%s! Reports abound that an uber version of a creature from our realm has appeared - quick, to arms!",  collectorexclamation[ number( 0, NUMELEMS( collectorexclamation ) -1 ) ] );
        do_quest(collector, buf, CMD_QUEST);
       }
    }
  }
  return FALSE;
}

bool is_order_item(int vnum) {
  for (int i = 0; i < NUMELEMS(aq_obj_master_list); i++) {
    if (vnum == aq_obj_master_list[i][0]) {
      return TRUE;
    }
  }

  return FALSE;
}

void assign_aquest_special(void) {
  assign_obj(TEMPLATE_AQORDER, aq_order_obj);
  assign_mob(COLLECTOR, aq_order_mob);
}
