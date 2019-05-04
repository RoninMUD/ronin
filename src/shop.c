/* ************************************************************************
*  file: shop.c , Shop module.                            Part of DIKUMUD *
*  Usage: Procedures handling shops and shopkeepers.                      *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "structs.h"
#include "constants.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "limits.h"
#include "interpreter.h"
#include "spells.h"
#include "utils.h"
#include "fight.h"
#include "act.h"
#include "cmd.h"
#include "utility.h"
#include "shop.h"
#include "spec_assign.h"

char *fread_string(FILE *fl);

struct shop_data *shop_index;
int number_of_shops=0;

int is_ok(CHAR *keeper, CHAR *ch, int shop_nr) {
  if (shop_index[shop_nr].open1 > time_info.hours) {
    do_say(keeper, "Come back later!", CMD_SAY);

    return FALSE;
  }
  else if (shop_index[shop_nr].close1 < time_info.hours) {
    if (shop_index[shop_nr].open2 > time_info.hours) {
      do_say(keeper, "Sorry, we have closed, but come back later.", CMD_SAY);

      return FALSE;
    }
    else if (shop_index[shop_nr].close2 < time_info.hours) {
      do_say(keeper, "Sorry, come back tomorrow.", CMD_SAY);

      return FALSE;
    }
  }

  if ((IS_SET(GET_PFLAG(ch), PLR_KILL) || IS_SET(GET_PFLAG(ch), PLR_THIEF)) && IS_MORTAL(ch) && !CHAOSMODE) {
    act("$n screams 'I don't trade with a killer or thief!'", FALSE, keeper, 0, 0, TO_ROOM);

    hit(keeper, ch, 0);

    return FALSE;
  }

  return TRUE;
}

int trade_with(struct obj_data *item, int shop_nr)
{
  int counter;

  if(item->obj_flags.cost < 1) return(FALSE);
  if(IS_SET(item->obj_flags.extra_flags, ITEM_ANTI_RENT)) return FALSE;
  if(item->obj_flags.type_flag == ITEM_TRASH) return FALSE;
  if(IS_SET(item->obj_flags.extra_flags, ITEM_CLONE)) return FALSE;

  for(counter=0;counter<MAX_TRADE;counter++) {
    if(shop_index[shop_nr].type[counter]==item->obj_flags.type_flag)
      return(TRUE);
    if(shop_index[shop_nr].type[counter]==ITEM_WEAPON &&
       item->obj_flags.type_flag==ITEM_2H_WEAPON)
      return(TRUE);
  }
  return(FALSE);
}

int shop_producing(struct obj_data *item, int shop_nr)
{
  int counter;

  if(item->item_number<0) return(FALSE);

  for(counter=0;counter<MAX_PROD;counter++)
    if (shop_index[shop_nr].producing[counter] == V_OBJ(item))
      return(TRUE);
  return(FALSE);
}

/* Added optional buy <number> - Ranger July 1998 */
void shopping_buy( char *arg, CHAR *ch, CHAR *keeper, int shop_nr) {
  char arg1[MAX_STRING_LENGTH],arg2[MAX_STRING_LENGTH],buf[MAX_STRING_LENGTH+100];
  struct obj_data *temp1;
  int number=1;

  if(!(is_ok(keeper,ch,shop_nr))) return;

  arg=one_argument(arg, arg1);
  arg=one_argument(arg, arg2);

  if((*arg2)) {
    if(!is_number(arg1)) {
      send_to_char ("Syntax for BUY is: BUY [number] <item>\n\r\"number\" is an optional number of items to buy.\n\r",ch);
      return;
    }
    number=atoi(arg1);
    strcpy (arg1,arg2);
  }

  if(number<1 || number > 50) {
    send_to_char("The number must be between 1 and 50.\n\r",ch);
    return;
  }

  if(!(*arg1)) {
    sprintf(buf, "%s what do you want to buy??", GET_NAME(ch));
    do_tell(keeper,buf,19);
    return;
  }

  if(!( temp1 = get_obj_in_list_vis(ch,arg1,keeper->carrying))) {
    sprintf(buf, shop_index[shop_nr].no_such_item1 ,GET_NAME(ch));
    do_tell(keeper,buf,19);
    return;
  }

  if(temp1->obj_flags.cost <= 0) {
    sprintf(buf, shop_index[shop_nr].no_such_item1, GET_NAME(ch));
    do_tell(keeper,buf,19);
    extract_obj(temp1);
    return;
  }

  int cost = (int)(OBJ_COST(temp1) * shop_index[shop_nr].profit_buy * number);

  // Prestige Perk 16
  if (GET_PRESTIGE_PERK(ch) >= 16) {
    cost *= 0.95;
  }

  if(GET_GOLD(ch) < cost &&
     GET_LEVEL(ch)<LEVEL_IMM) {
    sprintf(buf, shop_index[shop_nr].missing_cash2, GET_NAME(ch));
    do_tell(keeper,buf,19);
    switch(shop_index[shop_nr].temper1) {
      case 0:
        do_action(keeper,GET_NAME(ch),30);
        return;
      case 1:
        do_emote(keeper,"smokes on his joint",36);
        return;
      default:
        return;
    }
  }

  if(number>1) {
    if(!shop_producing(temp1,shop_nr)) {
      sprintf(buf,"%s I only have one of those.",GET_NAME(ch));
      do_tell(keeper,buf,CMD_TELL);
      return;
    }
  }

  if((IS_CARRYING_N(ch) + number > CAN_CARRY_N(ch))) {
    if(number>1)
      sprintf(buf,"%d*%s : You can't carry that many items.\n\r",number,fname(OBJ_NAME(temp1)));
    else
      sprintf(buf,"%s : You can't carry that many items.\n\r",fname(OBJ_NAME(temp1)));
    send_to_char(buf, ch);
    return;
  }

  if((IS_CARRYING_W(ch) + temp1->obj_flags.weight*number) > CAN_CARRY_W(ch)) {
    if(number>1)
      sprintf(buf,"%d*%s : You can't carry that much weight.\n\r",number,fname(OBJ_NAME(temp1)));
    else
      sprintf(buf,"%s : You can't carry that much weight.\n\r",fname(OBJ_NAME(temp1)));
    send_to_char(buf, ch);
    return;
  }

  if(number>1)
    sprintf(buf,"$n buys %d*$p.",number);
  else
    sprintf(buf,"$n buys $p.");
  act(buf, FALSE, ch, temp1, 0, TO_ROOM);

  sprintf(buf, shop_index[shop_nr].message_buy, GET_NAME(ch),
    cost);
  do_tell(keeper,buf,19);

  if(number>1)
    sprintf(buf,"You now have %d*%s.\n\r",number,OBJ_SHORT(temp1));
  else
    sprintf(buf,"You now have %s.\n\r",OBJ_SHORT(temp1));
  send_to_char(buf,ch);

  if(GET_LEVEL(ch)<LEVEL_IMM)
    GET_GOLD(ch) -= cost;

  GET_GOLD(keeper) += cost;

  /* Test if producing shop ! */
  if(shop_producing(temp1,shop_nr)) {
    for ( ;number> 0;number--) {
      temp1=read_object(temp1->item_number,REAL);
      obj_to_char(temp1,ch);
    }
  }
  else {
    obj_from_char(temp1);
    obj_to_char(temp1,ch);
  }
  save_char(ch,NOWHERE);
  return;
}

void shopping_sell( char *arg, CHAR *ch,CHAR *keeper,int shop_nr)
{
  char argm[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
  struct obj_data *temp1,*obj,*obj_n;

  if(!(is_ok(keeper,ch,shop_nr)))
    return;

  one_argument(arg, argm);

  if(!(*argm))
    {
      sprintf(buf, "%s What do you want to sell??", GET_NAME(ch));
      do_tell(keeper,buf,19);
      return;
    }

  if(!( temp1 = get_obj_in_list_vis(ch,argm,ch->carrying)))
    {
      sprintf(buf, shop_index[shop_nr].no_such_item2, GET_NAME(ch));
      do_tell(keeper,buf,19);
      return;
    }

  if(!(trade_with(temp1,shop_nr))||(temp1->obj_flags.cost<1))
    {
      sprintf(buf, shop_index[shop_nr].do_not_buy ,GET_NAME(ch));
      do_tell(keeper,buf,19);
      return;
    }

  if(GET_GOLD(keeper)<100000) GET_GOLD(keeper)=100000;
     /* Always have at least 100K - Ranger Feb 99 */

  int coins = (int)(temp1->obj_flags.cost * shop_index[shop_nr].profit_sell);

  if (GET_GOLD(keeper) < coins) {
    snprintf(buf, sizeof(buf), shop_index[shop_nr].missing_cash1, GET_NAME(ch));

    do_tell(keeper, buf, 19);

    return;
  }

  if ((INT_MAX - GET_GOLD(ch)) < coins) {
    send_to_char("You can't carry any more coins.\n\r", ch);

    return;
  }

  act("$n sells $p.", FALSE, ch, temp1, 0, TO_ROOM);

  sprintf(buf, shop_index[shop_nr].message_sell, GET_NAME(ch), coins);
  do_tell(keeper, buf, 19);

  sprintf(buf, "The shopkeeper now has %s.\n\r", OBJ_SHORT(temp1));
  send_to_char(buf, ch);

  GET_GOLD(ch) += coins;
  GET_GOLD(keeper) -= coins;

  if (GET_ITEM_TYPE(temp1) == ITEM_TRASH) {
    extract_obj(temp1);
  }

  bool found = FALSE;

  for (obj = keeper->carrying; obj && !found; obj = obj_n) {
    obj_n = obj->next_content;

    if (V_OBJ(obj) == V_OBJ(temp1)) {
      extract_obj(temp1);

      found = TRUE;
    }
  }

  if (!found) {
    obj_from_char(temp1);
    obj_to_char(temp1, keeper);
  }

  save_char(ch, NOWHERE);
}

void shopping_value( char *arg, CHAR *ch,
        CHAR *keeper, int shop_nr)
{
  char argm[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
  struct obj_data *temp1;

  if(!(is_ok(keeper,ch,shop_nr)))
    return;

  one_argument(arg, argm);

  if(!(*argm))
    {
      sprintf(buf,"%s What do you want me to value??",
        GET_NAME(ch));
      do_tell(keeper,buf,19);
      return;
    }

  if(!( temp1 = get_obj_in_list_vis(ch,argm,ch->carrying)))
    {
      sprintf(buf,shop_index[shop_nr].no_such_item2,
        GET_NAME(ch));
      do_tell(keeper,buf,19);
      return;
    }

  if(!(trade_with(temp1,shop_nr)))
    {
      sprintf(buf,
        shop_index[shop_nr].do_not_buy,
        GET_NAME(ch));
      do_tell(keeper,buf,19);
      return;
    }

  sprintf(buf,"%s I'll give you %d gold coins for that!",
    GET_NAME(ch),(int) (temp1->obj_flags.cost*
            shop_index[shop_nr].profit_sell));
  do_tell(keeper,buf,19);

  return;
}

void shopping_list(char *arg, CHAR *ch, CHAR *keeper, int shop_nr) {
  char buf[MSL];
  int count = 0;

  if (!is_ok(keeper, ch, shop_nr)) return;

  printf_to_char(ch, "You can buy:\n\r");

  if (keeper->carrying) {
    for (OBJ *tmp_obj = keeper->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
      if (OBJ_TYPE(tmp_obj) == ITEM_CONTAINER && OBJ_VALUE3(tmp_obj)) continue;
      if (!CAN_SEE_OBJ(ch, tmp_obj) || !OBJ_COST(tmp_obj)) continue;

      count++;

      int cost = (int)(OBJ_COST(tmp_obj) * shop_index[shop_nr].profit_buy);

      // Prestige Perk 16
      if (GET_PRESTIGE_PERK(ch) >= 16) {
        cost *= 0.95;
      }

      if (OBJ_TYPE(tmp_obj) == ITEM_DRINKCON) {
        snprintf(buf, sizeof(buf), "%s of %s", OBJ_SHORT(tmp_obj), drinks[OBJ_VALUE2(tmp_obj)]);
      }
      else {
        snprintf(buf, sizeof(buf), "%s", OBJ_SHORT(tmp_obj));
      }

      printf_to_char(ch, "%s for %d gold coins.\n\r", buf, cost);
    }
  }

  if (!count) {
    printf_to_char(ch, "Nothing!\n\r");
  }
}

void shopping_kill(CHAR *ch, CHAR *keeper) {
  char buf[MAX_STRING_LENGTH];
  sprintf(buf,"%s Don't ever try that again!",GET_NAME(ch));
  do_tell(keeper,buf,19);
  return;
}

void raw_kill(CHAR *ch);
void shop_kickout(CHAR *mob, CHAR *vict) {
  char buf[MAX_STRING_LENGTH];

  if(CHAR_REAL_ROOM(mob)!=CHAR_REAL_ROOM(vict)) return;
  sprintf(buf,"%s Get the hell out of my store!",GET_NAME(vict));
  do_tell(mob,buf,CMD_TELL);
  if(IS_NPC(vict)) {
    divide_experience(mob,vict,1);
    raw_kill(vict);
    return;
  }
  act("$n forces you to flee!",0,mob,0,vict,TO_VICT);
  act("$n forces $N to flee!",0,mob,0,vict,TO_NOTVICT);
  act("You force $N to flee!",0,mob,0,vict,TO_CHAR);
  do_flee(vict,"",CMD_FLEE);
}


int shop_keeper(CHAR *keeper,CHAR *ch,int cmd,char *arg)
{
  char argm[MAX_STRING_LENGTH];
  int shop_nr = 0;

  /* Added shop kickout function - Ranger Feb 99 */
  if(cmd==MSG_MOBACT && keeper->specials.fighting) {
    shop_kickout(keeper,keeper->specials.fighting);
    GET_HIT(keeper)=GET_MAX_HIT(keeper);
    return FALSE;
  }

  /* Moved for loop into an if - Ranger Feb 99 */
  if(cmd==CMD_BUY || cmd==CMD_SELL || cmd==CMD_VALUE || cmd==CMD_LIST)
    for(shop_nr=0 ; real_mobile(shop_index[shop_nr].keeper) != keeper->nr; shop_nr++);

  if((cmd == CMD_BUY) && (CHAR_REAL_ROOM(ch) ==
         real_room(shop_index[shop_nr].in_room)))
    {
      shopping_buy(arg,ch,keeper,shop_nr);
      return(TRUE);
    }

  if((cmd == CMD_SELL ) && (CHAR_REAL_ROOM(ch) ==
         real_room(shop_index[shop_nr].in_room)))
    {
      shopping_sell(arg,ch,keeper,shop_nr);
      return(TRUE);
    }

  if((cmd == CMD_VALUE) && (CHAR_REAL_ROOM(ch) ==
         real_room(shop_index[shop_nr].in_room)))
    {
      shopping_value(arg,ch,keeper,shop_nr);
      return(TRUE);
    }

  if((cmd == CMD_LIST) && (CHAR_REAL_ROOM(ch) ==
         real_room(shop_index[shop_nr].in_room)))
    {
      shopping_list(arg,ch,keeper,shop_nr);
      return(TRUE);
    }

  if(cmd==CMD_KILL || cmd==CMD_HIT || cmd==CMD_PUMMEL ||   /*Offensives*/
     cmd==CMD_CIRCLE || cmd==CMD_PUNCH || cmd==CMD_SHOOT ||
     cmd==CMD_BACKSTAB || cmd==CMD_BASH || cmd==CMD_ASSAULT ||
     cmd==CMD_AMBUSH || cmd==CMD_KICK || cmd==CMD_SPIN) {
    one_argument(arg, argm);
    if (keeper == get_char_room(argm,CHAR_REAL_ROOM(ch))) {
      shopping_kill(ch,keeper);
      return(TRUE);
    }
  }

  if ((cmd==CMD_CAST) || (cmd==CMD_USE)) {
    act("$N tells you 'No magic here - kid!'.", FALSE, ch, 0, keeper, TO_CHAR);
    return TRUE;
  }
  /* Allow identify - Ranger July 99 */
  if(cmd==CMD_RECITE) {
    one_argument(arg, argm);
    if(is_abbrev(argm,"identify")) return FALSE;
    act("$N tells you 'No magic here - kid!'.", FALSE, ch, 0, keeper, TO_CHAR);
    return TRUE;
  }

  return(FALSE);
}

void read_shop(FILE *shop_f)
{
  char *buf;
  int temp= 0, count = 0;

  while (*(buf = fread_string(shop_f)) != '$')
  {
    if (*buf == '#') /* a new shop */
    {
      if (!number_of_shops) /* first shop */
      {
        CREATE(shop_index, struct shop_data, 1);
      }
      else if (!(shop_index = (struct shop_data*) realloc(shop_index, (number_of_shops + 1) * sizeof(struct shop_data))))
      {
        log_f("Error in read_shop\n");
        produce_core();
      }

      for (count = 0; count < MAX_PROD; count++)
      {
        fscanf(shop_f, "%d \n", &temp);
        shop_index[number_of_shops].producing[count] = temp;
      }

      fscanf(shop_f, "%f \n", &shop_index[number_of_shops].profit_buy);
      fscanf(shop_f, "%f \n", &shop_index[number_of_shops].profit_sell);

      for (count = 0; count < MAX_TRADE; count++)
      {
        fscanf(shop_f, "%d \n", &temp);
        shop_index[number_of_shops].type[count] = (byte)temp;
      }

      shop_index[number_of_shops].no_such_item1 = fread_string(shop_f);
      shop_index[number_of_shops].no_such_item2 = fread_string(shop_f);
      shop_index[number_of_shops].do_not_buy    = fread_string(shop_f);
      shop_index[number_of_shops].missing_cash1 = fread_string(shop_f);
      shop_index[number_of_shops].missing_cash2 = fread_string(shop_f);
      shop_index[number_of_shops].message_buy   = fread_string(shop_f);
      shop_index[number_of_shops].message_sell  = fread_string(shop_f);
      fscanf(shop_f, "%d \n", &shop_index[number_of_shops].temper1);
      fscanf(shop_f, "%d \n", &shop_index[number_of_shops].temper2);
      fscanf(shop_f, "%d \n", &shop_index[number_of_shops].keeper);
      fscanf(shop_f, "%d \n", &shop_index[number_of_shops].with_who);
      fscanf(shop_f, "%d \n", &shop_index[number_of_shops].in_room);
      fscanf(shop_f, "%d \n", &shop_index[number_of_shops].open1);
      fscanf(shop_f, "%d \n", &shop_index[number_of_shops].close1);
      fscanf(shop_f, "%d \n", &shop_index[number_of_shops].open2);
      fscanf(shop_f, "%d \n", &shop_index[number_of_shops].close2);

      /* Was this shop previously loaded? If so, save info to old spot and then delete it*/
      for (temp = 0; temp < number_of_shops; temp++)
      {
        if (shop_index[temp].keeper == shop_index[number_of_shops].keeper)
        {
          for (count = 0; count < MAX_PROD; count++)
          {
            shop_index[temp].producing[count] = shop_index[number_of_shops].producing[count];
          }

          shop_index[temp].profit_buy         = shop_index[number_of_shops].profit_buy;
          shop_index[temp].profit_sell        = shop_index[number_of_shops].profit_sell;

          for (count = 0; count < MAX_TRADE; count++)
          {
            shop_index[temp].type[count]      = shop_index[number_of_shops].type[count];
          }

          shop_index[temp].no_such_item1      = str_dup(shop_index[number_of_shops].no_such_item1);
          shop_index[temp].no_such_item2      = str_dup(shop_index[number_of_shops].no_such_item2);
          shop_index[temp].do_not_buy         = str_dup(shop_index[number_of_shops].do_not_buy);
          shop_index[temp].missing_cash1      = str_dup(shop_index[number_of_shops].missing_cash1);
          shop_index[temp].missing_cash2      = str_dup(shop_index[number_of_shops].missing_cash2);
          shop_index[temp].message_buy        = str_dup(shop_index[number_of_shops].message_buy);
          shop_index[temp].message_sell       = str_dup(shop_index[number_of_shops].message_sell);
          shop_index[temp].temper1            = shop_index[number_of_shops].temper1;
          shop_index[temp].temper2            = shop_index[number_of_shops].temper2;
          shop_index[temp].with_who           = shop_index[number_of_shops].with_who;
          shop_index[temp].in_room            = shop_index[number_of_shops].in_room;
          shop_index[temp].open1              = shop_index[number_of_shops].open1;
          shop_index[temp].close1             = shop_index[number_of_shops].close1;
          shop_index[temp].open2              = shop_index[number_of_shops].open2;
          shop_index[temp].close2             = shop_index[number_of_shops].close2;

          //memset(&shop_index[number_of_shops], 0, sizeof(struct shop_data));
          //free(&shop_index[number_of_shops]);
          if (!(shop_index = (struct shop_data*) realloc(shop_index, (number_of_shops - 1) * sizeof(struct shop_data))))
          {
            log_f("Error in read_shop\n");
            produce_core();
          }
          number_of_shops--;
          break;
        }
      }

      number_of_shops++;
    }
  }
}

void boot_the_shops() { /* loaded in spec_assign.c (assign_mobiles) */
  FILE *sf;

  if (!(sf = fopen(SHOP_FILE, "r")))
    {
      log_f("No main shop file found\n");
      return;
    }
  read_shop(sf);
  fclose(sf);

}

int is_shop(CHAR *mob) {
  for (int i = 0; i < number_of_shops; i++) {
    if (shop_index[i].keeper == V_MOB(mob)) return TRUE;
  }

  return FALSE;
}

bool is_shop_v(int vnum) {
  for (int i = 0; i < number_of_shops; i++) {
    if (shop_index[i].keeper == vnum) return TRUE;
  }

  return FALSE;
}

void assign_the_shopkeepers() /* loaded in spec_assign.c (assign_mobiles) */
{
  int temp1;

  for(temp1=0 ; temp1<number_of_shops ; temp1++)
    assign_mob(shop_index[temp1].keeper, shop_keeper);

}
