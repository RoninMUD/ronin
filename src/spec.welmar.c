 /* ************************************************************************
  *  file: spec_procs.c , Special module.                   Part of DIKUMUD *
  *  Usage: Procedures handling special procedures for object/room/mobile   *
  *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
  ************************************************************************* */

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
#include "fight.h"
#include "act.h"
#include "reception.h"
#include "spec_assign.h"

 /**********************************************************************\
 |* Special procedures for Kings Castle by Pjotr (d90-pem@nada.kth.se) *|
 |* Coded by Sapowox (d90-jkr@nada.kth.se)                             *|
 \**********************************************************************/

 #ifndef Z_KINGS_C
 #define Z_KINGS_C 89
 #endif

 #define R_MOB(zone, mob) (real_mobile(((zone)*100)+(mob)))
 #define R_OBJ(zone, obj) (real_object(((zone)*100)+(obj)))
 #define R_ROOM(zone, num) (real_room(((zone)*100)+(num)))

 #define CASTLE_ITEM(item) (Z_KINGS_C*100+(item))

 /* Routine member_of_staff */
 /* Used to see if a character is a member of the castle staff */
 /* Used mainly by BANZAI:ng NPC:s */
 int member_of_staff (CHAR *chChar) {
   int ch_num ;

   if (!IS_NPC (chChar))
     return (FALSE) ;

   ch_num = mob_proto_table [chChar->nr].virtual ;
   return (ch_num == CASTLE_ITEM(1) ||
	   (ch_num > CASTLE_ITEM (2) && ch_num < CASTLE_ITEM(15)) ||
	   (ch_num > CASTLE_ITEM(15) && ch_num < CASTLE_ITEM(18)) ||
	   (ch_num > CASTLE_ITEM(18) && ch_num < CASTLE_ITEM(30))) ;
 }


 /* Function member_of_royal_guard */
 /* Returns TRUE if the character is a guard on duty, otherwise FALSE */
 /* Used by Peter the captain of the royal guard */
 int member_of_royal_guard (CHAR *chChar) {
   int ch_num ;

   if (!chChar || !IS_NPC (chChar))
     return FALSE ;

   ch_num = mob_proto_table [chChar->nr].virtual ;
   return (ch_num == CASTLE_ITEM(3) ||
	   ch_num == CASTLE_ITEM(6) ||
	   (ch_num > CASTLE_ITEM (7) && ch_num < CASTLE_ITEM(12)) ||
	   (ch_num > CASTLE_ITEM(23) && ch_num < CASTLE_ITEM(26))) ;
 }

 /* Function find_npc_by_name */
 /* Returns a pointer to an npc by the given name */
 /* Used by Tim and Tom */
 CHAR *find_npc_by_name (CHAR *chAtChar, char *pszName,
				     int iLen) {
   CHAR *ch ;

   for (ch = world[CHAR_REAL_ROOM(chAtChar)].people ; ch ; ch = ch->next_in_room)
     if (IS_NPC(ch))
       if (!strncmp (pszName, MOB_SHORT(ch), iLen))
	 return (ch) ;

   return NULL ;
 }

 /* Function find_guard */
 /* Returns the pointer to a guard on duty. */
 /* Used by Peter the Captain of the Royal Guard */
 CHAR *find_guard (CHAR *chAtChar) {

   CHAR *ch ;

   for (ch = world[CHAR_REAL_ROOM(chAtChar)].people ; ch ; ch = ch->next_in_room)
     if (!ch->specials.fighting && member_of_royal_guard (ch))
       return ch ;

   return NULL ;
 }


 /* Function get_victim */
 /* Returns a pointer to a randomly chosen character in the same room, */
 /* fighting someone in the castle staff... */
 /* Used by BANZAII-ing characters and King Welmar... */
 CHAR *get_victim (CHAR *chAtChar) {

   CHAR *ch ;
   int iNum_bad_guys = 0, iVictim ;

   for (ch = world[CHAR_REAL_ROOM(chAtChar)].people ; ch ; ch = ch->next_in_room)
     if (ch->specials.fighting &&
	 member_of_staff (ch->specials.fighting))
       iNum_bad_guys++ ;

   if (!iNum_bad_guys)
     return NULL ;

   iVictim = number (0,iNum_bad_guys) ; /* How nice, we give them a chance */
   if (!iVictim)
     return NULL ;

   iNum_bad_guys = 0 ;

   for (ch = world[CHAR_REAL_ROOM(chAtChar)].people ; ch ; ch = ch->next_in_room)
     if (ch->specials.fighting &&
	 member_of_staff (ch->specials.fighting) &&
	 ++iNum_bad_guys == iVictim)
       return ch ;
   return NULL;
 }


 /* Function banzaii */
 /* Makes a character banzaii on attackers of the castle staff */
 /* Used by Guards, Tim, Tom, Dick, David, Peter, Master, King and Guards */
 int banzaii (CHAR *ch) {

   CHAR *chOpponent = NULL ;

   if (!AWAKE (ch) || GET_POS (ch) == POSITION_FIGHTING)
     return FALSE ;

   if ((chOpponent = get_victim (ch))) {
     act ("$n roars: 'Protect the Kingdom of Great King Welmar! BANZAIIII!!!'",
	  FALSE, ch, 0, 0, TO_ROOM) ;
     hit (ch, chOpponent, TYPE_UNDEFINED) ;
     return FALSE ;
   }

   return FALSE ;
 }


 /* Function do_npc_rescue */
 /* Makes ch_hero rescue ch_victim */
 /* Used by Tim and Tom */
 int do_npc_rescue (CHAR *ch_hero, CHAR *ch_victim) {

   CHAR *ch_bad_guy ;

   for (ch_bad_guy=world[CHAR_REAL_ROOM(ch_hero)].people;
	ch_bad_guy && (ch_bad_guy->specials.fighting != ch_victim);
	ch_bad_guy=ch_bad_guy->next_in_room)  ;
   if (ch_bad_guy) {
     if (ch_bad_guy == ch_hero)
       return FALSE ; /* NO WAY I'll rescue the one I'm fighting! */
     act ("You bravely rescue $N.\n\r", FALSE, ch_hero, 0, ch_victim, TO_CHAR) ;
     act("You are rescued by $N, your loyal friend!\n\r",
	 FALSE, ch_victim, 0, ch_hero, TO_CHAR);
     act("$n heroically rescues $N.", FALSE, ch_hero, 0, ch_victim, TO_NOTVICT);

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


 /* Procedure to block a person trying to enter a room. */
 /* Used by Tim/Tom at Kings bedroom and Dick/David at treasury */
 int block_way(CHAR *ch, int cmd, char *arg, int iIn_room,
	       int iProhibited_direction) {

   if (cmd != ++iProhibited_direction || (
	  !strncmp (MOB_SHORT(ch), "King Welmar", 11)))
     return FALSE ;

   if ((CHAR_REAL_ROOM(ch) == real_room(iIn_room)) && (cmd == iProhibited_direction))
     {
       if (!member_of_staff (ch))
       {
	 act ("The guard roars at $n and pushes $m back.", FALSE, ch, 0, 0, TO_ROOM);
         send_to_char ("The guard roars: 'Entrance is Prohibited!', and pushes you back.\n\r", ch) ;
         return (TRUE) ;
       }
     }

   return FALSE ;
 }


 /* Routine to check if an object is trash... */
 /* Used by James the Butler and the Cleaning Lady */
 int is_trash (OBJ *i) {

   if(!IS_SET(i->obj_flags.wear_flags, ITEM_TAKE)) return FALSE;
   if(i->obj_flags.cost <= 10) return TRUE;
   return FALSE;
 }


 /* Function fry_victim */
 /* Finds a suitabe victim, and cast some _NASTY_ spell on him */
 /* Used by King Welmar */
 void fry_victim (CHAR *ch) {

   CHAR *tch ;


   if (ch->points.mana < 10)
     return ;

   /* Find someone suitable to fry ! */

   if (!(tch = get_victim (ch)))
     return ;

   switch (number (0,8)) {
   case 1:
   case 2:
   case 3:
     act("You raise your hand in a dramatic gesture.", 1, ch, 0, 0, TO_CHAR);
     act("$n raises $s hand in a dramatic gesture.", 1, ch, 0, 0, TO_ROOM);
     cast_color_spray (GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, tch, 0) ;
     break;
   case 4:
   case 5:
     act("You concentrate and mumble to yourself.", 1, ch, 0, 0, TO_CHAR) ;
     act("$n concentrates, and mumbles to $mself.", 1, ch, 0, 0, TO_ROOM) ;
     cast_harm (GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, tch, 0) ;
     break;
   case 6:
   case 7:
     act ("You look deep into the eyes of $N.", 1, ch, 0, tch, TO_CHAR) ;
     act ("$n looks deep into the eyes of $N.", 1, ch, 0, tch, TO_NOTVICT) ;
     act ("You see an ill-boding flame in the eye of $n.",
	  1, ch, 0, tch, TO_VICT) ;
     cast_fireball (GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, tch, 0) ;
     break ;
   default:
     if (!number (0,1)) {
       act ("You utter the words, 'pzar', to heal yourself.",
	    1, ch, 0, 0, TO_CHAR) ;
       act ("$n utters the words, 'pzar'", 1, ch, 0, 0, TO_ROOM) ;
       cast_heal (GET_LEVEL(ch), ch, "", SPELL_TYPE_SPELL, ch, 0) ;
     }
     break;
   }

   ch->points.mana -= 1 ;

   return ;
 }

 /* Function king_welmar */
 /* Control the actions and movements of the King. */
 /* Used by King Welmar */
 int king_welmar(CHAR *mob,CHAR *ch,int cmd,char *arg)
 {
   static char *monolog [] = {
     "$n declaims 'Primus in regnis Geticis coronam'.",
     "$n declaims 'regiam gessi, subiique regis'.",
     "$n declaims 'munus et mores colui sereno'.",
     "$n declaims 'principe dignos'." } ;

   static char bedroom_path[] = "s33004o1c1S.";

   static char throne_path[] = "W3o3cG52211rg.";

   static char monolog_path[] = "ABCD." ;

   static char *path;
   static int index;
   static bool move = FALSE;

   if (!move) {
     if (time_info.hours == 8 && CHAR_REAL_ROOM(mob) == R_ROOM (Z_KINGS_C, 51)) {
       move = TRUE;
       path = throne_path;
       index = 0;
     }
     else if (time_info.hours == 12 && CHAR_REAL_ROOM(mob) == R_ROOM (Z_KINGS_C, 17)) {
       move = TRUE;
       path = monolog_path;
       index = 0;
     }
     else if (time_info.hours == 21 && CHAR_REAL_ROOM(mob) == R_ROOM (Z_KINGS_C, 17)) {
       move = TRUE;
       path = bedroom_path;
       index = 0;
     }
   }

   if (cmd || (GET_POS(mob) < POSITION_SLEEPING) ||
       (GET_POS(mob) == POSITION_SLEEPING && !move))
     return FALSE;

   if (GET_POS(mob) == POSITION_FIGHTING) {
     fry_victim (mob) ;
     return FALSE ;
   }
   else
     if (banzaii (mob))
       return FALSE ;

   if (!move)
     return FALSE ;

   switch (path[index]) {
   case '0' :
   case '1' :
   case '2' :
   case '3' :
   case '4' :
   case '5' :
     do_move(mob,"",path[index]-'0'+1);
     break;
   case 'A': case 'B': case 'C': case 'D':
     act(monolog [path[index]-'A'], FALSE, mob, 0, 0, TO_ROOM);
     break ;
   case 'W' :
     GET_POS(mob) = POSITION_STANDING;
     act("$n awakens and stands up.",FALSE,mob,0,0,TO_ROOM);
     break;
   case 'S' :
     GET_POS(mob) = POSITION_SLEEPING;
     act("$n lies down on $s beautiful bed and instantly falls asleep.",
	 FALSE,mob,0,0,TO_ROOM);
     break;
   case 'r' :
     GET_POS(mob) = POSITION_SITTING;
     act("$n sits down on his great throne.", FALSE, mob, 0,0,TO_ROOM);
     break;
   case 's' :
     GET_POS(mob) = POSITION_STANDING;
     act("$n stands up.",FALSE,mob,0,0,TO_ROOM);
     break;
   case 'G' :
     do_say(mob, "Good morning, trusted friends.",CMD_SAY);
     break;
   case 'g' :
     do_say(mob, "Good morning, dear subjects.",CMD_SAY);
     break;
   case 'o' :
     do_unlock(mob, "door", 0);
     do_open(mob, "door", 0);
     break;
   case 'c' :
     do_close(mob, "door", 0);
     do_lock(mob, "door", 0);
     break;
   case '.' :
     move = FALSE;
     break;
   }

   index++;
   return FALSE;
 }

 /* Function training_master */
 /* Acts actions to the training room, if his students are present */
 /* Also allowes warrior-class to practice */
 /* Used by the Training Master */
 int training_master(CHAR *mob,CHAR *ch,int cmd,char *arg) {

   CHAR *pupil1, *pupil2, *tch ;
   int guild(CHAR *mob, CHAR *ch, int cmd, char *arg);

   if (!AWAKE(mob) || (GET_POS(mob)==POSITION_FIGHTING))
     return FALSE ;

   if ((cmd == CMD_PRACTICE)) {
     if (GET_CLASS(mob) == CLASS_WARRIOR)
       guild (mob, ch, cmd, arg) ;
     else
       send_to_char ("The Training Master does not allow your class to practice here.\n\r", ch) ;
     return TRUE ;
   }

   if (cmd)
     return FALSE ;

   if (!banzaii (mob) && !number (0,2)) {
     if ((pupil1 = find_npc_by_name (mob, "Brian", 5)) &&
	 (pupil2 = find_npc_by_name (mob, "Mick", 4)) &&
	 (!pupil1->specials.fighting) && (!pupil2->specials.fighting)) {
       if (number(0,1)) {
	 tch = pupil1 ;
	 pupil1 = pupil2 ;
	 pupil2 = tch ;
       }
       switch (number (0,7)) {
       case 0:
	 act ("$n hits $N on $s head with a powerful blow.",
	      FALSE, pupil1, 0, pupil2, TO_NOTVICT) ;
	 act ("You hit $N on $s head with a powerful blow.",
	      FALSE, pupil1, 0, pupil2, TO_CHAR) ;
	 act ("$n hits you on your head with a powerful blow.",
	      FALSE, pupil1, 0, pupil2, TO_VICT) ;
	 break ;
       case 1:
	 act ("$n hits $N in $s chest with a thrust.",
	      FALSE, pupil1, 0, pupil2, TO_NOTVICT) ;
	 act ("You manage to thrust $N in the chest.",
	      FALSE, pupil1, 0, pupil2, TO_CHAR) ;
	 act ("$n manages to thrust you in your chest.",
	      FALSE, pupil1, 0, pupil2, TO_VICT) ;
	 break ;
       case 2:
	 send_to_char ("You command your pupils to bow\n\r.", mob) ;
	 act ("$n commands his pupils to bow.", FALSE, mob, 0, 0, TO_ROOM) ;
	 act ("$n bows before $N.",
	      FALSE, pupil1, 0, pupil2, TO_NOTVICT) ;
	 act ("$N bows before $n.",
	      FALSE, pupil1, 0, pupil2, TO_NOTVICT) ;
	 act ("You bow before $N, who returns your gesture.",
	      FALSE, pupil1, 0, pupil2, TO_CHAR) ;
	 act ("You bow before $n, who returns your gesture.",
	      FALSE, pupil1, 0, pupil2, TO_VICT) ;
	 break ;
       case 3:
	 act ("$N yells at $n, as he fumbles and drops his sword.",
	      FALSE, pupil1, 0, mob, TO_NOTVICT) ;
	 act ("$n quickly picks up his weapon.", FALSE, pupil1, 0, 0, TO_ROOM) ;
	 act ("$N yells at you, as you fumble, losing your weapon.",
	      FALSE, pupil1, 0, mob, TO_CHAR) ;
	 send_to_char ("You quickly pick up your weapon again.", pupil1) ;
	 act ("You yell at $n, as he fumbles, losing his weapon.",
	      FALSE, pupil1, 0, mob, TO_VICT) ;
	 break ;
       case 4:
	 act ("$N tricks $n, and slashes him across the back.",
	      FALSE, pupil1, 0, pupil2, TO_NOTVICT) ;
	 act ("$N tricks you, and slashes you across your back.",
	      FALSE, pupil1, 0, pupil2, TO_CHAR) ;
	 act ("You trick $n, and quickly slash him across his back.",
	      FALSE, pupil1, 0, pupil2, TO_VICT) ;
	 break ;
       case 5:
	 act ("$n lunges a blow at $N but $N parries skillfully.",
	      FALSE, pupil1, 0, pupil2, TO_NOTVICT) ;
	 act ("You lunge a blow at $N but $E parries skillfully.",
	      FALSE, pupil1, 0, pupil2, TO_CHAR) ;
	 act ("$n lunges a blow at you, but you skillfully parry it.",
	      FALSE, pupil1, 0, pupil2, TO_VICT) ;
	 break ;
       case 6:
	 act ("$n clumsily tries to kick $N, but misses.",
	      FALSE, pupil1, 0, pupil2, TO_NOTVICT) ;
	 act ("You clumsily miss $N with your poor excuse for a kick.",
	      FALSE, pupil1, 0, pupil2, TO_CHAR) ;
	 act ("$n fails an unusually clumsy attempt at kicking you.",
	      FALSE, pupil1, 0, pupil2, TO_VICT) ;
	 break ;
       default:
	 send_to_char ("You show your pupils an advanced technique.", mob) ;
	 act ("$n shows his pupils an advanced technique.",
	      FALSE, mob, 0, 0, TO_ROOM) ;
	 break ;
       }
     }
   }
   return FALSE ;
 }


 int tom (CHAR *mob,CHAR *ch, int cmd, char *arg) {

   CHAR *king, *tim ;


   if (!AWAKE(mob))
     return FALSE ;

   if ((!cmd) && ((king = find_npc_by_name (mob, "King Welmar", 11)))) {
     if (!mob->master)
       do_follow (mob, "King Welmar", 0) ;
     if (king->specials.fighting)
       do_npc_rescue (mob, king) ;
   }

   if (!cmd)
     if ((tim = find_npc_by_name (mob, "Tim", 3)))
       if (tim->specials.fighting && 2*tim->points.hit < mob->points.hit)
	 do_npc_rescue (mob, tim) ;

   if (!cmd && GET_POS(mob) != POSITION_FIGHTING)
     banzaii (mob) ;

   return block_way(ch, cmd, arg, CASTLE_ITEM(49), 1) ;
 }

 int tim(CHAR *mob,CHAR *ch, int cmd, char *arg) {

   CHAR *king, *tom ;


   if (!AWAKE(mob))
     return FALSE ;

   if ((!cmd) && ((king = find_npc_by_name (mob, "King Welmar", 11)))) {
     if (!mob->master)
       do_follow (mob, "King Welmar", 0) ;
     if (king->specials.fighting)
       do_npc_rescue (mob, king) ;
   }

   if (!cmd)
     if ((tom = find_npc_by_name (mob, "Tom", 3)))
       if (tom->specials.fighting && 2*tom->points.hit < mob->points.hit)
	 do_npc_rescue (mob, tom) ;

   if (!cmd && GET_POS(mob) != POSITION_FIGHTING)
     banzaii (mob) ;

   return block_way(ch, cmd, arg, CASTLE_ITEM(49), 1) ;
 }


 /* Routine for James the Butler */
 /* Complains if he finds any trash... */
 int James (CHAR *mob,CHAR *ch, int cmd, char *arg) {

   OBJ *i,*temp;

   if (cmd || !AWAKE(mob) || (GET_POS(mob)==POSITION_FIGHTING))
     return (FALSE) ;

   for (i = world[CHAR_REAL_ROOM(mob)].contents; i; i = temp) {
     temp = i->next_content;  /* Added temp - Ranger June 96 */
     if (is_trash (i)) {
       do_say(mob,"My oh my!  I ought to fire that lazy woman!", CMD_SAY);
       act ("$n picks up a piece of trash.", FALSE, mob, 0, 0, TO_ROOM) ;
       obj_from_room (i) ;
       obj_to_char (i, mob) ;
       break;
     }
   }
   return FALSE;
 }

 /* Routine for the Cleaning Woman */
 /* Picks up any trash she finds... */
 int cleaning(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   OBJ *i,*temp;

   if (cmd || !AWAKE(mob))
     return(FALSE);

   for (i = world[CHAR_REAL_ROOM(mob)].contents; i; i = temp ) {
     temp = i->next_content;  /* Added temp - Ranger June 96 */
     if (is_trash (i)) {
       act("$n picks up some trash.", FALSE, mob, 0, 0, TO_ROOM);
       obj_from_room (i);
       obj_to_char (i, mob);
       break;
     }
   }

   return FALSE ;
 }


 /* Routine CastleGuard */
 /* Standard routine for ordinary castle guards */
 int CastleGuard (CHAR *mob,CHAR *ch,int cmd,char *arg)
 {

   if (cmd || !AWAKE(mob) || (GET_POS(mob) == POSITION_FIGHTING))
     return FALSE ;

   if (banzaii (mob)) return FALSE;
   return FALSE;
 }

 /* Routine DicknDave */
 /* Routine for the guards Dick and David */
 int DicknDavid(CHAR *mob,CHAR *ch,int cmd,char *arg) {

   if (!AWAKE(mob))
     return (FALSE) ;

   if (!cmd && GET_POS(mob) != POSITION_FIGHTING)
     banzaii (mob) ;

   return (block_way (ch, cmd, arg, CASTLE_ITEM(36), 1)) ;
 }

 /* Routine peter */
 /* Routine for Captain of the Guards. */
 int peter (CHAR *mob,CHAR *ch, int cmd, char *arg) {

   CHAR *ch_guard ;

   if (cmd || !AWAKE(mob) || GET_POS(mob) == POSITION_FIGHTING)
     return (FALSE) ;

   if (banzaii (mob))
     return FALSE ;

   if (!(number (0,3)) && (ch_guard = find_guard (mob)))
     switch (number (0,5)) {
     case 0:
       act ("$N comes sharply into attention as $n inspects $M.",
	    FALSE, mob, 0, ch_guard, TO_NOTVICT) ;
       act ("$N comes sharply into attention as you inspect $M.",
	    FALSE, mob, 0, ch_guard, TO_CHAR) ;
       act ("You go sharply into attention as $n inspects you.",
	    FALSE, mob, 0, ch_guard, TO_VICT) ;
       break ;
     case 1:
       act ("$N looks very small, as $n roars at $M.",
	    FALSE, mob, 0, ch_guard, TO_NOTVICT) ;
       act ("$N looks very small as you roar at $M.",
	    FALSE, mob, 0, ch_guard, TO_CHAR) ;
       act ("You feel very small as $N roars at you.",
	    FALSE, mob, 0, ch_guard, TO_VICT) ;
       break ;
     case 2:
       act ("$n gives $N some Royal directions.",
	    FALSE, mob, 0, ch_guard, TO_NOTVICT) ;
       act ("You give $N some Royal directions.",
	    FALSE, mob, 0, ch_guard, TO_CHAR) ;
       act ("$n gives you some Royal directions.",
	    FALSE, mob, 0, ch_guard, TO_VICT) ;
       break ;
     case 3:
       act ("$n looks at you.", FALSE, mob, 0, ch_guard, TO_VICT) ;
       act ("$n looks at $N.", FALSE, mob, 0, ch_guard, TO_NOTVICT) ;
       act ("$n growls: 'Those boots need polishing!'",
	    FALSE, mob, 0, ch_guard, TO_ROOM) ;
       act ("You growl at $N.", FALSE, mob, 0, ch_guard, TO_CHAR) ;
       break ;
     case 4:
       act ("$n looks at you.", FALSE, mob, 0, ch_guard, TO_VICT) ;
       act ("$n looks at $N.", FALSE, mob, 0, ch_guard, TO_NOTVICT) ;
       act ("$n growls: 'Straighten that collar!'",
	    FALSE, mob, 0, ch_guard, TO_ROOM) ;
       act ("You growl at $N.", FALSE, mob, 0, ch_guard, TO_CHAR) ;
       break ;
     default:
       act ("$n looks at you.", FALSE, mob, 0, ch_guard, TO_VICT) ;
       act ("$n looks at $N.", FALSE, mob, 0, ch_guard, TO_NOTVICT) ;
       act ("$n growls: 'That chain mail looks rusty!  CLEAN IT !!!'",
	    FALSE, mob, 0, ch_guard, TO_ROOM) ;
       act ("You growl at $N.", FALSE, mob, 0, ch_guard, TO_CHAR) ;
       break ;
     }

   return FALSE ;
 }

 /* Procedure for Jerry and Michael in x08 of King's Castle.      */
 /* Code by Sapowox modified by Pjotr.(Original code from Master) */

 int jerry (CHAR *mob,CHAR *ch, int cmd, char *arg) {

   CHAR *gambler1, *gambler2, *tch ;

   if (!AWAKE(mob) || (GET_POS(mob)==POSITION_FIGHTING))
     return FALSE ;

   if (cmd)
     return FALSE ;

   if (!banzaii (mob) && !number (0,2)) {
     if ((gambler1 = mob) &&
	 (gambler2 = find_npc_by_name (mob, "Michael", 7)) &&
	 (!gambler1->specials.fighting) && (!gambler2->specials.fighting)) {
       if (number(0,1)) {
	 tch = gambler1 ;
	 gambler1 = gambler2 ;
	 gambler2 = tch ;
       }
       switch (number (0,5)) {
       case 0:
	 act ("$n rolls the dice and cheers loudly at the result.",
	      FALSE, gambler1, 0, gambler2, TO_NOTVICT) ;
	 act ("You roll the dice and cheer. GREAT!",
	      FALSE, gambler1, 0, gambler2, TO_CHAR) ;
	 act ("$n cheers loudly as $e rolls the dice.",
	      FALSE, gambler1, 0, gambler2, TO_VICT) ;
	 break ;
       case 1:
	 act ("$n curses the Goddess of Luck roundly as he sees $N's roll.",
	      FALSE, gambler1, 0, gambler2, TO_NOTVICT) ;
	 act ("You curse the Goddess of Luck as $N rolls.",
	      FALSE, gambler1, 0, gambler2, TO_CHAR) ;
	 act ("$n swears angrily. You are in luck!",
	      FALSE, gambler1, 0, gambler2, TO_VICT) ;
	 break ;
       case 2:
	 act ("$n sighs loudly and gives $N some gold.",
	      FALSE, gambler1, 0, gambler2, TO_NOTVICT) ;
	 act ("You sigh loudly at the pain of having to give $N some gold.",
	      FALSE, gambler1, 0, gambler2, TO_CHAR) ;
	 act ("$n sighs loudly as $e gives you your rightful win.",
	      FALSE, gambler1, 0, gambler2, TO_VICT) ;
	 break ;
       case 3:
	 act ("$n smiles remorsefully as $N's roll tops his.",
	      FALSE, gambler1, 0, gambler2, TO_NOTVICT) ;
	 act ("You smile sadly as you see that $N beats you. Again.",
	      FALSE, gambler1, 0, gambler2, TO_CHAR) ;
	 act ("$n smiles remorsefully as your roll tops his.",
	      FALSE, gambler1, 0, gambler2, TO_VICT) ;
	 break ;
       case 4:
	 act ("$n excitedly follows the dice with his eyes.",
	      FALSE, gambler1, 0, gambler2, TO_NOTVICT) ;
	 act ("You excitedly follow the dice with your eyes.",
	      FALSE, gambler1, 0, gambler2, TO_CHAR) ;
	 act ("$n excitedly follows the dice with his eyes.",
	      FALSE, gambler1, 0, gambler2, TO_VICT) ;
	 break ;
       default:
	 act ("$n says 'Well, my luck has to change soon', as he shakes the dice.",
	      FALSE, gambler1, 0, gambler2, TO_NOTVICT) ;
	 act ("You say 'Well, my luck has to change soon' and shake the dice.",
	      FALSE, gambler1, 0, gambler2, TO_CHAR) ;
	 act ("$n says 'Well, my luck has to change soon', as he shakes the dice.",
	      FALSE, gambler1, 0, gambler2, TO_VICT) ;
	 break ;
       }
     }
   }
  return FALSE ;
 }

 /*********************************************************************\
 |*  End of King's Castle Special Procedures                          *|
 \*********************************************************************/

#define WELMAR_ZONE    8900
#define WELMAR_WELMAR  ITEM(WELMAR_ZONE, 1)
#define WELMAR_PETER   ITEM(WELMAR_ZONE,12)
#define WELMAR_TRAINER ITEM(WELMAR_ZONE,13)
#define WELMAR_JAMES   ITEM(WELMAR_ZONE,16)
#define WELMAR_CLEANER ITEM(WELMAR_ZONE,17)
#define WELMAR_TIM     ITEM(WELMAR_ZONE,20)
#define WELMAR_TOM     ITEM(WELMAR_ZONE,21)
#define WELMAR_DICK    ITEM(WELMAR_ZONE,24)
#define WELMAR_DAVE    ITEM(WELMAR_ZONE,25)
#define WELMAR_JERRY   ITEM(WELMAR_ZONE,26)

void assign_welmar(void) {

  assign_mob(WELMAR_WELMAR ,  king_welmar);
  assign_mob(WELMAR_PETER  ,  peter);
  assign_mob(WELMAR_TRAINER,  training_master);
  assign_mob(WELMAR_JAMES  ,  James);
  assign_mob(WELMAR_CLEANER,  cleaning);
  assign_mob(WELMAR_TIM    ,  tim);
  assign_mob(WELMAR_TOM    ,  tom);
  assign_mob(WELMAR_DICK   ,  DicknDavid);
  assign_mob(WELMAR_DAVE   , DicknDavid);
  assign_mob(WELMAR_JERRY  ,  jerry);
  assign_mob(8903          ,  CastleGuard);
  assign_mob(8904          ,  CastleGuard);
  assign_mob(8905          ,  CastleGuard);
  assign_mob(8906          ,  CastleGuard);
  assign_mob(8907          ,  CastleGuard);
  assign_mob(8908          ,  CastleGuard);
  assign_mob(8909          ,  CastleGuard);
  assign_mob(8910          ,  CastleGuard);
  assign_mob(8911          ,  CastleGuard);
  assign_mob(8927          ,  CastleGuard);
  assign_mob(8928          ,  CastleGuard);
  assign_mob(8929          ,  CastleGuard);

}

