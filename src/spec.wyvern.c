/***************************************************************************\
*            Special procedure module for the Wyvernspur area               *
*                             by Vishnu                                     *
*                                                                           *
*                       clay@nano.engr.mun.ca                               *
\***************************************************************************/

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
#include "fight.h"
#include "enchant.h"
#include "act.h"
#include "spec_assign.h"

int wyvern_enchantment(ENCH*, CHAR*, CHAR*, int, char*);

int find_obj(int v_num, int virt_room) {
  OBJ *obj;
  for (obj=world[real_room(virt_room)].contents;obj;obj=obj->next_content)
    if (V_OBJ(obj) == v_num)
      return TRUE;
  return FALSE;
}

int wyverneq(OBJ *obj, CHAR *ch, int cmd, char *argument)
{
  int eq_num;
  char buf[255];
  switch (cmd)
    {
    case MSG_TICK:
      if(obj->equipped_by)
        {
        if(!enchanted_by(obj->equipped_by, "Wyvern lycanthropy"))
          {
          for(eq_num=0;eq_num<18;eq_num++)
            {
            if (obj==EQ(obj->equipped_by,eq_num))
              {
             unequip_char (obj->equipped_by, eq_num);
              extract_obj(obj);
              break;
              }
            }
          }
        }
      else
        {
        if(obj->carried_by)
          {
          obj_from_char(obj);
          extract_obj(obj);
          }
        else
          {
          if(obj->in_room!= -1)
            {
            obj_from_room(obj);
            extract_obj(obj);
            }
          }
        }
      break;
   case CMD_REMOVE:
    if(!ch) return FALSE;
    if (!IS_MORTAL (ch) || obj->equipped_by != ch)
      return FALSE;
    one_argument (argument, buf);
    if (isname (buf, OBJ_NAME(obj)) || !strcmp (buf, "all")) {
      send_to_char ("What are you trying to do, skin yourself?!?\n\r",ch);
      return TRUE;
      }

    default:
      break;
    }
  return FALSE;
}
int wyvernspur(OBJ *obj ,CHAR *ch, int cmd, char *argument) {
  char buf[MAX_INPUT_LENGTH];
  int eq_pos;
  OBJ *tmp_obj;
  ENCH *tmp_enchantment;

  if(!cmd)
      return FALSE;
  if (cmd == MSG_TICK) {
    if(OBJ_SPEC(obj) > 0)
      OBJ_SPEC(obj)--;
    return FALSE;
  }

  if (cmd == CMD_USE) {
    if (!EQ(ch,HOLD) || EQ(ch,HOLD) != obj) return FALSE;
    one_argument (argument, buf);

    if (!isname(buf, OBJ_NAME(obj))) return FALSE;

    if (GET_LEVEL(ch) < 25) {
      send_to_char("You can't figure out how to use it!\n\r",ch);
      return TRUE;   /* Don't want newbies running around as wyverns.... */
    }

    if(OBJ_SPEC(obj)!=0) {
      send_to_char("Nothing happens.\n\r",ch);
      return TRUE;
    }

    OBJ_SPEC(obj) = 48;

    act("$n stretches and grows into the form of a huge wyvern.",FALSE,ch,0,ch,TO_NOTVICT);
    send_to_char("You feel yourself growing larger, and suddenly find you are a wyvern.\n\r",ch);

    for (eq_pos=1 ; eq_pos<17 ; eq_pos++) { /* Stick all the char's eq in */
      tmp_obj=EQ(ch,eq_pos);                /* their inventory, other than */
      if (tmp_obj) {                        /* held and light source */
        if(V_OBJ(tmp_obj)==20133) {         /* Check for Jade stuff */
          extract_obj(unequip_char(ch,eq_pos));
          tmp_obj=read_object(20132,VIRTUAL);
          obj_to_char(tmp_obj,ch);
          continue;
        }
        else if(V_OBJ(tmp_obj)==20135) {
          extract_obj(unequip_char(ch,eq_pos));
          continue;
        }
        else if(V_OBJ(tmp_obj)==20136) {
          extract_obj(unequip_char(ch,eq_pos));
          continue;
        }
        obj_to_char(unequip_char(ch,eq_pos), ch);
      }
    }

    for (eq_pos=1 ; eq_pos<17 ; eq_pos++) {
      tmp_obj=read_object(11507+eq_pos,VIRTUAL);
      equip_char(ch, tmp_obj, eq_pos);
    }

    CREATE(tmp_enchantment, ENCH, 1);
    tmp_enchantment->name     = str_dup("Wyvern lycanthropy");
    tmp_enchantment->duration = 5;            /* Define the enchantment */
    tmp_enchantment->func     = wyvern_enchantment;
    enchantment_to_char(ch, tmp_enchantment, FALSE);

    return TRUE;
  }

  return FALSE;
}

int wyvern_enchantment(ENCH *ench, CHAR *ench_ch, CHAR *ch, int cmd, char*arg)
{
  int eq_num;
  OBJ *tmp, *obj, *next_obj, *tmp2;
  CHAR *act_ch, *act_ch_next;

  if (cmd == MSG_REMOVE_ENCH)
  {
    for (eq_num = 1; eq_num < MAX_WEAR - 1; eq_num++)
    {
      tmp = EQ(ench_ch, eq_num);

      if (tmp && (V_OBJ(tmp) >= 11508 && V_OBJ(tmp) <= 11524))
      {
        tmp = unequip_char(ench_ch, eq_num);
        extract_obj(tmp);
      }
    }

    act("$n twists and shrinks, finally changing back into human form.",
      TRUE, ench_ch, NULL, NULL, TO_ROOM);
    send_to_char("You feel a wrenching sensation, and find yourself back to human form.\n\r", ench_ch);

    return FALSE;
  }

  if(cmd==MSG_MOBACT) {
    if(!AWAKE(ench_ch)) return FALSE;
    switch (number(1,8)) {
      case 1: for (act_ch = world[CHAR_REAL_ROOM(ench_ch)].people;act_ch;act_ch = act_ch_next)
                {
          act_ch_next = act_ch->next_in_room;
          if(act_ch==ench_ch) return FALSE;
          if (number(0,4)==0)
                  {
            act("You suddenly realize that you are sniffing at $N, and thinking of eating him.",1,ench_ch,0,act_ch,TO_CHAR);
            act("$N sniffs at you hungrily.",1,ench_ch,0,act_ch,TO_VICT);
            act("$n sniffs at $N hungrily.",1,ench_ch,0,act_ch,TO_ROOM);
            return FALSE;
            }
                }
           break;
      case 2: act("The wyvern in you takes over for a moment, letting off a monstrous roar.",1,ench_ch,0,NULL,TO_CHAR);
           act("$n lets off a deafening roar.",1,ench_ch,0,NULL,TO_ROOM);
           break;
      case 3: act("You rake your claws across the floor with wyvern-like impatience.",1,ench_ch,0,NULL,TO_CHAR);
           act("$n seems to lose control and gouges the floor with $s massive claws.",1,ench_ch,0,NULL,TO_ROOM);
           break;
      case 4:
              for (tmp=world[CHAR_REAL_ROOM(ench_ch)].contents;tmp;tmp=tmp2 ) {
                tmp2 = tmp->next_content;  /* Added tmp2 - Ranger June 96 */
                if (GET_ITEM_TYPE(tmp)==ITEM_CONTAINER && tmp->obj_flags.value[3]) {
                  for(obj = tmp->contains; obj; obj=next_obj)
                    {
                    next_obj = obj->next_content;
                    obj_from_obj(obj);
                    obj_to_room(obj,CHAR_REAL_ROOM(ench_ch));
                    }
            extract_obj(tmp);
            act("You hungrily devour the corpse, spitting out the bits of metal and other equipment.",1,ench_ch,0,NULL,TO_CHAR);
            act("$n savagely tears apart the corpse and devours it, spitting out equipment as $m chews.",1,ench_ch,0,NULL,TO_ROOM);
                  continue;
            }
                }
           break;
      default: break;
    }
  }

  if(cmd==MSG_SHOW_AFFECT_TEXT) {
    act("......$n is in the form of a huge wyvern!",
     FALSE, ench_ch,0,ch,TO_VICT);
    return(FALSE);
  }

  if(cmd==MSG_TICK && ench->duration==0) {
    /* Give a warning when about ot change back. */
    act("$n starts to shimmer and shift, $s normal shape beginning to return.",
     TRUE, ench_ch,NULL,NULL,TO_ROOM);
    send_to_char("You can feel your natural form beginning to take over.\n\r",
           ench_ch);
  }

  if(cmd==CMD_WEAR && ch==ench_ch) {
    send_to_char("You don't feel like wearing anything right now.\n\r",ench_ch);
    return TRUE;
  }

  return(FALSE);
}

int candles(OBJ *obj ,CHAR *ch, int cmd, char *argument) {
  int tel=FALSE,room_num;
  CHAR *tel_ch, *tel_char_next;
  OBJ *tmp, *tmp_next;
  char buf[80];

  if(!ch) return FALSE; /* Ranger June 96 */
  if (cmd==CMD_DROP || cmd==CMD_GET) {
    one_argument (argument, buf);
    if (!isname(buf,OBJ_NAME(obj)))
      return FALSE;

    switch (cmd) {
    case CMD_GET:
      do_get(ch, argument, cmd);
      break;
    case CMD_DROP:
      do_drop(ch, argument, cmd);
      break;
    }

    if (!find_obj(11500,11502) &&  find_obj(11500,11503) &&
      find_obj(11500,11504) && !find_obj(11500,11505) &&
      find_obj(11500,11506) &&  find_obj(11500,11507) &&
      find_obj(11500,11508) &&  find_obj(11500,11509) &&
      find_obj(11500,11510) &&  find_obj(11500,11511) &&
      find_obj(11500,11512) &&  find_obj(11500,11513) &&
     !find_obj(11500,11514) &&  find_obj(11500,11515) &&
      find_obj(11500,11516) && !find_obj(11500,11517))
      tel=TRUE;

    if (tel == TRUE) {
      act("$p flares and burn themselves up all around you.\n\rSuddenly you are transported!",TRUE,ch,obj,0,TO_ROOM);
      act("$p flares and burn themselves up all around you.\n\rSuddenly you are transported!",TRUE,ch,obj,0,TO_CHAR);

      for (room_num=11502; room_num<11518; room_num++) {
     for (tmp = world[real_room(room_num)].contents; tmp; tmp = tmp_next) {
       tmp_next = tmp->next_content;
       if (V_OBJ(tmp) == 11500)
         extract_obj(tmp);  /* Destroy all the candles lying around. */
     }
      }

      for (tel_ch = world[CHAR_REAL_ROOM(ch)].people;tel_ch;tel_ch = tel_char_next) {
     tel_char_next = tel_ch->next_in_room;
     char_from_room (tel_ch);
     char_to_room(tel_ch, real_room(11518));/* Move all chars in the room */
      }                                        /* to elemental section. */

      if (obj_proto_table[real_object(11505)].number < 20) {
     tmp = read_object(11505,VIRTUAL);
     obj_to_room(tmp, real_room(11598));  /* make sure the prismatic staff is there. */
      }
    }
    return TRUE;
  }
  return FALSE;
}

int prismatic_staff(OBJ *obj ,CHAR *ch, int cmd, char *argument) {
  int fire=FALSE, air=FALSE, water=FALSE, earth=FALSE;
  OBJ *tmp,*next_content;
  CHAR *tel_ch, *tel_ch_next;
  char buf[80];

  if(!ch) return FALSE; /* Ranger - June 96 */
  if(!obj) return FALSE;

  if (cmd == CMD_USE) {

    if (!EQ(ch,HOLD) || EQ(ch,HOLD) != obj)
      return FALSE;

    one_argument (argument, buf);
    if (!isname(buf,OBJ_NAME(obj)))
      return FALSE;
    for (tmp = ch->carrying; tmp; tmp = next_content) {
      next_content = tmp->next_content;
      if (V_OBJ(tmp) == 11501)   /* Check if the char has all */
     fire=TRUE;
      if (V_OBJ(tmp) == 11502)   /* of the elemental crystals. */
     air=TRUE;
      if (V_OBJ(tmp) == 11503)
     water=TRUE;
      if (V_OBJ(tmp) == 11504)
     earth=TRUE;
    }

    if (fire && air && water && earth) {

      act("You hold $p before you and watch it meld with elemental crystals of fire, air, water, and earth.\n\rSuddenly you are transported!",
       1,ch,obj,0,TO_CHAR);
      act("$n holds $p before $m and you see it meld with elemental crystals of fire, air, water, and earth.\n\rSuddenly you are transported!",
       1,ch,obj,0,TO_ROOM);

      for (tmp = ch->carrying; tmp; tmp = next_content) {
     next_content = tmp->next_content;
     if (V_OBJ(tmp) == 11501)   /* Destroy all of the elemental */
       extract_obj(obj_from_char(tmp));
     if (V_OBJ(tmp) == 11502)   /* crystals. */
       extract_obj(obj_from_char(tmp));
     if (V_OBJ(tmp) == 11503)
       extract_obj(obj_from_char(tmp));
     if (V_OBJ(tmp) == 11504)
       extract_obj(obj_from_char(tmp));
      }
      extract_obj(unequip_char(ch,HOLD));    /* And destroy the staff. */

      for (tel_ch=world[CHAR_REAL_ROOM(ch)].people;tel_ch;tel_ch=tel_ch_next)
     {
     tel_ch_next = tel_ch->next_in_room;
     char_from_room (tel_ch);
     char_to_room (tel_ch, real_room(11563));
     } /* Move all chars in the room to the caverns */

      tmp = read_object(11529,VIRTUAL);
      obj_to_room(tmp, real_room(11596)); /* make sure the artifact of binding is there */
      return TRUE;
    }
    else {
      send_to_char ("Nothing seems to happen.\n\r",ch);
      return TRUE;
    }
  }
  return FALSE;
}

int binding_artifact(OBJ *obj ,CHAR *ch, int cmd, char *argument) {
  int upper=FALSE, right=FALSE, left=FALSE, lower=FALSE;
  OBJ *tmp, *tmp_next;
  char buf[80];

  if(!ch) return FALSE; /* Ranger June 96 */
  if(!obj) return FALSE;

  if (cmd == CMD_USE) {
    if (!EQ(ch, HOLD) || EQ(ch,HOLD) != obj)
      return FALSE;
    one_argument (argument, buf);
    if (!isname(buf,OBJ_NAME(obj)))
      return FALSE;

    for (tmp = ch->carrying; tmp; tmp = tmp->next_content) {
      if (V_OBJ(tmp) == 11525) /* Check if the char has all the shards */
     upper=TRUE;
      if (V_OBJ(tmp) == 11526) /* of the artifact of concordant */
     right=TRUE;
      if (V_OBJ(tmp) == 11527) /* opposition. */
     left=TRUE;
      if (V_OBJ(tmp) == 11528)
     lower=TRUE;
    }

    if (upper && right && left && lower) {
      for (tmp = ch->carrying; tmp; tmp = tmp_next) {
     tmp_next = tmp->next_content;
     if (V_OBJ(tmp) == 11525)  /* Destroy all the shards of the */
       extract_obj(obj_from_char(tmp));
     if (V_OBJ(tmp) == 11526)  /* artifact of concordant opposition. */
       extract_obj(obj_from_char(tmp));
     if (V_OBJ(tmp) == 11527)
       extract_obj(obj_from_char(tmp));
     if (V_OBJ(tmp) == 11528)
       extract_obj(obj_from_char(tmp));
      }

      extract_obj(unequip_char(ch,HOLD));    /* And destroy the artifact. */

      tmp = read_object(11530,VIRTUAL);     /* Put the artifact of concordant opposition */
      equip_char(ch, tmp, HOLD); /* in the place of the artifact of binding. */
      act("The shards of alignment bond together with the artifact of binding, to form the Artifact of Concordant Opposition.",1,ch,0,0,TO_ROOM);
      act("The shards of alignment bond together with the artifact of binding, to form the Artifact of Concordant Opposition.",1,ch,0,0,TO_CHAR);
    } else {
      send_to_char("Nothing seems to happen.\n\r",ch);
    }
    return TRUE;
  }

  return FALSE;
}

int artifact_of_CO(OBJ *obj ,CHAR *ch, int cmd, char *argument) {

  if(!ch)
     return FALSE;
  if (cmd == MSG_TICK) {
    if (OBJ_SPEC(obj) < 24)
      OBJ_SPEC(obj)++;

    if (OBJ_SPEC(obj) == 24)     /* over and over again. */
      {
      extract_obj(unequip_char(ch,HOLD));    /* And destroy the artifact. */
      extract_obj(obj);  /* Destroy it after one day. */
      }
  }

  return FALSE;
}

int stone_bridge(int room, CHAR *ch, int cmd, char *arg) {
  OBJ *tmpobj;
  int found = FALSE;

  if (!ch)
    return FALSE;

  if (cmd == CMD_NORTH) {
    for(tmpobj = world[room].contents;tmpobj;tmpobj = tmpobj->next)
      if(tmpobj->item_number_v == 11530)
           found = TRUE;
    if(found == FALSE)

    {
    act("You slip, and fall from the bridge. The waters below carry you until you wash up in the caverns once again.",1,ch,0,0,TO_CHAR);
    act("$n slips, and falls to the water below!",1,ch,0,0,TO_ROOM);
    char_from_room (ch);
    char_to_room (ch, real_room(11563)); /* Move the char back to start. */
    return TRUE;
    }
  }

  return FALSE;
}

int inferno(int room, CHAR *ch, int cmd, char *arg) {
  if (!ch)
    return FALSE;

  if (cmd > 0 && cmd < 7 && number(1,4)==4 && GET_HIT(ch) > 200) {
    act("The movement rubs you against your heated armor, and your skin sears.  You roll on the ground in agony."
     , FALSE, ch, 0, 0, TO_CHAR);
    damage(ch, ch, 200, TYPE_UNDEFINED,DAM_NO_BLOCK);
    return TRUE;
  }
  return FALSE;
}

int hurricane(int room, CHAR *ch, int cmd, char *arg) {
  if (!ch)
    return FALSE;

  if (cmd>0 && cmd < 7 && number(1,4)==4 && GET_HIT(ch) > 200) {
    act("As you try to move, you are smashed against a stone wall.",
     FALSE, ch, 0, 0, TO_CHAR);
    damage(ch, ch, 200, TYPE_UNDEFINED,DAM_NO_BLOCK);
    return TRUE;
  }
  return FALSE;
}

int pool (int room, CHAR *ch, int cmd, char *arg) {
  if (!ch)
    return FALSE;

  if (cmd>0 && cmd <7 && number(1,4)==4 && GET_HIT(ch) > 200) {
    act("You are pulled under water and buffeted with debris as you try to move.", FALSE, ch, 0, 0, TO_CHAR);
    damage(ch, ch, 200, TYPE_UNDEFINED,DAM_NO_BLOCK);
    return TRUE;
  }
  return FALSE;
}

int earthquake(int room, CHAR *ch, int cmd, char *arg) {
  if (!ch)
    return FALSE;

  if (cmd>0 && cmd < 7 && number(1,4)==4 && GET_HIT(ch) > 200) {
    act("Huge boulders fall on you from the ceiling, giving you a nasty headache and blocking the way.", FALSE, ch, 0, 0, TO_CHAR);
    damage(ch, ch, 200, TYPE_UNDEFINED,DAM_NO_BLOCK);
    return TRUE;
  }
  return FALSE;
}

int Holy_Ghosts(CHAR *s, CHAR *ch, int cmd, char *arg) {
  OBJ *tmp_obj;

  if (cmd == MSG_DIE) {
    tmp_obj = read_object(11500,VIRTUAL); /* load up a candle */
    obj_to_char(tmp_obj, s); /* stick it on the ghost */
  }

  return FALSE;
}

int Guardian_NG(CHAR *s, CHAR *ch, int cmd, char *arg) {
  OBJ *tmp_obj;

  if (cmd==MSG_DIE) {   /* get the upper shard of concordant opposition */
    tmp_obj=read_object(11525,VIRTUAL);
    obj_to_char(tmp_obj, s); /* give it to the guardian */
  }
  return FALSE;
}

int Guardian_CN(CHAR *s, CHAR *ch, int cmd, char *arg) {
  OBJ *tmp_obj;

  if (cmd==MSG_DIE) {
    tmp_obj=read_object(11526,VIRTUAL); /* get the right shard of concordant opposition */
    obj_to_char(tmp_obj, s); /* give it to the guardian */
  }
  return FALSE;
}

int Guardian_LN(CHAR *s, CHAR *ch, int cmd, char *arg)
{
  OBJ *tmp_obj;

  if (cmd==MSG_DIE)
    {
      tmp_obj=read_object(11527,VIRTUAL); /* get the left shard of concordant opposition */
      obj_to_char(tmp_obj, s); /* give it to the guardian */
    }
  return FALSE;
}

int Guardian_NE(CHAR *s, CHAR *ch, int cmd, char *arg)
{
  OBJ *tmp_obj;

  if (cmd==MSG_DIE)
    {
      tmp_obj=read_object(11528,VIRTUAL); /* get the lower shard of concordant opposition */
      obj_to_char(tmp_obj, s); /* give it to the guardian */
    }
  return FALSE;
}

int Lei_Kung(CHAR *s, CHAR *ch, int cmd, char *arg) {
  CHAR *victim;
  OBJ *tmp_obj;

  if (cmd==MSG_MOBACT) {
    victim = s->specials.fighting;
    if(victim ? (CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(s)) : FALSE) {
      if (number(0,3)==0) {
     act("You scream in agony as $n touches you with $s electrifying armor.",1,s,0,victim,TO_VICT);
     act("$N screams in agony as sparks fly to $S from $n's armor.",1,s,0,victim,TO_NOTVICT);
     damage(s,victim,300,TYPE_UNDEFINED,DAM_NO_BLOCK);
     return FALSE;
      }
    }
  }
  if (cmd==MSG_DIE)
    {
      tmp_obj=read_object(11501,VIRTUAL); /* get the elemental crystal of fire */
      obj_to_char(tmp_obj, s); /* give it to him */
    }
  return FALSE;
}

int Fei_Lien_and_Feng_Po(CHAR *s, CHAR *ch, int cmd, char *arg)
{
  CHAR *victim;
  OBJ *tmp_obj;

  if(cmd == MSG_MOBACT)
    {
      victim = s->specials.fighting;
      if(victim && (CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(s))) {
     if (number(0,3)==0)
       {
         act("$n opens a small sack that buffets you with hurricane force winds.",1,s,0,victim,TO_VICT);
         act("$N is buffeted by whilwinds like those of a djinn as $n opens a small sack.",1,s,0,victim,TO_NOTVICT);
         damage(s,victim,300,TYPE_UNDEFINED,DAM_NO_BLOCK);
         return FALSE;
       }
      }
    }
  if (cmd==MSG_DIE)
    {
      tmp_obj=read_object(11502,VIRTUAL); /* get the elemental crystal of air */
      obj_to_char(tmp_obj, s); /* give it to him */
    }
  return FALSE;
}

int Chih_Sung_Tzu(CHAR *s, CHAR *ch, int cmd, char *arg)
{
  CHAR *victim;
  OBJ *tmp_obj;

  if(cmd==MSG_MOBACT)
    {
      victim = s->specials.fighting;
      if(victim && (CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(s))) {
     if (number(0,3)==0) {
       act("$n mutters some strange words, and you are attacked by a storm of hail.",1,s,0,victim,TO_VICT);
       act("$n mutters something, and $N is pelted by a storm of hail.",1,s,0,victim,TO_NOTVICT);
       damage(s,victim,300,TYPE_UNDEFINED,DAM_NO_BLOCK);
       return FALSE;
     }
      }
    }
  if (cmd==MSG_DIE)
    {
      tmp_obj=read_object(11503,VIRTUAL); /* get the elemental crystal of water */
      obj_to_char(tmp_obj, s); /* give it to him */
    }
  return FALSE;
}

int Shang_Ti(CHAR *s, CHAR *ch, int cmd, char *arg)
{
  CHAR *victim;
  OBJ *tmp_obj;


  if(cmd==MSG_MOBACT)
    {
      victim = s->specials.fighting;
      if(victim && (CHAR_REAL_ROOM(victim) == CHAR_REAL_ROOM(s))) {
     if (number(0,3)==0) {
       act("A shaft of blinding light escapes from $n's mouth, burning you with holy fire.",1,s,0,victim,TO_VICT);
       act("$N is burned with holy fire as a shaft of light escapes $n's mouth.",1,s,0,victim,TO_NOTVICT);
       damage(s,victim,300,TYPE_UNDEFINED,DAM_NO_BLOCK);
       return FALSE;
     }
      }
    }
  if (cmd==MSG_DIE)
    {
      tmp_obj=read_object(11504,VIRTUAL); /* get the elemental crystal of earth */
      obj_to_char(tmp_obj, s); /* give it to him */
    }
  return FALSE;
}

/* Thank Kafka for this bit....*/

int wyvern_eq (OBJ *eq, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];

  if (!ch)
    return FALSE;


  if (cmd == CMD_REMOVE) {
    if (!IS_MORTAL (ch) || eq->equipped_by != ch)
      return FALSE;
    one_argument (arg, buf);
    if (isname (buf, OBJ_NAME(eq)) || !strcmp (buf, "all")) {
      send_to_char ("What are you trying to do, skin yourself?!?\n\r",ch);
      return TRUE;
    }
  }
  return FALSE;
}

#define WYV_ZONE      11500
#define WYV_GHOST     ITEM(WYV_ZONE,2)
#define WYV_LEI       ITEM(WYV_ZONE,3)
#define WYV_FEI       ITEM(WYV_ZONE,4)
#define WYV_FENG      ITEM(WYV_ZONE,5)
#define WYV_CHIH      ITEM(WYV_ZONE,6)
#define WYV_SHANG     ITEM(WYV_ZONE,7)
#define WYV_GUARDNG   ITEM(WYV_ZONE,10)
#define WYV_GUARDCN   ITEM(WYV_ZONE,11)
#define WYV_GUARDLN   ITEM(WYV_ZONE,12)
#define WYV_GUARDNE   ITEM(WYV_ZONE,13)

#define WYV_STAFF     ITEM(WYV_ZONE,5)
#define WYV_SPUR      ITEM(WYV_ZONE,6)
#define WYV_BINDER    ITEM(WYV_ZONE,29)
#define WYV_ARTIFACT  ITEM(WYV_ZONE,30)
#define WYV_CANDLE    ITEM(WYV_ZONE,0)

#define WYV_BRIDGE    ITEM(WYV_ZONE,97)
#define WYV_INFERNO1  ITEM(WYV_ZONE,26)
#define WYV_INFERNO2  ITEM(WYV_ZONE,27)
#define WYV_INFERNO3  ITEM(WYV_ZONE,28)
#define WYV_HURRI1    ITEM(WYV_ZONE,37)
#define WYV_HURRI2    ITEM(WYV_ZONE,38)
#define WYV_HURRI3    ITEM(WYV_ZONE,39)
#define WYV_POOL1     ITEM(WYV_ZONE,48)
#define WYV_POOL2     ITEM(WYV_ZONE,49)
#define WYV_POOL3     ITEM(WYV_ZONE,50)
#define WYV_QUAKE1    ITEM(WYV_ZONE,59)
#define WYV_QUAKE2    ITEM(WYV_ZONE,60)
#define WYV_QUAKE3    ITEM(WYV_ZONE,61)

void assign_wyvern (void)
{
  int i = 0;

  assign_mob(WYV_GHOST   , Holy_Ghosts);
  assign_mob(WYV_LEI     , Lei_Kung);
  assign_mob(WYV_FEI     , Fei_Lien_and_Feng_Po);
  assign_mob(WYV_FENG    , Fei_Lien_and_Feng_Po);
  assign_mob(WYV_CHIH    , Chih_Sung_Tzu);
  assign_mob(WYV_SHANG   ,Shang_Ti);
  assign_mob(WYV_GUARDNG , Guardian_NG);
  assign_mob(WYV_GUARDCN , Guardian_CN);
  assign_mob(WYV_GUARDLN , Guardian_LN);
  assign_mob(WYV_GUARDNE , Guardian_NE);

  assign_obj(WYV_STAFF   ,  prismatic_staff);
  assign_obj(WYV_SPUR    ,  wyvernspur);
  assign_obj(WYV_BINDER ,binding_artifact);
  assign_obj(WYV_ARTIFACT, artifact_of_CO);
  assign_obj(WYV_CANDLE, candles);

  for (i = 11508; i < 11524; i++)
  {
    assign_obj(i, wyverneq);
  }

  assign_room(WYV_BRIDGE, stone_bridge);
  assign_room(WYV_INFERNO1, inferno);
  assign_room(WYV_INFERNO2, inferno);
  assign_room(WYV_INFERNO3, inferno);
  assign_room(WYV_HURRI1, hurricane);
  assign_room(WYV_HURRI2, hurricane);
  assign_room(WYV_HURRI3, hurricane);
  assign_room(WYV_POOL1, pool);
  assign_room(WYV_POOL2, pool);
  assign_room(WYV_POOL3, pool);
  assign_room(WYV_QUAKE1, earthquake);
  assign_room(WYV_QUAKE2, earthquake);
  assign_room(WYV_QUAKE3, earthquake);
}

