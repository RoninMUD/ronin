/*
April 30/01

 Special Procedure Module                   Orig. Date 19-Jan-2000

 spec.enchanted.c

 Written by Ranger of RoninMUD
*/

#include <stdlib.h>
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
#include "fight.h"
#include "cmd.h"
#include "spec_assign.h"
#include "interpreter.h"
#include "act.h"
#include "reception.h"

int shop_keeper(CHAR *keeper,CHAR *ch,int cmd,char *arg);
#define HESTIA_PASS   28761
#define PAN           28506
#define CRUCIFIX      28600
#define PLUTO         28703
#define ATHENA        28600
#define GATEKEEPER    28507

/*
mob 28701 cant be attacked
*/
int ench_typhon(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if(cmd==CMD_KILL || cmd==CMD_HIT || cmd==CMD_KICK ||
     cmd==CMD_AMBUSH || cmd==CMD_ASSAULT || cmd==CMD_BACKSTAB) {
    act("$n says 'Don't waste my time.'",0,mob,0,0,TO_ROOM);
    return TRUE;
  }
  if(cmd==MSG_MOBACT && mob->specials.fighting) {
    act("$n says 'You bore me, stop this foolishness.",0,mob,0,0,TO_ROOM);
    for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room)
      if(vict->specials.fighting==mob) stop_fighting(vict->specials.fighting);
    stop_fighting(mob);
    GET_HIT(mob)=GET_MAX_HIT(mob);
    return FALSE;
  }
  if(cmd==CMD_NORTH) {
    act("$n says 'You're not getting past me.'",0,mob,0,0,TO_ROOM);
    return TRUE;
  }

  return FALSE;
}

/*
obj crumble - 28500 28501 and 28502
*/
int ench_crumble_obj(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  OBJ *tmp_obj;
  char arg[MIL];

  if(!ch) return FALSE;

  if(cmd==MSG_OBJ_DROPPED) {
    act("Your $q falls to pieces as you try to drop it.",FALSE,ch,obj,0,TO_CHAR);
    act("$n's $q falls to pieces as $e tries to drop it.",FALSE,ch,obj,0,TO_ROOM);
    extract_obj(obj_from_char(obj));
    return TRUE;
  }

  if(cmd==CMD_DONATE) {
    one_argument(argument,arg);
    tmp_obj=get_obj_in_list_vis(ch,arg,ch->carrying);
    if(obj!=tmp_obj) {
      return FALSE;
    }
    act("Your $q falls to pieces as you try to donate it.",FALSE,ch,obj,0,TO_CHAR);
    act("$n's $q falls to pieces as $e tries to donate it.",FALSE,ch,obj,0,TO_ROOM);
    extract_obj(obj_from_char(obj));
    return TRUE;
  }
  return FALSE;
}

/*
thorn damage
Rooms 28567, 28574, 28575
10% of pcs current hps at 50% on leave and enter
*/
int ench_thorn_room(int room, CHAR *ch, int cmd, char *arg) {

  if (!ch) return FALSE;

  switch (cmd) {
    case MSG_LEAVE:
    case MSG_ENTER:
      if (IS_NPC(ch)) return FALSE;
      if(chance(50)) return FALSE;
      send_to_char("As you try to move, thorns dig deeply in your skin.\n\r", ch);
      damage(ch,ch,GET_HIT(ch)/10,TYPE_UNDEFINED,DAM_NO_BLOCK);
      return FALSE;
      break;
  }
  return FALSE;
}

/*
bushes rustle - rooms 28501 28506 28518 28514
*/
int ench_bush_room(int room, CHAR *ch, int cmd, char *arg) {
  if(cmd!=MSG_MOBACT) return FALSE;
  if(!world[room].people) return FALSE;
  if(chance(75)) return FALSE;
  send_to_room ("You can hear a rustling in the bushes nearby.\n\r",room);
  return FALSE;
}

/*
roots trip - rooms 28504 28511 28515 28516 28517
*/
int ench_root_room(int room, CHAR *ch, int cmd, char *arg) {

  if(!ch) return FALSE;
  if(cmd >= CMD_NORTH && cmd <= CMD_DOWN && chance(40)) {
    if (IS_NPC(ch)) return FALSE;
    if(ch->specials.riding) return FALSE;
    if(!IS_AFFECTED(ch,AFF_FLY)) {
      send_to_char("While your attention is elsewhere you trip heavily over a protruding root.\n\r", ch);
      act("While $s attention is else where, $n trips on a root.",0,ch,0,0,TO_ROOM);
    }
    else {
      send_to_char("You are clotheslined by an overhanging branch as you fly by.\n\r", ch);
      act("$n is clotheslined by an overhanging branch as $e fly by.",0,ch,0,0,TO_ROOM);
    }
    WAIT_STATE(ch,PULSE_VIOLENCE);
    GET_POS(ch)=POSITION_SITTING;
    return TRUE;
  }
  return FALSE;
}

/*
brush scratch - rooms 28505 28506
triggered by 50% of pcs entering and exiting. Damages for 20
chance of poison and blind for 30 ticks
28506 also gets bush rustle
*/
int ench_scratch_room(int room, CHAR *ch, int cmd, char *arg) {
  struct affected_type_5 af;

  switch (cmd) {
    case MSG_MOBACT:
      if(world[room].number==28506)
        return(ench_bush_room(room,ch,cmd,arg));
      break;
    case MSG_LEAVE:
    case MSG_ENTER:
      if(!ch) return FALSE;
      if(IS_NPC(ch)) return FALSE;
      if(chance(50)) return FALSE;
      send_to_char("The rough brush nearby scratches at your face painfully.\n\r", ch);
      if(chance(50)) {
        send_to_char("You have been blinded!\n\r",ch);
        af.type      = SPELL_BLINDNESS;
        af.location  = APPLY_HITROLL;
        af.modifier  = -4;  /* Make hitroll worse */
        af.duration  = 30;
        af.bitvector = AFF_BLIND;
        af.bitvector2 = 0;
        affect_to_char(ch, &af);
        af.location = APPLY_AC;
        af.modifier = +40; /* Make AC Worse! */
        affect_to_char(ch, &af);
      }
      if(chance(50)) {
        af.type = SPELL_POISON;
        af.duration = 30;
        af.modifier = -3;
        af.location = APPLY_STR;
        af.bitvector = AFF_POISON;
        af.bitvector2 = 0;
        affect_join(ch, &af, FALSE, FALSE);
        send_to_char("You feel very sick.\n\r", ch);
      }
      if(CHAR_REAL_ROOM(ch)!=NOWHERE) /* If he didn't die already */
        damage(ch,ch,20,TYPE_UNDEFINED,DAM_NO_BLOCK);
      return FALSE;
      break;
  }
  return FALSE;
}

/*
flute playing- rooms 28525 28526 and 28528
*/

int ench_flute_room(int room, CHAR *ch, int cmd, char *arg) {
  if(cmd!=MSG_MOBACT) return FALSE;
  if(!world[room].people) return FALSE;
  if(chance(75)) return FALSE;
  if(!mob_proto_table[real_mobile(PAN)].number) return FALSE;
  send_to_room ("You hear the sweet melody of a flute being played from nearby.\n\r",room);
  return FALSE;
}

/*Icarus 28700 */
int ench_icarus(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim,*next_vict;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) { /* 600 damage room spec */
      act("$n twirls $s wings wildly about the room!",0,mob,0,0,TO_ROOM);
      for(victim=world[CHAR_REAL_ROOM(mob)].people;victim; victim = next_vict) {
        next_vict=victim->next_in_room;
        if(!IS_MORTAL(victim)) continue;
        act("$n's wings slice into your body!",0,mob,0,victim,TO_VICT);
        damage(mob,victim,600,TYPE_UNDEFINED,DAM_NO_BLOCK);
      }
      return FALSE;
    }

    if(chance(20)) { /* 200 damage PC spec */
      if((victim=get_random_victim(mob))) {
        act("$n flies over your head and lands on you!",0,mob,0,victim,TO_VICT);
        act("$n flies over $N's head and lands on $m!",0,mob,0,victim,TO_NOTVICT);
        act("You fly over $N's head and lands on $m!",0,mob,0,victim,TO_CHAR);
        damage(mob,victim,200,TYPE_UNDEFINED,DAM_NO_BLOCK);
        return FALSE;
      }
    }

    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,3)) {
          case 1:
            sprintf(buf,"say How dare he leave me down here in the dark all these centuries, %s!",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"pout %s",GET_NAME(victim));
            break;
          case 3:
            sprintf(buf,"say You will regret disturbing my solitude, %s.",GET_NAME(victim));
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  return FALSE;
}

/* Pluto 28703 */
void check_equipment(CHAR *ch);
int ench_pluto(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(15)) {
      if((victim=get_random_victim(mob))) {
        if(GET_ALIGNMENT(victim)>499 && mob->specials.fighting!=victim) {
          act("$n cackles madly and scorches you with the flames of hellish wrath!",0,mob,0,victim,TO_VICT);
          act("$n cackles madly and scorches $N with the flames of hellish wrath!",0,mob,0,victim,TO_NOTVICT);
          act("You cackles madly and scorch $N with the flames of hellish wrath!",0,mob,0,victim,TO_CHAR);
          GET_ALIGNMENT(victim)=-1000;
          GET_HIT(victim)=66;
          GET_MANA(victim)=66;
          GET_MOVE(victim)=66;
          check_equipment(victim);  /* Linerfix 110203 */
          return FALSE;
        }
      }
    }

    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say Fear not, %s, It will be a quick death.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"laugh %s",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  return FALSE;
}

/* Charon 28704 */
int ench_charon(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(25)) {
      if((victim=get_ch_room(PLUTO,CHAR_REAL_ROOM(mob)))) {
        if(GET_HIT(victim)<GET_MAX_HIT(victim)) {
          act("$n waves $s hands and assists $s master.",0,mob,0,0,TO_ROOM);
          GET_HIT(victim)=GET_MAX_HIT(victim);
          return FALSE;
        }
      }
    }

    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say I shall accompany you across the river and into the underworld, %s.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"comfort %s",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  return FALSE;
}

/* Cerberus 28705 */
int ench_cerberus(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"snarl %s",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"growl %s",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  return FALSE;
}

/* follower 28706 */
int ench_follower(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say You are but a weakling child before the might of my dark master, %s!",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"rofl %s",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  return FALSE;
}

/* achilles 28708 */
int ench_achilles(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say Fear my might, %s, I am the hero Achilles.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"say If you turn away now you may leave with your life, %s.",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  return FALSE;
}

/* odysseus 28709 */
int ench_odysseus(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say I have heard of your travels, %s, and respect them as I was once a great traveler.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"say Best watch yourself, %s, there are those who would see you dead.",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  return FALSE;
}

/* persius 28710 */
int ench_persius(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say Welcome to the Village of the Gods, %s.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"smile %s",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  return FALSE;
}

/* oedipus 28711 */
int ench_oedipus(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say Know thyself, %s, lest you be stricken like me.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"sigh %s",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  return FALSE;
}

/* thesius 28712 */
int ench_thesius(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say I have heard of your exploits, %s, and am impressed.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"bow %s",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  return FALSE;
}

/* chimera 28713 */
int ench_chimera(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"snarl %s",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"growl %s",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  return FALSE;
}

/* promethius 28714 */
int ench_promethius(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say You have used the fire well, %s, but what brings you here I wonder.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"ponder %s",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  return FALSE;
}

/* agamemnon 28716 */
int ench_agamemnon(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say Those closest to you are most likely to be your end, %s.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"nod %s",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  return FALSE;
}

/* jason 28717 */
int ench_jason(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say I know you have traveled far, %s, don't lose heart.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"bow %s",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  return FALSE;
}

/* pandora 28718 */
int ench_pandora(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say Never give in to curiosity, %s, some things are best left alone.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"sigh %s",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  return FALSE;
}

/* cronus 28719 */
int ench_cronus(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim,*next_vict;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) { /* stop fighting spec */
      act("You feel time stop suddenly and you slow to a halt",0,mob,0,0,TO_ROOM);
      for(victim=world[CHAR_REAL_ROOM(mob)].people;victim; victim = next_vict) {
        next_vict=victim->next_in_room;
        stop_fighting(victim);
      }
      return FALSE;
    }

    if(chance(20)) { /* 200 damage PC spec */
      if((victim=get_random_victim(mob))) {
        act("$n points at you and laughs. You scream in agony!",0,mob,0,victim,TO_VICT);
        act("$n points at $N and laughs. $N screams in agony!",0,mob,0,victim,TO_NOTVICT);
        act("You point at $N and laugh. $N screams in agony!",0,mob,0,victim,TO_CHAR);
        damage(mob,victim,800,TYPE_UNDEFINED,DAM_NO_BLOCK);
        return FALSE;
      }
    }

    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say Fear me %s! I am Cronus! Rightful King of Gods!",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"say Someday soon, %s, I shall retake my place on Olympus.",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  return FALSE;
}

/* heracles 28720 */
int ench_heracles(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say How do thee %s?",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"say My wares will keep thee on thy long journey forthcoming, %s.",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  if(shop_keeper(mob,ch,cmd,argument)) return TRUE;
  return FALSE;
}

/* medusa 28721 */
int ench_medusa(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say You could use a little work, %s, and you could be radiant like me.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"wink %s",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  if(shop_keeper(mob,ch,cmd,argument)) return TRUE;
  return FALSE;
}

/* midas 28722 */
int ench_midas(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say Gold is the metal of the Gods, %s, and my wares are the finest created.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"say I would offer you a handshake, %s, but I think a bow would be safer.",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  if(shop_keeper(mob,ch,cmd,argument)) return TRUE;
  return FALSE;
}

/* dionysus 28723 */
int ench_dionysus(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say Take a load off, %s.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"say Have a drink and stay a while, %s, plenty of time for adventuring later.",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  if(shop_keeper(mob,ch,cmd,argument)) return TRUE;
  return FALSE;
}

/* atlas 28724 */
int ench_atlas(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say Every worthy adventurer needs a map, %s.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"say No one under the stars has seen the world more clearly than I, %s.",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  if(shop_keeper(mob,ch,cmd,argument)) return TRUE;
  return FALSE;
}

/* demeter 28725 */
int ench_demeter(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say My fine grain products will sustain you in the journey ahead, %s.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"smile %s",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  if(shop_keeper(mob,ch,cmd,argument)) return TRUE;
  return FALSE;
}

/* aphrodite 28727 */
int ench_aphrodite(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say I am far more beautiful than she! And yet they buy beauty from her, %s!",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"cry %s",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  if(shop_keeper(mob,ch,cmd,argument)) return TRUE;
  return FALSE;
}

/* hermes 28728 */
int postoffice(CHAR *mob, CHAR *ch, int cmd, char *arg);
int ench_hermes(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];


  if(ch && (cmd==CMD_MAIL || cmd==CMD_POST)) {
    if(postoffice(mob,ch,cmd,argument))
      return TRUE;
    return FALSE;
  }


  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say Any outgoing mail for me, %s?",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"say Communication is the key to a successful expidition, %s.",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  if(shop_keeper(mob,ch,cmd,argument)) return TRUE;
  return FALSE;
}

/* hypnos 28729 */
int ench_hypnos(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say Rest is what is needed to sustain you, %s.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"say Take a potion and head for the Traveler's Rest, %s.",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  if(shop_keeper(mob,ch,cmd,argument)) return TRUE;
  return FALSE;
}

/* Epimethius 28730 */
int ench_epimethius(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say A well forged blade is the difference between life and death, %s",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"say My weapons are made by the hand of Hephaestus himself, %s.",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  if(shop_keeper(mob,ch,cmd,argument)) return TRUE;
  return FALSE;
}

/* hestia 28731 */
int ench_hestia(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];
  OBJ *obj;

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say Welcome, %s, to the Traveler's Rest. Would you like a room?",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"say Stay and rest, %s. Your adventures await you on the morn.",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  if(cmd==CMD_WEST) {
    if(!ch) return FALSE;
    if(!get_obj_in_list_num(real_object(HESTIA_PASS), ch->carrying)) {
      act("$N tells you 'I don't see your pass in your inventory, want to buy one?'",0,ch,0,mob,TO_CHAR);
      return TRUE;
    }
    return FALSE;
  }
  if(cmd==CMD_LIST) {
    if(!ch) return FALSE;
    act("$N tells you 'You can buy a pass card for 200,000 coins'",0,ch,0,mob,TO_CHAR);
    return TRUE;
  }
  if(cmd==CMD_BUY) {
    one_argument(argument,buf);
    if(!*buf) {
      act("$N tells you 'Buy what?'",0,ch,0,mob,TO_CHAR);
      return TRUE;
    }
    if(strcmp(buf,"pass") && strcmp(buf,"card") && strcmp(buf,"passcard")) {
      act("$N tells you 'I don't have that.'",0,ch,0,mob,TO_CHAR);
      return TRUE;
    }
    if(GET_GOLD(ch)<200000) {
      act("$N tells you 'You don't have enough gold.'",0,ch,0,mob,TO_CHAR);
      return TRUE;
    }
    GET_GOLD(ch)-=200000;
    obj=read_object(HESTIA_PASS,VIRTUAL);
    obj_to_char(obj,ch);
    act("$N tells you 'Here you go, thanks for the gold'",0,ch,0,mob,TO_CHAR);
    return TRUE;
  }
  return FALSE;
}

/* daedalus 28732 */
int ench_daedalus(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say My wings are guaranteed to make you fly like a bird, %s!",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"Just watch how close you get to the sun, %s. Oh my poor boy..",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  if(shop_keeper(mob,ch,cmd,argument)) return TRUE;
  return FALSE;
}

/* eros 28733 */
int ench_eros(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  OBJ *obj;
  char obj_name[MAX_STRING_LENGTH], vict_name[MAX_STRING_LENGTH];
  char buf[MIL];

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say Love is all that is true in the universe, %s.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"say Tell her how you really feel, %s!",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }

  if(cmd==CMD_LIST || cmd==CMD_BUY) {
    argument = one_argument(argument, obj_name);
    argument = one_argument(argument, vict_name);

    if (cmd==CMD_LIST) {
      act("$N tells you 'You can buy a valentine for 500 coins'",0,ch,0,mob,TO_CHAR);
      return TRUE;
    }

    if(!*obj_name || !*vict_name) {
      act("$N tells you 'Buy what for who?'",0,ch,0,mob,TO_CHAR);
      return TRUE;
    }

    if(!strcmp(obj_name,"valentine")) {
      if(GET_GOLD(ch) < 500) {
        act("$N tells you 'You don't have enough coins!'",0,ch,0,mob,TO_CHAR);
        return TRUE;
      }

      if(!(victim = get_char_vis(ch, vict_name))) {
        act("$N tells you 'No-one by that name around.'",0,ch,0,mob,TO_CHAR);
        return TRUE;
      }

      GET_GOLD(ch) -= 500;
      send_to_char("Ok.\n\r", ch);
      act("$N says '$n has bought a valentine for someone.'", FALSE, ch, 0, mob, TO_ROOM);
      obj = read_object(28752, VIRTUAL);
      if(!obj) return TRUE;
      obj_to_char(obj, victim);
      act("A giddy little cherub flies down from the clouds and lands before $n.", TRUE, victim, 0, 0, TO_ROOM);
      act("A giddy little cherub flies down from the clouds and lands before you.", TRUE, victim, 0, 0, TO_CHAR);
      act("The little cherub winks slyly hands you a valentine and whispers 'This is from $N', before disappearing.", TRUE, victim, 0, ch, TO_CHAR);
      act("The little cherub winks slyly hands $n a valentine.", TRUE, victim, 0, 0, TO_ROOM);
    }
    else
      act("$N tells you 'Buy what?'",0,ch,0,mob,TO_CHAR);
    return TRUE;
  }
  return FALSE;
}

/* Pegasus 28734 sells out winged horses 28737 as mounts. */
int ench_pegasus(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MIL],mount_name[MIL],tmp_pwd[11];
  struct follow_type *k;

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say My mounts are unmatched in the mortal world, %s.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"say You look suprised that such a great steed as I have mastered your feeble tongue, %s.",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }

  if(cmd==CMD_LIST) {
    if(!ch) return FALSE;
    act("$N tells you 'You can buy a winged horse for 50,000 coins.'",0,ch,0,mob,TO_CHAR);
    return TRUE;
  }

  if(cmd==CMD_BUY) {
    argument=one_argument(argument,buf);
    if(!*buf) {
      act("$N tells you 'Buy what?'",0,ch,0,mob,TO_CHAR);
      return TRUE;
    }
    if(strcmp(buf,"horse") && strcmp(buf,"winged")) {
      act("$N tells you 'I don't have that.'",0,ch,0,mob,TO_CHAR);
      return TRUE;
    }
    one_argument(argument,mount_name);
    if(!*mount_name) {
      act("$N tells you 'You must give your steed a name. Try 'buy horse <name>'.",0,ch,0,mob,TO_CHAR);
      return TRUE;
    }
    if(GET_GOLD(ch)<50000) {
      act("$N tells you 'You don't have enough gold.'",0,ch,0,mob,TO_CHAR);
      return TRUE;
    }
    for(k=ch->followers; k; k= k->next) {
      if(IS_NPC(k->follower) && (k->follower->master==ch) &&
        IS_SET(k->follower->specials.act, ACT_MOUNT)) {
        act("$N tells you 'You already have one mount!'", FALSE, ch, 0, mob, TO_CHAR);
        return TRUE;
      }
    }
    if(test_char(mount_name, tmp_pwd)) {
      act("$n tells you 'Please use a different name for your mount'",FALSE, ch, 0, mob, TO_CHAR);
      return TRUE;
    }
    victim=read_mobile(28737,VIRTUAL);
    GET_EXP(victim) = 0;
    SET_BIT(victim->specials.affected_by, AFF_CHARM);
    sprintf(buf,"%s %s",MOB_NAME(victim),mount_name);
    if(victim->player.name) free(victim->player.name);
    victim->player.name = str_dup(buf);
    GET_GOLD(ch)-=50000;
    act("$N tells you 'Please take care of your mount.'", FALSE, ch, 0, mob, TO_CHAR);
    act("$n bought $N as a mount.",TRUE,ch,0,victim,TO_ROOM);
    char_to_room(victim,CHAR_REAL_ROOM(ch));
    add_follower(victim, ch);
    return TRUE;
  }

  return FALSE;
}

/* asclepius 28736 */
int ench_asclepius(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim;
  char buf[MSL];
  int cost,mult=1;

  if(IS_NPC(ch)) return FALSE;
  if(CHAOSMODE) mult=10;

  if (cmd==CMD_LIST) { /* List */
    act("$N tells you 'I can offer you the following spells.'",0, ch,0,mob,TO_CHAR);
    cost=GET_LEVEL(ch)*2500*mult;
    printf_to_char(ch,"Miracle will cost you %d coins. (miracle)\n\r", cost);

    cost=GET_LEVEL(ch)*1000*mult;
    printf_to_char(ch,"Heal will cost you %d coins. (heal)\n\r", cost);

    cost=GET_LEVEL(ch)*1000*mult;
    printf_to_char(ch,"Sanctuary will cost you %d coins. (sanctuary)\n\r",cost);

    cost=GET_LEVEL(ch)*500*mult;
    printf_to_char(ch,"Remove Poison will cost you %d coins. (poison)\n\r",cost);

    cost=GET_LEVEL(ch)*150*mult;
    printf_to_char(ch,"Cure Serious will cost you %d coins. (serious)\n\r",cost);

    cost=GET_LEVEL(ch)*100*mult;
    printf_to_char(ch,"Endure will cost you %d coins. (endure)\n\r", cost);

    cost=GET_LEVEL(ch)*50*mult;
    printf_to_char(ch,"Armor will cost you %d coins. (armor)\n\r", cost);

    cost=GET_LEVEL(ch)*45*mult;
    printf_to_char(ch,"Vitality will cost you %d coins. (vitality)\n\r",cost);

    return TRUE;
  }

  if (cmd==CMD_BUY) { /* Buy */

    if(ch->specials.fighting) {
      act("$N tells you 'This place isn't for fighting, go away!'",0,ch,0,mob,TO_CHAR);
      return TRUE;
    }

    argument=one_argument(argument, buf);
    if(!*buf) {
      act("$N tells you 'What would you like to buy?'",0,ch,0,mob,TO_CHAR);
      return TRUE;
    }

    if(!strcmp(buf,"miracle")) {
      cost=GET_LEVEL(ch)*2500*mult;
      if(GET_GOLD(ch)<cost) {
        act("$N tells you 'Sorry, you dont have enough money.'",0,ch,0,mob,TO_CHAR);
        return(TRUE);
      }
      cast_miracle(35,mob,"",SPELL_TYPE_SPELL,ch,0);
      GET_GOLD(ch)-=cost;
      sprintf(buf,"$N tells you 'That will be %d coins please.'\n\r",cost);
      act(buf,0,ch,0,mob,TO_CHAR);
      return(TRUE);
    }

    if(!strcmp(buf,"heal")) {
      cost=GET_LEVEL(ch)*1000*mult;
      if(GET_GOLD(ch)<cost) {
        act("$N tells you 'Sorry, you dont have enough money.'",0,ch,0,mob,TO_CHAR);
        return(TRUE);
      }
      cast_heal(35,mob,"",SPELL_TYPE_SPELL,ch,0);
      GET_GOLD(ch)-=cost;
      sprintf(buf,"$N tells you 'That will be %d coins please.'\n\r",cost);
      act(buf,0,ch,0,mob,TO_CHAR);
      return(TRUE);
    }

    if(!strcmp(buf,"sanctuary")) {
      cost=GET_LEVEL(ch)*1000*mult;
      if(GET_GOLD(ch)<cost) {
        act("$N tells you 'Sorry, you dont have enough money.'",0,ch,0,mob,TO_CHAR);
        return(TRUE);
      }
      cast_sanctuary(35,mob,"",SPELL_TYPE_SPELL,ch,0);
      GET_GOLD(ch)-=cost;
      sprintf(buf,"$N tells you 'That will be %d coins please.'\n\r",cost);
      act(buf,0,ch,0,mob,TO_CHAR);
      return(TRUE);
    }

    if(!strcmp(buf,"poison")) {
      cost=GET_LEVEL(ch)*500*mult;
      if(GET_GOLD(ch)<cost) {
        act("$N tells you 'Sorry, you dont have enough money.'",0,ch,0,mob,TO_CHAR);
        return(TRUE);
      }
      cast_remove_poison(35,mob,"",SPELL_TYPE_SPELL,ch,0);
      GET_GOLD(ch)-=cost;
      sprintf(buf,"$N tells you 'That will be %d coins please.'\n\r",cost);
      act(buf,0,ch,0,mob,TO_CHAR);
      return(TRUE);
    }

    if(!strcmp(buf,"serious")) {
      cost=GET_LEVEL(ch)*150*mult;
      if(GET_GOLD(ch)<cost) {
        act("$N tells you 'Sorry, you dont have enough money.'",0,ch,0,mob,TO_CHAR);
        return(TRUE);
      }
      cast_cure_serious(35,mob,"",SPELL_TYPE_SPELL,ch,0);
      GET_GOLD(ch)-=cost;
      sprintf(buf,"$N tells you 'That will be %d coins please.'\n\r",cost);
      act(buf,0,ch,0,mob,TO_CHAR);
      return(TRUE);
    }

    if(!strcmp(buf,"endure")) {
      cost=GET_LEVEL(ch)*100*mult;
      if(GET_GOLD(ch)<cost) {
        act("$N tells you 'Sorry, you dont have enough money.'",0,ch,0,mob,TO_CHAR);
        return(TRUE);
      }
      cast_endure(35,mob,"",SPELL_TYPE_SPELL,ch,0);
      GET_GOLD(ch)-=cost;
      sprintf(buf,"$N tells you 'That will be %d coins please.'\n\r",cost);
      act(buf,0,ch,0,mob,TO_CHAR);
      return(TRUE);
    }

    if (!strcmp(buf,"armor")) {
      cost=GET_LEVEL(ch)*50*mult;
      if(GET_GOLD(ch)<cost) {
        act("$N tells you 'Sorry, you dont have enough money.'",0,ch,0,mob,TO_CHAR);
        return(TRUE);
      }
      cast_armor(35,mob,"",SPELL_TYPE_SPELL,ch,0);
      GET_GOLD(ch)-=cost;
      sprintf(buf,"$N tells you 'That will be %d coins please.'\n\r",cost);
      act(buf,0,ch,0,mob,TO_CHAR);
      return(TRUE);
    }

    if (!strcmp(buf,"vitality")) {
      cost=GET_LEVEL(ch)*45*mult;
      if(GET_GOLD(ch)<cost) {
        act("$N tells you 'Sorry, you dont have enough money.'",0,ch,0,mob,TO_CHAR);
        return(TRUE);
      }
      cast_vitality(35,mob,"",SPELL_TYPE_SPELL,ch,0);
      GET_GOLD(ch)-=cost;
      sprintf(buf,"$N tells you 'That will be %d coins please.'\n\r",cost);
      act(buf,0,ch,0,mob,TO_CHAR);
      return(TRUE);
    }
    act("$N tells you 'You can't buy that.'",0,ch,0,mob,TO_CHAR);
    return(TRUE);
  }

  if(cmd==MSG_MOBACT) {
    if(chance(10)) {
      if((victim=get_random_victim(mob))) {
        switch(number(1,2)) {
          case 1:
            sprintf(buf,"say Are you wounded %s? Let me have a look at that.",GET_NAME(victim));
            break;
          case 2:
            sprintf(buf,"say Even the greatest warriors are injured in battle, %s.",GET_NAME(victim));
            break;
        }
        command_interpreter(mob,buf);
        return FALSE;
      }
    }
    return FALSE;
  }
  return FALSE;
}

int ench_crucifix(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  CHAR *tmp, *tch;
  char buf[1000];

  if(CHAOSMODE) return FALSE;

  if(cmd == MSG_DIE) {
    if(obj->equipped_by) {
      tch = obj->equipped_by;
      if(IS_MOB(tch)) return FALSE;
    }
    else
      return FALSE;

    if(IS_SET(world[CHAR_REAL_ROOM(tch)].room_flags, CHAOTIC)) return FALSE;

    act("$n's crucifix  explodes into a billion shimmering beads of light.\n\r$n disappears.", TRUE, tch, 0, 0, TO_ROOM);
    send_to_char("Your crucifix explodes into a billion shimmering beads of light and transports you home.\n\r",tch);
    GET_HIT(tch) = 1;
    GET_MANA(tch) = 0;
    GET_MOVE(tch) = 0;

    for(tmp = world[CHAR_REAL_ROOM(tch)].people;tmp;tmp=tmp->next_in_room)
      if(GET_OPPONENT(tmp) == tch)
        stop_fighting (tmp);

    sprintf(buf,"%s rescued %s from %s [%d].", OBJ_SHORT(obj),
            GET_NAME(tch), world[CHAR_REAL_ROOM(tch)].name,
            CHAR_VIRTUAL_ROOM(tch));
    wizlog(buf, LEVEL_WIZ, 6);
    log_s(buf);

    stop_fighting(tch);
    unequip_char(tch, WEAR_LIGHT);
    obj_to_char(obj, tch);
    extract_obj(obj);

    spell_word_of_recall(GET_LEVEL(tch), tch, tch, 0);
    return TRUE;
  }
  return FALSE;
}

int ench_athena(CHAR *mob,CHAR *ch,int cmd,char *argument) {
  CHAR *victim,*next_vict;
  int cleric=0,i;

  if(cmd==MSG_MOBACT) {
    if(!mob->specials.fighting) return FALSE;

    if(chance(2)) {
      if((victim=get_random_victim_fighting(mob))) {
        if(victim!=mob->specials.fighting) {
          act("$n points to the sky and calls to her Father for assistance!",0,mob,0,0,TO_ROOM);
          act("A bolt of lightning suddenly slices the sky and hits $n!",0,victim,0,0,TO_ROOM);
          act("A bolt of lightning suddenly slices the sky and hits you!",0,victim,0,0,TO_CHAR);
          GET_HIT(victim)=-1;
          return FALSE;
        }
      }
    }

#ifdef TEST_SITE
    if(chance(50)) {
#else
    if(chance(5)) {
#endif
      for(victim=world[CHAR_REAL_ROOM(mob)].people;victim; victim = victim->next_in_room)
        if(IS_MORTAL(victim) && GET_CLASS(victim)==CLASS_CLERIC) cleric++;
      if(cleric>0) {
        i=number(1,cleric);
        for(victim=world[CHAR_REAL_ROOM(mob)].people;victim; victim = victim->next_in_room) {
          if(IS_MORTAL(victim) && GET_CLASS(victim)==CLASS_CLERIC && i==cleric) {
            act("$n says 'You call yourself a healer but you use your powers for evil!",0,mob,0,0,TO_ROOM);
            GET_MANA(victim)=0;
          }
        }
        return FALSE;
      }
    }

    if(chance(5)) {
      if((victim=get_random_victim_fighting(mob))) {
        if(victim!=mob->specials.fighting) {
          act("$n says 'Bow before the feet of your master you vile heathen!",0,mob,0,0,TO_ROOM);
          char_from_room(victim);
          char_to_room(victim, real_room(28700));
          signal_room(CHAR_REAL_ROOM(victim), victim, MSG_ENTER,"");
          do_look(victim,"",CMD_LOOK);
          return FALSE;
        }
      }
    }

    if(chance(10)) {
      act("$n calls down the wrath of $s father upon the wicked.",0,mob,0,0,TO_ROOM);
      for(victim=world[CHAR_REAL_ROOM(mob)].people;victim; victim = next_vict) {
        next_vict=victim->next_in_room;
        if(IS_MORTAL(victim) && GET_ALIGNMENT(victim)<0) {
          act("You feel extreme pain and beg for forgiveness.",0,mob,0,victim,TO_VICT);
          damage(mob,victim,1000,TYPE_UNDEFINED,DAM_NO_BLOCK);
        }
      }
      return FALSE;
    }

    if(chance(30)) {
      act("$n waves $m arms in a circular pattern.",0, mob, 0, 0, TO_ROOM);
      act("You wave your arms in a circular pattern.",0,mob, 0, 0, TO_CHAR);
      for(victim=world[CHAR_REAL_ROOM(mob)].people;victim; victim = next_vict) {
        next_vict=victim->next_in_room;
        if(IS_MORTAL(victim) && EQ(victim,WIELD)) {
          act("You feel at peace suddenly and drop your weapon.",0, mob, 0, victim, TO_VICT);
          obj_to_char(unequip_char(victim, WIELD), victim);
        }
      }
      return FALSE;
    }

  } /* mobact */
  return FALSE;
}

/*
mob 28507 cant be attacked - token specs
*/
int ench_gatekeeper(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  static int countdown=0;
  int has_valor=0,has_truth=0,has_courage=0;
  char buf[MSL];

  if(cmd==CMD_KILL || cmd==CMD_HIT || cmd==CMD_KICK ||
     cmd==CMD_AMBUSH || cmd==CMD_ASSAULT || cmd==CMD_BACKSTAB) {
    act("$n says 'Don't bore me.'",0,mob,0,0,TO_ROOM);
    return TRUE;
  }
  if(cmd==MSG_MOBACT) {
    if(mob->specials.fighting) {
      act("$n says 'You bore me, stop this foolishness.",0,mob,0,0,TO_ROOM);
      for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = vict->next_in_room)
        if(vict->specials.fighting==mob) stop_fighting(vict->specials.fighting);
      stop_fighting(mob);
      GET_HIT(mob)=GET_MAX_HIT(mob);
      return FALSE;
    }
  }
  if(cmd==MSG_TICK) {
    if(CHAR_VIRTUAL_ROOM(mob)!=28542) return FALSE;
    if(real_room(28542)==NOWHERE || real_room(28780)==NOWHERE ||
       !world[real_room(28542)].dir_option[UP] ||
       !world[real_room(28780)].dir_option[DOWN]) {
      wizlog("ZINFO: Room 28542/28780 not loaded or have no exits up/down defined.",LEVEL_SUP,6);
      return FALSE;
    }

    if(world[real_room(28542)].dir_option[UP]->to_room_r==real_room(28780)) {
      /* no say if room exists - timer reduction */
      countdown--;
      countdown=MAX(countdown,0);
      if(countdown==0) { /* close the exit */
        world[real_room(28542)].dir_option[UP]->to_room_r = real_room(0);
        world[real_room(28780)].dir_option[DOWN]->to_room_r = real_room(0);
        send_to_room ("The stairs up magically disappear.\n\r",real_room(28542));
        send_to_room ("The stairs down magically disappear.\n\r",real_room(28780));
        strip_char(mob);
      }
      return FALSE;
    }
    else { /* no exit exists - do other stuff */
      if(!is_carrying_obj(mob,28500)) { /* doesn't have valor */
        if(count_mortals_real_room(CHAR_REAL_ROOM(mob))) {
          if(chance(50))
            act("$n says 'I can not let you pass until you are proven worthy.'",0,mob,0,0,TO_ROOM);
          else
            act("$n says 'I require proof of your valor.'",0,mob,0,0,TO_ROOM);
        }
        return FALSE;
      }
      /* has at least valor */

      countdown--;
      countdown=MAX(countdown,0);
      if(countdown) {
        sprintf(buf,"$n says 'I'm getting impatient, I'll only wait %d more cycles.",countdown);
        act(buf,0,mob,0,0,TO_ROOM);
      }
      else {
        strip_char(mob);
        act("$n says 'You have waited too long, you are not worthy.",0,mob,0,0,TO_ROOM);
        return FALSE;
      }

      if(!is_carrying_obj(mob,28501)) { /* doesn't have truth */
        act("$n says 'I require proof of your honesty.'",0,mob,0,0,TO_ROOM);
        return FALSE;
      }

      /* has truth */
      if(!is_carrying_obj(mob,28502)) { /* doesn't have courage */
        act("$n says 'I require proof of your courage.'",0,mob,0,0,TO_ROOM);
        return FALSE;
      }
      /* has courage */
    }/* end of else on exit */
    return FALSE;
  }/* end of MSG_TICK */

  if(cmd==MSG_GAVE_OBJ) {
    if(count_carrying_obj(mob,28500)==1) has_valor=1;
    if(count_carrying_obj(mob,28501)==1) has_truth=1;
    if(count_carrying_obj(mob,28502)==1) has_courage=1;

    if(isname("valor",arg)) { /* gave valor token */
      if(count_carrying_obj(mob,28500)==2) {
        act("$n says 'You have already proven your valor, this is worthless.'",0,mob,0,0,TO_ROOM);
        do_drop(mob,"valor",CMD_DROP);
        return TRUE;
      }
      if(has_valor) {
        act("$n says 'You have proven your valor, but I need more from you.'",0,mob,0,0,TO_ROOM);
        countdown=10;
        return TRUE;
      }
    }

    if(isname("truth",arg)) { /* gave truth token */
      if(count_carrying_obj(mob,28501)==2) {
        act("$n says 'You have already proven your honesty, this is worthless.'",0,mob,0,0,TO_ROOM);
        do_drop(mob,"truth",CMD_DROP);
        return TRUE;
      }
      if(has_valor && has_truth) {
        act("$n says 'You have proven your honesty, but I need more from you.'",0,mob,0,0,TO_ROOM);
        countdown=10;
        return TRUE;
      }
      else {
        act("$n says 'Why are you giving me this? You have other things to prove!'",0,mob,0,0,TO_ROOM);
        do_drop(mob,"truth",CMD_DROP);
        return TRUE;
      }

    }

    if(isname("courage",arg)) { /* gave courage token */
      if(count_carrying_obj(mob,28502)==2) {
        act("$n says 'You have already proven your courage, this is worthless.'",0,mob,0,0,TO_ROOM);
        do_drop(mob,"courage",CMD_DROP);
        return TRUE;
      }
      if(has_valor && has_truth && has_courage) {
        act("$n says 'The gods have deemed you worthy, you may pass.'\n\rA golden staircase descends from the heavens and touches the ground.\n\r",0,mob,0,0,TO_ROOM);
        countdown=5;
        if(real_room(28542)==NOWHERE || real_room(28780)==NOWHERE ||
           !world[real_room(28542)].dir_option[UP] ||
           !world[real_room(28780)].dir_option[DOWN]) {
          wizlog("ZINFO: Room 28542/28780 not loaded or have no exits up/down defined.",LEVEL_SUP,6);
          return TRUE;
        }
        world[real_room(28542)].dir_option[UP]->to_room_r = real_room(28780);
        world[real_room(28780)].dir_option[DOWN]->to_room_r = real_room(28542);
        return TRUE;
      }
      else {
        act("$n says 'Why are you giving me this? You have other things to prove!'",0,mob,0,0,TO_ROOM);
        do_drop(mob,"courage",CMD_DROP);
        return TRUE;
      }
    }
    one_argument(arg,buf);
    act("$n says 'This item means nothing to me.'",0,mob,0,0,TO_ROOM);
    do_drop(mob,buf,CMD_DROP);
    return TRUE;
  }/* end of MSG_GAVE_OBJ */

  return FALSE;
}

int check_for_level_32(OBJ *obj, CHAR *ch,int cmd, char *argument) {
  if(cmd==MSG_OBJ_WORN) {
    if(GET_LEVEL(ch)<32 || IS_NPC(ch)) {
      act("You are too inexperienced to use this.",0,ch,0,0,TO_CHAR);
      return TRUE;
    }
  }
  return FALSE;
}

int blade_unholy_sacrifice(OBJ *blade, CHAR *ch, int cmd, char *argument) {
  CHAR *vict, *tmp_vict;

	ch=blade->equipped_by;

  if(cmd==MSG_MOBACT && ch && ch->specials.fighting)
  {
		switch(number(0,39))
		{
			case 0:
	      act("Your blade whirls out of control and slashes everyone! Including you!", FALSE, ch, NULL, NULL, TO_CHAR);
	      for(vict = world[CHAR_REAL_ROOM(ch)].people; vict; vict = tmp_vict)
	      {
					tmp_vict = vict->next_in_room;
      	  if(!IS_MORTAL(vict)) continue;
      	  if(vict != ch) act("$n's blade whirls out of control and slashes you!", FALSE, ch, NULL, vict, TO_VICT);
      	  damage(vict, vict, 200, TYPE_UNDEFINED, DAM_NO_BLOCK);
      	}
	      break;
			case 1:
      	vict = get_random_victim(ch);
      	if(chance(25)) vict = ch;
      	if(vict)
      	{
            if(vict == ch)
            {
                act("Your blade whirls out of control and slashes you!", FALSE, ch, NULL, NULL, TO_CHAR);
                act("$n's blade whirls out of control and slashes $m!", FALSE, ch, NULL, NULL, TO_ROOM);
					}
					else
					{
		        act("Your blade whirls out of control and slashes $N!", FALSE, ch, NULL, vict, TO_CHAR);
		        act("$n's blade whirls out of control and slashes $N!", FALSE, ch, NULL, vict, TO_NOTVICT);
		        act("$n's blade whirls out of control and slashes you!", FALSE, ch, NULL, vict, TO_VICT);
					}
      	damage(vict, vict, 400, TYPE_UNDEFINED, DAM_NO_BLOCK);
        }
      	break;
			default:
				break;
		}
	}
	return FALSE;
}/* END blade_unholy_sacrifice : rewrite by Hemp */

int ench_crown_kings(OBJ *obj, CHAR *ch,int cmd, char *argument) {
  CHAR *vict;

  if(cmd!=MSG_TICK) return FALSE;
  if(!(vict=obj->equipped_by)) return FALSE;

#ifndef TEST_SITE
  if(chance(96)) return FALSE;
#endif

  act("$n's crown glows with a divine brilliance for a brief moment.",0,vict,0,0,TO_ROOM);
  act("Your crown glows with a divine brilliance for a brief moment.",0,vict,0,0,TO_CHAR);

  unequip_char(vict, WEAR_HEAD);
  if(obj->affected[2].modifier>-1000) {
    obj->affected[2].modifier-=1;
    if(obj->affected[2].modifier<-500) {
      obj->affected[0].modifier=4;
      obj->affected[1].modifier=4;
    }
    if(obj->affected[2].modifier==-1000) {
      obj->affected[0].modifier=5;
      obj->affected[1].modifier=5;
    }
  }
  equip_char(vict,obj,WEAR_HEAD);

  return FALSE;
}

/*
The spec is you put these three crowns into pandoras box, close it,
and drop it in room 28700 and it all disappears. Then another crown, 28745 loads on the ground
in its place.
*/

int ench_pandora_box(OBJ *obj ,CHAR *ch, int cmd, char *argument) {
  OBJ *obj2;
  char buf[MAX_INPUT_LENGTH];
  int crown1=0,crown2=0,crown3=0;

  if (cmd != CMD_DROP) return FALSE;
  if(!ch) return FALSE;
  if(ch!=obj->carried_by) return FALSE;
  if(CHAR_REAL_ROOM(ch)!=real_room(28700)) return FALSE;

  one_argument(argument, buf);
  if(!isname(buf,OBJ_NAME(obj))) return FALSE;

  for(obj2 = obj->contains;obj2;obj2=obj2->next_content) {
    if(obj2 && V_OBJ(obj2)==28742) crown1=1;
    if(obj2 && V_OBJ(obj2)==28743) crown2=1;
    if(obj2 && V_OBJ(obj2)==28744) crown3=1;
  }

  if(!crown1 || !crown2 || !crown3) return FALSE;
  if(!IS_SET(obj->obj_flags.value[1], CONT_CLOSED)) return FALSE;

  act("$n drops $p.", 1, ch, obj, 0, TO_ROOM);
  act("You drop $p.", 1, ch, obj, 0, TO_CHAR);
  act("The $p vibrates, glows an unearthly red and then vanishes!\n\r",0,ch,obj,0,TO_ROOM);
  act("The $p vibrates, glows an unearthly red and then vanishes!\n\r",0,ch,obj,0,TO_CHAR);
  extract_obj(obj);
  obj2=read_object(28745,VIRTUAL);
  obj_to_room(obj2,CHAR_REAL_ROOM(ch));
  act("A $p appears in its place!",0,ch,obj2,0,TO_ROOM);
  act("A $p appears in its place!",0,ch,obj2,0,TO_CHAR);
  return FALSE;
}

#define COIN 28741
int ench_coin_spec(OBJ *coin ,CHAR *ch, int cmd, char *argument) {
  char arg[MSL];

  if(cmd!=CMD_UNKNOWN) return FALSE;
  if(!ch) return FALSE;
  if (!AWAKE (ch)) return FALSE;

  if(!EQ(ch, HOLD) || EQ(ch,HOLD)!=coin) return FALSE;

  argument=one_argument(argument, arg);
  if(!*arg) return FALSE;
  if(!is_abbrev(arg,"flip")) return FALSE;
  one_argument(argument,arg);
  if(!*arg){
    send_to_char("Flip what?\n\r",ch);
    return TRUE;
  }
  string_to_lower(arg);
  if(strcmp(arg,"coin")) return FALSE;

  act("You flip the Coin of Fate into the air.",0,ch,0,0,TO_CHAR);
  act("$n flips the Coin of Fate into the air.",0,ch,0,0,TO_ROOM);

  obj_to_room(unequip_char(ch,HOLD),CHAR_REAL_ROOM(ch));

  switch(number(0,1)) {
    case 0:
      act("The Coin of Fate reveals a skull!",1,ch,0,0,TO_ROOM);
      act("The Coin of Fate reveals a skull!",1,ch,0,0,TO_CHAR);
      act("$n will have great luck and fortune in the near future.",1,ch,0,0,TO_ROOM);
      act("You will have great luck in the near future. Charon commends you.",1,ch,0,0,TO_CHAR);
      spell_bless(GET_LEVEL(ch),ch,ch,0);
      break;

    case 1:
      act("The Coin of Fate reveals a pair of crossbones!",1,ch,0,0,TO_ROOM);
      act("The Coin of Fate reveals a pair of crossbones!",1,ch,0,0,TO_CHAR);
      act("$n's luck has taken a sudden turn for the worse.",1,ch,0,0,TO_ROOM);
      act("You can sense $n's imminent demise approaching.",1,ch,0,0,TO_ROOM);
      act("Your luck has taken a sudden turn for the worse.",1,ch,0,0,TO_CHAR);
      act("You can sense your imminent demise approaching.",1,ch,0,0,TO_CHAR);
      spell_curse(GET_LEVEL(ch),ch,ch,0);
      break;
  }
  return TRUE;
}

int ench_bouncer (CHAR *mob, CHAR *ch, int cmd, char *arg)  {
  char buf[MSL];
  CHAR *drunk;

  if(CHAR_REAL_ROOM(mob)!=real_room(28796)) return(FALSE);

  if(cmd==CMD_NORTH) {
    send_to_char("The enormous bouncer blocks you from proceeding North.\n\r", ch);
    act("The Bouncer plants himself in front of $n, blocking the way.\n\r", FALSE, ch, 0, 0, TO_ROOM);
    return(TRUE);
  }

  if(cmd==MSG_TICK) {
    if(mob->specials.timer>0) {
      mob->specials.timer--;
      if(mob->specials.timer==0) {
        act("$n says 'The password has changed, and I ain't telling it to you.'",0,mob,0,0,TO_ROOM);
        sprintf(mob->pwd," ");
        if((drunk=get_ch_zone(28715,287))) {
          do_wake(drunk,"",CMD_WAKE);
          do_stand(drunk,"",CMD_STAND);
          drunk->specials.timer=0;
          sprintf(drunk->pwd," ");
        }
      }
    }
    if(mob->specials.timer==0 && chance(20)) {
      act("$n says 'Boss says you can't get inside without the password.'",0,mob,0,0,TO_ROOM);
    }
    return FALSE;
  }

  if(cmd==CMD_SAY) {
    one_argument(arg, buf);
    if (!*buf) return FALSE;
    if(mob->specials.timer==0) return FALSE;

    do_say(ch,arg,CMD_SAY);
    if(strcmp(buf,mob->pwd)) {
      act("$n says 'Bugger off before I end you.'",0,mob,0,0,TO_ROOM);
      return TRUE;
    }
    if(GET_POS(ch) == POSITION_RIDING) {
      act("$n says 'That might be the right password, but no animals are allowed. Dismount!'",0,mob,0,0,TO_ROOM);
      return(TRUE);
    }

    act("$n says 'That's the one, go on in.'", FALSE, mob, 0, 0, TO_ROOM);
    act("$n leaves north.", FALSE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch,real_room(28784));
    do_look(ch,"",CMD_LOOK);
    act("$n has arrived.", FALSE, ch, 0, 0, TO_ROOM);
    return TRUE;
  }

  return FALSE;
}

int ench_drunk(CHAR *mob, CHAR *ch, int cmd, char *arg)  {
  char buf[MSL],password[20],*drinks[5]={"","brew","wine","ale","whisky"};
  char *az[26]={"a","b","c","d","e","f","g","h","i","j","k","l","m","n","o","p","q","r","s","t","u","v","w","x","y","z"};
  char *vol[5]={"a","e","i","o","u"};
  int drink_num[5]={0,28711,28712,28713,28714};
  CHAR *bouncer;
  OBJ *obj;

  if(cmd==MSG_MOBACT) {
    if(chance(10)) { /* 10% random chance */
      if(!(bouncer=get_random_victim(mob))) return FALSE;
      switch(number(1,4)) {
        case 1:
          sprintf(buf,"burp %s",GET_NAME(bouncer));
          break;
        case 2:
          sprintf(buf,"stagger %s",GET_NAME(bouncer));
          break;
        case 3:
          sprintf(buf,"sick");
          break;
        case 4:
          sprintf(buf,"mumble %s",GET_NAME(bouncer));
          break;
      }
      command_interpreter(mob,buf);
      return FALSE;
    }
    return FALSE;
  } /* end of if mobact */

  if(cmd==MSG_TICK) {
    if(mob->specials.timer>4) {
      mob->specials.timer--;
      if(mob->specials.timer==4) {
        do_wake(mob,"",CMD_WAKE);
        do_stand(mob,"",CMD_STAND);
        mob->specials.timer=0;
        if((bouncer=get_ch_zone(28735,287))) {
          bouncer->specials.timer=0;
          sprintf(bouncer->pwd," ");
        }
      }
      return FALSE;
    }
    if(!AWAKE(mob)) return FALSE;
    if(mob->specials.timer==0) {
      /* need to pick a drink */
      mob->specials.timer=number(1,4);
      return FALSE;
    }

    if(chance(75)) return FALSE;
    switch(number(0,1)) {
      case 0:
        if(mob->specials.timer<5 && mob->specials.timer>0) {
          sprintf(buf,"$n says 'I've gone dry my friend, buy a pal some %s.",drinks[mob->specials.timer]);
          act(buf,0,mob,0,0,TO_ROOM);
        }
        break;
      case 1:
        act("$n stumbles and sways as $e moves about.",0,mob,0,0,TO_ROOM);
        break;
    }
    return FALSE;
  }

  if(cmd==MSG_GAVE_OBJ) {
    if(mob->specials.timer>4 || mob->specials.timer<1) return FALSE;
    if(!(obj=get_obj_in_list_num(real_object(drink_num[mob->specials.timer]), mob->carrying))) {
      act("$n says 'Have a heart man! I've no need of that!'",0,mob,0,0,TO_ROOM);
      do_drop(mob,"all",CMD_DROP);
      return FALSE;
    }
    /*create password*/
    sprintf(password,"%s%s%s%s%s%s",az[number(0,25)],az[number(0,25)],vol[number(0,4)],az[number(0,25)],vol[number(0,4)],az[number(0,25)]);
    act("$n says 'Thanky thanks good sir, you've a kind soul.'",0,mob,0,0,TO_ROOM);
    sprintf(buf,"$n downs the %s in one gulp.",drinks[mob->specials.timer]);
    act(buf,0,mob,0,0,TO_ROOM);
    sprintf(buf,"$n says '%s! %s says I! and passes out cold.",password,password);
    act(buf,0,mob,0,0,TO_ROOM);
    extract_obj(obj);
    do_sleep(mob,"",CMD_SLEEP);
    mob->specials.timer=15;
    if((bouncer=get_ch_zone(28735,287))) {
      bouncer->specials.timer=10;
      sprintf(bouncer->pwd,"%s",password);
    }
    return FALSE;
  }
  return FALSE;
}

void assign_enchanted(void) {
  assign_obj(COIN,    ench_coin_spec);
  assign_mob(28700,   ench_icarus);
  assign_mob(PLUTO,   ench_pluto);
  assign_mob(ATHENA,  ench_athena);
  assign_mob(28704,   ench_charon);
  assign_mob(28705,   ench_cerberus);
  assign_mob(28706,   ench_follower);
  assign_mob(28708,   ench_achilles);
  assign_mob(28709,   ench_odysseus);
  assign_mob(28710,   ench_persius);
  assign_mob(28711,   ench_oedipus);
  assign_mob(28712,   ench_thesius);
  assign_mob(28713,   ench_chimera);
  assign_mob(28714,   ench_promethius);
  assign_mob(28716,   ench_agamemnon);
  assign_mob(28717,   ench_jason);
  assign_mob(28718,   ench_pandora);
  assign_mob(28719,   ench_cronus);
  assign_mob(28720,   ench_heracles);
  assign_mob(28721,   ench_medusa);
  assign_mob(28722,   ench_midas);
  assign_mob(28723,   ench_dionysus);
  assign_mob(28724,   ench_atlas);
  assign_mob(28725,   ench_demeter);
  assign_mob(28727,   ench_aphrodite);
  assign_mob(28728,   ench_hermes);
  assign_mob(28729,   ench_hypnos);
  assign_mob(28730,   ench_epimethius);
  assign_mob(28731,   ench_hestia);
  assign_mob(28732,   ench_daedalus);
  assign_mob(28733,   ench_eros);
  assign_mob(28734,   ench_pegasus);
  assign_mob(28736,   ench_asclepius);
  assign_mob(GATEKEEPER,   ench_gatekeeper);
  assign_mob(28701,   ench_typhon);

  assign_obj(28500,   ench_crumble_obj);
  assign_obj(28501,   ench_crumble_obj);
  assign_obj(28502,   ench_crumble_obj);
  assign_obj(HESTIA_PASS, ench_crumble_obj);
  assign_obj(CRUCIFIX,    ench_crucifix);

  assign_room(28525,  ench_flute_room);
  assign_room(28526,  ench_flute_room);
  assign_room(28528,  ench_flute_room);

  assign_room(28505,  ench_scratch_room);
  assign_room(28506,  ench_scratch_room);

  assign_room(28501,  ench_bush_room);
  assign_room(28518,  ench_bush_room);
  assign_room(28514,  ench_bush_room);

  assign_room(28504,  ench_root_room);
  assign_room(28511,  ench_root_room);
  assign_room(28515,  ench_root_room);
  assign_room(28516,  ench_root_room);
  assign_room(28517,  ench_root_room);

  assign_room(28567,  ench_thorn_room);
  assign_room(28574,  ench_thorn_room);
  assign_room(28575,  ench_thorn_room);

  assign_obj(28763, check_for_level_32);
  assign_obj(28764, check_for_level_32);
  assign_obj(28721, blade_unholy_sacrifice);
  assign_obj(28745, ench_crown_kings);
  assign_obj(28750, ench_pandora_box);
  assign_mob(28715, ench_drunk);
  assign_mob(28735, ench_bouncer);

}
