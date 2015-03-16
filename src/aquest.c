/*
** aquest.c
**   Routines for autoquester
**
** Written for RoninMUD by Ranger
**
** Do not distribute without permission.
*/

/*
$Author: ronin $
$Date: 2004/05/05 20:40:57 $
$Header: /home/ronin/cvs/ronin/aquest.c,v 2.1 2004/05/05 20:40:57 ronin Exp $
$Id: aquest.c,v 2.1 2004/05/05 20:40:57 ronin Exp $
$Name:  $
$Log: aquest.c,v $
Revision 2.1  2004/05/05 20:40:57  ronin
Changed Greater Oni aquest to 4.
Added 1% chance of getting a double aquest.

Revision 2.0.0.1  2004/02/05 16:08:50  ronin
Reinitialization of cvs archives

Revision 1.5  2003/01/25 16:46:24  ronin
Quest fail on quit lowered from 30 to 25 ticks.
Revision - removed Pan from aquests, as he's now bard sc-master.

Revision - chief of guards removed from aquest due to his fleeing
Revision - increased attainable natural stats to 22

Revision 1.4  2002/07/26 16:46:24  ronin
Increased the point vale of the abyss mobs and a few others.

Revision 1.3  2002/05/29 04:58:54  ronin
Change of some aquest mobs and addition of 3 more.

Revision 1.2  2002/03/31 07:42:14  ronin
Addition of header lines.

$State: Exp $
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

extern struct descriptor_data *descriptor_list;

extern struct obj_data  *object_list;
extern int top_of_world;
extern int top_of_zone_table;

int check_guildmaster(CHAR *ch, CHAR *mob) {
  if(!ch) return TRUE;
  if(!IS_NPC(ch) && GET_CLASS(ch)==GET_CLASS(mob)) return TRUE;
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
  char arg[MAX_INPUT_LENGTH],buf[MAX_INPUT_LENGTH];
  char usage[]="\
This command is used to handle automatic questing from guildmasters.\n\r\n\r\
  Usage: aquest request  - request a quest (solo/low/mid/high optional)\n\r\
                card <#> - request a questcard hunt (# options: 1/2/3/4/5)\n\r\
                complete - get credit for the quest\n\r\
                info     - get info on current quest status\n\r\
                quit     - quit your current quest\n\r\
                list     - list things to buy\n\r\
                buy      - buy things with quest points\n\r";
  int option=0;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }
  if(is_abbrev(arg,"quit")) option=QUEST_QUIT;
  if(is_abbrev(arg,"request")) option=QUEST_REQUEST;
  if(is_abbrev(arg,"complete")) option=QUEST_COMPLETE;
  if(is_abbrev(arg,"info")) option=QUEST_STATUS;
  if(is_abbrev(arg,"list")) option=QUEST_LIST;
  if(is_abbrev(arg,"buy")) option=QUEST_BUY;
  if(is_abbrev(arg,"card")) option=QUEST_CARD;

  switch(option) {
    case QUEST_STATUS:
      if(ch->quest_status==QUEST_NONE) {
        send_to_char("You are not currently on any quest.\n\r",ch);
        if(ch->ver3.time_to_quest)
          sprintf(buf,"You must wait %d ticks to start another.\n\r",ch->ver3.time_to_quest);
        else
          sprintf(buf,"You can start another at any time.\n\r");
        send_to_char(buf,ch);
        return;
      }

      if(ch->quest_status==QUEST_RUNNING) {
        if(ch->questmob)
          sprintf(buf,"You are on a quest to kill %s, for %d point(s).  You have %d ticks left.\n\r",GET_SHORT(ch->questmob),ch->quest_level,ch->ver3.time_to_quest);
        else if(ch->questobj)
        {
          if(V_OBJ(ch->questobj) == 35)
            sprintf(buf,"You are on a quest to recover %d %s, for %d point(s). You have %d ticks left.\n\r",aq_card[ch->quest_level-1],OBJ_SHORT(ch->questobj),ch->quest_level,ch->ver3.time_to_quest);
          else
            sprintf(buf,"You are on a quest to recover %s, for %d point(s). You have %d ticks left.\n\r",OBJ_SHORT(ch->questobj),ch->quest_level,ch->ver3.time_to_quest);
        }
        else
          sprintf(buf,"You are on a quest to do something, dunno what it is.\n\r");
        send_to_char(buf,ch);
        return;
      }

      if(ch->quest_status==QUEST_COMPLETED) {
        send_to_char("You have completed your quest.  Return to the quest giver for credit.\n\r",ch);
        return;
      }

      if(ch->quest_status==QUEST_FAILED) {
        send_to_char("You have failed your quest for some reason.\n\r",ch);
        if(ch->ver3.time_to_quest)
          sprintf(buf,"You must wait %d ticks to start another.\n\r",ch->ver3.time_to_quest);
        else
          sprintf(buf,"You can start another at any time.\n\r");
        send_to_char(buf,ch);
        return;
      }
      break;

    case QUEST_QUIT:
      if(ch->quest_status==QUEST_NONE)
        send_to_char("You aren't on any quest.\n\r",ch);
      else if(ch->quest_status==QUEST_FAILED) {
        ch->questgiver=0;
        if(ch->questobj) ch->questobj->owned_by=0;
        ch->questobj=0;
        if(ch->questmob) ch->questmob->questowner=0;
        ch->questmob=0;
        ch->quest_status=QUEST_NONE;
        ch->quest_level=0;
        printf_to_char(ch,"You have failed your quest, you can start another in %d ticks.\n\r",ch->ver3.time_to_quest);
      }
      else {
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
#ifndef TESTSITE
        if(GET_LEVEL(ch)<LEVEL_IMM) ch->ver3.time_to_quest=25;
        else ch->ver3.time_to_quest=0;
#else
        ch->ver3.time_to_quest=0;
#endif
        printf_to_char(ch,"There, you have quit your quest, you can start another in %d ticks.\n\r",ch->ver3.time_to_quest);
      }
      return;
      break;
    case QUEST_COMPLETE:
    case QUEST_REQUEST:
    case QUEST_LIST:
    case QUEST_BUY:
    case QUEST_CARD:
      send_to_char("You must return to your guildmaster.\n\r",ch);
      return;
      break;
    default:
      send_to_char(usage,ch);
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

      argument=one_argument(argument,arg);
      if(*arg) {
        if(is_abbrev(arg,"solo")) lh_opt=1;
        if(is_abbrev(arg,"low")) lh_opt=2;
        if(is_abbrev(arg,"high")) lh_opt=3;
        if(is_abbrev(arg,"mid")) lh_opt=4;
      }
      if(!generate_quest(ch, mob,lh_opt)) {
        act("$N tells you, 'Sorry, I failed to find a quest for you.'",0,ch,0,mob,TO_CHAR);
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


int generate_quest(CHAR *ch, CHAR *mob,int lh_opt) {
  char buf[MAX_INPUT_LENGTH];
  CHAR *vict;
  /*OBJ *obj;*/
  int pick,rmob,count=0;
  int mobs[][2]={
{110,0}, /* chief sprite */
{1100,0}, /* elven wizard */
{1116,0}, /* elven warrior */
{1117,0}, /* dwarven prince */
{1303,0}, /* gargon */
{2306,0}, /* white king */
{2315,0}, /* black king */
{2634,0}, /* zoo keeper zookeeper head */
{2645,0}, /* alligator gator crocodile creature swamp */
{2807,0}, /* negress */
{2912,0}, /* fire newt leader */
{2915,0}, /* phoenix */
{2916,0}, /* lizard (an enormous lizard) */
{3730,0}, /* sister (Mayor's Sister) */
{4515,0}, /* troll large crab herder */
{4517,0}, /* tilik small pixie */
{4704,0}, /* Halfling Zombie */
{4809,0}, /* bright wizard */
{4811,0}, /* grey wizard */
{5303,0}, /* captain (Griffon Captain) */
{6000,0}, /* john lumberjack */
{6114,0}, /* queen spider */
{6115,0}, /* shargugh brownie */
{7009,0}, /* gnoll chieftain */
{7315,0}, /* bandit leader */
{7324,0}, /* nomad male */
{7334,0}, /* fido dog (A scruffy Fido) */
{7401,0}, /* guard beast */
{8206,0}, /* whale */
{8321,0}, /* boy cabin john thomas */
{8611,0}, /* prince */
{8901,0}, /* king welmar */
{9619,0}, /* unicorn */
{10008,0}, /* gargoyle */
{11310,0}, /* master burgonmaster */
{11607,0}, /* sailor elven */
{20805,0}, /* Osquip */
{20811,0}, /* Repairman Jocko */
{20821,0}, /* Fungus King */
{506,1}, /* Farmer */
{508,1}, /* Tea Master */
{542,1}, /* tengu leaflet */
{702,1}, /* small fairy */
{805,1}, /* Mummy (First Ancient Mummy) */
{806,1}, /* mummy (Second Ancient Mummy) */
{807,1}, /* mummy (Third Ancient Mummy) */
{1307,1}, /* overlord */
{1602,1}, /* turtle giant sea */
{1604,1}, /* chimp chmipanzee */
{1605,1}, /* ape giant */
{1909,1}, /* orak */
{1926,1}, /* dragon black */
{1933,1}, /* zyekian warder lorin */
{2023,1}, /* scorpio */
{2027,1}, /* pisces mermaid */
{2103,1}, /* merchant */
{2510,1}, /* rabbit (cute rabbit) */
{3405,1}, /* priest */
{4060,1}, /* captain orc orcish */
{4108,1}, /* mage orc arch orcish wizard */
{4109,1}, /* nim orc orcish statue */
{4410,1}, /* Gatekeeper Guard */
{4435,1}, /* milhouse star roach */
{4437,1}, /* Rizzo the cook */
{4513,1}, /* goblin grug pale creature */
{4806,1}, /* averland */
{4807,1}, /* ostermark */
{4808,1}, /* middenheim */
{5158,1}, /* rock elemental */
{5407,1}, /* stheno gorgon */
{5408,1}, /* euryale gorgon */
{5409,1}, /* medusa gorgon */
{6110,1}, /* tree ancient */
{7002,1}, /* ripper statue */
{7336,1}, /* master light */
{7402,1}, /* beast (The Beast) */
{7503,1}, /* pirate observer */
{8203,1}, /* neptune */
{8317,1}, /* pirate captain lost */
{8335,1}, /* Grand Poobah */
{8401,1}, /* aldor warlock */
{8506,1}, /* worm sand */
{8508,1}, /* lost fisherman */
{8509,1}, /* kingfisher bird */
{8607,1}, /* knight (The Knight with Three Heads) */
{8615,1}, /* rabbit (a Cute Little Rabbit) */
{9010,1}, /* elf chief elven guard */
{9012,1}, /* elf elven champion master */
{9505,1}, /* wolf queen */
{10318,1}, /* monk grand master human */
{10412,1}, /* tephonal son prince */
{11327,1}, /* jander vampire */
{11617,1}, /* elven mystic */
{11706,1}, /* ninja guardian */
{12022,1}, /* oak stoned huge */
{14001,1}, /* apparition girl marianne */
{14002,1}, /* apparition girl susanne */
{14003,1}, /* apparition girl annette */
{14518,1}, /* pixie glittering dust */
{16569,1}, /* werner curator */
{16690,1}, /* Aerial servant */
{16904,1}, /* spectre spirit trembling */
{20857,1}, /* Supreme Slug */
{26475,1}, /* protector */
{26476,1}, /* adept */
{27113,1}, /* cannibal warrior */
{27651,1}, /* king monkey monkeys */
{27662,1}, /* blood goddess */
{555,2}, /* Oni lesser */
{596,2}, /* monk priest sohei */
{713,2}, /* troll mother */
{716,2}, /* troll mother */
{804,2}, /* Tut */
{2029,2}, /* bear ursa major */
{2030,2}, /* cassiopeia queen */
{2809,2}, /* doctor */
{4110,2}, /* morian protector guard guardian */
{4484,2}, /* first lieutenant jobs */
{4516,2}, /* toxic young black dragon */
{5414,2}, /* aeacus */
{5415,2}, /* radamanthus */
{5416,2}, /* minos */
{5593,2}, /* black skeleton */
{5705,2}, /* golem statue stone akinra */
{7501,2}, /* pirate first mate */
{8360,2}, /* alchemist mad yuri madman */
{8406,2}, /* dragon (The Flying Dragon) */
{9504,2}, /* wolf king */
{10017,2}, /* Calcifer */
{10417,2}, /* genschleng were crocodile werecrocodile */
{11324,2}, /* soth lord */
{11504,2}, /* fei lien */
{11507,2}, /* shang-ti */
{12003,2}, /* ant gatemaster guard */
{12008,2}, /* dragon guard */
{12014,2}, /* cobra king */
{12026,2}, /* ancient crocodile cro */
{12209,2}, /* gigantic mushroom mus */
{12905,2}, /* mystic great */
{12911,2}, /* kansatsu mystic brother elder */
{12916,2}, /* yagyu mystic brother elder */
{13011,2}, /* owlbear */
{14000,2}, /* ghost father man figure */
{14502,2}, /* Yorla sayer truth hag old wretched */
{16507,2}, /* ancient wax knight faze */
{16513,2}, /* proto-horse proto horse wax */
{16526,2}, /* jenny consort girl */
{16803,2}, /* revenant form */
{16804,2}, /* hunter arctic man squat */
{17300,2}, /* crystal golem statue warrior */
{17301,2}, /* phasteus cat tiger */
{20163,2}, /* priest zen */
{20183,2}, /* Sensei Ryo */
{20840,2}, /* Cloaker Lord */
{21201,2}, /* cave ogre monster */
{21209,2}, /* terral castle guard */
{21218,2}, /* grydon guard gates castle */
{24005,2}, /* guardian tomb ancient statue */
{24900,2}, /* Dark Druid Guardian */
{26405,2}, /* statue demon */
{26479,2}, /* devotee */
{27100,2}, /* Centurion */
{27109,2}, /* tyrannosaurus rex */
{27112,2}, /* quazit */
{27699,2}, /* El Diablo */
{27700,2}, /* Mountain Yeti */
{28502,2}, /* minotaur bull man */
{701,3}, /* racti troll hermit */
{706,3}, /* juktoa troll foreman */
{3919,3}, /* celestial dragon */
{4447,3}, /* Gentle ben filthy inmate */
{4463,3}, /* doctor jacobs */
{4464,3}, /* gypsy alice chains prisoner */
{4465,3}, /* Gaarn were badger beast */
{4466,3}, /* drow drider */
{4483,3}, /* Captain Guard */
{4608,3}, /* nose hair */
{4706,3}, /* Garbage Golem */
{6201,3}, /* the Unholy Deacon */
{8361,3}, /* wind dust elemental */
{11712,3}, /* master pagoda */
{12005,3}, /* dragon ancient huge dra anc */
{12009,3}, /* lost adventurer */
{12021,3}, /* ettin wizard mage ett */
{12201,3}, /* cleric ettin et cl */
{13017,3}, /* tree ant treeant */
{16508,3}, /* wax knight bill janitor */
{17001,3}, /* atropos doctor agent */
{17003,3}, /* sand beast pile */
{20130,3}, /* warhorse skeletal */
{21109,3}, /* root tree large */
{21207,3}, /* aldrene bard lady singer */
{21334,3}, /* bugbear bug bear */
{24903,3}, /* druid protector */
{26403,3}, /* guide */
{26404,3}, /* seeress mistress */
{26702,3}, /* ghost prisoner (The Ship's Ghost) */
{27101,3}, /* roman tribune marcus aurelius */
{27102,3}, /* Damoclese the Gladiator */
{27106,3}, /* lord bundolo */
{27715,3}, /* gelugon guardian guard demon */
{501,4}, /* Oni Greater */
{540,4}, /* Tanoshi Wrestler */
{552,4}, /* Sojobo tengu king */
{598,4}, /* shukenja grand priest */
{703,4}, /* neyuv lizard assassin reptile */
{4469,4}, /* animate skeleton */
{4472,4}, /* Voldra Sage */
{4601,4}, /* gigantic eye */
{4612,4}, /* Kitzanti Captain Dark Purple */
{4703,4}, /* voodoo doll */
{4707,4}, /* Cleric Werra Garbage */
{5801,4}, /* slaphoff kender captain */
{6273,4}, /* ant lion */
{6298,4}, /* cave bear */
{7500,4}, /* pirate captain */
{7507,4}, /* cook old chinese */
{10907,4}, /* zyca */
{11702,4}, /* Chun Lui gong */
{12012,4}, /* king spider spi */
{12202,4}, /* thief ettin th et */
{12203,4}, /* rat giant */
{12811,4}, /* mankey pet */
{12904,4}, /* mystic ultimate */
{13501,4}, /* oglozt greater */
{14508,4}, /* eduard magistrate wererat grotesque rat */
{16515,4}, /* gorgo fur beast */
{17005,4}, /* marten man */
{17006,4}, /* maerlyn sorcerer wizard */
{17007,4}, /* oracle spirit */
{17330,4}, /* dwarf smith prisoner ragar */
{20108,4}, /* Miyamoto Musashi */
{20129,4}, /* Spectral Warlord */
{20165,4}, /* black panther */
{21203,4}, /* king morian moria ruler mandrial */
{25018,4}, /* elemental water prince */
{25019,4}, /* earth elemental king */
{25020,4}, /* air elemental lord */
{25021,4}, /* fire elemental sultan */
{25035,4}, /* demon balor */
{26401,4}, /* vizier */
{26481,4}, /* healer */
{27105,4}, /* cannibal witchdoctor */
{27720,4}, /* myrdon thief rogue master */
{27721,4}, /* shadowraith ninja assassin jal pur */
{700,5}, /* sakdul large troll gypsy */
{4600,5}, /* Neuron Beast Strands */
{5105,5}, /* drow apprentice */
{5107,5}, /* drow matron mother lower */
{5140,5}, /* spider sentry first */
{5177,5}, /* Dgarzah Drow rogue Leader */
{5184,5}, /* spider sentry third */
{5189,5}, /* spider sentry fourth */
{5191,5}, /* spider sentry second */
{5596,5}, /* myconid king mushroom */
{11326,5}, /* vampire strahd count */
{13019,5}, /* elf elven master beastmaster */
{17004,5}, /* twixt bard man master */
{17010,5}, /* minion lesser */
{17308,5}, /* marcus wizard mage */
{20145,5}, /* Shogun Warlord Samurai */
{21210,5}, /* priest high dark man */
{21332,5}, /* otyugh stench garbage pile vines */
{25002,5}, /* death crimson */
{25013,5}, /* kalas */
{26402,5}, /* emir malik */
{26482,5}, /* magus */
{27712,5}, /* bebilith stalker purple spider insect */
{4605,6}, /* worm heart */
{5125,6}, /* drow matron mother third */
{5126,6}, /* drow matron mother fourth */
{5127,6}, /* drow matron mother second */
{5132,6}, /* drow leader varrn */
{5553,6}, /* Garaek drow drider overseer */
{5901,6}, /* drow leader rezik */
{7703,6}, /* typik lizard shaman reptile */
{11514,6}, /* wyvern */
{14501,6}, /* keira banshee ghost */
{17002,6}, /* vermilion king */
{17342,6}, /* troll cook chef */
{20107,6}, /* Raiden */
{25010,6}, /* kraken */
{-1, -1} /* PADDING */
};

  while(count<200) {
    count++;
    pick=number(0,((sizeof(mobs)/sizeof(mobs[0])) - 2));
    rmob=real_mobile(mobs[pick][0]);
    if(rmob==-1) continue;
    if(mob_proto_table[rmob].number < 1) continue;
    if(!(vict=get_ch_world(mobs[pick][0]))) continue;
    if(vict->questowner) continue;
    if(GET_LEVEL(ch) < 25 && lh_opt==0 && mobs[pick][1] > 0) continue; /*newbie*/
    if(GET_LEVEL(ch) >=25 && mobs[pick][1] == 0) continue; /*non-newbie*/
    if(lh_opt==1 && mobs[pick][1]>1) continue; /*solo*/
    if(lh_opt==2 && mobs[pick][1]>3) continue; /*low*/
    if(lh_opt==3 && mobs[pick][1]<3) continue; /*high*/
    if((lh_opt==4 && mobs[pick][1]<2) || (lh_opt==4 && mobs[pick][1]>4)) continue; /*mid*/
    ch->quest_level= (mobs[pick][1] == 0) ? 1 : mobs[pick][1];
    ch->questmob=vict;
    ch->questobj=0;
    ch->quest_status=QUEST_RUNNING;
    vict->questowner=ch;
    ch->questgiver=mob;
    ch->ver3.time_to_quest=60;
    sprintf(buf,"$N tells you, 'Kill %s for %d quest point(s), in 60 ticks.'",GET_SHORT(vict),ch->quest_level);
    act(buf,0,ch,0,mob,TO_CHAR);
    sprintf(buf,"$N tells you, '%s can be found around %s'",GET_SHORT(vict),world[CHAR_REAL_ROOM(vict)].name);
    act(buf,0,ch,0,mob,TO_CHAR);
    if(chance(2)) {
      act("$N tells you, 'Its your lucky day!  I'm going to double your quest point reward!'",0,ch,0,mob,TO_CHAR);
      ch->quest_level*=2;
    }
    return TRUE;
  }
  wizlog("WIZINFO: Quest counter exceeded 200",LEVEL_IMP,5);
  log_f("WIZINFO: Quest counter exceeded 200");
  return FALSE;

/*  else {
    Object questing not implemented.  If it is added must add a check
    for someone else picking up the quest obj (or block anyone but the
    quester from picking up the object)

    obj=read_object(3,VIRTUAL);
    obj_to_room(obj,CHAR_REAL_ROOM(mob));
    ch->quest_level=1;
    ch->questmob=0;
    ch->questobj=obj;
    ch->quest_status=QUEST_RUNNING;
    obj->owned_by=ch;
    ch->questgiver=mob;
    ch->ver3.time_to_quest=60;
    sprintf(buf,"$N tells you, 'Get the $q for %d quest point(s), in 60 ticks.'",ch->quest_level);
    act(buf,0,ch,obj,mob,TO_CHAR);
    return TRUE;
  }*/
}

/* Object questing implemented through mob other than guildmaster in this case */

#define TEMPLATE_AQORDER  40

int aq_objs[][2]={ // { OBJ_vnum, value }
  {1, 1}, // TEMP steak
  {2, 2}, // TEMP beautiful rose
  {3, 3}, // TEMP postcard
  {4, 4}, // TEMP small package
  {10,4}, // TEMP gold wedding band
  {-1, -1} // PADDING
};

/*
int generate_aq_order(CHAR *requester) {
  time_t seconds = time(NULL);     // current time in seconds since Jan 1, 1970
  int days = seconds / (60*60*24); // days since Jan 1, 1970 : seconds * minutes * hours = days
  OBJ *aqorder;
  char buf[MAX_INPUT_LENGTH];
  int pick, j, i = 0, count = 0, value = 0; 

  aqorder = read_object(TEMPLATE_AQORDER, VIRTUAL);
  if(requester->ver3.id <= 0)
    requester->ver3.id = generate_id();
  aqorder->ownerid[0] = requester->ver3.id;   // tag the order with the requester's ID
  sprintf(buf, "acquisition order %s", GET_NAME(requester));
  aqorder->name = str_dup(buf);               // tag the item with the requester's name as a keyword
  sprintf(buf, "an acquisition order for %s", GET_NAME(requester));
  aqorder->short_description = str_dup(buf);  // change the short desc to include requester's name
  sprintf(buf, "An infamous acquisition order, forgotten here by %s.", GET_NAME(requester));
  aqorder->description = str_dup(buf);        // change the long desc to include requester's name
  aqorder->log = 1;
  
  for (i; i < 4; i++) { // get an obj from our aq obj table, assign it to the aqorder value[]
  {
    while(count < 50) {
      count++;
      pick = number(0, (( sizeof(aq_objs) / sizeof(aq_objs[0]) ) - 2));
      j = 0
      for (j; j < i; j++) {
        if (aqorder->flags.value[j] == aq_objs[pick][0]) 
          continue; // if that object is already in the aqorder
        // OPTION: add choice filter here
          //  if(lh_opt==1 && mobs[pick][1]>1) continue; // solo
          //  if(lh_opt==2 && mobs[pick][1]>3) continue; // low
          //  if(lh_opt==3 && mobs[pick][1]<3) continue; // high
          //  if((lh_opt==4 && mobs[pick][1]<2) || (lh_opt==4 && mobs[pick][1]>4)) continue; // mid
      }
      aqorder->flags.value[i] = aq_objs[pick][0];
      value += aq_objs[pick][1];
    }
    OBJ_SPEC(aqorder) = value;   // tag the order with the value it's worth (returned in ID)
    obj_to_char(aqorder, requester); 
    return value;                // so we can tell them how much it'll be worth
  }
  wizlog("WIZINFO: AQ Order object assignment counter exceeded 50.",LEVEL_IMP,5);
  log_f("WIZINFO: AQ Order object assignment counter exceeded 50.");
  return FALSE;  
} // end of generate_aq_order
*/

int aq_order_obj (OBJ *order, CHAR *ch, int cmd, char *arg) {
  ch = order->carried_by;
  
  // block methods of getting order onto another char and/or out of game
  // exclude gods
  if (cmd == CMD_GIVE) { // if target is PC w/o right charID
  } else if (cmd == CMD_GET || cmd == CMD_TAKE) { // if picker-upper is PC w/o right charID
  } else if (cmd == CMD_DONATE || cmd == CMD_DROP || cmd == CMD_JUNK || cmd == CMD_STORE) {
  }
  return FALSE;
}

int aq_obj_date_popped (OBJ *aqobj, CHAR *ch, int cmd, char *arg) {
  time_t seconds = time(NULL);     // current time in seconds since Jan 1, 1970
  int days = seconds / (60*60*24); // days since Jan 1, 1970 : seconds * minutes * hours = days
  
  ch = aqobj->carried_by;

  // to cover our bases here we need to tag: coming out of vaults, characters, loaded by a mob, appears when skinned, loaded by god
  if (aqobj->obj_flags.timer == 0 &&
      ((cmd == MSG_OBJ_ENTERING_GAME) ||        // covers coming out of vaults, rent
      (cmd == MSG_OBJ_WORN) ||                  // covers popping and wearing
      (cmd == MSG_BEING_REMOVED) ||             // covers already equipped
      ((cmd == MSG_MOBACT) && IS_MORTAL(ch)) || // covers popping and holding
      (cmd == MSG_GAVE_OBJ) ||                  // >
      (cmd == MSG_OBJ_PUT) ||                   // everything else
      (cmd == MSG_OBJ_DONATED))) {              // >
    // set the timer as today's date
    aqobj->obj_flags.timer = days;
    //TESTING ---- need to test order on a mob, at mobact
    //char buf[MAX_STRING_LENGTH];  
    //sprintf(buf, "CMD was %d, Days is %d\n\r", cmd, days);
    //send_to_world(buf);
    //END TESTING
  }  
  return FALSE;
}

//int aq_order_mob (CHAR *collector, CHAR *ch, int cmd, char *arg) {
//}

void assign_aquest_special(void) {
  int i;
  
  assign_obj( TEMPLATE_AQORDER, aq_obj_date_popped );
  for (i = 0; i < ((sizeof(aq_objs) / sizeof(aq_objs[0]))); i++)
    assign_obj( i, aq_obj_date_popped );
  //assign_mob( COLLECTOR, aq_order_mob );
}
