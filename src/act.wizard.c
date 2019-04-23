/**************************************************************************
*  file: actwiz.c , Implementation of commands.           Part of DIKUMUD *
*  Usage : Wizard Commands.                                               *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <limits.h>
#include <errno.h>

#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "cmd.h"
#include "spells.h"
#include "limits.h"
#include "utility.h"
#include "reception.h"
#include "act.h"
#include "modify.h"
#include "enchant.h"
#include "fight.h"
#include "spec.clan.h"
#include "shop.h"
#include "remortv2.h"

/* external functs */

#define MAX_STAT     25
extern char*   crypt __P((__const char *__key, __const char *__salt));
extern int insert_char_wizlist (struct char_data *ch);

void stop_riding(struct char_data *ch,struct char_data *vict);
void assign_mobiles(void); /* These 3 added by Ranger - July 96 */
void assign_objects(void);
void assign_rooms(void);
int str_cmp(char *arg1, char *arg2);
struct time_info_data age(struct char_data *ch);
int mana_limit(struct char_data *ch);
int hit_limit(struct char_data *ch);
int move_limit(struct char_data *ch);
int mana_gain(struct char_data *ch);
int hit_gain(struct char_data *ch);
int move_gain(struct char_data *ch);
int enchantment_special(struct enchantment_type_5 *enchantment,CHAR *mob,CHAR *ch,int cmd,char *arg);

void do_look(struct char_data *ch, char *argument, int cmd);
void dsearch(char *string, char *tmp);
char *string_to_lower(char *string);
long int exp_per_hour(int i);
int file_to_string(char *name, char *buf);

#define immortal_help \
"\n\rThis is the handbook for level 51 Immortal.\n\r\
(1) You can't stat objects or characters for mortal players or yourself.\n\r\
(2) You can't kill anything.\n\r\
(3) You can't get any equipment unless it is given to you.  Period.\n\r\
(4) You may not explore with your immortal.\n\r\
(5) You may not influence mortal play in any way except to run quests\n\r\
    and discipline mortals when appropriate.\n\r\
(6) You can't have your mortal players and immortal player on at the same time.\n\r\
(7) You may not snoop anyone unless they are acting suspiciously.\n\r\
(8) You may not locate equipment using any means.\n\r\n\r\
In general you may not use your immortal in any way that would give\n\r\
you an unfair advantage over mortal players.\n\r\n\r\
At this stage you should be figuring out what you can and want to do to help\n\r\
out, and learning the ropes of immorting.\n\r\
Hope you can have a good time in the process.\n\r\n\r"

#define demi_help \
"\n\rThis is the handbook for level 52 Demi Gods.\n\r\
(1) You can't stat objects or characters for mortal players or yourself.\n\r\
(2) You can't kill anything.\n\r\
(3) You can get equipment only with load or by having it given to you.\n\r\
(4) You may not influence mortal play in any way except to run quests,\n\r\
    discipline mortals, and give minor reimbursements when appropraite.\n\r\
(5) Don't give reimbursements for death traps or death.  If there are\n\r\
    extenuating circumstances its best to talk to a higher up.\n\r\
(6) You can't have your mortal players and immortal player on at the same time.\n\r\
(7) You may not snoop anyone unless they are acting suspiciously.\n\r\
(8) You may not explore with your immortal.\n\r\
(9) You may not locate equipment using any means.\n\r\n\r\
In general you may not use your immortal in any way that would give\n\r\
you an unfair advantage over mortal players.\n\r\n\r\
Now hopefully you have some idea where you want to go on this mud.\n\r\
You are a working class god now, so hop to it!\n\r\
Hope you can have a good time in the process.\n\r\n\r"

#define tem_help \
"\n\rThis is the handbook for level 53 Temporals.\n\r\
(1) You can't stat objects or characters for mortal players or yourself.\n\r\
(2) You can't kill anything.\n\r\
(3) You can get equipment only with load or by having it given to you.\n\r\
(4) You may not influence mortal play in any way except to run quests,\n\r\
    discipline mortals, and give minor reimbursements when appropraite.\n\r\
(5) Don't give reimbursements for death traps or death.  If there are\n\r\
    extenuating circumstances its best to talk to a higher up.\n\r\
(6) You can't have your mortal players and immortal player on at the same time.\n\r\
(7) You may not snoop anyone unless they are acting suspiciously.\n\r\
(8) You may not explore with your immortal.\n\r\
(9) You may not locate equipment using any means.\n\r\n\r\
In general you may not use your immortal in any way that would give\n\r\
you an unfair advantage over mortal players.\n\r\n\r\
Now you are more than a Deity and should be grooming yourself\n\r\
for a movement to Wizard.  You should now know where you want\n\r\
to go and how to get there.\n\r\
Hope you can have a good time in the process.\n\r\n\r"

#define wiz_help \
"\n\rThis is the handbook for level 54 Wizards.\n\r\
(1) You can't stat objects or characters for mortal players or yourself.\n\r\
(2) You can't kill anything.\n\r\
(3) You can get equipment only with load or by having it given to you.\n\r\
(4) You may not influence mortal play in any way except to run quests,\n\r\
    discipline mortals, and give minor reimbursements when appropriate.\n\r\
(5) Don't give reimbursements for death traps or death.  If there are\n\r\
    extenuating circumstances use your discretion or defer to a higher up\n\r\
    if one is present.\n\r\
(6) You may not snoop anyone unless they are acting suspiciously.\n\r\
(7) You may not explore with your immortal.\n\r\
(8) You may not locate equipment using any means.\n\r\n\r\
Now that you are a Wizard, you have less rules and may use your discretion.\n\r\
This is in a sense, the 'enforcer' level of god.  You should watch out\n\r\
for cheaters, both immortal and mortal.  You have greater powers to enable\n\r\
you to deal with them, but in the case of something serious leave a message\n\r\
to higher gods.\n\r\
Hope you can have a good time here.\n\r\n\r"

void do_handbook(struct char_data *ch, char *argument, int cmd) {
  char buf[MAX_INPUT_LENGTH];
  int level;

  if (IS_NPC(ch)) return;
  one_argument(argument, buf);

  if (!*buf) level=GET_LEVEL(ch);
  else {
    if(isdigit(*buf)) level = atoi(buf);
    else level = GET_LEVEL(ch);
  }

  level=MAX(level,LEVEL_IMM);
  level=MIN(level,GET_LEVEL(ch));

  if (level > LEVEL_WIZ) {
    send_to_char("This is only for level 51 to 54 immortals!\n\rUse handbook <level>.\n\r", ch);
    return;
  }

  if (level < LEVEL_DEI) {
    send_to_char(immortal_help, ch);
    return;
  }

  if (level < LEVEL_TEM) {
    send_to_char(demi_help, ch);
    return;
  }

  if (level < LEVEL_WIZ) {
    send_to_char(tem_help, ch);
    return;
  }

  if (level < LEVEL_SUP) {
    send_to_char(wiz_help, ch);
    return;
  }
}

extern char str_cut(char *source, char *dest, int number);

int check_god_access(CHAR *ch, int active) {

  if(IS_NPC(ch)) return FALSE;

  if (!IS_SET(ch->new.imm_flags, WIZ_TRUST)) {
    send_to_char("You need a Trust flag to do that!\n\r", ch);
    return FALSE;
  }

  if (IS_SET(ch->new.imm_flags, WIZ_FREEZE)) {
    send_to_char("You are Frozen, so you can't do it!\n\r", ch);
    return FALSE;
  }

  if(CHAOSMODE && !IS_SET(ch->new.imm_flags, WIZ_CHAOS)) {
    send_to_char("This is chaos! You aren't permitted to do very much.\n\r",ch);
    return FALSE;
  }

  if (active && !IS_SET(ch->new.imm_flags, WIZ_ACTIVE)) {
    send_to_char("You need an active flag for this command.\n\r", ch);
    return FALSE;
  }

  return TRUE;
}

int zone_locked(CHAR *ch, int zone) {
  if(zone_table[real_zone(zone)].reset_mode!=4) return FALSE;
  if(GET_LEVEL(ch)>LEVEL_WIZ) return FALSE;
  if(isname(GET_NAME(ch), zone_table[real_zone(zone)].name)) return FALSE;
  if(strstr(zone_table[real_zone(zone)].creators,GET_NAME(ch))) return FALSE;
  send_to_char("Sorry, zone is locked.\n\r",ch);
  return TRUE;
}

void do_warn(CHAR *ch, char *argument, int cmd) {
  struct tm *timeStruct;
  FILE *fl;
  struct program_info dtail;
  char usage_text[] = "\
Usage: `kwarn`q <name> [warning] or\n\r\
       `kwarn`q <search word>or\n\r\
       `kwarn`q <num> (last # of warns).\n\r";
  char buf[MAX_INPUT_LENGTH];
  char name[MAX_INPUT_LENGTH],warnings[MAX_INPUT_LENGTH];
  unsigned int num=0;
  long ct;
  if(!check_god_access(ch,FALSE)) return;

  half_chop(argument,name,MIL,warnings,MIL);

  if(!*name) {
    send_to_char(usage_text,ch);
    return;
  }

  /*string_to_lower(name);*/

  if (*warnings) {
    fl = fopen("warnings.log", "a");
    if(fl==0) {
      send_to_char("Unable to open file!!\n\r",ch);
      return;
    }
    ct=time(0);
    timeStruct = localtime(&ct);
    num=1900+timeStruct->tm_year;
    sprintf(buf,"(%s) %s %2d %d %s %s", GET_NAME(ch),Month[timeStruct->tm_mon],
            timeStruct->tm_mday,num,CAP(name),warnings);
    if(strlen(buf)>180) {
      send_to_char("Warning too long -- try again!!\n\r",ch);
      fclose(fl);
      return;
    }
    strcat(buf,"\n");
    fputs(buf, fl);
    send_to_char("Warning logged.\n\r",ch);
    fclose(fl);
    return;
  }

  if(GET_LEVEL(ch)<LEVEL_DEI) {
    send_to_char("Warning logs can only be viewed by DEI+\n\r",ch);
    return;
  }

  if(is_number(name)) {
    num=atoi(name);
    if(num<1) {
      send_to_char(usage_text,ch);
      return;
    }
    if(num>50) {
      send_to_char("Currently limiting to 50 warns.\n\r",ch);
      return;
    }
    dtail.args[0]=strdup("tail");
    sprintf(buf,"-%d",num);
    dtail.args[1]=strdup(buf);
    dtail.args[2]=strdup("warnings.log");
    dtail.args[3]=NULL;
    dtail.input=NULL;
    dtail.timeout=10;
    dtail.name=strdup("warnlast");
    add_program(dtail, ch);
  }
  else {
    dtail.args[0]=strdup("grep");
    dtail.args[1]=strdup("-i"); /* was =NULL; and set to [3] */
    dtail.args[2]=strdup(name);
    dtail.args[3]=strdup("warnings.log");
    dtail.input=NULL;
    dtail.timeout=10;
    dtail.name=strdup("warn");
    add_program(dtail, ch);
  }
}

void do_wizlog(struct char_data *ch, char *argument, int cmd)
{
  int num;
  char buf[MAX_INPUT_LENGTH];

  if(!check_god_access(ch,FALSE)) return;

  argument = one_argument(argument, buf);

  if (!*buf) {
    send_to_char("Usage: wizlog <num>\n\r\n\r", ch);
    send_to_char(" 1 -> People connecting, leaving, losing link, new player\n\r", ch);
    send_to_char(" 2 -> Unused\n\r", ch);
    send_to_char(" 3 -> Renting, dying, not enough rent\n\r", ch);
    send_to_char(" 4 -> Mortal actions, quest logs (QSTINFO,PLRINFO)\n\r", ch);
    send_to_char(" 5 -> Immortal actions (WIZINFO)\n\r", ch);
    if (GET_LEVEL(ch) > LEVEL_IMM) {
      send_to_char(" 6 -> Special procedure, game logs (WIZINFO)\n\r", ch);
    }
    send_to_char("\n\rThe players you can see depends on your wizinfo level.\n\r", ch);
    return;
  }

  num = atoi(buf);
  if ((num >= 1) && (num <= 6)) {
    if (GET_LEVEL(ch) < LEVEL_DEI && num > 5)
      num = 99;
    switch(num) {
    case 1: if (IS_SET(ch->new.imm_flags, WIZ_LOG_ONE)) {
      send_to_char("Ok, you turned that OFF.\n\r", ch);
      REMOVE_BIT(ch->new.imm_flags, WIZ_LOG_ONE);
    } else {
      send_to_char("Ok, you turned that ON.\n\r", ch);
      SET_BIT(ch->new.imm_flags, WIZ_LOG_ONE);
    } break;
    case 2: if (IS_SET(ch->new.imm_flags, WIZ_LOG_TWO)) {
      send_to_char("Ok, you turned that OFF.\n\r", ch);
      REMOVE_BIT(ch->new.imm_flags, WIZ_LOG_TWO);
    } else {
      send_to_char("Ok, you turned that ON.\n\r", ch);
      SET_BIT(ch->new.imm_flags, WIZ_LOG_TWO);
    } break;
    case 3: if (IS_SET(ch->new.imm_flags, WIZ_LOG_THREE)) {
      send_to_char("Ok, you turned that OFF.\n\r", ch);
      REMOVE_BIT(ch->new.imm_flags, WIZ_LOG_THREE);
    } else {
      send_to_char("Ok, you turned that ON.\n\r", ch);
      SET_BIT(ch->new.imm_flags, WIZ_LOG_THREE);
    } break;
    case 4: if (IS_SET(ch->new.imm_flags, WIZ_LOG_FOUR)) {
      send_to_char("Ok, you turned that OFF.\n\r", ch);
      REMOVE_BIT(ch->new.imm_flags, WIZ_LOG_FOUR);
    } else {
      send_to_char("Ok, you turned that ON.\n\r", ch);
      SET_BIT(ch->new.imm_flags, WIZ_LOG_FOUR);
    } break;
    case 5: if (IS_SET(ch->new.imm_flags, WIZ_LOG_FIVE)) {
      send_to_char("Ok, you turned that OFF.\n\r", ch);
      REMOVE_BIT(ch->new.imm_flags, WIZ_LOG_FIVE);
    } else {
      send_to_char("Ok, you turned that ON.\n\r", ch);
      SET_BIT(ch->new.imm_flags, WIZ_LOG_FIVE);
    } break;
    case 6: if (IS_SET(ch->new.imm_flags, WIZ_LOG_SIX)) {
      send_to_char("Ok, you turned that OFF.\n\r", ch);
      REMOVE_BIT(ch->new.imm_flags, WIZ_LOG_SIX);
    } else {
      send_to_char("Ok, you turned that ON.\n\r", ch);
      SET_BIT(ch->new.imm_flags, WIZ_LOG_SIX);
    } break;
    default:
      if(GET_LEVEL(ch)>LEVEL_IMM) send_to_char("Use 1 to 6 please!\n\r", ch);
      else send_to_char("Use 1 to 5 please!\n\r", ch);
      break;
    }
  } else {
    send_to_char("Usage: wizlog <num 1 to 5/6>\n\r", ch);
    return;
  }
}

void do_setflag(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  struct obj_data *dummy;
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) return;

  if (IS_SET(ch->new.imm_flags, WIZ_FREEZE) && (GET_LEVEL(ch) < LEVEL_IMP)) {
    send_to_char("You are Freezed, so you can't do it!\n\r", ch);
    return;
  }

  argument = one_argument(argument, buf);
  argument = one_argument(argument, buf2);

  if ((!*buf)||(!*buf2)) {
    send_to_char("\
Flags: trust, load, nonet, quest, create, judge, active, chaos and severed.\n\r\
Usage: setflag <victim> <flag>\n\r", ch);
    return; /* Updated flag list - Ranger May 96 - Added active June 96 */
  }
  else {
    if (!generic_find(buf, FIND_CHAR_WORLD, ch, &vict, &dummy)) {
      send_to_char("Couldn't find any such creature.\n\r", ch);
      return;
    }
    if(IS_NPC(vict)) {
      send_to_char("Can't do that to a beast.\n\r", ch);
      return;
    }

    if (!strcmp(buf2, "trust")) {
      if(GET_LEVEL(vict) < LEVEL_IMM) {
        send_to_char("He/She is not immortal yet, don't worry!\n\r", ch);
        return;
      }
      if (IS_SET(vict->new.imm_flags, WIZ_TRUST)) {
        send_to_char("OK, you have removed the trust flag!\n\r", ch);
        REMOVE_BIT(vict->new.imm_flags, WIZ_TRUST);
      } else {
        send_to_char("OK, you have set the trust flag!\n\r", ch);
        SET_BIT(vict->new.imm_flags, WIZ_TRUST);
      }
    }
    else if (!strcmp(buf2, "load")) {
      if(GET_LEVEL(vict) < LEVEL_IMM) {
        send_to_char("He/She is not immortal yet, don't worry!\n\r", ch);
        return;
      }
      if (IS_SET(vict->new.imm_flags, WIZ_LOAD)) {
        send_to_char("OK, you have taken away LOAD ability from him/her!\n\r", ch);
        REMOVE_BIT(vict->new.imm_flags, WIZ_LOAD);
      } else {
        send_to_char("OK, you have given LOAD ability to him/her!\n\r", ch);
        SET_BIT(vict->new.imm_flags, WIZ_LOAD);
      }
    }
    else if (!strcmp(buf2, "nonet")) {
      if(GET_LEVEL(vict) < LEVEL_IMM) {
        send_to_char("He/She is not immortal yet, don't worry!\n\r", ch);
        return;
      }
      if (IS_SET(vict->new.imm_flags, WIZ_NO_NET)) {
        send_to_char("OK, He/She can wiznet once again!\n\r", ch);
        REMOVE_BIT(vict->new.imm_flags, WIZ_NO_NET);
      } else {
        send_to_char("OK, He/She can't wiznet anymore!\n\r", ch);
        SET_BIT(vict->new.imm_flags, WIZ_NO_NET);
      }
    }
    else if (!strcmp(buf2, "quest")) {
      if(GET_LEVEL(vict) < LEVEL_IMM) {
        send_to_char("He/She is not immortal yet, don't worry!\n\r", ch);
        return;
      }
      if (GET_LEVEL(ch) < LEVEL_SUP) {
        send_to_char("You cannot set the quest flag.\n\r", ch);
        return;
      }
      if (IS_SET(vict->new.imm_flags, WIZ_QUEST)) {
        send_to_char("OK, You have taken back the ability for she/he to do QUEST!\n\r", ch);
        REMOVE_BIT(vict->new.imm_flags, WIZ_QUEST);
      } else {
        send_to_char("OK, You have allowed he/she to do some QUESTS!\n\r", ch);
        SET_BIT(vict->new.imm_flags, WIZ_QUEST);
      }
    }
    else if (!strcmp(buf2, "severed")) {
      if (GET_LEVEL(ch) < LEVEL_IMP) {
        send_to_char("You cannot set the severed flag.\n\r", ch);
        return;
      }
      if (IS_SET(vict->specials.affected_by2,AFF2_SEVERED)) {
        send_to_char("Severed affect removed.\n\r", ch);
        REMOVE_BIT(vict->specials.affected_by2,AFF2_SEVERED);
      } else {
        send_to_char("Severed affect added.\n\r", ch);
        SET_BIT(vict->specials.affected_by2,AFF2_SEVERED);
      }
    }
    else if (!strcmp(buf2, "chaos")) {
      if(GET_LEVEL(vict) < LEVEL_IMM) {
        send_to_char("He/She is not immortal yet, don't worry!\n\r", ch);
        return;
      }
      if (GET_LEVEL(ch) < LEVEL_SUP) {
        send_to_char("You cannot set the chaos flag.\n\r", ch);
        return;
      }
      if (IS_SET(vict->new.imm_flags, WIZ_CHAOS)) {
        send_to_char("OK, You have taken back the chaos flag!\n\r", ch);
       REMOVE_BIT(vict->new.imm_flags, WIZ_CHAOS);
      } else {
        send_to_char("OK, You have given the chaos flag!\n\r", ch);
        SET_BIT(vict->new.imm_flags, WIZ_CHAOS);
      }
    }
    else if (!strcmp(buf2, "create")) {
      if(GET_LEVEL(vict) < LEVEL_IMM) {
        send_to_char("He/She is not immortal yet, don't worry!\n\r", ch);
        return;
      }
      if (GET_LEVEL(ch) < LEVEL_SUP) {
        send_to_char("You cannot set the create flag.\n\r", ch);
        return;
      }
      if (IS_SET(vict->new.imm_flags, WIZ_CREATE)) {
        send_to_char("OK, You have taken back the ability for her/him to create!\n\r", ch);
        REMOVE_BIT(vict->new.imm_flags, WIZ_CREATE);
      } else {
        send_to_char("OK, You have allowed her/him to do create!\n\r", ch);
        SET_BIT(vict->new.imm_flags, WIZ_CREATE);
      }
    }

    /* Addition of Judge flag - Ranger May 96 */
    else if (!strcmp(buf2, "judge")) {
      if(GET_LEVEL(vict) < LEVEL_IMM) {
        send_to_char("He/She is not immortal yet, don't worry!\n\r", ch);
        return;
      }
      if (GET_LEVEL(ch) < LEVEL_SUP) {
        send_to_char("You cannot set the judge flag.\n\r", ch);
        return;
      }
      if (IS_SET(vict->new.imm_flags, WIZ_JUDGE)) {
        send_to_char("OK, You have taken back the ability for her/him to judge!\n\r", ch);
        REMOVE_BIT(vict->new.imm_flags, WIZ_JUDGE);
      } else {
        send_to_char("OK, You have allowed her/him to judge!\n\r", ch);
        SET_BIT(vict->new.imm_flags, WIZ_JUDGE);
      }
    }

    /* Addition of Active flag - Ranger June 96 */
    else if (!strcmp(buf2, "active")) {
      if(GET_LEVEL(vict) < LEVEL_IMM) {
        send_to_char("He/She is not immortal yet, don't worry!\n\r", ch);
        return;
      }

      if (GET_LEVEL(vict) > LEVEL_SUP &&
         (strcmp(GET_NAME(ch), "Lem") && strcmp(GET_NAME(ch), "Ranger") &&
          strcmp(GET_NAME(ch), "Sumo") && strcmp(GET_NAME(ch), "Liner") &&
          strcmp(GET_NAME(ch), "Sane") && strcmp(GET_NAME(ch), "Shun") &&
          strcmp(GET_NAME(ch), "Night"))) {
        send_to_char("You cannot set an IMP active.\n\r", ch);
        return;
      }

      if(GET_LEVEL(vict)>GET_LEVEL(ch)) {
        send_to_char("You cannot set the active flag on a player of a higher level.\n\r",ch);
        return;
      }

      if (IS_SET(vict->new.imm_flags, WIZ_ACTIVE)) {
        send_to_char("OK, You have removed the active flag!\n\r", ch);
        REMOVE_BIT(vict->new.imm_flags, WIZ_ACTIVE);
        insert_char_wizlist (vict);
      } else {
        if(vict==ch && GET_LEVEL(ch)<LEVEL_IMP) {
          send_to_char("You cannot set your own active flag.\n\r",ch);
          return;
        }
        send_to_char("OK, You have added the active flag!\n\r", ch);
        SET_BIT(vict->new.imm_flags, WIZ_ACTIVE);
        insert_char_wizlist (vict);
      }
    }

    else {
      send_to_char("What flag?\n\r", ch);
      return;
    }
  }

}

/* subclass restriction and material not a part of sos yet */
void do_setobjstat(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  struct obj_data *obj;
  int i = 0, num = 0, num2 = 0, tmp =0, which = 0;
  unsigned long int bitv;

  if(!check_god_access(ch,TRUE)) return;

  sprintf(buf, "WIZINFO: (%s) change object %s", GET_NAME(ch), argument);
  wizlog(buf, GET_LEVEL(ch)+1, 5);
  log_s(buf);

  argument=one_argument(argument, buf);

  if (*buf) {
    if (!(obj=get_obj_in_list_vis(ch, buf, ch->carrying))) {
      send_to_char("You don't seem to have anything like that.\n\r", ch);
      return;
    }

    argument=one_argument(argument, buf);
    argument=one_argument(argument, buf2);

    if (!*buf || !*buf2) {
      send_to_char("Where are the third and fourth argument please?\n\r", ch);
      return;
    }

    if (isdigit(*buf)) num = atoi(buf);
    else if(is_abbrev(buf,"type"))     num=1;
    else if(is_abbrev(buf,"extra"))    num=2;
    else if(is_abbrev(buf,"wear"))     num=3;
    else if(is_abbrev(buf,"value0"))   num=4;
    else if(is_abbrev(buf,"value1"))   num=5;
    else if(is_abbrev(buf,"value2"))   num=6;
    else if(is_abbrev(buf,"value3"))   num=7;
    else if(is_abbrev(buf,"weight"))   num=8;
    else if(is_abbrev(buf,"cost"))     num=9;
    else if(is_abbrev(buf,"cost_day")) num=10;
    else if(is_abbrev(buf,"affected")) num=11;
    else if(is_abbrev(buf,"owner"))    num=12;
    else if(is_abbrev(buf,"timer"))    num=13;
    else if(is_abbrev(buf,"popped"))   num=14;
    else if(is_abbrev(buf,"specvalue")) num=15;

    if (is_number(buf2))
      num2 = atoi(buf2);
    else {
      if((num!=2) && (num!=3) && (num !=11) && (num !=12)) {
        send_to_char("The fourth argument must be an integer!\n\r", ch);
        return;
      }
    }

    if(num>0) {
      switch (num) {
        case 1:
          obj->obj_flags.type_flag = num2;
          break;

        case 2:
          if(is_number(buf2)) {
            num2=atoi(buf2);
            obj->obj_flags.extra_flags=num2;
          }
          else {
            tmp = old_search_block(string_to_upper(buf2), 0, strlen(buf2), extra_bits, FALSE);

            if(tmp == -1) {
              tmp = old_search_block(string_to_upper(buf2), 0, strlen(buf2), extra_bits2, FALSE);

              if(tmp == -1) {
                send_to_char("Extra flag not found.\n\r",ch);
                return;
              }

              which=1;
            }

            bitv=1<<(tmp-1);

            if(!which) {
              if(IS_SET(obj->obj_flags.extra_flags,bitv)) REMOVE_BIT(obj->obj_flags.extra_flags,bitv);
              else SET_BIT(obj->obj_flags.extra_flags,bitv);
            }
            else {
              if(IS_SET(obj->obj_flags.extra_flags2,bitv)) REMOVE_BIT(obj->obj_flags.extra_flags2,bitv);
              else SET_BIT(obj->obj_flags.extra_flags2,bitv);
            }
          }
          break;

        case 3:
          if(is_number(buf2)) {
            num2=atoi(buf2);
            obj->obj_flags.wear_flags=num2;
          }
          else {
            tmp = old_search_block(string_to_upper(buf2), 0, strlen(buf2), wear_bits, FALSE);

            if(tmp == -1) {
             send_to_char("Wear flag not found.\n\r",ch);
             return;
            }

            bitv=1<<(tmp-1);

            if(IS_SET(obj->obj_flags.wear_flags,bitv)) REMOVE_BIT(obj->obj_flags.wear_flags,bitv);
            else SET_BIT(obj->obj_flags.wear_flags,bitv);
          }
          break;

        case 4:
          obj->obj_flags.value[0] = num2;
          break;

        case 5:
          obj->obj_flags.value[1] = num2;
          break;

        case 6:
          obj->obj_flags.value[2] = num2;
          break;

        case 7:
          obj->obj_flags.value[3] = num2;
          break;

        case 8:
          obj->obj_flags.weight = num2;
          break;

        case 9:
          obj->obj_flags.cost = num2;
          break;

        case 10:
          obj->obj_flags.cost_per_day = num2;
          break;

        case 11:
          if(is_number(buf2)) { /* Currently can only add bitvector as a number, must add bitvector2 as a name */
            num2=atoi(buf2);
            obj->obj_flags.bitvector=num2;
          }
          else {
            tmp = old_search_block(string_to_upper(buf2), 0, strlen(buf2), affected_bits, FALSE);

            if(tmp == -1) {
              tmp = old_search_block(string_to_upper(buf2), 0, strlen(buf2), affected_bits2, FALSE);

              if(tmp == -1) {
                send_to_char("Affect flag not found.\n\r",ch);
                return;
              }

              which=1;
            }

            bitv=1<<(tmp-1);

            if(!which) {
              if(IS_SET(obj->obj_flags.bitvector,bitv)) REMOVE_BIT(obj->obj_flags.bitvector,bitv);
              else SET_BIT(obj->obj_flags.bitvector,bitv);
            }
            else {
              if(IS_SET(obj->obj_flags.bitvector2,bitv)) REMOVE_BIT(obj->obj_flags.bitvector2,bitv);
              else SET_BIT(obj->obj_flags.bitvector2,bitv);
            }
          }
          break;

        case 12: /* owner*/
          if (!strcmp("0", buf2)) {// reset to no owners
            for(i=0;i<8;i++) {
              obj->ownerid[i] = 0;
            }
          }
          else {
            for(i=0;i<8;i++) {
              obj->ownerid[i]=0;
              if(!*buf2) continue;
              string_to_lower(buf2);
              for(tmp=0;tmp<MAX_ID;tmp++) {
                if(!strcasecmp(idname[tmp].name,buf2)) {
                  obj->ownerid[i]=tmp;
                  break;
                }
              }
              argument=one_argument(argument, buf2);
            }
          }
          send_to_char("Owners set/reset.  To individualize the obj for wear, be sure to set wear flag QUESTWEAR.\n\r",ch);
          break;

        case 13: // timer
          obj->obj_flags.timer = num2;
          break;

        case 14: // popped
          obj->obj_flags.popped = num2;
          break;

        case 15: // specvalue
          obj->spec_value = num2;
          break;

        default:
          send_to_char("Wrong! Wrong! Wrong! Type wizhelp sos.\n\r", ch);
          return;
          break;
      }
    }
    else if (*buf) {
      argument=one_argument(argument, buf2);
      if (isdigit(*buf2)) {
        i = atoi(buf2);
        if(i<0 || i>2) {
          send_to_char("The fifth argument must be 0, 1 or 2.\n\r",ch);
          return;
        }
      }
      else {
        send_to_char("For this case, the fifth argument must be 0 or 1.\n\r", ch);
        return;
      }

      if (!strcmp(buf, "str")) {
        obj->affected[i].location = APPLY_STR;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "dex")) {
        obj->affected[i].location = APPLY_DEX;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "int")) {
        obj->affected[i].location = APPLY_INT;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "wis")) {
        obj->affected[i].location = APPLY_WIS;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "con")) {
        obj->affected[i].location = APPLY_CON;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "age")) {
        obj->affected[i].location = APPLY_AGE;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "weight")) {
        obj->affected[i].location = APPLY_CHAR_WEIGHT;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "height")) {
        obj->affected[i].location = APPLY_CHAR_HEIGHT;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "mana")) {
        obj->affected[i].location = APPLY_MANA;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "hp")) {
        obj->affected[i].location = APPLY_HIT;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "move")) {
        obj->affected[i].location = APPLY_MOVE;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "ac") || !strcmp(buf, "armor")) {
        obj->affected[i].location = APPLY_AC;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "hit")) {
        obj->affected[i].location = APPLY_HITROLL;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "dam")) {
        obj->affected[i].location = APPLY_DAMROLL;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "mana_regen")) {
        obj->affected[i].location = APPLY_MANA_REGEN;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "hp_regen")) {
        obj->affected[i].location = APPLY_HP_REGEN;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "assault")) {
        obj->affected[i].location = APPLY_SKILL_ASSAULT;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "backflip")) {
        obj->affected[i].location = APPLY_SKILL_BACKFLIP;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "backstab")) {
        obj->affected[i].location = APPLY_SKILL_BACKSTAB;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "bash")) {
        obj->affected[i].location = APPLY_SKILL_BASH;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "block")) {
        obj->affected[i].location = APPLY_SKILL_BLOCK;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "circle")) {
        obj->affected[i].location = APPLY_SKILL_CIRCLE;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "dodge")) {
        obj->affected[i].location = APPLY_SKILL_DODGE;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "dual")) {
        obj->affected[i].location = APPLY_SKILL_DUAL;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "parry")) {
        obj->affected[i].location = APPLY_SKILL_PARRY;
        obj->affected[i].modifier = num2;
      }
      else if (!strcmp(buf, "triple")) {
        obj->affected[i].location = APPLY_SKILL_TRIPLE;
        obj->affected[i].modifier = num2;
      }
      else {
        send_to_char("Sorry not on that field!\n\r", ch);
        return;
      }
    }
    else {
      send_to_char("I don't understand your third argument!\n\r", ch);
      return;
    }
  }
  else {
    send_to_char("On what object? Type wizhelp sos.\n\r", ch);
    return;
  }

  act("$n has retransformed $p!", FALSE, ch, obj, 0, TO_ROOM);
  act("You have retransformed $p!", FALSE, ch, obj, 0, TO_CHAR);

}

void do_uptime(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  int hr=0, min=0, temp=0;

  if (IS_NPC(ch)) return;

  temp = (time(0)-uptime);

  if (temp > 59) {
    temp = (temp/60);
    min = temp;
  }

  if (min > 59) {
    hr = (min/60);
    min -= (hr*60);
  }

  sprintf(buf, "The mud has been up for %d hours %d mins without a crash.\n\r",hr,min);
  send_to_char(buf, ch);

}

int change_password(struct char_data *ch, char *name, char* newpassword) {
  char pass[11];
  int version;
  struct char_file_u_5 char_info_5;
  struct char_file_u_4 char_info_4;
  struct char_file_u_2 char_info_2;
  struct descriptor_data *d;
  FILE *fl;
  int i;
  char tmp_name[50];
  char buf[255];

  if(!*newpassword || strlen(newpassword) > 10) {
    send_to_char("Illegal password.\n\r", ch);
    return(0);
  }
  strncpy(pass, (char*)crypt(newpassword, name), 10);
  *(pass+10) = '\0';

  strcpy(tmp_name,GET_NAME(ch));
  if (!strcmp(string_to_lower(tmp_name),name)) {
    strcpy(ch->pwd, pass);
    save_char(ch,NOWHERE);
    return(TRUE);
  }
  for (d=descriptor_list;d;d=d->next)  {
    if(d->character)
     if(GET_NAME(d->character)) strcpy(tmp_name,GET_NAME(d->character));
        else tmp_name[0]= 0;
    else tmp_name[0]=0;
    if (!strcmp(string_to_lower(tmp_name),name) && !IS_NPC(ch)) break;
  }
  if(d) {
    strcpy(d->pwd, pass);
    strcpy(d->character->pwd, pass);
    save_char(d->character, NOWHERE);
    return(TRUE);
  }
  sprintf(tmp_name,"%s",name);
  for (i=0;i<strlen(tmp_name);i++) {
    tmp_name[i] = LOWER(tmp_name[i]);
  }

  sprintf(buf,"rent/%c/%s.dat",UPPER(tmp_name[0]),tmp_name);
  if(!(fl=fopen(buf,"rb"))) {
    sprintf(buf,"Error--can't open file %s for updating pwd (read)", buf);
    log_s(buf);
    return(FALSE);
  }

  version=char_version(fl);

  switch(version) {
    case 2:
      memset(&char_info_2,0,sizeof(char_info_2));
      if (fread(&char_info_2, sizeof(struct char_file_u_2), 1, fl) < 1) {
        log_s("Error reading rent file (change_password).");
        produce_core();
      }
      fclose(fl);
      strcpy((char*)&char_info_2.pwd, pass);
      sprintf(buf,"rent/%c/%s.dat",UPPER(tmp_name[0]),tmp_name);
      if(!(fl=fopen(buf,"r+b"))) {
        sprintf(buf,"Error--can't open file %s for updating pwd (write)", buf);
        log_s(buf);
        return(FALSE);
      }
      if (fwrite(&char_info_2, sizeof(struct char_file_u_2), 1, fl) < 1) {
        log_s("Error writing rent file (change_password).");
        produce_core();
      }
      break;
    case 3:
    case 4:
      memset(&char_info_4,0,sizeof(char_info_4));
      if (fread(&char_info_4, sizeof(struct char_file_u_4), 1, fl) < 1) {
        log_s("Error reading rent file (change_password).");
        produce_core();
      }
      fclose(fl);
      strcpy((char*)&char_info_4.pwd, pass);
      sprintf(buf,"rent/%c/%s.dat",UPPER(tmp_name[0]),tmp_name);
      if(!(fl=fopen(buf,"r+b"))) {
        sprintf(buf,"Error--can't open file %s for updating pwd (write)", buf);
        log_s(buf);
        return(FALSE);
      }
      if (fwrite(&char_info_4, sizeof(struct char_file_u_4), 1, fl) < 1) {
        log_s("Error writing rent file (change_password).");
        produce_core();
      }
      break;
    case 5:
      memset(&char_info_5,0,sizeof(char_info_5));
      if (fread(&char_info_5, sizeof(struct char_file_u_5), 1, fl) < 1) {
        log_s("Error reading rent file (change_password).");
        produce_core();
      }
      fclose(fl);
      strcpy((char*)&char_info_5.pwd, pass);
      sprintf(buf,"rent/%c/%s.dat",UPPER(tmp_name[0]),tmp_name);
      if(!(fl=fopen(buf,"r+b"))) {
        sprintf(buf,"Error--can't open file %s for updating pwd (write)", buf);
        log_s(buf);
        return(FALSE);
      }
      if (fwrite(&char_info_5, sizeof(struct char_file_u_5), 1, fl) < 1) {
        log_s("Error writing rent file (change_password).");
        produce_core();
      }
      break;
    default:
      log_s("Error getting pfile version (change_pwd)");
  }
  fclose(fl);
  return(TRUE);
}

void do_password(struct char_data *ch, char *argument, int cmdnum)
{
  char name[30], npasswd[20], oldpasswd[20],buf[MAX_INPUT_LENGTH];

  if(IS_NPC(ch))  /* NPC can NOT change their password */
    return;

  switch(GET_LEVEL(ch)) {
    case LEVEL_SUP: {
         if(!check_god_access(ch,TRUE)) return;

         if(!strcmp(GET_NAME(ch),"Nosferatu")) {
           argument = one_argument(argument, name);
           string_to_lower(name);
           if (!strcmp(name, "ranger") || !strcmp(name, "sumo") || !strcmp(name, "lem") || !strcmp(name, "liner") || !strcmp(name, "sane") || !strcmp(name, "shun") || !strcmp(name, "night")) {
             send_to_char("You cannot change the password of the active IMPs.\n\r", ch);
             return;
           }
         } else {
           sprintf(name,"%s",ch->player.name);
           argument = one_argument(argument, oldpasswd);
           if(strncmp((char*)crypt(oldpasswd,ch->pwd),ch->pwd,10)) {
             send_to_char("Incorrect password.\n\r",ch);
             return;
           }
         }
         break;
    }
    case LEVEL_IMP: {
         if(!check_god_access(ch,TRUE)) return;
         argument = one_argument(argument, name);
         string_to_lower(name);
         if ((strcmp(GET_NAME(ch), "Ranger") && !strcmp(name, "ranger")) ||
           (strcmp(GET_NAME(ch), "Sumo") && !strcmp(name, "sumo")) ||
           (strcmp(GET_NAME(ch), "Lem") && !strcmp(name, "lem")) ||
           (strcmp(GET_NAME(ch), "Liner") && !strcmp(name, "liner")) ||
           (strcmp(GET_NAME(ch), "Sane") && !strcmp(name, "sane")) ||
           (strcmp(GET_NAME(ch), "Shun") && !strcmp(name, "shun")) ||
           (strcmp(GET_NAME(ch), "Night") && !strcmp(name, "night"))) {
           send_to_char("You cannot change the password of the active IMPs.\n\r", ch);
           return;
         }
         break;
    }
    default: {
      sprintf(name,"%s",ch->player.name);
         argument = one_argument(argument, oldpasswd);
         if(strncmp((char*)crypt(oldpasswd,ch->pwd),ch->pwd,10)) {
           send_to_char("Incorrect password.\n\r",ch);
           return;
         }
         break;
    }
  }
  argument = one_argument(argument, npasswd);

  if (!change_password(ch,string_to_lower(name), npasswd)) {
    send_to_char("Password was not changed.\n\r",ch);
  }
  else {
    if(GET_LEVEL(ch)>=LEVEL_SUP) {
      sprintf(buf,"WIZINFO: %s changed %s's password.",GET_NAME(ch),name);
      log_s(buf);
    }
    send_to_char("Done.\n\r",ch);
  }
}

void do_plock(struct char_data *ch, char *argument, int cmd)
{
  char name[30],buf[MAX_INPUT_LENGTH],pass[11];
  int version;
  struct char_file_u_5 char_info_5;
  struct char_file_u_4 char_info_4;
  struct char_file_u_2 char_info_2;
  struct descriptor_data *d;
  FILE *fl;
  char tmp_name[50];

  if(IS_NPC(ch)) return;
  if(!check_god_access(ch,TRUE)) return;

  argument = one_argument(argument, name);
  if(!*name) {
    send_to_char("Who do you want to lock?\n\r",ch);
    return;
  }
  string_to_lower(name);

  if (!strcmp(name, "ranger") || !strcmp(name, "sumo") || !strcmp(name, "lem") || !strcmp(name, "liner") || !strcmp(name, "sane") || !strcmp(name, "shun") || !strcmp(name, "night")) {
    send_to_char("You cannot lock the active IMPs.\n\r", ch);
    return;
  }

  strncpy(pass, (char*)crypt(ch->pwd, "9#S2*ePha_"), 10);
  *(pass+10) = '\0';

  for (d=descriptor_list;d;d=d->next)  {
    if(d->character)
     if(GET_NAME(d->character)) strcpy(tmp_name,GET_NAME(d->character));
     else tmp_name[0]= 0;
    else tmp_name[0]=0;
    if (!strcmp(string_to_lower(tmp_name),name) && !IS_NPC(ch)) break;
  }
  if(d) {
    strcpy(d->pwd, pass);
    strcpy(d->character->pwd, pass);
    save_char(d->character, NOWHERE);
    log_f("WIZINFO: %s locked %s.",GET_NAME(ch),CAP(name));
    send_to_char("Done.\n\r",ch);
    return;
  }


  sprintf(buf,"rent/%c/%s.dat",UPPER(name[0]),name);
  if(!(fl=fopen(buf,"rb"))) {
    log_f("Error--can't open file %s for updating pwd (read)", buf);
    send_to_char("There was an error running this command.\n\r",ch);
    return;
  }

  version=char_version(fl);

  switch(version) {
    case 2:
      memset(&char_info_2,0,sizeof(char_info_2));
      if (fread(&char_info_2, sizeof(struct char_file_u_2), 1, fl) < 1) {
        log_s("Error reading rent file (lock).");
        produce_core();
      }
      fclose(fl);
      strcpy((char*)&char_info_2.pwd, pass);
      sprintf(buf,"rent/%c/%s.dat",UPPER(name[0]),name);
      if(!(fl=fopen(buf,"r+b"))) {
        log_f("Error--can't open file %s for updating pwd (lock)", buf);
        send_to_char("There was an error running this command.\n\r",ch);
        return;
      }
      if (fwrite(&char_info_2, sizeof(struct char_file_u_2), 1, fl) < 1) {
        log_s("Error writing rent file (lock).");
        produce_core();
      }
      break;
    case 3:
    case 4:
      memset(&char_info_4,0,sizeof(char_info_4));
      if (fread(&char_info_4, sizeof(struct char_file_u_4), 1, fl) < 1) {
        log_s("Error reading rent file (lock).");
        produce_core();
      }
      fclose(fl);
      strcpy((char*)&char_info_4.pwd, pass);
      sprintf(buf,"rent/%c/%s.dat",UPPER(name[0]),name);
      if(!(fl=fopen(buf,"r+b"))) {
        log_f("Error--can't open file %s for updating pwd (lock)", buf);
        send_to_char("There was an error running this command.\n\r",ch);
        return;
      }
      if (fwrite(&char_info_4, sizeof(struct char_file_u_4), 1, fl) < 1) {
        log_s("Error writing rent file (lock).");
        produce_core();
      }
      break;
    case 5:
      memset(&char_info_5,0,sizeof(char_info_5));
      if (fread(&char_info_5, sizeof(struct char_file_u_5), 1, fl) < 1) {
        log_s("Error reading rent file (lock).");
        produce_core();
      }
      fclose(fl);
      strcpy((char*)&char_info_5.pwd, pass);
      sprintf(buf,"rent/%c/%s.dat",UPPER(name[0]),name);
      if(!(fl=fopen(buf,"r+b"))) {
        log_f("Error--can't open file %s for updating pwd (lock)", buf);
        send_to_char("There was an error running this command.\n\r",ch);
        return;
      }
      if (fwrite(&char_info_5, sizeof(struct char_file_u_5), 1, fl) < 1) {
        log_s("Error writing rent file (lock).");
        produce_core();
      }
      break;
    default:
      log_s("Error getting pfile version (change_pwd)");
      send_to_char("There was an error running this command.\n\r",ch);
      return;
  }
  fclose(fl);

  log_f("WIZINFO: %s locked %s.",GET_NAME(ch),CAP(name));
  send_to_char("Done.\n\r",ch);
}

void do_waitlist(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH], line[200];

  struct descriptor_data *d;

  strcpy(buf, "The following players are waiting for 'allowin' :\n\r\n\r");

  for (d = descriptor_list; d; d = d->next)
    {
      if(d->character)
     if (GET_LEVEL(d->character) == 0)
       {
         if (d->character && d->character->player.name) {
           if (d->original)
          sprintf(line, "%-16s: ", d->original->player.name);
           else
          sprintf(line, "%-16s: ", d->character->player.name);
         }
         else
           strcpy(line, "UNDEFINED       : ");
         if (d->host)
           sprintf(line + strlen(line), "[%s]\n\r", d->host);
         else
           strcat(line, "[Hostname unknown]\n\r");

         strcat(buf, line);
       }
    }
  send_to_char(buf, ch);
}

void do_allowin(struct char_data *ch, char *argument, int cmd)
{
  char name[50];
  struct descriptor_data *d;
  int done=FALSE;

  if(!check_god_access(ch,FALSE)) return;

  one_argument(argument, name);

  for (d=descriptor_list; d && !done; d = d->next) {
    if (d->character)
      if (GET_NAME(d->character) &&
       (str_cmp(GET_NAME(d->character), name) == 0))  {
     done = TRUE;
     break;
      }
  }

  if (!d) {
    send_to_char("That player was not found.\n\r", ch);
    return;
  }

  d->character->player.hometown = 9;
  send_to_char("Done.\n\r", ch);
  SEND_TO_Q("\n\r \n\rYou are allowed in join us now!\n\r", d);
  SEND_TO_Q("*** PRESS RETURN:", d);
}

void do_bamfin(struct char_data *ch, char *arg, int cmd)
{
  char buf[255];
  int len;

  if(!check_god_access(ch,FALSE)) return;

  if(!*arg) {
    send_to_char("Bamfin <bamf definition>\n\r", ch);
    send_to_char(" Additional Arguments: \n\r", ch);
    send_to_char("    *n for your name (or type your name)\n\r",ch);
    send_to_char("    *s for his or her (or type his/her)\n\r",ch);
    send_to_char("    *e for he or she (or type he/she)\n\r",ch);
    send_to_char("    Keyword 'def' turns on the default bamfin\n\r", ch);
    send_to_char("    Keyword 'show' shows the current bamfin\n\r", ch);
    return;
  } else {
    arg++;
  }

  if(!strcmp(arg, "def")) {
    free(ch->player.poofin);
    ch->player.poofin = 0;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if(!strcmp(arg, "show")) {
    if (!ch->player.poofin)
      act("$n appears with an ear-splitting bang.", TRUE, ch, 0,0,TO_CHAR);
    else
      act(ch->player.poofin, TRUE, ch, 0,0,TO_CHAR);
    return;
  }

  len = strlen(arg);

  if(len > 149) {
    send_to_char("String too long.  Truncated to:\n\r", ch);
    arg[148] = '\0';
    sprintf(buf, "%s\n\r", arg);
    send_to_char(buf, ch);
    len = 149;
  }

  if (ch->player.poofin && len >= strlen(ch->player.poofin)) {
    free(ch->player.poofin);
    ch->player.poofin = (char *)malloc(len+1);
  } else {
    if (!ch->player.poofin)
      ch->player.poofin = (char *)malloc(len+1);
  }

  strcpy(buf, arg);
  dsearch(buf, ch->player.poofin);
  send_to_char("Ok.\n\r", ch);
  return;
}

void do_bamfout(struct char_data *ch, char *arg, int cmd)
{
  char buf[255];
  int len;

  if(!check_god_access(ch,FALSE)) return;

  if(!*arg) {
    send_to_char("Bamfout <bamf definition>\n\r", ch);
    send_to_char(" Additional Arguments: \n\r", ch);
    send_to_char("    *n for your name (or type your name)\n\r",ch);
    send_to_char("    *s for his or her (or type his/her)\n\r",ch);
    send_to_char("    *e for he or she (or type he/she)\n\r",ch);
    send_to_char("    Keyword 'def' turns on the default bamfout\n\r", ch);
    send_to_char("    Keyword 'show' shows the current bamfout\n\r", ch);
    return;
  } else {
    arg++;
  }

  if(!strcmp(arg, "def")) {
    free(ch->player.poofout);
    ch->player.poofout = 0;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if(!strcmp(arg, "show")) {
    if (!ch->player.poofout)
      act("$n disappears in a puff of smoke.", TRUE, ch, 0, 0, TO_CHAR);
    else
      act(ch->player.poofout, TRUE, ch, 0,0,TO_CHAR);
    return;
  }

  len = strlen(arg);

  if(len > 149) {
    send_to_char("String too long.  Truncated to:\n\r", ch);
    arg[148] = '\0';
    sprintf(buf, "%s\n\r", arg);
    send_to_char(buf, ch);
    len = 149;
  }

  if (ch->player.poofout && len >= strlen(ch->player.poofout)) {
    free(ch->player.poofout);
    ch->player.poofout = (char *)malloc(len+1);
  } else if (!ch->player.poofout) {
    ch->player.poofout = (char *)malloc(len+1);
  }

  strcpy(buf, arg);
  dsearch(buf, ch->player.poofout);
  send_to_char("Ok.\n\r", ch);
  return;
}

void dsearch(char *string, char *tmp)
{
  char *c, buf[MSL], buf2[MSL], buf3[MSL];
  int i, j,count=0;

  i = 0;
  while(i == 0 && count<10) {
    if(strchr(string, '*')==NULL) {
      i = 1;
      strcpy(tmp, string);
    } else {
      count++;
      c = strchr(string, '*');
      j = c-string;
      switch(string[j+1]) {
      case 'N': strcpy(buf2, "$n"); break;
      case 'S': strcpy(buf2, "$s"); break;
      case 'V': strcpy(buf2, "$V"); break;
      case 'E': strcpy(buf2, "$e"); break;
      case 'n': strcpy(buf2, "$n"); break;
      case 's': strcpy(buf2, "$s"); break;
      case 'e': strcpy(buf2, "$e"); break;
      case 'M': strcpy(buf2, "$m"); break;
      case 'm': strcpy(buf2, "$m"); break;
      case 'R': strcpy(buf2, "$r"); break;
      case 'r': strcpy(buf2, "$r"); break;
      }
      strncpy(buf, string, sizeof(buf));
      buf[j] = '\0';
      strcpy(buf3, (string+j+2));
      sprintf(tmp, "%s%s%s" ,buf, buf2, buf3);
      strcpy(string, tmp);

    }
  }
}

void do_walkin(struct char_data *ch, char *arg, int cmd)
{
  char buf[255],name[255];
  CHAR *vict;
  int len;

  if(!check_god_access(ch,FALSE)) return;

  arg=one_argument(arg,name);

  if(!*arg || !*name) {
    send_to_char("walkin <name> <keyword/walkin def>\n\r", ch);
    send_to_char(" Additional Arguments: \n\r", ch);
    send_to_char("    *n for the name (or type the name)\n\r",ch);
    send_to_char("    *s for his or her (or type his/her)\n\r",ch);
    send_to_char("    *e for he or she (or type he/she)\n\r",ch);
    send_to_char("    Keyword 'none' removes the walkin\n\r", ch);
    send_to_char("    Keyword 'show' shows the current walkin\n\r", ch);
    return;
  }
  arg++;
  if(!(vict = get_char_vis(ch, name))) {
    send_to_char("No-one by that name here..\n\r", ch);
    return;
  }

  if(is_abbrev(arg, "none")) {
    free(vict->player.poofin);
    vict->player.poofin = 0;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if(is_abbrev(arg, "show")) {
    if (!vict->player.poofin)
      send_to_char("None.\n\r",ch);
    else
      act(vict->player.poofin, TRUE, vict, 0,ch,TO_VICT);
    return;
  }

  len = strlen(arg);

  if(len > 101) {
    send_to_char("String too long.  Truncated to:\n\r", ch);
    arg[100] = '\0';
    sprintf(buf, "%s\n\r", arg);
    send_to_char(buf, ch);
    len = 101;
  }

  if (vict->player.poofin && len >= strlen(vict->player.poofin)) {
    free(vict->player.poofin);
    vict->player.poofin = (char *)malloc(len+1);
  } else {
    if (!vict->player.poofin)
      vict->player.poofin = (char *)malloc(len+1);
  }

  strcpy(buf, arg);
  dsearch(buf, vict->player.poofin);
  send_to_char("Ok.\n\r", ch);
  return;
}

void do_walkout(struct char_data *ch, char *arg, int cmd)
{
  char buf[255],name[255];
  CHAR *vict;
  int len;

  if(!check_god_access(ch,FALSE)) return;

  arg=one_argument(arg,name);

  if(!*arg || !*name) {
    send_to_char("walkout <name> <keyword/walk definition>\n\r", ch);
    send_to_char(" Additional Arguments: \n\r", ch);
    send_to_char("    *n for the name (or type the name)\n\r",ch);
    send_to_char("    *s for his or her (or type his/her)\n\r",ch);
    send_to_char("    *e for he or she (or type he/she)\n\r",ch);
    send_to_char("    Keyword 'none' removes the walkout\n\r", ch);
    send_to_char("    Keyword 'show' shows the current walkout\n\r", ch);
    send_to_char("\n\rNote, the direction will be added to the end of the walkout\n\r",ch);
    return;
  }
  arg++;

  if(!(vict = get_char_vis(ch, name))) {
    send_to_char("No-one by that name here..\n\r", ch);
    return;
  }

  if(!strcmp(arg, "none")) {
    free(vict->player.poofout);
    vict->player.poofout = 0;
    send_to_char("Ok.\n\r", ch);
    return;
  }

  if(!strcmp(arg, "show")) {
    if (!vict->player.poofout)
      send_to_char("None.\n\r",ch);
    else
      act(vict->player.poofout, TRUE, vict, 0,ch,TO_VICT);
    return;
  }

  len = strlen(arg);

  if(len > 101) {
    send_to_char("String too long.  Truncated to:\n\r", ch);
    arg[100] = '\0';
    sprintf(buf, "%s\n\r", arg);
    send_to_char(buf, ch);
    len = 101;
  }

  if (vict->player.poofout && len >= strlen(vict->player.poofout)) {
    free(vict->player.poofout);
    vict->player.poofout = (char *)malloc(len+1);
  } else if (!vict->player.poofout) {
    vict->player.poofout = (char *)malloc(len+1);
  }

  strcpy(buf, arg);
  dsearch(buf, vict->player.poofout);
  send_to_char("Ok.\n\r", ch);
  return;
}

void do_world(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH], buffer[16384];
  int zone_count;

  if(!check_god_access(ch,FALSE)) return;

  if (!*argument) {
    send_to_char("Usage: world <basic|zones>\n\r", ch);
    return;
  }
  else
    argument++;

  *buffer = '\0';

  if (is_abbrev(argument, "basic")) {
    sprintf(buf, "Total number of rooms in world: %d\n\r", top_of_world + 1);
    send_to_char(buf, ch);
    sprintf(buf, "Total number of zones in world: %d\n\r", top_of_zone_table + 1);
    send_to_char(buf, ch);
    sprintf(buf, "Total number of shops in world: %d\n\r\n\r", number_of_shops + 1);
    send_to_char(buf, ch);
  } else if (is_abbrev(argument, "zones")) {
    send_to_char(
           "  #    Name                     Reset Age Reset  Lower - Upper\n\r", ch);
    for (zone_count=0; zone_count<=top_of_zone_table; zone_count++) {
      sprintf(buffer + strlen(buffer), "%3d  %-27s %3d  %3d    %4d  %5d - %-5d\n\r",
           zone_table[zone_count].virtual,
           zone_table[zone_count].name,
           zone_table[zone_count].lifespan,
           zone_table[zone_count].age,
           zone_table[zone_count].lifespan - zone_table[zone_count].age,
           zone_table[zone_count].bottom,
           zone_table[zone_count].top);
    }
    page_string(ch->desc, buffer, 1);
  }
}

char gamecheck_txt[MAX_STRING_LENGTH];

void do_system(struct char_data *ch, char *argument, int cmd) {
  char buf[MAX_STRING_LENGTH];
  int count1=0,count2=0,count3=0;
  struct char_data *i_ch;
  struct obj_data *i_obj;

  if(!check_god_access(ch,FALSE)) return;

  send_to_char("WizLock -- ", ch);
  if (WIZLOCK == 1) send_to_char("Yes\n\r", ch);
  else send_to_char("No\n\r", ch);

  send_to_char("GameLock -- ", ch);
  if (GAMELOCK == 1) send_to_char("Yes\n\r", ch);
  else send_to_char("No\n\r", ch);

  send_to_char("GameCheck -- ", ch);
  if (GAMECHECK == 1) {
    sprintf(buf,"Yes - %s\n\r",gamecheck_txt);
    send_to_char(buf,ch);
  }
  else send_to_char("No\n\r", ch);

  send_to_char("ChaosMode -- ", ch);
  if (CHAOSMODE == 1) send_to_char("Yes\n\r", ch);
  else send_to_char("No\n\r", ch);
  send_to_char("BAM Day -- ", ch);

  if (BAMDAY == 1) send_to_char("Yes\n\r", ch);
  else send_to_char("No\n\r", ch);

  send_to_char("Full Load at boot -- ",ch);
  if (BOOTFULL == 1) send_to_char("Yes\n\r",ch);
  else send_to_char("No\n\r",ch);

  send_to_char("Double XP Day -- ",ch);
  if (DOUBLEXP) send_to_char("Yes\n\r",ch);
  else send_to_char("No\n\r",ch);

  send_to_char("GameHalt -- ", ch);
  if (GAMEHALT == 1) send_to_char("Yes\n\r", ch);
  else send_to_char("No\n\r", ch);

  send_to_char("Reboot -- ", ch);
  if (disablereboot == 0) send_to_char("Enabled\n\r", ch);
  else send_to_char("Disabled\n\r", ch);

  send_to_char("Reboot Type -- ", ch);
  if (reboot_type == 0) send_to_char("Normal\n\r", ch);
  else send_to_char("Hotboot\n\r", ch);

  sprintf(buf,"There are %d avail descriptors, %d (0-6 not in mud use) max descriptors.\n\r",avail_descs,maxdesc);
  send_to_char(buf,ch);

  for(i_ch=character_list;i_ch;i_ch=i_ch->next)
    if(IS_NPC(i_ch)) count1++;
    else if(CAN_SEE(ch,i_ch)) {
      count2++;
      if(i_ch->desc) count3++;
    }
  sprintf(buf,"There are %d (%d) PC's in the game and %d NPC's\n\r", count2,count3,count1);
  send_to_char(buf,ch);

  count1=0;
  for(i_obj=object_list;i_obj;i_obj=i_obj->next) count1++;
  sprintf(buf,"There are %d objects in the game.\n\r",count1);
  send_to_char(buf,ch);
}

void do_release(CHAR *ch, char *argument, int cmd) {
  char tmp_name[50],ch_name[50];
  char buf[MAX_STRING_LENGTH];
  CHAR *i, *next_dude;
  int sdesc;
  struct descriptor_data *d=0;

  if(!check_god_access(ch,TRUE)) return;

  one_argument(argument, tmp_name);
  if (!*tmp_name) {
    send_to_char("Usage: release <char>/<num>\n\r",ch);
    return;
  }

  if(isdigit(*tmp_name)) {
    sdesc=atoi(tmp_name);
    for(d=descriptor_list;d;d=d->next){
      if (d->descriptor==sdesc){
        sprintf(buf,"Closing socket to descriptor #%d\n\r",sdesc);
        send_to_char(buf,ch);
        sprintf(buf, "WIZINFO: (%s) close socket #%d", GET_NAME(ch),sdesc);
        wizlog(buf, GET_LEVEL(ch), 5);
        log_s(buf);
        close_socket(d);
        return;
      }
    }
  }
  else {
    for (i = character_list; i; i = next_dude) {
      next_dude = i->next;
      if(!IS_NPC(i)) {
        sprintf(ch_name,"%s",GET_NAME(i));
        string_to_lower(ch_name);
        if(!strcmp(tmp_name,ch_name)) {
          if(i->desc) {
            if(GET_LEVEL(i)>GET_LEVEL(ch)) {
              send_to_char("You can't release someone of a higher level.\n\r",ch);
              break;
            }
            d=i->desc;
            sprintf(buf,"Closing socket to descriptor #%d - %s\n\r",d->descriptor,GET_NAME(i));
            send_to_char(buf,ch);
            sprintf(buf, "WIZINFO: (%s) closed socket #%d - %s", GET_NAME(ch),d->descriptor,GET_NAME(i));
            log_s(buf);
            wizlog(buf, GET_LEVEL(ch), 5);
            close_socket(d);
          }
          return;
        }
      }
    }
  }
  send_to_char("Descriptor not found!\n\r",ch);
  send_to_char("Usage: release <char>/<num>\n\r",ch);
}


void do_gamemode(struct char_data *ch, char *argument, int cmd) {
  long ct;
  char buf[MAX_STRING_LENGTH],arg[MAX_STRING_LENGTH],*tmstr;
  struct descriptor_data *e;

  if(!check_god_access(ch,TRUE)) return;

  argument=one_argument(argument,arg);

  if(!*arg) {
    send_to_char("Usage: gamemode wizlock         WIZ+ (no new char creation)\n\r",ch);
    send_to_char("                lock            WIZ+ (no mortals allowed in)\n\r",ch);
    send_to_char("                check <reason>  WIZ+ (screens new chars)\n\r",ch);
    if(GET_LEVEL(ch)>LEVEL_ETE) {
      send_to_char("                bam             SUP+ (BAM rules)\n\r",ch);
    }
    if(GET_LEVEL(ch)>LEVEL_SUP) {
      send_to_char("                chaos           IMP  (chaos rules)\n\r",ch);
      send_to_char("                pulse           IMP  (measure of machine lag)\n\r",ch);
      send_to_char("                halt            IMP  (time stops)\n\r",ch);
      send_to_char("                doublexp        IMP  (normal exp gain is doubled)\n\r",ch);
    }
    return;
  }

  if(is_abbrev(arg,"wizlock")) {
    if ( WIZLOCK == 0 ) {
      sprintf(buf,"Game is WIZLOCKED.  No character creation.\n\r");
      WIZLOCK = 1;
    }
    else  {
      sprintf(buf,"Game is NOT WIZLOCKED.  New characters may be created.\n\r");
      WIZLOCK = 0;
    }
    send_to_char(buf,ch);
    for (e=descriptor_list;e;e=e->next)
      if(e->character !=ch && !e->connected &&
         (GET_LEVEL(e->character) >= LEVEL_IMM))
        act(buf,0,ch,0,e->character,TO_VICT);
    return;
  }

  if(is_abbrev(arg,"lock")) {
    if ( GAMELOCK == 0 ) {
      sprintf(buf,"GAME IS CLOSED. YOU MAY NOT RE-ENTER IF YOU DIE!\n\r");
      GAMELOCK = 1;
    }
    else  {
      sprintf(buf,"GAME IS OPEN AGAIN.\n\r");
      GAMELOCK = 0;
    }
    send_to_char(buf,ch);
    for (e=descriptor_list;e;e=e->next)
      if (e->character !=ch && !e->connected) act(buf,0,ch,0,e->character,TO_VICT);
    return;
  }

  if(is_abbrev(arg,"check")) {
    if ( GAMECHECK == 0 ) {
      if(!*argument) {
        send_to_char("You need to give a reason for checking.\n\r",ch);
        return;
      }
      if(strlen(argument)>MAX_INPUT_LENGTH) {
        send_to_char("Reason text is too long, please be a little more brief.\n\r",ch);
        return;
      }
      ct = time(0);
      tmstr = asctime(localtime(&ct));
      *(tmstr+strlen(tmstr)-1)='\0';
      sprintf(gamecheck_txt,"%s  By: %s  At: %s EST",argument,GET_NAME(ch),tmstr);
      sprintf(buf,"Every New Character need an Immortal to 'allowin' now.\n\r");
      GAMECHECK = 1;
    }
    else {
      sprintf(buf,"GAME IS NOT CHECKED.\n\r");
      GAMECHECK = 0;
    }
    send_to_char(buf,ch);
    for (e=descriptor_list;e;e=e->next)
      if (e->character !=ch && !e->connected && (GET_LEVEL(e->character) >= LEVEL_IMM))
        act(buf,0,ch,0,e->character,TO_VICT);
    return;
  }

  if(is_abbrev(arg,"bam") && GET_LEVEL(ch)>LEVEL_ETE) {
    if ( BAMDAY == 0 ) {
      sprintf(buf,"`iBAM Day started.\n\r`q");
      BAMDAY = 1;
    }
    else {
      sprintf(buf,"`iBAM Day stopped.`q\n\r");
      BAMDAY = 0;
    }
    send_to_char(buf,ch);
    for (e=descriptor_list;e;e=e->next)
      if(e->character !=ch && !e->connected &&
         (GET_LEVEL(e->character) >= LEVEL_IMM))
        act(buf,0,ch,0,e->character,TO_VICT);
    return;
  }

  if(is_abbrev(arg,"chaos") && GET_LEVEL(ch)>LEVEL_SUP) {
    if ( CHAOSMODE == 0 ) {
      sprintf(buf,"`i***** ENTERING CHAOS MODE *****\n\r\n\r***** ENTERING CHAOS MODE *****\n\r\n\r***** ENTERING CHAOS MODE *****`q\n\r\n\r");
      CHAOSMODE = 1;
    }
    else {
      sprintf(buf,"`i**** LEAVING CHAOS MODE *****`q\n\r");
      CHAOSMODE = 0;
    }
    send_to_char(buf,ch);
    for (e=descriptor_list;e;e=e->next)
      if (e->character !=ch && !e->connected)
        act(buf,0,ch,0,e->character,TO_VICT);
    return;
  }

  if(is_abbrev(arg,"pulse") && GET_LEVEL(ch)>LEVEL_SUP) {
    if ( PULSECHECK == 0 ) {
      sprintf(buf,"`iPulsecheck on.\n\r`q");
      PULSECHECK = 1;
    }
    else {
      sprintf(buf,"`iPulse check off.`q\n\r");
      PULSECHECK = 0;
    }
    send_to_char(buf,ch);
    return;
  }

  if(is_abbrev(arg,"halt") && GET_LEVEL(ch)>LEVEL_SUP) {
    if ( GAMEHALT == 0 ) {
      sprintf(buf,"Game has been halted - time has stopped!!\n\r");
      GAMEHALT = 1;
    }
    else {
      sprintf(buf,"Time has restarted!!\n\r");
      GAMEHALT = 0;
    }
    send_to_char(buf,ch);
    for (e=descriptor_list;e;e=e->next)
      if (e->character !=ch && !e->connected)
        act(buf,0,ch,0,e->character,TO_VICT);
    return;
  }

  if(is_abbrev(arg,"doublexp") && GET_LEVEL(ch)>LEVEL_SUP) {
    if ( DOUBLEXP == 0 ) {
      sprintf(buf,"`i***** DOUBLE EXPERIENCE MODE ENABLED!! *****\n\r\n\r***** DOUBLE EXPERIENCE MODE ENABLED!! *****\n\r\n\r***** DOUBLE EXPERIENCE MODE ENABLED!! *****`q\n\r\n\r");
      DOUBLEXP = 1;
    }
    else {
      sprintf(buf,"`i***** DOUBLE EXPERIENCE MODE DISABLED *****\n\r\n\r***** DOUBLE EXPERIENCE MODE DISABLED *****\n\r\n\r***** DOUBLE EXPERIENCE MODE DISABLED *****`q\n\r\n\r");
      DOUBLEXP = 0;
    }
    send_to_char(buf,ch);
    for (e=descriptor_list;e;e=e->next)
      if (e->character !=ch && !e->connected)
        act(buf,0,ch,0,e->character,TO_VICT);
    return;
  }
  /* invalid parameter */
  send_to_char("Usage: gamemode wizlock         WIZ+ (no new char creation)\n\r",ch);
  send_to_char("                lock            WIZ+ (no mortals allowed in)\n\r",ch);
  send_to_char("                check <reason>  WIZ+ (screens new chars)\n\r",ch);
  if(GET_LEVEL(ch)>LEVEL_ETE) {
    send_to_char("                bam             SUP+ (BAM rules)\n\r",ch);
  }
  if(GET_LEVEL(ch)>LEVEL_SUP) {
    send_to_char("                chaos           IMP  (chaos rules)\n\r",ch);
    send_to_char("                pulse           IMP  (measure of machine lag)\n\r",ch);
    send_to_char("                halt            IMP  (time stops)\n\r",ch);
    send_to_char("                doublexp        IMP  (normal exp gain is doubled)\n\r",ch);
  }
  return;
}

void do_emote(struct char_data *ch, char *argument, int cmd)
{
  int i;
  char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH],buf3[MAX_STRING_LENGTH];

  if(!AWAKE(ch)) {
    send_to_char("While asleep? Hah.\n\r",ch);
    return;
  }

  if(!IS_NPC(ch) && IS_SET(ch->specials.pflag, PLR_NOSHOUT))
    {
       send_to_char("You have displeased the gods, you cannot emote.\n\r", ch);
       return;
    }

  if(!IS_NPC(ch) && IS_SET(ch->specials.pflag,PLR_QUEST) && IS_SET(ch->specials.pflag,PLR_QUIET)) {
     send_to_char("The Questmaster has taken away your ability to interrupt.\n\r",ch);
     return; /* For quests - Ranger June 96 */
  }

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, QUIET) && (GET_LEVEL(ch) < LEVEL_IMM)) {
     send_to_char("A magical force prevents your actions.\n\r", ch);
     return; /* Ranger - July 96 */
  }

  for (i = 0; *(argument + i) == ' '; i++);
  if (!*(argument + i))
    send_to_char("Yes.. But what?\n\r", ch);
  else {
    half_chop(argument,buf,MSL,buf2,MSL);
    sprintf(buf3,"$i %s %s",buf,buf2);
    if(strstr(buf3,"lucky ivory die rolls a"))
      strcat(buf3,"- a FAKE ROLL!!");
    act(buf3,FALSE,ch,0,0,TO_ROOM);
    act(buf3,FALSE,ch,0,0,TO_CHAR);
  }
}

void do_killer(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  struct obj_data *dummy;
  char buf[MAX_INPUT_LENGTH];

  if(!check_god_access(ch,TRUE)) return;

  if(GET_LEVEL(ch)<LEVEL_SUP && !IS_SET(ch->new.imm_flags, WIZ_JUDGE)) {
    send_to_char("You need a Judge flag to do this.\n\r",ch);
    return;
  } /* Judge Flag check added by Ranger - May 96 */

  one_argument(argument, buf);

  if (!*buf)
    if (IS_SET(ch->specials.pflag, PLR_KILL)) {
      send_to_char("You are no longer a killer.\n\r", ch);
      REMOVE_BIT(ch->specials.pflag, PLR_KILL);
    }
    else {
      send_to_char("You are a killer! Beware!\n\r", ch);
      SET_BIT(ch->specials.pflag, PLR_KILL);
    }
  else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
    send_to_char("Couldn't find any such creature.\n\r", ch);
  else if (IS_NPC(vict))
    send_to_char("Can't do that to a beast.\n\r", ch);
  else if (GET_LEVEL(vict) > GET_LEVEL(ch))
    act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  else if (IS_SET(vict->specials.pflag, PLR_KILL)) {
    send_to_char("You are no longer a killer.\n\r", vict);
    send_to_char("KILLER flag removed.\n\r", ch);
    REMOVE_BIT(vict->specials.pflag, PLR_KILL);
  }
  else {
    send_to_char("Warning! you are a killer!\n\r", vict);
    send_to_char("KILLER flag set.\n\r", ch);
    SET_BIT(vict->specials.pflag, PLR_KILL);
  }
}

void do_thief(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  struct obj_data *dummy;
  char buf[MAX_INPUT_LENGTH];

  if(!check_god_access(ch,FALSE)) return;

  if(GET_LEVEL(ch)<LEVEL_SUP && !IS_SET(ch->new.imm_flags, WIZ_JUDGE)) {
    send_to_char("You need a Judge flag to do this.\n\r",ch);
    return;
  } /* Judge Flag check added by Ranger - May 96 */

  one_argument(argument, buf);

  if (!*buf)
    if (IS_SET(ch->specials.pflag, PLR_THIEF)) {
      send_to_char("You are no longer a thief.\n\r", ch);
      REMOVE_BIT(ch->specials.pflag, PLR_THIEF);
    } else {
      send_to_char("You are a thief! Beware!\n\r", ch);
      SET_BIT(ch->specials.pflag, PLR_THIEF);
  } else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
    send_to_char("Couldn't find any such creature.\n\r", ch);
  else if (IS_NPC(vict))
    send_to_char("Can't do that to a beast.\n\r", ch);
  else if (GET_LEVEL(vict) > GET_LEVEL(ch))
    act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  else if (IS_SET(vict->specials.pflag, PLR_THIEF)) {
    send_to_char("You are no longer a thief.\n\r", vict);
    send_to_char("THIEF flag removed.\n\r", ch);
    REMOVE_BIT(vict->specials.pflag, PLR_THIEF);
  }
  else {
    send_to_char("Warning! you are a thief!\n\r", vict);
    send_to_char("THIEF flag set.\n\r", ch);
    SET_BIT(vict->specials.pflag, PLR_THIEF);
  }
}

void do_lecho(struct char_data *ch, char *argument, int cmd)
{
  int i;
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];

  if(!check_god_access(ch,FALSE)) return;

  for (i = 0; *(argument + i) == ' '; i++);
  if (!*(argument + i))
    send_to_char("That must be a mistake...\n\r", ch);
  else {
    if (GET_LEVEL(ch) == LEVEL_IMM){
      sprintf(buf,"The following is an echo from %s.\n\r",
           GET_NAME(ch));
      send_to_room_except(buf, CHAR_REAL_ROOM(ch), ch);
    }
    sprintf(buf,"%s\r", argument + i);
    act (buf,0,ch,0,0, TO_ROOM);
    sprintf(buf, "You echo '%s'\n\r", argument+i);
    send_to_char(buf, ch);
    sprintf(buf2,"WIZINFO: %s lechos '%s'",GET_NAME(ch),argument+i);
    log_s(buf2);
    wizlog(buf2, GET_LEVEL(ch)+1, 5);
  }
}

void do_echo(struct char_data *ch, char *argument, int cmd)
{
  int i;
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  struct descriptor_data *e;

  if(!check_god_access(ch,FALSE)) return;

  for (i = 0; *(argument + i) == ' '; i++);
  if (!*(argument + i))
    send_to_char("That must be a mistake...\n\r", ch);
  else {
    sprintf(buf,"%s\r", argument + i);
    for (e=descriptor_list; e; e= e->next)
      if (e->character !=ch && !e->connected) {
          if (GET_LEVEL(ch) == LEVEL_IMM) {
           sprintf(buf2, "The following is an echo from %s.\n\r",
              GET_NAME(ch));
           send_to_char(buf2, e->character);
          }

          act (buf,0,ch,0,e->character, TO_VICT);
      }

    sprintf(buf,"You echo '%s'\n\r", argument + i);
    send_to_char(buf, ch);

    sprintf(buf2,"WIZINFO: %s echos '%s'",GET_NAME(ch),argument+i);
    log_s(buf2);
    wizlog(buf2, GET_LEVEL(ch)+1, 5);
  }

}

void do_wemote(struct char_data *ch, char *argument, int cmd)
{
  int i,level;
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  struct descriptor_data *e;

  if(!IS_NPC(ch)) {

    if (!IS_SET(ch->new.imm_flags, WIZ_TRUST)) {
      send_to_char("You need a Trust flag to do that!\n\r", ch);
      return;
    }

    if (IS_SET(ch->new.imm_flags, WIZ_FREEZE)) {
      send_to_char("You are Frozen, so you can't do it!\n\r", ch);
      return;
    }

  }

  if(IS_SET(ch->new.imm_flags,WIZ_NO_NET)) {
    send_to_char("You can't do that.\n\r",ch);
    return;
  }
  for(i=0;*(argument+i)==' ';i++);

  if(!*(argument+i)) {
    send_to_char("Yes! But what??\n\r",ch);
    return;
  }

  half_chop(argument,buf,MSL,buf2,MSL);
  if(!isdigit(*buf)) {
    level = LEVEL_IMM;
    sprintf(buf3,"[wiz] $i %s %s",buf,buf2);
  }
  else {
    level = atoi(buf);
    if((level > LEVEL_IMM) && (level <= LEVEL_IMP)) {
      sprintf(buf3,"[wiz] (%d) $i %s",level,buf2);
    } else {
      level=LEVEL_IMM;
      sprintf(buf3,"[wiz] $i %s %s",buf,buf2);
    }
  }

  for(e=descriptor_list;e;e=e->next) {
    if(!e->connected&&(GET_LEVEL(e->character)>=level)) {
      COLOR(e->character,10);
      act(buf3,0,ch,0,e->character,TO_VICT);
      ENDCOLOR(e->character);
    }
  }
  COLOR(ch,10);
  act(buf3,0,ch,0,0,TO_CHAR);
  ENDCOLOR(ch);
}

void do_wiznet(struct char_data *ch,char *argument, int cmd)
{
  int i, level;
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH],buf4[MAX_STRING_LENGTH];
  struct descriptor_data *e;

  if(!IS_NPC(ch)) {

    if (!IS_SET(ch->new.imm_flags, WIZ_TRUST)) {
      send_to_char("You need a Trust flag to do that!\n\r", ch);
      return;
    }

    if (IS_SET(ch->new.imm_flags, WIZ_FREEZE)) {
      send_to_char("You are Frozen, so you can't do it!\n\r", ch);
      return;
    }

  }

  if (IS_SET(ch->new.imm_flags, WIZ_NO_NET)) {
    send_to_char("Someone has taken your ability to do that!\n\r", ch);
    return;
  }
  for (i = 0; *(argument + i) == ' '; i++);
  if (!*(argument + i)) {
    if(IS_SET(ch->new.imm_flags, WIZ_WIZNET)) {
      REMOVE_BIT(ch->new.imm_flags, WIZ_WIZNET);
      send_to_char("Wiznet off.\n\r", ch);
    }
    else {
      SET_BIT(ch->new.imm_flags, WIZ_WIZNET);
      send_to_char("Wiznet on.\n\r",ch);
    }
    return;
  }

  if(!IS_SET(ch->new.imm_flags, WIZ_WIZNET)) {
    SET_BIT(ch->new.imm_flags, WIZ_WIZNET);
    send_to_char("Wiznet on.\n\r",ch);
  }

  half_chop(argument, buf3,MSL,buf4,MSL);
  if (!isdigit(*buf3)) {
    level=ch->specials.wiznetlvl;
    if(level>LEVEL_IMM && level<=LEVEL_IMP)
      sprintf(buf2,"$i(%d): %s\r", level, argument + i);
    else
      sprintf(buf2,"$i: %s\r", argument + i);
  }
  else {
    if(!*buf4) {
      level= atoi(buf3);
      level= MAX(51,level);
      level= MIN(57,level);
      ch->specials.wiznetlvl=level;
      printf_to_char(ch,"Wiznet level set to %d.\n\r",ch->specials.wiznetlvl);
      return;
    }
    level=atoi(buf3);
    if(level<LEVEL_IMM || level>LEVEL_IMP) {
      level=ch->specials.wiznetlvl;
      if(level>LEVEL_IMM && level<=LEVEL_IMP)
        sprintf(buf2,"$i(%d): %s %s\r", level,buf3,buf4);
      else
        sprintf(buf2,"$i: %s %s\r",buf3,buf4);
    }
    else {
      sprintf(buf2,"$i(%d): %s\r", level, buf4);
    }
  }

  for (e = descriptor_list; e; e = e->next)
    if (!e->connected &&
       ( GET_LEVEL(e->character)>=level || (e->character->desc->original && GET_LEVEL(e->character->desc->original)>=level) ) &&
       ( GET_LEVEL(e->character)>=LEVEL_IMM || (e->character->desc->original && GET_LEVEL(e->character->desc->original)>=LEVEL_IMM)) &&
       (IS_SET(e->character->new.imm_flags, WIZ_WIZNET) || e->character->desc->original)) {
       COLOR(e->character,10);
      act(buf2, 0, ch, 0, e->character, TO_VICT);
      ENDCOLOR(e->character);
    }
  COLOR(ch,10);
  act(buf2, 0,ch,0,0, TO_CHAR);
  ENDCOLOR(ch);
}

void do_roomlock(struct char_data *ch, char *argument, int cmd)
{
  if(!check_god_access(ch,TRUE)) return;

  if ( IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, LOCK)) {
    send_to_char("You have UNLOCKED the room.\n\r",ch);
    REMOVE_BIT(world[CHAR_REAL_ROOM(ch)].room_flags, LOCK);
  }
  else  {
    send_to_char("You have LOCKED the room, no one can come in now.\n\r",ch);
    SET_BIT(world[CHAR_REAL_ROOM(ch)].room_flags, LOCK);
  }

}

void do_trans(struct char_data *ch, char *argument, int cmd)
{
  struct descriptor_data *i;
  struct char_data *victim;
  sh_int target;
  char buf[MAX_STRING_LENGTH];

  if(!check_god_access(ch,FALSE)) return;

  one_argument(argument,buf);

  if (!*buf)
    send_to_char("Who do you wish to transfer?\n\r",ch);
  else if (strcmp("all", buf)) {
    if (!(victim = get_char_vis(ch,buf)))
      send_to_char("No-one by that name around.\n\r",ch);
    else {
      act("$n disappears in a mushroom cloud.",FALSE, victim, 0, 0, TO_ROOM);

      if(victim->specials.riding) stop_riding(victim,victim->specials.riding);
      if (victim->specials.rider) stop_riding(victim->specials.rider,victim);

      sprintf (buf, "WIZINFO: %s transferred %s from #%d to #%d",
            GET_NAME(ch), GET_NAME(victim),
            CHAR_VIRTUAL_ROOM(victim),
            CHAR_VIRTUAL_ROOM(ch));
      wizlog(buf, GET_LEVEL(ch)+1, 5);
      log_s(buf);

      target = CHAR_REAL_ROOM(ch);
      char_from_room(victim);
      char_to_room(victim,target);
      act("$n arrives from a puff of smoke.",
       FALSE, victim, 0, 0, TO_ROOM);
      act("$n has transferred you!",FALSE,ch,0,victim,TO_VICT);

      do_look(victim,"",15);
      send_to_char("Ok.\n\r",ch);
    }
  } else { /* Trans All */
    for (i = descriptor_list; i; i = i->next)
     if (i->character != ch && !i->connected) {
     victim = i->character;
     act("$n disappears in a mushroom cloud.",
         FALSE, victim, 0, 0, TO_ROOM);

      if(victim->specials.riding) stop_riding(victim,victim->specials.riding);
      if(victim->specials.rider) stop_riding(victim->specials.rider,victim);

     sprintf (buf, "WIZINFO: %s transferred all to #%d",
           GET_NAME(ch), CHAR_VIRTUAL_ROOM(ch));
     wizlog(buf, GET_LEVEL(ch)+1, 5);
     log_s(buf);

     target = CHAR_REAL_ROOM(ch);
     char_from_room(victim);
     char_to_room(victim,target);
     act("$n arrives from a puff of smoke.",
          FALSE, victim, 0, 0, TO_ROOM);
     act("$n has transferred you!",FALSE,ch,0,victim,TO_VICT);
     do_look(victim,"",15);
      }

    send_to_char("Ok.\n\r",ch);
  }
}

void do_at(struct char_data *ch, char *argument, int cmd)
{
  char command[MAX_INPUT_LENGTH], loc_str[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  int loc_nr, location, original_loc,zonenum;
  struct char_data *target_mob;
  struct obj_data *target_obj;

  if(!check_god_access(ch,FALSE)) return;

  half_chop(argument, loc_str,MIL,command,MIL);
  if (!*loc_str) {
    send_to_char("You must supply a player name, room number, or direction.\n\r", ch);
    return;
  }

  if (is_number(loc_str)) {
    loc_nr = atoi(loc_str);
    for (location = 0; location <= top_of_world; location++) {
      if (world[location].number == loc_nr) break;
      else if (location == top_of_world) {
        send_to_char("No room exists with that number.\n\r", ch);
        return;
      }
    }
  }
  else if ((target_mob = get_char_vis(ch, loc_str))) {
    location = CHAR_REAL_ROOM(target_mob);
    if(!strcmp(GET_NAME(target_mob),"rashgugh") && GET_LEVEL(ch)<LEVEL_SUP) {
      send_to_char("You can't do at by Rashgugh.\n\r",ch);
      return;
    }
  }
  else if(is_abbrev(loc_str,"north")) {
    if(!world[CHAR_REAL_ROOM(ch)].dir_option[0]) {
      send_to_char("There is no room to the north.\n\r",ch);
      return;
    }
    location=world[CHAR_REAL_ROOM(ch)].dir_option[0]->to_room_r;
  }
  else if(is_abbrev(loc_str,"east")) {
    if(!world[CHAR_REAL_ROOM(ch)].dir_option[1]) {
      send_to_char("There is no room to the east.\n\r",ch);
      return;
    }
    location=world[CHAR_REAL_ROOM(ch)].dir_option[1]->to_room_r;
  }
  else if(is_abbrev(loc_str,"south")) {
    if(!world[CHAR_REAL_ROOM(ch)].dir_option[2]) {
      send_to_char("There is no room to the south.\n\r",ch);
      return;
    }
    location=world[CHAR_REAL_ROOM(ch)].dir_option[2]->to_room_r;
  }
  else if(is_abbrev(loc_str,"west")) {
    if(!world[CHAR_REAL_ROOM(ch)].dir_option[3]) {
      send_to_char("There is no room to the west.\n\r",ch);
      return;
    }
    location=world[CHAR_REAL_ROOM(ch)].dir_option[3]->to_room_r;
  }
  else if(is_abbrev(loc_str,"up")) {
    if(!world[CHAR_REAL_ROOM(ch)].dir_option[4]) {
      send_to_char("There is no room up.\n\r",ch);
      return;
    }
    location=world[CHAR_REAL_ROOM(ch)].dir_option[4]->to_room_r;
  }
  else if(is_abbrev(loc_str,"down")) {
    if(!world[CHAR_REAL_ROOM(ch)].dir_option[5]) {
      send_to_char("There is no room down.\n\r",ch);
      return;
    }
    location=world[CHAR_REAL_ROOM(ch)].dir_option[5]->to_room_r;
  }
  else if ((target_obj = get_obj_vis_in_rooms(ch, loc_str))) {
    if (target_obj->in_room != NOWHERE)
      location = target_obj->in_room;
    else {
      send_to_char("The object is not available.\n\r", ch);
      return;
    }
  }
  else {
    send_to_char("No such creature or object around.\n\r", ch);
    return;
  }

  /* a location has been found. */
  if (IS_SET(world[location].room_flags, LAWFUL)&&(GET_LEVEL(ch)<LEVEL_ETE)) {
    send_to_char("Sorry, your level is not high enough to go there!\n\r", ch);
    return;
  }

  zonenum=inzone(world[location].number);
  if(zone_locked(ch,zonenum)) return;

  original_loc = CHAR_REAL_ROOM(ch);
  char_from_room(ch);
  char_to_room(ch, location);
  if(is_abbrev(command,"look")) {
    sprintf(buf,"WIZINFO: %s at %s(%d): %s",GET_NAME(ch),loc_str,world[location].number,command);
    log_s(buf);
    wizlog(buf,LEVEL_IMP,5);
  }
  if(GET_LEVEL(ch)<LEVEL_TEM && is_abbrev(command,"exit")) {
    sprintf(buf,"WIZINFO: %s at %s(%d): %s",GET_NAME(ch),loc_str,world[location].number,command);
    log_s(buf);
    wizlog(buf,LEVEL_IMP,5);
  }
  command_interpreter(ch, command);

  if(ch->specials.riding) stop_riding(ch,ch->specials.riding);

  /* check if the guy's still there */
  for (target_mob = world[location].people; target_mob; target_mob =
       target_mob->next_in_room)
    if (ch == target_mob) {
      char_from_room(ch);
      char_to_room(ch, original_loc);
    }
}


void do_goto(struct char_data *ch, char *argument, int cmd) {
  char buf[MAX_INPUT_LENGTH];
  char skibuf[MAX_STRING_LENGTH];
  int loc_nr, location,zonenum;
  struct char_data *target_mob;
  struct obj_data *target_obj;

  if(!check_god_access(ch,FALSE)) return;

  one_argument(argument, buf);

  if (!*buf) {
    send_to_char("You must supply a room number or a name.\n\r", ch);
    return;
  }

  if(is_number(buf)) {
    loc_nr = atoi(buf);
    for (location = 0; location <= top_of_world; location++)
      if (world[location].number == loc_nr)     break;
      else if (location == top_of_world) {
        send_to_char("No room exists with that number.\n\r", ch);
        return;
      }
  }
  else if ((target_mob = get_char_vis(ch, buf))) {
    location = CHAR_REAL_ROOM(target_mob);
    if(!strcmp(GET_NAME(target_mob),"rashgugh") && GET_LEVEL(ch)<LEVEL_SUP) {
      send_to_char("You can't goto Rashgugh.\n\r",ch);
      return;
    }
  }
  else if ((target_obj = get_obj_vis_in_rooms(ch, buf)))
    if (target_obj->in_room != NOWHERE) location = target_obj->in_room;
    else {
      send_to_char("The object is not available.\n\r", ch);
      return;
    }
  else {
    send_to_char("No such creature or object around.\n\r", ch);
    return;
  }

  /* a location has been found. */

  if(IS_SET(world[location].room_flags, LOCK) && (GET_LEVEL(ch) < LEVEL_IMP) &&
     !isname(GET_NAME(ch),world[location].name)) {
    send_to_char("The room is locked. There may be a private conversation there.\n\r",ch);
    return;
  }

  if (IS_SET(world[location].room_flags, LAWFUL)&&(GET_LEVEL(ch)<LEVEL_ETE)) {
    send_to_char("Sorry, your level is not high enough to go there!\n\r", ch);
    return;
  }

  zonenum=inzone(world[location].number);
  if(zone_locked(ch,zonenum)) return;

  if (!ch->player.poofout)
    act("$n disappears in a puff of smoke.", TRUE, ch, 0, 0, TO_ROOM);
  else
    act(ch->player.poofout, TRUE, ch, 0, 0, TO_ROOM);

  if(ch->specials.riding) stop_riding(ch,ch->specials.riding);

  char_from_room(ch);
  char_to_room(ch, location);

  sprintf(skibuf, "WIZINFO: (%s) goto '%s' (Room # %d)", GET_NAME(ch), argument, world[location].number);
/*  wizlog(skibuf, LEVEL_IMP, 5);*/
  log_s(skibuf);

  if (!ch->player.poofin)
    act("$n appears with an ear-splitting bang.", TRUE, ch, 0,0,TO_ROOM);
  else
    act(ch->player.poofin, TRUE, ch, 0,0,TO_ROOM);

  do_look(ch, "",15);
}


extern void show_zone_to_char(CHAR *ch, int zoneNum);
void do_stat(struct char_data *ch, char *argument, int cmd)
{
  char apt[3];
  struct affected_type_5 *aff;
  struct enchantment_type_5 *ench;
  struct obj_data *wielded=0,*hold=0;
  char arg1[MAX_STRING_LENGTH],buf[2*MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
  char type[100];
  char *tmstr;
  long ct;
  struct room_data *rm=0;
  struct char_data *k=0;
  struct obj_data  *j=0,*j2=0;
  struct extra_descr_data *desc;
  struct follow_type *fol;
  int i, virtual,zonenum,snum;
  int i2;
  int total_stats,total_real_stats,arm;
  long int mindam,maxdam,avgdam;
  bool found;
  struct sockaddr_in isa;

  if(!check_god_access(ch,FALSE)) return;

  argument_interpreter(argument, type, arg1);

  /* no argument */
  if (!*arg1) {
    send_to_char("Syntax:\n\rstat <'char' | 'obj' | 'room' | 'zone' | 'shop'> <name/#>.\n\r",ch);
    return;
  } else {
    sprintf(buf,"WIZINFO: %s stat %s",GET_NAME(ch),argument);
    wizlog(buf, GET_LEVEL(ch)+1, 5);
    log_s(buf);
  /* stats on room */
    if (is_abbrev(type, "room")) {
      i= atoi( arg1); /* Stat room in argument - Ranger May 96 */
      i2=real_room(i);
      if(i2==-1) {
        send_to_char("Room doesn't exist.\n\r",ch);
        return;
      }
      rm = &world[real_room(i)];
      zonenum=rm->zone;
      if(zone_locked(ch,zonenum)) return;

      sprintf(buf, "Room name: %s, Of zone : %d. V-Number : %d, R-number : %d\n\r",
           rm->name, rm->zone, rm->number, real_room(rm->number));
      send_to_char(buf, ch);

      sprinttype(rm->sector_type,sector_types,buf2);
      sprintf(buf, "Sector type : %s", buf2);
      send_to_char(buf, ch);

      strcpy(buf,"Special procedure : ");
      strcat(buf,(rm->funct) ? "Exists\n\r" : "No\n\r");
      send_to_char(buf, ch);

      send_to_char("Room flags: ", ch);
      sprintbit((long) rm->room_flags,room_bits,buf);
      strcat(buf,"\n\r");
      send_to_char(buf,ch);

      send_to_char("Description:\n\r", ch);
      send_to_char(rm->description, ch);

      strcpy(buf, "Extra description keywords(s): ");
      if (rm->ex_description) {
     strcat(buf, "\n\r");
     for (desc = rm->ex_description; desc; desc = desc->next) {
       strcat(buf, desc->keyword);
       strcat(buf, "\n\r");
     }
     strcat(buf, "\n\r");
     send_to_char(buf, ch);
      } else {
     strcat(buf, "None\n\r");
     send_to_char(buf, ch);
      }

      send_to_char("------- Chars present -------\n\r",ch);
      for (k = rm->people; k; k = k->next_in_room)
        if (CAN_SEE(ch, k))
         printf_to_char(ch,"%s %s\n\r",GET_NAME(k),(!IS_NPC(k) ? "(PC)" :
                        (!IS_MOB(k) ? "(NPC)" : "(MOB)")));

      send_to_char("\n\r--------- Contents ---------\n\r",ch);
      for (j = rm->contents; j; j = j->next_content)
        printf_to_char(ch,"%s\n\r",OBJ_NAME(j));


      send_to_char("\n\r------- Exits defined -------\n\r", ch);
      for (i = 0; i <= 5; i++) {
     if (rm->dir_option[i]) {
       sprintf(buf,"Direction %s . Keyword : %s\n\r",
            dirs[i], rm->dir_option[i]->keyword);
       send_to_char(buf, ch);
       strcpy(buf, "Description:\n\r  ");
       if(rm->dir_option[i]->general_description)
         strcat(buf, rm->dir_option[i]->general_description);
       else
         strcat(buf,"UNDEFINED\n\r");
       send_to_char(buf, ch);
       sprintbit(rm->dir_option[i]->exit_info,exit_bits,buf2);
       sprintf(buf,
            "Exit flag: %s \n\rKey no: %d\n\rTo room (V-Number): %d\n\r",
            buf2, rm->dir_option[i]->key, rm->dir_option[i]->to_room_v);
       send_to_char(buf, ch);
     }
      }
      return;
    } else if (is_abbrev(type, "shop")) {
      /* stats on shop */
      found=FALSE;
      i2= atoi( arg1);
      for(i=0 ; i<number_of_shops ; i++) {
        if(shop_index[i].keeper==i2) {
          found=TRUE;
          snum=i;
          break;
        }
      }
      if(!found) {
        send_to_char("Shop doesn't exist.\n\r",ch);
        return;
      }

      zonenum=inzone(i2);
      if(zone_locked(ch,zonenum)) return;

      printf_to_char(ch, "Shop number: %d, Of zone : %d\n\r",i2,zonenum);
      for(i=0;i<MAX_PROD;i++) {
        printf_to_char(ch,"Producing Item: %d\n\r",shop_index[snum].producing[i]);
      }

      printf_to_char(ch,"Profit multiplier on buying: %f\n\r",shop_index[snum].profit_buy);
      printf_to_char(ch,"Profit multiplier on selling: %f\n\r",shop_index[snum].profit_sell);

      for(i=0;i<MAX_TRADE;i++) {
        printf_to_char(ch,"Will buy item type: %d\n\r",shop_index[snum].type[i]);
      }

      printf_to_char(ch,"Keeper text if:\n\r");
      printf_to_char(ch,"Keeper doesn't have item: %s\n\r",shop_index[snum].no_such_item1);
      printf_to_char(ch,"Seller doesn't have item: %s\n\r",shop_index[snum].no_such_item2);
      printf_to_char(ch,"Keeper doesn't buy item:  %s\n\r",shop_index[snum].do_not_buy);
      printf_to_char(ch,"Keeper doesn't have cash: %s\n\r",shop_index[snum].missing_cash1);
      printf_to_char(ch,"Buyer doesn't have cash:  %s\n\r",shop_index[snum].missing_cash2);
      printf_to_char(ch,"Buyer buys item:          %s\n\r",shop_index[snum].message_buy);
      printf_to_char(ch,"Keeper buys item:         %s\n\r",shop_index[snum].message_sell);
      printf_to_char(ch,"Action if buyer hasn't enough cash (0=default, 1=smokes a joint): %d\n\r",shop_index[snum].temper1);
      printf_to_char(ch,"Unused flag for keeper action: %d\n\r",shop_index[snum].temper2);
      printf_to_char(ch,"Keeper number (should be same as shop number): %d\n\r",shop_index[snum].keeper);
      printf_to_char(ch,"Unused flag for who keeper trades with: %d\n\r",shop_index[snum].with_who);
      printf_to_char(ch,"Keeper room number: %d\n\r",shop_index[snum].in_room);
      printf_to_char(ch,"Hour store first opens (of 28):  %2d\n\r",shop_index[snum].open1);
      printf_to_char(ch,"Hour store first closes (of 28): %2d\n\r",shop_index[snum].close1);
      printf_to_char(ch,"Hour store opens later (of 28):  %2d\n\r",shop_index[snum].open2);
      printf_to_char(ch,"Hour store closes later (of 28): %2d\n\r",shop_index[snum].close2);

      return;
/*zone stat */
    } else if (is_abbrev(type, "zone")) {
      zonenum = atoi( arg1);
      zonenum = real_zone(zonenum);
      if(zonenum!= -1) {
        if(zone_locked(ch,zonenum)) return;
        show_zone_to_char(ch, zonenum);
      }
      return;
    } else if (is_abbrev(type, "obj")) {
      if ((j = get_obj_vis(ch, arg1))) {
     if (IS_SET(j->obj_flags.extra_flags, ITEM_LIMITED) &&
         GET_LEVEL(ch) < LEVEL_DEI) {
       send_to_char("Sorry this item is limited.\n\r", ch);
       return;
     }
     virtual = (j->item_number >= 0) ?
       obj_proto_table[j->item_number].virtual : 0;
     zonenum=inzone(virtual);
     if(zone_locked(ch,zonenum)) return;

     sprintf(buf,
          "Object name: [%s], R-number: [%d], V-number: [%d] Item type: ",
          OBJ_NAME(j), j->item_number, virtual);
     sprinttype(GET_ITEM_TYPE(j),item_types,buf2);
     strcat(buf,buf2); strcat(buf,"\n\r");
     send_to_char(buf, ch);
     sprintf(buf, "Short description: %s\n\rLong description:\n\r%s\n\r",
          ((OBJ_SHORT(j)) ? OBJ_SHORT(j): "None"),
          ((OBJ_DESCRIPTION(j)) ? OBJ_DESCRIPTION(j) : "None") );
     send_to_char(buf, ch);
     if (j->ex_description){
       strcpy(buf, "Extra description keyword(s) [Instance]:\n\r----------\n\r");
       for (desc = j->ex_description; desc; desc = desc->next) {
         strcat(buf, desc->keyword);
         strcat(buf, "\n\r");
       }
       strcat(buf, "----------\n\r");
       send_to_char(buf, ch);
     }
     else if ((j->item_number >= 0) &&
              (j->item_number <= top_of_objt) &&
              obj_proto_table[j->item_number].ex_description) {
       strcpy(buf, "Extra description keyword(s) [Proto]:\n\r----------\n\r");
       for (desc = obj_proto_table[j->item_number].ex_description; desc; desc = desc->next) {
         strcat(buf, desc->keyword);
         strcat(buf, "\n\r");
       }
       strcat(buf, "----------\n\r");
       send_to_char(buf, ch);
     }
     else {
       strcpy(buf,"Extra description keyword(s): None\n\r");
       send_to_char(buf, ch);
     }

        /* Added action descript - Ranger June 96 */
        if(virtual>0) {
          sprintf(buf, "Action description: %s\n\r",
            ((OBJ_ACTION(j)) ? OBJ_ACTION(j) : "None"));
          send_to_char(buf, ch);
        }
        if(virtual>0) {
          sprintf(buf, "Action description (no target): %s\n\r",
            ((OBJ_ACTION_NT(j)) ? OBJ_ACTION_NT(j) : "None"));
          send_to_char(buf, ch);
        }

     send_to_char("Can be worn on :", ch);
     sprintbit(j->obj_flags.wear_flags,wear_bits,buf);
     strcat(buf,"\n\r");
     send_to_char(buf, ch);

     send_to_char("Set char bits  :", ch);
     sprintbit(j->obj_flags.bitvector,affected_bits,buf);
     strcat(buf," ");
     send_to_char(buf,ch);
     sprintbit(j->obj_flags.bitvector2,affected_bits2,buf);
     strcat(buf,"\n\r");
     send_to_char(buf, ch);

     send_to_char("Extra flags: ", ch);
     sprintbit(j->obj_flags.extra_flags,extra_bits,buf);
     strcat(buf," ");
     send_to_char(buf,ch);
     sprintbit(j->obj_flags.extra_flags2,extra_bits2,buf);
     strcat(buf,"\n\r");
     send_to_char(buf,ch);

     send_to_char("Subclass restrictions: ", ch);
     sprintbit(j->obj_flags.subclass_res,subclass_res_bits,buf);
     strcat(buf,"\n\r");
     send_to_char(buf,ch);

     /* Repop added by Ranger - April 6 */

#ifdef TEST_SITE
     if (GET_LEVEL(ch)>LEVEL_SUP) {
#else
     if (GET_LEVEL(ch)>LEVEL_ETE) {
#endif
      if(virtual>0)
        printf_to_char(ch,"Repop Rate: %d\n\r",obj_proto_table[j->item_number].obj_flags.repop_percent);
     }

     printf_to_char(ch,"Weight: %d, Value: %d, Cost/day: %d, Timer: %d Spec Value: %d Popped: %d\n\r",
          j->obj_flags.weight,j->obj_flags.cost,
          j->obj_flags.cost_per_day,  j->obj_flags.timer, j->spec_value, j->obj_flags.popped);

     printf_to_char(ch,"Material Number: %d\n\r",j->obj_flags.material);

     strcpy(buf,"In room: ");
     if (j->in_room == NOWHERE)
       strcat(buf,"Nowhere");
     else {
       sprintf(buf2,"%d",world[j->in_room].number);
       strcat(buf,buf2);
     }
     strcat(buf," ,In object: ");
     strcat(buf, (!j->in_obj ? "None" : fname(OBJ_NAME(j->in_obj))));
     strcat(buf," ,Carried by:");
     strcat(buf, (!j->carried_by) ? "Nobody" : GET_NAME(j->carried_by));
     strcat(buf,"\n\r");
     send_to_char(buf, ch);
     strcpy(buf,"Equipped by: ");
     strcat(buf, (!j->equipped_by) ? "Nobody" : GET_NAME(j->equipped_by));
     strcat(buf," ,Owned by:");
     strcat(buf, (!j->owned_by) ? "Nobody" : GET_NAME(j->owned_by));
     strcat(buf,"\n\r");
     send_to_char(buf, ch);

     strcpy(buf,"Owners: ");
     for(i=0;i<8;i++) {
       if(j->ownerid[i]>0) {
         sprintf(buf2,"%s (%d) ",idname[j->ownerid[i]].name,j->ownerid[i]);
         strcat(buf,CAP(buf2));
       }
     }
     strcat(buf,"\n\r");
     send_to_char(buf, ch);

     switch (j->obj_flags.type_flag) {
     case ITEM_LIGHT :
       printf_to_char(ch,"Colour : [%d]\n\rType : [%d]\n\rHours : [%d]",
            j->obj_flags.value[0], j->obj_flags.value[1],
            j->obj_flags.value[2]);
       break;
     case ITEM_SCROLL :
       printf_to_char(ch,"Spells : %d, %d, %d, %d",
            j->obj_flags.value[0], j->obj_flags.value[1],
            j->obj_flags.value[2], j->obj_flags.value[3] );
       break;
     case ITEM_RECIPE :
       printf_to_char(ch,"Item created : %d  ", j->obj_flags.value[0]);
       printf_to_char(ch,"Needed objects : %d, %d, %d",
           j->obj_flags.value[1], j->obj_flags.value[2], j->obj_flags.value[3]);
       break;
     case ITEM_AQ_ORDER :
       printf_to_char(ch,"Fulfillment Requires Objects : %d, %d, %d, %d",
           j->obj_flags.value[0], j->obj_flags.value[1], j->obj_flags.value[2], j->obj_flags.value[3]);
       break;
     case ITEM_WAND :
       printf_to_char(ch,"Level : %d\n\rMax Charges: %d\n\rCharges Left: %d\n\r Spell : %d\n\r",
          j->obj_flags.value[0], j->obj_flags.value[1], j->obj_flags.value[2],
             j->obj_flags.value[3]);
       break;
     case ITEM_STAFF :
       printf_to_char(ch,"Level : %d\n\rMax Charges: %d\n\rCharges Left: %d\n\r Spell : %d\n\r",
          j->obj_flags.value[0], j->obj_flags.value[1], j->obj_flags.value[2],
             j->obj_flags.value[3]);
       break;
     case ITEM_WEAPON:
     case ITEM_2H_WEAPON:
       if((j->obj_flags.value[0]>-1) && (j->obj_flags.value[0]<64))
         printf_to_char(ch,"Extra : %s(%d)\n\rTodam : %dD%d\n\rType : %d",
            wpn_spc[j->obj_flags.value[0]],j->obj_flags.value[0], j->obj_flags.value[1],
            j->obj_flags.value[2], j->obj_flags.value[3]);
       else if((j->obj_flags.value[0]>300) && (j->obj_flags.value[0]<312))
         printf_to_char(ch,"Extra : %s Weapon(%d)\n\rTodam : %dD%d\n\rType : %d",
            pc_class_types[j->obj_flags.value[0]-300],j->obj_flags.value[0], j->obj_flags.value[1],
            j->obj_flags.value[2], j->obj_flags.value[3]);
       else
         printf_to_char(ch,"Extra : Out of range (%d)\n\rTodam : %dD%d\n\rType : %d",
            j->obj_flags.value[0], j->obj_flags.value[1],
            j->obj_flags.value[2], j->obj_flags.value[3]);
       break;
     case ITEM_FIREARM :
       printf_to_char(ch,"License Number : %d\n\rNumber of bullets left : %d\n\rTodam: %dD%d\n\r",
            j->obj_flags.value[0], j->obj_flags.value[1],
            j->obj_flags.value[2], j->obj_flags.value[3]);
       break;
     case ITEM_MISSILE :
       printf_to_char(ch,"Todam : %dD%d\n\r",
            j->obj_flags.value[0], j->obj_flags.value[1]);
       break;
     case ITEM_ARMOR :
       printf_to_char(ch,"AC-apply : [%d]",
            j->obj_flags.value[0]);
       break;
     case ITEM_POTION :
       printf_to_char(ch,"Spells : %d, %d, %d, %d",
            j->obj_flags.value[0], j->obj_flags.value[1],
            j->obj_flags.value[2], j->obj_flags.value[3]);
       break;
     case ITEM_TRAP :
       printf_to_char(ch,"Spell : %d\n\r- Hitpoints : %d",
            j->obj_flags.value[0], j->obj_flags.value[1]);
       break;
     case ITEM_CONTAINER :
       printf_to_char(ch,"Max-contains : %d\n\rLocktype : %d\n\rCorpse : %s",
            j->obj_flags.value[0], j->obj_flags.value[1],
            j->obj_flags.value[3]?"Yes":"No");
       if(j->obj_flags.value[3]) {
         printf_to_char(ch,"\n\rCorpse Type: %s Skin Value: %d coins",
                        npc_class_types[j->obj_flags.material],j->obj_flags.cost_per_day);
         printf_to_char(ch,"\n\rPossible objects from corpse: %d %d %d %d %d %d",
                        j->obj_flags.skin_vnum[0],j->obj_flags.skin_vnum[1],j->obj_flags.skin_vnum[2],
                        j->obj_flags.skin_vnum[3],j->obj_flags.skin_vnum[4],j->obj_flags.skin_vnum[5]);
       }
       break;
     case ITEM_SKIN :
       printf_to_char(ch,"Values 0-3 : [%d] [%d] [%d] [%d]",
            j->obj_flags.value[0], j->obj_flags.value[1],
            j->obj_flags.value[2], j->obj_flags.value[3]);
       printf_to_char(ch,"\n\rSkin Type: %s Skin Value: %d coins",
                      npc_class_types[j->obj_flags.material],j->obj_flags.cost);
       break;
     case ITEM_DRINKCON :
       sprinttype(j->obj_flags.value[2],drinks,buf2);
       printf_to_char(ch,"Max-contains : %d\n\rContains : %d\n\rPoisoned : %d\n\rLiquid : %s",
            j->obj_flags.value[0], j->obj_flags.value[1],
            j->obj_flags.value[3], buf2);
       break;
     case ITEM_BULLET :
       printf_to_char(ch,"For the gun of license number : %d\n\r",
            j->obj_flags.value[2]);
       break;
     case ITEM_NOTE :
       printf_to_char(ch,"Tongue : %d", j->obj_flags.value[0]);
       break;
     case ITEM_KEY :
       printf_to_char(ch,"Keytype : %d", j->obj_flags.value[0]);
       break;
     case ITEM_FOOD :
       printf_to_char(ch,"Makes full : %d\n\rPoisoned : %d",
            j->obj_flags.value[0], j->obj_flags.value[3]);
       break;
     case ITEM_LOCKPICK :
       printf_to_char(ch,"Number of Picks: %d (Max: %d)",
            j->obj_flags.value[0],j->obj_flags.value[1]);
       break;
     case ITEM_BOARD :
       printf_to_char(ch, "Min Read: %d   Min Write: %d   Min Remove: %d",
            j->obj_flags.value[0],j->obj_flags.value[1],j->obj_flags.value[2]);
       break;
     default :
       printf_to_char(ch,"Values 0-3 : [%d] [%d] [%d] [%d]",
            j->obj_flags.value[0], j->obj_flags.value[1],
            j->obj_flags.value[2], j->obj_flags.value[3]);
       break;
     }

     strcpy(buf,"\n\rEquipment Status: ");
     if (!j->carried_by)
       strcat(buf,"NONE");
     else {
       found = FALSE;
       for (i=0;i < MAX_WEAR;i++) {
         if (j->carried_by->equipment[i] == j) {
           sprinttype(i,equipment_types,buf2);
           strcat(buf,buf2);
           found = TRUE;
         }
       }
       if (!found)
         strcat(buf,"Inventory");
     }
     send_to_char(buf, ch);

     strcpy(buf, "\n\rSpecial procedure : ");
     if (j->item_number >= 0)
       strcat(buf, (obj_proto_table[j->item_number].func ? "exists\n\r" : "No\n\r"));
     else
       strcat(buf, "No\n\r");
     send_to_char(buf, ch);

     strcpy(buf, "Quested equipment: ");
     if (diff_obj_stats(j)) strcat(buf, "Yes\n\r");
     else strcat(buf, "No\n\r");
     send_to_char(buf, ch);

     strcpy(buf, "Contains :\n\r");
     found = FALSE;
     for (j2=j->contains;j2;j2 = j2->next_content) {
       strcat(buf,OBJ_SHORT(j2));
       strcat(buf,"\n\r");
       found = TRUE;
     }
     if (!found)
       strcpy(buf,"Contains : Nothing\n\r");
     send_to_char(buf, ch);

     send_to_char("Can affect char :\n\r", ch);
     for (i=0;i<MAX_OBJ_AFFECT;i++) {
       sprinttype(j->affected[i].location,apply_types,buf2);
       sprintf(buf,"    Affects : %s By %d\n\r", buf2,j->affected[i].modifier);
       send_to_char(buf, ch);
     }
      return;
      }
      else {
     send_to_char("No such object.\n\r", ch);
     return;
      }
    } else if(is_abbrev(type, "world")){
      i2=0;
      for(i=0; i <= top_of_mobt; i++)
     i2+=mob_proto_table[i].number;
      sprintf(buf,"Mobiles : %d\n\r",i2);
      send_to_char(buf, ch);
      i2=0;
      for(i=0; i <= top_of_objt; i++)
     i2+=obj_proto_table[i].number;
      sprintf(buf,"Objects : %d\n\r",i2);
      send_to_char(buf, ch);
      sprintf(buf,"top of objects: %d top of mobiles: %d top of world %d\n\r",
           top_of_objt, top_of_mobt, top_of_world);
      send_to_char(buf, ch);
      return;
    } else if (is_abbrev(type, "char")) {
      /* mobile in world */
      if ((k = get_char_vis(ch, arg1))) {
        if(IS_NPC(k)) {
         zonenum=inzone(V_MOB(k));
         if(zone_locked(ch,zonenum)) return;
        }

     switch(k->player.sex) {
     case SEX_NEUTRAL :
       strcpy(buf,"NEUTRAL-SEX");
       break;
     case SEX_MALE :
       strcpy(buf,"MALE");
       break;
     case SEX_FEMALE :
       strcpy(buf,"FEMALE");
       break;
     default:
       strcpy(buf,"ILLEGAL-SEX!!");
       break;
     }

     sprintf(buf2, " %s - Name : %s (%d)         [R-Number%d], In room [%d]\n\r",
          (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")),
          GET_NAME(k),GET_ID(k), k->nr, world[CHAR_REAL_ROOM(k)].number);
     strcat(buf, buf2);
     send_to_char(buf, ch);
     if (IS_MOB(k)) {
       sprintf(buf, "V-Number [%d]\n\r", mob_proto_table[k->nr].virtual);
       send_to_char(buf, ch);
     }

     strcpy(buf,"Short description: ");
     strcat(buf, GET_SHORT(k) );
     strcat(buf,"\n\r");
     send_to_char(buf,ch);

     strcpy(buf,"Title: ");
     strcat(buf, (k->player.title ? k->player.title : "None"));
     strcat(buf,"\n\r");
     send_to_char(buf,ch);

     send_to_char("Long description: ", ch);
     send_to_char(GET_LONG(k), ch);
     send_to_char("\n\r", ch);

     strcpy(buf,"Bamfin/Walkin: ");
     strcat(buf, (k->player.poofin ? k->player.poofin : "None"));
     strcat(buf,"\n\r");
     send_to_char(buf,ch);

     strcpy(buf,"Bamfout/Walkout: ");
     strcat(buf, (k->player.poofout ? k->player.poofout : "None"));
     strcat(buf,"\n\r");
     send_to_char(buf,ch);

     if (IS_NPC(k)) {
       strcpy(buf,"Monster Class: ");
       sprinttype(k->player.class,npc_class_types,buf2);
     } else {
       strcpy(buf,"Class: ");
       sprinttype(k->player.class,pc_class_types,buf2);
     }
     strcat(buf, buf2);

     sprintf(buf2,"   Level [%d]   Alignment [%d]\n\r",k->player.level,
          k->specials.alignment);
     strcat(buf, buf2);
     send_to_char(buf, ch);

     if(!IS_NPC(k)) {
       if(k->ver3.created<10000000)
         sprintf(buf,"Created: [0%d] (mmddyyyy)\n\r",k->ver3.created);
       else
         sprintf(buf,"Created: [%d] (mmddyyyy)\n\r",k->ver3.created);
       send_to_char(buf,ch);
       sprintf(buf,"Age: [%ld] (secs)\n\r",(long)k->player.time.birth);
       send_to_char(buf,ch);
       ct=(long)k->player.time.logon;
       tmstr = asctime(localtime(&ct));
       *(tmstr+strlen(tmstr)-1)='\0';
       sprintf(buf,"Last Logon : [%s]",tmstr);
       sprintf(buf2,"  Played[%ld]secs\n\r",(long)k->player.time.played);
       strcat(buf, buf2);
       send_to_char(buf, ch);

       sprintf(buf,"Mother-Host : [%s]\n\r", k->new.host);
       send_to_char(buf, ch);
       if(k->desc) {
         isa.sin_addr.s_addr=k->desc->addr;
         sprintf(buf,"Present-Host: [%s] (%s)\n\r", k->desc->host,inet_ntoa(isa.sin_addr));
       }
       else
         sprintf(buf,"Present-Host: [LINK-DEAD]\n\r");
       send_to_char(buf, ch);

       if(k->desc)
         sprintf(buf, "Desc: [#%d]         Idle Time: [%d mins]\n\r",
              k->desc->descriptor, k->specials.timer);
       else
         sprintf(buf, "Desc: [NONE]         Idle Time: [?? mins]\n\r");
       send_to_char(buf, ch);

       strcpy(buf,"Email Address: ");
       strcat(buf, (GET_EMAIL(k) ? GET_EMAIL(k) : "None"));
       strcat(buf,"\n\r");
       send_to_char(buf,ch);
       strcpy(buf,"Postcard Forwarding to Email: ");
       strcat(buf, (IS_SET(k->specials.pflag,PLR_EMAIL) ? "Yes\n\r" : "No\n\r"));
       send_to_char(buf,ch);
     }

     sprintf(buf,"Age: [%d] Years,  [%d] Months,  [%d] Days,  [%d] Hours\n\r",
          age(k).year, age(k).month, age(k).day, age(k).hours);
     send_to_char(buf,ch);

     sprintf(buf,"Height [%d]cm  Weight [%d]pounds \n\r",
          GET_HEIGHT(k), GET_WEIGHT(k));
     send_to_char(buf,ch);

     sprintf(buf,"Hometown[%d], Speaks[%d/%d/%d], (STL[%d]/per[%d]/NSTL[%d])\n\r",
          k->player.hometown, k->player.talks[0], k->player.talks[1],
          k->player.talks[2], k->specials.spells_to_learn,
          int_app[GET_INT(k)].learn, wis_app[GET_WIS(k)].bonus);
     send_to_char(buf, ch);

     sprintf(buf, "Killed [%d] monsters, Been killed [%d] times.\n\r",
          k->new.killed, k->new.been_killed);
     send_to_char(buf, ch);

     /* Total added by Ranger April 96 */
        total_stats=GET_STR(k)+GET_INT(k)+GET_WIS(k)+GET_DEX(k)+GET_CON(k);
        if(!IS_NPC(k)) total_real_stats=k->abilities.intel+k->abilities.wis+k->abilities.dex+k->abilities.con+k->abilities.str;
        else total_real_stats=0;

     sprintf(buf,"Str:[%d/%d]  Int:[%d]  Wis:[%d]  Dex:[%d]  Con:[%d]  Total/Unaff:[%d/%d]\n\r",
          GET_STR(k), GET_ADD(k), GET_INT(k), GET_WIS(k),
          GET_DEX(k), GET_CON(k), total_stats,total_real_stats);
     send_to_char(buf,ch);

     sprintf(buf,"Mana p.:[%d/%d+%d]  Hit p.:[%d/%d+%d]  Move p.:[%d/%d+%d]\n\r",
          GET_MANA(k),mana_limit(k),mana_gain(k),
          GET_HIT(k),hit_limit(k),hit_gain(k),
          GET_MOVE(k),move_limit(k),move_gain(k) );
     send_to_char(buf,ch);

     arm = k->points.armor+dex_app[GET_DEX(k)].defensive;
     if(!IS_NPC(k)) {
     if(affected_by_spell(k, SKILL_DEFEND))
       arm = MAX(-300, arm);
     else
       arm = MAX(-250, arm);
     }

     sprintf(buf,"AC:[%d/10], Exp: [%d], Hitroll: [%d], Damroll: [%d]\n\r",
          arm, GET_EXP(k), k->points.hitroll, k->points.damroll );
     send_to_char(buf,ch);

     if (GET_REMORT_EXP(k))
     {
       sprintf(buf, "Remort Exp: [%lld]\n\r", GET_REMORT_EXP(k));
       send_to_char(buf, ch);
     }

     if (GET_DEATH_EXP(k))
     {
       sprintf(buf, "Death Exp: [%lu]\n\r", GET_DEATH_EXP(k));
       send_to_char(buf, ch);
     }

     sprintf(buf,"Coins: [%d], Bank: [%d], TOTAL: [%d]\n\r",
          GET_GOLD(k), GET_BANK(k), (GET_GOLD(k) + GET_BANK(k)));
     send_to_char(buf,ch);

     sprinttype(GET_POS(k),position_types,buf2);
     sprintf(buf,"Position: %s, Fighting: %s",buf2,
          ((k->specials.fighting) ? GET_NAME(k->specials.fighting) : "Nobody") );
     if (k->desc) {
       sprinttype(k->desc->connected,connected_types,buf2);
       strcat(buf,", Connected: ");
       strcat(buf,buf2);
     }
     strcat(buf,"\n\r");
     send_to_char(buf, ch);

     strcpy(buf,"Default position: ");
     sprinttype((k->specials.default_pos),position_types,buf2);
     strcat(buf, buf2);
     if (IS_NPC(k)) {
       strcat(buf,",NPC flags: ");
       sprintbit(k->specials.act,action_bits,buf2);
       strcat(buf, buf2);
       strcat(buf, " ");
       sprintbit(k->specials.act2,action_bits2,buf2);
       strcat(buf, buf2);
     } else {
       strcat(buf,",PC flags: ");
       sprintbit(k->specials.pflag,player_bits,buf2);
       strcat(buf, buf2);
     }

     sprintf(buf2,",Timer [%d] \n\r", k->specials.timer);
     strcat(buf, buf2);
     send_to_char(buf, ch);

        /* New stuff - Ranger Sept 96 */
     if(IS_MOB(k)) {
       zonenum=inzone(V_MOB(k));
       if (zonenum!=-1 && (GET_LEVEL(ch) > LEVEL_ETE || isname(GET_NAME(ch),zone_table[real_zone(zonenum)].name) || strstr(zone_table[real_zone(zonenum)].creators,GET_NAME(ch)))) {
          strcpy(buf,"Mobile Immunities: ");
          sprintbit(k->specials.immune,immune_bits,buf2);
          strcat(buf,buf2);
          strcat(buf, " ");
          sprintbit(k->specials.immune2,immune_bits2,buf2);
          strcat(buf,buf2);
          strcat(buf,"\n\r");
          send_to_char(buf,ch);

          strcpy(buf,"Mobile Resistance: ");
          sprintbit(k->specials.resist,resist_bits,buf2);
          strcat(buf,buf2);
          strcat(buf,"\n\r");
          send_to_char(buf,ch);

          sprintf(buf,"Mobile Attacks: %d Timer: %d\n\r",k->specials.no_att,k->specials.att_timer);
          send_to_char(buf,ch);
          if(k->specials.no_att>0) {
             for (i=0;i<k->specials.no_att;i++) {
                if(i==MAX_ATTS) break;
                sprintf(buf,"%d) ",i+1);
                sprinttype(k->specials.att_type[i],att_types,buf2);
                strcat(buf2," ");
                strcat(buf,buf2);
                sprinttype(k->specials.att_target[i],att_targets,buf2);
                strcat(buf2," ");
                strcat(buf,buf2);
                sprintf(buf2,"%d ",k->specials.att_percent[i]);
                strcat(buf,buf2);
                if(k->specials.att_type[i]==ATT_SPELL_CAST) {
               sprinttype(k->specials.att_spell[i]-1,spells,buf2);
                  strcat(buf,CAP(buf2));
            strcat(buf," (cast)");
                }
                if(k->specials.att_type[i]==ATT_SPELL_SKILL) {
               sprinttype(k->specials.att_spell[i]-1,spells,buf2);
                  strcat(buf,CAP(buf2));
            strcat(buf," (skill)");
                }

                strcat(buf,"\n\r");
                send_to_char(buf,ch);
             }
         } /* if attack */
         printf_to_char(ch,"Skin Value: %d coins\n\r",mob_proto_table[k->nr].skin_value);
         printf_to_char(ch,"Possible objects from corpse: %d %d %d %d %d %d\n\r",
                        mob_proto_table[k->nr].skin_vnum[0], mob_proto_table[k->nr].skin_vnum[1],
                        mob_proto_table[k->nr].skin_vnum[2], mob_proto_table[k->nr].skin_vnum[3],
                        mob_proto_table[k->nr].skin_vnum[4], mob_proto_table[k->nr].skin_vnum[5]);

         printf_to_char(ch,"Zone Multipliers:  Hp Mana Hitroll Damage Armor  Xp Gold Level\n\r");
         printf_to_char(ch,"                  %3d  %3d   %3d    %3d    %3d  %3d  %3d  %3d\n\r",
                        zone_table[real_zone(zonenum)].mult_hp,zone_table[real_zone(zonenum)].mult_mana,
                        zone_table[real_zone(zonenum)].mult_hitroll,zone_table[real_zone(zonenum)].mult_damage,
                        zone_table[real_zone(zonenum)].mult_armor,zone_table[real_zone(zonenum)].mult_xp,
                        zone_table[real_zone(zonenum)].mult_gold,zone_table[real_zone(zonenum)].mult_level);
       } /* if zone */
       strcpy(buf, "Mobile Special procedure : ");
       strcat(buf, (mob_proto_table[k->nr].func ? "Exists\n\r" : "None\n\r"));
       send_to_char(buf, ch);
    } /* is mob */

     maxdam=(k->specials.damnodice*k->specials.damsizedice)+k->points.damroll;
     mindam=(k->specials.damnodice+k->points.damroll);
     avgdam=((k->specials.damnodice+k->specials.damnodice*k->specials.damsizedice)/2)+ k->points.damroll;
     sprintf(apt, ((mindam+maxdam)%2 ? ".5" : " "));
     if (IS_SET(k->specials.affected_by,AFF_FURY)) {
        avgdam  *= 2;
        maxdam  *= 2;
        mindam  *= 2;
        if((mindam+maxdam)%4) {avgdam+=1;sprintf(apt," ");}
     }

     if (IS_NPC(k)) {
       sprintf(buf, "Bare Hand Dmg %dd%d (%ld-%ld, Avg. %ld%s).\n\r",
          k->specials.damnodice, k->specials.damsizedice,mindam,maxdam,avgdam,apt);
       send_to_char(buf, ch);
     }
     wielded=EQ(k,WIELD);
     if(wielded) {
       maxdam=(wielded->obj_flags.value[1]*wielded->obj_flags.value[2])
           +k->points.damroll;
       mindam=(wielded->obj_flags.value[1]+ k->points.damroll);
       avgdam=((wielded->obj_flags.value[1]+wielded->obj_flags.value[1]
            *wielded->obj_flags.value[2])/2)+ k->points.damroll;

       if(!IS_NPC(k)) {
         mindam+=str_app[STRENGTH_APPLY_INDEX(k)].todam;
         maxdam+=str_app[STRENGTH_APPLY_INDEX(k)].todam;
         avgdam+=str_app[STRENGTH_APPLY_INDEX(k)].todam;
       }
        sprintf(apt, ((mindam+maxdam)%2 ? ".5" : " "));
        if (IS_SET(k->specials.affected_by,AFF_FURY)) {
            avgdam  *= 2;
            maxdam  *= 2;
            mindam  *= 2;
          if((mindam+maxdam)%4) {avgdam+=1;sprintf(apt," ");}
          }

          sprintf(buf, "Weapon damage %ld-%ld (Avg. %ld%s).  ",mindam,maxdam,avgdam,apt);
          send_to_char(buf, ch);
        }
        if (GET_CLASS(k)==CLASS_NINJA) {
          hold=EQ(k,HOLD);
          if (hold)  {
            maxdam=(hold->obj_flags.value[1]*hold->obj_flags.value[2])+ k->points.damroll;
            mindam=(hold->obj_flags.value[1]+ k->points.damroll);
            avgdam=((hold->obj_flags.value[1]+hold->obj_flags.value[1]
               *hold->obj_flags.value[2])/2)+k->points.damroll;

            if(!IS_NPC(k)) {
              mindam+=str_app[STRENGTH_APPLY_INDEX(k)].todam;
              maxdam+=str_app[STRENGTH_APPLY_INDEX(k)].todam;
              avgdam+=str_app[STRENGTH_APPLY_INDEX(k)].todam;
            }
             sprintf(apt, ((mindam+maxdam)%2 ? ".5" : " "));
            if (IS_SET(k->specials.affected_by,AFF_FURY)) {
              avgdam  *= 2;
              maxdam  *= 2;
              mindam  *= 2;
          if((mindam+maxdam)%4) {avgdam+=1;sprintf(apt," ");}
            }
            sprintf(buf2, "Secondary weapon damage %ld-%ld (Avg. %ld%s).\n\r",
            mindam,maxdam,avgdam,apt);
            send_to_char(buf2,ch);
          }
        }
     sprintf(buf,"Carried weight: %d   Carried items: %d\n\r",
          IS_CARRYING_W(k), IS_CARRYING_N(k) );
     send_to_char(buf,ch);

     for (i=0,j=k->carrying;j;j=j->next_content,i++);
     sprintf(buf,"Items in inventory: %d, ",i);

     for (i=0,i2=0;i<MAX_WEAR;i++)
       if (k->equipment[i]) i2++;
     sprintf(buf2,"Items in equipment: %d\n\r", i2);
     strcat(buf,buf2);
     send_to_char(buf, ch);

     sprintf(buf,"Apply saving throws: [%d] [%d] [%d] [%d] [%d]\n\r",
          k->specials.apply_saving_throw[0],
          k->specials.apply_saving_throw[1],
          k->specials.apply_saving_throw[2],
          k->specials.apply_saving_throw[3],
          k->specials.apply_saving_throw[4]);
     send_to_char(buf, ch);

     sprintf(buf, "Thirst: %d, Hunger: %d, Drunk: %d\n\r",
          k->specials.conditions[THIRST],
          k->specials.conditions[FULL],
          k->specials.conditions[DRUNK]);
     send_to_char(buf, ch);

     if(GET_LEVEL(ch)>LEVEL_IMM) {
       sprintf(buf, "Current Ranking: %d\n\r",k->ver3.ranking);
       send_to_char(buf,ch);
     }
     sprintf(buf, "Death Limit: %d\n\r",k->ver3.death_limit);
     send_to_char(buf, ch);
     sprintf(buf, "Death Timer: %d\n\r",k->specials.death_timer-1);
     send_to_char(buf, ch);
     sprintf(buf, "Bleed Limit: %d\n\r",k->ver3.bleed_limit);
     send_to_char(buf, ch);
     sprintf(buf, "SubClass Points: %d\n\r",k->ver3.subclass_points);
     send_to_char(buf, ch);
     sprintf(buf, "Quest Points: %d\n\r",k->ver3.quest_points);
     send_to_char(buf, ch);
     sprintf(buf, "Time to next quest: %d ticks\n\r",k->ver3.time_to_quest);
     send_to_char(buf, ch);
     sprintf(buf, "Score style: %d\n\r",k->ver3.sc_style);
     send_to_char(buf, ch);
     if(k->ver3.subclass) {
       sprintf(buf, "Subclass: %s Level: %d\n\r",subclass_name[k->ver3.subclass-1],k->ver3.subclass_level);
       send_to_char(buf,ch);
     }

     if(k->questgiver) {
       sprintf(buf,"Quest Giver: %s\n\r",GET_SHORT(k->questgiver));
       send_to_char(buf,ch);
     }
     if(k->questmob) {
       sprintf(buf,"Quest Mob: %s\n\r",GET_SHORT(k->questmob));
       send_to_char(buf,ch);
     }
     if(k->questobj) {
       sprintf(buf,"Quest Obj: %s\n\r",OBJ_SHORT(k->questobj));
       send_to_char(buf,ch);
     }
     if(k->questowner) {
       sprintf(buf,"Quest Owner: %s\n\r",GET_NAME(k->questowner));
       send_to_char(buf,ch);
     }
     if(k->quest_status) {
       sprintf(buf,"Quest Status: %d\n\r",k->quest_status);
       send_to_char(buf,ch);
     }
     if(k->quest_level) {
       sprintf(buf,"Quest Level: %d\n\r",k->quest_level);
       send_to_char(buf,ch);
     }

     if(!IS_NPC(k)) {
       sprintf(buf, "Current Vault: %s  Access: %d\n\r",
                  k->specials.vaultname,k->specials.vaultaccess);
       send_to_char(buf, ch);
       if(k->ver3.clan_num) {
         i=k->ver3.clan_num;
         sprintf(buf, "Clan: %s\n\r",clan_list[i].name);
         send_to_char(buf,ch);
       }
       if(k->desc) {
         if(k->desc->snoop.snoop_by && GET_LEVEL(ch)>LEVEL_SUP) {
           sprintf(buf, "Snooped by: %s\n\r",GET_NAME(k->desc->snoop.snoop_by));
           send_to_char(buf,ch);
         }
       }
       if(GET_LEVEL(ch)>LEVEL_SUP && k->specials.reply_to && get_ch_by_id(k->specials.reply_to)) {
         sprintf(buf, "Reply to: %s\n\r",GET_NAME(get_ch_by_id(k->specials.reply_to)));
         send_to_char(buf,ch);
       }
       if(k->specials.zone!=-1) {
          sprintf(buf, "Editting zone %d - %s.\n\r", k->specials.zone,zone_table[real_zone(k->specials.zone)].name);
          send_to_char(buf, ch);
       }
     }

     sprintf(buf, "Master is '%s'\n\r",
          ((k->master) ? GET_NAME(k->master) : "NOBODY"));
     if(k->specials.rider) {
       sprintf(buf, "Being ridden by %s.\n\r",GET_NAME(k->specials.rider));
       send_to_char(buf, ch);
     }
     if(k->specials.riding) {
       sprintf(buf, "Riding %s.\n\r",GET_NAME(k->specials.riding));
       send_to_char(buf, ch);
     }
     send_to_char("Followers are:\n\r", ch);
     for (fol=k->followers; fol; fol = fol->next)
       act("    $N", FALSE, ch, 0, fol->follower, TO_CHAR);

     /* Showing the wiz bitvector */
     sprintbit(k->new.imm_flags, wiz_bits, buf);
     send_to_char("Wiz Flags: ", ch);
     strcat(buf,"\n\r");
     send_to_char(buf, ch);

     send_to_char("Affected by: ", ch);
     sprintbit(k->specials.affected_by,affected_bits,buf);
     send_to_char(buf, ch);
     send_to_char("\n\r             ", ch);
     sprintbit(k->specials.affected_by2,affected_bits2,buf);
     strcat(buf,"\n\r");
     send_to_char(buf, ch);

     /* Routine to show what spells a char is affected by */
     if (k->affected) {
       send_to_char("\n\rAffecting Spells:\n\r--------------\n\r", ch);
       for (aff = k->affected; aff; aff = aff->next) {
         sprintf(buf, "Spell : '%s'\n\r",spells[aff->type-1]);
         send_to_char(buf, ch);
         sprintf(buf,"     Modifies %s by %d points\n\r",
              apply_types[(int)aff->location], aff->modifier);
         send_to_char(buf, ch);
         sprintf(buf,"     Expires in %3d ticks, Bits set ",
              aff->duration);
         send_to_char(buf, ch);
         sprintbit(aff->bitvector,affected_bits,buf);
         strcat(buf,"\n\r                                    ");
         send_to_char(buf, ch);
         sprintbit(aff->bitvector2,affected_bits2,buf);
         strcat(buf,"\n\r");
         send_to_char(buf, ch);
       }
     }
     if (k->enchantments) {
       send_to_char("\n\rEnchantments:\n\r--------------\n\r", ch);
       for(ench = k->enchantments;ench;ench = ench->next) {
         sprintf(buf,"   '%s'\n\r",ench->name);
         send_to_char(buf,ch);
       }
     }
     if(GET_LEVEL(ch)>LEVEL_SUP) {
       sprintf(buf,"Number fighting: %d (Max %d).\n\r",k->specials.num_fighting,k->specials.max_num_fighting);
       send_to_char(buf,ch);
     }
     return;
   } else {
     send_to_char("No mobile by that name in the world\n\r", ch);
      }
    }
  }
}

int copyover_write(int same_room);
void do_shutdown(struct char_data *ch, char *argument, int cmd)
{
  char buf[100],arg[MAX_INPUT_LENGTH];
  struct char_data *i=0;
  int next_boot;
  FILE *fl;
  if(!check_god_access(ch,TRUE)) return;

  argument=one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Usage: shutdown <full/reboot/hotboot/disable/type #/reset # (hr)>\n\r",ch);
    return;
  }

  if (!strcmp(arg, "full") && (GET_LEVEL(ch)>LEVEL_SUP)) {
    sprintf(buf, "Shutdown by %s.", GET_NAME(ch) );
    log_s(buf);
    cleanshutdown = 1;
  }
  else if (!strcmp(arg, "reboot")) {
    one_argument(argument,arg);
    if(*arg) {
      send_to_char("You added an argument after reboot. Do you really mean to reboot?\n\r",ch);
      return;
    }
    sprintf(buf, "Reboot by %s.\n\r", GET_NAME(ch));
    send_to_all(buf);
    log_s(buf);
    cleanshutdown = cleanreboot = 1;
  }
  else if (!strcmp(arg, "reset")) {
    one_argument(argument,arg);
    if(!(*arg) || !is_number(arg)) {
      send_to_char("Please supply an hour to reboot.\n\r",ch);
    }
    next_boot=atoi(arg);
    if(next_boot<0 || next_boot>23) {
      send_to_char("The reboot time must be between 0 and 23 hrs.\n\r",ch);
      return;
    }
    REBOOT_AT=next_boot;
    sprintf(buf, "Reboot time reset to %d:00 hours.\n\r",REBOOT_AT);
    send_to_all(buf);
    log_s(buf);
    next_boot=REBOOT_AT-2;
    if(next_boot<0) next_boot+=24;
    fl=fopen("reboot_time","w");
    fprintf(fl,"%d\n",next_boot);
    fclose(fl);
  }
  else if (!strcmp(arg, "type")) {
    one_argument(argument,arg);
    if(!(*arg) || !is_number(arg)) {
      send_to_char("Usage: shutdown <type 0> (normal) or <type 1> (hotboot).\n\r",ch);
      return;
    }
    reboot_type=atoi(arg);
    if(reboot_type!=0 && reboot_type != 1) {
      send_to_char("Usage: shutdown <type 0> (normal) or <type 1> (hotboot).\n\r",ch);
      return;
    }
    if(reboot_type)
      printf_to_char(ch,"Reboot type set to hotboot.\n\r");
    else
      printf_to_char(ch,"Reboot type set to normal.\n\r");
  }
  else if (!strcmp(arg, "disable")) {
    if(!disablereboot) {
      sprintf(buf, "Reboot disabled by %s.\n\r", GET_NAME(ch));
      disablereboot = 1;
      for (i=character_list; i; i=i->next) {
        if (IS_NPC(i) && V_MOB(i)==3) {
          i->specials.timer=0;
          break;
        }
      }
    }
    else {
      sprintf(buf,"Reboot reenabled by %s.\n\r",GET_NAME(ch));
      disablereboot=0;
    }
    send_to_all(buf);
    log_s(buf);
  }
  else if(!strcmp(arg,"hotboot")) {
    if(!copyover_write(0)) {
      send_to_char("Hotboot failed for some reason.  Check the log.\n\r",ch);
      return;
    }
  }
  else
    send_to_char("Go shut down someone your own size.\n\r", ch);
}


void do_snoop(struct char_data *ch, char *argument, int cmd)
{
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  struct char_data *victim;

  if (!ch->desc) return;

  if(!check_god_access(ch,TRUE)) return;

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Usage: snoop <victim>\n\r       snoop brief\n\r",ch);
    return;
  }

  if(is_abbrev(arg,"brief")) {
    if(IS_SET(ch->new.imm_flags, WIZ_SNOOP_BRIEF)) {
      send_to_char("You have turned brief snooping off. You will now see combat.\n\r", ch);
      REMOVE_BIT(ch->new.imm_flags, WIZ_SNOOP_BRIEF);
    } else {
      send_to_char("You have turned brief snooping on.  You will not see combat.\n\r", ch);
      SET_BIT(ch->new.imm_flags, WIZ_SNOOP_BRIEF);
    }
    return;
  }

  if (!(victim=get_char_vis(ch, arg))) {
    send_to_char("No such person around.\n\r",ch);
    return;
  }

  if (!victim->desc) {
    send_to_char("There's no link.. nothing to snoop.\n\r",ch);
    return;
  }

  if (victim == ch) {
    send_to_char("Ok, you just snoop yourself.\n\r",ch);
    if (ch->desc->snoop.snooping) {
      ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
      ch->desc->snoop.snooping = 0;
      sprintf (buf,"WIZINFO: %s snoops him/herself.", GET_NAME(ch));
      wizlog(buf, GET_LEVEL(ch)+1, 5);
    }
    return;
  }

  if (victim->desc->snoop.snoop_by) {
    send_to_char("Busy already. \n\r",ch);
    return;
  }

  if (GET_LEVEL(victim)>=GET_LEVEL(ch)) {
    send_to_char("You failed.\n\r",ch);
    return;
  }

  send_to_char("Ok. \n\r",ch);

  if (GET_LEVEL(ch) < LEVEL_DEI) {
    sprintf(buf,"You are snooped by %s.", GET_NAME(ch));
    act(buf, FALSE, victim, 0, 0, TO_CHAR);
  }

  if (ch->desc->snoop.snooping)
    ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;

  ch->desc->snoop.snooping = victim;
  victim->desc->snoop.snoop_by = ch;
  sprintf (buf,"WIZINFO: %s is snooping %s", GET_NAME(ch), GET_NAME(victim));
  log_s(buf);
  wizlog(buf, GET_LEVEL(ch)+1, 5);

  return;
}

void do_mobswitch(struct char_data *ch, char *argument, int cmd)
{
  char arg[MSL];
  char buf[MIL];
  CHAR *victim = NULL;
  int zonenum = 0;

  if(!check_god_access(ch,TRUE)) return;
  if(IS_NPC(ch)) return;

  if (!IS_SET(ch->new.imm_flags, WIZ_QUEST))
  {
    send_to_char("You need a quest flag!\n\r", ch);
    return;
  }

  one_argument(argument, arg);

  if (!*arg) {
    send_to_char("Switch with who?\n\r", ch);
  }
  else {
    if (!(victim = get_char(arg))) {
      send_to_char("They aren't here.\n\r", ch);
    }
    else {
      if (ch == victim) {
        send_to_char("He he he... We are jolly funny today, eh?\n\r", ch);
        return;
      }

      if (IS_NPC(victim)) {
        zonenum=inzone(V_MOB(victim));
        if(zone_locked(ch,zonenum)) return;
      }

      if (!ch->desc ||
          ch->desc->snoop.snoop_by ||
          ch->desc->snoop.snooping) {
        if (ch->desc->snoop.snoop_by) {
          send_to_char("Victim switched, snooping off.\n\r",ch->desc->snoop.snoop_by);
          ch->desc->snoop.snoop_by->desc->snoop.snooping=0;
          ch->desc->snoop.snoop_by=0;
        }
        else {
          send_to_char("Mixing snoop and switch is bad for your health.\n\r", ch);
          return;
        }
      }

      if (victim->desc) {
        send_to_char("You can't do that; the body is already in use!\n\r", ch);
        return;
      }

      if (!IS_NPC(victim) && !IS_IMPLEMENTOR(ch)) {
        send_to_char("You can't do that to another player!\n\r", ch);
        return;
      }

      send_to_char("Ok.\n\r", ch);
      sprintf (buf,"WIZINFO: %s switched to %s", GET_NAME(ch), GET_NAME(victim));
      log_s(buf);
      wizlog(buf, GET_LEVEL(ch)+1, 5);
      ch->desc->character = victim;
      ch->desc->original = ch;
      victim->desc = ch->desc;
      ch->switched = victim;
      ch->desc = 0;
    }
  }
}

void do_return(struct char_data *ch, char *argument, int cmd)
{
  char buf[255];
  if (!ch->desc) return;

  if (!ch->desc->original) {
    send_to_char("Arglebargle, glop-glyf!?!\n\r", ch);
    return;
  } else {
    send_to_char("You return to your original body.\n\r",ch);
    sprintf (buf,"WIZINFO: %s has returned to his/her original body.", GET_NAME(ch->desc->original));
    wizlog(buf, GET_LEVEL(ch->desc->original)+1, 5);
/*    if(IS_NPC(ch->desc->character)) {
      if(IS_SET(mob_proto_table[ch->desc->character->nr].affected_by, AFF_HIDE))
        SET_BIT(ch->desc->character->specials.affected_by, AFF_HIDE);
    }*/
    if(ch->desc->character->desc->snoop.snoop_by) {
      send_to_char("Victim returned, snooping off.\n\r",ch->desc->character->desc->snoop.snoop_by);
      ch->desc->character->desc->snoop.snoop_by->desc->snoop.snooping = 0;
      ch->desc->character->desc->snoop.snoop_by=0;
    }
    ch->desc->original->switched=0;
    ch->desc->character = ch->desc->original;
    ch->desc->original = 0;
    ch->desc->character->desc = ch->desc;
    ch->desc = 0;
  }
}


void do_force(struct char_data *ch, char *argument, int cmd)
{
  struct descriptor_data *i;
  struct char_data *vict;
  char name[100], to_force[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
  char logbuf[MAX_STRING_LENGTH];

  if(!check_god_access(ch,TRUE)) return;

  half_chop(argument, name,100,to_force,MIL);

  if (!*name || !*to_force) {
    send_to_char("Who do you wish to force to do what?\n\r", ch);
    return;
  }
  if (strcmp("all", name)) {
    if(!isdigit(*name)) {
      if (!(vict = get_char_vis(ch, name))) {
        send_to_char("No-one by that name here..\n\r", ch);
        return;
      }
    }
    else {
      if (!(vict = get_mob_vis(ch, name))) {
        send_to_char("No-mob by that name here..\n\r", ch);
        return;
      }
    }
    if ((GET_LEVEL(ch) < GET_LEVEL(vict)) && !IS_NPC(vict) ) {
      send_to_char("The victim's level is too high.\n\r",ch);
    }
    else {
      sprintf(logbuf,"WIZINFO: %s has forced %s to '%s'",
              GET_NAME(ch), GET_NAME(vict), to_force);
      log_s(logbuf);
      wizlog(logbuf, GET_LEVEL(ch), 5);
      sprintf(buf, "$n has forced you to '%s'.", to_force);
      act(buf, FALSE, ch, 0, vict, TO_VICT);
      send_to_char("Ok.\n\r", ch);
      command_interpreter(vict, to_force);
    }
  } else { /* force all */
    for (i = descriptor_list; i; i = i->next)
      if (i->character != ch && !i->connected) {
        vict = i->character;
        if (GET_LEVEL(ch) < GET_LEVEL(vict)) {  }
        else {
          sprintf(buf, "$n has forced you to '%s'.", to_force);
          act(buf, FALSE, ch, 0, vict, TO_VICT);
          command_interpreter(vict, to_force);
        }
      }
    send_to_char("Ok.\n\r", ch);
  }
}

int boot_area(char *name);
int adjust_ticket_strings(OBJ *obj); /*Added Oct 98 Ranger */

void do_load(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *mob;
  struct obj_data *obj;
  char args[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
  char *type, *arg1;
  int i,number = 0, r_num,tmp,zonenum;
  if (IS_NPC(ch)) return;

  if(!check_god_access(ch,FALSE)) return;

  if (!IS_SET(ch->new.imm_flags, WIZ_LOAD)) {
    send_to_char("You need to ask a high level god for this power!\n\r", ch);
    return;
  }

  sprintf(args, "%s", argument);
  type = strtok(args, " ");


  if (type) {
    tmp = -1;
    if (is_abbrev(type, "char"))
      tmp = 1;
    if (is_abbrev(type, "obj"))
      tmp = 2;
    if (is_abbrev(type, "zone"))
      tmp = 3;
    switch(tmp)
      {
      case 1:  /* load mob */
     arg1 = strtok(NULL, " ");
     if(arg1)
       number = atoi(arg1);
     if ((r_num = real_mobile(number)) < 0) {
       send_to_char("There is no monster with that number.\n\r", ch);
       return;
     }
     zonenum=inzone(number);
     if(zone_locked(ch,zonenum)) return;
     mob = read_mobile(r_num, REAL);
     char_to_room(mob, CHAR_REAL_ROOM(ch));

     act("$n makes a quaint, magical gesture with one hand.", TRUE, ch,
         0, 0, TO_ROOM);
     act("$n has created $N!", FALSE, ch, 0, mob, TO_ROOM);
     act("You have created $N!", FALSE, ch, 0, mob, TO_CHAR); /* Ranger - April 96 */
     sprintf(buf, "WIZINFO: (%s) load char (#%d) %d (%s)",
          GET_NAME(ch), world[CHAR_REAL_ROOM(ch)].number, number,
          GET_SHORT(mob));
     log_s(buf);
     wizlog(buf,GET_LEVEL(ch)+1,5);
     break;
      case 2:  /* object */
     arg1 = strtok(NULL, " ");
     if(arg1)
       number = atoi(arg1);
     if ((r_num = real_object(number)) < 0) {
       send_to_char("There is no object with that number.\n\r", ch);
       return;
     }
     zonenum=inzone(number);
     if(zone_locked(ch,zonenum)) return;

     obj = read_object(r_num, REAL);
  if(IS_SET(obj->obj_flags.extra_flags,ITEM_LIMITED)) {
#ifdef TEST_SITE
       if (GET_LEVEL(ch) < LEVEL_DEI) {
         send_to_char("That item is limited.\n\r",ch);
         extract_obj(obj);
         return;
    }
#else
    if (GET_LEVEL(ch) < LEVEL_WIZ) {
      send_to_char("That item is limited.\n\r",ch);
      extract_obj(obj);
      return;
    }

    if (GET_LEVEL(ch)==LEVEL_WIZ && !IS_SET(ch->new.imm_flags, WIZ_CREATE)) {
          send_to_char("That item is limited.\n\r", ch);
          extract_obj(obj);
          return;
    }
#endif
  }
     if(obj->obj_flags.type_flag == ITEM_SC_TOKEN) {
       if(GET_LEVEL(ch)<LEVEL_ETE || !IS_SET(ch->new.imm_flags, WIZ_ACTIVE)) {
         send_to_char("You are unable to load subclass tokens.\n\r",ch);
         extract_obj(obj);
         return;
       }
     }

     if (obj->obj_flags.type_flag == ITEM_TICKET) {
       if(!adjust_ticket_strings(obj)) return;
     }
     obj_to_char(obj, ch);
     act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
     act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
     act("You have created $p!", FALSE, ch, obj, 0, TO_CHAR);

     sprintf(buf, "WIZINFO: (%s) load obj (#%d) %d (%s)",
          GET_NAME(ch), world[CHAR_REAL_ROOM(ch)].number, number,
          OBJ_SHORT(obj));
     log_s(buf);
     wizlog(buf,GET_LEVEL(ch)+1,5);
      case 3:  /* load a zone */
       arg1 = strtok(NULL, "\0");
       if(arg1) {
#ifdef TEST_SITE
         if(GET_LEVEL(ch) > LEVEL_IMM) {
#else
         if(GET_LEVEL(ch) > LEVEL_ETE) {
#endif
           if((zonenum=boot_area(arg1))>-1) {
             for( i = 0; i <= top_of_zone_table; i++) {

               if(zonenum==zone_table[i].virtual) {
                 sprintf(buf, "[%d] %s\n\r", zonenum, zone_table[i].name);
                 send_to_char(buf,ch);

                 sprintf(buf, "WIZINFO: (%s) load zone [#%d] %s", GET_NAME(ch), zonenum, zone_table[i].name); /* Linerfix 10/27/03 */
                 log_s(buf);
                 wizlog(buf,GET_LEVEL(ch)+1,5);

                 break;
               }
             }
             send_to_char("Zone loaded.\r\n",ch);
           }
           else send_to_char("Zone files do not exist.\r\n",ch);
           /* Ranger - July 96 */
         }
         else
           send_to_char("You cannot load that zone (no priviledges).\r\n", ch);
       }
     break;
      default:
     send_to_char("That'll have to be either 'char' or 'obj'.\n\r", ch);
      }
  }
}
extern int read_zone(FILE *fl);
extern void read_objs(FILE *fl);
extern void read_mobs(FILE *fl);
extern void read_rooms(FILE *fl);
extern void read_shop(FILE *fl);

void initial_boot_area(char *name)
{
  FILE *zf=NULL, *mf=NULL, *of=NULL, *rf=NULL, *sf=NULL;
  char buf[100];
  char filename[35],path[15], *nameptr;
  int i;
  nameptr = filename;

  for(i=0;i<MIN(32, strlen(name));i++)
    {
      if(isalnum(name[i]))
     *nameptr++ = name[i];
    }
  *nameptr = 0;

  sprintf(path,"world/");

  /*first off, boot the zone */
  if(filename[0] != '\0')
    {
    sprintf(buf, "Booting zone:  %s", filename);
    log_s(buf);
      sprintf(buf, "%s%s.zon", path,filename);
      if((zf = fopen(buf, "r")))
     {
       sprintf(buf, "%s%s.wld", path,filename);
       if((rf = fopen(buf, "r")))
         {
           sprintf(buf, "%s%s.mob", path,filename);
           if((mf = fopen(buf, "r")))
          {
            sprintf(buf, "%s%s.obj", path,filename);
            if((of = fopen(buf, "r")))
              {
                read_zone(zf);
                read_rooms(rf);
                read_mobs(mf);
                read_objs(of);
                fclose(of);
                sprintf(buf, "%s%s.shp", path,filename);
                if((sf = fopen(buf, "r"))) {
                  read_shop(sf);
                  fclose(sf);
                }
              }
            else
              log_s("obj fopen");
            fclose(mf);
          }
           else
          log_s("mob fopen");
           fclose(rf);
         }
       else
         log_s("world fopen");
       fclose(zf);
     }
      else
     log_f("zone fopen: %s", buf);

    }
}

int boot_area(char *name)
{
  FILE *zf=NULL, *mf=NULL, *of=NULL, *rf=NULL, *sf=NULL;
  char buf[100];
  char filename[35], *nameptr;
  int i,vnum=-1; /* Exist replaced with vnum 28-Jan-03 for zone num display - Ranger*/

  nameptr = filename;

  for(i=0;i<MIN(32, strlen(name));i++)
    {
      if(isalnum(name[i]))
     *nameptr++ = name[i];
    }
  *nameptr = 0;

  /*first off, boot the zone */
  if(filename[0] != '\0')
    {
      sprintf(buf, "world/%s.zon", filename);
      if((zf = fopen(buf, "r")))
     {
       sprintf(buf, "world/%s.wld", filename);
       if((rf = fopen(buf, "r")))
         {
           sprintf(buf, "world/%s.mob", filename);
           if((mf = fopen(buf, "r")))
          {
            sprintf(buf, "world/%s.obj", filename);
            if((of = fopen(buf, "r")))
              {
                vnum=read_zone(zf);
                read_rooms(rf);
                read_mobs(mf);
                read_objs(of);
                renum_world();
                fclose(of);
                sprintf(buf, "world/%s.shp", filename);
                if((sf = fopen(buf, "r"))) {
                  read_shop(sf);
                  fclose(sf);
                }
                /* Resetting specials - Ranger July 96 */
                if (!no_specials) {
                   log_s("Reassigning specials");
                  assign_mobiles(); /* includes shopkeeper assign */
                  assign_objects();
                  assign_rooms();
                }
              }
            fclose(mf);
          }
           fclose(rf);
         }
       fclose(zf);
     }
    }
  return vnum;
}

/* clean a room of all mobiles and objects */
void do_purge(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict, *next_v;
  struct obj_data *obj, *next_o;

  char name[100];

  if(!check_god_access(ch,FALSE)) return;

  sprintf (name, "WIZINFO: %s purges %s in #%d",
        GET_NAME(ch), argument, CHAR_VIRTUAL_ROOM(ch));
  wizlog(name, GET_LEVEL(ch)+1, 5);
  log_s(name);

  one_argument(argument, name);

  if (*name)  /* argument supplied. destroy single object or char */
    {
      if ((vict = get_char_room_vis(ch, name))) {
     if (!IS_NPC(vict) && (GET_LEVEL(ch)<LEVEL_SUP)) {
       send_to_char("Fuuuuuuuuu!\n\r", ch);
       return;
     }
     act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);

     if (IS_NPC(vict)) {
       extract_char(vict);
     } else {
       if (vict->desc) {
         close_socket(vict->desc);
         vict->desc = 0;
         extract_char(vict);
         save_char(vict,NOWHERE);
       }
       else {
         extract_char(vict);
         save_char(vict,NOWHERE);
       }
     }
      }
      else if ((obj = get_obj_in_list_vis(ch, name,
                           world[CHAR_REAL_ROOM(ch)].contents))) {
     act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
     extract_obj(obj);
      }
      else {
     send_to_char("I don't know anyone or anything by that name.\n\r", ch);
     return;
      }
      send_to_char("Ok.\n\r", ch);
    }
  else   /* no argument. clean out the room */
    {
      if (IS_NPC(ch)) {
     send_to_char("Don't... You would only kill yourself..\n\r", ch);
     return;
      }

      act("$n gestures... You are surrounded by scorching flames!",
       FALSE, ch, 0, 0, TO_ROOM);
      send_to_room("The world seems a little cleaner.\n\r", CHAR_REAL_ROOM(ch));

      for (vict = world[CHAR_REAL_ROOM(ch)].people; vict; vict = next_v) {
     next_v = vict->next_in_room;
     if (IS_NPC(vict))
       extract_char(vict);
      }

      for (obj = world[CHAR_REAL_ROOM(ch)].contents; obj; obj = next_o) {
     next_o = obj->next_content;
     extract_obj(obj);
      }
    }
}



/* Give pointers to the five abilities */
void roll_abilities(struct char_data *ch)
{
  int i, j, k, temp;
  ubyte table[5];
  ubyte rools[4];

  for (i=0; i<5; table[i++]=0);
  for (i=0; i<5; i++) {
    for (j=0; j<4; j++)
      rools[j] = number(1,6);
    temp = rools[0]+rools[1]+rools[2]+rools[3] -
           MIN(rools[0], MIN(rools[1], MIN(rools[2],rools[3])));
    for (k=0; k<5; k++)
      if (table[k] < temp) SWITCH(temp, table[k]);
  }

  ch->abilities.str_add = 0;

  switch (GET_CLASS(ch)) {
  case CLASS_MAGIC_USER:
    ch->abilities.intel = table[0];
    ch->abilities.wis = table[1];
    ch->abilities.dex = table[2];
    ch->abilities.con = table[3];
    ch->abilities.str = table[4];
    break;
  case CLASS_CLERIC:
    ch->abilities.wis = table[0];
    ch->abilities.intel = table[1];
    ch->abilities.str = table[2];
    ch->abilities.dex = table[3];
    ch->abilities.con = table[4];
    break;
  case CLASS_THIEF:
    ch->abilities.dex = table[0];
    ch->abilities.str = table[1];
    ch->abilities.con = table[2];
    ch->abilities.intel = table[3];
    ch->abilities.wis = table[4];
    break;
  case CLASS_WARRIOR:
    ch->abilities.str = table[0];
    ch->abilities.con = table[1];
    ch->abilities.dex = table[2];
    ch->abilities.wis = table[3];
    ch->abilities.intel = table[4];
    if (ch->abilities.str == 18)
      ch->abilities.str_add = number(0,100);
    break;
   case CLASS_NOMAD:
    ch->abilities.con = table[0];
    ch->abilities.dex = table[1];
    ch->abilities.str = table[2];
    ch->abilities.wis = table[3];
    ch->abilities.intel = table[4];
    break;
  case CLASS_NINJA:
    ch->abilities.dex = table[0];
    ch->abilities.con = table[1];
    ch->abilities.str = table[2];
    ch->abilities.wis = table[3];
    ch->abilities.intel = table[4];
    break;
  case CLASS_PALADIN:
    ch->abilities.wis = table[0];
    ch->abilities.str = table[1];
    ch->abilities.intel = table[2];
    ch->abilities.con = table[3];
    ch->abilities.dex = table[4];
    break;
  case CLASS_ANTI_PALADIN:
    ch->abilities.dex = table[0];
    ch->abilities.wis = table[1];
    ch->abilities.str = table[2];
    ch->abilities.intel = table[3];
    ch->abilities.con = table[4];
    break;
  case CLASS_BARD:
    ch->abilities.dex = table[0];
    ch->abilities.intel = table[1];
    ch->abilities.wis = table[2];
    ch->abilities.str = table[3];
    ch->abilities.con = table[4];
    break;
  case CLASS_COMMANDO:
    ch->abilities.str = table[0];
    ch->abilities.con = table[1];
    ch->abilities.intel = table[2];
    ch->abilities.dex = table[3];
    ch->abilities.wis = table[4];
    break;
  case CLASS_AVATAR:
    ch->abilities.con = 18;
    ch->abilities.str = 18;
    ch->abilities.dex = 18;
    ch->abilities.wis = 18;
    ch->abilities.intel = 18;
    break;
  }
  ch->tmpabilities = ch->abilities;
}


void do_start(struct char_data *ch)
{
  struct affected_type_5 af;
  OBJ *obj;
  void advance_level(struct char_data *ch);

  assert(ch->skills);

  send_to_char("Welcome. This is now your character in Ronin Diku,\n\rYou can now earn XP, and lots more...\n\r\n\r", ch);

  GET_LEVEL(ch) = 1;
  GET_EXP(ch) = 1;

  set_title(ch,NULL);
  while(GET_STR(ch)+GET_DEX(ch)+GET_INT(ch)+GET_WIS(ch)+GET_CON(ch)<55) {
  roll_abilities(ch);
  }
  ch->points.max_hit  = 10;  /* These are BASE numbers   */

  send_to_char("Auto-equipping...\n\r",ch);
  switch (GET_CLASS(ch)) {

  case CLASS_MAGIC_USER : {
    obj=read_object(6604,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6608,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6612,VIRTUAL);if(obj) obj_to_char(obj,ch);
  } break;

  case CLASS_CLERIC : {
    obj=read_object(6601,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6608,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6612,VIRTUAL);if(obj) obj_to_char(obj,ch);
  } break;

  case CLASS_THIEF : {
    ch->skills[SKILL_SNEAK].learned = 10;
    ch->skills[SKILL_HIDE].learned =  5;
    ch->skills[SKILL_STEAL].learned = 15;
    ch->skills[SKILL_BACKSTAB].learned = 10;
    ch->skills[SKILL_PICK_LOCK].learned = 10;
    obj=read_object(6603,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6610,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6614,VIRTUAL);if(obj) obj_to_char(obj,ch);
  } break;

  case CLASS_WARRIOR: {
    obj=read_object(6620,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6606,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6611,VIRTUAL);if(obj) obj_to_char(obj,ch);
  } break;

  case CLASS_NINJA: {
    obj=read_object(6602,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6609,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6613,VIRTUAL);if(obj) obj_to_char(obj,ch);
  } break;

  case CLASS_NOMAD: {
    obj=read_object(6605,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6606,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6611,VIRTUAL);if(obj) obj_to_char(obj,ch);
  } break;

  case CLASS_PALADIN: {
    GET_ALIGNMENT (ch) = 1000;

    af.type      = SPELL_BLESS;
    af.duration  = -1;
    af.modifier  = 1;
    af.location  = APPLY_HITROLL;
    af.bitvector = 0;
    af.bitvector2 = 0;
    affect_to_char(ch, &af);

    af.location = APPLY_SAVING_SPELL;
    af.modifier = -1;                 /* Make better */
    affect_to_char(ch, &af);
    /*
       af.type      = SPELL_PROTECT_FROM_EVIL;
       af.duration  = -1;
       af.modifier  = 0;
       af.location  = APPLY_NONE;
       af.bitvector = AFF_PROTECT_EVIL;
       affect_to_char(ch, &af);
       */
    obj=read_object(6619,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6606,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6611,VIRTUAL);if(obj) obj_to_char(obj,ch);
  } break;

  case CLASS_ANTI_PALADIN: {
    GET_ALIGNMENT(ch) = -1000;
    obj=read_object(6603,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6610,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6614,VIRTUAL);if(obj) obj_to_char(obj,ch);
  } break;

  case CLASS_AVATAR: {
    GET_COND(ch,THIRST) = -1;
    GET_COND(ch,FULL) = -1;
    GET_COND(ch,DRUNK) = -1;
  } break;

  case CLASS_BARD: {
    obj=read_object(6605,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6607,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6614,VIRTUAL);if(obj) obj_to_char(obj,ch);
  } break;

  case CLASS_COMMANDO: {
    obj=read_object(6625,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6610,VIRTUAL);if(obj) obj_to_char(obj,ch);
    obj=read_object(6614,VIRTUAL);if(obj) obj_to_char(obj,ch);
  } break;

  }

  advance_level(ch);

  GET_HIT(ch) = hit_limit(ch);
  GET_MANA(ch) = mana_limit(ch);
  GET_MOVE(ch) = move_limit(ch);

  if (GET_CLASS(ch) != CLASS_AVATAR) {
    GET_COND(ch,THIRST) = 24;
    GET_COND(ch,FULL) = 24;
    GET_COND(ch,DRUNK) = 0;
  }

  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

  send_to_char("Auto-equipping complete.\n\r",ch);
  obj=read_object(3012,VIRTUAL);
  if(obj) obj_to_char(obj,ch);
  obj=read_object(3102,VIRTUAL);
  if(obj) obj_to_char(obj,ch);
  GET_GOLD(ch)=20000;

}

char *class_name[] = {
  "--",
  "Magic User",
  "Cleric",
  "Thief",
  "Warrior",
  "Ninja",
  "Nomad",
  "Paladin",
  "Anti-Paladin",
  "Avatar",
  "Bard",
  "Commando"
};

extern void advance_level(CHAR *ch);
void do_class(CHAR *ch, char *arg, int cmd)
{
  char buf[MSL];
  char name[MIL];
  char num[MIL];
  CHAR *victim;
  int class_num;
  int i = 0;

  if (!check_god_access(ch, TRUE)) return;

  argument_interpreter(arg, name, num);

  if (!*name)
  {
    sprintf(buf, "\
Change the class of the victim. Note: The level of the victim will be set to 1.\n\r\
Usage: class <victim> <num>\n\r\
%2d = Magic-User\n\r\
%2d = Cleric\n\r\
%2d = Thief\n\r\
%2d = Warrior\n\r\
%2d = Ninja\n\r\
%2d = Nomad\n\r\
%2d = Paladin\n\r\
%2d = Anti-Paladin\n\r\
%2d = Avatar\n\r\
%2d = Bard\n\r\
%2d = Commando\n\r",
      CLASS_MAGIC_USER,
      CLASS_CLERIC,
      CLASS_THIEF,
      CLASS_WARRIOR,
      CLASS_NINJA,
      CLASS_NOMAD,
      CLASS_PALADIN,
      CLASS_ANTI_PALADIN,
      CLASS_AVATAR,
      CLASS_BARD,
      CLASS_COMMANDO);
    send_to_char(buf, ch);

    return;
  }

  if (!(victim = get_char_room_vis(ch, name)))
  {
    send_to_char("That player is not here.\n\r", ch);
    return;
  }

  if (IS_NPC(victim) ||
    !ch->skills)
  {
    send_to_char("NO! Not on NPC's.\n\r", ch);
    return;
  }

  if (!*num)
  {
    send_to_char("You must supply a class number.\n\r", ch);
    return;
  }

  if (!isdigit(*num))
  {
    send_to_char("Second argument must be a positive integer.\n\r", ch);

    return;
  }

  class_num = atoi(num);

  if ((class_num < CLASS_MAGIC_USER) ||
      (class_num > CLASS_COMMANDO))
  {
    send_to_char("Invalid class number.\r\n", ch);
    return;
  }

  GET_CLASS(victim) = class_num;

  if (IS_MORTAL(victim))
  {
    GET_LEVEL(victim) = 1;
    GET_EXP(victim) = 1;

    set_title(victim, NULL);

    roll_abilities(victim);

    victim->points.max_hit = 10;
    victim->points.max_mana = 0;
    victim->points.max_move = 0;

    victim->specials.spells_to_learn = 0;

    advance_level(victim);

    victim->specials.affected_by = 0;
    victim->specials.affected_by2 = 0;

    for (i = 0; i <= (MAX_SKILLS5 - 1); i++)
    {
      victim->skills[i].learned = 0;
    }

    for (i = 0; i < 5; i++)
    {
      victim->specials.apply_saving_throw[i] = 0;
    }

    GET_HIT(victim) = hit_limit(victim);
    GET_MANA(victim) = mana_limit(victim);
    GET_MOVE(victim) = move_limit(victim);

    switch (GET_CLASS(victim))
    {
      case CLASS_MAGIC_USER:
        break;

      case CLASS_CLERIC:
        break;

      case CLASS_THIEF:
        victim->skills[SKILL_SNEAK].learned = 10;
        victim->skills[SKILL_HIDE].learned =  5;
        victim->skills[SKILL_STEAL].learned = 15;
        victim->skills[SKILL_BACKSTAB].learned = 10;
        victim->skills[SKILL_PICK_LOCK].learned = 10;
        break;

      case CLASS_WARRIOR:
        break;

      case CLASS_NINJA:
        break;

      case CLASS_NOMAD:
        break;

      case CLASS_PALADIN:
        break;

      case CLASS_ANTI_PALADIN:
        break;

      case CLASS_AVATAR:
        break;

      case CLASS_BARD:
        break;

      case CLASS_COMMANDO:
        break;
    }

    if (GET_CLASS(victim) == CLASS_AVATAR)
    {
      GET_COND(victim, THIRST) = -1;
      GET_COND(victim, FULL) = -1;
      GET_COND(victim, DRUNK) = -1;
    }
    else
    {
      GET_COND(victim, THIRST) = 24;
      GET_COND(victim, FULL) = 24;
      GET_COND(victim, DRUNK) = 0;
    }
  }

  save_char(victim, NOWHERE);

  sprintf(buf, "You changed %s to class %2s.\n\r",
          GET_NAME(victim), class_name[GET_CLASS(victim)]);
  send_to_char(buf, ch);

  sprintf (buf, "WIZINFO: %s changed %s to class %2s",
           GET_NAME(ch), GET_NAME(victim), class_name[GET_CLASS(victim)]);
  wizlog(buf, GET_LEVEL(ch) + 1, 5);
  log_s(buf);
}

void do_club(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[100], club[100];
  int newclub;

  if(!check_god_access(ch,TRUE)) return;

  argument_interpreter(argument, name, club);

  if (*name) {
    if (!(victim = get_char_room_vis(ch, name))) {
      send_to_char("That player is not here.\n\r", ch);
      return;
    }

  } else {
    send_to_char("Change the club membership of the victim.\n\rUsage club <victim> <num>\n\r0 = No club\n\r1 = Sane's Club\n\r2 = Liner's Club\n\r3 = Ranger's Club\n\r4 = Lem's Club\n\r", ch);
    return;
  }

  if (IS_NPC(victim)) {
    send_to_char("NO! Not on NPC's.\n\r", ch);
    return;
  }
  if (!*club) {
    send_to_char("You must supply a club number.\n\r", ch);
    return;
  } else {
    if (!isdigit(*club)) {
      send_to_char("Second argument must be a positive integer.\n\r",ch);
      return;
    }
  }

  newclub = atoi(club);

  if (IS_SET(victim->specials.pflag, PLR_LINERS_LOUNGE))
    REMOVE_BIT(victim->specials.pflag, PLR_LINERS_LOUNGE);
  else if (IS_SET(victim->specials.pflag, PLR_SANES_VOCAL_CLUB))
    REMOVE_BIT(victim->specials.pflag, PLR_SANES_VOCAL_CLUB);
  else if (IS_SET(victim->specials.pflag, PLR_RANGERS_RELIQUARY))
    REMOVE_BIT(victim->specials.pflag, PLR_RANGERS_RELIQUARY);
  else if (IS_SET(victim->specials.pflag, PLR_LEMS_LIQOUR_ROOM))
    REMOVE_BIT(victim->specials.pflag, PLR_LEMS_LIQOUR_ROOM);

  if (newclub == 1)
    SET_BIT(victim->specials.pflag, PLR_SANES_VOCAL_CLUB);
  else if (newclub == 2)
    SET_BIT(victim->specials.pflag, PLR_LINERS_LOUNGE);
  else if (newclub == 3)
    SET_BIT(victim->specials.pflag, PLR_RANGERS_RELIQUARY);
  else if (newclub == 4)
    SET_BIT(victim->specials.pflag, PLR_LEMS_LIQOUR_ROOM);

  sprintf (club, "WIZINFO: %s set %s's club to %d",
        GET_NAME(ch), GET_NAME(victim), newclub);
  wizlog(club, GET_LEVEL(ch)+1, 5);
  log_s(club);

  send_to_char("Done.\n\r", ch);

}

void advance_level(struct char_data *ch);
extern void gain_exp_regardless(struct char_data *ch, int gain);
void death_list(CHAR *ch);
void do_advance(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[100], level[100], buf2[MAX_STRING_LENGTH];
  int adv, newlevel = 0;

  if(!check_god_access(ch,TRUE)) return;

  argument_interpreter(argument, name, level);

  if (*name) {
    if (!(victim = get_char_room_vis(ch, name))) {
      send_to_char("That player is not here.\n\r", ch);
      return;
    }
  } else {
    send_to_char("Advance who?\n\r", ch);
    return;
  }

  if (IS_NPC(victim)) {
    send_to_char("NO! Not on NPC's.\n\r", ch);
    return;
  }

  if (GET_LEVEL(victim) == 0)
    adv = 1;
  else if (!*level) {
    send_to_char("You must supply a level number.\n\r", ch);
    return;
  } else {
    if (!isdigit(*level)) {
      send_to_char("Second argument must be a positive integer.\n\r",ch);
      return;
    }
    if ((newlevel = atoi(level)) <= GET_LEVEL(victim)) {
      send_to_char("Can't dimish a players status with this command.\n\r", ch);
      return;
    }
    adv = newlevel - GET_LEVEL(victim);
  }

  if (((adv + GET_LEVEL(victim)) > 1) && (GET_LEVEL(ch) < LEVEL_SUP)) {
    send_to_char("Thou art not godly enough.\n\r", ch);
    return;
  }

  if(newlevel>LEVEL_IMP) {
    send_to_char("Greater than IMP level?\n\r",ch);
    return;
  }

  sprintf (buf2, "WIZINFO: %s advanced %s from %d to %d",
        GET_NAME(ch), GET_NAME(victim), GET_LEVEL(victim), newlevel);
  wizlog(buf2, GET_LEVEL(ch)+1, 5);
  log_s(buf2);

  if (((adv + GET_LEVEL(victim)) > LEVEL_ETE)
      && (GET_LEVEL(ch) < LEVEL_IMP)) {
    send_to_char("Wizard is the highest possible level.\n\r", ch);
    return;
  }

  if(GET_LEVEL(victim)<LEVEL_IMM && newlevel>=LEVEL_IMM) { /* burn any gear and log it */
    victim->new.been_killed += 1;
    death_list(victim);
    send_to_char("Check with this new god about him/her having access to a vault, and get it removed.\n\r",ch);
  }

  if ((adv + GET_LEVEL(victim)) <= LEVEL_IMP) {
    send_to_char("You feel generous.\n\r", ch);
    act("$n makes some strange gestures.\n\rA strange feeling comes uppon you,"
     "\n\rLike a giant hand, light comes down from\n\rabove, grabbing your "
     "body, that begins\n\rto pulse with coloured lights from inside.\n\rYo"
     "ur head seems to be filled with deamons\n\rfrom another plane as your"
     " body dissolves\n\rto the elements of time and space itself.\n\rSudde"
     "nly a silent explosion of light snaps\n\ryou back to reality. You fee"
     "l slightly\n\rdifferent.",FALSE,ch,0,victim,TO_VICT);
  }

  if (GET_LEVEL(victim) == 0) {
    do_start(victim);
  } else {
    if (GET_LEVEL(victim) < LEVEL_IMP+1) {
      GET_LEVEL(victim)+=adv;
      GET_EXP(victim)=0;
      while(adv>0) {
        send_to_char("You raise a level!!\n\r", victim);
        advance_level(victim);
        adv--;
      }
    } else {
      send_to_char("Some idiot just tried to advance your level.\n\r", victim);
      send_to_char("IMPOSSIBLE! IDIOTIC!\n\r", ch);
    }
  }

  if (GET_LEVEL(victim) >= LEVEL_IMM) {
    victim->new.killed = 0;
    victim->new.been_killed = 0;
    set_title(victim,NULL);
    insert_char_wizlist(victim);
  }

  save_char(victim, NOWHERE);
}

extern void remove_wizlist_name (char *name);
void do_demote(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[100], level[100];
  int newlevel=0, oldlevel,i;
  char buf[250];

  void gain_exp(struct char_data *ch, int gain);

  if(!check_god_access(ch,TRUE)) return;

  argument_interpreter(argument, name, level);

  if (*name) {
    if (!(victim = get_char_room_vis(ch, name))) {
      send_to_char("That player is not here.\n\r", ch);
      return;
    }
  } else {
    send_to_char("WARNING!! Work better for demote a few level and equipping nothing!\n\r", ch);
    return;
  }

  if (IS_NPC(victim)) {
    send_to_char("NO! Not on NPC's.\n\r", ch);
    return;
  }

  if (!strcmp(GET_NAME(victim),"Lem") ||
      !strcmp(GET_NAME(victim),"Sumo") ||
      !strcmp(GET_NAME(victim),"Ranger") ||
      !strcmp(GET_NAME(victim),"Liner") ||
      !strcmp(GET_NAME(victim),"Sane") ||
      !strcmp(GET_NAME(victim),"Shun") ||
      !strcmp(GET_NAME(victim),"Night"))
  {
    send_to_char("You cannot demote the active IMPs.\n\r",ch);
    return;
  }

  if(GET_LEVEL(victim) <= 1) {
    send_to_char("You cannot demote him/her more...\n\r", ch);
    return;
  }

  if (!*level) {
    send_to_char("You must supply a level number.\n\r", ch);
    return;
  } else {
    if (!isdigit(*level)) {
      send_to_char("Second argument must be a positive integer.\n\r",ch);
      return;
    }
    if ((newlevel = atoi(level)) <= 0 ) {
      send_to_char("Hey lets just demote level 1... not negative..\n\r", ch);
      return;
    }
    if (newlevel >= GET_LEVEL(victim)) {
      send_to_char("This is a command to DEMOTE a player.\n\r", ch);
      return;
    }
  }

  GET_EXP(victim) = 0;
  oldlevel = GET_LEVEL(victim);
  GET_LEVEL(victim) = newlevel;
  victim->points.max_hit -= ((oldlevel - newlevel) *
                    (((victim->points.max_hit)-10)/oldlevel));
  victim->points.max_mana -= ((oldlevel - newlevel) *
                     ((victim->points.max_mana)/(oldlevel-1)));
  set_title(victim,NULL);

  if(newlevel==1){
    for(i=0;i<MAX_WEAR;i++) {
      if(victim->equipment[i])
        obj_to_char(unequip_char(victim,i),victim);
    }
    GET_EXP(victim) = 1;
    victim->points.max_hit  = 10;
    victim->points.max_mana =0;
    victim->points.max_move=0;
    advance_level(victim);
    for (i = 0; i <= MAX_SKILLS5 - 1; i++)
      victim->skills[i].learned = 0;
  }

  if (newlevel >= LEVEL_IMM) {
    insert_char_wizlist(victim);
  } else if (oldlevel >= LEVEL_IMM) {
    remove_wizlist_name(GET_NAME(victim));
  }

  sprintf (buf, "WIZINFO: %s demoted %s from %d to %d",
        GET_NAME(ch), GET_NAME(victim), oldlevel, newlevel);
  wizlog(buf, GET_LEVEL(ch)+1, 5);
  log_s(buf);
  send_to_char("Done.\n\r", ch);
  send_to_char("You have been demoted.\n\r",victim);

  save_char(victim, NOWHERE);
}


void do_reroll(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char buf[100];

  if(!check_god_access(ch,TRUE)) return;

  one_argument(argument,buf);
  if (!*buf)
    send_to_char("Who do you wich to reroll?\n\r",ch);
  else
    if (!(victim = get_char(buf)))
      send_to_char("No-one by that name in the world.\n\r",ch);
    else {
      sprintf (buf, "WIZINFO: %s rerolled %s", GET_NAME(ch), GET_NAME(victim));
      wizlog(buf, GET_LEVEL(ch)+1, 5);
      log_s(buf);

      send_to_char("Rerolled...\n\r", ch);
      roll_abilities(victim);
    }
}


void do_restore(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char buf[100];
  int i;

  void update_pos( struct char_data *victim );

  if(!check_god_access(ch,TRUE)) return;

  one_argument(argument,buf);
  if (!*buf)
    send_to_char("Who do you wich to restore?\n\r",ch);
  else
    if (!(victim = get_char(buf)))
      send_to_char("No-one by that name in the world.\n\r",ch);
    else {
      GET_MANA(victim) = GET_MAX_MANA(victim);
      GET_HIT(victim) = GET_MAX_HIT(victim);
      GET_MOVE(victim) = GET_MAX_MOVE(victim);

      if (GET_LEVEL(victim) >= LEVEL_IMM && victim->skills) {
     for (i = 0; i < MAX_SKILLS5; i++) {
       victim->skills[i].learned = 100;
     }

     if (GET_LEVEL(victim) >= LEVEL_IMM ||
         (GET_CLASS(victim) == CLASS_AVATAR)) {
       GET_COND(ch,THIRST) = -1;
       GET_COND(ch,FULL) = -1;
       GET_COND(ch,DRUNK) = -1;
     }

     if (GET_LEVEL(victim) >= LEVEL_IMM) {
       i=18;
       if(GET_LEVEL(victim)==LEVEL_TEM) i=20;
       if(GET_LEVEL(victim)==LEVEL_WIZ) i=22;
       if(GET_LEVEL(victim)==LEVEL_ETE) i=23;
       if(GET_LEVEL(victim)==LEVEL_SUP) i=24;
       if(GET_LEVEL(victim)==LEVEL_IMP) i=25;
       victim->abilities.str_add = 0;
       victim->abilities.intel = i;
       victim->abilities.wis = i;
       victim->abilities.dex = i;
       victim->abilities.str = i;
       victim->abilities.con = i;
     }
     victim->tmpabilities = victim->abilities;

      }
      update_pos( victim );
      sprintf (buf, "WIZINFO: %s restored %s in #%d",
            GET_NAME(ch), GET_NAME(victim),
            CHAR_VIRTUAL_ROOM(victim));
      wizlog(buf, GET_LEVEL(ch)+1, 5);
      log_s(buf);
      send_to_char("Done.\n\r", ch);
      act("You have been fully healed by $N!", FALSE, victim, 0, ch, TO_CHAR);
    }
}

void do_punish(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char buf[100];

  if(!check_god_access(ch,TRUE)) return;

  if(GET_LEVEL(ch)<LEVEL_SUP && !IS_SET(ch->new.imm_flags, WIZ_JUDGE)) {
    send_to_char("You need a Judge flag to do this.\n\r",ch);
    return;
  } /* Judge Flag check added by Ranger - May 96 */

  one_argument(argument,buf);
  if (!*buf) {
    send_to_char("It will punish the victim's hps, mana and move to 1.\n\r",ch);
    return;
  }
  else {
    if (!(victim = get_char(buf))) {
      send_to_char("No-one by that name in the world.\n\r",ch);
      return;
    }
    else {
      GET_HIT(victim) = 1;
      GET_MANA(victim) = 1;
      GET_MOVE(victim) = 1;

      sprintf (buf, "WIZINFO: %s punished %s",
            GET_NAME(ch), GET_NAME(victim));
      wizlog(buf, GET_LEVEL(ch)+1, 5);
      log_s(buf);
      update_pos( victim );
      send_to_char("Done.\n\r", ch);
    }
  }
}

void do_nogold(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char buf[100];

  if(!check_god_access(ch,TRUE)) return;

  if(GET_LEVEL(ch)<LEVEL_SUP && !IS_SET(ch->new.imm_flags, WIZ_JUDGE)) {
    send_to_char("You need a Judge flag to do this.\n\r",ch);
    return;
  } /* Judge Flag check added by Ranger - May 96 */

  one_argument(argument,buf);
  if (!*buf)
    send_to_char("It will set the victim's coins to 1000 and bank to 0\n\r",ch);
  else {
    if (!(victim = get_char(buf)))
      send_to_char("No-one by that name in the world.\n\r",ch);
    else {
      GET_GOLD(victim) = 1000;
      GET_BANK(victim) = 0;

      sprintf (buf, "WIZINFO: %s nogold'ed %s",
          GET_NAME(ch), GET_NAME(victim));
      wizlog(buf, GET_LEVEL(ch)+1, 5);
      log_s(buf);
    }
    send_to_char("Done.\n\r", ch);
  }
}


void do_noshout(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  struct obj_data *dummy;
  char buf[MAX_INPUT_LENGTH],buf2[MAX_INPUT_LENGTH];

  if(!check_god_access(ch,TRUE)) return;

  one_argument(argument, buf);

  if (!*buf)
    if (IS_SET(ch->specials.pflag, PLR_NOSHOUT)) {
      send_to_char("You can now hear shouts again.\n\r", ch);
     REMOVE_BIT(ch->specials.pflag, PLR_NOSHOUT);
    } else {
      send_to_char("From now on, you won't hear shouts.\n\r", ch);
      SET_BIT(ch->specials.pflag, PLR_NOSHOUT);
  } else if (!generic_find(argument,
                  FIND_CHAR_WORLD, ch, &vict, &dummy))
    send_to_char("Couldn't find any such creature.\n\r", ch);
  else if (IS_NPC(vict))
    send_to_char("Can't do that to a beast.\n\r", ch);
  else if (GET_LEVEL(vict) > GET_LEVEL(ch))
    act("$E might object to that.. better not.",
     0, ch, 0, vict, TO_CHAR);
  else if (IS_SET(vict->specials.pflag, PLR_NOSHOUT)) {
    send_to_char("You can shout again.\n\r", vict);
    send_to_char("NOSHOUT removed.\n\r", ch);
    REMOVE_BIT(vict->specials.pflag, PLR_NOSHOUT);
    sprintf(buf2,"WIZINFO: %s removed %s's NOSHOUT",GET_NAME(ch),GET_NAME(vict));
    wizlog(buf2, GET_LEVEL(ch), 5);
    log_s(buf2);
  }
  else {
    send_to_char("The gods take away your ability to shout!\n\r", vict);
    send_to_char("NOSHOUT set.\n\r", ch);
    SET_BIT(vict->specials.pflag, PLR_NOSHOUT);
    sprintf(buf2,"WIZINFO: %s set %s's NOSHOUT",GET_NAME(ch),GET_NAME(vict));
    wizlog(buf2, GET_LEVEL(ch), 5);
    log_s(buf2);
  }
}

void do_wizinv(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  int level;

  if (IS_NPC(ch)) return;

  one_argument(argument, buf);

  if (!*buf) {
    if (ch->new.wizinv > 0) {
      send_to_char("You appear out of thin air.\n\r", ch);
      ch->new.wizinv = 0;
    }
    else if (ch->new.wizinv == 0) {
      send_to_char("You vanish from the mortal world.\n\r", ch);
      ch->new.wizinv = GET_LEVEL(ch);
    }
  } else {
    if (isdigit(*buf))
      level = atoi(buf);
    else
      level = LEVEL_IMM;
    if (level < 0)
      level = LEVEL_IMM;
    if (level > GET_LEVEL(ch))
      level = GET_LEVEL(ch);

    ch->new.wizinv = level;

    sprintf(buf, "Your invisibility level is %d.\n\r", level);
    send_to_char(buf, ch);
  }
}

void do_wizinfo(struct char_data *ch, char *argument, int cmd)
{
  int number;
  char buf[256];

  if(GET_LEVEL(ch) < LEVEL_IMM) return;
  if(!ch->desc) return;

  one_argument(argument, buf);
  number = atoi(buf);

  if (number > GET_LEVEL(ch))
    number = GET_LEVEL(ch);

  if ((ch->desc->wizinfo) && (number < LEVEL_IMM-1)){
    ch->desc->wizinfo = 0;
    send_to_char("Wizinfo mode off.\n\r", ch);
  } else {
    if (number < LEVEL_IMM-1)
      number = GET_LEVEL(ch);
    ch->desc->wizinfo = number;
    sprintf(buf,"Wizinfo mode on. (level %d)\n\r", number);
    send_to_char(buf, ch);
  } /* if */
} /* do_wizinfo */

extern void imm_grace_remove_enchant(CHAR *ch);
extern void imm_grace_add_enchant(CHAR *ch);
void do_setstat(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  char name[100],buf[MAX_INPUT_LENGTH],class[100],num[100];
  char buf2[MAX_INPUT_LENGTH],logit[MAX_STRING_LENGTH],pwd[100];
  char usage[]="\
Syntax : setstat <victim> <field> <value> <passwd>\n\r\
Field can be: alignment, hp, mp, mana, bank, gold, str, add, dex,\n\r\
con, int, wis, age, xp, sex, free_rent(imp), deathlimit,\n\r\
subpts, qpts, prestige, remort_exp, death_exp and questtime\n\r";
  int value = 0;
  unsigned long ulValue = 0UL;
  long long int big_value = 0;

  if(!check_god_access(ch,TRUE)) return;

  half_chop(argument, name,100, buf,MIL);
  half_chop(buf, class,100, buf2,MIL);
  argument_interpreter(buf2, num, pwd);

  sprintf(logit,"WIZINFO: %s setstat %s",GET_NAME(ch),argument);
  wizlog(logit, GET_LEVEL(ch)+1, 5);
  log_s(logit);

  if (!*name || !*buf || !*class || !*num /*|| !isdigit(*num)*/ ) {
    send_to_char(usage,ch);
    return;
  }

  if (!(vict = get_char_vis(ch, name))) {
    send_to_char("No one here by that name...\n\r",ch);
    return;
  }

  if(GET_LEVEL(ch)<GET_LEVEL(vict)){
    send_to_char("Can't do it to a higher level.\n\r",ch);
    return;
  }

  if (!str_cmp(class, "remort_exp"))
  {
    errno = 0;

    big_value = strtoll(num, NULL, 10);

    if ((errno == ERANGE && (big_value == LONG_MAX || big_value == LONG_MIN)) || (errno != 0 && big_value == 0) || big_value < -1)
    {
      send_to_char("\
Usage: -1  : Restore target's Remort v2 enchantment (if it's missing).\n\r\
        0  : Disable target's Remort v2 EXP pool and enchantment.\n\r\
        1+ : Enable target's Remort v2 EXP pool and enchantment.\n\r", ch);

      return;
    }

    if (big_value >= 0)
    {
      sprintf(buf, "Target's Remort v2 EXP pool set to %lld.\n\r", big_value);
      send_to_char(buf, ch);

      GET_REMORT_EXP(vict) = big_value;
    }

    if (big_value != 0 && GET_REMORT_EXP(vict) && !enchanted_by_type(vict, ENCHANT_REMORTV2))
    {
      send_to_char("Target's Remort v2 enchantment turned ON.\n\r", ch);

      rv2_add_enchant(vict);
    }
    else if (big_value == 0)
    {
      send_to_char("Target's Remort v2 enchantment turned OFF.\n\r", ch);

      rv2_remove_enchant(vict);
    }

    return;
  }

  if (!str_cmp(class, "death_exp"))
  {
    if (num[0] == '-')
    {
      if (str_cmp(num, "-1"))
      {
        send_to_char("\
Usage: -1  : Restore target's Death Experience enchantment (if it's missing).\n\r\
        0  : Disable target's Death Experience pool and enchantment.\n\r\
        1+ : Enable target's Death Experience pool and enchantment.\n\r", ch);
      }
      else if (GET_DEATH_EXP(vict) && !enchanted_by_type(vict, ENCHANT_IMM_GRACE))
      {
        send_to_char("Target's Death Experience enchantment turned ON.\n\r", ch);
        imm_grace_add_enchant(vict);
      }
 
      return;
    }

    ulValue = strtoul(num, NULL, 10);

    if (ulValue >= 0)
    {
      sprintf(buf, "Target's Death Experience pool set to %lu.\n\r", ulValue);
      send_to_char(buf, ch);

      GET_DEATH_EXP(vict) = ulValue;
    }

    if (value == 0)
    {
      send_to_char("Target's Death Experience enchantment turned OFF.\n\r", ch);
      imm_grace_remove_enchant(vict);
    }

    return;
  }

  value = atoi(num);

  if (strcmp(class, "hp")  == 0)  {
    vict->points.max_hit  = value; return;
  }
  if (strcmp(class, "mp")  == 0) {
    vict->points.max_move  = value; return;
  }
  if (strcmp(class, "mana") == 0) {
    vict->points.max_mana = value; return;
  }
  if (strcmp(class, "gold") == 0) {
    vict->points.gold = value; return;
  }
  if (strcmp(class, "subpts") == 0) {
    vict->ver3.subclass_points = value; return;
  }
  if (strcmp(class, "qpts") == 0) {
    vict->ver3.quest_points = value; return;
  }
  if (strcmp(class, "prestige") == 0) {
    send_to_char("Warning: You probably want to adjust hit points/mana up or down commensurate to the adjustment you just made.\n\r", ch);
    vict->ver3.prestige = value; return;
  }

  if(strcmp(class,"xp")==0)
    {
      GET_EXP(vict)=value;return;
    }
  if (strcmp(class, "bank") == 0) {
    vict->points.bank = value; return;
  }

  if(strcmp(class, "sex")==0)
    {
      GET_SEX(vict) = value;return;
    }
  if (strcmp(class, "age") == 0) {
    value -= GET_AGE(vict);
    if ((GET_AGE(vict) + value) >= 16)
      { vict->player.time.birth -= ((long)SECS_PER_MUD_YEAR*(long)value);
     return; }
    else {
      send_to_char("Age must be over 16.\n\r", ch);
      return;
    }
  }
  if (strcmp(class, "alignment") == 0) {
    if (value >= -1000 && value <= 1000) {
      vict->specials.alignment = value;
      return;
    }
    else {
      send_to_char("Alignment must be >= -1000 and <= 1000.", ch);
      return;
    }
  }
  if (strcmp(class, "str") == 0)  {
    if (value < 3 || value > MAX_STAT) {
      send_to_char("Strength must be >= 3, <= 25.\n\r", ch);
      return;
    }
    vict->abilities.str = value;
    vict->tmpabilities.str = value;
    return;
  }
  if (strcmp(class, "add")  == 0)  {
    if (value < 0 || value > 100) {
      send_to_char("Strength add must be >= 0, <= 100.\n\r", ch);
      return;
    }
    vict->abilities.str_add = value;
    vict->tmpabilities.str_add = value;
    return;
  }
  if (strcmp(class, "dex")  == 0) {
    if (value < 3 || value > MAX_STAT) {
      send_to_char("Dexterity must be >= 3, <= 25.\n\r", ch);
      return;
    }
    vict->abilities.dex = value;
    vict->tmpabilities.dex = value;
    return;
  }
  if (strcmp(class, "con") == 0) {
    if (value < 3 || value > MAX_STAT) {
      send_to_char("Mana must be >= 3, <= 25.\n\r", ch);
      return;
    }
    vict->abilities.con = value;
    vict->tmpabilities.con = value;
    return;
  }
  if (strcmp(class, "int") == 0) {
    if (value < 3 || value > MAX_STAT) {
      send_to_char("Intelligence must be >= 3, <= 25.\n\r", ch);
      return;
    }
    vict->abilities.intel = value;
    vict->tmpabilities.intel = value;
    return;
  }
   if (strcmp(class, "wis") == 0) {
     if (value < 3 || value > MAX_STAT) {
       send_to_char("Wisdom must be >= 3, <= 25.\n\r", ch);
       return;
     }
     vict->abilities.wis = value;
     vict->tmpabilities.wis = value;
     return;
   }
  if (strcmp(class, "deathlimit") == 0) {
    if (value < 0 || value > 10000)
        send_to_char("You can't set to that number\n\r", ch);
    else
      vict->ver3.death_limit = value;
    return;
  }

  if (strcmp(class, "free_rent") == 0) {
    if(GET_LEVEL(ch) == LEVEL_IMP) {
      if(value != 0 && value != 1){
     send_to_char("value must be 0 or 1 !\n\r", ch);
     return;
      }
      if(vict->desc)
     vict->desc->free_rent = value;
      if(value){
     send_to_char("Done\n\r",ch);
     send_to_char("You have now one free rent...;)\n\r", vict);
      } else {
     send_to_char("You have made sure that he/she doesn't have free rent :(\n\r", ch);
     send_to_char("You surely don't have free rent now :(\n\r", ch);
      }
    } else
      send_to_char("Sorry but you cannot do this! :(\n\r", ch);

    return;
  }

  if (strcmp(class, "questtime") == 0) {
    vict->ver3.time_to_quest = value;
    return;
  }


  send_to_char ("Invalid field specified.\n\r", ch);
  return;

} /* do_setstat */

void do_session(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[100], number[100];
  char buf[256];

  if(!check_god_access(ch,TRUE)) return;

  argument_interpreter(argument, name, number);

  if (*name) {
    if (!(victim = get_char_room_vis(ch, name))) {
      send_to_char("That player is not here.\n\r", ch);
      return;
    }
  } else {
    send_to_char("This command is to add sessions to a particular player!\n\r", ch);
    return;
  }

  sprintf (buf, "WIZINFO: %s added %d sessions to %s",
        GET_NAME(ch), atoi(number), GET_NAME(victim));
  wizlog(buf, GET_LEVEL(ch)+1, 5);
  log_s(buf);

  victim->specials.spells_to_learn += atoi(number);
  send_to_char("Done.\n\r", ch);

}

void do_alert(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char buf[MAX_STRING_LENGTH];
  char alertstring[]={7,7,7,'\n','\r',0},name[MAX_STRING_LENGTH];
  struct descriptor_data *pt;

  if(!check_god_access(ch,FALSE)) return;

  sprintf(buf,"You are alerted by %s.\n\r",GET_NAME(ch));
  one_argument(argument,name);

  if(!strcmp(name,"all")) {
    for (pt = descriptor_list; pt; pt = pt->next)
      if(pt->connected==CON_PLYNG && pt->character != ch)
        if(CAN_SEE(ch, pt->character)) {
          send_to_char(alertstring, pt->character);
          send_to_char(buf,pt->character);
        }
  }
  else {
    if(!(victim = get_char_vis( ch, name))) {
      send_to_char("That person can't be found.\n\r",ch);
      return;
    }
    send_to_char(alertstring, victim);
    send_to_char(buf,victim);
  }
}

/*
   ostat command for Imperial Diku. (c) Ossi Tulonen 1994.
   otulonen@snakemail.hut.fi
   Not for public domain, do not distribute.
   */

void item_type_flag_to_string(struct obj_flag_data *flags, char *str)
{
  long int mindam,maxdam,avgdam;
  char apt[3];
  char buf0[245];
  char buf1[245];
  char buf2[245];
  char buf3[245];

  *buf0 = '\0';
  *buf1 = '\0';
  *buf2 = '\0';
  *buf3 = '\0';

  switch(flags->type_flag) {
  case ITEM_LIGHT :
    sprintf(str, "-Colour : [%d]\n\r-Type : [%d]\n\rHours : [%d]\n\r(not used: [%d])\n\r",
         flags->value[0],
         flags->value[1],
         flags->value[2],
         flags->value[3]);
    break;
  case ITEM_RECIPE :
    sprintf(str, "Creates: [%d]\n\rRequires [%d] [%d] [%d]\n\r",
         flags->value[0],
         flags->value[1],
         flags->value[2],
         flags->value[3]);
    break;
  case ITEM_AQ_ORDER :
    sprintf(str, "Fulfillment Requires Objects: [%d] [%d] [%d] [%d]\n\r",
         flags->value[0],
         flags->value[1],
         flags->value[2],
         flags->value[3]);
    break;    
  case ITEM_SCROLL :
  case ITEM_POTION :
    if(flags->value[1] > 0)
      sprinttype(flags->value[1]-1, spells, buf1);
    if(flags->value[2] > 0)
      sprinttype(flags->value[2]-1, spells, buf2);
    if(flags->value[3] > 0)
      sprinttype(flags->value[3]-1, spells, buf3);
    sprintf(str, "Level: %d\n\rSpells : %d (%s)\n\r         %d (%s)\n\r         %d (%s)\n\r",
         flags->value[0],
         flags->value[1],
         buf1,
         flags->value[2],
         buf2,
         flags->value[3],
         buf3);
    break;
  case ITEM_WAND :
  case ITEM_STAFF :
    if(flags->value[3] > 0)
      sprinttype(flags->value[3]-1, spells, buf3);
    sprintf(str, "Level : %d\n\rMax Charges : %d\n\rCharges Left: %d\n\rSpell: %d (%s)\n\r",
         flags->value[0],
         flags->value[1],
         flags->value[2],
         flags->value[3],
         buf3);
    break;
  case ITEM_WEAPON :
  case ITEM_2H_WEAPON:
    maxdam=flags->value[1]*flags->value[2];
    mindam=flags->value[1];
    avgdam=((flags->value[1]+flags->value[1]
            *flags->value[2])/2);
    sprintf(apt, ((mindam+maxdam)%2 ? ".5" : ""));

    if((flags->value[0]>-1) && (flags->value[0]<42) &&
       (flags->value[3]>-1) && (flags->value[3]<15) )
      sprintf(str, "Extra: %s(%d)\n\rDmg:%dD%d (%ld-%ld, Avg. %ld%s)\n\rType : %s(%d) \n\r",
           wpn_spc[flags->value[0]],flags->value[0], flags->value[1], flags->value[2],
           mindam,maxdam,avgdam,apt, weapon_type[flags->value[3]], flags->value[3]);
    else if((flags->value[0]>50) && (flags->value[0]<101) &&
       (flags->value[3]>-1) && (flags->value[3]<15) )
      sprintf(str, "Extra: %s(%d)\n\rDmg:%dD%d (%ld-%ld, Avg. %ld%s)\n\rType : %s(%d) \n\r",
           wpn_spc[flags->value[0]],flags->value[0], flags->value[1], flags->value[2],
           mindam,maxdam,avgdam,apt, weapon_type[flags->value[3]], flags->value[3]);
    else if((flags->value[0]>300) && (flags->value[0]<312) &&
       (flags->value[3]>-1) && (flags->value[3]<15) )
      sprintf(str, "Extra: %s Weapon(%d)\n\rDmg:%dD%d (%ld-%ld, Avg. %ld%s)\n\rType : %s(%d) \n\r",
           pc_class_types[flags->value[0]-300],flags->value[0], flags->value[1], flags->value[2],
           mindam,maxdam,avgdam,apt, weapon_type[flags->value[3]], flags->value[3]);
    else
      sprintf(str, "Extra: OoB?(%d)\n\rDmg:%dD%d (%ld-%ld, Avg. %ld%s)\n\rType : OoB?(%d) \n\r",
           flags->value[0], flags->value[1], flags->value[2],
           mindam,maxdam,avgdam,apt, flags->value[3]);
    break;
  case ITEM_FIREARM :
    maxdam=flags->value[2]*flags->value[3];
    mindam=flags->value[2];
    avgdam=((flags->value[1]+flags->value[2]
            *flags->value[3])/2);
             sprintf(apt, ((mindam+maxdam)%2 ? ".5" : ""));
    sprintf(str, "License Number : %d\n\rNumber of bullets left : %d\n\rDmg: %dD%d (%ld-%ld, Avg. %ld%s)\n\r",
         flags->value[0],
         flags->value[1],
         flags->value[2],
         flags->value[3],
            mindam,maxdam,avgdam,apt);
    break;
  case ITEM_MISSILE :
    maxdam=flags->value[0]*flags->value[1];
    mindam=flags->value[1];
    avgdam=((flags->value[0]+flags->value[0]*flags->value[1])/2);
    sprintf(apt, ((mindam+maxdam)%2 ? ".5" : ""));

    sprintf(str, "Dmg : %dD%d (%ld-%ld, Avg. %ld%s)  Not used: [%d] [%d])\n\r",
         flags->value[0],
         flags->value[1],
            mindam,maxdam,avgdam,apt,
         flags->value[2],
         flags->value[3]);
    break;
  case ITEM_ARMOR :
    sprintf(str, "AC-apply : [%d]\n\r(not used: [%d] [%d] [%d])\n\r",
         flags->value[0],
         flags->value[1],
         flags->value[2],
         flags->value[3]);
    break;
  case ITEM_TRAP :
    sprintf(str, "-Spell : %d\n\r-Hitpoints : %d\n\r(not used: [%d] [%d])\n\r",
         flags->value[0],
         flags->value[1],
         flags->value[2],
         flags->value[3]);
    break;
  case ITEM_CONTAINER :
    sprintf(str, "Max-contains : %d\n\rLocktype : %d\n\rKey: %d\n\rCorpse : %s\n\r",
         flags->value[0],
         flags->value[1],
         flags->value[2],
         flags->value[3] ? "Yes" : "No");
    break;
  case ITEM_NOTE :
    sprintf(str, "-Tongue : %d\n\r(not used: [%d] [%d] [%d])\n\r",
         flags->value[0],
         flags->value[1],
         flags->value[2],
         flags->value[3]);
    break;
  case ITEM_DRINKCON :
    sprinttype(flags->value[2], drinks, buf2);
    sprintf(str, "Max-contains : %d\n\rContains : %d\n\rLiquid : %s \n\rPoisoned : %s\n\r",
         flags->value[0],
         flags->value[1],
         buf2,
         flags->value[3] ? "Yes" : "No");
    break;
  case ITEM_BULLET :
    sprintf(str, "(not used: [%d] [%d])\n\rFor the gun of license number : %d\n\r(not used: [%d])\n\r",
         flags->value[0],
         flags->value[1],
         flags->value[2],
         flags->value[3]);
    break;
  case ITEM_KEY :
    sprintf(str, "Keytype : %d\n\r(not used: [%d] [%d] [%d])\n\r",
         flags->value[0],
         flags->value[1],
         flags->value[2],
         flags->value[3]);
    break;
  case ITEM_FOOD :
    sprintf(str, "Makes full : %d\n\r(not used: [%d] [%d])\n\rPoisoned : %s\n\r",
         flags->value[0],
         flags->value[1],
         flags->value[2],
         flags->value[3] ? "Yes" : "No");
    break;
  case ITEM_MONEY :
    sprintf(str, "Number of coins: %d\n\r(not used: [%d] [%d] [%d])\n\r",
         flags->value[0],
         flags->value[1],
         flags->value[2],
         flags->value[3]);
    break;
  default :
    sprintf(str, "Values 0-3 : [%d] [%d] [%d] [%d]",
         flags->value[0],
         flags->value[1],
         flags->value[2],
         flags->value[3]);
    break;
  } /* switch */
}

void do_ostat(struct char_data *ch, char *argument, int cmd) {
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH],ltd[4];
  int number, r_number, i, found = FALSE;
  int type, wear,zonenum;
  struct obj_proto *proto;
  struct extra_descr_data *desc;
  struct string_block sb;
  char usage_text[] = "Usage: ostat (w)ear <number>\n\r\
or ostat a(c)tion junktext\n\r    or ostat (a)ffect <name>\n\r\
or ostat (t)ype < 1 - 24 >\n\r    or ostat (f)rom <real number>\n\r\
or ostat (b)itvector <number>\n\r    or ostat (e)xtra <number>\n\r\
or ostat <item name>\n\r    or ostat <item number>\n\r";

  if(!check_god_access(ch,FALSE)) return;

  argument = one_argument(argument, buf);

  if(!*buf) {
    send_to_char(usage_text, ch);
    return;
  }
  if(is_number(buf)) {
    number = atoi(buf);
    if(number < 0 || (r_number = real_object(number)) < 0) {
      send_to_char("No such object in object file.\n\r", ch);
      return;
    }
    zonenum=inzone(number);
    if(zone_locked(ch,zonenum)) return;
    proto = &obj_proto_table[r_number];

    sprintf(buf,"WIZINFO: %s ostat %d (%s)",GET_NAME(ch),number,proto->name);
    wizlog(buf, GET_LEVEL(ch)+1, 5);
    log_s(buf);

    if(IS_SET(proto->obj_flags.extra_flags, ITEM_LIMITED) && GET_LEVEL(ch) < LEVEL_WIZ &&
       !isname(GET_NAME(ch),zone_table[real_zone(zonenum)].name) && !strstr(zone_table[real_zone(zonenum)].creators,GET_NAME(ch))) {
      send_to_char("Sorry this item is limited.\n\r", ch);
      return;
    }

    sprinttype(proto->obj_flags.type_flag, item_types, buf2);
    sprintf(buf, "Object name: [%s]\n\rR-number: [%d], V-number: [%d], In-game: [%d] Item type: %s\n\r",
         proto->name, r_number, obj_proto_table[r_number].virtual,obj_proto_table[r_number].number, buf2);
    send_to_char(buf, ch);

    sprintf(buf, "Short description: %s\n\rLong description:\n\r%s\n\r",
         ((proto->short_description) ? proto->short_description : "None"),
         ((proto->description) ? proto->description : "None"));
    send_to_char(buf, ch);

    if(proto->ex_description) {
      sprintf(buf,"Extra description keyword(s):\n\r------------\n\r");
      for(desc = proto->ex_description; desc; desc = desc->next) {
        strcat(buf, desc->keyword);
        strcat(buf, "\n\r");
      }
      strcat(buf, "------------\n\r");
      send_to_char(buf, ch);
    } else {
      send_to_char("Extra description keyword(s): None\n\r", ch);
    }

    /* Added action descript - Ranger June 96 */
    printf_to_char(ch, "Action description: %s\n\r",
            ((proto->action_description) ? proto->action_description : "None"));
    printf_to_char(ch, "Action description (no target): %s\n\r",
            ((proto->action_description_nt) ? proto->action_description_nt : "None"));
    printf_to_char(ch, "Character wear description: %s\n\r",
            ((proto->char_wear_desc) ? proto->char_wear_desc : "None"));
    printf_to_char(ch, "Room wear description: %s\n\r",
            ((proto->room_wear_desc) ? proto->room_wear_desc : "None"));
    printf_to_char(ch, "Character remove description: %s\n\r",
            ((proto->char_rem_desc) ? proto->char_rem_desc : "None"));
    printf_to_char(ch, "Room remove description: %s\n\r",
            ((proto->room_rem_desc) ? proto->room_rem_desc : "None"));

    sprintbit((long)proto->obj_flags.wear_flags, wear_bits, buf2);
    printf_to_char(ch, "Can be worn on: %s\n\r", buf2);

    sprintbit(proto->obj_flags.bitvector, affected_bits, buf2);
    printf_to_char(ch, "Set char bits: %s\n\r", buf2);
    sprintbit(proto->obj_flags.bitvector2, affected_bits2, buf2);
    printf_to_char(ch, "               %s\n\r", buf2);

    sprintbit(proto->obj_flags.extra_flags, extra_bits, buf2);
    sprintf(buf, "Extra flags: %s", buf2);
    sprintbit(proto->obj_flags.extra_flags2, extra_bits2, buf2);
    printf_to_char(ch, "%s %s\n\r",buf,buf2);

    sprintbit(proto->obj_flags.subclass_res, subclass_res_bits, buf2);
    printf_to_char(ch, "Subclass Restrictions: %s\n\r", buf2);

    /* Repop added by Ranger */

    printf_to_char(ch,"Repop Rate: %d\n\r",obj_proto_table[r_number].obj_flags.repop_percent);

    printf_to_char(ch, "Weight: %d, Value: %d, Cost/day: %d, Timer: %d\n\r",
         proto->obj_flags.weight, proto->obj_flags.cost,
         proto->obj_flags.cost_per_day, proto->obj_flags.timer);
    printf_to_char(ch, "Material Number: %d.\n\r",proto->obj_flags.material);

    item_type_flag_to_string(&proto->obj_flags, buf);
    send_to_char(buf, ch);

    printf_to_char(ch, "Special procedure: %s\n\r",
         (obj_proto_table[r_number].func) ? "Exists" : "No");

    send_to_char("Can affect char:\n\r", ch);
    for(i = 0; i < MAX_OBJ_AFFECT; i++) {
      sprinttype(proto->affected[i].location, apply_types, buf2);
      printf_to_char(ch, "    Affects: %s by %d\n\r", buf2, proto->affected[i].modifier);
    }
  } else {
    sprintf(buf2,"WIZINFO: %s ostat %s %s",GET_NAME(ch),buf,argument);
    wizlog(buf2, GET_LEVEL(ch)+1, 5);
    log_s(buf2);

    one_argument(argument, buf2);
    init_string_block(&sb);
    if(*buf2) {
     switch(*buf) {
      case 'a': {
        if(GET_LEVEL(ch)<LEVEL_WIZ) {
          send_to_char("You cannot use this option.\n\r",ch);
          return;
        }
        wear = old_search_block(string_to_upper(buf2), 0, strlen(buf2), apply_types, FALSE);
        if(wear!=-1) {
          wear--;
          for(i = 0; i <= top_of_objt; i++) {
            if(obj_proto_table[i].affected[0].location==wear) {
              sprintbit((long)(obj_proto_table[i].obj_flags.wear_flags-1), wear_bits, buf);
              if (IS_SET(obj_proto_table[i].obj_flags.extra_flags, ITEM_LIMITED)) sprintf(ltd,"*");
              else sprintf(ltd," ");
              sprintf(buf2, "[%-5d]%s (%d) %s (%s) [%s]\n\r",
                obj_proto_table[i].virtual,ltd,
                obj_proto_table[i].affected[0].modifier,
                obj_proto_table[i].short_description,
                obj_proto_table[i].name,buf);
              append_to_string_block(&sb, buf2);
              found = TRUE;
            }
            else if(obj_proto_table[i].affected[1].location==wear) {
              sprintbit((long)(obj_proto_table[i].obj_flags.wear_flags-1), wear_bits, buf);
              if (IS_SET(obj_proto_table[i].obj_flags.extra_flags, ITEM_LIMITED)) sprintf(ltd,"*");
              else sprintf(ltd," ");
              sprintf(buf2, "[%-5d]%s (%d) %s (%s) [%s]\n\r",
                obj_proto_table[i].virtual,ltd,
                obj_proto_table[i].affected[1].modifier,
                obj_proto_table[i].short_description,
                obj_proto_table[i].name,buf);
              append_to_string_block(&sb, buf2);
              found = TRUE;
            }
            else if(obj_proto_table[i].affected[2].location==wear) {
              sprintbit((long)(obj_proto_table[i].obj_flags.wear_flags-1), wear_bits, buf);
              if (IS_SET(obj_proto_table[i].obj_flags.extra_flags, ITEM_LIMITED)) sprintf(ltd,"*");
              else sprintf(ltd," ");
              sprintf(buf2, "[%-5d]%s (%d) %s (%s) [%s]\n\r",
                obj_proto_table[i].virtual,ltd,
                obj_proto_table[i].affected[2].modifier,
                obj_proto_table[i].short_description,
                obj_proto_table[i].name,buf);
              append_to_string_block(&sb, buf2);
              found = TRUE;
            }
          }
        }
      } break;
      case 'w': {
        if(GET_LEVEL(ch)<LEVEL_WIZ) {
          send_to_char("You cannot use this option.\n\r",ch);
          return;
        }
        wear = atoi(buf2);
        REMOVE_BIT(wear, 1);
        for(i = 0; i <= top_of_objt; i++) {
          if(IS_SET(obj_proto_table[i].obj_flags.wear_flags, wear)) {
            if (IS_SET(obj_proto_table[i].obj_flags.extra_flags, ITEM_LIMITED)) sprintf(ltd,"*");
            else sprintf(ltd," ");
            sprintf(buf2, "[%-5d]%s %s (%s)\n\r",
              obj_proto_table[i].virtual,ltd,
              obj_proto_table[i].short_description,
              obj_proto_table[i].name);
            append_to_string_block(&sb, buf2);
            found = TRUE;
          }
        }
      } break;
      case 'c': {
        if(GET_LEVEL(ch)<LEVEL_WIZ) {
          send_to_char("You cannot use this option.\n\r",ch);
          return;
        }
        for(i = 0; i <= top_of_objt; i++) {
          if(obj_proto_table[i].action_description) {
            if(IS_SET(obj_proto_table[i].obj_flags.extra_flags, ITEM_LIMITED)) sprintf(ltd,"*");
            else sprintf(ltd," ");
            sprintf(buf2, "[%-5d]%s %s (%s)\n\r",
              obj_proto_table[i].virtual,ltd,
              obj_proto_table[i].short_description,
              obj_proto_table[i].name);
            append_to_string_block(&sb, buf2);
            found = TRUE;
          }
        }
      } break;
      case 'b': {
        if(GET_LEVEL(ch)<LEVEL_WIZ) {
          send_to_char("You cannot use this option.\n\r",ch);
          return;
        }
        wear = atoi(buf2);
        buf2[0]=0;
        for(i = 0; i <= top_of_objt; i++) {
          if (IS_SET(obj_proto_table[i].obj_flags.bitvector, wear)) {
            if (IS_SET(obj_proto_table[i].obj_flags.extra_flags, ITEM_LIMITED)) sprintf(ltd,"*");
            else sprintf(ltd," ");
            sprintf(buf2, "[%-5d]%s %s (%s)\n\r",
              obj_proto_table[i].virtual,ltd,
              obj_proto_table[i].short_description,
              obj_proto_table[i].name);
            append_to_string_block(&sb, buf2);
            found = TRUE;
          }
        }
      } break;
      case 'e': {
        if(GET_LEVEL(ch)<LEVEL_WIZ) {
          send_to_char("You cannot use this option.\n\r",ch);
          return;
        }
        wear = atoi(buf2);
        buf2[0]=0;
        for(i = 0; i <= top_of_objt; i++) {
          if (IS_SET(obj_proto_table[i].obj_flags.extra_flags, wear)) {
            if (IS_SET(obj_proto_table[i].obj_flags.extra_flags, ITEM_LIMITED)) sprintf(ltd,"*");
            else sprintf(ltd," ");
            sprintf(buf2, "[%-5d]%s %s (%s)\n\r",
              obj_proto_table[i].virtual,ltd,
              obj_proto_table[i].short_description,
              obj_proto_table[i].name);
            append_to_string_block(&sb, buf2);
            found = TRUE;
          }
        }
      } break;
      case 't': {
        if(GET_LEVEL(ch)<LEVEL_WIZ) {
          send_to_char("You cannot use this option.\n\r",ch);
          return;
        }
        type = atoi(buf2);
        buf2[0]=0;
        for(i = 0; i <= top_of_objt; i++) {
          if(obj_proto_table[i].obj_flags.type_flag == type) {
            if (IS_SET(obj_proto_table[i].obj_flags.extra_flags, ITEM_LIMITED)) sprintf(ltd,"*");
            else sprintf(ltd," ");
            sprintf(buf2, "[%-5d]%s %s (%s)\n\r",
              obj_proto_table[i].virtual,ltd,
              obj_proto_table[i].short_description,
              obj_proto_table[i].name);
            append_to_string_block(&sb, buf2);
            found = TRUE;
          }
        }
      } break;
      case 'f': {
        if(GET_LEVEL(ch)<LEVEL_WIZ) {
          send_to_char("You cannot use this option.\n\r",ch);
          return;
        }
        number = atoi(buf2);
        buf2[0]=0;
        for(i = number; i <= top_of_objt && i <= number + 25; i++) {
          if (IS_SET(obj_proto_table[i].obj_flags.extra_flags, ITEM_LIMITED)) sprintf(ltd,"*");
          else sprintf(ltd," ");
          sprintf(buf2, "%4d.[%-5d]%s %s (%s)\n\r",i,
            obj_proto_table[i].virtual,ltd,
            obj_proto_table[i].short_description,
            obj_proto_table[i].name);
            append_to_string_block(&sb, buf2);
          found = TRUE;
        }
      } break;
      default: {
        send_to_char(usage_text, ch);
        destroy_string_block(&sb);
        return;
      } break;
     }
     if(!found) {
       send_to_char("There isn't any such objects in object file.\n\r", ch);
       destroy_string_block(&sb);
       return;
     }
    } else {
      for( i = 0; i <= top_of_objt; i++) {
        if(isname(buf, obj_proto_table[i].name)) {
          if(IS_SET(obj_proto_table[i].obj_flags.extra_flags, ITEM_LIMITED)) {
            sprintf(ltd,"*");
            if(GET_LEVEL(ch)<LEVEL_WIZ) continue;
          }
          else sprintf(ltd," ");
          sprintf(buf2, "[%-5d]%s %s (%s)\n\r",
            obj_proto_table[i].virtual,ltd,
            obj_proto_table[i].short_description,
            obj_proto_table[i].name);
            append_to_string_block(&sb, buf2);
          found = TRUE;
        }
      }
    }

    if (found) page_string_block(&sb,ch);
    else {
      sprintf(buf2, "There isn't such objects as '%s' in object file.\n\r", buf);
      send_to_char(buf2, ch);
    }
    destroy_string_block(&sb);
  }
}

void show_zone_to_char(CHAR *ch, int zoneNum)
{

  int tab = 0, cmd_no, mobile, room, object, i,object_to;
  struct string_block sb;
  struct zone_data *zone;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  init_string_block(&sb);
  zone=&zone_table[zoneNum];
  sprintf(buf,"Zone name: %s.\n\rCreators: %s\n\r",
          zone->name,zone->creators);
  append_to_string_block(&sb, buf);
  sprintf(buf,"Creation Date: %s   Modify Date: %s\n\r",
          zone->create_date,zone->mod_date);
  append_to_string_block(&sb,buf);
  sprintf(buf,"  Climate: %d Lifespan: %d Age: %d  Bottom: %d Top: %d\n\r",
          zone->climate,zone->lifespan,zone->age,zone->bottom,zone->top);
  append_to_string_block(&sb,buf);

  switch(zone->reset_mode)
    {
    case 0:
     append_to_string_block(&sb,"  Zone never resets.\n\r");
     break;
    case 1:
     append_to_string_block(&sb,"  Zone resets when empty.\n\r");
     break;
    case 2:
     append_to_string_block(&sb,"  Zone resets regardless.\n\r");
     break;
    case 3:
     append_to_string_block(&sb,"  Zone reset is blocked.\n\r");
     break;
    case 4:
     append_to_string_block(&sb,"  Zone is locked.\n\r");
     break;
    case 5:
     append_to_string_block(&sb,"  Only doors reset.\n\r");
     break;
    default:
     append_to_string_block(&sb,"  Invalid reset mode!\n\r");
     break;
    }

#ifdef TEST_SITE
  if (GET_LEVEL(ch) >= LEVEL_DEI || isname(GET_NAME(ch),zone->name) || strstr(zone->creators,GET_NAME(ch))) {
#else
  if (GET_LEVEL(ch) >= LEVEL_ETE || isname(GET_NAME(ch),zone->name) || strstr(zone->creators,GET_NAME(ch))) {
#endif

    sprintf(buf," Multipliers:  Hp Mana Hitroll Damage Armor  Xp Gold Level\n\r");
    append_to_string_block(&sb,buf);
    sprintf(buf,"              %3d  %3d   %3d    %3d    %3d  %3d  %3d  %3d\n\r",
            zone->mult_hp,zone->mult_mana,zone->mult_hitroll,zone->mult_damage,
            zone->mult_armor,zone->mult_xp,zone->mult_gold,zone->mult_level);
    append_to_string_block(&sb,buf);

    for (cmd_no = 0;;cmd_no++)
      {
     if (zone->cmd[cmd_no].command == 'S')
       break;

     switch(zone->cmd[cmd_no].command)
       {
       case 'M': /* read a mobile */
         tab = 2;
         mobile  = real_mobile(zone->cmd[cmd_no].arg1);
         room    = real_room(zone->cmd[cmd_no].arg3);
         if(mobile != -1 && room != -1 )
           {
          sprintf(buf,"%3d) ",cmd_no);
          for(i=0;i<tab;i++)
            strcat(buf," ");
          append_to_string_block(&sb,buf);
          sprintf(buf,"Load mobile %s (#%d) (if %d) (max %d) in %s (#%d).\n\r",mob_proto_table[mobile].name,
                        zone->cmd[cmd_no].arg1, zone->cmd[cmd_no].if_flag, zone->cmd[cmd_no].arg2,
               world[room].name, zone->cmd[cmd_no].arg3); /* Added Max & if - Ranger April 96*/
          tab += 2;
          append_to_string_block(&sb,buf);
           }
         break;

/* Follower - Ranger June 96 */
       case 'F': /* add a follower */
         tab = 2;
         mobile  = real_mobile(zone->cmd[cmd_no].arg1);
         room    = real_room(zone->cmd[cmd_no].arg3);
         if(mobile != -1 && room != -1 )
           {
          sprintf(buf,"%3d) ",cmd_no);
          for(i=0;i<tab;i++)
            strcat(buf," ");
          append_to_string_block(&sb,buf);
          sprintf(buf,"Load follower %s (#%d) (if %d) (max %d) in %s (#%d).\n\r",mob_proto_table[mobile].name,
                        zone->cmd[cmd_no].arg1, zone->cmd[cmd_no].if_flag, zone->cmd[cmd_no].arg2,
               world[room].name, zone->cmd[cmd_no].arg3); /* Added Max & if - Ranger April 96*/
          tab += 2;
          append_to_string_block(&sb,buf);
           }
         break;

       /* Mount - Ranger June 96 */
       case 'R': /* add a mount */
         tab = 2;
         mobile  = real_mobile(zone->cmd[cmd_no].arg1);
         room    = real_room(zone->cmd[cmd_no].arg3);
         if(mobile != -1 && room != -1 )
           {
          sprintf(buf,"%3d) ",cmd_no);
          for(i=0;i<tab;i++)
            strcat(buf," ");
          append_to_string_block(&sb,buf);
          sprintf(buf,"Ride %s (#%d) (if %d) (max %d) in %s (#%d).\n\r",mob_proto_table[mobile].name,
                        zone->cmd[cmd_no].arg1, zone->cmd[cmd_no].if_flag, zone->cmd[cmd_no].arg2,
               world[room].name, zone->cmd[cmd_no].arg3); /* Added Max & if - Ranger April 96*/
          tab += 2;
          append_to_string_block(&sb,buf);
           }
         break;

       case 'O': /* read an object */
         tab = 2;
         object = real_object(zone->cmd[cmd_no].arg1);
         room   = real_room(zone->cmd[cmd_no].arg3);
         if(object != -1 && room != -1)
           {
          sprintf(buf,"%3d) ",cmd_no);
          for(i=0;i<tab;i++)
            strcat(buf," ");
          append_to_string_block(&sb,buf);

          sprintf(buf,"Load object %s (#%d) (if %d) (max %d) in %s (#%d).\n\r",obj_proto_table[object].name,
                        zone->cmd[cmd_no].arg1, zone->cmd[cmd_no].if_flag, zone->cmd[cmd_no].arg2,
               world[room].name, zone->cmd[cmd_no].arg3); /* Added Max & if - Ranger April 96*/
          append_to_string_block(&sb,buf);
           }
         break;

       case 'T': /* take an object */
         tab = 2;
         object = real_object(zone->cmd[cmd_no].arg1);
         room   = real_room(zone->cmd[cmd_no].arg3);
         if(object != -1 && room != -1)
           {
          sprintf(buf,"%3d) ",cmd_no);
          for(i=0;i<tab;i++)
            strcat(buf," ");
          append_to_string_block(&sb,buf);

          sprintf(buf,"Take object %s (#%d) (if %d) (max %d (0=All)) in %s (#%d).\n\r",obj_proto_table[object].name,
                        zone->cmd[cmd_no].arg1, zone->cmd[cmd_no].if_flag, zone->cmd[cmd_no].arg2,
               world[room].name, zone->cmd[cmd_no].arg3);
          append_to_string_block(&sb,buf);
           }
         break;

       case 'P': /* object to object */
         if(zone->cmd[cmd_no-1].command != 'P')
           tab+=2;
         object = real_object(zone->cmd[cmd_no].arg1);
         object_to = real_object(zone->cmd[cmd_no].arg3);
         if(object != -1 && object_to != -1)
           {
          sprintf(buf,"%3d) ",cmd_no);
          for(i=0;i<tab;i++)
            strcat(buf," ");
          append_to_string_block(&sb,buf);

          sprintf(buf,"Put object %s (#%d) (if %d) (max %d) in it.\n\r",obj_proto_table[object].name,
                        zone->cmd[cmd_no].arg1, zone->cmd[cmd_no].if_flag, zone->cmd[cmd_no].arg2); /* Added Max & if - Ranger April 96*/
          append_to_string_block(&sb,buf);
           }
         break;

       case 'G': /* obj_to_char */
         object = real_object(zone->cmd[cmd_no].arg1);
         if(object != -1)
           {
          sprintf(buf,"%3d) ",cmd_no);
          for(i=0;i<tab;i++)
            strcat(buf," ");
          append_to_string_block(&sb,buf);

          sprintf(buf,"Give %s (#%d) (if %d) (max %d).\n\r",obj_proto_table[object].name,
                        zone->cmd[cmd_no].arg1,zone->cmd[cmd_no].if_flag,zone->cmd[cmd_no].arg2); /* Added Max & if - Ranger April 96*/
          append_to_string_block(&sb,buf);
           }
         break;

       case 'E': /* object to equipment list */
         object = real_object(zone->cmd[cmd_no].arg1);
         if(object != -1)
           {
          sprintf(buf,"%3d) ",cmd_no);
          for(i=0;i<tab;i++)
            strcat(buf," ");
          append_to_string_block(&sb,buf);

          sprintf(buf,"Equip %s (#%d) (if %d) (max %d) in the %s(#%d) position.\n\r",obj_proto_table[object].name,
                        zone->cmd[cmd_no].arg1,zone->cmd[cmd_no].if_flag,zone->cmd[cmd_no].arg2,
               equipment_types[zone->cmd[cmd_no].arg3],zone->cmd[cmd_no].arg3); /* Added Max & if - Ranger April 96*/
          append_to_string_block(&sb,buf);
           }
         break;

       case 'D': /* set state of door */
         room  = real_room(zone->cmd[cmd_no].arg1);
         if(room != -1)
           {
          tab = 2;
          sprintf(buf,"%3d) ",cmd_no);
          for(i=0;i<tab;i++)
            strcat(buf," ");
          append_to_string_block(&sb,buf);
          switch (zone->cmd[cmd_no].arg3)
            {
            case 0:
              sprintf(buf2, "unlocked and open");
              break;
            case 1:
              sprintf(buf2, "unlocked, but closed");
              break;
            case 2:
              sprintf(buf2, "locked and closed");
              break;
            }
                /* Check on door resets added by Ranger - May 96 */
                if(!world[room].dir_option[zone->cmd[cmd_no].arg2]) {
             sprintf(buf,"Set the door %s from room #%d to (direction doesn't exist).\n\r",
                 dirs[zone->cmd[cmd_no].arg2], zone->cmd[cmd_no].arg1);
            append_to_string_block(&sb,buf);
                }
                else if(world[room].dir_option[zone->cmd[cmd_no].arg2]->to_room_v==0) {
                    sprintf(buf,"Set the door %s from room #%d to (no room in that direction).\n\r",
               dirs[zone->cmd[cmd_no].arg2], zone->cmd[cmd_no].arg1);
                    append_to_string_block(&sb,buf);
                }
                /* End of door reset check */
                else {
                  sprintf(buf,"Set the door %s from room #%d to #%d to %s.\n\r",
               dirs[zone->cmd[cmd_no].arg2], zone->cmd[cmd_no].arg1,
               world[room].dir_option[zone->cmd[cmd_no].arg2]->to_room_v,
               buf2);
            append_to_string_block(&sb,buf);
             }
           }
         break;

       default:
         sprintf(buf, "Undefd cmd in reset table; zone %d cmd %d.\n\r",
              (int)zone, (int)cmd_no);
         append_to_string_block(&sb,buf);
         break;
       }
      }
  }
  page_string_block(&sb, ch);
  destroy_string_block(&sb);
  return;
}

void do_snooplist(struct char_data *ch, char *argument, int cmd) {
 struct descriptor_data *d;
 char buf[MAX_STRING_LENGTH];
 int count=0;

  if(!check_god_access(ch,FALSE)) return;

    send_to_char("Snooplist:\n\r",ch);
    send_to_char("-----------\n\r",ch);

    for (d = descriptor_list; d; d = d->next) {
     if (d->character && (d->connected == CON_PLYNG)
      && d->character->desc->snoop.snoop_by && (GET_LEVEL(ch)>=GET_LEVEL(d->character->desc->snoop.snoop_by))) {
        sprintf(buf,"%s(%d) snooped by %s(%d)\n\r",GET_NAME(d->character),GET_LEVEL(d->character),GET_NAME(d->character->desc->snoop.snoop_by),GET_LEVEL(d->character->desc->snoop.snoop_by));
        send_to_char(buf,ch);
        count++;
  }
 }
    send_to_char("--------------------------\n\r",ch);
    sprintf(buf,"Total:%d\n\r",count);
    send_to_char(buf,ch);
}

void do_mstat(struct char_data *ch, char *argument, int cmd) {
  char buf[MAX_STRING_LENGTH*2],buf2[MAX_STRING_LENGTH],buf3[MAX_STRING_LENGTH];
  char apt[3];
  char usage_text[] = "\
Usage: mstat ac(t) <ACT>\n\r\
             (i)mmune <IMMUNE>\n\r\
             af(f) <AFF>\n\r\
             (l)evel <#>\n\r\
             <mob name>\n\r\
             <mob number>\n\r";
  struct mob_proto *proto;
  struct tagline_data *tag;
  int r_number, number=-1,zonenum, i, found = 0,act,aff,immune,resist;
  long int mindam,maxdam,avgdam,tmp;
  struct string_block sb;

  argument=one_argument(argument, buf);

  if (!*buf) {
    send_to_char(usage_text,ch);
    return;
  }

  if (is_number(buf)) {
    number = atoi(buf);
    if(number < 0 || (r_number = real_mobile(number)) < 0) {
      send_to_char("No such object in mobile file.\n\r", ch);
      return;
    }
    zonenum=inzone(number);
    if(zone_locked(ch,zonenum)) return;

    proto = &mob_proto_table[r_number];

    sprintf(buf,"WIZINFO: %s mstat %d (%s)",GET_NAME(ch),number,proto->name);
    wizlog(buf, LEVEL_IMP, 5);
    log_s(buf);

    sprintf(buf, "Mobile name: [%s]\n\rR-number: [%d], V-number: [%d], In-game: [%d]\n\r",
         proto->name, r_number, proto->virtual, proto->number);
    send_to_char(buf, ch);
    send_to_char("Short description:  ", ch);
    send_to_char(((proto->short_descr) ? proto->short_descr : "None") , ch);
    send_to_char("\r\nLong description:\r\n", ch);
    send_to_char(((proto->long_descr) ? proto->long_descr : "None"), ch);
    send_to_char("Full description:\r\n", ch);
    send_to_char(((proto->description) ? proto->description : "None"), ch);

    sprinttype(proto->class,npc_class_types,buf2);
    sprintf(buf,"Monster Class: %s(%d)\n\r",buf2,proto->class);
    send_to_char(buf,ch);

    switch(proto->sex) {
    case SEX_NEUTRAL :
      strcpy(buf,"NEUTRAL-SEX");
      break;
    case SEX_MALE :
      strcpy(buf,"MALE");
      break;
    case SEX_FEMALE :
      strcpy(buf,"FEMALE");
      break;
    default:
      strcpy(buf,"ILLEGAL-SEX!!");
      break;
    }

    sprintf(buf2,"   Level [%d]   Alignment [%d]\n\r",proto->level, proto->alignment);
    strcat(buf, buf2);
    send_to_char(buf, ch);

    sprintf(buf,"AC:[%d/10], Exp: [%d], Hitroll: [%d]\n\r", proto->armor, proto->exp, proto->hitroll);
    send_to_char(buf,ch);

    sprintf(buf,"Gold: [%d] \n\r", proto->gold);
    send_to_char(buf,ch);

    sprinttype(proto->position,position_types,buf2);
    sprinttype(proto->default_pos,position_types,buf3);
    sprintf(buf,"Position: %s, Default:  %s ",buf2, buf3);
    strcat(buf,"\n\r");
    send_to_char(buf, ch);

    buf[0] = 0;
    strcat(buf,"NPC flags: ");
    sprintbit(proto->act,action_bits,buf2);
    strcat(buf, buf2);
    sprintbit(proto->act2,action_bits2,buf2);
    strcat(buf, buf2);
    send_to_char(buf, ch);

    /* New stuff - Ranger Sept 96 */
    strcpy(buf,"\n\rMobile Immunities: ");
    sprintbit(proto->immune,immune_bits,buf2);
    strcat(buf,buf2);
    strcat(buf," ");
    sprintbit(proto->immune2,immune_bits2,buf2);
    strcat(buf,buf2);
    strcat(buf,"\n\r");
    send_to_char(buf,ch);

    strcpy(buf,"Mobile Resistance: ");
    sprintbit(proto->resist,resist_bits,buf2);
    strcat(buf,buf2);
    strcat(buf,"\n\r");
    send_to_char(buf,ch);

    sprintf(buf,"Mobile Attacks: %d\n\r",proto->no_att);
    send_to_char(buf,ch);
    if(proto->no_att>0) {
       for (i=0;i<proto->no_att;i++) {
          if(i==MAX_ATTS) break;
          sprintf(buf,"%d) ",i+1);
          sprinttype(proto->att_type[i],att_types,buf2);
          strcat(buf2," ");
          strcat(buf,buf2);
          sprinttype(proto->att_target[i],att_targets,buf2);
          strcat(buf2," ");
          strcat(buf,buf2);
          sprintf(buf2,"%d ",proto->att_percent[i]);
          strcat(buf,buf2);
          if(proto->att_type[i]==ATT_SPELL_CAST) {
            sprinttype(proto->att_spell[i]-1,spells,buf2);
            strcat(buf,CAP(buf2));
         strcat(buf," (cast)");
          }
          if(proto->att_type[i]==ATT_SPELL_SKILL) {
            sprinttype(proto->att_spell[i]-1,spells,buf2);
            strcat(buf,CAP(buf2));
         strcat(buf," (skill)");
          }


          strcat(buf,"\n\r");
          send_to_char(buf,ch);
       }
    }

    printf_to_char(ch,"Skin Value: %d coins\n\r",proto->skin_value);
    printf_to_char(ch,"Possible objects from corpse: %d %d %d %d %d %d\n\r",
                     proto->skin_vnum[0],proto->skin_vnum[1], proto->skin_vnum[2],
                     proto->skin_vnum[3],proto->skin_vnum[4], proto->skin_vnum[5]);

    printf_to_char(ch,"Zone Multipliers:  Hp Mana Hitroll Damage Armor  Xp Gold Level\n\r");
    printf_to_char(ch,"                  %3d  %3d   %3d    %3d    %3d  %3d  %3d  %3d\n\r",
                   zone_table[real_zone(zonenum)].mult_hp,zone_table[real_zone(zonenum)].mult_mana,
                   zone_table[real_zone(zonenum)].mult_hitroll,zone_table[real_zone(zonenum)].mult_damage,
                   zone_table[real_zone(zonenum)].mult_armor,zone_table[real_zone(zonenum)].mult_xp,
                   zone_table[real_zone(zonenum)].mult_gold,zone_table[real_zone(zonenum)].mult_level);

    strcpy(buf, "Mobile Special procedure : ");
    strcat(buf, (proto->func ? "Exists\n\r" : "None\n\r"));
    send_to_char(buf, ch);
    avgdam=((proto->damnodice+(proto->damnodice*proto->damsizedice))/2
           +proto->damroll);
    maxdam=(proto->damnodice*proto->damsizedice)+proto->damroll;
    mindam=(proto->damnodice+proto->damroll);
     sprintf(apt, ((mindam+maxdam)%2 ? ".5" : " "));
    if (IS_SET(proto->affected_by,AFF_FURY)) {
     avgdam  *= 2;
     maxdam  *= 2;
     mindam  *= 2;
    if((mindam+maxdam)%4) {avgdam+=1;sprintf(apt," ");}
    }

    sprintf(buf, "Normal Dmg: %dd%d+%d. (%ld-%ld, Avg. %ld%s) / (%ld-%ld, Avg. %ld)",
         proto->damnodice, proto->damsizedice, proto->damroll, mindam, maxdam, avgdam,apt,mindam/2,maxdam/2,avgdam/2);
    if (IS_SET(proto->affected_by,AFF_DUAL)) strcat(buf,"  (x2)");
    strcat(buf,"\n\r");
    send_to_char(buf, ch);
    avgdam=((proto->hp_nodice+(proto->hp_nodice*proto->hp_sizedice))/2
           +proto->hp_add);
    maxdam=(proto->hp_nodice*proto->hp_sizedice)+proto->hp_add;
    mindam=(proto->hp_nodice+proto->hp_add);

    sprintf(buf, "NPC HitPoints %dd%d+%d. (%ld-%ld, Avg. %ld)\n\r",
         proto->hp_nodice, proto->hp_sizedice, proto->hp_add, mindam, maxdam, avgdam);
    send_to_char(buf, ch);
    avgdam=((proto->mana_nodice+(proto->mana_nodice*proto->mana_sizedice))/2
           +proto->mana_add);
    maxdam=(proto->mana_nodice*proto->mana_sizedice)+proto->mana_add;
    mindam=(proto->mana_nodice+proto->mana_add);

    sprintf(buf, "NPC ManaPoints %dd%d+%d. (%ld-%ld, Avg. %ld)\n\r",
         proto->mana_nodice, proto->mana_sizedice, proto->mana_add,
            mindam, maxdam, avgdam);
    send_to_char(buf, ch);


    send_to_char("Affected by: ", ch);
    sprintbit(proto->affected_by,affected_bits,buf);
    strcat(buf," ");
    send_to_char(buf, ch);
    sprintbit(proto->affected_by2,affected_bits2,buf);
    strcat(buf,"\n\r");
    send_to_char(buf, ch);
    if(mob_proto_table[r_number].exp>0) {
      sprintf(buf, "Exp per hour (1 of 3 chars): %ld Num of mobs: %ld\n\r",exp_per_hour(r_number)/3,exp_per_hour(r_number)*3/mob_proto_table[r_number].exp);
      send_to_char(buf,ch);
    }

    if(proto->tagline) {
      sprintf(buf,"Tagline(s):\n\r--------------\n\r");
      for(tag = proto->tagline; tag; tag = tag->next) {
        strcat(buf, tag->desc);
        strcat(buf, "\n\r");
      }
      strcat(buf, "--------------\n\r");
      send_to_char(buf, ch);
    }

    return;
  } else {
    init_string_block(&sb);
    one_argument(argument, buf2);

    if(*buf2) {
      switch(*buf) {
        case 'l': {
          if(!is_number(buf2)) {
            send_to_char("Supply a level.\n\r",ch);
            destroy_string_block(&sb);
            return;
          }
          number=atoi(buf2);
          if(number<1 || number >LEVEL_MAX) {
            printf_to_char(ch,"Supply a level between 1 and %d.\n\r",LEVEL_MAX);
            destroy_string_block(&sb);
            return;
          }

          for( i = 0; i <= top_of_mobt; i++) {
            if(mob_proto_table[i].level==number) {
              sprintf(buf2, "[%-5d] (%d) %s (%s)\n\r",
                mob_proto_table[i].virtual,
                mob_proto_table[i].level,
                mob_proto_table[i].short_descr,
                mob_proto_table[i].name);
              append_to_string_block(&sb, buf2);
              found=TRUE;
            }
          }
        } break;
        case 'f': {
          aff=1;
          number = old_search_block(string_to_upper(buf2), 0, strlen(buf2), affected_bits, FALSE);
          if(number== -1) {
            aff=2;
            number = old_search_block(string_to_upper(buf2), 0, strlen(buf2), affected_bits2, FALSE);
            if(number==-1) {
              send_to_char("Not an aff flag.\n\r",ch);
              destroy_string_block(&sb);
              return;
            }
          }
          tmp = 1<<(number-1);
          for( i = 0; i <= top_of_mobt; i++) {
            if((aff==1 && IS_SET(mob_proto_table[i].affected_by,tmp)) ||
               (aff==2 && IS_SET(mob_proto_table[i].affected_by2,tmp)) ){
              sprintf(buf2, "[%-5d] %s (%s)\n\r",
                mob_proto_table[i].virtual,
                mob_proto_table[i].short_descr,
                mob_proto_table[i].name);
              append_to_string_block(&sb, buf2);
              found=TRUE;
            }
          }
        } break;
        case 't': {
          act=1;
          number = old_search_block(string_to_upper(buf2), 0, strlen(buf2), action_bits, FALSE);
          if(number== -1) {
            act=2;
            number = old_search_block(string_to_upper(buf2), 0, strlen(buf2), action_bits2, FALSE);
            if(number==-1) {
              send_to_char("Not an action flag.\n\r",ch);
              destroy_string_block(&sb);
              return;
            }
          }
          tmp = 1<<(number-1);
          for( i = 0; i <= top_of_mobt; i++) {
            if((act==1 && IS_SET(mob_proto_table[i].act,tmp)) ||
               (act==2 && IS_SET(mob_proto_table[i].act2,tmp)) ){
              sprintf(buf2, "[%-5d] %s (%s)\n\r",
                mob_proto_table[i].virtual,
                mob_proto_table[i].short_descr,
                mob_proto_table[i].name);
              append_to_string_block(&sb, buf2);
              found=TRUE;
            }
          }
        } break;
        case 'r': {
          resist=1;
          number = old_search_block(string_to_upper(buf2), 0, strlen(buf2), resist_bits, FALSE);
          if (number==-1)
          {
             send_to_char("Not a resist flag.\n\r", ch);
             destroy_string_block(&sb);
             return;
          }
        }
        tmp = 1<<(number-1);
        for (i = 0; i <= top_of_mobt; i++) {
           if (resist==1 && IS_SET(mob_proto_table[i].resist, tmp)) {
              sprintf(buf2, "[%-5d] %s (%s)\n\r",
                mob_proto_table[i].virtual,
                mob_proto_table[i].short_descr,
                mob_proto_table[i].name);
              append_to_string_block(&sb, buf2);
              found=TRUE;
           }
        }break;
        case 'i': {
          immune=1;
          number = old_search_block(string_to_upper(buf2), 0, strlen(buf2), immune_bits, FALSE);
          if(number== -1) {
            immune=2;
            number = old_search_block(string_to_upper(buf2), 0, strlen(buf2), immune_bits2, FALSE);
            if(number==-1) {
              send_to_char("Not an immunity.\n\r",ch);
              destroy_string_block(&sb);
              return;
            }
          }
          tmp = 1<<(number-1);
          for( i = 0; i <= top_of_mobt; i++) {
            if( (immune==1 && IS_SET(mob_proto_table[i].immune,tmp)) ||
                (immune==2 && IS_SET(mob_proto_table[i].immune2,tmp)) ) {
              sprintf(buf2, "[%-5d] %s (%s)\n\r",
                mob_proto_table[i].virtual,
                mob_proto_table[i].short_descr,
                mob_proto_table[i].name);
              append_to_string_block(&sb, buf2);
              found=TRUE;
            }
          }
        } break;
        default: {
          send_to_char(usage_text, ch);
          destroy_string_block(&sb);
          return;
        } break;
      }
      if(!found) {
        send_to_char("There isn't any such mob in the mob file.\n\r", ch);
        destroy_string_block(&sb);
        return;
      }
    }
    else {
      for( i = 0; i <= top_of_mobt; i++) {
        if(isname(buf, mob_proto_table[i].name)) {
          sprintf(buf2, "[%-5d] %s (%s)\n\r",
            mob_proto_table[i].virtual,
            mob_proto_table[i].short_descr,
            mob_proto_table[i].name);
          append_to_string_block(&sb, buf2);
          found=TRUE;
        }
      }
    }

    if (found) page_string_block(&sb,ch);
    else {
      sprintf(buf2, "There isn't such mobiles as '%s' in mob file.\n\r", buf);
      send_to_char(buf2, ch);
    }
    destroy_string_block(&sb);
  }
}


/* read_dlist & do_reimb - written for RoninMud by Quack

   latest mod date: May 13, 1997
     - parsed reimb with no number to the read_dlist proc
     - added reading of values, extra, bit and weight, etc
     - changed whole thing to a binary file - total revamp - Ranger

   latest mod date: Feb 10, 1999
     - added function to read last # of deaths if dlist arg is a number - Ranger
*/

void list_last_deaths(CHAR *ch, int num) {
  char buf[MAX_INPUT_LENGTH];
  struct program_info dtail;

  dtail.args[0]=strdup("tail");
  sprintf(buf,"-%d",num);
  dtail.args[1]=strdup(buf);
  dtail.args[2]=strdup("death.log");
  dtail.args[3]=NULL;
  dtail.input=NULL;
  dtail.timeout=5;
  dtail.name=strdup("dlist");
  add_program(dtail, ch);
}

int death_flag_read(FILE *fl) {
  struct death_file_check dcheck;
  memset(&dcheck,0,sizeof(dcheck));
  fread(&dcheck,sizeof(dcheck),1,fl);
  return(dcheck.flag);
}

struct obj_data *dlist_to_obj_ver3(FILE *fl,int fail) {
  struct obj_file_elem_ver3 object;
  int i;
  struct obj_data *obj=NULL;

  memset(&object,0,sizeof (object));
  fail=0;
  if( (fread(&object,sizeof(object),1,fl))!=1) {
    fail=1;
    return obj;
  }

  if(real_object(object.item_number)>-1) {
       obj=read_object(object.item_number,VIRTUAL);
       obj->obj_flags.timer      =object.timer;
       obj->obj_flags.value[0]   =object.value[0];
       obj->obj_flags.value[1]   =object.value[1];
       obj->obj_flags.value[2]   =object.value[2];
    if(obj->obj_flags.type_flag != ITEM_CONTAINER)
      obj->obj_flags.value[3]=object.value[3];
    else obj->obj_flags.value[3]=0;
    obj->obj_flags.extra_flags=object.extra_flags;
    obj->obj_flags.weight     =object.weight;
    obj->obj_flags.bitvector  =object.bitvector;
/* new obj reads */
    obj->obj_flags.type_flag    =object.type_flag;
    obj->obj_flags.wear_flags   =object.wear_flags;
    obj->obj_flags.extra_flags2 =object.extra_flags2;
    obj->obj_flags.subclass_res =object.subclass_res;
    obj->obj_flags.material     =object.material;
    OBJ_SPEC(obj)               =object.spec_value;

    for(i=0;i<MAX_OBJ_AFFECT;i++)
      obj->affected[i]  =object.affected[i];
/* end new obj reads */

    obj->obj_flags.bitvector2   = object.bitvector2;
    obj->obj_flags.popped       = object.popped;

/* New ownerid field */
    obj->ownerid[0]             =object.ownerid[0];
    obj->ownerid[1]             =object.ownerid[1];
    obj->ownerid[2]             =object.ownerid[2];
    obj->ownerid[3]             =object.ownerid[3];
    obj->ownerid[4]             =object.ownerid[4];
    obj->ownerid[5]             =object.ownerid[5];
    obj->ownerid[6]             =object.ownerid[6];
    obj->ownerid[7]             =object.ownerid[7];

  }
  else {
    if(object.item_number != 0)
      log_f("BUG: No such item #%d",object.item_number);
  }
  return obj;
}

struct obj_data *dlist_to_obj_ver2(FILE *fl,int fail) {
  struct obj_file_elem_ver2 object;
  int i;
  struct obj_data *obj=NULL;

  memset(&object,0,sizeof (object));
  fail=0;
  if( (fread(&object,sizeof(object),1,fl))!=1) {
    fail=1;
    return obj;
  }

  if(real_object(object.item_number)>-1) {
       obj=read_object(object.item_number,VIRTUAL);
       obj->obj_flags.timer      =object.timer;
       obj->obj_flags.value[0]   =object.value[0];
       obj->obj_flags.value[1]   =object.value[1];
       obj->obj_flags.value[2]   =object.value[2];
    if(obj->obj_flags.type_flag != ITEM_CONTAINER)
      obj->obj_flags.value[3]=object.value[3];
    else obj->obj_flags.value[3]=0;
    obj->obj_flags.extra_flags=object.extra_flags;
    obj->obj_flags.weight     =object.weight;
    obj->obj_flags.bitvector  =object.bitvector;
/* new obj reads */
    obj->obj_flags.type_flag    =object.type_flag;
    obj->obj_flags.wear_flags   =object.wear_flags;
    obj->obj_flags.extra_flags2 =object.extra_flags2;
    obj->obj_flags.subclass_res =object.subclass_res;
    obj->obj_flags.material     =object.material;
    OBJ_SPEC(obj)               =object.spec_value;

    for(i=0;i<MAX_OBJ_AFFECT;i++)
      obj->affected[i]  =object.affected[i];
/* end new obj reads */

/* New ownerid field */
    obj->ownerid[0]             =object.ownerid[0];
    obj->ownerid[1]             =object.ownerid[1];
    obj->ownerid[2]             =object.ownerid[2];
    obj->ownerid[3]             =object.ownerid[3];
    obj->ownerid[4]             =object.ownerid[4];
    obj->ownerid[5]             =object.ownerid[5];
    obj->ownerid[6]             =object.ownerid[6];
    obj->ownerid[7]             =object.ownerid[7];

  }
  else {
    if(object.item_number != 0)
      log_f("BUG: No such item #%d",object.item_number);
  }
  return obj;
}

struct obj_data *dlist_to_obj_ver1(FILE *fl,int fail) {
  struct obj_file_elem_ver1 object;
  int i;
  struct obj_data *obj=NULL;

  memset(&object,0,sizeof (object));
  fail=0;
  if( (fread(&object,sizeof(object),1,fl))!=1) {
    fail=1;
    return obj;
  }

  if(real_object(object.item_number)>-1) {
       obj=read_object(object.item_number,VIRTUAL);
       obj->obj_flags.timer      =object.timer;
       obj->obj_flags.value[0]   =object.value[0];
       obj->obj_flags.value[1]   =object.value[1];
       obj->obj_flags.value[2]   =object.value[2];
    if(obj->obj_flags.type_flag != ITEM_CONTAINER)
      obj->obj_flags.value[3]=object.value[3];
    else obj->obj_flags.value[3]=0;
    obj->obj_flags.extra_flags=object.extra_flags;
    obj->obj_flags.weight     =object.weight;
    obj->obj_flags.bitvector  =object.bitvector;
/* new obj reads */
    obj->obj_flags.type_flag    =object.type_flag;
    obj->obj_flags.wear_flags   =object.wear_flags;
    obj->obj_flags.extra_flags2 =object.extra_flags2;
    obj->obj_flags.subclass_res =object.subclass_res;
    obj->obj_flags.material     =object.material;
    OBJ_SPEC(obj)               =object.spec_value;

    for(i=0;i<MAX_OBJ_AFFECT;i++)
      obj->affected[i]  =object.affected[i];
/* end new obj reads */

  }
  else {
    if(object.item_number != 0)
      log_f("BUG: No such item #%d",object.item_number);
  }
  return obj;
}

struct obj_data *dlist_to_obj_ver0(FILE *fl,int fail) {
  struct obj_file_elem_ver0 object;
  int i;
  struct obj_data *obj=NULL;

  memset(&object,0,sizeof (object));
  fail=0;
  if( (fread(&object,sizeof(object),1,fl))!=1) {
    fail=1;
    return obj;
  }

  if(real_object(object.item_number)>-1) {
    obj=read_object(object.item_number,VIRTUAL);
    obj->obj_flags.timer      =object.timer;
    obj->obj_flags.value[0]   =object.value[0];
    obj->obj_flags.value[1]   =object.value[1];
    obj->obj_flags.value[2]   =object.value[2];
    if(obj->obj_flags.type_flag != ITEM_CONTAINER) obj->obj_flags.value[3]=object.value[3];
    else obj->obj_flags.value[3]=0;
    obj->obj_flags.extra_flags=object.extra_flags;
    obj->obj_flags.weight     =object.weight;
    obj->obj_flags.bitvector  =object.bitvector;
    for(i=0;i<OFILE_MAX_OBJ_AFFECT;i++) {
      obj->affected[i].location = (int)object.affected[i].location;
      obj->affected[i].modifier = (int)object.affected[i].modifier;
    }
  }
  else {
    if(object.item_number != 0)
      log_f("BUG: No such item #%d",object.item_number);
  }
  return obj;
}

void read_dlist(CHAR *ch, char *argument, int cmd) {
  FILE *fl;
  struct string_block sb;
  struct death_file_u dfile;
  struct obj_data *obj;
  char usage_text[] = "\
Usage: `kdlist`q <name> <death #> or.\n\r\
       `kdlist`q # for last # of deaths.\n\r";
  char buf[MAX_INPUT_LENGTH],buf2[MAX_INPUT_LENGTH];
  char name[30],deaths[50];
  int num,dflag,fail=0,found=0;
  char *tmstr;

  if(!check_god_access(ch,FALSE)) return;

  if((GET_LEVEL(ch)<LEVEL_WIZ) && !IS_SET(ch->new.imm_flags,WIZ_JUDGE)) {
    send_to_char("`iYou need a Judge flag to use this command.`q\n\r",ch);
    return;
  }

  argument_interpreter(argument, name, deaths);
  if (!*name) {
    send_to_char(usage_text,ch);
    return;
  }
  if(*deaths) {
    if(!isdigit(*deaths)){
      send_to_char(usage_text,ch);
      return;
    }
  }

  if(is_number(name)) {
    num=atoi(name);
    if(num<1) {
      send_to_char(usage_text,ch);
      return;
    }
    if(num>30) {
      send_to_char("Currently limiting to 30 deaths.\n\r",ch);
      return;
    }
    list_last_deaths(ch,num);
    return;
  }


  string_to_lower(name);
  sprintf(buf,"rent/death/%s.lst",name);
  if (!(fl= fopen(buf, "rb"))) {
       sprintf(buf,"`iNo deaths recorded for `k%s`i.`q\n\r",CAP(name));
       send_to_char(buf,ch);
       return;
  }

  init_string_block(&sb);
  sprintf(buf,"`iDeathlist for `k%s`i:\n\r---------------------------------------------------`q\n\r",CAP(name));
  append_to_string_block(&sb, buf);

  if (!*deaths) {
    while(!feof(fl)) {
      dflag=death_flag_read(fl);
      switch(dflag) {
        case 1:
          if((fread(&dfile,sizeof(dfile),1,fl))!=1) {
            send_to_char("Error reading dlist file(dfile).\n\r",ch);
            page_string_block(&sb,ch);
            destroy_string_block(&sb);
            fclose(fl);
            return;
          }
          tmstr = asctime(localtime(&dfile.time_death));
          *(tmstr + strlen(tmstr) - 1) = '\0';
          if(real_room(dfile.location)!=-1) strncpy(buf2,world[real_room(dfile.location)].name,sizeof(buf2));
          else strncpy(buf2,"Unknown",sizeof(buf2));
          sprintf(buf,"%d) : Location: %s (%d): Time: %s\n\r",
                  dfile.number,buf2,dfile.location,tmstr);
          append_to_string_block(&sb, buf);
          break;
        case 2:
          obj=dlist_to_obj_ver0(fl,fail);
          if(fail) {
            send_to_char("Error reading dlist file(obj).\n\r",ch);
            page_string_block(&sb,ch);
            destroy_string_block(&sb);
            fclose(fl);
            return;
          }
          if(obj) extract_obj(obj);
          break;
        case 3:
          obj=dlist_to_obj_ver1(fl,fail);
          if(fail) {
            send_to_char("Error reading dlist file(obj).\n\r",ch);
            page_string_block(&sb,ch);
            destroy_string_block(&sb);
            fclose(fl);
            return;
          }
          if(obj) extract_obj(obj);
          break;
        case 4:
          obj=dlist_to_obj_ver2(fl,fail);
          if(fail) {
            send_to_char("Error reading dlist file(obj).\n\r",ch);
            page_string_block(&sb,ch);
            destroy_string_block(&sb);
            fclose(fl);
            return;
          }
          if(obj) extract_obj(obj);
          break;
        case 5:
          obj=dlist_to_obj_ver3(fl,fail);
          if(fail) {
            send_to_char("Error reading dlist file(obj).\n\r",ch);
            page_string_block(&sb,ch);
            destroy_string_block(&sb);
            fclose(fl);
            return;
          }
          if(obj) extract_obj(obj);
          break;
        case 9:
          break;
      }
    }
    append_to_string_block(&sb,"`i---------------------------------------------------`q\n\r");
    page_string_block(&sb,ch);
    destroy_string_block(&sb);
    fclose(fl);
    return;
  }

  num = atoi(deaths);
  while(!feof(fl)) {
    dflag=death_flag_read(fl);
    switch(dflag) {
      case 1:
        if((fread(&dfile,sizeof(dfile),1,fl))!=1) {
          send_to_char("Error reading dlist file(dfile).\n\r",ch);
          page_string_block(&sb,ch);
          destroy_string_block(&sb);
          fclose(fl);
          return;
        }
        if(num==dfile.number) {
          found=TRUE;
          tmstr = asctime(localtime(&dfile.time_death));
          *(tmstr + strlen(tmstr) - 1) = '\0';
          if(real_room(dfile.location)!=-1) strncpy(buf2,world[real_room(dfile.location)].name,sizeof(buf2));
          else strncpy(buf2,"Unknown",sizeof(buf2));
          sprintf(buf,"%d) : Location: %s (%d): Time: %s\n\r",
                  dfile.number,buf2,dfile.location,tmstr);
          append_to_string_block(&sb, buf);
          sprintf(buf,"Level: %d Gold: %d Exp: %d Hp: %d Mana: %d Move: %d\n\r",
                  dfile.level,dfile.gold,dfile.exp,dfile.hp,dfile.mana,dfile.move);
          append_to_string_block(&sb, buf);
          sprintf(buf,"Str: %d/%d Int: %d Wis: %d Dex: %d Con: %d\n\r",
                  dfile.str,dfile.add,dfile.intel,dfile.wis,dfile.dex,dfile.con);
          append_to_string_block(&sb, buf);
          append_to_string_block(&sb,"\n\rInventory:\n\r");
        }
        break;
      case 2:
        obj=dlist_to_obj_ver0(fl,fail);
        if(fail) {
          send_to_char("Error reading dlist file(obj).\n\r",ch);
          page_string_block(&sb,ch);
          destroy_string_block(&sb);
          fclose(fl);
          return;
        }
        if(obj) {
          if(num==dfile.number) {
            sprintf(buf," %d - %s\n\r",V_OBJ(obj),OBJ_SHORT(obj));
            append_to_string_block(&sb, buf);
          }
          extract_obj(obj);
        }
        break;
      case 3:
        obj=dlist_to_obj_ver1(fl,fail);
        if(fail) {
          send_to_char("Error reading dlist file(obj).\n\r",ch);
          page_string_block(&sb,ch);
          destroy_string_block(&sb);
          fclose(fl);
          return;
        }
        if(obj) {
          if(num==dfile.number) {
            sprintf(buf," %d - %s\n\r",V_OBJ(obj),OBJ_SHORT(obj));
            append_to_string_block(&sb, buf);
          }
          extract_obj(obj);
        }
        break;
      case 4:
        obj=dlist_to_obj_ver2(fl,fail);
        if(fail) {
          send_to_char("Error reading dlist file(obj).\n\r",ch);
          page_string_block(&sb,ch);
          destroy_string_block(&sb);
          fclose(fl);
          return;
        }
        if(obj) {
          if(num==dfile.number) {
            sprintf(buf," %d - %s\n\r",V_OBJ(obj),OBJ_SHORT(obj));
            append_to_string_block(&sb, buf);
          }
          extract_obj(obj);
        }
        break;
      case 5:
        obj=dlist_to_obj_ver3(fl,fail);
        if(fail) {
          send_to_char("Error reading dlist file(obj).\n\r",ch);
          page_string_block(&sb,ch);
          destroy_string_block(&sb);
          fclose(fl);
          return;
        }
        if(obj) {
          if(num==dfile.number) {
            sprintf(buf," %d - %s\n\r",V_OBJ(obj),OBJ_SHORT(obj));
            append_to_string_block(&sb, buf);
          }
          extract_obj(obj);
        }
        break;
      case 9:
        break;
    }
  }

  append_to_string_block(&sb,"`i---------------------------------------------------`q\n\r");
  if(found) page_string_block(&sb,ch);
  else {
    sprintf(buf,"`iNo death `k#%d `ilogged for `k%s`i.`q\n\r",num,CAP(name));
    send_to_char(buf,ch);
  }
  destroy_string_block(&sb);
  fclose(fl);
  return;
}

void do_reimb(CHAR *ch, char *argument, int cmd) {
  FILE *fl;
  struct obj_data *obj;
  struct death_file_u dfile;
  char usage_text[] = "Usage: reimb <name> <death #>.\n\r";
  char buf[MAX_INPUT_LENGTH],buf2[MAX_INPUT_LENGTH];
  char name[30],deaths[50];
  int num,dflag,fail=0,found=0;

  if(!check_god_access(ch,TRUE)) return;

  if((GET_LEVEL(ch)<LEVEL_SUP) && !IS_SET(ch->new.imm_flags,WIZ_JUDGE)){
    send_to_char("You need a Judge flag to use this command.\n\r",ch);
    return;
  }

  argument_interpreter(argument, name, deaths);
  if (!*name) {
    send_to_char(usage_text,ch);
        return;
  }
  if(!*deaths) {
    read_dlist(ch,argument,cmd);
    return;
  }
  if(!isdigit(*deaths)) {
    send_to_char(usage_text,ch);
        return;
  }

  string_to_lower(name);
  sprintf(buf,"rent/death/%s.lst",name);
  if (!(fl= fopen(buf, "rb"))) {
    sprintf(buf,"No deaths recorded for %s.\n\r",CAP(name));
    send_to_char(buf,ch);
    return;
  }

  num = atoi(deaths);

  while(!feof(fl)) {
    dflag=death_flag_read(fl);
    switch(dflag) {
      case 1:
        if((fread(&dfile,sizeof(dfile),1,fl))!=1) {
          send_to_char("Error reading dlist file(dfile).\n\r",ch);
          fclose(fl);
          return;
        }
        if(num==dfile.number) {
          found=TRUE;
          sprintf(buf2,"Level: %d Gold: %d Exp: %d Hp: %d Mana: %d Move: %d\n\r",
                  dfile.level,dfile.gold,dfile.exp,dfile.hp,dfile.mana,dfile.move);
        }
        break;
      case 2:
        obj=dlist_to_obj_ver0(fl,fail);
        if(fail) {
          send_to_char("Error reading dlist file(obj).\n\r",ch);
          fclose(fl);
          return;
        }
        if(obj) {
          if(num==dfile.number) {
            if (obj->obj_flags.type_flag == ITEM_TICKET) {
              if(!adjust_ticket_strings(obj)) return;
            }
            obj_to_char(obj,ch);
            act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
            act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
            act("You have created $p!", FALSE, ch, obj, 0, TO_CHAR);
               sprintf(buf, "WIZINFO: (%s) load obj (#%d) %d (%s)",
                    GET_NAME(ch), world[CHAR_REAL_ROOM(ch)].number, V_OBJ(obj), OBJ_SHORT(obj));
            log_s(buf);
          }
          else {
            extract_obj(obj);
          }
        }
        break;
      case 3:
        obj=dlist_to_obj_ver1(fl,fail);
        if(fail) {
          send_to_char("Error reading dlist file(obj).\n\r",ch);
          fclose(fl);
          return;
        }
        if(obj) {
          if(num==dfile.number) {
            if (obj->obj_flags.type_flag == ITEM_TICKET) {
              if(!adjust_ticket_strings(obj)) return;
            }
            obj_to_char(obj,ch);
            act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
            act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
            act("You have created $p!", FALSE, ch, obj, 0, TO_CHAR);
               sprintf(buf, "WIZINFO: (%s) load obj (#%d) %d (%s)",
                    GET_NAME(ch), world[CHAR_REAL_ROOM(ch)].number, V_OBJ(obj), OBJ_SHORT(obj));
            log_s(buf);
          }
          else {
            extract_obj(obj);
          }
        }
        break;
      case 4:
        obj=dlist_to_obj_ver2(fl,fail);
        if(fail) {
          send_to_char("Error reading dlist file(obj).\n\r",ch);
          fclose(fl);
          return;
        }
        if(obj) {
          if(num==dfile.number) {
            if (obj->obj_flags.type_flag == ITEM_TICKET) {
              if(!adjust_ticket_strings(obj)) return;
            }
            obj_to_char(obj,ch);
            act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
            act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
            act("You have created $p!", FALSE, ch, obj, 0, TO_CHAR);
               sprintf(buf, "WIZINFO: (%s) load obj (#%d) %d (%s)",
                    GET_NAME(ch), world[CHAR_REAL_ROOM(ch)].number, V_OBJ(obj), OBJ_SHORT(obj));
            log_s(buf);
          }
          else {
            extract_obj(obj);
          }
        }
        break;
      case 5:
        obj=dlist_to_obj_ver3(fl,fail);
        if(fail) {
          send_to_char("Error reading dlist file(obj).\n\r",ch);
          fclose(fl);
          return;
        }
        if(obj) {
          if(num==dfile.number) {
            if (obj->obj_flags.type_flag == ITEM_TICKET) {
              if(!adjust_ticket_strings(obj)) return;
            }
            obj_to_char(obj,ch);
            act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
            act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
            act("You have created $p!", FALSE, ch, obj, 0, TO_CHAR);
               sprintf(buf, "WIZINFO: (%s) load obj (#%d) %d (%s)",
                    GET_NAME(ch), world[CHAR_REAL_ROOM(ch)].number, V_OBJ(obj), OBJ_SHORT(obj));
            log_s(buf);
          }
          else {
            extract_obj(obj);
          }
        }
        break;
      case 9:
        break;
    }
  }

  if(found) {
    send_to_char("All items loaded.\n\r",ch);
    send_to_char(buf2,ch);
  }
  else {
    sprintf(buf,"No death #%d logged for %s.\n\r",num,CAP(name));
    send_to_char(buf,ch);
  }
  fclose(fl);
}

/*
** Procedure rshow written for Ronin by Ranger
**
** Allows a search for rooms by text
** Please do not distribute
**
** May 6, 1996  Last Modified Jan 26/97
*/

void do_rshow(struct char_data *ch, char *argument, int cmd) {

  char buf2[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];
  struct string_block sb;
  char sects[MAX_INPUT_LENGTH],flags[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
  int i, found = FALSE,flag_check=FALSE,tmp;
  char usage_text[] = "Usage: rshow <text> or rshow flag <flag>. Text must be at least 3 chars.\n\r";

  if(!check_god_access(ch,FALSE)) return;

  half_chop(argument, arg,MIL,arg2,MIL);

  if(!*arg) {
    send_to_char(usage_text, ch);
    return;
  }

  if(is_abbrev(arg, "flag")) flag_check=TRUE;
  else sprintf(arg2,"%s",arg);

  if(strlen(arg2)<3) {
    send_to_char(usage_text, ch);
    return;
  }

  init_string_block(&sb);

  if(flag_check) {

   tmp = old_search_block(arg2, 0, strlen(arg2), room_bits, FALSE);
   if(tmp==-1) {
     send_to_char("Not a valid flag.\n\r",ch);
     return;
   }
   tmp=1<<(tmp-1);
   for( i = 0; i <= top_of_world; i++) {
    if (IS_SET(world[i].room_flags, tmp)) {
      sprintf(buf2, "[%d] %s :: ", world[i].number, world[i].name);

      sprintbit((long) world[i].room_flags,room_bits,flags);
      strcat(buf2,flags);
      sprinttype(world[i].sector_type,sector_types,sects);
      strcat(buf2,sects);
      strcat(buf2,"\n\r");
      append_to_string_block(&sb, buf2);
      found = TRUE;
    }
   }
  }
  else {
   for( i = 0; i <= top_of_world; i++) {
    if(isname(arg2, world[i].name)) {
      sprintf(buf2, "[%d] %s :: ", world[i].number, world[i].name);

      sprintbit((long) world[i].room_flags,room_bits,flags);
      strcat(buf2,flags);
      strcat(buf2,"\n\r");
      append_to_string_block(&sb, buf2);
      found = TRUE;
    }
   }
  }

  if(found) page_string_block(&sb,ch);
  else {
    sprintf(buf2, "Nothing at all found with name: %s.\n\r", arg);
    send_to_char(buf2, ch);
  }
  destroy_string_block(&sb);
}

void do_zbrief(struct char_data *ch, char *argument, int cmd) {
  struct string_block sb;
  int i,x,zone,rzone;
  long int tmp1,tmp2;
  char buf[MAX_INPUT_LENGTH],buf2[MAX_INPUT_LENGTH], buf3[MAX_INPUT_LENGTH],buf4[MAX_INPUT_LENGTH];
  char num[100],type[100];
  struct extra_descr_data *desc;
  char usage_text[] = "Usage: zbrief <zone #> <mobile/object/room> <all mobile> (IMP Only).\n\r";
  char fu[]=" ",sa[]=" ",sp[]=" ",sh[]=" ",in[]=" ",du[]=" ",pe[]=" ",al[]=" ",dg[]=" ",ag[]=" ";
  FILE *fl;
  if(!check_god_access(ch,FALSE)) return;

  argument_interpreter(argument,num,type);

  if(!*type) {
    send_to_char(usage_text,ch);
    return;
  }

  if(!strcmp(num,"all") && is_abbrev(type, "mobile") && GET_LEVEL(ch)==LEVEL_IMP) {
    fl = fopen("allmobs.txt", "w");
    if(fl==0) {
      send_to_char("Unable to open file!!\n\r",ch);
      return;
    }

    fprintf(fl,"Vnum Name Level ExpPerHr NumMob Hps Dam Exp Gold Loads AC\n\r");
    for( i = 0; i <= top_of_mobt; i++) {
      if(mob_proto_table[i].exp==0) continue;
      tmp1=(mob_proto_table[i].hp_nodice+mob_proto_table[i].hp_nodice*
            mob_proto_table[i].hp_sizedice)/2+mob_proto_table[i].hp_add;
      tmp2=(mob_proto_table[i].damnodice+mob_proto_table[i].damnodice*
            mob_proto_table[i].damsizedice)/2+mob_proto_table[i].damroll;

      fprintf(fl, "%d \"%s\" %d %ld %ld %ld %ld %d %d %d %d\n\r",
           mob_proto_table[i].virtual,mob_proto_table[i].name,mob_proto_table[i].level,
           exp_per_hour(i)/3,exp_per_hour(i)*3/mob_proto_table[i].exp,
           tmp1,tmp2,mob_proto_table[i].exp/3,mob_proto_table[i].gold,mob_proto_table[i].loads,
           mob_proto_table[i].armor);
    }
    fclose(fl);
    send_to_char("allmobs.txt file written to lib directory.\n\r",ch);
    return;
  }

  for (i = 0; i < strlen(num); i++) {
    if (!isdigit(*num)) {
      send_to_char("Zone value is not a valid number.\n\r", ch);
      send_to_char(usage_text,ch);
      return;
    }
  }

  zone = atoi(num);
  rzone=real_zone(zone);
  if(rzone==-1) {
    send_to_char("That zone doesn't exist.\n\r",ch);
    return;
  }

  if(GET_LEVEL(ch) < LEVEL_SUP && !isname(GET_NAME(ch), zone_table[real_zone(zone)].name) && !strstr(zone_table[real_zone(zone)].creators,GET_NAME(ch))) {
    send_to_char("You are not allowed view a zone without your name on it.\r\n",ch);
    return;
  }

  init_string_block(&sb);

  if(is_abbrev(type, "mobile")) {
    append_to_string_block(&sb,"VNUM   HPS    C.EXP   GOLD THAC0 AC   DMG Lvl Al Fu Du Sa Sp Sh Pe In Do Ag exp/hr\n\r---------------------------------------------------------------------------\n\r");
    for( i = 0; i <= top_of_mobt; i++) {
      if(zone == inzone(mob_proto_table[i].virtual)) {
        tmp1=(mob_proto_table[i].hp_nodice+
              mob_proto_table[i].hp_nodice*
              mob_proto_table[i].hp_sizedice)/2+
              mob_proto_table[i].hp_add;
        tmp2=(mob_proto_table[i].damnodice+
              mob_proto_table[i].damnodice*
              mob_proto_table[i].damsizedice)/2+
              mob_proto_table[i].damroll;

        if(IS_SET(mob_proto_table[i].affected_by,AFF_FURY))
          sprintf(fu,"Y");
        else sprintf(fu," ");
        if(IS_SET(mob_proto_table[i].affected_by,AFF_DUAL))
          sprintf(du,"Y");
        else sprintf(du," ");
        if(IS_SET(mob_proto_table[i].affected_by,AFF_SANCTUARY))
          sprintf(sa,"Y");
        else sprintf(du," ");
        if(IS_SET(mob_proto_table[i].affected_by,AFF_DODGE))
          sprintf(dg,"Y");
        else sprintf(dg," ");
        if(IS_SET(mob_proto_table[i].affected_by,AFF_SPHERE) ||
           IS_SET(mob_proto_table[i].affected_by,AFF_INVUL))
          sprintf(sp,"Y");
        else sprintf(sp," ");
        if(IS_SET(mob_proto_table[i].act,ACT_SHIELD))
          sprintf(sh,"Y");
        else sprintf(sh," ");
        if(IS_SET(mob_proto_table[i].affected_by,AFF_PROTECT_EVIL) ||
           IS_SET(mob_proto_table[i].affected_by,AFF_PROTECT_GOOD))
          sprintf(pe,"Y");
        else sprintf(pe," ");
        if(IS_SET(mob_proto_table[i].affected_by,AFF_IMINV) ||
           IS_SET(mob_proto_table[i].affected_by,AFF_HIDE))
          sprintf(in,"Y");
        else sprintf(in," ");
        sprintf(al,"N");
        if(mob_proto_table[i].alignment>349)
          sprintf(al,"G");
        if(mob_proto_table[i].alignment<-349)
          sprintf(al,"E");
        if(IS_SET(mob_proto_table[i].act,ACT_AGGRESSIVE) ||
           IS_SET(mob_proto_table[i].act,ACT_AGGWA) ||
           IS_SET(mob_proto_table[i].act,ACT_AGGCL) ||
           IS_SET(mob_proto_table[i].act,ACT_AGGMU) ||
           IS_SET(mob_proto_table[i].act,ACT_AGGNI) ||
           IS_SET(mob_proto_table[i].act,ACT_AGGNO) ||
           IS_SET(mob_proto_table[i].act,ACT_AGGPA) ||
           IS_SET(mob_proto_table[i].act,ACT_AGGAP) ||
           IS_SET(mob_proto_table[i].act,ACT_AGGCO) ||
           IS_SET(mob_proto_table[i].act,ACT_AGGTH) ||
           IS_SET(mob_proto_table[i].act,ACT_AGGBA) ||
           IS_SET(mob_proto_table[i].act,ACT_AGGGOOD) ||
           IS_SET(mob_proto_table[i].act,ACT_AGGNEUT) ||
           IS_SET(mob_proto_table[i].act,ACT_AGGEVIL) ||
           IS_SET(mob_proto_table[i].act,ACT_AGGLEADER) ||
           IS_SET(mob_proto_table[i].act,ACT_AGGRANDOM))
          sprintf(ag,"Y");
        else sprintf(ag," ");

        sprintf(buf2, "#%-5d %-5ld %7d %7d %-2d %-5d %-3ld %3d  %s  %s  %s  %s  %s  %s  %s  %s  %s %s %ld\n\r",
           mob_proto_table[i].virtual,tmp1,mob_proto_table[i].exp/3,
           mob_proto_table[i].gold,mob_proto_table[i].hitroll,
           mob_proto_table[i].armor,tmp2,mob_proto_table[i].level,
           al,fu,du,sa,sp,sh,pe,in,dg,ag,exp_per_hour(i)/3);
        append_to_string_block(&sb, buf2);
      }
    }
    page_string_block(&sb,ch);
    destroy_string_block(&sb);
    return;
  } /* End of Mobile */

  if(is_abbrev(type, "object")) {
    append_to_string_block(&sb,"#V  TYPE         VALUES              Bv Ma  Affects\n\r---------------------------------------------------------------------------\n\r");
    for( i = 0; i <= top_of_objt; i++) {
      if (zone == inzone(obj_proto_table[i].virtual)) {
     sprintf(fu,"N");
     sprintf(du,"N");
     sprintf(sa,"N");
     sprintf(sp,"N");
     sprintf(sh,"N");
     sprintf(pe,"N");
     sprintf(in,"N");
        sprintf(buf3," ");
        for(x = 0; x < MAX_OBJ_AFFECT; x++) {
          sprinttype(obj_proto_table[i].affected[x].location, apply_types, buf2);
          if(strstr(buf2,"SKILL")) {
         str_cut(buf2,buf4,6);
         strncpy(buf2,buf4,sizeof(buf2));
         }
          if(strstr(buf2,"SAVING")) {
         str_cut(buf2,buf4,7);
         strncpy(buf2,buf4,sizeof(buf2));
          }
          sprintf(buf, "%-9s %3d  ", buf2, obj_proto_table[i].affected[x].modifier);
          strcat(buf3, buf);
        }
     sprintf(buf," ");
     strncat(buf,item_types[obj_proto_table[i].obj_flags.type_flag],9);
     if(obj_proto_table[i].obj_flags.bitvector!=0) sprintf(fu,"Y");
     if(IS_SET(obj_proto_table[i].obj_flags.extra_flags, ITEM_MAGIC)) sprintf(sa,"Y");
        sprintf(buf2, "#%-2d%-10s  %4d %4d %4d %4d   %s  %s  %s\n\r",
       obj_proto_table[i].virtual-((obj_proto_table[i].virtual/100)*100),
       buf, obj_proto_table[i].obj_flags.value[0],
       obj_proto_table[i].obj_flags.value[1], obj_proto_table[i].obj_flags.value[2],
       obj_proto_table[i].obj_flags.value[3], fu,sa,buf3);
        append_to_string_block(&sb, buf2);
      }
    }
    page_string_block(&sb,ch);
    destroy_string_block(&sb);
    return;
  }/* End of Object */

  if(is_abbrev(type, "room")) {
    for( i = 0; i <= top_of_world; i++) {
      if (zone == inzone(world[i].number)) {
        sprintf(buf2, "[%d] %s :: ", world[i].number, world[i].name);
        if (world[i].ex_description) {
          for(desc = world[i].ex_description; desc; desc = desc->next) {
            strcat(buf2, desc->keyword);
            strcat(buf2," ");
          }
        }
        strcat(buf2, "\n\r");
        append_to_string_block(&sb, buf2);
      }
    }
    page_string_block(&sb,ch);
    destroy_string_block(&sb);
    return;
  } /* End of room */
}


/****************************************************************
* Procedure zshow written for Ronin by Ranger                *
*                                        *
* Allows a list of things in a zone                    *
* Please do not distribute                         *
*                                             *
*****************************************************************/

void do_zshow(struct char_data *ch, char *argument, int cmd) {

  int i,zone,rzone,found = FALSE;
  char buf2[MAX_INPUT_LENGTH],flags[MAX_INPUT_LENGTH];
  struct string_block sb;
  char num[100],type[100];
  char usage_text[] = "Usage: zshow <zone #> <mobile/object/room/shop>.\n\r";
  bool show_repop=FALSE;

  if(!check_god_access(ch,FALSE)) return;

  argument_interpreter(argument,num,type);

  if(!*type) {
    send_to_char(usage_text,ch);
    return;
  }

  for (i = 0; i < strlen(num); i++) {
    if (!isdigit(*num)) {
      send_to_char("Zone value is not a valid number.\n\r", ch);
      send_to_char(usage_text,ch);
      return;
    }
  }

  zone = atoi(num);
  rzone=real_zone(zone);
  if(rzone==-1) {
    send_to_char("That zone doesn't exist.\n\r",ch);
    return;
  }
  if(GET_LEVEL(ch) < LEVEL_SUP && !isname(GET_NAME(ch), zone_table[real_zone(zone)].name) && !strstr(zone_table[real_zone(zone)].creators,GET_NAME(ch))) {
    send_to_char("You are not allowed view a zone without your name on it.\r\n",ch);
    return;
  }

  init_string_block(&sb);

  if(is_abbrev(type, "room")) {
    for( i = 0; i <= top_of_world; i++) {
      if (zone == inzone(world[i].number)) {
        sprintf(buf2, "[%d] %s :: ", world[i].number, world[i].name);

        sprintbit((long) world[i].room_flags,room_bits,flags);
        strcat(buf2,flags);
        strcat(buf2," (");
        sprinttype(world[i].sector_type,sector_types,flags);
        strcat(buf2,flags);
        strcat(buf2,")\n\r");
        append_to_string_block(&sb, buf2);
        found = TRUE;
      }
    }

    if(found) page_string_block(&sb,ch);
    else {
      sprintf(buf2, "Nothing found for zone: %d.\n\r", zone);
      send_to_char(buf2, ch);
    }
    destroy_string_block(&sb);
    return;
  } /* End of room */

  if(is_abbrev(type, "mobile")) {
    for( i = 0; i <= top_of_mobt; i++) {
      if (zone == inzone(mob_proto_table[i].virtual)) {
        sprintf(buf2, "[%-5d] %s (%s) - %s\n\r", mob_proto_table[i].virtual,
                mob_proto_table[i].short_descr,mob_proto_table[i].name,
                npc_class_types[(int)mob_proto_table[i].class]);
        append_to_string_block(&sb, buf2);
        found = TRUE;
      }
    }

    if(found) page_string_block(&sb,ch);
    else {
      sprintf(buf2, "Nothing found for zone: %d.\n\r", zone);
      send_to_char(buf2, ch);
    }
    destroy_string_block(&sb);
    return;
  } /* End of mobile */

  if(is_abbrev(type, "object")) {
    if((GET_LEVEL(ch) > LEVEL_SUP) || isname(GET_NAME(ch), zone_table[real_zone(zone)].name) || strstr(zone_table[real_zone(zone)].creators,GET_NAME(ch))) show_repop=TRUE;
    for( i = 0; i <= top_of_objt; i++) {
      if (zone == inzone(obj_proto_table[i].virtual)) {
        if(show_repop) {
          sprintf(buf2, "[%-5d] %s (%s) %s Repop: %d\n\r",
          obj_proto_table[i].virtual,
          obj_proto_table[i].short_description,
       obj_proto_table[i].name,
          item_types[obj_proto_table[i].obj_flags.type_flag],
          obj_proto_table[i].obj_flags.repop_percent);
        }
        else {
          sprintf(buf2, "[%-5d] %s (%s) %s\n\r",
       obj_proto_table[i].virtual,
          obj_proto_table[i].short_description,
       obj_proto_table[i].name,
          item_types[obj_proto_table[i].obj_flags.type_flag]);
        }
        append_to_string_block(&sb, buf2);
        found = TRUE;
      }
    }

    if(found) page_string_block(&sb,ch);
    else {
      sprintf(buf2, "Nothing found for zone: %d.\n\r", zone);
      send_to_char(buf2, ch);
    }
    destroy_string_block(&sb);
    return;
  } /* End of object */


  if(is_abbrev(type, "shop")) {
    for( i = 0; i <number_of_shops; i++) {
      if (zone == inzone(shop_index[i].keeper)) {
        sprintf(buf2, "Keeper: %5d  Room: %5d\n\r", shop_index[i].keeper, shop_index[i].in_room);
        append_to_string_block(&sb, buf2);
        found = TRUE;
      }
    }

    if(found) page_string_block(&sb,ch);
    else {
      sprintf(buf2, "Nothing found for zone: %d.\n\r", zone);
      send_to_char(buf2, ch);
    }
    destroy_string_block(&sb);
    return;
  } /* End of shop */
}

/*
* Procedure zlist written for Ronin by Ranger
*
* List all loaded zones
* Please do not distribute
*
* June 11, 1996
*
* Last modified Nov 28, 1997 (dynamic memory string added)
*/

void do_zlist(struct char_data *ch, char *argument, int cmd) {
  int i,found=FALSE;
  char buf[MAX_INPUT_LENGTH],arg[MAX_INPUT_LENGTH];
  struct string_block sb;

  if(!check_god_access(ch,FALSE)) return;

  init_string_block(&sb);

  argument = one_argument(argument, arg);

  if(*arg) {
    for( i = 0; i <= top_of_zone_table; i++) {
      strcpy(buf,zone_table[i].name);
      string_to_lower(buf);
      if(isname(arg, buf)) {
        found=TRUE;
        sprintf(buf, "[%d] %s\n\r", zone_table[i].virtual, zone_table[i].name);
        append_to_string_block(&sb, buf);
      }
    }
  } else {
    found=TRUE;
    for( i = 0; i <= top_of_zone_table; i++) {
      sprintf(buf, "[%d] %s\n\r", zone_table[i].virtual, zone_table[i].name);
      append_to_string_block(&sb, buf);
    }
  }

  if(found) page_string_block(&sb,ch);
  else send_to_char("Nothing found.\n\r",ch);
  destroy_string_block(&sb);
}

/*
** do_core - Ranger Jan 19, 1999
*/
void do_core(struct char_data *ch, char *argument, int cmd) {
  char buf[MAX_INPUT_LENGTH],arg[MAX_INPUT_LENGTH];
  char usage[]={"\
This command is for checking the core of a mud crash.\n\r\n\r\
  Usage: `kcore`q <update/read>\n\r"};
  FILE *fl;
  struct string_block sb;

  if(!check_god_access(ch,TRUE)) return;

  one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(is_abbrev(arg, "create")) {
    fl = 0;
    *((int*)fl) = 0;
  }

  if(is_abbrev(arg,"update")) {
    if (!(fl= fopen("core","r"))) {
      send_to_char("`iNo core file exists.`q\n\r",ch);
      return;
    }
    fclose(fl);
    system("gdb.scr > gdb.txt &");
    send_to_char("`iCore information updated, check dates on core and exec.`q\n\r",ch);
    return;
  }

  if(is_abbrev(arg,"read")) {
    if (!(fl= fopen("gdb.txt","r"))) {
      send_to_char("`iNo core summary file exists.`q\n\r",ch);
      return;
    }
    init_string_block(&sb);
    append_to_string_block(&sb,"`iCore summary information\n\r------------------------`q\n\r");
    while (!feof(fl)) {
      fgets(buf, 254, fl);
      append_to_string_block(&sb, buf);
    }
    fclose(fl);
    page_string_block(&sb,ch);
    destroy_string_block(&sb);
    return;
  }
  send_to_char(usage,ch);
}

void command_interpreter(CHAR *ch, char *argument);
void do_doas(struct char_data *ch, char *argument, int cmd)
{
  CHAR *vict;
  struct descriptor_data *orig;
  char arg[MAX_INPUT_LENGTH];

  if(IS_NPC(ch)) return;
  if(!check_god_access(ch,TRUE)) return;

  argument=one_argument(argument,arg);
  if(!*arg) {
    send_to_char("Usage: `kdoas`q <victim> <command>\n\r       Some commands are blocked and the victim must normally be able to do the command.\n\r",ch);
    return;
  }
  if (ch->desc->snoop.snooping) {
    send_to_char("You can't use this command while snooping.\n\r",ch);
    return;
  }
  if(!(vict = get_char_vis(ch, arg))) {
    send_to_char("Victim not found.\n\r",ch);
    return;
  }

  if(!IS_NPC(vict) && GET_LEVEL(vict)>=GET_LEVEL(ch)) {
    send_to_char("This command doesn't work on someone the same or higher level than you.\n\r",ch);
    return;
  }

  if(!*argument) {
    send_to_char("What about a command?\n\r",ch);
    return;
  }
  one_argument(argument,arg);
  if(is_abbrev(arg,"quit") || is_abbrev(arg,"password") || is_abbrev(arg,"doas")) {
    send_to_char("Forbidden command!\n\r",ch);
    return;
  }
  sprintf(arg,"WIZINFO: %s as %s: %s",GET_NAME(ch),GET_NAME(vict),argument);
  wizlog(arg,LEVEL_IMP,5);
  log_s(arg);

  orig = vict->desc;
  vict->desc = ch->desc;
  ch->desc = NULL;

  command_interpreter(vict,argument);
  ch->desc = vict->desc;
  vict->desc = orig;
}

long int exp_per_hour(int i) {
  int j;
  float rounds_per_tank,extra_miracles,extra_rounds,rounds_to_kill,num_of_kills,rounds_plus_rest;
  long int hps,dam,exp,exp_per_hour;

  if(mob_proto_table[i].exp<1) return 0;

  hps=(mob_proto_table[i].hp_nodice+mob_proto_table[i].hp_nodice*
       mob_proto_table[i].hp_sizedice)/2+mob_proto_table[i].hp_add;
  dam=(mob_proto_table[i].damnodice+mob_proto_table[i].damnodice*
       mob_proto_table[i].damsizedice)/2+mob_proto_table[i].damroll;
  exp=mob_proto_table[i].exp/3;

  if(IS_SET(mob_proto_table[i].affected_by,AFF_FURY)) dam*=2;
  if(IS_SET(mob_proto_table[i].affected_by,AFF_DUAL)) dam*=2;
  if(IS_SET(mob_proto_table[i].affected_by,AFF_SANCTUARY)) hps*=2;
  if(IS_SET(mob_proto_table[i].affected_by,AFF_DODGE)) hps+=hps/5; /* Assume 1/5 more effect hps for dodge */
  dam/=2; /* For vict sanct */

  if(dam==0) return -1;

  rounds_per_tank=800/dam; /* tank with 800 hps */
  if(rounds_per_tank>=80) { /* cleric can constantly miracle  4 ticks to regen */
    rounds_to_kill=hps/300;
    rounds_to_kill=MAX(rounds_to_kill,1);
    rounds_to_kill=MIN(rounds_to_kill,1200);
    exp_per_hour=(long)(1200/rounds_to_kill*exp);
    return exp_per_hour;
  }

  rounds_per_tank*=8; /* Cleric has 8 miracles */
  if(rounds_per_tank>=1200) { /* thats at least an hour */
    rounds_to_kill=hps/300;
    rounds_to_kill=MAX(rounds_to_kill,1);
    rounds_to_kill=MIN(rounds_to_kill,1200);
    exp_per_hour=(long)(1200/rounds_to_kill*exp);
    return exp_per_hour;
  }

  extra_miracles=rounds_per_tank/80; /* Cleric needs 80 rounds to regen 1 miracle*/
  extra_rounds=extra_miracles*800/dam; /* More rounds */
  for(j=0;j<15;j++) { /* Even more rounds */
    rounds_per_tank+=extra_rounds;
    extra_miracles=extra_rounds/80;
    extra_rounds=extra_miracles*800/dam;
  }

  if(rounds_per_tank>=1200) { /* thats at least an hour */
    rounds_to_kill=hps/300;
    rounds_to_kill=MAX(rounds_to_kill,1);
    rounds_to_kill=MIN(rounds_to_kill,1200);
    exp_per_hour=(long)(1200/rounds_to_kill*exp);
    return exp_per_hour;
  }

  rounds_to_kill=hps/300;
  rounds_to_kill=MAX(rounds_to_kill,1);
  rounds_to_kill=MIN(rounds_to_kill,1200);
  num_of_kills=rounds_per_tank/rounds_to_kill;
  rounds_plus_rest=200+rounds_per_tank;
  rounds_plus_rest=MIN(rounds_plus_rest,1200);
  exp_per_hour=(long)(1200/(rounds_plus_rest)*num_of_kills*exp);
  exp_per_hour=MAX(exp_per_hour,0);
  return exp_per_hour;
}

void do_deputize(struct char_data *ch, char *argument, int cmd)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR *vict;

  if(!check_god_access(ch,TRUE)) return;
  one_argument(argument,arg);
  if(!*arg) {
    send_to_char("Deputize who?\n\r",ch);
    return;
  }

  if(!(vict=get_char_vis(ch,arg))) {
    send_to_char("That person isn't here.\n\r",ch);
    return;
  }
  if(CHAR_REAL_ROOM(ch)!=CHAR_REAL_ROOM(vict)) {
    send_to_char("That person isn't here.\n\r",ch);
    return;
  }

  if(IS_SET(vict->specials.pflag, PLR_DEPUTY)) {
    send_to_char("Deputy status removed.\n\r",ch);
    send_to_char("Your deputy status has been removed.\n\r",vict);
    REMOVE_BIT(vict->specials.pflag, PLR_DEPUTY);
    sprintf(arg,"WIZINFO: %s removed %s from deputy status.",GET_NAME(ch),GET_NAME(vict));
    wizlog(arg,LEVEL_IMP,5);
    log_s(arg);
    return;
  }
  else {
    send_to_char("Deputy status added.\n\r",ch);
    send_to_char("You are now a Midgaard Deputy!\n\r",vict);
    SET_BIT(vict->specials.pflag, PLR_DEPUTY);
    sprintf(arg,"WIZINFO: %s added %s to deputy status.",GET_NAME(ch),GET_NAME(vict));
    wizlog(arg,LEVEL_IMP,5);
    log_s(arg);
    return;
  }
}

void do_rip(CHAR *ch, char *argument, int cmd) {
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  CHAR *victim;
  OBJ *obj;

  if(!check_god_access(ch,TRUE)) return;

  one_argument(argument,arg);

  if(!*arg) {
    send_to_char("Who are you going to rip in half?\n\r",ch);
    return;
  }

  if(!(victim=get_char_room_vis(ch,arg))) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if(victim == ch) {
    send_to_char("You reach your neck fine, but cant' get your ankles.\n\r",ch);
    return;
  }

  if(IS_SET(victim->specials.affected_by2,AFF2_SEVERED) || !CHAR_HAS_LEGS(victim)) {
    act("$N has no legs to rip off.",0,ch,0,victim,TO_CHAR);
    return;
  }

  log_f("%s ripped by %s.", IS_NPC(victim) ? MOB_SHORT(victim) : GET_NAME(victim), GET_NAME(ch));

  SET_BIT(victim->specials.affected_by2,AFF2_SEVERED);
  if(!IS_NPC(victim))
    act("$n picks you up and rips you in half!",0,ch,0,victim,TO_VICT);
  act("$n picks up $N and rips $S legs off!",0,ch,0,victim,TO_NOTVICT);
  act("You rip $M in half!\n\r",0,ch,0,victim,TO_CHAR);
  if(victim->specials.riding)
    stop_riding(victim,victim->specials.riding);
  GET_HIT(victim)=-1;
  GET_POS(victim)=POSITION_MORTALLYW;

  if (victim->equipment[WEAR_LEGS])
    obj_to_char(unequip_char(victim,WEAR_LEGS),victim);
  if (victim->equipment[WEAR_FEET])
    obj_to_char(unequip_char(victim,WEAR_FEET),victim);


  obj=read_object(19,VIRTUAL);

  if(IS_NPC(victim))
    sprintf(buf,"A pair of %s's legs are here, twitching.",MOB_SHORT(victim));
  else
    sprintf(buf,"A pair of %s's legs are here, twitching.",GET_NAME(victim));
  free(obj->description);
  obj->description = str_dup( buf );
  obj->obj_flags.timer=30;
  SET_BIT(obj->obj_flags.extra_flags2, ITEM_ALL_DECAY);

  if(IS_NPC(victim))
    sprintf(buf,"A pair of %s's legs",MOB_SHORT(victim));
  else
    sprintf(buf,"A pair of %s's legs",GET_NAME(victim));
  free(obj->short_description);
  obj->short_description = str_dup( buf );

  if(IS_NPC(victim))
    sprintf(buf,"$n swings %s's legs around, spraying blood everywhere!",MOB_SHORT(victim));
  else
    sprintf(buf,"$n swings %s's legs around, spraying blood everywhere!",GET_NAME(victim));
  free(obj->action_description);
  obj->action_description = str_dup( buf );

  obj_to_char(obj,ch);
}

void do_freeze(struct char_data *ch, char *argument, int cmd) {
  CHAR *vict;
  OBJ *dummy;
  char buf[MAX_INPUT_LENGTH];

  if(IS_NPC(ch)) return;

  if (!IS_SET(ch->new.imm_flags, WIZ_TRUST)) {
    send_to_char("You need a Trust flag to do that!\n\r", ch);
    return;
  }

  if (IS_SET(ch->new.imm_flags, WIZ_FREEZE) && GET_LEVEL(ch)<LEVEL_IMP) {
    send_to_char("You are Frozen, so you can't do it!\n\r", ch);
    return;
  }

  if(CHAOSMODE && !IS_SET(ch->new.imm_flags, WIZ_CHAOS)) {
    send_to_char("This is chaos! You aren't permitted to do very much.\n\r",ch);
    return;
  }

  if (!IS_SET(ch->new.imm_flags, WIZ_ACTIVE)) {
    send_to_char("You need an active flag for this command.\n\r", ch);
    return;
  }

  one_argument(argument, buf);

  if (!*buf) {
    send_to_char("\
Usage: `kfreeze`q <victim>\n\r\
Freeze has been extended to totally freeze mortals.\n\r", ch);
    return;
  }

  if(!generic_find(buf, FIND_CHAR_WORLD, ch, &vict, &dummy)) {
    send_to_char("Couldn't find anyone by that name.\n\r", ch);
    return;
  }
  if(IS_NPC(vict)) {
    send_to_char("The victim must be a player.\n\r", ch);
    return;
  }

  if(IS_SET(vict->new.imm_flags, WIZ_FREEZE)) {
    act("You have unfrozen $N!",0,ch,0,vict,TO_CHAR);
    act("$n has unfrozen you!",0,ch,0,vict,TO_VICT);
    sprintf(buf,"WIZLOG: %s has unfrozen %s.",GET_NAME(ch),GET_NAME(vict));
    log_s(buf);
    wizlog(buf,GET_LEVEL(ch),5);
    REMOVE_BIT(vict->new.imm_flags, WIZ_FREEZE);
  } else {
    if (GET_LEVEL(ch) < GET_LEVEL(vict))
      return;
    act("You have frozen $N!",0,ch,0,vict,TO_CHAR);
    act("$n has frozen you!",0,ch,0,vict,TO_VICT);
    sprintf(buf,"WIZLOG: %s has frozen %s.",GET_NAME(ch),GET_NAME(vict));
    log_s(buf);
    wizlog(buf,GET_LEVEL(ch),5);
    SET_BIT(vict->new.imm_flags, WIZ_FREEZE);
  }
}
/*
** do_reindex - Ranger March 2000
*/
void help_contents(struct help_index_element *help_index,char *contents, int top);
void boot_social_messages(void);

void do_reindex(struct char_data *ch, char *argument, int cmd) {
  char arg[MAX_INPUT_LENGTH];
  char usage[]={"\
This command is for reindexing and reloading various\n\r\
files after they have been edited.\n\r\
Usage: `kreindex`q <file>\n\r\
Supported files: help wizhelp olchelp misc socials\n\r\
misc=(motds, heros, credits, etc)\n\r"};

  if(!check_god_access(ch,TRUE)) return;

  one_argument(argument,arg);
  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  if(!strcmp(arg,"help")) {
    rewind(help_fl);
    help_index = build_help_index(help_fl, &top_of_helpt);
    help_contents(help_index,helpcontents,top_of_helpt);
    send_to_char("Done.\n\r",ch);
    return;
  }
  if(!strcmp(arg,"wizhelp")) {
    rewind(wizhelp_fl);
    wizhelp_index = build_help_index(wizhelp_fl, &top_of_wizhelpt);
    help_contents(wizhelp_index,wizhelpcontents,top_of_wizhelpt);
    send_to_char("Done.\n\r",ch);
    return;
  }
  if(!strcmp(arg,"olchelp")) {
    rewind(olchelp_fl);
    olchelp_index = build_help_index(olchelp_fl, &top_of_olchelpt);
    help_contents(olchelp_index,olchelpcontents,top_of_olchelpt);
    send_to_char("Done.\n\r",ch);
    return;
  }
  if(!strcmp(arg,"socials")) {
    boot_social_messages();
    send_to_char("Done.\n\r",ch);
    return;
  }
  if(!strcmp(arg,"misc")) {
    file_to_string(NEWS_FILE, news);
    file_to_string(CREDITS_FILE, credits);
    file_to_string(HEROES_FILE, heroes);
    file_to_string(MOTD_FILE, motd);
    file_to_string(NEWBIEMOTD_FILE, newbiemotd);
    file_to_string(GODMOTD_FILE, godmotd);
    file_to_string(HELP_PAGE_FILE, help);
    send_to_char("Done.\n\r",ch);
    return;
  }

  send_to_char(usage,ch);
}

void promote_mage(CHAR *promoter, CHAR *ch);
void promote_cleric(CHAR *promoter, CHAR *ch);
void promote_ninja(CHAR *promoter, CHAR *ch);
void promote_warrior(CHAR *promoter, CHAR *ch);
void promote_paladin(CHAR *promoter, CHAR *ch);
void promote_nomad(CHAR *promoter, CHAR *ch);
void promote_antipaladin(CHAR *promoter, CHAR *ch);
void promote_commando(CHAR *promoter, CHAR *ch);
void promote_thief(CHAR *promoter, CHAR *ch);
void promote_bard(CHAR *promoter, CHAR *ch);

void do_rank(struct char_data *ch, char *argument, int cmd) {
  int rank = 0, stop = 0;
  CHAR *vict = NULL;
  struct enchantment_type_5 *tmp_ench = NULL;
  char name[100], buf[MSL];

  if(!check_god_access(ch,TRUE)) return;

  argument=one_argument(argument,name);

  if(!*name) {
    send_to_char("Usage: rank <victim> {remove}\n\r", ch);
    return;
  }

  if(!(vict = get_char_room_vis(ch, name))) {
    send_to_char("That player is not here.\n\r", ch);
    return;
  }

  if (IS_NPC(vict)) {
    send_to_char("NO! Not on NPC's.\n\r", ch);
    return;
  }

  /* check for removal */
  one_argument(argument,name);
  if(*name && !strcmp(name,"remove")) {
    for(tmp_ench=vict->enchantments;!stop && tmp_ench;tmp_ench = tmp_ench->next) {
      stop = enchantment_special(tmp_ench,vict,vict,MSG_DEAD,"");
    }
    sprintf(buf,"WIZLOG: %s has dropped %s a rank.",GET_NAME(ch),GET_NAME(vict));
    log_s(buf);
    wizlog(buf,GET_LEVEL(ch),5);
    return;
  }

  /*check ranks and add enough xp/gold */

  /* person already ranked to top level */
  if(enchanted_by_type(vict, ENCHANT_SORCERER) ||
     enchanted_by_type(vict, ENCHANT_PROPHET) ||
     enchanted_by_type(vict, ENCHANT_SHOGUN) ||
     enchanted_by_type(vict, ENCHANT_KNIGHT) ||
     enchanted_by_type(vict, ENCHANT_LORDLADY) ||
     enchanted_by_type(vict, ENCHANT_TAMER) ||
     enchanted_by_type(vict, ENCHANT_ASSASSIN) ||
     enchanted_by_type(vict, ENCHANT_COMMANDER) ||
     enchanted_by_type(vict, ENCHANT_DARKLORDLADY) ||
     enchanted_by_type(vict, ENCHANT_CONDUCTOR))
  {
    act("$N has reached the highest rank already.",0,ch,0,vict,TO_CHAR);
    return;
  }

  if(enchanted_by_type(vict, ENCHANT_WARLOCK) ||
     enchanted_by_type(vict, ENCHANT_BISHOP) ||
     enchanted_by_type(vict, ENCHANT_SHINOBI) ||
     enchanted_by_type(vict, ENCHANT_SWASHBUCKLER) ||
     enchanted_by_type(vict, ENCHANT_JUSTICIAR) ||
     enchanted_by_type(vict, ENCHANT_FORESTER) ||
     enchanted_by_type(vict, ENCHANT_DARKWARDER) ||
     enchanted_by_type(vict, ENCHANT_POET) ||
     enchanted_by_type(vict, ENCHANT_COMMODORE) ||
     enchanted_by_type(vict, ENCHANT_BRIGAND))
  {
    GET_GOLD(vict)+=10000000;
    GET_EXP(vict)+=15000000;
    rank=3;
  }
  else
  if(enchanted_by_type(vict, ENCHANT_APPRENTICE) ||
     enchanted_by_type(vict, ENCHANT_ACOLYTE) ||
     enchanted_by_type(vict, ENCHANT_TSUME) ||
     enchanted_by_type(vict, ENCHANT_SQUIRE) ||
     enchanted_by_type(vict, ENCHANT_FIRSTSWORD) ||
     enchanted_by_type(vict, ENCHANT_WANDERER) ||
     enchanted_by_type(vict, ENCHANT_MINION) ||
     enchanted_by_type(vict, ENCHANT_MINSTREL) ||
     enchanted_by_type(vict, ENCHANT_PRIVATE) ||
     enchanted_by_type(vict, ENCHANT_HIGHWAYMAN))
  {
    GET_GOLD(vict)+=5000000;
    GET_EXP(vict)+=10000000;
    rank=2;
  }
  else {
    GET_GOLD(vict)+=1000000;
    GET_EXP(vict)+=5000000;
    rank=1;
  }

  switch(GET_CLASS(vict)) {
    case CLASS_MAGIC_USER:
      promote_mage(ch,vict);
      break;
    case CLASS_CLERIC:
      promote_cleric(ch,vict);
      break;
    case CLASS_THIEF:
      promote_thief(ch,vict);
      break;
    case CLASS_WARRIOR:
      promote_warrior(ch,vict);
      break;
    case CLASS_NINJA:
      promote_ninja(ch,vict);
      break;
    case CLASS_NOMAD:
      promote_nomad(ch,vict);
      break;
    case CLASS_PALADIN:
      promote_paladin(ch,vict);
      break;
    case CLASS_ANTI_PALADIN:
      promote_antipaladin(ch,vict);
      break;
    case CLASS_BARD:
      promote_bard(ch,vict);
      break;
    case CLASS_COMMANDO:
      promote_commando(ch,vict);
      break;
    default:
      act("$N's class does not have any ranks.",0,ch,0,vict,TO_CHAR);
      return;
      break;
  }

  sprintf(buf,"WIZLOG: %s has given rank %d to %s.",GET_NAME(ch),rank,GET_NAME(vict));
  log_s(buf);
  wizlog(buf,GET_LEVEL(ch),5);
}

void do_playeravg(struct char_data *ch, char *argument, int cmd) {
  int i,cl,avg_hp,count,avg_mana,low,high;
  OBJ *eq[MAX_WEAR];
  CHAR *vict;
  struct descriptor_data *d;
  extern struct descriptor_data *descriptor_list;
  char buf[MSL];

  if(!check_god_access(ch,TRUE)) return;

  if(!CHAOSMODE) {
    send_to_char("This command can only be used during CHAOS NIGHT.\n\r", ch);
    return;
  }

  one_argument(argument,buf);

  if(!*buf) {
    send_to_char("\
This command changes the hps/mana of all connected players\n\r\
to a range of average hps/mana to 1.5x average hps/mana. Move\n\r\
points are set to 1000.\n\r\n\r\
THIS WILL CHANGE PLAYER FILES AND SHOULD ONLY BE RAN FOR CHAOS\n\r\
NIGHT ON THE GAME COPY.\n\r\n\r\
    Usage: playeravg confirm\n\r", ch);
    return;
  }

  if(strcmp(buf,"confirm")) {
    send_to_char("\
This command changes the hps/mana of all connected players\n\r\
to a range of average hps/mana to 1.5x average hps/mana. Move\n\r\
points are set to 1000.\n\r\n\r\
THIS WILL CHANGE PLAYER FILES AND SHOULD ONLY BE RAN FOR CHAOS\n\r\
NIGHT ON THE GAME COPY.\n\r\n\r\
    Usage: playeravg confirm\n\r", ch);
    return;
  }

  for(cl=1;cl<12;cl++) {
    avg_hp=0;
    avg_mana=0;
    count=0;
    /* Scan all players and gather info for class cl */
    for (d = descriptor_list; d; d = d->next) {
      if(!d->character || d->connected!=CON_PLYNG) continue;
      vict=d->character;
      if(IS_MORTAL(vict) && GET_CLASS(vict)==cl) {

        /* Unequip character before avg calc */
        for (i=0;i<MAX_WEAR;i++) {
          if (vict->equipment[i])
            eq[i]=unequip_char(vict, i);
          else
            eq[i] = 0;
        }

        /* stats */
        count++;
        avg_hp+=GET_MAX_HIT(vict);
        avg_mana+=GET_MAX_MANA(vict);

        /* Re-equip character */
        for (i=0;i<WIELD;i++)
          if (eq[i]) equip_char (vict, eq[i], i);
        if (eq[HOLD]) equip_char(vict, eq[HOLD], HOLD);
        if (eq[WIELD]) equip_char(vict, eq[WIELD], WIELD);
      }
    }
    /* Done scanning list for class cl - calc final stats */
    if(count) {
      avg_hp=avg_hp/count;
      avg_mana=avg_mana/count;

      printf_to_char(ch,"Averaging %s to %d hps and %d mana.\n\r",pc_class_types[cl],avg_hp,avg_mana);

      /* Rescan players and apply stats */
      for (d = descriptor_list; d; d = d->next) {
        if(!d->character || d->connected!=CON_PLYNG) continue;
        vict=d->character;
        if(IS_MORTAL(vict) && GET_CLASS(vict)==cl) {

          /* Unequip character before applying */
          for (i=0;i<MAX_WEAR;i++) {
            if (vict->equipment[i])
              eq[i]=unequip_char(vict, i);
            else
              eq[i] = 0;
          }

          /* random adjustment of stats within range */
          low=avg_hp;
          high=(3*avg_hp/2);
          vict->points.max_hit = number(low,high);
          low=avg_mana-100;
          high=(3*avg_mana/2)-100;
          if(cl!=CLASS_THIEF && cl!=CLASS_WARRIOR && cl!=CLASS_NOMAD)
            vict->points.max_mana = number(low,high);
          vict->points.max_move = 900;

          /* Re-equip character */
          for (i=0;i<WIELD;i++)
            if (eq[i]) equip_char (vict, eq[i], i);
          if (eq[HOLD]) equip_char(vict, eq[HOLD], HOLD);
          if (eq[WIELD]) equip_char(vict, eq[WIELD], WIELD);

          /* Restore stats */
          GET_HIT(vict)=GET_MAX_HIT(vict);
          GET_MANA(vict)=GET_MAX_MANA(vict);
          GET_MOVE(vict)=GET_MAX_MOVE(vict);
        }
      } /* for from rescan */
    }
    else {
      printf_to_char(ch,"No %s present.\n\r",pc_class_types[cl]);
    }
  }  /* repeat process for next class */
  send_to_char("Done.\n\r",ch);
}

int check_god_access(CHAR *ch, int active);
void write_idname (void);

void do_idname(CHAR *ch, char *argument, int cmd) {
  int i,idnum;
  char buf[MIL];
  char usage[]="\
Usage: idname <#>     (returns the name of a player with that id number)\n\r\
              <name>  (returns the id number of a player with that name)\n\r\n\r\
SUP+ Only\n\r\
              list    (list all ids/players (large list - can cause link loss)\n\r\
              reset <#/all>  (set id number or all to null)\n\r";

  if(!check_god_access(ch,1)) return;
  argument=one_argument(argument, buf);
  if(!*buf) {
    send_to_char(usage,ch);
    return;
  }

  if(!strcmp(buf,"reset")) {
    if(GET_LEVEL(ch)>=LEVEL_SUP) {
      argument=one_argument(argument, buf);

      if (!strcmp(buf, "all")) {
        for(i=0;i<MAX_ID;i++) {
          strcpy(idname[i].name, "\0");
        }
        send_to_char("All names in the id list reset.\n\r",ch);
        write_idname();
      }
      else if (is_number(buf)) {
        idnum=atoi(buf);
        if(idnum<0 || idnum>=MAX_ID) {
          printf_to_char(ch,"The id number must be between 0 and %d.\n\r",MAX_ID);
          return;
        }
        strcpy(idname[idnum].name, "\0");
        printf_to_char(ch,"ID: %5d reset.\n\r",idnum);
        write_idname();
      }
    }
    else {
      send_to_char("You must at least be a SUP for this option.\n\r",ch);
    }
    return;
  }

  if(!strcmp(buf,"list")) {
    if(GET_LEVEL(ch)>=LEVEL_SUP) {
      for(i=0;i<MAX_ID;i++) {
        printf_to_char(ch,"ID: %5d, Player: %s\n\r",i,CAP(idname[i].name));
      }
    }
    else {
      send_to_char("You must at least be a SUP for this option.\n\r",ch);
    }
    return;
  }

  if(is_number(buf)) {
    idnum=atoi(buf);
    if(idnum<0 || idnum>=MAX_ID) {
      printf_to_char(ch,"The id number must be between 0 and %d.\n\r",MAX_ID);
      return;
    }
    printf_to_char(ch,"ID: %5d, Player: %s\n\r",idnum,CAP(idname[idnum].name));
    return;
  }

  /* Must be player name */
  string_to_lower(buf);
  for(i=0;i<MAX_ID;i++) {
    if(!strcasecmp(idname[i].name,buf)) {
      printf_to_char(ch,"ID: %5d, Player: %s\n\r",i,CAP(buf));
      return;
    }
  }
  printf_to_char(ch,"Player: %s not found in id list.\n\r",CAP(buf));
}

void do_movestat(struct char_data *ch, char *arg, int cmd)
{
  struct char_data *source_char;
  struct char_data *target_char;
  char buf[MAX_INPUT_LENGTH];
  char source_name[100],target_name[100];
  char stat[100],amount[100];
  char usage[]="\n\r\
Syntax:       movestat <stat> <source> <target> <amount>\n\r\
Stat can be:  scp, qp, remort\n\r\
\n\r\
Minimum amount to transfer is 700 mil remort experience.\n\r\
Remort enchantment will be set/removed as necessary.\n\r";
  int value = 0,yield = 0;
  long long int big_value = 0;

  if(!check_god_access(ch,TRUE)) return;

  sprintf(buf,"WIZINFO: %s movestat %s",GET_NAME(ch),arg);
  wizlog(buf, GET_LEVEL(ch)+1, 5);
  log_s(buf);

  if(!*arg) {
    send_to_char(usage,ch);
    return;
  }

  arg = one_argument(arg, stat);
  arg = one_argument(arg, source_name);
  arg = one_argument(arg, target_name);
  arg = one_argument(arg, amount);

  if (!*stat || !*source_name || !*source_name || !*amount ) {
    send_to_char("`iInvalid syntax.\n\r",ch);
    send_to_char(usage,ch);
    return;
  }
  if (!(source_char = get_char_vis(ch, source_name))) {
    send_to_char("`iSource character not found.\n\r",ch);
    send_to_char(usage,ch);
    return;
  }
  if (!(target_char = get_char_vis(ch, target_name))) {
    send_to_char("`iTarget character not found.\n\r",ch);
    send_to_char(usage,ch);
    return;
  }
  if(IS_NPC(target_char) || IS_NPC(source_char)){
    send_to_char("`iTarget and source characters cannot be NPCs.\n\r",ch);
    send_to_char(usage,ch);
    return;
  }
  if(target_char == source_char) {
    send_to_char("`iTarget and source cannot be the same character.\n\r",ch);
    send_to_char(usage,ch);
    return;
  }

  if (is_abbrev(stat, "qp") || is_abbrev(stat, "aqpoints") || is_abbrev(stat, "questpoints") || is_abbrev(stat, "quest_points"))
  {
    value = atoi(amount);
    if(value < 10) { //min 10 to transfer
      send_to_char("`iA minimum transfer of 10 quest points is required.\n\r",ch);
      send_to_char(usage,ch);
      return;
    }
    if(source_char->ver3.quest_points < value) {
      send_to_char("`iSource character does not have that amount of quest points.\n\r",ch);
      send_to_char(usage,ch);
    } else {
      yield = value*0.8;
      source_char->ver3.quest_points -= value;
      sprintf(buf, "%d quest points were removed from %s.\n\r",value,GET_NAME(source_char));
      send_to_char(buf, ch);
      sprintf(buf, "%s removes %d of your quest points and transfers %d to %s. (20%% fee)\n\r",GET_NAME(ch),value,yield,GET_NAME(target_char));
      send_to_char(buf, source_char);

      target_char->ver3.quest_points += yield;
      sprintf(buf, "%d quest points were transfered to %s.\n\r",yield,GET_NAME(target_char));
      send_to_char(buf, ch);
      sprintf(buf, "%s transfers %d quest points to you.\n\r",GET_NAME(ch),yield);
      send_to_char(buf, target_char);
    }
    return;
  }
  if (is_abbrev(stat, "sc") || is_abbrev(stat, "scpoints") || is_abbrev(stat, "subclasspoints") || is_abbrev(stat, "subclass_points"))
  {
    value = atoi(amount);
    if(value < 10) { //min 10 to transfer
      send_to_char("`iA minimum transfer of 10 subclass points is required.\n\r",ch);
      send_to_char(usage,ch);
      return;
    }
    if(source_char->ver3.subclass_points < value) {
      send_to_char("`iSource character does not have that amount of subclass points.\n\r",ch);
      send_to_char(usage,ch);
    } else {
      yield = value*0.8;
      source_char->ver3.subclass_points -= value;
      sprintf(buf, "%d subclass points were removed from %s.\n\r",value,GET_NAME(source_char));
      send_to_char(buf, ch);
      sprintf(buf, "%s removes %d of your subclass points and transfers %d to %s. (20%% fee)\n\r",GET_NAME(ch),value,yield,GET_NAME(target_char));
      send_to_char(buf, source_char);

      target_char->ver3.subclass_points += yield;
      sprintf(buf, "%d subclass points were transfered to %s.\n\r",yield,GET_NAME(target_char));
      send_to_char(buf, ch);
      sprintf(buf, "%s transfers %d subclass points to you.\n\r",GET_NAME(ch),yield);
      send_to_char(buf, target_char);
    }
    return;
  }
  if (is_abbrev(stat, "remortxp") || is_abbrev(stat, "remortexp") || is_abbrev(stat, "remort_xp") || is_abbrev(stat, "remort_exp") || is_abbrev(stat, "remortpool") || is_abbrev(stat, "remort_pool"))
  {
    big_value = strtoll(amount, NULL, 10);
    if(big_value < 700000000) { //min of 700mil to transfer
      send_to_char("`iA minimum transfer of 700 mil remort experience is required.\n\r",ch);
      send_to_char(usage,ch);
      return;
    }
    if(source_char->ver3.remort_exp < big_value) {
      send_to_char("`iSouce character does not have that amount of remort experience.\n\r",ch);
      send_to_char(usage,ch);
    } else {
      source_char->ver3.remort_exp -= big_value;
      sprintf(buf, "%lld remort experience was removed from %s.\n\r",big_value,GET_NAME(source_char));
      send_to_char(buf, ch);
      sprintf(buf, "%s removes %lld of your remort experience and transfers it to %s. (less a 20%% fee)\n\r",GET_NAME(ch),big_value,GET_NAME(target_char));
      send_to_char(buf, source_char);

      target_char->ver3.remort_exp += big_value*0.8;
      sprintf(buf, "%lld remort experience was transfered to %s.\n\r",(long long int)(big_value*0.8),GET_NAME(target_char));
      send_to_char(buf, ch);
      sprintf(buf, "%s transfers %lld remort experience to you.\n\r",GET_NAME(ch),(long long int)(big_value*0.8));
      send_to_char(buf, target_char);

      /* set/remove remort enchantment */
      if(source_char->ver3.remort_exp == 0) {
        rv2_remove_enchant(source_char);
        sprintf(buf, "%s's remort enchantment was turned off.\n\r",GET_NAME(source_char));
        send_to_char(buf, ch);
      }
      if(target_char->ver3.remort_exp >= 1) {
        sprintf(buf, "%s's remort enchantment was turned on.\n\r",GET_NAME(target_char));
        send_to_char(buf, ch);
        rv2_add_enchant(target_char);
      }
    }
    return;
  }
  send_to_char ("`iInvalid field specified.\n\r", ch);
  send_to_char(usage,ch);
  return;
} /* do_movestat */
