
/*
$Author: ronin $
$Date: 2005/01/21 14:55:27 $
$Header: /home/ronin/cvs/ronin/enchant.c,v 2.3 2005/01/21 14:55:27 ronin Exp $
$Id: enchant.c,v 2.3 2005/01/21 14:55:27 ronin Exp $
$Name:  $
$Log: enchant.c,v $
Revision 2.3  2005/01/21 14:55:27  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.2  2004/03/02 20:54:08  ronin
Fix to prevent loss of rank in a CHAOTIC room.  Bug introducted when
trying to fix rank loss at near death.

Revision 2.1  2004/02/25 19:40:16  ronin
Change of MSG_DIE to MSG_DEAD to prevent near deaths from causing
rank loss.

Revision 2.0.0.1  2004/02/05 16:09:11  ronin
Reinitialization of cvs archives

Revision 1.4  2002/05/29 04:53:24  ronin
Activation of enchantments for hell.

Revision 1.3  2002/05/09 20:15:04  ronin
Added hell enchantments for saving through rent.

Revision 1.2  2002/03/31 07:42:14  ronin
Addition of header lines.

$State: Exp $
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "utility.h"
#include "cmd.h"
#include "act.h"
#include "enchant.h"
#include "spells.h"
#include "fight.h"

struct enchantment_type_5 *enchantments;
void command_interpreter (CHAR *ch, char *arg);
void update_pos( struct char_data *victim );


extern struct char_data *character_list;
extern struct time_info_data time_info;
extern Room *world;
extern int hit_limit(struct char_data *ch);
extern int mana_limit(struct char_data *ch);
extern int move_limit(struct char_data *ch);
extern int hit_gain(struct char_data *ch);
extern int mana_gain(struct char_data *ch);
extern int move_gain(struct char_data *ch);
/*

In ranks at MSG_TICK added ench->duration=-1.
Its my belief the duration of rank enchantments
is being changed somehow and this results in a lost
rank.

Ranger Aug 96 */

/* 2nd and 3rd Ranks added July 1998 - Ranger */

/****************************************************************************
*  Enchantments are basically the standard diku 'affect_type' with added    *
*  functions to make them a little more powerful.   This 'power' adds a bit *
*  of complexity to their handling.   Some tips and traps to look out for:  *
*  o  All affects and enchantments get removed at death (except those with  *
*     a duration of -1 (never ending)                                       *
*  o  An enchantment gets EVERY cmd/signal that a player/mob would get.     *
*  o  In general, every message needs a return (FALSE) so that the true     *
*     intent of the message/command will be carried out.                    *
*     Return (TRUE) if you WANT to stop the command from happening.         *
*  o  when you return TRUE, if there's any other items that intercept the   *
*     command, and dont return TRUE will still happen.  For example, you    *
*     COULD, if your heart desires, create an item to "stop time" (i.e.     *
*     return TRUE on MSG_TICK) but there's probably a LOT of items ahead of *
*     it that get signalled, so it'll only partially stop time.             *
*****************************************************************************/

int remortv2_enchantment(ENCH *ench, CHAR *enchanted_ch, CHAR *char_in_room, int cmd, char *arg)
{
  return FALSE;
}

int imm_grace_enchantment(ENCH *ench, CHAR *enchanted_ch, CHAR *char_in_room, int cmd, char *arg)
{
  return FALSE;
}

int cold_enchantment(struct enchantment_type_5 *ench, CHAR *enchanted_ch, CHAR *char_in_room,int cmd, char*arg)
{
  char *command;
  if (cmd!=CMD_CAST && cmd!=CMD_RECITE) return(FALSE);
  if(!number(0,5))
    {
    command = str_dup("cough");
    command_interpreter(enchanted_ch,command);
    free(command);
    }
  return(FALSE);
}
int regeneration_enchantment(struct enchantment_type_5 *ench, CHAR *enchanted_ch, CHAR *char_in_room,int cmd, char*arg)
{
  int HealingPoints;
  OBJ *flower;

  if(cmd==MSG_REMOVE_ENCH)
    {
    act("$n's skin loses its green tint.",TRUE, enchanted_ch,NULL,NULL,TO_ROOM);
    send_to_char("Your skin returns to its normal color.\n\r",enchanted_ch);
    return(FALSE);
    }

  if(cmd==MSG_TICK)
    {
    if(IS_DAY && !IS_SET(world[CHAR_REAL_ROOM(enchanted_ch)].room_flags,INDOORS))
      {
      HealingPoints = 14 - abs(14-time_info.hours);
      if(HealingPoints + GET_HIT(enchanted_ch) > hit_limit(enchanted_ch))
        {
        GET_HIT(enchanted_ch) = hit_limit(enchanted_ch);
        }
      else
        GET_HIT(enchanted_ch) += HealingPoints;
      update_pos(enchanted_ch);
      if(!number(0,20))
        {
        act("$n sprouts a rose, and it drops to the ground.",FALSE, enchanted_ch,NULL,NULL,TO_ROOM);
        send_to_char("A rose suddenly grows from your nose and falls to the ground.\n\r",enchanted_ch);
        flower = read_object(1,REAL);
        obj_to_room(flower,CHAR_REAL_ROOM(enchanted_ch));
        }
      }
    }
  return(FALSE);
}

int firebreath_enchantment(struct enchantment_type_5 *ench, CHAR *enchanted_ch, CHAR *char_in_room,int cmd, char*arg)
{
  CHAR *victim;
  char name[100];
  int lvl;

  if(cmd==MSG_SHOW_AFFECT_TEXT)
    {
    act("......$n's eyes glow an unearthly red light!",FALSE, enchanted_ch,0,char_in_room,TO_VICT);
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
    {
    act("$n's belly quits rumbling and $s eyes stop glowing red.",FALSE, enchanted_ch,NULL,NULL,TO_ROOM);
    send_to_char("The fire in your belly subsides.\n\r",enchanted_ch);
    return(FALSE);
    }

/* added by Sane 2-7-00
  if(cmd==MSG_TICK)
  {
    act("$n's belly rumbles loudly, and smoke escapes out of $s ears.",TRUE, enchanted_ch,NULL,NULL,TO_ROOM);
    send_to_char("Your belly rumbles loudly, and smoke escapes out of your ears.\n\r",enchanted_ch);
    ench->duration--;
    if(ench->duration<0)
    {
       firebreath_enchantment(ench,enchanted_ch,char_in_room,MSG_REMOVE_ENCH,NULL);
       enchantment_remove(enchanted_ch,ench,0);
    }
    return(FALSE);
  }


  if(cmd == MSG_MOBACT)
   {
      if((victim=enchanted_ch->specials.fighting))
      {
         if(number(1,100) <= (20+GET_LEVEL(enchanted_ch)-GET_LEVEL(victim)))
         {
            act("$n's stomach rumbles and a tremendous belch of fire erupts.",TRUE, enchanted_ch,NULL,victim,TO_ROOM);
            act("Your stomach rumbles and a tremendous belch of fire erupts.",TRUE, enchanted_ch,NULL,victim,TO_CHAR);
            act("$n's fiery belch envelopes $N, igniting $M into a very animated torch.",TRUE, enchanted_ch,NULL,victim,TO_NOTVICT);
            act("Your fiery belch envelopes $N, igniting $M into a very animated torch.",TRUE, enchanted_ch,NULL,victim,TO_CHAR);
            act("You are enveloped by $n's fiery belch and are ignited into a very animated torch.",TRUE, enchanted_ch,NULL,victim,TO_VICT);
            damage(enchanted_ch,victim,number(GET_LEVEL(enchanted_ch)*2,GET_LEVEL(enchanted_ch)*5),TYPE_UNDEFINED,DAM_FIRE);
         }
      }
      else
      {
         if(number(1,10)==1)
         {
            act("$n gets a pained look in $s face as he holds in a fiery belch.",TRUE, enchanted_ch,NULL,NULL,TO_ROOM);
            send_to_char("Without a target, your belch gives you heartburn.\n\r", enchanted_ch);
            damage(enchanted_ch,enchanted_ch,number(lvl/5,lvl/4),TYPE_UNDEFINED,DAM_FIRE);
         }
      }
      return(FALSE);
   }
*/

  if((cmd == CMD_FIREBREATH) && (enchanted_ch==char_in_room))
    {
    one_argument(arg,name);
    if(!*name) {
      victim=enchanted_ch->specials.fighting;
    }
    else {
      victim = get_char_room_vis(enchanted_ch,name);
    }
    //lvl=MIN(GET_LEVEL(enchanted_ch),30);
    lvl=GET_LEVEL(enchanted_ch);
    ench->duration--;
    if(victim)
      {
      if(number(1,110)<
             MIN(100,(50+(lvl-GET_LEVEL(victim))*3)))
        {
        act("$n's stomach rumbles and a tremendous belch of fire erupts.",TRUE, enchanted_ch,NULL,victim,TO_ROOM);
        act("Your stomach rumbles and a tremendous belch of fire erupts.",TRUE, enchanted_ch,NULL,victim,TO_CHAR);

        act("$n's fiery belch envelopes $N, igniting $M into a human torch.",TRUE, enchanted_ch,NULL,victim,TO_NOTVICT);
        act("Your fiery belch envelopes $N, igniting $M into a human torch.",TRUE, enchanted_ch,NULL,victim,TO_CHAR);
        act("You are enveloped by $n's fiery belch and are ignited into a human torch.",TRUE, enchanted_ch,NULL,victim,TO_VICT);
        damage(enchanted_ch,victim,number(lvl*2,lvl*4),TYPE_UNDEFINED,DAM_FIRE);
        }
      else
        {
        act("$n's stomach rumbles,but only a smoking oily stream of fire emerges.",TRUE, enchanted_ch,NULL,victim,TO_NOTVICT);
        act("Your stomach rumbles,but only a smoking oily stream of fire emerges.",TRUE, enchanted_ch,NULL,victim,TO_CHAR);

        act("$n's belch fizzles, only managing to singe $N's hair.",TRUE, enchanted_ch,NULL,victim,TO_ROOM);
        act("Your belch fizzles, only managing to singe $N's hair.",TRUE, enchanted_ch,NULL,victim,TO_CHAR);
        act("$n's fizzling belch only manages to singe your hair.",TRUE, enchanted_ch,NULL,victim,TO_VICT);
        damage(enchanted_ch,victim,number(lvl,lvl*2),TYPE_UNDEFINED,DAM_FIRE);
        }
      if(ench->duration<0)
        {
        firebreath_enchantment(ench,enchanted_ch,char_in_room,MSG_REMOVE_ENCH,NULL);
        enchantment_remove(enchanted_ch,ench,0);
        }
      }
    else
      {
      act("$n gets a pained look in $s face as he holds in a fiery belch.",TRUE, enchanted_ch,NULL,victim,TO_ROOM);
      send_to_char("Without a target, your belch gives you heartburn.\n\r", enchanted_ch);
      damage(enchanted_ch,enchanted_ch,number(lvl/5,lvl/4),TYPE_UNDEFINED,DAM_FIRE);
      }
    return(TRUE);
  }
  return(FALSE);
}

/* Next 3 Added by Quack for testing in Early 1997
int silence_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
 switch (cmd) {
  case CMD_SHOUT:
  case CMD_GROUPTELL:
  case CMD_BUY:
  case CMD_ASK:
  case CMD_WHISPER:
  case CMD_TELL:
  case CMD_AUCTION:
  case CMD_YELL:
  case CMD_GOSSIP:
  case CMD_SAY:
  case CMD_RECITE:
  case CMD_CAST:
   if (ch!=char_in_room) return(FALSE);
    act("$n moves $s lips but no words come out.",TRUE, ch,NULL,NULL,TO_ROOM);
    send_to_char("You move your lips but no words come out.\n\r",ch);
    return(TRUE);
    break;


  case MSG_REMOVE_ENCH:
     send_to_char("Your are no longer silenced.\n\r",ch);
     return(FALSE);
     break;

  if(ench->duration<0)
   {
    silence_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
    enchantment_remove(ch,ench,0);
   }
 }
 return(FALSE);
}

int shapeshift_enchantment(struct enchantment_type_5 *ench,  CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
 switch (cmd) {
  case CMD_GOSSIP:
  case CMD_SHOUT:
  case CMD_GROUPTELL:
  case CMD_BUY:
  case CMD_ASK:
  case CMD_WHISPER:
  case CMD_TELL:
  case CMD_AUCTION:
  case CMD_YELL:
  case CMD_SAY:
  case CMD_RECITE:
  case CMD_CAST:
    if (ch!=char_in_room) return(FALSE);

      act("$n bares $s fangs and lets out a low growl.",TRUE, ch,NULL,NULL,TO_ROOM);
    send_to_char("You bare your fangs and let out a low growl.\n\r",ch);
    return(TRUE);
    break;

  case CMD_WEAR:
  case CMD_REMOVE:
   if (ch!=char_in_room) return(FALSE);
    act("$n scratches $s chin with one clawed paw.",TRUE, ch,NULL,NULL,TO_ROOM);
    send_to_char("You scratch your chin with one clawed paw.\n\r",ch);
    return(TRUE);
    break;

  case MSG_SHOW_PRETITLE:
      strcat(arg,"The werewolf ");
      return(TRUE);
      break;

  case MSG_FIGHT:
   if (ch->specials.fighting) {
    note this damage cmd is old style
     damage(ch, ch->specials.fighting, number(25,100), TYPE_BITE );
     damage(ch, ch->specials.fighting, number(25,100), TYPE_CLAW );
     return(FALSE);
   }
     break;

  case MSG_REMOVE_ENCH:
     act("$n screams in pain as $e reverts to $s human form.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You scream in pain as you revert to your human form.\n\r",ch);
      extract_obj(unequip_char(ch,WIELD));
     return FALSE;
     break;

   case MSG_TICK:
    if (!number(0,4) && AWAKE(ch)) {
     act("$n lets out a loud roar of frustration.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You let out a loud roar of frustration.\n\r",ch);
      }
     return(FALSE);
     break;

    case CMD_KILL:
     if (ch!=char_in_room) return(FALSE);
     act("$n roars loudly and attacks.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You roar loudly and attack.\n\r",ch);
     return FALSE;
     break;

    if(ench->duration<0)
    {
    shapeshift_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
    enchantment_remove(ch,ench,0);
    }
 }
 return(FALSE);
}

int quester_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
 switch (cmd) {

  case MSG_SHOW_AFFECT_TEXT:
    act("......$n glows with a blue light.",FALSE, ch,0,char_in_room,TO_VICT);
    return(FALSE);
    break;

  case MSG_REMOVE_ENCH:
     send_to_char("You are no longer a Champion Quester!\n\r",ch);
     return(FALSE);
     break;

  if(ench->duration<0)
   {
    quester_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
    enchantment_remove(ch,ench,0);
   }
 }
 return(FALSE);
}
*/

/* Linerfix, adding hell ench calls */

int sin_wrath(struct enchantment_type_5 *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg);
int sin_envy(struct enchantment_type_5 *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg);
int sin_lust(struct enchantment_type_5 *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg);
int sin_avarice(struct enchantment_type_5 *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg);
int sin_pride(struct enchantment_type_5 *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg);
int sin_gluttony(struct enchantment_type_5 *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg);
int sin_sloth(struct enchantment_type_5 *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg);
int red_death(struct enchantment_type_5 *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg);
int lizard_bite(struct enchantment_type_5 *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg);
int greasy_palms(struct enchantment_type_5 *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg);

#ifdef TEST_SITE
/* Hemp - adding Digsite enchantment calls */
int toxic_fumes_ench(struct enchantment_type_5 *ench, CHAR *ench_ch, CHAR *char_in_room, int cmd, char*arg);
#endif

/* Red Dragons enchantment - Solmyr */
int frightful_presence(struct enchantment_type_5 *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg);

/* enchantment calls for Abyss revamp by Solmyr */
int lich_curse(struct enchantment_type_5 *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg);

/******************************************************************************
 *               Ranks.....oh joy, fun fun                                    *
 ******************************************************************************/
/*  Warriors*/
int squire_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The title of Swashbuckler"))
      {
      strcat(arg,"Squire ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Squire") && !enchanted_by(ch,"The title of Swashbuckler") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      squire_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }

   if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s nobility.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your nobility.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.15* hit_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.15*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}

int swashbuckler_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The title of Knight"))
      {
      strcat(arg,"Swashbuckler ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Swashbuckler") && !enchanted_by(ch,"The title of Knight") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      swashbuckler_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN(hit_limit(ch), GET_HIT (ch) + 0.075*hit_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.075*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
int knight_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    strcat(arg,"Knight ");
    return(TRUE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Knight") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      knight_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN(hit_limit(ch), GET_HIT (ch) + 0.075*hit_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.075*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}

/*  Paladin's*/
int firstsword_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The title of Justiciar"))
      {
      strcat(arg,"First Sword ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of First Sword") && !enchanted_by(ch,"The title of Justiciar") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      firstsword_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }

   if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s rank.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your rank.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.1* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.1*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.1*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}

int justiciar_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The title of Lord/Lady"))
      {
      strcat(arg,"Justiciar ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Justiciar") && !enchanted_by(ch,"The title of Lord/Lady") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      justiciar_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.05*hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
int lordlady_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
     if(GET_SEX(ch)==SEX_MALE)
        strcat(arg,"Lord ");
      else
        strcat(arg,"Lady ");
      return(TRUE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Lord/Lady") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      lordlady_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.05* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
/*  Nomads  */
int wanderer_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The title of Forester"))
      {
      strcat(arg,"Wanderer ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Wanderer") && !enchanted_by(ch,"The title of Forester") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      wanderer_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s wanderer status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your wanderer status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.15* hit_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.15*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
int forester_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The title of Tamer"))
      {
      strcat(arg,"Forester ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Forester") && !enchanted_by(ch,"The title of Tamer") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      forester_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.075* hit_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.075*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
int tamer_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
      strcat(arg,"Tamer ");
      return(TRUE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Tamer") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      tamer_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.075* hit_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.075*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
/*  mages  */
int apprentice_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The title of Warlock"))
      {
      strcat(arg,"Apprentice ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Apprentice") && !enchanted_by(ch,"The title of Warlock") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      apprentice_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s apprenticeship.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your apprenticeship.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.1* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.1*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.1*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
int warlock_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The title of Sorcerer"))
      {
      strcat(arg,"Warlock ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Warlock") && !enchanted_by(ch,"The title of Sorcerer") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      warlock_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.05* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
int sorcerer_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
      strcat(arg,"Sorcerer ");
      return(TRUE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Sorcerer") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      sorcerer_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.05* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
/* bard */
int minstrel_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The title of Poet"))
      {
      strcat(arg,"Minstrel ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Minstrel") && !enchanted_by(ch,"The title of Poet") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      minstrel_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }

  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.1* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.1*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.1*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
int poet_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The title of Conductor"))
      {
      strcat(arg,"Poet ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Poet") && !enchanted_by(ch,"The title of Conductor") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      poet_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.05* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
int conductor_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
      strcat(arg,"Conductor ");
      return(TRUE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Conductor") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      conductor_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.05* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}

/* Commando */
int private_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The rank of Commodore"))
      {
      strcat(arg,"Private ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The rank of Private") && !enchanted_by(ch,"The rank of Commodore") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      private_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s stripes.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your stripes.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.1* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.1*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.1*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
int commodore_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The rank of Commander"))
      {
      strcat(arg,"Commodore ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The rank of Commodore") && !enchanted_by(ch,"The rank of Commander") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      commodore_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.05* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
int commander_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
      strcat(arg,"Commander ");
      return(TRUE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The rank of Commander") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      commander_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.05* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
/* Thief */
int highwayman_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The status of Brigand"))
      {
      strcat(arg,"Highwayman ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The status of Highwayman") && !enchanted_by(ch,"The status of Brigand") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      highwayman_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.15* hit_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.15*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}

int brigand_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The status of Assassin"))
      {
      strcat(arg,"Brigand ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The status of Brigand") && !enchanted_by(ch,"The status of Assassin") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      brigand_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.075* hit_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.075*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
int assassin_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
      strcat(arg,"Assassin ");
      return(TRUE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The status of Assassin") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      assassin_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.075* hit_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.075*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
/* Anti Paladin */
int minion_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The title of Dark Warder"))
      {
      strcat(arg,"Evil Minion ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Minion of Darkness") && !enchanted_by(ch,"The title of Dark Warder") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      minion_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s minionhood.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your minionhood.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.1* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.1*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.1*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}

int darkwarder_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The title of Dark Lord/Lady"))
      {
      strcat(arg,"Dark Warder ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Dark Warder") && !enchanted_by(ch,"The title of Dark Lord/Lady") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      darkwarder_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.05* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
int darklordlady_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
     if(GET_SEX(ch)==SEX_MALE)
        strcat(arg,"Dark Lord ");
      else
        strcat(arg,"Dark Lady ");
      return(TRUE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Dark Lord/Lady") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      darklordlady_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.05* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
/* Ninja */
int tsume_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The title of Shinobi"))
      {
      strcat(arg,"Tsume ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Tsume") && !enchanted_by(ch,"The title of Shinobi") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      tsume_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.1* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.1*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.1*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
int shinobi_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The title of Shogun"))
      {
      strcat(arg,"Shinobi ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Shinobi") && !enchanted_by(ch,"The title of Shogun") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      shinobi_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.05* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
int shogun_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    strcat(arg,"Shogun ");
    return(TRUE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Shogun") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      shogun_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.05* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}

/* cleric */

int acolyte_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The title of Bishop"))
      {
      strcat(arg,"Acolyte ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Acolyte") && !enchanted_by(ch,"The title of Bishop") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      acolyte_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s position in the church.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your position in the church.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.1* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.1*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.1*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
int bishop_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    if(!enchanted_by(ch,"The title of Prophet"))
      {
      strcat(arg,"Bishop ");
      return(TRUE);
      }
      else return(FALSE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Bishop") && !enchanted_by(ch,"The title of Prophet") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      bishop_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.05* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}
int prophet_enchantment(struct enchantment_type_5 *ench, CHAR *ch, CHAR *char_in_room,int cmd, char*arg)
{
  if(cmd==MSG_SHOW_PRETITLE)
    {
    strcat(arg,"Prophet ");
    return(TRUE);
    }
  if(cmd==MSG_DEAD)
    {
    if(enchanted_by(ch,"The title of Prophet") &&
       !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,CHAOTIC))
      {
      prophet_enchantment(ench,ch,char_in_room,MSG_REMOVE_ENCH,NULL);
      enchantment_remove(ch,ench,1);
      }
    return(FALSE);
    }
  if(cmd==MSG_REMOVE_ENCH)
     {
     act("$n is stripped of $s ranking status.",FALSE, ch,NULL,NULL,TO_ROOM);
     send_to_char("You are stripped of your ranking status.\n\r",ch);
     return FALSE;
     }
  if(cmd==MSG_TICK)
    {
    GET_HIT (ch) = MIN( hit_limit(ch), GET_HIT (ch) + 0.05* hit_gain(ch));
    GET_MANA(ch) = MIN(mana_limit(ch), GET_MANA(ch) + 0.05*mana_gain(ch));
    GET_MOVE(ch) = MIN(move_limit(ch), GET_MOVE(ch) + 0.05*move_gain(ch));
    ench->duration=-1;
    return(FALSE);
    }
  return(FALSE);
}

void assign_enchantments(void)
{
  log_f("Defining enchantments");
  enchantments = (struct enchantment_type_5*)calloc(TOTAL_ENCHANTMENTS,
                                                  sizeof(struct enchantment_type_5));

/*         name                              enchant #        duration,mod.,location,        bitvector,bitvector2,function*/
  ENCHANTO("Remort"                         ,ENCHANT_REMORTV2     , -1 , 0  ,0                    ,0,0                    ,remortv2_enchantment);
  ENCHANTO("Immortalis' Grace"              ,ENCHANT_IMM_GRACE    , -1 , 0  ,0                    ,0,0                    ,imm_grace_enchantment); /* Project Broadsword */

  ENCHANTO("Common Cold"                    ,ENCHANT_COLD         , 20 , 0  ,0                    ,0,0                    ,cold_enchantment);
  ENCHANTO("Fire Breath"                    ,ENCHANT_FIREBREATH   ,  6 , 0  ,0                    ,0,0                    ,firebreath_enchantment);
  ENCHANTO("Regeneration"                   ,ENCHANT_REGENERATION , 24 , 0  ,0                    ,0,0                    ,regeneration_enchantment);

  ENCHANTO("The title of Squire"            ,ENCHANT_SQUIRE       , -1 , 1  ,APPLY_HITROLL        ,0,0                    ,squire_enchantment);
  ENCHANTO("The title of Swashbuckler"      ,ENCHANT_SWASHBUCKLER , -1 , 5  ,APPLY_SKILL_BLOCK    ,0,0                    ,swashbuckler_enchantment);
  ENCHANTO("The title of Knight"            ,ENCHANT_KNIGHT       , -1 , 5  ,APPLY_SKILL_DUAL     ,AFF_SENSE_LIFE,0       ,knight_enchantment);

  ENCHANTO("The title of First Sword"       ,ENCHANT_FIRSTSWORD   , -1 , 1  ,APPLY_HITROLL        ,AFF_DETECT_ALIGNMENT,0 ,firstsword_enchantment);
  ENCHANTO("The title of Justiciar"         ,ENCHANT_JUSTICIAR    , -1 , 5  ,APPLY_SKILL_BLOCK    ,0,0                    ,justiciar_enchantment);
  ENCHANTO("The title of Lord/Lady"         ,ENCHANT_LORDLADY     , -1 , 5  ,APPLY_SKILL_PUMMEL   ,0,0                    ,lordlady_enchantment);

  ENCHANTO("The title of Apprentice"        ,ENCHANT_APPRENTICE   , -1 , 0  ,APPLY_NONE           ,AFF_DETECT_INVISIBLE,0 ,apprentice_enchantment);
  ENCHANTO("The title of Warlock"           ,ENCHANT_WARLOCK      , -1 , 1  ,APPLY_HITROLL        ,AFF_DETECT_MAGIC,0     ,warlock_enchantment);
  ENCHANTO("The title of Sorcerer"          ,ENCHANT_SORCERER     , -1 , 1  ,APPLY_DAMROLL        ,AFF_FLY,0              ,sorcerer_enchantment);

  ENCHANTO("The rank of Private"            ,ENCHANT_PRIVATE      , -1 , 5  ,APPLY_SKILL_DUAL     ,0,0                    ,private_enchantment);
  ENCHANTO("The rank of Commodore"          ,ENCHANT_COMMODORE    , -1 , 5  ,APPLY_SKILL_ASSAULT  ,0,0                    ,commodore_enchantment);
  ENCHANTO("The rank of Commander"          ,ENCHANT_COMMANDER    , -1 , 5  ,APPLY_SKILL_TRIPLE   ,0,0                    ,commander_enchantment);

  ENCHANTO("The title of Wanderer"          ,ENCHANT_WANDERER     , -1 , 5  ,APPLY_SKILL_AMBUSH   ,AFF_DETECT_MAGIC,0     ,wanderer_enchantment);
  ENCHANTO("The title of Forester"          ,ENCHANT_FORESTER     , -1 , 1  ,APPLY_DAMROLL        ,AFF_INFRAVISION,0      ,forester_enchantment);
  ENCHANTO("The title of Tamer"             ,ENCHANT_TAMER        , -1 , 5  ,APPLY_SKILL_DISEMBOWEL,AFF_DETECT_INVISIBLE,0,tamer_enchantment);

  ENCHANTO("The title of Tsume"             ,ENCHANT_TSUME        , -1 , 0  ,APPLY_NONE           ,AFF_INFRAVISION,0      ,tsume_enchantment);
  ENCHANTO("The title of Shinobi"           ,ENCHANT_SHINOBI      , -1 , 5  ,APPLY_SKILL_PUMMEL   ,AFF_SENSE_LIFE,0       ,shinobi_enchantment);
  ENCHANTO("The title of Shogun"            ,ENCHANT_SHOGUN       , -1 , 5  ,APPLY_SKILL_ASSAULT  ,0,0                    ,shogun_enchantment);

  ENCHANTO("The title of Minstrel"          ,ENCHANT_TRAVELER     , -1 , 1  ,APPLY_HITROLL        ,0,0                    ,minstrel_enchantment);
  ENCHANTO("The title of Poet"              ,ENCHANT_POET         , -1 , 1  ,APPLY_DAMROLL        ,AFF_SENSE_LIFE,0       ,poet_enchantment);
  ENCHANTO("The title of Conductor"         ,ENCHANT_CONDUCTOR    , -1 , 5  ,APPLY_SKILL_BACKFLIP ,AFF_INFRAVISION,0      ,conductor_enchantment);

  ENCHANTO("The title of Minion of Darkness",ENCHANT_MINION       , -1 , 5  ,APPLY_SKILL_BACKSTAB ,0,0                    ,minion_enchantment);
  ENCHANTO("The title of Dark Warder"       ,ENCHANT_DARKWARDER   , -1 , 0  ,0                    ,AFF_INFRAVISION,0      ,darkwarder_enchantment);
  ENCHANTO("The title of Dark Lord/Lady"    ,ENCHANT_DARKLORDLADY , -1 , 1  ,APPLY_DAMROLL        ,0,0                    ,darklordlady_enchantment);

  ENCHANTO("The title of Acolyte"           ,ENCHANT_ACOLYTE      , -1 , 0  ,APPLY_NONE           ,AFF_SPHERE,0           ,acolyte_enchantment);
  ENCHANTO("The title of Bishop"            ,ENCHANT_BISHOP       , -1 , 1  ,APPLY_HITROLL        ,AFF_DETECT_ALIGNMENT,0 ,bishop_enchantment);
  ENCHANTO("The title of Prophet"           ,ENCHANT_PROPHET      , -1 , 1  ,APPLY_DAMROLL        ,AFF_SENSE_LIFE,0       ,prophet_enchantment);

  ENCHANTO("The status of Highwayman"       ,ENCHANT_HIGHWAYMAN   , -1 , 0  ,APPLY_NONE           ,AFF_SNEAK,0            ,highwayman_enchantment);
  ENCHANTO("The status of Brigand"          ,ENCHANT_BRIGAND      , -1 , 1  ,APPLY_HITROLL        ,AFF_SENSE_LIFE,0       ,brigand_enchantment);
  ENCHANTO("The status of Assassin"         ,ENCHANT_ASSASSIN     , -1 , 5  ,APPLY_SKILL_CIRCLE   ,AFF_INFRAVISION,0      ,assassin_enchantment);

  ENCHANTO("Deadly Sin - Wrath"             ,ENCHANT_WRATH        , 66 , 0  ,0                    ,0,0,sin_wrath);
  ENCHANTO("Deadly Sin - Envy"              ,ENCHANT_ENVY         , 66 , 0  ,0                    ,0,0,sin_envy);
  ENCHANTO("Deadly Sin - Lust"              ,ENCHANT_LUST         , 66 , 0  ,0                    ,0,0,sin_lust);
  ENCHANTO("Deadly Sin - Pride"             ,ENCHANT_PRIDE        , 66 , 0  ,0                    ,0,0,sin_pride);
  ENCHANTO("Deadly Sin - Avarice"           ,ENCHANT_AVARICE      , 66 , 0  ,0                    ,0,0,sin_avarice);
  ENCHANTO("Deadly Sin - Gluttony"          ,ENCHANT_GLUTTONY     , 66 , 0  ,0                    ,0,0,sin_gluttony);
  ENCHANTO("Deadly Sin - Sloth"             ,ENCHANT_SLOTH        , 66 , 0  ,0                    ,0,0,sin_sloth);
  ENCHANTO("Greasy Palms"                   ,ENCHANT_GREASY       , 6  , 0  ,0                    ,0,0,greasy_palms);
  ENCHANTO("Red Death"                      ,ENCHANT_REDDEATH     , 30 , 0  ,0                    ,0,0,red_death);
  ENCHANTO("Lizard Lycanthropy"             ,ENCHANT_LIZARD       , 30 , 0  ,0                    ,0,0,lizard_bite);

#ifdef TEST_SITE
  ENCHANTO("Toxic Fumes"                    ,ENCHANT_TOXICFUMES   , -1 , 0  ,0                    ,0,0,toxic_fumes_ench);
#endif
  ENCHANTO("Frightful Presence"             ,ENCHANT_FRIGHTFUL    , 10 , -5 ,APPLY_HITROLL        ,0,0,frightful_presence);
  ENCHANTO("The Curse of the Lich"          ,ENCHANT_LICH         , 5  , -5 ,APPLY_HITROLL        ,0,0,lich_curse);
/* These 3 were added by Quack for trial in Early 1997
  ENCHANTO("Magical silence"                ,ENCHANT_SILENCE      ,  2 , 0  ,APPLY_NONE           ,AFF_SNEAK,0            ,silence_enchantment);
  ENCHANTO("Wolf lycanthropy"               ,ENCHANT_SHAPESHIFT   ,  2 , 0  ,APPLY_NONE           ,AFF_INFRAVISION,0      ,shapeshift_enchantment);
  ENCHANTO("Champion Quester"               ,ENCHANT_QUESTER      , -1 , 3  ,APPLY_DAMROLL        ,AFF_SANCTUARY,0        ,quester_enchantment);*/
}

/*Name takes precedence over enchantment number, unless the name isnt in the
 *global enchantment list */

struct enchantment_type_5 *get_enchantment(struct enchantment_type_5 *enchantment, int must_find)
{
   int i;
   int found=-1;
   struct enchantment_type_5 *TmpEnchantment;
   if(!enchantment->name)
     found = enchantment->type;
   else
     {
     for(i= 0;i<TOTAL_ENCHANTMENTS;i++)
       {
       if(enchantments[i].name && enchantment->name)
         {
         if(!strcmp(enchantments[i].name,enchantment->name))
            found = i;
         }
       }
     }
   if(found == -1 && must_find)
      return NULL;
   CREATE(TmpEnchantment,struct enchantment_type_5,1);
   if(found!=-1)
     {
     if(!enchantment->name)
       TmpEnchantment->name     = str_dup(enchantments[found].name);
     else
       TmpEnchantment->name     = str_dup(enchantment->name);
     if(!enchantment->duration)
       TmpEnchantment->duration = enchantments[found].duration;
     else
       TmpEnchantment->duration = enchantment->duration;
     TmpEnchantment->location   = enchantments[found].location;
     TmpEnchantment->modifier   = enchantments[found].modifier;
     TmpEnchantment->bitvector  = enchantments[found].bitvector;
     TmpEnchantment->bitvector2  = enchantments[found].bitvector2;
     TmpEnchantment->type       = found;
     TmpEnchantment->func      = enchantments[found].func;
     }
   else
     {
     TmpEnchantment->name       = str_dup(enchantment->name);
     TmpEnchantment->location   = enchantment->location;
     TmpEnchantment->modifier   = enchantment->modifier;
     TmpEnchantment->bitvector  = enchantment->bitvector;
     TmpEnchantment->bitvector2  = enchantment->bitvector2;
     TmpEnchantment->type       = 0;
     TmpEnchantment->func       = enchantment->func;
     TmpEnchantment->duration   = enchantment->duration;
     }
   return TmpEnchantment;
}
void enchantment_to_char(CHAR *victim,struct enchantment_type_5 *enchantment, int must_find)
{
     int found = FALSE;

     struct enchantment_type_5 *ench;
     struct enchantment_type_5 *next_ench;
     struct enchantment_type_5 *TmpEnch;
     struct enchantment_type_5 *i;

     TmpEnch = get_enchantment(enchantment,must_find);
     if(!TmpEnch)
       return;
     /*If there's an enchantment already there, it replaces it.*/
     for(ench=victim->enchantments;ench&&!found;ench=next_ench)
        {
        next_ench = ench->next;
        if(!strcmp(ench->name,TmpEnch->name))
          {
          enchantment_remove(victim,ench,1);
          ench = NULL;
          found = TRUE;
          }
        }
     if(!victim->enchantments)
       victim->enchantments = TmpEnch;
     else
       {
       for(i = victim->enchantments;i->next;i= i->next)
          ;
       i->next = TmpEnch;
       }
     affect_modify(victim,TmpEnch->location,TmpEnch->modifier,
                          TmpEnch->bitvector,TmpEnch->bitvector2,TRUE);

     affect_total(victim);
     check_equipment(victim);
}
struct enchantment_type_5 *enchantment_remove(CHAR *victim,struct enchantment_type_5 *enchantment, int tolog)
{
     char buf[MAX_STRING_LENGTH];
     struct enchantment_type_5 *ench=NULL;
     struct enchantment_type_5 *next=NULL;
     struct enchantment_type_5 **previous;
     affect_modify(victim,enchantment->location,enchantment->modifier,
                          enchantment->bitvector,enchantment->bitvector2,FALSE);

     /*  find it in the linked list, and remove it */
     previous = &victim->enchantments;
     for(ench=victim->enchantments;ench;ench = next)
      {
      next = ench->next;
      if(ench==enchantment)
        {
        *previous = next;
        if(tolog) {
          sprintf(buf,"PLRINFO: %s just had enchantment %s removed.",GET_NAME(victim),ench->name);
          log_f(buf);
        }
        if(enchantment->name)
          free(enchantment->name);
        free(enchantment);
        affect_total(victim);
        check_equipment(victim);
        }
      else
        {
        previous = &ench->next;
        }
      }
     return NULL;
}

int enchanted_by(CHAR *ch, char *enchantment_name)
{
  struct enchantment_type_5 *ench     =NULL;
  struct enchantment_type_5 *next_ench=NULL;
  int found = FALSE;
     for(ench=ch->enchantments;ench&&!found;ench=next_ench)
        {
        next_ench = ench->next;
        if(!strcmp(ench->name,enchantment_name))
          {
          found = TRUE;
          }
        }
  return found;
}

int enchanted_by_type(CHAR *ch, int type)
{
  struct enchantment_type_5 *ench = NULL;

  for (ench = ch->enchantments; ench; ench = ench->next)
  {
    if (ench->type == type)
    {
      return TRUE;
    }
  }

  return FALSE;
}

int get_rank(CHAR *ch)
{
  struct enchantment_type_5 *ench = NULL;
  int rank = 0;

  for (ench = ch->enchantments; ench; ench = ench->next)
  {
    switch (ench->type)
    {
      case ENCHANT_SQUIRE:
      case ENCHANT_FIRSTSWORD:
      case ENCHANT_APPRENTICE:
      case ENCHANT_PRIVATE:
      case ENCHANT_WANDERER:
      case ENCHANT_TSUME:
      case ENCHANT_TRAVELER:
      case ENCHANT_MINION:
      case ENCHANT_ACOLYTE:
      case ENCHANT_HIGHWAYMAN:
        if (rank < 1)
        {
          rank = 1;
        }
      break;

      case ENCHANT_SWASHBUCKLER:
      case ENCHANT_JUSTICIAR:
      case ENCHANT_WARLOCK:
      case ENCHANT_COMMODORE:
      case ENCHANT_FORESTER:
      case ENCHANT_SHINOBI:
      case ENCHANT_POET:
      case ENCHANT_DARKWARDER:
      case ENCHANT_BISHOP:
      case ENCHANT_BRIGAND:
        if (rank < 2)
        {
          rank = 2;
        }
      break;

      case ENCHANT_KNIGHT:
      case ENCHANT_LORDLADY:
      case ENCHANT_SORCERER:
      case ENCHANT_COMMANDER:
      case ENCHANT_TAMER:
      case ENCHANT_SHOGUN:
      case ENCHANT_CONDUCTOR:
      case ENCHANT_DARKLORDLADY:
      case ENCHANT_PROPHET:
      case ENCHANT_ASSASSIN:
        if (rank < 3)
        {
          rank = 3;
        }
      break;
    }
  }

  return rank;
}

/* Project Epee */
char *get_rank_name(CHAR *ch)
{
  struct enchantment_type_5 *ench = NULL;
  int rank = 0;

  rank = get_rank(ch);

  if (rank >= 1 && rank <= 3)
  {
    for (ench = ch->enchantments; ench; ench = ench->next)
    {
      switch (ench->type)
      {
        case ENCHANT_SQUIRE:
          if (rank == 1) return "Squire";
          break;
        case ENCHANT_FIRSTSWORD:
          if (rank == 1) return "First Sword";
          break;
        case ENCHANT_APPRENTICE:
          if (rank == 1) return "Apprentice";
          break;
        case ENCHANT_PRIVATE:
          if (rank == 1) return "Private";
          break;
        case ENCHANT_WANDERER:
          if (rank == 1) return "Wanderer";
          break;
        case ENCHANT_TSUME:
          if (rank == 1) return "Tsume";
          break;
        case ENCHANT_TRAVELER:
          if (rank == 1) return "Minstrel";
          break;
        case ENCHANT_MINION:
          if (rank == 1) return "Evil Minion";
          break;
        case ENCHANT_ACOLYTE:
          if (rank == 1) return "Acolyte";
          break;
        case ENCHANT_HIGHWAYMAN:
          if (rank == 1) return "Highwayman";
          break;
        case ENCHANT_SWASHBUCKLER:
          if (rank == 2) return "Swashbuckler";
          break;
        case ENCHANT_JUSTICIAR:
          if (rank == 2) return "Justiciar";
          break;
        case ENCHANT_WARLOCK:
          if (rank == 2) return "Warlock";
          break;
        case ENCHANT_COMMODORE:
          if (rank == 2) return "Commodore";
          break;
        case ENCHANT_FORESTER:
          if (rank == 2) return "Forester";
          break;
        case ENCHANT_SHINOBI:
          if (rank == 2) return "Shinobi";
          break;
        case ENCHANT_POET:
          if (rank == 2) return "Poet";
          break;
        case ENCHANT_DARKWARDER:
          if (rank == 2) return "Dark Warder";
          break;
        case ENCHANT_BISHOP:
          if (rank == 2) return "Bishop";
          break;
        case ENCHANT_BRIGAND:
          if (rank == 2) return "Brigand";
          break;
        case ENCHANT_KNIGHT:
          if (rank == 3) return "Knight";
          break;
        case ENCHANT_LORDLADY:
          if (rank == 3)
          {
            if (GET_SEX(ch) == SEX_FEMALE)
              return "Lady";
            else
              return "Lord";
          }
          break;
        case ENCHANT_SORCERER:
          if (rank == 3) return "Sorcerer";
          break;
        case ENCHANT_COMMANDER:
          if (rank == 3) return "Commander";
          break;
        case ENCHANT_TAMER:
          if (rank == 3) return "Tamer";
          break;
        case ENCHANT_SHOGUN:
          if (rank == 3) return "Shogun";
          break;
        case ENCHANT_CONDUCTOR:
          if (rank == 3) return "Conductor";
          break;
        case ENCHANT_DARKLORDLADY:
          if (rank == 3)
          {
            if (GET_SEX(ch) == SEX_FEMALE)
              return "Dark Lady";
            else
              return "Dark Lord";
          }
          break;
        case ENCHANT_PROPHET:
          if (rank == 3) return "Prophet";
          break;
        case ENCHANT_ASSASSIN:
          if (rank == 3) return "Assassin";
          break;
      }
    }
  }

  return "None";
}
