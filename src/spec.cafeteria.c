/*
///   Special Procedure Module                   Orig. Date 08-27-1996
///                                              Last Modif 09-17-1996
///
///   Spec.cafeteria.c --- Specials for Fidos and Cafe, by Jarldian
///   cafe_bouncer:
///   Designed and coded by Lem of RoninMUD (im478@cleveland.freenet.edu
///
///   cafe_waiter, cafe_waitress, cafe_attendant:
///   Designed and coded (under guidance of Ranger) by Thyas of RoninMUD.
///   Using this code is not allowed without permission from originator.
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
#include "spec_assign.h"

#define ZONE_CAFETERIA     3600
#define CAFE_BOUNCER       ITEM(ZONE_CAFETERIA,10)
#define CAFE_ATTENDANT     ITEM(ZONE_CAFETERIA,12)
#define CAFE_WAITER        ITEM(ZONE_CAFETERIA,13)
#define CAFE_WAITRESS      ITEM(ZONE_CAFETERIA,14)

#define NEW_ROOM           3600

int cafe_bouncer (CHAR *mob, CHAR *ch, int cmd, char *arg)  {

  char buf[MAX_STRING_LENGTH];

  if(CHAR_REAL_ROOM(mob)!=real_room(3610)) return(FALSE);

  if ((cmd != CMD_GIVE) && (cmd != CMD_BUY) && (cmd != CMD_UP)) return(FALSE);

  arg = one_argument(arg, buf);

  if ((cmd == CMD_BUY) || (cmd == CMD_GIVE) )
    {
       if (!*buf)
         {
           send_to_char("What?\n\r",ch);
           return(TRUE);
         }

       if (!strcmp(buf,"bribe"))
         {
         if (GET_POS(ch) == POSITION_RIDING)
          {
           send_to_char("The Bouncer says 'Hey buddy, no freakin' animals allowed, dismount now!'\n\r",ch);
           return(TRUE);
          }
           if (GET_GOLD(ch) < 15000) {
           send_to_char("You don't have enough coins!", ch);
           return(TRUE);
           }

           GET_GOLD(ch) -= 15000;
	   send_to_char("You slip the Bouncer some cash.\n\r", ch);
           act("$n slips the Bouncer some cash.", FALSE, ch, 0, 0, TO_ROOM);
	   send_to_char("The Bouncer says 'Don't tell nobody or I'll bust yer arms.'\n\r", ch);
	   send_to_char("The Bouncer shoves you quickly up the staircase.\n\r", ch);
	   send_to_char(" \n\r",ch);
           act("The Bouncer escorts $n inside.\n\r", FALSE, ch, 0, 0, TO_ROOM);
	   char_from_room(ch);
	   char_to_room(ch,real_room(NEW_ROOM));
           do_look(ch,"",CMD_LOOK);
           act("$n has arrived.", FALSE, ch, 0, 0, TO_ROOM);
           return(TRUE);
         }
     return(FALSE);
     }

  if (cmd == CMD_UP)
    {
      send_to_char("The Bouncer says 'YO!! Nobody gets in here without my say-so.'\n\rFor that you need to gimme a little somethin'\n\r", ch);
      act("The Bouncer plants himself in front of $n, blocking the way.\n\r", FALSE, ch, 0, 0, TO_ROOM);
    return(TRUE);
    }
  return(FALSE);
}
int cafe_waiter(CHAR *mob, CHAR *ch, int cmd, char *arg){

   CHAR *wait1=0, *wait2=0, *tch=0;
   int found=FALSE;
   wait1=mob;

   if(cmd) return FALSE;
   if (GET_POS(wait1)<=POSITION_FIGHTING) return FALSE;
   if(number(0,4)) return FALSE;
   for (wait2 = world[CHAR_REAL_ROOM(wait1)].people; wait2; wait2 = wait2->next_in_room) {
     if (IS_NPC(wait2) && GET_POS(wait2)>POSITION_FIGHTING && (V_MOB(wait2)==CAFE_WAITRESS || V_MOB(wait2)==CAFE_ATTENDANT)){
      found=TRUE;
      break;
     }
   }
   if(!found) return FALSE;
   if(number(0,1)){
       tch=wait1;
       wait1=wait2;
       wait2=tch;
   }
   if((!wait1)||(!wait2)) return FALSE;
   switch (number (0,6)) {
       case 0:
	 act ("$n screams at $N 'You idiot,\n\rthat was my tip!'",
	      FALSE, wait1, 0, wait2, TO_NOTVICT) ;
	 act ("You scream at $N 'You idiot, that was my tip!'",
	      FALSE, wait1, 0, wait2, TO_CHAR) ;
	 act ("$n screams at you 'You idiot, that was my tip!'",
	      FALSE, wait1, 0, wait2, TO_VICT) ;
	 break ;
       case 1:
	 act ("$n kicks $N in the shins.\n\r$n laughs hysterically at $N's pain.",
	      FALSE, wait1, 0, wait2, TO_NOTVICT) ;
	 act ("You kick $N in shins.\n\rYou laugh at $N's pain.",
	      FALSE, wait1, 0, wait2, TO_CHAR) ;
	 act ("$n kicks you in the shins.\n\r$n laughs at your pain.",
	      FALSE,wait1, 0, wait2, TO_VICT) ;
	 break ;
       case 2:
	 act ("$n cleans a table with a ragged cloth.",
              FALSE, wait1, 0, 0, TO_ROOM) ;
	 act ("You clean the table with your cloth.",
	      FALSE, 0, 0, 0, TO_CHAR) ;
	 break ;
       case 3:
	 act ("$n yells at $N,\n\ras $E fumbles and drops $S tray.",
	      FALSE, wait1, 0, wait2, TO_NOTVICT) ;
	 act ("$n quickly picks up $s tray.", FALSE, wait2, 0, 0, TO_ROOM) ;
	 act ("$N yells at you, as you fumble, losing your tray.",
	      FALSE, wait1, 0, wait2, TO_VICT) ;
	 act ("You yell at $n, as $s fumbles, losing his weapon.",
	      FALSE, wait1, 0, wait2, TO_CHAR) ;
	 break ;
       case 4:
	 act ("$N glares angrily at $n, no love lost there.",
	      FALSE, wait1, 0, wait2, TO_NOTVICT) ;
	 act ("$N glares angrily at you, you feel the lack of love. ",
	      FALSE, wait1, 0, wait2, TO_VICT) ;
	 act ("You glare angrily at $n, you feel no love for $M.",
	      FALSE, wait1, 0, wait2, TO_CHAR) ;
	 break ;
       case 5:
	 act ("$n lunges at $N but $E evades $s\n\routstretched hands.",
	      FALSE, wait1, 0, wait2, TO_NOTVICT) ;
	 act ("You lunge at $N but $E evades your outstreched hands.",
	      FALSE, wait1, 0, wait2, TO_CHAR) ;
	 act ("$n lunges a blow at you, but you evade $s outstretched hands.",
	      FALSE, wait1, 0, wait2, TO_VICT) ;
	 break ;
       case 6:
	 act ("You pick up some old napkins and rearrange a chair.",
	      FALSE, wait1, 0, 0, TO_CHAR) ;
	 act ("$n picks up some old napkins and rearranges a chair.",
	      FALSE, wait1, 0, 0, TO_NOTVICT) ;
	 break ;
     }
   return FALSE ;
 }

int cafe_attendant(CHAR *mob, CHAR *ch, int cmd, char *arg){
  char buf[MAX_STRING_LENGTH];
  OBJ *tmp;
  if(GET_POS(mob)<=POSITION_FIGHTING) return(FALSE);
  if(CHAR_REAL_ROOM(mob)!=real_room(3612)) return(FALSE);

   if (cmd==CMD_LIST) { /* List */
    send_to_char("The attendant says 'Look at the menu, I don't feel like listing them all again.\n\r", ch);
    return(TRUE);
  }

  if (cmd==CMD_BUY) { /* Buy */

    arg = one_argument(arg, buf);

    if (!*buf)
      {
	send_to_char("Buy what?\n\r",ch);
	return(TRUE);
      }
    if (!strcmp(buf,"muffin"))
      {
	if (GET_GOLD(ch)<24)
	  {
	    send_to_char("You don't have enough money.\n\r",ch);
	    return(TRUE);
	  }
	else
	  {
	    tmp=read_object(3620,VIRTUAL);
            obj_to_char(tmp,ch);
	    GET_GOLD(ch)-=24;
	    send_to_char("That will cost you 24 coins.\n\r",ch);
            act("$n gives $p to $N.",1,mob,tmp,ch,TO_NOTVICT);
	     return(TRUE);
	  }
      }
    else
    if (!strcmp(buf,"bread"))
      {
	if (GET_GOLD(ch)<72)
	  {
	    send_to_char("You don't have enough money.\n\r",ch);
	    return(TRUE);
	  }
	else
	  {
	    tmp=read_object(3616,VIRTUAL);
            obj_to_char(tmp,ch);
	    GET_GOLD(ch)-=72;
	    send_to_char("That's 72 coins.  Enjoy.\n\r",ch);
            act("$n gives $p to $N.",1,mob,tmp,ch,TO_NOTVICT);
	     return(TRUE);
	  }
      }
    else
    if (!strcmp(buf,"rice"))
      {
	if (GET_GOLD(ch)<96)
	  {
	    send_to_char("You don't have enough money.\n\r",ch);
	    return(TRUE);
	  }
	else
	  {
	    tmp=read_object(3615,VIRTUAL);
            obj_to_char(tmp,ch);
	    GET_GOLD(ch)-=96;
	    send_to_char("You spend 96 coins on a rice cake?\n\r",ch);
	    act("$n gives $p to $N.",1,mob,tmp,ch,TO_NOTVICT);
	     return(TRUE);
	  }
      }
    else
    if (!strcmp(buf,"pasta"))
      {
	if (GET_GOLD(ch)<120)
	  {
	    send_to_char("You don't have enough money.\n\r",ch);
	    return(TRUE);
	  }
	else
	  {
            tmp=read_object(3614,VIRTUAL);
	    obj_to_char(tmp,ch);
	    GET_GOLD(ch)-=120;
	    send_to_char("That's 120 coins please.\n\r",ch);
	    act("$n gives $p to $N.",1,mob,tmp,ch,TO_NOTVICT);
	     return(TRUE);
	  }
      }
    else
    if (!strcmp(buf,"potato"))
      {
	if (GET_GOLD(ch)<48)
	  {
	    send_to_char("You don't have enough money.\n\r",ch);
	    return(TRUE);
	  }
	else
	  {
	    tmp=read_object(3611,VIRTUAL);
            obj_to_char(tmp,ch);
	    GET_GOLD(ch)-=48;
	    send_to_char("That will cost you 48 coins.\n\r",ch);
             act("$n gives $p to $N.",1,mob,tmp,ch,TO_NOTVICT);
	     return(TRUE);
	  }
      }
    else
    if (!strcmp(buf,"vodka"))
      {
	if (GET_GOLD(ch)<240000)
	  {
	    send_to_char("You don't have enough money.\n\r",ch);
	    return(TRUE);
	  }
	else
	  {
	    tmp=read_object(3609,VIRTUAL);
            obj_to_char(tmp,ch);
	    GET_GOLD(ch)-=240000;
	    send_to_char("That's 240,000 coins.  Enjoy.\n\r",ch);
	    act("$n gives $p to $N.",1,mob,tmp,ch,TO_NOTVICT);
	    return(TRUE);
	  }
      }
    else
    if (!strcmp(buf,"coffee"))
      {
	if (GET_GOLD(ch)<150)
	  {
	    send_to_char("You don't have enough money.\n\r",ch);
	    return(TRUE);
	  }
	else
	  {
	    tmp=read_object(3608,VIRTUAL);
            obj_to_char(tmp,ch);
	    GET_GOLD(ch)-=150;
	    send_to_char("You spend 150 coins on a coffee?\n\r",ch);
             act("$n gives $p to $N.",1,mob,tmp,ch,TO_NOTVICT);
	     return(TRUE);
	  }
      }
    else
    if (!strcmp(buf,"beer"))
      {
	if (GET_GOLD(ch)<108000)
	  {
	    send_to_char("You don't have enough money.\n\r",ch);
	    return(TRUE);
	  }
	else
	  {
            tmp=read_object(3607,VIRTUAL);
	    obj_to_char(tmp,ch);
	    GET_GOLD(ch)-=108000;
	    send_to_char("That's 108,000 coins please.\n\r",ch);
	    act("$n gives $p to $N.",1,mob,tmp,ch,TO_NOTVICT);
	     return(TRUE);
	  }
      }
    else
    if (!strcmp(buf,"spritzer"))
      {
	if (GET_GOLD(ch)<3400)
	  {
	    send_to_char("You don't have enough money.\n\r",ch);
	    return(TRUE);
	  }
	else
	  {
	    tmp=read_object(3606,VIRTUAL);
            obj_to_char(tmp,ch);
	    GET_GOLD(ch)-=3400;
	    send_to_char("You spend 3400 coins on a spritzer?\n\r",ch);
	    act("$n gives $p to $N.",1,mob,tmp,ch,TO_NOTVICT);
	    return(TRUE);
	  }
      }
    else
    if (!strcmp(buf,"water"))
      {
	if (GET_GOLD(ch)<300)
	  {
	    send_to_char("You don't have enough money.\n\r",ch);
	    return(TRUE);
	  }
	else
	  {
            tmp=read_object(3605,VIRTUAL);
	    obj_to_char(tmp,ch);
	    GET_GOLD(ch)-=300;
	    send_to_char("That's 300 coins please.  Enjoy\n\r",ch);
	    act("$n gives $p to $N.",1,mob,tmp,ch,TO_NOTVICT);
	     return(TRUE);
	  }
      }
    else
      {
	send_to_char("You can't buy that.\n\r",ch);
	return(TRUE);
      }
   }
   return(FALSE);
}


int cafe_waitress(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd)
    return FALSE;

  if (GET_POS(mob)>POSITION_FIGHTING) {
    if(!number(0,10)) {
      act("The waitress casually wipes the table with her ragged cloth.",FALSE,mob,0,0,TO_ROOM);
      act("$n smiles happily",FALSE,mob,0,0,TO_ROOM);
    }
  }
  return FALSE;
}

void assign_cafeteria(void) {
  assign_mob(CAFE_BOUNCER,cafe_bouncer);
  assign_mob(CAFE_ATTENDANT,cafe_attendant);
  assign_mob(CAFE_WAITER,cafe_waiter);
  assign_mob(CAFE_WAITRESS,cafe_waitress);
}

