
/*
///   Special Procedure Module                   Orig. Date 18-02-1996
///
///   spec.druids.c --- Specials for the Druids
///
///   Written by Shun of RoninMUD - qxotl@u.washington.edu
///
///   Using this code is not allowed without permission from originator.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "utility.h"
#include "fight.h"

#include "utility.h"
#include "cmd.h"
#include "spec_assign.h"

#define DRUID_WOLF   24904

extern void do_say(CHAR*,char*,int);

int druid_protector(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
   CHAR *vict, *next_vict;
   int room;

   switch (cmd)
     {
     case CMD_EAST:
     case CMD_NORTH:
         if(!ch) return FALSE;
         room=CHAR_REAL_ROOM(ch);
         send_to_room("The Druid Protector gestures, and a wall of trees springs up.\n\r",room);
         return TRUE;

     case MSG_MOBACT:

         if (!mob->specials.fighting)
             return FALSE;

         switch (number(0,2))
         {
            /* The Vine Lash */

            case 0:
              if(!number(0,1))
              {
                if(!(vict = get_random_victim(mob))) return FALSE;
                if(vict == mob) return FALSE;

                act("$n calls upon the spirits of the forest...",
                     1, mob, 0, vict, TO_ROOM);
                act("Thorny vines lash out at $N from all angles!",
                     1, mob, 0, vict, TO_NOTVICT);
                act("Thorny vines lash out at you from all angles!",
                     1, mob, 0, vict, TO_VICT);
                act("You summon the vines of wrath to disable $N!",
                     1, mob, 0, vict, TO_CHAR);
                damage(mob, vict, 100, TYPE_UNDEFINED,DAM_NO_BLOCK);
                WAIT_STATE(vict,PULSE_VIOLENCE*2);

                if (mob_disarm(mob, vict, FALSE)) {
                  act("The vines tear $N's weapon from $M grasp!",
                    TRUE, mob, 0, vict, TO_NOTVICT);
                  act("The vines tear your weapon from your hands!",
                    TRUE, mob, 0, vict, TO_VICT);
                }
              }
              break;

            /* Ensaring */

            case 1:
              if(!number(0,1))
              {
                if(!(vict = get_random_victim(mob)))
                     return FALSE;
                if(vict == mob->specials.fighting)
                     return FALSE;
                if(vict == mob)
                     return FALSE;

                act("$n calls upon the spirits of the forest...",
                     1, mob, 0, vict, TO_ROOM);
                act("Tendril-like roots shoot from the earth, ensnaring $N!",
                     1, mob, 0, vict, TO_NOTVICT);
                act("Tendril-like roots shoot from the earth, ensnaring you!",
                     1, mob, 0, vict, TO_VICT);
                act("You summon the root children to restrain $N!",
                     1, mob, 0, vict, TO_CHAR);
                stop_fighting(vict);
                WAIT_STATE(vict,PULSE_VIOLENCE*5);
                break;
              }
              break;

            /* Shards of Earth */

            case 2:
              if(!number(0,1))
              {
                if(!(vict = mob->specials.fighting))
                     return FALSE;
                if(vict == mob)
                     return FALSE;

                act("$n calls upon the spirits of the earth...",
                     1, mob, 0, vict, TO_ROOM);
                act("You call upon the spirits of the earth to eliminate your enemies...",
                     1, mob, 0, vict, TO_CHAR);

                for(vict = world[mob->in_room_r].people; vict;vict = next_vict)
                {
                     if(vict == mob)
                          return FALSE;
                     if(!IS_NPC(vict)) {
                        act("Spiral shards of rock rise from the earth, impaling you!",
                             1, mob, 0, vict, TO_VICT);
                        damage(mob, vict, 200, TYPE_UNDEFINED,DAM_NO_BLOCK);
                     }
                     next_vict = vict->next_in_room;
                }
                act("The earth rises up in spiral shards, impaling your enemies!",
                     1, mob, 0, vict, TO_CHAR);
              }
              break;
         }
     }
     return FALSE;
}

int druid_forest(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
   CHAR *vict;

   switch (cmd)
     {
     case MSG_MOBACT:
         if (!mob->specials.fighting)
             return FALSE;
         if (!(vict = mob->specials.fighting))
             return FALSE;

         if(!number(0,3))
           {
              act("$n closes $s eyes in intense concentration...",
                   1, mob, 0, vict,TO_ROOM);
              act("An enormous eagle swoops from the heavens and rakes at $N's eyes!",
                   1, mob, 0, vict, TO_NOTVICT);
              act("An enormous eagle swoops from the heavens and claws at your eyes!",
                   1, mob, 0, vict ,TO_VICT);
              act("You summon an eagle to gouge the eyes from $N's head!",
                   1, mob, 0, vict, TO_CHAR);
              damage(mob, vict, 60, TYPE_UNDEFINED,DAM_NO_BLOCK);
              spell_blindness(GET_LEVEL(mob), mob, vict, 0);
              WAIT_STATE(vict,PULSE_VIOLENCE*2);
              act("The eagle shrieks as it returns to the heavens, its duty done.",
                   1, mob, 0, vict, TO_ROOM);
              break;
           }
         break;

     }

   return FALSE;

}

int druid_guardian(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
   CHAR *vict, *wolf;
   OBJ *wield;
   char buf[MAX_INPUT_LENGTH];
   int wolf_count, i, num_wolves;

   switch (cmd)
     {
     case CMD_RECITE:

         if (!ch) return FALSE;

         if(EQ(ch,WIELD)) {
            wield = EQ(ch,WIELD);
            if(V_OBJ(wield)==11523) return FALSE;
            act("$n gives your hand a swift kick as you read.\n",
                1, mob, 0, ch, TO_VICT);
            act("$n gives $N a swift kick to the hand as $M reads $S scroll.\n",
                1, mob, 0, ch, TO_NOTVICT);
            act("You kick the heathen's weapon of destruction from $S hand as $M reads.\n",
                1, mob, 0, ch, TO_CHAR);
            sprintf(buf,"WIZINFO: %s disarms %s's %s (Room %d)",GET_NAME(mob),GET_NAME(ch),OBJ_SHORT(wield),world[CHAR_REAL_ROOM(ch)].number);
            log_s(buf);
            wield->log = TRUE;
            unequip_char(ch, WIELD);
            obj_to_room(wield, CHAR_REAL_ROOM(ch));
            wield->log=1;
         }
         break;

     case MSG_ENTER:

         if (!ch) return FALSE;
         if(IS_NPC(ch)) return FALSE;
         if (mob->specials.fighting) return FALSE;
         if (!CAN_SEE(mob,ch)) return FALSE;

         act("$n is alerted to $N's presence.",0,mob,0,ch,TO_NOTVICT);
         act("You are alerted to $N's presence.",0,mob,0,ch,TO_CHAR);
         act("$n is alerted to your presence.",0,mob,0,ch,TO_VICT);
         damage(mob, ch, 30, TYPE_UNDEFINED,DAM_NO_BLOCK);
         break;

     case CMD_KILL:

         if(!ch) return FALSE;
         if(IS_NPC(ch)) return FALSE;
         if(mob->specials.fighting)
            return FALSE;

         wolf_count = mob_proto_table[real_mobile(DRUID_WOLF)].number;

         if(wolf_count < 4 && wolf_count >= 0) {

            num_wolves = number(1,5);

            for(i = 0; i < num_wolves; i++) {
                wolf = read_mobile(DRUID_WOLF, VIRTUAL);
                char_to_room(wolf, CHAR_REAL_ROOM(ch));
                act("$n summons a friend from the shadows to assist him!",
                    1, mob, 0, ch, TO_ROOM);
                damage(wolf, ch, 0, TYPE_UNDEFINED,DAM_NO_BLOCK);
            }

         }
         break;

     case MSG_MOBACT:
         if (!mob->specials.fighting)
             return FALSE;
         if (!(vict = get_random_victim(mob)))
             return FALSE;

         if(!number(0,3)) {

            switch(number(0,2)) {

              case 0:     /* Cloak over the head and blindness */

                act("$n pulls his cloak over $N's head!\n$N seems to be blinded!",
                     1, mob, 0, vict,TO_NOTVICT);
                act("$n pulls his cloak over your head!\nYou seem to be blinded!",
                     1, mob, 0, vict,TO_VICT);
                spell_blindness(GET_LEVEL(mob), mob, vict,0);
                WAIT_STATE(vict,PULSE_VIOLENCE*2);
                break;

              case 1:          /* skin mutation and self-heal */

                 act("$n's skin bubbles and mutates in front of your eyes...\n$n has healed his own wounds!",
                     1, mob, 0, vict, TO_ROOM);
                 spell_heal(30, mob, mob, 0);
                 spell_heal(30, mob, mob, 0);
                 spell_heal(30, mob, mob, 0);
                 break;

              case 2:          /* disappear and lunge */

                 act("$n disappears into the shadows...",
                     1, mob, 0, vict, TO_ROOM);

                 act("and lunges toward $N with raging fury!",
                     1, mob, 0, vict, TO_NOTVICT);
                 act("and lunges toward you with raging fury!",
                     1, mob, 0, vict, TO_VICT);
                 damage(mob, vict, 150, TYPE_UNDEFINED,DAM_NO_BLOCK);
                 WAIT_STATE(vict, PULSE_VIOLENCE);
                 break;

             }
           }
         break;

     }

   return FALSE;

 }


 /**********************************************************************\
 |* End Of the Special procedures for mobs                             *|
 \**********************************************************************/

/* Special Procedures for Rooms */

int
druid_warning_room(int room, CHAR *ch, int cmd, char *arg)
{

    char* message = "\n\r                An interwoven cross of burning brambles\n\r";
    char* message1 ="                appears in the sky to the north.\n\r";
    char* message2 ="\n\r                A voice whispers in your ear, the voice\n\r";
    char* message3 ="                of the wind, telling you to go back to\n\r";
    char* message4 ="                your comfortable life and leave these\n\r";
    char* message5 ="                dangerous lands.\n\r";

    if(cmd!=MSG_ENTER) return FALSE;

    if(!ch) return FALSE;
    send_to_char(message,ch);
    send_to_char(message1,ch);
    send_to_char(message2,ch);
    send_to_char(message3,ch);
    send_to_char(message4,ch);
    send_to_char(message5,ch);

    return FALSE;
}

/* End Special Procedures for Rooms */


void assign_druid (void) {

  assign_room(24971 ,  druid_warning_room);

  assign_mob(24900  ,  druid_guardian);
  assign_mob(24902  ,  druid_forest);
  assign_mob(24903  ,  druid_protector);
}

