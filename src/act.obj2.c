/* ************************************************************************
*  file: act.obj2.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Commands mainly using objects.                                 *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "utility.h"
#include "act.h"
#include "limits.h"
#include "cmd.h"

/* extern functions */
extern int signal_object(OBJ *obj, CHAR *ch, int cmd, char *arg);
struct obj_data *get_object_in_equip_vis(struct char_data *ch,char *arg,struct obj_data **equipment, int *j);
char *str_dup(char *source);
char *sstrdel(char *s, ...);
void raw_kill(struct char_data *ch);

void name_from_drinkcon(struct obj_data *obj,int type) {
  char name[50],new_name[MAX_INPUT_LENGTH];

  if(type < LIQ_WATER || type >LIQ_COKE) return;
  sprintf(name,"%s ",drinknames[type]);
  sprintf(new_name,"%s",OBJ_NAME(obj));
  if(!strstr(new_name,name)) {
    sprintf(name," %s",drinknames[type]);
    if(!strstr(new_name,name)) return;
  }
  sstrdel(new_name,name,NULL);

  if(obj->item_number < 0 ||
     obj->name != obj_proto_table[obj->item_number].name)
    free(obj->name);
  obj->name=str_dup(new_name);
}

void name_to_drinkcon(struct obj_data *obj,int type) {
  char name[50],*new_name;

  if(type < LIQ_WATER || type >LIQ_COKE) return;
  sprintf(name,"%s",drinknames[type]);
  if(strstr(OBJ_NAME(obj),name)) return;

  CREATE(new_name,char,strlen(OBJ_NAME(obj))+strlen(name)+2);
  sprintf(new_name,"%s %s",name,OBJ_NAME(obj));

  if(obj->item_number < 0 || obj->name != obj_proto_table[obj->item_number].name)
    free(obj->name);
  obj->name = new_name;
}

void do_drink(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct obj_data *temp, *temp2;
  struct affected_type_5 af;
  int amount,was_drunk=0;


  one_argument(argument,buf);

  if (!(temp2 = get_obj_in_list_vis(ch, buf, world[CHAR_REAL_ROOM(ch)].contents)))
    {

      if(!(temp = get_obj_in_list_vis(ch,buf,ch->carrying)))
     {
       act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
       return;
     }
    }

  else
    temp = temp2;

  if (temp->obj_flags.type_flag!=ITEM_DRINKCON)
    {
      act("You can't drink from that!",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if((GET_COND(ch,DRUNK)>10)&&(GET_COND(ch,THIRST)>0)) /* The pig is drunk */
    {
      act("Yoo shimply faiw to weatch yowr moouth!", FALSE, ch, 0, 0, TO_CHAR);
      act("$n tried to drink but missed $s mouth!", TRUE, ch, 0, 0, TO_ROOM);
      return;
    }

  if(GET_COND(ch,THIRST)>23) /* Stomach full */
    {
      act("Your stomach can't contain anymore!",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if (temp->obj_flags.type_flag==ITEM_DRINKCON){
    if(temp->obj_flags.value[1]>0)  /* Not empty */
      {
     sprintf(buf,"$n drinks %s from $p.",drinks[temp->obj_flags.value[2]]);
     act(buf, TRUE, ch, temp, 0, TO_ROOM);
     sprintf(buf,"You drink the %s.\n\r",drinks[temp->obj_flags.value[2]]);
     send_to_char(buf,ch);

     if (drink_aff[temp->obj_flags.value[2]][DRUNK] > 0 )
       amount = (25-GET_COND(ch,THIRST))/drink_aff[temp->obj_flags.value[2]][DRUNK];
     else
       amount = number(3,10);

     amount = MIN(amount,temp->obj_flags.value[1]);

     /*               weight_change_object(temp, -amount);*/
     /* Subtract amount */

     if(GET_COND(ch,DRUNK)>10) was_drunk=1;

     gain_condition(ch,DRUNK,(int)((int)drink_aff
                          [temp->obj_flags.value[2]][DRUNK]*amount)/2); /* was /4 */

     gain_condition(ch,FULL,(int)((int)drink_aff
                         [temp->obj_flags.value[2]][FULL]*amount)/4);

     gain_condition(ch,THIRST,(int)((int)drink_aff
                           [temp->obj_flags.value[2]][THIRST]*amount)/4);

     if(GET_COND(ch,DRUNK)>10) {
       if(was_drunk)
         act("You still feel drunk.",FALSE,ch,0,0,TO_CHAR);
       else
         act("You feel drunk.",FALSE,ch,0,0,TO_CHAR);
     }

     if(GET_COND(ch,THIRST)>20)
       act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);

     if(GET_COND(ch,FULL)>20)
       act("You are full.",FALSE,ch,0,0,TO_CHAR);

     if(temp->obj_flags.value[3]) /* The shit was poisoned ! */
       {
         act("Ooups, it tasted rather strange ?!!?",FALSE,ch,0,0,TO_CHAR);
         act("$n chokes and utters some strange sounds.",
          TRUE,ch,0,0,TO_ROOM);
         af.type = SPELL_POISON;
         af.duration = amount*3;
         af.modifier = 0;
         af.location = APPLY_NONE;
         af.bitvector = AFF_POISON;
         af.bitvector2 = 0;
         affect_join(ch,&af, FALSE, FALSE);
       }

     /* empty the container, and no longer poison. */
     temp->obj_flags.value[1]-= amount;
     if(!temp->obj_flags.value[1]) {  /* The last bit */
       name_from_drinkcon(temp,temp->obj_flags.value[2]);
       temp->obj_flags.value[2]=0;
       temp->obj_flags.value[3]=0;
     }
     return;

    }
  }

  act("It's empty already.",FALSE,ch,0,0,TO_CHAR);
  return;
}

void do_fill(struct char_data *ch, char *argument, int cmd) {
  char buf[100], buf2[100];
  struct obj_data *temp;
  struct obj_data *obj_object;
  struct obj_data *next_obj;

  one_argument(argument,buf);

  if (!(temp=get_obj_in_list_vis(ch,buf,ch->carrying))) {
    act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  for(obj_object = world[CHAR_REAL_ROOM(ch)].contents; obj_object; obj_object = next_obj) {
    next_obj = obj_object->next_content;
    if (CAN_SEE_OBJ(ch,obj_object))
      if (obj_object->obj_flags.type_flag==ITEM_DRINKCON) {
        if (temp->obj_flags.type_flag==ITEM_DRINKCON)
          if (temp->obj_flags.value[1] < temp->obj_flags.value[0]) {
            name_to_drinkcon(temp,obj_object->obj_flags.value[2]);
            temp->obj_flags.value[2]=obj_object->obj_flags.value[2];
            temp->obj_flags.value[1]=temp->obj_flags.value[0];
            sprintf(buf2,"$n fills $s $q with %s from %s.",
               drinks[temp->obj_flags.value[2]],OBJ_SHORT(obj_object));
            act(buf2, TRUE, ch, temp, 0, TO_ROOM);
            sprintf(buf,"You fill your $q with %s from %s.",
               drinks[temp->obj_flags.value[2]],OBJ_SHORT(obj_object));
            act(buf,0,ch,temp,0,TO_CHAR);
            return;
          } else {
            act("There is no more room in your $q.",FALSE,ch,temp,0,TO_CHAR);
            return;
          }
        else {
          act("You can't fill that.",FALSE,ch,0,0,TO_CHAR);
          return;
        }
      }
  }

  if (IS_OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING)) {
    temp->obj_flags.value[1] = temp->obj_flags.value[0];
    temp->obj_flags.value[2] = 0;
    temp->obj_flags.value[3] = 0;
    name_to_drinkcon(temp, LIQ_WATER);
    act("$n fills $s $q with rain.", TRUE, ch, temp, 0, TO_ROOM);
    act("You fill your $q with rain.",0,ch,temp,0,TO_CHAR);
    return;
  }

  act("From where?",FALSE,ch,0,0,TO_CHAR);
}

void death_cry(CHAR *ch);
void strip_char(CHAR *ch);
void death_list(CHAR *ch);
int check_god_access(CHAR *ch, int active);
void do_tap(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct obj_data *temp;
  CHAR *vict;

  if(GET_LEVEL(ch)<3) {
    send_to_char("You must be at least level 3.\n\r",ch);
    return;
  }

  one_argument(argument,buf);

  if(GET_LEVEL(ch)>=LEVEL_ETE && check_god_access(ch,TRUE)) {
    vict=get_char_room(buf,CHAR_REAL_ROOM(ch));
    if(vict) {
      if (GET_LEVEL(ch) == LEVEL_ETE && !IS_NPC(vict))
      {
        send_to_char("You can't tap players.\n\r", ch);
        return;
      }
      if(GET_LEVEL(vict)>GET_LEVEL(ch) && !IS_NPC(vict)) {
        send_to_char("You can't tap someone of a higher level.\n\r",ch);
        return;
      } else if (!strcmp(buf,"lem")) {
        send_to_char("You can't tap Lem!  HAHAHAHA!!!\n\r",ch);
        return;
      }
      act("$n taps the ground three times.\n\rA hungry ghost appears suddenly and eats you!",0,ch,0,vict,TO_VICT);
      act("$n taps the ground three times.\n\rA hungry ghost appears suddenly and eats $N!",0,ch,0,vict,TO_NOTVICT);
      act("You tap the ground three times.\n\rA hungry ghost appears suddenly and eats $N!",0,ch,0,vict,TO_CHAR);
      sprintf(buf,"WIZLOG: %s taps %s in room %d.\n\r",GET_NAME(ch),GET_NAME(vict),CHAR_VIRTUAL_ROOM(ch));
      log_s(buf);
      wizlog(buf,LEVEL_IMP,5);
      death_cry(vict);
      vict->new.been_killed += 1;
      death_list(vict);
      strip_char(vict);

      if(vict->quest_status==QUEST_RUNNING || vict->quest_status==QUEST_COMPLETED)
        vict->ver3.time_to_quest=30;
      vict->questgiver=0;
      if(vict->questobj) vict->questobj->owned_by=0;
      vict->questobj=0;
      if(vict->questmob) vict->questmob->questowner=0;
      vict->questmob=0;
      vict->quest_status=QUEST_NONE;
      vict->quest_level=0;

      save_char(vict, NOWHERE);
      extract_char(vict);
      return;
    }
  }

  if (!*buf || !(temp=get_obj_in_list_vis(ch,buf,world[CHAR_REAL_ROOM(ch)].contents)))
    {
      act("You tap the ground three times.\n\rA hungry ghost appears suddenly, but finds nothing to eat!",FALSE,ch,0,0,TO_CHAR);
      act("$n taps the ground three times.\n\rA hungry ghost appears suddenly, but finds nothing to eat!",FALSE,ch,0,0,TO_ROOM);
      return;
    }
  else
    {
      if (!strcmp(buf,"fountain") || !strcmp(buf, "corpse") || !strcmp(buf, "statue"))
     {
       act("You can't tap that!\n\r",FALSE,ch,0,0,TO_CHAR);
       return;
     }

      if (!CAN_WEAR(temp,ITEM_TAKE))
     {
       act("You can't tap that!", FALSE,ch,0,0,TO_CHAR);
       return;
     }

      act("You tap the ground three times.\n\rA hungry ghost appears suddenly and eats $p.",FALSE,ch,temp,0,TO_CHAR);
      act("$n taps the ground three times.\n\rA hungry ghost appears suddenly and eats $p.",FALSE,ch,temp,0,TO_ROOM);
      if (GET_LEVEL(ch) > LEVEL_IMM) {
     sprintf (buf, "WIZINFO: %s tapped %s in #%d", GET_NAME(ch),
           OBJ_SHORT(temp), CHAR_VIRTUAL_ROOM(ch));
     wizlog(buf, GET_LEVEL(ch)+1, 5);
     log_s(buf);
      }

      extract_obj(temp);
    }
}

void do_donate(CHAR *ch, char *arg, int cmd)
{
  char buf[MSL] = "";
  char name[MIL] = "";
  OBJ *obj = NULL;
  int room = 0;

  if (CHAR_VIRTUAL_ROOM(ch) == 10)
  {
    send_to_char("You can't donate from prison.\n\r", ch);
    return;
  }

  one_argument(arg, name);

  if (!(obj = get_obj_in_list_vis(ch, name, ch->carrying)))
  {
    send_to_char("You don't have that item!\n\r", ch);
    return;
  }

  if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_NODROP))
  {
    send_to_char("You can't let go of it, it must be CURSED!\n\r", ch);
    return;
  }

  if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_CHAOTIC) &&
      IS_SET(CHAR_ROOM_FLAGS(ch), CHAOTIC))
  {
    send_to_char("You can't donate chaotic items from here.\n\r", ch);
    return;
  }

  if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_CLONE))
  {
    act("Your cloned $p falls to pieces as you try to donate it.", FALSE, ch, obj, 0, TO_CHAR);
    act("$n's cloned $p falls to pieces as $e tries to donate it.", FALSE, ch, obj, 0, TO_ROOM);

    obj_from_char(obj);
    extract_obj(obj);

    return;
  }

  if (GET_ITEM_TYPE(obj) == ITEM_FOOD  ||
      GET_ITEM_TYPE(obj) == ITEM_TRASH)
  {
    send_to_char("You can't donate that!\n\r", ch);
    return;
  }

  if (signal_object(obj, ch, MSG_OBJ_DONATED, "")) return;

  if (IS_CORPSE(obj))
  {
    room = real_room(3088); /* The Midgaard Morgue */

    act("You send $p to the morgue.", TRUE, ch, obj, NULL, TO_CHAR);
    act("$n sends $p to the morgue.", TRUE, ch, obj, NULL, TO_ROOM);

    log_f("WIZINFO: %s sent %s to the morgue", GET_NAME(ch), OBJ_SHORT(obj));
  }
  else
  {
    room = real_room(3084); /* Donation Room */

    act("You donate $p.", TRUE, ch, obj, NULL, TO_CHAR);
    act("$n sends $p to the donation room.", TRUE, ch, obj, NULL, TO_ROOM);
  }

  sprintf(buf,"[Pop!] %s comes spiraling from the ether and drops on the floor!\n\r", OBJ_SHORT(obj));
  send_to_room(buf, room);

  if (IS_IMMORTAL(ch))
  {
    sprintf (buf, "WIZINFO: %s donated %s in room #%d",
      GET_NAME(ch), OBJ_SHORT(obj), CHAR_VIRTUAL_ROOM(ch));
    wizlog(buf, GET_LEVEL(ch) + 1, 5);
    log_f("%s", buf);
  }

  OBJ_COST(obj) = 0;

  if (!IS_SET(OBJ_WEAR_FLAGS(obj), ITEM_QUESTWEAR)) SET_BIT(OBJ_EXTRA_FLAGS(obj), ITEM_ANTI_AUCTION);

  obj_from_char(obj);
  obj_to_room(obj, room);

  save_char(ch, NOWHERE);
}

void do_jun(struct char_data *ch, char *argument, int cmd)
{
  send_to_char("You have to write junk - no less, to junk!\n\r", ch);
}

void do_junk(CHAR *ch, char *arg, int cmd)
{
  char buf[MSL] = "";
  char name[MIL] = "";
  OBJ *obj = NULL;

  if (GET_LEVEL(ch) < 3)
  {
    send_to_char("You must be at least level 3 to junk something.\n\r", ch);
    return;
  }

  one_argument(arg, name);

  if (!(obj = get_obj_in_list_vis(ch, name, ch->carrying)))
  {
    send_to_char("You don't have anything like that.\n\r", ch);
    return;
  }

  if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_NODROP))
  {
    send_to_char("You can't let go of it, it must be CURSED!\n\r", ch);
    return;
  }

  if (IS_CORPSE(obj))
  {
    send_to_char("You can't bring yourself to tear apart the corpse.\n\r", ch);
    return;
  }

  if (signal_object(obj, ch, MSG_OBJ_JUNKED, "")) return;

  if (IS_IMMORTAL(ch))
  {
    sprintf (buf, "WIZINFO: %s junked %s in #%d",
      GET_NAME(ch), OBJ_SHORT(obj), CHAR_VIRTUAL_ROOM(ch));
    wizlog(buf, GET_LEVEL(ch) + 1, 5);
    log_f("%s", buf);
  }

  act("You junk $p.", FALSE, ch, obj, NULL, TO_CHAR);
  act("$n junks $p.", TRUE, ch, obj, NULL, TO_ROOM);

  obj_from_char(obj);
  extract_obj(obj);

  save_char(ch, NOWHERE);
}

void do_eat(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  struct obj_data *temp;
  struct affected_type_5 af;

  one_argument(argument,buf);

  if(!(temp = get_obj_in_list_vis(ch,buf,ch->carrying)))
    {
      act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if((temp->obj_flags.type_flag != ITEM_FOOD) && (GET_LEVEL(ch) < LEVEL_IMM))
    {
      act("Your stomach refuses to eat that!?!",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if(GET_COND(ch,FULL)>20) /* Stomach full */
    {
      act("You are too full to eat more!",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  act("$n eats $p.",TRUE,ch,temp,0,TO_ROOM);
  act("You eat the $o.",FALSE,ch,temp,0,TO_CHAR);

  gain_condition(ch,FULL,temp->obj_flags.value[0]);

  if(GET_COND(ch,FULL)>20)
    act("You are full.",FALSE,ch,0,0,TO_CHAR);

  if(temp->obj_flags.value[3] && (GET_LEVEL(ch) < LEVEL_IMM))
    /* The shit was poisoned ! */
    {
      act("Ooups, it tasted rather strange ?!!?",FALSE,ch,0,0,TO_CHAR);
      act("$n coughs and utters some strange sounds.",FALSE,ch,0,0,TO_ROOM);

      af.type = SPELL_POISON;
      af.duration = temp->obj_flags.value[0]*2;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_POISON;
      af.bitvector2 = 0;
      affect_join(ch,&af, FALSE, FALSE);
    }

  if (GET_LEVEL(ch) > LEVEL_IMM) {
    sprintf (buf, "WIZINFO: %s ate %s in #%d", GET_NAME(ch),
          OBJ_SHORT(temp), CHAR_VIRTUAL_ROOM(ch));
    wizlog(buf, GET_LEVEL(ch)+1, 5);
    log_s(buf);
  }

  extract_obj(temp);
}


void do_pour(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  struct obj_data *from_obj;
  struct obj_data *to_obj;
  int amount;

  argument_interpreter(argument, arg1, arg2);

  if(!*arg1) /* No arguments */
    {
      act("What do you want to pour from?",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if(!(from_obj = get_obj_in_list_vis(ch,arg1,ch->carrying)))
    {
      act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if(from_obj->obj_flags.type_flag!=ITEM_DRINKCON)
    {
      act("You can't pour from that!",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if(from_obj->obj_flags.value[1]==0)
    {
      act("$p is empty.",FALSE, ch,from_obj, 0,TO_CHAR);
      return;
    }

  if(!*arg2)
    {
      act("Where do you want it? Out or in what?",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if(!strcmp(arg1, arg2))
    {
      act("You can't pour $p back to itself.", FALSE, ch, from_obj, 0,TO_CHAR);
      return;
    }

  if(!strcmp(arg2,"out"))
    {
      act("$n empties $p", TRUE, ch,from_obj,0,TO_ROOM);
      act("You empty $p.", FALSE, ch,from_obj,0,TO_CHAR);

      name_from_drinkcon(from_obj,from_obj->obj_flags.value[2]);
      from_obj->obj_flags.value[1]=0;
      from_obj->obj_flags.value[2]=0;
      from_obj->obj_flags.value[3]=0;

      return;

    }

  if(!(to_obj = get_obj_in_list_vis(ch,arg2,ch->carrying)))
    {
      act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if(to_obj->obj_flags.type_flag!=ITEM_DRINKCON)
    {
      act("You can't pour anything into that.",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if((to_obj->obj_flags.value[1]!=0)&&
     (to_obj->obj_flags.value[2]!=from_obj->obj_flags.value[2]))
    {
      act("There is already another liquid in it!",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if(!(to_obj->obj_flags.value[1]<to_obj->obj_flags.value[0]))
    {
      act("There is no room for more.",FALSE,ch,0,0,TO_CHAR);
      return;
    }

   if (from_obj == to_obj)
     {
      act("You can't pour a container into itself!",FALSE,ch,0,0,TO_CHAR);
      return;
     }

  sprintf(buf,"You pour the %s into the %s.",
       drinks[from_obj->obj_flags.value[2]],arg2);
  send_to_char(buf,ch);

  /* New alias */
  if (to_obj->obj_flags.value[1]==0)
    name_to_drinkcon(to_obj,from_obj->obj_flags.value[2]);

  /* First same type liq. */
  to_obj->obj_flags.value[2]=from_obj->obj_flags.value[2];

  /* Then how much to pour */
  from_obj->obj_flags.value[1]-= (amount=
                      (to_obj->obj_flags.value[0]-to_obj->obj_flags.value[1]));

  to_obj->obj_flags.value[1]=to_obj->obj_flags.value[0];

  if(from_obj->obj_flags.value[1]<0)    /* There was to little */
    {
      to_obj->obj_flags.value[1]+=from_obj->obj_flags.value[1];
      amount += from_obj->obj_flags.value[1];
      name_from_drinkcon(from_obj,from_obj->obj_flags.value[2]);
      from_obj->obj_flags.value[1]=0;
      from_obj->obj_flags.value[2]=0;
      from_obj->obj_flags.value[3]=0;
    }

  /* Then the poison boogie */
  to_obj->obj_flags.value[3]=
    (to_obj->obj_flags.value[3]||from_obj->obj_flags.value[3]);

  return;
}

void do_sip(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type_5 af;
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  struct obj_data *temp;
  int was_drunk=0;

  one_argument(argument,arg);

  if(!(temp = get_obj_in_list_vis(ch,arg,ch->carrying)))
    {
      act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if(temp->obj_flags.type_flag!=ITEM_DRINKCON)
    {
      act("You can't sip from that!",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if(GET_COND(ch,DRUNK)>10) /* The pig is drunk ! */
    {
      act("You shimply faiw to weatch yow mouff! (HIC!)",FALSE,ch,0,0,TO_CHAR);
      act("$n tries to sip, but fails!",TRUE,ch,0,0,TO_ROOM);
      return;
    }

  if(!temp->obj_flags.value[1])  /* Empty */
    {
      act("But there is nothing in it?",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  act("$n sips from the $o",TRUE,ch,temp,0,TO_ROOM);
  sprintf(buf,"It tastes like %s.\n\r",drinks[temp->obj_flags.value[2]]);
  send_to_char(buf,ch);

  if(GET_COND(ch,DRUNK)>10) was_drunk=1;

  gain_condition(ch,DRUNK,(int)(drink_aff[temp->obj_flags.value[2]][DRUNK]/4));

  gain_condition(ch,FULL,(int)(drink_aff[temp->obj_flags.value[2]][FULL]/4));

  gain_condition(ch,THIRST,(int)(drink_aff[temp->obj_flags.value[2]][THIRST]/4));

  /*weight_change_object(temp, -1);*/  /* Subtract one unit */

   if(GET_COND(ch,DRUNK)>10) {
     if(was_drunk)
       act("You still feel drunk.",FALSE,ch,0,0,TO_CHAR);
     else
       act("You feel drunk.",FALSE,ch,0,0,TO_CHAR);
   }

  if(GET_COND(ch,THIRST)>20)
    act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);

  if(GET_COND(ch,FULL)>20)
    act("You are full.",FALSE,ch,0,0,TO_CHAR);

  if(temp->obj_flags.value[3]&&!IS_AFFECTED(ch,AFF_POISON)) /* The shit was poisoned ! */
    {
      act("But it also had a strange taste!",FALSE,ch,0,0,TO_CHAR);

      af.type = SPELL_POISON;
      af.duration = 3;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_POISON;
      af.bitvector2 = 0;
      affect_to_char(ch,&af);
    }

  temp->obj_flags.value[1]--;

  if(!temp->obj_flags.value[1])  /* The last bit */
    {
      name_from_drinkcon(temp,temp->obj_flags.value[2]);
      temp->obj_flags.value[2]=0;
      temp->obj_flags.value[3]=0;
    }

  return;

}


void do_taste(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type_5 af;
  char arg[MAX_STRING_LENGTH];
  struct obj_data *temp;

  one_argument(argument,arg);

  if(!(temp = get_obj_in_list_vis(ch,arg,ch->carrying)))
    {
      act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if(temp->obj_flags.type_flag==ITEM_DRINKCON)
    {
      do_sip(ch,argument,0);
      return;
    }

  if(!(temp->obj_flags.type_flag==ITEM_FOOD))
    {
      act("Taste that?!? Your stomach refuses!",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  act("$n tastes the $o", FALSE, ch, temp, 0, TO_ROOM);
  act("You taste the $o", FALSE, ch, temp, 0, TO_CHAR);

  gain_condition(ch,FULL,1);

  if(GET_COND(ch,FULL)>20)
    act("You are full.",FALSE,ch,0,0,TO_CHAR);

  if(temp->obj_flags.value[3]&&!IS_AFFECTED(ch,AFF_POISON)) /* The shit was poisoned ! */
    {
      act("Oops, it did not taste good at all!",FALSE,ch,0,0,TO_CHAR);

      af.type = SPELL_POISON;
      af.duration = 2;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_POISON;
      af.bitvector2 = 0;
      affect_to_char(ch,&af);
    }

  temp->obj_flags.value[0]--;

  if(!temp->obj_flags.value[0])     /* Nothing left */
    {
      act("There is nothing left now.",FALSE,ch,0,0,TO_CHAR);
      extract_obj(temp);
    }

  return;

}


void show_bitvector_wear(struct char_data *ch, struct obj_data *obj_object) {
  if (!IS_SET(ch->specials.affected_by, AFF_IMINV) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_IMINV)) {
    act("$n slowly fades out of existence.", TRUE, ch, 0, 0, TO_ROOM);
    send_to_char("You vanish.\n\r", ch);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_POISON) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_POISON)) {
    send_to_char("You feel very sick.\n\r", ch);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_FURY) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_FURY)) {
    send_to_char("You feel very angry.\n\r", ch);
    act("$n starts snarling and fuming with fury.", FALSE, ch, 0, 0, TO_ROOM);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_BLIND) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_BLIND)) {
    send_to_char("You can't see anything!!\n\r", ch);
    act("$n is blinded!!", FALSE, ch, 0, 0, TO_ROOM);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_PARALYSIS) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_PARALYSIS)) {
    act("Your limbs freeze in place.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n's limbs freeze in place!", TRUE, ch, 0, 0, TO_ROOM);
  }

  if ((!IS_SET(ch->specials.affected_by, AFF_DETECT_INVISIBLE) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_DETECT_INVISIBLE)) ||
    (!IS_SET(ch->specials.affected_by, AFF_DETECT_MAGIC) &&
      IS_SET(obj_object->obj_flags.bitvector, AFF_DETECT_MAGIC)) ||
      (!IS_SET(ch->specials.affected_by, AFF_DETECT_ALIGNMENT) &&
        IS_SET(obj_object->obj_flags.bitvector, AFF_DETECT_ALIGNMENT))) {
    send_to_char("Your eyes tingle.\n\r", ch);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_FLY) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_FLY)) {
    send_to_char("You start to fly.\n\r", ch);
    act("$n starts to fly.", TRUE, ch, 0, 0, TO_ROOM);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_INFRAVISION) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_INFRAVISION)) {
    act("$n's eyes start glowing a bright red.", TRUE, ch, 0, 0, TO_ROOM);
    send_to_char("Your eyes glow a bright red.\n\r", ch);
  }


  if (!IS_SET(ch->specials.affected_by, AFF_SANCTUARY) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_SANCTUARY)) {
    act("$n is surrounded by a white aura.", TRUE, ch, 0, 0, TO_ROOM);
    act("You start glowing.", TRUE, ch, 0, 0, TO_CHAR);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_INVUL) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_INVUL)) {
    act("$n is surrounded by a powerful sphere.", TRUE, ch, 0, 0, TO_ROOM);
    act("You are surrounded by a powerful sphere.", TRUE, ch, 0, 0, TO_CHAR);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_SPHERE) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_SPHERE)) {
    act("$n is surrounded by a golden sphere.", TRUE, ch, 0, 0, TO_ROOM);
    act("You are surrounded by a golden sphere.", TRUE, ch, 0, 0, TO_CHAR);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_PROTECT_EVIL) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_PROTECT_EVIL)) {
    send_to_char("You have a rightous feeling.\n\r", ch);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_PROTECT_GOOD) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_PROTECT_GOOD)) {
    send_to_char("A cloak of chaotic energy surrounds you.\n\r", ch);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_HIDE) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_HIDE)) {
    send_to_char("You try to hide.\n\r", ch);
    act("$n's tries to hide in the shadows.", TRUE, ch, 0, 0, TO_ROOM);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_SNEAK) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_SNEAK)) {
    send_to_char("You start moving quietly.\n\r", ch);
    act("$n's starts moving quietly.", TRUE, ch, 0, 0, TO_ROOM);
  }

  if (!IS_SET(ch->specials.affected_by, AFF2_FORTIFICATION) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF2_FORTIFICATION)) {
    send_to_char("You feel able to withstand any attack.\n\r", ch);
    act("$n seems able to withstand any attack.", TRUE, ch, 0, 0, TO_ROOM);
  }

  if (!IS_SET(ch->specials.affected_by, AFF2_PERCEIVE) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF2_PERCEIVE)) {
    send_to_char("Your eyes glow with unearthly light.\n\r", ch);
  }

  if (!IS_SET(ch->specials.affected_by, AFF2_RAGE) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF2_RAGE)) {
    send_to_char("A surge of rage and bloodlust courses through your body!\n\r", ch);
    act("$n's eyes turn blood-red, rage and bloodlust coursing through $s body.", FALSE, ch, 0, 0, TO_ROOM);
  }
}


void show_bitvector_remove(struct char_data *ch, struct obj_data *obj_object) {
  if (!IS_SET(ch->specials.affected_by, AFF_IMINV) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_IMINV)) {
    act("$n slowly fades into existence.", TRUE, ch, 0, 0, TO_ROOM);
    send_to_char("You slowly fade into view.\n\r", ch);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_POISON) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_POISON)) {
    send_to_char("You feel better.\n\r", ch);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_FURY) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_FURY)) {
    send_to_char("You calm down.\n\r", ch);
    act("$n stops snarling and fuming with rage.", FALSE, ch, 0, 0, TO_ROOM);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_BLIND) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_BLIND)) {
    send_to_char("You can see again.\n\r", ch);
    act("$n is no longer blinded.", FALSE, ch, 0, 0, TO_ROOM);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_PARALYSIS) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_PARALYSIS)) {
    act("You can move again.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n is no longer paralyzed.", TRUE, ch, 0, 0, TO_ROOM);
  }

  if ((!IS_SET(ch->specials.affected_by, AFF_DETECT_INVISIBLE) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_DETECT_INVISIBLE)) ||
    (!IS_SET(ch->specials.affected_by, AFF_DETECT_MAGIC) &&
      IS_SET(obj_object->obj_flags.bitvector, AFF_DETECT_MAGIC)) ||
      (!IS_SET(ch->specials.affected_by, AFF_DETECT_ALIGNMENT) &&
        IS_SET(obj_object->obj_flags.bitvector, AFF_DETECT_ALIGNMENT))) {
    send_to_char("Your eyes stop tingling.\n\r", ch);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_FLY) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_FLY)) {
    send_to_char("You stop flying and fall to the ground.\n\r", ch);
    act("$n stops flying and falls to the ground.", TRUE, ch, 0, 0, TO_ROOM);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_INFRAVISION) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_INFRAVISION)) {
    act("$n's eyes stop glowing.", TRUE, ch, 0, 0, TO_ROOM);
    send_to_char("Your eyes stop glowing.\n\r", ch);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_SANCTUARY) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_SANCTUARY)) {
    act("The white aura around $n's body fades.", TRUE, ch, 0, 0, TO_ROOM);
    act("The white aura around your body fades.", TRUE, ch, 0, 0, TO_CHAR);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_INVUL) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_INVUL)) {
    act("$n's powerful sphere disappears.", TRUE, ch, 0, 0, TO_ROOM);
    act("Your powerful sphere disappears.", TRUE, ch, 0, 0, TO_CHAR);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_SPHERE) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_SPHERE)) {
    act("$n's golden sphere disappears.", TRUE, ch, 0, 0, TO_ROOM);
    act("Your golden sphere disappears.", TRUE, ch, 0, 0, TO_CHAR);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_PROTECT_EVIL) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_PROTECT_EVIL)) {
    send_to_char("You stop feeling rightous.\n\r", ch);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_PROTECT_GOOD) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_PROTECT_GOOD)) {
    send_to_char("Your protective aura of chaos disappears.\n\r", ch);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_HIDE) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_HIDE)) {
    send_to_char("You stop hiding.\n\r", ch);
    act("$n's stops hiding.", TRUE, ch, 0, 0, TO_ROOM);
  }

  if (!IS_SET(ch->specials.affected_by, AFF_SNEAK) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF_SNEAK)) {
    send_to_char("You stop sneaking.\n\r", ch);
    act("$n stops sneaking.", TRUE, ch, 0, 0, TO_ROOM);
  }

  if (!IS_SET(ch->specials.affected_by, AFF2_FORTIFICATION) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF2_FORTIFICATION)) {
    send_to_char("Your feeling of fortification diminishes.\n\r", ch);
    act("$n's magical foritication diminishes.", TRUE, ch, 0, 0, TO_ROOM);
  }

  if (!IS_SET(ch->specials.affected_by, AFF2_PERCEIVE) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF2_PERCEIVE)) {
    send_to_char("Your depth of perception diminishes.\n\r", ch);
  }

  if (!IS_SET(ch->specials.affected_by, AFF2_RAGE) &&
    IS_SET(obj_object->obj_flags.bitvector, AFF2_RAGE)) {
    send_to_char("You calm down and feel less angry.\n\r", ch);
    act("$n calms down and looks less angry.", TRUE, ch, 0, 0, TO_ROOM);
  }
}


void wear(CHAR *ch, OBJ *obj, int eq_slot) {
  typedef struct wear_info_t {
    int eq_slot;
    int wear_pos;
    int wear_pos2;
    bool is_two_slot;
    char *cant_wear;
    char *already_worn;
    char *ch_wear;
    char *rm_wear;
  } wear_info_t;

  const wear_info_t wear_info[] = {
    {
      ITEM_LIGHT_SOURCE,
      WEAR_LIGHT,
      -1,
      FALSE,
      "You can't hold that.",
      "You are already holding a light source.",
      "You light $p and hold it.",
      "$n lights $p and holds it.",
    },
    {
      ITEM_WEAR_FINGER,
      WEAR_FINGER_R,
      WEAR_FINGER_L,
      FALSE,
      "You can't wear that on your finger.",
      "You already wear something on both your fingers.",
      "You put the $p on your %s finger.",
      "$n wears $p on $s finger.",
    },
    {
      ITEM_WEAR_NECK,
      WEAR_NECK_1,
      WEAR_NECK_2,
      FALSE,
      "You can't wear that around your neck.",
      "You can't wear any more around your neck.",
      "You wear $p around your neck.",
      "$n wears $p around $s neck.",
    },
    {
      ITEM_WEAR_2NECK,
      WEAR_NECK_1,
      WEAR_NECK_2,
      TRUE,
      "You can't wear that around your neck.",
      "You can't wear any more around your neck.",
      "You wear $p around your neck.",
      "$n wears $p around $s neck.",
    },
    {
      ITEM_WEAR_BODY,
      WEAR_BODY,
      -1,
      FALSE,
      "You can't wear that on your body.",
      "You already wear something on your body.",
      "You wear $p on your body.",
      "$n wears $p on $s body.",
    },
    {
      ITEM_WEAR_HEAD,
      WEAR_HEAD,
      -1,
      FALSE,
      "You can't wear that on your head.",
      "You already wear something on your head.",
      "You wear $p on your head.",
      "$n wears $p on $s head.",
    },
    {
      ITEM_WEAR_LEGS,
      WEAR_LEGS,
      -1,
      FALSE,
      "You can't wear that on your legs.",
      "You already wear something on your legs.",
      "You wear $p on your legs.",
      "$n wears $p on $s legs.",
    },
    {
      ITEM_WEAR_FEET,
      WEAR_FEET,
      -1,
      FALSE,
      "You can't wear that on your feet.",
      "You already wear something on your feet.",
      "You wear $p on your feet.",
      "$n wears $p on $s feet.",
    },
    {
      ITEM_WEAR_HANDS,
      WEAR_HANDS,
      -1,
      FALSE,
      "You can't wear that on your hands.",
      "You already wear something on your hands.",
      "You wear $p on your hands.",
      "$n wears $p on $s hands.",
    },
    {
      ITEM_WEAR_ARMS,
      WEAR_ARMS,
      -1,
      FALSE,
      "You can't wear that on your arms.",
      "You already wear something on your arms.",
      "You wear $p on your arms.",
      "$n wears $p on $s arms.",
    },
    {
      ITEM_WEAR_SHIELD,
      WEAR_SHIELD,
      -1,
      FALSE,
      "You can't hold this.",
      "You are already holding a light source.",
      "You light $p and hold it.",
      "$n lights $p and holds it.",
    },
    {
      ITEM_WEAR_ABOUT,
      WEAR_ABOUT,
      -1,
      FALSE,
      "You can't wear that about your body.",
      "You already wear something about your body.",
      "You wear $p about your body.",
      "$n wears $p about $s body.",
    },
    {
      ITEM_WEAR_WAIST,
      WEAR_WAIST,
      -1,
      FALSE,
      "You can't wear that about your waist.",
      "You already wear something about your waist.",
      "You wear $p about your waist.",
      "$n wears $p about $s waist.",
    },
    {
      ITEM_WEAR_WRIST,
      WEAR_WRIST_R,
      WEAR_WRIST_L,
      FALSE,
      "You can't wear that around your wrist.",
      "You already wear something around both your wrists.",
      "You wear the $p around your %s wrist.",
      "$n wears $p around $s wrist.",
    },
    {
      ITEM_WIELD,
      WIELD,
      -1,
      FALSE, // Always false, even for 2H weapons, due to using ITEM_TYPE instead.
      "You can't wield that.",
      "You are already wielding something.",
      "You wield $p.",
      "$n wields $p.",
    },
    {
      ITEM_HOLD,
      HOLD,
      -1,
      FALSE,
      "You can't hold that.",
      "You are already holding something.",
      "You grab $p.",
      "$n grabs $p.",
    },
  };

  if (!ch || !obj) return;

  /* Validate that we're trying to equip an available eq_slot. */
  int index = -1;

  for (int i = 0; i < NUMELEMS(wear_info); i++) {
    if (wear_info[i].eq_slot == eq_slot) {
      index = i;

      break;
    }
  }

  if (index == -1) {
    log_f("Unknown equipment slot '%d' passed to wear().", eq_slot);

    return;
  }

  int wear_pos = -1;

  /* Handle WIELD separately, since its more complex. */
  if (eq_slot == ITEM_WIELD) {
    if ((OBJ_TYPE(obj) == ITEM_2HWEAPON) && (EQ(ch, WIELD) || EQ(ch, HOLD))) {
      printf_to_char(ch, "You need both hands to wield this.\n\r");

      return;
    }

    if (GET_CLASS(ch) == CLASS_CLERIC) {
      if (((OBJ_TYPE(obj) == ITEM_WEAPON) || (OBJ_TYPE(obj) == ITEM_WEAPON)) &&
          ((OBJ_VALUE(obj, 3) == 3) || (OBJ_VALUE(obj, 3) > 8))) {
        printf_to_char(ch, "You can't wield that; it's SHARP!\n\rYour religion forbids the use of sharp weapons!\n\r");

        return;
      }
    }

    if (GET_CLASS(ch) == CLASS_MAGIC_USER) {
      if (((OBJ_TYPE(obj) == ITEM_WEAPON) || (OBJ_TYPE(obj) == ITEM_WEAPON)) &&
          ((OBJ_VALUE(obj, 1) > 3) || (OBJ_VALUE(obj, 2) > 9))) {
        /* Mage Weapon Exceptions */
        if ((V_OBJ(obj) != 129) &&
            (V_OBJ(obj) != 570) &&
            (V_OBJ(obj) != 29981)) {
          printf_to_char(ch, "Your magical training did not include the use of this weapon!\n\r");

          return;
        }
      }
    }

    if (GET_CLASS(ch) == CLASS_NINJA) {
      if (EQ(ch, WIELD) && (OBJ_TYPE(obj) == ITEM_2HWEAPON)) {
        printf_to_char(ch, "You are already wielding a two-handed weapon.\n\r");

        return;
      }

      if (EQ(ch, WIELD) && EQ(ch, HOLD)) {
        if (OBJ_TYPE(EQ(ch, HOLD)) == ITEM_WEAPON) {
          printf_to_char(ch, "You are already wielding two weapons.\n\r");
        }
        else {
          printf_to_char(ch, "You are already wielding and holding something.\n\r");
        }

        return;
      }

      if (EQ(ch, WIELD) && (OBJ_TYPE(EQ(ch, WIELD)) != ITEM_2HWEAPON) && !EQ(ch, HOLD)) {
        wear_pos = HOLD;
      }
    }

    /* Check weapon weight if object is being wielded. */
    if (((OBJ_TYPE(obj) == ITEM_WEAPON) || (OBJ_TYPE(obj) == ITEM_2HWEAPON)) &&
        (OBJ_WEIGHT(obj) > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)) {
      printf_to_char(ch, "It's too heavy for you to use.\n\r");

      return;
    }
  }

  /* Handle HOLD separately, since its more complex. */
  if (eq_slot == ITEM_HOLD) {
    if (EQ(ch, WIELD) && (OBJ_TYPE(EQ(ch, WIELD)) == ITEM_2HWEAPON)) {
      printf_to_char(ch, "You're wielding a two-handed weapon, so you can't hold that.\n\r");

      return;
    }
  }

  /* Can we wear the object in the eq slot we've been told to wear it on? */
  if (!CAN_WEAR(obj, wear_info[index].eq_slot)) {
    act(wear_info[index].cant_wear, TRUE, ch, obj, 0, TO_CHAR);

    return;
  }

  /* Can we wear the object in the 1st position?*/
  if ((wear_pos == -1) && !EQ(ch, wear_info[index].wear_pos) && (!wear_info[index].is_two_slot || (wear_info[index].is_two_slot && !EQ(ch, wear_info[index].wear_pos2)))) {
    wear_pos = wear_info[index].wear_pos;
  }

  /* Do some crazy stuff to check if we're already wearing a two-slot item in the first wear position. */
  bool wearing_two_slot = FALSE;

  /* We only need to perform the two-slot logic if slot 1 wasn't eligible. */
  if (wear_pos == -1) {
    /* Loop through the wear_info struct array and look for the element defining the two-slot version of the specified eq slot. */
    for (int i = 0; i < NUMELEMS(wear_info); i++) {
      if (i == index) continue; // No need to check the same eq slot info; we're looking for the other slot with the similar wear positions.

      /* Is this the two-slot version we're looking for? */
      if (wear_info[i].is_two_slot && (wear_info[i].wear_pos == wear_info[index].wear_pos)) {
        /* Check the wear positions of the two-slot version and see if we're wearing a two-slot item of that type in either position already. */
        if ((EQ(ch, wear_info[i].wear_pos) && IS_SET(OBJ_WEAR_FLAGS(EQ(ch, wear_info[i].wear_pos)), wear_info[i].eq_slot))) {
          wearing_two_slot = TRUE; // The object already equipped in wear position 1 or 2 is a two-slot version of the object we're trying to wear.

          break;
        }
      }
    }
  }

  /* Can we wear the object in the 2nd position? */
  if ((wear_pos == -1) && !wear_info[index].is_two_slot && !wearing_two_slot && !EQ(ch, wear_info[index].wear_pos2)) {
    wear_pos = wear_info[index].wear_pos2;
  }

  /* Something is already worn in the eq slot. */
  if (wear_pos == -1) {
    act(wear_info[index].already_worn, TRUE, ch, obj, 0, TO_CHAR);

    return;
  }

  if (signal_object(obj, ch, MSG_OBJ_WORN, "")) return;

  show_bitvector_wear(ch, obj);

  bool show_to_ch = TRUE, show_to_rm = TRUE;

  if (V_OBJ(obj) && OBJ_CWEAR_DESC(obj)) {
    act(OBJ_CWEAR_DESC(obj), FALSE, ch, obj, 0, TO_CHAR);

    show_to_ch = FALSE;
  }

  if (V_OBJ(obj) && OBJ_RWEAR_DESC(obj)) {
    act(OBJ_RWEAR_DESC(obj), FALSE, ch, obj, 0, TO_ROOM);

    show_to_rm = FALSE;
  }

  if (show_to_ch) {
    char buf[MIL];

    snprintf(buf, sizeof(buf), wear_info[index].ch_wear, ((wear_pos == wear_info[index].wear_pos) ? "right" : "left"));

    act(buf, TRUE, ch, obj, 0, TO_CHAR);
  }

  if (show_to_rm) {
    act(wear_info[index].rm_wear, TRUE, ch, obj, 0, TO_ROOM);
  }

  equip_char(ch, obj_from_char(obj), wear_pos);
}


void do_wear(CHAR *ch, char *arg, int cmd) {
  typedef struct eq_slot_info_t {
    char *eq_slot_keyword;
    int eq_slot_wear_flag;
  } eq_slot_info_t;

  const eq_slot_info_t eq_slot_info[] = {
    { "finger", ITEM_WEAR_FINGER },
    { "neck",   ITEM_WEAR_NECK   },
    { "neck",   ITEM_WEAR_2NECK  },
    { "body",   ITEM_WEAR_BODY   },
    { "head",   ITEM_WEAR_HEAD   },
    { "legs",   ITEM_WEAR_LEGS   },
    { "feet",   ITEM_WEAR_FEET   },
    { "hands",  ITEM_WEAR_HANDS  },
    { "arms",   ITEM_WEAR_ARMS   },
    { "shield", ITEM_WEAR_SHIELD },
    { "about",  ITEM_WEAR_ABOUT  },
    { "waist",  ITEM_WEAR_WAIST  },
    { "wrist",  ITEM_WEAR_WRIST  },
  };

  char obj_name[MIL], eq_slot_keyword[MIL];

  argument_interpreter(arg, obj_name, eq_slot_keyword);

  if (!*obj_name) {
    printf_to_char(ch, "Wear what?\n\r");

    return;
  }

  if (!strcmp(obj_name, "all")) {
    for (OBJ *temp_obj = ch->carrying, *next_obj; temp_obj; temp_obj = next_obj) {
      next_obj = temp_obj->next_content;

      int eq_slot = -1;

      for (int i = 0; i < NUMELEMS(eq_slot_info); i++) {
        if (!OBJ_EQUIPPED_BY(temp_obj) && CAN_WEAR(temp_obj, eq_slot_info[i].eq_slot_wear_flag)) {
          eq_slot = eq_slot_info[i].eq_slot_wear_flag;

          wear(ch, temp_obj, eq_slot);

          break;
        }
      }

      if (eq_slot == -1) {
        printf_to_char(ch, "You can't wear the %s.\n\r", fname(OBJ_NAME(temp_obj)));
      }
    }
  }
  else {
    OBJ *temp_obj = get_obj_in_list_vis(ch, obj_name, ch->carrying);

    if (!temp_obj) {
      printf_to_char(ch, "You do not seem to have the '%s'.\n\r", obj_name);

      return;
    }

    if (*eq_slot_keyword) {
      int eq_slot = -1;

      for (int i = 0; i < NUMELEMS(eq_slot_info); i++) {
        if (!strncmp(eq_slot_keyword, eq_slot_info[i].eq_slot_keyword, strlen(eq_slot_keyword)) &&
            CAN_WEAR(temp_obj, eq_slot_info[i].eq_slot_wear_flag)) {
          eq_slot = eq_slot_info[i].eq_slot_wear_flag;

          break;
        }
      }

      if (eq_slot == -1) {
        printf_to_char(ch, "Unknown equipment location '%s'.\n\r", eq_slot_keyword);

        return;
      }

      wear(ch, temp_obj, eq_slot);
    }
    else {
      int eq_slot = -1;

      for (int i = 0; i < NUMELEMS(eq_slot_info); i++) {
        if (!OBJ_EQUIPPED_BY(temp_obj) && CAN_WEAR(temp_obj, eq_slot_info[i].eq_slot_wear_flag)) {
          eq_slot = eq_slot_info[i].eq_slot_wear_flag;

          wear(ch, temp_obj, eq_slot);

          break;
        }
      }

      if (eq_slot == -1) {
        printf_to_char(ch, "You can't wear the %s.\n\r", fname(OBJ_NAME(temp_obj)));
      }
    }
  }

  check_equipment(ch);
}


void do_wield(CHAR *ch, char *arg, int cmd) {
  char obj_name[MIL];

  one_argument(arg, obj_name);

  if (!*obj_name) {
    printf_to_char(ch, "Wield what?\n\r");

    return;
  }

  OBJ *obj = get_obj_in_list_vis(ch, obj_name, ch->carrying);

  if (!obj) {
    printf_to_char(ch, "You do not seem to have the '%s'.\n\r", obj_name);

    return;
  }

  wear(ch, obj, ITEM_WIELD);

  check_equipment(ch);
}


void do_grab(CHAR *ch, char *arg, int cmd) {
  char obj_name[MIL];

  one_argument(arg, obj_name);

  if (!*obj_name) {
    printf_to_char(ch, "Hold what?\n\r");

    return;
  }

  OBJ *obj = get_obj_in_list_vis(ch, obj_name, ch->carrying);

  if (!obj) {
    printf_to_char(ch, "You do not seem to have the '%s'.\n\r", obj_name);

    return;
  }

  if (OBJ_TYPE(obj) == ITEM_LIGHT) {
    wear(ch, obj, ITEM_LIGHT_SOURCE);
  }
  else {
    wear(ch, obj, ITEM_HOLD);
  }

  check_equipment(ch);
}


void remove_item(struct char_data *ch, struct obj_data *obj, int pos)
{
  if (CAN_CARRY_N(ch) > IS_CARRYING_N(ch)) {
    if(!signal_object(obj, ch, MSG_BEING_REMOVED, "")) {
      if(CAN_WEAR(obj,ITEM_NO_REMOVE) && GET_LEVEL(ch)<LEVEL_IMM ) {
        act("You can't remove $p.",0,ch,obj,0,TO_CHAR);
      }
      else {
        if(IS_SET(obj->obj_flags.extra_flags2,ITEM_EQ_DECAY) &&
           obj->obj_flags.timer>0) {
          obj->obj_flags.timer--;
        }

        if(V_OBJ(obj)>0 && OBJ_CREM_DESC(obj))
          act(OBJ_CREM_DESC(obj),FALSE,ch,obj,0,TO_CHAR);
        else
          act("You stop using $p.",FALSE,ch,obj,0,TO_CHAR);
        if(V_OBJ(obj)>0 && OBJ_RREM_DESC(obj))
          act(OBJ_RREM_DESC(obj),FALSE,ch,obj,0,TO_ROOM);
        else
          act("$n stops using $p.",TRUE,ch,obj,0,TO_ROOM);
        obj_to_char(unequip_char(ch, pos), ch);
        show_bitvector_remove(ch,obj);
        if (obj->obj_flags.type_flag == ITEM_LIGHT)
          if (obj->obj_flags.value[2])
               world[CHAR_REAL_ROOM(ch)].light--;
      }
    }
  } else {
    send_to_char("You can't carry that many items.\n\r", ch);
  }
  check_equipment(ch);
}

void do_remove(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  struct obj_data *obj_object;
  int i, j;

  one_argument(argument, arg1);

  if (!*arg1) {
    send_to_char("\
remove <obj> or\n\r\
       all/light/rfinger/lfinger/neck1/neck2/body/head/legs/feet/\n\r\
       hands/arms/shield/about/waist/rwrist/lwrist/wield/hold\n\r",ch);
    return;
  }

  if (!strcmp(arg1, "all")) {
    for (i=0; i < MAX_WEAR; i++) {
      obj_object = ch->equipment[i];
      if (obj_object)
        remove_item(ch, obj_object, i);
    }
    return;
  }

  if(!strcmp(arg1,"light")) {
    if(ch->equipment[WEAR_LIGHT])
      remove_item(ch, ch->equipment[WEAR_LIGHT], WEAR_LIGHT);
    else
      send_to_char("You are not holding a light.\n\r",ch);
    return;
  }

  if(!strcmp(arg1,"rfinger")) {
    if(ch->equipment[WEAR_FINGER_R])
      remove_item(ch, ch->equipment[WEAR_FINGER_R], WEAR_FINGER_R);
    else
      send_to_char("You aren't wearing anything on your right finger.\n\r",ch);
    return;
  }

  if(!strcmp(arg1,"lfinger")) {
    if(ch->equipment[WEAR_FINGER_L])
      remove_item(ch, ch->equipment[WEAR_FINGER_L], WEAR_FINGER_L);
    else
      send_to_char("You aren't wearing anything on your left finger.\n\r",ch);
    return;
  }

  if(!strcmp(arg1,"neck1")) {
    if(ch->equipment[WEAR_NECK_1])
      remove_item(ch, ch->equipment[WEAR_NECK_1], WEAR_NECK_1);
    else
      send_to_char("You aren't wearing anything on your first next position.\n\r",ch);
    return;
  }

  if(!strcmp(arg1,"neck2")) {
    if(ch->equipment[WEAR_NECK_2])
      remove_item(ch, ch->equipment[WEAR_NECK_2], WEAR_NECK_2);
    else
      send_to_char("You aren't wearing anything on your second next position.\n\r",ch);
    return;
  }

  if(!strcmp(arg1,"body")) {
    if(ch->equipment[WEAR_BODY])
      remove_item(ch, ch->equipment[WEAR_BODY], WEAR_BODY);
    else
      send_to_char("You aren't wearing anything on your body.\n\r",ch);
    return;
  }

  if(!strcmp(arg1,"head")) {
    if(ch->equipment[WEAR_HEAD])
      remove_item(ch, ch->equipment[WEAR_HEAD], WEAR_HEAD);
    else
      send_to_char("You aren't wearing anything on your head.\n\r",ch);
    return;
  }

  if(!strcmp(arg1,"legs")) {
    if(ch->equipment[WEAR_LEGS])
      remove_item(ch, ch->equipment[WEAR_LEGS], WEAR_LEGS);
    else
      send_to_char("You aren't wearing anything on your legs.\n\r",ch);
    return;
  }

  if(!strcmp(arg1,"feet")) {
    if(ch->equipment[WEAR_FEET])
      remove_item(ch, ch->equipment[WEAR_FEET], WEAR_FEET);
    else
      send_to_char("You aren't wearing anything on your feet.\n\r",ch);
    return;
  }

  if(!strcmp(arg1,"hands")) {
    if(ch->equipment[WEAR_HANDS])
      remove_item(ch, ch->equipment[WEAR_HANDS], WEAR_HANDS);
    else
      send_to_char("You aren't wearing anything on your hands.\n\r",ch);
    return;
  }

  if(!strcmp(arg1,"arms")) {
    if(ch->equipment[WEAR_ARMS])
      remove_item(ch, ch->equipment[WEAR_ARMS], WEAR_ARMS);
    else
      send_to_char("You aren't wearing anything on your arms.\n\r",ch);
    return;
  }

  if(!strcmp(arg1,"shield")) {
    if(ch->equipment[WEAR_SHIELD])
      remove_item(ch, ch->equipment[WEAR_SHIELD], WEAR_SHIELD);
    else
      send_to_char("You aren't wearing a shield.\n\r",ch);
    return;
  }

  if(!strcmp(arg1,"about")) {
    if(ch->equipment[WEAR_ABOUT])
      remove_item(ch, ch->equipment[WEAR_ABOUT], WEAR_ABOUT);
    else
      send_to_char("You aren't wearing anything about your body.\n\r",ch);
    return;
  }

  if(!strcmp(arg1,"waist")) {
    if(ch->equipment[WEAR_WAIST])
      remove_item(ch, ch->equipment[WEAR_WAIST], WEAR_WAIST);
    else
      send_to_char("You aren't wearing anything around your waist.\n\r",ch);
    return;
  }

  if(!strcmp(arg1,"waiste")) {
    if(ch->equipment[WEAR_WAIST])
      remove_item(ch, ch->equipment[WEAR_WAIST], WEAR_WAIST);
    else
      send_to_char("You aren't wearing anything around your waist.\n\r",ch);
    return;
  }

  if(!strcmp(arg1,"rwrist")) {
    if(ch->equipment[WEAR_WRIST_R])
      remove_item(ch, ch->equipment[WEAR_WRIST_R], WEAR_WRIST_R);
    else
      send_to_char("You aren't wearing anything on your right wrist.\n\r",ch);
    return;
  }

  if(!strcmp(arg1,"lwrist")) {
    if(ch->equipment[WEAR_WRIST_L])
      remove_item(ch, ch->equipment[WEAR_WRIST_L], WEAR_WRIST_L);
    else
      send_to_char("You aren't wearing anything on your left wrist.\n\r",ch);
    return;
  }

  if(!strcmp(arg1,"wield")) {
    if(ch->equipment[WIELD])
      remove_item(ch, ch->equipment[WIELD], WIELD);
    else
      send_to_char("You aren't wielding anything.\n\r",ch);
    return;
  }

  if(!strcmp(arg1,"hold")) {
    if(ch->equipment[HOLD])
      remove_item(ch, ch->equipment[HOLD], HOLD);
    else
      send_to_char("You aren't holding anything.\n\r",ch);
    return;
  }

  obj_object = get_object_in_equip_vis(ch, arg1, ch->equipment, &j);
  if(obj_object) {
    remove_item(ch, obj_object, j);
    return;
  }
  send_to_char("You are not using it.\n\r", ch);
  return;
}



/*void do_remove(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  struct obj_data *obj_object;
  int i, j;

  one_argument(argument, arg1);

  if (*arg1) {
    if (!strcmp(arg1, "all")) {
      for (i=0; i < MAX_WEAR; i++) {
     obj_object = ch->equipment[i];
     if (obj_object)
       remove_item(ch, obj_object, i);
      }
    } else {
      obj_object = get_object_in_equip_vis(ch, arg1, ch->equipment, &j);
      if (obj_object)
     remove_item(ch, obj_object, j);
      else
     send_to_char("You are not using it.\n\r", ch);
    }
  } else {
    send_to_char("Remove what?\n\r", ch);
  }
}
*/
