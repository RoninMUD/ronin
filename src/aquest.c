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

extern struct idname_struct idname[MAX_ID];
extern void mob_do(CHAR* mob, char *arg);
extern OBJ  *object_list;

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
  char arg[MAX_INPUT_LENGTH],buf[MAX_INPUT_LENGTH];
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
  if(is_abbrev(arg,"order")) option=QUEST_ORDER;

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
    case QUEST_ORDER:
      send_to_char("You'll have to find someone who has those.\n\r",ch);
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
{4601,3}, /* gigantic eye */
{4605,3}, /* worm heart */
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
int aq_objs[][2] = {
  // {VNUM, value} // <short desc> <repop>
  {104, 1}, // A small stethoscope 50
  {106, 1}, // A small wand 50
  {508, 1}, // Porcelain Teacup 25
  {543, 1}, // a Tengu Cloak of Straw 7
  {709, 1}, // A Heavy Stone Club 46
  {1107, 1}, // mithril a1e 50
  {1108, 1}, // the thain girth 50
  {1110, 1}, // an egg 50
  {1301, 1}, // An ivory headband 50
  {1608, 1}, // a woven vine sash 20
  {1910, 1}, // a blued-steel falchion 50
  {1918, 1}, // a red apple 50
  {2021, 1}, // the Sceptre of Might 50
  {2101, 1}, // a cloak of deception 25
  {2103, 1}, // rapier 25
  {2305, 1}, // a white crosier 50
  {2310, 1}, // a black crosier 50
  {2502, 1}, // wooden spear 50
  {2506, 1}, // a black mask 50
  {2700, 1}, // a ragged cotton shirt 50
  {2806, 1}, // a book of songs 50
  {2807, 1}, // a book of magic 50
  {2808, 1}, // a book of wars 50
  {2809, 1}, // a book of maths 50
  {2810, 1}, // a book of languages 50
  {2811, 1}, // a book of jokes 50
  {2812, 1}, // a book of history 50
  {2911, 1}, // A Large Broada1e 15
  {2913, 1}, // A Large Spiked Chain Mace 15
  {3414, 1}, // a whisper thin rapier 50
  {3425, 1}, // An auburn-colored orb 50
  {3701, 1}, // A Silver Necklace 20
  {4051, 1}, // a metal helmet 35
  {4100, 1}, // A Black Demon's Blade 50
  {4400, 1}, // a long silk skirt 11
  {4812, 1}, // Averland greatsword 50
  {4813, 1}, // Ostermark greathammer 50
  {4814, 1}, // Middenheim greatdagger 50
  {5301, 1}, // field rations 50
  {5601, 1}, // a bearskin 50
  {5717, 1}, // A bit of wrapping 50
  {5809, 1}, // bratwurst 50
  {6001, 1}, // a chequered shirt 50
  {6112, 1}, // an iron crown 50
  {6113, 1}, // an iron sceptre 50
  {6506, 1}, // long
  {7003, 1}, // An evil sabre 50
  {7310, 1}, // A Curved sword 50
  {8011, 1}, // sparkling lightening sword 50
  {8202, 1}, // a sealskin jacket 50
  {8323, 1}, // the Cabin Boy's Sponge 50
  {8333, 1}, // a seaweed loincloth 50
  {8402, 1}, // The Dagger of Aldor 17
  {8403, 1}, // a chef's hat 25
  {8503, 1}, // Feathers of the Kingfisher 50
  {8510, 1}, // Ragged Sleeves 50
  {8602, 1}, // a Coconut 100
  {8603, 1}, // The Holy Grail 10
  {8605, 1}, // a Helmet of the Black Knight 50
  {8920, 1}, // some Russian caviar 50
  {9004, 1}, // a Two-handed Mithril Sword 10
  {9414, 1}, // some platinum leg bands 50
  {9421, 1}, // a lodestone 50
  {9515, 1}, // A crystal  50
  {10903, 1}, // a steel ring 50
  {11062, 1}, // a devil's trident 50
  {11068, 1}, // a black cape 50
  {11313, 1}, // a crystal ball 50
  {11315, 1}, // cloak of sadness 10
  {11618, 1}, // The robe of Aman 10
  {14008, 1}, // a rosette bracelet 50
  {16514, 1}, // A hardened wa1 breast plate 20
  {16620, 1}, // a flaming whip 40
  {16681, 1}, // Boots of Water Walking 50
  {19009, 1}, // a wool sweater 11
  {20820, 1}, // Stick of Fungus Powers 25
  {21316, 1}, // A Rotten Dwarven Leg 3
  {26408, 1}, // A Crackled Black Urn 10
  {26409, 1}, // A Flying Carpet 10
  {26411, 1}, // lucky ivory die 30
  {27100, 1}, // a Centurion's helmet 13
  {27621, 1}, // The necklace of shrunken heads 35
  {28506, 1}, // A Jeweled Tiara 60
  {201, 2}, // A thick cudgel 58
  {593, 2}, // An ancient journal 100
  {803, 2}, // Tattered Leggings 30
  {910, 2}, // The Rebel's Breastplate 17
  {1604, 2}, // a metal hoop 30
  {2023, 2}, // Great Dipper of the Skies 50
  {2606, 2}, // A pair of alligator hide sleeves 19
  {4053, 2}, // An Orcish Whip 20
  {4107, 2}, // A Marble Pendant 40
  {4448, 2}, // a shield made from solid oak 18
  {4454, 2}, // An Engraved Eagle Breastplate 30
  {4464, 2}, // iron shackles and chains 20
  {4465, 2}, // A Were-Badger's Mane 21
  {4477, 2}, // An Adamanite Scimitar 30
  {4508, 2}, // Boots of Striding 10
  {4510, 2}, // A Mushroom-Top Loincloth 10
  {4700, 2}, // a small voodoo doll 3
  {4701, 2}, // the loves of Leoric 9
  {5200, 2}, // A ringmail vest 50
  {5203, 2}, // Some iron sleeves 50
  {5208, 2}, // A sturdy iron shield 50
  {5427, 2}, // The shield of Pallas 50
  {5435, 2}, // a jeweled ring 50
  {5436, 2}, // a crown of dreams 50
  {5449, 2}, // a pair of sandals 50
  {5454, 2}, // the brassard of the hydra 50
  {5455, 2}, // A heavy metal wrist band 50
  {5802, 2}, // Utility belt 25
  {6210, 2}, // a priest's skirt 10
  {6519, 2}, // a box of birth control pills 50
  {7334, 2}, // An Obsidian shard. 10
  {7506, 2}, // a silk scarf 15
  {8335, 2}, // A Rusted Iron Gauntlet 50
  {8337, 2}, // a long white coat 50
  {8406, 2}, // a Dragon's Scale 50
  {8513, 2}, // Fishing Line 50
  {9307, 2}, // Gold Dragon Shield 50
  {9308, 2}, // A black leather belt with dangling skulls 50
  {9309, 2}, // A  with arcane symbols 50
  {9321, 2}, // An Arcane Runed Robe 50
  {9521, 2}, // a broken ring 50
  {10022, 2}, // Obsidian Ring 5
  {10405, 2}, // a Magical Eyeglass 65
  {10409, 2}, // Colossal Claw of the Greater Lycanthrope 10
  {10410, 2}, // Belt of the Class Hero 30
  {11069, 2}, // a pair of black pants 50
  {11072, 2}, // a black belt 40
  {11316, 2}, // breast plate of Black rose 10
  {11507, 2}, // the Hammer of Lei Kung 10
  {11533, 2}, // the  of Shang-Ti 10
  {12026, 2}, // Ring of Wizardry 50
  {12028, 2}, // Snakeskin Belt 10
  {12035, 2}, // dragon skin leggings 50
  {12036, 2}, // dragon skin sleeves 50
  {12042, 2}, // blue gem 50
  {12201, 2}, // Crystal Lantern 50
  {12202, 2}, // Crystal Shield 50
  {12911, 2}, // a blue silk shirt 50
  {12915, 2}, // a blue silk garb 20
  {13018, 2}, // a wooden seed 20
  {13019, 2}, // a Heart-Shaped Amulet 20
  {14001, 2}, // a small bottle of ink 50
  {14028, 2}, // Shield of Ancient Powers 25
  {14037, 2}, // a black scarf 50
  {14503, 2}, // a Blood-Red Amulet 12
  {16508, 2}, // An emerald muslin sash 43
  {16807, 2}, // a tattered shroud 22
  {16904, 2}, // A shattered headstone 100
  {17021, 2}, // A jagged scalpel 30
  {20840, 2}, // a shield of mottled flesh 12
  {20856, 2}, // a bracelet of slug entrails 30
  {21321, 2}, // A Pair of Gleaming Gauntlets 2
  {21322, 2}, // A Suit of Red Scale Mail 1
  {24900, 2}, // druidic battle wrap 14
  {25017, 2}, // Sword of Sharpness 50
  {26475, 2}, //  The Gloves Of Siva 10
  {26476, 2}, //  A Jade Ring 20
  {27103, 2}, // A white toga 75
  {27104, 2}, // Cestus 23
  {27113, 2}, // the thick hide of a dinosaur 23
  {27651, 2}, // the jade monkey 25
  {28503, 2}, // Pan's Flute 15
  {702, 3}, // a pair of baggy pants 10
  {3907, 3}, // The tail of the Celestial Dragon 10
  {4604, 3}, // the Finger of Thievery 10
  {4608, 3}, // a Parasite's Fang 5
  {4704, 3}, // Werra's Belt of Garbage 5
  {5105, 3}, // a jeweled scimitar 12
  {5110, 3}, // A tome of forbidden magic 8
  {5804, 3}, // Pair of red lederhosen 20
  {5920, 3}, // Myconid blood 15
  {6208, 3}, // a chitonous exoskeleton 10
  {6215, 3}, // a pair of bearskin sleeves 10
  {6217, 3}, // a flute made of bone 8
  {6802, 3}, // leg plates of devotion 50
  {6803, 3}, // buckler of obeisance 50
  {7502, 3}, // a dueling pistol 50
  {7520, 3}, // a butchering knife 30
  {7701, 3}, // a heavy crocodile skin 100
  {8361, 3}, // a Granite  20
  {10905, 3}, // a pair of Zyca arm plates 10
  {10908, 3}, // a Zyca Orb 50
  {11314, 3}, // bracelet of magic 10
  {11704, 3}, // a slender mallet 100
  {11707, 3}, // A blackened parchment 100
  {12023, 3}, // Dark Banded Mail 25
  {12206, 3}, // Ettin Leg Plates 50
  {12207, 3}, // Ettin Arm Plates 50
  {12209, 3}, // small black stone 50
  {12823, 3}, // A Tarion Badge 100
  {12929, 3}, // a black silk hood 20
  {12932, 3}, // a human skull on a chain 18
  {13025, 3}, // An Oak-Leaf Net 50
  {13101, 3}, // a living flame 15
  {13201, 3}, // a shard of frozen water 15
  {13301, 3}, // a heart of stone 15
  {13401, 3}, // a wispy cloud 15
  {13503, 3}, // crimson mask 30
  {13506, 3}, // Glowing armbands of Creaz 20
  {13507, 3}, // Ornate platinum belt 20
  {14502, 3}, // The Hell Stick 10
  {17023, 3}, // Dune Boots 40
  {17024, 3}, // A pointy wizard's hat 20
  {20147, 3}, // a Ninjato 100
  {21108, 3}, // A giant leaf shield 10
  {21205, 3}, // Boots of Stealth 10
  {21215, 3}, // Golden Dagger 29
  {21217, 3}, // the Mallet of the Underworld 10
  {24904, 3}, // Bracelet made from roots 25
  {24905, 3}, // Druid's Leafy Green Cloak 12
  {25016, 3}, // a Silver Trident 50
  {25026, 3}, // the Soul Amulet 10
  {25027, 3}, // The Silver Circlet of Concentration 50
  {27106, 3}, // Gladius 9
  {27108, 3}, // a  of the Magi 78
  {27110, 3}, // a hunting knife 12
  {27720, 3}, // Banded Mail of the Bandit 5
  {27721, 3}, // Wristband of the Assassin 10
  {204, 4}, // A gleaming spyglass 7
  {5109, 4}, // Drow Battle Gauntlets 50
  {5111, 4}, // a polished marble ring 13
  {5118, 4}, // a pair of black gloves 19
  {5175, 4}, // an eyepatch 19
  {5197, 4}, // Ball of Faerie Fire 10
  {5546, 4}, // Long Spiked Whip 10
  {12928, 4}, // a silver katana 8
  {13001, 4}, // an Elven Cloak 15
  {17002, 4}, // Vermilion's Blade 16
  {17309, 4}, // Fiend's Girdle 30
  {17321, 4}, // Cube of Awareness 50
  {17371, 4}, // Dragon Horn 15
  {20107, 4}, // Drums of Panic 35
  {20145, 4}, // War Fan 17
  {21203, 4}, // The Mask of Concentration 18
  {21210, 4}, // a Dragon Scale Belt 17
  {21214, 4}, // Thin
  {23006, 4}, // a Razor-Sharp Tooth from the Remorhaz 20
  {24906, 4}, // Evil Avenger 11
  {25015, 4}, // A Flaming Two-Handed Sword 50
  {25018, 4}, // A Crystal Ring 10
  {26406, 4}, // A Holey Majestic Cloak 12
  {26578, 4}, // A Tortoise Shell Shield 8
  {26579, 4}, // The Loincloth of Good Omens 8
  {26711, 4}, // The Greatsword of the Guard 8
  {27102, 4}, // A bone mace 12
  {11702, 5}, // A Silver Full Plate 11
  {11720, 5}  // a cracked dragon's tooth 13
};

#define STORAGE_ROOM            5807
#define CENTRAL_PROCESSING      5812
#define COLLECTOR               5812
#define TEMPLATE_AQORDER        40
#define AQ_ORDER_QUIT_TIME      120

char *kendernames[9] = {"Karl","Dieter","Hans","Jurgen","Hilda",
                          "Erwin","Herman","Eva","Marlene"};
                          
int generate_aq_order(CHAR *requester, CHAR *ordergiver, int lh_opt) {
  OBJ *aqorder;
  bool assigned[4] = {FALSE, FALSE, FALSE, FALSE};
  char buf[MAX_STRING_LENGTH];
  int pick, i, count = 0, questvalue = 0; 
  struct extra_descr_data *tmp_descr;
  
  if(requester->ver3.id <= 0)
    requester->ver3.id = generate_id();

  aqorder = read_object(TEMPLATE_AQORDER, VIRTUAL);

  // cleanup first
  DESTROY(aqorder->description);
  DESTROY(aqorder->short_description);
  aqorder->ownerid[0] = requester->ver3.id;   // tag the order with the requester's ID
  sprintf(buf, "An infamous acquisition order, forgotten here by %s.", GET_NAME(requester));
  aqorder->description = str_dup(buf);        // change the long desc to include requester's name
  sprintf(buf, "%s's acquisition order", GET_NAME(requester));
  aqorder->short_description = str_dup(buf);  // change the short desc to include requester's name
  aqorder->log = 1;
 
  for (i = 0; i < 4; i++) { 
    // get an obj from our aq_objs table, assign it to the aqorder value[i]
    while(count < 100) {
      count++;
      pick = number(0, NUMELEMS(aq_objs) - 1);
      
      if (aq_objs[pick][0] == aqorder->obj_flags.value[0]) continue;
      if (aq_objs[pick][0] == aqorder->obj_flags.value[1]) continue;
      if (aq_objs[pick][0] == aqorder->obj_flags.value[2]) continue;
      if (aq_objs[pick][0] == aqorder->obj_flags.value[3]) continue;
      if (lh_opt==1 && aq_objs[pick][1]>1) continue; // solo
      if (lh_opt==2 && aq_objs[pick][1]>2) continue; // low
      if (lh_opt==3 && aq_objs[pick][1]<3) continue; // high
      if ((lh_opt==4 && aq_objs[pick][1]<2) || (lh_opt==4 && aq_objs[pick][1]>3)) continue; // mid
      aqorder->obj_flags.value[i] = aq_objs[pick][0];
      questvalue += aq_objs[pick][1];
      assigned[i] = TRUE;
      break;
    }
  }
  if (assigned[0] && assigned[1] && assigned[2] && assigned[3]) {
    // tag order with questvalue - visible in magic.c "identify"
    OBJ_SPEC(aqorder) = questvalue;
    
    // change the extra desc to include required objects
    CREATE(tmp_descr, struct extra_descr_data, 1);
    tmp_descr->keyword = str_dup("order");
    sprintf(buf, "A well-worn, faded canvas bag has a large blue patch sewn\n\r\
to it. The patch has been sewn over half-a-hundred times and\n\r\
had those stitches ripped out just as many. There are four\n\r\
hastily sewn lines of text detailing the kenders' wish list.\n\r\
\n\r   %s\n\r   %s\n\r   %s\n\r   %s\n\r",
        aqorder->obj_flags.value[0] >= 0 ? real_object(aqorder->obj_flags.value[0]) >= 0 ? obj_proto_table[real_object(aqorder->obj_flags.value[0])].short_description : "something" : "nothing",
        aqorder->obj_flags.value[1] >= 0 ? real_object(aqorder->obj_flags.value[1]) >= 0 ? obj_proto_table[real_object(aqorder->obj_flags.value[1])].short_description : "something" : "nothing",
        aqorder->obj_flags.value[2] >= 0 ? real_object(aqorder->obj_flags.value[2]) >= 0 ? obj_proto_table[real_object(aqorder->obj_flags.value[2])].short_description : "something" : "nothing",
        aqorder->obj_flags.value[3] >= 0 ? real_object(aqorder->obj_flags.value[3]) >= 0 ? obj_proto_table[real_object(aqorder->obj_flags.value[3])].short_description : "something" : "nothing");
    tmp_descr->description = str_dup(buf);
    aqorder->ex_description = tmp_descr;
    tmp_descr = NULL;

    sprintf(buf, "Good luck %s, try to do a better job than %s, it'll be worth %d points if you can.",
        GET_NAME(requester), kendernames[number(0, NUMELEMS(kendernames)-1 )], questvalue);
    do_say(ordergiver, buf, CMD_SAY);
    obj_to_char(aqorder, requester);
    sprintf(buf, "%s gives %s to %s.", GET_SHORT(ordergiver),
        OBJ_SHORT(aqorder), GET_NAME(requester));
    act(buf,0,ordergiver,0,requester,TO_NOTVICT);
    sprintf(buf, "%s gives %s to you.", GET_SHORT(ordergiver),
        OBJ_SHORT(aqorder));
    act(buf,0,ordergiver,0,requester,TO_VICT);
    return TRUE;
  } else {
    wizlog("WIZINFO: AQ Order object assignment counter exceeded 100.",LEVEL_IMP,5);
    log_f("WIZINFO: AQ Order object assignment counter exceeded 100.");
    return FALSE;
  }
  return FALSE;
} // end of generate_aq_order()

int aq_order_obj (OBJ *order, CHAR *ch, int cmd, char *arg) {
  char argument[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct extra_descr_data *tmp_descr;
  CHAR *collector = NULL;
  char *collectorinsult[8] = {"wimp","quitter","lame-o","goldbricker",
                              "pansy","slacker","chump","loser"};
  // block methods of getting order out of game and back again on non-owner
  //   since this would allow multiple orders for a character at once
  // "junk" only real method to quit an order
  // Dump/Scrapyard workaround is in int dump()

  if (ch != order->carried_by) return FALSE;
  
  switch(cmd) {
  case MSG_OBJ_ENTERING_GAME:
    if (V_OBJ(order) != TEMPLATE_AQORDER) return FALSE;
    // redo the short/long desc strings based on Owner
    if (order->ownerid[0] > 0) {
      // cleanup first
      DESTROY(order->description);
      DESTROY(order->short_description);
      sprintf(buf, "An infamous acquisition order, forgotten here by %s.", CAP(idname[order->ownerid[0]].name));
      order->description = str_dup(buf);        // change the long desc to include requester's name
      sprintf(buf, "%s's acquisition order", CAP(idname[order->ownerid[0]].name));
      order->short_description = str_dup(buf);  // change the short desc to include requester's name

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
    }
    break;
  case MSG_CORPSE:
    // pop orders out when a corpse is made with a message
    if (ch && (V_OBJ(order) == TEMPLATE_AQORDER)) {
      obj_from_char(order);
      obj_to_room(order, CHAR_REAL_ROOM(ch));
      sprintf(buf, "Just as %s expires, %s drops %s.\n\r",
          IS_NPC(ch) ? GET_SHORT(ch) : GET_NAME(ch),
          HSSH(ch), OBJ_SHORT(order));
      send_to_room(buf, CHAR_REAL_ROOM(ch));
    }
    break;
  case MSG_AUTORENT:
    if (V_OBJ(order) != TEMPLATE_AQORDER) return FALSE;
    if ((order->ownerid[0] != ch->ver3.id) && (order->ownerid[0] > 0)) {
      obj_from_char(order);
      obj_to_room(order, real_room(STORAGE_ROOM));

      // want to have COLLECTOR send a quest message when he intercepts an
      //   autorent like this, so we find him or load him
      if (mob_proto_table[real_mobile(COLLECTOR)].number < 1) {
        collector = read_mobile(COLLECTOR, VIRTUAL);
        char_to_room(collector, real_room(number(3000,3072)));
      } else {
        collector = get_ch_world(COLLECTOR);
        if (!collector) return FALSE; // shouldn't be possible, but for safety
      }
      sprintf(buf, "Oops, %s nearly auto-rented with %s, but I moved it to our main storage \
facility for safekeeping.", GET_NAME(ch), OBJ_SHORT(order));
      do_quest(collector, buf, CMD_QUEST);
    }
    break;
  case CMD_RENT:
    if (V_OBJ(order) != TEMPLATE_AQORDER) return FALSE;
    if(!IS_NPC(ch) && GET_LEVEL(ch) >= LEVEL_IMM) return FALSE;
    if ((order->ownerid[0] != ch->ver3.id) && (order->ownerid[0] > 0)) {
      sprintf(buf, "Something prevents you from renting with %s.\n\r",
          OBJ_SHORT(order));
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

    sprintf(buf, "Something prevents you from storing %s.\n\r",
        OBJ_SHORT(order));
    send_to_char(buf, ch);
    return TRUE;
    break;
  case CMD_JUNK:
    if(!IS_NPC(ch) && GET_LEVEL(ch) >= LEVEL_IMM) return FALSE;
    arg = one_argument(arg, argument);
    if (!*argument) return FALSE; // no argument after "junk"
    if (!(order = get_obj_in_list_vis(ch, argument, ch->carrying))) return FALSE;
    if (V_OBJ(order) != TEMPLATE_AQORDER) return FALSE;

    // want to have COLLECTOR send a quest message when he intercepts an
    //   junk like this, so we find him or load him
    if (mob_proto_table[real_mobile(COLLECTOR)].number < 1) {
      collector = read_mobile(COLLECTOR, VIRTUAL);
    } else {
      collector = get_ch_world(COLLECTOR);
      if (!collector) return FALSE; // shouldn't be possible, but for safety
      sprintf(buf, "%s disappears in a beam of bright light.\n\r", GET_SHORT(collector));
      send_to_room(buf, CHAR_REAL_ROOM(collector));
      char_from_room(collector);
    }
    char_to_room(collector, CHAR_REAL_ROOM(ch));
    sprintf(buf, "%s appears in a corona of bright light and \
points a strange pistol at %s, which disappears.\n\r", GET_SHORT(collector), OBJ_SHORT(order));
    send_to_room(buf, CHAR_REAL_ROOM(collector));

    if (order->ownerid[0] > 0 && // someone other than owner is junking
        strcmp(idname[ch->ver3.id].name, idname[order->ownerid[0]].name)) {
      sprintf(buf1, "Tsk tsk %s, doing %s's dirty work. ",
          IS_NPC(ch) ? GET_SHORT(ch) : GET_NAME(ch),
          CAP(idname[order->ownerid[0]].name));
      sprintf(buf2, "If %s can't handle the order I'll just send it back to Central \
Processing to be requeued, that %s.", 
          CAP(idname[order->ownerid[0]].name),
          collectorinsult[number(0, NUMELEMS(collectorinsult)-1 )]);
      sprintf(buf, "%s %s", buf1, buf2);
      // for some reason referencing CAP(idname[order->ownerid[0]].name)
      //   twice in the same sprintf() creates a warning, so splitting it
      //   up since warnings are annoying
    } else {
      sprintf(buf, "I saw that %s, if you don't think you can handle the order I'll just \
send it back to Central Processing to be requeued... you %s.",
          IS_NPC(ch) ? GET_SHORT(ch) : GET_NAME(ch),
          collectorinsult[number(0, NUMELEMS(collectorinsult)-1 )]);
    }
    do_quest(collector, buf, CMD_QUEST);

    // this is where it gets moved to another room to "hack" a wait timer
    //   --- boots and crashes will cancel this, so be it
    obj_from_char(order);
    obj_to_room(order, real_room(CENTRAL_PROCESSING));
    order->obj_flags.timer = AQ_ORDER_QUIT_TIME; // setup "hack" wait timer
    SET_BIT(order->obj_flags.extra_flags2, ITEM_ALL_DECAY);
    return TRUE;
    break;
  default:
    break;
  }
  return FALSE;
} // end of aq_order_obj()

int aq_order_mob (CHAR *collector, CHAR *ch, int cmd, char *arg) {
  OBJ *order = NULL;
  OBJ *obj = NULL, *next_obj  = NULL;
  char buf[MAX_STRING_LENGTH];
  char argument[MAX_INPUT_LENGTH];
  int i, j, k, tmp_value, lh_opt = 0, questvalue = 0;
  int requirements[4] = {-1, -1, -1, -1};
  bool value_exists = FALSE;
  bool found[4] = {FALSE, FALSE, FALSE, FALSE};
  char *collectoraction[8] = {"groan","frustration","cod","fume",
                              "blorf","roll","sneor","mumble"};
  char *kenderinsults[10] = {"fool","moron","idiot","bonehead",
                            "nitwit","nincompoop","imbecile","dullard",
                            "cotton-headed ninnymuggins","peabrain"};
  
  if (cmd == CMD_AQUEST) {
    // process order request
    arg = one_argument(arg, argument);
    if (*argument) {
      if (is_abbrev(argument,"order")) {
        // check if already an order in game for that player
        for (obj = object_list; obj; obj = obj->next) {
          if ((TEMPLATE_AQORDER == V_OBJ(obj)) && (obj->ownerid[0] == ch->ver3.id)) {
            sprintf(buf, "You've already got an order %s, I doubt you could handle more.",
                GET_NAME(ch));
            do_say(collector, buf, CMD_SAY);
            return TRUE;
          }
        }
        
        arg = one_argument(arg, argument);
        if (*argument) {
          if (is_abbrev(argument,"solo")) lh_opt=1;
          if (is_abbrev(argument,"low"))  lh_opt=2;
          if (is_abbrev(argument,"high")) lh_opt=3;
          if (is_abbrev(argument,"mid"))  lh_opt=4;
        }
        if (!generate_aq_order(ch, collector, lh_opt)) {
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
      mob_do(collector, "roll");
      break;
    default:
      break;
    }
    
    if(collector->specials.fighting) {
      do_say(collector, "What the heck are you doing? Skotty get me outta' here!", CMD_SAY);
      sprintf(buf, "%s disappears in a beam of bright light.\n\r", GET_SHORT(collector));
      send_to_room(buf, CHAR_REAL_ROOM(collector));
      GET_HIT(collector) = GET_MAX_HIT(collector);
      char_from_room(collector);
      char_to_room(collector, real_room(number(3000,3072)));
    }
  }
  
  if (cmd == MSG_GAVE_OBJ) {
    // process order delivery
    order = get_obj_in_list_vis(collector, "order", collector->carrying);

    if (order && GET_ITEM_TYPE(order) == ITEM_AQ_ORDER) {
      
      if ((order->ownerid[0] != ch->ver3.id) &&
          (order->ownerid[0] > 0) &&
          (strcmp(idname[order->ownerid[0]].name, idname[ch->ver3.id].name))) {
        // handed in by non-owner
        sprintf(buf, "Where'd you get this? Did you kill 'em? Give it back to %s.",
            CAP(idname[order->ownerid[0]].name));
        do_say(collector, buf, CMD_SAY);
        obj_from_char(order);
        obj_to_char(order, ch);
        sprintf(buf, "%s hands %s back to %s, eyeing %s warily.",
            GET_SHORT(collector), OBJ_SHORT(order), GET_NAME(ch), HMHR(ch));
        act(buf,0,collector,0,ch,TO_NOTVICT);
        sprintf(buf, "%s hands %s back to you warily.", GET_SHORT(collector),
            OBJ_SHORT(order));
        act(buf,0,collector,0,ch,TO_VICT);
        return TRUE;
      }
      
      for (i = 0; i < 4; i++) {
        requirements[i] = order->obj_flags.value[i];
        if(requirements[i] < 0) { // null (-1) requirement object
          found[i] = TRUE;
        } else {
          for (obj = order->contains; obj; obj = next_obj) {
            next_obj = obj->next_content;
            
            if (V_OBJ(obj) == requirements[i]) {
              // required object found, but is it new enough?
              if (obj->obj_flags.popped < order->obj_flags.popped) {
                // object was popped before order received : see aq_obj_date_popped()
                do_say(collector, "No, no, no - this won't do at all.", CMD_SAY);
                sprintf(buf, "%s was popped too long ago, I need a new one!", CAP(OBJ_SHORT(obj)));
                do_say(collector, buf, CMD_SAY);
                obj_from_char(order);
                obj_to_char(order, ch);
                sprintf(buf, "%s gives %s back to %s disappointedly.", GET_SHORT(collector),
                    OBJ_SHORT(order), GET_NAME(ch));
                act(buf,0,collector,0,ch,TO_NOTVICT);
                sprintf(buf, "%s gives %s back to you disappointedly.", GET_SHORT(collector),
                    OBJ_SHORT(order));
                act(buf,0,collector,0,ch,TO_VICT);                
                return TRUE;
              } else {
                found[i] = TRUE;
                value_exists = FALSE;
                // OBJ_SPEC(order) is set initially by aq_generate_order() to be
                //   the order's AQP value, for magic.c identify output;
                //   but we'll count here anyway with aq_objs[][] current values
                //   also gives us a chance to award a double point bonus
                for (k = 0; k < NUMELEMS(aq_objs); k++) {
                  if (aq_objs[k][0] == V_OBJ(obj)) {  
                    value_exists = TRUE;
                    tmp_value = aq_objs[k][1];
                    if (chance(2)) {
                      tmp_value *= 2;
                    }                    
                    questvalue += tmp_value;
                  }
                }
                if(!value_exists) {
                  // order included an object VNUM not in aq_objs[]
                  sprintf(buf, "WIZINFO: AQ Order included object #: %d which \
is not in aq_objs point value table.",
                      V_OBJ(obj));
                  wizlog(buf,LEVEL_SUP,5);
                  log_f("%s",buf);
                }
              }
            }
          }
        }
      }

      if(found[0] && found[1] && found[2] && found[3]) { 
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
          sprintf(buf, "The market is always influx my %s. This is worth %d points to me now.",
              GET_SEX(ch) == SEX_MALE ? "boy" : "dear", questvalue);
          do_say(collector, buf, CMD_SAY);
        } else {
          sprintf(buf, "Well done! Your %d points are well deserved %s.",
              questvalue, GET_SEX(ch) == SEX_MALE ? "sir" : "madam");
          do_say(collector, buf, CMD_SAY);
        }
        sprintf(buf, "%s casually points a strange pistol at the order\n\r\
and it disappears before your very eyes!\n\r", GET_SHORT(collector));
        send_to_room(buf, CHAR_REAL_ROOM(collector));
        
        sprintf(buf, "%s completed an order for me, what a %s!", GET_NAME(ch),
            GET_SEX(ch) == SEX_MALE ? "guy" : "gal");
        extract_obj(order);
        do_quest(collector, buf, CMD_QUEST);
        ch->ver3.quest_points += questvalue;
        return FALSE;      
      } else { // missing at least one object
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
        obj_from_char(order);
        obj_to_char(order, ch);
        sprintf(buf, "%s tosses %s back to %s tiredly.", GET_SHORT(collector),
            OBJ_SHORT(order),GET_NAME(ch));
        act(buf,0,collector,0,ch,TO_NOTVICT);
        sprintf(buf, "%s tosses %s back to you tiredly.", GET_SHORT(collector),
            OBJ_SHORT(order));
        act(buf,0,collector,0,ch,TO_VICT);        
        return TRUE;
      }
    } else { // given non-AQ_ORDER
      do_say(collector, "Thanks, I guess?", CMD_SAY);
    }
  } // end "deliver order"
  return FALSE;
} // end of aq_order_mob()

void assign_aquest_special(void) {
  assign_obj( TEMPLATE_AQORDER, aq_order_obj );
  assign_mob( COLLECTOR, aq_order_mob );
}
