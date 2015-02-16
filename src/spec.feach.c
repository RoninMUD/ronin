/*
///   Special Procedure Module                   Orig. Date 12-24-1998
///
///   spec.feach.c --- Specials for Feach, by Zorak
///
///   Designed by Sumo, Ranger, and Eparchos of RoninMUD.
///   Using this code is not allowed without express permission of
///   the originators or the current administration of RoninMUD.
*/

/*
$Author: ronin $
$Date: 2004/02/05 16:10:37 $
$Header: /home/ronin/cvs/ronin/spec.feach.c,v 2.0.0.1 2004/02/05 16:10:37 ronin Exp $
$Id: spec.feach.c,v 2.0.0.1 2004/02/05 16:10:37 ronin Exp $
$Name:  $
$Log: spec.feach.c,v $
Revision 2.0.0.1  2004/02/05 16:10:37  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
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
#include "cmd.h"
#include "spec_assign.h"
#include "fight.h"
#define ZONE_FEACH              8300

#define POOBAH               ITEM(ZONE_FEACH, 35)
#define BABOON               ITEM(ZONE_FEACH, 52)
#define PLANT                ITEM(ZONE_FEACH, 53)
#define COCKATIEL            ITEM(ZONE_FEACH, 54)
#define FUNGUS               ITEM(ZONE_FEACH, 55)
#define KNIGHT               ITEM(ZONE_FEACH, 56)
#define ICEBEAR              ITEM(ZONE_FEACH, 57)
#define LASHERTOAD           ITEM(ZONE_FEACH, 58)
#define MILLIPEDE            ITEM(ZONE_FEACH, 36)
#define RUSTYGAUNT           ITEM(ZONE_FEACH, 35)


int f_poobah(CHAR *mob,CHAR *ch, int cmd, char *arg) {
   CHAR *vict=0;

   switch (cmd)
     {
     case MSG_MOBACT:
       if (!mob->specials.fighting) return FALSE;
       if(number(0,3)) return FALSE;
       vict=get_random_victim_fighting(mob);
       if(!vict) return FALSE;

       switch(number(0,1))
         {
         case 0:
           act("You kick sand in $N's face, cackling!", 1, mob, 0, vict,TO_CHAR);
           act("$n kicks sand in $N's face, cackling!", 1, mob, 0, vict,TO_NOTVICT);
           act("AAAGH! $n has blinded you with a bunch of sand!", 1, mob, 0, vict ,TO_VICT);
           spell_blindness(GET_LEVEL(mob), mob, vict, 0);
           break;
         case 1:
           act("You kick a bunch of sand around, looking foolish.", 1, mob, 0, vict,TO_CHAR);
           act("$n kicks a bunch of sand around, looking foolish.", 1, mob, 0, vict,TO_NOTVICT);
           act("$n tries to kick sand in your face.", 1, mob, 0, vict ,TO_VICT);
           break;
         }
       break;
     case MSG_DIE:
       if(number(0,3)) return FALSE;
       if(!mob->equipment[WEAR_HANDS]) return FALSE;
       if(!(vict=get_random_victim_fighting(mob))) return FALSE;;

       act("$n throws $s gauntlet at $N, striking $M squarely in the nose!", 0, mob, 0,vict ,TO_NOTVICT);
       act("You are struck in the nose by a heavy gauntlet which $n has hucked at you!", 0, mob, 0,vict ,TO_VICT);
       obj_to_char(unequip_char(mob,WEAR_HANDS),vict);
       damage(mob,vict, 100, TYPE_UNDEFINED,DAM_NO_BLOCK);
       break;
   }
   return FALSE;
 }

int f_baboon(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(cmd!=CMD_UP) return FALSE;
  act("$n slams his red rump into you, knocking you off the stairs.",0,mob,0,ch,TO_VICT);
  act("$n bashes $N off the stairs with $s red buttocks.",0,mob,0,ch,TO_NOTVICT);
  act("You slap $N off the stairs with your butt! Tingly!",0,mob,0,ch,TO_CHAR);
  return TRUE;
}

int f_plant(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(cmd!=CMD_UP) return FALSE;
  act("$n gently pulls you off the stairs with one of $s tendrils.",0,mob,0,ch,TO_VICT);
  act("$n pulls $N off the stairs with one of $s tendrils.",0,mob,0,ch,TO_NOTVICT);
  act("You pull $N off the stairs with your eighth tendril.",0,mob,0,ch,TO_CHAR);
  return TRUE;
}

int f_cockatiel(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(cmd!=CMD_UP) return FALSE;
  act("You fly off the stairs as $n flaps about your face and neck, screeching!",0,mob,0,ch,TO_VICT);
  act("$N falls off the stairs as $n flaps around his face.",0,mob,0,ch,TO_NOTVICT);
  act("You bravely flap around, screeching like a chicken, and knock $N back.",0,mob,0,ch,TO_CHAR);
  return TRUE;
}

int f_fungus(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(cmd!=CMD_UP) return FALSE;
  act("An extension of $n blorps across the stairs, blocking your escape.",0,mob,0,ch,TO_VICT);
  act("$N runs into a part of $n as $E tries to desert you.",0,mob,0,ch,TO_NOTVICT);
  act("Fungus stop $N.... GOOD Fungus!",0,mob,0,ch,TO_CHAR);
  return TRUE;
}

int f_knight(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(cmd!=CMD_UP) return FALSE;
  act("$n clanks around to block your passage.",0,mob,0,ch,TO_VICT);
  act("$N whimpers and whines as $n blocks $S escape.",0,mob,0,ch,TO_NOTVICT);
  act("You majestically clank around and block $N's retreat.",0,mob,0,ch,TO_CHAR);
  return TRUE;
}

int f_icebear(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(cmd!=CMD_UP) return FALSE;
  act("A gigantic bear seems to have installed itself in your path.",0,mob,0,ch,TO_VICT);
  act("$N whimpers as $n intercepts $S flight.",0,mob,0,ch,TO_NOTVICT);
  act("Looks like lunch wants to leave, you stop it.",0,mob,0,ch,TO_CHAR);
  return TRUE;
}

int f_lashertoad(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(cmd!=CMD_UP) return FALSE;
  act("A slimy, sticky tongue yanks you off the stairs as you try to escape.",0,mob,0,ch,TO_VICT);
  act("You see a pinkish flash as $n loops $s tongue around $N's leg and pulls $M off the stairs.",0,mob,0,ch,TO_NOTVICT);
  act("$N sure does look like a big fly from here....",0,mob,0,ch,TO_CHAR);
  return TRUE;
}

int f_millipede(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(cmd!=CMD_UP) return FALSE;
  act("$n rolls in front of you, tripping you up.",0,mob,0,ch,TO_VICT);
  act("$N trips and stumbles over $n as $E tries to escape.",0,mob,0,ch,TO_NOTVICT);
  act("You roll up and trip $N",0,mob,0,ch,TO_CHAR);
  return TRUE;
}

void assign_feach(void) {
  assign_mob(POOBAH,    f_poobah);
  assign_mob(BABOON,    f_baboon);
  assign_mob(PLANT,     f_plant);
  assign_mob(COCKATIEL, f_cockatiel);
  assign_mob(FUNGUS,    f_fungus);
  assign_mob(KNIGHT,    f_knight);
  assign_mob(ICEBEAR,   f_icebear);
  assign_mob(LASHERTOAD,f_lashertoad);
  assign_mob(MILLIPEDE, f_millipede);
}
