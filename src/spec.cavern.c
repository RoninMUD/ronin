/*
** Cavern of Lost Souls, by Liner
**
** Nov 13, 1998 - Liner
**
** 09/20/02, check for chaotic rooms only. -Liner
*/

/*
$Author: ronin $
$Date: 2004/02/05 16:10:16 $
$Header: /home/ronin/cvs/ronin/spec.cavern.c,v 2.0.0.1 2004/02/05 16:10:16 ronin Exp $
$Id: spec.cavern.c,v 2.0.0.1 2004/02/05 16:10:16 ronin Exp $
$Name:  $
$Log: spec.cavern.c,v $
Revision 2.0.0.1  2004/02/05 16:10:16  ronin
Reinitialization of cvs archives

Revision 1.4  2002/09/20 18:36:10  ronin
Added check for CHAOTIC room to do the deathlist creation.

Revision 1.3  2002/09/15 16:32:15  ronin
Addition of deathlist creation for soul eq transfer in int cs_soul_spec.

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
#include "act.h"
#include "fight.h"
#include "spec_assign.h"
#include "mob.spells.h"

#define END_A       16901
#define END_B       16922
#define END_C       16943
#define END_D       16952
#define COFFA_ROOM  16988
#define COFFB_ROOM  16990
#define TAJJO       16801
#define TAJTE       16802
#define SOUL_MOB    16903
#define SOULB_MOB   16901
#define REVENANT    16803
#define HUNTER      16804
#define WOMAN       16806
#define MAN         16807
#define BOY         16809
#define GIRL        16810
#define CLEM_MOB    16811
#define POST_ROOM   16894
#define SCYTHE      16805
#define LASH        16806
#define STONE       16818
#define IT          16825
#define SHOES       16811
#define RING        16828
#define GATESMAN    16812

extern struct time_info_data time_info;

int tajjo_ring(OBJ *ring, CHAR *ch, int cmd, char *arg) {

  if(!ch) return FALSE;

  switch(cmd)
    {
    case MSG_OBJ_WORN:
      act("A swirling cloud of darkness envelops $n for an instant.",FALSE,ch,ring,NULL,TO_NOTVICT);
      act("A swirling cloud of darkness envelops your hand for an instant.",FALSE,ch,ring,NULL,TO_CHAR);
      break;

    case MSG_BEING_REMOVED:
      act("A voice from $n's sigil screams out in sudden agony.",FALSE,ch,ring,NULL,TO_NOTVICT);
      act("A voice from your sigil screams out in agony.",FALSE,ch,ring,NULL,TO_CHAR);
      break;
    }

  return FALSE;
}


int cs_room_spec(int room, CHAR *ch, int cmd, char *arg) {

  if(!ch) return FALSE;

  if(cmd==CMD_DOWN && !ch->master) {
    switch(number(0,3))
    {
    case 0:
      world[room].dir_option[DOWN]->to_room_r = real_room(END_A);
      break;

    case 1:
      world[room].dir_option[DOWN]->to_room_r = real_room(END_B);
      break;

    case 2:
      world[room].dir_option[DOWN]->to_room_r = real_room(END_C);
      break;

    case 3:
      world[room].dir_option[DOWN]->to_room_r = real_room(END_D);
      break;

    }

  }
  return FALSE;
}


int cs_coffina_spec (int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  if(!ch) return(FALSE);

  if (!AWAKE (ch)) return FALSE;


  switch (cmd) {
    case CMD_CRAWL:
      one_argument (arg, buf);
      if(!*buf) return FALSE;
      if(!is_abbrev(buf,"coffin")) return FALSE;


      if (IS_NIGHT) {
          act("$n disappears into the open coffin.",0,ch,0,ch,TO_ROOM);
          act("You crawl through a cramped passage within the open coffin.",0,ch,0,ch,TO_CHAR);
          char_from_room(ch);
          char_to_room(ch, real_room(COFFB_ROOM));
          act("$n emerges from the mouth of an open coffin.",0,ch,0,ch,TO_ROOM);
          do_look(ch,"",CMD_LOOK);
          signal_room(CHAR_REAL_ROOM(ch),ch,MSG_ENTER,"");
          return TRUE;
      }
      if (IS_DAY) {
        act("$n attempts to crawl through the occupied coffin.",FALSE,ch,0,ch,TO_ROOM);
        act("You attempt to crawl through the occupied coffin, but cannot.",FALSE,ch,0,ch,TO_CHAR);
        return TRUE;
      }
      break;

  }
  return FALSE;
}


int cs_coffinb_spec (int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  if(!ch) return(FALSE);

  if (!AWAKE (ch)) return FALSE;

  switch (cmd) {
    case CMD_CRAWL:
      one_argument (arg, buf);
      if(!*buf) return FALSE;
      if(!is_abbrev(buf,"coffin")) return FALSE;

      if (IS_NIGHT) {
          act("$n disappears into the open coffin.",0,ch,0,ch,TO_ROOM);
          act("You crawl through a cramped passage within the open coffin.",0,ch,0,ch,TO_CHAR);
          char_from_room(ch);
          char_to_room(ch, real_room(COFFA_ROOM));
          act("$n emerges from the mouth of an open coffin.",0,ch,0,ch,TO_ROOM);
          do_look(ch,"",CMD_LOOK);
          signal_room(CHAR_REAL_ROOM(ch),ch,MSG_ENTER,"");
          return TRUE;

      }
      if (IS_DAY) {
        act("$n attempts to crawl through the occupied coffin.",FALSE,ch,0,ch,TO_ROOM);
        act("You attempt to crawl through the occupied coffin, but cannot.",FALSE,ch,0,ch,TO_CHAR);
        return TRUE;
      }
      break;
  }
  return FALSE;
}


void death_list(CHAR *ch);
int cs_soul_spec(int room, CHAR *ch, int cmd, char *arg) {
  int soul_room = NOWHERE;
  char buf[MAX_STRING_LENGTH];
  CHAR *soul;

  if(cmd!=MSG_CORPSE) return FALSE;
  if(!ch) return FALSE;
  if(IS_NPC(ch)) return FALSE;
  if(GET_LEVEL(ch)<10) return FALSE; /* so newbies don't deal with it - Linerfix 110202 */

  switch(number(0,7))
    {
    case 0:
      soul_room = 16906;
      break;

    case 1:
      soul_room = 16908;
      break;

    case 2:
      soul_room = 16928;
      break;

    case 3:
      soul_room = 16930;
      break;

    case 4:
      soul_room = 16949;
      break;

    case 5:
      soul_room = 16951;
      break;

    case 6:
      soul_room = 16957;
      break;

    case 7:
      soul_room = 16959;
      break;
  }


  soul = read_mobile(SOUL_MOB,VIRTUAL);

  GET_LEVEL(soul)                 = GET_LEVEL(ch);
  soul->points.max_mana           = GET_MAX_MANA(ch);
  soul->points.max_move           = GET_MAX_MOVE(ch);
  soul->points.max_hit            = GET_MAX_HIT(ch);

  GET_HIT(soul)                   = GET_MAX_HIT(soul);
  GET_MANA(soul)                  = GET_MAX_MANA(soul);
  GET_MOVE(soul)                  = GET_MAX_MOVE(soul);
  GET_SEX(soul)                   = GET_SEX(ch);

  soul->specials.alignment        = ch->specials.alignment;

  if(soul->player.name)
    free (soul->player.name);
  sprintf (buf,"%s soul vision",CAP(GET_NAME(ch)));
  soul->player.name = str_dup(buf);

  if(soul->player.short_descr)
    free (soul->player.short_descr);
  sprintf (buf,"The soul of %s",CAP(GET_NAME(ch)));
  soul->player.short_descr = str_dup(buf);

  REMOVE_BIT(soul->specials.affected_by,AFF_BLIND);
  REMOVE_BIT(soul->specials.affected_by,AFF_HOLD);
  REMOVE_BIT(soul->specials.affected_by,AFF_CURSE);
  REMOVE_BIT(soul->specials.affected_by,AFF_POISON);
  REMOVE_BIT(soul->specials.affected_by,AFF_PARALYSIS);
  REMOVE_BIT(soul->specials.affected_by,AFF_CHARM);
  REMOVE_BIT(soul->specials.affected_by,AFF_GROUP);

  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC)) {
  ch->new.been_killed += 1;
  if(!IS_NPC(ch) && ch->ver3.death_limit) ch->ver3.death_limit++;
  death_list(ch);                                 /* Linerfix 091502, account for dlist on soul */
  }

  move_eq_from_to(ch,soul);
  move_inv_from_to(ch,soul);
  GET_GOLD(soul)+=GET_GOLD(ch);
  GET_GOLD(ch)=0;

  act("The soul of $n appears suddenly from nowhere.",FALSE,soul,0,0,TO_ROOM);

  char_to_room(soul,real_room(soul_room));

  return FALSE;
}


int cs_tajjo_spec(CHAR *tajjo, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  char buf[MAX_INPUT_LENGTH];
  int num;

  if(cmd!=MSG_MOBACT) return FALSE;

  if(number(0,2)==0 && !tajjo->specials.fighting) {
      switch(number(0,2))
      {
      case 0:
        sprintf(buf, "crawl hole");
        command_interpreter (tajjo, buf);
        break;

      case 1:
        sprintf(buf, "crawl tunnel");
        command_interpreter (tajjo, buf);
        break;

      case 2:
        sprintf(buf, "crawl opening");
        command_interpreter (tajjo, buf);
        break;
      }
    return FALSE;
  }

  if(!tajjo->specials.fighting) return FALSE;

  if(!(vict=get_random_victim_fighting(tajjo))) return FALSE;

  if(IS_NPC(vict)) return FALSE;

  if(GET_HIT(vict)<=15) return FALSE;

  if(GET_POS(vict)==POSITION_DEAD) return FALSE;
  num=number(0,6);
  switch(num) {
    case 0:
      act("$n pulls $N near $m and exhales acrid breath down $S throat!",0,tajjo,0,vict,TO_NOTVICT);
      act("$n pulls you near and exhales acrid breath down your throat!",0,tajjo,0,vict,TO_VICT);
      act("You quiver in unimaginable pain as $n rips part of your very soul away.",0,tajjo,0,vict,TO_VICT);
      act("You pull $N near you and exhale acrid breath down $S throat!",0,tajjo,0,vict,TO_CHAR);
      spell_poison(30,vict,vict,0);
      GET_HIT(vict)=15;
      break;

    case 3:
      do_say(tajjo,"Give in now, for your soul will soon be mine.", CMD_SAY);
      break;

    case 6:
      do_say(tajjo,"Eternity in the cold clutches of darkness.", CMD_SAY);
      break;
  }
  return FALSE;
}


int cs_tajte_spec(CHAR *tajte, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  CHAR *vict;
  int num;

  if(cmd!=MSG_MOBACT) return FALSE;

  if(number(0,2)==0 && !tajte->specials.fighting) {

     switch(number(0,2))
      {
      case 0:
        sprintf(buf, "crawl hole");
        command_interpreter (tajte, buf);
        break;

      case 1:
        sprintf(buf, "crawl tunnel");
        command_interpreter (tajte, buf);
        break;

      case 2:
        sprintf(buf, "crawl opening");
        command_interpreter (tajte, buf);
        break;
      }
    return FALSE;
  }

  if(!tajte->specials.fighting) return FALSE;

  if(!(vict=get_random_victim_fighting(tajte))) return FALSE;

  if(IS_NPC(vict)) return FALSE;

  if(GET_HIT(vict)<=15) return FALSE;

  if(GET_POS(vict)==POSITION_DEAD) return FALSE;
  num=number(0,6);
  switch(num) {

    case 0:
      act("$n pulls $N near $m and exhales acrid breath down $S throat!",0,tajte,0,vict,TO_NOTVICT);
      act("$n pulls you near and exhales acrid breath down your throat!",0,tajte,0,vict,TO_VICT);
      act("You quiver in unimaginable pain as $n rips part of your very soul away.",0,tajte,0,vict,TO_VICT);
      act("You pull $N near you and exhale acrid breath down $S throat!",0,tajte,0,vict,TO_CHAR);
      spell_poison(30,vict,vict,0);
      GET_HIT(vict)=15;
      break;

    case 1:
      do_say(tajte,"Pain is the only reality in this world.", CMD_SAY);
      break;

    case 3:
      do_say(tajte,"This will all be over soon.", CMD_SAY);
      break;

    case 6:
      do_say(tajte,"Neui ma na dau, ti dau.  Ma mai mung chut ti mai chut di.", CMD_SAY);
      break;
  }

  return FALSE;
}

int cs_soulb_spec(CHAR *soul, CHAR *ch, int cmd, char *arg) {

  if(ch && cmd==MSG_ENTER && (!number(0,3))) {
      act("$n says 'Look no further, for madness lies therein..'",0,soul,0,ch,TO_VICT);
      return FALSE;
  }
  return FALSE;
}

int cs_revenant_spec(CHAR *revenant, CHAR *ch, int cmd, char *arg) {
  if(ch && cmd==MSG_ENTER) {
      act("\n\r$n intones 'In mortis venit purgatio.'",FALSE,revenant,0,ch,TO_VICT);
      act("$n's eyes roll back to white.",FALSE,revenant,0,ch,TO_VICT);
      return FALSE;
  }
  return FALSE;
}

int cs_hunter_spec(CHAR *hunter, CHAR *ch, int cmd, char *arg) {

  if(ch && cmd==MSG_ENTER) {

    switch(number(0,1))
    {
    case 0:
      act("\n\r$n shakes your hand vigorously.",FALSE,hunter,0,ch,TO_VICT);
      act("$n says 'It's good to see another livin' soul!'",0,hunter,0,ch,TO_VICT);
      break;

    case 1:
      act("\n\r$n shakes your hand vigorously.",FALSE,hunter,0,ch,TO_VICT);
      act("$n says 'Stay awhile and warm your bones, fellow traveler.'",0,hunter,0,ch,TO_VICT);
      break;
    }
    return FALSE;
  }
  return FALSE;
}

int cs_woman_spec(CHAR *woman, CHAR *ch, int cmd, char *arg) {
  static int state = 0;

  if(ch && cmd==MSG_ENTER && state ==0) {
     act("\n\r$n digs halfheartedly at the dying earth.",FALSE,woman,0,ch,TO_VICT);
     state++;
     return FALSE;
   }

  if(cmd==MSG_MOBACT && state) {
    switch (state)
    {
    case 1:
      act("$n sighs loudly and wipes her forehead.",FALSE,woman,0,ch,TO_ROOM);
      state++;
      break;

    case 2:
      act("$n places a handful of roots carefully into her bag.",FALSE,woman,0,ch,TO_ROOM);
      state++;
      break;

    case 3:
      state = 0;
      break;

    }
  }
  return FALSE;
}

int cs_man_spec(CHAR *man, CHAR *ch, int cmd, char *arg) {
  CHAR *boy;
  int found;

  static int state = 0;

  if(ch && cmd==MSG_ENTER) {
    if(!state) {
        act("\n\r$n snores loudly beneath a pile of furs.",FALSE,man,0,ch,TO_ROOM);
        state++;
        return FALSE;
    }
  }

  if(cmd==MSG_MOBACT && state) {
    found=0;
    for (boy=world[CHAR_REAL_ROOM(man)].people; boy; boy=boy->next_in_room) {
      if (IS_NPC(boy) && V_MOB(boy)==16808) {
         found=TRUE;
         break;
      }
    }

    switch (state)
    {
    case 1:
      if(!found) break;
      act("$n bends over the sleeping man and tickles his nose with a feather.",FALSE,boy,0,ch,TO_ROOM);
      state++;
      break;

    case 2:
      act("$n mumbles 'Whuzzat? Hey....zzzzzzz'",FALSE,man,0,ch,TO_ROOM);
      state++;
      break;

    case 3:
      state = 0;
      break;
    }
  }
  return FALSE;
}

int cs_boy_spec(CHAR *boy, CHAR *ch, int cmd, char *arg) {

  if(ch && cmd==MSG_ENTER && (!number(0,2))) {
    act("\n\r$n packs a snowball in his hands and hurls it at your face!",FALSE,boy,0,ch,TO_VICT);
    act("$n says 'GOTCHA!'",0,boy,0,ch,TO_VICT);
  }
  return FALSE;
}

int cs_girl_spec(CHAR *girl, CHAR *ch, int cmd, char *arg) {

  if(ch && cmd==MSG_ENTER && (!number(0,2))) {
    act("\n\r$n beams an innocent smile at you.",FALSE,girl,0,ch,TO_VICT);
    act("$n rolls about in the snow and begins to make a snow angel.",FALSE,girl,0,ch,TO_VICT);
  }
  return FALSE;
}

int cs_post_spec(int room, CHAR *ch, int cmd, char *argument) {
  int found;
  char arg[MAX_STRING_LENGTH];
  CHAR *clem;
  OBJ *obj,*next_item;

  if(cmd!=MSG_TICK && cmd!=CMD_UNKNOWN) return FALSE;

  found=0;
  for (clem=world[room].people; clem; clem=clem->next_in_room)
    {
    if (IS_NPC(clem) && V_MOB(clem)==16811) {
      found=TRUE;
      break;
      }
  }

  if (cmd==MSG_TICK && found) {
    if (!count_mortals_room(clem,1)) {
      for (obj=clem->carrying; obj; obj=next_item) {
        next_item = obj->next_content;
        extract_obj(obj);
      }
      extract_char(clem);
    }
  }
  if(!ch) return FALSE;

  if (!AWAKE (ch)) return FALSE;

  if(cmd==CMD_UNKNOWN) {
    argument = one_argument (argument, arg);
    if(!*arg) return FALSE;
    if(!is_abbrev(arg,"ring")) return FALSE;
    one_argument(argument,arg);
    if(!*arg){
      send_to_char("Ring what?\n\r",ch);
      return TRUE;
    }
    string_to_lower(arg);
    if(!strcmp(arg,"bell")){
      act("You ring the brass bell.",0,ch,0,0,TO_CHAR);
      act("$n rings the brass bell.",0,ch,0,0,TO_ROOM);

    if(found) {
      act("\n\r$n smiles happily.",0,clem,0,0,TO_ROOM);
      return TRUE;
    }

      clem = read_mobile(CLEM_MOB,VIRTUAL);
      char_to_room(clem,room);
      act("\n\rA jolly-looking man stumbles out from behind some junk, smiling brightly at you.",FALSE,clem,0,0,TO_ROOM);

      obj = read_object(16824,VIRTUAL);
      obj_to_char(obj,clem);
      obj = read_object(16823,VIRTUAL);
      obj_to_char(obj,clem);
      obj = read_object(16822,VIRTUAL);
      obj_to_char(obj,clem);
      obj = read_object(16821,VIRTUAL);
      obj_to_char(obj,clem);
      obj = read_object(16820,VIRTUAL);
      obj_to_char(obj,clem);
      obj = read_object(16815,VIRTUAL);
      obj_to_char(obj,clem);
      return TRUE;
    }
  }
  return FALSE;
}

int cs_scythe_spec(OBJ *scythe, CHAR *ch, int cmd, char *arg) {
  CHAR *mob,*owner;

  if (cmd != MSG_MOBACT) return FALSE;
  if(!scythe->equipped_by) return FALSE;
  owner=scythe->equipped_by;

  if (!owner->specials.fighting) return FALSE;
  mob=owner->specials.fighting;
  if(number(0,2)) return FALSE;
    switch(number(0,2))
    {
    case 0:
      act("The cunning edge of your scythe tears deep within $N's vital organs.",1,owner,0,mob,TO_CHAR);
      act("The cunning edge of $n's scythe tears deep within $N's vital organs.",1,owner,0,mob,TO_NOTVICT);
      act("The cunning edge of $n's scythe tears deep within your quivering bowels!",1,owner,0,mob,TO_VICT);
      damage(owner,mob,20,TYPE_UNDEFINED,DAM_PHYSICAL);
      break;

    case 1:
      act("A gout of blood jets from $N's throat as your scythe grazes it.",1,owner,0,mob,TO_CHAR);
      act("A gout of blood jets from $N's throat as $n's scythe grazes it.",1,owner,0,mob,TO_NOTVICT);
      act("A gout of blood jets from your throat as $n's scythe grazes it!",1,owner,0,mob,TO_VICT);
      damage(owner,mob,16,TYPE_UNDEFINED,DAM_PHYSICAL);
      break;

    case 2:
      act("Your scythe catches briefly on a bone as you hack $N repeatedly.",1,owner,0,mob,TO_CHAR);
      act("$n's scythe catches briefly on a bone as $e hacks $N repeatedly.",1,owner,0,mob,TO_NOTVICT);
      act("$n's scythe catches briefly on a bone as $e hacks you repeatedly!",1,owner,0,mob,TO_VICT);
      damage(owner,mob,18,TYPE_UNDEFINED,DAM_PHYSICAL);
      break;
    }
  return FALSE;
}

int cs_lash_spec(OBJ *lash, CHAR *ch, int cmd, char *arg) {
  CHAR *mob,*owner;

  if (cmd != MSG_MOBACT) return FALSE;
  if(!lash->equipped_by) return FALSE;
  owner=lash->equipped_by;
  if (!owner->specials.fighting) return FALSE;
  mob=owner->specials.fighting;

  if(number(0,2)) return FALSE;
    switch(number(0,2))
    {
    case 0:
      act("The bladed tips of your lash peel ribbons of flesh from $N's face.",1,owner,0,mob,TO_CHAR);
      act("The bladed tips of $n's lash peel ribbons of flesh from $N's face.",1,owner,0,mob,TO_NOTVICT);
      act("The bladed tips of $n's lash peel ribbons of flesh from your face!",1,owner,0,mob,TO_VICT);
      damage(owner,mob,20,TYPE_UNDEFINED,DAM_PHYSICAL);
      break;

    case 1:
      act("Your bladed lash gashes $N's neck as it coils briefly around it.",1,owner,0,mob,TO_CHAR);
      act("$n's bladed lash gashes $N's neck as it coils briefly around it.",1,owner,0,mob,TO_NOTVICT);
      act("$n's bladed lash gashes your neck as it coils briefly around it!",1,owner,0,mob,TO_VICT);
      damage(owner,mob,16,TYPE_UNDEFINED,DAM_PHYSICAL);
      break;

    case 2:
      act("$N screams in agony as you flay $M in wild abandon.",1,owner,0,mob,TO_CHAR);
      act("$N screams in agony as $n flays $M in wild abandon.",1,owner,0,mob,TO_NOTVICT);
      act("You scream in agony as $n flays you in wild abandon!",1,owner,0,mob,TO_VICT);
      damage(owner,mob,18,TYPE_UNDEFINED,DAM_PHYSICAL);
      break;
    }
  return FALSE;
}

int cs_stone_spec(OBJ *stone, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;

  if (cmd != MSG_TICK) return FALSE;
  if(!stone->carried_by) return FALSE;
  vict=stone->carried_by;


  if(number(0,3)) return FALSE;
  act("A lucky stone glows brightly and envelops you in a purple shroud.",1,vict,0,0,TO_CHAR);
  act("A lucky stone glows brightly and envelops $n in a purple shroud.",1,vict,0,0,TO_ROOM);

  return FALSE;
}

int cs_it_spec(OBJ *it, CHAR *ch, int cmd, char *arg) {

  if (cmd != MSG_TICK) return FALSE;
  if(it->in_room==NOWHERE) return FALSE;
  if(number(0,3)) return FALSE;
  send_to_room("The object flashes a brilliant orange as it shrieks within the sand!\n\r",it->in_room);
  return FALSE;
}

int cs_shoes_spec(OBJ *shoes, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  if(cmd!=MSG_TICK) return FALSE;
  if(!(vict=shoes->equipped_by)) return FALSE;
  if(!IS_MORTAL(vict)) return FALSE;
  if(world[CHAR_REAL_ROOM(vict)].sector_type==SECT_ARCTIC) {
    vict->points.move=MAX(vict->points.move+25-GET_CON(vict),0);
    vict->points.move=MIN(vict->points.move,vict->points.max_move);
  }
  return FALSE;
}

int cs_gatesman_spec(CHAR *gatesman, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];

  if(!ch) return(FALSE);
  if (!AWAKE(ch)) return FALSE;

  if(cmd==CMD_LIST) {
    act("$N tells you 'You can buy a pass to the air barge for 2000000'",FALSE,ch,0,gatesman,TO_CHAR);
    return TRUE;
  }

  if (cmd==CMD_BUY) {
    one_argument(arg, buf);
    if (!*buf) return FALSE;
    if(strcmp(buf,"pass"))  return FALSE;

    if(GET_GOLD(ch)<2000000) {
      act("\n\r$N tells you 'You do not have enough cash!'",FALSE,ch,0,gatesman,TO_CHAR);
      return TRUE;
    }
    act("\n\r$N thanks you for your purchase and hurries you along.",FALSE,ch,0,gatesman,TO_CHAR);
    act("$N gives $n a pass and hurries $m along to the air barge.",FALSE,ch,0,gatesman,TO_ROOM);
    GET_GOLD(ch)-=2000000;
    char_from_room(ch);
    char_to_room(ch,real_room(16887));
    do_look(ch,"",CMD_LOOK);
    do_say(ch,"Man, it sure is cold up here!",CMD_SAY);
    return TRUE;
  }
  return FALSE;
}

void assign_cavern(void) {
  int i;
  assign_mob(GATESMAN, cs_gatesman_spec);
  assign_mob(TAJJO, cs_tajjo_spec);
  assign_mob(TAJTE, cs_tajte_spec);
  assign_mob(SOUL_MOB, cs_soulb_spec);
  assign_mob(SOULB_MOB, cs_soulb_spec);
  assign_mob(REVENANT, cs_revenant_spec);
  assign_mob(HUNTER, cs_hunter_spec);
  assign_mob(WOMAN, cs_woman_spec);
  assign_mob(MAN, cs_man_spec);
  assign_mob(BOY, cs_boy_spec);
  assign_mob(GIRL, cs_girl_spec);
  for(i = 16800; i < 16884; i++) assign_room(i, cs_soul_spec);
  for(i = 16901; i < 16999; i++) assign_room(i, cs_soul_spec);
  assign_room(16900, cs_room_spec);
  assign_room(COFFA_ROOM, cs_coffina_spec);
  assign_room(COFFB_ROOM, cs_coffinb_spec);
  assign_room(POST_ROOM, cs_post_spec);
  assign_obj(SCYTHE, cs_scythe_spec);
  assign_obj(LASH, cs_lash_spec);
  assign_obj(STONE, cs_stone_spec);
  assign_obj(IT, cs_it_spec);
  assign_obj(SHOES, cs_shoes_spec);
  assign_obj(RING, tajjo_ring);
}

