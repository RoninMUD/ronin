/*
** Special Procedure Module                    Orig. Date     2009/12/26
**                                             Last Modified  2015/05/20
**
** spec.questgearII.c --- specials for Quest Gear II (10)
**
** Authors: Shun
** Written for RoninMUD
**
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "utility.h"
#include "cmd.h"
#include "fight.h"
#include "spec_assign.h"
#include "interpreter.h"
#include "act.h"
#include "shop.h"

/*
** Defines
*/

/* Objects */

#define QGII_SHABTIS      1020
#define QGII_COWL         1028
#define QGII_PHYLACTERY   1029
#define QGII_BULKATHOS    1033
#define QGII_VIZARD       1034
#define QGII_VILYA        1039
#define QGII_NARYA        1040
#define QGII_ENLIL        1061
#define QGII_ANU          1062
#define QGII_VINDICTAE    1092

/*
** Spec Functions
*/

/* Objects */

#define TEMPLE_OF_MIDGAARD                3001

#define SHABTIS_SWNU_CHARGE_TIME          1344 /*2 MUD weeks*/
#define SHABTIS_SWNU_TIME_PER_CONDITION    336 /* SHABTIS_SWNU_CHARGE_TIME / SHABTIS_CONDITIONS_MAX_SIZE */
#define SHABTIS_NERU_CHARGE_TIME            60 /*15 MUD hours*/
#define SHABTIS_NERU_TIME_PER_CONDITION     15 /* SHABTIS_NERU_CHARGE_TIME / SHABTIS_CONDITIONS_MAX_SIZE */
#define SHABTIS_KANI_CHARGE_TIME          2688 /*4 MUD weeks*/
#define SHABTIS_KANI_TIME_PER_CONDITION    672 /* SHABTIS_KANI_CHARGE_TIME / SHABTIS_CONDITIONS_MAX_SIZE */
#define SHABTIS_WAW_CHARGE_TIME            672 /*1 MUD week*/
#define SHABTIS_WAW_TIME_PER_CONDITION     168 /* SHABTIS_WAW_CHARGE_TIME / SHABTIS_CONDITIONS_MAX_SIZE */

const int shabtis_charge_nums[][2] =
{
  {SHABTIS_SWNU_CHARGE_TIME, SHABTIS_SWNU_TIME_PER_CONDITION},
  {SHABTIS_NERU_CHARGE_TIME, SHABTIS_NERU_TIME_PER_CONDITION},
  {SHABTIS_KANI_CHARGE_TIME, SHABTIS_KANI_TIME_PER_CONDITION},
  {SHABTIS_WAW_CHARGE_TIME, SHABTIS_WAW_TIME_PER_CONDITION}
};

#define SHABTIS_CONDITIONS_MAX_SIZE        4

const char* shabtis_condition_msgs[] =
{
  "fractured",
  "cracked",
  "chipped",
  "scuffed",
  "pristine"
};

const char* shabtis_statue_bag_material[][2] =
{/* nifty string array for recharge messages */
  {"Swnu","silk"},    /* obj_flags.value[0] */
  {"Neru","wool"},    /* obj_flags.value[1] */
  {"Kani","leather"}, /* obj_flags.value[2] */
  {"Waw","linen"}     /* obj_flags.value[3] */
};

int qgII_shabtis(OBJ *shabtis, CHAR *owner, int cmd, char *arg)
{
  CHAR *vict, *next_vict;
  char buf[MIL];
  char info_str[MSL];
  int outTime = 0;
  int bReturn = FALSE;
  int i = 0;
  int old_condition = 0;
  int current_condition = 0;

  switch(cmd)
  {
    case MSG_TICK: /* update time until charged */
      owner = shabtis->equipped_by;
      if(!owner)
        owner = shabtis->carried_by;
      if(owner && !IS_NPC(owner))
      {
        /* cycle through 4 obj_flags.value[i] which store the time-to-recharge for each statue */
        for(i = 0; i < 4; i++)
        { /* i: 0,Swnu; 1,Neru; 2,Kani; 3,Waw */
          if(shabtis->obj_flags.value[i] <= 0)
            shabtis->obj_flags.value[i] = 0;
          else
          { /* calculate what condition the statue is currently in: time_to_charge - current_value / time_per_condition */
            old_condition = (shabtis_charge_nums[i][0] - shabtis->obj_flags.value[i]) / shabtis_charge_nums[i][1];
            /* update time_to_charge */
            shabtis->obj_flags.value[i]--;
            /* calculate what condition the statue is currently in: time_to_charge - current_value / time_per_condition */
            current_condition = (shabtis_charge_nums[i][0] - shabtis->obj_flags.value[i]) / shabtis_charge_nums[i][1];
            /* if current_condition != old_condition send an informational recharge message */
            if(current_condition != old_condition)
            {
              sprintf(info_str, "Diminutive fragments of the now %s statue of %s slide deliberately back into place.\n\r", shabtis_condition_msgs[current_condition], shabtis_statue_bag_material[i][0]);
              send_to_char(info_str, owner);
              for(vict = world[CHAR_REAL_ROOM(owner)].people; vict; vict = next_vict)
              {
                next_vict = vict->next_in_room;
                if(vict == owner)
                  continue;
                if(!CAN_SEE(vict, owner))
                  send_to_char("Somewhere nearby you hear the faint clicking of stone on stone.\n\r", vict);
                else
                {
                  sprintf(info_str, "A faint clicking is heard from the %s bag around %s's waist as the statue of %s reassembles.\n\r", shabtis_statue_bag_material[i][1], PERS(owner,owner), shabtis_statue_bag_material[i][0]);
                  send_to_char(info_str, vict);
                }
              }
            }
          }
        }
      }
      break;
    case MSG_OBJ_ENTERING_GAME:
      if(!is_number(arg))
        break;
      outTime = atoi(arg); /* outTime = time in seconds since last in-game (time out of game) */
      outTime /= 60;       /* outTime = time in minutes since last in-game */
      for(i = 0; i < 4; i++) /* update time_to_charge, value[x] = 0 is charged */
        shabtis->obj_flags.value[i] = MAX(shabtis->obj_flags.value[i] - outTime, 0);
       break;
    case CMD_EXAMINE:
      one_argument(arg, buf);
      if(!IS_NPC(owner) && AWAKE(owner) && (owner==shabtis->equipped_by || owner==shabtis->carried_by) && V_OBJ(shabtis)==QGII_SHABTIS && shabtis && owner)
      {
        if(!strncmp(buf, "swnu", MIL) || !strncmp(buf, "Swnu", MIL))
        { i = 0; /* 0:Swnu */
          /* calculate what condition the statue is currently in: time_to_charge - current_value / time_per_condition */
          current_condition = (shabtis_charge_nums[i][0] - shabtis->obj_flags.value[i]) / shabtis_charge_nums[i][1];
          sprintf(info_str, "The shabti of Swnu the Doctor looks %s.\n\r", shabtis_condition_msgs[current_condition]);
          send_to_char(info_str, owner);
        }
        else if(!strncmp(buf, "neru", MIL) || !strncmp(buf, "Neru", MIL))
        { i = 1; /* 1:Neru */
          /* calculate what condition the statue is currently in: time_to_charge - current_value / time_per_condition */
          current_condition = (shabtis_charge_nums[i][0] - shabtis->obj_flags.value[i]) / shabtis_charge_nums[i][1];
          sprintf(info_str, "The shabti of Neru the Shepherd looks %s.\n\r", shabtis_condition_msgs[current_condition]);
          send_to_char(info_str, owner);
        }
        else if(!strncmp(buf, "kani", MIL) || !strncmp(buf, "Kani", MIL))
        { i = 2; /* 2:Kani */
          /* calculate what condition the statue is currently in: time_to_charge - current_value / time_per_condition */
          current_condition = (shabtis_charge_nums[i][0] - shabtis->obj_flags.value[i]) / shabtis_charge_nums[i][1];
          sprintf(info_str, "The shabti of Kani the Gardener looks %s.\n\r", shabtis_condition_msgs[current_condition]);
          send_to_char(info_str, owner);
        }
        else if(!strncmp(buf, "waw", MIL) || !strncmp(buf, "Waw", MIL))
        { i = 3; /* 3:Waw */
          /* calculate what condition the statue is currently in: time_to_charge - current_value / time_per_condition */
          current_condition = (shabtis_charge_nums[i][0] - shabtis->obj_flags.value[i]) / shabtis_charge_nums[i][1];
          sprintf(info_str, "The shabti of Waw the Soldier looks %s.\n\r", shabtis_condition_msgs[current_condition]);
          send_to_char(info_str, owner);
        }
        else if(!strncmp(buf, "shabti", MIL) || !strncmp(buf, "shabtis", MIL))
        { /* "hacked" extra description */
          sprintf(info_str, " History details the exploits of ancient pharaohs, their inclinations towards ritual\n\r\
magicks and experiments involving infusion and extraction of life-forces from a\n\r\
range of beings: human, animal and otherwise. While these deeds were not inherently\n\r\
evil by nature, they did demonstrate a bias toward the darker side of arcane magic,\n\r\
and as a result, the lore and myths surrounding certain ancient curses were born.\n\r\
The results of a certain pharaoh's experimentation with this taboo magic resulted\n\r\
in his inevitable overthrow and subsequent mummification, just retribution for the\n\r\
crimes committed against his own people.\n\r\
\n\r\
 While historical records about this ill-fated pharaoh are scattered and incomplete,\n\r\
there is one particular set of experiments mentioned by all. The story tells of a\n\r\
potent set of artifacts borne of dark ritualistic magic: four shabti made of stone\n\r\
and clay, statues molded into the shape of four key symbols of arcane power. The\n\r\
ancient names of these aspects were Waw, Kani, Swnu and Neru, and into the bodies\n\r\
of each statue he poured a wellspring of forbidden magic.\n\r\
\n\r\
 In order to transport these statues, the pharaoh fashioned a belt from fine linen-\n\r\
wrapped leopard skin, embedding into the length of the belt four small hooks, as\n\r\
might have been used to catch and carry fish. Strangely, the hooks are set into the\n\r\
belt directed inward, intended to be lodged into the skin of the wearer, although\n\r\
for what sinister purpose is anyone's guess. Attached to the blunt end of each hook\n\r\
is a thick drawstring made from hemp, one for each of four distinct pouches meant\n\r\
to hold the shabtis. These pouches are fashioned from a variety of material: wool,\n\r\
leather, linen and silk. Each fabric is stained by a variety of unsettling sources:\n\r\
one appears to be blood, another is some sort of viscous green liquid, the next is\n\r\
bleached completely white and the last is coated in a thin dusting of desert sand.\n\r");
            send_to_char(info_str, owner);
        }
        else
          break; /* skip to normal examine procedure */
        bReturn = TRUE;
      }
      break;
    case CMD_USE:
      if(!IS_NPC(owner) && owner && AWAKE(owner) && (owner==shabtis->equipped_by || owner==shabtis->carried_by) && shabtis==EQ(owner,WEAR_WAIST) && V_OBJ(shabtis)==QGII_SHABTIS)
      {
        one_argument(arg, buf);
        if(!strncmp(buf, "swnu", MIL) || !strncmp(buf, "Swnu", MIL))
        { i = 0; /* 0:Swnu - "Doctor" great mira */
          if(shabtis->obj_flags.value[i] == 0)
          { /* if charged */
            sprintf(info_str, "Using both hands, you shake the shabti of Swnu violently; an eerie whine emanates\n\r\
from the statue's now gaping mouth followed by an eruption of fetid blood pouring\n\r\
out towards the ground. In mid-air, the fountain of blood appears to both boil and\n\r\
then burst into thin wisps of crimson gas spiraling upwards and arcing suddenly\n\r\
towards you, filling your lungs as you deeply breath it in. The blood mist seems\n\r\
to suffuse you as your wounds instantly seal themselves shut.\n\r");
            send_to_char(info_str, owner);
            /* NOTE: this sends the message to mobs/gods as well but I don't care */
            sprintf(info_str, "You look over to see %s holding a strange stone statue in %s hands, eyes\n\r\
glazed over as %s mumbles in a forgotten language. An unexpected series of spasms\n\r\
wracks %s body as %s falls to %s knees, a fountain of blood erupting from %s\n\r\
gaping maw. Before it hits the ground, the blood bursts into an enormous cloud of\n\r\
crimson gas, suffusing your body as your wounds seal shut.\n\r", PERS(owner,owner), HSHR(owner), HSSH(owner), HSHR(owner), HSSH(owner), HSHR(owner), HSHR(owner));
            send_to_room_except(info_str, CHAR_REAL_ROOM(owner), owner);
            for(vict = world[CHAR_REAL_ROOM(owner)].people; vict; vict = next_vict)
            {
              next_vict = vict->next_in_room;
              if(IS_NPC(vict) || !IS_MORTAL(vict))
                continue; /* no affect on mobs/gods */
              magic_heal(vict, SPELL_MIRACLE, 2000, FALSE);
            }
            /* reset the charge counter */
            shabtis->obj_flags.value[i] = shabtis_charge_nums[i][0];
            WAIT_STATE(owner, PULSE_VIOLENCE);
          }
          else /* if not charged */
          { /* calculate what condition the statue is currently in: time_to_charge - current_value / time_per_condition */
            current_condition = (shabtis_charge_nums[i][0] - shabtis->obj_flags.value[i]) / shabtis_charge_nums[i][1];
            sprintf(info_str, "You grab the %s shabti of Swnu in both hands and shake it violently...\n\r\
.....But nothing happens.\n\r", shabtis_condition_msgs[current_condition]);
            send_to_char(info_str, owner);
          }
        }

        else if(!strncmp(buf, "neru", MIL) || !strncmp(buf, "Neru", MIL))
        { i = 1; /* 1:Neru - "Shepherd" total recall */
          if(shabtis->obj_flags.value[i] == 0)
          { /* if charged */
            sprintf(info_str, "Twisting the base of the shabti of Neru, you hear the subtle grinding of stone on\n\r\
stone, stopping with a faint click. The statue begins to tremble in your hands, a\n\r\
single mote of light streaming out of a miniscule hole in the face of the shepherd.\n\r\
Suddenly long cracks split the shabti's surface as a blinding cascade of radiance\n\r\
pours forth, overwhelming your senses. As the light fades, you find yourself in a\n\r\
familiar place, bits of the broken statue left in your hands.\n\n\r");
            send_to_char(info_str, owner);
            /* NOTE: this sends the message to mobs/gods as well but I don't care */
            sprintf(info_str, "A faint murmur can be heard from %s and %s abruptly begins to\n\r\
tremble as though subject to a chilling wind. A strange light seems to peek out\n\r\
from %s mouth, dim at first but intensifying before suddenly erupting from both\n\r\
eyes and consuming your entire vision with a brilliant radiance. As the light\n\r\
diminishes, you see %s kneeling on the ground, panting, and your surroundings\n\r\
changed to a more familiar place.\n\r", PERS(owner,owner), HSSH(owner), HSHR(owner), HMHR(owner));
            send_to_room_except(info_str, CHAR_REAL_ROOM(owner), owner);
            for(vict = world[CHAR_REAL_ROOM(owner)].people; vict; vict = next_vict)
            {
              next_vict = vict->next_in_room;
              if(IS_NPC(vict) && !vict->master && !IS_AFFECTED(vict, AFF_CHARM) && !IS_MOUNT(vict))
                continue; /* skip mobs that aren't pets/charmies/mounts */
              char_from_room(vict);
              char_to_room(vict, real_room(TEMPLE_OF_MIDGAARD));
              sprintf(info_str, "%s appears in the middle of the room.\n\r", PERS(vict,vict));
              send_to_room_except(info_str, real_room(TEMPLE_OF_MIDGAARD), vict);
              do_look(vict, "", CMD_LOOK);
            }
            /* reset the charge counter */
            shabtis->obj_flags.value[i] = shabtis_charge_nums[i][0];
            WAIT_STATE(owner, PULSE_VIOLENCE);
          }
          else /*if not charged */
          { /* calculate what condition the statue is currently in: time_to_charge - current_value / time_per_condition */
            current_condition = (shabtis_charge_nums[i][0] - shabtis->obj_flags.value[i]) / shabtis_charge_nums[i][1];
            sprintf(info_str, "You grab the %s shabti of Neru and twist its base, the stone grinding subtly...\n\r\
.....But nothing happens.\n\r", shabtis_condition_msgs[current_condition]);
            send_to_char(info_str, owner);
          }
        }

        else if(!strncmp(buf, "kani", MIL) || !strncmp(buf, "Kani", MIL))
        { i = 2; /* 2:Kani - "Gardener" great mana */
          if(shabtis->obj_flags.value[i] == 0)
          { /* if charged */
            sprintf(info_str, "Holding the shabti of Kani in the palm of one hand, you press a thumb into the\n\r\
face of the statue causing the surface to crack and spill open. Pouring out from\n\r\
within is an emerald mist, a trickle at first until it becomes a torrent, burying\n\r\
your surroundings under a thick green veil. As the haze clears, you find yourself\n\r\
inside of a beautiful, lush garden, infusing you with tremendous arcane energy\n\r\
before the landscape vanishes as quickly as it appeared.\n\r");
            send_to_char(info_str, owner);
            /* NOTE: this sends the message to mobs/gods as well but I don't care */
            sprintf(info_str, "The sound of fracturing stone and tearing skin fills your ears and you turn to\n\r\
see %s pressing a broken statue into %s chest, a gaping crater\n\r\
forming that oozes with a thick, emerald mist. The green haze billows down to\n\r\
form a growing cloud that completely veils your surroundings, transforming into\n\r\
a lush garden that shimmers with arcane energy as you feel your magical powers\n\r\
restored.\n\r", PERS(owner,owner), HSHR(owner));
            send_to_room_except(info_str, CHAR_REAL_ROOM(owner), owner);
            for(vict = world[CHAR_REAL_ROOM(owner)].people; vict; vict = next_vict)
            {
              next_vict = vict->next_in_room;
              if(IS_NPC(vict) || !IS_MORTAL(vict))
                continue; /* skip mobs/gods */
              GET_MANA(vict) = GET_MAX_MANA(vict);
            }
            /* reset the charge counter */
            shabtis->obj_flags.value[i] = shabtis_charge_nums[i][0];
            WAIT_STATE(owner, PULSE_VIOLENCE);
          }
          else /* if not charged */
          { /* calculate what condition the statue is currently in: time_to_charge - current_value / time_per_condition */
            current_condition = (shabtis_charge_nums[i][0] - shabtis->obj_flags.value[i]) / shabtis_charge_nums[i][1];
            sprintf(info_str, "You grab the %s shabti of Kani and press a thumb into the face of the statue...\n\r\
.....But nothing happens.\n\r", shabtis_condition_msgs[current_condition]);
            send_to_char(info_str, owner);
          }
        }

        else if(!strncmp(buf, "waw", MIL) || !strncmp(buf, "Waw", MIL))
        { i = 3; /* i:Waw - "Soldier" 6000dmg to current target */
          if(shabtis->obj_flags.value[i] == 0)
          {
            vict = owner->specials.fighting;
            if(vict && IS_NPC(vict))
            {
              sprintf(info_str, "Gripping the shabti of Waw in one hand, you tap a series of symbols on its face,\n\r\
it pulses briefly then explodes into a blinding cloud of desert sand. When the\n\r\
sandstorm clears, a forgotten battalion of dune cavaliers stands encircling your\n\r\
foe, lances lowered. With a rash burst of speed they charge forward, impaling\n\r\
%s simultaneously before vanishing suddenly.\n\r\
At their disappearance the statue fractures in your hands.\n\r", GET_SHORT(vict));
              send_to_char(info_str, owner);
              sprintf(info_str, "An ear-piercing howl splits the air as you spin in place to see %s\n\r\
kneeling with both arms outstretched, a stone statue held in one hand as a stream\n\r\
of desert sand erupts from every pore of %s skin. Rapidly swirling into a blinding\n\r\
tempest, it swiftly fades, leaving a battalion of dune cavaliers in a circle around\n\r\
%s, who quickly charge, impaling %s and vanishing in one fell swoop.\n\r", PERS(owner,owner), HSHR(owner), GET_SHORT(vict), HMHR(vict));
              send_to_room_except_two(info_str, CHAR_REAL_ROOM(owner), owner, vict);
              send_to_char("Wawwwwwwwwwwwwwwwwwwwwwwwwwwwwwww!\n\r", vict);
              damage(owner, vict, 6000, TYPE_UNDEFINED, DAM_PHYSICAL);
              /* reset the charge counter */
              shabtis->obj_flags.value[i] = shabtis_charge_nums[i][0];
              WAIT_STATE(owner, PULSE_VIOLENCE);
            }
            else if(vict == NULL || (vict && !IS_NPC(vict)))
            {
              sprintf(info_str, "You grip the shabti of Waw and tap at a series of symbols on its face...\n\r\
.....The statue shakes momentarily with fury, but nothing more.\n\r");
              send_to_char(info_str, owner);
            }
          }
          else
          { /* calculate what condition the statue is currently in: time_to_charge - current_value / time_per_condition */
            current_condition = (shabtis_charge_nums[i][0] - shabtis->obj_flags.value[i]) / shabtis_charge_nums[i][1];
            sprintf(info_str, "You grab the %s shabti of Waw and tap a series of symbols on its face...\n\r\
.....But nothing happens.\n\r", shabtis_condition_msgs[current_condition]);
            send_to_char(info_str, owner);
          }
        }
        else /* if use isn't one of the shabti figures, skip to normal use procedure */
          break;
        bReturn = TRUE;
      }
      break;
    default:
      break;
  }
  return bReturn;
}


char* cowl_color[] =
{/* indicates harvest charges */
  "ghostly white",
  "sea-foam gray",
  "charcoal grey",
  "midnight black"
};

void change_cowl_color( OBJ *cowl )
{
  CHAR *owner = cowl->equipped_by;
  if(!owner)
    owner = cowl->carried_by;

  char buf[100];
  sprintf(buf, "%s cowl of the Shadowmage", cowl_color[cowl->spec_value]);
  cowl->short_description = str_dup(buf);
}

#define COWL_CHARGE_TIME  24 /* 6 MUD hours */
#define COWL_MAX_CHARGES  3

int qgII_cowl(OBJ *cowl, CHAR *owner, int cmd, char *arg)
{
  CHAR *vict, *next_vict;
  char buf[MIL];
  char info_str[MSL];
  int bReturn = FALSE;
  int outTime = 0, mana = 0;

  switch(cmd)
  {
    case MSG_TICK:
      owner = cowl->equipped_by;
      if(!owner)
        owner = cowl->carried_by;
      if(owner && !IS_NPC(owner))
      {
        if(cowl->spec_value > COWL_MAX_CHARGES) /* spec_value is # of "harvest" charges */
        {
          cowl->spec_value = COWL_MAX_CHARGES; /* max of COWL_MAX_CHARGES "harvest" charges */
        }
        else if(cowl->spec_value < COWL_MAX_CHARGES)
        {
          cowl->obj_flags.timer--; /* timer is time left until next recharge */
          if(cowl->obj_flags.timer <= 0)
          {
            cowl->spec_value++; /* add a "harvest" charge */
            change_cowl_color(cowl);
            sprintf(info_str, "The Shadow exerts a sudden force against your will, momentarily overwhelming your\n\r\
psychic defenses; it strains against the magical bindings woven in the %s\n\r\
fabric of your cowl before you once more reassert control over the darkness within.\n\r", cowl_color[cowl->spec_value]);
            send_to_char(info_str, owner);
            for(vict = world[CHAR_REAL_ROOM(owner)].people; vict; vict = next_vict)
            {
              next_vict = vict->next_in_room;
              if(vict == owner)
                continue;
              if(!CAN_SEE(vict, owner))
                continue;/* no message if you can't see the wearer */
              else
              {
                sprintf(info_str, "A dark tendril lashes out wildly from %s's cowl and %s appears\n\r\
physically strained before the shadowy limb is absorbed once more into the now\n\r\
%s fabric.\n\r", PERS(owner,owner), HSSH(owner), cowl_color[cowl->spec_value]);
                send_to_char(info_str, vict);
              }
            }
            if(cowl->spec_value < COWL_MAX_CHARGES)
              cowl->obj_flags.timer = COWL_CHARGE_TIME; /* set time until next recharge */
          }
        }
      }
      break;
    case MSG_OBJ_ENTERING_GAME:
      if(!is_number(arg))
        break;
       if(cowl->spec_value == COWL_MAX_CHARGES)
       {
        change_cowl_color(cowl);
         break; /* if cowl charges are maxed, skip */
      }
       outTime = atoi(arg); /* outTime = time in seconds since last in-game (time out of game) */
      outTime /= 60;       /* outTime = time in minutes since last in-game */
      cowl->obj_flags.timer -= outTime; /* update time until next recharge based on time since last in-game */
      while (cowl->obj_flags.timer <= 0 && cowl->spec_value < COWL_MAX_CHARGES)
      { /* while time until next charge is less than 1, keep adding a charge and updating time to next charge - stop at COWL_MAX_CHARGES (full) charges */
        cowl->spec_value++; /* add a charge if time since last in-game is greater than time to charge */
        if(cowl->spec_value < COWL_MAX_CHARGES)
          cowl->obj_flags.timer += COWL_CHARGE_TIME;
        else
          cowl->obj_flags.timer = 0;
      }
      change_cowl_color(cowl);
       break;
     case CMD_EXAMINE:
      one_argument(arg, buf);
      if(AWAKE(owner) && (owner==cowl->equipped_by || owner==cowl->carried_by) && V_OBJ(cowl)==QGII_COWL)
      {
        if(!strncmp(buf, "cowl", MIL))
        {
          sprintf(info_str, " While every magical foci has roots in the light and the dark, few exist as close to\n\r\
the darkness as Shadow magic. Throughout history the masters of this magical aspect,\n\r\
known as Shadowmages, have been universally distrusted, and with good reason. Many\n\r\
a culture has a story of their own struggle against a rogue Shadowmage, thirsty for\n\r\
control and power, aided by an army of darkness.\n\r\
 One such learned Shadowmage had conquered the darkness so thoroughly that she had\n\r\
managed to capture the root of Shadow magic's power, a race of dark beings from the\n\r\
spirit realm, in enchanted items of her own design. When she finally fell, an entire\n\r\
kingdom lay in ruin, and all that remained at the site of her last stand was a cowl.\n\r\
 The few wizened sages who survived the Shadowmage's campaign sensed the presence of\n\r\
darkness within the cowl. Something dark, aware, and powerful struggled against the\n\r\
enchanted fabric, eager to escape. Fearing that the destruction of the cowl itself\n\r\
would free whatever magic was contained within, the sages left the kingdom to bury\n\r\
the cowl where no one would ever find it and were never heard from again.\n\r\
 Unearthed, returned from exile, the cowl's power has been unleashed upon the world\n\r\
once more, imbuing its wearer with the darkness once tamed by the Shadowmage.\n\r");
          send_to_char(info_str, owner);
        }
      }
      break;
    case CMD_UNKNOWN:
      if(owner==cowl->equipped_by && owner && !IS_NPC(owner))
      {
        arg = one_argument(arg, buf);
        if(AWAKE(owner) && cowl==EQ(owner,WEAR_HEAD) && V_OBJ(cowl)==QGII_COWL && !strncmp(buf, "harvest", MIL))
        {
          one_argument(arg, buf);
          vict = get_char_room_vis(owner, buf);
          if(vict)
          {
            if(IS_NPC(vict)) /* "harvest" mob */
            {
              if(cowl->spec_value == COWL_MAX_CHARGES) /* if fully charged, deal 500 dmg and mira "harvester" */
              {
                sprintf(info_str, "You release the minor effort of will that was holding the Shadow magic within the\n\r\
cowl at bay; darkness instantly pours forth from the cowl's %s fabric.\n\r\
A shadowy, sickle-wielding silhouette takes shape as the darkness coalesces, light\n\r\
around it is snuffed out. Suddenly it surges forward engulfing %s,\n\r\
harvesting %s lifeforce, before hastening toward you and pouring into your body its\n\r\
stolen energies. As you shudder, recovering from the experience, you notice, out\n\r\
of the corner of one eye, the fabric of the cowl has drained to a %s.\n\r", cowl_color[cowl->spec_value], GET_SHORT(vict), HSHR(vict), cowl_color[0]);
                send_to_char(info_str, owner);
                send_to_char("Shadow chomp! NOM NOM NOM!", vict);
                sprintf(info_str, "Without warning a thick billowing form pours forth from %s's cowl.\n\r\
A shadowy, sickle-wielding silhouette surges forward, engulfing %s,\n\r\
harvesting %s lifeforce, before merging with %s. Though seemingly\n\r\
restored, %s body is wracked by a brief spasm and as %s recovers you notice the\n\r\
previously %s cowl has drained to a %s.\n\r", PERS(owner,owner), GET_SHORT(vict), HSHR(vict), PERS(owner,owner), HSHR(owner), HSSH(owner), cowl_color[cowl->spec_value], cowl_color[0]);
                send_to_room_except_two(info_str, CHAR_REAL_ROOM(owner), owner, vict);
                damage(owner, vict, 500, TYPE_UNDEFINED, DAM_NO_BLOCK);
                GET_HIT(owner) = GET_MAX_HIT(owner);
                cowl->spec_value = 0; /* remove all "harvest" charges */
                change_cowl_color(cowl);
                cowl->obj_flags.timer = COWL_CHARGE_TIME; /* start recharge timer */
                WAIT_STATE(owner, PULSE_VIOLENCE);
              }
              else
              {
                sprintf(info_str, "You attempt to call upon the power of the Shadow magic in your cowl...\n\r\
.....But nothing happens.\n\r");
                send_to_char(info_str, owner);
              }
            }
            else if(IS_MORTAL(vict)) /* "harvest" other player */
            {
              if(cowl->spec_value > 0)
              {
                sprintf(info_str, "You extend an effort of will toward the Shadow magic harnessed in your cowl, your\n\r\
thoughts focused intently upon %s, and darkness oozes from the cowl's\n\r\
%s fabric. A shadowy, urchin-like form coalesces, then spirals through\n\r\
%s chest in a blur of motion before returning to you with the harvested magical\n\r\
energies. The shadowy urchin disappears and you notice the force exerted upon your\n\r\
will by the captured Shadow magic fade slightly.\n\r", PERS(vict,vict), cowl_color[cowl->spec_value], HSHR(vict));
                send_to_char(info_str, owner);
                sprintf(info_str, "A shiver running up your spine is the only warning before a shadowy, urchin-like\n\r\
form spirals into your chest in a blur of motion from %s's direction.\n\r\
You feel as if your magical energies have been drained, harvested. The spiny orb\n\r\
rushes back toward %s and fuses with %s body, as it disappears from\n\r\
view you notice %s cowl's fabric fade from %s to a %s.\n\r",  PERS(owner,owner), PERS(owner,owner), HSHR(owner), HSHR(owner), cowl_color[cowl->spec_value], cowl_color[cowl->spec_value - 1]);
                send_to_char(info_str, vict);
                sprintf(buf, "%s", PERS(vict,vict));
                sprintf(info_str, "A hint of peripheral motion draws your attention toward %s where a spiny\n\r\
orb of shadow has just finished spiraling into and through %s chest. In one fluid\n\r\
motion it rushes toward %s, doomed to a similar fate you're certain,\n\r\
before fusing with %s body unceremoniously. A grin stretches across %s face and\n\r\
you notice %s cowl's fabric fade from %s to a %s.\n\r",  buf, HSHR(vict), PERS(owner,owner), HSHR(owner), HSHR(owner), HSHR(owner), cowl_color[cowl->spec_value], cowl_color[cowl->spec_value - 1]);
                send_to_room_except_two(info_str, CHAR_REAL_ROOM(owner), owner, vict);
                mana = MIN(200, GET_MANA(vict));
                GET_MANA(vict) -= mana;
                GET_MANA(owner) += mana;
                if(cowl->spec_value == COWL_MAX_CHARGES)
                  cowl->obj_flags.timer = COWL_CHARGE_TIME; /* start recharge timer - if charges were less than COWL_MAX_CHARGES, should already have a running timer */
                cowl->spec_value--; /* remove a "harvest" charge */
                change_cowl_color(cowl);
                WAIT_STATE(owner, PULSE_VIOLENCE);
              }
              else /* give message if "harvest" without charges */
              {
                sprintf(info_str, "You attempt to call upon the power of the Shadow magic in your cowl...\n\r\
.....But nothing happens.\n\r");
                send_to_char(info_str, owner);
              }
            }
            else /* if "harvest" god */
              break;
            bReturn = TRUE;
          }
        }
      }
      break;
    default:
      break;
  }
  return bReturn;
}

#define PHYLACTERY_CHARGE_TIME       48 /* 12 MUD hours */
#define PHYLACTERY_MAX_CHARGES      3

int qgII_phylactery(OBJ *phylactery, CHAR *owner, int cmd, char *arg)
{
  CHAR *vict, *next_vict;
  char buf[MIL];
  int bReturn = FALSE;
  int outTime = 0;
  struct descriptor_data *i;

  switch(cmd)
  {
    case MSG_TICK:
      owner = phylactery->equipped_by;
      if(!owner)
        owner = phylactery->carried_by;
      if(owner && !IS_NPC(owner))
      {
        if(phylactery->spec_value > PHYLACTERY_MAX_CHARGES) /* spec_value is # of "wail" charges */
        {
          phylactery->spec_value = PHYLACTERY_MAX_CHARGES; /* max of PHYLACTERY_MAX_CHARGES "wail" charges */
        }
        else if(phylactery->spec_value < PHYLACTERY_MAX_CHARGES)
        {
          phylactery->obj_flags.timer--; /* timer is time left until next recharge */
          if(phylactery->obj_flags.timer <= 0)
          {
            act("A shrill wail boils to the surface of the phylactery, seeping out through the gleaming gem.", FALSE, owner, NULL, NULL, TO_CHAR);
            act("A faint wail seeps out from within the phylactery around $n's waist.", FALSE, owner, NULL, NULL, TO_ROOM);
            phylactery->spec_value++; /* add a "wail" charge */
            if(phylactery->spec_value < PHYLACTERY_MAX_CHARGES)
            {
              phylactery->obj_flags.timer = PHYLACTERY_CHARGE_TIME; /* set time until next recharge */
            }
          }
        }
      }
      break;
    case MSG_OBJ_ENTERING_GAME:
      if((owner==phylactery->equipped_by || owner==phylactery->carried_by) && owner && !IS_NPC(owner))
      {
        if(phylactery->spec_value == PHYLACTERY_MAX_CHARGES)
          break; /* if phylactery charges are maxed, skip */
        if(is_number(arg))
          outTime = atoi(arg); /* outTime = time in seconds since last in-game */
        else
          break;
        outTime /= 60; /* time in minutes since last in-game */
        phylactery->obj_flags.timer -= outTime; /* update time until next recharge based on time since last in-game */
        while (phylactery->obj_flags.timer <= 0 && phylactery->spec_value < PHYLACTERY_MAX_CHARGES)
        { /* while time until next charge is less than 1, keep adding a charge and updating time to next charge - stop at PHYLACTERY_MAX_CHARGES (full) charges */
          phylactery->spec_value++; /* add a charge if time since last in-game is greater than time to charge */
          if(phylactery->spec_value < PHYLACTERY_MAX_CHARGES)
            phylactery->obj_flags.timer += PHYLACTERY_CHARGE_TIME;
          else
            phylactery->obj_flags.timer = 0;
        }
      }
      break;
    case CMD_EXAMINE:
      if((owner==phylactery->equipped_by || owner==phylactery->carried_by) && owner && !IS_NPC(owner))
      {
        one_argument(arg, buf);
        if(AWAKE(owner) && phylactery && V_OBJ(phylactery)==QGII_PHYLACTERY && !strncmp(buf, "phylactery", MIL))
        {
          if(phylactery->spec_value <= 0)
            send_to_char("The faint wailing that once issued from the phylactery has been silenced.\n\r\n\r", owner);
          else if(phylactery->spec_value == 1)
            send_to_char("A faint, shrill wail echoes from within the phylactery.\n\r", owner);
          else if(phylactery->spec_value == 2)
            send_to_char("A duet of keening wails issue forth from the phylactery.\n\r", owner);
          else if(phylactery->spec_value >= 3)
            send_to_char("A cacophony of keening wails erupts from the phylactery.\n\r", owner);
          bReturn = TRUE;
        }
      }
      break;
    case CMD_UNKNOWN:
      if((owner==phylactery->equipped_by || owner==phylactery->carried_by) && owner && !IS_NPC(owner))
      {
        arg = one_argument(arg, buf);
        if(AWAKE(owner) && phylactery==EQ(owner,WEAR_WAIST) && V_OBJ(phylactery)==QGII_PHYLACTERY && !strncmp(buf, "wail", MIL))
        {
          if(phylactery->spec_value > 0)
          {
            act("$n surges with puissance of the Bean-Sidhe and expels a keening wail!", FALSE, owner, NULL, NULL, TO_ROOM);
            act("The spirit of the Bean-Sidhe erupts from within as you expel a keening wail!", FALSE, owner, NULL, NULL, TO_CHAR);
            for(i = descriptor_list; i; i = i->next)
            { /* display wail message to all characters in zone */
              if(i->character != owner && !i->connected && (world[CHAR_REAL_ROOM(i->character)].zone == world[CHAR_REAL_ROOM(owner)].zone))
                act("A shrieking wail echoes across the land.", FALSE, owner, NULL, i->character, TO_VICT);
            }
            for(vict = world[CHAR_REAL_ROOM(owner)].people; vict; vict = next_vict)
            {
              next_vict = vict->next_in_room;
              if(!IS_NPC(vict) && GET_LEVEL(vict)>=LEVEL_IMM)
                continue; /* skip gods */
              if(vict->master==owner || vict->specials.rider==owner)
                continue; /* skip wailers charmies/pets/mount */
              if(!ROOM_CHAOTIC(CHAR_REAL_ROOM(owner)) && (IS_MOUNT(vict) || !IS_NPC(vict) || vict->master || IS_AFFECTED(vict, AFF_CHARM)))
                continue; /* skip mounts/PCs/pets/charmies in non-chaos rooms/mode */
              stop_fighting(vict); /* stop the vict fighting so damage() will make the wailer tank against that vict */
              damage(owner, vict, 333, TYPE_UNDEFINED, DAM_NO_BLOCK);
            }
            if(phylactery->spec_value == PHYLACTERY_MAX_CHARGES)
              phylactery->obj_flags.timer = PHYLACTERY_CHARGE_TIME; /* start recharge timer - if charges were less than PHYLACTERY_MAX_CHARGES, should already have a running timer */
            phylactery->spec_value--; /* remove a "wail" charge */
            WAIT_STATE(owner, PULSE_VIOLENCE);
          }
          else
          {
            act("$n opens $s mouth wide but utters only a quiet whimper.", FALSE, owner, NULL, NULL, TO_ROOM);
            act("You attempt a keening ululation but produce only a soft whimper.", FALSE, owner, NULL, NULL, TO_CHAR);
          }
          bReturn = TRUE;
        }
      }
      break;
    default:
      break;
  }
  return bReturn;
}

int qgII_bulkathos(OBJ *bulkathos, CHAR *owner, int cmd, char *arg)
{
  int bReturn = FALSE;

  if(owner==bulkathos->equipped_by && owner)
  {
    if(cmd==CMD_PRACTICE && AWAKE(owner) && *arg)
    {
      send_to_char("A nostalgic voice you have heard many a time says, `h\"I can't do that..... yet.\"\n\r", owner);
      bReturn = TRUE;
    }
  }
  return bReturn;
}

int qgII_vizard(OBJ *vizard, CHAR *owner, int cmd, char *arg)
{
  int bReturn = FALSE;

  if(owner==vizard->equipped_by && owner)
  {
    if(cmd==CMD_PRACTICE && AWAKE(owner) && *arg)
    {
      send_to_char("You aren't able to remember who you are, much less what you wanted to practice.\n\r", owner);
      bReturn = TRUE;
    }
  }
  return bReturn;
}

/*
** Unified spec for quest rings.  Each ring has its own
** vnum and keywords so they may be used  while equipped.
*/

typedef enum {
  RING_STR_KEYWORD = 0,
  RING_STR_CHARGE_TOCHAR,
  RING_STR_CHARGE_TOROOM,
  RING_STR_EXA_EMPTY,
  RING_STR_EXA_ONE,
  RING_STR_EXA_TWO,
  RING_STR_EXA_FULL,
  RING_STR_TWIST_TOCHAR,
  RING_STR_TWIST_TOROOM,
  RING_STR_TWIST_NOCHARGE,
  RING_STR_HEAL_TOCHAR,
  RING_STR_HEAL_TOROOM,
  RING_STR_MIRA_TOCHAR,
  RING_STR_MIRA_TOROOM,
  RING_STR_HASTE_TOCHAR,
  RING_STR_HASTE_TOROOM,
  RING_STR_FURY_TOCHAR,
  RING_STR_FURY_TOROOM,
  RING_STR_COUNT
} ring_string_id;

typedef enum {
  RING_TYPE_VILYA = 0,
  RING_TYPE_NARYA,
  RING_TYPE_ENLIL,
  RING_TYPE_ANU,
  RING_TYPE_COUNT
} ring_type;

static const char const *ring_string_table[RING_TYPE_COUNT][RING_STR_COUNT] = {
  { /* Vilya */
    "vilya",
    "A rush of air passes over you, and Vilya's sapphire glows a deeper shade of blue.",
    "$n's ring draws in the winds, and it glows a deeper shade of blue.",
    "Vilya's sapphire is faded to almost transparent clearness.",
    "Vilya's sapphire is the lightest of periwinkles.",
    "Vilya's sapphire is a rich cerulean.",
    "Vilya's sapphire is a deep midnight hue.",
    "You twist Vilya on your finger and a mighty wind rushes about you.",
    "$n twists Vilya on $s finger and a mighty wind rushes all about $m.",
    "You twist Vilya on your finger and a light breeze rustles your hair.",
    "The wind blows over you calmly, and you feel at peace.",
    "The wind blows over $n, and $e seems somehwat soothed.",
    "The wind blows over you gently, the smell of sweet nectar and freshly cut grass soothing your soul.",
    "$n seems soothed by the breeze that passes over $m.",
    "You feel yourself borne up by the force of the gale, and feel a spring in your step.",
    "The wind lifts $n slightly off the ground, and $e seems sprightlier.",
    "You feel the wind blowing through you, a hurricane building within.",
    "The wind seems to enter $n, and a hurricane builds behind $s eyes."
  },
  { /* Narya */
    "narya",
    "You feel a surge of warmth as Narya's ruby glows a deeper shade of red.",
    "Some of the heat seems to leave your body as Narya's ruby glows a deeper shade of red.",
    "Narya's ruby is faded to almost transparent clearness.",
    "Narya's ruby is the lightest of dusty roses.",
    "Narya's ruby is a rich vermillion.",
    "Narya's ruby is a deep carmine hue.",
    "You twist Narya on your finger and a searing heat radiates out from you.",
    "$n twists Narya on $s finger and a searing heat pulses out from $m.",
    "You twist Narya on your finger and feel a soft throbbing of warmth.",
    "The warmth of Narya comforts you, and you feel healed.",
    "A warm smile crosses $n's face.",
    "A warmth suffuses your body, and you feel healed.",
    "A content smile crosses $n's face.",
    "You feel your blood pumping faster in your veins, giving you a burst of energy.",
    "$n flushes with the pulse of blood, and $s movements seem to speed.",
    "You feel your blood burn with dragon's fire, and a veil of red falls over the world.",
    "$n flushes a deep red, heat pulsing out from $s body."
  },
  { /* Enlil */
    "enlil",
    "The ground beneath your feet rumbles as Enlil's topaz flashes a bright orange.",
    "$n almost loses his footing as Enlil's topaz flashes a bright orange.",
    "Enlil's topaz reveals the image of a desolate wasteland.",
    "Enlil's topaz reveals the images of an idyllic rolling hill.",
    "Enlil's topaz reveals the image of a mighty mountain.",
    "Enlil's topaz reveals the image of a roaring, erupting volcano.",
    "You twist Enlil on your finger and a violent shaking emanates from you.",
    "$n twists Enlil on $s finger and a violent tremor nearly knocks $m over.",
    "You twist Enlil on your finger and a minor rumbling knocks you slightly off balance.",
    "A warm, restoring rain falls gently on your face.",
    "A warm, restoring rain falls gently on $n's face.",
    "A geyser suddenly erupts beneath your feet, bathing you in healing steam.",
    "A geyser erupts beneath $n's feet, bathing $m in healing steam.",
    "A powerful earthquake shakes you to your core, filling you with supernatural speed.",
    "A powerful earthquake violently shakes $n, moving $m with intense speed.",
    "Hot jets of magma erupt from every pore on your body, setting your soul on fire.",
    "Glowing rock pours from $n, turning $m into a volcanic beast of fury and rage."
  },
  { /* Anu */
    "anu",
    "A beam of light warms your face, as Anu's diamond flashes a brilliant blue.",
    "$n is bathed in sunlight as Anu's diamond flashes a brilliant blue.",
    "Anu's diamond reveals the image of a dark, overcast rainy sky.",
    "Anu's diamond reveals the image of a peaceful sunrise.",
    "Anu's diamond reveals the image of the bright, hot sun.",
    "Anu's diamond reveals the image of an omninous, total solar eclipse.",
    "You twist Anu on your finger and a powerful gust of wind emanates from you.",
    "$n twists Anu on $s finger and hurricane force winds nearly knock $m over.",
    "You twist Anu on your finger and a pleasant breeze hits your face.",
    "A sunbeam passes over your face, filling you with a pleasant warmth.",
    "A pleasant sunbeam passes over $n's face.",
    "The clouds part above your head and a ray of divine light bathes you in a healing glow.",
    "The clouds part above $n's head and a ray of divine light bathes $m in a healing glow.",
    "A strong wind envelops you, pushing you forward and speeding your actions.",
    "A strong wind envelops $n, pushing $m foward and speeding $s actions.",
    "An intense bright light erupts from your chest, filling you with the rage of a thousands suns.",
    "$n explodes into a supernova of intense light, filling $m with unimaginable rage."
  }
};

static const char const *ring_string(int vnum, ring_string_id id) {
  ring_type type = RING_TYPE_COUNT;
  char const *result = NULL;

  switch (vnum) {
    case QGII_VILYA:
      type = RING_TYPE_VILYA;
      break;

    case QGII_NARYA:
      type = RING_TYPE_NARYA;
      break;

    case QGII_ENLIL:
      type = RING_TYPE_ENLIL;
      break;

    case QGII_ANU:
      type = RING_TYPE_ANU;
      break;
  }

  if (type != RING_TYPE_COUNT) {
    result = ring_string_table[type][id];
  }

  return result;
}

static void ring_act(int vnum, ring_string_id id, CHAR *target, int type) {
  char const *str = NULL;
  char buf[MAX_STRING_LENGTH+1];

  memset(buf, 0, sizeof(buf));

  if (!(str = ring_string(vnum, id))) return;

  strncpy(buf, str, MAX_STRING_LENGTH);
  act(buf, FALSE, target, NULL, NULL, type);
}

int qgII_ring(OBJ *ring, CHAR *owner, int cmd, char *arg) {

#ifdef TEST_SITE
  const int RING_CHARGE_TIME = 1;
#else
  const int RING_CHARGE_TIME = 96; /* 24 MUD hours */
#endif
  const int RING_MAX_CHARGES = 3;

  CHAR *vict = NULL, *next_vict = NULL;
  char buf[MIL];
  int bReturn = FALSE;
  int outTime = 0;
  int num = 0;
  int vnum = V_OBJ(ring);

  switch(cmd) {
    case MSG_TICK:
      owner = ring->equipped_by ? ring->equipped_by : ring->carried_by;

      if (owner && !IS_NPC(owner)) {

        if (ring->spec_value > RING_MAX_CHARGES) { /* spec_value is # of "twist" charges */
          ring->spec_value = RING_MAX_CHARGES; /* max of RING_MAX_SPECS "twist" charges */
        }
        else if (ring->spec_value < RING_MAX_CHARGES) {
          ring->obj_flags.timer--; /* timer is time left until next recharge */

          if (ring->obj_flags.timer <= 0)
          {
            ring_act(vnum, RING_STR_CHARGE_TOCHAR, owner, TO_CHAR);
            ring_act(vnum, RING_STR_CHARGE_TOROOM, owner, TO_ROOM);
            ring->spec_value++; /* add a "twist" charge */

            if (ring->spec_value < RING_MAX_CHARGES) {
              ring->obj_flags.timer = RING_CHARGE_TIME; /* set time until next recharge */
            }
          }
        }
      }
      break;

    case MSG_OBJ_ENTERING_GAME:
      if ((owner==ring->equipped_by || owner==ring->carried_by) && owner && !IS_NPC(owner)) {
        if (ring->spec_value == RING_MAX_CHARGES) {
          break; /* if ring charges are maxed, skip */
        }

        if(is_number(arg)) {
          outTime = atoi(arg); /* outTime = time in seconds since last in-game */
        }
        else {
          break;
        }

        outTime /= 60; /* time in minutes since last in-game */
        ring->obj_flags.timer -= outTime; /* update time until next recharge based on time since last in-game */

        while (ring->obj_flags.timer <= 0 && ring->spec_value < RING_MAX_CHARGES) {
          /* while time until next charge is less than 1, keep adding a charge and updating time to next charge - stop at RING_MAX_CHARGES (full) charges */
          ring->spec_value++; /* add a charge if time since last in-game is greater than time to charge */
          if (ring->spec_value < RING_MAX_CHARGES) {
            ring->obj_flags.timer += RING_CHARGE_TIME;
          }
          else {
            ring->obj_flags.timer = 0;
          }
        }
      }
      break;

    case CMD_EXAMINE:
      if ((owner==ring->equipped_by || owner==ring->carried_by) && owner && !IS_NPC(owner)) {
        one_argument(arg, buf);

        if (AWAKE(owner) && ring && !strncmp(buf, ring_string(vnum, RING_STR_KEYWORD), MIL)) {
          if (ring->spec_value <= 0) {
            ring_act(vnum, RING_STR_EXA_EMPTY, owner, TO_CHAR);
          }
          else if(ring->spec_value == 1) {
            ring_act(vnum, RING_STR_EXA_ONE, owner, TO_CHAR);
          }
          else if(ring->spec_value == 2) {
            ring_act(vnum, RING_STR_EXA_TWO, owner, TO_CHAR);
          }
          else if(ring->spec_value >= 3) {
            ring_act(vnum, RING_STR_EXA_FULL, owner, TO_CHAR);
          }

          bReturn = TRUE;
        }
      }
      break;

    case CMD_UNKNOWN:
      if ((owner==ring->equipped_by || owner==ring->carried_by) && owner && !IS_NPC(owner)) {
        arg = one_argument(arg, buf);

        if (AWAKE(owner) && (ring==EQ(owner, WEAR_FINGER_L) || ring==EQ(owner, WEAR_FINGER_R)) && !strncmp(buf, "twist", MIL)) {
          one_argument(arg, buf);

          if (!strncmp(buf, ring_string(vnum, RING_STR_KEYWORD), MIL)) { /* command must be: "twist <ring keyword>" */

            if(ring->spec_value > 0) {
              ring_act(vnum, RING_STR_TWIST_TOCHAR, owner, TO_CHAR);
              ring_act(vnum, RING_STR_TWIST_TOROOM, owner, TO_ROOM);

              for(vict = world[CHAR_REAL_ROOM(owner)].people; vict; vict = next_vict) {
                next_vict = vict->next_in_room;
                if(IS_NPC(vict)) continue; /* skip mobs */
#ifndef TEST_SITE
                if(!IS_MORTAL(vict)) continue; /* skip gods */
#endif
                num = number(1,4);

                switch(num) {
                  case 1: /* triple heal */
                    ring_act(vnum, RING_STR_HEAL_TOCHAR, vict, TO_CHAR);
                    ring_act(vnum, RING_STR_HEAL_TOROOM, vict, TO_ROOM);
                    spell_heal(50, vict, vict, 0);
                    spell_heal(50, vict, vict, 0);
                    spell_heal(50, vict, vict, 0);
                    break;
                  case 2: /* miracle */
                    ring_act(vnum, RING_STR_MIRA_TOCHAR, vict, TO_CHAR);
                    ring_act(vnum, RING_STR_MIRA_TOROOM, vict, TO_ROOM);
                    spell_miracle(50, vict, vict, 0);
                    break;
                  case 3: /* haste */
                    ring_act(vnum, RING_STR_HASTE_TOCHAR, vict, TO_CHAR);
                    ring_act(vnum, RING_STR_HASTE_TOROOM, vict, TO_ROOM);
                    spell_haste(50, vict, vict, 0);
                    break;
                  case 4: /* fury */
                    ring_act(vnum, RING_STR_FURY_TOCHAR, vict, TO_CHAR);
                    ring_act(vnum, RING_STR_FURY_TOROOM, vict, TO_ROOM);
                    spell_fury(50, vict, vict, 0);
                    break;
                  default:
                    break;
                }
              }

              if(ring->spec_value == RING_MAX_CHARGES) {
                ring->obj_flags.timer = RING_CHARGE_TIME; /* start recharge timer - if charges were less than RING_MAX_CHARGES, should already have a running timer */
              }

              ring->spec_value--; /* remove a "twist" charge */
              WAIT_STATE(owner, PULSE_VIOLENCE);
            }
            else {
              ring_act(vnum, RING_STR_TWIST_NOCHARGE, owner, TO_CHAR);
            }

            bReturn = TRUE;
          }
        }
      }
      break;

    default:
      break;
  }
  return bReturn;
}

#define VIND_MULTIPLIER_HIT     1.0
#define VIND_MULTIPLIER_MANA    2.0
#define VIND_MULTIPLIER_MOVE    0.1
#define VIND_INTERVAL_CHARGE    6
#define VIND_MODULUS_SPEC       10
#define VIND_DAMROLL_BASE       14
#define VIND_DAMROLL_NIGHT      19

int qgII_vindictae(OBJ *vindictae, CHAR *ch, int cmd, char *arg)
{
  CHAR *wielder = vindictae->equipped_by;

  switch(cmd)
  {
    case MSG_TICK:

      /*
      ** Duplicate SoN night damage increase.
      */

      if (wielder)
      {
        if (IS_NIGHT)
        {
          if (vindictae->affected[0].modifier>VIND_DAMROLL_BASE) break;  /* if already improved, do nothing */
          unequip_char(wielder,WIELD);
          vindictae->affected[0].modifier=VIND_DAMROLL_NIGHT;
          equip_char(wielder,vindictae,WIELD);
          save_char(wielder,NOWHERE);
        }
        else
        {
          if (vindictae->affected[0].modifier<VIND_DAMROLL_NIGHT) break;  /* if already changed to normal, do nothing */
          unequip_char(wielder,WIELD);
          vindictae->affected[0].modifier=VIND_DAMROLL_BASE;
          equip_char(wielder,vindictae,WIELD);
          save_char(wielder,NOWHERE);
        }
      }
      break;

    case MSG_MOBACT:

      /*
      ** Charge or release blood power
      */

      break;

    case MSG_BEING_REMOVED:

     /*
     ** Punish on remove, release blood power
     */

     if (ch != wielder) break;

     break;



  }

  return FALSE;
}

/*
** Assign Function
*/

void assign_questgearII(void)
{
  assign_obj(QGII_SHABTIS,      qgII_shabtis);
  assign_obj(QGII_COWL,         qgII_cowl);
  assign_obj(QGII_PHYLACTERY,   qgII_phylactery);
  assign_obj(QGII_VIZARD,       qgII_vizard);
  assign_obj(QGII_BULKATHOS,    qgII_bulkathos);
  assign_obj(QGII_VILYA,        qgII_ring);
  assign_obj(QGII_NARYA,        qgII_ring);
  assign_obj(QGII_ENLIL,        qgII_ring);
  assign_obj(QGII_ANU,          qgII_ring);
  assign_obj(QGII_VINDICTAE,    qgII_vindictae);
}

