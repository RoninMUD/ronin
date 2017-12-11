/*
///   Special Procedure Module                   Orig. Date    10/28/97
///                                              Last Modified  10/28/97
///
///   spec.vanity.c --- misc specials for the vanity zone
///
///
///
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
#include "utility.h"
#include "cmd.h"
#include "fight.h"
#include "spec_assign.h"
#include "interpreter.h"

/* #define BRAND       29911 purged tan crap too */
#define LANCE 	    29914
/* #define HOLY        29920 purged tan questies */
#define FLAMBERGE   29922
#define RAPIER      29924
#define JADE        29927
/* #define AURA        29909 purged tan questies */
#define MJOLNIR     29957
#define BELLS       29962
#define EP_PAN      29910
#define DEATH_BUD   29973
#define AK_SYM      29996
#define PHISH_SYM   29963
#define FUSE_FANG   29928
#define HELL_STING  29952
#define JAMIE_SWORD 29942
#define JAMIE_HOOD   29805
#define CHUD_SYM 29819
#define JEAROM_SYM 29840
#define FRENZY_SYM 29463
#define TIM_TAIL 29467
#define LEMON_BONG 29487
#define MOG_MUFFLER 29488

int is_shop(CHAR *mob);

int boner (OBJ *sh, CHAR *ch, int cmd, char *arg)
{
  CHAR *tmp, *tch;
  char buf[1000];

  if (cmd == MSG_DIE)
    {
      /* Find the person who is getting killed ... */

      if (sh->equipped_by) {
	tch = sh->equipped_by;
        if(IS_MOB(tch)) return FALSE;
      }
      else
	return FALSE;

  if (IS_SET(world[CHAR_REAL_ROOM(tch)].room_flags, CHAOTIC)) return FALSE;

      act ("$n is just about to die, but $p grows and envelopes $m.\n\r$p disappears.", TRUE, tch, sh, 0, TO_ROOM);
      send_to_char ("As you draw your last breath, your shield envelopes you\n\rand there is only blackness...\n\r",tch);

      GET_HIT (tch) = 1;
      GET_MANA (tch) = 0;
      GET_MOVE (tch) = 0;

      for (tmp = world[CHAR_REAL_ROOM(tch)].people;tmp;tmp=tmp->next_in_room)
	if (GET_OPPONENT(tmp) == tch)
	  stop_fighting (tmp);

      sprintf (buf, "%s rescued %s from %s [%d].", OBJ_SHORT(sh),
	       GET_NAME(tch), world[CHAR_REAL_ROOM(tch)].name,
	       CHAR_VIRTUAL_ROOM(tch));
      wizlog(buf, LEVEL_WIZ, 6);
      log_s(buf);

      stop_fighting (tch);
      unequip_char (tch, WEAR_SHIELD);
      obj_to_char (sh, tch);
      extract_obj (sh);

      spell_word_of_recall (GET_LEVEL(tch), tch, tch, 0);
      return TRUE;
    }

  return FALSE;
}

int sting (OBJ *st, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_INPUT_LENGTH];
  CHAR *vict;

  if (st->equipped_by != ch)
    return FALSE;

  if (cmd != CMD_BACKSTAB)
    return FALSE;

  if ((GET_CLASS (ch) == CLASS_THIEF || GET_CLASS(ch) == CLASS_ANTI_PALADIN) &&
      !number (0,69))
    {
      one_argument (arg, buf);

      if (!(vict = get_char_room_vis (ch, buf)))
	return FALSE;

      act ("$n sticks $p into $N's back, and waves of greenish energy wash over $M.", TRUE, ch, st, vict, TO_NOTVICT);
      act ("$n sticks $p into your back, and waves of lethal poison wash over you.", FALSE, ch, st, vict, TO_VICT);
      act ("You stick $p into $N's back, and waves of greenish energy wash over $M.", TRUE, ch, st, vict, TO_CHAR);

      if (!number(0,19))
	{
	  act ("$p cracks and falls apart.", FALSE,ch, st, 0, TO_ROOM);
	  act ("$p cracks and falls apart.", FALSE,ch, st, 0, TO_CHAR);
	  unequip_char (ch, WIELD);
	  extract_obj (st);
	}

      send_to_char ("You can feel the poison burning its way into your heart.\n\rYou are dying...\n\r", vict);
      damage (ch, vict, 1000, TYPE_KILL,DAM_NO_BLOCK);
      return TRUE;
    }
  return FALSE;
}


int v_hammer(OBJ *obj,CHAR *ch, int cmd, char *argument)
{
  CHAR *vic;
  char arg[MAX_STRING_LENGTH];

  if(cmd != CMD_KILL) return FALSE;

  if(!ch->specials.fighting &&ch->equipment[WIELD]  == obj)
    {

    /* Check that the target is valid */

    one_argument(argument, arg);

    if(!*arg) {
      act("You hear a deafening peal of thunder as $n swings $s mighty hammer.",FALSE,ch,0,ch,TO_NOTVICT);
      send_to_char("The heavens crackle with thunderous noise as you swing your mighty hammer.\n\r",ch);
      return TRUE;
    }

    if (!(vic = get_char_room_vis(ch, arg))) {
      send_to_char("They aren't here.\n\r",ch);
      return TRUE;
    }
    else {
      act("A bolt of lightning from $p strikes $N in the chest as you swing your mighty hammer.",FALSE,ch,obj,vic,TO_CHAR);
      act("Lightning strikes, shattering $N as $n swings $s Mighty Hammer!",FALSE,ch,obj,vic,TO_NOTVICT);
      act("Lightning strikes, shattering you as $n swings $s Mighty Hammer!",FALSE,ch,obj,vic,TO_VICT);
      return FALSE;
    }
  }
  return FALSE;
}


int v_gong(OBJ *obj, CHAR *tmp, int cmd, char *arg) {
  CHAR *ch;
  if (cmd != MSG_TICK) return (FALSE);
  if (!obj->equipped_by) return (FALSE);
  if(obj->obj_flags.value[3]>0) obj->obj_flags.value[3]--;
  ch=obj->equipped_by;
  if (!ch->specials.fighting) return (FALSE);
  if (obj != EQ(ch,HOLD)) return (FALSE);
  if (!number(0,29)) return (FALSE);

  if (IS_NEUTRAL(ch)) return (FALSE);
  if (IS_NEUTRAL(ch->specials.fighting)) return (FALSE);
  if (IS_EVIL(ch) && IS_EVIL(ch->specials.fighting)) return (FALSE);
  if (IS_GOOD(ch) && IS_GOOD(ch->specials.fighting)) return (FALSE);

  if(affected_by_spell(ch,SPELL_FURY)) return (FALSE);

  if(!obj->obj_flags.value[3]) {
    act("The ground shakes and the sky rumbles as your bells begin to chime.",FALSE,ch,obj,0,TO_CHAR);
    act("The ground shakes and the sky rumbles as $n's  bells begin to chime.",FALSE,ch,obj,0,TO_ROOM);
    spell_haste(LEVEL_IMM-1,ch,ch,SPELL_TYPE_SPELL);
    obj->obj_flags.value[3]=30;
    return (FALSE);
  }
  return (FALSE);
}


int v_pan(OBJ *obj,CHAR *ch, int cmd, char *argument) {
  CHAR *vict;
  char arg[MAX_STRING_LENGTH];

  if(!ch) return FALSE;
  if(cmd !=CMD_KILL) return FALSE;
  if(ch->specials.fighting) return FALSE;
  if(!ch->equipment[WIELD]) return FALSE;
  if(ch->equipment[WIELD] != obj) return FALSE;
  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;

  one_argument(argument, arg);
  if(!*arg) return FALSE;
  if(!(vict=get_char_room_vis(ch, arg))) return FALSE;
  if((GET_LEVEL(ch)-GET_LEVEL(vict)) > 2) return FALSE;
  if(IS_SET(ch->specials.pflag, PLR_NOKILL) && !IS_NPC(vict)) return FALSE;
  if (number(0,19)) return FALSE;
  if(is_shop(vict)) return FALSE;

  act("$N's skull makes a crunchy noise as your $p connects with the side of $s head!",FALSE,ch,0,vict,TO_CHAR);
  act("$n's eyes glaze over, and $e suddenly crashes $s $p into your skull!",FALSE,ch,0,vict,TO_VICT);
  act("$n's eyes glaze over as $e beats the living hell out of $N with $s Pan of Wuppass.",FALSE,ch,0,vict,TO_NOTVICT);

  GET_MANA(vict)=MAX(0,GET_MANA(vict) - 50);
  GET_MANA(ch)+=25;
  GET_HIT(vict)=MAX(0,GET_HIT(vict) - 50);
  GET_HIT(ch)+=25;
  return FALSE;
}

int doobies_sobr(OBJ *obj,CHAR *ch, int cmd, char *argument)
{
  CHAR *vic;
  char arg[MAX_STRING_LENGTH];

  if(cmd != CMD_KILL) return FALSE;
  /* Added safe room check - Ranger Sept 97 */
  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;

  if(!ch->specials.fighting &&ch->equipment[WIELD]  == obj)
    {

    /* Check that the target is valid */

    one_argument(argument, arg);

    if(!*arg) {
      act("$n's Bud of Death cuts $m viciously with a sharp leaf, craving more attention.",FALSE,ch,0,ch,TO_NOTVICT);
      send_to_char("Your Bud of Death slashes your arm with a sharp leaf, think you ought to water it *wink*\n\r",ch);
      damage(ch,ch,number(15,40),TYPE_UNDEFINED,DAM_NO_BLOCK);
      return TRUE;
    }

    if (!(vic = get_char_room_vis(ch, arg))) {
      send_to_char("The Bud of Death says 'bah you must really be stoned FOO, aint nobody here named that!'\n\r",ch);
      return TRUE;
    }

    if(!IS_NPC(vic) && !IS_NPC(ch)){
      send_to_char("The Bud of Death says 'bah foo, dont try dat again *glare*'\n\r",ch);
      return TRUE;
    }

    if((GET_LEVEL(ch)-GET_LEVEL(vic)) > 20) return FALSE;

    if(is_shop(vic)) return FALSE;
    /* Cast the spell on him */

    if(number(0,40) != 0) return FALSE;

    act("The Bud of Death ignites into a ball of fire and hallucinogenic smoke, and envelops $n!",FALSE,ch,0,0,TO_ROOM);
    send_to_char("A cloud of hallucinogenic smoke drifts off of your Bud, causing your eyeballs to roll back in your head!\n\r",ch);
    cast_fury(30, ch, "", SPELL_TYPE_SPELL, ch, 0);
  }
  return FALSE;
}



int v_fang(OBJ *obj,CHAR *ch, int cmd, char *argument) {
  CHAR *vict;

  if(cmd != MSG_MOBACT) return FALSE;

  vict=obj->equipped_by;
  if(!vict) return FALSE;
  if(GET_LEVEL(vict)<11) return FALSE;
  if(!vict->specials.fighting) return FALSE;
  if(number(0,12)) return FALSE;
  if(CHAR_REAL_ROOM(vict)!=CHAR_REAL_ROOM(vict->specials.fighting)) return FALSE;
  act("$n's Overly-Ripened Grapefruit spontaneously combusts, shooting citric acid into $N's eyes!",FALSE,vict,obj,vict->specials.fighting,TO_ROOM);
  act("You shield your eyes as your Overly-Ripened Grapefruit explodes, spraying citric acid everywhere.",FALSE,vict,0,vict->specials.fighting,TO_CHAR);
  spell_fireball(GET_LEVEL(vict), vict, vict->specials.fighting, 0);
  return FALSE;
}

int jamie_sword(OBJ *obj,CHAR *ch, int cmd, char *argument)
{

  CHAR *vic;
  char arg[MAX_STRING_LENGTH];

  if(cmd != CMD_KILL) return FALSE;
  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, SAFE) && (!CHAOSMODE)) return FALSE;

  if(!ch->specials.fighting &&ch->equipment[WIELD]  == obj)
    {

    /* Check that the target is valid */

    one_argument(argument, arg);


    if (!(vic = get_char_room_vis(ch, arg))) {
      /*send_to_char("Your tachi splits the air, as your victim isn't here !\n\r",ch);*/
      return FALSE;
    }

    if(!IS_NPC(vic) && !IS_NPC(ch)){
      send_to_char("Your tachi will not unveil it's wrath on other players !\n\r",ch);
      return TRUE;
    }


    /* Cast the spell on him */
    if(is_shop(vic)) return FALSE;

    if(number(0,20) != 0) return FALSE;

    act("A briliant flash of light envelops $n !",FALSE,ch,0,0,TO_ROOM);
    send_to_char("A brilliant flash of light envelopes you !\n\r",ch);
    cast_heal(30, ch, "", SPELL_TYPE_SPELL, ch, 0);
  }
  return FALSE;
}


int
v_symbol (OBJ *sym, CHAR *ch, int cmd, char *arg)
{
  CHAR *tch,*temp;
  OBJ *tmp;
  char buf[MAX_INPUT_LENGTH],buf2[MAX_INPUT_LENGTH];
  int i;
  void remove_item (CHAR*, OBJ*, int);

  if(!ch) return FALSE;

  if (!IS_MORTAL (ch) || sym->equipped_by != ch)
    return FALSE;

  GET_ALIGNMENT (ch) = MAX(-1000, GET_ALIGNMENT (ch) - 100);

  if (cmd == CMD_REMOVE)
    {
      one_argument (arg, buf);
      if (isname (buf, OBJ_NAME(sym)))
	{
	  send_to_char ("Eeek! You can't let it go!\n\r",ch);
	  return TRUE;
	}
      else if (!strcmp (buf, "all")) {
        for (i=0; i < MAX_WEAR; i++) {
          tmp = ch->equipment[i];
          if (tmp) {
            sprintf(buf2,"remove %s",OBJ_NAME(tmp));
            command_interpreter(ch,buf2);
          }
	}
	return TRUE;
      }
    }
  if(cmd == MSG_MOBACT) {
    for (tch = world[CHAR_REAL_ROOM(ch)].people;tch;tch = temp) {
      temp = tch->next_in_room;  /* Added temp - Ranger June 96 */
      if (IS_NPC(tch) && GET_ALIGNMENT (tch) == 1000 && AWAKE (tch) &&
	!GET_OPPONENT (tch) && !tch->master &&
	GET_LEVEL (tch) > GET_LEVEL (ch) && CAN_SEE(tch, ch) &&
        (number(1,10)==1)) {
	  act ("$n attacks you, screaming 'Evil must be punished!'",
	     FALSE,tch,0,ch,TO_VICT);
	  act ("$n attacks $N, screaming 'Evil must be punished!'",
	     FALSE,tch,0,ch,TO_NOTVICT);
	  act ("You attack $N and scream 'Evil must be punished!'",
	     FALSE,tch,0,ch,TO_CHAR);
	  hit (tch, ch, TYPE_HIT);
	  return FALSE;
      }
    }
  }
  return FALSE;
}


int inky_hood(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  /* purple_hat spec from spec.quests.c */
  if(cmd != MSG_TICK) return FALSE;

  if(!obj->equipped_by) return FALSE;

  if(number(0,29)) return FALSE;

  act("$n's hood flickers for a moment.",FALSE,obj->equipped_by,0,0,TO_ROOM);
  act("Your hood flickers for a moment.",FALSE,obj->equipped_by,0,0,TO_CHAR);
  spell_cure_blind(30, obj->equipped_by, obj->equipped_by, 0);
  return FALSE;
}

#define FEROZ_SOCK 29848
int
feroz_sock_lash(OBJ *lash, CHAR *ch, int cmd, char *arg) {

  if(lash->equipped_by)
    {
    if(GET_CLASS(lash->equipped_by)==CLASS_NINJA)
       {
       if(lash->affected[0].modifier == 10)
         {
         lash->equipped_by->points.damroll-=5;
         lash->affected[0].modifier = 5;
         }
       }
    else
       lash->affected[0].modifier = 10;
    switch(cmd)
      {
      case MSG_MOBACT:
        if(!number(0,2))
          {
          if(lash->equipped_by->specials.fighting)
            {
            act("$n holds $s sock up to the nose of $N, causing $M to gag.", 0, lash->equipped_by, 0,  lash->equipped_by->specials.fighting, TO_NOTVICT);
            act("$n holds $s sock up to your nose, causing you to gag.", 0, lash->equipped_by, 0,  lash->equipped_by->specials.fighting, TO_VICT);
            act("You hold your sock up to the nose of $N, causing $M to gag.", 0, lash->equipped_by, 0,  lash->equipped_by->specials.fighting, TO_CHAR);
            GET_POS(lash->equipped_by->specials.fighting) = POSITION_STUNNED;
            WAIT_STATE(lash->equipped_by->specials.fighting, PULSE_VIOLENCE*2);
            }
          }
        break;
      }
  }
  else
    {
    lash->affected[0].modifier = 10;
    switch(cmd)
      {
      case CMD_WIELD:
        if(GET_CLASS(ch)==CLASS_NINJA)
           lash->affected[0].modifier = 5;
        else
           lash->affected[0].modifier = 10;
        break;
      }
    }

  return FALSE;
}



#define LOZER_SEAL 29866
int
lozer_seal_lash(OBJ *lash, CHAR *ch, int cmd, char *arg) {

  if(lash->equipped_by)
    {
    if(GET_CLASS(lash->equipped_by)==CLASS_NINJA)
       {
       if(lash->affected[0].modifier == 10)
         {
         lash->equipped_by->points.damroll-=5;
         lash->affected[0].modifier = 5;
         }
       }
    else
       lash->affected[0].modifier = 10;
    switch(cmd)
      {
      case MSG_MOBACT:
        if(!number(0,2))
          {
          if(lash->equipped_by->specials.fighting)
            {
            act("$r seal loses its cuteness as it pummels $N senseless with its flippers.", 0, lash->equipped_by, 0,  lash->equipped_by->specials.fighting, TO_NOTVICT);
            act("$n holds $s seal up to your head, and it pummels you about the face with its flippers!", 0, lash->equipped_by, 0,  lash->equipped_by->specials.fighting, TO_VICT);
            act("You hold your seal up to the head of $N, and it pummels $N with its flippers!", 0, lash->equipped_by, 0,  lash->equipped_by->specials.fighting, TO_CHAR);
            GET_POS(lash->equipped_by->specials.fighting) = POSITION_STUNNED;
            WAIT_STATE(lash->equipped_by->specials.fighting, PULSE_VIOLENCE*2);
            }
          }
        break;
      }
  }
  else
    {
    lash->affected[0].modifier = 10;
    switch(cmd)
      {
      case CMD_WIELD:
        if(GET_CLASS(ch)==CLASS_NINJA)
           lash->affected[0].modifier = 5;
        else
           lash->affected[0].modifier = 10;
        break;
      }
    }

  return FALSE;
}

struct char_data *get_random_vict(struct char_data *ch);

int Tim_Tail (OBJ *tail, CHAR *ch, int cmd, char *arg) {
  CHAR *owner, *vict;
  struct descriptor_data *d;

  if(!(owner=tail->equipped_by)) return FALSE;

  d=owner->desc;
  if(!d) return FALSE;
  if(d->connected!=CON_PLYNG) return FALSE; /* If char died */

  if(cmd==MSG_MOBACT && chance(1)) {
    if(chance(50)) return FALSE;
    switch(number(1,3)) {
      case 1:
      if(count_mortals_room(owner,TRUE) < 2) break;
      vict=get_random_vict(owner);
      if(!vict) break;
      act("A beam of dark light shoots out towards you momentarily causing you to shudder in fear of the unknown power.",FALSE,owner,tail,vict,TO_VICT);
      act("A beam of dark light shoots out towards $N momentarily causing $M to shudder in fear of the unknown power.",FALSE,owner,tail,vict,TO_NOTVICT);
      act("A beam of dark light shoots out towards $N momentarily causing $M to shudder in fear of the unknown power.",FALSE,owner,tail,vict,TO_CHAR);
      break;

      case 2:
      act("A deafening CRACK rings in your ears as a blast of cosmic energy shoots from the sky and surrounds you!",FALSE,owner,tail,0,TO_CHAR);
      act("With a loud crack, a thick beam of cosmic energy shoots from the sky completely engulfing $n!",FALSE,owner,tail,0,TO_ROOM);
      break;

      case 3:
      act("A deafening CRACK rings in your ears as a blast of cosmic energy shoots from the sky and surrounds you!",FALSE,owner,tail,0,TO_CHAR);
      act("With a loud crack, a thick beam of cosmic energy shoots from the sky completely engulfing $n!",FALSE,owner,tail,0,TO_ROOM);
      break;

      default:
      break;

    }
    return FALSE;
  }
  return FALSE;
}

int mog_muffler (OBJ *snow, CHAR *ch, int cmd, char *arg) {
  CHAR *owner, *vict;

  if(!(owner=snow->equipped_by)) return FALSE;

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    act("......$n looks ready to brave a snowstorm, mumbling something about Umaro.",FALSE,owner,snow,ch,TO_VICT);
    return FALSE;
  }

  if(cmd==MSG_MOBACT && chance(6)) {
    if(chance(10)) {
      if(count_mortals_room(owner,TRUE) < 2) return FALSE;
      vict=get_random_vict(owner);
      if(!vict) return FALSE;
      if(IS_AFFECTED(vict, AFF_IMINV)) return FALSE;
      act("Your $p protects you from a sudden wintry gust, which instead envelops $N in a small blizzard!",FALSE,owner,snow,vict,TO_CHAR);
      act("$N suddenly disappears, engulfed in a freak barometric shift! Thankfully $n is unscathed.",FALSE,owner,snow,vict,TO_NOTVICT);
      act("A freak snowstorm looked ready to engulf $n, but instead surrounds you, making you unable to be seen!",FALSE,owner,snow,vict,TO_VICT);
      cast_imp_invisibility(GET_LEVEL(owner), owner, "", SPELL_TYPE_SPELL, vict, 0);
      return FALSE;
    }
    return FALSE;
  }
  return FALSE;
}


void assign_vanity(void) {

assign_obj(DEATH_BUD, doobies_sobr);
/* assign_obj(BRAND, sting); */
assign_obj(LANCE, sting);
/* assign_obj(HOLY, sting); */
assign_obj(FLAMBERGE, sting);
assign_obj(RAPIER, sting);
assign_obj(29988, sting);
assign_obj(JADE, sting);
/* assign_obj(AURA, boner); */
assign_obj(MJOLNIR, v_hammer);
assign_obj(BELLS, v_gong);
assign_obj(EP_PAN, v_pan);
assign_obj(AK_SYM, v_symbol);
assign_obj(PHISH_SYM, v_symbol);
assign_obj(CHUD_SYM, v_symbol);
assign_obj(JEAROM_SYM, v_symbol);
assign_obj(29856,v_symbol);
assign_obj(FUSE_FANG, v_fang);
assign_obj(HELL_STING,sting);
assign_obj(JAMIE_SWORD, jamie_sword);
assign_obj(JAMIE_HOOD, inky_hood);
assign_obj(FEROZ_SOCK,feroz_sock_lash);
assign_obj(LOZER_SEAL,lozer_seal_lash);
assign_obj(FRENZY_SYM, v_symbol);
assign_obj(TIM_TAIL, Tim_Tail);
assign_obj(LEMON_BONG, v_symbol);
assign_obj(MOG_MUFFLER, mog_muffler);
}

