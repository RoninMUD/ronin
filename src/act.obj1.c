/* ************************************************************************
*  file: act.obj1.c , Implementation of commands.         Part of DIKUMUD *
*  Usage : Commands mainly moving around objects.                         *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

/*
$Author: ronin $
$Date: 2005/04/27 17:12:28 $
$Header: /home/ronin/cvs/ronin/act.obj1.c,v 2.1 2005/04/27 17:12:28 ronin Exp $
$Id: act.obj1.c,v 2.1 2005/04/27 17:12:28 ronin Exp $
$Name:  $
$Log: act.obj1.c,v $
Revision 2.1  2005/04/27 17:12:28  ronin
Addition of obj vnum to vault log.

Revision 2.0.0.1  2004/02/05 16:08:33  ronin
Reinitialization of cvs archives

Revision 1.2  2002/03/31 07:42:14  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

#include "structs.h"
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

/* extern variables */

extern struct str_app_type str_app[];
extern struct descriptor_data *descriptor_list;
extern struct room_data *world;
extern int CHAOSMODE;

/* extern functions */

struct obj_data *create_money( int amount );
void do_put(struct char_data *ch, char *argument, int cmd);
struct obj_data *get_obj_in_equip_vis( struct char_data *ch, char *name,
                                       struct obj_data *equipment[], int *i);

void number_argument_interpreter(char *argument, int *number,
                         char *first_arg, char *second_arg )
{
  bool number_ok  = TRUE;
  int look_at, begin;

  begin = 0;

  *number = 1;

  do {
    /* Find first non blank */
    for ( ;*(argument + begin ) == ' ' ; begin++);

    /* Find length of first word */
    /* Make all letters lower case, AND copy them to first_arg */
    for ( look_at = 0; *(argument+begin+look_at) > ' ' ; look_at++) {
      *(first_arg + look_at) = LOWER(*(argument + begin + look_at));
      if(!isdigit(*(argument + begin +look_at)) || look_at > 7)
        number_ok = FALSE;
    }

    if(look_at == 0 && number_ok)
      number_ok = FALSE;

    *(first_arg + look_at)='\0';
    begin += look_at;

    if(number_ok)
      *number = atoi(first_arg);

  } while( fill_word(first_arg) || number_ok);

  do {
    /* Find first non blank */
    for ( ;*(argument + begin ) == ' ' ; begin++);

    /* Find length of first word */
    for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

      /* Make all letters lower case,
       AND copy them to second_arg */
      *(second_arg + look_at) =
      LOWER(*(argument + begin + look_at));

    *(second_arg + look_at)='\0';
    begin += look_at;

  }
  while( fill_word(second_arg));
}

int is_all_dot(char *arg, char *allbuf)
{
  allbuf[0] = '\0';
  if ((str_cmp(arg, "all") != 0) &&
      (sscanf(arg, "all.%s", allbuf) != 0)) {
    strcpy(arg, "all");
    return(TRUE);
  }
  return(FALSE);
}

void v_log_f(char *str);
/* procedures related to get */
void get(struct char_data *ch, struct obj_data *obj_object,
       struct obj_data *sub_object,bool showit)
{
  char buffer[MAX_STRING_LENGTH];
  char buf[250];

  if (signal_object(obj_object, ch, MSG_GET, ""))
     return;

  if (sub_object) {
    /* Log loot of PC corpse - Ranger Feb 97 */
    /* Check for PC corpse or Statue */
    if((GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) &&
       (sub_object->obj_flags.value[3]) &&
       (sub_object->obj_flags.cost==PC_CORPSE ||
        sub_object->obj_flags.cost==PC_STATUE) ) {
      if(!isname(GET_NAME(ch),OBJ_SHORT(sub_object))) {
        sprintf(buffer,"WIZ: %s get %s from %s",GET_NAME(ch),
                OBJ_SHORT(obj_object),OBJ_SHORT(sub_object));
        log_s(buffer);
      }
    }
    /* get log from vaults - Ranger March 00 */
    if(!strcmp(OBJ_SHORT(sub_object),"storage vault")) {
      sprintf(buffer,"%s gets %s (#%d) from %s.",
              GET_NAME(ch),OBJ_SHORT(obj_object),V_OBJ(obj_object),OBJ_NAME(sub_object));
      v_log_f(buffer);
    }
    obj_from_obj(obj_object);
    obj_to_char(obj_object, ch);
    if (sub_object->carried_by == ch) {
      act("You get $p from $P.", 0, ch, obj_object, sub_object, TO_CHAR);
      if(showit)
        act("$n gets $p from $s $P.",1, ch, obj_object, sub_object, TO_ROOM);
    } else {
      act("You get $p from $P.", 0, ch, obj_object, sub_object, TO_CHAR);
      if(showit)
        act("$n gets $p from $P.", 1, ch, obj_object, sub_object, TO_ROOM);
    }
  } else {
    /* Log corpse getting picked up - Ranger June 97 */
    if((GET_ITEM_TYPE(obj_object) == ITEM_CONTAINER) &&
       (obj_object->obj_flags.value[3]) &&
       (obj_object->obj_flags.cost==PC_CORPSE ||
        obj_object->obj_flags.cost==PC_STATUE) ) {
      sprintf(buffer,"WIZ: %s gets %s.",GET_NAME(ch),OBJ_SHORT(obj_object));
      log_s(buffer);
    }
    /* Log getting eq from the morgue - Ranger June 97 */
    if(CHAR_VIRTUAL_ROOM(ch)==3088) {
      sprintf(buffer,"WIZ: %s gets %s in the morgue.",GET_NAME(ch),OBJ_SHORT(obj_object));
      log_s(buffer);
    }
    obj_from_room(obj_object);
    obj_to_char(obj_object, ch);
    act("You get $p.", 0, ch, obj_object, 0, TO_CHAR);
    if(showit)
      act("$n gets $p.", 1, ch, obj_object, 0, TO_ROOM);
  }

  if (GET_LEVEL(ch) > LEVEL_IMM) {
    sprintf (buf, "WIZINFO: %s gets %s in %d", GET_NAME(ch),
           OBJ_SHORT(obj_object), CHAR_VIRTUAL_ROOM(ch));
    wizlog(buf, GET_LEVEL(ch)+1, 5);
    log_s(buf);
  }

  if((obj_object->obj_flags.type_flag == ITEM_MONEY) &&
     (obj_object->obj_flags.value[0]>=1)) {
    obj_from_char(obj_object);
    sprintf(buffer,"There were %d coins.\n\r",
          obj_object->obj_flags.value[0]);
    send_to_char(buffer,ch);
    GET_GOLD(ch) += obj_object->obj_flags.value[0];
    if(obj_object->obj_flags.value[0] > 1000000) { /* changed min to 1mil 031803 Liner */
      sprintf(buffer,"WIZ: Large transaction: %s gets %d coins",
            GET_NAME(ch),obj_object->obj_flags.value[0]);
      log_s(buffer);
    }
    extract_obj(obj_object);
  }
  save_char(ch,NOWHERE);
}

int get_obj_from_object(struct char_data *ch, char *name,
                  struct obj_data *sub_object)
{
  struct obj_data *obj_object;
  int total=0;

  if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER)
    {
      if (IS_SET(sub_object->obj_flags.value[1], CONT_NOREMOVE))
      {
        printf_to_char(ch,"You can't seem to get anything out of the %s.\n\r", fname(OBJ_NAME(sub_object)));
        return(0);
      } /* if */
      if (IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED))
      {
        printf_to_char(ch,"The %s is closed.\n\r", fname(OBJ_NAME(sub_object)));
        return(0);
      } /* if */
      obj_object = get_obj_in_list_vis(ch, name, sub_object->contains);
      if (obj_object)
      {
        if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch)))
          {
            if ((CAN_CARRY_OBJ(ch,obj_object)) || (sub_object->carried_by==ch))
            {
              if (CAN_TAKE(ch,obj_object))
                {
                  get( ch, obj_object, sub_object, TRUE);
                  total = 1;
                }
              else
                {
                  printf_to_char(ch, "You can't take %s!\n\r", fname(OBJ_NAME(obj_object)));
                }
            } else {
            printf_to_char(ch,"%s : You can't carry that much weight.\n\r",
                  fname(OBJ_NAME(obj_object)));
            }
        } else {
          printf_to_char(ch,"%s : You can't carry that many items.\n\r",
                fname(OBJ_NAME(obj_object)));
        }
      } else {
        if(isname("vault",OBJ_NAME(sub_object)))
          printf_to_char(ch,"The vault does not contain the %s.\n\r",name);
        else
          printf_to_char(ch,"The %s does not contain the %s.\n\r",
                         fname(OBJ_NAME(sub_object)), name);
      }
  } else {
    printf_to_char(ch,"The %s is not a container.\n\r",
          fname(OBJ_NAME(sub_object)));
  }
  return(total);
}

int get_all_from_object(struct char_data *ch, char *name, bool alldot,
                  struct obj_data *sub_object)
{
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *remember=NULL;
  struct obj_data *obj_object, *next_obj;
  bool fail=FALSE;
  int total=0;

  assert(ch && sub_object);

  if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
    if (IS_SET(sub_object->obj_flags.value[1], CONT_NOREMOVE))
    {
      printf_to_char(ch,"You can't seem to get anything out of the %s.\n\r", fname(OBJ_NAME(sub_object)));
      return(0);
    } /* if */
    if (IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)) {
      sprintf(buffer,"The %s is closed.\n\r", fname(OBJ_NAME(sub_object)));
      send_to_char(buffer, ch);
      return(0);
    } /* if */
    for(obj_object = sub_object->contains;
      obj_object;
      obj_object = next_obj) {
      next_obj = obj_object->next_content;

      /* IF all.obj, only get those named "obj" */
      if (alldot && !isname( name, OBJ_NAME(obj_object)) ) {
      continue;
      } /* if */

      if (CAN_SEE_OBJ( ch, obj_object)) {
      if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
        if ((CAN_CARRY_OBJ(ch,obj_object)) || (sub_object->carried_by == ch)) {
          if (CAN_TAKE(ch,obj_object)) {
              if(obj_object->obj_flags.type_flag != ITEM_MONEY)
              remember = obj_object;
              else
                remember = NULL;
            get( ch, obj_object, sub_object, FALSE);
            total++;
          } else {
            sprintf(buffer, "You can't take %s!\n\r", fname(OBJ_NAME( obj_object)));
            send_to_char( buffer, ch);
            fail = TRUE;
          }
        } else {
          sprintf(buffer,"%s : You can't carry that much weight.\n\r",
                fname(OBJ_NAME(obj_object)));
          send_to_char(buffer, ch);
          fail = TRUE;
        }
      } else {
        sprintf(buffer,"%s : You can't carry that many items.\n\r",
              fname(OBJ_NAME(obj_object)));
        send_to_char(buffer, ch);
        fail = TRUE;
      }
      }
    }
    if (!total && !fail) {
      if(alldot){
      sprintf(buffer, "You don't see any %s in the %s.\n\r",
            name,
            fname(OBJ_NAME(sub_object)));
      send_to_char(buffer, ch);
      } else {
      sprintf(buffer,"You do not see anything in the %s.\n\r",
            fname(OBJ_NAME(sub_object)));
      send_to_char(buffer, ch);
      }
    } else if(total == 1)
      if(remember)
        act("$n gets $p.", TRUE, ch, remember, 0, TO_ROOM);
      else
        act("$n gets some coins.", TRUE, ch, NULL, 0, TO_ROOM);
    else if(total > 1 && total < 6)
      act("$n gets some stuff from $p.",TRUE,ch,sub_object,0,TO_ROOM);
    else if(total > 5)
      act("$n gets a bunch of stuff from $p.",
        TRUE,ch,sub_object,0,TO_ROOM);
    return(total);
  } else {
    sprintf(buffer,"The %s is not a container.\n\r",
          fname(OBJ_NAME(sub_object)));
    send_to_char(buffer, ch);
    return(0);
  }
}

int get_number_from_object(struct char_data *ch, char *name, int number,
                           struct obj_data *sub_object)
{
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *remember=NULL;
  struct obj_data *obj_object, *next_obj;
  bool fail=FALSE;
  int total=0;

  assert(ch && sub_object);

  if(!number)
    return(0);

  if (GET_ITEM_TYPE(sub_object) == ITEM_CONTAINER) {
    if (IS_SET(sub_object->obj_flags.value[1], CONT_NOREMOVE))
    {
      printf_to_char(ch,"You can't seem to get anything out of the %s.\n\r", fname(OBJ_NAME(sub_object)));
      return(0);
    } /* if */
    if (IS_SET(sub_object->obj_flags.value[1], CONT_CLOSED)) {
      sprintf(buffer,"The %s is closed.\n\r", fname(OBJ_NAME(sub_object)));
      send_to_char(buffer, ch);
      return(0);
    } /* if */
    for(obj_object = sub_object->contains, total = 0;
      obj_object && total < number;
      obj_object = next_obj) {
      next_obj = obj_object->next_content;

      if (!isname( name, OBJ_NAME(obj_object)) ) {
      continue;
      } /* if */

      if (CAN_SEE_OBJ( ch, obj_object)) {
      if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
        if ((CAN_CARRY_OBJ(ch,obj_object)) || (sub_object->carried_by == ch)) {
          if (CAN_TAKE(ch,obj_object)) {
            get( ch, obj_object, sub_object, FALSE);
            total++;
            remember = obj_object;
          } else {
            sprintf(buffer, "You can't take %s!\n\r", fname(OBJ_NAME(obj_object)));
            send_to_char( buffer, ch);
            fail = TRUE;
          }
        } else {
          sprintf(buffer,"%s : You can't carry that much weight.\n\r",
                fname(OBJ_NAME(obj_object)));
          send_to_char(buffer, ch);
          fail = TRUE;
        }
      } else {
        sprintf(buffer,"%s : You can't carry that many items.\n\r",
              fname(OBJ_NAME(obj_object)));
        send_to_char(buffer, ch);
        fail = TRUE;
      }
      }
    }
    if (!total && !fail) {
      sprintf(buffer, "You don't see any %s in the %s.\n\r",
            name,
            fname(OBJ_NAME(sub_object)));
      send_to_char(buffer, ch);
    } else if(total == 1)
      act("$n gets $p.", TRUE, ch, remember, 0, TO_ROOM);
    else if(total > 1 && total < 6)
      act("$n gets some stuff from $p.",TRUE,ch,sub_object,0,TO_ROOM);
    else if(total > 5)
      act("$n gets a bunch of stuff from $p.",
        TRUE,ch,sub_object,0,TO_ROOM);
    return(total);
  } else {
    sprintf(buffer,"The %s is not a container.\n\r",
          fname(OBJ_NAME(sub_object)));
    send_to_char(buffer, ch);
    return(0);
  }
}

void do_get(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *sub_object=0;
  struct obj_data *obj_object;
  struct obj_data *next_obj;
  bool fail = FALSE;
  int type  = 0;
  int total = 0;
  int found2 = 0;
  int i;
  int number = 1;
  bool alldot1 = FALSE;
  bool alldot2 = FALSE;
  char allbuf1[MAX_STRING_LENGTH];
  char allbuf2[MAX_STRING_LENGTH];

  number_argument_interpreter(argument, &number, arg1, arg2);

  if( number <= 0) {
    send_to_char("Sorry but you cannot use negative amount number here.\n\r", ch);
    return;
  }

  /* get type */
  if (!*arg1) {
    type = 0;
  } else if (*arg1 && !*arg2) {
    alldot1 = is_all_dot(arg1, allbuf1);
    if (!str_cmp(arg1,"all")) {
      if(V_ROOM(ch)==3084 && GET_LEVEL(ch) < LEVEL_DEI) {
        send_to_char("Now don't be greedy.\n\r",ch);
        return;
      }
      type = 1;
    } else {
      type = 2;
    }
  } else if (*arg1 && *arg2) {
    alldot1 = is_all_dot(arg1, allbuf1);
    alldot2 = is_all_dot(arg2, allbuf2);
    if (!str_cmp(arg1,"all")) {
      if (!str_cmp(arg2,"all")) {
      type = 3;
      } else {
      type = 4;
      }
    } else {
      if (!str_cmp(arg2,"all")) {
      type = 5;
      } else {
      type = 6;
      }
    }
  }

  if (!str_cmp(arg1,"all") && number != 1){
    send_to_char("Sorry but you cannot use amount number with all or all.something\n\r", ch);
    return;
  }

  switch (type) {
    /* get */
  case 0:{
    send_to_char("Get what?\n\r", ch);
  } break;
    /* get all */
  case 1:{
    for(obj_object = world[CHAR_REAL_ROOM(ch)].contents;
      obj_object;
      obj_object = next_obj) {
      next_obj = obj_object->next_content;

      /* IF all.obj, only get those named "obj" */
      if (alldot1 && !isname(allbuf1,OBJ_NAME(obj_object))) {
      continue;
      } /* if */

      if (CAN_SEE_OBJ(ch,obj_object)) {
      if ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)) {
        if ((CAN_CARRY_OBJ(ch,obj_object)) || (GET_LEVEL(ch)>=LEVEL_IMM))
          {
            if (CAN_TAKE(ch,obj_object)) {
            get(ch, obj_object, sub_object, TRUE);
            total++;
            } else {
            sprintf(buffer, "You can't take %s\n\r", fname(OBJ_NAME(obj_object)));
            send_to_char( buffer, ch);
            fail = TRUE;
            }
        } else {
          sprintf(buffer,"%s : You can't carry that much weight.\n\r",
                fname(OBJ_NAME(obj_object)));
          send_to_char(buffer, ch);
          fail = TRUE;
        }
      } else {
        sprintf(buffer,"%s : You can't carry that many items.\n\r",
              fname(OBJ_NAME(obj_object)));
        send_to_char(buffer, ch);
        fail = TRUE;
      }
      }
    }
    if (total) {
      sprintf(buffer,"You got %d item(s).\n\r",total);
      send_to_char(buffer, ch);
    } else if (!fail) {
      send_to_char("You see nothing here.\n\r", ch);
    }
  } break;
    /* get ??? */
  case 2:{
    if(number == 1) {
      obj_object = get_obj_in_list_vis(ch, arg1,
                                     world[CHAR_REAL_ROOM(ch)].contents);
      if (obj_object) {
        if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
        if ((CAN_CARRY_OBJ(ch,obj_object)))
          {
            if (CAN_TAKE(ch,obj_object)) {
            get( ch, obj_object, sub_object, TRUE);
            total = 1;
            } else {
            send_to_char("You can't take that\n\r", ch);
            fail = TRUE;
            }
        } else {
          sprintf(buffer,"%s : You can't carry that much weight.\n\r",
                fname(OBJ_NAME(obj_object)));
          send_to_char(buffer, ch);
          fail = TRUE;
        }
        } else {
        sprintf(buffer,"%s : You can't carry that many items.\n\r",
              fname(OBJ_NAME(obj_object)));
          send_to_char(buffer, ch);
        fail = TRUE;
        }
      } else {
        sprintf(buffer,"You do not see a %s here.\n\r", arg1);
        send_to_char(buffer, ch);
      }
    } else {
      for(obj_object = world[CHAR_REAL_ROOM(ch)].contents, total = 0;
        obj_object && total < number;
        obj_object = next_obj) {
      next_obj = obj_object->next_content;
        if (isname( arg1, OBJ_NAME(obj_object)))
          if (CAN_SEE_OBJ(ch, obj_object)) {
            if ((IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch))) {
            if ((CAN_CARRY_OBJ(ch,obj_object)))
            {
              if (CAN_TAKE(ch,obj_object)) {
                get( ch, obj_object, sub_object, TRUE);
                total++;
              } else {
                send_to_char("You can't take that\n\r", ch);
                fail = TRUE;
              }
              } else {
              sprintf(buffer,"%s : You can't carry that much weight.\n\r",
                  fname(OBJ_NAME(obj_object)));
              send_to_char(buffer, ch);
              fail = TRUE;
            }
            } else {
            sprintf(buffer,"%s : You can't carry that many items.\n\r",
                  fname(OBJ_NAME(obj_object)));
              send_to_char(buffer, ch);
            fail = TRUE;
            }
          }
      }

      if(!fail && !total) {
        sprintf(buffer, "You don't see or have any %s.\n\r", arg1);
        send_to_char(buffer, ch);
      } else {
        sprintf(buffer, "You got %d(%d) %s(s).\n\r",
                total,  number, arg1);
        send_to_char(buffer, ch);
      }
    }
  } break;
    /* get all all */
  case 3:{
    for(sub_object = ch->carrying; sub_object; sub_object = next_obj){
      next_obj = sub_object->next_content;
      if (alldot2 && !isname(allbuf2, OBJ_NAME(sub_object))) {
      continue;
      } /* if */
      found2++;
      total += get_all_from_object( ch, allbuf1, alldot1, sub_object);
    }
    for(sub_object = world[CHAR_REAL_ROOM(ch)].contents;
      sub_object;
      sub_object = next_obj) {
      next_obj = sub_object->next_content;
      if (alldot2 && !isname(allbuf2, OBJ_NAME(sub_object))) {
      continue;
      } /* if */
      found2++;
      total += get_all_from_object( ch, allbuf1, alldot1,  sub_object);
    }
    for( i = 0; i < MAX_WEAR; i++)
      if(ch->equipment[i]) {
      if (alldot2 && !isname(allbuf2, OBJ_NAME(ch->equipment[i]))) {
        continue;
      } /* if */
      found2++;
      total += get_all_from_object( ch, allbuf1, alldot1, ch->equipment[i]);
      }

    if (total) {
      sprintf( buffer, "You got %d item(s).\n\r",total);
      send_to_char(buffer, ch);
    } else if(!found2) {
      if(alldot2){
      sprintf(buffer,"You do not see or have any %s.\n\r", allbuf2);
      send_to_char(buffer, ch);
      } else {
      send_to_char("You do not see or have any object.\n\r", ch);
      }
    }
  } break;
    /* get all ??? */
  case 4:{
    if(!(sub_object = get_obj_in_list_vis(ch, arg2, ch->carrying)))
      if(!(sub_object = get_obj_in_list_vis(ch, arg2, world[CHAR_REAL_ROOM(ch)].contents)))
        sub_object = get_obj_in_equip_vis( ch, arg2, ch->equipment, &i);

    if (sub_object) {
      total = get_all_from_object( ch, allbuf1, alldot1, sub_object);
    } else {
      sprintf(buffer,"You do not see or have the %s.\n\r", arg2);
      send_to_char(buffer, ch);
    }
  } break;
  case 5:{
    for(sub_object = ch->carrying; sub_object; sub_object = next_obj){
      next_obj = sub_object->next_content;
      if (alldot2 && !isname(allbuf2, OBJ_NAME(sub_object))) {
      continue;
      } /* if */
      found2++;
      if(number == 1)
        total += get_obj_from_object( ch, arg1, sub_object);
      else
        total += get_number_from_object( ch, arg1, number, sub_object);
    }
    for(sub_object = world[CHAR_REAL_ROOM(ch)].contents;
      sub_object;
      sub_object = next_obj) {
      next_obj = sub_object->next_content;
      if (alldot2 && !isname(allbuf2, OBJ_NAME(sub_object))) {
      continue;
      } /* if */
      found2++;
      if(number - total == 1)
        total += get_obj_from_object( ch, arg1, sub_object);
      else
        total += get_number_from_object( ch, arg1, number - total, sub_object);
    }
    for( i = 0; i < MAX_WEAR; i++)
      if(ch->equipment[i]) {
      if (alldot2 && !isname(allbuf2, OBJ_NAME(ch->equipment[i]))) {
        continue;
      } /* if */
      found2++;
        if(number - total == 1)
        total += get_obj_from_object( ch, arg1, ch->equipment[i]);
        else
          total += get_number_from_object( ch, arg1, number - total, ch->equipment[i]);
      }

    if (total) {
      sprintf( buffer, "You got %d(%d) %s(s).\n\r", total, number, arg1);
      send_to_char(buffer, ch);
    } else if(!found2) {
      if(alldot2) {
      sprintf(buffer,"You do not see or have any %s.\n\r", allbuf2);
      send_to_char(buffer, ch);
      } else {
      send_to_char("You do not see or have any object.\n\r", ch);
      }
    }
  } break;
  case 6:{
    if(!(sub_object = get_obj_in_list_vis(ch, arg2, ch->carrying)))
      if(!(sub_object = get_obj_in_list_vis(ch, arg2, world[CHAR_REAL_ROOM(ch)].contents)))
        sub_object = get_obj_in_equip_vis( ch, arg2, ch->equipment, &i);

    if (sub_object) {
      if(number == 1)
        total = get_obj_from_object( ch, arg1, sub_object);
      else {
        total = get_number_from_object( ch, arg1, number, sub_object);
        if (total) {
          sprintf( buffer, "You got %d(%d) %s(s).\n\r", total, number, arg1);
          send_to_char(buffer, ch);
        } else {
          sprintf(buffer, "You don't see or have any %s in %s.\n\r", arg1, arg2);
          send_to_char(buffer, ch);
        }
      }
    } else {
      sprintf(buffer,"You do not see or have the %s.\n\r", arg2);
      send_to_char(buffer, ch);
    }
  } break;
  }

  if(total){
    save_char(ch,NOWHERE);
  }
}

int drop_coins(struct char_data *ch, int amount)
{
  char buffer[MAX_STRING_LENGTH];
  struct obj_data *tmp_object;

  if(GET_GOLD(ch) < amount) {
    send_to_char("You haven't got that many coins!\n\r",ch);
    return(0);
  }
  sprintf(buffer, "You drop %d coin(s).\n\r", amount);
  send_to_char(buffer, ch);
  if(amount <= 0)
    {
      sprintf(buffer, "%s tried to drop negative coins.", GET_NAME(ch));
      log_s(buffer);
      return(0);
    }
  act("$n drops some gold.", FALSE, ch, 0, 0, TO_ROOM);
  GET_GOLD(ch) -= amount;
  if(amount > 1000000) { /* changed min to 1 mil 031803 Liner */
    sprintf(buffer,"WIZ: Large transaction: %s drops %d coins",
            GET_NAME(ch), amount);
    log_s(buffer);
  }

  if(GET_LEVEL(ch) >= LEVEL_IMM) {
    sprintf(buffer, "WIZINFO: (%s) dropped %d coins in room #%d",
          GET_NAME(ch),
          amount,
          world[CHAR_REAL_ROOM(ch)].number);
    wizlog(buffer, GET_LEVEL(ch)+1, 5);
    log_s(buffer);
  }


  if(0 != (tmp_object = get_obj_in_list("gold", world[CHAR_REAL_ROOM(ch)].contents))){
    amount += tmp_object->obj_flags.value[0];
    extract_obj(tmp_object);
    send_to_char("You add your gold to the pile.\n\r",ch);
  }
  tmp_object = create_money(amount);
  obj_to_room(tmp_object, CHAR_REAL_ROOM(ch));

  return(amount);
}

int drop_object(CHAR *ch, OBJ *obj)
{
  char buf[MSL] = "";

  if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_NODROP))
  {
    send_to_char("You can't drop it, it must be CURSED!\n\r", ch);
    return 0;
  }
  else if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_CLONE))
  {
    act("Your cloned $p falls to pieces as you drop it.", FALSE, ch, obj, NULL, TO_CHAR);
    act("$n's cloned $p falls to pieces as it is dropped.", FALSE, ch, obj, NULL, TO_ROOM);

    obj_from_char(obj);
    extract_obj(obj);

    return 1;
  }

  if (signal_object(obj, ch, MSG_OBJ_DROPPED, "")) return 1;

  if (IS_IMMORTAL(ch))
  {
    sprintf(buf, "WIZINFO: %s drops %s (#%d) in room (#%d)",
      GET_NAME(ch), OBJ_SHORT(obj), V_OBJ(obj), CHAR_VIRTUAL_ROOM(ch));
    wizlog(buf, GET_LEVEL(ch) + 1, 5);
    log_f("%s", buf);
  }

  act("You drop the $o.", FALSE, ch, obj, NULL, TO_CHAR);
  act("$n drops $p.", TRUE, ch, obj, NULL, TO_ROOM);

  obj_from_char(obj);
  obj_to_room(obj, CHAR_REAL_ROOM(ch));

  return 1;
}

void do_drop(struct char_data *ch, char *argument, int cmd)
{
  bool type = 0;
  bool found = FALSE;
  bool alldot = FALSE;
  char allbuf[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char buffer[MAX_STRING_LENGTH];
  int number = 0;
  int total = 0;
  struct obj_data *tmp_object;
  struct obj_data *next_obj;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  if(!*arg1) {
    type = 0;
  } else if(is_number(arg1)) {
    if(strlen(arg1) > 7){
      send_to_char("Number field too big.\n\r",ch);
      return;
    }
    if((number = atoi(arg1)) < 0) {
      send_to_char("Sorry, but you can't do that!\n\r", ch);
      return;
    }
    if(*arg2)
      type = 1;
    else
      type = 0;
  } else {
    alldot = is_all_dot(arg1, allbuf);
    if (!str_cmp(arg1, "all")){
      if (*arg2)
        type = 2;
      else
        type = 3;
    } else {
      type = 4;
    }
  }

  switch(type) {
  case 0:{
    send_to_char("Drop what?\n\r", ch);
  }  break;
  case 1: { /* drop # <item> */
    if (!str_cmp("coins", arg2) || !str_cmp("coin", arg2)) {
      total = drop_coins( ch, number);
    } else {
      for(tmp_object = ch->carrying, total = 0;
        tmp_object && total < number;
        tmp_object = next_obj) {
      next_obj = tmp_object->next_content;
        if (isname( arg2, OBJ_NAME(tmp_object)))
          if (CAN_SEE_OBJ(ch, tmp_object)) {
            total += drop_object( ch, tmp_object);
            found = TRUE;
          }
      }
      if (!found && number) {
        sprintf(buffer, "You don't see or have any %s.\n\r", arg2);
        send_to_char(buffer, ch);
      } else {
        sprintf(buffer, "You drop %d item(s).\n\r", total);
        send_to_char(buffer, ch);
      }
    }
  }  break;
  case 2: { /* drop all <item> */
    if (!str_cmp("coins", arg2) || !str_cmp("coin", arg2)) {
      total = drop_coins( ch, GET_GOLD(ch));
    } else {
      for(tmp_object = ch->carrying;
          tmp_object;
          tmp_object = next_obj) {
        next_obj = tmp_object->next_content;
        if (isname( arg2, OBJ_NAME(tmp_object)))
          if (CAN_SEE_OBJ(ch, tmp_object)) {
            total += drop_object( ch, tmp_object);
            found = TRUE;
          }
      }
      if (!found) {
        sprintf(buffer, "You don't see or have any %s.\n\r", arg2);
        send_to_char(buffer, ch);
      } else {
        sprintf(buffer, "You drop %d item(s).\n\r", total);
        send_to_char(buffer, ch);
      }
    }
  } break;
  case 3: { /* drop all.<item> and drop all */
    if (alldot && (!str_cmp("coins", allbuf) || !str_cmp("coin", allbuf))) {
      total = drop_coins( ch, GET_GOLD(ch));
    } else {
      for(tmp_object = ch->carrying;
          tmp_object;
          tmp_object = next_obj) {
        next_obj = tmp_object->next_content;
        if (alldot && !isname(allbuf, OBJ_NAME(tmp_object))) {
          continue;
        } /* if */
        total += drop_object( ch, tmp_object);
        found = TRUE;
      }
      if (!found && alldot) {
        sprintf(buffer, "You don't see or have any %s.\n\r", allbuf);
        send_to_char(buffer, ch);
      } else if (!found) {
        send_to_char("You don't have anything to drop.\n\r", ch);
      } else {
        sprintf(buffer, "You drop %d item(s).\n\r", total);
        send_to_char(buffer, ch);
      }
    }
  } break;
  case 4: { /* drop <item> */
    tmp_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
    if (tmp_object) {
      total = drop_object( ch, tmp_object);
    } else {
      send_to_char("You do not have that item.\n\r", ch);
    }
  } break;
  default: {
    log_f("BUG: Wrong type in do_drop!");
    produce_core();
  } break;
  }
  if(total) {
    save_char(ch,NOWHERE);
  }
}

int put(CHAR *ch, OBJ *obj, OBJ *sub_obj, bool show)
{
  char buf[MSL] = "";

  if (obj == sub_obj)
  {
    if (show)
    {
      send_to_char("You attempt to fold it into itself, but fail.\n\r", ch);
    }

    return FALSE;
  }

  if (IS_SET(OBJ_EXTRA_FLAGS2(obj), ITEM_NO_PUT))
  {
    if (show)
    {
      send_to_char("That item can't be put in another.\n\r", ch);
    }

    return FALSE;
  }

  if (signal_object(obj, ch, MSG_OBJ_PUT, OBJ_NAME(sub_obj))) return TRUE;

  if (((GETOBJ_WEIGHT(sub_obj) + GETOBJ_WEIGHT(obj)) >= OBJ_VALUE0(sub_obj)) ||
      IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_CLONE))
  {
    if (show)
    {
      send_to_char("It won't fit.\n\r", ch);
    }

    return FALSE;
  }

  if (IS_IMMORTAL(ch))
  {
    sprintf (buf, "WIZINFO: %s put %s in %s",
      GET_NAME(ch), OBJ_SHORT(obj), OBJ_SHORT(sub_obj));
    wizlog(buf, GET_LEVEL(ch) + 1, 5);
    log_f("%s", buf);
  }

  if (show)
  {
    send_to_char("Ok.\n\r", ch);
    act("$n puts $p in $P.",TRUE, ch, obj, sub_obj, TO_ROOM);
  }

  if (obj->carried_by)
  {
    obj_from_char(obj);
  }
  else
  {
    obj_from_room(obj);
  }

  obj_to_obj(obj, sub_obj);

  return TRUE;
}

int put_all_to(struct char_data *ch, char *allbuf, bool alldot,
             struct obj_data *to_obj)
{
  struct obj_data *tmp_object;
  struct obj_data *next_object;
  char buf[MAX_STRING_LENGTH];
  int total = 0;

  for (tmp_object = ch->carrying; tmp_object; tmp_object = next_object) {
    next_object = tmp_object->next_content;
    if (alldot && !isname(allbuf, OBJ_NAME(tmp_object))) {
      continue;
    } /* if */

    if(put( ch, tmp_object, to_obj, FALSE)) {
      total++;
    } else /* Let them see why */
      put(ch, tmp_object, to_obj, TRUE);
  }

  if (total && alldot) {
    sprintf(buf, "You put %d %s(s) in $o", total, allbuf);
    act(buf, FALSE, ch, to_obj, 0, TO_CHAR);
    if(total == 1)
      sprintf(buf, "$n puts %s in $o." ,allbuf);
    else if(total < 6)
      sprintf(buf, "$n puts some %s(s) in $o.", allbuf);
    else
      sprintf(buf,"$n puts a bunch of %ss in $o.",allbuf);
    act(buf, TRUE, ch, to_obj, 0, TO_ROOM);
  } else if (total) {
    sprintf(buf,"You put in %d items in $o",total);
    act(buf, FALSE, ch, to_obj, 0, TO_CHAR);
    if(total < 6)
      act("$n puts some stuff in $o.", TRUE, ch, to_obj, 0, TO_ROOM);
    else
      act("$n puts a bunch of stuff in $o.", TRUE, ch, to_obj, 0, TO_ROOM);
  } else {
    send_to_char("You don't have anything to put in it.\n\r", ch);
  }

  return(total);
}

extern struct idname_struct idname[MAX_ID]; // for acquisition orders

void do_put(struct char_data *ch, char *argument, int cmd)
{
  char buffer[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  struct obj_data *obj_object;
  struct obj_data *to_object;
  struct obj_data *tmp_object, *next_obj;
  struct char_data *dummy_char;
  int bits=0;
  int number;
  int type = 0;
  int total = 0;
  bool alldot = FALSE;
  bool found = FALSE;
  char allbuf[MAX_STRING_LENGTH];

  number_argument_interpreter(argument, &number, arg1, arg2);

  if (number <= 0) {
    send_to_char("Sorry but you cannot put negative amount in something...\n\r", ch);
    return;
  }

  if (!*arg1) {
    type = 0;
  } else if (!*arg2) {
    type =1;
  } else {
    bits = generic_find(arg2, FIND_OBJ_EQUIP | FIND_OBJ_INV | FIND_OBJ_ROOM, ch,
                        &dummy_char, &to_object);
    if(!to_object) {
      type = 2;
    } else {
      switch (GET_ITEM_TYPE(to_object)) {
	  case ITEM_CONTAINER:
        if (!IS_SET(to_object->obj_flags.value[1], CONT_CLOSED)) {
          alldot = is_all_dot(arg1, allbuf);
          if (!str_cmp(arg1, "all")){
            type = 3; // put has "all"
          } else {
            type = 4; // put does not have "all"
          }
        } else {
          type = 5; // container is closed
        }
		break;
	  case ITEM_AQ_ORDER:
      if (number == 1) {
        obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
        if (obj_object) { // this is where it gets special for AQ_ORDER
          if (V_OBJ(obj_object) == to_object->obj_flags.value[0] || // we only want to allow you to put things in the order that the order wants
              V_OBJ(obj_object) == to_object->obj_flags.value[1] ||
              V_OBJ(obj_object) == to_object->obj_flags.value[2] ||
              V_OBJ(obj_object) == to_object->obj_flags.value[3]) {
            total = put(ch, obj_object, to_object, TRUE);
          } else {
            sprintf(buffer, "The acquisition order for %s did not include %s.\n\r", idname[obj_object->ownerid[0]].name, OBJ_SHORT(obj_object));
                send_to_char(buffer, ch);
          }
        } else {
          sprintf(buffer, "You don't have the %s.\n\r", arg1);
          send_to_char(buffer, ch);
        }
      } else {    
        for(tmp_object = ch->carrying, total = 0;        
            tmp_object && total < number;
            tmp_object = next_obj) {
          next_obj = tmp_object->next_content;
          if (isname( arg1, OBJ_NAME(tmp_object)) && (CAN_SEE_OBJ(ch, tmp_object))) {
            if ( put( ch, tmp_object, to_object, FALSE)) {
              total++;
            } else {
              put( ch, tmp_object, to_object, TRUE);
            }
          found = TRUE;
          }
        }
        if(!found) {
          sprintf(buffer, "You don't see or have any %s.\n\r", arg1);
          send_to_char(buffer, ch);
        } else {
          sprintf(buffer, "You put %d(%d) %s(s) to %s.\n\r", total, number, arg1, arg2);
          send_to_char(buffer, ch);
          if(total < 6) {
            sprintf(buffer,"$n puts some %s in $o.", arg1);
          } else {
            sprintf(buffer,"$n puts a bunch of %s in $o.",arg1);
          }
          act(buffer, TRUE, ch, to_object, 0, TO_ROOM);
        }
      }    
      break;
	  default:
        type = 6; // target of do_put is not a container-ish object
		break;
	  }
    }
  }

  switch(type){
  case 0: {
    send_to_char("Put what in what?\n\r",ch);
  } break;
  case 1: {
    sprintf(buffer, "Put %s in what?\n\r", arg1);
    send_to_char(buffer, ch);
  } break;
  case 2: {
    sprintf(buffer, "You don't have the %s.\n\r", arg2);
    send_to_char(buffer, ch);
  } break;
  case 3: {
    if(number != 1) {
      sprintf(buffer, "You cannot put %d all %s.\n\r", number, arg2);
      send_to_char(buffer, ch);
    } else {
      total = put_all_to(ch, allbuf, alldot, to_object);
    }
  } break;
  case 4: {
    if (number == 1) {
      obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
      if (obj_object) {
        total = put(ch, obj_object, to_object, TRUE);
      } else {
      sprintf(buffer, "You don't have the %s.\n\r", arg1);
      send_to_char(buffer, ch);
      }
    } else {
      for(tmp_object = ch->carrying, total = 0;
        tmp_object && total < number;
        tmp_object = next_obj) {
      next_obj = tmp_object->next_content;
        if (isname( arg1, OBJ_NAME(tmp_object)))
          if (CAN_SEE_OBJ(ch, tmp_object)) {
            if(put( ch, tmp_object, to_object, FALSE)) {
              total++;
            } else
              put( ch, tmp_object, to_object, TRUE);

            found = TRUE;
          }
      }
      if(!found) {
        sprintf(buffer, "You don't see or have any %s.\n\r", arg1);
        send_to_char(buffer, ch);
      } else {
        sprintf(buffer, "You put %d(%d) %s(s) to %s.\n\r",
                total,  number, arg1, arg2);
        send_to_char(buffer, ch);
        if(total < 6)
          sprintf(buffer,"$n puts some %s in $o.", arg1);
        else
          sprintf(buffer,"$n puts a bunch of %s in $o.",arg1);
        act(buffer, TRUE, ch, to_object, 0, TO_ROOM);
      }
    }
  } break;
  case 5: {
    sprintf(buffer, "The %s seems to closed.\n\r", arg2);
    send_to_char(buffer, ch);
  } break;
  case 6: {
    sprintf(buffer,"The %s is not a container.\n\r", arg2);
    send_to_char(buffer, ch);
  } break;
  default: {
    log_f("BUG: Wrong type in do_put!");
    produce_core();
  } break;
  }
  if (bits == FIND_OBJ_ROOM && total) {
    save_char(ch,NOWHERE);
  }
}

bool give_object_to(CHAR *ch, OBJ *obj, CHAR *vict, int show)
{
  char buf[MSL] = "";

  if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_NODROP))
  {
    if (show) send_to_char("You can't let go of it! Yeech!\n\r", ch);
    return FALSE;
  }

  if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_CLONE))
  {
    if (show)
    {
      act("Your cloned $p falls to pieces as you try to give it away.", FALSE, ch, obj, NULL, TO_CHAR);
      act("$n's cloned $p falls to pieces as $e tries to give it away.", FALSE, ch, obj, NULL, TO_ROOM);
    }

    obj_from_char(obj);
    extract_obj(obj);

    return TRUE;
  }

  if (!CAN_WEAR(obj, ITEM_TAKE))
  {
    if (show) send_to_char("That item can't be given away!\n\r", ch);
    return FALSE;
  }

  if (IS_SET(OBJ_EXTRA_FLAGS2(obj), ITEM_NO_GIVE))
  {
    if (show) send_to_char("You can't seem to give it away!\n\r", ch);
    return FALSE;
  }

  if (IS_SET(OBJ_EXTRA_FLAGS2(obj), ITEM_NO_GIVE_MOB) &&
      IS_NPC(vict) &&
      GET_LEVEL(ch) < LEVEL_IMM)
  {
    if (show) send_to_char("You can't give that to a mob!\n\r", ch);
    return FALSE;
  }

  if ((IS_CARRYING_N(vict) + 1) > CAN_CARRY_N(vict))
  {
    if (show) act("$N seems to have $S hands full.", FALSE, ch, NULL, vict, TO_CHAR);
    return FALSE;
  }

  if (!CAN_CARRY_OBJ(vict, obj))
  {
    if (show) act("$E can't carry that much weight.", FALSE, ch, NULL, vict, TO_CHAR);
    return FALSE;
  }

  if (signal_object(obj, ch, MSG_GAVE_OBJ, MOB_NAME(vict))) return TRUE;

  if (IS_IMMORTAL(ch) || IS_IMMORTAL(vict))
  {
    sprintf(buf, "WIZINFO: %s gives %s (#%d) to %s in room #%d",
      GET_NAME(ch), OBJ_SHORT(obj), V_OBJ(obj), GET_NAME(vict), CHAR_VIRTUAL_ROOM(ch));
    wizlog(buf, GET_LEVEL(vict) + 1, 5);
    log_f("%s", buf);
  }

  if (show)
  {
    act("You give $p to $N.", FALSE, ch, obj, vict, TO_CHAR);
    act("$n gives you $p.", FALSE, ch, obj, vict, TO_VICT);
    act("$n gives $p to $N.", TRUE, ch, obj, vict, TO_NOTVICT);
  }

  obj_from_char(obj);
  obj_to_char(obj, vict);

  signal_char(vict, ch, MSG_GAVE_OBJ, OBJ_NAME(obj));

  return TRUE;
}

int give_number_object_to(struct char_data *ch, int number, char *name,
                          struct char_data *vict)
{
  char buf[MAX_STRING_LENGTH];
  int total;
  struct obj_data *tmp_object, *next_obj;

  for(tmp_object = ch->carrying, total = 0;
      tmp_object && total < number;
      tmp_object = next_obj) {
    next_obj = tmp_object->next_content;
    if (isname( name, OBJ_NAME(tmp_object)))
      if (CAN_SEE_OBJ(ch, tmp_object)) {
        total += give_object_to(ch, tmp_object, vict, FALSE);
      }
  }
  if(total) {
    act("$n gives some stuff to $N.", 1, ch, 0, vict, TO_NOTVICT);
    sprintf(buf, "%s gives you %d %s(s).\n\r", GET_NAME(ch), total, name);
    send_to_char(buf, vict);
    sprintf(buf, "You give %s %d %s(s).\n\r", GET_NAME(vict), total, name);
    send_to_char(buf, ch);
  }

  return(total);
}

int give_coins_to(struct char_data *ch, int amount, struct char_data *vict)
{
  char buf[MAX_STRING_LENGTH];

  if(GET_GOLD(ch) < amount &&
     (IS_NPC(ch) || (GET_LEVEL(ch) < LEVEL_SUP && !IS_IMMORTAL(ch)))) {
    send_to_char("You haven't got that many coins!\n\r",ch);
    return(0);
  }

  send_to_char("Ok.\n\r",ch);
  sprintf(buf,"%s gives you %d gold coins.\n\r", PERS(ch,vict), amount);
  send_to_char(buf,vict);
  act("$n gives some gold to $N.", 1, ch, 0, vict, TO_NOTVICT);

  if(amount > 1000000) { /* changed min to 1 mil 031803 Liner */
    sprintf(buf,"WIZ: Large transaction: %s gives %d coins to %s",
          GET_NAME(ch),amount, GET_NAME(vict));
    log_s(buf);
  }
  if(IS_IMMORTAL(ch)) {
    sprintf(buf, "WIZINFO: (%s) gives %d coins to %s\n\r",
            GET_NAME(ch), amount, GET_NAME(vict));
    wizlog(buf, GET_LEVEL(ch)+1, 5);
    log_s(buf);
  }

  /* Lose gold only if you're an NPC or mortal */
  if (IS_NPC(ch) || (GET_LEVEL(ch) < LEVEL_SUP || !IS_IMMORTAL(ch)))
    GET_GOLD(ch) -= amount;
  GET_GOLD(vict) += amount;

  return(amount);
}

void do_give(struct char_data *ch, char *argument, int cmd)
{
  char obj_name[MAX_INPUT_LENGTH], vict_name[MAX_INPUT_LENGTH],
  buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  int number;
  int total =0;
  int type = 2;
  bool alldot=FALSE;
  char allbuf[MAX_STRING_LENGTH];
  struct char_data *vict=NULL;
  struct obj_data *obj=0, *next_obj;

  number_argument_interpreter(argument, &number, obj_name, vict_name);
  if(number <= 0)
    type = 0;
  else if (is_number(obj_name))
    type = 1;
  else if(!*obj_name || !*vict_name)
    type = 2;
  else if (!(vict = get_char_room_vis(ch, vict_name)))
    type = 3;
  else {
    alldot = is_all_dot(obj_name, allbuf);

    if (!str_cmp(obj_name, "all")){
      if(number != 1)
        type = 4;
      else
        type = 5;
    } else {
      type = 6;
    }
  }
  if (ch==vict)
    {
      sprintf(buf,"$n tries to juggle some of $s equipment.");
      act(buf,FALSE,ch,0,0,TO_ROOM);
      sprintf(buf,"You look like a fool, handing yourself items like that.\n\r");
      send_to_char(buf,ch);
      return;
    }
  switch(type) {
  case 0: {
    send_to_char("Sorry but you cannot use negative number amounts.\n\r", ch);
  } break;
  case 1: {
    send_to_char("Number field too large.\n\r",ch);
  } break;
  case 2: {
    send_to_char("Give what to who?\n\r", ch);
  } break;
  case 3: {
    send_to_char("To who?\n\r",ch);
  } break;
  case 4: {
    send_to_char("You cannot use amount number and all or all.something\n\r", ch);
  } break;
  case 5: {
    if (alldot && (!str_cmp("coins", allbuf) || !str_cmp("coin", allbuf))) {
      total = give_coins_to(ch, GET_GOLD(ch), vict);
    } else {
      for(obj = ch->carrying;
          obj;
          obj = next_obj) {
        next_obj = obj->next_content;
        if(alldot && !isname( allbuf, OBJ_NAME(obj)))
          continue;

        total += give_object_to(ch, obj, vict, TRUE);
      }
    }
  } break;
  case 6: {
    if (!str_cmp("coins",obj_name) || !str_cmp("coin", obj_name)) {
      total = give_coins_to(ch, number, vict);
    } else if(number == 1) {
      if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
        send_to_char("You do not seem to have anything like that.\n\r",
                   ch);
      } else {
        total = give_object_to(ch, obj, vict, TRUE);
      }
    } else {
      total = give_number_object_to(ch, number, obj_name, vict);
    }
  } break;
  default: {
    log_f("BUG: Wrong type in do_give!");
    produce_core();
  } break;
  }

  if(total) {
    save_char(ch,NOWHERE);
    save_char(vict,NOWHERE);

    /* Log info about flagged people */
    if((IS_SET( ch->specials.pflag, PLR_KILL | PLR_THIEF) &&
      !IS_NPC(ch) && !CHAOSMODE) ||
       (IS_SET( vict->specials.pflag, PLR_KILL | PLR_THIEF) &&
      !IS_NPC(vict)&& !CHAOSMODE)) {
      sprintf(buf,"%s ",GET_NAME(ch));
      if(IS_SET( ch->specials.pflag, PLR_KILL) && !IS_NPC(ch))
      strcat( buf, "(killer)");
      if(IS_SET( ch->specials.pflag, PLR_THIEF) && !IS_NPC(ch))
      strcat( buf, "(thief)");
      sprintf(buf2, " gives %d %s to %s", total, obj_name,GET_NAME(vict));
      strcat(buf, buf2);
      if(IS_SET( vict->specials.pflag, PLR_KILL) && !IS_NPC(vict))
      strcat(buf,"(killer)");
      if(IS_SET( vict->specials.pflag, PLR_THIEF) && !IS_NPC(vict))
      strcat(buf,"(thief)");
      wizlog(buf, GET_LEVEL(ch)+1, 5);
      log_s(buf);
    }
  }
}

void do_split(struct char_data *ch, char *argument, int cmd) { /* Updated Dec 98 - Ranger */
  char buf[MAX_STRING_LENGTH];
  int no_members=0, amount;
  struct char_data *k;
  struct follow_type *f;

  /* Initial Checks */
  amount = 0;
  one_argument(argument,buf);
  if(!*buf) {
    send_to_char("How much do you want to split?\n\r",ch);
    return;
  }
  if(!is_number(buf)) {
    send_to_char("Sorry, you can't do that!\n\r",ch);
    return;
  }

  amount=atoi(buf);
  if (amount<=0) {
    send_to_char("Sorry, you can't do that!\n\r",ch);
    return;
  }
  if (GET_GOLD(ch)<amount) {
    send_to_char("You haven't got that many coins!\n\r",ch);
    return;
  }

  if(!(k=ch->master)) k=ch; /* If ch doesn't have a master, ch must be the leader */

  /* Check for same room */
  if(IS_AFFECTED(k, AFF_GROUP) && (CHAR_REAL_ROOM(k) == CHAR_REAL_ROOM(ch)))
    no_members = 1;
  else {
    send_to_char("Your leader is not here!\n\r",ch);
    return; /* Added by Ranger April 96 */
  }

  /* Count the number of followers */
  for(f=k->followers; f; f=f->next)
    if(IS_AFFECTED(f->follower, AFF_GROUP) &&
       (CHAR_REAL_ROOM(f->follower) == CHAR_REAL_ROOM(ch)))
      no_members++;

  if(no_members==0) {
    send_to_char("You can't split the money with yourself!.\n\r",ch);
    return;
  }

  amount=amount/no_members;

  if(IS_IMMORTAL(ch)) amount=0;

  /* Distribute to the followers */
  for(f=k->followers; f; f=f->next) {
    if(IS_AFFECTED(f->follower, AFF_GROUP) && (f->follower!=ch) &&
       (CHAR_REAL_ROOM(f->follower) == CHAR_REAL_ROOM(ch))) {
      sprintf(buf,"%s splits %d coins to the group.\n\r", GET_NAME(ch), amount);
      send_to_char(buf,f->follower);
      GET_GOLD(ch)-=amount;
      GET_GOLD(f->follower)+=amount;
      save_char(f->follower,NOWHERE);
    }
  }

  /* If ch wasn't the leader, give some coins to the leader */
  if(k!=ch) {
    GET_GOLD(k)+=amount;
    GET_GOLD(ch)-=amount;
    sprintf(buf,"%s splits %d coins to the group.\n\r", GET_NAME(ch), amount);
    send_to_char(buf,k);
    save_char(k,NOWHERE);
  }
  sprintf(buf,"You give %d coins to each of your group members.\n\r",amount);
  send_to_char(buf,ch);
  save_char(ch, NOWHERE);
  return;
}
