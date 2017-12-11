/*
///   Special Procedure Module                   Orig. Date 2009-06-01
///
///   spec.haondor.c --- Specials for Haon'Dor Dark
///
///   Designed and coded by Hemp of (phillips.kevin@gmail.com)
///   Using this code is not allowed without permission from originator.
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
#include "limits.h"
#include "fight.h"
#include "utility.h"
#include "cmd.h"
#include "spec_assign.h"
#include "enchant.h"

int green_beret(OBJ *beret, CHAR *ch, int cmd, char *arg)
{
  bool bReturn = FALSE;
  struct enchantment_type_5 ench;

  switch(cmd)
  {
	case MSG_BEING_REMOVED:
      if (ch != beret->equipped_by) return bReturn;
      if (!ch) return bReturn;
      if(beret==EQ(ch,WEAR_HEAD))
      {
        unequip_char(ch,WEAR_HEAD);
        beret->affected[0].modifier=3; /* Location 0 should be set as DAMROLL */
		beret->affected[1].location=0; /* Location 1 varies by class */
        beret->affected[1].modifier=0;
		beret->affected[2].location=0; /* Location 2 varies by class */
        beret->affected[2].modifier=0;
        beret->obj_flags.value[0]=2;
        equip_char(ch,beret,WEAR_HEAD);
    	OBJ_SPEC(beret)=0;
      }
	  break;
	case CMD_PRACTICE:
      /* with all the SKILL modifiers, this will prevent skill-looping with the beret */
      if (ch != beret->equipped_by) return bReturn;
      if (!ch) return bReturn;
      if(AWAKE(ch) && *arg)
      {
	    send_to_char("Try as you might, you can't seem to think about anything other than the forest.\n\r",ch);
	    bReturn = TRUE;
      }
	  break;
	case MSG_TICK:
      ch = beret->equipped_by;
      if (!ch) return bReturn;

	  if(world[CHAR_REAL_ROOM(ch)].sector_type == SECT_FOREST && chance(10))
	  {
	    act("The forest recognizes $n's kinship and bathes $m in a warm glow.",0,ch,0,0,TO_ROOM);
	    act("The forest recognizes your kinship and you are bathed in a warm glow.",0,ch,0,0,TO_CHAR);
	    act("$n's skin turns green, and $e seems to bend toward the sun.",0,ch,0,0,TO_ROOM);
	    act("Your skin turns green, and you feel an affinity for the shining sun.",0,ch,0,0,TO_CHAR);
	    memset(&ench,0,sizeof(struct enchantment_type_5));
	    ench.name     = str_dup("Regeneration");
	    enchantment_to_char(ch,&ench,TRUE);
	    free(ench.name);
	  }
	  if(OBJ_SPEC(beret)==GET_CLASS(ch)) break;
	  unequip_char(ch,WEAR_HEAD);
	  switch(GET_CLASS(ch))
	  {
	    case CLASS_CLERIC:
	    case CLASS_ANTI_PALADIN:
	    case CLASS_PALADIN:
	    case CLASS_COMMANDO:
	      beret->affected[0].modifier=1;
	      beret->affected[1].location=0;
	      beret->affected[1].modifier=0;
	      beret->affected[2].location=0;
	      beret->affected[2].modifier=0;
	      beret->obj_flags.value[0]=0;
	      act("The leaves in $n's beret wilt and fall to the ground.",0,ch,0,0,TO_ROOM);
	      act("The leaves in your beret wilt and fall to the ground.",0,ch,0,0,TO_CHAR);
	      break;
	    case CLASS_AVATAR:
	      beret->affected[0].modifier=0;
	      beret->affected[1].location=APPLY_HIT;
	      beret->affected[1].modifier=-10000;
	      beret->affected[2].location=0;
	      beret->affected[2].modifier=0;
	      beret->obj_flags.value[0]=0;
	      act("The flora comprising $n's beret turns black with a virulent mold.",0,ch,0,0,TO_ROOM);
	      act("The flora comprising your beret turns black with a virulent mold.",0,ch,0,0,TO_CHAR);
	      break;
	    case CLASS_MAGIC_USER:
	      beret->affected[0].modifier=2;
	      beret->affected[1].location=APPLY_MANA;
	      beret->affected[1].modifier=GET_LEVEL(ch)/2;
	      beret->affected[2].location=APPLY_INT;
	      beret->affected[2].modifier=1;
	      beret->obj_flags.value[0]=2;
	      act("A small group of tiny faeries alight upon $n's beret.",0,ch,0,0,TO_ROOM);
	      act("A small group of tiny faeries alight upon your beret.",0,ch,0,0,TO_CHAR);
	      break;
	    case CLASS_WARRIOR:
	      beret->affected[0].modifier=2;
	      beret->affected[1].location=APPLY_HIT;
	      beret->affected[1].modifier=GET_LEVEL(ch)/2;
	      beret->affected[2].location=0;
	      beret->affected[2].modifier=0;
	      beret->obj_flags.value[0]=13;
	      act("The wood in $n's beret petrifies, becoming stronger than metal.",0,ch,0,0,TO_ROOM);
	      act("The wood in your beret petrifies, becoming stronger than metal.",0,ch,0,0,TO_CHAR);
	      break;
	    case CLASS_NOMAD:
	      beret->affected[0].modifier=3;
	      beret->affected[1].location=APPLY_HP_REGEN;
	      beret->affected[1].modifier=GET_LEVEL(ch)/10;
	      beret->affected[2].location=APPLY_SKILL_AMBUSH;
	      beret->affected[2].modifier=GET_LEVEL(ch)/7;
	      beret->obj_flags.value[0]=4;
	      act("$n's beret suddenly sprouts additional vegetation.",0,ch,0,0,TO_ROOM);
	      act("Your beret suddenly sprouts additional vegetation.",0,ch,0,0,TO_CHAR);
	      break;
	    case CLASS_NINJA:
	      beret->affected[0].modifier=3;
	      beret->affected[1].location=APPLY_HITROLL;
	      beret->affected[1].modifier=2;
	      beret->affected[2].location=APPLY_SKILL_PUMMEL;
	      beret->affected[2].modifier=GET_LEVEL(ch)/7;
	      beret->obj_flags.value[0]=3;
	      act("The colors in $n's beret seem to shift to match $s surroundings.",0,ch,0,0,TO_ROOM);
	      act("The colors in your beret seem to shift to match your surroundings.",0,ch,0,0,TO_CHAR);
	      break;
	    case CLASS_THIEF:
	      beret->affected[0].modifier=3;
	      beret->affected[1].location=APPLY_SKILL_BACKSTAB;
	      beret->affected[1].modifier=GET_LEVEL(ch)/6;
	      beret->affected[2].location=APPLY_DEX;
	      beret->affected[2].modifier=1;
	      beret->obj_flags.value[0]=6;
	      act("A drooping fern sprouts from atop $n's beret, concealing $s face.",0,ch,0,0,TO_ROOM);
	      act("A drooping fern sprouts from atop your beret, concealing your face.",0,ch,0,0,TO_CHAR);
	      break;
	    case CLASS_BARD:
	      beret->affected[0].modifier=3;
	      beret->affected[1].location=APPLY_HITROLL;
	      beret->affected[1].modifier=1;
	      beret->affected[2].location=APPLY_SKILL_BACKFLIP;
	      beret->affected[2].modifier=GET_LEVEL(ch)/6;
	      beret->obj_flags.value[0]=2;
	      act("Wild flowers sprout from $n's beret, attracting hummingbirds.",0,ch,0,0,TO_ROOM);
	      act("Wild flowers sprout from your beret, attracting hummingbirds.",0,ch,0,0,TO_CHAR);
	      break;
      }/* end switch(GET_CLASS(ch)) */
      equip_char(ch,beret,WEAR_HEAD);
	  OBJ_SPEC(beret)=GET_CLASS(ch);
      break;
    default:
      break;
  }/* end switch(cmd) */
  return bReturn;
}/* end green_beret() */

/* ************************ End of Haon'Dor ******************** */

#define GREEN_BERET		6116

void assign_haondor (void)
{
  assign_obj(GREEN_BERET, green_beret);
}
