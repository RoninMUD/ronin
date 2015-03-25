/* Gambling code for Ronin
**
** Lottery code by Ranger October 1998
**   - similar to auction board
*/

/*
$Author: ronin $
$Date: 2005/04/27 17:13:30 $
$Header: /home/ronin/cvs/ronin/gamble.c,v 2.6 2005/04/27 17:13:30 ronin Exp $
$Id: gamble.c,v 2.6 2005/04/27 17:13:30 ronin Exp $
$Name:  $
$Log: gamble.c,v $
Revision 2.6  2005/04/27 17:13:30  ronin
Minor changes needed to compile on Slackware 10 for the new machine.

Revision 2.5  2005/01/25 21:51:00  ronin
Added Void for lotto access.

Revision 2.4  2005/01/21 14:55:27  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.3  2004/09/30 15:57:32  ronin
Changed mist from ETE to SUP in lotto_access.

Revision 2.2  2004/08/19 13:00:20  ronin
Added Mist to lottery access. (Lem)

Revision 2.1  2004/03/04 17:23:57  ronin
Addition of object file version 2 which includes 8 ownerid fields
for addition of some objects only being able to be used by those
owners.

Revision 2.0.0.1  2004/02/05 16:09:19  ronin
Reinitialization of cvs archives

Revision - Added cool Dealer_Visor for poker games, etc.

Revision - Changed extract range to 131 for extracting old tickets
Ranger 7-Feb-03

Revision 1.4  2002/04/18 04:07:31  ronin
Changing log output from perror to log_f for internal syslog manipulation.

Revision 1.3  2002/03/31 08:44:42  ronin
Replaced #include <sys/time.h> with <time.h> to elminate several time
warnings.

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "structs.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "spells.h"
#include "utils.h"
#include "reception.h"
#include "utility.h"
#include "utils.h"
#include "act.h"
#include "modify.h"
#include "cmd.h"
#include "spec_assign.h"

#define LOTTERY_FILE      "lottery.data"
#define MAX_OBJS_LOTTERY  10
#define TICKET            17
#define LOTTERY_MACHINE   18

OBJ *lotto_machine=0;

/* Lottery Routines */
struct lottery_elem {
  struct obj_data *obj;
  int lot;
  int price;
  int top_ticket;
  char *drawdate;
};

struct lottery_mach {
  int top_lot;
  struct lottery_elem lotto[MAX_OBJS_LOTTERY];
  int obj_num;
} l_mach;

struct lottery_file_elem {
  struct obj_file_elem_ver3 obj_elem;
  int lot;
  int price;
  int top_ticket;
  char drawdate[30];
};

void intialize_lottery(void) {
  char buf[256];
  int i, j;
  struct lottery_file_elem elem;
  struct obj_data *obj;
  FILE *fl;

  for(i = 0; i < MAX_OBJS_LOTTERY; i++) {
    l_mach.lotto[i].obj=0;
    l_mach.lotto[i].lot=0;
    l_mach.lotto[i].price=0;
    l_mach.lotto[i].top_ticket=0;
    l_mach.lotto[i].drawdate=0;
  }
  l_mach.obj_num=0;
  l_mach.top_lot=0;

  if(!(fl=fopen(LOTTERY_FILE, "rb+"))) {
    log_f("  No lottery file.");
    return;
  }

  fread(&l_mach.top_lot,sizeof(l_mach.top_lot),1,fl);

  while (!feof(fl)) {
    fread(&elem, sizeof(elem), 1, fl);
    if (ferror(fl)) {
      log_f("Error reading lottery file");
      fclose(fl);
      return;
    }
    if(!feof(fl)) {
      if(elem.obj_elem.item_number == -1) obj = 0;
      else
      if((obj = read_object(elem.obj_elem.item_number, VIRTUAL))) {
        obj->obj_flags.timer       = elem.obj_elem.timer;
        obj->obj_flags.value[0]    = elem.obj_elem.value[0];
        obj->obj_flags.value[1]    = elem.obj_elem.value[1];
        obj->obj_flags.value[2]    = elem.obj_elem.value[2];
        obj->obj_flags.value[3]    = elem.obj_elem.value[3];
        obj->obj_flags.extra_flags = elem.obj_elem.extra_flags;
        obj->obj_flags.weight      = elem.obj_elem.weight;
        obj->obj_flags.bitvector   = elem.obj_elem.bitvector;
        for(j = 0; j < OFILE_MAX_OBJ_AFFECT; j++) {
          obj->affected[j].location = (int)elem.obj_elem.affected[j].location;
          obj->affected[j].modifier = (int)elem.obj_elem.affected[j].modifier;
        }

        /* new obj saves for obj ver3 */
        obj->obj_flags.bitvector2   = elem.obj_elem.bitvector2;
        /* end new ver3 obj saves */

      } else {
        log_f("Unknown object in lottery.");
        continue;
      }
      l_mach.lotto[l_mach.obj_num].obj        = obj;
      l_mach.lotto[l_mach.obj_num].lot        = elem.lot;
      l_mach.lotto[l_mach.obj_num].price      = elem.price;
      l_mach.lotto[l_mach.obj_num].top_ticket = elem.top_ticket;
      if(*elem.drawdate)
        l_mach.lotto[l_mach.obj_num].drawdate = str_dup(elem.drawdate);
      l_mach.obj_num++;
    }
  }
  fclose(fl);
  sprintf(buf, "  Lottery machine size: %d objects",l_mach.obj_num);
  log_s(buf);
}

void save_lottery(void) {
  int i, j;
  struct lottery_file_elem elem;
  FILE *fl;

  if(!(fl = fopen(LOTTERY_FILE, "wb+"))) {
    log_f(LOTTERY_FILE);
    return;
  }

  if(fwrite(&l_mach.top_lot,sizeof(l_mach.top_lot), 1, fl) < 1) {
    log_f("ERROR! writing lottery data lot size");
    return;
  }

  for(i = 0; i < MAX_OBJS_LOTTERY; i++) {
    if(!l_mach.lotto[i].obj) continue;
    elem.lot        = l_mach.lotto[i].lot;
    elem.price      = l_mach.lotto[i].price;
    elem.top_ticket = l_mach.lotto[i].top_ticket;
    if(l_mach.lotto[i].drawdate)
      strcpy(elem.drawdate, l_mach.lotto[i].drawdate);
    else *elem.drawdate = '\0';

    elem.obj_elem.item_number = obj_proto_table[l_mach.lotto[i].obj->item_number].virtual;
    elem.obj_elem.timer       = l_mach.lotto[i].obj->obj_flags.timer;
    elem.obj_elem.value[0]    = l_mach.lotto[i].obj->obj_flags.value[0];
    elem.obj_elem.value[1]    = l_mach.lotto[i].obj->obj_flags.value[1];
    elem.obj_elem.value[2]    = l_mach.lotto[i].obj->obj_flags.value[2];
    elem.obj_elem.value[3]    = l_mach.lotto[i].obj->obj_flags.value[3];
    elem.obj_elem.extra_flags = l_mach.lotto[i].obj->obj_flags.extra_flags;
    elem.obj_elem.weight      = l_mach.lotto[i].obj->obj_flags.weight;
    elem.obj_elem.bitvector   = l_mach.lotto[i].obj->obj_flags.bitvector;

/* new obj saves */
    elem.obj_elem.version=32003;
    elem.obj_elem.type_flag   = l_mach.lotto[i].obj->obj_flags.type_flag;
    elem.obj_elem.wear_flags  = l_mach.lotto[i].obj->obj_flags.wear_flags;
    elem.obj_elem.extra_flags2= l_mach.lotto[i].obj->obj_flags.extra_flags2;
    elem.obj_elem.subclass_res= l_mach.lotto[i].obj->obj_flags.subclass_res;
    elem.obj_elem.material    = l_mach.lotto[i].obj->obj_flags.material;
    elem.obj_elem.spec_value  = l_mach.lotto[i].obj->spec_value;
    for(j = 0; j < MAX_OBJ_AFFECT; j++)
	   elem.obj_elem.affected[j] = l_mach.lotto[i].obj->affected[j];
/* end new obj saves */

/* new obj saves for obj ver3 */
      elem.obj_elem.bitvector2  = l_mach.lotto[i].obj->obj_flags.bitvector2;
/* end new ver3 obj saves */

    elem.obj_elem.ownerid[0] = 0;
    elem.obj_elem.ownerid[1] = 0;
    elem.obj_elem.ownerid[2] = 0;
    elem.obj_elem.ownerid[3] = 0;
    elem.obj_elem.ownerid[4] = 0;
    elem.obj_elem.ownerid[5] = 0;
    elem.obj_elem.ownerid[6] = 0;
    elem.obj_elem.ownerid[7] = 0;

    if (fwrite(&elem, sizeof(elem), 1, fl) < 1) {
      log_f("ERROR! writing lottery data");
      break;
    }
  }
  fclose(fl);
}

int lottery_access(CHAR *ch) {
  if(GET_LEVEL(ch)>=LEVEL_ETE && IS_SET(ch->new.imm_flags, WIZ_ACTIVE))
    return TRUE;
  if(!strcmp(GET_NAME(ch),"Kith"))
    return TRUE;
  return FALSE;
}

int lottery_look(CHAR *ch, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  int i,total = 0;
  struct string_block sb;

  one_argument( arg, buf);

  if (!*buf || !isname(buf, "lottery machine")) return(FALSE);
  init_string_block(&sb);

  append_to_string_block(&sb,"\
Welcome to the Midgaard Lottery.\n\r\n\r\
To buy a ticket on an item simply type buy <#> <amt>\n\r\
                                   ex: buy 0 5\n\r\
More information can be gained by typing: HELP lottery\n\r\n\r");

  if(lottery_access(ch))
    append_to_string_block(&sb,"\
For those with Lotto Control only:\n\r\n\r\
Draw <#> <name>/cancel will generate a random ticket number for the object.\n\r\
  <name> has stop control and must say 'stop' to finish the drawing.\n\r\
Put <obj> <price> <drawdate> will place the object in the lottery.\n\r\
Recover <#> to get the object from the lotto machine\n\r\
Date <#> <new date> to fix the drawing date.\n\r\n\r");

  if(!l_mach.obj_num)
    append_to_string_block(&sb,"There are no objects in the lottery at this moment.\n\r");
  else {
    sprintf(buf, "There are %d object(s) in the lottery.\n\r\n\r", l_mach.obj_num);
    append_to_string_block(&sb,buf);
    append_to_string_block(&sb,"Num Lot # Object:                   Price   # Sold  Draw Date:\n\r");
    append_to_string_block(&sb,"--------------------------------------------------------------------------\n\r");
    total=0;
    for( i = 0; i < MAX_OBJS_LOTTERY; i++) {
      if(!l_mach.lotto[i].obj) continue;
      sprintf(buf,"[%1d] [%3d] [%21.21s] [%7d] [%5d] [%s]\n\r",
              i,l_mach.lotto[i].lot,OBJ_SHORT(l_mach.lotto[i].obj),
              l_mach.lotto[i].price,l_mach.lotto[i].top_ticket,
              l_mach.lotto[i].drawdate);
      append_to_string_block(&sb,buf);
      total+=l_mach.lotto[i].price*l_mach.lotto[i].top_ticket;
    }
  }
  sprintf(buf,"\n\rTotal coins in machine: %d\n\r",total);
  if(lottery_access(ch)) append_to_string_block(&sb,buf);
  page_string_block(&sb,ch);
  destroy_string_block(&sb);
  return(TRUE);
}


int adjust_ticket_strings(OBJ *obj) { /*Added Oct 98 Ranger */
  char buf[MAX_INPUT_LENGTH];
  char *Tcolor[10]={
   "purple","brown","grey","white","cyan",
   "red","orange","green","yellow","blue"};
  int lot,clr;
  if(!obj) return FALSE;
  if(obj->obj_flags.type_flag != ITEM_TICKET) return FALSE;
  lot=obj->obj_flags.value[0];
  /* Ticket extractions */
  if(lot>0 && lot<131) {
      extract_obj(obj);return FALSE;
  }

  clr=lot%10;
  sprintf(buf,"lottery ticket lot%d %s",lot,Tcolor[clr]);
  obj->name=str_dup(buf);
  sprintf(buf,"%s lottery ticket from lot %d",Tcolor[clr],lot);
  obj->short_description=str_dup(buf);
/*  sprintf(buf,"There is a tiny %s lottery ticket from lot %d lying here.",Tcolor[clr],lot);
  obj->description=str_dup(buf);*/
  return TRUE;
}

void lottery_buy(CHAR *ch, char *arg) {
  char buf[MAX_STRING_LENGTH];
  int number,amount;
  OBJ *obj;

  arg=one_argument(arg, buf);
  if(!*buf) {
    send_to_char("Which object did you want a ticket for?\n\r",ch);
    return;
  }
  if(!is_number(buf) || (number = atoi(buf)) < 0) {
    send_to_char("Please make that a positive number.\n\r",ch);
    return;
  }
  if(number >= MAX_OBJS_LOTTERY || !l_mach.lotto[number].obj) {
    send_to_char("There isn't any such object number.",ch);
    return;
  }
  one_argument(arg,buf);
  if(!*buf) {
    send_to_char("Please specify the number of tickets.\n\r",ch);
    return;
  }
  if(!is_number(buf) || (amount=atoi(buf))<1) {
    send_to_char("You must buy at least 1 ticket.\n\r",ch);
    return;
  }
  if(amount>250) {
    send_to_char("Only 250 tickets can fit on one ticket stub.\n\r",ch);
    return;
  }

  if(GET_GOLD(ch) < l_mach.lotto[number].price*amount) {
    send_to_char("Sorry, you don't have enough gold on you.\n\r",ch);
    return;
  }
  if(!(obj=read_object(TICKET, VIRTUAL))) {
    log_f("Error, lottery TICKET doesn't exist");
    return;
  }
  obj->obj_flags.value[0]=l_mach.lotto[number].lot;
  obj->obj_flags.value[1]=l_mach.lotto[number].top_ticket+1;
  l_mach.lotto[number].top_ticket+=amount;
  obj->obj_flags.value[2]=l_mach.lotto[number].top_ticket;

  if(!adjust_ticket_strings(obj)) return;
  send_to_char("The lottery machine gives you a ticket.\n\r",ch);
  act("The lottery machine gives $n a ticket.\n\r",0,ch,0,0,TO_ROOM);
  GET_GOLD(ch)-=l_mach.lotto[number].price*amount;
  obj_to_char(obj, ch);
  save_lottery();
  save_char(ch, NOWHERE);
}

/* Assumes access has already been checked for*/
void lottery_put(CHAR *ch, char *arg) {
  char buf[MAX_STRING_LENGTH];
  int number, start;
  struct obj_data *tmp_obj;

  for(number = 0; number < MAX_OBJS_LOTTERY; number++)
    if(!l_mach.lotto[number].obj) break;

  if(number == MAX_OBJS_LOTTERY) {
    send_to_char("The lottery machine is full.\n\r",ch);
    return;
  }

  arg=one_argument(arg,buf);
  if(!*buf) {
    send_to_char("What did you want to place in the lottery?\n\r",ch);
    return;
  }
  if(!(tmp_obj=get_obj_in_list_vis( ch, buf, ch->carrying))) {
    send_to_char("You don't have that item.\n\r",ch);
    return;
  }
  if((tmp_obj->obj_flags.type_flag == ITEM_FOOD) ||
     (tmp_obj->obj_flags.type_flag == ITEM_TRASH) ||
     (tmp_obj->item_number < 0) ||
     (IS_SET(tmp_obj->obj_flags.extra_flags, ITEM_CLONE)) ||
     (IS_SET(tmp_obj->obj_flags.extra_flags, ITEM_ANTI_RENT))) {
    send_to_char("You cannot put the item in the lottery.\n\r",ch);
    return;
  }

  arg=one_argument(arg, buf);
  if(!*buf) {
    send_to_char("What is the ticket price?",ch);
    return;
  }
  if((start = atoi(buf)) < 1) {
    send_to_char("The ticket price must be a positive number.\n\r",ch);
    return;
  }

  if(!*arg) {
    send_to_char("How about a draw date for the players benifit?\n\r",ch);
    return;
  }
  if(strlen(arg)>30) {
    send_to_char("Enter a shorter draw date.\n\r",ch);
    return;
  }

  obj_from_char(tmp_obj);
  l_mach.top_lot++;
  l_mach.lotto[number].obj        = tmp_obj;
  l_mach.lotto[number].price      = start;
  l_mach.lotto[number].top_ticket = 0;
  l_mach.lotto[number].drawdate   = str_dup(arg);
  l_mach.lotto[number].lot        = l_mach.top_lot;
  l_mach.obj_num++;

  act("$n puts $p in the lottery machine.",1,ch,tmp_obj,0,TO_ROOM);
  act("You put $p in the lottery machine.",0,ch,tmp_obj,0,TO_CHAR);
  save_lottery();
  save_char(ch, NOWHERE);
}

/* Assumes access has already been checked for */
void lottery_recover(CHAR *ch, char *arg) {
  char buf[MAX_STRING_LENGTH];
  int number;

  one_argument(arg, buf);
  if(!*buf) {
    send_to_char("Which object did you want to recover?\n\r",ch);
    return;
  }
  if((number = atoi(buf)) < 0) {
    send_to_char("Please make that a positive number.\n\r",ch);
    return;
  }
  if(number >= MAX_OBJS_LOTTERY || !l_mach.lotto[number].obj) {
    send_to_char("There isn't any such object number.",ch);
    return;
  }

  act("$n gets $p from the lottery machine.", FALSE, ch, l_mach.lotto[number].obj, 0, TO_ROOM);
  act("You get $p from the lottery machine.", FALSE, ch, l_mach.lotto[number].obj, 0, TO_CHAR);

  obj_to_char(l_mach.lotto[number].obj, ch);
  l_mach.lotto[number].obj = 0;
  l_mach.obj_num--;

  save_lottery();
  save_char(ch, NOWHERE);
}

/* Assumes access has been checked for */
void lottery_draw(CHAR *ch, OBJ *machine, char *arg) {
  char buf[MAX_INPUT_LENGTH],buf2[MAX_STRING_LENGTH],*new_name;
  int number;

  if(machine->obj_flags.value[0]) {
    one_argument(arg,buf);
    if(*buf) {
      if(!strcmp(buf,"cancel")) {
        send_to_char("Drawing cancelled.\n\r",ch);
        act("The Drawing was cancelled.",0,ch,0,0,TO_ROOM);
        machine->name=str_dup("lottery machine");
        l_mach.lotto[machine->obj_flags.value[1]].drawdate = str_dup("Cancelled");
        machine->obj_flags.value[0]=0;
        machine->obj_flags.value[1]=0;
        machine->obj_flags.value[2]=0;
        return;
      }
    }
    send_to_char("A drawing is already in progress.\n\r",ch);
    return;
  }

  arg=one_argument(arg, buf);
  if(!*buf) {
    send_to_char("Which object number did you wish to draw for?\n\r",ch);
    return;
  }
  if((number = atoi(buf)) < 0) {
    send_to_char("Please make that a positive number.\n\r",ch);
    return;
  }
  if(number >= MAX_OBJS_LOTTERY || !l_mach.lotto[number].obj) {
    send_to_char("There isn't any such object number.",ch);
    return;
  }
  one_argument(arg,buf);
  if(!*buf) {
    send_to_char("Who is going to control the draw?\n\r",ch);
    return;
  }

  machine->obj_flags.value[0]=1;
  machine->obj_flags.value[1]=number;
  machine->obj_flags.value[2]=1000;
  sprintf(buf2,"%s",buf);
  CREATE(new_name,char,strlen(OBJ_NAME(machine))+strlen(buf2)+2);
  sprintf(new_name,"%s %s",OBJ_NAME(machine),buf);
  if(machine->item_number < 0 || machine->name != obj_proto_table[machine->item_number].name)
    free(machine->name);
  machine->name = new_name;
  l_mach.lotto[number].drawdate = str_dup("Drawing in Progress");
  sprintf(buf2,"Lottery drawing started for lot number %d.\n\r\n\r%s will be controlling the draw!\n\r",l_mach.lotto[number].lot,CAP(buf));
  act(buf2,0,ch,0,0,TO_ROOM);
  act(buf2,0,ch,0,0,TO_CHAR);

}

void lottery_stop(OBJ *machine, CHAR *ch) {
  char buf[MAX_INPUT_LENGTH];

  if(!machine->obj_flags.value[0]) return;
  if(!machine->obj_flags.value[2]) return;

  sprintf(buf,"%s has caused the lottery machine to stop on number %d!",GET_NAME(ch),machine->obj_flags.value[2]);
  act(buf,0,ch,0,0,TO_ROOM);
  sprintf(buf,"You have caused the lottery machine to stop on number %d!\n\r",machine->obj_flags.value[2]);
  send_to_char(buf,ch);
  sprintf(buf,"\n\rATTENTION: For Lot %d, the winning lottery ticket is number %d!\n\r",l_mach.lotto[machine->obj_flags.value[1]].lot,machine->obj_flags.value[2]);
  send_to_world(buf);
  send_to_world(buf);
  sprintf(buf,"Winning Ticket: %d",machine->obj_flags.value[2]);
  l_mach.lotto[machine->obj_flags.value[1]].drawdate = str_dup(buf);
  machine->obj_flags.value[0]=0;
  machine->obj_flags.value[1]=0;
  machine->obj_flags.value[2]=0;
  machine->name=str_dup("lottery machine");
  save_lottery();
}

void lottery_fix_date(CHAR *ch, OBJ *machine, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  int number;

  if(machine->obj_flags.value[0]) {
    send_to_char("Wait for the drawing to be over, or cancel it.\n\r",ch);
    return;
  }

  arg=one_argument(arg, buf);
  if(!*buf) {
    send_to_char("Which object number did you wish to fix the date for?\n\r",ch);
    return;
  }
  if((number = atoi(buf)) < 0) {
    send_to_char("Please make that a positive number.\n\r",ch);
    return;
  }
  if(number >= MAX_OBJS_LOTTERY || !l_mach.lotto[number].obj) {
    send_to_char("There isn't any such object number.",ch);
    return;
  }
  if(!*arg) {
    send_to_char("What is the new date?\n\r",ch);
    return;
  }

  l_mach.lotto[number].drawdate = str_dup(arg);
  printf_to_char(ch,"Date for lot %d changed to %s.\n\r",number,arg);
  save_lottery();
  return;
}

int lottery_machine(OBJ *machine, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  int top;

  if(cmd==MSG_TICK) {
    if(!lotto_machine) lotto_machine=machine;
    return FALSE;
  }

  if(cmd==MSG_VIOLENCE) {
    if(machine->in_room==NOWHERE) return FALSE;
    if(!machine->obj_flags.value[0]) return FALSE;
    top=l_mach.lotto[machine->obj_flags.value[1]].top_ticket;
    machine->obj_flags.value[2]=number(1,top);
    sprintf(buf,"The machine rolls a random number for lot %d between 1 and %d.\n\r",l_mach.lotto[machine->obj_flags.value[1]].lot,top);
    send_to_room(buf,machine->in_room);
    return FALSE;
  }

  if(!ch) return(FALSE);
  if(IS_NPC(ch)) return(FALSE);

  switch(cmd) {
    case CMD_LOOK:
      return(lottery_look(ch, arg));
      break;
    case CMD_BUY:
      lottery_buy(ch, arg);
      return TRUE;
      break;
    case CMD_PUT:
      if(!lottery_access(ch)) return FALSE;
      lottery_put(ch, arg);
      return(TRUE);
      break;
    case CMD_RECOVER:
      if(!lottery_access(ch)) return FALSE;
      lottery_recover(ch, arg);
      return(TRUE);
      break;
    case CMD_UNKNOWN:
      arg=one_argument(arg,buf);
      if(!*buf) return FALSE;
      if(is_abbrev(buf,"draw")) {
        if(!lottery_access(ch)) return FALSE;
        lottery_draw(ch,machine,arg);
        return(TRUE);
      }
      if(is_abbrev(buf,"date")) {
        if(!lottery_access(ch)) return FALSE;
        lottery_fix_date(ch,machine,arg);
        return(TRUE);
      }
      return FALSE;
      break;
    case CMD_SAY:
      if(!machine->obj_flags.value[0]) return FALSE;
      if(!machine->obj_flags.value[2]) return FALSE;
      sprintf(buf,"%s",GET_NAME(ch));
      string_to_lower(buf);
      if(!isname(buf,machine->name)) return FALSE;
      one_argument(arg,buf);
      if(!strcmp(buf,"stop")) {
        do_say(ch,arg,CMD_SAY);
        lottery_stop(machine,ch);
        return TRUE;
      }
      return FALSE;
    default:
      return(FALSE);
    break;
  }
  return(FALSE);
}

int slot_machine(OBJ *machine, CHAR *ch, int cmd, char *arg) {
  int num1, num2, num3, win = 0;
  char buf[MIL],*slot_msg[] = {
	"*YOU SHOULDN'T SEE THIS*",
	"a mithril bar",              /* 1 */
	"a golden dragon",
	"a Dwarven hammer",
	"a temple",
	"an Elven bow",               /* 5 */
	"a red brick",
	"a refuse pile",
	"a waybread",
	"a Gnomish bell",
	"a beggar",                   /* 10 */
};

  if(cmd!=CMD_USE) return FALSE;
  one_argument(arg,buf);
  if(!*buf) return FALSE;
  if(strcmp(buf,"slot")) return FALSE;

  if (GET_GOLD(ch) < 1) {
	send_to_char("You do not have enough money to play the slots!\r\n", ch);
	return TRUE;
     } else
	GET_GOLD(ch) -= 1;

     act("$N pulls on the crank of the Gnomish slot machine.",
	 FALSE, 0, ch, 0, TO_ROOM);
     send_to_char("You pull on the crank of the Gnomish slot machine.\r\n", ch);

     /* very simple roll 3 random numbers from 1 to 10 */
     num1 = number(1, 10);
     num2 = number(1, 10);
     num3 = number(1, 10);

     if (num1 == num2 && num2 == num3) {
	/* all 3 are equal! Woohoo! */
	if (num1 == 1)
	   win += 50;
	else if (num1 == 2)
	   win += 25;
	else if (num1 == 3)
	   win += 15;
	else if (num1 == 4)
	   win += 10;
	else if (num1 == 5)
	   win += 5;
	else if (num1 == 10)
	   win += 1;
     }

/*     sprintf(buf, "$n got %s, %s, and %s.", slot_msg[num1],
	     slot_msg[num2], slot_msg[num3]);
     act(buf,0,ch,0,0,TO_ROOM);
*/
     sprintf(buf, "You got %s, %s, %s, ", slot_msg[num1],
	     slot_msg[num2], slot_msg[num3]);
     if (win > 1)
	sprintf(buf, "%syou win %d gold pieces!\r\n", buf, win);
     else if (win == 1)
	sprintf(buf, "%syou win 1 gold piece!\r\n", buf);
     else
	sprintf(buf, "%syou lose.\r\n", buf);
     send_to_char(buf, ch);
     GET_GOLD(ch) += win;

     return TRUE;
}



int dealer_visor(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  struct obj_data *tmp;
  struct obj_data *next_tmp;
  struct obj_data *tmp_obj;
  char buf[MIL];
  int top=0, num, num2=0,i,num_cards;
  char *card[] = {
    "`pAce of Spades     -  A S`q\n",
    "`pDeuce of Spades   -  2 S`q\n",
    "`pThree of Spades   -  3 S`q\n",
    "`pFour of Spades    -  4 S`q\n",
    "`pFive of Spades    -  5 S`q\n",
    "`pSix of Spades     -  6 S`q\n",
    "`pSeven of Spades   -  7 S`q\n",
    "`pEight of Spades   -  8 S`q\n",
    "`pNine of Spades    -  9 S`q\n",
    "`pTen of Spades     - 10 S`q\n",
    "`pJack of Spades    -  J S`q\n",
    "`pQueen of Spades   -  Q S`q\n",
    "`pKing of Spades    -  K S`q\n",
    "`pAce of Clubs      -  A C`q\n",
    "`pDeuce of Clubs    -  2 C`q\n",
    "`pThree of Clubs    -  3 C`q\n",
    "`pFour of Clubs     -  4 C`q\n",
    "`pFive of Clubs     -  5 C`q\n",
    "`pSix of Clubs      -  6 C`q\n",
    "`pSeven of Clubs    -  7 C`q\n",
    "`pEight of Clubs    -  8 C`q\n",
    "`pNine of Clubs     -  9 C`q\n",
    "`pTen of Clubs      - 10 C`q\n",
    "`pJack of Clubs     -  J C`q\n",
    "`pQueen of Clubs    -  Q C`q\n",
    "`pKing of Clubs     -  K C`q\n",
    "`iAce of Hearts     -  A H`q\n",
    "`iDeuce of Hearts   -  2 H`q\n",
    "`iThree of Hearts   -  3 H`q\n",
    "`iFour of Hearts    -  4 H`q\n",
    "`iFive of Hearts    -  5 H`q\n",
    "`iSix of Hearts     -  6 H`q\n",
    "`iSeven of Hearts   -  7 H`q\n",
    "`iEight of Hearts   -  8 H`q\n",
    "`iNine of Hearts    -  9 H`q\n",
    "`iTen of Hearts     - 10 H`q\n",
    "`iJack of Hearts    -  J H`q\n",
    "`iQueen of Hearts   -  Q H`q\n",
    "`iKing of Hearts    -  K H`q\n",
    "`iAce of Diamonds   -  A D`q\n",
    "`iDeuce of Diamonds -  2 D`q\n",
    "`iThree of Diamonds -  3 D`q\n",
    "`iFour of Diamonds  -  4 D`q\n",
    "`iFive of Diamonds  -  5 D`q\n",
    "`iSix of Diamonds   -  6 D`q\n",
    "`iSeven of Diamonds -  7 D`q\n",
    "`iEight of Diamonds -  8 D`q\n",
    "`iNine of Diamonds  -  9 D`q\n",
    "`iTen of Diamonds   - 10 D`q\n",
    "`iJack of Diamonds  -  J D`q\n",
    "`iQueen of Diamonds -  Q D`q\n",
    "`iKing of Diamonds  -  K D`q\n",
    "`pThe Joker !`q\n",
    "`iThe Joker !`q\n"
  };

  if(!ch) return FALSE;
  if(ch != obj->equipped_by) return FALSE;
  if(IS_NPC(ch)) return FALSE;

  if(ch==obj->equipped_by && cmd==CMD_UNKNOWN) {
    arg=one_argument(arg,buf);
    if(!*buf) return FALSE;

    if(is_abbrev(buf,"deal")) {
      for(i=0;i<52;i++) {
        tmp_obj=read_object(37,VIRTUAL);
        tmp_obj->action_description=str_dup(card[i]);
        obj_to_char(tmp_obj,ch);
      }
      act("\n\rYour hands blur the air, creating a standard deck of playing cards.\n\r",0,ch,0,0,TO_CHAR);
      act("\n\r$n's hands blur the air, creating a standard deck of playing cards.\n\r",0,ch,0,0,TO_ROOM);
      return TRUE;
    }

    if(is_abbrev(buf,"joker")) {
      for(i=52;i<54;i++) {
        tmp_obj=read_object(37,VIRTUAL);
        tmp_obj->action_description=str_dup(card[i]);
        obj_to_char(tmp_obj,ch);
      }
      act("\n\rYou juggle your hands in the air, creating two Jokers.\n\r",0,ch,0,0,TO_CHAR);
      act("\n\r$n juggles $s hands in the air, creating two Jokers.\n\r",0,ch,0,0,TO_ROOM);
      return TRUE;
    }

    if(!is_abbrev(buf,"shuffle")) return FALSE;
    top = count_carrying_obj(ch,37);
    if(top>0) {
      sprintf(buf,"\n\rYou begin to shuffle %d playing cards.\n\r",top);
      send_to_char(buf,ch);

      for(tmp=ch->carrying; tmp; tmp=next_tmp) {
        next_tmp=tmp->next_content;
        if(V_OBJ(tmp)==37) {
          obj_from_char(tmp);
          obj_to_room(tmp,CHAR_REAL_ROOM(ch));
        }
      }

      num_cards=top;
      for(i=1;i<=num_cards;i++) { /* 52 card pickup :P */
        num=(number(1,top));
        num2=0;
        for(tmp=world[CHAR_REAL_ROOM(ch)].contents; tmp; tmp=next_tmp) {
          next_tmp=tmp->next_content;
          if(V_OBJ(tmp)==37) {
            num2++;
            if(num==num2) {
              obj_from_room(tmp);
              obj_to_char(tmp,ch);
              top--;
              break;
            }/* end if numbers match */
          }/* end if obj is card */
        }/* end for loop checking room */
      }/* end for loop cardtop is over zero */
      send_to_char("\n\rCards Shuffled!\n\r",ch);
      act("\n\r$n shuffles the deck of cards like a pro.\n\r",0,ch,0,0,TO_ROOM);
      return TRUE;
    }/* end if top is over 0 */
    send_to_char("\n\rYou have no playing cards to shuffle.\n\r",ch);
    return TRUE;
  }/* end if command is unknown */
  return FALSE;
}


#define SLOT_MACHINE 32
#define DEALER_VISOR 38

void assign_gamble(void) {
  assign_obj(SLOT_MACHINE,slot_machine);
  assign_obj(LOTTERY_MACHINE,lottery_machine);
  assign_obj(DEALER_VISOR,dealer_visor);
}
