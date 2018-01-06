/*
///       Special Procedure Module           Orig. Date 04-03-97
///                                          Last Mod.  07-04-97
///       By Sumo
///       written for RoninMUD
///
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
#include "enchant.h"

#define PYRAK_DRAGON   17371
#define PYRAK_TARGET   17370
#define KIRI_TORIN     17322
#define ELF_MAIDEN     17323
#define TROLL_CHEF     17342
#define BABY_DRAGON    17360
#define MARCUS         17308
#define TIGER          17301
#define MIRROR         17310
#define MIRROR_ROOM    17310
#define MIRROR_TARGET  17312
#define FANG           17398
#define ANVIL_ROOM     17330
#define HAMMER         17396
#define TOOTH          17395
#define MITHRIL_BAR    17394
#define ANVIL          17392
#define KEY_DECAY      17316
#define VAULT_ROOM     17338
#define KIRI_VAULT     17339
#define GOLEM          17300
#define PAN            17342
#define DRAGON_TOOTH   17346

void stop_riding(CHAR *ch,CHAR *vict);
int is_shop(CHAR *mob);

int pyrak_dragon (CHAR *pyrak, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*next_vict;
  struct descriptor_data *d;

  if (cmd!=MSG_MOBACT) return FALSE;
  if (!pyrak->specials.fighting) return FALSE;
  switch (number(1,4)) {
    case 1:
      vict=get_random_victim_fighting(pyrak);
      if(vict && vict!=pyrak->specials.fighting && !number(0,3) ) {
        d=vict->desc;
        act ("$n fans its great wings and sends $N tumbling out of the cave!",FALSE,pyrak,0,vict,TO_NOTVICT);
        act ("A blast of air fron $n's mighty wings sends you tumbling out of the room!",FALSE,pyrak,0,vict,TO_VICT);
        act ("You knock $N out of the room with a simple flap of your wings.",FALSE,pyrak,0,vict,TO_CHAR);
        damage(pyrak,vict,150,TYPE_UNDEFINED,DAM_NO_BLOCK);

        if(!d) return FALSE;
        if(d->connected!=CON_PLYNG) return FALSE; /* If char died */

        if(vict->specials.riding) stop_riding(vict,vict->specials.riding);
        char_from_room (vict);
        char_to_room (vict, real_room (PYRAK_TARGET));
        do_look(vict,"",CMD_LOOK);
        GET_POS(vict)=POSITION_SITTING;
      }
      break;

     case 2:
       if(!number(0,5)) {
         act("You scorch everyone with a pitiful belch of flame.",
              1, pyrak, 0, 0, TO_CHAR);
         for(vict = world[pyrak->in_room_r].people; vict;vict = next_vict) {
           next_vict = vict->next_in_room;
           if(vict !=pyrak && !IS_NPC(vict) && GET_LEVEL(vict)<LEVEL_IMM) {
             act("$n opens $s huge maw and scorches you with $s breath!", 1, pyrak, 0, vict, TO_VICT);
             damage(pyrak, vict, 40, TYPE_UNDEFINED,DAM_FIRE);
           }
         }
       }
       break;

     case 3:
       vict=get_random_victim_fighting(pyrak);
       if(vict && vict!=pyrak->specials.fighting && !number(0,3) ) {
         act ("$n opens $s huge maw and wreathes $N in flames!",FALSE,pyrak,0,vict,TO_NOTVICT);
         act ("$n opens $s huge maw and wreathes your body in flames!",FALSE,pyrak,0,vict,TO_VICT);
         act ("You roast $N!",FALSE,pyrak,0,vict,TO_CHAR);
         damage(pyrak,vict,200,TYPE_UNDEFINED,DAM_NO_BLOCK);
       }
       break;

     case 4:
      if(!number(0,5)) {
        act ("You slam your mighty tail against the ground. Foolish humans.",FALSE,pyrak,0,0,TO_CHAR);
        act ("$n's tail slams heavily against the floor, shaking the room violently!",FALSE,pyrak,0,0,TO_ROOM);
        for(vict = world[pyrak->in_room_r].people; vict;vict = next_vict) {
          next_vict = vict->next_in_room;
          if(vict !=pyrak && !IS_NPC(vict) && GET_LEVEL(vict)<LEVEL_IMM) {
            act ("$n's tail slams heavily against the floor, knocking you off your feet!",FALSE,pyrak,0,vict,TO_VICT);
            damage(pyrak,vict,100,TYPE_UNDEFINED,DAM_NO_BLOCK);
            stop_fighting(vict);
            WAIT_STATE(vict, PULSE_VIOLENCE*2);
            GET_POS(vict)=POSITION_SITTING;
          }
        }
      }
      break;
  }
  return FALSE;
}

int baby_dragon (CHAR *baby, CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  if (cmd==MSG_MOBACT) {
    if (!baby->specials.fighting) return FALSE;
    vict=get_random_victim_fighting(baby);
    if(vict && vict!=baby->specials.fighting && !number(0,2) ) {
      act ("$n opens $s maw and scorches $N with $s breath!",FALSE,baby,0,vict,TO_NOTVICT);
      act("$n opens $s maw and scorches you with $s breath!", FALSE, baby, 0, vict, TO_VICT);
      act ("You belch fire at $N!",FALSE,baby,0,vict,TO_CHAR);
      damage(baby,vict,number(20,40),TYPE_UNDEFINED,DAM_FIRE);
    }
  }
  return FALSE;
}

int troll_chef(CHAR *troll, CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*next_vict;
  if(cmd!=MSG_MOBACT) return FALSE;

  switch(number(1,2)) {
     case 1:
       vict=get_random_victim_fighting(troll);
       if(vict && vict!=troll->specials.fighting && !number(0,4)) {
         act("$n whacks $N upside the head with his frying pan!",0,troll,0,vict,TO_NOTVICT);
         act("You give $N a taste of your frying pan!",0,troll,0,vict,TO_CHAR);
         act("$n smacks you upside the head with his frying pan!",0,troll,0,vict,TO_VICT);
         damage(troll,vict,100,TYPE_UNDEFINED,DAM_NO_BLOCK);
         GET_MANA (vict) = MAX(0, GET_MANA (vict) - 200);
       }
       break;

     case 2:
       if (!number(0,5)) {
         for(vict = world[troll->in_room_r].people; vict;vict = next_vict) {
           next_vict = vict->next_in_room;
           if(vict !=troll && !IS_NPC(vict) && GET_LEVEL(vict)<LEVEL_IMM) {
             act("$n flails madly around the room, catching $N in the teeth with $s frying pan!",0,troll,0,vict,TO_NOTVICT);
             act("You smack $N in the teeth with your frying pan!",0,troll,0,vict,TO_CHAR);
             act("$n flails about the room, catching you in the teeth with $s frying pan!",0,troll,0,vict,TO_VICT);
             damage(troll,vict,20,TYPE_UNDEFINED,DAM_NO_BLOCK);
             GET_MANA (vict) = MAX(0, GET_MANA (vict) - 50);
           }
         }
       }
       break;
  }
  return FALSE;
}

int mirror (int room, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  if(!ch) return(FALSE);
  if (cmd != CMD_ENTER || !AWAKE (ch)) return FALSE;
  one_argument (arg,buf);
  if (strcmp(buf,"mirror")) return FALSE;

  if(GET_POS(ch)==POSITION_RIDING) {
     send_to_char("Dismount first.\n\r",ch);
     return TRUE;
  }
  act ("$n walks right into the mirror and disappears!", TRUE, ch, 0, 0, TO_ROOM);
  send_to_char ("You pass right through the mirror and end up somewhere else!\n\r",ch);
  act ("$n disappears into the mirror's rippling surface.",TRUE,ch,0,0,TO_ROOM);
  char_from_room (ch);
  char_to_room (ch, real_room(MIRROR_TARGET));
  act ("$n appears out of nowhere.",TRUE,ch,0,0,TO_ROOM);
  do_look(ch,"",CMD_LOOK);
  return TRUE;
}

int hammer(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  if (!ch) return FALSE;
  if (!obj) return FALSE;
  if (cmd != CMD_USE) return FALSE;
  if (V_ROOM(ch) != ANVIL_ROOM) return FALSE;
  if (EQ(ch, HOLD) != obj) return FALSE;
  if (GET_CLASS(ch) != CLASS_MAGIC_USER) return FALSE;

  char buf[MIL];

  one_argument(argument, buf);

  if (!isname(buf, OBJ_NAME(obj))) return FALSE;

  bool mithril_bar = FALSE;
  bool tooth = FALSE;

  for (OBJ *tmp_obj = ch->carrying, *next_obj = NULL; tmp_obj; tmp_obj = next_obj) {
    next_obj = tmp_obj->next_content;

    if (!mithril_bar && V_OBJ(tmp_obj) == 17394) mithril_bar = TRUE;
    if (!tooth && V_OBJ(tmp_obj) == 17395) tooth = TRUE;
    if (mithril_bar && tooth) break;
  }

  if (tooth && mithril_bar) {
    for (OBJ *tmp_obj = ch->carrying, *next_obj = NULL; tmp_obj; tmp_obj = next_obj) {
      next_obj = tmp_obj->next_content;

      if (mithril_bar && V_OBJ(tmp_obj) == 17394) {
        extract_obj(obj_from_char(tmp_obj));
        mithril_bar = FALSE;
      }
      if (tooth && V_OBJ(tmp_obj) == 17395) {
        extract_obj(obj_from_char(tmp_obj));
        tooth = FALSE;
      }
      if (!mithril_bar && !tooth) break;
    }

    act("$n recites a strange incantation, and in a burst of light the tooth\n\rand mithril bar fuse into a strange looking weapon.", FALSE, ch, 0, 0, TO_ROOM);
    act("You forge the Tooth and Mithril Bar into a rather unusual mystical weapon.", FALSE, ch, 0, 0, TO_CHAR);
    act("Your hammer crumbles to dust after its purpose has been served.", FALSE, ch, 0, 0, TO_CHAR);

    extract_obj(unequip_char(ch, HOLD));
    OBJ *fang = read_object(17398, VIRTUAL);
    if (fang) obj_to_room(fang, real_room(ANVIL_ROOM));

    return TRUE;
  }
  else {
    send_to_char("Nothing seems to happen.\n\r", ch);

    return TRUE;
  }

  return FALSE;
}

int fang(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  if (cmd != MSG_MOBACT) return FALSE;

  CHAR *owner = obj->equipped_by;
  if (!owner) return FALSE;

  CHAR *vict = GET_OPPONENT(owner);
  if (!vict) return FALSE;

  if (!SAME_ROOM(owner, vict)) return FALSE;

  if (GET_LEVEL(owner) < 11) return FALSE;

  if (number(0, 12)) return FALSE;

  act("$n's $p bursts into searing flames and engulfs $N!", FALSE, owner, obj, vict, TO_ROOM);
  act("Your fang bursts into searing flames and envelops $N!", FALSE, owner, 0, vict, TO_CHAR);

  spell_fireball(GET_LEVEL(owner), owner, vict, 0);

  return FALSE;
}

int elf_maiden(CHAR *mob, CHAR *vict, int cmd, char *arg) {

   if(cmd!=MSG_DIE) return FALSE;
   if(!vict) return FALSE;
   act("You curse $N with your dying breath.",1,mob,0,vict,TO_CHAR);
   act("$n curses you with $s dying breath!",1,mob,0,vict,TO_VICT);
   act("$n curses $N with $s dying breath!",1,mob,0,vict,TO_NOTVICT);
   damage(mob, vict, 400, TYPE_UNDEFINED,DAM_NO_BLOCK);
   return FALSE;
}

int key_decay(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  bool carry=FALSE,wear=FALSE;
  CHAR *vict;
  int dam;

  if(cmd!=MSG_TICK) return FALSE;

  if (obj->equipped_by) {
    wear=TRUE;
    vict=obj->equipped_by;
  }
  else if (obj->carried_by) {
    carry=TRUE;
    vict=obj->carried_by;
  }
  else return FALSE;

  if(IS_NPC(vict) && V_MOB(vict)==17322) return FALSE;

  if(obj_proto_table[obj->item_number].number > 1) {
    act("The $o explodes with fiery force!",FALSE,vict,obj,0,TO_CHAR);
    act("$n's $o explodes with fiery force!",FALSE,vict,obj,0,TO_ROOM);
    if(wear) extract_obj(unequip_char (vict, HOLD));
    if(carry) extract_obj (obj);
    dam=MIN(50,GET_HIT(vict)-1);
    damage(vict,vict,dam,TYPE_UNDEFINED,DAM_NO_BLOCK);
  }
  return FALSE;
}

int volcano_rooms(int room, CHAR *ch, int cmd, char *arg)
{
  OBJ *k;
  char buf[100];
  CHAR *tmp_char;

  if(!ch) return(FALSE);
  for(k = world[CHAR_REAL_ROOM(ch)].contents; k ;
      k = world[CHAR_REAL_ROOM(ch)].contents)
    {
      sprintf(buf, "The %s vanishes mysteriously.\n\r",
        OBJ_SHORT(k));
      for(tmp_char = world[CHAR_REAL_ROOM(ch)].people; tmp_char;
    tmp_char = tmp_char->next_in_room)
  if (CAN_SEE_OBJ(tmp_char, k))
    send_to_char(buf,tmp_char);
      obj_from_room(k);
      obj_to_room(k,real_room(VAULT_ROOM));
    }
  if(cmd != CMD_DROP)
    return(FALSE);

  do_drop(ch, arg, cmd);

  for(k = world[CHAR_REAL_ROOM(ch)].contents; k ;
      k = world[CHAR_REAL_ROOM(ch)].contents)
    {
      sprintf(buf, "The %s vanishes mysteriously.\n\r",OBJ_SHORT(k));
      for(tmp_char = world[CHAR_REAL_ROOM(ch)].people; tmp_char;
    tmp_char = tmp_char->next_in_room)
  if (CAN_SEE_OBJ(tmp_char, k))
    send_to_char(buf,tmp_char);
      obj_from_room(k);
      obj_to_room(k,real_room(VAULT_ROOM));
    }
  return (TRUE);
}


int kiri_torin(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  CHAR *victim;
  OBJ *obj;
  int pos;
  char buf[MAX_INPUT_LENGTH];
  if(cmd!=MSG_MOBACT) return FALSE;
  if(!number(0,6) && mob->specials.fighting) {
    if((victim = get_random_victim(mob))) {
      pos = number(0,17);
      if (victim->equipment[pos]) {
        obj=victim->equipment[pos];
        obj_to_room(unequip_char(victim,pos),real_room(KIRI_VAULT));
        /* Added gearsteal log, Solmyr - 2009 */
        sprintf(buf, "WIZINFO: %s puts %s's %s in %d", GET_NAME(mob), GET_NAME(victim), OBJ_SHORT(obj), world[real_room(KIRI_VAULT)].number);
        log_s(buf);
        obj->log = TRUE;
        act("$n's eyes glow with a fiendish red light as he grabs your $p!",1,mob,obj,victim,TO_VICT);
        act("$n's eyes glow with a fiendish red light!",1,mob,obj,victim,TO_NOTVICT);
        act("You wrest $p from $n's clutches.",1,mob,obj,victim,TO_CHAR);
        save_char(victim, NOWHERE);
      }
    }
  }
  return FALSE;
}


int tiger(CHAR *mob,CHAR *ch, int cmd, char *arg) {
  CHAR *vict;
  if(cmd !=MSG_MOBACT) return FALSE;
  vict=get_random_victim_fighting(mob);
  if(number(0,5)==0 && vict) {
    act("$n rakes $N viciously with $s hind claws!",0,mob,0,vict,TO_NOTVICT);
    act("You rake at $N with your hind claws.",0,mob,0,vict,TO_CHAR);
    act("$n savagely rakes at you with $s hind claws!",0,mob,0,vict,TO_VICT);
    hit(mob, vict, TYPE_HIT);
  }
  return FALSE;
}


int marcus(CHAR *mob,CHAR *ch, int cmd, char *arg) {
  CHAR *vict,*next_vict;

  switch (cmd) {
    case CMD_SONG:
      if (!ch) return FALSE;
      if(!mob->specials.fighting) return FALSE;
      if(number(0,4)) return FALSE;
      act("$n waves his hands and utters an arcane mantra.\n\r",1, mob, 0, 0, TO_ROOM);
      act("You chant an arcane mantra to slay these foolish adventurers.",1, mob, 0, 0, TO_CHAR);
      act("$N covers his ears and $n's song is amplified into a deafening shriek!",1, ch, 0, mob, TO_ROOM);
      act("$N covers his ears and your song is amplified into a deafening shriek!",1, ch, 0, mob, TO_CHAR);
      for(vict = world[mob->in_room_r].people; vict;vict = next_vict) {
        next_vict = vict->next_in_room;
        if(vict!=mob && !IS_NPC(vict) && GET_LEVEL(vict)<LEVEL_IMM) {
          damage(mob, vict, 100, TYPE_UNDEFINED,DAM_NO_BLOCK);
        }
      }
      return TRUE;
      break;

    case CMD_CAST:
      if(!ch) return FALSE;
      if (number(0,5)) return FALSE;
      if(ch==mob->specials.fighting) return FALSE;
      act("$n leers at $N and says 'Begone, fool!'",0,mob,0,ch,TO_NOTVICT);
      act("You gesture at $N, and $E dissapears!",0,mob,0,ch,TO_CHAR);
      act("$n leers angrily at you and says 'Begone, fool!'",0,mob,0,ch,TO_VICT);

      if(ch->specials.riding) stop_riding(ch,ch->specials.riding);
      char_from_room(ch);
      char_to_room(ch,real_room(17311));
      do_look(ch,"",CMD_LOOK);
      act("$n appears in the middle of the room.", FALSE, ch, 0, 0, TO_ROOM);
      return TRUE;
      break;
    }
  return FALSE;
}

int golem_block(CHAR *golem, CHAR *ch, int cmd, char *arg) {
  if(!ch) return FALSE;
  if(cmd==CMD_NORTH) {
    act("$n silently steps in front of your path.",0,golem,0,ch,TO_VICT);
    act("$n silently steps in $N's path.",0,golem,0,ch,TO_ROOM);
    act("You step in front of $N, blocking $S way.",0,golem,0,ch,TO_CHAR);
    return TRUE;
  }
  return FALSE;
}

int pan(OBJ *obj,CHAR *ch, int cmd, char *argument) {
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

  act("$N's eyes cross  as $n's Frying Pan connects with $S head!",FALSE,ch,0,vict,TO_NOTVICT);
  act("Your eyes cross  as $n's Frying Pan connects with your head!",FALSE,ch,0,vict,TO_VICT);
  act("As you strike $N, $S eyes cross and $E looks decidedly dazed!",FALSE,ch,0,vict,TO_CHAR);

  GET_MANA(vict)=MAX(0,GET_MANA(vict) - 50);
  GET_MANA(ch)+=25;
  GET_HIT(vict)=MAX(0,GET_HIT(vict) - 50);
  GET_HIT(ch)+=25;
  return FALSE;
}

int dragon_tooth(OBJ *tooth, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *tmp_target, *target, *grp_leader;
  struct enchantment_type_5 ench;

  if(cmd!=MSG_MOBACT && cmd!=MSG_TICK) return FALSE;
  if(!(ch=tooth->equipped_by)) return FALSE;
  if(!(ch->specials.fighting)) return FALSE;

  switch(cmd)
  {
	case MSG_TICK:
	  if(GET_COND(ch,DRUNK)>10)
	  {
#ifdef TEST_SITE
		if (chance(10)) return FALSE;
#else
		if(chance(95)) return FALSE;
#endif
	  }
	  else
	  {
#ifdef TEST_SITE
		if (chance(10)) return FALSE;
#else
		if(chance(98)) return FALSE;
#endif
      }
	  act("$n's $p glows white hot and suddenly releases a series of flaming wisps.",0,ch,tooth,0,TO_ROOM);
	  act("Your $p glows white hot and suddenly releases a series of flaming wisps.",0,ch,tooth,0,TO_CHAR);
	  for(target = world[CHAR_REAL_ROOM(ch)].people; target; target = tmp_target)
	  {
		tmp_target=target->next_in_room;
		if(!IS_AFFECTED(ch,AFF_GROUP) || !ch->master)
		{
		  grp_leader=ch;
	    }
	    else
	    {
		  grp_leader=ch->master;
	    }
	    if(target==grp_leader || target->master==grp_leader)
	    {
	      act("$n's eyes burn red and smoke rises from the ground.",0,target,0,0,TO_ROOM);
	      act("Your eyes burn red and smoke rises from the ground.",0,target,0,0,TO_CHAR);
	      memset(&ench,0,sizeof(struct enchantment_type_5));
	      ench.name     = str_dup("Fire Breath");
	      enchantment_to_char(target,&ench,TRUE);
	      free(ench.name);
	    }
	  }
	  break;
	case MSG_MOBACT:
	  vict=ch->specials.fighting;
	  if(CHAR_REAL_ROOM(ch)!=CHAR_REAL_ROOM(vict)) return FALSE;
	  if(chance(90)) return FALSE;
	  act("A halo of flame erupts from the tip of $n's $p and engulfs $N.",0,ch,tooth,vict,TO_ROOM);
	  act("A halo of flame erupts from the tip of your $p and envelops $N.",0,ch,tooth,vict,TO_CHAR);
	  spell_fireball(GET_LEVEL(ch)+10, ch, vict, 0);
	  break;
  }
  return FALSE;
}

void assign_ctower(void) {
  int i;
  for(i = 17356; i <= 17370; i++) assign_room(i,volcano_rooms);
  for(i = 17372; i <= 17376; i++) assign_room(i,volcano_rooms);
  assign_mob(GOLEM,        golem_block);
  assign_mob(PYRAK_DRAGON, pyrak_dragon);
  assign_mob(ELF_MAIDEN,   elf_maiden);
  assign_mob(TROLL_CHEF,   troll_chef);
  assign_mob(BABY_DRAGON,  baby_dragon);
  assign_mob(MARCUS,       marcus);
  assign_mob(TIGER,        tiger);
  assign_mob(KIRI_TORIN,   kiri_torin);
  assign_room(MIRROR_ROOM, mirror);
  assign_obj(FANG,         fang);
  assign_obj(KEY_DECAY,    key_decay);
  assign_obj(HAMMER,       hammer);
  assign_obj(PAN,          pan);
  assign_obj(DRAGON_TOOTH, dragon_tooth);
}

