
/**************************************************************************
*  file: act.other.c , Implementation of commands.        Part of DIKUMUD *
*  Usage : Other commands.                                                *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "utility.h"
#include "handler.h"
#include "fight.h"
#include "act.h"
#include "modify.h"
#include "enchant.h"
#include "cmd.h"
#include "subclass.h"

/* extern procedures */

extern void do_create_save(CHAR *ch, char* arg);
extern void do_practice(struct char_data *ch, char *arg, int cmd);
extern int SPELL_LEVEL(struct char_data *ch, int sn);

int USE_MANA(CHAR *ch, int sn);

void hit(struct char_data *ch, struct char_data *victim, int type);
void do_shout(struct char_data *ch, char *argument, int cmd);

void do_vicious(struct char_data *ch, char *argument, int cmd) {

     if(IS_NPC(ch)) return;

  if(IS_SET(ch->specials.pflag, PLR_VICIOUS)) {
    send_to_char("Vicious mode off.\n\r", ch);
    send_to_char("You will no longer autokill mortally wounded creatures.\n\r", ch);
    REMOVE_BIT(ch->specials.pflag, PLR_VICIOUS);
  }
  else {
    send_to_char("Vicious mode on.\n\r", ch);
    send_to_char("You will now autokill mortally wounded creatures.\n\r", ch);
    SET_BIT(ch->specials.pflag, PLR_VICIOUS);
  }
}

void do_descr(struct char_data *ch, char *argument, int cmd)
{

  if(IS_SET(ch->specials.pflag, PLR_NOSHOUT)) {
    send_to_char("Sorry, can't change your description while noshouted!\n\r", ch);
    return;
  }

  if(ch->desc&&!IS_NPC(ch))
   {
   send_to_char("Enter your description...add a `o@`q to end it.\n\rYour description must be 240 characters or less.\n\rMake sure to `osave`q when you are finished.\n\r",ch);
   ch->player.description = NULL;
   ch->desc->str = &ch->player.description;
   ch->desc->max_str = 240;
   }
}
void do_trap(struct char_data *ch, char *argument, int cmd)
{
  int percent;

  if(!ch->skills)
    return;

  if ((GET_CLASS(ch) != CLASS_NOMAD) &&
      (GET_LEVEL(ch) < LEVEL_IMM)) {
    send_to_char("You don't know this skill.\n\r", ch);
    return;
  }

  percent = number(1, 101);

  if(percent > ch->skills[SKILL_TRAP].learned)
    {     send_to_char("You failed.\n\r", ch);
     return;
      }

     if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE)&& (!CHAOSMODE))
          send_to_char("Behave yourself here please!\n\r", ch);

     if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, TRAP))
          send_to_char("There is already a trap here.\n\r", ch);
     else {
          send_to_char("You have set up a trap here.\n\r", ch);
          act("$n has set up a trap here.", TRUE, ch, 0, 0,TO_ROOM);

          SET_BIT(world[CHAR_REAL_ROOM(ch)].room_flags, TRAP);
     }
}

void do_glance(struct char_data *ch, char *argument, int cmd)
{
     int percent;
     char buffer[MAX_STRING_LENGTH];
     struct char_data *victim;
     char arg[MAX_STRING_LENGTH];

     one_argument(argument, arg);

     if(!*arg)
     { send_to_char("Glance at who?\n\r", ch);
       return;
     }

     if(!(victim = get_char_room_vis(ch, arg)))
     {     send_to_char("No-one by that name is here.\n\r", ch);
          return;}

     percent = (100*GET_HIT(victim))/GET_MAX_HIT(victim);

     if (IS_NPC(victim))
     {
         sprintf(buffer, "%s", GET_SHORT(victim));
     }
     else
     {
         sprintf(buffer, "%s", GET_NAME(victim));
     }
                if(percent >= 100)
                        strcat(buffer, " is in an excellent condition.\n\r");
                else if(percent >= 90)
                        strcat(buffer, " has a few scratches.\n\r");
                else if(percent >= 75)
                        strcat(buffer, " has some small wounds and bruises.\n\r");
                else if(percent >= 50)
                        strcat(buffer, " has quite a few wounds.\n\r");
                else if(percent >= 30)
                        strcat(buffer, " has some big nasty wounds and scratches.\n\r");
                else if(percent >= 15)
                        strcat(buffer, " looks pretty hurt.\n\r");
                else if(percent >= 0)
                        strcat(buffer, " is in an awful condition.\n\r");
                else
                        strcat(buffer, " is bleeding awfully from big wounds.\n\r");

                send_to_char(buffer, ch);


     act("$n glances at $N.", TRUE, ch, 0, victim, TO_NOTVICT);
     act("$n glances at you.", TRUE, ch, 0, victim, TO_VICT);


}

void do_qui(struct char_data *ch, char *argument, int cmd)
{
     send_to_char("You have to write quit - no less, to quit!\n\r",ch);
     return;
}

void do_quit(struct char_data *ch, char *argument, int cmd)
{
  struct obj_data *obj,*next_item;
  struct descriptor_data *d;
  int i;
  int wearing = FALSE;
  char buf[MAX_STRING_LENGTH];
  OBJ *tmp_obj;

  void die(struct char_data *ch);
  void strip_char(CHAR *ch);
  void stop_riding(struct char_data *ch,struct char_data *vict);

  if(IS_NPC(ch) || !ch->desc)
    return;
  for (i=0;i<MAX_WEAR;i++)
    if(ch->equipment[i]) wearing = TRUE;

  if(GET_POS(ch) == POSITION_FIGHTING) {
    send_to_char("No way! You are fighting.\n\r", ch);
    return;
  }

  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) && !IS_NPC(ch) &&
     !ch->specials.death_timer &&
     !IS_SET(ch->specials.affected_by2,AFF_SEVERED)) {
    sprintf(buf,"\n\rThe Dungeonmaster (chaos) [ ** %s wimps out at %s ** ]\n\r\n\r",
            GET_NAME(ch),world[CHAR_REAL_ROOM(ch)].name);
    for(d = descriptor_list; d; d = d->next) {
      if(d->character && (d->character != ch) && !d->connected
         && ( (!IS_SET(d->character->specials.pflag, PLR_NOSHOUT)
         && IS_SET(d->character->specials.pflag, PLR_CHAOS) )
         || d->original )) {
        COLOR(d->character,15);
        send_to_char(buf, d->character);
        ENDCOLOR(d->character);
      }
    }
    die(ch);
    return;
  }

  if((ch->carrying || wearing) && IS_MORTAL(ch))
    {
      send_to_char("You are carrying eq you would lose if you quit.\n\r", ch);
      return;
    }

  if(ch->specials.riding) {
    stop_follower(ch->specials.riding);
    stop_riding(ch,ch->specials.riding);
  }

  ch->ver3.time_to_quest=MAX(ch->ver3.time_to_quest - 40, 5);
  if(ch->quest_status==QUEST_FAILED) {
    printf_to_char(ch,"You have failed your quest, you can start another in %d ticks.\n\r",ch->ver3.time_to_quest);
  }
  if(ch->quest_status==QUEST_RUNNING || ch->quest_status==QUEST_COMPLETED) {
    printf_to_char(ch,"Your quest has been automatically ended, you can start another in %d ticks.\n\r",ch->ver3.time_to_quest);
  }
  ch->questgiver=0;
  if(ch->questobj)
  {
    if(V_OBJ(ch->questobj) == 35)
    {
      for(tmp_obj = object_list; tmp_obj; tmp_obj = tmp_obj->next)
      {
        if(V_OBJ(tmp_obj) != 35) continue; //not a questcard? skip
        if(OBJ_SPEC(tmp_obj) != ch->ver3.id) continue; //not the char's questcard? skip
        extract_obj(tmp_obj);
      }
    }
    else
      ch->questobj->owned_by=0;
  }
  ch->questobj=0;
  if(ch->questmob) ch->questmob->questowner=0;
  ch->questmob=0;
  ch->quest_status=QUEST_NONE;
  ch->quest_level=0;

  if( (GET_LEVEL(ch) < LEVEL_IMM) && IS_SET(ch->specials.pflag, PLR_QUEST)) REMOVE_BIT(ch->specials.pflag, PLR_QUEST);
  /* Ranger - June 96 */
  if( (GET_LEVEL(ch) < LEVEL_IMM) && IS_SET(ch->specials.pflag, PLR_QUIET)) REMOVE_BIT(ch->specials.pflag, PLR_QUIET);

  if(GET_POS(ch) < POSITION_STUNNED) {
    send_to_char("You die before your time!\n\r", ch);
    die(ch);
    return;
  }


  GET_POS(ch) = POSITION_STANDING;

  act("Goodbye, friend.. Come back soon!", FALSE, ch, 0, 0, TO_CHAR);
  act("$n has left the game.", TRUE, ch,0,0,TO_ROOM);
  if(IS_MORTAL(ch)) {
    save_char(ch, NOWHERE);
    extract_char(ch);
  }
  else {
    save_char(ch, NOWHERE);
    for (obj=ch->carrying; obj; obj=next_item)  {
      next_item = obj->next_content;
      extract_obj(obj);
    }
    for (i=0;i<MAX_WEAR;i++)
    if(ch->equipment[i]) extract_obj(unequip_char(ch,i));
    extract_char(ch);
  }
}

void do_withdraw(struct char_data *ch, char *argument, int cmd)
{
     send_to_char("Sorry you can't do that here.\n\r",ch);
}

void do_balance(struct char_data *ch, char *argument, int cmd)
{
     send_to_char("Sorry you can't do that here.\n\r",ch);
}

void do_deposit(struct char_data *ch, char *argument, int cmd)
{
     send_to_char("Sorry you can't do that here.\n\r",ch);
}

void do_wimpy(struct char_data *ch, char *argument, int cmd)
{
        int number;
        char buf[100];

        if(IS_NPC(ch))
                return;

        if(!(*argument))
        {
                if(ch->new.wimpy != 0)
                {
                        ch->new.wimpy = 0;
                        send_to_char("You will not wimpy anymore.", ch);
                        send_to_char("\n\r", ch);
                }
                else
                {
                        ch->new.wimpy = GET_MAX_HIT(ch)/5;
               sprintf(buf, "Wimpy set to %d.", ch->new.wimpy);
                        send_to_char(buf, ch);
                        send_to_char("\n\r", ch);
                }
                return ;
        }
        for (; *argument == ' '; argument++);
        if(isdigit(*argument))
        {
                number = atoi(argument);
                if(number <= GET_MAX_HIT(ch))
                {
                        ch->new.wimpy = number;
                        sprintf(buf, "Wimpy set to %d.", number);
                        send_to_char(buf, ch);
                        send_to_char("\n\r", ch);
                }
                else
                {
                        send_to_char("You can't set your wimpy higher than your max hp.", ch);
                        send_to_char("\n\r", ch);
                }
        }
        else
        {
                sprintf(buf, "%s", argument);
                send_to_char(buf, ch);
                send_to_char("You need to supply a number for wimpy.", ch);
                send_to_char("\n\r", ch);
        }
}

void do_bleed(struct char_data *ch, char *argument, int cmd) {
  int number;
  char buf[100];

  if(IS_NPC(ch)) return;

  if(!(*argument)) {
    if(ch->ver3.bleed_limit) {
      ch->ver3.bleed_limit = 0;
      send_to_char("Bleed limit reset to a default of 1/5 max hit points.\n\r", ch);
    }
    return ;
  }

  for(; *argument == ' '; argument++);
  if(isdigit(*argument)) {
    number = atoi(argument);
    if(number<=GET_MAX_HIT(ch)) {
      ch->ver3.bleed_limit = number;
      sprintf(buf, "Bleed limit set to %d.\n\r", number);
      send_to_char(buf, ch);
    }
    else {
      send_to_char("You can't set your bleed limit higher than your max hp.\n\r", ch);
    }
    return;
  }

  send_to_char("You need to supply a number for your bleed limit.\n\r", ch);
}

void do_save(struct char_data *ch, char *argument, int cmd)
{
  char buf[255];

  if(IS_NPC(ch) || !ch->desc)
    return;
  if(strlen(argument)&& GET_LEVEL(ch)>LEVEL_IMM)
     do_create_save(ch, argument);
  if(cmd !=0) {
  sprintf(buf, "Saving %s.\n\r", GET_NAME(ch));
  send_to_char(buf, ch);
  }
  save_char(ch, NOWHERE);
}

void do_not_here(struct char_data *ch, char *argument, int cmd)
{
     send_to_char("Sorry, but you cannot do that here!\n\r",ch);
}
void do_unknown(struct char_data *ch, char *argument, int cmd)
{
      send_to_char(unknownCMD[number(0, 48)], ch);
}

void do_sneak(struct char_data *ch, char *argument, int cmd)
{
  AFF af;
  int check = 0;

  if (!GET_SKILLS(ch))
    return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_THIEF) &&
      (GET_CLASS(ch) != CLASS_NINJA) &&
      (GET_CLASS(ch) != CLASS_ANTI_PALADIN)) {
    send_to_char("You don't know this skill.\n\r", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_SNEAK)) {
    affect_from_char(ch, SKILL_SNEAK);
    send_to_char("You no longer sneak about in the shadows.\n\r", ch);
    return;
  }

  send_to_char("Ok, you'll try to move silently for a while.\n\r", ch);

  check = (number(1, 101) - dex_app_skill[GET_DEX(ch)].sneak);

  if (affected_by_spell(ch, SPELL_BLUR)) {
    check -= (GET_LEVEL(ch) / 10);
  }

  if (check > GET_LEARNED(ch, SKILL_SNEAK)) {
    send_to_char("You don't feel very sneaky right now.\n\r", ch);
    return;
  }
  else {
    af.type       = SKILL_SNEAK;
    af.duration   = -1;
    af.modifier   = 0;
    af.location   = 0;
    af.bitvector  = AFF_SNEAK;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);

    send_to_char("You slip into the shadows and begin sneaking.\n\r", ch);
    return;
  }
}

void do_hide(struct char_data *ch, char *argument, int cmd)
{
  int check = 0;

  if (!GET_SKILLS(ch)) return;

  if (IS_MORTAL(ch) &&
      (GET_CLASS(ch) != CLASS_THIEF) &&
      (GET_CLASS(ch) != CLASS_NINJA) &&
      (GET_CLASS(ch) != CLASS_ANTI_PALADIN)) {
    send_to_char("You don't know this skill.\n\r", ch);
    return;
  }

  if (IS_AFFECTED(ch, AFF_HIDE)) {
    /* This will never actually get called, due to the way hide works. */
    REMOVE_BIT(GET_AFF(ch), AFF_HIDE);
    send_to_char("You are no longer hidden.\n\r", ch);
    return;
  }

  send_to_char("You attempt to hide yourself.\n\r", ch);

  check = (number(1, 101) - dex_app_skill[GET_DEX(ch)].hide);

  if (affected_by_spell(ch, SPELL_BLUR)) {
    check -= (GET_LEVEL(ch) / 10);
  }

  if (check > GET_LEARNED(ch, SKILL_HIDE)) {
    send_to_char("You failed to hide.\n\r", ch);
    return;
  }
  else {
    SET_BIT(GET_AFF(ch), AFF_HIDE);
    send_to_char("You are now hidden.\n\r", ch);
    return;
  }
}

void do_butcher(struct char_data *ch, char *argument, int cmd) {
  struct obj_data *obj, *steak;
  char obj_name[240],steak_name[240],buf[MIL];
  int per;

  if ((GET_CLASS(ch) != CLASS_NOMAD) &&
      (GET_LEVEL(ch) < LEVEL_IMM)) {
    send_to_char("You don't know this skill.\n\r", ch);
    return;
  }

  argument = one_argument(argument, obj_name);
  if(!(obj = get_obj_in_list_vis(ch, obj_name, world[CHAR_REAL_ROOM(ch)].contents))){
    send_to_char("Butcher what?\n\r", ch);
    return;
  }
  else {
    per = number(1, 101);
    if(!IS_NPC(ch) && per > ch->skills[SKILL_BUTCHER].learned) {
      send_to_char("You failed.\n\r", ch);
      return;
    }

    if(!(GET_ITEM_TYPE(obj) == ITEM_CONTAINER) &&
             (obj->obj_flags.value[3] != 1)) {
               send_to_char("You failed.\n\r", ch);
               return;
    }
    else {
      if(obj->obj_flags.cost==PC_STATUE || obj->obj_flags.cost==NPC_STATUE) {
        send_to_char("You failed.\n\r", ch);
        return;
      }
      if(obj->obj_flags.cost==PC_CORPSE && obj->contains) {
        send_to_char("The corpse has something in it.\n\r", ch);
        return;
      }
      /* Empty corpse of contents */
      empty_container(obj);
      send_to_char("You make a tasty steak from the corpse.\n\r", ch);
      act("$n makes a tasty steak from the corpse.", TRUE, ch, 0,0, TO_ROOM);

      if((GET_CLASS(ch)==CLASS_NOMAD) && (ch->skills[SKILL_BUTCHER].learned < 85))
        ch->skills[SKILL_BUTCHER].learned += 2;

      steak = read_object(0, REAL);

      if(obj->description) {
        half_chop(obj->description,buf,MIL,steak_name,240);
        sprintf(buf,"steak %s",steak_name);
        steak->description = str_dup(buf);
      }

      if(obj->short_description) {
        half_chop(obj->short_description,buf,MIL,steak_name,240);
        sprintf(buf,"Steak %s",steak_name);
        steak->short_description = str_dup(buf);
      }

      obj_to_room(steak, CHAR_REAL_ROOM(ch));
      extract_obj(obj);
    }
  }
}

void do_steal(struct char_data *ch, char *argument, int cmd) {
  struct char_data *victim;
  struct obj_data *obj;
  char victim_name[240];
  char obj_name[240];
  char buf[240],buf2[MAX_INPUT_LENGTH];
  int percent;
  int gold, eq_pos;
  bool ohoh = FALSE;

  if(!ch->skills) return;

  if ((GET_CLASS(ch) != CLASS_THIEF) &&
      (GET_CLASS(ch) != CLASS_ANTI_PALADIN) &&
      IS_MORTAL(ch) && 
      !CHAOSMODE) /* if CHAOSMODE, allow anyone to steal, for now */
  { 
    send_to_char("You don't know this skill.\n\r", ch);
    return;
  }

  argument = one_argument(argument, obj_name);
  one_argument(argument, victim_name);

  if(!(victim = get_char_room_vis(ch, victim_name))) {
    send_to_char("Steal what from who?\n\r", ch);
    return;
  } else if(victim == ch) {
    send_to_char("Come on now, that's rather stupid!\n\r", ch);
    return;
  }

  if((GET_EXP(ch) < 1250) && (!IS_NPC(victim))) {
    send_to_char("Due to misuse of steal, you can't steal from other players\n\r", ch);
    send_to_char("unless you got at least 1,250 experience points.\n\r", ch);
    return;
  }

  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) &&
     GET_LEVEL(ch) < LEVEL_IMM && (!CHAOSMODE))  {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  };

  if(!IS_NPC(victim) /*&& AWAKE(victim)*/ && (GET_LEVEL(ch) < LEVEL_DEI)
     && !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, ARENA)) {
    if(!IS_SET(ch->specials.pflag, PLR_THIEF)) {
      SET_BIT(ch->specials.pflag, PLR_THIEF);
      sprintf(buf,"PLRINFO: %s just tried to steal from %s; Thief flag set. (Room %d)",
              GET_NAME(ch),GET_NAME(victim),world[CHAR_REAL_ROOM(ch)].number);
      wizlog(buf,LEVEL_SUP,4);
      log_f("%s",buf);
    }
    send_to_char("You're a thief!\n\r", ch);
    return;
  }

  if(strstr(obj_name,"token") && GET_LEVEL(ch)<LEVEL_IMP) {
    act("You can't tell if $E has that item.",FALSE,ch,0,victim,TO_CHAR);
    return;
  }

  /* 101% is a complete failure */
  percent=number(1,101) - dex_app_skill[GET_DEX(ch)].p_pocket;

  if (affected_by_spell(ch, SKILL_VEHEMENCE))
  {
    percent -= 5 + (GET_DEX_APP(ch) / 2);
  }

  if(GET_POS(victim) < POSITION_SLEEPING)
    percent = -1; /* ALWAYS SUCCESS */

  if(GET_LEVEL(victim)>LEVEL_IMM) /* NO NO With Imp's and Shopkeepers! */
    percent = 201; /* Failure */

  if(IS_AFFECTED(victim, AFF_SPHERE) || IS_AFFECTED(victim, AFF_INVUL))
    percent = 201; /* Failure */

  if(GET_LEVEL(ch)>LEVEL_SUP && IS_SET(ch->new.imm_flags, WIZ_ACTIVE))
    percent = -1; /* Active IMP can steal anything */

  if(IS_NPC(victim) && IS_SET(victim->specials.immune2,IMMUNE_STEAL))
    percent = 201; /* Failure */

  if(str_cmp(obj_name, "coins") && str_cmp(obj_name,"gold")) 
  {
    if(!(obj = get_obj_in_list_vis(victim, obj_name, victim->carrying))) 
    {
      for(eq_pos = 0; (eq_pos < MAX_WEAR); eq_pos++)
      {
        if(victim->equipment[eq_pos] &&
           (isname(obj_name, OBJ_NAME(victim->equipment[eq_pos]))) &&
           CAN_SEE_OBJ(ch,victim->equipment[eq_pos])) 
        {
           obj = victim->equipment[eq_pos];
           break;
        }
      }

      if(!obj) 
      {
        act("You can't tell if $E has that item.",FALSE,ch,0,victim,TO_CHAR);
        return;
      } 
	  else /* It is equipment */
	  { 
        /*if((GET_POS(victim) > POSITION_STUNNED)) 
		{*/
          send_to_char("Steal the equipment now? Impossible!\n\r", ch);
          return;
        /*} 
		else 
		{
          act("You unequip $p and steal it.",FALSE, ch, obj ,0, TO_CHAR);
          act("$n steals $p from $N.",FALSE,ch,obj,victim,TO_NOTVICT);
          obj_to_char(unequip_char(victim, eq_pos), ch);
        }*/
      }
    }
    else /* obj found in inventory */
    {  
      percent += GETOBJ_WEIGHT(obj); /* Make heavy harder */

      if(AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned)) 
      {
        ohoh = TRUE;
        act("Oops..", FALSE, ch,0,0,TO_CHAR);
        act("$n tried to steal something from you!",FALSE,ch,0,victim,TO_VICT);
        act("$n tries to steal something from $N.", TRUE, ch, 0, victim, TO_NOTVICT);
      }
      else /* Steal the item */
      { 
        if((percent > ch->skills[SKILL_STEAL].learned) ||
           (obj->obj_flags.type_flag==ITEM_SC_TOKEN)) 
        {
          send_to_char("You failed.\n\r", ch);
          WAIT_STATE(ch, PULSE_VIOLENCE);
          return;
        }

        if(CAN_CARRY_OBJ(ch,obj)) 
        {
          sprintf(buf2,"%s steals %s from %s",GET_NAME(ch),OBJ_SHORT(obj),GET_NAME(victim));
          wizlog(buf2, GET_LEVEL(ch)+1, 4);
          log_f("%s",buf2);
          obj_from_char(obj);
          obj_to_char(obj, ch);

          if(IS_SET(obj->obj_flags.extra_flags, ITEM_CLONE)) 
          {
            act("The cloned $p falls to pieces as you steal it.",FALSE,ch,obj,0,TO_CHAR);
            act("A cloned $p falls to pieces as $n steals it.",FALSE,ch,obj,0,TO_ROOM);
            extract_obj(obj_from_char(obj));
          }
          else
            send_to_char("Got it!\n\r", ch);
        }
        else
          send_to_char("You cannot carry that much.\n\r", ch);
      }
    }
  } else { /* Steal some coins */
    if(AWAKE(victim) && (percent > ch->skills[SKILL_STEAL].learned)) {
      ohoh = TRUE;
      act("Oops..", FALSE, ch,0,0,TO_CHAR);
      act("You discover that $n has $s hands in your wallet.",FALSE,ch,0,victim,TO_VICT);
      act("$n tries to steal gold from $N.",TRUE, ch, 0, victim, TO_NOTVICT);
    } else {
      /* Steal some gold coins */
      gold = (int) ((GET_GOLD(victim)*number(1,10))/100);
      if(GET_LEVEL(ch) > LEVEL_WIZ)
      {gold = MIN(100000, gold);}
      else { gold = MIN(1782, gold);}
      if(gold > 0) 
      {
        GET_GOLD(ch) += gold;
        GET_GOLD(victim) -= gold;
        sprintf(buf, "Bingo! You got %d gold coins.\n\r", gold);
        send_to_char(buf, ch);
      }
      else 
      {
        send_to_char("You couldn't get any gold...\n\r", ch);
      }
    }
  }

  save_char(ch, NOWHERE);
  save_char(victim, NOWHERE);
  WAIT_STATE(ch, PULSE_VIOLENCE);

  if(ohoh && IS_NPC(victim) && AWAKE(victim)) {
    if(IS_SET(victim->specials.act, ACT_NICE_THIEF)) {
      sprintf(buf, "%s is a bloody thief.", GET_NAME(ch));
      do_shout(victim, buf, 0);
      log_s(buf);
      send_to_char("Don't you ever do that again!\n\r", ch);
    } else {
      hit(victim, ch, TYPE_UNDEFINED);
    }
  }
}

void list_skills_to_prac(CHAR *ch);
void list_spells_to_prac(CHAR *ch,int listall); /* In spec.midgaard.c */
void do_practice(CHAR *ch, char *arg, int cmd)
{
  int showSpells = FALSE;
  int showSkills = FALSE;

  if (!ch->desc || !ch->skills) return;

  if (*arg)
  {
    send_to_char("`iYou can't practice here.`q\n\r", ch);
    return;
  }

  send_to_char("`iYou are currently practiced in these areas:`q\n\r\n\r", ch);

  switch (GET_CLASS(ch))
  {
    case CLASS_MAGIC_USER:
      showSpells = TRUE;
      break;

    case CLASS_CLERIC:
      showSpells = TRUE;

      if (GET_LEVEL(ch) >= 35 || check_sc_access(ch, SKILL_MEDITATE)) {
        showSkills = TRUE;
      }
      break;

    case CLASS_NINJA:
    case CLASS_PALADIN:
    case CLASS_ANTI_PALADIN:
    case CLASS_AVATAR:
    case CLASS_BARD:
    case CLASS_COMMANDO:
      showSkills = TRUE;
      showSpells = TRUE;
      break;

    case CLASS_THIEF:
    case CLASS_WARRIOR:
    case CLASS_NOMAD:
      showSkills = TRUE;
      break;
  }

  if (showSkills) {
    list_skills_to_prac(ch);
  }

  if (showSpells && showSkills) {
    send_to_char("\n\r", ch);
  }

  if (showSpells) {
    list_spells_to_prac(ch, TRUE);
  }
}

void do_useidea(struct char_data *ch, char *argument, int cmd)
{
/*     FILE *fl;
     char str[MAX_STRING_LENGTH];*/

   send_to_char("Please use the idea board, located west of the altar.\n\r",ch);
   return;

/*     if(IS_NPC(ch))
     {
          send_to_char("Monsters can't have ideas - Go away.\n\r", ch);
          return;
     }

     for (; isspace(*argument); argument++);

     if(!*argument)
     {
          send_to_char("That doesn't sound like a good idea to me.. Sorry.\n\r",
               ch);
          return;
     }

     if(!(fl = fopen(IDEA_FILE, "a")))
     {
          log_f ("do_idea");
          send_to_char("Could not open the idea-file.\n\r", ch);
          return;
     }

     sprintf(str, "**%s: %s\n", GET_NAME(ch), argument);
     fputs(str, fl);
     fclose(fl);
     send_to_char("Ok, thanks.\n\r", ch);*/
}

void do_typo(struct char_data *ch, char *argument, int cmd) {
     FILE *fl;
     char str[MAX_STRING_LENGTH];

     if(IS_NPC(ch)) {
       send_to_char("Monsters can't spell - leave me alone.\n\r", ch);
       return;
     }

     for (; isspace(*argument); argument++);

     if(!*argument) {
       send_to_char("You have a typo to report?\n\r",ch);
       return;
     }

     if(!(fl = fopen(TYPO_FILE, "a"))) {
       log_f ("do_typo");
       send_to_char("Could not open the typo-file.\n\r", ch);
       return;
     }

     sprintf(str, "**%s[%d]: %s\n",
          GET_NAME(ch), world[CHAR_REAL_ROOM(ch)].number, argument);
     fputs(str, fl);
     fclose(fl);
     send_to_char("Ok, thanks.\n\r", ch);
}

/*void do_bug(struct char_data *ch, char *argument, int cmd)
{
     FILE *fl;
     char str[MAX_STRING_LENGTH];

        send_to_char("Please use the idea board west of the altar.\n\r",ch);
        return;

     if(IS_NPC(ch))
     {
          send_to_char("You are a monster! Bug off!\n\r", ch);
          return;
     }

 for (; isspace(*argument); argument++);

     if(!*argument)
     {
          send_to_char("Pardon?\n\r",
               ch);
          return;
     }

     if(!(fl = fopen(BUG_FILE, "a")))
     {
          log_f ("do_bug");
          send_to_char("Could not open the bug-file.\n\r", ch);
          return;
     }

     sprintf(str, "**%s[%d]: %s\n",
          GET_NAME(ch), world[CHAR_REAL_ROOM(ch)].number, argument);
     fputs(str, fl);
     fclose(fl);
     send_to_char("Ok, it'll be checked out.\n\r", ch);
}*/

void do_brief(struct char_data *ch, char *argument, int cmd) {
  char arg[MAX_STRING_LENGTH];

  if (IS_NPC(ch)) return;

  one_argument(argument, arg);

  if (*arg) {
    if (is_abbrev(arg, "all")) {
      if (IS_SET(GET_PFLAG(ch), PLR_BRIEF)) {
        send_to_char("All brief modes off.\n\r", ch);
        REMOVE_BIT(GET_PFLAG(ch), PLR_BRIEF);
        REMOVE_BIT(GET_PFLAG(ch), PLR_SECTOR_BRIEF);
        REMOVE_BIT(GET_PFLAG(ch), PLR_SUPERBRF);
        REMOVE_BIT(GET_PFLAG(ch), PLR_TAGBRF);
        REMOVE_BIT(GET_PFLAG(ch), PLR_FIGHTBRF);
      }
      else {
        send_to_char("All brief modes on.\n\r", ch);
        SET_BIT(GET_PFLAG(ch), PLR_BRIEF);
        SET_BIT(GET_PFLAG(ch), PLR_SECTOR_BRIEF);
        SET_BIT(GET_PFLAG(ch), PLR_SUPERBRF);
        SET_BIT(GET_PFLAG(ch), PLR_TAGBRF);
        SET_BIT(GET_PFLAG(ch), PLR_FIGHTBRF);
      }
      return;
    }

    if (is_abbrev(arg, "normal")) {
      if (IS_SET(GET_PFLAG(ch), PLR_BRIEF)) {
        send_to_char("Brief mode off.\n\r", ch);
        REMOVE_BIT(GET_PFLAG(ch), PLR_BRIEF);
      }
      else {
        send_to_char("Brief mode on.\n\r", ch);
        SET_BIT(GET_PFLAG(ch), PLR_BRIEF);
      }
      return;
    }

    if (is_abbrev(arg, "sector")) {
      if (IS_SET(GET_PFLAG(ch), PLR_SECTOR_BRIEF)) {
        send_to_char("Sector brief mode off.\n\r", ch);
        REMOVE_BIT(GET_PFLAG(ch), PLR_SECTOR_BRIEF);
      }
      else {
        send_to_char("Sector brief mode on.\n\r", ch);
        SET_BIT(GET_PFLAG(ch), PLR_SECTOR_BRIEF);
      }
      return;
    }

    if (is_abbrev(arg, "super")) {
      if (IS_SET(GET_PFLAG(ch), PLR_SUPERBRF)) {
        send_to_char("Super brief mode off.\n\r", ch);
        REMOVE_BIT(GET_PFLAG(ch), PLR_SUPERBRF);
      }
      else {
        send_to_char("Super brief mode on.\n\r", ch);
        SET_BIT(GET_PFLAG(ch), PLR_SUPERBRF);
      }
      return;
    }

    if (is_abbrev(arg, "tagline")) {
      if (IS_SET(GET_PFLAG(ch), PLR_TAGBRF)) {
        send_to_char("Tagline brief mode off.\n\r", ch);
        REMOVE_BIT(GET_PFLAG(ch), PLR_TAGBRF);
      }
      else {
        send_to_char("Tagline brief mode on.\n\r", ch);
        SET_BIT(GET_PFLAG(ch), PLR_TAGBRF);
      }
      return;
    }

    if (is_abbrev(arg, "fight")) {
      if (IS_SET(GET_PFLAG(ch), PLR_FIGHTBRF)) {
        send_to_char("Fight brief mode off.\n\r", ch);
        REMOVE_BIT(GET_PFLAG(ch), PLR_FIGHTBRF);
      }
      else {
        send_to_char("Fight brief mode on.\n\r", ch);
        SET_BIT(GET_PFLAG(ch), PLR_FIGHTBRF);
      }
      return;
    }

    if (is_abbrev(arg, "list")) {
      if (IS_SET(GET_PFLAG(ch), PLR_BRIEF)) {
        send_to_char("Brief mode:         ON\n\r", ch);
      }
      else {
        send_to_char("Brief mode:         OFF\n\r", ch);
      }
      if (IS_SET(GET_PFLAG(ch), PLR_SECTOR_BRIEF)) {
        send_to_char("Sector brief mode:  ON\n\r", ch);
      }
      else {
        send_to_char("Sector brief mode:  OFF\n\r", ch);
      }
      if (IS_SET(GET_PFLAG(ch), PLR_SUPERBRF)) {
        send_to_char("Super brief mode:   ON\n\r", ch);
      }
      else {
        send_to_char("Super brief mode:   OFF\n\r", ch);
      }
      if (IS_SET(GET_PFLAG(ch), PLR_TAGBRF)) {
        send_to_char("Tagline brief mode: ON\n\r", ch);
      }
      else {
        send_to_char("Tagline brief mode: OFF\n\r", ch);
      }
      if (IS_SET(GET_PFLAG(ch), PLR_FIGHTBRF)) {
        send_to_char("Fight brief mode:   ON\n\r", ch);
      }
      else {
        send_to_char("Fight brief mode:   OFF\n\r", ch);
      }
      return;
    }
  }

  send_to_char("Usage: brief normal  - You will not see room descriptions.\n\r", ch);
  send_to_char("             sector  - You will not see room sector types.\n\r", ch);
  send_to_char("             super   - You will not see players/NPCs in rooms, or their movement.\n\r", ch);
  send_to_char("             tagline - You will not see player/NPC taglines.\n\r", ch);
  send_to_char("             fight   - You will not see verbose hit messages for other players.\n\r", ch);
  send_to_char("             list    - Shows your active brief settings.\n\r", ch);
  send_to_char("             all     - Toggles all breif modes on or off.\n\r\n\r", ch);
}

void do_compact(struct char_data *ch, char *argument, int cmd)
{
     if(IS_NPC(ch))
          return;

     if(IS_SET(ch->specials.pflag, PLR_COMPACT))
     {
          send_to_char("You are now in the uncompacted mode.\n\r", ch);
          REMOVE_BIT(ch->specials.pflag, PLR_COMPACT);
     }
     else
     {
          send_to_char("You are now in compact mode.\n\r", ch);
          SET_BIT(ch->specials.pflag, PLR_COMPACT);
     }
}

void do_refollow(struct char_data *ch, char *argument, int cmd)
{
  char name[256];
  struct char_data *victim;
  struct follow_type *f;
  bool found;

  one_argument(argument, name);

  if(!*name) {
    send_to_char("Who do you want to refollow you?\n\r", ch);
    return;
  } else {
    if(!(victim = get_char_room_vis(ch, name))) {
      send_to_char("No one here by that name.\n\r", ch);
    } else {
      if(ch->master) {
     act("You can not enroll group members without being the leader of a group.",
         FALSE, ch, 0,0, TO_CHAR);
     return;
      }
      found = FALSE;

      if(victim == ch) {
     found = FALSE;
     send_to_char("But you are the group leader?\n\r", ch);
     return;
      } else {
     for (f=ch->followers; f; f=f->next) {
       if(f->follower == victim) {
         found = TRUE;
         break;
       }
     }
      }
      if(found) {
     if(victim->master)
       stop_follower(victim);
     add_follower(victim, ch);
     if(!IS_AFFECTED(victim, AFF_GROUP))
       SET_BIT(victim->specials.affected_by, AFF_GROUP);
      }
    }
}
}

void do_unfollow(struct char_data *ch, char *argument, int cmd)
{
  char name[256];
  struct char_data *victim;
  struct follow_type *f;
  bool found;

  one_argument(argument, name);

  if(!*name) {
    send_to_char("Who do you want to stop following you?\n\r", ch);
    return;
  }

  if(!(victim = get_char_room_vis(ch, name))) {
    send_to_char("No one here by that name.\n\r", ch);
    return;
  }

  found = FALSE;

  if(victim == ch) {
    send_to_char("Stop following yourself?\n\r", ch);
    return;
  }

  if(IS_NPC(victim)) {
    send_to_char("You can only stop other players from following you.\n\r",ch);
    return;
  }

  for(f=ch->followers; f; f=f->next) {
    if(f->follower == victim) {
      found = TRUE;
      break;
    }
  }

  if(found) {
    if(victim->master)
      stop_follower(victim);
    if(IS_AFFECTED(victim, AFF_GROUP))
      REMOVE_BIT(victim->specials.affected_by, AFF_GROUP);
  }
  else {
    act("$N isn't following you.",0,ch,0,victim,TO_CHAR);
  }
}

char *head_types[] = {
        "--",
        "Mu",
        "Cl",
        "Th",
        "Wa",
        "Ni",
        "No",
        "Pa",
        "Ap",
        "Ap",
        "Ba",
        "Co",
        };

void do_group(struct char_data *ch, char *argument, int cmd)
{
     char name[256];
     char buffer[MAX_STRING_LENGTH];
     struct char_data *victim, *k;
     struct follow_type *f;
     bool found;

  if(CHAOSMODE) {
    send_to_char("You feel too chaotic to join in a group.\n\r",ch);
    return;
  }

     one_argument(argument, name);

     if(!*name) {
          if(!IS_AFFECTED(ch, AFF_GROUP)) {
               send_to_char("But you are a member of no group?!\n\r", ch);
          } else {
               send_to_char("Your group consists of:\n\r", ch);
               if(ch->master)
                    k = ch->master;
               else
                    k = ch;

               if(IS_AFFECTED(k, AFF_GROUP))
{
               if(!IS_NPC(k))
                 sprintf(buffer, "[%2d][%2d %2s][%3d(%3d)h %3d(%3d)m %3d(%3d)v] %s (Leader)\n\r",
                    k->specials.timer,
                    GET_LEVEL(k), head_types[k->player.class],
                    GET_HIT(k), GET_MAX_HIT(k),
                    GET_MANA(k), GET_MAX_MANA(k),
                    GET_MOVE(k), GET_MAX_MOVE(k),
                    GET_NAME(k));
               else
                 sprintf(buffer, "[%2d][%2d   ][%3d(%3d)h %3d(%3d)m %3d(%3d)v] %s (Leader)\n\r",
                    k->specials.timer,
                    GET_LEVEL(k),
                    GET_HIT(k), GET_MAX_HIT(k),
                    GET_MANA(k), GET_MAX_MANA(k),
                    GET_MOVE(k), GET_MAX_MOVE(k),
                    GET_NAME(k));
                    send_to_char(buffer, ch);
}
               for(f=k->followers; f; f=f->next)
                    if(IS_AFFECTED(f->follower, AFF_GROUP))
{
                 if(!IS_NPC(f->follower))
                     sprintf(buffer, "[%2d][%2d %2s][%3d(%3d)h %3d(%3d)m %3d(%3d)v] %s\n\r",
                         f->follower->specials.timer,
                         GET_LEVEL(f->follower),
                         head_types[f->follower->player.class],
                         GET_HIT(f->follower),
                         GET_MAX_HIT(f->follower),
                         GET_MANA(f->follower),
                         GET_MAX_MANA(f->follower),
                         GET_MOVE(f->follower),
                         GET_MAX_MOVE(f->follower),
                         GET_NAME(f->follower));
                 else
                     sprintf(buffer, "[%2d][%2d   ][%3d(%3d)h %3d(%3d)m %3d(%3d)v] %s\n\r",
                         f->follower->specials.timer,
                         GET_LEVEL(f->follower),
                         GET_HIT(f->follower),
                         GET_MAX_HIT(f->follower),
                         GET_MANA(f->follower),
                         GET_MAX_MANA(f->follower),
                         GET_MOVE(f->follower),
                         GET_MAX_MOVE(f->follower),
                         GET_NAME(f->follower));
                      send_to_char(buffer, ch);
}
          }

          return;
     }

     if(!str_cmp(name, "all"))
     {     if(ch->master)
          {     act("You are not the group leader.", FALSE, ch, 0,0,TO_CHAR);
               return;
          }

          if(!IS_AFFECTED(ch, AFF_GROUP))
          {     act("You are now a group member.", FALSE, ch,0,0,TO_CHAR);
               SET_BIT(ch->specials.affected_by, AFF_GROUP);
          }
               if(ch->master)
                    k = ch->master;
               else
                    k = ch;

          for (f=k->followers;f; f=f->next)
          {     if(!IS_AFFECTED(f->follower, AFF_GROUP) &&
                  (CHAR_REAL_ROOM(ch) == CHAR_REAL_ROOM(f->follower)) ) {
                           /* Check for player trying to group a mount - Ranger April 96 */
                           if(!IS_SET(f->follower->specials.act, ACT_MOUNT)) {
                             act("$n is now a group member.", FALSE, f->follower,0,0,TO_ROOM);
                    act("You are now a group member.", FALSE, f->follower,0,0,TO_CHAR);
                             SET_BIT(f->follower->specials.affected_by, AFF_GROUP);
                          }
                           else act("You cannot group a mount.", FALSE, ch, 0,0,TO_CHAR);
               }
          }
     }
     else {
     if(!(victim = get_char_room_vis(ch, name))) {
          send_to_char("No one here by that name.\n\r", ch);
     } else {

          if(ch->master) {
               act("You can not enroll group members without being the leader of a group.",
                  FALSE, ch, 0, 0, TO_CHAR);
               return;
          }

          found = FALSE;

          if(victim == ch)
               found = TRUE;
          else {
               for(f=ch->followers; f; f=f->next) {
                    if(f->follower == victim) {
                         found = TRUE;
                         break;
                    }
               }
          }

          if(found) {
               if(IS_AFFECTED(victim, AFF_GROUP)) {
                    act("$n has been kicked out of the group!", FALSE, victim, 0, ch, TO_ROOM);
                    act("You are no longer a member of the group!", FALSE, victim, 0, 0, TO_CHAR);
                    REMOVE_BIT(victim->specials.affected_by, AFF_GROUP);
               } else {
                           /* Check for player trying to group a mount - Ranger April 96 */
                           if(!IS_SET(victim->specials.act, ACT_MOUNT)) {
                             act("$n is now a group member.", FALSE, victim,0,0,TO_ROOM);
                             act("You are now a group member.", FALSE, victim,0,0,TO_CHAR);
                             SET_BIT(victim->specials.affected_by, AFF_GROUP);
                           }
                           else act("You cannot group a mount.", FALSE, ch, 0,0,TO_CHAR);
               }
          } else {
               act("$N must follow you, to enter the group", FALSE, ch, 0, victim, TO_CHAR);
          }
     }
     }
}



void do_quaff(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct obj_data *temp;
  int i;
     bool equipped;

     equipped = FALSE;

  one_argument(argument,buf);

     if(!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))) {
          temp = ch->equipment[HOLD];
          equipped = TRUE;
       if((temp==0) || !isname(buf, OBJ_NAME(temp))) {
               act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
         return;
       }
     }

  if(temp->obj_flags.type_flag!=ITEM_POTION)
  {
    act("You can only quaff potions.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_QUAFF)) {
    send_to_char("Something prevents you from putting the potion to your lips!\n\r",ch);
    return;
  }

  if((GET_COND(ch,QUAFF)>6)||GET_COND(ch,THIRST)>23)
    {
    act("You are too full to drink any more.",FALSE,ch,0,0,TO_CHAR);
    return;
    }

  act("$n quaffs $p.", TRUE, ch, temp, 0, TO_ROOM);
  act("You quaff $p which dissolves.",FALSE,ch,temp,0,TO_CHAR);

        if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_MAGIC))
        {       send_to_char("Your magic has been absorbed by the surroundings.\n\r", ch);
          if(equipped)
               unequip_char(ch, HOLD);

          extract_obj(temp);
                return;
        }

  gain_condition(ch,QUAFF,1);
  gain_condition(ch,THIRST,5);

  for (i=1; i<4; i++)
    if(temp->obj_flags.value[i] >= 1)
      ((*spell_info[temp->obj_flags.value[i]].spell_pointer)
        ((byte) temp->obj_flags.value[0], ch, "", SPELL_TYPE_POTION, ch, 0));

     if(equipped)
          unequip_char(ch, HOLD);

  extract_obj(temp);
}


void do_recite(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct obj_data *scroll, *obj;
  struct char_data *victim;
  int i, bits;
  bool equipped;

  equipped = FALSE;
  obj = 0;
  victim = 0;

  argument = one_argument(argument,buf);

  if(!(scroll = get_obj_in_list_vis(ch,buf,ch->carrying))) {
    scroll = ch->equipment[HOLD];
    equipped = TRUE;
    if((scroll==0) || !isname(buf, OBJ_NAME(scroll))) {
      act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
      return;
    }
  }

  if(scroll->obj_flags.type_flag!=ITEM_SCROLL)
    {
      act("Recite is normally used for scroll's.",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if(*argument) {
    bits = generic_find(argument, FIND_OBJ_INV | FIND_OBJ_ROOM |
               FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &victim, &obj);
    if(bits == 0) {
      send_to_char("No such thing around to recite the scroll on.\n\r", ch);
      return;
    }
  } else {
    victim = ch;
  }

  act("$n recites $p.", TRUE, ch, scroll, 0, TO_ROOM);
  act("You recite $p which dissolves.",FALSE,ch,scroll,0,TO_CHAR);

  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_MAGIC))
    {
      send_to_char("Your magic has been absorbed by the surroundings.\n\r", ch);
      if(equipped)
     unequip_char(ch, HOLD);

      extract_obj(scroll);
      return;
    }

  /* Addition: To fix person targetting spells from crashing ...
     03-12-1995  -Kafka
  */
  if(!victim)
    victim = ch;

  for (i=1; i<4; i++)
    if(scroll->obj_flags.value[i] >= 1)
      ((*spell_info[scroll->obj_flags.value[i]].spell_pointer)
       ((byte) scroll->obj_flags.value[0], ch, "", SPELL_TYPE_SCROLL, victim, obj));

  if(equipped)
    unequip_char(ch, HOLD);

  extract_obj(scroll);
}

void do_use(struct char_data *ch, char *argument, int cmd) {
  char buf[100];
  struct char_data *tmp_char=0;
  struct obj_data *tmp_object=0, *stick=0;
  int bits=0;

  argument = one_argument(argument,buf);

  if(ch->equipment[HOLD] == 0 ||
     !isname(buf, OBJ_NAME(ch->equipment[HOLD]))) {
    act("You do not hold that item in your hand.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  stick = ch->equipment[HOLD];

  if(IS_NPC(ch) &&
     (IS_AFFECTED(ch, AFF_CHARM) || IS_SET(ch->specials.act, ACT_MOUNT))) return;

  if(stick->obj_flags.type_flag == ITEM_STAFF) {
    act("$n taps $p three times on the ground.",TRUE, ch, stick, 0,TO_ROOM);
    act("You tap $p three times on the ground.",FALSE,ch, stick, 0,TO_CHAR);
    if(stick->obj_flags.value[2] > 0) {  /* Are there any charges left? */
      stick->obj_flags.value[2]--;
      if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_MAGIC)) {
        send_to_char("Your magic has been absorbed by the surroundings.\n\r", ch);
        return;
      }
      ((*spell_info[stick->obj_flags.value[3]].spell_pointer)
      ((byte) stick->obj_flags.value[0], ch, "", SPELL_TYPE_STAFF, 0, 0));
    } else {
      send_to_char("The staff seems powerless.\n\r", ch);
    }
  }
  else if(stick->obj_flags.type_flag == ITEM_WAND) {
    one_argument(argument,buf);
    if(*buf) { /*parse the argument, grab the target*/
       bits = generic_find(buf, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
                           FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
    }
    else {
      if((tmp_char=ch->specials.fighting))
        bits = FIND_CHAR_ROOM;
    }
    if(bits) {
      if(bits == FIND_CHAR_ROOM) {
        act("$n point $p at $N.", TRUE, ch, stick, tmp_char, TO_ROOM);
        act("You point $p at $N.",FALSE,ch, stick, tmp_char, TO_CHAR);
      } else {
        act("$n point $p at $P.", TRUE, ch, stick, tmp_object, TO_ROOM);
        act("You point $p at $P.",FALSE,ch, stick, tmp_object, TO_CHAR);
      }
      if(stick->obj_flags.value[2] > 0) { /* Is there any charges left? */
        stick->obj_flags.value[2]--;
         if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_MAGIC)) {
          send_to_char("Your magic has been absorbed by the surroundings.\n\r", ch);
          return;
        }
        ((*spell_info[stick->obj_flags.value[3]].spell_pointer)
        ((byte) stick->obj_flags.value[0], ch, "", SPELL_TYPE_WAND, tmp_char, tmp_object));
      }
      else {
        send_to_char("The wand seems powerless.\n\r", ch);
      }
    }
    else {
      send_to_char("What should the wand be pointed at?\n\r", ch);
    }
  }
  else {
    send_to_char("Use is normally only for a wand or a staff.\n\r", ch);
  }
  WAIT_STATE(ch, PULSE_VIOLENCE);
}

void do_nokill(struct char_data *ch, char *argument, int cmd)
{
        char buf[MAX_INPUT_LENGTH];

        if(IS_NPC(ch))
                return;

        if( GET_LEVEL(ch)<6 && (!CHAOSMODE)) {
           send_to_char("You must be at least level 6.\n\r",ch);
           return;
        } /* Ranger - June 96 */

        one_argument(argument, buf);

        if(!*buf)
                if(IS_SET(ch->specials.pflag, PLR_NOKILL))
                {
                        send_to_char("You can now kill other players.\n\r", ch);
                        REMOVE_BIT(ch->specials.pflag, PLR_NOKILL);
                }
                else
                {
                        send_to_char("From now on, you can't kill other players.\n\r", ch);
                        SET_BIT(ch->specials.pflag, PLR_NOKILL);
                }
        else
        {
                send_to_char("You can't set the flag to another player.\n\r", ch);
        }
}


void do_nosummon(struct char_data *ch, char *argument, int cmd)
{
        char buf[MAX_INPUT_LENGTH];

        if(IS_NPC(ch))
                return;

        one_argument(argument, buf);

        if(!*buf)
                if(IS_SET(ch->specials.pflag, PLR_NOSUMMON))
                {
                        send_to_char("You can now be summoned.\n\r", ch);
                        REMOVE_BIT(ch->specials.pflag, PLR_NOSUMMON);
                }
                else
                {
                        send_to_char("From now on, you can't be summoned.\n\r", ch);
                        SET_BIT(ch->specials.pflag, PLR_NOSUMMON);
                }
        else
        {
                send_to_char("You can't set the flag to another player.\n\r", ch);
        }
}


void do_nomessage(struct char_data *ch, char *argument, int cmd)
{
        char buf[MAX_INPUT_LENGTH];

        if(IS_NPC(ch))
                return;

        one_argument(argument, buf);

        if(!*buf)
                if(IS_SET(ch->specials.pflag, PLR_NOMESSAGE))
                {
                        send_to_char("You can now hear the messages.\n\r", ch);
                        REMOVE_BIT(ch->specials.pflag, PLR_NOMESSAGE);
                }
                else
                {
                        send_to_char("From now on, you can't hear any message.\n\r", ch);
                        SET_BIT(ch->specials.pflag, PLR_NOMESSAGE);
                }
        else
        {
                send_to_char("You can't set the flag to other players.\n\r", ch);
        }
}

void do_throw(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct char_data *tmp_char;
  struct obj_data *tmp_object, *stick;
  int a,b, dam, percent;
  int bits;

  if(!ch->skills) return;

  argument = one_argument(argument,buf);

  if((GET_CLASS(ch) != CLASS_WARRIOR) &&
     (GET_CLASS(ch) != CLASS_THIEF) &&
     (GET_CLASS(ch) != CLASS_NINJA) &&
     (GET_CLASS(ch) != CLASS_NOMAD) &&
     (GET_CLASS(ch) != CLASS_BARD) &&
     (GET_CLASS(ch) != CLASS_COMMANDO) &&
     (GET_CLASS(ch) != CLASS_AVATAR)) {
    if(GET_LEVEL(ch) < LEVEL_IMM) {
      act("Leave this job to the warriors, thieves, ninja or nomads.", FALSE, ch, 0, 0, TO_CHAR);
      return;
    }
  }

  if(ch->equipment[HOLD] == 0 ||
      !isname(buf, OBJ_NAME(ch->equipment[HOLD]))) {
    act("You do not hold that item in your hand.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if(ROOM_SAFE(CHAR_REAL_ROOM(ch))) {
    send_to_char("Behave yourself here please!\n\r", ch);
    return;
  }

  stick = ch->equipment[HOLD];

  if(CAN_WEAR(stick, ITEM_THROW)) {

    if(!ch->specials.fighting) {
      bits = generic_find(argument, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM |
                                FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
    }
    else {
      bits=FIND_CHAR_ROOM;
      tmp_char=ch->specials.fighting;
    }

    if(bits) {
      if(bits == FIND_CHAR_ROOM) {
        /* Shun: Check if target is actually in same room */
        if (CHAR_REAL_ROOM(ch) != CHAR_REAL_ROOM(tmp_char)) {
          send_to_char("Your target isn't here.\n\r", ch);
          return;
        }
        percent = number(1, 101);

        if (affected_by_spell(ch, SPELL_BLUR))
        {
          percent -= (GET_LEVEL(ch) / 10);
        }

        if(percent > ch->skills[SKILL_THROW].learned) {
          act("$n throws $p into the sky.", TRUE, ch, stick, tmp_char, TO_ROOM);
          act("Your throw misses $N and your $p flies into the sky.", FALSE, ch, stick, tmp_char, TO_CHAR);
          act("$n throws $p into the sky.", 0, ch, stick, tmp_char, TO_VICT);
          unequip_char(ch, HOLD);
          extract_obj(stick);
          /*  WAIT_STATE(ch, PULSE_VIOLENCE); */
          return;
        }

        act("$n throws $p at $N.", TRUE, ch, stick, tmp_char, TO_NOTVICT);
        act("You throw $p at $N.",FALSE,ch, stick, tmp_char, TO_CHAR);
        act("$n throws $p at you.", 0, ch, stick, tmp_char, TO_VICT);


        if(ch->skills[SKILL_THROW].learned < 85)
          ch->skills[SKILL_THROW].learned += 2;

      }
      else {
        send_to_char("You can't do that.\n\r", ch);
        return;
      }

      a = stick->obj_flags.value[0];
      b = stick->obj_flags.value[1];

      dam = dice(a, b);
      unequip_char(ch, HOLD);
      obj_to_char(stick, tmp_char);
      save_char(ch, NOWHERE);
      /* WAIT_STATE(ch, PULSE_VIOLENCE); */

      if(ROOM_CHAOTIC(CHAR_REAL_ROOM(tmp_char))) dam = MIN(dam,500);  /* Chaos04 */

      damage(ch, tmp_char, dam, TYPE_UNDEFINED,DAM_PHYSICAL);
    }
    else {
       send_to_char("Throw it to who?\n\r", ch);
       return;
    }
  }
  else {
    send_to_char("You can't throw that.\n\r", ch);
    return;
  }
}


void do_shoot(struct char_data *ch, char *argument, int cmd) {
  if (!ch) return;

  OBJ *gun = EQ(ch, HOLD);

  if (!gun) {
    send_to_char("You are not holding anything.\n\r", ch);

    return;
  }

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && !CHAOSMODE) {
    send_to_char("Behave yourself here please!\n\r", ch);

    return;
  }

  if (OBJ_TYPE(gun) != ITEM_FIREWEAPON) {
    send_to_char("You can't shoot that.\n\r", ch);

    return;
  }

  argument = skip_spaces(argument);

  if (!*argument && IS_IMPLEMENTOR(ch)) {
    act("$n shoots $s gun wildly in the air!", FALSE, ch, 0, 0, TO_ROOM);
    act("You shoot your gun wildly in the air!", FALSE, ch, 0, 0, TO_CHAR);

    return;
  }

  CHAR *target = NULL;

  int target_bits = generic_find(argument, FIND_CHAR_ROOM, ch, &target, NULL);

  if (!target || !target_bits) {
    send_to_char("Shoot who?\n\r", ch);

    return;
  }

  if (target == ch) {
    if (IS_IMPLEMENTOR(ch)) {
      act("$n puts a gun to $s head and pulls the trigger!  BANG!", FALSE, ch, 0, 0, TO_ROOM);
      act("You put a gun to your head and pull the trigger!  BANG!", FALSE, ch, 0, 0, TO_CHAR);
    }
    else {
      send_to_char("Shoot yourself?  You must be kidding.\n\r", ch);
    }

    return;
  }

  if (!OBJ_VALUE(gun, 1) && !IS_IMPLEMENTOR(ch)) {
    send_to_char("Your weapon has run out of ammunition!\n\r", ch);

    return;
  }

  if (!OBJ_VALUE(gun, 2) && !IS_IMPLEMENTOR(ch)) {
    send_to_char("Your gun is too old to shoot anymore.\n\r", ch);

    return;
  }

  if (!IS_IMPLEMENTOR(ch)) {
    OBJ_VALUE(gun, 1) -= 1;
  }

  if ((number(1, 100) > 70) && !IS_IMPLEMENTOR(ch)) {
    act("$n fires a shot into the sky.", TRUE, ch, gun, target, TO_ROOM);
    act("You miss your shot at $N.", FALSE, ch, gun, target, TO_CHAR);
    
    WAIT_STATE(ch, PULSE_VIOLENCE);
    
    return;
  }

  act("$n shoots at $N.", FALSE, ch, 0, target, TO_NOTVICT);
  act("$n shoots at you.", FALSE, ch, 0, target, TO_VICT);
  act("You shoot at $N.", FALSE, ch, 0, target, TO_CHAR);

  if (IS_IMPLEMENTOR(ch) && IS_SET(GET_IMM_FLAGS(ch), WIZ_ACTIVE)) {
    act("$n's shot hits $N right between the eyes!", FALSE, ch, 0, target, TO_NOTVICT);
    act("$n's shot hits you right between the eyes!", FALSE, ch, 0, target, TO_VICT);
    act("Your shot hits $M right between the eyes!", FALSE, ch, 0, target, TO_CHAR);

    signal_char(target, ch, MSG_DIE, "\0");
    divide_experience(ch, target, 1);
    raw_kill(target);

    return;
  }

  damage(ch, target, dice(OBJ_VALUE(gun, 2), OBJ_VALUE(gun, 3)), TYPE_UNDEFINED, DAM_PHYSICAL);

  WAIT_STATE(ch, PULSE_VIOLENCE);
}

void do_reload(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  struct obj_data  *gun, *bullet;

  argument_interpreter(argument, arg1, arg2);

     if(!(gun = get_obj_in_list_vis(ch, arg1, ch->carrying)))
     {     act("You do not have that item.", FALSE, ch, 0, 0, TO_CHAR);
          return;
     }

     if(!(bullet = get_obj_in_list_vis(ch,arg2,ch->carrying))) {
          act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
          return;
       }


     if(bullet->obj_flags.value[2] != gun->obj_flags.value[0])
     {     send_to_char("Your ammunition doesn't fit the weapon!\n\r", ch);
          return;
     }

     if((gun->obj_flags.type_flag==ITEM_FIREWEAPON) &&
          (bullet->obj_flags.type_flag==ITEM_BULLET)) {
          if(gun->obj_flags.value[1] == bullet->obj_flags.value[3])
               {     send_to_char("Your weapon is loaded already!\n\r", ch);
                         return;
               }
          else     { act("$n reloads $p.", TRUE, ch, gun, 0, TO_ROOM);
                 act("You reload it.", FALSE, ch, gun, 0, TO_CHAR);
                 gun->obj_flags.value[1] = bullet->obj_flags.value[3];
                 gun->obj_flags.cost = 0;
                 extract_obj(bullet);
                 if(gun->obj_flags.value[2] > 0)
                      gun->obj_flags.value[2] -= 1;
                 return;
               }
          }
     else { send_to_char("You can't do that.\n\r", ch);
            return; }

}

void do_display(struct char_data *ch, char *argument, int cmd)
{
  char option[256], number[256];

  if(!ch->desc)
    return;

  argument = one_argument(argument, option);

  if(option[0] != '\0') {
    if(strcmp(option, "name") == 0) {
      if(IS_SET(ch->desc->prompt, PROMPT_NAME)) {
     send_to_char("Name display off.\n\r", ch);
     REMOVE_BIT(ch->desc->prompt, PROMPT_NAME);
      } else {
     send_to_char("Name display on.\n\r", ch);
     SET_BIT(ch->desc->prompt, PROMPT_NAME);
      } /* if*/
    } else if(strcmp(option, "hp") == 0) {
      argument = one_argument(argument, number);
      if(number[0] != '\0') {
        if(*number == '1') {
       SET_BIT(ch->desc->prompt, PROMPT_HP);
       send_to_char("Hit point display on.\n\r", ch);
        } else {
       REMOVE_BIT(ch->desc->prompt, PROMPT_HP);
       send_to_char("Hit point display off.\n\r", ch);
        }
        if(*(number + 1) == '1') {
       SET_BIT(ch->desc->prompt, PROMPT_HP_MAX);
       send_to_char("Max hit point display on.\n\r", ch);
        } else {
       REMOVE_BIT(ch->desc->prompt, PROMPT_HP_MAX);
       send_to_char("Max hit point display off.\n\r", ch);
        }
        if(*(number + 2) == '1') {
       SET_BIT(ch->desc->prompt, PROMPT_HP_TEX);
       send_to_char("Hit point text display on.\n\r", ch);
        } else {
       REMOVE_BIT(ch->desc->prompt, PROMPT_HP_TEX);
       send_to_char("Hit point tex display off.\n\r", ch);
        }
      } else {
        if(IS_SET(ch->desc->prompt, PROMPT_HP | PROMPT_HP_MAX)) {
       send_to_char("Hp and max hp display off.\n\r", ch);
       REMOVE_BIT(ch->desc->prompt, PROMPT_HP | PROMPT_HP_MAX);
        } else {
       send_to_char("Hp and max hp display on.\n\r", ch);
       SET_BIT(ch->desc->prompt, PROMPT_HP | PROMPT_HP_MAX);
        } /* if*/
      }
    } else if(strcmp(option, "mana") == 0) {
      argument = one_argument(argument, number);
      if(number[0] != '\0') {
        if(*number == '1') {
       SET_BIT(ch->desc->prompt, PROMPT_MANA);
       send_to_char("Mana display on.\n\r", ch);
        } else {
       REMOVE_BIT(ch->desc->prompt, PROMPT_MANA);
       send_to_char("Mana display off.\n\r", ch);
        }
        if(*(number + 1) == '1') {
       SET_BIT(ch->desc->prompt, PROMPT_MANA_MAX);
       send_to_char("Max mana display on.\n\r", ch);
        } else {
       REMOVE_BIT(ch->desc->prompt, PROMPT_MANA_MAX);
       send_to_char("Max mana display off.\n\r", ch);
        }
        if(*(number + 2) == '1') {
       SET_BIT(ch->desc->prompt, PROMPT_MANA_TEX);
       send_to_char("Mana text display on.\n\r", ch);
        } else {
       REMOVE_BIT(ch->desc->prompt, PROMPT_MANA_TEX);
       send_to_char("Mana text display off.\n\r", ch);
        }
      } else {
        if(IS_SET(ch->desc->prompt, PROMPT_MANA | PROMPT_MANA_MAX)) {
       send_to_char("Mana and max mana display off.\n\r", ch);
       REMOVE_BIT(ch->desc->prompt, PROMPT_MANA | PROMPT_MANA_MAX);
        } else {
       send_to_char("Mana and max mana display on.\n\r", ch);
       SET_BIT(ch->desc->prompt, PROMPT_MANA | PROMPT_MANA_MAX);
        } /* if*/
      }
    } else if(strcmp(option, "move") == 0) {
      argument = one_argument(argument, number);
      if(number[0] != '\0') {
        if(*number == '1') {
       SET_BIT(ch->desc->prompt, PROMPT_MOVE);
       send_to_char("Move display on.\n\r", ch);
        } else {
       REMOVE_BIT(ch->desc->prompt, PROMPT_MOVE);
       send_to_char("Move display off.\n\r", ch);
        }
        if(*(number + 1) == '1') {
       SET_BIT(ch->desc->prompt, PROMPT_MOVE_MAX);
       send_to_char("Max move display on.\n\r", ch);
        } else {
       REMOVE_BIT(ch->desc->prompt, PROMPT_MOVE_MAX);
       send_to_char("Max move display off.\n\r", ch);
        }
        if(*(number + 2) == '1') {
       SET_BIT(ch->desc->prompt, PROMPT_MOVE_TEX);
       send_to_char("Move tex display on.\n\r", ch);
        } else {
       REMOVE_BIT(ch->desc->prompt, PROMPT_MOVE_TEX);
       send_to_char("Move tex display off.\n\r", ch);
        }
      } else {
        if(IS_SET(ch->desc->prompt, PROMPT_MOVE | PROMPT_MOVE_MAX)) {
       send_to_char("Movement point and max movement point display off.\n\r", ch);
       REMOVE_BIT(ch->desc->prompt, PROMPT_MOVE | PROMPT_MOVE_MAX);
        } else {
       send_to_char("Movement point and max movement point display on.\n\r", ch);
       SET_BIT(ch->desc->prompt, PROMPT_MOVE | PROMPT_MOVE_MAX);
        } /* if*/
      }
    } else if(strcmp(option, "buf") == 0) {
      argument = one_argument(argument, number);
      if(number[0] != '\0') {
        if(*number == '1') {
       SET_BIT(ch->desc->prompt, PROMPT_BUFFER);
       send_to_char("Buffer display on.\n\r", ch);
        } else {
       REMOVE_BIT(ch->desc->prompt, PROMPT_BUFFER);
       send_to_char("Buffer display off.\n\r", ch);
        }
        if(*(number + 1) == '1') {
       SET_BIT(ch->desc->prompt, PROMPT_BUFFER_A);
       send_to_char("Buffer always display on.\n\r", ch);
        } else {
       REMOVE_BIT(ch->desc->prompt, PROMPT_BUFFER_A);
       send_to_char("Buffer always display off.\n\r", ch);
        }
        if(*(number + 2) == '1' || GET_LEVEL(ch) < LEVEL_SUP) {
       SET_BIT(ch->desc->prompt, PROMPT_BUFFER_TEX);
       send_to_char("Buffer text display on.\n\r", ch);
        } else {
       REMOVE_BIT(ch->desc->prompt, PROMPT_BUFFER_TEX);
       send_to_char("Buffer number display on.\n\r", ch);
        }
      } else {
        if(IS_SET(ch->desc->prompt, PROMPT_BUFFER | PROMPT_BUFFER_A)) {
       send_to_char("Buffer and always buffer display off.\n\r", ch);
       REMOVE_BIT(ch->desc->prompt, PROMPT_BUFFER | PROMPT_BUFFER_A);
        } else {
       send_to_char("Buffer and always buffer display on.\n\r", ch);
       SET_BIT(ch->desc->prompt, PROMPT_BUFFER | PROMPT_BUFFER_A);
        } /* if*/
      }
    } else if(strcmp(option, "vic") == 0) {
      argument = one_argument(argument, number);
      if(number[0] != '\0') {
        if(*number == '1') {
       SET_BIT(ch->desc->prompt, PROMPT_VICTIM);
       send_to_char("Victim display on.\n\r", ch);
        } else {
       REMOVE_BIT(ch->desc->prompt, PROMPT_VICTIM);
       send_to_char("Victim display off.\n\r", ch);
        }
        if(*(number + 1) == '1') {
       SET_BIT(ch->desc->prompt, PROMPT_VICTIM_A);
       send_to_char("Victim always display on.\n\r", ch);
        } else {
       REMOVE_BIT(ch->desc->prompt, PROMPT_VICTIM_A);
       send_to_char("Victim always display off.\n\r", ch);
        }
        if(*(number + 2) == '1' || GET_LEVEL(ch) < LEVEL_SUP) {
       SET_BIT(ch->desc->prompt, PROMPT_VICTIM_TEX);
       send_to_char("Victim text display on.\n\r", ch);
        } else {
       REMOVE_BIT(ch->desc->prompt, PROMPT_VICTIM_TEX);
       send_to_char("Victim number display on.\n\r", ch);
        }
      } else {
        if(IS_SET(ch->desc->prompt, PROMPT_VICTIM | PROMPT_VICTIM_A)) {
       send_to_char("Victim and always victim display off.\n\r", ch);
       REMOVE_BIT(ch->desc->prompt, PROMPT_VICTIM | PROMPT_VICTIM_A);
        } else {
       send_to_char("Victim and always victim display on.\n\r", ch);
       SET_BIT(ch->desc->prompt, PROMPT_VICTIM | PROMPT_VICTIM_A);
        } /* if*/
      }
    } else if(strcmp(option, "all") == 0) {
      send_to_char("Full display on.\n\r", ch);
      ch->desc->prompt = 0xFFFFFFFF;
    } else if(strcmp(option, "none") == 0) {
      send_to_char("Display off.\n\r", ch);
      if(GET_LEVEL(ch) >= LEVEL_SUP)
        ch->desc->prompt = 0;
      else
        ch->desc->prompt = PROMPT_VICTIM_TEX + PROMPT_BUFFER_TEX;
    } else {
      send_to_char("Display what???\n\r", ch);
    } /* if*/
  } else {
    send_to_char("Display usage : display [name all none].\n\r", ch);
    send_to_char("             or display <hp mana move buf vic> [number].\n\r", ch);
    send_to_char("Example: display hp 101\n\r", ch);
  } /* if*/

  ch->new.prompt = ch->desc->prompt;

} /* do_display */

void do_home(struct char_data *ch, char *argument, int cmd) {

  if(!ch) return;
  if(IS_NPC(ch)) return;

  if(GET_LEVEL(ch)>9) {
    send_to_char("Sorry you are too experienced to need this command, buy recalls at the magic shop.\n\r",ch);
    return;
  }

  if(world[CHAR_REAL_ROOM(ch)].number==10 ||
     world[CHAR_REAL_ROOM(ch)].number==1200) {
    send_to_char("You cannot escape!\n\r",ch);
    return;
  }

  spell_word_of_recall(GET_LEVEL(ch),ch,ch,0);
  if(GET_LEVEL(ch)==9) {
    send_to_char("\n\rThis is your last level for free recalling.\n\rAfter this you need to buy recall scrolls from the magic shop.\n\r",ch);
    return;
  }
}

void do_skin(struct char_data *ch, char *argument, int cmd)
{
  struct obj_data *corpse=0,*skin, *obj=0;
  char obj_name[240],skin_name[240],buf[MIL];
  int i,rnum,decay,repop_bonus = 0;

  argument = one_argument(argument, obj_name);
  if(!(corpse = get_obj_in_list_vis(ch, obj_name, world[CHAR_REAL_ROOM(ch)].contents))){
    send_to_char("Skin what?\n\r", ch);
    return;
  }

  if(GET_ITEM_TYPE(corpse)!=ITEM_CONTAINER && corpse->obj_flags.value[3]!=1) {
    send_to_char("That isn't a corpse.\n\r",ch);
    return;
  }

  if(!IS_NPC(ch) && (corpse->obj_flags.cost==PC_CORPSE || corpse->obj_flags.cost==CHAOS_CORPSE)) {
    send_to_char("The thought of skinning a fellow adventurer makes you cringe!\n\r",ch);
    return;
  }

  if(corpse->obj_flags.cost==PC_STATUE || corpse->obj_flags.cost==NPC_STATUE) {
    send_to_char("You find it impossible to chip away at the stone.\n\r",ch);
    return;
  }

  if(GET_LEVEL(ch) < LEVEL_IMM) {
    /* Check decayed status of corpse */
    decay=10-10*corpse->obj_flags.timer/MAX_NPC_CORPSE_TIME;
    if(decay<0) decay=4;
    if(decay>3) {
      send_to_char("The corpse is too badly damaged.\n\r",ch);
      return;
    }
  }


  /* make a skin of whatever it is */
  CREATE(skin, struct obj_data, 1);
  clear_object(skin);
  skin->item_number = NOWHERE;
  skin->in_room = NOWHERE;
  skin->name = str_dup("skin");

  if(corpse->description) {
  half_chop(corpse->description,buf,MIL,skin_name,240);
  sprintf(buf,"skin %s",skin_name);
  }
  else {
    sprintf(buf,"skin of something");
  }
  skin->description = str_dup(buf);

  if(corpse->short_description) {
  half_chop(corpse->short_description,buf,MIL,skin_name,240);
  sprintf(buf,"Skin %s",skin_name);
  }
  else {
    sprintf(buf,"Skin of something is here");
  }
  skin->short_description = str_dup(buf);

  skin->obj_flags.type_flag    = ITEM_SKIN;
  skin->obj_flags.wear_flags   = ITEM_TAKE+ITEM_WEAR_ABOUT;
  skin->obj_flags.extra_flags  = ITEM_ANTI_RENT;
  skin->obj_flags.extra_flags2 = ITEM_ALL_DECAY;
  skin->obj_flags.material     = corpse->obj_flags.material;
  skin->obj_flags.value[0]     = 0;
  skin->obj_flags.value[1]     = 0;
  skin->obj_flags.value[2]     = 0;
  skin->obj_flags.value[3]     = 0;
  skin->obj_flags.weight       = 1;

  /* Random gold over 100k - Ranger Feb-02 for skins */
  if(corpse->obj_flags.cost_per_day>100000)
    skin->obj_flags.cost       = 100000+(corpse->obj_flags.cost_per_day-100000)/100*number(75,105);
  else
    skin->obj_flags.cost       = corpse->obj_flags.cost_per_day;

  if(GET_CLASS(ch)==CLASS_NOMAD)
    skin->obj_flags.cost       = (corpse->obj_flags.cost_per_day)/100*number(105,110);

  skin->obj_flags.timer        = 120;

  skin->next = object_list;
  object_list = skin;
  obj_to_room(skin, CHAR_REAL_ROOM(ch));

  send_to_char("You strip the skin off the corpse and rip the corpse apart.\n\r", ch);
  act("$n strips the skin off the corpse and rips the corpse apart.", TRUE, ch, 0,0, TO_ROOM);

  repop_bonus = (BAMDAY) ? 10 : 0;

  for(i = 0; i < 6; i++) {
    if(corpse->obj_flags.skin_vnum[i] == 0)
      continue;
    if((rnum=real_object(corpse->obj_flags.skin_vnum[i])) > 0) {
      /* repop percent check */
      if(number(1,100)<=(obj_proto_table[rnum].obj_flags.repop_percent + repop_bonus) ||
         obj_proto_table[rnum].obj_flags.type_flag==ITEM_KEY) {
        obj = read_object(rnum, REAL);
        obj_to_room(obj, CHAR_REAL_ROOM(ch));
        act("You find $p within the corpse!", TRUE, ch, obj, 0, TO_CHAR);
        act("$n finds $p within the corpse!", TRUE, ch, obj, 0, TO_ROOM);
      }
    }
  }

/* Decay corpse */
  corpse->obj_flags.timer-=(MAX_NPC_CORPSE_TIME/2);
  corpse->obj_flags.timer=MAX(1,corpse->obj_flags.timer);
}

void do_email(struct char_data *ch, char *argument, int cmd) {
  char usage[]="\
This command allows you to assign an email address and turn on/off postcard\n\r\
copying to that address.\n\r\n\r\
  Usage: email yes/no - toggles postcard email copies\n\r\
               show   - shows current email address\n\r\
               set <address> - sets email address\n\r";
  char arg[MIL];

  if(IS_NPC(ch)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage, ch);
    return;
  }

  else if(is_abbrev(arg, "yes")) {
    SET_BIT(ch->specials.pflag, PLR_EMAIL);
    send_to_char("Postcard copying to email turned on.\n\r", ch);
    return;
  }
  else if(is_abbrev(arg, "no")) {
    REMOVE_BIT(ch->specials.pflag, PLR_EMAIL);
    send_to_char("Postcard copying to email turned off.\n\r", ch);
    return;
  }
  else if(is_abbrev(arg, "show")) {
    printf_to_char(ch, "Current email address: %s\n\r",GET_EMAIL(ch));
    return;
  }
  else if(is_abbrev(arg, "set")) {
    argument=one_argument(argument,arg);
    if(!*arg) {
      send_to_char(usage, ch);
      return;
    }
    sprintf(GET_EMAIL(ch),"%s",arg);
    send_to_char("Done\n\r", ch);
    return;
  }
  else {
    send_to_char(usage, ch);
    return;
  }
}

