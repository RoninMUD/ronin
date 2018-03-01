/*
  score.c - Improved do_score() and related functions

  Written by Alan K. Miles for RoninMUD
  Last Modification Date: 12/06/2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include "structs.h"
#include "constants.h"
#include "comm.h"
#include "db.h"
#include "fight.h"
#include "handler.h"
#include "interpreter.h"
#include "limits.h"
#include "spec.clan.h"
#include "spells.h"
#include "utility.h"
#include "utils.h"
#include "enchant.h"
#include "remortv2.h"
#include "score.h"
#include "cmd.h"

extern char *skip_spaces(char *string);
extern int hit_gain(CHAR *ch);
extern int mana_gain(CHAR *ch);
extern int move_gain(CHAR *ch);
extern int check_subclass(CHAR *ch, int sub, int lvl);

void score_query(CHAR *ch, int query, bool opt_text, bool new_line)
{
  char buf[MSL];
  char buf2[MSL];

  sprintf(buf, "%s", "");
  sprintf(buf2, "%s", "");

  switch (query)
  {
  case SCQ_NAME:
    sprintf(buf, "%sName:%s %s",
      CHCLR(ch, 7), ENDCHCLR(ch),
      !IS_NPC(ch) ? GET_NAME(ch) ? GET_NAME(ch) : "(null)" : GET_SHORT(ch) ? GET_SHORT(ch) : "(null)");
    break;
  case SCQ_TITLE:
    sprintf(buf, "%sTitle:%s %s",
      CHCLR(ch, 7), ENDCHCLR(ch),
      GET_TITLE(ch) ? GET_TITLE(ch) : "(no title)");
    break;
  case SCQ_LEVEL:
  case SCQ_CLASS_NAME:
    sprintf(buf, "%sLevel: [%s%d%s]%s %s",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_LEVEL(ch),
      CHCLR(ch, 7), ENDCHCLR(ch),
      ((!IS_NPC(ch) && GET_CLASS(ch) > CLASS_NONE && GET_CLASS(ch) <= CLASS_LAST) ||
       (IS_NPC(ch) && GET_CLASS(ch) >= CLASS_OTHER && GET_CLASS(ch) <= CLASS_MOB_LAST)) ?
      GET_CLASS_NAME(ch) : "Undefined");
    break;
  case SCQ_SC_LEVEL:
  case SCQ_SC_NAME:
    if (GET_SC(ch) > SC_NONE && GET_SC(ch) <= SC_LAST)
    {
    sprintf(buf, "%sSubclass: [%s%d%s]%s %s",
        CHCLR(ch, 7), ENDCHCLR(ch), GET_SC_LEVEL(ch),
        CHCLR(ch, 7), ENDCHCLR(ch), GET_SC_NAME(ch) ? GET_SC_NAME(ch) : "None");
    }
    else
    {
      sprintf(buf, "%sSubclass:%s None",
        CHCLR(ch, 7), ENDCHCLR(ch));
    }
    break;
  case SCQ_RANK_LEVEL:
  case SCQ_RANK_NAME:
    if (get_rank(ch))
    {
      sprintf(buf2, "%s", get_rank_name(ch));

      sprintf(buf, "%sRank: [%s%d%s]%s %s",
        CHCLR(ch, 7), ENDCHCLR(ch), get_rank(ch),
        CHCLR(ch, 7), ENDCHCLR(ch), string_to_upper(buf2));
    }
    else
    {
      sprintf(buf, "%sRank:%s None",
        CHCLR(ch, 7), ENDCHCLR(ch));
    }
    break;
  case SCQ_MAX_HIT:
    if (opt_text)
      sprintf(buf, "%sMaximum%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    sprintf(buf2, "%sHP: [%s%5d%s]%s",
      CHCLR(ch, 7), ENDCHCLR(ch),
      GET_MAX_HIT(ch),
      CHCLR(ch, 7), ENDCHCLR(ch));

    strcat(buf, buf2);
    break;
  case SCQ_MAX_MANA:
    if (opt_text)
      sprintf(buf, "%sMaximum%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    sprintf(buf2, "%sMana: [%s%5d%s]%s",
      CHCLR(ch, 7), ENDCHCLR(ch),
      GET_MAX_MANA(ch),
      CHCLR(ch, 7), ENDCHCLR(ch));

    strcat(buf, buf2);
    break;
  case SCQ_MAX_MOVE:
    if (opt_text)
      sprintf(buf, "%sMaximum%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    sprintf(buf, "%sMove: [%s%5d%s]%s",
      CHCLR(ch, 7), ENDCHCLR(ch),
      GET_MAX_MOVE(ch),
      CHCLR(ch, 7), ENDCHCLR(ch));

    strcat(buf, buf2);
    break;
  case SCQ_NAT_HIT:
    if (opt_text)
      sprintf(buf, "%sNatural%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    sprintf(buf2, "%sHP: [%s%5d%s]%s",
      CHCLR(ch, 7), ENDCHCLR(ch),
      GET_NAT_HIT(ch),
      CHCLR(ch, 7), ENDCHCLR(ch));

    strcat(buf, buf2);
    break;
  case SCQ_NAT_MANA:
    if (opt_text)
      sprintf(buf, "%sNatural%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    sprintf(buf2, "%sMana: [%s%5d%s]%s",
      CHCLR(ch, 7), ENDCHCLR(ch),
      GET_NAT_MANA(ch),
      CHCLR(ch, 7), ENDCHCLR(ch));

    strcat(buf, buf2);
    break;
  case SCQ_NAT_MOVE:
    if (opt_text)
      sprintf(buf, "%sNatural%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    sprintf(buf2, "%sMove: [%s%5d%s]%s",
      CHCLR(ch, 7), ENDCHCLR(ch),
      GET_NAT_MOVE(ch),
      CHCLR(ch, 7), ENDCHCLR(ch));

    strcat(buf, buf2);
    break;
  case SCQ_HIT_GAIN:
    if (opt_text)
      sprintf(buf, "%sGain +- HP:%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));
    else
      sprintf(buf, "%sHP:%s  ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    sprintf(buf2, "%+-13d", hit_gain(ch));

    strcat(buf, buf2);
    break;
  case SCQ_MANA_GAIN:
    if (opt_text)
      sprintf(buf, "%sGain +- Mana:%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));
    else
      sprintf(buf, "%sMana:%s  ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    sprintf(buf2, "%+-13d", mana_gain(ch));

    strcat(buf, buf2);
    break;
  case SCQ_MOVE_GAIN:
    if (opt_text)
      sprintf(buf, "%sGain +- Move:%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));
    else
      sprintf(buf, "%sMove:%s  ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    sprintf(buf2, "%+-13d", move_gain(ch));

    strcat(buf, buf2);
    break;
  case SCQ_STR:
    if (opt_text)
      sprintf(buf, "%sCurrent STR:%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    if (GET_LEVEL(ch) < 6)
      strcat(buf, "--");
    else
    {
      sprintf(buf2, "%-2d", GET_STR(ch));
      strcat(buf, buf2);

      if (GET_STR(ch) == 18)
      {
        sprintf(buf2, "/%-3d", GET_ADD(ch));
        strcat(buf, buf2);
      }
    }
    break;
  case SCQ_DEX:
    if (opt_text)
      sprintf(buf, "%sCurrent DEX:%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    if (GET_LEVEL(ch) < 6)
      strcat(buf, "--");
    else
    {
      sprintf(buf2, "%-2d", GET_DEX(ch));
      strcat(buf, buf2);
    }
    break;
  case SCQ_CON:
    if (opt_text)
      sprintf(buf, "%sCurrent CON:%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    if (GET_LEVEL(ch) < 6)
      strcat(buf, "--");
    else
    {
      sprintf(buf2, "%-2d", GET_CON(ch));
      strcat(buf, buf2);
    }
    break;
  case SCQ_INT:
    if (opt_text)
      sprintf(buf, "%sCurrent INT:%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    if (GET_LEVEL(ch) < 6)
      strcat(buf, "--");
    else
    {
      sprintf(buf2, "%-2d", GET_INT(ch));
      strcat(buf, buf2);
    }
    break;
  case SCQ_WIS:
    if (opt_text)
      sprintf(buf, "%sCurrent WIS:%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    if (GET_LEVEL(ch) < 6)
      strcat(buf, "--");
    else
    {
      sprintf(buf2, "%-2d", GET_WIS(ch));
      strcat(buf, buf2);
    }
    break;
  case SCQ_OSTR:
    if (opt_text)
      sprintf(buf, "%sNatural STR:%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    if (GET_LEVEL(ch) < 6)
      strcat(buf, "--");
    else
    {
      sprintf(buf2, "%-2d", GET_OSTR(ch));
      strcat(buf, buf2);

      if (GET_OSTR(ch) == 18)
      {
        sprintf(buf2, "/%-3d", GET_OADD(ch));
        strcat(buf, buf2);
      }
    }
    break;
  case SCQ_ODEX:
    if (opt_text)
      sprintf(buf, "%sNatural DEX:%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    if (GET_LEVEL(ch) < 6)
      strcat(buf, "--");
    else
    {
      sprintf(buf2, "%-2d", GET_ODEX(ch));
      strcat(buf, buf2);
    }
    break;
  case SCQ_OCON:
    if (opt_text)
      sprintf(buf, "%sNatural CON:%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    if (GET_LEVEL(ch) < 6)
      strcat(buf, "--");
    else
    {
      sprintf(buf2, "%-2d", GET_OCON(ch));
      strcat(buf, buf2);
    }
    break;
  case SCQ_OINT:
    if (opt_text)
      sprintf(buf, "%sNatural INT:%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    if (GET_LEVEL(ch) < 6)
      strcat(buf, "--");
    else
    {
      sprintf(buf2, "%-2d", GET_OINT(ch));
      strcat(buf, buf2);
    }
    break;
  case SCQ_OWIS:
    if (opt_text)
      sprintf(buf, "%sNatural WIS:%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));

    if (GET_LEVEL(ch) < 6)
      strcat(buf, "--");
    else
    {
      sprintf(buf2, "%-2d", GET_OWIS(ch));
      strcat(buf, buf2);
    }
    break;
  case SCQ_MOD_AC:
    sprintf(buf, "%sArmor Class:%s %+-13d",
      CHCLR(ch, 7), ENDCHCLR(ch), calc_ac(ch));
    break;
  case SCQ_WIMPY_LIMIT:
    sprintf(buf, "%sWimpy Limit:%s %-13d",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_WIMPY(ch));
    break;
  case SCQ_BLEED_LIMIT:
    sprintf(buf, "%sBleed Limit:%s %-13d",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_BLEED(ch));
    break;
  case SCQ_SCP:
    sprintf(buf, "%sSubclass Pts:%s %-13d",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_SCP(ch));
    break;
  case SCQ_QP:
    sprintf(buf, "%sQuest Points:%s %-13d",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_QP(ch));
    break;
  case SCQ_RANKING:
    sprintf(buf, "%sClass Points:%s %-13d",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_RANKING(ch));
    break;
  case SCQ_BEEN_KILLED:
    sprintf(buf, "%sTotal Deaths:%s %-13d",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_BEEN_KILLED(ch));
    break;
  case SCQ_AGE:
    sprintf(buf, "%sAge in Years:%s %-13d",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_AGE(ch));
    break;
  case SCQ_PRAC:
    sprintf(buf, "%sPractices:%s %-13d",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_PRAC(ch));
    break;
  case SCQ_AVG_DAMAGE:
    sprintf(buf, "%sAverage Damage:%s ",
      CHCLR(ch, 7), ENDCHCLR(ch));

    if (GET_CLASS(ch) == CLASS_NINJA)
      sprintf(buf2, "%d / %d",
        calc_hit_damage(ch, NULL, EQ(ch, WIELD), RND_AVG),
        calc_hit_damage(ch, NULL, EQ(ch, HOLD), RND_AVG));
    else
      sprintf(buf2, "%d",
        calc_hit_damage(ch, NULL, EQ(ch, WIELD), RND_AVG));
    strcat(buf, buf2);
    break;
  case SCQ_DAMROLL:
    sprintf(buf, "%sDamroll:%s %+-13d",
      CHCLR(ch, 7), ENDCHCLR(ch), calc_damroll(ch));
    break;
  case SCQ_HITROLL:
    sprintf(buf, "%sHitroll:%s %+-13d",
      CHCLR(ch, 7), ENDCHCLR(ch), calc_hitroll(ch));
    break;
  case SCQ_ALIGNMENT:
    sprintf(buf, "%sAlignment:%s %d ",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_ALIGNMENT(ch));

    if (GET_ALIGNMENT(ch) >= 750)
      strcat(buf, "(Saintly)");
    else if (GET_ALIGNMENT(ch) >= 350)
      strcat(buf, "(Good)");
    else if (GET_ALIGNMENT(ch) > -350)
      strcat(buf, "(Neutral)");
    else if (GET_ALIGNMENT(ch) > -750)
      strcat(buf, "(Evil)");
    else
      strcat(buf, "(Prime Evil)");
    break;
  case SCQ_EXP:
    sprintf(buf, "%sExperience:%s %-13d",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_EXP(ch));
    break;
  case SCQ_EXP_TO_LEVEL:
    if (GET_LEVEL(ch) < LEVEL_MORT)
      sprintf(buf, "%sXP to Level:%s %-13d",
        CHCLR(ch, 7), ENDCHCLR(ch), GET_EXP_TO_LEVEL(ch));
    else
      sprintf(buf, "%sXP to Level:%s Max Level",
        CHCLR(ch, 7), ENDCHCLR(ch));
    break;
  case SCQ_REMORT_EXP:
    sprintf(buf, "%sRemort XP:%s %-13lld",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_REMORT_EXP(ch));
    break;
  case SCQ_REMORT_MULT:
    sprintf(buf, "%sRemort Multiplier:%s %d",
      CHCLR(ch, 7), ENDCHCLR(ch), rv2_calc_remort_mult(ch));
    break;
  case SCQ_DEATH_EXP:
    sprintf(buf, "%sDeath XP:%s %-13lu",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_DEATH_EXP(ch));
    break;
  case SCQ_GOLD:
    sprintf(buf, "%sGold Carried:%s %-13d",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_GOLD(ch));
    break;
  case SCQ_BANK:
    sprintf(buf, "%sGold in Bank:%s %-13d",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_BANK(ch));
    break;
  case SCQ_QUEST_TIMER:
    if (GET_QUEST_STATUS(ch) == QUEST_NONE && !GET_QUEST_TIMER(ch))
      sprintf(buf, "%sQuest Timer:%s None",
        CHCLR(ch, 7), ENDCHCLR(ch));
    else
      sprintf(buf, "%sQuest Timer:%s %s in %d Tick%s",
        CHCLR(ch, 7), ENDCHCLR(ch),
        GET_QUEST_STATUS(ch) == QUEST_RUNNING ? "Expires" : "Next",
        GET_QUEST_TIMER(ch), GET_QUEST_TIMER(ch) == 1 ? "" : "s");
    break;
  case SCQ_CLUB:
    sprintf(buf, "%sMember of Club:%s ",
      CHCLR(ch, 7), ENDCHCLR(ch));

    if (IS_SET(GET_PFLAG(ch), PLR_SANES_VOCAL_CLUB))
      strcat(buf, "Sane's Vocal Club");
    else if (IS_SET(GET_PFLAG(ch), PLR_LINERS_LOUNGE))
      strcat(buf, "Liner's Lounge");
    else if (IS_SET(GET_PFLAG(ch), PLR_LEMS_LIQOUR_ROOM))
      strcat(buf, "Lem's Liqour Room");
    else if (IS_SET(GET_PFLAG(ch), PLR_RANGERS_RELIQUARY))
      strcat(buf, "Ranger's Reliquary");
    else
      strcat(buf, "None");
    break;
  case SCQ_CLAN:
    sprintf(buf, "%sClan/Guild:%s %s",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_CLAN_NAME(ch));
    break;
  case SCQ_PLAY_TIME:
    sprintf(buf, "%sTime Played:%s %d Day%s, %d Hour%s",
      CHCLR(ch, 7), ENDCHCLR(ch),
      GET_PLAY_TIME(ch).day, GET_PLAY_TIME(ch).day == 1 ? "" : "s",
      GET_PLAY_TIME(ch).hours, GET_PLAY_TIME(ch).hours == 1 ? "" : "s");
    break;
  case SCQ_WEIGHT_CARRIED:
    sprintf(buf, "%sWeight Carried:%s %d of %d",
      CHCLR(ch, 7), ENDCHCLR(ch), IS_CARRYING_W(ch), CAN_CARRY_W(ch));
    break;
  case SCQ_TOGGLES:
    sprintf(buf, "%sToggles:%s",
      CHCLR(ch, 7), ENDCHCLR(ch));

    if (!IS_NPC(ch) && GET_PFLAG(ch))
    {
      if (IS_SET(GET_PFLAG(ch), PLR_NOKILL))
        strcat(buf, " NO_KILL");
      if (IS_SET(GET_PFLAG(ch), PLR_NOMESSAGE))
        strcat(buf, " NO_MESSAGE");
      if (IS_SET(GET_PFLAG(ch), PLR_NOSUMMON))
        strcat(buf, " NO_SUMMON");
      if (IS_SET(GET_PFLAG(ch), PLR_GOSSIP))
        strcat(buf, " GOSSIP");
      if (IS_SET(GET_PFLAG(ch), PLR_AUCTION))
        strcat(buf, " AUCTION");
      if (IS_SET(GET_PFLAG(ch), PLR_CHAOS))
        strcat(buf, " CHAOS");
      if (IS_SET(GET_PFLAG(ch), PLR_QUESTC))
        strcat(buf, " Q-CHANNEL");
      if (IS_SET(GET_PFLAG(ch), PLR_QUEST))
        strcat(buf, " Q-PLAYER");
      if (IS_SET(GET_PFLAG(ch), PLR_QUIET))
        strcat(buf, " Q-QUIET");
    }
    else
      strcat(buf, " None");
    break;
  case SCQ_FLAGS:
    if (!IS_NPC(ch) &&
        (IS_SET(GET_PFLAG(ch), PLR_DEPUTY) ||
         GET_COND(ch, DRUNK) > 10 ||
         IS_SET(GET_PFLAG(ch), PLR_KILL) ||
         IS_SET(GET_PFLAG(ch), PLR_THIEF)))
    {
      if (IS_SET(GET_PFLAG(ch), PLR_DEPUTY))
      {
        sprintf(buf2, "%sYou are a Midgaard Deputy.%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
        strcat(buf, buf2);
      }

      if (GET_COND(ch, DRUNK) > 10)
      {
        sprintf(buf2, "%sYou are intoxicated.%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
        strcat(buf, buf2);
      }

      if (IS_SET(GET_PFLAG(ch), PLR_KILL))
      {
        sprintf(buf2, "%sYou are a killer.%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
        strcat(buf, buf2);
      }

      if (IS_SET(GET_PFLAG(ch), PLR_THIEF))
      {
        sprintf(buf2, "%sYou are a thief.%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
        strcat(buf, buf2);
      }
    }
    else
      sprintf(buf, "%sFlags:%s None\n\r",
        CHCLR(ch, 7), ENDCHCLR(ch));
    break;
  case SCQ_DEATH_LIMIT:
    if (GET_DEATH_LIMIT(ch))
      sprintf(buf, "%sAfter%s %d %sdeaths you will lose stats regardless.%s",
        CHCLR(ch, 7), ENDCHCLR(ch), GET_DEATH_LIMIT(ch), CHCLR(ch, 7), ENDCHCLR(ch));
    else if (opt_text)
      sprintf(buf, "%sYou are not affected by a death limit.%s",
        CHCLR(ch, 7), ENDCHCLR(ch));
    break;
  case SCQ_POSITION:
    switch (GET_POS(ch))
    {
    case POSITION_DEAD:
      sprintf(buf, "%sYou are DEAD!%s",
        CHCLR(ch, 7), ENDCHCLR(ch));
      break;
    case POSITION_MORTALLYW:
      sprintf(buf, "%sYou are mortally wounded! You should seek help!%s",
        CHCLR(ch, 7), ENDCHCLR(ch));
      break;
    case POSITION_INCAP:
      sprintf(buf, "%sYou are incapacitated, slowly fading away...%s",
        CHCLR(ch, 7), ENDCHCLR(ch));
      break;
    case POSITION_STUNNED:
      sprintf(buf, "%sYou are stunned! You can't move!%s",
        CHCLR(ch, 7), ENDCHCLR(ch));
      break;
    case POSITION_SLEEPING:
      sprintf(buf, "%sYou are sleeping.%s",
        CHCLR(ch, 7), ENDCHCLR(ch));
      break;
    case POSITION_RESTING:
      sprintf(buf, "%sYou are resting.%s",
        CHCLR(ch, 7), ENDCHCLR(ch));
      break;
    case POSITION_SITTING:
      sprintf(buf, "%sYou are sitting.%s",
        CHCLR(ch, 7), ENDCHCLR(ch));
      break;
    case POSITION_FLYING:
      sprintf(buf, "%sYou are flying.%s",
        CHCLR(ch, 7), ENDCHCLR(ch));
      break;
    case POSITION_RIDING:
      sprintf(buf, "%sYou are riding.%s",
        CHCLR(ch, 7), ENDCHCLR(ch));
      break;
    case POSITION_SWIMMING:
      sprintf(buf, "%sYou are swimming.%s",
        CHCLR(ch, 7), ENDCHCLR(ch));
      break;
    case POSITION_FIGHTING:
      if (GET_OPPONENT(ch))
        sprintf(buf, "%sYou are fighting %s.%s",
          CHCLR(ch, 7),
          !IS_MOB(GET_OPPONENT(ch)) ?
          GET_NAME(GET_OPPONENT(ch)) ? GET_NAME(GET_OPPONENT(ch)) : "(null)" :
          GET_SHORT(GET_OPPONENT(ch)) ? GET_SHORT(GET_OPPONENT(ch)) : "(null)",
          ENDCHCLR(ch));
      else
        sprintf(buf, "%sYou are fighting thin air.%s",
          CHCLR(ch, 7), ENDCHCLR(ch));
      break;
    case POSITION_STANDING:
      sprintf(buf, "%sYou are standing.%s",
        CHCLR(ch, 7), ENDCHCLR(ch));
      break;
    default:
      sprintf(buf, "%sYou are floating.%s",
        CHCLR(ch, 7), ENDCHCLR(ch));
      break;
    }
    break;
  case SCQ_WIZINV:
    if (IS_IMMORTAL(ch))
      sprintf(buf, "%sInvisibility Level:%s %d",
        CHCLR(ch, 7), ENDCHCLR(ch), GET_WIZINV(ch));
    else
      sprintf(buf, "This information is for gods only.");
    break;
  case SCQ_EDITING_ZONE:
    if (IS_IMMORTAL(ch))
      if (ZONE_NUM_CH(ch) >= 0)
        sprintf(buf, "%sEditting Zone:%s %d - %s",
          CHCLR(ch, 7), ENDCHCLR(ch),
          ZONE_NUM_CH(ch),
          ZONE_NAME_CH(ch) ? ZONE_NAME_CH(ch) : "Undefined");
      else
        sprintf(buf, "%sEditting Zone:%s None",
          CHCLR(ch, 7), ENDCHCLR(ch));
    else
      sprintf(buf, "This information is for gods only.");
    break;
  default:
    sprintf(buf, "Invalid query. See HELP \"SCORE QUERY\" for more information.");
  }

  if (new_line && query != SCQ_FLAGS)
    strcat(buf, "\n\r");

  send_to_char(buf, ch);
}

void do_score(CHAR *ch, char *argument, int cmd)
{
  char buf[MSL];
  char arg[MSL];
  int style = 0;
  int query = -1;

  argument_interpreter(argument, buf, arg);

  style = GET_SCORE_STYLE(ch);

  if (*buf)
  {
    if (is_abbrev(buf, "full") || !strcmp(buf, "0")) style = 0;
    else if (is_abbrev(buf, "old") || !strcmp(buf, "1")) style = 1;
    else if (is_abbrev(buf, "brief") || !strcmp(buf, "2")) style = 2;
    else if (is_abbrev(buf, "set"))
    {
      if (!*arg)
      {
        send_to_char("Please specify a score style.\n\r", ch);

        return;
      }

      sprintf(arg, "%s", skip_spaces(arg));

      if (is_number(arg)) style = atoi(arg);
      else if (is_abbrev(arg, "full")) style = 0;
      else if (is_abbrev(arg, "old")) style = 1;
      else if (is_abbrev(arg, "brief")) style = 2;
      else style = -1;

      if (style == 0) send_to_char("Score set to full display style.\n\r", ch);
      else if (style == 1) send_to_char("Score set to old display style.\n\r", ch);
      else if (style == 2) send_to_char("Score set to brief display style.\n\r", ch);
      else
      {
        send_to_char("Invalid score style.\n\r", ch);

        return;
      }

      GET_SCORE_STYLE(ch) = style;

      return;
    }
    else if (is_abbrev(buf, "query"))
    {
      if (!*arg)
      {
        send_to_char("Invalid query. See HELP \"SCORE QUERY\" for more information.\n\r", ch);

        return;
      }

      sprintf(arg, "%s", skip_spaces(arg));

      if (is_abbrev(arg, "name")) query = SCQ_NAME;
      else if (is_abbrev(arg, "title")) query = SCQ_TITLE;
      else if (is_abbrev(arg, "level")) query = SCQ_LEVEL;
      else if (is_abbrev(arg, "class")) query = SCQ_CLASS_NAME;
      else if (is_abbrev(arg, "subclass_name") || is_abbrev(arg, "sc_name")) query = SCQ_SC_NAME;
      else if (is_abbrev(arg, "subclass_level") || is_abbrev(arg, "sc_level")) query = SCQ_SC_LEVEL;
      else if (is_abbrev(arg, "rank_name")) query = SCQ_RANK_NAME;
      else if (is_abbrev(arg, "rank_level")) query = SCQ_RANK_LEVEL;
      else if (is_abbrev(arg, "maximum_hit") || is_abbrev(arg, "max_hit") || is_abbrev(arg, "hit") || is_abbrev(arg, "hp")) query = SCQ_MAX_HIT;
      else if (is_abbrev(arg, "maximum_mana") || is_abbrev(arg, "max_mana") || is_abbrev(arg, "mana")) query = SCQ_MAX_MANA;
      else if (is_abbrev(arg, "maximum_movement") || is_abbrev(arg, "max_movement") || is_abbrev(arg, "movement")) query = SCQ_MAX_MOVE;
      else if (is_abbrev(arg, "natural_hit") || is_abbrev(arg, "nat_hit") || is_abbrev(arg, "natural_hp") || is_abbrev(arg, "nat_hp")) query = SCQ_NAT_HIT;
      else if (is_abbrev(arg, "natural_mana") || is_abbrev(arg, "nat_mana")) query = SCQ_NAT_MANA;
      else if (is_abbrev(arg, "natural_movement") || is_abbrev(arg, "nat_movement")) query = SCQ_NAT_MOVE;
      else if (is_abbrev(arg, "gain_hit") || is_abbrev(arg, "gain_hp") || is_abbrev(arg, "hit_gain") || is_abbrev(arg, "hp_gain")) query = SCQ_HIT_GAIN;
      else if (is_abbrev(arg, "gain_mana") || is_abbrev(arg, "mana_gain")) query = SCQ_MANA_GAIN;
      else if (is_abbrev(arg, "gain_movement") || is_abbrev(arg, "movement_gain") || is_abbrev(arg, "move_gain")) query = SCQ_MOVE_GAIN;
      else if (is_abbrev(arg, "current_strength") || is_abbrev(arg, "cur_strength") || is_abbrev(arg, "strength")) query = SCQ_STR;
      else if (is_abbrev(arg, "current_dexterity") || is_abbrev(arg, "cur_dexterity") || is_abbrev(arg, "dexterity")) query = SCQ_DEX;
      else if (is_abbrev(arg, "current_constitution") || is_abbrev(arg, "cur_constitution") || is_abbrev(arg, "constitution")) query = SCQ_CON;
      else if (is_abbrev(arg, "current_intelligence") || is_abbrev(arg, "cur_intelligence") || is_abbrev(arg, "intelligence")) query = SCQ_INT;
      else if (is_abbrev(arg, "current_wisdom") || is_abbrev(arg, "cur_wisdom") || is_abbrev(arg, "wisdom")) query = SCQ_WIS;
      else if (is_abbrev(arg, "natural_strength") || is_abbrev(arg, "nat_strength")) query = SCQ_OSTR;
      else if (is_abbrev(arg, "natural_dexterity") || is_abbrev(arg, "nat_dexterity")) query = SCQ_ODEX;
      else if (is_abbrev(arg, "natural_constitution") || is_abbrev(arg, "nat_constitution")) query = SCQ_OCON;
      else if (is_abbrev(arg, "natural_intelligence") || is_abbrev(arg, "nat_intelligence")) query = SCQ_OINT;
      else if (is_abbrev(arg, "natural_wisdom") || is_abbrev(arg, "nat_wisdom")) query = SCQ_OWIS;
      else if (is_abbrev(arg, "armor_class") || is_abbrev(arg, "ac") || is_abbrev(arg, "armor")) query = SCQ_MOD_AC;
      else if (is_abbrev(arg, "wimpy_limit")) query = SCQ_WIMPY_LIMIT;
      else if (is_abbrev(arg, "bleed_limit")) query = SCQ_BLEED_LIMIT;
      else if (is_abbrev(arg, "subclass_points") || is_abbrev(arg, "subclass_pts") || is_abbrev(arg, "scp")) query = SCQ_SCP;
      else if (is_abbrev(arg, "quest_points") || is_abbrev(arg, "qp") || is_abbrev(arg, "aqp")) query = SCQ_QP;
      else if (is_abbrev(arg, "class_points") || is_abbrev(arg, "cpp")) query = SCQ_RANKING;
      else if (is_abbrev(arg, "deaths")) query = SCQ_BEEN_KILLED;
      else if (is_abbrev(arg, "age")) query = SCQ_AGE;
      else if (is_abbrev(arg, "practices")) query = SCQ_PRAC;
      else if (is_abbrev(arg, "average_damage") || is_abbrev(arg, "avg_damage")) query = SCQ_AVG_DAMAGE;
      else if (is_abbrev(arg, "damroll")) query = SCQ_DAMROLL;
      else if (is_abbrev(arg, "hitroll")) query = SCQ_HITROLL;
      else if (is_abbrev(arg, "alignment")) query = SCQ_ALIGNMENT;
      else if (is_abbrev(arg, "experience") || is_abbrev(arg, "xp")) query = SCQ_EXP;
      else if (is_abbrev(arg, "exp_to_level") || is_abbrev(arg, "xp_to_level")) query = SCQ_EXP_TO_LEVEL;
      else if (is_abbrev(arg, "remort_exp") || is_abbrev(arg, "remort_xp")) query = SCQ_REMORT_EXP;
      else if (is_abbrev(arg, "remort_multiplier")) query = SCQ_REMORT_MULT;
      else if (is_abbrev(arg, "death_exp") || is_abbrev(arg, "death_xp")) query = SCQ_DEATH_EXP;
      else if (is_abbrev(arg, "gold") || is_abbrev(arg, "coins") || is_abbrev(arg, "money")) query = SCQ_GOLD;
      else if (is_abbrev(arg, "bank_gold") || is_abbrev(arg, "bank_coins") || is_abbrev(arg, "bank_money")) query = SCQ_BANK;
      else if (is_abbrev(arg, "quest_timer")) query = SCQ_QUEST_TIMER;
      else if (is_abbrev(arg, "club")) query = SCQ_CLUB;
      else if (is_abbrev(arg, "clan") || is_abbrev(arg, "guild")) query = SCQ_CLAN;
      else if (is_abbrev(arg, "time_played") || is_abbrev(arg, "played")) query = SCQ_PLAY_TIME;
      else if (is_abbrev(arg, "weight_carried") || is_abbrev(arg, "carried_weight")) query = SCQ_WEIGHT_CARRIED;
      else if (is_abbrev(arg, "toggles")) query = SCQ_TOGGLES;
      else if (is_abbrev(arg, "flags")) query = SCQ_FLAGS;
      else if (is_abbrev(arg, "death_limit")) query = SCQ_DEATH_LIMIT;
      else if (is_abbrev(arg, "position")) query = SCQ_POSITION;
      else if (is_abbrev(arg, "wizinv")) query = SCQ_WIZINV;
      else if (is_abbrev(arg, "editing_zone")) query = SCQ_EDITING_ZONE;
      else {
        send_to_char("Invalid query. See HELP \"SCORE QUERY\" for more information.\n\r", ch);

        return;
      }

      score_query(ch, query, TRUE, TRUE);

      return;
    }
  }

  if (style == 0)
  {
    printf_to_char(ch, "%s%s",
      CHCLR(ch, 7),
      !IS_NPC(ch) ? GET_NAME(ch) ? GET_NAME(ch) : "(null)" : GET_SHORT(ch) ? GET_SHORT(ch) : "(null)");
    if (!IS_NPC(ch))
      printf_to_char(ch, " %s",
        GET_TITLE(ch) ? GET_TITLE(ch) : "(no title)");
    printf_to_char(ch, "%s\n\r", ENDCHCLR(ch));
    printf_to_char(ch, "%sLevel: [%s%d%s]%s %s",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_LEVEL(ch), CHCLR(ch, 7), ENDCHCLR(ch),
      ((!IS_NPC(ch) && GET_CLASS(ch) > CLASS_NONE && GET_CLASS(ch) <= CLASS_LAST) ||
       (IS_NPC(ch) && GET_CLASS(ch) >= CLASS_OTHER && GET_CLASS(ch) <= CLASS_MOB_LAST)) ?
      GET_CLASS_NAME(ch) : "Undefined");
    if (GET_SC(ch) > SC_NONE && GET_SC(ch) <= SC_LAST)
      printf_to_char(ch, "%s, Subclass: [%s%d%s]%s %s",
        CHCLR(ch, 7), ENDCHCLR(ch), GET_SC_LEVEL(ch),
        CHCLR(ch, 7), ENDCHCLR(ch), GET_SC_NAME(ch) ? GET_SC_NAME(ch) : "None");
    if (get_rank(ch))
    {
      sprintf(buf, "%s", get_rank_name(ch));

      printf_to_char(ch, "%s, Rank: [%s%d%s]%s %s",
        CHCLR(ch, 7), ENDCHCLR(ch), get_rank(ch),
        CHCLR(ch, 7), ENDCHCLR(ch), string_to_upper(buf));
    }
    send_to_char("\n\r\n\r", ch);

    printf_to_char(ch, "%sMaximum HP: [%s%5d%s]   Mana: [%s%5d%s]   Move: [%s%5d%s]%s\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_MAX_HIT(ch),
      CHCLR(ch, 7), ENDCHCLR(ch), GET_MAX_MANA(ch),
      CHCLR(ch, 7), ENDCHCLR(ch), GET_MAX_MOVE(ch),
      CHCLR(ch, 7), ENDCHCLR(ch));
    printf_to_char(ch, "%sNatural HP: [%s%5d%s]   Mana: [%s%5d%s]   Move: [%s%5d%s]%s\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_NAT_HIT(ch),
      CHCLR(ch, 7), ENDCHCLR(ch), GET_NAT_MANA(ch),
      CHCLR(ch, 7), ENDCHCLR(ch), GET_NAT_MOVE(ch),
      CHCLR(ch, 7), ENDCHCLR(ch));
    printf_to_char(ch, "%sGain +- HP:  %s%+5d%s    Mana:  %s%+5d%s    Move:  %s%+5d\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), hit_gain(ch),
      CHCLR(ch, 7), ENDCHCLR(ch), mana_gain(ch),
      CHCLR(ch, 7), ENDCHCLR(ch), move_gain(ch));
    send_to_char("\n\r", ch);

    printf_to_char(ch, "     %sNatural  Current%s       ",
      CHCLR(ch, 7), ENDCHCLR(ch));
    printf_to_char(ch, "%sAverage Damage:%s ",
      CHCLR(ch, 7), ENDCHCLR(ch));
    if (GET_CLASS(ch) == CLASS_NINJA)
      printf_to_char(ch, "%d / %d\n\r",
        calc_hit_damage(ch, NULL, EQ(ch, WIELD), RND_AVG),
        calc_hit_damage(ch, NULL, EQ(ch, HOLD), RND_AVG));
    else
      printf_to_char(ch, "%d\n\r",
        calc_hit_damage(ch, NULL, EQ(ch, WIELD), RND_AVG));
    printf_to_char(ch, "%sSTR:%s ",
      CHCLR(ch, 7), ENDCHCLR(ch));
    if (GET_LEVEL(ch) < 6)
      send_to_char("--       --            ", ch);
    else
    {
      printf_to_char(ch, "%-2d",
        GET_OSTR(ch));
      if (GET_OSTR(ch) == 18)
        printf_to_char(ch, "/%-3d   ",
          GET_OADD(ch));
      else
        send_to_char("       ", ch);
      printf_to_char(ch, "%-2d",
        GET_STR(ch));
      if (GET_STR(ch) == 18)
        printf_to_char(ch, "/%-3d        ",
          GET_ADD(ch));
      else
        send_to_char("            ", ch);
    }
    printf_to_char(ch, "       %sDamroll:%s %+d\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), calc_damroll(ch));

    printf_to_char(ch, "%sDEX:%s ",
      CHCLR(ch, 7), ENDCHCLR(ch));
    if (GET_LEVEL(ch) < 6)
      send_to_char("--       --            ", ch);
    else
      printf_to_char(ch, "%-2d       %-2d            ",
        GET_ODEX(ch), GET_DEX(ch));
    printf_to_char(ch, "       %sHitroll:%s %+d\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), calc_hitroll(ch));

    printf_to_char(ch, "%sCON:%s ",
      CHCLR(ch, 7), ENDCHCLR(ch));
    if (GET_LEVEL(ch) < 6)
      send_to_char("--       --            ", ch);
    else
      printf_to_char(ch, "%-2d       %-2d            ",
        GET_OCON(ch), GET_CON(ch));
    printf_to_char(ch, "     %sAlignment:%s %d ",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_ALIGNMENT(ch));
    if (GET_ALIGNMENT(ch) >= 750)
      send_to_char("(Saintly)\n\r", ch);
    else if (GET_ALIGNMENT(ch) >= 350)
      send_to_char("(Good)\n\r", ch);
    else if (GET_ALIGNMENT(ch) > -350)
      send_to_char("(Neutral)\n\r", ch);
    else if (GET_ALIGNMENT(ch) > -750)
      send_to_char("(Evil)\n\r", ch);
    else
      send_to_char("(Prime Evil)\n\r", ch);

    printf_to_char(ch, "%sINT:%s ",
      CHCLR(ch, 7), ENDCHCLR(ch));
    if (GET_LEVEL(ch) < 6)
      send_to_char("--       --            ", ch);
    else
      printf_to_char(ch, "%-2d       %-2d            ",
        GET_OINT(ch), GET_INT(ch));
    printf_to_char(ch, "    %sExperience:%s %d\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_EXP(ch));

    printf_to_char(ch, "%sWIS:%s ",
      CHCLR(ch, 7), ENDCHCLR(ch));
    if (GET_LEVEL(ch) < 6)
      send_to_char("--       --            ", ch);
    else
      printf_to_char(ch, "%-2d       %-2d            ",
        GET_OWIS(ch), GET_WIS(ch));
    if (GET_LEVEL(ch) < LEVEL_MORT)
      printf_to_char(ch, "   %sXP to Level:%s %d\n\r",
        CHCLR(ch, 7), ENDCHCLR(ch), GET_EXP_TO_LEVEL(ch));
    else
      printf_to_char(ch, "   %sXP to Level:%s Max Level\n\r",
        CHCLR(ch, 7), ENDCHCLR(ch));

    printf_to_char(ch, " %sArmor Class:%s %+-13d ",
      CHCLR(ch, 7), ENDCHCLR(ch), calc_ac(ch));
    printf_to_char(ch, "     %sRemort XP:%s %lld (%dx)\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_REMORT_EXP(ch), rv2_calc_remort_mult(ch));

    printf_to_char(ch, " %sWimpy Limit:%s %-13d       ",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_WIMPY(ch));
    printf_to_char(ch, "%sDeath XP:%s %lu (2x)\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_DEATH_EXP(ch));

    printf_to_char(ch, " %sBleed Limit:%s %-13d   ",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_BLEED(ch));
    printf_to_char(ch, "%sGold Carried:%s %d\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_GOLD(ch));

    printf_to_char(ch, "%sSubclass Pts:%s %-13d   ",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_SCP(ch));
    printf_to_char(ch, "%sGold in Bank:%s %d\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_BANK(ch));

    printf_to_char(ch, "%sQuest Points:%s %-13d    ",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_QP(ch));
    if (GET_QUEST_STATUS(ch) == QUEST_NONE && !GET_QUEST_TIMER(ch))
      printf_to_char(ch, "%sQuest Timer:%s None\n\r",
        CHCLR(ch, 7), ENDCHCLR(ch));
    else
      printf_to_char(ch, "%sQuest Timer:%s %s in %d Tick%s\n\r",
        CHCLR(ch, 7), ENDCHCLR(ch),
        GET_QUEST_STATUS(ch) == QUEST_RUNNING ? "Expires" : "Next",
        GET_QUEST_TIMER(ch), GET_QUEST_TIMER(ch) == 1 ? "" : "s");

    printf_to_char(ch, "%sClass Points:%s %-13d ",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_RANKING(ch));
    printf_to_char(ch, "%sMember of Club:%s ",
      CHCLR(ch, 7), ENDCHCLR(ch));
    if (IS_SET(GET_PFLAG(ch), PLR_SANES_VOCAL_CLUB))
      send_to_char("Sane's Vocal Club\n\r", ch);
    else if (IS_SET(GET_PFLAG(ch), PLR_LINERS_LOUNGE))
      send_to_char("Liner's Lounge\n\r", ch);
    else if (IS_SET(GET_PFLAG(ch), PLR_LEMS_LIQOUR_ROOM))
      send_to_char("Lem's Liqour Room\n\r", ch);
    else if (IS_SET(GET_PFLAG(ch), PLR_RANGERS_RELIQUARY))
      send_to_char("Ranger's Reliquary\n\r", ch);
    else
      send_to_char("None\n\r", ch);

    printf_to_char(ch, "%sTotal Deaths:%s %-13d     ",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_BEEN_KILLED(ch));
    printf_to_char(ch, "%sClan/Guild:%s %s\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_CLAN_NAME(ch));

    printf_to_char(ch, "%sAge in Years:%s %-13d    ",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_AGE(ch));
    printf_to_char(ch, "%sTime Played:%s %d Day%s, %d Hour%s\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch),
      GET_PLAY_TIME(ch).day, GET_PLAY_TIME(ch).day == 1 ? "" : "s",
      GET_PLAY_TIME(ch).hours, GET_PLAY_TIME(ch).hours == 1 ? "" : "s");

    printf_to_char(ch, "   %sPractices:%s %-13d ",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_PRAC(ch));
    printf_to_char(ch, "%sWeight Carried:%s %d of %d\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), IS_CARRYING_W(ch), CAN_CARRY_W(ch));
    send_to_char("\n\r", ch);

    if (!IS_NPC(ch))
    {
      printf_to_char(ch, "%sToggles:%s",
        CHCLR(ch, 7), ENDCHCLR(ch));

      if (!IS_NPC(ch) && GET_PFLAG(ch))
      {
        if (IS_SET(GET_PFLAG(ch), PLR_NOKILL))
          send_to_char(" NO_KILL", ch);
        if (IS_SET(GET_PFLAG(ch), PLR_NOMESSAGE))
          send_to_char(" NO_MESSAGE", ch);
        if (IS_SET(GET_PFLAG(ch), PLR_NOSUMMON))
          send_to_char(" NO_SUMMON", ch);
        if (IS_SET(GET_PFLAG(ch), PLR_GOSSIP))
          send_to_char(" GOSSIP", ch);
        if (IS_SET(GET_PFLAG(ch), PLR_AUCTION))
          send_to_char(" AUCTION", ch);
        if (IS_SET(GET_PFLAG(ch), PLR_CHAOS))
          send_to_char(" CHAOS", ch);
        if (IS_SET(GET_PFLAG(ch), PLR_QUESTC))
          send_to_char(" Q-CHANNEL", ch);
        if (IS_SET(GET_PFLAG(ch), PLR_QUEST))
          send_to_char(" Q-PLAYER", ch);
        if (IS_SET(GET_PFLAG(ch), PLR_QUIET))
          send_to_char(" Q-QUIET", ch);
        send_to_char("\n\r", ch);
      }
      else
        send_to_char(" None\n\r", ch);
    }

    if (IS_IMMORTAL(ch))
    {
      printf_to_char(ch, "%sWiz Flags:%s ",
        CHCLR(ch, 7), ENDCHCLR(ch));

      sprintbit(GET_IMM_FLAGS(ch), wiz_bits, buf);
      strcat(buf, "\n\r");

      send_to_char(buf, ch);
    }

    if (!IS_NPC(ch))
    {
      if (IS_SET(GET_PFLAG(ch), PLR_DEPUTY))
        printf_to_char(ch, "%sYou are a Midgaard Deputy.%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
      if (GET_COND(ch, DRUNK) > 10)
        printf_to_char(ch, "%sYou are intoxicated.%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
      if (IS_SET(GET_PFLAG(ch), PLR_KILL))
        printf_to_char(ch, "%sYou are a killer.%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
      if (IS_SET(GET_PFLAG(ch), PLR_THIEF))
        printf_to_char(ch, "%sYou are a thief.%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));

      if (GET_DEATH_LIMIT(ch))
        printf_to_char(ch, "%sAfter%s %d %sdeaths you will lose stats regardless.%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch), GET_DEATH_LIMIT(ch), CHCLR(ch, 7), ENDCHCLR(ch));
    }

    switch (GET_POS(ch))
    {
      case POSITION_DEAD:
        printf_to_char(ch, "%sYou are DEAD!%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
        break;
      case POSITION_MORTALLYW:
        printf_to_char(ch, "%sYou are mortally wounded! You should seek help!%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
        break;
      case POSITION_INCAP:
        printf_to_char(ch, "%sYou are incapacitated, slowly fading away...%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
        break;
      case POSITION_STUNNED:
        printf_to_char(ch, "%sYou are stunned! You can't move!%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
        break;
      case POSITION_SLEEPING:
        printf_to_char(ch, "%sYou are sleeping.%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
        break;
      case POSITION_RESTING:
        printf_to_char(ch, "%sYou are resting.%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
        break;
      case POSITION_SITTING:
        printf_to_char(ch, "%sYou are sitting.%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
        break;
      case POSITION_FLYING:
        printf_to_char(ch, "%sYou are flying.%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
        break;
      case POSITION_RIDING:
        printf_to_char(ch, "%sYou are riding.%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
        break;
      case POSITION_SWIMMING:
        printf_to_char(ch, "%sYou are swimming.%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
        break;
      case POSITION_FIGHTING:
        if (GET_OPPONENT(ch))
          printf_to_char(ch, "%sYou are fighting %s.%s\n\r",
            CHCLR(ch, 7),
            !IS_MOB(GET_OPPONENT(ch)) ?
            GET_NAME(GET_OPPONENT(ch)) ? GET_NAME(GET_OPPONENT(ch)) : "(null)" :
            GET_SHORT(GET_OPPONENT(ch)) ? GET_SHORT(GET_OPPONENT(ch)) : "(null)",
            ENDCHCLR(ch));
        else
          printf_to_char(ch, "%sYou are fighting thin air.%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
        break;
      case POSITION_STANDING:
        printf_to_char(ch, "%sYou are standing.%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
        break;
      default:
        printf_to_char(ch, "%sYou are floating.%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch));
        break;
    }

    if (GET_PROTECTOR(ch)) {
      printf_to_char(ch, "%sYou are being protected by %s.%s\n\r",
          CHCLR(ch, 7),
          GET_NAME(GET_PROTECTOR(ch)) ? GET_NAME(GET_PROTECTOR(ch)) : "(null)",
          ENDCHCLR(ch));
    }

    if (GET_PROTECTEE(ch)) {
      printf_to_char(ch, "%sYou are protecting %s.%s\n\r",
          CHCLR(ch, 7),
          GET_NAME(GET_PROTECTEE(ch)) ? GET_NAME(GET_PROTECTEE(ch)) : "(null)",
          ENDCHCLR(ch));
    }

    if (IS_IMMORTAL(ch))
    {
      printf_to_char(ch, "\n\r%sInvisibility Level:%s %d\n\r",
        CHCLR(ch, 7), ENDCHCLR(ch), GET_WIZINV(ch));

      if (ZONE_NUM_CH(ch) >= 0)
        printf_to_char(ch, "%sEditting Zone:%s %d - %s%s\n\r",
          CHCLR(ch, 7), ENDCHCLR(ch),
          ZONE_NUM_CH(ch),
          ZONE_NAME_CH(ch) ? ZONE_NAME_CH(ch) : "Undefined",
          ENDCHCLR(ch));
    }
  }
  else if (style == 1)
  {
    printf_to_char(ch, "This ranks you as %s %s%s (level %d).\n\r",
      GET_NAME(ch) ? GET_NAME(ch) : "(null)",
      GET_TITLE(ch) ? GET_TITLE(ch) : "(no title)",
      ENDCHCLR(ch), GET_LEVEL(ch));

    printf_to_char(ch, "You are %d years old.%s\n\r",
      GET_AGE(ch), ((age(ch).month == 0) && (age(ch).day == 0)) ? " It's your birthday today." : "");

    printf_to_char(ch, "Your alignment is %d (-1000 to 1000) and your armor is %d (100 to %d).\n\r",
      GET_ALIGNMENT(ch), calc_ac(ch), IS_NPC(ch) || affected_by_spell(ch, SKILL_DEFEND) ? -300 : -250);

    if (GET_ALIGNMENT(ch) >= 750)
      send_to_char("You are saintly.\n\r", ch);
    else if (GET_ALIGNMENT(ch) >= 350)
      send_to_char("You are good.\n\r", ch);
    else if (GET_ALIGNMENT(ch) >= 250)
      send_to_char("You are neutral, with tendencies towards good.\n\r", ch);
    else if (GET_ALIGNMENT(ch) > -250)
      send_to_char("You are neutral.\n\r", ch);
    else if (GET_ALIGNMENT(ch) > -350)
      send_to_char("You are neutral, with tendencies towards evil.\n\r", ch);
    else if (GET_ALIGNMENT(ch) > -750)
      send_to_char("You are evil.\n\r", ch);
    else
      send_to_char("You are prime evil.\n\r", ch);

    if (GET_LEVEL(ch) > 5)
    {
      if (GET_STR(ch) == 18)
        printf_to_char(ch, "You have %d/%d STR, %d INT, %d WIS, %d DEX, %d CON\n\r",
          GET_STR(ch), GET_ADD(ch), GET_INT(ch), GET_WIS(ch), GET_DEX(ch), GET_CON(ch));
      else
        printf_to_char(ch, "You have %d STR, %d INT, %d WIS, %d DEX, %d CON\n\r",
          GET_STR(ch), GET_INT(ch), GET_WIS(ch), GET_DEX(ch), GET_CON(ch));
    }

    printf_to_char(ch, "You have %d(%d) hit, %d(%d) mana and %d(%d) movement points.\n\r",
      GET_HIT(ch), GET_MAX_HIT(ch), GET_MANA(ch), GET_MAX_MANA(ch), GET_MOVE(ch), GET_MAX_MOVE(ch));

    if (GET_CLASS(ch) == CLASS_NINJA)
      printf_to_char(ch, "Your average damage is %d / %d.\n\r",
        calc_hit_damage(ch, NULL, EQ(ch, WIELD), RND_AVG),
        calc_hit_damage(ch, NULL, EQ(ch, HOLD), RND_AVG));
    else
      printf_to_char(ch, "Your average damage is %d.\n\r",
        calc_hit_damage(ch, NULL, EQ(ch, WIELD), RND_AVG));

    if (GET_LEVEL(ch) > 5)
      printf_to_char(ch, "You are carrying %d weight, and the max weight without penalties is %d.\n\r",
        IS_CARRYING_W(ch), CAN_CARRY_W(ch));

    printf_to_char(ch, "You have scored %d exp, and have %d gold coins.\n\r",
      GET_EXP(ch), GET_GOLD(ch));

    printf_to_char(ch, "You have been playing for %d days and %d hours.\n\r",
      GET_PLAY_TIME(ch).day, GET_PLAY_TIME(ch).hours);

    if (GET_LEVEL(ch) < LEVEL_MORT)
    {
      printf_to_char(ch, "You need %d experience points to reach level %d.\n\r",
        exp_table[GET_LEVEL(ch) + 1] - GET_EXP(ch), GET_LEVEL(ch) + 1);
    }

    if (GET_REMORT_EXP(ch))
    {
      printf_to_char(ch, "You have %lld remort experience to re-earn at a %dx multiplier.\n\r",
        GET_REMORT_EXP(ch), rv2_calc_remort_mult(ch));
    }

    if (GET_DEATH_EXP(ch))
    {
      printf_to_char(ch, "You have %lu death experience to re-earn at a 2x multiplier.\n\r",
        GET_DEATH_EXP(ch));
    }

    printf_to_char(ch, "You have %d class placement points.\n\r", GET_RANKING(ch));
    printf_to_char(ch, "You have %d quest points.\n\r", GET_QP(ch));
    printf_to_char(ch, "You have %d subclass points.\n\r", GET_SCP(ch));

    if (GET_SC(ch) > SC_NONE && GET_SC(ch) <= SC_LAST)
      printf_to_char(ch, "You are a level %d %s.\n\r",
        GET_SC_LEVEL(ch), GET_SC_NAME(ch) ? GET_SC_NAME(ch) : "None");

    if (!IS_NPC(ch))
    {
      if (IS_SET(GET_PFLAG(ch), PLR_DEPUTY))
        send_to_char("You are a Midgaard Deputy.\n\r", ch);
      if (GET_COND(ch, DRUNK) > 10)
        send_to_char("You are intoxicated.\n\r", ch);
      if (IS_SET(GET_PFLAG(ch), PLR_KILL))
        send_to_char("You are a killer.\n\r", ch);
      if (IS_SET(GET_PFLAG(ch), PLR_THIEF))
        send_to_char("You are a thief.\n\r", ch);
    }

    switch (GET_POS(ch))
    {
      case POSITION_DEAD:
        send_to_char("You are DEAD!\n\r", ch);
        break;
      case POSITION_MORTALLYW:
        send_to_char("You are mortally wounded! You should seek help!\n\r", ch);
        break;
      case POSITION_INCAP:
        send_to_char("You are incapacitated, slowly fading away...\n\r", ch);
        break;
      case POSITION_STUNNED:
        send_to_char("You are stunned! You can't move!\n\r", ch);
        break;
      case POSITION_SLEEPING:
        send_to_char("You are sleeping.\n\r", ch);
        break;
      case POSITION_RESTING:
        send_to_char("You are resting.\n\r", ch);
        break;
      case POSITION_SITTING:
        send_to_char("You are sitting.\n\r", ch);
        break;
      case POSITION_FLYING:
        send_to_char("You are flying.\n\r", ch);
        break;
      case POSITION_RIDING:
        send_to_char("You are riding.\n\r", ch);
        break;
      case POSITION_SWIMMING:
        send_to_char("You are swimming.\n\r", ch);
        break;
      case POSITION_FIGHTING:
        if (GET_OPPONENT(ch))
          printf_to_char(ch, "You are fighting %s.\n\r",
            !IS_MOB(GET_OPPONENT(ch)) ?
            GET_NAME(GET_OPPONENT(ch)) ? GET_NAME(GET_OPPONENT(ch)) : "(null)" :
            GET_SHORT(GET_OPPONENT(ch)) ? GET_SHORT(GET_OPPONENT(ch)) : "(null)");
        else
          send_to_char("You are fighting thin air.\n\r", ch);
        break;
      case POSITION_STANDING:
        send_to_char("You are standing.\n\r", ch);
        break;
      default:
        send_to_char("You are floating.\n\r", ch);
        break;
    }

    if (!IS_NPC(ch))
    {
      if (GET_WIMPY(ch) > 0)
        printf_to_char(ch, "You will flee when you only have %d hit points.\n\r",
          GET_WIMPY(ch));

      if (GET_BLEED(ch) > 0)
        printf_to_char(ch, "You will be warned about bleeding at %d hit points.\n\r",
          GET_BLEED(ch));

      if (GET_DEATH_LIMIT(ch) > 0)
        printf_to_char(ch, "After %d deaths you will lose stats regardless.\n\r",
          GET_DEATH_LIMIT(ch));

      if (IS_SET(GET_PFLAG(ch), PLR_SANES_VOCAL_CLUB))
        send_to_char("You are a member of Sane's Vocal Club.\n\r", ch);
      else if (IS_SET(GET_PFLAG(ch), PLR_LINERS_LOUNGE))
        send_to_char("You are a member of Liner's Lounge.\n\r", ch);
      else if (IS_SET(GET_PFLAG(ch), PLR_LEMS_LIQOUR_ROOM))
        send_to_char("You are a member of Lem's Liqour Room.\n\r", ch);
      else if (IS_SET(GET_PFLAG(ch), PLR_RANGERS_RELIQUARY))
        send_to_char("You are a member of Ranger's Reliquary.\n\r", ch);

      if (GET_CLAN_NUM(ch))
        printf_to_char(ch, "You are a member of %s.\n\r", GET_CLAN_NAME(ch));

      if (GET_PFLAG(ch))
      {
        send_to_char("You are affected by : ", ch);
        if (GET_PFLAG(ch))
        {
          if (IS_SET(GET_PFLAG(ch), PLR_NOKILL))
            send_to_char("NO_KILL ", ch);
          if (IS_SET(GET_PFLAG(ch), PLR_NOMESSAGE))
            send_to_char("NO_MESSAGE ", ch);
          if (IS_SET(GET_PFLAG(ch), PLR_NOSUMMON))
            send_to_char("NO_SUMMON ", ch);
          if (IS_SET(GET_PFLAG(ch), PLR_GOSSIP))
            send_to_char("GOSSIP ", ch);
          if (IS_SET(GET_PFLAG(ch), PLR_AUCTION))
            send_to_char("AUCTION ", ch);
          if (IS_SET(GET_PFLAG(ch), PLR_CHAOS))
            send_to_char("CHAOS ", ch);
          if (IS_SET(GET_PFLAG(ch), PLR_QUESTC))
            send_to_char("Q-CHANNEL ", ch);
          if (IS_SET(GET_PFLAG(ch), PLR_QUEST))
            send_to_char("Q-PLAYER ", ch);
          if (IS_SET(GET_PFLAG(ch), PLR_QUIET))
            send_to_char("Q-QUIET ", ch);
          send_to_char(" \n\r", ch);
        }
      }
    }

    if (IS_IMMORTAL(ch))
    {
      printf_to_char(ch, "Your invisibility level is %d.\n\r", GET_WIZINV(ch));

      if (ZONE_NUM_CH(ch) >= 0)
        printf_to_char(ch, "You are editting zone %d - %s.\n\r",
          ZONE_NUM_CH(ch), ZONE_NAME_CH(ch) ? ZONE_NAME_CH(ch) : "Undefined");
    }
  }
  else if (style == 2)
  {
    printf_to_char(ch, "%s%s",
      CHCLR(ch, 7),
      !IS_NPC(ch) ? GET_NAME(ch) ? GET_NAME(ch) : "(null)" : GET_SHORT(ch) ? GET_SHORT(ch) : "(null)");
    if (!IS_NPC(ch))
      printf_to_char(ch, " %s",
        GET_TITLE(ch) ? GET_TITLE(ch) : "(no title)");
    printf_to_char(ch, "%s\n\r", ENDCHCLR(ch));
    printf_to_char(ch, "%sLevel: [%s%d%s]%s %s",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_LEVEL(ch),
      CHCLR(ch, 7), ENDCHCLR(ch),
      ((!IS_NPC(ch) && GET_CLASS(ch) > CLASS_NONE && GET_CLASS(ch) <= CLASS_LAST) ||
       (IS_NPC(ch) && GET_CLASS(ch) >= CLASS_OTHER && GET_CLASS(ch) <= CLASS_MOB_LAST)) ?
      GET_CLASS_NAME(ch) : "Undefined");
    if (GET_SC(ch) > SC_NONE && GET_SC(ch) <= SC_LAST)
      printf_to_char(ch, "%s, Subclass: [%s%d%s]%s %s",
        CHCLR(ch, 7), ENDCHCLR(ch), GET_SC_LEVEL(ch),
        CHCLR(ch, 7), ENDCHCLR(ch), GET_SC_NAME(ch) ? GET_SC_NAME(ch) : "None");
    if (get_rank(ch))
    {
      sprintf(buf, "%s", get_rank_name(ch));

      printf_to_char(ch, "%s, Rank: [%s%d%s]%s %s",
        CHCLR(ch, 7), ENDCHCLR(ch), get_rank(ch),
        CHCLR(ch, 7), ENDCHCLR(ch), string_to_upper(buf));
    }
    send_to_char("\n\r\n\r", ch);

    printf_to_char(ch, "%sMaximum HP: [%s%5d%s]  Mana: [%s%5d%s]  Move: [%s%5d%s]%s\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_MAX_HIT(ch),
      CHCLR(ch, 7), ENDCHCLR(ch), GET_MAX_MANA(ch),
      CHCLR(ch, 7), ENDCHCLR(ch), GET_MAX_MOVE(ch),
      CHCLR(ch, 7), ENDCHCLR(ch));
    printf_to_char(ch, "%sGain +- HP:  %s%+5d%s   Mana:  %s%+5d%s   Move:  %s%+5d\n\r\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), hit_gain(ch),
      CHCLR(ch, 7), ENDCHCLR(ch), mana_gain(ch),
      CHCLR(ch, 7), ENDCHCLR(ch), move_gain(ch));

    printf_to_char(ch, " %sArmor Class:%s %+-13d ",
      CHCLR(ch, 7), ENDCHCLR(ch), calc_ac(ch));
    printf_to_char(ch, "     %sAlignment:%s %d ",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_ALIGNMENT(ch));
    if (GET_ALIGNMENT(ch) >= 750)
      send_to_char("(Saintly)\n\r", ch);
    else if (GET_ALIGNMENT(ch) >= 350)
      send_to_char("(Good)\n\r", ch);
    else if (GET_ALIGNMENT(ch) > -350)
      send_to_char("(Neutral)\n\r", ch);
    else if (GET_ALIGNMENT(ch) > -750)
      send_to_char("(Evil)\n\r", ch);
    else
      send_to_char("(Prime Evil)\n\r", ch);

    printf_to_char(ch, "%sGold Carried:%s %-13d     ",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_GOLD(ch));
    printf_to_char(ch, "%sExperience:%s %d\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_EXP(ch));

    printf_to_char(ch, "%sGold in Bank:%s %-13d    ",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_BANK(ch));
    printf_to_char(ch, "%sXP to Level:%s %d\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_EXP_TO_LEVEL(ch));

    printf_to_char(ch, "%sSubclass Pts:%s %-13d      ",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_SCP(ch));
    printf_to_char(ch, "%sRemort XP:%s %lld (%dx)\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_REMORT_EXP(ch), rv2_calc_remort_mult(ch));

    printf_to_char(ch, "%sQuest Points:%s %-13d       ",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_QP(ch));
    printf_to_char(ch, "%sDeath XP:%s %lu (2x)\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_DEATH_EXP(ch));

    printf_to_char(ch, "%sAge in Years:%s %-13d ",
      CHCLR(ch, 7), ENDCHCLR(ch), GET_AGE(ch));
    printf_to_char(ch, "%sWeight Carried:%s %d of %d\n\r",
      CHCLR(ch, 7), ENDCHCLR(ch), IS_CARRYING_W(ch), CAN_CARRY_W(ch));
  }
}
