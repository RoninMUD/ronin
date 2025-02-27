/*
**  file: spec_procs.c , Special module.                   Part of DIKUMUD
**  Usage: Procedures handling special procedures for object/room/mobile
**  Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <limits.h>

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
#include "act.h"
#include "reception.h"
#include "spec_assign.h"
#include "time.h"
#include "subclass.h"
#include "enchant.h"

char *Month[12]= {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

int USE_MANA(CHAR *ch, int sn);

extern char *skip_spaces(char * string);
extern int is_all_dot(char *arg, char *allbuf);

int SPELL_LEVEL(struct char_data *ch, int sn);
void send_to_world(char*arg);
char *PERS(CHAR *ch, CHAR*vict);
void v_log_f(char *str);

/* Data declarations */
struct social_type {
  char *cmd;
  int next_line;
};


/* ********************************************************************
 *  Special procedures for rooms                                       *
 ******************************************************************** */


void list_skills_to_prac(CHAR *ch, bool list_all)
{
  int number = 0;
  char buf[MIL];

  send_to_char("\
`nSkill Name                     `kHow Well       `oPrac\n\r\
`n------------------------------ `k-------------- `o-----`q\n\r", ch);

  for (int i = 0, done = FALSE; !done; i++) {
    switch (GET_CLASS(ch))
    {
      case CLASS_MAGIC_USER:
        done = TRUE;
      break;

      case CLASS_CLERIC:
        if (*cleric_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(cleric_skills[i], 0, strlen(cleric_skills[i]), (const char * const * const)spells, TRUE);

          if (number == 0) continue;
          else if ((GET_LEARNED(ch, number) >= MAX_PRAC(ch)) && !list_all) continue;
          else if (!check_sc_access(ch, number)) continue;
          else if ((number == SKILL_BASH) && (GET_LEVEL(ch) < 35)) continue;
          else if ((number == SKILL_MEDITATE) && (GET_LEVEL(ch) < 40)) continue;
          else {
            sprintf(buf, "`n%-30s `k%-14s `o(%3d)`q\n\r", cleric_skills[i], how_good(GET_LEARNED(ch, number)), GET_LEARNED(ch, number));
            send_to_char(buf, ch);
          }
        }
        break;

      case CLASS_THIEF:
        if (*thief_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(thief_skills[i], 0, strlen(thief_skills[i]), (const char * const * const)spells, TRUE);

          if (number == 0) continue;
          else if ((GET_LEARNED(ch, number) >= MAX_PRAC(ch)) && !list_all) continue;
          else if (!check_sc_access(ch, number)) continue;
          else if ((number == SKILL_SCAN) && (GET_LEVEL(ch) < 35)) continue;
          else if ((number == SKILL_TWIST) && (GET_LEVEL(ch) < 45)) continue;
          else if ((number == SKILL_CUNNING) && (GET_LEVEL(ch) < 50)) continue;
          else
          {
            sprintf(buf, "`n%-30s `k%-14s `o(%3d)`q\n\r", thief_skills[i], how_good(GET_LEARNED(ch, number)), GET_LEARNED(ch, number));
            send_to_char(buf, ch);
          }
        }
        break;

      case CLASS_WARRIOR:
        if (*warrior_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(warrior_skills[i], 0, strlen(warrior_skills[i]), (const char * const * const)spells, TRUE);

          if (number == 0) continue;
          else if ((GET_LEARNED(ch, number) >= MAX_PRAC(ch)) && !list_all) continue;
          else if (!check_sc_access(ch, number)) continue;
          else if ((number == SKILL_TRIPLE) && (GET_LEVEL(ch) < 20)) continue;
          else if ((number == SKILL_DISEMBOWEL) && (GET_LEVEL(ch) < 40)) continue;
          else if ((number == SKILL_QUAD) && (GET_LEVEL(ch) < 50)) continue;
          else
          {
            sprintf(buf, "`n%-30s `k%-14s `o(%3d)`q\n\r", warrior_skills[i], how_good(GET_LEARNED(ch, number)), GET_LEARNED(ch, number));
            send_to_char(buf, ch);
          }
        }
        break;

      case CLASS_NINJA:
        if (*ninja_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(ninja_skills[i], 0, strlen(ninja_skills[i]), (const char * const * const)spells, TRUE);

          if (number == 0) continue;
          else if ((GET_LEARNED(ch, number) >= MAX_PRAC(ch)) && !list_all) continue;
          else if (!check_sc_access(ch, number)) continue;
          else {
            sprintf(buf, "`n%-30s `k%-14s `o(%3d)`q\n\r", ninja_skills[i], how_good(GET_LEARNED(ch, number)), GET_LEARNED(ch, number));
            send_to_char(buf, ch);
          }
        }
        break;

      case CLASS_NOMAD:
        if (*nomad_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(nomad_skills[i], 0, strlen(nomad_skills[i]), (const char * const * const)spells, TRUE);

          if (number == 0) continue;
          else if ((GET_LEARNED(ch, number) >= MAX_PRAC(ch)) && !list_all) continue;
          else if (!check_sc_access(ch, number)) continue;
          else if ((number == SKILL_DISEMBOWEL) && (GET_LEVEL(ch) < 40)) continue;
          else if ((number == SKILL_EVASION) && (GET_LEVEL(ch) < 50)) continue;
          else if ((number == SKILL_SCAN) && !check_subclass(ch, SC_TRAPPER, 1)) continue;
          else if ((number == SKILL_BLOCK) && !check_subclass(ch, SC_RANGER, 3)) continue;
          else {
            sprintf(buf, "`n%-30s `k%-14s `o(%3d)`q\n\r", nomad_skills[i], how_good(GET_LEARNED(ch, number)), GET_LEARNED(ch, number));
            send_to_char(buf, ch);
          }
        }
        break;

      case CLASS_PALADIN:
        if (*paladin_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(paladin_skills[i], 0, strlen(paladin_skills[i]), (const char * const * const)spells, TRUE);

          if (number == 0) continue;
          else if ((GET_LEARNED(ch, number) >= MAX_PRAC(ch)) && !list_all) continue;
          else if (!check_sc_access(ch, number)) continue;
          else if ((number == SKILL_PRAY) && (GET_LEVEL(ch) < 40)) continue;
          else
          {
            sprintf(buf, "`n%-30s `k%-14s `o(%3d)`q\n\r", paladin_skills[i], how_good(GET_LEARNED(ch, number)), GET_LEARNED(ch, number));
            send_to_char(buf, ch);
          }
        }
        break;

      case CLASS_ANTI_PALADIN:
        if (*anti_paladin_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(anti_paladin_skills[i], 0, strlen(anti_paladin_skills[i]), (const char * const * const)spells, TRUE);

          if (number == 0) continue;
          else if ((GET_LEARNED(ch, number) >= MAX_PRAC(ch)) && !list_all) continue;
          else if (!check_sc_access(ch, number)) continue;
          else if ((number == SKILL_HIDDEN_BLADE) && (GET_LEVEL(ch) < 40)) continue;
          else if ((number == SKILL_ASSASSINATE) && (GET_LEVEL(ch) < 45)) continue;
          else
          {
            sprintf(buf, "`n%-30s `k%-14s `o(%3d)`q\n\r", anti_paladin_skills[i], how_good(GET_LEARNED(ch, number)), GET_LEARNED(ch, number));
            send_to_char(buf, ch);
          }
        }
        break;

      case CLASS_AVATAR:
        if (*avatar_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(avatar_skills[i], 0, strlen(avatar_skills[i]), (const char * const * const)spells, TRUE);

          if (number == 0) continue;
          else if ((GET_LEARNED(ch, number) >= MAX_PRAC(ch)) && !list_all) continue;
          else if (!check_sc_access(ch, number)) continue;
          else
          {
            sprintf(buf, "`n%-30s `k%-14s `o(%3d)`q\n\r", avatar_skills[i], how_good(GET_LEARNED(ch, number)), GET_LEARNED(ch, number));
            send_to_char(buf, ch);
          }
        }
        break;

      case CLASS_BARD:
        if (*bard_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(bard_skills[i], 0, strlen(bard_skills[i]), (const char * const * const)spells, TRUE);

          if (number == 0) continue;
          else if ((GET_LEARNED(ch, number) >= MAX_PRAC(ch)) && !list_all) continue;
          else if (!check_sc_access(ch, number)) continue;
          else if ((number == SKILL_TAUNT) && (GET_LEVEL(ch) < 20)) continue;
          else
          {
            sprintf(buf, "`n%-30s `k%-14s `o(%3d)`q\n\r", bard_skills[i], how_good(GET_LEARNED(ch, number)), GET_LEARNED(ch, number));
            send_to_char(buf, ch);
          }
        }
        break;

      case CLASS_COMMANDO:
        if (*commando_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(commando_skills[i], 0, strlen(commando_skills[i]), (const char * const * const)spells, TRUE);

          if (number == 0) continue;
          else if ((GET_LEARNED(ch, number) >= MAX_PRAC(ch)) && !list_all) continue;
          else if (!check_sc_access(ch, number)) continue;
          else if ((number == SKILL_TRIPLE) && (GET_LEVEL(ch) < 20)) continue;
          else
          {
            sprintf(buf, "`n%-30s `k%-14s `o(%3d)`q\n\r", commando_skills[i], how_good(GET_LEARNED(ch, number)), GET_LEARNED(ch, number));
            send_to_char(buf, ch);
          }
        }
        break;
    }
  }
}


void list_spells_to_prac(CHAR *ch, bool list_all) {
  struct string_block sb;
  char buf[MIL];

  init_string_block(&sb);

  snprintf(buf, sizeof(buf), "\
`nSpell Name                     `kHow Well       `oPrac  `jMana\n\r\
`n------------------------------ `k-------------- `o----- `j-----`q\n\r");
  append_to_string_block(&sb, buf);

  for (int i = 0; *spells[i] != '\n'; i++) {
    if (!spell_info[i + 1].spell_pointer) continue;
    else if ((ch->skills[i + 1].learned >= MAX_PRAC(ch)) && !list_all) continue;
    else if (SPELL_LEVEL(ch, i + 1) > GET_LEVEL(ch)) continue;
    else if (!check_sc_access(ch, i + 1)) continue;
    else {
      snprintf(buf, sizeof(buf), "`n%-30s `k%-14s `o(%3d) `j[%3d]`q\n\r",
        spells[i], how_good(GET_LEARNED(ch, i + 1)), GET_LEARNED(ch, i + 1), USE_MANA(ch, i + 1));
      append_to_string_block(&sb, buf);
    }
  }

  page_string_block(&sb, ch);

  destroy_string_block(&sb);
}


int practice_skill(CHAR *ch, int number) {
  if (number == 0 || number == 200) {
    send_to_char("`iThat skill wasn't found.`q\n\r", ch);

    return TRUE;
  }

  if (GET_PRAC(ch) <= 0) {
    send_to_char("`iYou do not seem to be able to practice now.`q\n\r", ch);

    return TRUE;
  }

  if (!check_sc_access(ch, number)) {
    return FALSE;
  }

  int bonus = 0;

  switch (number) {
    case SKILL_BACKSTAB:
      if (enchanted_by_type(ch, ENCHANT_MINION)) {
        bonus += 5;
      }
      break;

    case SKILL_BASH:
      if (GET_CLASS(ch) == CLASS_CLERIC) {
        bonus -= 10;
      }
      break;

    case SKILL_BLOCK:
      if (enchanted_by_type(ch, ENCHANT_SWASHBUCKLER) || enchanted_by_type(ch, ENCHANT_JUSTICIAR)) {
        bonus += 5;
      }
      break;

    case SKILL_DUAL:
      if (enchanted_by_type(ch, ENCHANT_KNIGHT) || enchanted_by_type(ch, ENCHANT_PRIVATE)) {
        bonus += 5;
      }
      break;

    case SKILL_CIRCLE:
      if (enchanted_by_type(ch, ENCHANT_ASSASSIN)) {
        bonus += 5;
      }
      break;

    case SKILL_TRIPLE:
      if (enchanted_by_type(ch, ENCHANT_COMMANDER)) {
        bonus += 5;
      }
      break;

    case SKILL_ASSAULT:
      if (enchanted_by_type(ch, ENCHANT_COMMODORE) || enchanted_by_type(ch, ENCHANT_SHOGUN)) {
        bonus += 5;
      }
      break;

    case SKILL_AMBUSH:
      if (enchanted_by_type(ch, ENCHANT_WANDERER)) {
        bonus += 5;
      }
      break;

    case SKILL_DISEMBOWEL:
      if (enchanted_by_type(ch, ENCHANT_TAMER)) {
        bonus += 5;
      }
      break;

    case SKILL_PUMMEL:
      if (enchanted_by_type(ch, ENCHANT_LORDLADY) || enchanted_by_type(ch, ENCHANT_SHINOBI)) {
        bonus += 5;
      }
      break;

    case SKILL_TAUNT:
      if (enchanted_by_type(ch, ENCHANT_CONDUCTOR)) {
        bonus += 5;
      }
      break;

    case SKILL_MEDITATE:
    case SKILL_DEGENERATE:
      if (GET_CLASS(ch) == CLASS_CLERIC) {
        bonus -= 10;
      }
      break;
  }

  if (GET_LEARNED(ch, number) >= (MAX_PRAC(ch) + bonus)) {
    send_to_char("`iYou are already learned in this area.`q\n\r", ch);

    return TRUE;
  }

  send_to_char("`iYou practice for a while...`q\n\r", ch);

  GET_PRAC(ch)--;

  GET_LEARNED(ch, number) = MIN((GET_LEARNED(ch, number) + MIN(int_app[GET_INT(ch)].learn, 18)), (MAX_PRAC(ch) + bonus));

  if (GET_LEARNED(ch, number) >= MAX_PRAC(ch)) {
    send_to_char("`iYou are now learned in this area.`q\n\r", ch);
  }

  return TRUE;
}


int practice_spell(CHAR *ch, int number) {
  if (!spell_info[number].spell_pointer) return FALSE;

  if (number == 0) {
    send_to_char("`iThat spell wasn't found.`q\n\r", ch);

    return TRUE;
  }

  if (GET_PRAC(ch) <= 0) {
    send_to_char("`iYou do not seem to be able to practice now.`q\n\r", ch);

    return TRUE;
  }

  if ((GET_LEVEL(ch) < SPELL_LEVEL(ch, number)) || !check_sc_access(ch, number)) {
    return FALSE;
  }

  if (GET_LEARNED(ch, number) >= MAX_PRAC(ch)) {
    send_to_char("`iYou are already learned in this area.`q\n\r", ch);

    return TRUE;
  }

  send_to_char("`iYou practice for a while...`q\n\r", ch);

  GET_PRAC(ch)--;

  GET_LEARNED(ch, number) = MIN(GET_LEARNED(ch, number) + int_app[GET_INT(ch)].learn, MAX_PRAC(ch));

  if (GET_LEARNED(ch, number) >= MAX_PRAC(ch)) {
    send_to_char("`iYou are now learned in this area.`q\n\r", ch);
  }

  return TRUE;
}


int quest_giver(CHAR *mob, CHAR *ch, int cmd, char *argument);
int check_sc_master(CHAR *ch, CHAR *mob);
int guild(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  int showSpells = FALSE;
  int showSkills = FALSE;
  int spell = -1;
  int skill = -1;
  int index = -1;
  OBJ *qcard = NULL;
  int i = 0;

  arg = skip_spaces(arg);

  if (cmd == CMD_AQUEST) {
    if (quest_giver(mob, ch, cmd, arg)) return TRUE;

    return FALSE;
  }

  /* Give back any questcards given to the mob, as players should use 'aquest complete' to get credit. */
  if (cmd == MSG_OBJ_GIVEN) {
    if (!isname("questcard", arg)) return FALSE;

    qcard = get_obj_in_list_vis(mob, "questcard", mob->carrying);

    if (qcard && (V_OBJ(qcard) == 35)) {
      act("$N tells you 'You should hold on to these.'", FALSE, ch, 0, mob, TO_CHAR);
      act("$N gives you $q.", FALSE, ch, qcard, mob, TO_CHAR);
      obj_from_char(qcard);
      obj_to_char(qcard, ch);

      return TRUE;
    }
    else {
      act("$N tells you 'That wasn't a questcard you gave me...'", FALSE, ch, 0, mob, TO_CHAR);

      return TRUE;
    }
  }

  if (cmd != CMD_PRACTICE || !ch->skills) return FALSE;

  if (!*arg) {
    printf_to_char(ch, "`iYou have %d practice sessions.`q\n\r", ch->specials.spells_to_learn);

    switch (GET_CLASS(ch)) {
      case CLASS_MAGIC_USER:
        showSpells = TRUE;
        break;

      case CLASS_CLERIC:
        showSpells = TRUE;

        if ((GET_LEVEL(ch) >= 35) || check_sc_access(ch, SKILL_MEDITATE)) {
          showSkills = TRUE;
        }
        break;

      case CLASS_NINJA:
      case CLASS_PALADIN:
      case CLASS_ANTI_PALADIN:
      case CLASS_AVATAR:
      case CLASS_BARD:
      case CLASS_COMMANDO:
        showSkills = TRUE;
        showSpells = TRUE;
        break;

      case CLASS_THIEF:
      case CLASS_WARRIOR:
      case CLASS_NOMAD:
        showSkills = TRUE;
        break;
    }

    if (showSpells || showSkills) {
      send_to_char("\n\r", ch);
    }

    if (showSkills) {
      send_to_char("`iYou can practice any of these skills:`q\n\r\n\r", ch);
      list_skills_to_prac(ch, FALSE);
    }

    if (showSpells && showSkills) {
      send_to_char("\n\r", ch);
    }

    if (showSpells) {
      send_to_char("`iYou can practice any of these spells:`q\n\r\n\r", ch);
      list_spells_to_prac(ch, FALSE);
    }

    return TRUE;
  }

  /* convert argument to lowercase */
  for(i = 0; arg[i]; i++){
    arg[i] = tolower(arg[i]);
  }

  spell = old_search_block(arg, 0, strlen(arg),(const char * const * const)spells, FALSE);

  if (spell == -1) {
    act("`i$N shrugs, indicating that $E doesn't know what you're talking about.`q", FALSE, ch, 0, mob, TO_CHAR);
    return TRUE;
  }

  switch (GET_CLASS(ch))
  {
    case CLASS_MAGIC_USER:
      index = -1;
    break;

    case CLASS_CLERIC:
      index = search_block(arg, cleric_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(cleric_skills[index], 0, strlen(cleric_skills[index]), (const char * const * const)spells, TRUE);
      }

      if ((skill == SKILL_BASH) && (GET_LEVEL(ch) < 35))
      {
        index = -2;
      }
      break;

    case CLASS_THIEF:
      index = search_block(arg, thief_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(thief_skills[index], 0, strlen(thief_skills[index]), (const char * const * const)spells, TRUE);
      }

      if (((skill == SKILL_SCAN) && (GET_LEVEL(ch) < 35)) ||
          ((skill == SKILL_CUNNING) && (GET_LEVEL(ch) < 50)))
      {
        index = -2;
      }
      break;

    case CLASS_WARRIOR:
      index = search_block(arg, warrior_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(warrior_skills[index], 0, strlen(warrior_skills[index]), (const char * const * const)spells, TRUE);
      }

      if (((skill == SKILL_TRIPLE) && (GET_LEVEL(ch) < 20)) ||
          ((skill == SKILL_DISEMBOWEL) && (GET_LEVEL(ch) < 40)) ||
          ((skill == SKILL_QUAD) && (GET_LEVEL(ch) < 50)))
      {
        index = -2;
      }
      break;

    case CLASS_NINJA:
      index = search_block(arg, ninja_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(ninja_skills[index], 0, strlen(ninja_skills[index]), (const char * const * const)spells, TRUE);
      }

      if ((skill == SPELL_DIVINE_WIND) && (GET_LEVEL(ch) < 40))
      {
        index = -2;
      }
      break;

    case CLASS_NOMAD:
      index = search_block(arg, nomad_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(nomad_skills[index], 0, strlen(nomad_skills[index]), (const char * const * const)spells, TRUE);
      }

      if (((skill == SKILL_DISEMBOWEL) && (GET_LEVEL(ch) < 40)) ||
          ((skill == SKILL_EVASION) && (GET_LEVEL(ch) < 50))) {
        index = -2;
      }
      else if ((skill == SKILL_SCAN) && !check_subclass(ch, SC_TRAPPER, 1)) {
        index = -1;
      }

      break;

    case CLASS_PALADIN:
      index = search_block(arg, paladin_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(paladin_skills[index], 0, strlen(paladin_skills[index]), (const char * const * const)spells, TRUE);
      }

      if ((skill == SKILL_PRAY) && (GET_LEVEL(ch) < 40))
      {
        index = -2;
      }
      break;

    case CLASS_ANTI_PALADIN:
      index = search_block(arg, anti_paladin_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(anti_paladin_skills[index], 0, strlen(anti_paladin_skills[index]), (const char * const * const)spells, TRUE);
      }

      if ((skill == SKILL_HIDDEN_BLADE) && (GET_LEVEL(ch) < 40))
      {
        index = -2;
      }

      if ((skill == SKILL_ASSASSINATE) && (GET_LEVEL(ch) < 45))
      {
        index = -2;
      }
      break;

    case CLASS_AVATAR:
      index = search_block(arg, avatar_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(avatar_skills[index], 0, strlen(avatar_skills[index]), (const char * const * const)spells, TRUE);
      }
      break;

    case CLASS_BARD:
      index = search_block(arg, bard_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(bard_skills[index], 0, strlen(bard_skills[index]), (const char * const * const)spells, TRUE);
      }

      if (((skill == SKILL_TAUNT) && (GET_LEVEL(ch) < 20)) ||
          ((skill == SKILL_CAMP) && (GET_LEVEL(ch) < 35)))
      {
        index = -2;
      }
      break;

    case CLASS_COMMANDO:
      index = search_block(arg, commando_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(commando_skills[index], 0, strlen(commando_skills[index]), (const char * const * const)spells, TRUE);
      }

      if ((skill == SKILL_TRIPLE) && (GET_LEVEL(ch) < 20))
      {
        index = -2;
      }
      break;
  }

  if (index == -1) {
    if (spell == -1) {
      act("`i$N shrugs, indicating that $E doesn't know what you're talking about.`q", FALSE, ch, 0, mob, TO_CHAR);
      return TRUE;
    }

    if ((spell > 165) &&
        check_sc_access(ch, spell) &&
        ((GET_CLASS(ch) != CLASS_NINJA) || (spell != SPELL_DIVINE_WIND)) &&
        ((GET_CLASS(ch) != CLASS_ANTI_PALADIN) || (spell != SPELL_RAGE)) &&
        ((GET_CLASS(ch) != CLASS_COMMANDO) || (spell != SPELL_IRON_SKIN)) &&
        !check_sc_master(ch, mob)) {
      act("`i$N tells you 'Your subclass master is the only one who can teach that spell.'`q", FALSE, ch, 0, mob, TO_CHAR);
      return TRUE;
    }

    if (practice_spell(ch, spell)) return TRUE;

    act("`i$N shrugs, indicating that $E doesn't know what you're talking about.`q", FALSE, ch, 0, mob, TO_CHAR);
    return TRUE;
  }
  else if (index == -2) {
    act("`i$N tells you 'Come back when you're more experienced.'`q", FALSE, ch, 0, mob, TO_CHAR);
    return TRUE;
  }

  if ((GET_CLASS(ch) == CLASS_NOMAD) &&
      (((skill == SKILL_SCAN) && check_subclass(ch, SC_TRAPPER, 1)) ||
       ((skill == SKILL_CAMP) && check_subclass(ch, SC_TRAPPER, 2)) ||
       ((skill == SKILL_BLOCK) && check_subclass(ch, SC_RANGER, 3))) &&
      !check_sc_master(ch, mob)) {
    act("`i$N tells you 'Go see your subclass master to practice that.'`q", FALSE, ch, 0, mob, TO_CHAR);
    return TRUE;
  }

  if ((skill > 165) &&
      check_sc_access(ch, skill) &&
      ((GET_CLASS(ch) != CLASS_CLERIC) || (spell != SKILL_MEDITATE)) &&
      ((GET_CLASS(ch) != CLASS_NOMAD) || (skill != SKILL_EVASION)) &&
      ((GET_CLASS(ch) != CLASS_BARD) || (skill != SKILL_CAMP)) &&
      !check_sc_master(ch, mob)) {
    act("`i$N tells you 'Go see your subclass master to practice that.'`q", FALSE, ch, 0, mob, TO_CHAR);
    return TRUE;
  }

  if (practice_skill(ch, skill)) return TRUE;

  act("`i$N shrugs, indicating that $E doesn't know what you're talking about.`q", FALSE, ch, 0, mob, TO_CHAR);
  return TRUE;
}

#define STORAGE_ROOM    5807

int dump(int room, CHAR *ch, int cmd, char *arg) {
  OBJ *k;
  OBJ *tmp_c = NULL, *next_c = NULL;
  char buf[100];
  CHAR *tmp_char;
  int value=0;

  if(!ch) return(FALSE);

  /* On any command, an object would be removed if it was on the ground. */
  for(k = world[CHAR_REAL_ROOM(ch)].contents; k ;
      k = world[CHAR_REAL_ROOM(ch)].contents) {
    sprintf(buf, "The %s vanish in a puff of smoke.\n\r" , OBJ_SHORT(k));
    for(tmp_char = world[CHAR_REAL_ROOM(ch)].people; tmp_char;
        tmp_char = tmp_char->next_in_room)
      if (CAN_SEE_OBJ(tmp_char, k)) send_to_char(buf,tmp_char);
    extract_obj(k);
  }

  if(cmd != CMD_DROP)  return(FALSE);
  do_drop(ch, arg, cmd);
  value = 0;

  for(k = world[CHAR_REAL_ROOM(ch)].contents; k; k = world[CHAR_REAL_ROOM(ch)].contents) {
    // updated to unpack containers which might contain an AQ_ORDER
    // and moves AQ_ORDERs to Storage Room - no free way to "quit" them
    // also by unpacking containers, get poof message for all contents
    if (OBJ_TYPE(k) == ITEM_AQ_ORDER) {
      sprintf(buf, "%s vanishes in a brilliant flash of bright light.\n\r", CAP(OBJ_SHORT(k)));
      send_to_room(buf, CHAR_REAL_ROOM(ch));
      obj_from_room(k);
      obj_to_room(k, real_room(STORAGE_ROOM));
    }
    else {
      if (OBJ_TYPE(k)==ITEM_CONTAINER) {
        for (tmp_c = k->contains; tmp_c; tmp_c = next_c) {
            next_c = tmp_c->next_content;
            obj_from_obj(tmp_c);
            obj_to_room(tmp_c, CHAR_REAL_ROOM(ch));
        }
      }

      sprintf(buf, "The %s vanishes in a puff of smoke.\n\r", rem_prefix(OBJ_SHORT(k)));
      for(tmp_char = world[CHAR_REAL_ROOM(ch)].people; tmp_char;
          tmp_char = tmp_char->next_in_room) {
        if (CAN_SEE_OBJ(tmp_char, k)) {
          send_to_char(buf,tmp_char);
        }
      }
      value += MAX(1, MIN(50, k->obj_flags.cost/10));
      extract_obj(k);
      if(IS_MORTAL(ch)) {
        sprintf(buf,"DUMP: [ %s dropped %s at The Dump ]",GET_NAME(ch),OBJ_SHORT(k));
      } else if(IS_NPC(ch)) {
        sprintf(buf,"DUMP: [ %s dropped %s at The Dump ]",MOB_SHORT(ch),OBJ_SHORT(k));
      }
      log_s(buf);
    }
  }

  if (value) {
    act("You are awarded for outstanding performance.",
         FALSE, ch, 0, 0, TO_CHAR);
    act("$n has been awarded for being a good citizen.",
         TRUE, ch, 0,0, TO_ROOM);

    if (GET_LEVEL(ch) < 3) gain_exp(ch, value);
    else GET_GOLD(ch) += value;
  }
  return (TRUE);
}

void v_log_f(char *str) {
  long ct;
  char *tmstr;
  FILE *vlog;

  vlog=fopen("vault/vault.log","a");
  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  fprintf(vlog, "%s :: %s\n", tmstr, str);
  fclose(vlog);
}

/*
**Storage Vaults - By Quack (Dec 96)
**
** Last Update: Sept 97 by Ranger
**  - Total key removal
**  - name list for each vault
**  - active IMP control access
*/
void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode, bool show);

void obj_to_vault(struct obj_data *obj, FILE *fl, CHAR * ch,char pos, char *name) {
  int j;
  char logit[MAX_STRING_LENGTH];
  struct obj_data *tmp;
  struct obj_file_elem_ver3 object;

  memset(&object,0,sizeof(object));

  if (!obj) return;
  if(!IS_RENTABLE(obj)) return; /*damn mushroom */

  object.position   =pos;
  object.item_number=obj_proto_table[obj->item_number].virtual;
  object.value[0]   =obj->obj_flags.value[0];
  object.value[1]   =obj->obj_flags.value[1];
  object.value[2]   =obj->obj_flags.value[2];
  object.value[3]   =obj->obj_flags.value[3];
  if (obj->obj_flags.type_flag == ITEM_CONTAINER)
       object.value[3]=0; /* for storing stuff in containters =COUNT_RENTABLE_CONTENTS(obj)*/
  object.extra_flags=obj->obj_flags.extra_flags;
  object.weight     =obj->obj_flags.weight;
  object.timer      =obj->obj_flags.timer;
  object.bitvector  =obj->obj_flags.bitvector;

/* new obj saves */
  object.version=32003;
  object.type_flag=obj->obj_flags.type_flag;
  object.wear_flags=obj->obj_flags.wear_flags;
  object.extra_flags2=obj->obj_flags.extra_flags2;
  object.subclass_res=obj->obj_flags.subclass_res;
  object.material=obj->obj_flags.material;
  object.spec_value =OBJ_SPEC(obj);
  for(j=0;j<MAX_OBJ_AFFECT;j++)
    object.affected[j]=obj->affected[j];
/* end new obj saves */

/* new obj saves for obj ver3 */
      object.bitvector2  = obj->obj_flags.bitvector2;
      object.popped      = obj->obj_flags.popped;
/* end new ver3 obj saves */

/* New owner id */
      object.ownerid[0] =obj->ownerid[0];
      object.ownerid[1] =obj->ownerid[1];
      object.ownerid[2] =obj->ownerid[2];
      object.ownerid[3] =obj->ownerid[3];
      object.ownerid[4] =obj->ownerid[4];
      object.ownerid[5] =obj->ownerid[5];
      object.ownerid[6] =obj->ownerid[6];
      object.ownerid[7] =obj->ownerid[7];

  fwrite(&object, sizeof(object),1,fl);

  sprintf(logit,"%s stores %s (%d) in %s's vault.",
          GET_NAME(ch), OBJ_SHORT(obj),V_OBJ(obj),CAP(name));
  v_log_f(logit);

  if(GET_LEVEL(ch)>=LEVEL_IMM) {
    sprintf(logit, "WIZINFO: %s stores %s in %s's vault.",
            GET_NAME(ch), OBJ_SHORT(obj),CAP(name));
    wizlog(logit, GET_LEVEL(ch)+1, 5);
    log_s(logit);
  }

  if(COUNT_CONTENTS(obj)) {
    for (tmp = obj->contains;tmp;tmp = tmp->next_content)
      obj_to_vault(tmp, fl, ch,-1,name);
  }
}

#define NO_VAULT       0
#define NO_ACCESS      1
#define ACCESS_LOOK    2
#define ACCESS_GET     3
#define ACCESS_CONTROL 4

int vault_filter(OBJ *obj,char *arg) {
  int wear_pos=99,class_wear=99;
  int wearable=FALSE;
  char buf[MAX_STRING_LENGTH];

  string_to_lower(arg);
  one_argument(arg,buf);
  if(!*buf) return FALSE;

  if(is_abbrev(buf, "key") && obj->obj_flags.type_flag==ITEM_KEY) wearable=TRUE;
  if(is_abbrev(buf, "boat") && obj->obj_flags.type_flag==ITEM_BOAT) wearable=TRUE;
  if(is_abbrev(buf, "lockpick") && obj->obj_flags.type_flag==ITEM_LOCKPICK) wearable=TRUE;
  if(is_abbrev(buf, "container") && obj->obj_flags.type_flag==ITEM_CONTAINER) wearable=TRUE;
  if(is_abbrev(buf, "recipe") && obj->obj_flags.type_flag==ITEM_RECIPE) wearable=TRUE;
  if(is_abbrev(buf, "potion") && obj->obj_flags.type_flag==ITEM_POTION) wearable=TRUE;
  if(is_abbrev(buf, "scroll") && obj->obj_flags.type_flag==ITEM_SCROLL) wearable=TRUE;
  if(is_abbrev(buf, "wand") && obj->obj_flags.type_flag==ITEM_WAND) wearable=TRUE;
  if(is_abbrev(buf, "staff") && obj->obj_flags.type_flag==ITEM_STAFF) wearable=TRUE;
  if(is_abbrev(buf, "drink") && obj->obj_flags.type_flag==ITEM_DRINKCON) wearable=TRUE;

  if(is_abbrev(buf, "light"))        wear_pos   = ITEM_LIGHT_SOURCE;
  if(is_abbrev(buf, "light") && obj->obj_flags.type_flag==ITEM_LIGHT) wearable=TRUE;
  if(is_abbrev(buf, "finger"))       wear_pos   = ITEM_WEAR_FINGER;
  if(is_abbrev(buf, "neck"))         wear_pos   = ITEM_WEAR_NECK;
  if(is_abbrev(buf, "2neck"))        wear_pos   = ITEM_WEAR_2NECK;
  if(is_abbrev(buf, "body"))         wear_pos   = ITEM_WEAR_BODY;
  if(is_abbrev(buf, "head"))         wear_pos   = ITEM_WEAR_HEAD;
  if(is_abbrev(buf, "legs"))         wear_pos   = ITEM_WEAR_LEGS;
  if(is_abbrev(buf, "feet"))         wear_pos   = ITEM_WEAR_FEET;
  if(is_abbrev(buf, "hands"))        wear_pos   = ITEM_WEAR_HANDS;
  if(is_abbrev(buf, "arms"))         wear_pos   = ITEM_WEAR_ARMS;
  if(is_abbrev(buf, "shield"))       wear_pos   = ITEM_WEAR_SHIELD;
  if(is_abbrev(buf, "about"))        wear_pos   = ITEM_WEAR_ABOUT;
  if(is_abbrev(buf, "waist"))        wear_pos   = ITEM_WEAR_WAIST;
  if(is_abbrev(buf, "wrist"))        wear_pos   = ITEM_WEAR_WRIST;
  if(is_abbrev(buf, "wield"))        wear_pos   = ITEM_WIELD;
  if(is_abbrev(buf, "hold"))         wear_pos   = ITEM_HOLD;
  if(is_abbrev(buf, "throw"))        wear_pos   = ITEM_THROW;

  if(wear_pos!=99 && CAN_WEAR(obj,wear_pos)) wearable=TRUE;

  if(is_abbrev(buf, "magic-user"))   class_wear = ITEM_ANTI_MAGIC_USER;
  if(is_abbrev(buf, "mage"))         class_wear = ITEM_ANTI_MAGIC_USER;
  if(is_abbrev(buf, "wizard"))       class_wear = ITEM_ANTI_MAGIC_USER;
  if(is_abbrev(buf, "cleric"))       class_wear = ITEM_ANTI_CLERIC;
  if(is_abbrev(buf, "thief"))        class_wear = ITEM_ANTI_THIEF;
  if(is_abbrev(buf, "warrior"))      class_wear = ITEM_ANTI_WARRIOR;
  if(is_abbrev(buf, "ninja"))        class_wear = ITEM_ANTI_NINJA;
  if(is_abbrev(buf, "nomad"))        class_wear = ITEM_ANTI_NOMAD;
  if(is_abbrev(buf, "paladin"))      class_wear = ITEM_ANTI_PALADIN;
  if(is_abbrev(buf, "anti-paladin")) class_wear = ITEM_ANTI_ANTIPALADIN;
  if(is_abbrev(buf, "ap"))           class_wear = ITEM_ANTI_ANTIPALADIN;
  if(is_abbrev(buf, "avatar"))       class_wear = ITEM_ANTI_AVATAR;
  if(is_abbrev(buf, "bard"))         class_wear = ITEM_ANTI_BARD;
  if(is_abbrev(buf, "commando"))     class_wear = ITEM_ANTI_COMMANDO;
  if(is_abbrev(buf, "evil"))         class_wear = ITEM_ANTI_EVIL;
  if(is_abbrev(buf, "neutral"))      class_wear = ITEM_ANTI_NEUTRAL;
  if(is_abbrev(buf, "good"))         class_wear = ITEM_ANTI_GOOD;

  if(class_wear!=99 && !IS_OBJ_STAT(obj, class_wear)) wearable=TRUE;

  if(is_abbrev(buf, "paladin"))      class_wear = ITEM_ANTI_GOOD;
  if(is_abbrev(buf, "anti-paladin")) class_wear = ITEM_ANTI_EVIL;
  if(is_abbrev(buf, "evil"))         class_wear = ITEM_ANTI_ANTIPALADIN;
  if(is_abbrev(buf, "good"))         class_wear = ITEM_ANTI_PALADIN;

  if(class_wear!=99 && !IS_OBJ_STAT(obj, class_wear)) wearable=TRUE;

  if(class_wear==ITEM_ANTI_CLERIC && !IS_OBJ_STAT(obj, class_wear) &&
       obj->obj_flags.type_flag==ITEM_WEAPON &&
       (obj->obj_flags.value[3]==3 || obj->obj_flags.value[3]>8))
      wearable=FALSE;

  if(class_wear==ITEM_ANTI_MAGIC_USER && !IS_OBJ_STAT(obj, class_wear) &&
       obj->obj_flags.type_flag==ITEM_WEAPON &&
       (obj->obj_flags.value[1]>3 || obj->obj_flags.value[2]>9))
      wearable=FALSE;

  return wearable;
}

int vault_access(char *vault_name, char *name)
{
  FILE *fd = NULL;
  char filename[MIL];
  char chk[MIL];

  sprintf(filename, "vault/%s.vault", vault_name);
  fd = fopen(filename, "r");

  if (!fd)
  {
    return NO_VAULT;
  }

  fclose(fd);

  sprintf(filename, "vault/%s.name", vault_name);
  fd = fopen(filename, "r");

  if (!fd)
  {
    return NO_VAULT;
  }

  /* Access to your own vault. */
  if (!strcmp(vault_name, name) ||
      !strcmp("ranger", name) ||
      !strcmp("sumo", name) ||
      !strcmp("lem", name) ||
      !strcmp("liner", name) ||
      !strcmp("sane", name) ||
      !strcmp("shun", name) ||
      !strcmp("night", name))
  {
    fclose(fd);

    return ACCESS_CONTROL;
  }

  /* Check vault access list. */
  sprintf(chk, " ");

  while (!feof(fd))
  {
    fscanf(fd, "%s\n", chk);

    if (!strcmp(chk, name))
    {
      fclose(fd);

      return ACCESS_GET;
    }
  }

  fclose(fd);

  return NO_ACCESS;
}

int get_vault_access_level(CHAR *ch, char *vault_name)
{
  FILE *fd = NULL;
  char name[MIL];
  char filename[MIL];
  char chk[MIL];

  strcpy(name, GET_NAME(ch));
  string_to_lower(name);
  sprintf(filename, "vault/%s.vault", vault_name);
  fd = fopen(filename, "r");

  if (!fd)
  {
    return NO_VAULT;
  }

  fclose(fd);

  sprintf(filename, "vault/%s.name", vault_name);
  fd = fopen(filename, "r");

  if (!fd)
  {
    return NO_VAULT;
  }

  /* Access to your own vault. */
  if (!strcmp(vault_name, name) ||
      IS_IMPLEMENTOR(ch))
  {
    fclose(fd);

    return ACCESS_CONTROL;
  }

  /* Check vault access list. */
  sprintf(chk, " ");
  
  while (!feof(fd))
  {
    fscanf(fd, "%s\n", chk);

    if (!strcmp(chk, name))
    {
      fclose(fd);

      return ACCESS_GET;
    }
  }

  fclose(fd);

  return NO_ACCESS;
}

void number_argument_interpreter(char *argument, int *number, char *first_arg, char *second_arg);

int vault_give(CHAR *vault_guard, CHAR *ch, char *arg, int cmd)
{
  char buf[MSL];
  char name[MIL];
  int num = 0;

  number_argument_interpreter(arg, &num, buf, name);

  if (!*arg || !*name) return FALSE;

  if (get_char_room_vis(ch, name) == vault_guard)
  {
    act("$n tells you 'Use the 'store' command if you want to store items.'", FALSE, vault_guard, 0, ch, TO_VICT);

    return TRUE;
  }

  return FALSE;
}

int vault_offer(CHAR *vault_guard, CHAR *ch, char *arg, int cmd)
{
  struct obj_data *obj;
  char buf[MAX_STRING_LENGTH];
  char obj_name[MAX_INPUT_LENGTH];

  if (!*arg)
  {
    act("$n asks you 'An offer for what?'", 0, vault_guard, 0, ch, TO_VICT);

    return TRUE;
  }

  one_argument(arg, obj_name);
  string_to_lower(obj_name);

  obj = get_obj_in_list_vis(ch, obj_name, ch->carrying);

  if (!obj)
  {
    sprintf(buf, "$n tells you 'You don't seem to have the %s.'", obj_name);
    act(buf, 0, vault_guard, 0, ch, TO_VICT);
  }
  else if (!IS_RENTABLE(obj))
  {
    act("$n tells you 'You can't store $p in a vault.'", FALSE, vault_guard, obj, ch, TO_VICT);
  }
  else
  {
    int storage_price = (total_cost_of_obj(obj) * 3) / 2;

    // Prestige Perk 18
    if (GET_PRESTIGE_PERK(ch) >= 18) {
      storage_price *= 0.9;
    }

    sprintf(buf, "$n tells you '$p will cost %d coins to store.'", storage_price);
    act(buf, FALSE, vault_guard, obj, ch, TO_VICT);
  }

  return TRUE;
}

int vault_use(CHAR *vault_guard, CHAR *ch, char *arg, int cmd)
{
  char buf[MSL];
  char temp[MIL];
  char vault_name[20];
  int access_level = 0;

  if (!*arg) return FALSE;

  argument_interpreter(arg, temp, vault_name);
  string_to_lower(temp);
  string_to_lower(vault_name);

  if (!is_abbrev(temp, "vault"))
  {
    return FALSE;
  }

  if (!*vault_name)
  {
    sprintf(buf, "Current Vault: %s\n\r", ch->specials.vaultname);
    send_to_char(buf, ch);

    return TRUE;
  }

  access_level = get_vault_access_level(ch, vault_name);

  switch (access_level)
  {
    case NO_VAULT:
      act("$n tells you 'That vault doesn't exist.'", FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
      break;

    case NO_ACCESS:
      act("$n tells you 'You don't have access to that vault.'", FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
      break;

    case ACCESS_LOOK:
      act("$n tells you 'Look access enabled.'", FALSE, vault_guard, 0, ch, TO_VICT);

      break;

    case ACCESS_GET:
      act("$n tells you 'Look/Recover/Store access enabled.'", FALSE, vault_guard, 0, ch, TO_VICT);

      break;

    case ACCESS_CONTROL:
      act("$n tells you 'Look/Recover/Store/Control access enabled.'", FALSE, vault_guard, 0, ch, TO_VICT);

      break;

    default:
      send_to_char("Invalid vault access. Please report this to an immortal.\n\r", ch);
      sprintf(buf, "[vault_use()] Invalid vault access. Player: %s, Vault: %s, Access: %d",
        GET_NAME(ch), ch->specials.vaultname, ch->specials.vaultaccess);
      wizlog(buf, LEVEL_ETE, WIZ_LOG_SIX);

      return TRUE;
      break;
  }

  ch->specials.vaultaccess = access_level;
  strcpy(ch->specials.vaultname, CAP(vault_name));

  return TRUE;
}

int vault_list(CHAR *vault_guard, CHAR *ch, char *arg, int cmd)
{
  FILE *fd = NULL;
  OBJ *obj = NULL;
  OBJ *temp_obj = NULL;
  struct program_info dtail;
  char buf[MSL];
  char filename[MIL];
  char vault_name[20];
  char last_used[20];
  char name[20];
  int counter = 0;

  arg = skip_spaces(arg);

  if (!*arg)
  {
    send_to_char("\
Usage: list <access|all|keyword|vaults> or\n\r\
light, finger, neck, body, head, legs, feet, hands,\n\r\
arms, shield, about, waist, wrist, wield, hold, throw,\n\r\
magic_user/mage/wizard, cleric, thief, warrior, ninja,\n\r\
nomad, paladin, anti-paladin, avatar, bard, commando,\n\r\
evil, neutral, good\n\r\
container, drink, boat, lockpick, recipe.\n\r\
potion, scroll, wand, staff.\n\r",ch);

    return TRUE;
  }

  one_argument(arg, buf);
  string_to_lower(buf);

  /* Listing all vaults player has access to */
  if (!strcmp(buf, "vaults"))
  {
    strcpy(name, GET_NAME(ch));
    string_to_lower(name);

    dtail.args[0] = strdup("vault/list_vaults");
    dtail.args[1] = strdup(name);
    dtail.args[2] = NULL;
    dtail.args[3] = NULL;
    dtail.input = NULL;
    dtail.timeout = 10;
    dtail.name = strdup("lvault");

    add_program(dtail, ch);

    return TRUE;
  }

  switch (ch->specials.vaultaccess)
  {
    case NO_VAULT:
      act("$n tells you 'Your currently set vault doesn't exist.'", 0, vault_guard, 0, ch, TO_VICT);

      return TRUE;
      break;

    case NO_ACCESS:
      act("$n tells you 'You don't have access to your currently set vault.'", 0, vault_guard, 0, ch, TO_VICT);

      return TRUE;
      break;

    case ACCESS_LOOK:
    case ACCESS_GET:
    case ACCESS_CONTROL:
      break;

    default:
      send_to_char("Invalid vault access. Please report this to an immortal.\n\r", ch);
      sprintf(buf, "[vault_list()] Invalid vault access. Player: %s, Vault: %s, Access: %d",
        GET_NAME(ch), ch->specials.vaultname, ch->specials.vaultaccess);
      wizlog(buf, LEVEL_ETE, WIZ_LOG_SIX);

      return TRUE;
      break;
  }

  strcpy(vault_name, ch->specials.vaultname);
  string_to_lower(vault_name);

  /* Checking access of current set vault */
  if (!strcmp(buf, "access"))
  {
    sprintf(filename, "vault/%s.name", vault_name);
    fd = fopen(filename, "r");

    if (!fd)
    {
      act("$n tells you 'Your currently set vault doesn't exist.'", FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
    }
    else
    {
      sprintf(buf, "The following players have access to %s's vault.\n\r\n\r", CAP(vault_name));
      send_to_char(buf, ch);

      sprintf(buf, "%s\n\r", CAP(vault_name));
      send_to_char(buf, ch);

      *name = '\0';

      while (!feof(fd))
      {
        fscanf(fd, "%s\n", name);

        if (*name)
        {
          sprintf(buf, "%s\n\r", CAP(name));
          send_to_char(buf, ch);
        }
      }

      fclose(fd);
    }

    return TRUE;
  }

  sprintf(filename, "vault/%s.last", vault_name);
  fd = fopen(filename, "r");

  if (!fd)
  {
    sprintf(last_used, "Unknown");
  }
  else
  {
    *name = '\0';

    while (!feof(fd))
    {
      fscanf(fd, "%s\n", name);

      if (*name)
      {
        sprintf(last_used, "%s", CAP(name));
      }
      else
      {
        sprintf(last_used, "Unknown");
      }
    }

    fclose(fd);
  }

  sprintf(filename, "vault/%s.vault", vault_name);
  fd = fopen(filename, "rb");

  if (!fd)
  {
    act("$n tells you 'Your currently set vault doesn't exist.'", FALSE, vault_guard, 0, ch, TO_VICT);

    return TRUE;
  }

  CREATE(temp_obj, OBJ, 1);
  clear_object(temp_obj);
  temp_obj->obj_flags.cost_per_day = 0;

  sprintf(buf, "\n\r%s's Storage Vault: %s items\n\r", CAP(vault_name), arg);
  send_to_char(buf, ch);
  send_to_char("---------------------------------------------\n\r", ch);

  sprintf(buf, "Last Transaction(s) by: %s\n\r", last_used);
  send_to_char(buf, ch);
  send_to_char("---------------------------------------------\n\r", ch);

  string_to_lower(arg);
  arg = skip_spaces(arg);

  while (!feof(fd))
  {
    strcpy(buf, arg);

    switch (obj_version(fd))
    {
      case 3:
        obj = store_to_obj_ver3(fd, ch);
        break;

      case 2:
        obj = store_to_obj_ver2(fd, ch);
        break;

      case 1:
        obj = store_to_obj_ver1(fd, ch);
        break;

      case 0:
        obj = store_to_obj_ver0(fd, ch);
        break;

      default:
        obj = 0;
        break;
    }

    if (obj)
    {
      if (is_abbrev(buf, "all") ||
          isname(buf, OBJ_NAME(obj)) ||
          vault_filter(obj, buf))
      {
        obj_to_obj(obj, temp_obj);
      }
      else
      {
        extract_obj(obj);
      }
    }
  }

  counter = COUNT_RENTABLE_CONTENTS(temp_obj);

  list_obj_to_char(temp_obj->contains, ch, 2, TRUE);

  send_to_char("=============================================\n\r", ch);
  sprintf(buf, "%d Items - Total Storage Cost: %ld\n\r", counter, ((long)(total_cost_of_obj(temp_obj) * 3) / 2));
  send_to_char(buf, ch);

  extract_obj(temp_obj);

  fclose(fd);

  return TRUE;
}

int vault_put(CHAR *ch, OBJ *obj_object, OBJ *vault_object)
{
  if (CAN_SEE_OBJ(ch, obj_object) && IS_RENTABLE(obj_object))
  {
    obj_from_char(obj_object);
    obj_to_obj(obj_object, vault_object);

    return TRUE;
  }

  return FALSE;
}

void vault_give_back(CHAR *ch, OBJ *vault_obj)
{
  OBJ *temp_obj = NULL;
  OBJ *next_obj = NULL;

  for (temp_obj = vault_obj->contains; temp_obj; temp_obj = next_obj)
  {
    next_obj = temp_obj->next_content;

    obj_from_obj(temp_obj);
    obj_to_char(temp_obj, ch);
  }
}

#define VAULT_MAX 1000

int vault_store(CHAR *vault_guard, CHAR *ch, char *arg, int cmd)
{
  FILE *fd = NULL;
  OBJ *vault_obj = NULL;
  OBJ *temp_obj = NULL;
  OBJ *next_obj = NULL;
  char buf[MSL];
  char vault_name[20];
  char filename[MIL];
  char arg1[MIL];
  char arg2[MIL];
  char allbuf[MIL];
  int number = 0;
  int counter = 0;
  int total = 0;
  int num_no_rent = 0;
  int storage_price = 0;
  int alldot = FALSE;

  number_argument_interpreter(arg, &number, arg1, arg2);

  if (number < 1)
  {
    act("$n tells you 'You can't store a non-positive number of items in your vault.'", FALSE, vault_guard, 0, ch, TO_VICT);

    return TRUE;
  }

  // only a number was provided, or nothing
  if (!*arg1)
  {
    act("$n asks you 'Store what?'", FALSE, vault_guard, 0, ch, TO_VICT);

    return TRUE;
  }

  switch (ch->specials.vaultaccess)
  {
    case NO_VAULT:
      act("$n tells you 'Your currently set vault doesn't exist.'", FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
      break;

    case NO_ACCESS:
      act("$n tells you 'You don't have access to your currently set vault.'", FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
      break;

    case ACCESS_LOOK:
      act("$n tells you 'You only have look access to your currently set vault.'", FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
      break;

    case ACCESS_GET:
    case ACCESS_CONTROL:
      break;

    default:
      send_to_char("Invalid vault access. Please report this to an immortal.\n\r", ch);
      sprintf(buf, "[vault_store()] Invalid vault access. Player: %s, Vault: %s, Access: %d",
        GET_NAME(ch), ch->specials.vaultname, ch->specials.vaultaccess);
      wizlog(buf, LEVEL_ETE, WIZ_LOG_SIX);

      return TRUE;
      break;
  }

  strcpy(vault_name, ch->specials.vaultname);
  string_to_lower(vault_name);
  sprintf(filename, "vault/%s.vault", vault_name);
  fd = fopen(filename, "rb");

  if (!fd)
  {
    act("$n tells you 'Your currently set vault doesn't exist.'", FALSE, vault_guard, 0, ch, TO_VICT);

    return TRUE;
  }

  while (!feof(fd))
  {
    switch (obj_version(fd))
    {
      case 3:
        temp_obj = store_to_obj_ver3(fd, ch);
        break;

      case 2:
        temp_obj = store_to_obj_ver2(fd, ch);
        break;

      case 1:
        temp_obj = store_to_obj_ver1(fd, ch);
        break;

      case 0:
        temp_obj = store_to_obj_ver0(fd, ch);
        break;

      default:
        temp_obj = 0;
        break;
    }

    if (temp_obj)
    {
      extract_obj(temp_obj);
    }

    counter++;
  }

  fclose(fd);

  // Create the vault object.
  CREATE(vault_obj, OBJ, 1);
  clear_object(vault_obj);
  sprintf(buf, "%s's vault", CAP(vault_name));
  vault_obj->name = str_dup(buf);
  vault_obj->short_description = str_dup("storage vault");
  vault_obj->obj_flags.cost_per_day = 0;
  vault_obj->obj_flags.type_flag = ITEM_CONTAINER;

  obj_to_char(vault_obj, ch);

  alldot = is_all_dot(arg1, allbuf);

  // arg1 is 'all' or was 'all.' (and is now 'all')
  if (!str_cmp(arg1, "all"))
  {
    if (!*arg2 || str_cmp(arg2, "confirm"))
    {
      act("$n tells you 'You must type 'store all confirm' to store everything.'", FALSE, vault_guard, 0, ch, TO_VICT);

      extract_obj(vault_obj);

      return TRUE;
    }

    // 'store x all.item' or 'store x all'
    if (number != 1)
    {
      sprintf(buf, "$n tells you 'You can't store '%d all' of something.'", number);
      act(buf, FALSE, vault_guard, 0, ch, TO_VICT);

      extract_obj(vault_obj);

      return TRUE;
    }
    else
    {
      for (temp_obj = ch->carrying; temp_obj; temp_obj = next_obj)
      {
        next_obj = temp_obj->next_content;

        if (temp_obj == vault_obj) continue;
        if (alldot && !isname(allbuf, OBJ_NAME(temp_obj))) continue;

        if (!IS_RENTABLE(temp_obj) || (OBJ_TYPE(temp_obj) == ITEM_AQ_ORDER))
        {
          act("$n tells you 'You can't store $p in a vault.'", FALSE, vault_guard, temp_obj, ch, TO_VICT);

          num_no_rent++;
        }
        else
        {
          if (vault_put(ch, temp_obj, vault_obj))
          {
            total++;
          }
        }
      }

      if (!total)
      {
        if (!num_no_rent)
        {
          act("$n asks you 'Store all of what?'", FALSE, vault_guard, 0, ch, TO_VICT);

          extract_obj(vault_obj);

          return TRUE;
        }
      }
    }
  }
  else
  {
    // 'store item' or 'store 1 item' or 'store x.item'
    if (number == 1)
    {
      temp_obj = get_obj_in_list_vis(ch, arg1, ch->carrying);

      if (temp_obj)
      {
        if (!IS_RENTABLE(temp_obj))
        {
          act("$n tells you 'You can't store $p in a vault.'", FALSE, vault_guard, temp_obj, ch, TO_VICT);

          num_no_rent++;
        }
        else
        {
          if ((OBJ_TYPE(temp_obj) == ITEM_CONTAINER) && (!*arg2 || str_cmp(arg2, "confirm")))
          {
            if (strchr(arg1, '.')) {
              act("$n tells you 'You must type 'store x.<container_keyword> confirm' to store a container.'", FALSE, vault_guard, 0, ch, TO_VICT);
            }
            else {
              act("$n tells you 'You must type 'store <container_keyword> confirm' to store a container.'", FALSE, vault_guard, 0, ch, TO_VICT);
            }

            extract_obj(vault_obj);

            return TRUE;
          }

          vault_put(ch, temp_obj, vault_obj);

          total = 1;
        }
      }
      else
      {
        sprintf(buf, "$n tells you 'You don't seem to have the %s.'", arg1);
        act(buf, FALSE, vault_guard, 0, ch, TO_VICT);

        extract_obj(vault_obj);

        return TRUE;
      }
    }
    else // 'store x arg1'
    {
      alldot = TRUE; // kludgy
      strcpy(allbuf, arg1);

      for (temp_obj = ch->carrying, total = 0; temp_obj && total < number; temp_obj = next_obj)
      {
        next_obj = temp_obj->next_content;

        if (!isname(arg1, OBJ_NAME(temp_obj))) continue;

        if (!IS_RENTABLE(temp_obj))
        {
          act("$n tells you 'You can't store $p in a vault.'", FALSE, vault_guard, temp_obj, ch, TO_VICT);

          num_no_rent++;
        }
        else
        {
          if ((OBJ_TYPE(temp_obj) == ITEM_CONTAINER) && (!*arg2 || str_cmp(arg2, "confirm")))
          {
            act("$n tells you 'You must type 'store x <container_keyword> confirm' to store multiple containers.'", FALSE, vault_guard, 0, ch, TO_VICT);

            extract_obj(vault_obj);

            return TRUE;
          }

          vault_put(ch, temp_obj, vault_obj);

          total++;
        }
      }
    }
  }

  if (!total)
  {
    if (!num_no_rent)
    {
      sprintf(buf, "$n tells you 'You don't seem to have any %s.'", arg1);
      act(buf, FALSE, vault_guard, 0, ch, TO_VICT);
    }

    extract_obj(vault_obj);

    return TRUE;
  }

  counter += COUNT_RENTABLE_CONTENTS(vault_obj) - 1;

  if (counter > VAULT_MAX)
  {
    sprintf(buf, "$n tells you 'You can't store more than %d items in your vault.'", VAULT_MAX);
    act(buf, FALSE, vault_guard, 0, ch, TO_VICT);

    act("$n shoves your stuff back into your hands.", FALSE, vault_guard, 0, ch, TO_VICT);

    vault_give_back(ch, vault_obj);
    extract_obj(vault_obj);

    return TRUE;
  }

  storage_price = (total_cost_of_obj(vault_obj) * 3) / 2;

  // Prestige Perk 18
  if (GET_PRESTIGE_PERK(ch) >= 18) {
    storage_price *= 0.9;
  }

  sprintf(buf, "$n tells you 'That will cost %d coins.'", storage_price);
  act(buf, FALSE, vault_guard, 0, ch, TO_VICT);

  if (storage_price > GET_GOLD(ch))
  {
    act("$n tells you 'Which I see you can't afford.'", FALSE, vault_guard, 0, ch, TO_VICT);

    act("$n shoves your stuff back into your hands.", FALSE, vault_guard, 0, ch, TO_VICT);

    vault_give_back(ch, vault_obj);
    extract_obj(vault_obj);

    return TRUE;
  }

  fd = fopen(filename, "ab+");

  if (!fd)
  {
    send_to_char("Unable to open vault for storing. Please report this to an immortal.\n\r", ch);
    sprintf(buf, "[vault_store()] Unable to open vault for storing. Player: %s, Vault: %s, Access: %d",
      GET_NAME(ch), ch->specials.vaultname, ch->specials.vaultaccess);
    wizlog(buf, LEVEL_ETE, WIZ_LOG_SIX);

    act("$n shoves your stuff back into your hands.", FALSE, vault_guard, 0, ch, TO_VICT);

    vault_give_back(ch, vault_obj);
    extract_obj(vault_obj);

    return TRUE;
  }

  GET_GOLD(ch) -= storage_price;

  if (total == 1)
  {
    temp_obj = vault_obj->contains;

    sprintf(buf, "You store %s.\n\r", OBJ_SHORT(temp_obj));
    send_to_char(buf, ch);

    act("$n gives $p to $N.", TRUE, ch, temp_obj, vault_guard, TO_ROOM);

    obj_to_vault(temp_obj, fd, ch, -1, vault_name);
    extract_obj(temp_obj);
  }
  else
  {
    for (temp_obj = vault_obj->contains; temp_obj; temp_obj = next_obj)
    {
      next_obj = temp_obj->next_content;

      sprintf(buf, "You store %s.\n\r", OBJ_SHORT(temp_obj));
      send_to_char(buf, ch);

      obj_to_vault(temp_obj, fd, ch, -1, vault_name);
      extract_obj(temp_obj);
    }

    if (total && alldot)
    {
      if (total < 6)
      {
        sprintf(buf, "$n gives some %s(s) to $N.", allbuf);
      }
      else
      {
        sprintf(buf, "$n gives a bunch of %s(s) to $N.", allbuf);
      }

      act(buf, TRUE, ch, 0, vault_guard, TO_ROOM);
    }
    else if (total)
    {
      if (total < 6)
      {
        act("$n gives some stuff to $N.", TRUE, ch, 0, vault_guard, TO_ROOM);
      }
      else
      {
        act("$n gives a bunch of stuff to $N.", TRUE, ch, 0, vault_guard, TO_ROOM);
      }
    }
  }

  extract_obj(vault_obj);

  fclose(fd);

  save_char(ch, NOWHERE);

  string_to_lower(vault_name);
  sprintf(filename, "vault/%s.last", vault_name);
  fd = fopen(filename, "w");

  if (fd)
  {
    fprintf(fd, "%s\n", GET_NAME(ch));
  }

  fclose(fd);

  return TRUE;
}

#define VAULT_BUY_IDENTIFY 3500

int vault_recover(CHAR *vault_guard, CHAR *ch, char *arg, int cmd)
{
  FILE *fd = NULL;
  OBJ *vault_obj = NULL;
  OBJ *temp_obj = NULL;
  OBJ *next_obj = NULL;
  OBJ *prev_obj = NULL;
  char vault_name[20];
  char buf[MSL];
  char filename[MIL];
  char arg1[MIL];
  char arg2[MIL];
  char allbuf[MIL];
  int number = 0;
  int counter = 0;
  //int alldot = FALSE;
  int found = FALSE;

  if (!*arg)
  {
    if (cmd == CMD_RECOVER)
    {
      act("$n tells you 'Recover what?'", FALSE, vault_guard, 0, ch, TO_VICT);
    }
    else if (cmd == CMD_IDENTIFY)
    {
      act("$n tells you 'Identify what?'", FALSE, vault_guard, 0, ch, TO_VICT);
    }

    return TRUE;
  }

  switch (ch->specials.vaultaccess)
  {
    case NO_VAULT:
      act("$n tells you 'Your currently set vault doesn't exist.'", FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
      break;

    case NO_ACCESS:
      act("$n tells you 'You don't have access to your currently set vault.'", FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
      break;

    case ACCESS_LOOK:
      if (cmd != CMD_IDENTIFY)
      {
        act("$n tells you 'You only have look access to your currently set vault.'", FALSE, vault_guard, 0, ch, TO_VICT);

        return TRUE;
      }
      break;

    case ACCESS_GET:
    case ACCESS_CONTROL:
      break;

    default:
      send_to_char("Invalid vault access. Please report this to an immortal.\n\r", ch);
      sprintf(buf, "[vault_recover()] Invalid vault access. Player: %s, Vault: %s, Access: %d",
        GET_NAME(ch), ch->specials.vaultname, ch->specials.vaultaccess);
      wizlog(buf, LEVEL_ETE, WIZ_LOG_SIX);

      return TRUE;
      break;
  }

  strcpy(vault_name, ch->specials.vaultname);
  string_to_lower(vault_name);
  sprintf(filename, "vault/%s.vault", vault_name);
  fd = fopen(filename, "rb");

  if (!fd)
  {
    act("$n tells you 'Your currently set vault doesn't exist.'", FALSE, vault_guard, 0, ch, TO_VICT);

    return TRUE;
  }

  // Create the vault object.
  CREATE(vault_obj, OBJ, 1);
  clear_object(vault_obj);
  sprintf(buf, "%s's vault", CAP(vault_name));
  vault_obj->name = str_dup(buf);
  vault_obj->short_description = str_dup("storage vault");
  vault_obj->obj_flags.cost_per_day = 0;
  vault_obj->obj_flags.type_flag = ITEM_CONTAINER;

  while (!feof(fd))
  {
    switch (obj_version(fd))
    {
      case 3:
        obj_to_obj(store_to_obj_ver3(fd, ch), vault_obj);
        break;

      case 2:
        obj_to_obj(store_to_obj_ver2(fd, ch), vault_obj);
        break;

      case 1:
        obj_to_obj(store_to_obj_ver1(fd, ch), vault_obj);
        break;

      case 0:
        obj_to_obj(store_to_obj_ver0(fd, ch), vault_obj);
        break;
    }
  }

  fclose(fd);

  obj_to_char(vault_obj, ch);

  if (cmd == CMD_RECOVER)
  {
    sprintf(buf, "%s vault", arg);
    do_get(ch, buf, CMD_GET);

    save_char(ch, NOWHERE);
  }
  else if (cmd == CMD_IDENTIFY)
  {
    number_argument_interpreter(arg, &number, arg1, arg2);

    // arg1 is a number
    if (!*arg1)
    {
      sprintf(buf, "$n asks you 'Identify what?'");
    }
    else
    {
      //alldot = is_all_dot(arg1, allbuf);

      // arg1 is 'all' or was 'all.' (and is now 'all')
      if (!str_cmp(arg1, "all"))
      {
        sprintf(buf, "$n tells you 'You can't identify 'all' of something.'");
      }
      else
      {
        // set guard action if item isn't found
        sprintf(buf, "$n tells you 'The vault doesn't contain the %s.'", arg1);

        // 'store item' or 'store 1 item'
        if (number == 1)
        {
          temp_obj = get_obj_in_list_vis(ch, arg1, vault_obj->contains);

          if (temp_obj)
          {
            found = TRUE;
          }
        }
        else // 'identify x.arg1'
        {
          //alldot = TRUE;
          strcpy(allbuf, arg1);

          for (temp_obj = ch->carrying; temp_obj && counter <= number; temp_obj = next_obj)
          {
            next_obj = temp_obj->next_content;

            if (isname(arg1, OBJ_NAME(temp_obj)))
            {
              counter++;

              if (counter == number)
              {
                found = TRUE;

                break;
              }
            }
          }
        }
      }
    }

    if (found && temp_obj)
    {
      sprintf(buf, "$n tells you 'That will cost %d coins.'\n\r", VAULT_BUY_IDENTIFY);
      act(buf, FALSE, vault_guard, 0, ch, TO_VICT);

      if (!IS_IMPLEMENTOR(ch) && (VAULT_BUY_IDENTIFY > GET_GOLD(ch)))
      {
        act("$n tells you 'Which I see you can't afford.'", FALSE, vault_guard, 0, ch, TO_VICT);
      }
      else
      {
        if (!IS_IMPLEMENTOR(ch)) GET_GOLD(ch) -= VAULT_BUY_IDENTIFY;

        spell_identify(50, ch, 0, temp_obj);
      }
    }
    else
    {
      act(buf, FALSE, vault_guard, 0, ch, TO_VICT);
    }

    extract_obj(vault_obj);
  }

  if (cmd == CMD_RECOVER) {

    fd = fopen(filename, "wb+");

    if (!fd)
    {
      send_to_char("Unable to open vault for storing. Please report this to an immortal.\n\r", ch);
      sprintf(buf, "[vault_recover()] Unable to open vault for storing. Player: %s, Vault: %s, Access: %d",
        GET_NAME(ch), ch->specials.vaultname, ch->specials.vaultaccess);
      wizlog(buf, LEVEL_ETE, WIZ_LOG_SIX);

      return TRUE;
    }

    /* Reverse the list order to maintain vault order. - Sane */
    for (temp_obj = vault_obj->contains; temp_obj; temp_obj = next_obj)
    {
      next_obj = temp_obj->next_content;
      temp_obj->next_content = prev_obj;
      prev_obj = temp_obj;
    }

    vault_obj->contains = prev_obj;

    for (temp_obj = vault_obj->contains; temp_obj; temp_obj = next_obj)
    {
      next_obj = temp_obj->next_content;
      obj_to_store(temp_obj, fd, ch, -1, FALSE);
    }

    extract_obj(vault_obj);

    fclose(fd);
  }

  string_to_lower(vault_name);
  sprintf(filename, "vault/%s.last", vault_name);
  fd = fopen(filename, "w");

  if (fd)
  {
    fprintf(fd, "%s\n", GET_NAME(ch));
  }

  fclose(fd);

  return TRUE;
}

int vault_remove(CHAR *vault_guard, CHAR *ch, char *arg, int cmd)
{
  FILE *fd = NULL;
  FILE *tmpfd = NULL;
  char buf[MSL];
  char filename[MIL];
  char name[20];
  char vault_name[20];
  int access_level = 0;

  arg = one_argument(arg, buf);
  string_to_lower(buf);

  if (!is_abbrev(buf, "access")) return FALSE;

  if (!*arg)
  {
    act("$n asks you 'Remove access for whom?'", FALSE, vault_guard, 0, ch, TO_VICT);

    return TRUE;
  }

  arg = one_argument(arg, name);
  string_to_lower(name);

  switch (ch->specials.vaultaccess)
  {
    case NO_VAULT:
      act("$n tells you 'Your currently set vault doesn't exist.'", FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
      break;

    case NO_ACCESS:
      act("$n tells you 'You don't have access to your currently set vault.'", FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
      break;

    case ACCESS_LOOK:
    case ACCESS_GET:
      act("$n tells you 'You don't have control access to your currently set vault.'", FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
      break;

    case ACCESS_CONTROL:
      break;

    default:
      send_to_char("Invalid vault access. Please report this to an immortal.\n\r", ch);
      sprintf(buf, "[vault_remove()] Invalid vault access. Player: %s, Vault: %s, Access: %d",
        GET_NAME(ch), ch->specials.vaultname, ch->specials.vaultaccess);
      wizlog(buf, LEVEL_ETE, WIZ_LOG_SIX);

      return TRUE;
      break;
  }

  strcpy(vault_name, ch->specials.vaultname);
  string_to_lower(vault_name);
  access_level = vault_access(vault_name, name);

  if (access_level == ACCESS_CONTROL)
  {
    act("$n tells you 'You can't remove the vault owner's access.'", FALSE, vault_guard, 0, ch, TO_VICT);

    return TRUE;
  }
  else if (access_level < ACCESS_GET)
  {
    sprintf(buf, "$n tells you '%s doesn't have access.'", CAP(name));
    act(buf, FALSE, vault_guard, 0, ch, TO_VICT);

    return TRUE;
  }

  sprintf(filename, "vault/%s.name", vault_name);
  fd = fopen(filename, "r");

  if (!fd)
  {
    send_to_char("Unable to open vault name file. Please report this to an immortal.\n\r", ch);
    sprintf(buf, "[vault_remove()] Unable to open vault name file for reading. Player: %s, Vault: %s, Access: %d",
      GET_NAME(ch), ch->specials.vaultname, ch->specials.vaultaccess);
    wizlog(buf, LEVEL_ETE, WIZ_LOG_SIX);

    return TRUE;
  }

  tmpfd = fopen("vault/names.tmp", "w");

  if (!tmpfd)
  {
    send_to_char("Unable to open temporary vault name file. Please report this to an immortal.\n\r", ch);
    sprintf(buf, "[vault_remove()] Unable to open temporary vault name file for reading. Player: %s, Vault: %s, Access: %d",
      GET_NAME(ch), ch->specials.vaultname, ch->specials.vaultaccess);
    wizlog(buf, LEVEL_ETE, WIZ_LOG_SIX);

    return TRUE;
  }

  while (!feof(fd))
  {
    if (fscanf(fd, "%s\n", vault_name))
    {
      if (strcmp(name, vault_name))
      {
        fprintf(tmpfd, "%s\n", vault_name);
      }
    }
  }

  sprintf(buf, "mv vault/names.tmp %s", filename);
  system(buf);

  fclose(fd);
  fclose(tmpfd);

  sprintf(buf, "$n tells you 'Access removed from %s.'", CAP(name));
  act(buf, FALSE, vault_guard, 0, ch, TO_VICT);

  return TRUE;
}

#define VAULT_BUY_VAULT  200000
#define VAULT_BUY_ACCESS 100000

int vault_buy(CHAR *vault_guard, CHAR *ch, char *arg, int cmd)
{
  FILE *fd = NULL;
  CHAR *vict = NULL;
  char buf[MSL];
  char filename[MIL];
  char name[20];
  char vault_name[20];
  int access_level = 0;

  if (!*arg)
  {
    sprintf(buf, "\n\rCost: Vault  - %d\n\r", VAULT_BUY_VAULT);
    send_to_char(buf, ch);
    sprintf(buf, "      Access - %d\n\r", VAULT_BUY_ACCESS);
    send_to_char(buf, ch);
    send_to_char("      View   - Free\n\r\n\r", ch);
    send_to_char("For more info, please type 'help vault'.\n\r", ch);

    return TRUE;
  }

  arg = one_argument(arg, buf);
  string_to_lower(buf);

  if (is_abbrev(buf, "vault"))
  {
    sprintf(vault_name, "%s", GET_NAME(ch));
    string_to_lower(vault_name);
    sprintf(name, "%s", GET_NAME(ch));
    string_to_lower(name);
    access_level = vault_access(vault_name, name);

    if (access_level == ACCESS_CONTROL)
    {
      act("$n tells you 'You already have a vault.'", FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
    }

    sprintf(buf, "$n tells you 'That will cost %d coins.'", VAULT_BUY_VAULT);
    act(buf, FALSE, vault_guard, 0, ch, TO_VICT);

    if (VAULT_BUY_VAULT > GET_GOLD(ch))
    {
      act("$n tells you 'Which I see you can't afford.'", FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
    }

    GET_GOLD(ch) -= VAULT_BUY_VAULT;

    sprintf(filename, "vault/%s.vault", vault_name);

    if (!(fd = fopen(filename, "wb+")))
    {
      send_to_char("Unable to open vault for creating. Please report this to an immortal.\n\r", ch);
      sprintf(buf, "[vault_buy()] Unable to open vault for creating. Player: %s, Vault: %s, Access: %d",
        GET_NAME(ch), ch->specials.vaultname, ch->specials.vaultaccess);
      wizlog(buf, LEVEL_ETE, WIZ_LOG_SIX);

      return TRUE;
    }

    fclose(fd);

    sprintf(filename, "vault/%s.name", vault_name);

    if (!(fd = fopen(filename, "w")))
    {
      send_to_char("Unable to open vault name file. Please report this to an immortal.\n\r", ch);
      sprintf(buf, "[vault_buy()] Unable to open vault name file for reading. Player: %s, Vault: %s, Access: %d",
        GET_NAME(ch), ch->specials.vaultname, ch->specials.vaultaccess);
      wizlog(buf, LEVEL_ETE, WIZ_LOG_SIX);

      return TRUE;
    }

    fclose(fd);

    act("$n tells you 'Your vault is now ready.'", FALSE, vault_guard, 0, ch, TO_VICT);

    return TRUE;
  }
  else if (is_abbrev(buf, "access"))
  {
    if (!*arg)
    {
      act("$n tells you 'Buy access for whom?'", FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
    }

    arg = one_argument(arg, name);
    string_to_lower(name);

    if (!test_char(name, vault_name))
    {
      send_to_char("There is no player with that name.\n\r", ch);

      return TRUE;
    }

    switch (ch->specials.vaultaccess)
    {
      case NO_VAULT:
        act("$n tells you 'Your currently set vault doesn't exist.'", FALSE, vault_guard, 0, ch, TO_VICT);

        return TRUE;
        break;

      case NO_ACCESS:
      case ACCESS_LOOK:
      case ACCESS_GET:
        act("$n tells you 'You don't have control access to your currently set vault.'", FALSE, vault_guard, 0, ch, TO_VICT);

        return TRUE;
        break;

      case ACCESS_CONTROL:
        break;

      default:
        send_to_char("Invalid vault access. Please report this to an immortal.\n\r", ch);
        sprintf(buf, "[vault_buy()] Invalid vault access. Player: %s, Vault: %s, Access: %d",
        GET_NAME(ch), ch->specials.vaultname, ch->specials.vaultaccess);
        wizlog(buf, LEVEL_ETE, WIZ_LOG_SIX);

        return TRUE;
        break;
    }

    strcpy(vault_name, ch->specials.vaultname);
    string_to_lower(vault_name);
    access_level = vault_access(vault_name, name);

    if (access_level >= ACCESS_GET)
    {
      sprintf(buf, "$n tells you '%s already has access.'", CAP(name));
      act(buf, FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
    }

    sprintf(buf, "$n tells you 'That will cost %d coins.'", VAULT_BUY_ACCESS);
    act(buf, FALSE, vault_guard, 0, ch, TO_VICT);

    if (VAULT_BUY_ACCESS > GET_GOLD(ch))
    {
      act("$n tells you 'Which I see you can't afford.'", FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
    }

    GET_GOLD(ch) -= VAULT_BUY_ACCESS;

    sprintf(filename, "vault/%s.name", vault_name);

    if (!(fd = fopen(filename, "a")))
    {
      send_to_char("Unable to open vault name file. Please report this to an immortal.\n\r", ch);
      sprintf(buf, "[vault_buy()] Unable to open vault name file for reading. Player: %s, Vault: %s, Access: %d",
        GET_NAME(ch), ch->specials.vaultname, ch->specials.vaultaccess);
      wizlog(buf, LEVEL_ETE, WIZ_LOG_SIX);

      return TRUE;
    }

    fprintf(fd, "%s\n", name);

    fclose(fd);

    sprintf(buf, "$n tells you 'Access given to %s.'", CAP(name));
    act(buf, FALSE, vault_guard, 0, ch, TO_VICT);

    return TRUE;
  }
  else if (is_abbrev(buf, "view"))
  {
    if (!*arg)
    {
      act("$n tells you 'View access for whom?'", FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
    }

    arg = one_argument(arg, name);
    string_to_lower(name);

    switch (ch->specials.vaultaccess)
    {
      case NO_VAULT:
        act("$n tells you 'Your currently set vault doesn't exist.'", FALSE, vault_guard, 0, ch, TO_VICT);

        return TRUE;
        break;

      case NO_ACCESS:
        act("$n tells you 'You don't have control access to your currently set vault.'", FALSE, vault_guard, 0, ch, TO_VICT);

        return TRUE;
        break;

      case ACCESS_LOOK:
      case ACCESS_GET:
      case ACCESS_CONTROL:
          break;

      default:
        send_to_char("Invalid vault access. Please report this to an immortal.\n\r", ch);
        sprintf(buf, "[vault_buy()] Invalid vault access. Player: %s, Vault: %s, Access: %d",
          GET_NAME(ch), ch->specials.vaultname, ch->specials.vaultaccess);
        wizlog(buf, LEVEL_ETE, WIZ_LOG_SIX);

        return TRUE;
        break;
    }

    strcpy(vault_name, ch->specials.vaultname);
    string_to_lower(vault_name);
    access_level = vault_access(vault_name, name);

    if (access_level >= ACCESS_GET)
    {
      sprintf(buf, "$n tells you '%s already has view access.'", CAP(name));
      act(buf, FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
    }

    if (!(vict = get_char_room_vis(ch, name)))
    {
      sprintf(buf, "$n tells you '%s isn't here.'", CAP(name));
      act(buf, FALSE, vault_guard, 0, ch, TO_VICT);

      return TRUE;
    }

    vict->specials.vaultaccess = ACCESS_LOOK;
    strcpy(vict->specials.vaultname, vault_name);

    sprintf(buf, "$n tells you 'View access given to %s.'", CAP(name));
    act(buf, FALSE, vault_guard, 0, ch, TO_VICT);

    sprintf(buf, "$n tells you 'View access given to %s's vault.'", CAP(vault_name));
    act(buf, FALSE, vault_guard, 0, vict, TO_VICT);

    return TRUE;
  }

  return FALSE;
}


/* This is the master vault spec function. It has been split out into multiple sub-functions for simplicity; previously it
   was one giant switch () statement, and was horrible. Storing multiple items is now a feature. Identify has been added as
   a feature. The vast majority of this code has been re-written and (hopefully) optimized.
   Re-written by Night, 11/09/2011 */
int do_vault(CHAR *vault_guard, CHAR *ch, int cmd, char *arg)
{
  if (!ch)
  {
    return FALSE;
  }

  if (cmd != CMD_USE &&
      cmd != CMD_LIST &&
      cmd != CMD_STORE &&
      cmd != CMD_IDENTIFY &&
      cmd != CMD_RECOVER &&
      cmd != CMD_BUY &&
      cmd != CMD_REMOVE &&
      cmd != CMD_OFFER &&
      cmd != CMD_GIVE)
  {
    return FALSE;
  }

  if (IS_NPC(ch))
  {
    send_to_char("NPCs don't have access to vaults.\n\r", ch);

    return FALSE;
  }

  if (CHAOSMODE)
  {
    act("$n tells you 'I take a break during Chaos, go away!'", 0, vault_guard, 0, ch, TO_VICT);

    return TRUE;
  }

  switch (cmd)
  {
    case CMD_USE:
      return vault_use(vault_guard, ch, arg, cmd);
      break;

    case CMD_LIST:
      return vault_list(vault_guard, ch, arg, cmd);
      break;

    case CMD_STORE:
      return vault_store(vault_guard, ch, arg, cmd);
      break;

    case CMD_RECOVER:
      return vault_recover(vault_guard, ch, arg, cmd);
      break;

    case CMD_IDENTIFY:
      /* Identifying something is the same as recovering it, identifying it, then storing it again (but without storage costs). */
      return vault_recover(vault_guard, ch, arg, cmd);
      break;

    case CMD_BUY:
      return vault_buy(vault_guard, ch, arg, cmd);
      break;

    case CMD_REMOVE:
      return vault_remove(vault_guard, ch, arg, cmd);
      break;

    case CMD_OFFER:
      return vault_offer(vault_guard, ch, arg, cmd);
      break;

    case CMD_GIVE:
      return vault_give(vault_guard, ch, arg, cmd);
      break;

    default:
      return FALSE;
      break;
  }

  return FALSE;
}

#define BRONZE_BAR   3013
#define SILVER_BAR   3014
#define GOLD_BAR     3015
#define PLATINUM_BAR 3016
#define MITHRIL_BAR  3017
#define DIAMOND      3018
#define PALLADIUM    3146
#define ADAMANTIUM   3145
#define ZYCA_SILVER  10923
#define ZYCA_BRONZE  10922
#define ABYSS_BAR    25034
#define DI_OPAL      27700
#define DI_CRYSTAL   27701

int jeweler(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  extern OBJ *read_object(int num, int type);

  OBJ *obj = NULL;
  char buf[MSL];
  int cost = 0;

  switch (cmd) {
  case CMD_LIST:
    send_to_char("\
You can buy:\n\r\
---------------------------------------\n\r\
A Bronze Bar      :        50,000 coins\n\r\
A Silver Bar      :       100,000 coins\n\r\
A Gold Bar        :       200,000 coins\n\r\
A Platinum Bar    :       500,000 coins\n\r\
A Mithril Bar     :     1,000,000 coins\n\r\
A Large Diamond   :     5,000,000 coins\n\r\
A Palladium Bar   :   100,000,000 coins\n\r\
An Adamantium Bar : 1,000,000,000 coins\n\r", ch);

    return TRUE;
    break;

  case CMD_SELL:
    if (!*arg) {
      send_to_char("The Jeweler tells you 'Sell what?'\n\r", ch);

      return TRUE;
    }

    one_argument(arg, buf);
    string_to_lower(buf);

    obj = get_obj_in_list_vis(ch, buf, ch->carrying);

    if (!obj) {
      send_to_char("The Jeweler tells you 'Sell what?'\n\r", ch);

      return TRUE;
    }

    switch (V_OBJ(obj)) {
    case BRONZE_BAR:
    case SILVER_BAR:
    case GOLD_BAR:
    case PLATINUM_BAR:
    case MITHRIL_BAR:
    case DIAMOND:
    case PALLADIUM:
    case ADAMANTIUM:
    case ZYCA_SILVER:
    case ZYCA_BRONZE:
    case ABYSS_BAR:
    case DI_OPAL:
    case DI_CRYSTAL:
      break;

    default:
      send_to_char("The Jeweler tells you 'I only buy trade bars, diamonds, and a few other treasures.'\n\r", ch);

      return TRUE;
    }

    cost = OBJ_COST(obj);

    if ((INT_MAX - GET_GOLD(ch)) < cost) {
      send_to_char("You can't carry any more coins.\n\r", ch);

      return TRUE;
    }

    act("You give $p to $N.", FALSE, ch, obj, mob, TO_CHAR);
    act("$N gives you $P.", FALSE, ch, obj, mob, TO_VICT);
    act("$n gives $p to $N.", FALSE, ch, obj, mob, TO_ROOM);

    printf_to_char(ch, "The Jeweler gives you %d coins.\n\r", cost);

    GET_GOLD(ch) += cost;

    obj_from_char(obj);
    extract_obj(obj);

    save_char(ch, NOWHERE);

    return TRUE;
    break;

  case CMD_BUY:
    arg = one_argument(arg, buf);

    if (!*buf) {
      send_to_char("The Jeweler tells you 'Buy what?'\n\r", ch);

      return TRUE;
    }

    if (!strcmp(buf, "bronze"))
      obj = read_object(BRONZE_BAR, VIRTUAL);
    else if (!strcmp(buf, "silver"))
      obj = read_object(SILVER_BAR, VIRTUAL);
    else if (!strcmp(buf, "gold"))
      obj = read_object(GOLD_BAR, VIRTUAL);
    else if (!strcmp(buf, "platinum"))
      obj = read_object(PLATINUM_BAR, VIRTUAL);
    else if (!strcmp(buf, "mithril"))
      obj = read_object(MITHRIL_BAR, VIRTUAL);
    else if (!strcmp(buf, "diamond"))
      obj = read_object(DIAMOND, VIRTUAL);
    else if (!strcmp(buf, "palladium"))
      obj = read_object(PALLADIUM, VIRTUAL);
    else if (!strcmp(buf, "adamantium"))
      obj = read_object(ADAMANTIUM, VIRTUAL);
    else {
      send_to_char("The Jeweler tells you 'I don't sell that.'\n\r", ch);

      return TRUE;
    }

    if (!obj) {
      wizlog_f(LEVEL_ETE, 6, "[jeweler()] Unable to load object '%s'.", buf);

      return TRUE;
    }

    cost = obj->obj_flags.cost;

    if (GET_GOLD(ch) < cost) {
      send_to_char("The Jeweler tells you 'You don't have enough money.'\n\r", ch);

      return TRUE;
    }

    printf_to_char(ch, "The Jeweler tells you 'That will cost you %d coins.'\n\r", cost);
    act("$n gives $p to $N.", TRUE, mob, obj, ch, TO_NOTVICT);
    act("$n gives $p to you.", TRUE, mob, obj, ch, TO_VICT);

    obj_to_char(obj, ch);

    GET_GOLD(ch) -= cost;

    save_char(ch, NOWHERE);

    return TRUE;
    break;
  }

  return FALSE;
}

int total_cost_of_package(struct obj_data *obj) {
  int cost=0;
  struct obj_data *tmp;
  if(obj) {
    for(tmp = obj->contains;tmp;tmp=tmp->next_content) {
      if((ITEM_SC_TOKEN==tmp->obj_flags.type_flag || IS_RENTABLE(tmp)) && cost !=-1) {
        if(total_cost_of_package(tmp)!=-1 && cost!=-1)
          cost += total_cost_of_package(tmp);
        else
          cost=-1;
      }
      else
        cost=-1;
    }
    if((ITEM_SC_TOKEN==obj->obj_flags.type_flag || IS_RENTABLE(obj)) && cost !=-1)
      cost += MAX(0,obj->obj_flags.cost_per_day)/2;
    else
      cost=-1;
  }
  return cost;
}

int postoffice(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  struct char_file_u_5 char_data_5;
  struct char_file_u_4 char_data_4;
  CHAR *vict=0;
  struct tm *timeStruct;
  extern char *fread_string(FILE *fd);
  extern OBJ *read_object(int num, int type);
  OBJ *obj = NULL, *tmp = NULL;
  char *message,*p;
  char name[MIL],sender[MAX_STRING_LENGTH],filename[MAX_STRING_LENGTH];
  char test[MAX_STRING_LENGTH],email_addr[80],buf[MSL * 2],reply_addr[80];
  FILE *fd,*fl;
  int i,j,vnum,cost,send_email,version;
  long ct;


  if (IS_NPC(ch))
    return(FALSE);

switch (cmd) {

 case CMD_MAIL:
  if(!*arg){
    for(i=0;ch->player.name[i];++i){
      name[i]=ch->player.name[i];
      if(isupper(name[i])) name[i]=tolower(name[i]);
    }
    name[i]=0;
    sprintf(filename,"mail/%s",name);
    fd=fopen(filename,"r");
    if(fd==NULL){
      send_to_char("You haven't got any mail.\n\r",ch);
      return(TRUE);
    }
    while(fgets(sender,32,fd)){
      for(i=0;isalpha(sender[i]);++i);
      sender[i]=0;
      message=fread_string(fd);
      CREATE(p,char,strlen(sender)+strlen(message)+16);
      sprintf(p,"From: %s\n\r\n\r%s",sender,message);
      free(message);
      obj=read_object(2,REAL);
      if(!obj){
         log_s("Couldn't load a postcard.");
         fclose(fd);
         return(TRUE);
      }
      obj->action_description = p;
      obj_to_char(obj,ch);
       act("$N gives a postcard to $n.",FALSE,ch,0,mob, TO_ROOM);
       act("$N gives you a postcard.",FALSE,ch,0,mob,TO_CHAR);
       act("You give a postcard to $n.",FALSE,ch,0,mob, TO_VICT);
    }
    fclose(fd);
    unlink(filename);
  } else {
    obj=get_obj_in_list_vis(ch,"postcard",ch->carrying);
    if(!obj){
       act("$N tells you 'You don't have a postcard.'",FALSE,ch,0,mob,TO_CHAR);
       return(TRUE);
    }
    while(*arg) {
     arg=one_argument(arg,name);
     string_to_lower(name);
     if(!strcmp(name,"postcard") || !strcmp(name,"package")) {
      send_to_char("To mail a postcard type: mail <name> <name2> etc\n\r",ch);
      return TRUE;
     }
     if(!test_char(name,test)) {
       sprintf(test,"There is no player named %s.\n\r",CAP(name));
       send_to_char(test,ch);
       return(TRUE);
     }
     ct=time(0);
     timeStruct = localtime(&ct);
     sprintf(filename,"mail/%s", name);
     fd=fopen(filename,"a");
     if(fd==NULL){
       send_to_char("Hmm...\n\r",ch);
       return(TRUE);
     }
     fprintf(fd,"%s\n\r",GET_NAME(ch));
     fprintf(fd,"Date: %s %2d\n\r\n\r", Month[timeStruct->tm_mon], timeStruct->tm_mday);
     fprintf(fd,"%s~\n",obj->action_description);
     fclose(fd);
     sprintf(test,"$N tells you 'Postcard to %s on its way.'",CAP(name));
     act(test,FALSE,ch,0,mob,TO_CHAR);

     send_email=0;
     string_to_lower(name);
     vict=get_ch_by_name(name);
     if(vict) {
       if(IS_SET(vict->specials.pflag, PLR_EMAIL)) {
         send_email=1;
         sprintf(email_addr,"%s",GET_EMAIL(vict));
       }
     }
     else {
       sprintf(buf,"rent/%c/%s.dat",UPPER(name[0]),name);
       if(!(fl = fopen(buf, "rb")))
         return TRUE;

       version=char_version(fl);
       switch(version) {
         case 2:
         case 3:
           return TRUE;
           break;
         case 4:
           if((fread(&char_data_4,sizeof(struct char_file_u_4),1,fl))!=1) {
             log_s("Error Reading rent file(postoffice)");
             fclose(fl);
             return TRUE;
           }
           break;
         case 5:
           if((fread(&char_data_5,sizeof(struct char_file_u_5),1,fl))!=1) {
             log_s("Error Reading rent file(postoffice)");
             fclose(fl);
             return TRUE;
           }
           break;
         default:
           log_s("Error getting pfile version (postoffice)");
           return TRUE;
       }
       CREATE(vict, CHAR, 1);
       clear_char(vict);
       vict->desc = 0;
       CREATE(vict->skills, struct char_skill_data, MAX_SKILLS5);
       clear_skills(vict->skills);
       reset_char(vict);

       switch (version) {
         case 4:
           store_to_char_4(&char_data_4,vict);
           break;
         case 5:
           store_to_char_5(&char_data_5,vict);
           break;
         default:
           log_s("Version number corrupted? (postcard to email copy)");
           return TRUE;
       }
       fclose(fl);
       vict->next = character_list;
       character_list = vict;
       char_to_room(vict, CHAR_REAL_ROOM(ch));

       if(IS_SET(vict->specials.pflag, PLR_EMAIL)) {
         send_email=1;
         sprintf(email_addr,"%s",GET_EMAIL(vict));
       }
       extract_char(vict);
     }

     if(send_email) {
       sprintf(filename,"mail/temp_email.txt");
       fd=fopen(filename,"w");
       if(fd==NULL){
         send_to_char("Hmm...\n\r",ch);
         return(TRUE);
       }
       fprintf(fd,"%s~\n",obj->action_description);
       fclose(fd);
       sprintf(reply_addr,"%s",GET_EMAIL(ch));
       if(!reply_addr[0]) strncpy(reply_addr,"Not.defined",80);
/*
       sprintf(buf,"mail -s 'Postcard from %s' -r '%s' %s < %s",GET_NAME(ch),reply_addr,email_addr,filename);
*/
       sprintf(buf,"mail -s 'Postcard from %s' %s < %s",GET_NAME(ch),email_addr,filename);
       system(buf);
     }

    }
    obj_from_char(obj);
    extract_obj(obj);
    act("$n gives a postcard to $N.",FALSE,ch,0,mob, TO_ROOM);
    act("$N gives you a postcard.",FALSE,ch,0,mob,TO_VICT);
    act("You give a postcard to $N.",FALSE,ch,0,mob, TO_CHAR);
  }
  return(TRUE);
  break;

  case CMD_POST:
    if(!*arg){
    for(i=0;ch->player.name[i];++i){
      name[i]=ch->player.name[i];
      if(isupper(name[i])) name[i]=tolower(name[i]);
    }
    name[i]=0;
    sprintf(filename,"post/%s",name);
    fd=fopen(filename,"r");
    if(fd==NULL){
      send_to_char("You haven't got any packages.\n\r",ch);
      return(TRUE);
    }

    while(!feof(fd)) {
      switch(obj_version(fd)) {
        case 3:
          obj_to_char(store_to_obj_ver3(fd,ch),ch);
          break;
        case 2:
          obj_to_char(store_to_obj_ver2(fd,ch),ch);
          break;
        case 1:
          obj_to_char(store_to_obj_ver1(fd,ch),ch);
          break;
        case 0:
          obj_to_char(store_to_obj_ver0(fd,ch),ch);
          break;
      }

      if(feof(fd)) break;
      act("$N gives a small package to $n.",FALSE,ch,0,mob, TO_ROOM);
      act("$N gives you a small package.",FALSE,ch,0,mob,TO_CHAR);
      act("You give a small package to $n.",FALSE,ch,0,mob, TO_VICT);
      if(GET_LEVEL(ch)>=LEVEL_IMM) {
        sprintf(test, "WIZINFO: %s gets a package.", GET_NAME(ch));
        wizlog(test, GET_LEVEL(ch)+1, 5);
        log_s(test);
      }
    }
    fclose(fd);
    unlink(filename);
    } else {
    obj=get_obj_in_list_vis(ch,"package",ch->carrying);
    if(!obj){
       act("$N tells you 'You don't have a package.'",FALSE,ch,0,mob,TO_CHAR);
       return(TRUE);
    }
    one_argument(arg,name);
    string_to_lower(name);
    if(!strcmp(name,"postcard") || !strcmp(name,"package")) {
      send_to_char("To mail a package type: post <name>\n\r",ch);
      return TRUE;
    }
    if(!test_char(name,test)){
       send_to_char("There is no player with that name.\n\r",ch);
       return(TRUE);
    }
    cost=total_cost_of_package(obj)*3/2;
    if(cost<0) {
      act("$N tells you 'That package contains an item that can't be posted.'",FALSE,ch,0,mob,TO_CHAR);
      return(TRUE);
    }
    sprintf(test,"$N tells you 'That will cost %d coins.'",cost);
    act(test,FALSE,ch,0,mob,TO_CHAR);
    if (cost>GET_GOLD(ch)) {
     act("$N tells you 'Which I see you can't afford.'",FALSE,ch,0,mob,TO_CHAR);
     return(TRUE);
    }
    GET_GOLD(ch)-=cost;
    sprintf(filename,"post/%s",name);
    fd=fopen(filename,"ab+");
    if(fd==NULL){
      send_to_char("NULL filename\n\r",ch);
      return(TRUE);
    }

    obj_to_store(obj,fd,ch,-1,TRUE);
    fclose(fd);

    ct=time(0);
    timeStruct = localtime(&ct);
    sprintf(filename,"mail/%s", name);
    fd=fopen(filename,"a");
    if(fd==NULL){
      send_to_char("Hmm...\n\r",ch);
      return(TRUE);
    }
    fprintf(fd,"%s\n\r",GET_NAME(ch));
    fprintf(fd,"Date: %s %2d\n\r\n\r", Month[timeStruct->tm_mon], timeStruct->tm_mday);
    fprintf(fd,"Package Sent: Contents are...\n\r");
    for (tmp=obj->contains;tmp;tmp = tmp->next_content)
      fprintf(fd,"%s\n\r",OBJ_SHORT(tmp));
    fprintf(fd,"~\n");
    fclose(fd);

    extract_obj(obj);
     act("$n gives a small package to $N.",FALSE,ch,0,mob, TO_ROOM);
     act("$N gives you a small package.",FALSE,ch,0,mob,TO_VICT);
     act("You give a small package to $N.",FALSE,ch,0,mob, TO_CHAR);
     sprintf(test,"$N tells you 'Package to %s on its way'",CAP(name));
     act(test,FALSE,ch,0,mob,TO_CHAR);
     if(GET_LEVEL(ch)>=LEVEL_IMM) {
       sprintf(test, "WIZINFO: %s sent a package to %s", GET_NAME(ch), CAP(name));
       wizlog(test, GET_LEVEL(ch)+1, 5);
       log_s(test);
     }
    }
    return(TRUE);
    break;

  case CMD_LIST:
    send_to_char("\
Use buy <#> <item> to buy the following:\n\r\
----------------------------------------\n\r\
A Pen           :             75 coins\n\r\
A Postcard      :            450 coins\n\r\
A Package       :            750 coins\n\r\n\r",ch);
    return TRUE;
    break;

  case CMD_BUY:
    arg = one_argument(arg, test);
    if (!*test) {
      send_to_char("Buy what?\n\r",ch);
      return(TRUE);
    }

    if(is_number(test)) {
      i=atoi(test);
      if(i<1) {
        send_to_char("The number of items you would like to buy must be positive.\n\r",ch);
        return TRUE;
      }
      arg=one_argument(arg,test);
      if (!*test) {
        send_to_char("Buy what?\n\r",ch);
        return(TRUE);
      }
    }
    else i=1;

    if (!strcmp(test,"postcard")) {
       vnum=3;
       cost=i*450;
    }
    else if (!strcmp(test,"pen")) {
       vnum=2996;
       cost=i*75;
    }
    else if (!strcmp(test,"package")) {
       vnum=4;
       cost=i*750;
    }
    else {
     send_to_char("You can't buy that. Try list.\n\r",ch);
     return(TRUE);
    }

    if(!IS_NPC(ch) && GET_LEVEL(ch)>=LEVEL_IMM) cost=0;

    if((IS_CARRYING_N(ch) + i > CAN_CARRY_N(ch))) {
      send_to_char("You can't carry that many items.\n\r",ch);
      return TRUE;
    }

    if(GET_GOLD(ch)<cost) {
      act("$N tells you 'You don't have enough money.'",FALSE,ch,0,mob,TO_CHAR);
      return(TRUE);
    }
    GET_GOLD(ch)-=cost;

    sprintf(test,"$N tells you 'That will cost you %d coins.'\n\r",cost);
    act(test,FALSE,ch,0,mob,TO_CHAR);
    for(j=0;j<i;j++) {
      obj=read_object(vnum,VIRTUAL);
      obj_to_char(obj,ch);
    }
    if(i>1)
      sprintf(test,"$n gives (%d) $p to $N.",i);
    else
      sprintf(test,"$n gives $p to $N.");
    act(test,1,mob,obj,ch,TO_NOTVICT);
    if(i>1)
      sprintf(test,"$n gives (%d) $p to you.",i);
    else
      sprintf(test,"$n gives $p to you.");
    act(test,1,mob,obj,ch,TO_VICT);
    return TRUE;
    break;
  }

  return(FALSE);
}
/*Promotion Routines */

void promote_mage(CHAR *promoter, CHAR *ch)
{
  char buf[1000],title[50];
  ENCH ench;
  int exp=0,gold=0;

  if (enchanted_by_type(ch, ENCHANT_SORCERER)) {
    exp=0;
  }
  else if (enchanted_by_type(ch, ENCHANT_WARLOCK)) {
    sprintf(title,"Sorcerer");
    exp=15000000;
    gold=10000000;
  }
  else if (enchanted_by_type(ch, ENCHANT_APPRENTICE)) {
    sprintf(title,"Warlock");
    exp=10000000;
    gold=5000000;
  }
  else {
    sprintf(title,"Apprentice");
    exp=5000000;
    gold=1000000;
  }

  // Prestige Perk 4
  if (GET_PRESTIGE_PERK(ch) >= 4) {
    exp *= 0.9;
    gold *= 0.9;
  }

  act("\
$n whirls around and changes into a wizened mage surrounded by\n\r\
glowing pixies and imps.",FALSE,promoter, 0, ch, TO_ROOM);

  if(!exp) {
    act("\
The mage says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_ROOM);
    act("\
The mage says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_CHAR);
    act("\
In a bright flash of light and smoke, the mage disappears.\n\r\
When the smoke clears, $n is left standing there.",FALSE,promoter,0,ch,TO_ROOM);
    return;
  }

  if(GET_EXP(ch) >= exp)
  {
    act("\
The mage peers deep into your soul, and decides that you are worthy to\n\r\
join the ranks of the magus.",FALSE, promoter, 0, ch, TO_VICT);
    act("\
The mage peers deep into $N's soul, and decides that $E is worthy to\n\r\
join the ranks of the magus.",FALSE, promoter, 0, ch, TO_NOTVICT);
    if(GET_GOLD(ch) >= gold)
      {
        act("\
The mage utters a strange word to his familiars, and they swoop around you,\n\rdigging through your pockets.  They leave you with a warm aura, and\n\r\
considerably fewer coins.",FALSE, promoter, 0, ch, TO_VICT);
        act("\
The mage utters a strange word to his familiars, and they swoop around $N,\n\r\
digging through $S pockets. They leave $M with a warm aura, and\n\r\
considerably fewer coins.",FALSE, promoter, 0, ch, TO_NOTVICT);
        send_to_room("The wizened mage raises his head and shouts in an uneathly voice.\n\r",CHAR_REAL_ROOM(promoter));
        sprintf(buf,"The mage shouts 'The order of the magus has accepted %s as an %s!'\n\r",PERS(ch,promoter),title);
      send_to_world(buf);
      sprintf(buf,"The title of %s",title);
      ench.name = buf;
      enchantment_to_char(ch,&ench,TRUE);
      GET_GOLD(ch) -= gold;
      GET_EXP(ch)  -= exp;
      }
    else
      {
        act("\
The mage utters a strange word to his familiars and they swoop around you,\n\r\
digging through your pockets. They leave disgusted because you havent\n\r\
enough pretties.",FALSE, promoter, 0, ch, TO_VICT);
        act("\
The mage utters a strange word to his familiars and they swoop around $N,\n\r\
digging through $S pockets.  With a scowl on their tiny faces, they \n\r\
fly back to the mage, disappointed at the lack of shiny coins.",FALSE, promoter, 0, ch, TO_NOTVICT);
      }
  }
      else
  {
    act("\
The mage peers deep into your soul, and decides that you yet unworthy to\n\r\
join the ranks of the magus.",FALSE, promoter, 0, ch, TO_VICT);
    act("\
The mage peers deep into $N's soul, and decides that $E is unworthy to\n\r\
join the ranks of the magus.",FALSE, promoter, 0, ch, TO_NOTVICT);
  }
      act("\
In a bright flash of light and smoke, the mage disappears.\n\r\
When the smoke clears, $n is left standing there.",FALSE,promoter,0,ch,TO_ROOM);

}

void promote_cleric(CHAR *promoter, CHAR *ch)
{
  char buf[1000],title[50];
  ENCH ench;
  int exp=0,gold=0;

  if (enchanted_by_type(ch, ENCHANT_PROPHET)) {
    exp=0;
  }
  else if (enchanted_by_type(ch, ENCHANT_BISHOP)) {
    sprintf(title,"Prophet");
    exp=15000000;
    gold=10000000;
  }
  else if(enchanted_by_type(ch, ENCHANT_ACOLYTE)) {
    sprintf(title,"Bishop");
    exp=10000000;
    gold=5000000;
  }
  else {
    sprintf(title,"Acolyte");
    exp=5000000;
    gold=1000000;
  }

  // Prestige Perk 4
  if (GET_PRESTIGE_PERK(ch) >= 4) {
    exp *= 0.9;
    gold *= 0.9;
  }

  act("\
A bright light shines down from the heavens and basks\n\r\
$n in its unearthly glow.  When your vision\n\r\
clears, a prophet stands before you.",FALSE,promoter, 0, ch, TO_ROOM);

  if(!exp) {
    act("\
The prophet says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_ROOM);
    act("\
The prophet says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_CHAR);
    act("\
With a clap of thunder, lightning strikes and\n\r\
you are blinded.  When your vision clears, \n\r\
$n is left standing there.",FALSE, promoter, 0, ch, TO_ROOM);
    return;
  }

  if(GET_EXP(ch) >= exp)
  {
    act("\
The prophet weighs your deeds, and decides\n\r\
that you are worthy to progress in the clergy.",FALSE, promoter, 0, ch, TO_VICT);
    act("\
The prophet weighs $N's deeds, and decides\n\r\
that $E is worthy to progress in the clergy.",FALSE, promoter, 0, ch, TO_NOTVICT);
    if(GET_GOLD(ch) >= gold)
      {
        act("\
The prophet passes you the collection plate.\n\r\
Coins stream from your from your pockets as you\n\r\
make a considerable donation.",FALSE, promoter, 0, ch, TO_VICT);
        act("\
The prophet passes $N the collection plate.\n\r\
Coins stream from $S pockets as $E\n\r\
makes a hefty donation.",FALSE, promoter, 0, ch, TO_NOTVICT);
        send_to_room("\
The prophet offers his benedictions.\n\r",CHAR_REAL_ROOM(promoter));
        sprintf(buf,"\
The clouds peal back, and trumpets blare. \n\r\
A voice from the clouds booms out '%s\n\r\
has been accepted as a %s!'\n\r",PERS(ch,promoter),title);
      send_to_world(buf);
      sprintf(buf,"The title of %s",title);
      ench.name = buf;
      enchantment_to_char(ch,&ench,TRUE);
      GET_GOLD(ch) -= gold;
      GET_EXP(ch)  -= exp;
      }
    else
      {
        act("\
The prophet passes you the collection plate,\n\r\
but you haven't enough to fill it.",FALSE, promoter, 0, ch, TO_VICT);
        act("\
The prophet passes $N the collection plate,\n\r\
but $E hasn't enough coins to offer.",FALSE, promoter, 0, ch, TO_NOTVICT);
      }
  }
      else
  {
    act("\
The prophet weighs your deeds and decides against\n\r\
admitting you to the order.",FALSE, promoter, 0, ch, TO_VICT);
    act("\
The prophet weighs $N's deeds, and decides against\n\r\
admitting $M to the order.",FALSE, promoter, 0, ch, TO_NOTVICT);
  }
      act("\
With a clap of thunder, lightning strikes and\n\r\
you are blinded.  When your vision clears, \n\r\
$n is left standing there.",FALSE, promoter, 0, ch, TO_ROOM);

}

void promote_ninja(CHAR *promoter, CHAR *ch)
{
  char buf[1000],title[50];
  ENCH ench;
  int exp=0,gold=0;

  if (enchanted_by_type(ch, ENCHANT_SHOGUN)) {
    exp=0;
  }
  else if (enchanted_by_type(ch, ENCHANT_SHINOBI)) {
    sprintf(title,"Shogun");
    exp=15000000;
    gold=10000000;
  }
  else if (enchanted_by_type(ch, ENCHANT_TSUME)) {
    sprintf(title,"Shinobi");
    exp=10000000;
    gold=5000000;
  }
  else {
    sprintf(title,"Tsume");
    exp=5000000;
    gold=1000000;
  }

  // Prestige Perk 4
  if (GET_PRESTIGE_PERK(ch) >= 4) {
    exp *= 0.9;
    gold *= 0.9;
  }

  act("\
A shadow covers your eyes, where once $n stood,\n\r\
the shadow ninja now stands.\n\r",FALSE,promoter, 0, ch, TO_ROOM);
  if(!exp) {
    act("\
The shadow ninja says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_ROOM);
    act("\
The shadow ninja says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_CHAR);
    act("\
The shadow ninja throws down a small rock,\n\r\
and a cloud of smoke billows up to engulf him.\n\r\
When the smoke clears, $n is left standing there.\n\r",FALSE, promoter, 0, ch, TO_ROOM);
    return;
  }

  if(GET_EXP(ch) >= exp) {
    act("\
The shadow ninja attacks you suddenly,\n\r\
but you avoid his attack and\n\r\
drive a kick into his solar plexus.\n\r\
The shadow ninja looks surprised and you sense his approval.\n\r",FALSE, promoter, 0, ch, TO_VICT);
    act("\
The shadow ninja attacks $N suddenly,\n\r\
but $N avoids his attack and\n\r\
kicks him in the solar plexus.\n\r\
The shadow ninja looks surprised and looks to approve $M.\n\r",FALSE, promoter, 0, ch, TO_NOTVICT);

    if(GET_GOLD(ch) >= gold) {
      act("\
The shadow ninja suddenly starts counting a\n\r\
large sum of coins. Your ninja senses\n\r\
tell you they were yours moments before.\n\r",FALSE, promoter, 0, ch, TO_VICT);
      act("\
The shadow ninja suddenly starts counting a\n\r\
large sum of coins. $N realises that moments\n\r\
before those coins were in $S pockets.\n\r",FALSE,promoter, 0, ch, TO_NOTVICT);
      send_to_room("\
The shadow ninja slinks off into the shadows to\n\r\
inform the rest of the brotherhood.\n\r\n\r",CHAR_REAL_ROOM(promoter));

      sprintf(buf,"\
In the back of your mind, you hear the shadow ninja whisper\n\r\
'%s has been accepted into the brotherhood as a %s.'\n\r\
When you turn around, he is no longer there.\n\r\n\r",PERS(ch,promoter),title);
      send_to_world(buf);
      sprintf(buf,"The title of %s",title);
      ench.name = buf;
      enchantment_to_char(ch,&ench,TRUE);
      GET_GOLD(ch) -= gold;
      GET_EXP(ch)  -= exp;
    }
    else {
      act("\
The shadow ninja tosses your measly 'fortune' back\n\r\
to you, disappointed at the amount of coins you could\n\r\
'offer' to the brotherhood.\n\r",FALSE, promoter, 0, ch, TO_VICT);
      act("\
The shadow ninja tosses $N's measly 'fortune'\n\r\
back to $M, disappointed at the 'offering' to the\n\r\
brotherhood.\n\r",FALSE, promoter, 0, ch, TO_NOTVICT);
    }
  }
  else {
    act("\
The shadow ninja tests your skills, and defeats you\n\r\
easily. The shadow ninja says 'come back when you\n\r\
can best me.'\n\r",FALSE, promoter, 0, ch, TO_VICT);
    act("\
The shadow ninja tests $N's skills, and defeats\n\r\
$M easily.  The shadow ninja says 'come back \n\r\
when you can best me.'\n\r",FALSE, promoter, 0, ch, TO_NOTVICT);
  }

  act("\
The shadow ninja throws down a small rock,\n\r\
and a cloud of smoke billows up to engulf him.\n\r\
When the smoke clears, $n is left standing there.\n\r",FALSE, promoter, 0, ch, TO_ROOM);
}

void promote_warrior(CHAR *promoter, CHAR *ch)
{
  char buf[1000],title[50];
  ENCH ench;
  int exp=0,gold=0;

  if (enchanted_by_type(ch, ENCHANT_KNIGHT)) {
    exp=0;
  }
  else if (enchanted_by_type(ch, ENCHANT_SWASHBUCKLER)) {
    sprintf(title,"Knight");
    exp=15000000;
    gold=10000000;
  }
  else if (enchanted_by_type(ch, ENCHANT_SQUIRE)) {
    sprintf(title,"Swashbuckler");
    exp=10000000;
    gold=5000000;
  }
  else {
    sprintf(title,"Squire");
    exp=5000000;
    gold=1000000;
  }

  // Prestige Perk 4
  if (GET_PRESTIGE_PERK(ch) >= 4) {
    exp *= 0.9;
    gold *= 0.9;
  }

  act("\
$n pulls a silver horn from a fold in his cloak\n\r\
and blows it with all his might, transforming him\n\r\
into a warrior clad in furs and a horned helm.",FALSE,promoter,0, ch, TO_ROOM);
  if(!exp) {
    act("\
The warrior says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_ROOM);
    act("\
The warrior says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_CHAR);
    act("\
The warrior throw a small object on the ground and\n\r\
you are surrounded by smoke.\n\r\
When the smoke clears, $n is left standing there.",FALSE, promoter, 0, ch, TO_ROOM);
    return;
  }

      if(GET_EXP(ch) >= exp)
  {
    act("\
Erik challenges you to a drinking contest\n\r\
in which you quickly drink him under the table.",FALSE,promoter,0,ch,TO_VICT);
    act("\
Erik challenges $N to a drinking contest\n\r\
and quickly ends up under the table before $N.",FALSE,promoter,0,ch,TO_NOTVICT);

    if(GET_GOLD(ch) >= gold)
      {
        act("\
Seeing as Erik has passed out, you're stuck with\n\r\
the sizable bar tab.",FALSE, promoter, 0, ch, TO_VICT);
        act("\
Seeing as Erik has passed out, $N is stuck with \n\r\
a sizable bar tab.",FALSE, promoter, 0, ch, TO_NOTVICT);
        send_to_room("\
Erik raises his head and begins to sing VERY loudly.\n\r",CHAR_REAL_ROOM(promoter));
        sprintf(buf,"\
Erik the viking shouts '%s is a tremendous warrior of\n\r\
extra ordinary magnitude and has joined the ranks of the %s!'\n\r\
",PERS(ch,promoter),title);

      send_to_world(buf);
      sprintf(buf,"The title of %s",title);
      ench.name = buf;
      enchantment_to_char(ch,&ench,TRUE);
      GET_GOLD(ch) -= gold;
      GET_EXP(ch)  -= exp;
      }
    else
      {
        act("\
Because you haven't the coins to pay for this\n\r\
drunk fest, the royal order of Knights picks up your\n\r\
tab, and denies you membership till you can pay up.",
      FALSE,promoter,0,ch,TO_VICT);
        act("\
$N doesnt have the coins to pay for this\n\r\
drunk fest.  A member of the royal order of\n\r\
Knights will have to pick up the tab...they WON'T\n\r\
be pleased.",FALSE, promoter, 0, ch, TO_NOTVICT);
      }
  }
      else
  {
    act("\
Erik laughs in your face as you puke your guts out.\n\r\
Erik says 'Once you can drink me under the table,\n\r\
then we'll talk.'",FALSE, promoter, 0, ch, TO_VICT);
    act("\
Erik laughs in $N's face as $E pukes $S guts out.\n\r\
Erik says 'Once you can drink me under the table,\n\r\
then we'll talk.'",FALSE, promoter, 0, ch, TO_NOTVICT);
  }
      act("\
When the smoke clears, $n is left standing there.",FALSE, promoter, 0, ch, TO_ROOM);

}

void promote_paladin(CHAR *promoter, CHAR *ch)
{
  char buf[1000],title[50],tmptitle[50];
  ENCH ench;
  int exp=0,gold=0;

  if (enchanted_by_type(ch, ENCHANT_LORDLADY)) {
    exp=0;
  }
  else if (enchanted_by_type(ch, ENCHANT_JUSTICIAR)) {
    sprintf(title,"Lord/Lady");
    if(GET_SEX(ch)==SEX_FEMALE) sprintf(tmptitle,"Lady");
    else sprintf(tmptitle,"Lord");
    exp=15000000;
    gold=10000000;
  }
  else if (enchanted_by_type(ch, ENCHANT_FIRSTSWORD)) {
    sprintf(title,"Justiciar");
    sprintf(tmptitle,"Justiciar");
    exp=10000000;
    gold=5000000;
  }
  else {
    sprintf(title,"First Sword");
    sprintf(tmptitle,"First Sword");
    exp=5000000;
    gold=1000000;
  }

  // Prestige Perk 4
  if (GET_PRESTIGE_PERK(ch) >= 4) {
    exp *= 0.9;
    gold *= 0.9;
  }

    act("\
A column of brilliant white light appears in front\n\r\
of $n.  When your vision clears, \n\r\
you see a glorious Angel, wielding a sword, and\n\r\
holding a crippled child in her arms.",FALSE,promoter, NULL, ch, TO_ROOM);
  if(!exp) {
    act("\
The angel says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_ROOM);
    act("\
The angel says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_CHAR);
    act("\
There is a blinding flash of light and when your vision\n\r\
clears, the Angel and child are gone, leaving $n standing\n\r\
there.  You find yourself needing a few deep breaths to\n\r\
calm your racing heart.",FALSE, promoter, NULL, ch, TO_ROOM);
    return;
  }


    if(GET_EXP(ch) >= exp)
       {
        act("\
White flames engulf you, as the Angel tests your\n\r\
heart and soul and rejoices at finding you worthy of\n\r\
joining the hosts of the righteous.",FALSE, promoter, NULL, ch,
TO_VICT);
        act("\
White flames engulf $N, as the Angel tests $N's\n\r\
heart and soul and rejoices at finding $E worthy of\n\r\
joining the hosts of the righteous.",FALSE, promoter, NULL, ch,
TO_NOTVICT);

        if(GET_GOLD(ch) >= gold)
          {
          act("\
With great peity, you give the Angel much of the wealth you\n\r\
have collected to help the less fortunate.\n\r\
The child in the Angel's arms smiles at you and annoints\n\r\
your forehead with oil.",FALSE, promoter, NULL, ch, TO_VICT);
          act("\
With great peity, $N gives the Angel much of the wealth $E\n\r\
has collected to help the less fortunate.\n\r\
The child in the Angel's arms smiles at $N and annoints\n\r\
$N's forehead with oil.",FALSE, promoter, NULL, ch, TO_NOTVICT);
         send_to_room("\
The Angel voices a great trumpet, and you tremble at its\n\r\
sound.",CHAR_REAL_ROOM(promoter));
         sprintf(buf,"\
The sound of a great trumpet blares in the distance.\n\r\
A heavenly host sings with joy that %s has\n\r\
been chosen as a %s!'\n\r",PERS(ch,promoter),tmptitle);
      send_to_world(buf);
      sprintf(buf,"The title of %s",title);
      ench.name = buf;
      enchantment_to_char(ch,&ench,TRUE);
      GET_GOLD(ch) -= gold;
      GET_EXP(ch)  -= exp;
         }
        else
         {
         act("\
You are ashamed that you cannot contribute a substantial\n\r\
amount of gold to help relieve the suffering of the\n\r\
innocent and beg the Angel to withhold this honor until\n\r\
another time.",FALSE, promoter, NULL, ch, TO_VICT);
         act("\
$S is ashamed that $E cannot contribute a substantial\n\r\
amoutn of gold to help  relieve the suffering of the\n\r\
innocent and begs the Angel to withhold this honor until\n\r\
another time.",FALSE, promoter, NULL, ch, TO_NOTVICT);
         }
       }
     else
       {
       act("\
The Angel tests your heart and soul and, with a sad smile,\n\r\
tells you that you are not yet ready to join the hosts of\n\r\
the righteous.",FALSE, promoter, NULL, ch, TO_VICT);
       act("\
The Angel tests $N's heart and soul and, with a sad smile,\n\r\
shakes her head.",FALSE, promoter, NULL, ch, TO_NOTVICT);
       }
     act("\
There is a blinding flash of light and when your vision\n\r\
clears, the Angel and child are gone, leaving $n standing\n\r\
there.  You find yourself needing a few deep breaths to\n\r\
calm your racing heart.",FALSE, promoter, NULL, ch, TO_ROOM);


}


void promote_nomad(CHAR *promoter, CHAR *ch)
{
  char buf[1000],title[50];
  ENCH ench;
  int exp=0,gold=0;

  if (enchanted_by_type(ch, ENCHANT_TAMER)) {
    exp=0;
  }
  else if (enchanted_by_type(ch, ENCHANT_FORESTER)) {
    sprintf(title,"Tamer");
    exp=15000000;
    gold=10000000;
  }
  else if (enchanted_by_type(ch, ENCHANT_WANDERER)) {
    sprintf(title,"Forester");
    exp=10000000;
    gold=5000000;
  }
  else {
    sprintf(title,"Wanderer");
    exp=5000000;
    gold=1000000;
  }

  // Prestige Perk 4
  if (GET_PRESTIGE_PERK(ch) >= 4) {
    exp *= 0.9;
    gold *= 0.9;
  }

      act("\
A humongous bull rides up, creating a giant cloud,\n\r\
of dust that obscures $n.  When the dust clears,\n\r\
a muscled woman wearing furs stands there. She\n\r\
points toward the bull, indicating it is the test\n\r\
you must pass to join the clan.",FALSE,promoter, 0, ch, TO_ROOM);
  if(!exp) {
    act("\
The woman says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_ROOM);
    act("\
The woman says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_CHAR);
    act("\
The nomadess arrives in a tremendous dust cloud.\n\r\
When the dust clears, $n is left standing there.",FALSE, promoter, 0, ch, TO_ROOM);
    return;
  }

      if(GET_EXP(ch) >= exp)
  {
    act("\
The bull gives a decent fight, but in the end\n\r\
you last the 8 seconds.",FALSE, promoter, 0, ch, TO_VICT);
    act("\
The bull gives $N a decent fight, but in the end $E\n\r\
lasts the 8 seconds.",FALSE, promoter, 0, ch, TO_NOTVICT);

    if(GET_GOLD(ch) >= gold)
      {
        act("\
Contrary to most rodeo prizes, yours is a the priviledge\n\r\
of paying the clan's entry fees. You quickly pay up, as\n\r\
the fur clad nomadess looks AWFULLY tough.\n\r",FALSE, promoter, 0, ch, TO_VICT);
        act("\
The fur clad nomadess threatens $N until $E pays the \n\r\
clan's entry fees.\n\r",FALSE, promoter, 0, ch, TO_NOTVICT);
        send_to_room("\
The fur clad nomadess hops upon the bull and\n\r\
rides off to inform the rest of the clan.\n\r",CHAR_REAL_ROOM(promoter));
        sprintf(buf,"\
A fur clad nomadess rides through on a humongous\n\r\
bull yelling at the top of her lungs '%s has joined\n\r\
the clan of the %s!'\n\r",PERS(ch,promoter),title);
      send_to_world(buf);
      sprintf(buf,"The title of %s",title);
      ench.name = buf;
      enchantment_to_char(ch,&ench,TRUE);
      GET_GOLD(ch) -= gold;
      GET_EXP(ch)  -= exp;
      }
    else
      {
        act("\
The fur clad nomadess shakes you furiously demanding\n\r\
your dues to the clan.  When she notices you havent\n\r\
enough, she throws you to the ground.",FALSE, promoter, 0, ch, TO_VICT);
        act("\
The fur clad nomadess shakes $N furiously, demanding\n\r\
$S dues to the clan.  Finally she gives up and throws\n\r\
$M to the ground.",FALSE, promoter, 0, ch, TO_NOTVICT);
        GET_POS(ch) = POSITION_RESTING;
      }
  }
      else
  {
    act("\
You are bucked off the bull rather quickly.  The fur\n\r\
clad nomadess sneers at your pitiful riding skills.",FALSE,promoter,0,ch,TO_VICT);
    act("\
$N is bucked off the bull rather quickly.  The fur\n\r\
clad nomadess sneers at $S pitiful riding skills.",FALSE,promoter,0,ch,TO_NOTVICT);
  }
      act("\
The nomadess arrives in a tremendous dust cloud.\n\r\
When the dust clears, $n is left standing there.",FALSE, promoter, 0, ch, TO_ROOM);

}

void promote_antipaladin(CHAR *promoter, CHAR *ch)
{
  char buf[1000],title[50],tmptitle[50];
  ENCH ench;
  int exp=0,gold=0;

  if (enchanted_by_type(ch, ENCHANT_DARKLORDLADY)) {
    exp=0;
  }
  else if (enchanted_by_type(ch, ENCHANT_DARKWARDER)) {
    sprintf(title,"Dark Lord/Lady");
    if(GET_SEX(ch)==SEX_FEMALE) sprintf(tmptitle,"Dark Lady");
    else sprintf(tmptitle,"Dark Lord");
    exp=15000000;
    gold=10000000;
  }
  else if (enchanted_by_type(ch, ENCHANT_MINION)) {
    sprintf(title,"Dark Warder");
    sprintf(tmptitle,"Dark Warder");
    exp=10000000;
    gold=5000000;
  }
  else {
    sprintf(title,"Minion of Darkness");
    sprintf(tmptitle,"Minion of Darkness");
    exp=5000000;
    gold=1000000;
  }

  // Prestige Perk 4
  if (GET_PRESTIGE_PERK(ch) >= 4) {
    exp *= 0.9;
    gold *= 0.9;
  }

      act ("\
Darkness rises up from the ground and envelopes $n in itself.\n\r\
In a flash of abyssmal fire, a demon stands in front of you.", FALSE,promoter, 0, ch, TO_ROOM);
  if(!exp) {
    act("\
The demon says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_ROOM);
    act ("\
The demon says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_CHAR);
      act ("\
As suddenly as the darkness appeared, it slithers back down to the\n\r\
ground and $n is left standing there, looking a bit pale and weakened.",
     FALSE,promoter, 0, ch, TO_ROOM);
    return;
  }

      if (GET_EXP(ch) >= exp)
  {
    act ("\
The demon demands proof of your souls ultimate corruption,\n\r\
and seems to accept your claims of your evil deeds.",FALSE,promoter, 0, ch, TO_VICT);
    act ("\
The demon demands proof from $N of $S ultimate corruption,\n\r\
and seems to accept the claims $E makes.",FALSE,promoter, 0, ch, TO_NOTVICT);

    if (GET_GOLD(ch) >= gold)
      {
        act("\
The demon demands bribes from you in gold or in lives.\n\r\
You satisfy its claims with your coins.", FALSE, promoter, 0, ch, TO_VICT);
        act("\
The demon demands bribes from $N in gold or in lives.\n\r\
$E satisfies its claims with $S coins.", FALSE, promoter, 0, ch, TO_NOTVICT);
        send_to_room ("\
The demon gestures and marks you with the sign of Evil Incarnate.\n\r",
          CHAR_REAL_ROOM(promoter));
        sprintf (buf, "\
The unearthly screaming of %s marks the passage of another\n\r\
soul down the dark path. %s has become a %s.\n\r",
           PERS(ch,promoter), PERS(ch, promoter),tmptitle);
      send_to_world(buf);
      sprintf(buf,"The title of %s",title);
      ench.name = buf;
      enchantment_to_char(ch,&ench,TRUE);
      GET_GOLD(ch) -= gold;
      GET_EXP(ch)  -= exp;
      }
    else
      {
        act("\
The demon demands bribes from you in gold or in lives.\n\r\
Unfortunately, you have neither available in sufficient amounts.",
      FALSE, promoter, 0, ch, TO_VICT);
        act("\
The demon demands bribes from $N in gold or in lives.\n\r\
$E nearly angers the demon for lacking the proper sacrifices.",
FALSE, promoter, 0, ch, TO_NOTVICT);
      }
  }
      else
  {
    act("\
The demon takes one look at you and sneers.\n\r\
The demon says 'You truly aspire to become a minion of darkness. Bah!'",
        FALSE,promoter,0,ch,TO_VICT);
    act("\
The demon takes one look at $M and sneers.\n\r\
The demon says 'You truly aspire to become a minion of darkness. Bah!'",
        FALSE,promoter,0,ch,TO_NOTVICT);
  }

      act ("\
As suddenly as the darkness appeared, it slithers back down to the\n\r\
ground and $n is left standing there, looking a bit pale and weakened.",
     FALSE,promoter, 0, ch, TO_ROOM);

}

void promote_bard(CHAR *promoter, CHAR *ch)
{
  char buf[1000],title[50];
  ENCH ench;
  int exp=0,gold=0;

  if (enchanted_by_type(ch, ENCHANT_CONDUCTOR)) {
    exp=0;
  }
  else if (enchanted_by_type(ch, ENCHANT_POET)) {
    sprintf(title,"Conductor");
    exp=15000000;
    gold=10000000;
  }
  else if (enchanted_by_type(ch, ENCHANT_MINSTREL)) {
    sprintf(title,"Poet");
    exp=10000000;
    gold=5000000;
  }
  else {
    sprintf(title,"Minstrel");
    exp=5000000;
    gold=1000000;
  }

  // Prestige Perk 4
  if (GET_PRESTIGE_PERK(ch) >= 4) {
    exp *= 0.9;
    gold *= 0.9;
  }

      act("\
$n grabs a small flute from $s belt and starts playing a merry tune.\n\r\
The mesmerizing notes flow forth from the flute, bringing tears to your\n\r\
eyes. As soon as your eyes clear, you see an old meistersinger standing here.",
    FALSE,promoter, 0, ch, TO_ROOM);
  if(!exp) {
    act("\
The singer says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_ROOM);
    act("\
The singer says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_CHAR);
    act("With the merry tune fading, you see $n again, standing there.",
    FALSE, promoter, 0, ch, TO_ROOM);
    return;
  }

      if(GET_EXP(ch) >= exp)
  {
    act("\
The meistersinger challenges you to a contest of lore. $e and you spend\n\r\
hours after hours singing and playing nearly forgotten songs and after\n\r\
a time, the meistersinger looks at you and seems pleased with your\n\r\
knowledge and skills.",FALSE, promoter, 0, ch, TO_VICT);
    act("\
The meistersinger challenges $N to a contest of lore. $e and $E spend\n\r\
hours after hours singing and playing nearly forgotten songs and after\n\r\
a time, the meistersinger looks at $M and seems pleased with $S\n\r\
knowledge and skills.",FALSE, promoter, 0, ch, TO_NOTVICT);

    if(GET_GOLD(ch) >= gold)
      {
        act("\
The meistersinger asks for a donation to aid in the construction of the\n\r\
Opera House and you gladly comply with $s wishes, giving gold to bring\n\r\
Art to all men.",FALSE, promoter, 0, ch, TO_VICT);
        act("\
The meistersinger asks for a donation to aid in the construction of the\n\r\
Opera House and $N gladly complies with $s wishes, giving gold to bring\n\r\
Art to all men.",FALSE, promoter, 0, ch, TO_NOTVICT);
        send_to_room("\
The meistersinger starts to sing a joyous verse that sounds through out\n\r\
the land.\n\r",CHAR_REAL_ROOM(promoter));
        sprintf(buf,"\
You hear a verse or two from a song celebrating the new member of\n\r\
the %s Guild.\n\r\
...Now it has come time again, To turn our ears and listen to,\n\r\
...Show the verses of %s so fine to all the world...'\n\r",title,PERS(ch,promoter));
      send_to_world(buf);
      sprintf(buf,"The title of %s",title);
      ench.name = buf;
      enchantment_to_char(ch,&ench,TRUE);
      GET_GOLD(ch) -= gold;
      GET_EXP(ch)  -= exp;
      }
    else
      {
        act("\
The meistersinger seems disappointed that you can't contribute to costs\n\r\
of the new Opera House. Such a pity.\n\r\
It seems though that the Opera House has priority to enlisting new members.",
      FALSE, promoter, 0, ch, TO_VICT);
        act("\
The meistersinger seems disappointed that $N cannot contribute to costs\n\r\
of the new Opera House. Such a pity.\n\r\
It seems though that the Opera House has priority to enlisting new members.",
      FALSE, promoter, 0, ch, TO_NOTVICT);
      }
  }
      else
  {
    act("\
The meistersinger challenges you to a contest of lore, but $e quickly\n\r\
stops and chides you for not memorizing even the most important verses.\n\r\
Now all you can do is to go back to your studies and try again later.",
        FALSE, promoter, 0, ch, TO_VICT);
    act("\
The meistersinger challenges $N to a contest of lore, but $e quickly\n\r\
stops and chides $M for not memorizing even the most important verses.\n\r\
Now $E looks really disappointed, but the meistersinger only encourages\n\r\
$M to study more and apply again.",FALSE, promoter, 0, ch, TO_NOTVICT);
  }

      act("With the merry tune fading, you see $n again, standing there.",
    FALSE, promoter, 0, ch, TO_ROOM);

}

void promote_commando(CHAR *promoter, CHAR *ch)
{
  char buf[1000],title[50];
  ENCH ench;
  int exp=0,gold=0;

  if (enchanted_by_type(ch, ENCHANT_COMMANDER)) {
    exp=0;
  }
  else if (enchanted_by_type(ch, ENCHANT_COMMODORE)) {
    sprintf(title,"Commander");
    exp=15000000;
    gold=10000000;
  }
  else if (enchanted_by_type(ch, ENCHANT_PRIVATE)) {
    sprintf(title,"Commodore");
    exp=10000000;
    gold=5000000;
  }
  else {
    sprintf(title,"Private");
    exp=5000000;
    gold=1000000;
  }

  // Prestige Perk 4
  if (GET_PRESTIGE_PERK(ch) >= 4) {
    exp *= 0.9;
    gold *= 0.9;
  }

      act("\
$n screams 'INCOMING!' and throws you to the ground.  When you try to\n\r\
get up, you realise you've been saved by the legendary commando, Ahnohld.",
    FALSE,promoter, 0, ch, TO_ROOM);
  if(!exp) {
    act("\
The commando says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_ROOM);
      act("\
The commando says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_CHAR);
      act("\
A swarm of Pink horrors run through, and Ahnohld chases after them.\n\r\
When you turn back, you see $n standing there.",
    FALSE, promoter, 0, ch, TO_ROOM);
    return;
  }

      if(GET_EXP(ch) >= exp)
  {
    act("\
Ahnohld screams in your face, 'Drop and Give me twenty!'\n\r\
You immediately fall to the ground and do 20 one handed push-ups.\n\r\
Ahnohld is suitibly impressed."
          ,FALSE, promoter, 0, ch, TO_VICT);
    act("\
Ahnohld screams in $N's face, 'Drop and Give me twenty!'\n\r\
$N immediately falls to the ground and does 20 one handed push-ups.\n\r\
Ahnohld is suitibly impressed."
          ,FALSE, promoter, 0, ch, TO_NOTVICT);

    if(GET_GOLD(ch) >= gold)
      {
        act("\
Ahnohld demands you pay for your uniform and training, Looking at his\n\r\
bulging muscles, you can't help but volunteer more than enough."
              ,FALSE, promoter, 0, ch, TO_VICT);
        act("\
Ahnohld demands $N pays for $S uniform and training, Looking at his\n\r\
bulging muscles, $N can't help but volunteer more than enough."
              ,FALSE, promoter, 0, ch, TO_NOTVICT);
        send_to_room("\
Ahnohld pulls a bugle from his bandolier and puts it to his lips."
              ,CHAR_REAL_ROOM(promoter));
        sprintf(buf,"\
You hear a tinny, out of tune, bugle play 'reveille' followed by a shout\n\r\
of acceptance.  '%s has been promoted to %s!'\n\r",
              PERS(ch,promoter),title);
      send_to_world(buf);
      sprintf(buf,"The rank of %s",title);
      ench.name = buf;
      enchantment_to_char(ch,&ench,TRUE);
      GET_GOLD(ch) -= gold;
      GET_EXP(ch)  -= exp;
      }
    else
      {
        act("\
Ahnohld demands you pay for your uniform and stripes.  A shame you don't\n\r\
have it, as he looks awfully annoyed.",
      FALSE, promoter, 0, ch, TO_VICT);
        act("\
Ahnohld demands $N pays for $S uniform and stripes.  A shame $E doesn't\n\r\
have it, as he looks awfully annoyed.",
      FALSE, promoter, 0, ch, TO_NOTVICT);
      }
  }
      else
  {
    act("\
Ahnohld screams in your face, 'Drop and Give me twenty!'\n\r\
You immediately fall to the ground try to struggle out 5 pushups.\n\r\
Ahnohld is NOT impressed."
        ,FALSE, promoter, 0, ch, TO_VICT);
    act("\
Ahnohld screams in $N's face, 'Drop and Give me twenty!'\n\r\
$N immediately falls to the ground tries to struggle out 5 pushups.\n\r\
Ahnohld is NOT impressed."
              ,FALSE, promoter, 0, ch, TO_NOTVICT);
  }
      act("\
A swarm of Pink horrors run through, and Ahnohld chases after them.\n\r\
When you turn back, you see $n standing there.",
    FALSE, promoter, 0, ch, TO_ROOM);

}

void promote_thief(CHAR *promoter, CHAR *ch)
{
  char buf[1000],title[50];
  ENCH ench;
  int exp=0,gold=0;

  if (enchanted_by_type(ch, ENCHANT_ASSASSIN)) {
    exp=0;
  }
  else if (enchanted_by_type(ch, ENCHANT_BRIGAND)) {
    sprintf(title,"Assassin");
    exp=15000000;
    gold=10000000;
  }
  else if (enchanted_by_type(ch, ENCHANT_HIGHWAYMAN)) {
    sprintf(title,"Brigand");
    exp=10000000;
    gold=5000000;
  }
  else {
    sprintf(title,"Highwayman");
    exp=5000000;
    gold=1000000;
  }

  // Prestige Perk 4
  if (GET_PRESTIGE_PERK(ch) >= 4) {
    exp *= 0.9;
    gold *= 0.9;
  }

      act("\
As $n sheds $s superb disguise, you see that $e in fact is The Shadow,\n\r\
a master thief with a legendary reputation. He looks at you as your peer.",
    FALSE,promoter, 0, ch, TO_ROOM);

  if(!exp) {
    act("\
The Shadow says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_ROOM);
      act("\
The Shadow says, 'I can do no more for you $n.\n\r\
You have attained the highest plane.\n\r",FALSE,ch,0,0,TO_CHAR);
      act("\
With a flair of eloquence, he reapplies the disquise, and is $n again.",
    FALSE, promoter, 0, ch, TO_ROOM);
    return;
  }

      if(GET_EXP(ch) >= exp)
  {
    act("\
The Shadow flicks a dagger at you with a sharp twist of $s wrist and looks\n\r\
quite satisfied at your skill in dodging it.",
        FALSE, promoter, 0, ch, TO_VICT);
    act("\
The Shadow flicks a dagger at $N with a sharp twist of $s wrist and looks\n\r\
quite satisfied at $S skill in dodging it.",
        FALSE, promoter, 0, ch, TO_NOTVICT);

    if(GET_GOLD(ch) >= gold)
      {
        act("\
The Shadow quickly pinches your purse and peers in it. He looks quite pleased\n\r\
at the jingle of gold coins in there. He politely gives you the purse back,\n\r\
after relieving you from a part of your load.",
      FALSE, promoter, 0, ch, TO_VICT);
        act("\
The Shadow quickly pinches $N's purse and peers in it. He looks quite pleased\n\r\
at the jingle of gold coins in there. He politely gives $M the purse back,\n\r\
after relieving $M from a part of $S load.",
      FALSE, promoter, 0, ch, TO_NOTVICT);

        send_to_room("\
The Shadow whispers quickly to a small hole in the wall.\n\r",
         CHAR_REAL_ROOM(promoter));
        sprintf(buf,"\
You hear a rumour that %s has made it to one of the %s.\n\r",
          PERS(ch,promoter),title);
      send_to_world(buf);
      sprintf(buf,"The status of %s",title);
      ench.name = buf;
      enchantment_to_char(ch,&ench,TRUE);
      GET_GOLD(ch) -= gold;
      GET_EXP(ch)  -= exp;
      }
    else
      {
        act("\
The Shadow quickly pinches your purse and peers in. He looks very upset\n\r\
and quite nonchalantly throws it back at you.",
      FALSE, promoter, 0, ch, TO_VICT);
        act("\
The Shadow quickly pinches $N's purse and peers in. He looks very upset\n\r\
and quite nonchalantly throws it back at $M.",
      FALSE, promoter, 0, ch, TO_NOTVICT);
      }
  }
      else
  {
    act("\
The Shadow flicks a dagger at you, and you can only wonder at his speed.\n\r\
Although the dagger makes only a tiny wound in your arm, you obviously\n\r\
didn't make much of an impression with your skills.",
        FALSE, promoter, 0, ch, TO_VICT);
    act("\
The Shadow flicks a dagger at $N, and $E can only wonder at his speed.\n\r\
Although the dagger makes only a tiny wound in $S arm, $E obviously\n\r\
didn't make much of an impression with $S skills.",
        FALSE, promoter, 0, ch, TO_NOTVICT);
  };

      act("\
With a flair of eloquence, he reapplies the disquise, and is $n again.",
    FALSE, promoter, 0, ch, TO_ROOM);

}

int mayor(CHAR *mayor, CHAR *ch, int cmd, char *arg) {
  static char open_path[] =
    "W3A3003b33000c111d0d1111e333333332e22c22111221a1S.";
  static char close_path[] =
    "W3A3003b33000c111d0d1111E333333332E22c22111221a1S.";

  static char *path;
  static int index;
  static bool move = FALSE;

  char buf[MIL];

  if (!move) {
    if (time_info.hours == 6) {
      move = TRUE;
      path = open_path;
      index = 0;
    } else if (time_info.hours == 20) {
      move = TRUE;
      path = close_path;
      index = 0;
    }
  }

  if (cmd == CMD_SAY && !strncmp(" promote", arg, strlen(" promote"))) {
    do_say(ch, arg, CMD_SAY);

    switch (GET_CLASS(ch)) {
      case CLASS_MAGIC_USER:
        promote_mage(mayor, ch);
        break;
      case CLASS_CLERIC:
        promote_cleric(mayor, ch);
        break;
      case CLASS_THIEF:
        promote_thief(mayor, ch);
        break;
      case CLASS_WARRIOR:
        promote_warrior(mayor, ch);
        break;
      case CLASS_NINJA:
        promote_ninja(mayor, ch);
        break;
      case CLASS_NOMAD:
        promote_nomad(mayor, ch);
        break;
      case CLASS_PALADIN:
        promote_paladin(mayor, ch);
        break;
      case CLASS_ANTI_PALADIN:
        promote_antipaladin(mayor, ch);
        break;
      case CLASS_BARD:
        promote_bard(mayor, ch);
        break;
      case CLASS_COMMANDO:
        promote_commando(mayor, ch);
        break;
    }

    save_char(ch, NOWHERE);

    return TRUE;
  }

  if (cmd != MSG_MOBACT || !move || GET_POS(mayor) < POSITION_SLEEPING || GET_POS(mayor) == POSITION_FIGHTING)
    return FALSE;

  switch (path[index]) {
    case '0':
      do_move(mayor, "\0", path[index] - '0' + 1);
      break;
    case '1':
      do_move(mayor, "\0", path[index] - '0' + 1);
      break;
    case '2':
      do_move(mayor, "\0", path[index] - '0' + 1);
      break;
    case '3':
      do_move(mayor, "\0", path[index] - '0' + 1);
      break;
    case 'W':
      snprintf(buf, sizeof(buf), "wake");
      command_interpreter(mayor, buf);
      snprintf(buf, sizeof(buf), "stand");
      command_interpreter(mayor, buf);
      act("$n stretches and groans loudly.", FALSE, mayor, 0, 0, TO_ROOM);
      break;
    case 'S':
      act("$n nods off and begins to snore loudly.", FALSE, mayor, 0, 0, TO_ROOM);
      snprintf(buf, sizeof(buf), "sleep");
      command_interpreter(mayor, buf);
      break;
    case 'a':
      do_say(mayor, "Time for a nap!", CMD_SAY);
      snprintf(buf, sizeof(buf), "yawn");
      command_interpreter(mayor, buf);
      break;
    case 'A':
      do_say(mayor, "Time for a walk through town!", CMD_SAY);
      break;
    case 'b':
      do_say(mayor, "What a view!", CMD_SAY);
      act("$n gazes upon the park to the south with pride.", FALSE, mayor, 0, 0, TO_ROOM);
      break;
    case 'c':
      do_say(mayor, "Vandals! Youngsters nowadays have no respect for anything!", CMD_SAY);
      snprintf(buf, sizeof(buf), "fist");
      command_interpreter(mayor, buf);
      break;
    case 'd':
      do_say(mayor, "Good day, citizens!", CMD_SAY);
      snprintf(buf, sizeof(buf), "smile");
      command_interpreter(mayor, buf);
      break;
    case 'e':
      do_say(mayor, "May our gates welcome one and all to our fair city.", CMD_SAY);
      act("$n looks proudly at the magnificent city gates.", FALSE, mayor, 0, 0, TO_ROOM);
      break;
    case 'E':
      do_say(mayor, "Trust in the strength of these old gates keep us safe from the dangers outside.", CMD_SAY);
      act("$n examines the sturdy city gates and nods $s head.", FALSE, mayor, 0, 0, TO_ROOM);
      break;
    case 'O':
      do_unlock(mayor, "gate", 0);
      do_open(mayor, "gate", 0);
      break;
    case 'C':
      do_close(mayor, "gate", 0);
      do_lock(mayor, "gate", 0);
      break;
    case '.':
      move = FALSE;
      break;
  }

  index++;

  return FALSE;
}

/* ********************************************************************
 *  General special procedures for mobiles                            *
 ******************************************************************** */

/* SOCIAL GENERAL PROCEDURES

   If first letter of the command is '!' this will mean that the following
   command will be executed immediately.

   "G",n      : Sets next line to n
   "g",n      : Sets next line relative to n, fx. line+=n
   "m<dir>",n : move to <dir>, <dir> is 0,1,2,3,4 or 5
   "w",n      : Wake up and set standing (if possible)
   "c<txt>",n : Look for a person named <txt> in the room
   "o<txt>",n : Look for an object named <txt> in the room
   "r<int>",n : Test if the npc in room number <int>?
   "s",n      : Go to sleep, return false if can't go sleep
   "e<txt>",n : echo <txt> to the room, can use $o/$p/$N depending on
   contents of the **thing
   "E<txt>",n : Send <txt> to person pointed to by thing
   "B<txt>",n : Send <txt> to room, except to thing
   "?<num>",n : <num> in [1..99]. A random chance of <num>% success rate.
   Will as usual advance one line upon sucess, and change
   relative n lines upon failure.
   "O<txt>",n : Open <txt> if in sight.
   "C<txt>",n : Close <txt> if in sight.
   "L<txt>",n : Lock <txt> if in sight.
   "U<txt>",n : Unlock <txt> if in sight.    */

/* Execute a social command.                                        */
void exec_social(CHAR *npc, char *cmd, int next_line,
     int *cur_line, void **thing)
{
  bool ok;

  if (GET_POS(npc) == POSITION_FIGHTING)
    return;

  ok = TRUE;

  switch (*cmd) {

  case 'G' :
    *cur_line = next_line;
    return;

  case 'g' :
    *cur_line += next_line;
    return;

  case 'e' :
    act(cmd+1, FALSE, npc, *thing, *thing, TO_ROOM);
    break;

  case 'E' :
    act(cmd+1, FALSE, npc, 0, *thing, TO_VICT);
    break;

  case 'B' :
    act(cmd+1, FALSE, npc, 0, *thing, TO_NOTVICT);
    break;

  case 'm' :
    do_move(npc, "", *(cmd+1)-'0'+1);
    break;

  case 'w' :
    if (GET_POS(npc) != POSITION_SLEEPING)
      ok = FALSE;
    else
      GET_POS(npc) = POSITION_STANDING;
    break;

  case 's' :
    if (GET_POS(npc) <= POSITION_SLEEPING)
      ok = FALSE;
    else
      GET_POS(npc) = POSITION_SLEEPING;
    break;

  case 'c' :  /* Find char in room */
    *thing = get_char_room_vis(npc, cmd+1);
    ok = (*thing != 0);
    break;

  case 'o' : /* Find object in room */
    *thing = get_obj_in_list_vis(npc, cmd+1,
         world[CHAR_REAL_ROOM(npc)].contents);
    ok = (*thing != 0);
    break;

  case 'r' : /* Test if in a certain room */
    ok = (CHAR_REAL_ROOM(npc) == atoi(cmd+1));
    break;

  case 'O' : /* Open something */
    do_open(npc, cmd+1, 0);
    break;

  case 'C' : /* Close something */
    do_close(npc, cmd+1, 0);
    break;

  case 'L' : /* Lock something  */
    do_lock(npc, cmd+1, 0);
    break;

  case 'U' : /* UnLock something  */
    do_unlock(npc, cmd+1, 0);
    break;

  case '?' : /* Test a random number */
    if (atoi(cmd+1) <= number(1,100))
      ok = FALSE;
    break;

  default:
    break;
  }  /* End Switch */

  if (ok)
    (*cur_line)++;
  else
    (*cur_line) += next_line;

}


int teacher(CHAR *mob, CHAR *ch, int cmd, char *arg)
{

  if (cmd==CMD_LIST) { /* List */
    send_to_char("You can buy a practice session : 200000\n\r", ch);
    return(TRUE);
  }

  else if (cmd==CMD_BUY) { /* Buy */
    if (GET_GOLD(ch) < 200000)
      {
  send_to_char("The Teacher tells you 'You don't have enough coins.'\n\r", ch);
  return(TRUE);
      }
    else {
      ch->specials.spells_to_learn += 1;
      GET_GOLD(ch)-=200000;
      send_to_char("The Teacher tells you 'You got 1 more practice session now.'\n\r", ch);
      return(TRUE);
    }
  }
  return(FALSE);
}

int exploded(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  CHAR *mur, *temp, *tch, *tmp_victim;

  mur = 0;

  if(cmd) return FALSE;

  if(GET_POS(mob)!=POSITION_FIGHTING)
    {
      for (tch=world[CHAR_REAL_ROOM(mob)].people; tch; tch = tch->next_in_room)
  {
    if ((!IS_NPC(tch))
        && ((IS_SET(tch->specials.pflag, PLR_KILL)) ||
      (IS_SET(tch->specials.pflag, PLR_THIEF))) && !CHAOSMODE)
      {   mur = tch; }

  }

      if (mur)
  {  act("$n screams 'I HAVE BEEN LOOKING FOR YOU!'",
        FALSE, mob, 0, 0, TO_ROOM);
    hit(mob, mur, TYPE_UNDEFINED);
    return(FALSE);
        }
    }

  if (GET_POS(mob)==POSITION_FIGHTING && (GET_HIT(mob) < 300))
    {        act("$n utters the words 'EXPLODED'.", TRUE, mob, 0,0, TO_ROOM);

       for(tmp_victim = character_list; tmp_victim; tmp_victim = temp)
         {
     temp = tmp_victim->next;
     if ( (CHAR_REAL_ROOM(mob) == CHAR_REAL_ROOM(tmp_victim)) &&
         (mob != tmp_victim) ) {
       send_to_char("That really HURT!!!", tmp_victim);
       GET_HIT(tmp_victim) = 10;
     }
         }
       extract_char(mob);
       return FALSE;
     }
  return FALSE;
}

#define MAGIC_USER_GUARD_ROOM 3017
#define CLERIC_GUARD_ROOM 3004
#define THIEF_GUARD_ROOM 3027
#define WARRIOR_GUARD_ROOM 3021
#define NINJA_GUARD_ROOM 3034
#define NOMAD_GUARD_ROOM 3036
#define ANTI_PALADIN_GUARD_ROOM 3065
#define PALADIN_GUARD_ROOM 3063
#define COMMANDO_GUARD_ROOM 3067
#define BARD_GUARD_ROOM 3069
int guild_guard(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
  bool block_movement = FALSE;

  if (cmd < CMD_NORTH || cmd > CMD_DOWN) return FALSE;
  if (IS_IMMORTAL(ch)) return FALSE;

  switch (CHAR_VIRTUAL_ROOM(ch)) {
    case MAGIC_USER_GUARD_ROOM:
      if (cmd == CMD_SOUTH && (GET_CLASS(ch) != CLASS_MAGIC_USER || GET_POS(ch) == POSITION_RIDING)) {
        block_movement = TRUE;
    }
      break;
    case CLERIC_GUARD_ROOM:
      if (cmd == CMD_NORTH && (GET_CLASS(ch) != CLASS_CLERIC || GET_POS(ch) == POSITION_RIDING)) {
        block_movement = TRUE;
    }
      break;
    case THIEF_GUARD_ROOM:
      if (cmd == CMD_EAST && (GET_CLASS(ch) != CLASS_THIEF || GET_POS(ch) == POSITION_RIDING)) {
        block_movement = TRUE;
    }
      break;
    case WARRIOR_GUARD_ROOM:
      if (cmd == CMD_EAST && (GET_CLASS(ch) != CLASS_WARRIOR || GET_POS(ch) == POSITION_RIDING)) {
        block_movement = TRUE;
    }
      break;
    case NINJA_GUARD_ROOM:
      if (cmd == CMD_WEST && (GET_CLASS(ch) != CLASS_NINJA || GET_POS(ch) == POSITION_RIDING)) {
        block_movement = TRUE;
    }
      break;
    case NOMAD_GUARD_ROOM:
      if (cmd == CMD_EAST && (GET_CLASS(ch) != CLASS_NOMAD || GET_POS(ch) == POSITION_RIDING)) {
        block_movement = TRUE;
    }
      break;
    case ANTI_PALADIN_GUARD_ROOM:
      if (cmd == CMD_NORTH && (GET_CLASS(ch) != CLASS_ANTI_PALADIN || GET_POS(ch) == POSITION_RIDING)) {
        block_movement = TRUE;
    }
      break;
    case PALADIN_GUARD_ROOM:
      if (cmd == CMD_NORTH && (GET_CLASS(ch) != CLASS_PALADIN || GET_POS(ch) == POSITION_RIDING)) {
        block_movement = TRUE;
    }
      break;
    case COMMANDO_GUARD_ROOM:
      if (cmd == CMD_EAST && (GET_CLASS(ch) != CLASS_COMMANDO || GET_POS(ch) == POSITION_RIDING)) {
        block_movement = TRUE;
    }
      break;
    case BARD_GUARD_ROOM:
      if (cmd == CMD_SOUTH && (GET_CLASS(ch) != CLASS_BARD || GET_POS(ch) == POSITION_RIDING)) {
        block_movement = TRUE;
    }
      break;
    default:
      block_movement = FALSE;
      break;
    }

  if (block_movement) {
    send_to_char("The guard humiliates you, and blocks your way.\n\r", ch);
    act("The guard humiliates $n, and blocks $s way.", FALSE, ch, 0, 0, TO_ROOM);

    return TRUE;
  }

  return FALSE;
}

#define SANE_CLUB_ENTRANCE   3075
#define LINER_CLUB_ENTRANCE  3078
#define LEM_CLUB_ENTRANCE    3080
#define RANGER_CLUB_ENTRANCE 3082

#define CLUB_MEMBERSHIP_FEE  500000
#define CLUB_CANCEL_FEE      100000

int membership(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  char buf[MIL];

  if ((cmd != CMD_BUY) && (cmd != CMD_LIST)) return FALSE;

  arg = one_argument(arg, buf);

  if (cmd == CMD_LIST) {
    snprintf(buf, sizeof(buf), "The fee to BUY a membership is %d coins.\n\r", CLUB_MEMBERSHIP_FEE);
    send_to_char(buf, ch);
    snprintf(buf, sizeof(buf), "The fee to CANCEL a membership is %d coins.\n\r", CLUB_CANCEL_FEE);
    send_to_char(buf, ch);
    return TRUE;
  }

  if (!*buf) {
    send_to_char("Buy what?\n\r", ch);
    return TRUE;
  }

  if (!strcmp(buf, "membership")) {
    if (GET_GOLD(ch) < CLUB_MEMBERSHIP_FEE) {
      send_to_char("You don't have enough coins!", ch);
      return TRUE;
    }

    if (IS_SET(GET_PFLAG(ch), PLR_SANES_VOCAL_CLUB) ||
        IS_SET(GET_PFLAG(ch), PLR_LEMS_LIQOUR_ROOM) ||
        IS_SET(GET_PFLAG(ch), PLR_LINERS_LOUNGE) ||
        IS_SET(GET_PFLAG(ch), PLR_RANGERS_RELIQUARY)) {
      send_to_char("You can only join one club!\n\r", ch);
      return TRUE;
    }

    if (CHAR_REAL_ROOM(ch) == real_room(SANE_CLUB_ENTRANCE)) {
      send_to_char("You are now a member of Sane's Vocal Club.\n\r", ch);
      act("$n is now a member of Sane's Vocal Club.", FALSE, ch, 0, 0, TO_ROOM);
      SET_BIT(GET_PFLAG(ch), PLR_SANES_VOCAL_CLUB);
    }
    else if (CHAR_REAL_ROOM(ch) == real_room(LINER_CLUB_ENTRANCE)) {
      send_to_char("You are now a member of Liner's Lounge.\n\r", ch);
      act("$n is now a member of Liner's Lounge.", FALSE, ch, 0, 0, TO_ROOM);
      SET_BIT(GET_PFLAG(ch), PLR_LINERS_LOUNGE);
    }
    else if (CHAR_REAL_ROOM(ch) == real_room(LEM_CLUB_ENTRANCE)) {
      send_to_char("You are now a member of Lem's Liqour Lounge.\n\r", ch);
      act("$n is now a member of Lem's Liqour Lounge.", FALSE, ch, 0, 0, TO_ROOM);
      SET_BIT(GET_PFLAG(ch), PLR_LEMS_LIQOUR_ROOM);
    }
    else if (CHAR_REAL_ROOM(ch) == real_room(RANGER_CLUB_ENTRANCE)) {
      send_to_char("You are now a member of Ranger's Reliquary.\n\r", ch);
      act("$n is now a member of Ranger's Reliquary.", FALSE, ch, 0, 0, TO_ROOM);
      SET_BIT(GET_PFLAG(ch), PLR_RANGERS_RELIQUARY);
    }
    else {
      send_to_char("You aren't at the entrance of a club!\n\r", ch);
      return TRUE;
    }

    GET_GOLD(ch) -= CLUB_MEMBERSHIP_FEE;

    return TRUE;
  }
  else if (!strcmp(buf, "cancel")) {
    if (GET_GOLD(ch) < CLUB_CANCEL_FEE) {
      send_to_char("You don't have enough coins!\n\r", ch);
      return TRUE;
    }

    if (IS_SET(GET_PFLAG(ch), PLR_RANGERS_RELIQUARY))
      REMOVE_BIT(GET_PFLAG(ch), PLR_RANGERS_RELIQUARY);
    else if (IS_SET(GET_PFLAG(ch), PLR_LINERS_LOUNGE))
      REMOVE_BIT(GET_PFLAG(ch), PLR_LINERS_LOUNGE);
    else if (IS_SET(GET_PFLAG(ch), PLR_SANES_VOCAL_CLUB))
      REMOVE_BIT(GET_PFLAG(ch), PLR_SANES_VOCAL_CLUB);
    else if (IS_SET(GET_PFLAG(ch), PLR_LEMS_LIQOUR_ROOM))
      REMOVE_BIT(GET_PFLAG(ch), PLR_LEMS_LIQOUR_ROOM);
    else {
      send_to_char("But you are not a member of a club!\n\r", ch);
      return TRUE;
    }

    GET_GOLD(ch) -= CLUB_CANCEL_FEE;

    send_to_char("You are no longer a member of a club.\n\r", ch);
    return TRUE;
  }
  else {
    send_to_char("Buy what?\n\r", ch);
    return TRUE;
  }

  return FALSE;
}

int club_guard(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  if (cmd < CMD_NORTH || cmd > CMD_WEST) return FALSE;

  if (((CHAR_REAL_ROOM(ch) == real_room(SANE_CLUB_ENTRANCE) &&
        cmd == CMD_WEST) ||
       (CHAR_REAL_ROOM(ch) == real_room(LINER_CLUB_ENTRANCE) &&
        cmd == CMD_NORTH) ||
       (CHAR_REAL_ROOM(ch) == real_room(LEM_CLUB_ENTRANCE) &&
        cmd == CMD_EAST) ||
       (CHAR_REAL_ROOM(ch) == real_room(RANGER_CLUB_ENTRANCE) &&
        cmd == CMD_SOUTH))) {
    if (!IS_SET(GET_PFLAG(ch), PLR_SANES_VOCAL_CLUB) &&
        !IS_SET(GET_PFLAG(ch), PLR_LINERS_LOUNGE) &&
        !IS_SET(GET_PFLAG(ch), PLR_LEMS_LIQOUR_ROOM) &&
        !IS_SET(GET_PFLAG(ch), PLR_RANGERS_RELIQUARY)) {
      send_to_char("The guard humiliates you and says, 'You are not a member of a club!'\n\r", ch);
      act("The guard humiliates $n and says, 'You are not a member a club!'", FALSE, ch, 0, 0, TO_ROOM);
      return TRUE;
    }
    else if (GET_POS(ch) == POSITION_RIDING) {
      send_to_char("Dismount please!\n\r", ch);
      return TRUE;
    }
  }

  return FALSE;
}

int fido(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
  OBJ *i, *tmp, *temp, *next_obj;

  if (cmd || !AWAKE(mob))
    return(FALSE);

  for (i = world[CHAR_REAL_ROOM(mob)].contents; i; i = tmp) {
    tmp=i->next_content;  /* Added tmp - Ranger June 96 */
    if (OBJ_TYPE(i)==ITEM_CONTAINER && i->obj_flags.value[3] &&
        i->obj_flags.cost!=PC_STATUE && i->obj_flags.cost!=NPC_STATUE) {
      act("$n savagely devours a corpse.", FALSE, mob, 0, 0, TO_ROOM);
      for(temp = i->contains; temp; temp=next_obj)
  {
    next_obj = temp->next_content;
    obj_from_obj(temp);
    obj_to_room(temp,CHAR_REAL_ROOM(mob));
  }
      extract_obj(i);
      return(FALSE);
    }
  }
  return(FALSE);
}

#define DOG_CATCHER 3080

int dog_catcher(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
    CHAR* victim;

    if(!AWAKE(mob)) return FALSE;
    if(cmd != MSG_MOBACT) return FALSE;

    if(chance(2))
    {
        for(victim=world[CHAR_REAL_ROOM(mob)].people;victim;victim=victim->next_in_room)
       {
            if(!victim) return FALSE;
            if(IS_NPC(victim) && GET_CLASS(victim) == CLASS_CANINE)
            {
                act("Look, a dog!",1,mob,0,victim,TO_CHAR);
                act("Oh no, $n's got you!",1,mob,0,victim,TO_VICT);
                act("$n sees $N and attacks $M",1,mob,0,victim,TO_NOTVICT);
                hit(mob,victim,CMD_ASSAULT);
                return FALSE;
            }
        }
    }
    return FALSE;
}

int janitor(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
  OBJ *i,*temp;

  if (cmd || !AWAKE(mob))
    return(FALSE);

  for (i = world[CHAR_REAL_ROOM(mob)].contents; i; i = temp) {
    temp = i->next_content;  /* Added temp - Ranger June 96 */
    if(CAN_TAKE(mob,i) &&
  ((i->obj_flags.type_flag == ITEM_DRINKCON) ||
   (i->obj_flags.cost <= 10))) {
      act("$n picks up some trash.", FALSE, mob, 0, 0, TO_ROOM);

      obj_from_room(i);
      obj_to_char(i, mob);
      return(FALSE);
    }
  }
  return(FALSE);
}


int cityguard(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
  CHAR *tch, *evil, *mur;
  int max_evil;

  if (cmd || !AWAKE(mob) || (GET_POS(mob) == POSITION_FIGHTING))
    return (FALSE);

  max_evil = 1000;
  evil = 0;
  mur = 0;

  for (tch=world[CHAR_REAL_ROOM(mob)].people; tch; tch = tch->next_in_room) {
    if (tch->specials.fighting) {
      if ((GET_ALIGNMENT(tch) < max_evil) &&
    (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
  max_evil = GET_ALIGNMENT(tch);
  evil = tch;
      }
    }
    else if ((!IS_NPC(tch))
       && ((IS_SET(tch->specials.pflag, PLR_KILL)) || (IS_SET(tch->specials.pflag, PLR_THIEF)))&& !CHAOSMODE)
      {  mur = tch; }

  }
  if(CHAOSMODE) return FALSE;
  if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0)) {
    act("$n screams 'PROTECT THE INNOCENT!  BANZAI!!! CHARGE!!! ARARARAGGGHH!'", FALSE, mob, 0, 0, TO_ROOM);
    hit(mob, evil, TYPE_UNDEFINED);
    return(FALSE);
  }

  else if (mur) {
    act("$n screams 'I HAVE BEEN LOOKING FOR YOU! PROTECT THE INNOCENT! PROTECT THE LAW!'", FALSE, mob, 0, 0, TO_ROOM);
    hit(mob, mur, TYPE_UNDEFINED);
    return(FALSE);
  }

  return(FALSE);
}

int jail(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
  CHAR *vict, *mur, *tch, *temp;

  mur = 0;

  if(cmd) return FALSE;

  if(CHAOSMODE) return FALSE;

  if(GET_POS(mob)!=POSITION_FIGHTING)
    {
      for (tch=world[CHAR_REAL_ROOM(mob)].people; tch; tch = temp) {
        temp = tch->next_in_room;  /* Added temp - Ranger June 96 */
    if ((!IS_NPC(tch))
        && ((IS_SET(tch->specials.pflag, PLR_KILL)) ||
      (IS_SET(tch->specials.pflag, PLR_THIEF)))&& !CHAOSMODE)
      {   mur = tch; }

  }

      if (mur)
  {  act("$n screams 'I HAVE BEEN LOOKING FOR YOU!'",
        FALSE, mob, 0, 0, TO_ROOM);
    act("$N screams loudy when $n takes $N to jail", 1, mob, 0, mur, TO_NOTVICT);
    act("$n takes you to the jail!", 1, mob, 0, mur, TO_VICT);
    char_from_room(mur);
    char_to_room(mur, real_room(10));
    return(FALSE);
        }
    }

  if(mob->specials.fighting &&
     (CHAR_REAL_ROOM(mob->specials.fighting) == CHAR_REAL_ROOM(mob)))
    {
      vict = mob->specials.fighting;
      act("$N screams loudly when $n takes $N to jail", 1, mob, 0, vict, TO_NOTVICT);
      act("$n takes you to the jail!", 1, mob, 0, vict, TO_VICT);
      char_from_room(vict);
      char_to_room(vict, real_room(10));

      return FALSE;
    }

  return FALSE;
}


int jail_room(int room,CHAR *ch, int cmd, char *arg) {

  if(!ch) return FALSE;
  if(cmd==CMD_QUIT || cmd==CMD_RECITE) {
    if(!IS_MORTAL(ch) && !IS_NPC(ch)) return FALSE;

    send_to_char("That action has no meaning here...", ch);

    return TRUE;
  }
  return FALSE;
}


int pet_shops(int room,CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH], pet_name[MIL],tmp_pwd[11];
  int pet_room;
  CHAR *pet;
  if(!ch)
    return(FALSE);

  pet_room = CHAR_REAL_ROOM(ch)+1;

  if (cmd==CMD_LIST) {    /* List */
    send_to_char("Available pets are:\n\r", ch);
    for(pet = world[pet_room].people; pet; pet = pet->next_in_room) {
      sprintf(buf, "%8d - %s\n\r", 3*GET_EXP(pet), MOB_SHORT(pet));
      send_to_char(buf, ch);
    }
    return(TRUE);
  } else if (cmd==CMD_BUY) {  /* Buy */

    arg = one_argument(arg, buf);
    arg = one_argument(arg, pet_name);
    /* Pet_Name is for later use when I feel like it */

    if (!(pet = get_char_room(buf, pet_room))) {
      send_to_char("There is no such pet!\n\r", ch);
      return(TRUE);
    }

    if (GET_GOLD(ch) < (GET_EXP(pet)*3)) {
      send_to_char("You don't have enough gold!\n\r", ch);
      return(TRUE);
    }

    /* Addition of mob follow limit - Ranger April 98 */
    if(count_mob_followers(ch)>9) {
      send_to_char("You can't control anymore followers.\n\r",ch);
      return(TRUE);
    }

    GET_GOLD(ch) -= GET_EXP(pet)*3;

    pet = read_mobile(pet->nr, REAL);
    GET_EXP(pet) = 0;
    SET_BIT(pet->specials.affected_by, AFF_CHARM);

    /* In below added in MOB_NAME and MOB_DESCRIPTION instead of the pointers
       that did'nt work - Ranger April 96 */

    if (*pet_name) {
      if(test_char(pet_name, tmp_pwd)) {
        send_to_char("Please use a different name for your pet.\n\r",ch);
        return(TRUE);
      }

      sprintf(buf,"%s %s", MOB_NAME(pet), pet_name);
      free(pet->player.name);
      pet->player.name = str_dup(buf);

      sprintf(buf,"%sA small sign on a chain around the neck says 'My Name is %s'\n\r",
        MOB_DESCRIPTION(pet), pet_name);
      free(pet->player.description);
      pet->player.description = str_dup(buf);
    }

    char_to_room(pet, CHAR_REAL_ROOM(ch));
    add_follower(pet, ch);

    /* Be certain that pet's can't get/carry/use/weild/wear items */

    send_to_char("May you enjoy your pet.\n\r", ch);
    act("$n bought $N as a pet.",FALSE,ch,0,pet,TO_ROOM);

    return(TRUE);
  }

  /* All commands except list and buy */
  return(FALSE);
}

int restguard(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
  CHAR *tch,*temp;

  if (cmd || !AWAKE(mob) || (GET_POS(mob) == POSITION_FIGHTING))
    return (FALSE);


  for (tch=world[CHAR_REAL_ROOM(mob)].people; tch; tch = temp ) {
    temp = tch->next_in_room; /* Added temp - Ranger June 96 */
      if (tch->specials.fighting && !IS_NPC(tch))
  {

    act("$n screams 'PROTECT THE RESTAURANT!!!!!!!!!!'", FALSE, mob, 0, 0, TO_ROOM);
    hit(mob, tch, TYPE_UNDEFINED);
    return(FALSE);
  }
    }

  return(FALSE);
}

int teller(OBJ *obj, CHAR *ch, int cmd, char *arg) {
  if (cmd != CMD_BALANCE && cmd != CMD_DEPOSIT && cmd != CMD_WITHDRAW) return FALSE;

  if (IS_NPC(ch)) return FALSE;

  if (cmd == CMD_BALANCE) {
    printf_to_char(ch, "Your balance is %d coins.\n\r", GET_BANK(ch));

    return TRUE;
  }

  char buf[MIL];

  one_argument(arg, buf);

  if (!*buf) {
    send_to_char("Please specify an amount.\n\r", ch);

    return TRUE;
  }

  uint64_t coins = 0;

  if (!strcmp(buf, "all")) {
    if (cmd == CMD_DEPOSIT) {
      coins = GET_GOLD(ch);
    }
    else if (cmd == CMD_WITHDRAW) {
      coins = GET_BANK(ch);
    }
  }
  else {
    coins = strtoll(buf, NULL, 10);

    if (coins <= 0) {
      send_to_char("Amount must be positive.\n\r", ch);

      return TRUE;
    }
  }

  if (cmd == CMD_DEPOSIT) {
    if (GET_GOLD(ch) < coins) {
      send_to_char("You don't have that many coins!\n\r", ch);
    }
    else if (coins <= 0) {
      send_to_char("You don't have any coins to deposit.\n\r", ch);
    }
    else if ((INT_MAX - GET_BANK(ch)) < coins) {
      send_to_char("Your bank account is full!\n\r", ch);
    }
    else {
      GET_GOLD(ch) -= coins;
      GET_BANK(ch) += coins;

      printf_to_char(ch, "You deposit %ld coins.\n\r", coins);
    }

    return TRUE;
  }

  if (cmd == CMD_WITHDRAW) {
    if (GET_BANK(ch) < coins) {
      send_to_char("You don't have that many coins in the bank!\n\r", ch);
    }
    else if (coins <= 0) {
      send_to_char("You don't have any coins to withdraw.\n\r", ch);
    }
    else if ((INT_MAX - GET_GOLD(ch)) < coins) {
      send_to_char("You can't carry any more coins.\n\r", ch);
    }
    else {
      GET_BANK(ch) -= coins;
      GET_GOLD(ch) += coins;

      printf_to_char(ch, "You withdraw %ld coins.\n\r", coins);
    }

    return TRUE;
  }

  return FALSE;
}

int flower(OBJ *obj,CHAR *ch, int cmd, char *arg) {
  char obj_name[MAX_STRING_LENGTH], vict_name[MAX_STRING_LENGTH];
  CHAR *victim;
  OBJ *flower;

  if ((cmd != CMD_LIST) && (cmd != CMD_BUY)) return(FALSE);

  arg = one_argument(arg, obj_name);
  arg = one_argument(arg, vict_name);

  if (cmd==CMD_LIST) { /* List */
    send_to_char("You can buy :\n\rA rose for 500 coins\n\r", ch);
    return(TRUE);
  }

  else {

    if (!*obj_name || !*vict_name)
      {
  send_to_char("Buy what to who?\n\r",ch);
  return(TRUE);
      }

    if (!strcmp(obj_name,"rose"))
      {
  if (GET_GOLD(ch) < 500) {
    send_to_char("You don't have enough coins!", ch);
    return(TRUE);
  }

  if (!(victim = get_char_vis(ch, vict_name)))
    {  send_to_char("No-one by that name around.\n\r", ch);
    return(TRUE);
        }

  GET_GOLD(ch) -= 500;
  send_to_char("Ok.\n\r", ch);
  act("$n has bought a flower to someone.", FALSE, ch, 0, 0, TO_ROOM);

  flower = read_object(1, REAL);
  obj_to_char(flower, victim);
  act("A fairy appears suddenly and gives a rose to $n.", TRUE, victim, 0, 0, TO_ROOM);
  act("A fairy appears suddenly and gives a rose to you.", TRUE, victim, 0, 0, TO_CHAR);
  act("The Fairy tells you 'The flower is from $N', and she disappears.", TRUE, victim, 0, ch, TO_CHAR);
  act("The Fairy disappears.", TRUE, victim, 0, 0, TO_ROOM);

      }
    else
      send_to_char("Buy what?\n\r", ch);
    return(TRUE);
  }
  return FALSE;
}

int newbie_hospital(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_STRING_LENGTH];
  int cost,mult=1;

  if(IS_NPC(ch)) return FALSE;
  if(cmd!=CMD_BUY && cmd!=CMD_LIST) return FALSE;

  if(CHAOSMODE) mult=10;
  if (cmd==CMD_LIST) { /* List */
    send_to_char("The Healer says 'I can offer you the following until you are level 16:'\n\r", ch);
    cost=GET_LEVEL(ch)*2500*mult;
    sprintf(buf,"Miracle will cost you %d coins. (miracle)\n\r", cost);
    send_to_char(buf,ch);

    cost=GET_LEVEL(ch)*1000*mult;
    sprintf(buf,"Heal will cost you %d coins. (heal)\n\r", cost);
    send_to_char(buf,ch);

    cost=GET_LEVEL(ch)*1000*mult;
    sprintf(buf,"Sanctuary will cost you %d coins. (sanctuary)\n\r",cost);
    send_to_char(buf,ch);

    cost=GET_LEVEL(ch)*500*mult;
    sprintf(buf,"Remove Poison will cost you %d coins. (poison)\n\r",cost);
    send_to_char(buf,ch);

    cost=GET_LEVEL(ch)*150*mult;
    sprintf(buf,"Cure Serious will cost you %d coins. (serious)\n\r",cost);
    send_to_char(buf,ch);

    cost=GET_LEVEL(ch)*100*mult;
    sprintf(buf,"Endure will cost you %d coins. (endure)\n\r", cost);
    send_to_char(buf,ch);

    cost=GET_LEVEL(ch)*50*mult;
    sprintf(buf,"Armor will cost you %d coins. (armor)\n\r", cost);
    send_to_char(buf, ch);

    cost=GET_LEVEL(ch)*45*mult;
    sprintf(buf,"Vitality will cost you %d coins. (vitality)\n\r",cost);
    send_to_char(buf, ch);

    return(TRUE);
  }

  if (cmd==CMD_BUY) { /* Buy */

  if(ch->specials.fighting) {
    send_to_char("Doctor Naikrovek says 'This hospital isn't for fighting, go away!'\n\r",ch);
    return TRUE;
  }

  if(GET_LEVEL(ch)> 25 && !CHAOSMODE) {
    send_to_char("Doctor Naikrovek says 'This hospital is for level 25 and under only.'\n\r",ch);
    return TRUE;
  }
    arg=one_argument(arg, buf);
    if(!*buf) {
      send_to_char("Doctor Naikrovek says 'What would you like to buy?'\n\r",ch);
      return(TRUE);
    }

    if(!strcmp(buf,"miracle")) {
      cost=GET_LEVEL(ch)*2500*mult;
      if(GET_GOLD(ch)<cost) {
        send_to_char("Doctor Naikrovek says 'Sorry, you dont have enough money.'\n\r",ch);
        return(TRUE);
      }
      cast_miracle(35,mob,"",SPELL_TYPE_SPELL,ch,0);
      GET_GOLD(ch)-=cost;
      sprintf(buf,"Doctor Naikrovek says 'That will be %d coins please.'\n\r",cost);
      send_to_char(buf,ch);
      return(TRUE);
    }

    if(!strcmp(buf,"heal")) {
      cost=GET_LEVEL(ch)*1000*mult;
      if(GET_GOLD(ch)<cost) {
        send_to_char("Doctor Naikrovek says 'Sorry, you dont have enough money.'\n\r",ch);
        return(TRUE);
      }
      cast_heal(35,mob,"",SPELL_TYPE_SPELL,ch,0);
      GET_GOLD(ch)-=cost;
      sprintf(buf,"Doctor Naikrovek says 'That will be %d coins please.'\n\r",cost);
      send_to_char(buf,ch);
      return(TRUE);
    }

    if(!strcmp(buf,"sanctuary")) {
      cost=GET_LEVEL(ch)*1000*mult;
      if(GET_GOLD(ch)<cost) {
        send_to_char("Doctor Naikrovek says 'Sorry, you dont have enough money.'\n\r",ch);
        return(TRUE);
      }
      cast_sanctuary(35,mob,"",SPELL_TYPE_SPELL,ch,0);
      GET_GOLD(ch)-=cost;
      sprintf(buf,"Doctor Naikrovek says 'That will be %d coins please.'\n\r",cost);
      send_to_char(buf,ch);
      return(TRUE);
    }

    if(!strcmp(buf,"poison")) {
      cost=GET_LEVEL(ch)*500*mult;
      if(GET_GOLD(ch)<cost) {
        send_to_char("Doctor Naikrovek says 'Sorry, you dont have enough money.'\n\r",ch);
        return(TRUE);
      }
      cast_remove_poison(35,mob,"",SPELL_TYPE_SPELL,ch,0);
      GET_GOLD(ch)-=cost;
      sprintf(buf,"Doctor Naikrovek says 'That will be %d coins please.'\n\r",cost);
      send_to_char(buf,ch);
      return(TRUE);
    }

    if(!strcmp(buf,"serious")) {
      cost=GET_LEVEL(ch)*150*mult;
      if(GET_GOLD(ch)<cost) {
        send_to_char("Doctor Naikrovek says 'Sorry, you dont have enough money.'\n\r",ch);
        return(TRUE);
      }
      cast_cure_serious(35,mob,"",SPELL_TYPE_SPELL,ch,0);
      GET_GOLD(ch)-=cost;
      sprintf(buf,"Doctor Naikrovek says 'That will be %d coins please.'\n\r",cost);
      send_to_char(buf,ch);
      return(TRUE);
    }

    if(!strcmp(buf,"endure")) {
      cost=GET_LEVEL(ch)*100*mult;
      if(GET_GOLD(ch)<cost) {
        send_to_char("Doctor Naikrovek says 'Sorry, you dont have enough money.'\n\r",ch);
        return(TRUE);
      }
      cast_endure(35,mob,"",SPELL_TYPE_SPELL,ch,0);
      GET_GOLD(ch)-=cost;
      sprintf(buf,"Doctor Naikrovek says 'That will be %d coins please.'\n\r",cost);
      send_to_char(buf,ch);
      return(TRUE);
    }

    if (!strcmp(buf,"armor")) {
      cost=GET_LEVEL(ch)*50*mult;
      if(GET_GOLD(ch)<cost) {
        send_to_char("Doctor Naikrovek says 'Sorry, you dont have enough money.'\n\r",ch);
        return(TRUE);
      }
      cast_armor(35,mob,"",SPELL_TYPE_SPELL,ch,0);
      GET_GOLD(ch)-=cost;
      sprintf(buf,"Doctor Naikrovek says 'That will be %d coins please.'\n\r",cost);
      send_to_char(buf,ch);
      return(TRUE);
    }

    if (!strcmp(buf,"vitality")) {
      cost=GET_LEVEL(ch)*45*mult;
      if(GET_GOLD(ch)<cost) {
        send_to_char("Doctor Naikrovek says 'Sorry, you dont have enough money.'\n\r",ch);
        return(TRUE);
      }
      cast_vitality(35,mob,"",SPELL_TYPE_SPELL,ch,0);
      GET_GOLD(ch)-=cost;
      sprintf(buf,"Doctor Naikrovek says 'That will be %d coins please.'\n\r",cost);
      send_to_char(buf,ch);
      return(TRUE);
    }

    send_to_char("Doctor Naikrovek says 'You can't buy that.'\n\r",ch);
    return(TRUE);
  }
  return(FALSE);
}

int newbie_guardian(CHAR *mob,CHAR *ch, int cmd, char *arg) {
   CHAR *victim;

   for (victim = world[CHAR_REAL_ROOM(mob)].people; victim; victim = victim->next_in_room ) {
     if (number(0,2) == 0)
         break;
     }
   if (!victim)
     return FALSE;

   if (cmd) return FALSE;

   if ((victim == mob) || (GET_HIT(victim) == GET_MAX_HIT(victim)) ||
       (GET_LEVEL(victim) > 5) || IS_NPC(victim))
     return FALSE;

   cast_heal(35,mob,"",SPELL_TYPE_SPELL,victim,0);

   return FALSE;
 }

int newbie_warn(int room,CHAR *ch, int cmd, char *arg) {
  char buf[MAX_STRING_LENGTH];

  if(cmd!=MSG_ENTER) return FALSE;
  if(!ch) return FALSE;
  if(IS_NPC(ch)) return FALSE;
  if(GET_LEVEL(ch)>5) return FALSE;
  if(ch->master) return FALSE;

  if(world[room].number==3053)
    sprintf(buf,"\n\r\n\rAs you step to the East gate of Midgaard\n\r");
  if(world[room].number==3052)
    sprintf(buf,"\n\r\n\rAs you step to the West gate of Midgaard\n\r");
  if(world[room].number==2072)
    sprintf(buf,"\n\r\n\rAs you step to the Long Stair\n\r");

  send_to_char(buf,ch);
  send_to_char("you realize you are outside the safer environment\n\r",ch);
  send_to_char("of Midgaard.  Perhaps you should turn back and gain\n\r",ch);
  send_to_char("a little more experience before exploring.\n\r\n\r",ch);
  return FALSE;
}

void show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode,int num);
void list_donation_to_char(OBJ *list,CHAR *ch, char *buf) {
  OBJ *i,*back;
  bool c,f,found=FALSE;
  int x=0;

  c=TRUE;
  i=list;
  if(i) {
    while(c) {
      if(CAN_SEE_OBJ(ch,i) && i->obj_flags.type_flag!=ITEM_BOARD &&
         (isname(buf,OBJ_NAME(i)) || vault_filter(i,buf))) {
        found=TRUE;
        f=FALSE;x=1;
        back=i;
        i=i->next_content;
        if(i) f=TRUE;
        else c=FALSE;
        while(f) {
          if((i->item_number==back->item_number)
             &&(IS_OBJ_STAT(i,ITEM_CLONE)==IS_OBJ_STAT(back,ITEM_CLONE))
             &&(IS_OBJ_STAT(i,ITEM_INVISIBLE)==IS_OBJ_STAT(back,ITEM_INVISIBLE))
             &&(!str_cmp(OBJ_NAME(i),OBJ_NAME(back)))
             &&(!str_cmp(OBJ_SHORT(i),OBJ_SHORT(back)))
             &&(!str_cmp(OBJ_DESCRIPTION(i),OBJ_DESCRIPTION(back))))
            {
            x++;
            i=i->next_content;
            if(!i) {f=FALSE;c=FALSE;}
          }
          else f=FALSE;
        }
        show_obj_to_char(back,ch,0,x);
      }
      else   {
        i=i->next_content;
        if(!i) c=FALSE;
      }
    }
  }
  if (!found) send_to_char("Sorry, nothing here like that.\n\r", ch);
}

void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode,bool show);

/* Listing objs in the room is blocked by act.informative.c */
int donation(int room,CHAR *ch, int cmd, char *arg) {
  OBJ *temp_obj = NULL;
  OBJ *next_obj = NULL;
  char buf[MSL];
  char arg1[MIL];
  char arg2[MIL];
  char allbuf[MIL];
  int number = 0;
  int counter = 0;
  int found = FALSE;

  if(cmd == CMD_IDENTIFY)
  {
    number_argument_interpreter(arg, &number, arg1, arg2);
    // arg1 is a number
    if (!*arg1)
    {
      send_to_char("What item would you like to identify?\n\r", ch);
      return TRUE;
    }
    else
    {
      // arg1 is 'all' or was 'all.' (and is now 'all')
      if (!str_cmp(arg1, "all"))
      {
        send_to_char("You can't identify 'all' of something.\n\r", ch);
        return TRUE;
      }
      else
      {
        // 'identify item' or 'identify 1 item'
        if (number == 1)
        {
          temp_obj = get_obj_in_list_vis(ch, arg1, world[real_room(3084)].contents);
          if (temp_obj)
          {
            found = TRUE;
          }
        }
        else // 'identify x.arg1'
        {
          strcpy(allbuf, arg1);
          for (temp_obj = world[real_room(3084)].contents; temp_obj && counter <= number; temp_obj = next_obj)
          {
            next_obj = temp_obj->next_content;
            if (isname(arg1, OBJ_NAME(temp_obj)))
            {
              counter++;
              if (counter == number)
              {
                found = TRUE;
                break;
              }
            }
          }
        }
      }
    }
    if (found && temp_obj)
    {
      if(GET_LEVEL(ch) > 30 )
      {
        send_to_char("It will cost 3500 coins...\n\r", ch);
        if(!IS_IMPLEMENTOR(ch) && (3500 > GET_GOLD(ch)))
        {
          send_to_char("You do not have enough coins.\n\r", ch);
          return TRUE;
        } else {
          if(!IS_IMPLEMENTOR(ch)) GET_GOLD(ch) -= 3500;
        }
      }
      spell_identify(50, ch, 0, temp_obj);
    } else {
      send_to_char("You can't seem to find that item.\n\r", ch);
    }
    return TRUE;
  }

  if(cmd!=CMD_LIST) return FALSE;
  if(!ch) return FALSE;
  one_argument(arg,buf);
  if(!*buf) {
    send_to_char("\
Usage: list <all|keyword> or\n\r\
     light, finger, neck, body, head, legs, feet, hands,\n\r\
     arms, shield, about, waist, wrist, wield, hold, throw,\n\r\
     magic-user, cleric, thief, warrior, ninja, nomad, paladin,\n\r\
     anti-paladin, avatar, bard, commando, evil, neutral, good,\n\r\
     container, drink, boat, lockpick, recipe, key, potion,\n\r\
     scroll, wand, staff.\n\r",ch);
    send_to_char("You can also learn the stats of an item: identify <keyword>\n\r",ch);
    return TRUE;
  }

  if(is_abbrev(buf,"all")) {
    list_obj_to_char(world[CHAR_REAL_ROOM(ch)].contents, ch, 0,FALSE);
    return TRUE;
  }

  list_donation_to_char(world[CHAR_REAL_ROOM(ch)].contents,ch,buf);
  return TRUE;
}

int hide_buyer(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  OBJ *obj;
  extern OBJ *read_object(int num, int type);
  char name[MSL],buf[MSL];
  int i;

  switch (cmd) {
    case CMD_DROP:
      do_drop(ch, arg, cmd);
      for(obj = world[CHAR_REAL_ROOM(ch)].contents; obj ;
        obj = world[CHAR_REAL_ROOM(ch)].contents) {
        if(obj->obj_flags.type_flag==ITEM_SKIN) {
          sprintf(buf,"say A freebie! I'll take that!");
          command_interpreter(mob,buf);
          obj_from_room(obj);
          obj_to_char(obj,mob);
        }  else
          return TRUE;
      }
      return TRUE;
      break;
    case CMD_LIST:
    case CMD_BUY:
      act("$N tells you 'I refuse to sell any of my precious skins!'",0,ch,0,mob,TO_CHAR);
      return TRUE;
      break;
    case CMD_SELL:
      if(!*arg) {
        act("$N tells you 'Sell what?'",0,ch,0,mob,TO_CHAR);
        return(TRUE);
      }

      one_argument(arg,name);
      string_to_lower(name);
      obj=get_obj_in_list_vis(ch,name,ch->carrying);
      if(!obj) {
        act("$N tells you 'You don't seem to have that item.'",0,ch,0,mob,TO_CHAR);
        return(TRUE);
      }

      if(obj->obj_flags.type_flag!=ITEM_SKIN) {
        act("$N tells you 'That isn't a skin!'",0,ch,0,mob,TO_CHAR);
        return(TRUE);
      }

      i=obj->obj_flags.cost;
      if(i<1) {
        act("$N tells you 'That skin is worthless!'",0,ch,0,mob,TO_CHAR);
      }
      else {
        act("$n gives $p to $N.",FALSE,ch,obj,mob, TO_ROOM);
        act("$N gives you $P.",FALSE,ch,obj,mob,TO_VICT);
        act("You give $p to $N.",FALSE,ch,obj,mob, TO_CHAR);
        sprintf(buf,"$N gives you %d coins.",i);
        act(buf,0,ch,0,mob,TO_CHAR);
        GET_GOLD(ch)+=i;
        obj_from_char(obj);
        extract_obj(obj);
        save_char(ch, NOWHERE);
      }
      return TRUE;
      break;
    case CMD_OFFER:
      if(!*arg) {
        act("$N tells you 'You want an offer for what?'",0,ch,0,mob,TO_CHAR);
        return(TRUE);
      }

      one_argument(arg,name);
      string_to_lower(name);
      obj=get_obj_in_list_vis(ch,name,ch->carrying);
      if(!obj) {
        act("$N tells you 'You don't seem to have that item.'",0,ch,0,mob,TO_CHAR);
        return(TRUE);
      }

      if(obj->obj_flags.type_flag!=ITEM_SKIN) {
        act("$N tells you 'That isn't a skin!'",0,ch,0,mob,TO_CHAR);
        return(TRUE);
      }

      i=obj->obj_flags.cost;
      if(i<1) {
        act("$N tells you 'That skin is worthless!'",0,ch,0,mob,TO_CHAR);
      }
      else {
        sprintf(buf,"$N tells you '%d coins - no more!'",i);
        act(buf,0,ch,0,mob,TO_CHAR);
      }
      return TRUE;
      break;
    default:
    break;
  }
  return(FALSE);
}

#define GEAR_GYSPY  3144
#define GEAR_GYSPY_POT 3144

int gear_gypsy(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
    OBJ *pot, *recipe, *obj, *next_obj;
    int newObj, i, timerReset, r_number;
    int ingredients[] = {-1, -1, -1};
    bool found[] = {FALSE, FALSE, FALSE};
    OBJ* ingObj[] = {NULL, NULL, NULL};
    char buf[MIL];
    CHAR *vict, *next_vict;
    struct affected_type_5 af;
    char cough[] = "cough\0";
    bool success = FALSE;
    bool bRepair = FALSE;

    timerReset = 0;
    if(cmd==MSG_TICK)
    {
        if(chance(50))
        {
            switch(number(0,5))
            {
                case(0):
                    do_say(mob,"No adlibbing, you'll need a recipe.",CMD_SAY);
                    break;
                case(1):
                    do_say(mob,"Stir the pot! You must stir the pot!",CMD_SAY);
                    break;
                case(2):
                    do_say(mob,"Poor squirrels, they didn't fare so well after the last botched recipe.",CMD_SAY);
                    do_social(mob,cough,CMD_SOCIAL);
                    break;
                case(3):
                    do_say(mob,"Plop plop plop, stir stir stir...",CMD_SAY);
                    break;
                case(4):
                    do_say(mob,"Ears of bat, tears of cat, and a meatball - yum, stew!",CMD_SAY);
                    break;
                case(5):
                    do_say(mob,"I could cook for a kingdom with this pot, big enough for suits of armor as an ingredient I tell you!",CMD_SAY);
                    break;
                default:
                    break;
            }
        }
        return FALSE;
    }

    recipe = NULL;
    pot = get_obj_room(GEAR_GYSPY_POT, CHAR_VIRTUAL_ROOM(mob));
    if (pot && cmd == CMD_UNKNOWN)
    {
        arg = one_argument(arg, buf);
        if (*buf && is_abbrev(buf, "stir") && !is_abbrev(buf, "south"))
        {
            arg = one_argument(arg, buf);
            if (*buf && isname(buf, OBJ_NAME(pot)))
            {
                for (obj = pot->contains; obj; obj = next_obj)
                {
                    next_obj = obj->next_content;

                    if (OBJ_TYPE(obj) == ITEM_RECIPE)
                    {
                        recipe = obj;
                        newObj = recipe->obj_flags.value[0];
                        ingredients[0] = recipe->obj_flags.value[1];
                        ingredients[1] = recipe->obj_flags.value[2];
                        ingredients[2] = recipe->obj_flags.value[3];
                        //if the resulting object is the same as any of the ingredients, it's a repair
                        if (newObj == ingredients[0] ||
                            newObj == ingredients[1] ||
                            newObj == ingredients[2])
                        {
                            r_number = real_object(newObj);
                            if (r_number > 0)
                            {
                                bRepair = TRUE;
                                timerReset = obj_proto_table[r_number].obj_flags.timer;
                            }
                        }
                        break;
                    }
                }
                if (recipe)
                {
                    for (i = 0; i < 3; ++i)
                    {
                        if (ingredients[i] < 0)
                        {
                            found[i] = TRUE;
                        }
                        else
                        {
                            for (obj = pot->contains; obj; obj = next_obj)
                            {
                                next_obj = obj->next_content;

                                if (obj->item_number_v == ingredients[i])
                                {
                                    ingObj[i] = obj;
                                    found[i] = TRUE;
                                }
                            }
                        }
                    }
                    if (found[0] && found[1] && found[2])
                    { // success
                        success = TRUE;
                        /* they've got all the objects */
                        if (!bRepair)
                        {
                            next_obj = read_object(newObj, VIRTUAL);
                            if (next_obj)
                            {
    
                                obj_from_obj(recipe);
                                extract_obj(recipe);
                                if (ingObj[0])
                                {
                                    obj_from_obj(ingObj[0]);
                                    extract_obj(ingObj[0]);
                                }
                                if (ingObj[1])
                                {
                                    obj_from_obj(ingObj[1]);
                                    extract_obj(ingObj[1]);
                                }
                                if (ingObj[2])
                                {
                                    obj_from_obj(ingObj[2]);
                                    extract_obj(ingObj[2]);
                                }
    
                                obj_to_obj(next_obj, pot);
                                act("The pot's contents bubble up furiously then disappear in a puff of smoke.",0,mob,0,ch,TO_ROOM);
                                act("$n gawks at you, shocked by your apparent skill in alchemy.",0,mob,0,ch,TO_VICT);
                                act("$n gawks at $N, shocked by $S skill in alchemy.",0,mob,0,ch,TO_NOTVICT);
                                act("You stare wonderingly at $N, amazed by the alchemy $E just performed.",0,mob,0,ch,TO_CHAR);
                            }
                        }
                        else //Repairing
                        {
                            obj_from_obj(recipe);
                            extract_obj(recipe);
                            if (ingObj[0])
                            {
                                if (newObj != ingredients[0])
                                {
                                    obj_from_obj(ingObj[0]);
                                    extract_obj(ingObj[0]);
                                }
                                else
                                {
                                    ingObj[0]->obj_flags.timer = timerReset;
                                }
                            }
                            if (ingObj[1])
                            {
                                if (newObj != ingredients[1])
                                {
                                    obj_from_obj(ingObj[1]);
                                    extract_obj(ingObj[1]);
                                }
                                else
                                {
                                    ingObj[1]->obj_flags.timer = timerReset;
                                }
                            }
                            if (ingObj[2])
                            {
                                if (newObj != ingredients[2])
                                {
                                    obj_from_obj(ingObj[2]);
                                    extract_obj(ingObj[2]);
                                }
                                else
                                {
                                    ingObj[2]->obj_flags.timer = timerReset;
                                }
                            }
                            act("The pot's contents bubble up furiously then disappear in a puff of smoke, leaving a fresh looking item.",0,mob,0,ch,TO_ROOM);
                            act("$n gawks at you, shocked by your apparent skill in alchemy.",0,mob,0,ch,TO_VICT);
                            act("$n gawks at $N, shocked by $S skill in alchemy.",0,mob,0,ch,TO_NOTVICT);
                            act("You stare wonderingly at $N, amazed by the alchemy $E just performed.",0,mob,0,ch,TO_CHAR);
                        }
                    }
                }
                if (!success)
                { // failure
                    act("The pot's contents bubble up furiously releasing an acrid cloud of poisonous funk.",0,mob,0,ch,TO_ROOM);
                    act("$n laughs derisively at you for your terrible cooking.",0,mob,0,ch,TO_VICT);
                    act("$n laughs derisively at $N as $S cooking befouls the air.",0,mob,0,ch,TO_NOTVICT);
                    act("You point your finger and laugh at $N for $S terrible cooking.",0,mob,0,ch,TO_CHAR);
                    for (vict = world[CHAR_REAL_ROOM(mob)].people; vict; vict = next_vict)
                    {
                        next_vict = vict->next_in_room;
                        if (vict == mob) continue;
                        if (!IS_MORTAL(vict)) continue;
                        if (!affected_by_spell(vict, SPELL_POISON))
                        {
                            af.type = SPELL_POISON;
                            af.location = APPLY_STR;
                            af.modifier = -2;
                            af.duration = 1;
                            af.bitvector = AFF_POISON;
                            af.bitvector2 = 0;
                            affect_to_char(vict, &af);
                        }
                        if (!affected_by_spell(vict, SPELL_BLINDNESS))
                        {
                            af.type = SPELL_BLINDNESS;
                            af.location = APPLY_HITROLL;
                            af.modifier = -2;
                            af.duration = 1;
                            af.bitvector = AFF_BLIND;
                            af.bitvector2 = 0;
                            affect_to_char(vict, &af);
                            af.location = APPLY_ARMOR;
                            af.modifier = 40;
                            affect_to_char(vict, &af);
                        }
                        if (!affected_by_spell(vict, SPELL_CLOUD_CONFUSION))
                        {
                            af.type = SPELL_CLOUD_CONFUSION;
                            af.location = APPLY_HITROLL;
                            af.modifier = -8;
                            af.duration = 1;
                            af.bitvector = 0;
                            af.bitvector2 = 0;
                            affect_to_char(vict, &af);
                        }
                    }
                    act("You gag on the cloud of poisonous funk.",0,mob,0,vict,TO_VICT);
                }
                return TRUE;
            }
            else
            {
                send_to_char("Stir what?\n\r", ch);
                return TRUE;
            }
        }
        else
            return FALSE;
    }
    return FALSE;
}


int saga_prestige(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  const char * const prestige_perk_descriptions[] = {
    "1x increased remort experience multiplier (maximum of 25x).",                                    // Prestige Perk 1
    "5% chance for free rent, determined after logging in.",                                          // Prestige Perk 2
    "1% increased chance for half-price metas.",                                                      // Prestige Perk 3
    "10% experience and gold discount on rank purchases.",                                            // Prestige Perk 4
    "10% discount on rent cost.",                                                                     // Prestige Perk 5
    "1x increased maximum death experience multiplier.(3x instead of 2x)",                                              // Prestige Perk 6
    "50% chance to avoid entering a death trap or hazard, setting your movement to zero.",            // Prestige Perk 7
    "5% increased hit point, mana, and movement regeneration rate.",                                  // Prestige Perk 8
    "Identify command now available for 5,000 coins; acts as if you recited a scroll.",               // Prestige Perk 9
    "5% discount on bribes at the metaphysician.",                                                    // Prestige Perk 10
    "1% less permanent experience loss upon death.",                                                  // Prestige Perk 11
    "10% chance that a 1 point token is worth 2 subclass points.",                                    // Prestige Perk 12
    "10% quest point and subclass point discount on remort cost.",                                    // Prestige Perk 13
    "10% increase in number of items that can be carried.",                                           // Prestige Perk 14
    "Instant passage to Daimyo on a new airship located above the Midgaard docks.",                   // Prestige Perk 15
    "5% discount on items purchased from shops.",                                                     // Prestige Perk 16
    "10% chance to maintain existing rank upon death.",                                               // Prestige Perk 17
    "10% discount on vault storage costs.",                                                           // Prestige Perk 18
    "2% increased chance for double-point auto quest.",                                               // Prestige Perk 19
    "10% increase in amount of weight that can be carried.",                                          // Prestige Perk 20
    "Home command now available for 20,000 coins; acts as if you recited a scroll.",                  // Prestige Perk 21
    "10% chance per tick to maintain existing decay level on worn/held items.",                       // Prestige Perk 22
    "1 less movement point required to traverse all sector types (minimum of 1 point).",              // Prestige Perk 23
    "2% chance when purchasing a normal meta to receive a bribe meta instead.",                      // Prestige Perk 24
    "5 point increase to mana regen cap while engaged in combat.",                                    // Prestige Perk 25
    "You no longer require food or drink, and you can now quaff two potions per tick.",               // Prestige Perk 26
  };
  const int EXP_REQUIRED = 100000000;
  const int GOLD_REQUIRED = 20000000;

  if (!mob || !ch || IS_NPC(ch)) return FALSE;

  if (cmd == CMD_LIST) {
    printf_to_char(ch, "\
Your Prestige: %d\n\r\n\r\
A + symbol to the left of the perk description indicates that you've earned its benefits.\n\r\n\r\
Lvl   Perk Description\n\r\
---   ----------------\n\r", GET_PRESTIGE(ch));

    for (int i = 0; i < NUMELEMS(prestige_perk_descriptions); i++) {
      int perk_level = 5 + (10 * i);
      printf_to_char(ch, "%3d %s %s\n\r", perk_level, (GET_PRESTIGE(ch) >= perk_level) ? "+" : " ", prestige_perk_descriptions[i]);
    }

    return TRUE;
  }

  if (cmd == CMD_UNKNOWN) {
    if (IS_NPC(ch)) return FALSE;

    char buf[MIL];

    arg = one_argument(arg, buf);

    if (!strcmp(buf, "earn")) {
      arg = one_argument(arg, buf);

      if (strcmp(buf, "prestige")) {
        snprintf(buf, sizeof(buf), "$n tells you 'You've gotta type the words 'earn prestige' if you want me to recognize your deeds.'");
        act(buf, FALSE, mob, 0, ch, TO_VICT);

        return TRUE;
      }

      if (IS_IMMORTAL(ch)) {
        send_to_char("Consult with a SUP+ about earning Prestige as an immortal.\n\r", ch);

        return TRUE;
      }

      if (GET_PRESTIGE(ch) >= PRESTIGE_MAX) {
        snprintf(buf, sizeof(buf), "$n tells you 'You've already attained the highest level of prestige!'");
        act(buf, FALSE, mob, 0, ch, TO_VICT);

        return TRUE;
      }

      bool enough_exp = (GET_EXP(ch) >= EXP_REQUIRED);
      bool enough_gold = (GET_GOLD(ch) >= GOLD_REQUIRED);

      if (!enough_exp && !enough_gold) {
        snprintf(buf, sizeof(buf), "$n tells you 'You need at least %d experience points and %d gold coins to earn the next level of prestige.'", EXP_REQUIRED, GOLD_REQUIRED);
        act(buf, FALSE, mob, 0, ch, TO_VICT);

        return TRUE;
      }
      else if (!enough_exp) {
        snprintf(buf, sizeof(buf), "$n tells you 'You need at least %d experience points to earn the next level of prestige.'", EXP_REQUIRED);
        act(buf, FALSE, mob, 0, ch, TO_VICT);

        return TRUE;
      }
      else if (!enough_gold) {
        snprintf(buf, sizeof(buf), "$n tells you 'You need at least %d gold coins to earn the next level of prestige.'", GOLD_REQUIRED);
        act(buf, FALSE, mob, 0, ch, TO_VICT);

        return TRUE;
      }

      GET_EXP(ch) -= EXP_REQUIRED;
      GET_GOLD(ch) -= GOLD_REQUIRED;

      GET_PRESTIGE(ch) += 1;

      ch->points.max_hit += PRESTIGE_HIT_GAIN;

      if ((GET_CLASS(ch) == CLASS_THIEF) || (GET_CLASS(ch) == CLASS_WARRIOR) || (GET_CLASS(ch) == CLASS_NOMAD)) {
        ch->points.max_hit += PRESTIGE_MANA_GAIN;
      }
      else {
        ch->points.max_mana += PRESTIGE_MANA_GAIN;
      }

      snprintf(buf, sizeof(buf), "$n tells you 'Congratulations %s, your prestige has increased!  You've earned quite the reputation!'", GET_NAME(ch) ? GET_NAME(ch) : "(null)");
      act(buf, FALSE, mob, 0, ch, TO_VICT);

      affect_total(ch);

      save_char(ch, NOWHERE);

      if (GET_PRESTIGE(ch) == PRESTIGE_MAX) {
        send_to_char("Amazing! You've joined the ranks of the most prestigious adventurers in all the land!\n\r", ch);
        snprintf(buf, sizeof(buf), "%s has joined the ranks of the most prestigious adventurers in all the land!\n\r", GET_NAME(ch) ? GET_NAME(ch) : "(null)");
        send_to_world_except(buf, ch);
      }

      return TRUE;
    }

    return FALSE;
  }

  return FALSE;
}

int wesley_zeppelin(CHAR *mob, CHAR *ch, int cmd, char *arg) {
  const char *vip_msg[] = {
    "says",
    "states",
    "remarks",
    "exclaims",
    "chimes",
  };

  const char *rif_msg[] = {
    "says",
    "quips",
    "mutters",
    "grunts",
    "scoffs",
  };

  const int dest_room = 500;

  char buf[MIL];

  if (cmd == MSG_MOBACT) {
    if (!chance(20) || CHAOSMODE) return FALSE;

    int num_vip = 0, num_all = 0;

    for (CHAR *temp_ch = world[CHAR_REAL_ROOM(mob)].people; temp_ch; temp_ch = temp_ch->next_in_room) {
      if (!IS_MORTAL(temp_ch)) continue;

      if (GET_PRESTIGE_PERK(temp_ch) >= 15) num_vip++;

      num_all++;
    }

    if (num_vip) {
      const char *msg = vip_msg[number(0, NUMELEMS(vip_msg) - 1)];

      switch (number(1, 3)) {
      case 1:
        snprintf(buf, sizeof(buf), "$n %s 'A VIP!  Well met!  We are prepared to take off at any moment.  Just say the word 'ready', and off we go!'", msg);
        act(buf, FALSE, mob, 0, 0, TO_ROOM);
        break;
      case 2:
        snprintf(buf, sizeof(buf), "$n %s 'Ah, an esteemed member!  Good %s to you.  No sooner shall the word 'ready' leave your lips and you'll be on your way!'", msg, IS_DAY ? "day" : "evening");
        act(buf, FALSE, mob, 0, 0, TO_ROOM);
        break;
      case 3:
        snprintf(buf, sizeof(buf), "$n %s 'Make yourself comfortable aboard our magnificent vessel.  We are 'ready' and willing to depart at your liesure.'", msg);
        act(buf, FALSE, mob, 0, 0, TO_ROOM);
        break;
      }
    }
    else if (num_all) {
      const char *msg = rif_msg[number(0, NUMELEMS(rif_msg) - 1)];

      switch (number(1, 3)) {
      case 1:
        snprintf(buf, sizeof(buf), "$n %s 'Who is this riffraff?  Begone.  There is nothing for you here.'", msg);
        act(buf, FALSE, mob, 0, 0, TO_ROOM);
        break;
      case 2:
        snprintf(buf, sizeof(buf), "$n %s 'I'm afraid access to our fine zeppelins is reserved for a more... exclusive clientele.'", msg);
        act(buf, FALSE, mob, 0, 0, TO_ROOM);
        break;
      case 3:
        snprintf(buf, sizeof(buf), "$n %s 'I don't believe the expression 'when pigs fly' was referring to today...'", msg);
        act(buf, FALSE, mob, 0, 0, TO_ROOM);
        break;
      }
    }

    return FALSE;
  }

  if (cmd == CMD_SAY) {
    arg = one_argument(arg, buf);

    if (strcmp(buf, "ready")) return FALSE;

    if (GET_PRESTIGE_PERK(ch) >= 15) {
      act("$n says 'Your wish is our command.  Enjoy your flight!'", FALSE, mob, 0, 0, TO_ROOM);

      for (CHAR *temp_ch = world[CHAR_REAL_ROOM(mob)].people, *next_ch; temp_ch; temp_ch = next_ch) {
        next_ch = temp_ch->next_in_room;

        if (!IS_MORTAL(temp_ch)) continue;

        act("You embark upon the sleek metallic zeppelin and are whisked away to your destination.", FALSE, temp_ch, 0, 0, TO_CHAR);
        act("$n embarks upon a sleek metallic zeppelin and is whisked away to $s destination.", TRUE, temp_ch, 0, 0, TO_ROOM);
        
        char_from_room(temp_ch);
        char_to_room(temp_ch, real_room(dest_room));

        act("You disembark from the magnificent zeppelin moments after it swoops from the sky to drop off its passengers.", FALSE, temp_ch, 0, 0, TO_CHAR);
        act("$n disembarks from a magnificent zeppelin moments after it swoops from the sky to drop off its passengers.", TRUE, temp_ch, 0, 0, TO_ROOM);

        do_look(temp_ch, "", CMD_LOOK);
      }
    }
    else {
      const char *msg = rif_msg[number(0, NUMELEMS(rif_msg) - 1)];

      snprintf(buf, sizeof(buf), "$n %s 'Our services are for VIPs and their guests only.  Please take your business elsewhere.'", msg);
      act(buf, FALSE, mob, 0, 0, TO_ROOM);
    }

    send_to_room("The zeppelin lifts high into the air and is gone in a moment,\n\rleaving behind a wake of powerful energy that trails away into the clouds.\n\r", real_room(dest_room));

    return FALSE;
  }

  return FALSE;
}

 /**********************************************************************\
 |* End Of the Special procedures for Midgaard                         *|
 \**********************************************************************/

void assign_midgaard (void) {

  int spec_auctioneer(CHAR *mob, CHAR *ch, int cmd, char *arg); /* auction.c */
  int receptionist(CHAR *mob, CHAR *ch, int cmd, char *arg); /* reception.c */

  assign_obj(2999,teller);
/*  assign_obj(2998,insurance);*/
  assign_obj(2997,flower);
  assign_obj(2995,membership);

  assign_mob(7,jail);
  assign_mob(8,teacher);
  assign_mob(10,postoffice);
  assign_mob(3095,newbie_hospital);

  assign_mob(3005,receptionist);
  assign_mob(3010,spec_auctioneer);
  assign_mob(3011,jeweler);

  assign_mob(2998,club_guard);

  assign_mob(3003,exploded);
  assign_mob(5 ,    exploded);

  assign_mob(3000,exploded);

  assign_mob(3060,cityguard);
  assign_mob(3067,cityguard);
  assign_mob(3068,cityguard);
  assign_mob(8001,cityguard);
  assign_mob(3061,janitor);
  assign_mob(3062,fido);
  assign_mob(DOG_CATCHER, dog_catcher);
  assign_mob(3066,fido);
  assign_mob(3018,guild);
  assign_mob(3019,guild);
  assign_mob(3020,guild);
  assign_mob(3021,guild);
  assign_mob(3022,guild);
  assign_mob(3023,guild);
  assign_mob(3029,guild);
  assign_mob(3031,guild);
  assign_mob(3033,guild);
  assign_mob(3035,guild);
  assign_mob(3016,guild_guard);
  assign_mob(3017,guild_guard);
  assign_mob(3024,guild_guard);
  assign_mob(3025,guild_guard);
  assign_mob(3026,guild_guard);
  assign_mob(3027,guild_guard);
  assign_mob(3028,guild_guard);
  assign_mob(3030,guild_guard);
  assign_mob(3032,guild_guard);
  assign_mob(3034,guild_guard);
  assign_mob(3143,mayor);
  assign_mob(8011,restguard);
  assign_mob(3069, do_vault);
  assign_mob(3504, newbie_guardian);
  assign_mob(3096, hide_buyer);
  assign_room(3030 , dump);
  assign_room(3084 , donation);
  assign_room(3031, pet_shops);
  assign_room(3052, newbie_warn);
  assign_room(3053, newbie_warn);
  assign_room(2072, newbie_warn);
  assign_room(10,   jail_room);
  assign_mob(GEAR_GYSPY, gear_gypsy);
  assign_mob(3040, saga_prestige);
  assign_mob(3110, wesley_zeppelin);
}
