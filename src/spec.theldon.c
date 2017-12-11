/****************************************************************
 *             Theldon's Crypt Specs                            *
 *   Special procedure module for Theldon's Crypt               *
 ****************************************************************/

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
#include "spec_assign.h"

#define THELDON_ZONE    29700
#define THELDONS_AXE    ITEM(THELDON_ZONE,0)
#define THELDONS_COFFIN ITEM(THELDON_ZONE,1)
#define TOMB_ENTRANCE   ITEM(THELDON_ZONE,6)
#define THELDONS_TOMB   ITEM(THELDON_ZONE,7)
#define UNDEAD_THELDON  ITEM(THELDON_ZONE,3)

extern int find_obj(int v_num, int virt_room);

/* The Axe of Theldon */

int axe_of_theldon(OBJ *axe,CHAR *ch, int cmd, char *argument)
{
  char buf[MAX_INPUT_LENGTH];
  CHAR *vic;


  if (!ch) return (FALSE);
  if (axe != EQ(ch,WIELD)) return (FALSE);

  one_argument(argument, buf);
  switch (cmd) {

  case CMD_FLEE:
   vic=ch->specials.fighting;
      if(!vic) return FALSE;
   if(GET_ALIGNMENT(vic) < -349) {
	send_to_char("Your axe will not allow you to flee in the face of evil!\n\r",ch);
        return TRUE;
	}
	break;

  case CMD_KILL:

    if (!*buf) return FALSE;
    if (!(vic = get_char_room_vis(ch, buf))) {
      act("You raise your $p to strike!",FALSE,ch,axe,0,TO_CHAR);
      act("$n raises $s $p to strike!",FALSE,ch,axe,0,TO_ROOM);
      return TRUE;
    }

    if(GET_ALIGNMENT(vic) > 350) {
      send_to_char("Your axe refuses to attack any beings pure of heart!\n\r",ch);
      return TRUE;
    }
    break;

    case CMD_USE:
     if (!isname (buf,OBJ_NAME(axe))) return (FALSE);
     if (GET_LEVEL(ch) > 20 || OBJ_SPEC(axe)==time_info.day) {
     	    send_to_char("Nothing happens!\n\r",ch);
            return TRUE;
           }

    act("$n's $p glows with a bright light.",FALSE,ch,axe,0,TO_ROOM);
    act("Your $p glows with a bright light.",FALSE,ch,axe,0,TO_CHAR);
    spell_layhands(20,ch,ch,SPELL_TYPE_SPELL);
    OBJ_SPEC(axe)=time_info.day;
    return TRUE;
    break;
  }
return (FALSE);
}

int tomb_entrance (int room, CHAR *ch, int cmd, char *arg) {

  if(!ch)
    return(FALSE);
  if (!IS_MORTAL(ch))
    return FALSE;
  if (cmd != CMD_SOUTH)
   return FALSE;

    if (GET_LEVEL(ch) > 20) {

    act("$n is forced back by an icy gust of wind.",TRUE,ch,0,0,TO_ROOM);
    send_to_char ("An icy wind from within the crypt blows you back\n\r",ch);
    return TRUE;
 }
  	if(count_mortals_real_room(real_room(THELDONS_TOMB))>0) {
     send_to_char ("An icy wind from within the crypt blows you back.\n\r",ch);
     act("$n is forced back by an icy gust of wind.",TRUE,ch,0,0,TO_ROOM);
    return TRUE;
 }
   return FALSE;
}

int maincrypt (OBJ *coffin, CHAR *ch, int cmd, char *arg) {
  CHAR *tmp;
  CHAR *theldon;
  struct obj_data *axe;
  char buf[MAX_INPUT_LENGTH];
  one_argument (arg,buf);

 if(!count_mortals_real_room(real_room(THELDONS_TOMB))) {
  for(theldon = world[real_room(THELDONS_TOMB)].people; theldon; theldon = tmp) {
     tmp=theldon->next_in_room;
     if(IS_MOB(theldon)) {
          if(V_MOB(theldon)==UNDEAD_THELDON) {
           if (theldon->equipment[WIELD])
		extract_obj(unequip_char(theldon,WIELD));
	  extract_char(theldon);
       }
     }
   }
 }
  if (cmd != CMD_OPEN) return FALSE;
  if (!ch) return(FALSE);
  if (!AWAKE (ch)) return FALSE;

  if (!isname (buf,OBJ_NAME(coffin))) return (FALSE);

  send_to_char("When you touch the lid of the sarcophagus, the skeletal undead form of\n\rTheldon Wolfsbane the Grim bursts forth, shattering it.  His words echo\n\reerily in the small chamber:'For defiling the sanctity of my tomb, I shall\n\rslay thee, evil one!!'\n\r",ch);

  theldon = read_mobile (UNDEAD_THELDON, VIRTUAL);
  char_to_room (theldon, real_room(THELDONS_TOMB));
  axe=read_object(THELDONS_AXE, VIRTUAL);
  if (!number(0,2)) equip_char(theldon,axe,WIELD);
  extract_obj(coffin);
  set_fighting (theldon, ch);
  return (TRUE);
}

void assign_theldon (void) {
  assign_obj(THELDONS_AXE, axe_of_theldon);
  assign_room(TOMB_ENTRANCE, tomb_entrance);
  assign_obj(THELDONS_COFFIN, maincrypt);
}
