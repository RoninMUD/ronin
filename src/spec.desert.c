/***************************************************************************\
*            Special procedure module for the Tarion Desert                 *
*                             by Thyas                                      *
*                                                                           *
*  Last Modified:    July 28, 1996 by Ranger
*                                                                           *
\***************************************************************************/

/*
$Author: ronin $
$Date: 2004/02/05 16:10:26 $
$Header: /home/ronin/cvs/ronin/spec.desert.c,v 2.0.0.1 2004/02/05 16:10:26 ronin Exp $
$Id: spec.desert.c,v 2.0.0.1 2004/02/05 16:10:26 ronin Exp $
$Name:  $
$Log: spec.desert.c,v $
Revision 2.0.0.1  2004/02/05 16:10:26  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
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
#include "fight.h"
#include "enchant.h"
#include "act.h"
#include "spec_assign.h"

#define ZONE_DESERT      10500
#define DESERT_SAND_STORM1    ITEM(ZONE_DESERT,9)
#define DESERT_SAND_STORM2    ITEM(ZONE_DESERT,17)
#define DESERT_SAND_STORM3    ITEM(ZONE_DESERT,22)
#define CAVALIER     10502
#define CHARGER      10503

void stop_riding(struct char_data *ch,struct char_data *vict);

int sand_storm(int room, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*tmp_ch;

  if (!ch) return FALSE;
  if (IS_NPC(ch)) return FALSE;

  if (GET_LEVEL(ch)>=LEVEL_IMM) return FALSE;
  if (room!=CHAR_REAL_ROOM(ch)) return FALSE;

  if (cmd > 0 && cmd < 7 && number(1,5)==2 ) {
    for (vict=world[room].people;vict;vict=tmp_ch) {
      tmp_ch=vict->next_in_room;
      if(GET_LEVEL(vict)<LEVEL_IMM) {
        act("A storm of sand and wind buffets you causing a moment of temporary blindness, you feel lost."
         ,FALSE,vict,0,0,TO_CHAR);
        act("$n is blinded by a storm of wind and sand and seems to disappear from the group!"
          ,1,vict,0,0,TO_ROOM);

/* Checks for a mount and a rider */

        if(vict->specials.riding) stop_riding(vict,vict->specials.riding);
        if(vict->specials.rider) stop_riding(vict->specials.rider,vict);

        char_from_room (vict);
        char_to_room (vict, real_room(10500 + number(2,30)));
      }
    }
    return TRUE;
  }
  return FALSE;
}


int cavalier(CHAR *cavalier,CHAR *ch, int cmd, char *arg) {
  struct char_data *charger=0, *vict, *v;
  int rescued = 0;

  if (cmd!=MSG_MOBACT) return FALSE;

  for(v=world[CHAR_REAL_ROOM(cavalier)].people;v;v=v->next_in_room) {
    if(IS_NPC(v) && V_MOB(v)==CHARGER)
      charger=v;
  }

  if(charger) {
    for(v=world[CHAR_REAL_ROOM(charger)].people;v;v=v->next_in_room) {
      if(IS_MORTAL(v) && v->specials.fighting == charger) {
         if(charger->specials.fighting == v)
           stop_fighting(charger);
         if(v->specials.fighting)
           stop_fighting(v);
         if (cavalier->specials.fighting)
           stop_fighting(cavalier);
         rescued = 1;
         set_fighting(cavalier, v);
         set_fighting(v, cavalier);
      }
    }
    if(rescued) {
      act("The Tarion Cavalier heroically rescues the Tarion War-Charger!",0,cavalier,0,charger,TO_NOTVICT);
      act("The Tarion Cavalier rescues you! You feel confused.",0,cavalier,0,charger,TO_VICT);
      act("You heroically rescue the Tarion War-Charger!",0,cavalier,0,charger,TO_CHAR);
      rescued = 0;
    }
  }


  if(cavalier->specials.fighting && CHAR_REAL_ROOM(cavalier->specials.fighting) == CHAR_REAL_ROOM(cavalier)) {
    switch(number(1,8)) {
      case 1:
       act("$n screams 'Death to the Infidels!'",1,cavalier,0,0,TO_ROOM);
       if(charger) {
         stop_fighting(charger);
         set_fighting(charger, cavalier->specials.fighting);
       }
       break;
      case 2:
       act("$n begins singing a war chant to lift his spirits.",1,cavalier,0,0,TO_ROOM);
       cast_layhands(GET_LEVEL(cavalier),cavalier,"",SPELL_TYPE_SPELL,cavalier,0);
       break;
      case 3:
       if(!(vict=get_random_victim_fighting(cavalier))) return FALSE;
       if(IS_NPC(vict)) return FALSE;
       if(GET_HIT(vict)>=GET_MAX_HIT(vict)*2/3) return FALSE;
       if(GET_ALIGNMENT(vict)>=0) return FALSE;
       if(GET_POS(vict)==POSITION_DEAD) return FALSE;

       act("$n streams blue holy light from his fingers at $N!",0,cavalier,0,vict,TO_NOTVICT);
       act("$n consumes you in a torrent of holy light!",0,cavalier,0,vict,TO_VICT);
       act("You pray to your Gods and stream blue holy light toward $N.",0,cavalier,0,vict,TO_CHAR);
       GET_HIT(vict)=-10;
       break;
      case 4:
       if(!(vict=get_random_victim_fighting(cavalier))) return FALSE;
       if(IS_NPC(vict)) return FALSE;
       if(GET_POS(vict)==POSITION_DEAD) return FALSE;

       act("As $n prays, a flash of light eclipses $N.",0,cavalier,0,vict,TO_NOTVICT);
       act("$n mumbles a prayer, and you are eclipsed within a flash of light.",0,cavalier,0,vict,TO_VICT);
       act("You pray to your Gods and a flash of light eclipses $N.",0,cavalier,0,vict,TO_CHAR);
       GET_MANA(vict)=number(0,100);
       break;
      case 5:
       act("$n emits a keening whistle to summon his trusty steed!",1,cavalier,0,0,TO_ROOM);
       if(charger) {
         stop_fighting(charger);
         set_fighting(charger, cavalier->specials.fighting);
       }
       break;
      case 6:
       act("$n screams 'Into the fray, my brethren!'",1,cavalier,0,0,TO_ROOM);
       if(charger) {
         stop_fighting(charger);
         set_fighting(charger, cavalier->specials.fighting);
       }
       break;
      case 7:
      break;
      case 8:
      break;
    }
  }
  return FALSE;
}


void assign_desert (void) {
  assign_room(DESERT_SAND_STORM1,sand_storm);
  assign_room(DESERT_SAND_STORM2,sand_storm);
  assign_room(DESERT_SAND_STORM3,sand_storm);
  assign_mob(CAVALIER,   cavalier);

}
