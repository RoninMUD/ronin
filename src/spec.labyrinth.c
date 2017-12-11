/**************************************************************************
 *  file: spec.labryinth.c , Special module.              Part of DIKUMUD *
 *  Programed by: R. Rogers (Ranger)                                      *
 *  Slightly modified by I. Joakimsson (Quack)                            *
 *  Date: Nov 16/96                                                       *
 *  Last Mod: Jan 19/99                                                   *
 *  Distribution without Permission Prohibited.                           *
 *                                                                        *
 **************************************************************************/

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
#include "act.h"
#include "fight.h"
#include "spec_assign.h"

#define OBSERVATORY    30000
#define PORTAL_ROOM    30032
#define LAB_ENTRANCE   30031
#define DUNGEON_MASTER 30001
#define DEATHJESTER    30004
#define POWDERKEG      30004
#define LANDMINE       30005
#define SMOKEBOMB      30006
#define BOTMAZE        30001
#define TOPMAZE        30031
#define BLOODWORM      30002

int c_keg(OBJ *keg, CHAR *ch,int cmd, char *arg) {
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR *vict;

  if(!ch) return (FALSE);
  if(cmd!=CMD_THROW) return (FALSE);

  argument_interpreter (arg,arg1,arg2);

  if (!isname (arg1,OBJ_NAME(keg))) return (FALSE);
  if (keg!= EQ(ch,HOLD)) return (FALSE);

  if (!(vict = get_char_room_vis(ch,arg2))) {
     send_to_char("Throw it at whom??\n\r",ch);
     return(TRUE);
  }

  if(ch==vict) {
    send_to_char("Haha, very funny.\n\r",ch);
    return(TRUE);
  }

  if(IS_NPC(vict)) {
    send_to_char("This weapon won't work on mobs.\n\r",ch);
     return(TRUE);
  }

  if(!IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) && !CHAOSMODE) {
    send_to_char("It won't work here.\n\r",ch);
    return(TRUE);
  }

  extract_obj(unequip_char(ch,HOLD));

  if(GET_DEX(ch)>number(1,21)) {
    if(GET_DEX(vict)>number(4,32)) {
      act("$n throws a powderkeg at $N, but $E nimbly dodges!",  FALSE, ch, 0, vict, TO_NOTVICT);
      act("You pull out a powderkeg and throw it at $N, but $E nimbly dodges!",  FALSE, ch, 0, vict, TO_CHAR);
      act("$n throws a powderkeg at you but you nimbly dodge it!!",  FALSE, ch, 0, vict, TO_VICT);
      return(TRUE);
    }

    act("$n throws a powderkeg into $N's face, BOOOOOM!!!",  FALSE, ch, 0, vict, TO_NOTVICT);
    act("You pull out a powderkeg and throw it into $N's face, BOOOOOM!!!",  FALSE, ch, 0, vict, TO_CHAR);
    act("$n throws a lit powderkeg in your face, BOOOOOM!!!",  FALSE, ch, 0, vict, TO_VICT);
    damage(ch, vict, (GET_HIT(vict)/10)*number(2,6), TYPE_UNDEFINED,DAM_NO_BLOCK);
    return(TRUE);
  }

  act("$n throws a powderkeg at $N, but it misses!!!",  FALSE, ch, 0, vict, TO_NOTVICT);
  act("You pull out a powderkeg and throw it at $N, but miss!!!",  FALSE, ch, 0, vict, TO_CHAR);
  act("$n throws a powderkeg at you but misses!!!",  FALSE, ch, 0, vict, TO_VICT);
  return(TRUE);
}

int c_mine(OBJ *mine, CHAR *ch,int cmd, char *arg) {
  char arg1[MAX_INPUT_LENGTH];
  CHAR *vict,*next_vict;
  int explode=FALSE;

  if(cmd!=MSG_ENTER && cmd!=CMD_DROP && cmd!=CMD_GET && cmd!=CMD_TAP) return (FALSE);
  if(!ch) return (FALSE);
  arg=one_argument(arg,arg1);

  if(cmd==CMD_DROP) {
    if(!*arg1) return FALSE;
    if(!isname(arg1,OBJ_NAME(mine))) return FALSE;
    if(ch!=mine->carried_by) return FALSE;

    mine->obj_flags.value[3]=number(1,4);
    act("$n pulls out a small landmine and places it on the floor.",  FALSE, ch,0, 0, TO_ROOM);
    act("You pull out a landmine and place it on the floor.",  FALSE, ch, 0, 0, TO_CHAR);
    obj_to_room(obj_from_char(mine),CHAR_REAL_ROOM(ch));
    return(TRUE);
  }

  if(mine->in_room==NOWHERE) return FALSE;
  if(!IS_SET(world[mine->in_room].room_flags, CHAOTIC) && !CHAOSMODE) return(FALSE);
  if(!IS_MORTAL(ch)) return(FALSE);

  if(cmd==MSG_ENTER) {
    if(mine->obj_flags.value[3]>0) {
       mine->obj_flags.value[3]--;
      send_to_room("\n\rThe landmine sends out a sharp beep!\n\r",mine->in_room);
      return(FALSE);
    }
    else explode=TRUE;
  }

  if(cmd==CMD_GET) {
    if(!*arg1) return FALSE;
    if(isname(arg1, OBJ_NAME(mine)) || !strcmp(arg1,"all")) explode=TRUE;
  }

  if(cmd==CMD_TAP) {
    if(!*arg1) return FALSE;
    if(isname(arg1, OBJ_NAME(mine))) explode=TRUE;
  }

  if(explode) {

    if(!number(0,6)) {
       send_to_room("\n\rThe landmine malfunctions and melts into a puddle of metal.\n\r",mine->in_room);
       extract_obj(mine);
       return TRUE;
    }

    send_to_room("\n\rThe landmine explodes, sending bits of shrapnel everywhere!!!\n\r",mine->in_room);
    for(vict = world[mine->in_room].people;vict;vict = next_vict) {
      next_vict=vict->next_in_room;
      send_to_char("Tiny metal splinters tear deep gashes into you!!\n\r", vict);
      damage(vict,vict,(GET_HIT(vict)/5)*number(1,3),TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
    extract_obj(mine);
    if(cmd==CMD_TAP || cmd==CMD_GET) return TRUE;
  }
  return FALSE;
}

int c_smoke(OBJ *smoke, CHAR *ch,int cmd, char *arg) {
  char arg1[MAX_INPUT_LENGTH];
  CHAR *vict,*next_vict;

  if(cmd!=CMD_DROP) return (FALSE);
  if(!ch) return (FALSE);
  arg=one_argument(arg,arg1);

  if(cmd==CMD_DROP) {
    if(!*arg1) return FALSE;
    if(!isname(arg1,OBJ_NAME(smoke))) return FALSE;
    if(ch!=smoke->carried_by) return FALSE;

    smoke->obj_flags.value[3]=number(1,4);
    act("$n pulls out a smokebomb and smashes it on the floor.",  FALSE, ch,0, 0, TO_ROOM);
    act("You pull out a smokebomb and smash it on the floor.",  FALSE, ch, 0, 0, TO_CHAR);
    obj_to_room(obj_from_char(smoke),CHAR_REAL_ROOM(ch));

    if(smoke->in_room==NOWHERE) return FALSE;
    if(!IS_SET(world[smoke->in_room].room_flags, CHAOTIC) && !CHAOSMODE) return(FALSE);

    send_to_room("\n\rThe smokebomb explodes, filling the room with blinding fumes!!!\n\r",smoke->in_room);
    for(vict = world[smoke->in_room].people;vict;vict = next_vict) {
      next_vict=vict->next_in_room;
      send_to_char("The gas stings you in the eyes!!\n\r", vict);
      damage(vict,vict,(GET_HIT(vict)/12)*number(1,3),TYPE_UNDEFINED,DAM_NO_BLOCK);
      spell_blindness(30, ch, vict, 0);
          }
    extract_obj(smoke);
   }
  return TRUE;
}

int lab_portal(int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_STRING_LENGTH];
  struct descriptor_data *d;

  if (!ch) return FALSE;
  if (cmd != CMD_ENTER || !AWAKE (ch)) return FALSE;
  if (IS_NPC(ch) && CHAR_REAL_ROOM(ch)!=real_room(OBSERVATORY)) return FALSE;

  if(strcmp(arg, " portal")) return FALSE;

  if (GET_POS(ch) == POSITION_RIDING) {
     send_to_char("You must dismount to enter The Labyrinth!\n\r", ch);
     return TRUE;
  }

  act ("$n enters the portal and vanishes!", TRUE, ch, 0, 0, TO_ROOM);
  send_to_char ("You enter the portal and are suddenly transported!\n\r",ch);
  char_from_room (ch);
  char_to_room (ch, real_room(LAB_ENTRANCE));
  if (!IS_NPC(ch)) sprintf(buf,"\n\rThe Dungeonmaster (chaos) [ ** %s has entered the Labyrinth ** ]\n\r\n\r ",GET_NAME(ch));
  else sprintf(buf,"\n\rThe Dungeonmaster (chaos) [ ** %s has entered the Labyrinth ** ]\n\r\n\r ",CAP(GET_SHORT(ch)));
  act ("$n appears seemingly out of nowhere.", TRUE,ch,0,0,TO_ROOM);
  GET_HIT(ch)=GET_MAX_HIT(ch);
  do_look(ch,"",0);


  for (d = descriptor_list; d; d = d->next) {
    if (d->character && (d->character != ch) && !d->connected &&
        world[CHAR_REAL_ROOM(d->character)].number!=PORTAL_ROOM
        && ( (!IS_SET(d->character->specials.pflag, PLR_NOSHOUT)
        && IS_SET(d->character->specials.pflag, PLR_CHAOS) )
             || d->original ))   {
          COLOR(d->character,15);
          send_to_char(buf, d->character);
    ENDCOLOR(d->character);
    }
  }

  return TRUE;
}



int lab_deathjester(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_STRING_LENGTH];
  CHAR *victim=0;

  if(!AWAKE(mob)) return FALSE;

  if(cmd==MSG_LEAVE && mob==ch && mob->master) {
    return TRUE;
  }


  if(cmd==MSG_MOBACT && mob->master && mob->master->specials.fighting && !number(0,1)) {
    victim=get_random_victim(mob);
    if (!victim) return FALSE;
    switch (number(0,8)) {
      case 0:
        act("$n says 'Let's see if we can do something about those nasty scratches, shall we?'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'Let's see if we can do something about those nasty scratches, shall we?'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        spell_miracle(GET_LEVEL(mob),mob,victim,0);
        return FALSE;
        break;
      case 1:
        act("$n says 'Come on....let's see some decent blood here!!'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'Come on....let's see some decent blood here!!'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        spell_dispel_sanct(GET_LEVEL(mob),mob,victim,0);
        return FALSE;
        break;
      case 2:
        act("$n says 'I think spells give wimpy magicians an unfair advantage'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'I think spells give wimpy magicians an unfair advantage'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        spell_disenchant(GET_LEVEL(mob),mob,victim,0);
        return FALSE;
        break;
      case 3:
        act("$n says 'Hang on a minute'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'Hang on a minute'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        spell_paralyze(GET_LEVEL(mob),mob,victim,0);
        return FALSE;
        break;
      case 4:
        act("$n says 'Bless you, my child'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'Bless you, my child'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        spell_bless(GET_LEVEL(mob),mob,victim,0);
        return FALSE;
        break;
      case 5:
        act("$n says 'I haven't had this much fun since uncle Albert died!'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'I haven't had this much fun since uncle Albert died!'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        spell_vampiric_touch(GET_LEVEL(mob),mob,victim,0);
        return FALSE;
        break;

      case 6:
        act("$n says 'Great fight, let's make it last a little longer!'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'Great fight, let's make it last a little longer!'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        spell_great_miracle(GET_LEVEL(mob),mob,victim,0);
        return FALSE;
        break;

      case 7:
        act("$n says 'One free sanc spell coming up!!'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'One free sanc spell coming up!!'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        spell_sanctuary(GET_LEVEL(mob),mob,victim,0);
        return FALSE;
        break;
      case 8:
        sprintf (buf, "say Let's fight!!");
        command_interpreter (mob, buf);
        sprintf (buf, "assist %s", GET_NAME(victim));
        command_interpreter (mob, buf);
        return FALSE;
        break;
    }
  }


  if (cmd==MSG_TICK) {
    if(!number(0,4)) {
      sprintf (buf, "say I'm bored");
      command_interpreter (mob, buf);
      sprintf (buf, "snore");
      command_interpreter (mob, buf);
      sprintf (buf, "follow %s", GET_NAME(mob));
      command_interpreter (mob, buf);
      return FALSE;
    }

    switch (number(0,4)) {
      case 0:
        act("$n says 'I had friends who died face down in the mud, mister!'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'I had friends who died face down in the mud, mister!'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
      case 1:
        act("$n grins wickedly and rubs $s hands together."
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You grin wickedly and rub your hands together."
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
      case 2:
        act("$n cackles quietly to $mself."
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You cackle quietly to yourself."
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
      case 3:
        act("$n chuckles politely."
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You chuckle politely."
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
    }
  }

  if(cmd==MSG_ENTER) {
    if(mob==ch) return FALSE;
    if (number(0,6)) return FALSE;
    victim=get_random_victim(mob);
    if(victim==mob->master) return FALSE;
    sprintf (buf, "say I'm going to play with YOU!!!!");
    command_interpreter (mob, buf);
    sprintf (buf, "cac");
    command_interpreter (mob, buf);
    sprintf (buf, "follow %s", GET_NAME(victim));
    command_interpreter (mob, buf);
    return FALSE;
  }
  return FALSE;
}


int lab_dungeon_master(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  struct descriptor_data *d;
  char buf[MAX_STRING_LENGTH];
  OBJ *tmp;
  CHAR *attacker;
  int i,gold=0,vnum;

  if(!AWAKE(mob)) return FALSE;

  if (cmd==CMD_LIST) {
    if (!CAN_SEE(mob, ch)) {
      act("$n says, 'Sorry, I don't deal with invisibles!'",
          FALSE, mob, 0, 0, TO_ROOM);
      return(TRUE);
    }

    act("$n tells you 'You can buy:'", FALSE, mob, 0, ch, TO_VICT);
    send_to_char("- a powderkeg      25000\n\r", ch);
    send_to_char("- a landmine       25000\n\r", ch);
    send_to_char("- a smokebomb      25000\n\r", ch);
    return(TRUE);
  }

  if (cmd==CMD_BUY) { /* Buy */
    arg = one_argument(arg, buf);
    if (!*buf) {
        send_to_char("Buy what?\n\r",ch);
        return(TRUE);
    }
    if (!strcmp(buf,"powderkeg")) {
      gold=25000;
      vnum=POWDERKEG;
    }
    if(!strcmp(buf,"landmine")) {
      gold=25000;
      vnum=LANDMINE;
    }
    if(!strcmp(buf,"smokebomb")) {
      gold=25000;
      vnum=SMOKEBOMB;
    }
    if(gold==0) {
      act("$n tells you, 'Sorry, I don't have that, try list.'",
          FALSE, mob, 0, ch, TO_VICT);
      return(TRUE);
    }

    if (GET_GOLD(ch)<gold) {
      send_to_char("You don't have enough money.\n\r",ch);
      return(TRUE);
    }

    tmp=read_object(vnum,VIRTUAL);
    obj_to_char(tmp,ch);
    sprintf(buf,"That will cost you %d coins.\n\r",gold);
    send_to_char(buf,ch);
    GET_GOLD(ch)-=gold;
    act("$n gives $p to $N.",1,mob,tmp,ch,TO_NOTVICT);
    act("$n gives $p to you.",1,mob,tmp,ch,TO_VICT);
    return(TRUE);
  }

  if (cmd==MSG_TICK) {
    for(i= BOTMAZE; i<TOPMAZE; i++) {
      if(IS_SET(world[real_room(i)].room_flags, NO_MAGIC) && chance(40)) {
        REMOVE_BIT(world[real_room(i)].room_flags, NO_MAGIC);
        continue;
      }

      if (!IS_SET(world[real_room(i)].room_flags, NO_MAGIC) && chance(10))
        SET_BIT(world[real_room(i)].room_flags, NO_MAGIC);
    }

    for(d=descriptor_list; d; d = d->next) {
      if (d->character && (d->connected == CON_PLYNG)
          && (CHAR_REAL_ROOM(d->character) != NOWHERE) && (d->character->specials.timer>3)
          && IS_SET(world[CHAR_REAL_ROOM(d->character)].room_flags, CHAOTIC) && IS_MORTAL(d->character)) {
        d->character->specials.timer=0;
        if(CHAR_REAL_ROOM(d->character)!=real_room(LAB_ENTRANCE)) {
          attacker = read_mobile(BLOODWORM,VIRTUAL);
          send_to_char("Suddenly, a dark shape detaches itself from the ceiling and drops down on you!\n\r",d->character);
          act("Suddenly, a dark shape detaches itself from the ceiling and drops down on $n!",1, d->character, 0, 0, TO_ROOM);
          char_to_room(attacker,CHAR_REAL_ROOM(d->character));
          set_fighting(attacker,d->character);
        }

        if(CHAR_REAL_ROOM(d->character)==real_room(LAB_ENTRANCE)) {
          act("$n says 'Fight already, you cowardly chicken!!'"
              ,FALSE, mob, 0, 0, TO_ROOM);
          act("You say 'Fight already, you cowardly chicken!!'"
              ,FALSE, mob, 0, 0, TO_CHAR);

          act("$n grabs $N and throws $M through the west exit.",  FALSE, mob, 0, d->character, TO_NOTVICT);
          act("You grab $N, and throw $M through the west exit.",  FALSE, mob, 0, d->character, TO_CHAR);
          act("$n grabs you and throws you through the west exit.",  FALSE, mob, 0, d->character, TO_VICT);
          char_from_room(d->character);
          char_to_room(d->character,real_room(TOPMAZE-1));

          act("$n comes flying through the east exit and crashes onto the ground."
              ,FALSE, d->character, 0, 0, TO_ROOM);
          do_look(d->character,"",CMD_LOOK);
          damage(mob, d->character, (GET_HIT(d->character)/10)*number(1,3), TYPE_UNDEFINED,DAM_NO_BLOCK);
        }
      }
    }

    switch (number(0,13)) {
      case 0:
        act("$n says 'No one here gets out alive'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'No one here gets out alive'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
      case 1:
        act("$n starts sharpening $s bloody axe."
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You start sharpening your bloody axe."
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
      case 2:
        act("$n says 'Abandon hope all ye who enter here'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'Abandon hope all ye who enter here'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
      case 3:
        act("$n says 'Beware the fool who fears nothing'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'Beware the fool who fears nothing'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
      case 4:
        act("$n says 'Death...the final frontier'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'Death...the final frontier'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
      case 5:
        act("$n says 'Salvation lies in cold steel'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'Salvation lies in cold steel'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
      case 6:
        act("$n says 'Only the strong will survive'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'Only the strong will survive'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
      case 7:
        act("$n says 'Battle is it's own purpose'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'Battle is it's own purpose'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
      case 8:
        act("$n says 'Beware the mad wizard Skelos and his creed'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'Beware the mad wizard Skelos and his creed'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
      case 9:
        act("$n says 'In the end there can be only one'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'In the end there can be only one '"
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
      case 10:
        act("$n says 'Eternal imprisonment...or a quick, painless death'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'Eternal imprisonment...or a quick, painless death'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
      case 11:
        act("$n sighs loudly and closes $s eyes in meditation."
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You sigh loudly and close your eyes in meditation."
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
      case 12:
        act("$n says 'Pity the creature who doesn't fear death'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'Pity the creature who doesn't fear death'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
      case 13:
        act("$n bows $s head in a silent prayer to the gods of Chaos."
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You bow your head in a silent prayer to the gods of Chaos."
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
      case 14:
        act("$n says 'Who wants to live forever?'"
              ,FALSE, mob, 0, 0, TO_ROOM);
        act("You say 'Who wants to live forever?'"
              ,FALSE, mob, 0, 0, TO_CHAR);
        return FALSE;
        break;
    }
  }
  return FALSE;
}

void assign_labyrinth(void) {
  assign_room(OBSERVATORY,   lab_portal);
  assign_room(PORTAL_ROOM,   lab_portal);
  assign_mob(DUNGEON_MASTER, lab_dungeon_master);
  assign_mob(DEATHJESTER,    lab_deathjester);
  assign_obj(POWDERKEG,      c_keg);
  assign_obj(LANDMINE,       c_mine);
  assign_obj(SMOKEBOMB,      c_smoke);
}
