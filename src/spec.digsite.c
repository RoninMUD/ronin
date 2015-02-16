/*   spec.digsite.c - Specs for DigsitebyAlexander

     Written by Hemp for RoninMUD
     Last Modification Date: August 27, 2009

*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

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
#include "enchant.h"

/* Zone */
#define DIGSITE				185
/* Rooms */
#define ARCHAEOLOGIST_ROOM	18542
/* Mobs */
#define ARCHAEOLOGIST		18504
/* Objects */
#define BUSH				18511
#define BUSH_BEGIN			18515
#define BUSH_END			18533

/* damage per mobact that escalates, enchantment removed when outside specific rooms */
int toxic_fumes_ench(ENCH *ench, CHAR *ench_ch, CHAR *char_in_room, int cmd, char *arg)\
{
  int imult;

  if(cmd==MSG_REMOVE_ENCH) {
    act("$n gulps in huge lungfuls of fresh, clean air.",1,ench_ch,0,0,TO_ROOM);
    act("You gulp in huge lungfuls of fresh, clean air.",1,ench_ch,0,0,TO_CHAR);
    return FALSE;
  }

  if(cmd==MSG_MOBACT)
  {
	if(world[CHAR_REAL_ROOM(ench_ch)].number==ARCHAEOLOGIST_ROOM)
	{
	  imult = ench->modifier;
	  imult++;
	  act("$n chokes as $e tries to take a breath and begins coughing uncontrollably.",1,ench_ch,0,0,TO_ROOM);
	  act("You choke as you try to take a breath and begin coughing uncontrollably.",1,ench_ch,0,0,TO_CHAR);
	  damage(ench_ch, ench_ch, imult*60, TYPE_UNDEFINED, DAM_NO_BLOCK);
	  ench->duration = -1;
	  ench->modifier = imult;
	} else {
	  toxic_fumes_ench(ench, ench_ch, char_in_room, MSG_REMOVE_ENCH, NULL);
      enchantment_remove(ench_ch, ench, 1);
    }
  }
  return FALSE;
}

/* set toxic_fumes enchantment when entering */
int dig_archaeologist_room(int room, CHAR *ch, int cmd, char *arg)
{
  char buf[MIL];
  ENCH *tmp_enchantment;

  if(cmd==MSG_ENTER && V_ROOM(ch)==ARCHAEOLOGIST_ROOM && IS_MORTAL(ch) && !IS_NPC(ch) && !enchanted_by(ch,"Toxic Fumes"))
  {
    CREATE(tmp_enchantment, ENCH, 1);
    tmp_enchantment->name     = str_dup("Toxic Fumes");
    tmp_enchantment->duration = -1;
    tmp_enchantment->func     = toxic_fumes_ench;
    enchantment_to_char(ch, tmp_enchantment, FALSE);
    act("$n gags as the toxic fumes begin to suffocate $m!",0,ch,0,0,TO_ROOM);
    act("\n\rYou gag as the toxic fumes begin to suffocate you!",0,ch,0,0,TO_CHAR);
    sprintf(buf,"Digsite Log Ench: [ %s is breathing TOXIC_FUMES at %d ]",GET_NAME(ch),world[CHAR_REAL_ROOM(ch)].number);
    log_f(buf);
  }
  return FALSE;
}

/* heal to full if no mortals in room */
int dig_archaeologist(CHAR *arch, CHAR *ch, int cmd, char *arg)
{
  if(cmd==MSG_MOBACT && (GET_HIT(arch) < GET_MAX_HIT(arch)) && (count_mortals_room(arch,TRUE)==0))
  {
    GET_HIT(arch) = GET_MAX_HIT(arch);
  }
  return FALSE;
}

/*   spec.bush.c - Specs for <Bush>

     Written by <Sephiroth> for RoninMUD
     Last Modification Date:   16.11.2010

     values:
	 0 - % chance of harvest
	 1 - obj to create on successful harvest
	 2 - timer for harvest
*/
int spec_bush(OBJ *bush,CHAR *ch,int cmd,char *argument)
{
    int harvest_roll, chance;
    char arg[MAX_STRING_LENGTH];
    OBJ* obj;

    if(cmd==MSG_TICK && bush->spec_value > 0)             /*Count down the harvest delay */
    {
        bush->spec_value--;
        return(FALSE);  
    }
 
    if(cmd==CMD_UNKNOWN && ch)                                  /*Harvest bush trigger */
    {
        argument = one_argument (argument, arg);
        if(!*arg) return FALSE;                      /*Check that a command is entered */
        if(!is_abbrev(arg,"harvest")) return FALSE;  /*Check that "harvest" CMD is entered */
        if(!*arg)                                         /*Check that there is entered word after harvest */
        {
              send_to_char("Harvest what?\n\r",ch);
              return TRUE;
        }
        argument = one_argument(argument, arg);
        string_to_lower(arg);
        if(isname(arg, OBJ_NAME(bush)))                           /*Check that second argument is "bush" */
        {
            if (bush->spec_value==0)                      /*Check that bush is harvestable */
            {
				chance = bush->obj_flags.value[0];
				harvest_roll = number(0, 100);
				if (GET_CLASS(ch) == CLASS_NOMAD)
				{
					chance = 1.1 * chance; /* 10% bonus for nomads - pot smoking hippies that they are */
				}
                if(harvest_roll <= chance)                        /* 1/3 chance of happening */
                {
                    act("You successfully harvest a leaf without damaging it.",0,ch,0,0,TO_CHAR);
                    act("$n successfully harvests a leaf without damaging it.",0,ch,0,0,TO_ROOM);
                    obj = read_object(bush->obj_flags.value[1], VIRTUAL);     /*Loading herb */
                    if(obj)
                    {
                        obj_to_char(obj,ch);              /*Giving herb to char */
                    }
                    bush->spec_value = bush->obj_flags.value[2];      /*Reset harvest delay timer */
                    return TRUE;
                }
                else                                      /*If non-successful harvest */
                {
                    act("You destroy the leaf while trying to harvest it.",0,ch,0,0,TO_CHAR);
                    act("$n destroys the leaf while trying to harvest it.",0,ch,0,0,TO_ROOM);
                    bush->spec_value = bush->obj_flags.value[2];      /*Reset harvest delay timer */
                    return TRUE;
                }  
            } /* if (bush->spec_value==0) */
            else                                          /*If delay timer is not 0 */
            {
                act("The bush is not ready to be harvested.",0,ch,0,0,TO_CHAR);
                return TRUE;
            } /* else (if (bush->spec_value==0) */

        } /*if(!strcmp(arg,"bush"))    */
         
    } /* if(cmd==CMD_UNKNOWN) */

	return FALSE;
} /* (int spec_bush(CHAR *mob,CHAR *ch,int cmd,char *argument)) */

/* assignments */
void assign_digsite(void)
{
  int i;
  /* Rooms */
  assign_room(ARCHAEOLOGIST_ROOM,	dig_archaeologist_room);
  /* Mobs */
  assign_mob(ARCHAEOLOGIST,			dig_archaeologist);
  /* Objects */
  assign_obj(BUSH, spec_bush);
  for (i = BUSH_BEGIN; i <= BUSH_END; ++i)
  {
    assign_obj(i, spec_bush);
  }
}
