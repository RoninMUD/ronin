/*
** subclass.c
**   Routines for subclasses.
**
** Written for RoninMUD by Ranger
**
** Do not distribute without permission.
*/

/*
$Author: void $
$Date: 2004/10/21 17:08:23 $
$Header: /home/ronin/cvs/ronin/subclass.c,v 2.7 2004/10/21 17:08:23 void Exp $
$Id: subclass.c,v 2.7 2004/10/21 17:08:23 void Exp $
$Name:  $
$Log: subclass.c,v $
Revision 2.7  2004/10/21 17:08:23  void
Added level 50 AP spell Blood Lust

Revision 2.6  2004/09/29 22:56:13  void
Spell Rush for Commando

Revision 2.5  2004/09/28 20:07:03  void
Some more fixes to Divine Intervention

Revision 2.4  2004/05/12 13:20:33  ronin
Fix for Rashgugh to not pop in certain zones.  Previous fix prevented
him from moving into certain zones.

Revision 2.3  2004/05/05 12:38:27  ronin
Added Olyumpus zones to those blocked from Rashgugh.

Revision 2.2  2004/04/29 11:40:35  ronin
Added Hell and Dagoor zones to those Rashgugh does not visit.

Revision 2.1  2004/02/19 19:20:22  void
Added Spell Shroud of Existence (Good Align Commando Spell)

Revision 2.0.0.1  2004/02/05 16:11:49  ronin
Reinitialization of cvs archives

Revision 2003/07/31 Ranger
Check to allow mobs to be able to use subclass skills/spells added to
check_subclass.

Revision 1.7  2003/02/11 13:25:03  ronin
Replaced Twixt with Pan as bard sc master.

Revision 1.6  2002/11/03 13:25:03  ronin
Fix to add Merc level 4 to check_sc_access for FEINT.

Revision 1.5  2002/04/27 05:55:03  ronin
Fix to have twixt spec working and have twixt as subclass master.

Revision 1.4  2002/04/16 18:00:11  ronin
Fix to allow mobs to cast certain subclass spells and skills.
The skills allowed are those that require an active command.
No access is given to automatic skills.

Revision 1.3  2002/04/15 03:25:35  ronin
Update to allow mobs to use subclass spells.

Revision 1.2  2002/03/31 07:42:16  ronin
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
#include "subclass.h"

extern struct descriptor_data *descriptor_list;
int check_subclass(CHAR *ch,int sub,int lvl);
int check_god_access(CHAR *ch, int active);
extern int CHAOSMODE;
extern int TOKENCOUNT;
int distribute_token(void);

int check_sc_song_access(CHAR *ch, int s) {
  if(s<=17) return TRUE;
  if(GET_LEVEL(ch)>=LEVEL_ETE) return TRUE;
  switch(s) {
    case 18: /* song of remove poison*/
      if(check_subclass(ch,SC_CHANTER,1)) return TRUE;
      break;
    case 19: /* rally song*/
      if(check_subclass(ch,SC_BLADESINGER,1)) return TRUE;
      break;
    case 20: /* warchant */
      if(check_subclass(ch,SC_CHANTER,2)) return TRUE;
      break;
    case 21: /* song of luck */
      if(check_subclass(ch,SC_CHANTER,3)) return TRUE;
      break;
    case 22: /* rejuvenation song*/
      /* rejuvenation is now a class song */
      return TRUE;
      /*if(check_subclass(ch,SC_BLADESINGER,3)) return TRUE;*/
      break;
    case 23: /* aid */
      if(check_subclass(ch,SC_CHANTER,4)) return TRUE;
      break;
    case 24: /* protection evil good */
      if(check_subclass(ch,SC_BLADESINGER,4)) return TRUE;
      break;
    case 25: /* thunderball */
      if(check_subclass(ch,SC_BLADESINGER,5)) return TRUE;
      break;
    case 26: /* sphere */
      if(check_subclass(ch,SC_CHANTER,5)) return TRUE;
      break;
  }
  return FALSE;
}

int check_sc_access(CHAR *ch, int skill)
{
  if (skill <= 165) return TRUE;
  if (GET_LEVEL(ch) >= LEVEL_SUP) return TRUE;
  if (IS_NPC(ch)) return TRUE;

  switch (skill)
  {
    case SKILL_MEDITATE:
      if (check_subclass(ch, SC_DRUID, 1)) return TRUE;
      if (check_subclass(ch, SC_TEMPLAR, 1)) return TRUE;
      if (check_subclass(ch, SC_CAVALIER, 1)) return TRUE;
      break;
    case SPELL_CLARITY:
      if (check_subclass(ch, SC_DRUID, 2)) return TRUE;
      break;
    case SPELL_WALL_THORNS:
      if (check_subclass(ch, SC_DRUID, 3)) return TRUE;
      break;
    case SPELL_MAGIC_ARMAMENT:
      if (check_subclass(ch, SC_DRUID, 4)) return TRUE;
      break;
    case SPELL_DEGENERATE:
      if (check_subclass(ch, SC_DRUID, 5)) return TRUE;
      break;
    case SPELL_MIGHT:
      if (check_subclass(ch, SC_TEMPLAR, 2)) return TRUE;
      if (check_subclass(ch, SC_CAVALIER, 2)) return TRUE;
      break;
    case SPELL_SANCTIFY:
      if (check_subclass(ch, SC_TEMPLAR, 3)) return TRUE;
      break;
    case SPELL_ORB_PROTECTION:
      if (check_subclass(ch, SC_ARCHMAGE, 2)) return TRUE;
      if (check_subclass(ch, SC_TEMPLAR, 4)) return TRUE;
      break;
    case SPELL_FORTIFICATION:
      if (check_subclass(ch, SC_TEMPLAR, 5)) return TRUE;
      break;
    case SPELL_BLADE_BARRIER:
      if (check_subclass(ch, SC_ENCHANTER, 1)) return TRUE;
      break;
    case SPELL_PASSDOOR:
      if (check_subclass(ch, SC_ENCHANTER, 2)) return TRUE;
      break;
    case SPELL_ENGAGE:
      if (check_subclass(ch, SC_ENCHANTER, 3)) return TRUE;
      break;
    case SPELL_ETHEREAL_NATURE:
      if (check_subclass(ch, SC_ENCHANTER, 4)) return TRUE;
      break;
    case SPELL_DISRUPT_SANCT:
      if (check_subclass(ch, SC_ENCHANTER, 5)) return TRUE;
      break;
    case SPELL_METEOR:
      if (check_subclass(ch, SC_ARCHMAGE, 1)) return TRUE;
      break;
    case SPELL_FROSTBOLT:
      if (check_subclass(ch, SC_ARCHMAGE, 3)) return TRUE;
      if (check_subclass(ch, SC_LEGIONNAIRE, 4)) return TRUE;
      break;
    case SPELL_WRATH_ANCIENTS:
      if (check_subclass(ch, SC_ARCHMAGE, 4)) return TRUE;
      break;
    case SPELL_DISTORTION:
      if (check_subclass(ch, SC_ARCHMAGE, 5)) return TRUE;
      break;
    case SKILL_BLITZ:
      if (check_subclass(ch, SC_BLADESINGER, 2)) return TRUE;
      break;
    case SKILL_CAMP:
      if ((GET_CLASS(ch) == CLASS_BARD) && (GET_LEVEL(ch) >= 35)) return TRUE;
      if (check_subclass(ch, SC_TRAPPER, 2)) return TRUE;
      break;
    case SKILL_BATTER:
      if (check_subclass(ch, SC_TRAPPER, 3)) return TRUE;
      break;
    case SKILL_FRENZY:
      if (check_subclass(ch, SC_TRAPPER, 5)) return TRUE;
      break;
    case SKILL_AWARENESS:
      if (check_subclass(ch, SC_RANGER, 1)) return TRUE;
      if (check_subclass(ch, SC_WARLORD, 2)) return TRUE;
      break;
    case SKILL_PROTECT:
      if (check_subclass(ch, SC_CAVALIER, 3)) return TRUE;
      if (check_subclass(ch, SC_GLADIATOR, 1)) return TRUE;
      if (check_subclass(ch, SC_RANGER, 2)) return TRUE;
      break;
    case SKILL_SWEEP: /* !PALADIN! Give to paladin subclass */
      if (check_subclass(ch, SC_RANGER, 3)) return TRUE;
      break;
    case SKILL_BERSERK:
      if (check_subclass(ch, SC_RANGER, 4)) return TRUE;
      break;
    case SKILL_DEFEND:
      if (check_subclass(ch, SC_WARLORD, 4)) return TRUE;
      if (check_subclass(ch, SC_RANGER, 5)) return TRUE;
      break;
    case SKILL_FADE:
      if (check_subclass(ch, SC_BANDIT, 1)) return TRUE;
      break;
    case SKILL_IMPAIR:
      if (check_subclass(ch, SC_BANDIT, 2)) return TRUE;
      break;
    case SKILL_EVASION:
      if (check_subclass(ch, SC_BANDIT, 5)) return TRUE;
      break;
    case SKILL_DIRTY_TRICKS:
      if (check_subclass(ch, SC_ROGUE, 1)) return TRUE;
      break;
    case SKILL_TROPHY:
      if (check_subclass(ch, SC_ROGUE, 2)) return TRUE;
      if (check_subclass(ch, SC_TRAPPER, 4)) return TRUE;
      break;
    case SKILL_VEHEMENCE:
      if (check_subclass(ch, SC_ROGUE, 3)) return TRUE;
      break;
    case SKILL_TRIP:
      if (check_subclass(ch, SC_ROGUE, 4)) return TRUE;
      break;
    case SKILL_SWITCH: /* !PALADIN! */
      if (check_subclass(ch, SC_GLADIATOR, 2)) return TRUE;
      if (check_subclass(ch, SC_CRUSADER, 3)) return TRUE;
      break;
    case SKILL_FLANK:
      if (check_subclass(ch, SC_GLADIATOR, 3)) return TRUE;
      break;
    case SKILL_HOSTILE:
      if (check_subclass(ch, SC_GLADIATOR, 4)) return TRUE;
      break;
    case SKILL_HEADBUTT:
      if (check_subclass(ch, SC_GLADIATOR, 5)) return TRUE;
      break;
    case SKILL_CHARGE:
      if (check_subclass(ch, SC_WARLORD, 5)) return TRUE;
      break;
    case SPELL_IRONSKIN:
      if (check_subclass(ch, SC_MERCENARY, 1)) return TRUE;
      break;
    case SPELL_CLOUD_CONFUSION:
      if (check_subclass(ch, SC_MERCENARY, 2)) return TRUE;
      break;
    case SKILL_RIPOSTE:
      if (check_subclass(ch, SC_MERCENARY, 3)) return TRUE;
      break;
    case SPELL_TREMOR:
      if (check_subclass(ch, SC_MERCENARY, 4)) return TRUE;
      break;
    case SPELL_DEVASTATION:
      if (check_subclass(ch, SC_MERCENARY, 5)) return TRUE;
      break;
    case SKILL_LUNGE:
      if (check_subclass(ch, SC_LEGIONNAIRE, 1)) return TRUE;
      break;
    case SPELL_RIMEFANG:
      if (check_subclass(ch, SC_LEGIONNAIRE, 2)) return TRUE;
      break;
    case SKILL_CLOBBER:
      if (check_subclass(ch, SC_LEGIONNAIRE, 3)) return TRUE;
      break;
    case SPELL_INCENDIARY_CLOUD:
      if (check_subclass(ch, SC_LEGIONNAIRE, 5)) return TRUE;
      break;
    case SKILL_BACKFIST:
      if (check_subclass(ch, SC_RONIN, 2)) return TRUE;
      break;
    case SPELL_BLUR:
      if (check_subclass(ch, SC_RONIN, 4)) return TRUE;
      break;
    case SKILL_BANZAI:
      if (check_subclass(ch, SC_RONIN, 5)) return TRUE;
      break;
    case SPELL_DEBILITATE:
      if (check_subclass(ch, SC_MYSTIC, 1)) return TRUE;
      break;
    case SKILL_TIGERKICK:
      if (check_subclass(ch, SC_MYSTIC, 2)) return TRUE;
      break;
    case SPELL_DIVINE_WIND:
      if (check_subclass(ch, SC_MYSTIC, 3)) return TRUE;
      break;
    case SKILL_MANTRA:
      if (check_subclass(ch, SC_MYSTIC, 4)) return TRUE;
      break;
    case SPELL_TRANQUILITY:
      if (check_subclass(ch, SC_MYSTIC, 5)) return TRUE;
      break;
    case SKILL_PRAY:
      if (check_subclass(ch, SC_CRUSADER, 1)) return TRUE;
      break;
    case SPELL_RIGHTEOUSNESS:
      if (check_subclass(ch, SC_CRUSADER, 2)) return TRUE;
      break;
    case SPELL_FOCUS:
      if (check_subclass(ch, SC_CRUSADER, 4)) return TRUE;
      break;
    case SPELL_POWER_OF_FAITH:
      if (check_subclass(ch, SC_CRUSADER, 5)) return TRUE;
      break;
    case SPELL_WRATH_OF_GOD:
      if (check_subclass(ch, SC_CAVALIER, 4)) return TRUE;
      break;
    case SPELL_POWER_OF_DEVOTION:
      if (check_subclass(ch, SC_CAVALIER, 5)) return TRUE;
      break;
    case SPELL_DARK_RITUAL:
      if (check_subclass(ch, SC_DEFILER, 1)) return TRUE;
      break;
    case SPELL_BLACKMANTLE:
      if (check_subclass(ch, SC_DEFILER, 2)) return TRUE;
      break;
    case SKILL_FEINT:
      if (check_subclass(ch, SC_DEFILER, 3)) return TRUE;
      break;
    case SPELL_DESECRATE:
      if (check_subclass(ch, SC_DEFILER, 4)) return TRUE;
      break;
    case SKILL_SHADOW_WALK:
      if (check_subclass(ch, SC_DEFILER, 5)) return TRUE;
      break;
    case SKILL_ASSASSINATE:
      if (check_subclass(ch, SC_INFIDEL, 1)) return TRUE;
      break;
    case SKILL_VICTIMIZE:
      if (check_subclass(ch, SC_INFIDEL, 2)) return TRUE;
      break;
    case SPELL_RAGE:
      if (check_subclass(ch, SC_INFIDEL, 3)) return TRUE;
      break;
    case SPELL_DEMONIC_THUNDER:
      if (check_subclass(ch, SC_INFIDEL, 4)) return TRUE;
      break;
    case SPELL_SHADOW_WRAITH:
      if (check_subclass(ch, SC_INFIDEL, 5)) return TRUE;
      break;
    case SPELL_REJUVENATION:
      return TRUE;
      break;
  }

  return FALSE;
}

int check_subclass(CHAR *ch,int sub,int lvl) {
  int pclass;

  if(IS_NPC(ch)) return TRUE; /* Allows mobs to use subclass skills/spells */
  if(GET_LEVEL(ch)>=LEVEL_ETE) return TRUE;
  /* Check Level */
  if(GET_LEVEL(ch)<30) return FALSE;
  if(GET_LEVEL(ch)<40 && lvl >2) return FALSE;
  if(GET_LEVEL(ch)<45 && lvl >4) return FALSE;
  /* Check proper class */
  pclass=GET_CLASS(ch);
  if((sub>2*pclass) || (sub<=2*(pclass-1))) return FALSE;
  if(ch->ver3.subclass==sub && ch->ver3.subclass_level>=lvl) return TRUE;
  return FALSE;
}

int set_subclass(CHAR *ch,int sub,int lvl) {
  int pclass;

  /* Check player level */
  if(GET_LEVEL(ch)<30) return FALSE;
  if(GET_LEVEL(ch)<30) return FALSE;
  if(GET_LEVEL(ch)<40 && lvl >2) return FALSE;
  if(GET_LEVEL(ch)<45 && lvl >4) return FALSE;
  /* Check proper class */
  pclass=GET_CLASS(ch);
  if((sub>2*pclass) || (sub<=2*(pclass-1))) return FALSE;
  ch->ver3.subclass=sub;
  ch->ver3.subclass_level=lvl;
  return TRUE;
}

void remove_subclass(CHAR *ch) {
  ch->ver3.subclass=0;
  ch->ver3.subclass_level=0;
}

extern char *subclass_name[];
void do_subclass(CHAR *ch, char *argument, int cmd) {
  CHAR *vict;
  char buf[MAX_INPUT_LENGTH];
  int subclass=0,level=0,add=0;

  char Usage[]={"\
Usage: subclass add <player> <subclass> <level>\n\r\
       subclass rem <player>\n\r\
       subclass list\n\r\
       subclass points <player> <amount>\n\r"};

  if(IS_NPC(ch)) return;
  if(GET_LEVEL(ch)<LEVEL_SUP) {
    send_to_char("Your level isn't high enough.\n\r",ch);
    return;
  }
  if (!IS_SET(ch->new.imm_flags, WIZ_TRUST)) {
    send_to_char("You need a Trust flag to do that!\n\r", ch);
    return;
  }
  if (IS_SET(ch->new.imm_flags, WIZ_FREEZE)) {
    send_to_char("You are frozen, so you can't do it!\n\r", ch);
    return;
  }
  if (!IS_SET(ch->new.imm_flags, WIZ_ACTIVE)) {
    send_to_char("You need an active flag for this command.\n\r", ch);
    return;
  }

  argument=one_argument(argument, buf);
  if(!*buf) { send_to_char(Usage,ch); return; }
  if(!strcmp(buf,"add")) add=1;
  else if(!strcmp(buf,"rem")) add=0;
  else if(!strcmp(buf,"list")) {
    send_to_char("\
Subclasses are: (Mu) ENCHANTER    ARCHMAGE\n\r\
                (Cl) DRUID        TEMPLAR\n\r\
                (Th) ROGUE        BANDIT\n\r\
                (Wa) WARLORD      GLADIATOR\n\r\
                (Ni) RONIN        MYSTIC\n\r\
                (No) RANGER       TRAPPER\n\r\
                (Pa) CAVALIER     CRUSADER\n\r\
                (Ap) DEFILER      INFIDEL\n\r\
                (Ba) BLADESINGER  CHANTER\n\r\
                (Co) LEGIONNAIRE  MERCENARY\n\r",ch);
    return;
  }
  else if(!strcmp(buf,"points")) {
    argument=one_argument(argument, buf);
    if(!*buf) { send_to_char(Usage,ch); return; }
    if(!(vict=get_char_vis(ch,buf))) {
      send_to_char("That player isn't here.\n\r",ch);
      return;
    }
    if(IS_NPC(vict)) {
      send_to_char("You can't set the subclass points on a mob.\n\r",ch);
      return;
    }
    argument=one_argument(argument, buf);
    if(!*buf) { send_to_char(Usage,ch); return; }
    if(!is_number(buf)) { send_to_char(Usage,ch); return; }
    level=atoi(buf);
    vict->ver3.subclass_points=level;
    sprintf(buf,"WIZINFO: %s set %d subclass points on %s.",GET_NAME(ch),level,GET_NAME(vict));
    wizlog(buf,GET_LEVEL(ch)+1,5);
    send_to_char("Done\n\r",ch);
    return;
  }
  else { send_to_char(Usage,ch); return; }

  argument=one_argument(argument, buf);
  if(!*buf) { send_to_char(Usage,ch); return; }
  if(!(vict=get_char_vis(ch,buf))) {
    send_to_char("That player isn't here.\n\r",ch);
    return;
  }
  if(IS_NPC(vict)) {
    send_to_char("You can't set/rem the subclass on a mob.\n\r",ch);
    return;
  }

  if(add) {
    argument=one_argument(argument, buf);
    if(!*buf) { send_to_char(Usage,ch); return; }
    subclass=old_search_block(string_to_upper(buf),0,strlen(buf),subclass_name,TRUE);
    if(subclass==-1) {
      send_to_char("\
Subclasses are: (Mu) ENCHANTER    ARCHMAGE\n\r\
                (Cl) DRUID        TEMPLAR\n\r\
                (Th) ROGUE        BANDIT\n\r\
                (Wa) WARLORD      GLADIATOR\n\r\
                (Ni) RONIN        MYSTIC\n\r\
                (No) RANGER       TRAPPER\n\r\
                (Pa) CAVALIER     CRUSADER\n\r\
                (Ap) DEFILER      INFIDEL\n\r\
                (Ba) BLADESINGER  CHANTER\n\r\
                (Co) LEGIONNAIRE  MERCENARY\n\r",ch);
      return;
    }

    argument=one_argument(argument, buf);
    if(!*buf) { send_to_char(Usage,ch); return; }
    if(!is_number(buf)) { send_to_char(Usage,ch); return; }
    level=atoi(buf);
    if(level<1 || level >5) { send_to_char(Usage,ch); return; }
  }

  if(add) {
    if(!set_subclass(vict,subclass,level)) {
      send_to_char("Subclass assignment failed.\n\r",ch);
      return;
    }
    sprintf(buf,"WIZINFO: %s set %s to subclass %s, level %d.",GET_NAME(ch),GET_NAME(vict),subclass_name[subclass-1],level);
    wizlog(buf,GET_LEVEL(ch)+1,5);
  }
  else {
    sprintf(buf,"WIZINFO: %s removed %s's subclass.",GET_NAME(ch),GET_NAME(vict));
    wizlog(buf,GET_LEVEL(ch)+1,5);
    remove_subclass(vict);
  }
  send_to_char("Done.\n\r",ch);
}

/* Any special objects used in subclasses - in zone LIMBO */

#define STATUE_PEACE 9
int statue_peace(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  if(cmd!=MSG_TICK) return FALSE;
  obj->spec_value--;
  if(obj->spec_value>0) return FALSE;
  send_to_room("The statue of peace crumbles to dust.\n\r",obj->in_room);
  REMOVE_BIT(world[obj->in_room].room_flags,NO_MOB);
  extract_obj(obj);
  return FALSE;
}

#define WALL_THORNS 34
int wall_thorns(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  if(cmd!=MSG_TICK) return FALSE;
  obj->spec_value--;
  if(obj->spec_value>0) return FALSE;
  send_to_room("The wall of thorns slowly wilts and disappears.\n\r",obj->in_room);
  extract_obj(obj);
  return FALSE;
}

int check_sc_master(CHAR *ch, CHAR *mob) {
  if(IS_NPC(ch)) {
    act("$N tells you 'Go Away! I don't deal with your type!'",FALSE,ch,0,mob,TO_CHAR);
    return FALSE;
  }
  if(!IS_NPC(mob)) return FALSE;
  if(V_MOB(mob)!=1915 && V_MOB(mob)!=2104 && V_MOB(mob)!=7322 && V_MOB(mob)!=12913 &&
     V_MOB(mob)!=9640 && V_MOB(mob)!=28506 && V_MOB(mob)!=11096 && V_MOB(mob)!=2803 &&
     V_MOB(mob)!=9012 && V_MOB(mob)!=2107)
    return FALSE;

  if(GET_CLASS(ch)==GET_CLASS(mob)) return TRUE;
  act("$N tells you 'Go Away! I don't deal with your type!'",FALSE,ch,0,mob,TO_CHAR);
  return FALSE;
}

int check_sc_points(CHAR *ch, CHAR *mob) {
  int points;
  points=ch->ver3.subclass_level*70+70;
  if(ch->ver3.subclass_points>=points) {
    ch->ver3.subclass_points-=points;
    return TRUE;
  }
  act("$N tells you 'You do not have enough subclass points.",0,ch,0,mob,TO_CHAR);
  return FALSE;
}


int guild(CHAR *mob, CHAR *ch, int cmd, char *arg);
int subclass_master(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  char name1[20],name2[20];
  OBJ *obj;
  int point,subclass,pclass,token_number,loop=0;
  char *sc_name_l[] = {
    "enchanter",
    "archmage",
    "druid",
    "templar",
    "rogue",
    "bandit",
    "warlord",
    "gladiator",
    "ronin",
    "mystic",
    "ranger",
    "trapper",
    "cavalier",
    "crusader",
    "defiler",
    "infidel",
    "",/* avatar*/
    "",
    "bladesinger",
    "chanter",
    "legionnaire",
    "mercenary",
    "\n"
  };

  if (cmd == CMD_PRACTICE) {
    if(guild(mob,ch,cmd, arg)) return TRUE;
    else return FALSE;
  }

  if(cmd==CMD_SAY) {
    one_argument(arg,buf);
    if(!*buf || !is_abbrev(buf,"choose")) return FALSE;
    do_say(ch, arg, CMD_SAY);
    arg=one_argument(arg,buf);
    if(!check_sc_master(ch,mob)) return TRUE;
    one_argument(arg,buf);
    pclass=GET_CLASS(ch);
    if(pclass<1 || pclass>11) return FALSE;
    sprintf(name1,"%s",sc_name_l[2*pclass-2]);
    sprintf(name2,"%s",sc_name_l[2*pclass-1]);
    if(!*buf) {
      sprintf(buf,"$N tells you 'Which subclass path did you want to choose: %s or %s?'",
              CAP(name1),CAP(name2));
      act(buf,0,ch,0,mob,TO_CHAR);
      return TRUE;
    }

    subclass=old_search_block(string_to_upper(buf),0,strlen(buf),subclass_name,TRUE);
    if(subclass==-1) {
      sprintf(buf,"$N tells you 'Thats not a valid subclass.  Your choices are: %s or %s.'",
              CAP(name1),CAP(name2));
      act(buf,0,ch,0,mob,TO_CHAR);
      return TRUE;
    }
    if((subclass>2*pclass) || (subclass<2*pclass-1)) {
      sprintf(buf,"$N tells you 'Thats not a valid choice.  Your choices are: %s or %s.'",
              CAP(name1),CAP(name2));
      act(buf,0,ch,0,mob,TO_CHAR);
      return TRUE;
    }

    if(ch->ver3.subclass) {
      if(ch->ver3.subclass!=subclass) { /* Change of subclass */
        act("$N tells you 'You can't change your subclass - yet!",0,ch,0,mob,TO_CHAR);
        return TRUE;
      }
      else { /* Increase in level */
        if(ch->ver3.subclass_level>4) {
          act("$N tells you 'You already have the highest possible subclass level.",0,ch,0,mob,TO_CHAR);
          return TRUE;
        }
        if(!check_sc_points(ch,mob)) return TRUE; /* Also subtracts points */
        ch->ver3.subclass_level++;
        sprintf(name1,"%s",sc_name_l[subclass-1]);
        sprintf(buf,"%s shouts '%s has taken another step along the path of the %s!'\n\r",
                GET_SHORT(mob),GET_NAME(ch),CAP(name1));
        send_to_world(buf);
        save_char(ch,NOWHERE);
        return TRUE;
      }
    }
    else { /* New Subclass at lvl 1 */
      if(!check_sc_points(ch,mob)) return TRUE; /* Also subtracts points */
      ch->ver3.subclass=subclass;
      ch->ver3.subclass_level=1;
      sprintf(name1,"%s",sc_name_l[subclass-1]);
      sprintf(buf,"%s shouts '%s has joined the path of the %s! All bow before %s might!'\n\r",
              GET_SHORT(mob),GET_NAME(ch),CAP(name1),HSHR(ch));
      send_to_world(buf);
      save_char(ch,NOWHERE);
      return TRUE;
    }
    return TRUE;
  }

  if(cmd==MSG_GAVE_OBJ) {
    if(!isname("token",arg)) return FALSE;
    if(!(obj=get_obj_in_list_vis(mob,"token",mob->carrying))) {
      act("$N tells you 'That wasn't a token you gave me.'",FALSE,ch,0,mob,TO_CHAR);
      return TRUE;
    }
    if(obj->obj_flags.type_flag!=ITEM_SC_TOKEN || IS_SET(obj->obj_flags.extra_flags, ITEM_CLONE)) {
      act("$N tells you 'That wasn't a valid token.'",FALSE,ch,0,mob,TO_CHAR);
      return TRUE;
    }
    if(!check_sc_master(ch,mob)) {
      act("$N tells you 'Here, take it back.'",FALSE,ch,0,mob,TO_CHAR);
      act("$N gives you a token.",FALSE,ch,0,mob,TO_CHAR);
      obj_from_char(obj);
      obj_to_char(obj,ch);
      return TRUE;
    }
    point=obj->obj_flags.value[0];
    if(point==1)
      sprintf(buf,"$N tells you 'Thank you.  You get a subclass point for that token.'");
    else if(point>1)
      sprintf(buf,"$N tells you 'Thank you.  You get %d subclass points for that token.'",point);
    else
      sprintf(buf,"$N tells you 'Sorry, that token is worthless.'");
    act(buf,FALSE,ch,0,mob,TO_CHAR);
    log_f("SUBLOG: %s turns in a token.",GET_NAME(ch));
    extract_obj(obj);
    if(point>0) ch->ver3.subclass_points+=point;
    token_number=TOKENCOUNT-obj_proto_table[real_object(5)].number;
	/*log_f("Redistributing %d of %d tokens", token_number, TOKENCOUNT);*/
    while(token_number>0) {
      if(distribute_token()) token_number--;
      loop++;
      if(loop>1000) {
        log_f("Breaking loop distribute tokens");
        break;
      }
    }
    distribute_token();
    return TRUE;
  }
  return FALSE;
}


#define TOKEN_MOB 11
#define MAX_RATINGS 120 /* Currently 115 zones */
int token_mob_time;
extern int top_of_world;

struct zone_token_rating
{
    int virtual;          /* virtual index for this zone */
    int rating;           /* rating - 20 unsafe, 80 safe */
};

struct zone_token_rating zrate[MAX_RATINGS];
int top_zone_rating=0;
extern int top_of_zone_table;
extern struct zone_data *zone_table;

void reset_zone_rating(void) {
  int i;
  top_zone_rating=0;
  for(i=0; i<=top_of_zone_table; i++) {
    top_zone_rating++;
    zrate[i].virtual=zone_table[i].virtual;
    zrate[i].rating=50;
  }
}

void write_zone_rating(void) {
  int i;
  FILE *fl;

  if(!(fl=fopen("zone_rating","w"))) {
     log_f("Error opening zone rating file.");
     return;
  }

  for (i=0; i<top_zone_rating; i++) {
    fprintf(fl,"%d %d\n",zrate[i].virtual,zrate[i].rating);
  }
  fclose(fl);
}

void read_zone_rating(void) {
  int i=-1;
  FILE *fl;

  if(!(fl=fopen("zone_rating","r"))) {
    log_f("Error opening zone rating file.");
    reset_zone_rating();
    write_zone_rating();
    return;
  }

  while(!feof(fl)) {
    i++;
    if(EOF == fscanf(fl,"%d %d\n",&zrate[i].virtual,&zrate[i].rating)) continue;
    if(i>0 && zrate[i].virtual>0) top_zone_rating=i+1;
    if(i>=MAX_RATINGS) break;
  }
  fclose(fl);
}

void do_zrate(CHAR *ch, char *argument, int cmd) {
  int i;
  char buf[MAX_INPUT_LENGTH];
  char usage[]="\
Usage: zrate list\n\r\
             reset\n\r\
             read\n\r\
             save\n\r\n\r\
These commands should not be necessary during normal operation.\n\r.";

  if(!check_god_access(ch,1)) return;
  argument=one_argument(argument, buf);
  if(!*buf) {
    send_to_char(usage,ch);
    return;
  }
  if(!strcmp(buf,"reset")) {
    reset_zone_rating();
    send_to_char("All zone ratings reset.\n\r'zrate save' to save.\n\r'zrate read' to ignore reset.",ch);
    return;
  }
  if(!strcmp(buf,"read")) {
    read_zone_rating();
    send_to_char("All zone ratings reread.\n\r",ch);
    return;
  }
  if(!strcmp(buf,"save")) {
    write_zone_rating();
    send_to_char("All zone ratings saved.\n\r",ch);
    return;
  }
  if(!strcmp(buf,"list")) {
    send_to_char("Vnum Rating Name\n\r==================================================================\n\r",ch);
    for (i=0; i<top_zone_rating; i++) {
      printf_to_char(ch," %3d  %3d    %s\n\r",zrate[i].virtual,zrate[i].rating,zone_table[real_zone(zrate[i].virtual)].name);
    }
    send_to_char("==================================================================\n\r",ch);
    return;
  }
  send_to_char(usage,ch);
}

void zone_rate(int zone,int change) {
  int i;
  for (i=0; i<top_zone_rating; i++) {
    if(zone==zrate[i].virtual) {
      zrate[i].rating+=change;
      if(zrate[i].rating>80) zrate[i].rating=50;
      if(zrate[i].rating<20) zrate[i].rating=20;
    }
  }
}

int zone_rating(int zone) {
  int i,rating=100;
  for (i=0; i<top_zone_rating; i++) {
    if(zone==zrate[i].virtual) {
      rating=zrate[i].rating;
      break;
    }
  }
  return rating;
}

void initialize_token_mob() {
#ifndef TEST_SITE
  if(CHAOSMODE) return;
  token_mob_time=number(40,80);
#endif
  return;
}

static int token_mob_target_room() {
  int goto_room = 0;

#ifndef TEST_SITE

  int zone = 0;

#endif

  while(goto_room==0) {
    goto_room=number(1,top_of_world);

#ifndef TEST_SITE

    zone=inzone(world[goto_room].number);

    if(zone==275 ||
       zone==300 ||
       zone==39  ||
       zone==36  ||
       zone==35  ||
       zone==30  ||
       zone==31  ||
       zone==58  ||
       zone==0   ||
       zone==12  ||
       zone==253 ||
       zone==254 ||
       zone==255 ||
       zone==51  ||
       zone==55  ||
       zone==59  ||
       zone==285 ||
       zone==286 ||
       zone==287 ||
       zone==261 ||
       zone==260 ||
       zone==278 ||
       zone==66  ||
       zone==10  ||
       zone==262 ||
       IS_SET(world[goto_room].room_flags, SAFE) ||
       IS_SET(world[goto_room].room_flags, PRIVATE) ||
       IS_SET(world[goto_room].room_flags, DEATH) ||
       IS_SET(world[goto_room].room_flags, HAZARD))
      goto_room=0;

    if(number(1,100)>zone_rating(zone)) /* zone rating check */
      goto_room=0;

#endif

  }

  return goto_room;
}

void check_token_mob() {
#ifndef TEST_SITE
  CHAR *mob;
  OBJ *obj;
  int goto_room=0;
  if(CHAOSMODE) return;
  if(token_mob_time>0) token_mob_time--;
  if(token_mob_time<=0) {
    token_mob_time=0;
    if(mob_proto_table[real_mobile(TOKEN_MOB)].number>0) return;
    if(!(mob=read_mobile(TOKEN_MOB,VIRTUAL))) return;
    if(!(obj=read_object(5,VIRTUAL))) return;

    goto_room = token_mob_target_room();
    char_to_room(mob,goto_room);
    obj_to_char(obj,mob);
    log_f("SUBLOG: Token mob placed in room %d.",world[goto_room].number);

    switch(number(0,3)) {
      case 1:
        do_yell(mob,"Death to all that oppose me!",CMD_YELL);
        break;
      case 2:
        do_yell(mob,"I have returned!",CMD_YELL);
        break;
      case 3:
        do_yell(mob, "Yo, whassup?!", CMD_YELL);
        break;
      default:
        do_yell(mob,"Death to you all!",CMD_YELL);
        break;
    }
  }
#endif
}
int inzone(int number);

int do_roomyell(CHAR* mob)
{
  char buf[MSL];

  /* yell something about the room name */
  switch(number(8,21)) {
    case 8:
      sprintf(buf,"Come to %s, I want to spill your blood.",world[CHAR_REAL_ROOM(mob)].name);break;
    case 9:
      sprintf(buf,"At least %s isn't as boring as Midgaard.",world[CHAR_REAL_ROOM(mob)].name);break;
    case 10:
      sprintf(buf,"I've been to exciting places.  %s isn't one of em.",world[CHAR_REAL_ROOM(mob)].name);break;
    case 11:
      sprintf(buf,"I went to %s and all I got was this t-shirt!",world[CHAR_REAL_ROOM(mob)].name);break;
    case 12:
      sprintf(buf,"Anyone know of a good chinese restaurant near %s?", world[CHAR_REAL_ROOM(mob)].name);break;
    case 13:
      sprintf(buf,"You know what %s needs? More cowbell!", world[CHAR_REAL_ROOM(mob)].name);break;
    case 14:
      sprintf(buf,"%s looks like the moon landing set!", world[CHAR_REAL_ROOM(mob)].name);break;
    case 15:
      sprintf(buf,"Hey Eva! I found a good, long pole at %s!", world[CHAR_REAL_ROOM(mob)].name);break;
    case 16:
      sprintf(buf,"Anyone know of some fun things to do at %s?", world[CHAR_REAL_ROOM(mob)].name);break;
    case 17:
      sprintf(buf,"I'm at %s. Come and get me!",world[CHAR_REAL_ROOM(mob)].name);break;
    case 18:
      sprintf(buf,"%s is more lively than Immortal Reception!", world[CHAR_REAL_ROOM(mob)].name);break;
    case 19:
      sprintf(buf,"Does anyone have dirs from %s to Funland?", world[CHAR_REAL_ROOM(mob)].name);break;
    case 20:
      sprintf(buf,"%s looks like a good place to idle!", world[CHAR_REAL_ROOM(mob)].name);break;
    default:
      sprintf(buf,"Why am I at %s and why am I in this handbasket?", world[CHAR_REAL_ROOM(mob)].name);break;
  }

  do_yell(mob,buf,CMD_YELL);
  return FALSE;
}

int token_mob(CHAR *mob,CHAR *ch, int cmd, char *argument) {
  struct descriptor_data *d;
  char buf[MSL];
  int zone,door,goto_room=0,count=0;
  char *dirs[] = {"north","east","south","west","up","down","\n"};

  if(cmd==MSG_DIE) {
    token_mob_time=number(50,80);
    zone=inzone(CHAR_VIRTUAL_ROOM(mob));
    if(mob->specials.timer<5) {
      zone_rate(zone,-5);
    }
    else if(mob->specials.timer<10) {
      zone_rate(zone,-3);
    }

    write_zone_rating();
    if(!ch || ch == mob) return FALSE;
    if (IS_NPC(ch))
      sprintf(buf,"AHHHHHHHH, I curse you %s!",GET_SHORT(ch));
    else
      sprintf(buf,"AHHHHHHHH, I curse you %s!",GET_NAME(ch));
    do_yell(mob,buf,CMD_YELL);
    spell_curse(GET_LEVEL(mob),mob,ch,0);
    return FALSE;
  }

  if(cmd==MSG_TICK) { /* Beam away if still alive at 15 ticks */
    mob->specials.timer++;
    if (mob->specials.timer == 1)
      do_roomyell(mob);

    if(mob->specials.timer==8) {
      zone=inzone(CHAR_VIRTUAL_ROOM(mob));
      zone_rate(zone,1);
    }

    if(mob->specials.timer>=15 && !mob->specials.fighting) {
      /* if morts in the zone, don't beam */
      if(count_mortals_zone(mob,TRUE)) return FALSE;

      goto_room = token_mob_target_room();

      switch (number(1,4)) {
        case 1:
          do_yell(mob,"I don't like this place, I'm moving!",CMD_YELL); break;
        case 2:
          do_yell(mob,"Time to find some new real estate!", CMD_YELL); break;
        case 3:
          do_yell(mob, "I'm still looking for that elusive mudder!", CMD_YELL); break;
        case 4:
          do_yell(mob, "Well, if no one's here, I'm moving on!", CMD_YELL); break;
      }

      act("$n disappears in a puff of smoke.",0,mob,0,0,TO_ROOM);
      zone=inzone(CHAR_VIRTUAL_ROOM(mob));
      zone_rate(zone,2);
      char_from_room(mob);
      char_to_room(mob,goto_room);
      mob->specials.timer=0;
    }
  }

  /* movement */
  if(cmd==MSG_MOBACT) {
    /* open or close all surrounding doors */
    if(chance(10)) {
      for (door = 0; door <= 5; door++) {
        if (!EXIT(mob, door)) continue;
        if(!IS_SET(EXIT(mob, door)->exit_info, EX_ISDOOR)) continue;
        if(!EXIT(mob, door)->keyword) continue;
        sprintf(buf," %s %s",EXIT(mob,door)->keyword,dirs[door]);
        if(IS_SET(EXIT(mob, door)->exit_info, EX_CLOSED)) {
          do_open(mob,buf,CMD_OPEN);
        }
        else {
          do_close(mob,buf,CMD_CLOSE);
        }
      }
    }

    /* move around quickly */
    door=number(0,5);
    if(chance(30) && GET_MOVE(mob)>0 &&
       !mob->specials.fighting &&
       (GET_POS(mob)==POSITION_STANDING ||
        GET_POS(mob)==POSITION_RIDING ||
        GET_POS(mob)==POSITION_FLYING) &&
       CAN_GO(mob,door) &&
       !IS_SET(world[EXIT(mob, door)->to_room_r].room_flags, SAFE) &&
       !IS_SET(world[EXIT(mob, door)->to_room_r].room_flags, DEATH) &&
       !IS_SET(world[EXIT(mob, door)->to_room_r].room_flags, HAZARD) &&
       !IS_AFFECTED(mob, AFF_HOLD)) {
      if (mob->specials.last_direction==door) {
        mob->specials.last_direction=-1;
      }
      else {
        mob->specials.last_direction=door;
        do_move(mob,"",++door);
      }
    }
    if(chance(1))
        return do_roomyell(mob);

    /* taunt players */
    if(chance(1)) {
      for(d=descriptor_list; d; d=d->next)
        if(!d->connected && CAN_SEE(mob, d->character) &&
           GET_LEVEL(d->character)<LEVEL_IMM && GET_LEVEL(d->character)>24) count++;
      door=number(1,count);
      count=0;

      for(d = descriptor_list; d; d = d->next) {
        if(!d->connected && CAN_SEE(mob, d->character) &&
           GET_LEVEL(d->character) <LEVEL_IMM && GET_LEVEL(d->character)>24 && count++==door) {
            switch(number(0,48)) {
              case 0:
                sprintf(buf,"I'd like to get to know you, %s!", GET_NAME(d->character));break;
              case 1:
                sprintf(buf,"Am I annoying you yet %s?",GET_NAME(d->character));break;
              case 2:
                sprintf(buf,"You talking to me, %s?!...     You talking to ME?!!",GET_NAME(d->character));break;
              case 3:
                sprintf(buf,"I know what you're thinking. Did he fire 6 shots or only 5?");break;
              case 4:
                sprintf(buf,"Help! Help Help!  Save me %s, help!",GET_NAME(d->character));break;
              case 5:
                sprintf(buf,"One, two, buckle my shoe!");break;
              case 6:
                sprintf(buf,"Thou art a forward fat-kidneyed lewdster, %s!",GET_NAME(d->character));break;
              case 7:
                sprintf(buf,"%s couldn't hit the broad side of a barn, let alone me!",GET_NAME(d->character));break;
              case 8:
                sprintf(buf,"Hey %s, some people have called you a wit... They're half right!",GET_NAME(d->character));break;
              case 9:
                sprintf(buf,"Row, row, row your boat, gently down the stream!");break;
              case 10:
                sprintf(buf,"Buy you a beer, %s?", GET_NAME(d->character));break;
              case 11:
                sprintf(buf,"%s is so dense that light bends around %s!",GET_NAME(d->character),HMHR(d->character));break;
              case 12:
                sprintf(buf,"I would like to insult you, %s, but with your intelligence you wouldn't get offended.",GET_NAME(d->character));break;
              case 13:
                sprintf(buf,"Oooh, %s you're such a pansy!",GET_NAME(d->character));break;
              case 14:
                sprintf(buf,"Hey %s, you're proof even gods make mistakes!",GET_NAME(d->character));break;
              case 15:
                sprintf(buf,"You killed my father.  Prepare to die!");break;
              case 16:
                sprintf(buf,"Why you stuck-up, halfwitted, scruffy looking nurf herder!");break;
              case 17:
                sprintf(buf,"Of course you know this means war.");break;
              case 18:
                sprintf(buf,"Your head is as empty as a eunuch's underpants.");break;
              case 19:
                sprintf(buf,"I love the smell of napalm in the morning!");break;
              case 20:
                sprintf(buf,"Come here so I can get medieval on your ass!");break;
              case 21:
                sprintf(buf,"Kill me, God dangit!");break;
              case 22:
                sprintf(buf,"Screw you, hippie!");break;
              case 23:
                sprintf(buf,"I don't wanna die! I don't wanna die! I don't wanna die!");break;
              case 24:
                sprintf(buf,"Say your prayers, you heathen baboons!");break;
              case 25:
                sprintf(buf,"What do you do for an encore, fall asleep?");break;
              case 26:
                sprintf(buf,"I know nooooothing!");break;
              case 27:
                sprintf(buf,"Hemp for IMP!");break;
              case 28:
                sprintf(buf,"What are you gonna do, bleed on me?");break;
              case 29:
                sprintf(buf,"I'm invincible!");break;
              case 30:
                sprintf(buf,"OK, everybody get in a line so I can whup all your asses!");break;
              case 31:
                sprintf(buf,"This space for rent!");break;
              case 32:
                sprintf(buf,"Hey who wants some of this?");break;
              case 33:
                sprintf(buf,"Want a subclass?  Can't have it..");break;
              case 34:
                sprintf(buf,"Come on baby light my fire!");break;
              case 35:
                sprintf(buf,"Want my tokens? Come and get 'em!");break;
              case 36:
                sprintf(buf,"I can't wait to open up a can of whoop ass on someone!");break;
              case 37:
                sprintf(buf,"%s for IMP!",GET_NAME(d->character));break;
              case 38:
                sprintf(buf,"Dying ain't much of a living, boy.");break;
              case 39:
                sprintf(buf,"Don't make me angry, %s.  You wouldn't like me when I'm angry.",GET_NAME(d->character));break;
              case 40:
                sprintf(buf,"Yo %s!  Remind me to whup your ass good next time I see you!",GET_NAME(d->character));break;
              case 41:
                sprintf(buf,"Be prepared to reap the whirlwind, gentlemen.");break;
              case 43:
                sprintf(buf,"Live Free or Die!");break;
              case 44:
                sprintf(buf,"FREEEEEEEDDDDDOOOOMMMMMM!!!!!");break;
              case 45:
                sprintf(buf,"I am as constant as the northern star!");break;
              case 46:
                sprintf(buf,"O death, where is thy sting?  O grave, where is thy victory?");break;
              case 47:
                sprintf(buf,"I shall break thee with a rod of iron, %s; I shall dash thee in pieces like a potter's vessel!",GET_NAME(d->character));break;
              case 48:
                sprintf(buf,"%s is a complete wuss!",GET_NAME(d->character));break;
              default:
                sprintf(buf,"Can't catch me, I'm the gingerbreadman!");break;
            }
          do_yell(mob,buf,CMD_YELL);
          return FALSE;
        }
      }
    }
    return FALSE;
  } /* Mob_act */

  return FALSE;
}

void assign_subclass_master(void) {
  assign_mob(1915,  subclass_master);
  assign_mob(2104,  subclass_master);
  assign_mob(7322,  subclass_master);
  assign_mob(12913, subclass_master);
  assign_mob(9640,  subclass_master);
  assign_mob(28506, subclass_master); /* Pan made Bard master */
  assign_mob(11096, subclass_master);
  assign_mob(2803,  subclass_master);
  assign_mob(9012,  subclass_master);
  assign_mob(2107,  subclass_master);
}

void assign_subclass(void) {
  assign_obj(STATUE_PEACE,statue_peace);
  assign_obj(WALL_THORNS,wall_thorns);
  assign_subclass_master();
  assign_mob(TOKEN_MOB,token_mob);
}

