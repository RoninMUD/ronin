
/*
///   Special Procedure Module                   Orig. Date 15-02-1994
///
///   spec.chaos.c --- Specials for The Chaos Kingdom
///
///   Using this code is not allowed without permission from originator.
*/

/*
$Author: ronin $
$Date: 2005/01/21 14:55:29 $
$Header: /home/ronin/cvs/ronin/spec.nergal.c,v 2.1 2005/01/21 14:55:29 ronin Exp $
$Id: spec.nergal.c,v 2.1 2005/01/21 14:55:29 ronin Exp $
$Name:  $
$Log: spec.nergal.c,v $
Revision 2.1  2005/01/21 14:55:29  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.0.0.1  2004/02/05 16:11:07  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "mob.spells.h"
#include "limits.h"
#include "utility.h"
#include "fight.h"
#include "act.h"

#include "utility.h"
#include "cmd.h"
#include "spec_assign.h"


int nergal_ring(OBJ *ring, CHAR *ch, int cmd, char *arg) {

  if(!ch) return FALSE;

  switch(cmd)
    {
    case MSG_OBJ_WORN:
      act("$n's face flushes dark with anger as $e wears the ring.",FALSE,ch,ring,NULL,TO_NOTVICT);
      act("You feel a surge of dark power as you wear the ring.",FALSE,ch,ring,NULL,TO_CHAR);
      break;

    case MSG_OBJ_REMOVED:
      act("$n seems more peaceful, yet less powerful as $e removes the ring.",FALSE,ch,ring,NULL,TO_NOTVICT);
      act("You feel the power bleed out of you as you remove the Ring of the Conqueror.",FALSE,ch,ring,NULL,TO_CHAR);
      break;
    }

  return FALSE;
}


/* This assumes that the room controls the next 9 rooms, and changes
   the exits to two way exits among them, randomly */
/*  it also assumes each of those rooms has all 4 (n/s/e/w) exits */
int nergal_MazePoint(int room, CHAR *ch, int cmd, char *arg)
{
    int z,i, x, found, target, direct, tries;
    switch (cmd)
	{
        case MSG_MOBACT:
          for(i= room; i< room+10; i++)
            {
            switch(number(0,35))
              {
              case 0:
                 send_to_room ("You hear quiet gibbering coming from the east.\r\n", i);
                 break;
              case 1:
                 send_to_room ("You hear quiet gibbering coming from the west.\r\n", i);
                 break;
              case 2:
                 send_to_room ("You hear quiet gibbering coming from the north.\r\n", i);
                 break;
              case 3:
                 send_to_room ("You hear quiet gibbering coming from the south.\r\n", i);
                 break;
              case 4:
                 send_to_room ("In the distance, a keening wail raises above the silence.\r\n", i);
                 break;
              case 5:
                 send_to_room ("Very close, a keening wail pierces above the silence.\r\n", i);
                 break;
              case 6:
                 send_to_room ("Something whispers in your ear, 'xua mant goda Nergal!'\r\n", i);
                 break;
              case 7:
                 send_to_room ("Seemingly right behind you, giggling breaks out.\r\n", i);
                 break;
              }
            }
          break;
        case MSG_TICK:
          /*  reset all exits to 0 */
          for(i= room; i< room+10; i++)
            {
            if(!number(0,3))
              send_to_room ("A chill wind blows down the street.\r\n", i);
            for(x=0;x<4;x++)
              {
              if(world[i].dir_option[x])
                {
                world[i].dir_option[x]->to_room_r = -1;
                world[i].dir_option[x]->to_room_v = -1;
                }
              }
            }
          if(world[room].dir_option[WEST])
             {
             world[room].dir_option[WEST]->to_room_r = room -1;
             world[room].dir_option[WEST]->to_room_v = world[room -1].number;
             }
          if(world[room+9].dir_option[EAST])
             {
             world[room+9].dir_option[EAST]->to_room_r = room +10;
             world[room+9].dir_option[EAST]->to_room_v = world[room +10].number;
             }
          for(i=room;i<room+10;i++)
            {
            for(x=0;x<2;x++)
              {
              tries = 0;
              found = TRUE;
              for(z=0;z<4;z++)
                {
                if(world[i].dir_option[z])
                  if(world[i].dir_option[z]->to_room_r == -1)
                     found = FALSE;
                 }
              while(!found)
                {
                tries++;
                if(tries > 300)
                    found = TRUE;
                target = number(room+1,room+9);
                while(target == i)
                  target = number(room+1,room+9);
                direct = number (0,3);
                if(world[i].dir_option[direct])
                  {
                  if(world[i].dir_option[direct]->to_room_r == -1)
                    {
                    if(world[target].dir_option[rev_dir[direct]])
                      {
                      if(world[target].dir_option[rev_dir[direct]]->to_room_r == -1)
                        {
                        found = TRUE;
                        world[i].dir_option[direct]->to_room_r = target;
                        world[i].dir_option[direct]->to_room_v = world[target].number;
                        world[target].dir_option[rev_dir[direct]]->to_room_r = i;
                        world[target].dir_option[rev_dir[direct]]->to_room_v = world[i].number;
                        }
                      }
                    }
                  }
                }
              }
            }
          break;
        }
   return FALSE;
}

int
nergal_Lash (OBJ *lash, CHAR *ch, int cmd, char *arg) {

  if(lash->equipped_by) {
/*
    if(GET_CLASS(lash->equipped_by)==CLASS_NINJA) {
       if(lash->affected[0].modifier == 10) {
         lash->equipped_by->points.damroll-=5;
         lash->affected[0].modifier = 5;
       }
     }
     else {
       lash->affected[0].modifier = 10;
     }
*/
    switch(cmd) {
      case MSG_MOBACT:
        if(!number(0,2)) {
          if(lash->equipped_by->specials.fighting) {
            act("$n lashes $N with a stinging blow!", 0, lash->equipped_by, 0,  lash->equipped_by->specials.fighting, TO_NOTVICT);
            act("$n lashes you with a stinging blow!", 0, lash->equipped_by, 0,  lash->equipped_by->specials.fighting, TO_VICT);
            act("You lash $N with a stinging blow!", 0, lash->equipped_by, 0,  lash->equipped_by->specials.fighting, TO_CHAR);
            GET_POS(lash->equipped_by->specials.fighting) = POSITION_STUNNED;
            WAIT_STATE(lash->equipped_by->specials.fighting, PULSE_VIOLENCE*2);
          }
        }
        break;
    }
  }

/*
  else {
    lash->affected[0].modifier = 10;
    switch(cmd) {
      case CMD_WIELD:
        if(GET_CLASS(ch)==CLASS_NINJA)
           lash->affected[0].modifier = 5;
        else
           lash->affected[0].modifier = 10;
        break;
    }
  }
*/

  return FALSE;
}




int nergal_Consort(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
   CHAR *vict;
   struct affected_type_5 af;

   switch (cmd)
     {
     case MSG_MOBACT:
           if (!mob->specials.fighting)
              return FALSE;
	   for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room )
	     if (vict->specials.fighting==mob && number(0,3)==0)
	       break;

	   if (!vict) return FALSE;

           switch(number(0,2))
             {
             case 0:   /*VOMIT!*/
	       spell_acid_breath(GET_LEVEL(mob), mob, vict, 0);
	       spell_acid_breath(GET_LEVEL(mob), mob, vict, 0);
               break;
             case 1:  /*Diseased Claw!*/
               af.type = SPELL_POISON;
               af.modifier = -6;
               af.location = APPLY_STR;
               af.duration = number(2,4);
               af.bitvector = AFF_POISON;
               af.bitvector2 = 0;
               affect_to_char (vict, &af);

	       act("$n claws $N with $s filthy claws!", 1, mob, 0, vict,TO_NOTVICT);
	       act("$n rakes you with $s filthy claws!", 1, mob, 0, vict ,TO_VICT);
               break;
             case 2: /*Ghastly Howl*/
	       act("$n emits a ghastly howl!", 1, mob, 0, 0,TO_ROOM);
	       for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room )
                   {
	           if (vict!=mob)
                       {
                       WAIT_STATE(vict, number(1,5)*PULSE_VIOLENCE);
	               act("$n's howl stuns you!", 1, mob, 0, vict ,TO_VICT);
                       }
                   }
               break;
             }
           break;
      case MSG_DIE:
           break;
       }
   return FALSE;
 }
int nergal_Lord(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
   OBJ *obj;
   CHAR *vict;
   struct affected_type_5 af;

   switch (cmd)
     {
     case MSG_MOBACT:
           if (!mob->specials.fighting)
              return FALSE;
	   for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room )
	     if (vict->specials.fighting==mob && number(0,3)==0)
	       break;

	   if (!vict) return FALSE;

           switch(number(0,2))
             {
             case 0:   /*VOMIT!*/
	       act("$n vomits on $N!", 1, mob, 0, vict,TO_NOTVICT);
	       act("$n vomits on you!", 1, mob, 0, vict ,TO_VICT);
	       spell_acid_breath(GET_LEVEL(mob), mob,  vict, 0);
	       spell_acid_breath(GET_LEVEL(mob), mob,  vict, 0);
               break;
             case 1:  /*Diseased Claw!*/
               af.type = SPELL_POISON;
               af.modifier = -6;
               af.location = APPLY_STR;
               af.duration = number(2,4);
               af.bitvector = AFF_POISON;
               af.bitvector2 = 0;
               affect_to_char (vict, &af);

	       act("$n claws $N with $s filthy claws!", 1, mob, 0, vict,TO_NOTVICT);
	       act("$n rakes you with $s filthy claws!", 1, mob, 0, vict ,TO_VICT);
               break;
             case 2: /*Ghastly Howl*/
	       act("$n emits a ghastly howl!", 1, mob, 0, 0,TO_ROOM);
	       for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room )
                   {
	           if (vict!=mob)
                       {
                       WAIT_STATE(vict, number(1,5)*PULSE_VIOLENCE);
	               act("$n's howl stuns you!", 1, mob, 0, vict ,TO_VICT);
                       }
                   }
               break;
             }
           break;
      case MSG_DIE:
           if(!ch || ch == mob)
             return FALSE;
	   act("$n's body ruptures in a noxious cloud.", 0, mob, 0,0 ,TO_ROOM);
	   cast_poison_smoke(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, 0, 0);
           act("$n's death cry wakes the dead.", 0, mob, 0, 0, TO_ROOM);
           /*stun everybody, for the kill */
	   for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room )
               {
	       if (vict!=mob)
                   {
                   WAIT_STATE(vict, number(2,5)*PULSE_VIOLENCE);
	           act("$n's howl stuns you and leaves you gasping in agony!", 1, mob, 0, vict ,TO_VICT);
                   }
               }
           /* summon a bunch of guards, and  his consort*/
           if(CHAR_VIRTUAL_ROOM(mob)!=6851) return FALSE;
           send_to_room("Nergal's consort and her entourage of guards burst in from the west.\r\n", mob->in_room_r);
           vict = read_mobile(6803, VIRTUAL);
           if(!number(0,10))
             {
             obj = read_object(6806, VIRTUAL);
             equip_char(vict, obj, HOLD);
             }
           char_to_room(vict, mob->in_room_r);
           set_fighting(vict, ch);
           do_yell(vict, "Nergal, my Lord!", CMD_YELL);
           send_to_world("A sharp, hollow keening raises above the wind.\r\n");
           do_say(vict, "Get them!", CMD_SAY);
           vict = read_mobile(6802, VIRTUAL);
           char_to_room(vict, mob->in_room_r);
           set_fighting(vict, ch);
           vict = read_mobile(6802, VIRTUAL);
           char_to_room(vict, mob->in_room_r);
           set_fighting(vict, ch);
           vict = read_mobile(6802, VIRTUAL);
           char_to_room(vict, mob->in_room_r);
           set_fighting(vict, ch);
          break;
       }
   return FALSE;
}
int nergal_bodyguard(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
   CHAR *vict;
   struct affected_type_5 af;

   switch (cmd) {
   case MSG_MOBACT:
     if (!mob->specials.fighting)
       return FALSE;
     for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room)
       if (vict->specials.fighting==mob && number(0,4)==0)
         break;

     if (!vict) return FALSE;

     switch(number(0,1)) {
     case 0:   /*VOMIT!*/
       act("$n vomits on $N!", 1, mob, 0, vict,TO_NOTVICT);
       act("$n vomits on you!", 1, mob, 0, vict ,TO_VICT);
       spell_acid_breath(GET_LEVEL(mob), mob, vict, 0);
       break;
     case 1:  /*Diseased Claw!*/
       af.type = SPELL_POISON;
       af.modifier = -6;
       af.location = APPLY_STR;
       af.duration = number(2,4);
       af.bitvector = AFF_POISON;
       af.bitvector2 = 0;
       affect_to_char (vict, &af);

       act("$n claws $N with $s filthy claws!", 1, mob, 0, vict,TO_NOTVICT);
       act("$n rakes you with $s filthy claws!", 1, mob, 0, vict ,TO_VICT);
       break;
     }
     break;
   case MSG_DIE:
     act("$n's body ruptures in a noxious cloud.", 0, mob, 0,0 ,TO_ROOM);
     cast_poison_smoke(GET_LEVEL(mob), mob, "", SPELL_TYPE_SPELL, 0, 0);
     break;
   }
   return FALSE;
}

int nergal_larvae(CHAR *mob,CHAR *ch, int cmd, char *arg)
 {
   CHAR *vict,*temp;

   switch (cmd)
     {
     case MSG_MOBACT:
         if (!mob->specials.fighting)
             return FALSE;
	   for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room )
	     if (vict->specials.fighting==mob && number(0,4)==0)
	       break;

	   if (!vict) return FALSE;

	   act("$n vomits on $N!", 1, mob, 0, vict,TO_NOTVICT);
	   act("$n vomits on you!", 1, mob, 0, vict ,TO_VICT);
	   spell_acid_breath(GET_LEVEL(mob), mob, vict, 0);
           break;
      case MSG_DIE:
	   act("$n's body explodes in a spray of ichor.", 0, mob, 0,0 ,TO_ROOM);
	   for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = temp) {
             temp = vict->next_in_room;  /* Added temp - Ranger June 96 */
	     if (vict!=mob && number(0,5)==0)
                {
	        act("$n's ichor sprays on $N!", 1, mob, 0, vict,TO_NOTVICT);
	        act("$n's ichor sprays on you!", 1, mob, 0, vict ,TO_VICT);
	        spell_acid_breath(GET_LEVEL(mob), mob, vict, 0);
                }
             }
           break;
       }
   return FALSE;
 }

  /**********************************************************************\
 |* End Of the Special procedures for misc mobs and such               *|
 \**********************************************************************/

void assign_nergal (void) {

  assign_room(6810,  nergal_MazePoint);
  assign_room(6820,  nergal_MazePoint);
  assign_room(6830,  nergal_MazePoint);
  assign_room(6840,  nergal_MazePoint);

//  assign_mob(6801 ,  nergal_larvae);
//  assign_mob(6802 ,  nergal_bodyguard);
// ^^ chained in spec.elmuseo.c
  assign_mob(6804 ,  nergal_Lord);

  assign_obj(6806 , nergal_Lash);
  assign_obj(29839, nergal_Lash);
  assign_obj(6807 , nergal_ring);
}

