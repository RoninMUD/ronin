/**************************************************************************
*  file: act.informative.c , Implementation of commands.  Part of DIKUMUD *
*  Usage : Informative commands.                                          *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <netinet/in.h>
#include <time.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <math.h>
#include <limits.h>
#include <errno.h>

#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "limits.h"
#include "utility.h"
#include "fight.h"
#include "modify.h"
#include "utility.h"
#include "cmd.h"
#include "reception.h"
#include "spec.clan.h"
#include "enchant.h"
#include "meta.h"
#include "quest.h"
#include "subclass.h"

/* extern functions */

extern int file_to_string (char *name, char *buf);
extern int signal_char(CHAR *ch, CHAR *signaler, int cmd, char *arg) ;
struct time_info_data age(struct char_data *ch);
void page_string(struct descriptor_data *d, char *str, int keep_internal);
void make_chaos_corpse(struct char_data *ch);
void create_wizlist (FILE *wizlist);
void create_inactive_wizlist (FILE *wizlist);
void give_prompt(struct descriptor_data *point);
void death_list(CHAR *ch);

/* intern functions */

void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode,
          bool show);
void look_in_room(CHAR *ch, int vnum);

void show_char_to_char(struct char_data *i, struct char_data *ch, int mode);


const char *object_decay_text[] = {
  "like new",
  "almost new",
  "fairly new",
  "slightly worn",
  "worn",
  "fairly worn",
  "very worn",
  "slightly cracked",
  "cracked",
  "about to crumble"
};

/* Procedures related to 'look' */

char *find_ex_description(char *string, struct extra_descr_data *list) {
  for (struct extra_descr_data *desc = list; desc; desc = desc->next) {
    if (isname(string, desc->keyword)) {
      return desc->description;
    }
  }

  return NULL;
}

void make_statue(CHAR *ch) {
  if (!ch) return;

  char buf[MIL];
  OBJ *statue;

  CREATE(statue, OBJ, 1);

  OBJ_RNUM(statue) = -1;
  OBJ_IN_ROOM(statue) = NOWHERE;


  if (IS_NPC(ch)) {
    OBJ_GET_NAME(statue) = strdup("statue");
  }
  else {
    snprintf(buf, sizeof(buf), "statue %s", GET_NAME(ch));
    OBJ_GET_NAME(statue) = strdup(str_lwr(buf));
  }

  snprintf(buf, sizeof(buf), "A statue of %s is standing here.", GET_DISP_NAME(ch));
  OBJ_GET_DESCRIPTION(statue) = strdup(buf);

  snprintf(buf, sizeof(buf), "Statue of %s", GET_DISP_NAME(ch));
  OBJ_GET_SHORT(statue) = strdup(buf);

  OBJ_TYPE(statue) = ITEM_CONTAINER;

  SET_BIT(OBJ_WEAR_FLAGS(statue), ITEM_TAKE);

  OBJ_VALUE(statue, 0) = 0;
  OBJ_VALUE(statue, 2) = GET_LEVEL(ch);
  OBJ_VALUE(statue, 3) = 1;

  OBJ_WEIGHT(statue) = 10000;

  OBJ_COST(statue) = IS_NPC(ch) ? NPC_STATUE : PC_STATUE;
  OBJ_TIMER(statue) = IS_NPC(ch) ? MAX_NPC_STATUE_TIME : MAX_PC_STATUE_TIME;

  OBJ_CONTAINS(statue) = GET_CARRYING(ch);
  GET_CARRYING(ch) = NULL;

  for (OBJ *obj = OBJ_CONTAINS(statue); obj; OBJ_IN_OBJ(obj) =statue, obj = OBJ_NEXT_CONTENT(obj));

  object_list_new_owner(statue, NULL);

  for (int i = 0; i < MAX_WEAR; i++) {
    if (EQ(ch, i)) {
      obj_to_obj(unequip_char(ch, i), statue);
    }
  }

  if (GET_GOLD(ch) > 0) {
    obj_to_obj(create_money(GET_GOLD(ch)), statue);

    GET_GOLD(ch) = 0;
  }

  statue->next = object_list;
  object_list = statue;

  if (IS_MORTAL(ch) && (GET_LEVEL(ch) < 10) &&
      !IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), DEATH) &&
      !IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), HAZARD)) {
    send_to_char("\n\rYour statue is in the Midgaard Morgue, 2 west from the Temple of Midgaard.\n\rWhen you reach level 10, your statue will remain where you died.\n\r", ch);

    obj_to_room(statue, real_room(ROOM_MORGUE));
  }
  else {
    obj_to_room(statue, CHAR_REAL_ROOM(ch));
  }

  if (IS_MORTAL(ch)) {
    GET_BEEN_KILLED(ch) += 1;

    death_list(ch);

    GET_HIT(ch) = 1;
    GET_MANA(ch) = 1;
  }

  remove_all_affects(ch);
}


// TODO: Remove or define "magic number" mode.
void show_obj_to_char(OBJ *obj, CHAR *ch, int mode, int num) {
  const char *statue_decay_text[] = {
    "The new ",
    "The ",
    "The slightly chipped ",
    "The chipped ",
    "The badly chipped ",
    "The awfully chipped ",
    "The cracked and chipped ",
    "The awfully cracked ",
    "The barely recognizable crumbling ",
    "A crumbling statue of unknown origin stands here."
  };

  const char *corpse_decay_text[] = {
    "The fresh ",
    "The ",
    "The slightly decayed ",
    "The mildly decayed ",
    "The badly decayed ",
    "The awfully decayed ",
    "The rotting ",
    "The putrid rotten ",
    "The barely recognizable ",
    "Flies and maggots surround this gruesomely decayed corpse."
  };

  char buf[4 * MSL], *tmp;

  switch (mode) {
    case 0:
      snprintf(buf, sizeof(buf), "%s", OBJ_DESCRIPTION(obj));
      break;

    case 1:
    case 2:
    case 3:
    case 4:
      snprintf(buf, sizeof(buf), "%s", OBJ_SHORT(obj));
      break;

    case 5:
      if (OBJ_TYPE(obj) == ITEM_NOTE) {
        if (OBJ_ACTION(obj)) {
          snprintf(buf, sizeof(buf), "There is something written upon it:\n\r\n\r%s", OBJ_ACTION(obj));

          page_string(GET_DESCRIPTOR(ch), buf, 1);
        }
        else {
          send_to_char("It's blank.\n\r", ch);
        }

        return;
      }
      else if (OBJ_TYPE(obj) == ITEM_TICKET) {
        if ((OBJ_VALUE(obj, 1) == OBJ_VALUE(obj, 2)) || (OBJ_VALUE(obj, 2) == 0)) {
          snprintf(buf, sizeof(buf), "This is ticket # %d from lot %d.\n\r", OBJ_VALUE(obj, 1), OBJ_VALUE(obj, 0));
        }
        else {
          snprintf(buf, sizeof(buf), "This is ticket # %d to %d from lot %d.\n\r", OBJ_VALUE(obj, 1), OBJ_VALUE(obj, 2), OBJ_VALUE(obj, 0));
        }

        page_string(GET_DESCRIPTOR(ch), buf, 1);

        return;
      }
      else if (IS_SET(OBJ_EXTRA_FLAGS2(obj), ITEM_ALL_DECAY) || IS_SET(OBJ_EXTRA_FLAGS2(obj), ITEM_EQ_DECAY)) {
        const int decay_string_max_idx = NUMELEMS(object_decay_text) - 1;

        int decay_level = OBJ_PROTO_TIMER(obj) ? decay_string_max_idx - ((10 * OBJ_TIMER(obj)) / OBJ_PROTO_TIMER(obj)) : decay_string_max_idx;

        decay_level = MIN(MAX(decay_level, 0), decay_string_max_idx);

        snprintf(buf, sizeof(buf), "This item decays and seems %s.", object_decay_text[decay_level]);
      }
      else if (OBJ_TYPE(obj) == ITEM_DRINKCON) {
        snprintf(buf, sizeof(buf), "It looks like a drink container.");
      }
      else {
        snprintf(buf, sizeof(buf), "You see nothing special.");
      }
      break;
    default:
      buf[0] = '\0';
      break;
  }

  if (mode != 3) {
    if ((mode != 5) && IS_CORPSE(obj)) {
      tmp = strdup(buf);

      int max_time = 0;

      switch (OBJ_COST(obj)) {
        case PC_CORPSE:
          max_time = MAX_PC_CORPSE_TIME;
          break;

        case NPC_CORPSE:
          max_time = MAX_NPC_CORPSE_TIME;
          break;

        case PC_STATUE:
          max_time = MAX_PC_STATUE_TIME;
          break;

        case NPC_STATUE:
          max_time = MAX_NPC_STATUE_TIME;
          break;

        case CHAOS_CORPSE:
          max_time = MAX_CHAOS_CORPSE_TIME;
          break;

        default:
          max_time = 0;
          break;
      }

      int decay_text_max_idx = 0;
      int decay_level = 0;

      if ((OBJ_RENT_COST(obj) == PC_STATUE) || (OBJ_RENT_COST(obj) == NPC_STATUE)) {
        decay_text_max_idx = NUMELEMS(statue_decay_text) - 1;

        decay_level = max_time ? decay_text_max_idx - ((10 * OBJ_TIMER(obj)) / max_time) : decay_text_max_idx;
        decay_level = MIN(MAX(decay_level, 0), decay_text_max_idx);

        snprintf(buf, sizeof(buf), "%s", statue_decay_text[decay_level]);
      }
      else {
        decay_text_max_idx = NUMELEMS(corpse_decay_text) - 1;

        decay_level = max_time ? decay_text_max_idx - ((10 * OBJ_TIMER(obj)) / max_time) : decay_text_max_idx;
        decay_level = MIN(MAX(decay_level, 0), decay_text_max_idx);

        snprintf(buf, sizeof(buf), "%s", corpse_decay_text[decay_level]);
      }

      if (decay_level < decay_text_max_idx) {
        str_cat(buf, sizeof(buf), LOW(tmp));
      }

      free(tmp);
    }

    if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_MAGIC) && (IS_AFFECTED(ch, AFF_DETECT_MAGIC) || IS_IMMORTAL(ch))) {
      str_cat(buf, sizeof(buf), "(blue)");
    }
    if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_GLOW)) {
      str_cat(buf, sizeof(buf), "(glowing)");
    }
    if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_HUM)) {
      str_cat(buf, sizeof(buf), "(humming)");
    }
    if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_INVISIBLE)) {
      str_cat(buf, sizeof(buf), "(invisible)");
    }
    if (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_CLONE)) {
      str_cat(buf, sizeof(buf), "(cloned)");
    }
    if (IS_ETERNAL(ch) && diff_obj_stats(obj)) {
      str_cat(buf, sizeof(buf), "(`iQUESTED`q)");
    }
  }

  if (num > 0) {
    tmp = strdup(buf);

    snprintf(buf, sizeof(buf), "[%d] %s", num, tmp);

    free(tmp);
  }

  str_cat(buf, sizeof(buf), "\n\r");

  page_string(GET_DESCRIPTOR(ch), buf, 1);
}


void list_obj_to_char(OBJ *list, CHAR *ch, int mode, bool show) {
  if (!ch) return;

  if (!list && show) {
    send_to_char("Nothing\n\r", ch);

    return;
  }

  OBJ *obj = list;

  while(obj) {
    if (!CAN_SEE_OBJ(ch, obj)) continue;

    int count = 1;

    OBJ *obj_ptr = obj;

    while ((obj = OBJ_NEXT_CONTENT(obj)) &&
           (OBJ_RNUM(obj) == OBJ_RNUM(obj_ptr)) &&
           (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_CLONE) == IS_SET(OBJ_EXTRA_FLAGS(obj_ptr), ITEM_CLONE)) &&
           (IS_SET(OBJ_EXTRA_FLAGS(obj), ITEM_INVISIBLE) == IS_SET(OBJ_EXTRA_FLAGS(obj_ptr), ITEM_INVISIBLE)) &&
           !strcmp(OBJ_NAME(obj), OBJ_NAME(obj_ptr)) &&
           !strcmp(OBJ_SHORT(obj), OBJ_SHORT(obj_ptr))) {
      count++;
    }

    show_obj_to_char(obj_ptr, ch, mode, count);
  }
}


#define SHOW_CHAR_IN_ROOM   0
#define SHOW_CHAR_LOOK_AT   1
#define SHOW_CHAR_INVENTORY 2

void show_char_to_char(CHAR *target, CHAR *ch, int mode) {
  char buf[MSL], buf2[MIL];

  switch (mode) {
    case SHOW_CHAR_IN_ROOM:
      if (GET_WIZINV(target) > GET_LEVEL(ch)) return;

      if (!CAN_SEE(ch, target) || IS_HIDING_FROM(ch, target)) {
        if (GET_MOUNT(target)) {
          if (GET_POS(GET_MOUNT(target)) == POSITION_FLYING) {
            act("$n is here, flown by someone.", TRUE, GET_MOUNT(target), 0, ch, TO_VICT);
          }
          else {
            act("$n is here, ridden by someone.", TRUE, GET_MOUNT(target), 0, ch, TO_VICT);
          }

          return;
        }

        if (IS_AFFECTED(ch, AFF_SENSE_LIFE) && (GET_LEVEL(ch) < LEVEL_IMM)) {
          send_to_char("You sense a hidden life form.\n\r", ch);

          return;
        }

        return;
      }

      if (GET_RIDER(target) && (GET_RIDER(target) == ch)) {
        if (GET_POS(target) == POSITION_FLYING) {
          act("$n is here, flown by you.", FALSE, target, 0, ch, TO_VICT);
        }
        else {
          act("$n is here, ridden by you.", FALSE, target, 0, ch, TO_VICT);
        }

        return;
      }

      buf[0] = '\0', buf2[0] = '\0';

      if (IS_AFFECTED(target, AFF_INVISIBLE)) {
        str_cat(buf, sizeof(buf), "*");
      }

      if (GET_QUEST_OWNER(target)) {
        if (GET_QUEST_OWNER(target) == ch) {
          str_cat(buf, sizeof(buf), "[TARGET] ");
        }
        else {
          snprintf(buf2, sizeof(buf2), "[%s's TARGET] ", GET_DISP_NAME(GET_QUEST_OWNER(target)));
          str_cat(buf, sizeof(buf), buf2);
        }
      }

      if (!IS_NPC(target) || !MOB_LONG(target) || (GET_POS(target) != GET_DEFAULT_POSITION(target))) {
        buf[0] = '\0';

        if (!IS_NPC(target)) {
          str_cat(buf, sizeof(buf), PERS(target, ch));

          if (GET_TITLE(ch)) {
            snprintf(buf2, sizeof(buf2), " %s", GET_TITLE(target));
            str_cat(buf, sizeof(buf), buf2);
          }
        }
        else {
          snprintf(buf2, sizeof(buf2), "%s", GET_SHORT(target));
          str_cat(buf, sizeof(buf), CAP(buf2));
        }

        str_cat(buf, sizeof(buf), "`q");

        if (IS_AFFECTED2(target, AFF2_SEVERED)) {
          str_cat(buf, sizeof(buf), "'s upper torso is here... twitching.");
        }
        else if (GET_DEATH_TIMER(target) >= 2) {
          str_cat(buf, sizeof(buf), " lies here... near death.");
        }
        else {
          switch (GET_POS(target)) {
            case POSITION_STUNNED:
              str_cat(buf, sizeof(buf), " is lying here, stunned.");
              break;

            case POSITION_INCAP:
              str_cat(buf, sizeof(buf), " is lying here, incapacitated.");
              break;

            case POSITION_MORTALLYW:
              str_cat(buf, sizeof(buf), " is lying here, mortally wounded.");
              break;

            case POSITION_DEAD:
              str_cat(buf, sizeof(buf), " is lying here, dead.");
              break;

            case POSITION_STANDING:
              str_cat(buf, sizeof(buf), " is standing here.");
              break;

            case POSITION_SITTING:
              str_cat(buf, sizeof(buf), " is sitting here.");
              break;

            case POSITION_RESTING:
              str_cat(buf, sizeof(buf), " is resting here.");
              break;

            case POSITION_SLEEPING:
              str_cat(buf, sizeof(buf), " is sleeping here.");
              break;

            case POSITION_FLYING:
              str_cat(buf, sizeof(buf), " is flying here.");
              break;

            case POSITION_SWIMMING:
              str_cat(buf, sizeof(buf), " is swimming here.");
              break;

            case POSITION_RIDING:
              if (GET_MOUNT(target) && (GET_POS(GET_MOUNT(target)) == POSITION_FLYING)) {
                str_cat(buf, sizeof(buf), " is here, flying on");
              }
              else {
                str_cat(buf, sizeof(buf), " is here, riding");
              }

              snprintf(buf2, sizeof(buf2), " %s.", GET_MOUNT(target) ? GET_DISP_NAME(GET_MOUNT(target)) : "something");
              str_cat(buf, sizeof(buf), buf2);
              break;

            case POSITION_FIGHTING:
              if (GET_OPPONENT(target)) {
                str_cat(buf, sizeof(buf), " is here, fighting");

                if (GET_OPPONENT(target) == ch) {
                  str_cat(buf, sizeof(buf), " YOU!");
                }
                else {
                  snprintf(buf2, sizeof(buf2), " %s.", SAME_ROOM(target, GET_OPPONENT(target)) ? GET_DISP_NAME(GET_OPPONENT(target)) : " someone who has already left.");
                  str_cat(buf, sizeof(buf), buf2);
                }
              }
              else {
                str_cat(buf, sizeof(buf), " is here, struggling with thin air.");
              }
              break;

            default:
              str_cat(buf, sizeof(buf), " is floating here.");
              break;
          }
        }

        if (IS_AFFECTED(target, AFF_INVISIBLE)) {
          str_cat(buf, sizeof(buf), " (Invisible)");
        }

        if (IS_AFFECTED(target, AFF_HIDE)) {
          str_cat(buf, sizeof(buf), " (Hiding)");
        }

        if (IS_SET(GET_PFLAG(target), PLR_KILL) && !IS_NPC(target)) {
          str_cat(buf, sizeof(buf), " (Killer)");
        }

        if (IS_SET(GET_PFLAG(target), PLR_THIEF) && !IS_NPC(target)) {
          str_cat(buf, sizeof(buf), " (Thief)");
        }

        str_cat(buf, sizeof(buf), "\n\r");
      }
      else {
        buf[0] = '\0';

        if (IS_AFFECTED(target, AFF_INVISIBLE)) {
          str_cat(buf, sizeof(buf), "*");
        }

        if (IS_AFFECTED(ch, AFF_DETECT_ALIGNMENT)) {
          if (IS_EVIL(target)) {
            str_cat(buf, sizeof(buf), "(Red Aura) ");
          }
          else if (IS_GOOD(target)) {
            str_cat(buf, sizeof(buf), "(Yellow Aura) ");
          }
        }

        if (GET_QUEST_OWNER(target)) {
          if (GET_QUEST_OWNER(target) == ch) {
            str_cat(buf, sizeof(buf), "[TARGET] ");
          }
          else {
            snprintf(buf2, sizeof(buf2), "[%s's TARGET] ", GET_DISP_NAME(GET_QUEST_OWNER(target)));
            str_cat(buf, sizeof(buf), buf2);
          }
        }

        if (IS_AFFECTED2(target, AFF2_SEVERED)) {
          str_cat(buf, sizeof(buf), "'s upper torso is here... twitching.");
        }
        else if (GET_DEATH_TIMER(target) >= 2) {
          str_cat(buf, sizeof(buf), " lies here... near death.");
        }
        else {
          str_cat(buf, sizeof(buf), MOB_LONG(target));
        }
      }

      send_to_char(buf, ch);

      /* Store 'simple' affects "all in one go"; for 'complex' affects
         (e.g. warchant, wrath of god, etc.), or enchantments, use the
         explicit method. */
      bool af_list[MAX_SPL_LIST] = { FALSE };

      for (AFF *af = target->affected; af; af = af->next) {
        af_list[af->type] = TRUE;
      }

      if (IS_SET(GET_PFLAG(target), PLR_WRITING)) {
        act("......$n is writing a message.", FALSE, target, 0, ch, TO_VICT);
      }

      if (!IS_SET(GET_PFLAG(ch), PLR_TAGBRF)) {
        if (af_list[SMELL_FARTMOUTH]) {
          act("......brown fumes waft from $n's mouth.", FALSE, target, 0, ch, TO_VICT);
        }

        if (af_list[SKILL_CAMP]) {
          act("......$n is camping here.", FALSE, target, 0, ch, TO_VICT);
        }

        if (af_list[SKILL_PRAY]) {
          act("......$n is bowing $s head in prayer.", FALSE, target, 0, ch, TO_VICT);
        }

        signal_char(target, ch, MSG_SHOW_AFFECT_TEXT, "");
      }

      if (affected_by_spell(target, SPELL_DIVINE_INTERVENTION)) {
        act("......$n is sheltered from death by a divine aura.", FALSE, target, 0, ch, TO_VICT);
      }

      if (IS_AFFECTED(target, AFF_SANCTUARY) && !af_list[SPELL_DISRUPT_SANCT]) {
        act("......$n glows with a bright light!", FALSE, target, 0, ch, TO_VICT);
      }
      else if (IS_AFFECTED(target, AFF_SANCTUARY) && af_list[SPELL_DISRUPT_SANCT]) {
        act("......$n's protective aura has been disrupted!", FALSE, target, 0, ch, TO_VICT);
      }

      if ((!IS_NPC(target) && IS_AFFECTED2(target, AFF2_FORTIFICATION)) || af_list[SPELL_FORTIFICATION]) {
        act("......$n is protected by a barrier of magical fortification!", FALSE, target, 0, ch, TO_VICT);
      }

      if ((!IS_NPC(target) && IS_AFFECTED(target, AFF_FURY)) || af_list[SPELL_FURY]) {
        act("......$n is snarling and fuming with fury!", FALSE, target, 0, ch, TO_VICT);
      }

      if ((!IS_NPC(target) && IS_AFFECTED2(target, AFF2_RAGE)) || af_list[SPELL_RAGE]) {
        act("......$n is seething with hatred and rage!", FALSE, target, 0, ch, TO_VICT);
      }

      if (IS_AFFECTED(target, AFF_SPHERE) && !af_list[SPELL_DISTORTION]) {
        act("......$n is surrounded by a golden sphere!", FALSE, target, 0, ch, TO_VICT);
      }
      else if (IS_AFFECTED(target, AFF_SPHERE) && af_list[SPELL_DISTORTION]) {
        act("......$n's golden sphere seems to shimmer and blur in weakness!", FALSE, target, 0, ch, TO_VICT);
      }

      if (IS_AFFECTED(target, AFF_INVUL)) {
        act("......$n is surrounded by a powerful sphere!", FALSE, target, 0, ch, TO_VICT);
      }

      if (!IS_SET(GET_PFLAG(ch), PLR_TAGBRF)) {
        if (af_list[SPELL_IRON_SKIN]) {
          act("......$n's skin is as hard and impervious as iron.", FALSE, target, 0, ch, TO_VICT);
        }

        if (af_list[SPELL_ORB_PROTECTION]) {
          act("......a shield of power emanates from an orb above $n's head.", FALSE, target, 0, ch, TO_VICT);
        }

        if (af_list[SPELL_BLADE_BARRIER]) {
          act("......$n is surrounded by a barrier of whirling blades.", FALSE, target, 0, ch, TO_VICT);
        }

        if (af_list[SPELL_DESECRATE]) {
          act("......$n's sinister presence desecrates the surroundings.", FALSE, target, 0, ch, TO_VICT);
        }

        if (af_list[SPELL_BLACKMANTLE]) {
          act("......$n is surrounded by an eerie mantle of darkness.", FALSE, target, 0, ch, TO_VICT);
        }

        if (af_list[SPELL_QUICK]) {
          act("......$n's words and gestures are magically quickened.", FALSE, target, 0, ch, TO_VICT);
        }

        if (af_list[SPELL_HASTE]) {
          act("......$n's hastened actions move faster than reality.", FALSE, target, 0, ch, TO_VICT);
        }

        if (af_list[SPELL_MYSTIC_SWIFTNESS]) {
          act("......$n's hands move with mystical swiftness.", FALSE, target, 0, ch, TO_VICT);
        }

        if (af_list[SPELL_BLUR]) {
          act("......$n's movements blur in and out of reality.", FALSE, target, 0, ch, TO_VICT);
        }

        if (af_list[SPELL_RUSH]) {
          act("......$n's pulse rushes with supernatural speed.", FALSE, target, 0, ch, TO_VICT);
        }

        if (af_list[SPELL_BLOOD_LUST]) {
          act("......$n thirsts for blood and lusts for carnage.", FALSE, target, 0, ch, TO_VICT);
        }

        if (af_list[SPELL_SHADOW_WRAITH]) {
          int duration = duration_of_spell(target, SPELL_SHADOW_WRAITH);

          if (duration >= 0) {
            act("......$n's shadow stretches to the north.", FALSE, target, 0, ch, TO_VICT);
          }

          if (duration > (10 * 1)) {
            act("......$n's shadow stretches to the south.", FALSE, target, 0, ch, TO_VICT);
          }

          if (duration > (10 * 2)) {
            act("......$n's shadow stretches to the east.", FALSE, target, 0, ch, TO_VICT);
          }

          if (duration > (10 * 3)) {
            act("......$n's shadow stretches to the west.", FALSE, target, 0, ch, TO_VICT);
          }
        }

        /* Inner Peace */
        if (af_list[SPELL_BLINDNESS] && !(IS_MORTAL(target) && check_subclass(target, SC_MYSTIC, 3))) {
          act("......$n stumbles about wildly!", FALSE, target, 0, ch, TO_VICT);
        }

        if (af_list[SPELL_PARALYSIS]) {
          act("......$n is completely immobilized!", FALSE, target, 0, ch, TO_VICT);
        }

        if (af_list[SPELL_HOLD]) {
          act("......$n is rooted to the ground!", FALSE, target, 0, ch, TO_VICT);
        }

        if (af_list[SPELL_DEBILITATE]) {
          act("......$n is enveloped by a greenish smoke.", FALSE, target, 0, ch, TO_VICT);
        }

        if (af_list[SPELL_CLOUD_CONFUSION]) {
          act("......$n drools absentmindedly.", FALSE, target, 0, ch, TO_VICT);
        }

        if (af_list[SPELL_INCENDIARY_CLOUD] || af_list[SPELL_INCENDIARY_CLOUD_NEW]) {
          act("......$n is enveloped by a huge ball of flame!", FALSE, target, 0, ch, TO_VICT);
        }

        for (AFF *temp_aff = target->affected; temp_aff; temp_aff = temp_aff->next) {
          if ((temp_aff->type == SPELL_WARCHANT) && (temp_aff->location == APPLY_HITROLL) && (temp_aff->modifier < 0)) {
            act("......$n hears the sound of war!", FALSE, target, 0, ch, TO_VICT);

            break;
          }
        }

        for (AFF *temp_aff = target->affected; temp_aff; temp_aff = temp_aff->next) {
          if ((temp_aff->type == SPELL_WRATH_OF_GOD) && (temp_aff->location == APPLY_DAMROLL)) {
            if (temp_aff->modifier <= -50) {
              act("......$n is being crushed by the wrath of gods!", FALSE, target, 0, ch, TO_VICT);
            }
            else if (temp_aff->modifier <= -40) {
              act("......$n is tormented by a holy force of will!", FALSE, target, 0, ch, TO_VICT);
            }
            else if (temp_aff->modifier <= -30) {
              act("......$n is persecuted by celestial powers!", FALSE, target, 0, ch, TO_VICT);
            }
            else if (temp_aff->modifier <= -20) {
              act("......$n is oppressed by divine condemnation!", FALSE, target, 0, ch, TO_VICT);
            }
            else {
              act("......$n is suffering from righteous indignation!", FALSE, target, 0, ch, TO_VICT);
            }

            break;
          }
        }

        if (target->enchantments) {
          if (enchanted_by(target, "Savaged (Maim)")) {
            act("......$n has been horribly maimed!", FALSE, target, 0, ch, TO_VICT);
          }

          if (enchanted_by(target, "Staggering (Tremor)")) {
            act("......$n is stumbling and staggering!", FALSE, target, 0, ch, TO_VICT);
          }
        }

        if (IS_NPC(target)) {
          if (mob_proto_table[target->nr].tagline) {
            for (struct tagline_data *tag = mob_proto_table[target->nr].tagline; tag; tag = tag->next) {
              act(tag->desc, FALSE, target, 0, ch, TO_VICT);
            }
          }
        }
      }
      break;

    case SHOW_CHAR_LOOK_AT:
      if (IS_NPC(target) && IS_IMMORTAL(ch)) {
        snprintf(buf, sizeof(buf), "WIZINFO: %s looks at %s",
          GET_NAME(ch),
          GET_SHORT(target));
        wizlog(buf, GET_LEVEL(ch) + 1, 5);
        log_s(buf);
      }

      if (IS_NPC(target) && IS_AFFECTED(target, AFF_STATUE) && !IS_IMMORTAL(ch)) {
        if (signal_char(ch, ch, MSG_STONE, "")) return;

        time_t rawtime;
        time(&rawtime);

        send_to_char("When you look into its eyes, you slowly turn into a statue.\n\r", ch);

        if (GET_OPPONENT(target) && (GET_OPPONENT(target) == ch)) {
          stop_fighting(target);
        }

        if (GET_OPPONENT(target)) {
          stop_fighting(ch);
        }

        act("$n screams in pain as $e slowly turns to stone.", FALSE, ch, 0, target, TO_ROOM);

        if (ROOM_CHAOTIC(CHAR_REAL_ROOM(ch)) && IS_MORTAL(ch)) {
          snprintf(buf, sizeof(buf), "%s stoned by %s at %s",
            GET_NAME(ch),
            GET_SHORT(target),
            world[CHAR_REAL_ROOM(ch)].name);

          for (DESC *temp_desc = descriptor_list; temp_desc; temp_desc = temp_desc->next) {
            if (!temp_desc->connected) {
              act(buf, 0, temp_desc->character, 0, 0, TO_CHAR);
            }
          }

          number_of_kills++;

          if (number_of_kills < 100) {
            snprintf(scores[number_of_kills].killer, sizeof(scores[number_of_kills].killer), "%s",
              IS_NPC(target) ? MOB_SHORT(target) : GET_NAME(target));
            snprintf(scores[number_of_kills].killed, sizeof(scores[number_of_kills].killed), "%s",
              GET_NAME(ch));
            snprintf(scores[number_of_kills].location, sizeof(scores[number_of_kills].location), "%s",
              world[CHAR_REAL_ROOM(ch)].name);
            snprintf(scores[number_of_kills].time_txt, sizeof(scores[number_of_kills].time_txt), "%s",
              asctime(localtime(&rawtime)));

            if (number_of_kills == CHAOSDEATH) {
              snprintf(buf, sizeof(buf), "**** Kill number %d has been reached, we have a winner!!! ****\n\r\n\r", CHAOSDEATH);
              send_to_all(buf);
              send_to_all(buf);
            }
          }
          else {
            number_of_kills = 99;
          }
        }
        else {
          snprintf(buf, sizeof(buf), "%s stoned by %s at %s (%d)",
            GET_NAME(ch),
            GET_SHORT(target),
            world[CHAR_REAL_ROOM(ch)].name,
            world[CHAR_REAL_ROOM(ch)].number);
          wizlog(buf, LEVEL_IMM, 3);
        }

        log_s(buf);
        deathlog(buf);

        GET_QUEST_GIVER(ch) = NULL;
        if (GET_QUEST_OBJ(ch)) OBJ_OWNED_BY(GET_QUEST_OBJ(ch)) = NULL;
        GET_QUEST_OBJ(ch) = NULL;
        if (GET_QUEST_MOB(ch)) GET_QUEST_OWNER(GET_QUEST_MOB(ch)) = NULL;
        GET_QUEST_MOB(ch) = NULL;
        GET_QUEST_LEVEL(ch) = 0;
        if (GET_QUEST_STATUS(ch) != QUEST_FAILED) GET_QUEST_TIMER(ch) = 30;
        GET_QUEST_STATUS(ch) = QUEST_NONE;

        death_cry(ch);
        make_statue(ch);
        save_char(ch, NOWHERE);
        extract_char(ch);

        return;
      }

      if (MOB_DESCRIPTION(target)) {
        send_to_char(MOB_DESCRIPTION(target), ch);
      }
      else {
        act("You see nothing special about $m.", FALSE, target, 0, ch, TO_VICT);
      }

      buf[0] = '\0';

      str_cat(buf, sizeof(buf), GET_DISP_NAME(target));

      int percent = 0;

      if (GET_MAX_HIT(target) > 0) {
        percent = (100 * GET_HIT(target)) / GET_MAX_HIT(target);
      }
      else {
        percent = -1;
      }

      if (percent >= 100) {
        str_cat(buf, sizeof(buf), " is in an excellent condition.\n\r");
      }
      else if (percent >= 90) {
        str_cat(buf, sizeof(buf), " has a few scratches.\n\r");
      }
      else if (percent >= 75) {
        str_cat(buf, sizeof(buf), " has some small wounds and bruises.\n\r");
      }
      else if (percent >= 50) {
        str_cat(buf, sizeof(buf), " has quite a few wounds.\n\r");
      }
      else if (percent >= 30) {
        str_cat(buf, sizeof(buf), " has some big nasty wounds and scratches.\n\r");
      }
      else if (percent >= 15) {
        str_cat(buf, sizeof(buf), " looks pretty hurt.\n\r");
      }
      else if (percent >= 0) {
        str_cat(buf, sizeof(buf), " is in an awful condition.\n\r");
      }
      else {
        str_cat(buf, sizeof(buf), " is bleeding awfully from big wounds.\n\r");
      }

      send_to_char(buf, ch);

      bool found = FALSE;

      for (int i = 0; i < MAX_WEAR; i++) {
        OBJ *obj = EQ(target, i);

        if (!obj || !CAN_SEE_OBJ(ch, obj)) continue;

        found = TRUE;
        break;
      }

      if (found) {
        act("\n\r$n is using:", FALSE, target, 0, ch, TO_VICT);

        for (int i = 0; i < MAX_WEAR; i++) {
          OBJ *obj = EQ(target, i);

          if (!obj || !CAN_SEE_OBJ(ch, obj)) continue;

          send_to_char(where[i], ch);
          show_obj_to_char(obj, ch, 1, 0);

          const int EMITH_CAPE_1 = 26709;
          const int EMITH_CAPE_2 = 26719;

          switch (i) {
            case WEAR_NECK_1:
              /* ITEM_WEAR_2NECK and Emith cape uses both neck slots, similar to a 2-handed weapon. */
              if (CAN_WEAR(obj, ITEM_WEAR_2NECK) || (V_OBJ(obj) == EMITH_CAPE_1) || (V_OBJ(obj) == EMITH_CAPE_2)) {
                printf_to_char(ch, "%s********\n\r", where[WEAR_NECK_2]);
                i++; /* Skip WEAR_NECK_2. This assumes WEAR_NECK_2 is always immediately after WEAR_NECK_1. */
              }
              break;

            case WIELD:
              if ((OBJ_TYPE(obj) == ITEM_2H_WEAPON) &&
                !(IS_MORTAL(target) && check_subclass(target, SC_MERCENARY, 5) && GET_WEAPON2(target))) { /* Sidearm */
                printf_to_char(ch, "%s********\n\r", where[i]);
                i++; /* Skip HOLD. This assumes HOLD is always immediately after WIELD. */
              }
              break;
          }
        }
      }

      if (GET_MOUNT(target)) {
        printf_to_char(ch, "<riding>             %s\n\r", GET_SHORT(GET_MOUNT(target)));
      }

      if ((IS_IMMORTAL(ch) && (GET_LEVEL(ch) > LEVEL_DEI)) ||
        (IS_MORTAL(ch) && (GET_CLASS(ch) == CLASS_THIEF) && (ch != target))) {
        found = FALSE;

        if (ch != target) {
          act("\n\rYou attempt to peek at $s inventory:\n\r", FALSE, target, 0, ch, TO_VICT);
        }
        else {
          send_to_char("\n\rYou are carrying:\n\r", ch);
        }

        for (OBJ *temp_obj = target->carrying; temp_obj; temp_obj = temp_obj->next_content) {
          if ((OBJ_TYPE(temp_obj) == ITEM_SC_TOKEN) && !(IS_IMMORTAL(ch) && GET_LEVEL(ch) >= LEVEL_SUP)) continue;
          if (!CAN_SEE_OBJ(ch, temp_obj) || (number(0, 40) > GET_LEVEL(ch))) continue;

          found = TRUE;

          show_obj_to_char(temp_obj, ch, 1, 0);
        }

        if (!found) {
          if (ch != target) {
            send_to_char("You can't see anything.\n\r", ch);
          }
          else {
            send_to_char("Nothing\n\r", ch);
          }
        }
      }
      break;

    case SHOW_CHAR_INVENTORY:
      act("$n is carrying:", FALSE, target, 0, ch, TO_VICT);
      list_obj_to_char(target->carrying, ch, 1, TRUE);
      break;
  }
}


void list_char_to_char(CHAR *list, CHAR *ch, int mode) {
  for (CHAR *temp_ch = list, *next_ch; temp_ch; temp_ch = next_ch) {
    next_ch = temp_ch->next_in_room;

    if (ch != temp_ch) {
      show_char_to_char(temp_ch, ch, 0);
    }
  }
}


bool show_object_extra_desc(OBJ *obj, CHAR *ch, char *arg) {
  if (!obj || !ch || !CAN_SEE_OBJ(ch, obj)) return FALSE;

  struct extra_descr_data *ex_desc_list = NULL;

  if (OBJ_GET_EX_DESC(obj)) {
    ex_desc_list = OBJ_GET_EX_DESC(obj);
  }
  else if ((OBJ_RNUM(obj) >= 0) && (OBJ_RNUM(obj) <= top_of_objt)) {
    ex_desc_list = OBJ_PROTO_GET_EX_DESC(obj);
  }

  if (!ex_desc_list) return FALSE;

  char *ex_desc = find_ex_description(arg, ex_desc_list);

  if (!ex_desc) return FALSE;

  /* A "window description" is a special extra description composed of a room virtual number.
     When the description is shown to a character, they will see the description of the room that matches the virtual number. */
  int window = atoi(ex_desc);

  if (window > 0) {
    look_in_room(ch, window);
  }
  else {
    page_string(GET_DESCRIPTOR(ch), ex_desc, 1);

    if (IS_SET(OBJ_EXTRA_FLAGS2(obj), ITEM_ALL_DECAY) || IS_SET(OBJ_EXTRA_FLAGS2(obj), ITEM_EQ_DECAY)) {
      const int decay_string_max_idx = NUMELEMS(object_decay_text) - 1;

      int decay_level = OBJ_PROTO_TIMER(obj) ? decay_string_max_idx - ((10 * OBJ_TIMER(obj)) / OBJ_PROTO_TIMER(obj)) : decay_string_max_idx;

      decay_level = MIN(MAX(decay_level, 0), decay_string_max_idx);

      printf_to_char(ch, "This item decays and seems %s.", object_decay_text[decay_level]);
    }
  }

  return TRUE;
}


void do_look(CHAR *ch, char *argument, int cmd) {
  const char * const keywords[] = {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "in",
    "at",
    "",
    "\n"
  };

  if (!ch || !GET_DESCRIPTOR(ch)) return;

  if (GET_POS(ch) < POSITION_SLEEPING) {
    send_to_char("You can't see anything but stars!\n\r", ch);

    return;
  }

  if (GET_POS(ch) == POSITION_SLEEPING) {
    send_to_char("You can't see anything; you're sleeping!\n\r", ch);

    return;
  }

  if (!IS_IMMORTAL(ch) && IS_AFFECTED(ch, AFF_BLIND) && !(IS_MORTAL(ch) && check_subclass(ch, SC_MYSTIC, 2))) {
    send_to_char("You can't see anything; you're blind!\n\r", ch);

    return;
  }

  if (!IS_IMMORTAL(ch) && IS_DARK(CHAR_REAL_ROOM(ch)) && !IS_AFFECTED(ch, AFF_INFRAVISION)) {
    send_to_char("It is pitch black...\n\r", ch);

    return;
  }

  char arg1[MIL], arg2[MIL];

  two_arguments_ex(argument, arg1, sizeof(arg1), arg2, sizeof(arg2), TRUE);

  int keyword_idx = search_block_ex(arg1, keywords, FALSE, FALSE);

  /* Default to "look at" if no keyword was found. */
  if (keyword_idx < 0) {
    keyword_idx = 7;

    /* Copy arg1 to arg2 because arg1 would normally be "at". */
    strcpy(arg2, arg1);
  }

  switch (keyword_idx) {
    case NORTH:
    case EAST:
    case WEST:
    case SOUTH:
    case UP:
    case DOWN:
      if (EXIT(ch, keyword_idx)) {
        if (EXIT(ch, keyword_idx)->general_description) {
          send_to_char(EXIT(ch, keyword_idx)->general_description, ch);
        }
        else {
          send_to_char("You see nothing special.\n\r", ch);
        }

        if (EXIT(ch, keyword_idx)->keyword) {
          if (IS_SET(EXIT(ch, keyword_idx)->exit_info, EX_CLOSED)) {
            printf_to_char(ch, "The %s is closed.\n\r", fname(EXIT(ch, keyword_idx)->keyword));
          }
          else {
            printf_to_char(ch, "The %s is open.\n\r", fname(EXIT(ch, keyword_idx)->keyword));
          }
        }
      }
      else {
        send_to_char("Nothing special there...\n\r", ch);
      }
    break;

    case 6: /* look in */
    {
      if (*arg2) {
        OBJ *temp_obj;

        int bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, NULL, &temp_obj);

        if (temp_obj) {
          switch (OBJ_TYPE(temp_obj)) {
            case ITEM_CONTAINER:
            case ITEM_AQ_ORDER:
              if (!IS_SET(OBJ_VALUE(temp_obj, 1), CONT_CLOSED) || (OBJ_TYPE(temp_obj) == ITEM_AQ_ORDER)) {
                switch (bits) {
                  case FIND_OBJ_INV:
                    printf_to_char(ch, "%s (carried) :\n\r", fname(OBJ_NAME(temp_obj)));
                    break;
                  case FIND_OBJ_ROOM:
                    printf_to_char(ch, "%s (here) :\n\r", fname(OBJ_NAME(temp_obj)));
                    break;
                  case FIND_OBJ_EQUIP:
                    printf_to_char(ch, "%s (used) :\n\r", fname(OBJ_NAME(temp_obj)));
                    break;
                }

                list_obj_to_char(OBJ_CONTAINS(temp_obj), ch, 2, TRUE);
              }
              else {
                send_to_char("It is closed.\n\r", ch);
              }
              break;

            case ITEM_DRINKCON:
              if (OBJ_VALUE(temp_obj, 1)) {
                printf_to_char(ch, "It's %sfull of %s %s liquid.\n\r",
                  fullness[(OBJ_VALUE(temp_obj, 1) * 3) / OBJ_VALUE(temp_obj, 0)],
                  S_ANA(color_liquid[OBJ_VALUE(temp_obj, 2)]),
                  color_liquid[OBJ_VALUE(temp_obj, 2)]);
              }
              else {
                send_to_char("It is empty.\n\r", ch);
              }
              break;

            default:
              send_to_char("That is not a container.\n\r", ch);
          }
        }
        else {
          send_to_char("You do not see that item here.\n\r", ch);
        }
      }
      else {
        send_to_char("Look in what?\n\r", ch);
      }
    }
    break;

    case 7: /* look at */
    {
      if (*arg2) {
        CHAR *temp_ch;
        OBJ *temp_obj;
        char *room_extra_desc;

        generic_find(arg2, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &temp_ch, &temp_obj);

        /* Show the character found in the room. Keep this first, to ensure characters take precedence. */
        if (temp_ch) {
          if (temp_ch != ch) {
            act("$n looks at you.", TRUE, ch, 0, temp_ch, TO_VICT);
            act("$n looks at $N.", TRUE, ch, 0, temp_ch, TO_NOTVICT);
          }

          show_char_to_char(temp_ch, ch, 1);
        }
        /* Show the extra description found in the room. */
        else if ((room_extra_desc = find_ex_description(arg2, ROOM_GET_EXTRA_DESC(CHAR_REAL_ROOM(ch))))) {
          char buf[MSL];

          snprintf(buf, sizeof(buf), "$n looks at the %s.", arg2);

          act(buf, TRUE, ch, 0, 0, TO_ROOM);

          /* A "window description" is a special extra description composed of a room virtual number.
              When the description is shown to a character, they will see the description of the room that matches the virtual number. */
          int window = atoi(room_extra_desc);

          if (window > 0) {
            look_in_room(ch, window);
          }
          else {
            page_string(GET_DESCRIPTOR(ch), room_extra_desc, 0);
          }
        }
        /* Show the object found. */
        else if (temp_obj) {
          /* Show the object extra description, if one exist. */
          if (!show_object_extra_desc(temp_obj, ch, arg2)) {
            show_obj_to_char(temp_obj, ch, 5, 0); /* Show without description. */
          }
          else {
            show_obj_to_char(temp_obj, ch, 6, 0); /* Show only glowing, humming, etc. */
          }
        }
        else {
          send_to_char("You do not see that here.\n\r", ch);
        }
      }
      else {
        send_to_char("Look at what?\n\r", ch);
      }
    }
    break;

    case 8:
      look_in_room(ch, V_ROOM(ch));
    break;
  }
}


#define ROOM_IMMORTAL_KITCHEN 1203
#define ROOM_DONATION_ROOM    3084

void look_in_room(CHAR *ch, int vnum) {
  const char *keywords[] = {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down"
  };

  const char *blood_messages[] = {
    "There's a little blood here.",
    "You're standing in some blood.",
    "The blood here is flowing.",
    "There's so much blood here it's intoxicating!",
    "How much more blood can there be in any one place?",
    "Such carnage. The God of Death is feasting tonight!",
    "You are splashing around in the blood of the slain!",
    "Even the spirits are revolted by the death and destruction!",
    "The Gods should show some mercy and cleanse this horrid place!",
    "So much blood has been shed here, you are drowning in it!"
  };

  char buf[MSL], tmp[MIL];

  int room = real_room(vnum);

  if (room < 0) {
    send_to_char("All you see is an endless void.\n\r", ch);

    return;
  }

  COLOR(ch, 3);
  send_to_char(ROOM_NAME(room), ch);
  ENDCOLOR(ch);

  buf[0] = '\0';

  if (IS_SET(ROOM_FLAGS(room), ARENA)) str_cat(buf, sizeof(buf), " [*]");

  if (IS_IMMORTAL(ch)) {
    snprintf(tmp, sizeof(tmp), " [%d]", ROOM_VNUM(room));
    str_cat(buf, sizeof(buf), tmp);

    if (IS_SET(ROOM_FLAGS(room), LIT)) str_cat(buf, sizeof(buf), " (LIT)");
    if (IS_SET(ROOM_FLAGS(room), DARK)) str_cat(buf, sizeof(buf), " (DARK)");
    if (IS_SET(ROOM_FLAGS(room), INDOORS)) str_cat(buf, sizeof(buf), " (INDOORS)");
    if (IS_SET(ROOM_FLAGS(room), FLYING)) str_cat(buf, sizeof(buf), " (FLYING)");
    if (IS_SET(ROOM_FLAGS(room), TUNNEL)) str_cat(buf, sizeof(buf), " (TUNNEL)");
    if (IS_SET(ROOM_FLAGS(room), TRAP)) str_cat(buf, sizeof(buf), " (TRAP)");
    if (IS_SET(ROOM_FLAGS(room), CLUB)) str_cat(buf, sizeof(buf), " (CLUB)");
    if (IS_SET(ROOM_FLAGS(room), LAWFUL)) str_cat(buf, sizeof(buf), " (LAWFUL)");
    if (IS_SET(ROOM_FLAGS(room), CHAOTIC)) str_cat(buf, sizeof(buf), " (CHAOTIC)");
    if (IS_SET(ROOM_FLAGS(room), PRIVATE)) str_cat(buf, sizeof(buf), " (PRIVATE)");
    if (IS_SET(ROOM_FLAGS(room), QUIET)) str_cat(buf, sizeof(buf), " (QUIET)");
    if (IS_SET(ROOM_FLAGS(room), LOCK)) str_cat(buf, sizeof(buf), " (LOCK)");
    if (IS_SET(ROOM_FLAGS(room), SAFE)) str_cat(buf, sizeof(buf), " (SAFE)");
    if (IS_SET(ROOM_FLAGS(room), ARENA)) str_cat(buf, sizeof(buf), " (ARENA)");
    if (IS_SET(ROOM_FLAGS(room), HAZARD)) str_cat(buf, sizeof(buf), " (HAZARD)");
    if (IS_SET(ROOM_FLAGS(room), DEATH)) str_cat(buf, sizeof(buf), " (DEATH)");
    if (IS_SET(ROOM_FLAGS(room), NO_MOB)) str_cat(buf, sizeof(buf), " (NO_MOB)");
    if (IS_SET(ROOM_FLAGS(room), NO_MAGIC)) str_cat(buf, sizeof(buf), " (NO_MAGIC)");
    if (IS_SET(ROOM_FLAGS(room), NO_SUM)) str_cat(buf, sizeof(buf), " (NO_SUMMON)");
    if (IS_SET(ROOM_FLAGS(room), NO_BEAM)) str_cat(buf, sizeof(buf), " (NO_BEAM)");
    if (IS_SET(ROOM_FLAGS(room), NO_PEEK)) str_cat(buf, sizeof(buf), " (NO_PEEK)");
    if (IS_SET(ROOM_FLAGS(room), NO_SONG)) str_cat(buf, sizeof(buf), " (NO_SONG)");
    if (IS_SET(ROOM_FLAGS(room), NO_ORB)) str_cat(buf, sizeof(buf), " (NO_ORB)");
    if (IS_SET(ROOM_FLAGS(room), NO_QUAFF)) str_cat(buf, sizeof(buf), " (NO_QUAFF)");
    if (IS_SET(ROOM_FLAGS(room), NO_REGEN)) str_cat(buf, sizeof(buf), " (NO_REGEN)");
    if (IS_SET(ROOM_FLAGS(room), REV_REGEN)) str_cat(buf, sizeof(buf), " (REV_REGEN)");
    if (IS_SET(ROOM_FLAGS(room), MANA_DRAIN)) str_cat(buf, sizeof(buf), " (MANA_DRAIN)");
    if (IS_SET(ROOM_FLAGS(room), DOUBLE_MANA)) str_cat(buf, sizeof(buf), " (DOUBLE_MANA)");
    if (IS_SET(ROOM_FLAGS(room), MOVE_TRAP)) str_cat(buf, sizeof(buf), " (MOVE_TRAP)");
    if (IS_SET(ROOM_FLAGS(room), HALF_CONC)) str_cat(buf, sizeof(buf), " (HALF_CONC)");
    if (IS_SET(ROOM_FLAGS(room), QRTR_CONC)) str_cat(buf, sizeof(buf), " (QRTR_CONC)");
  }

  printf_to_char(ch, "%s\n\r", buf);

  if (!IS_SET(GET_PFLAG(ch), PLR_BRIEF)) {
    send_to_char(ROOM_DESC(room), ch);

    int blood_level = MIN(MAX(ROOM_BLOOD(room), 0), NUMELEMS(blood_messages));

    if (blood_level) {
      printf_to_char(ch, "   %s\n\r", blood_messages[blood_level - 1]);
    }
  }

  send_to_char("   Obvious Exits:", ch);

  buf[0] = '\0';
  int dir_count = 0;
  for (int dir = NORTH; dir <= DOWN; dir++) {
    if (world[room].dir_option[dir]) {
      if ((world[room].dir_option[dir]->to_room_r != NOWHERE) &&
          (world[room].dir_option[dir]->to_room_r != real_room(0)) &&
          !IS_SET(world[room].dir_option[dir]->exit_info, EX_CLOSED) &&
          !IS_SET(world[room].dir_option[dir]->exit_info, EX_CLIMB) &&
          !IS_SET(world[room].dir_option[dir]->exit_info, EX_JUMP) &&
          !IS_SET(world[room].dir_option[dir]->exit_info, EX_CRAWL) &&
          !IS_SET(world[room].dir_option[dir]->exit_info, EX_ENTER)) {
        dir_count++;

        snprintf(tmp, sizeof(tmp), " [%s]", keywords[dir]);
        str_cat(buf, sizeof(buf), tmp);
      }
    }
  }

  if (!dir_count) {
    str_cat(buf, sizeof(buf), "  None.");
  }

  COLOR(ch, 4);
  printf_to_char(ch, "%s\n\r", buf);
  ENDCOLOR(ch);

  if (IS_IMMORTAL(ch)) {
    send_to_char("           Doors:", ch);

    buf[0] = '\0';
    dir_count = 0;
    for (int dir = NORTH; dir <= DOWN; dir++) {
      if (world[room].dir_option[dir]) {
        if ((world[room].dir_option[dir]->to_room_r != NOWHERE) &&
            (IS_SET(world[room].dir_option[dir]->exit_info, EX_CLOSED))) {
          dir_count++;

          snprintf(tmp, sizeof(tmp), " [%s]", keywords[dir]);
          str_cat(buf, sizeof(buf), tmp);
        }
      }
    }

    if (!dir_count) {
      str_cat(buf, sizeof(buf), "  None.");
    }

    COLOR(ch, 4);
    printf_to_char(ch, "%s\n\r", buf);
    ENDCOLOR(ch);
  }

  if (IS_ETERNAL(ch)) {
    printf_to_char(ch, "     Sector Type:  %s (%d)\n\r",
      sector_types[ROOM_SECTOR_TYPE(room)], movement_loss[ROOM_SECTOR_TYPE(room)]);
  }
  else if (IS_IMMORTAL(ch) || (IS_MORTAL(ch) && !IS_SET(GET_PFLAG(ch), PLR_SECTOR_BRIEF))) {
    printf_to_char(ch, "     Sector Type:  %s\n\r",
      (ROOM_SECTOR_TYPE(room) < SECT_WATER_SWIM || ROOM_SECTOR_TYPE(room) > SECT_WATER_NOSWIM) ? sector_types[ROOM_SECTOR_TYPE(room)] : "Water");
  }

  if (IS_WIZARD(ch)) {
    send_to_char("     Extra Exits:", ch);

    buf[0] = '\0';
    dir_count = 0;
    for (int dir = NORTH; dir <= DOWN; dir++) {
      if (world[room].dir_option[dir]) {
        if ((world[room].dir_option[dir]->to_room_r != NOWHERE) &&
            (world[room].dir_option[dir]->to_room_r != real_room(0)) &&
            (IS_SET(world[room].dir_option[dir]->exit_info, EX_CRAWL) ||
             IS_SET(world[room].dir_option[dir]->exit_info, EX_JUMP) ||
             IS_SET(world[room].dir_option[dir]->exit_info, EX_ENTER) ||
             IS_SET(world[room].dir_option[dir]->exit_info, EX_CLIMB))) {
          dir_count++;

          snprintf(tmp, sizeof(tmp), " [%s]", keywords[dir]);
          str_cat(buf, sizeof(buf), tmp);
        }
      }
    }

    if (!dir_count) {
      str_cat(buf, sizeof(buf), "  None.");
    }

    COLOR(ch, 4);
    printf_to_char(ch, "%s\n\r", buf);
    ENDCOLOR(ch);
  }

  if (!IS_SET(GET_PFLAG(ch), PLR_SUPERBRF)) {
    if (ROOM_VNUM(room) == ROOM_DONATION_ROOM) {
      int item_count = 0;

      for (OBJ *temp_obj = ROOM_CONTENTS(room); temp_obj; temp_obj = OBJ_NEXT_CONTENT(temp_obj)) {
        if (CAN_SEE_OBJ(ch, temp_obj) && (OBJ_TYPE(temp_obj) != ITEM_BOARD)) item_count++;
      }

      if (item_count) {
        printf_to_char(ch, "There are `i[%d]`q donated items here, use `i'LIST'`q to see them.\n\r", item_count);
      }

      send_to_char("[1] An Adventure bulletin board is mounted on a wall here.(glowing)\n\r", ch);
    }
    else {
      list_obj_to_char(ROOM_CONTENTS(room), ch, 0, FALSE);
    }

    if (ROOM_VNUM(room) != ROOM_IMMORTAL_KITCHEN) {
      list_char_to_char(ROOM_PEOPLE(room), ch, 0);
    }
  }
}


void do_read(struct char_data *ch, char *argument, int cmd) {
  char buf[MIL+5];

  /* This is just for now - To be changed later.! */

  sprintf(buf,"at %s",argument);
  do_look(ch,buf,15);
}

void do_examine(struct char_data *ch, char *argument, int cmd) {
  char name[MIL], buf[MIL+5];
  struct char_data *tmp_char;
  struct obj_data *tmp_object;

  sprintf(buf,"at %s",argument);
  do_look(ch,buf,15);

  if(CHAR_REAL_ROOM(ch)==NOWHERE) return;  /* Character died - happens when
          a stoning mob has the same keyword as an object in the room being
          examined - Ranger March 98 */

  one_argument(argument, name);

  if (!*name) {
    send_to_char("Examine what?\n\r", ch);
    return;
  }

  generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM |
          FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

  if (tmp_object) {
    if ((OBJ_TYPE(tmp_object)==ITEM_DRINKCON) ||
        (OBJ_TYPE(tmp_object)==ITEM_CONTAINER) ||
        (OBJ_TYPE(tmp_object)==ITEM_AQ_ORDER)) {
      send_to_char("When you look inside, you see:\n\r", ch);
      sprintf(buf,"in %s",argument);
      do_look(ch,buf,15);
    }
  }
}

void do_exits(struct char_data *ch, char *argument, int cmd) {
  int door;
  char buf[MSL];
  char *exits[] =
    {
      "North",
      "East ",
      "South",
      "West ",
      "Up   ",
      "Down "
    };

  *buf = '\0';
  if (GET_LEVEL(ch) < LEVEL_IMM) {
    for (door = 0; door <= 5; door++)
      if (EXIT(ch, door))
   if (EXIT(ch, door)->to_room_r != NOWHERE && EXIT(ch, door)->to_room_r != real_room(0) &&
      !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)) {
    if (IS_DARK(EXIT(ch, door)->to_room_r) && !IS_AFFECTED(ch, AFF_INFRAVISION))
      sprintf(buf + strlen(buf), "%s - Too dark to tell\n\r",exits[door]);
    else
      sprintf(buf + strlen(buf), "%s - %s\n\r", exits[door],world[EXIT(ch, door)->to_room_r].name);
   }
    send_to_char("Obvious exits:\n\r", ch);
  } else {
    for (door = 0; door <= 5; door++)
      if (EXIT(ch, door))
  if (EXIT(ch, door)->to_room_r != NOWHERE &&
            EXIT(ch, door)->to_room_r != real_room(0) &&
      !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED)
        /*  && !IS_SET(EXIT(ch, door)->exit_info,EX_CRAWL)
          && !IS_SET(EXIT(ch, door)->exit_info,EX_JUMP)
          && !IS_SET(EXIT(ch, door)->exit_info,EX_ENTER)
          && !IS_SET(EXIT(ch, door)->exit_info,EX_CLIMB)*/) {

    sprintf(buf + strlen(buf), "%s - %s [%d]", exits[door],
      world[EXIT(ch, door)->to_room_r].name,
      world[EXIT(ch, door)->to_room_r].number);
    if (IS_SET(world[EXIT(ch, door)->to_room_r].room_flags, DARK))
      strcat(buf, " (DARK)");
    if (IS_SET(world[EXIT(ch, door)->to_room_r].room_flags, DEATH))
      strcat(buf, " (DEATH)");
    if (IS_SET(world[EXIT(ch, door)->to_room_r].room_flags, TRAP))
      strcat(buf," (TRAP)");
    if (IS_SET(world[EXIT(ch, door)->to_room_r].room_flags, NO_MOB))
      strcat(buf," (NOMOB)");
    if (IS_SET(world[EXIT(ch, door)->to_room_r].room_flags, NO_MAGIC))
      strcat(buf, " (NOMAGIC)");
    if (IS_SET(world[EXIT(ch, door)->to_room_r].room_flags, SAFE))
      strcat(buf, " (SAFE)");
    if (IS_SET(world[EXIT(ch, door)->to_room_r].room_flags, ARENA))
      strcat(buf, " (ARENA)");
    if (IS_SET(world[EXIT(ch, door)->to_room_r].room_flags, CLUB))
      strcat(buf, " (CLUB)");
    strcat(buf, "\n\r");
  }

    send_to_char("Obvious exits:\n\r", ch);
  }

  if (*buf)
    send_to_char(buf, ch);
  else
    send_to_char("None.\n\r", ch);
}

/* Remove the Immortalis' Grace enchant. */
void imm_grace_remove_enchant(CHAR *ch) {
  for (ENCH *ench = ch->enchantments; ench; ench = ench->next) {
    if (ench->type == ENCHANT_IMM_GRACE) {
      enchantment_remove(ch, ench, FALSE);

      break;
    }
  }
}

/* Add the Immortali's Grace enchant. */
void imm_grace_add_enchant(CHAR *ch) {
  ENCH ench = { 0 };

  ench.name = strdup("Immortalis' Grace");

  enchantment_to_char(ch, &ench, TRUE);

  free(ench.name);
}

/* Maniplulate a player's death experience. Don't use this directly unless you know what you're doing. */
int adjust_death_exp(CHAR *ch, int exp) {
  if (!ch || (exp == 0) || (GET_DEATH_EXP(ch) == 0)) return 0;

  int64_t tmp_exp = GET_DEATH_EXP(ch) + exp;

  if (tmp_exp > INT_MAX) {
    exp = INT_MAX - GET_DEATH_EXP(ch);
  }
  else if (tmp_exp < 0) {
    exp = -GET_DEATH_EXP(ch);
  }

  GET_DEATH_EXP(ch) += exp;

  /* Did the player exhaust their death xp pool? */
  if (GET_DEATH_EXP(ch) == 0) {
    send_to_char("You are no longer affected by Immortalis' Grace.\n\r", ch);

    imm_grace_remove_enchant(ch);
  }

  return exp;
}

/* Calculate a player's death experience multiplier. */
int calc_death_exp_mult(CHAR *ch) {
  if (!ch) return 0;

  const int DEATH_EXP_MULT = 2;

  // Prestige Perk 6
  if (GET_PRESTIGE_PERK(ch) >= 6) {
    return DEATH_EXP_MULT + 1;
  }

  return DEATH_EXP_MULT;
}

/* Give a player death experience and return how much experience was given. */
int gain_death_exp(CHAR *ch, int exp) {
  if (!ch || (exp <= 0) || (GET_DEATH_EXP(ch) == 0)) return 0;

  int mult = calc_death_exp_mult(ch);

  int64_t tmp_exp = exp * mult;

  if (tmp_exp > INT_MAX) {
    exp = INT_MAX;
  }
  else {
    exp *= mult;
  }

  exp = abs(adjust_death_exp(ch, -exp)); /* Deduct the experience from the player's death experience pool. */
  gain_exp(ch, exp);                     /* Give the (adjusted) death experience to the player. */

  return exp;
}

#define DEATH_EXP_PERCENT 0.90
void die(CHAR *ch)
{
                                /* mu   cl   th   wa   ni   no   pa   ap   av   ba   co */
  const int hit_loss_limit[11]  = {250, 250, 450, 500, 350, 600, 350, 350,   0, 300, 400};
  const int mana_loss_limit[11] = {500, 500, 100, 100, 350, 100, 350, 350, 100, 400, 350};

  char buf[MAX_INPUT_LENGTH];
  int i = 0, factor = 1, percent = 0, mana_chk = 0, hit_chk = 0, death_chk = 0, mana_diff = 0, hit_diff = 0;
  unsigned long death_exp = 0;

  if (signal_room(CHAR_REAL_ROOM(ch), ch, MSG_CORPSE, "")) return;

  /* Characters affected by Animate, Charm and Subdue do not leave blood, to avoid gaming Bathed in Blood bonuses. */
  if (!IS_AFFECTED(ch, AFF_ANIMATE) && !IS_AFFECTED(ch, AFF_CHARM) && !IS_AFFECTED(ch, AFF_SUBDUE)) {
    ROOM_BLOOD(CHAR_REAL_ROOM(ch)) = MIN(ROOM_BLOOD(CHAR_REAL_ROOM(ch)) + 1, 10);
  }

  if (!IS_NPC(ch) && ch->specials.death_timer == 1 && GET_HIT(ch) >= 0)
  {
    sprintf(buf, "%s killed by Imminent Death.", GET_NAME(ch));
    wizlog(buf, LEVEL_IMM, 3);
    log_s(buf);
    deathlog(buf);

    death_chk = 1;
    factor = 2;
  }

  if (!IS_NPC(ch) && IS_SET(ch->specials.affected_by2, AFF2_SEVERED) && GET_HIT(ch) >= 0)
  {
    sprintf(buf, "%s killed by massive body trama.", GET_NAME(ch));
    log_s(buf);
    deathlog(buf);
    wizlog(buf, LEVEL_IMM, 3);
  }

  if (!IS_NPC(ch) && IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) && !ch->specials.death_timer && !IS_SET(ch->specials.affected_by2, AFF2_SEVERED))
  {
    send_to_char("The forces of Chaos send you spinning through reality!", ch);

    death_cry(ch);
    make_chaos_corpse(ch);
    char_from_room(ch);
    char_to_room(ch, real_room(3088));

    act("$n suddenly appears before you, covered in blood!", 0, ch, 0, 0, TO_ROOM);

    if (affected_by_spell(ch, SPELL_POISON))
    {
      affect_from_char(ch, SPELL_POISON);
    }

    GET_HIT(ch) = 1;
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    GET_POS(ch) = POSITION_STUNNED;
    save_char(ch, NOWHERE);

    return;
  }

  for (i = 0; i < MAX_WEAR; i++)
  {
    if (ch->equipment[i])
    {
      obj_to_char(unequip_char(ch, i), ch);
    }
  }

  act("The Reaper appears and escorts $n to the afterlife!", FALSE, ch, 0, 0, TO_ROOM);

  int prestige_mana = 0;

  if (!((GET_CLASS(ch) == CLASS_THIEF) || (GET_CLASS(ch) == CLASS_WARRIOR) || (GET_CLASS(ch) == CLASS_NOMAD))) {
    prestige_mana = GET_PRESTIGE(ch) * PRESTIGE_MANA_GAIN;
  }

  if (!IS_NPC(ch) && ((GET_NAT_MANA(ch) - prestige_mana) > mana_loss_limit[GET_CLASS(ch) - 1]))
  {
    mana_chk = TRUE;
  }

  int prestige_hit = 0;

  if ((GET_CLASS(ch) == CLASS_THIEF) || (GET_CLASS(ch) == CLASS_WARRIOR) || (GET_CLASS(ch) == CLASS_NOMAD)) {
    prestige_hit = GET_PRESTIGE(ch) * (PRESTIGE_HIT_GAIN + PRESTIGE_MANA_GAIN);
  }
  else {
    prestige_hit = GET_PRESTIGE(ch) * PRESTIGE_HIT_GAIN;
  }

  if (!IS_NPC(ch) && ((GET_NAT_HIT(ch) - prestige_hit) > hit_loss_limit[GET_CLASS(ch) - 1]))
  {
    hit_chk = TRUE;
  }

  if (!IS_NPC(ch) && GET_DEATH_LIMIT(ch) && GET_BEEN_KILLED(ch) > GET_DEATH_LIMIT(ch))
  {
    death_chk = TRUE;
    factor = number(2, 5);
  }

  if (mana_chk || hit_chk  ||
      (death_chk && ((GET_NAT_MANA(ch) - prestige_mana) > 0)) ||
      (death_chk && ((GET_NAT_HIT(ch) - prestige_hit) > 0)))
  {
    send_to_char("\n\r\n\rThe Reaper demands his toll for your passage through the underworld.\n\r", ch);
    send_to_char("Your soul burns as he tears some lifeforce from you.\n\r", ch);

    mana_diff = GET_NAT_MANA(ch) - mana_loss_limit[GET_CLASS(ch) - 1] - prestige_mana;
    hit_diff = GET_NAT_HIT(ch) - hit_loss_limit[GET_CLASS(ch) - 1] - prestige_hit;
    mana_diff = MAX(mana_diff, 0);
    hit_diff = MAX(hit_diff, 0);

    percent = number(1, 4); /* was 2, 7 - Ranger Dec 2000 */
    mana_diff = (int)(sqrt(mana_diff) / 10 * percent);
    percent = number(1, 4); /* was 2, 7 - Ranger Dec 2000 */
    hit_diff = (int)(sqrt(hit_diff) / 10 * percent);

    if (mana_chk || death_chk)
    {
      if (death_chk)
      {
        mana_diff = MAX(3, mana_diff);
      }
      else
      {
        mana_diff = MAX(1, mana_diff);
      }
    }
    else
    {
      mana_diff = 0;
    }

    if (hit_chk || death_chk)
    {
      if (death_chk)
      {
        hit_diff = MAX(3, hit_diff);
      }
      else
      {
        hit_diff = MAX(1, hit_diff);
      }
    }
    else
    {
      hit_diff = 0;
    }

    /* Must lose stats on death_chk. */
    if (death_chk && !hit_diff)
    {
      hit_diff = 2;
    }

    if (death_chk && !mana_diff)
    {
      mana_diff = 2;
    }

    hit_diff *= factor;
    mana_diff *= factor;

    while (mana_diff && (ch->points.max_mana - mana_diff) < 0)
    {
      mana_diff--;
    }

    while (hit_diff && (ch->points.max_hit - hit_diff) < 1)
    {
      hit_diff--;
    }

    sprintf(buf,"WIZINFO: %s lost %d/%d hps and %d/%d mana.", GET_NAME(ch),
            hit_diff, GET_MAX_HIT(ch), mana_diff, GET_MAX_MANA(ch));
    wizlog(buf, LEVEL_SUP, 3);
    log_s(buf);

    ch->points.max_mana -= mana_diff;
    ch->points.max_mana = MAX(ch->points.max_mana, 0);
    ch->points.max_hit -= hit_diff;
    ch->points.max_hit = MAX(ch->points.max_hit, 1);

    affect_total(ch);

    death_exp += lround(((double)mana_diff / 4.5) * hit_mana_cost(ch, META_MANA));
    death_exp += lround(((double)hit_diff / 4.5) * hit_mana_cost(ch, META_HIT));
  }
  else
  {
    send_to_char("\n\r\n\rThe Reaper considers your soul and intones 'You may pass for free--for now.'\n\r", ch);
  }

  if (IS_SET(ch->specials.affected_by2, AFF2_SEVERED))
  {
    REMOVE_BIT(ch->specials.affected_by2, AFF2_SEVERED);
  }

  if (IS_SET(ch->specials.affected_by2, AFF2_IMMINENT_DEATH))
  {
    REMOVE_BIT(ch->specials.affected_by2, AFF2_IMMINENT_DEATH);
  }

  ch->specials.death_timer = 0;

  if (!IS_NPC(ch))
  {
    if (GET_LEVEL(ch) > 6)
    {
      death_exp += GET_EXP(ch) / 2;

      GET_EXP(ch) = MAX(GET_EXP(ch) - (GET_EXP(ch) / 2), 0);
    }
    else
    {
      send_to_char("\n\rThe Reaper intones 'You may keep your current experience, but\n\r\after level six you will begin to lose some of it at death.'\n\r", ch);
    }

    if (death_exp)
    {
      double mult = DEATH_EXP_PERCENT;

      // Prestige Perk 11
      if (GET_PRESTIGE_PERK(ch) >= 11) {
        mult += 0.01;
      }

      death_exp = lround((double)death_exp * mult);

      GET_DEATH_EXP(ch) += death_exp;
      imm_grace_add_enchant(ch);

      sprintf(buf, "WIZINFO: Adding %lu Death Experience to %s's pool.", death_exp, GET_NAME(ch));
      wizlog(buf, LEVEL_SUP, 3);
      log_s(buf);

      send_to_char("\n\r\
Suddenly, Immortalis' spirit intervenes and interrupts the Reaper's harvest!\n\r\
You feel the ethereal tendrils of your soul still clinging to your existence...\n\r\
Immortalis' spirit whispers to you, 'You may re-claim in life much of what you\n\r\
have lost in death. Redeem yourself and you shall return to your former glory!'\n\r", ch);
    }
  }

  if (ch->desc)
  {
    give_prompt(ch->desc);
  }

  raw_kill(ch);
}

char *get_club_name(CHAR *ch)
{
  if (IS_SET(ch->specials.pflag, PLR_SANES_VOCAL_CLUB))
    return "Sane's Vocal Club";
  else if (IS_SET(ch->specials.pflag, PLR_LINERS_LOUNGE))
    return "Liner's Lounge";
  else if (IS_SET(ch->specials.pflag, PLR_LEMS_LIQOUR_ROOM))
    return "Lem's Liqour Room";
  else if (IS_SET(ch->specials.pflag, PLR_RANGERS_RELIQUARY))
    return "Ranger's Reliquary";
  else
    return "None";
}


#define AFF_MODE_N 0
#define AFF_MODE_O 1
#define AFF_MODE_B 2

#define AFF_SRC_AF 1
#define AFF_SRC_EN 2
#define AFF_SRC_EQ 3

struct affect {
  char name[MIL];
  int duration;
  int interval;
  int type;
  int source;
};

int compare_affects(const void *affect1, const void *affect2) {
  struct affect *af1 = (struct affect *)affect1;
  struct affect *af2 = (struct affect *)affect2;

  return strcmp(af1->name, af2->name);
}

/* Prints skill/spell affects from worn equipment, applied skills and
spells, and enchantments. */
void do_affect(CHAR *ch, char *arg, int cmd) {
  char buf[MIL];
  char buf2[MIL];
  int mode = 0;
  int i = 0;
  int count = 0;
  int longest_str = 0;
  int longest_dur = 0;
  OBJ *obj = NULL;
  AFF *tmp_af = NULL;
  ENCH *tmp_ench = NULL;
  bool eq_af = FALSE;
  bool equipment[MAX_SPL_LIST] = { FALSE };
  bool affects[MAX_SPL_LIST] = { FALSE };
  struct affect af_list[MAX_SPL_LIST];
  struct affect af_new;

  one_argument(arg, buf);

  if (*buf && is_abbrev(buf, "old")) {
    mode = AFF_MODE_O;
  }
  else if (*buf && is_abbrev(buf, "brief")) {
    mode = AFF_MODE_B;
  }
  else {
    mode = AFF_MODE_N;
  }

  /* Old do_affect for those that still want it. */
  if (mode == AFF_MODE_O) {
    if (ch->affected || ch->enchantments) {
      if (ch->affected) {
        send_to_char("\n\rAffecting Spells/Skills:\n\r-----------------------\n\r", ch);

        for (tmp_af = ch->affected; tmp_af; tmp_af = tmp_af->next) {
          sprintf(buf, "      Spell/Skill : '%s'\n\r", spells[tmp_af->type - 1]);
          send_to_char(buf, ch);

          if (tmp_af->type == SKILL_MANTRA || tmp_af->type == SPELL_WITHER || tmp_af->type == SPELL_INCENDIARY_CLOUD_NEW) {
            sprintf(buf, "            Expires in %3d seconds (approx.)\n\r", tmp_af->duration * 10);
          }
          else {
            sprintf(buf, "            Expires in %3d ticks\n\r", tmp_af->duration);
          }

          send_to_char(buf, ch);
        }
      }

      if (ch->enchantments) {
        send_to_char("\n\rEnchantments:\n\r------------\n\r", ch);

        for (tmp_ench = ch->enchantments; tmp_ench; tmp_ench = tmp_ench->next) {
          sprintf(buf, "     '%s'\n\r", tmp_ench->name);
          send_to_char(buf, ch);
        }
      }
    }
    else {
      send_to_char("You are not affected by any spell, skill or enchantment.\n\r", ch);
    }

    return;
  }

  /* Get affects applied by worn equipment. This is a bit messy
  because these are bits set in a bitvector and don't directly
  map to anything in spells[]. */
  for (i = 0; i < MAX_WEAR; i++) {
    if (!(obj = EQ(ch, i))) continue;

    /* Set a simple flag for use later that shows that there was
    some equpment worn that applied an affect. */
    if (!eq_af &&
        (OBJ_BITS(obj) || OBJ_BITS2(obj))) {
      eq_af = TRUE;
    }

    /* affected_by */
    if (IS_SET(OBJ_BITS(obj), AFF_BLIND) &&
        IS_SET(GET_AFF(ch), AFF_BLIND)) {
      equipment[SPELL_BLINDNESS] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_INVISIBLE) &&
        IS_SET(GET_AFF(ch), AFF_INVISIBLE)) {
      equipment[SPELL_INVISIBLE] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_DETECT_ALIGNMENT) &&
        IS_SET(GET_AFF(ch), AFF_DETECT_ALIGNMENT)) {
      equipment[SPELL_DETECT_ALIGNMENT] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_DETECT_INVISIBLE) &&
        IS_SET(GET_AFF(ch), AFF_DETECT_INVISIBLE)) {
      equipment[SPELL_DETECT_INVISIBLE] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_DETECT_MAGIC) &&
        IS_SET(GET_AFF(ch), AFF_DETECT_MAGIC)) {
      equipment[SPELL_DETECT_MAGIC] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_SENSE_LIFE) &&
        IS_SET(GET_AFF(ch), AFF_SENSE_LIFE)) {
      equipment[SPELL_SENSE_LIFE] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_HOLD) &&
        IS_SET(GET_AFF(ch), AFF_HOLD)) {
      equipment[SPELL_HOLD] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_SANCTUARY) &&
        IS_SET(GET_AFF(ch), AFF_SANCTUARY)) {
      equipment[SPELL_SANCTUARY] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_CURSE) &&
        IS_SET(GET_AFF(ch), AFF_CURSE)) {
      equipment[SPELL_CURSE] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_SPHERE) &&
        IS_SET(GET_AFF(ch), AFF_SPHERE)) {
      equipment[SPELL_SPHERE] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_POISON) &&
        IS_SET(GET_AFF(ch), AFF_POISON)) {
      equipment[SPELL_POISON] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_PROTECT_EVIL) &&
        IS_SET(GET_AFF(ch), AFF_PROTECT_EVIL)) {
      equipment[SPELL_PROTECT_FROM_EVIL] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_PARALYSIS) &&
        IS_SET(GET_AFF(ch), AFF_PARALYSIS)) {
      equipment[SPELL_PARALYSIS] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_INFRAVISION) &&
        IS_SET(GET_AFF(ch), AFF_INFRAVISION)) {
      equipment[SPELL_INFRAVISION] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_SLEEP) &&
        IS_SET(GET_AFF(ch), AFF_SLEEP)) {
      equipment[SPELL_SLEEP] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_DODGE) &&
        IS_SET(GET_AFF(ch), AFF_DODGE)) {
      equipment[SKILL_DODGE] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_SNEAK) &&
        IS_SET(GET_AFF(ch), AFF_SNEAK)) {
      equipment[SKILL_SNEAK] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_HIDE) &&
        IS_SET(GET_AFF(ch), AFF_HIDE)) {
      equipment[SKILL_HIDE] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_ANIMATE) &&
        IS_SET(GET_AFF(ch), AFF_ANIMATE)) {
      equipment[SPELL_ANIMATE_DEAD] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_CHARM) &&
        IS_SET(GET_AFF(ch), AFF_CHARM)) {
      equipment[SPELL_CHARM_PERSON] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_PROTECT_GOOD) &&
        IS_SET(GET_AFF(ch), AFF_PROTECT_GOOD)) {
      equipment[SPELL_PROTECT_FROM_GOOD] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_FLY) &&
        IS_SET(GET_AFF(ch), AFF_FLY)) {
      equipment[SPELL_FLY] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_IMINV) &&
        IS_SET(GET_AFF(ch), AFF_IMINV)) {
      equipment[SPELL_IMP_INVISIBLE] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_INVUL) &&
        IS_SET(GET_AFF(ch), AFF_INVUL)) {
      equipment[SPELL_INVUL] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_DUAL) &&
        IS_SET(GET_AFF(ch), AFF_DUAL)) {
      equipment[SKILL_DUAL] = TRUE;
    }

    if (IS_SET(OBJ_BITS(obj), AFF_FURY) &&
        IS_SET(GET_AFF(ch), AFF_FURY)) {
      equipment[SPELL_FURY] = TRUE;
    }

    /* affected_by2 */
    if (IS_SET(OBJ_BITS2(obj), AFF2_TRIPLE) &&
        IS_SET(GET_AFF2(ch), AFF2_TRIPLE)) {
      equipment[SKILL_TRIPLE] = TRUE;
    }

    if (IS_SET(OBJ_BITS2(obj), AFF2_QUAD) &&
        IS_SET(GET_AFF2(ch), AFF2_QUAD)) {
      equipment[SKILL_QUAD] = TRUE;
    }

    if (IS_SET(OBJ_BITS2(obj), AFF2_FORTIFICATION) &&
        IS_SET(GET_AFF2(ch), AFF2_FORTIFICATION)) {
      equipment[SPELL_FORTIFICATION] = TRUE;
    }

    if (IS_SET(OBJ_BITS2(obj), AFF2_PERCEIVE) &&
        IS_SET(GET_AFF2(ch), AFF2_PERCEIVE)) {
      equipment[SPELL_PERCEIVE] = TRUE;
    }

    if (IS_SET(OBJ_BITS2(obj), AFF2_RAGE) &&
        IS_SET(GET_AFF2(ch), AFF2_RAGE)) {
      equipment[SPELL_RAGE] = TRUE;
    }
  }

  if (!eq_af &&
      !ch->affected &&
      !ch->enchantments) {
    send_to_char("You are not affected by any spell, skill or enchantment.\n\r", ch);

    return;
  }

  /* Check if we actually need to print anything. */
  if (eq_af ||
      ch->affected ||
      ch->enchantments) {
    /* This variable keeps track of where we are in the list of affects. */
    count = 0;
    /* Clear the list just to be safe. */
    memset(af_list, 0, sizeof(struct affect) * MAX_SPL_LIST);

    /* Process skill/spell affects. */
    if (ch->affected) {
      for (tmp_af = ch->affected; tmp_af && (count < MAX_SPL_LIST); tmp_af = tmp_af->next, count++) {
        /* Only count the affect once, from either equipment, or applied skill/spell affects. */
        if (!equipment[tmp_af->type] && !affects[tmp_af->type]) {
          affects[tmp_af->type] = TRUE;

          memset(&af_new, 0, sizeof(struct affect));

          strcpy(af_new.name, spells[tmp_af->type - 1]);
          af_new.type = tmp_af->type;
          af_new.duration = tmp_af->duration;
          af_new.source = AFF_SRC_AF;

          memcpy(&af_list[count], &af_new, sizeof(struct affect));

          /* Record the longest string for use in printing. */
          if (strlen(af_list[count].name) > longest_str) {
            longest_str = strlen(af_list[count].name);
          }

          /* Record the longest duration for use in printing. */
          if (af_list[count].duration > longest_dur) {
            longest_dur = (((af_list[count].type == SKILL_MANTRA) ||
                            (af_list[count].type == SPELL_WITHER)) ?
                           (af_list[count].duration * 100) : af_list[count].duration);
          }
        }
      }
    }

    /* Process enchantments. */
    if (ch->enchantments) {
      for (tmp_ench = ch->enchantments; tmp_ench && (count < MAX_SPL_LIST); tmp_ench = tmp_ench->next, count++) {
        memset(&af_new, 0, sizeof(struct affect));

        if (tmp_ench->name) {
          strncpy(af_new.name, tmp_ench->name, FIELD_SIZE(affect, name) - 1);
        }

        af_new.type = AFF_NONE;
        af_new.duration = tmp_ench->duration;
        af_new.interval = tmp_ench->interval;
        af_new.source = AFF_SRC_EN;

        memcpy(&af_list[count], &af_new, sizeof(struct affect));

        /* Record the longest string for use in printing. */
        if (strlen(af_list[count].name) > longest_str) {
          longest_str = strlen(af_list[count].name);
        }

        /* Record the longest duration for use in printing. */
        if (af_list[count].duration > longest_dur) {
          longest_dur = af_list[count].duration;
        }
      }
    }

    /* Process affects applied by equipment. */
    if (eq_af) {
      for (i = 0; (i < MAX_SPL_LIST) && (count < MAX_SPL_LIST); i++, count++) {
        if (equipment[i]) {
          memset(&af_new, 0, sizeof(struct affect));

          strcpy(af_new.name, spells[i - 1]);
          af_new.type = i;
          af_new.duration = -1;
          af_new.source = AFF_SRC_EQ;

          memcpy(&af_list[count], &af_new, sizeof(struct affect));

          /* Record the longest string for use in printing. */
          if (strlen(af_list[count].name) > longest_str) {
            longest_str = strlen(af_list[count].name);
          }

          /* Record the longest duration for use in printing. */
          if (af_list[count].duration > longest_dur) {
            longest_dur = af_list[count].duration;
          }
        }
      }
    }

    /* Calculate the length of the longest duration for use in printing. */
    longest_dur = (longest_dur == 0) ? 1 : (floor(log10(abs(longest_dur))) + 1);

    /* Sort the list of applied affects alphabetically. */
    qsort((void *)&af_list, MAX_SPL_LIST, sizeof(struct affect), compare_affects);

    send_to_char("\n\rAffected by:\n\r-----------\n\r", ch);

    /* Print skill/spell affects first. */
    if (ch->affected) {
      for (i = 0; i < MAX_SPL_LIST; i++) {
        if ((af_list[i].source != AFF_SRC_AF) || ((af_list[i].duration == -1) && (mode == AFF_MODE_B))) {
          continue;
        }

        snprintf(buf, sizeof(buf), "'%.252s'", af_list[i].name);

        if (af_list[i].duration < 0) {
          snprintf(buf2, sizeof(buf2), "Never Expires");
        }
        else {
          if ((af_list[i].type == SKILL_MANTRA) ||
              (af_list[i].type == SPELL_WITHER)) {
            snprintf(buf2, sizeof(buf2), "Expires in: ~%*d Second%s",
              ((longest_dur < 2) ? longest_dur : (longest_dur - 1)),
              (af_list[i].duration * 10),
              ((af_list[i].duration != 1) ? "s" : ""));
          }
          else {
            snprintf(buf2, sizeof(buf2), "Expires in: %*d Tick%s",
              longest_dur, af_list[i].duration, ((af_list[i].duration != 1) ? "s" : ""));
          }
        }

        printf_to_char(ch, "Skill/Spell: %-*s %s\n\r", longest_str + 2, buf, buf2);
      }
    }

    /* Next, print enchantments. They are unique by name, so this is a bit more simple than skill/spell affects. */
    if (ch->enchantments) {
      if (ch->affected && (mode != AFF_MODE_B)) {
        send_to_char("\n\r", ch);
      }

      for (i = 0; i < MAX_SPL_LIST; i++) {
        if ((af_list[i].source != AFF_SRC_EN) || ((af_list[i].duration == -1) && (mode == AFF_MODE_B))) {
          continue;
        }

        snprintf(buf, sizeof(buf), "'%.252s'", af_list[i].name);

        if (af_list[i].duration < 0) {
          snprintf(buf2, sizeof(buf2), "Never Expires");
        }
        else if (af_list[i].interval == ENCH_INTERVAL_TICK) {
          snprintf(buf2, sizeof(buf2), "Expires in: %*d Tick%s",
            longest_dur, af_list[i].duration, ((af_list[i].duration != 1) ? "s" : ""));
        }
        else if (af_list[i].interval == ENCH_INTERVAL_MOBACT) {
          snprintf(buf2, sizeof(buf2), "Expires in: ~%*d Second%s",
            ((longest_dur < 2) ? longest_dur : (longest_dur - 1)),
            (af_list[i].duration * 10),
            ((af_list[i].duration != 1) ? "s" : ""));
        }
        else if (af_list[i].interval == ENCH_INTERVAL_ROUND) {
          snprintf(buf2, sizeof(buf2), "Expires in: %*d Round%s",
            longest_dur, af_list[i].duration, ((af_list[i].duration != 1) ? "s" : ""));
        }
        else {
          buf2[0] = '\0';
        }

        printf_to_char(ch, "Enchantment: %-*s %s\n\r", longest_str + 2, buf, buf2);
      }
    }

    /* Finally, print any affects from worn equipment. */
    if (eq_af && (mode != AFF_MODE_B)) {
      if ((ch->affected || ch->enchantments) && (mode != AFF_MODE_B)) {
        send_to_char("\n\r", ch);
      }

      for (i = 0; i < MAX_SPL_LIST; i++) {
        if (af_list[i].source != AFF_SRC_EQ) continue;

        snprintf(buf, sizeof(buf), "'%.252s'", af_list[i].name);

        printf_to_char(ch, "Equipment%s: %-*s Never Expires\n\r",
          (ch->affected || ch->enchantments) ? "  " : "", longest_str + 2, buf);
      }
    }
  }
}


void do_time(struct char_data *ch, char *argument, int cmd) {
  long ct;
  char buf[100], *suf, *tmstr;
  int weekday, day;

  sprintf(buf, "It is %d o'clock %s, on ",
    ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
    ((time_info.hours >= 12) ? "pm" : "am") );

  weekday = ((28*time_info.month)+time_info.day+1) % 7;/* 28 days in a month */

  strcat(buf,weekdays[weekday]);
  strcat(buf,"\n\r");
  send_to_char(buf,ch);

  day = time_info.day + 1;   /* day in [1..28] */

  if (day == 1)
    suf = "st";
  else if (day == 2)
    suf = "nd";
  else if (day == 3)
    suf = "rd";
  else if (day < 20)
    suf = "th";
  else if ((day % 10) == 1)
    suf = "st";
  else if ((day % 10) == 2)
    suf = "nd";
  else if ((day % 10) == 3)
    suf = "rd";
  else
    suf = "th";

  sprintf(buf, "The %d%s Day of the %s, Year %d.\n\r",
    day,
    suf,
    month_name[time_info.month],
    time_info.year);

  send_to_char(buf,ch);

  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr+strlen(tmstr)-1)='\0';
#ifdef BCHS
  sprintf(buf,"The real local time is: %s CST\n\r",tmstr);
#else
  sprintf(buf,"The real local time is: %s PST with the reboot due %d:00.\n\r",tmstr,REBOOT_AT);
#endif
  send_to_char(buf,ch);

}


void do_weather(struct char_data *ch, char *argument, int cmd)
{
  char buf[100];
  char *sky_look[4]= {
    "cloudless",
    "cloudy",
    "rainy",
    "lit by flashes of lightning"};

  if (IS_OUTSIDE(ch)) {
    sprintf(buf,
      "The sky is %s and %s.\n\r",
      sky_look[weather_info.sky],
      (weather_info.change >=0 ? "you feel a warm wind from south" :
       "your foot tells you bad weather is due"));
    send_to_char(buf,ch);
  } else
    send_to_char("You have no feeling about the weather at all.\n\r", ch);
}

void list_socials(struct char_data *ch);

void do_help(struct char_data *ch, char *argument, int cmd) {
  FILE *fl;
  struct string_block sb;
  int chk, bot, top, mid, minlen,i;
  char buf[MSL];

  if (!ch->desc) return;
  for(;isspace(*argument); argument++)  ;

  if (!*argument) {
    send_to_char(help, ch);
    return;
  }

  if (!help_index) {
    send_to_char("No help available.\n\r", ch);
    return;
  }

  if(!strcmp(argument,"social list")) {
    list_socials(ch);
    return;
  }

  if(is_abbrev(argument,"index")) {
    page_string(ch->desc, helpcontents, 0);
    return;
  }

  init_string_block(&sb);
  bot = 0;
  top = top_of_helpt;
  for (;;) {
    mid = (bot + top) / 2;
    minlen = strlen(argument);
    if (!(chk = strn_cmp(argument, help_index[mid].keyword, minlen))) {
      /* Go back 5 keywords and check them out */
      for(i=MAX(mid-5,0);i<=mid;i++) {
        if (!(chk = strn_cmp(argument, help_index[i].keyword, minlen))) {
           mid=i;
           break;
        }
      }

      fseek(help_fl, help_index[mid].pos, 0);
      for (;;) {
        if (!fgets(buf, 80, help_fl)) break;
        if (*buf == '#') break;
        append_to_string_block(&sb, buf);
        if (buf[MAX(0, strnlen(buf, 80) - 1)] == '\n') {
          append_to_string_block(&sb, "\r");
        }
      }
      page_string_block(&sb,ch);
      destroy_string_block(&sb);
      return;
    }
    else if (bot >= top) {
      send_to_char("There is no help on that word.\n\r", ch);
      send_to_char("Here is the help entry closest to it alphabetically.\n\r",ch);
      fseek(help_fl, help_index[mid].pos, 0);
      for (;;) {
        if (!fgets(buf, 80, help_fl)) break;
        if (*buf == '#') break;
        append_to_string_block(&sb, buf);
        if (buf[MAX(0, strnlen(buf, 80) - 1)] == '\n') {
          append_to_string_block(&sb, "\r");
        }
      }
      page_string_block(&sb,ch);
      destroy_string_block(&sb);

      if(!(fl=fopen("help_log","a"))) {
        log_s("Error opening help_log file");
        return;
      }
      fprintf(fl,"%s\n",argument);
      fclose(fl);
      return;
    }
    else if (chk > 0) bot = ++mid;
    else top = --mid;
  }
  destroy_string_block(&sb);
  return;
}

void do_wizcmd(struct char_data *ch, char *argument)
{
  int chk, bot, top, mid, minlen;
  char buf[MSL], buffer[2*MSL];

  if (!ch->desc) return;

  for(;isspace(*argument); argument++)  ;
  if (*argument) {
    if (!wizhelp_index) {
      send_to_char("No help available.\n\r", ch);
      return;
    }

    if(is_abbrev(argument,"index")) {
      page_string(ch->desc, wizhelpcontents, 0);
      return;
    }

    bot = 0;
    top = top_of_wizhelpt;
    minlen = strlen(argument);

    for (;;) {
      mid = (bot + top) / 2;
      if (!(chk = strn_cmp(argument, wizhelp_index[mid].keyword, minlen))) {
        rewind(wizhelp_fl);
        if (fseek(wizhelp_fl, wizhelp_index[mid].pos, SEEK_SET) < 0) {
          log_f("Could not read wizhelp file, errno=%d.\n\r", errno);
          send_to_char("The help file appears to be corrupted. Please report this to an admin.\n\r", ch);
        }

        *buffer = '\0';

        for (;;) {
          if (!fgets(buf, 80, wizhelp_fl)) break;
          if (*buf == '#') break;
          strcat(buffer, buf);
          strcat(buffer, "\r");
        }

        page_string(ch->desc, buffer, 1);
        return;
      }
      else if (bot >= top) {
        send_to_char("There is no help on that word.\n\r", ch);
        return;
      }
      else if (chk > 0) {
        bot = ++mid;
      }
      else
      {
        top = --mid;
      }
    }
    return;
  }

  send_to_char("No help on that command!\n\r", ch);

}

void do_wizhelp(struct char_data *ch, char *argument, int cmd) {
  char buf[MSL];
  int no, i,level,to_level,breakdown=0;

  if (IS_NPC(ch)) return;

  level=GET_LEVEL(ch);
  one_argument(argument, buf);
  if (*buf)
    {
    if (isdigit(*buf))
      {
      level = atoi(buf);
      }
    else if(is_abbrev(buf,"level"))
      {
      breakdown=1;
      }
    else
      {
      do_wizcmd(ch, argument);
      return;
      }
    }

  to_level=MAX(level,LEVEL_IMM);
  to_level=MIN(level,GET_LEVEL(ch));

  printf_to_char(ch,"\
Use 'wizhelp index' for a complete list of wizhelp entries,\n\r\
    'wizhelp level' for a list broken down by level,\n\r\
    'wizhelp <#>'   for a list of cmds at that level.\n\r\
The following privileged commands are available up to level %d:",to_level);

  *buf = '\0';

  if(!breakdown) strcat(buf,"\n\r\n\r");
  no=1;
  for(level=LEVEL_IMM;level<=to_level;level++)
    {
    if(breakdown)
      {
      no=1;
      sprintf(buf + strlen(buf),"\n\r\n\r*** Level %d ***\n\r",level);
      }
    for(i = 1;cmd_info[i].cmd_text; i++)
      {
      if(cmd_info[i].minimum_level<LEVEL_IMM) continue;
      if(cmd_info[i].minimum_level==level)
        {
        if(cmd_info[i].num==CMD_NEWOLC) continue;
        if(cmd_info[i].num==CMD_NEWOLCR) continue;
        if(cmd_info[i].num==CMD_NEWOLCZ) continue;
        if(cmd_info[i].num==CMD_NEWOLCO) continue;
        if(cmd_info[i].num==CMD_NEWOLCM) continue;
        sprintf(buf + strlen(buf), "%-10s", cmd_info[i].cmd_text);
        if (!(no % 7)) strcat(buf, "\n\r");
        no++;
        }
      }
    }
  strcat(buf, "\n\r");
  send_to_char(buf,ch);
}

void do_olchelp(struct char_data *ch, char *argument, int cmd)
{
  struct string_block sb;
  int chk, bot, top, mid, minlen,no,i;
  char buf[MSL];
  bool found=FALSE;

  if (!ch->desc) return;

  for(;isspace(*argument); argument++)  ;
  if (*argument) {
    if (!olchelp_index) {
      send_to_char("No help available.\n\r", ch);
      return;
    }

    if(is_abbrev(argument,"index")) {
      page_string(ch->desc, olchelpcontents, 0);
      return;
    }

    init_string_block(&sb);
    bot = 0;
    top = top_of_olchelpt;
    for (;;) {
      mid = (bot + top) / 2;
      minlen = strlen(argument);
      if (!(chk = strn_cmp(argument, olchelp_index[mid].keyword, minlen))) {
        /* Added a step back to make sure a similar keyword wasn't missed - Ranger July 96 */
        mid=MAX(mid-2,0); /* Step back two keywords */
        if (!(chk = strn_cmp(argument, olchelp_index[mid].keyword, minlen))) found=TRUE;
        else mid=mid+1; /* mid-2 didn't match, so add 1. */
        if (!found && !(chk = strn_cmp(argument, olchelp_index[mid].keyword, minlen))) { }
        else mid=mid+1; /* mid-1 didn't match either, so add 1 */
        /* End of modification */

        fseek(olchelp_fl, olchelp_index[mid].pos, 0);
        for (;;) {
          if (!fgets(buf, 80, olchelp_fl)) break;
          if (*buf == '#')
            break;
          append_to_string_block(&sb,buf);
          append_to_string_block(&sb,"\r");
        }
        page_string_block(&sb,ch);
        destroy_string_block(&sb);
        return;
      } else if (bot >= top) {
        send_to_char("There is no help on that word.\n\r", ch);
        destroy_string_block(&sb);
        return;
      } else if (chk > 0)
          bot = ++mid;
        else
          top = --mid;
    }
    destroy_string_block(&sb);
    return;
  }

  send_to_char("`iUse 'olchelp commands' for an alternate index.`q\n\r",ch);
  send_to_char("`iUse 'olchelp index' for a complete list.`q\n\r\n\r",ch);
  send_to_char("`iThe following commands are available for builders:`q\n\r\n\r",ch);
  *buf = '\0';

  sprintf(buf,"`iGENERAL:`q\n\r");
  for(no = 1, i = 1;cmd_info[i].cmd_text; i++) {
    if ( GET_LEVEL(ch)>=cmd_info[i].minimum_level &&
         cmd_info[i].minimum_level >= LEVEL_IMM &&
         cmd_info[i].num==CMD_NEWOLC &&
         IS_SET(ch->new.imm_flags, WIZ_CREATE)) {
      sprintf(buf + strlen(buf), "%-10s", cmd_info[i].cmd_text);
      if (!(no % 7)) strcat(buf, "\n\r");
      no++;
    }
  }
  strcat(buf, "\n\r\n\r");

  strcat(buf,"`iROOMS:`q (OLCHELP ROOMS)\n\r");
  for(no = 1, i = 1;cmd_info[i].cmd_text; i++) {
    if ( GET_LEVEL(ch)>=cmd_info[i].minimum_level &&
         cmd_info[i].minimum_level >= LEVEL_IMM &&
         cmd_info[i].num==CMD_NEWOLCR &&
         IS_SET(ch->new.imm_flags, WIZ_CREATE)) {
      sprintf(buf + strlen(buf), "%-10s", cmd_info[i].cmd_text);
      if (!(no % 7)) strcat(buf, "\n\r");
      no++;
    }
  }
  strcat(buf, "\n\r\n\r");

  strcat(buf,"`iMOBS:`q (OLCHELP MOBS)\n\r");
  for(no = 1, i = 1;cmd_info[i].cmd_text; i++) {
    if ( GET_LEVEL(ch)>=cmd_info[i].minimum_level &&
         cmd_info[i].minimum_level >= LEVEL_IMM &&
         cmd_info[i].num==CMD_NEWOLCM &&
         IS_SET(ch->new.imm_flags, WIZ_CREATE)) {
      sprintf(buf + strlen(buf), "%-10s", cmd_info[i].cmd_text);
      if (!(no % 7)) strcat(buf, "\n\r");
      no++;
    }
  }
  strcat(buf, "\n\r\n\r");

  strcat(buf, "`iOBJS:`q (OLCHELP OBJECTS)\n\r");
  for (no = 0, i = 1; cmd_info[i].cmd_text; i++) {
    if (GET_LEVEL(ch) >= cmd_info[i].minimum_level &&
        cmd_info[i].minimum_level >= LEVEL_IMM &&
        cmd_info[i].num == CMD_NEWOLCO &&
        IS_SET(ch->new.imm_flags, WIZ_CREATE)) {
      if (strlen(cmd_info[i].cmd_text) < 10) {
        sprintf(buf + strlen(buf), "%-10s", cmd_info[i].cmd_text);
        no += 1;
      }
      else {
        sprintf(buf + strlen(buf), "%-20s", cmd_info[i].cmd_text);
        no += 2;
      }
      if (no >= 7) {
        strcat(buf, "\n\r");
        no = 0;
      }
    }
  }
  strcat(buf, "\n\r\n\r");

  strcat(buf,"`iZONES:`q (OLCHELP ZONES)\n\r");
  for(no = 1, i = 1;cmd_info[i].cmd_text; i++) {
    if ( GET_LEVEL(ch)>=cmd_info[i].minimum_level &&
         cmd_info[i].minimum_level >= LEVEL_IMM &&
         cmd_info[i].num==CMD_NEWOLCZ &&
         IS_SET(ch->new.imm_flags, WIZ_CREATE)) {
      sprintf(buf + strlen(buf), "%-10s", cmd_info[i].cmd_text);
      if (!(no % 7)) strcat(buf, "\n\r");
      no++;
    }
  }
  strcat(buf, "\n\r\n\r");

  page_string(ch->desc, buf, 1);
}


const char *immortal_abbrevs[] = {
  "IMM", /* Immortal */
  "DEI", /* Deity */
  "TEM", /* Temporal */
  "WIZ", /* Wizard */
  "ETE", /* Eternal */
  "SUP", /* Supreme */
  "IMP", /* Implementor */
};

const char *class_abbrevs[] = {
  "--",
  "Mu", /* Magic-User */
  "Cl", /* Cleric */
  "Th", /* Thief */
  "Wa", /* Warrior */
  "Ni", /* Ninja */
  "No", /* Nomad */
  "Pa", /* Paladin */
  "Ap", /* Anti-Paladin */
  "Av", /* Avatar */
  "Ba", /* Bard */
  "Co", /* Commando */
};

const char *subclass_abbrevs[] = {
  "--",
  "En", /* Enchanter */
  "Ar", /* Archmage */
  "Dr", /* Druid */
  "Te", /* Templar */
  "Rg", /* Rogue */
  "Bt", /* Bandit */
  "Wl", /* Warlord */
  "Gl", /* Gladiator */
  "Rn", /* Ronin */
  "My", /* Mystic */
  "Ra", /* Ranger */
  "Tr", /* Trapper */
  "Ca", /* Cavalier */
  "Cr", /* Crusader */
  "De", /* Defiler */
  "In", /* Infidel */
  "--", /* Avatar1 */
  "--", /* Avatar2 */
  "Bl", /* Bladesinger */
  "Ch", /* Chanter */
  "Le", /* Legionnaire */
  "Me", /* Mercenary */
};

#define WHO_FLT_CLASS    (1 << 0)
#define WHO_FLT_SUBCLASS (1 << 1)
#define WHO_FLT_RANK     (1 << 2)
#define WHO_FLT_PRESTIGE (1 << 3)
#define WHO_FLT_TITLE    (1 << 4)
#define WHO_FLT_FLAGS    (1 << 5)

void do_who(CHAR *ch, char *arg, int cmd) {
  char buf[MSL], buf2[MIL];
  int count = 0;

  arg = one_argument(arg, buf);

  if (is_abbrev(buf, "help")) {
    goto who_help;
  }

  if (is_abbrev(buf, "filter")) {
    arg = one_argument(arg, buf);

    if (!*buf) {
      snprintf(buf2, sizeof(buf2), "Who Filters:");
      if (!GET_WHO_FILTER(ch)) {
        strcat(buf2, " None");
      }
      else {
        if (IS_SET(GET_WHO_FILTER(ch), WHO_FLT_CLASS)) {
          strcat(buf2, " Class");
        }
        if (IS_SET(GET_WHO_FILTER(ch), WHO_FLT_SUBCLASS)) {
          strcat(buf2, " Subclass");
        }
        if (IS_SET(GET_WHO_FILTER(ch), WHO_FLT_PRESTIGE)) {
          strcat(buf2, " Prestige");
        }
        if (IS_SET(GET_WHO_FILTER(ch), WHO_FLT_RANK)) {
          strcat(buf2, " Rank");
        }
        if (IS_SET(GET_WHO_FILTER(ch), WHO_FLT_TITLE)) {
          strcat(buf2, " Title");
        }
        if (IS_SET(GET_WHO_FILTER(ch), WHO_FLT_FLAGS)) {
          strcat(buf2, " Flags");
        }
      }
      printf_to_char(ch, "%s\n\r", buf2);

      return;
    }

    if (is_abbrev(buf, "help")) {
      goto who_help;
    }

    for (char *who_filter = buf; *who_filter; arg = one_argument(arg, who_filter)) {
      if (is_abbrev(buf, "class") || is_abbrev(buf, "level")) {
        if (IS_SET(GET_WHO_FILTER(ch), WHO_FLT_CLASS)) {
          REMOVE_BIT(GET_WHO_FILTER(ch), WHO_FLT_CLASS);
          printf_to_char(ch, "Who filter set to include Class and Level data.\n\r");
        }
        else {
          SET_BIT(GET_WHO_FILTER(ch), WHO_FLT_CLASS);
          printf_to_char(ch, "Who filter set to exclude Class and Level data.\n\r");
        }
      }
      else if (is_abbrev(buf, "subclass")) {
        if (IS_SET(GET_WHO_FILTER(ch), WHO_FLT_SUBCLASS)) {
          REMOVE_BIT(GET_WHO_FILTER(ch), WHO_FLT_SUBCLASS);
          printf_to_char(ch, "Who filter set to include Subclass data.\n\r");
        }
        else {
          SET_BIT(GET_WHO_FILTER(ch), WHO_FLT_SUBCLASS);
          printf_to_char(ch, "Who filter set to exclude Subclass data.\n\r");
        }
      }
      else if (is_abbrev(buf, "rank")) {
        if (IS_SET(GET_WHO_FILTER(ch), WHO_FLT_RANK)) {
          REMOVE_BIT(GET_WHO_FILTER(ch), WHO_FLT_RANK);
          printf_to_char(ch, "Who filter set to include Rank data.\n\r");
        }
        else {
          SET_BIT(GET_WHO_FILTER(ch), WHO_FLT_RANK);
          printf_to_char(ch, "Who filter set to exclude Rank data.\n\r");
        }
      }
      else if (is_abbrev(buf, "prestige")) {
        if (IS_SET(GET_WHO_FILTER(ch), WHO_FLT_PRESTIGE)) {
          REMOVE_BIT(GET_WHO_FILTER(ch), WHO_FLT_PRESTIGE);
          printf_to_char(ch, "Who filter set to include Prestige data.\n\r");
        }
        else {
          SET_BIT(GET_WHO_FILTER(ch), WHO_FLT_PRESTIGE);
          printf_to_char(ch, "Who filter set to exclude Prestige data.\n\r");
        }
      }
      else if (is_abbrev(buf, "title")) {
        if (IS_SET(GET_WHO_FILTER(ch), WHO_FLT_TITLE)) {
          REMOVE_BIT(GET_WHO_FILTER(ch), WHO_FLT_TITLE);
          printf_to_char(ch, "Who filter set to include Title data.\n\r");
        }
        else {
          SET_BIT(GET_WHO_FILTER(ch), WHO_FLT_TITLE);
          printf_to_char(ch, "Who filter set to exclude Title data.\n\r");
        }
      }
      else if (is_abbrev(buf, "flags")) {
        if (IS_SET(GET_WHO_FILTER(ch), WHO_FLT_FLAGS)) {
          REMOVE_BIT(GET_WHO_FILTER(ch), WHO_FLT_FLAGS);
          printf_to_char(ch, "Who filter set to include Flag data.\n\r");
        }
        else {
          SET_BIT(GET_WHO_FILTER(ch), WHO_FLT_FLAGS);
          printf_to_char(ch, "Who filter set to exclude Flag data.\n\r");
        }
      }
      else {
        printf_to_char(ch, "Unknown filter '%s'.  Type 'who --help' for usage info.\n\r", buf);
      }
    }

    return;
  }

  bool use_temp_filter = FALSE;
  ubyte temp_filter = UCHAR_MAX;

  int min_level = 0, max_level = 0;
  char *p;

  for (char *who_switch = buf; *who_switch; arg = one_argument(arg, who_switch)) {
    if (isdigit(*(who_switch))) {
      p = strchr(who_switch, '-');

      if (p) {
        snprintf(buf2, sizeof(buf2), "%.*s", strcspn(who_switch, "-"), who_switch);

        min_level = MAX(1, atoi(buf2));
        max_level = MIN(LEVEL_IMP, atoi(p + 1));
      }
      else {
        min_level = MAX(1, atoi(who_switch));
        max_level = LEVEL_IMP;
      }

      if (min_level > max_level) {
        printf_to_char(ch, "Minimum level must be greater than maximum level; disabling level filter.\n\r");

        min_level = 0;
        max_level = 0;
      }

      continue;
    }

    if (*who_switch != '-') continue;

    ++who_switch;

    if (*who_switch  == '-') ++who_switch;

    if (is_abbrev(who_switch, "class") || is_abbrev(who_switch, "level")) {
      REMOVE_BIT(temp_filter, WHO_FLT_CLASS);
    }
    else if (is_abbrev(who_switch, "subclass")) {
      REMOVE_BIT(temp_filter, WHO_FLT_SUBCLASS);
    }
    else if (is_abbrev(who_switch, "prestige")) {
      REMOVE_BIT(temp_filter, WHO_FLT_PRESTIGE);
    }
    else if (is_abbrev(who_switch, "rank")) {
      REMOVE_BIT(temp_filter, WHO_FLT_RANK);
    }
    else if (is_abbrev(who_switch, "title")) {
      REMOVE_BIT(temp_filter, WHO_FLT_TITLE);
    }
    else if (is_abbrev(who_switch, "flags")) {
      REMOVE_BIT(temp_filter, WHO_FLT_FLAGS);
    }
    else if (is_abbrev(who_switch, "help")) {
      goto who_help;
    }
    else {
      printf_to_char(ch, "Unknown switch '%s'.  Type 'who help' or 'who -h' for usage info.\n\r", who_switch);
    }

    use_temp_filter = TRUE;
  }

  send_to_char("Players\n\r-------\n\r", ch);

  for (DESC *d = descriptor_list; d; d = d->next) {
    buf[0] = '\0';

    if (d->connected != CON_PLYNG) continue;

    CHAR *c = d->original ? d->original : d->character;

    if (IS_IMMORTAL(c) && !CAN_SEE(ch, c)) continue;
    if ((min_level > 1 && (GET_LEVEL(c) < min_level)) || (max_level > 1 && (GET_LEVEL(c) > max_level))) continue;

    bool add_space = FALSE;

    if ((!use_temp_filter && !IS_SET(WHO_FLT_CLASS, GET_WHO_FILTER(ch))) || (use_temp_filter && !IS_SET(WHO_FLT_CLASS, temp_filter))) {
      if (IS_IMMORTAL(c)) {
        snprintf(buf2, sizeof(buf2), "[ %3s ]",
          immortal_abbrevs[GET_LEVEL(c) - LEVEL_IMM]);
      }
      else {
        snprintf(buf2, sizeof(buf2), "[%2d %2s]",
          GET_LEVEL(c), class_abbrevs[GET_CLASS(c)]);
      }
      strcat(buf, buf2);

      add_space = TRUE;
    }

    if ((!use_temp_filter && !IS_SET(WHO_FLT_SUBCLASS, GET_WHO_FILTER(ch))) || (use_temp_filter && !IS_SET(WHO_FLT_SUBCLASS, temp_filter))) {
      if (IS_IMMORTAL(c)) {
        snprintf(buf2, sizeof(buf2), "[ ** ]");
      }
      else {
        snprintf(buf2, sizeof(buf2), "[%d %2s]",
          GET_SC_LEVEL(c), subclass_abbrevs[GET_SC(c)]);
      }
      strcat(buf, buf2);

      add_space = TRUE;
    }

    if ((!use_temp_filter && !IS_SET(WHO_FLT_PRESTIGE, GET_WHO_FILTER(ch))) || (use_temp_filter && !IS_SET(WHO_FLT_PRESTIGE, temp_filter))) {
      if (IS_IMMORTAL(c)) {
        snprintf(buf2, sizeof(buf2), "[ **** ]");
      }
      else {
        snprintf(buf2, sizeof(buf2), "[%3d Pr]",
          GET_PRESTIGE(c));
      }
      strcat(buf, buf2);

      add_space = TRUE;
    }

    if ((!use_temp_filter && !IS_SET(WHO_FLT_RANK, GET_WHO_FILTER(ch))) || (use_temp_filter && !IS_SET(WHO_FLT_RANK, temp_filter))) {
      if (add_space) {
        snprintf(buf2, sizeof(buf2), " ");
      }
      else {
        buf2[0] = '\0';
      }

      signal_char(c, ch, MSG_SHOW_PRETITLE, buf2);
      strcat(buf, buf2);

      add_space = FALSE;
    }

    snprintf(buf2, sizeof(buf2), "%s%s",
      add_space ? " " : "",
      GET_NAME(c) ? GET_NAME(c) : "(null)");
    strcat(buf, buf2);

    if ((!use_temp_filter && !IS_SET(WHO_FLT_TITLE, GET_WHO_FILTER(ch))) || (use_temp_filter && !IS_SET(WHO_FLT_TITLE, temp_filter))) {
      snprintf(buf2, sizeof(buf2), " %s%s",
        GET_TITLE(c) ? GET_TITLE(c) : "", ENDCHCLR(ch));
      strcat(buf, buf2);
    }

    if ((!use_temp_filter && !IS_SET(WHO_FLT_FLAGS, GET_WHO_FILTER(ch))) || (use_temp_filter && !IS_SET(WHO_FLT_FLAGS, temp_filter))) {
      if (IS_SET(GET_PFLAG(c), PLR_KILL)) {
        strcat(buf, " (killer)");
      }

      if (IS_SET(GET_PFLAG(c), PLR_THIEF)) {
        strcat(buf, " (thief)");
      }

      if (IS_SET(GET_PFLAG(c), PLR_KILL)) {
        strcat(buf, " (deputy)");
      }

      if (IS_SET(GET_PFLAG(c), PLR_NOMESSAGE)) {
        strcat(buf, " (deaf)");
      }

      if ((IS_AFFECTED(c, AFF_INVISIBLE) && !IS_AFFECTED(c, AFF_IMINV)) && (NRM_INV(ch, c) || IS_IMMORTAL(ch))) {
        strcat(buf, " (invis)");
      }

      if (IS_AFFECTED(c, AFF_IMINV) && (IMP_INV(ch, c) || IS_IMMORTAL(ch))) {
        strcat(buf, " (impy)");
      }

      if (IS_SET(GET_PFLAG(c), PLR_QUEST)) {
        strcat(buf, " (quest)");
      }
    }

    if (GET_WIZINV(c) > 1) {
      snprintf(buf2, sizeof(buf2), " (i%d)", GET_WIZINV(c));
      strcat(buf, buf2);
    }

    printf_to_char(ch, "%s\n\r", buf);

    count++;
  }

  printf_to_char(ch, "\n\rThere %s %d visible player%s connected.\n\r",
    count != 1 ? "are" : "is",
    count,
    count != 1 ? "s" : "");

  printf_to_char(ch, "With a boot time high of %d player%s.\n\r",
    max_connects,
    max_connects != 1 ? "s" : "");

  if (IS_IMMORTAL(ch))
    printf_to_char(ch, "%d player%s %s connected since boot.\n\r",
      total_connects,
      total_connects != 1 ? "s" : "",
      total_connects != 1 ? "have" : "has");

  return;

who_help:
  printf_to_char(ch, "\
Usage:\n\r\
\n\r\
who [-c|-s|-p|-r|-t|-f] [min_level][-[max_level]]\n\r\
\n\r\
Switches:\n\r\
  -c | --class\n\r\
  -l | --level      Display player Class and Level data.\n\r\
  -s | --subclass   Display player Subclass data.\n\r\
  -p | --prestige   Display player Prestige data.\n\r\
  -r | --rank       Display player Rank data.\n\r\
  -t | --title      Display player Title data.\n\r\
  -f | --flags      Display player Flag data.\n\r\
\n\r\
Switch Example: who -c -r -f (shows output with Class, Rank, and Flags data).\n\r\
\n\r\
Setting Filters:\n\r\
\n\r\
who filter [class/level|subclass|prestige|rank|title|flags]\n\r\
\n\r\
Filter Example: who filter subclass (toggles Subclass data filter on/off).\n\r");

  return;
}


void do_users(struct char_data *ch, char *argument, int cmd)
{
  char line[256],tmp[256],arg[MSL];
  struct descriptor_data *d;
  int count = 0,timer;
  struct sockaddr_in isa;

  one_argument(argument, arg);
  if(!*arg) strcpy(arg,"#");

  send_to_char("Num Class   Name         State        Idle Site               \n\r", ch);
  send_to_char("--- ------- ------------ ------------ ---- -------------------\n\r", ch);

  for (d = descriptor_list; d; d = d->next){
    if (d->character && GET_NAME(d->character) &&
        CAN_SEE(ch, d->character)) {

      if(d->connected==CON_PLYNG) timer=d->character->specials.timer;
      else timer=d->timer;
      if(d->original)
        sprintf(line, "#%2d [-- --] %-12s %-20s %2d ", d->descriptor,
               GET_NAME(d->original),connected_types[d->connected],
               timer);
      else
        sprintf(line, "#%2d [%2d %2s] %-12s %-12s %2d ",
                d->descriptor, d->character->player.level,
                class_abbrevs[d->character->player.class],
                GET_NAME(d->character),connected_types[d->connected],
                timer);
      if (*d->host!='\0')
        sprintf(line + strlen(line), "[%-20s]\n\r", d->host);
      else if (d->addr) {
        isa.sin_addr.s_addr=d->addr;
        sprintf(line + strlen(line), "[%-20s]\n\r", inet_ntoa(isa.sin_addr));
      }
      else
        strcat(line, "[Hostname unknown]\n\r");
      strcpy(tmp,line);
      string_to_lower(tmp);
      if(strstr(tmp,arg)) {
        count++;
        send_to_char(line, ch);
      }
    }
  }
  sprintf(line,"\n\r%d visible sockets connected.\n\r",count);
  send_to_char(line, ch);

}

void do_inventory(struct char_data *ch, char *argument, int cmd) {
  send_to_char("You are carrying:\n\r", ch);
  list_obj_to_char(ch->carrying, ch, 1, TRUE);
}


void do_equipment(struct char_data *ch, char *argument, int cmd)
{
  int equip_pos = 0;
  bool found = FALSE;
  OBJ *equipment = NULL;

  const int EMITH_CAPE_1 = 26709;
  const int EMITH_CAPE_2 = 26719;

  send_to_char("You are using:\n\r", ch);

  for (equip_pos = 0; equip_pos < MAX_WEAR; equip_pos++) {
    equipment = EQ(ch, equip_pos);

    if (equipment) {
      found = TRUE;

      if (CAN_SEE_OBJ(ch, equipment)) {
        send_to_char(where[equip_pos], ch);
        show_obj_to_char(equipment, ch, 1, 0);
      }
      else {
        send_to_char(where[equip_pos], ch);
        send_to_char("Something.\n\r", ch);
      }

      switch (equip_pos) {
        case WEAR_NECK_1:
          /* ITEM_WEAR_2NECK and Emith cape uses both neck slots, similar to a 2-handed weapon. */
          if (CAN_WEAR(equipment, ITEM_WEAR_2NECK) || (V_OBJ(equipment) == EMITH_CAPE_1) || (V_OBJ(equipment) == EMITH_CAPE_2)) {
            send_to_char(where[WEAR_NECK_2], ch);
            send_to_char("********\n\r", ch);
            equip_pos++; /* Skip WEAR_NECK_2. This assumes WEAR_NECK_2 is always immediately after WEAR_NECK_1. */
          }
          break;
        case WIELD:
          if ((OBJ_TYPE(equipment) == ITEM_2H_WEAPON) &&
              !(IS_MORTAL(ch) && check_subclass(ch, SC_MERCENARY, 5) && GET_WEAPON2(ch))) { /* Sidearm */
            send_to_char(where[HOLD], ch);
            send_to_char("********\n\r", ch);
            equip_pos++; /* Skip HOLD. This assumes HOLD is always immediately after WIELD. */
          }
          break;
      }
    }
    else {
      if (GET_LEVEL(ch) < LEVEL_IMM) {
        send_to_char(where[equip_pos], ch);
        send_to_char("Nothing.\n\r", ch);
      }
    }
  }

  if (!found && GET_LEVEL(ch) >= LEVEL_IMM) {
    send_to_char("Nothing.\n\r", ch);
  }
}

void do_credits(struct char_data *ch, char *argument, int cmd) {
  page_string(ch->desc, credits, 0);
}

void do_heroes(struct char_data *ch, char *argument, int cmd) {
  page_string(ch->desc, heroes, 0);
}


void do_motd(struct char_data *ch, char *argument, int cmd) {
  char buf[MAX_INPUT_LENGTH];
  int level = GET_LEVEL(ch);

  one_argument (argument, buf);

  if (!*buf) level = GET_LEVEL(ch);
  else if (!strcmp(buf, "newbie")) level = 1;
  else if (!strcmp(buf, "mortal")) level = 6;
  else if (!strcmp(buf, "immortal") && GET_LEVEL(ch) >= LEVEL_IMM)
    level = LEVEL_IMM;

  if(level < 6)
    page_string(ch->desc, newbiemotd, 0);
  else if (level < LEVEL_IMM)
    page_string(ch->desc, motd, 0);
  else
    page_string(ch->desc, godmotd, 0);
}

void do_wizlist(struct char_data *ch, char *argument, int cmd) {
  char buf[40];
  FILE *wiz_fl,*wiz_in_fl;

  one_argument (argument, buf);

  if(*buf && !strcmp(buf, "create") && GET_LEVEL(ch) > LEVEL_SUP) {
    if (!(wiz_fl = fopen (WIZLIST_FILE, "w"))) {
      send_to_char ("ATTENTION:: Could not open wizlist for online update!\n",
        ch);
      return;
    } else {
      create_wizlist (wiz_fl);
      file_to_string (WIZLIST_FILE, wizlist);
    }

    if (!(wiz_in_fl = fopen (WIZLIST_INACTIVE_FILE, "w"))) {
      log_s("   Could not open inactive wizlist.");
    } else {
      create_inactive_wizlist (wiz_in_fl);
      file_to_string(WIZLIST_INACTIVE_FILE, wizlist_inactive);
    }
  }
  if(*buf && !strcmp(buf, "inactive"))
    page_string(ch->desc, wizlist_inactive, 0);
  else
    page_string(ch->desc, wizlist, 0);
}

static int which_number_mobile(struct char_data *ch, struct char_data *mob)
{
  struct char_data      *i;
  char  *name;
  int   number;

  name = fname(GET_NAME(mob));
  for (i=character_list, number=0; i; i=i->next) {
    if (isname(name, GET_NAME(i)) && CHAR_REAL_ROOM(i) != NOWHERE) {
      number++;
      if (i==mob)
        return number;
    }
  }
  return 0;
}

char *numbered_person(struct char_data *ch, struct char_data *person)
{
  static char buf[MSL];
  if (IS_NPC(person)) {
    sprintf(buf, "%d.%s", which_number_mobile(ch, person),
            fname(GET_NAME(person)));
  }  else {
    strcpy(buf, PERS(person, ch));
  }
  return buf;
}

static void do_where_person(struct char_data *ch, struct char_data *person,
          struct string_block *sb)
{
  char buf[MSL];

  sprintf(buf, "%-30s- %s ", PERS(person, ch),
          (CHAR_REAL_ROOM(person) -1 ? world[CHAR_REAL_ROOM(person)].name : "Nowhere"));

  if (GET_LEVEL(ch) >= LEVEL_IMM)
    sprintf(buf+strlen(buf),"[%d]", world[CHAR_REAL_ROOM(person)].number);

  strcpy(buf+strlen(buf), "\n\r");

  append_to_string_block(sb, buf);
}

static void do_where_object(struct char_data *ch, struct obj_data *obj,
                            int recurse, struct string_block *sb)
{
  int virtual;
  char buf[MSL];

  virtual = (obj->item_number >= 0 ? obj_proto_table[obj->item_number].virtual : 0);
  if (obj->in_room != NOWHERE) { /* object in a room */
    sprintf(buf, "[%5d]%-25s- %s [%d]\n\r",
      virtual,
            OBJ_SHORT(obj),
            world[obj->in_room].name,
            world[obj->in_room].number);
  } else if (obj->carried_by != NULL) { /* object carried by monster */
    sprintf(buf, "[%5d]%-25s- carried by %s (Room %5d)\n\r",
      virtual,
            OBJ_SHORT(obj),
            numbered_person(ch, obj->carried_by), obj->carried_by->in_room_v);
  } else if (obj->equipped_by != NULL) { /* object equipped by monster */
    sprintf(buf, "[%5d]%-25s- equipped by %s (Room %5d)\n\r",
      virtual,
            OBJ_SHORT(obj),
            numbered_person(ch, obj->equipped_by), obj->equipped_by->in_room_v);
  } else if (obj->in_obj) { /* object in object */
      /* Single level parsing of an obj in a container - Ranger June 97*/
      if(obj->in_obj->in_room != NOWHERE)
        sprintf(buf, "[%5d]%-25s- in %s (Room %d)\n\r",virtual,OBJ_SHORT(obj),
                OBJ_SHORT(obj->in_obj),world[obj->in_obj->in_room].number);
      else if(obj->in_obj->carried_by != NULL)
        sprintf(buf, "[%5d]%-25s- in %s (%s)\n\r",virtual,OBJ_SHORT(obj),
                OBJ_SHORT(obj->in_obj),numbered_person(ch, obj->in_obj->carried_by));
      else if(obj->in_obj->equipped_by != NULL)
        sprintf(buf, "[%5d]%-25s- in %s (%s)\n\r",virtual,OBJ_SHORT(obj),
                OBJ_SHORT(obj->in_obj),numbered_person(ch, obj->in_obj->equipped_by));
      else if (obj->in_obj->in_obj)
        sprintf(buf, "[%5d]%-25s- in %s (In: %s)\n\r",virtual,OBJ_SHORT(obj),
                OBJ_SHORT(obj->in_obj),OBJ_SHORT(obj->in_obj->in_obj));
      else
        sprintf(buf, "[%5d]%-25s- in %s (unknown)\n\r",virtual,OBJ_SHORT(obj),
                OBJ_SHORT(obj->in_obj));
  } else {
    sprintf(buf, "%-30s- god doesn't even know where...\n\r",
            OBJ_SHORT(obj));
  }
  if (*buf)
    append_to_string_block(sb, buf);
  if (recurse) {
    if (obj->in_room != NOWHERE)
      return;
    else if (obj->carried_by != NULL)
      do_where_person(ch, obj->carried_by, sb);
    else if (obj->equipped_by != NULL)
      do_where_person(ch, obj->equipped_by, sb);
    else if (obj->in_obj != NULL)
      do_where_object(ch, obj->in_obj, TRUE, sb);
  }
}

void do_where(struct char_data *ch, char *argument, int cmd)
{
  char name[MAX_INPUT_LENGTH], buf[MSL];
  char  *nameonly;
  register struct char_data *i;
  register struct obj_data *k;
  struct descriptor_data *d;
  int   number, count,found=FALSE,owhere=FALSE;
  struct string_block   sb;

  argument=one_argument(argument, name);
  if(!strcmp("o", name)) {
    owhere=TRUE;
    one_argument(argument, name);
  }
  if(GET_LEVEL(ch)<LEVEL_IMM) owhere=FALSE;

  if (!*name) {
    if (GET_LEVEL(ch) < LEVEL_IMM) {
      strcpy(buf, "Players:\n\r--------\n\r");
      for (d = descriptor_list; d; d = d-> next) {
        if (d->character && (d->connected == CON_PLYNG)
            && (CHAR_REAL_ROOM(d->character) != NOWHERE) &&
            (world[CHAR_REAL_ROOM(d->character)].zone==world[CHAR_REAL_ROOM(ch)].zone)
            && CAN_SEE(ch, d->character)) {
          sprintf(buf, "%-20s - %s \n\r",
                  GET_NAME(d->character),
                  world[CHAR_REAL_ROOM(d->character)].name);
          send_to_char(buf, ch);
        }
      }
      return;
    } else      {
      init_string_block(&sb);
      append_to_string_block(&sb, "Players:\n\r--------\n\r");

      for (d = descriptor_list; d; d = d->next) {
        if (d->character && (d->connected == CON_PLYNG)
      && (CHAR_REAL_ROOM(d->character) != NOWHERE)
      && CAN_SEE(ch, d->character)) {
          if (d->original)   /* If switched */
            sprintf(buf, "%-20s - %s [%d] In body of %s\n\r",
                    GET_NAME(d->original),
                    world[CHAR_REAL_ROOM(d->character)].name,
                    world[CHAR_REAL_ROOM(d->character)].number, /* Ranger - June 96 */
                    fname(GET_NAME(d->character)));
          else
            sprintf(buf, "%-20s - %s [%d]\n\r",
                    GET_NAME(d->character),
                    world[CHAR_REAL_ROOM(d->character)].name,
                    world[CHAR_REAL_ROOM(d->character)].number);

          append_to_string_block(&sb, buf);
        }
      }
      page_string_block(&sb,ch);
      destroy_string_block(&sb);
      return;
    }
  }

  if (isdigit(*name)) {
    nameonly = name;
    count = number = get_number(&nameonly);
  } else {
    count = number = 0;
  }

  *buf = '\0';

  init_string_block(&sb);

  if(!owhere) {
    for (i = character_list; i; i = i->next)
      if (isname(name, GET_NAME(i)) && CAN_SEE(ch, i) )        {
         if(!strcmp(GET_NAME(i),"rashgugh") && GET_LEVEL(ch)>=LEVEL_IMM) {
           sprintf(buf,"WIZINFO: %s where'd Rashgugh.",GET_NAME(ch));
           wizlog(buf,LEVEL_IMP,5);
           log_s(buf);
           *buf = '\0';
/*           send_to_char("Couldn't find any such thing.\n\r",ch);
           destroy_string_block(&sb);
          return;*/
         }

        if ((CHAR_REAL_ROOM(i) != NOWHERE) &&
            ((GET_LEVEL(ch)>=LEVEL_IMM) || (world[CHAR_REAL_ROOM(i)].zone ==
              world[CHAR_REAL_ROOM(ch)].zone))) {
          if (number==0 || (--count) == 0) {
            if (number==0) {
              sprintf(buf, "[%2d] ", ++count); /* I love short circuiting :) */
              append_to_string_block(&sb, buf);
            }
            do_where_person(ch, i, &sb);
            *buf = 1;
            if (number!=0)
              break;
          }
          if (GET_LEVEL(ch) < LEVEL_IMM)
            break;
        }
      }
  }

  /*  count = number;*/

  if (GET_LEVEL(ch) >= LEVEL_IMM && owhere) {
    for (k = object_list; k; k = k->next)
      if (isname(name, OBJ_NAME(k)) && CAN_SEE_OBJ(ch, k)) {
        if(k->obj_flags.type_flag==ITEM_SC_TOKEN && GET_LEVEL(ch)<LEVEL_SUP)
          continue;
        if (number==0 || (--count)==0) {
          if (number==0) {
            sprintf(buf, "[%2d] ", ++count);
            append_to_string_block(&sb, buf);
          }
          if(!found) {
            sprintf(buf, "WIZINFO: (%s) where object %s", GET_NAME(ch), name);
            wizlog(buf, GET_LEVEL(ch)+1, 5);
            log_s(buf);
          }

          do_where_object(ch, k, number!=0, &sb);
          found=TRUE;
          *buf = 1;
          if (number!=0)
            break;
        }
      }
  }

  if (!*sb.data)
    send_to_char("Couldn't find any such thing.\n\r", ch);
  else
    page_string_block(&sb, ch);
  destroy_string_block(&sb);
}

void do_levels(struct char_data *ch, char *argument, int cmd)
{
  int i;
  char buf[MSL*2];

  if (IS_NPC(ch)) {
    send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
    return;
  }

  *buf = '\0';
  for (i = 1; i < LEVEL_IMM; i++) {
    sprintf(buf + strlen(buf), "[%2d] %10d : ", i, exp_table[i]);
    switch(GET_SEX(ch)) {
    case SEX_MALE:
      strcat(buf, titles[GET_CLASS(ch) - 1][i].title_m); break;
    case SEX_FEMALE:
      strcat(buf, titles[GET_CLASS(ch) - 1][i].title_f); break;
    default:
      send_to_char("Oh dear.\n\r", ch); break;
    }
    strcat(buf, "\n\r");
  }
  send_to_char(buf, ch);
}



void do_consider(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  int diff;

  one_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("Consider killing who?\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("Easy! Very easy indeed!\n\r", ch);
    return;
  }

  if (!IS_NPC(victim)) {
    send_to_char("Would you like to borrow a cross and a shovel?\n\r", ch);
    return;
  }

  diff = (GET_LEVEL(victim)-GET_LEVEL(ch));

  /* monsters are harder at level 20+ */
  if (GET_LEVEL(victim) > 19)
    diff += 5;

  if (diff <= -10)
    send_to_char("Now where did that chicken go?\n\r", ch);
  else if (diff <= -5)
    send_to_char("You could do it with a needle!\n\r", ch);
  else if (diff <= -2)
    send_to_char("Easy.\n\r", ch);
  else if (diff <= -1)
    send_to_char("Fairly easy.\n\r", ch);
  else if (diff == 0)
    send_to_char("The perfect match!\n\r", ch);
  else if (diff <= 1)
    send_to_char("You would need some luck!\n\r", ch);
  else if (diff <= 2)
    send_to_char("You would need a lot of luck!\n\r", ch);
  else if (diff <= 3)
    send_to_char("You would need a lot of luck and great equipment!\n\r", ch);
  else if (diff <= 5)
    send_to_char("Do you feel lucky, punk?\n\r", ch);
  else if (diff <= 10)
    send_to_char("Are you mad!?\n\r", ch);
  else if (diff <= 15)
    send_to_char("You ARE mad!\n\r", ch);
  else if (diff <= 100)
    send_to_char("You're either insane or suicidal!\n\r", ch);
}

void do_whois(struct char_data *ch, char *argument, int cmd) {
  struct descriptor_data *d;
  FILE *fl;
  char buf[MSL], buf2[MSL];
  char name[MIL],host[50];
  int days, hours, mins, secs;
  int version,class,level,subclass=0,subclass_level=0,prestige=0;
  struct char_file_u_5 char_info_5;
  struct char_file_u_4 char_info_4;
  struct char_file_u_2 char_info_2;

  one_argument(argument, name);

  if (!*name) {
    send_to_char("Whois who?\n\r", ch);
    return;
  }

  for (d = descriptor_list; d; d = d->next) {
    if (d->character &&
        GET_NAME(d->character) &&
        isname(name, GET_NAME(d->character)) &&
        CAN_SEE(ch, d->character)) {
      if (IS_IMMORTAL(d->character)) {
        printf_to_char(ch, "%s is %s %s.\n\r",
          GET_NAME(d->character),
          index("aeiouyAEIOUY", *(immortal_types[GET_LEVEL(d->character) - LEVEL_IMM])) ? "an" : "a",
          immortal_types[GET_LEVEL(d->character) - LEVEL_IMM]);
      }
      else {
        printf_to_char(ch, "%s is a level %d %s.\n\r",
          GET_NAME(d->character),
          GET_LEVEL(d->character),
          pc_class_types[(int)GET_CLASS(d->character)]);
      }

      if (!IS_IMMORTAL(d->character)) {
        if (GET_SC(d->character)) {
          printf_to_char(ch, "Subclass: %s, Level %d\n\r",
            subclass_name[GET_SC(d->character) - 1],
            GET_SC_LEVEL(d->character));
        }

        if (GET_PRESTIGE(d->character)) {
          printf_to_char(ch, "Prestige: %d\n\r", GET_PRESTIGE(d->character));
        }
      }

      if (d->host) {
        buf2[0] = 0;

        sscanf(&d->host[strlen(d->host) - 5], "%*[^.].%s", buf2);

        if (is_number(buf2)) {
          send_to_char("Last logged from numeric address.\n\r", ch);
        }
        else {
          printf_to_char(ch, "Last logged from *.%s.\n\r", buf2);
        }
      }
      else {
        send_to_char("Last logged from unknown.\n\r", ch);
      }

      if (!IS_IMMORTAL(d->character) && d->character->specials.timer) {
        printf_to_char(ch, "Idle: %d minute%s.\n\r",
          d->character->specials.timer,
          d->character->specials.timer == 1 ? "" : "s");
      }

      return;
    }
  }

  string_to_lower(name);
  sprintf(buf, "rent/%c/%s.dat", UPPER(name[0]), name);

  if (!(fl = fopen(buf, "rb"))) {
    send_to_char("You didn't find anyone by that name.\n\r", ch);
    return;
  }

  version = char_version(fl);

  switch (version) {
  case 2:
    memset(&char_info_2, 0, sizeof(char_info_2));
    if (fread(&char_info_2, sizeof(struct char_file_u_2), 1, fl) < 1) {
      log_s("Error reading rent file (do_whois).");
      fclose(fl);
      return;
    }
    fclose(fl);
    class = (int)char_info_2.class;
    level = char_info_2.level;
    secs = time(0) - char_info_2.last_update;
    sprintf(host, "%s", char_info_2.new.host);
    break;
  case 3:
    memset(&char_info_4, 0, sizeof(char_info_4));
    if (fread(&char_info_4, sizeof(struct char_file_u_4), 1, fl) < 1) {
      log_s("Error reading rent file (do_whois).");
      fclose(fl);
      return;
    }
    fclose(fl);
    class = (int)char_info_4.class;
    subclass = char_info_4.ver3.subclass;
    subclass_level = char_info_4.ver3.subclass_level;
    level = char_info_4.level;
    if (level >= 31) level = LEVEL_IMM;
    secs = time(0) - char_info_4.last_update;
    sprintf(host, "%s", char_info_4.new.host);
    break;
  case 4:
    memset(&char_info_4, 0, sizeof(char_info_4));
    if (fread(&char_info_4, sizeof(struct char_file_u_4), 1, fl) < 1) {
      log_s("Error reading rent file (do_whois).");
      fclose(fl);
      return;
    }
    fclose(fl);
    class = (int)char_info_4.class;
    subclass = char_info_4.ver3.subclass;
    subclass_level = char_info_4.ver3.subclass_level;
    level = char_info_4.level;
    secs = time(0) - char_info_4.last_update;
    sprintf(host, "%s", char_info_4.new.host);
    break;
  case 5:
    memset(&char_info_5, 0, sizeof(char_info_5));
    if (fread(&char_info_5, sizeof(struct char_file_u_5), 1, fl) < 1) {
      log_s("Error reading rent file (do_whois).");
      fclose(fl);
      return;
    }
    fclose(fl);
    class = (int)char_info_5.class;
    subclass = char_info_5.ver3.subclass;
    subclass_level = char_info_5.ver3.subclass_level;
    prestige = char_info_5.ver3.prestige;
    level = char_info_5.level;
    secs = time(0) - char_info_5.last_update;
    sprintf(host, "%s", char_info_5.new.host);
    break;
  default:
    log_s("Error getting pfile version (do_whois)");
    return;
  }

  CAP(name);

  if (level >= LEVEL_IMM) {
    printf_to_char(ch, "%s is %s %s.\n\r",
      name,
      index("aeiouyAEIOUY", *(immortal_types[level - LEVEL_IMM])) ? "an" : "a",
      immortal_types[level - LEVEL_IMM]);
  }
  else {
    printf_to_char(ch, "%s is a level %d %s.\n\r",
      name,
      level,
      pc_class_types[class]);
  }

  if (level <= LEVEL_IMM) {
    if (subclass) {
      printf_to_char(ch, "Subclass: %s, Level %d\n\r",
        subclass_name[subclass - 1],
        subclass_level);
    }

    if (prestige) {
      printf_to_char(ch, "Prestige: %d\n\r", prestige);
    }
  }

  printf_to_char(ch, "%s isn't in now.\n\r", name);

  days = secs / SECS_PER_REAL_DAY;
  secs -= days * SECS_PER_REAL_DAY;
  hours = secs / SECS_PER_REAL_HOUR;
  secs -= hours * SECS_PER_REAL_HOUR;
  mins = secs / SECS_PER_REAL_MIN;
  secs -= (mins * SECS_PER_REAL_MIN);

  buf2[0] = 0;

  sscanf(host + strlen(host) - 5, "%*[^.].%s", buf2);

  if (is_number(buf2))
    send_to_char("Originally logged from numeric address.\n\r", ch);
  else {
    printf_to_char(ch, "Originally logged from *.%s.\n\r", buf2);
  }

  printf_to_char(ch, "Last on %d days, %d hours, %d minute, and %d seconds ago.\n\r",
    days,
    hours,
    mins,
    secs);
}

/* For future use.
bool compare_objects(OBJ *obj1, OBJ *obj2) {
  if (!obj1 || !obj2) return FALSE;

  if (OBJ_RNUM(obj1) != OBJ_RNUM(obj2)) return FALSE;
  if (OBJ_TYPE(obj1) != OBJ_TYPE(obj2)) return FALSE;
  if (OBJ_WEAR_FLAGS(obj1) != OBJ_WEAR_FLAGS(obj2)) return FALSE;
  if (OBJ_BITS(obj1) != OBJ_BITS(obj2)) return FALSE;
  if (OBJ_BITS2(obj1) != OBJ_BITS2(obj2)) return FALSE;
  if (OBJ_EXTRA_FLAGS(obj1) != OBJ_EXTRA_FLAGS(obj2)) return FALSE;
  if (OBJ_EXTRA_FLAGS2(obj1) != OBJ_EXTRA_FLAGS2(obj2)) return FALSE;
  if (strcmp(OBJ_NAME(obj1), OBJ_NAME(obj2))) return FALSE;
  if (strcmp(OBJ_SHORT(obj1), OBJ_SHORT(obj2))) return FALSE;
  if (strcmp(OBJ_DESCRIPTION(obj1), OBJ_DESCRIPTION(obj2))) return FALSE;

  return TRUE;
}
*/
