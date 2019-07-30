/*
** subclass.c
**   Routines for subclasses.
**
** Written for RoninMUD by Ranger
**
** Do not distribute without permission.
*/

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
#include "mob.spells.h"
#include "subclass.h"
#include "enchant.h"

int check_subclass(CHAR *ch,int sub,int lvl);
int check_god_access(CHAR *ch, int active);
bool distribute_token(void);
int guild(CHAR *mob, CHAR *ch, int cmd, char *arg);

int check_sc_access(CHAR *ch, int skill) {
  if (!ch) return FALSE;

  if (IS_NPC(ch) || IS_IMMORTAL(ch)) return TRUE;

  if ((skill == SPELL_DIVINE_WIND) && (GET_CLASS(ch) == CLASS_NINJA)) return TRUE;
  if ((skill == SPELL_REJUVENATION) && (GET_CLASS(ch) == CLASS_BARD)) return TRUE;
  if ((skill == SPELL_RAGE) && (GET_CLASS(ch) == CLASS_ANTI_PALADIN)) return TRUE;
  if ((skill == SPELL_IRON_SKIN) && (GET_CLASS(ch) == CLASS_COMMANDO)) return TRUE;

  if (skill <= 165) return TRUE;

  switch (skill) {
    /* Enchanter */
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

      /* Archmage */
    case SPELL_METEOR:
      if (check_subclass(ch, SC_ARCHMAGE, 1)) return TRUE;
      break;
    case SPELL_ORB_PROTECTION:
      if (check_subclass(ch, SC_ARCHMAGE, 2)) return TRUE;
      if (check_subclass(ch, SC_TEMPLAR, 4)) return TRUE;
      break;
    case SPELL_FROSTBOLT:
      if (check_subclass(ch, SC_ARCHMAGE, 3)) return TRUE;
      break;
    case SPELL_WRATH_ANCIENTS:
      if (check_subclass(ch, SC_ARCHMAGE, 4)) return TRUE;
      break;
    case SPELL_DISTORTION:
      if (check_subclass(ch, SC_ARCHMAGE, 5)) return TRUE;
      break;

    /* Druid */
    case SKILL_MEDITATE:
      if (check_subclass(ch, SC_DRUID, 1)) return TRUE;
      if (check_subclass(ch, SC_TEMPLAR, 1)) return TRUE;
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

    /* Templar */
    case SPELL_SANCTIFY:
      if (check_subclass(ch, SC_TEMPLAR, 3)) return TRUE;
      break;
    case SPELL_FORTIFICATION:
      if (check_subclass(ch, SC_TEMPLAR, 5)) return TRUE;
      break;

    /* Rogue */
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

    /* Bandit */
    case SKILL_EVASION:
      if (check_subclass(ch, SC_BANDIT, 5)) return TRUE;
      if ((GET_CLASS(ch) == CLASS_NOMAD) && (GET_LEVEL(ch) >= 50)) return TRUE;
      break;

    /* Warlord */
    case SKILL_AWARENESS:
      if (check_subclass(ch, SC_WARLORD, 1)) return TRUE;
      if (check_subclass(ch, SC_RANGER, 1)) return TRUE;
      break;
    case SKILL_PROTECT:
      if (check_subclass(ch, SC_WARLORD, 2)) return TRUE;
      if (check_subclass(ch, SC_RANGER, 2)) return TRUE;
      break;

    /* Gladiator */
    case SKILL_FLANK:
      if (check_subclass(ch, SC_GLADIATOR, 1)) return TRUE;
      break;
    case SKILL_MAIM:
      if (check_subclass(ch, SC_GLADIATOR, 3)) return TRUE;
      break;
    case SKILL_HEADBUTT:
      if (check_subclass(ch, SC_GLADIATOR, 4)) return TRUE;
      break;
    case SKILL_HOSTILE:
      if (check_subclass(ch, SC_GLADIATOR, 5)) return TRUE;
      break;

    /* Ronin */
    case SPELL_BLUR:
      if (check_subclass(ch, SC_RONIN, 1)) return TRUE;
      break;
    case SKILL_BACKFIST:
      if (check_subclass(ch, SC_RONIN, 2)) return TRUE;
      break;
    case SKILL_BANZAI:
      if (check_subclass(ch, SC_RONIN, 4)) return TRUE;
      break;

    /* Mystic */
    case SPELL_DEBILITATE:
      if (check_subclass(ch, SC_MYSTIC, 1)) return TRUE;
      break;
    case SKILL_TIGERKICK:
      if (check_subclass(ch, SC_MYSTIC, 3)) return TRUE;
      break;
    case SKILL_MANTRA:
      if (check_subclass(ch, SC_MYSTIC, 4)) return TRUE;
      break;
    case SPELL_TRANQUILITY:
      if (check_subclass(ch, SC_MYSTIC, 5)) return TRUE;
      break;

    /* Ranger */
    case SKILL_BERSERK:
      if (check_subclass(ch, SC_RANGER, 4)) return TRUE;
      break;
    case SKILL_DEFEND:
      if (check_subclass(ch, SC_RANGER, 5)) return TRUE;
      break;

    /* Trapper */
    case SKILL_CAMP:
      if (check_subclass(ch, SC_TRAPPER, 2)) return TRUE;
      if ((GET_CLASS(ch) == CLASS_BARD) && (GET_LEVEL(ch) >= 35)) return TRUE;
      break;
    case SKILL_BATTER:
      if (check_subclass(ch, SC_TRAPPER, 3)) return TRUE;
      break;
    case SKILL_FRENZY:
      if (check_subclass(ch, SC_TRAPPER, 5)) return TRUE;
      break;

    /* Cavalier */
    case SPELL_MIGHT:
      if (check_subclass(ch, SC_CAVALIER, 1)) return TRUE;
      if (check_subclass(ch, SC_TEMPLAR, 2)) return TRUE;
      break;
    case SKILL_TRUSTY_STEED:
      if (check_subclass(ch, SC_CAVALIER, 2)) return TRUE;
      break;
    case SPELL_WRATH_OF_GOD:
      if (check_subclass(ch, SC_CAVALIER, 3)) return TRUE;
      break;
    case SPELL_POWER_OF_DEVOTION:
      if (check_subclass(ch, SC_CAVALIER, 4)) return TRUE;
      break;
    case SKILL_SMITE:
      if (check_subclass(ch, SC_CAVALIER, 5)) return TRUE;
      break;

    /* Crusader */
    case SPELL_RIGHTEOUSNESS:
      if (check_subclass(ch, SC_CRUSADER, 1)) return TRUE;
      break;
    case SKILL_ZEAL:
      if (check_subclass(ch, SC_CRUSADER, 4)) return TRUE;
      break;
    case SPELL_POWER_OF_FAITH:
      if (check_subclass(ch, SC_CRUSADER, 5)) return TRUE;
      break;

    /* Defiler */
    case SPELL_DESECRATE:
      if (check_subclass(ch, SC_DEFILER, 1)) return TRUE;
      break;
    case SPELL_BLACKMANTLE:
      if (check_subclass(ch, SC_DEFILER, 2)) return TRUE;
      break;
    case SKILL_FEINT:
      if (check_subclass(ch, SC_DEFILER, 3)) return TRUE;
      break;
    case SKILL_SHADOWSTEP:
      if (check_subclass(ch, SC_DEFILER, 4)) return TRUE;
      break;

    /* Infidel */
    case SKILL_VICTIMIZE:
      if (check_subclass(ch, SC_INFIDEL, 2)) return TRUE;
      break;
    case SPELL_SHADOW_WRAITH:
      if (check_subclass(ch, SC_INFIDEL, 3)) return TRUE;
      break;
    case SPELL_DUSK_REQUIEM:
      if (check_subclass(ch, SC_INFIDEL, 4)) return TRUE;
      break;
    case SPELL_WITHER:
      if (check_subclass(ch, SC_INFIDEL, 5)) return TRUE;
      break;

    /* Blade Singer */
    case SKILL_BLITZ:
      if (check_subclass(ch, SC_BLADESINGER, 2)) return TRUE;
      break;

    /* Legionnaire */
    case SKILL_LUNGE:
      if (check_subclass(ch, SC_LEGIONNAIRE, 1)) return TRUE;
      break;
    case SPELL_RIMEFANG:
      if (check_subclass(ch, SC_LEGIONNAIRE, 2)) return TRUE;
      break;
    case SKILL_CLOBBER:
      if (check_subclass(ch, SC_LEGIONNAIRE, 3)) return TRUE;
      break;
    case SPELL_DEVASTATION:
      if (check_subclass(ch, SC_LEGIONNAIRE, 4)) return TRUE;
      break;
    case SKILL_SNIPE:
      if (check_subclass(ch, SC_LEGIONNAIRE, 5)) return TRUE;
      break;

    /* Mercenary */
    case SPELL_TREMOR:
      if (check_subclass(ch, SC_MERCENARY, 1)) return TRUE;
      break;
    case SPELL_CLOUD_CONFUSION:
      if (check_subclass(ch, SC_MERCENARY, 2)) return TRUE;
      break;
    case SKILL_RIPOSTE:
      if (check_subclass(ch, SC_MERCENARY, 3)) return TRUE;
      break;
    case SPELL_INCENDIARY_CLOUD_NEW:
      if (check_subclass(ch, SC_MERCENARY, 4)) return TRUE;
      break;
  }

  return FALSE;
}

int check_sc_song_access(CHAR *ch, int song) {
  if (!ch) return FALSE;

  if (IS_NPC(ch) || IS_IMMORTAL(ch)) return TRUE;

  /* Note: Song 22 is Rejuvenation. */
  if ((song <= 17) || (song == 22)) return TRUE;

  switch (song) {
    case 18: /* song of remove poison */
      if (check_subclass(ch, SC_CHANTER, 1)) return TRUE;
      break;
    case 19: /* rally song */
      if (check_subclass(ch, SC_BLADESINGER, 1)) return TRUE;
      break;
    case 20: /* warchant */
      if (check_subclass(ch, SC_CHANTER, 2)) return TRUE;
      break;
    case 21: /* song of luck */
      if (check_subclass(ch, SC_CHANTER, 3)) return TRUE;
      break;
    case 23: /* aid */
      if (check_subclass(ch, SC_CHANTER, 4)) return TRUE;
      break;
    case 24: /* protection vs. evil/good */
      if (check_subclass(ch, SC_BLADESINGER, 4)) return TRUE;
      break;
    case 25: /* thunderball */
      if (check_subclass(ch, SC_BLADESINGER, 5)) return TRUE;
      break;
    case 26: /* sphere */
      if (check_subclass(ch, SC_CHANTER, 5)) return TRUE;
      break;
  }

  return FALSE;
}

int check_subclass(CHAR *ch, int sc, int level) {
  if (!ch) return FALSE;

  if (IS_NPC(ch) || IS_IMMORTAL(ch)) return TRUE;

  if ((GET_LEVEL(ch) < 30) ||
      ((GET_LEVEL(ch) < 40) && (level > 2)) ||
      ((GET_LEVEL(ch) < 45) && (level > 4))) return FALSE;

  if ((sc < ((2 * GET_CLASS(ch)) - 1)) || (sc > (2 * GET_CLASS(ch)))) return FALSE;

  if ((GET_SC(ch) == sc) && (GET_SC_LEVEL(ch) >= level)) return TRUE;

  return FALSE;
}

int set_subclass(CHAR *ch, int sc, int level) {
  if (!ch || IS_NPC(ch)) return FALSE;

  if ((GET_LEVEL(ch) < 30) ||
      ((GET_LEVEL(ch) < 40) && (level > 2)) ||
      ((GET_LEVEL(ch) < 45) && (level > 4))) return FALSE;

  if ((sc < ((2 * GET_CLASS(ch)) - 1)) || (sc > (2 * GET_CLASS(ch)))) return FALSE;

  GET_SC(ch) = sc;
  GET_SC_LEVEL(ch) = level;

  return TRUE;
}

void remove_subclass(CHAR *ch) {
  GET_SC(ch) = 0;
  GET_SC_LEVEL(ch) = 0;
}

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
    subclass = new_search_block(buf, subclass_name, TRUE, FALSE);
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
    if(!set_subclass(vict,subclass+1,level)) {
      send_to_char("Subclass assignment failed.\n\r",ch);
      return;
    }
    sprintf(buf,"WIZINFO: %s set %s to subclass %s, level %d.",GET_NAME(ch),GET_NAME(vict),subclass_name[subclass],level);
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
  if (cmd == MSG_TICK) {
    OBJ_SPEC(obj) -= 1;

    if (OBJ_SPEC(obj) <= 0) {
      printf_to_room(OBJ_REAL_ROOM(obj), "The statue of peace crumbles to dust.\n\r");

      REMOVE_BIT(ROOM_FLAGS(OBJ_REAL_ROOM(obj)), NO_MOB);

      extract_obj(obj);

      return FALSE;
    }

    return FALSE;
  }

  return FALSE;
}

#define WALL_THORNS 34
int wall_thorns(OBJ *obj, CHAR *ch, int cmd, char *argument) {
  if (cmd == MSG_TICK) {
    OBJ_SPEC(obj) -= 1;

    if (OBJ_SPEC(obj) <= 0) {
      printf_to_room(OBJ_REAL_ROOM(obj), "The wall of thorns slowly wilts and disappears.\n\r");

      REMOVE_BIT(ROOM_FLAGS(OBJ_REAL_ROOM(obj)), NO_MOB);

      extract_obj(obj);

      return FALSE;
    }

    return FALSE;
  }

  return FALSE;
}

int check_sc_master(CHAR *ch, CHAR *mob) {
  if (!mob || !ch || !IS_NPC(mob) || !IS_MORTAL(ch) ||
    ((GET_CLASS(ch) < CLASS_FIRST) && (GET_CLASS(ch) > CLASS_LAST)) ||
    (NUMELEMS(subclass_master) < GET_CLASS(ch))) return FALSE;

  if (V_MOB(mob) == subclass_master[GET_CLASS(ch) - 1]) return TRUE;

  return FALSE;
}

int sc_master(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd == CMD_PRACTICE) {
    return guild(mob, ch, cmd, arg);
  }

  if (cmd == MSG_SAID) {
    char buf[MIL];

    arg = one_argument(arg, buf);

    if (!*buf || (strcasecmp(buf, "choose") != 0)) return FALSE;

    if (!check_sc_master(ch, mob)) {
      comm_special(mob, ch, COMM_TELL, "Go away!  I don't deal with your type!");

      return FALSE;
    }

    one_argument(arg, buf);

    if (!*buf) {
      comm_special(mob, ch, COMM_TELL, "Which subclass path did you want to choose?  Your choices are: %s or %s.",
        subclass_name[(2 * GET_CLASS(ch)) - 2], subclass_name[(2 * GET_CLASS(ch)) - 1]);

      return FALSE;
    }

    int sc_idx = get_index_of_string_in_list(buf, subclass_name, TRUE, FALSE);

    if ((sc_idx < ((2 * GET_CLASS(ch)) - 2)) || (sc_idx > ((2 * GET_CLASS(ch)) - 1))) {
      comm_special(mob, ch, COMM_TELL, "That's not a valid subclass.  Your choices are: %s or %s.",
        subclass_name[(2 * GET_CLASS(ch)) - 2], subclass_name[(2 * GET_CLASS(ch)) - 1]);

      return FALSE;
    }

    if (GET_SC(ch) && (GET_SC(ch) != (sc_idx + 1))) {
      comm_special(mob, ch, COMM_TELL, "Go see Immortalis about remorting if you want to change your subclass.");

      return FALSE;
    }

    if ((GET_LEVEL(ch) < 30) ||
        ((GET_LEVEL(ch) < 40) && ((GET_SC_LEVEL(ch) + 1) > 2)) ||
        ((GET_LEVEL(ch) < 45) && ((GET_SC_LEVEL(ch) + 1) > 4))) {
      comm_special(mob, ch, COMM_TELL, "Come back and see me after you've gained a few class levels.");

      return FALSE;
    }

    if (GET_SC_LEVEL(ch) >= 5) {
      comm_special(mob, ch, COMM_TELL, "You already have the highest possible subclass level.");

      return FALSE;
    }

    int scp_required = (GET_SC_LEVEL(ch) * 70) + 70;

    if (GET_SCP(ch) < scp_required) {
      comm_special(mob, ch, COMM_TELL, "You do not have enough subclass points.");

      return FALSE;
    }

    if (GET_SC_LEVEL(ch) <= 0) {
      printf_to_world("%s shouts '%s has joined the path of the %s!  All bow before %s might!'\n\r",
        GET_SHORT(mob), GET_NAME(ch), subclass_name[sc_idx], HSHR(ch));

      GET_SC(ch) = sc_idx + 1;
    }
    else {
      printf_to_world("%s shouts '%s has taken another step along the path of the %s!'\n\r",
        GET_SHORT(mob), GET_NAME(ch), subclass_name[sc_idx]);
    }

    GET_SCP(ch) -= scp_required;
    GET_SC_LEVEL(ch) += 1;

    save_char(ch, NOWHERE);

    return FALSE;
  }

  if (cmd == MSG_OBJ_GIVEN) {
    if (!isname("token", arg)) return FALSE;

    OBJ *obj = get_obj_in_list_ex(mob, "token", mob->carrying, FALSE);

    if (!obj) return TRUE;

    bool give_back = FALSE;

    if (!check_sc_master(ch, mob)) {
      comm_special(mob, ch, COMM_TELL, "Go away!  I don't deal with your type!");

      give_back = TRUE;
    }

    if (OBJ_TYPE(obj) != ITEM_SC_TOKEN) {
      comm_special(mob, ch, COMM_TELL, "That wasn't a subclass token.");

      give_back = TRUE;
    }

    if (give_back) {
      comm_special(mob, ch, COMM_TELL, "Here, take it back.");

      act("$N gives you $p.", FALSE, ch, obj, mob, TO_CHAR);

      obj_from_char(obj);
      obj_to_char(obj, ch);

      return TRUE;
    }

    int token_scp = OBJ_VALUE(obj, 0);

    // Prestige Perk 12
    if ((GET_PRESTIGE_PERK(ch) >= 12) && (token_scp == 1) && chance(10)) {
      token_scp = 2;
    }

    if (token_scp <= 0) {
      comm_special(mob, ch, COMM_TELL, "Sorry, that token is worthless.");
    }
    else {
      comm_special(mob, ch, COMM_TELL, "Thank you.  You get %d subclass point%s for that token.",
        token_scp, ((abs(token_scp) == 1) ? "" : "s"));

      GET_SCP(ch) += token_scp;
    }

    log_f("SUBLOG: %s turns in a token.", GET_NAME(ch));

    extract_obj(obj);

    distribute_tokens(CHAOSMODE ? 0 : (TOKENCOUNT - obj_proto_table[real_object(5)].number));

    return TRUE;
  }

  return FALSE;
}

#define TOKEN_MOB 11
#define MAX_RATINGS 120 /* Currently 115 zones */
int token_mob_time;

struct zone_token_rating
{
    int virtual;          /* virtual index for this zone */
    int rating;           /* rating - 20 unsafe, 80 safe */
};

struct zone_token_rating zrate[MAX_RATINGS];
int top_zone_rating=0;

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
  token_mob_time = number(40,80);
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
                sprintf(buf,"You talking to me, %s?!... You talking to ME?!!",GET_NAME(d->character));break;
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


void assign_subclass(void) {
  assign_obj(STATUE_PEACE, statue_peace);
  assign_obj(WALL_THORNS, wall_thorns);

  assign_mob(TOKEN_MOB, token_mob);

  for (int i = 0; i < NUMELEMS(subclass_master); i++) {
    if (subclass_master[i] > 0) {
      assign_mob(subclass_master[i], sc_master);
    }
  }
}

