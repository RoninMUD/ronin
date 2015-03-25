/*
**  file: spec_procs.c , Special module.                   Part of DIKUMUD
**  Usage: Procedures handling special procedures for object/room/mobile
**  Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
*/

/*
$Author: ronin $
$Date: 2005/04/27 17:13:30 $
$Header: /home/ronin/cvs/ronin/spec.midgaard.c,v 2.6 2005/04/27 17:13:30 ronin Exp $
$Id: spec.midgaard.c,v 2.6 2005/04/27 17:13:30 ronin Exp $
$Name:  $
$Log: spec.midgaard.c,v $
Revision 2.6  2005/04/27 17:13:30  ronin
Minor changes needed to compile on Slackware 10 for the new machine.

Revision 2.5  2005/01/21 14:55:29  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.4  2004/11/17 19:21:52  void
Added Nomad Skill Cover (1/2 Damage -10 Hitroll)

Revision 2.3  2004/08/18 17:49:17  void
Fixed some problems with Quad.

Revision 2.2  2004/08/17 15:37:49  void
Added Warrior Skill Quad

Revision 2.1  2004/03/04 17:23:58  ronin
Addition of object file version 2 which includes 8 ownerid fields
for addition of some objects only being able to be used by those
owners.

Revision 2.0.0.1  2004/02/05 16:11:01  ronin
Reinitialization of cvs archives


Revision 20-Jan-04 Ranger
Addition of senders email address if present to postcard
forwarding by email.

Revision 20-Dec-03 Ranger
Addition of postcard forwarding by email.

Revision 05/01/03 Liner
Addition of log for stuff dropped in Dump.

Revision 7-Mar-03 Ranger
Upped warrior practice level to very good.

Revision 1.4  2003/06/01  16:35:06  ronin
Revision of DROP for hide_buyer so only skins are picked up.

Revision 1.3  2002/03/31 16:35:06  ronin
Added braces to remove ambiguous else warning.

Revision 1.2  2002/03/31 07:42:15  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
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
#include "fight.h"
#include "act.h"
#include "reception.h"
#include "spec_assign.h"
#include "time.h"
#include "subclass.h"
#include "enchant.h"

char *Month[12]= {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

/*   external vars  */
extern int CHAOSMODE;
/*
#define USE_MANA(ch, sn)                            \
  MAX(spell_info[sn].min_usesmana, 100/(2+GET_LEVEL(ch)-SPELL_LEVEL(ch,sn)))
  */

int USE_MANA(CHAR *ch, int sn);

extern CHAR *character_list;
extern struct descriptor_data *descriptor_list;
extern struct time_info_data time_info;
extern struct spell_info_type spell_info[MAX_SPL_LIST];
extern char *spells[];
extern struct int_app_type int_app[26];
extern char *w_skills[];
extern char *t_skills[];
extern char *n_skills[];
extern char *nomad_skills[];
extern char *pal_skills[];
extern char *cl_skills[];
extern char *antipal_skills[];
extern char *avatar_skills[];
extern char *bard_skills[];
extern char *commando_skills[];
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

int MAX_PRAC(CHAR *ch)
{
  switch(GET_CLASS(ch))
  {
    case CLASS_MAGIC_USER:
    case CLASS_CLERIC:
      return 95;
      break;

    case CLASS_AVATAR:
      return 100;
      break;

    default:
      return 85;
      break;
  }

  return 0;
}

void list_spells_to_prac(CHAR *ch, int listall)
{
  struct string_block sb;
  int bonus = 0;
  char buf[MIL];
  int i = 0;

  init_string_block(&sb);

  if (enchanted_by_type(ch, ENCHANT_SHOGUN)) bonus = 5;
  else if (enchanted_by_type(ch, ENCHANT_LORDLADY)) bonus = 5;
  else if (enchanted_by_type(ch, ENCHANT_DARKLORDLADY)) bonus = 5;

  sprintf(buf, "\
`nSpell Name                      `kHow Well      `jMana Cost\n\r\
`n------------------------------- `k------------- `j---------`q\n\r");
  append_to_string_block(&sb, buf);

  for (i = 0; *spells[i] != '\n'; i++)
  {
    if (!spell_info[i + 1].spell_pointer) continue;
    else if ((ch->skills[i + 1].learned >= MAX_PRAC(ch)) && !listall) continue;
    else if (SPELL_LEVEL(ch, i + 1) > GET_LEVEL(ch)) continue;
    else if (!check_sc_access(ch, i + 1)) continue;
    else
    {
      sprintf(buf, "`n%-30s `k%-14s `j[%3d]`q\n\r",
        spells[i], how_good(ch->skills[i + 1].learned + bonus), USE_MANA(ch, i + 1));
      append_to_string_block(&sb, buf);
    }
  }

  page_string_block(&sb, ch);

  destroy_string_block(&sb);
}

void list_skills_to_prac(CHAR *ch)
{
  int i = 0;
  int done = FALSE;
  int number = 0;
  char buf[MIL];

  send_to_char("\
`nSkill Name                     `kHow Well     \n\r\
`n------------------------------ `k-------------`q\n\r", ch);

  for (i = 0; !done; i++)
  {
    switch (GET_CLASS(ch))
    {
      case CLASS_MAGIC_USER:
        done = TRUE;
      break;

      case CLASS_CLERIC:
        if (*cl_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(cl_skills[i], 0, strlen(cl_skills[i]), spells, TRUE);

          if (number == 0) continue;
          else if (!check_sc_access(ch, number)) continue;
          else if ((number == SKILL_BASH) && (GET_LEVEL(ch) < 35)) continue;
          else
          {
            sprintf(buf, "`n%-30s `k%-13s`q\n\r", cl_skills[i], how_good(ch->skills[number].learned));
            send_to_char(buf, ch);
          }
        }
        break;

      case CLASS_THIEF:
        if (*t_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(t_skills[i], 0, strlen(t_skills[i]), spells, TRUE);

          if (number == 0) continue;
          else if (!check_sc_access(ch, number)) continue;
          else if ((number == SKILL_SCAN) && (GET_LEVEL(ch) < 40)) continue;
          else if ((number == SKILL_TWIST) && (GET_LEVEL(ch) < 45)) continue;
          else if ((number == SKILL_CUNNING) && (GET_LEVEL(ch) < 50)) continue;
          else
          {
            sprintf(buf, "`n%-30s `k%-13s`q\n\r", t_skills[i], how_good(ch->skills[number].learned));
            send_to_char(buf, ch);
          }
        }
        break;

      case CLASS_WARRIOR:
        if (*w_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(w_skills[i], 0, strlen(w_skills[i]), spells, TRUE);

          if (number == 0) continue;
          else if (!check_sc_access(ch, number)) continue;
          else if ((number == SKILL_TRIPLE) && (GET_LEVEL(ch) < 20)) continue;
          else if ((number == SKILL_QUAD) && (GET_LEVEL(ch) < 50)) continue;
          else
          {
            sprintf(buf, "`n%-30s `k%-13s`q\n\r", w_skills[i], how_good(ch->skills[number].learned));
            send_to_char(buf, ch);
          }
        }
        break;

      case CLASS_NINJA:
        if (*n_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(n_skills[i], 0, strlen(n_skills[i]), spells, TRUE);

          if (number == 0) continue;
          else if (!check_sc_access(ch, number)) continue;
          else
          {
            sprintf(buf, "`n%-30s `k%-13s`q\n\r", n_skills[i], how_good(ch->skills[number].learned));
            send_to_char(buf, ch);
          }
        }
        break;

      case CLASS_NOMAD:
        if (*nomad_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(nomad_skills[i], 0, strlen(nomad_skills[i]), spells, TRUE);

          if (number == 0) continue;
          else if (!check_sc_access(ch, number)) continue;
          else if ((number == SKILL_DISEMBOWEL) && (GET_LEVEL(ch) < 20)) continue;
          else
          {
            sprintf(buf, "`n%-30s `k%-13s`q\n\r", nomad_skills[i], how_good(ch->skills[number].learned));
            send_to_char(buf, ch);
          }
        }
        break;

      case CLASS_PALADIN:
        if (*pal_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(pal_skills[i], 0, strlen(pal_skills[i]), spells, TRUE);

          if (number == 0) continue;
          else if (!check_sc_access(ch, number)) continue;
          else
          {
            sprintf(buf, "`n%-30s `k%-13s`q\n\r", pal_skills[i], how_good(ch->skills[number].learned));
            send_to_char(buf, ch);
          }
        }
        break;

      case CLASS_ANTI_PALADIN:
        if (*antipal_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(antipal_skills[i], 0, strlen(antipal_skills[i]), spells, TRUE);

          if (number == 0) continue;
          else if (!check_sc_access(ch, number)) continue;
          else
          {
            sprintf(buf, "`n%-30s `k%-13s`q\n\r", antipal_skills[i], how_good(ch->skills[number].learned));
            send_to_char(buf, ch);
          }
        }
        break;

      case CLASS_AVATAR:
        if (*avatar_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(avatar_skills[i], 0, strlen(avatar_skills[i]), spells, TRUE);

          if (number == 0) continue;
          else if (!check_sc_access(ch, number)) continue;
          else
          {
            sprintf(buf, "`n%-30s `k%-13s`q\n\r", avatar_skills[i], how_good(ch->skills[number].learned));
            send_to_char(buf, ch);
          }
        }
        break;

      case CLASS_BARD:
        if (*bard_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(bard_skills[i], 0, strlen(bard_skills[i]), spells, TRUE);

          if (number == 0) continue;
          else if (!check_sc_access(ch, number)) continue;
          else if ((number == SKILL_BACKFLIP) && (GET_LEVEL(ch) < 20)) continue;
          else
          {
            sprintf(buf, "`n%-30s `k%-13s`q\n\r", bard_skills[i], how_good(ch->skills[number].learned));
            send_to_char(buf, ch);
          }
        }
        break;

      case CLASS_COMMANDO:
        if (*commando_skills[i] == '\n') done = TRUE;
        else
        {
          number = old_search_block(commando_skills[i], 0, strlen(commando_skills[i]), spells, TRUE);

          if (number == 0) continue;
          else if (!check_sc_access(ch, number)) continue;
          else if ((number == SKILL_TRIPLE) && (GET_LEVEL(ch) < 20)) continue;
          else
          {
            sprintf(buf, "`n%-30s `k%-13s`q\n\r", commando_skills[i], how_good(ch->skills[number].learned));
            send_to_char(buf, ch);
          }
        }
        break;
    }
  }
}

int practice_spell(CHAR *ch, int number)
{
  int learned = 0;
  int max_prac = 0;

  /* Check for it being a skill. */
  if (!spell_info[number].spell_pointer) return FALSE;

  if (number == 0)
  {
    send_to_char("`iThat spell wasn't found.`q\n\r", ch);

    return TRUE;
  }
  else if (ch->specials.spells_to_learn <= 0)
  {
    send_to_char("`iYou do not seem to be able to practice now.`q\n\r", ch);

    return TRUE;
  }
  else if ((GET_LEVEL(ch) < SPELL_LEVEL(ch, number)) || !check_sc_access(ch, number))
  {
    send_to_char("`iYou do not know of this spell...`q\n\r", ch);

    return TRUE;
  }

  if (ch->specials.spells_to_learn <= 0)
  {
    send_to_char("`iYou do not seem to be able to practice now.`q\n\r", ch);

    return TRUE;
  }

  learned = ch->skills[number].learned;
  max_prac = MAX_PRAC(ch);

  if (learned >= max_prac)
  {
    send_to_char("`iYou are already learned in this area.`q\n\r", ch);

    return TRUE;
  }

  send_to_char("`iYou practice for a while...`q\n\r", ch);

  ch->specials.spells_to_learn--;

  learned += MAX(int_app[GET_INT(ch)].learn, 25);
  learned = MIN(learned, max_prac);

  ch->skills[number].learned = learned;

  if (ch->skills[number].learned >= max_prac)
  {
    send_to_char("`iYou are now learned in this area.`q\n\r", ch);
  }

  return TRUE;
}

void practice_skill(CHAR *ch, int number)
{
  int learned = 0;
  int max_prac = 0;

  if (number == 0 || number == 200)
  {
    send_to_char("`iThat skill wasn't found.`q\n\r", ch);
    return;
  }
  else if (ch->specials.spells_to_learn <= 0)
  {
    send_to_char("`iYou do not seem to be able to practice now.`q\n\r", ch);
    return;
  }
  else if (!check_sc_access(ch, number))
  {
    send_to_char("`iYou do not know of this skill...`q\n\r", ch);
    return;
  }

  learned = ch->skills[number].learned;
  max_prac = MAX_PRAC(ch);

  switch (number)
  {
    case SKILL_BACKSTAB:
      if (enchanted_by_type(ch, ENCHANT_MINION))
      {
        learned -= 5;
      }
      break;

    case SKILL_BASH:
      if (GET_CLASS(ch) == CLASS_CLERIC)
      {
        max_prac -= 20;
      }
      break;

    case SKILL_BLOCK:
      if (enchanted_by_type(ch, ENCHANT_SWASHBUCKLER) ||
          enchanted_by_type(ch, ENCHANT_JUSTICIAR))
      {
        learned -= 5;
      }
      break;

    case SKILL_DUAL:
      if (enchanted_by_type(ch, ENCHANT_KNIGHT) ||
          enchanted_by_type(ch, ENCHANT_PRIVATE))
      {
        learned -= 5;
      }
      break;

    case SKILL_CIRCLE:
      if (enchanted_by_type(ch, ENCHANT_ASSASSIN))
      {
        learned -= 5;
      }
      break;

    case SKILL_TRIPLE:
      if (enchanted_by_type(ch, ENCHANT_COMMANDER))
      {
        learned -= 5;
      }
      break;

    case SKILL_ASSAULT:
      if (enchanted_by_type(ch, ENCHANT_COMMODORE) ||
          enchanted_by_type(ch, ENCHANT_SHOGUN))
      {
        learned -= 5;
      }
      break;

    case SKILL_AMBUSH:
      if (enchanted_by_type(ch, ENCHANT_WANDERER))
      {
        learned -= 5;
      }
      break;

    case SKILL_DISEMBOWEL:
      if (enchanted_by_type(ch, ENCHANT_TAMER))
      {
        learned -= 5;
      }
      break;

    case SKILL_PUMMEL:
      if (enchanted_by_type(ch, ENCHANT_LORDLADY) ||
          enchanted_by_type(ch, ENCHANT_SHINOBI))
      {
        learned -= 5;
      }
      break;

    case SKILL_BACKFLIP:
      if (enchanted_by_type(ch, ENCHANT_CONDUCTOR))
      {
        learned -= 5;
      }
      break;
  }

  if (learned >= max_prac)
  {
    send_to_char("`iYou are already learned in this area.`q\n\r", ch);
    return;
  }

  send_to_char("`iYou practice for a while...`q\n\r", ch);

  ch->specials.spells_to_learn--;

  learned += MIN(int_app[GET_INT(ch)].learn, 18);
  learned = MIN(learned, max_prac);

  ch->skills[number].learned = learned;

  if (ch->skills[number].learned >= max_prac)
  {
    send_to_char("`iYou are now learned in this area.`q\n\r", ch);
  }
}

int quest_giver(CHAR *mob,CHAR *ch, int cmd, char *argument);
int check_sc_master(CHAR *ch,CHAR *mob);
int guild(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  int showSpells = FALSE;
  int showSkills = FALSE;
  int spell = -1;
  int skill = -1;
  int index = -1;
  OBJ *qcard = NULL;

  arg = skip_spaces(arg);

  if (cmd == CMD_AQUEST)
  {
    if (quest_giver(mob, ch, cmd, arg)) return TRUE;

    return FALSE;
  }

  /* Give back any questcards given to the mob, as players should use 'aquest complete' to get credit. */
  if (cmd == MSG_GAVE_OBJ)
  {
    if (!isname("questcard", arg)) return FALSE;

    qcard = get_obj_in_list_vis(mob, "questcard", mob->carrying);

    if (qcard && (V_OBJ(qcard) == 35))
    {
      act("$N tells you 'You should hold on to these.'", FALSE, ch, 0, mob, TO_CHAR);
      act("$N gives you $q.", FALSE, ch, qcard, mob, TO_CHAR);
      obj_from_char(qcard);
      obj_to_char(qcard, ch);

      return TRUE;
    }
    else
    {
      act("$N tells you 'That wasn't a questcard you gave me...'", FALSE, ch, 0, mob, TO_CHAR);

      return TRUE;
    }
  }

  if (cmd != CMD_PRACTICE || !ch->skills) return FALSE;

  if (!*arg)
  {
    printf_to_char(ch,"`iYou have %d practice sessions.`q\n\r", ch->specials.spells_to_learn);

    switch (GET_CLASS(ch))
    {
      case CLASS_MAGIC_USER:
        showSpells = TRUE;
        break;

      case CLASS_CLERIC:
        showSpells = TRUE;

        if ((GET_LEVEL(ch) >= 35) || check_sc_access(ch, SKILL_MEDITATE))
        {
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

    if (showSkills)
    {
      send_to_char("`iYou can practice any of these skills:`q\n\r\n\r", ch);
      list_skills_to_prac(ch);
    }

    if (showSpells && showSkills)
    {
      send_to_char("\n\r", ch);
    }

    if (showSpells)
    {
      send_to_char("`iYou can practice any of these spells:`q\n\r\n\r", ch);
      list_spells_to_prac(ch, FALSE);
    }

    return TRUE;
  }

  spell = old_search_block(arg, 0, strlen(arg), spells, FALSE);

  if (spell == -1)
  {
    send_to_char("`iYou don't know of this spell.`q\n\r", ch);

    return TRUE;
  }

  if ((spell > 200) &&
      (spell != SPELL_REJUVENATION) &&
      (spell != SKILL_CAMP) &&
      !check_sc_master(ch, mob)) {
    send_to_char("You must practice in the presence of your subclass master.\n\r", ch);
    return TRUE;
  }

  switch (GET_CLASS(ch))
  {
    case CLASS_MAGIC_USER:
      index = -1;
    break;

    case CLASS_CLERIC:
      index = search_block(arg, cl_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(cl_skills[index], 0, strlen(cl_skills[index]), spells, TRUE);
      }

      if ((skill == SKILL_BASH) && (GET_LEVEL(ch) < 35))
      {
        index = -2;
      }
      break;

    case CLASS_THIEF:
      index = search_block(arg, t_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(t_skills[index], 0, strlen(t_skills[index]), spells, TRUE);
      }

      if ((skill == SKILL_CUNNING) && (GET_LEVEL(ch) < 50))
      {
        index = -2;
      }
      break;

    case CLASS_WARRIOR:
      index = search_block(arg, w_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(w_skills[index], 0, strlen(w_skills[index]), spells, TRUE);
      }

      if (((skill == SKILL_TRIPLE) && (GET_LEVEL(ch) < 20)) ||
          ((skill == SKILL_QUAD) && (GET_LEVEL(ch) < 50)))
      {
        index = -2;
      }
      break;

    case CLASS_NINJA:
      index = search_block(arg, n_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(n_skills[index], 0, strlen(n_skills[index]), spells, TRUE);
      }
      break;

    case CLASS_NOMAD:
      index = search_block(arg, nomad_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(nomad_skills[index], 0, strlen(nomad_skills[index]), spells, TRUE);
      }

      if ((skill == SKILL_DISEMBOWEL) && (GET_LEVEL(ch) < 20))
      {
        index = -2;
      }
      break;

    case CLASS_PALADIN:
      index = search_block(arg, pal_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(pal_skills[index], 0, strlen(pal_skills[index]), spells, TRUE);
      }

      if ((skill == SKILL_PRAY) && (GET_LEVEL(ch) < 40))
      {
        index = -2;
      }
      break;

    case CLASS_ANTI_PALADIN:
      index = search_block(arg, antipal_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(antipal_skills[index], 0, strlen(antipal_skills[index]), spells, TRUE);
      }
      break;

    case CLASS_AVATAR:
      index = search_block(arg, avatar_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(avatar_skills[index], 0, strlen(avatar_skills[index]), spells, TRUE);
      }
      break;

    case CLASS_BARD:
      index = search_block(arg, bard_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(bard_skills[index], 0, strlen(bard_skills[index]), spells, TRUE);
      }

      if ((skill == SKILL_BACKFLIP) && (GET_LEVEL(ch) < 20))
      {
        index = -2;
      }

      if ((skill == SKILL_CAMP) && (GET_LEVEL(ch) < 35)) {
        index = -2;
      }

      break;

    case CLASS_COMMANDO:
      index = search_block(arg, commando_skills, FALSE);

      if (index > -1)
      {
        skill = old_search_block(commando_skills[index], 0, strlen(commando_skills[index]), spells, TRUE);
      }

      if ((skill == SKILL_TRIPLE) && (GET_LEVEL(ch) < 20))
      {
        index = -2;
      }
      break;
  }

  if (index == -1)
  {
    if (spell == -1)
    {
      send_to_char("`iYou don't know of this spell.`q\n\r", ch);

      return TRUE;
    }

    if ((spell > 165) && (spell != SPELL_REJUVENATION) && !check_sc_master(ch, mob))
    {
      send_to_char("You must practice in the presence of your subclass master.\n\r", ch);

      return TRUE;
    }

    if (practice_spell(ch, spell)) return TRUE;

    send_to_char("`iYou don't know of this skill.`q\n\r", ch);

    return TRUE;
  }
  else if (index == -2)
  {
    send_to_char("`iYou can't practice this skill yet.`q\n\r", ch);

    return TRUE;
  }

  if (skill > 165 && (skill != SKILL_CAMP) && !check_sc_master(ch, mob))
  {
    send_to_char("You must practice in the presence of your subclass master.\n\r", ch);

    return TRUE;
  }

  practice_skill(ch, skill);

  return TRUE;
}

int dump(int room, CHAR *ch, int cmd, char *arg) {
  OBJ *k;
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

  for(k = world[CHAR_REAL_ROOM(ch)].contents; k ;
      k = world[CHAR_REAL_ROOM(ch)].contents) {
    sprintf(buf, "The %s vanishes in a puff of smoke.\n\r",rem_prefix(OBJ_SHORT(k)));
    for(tmp_char = world[CHAR_REAL_ROOM(ch)].people; tmp_char;
        tmp_char = tmp_char->next_in_room)
      if (CAN_SEE_OBJ(tmp_char, k)) send_to_char(buf,tmp_char);
    value += MAX(1, MIN(50, k->obj_flags.cost/10));
    extract_obj(k);
    if(IS_MORTAL(ch))
      sprintf(buf,"DUMP: [ %s dropped %s at The Dump ]",GET_NAME(ch),OBJ_SHORT(k));
    else if(IS_NPC(ch))
      sprintf(buf,"DUMP: [ %s dropped %s at The Dump ]",MOB_SHORT(ch),OBJ_SHORT(k));
    log_s(buf);
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
      object.bitvector2  =obj->obj_flags.bitvector2;
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

  if(is_abbrev(buf, "light"))        wear_pos   = ITEM_LIGHT_SOURCE;
  if(is_abbrev(buf, "light") && obj->obj_flags.type_flag==ITEM_LIGHT) wearable=TRUE;
  if(is_abbrev(buf, "finger"))       wear_pos   = ITEM_WEAR_FINGER;
  if(is_abbrev(buf, "neck"))         wear_pos   = ITEM_WEAR_NECK;
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

  if(is_abbrev(buf, "wizard"))       class_wear = ITEM_ANTI_MAGIC_USER;
  if(is_abbrev(buf, "mage"))         class_wear = ITEM_ANTI_MAGIC_USER;
  if(is_abbrev(buf, "cleric"))       class_wear = ITEM_ANTI_CLERIC;
  if(is_abbrev(buf, "thief"))        class_wear = ITEM_ANTI_THIEF;
  if(is_abbrev(buf, "warrior"))      class_wear = ITEM_ANTI_WARRIOR;
  if(is_abbrev(buf, "ninja"))        class_wear = ITEM_ANTI_NINJA;
  if(is_abbrev(buf, "nomad"))        class_wear = ITEM_ANTI_NOMAD;
  if(is_abbrev(buf, "paladin"))      class_wear = ITEM_ANTI_PALADIN;
  if(is_abbrev(buf, "anti-paladin")) class_wear = ITEM_ANTI_ANTIPALADIN;
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
      !strcmp("shun", name))
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
    sprintf(buf, "$n tells you '$p will cost %d coins to store.'", (total_cost_of_obj(obj) * 3) / 2);
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
evil, neutral, good\n\r", ch);

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

#define VAULT_MAX 500

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

  if (number <= 0)
  {
    act("$n tells you 'You can't store a negative number of items in your vault.'", FALSE, vault_guard, 0, ch, TO_VICT);

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
    // 'store x all.item' or 'store x all'
    if (number != 1)
    {
      sprintf(buf, "$n tells you 'You can't store '%d all' of something.'\n\r", number);
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

        if (!IS_RENTABLE(temp_obj))
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
    // 'store item' or 'store 1 item'
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
        else if (vault_put(ch, temp_obj, vault_obj))
        {
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

#define BRONZE_BAR  3013
#define SILVER_BAR  3014
#define GOLD_BAR    3015
#define PLATINUM_BAR    3016
#define MITHRIL_BAR    3017
#define DIAMOND     3018
#define ZYCA_SILVER 10923
#define ZYCA_BRONZE 10922
#define ABYSS_BAR   25034
#define DI_OPAL     27700
#define DI_CRYSTAL  27701

int jeweler(CHAR *mob, CHAR *ch, int cmd, char *arg)
{
  extern OBJ *read_object(int num, int type);

  OBJ *obj = NULL;
  char buf[MSL];
  int cost = 0;

  switch (cmd)
  {
    case CMD_LIST:
      send_to_char("\
You can buy:\n\r\
-------------------------------\n\r\
A Bronze Bar    :   50000 coins\n\r\
A Silver Bar    :  100000 coins\n\r\
A Gold Bar      :  200000 coins\n\r\
A Platinum Bar  :  500000 coins\n\r\
A Mithril Bar   : 1000000 coins\n\r\
A Large Diamond : 5000000 coins\n\r", ch);

      return TRUE;
      break;

    case CMD_SELL:
      if (!*arg)
      {
        send_to_char("The Jeweler tells you 'Sell what?'\n\r", ch);

        return TRUE;
      }

      one_argument(arg, buf);
      string_to_lower(buf);

      obj = get_obj_in_list_vis(ch, buf, ch->carrying);

      if (!obj)
      {
        send_to_char("The Jeweler tells you 'Sell what?'\n\r", ch);

        return TRUE;
      }

      switch (V_OBJ(obj))
      {
        case BRONZE_BAR:
        case SILVER_BAR:
        case GOLD_BAR:
        case PLATINUM_BAR:
        case MITHRIL_BAR:
        case DIAMOND:
        case ZYCA_SILVER:
        case ZYCA_BRONZE:
        case ABYSS_BAR:
        case DI_OPAL:
        case DI_CRYSTAL:
          break;

        default:
          send_to_char("The Jeweler tells you 'I only buy Midgaard trade bars, diamonds and a few other treasures.'\n\r", ch);

          return TRUE;
      }

      cost = obj->obj_flags.cost;

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

      if (!*buf)
      {
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
      else
      {
        send_to_char("The Jeweler tells you 'I don't sell that.'\n\r", ch);

        return TRUE;
      }

      if (!obj)
      {
        sprintf(buf, "[jeweler()] Unable to load object '%s'.", buf);
        wizlog(buf, LEVEL_ETE, WIZ_LOG_SIX);

        return TRUE;
      }

      cost = obj->obj_flags.cost;

      if (GET_GOLD(ch) < cost)
      {
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
  char name[MAX_STRING_LENGTH],sender[MAX_STRING_LENGTH],filename[MAX_STRING_LENGTH];
  char test[MAX_STRING_LENGTH],email_addr[80],buf[MAX_INPUT_LENGTH],reply_addr[80];
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
       act("$N tells you 'You don't have a postcard.",FALSE,ch,0,mob,TO_CHAR);
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
     sprintf(test,"$N tells you 'Postcard to %s on its way'",CAP(name));
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
       act("$N tells you 'You don't have a package.",FALSE,ch,0,mob,TO_CHAR);
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
      act("$N tells you 'That package contains an item that can't be posted.",FALSE,ch,0,mob,TO_CHAR);
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
  struct enchantment_type_5 ench;
  int exp=0,gold;

  if(enchanted_by(ch,"The title of Sorcerer")) {
    exp=0;
  }
  else if(enchanted_by(ch,"The title of Warlock")) {
    sprintf(title,"Sorcerer");
    exp=15000000;
    gold=10000000;
  }
  else if(enchanted_by(ch,"The title of Apprentice")) {
    sprintf(title,"Warlock");
    exp=10000000;
    gold=5000000;
  }
  else {
    sprintf(title,"Apprentice");
    exp=5000000;
    gold=1000000;
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
  struct enchantment_type_5 ench;
  int exp=0,gold;

  if(enchanted_by(ch,"The title of Prophet")) {
    exp=0;
  }
  else if(enchanted_by(ch,"The title of Bishop")) {
    sprintf(title,"Prophet");
    exp=15000000;
    gold=10000000;
  }
  else if(enchanted_by(ch,"The title of Acolyte")) {
    sprintf(title,"Bishop");
    exp=10000000;
    gold=5000000;
  }
  else {
    sprintf(title,"Acolyte");
    exp=5000000;
    gold=1000000;
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
  struct enchantment_type_5 ench;
  int exp=0,gold;

  if(enchanted_by(ch,"The title of Shogun")) {
    exp=0;
  }
  else if(enchanted_by(ch,"The title of Shinobi")) {
    sprintf(title,"Shogun");
    exp=15000000;
    gold=10000000;
  }
  else if(enchanted_by(ch,"The title of Tsume")) {
    sprintf(title,"Shinobi");
    exp=10000000;
    gold=5000000;
  }
  else {
    sprintf(title,"Tsume");
    exp=5000000;
    gold=1000000;
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
  struct enchantment_type_5 ench;
  int exp=0,gold;

  if(enchanted_by(ch,"The title of Knight")) {
    exp=0;
  }
  else if(enchanted_by(ch,"The title of Swashbuckler")) {
    sprintf(title,"Knight");
    exp=15000000;
    gold=10000000;
  }
  else if(enchanted_by(ch,"The title of Squire")) {
    sprintf(title,"Swashbuckler");
    exp=10000000;
    gold=5000000;
  }
  else {
    sprintf(title,"Squire");
    exp=5000000;
    gold=1000000;
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
  struct enchantment_type_5 ench;
  int exp=0,gold;

  if(enchanted_by(ch,"The title of Lord/Lady")) {
    exp=0;
  }
  else if(enchanted_by(ch,"The title of Justiciar")) {
    sprintf(title,"Lord/Lady");
    if(GET_SEX(ch)==SEX_FEMALE) sprintf(tmptitle,"Lady");
    else sprintf(tmptitle,"Lord");
    exp=15000000;
    gold=10000000;
  }
  else if(enchanted_by(ch,"The title of First Sword")) {
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
  struct enchantment_type_5 ench;
  int exp=0,gold;

  if(enchanted_by(ch,"The title of Tamer")) {
    exp=0;
  }
  else if(enchanted_by(ch,"The title of Forester")) {
    sprintf(title,"Tamer");
    exp=15000000;
    gold=10000000;
  }
  else if(enchanted_by(ch,"The title of Wanderer")) {
    sprintf(title,"Forester");
    exp=10000000;
    gold=5000000;
  }
  else {
    sprintf(title,"Wanderer");
    exp=5000000;
    gold=1000000;
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
  struct enchantment_type_5 ench;
  int exp=0,gold;

  if(enchanted_by(ch,"The title of Dark Lord/Lady")) {
    exp=0;
  }
  else if(enchanted_by(ch,"The title of Dark Warder")) {
    sprintf(title,"Dark Lord/Lady");
    if(GET_SEX(ch)==SEX_FEMALE) sprintf(tmptitle,"Dark Lady");
    else sprintf(tmptitle,"Dark Lord");
    exp=15000000;
    gold=10000000;
  }
  else if(enchanted_by(ch,"The title of Minion of Darkness")) {
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
  struct enchantment_type_5 ench;
  int exp=0,gold;

  if(enchanted_by(ch,"The title of Conductor")) {
    exp=0;
  }
  else if(enchanted_by(ch,"The title of Poet")) {
    sprintf(title,"Conductor");
    exp=15000000;
    gold=10000000;
  }
  else if(enchanted_by(ch,"The title of Minstrel")) {
    sprintf(title,"Poet");
    exp=10000000;
    gold=5000000;
  }
  else {
    sprintf(title,"Minstrel");
    exp=5000000;
    gold=1000000;
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
  struct enchantment_type_5 ench;
  int exp=0,gold;

  if(enchanted_by(ch,"The rank of Commander")) {
    exp=0;
  }
  else if(enchanted_by(ch,"The rank of Commodore")) {
    sprintf(title,"Commander");
    exp=15000000;
    gold=10000000;
  }
  else if(enchanted_by(ch,"The rank of Private")) {
    sprintf(title,"Commodore");
    exp=10000000;
    gold=5000000;
  }
  else {
    sprintf(title,"Private");
    exp=5000000;
    gold=1000000;
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
  struct enchantment_type_5 ench;
  int exp=0,gold;

  if(enchanted_by(ch,"The status of Assassin")) {
    exp=0;
  }
  else if(enchanted_by(ch,"The status of Brigand")) {
    sprintf(title,"Assassin");
    exp=15000000;
    gold=10000000;
  }
  else if(enchanted_by(ch,"The status of Highwayman")) {
    sprintf(title,"Brigand");
    exp=10000000;
    gold=5000000;
  }
  else {
    sprintf(title,"Highwayman");
    exp=5000000;
    gold=1000000;
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

int mayor(CHAR *mayor, CHAR *ch, int cmd, char *arg)
{
  static char open_path[] =
    "W3a3003b33000c111d0d1111e333333332e22c22111221a1S.";
  static char close_path[] =
    "W3a3003b33000c111d0d111CE333333332CE22c22111221a1S.";
  /*
     const struct social_type open_path[] = {
     {"G",0}
     };

     static void *thingy = 0;
     static int cur_line = 0;

     for (i=0; i < 1; i++)
     {
     if (*(open_path[cur_line].cmd) == '!') {
     i++;
     exec_social(ch, (open_path[cur_line].cmd)+1,
     open_path[cur_line].next_line, &cur_line, &thingy);
     } else {
     exec_social(ch, open_path[cur_line].cmd,
     open_path[cur_line].next_line, &cur_line, &thingy);
     }
     */
  static char *path;
  static int index;
  static bool move = FALSE;

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

  if (1 && cmd == CMD_SAY && !strncmp(" promote", arg, strlen(" promote")))
    {
    do_say(ch, arg, CMD_SAY);
    switch (GET_CLASS(ch))
      {
      case CLASS_MAGIC_USER:
        promote_mage(mayor,ch);
        break;
      case CLASS_THIEF:
        promote_thief(mayor,ch);
        break;
      case CLASS_WARRIOR:
        promote_warrior(mayor,ch);
        break;
      case CLASS_PALADIN:
        promote_paladin(mayor,ch);
        break;
      case CLASS_BARD:
        promote_bard(mayor,ch);
        break;
      case CLASS_NINJA:
        promote_ninja(mayor,ch);
        break;
      case CLASS_ANTI_PALADIN:
        promote_antipaladin(mayor,ch);
        break;
      case CLASS_CLERIC:
        promote_cleric(mayor,ch);
        break;
      case CLASS_NOMAD:
        promote_nomad(mayor,ch);
        break;
      case CLASS_COMMANDO:
        promote_commando(mayor,ch);
        break;
      }
      save_char(ch,NOWHERE);
      return TRUE;
    }

  if (cmd || !move || (GET_POS(mayor) < POSITION_SLEEPING) ||
      (GET_POS(mayor) == POSITION_FIGHTING))
    return FALSE;

  switch (path[index]) {
  case '0' :
  case '1' :
  case '2' :
  case '3' :
    do_move(mayor,"",path[index]-'0'+1);
    break;

  case 'W' :
    GET_POS(mayor) = POSITION_STANDING;
    act("$n awakens and groans loudly.",FALSE,mayor,0,0,TO_ROOM);
    break;

  case 'S' :
    GET_POS(mayor) = POSITION_SLEEPING;
    act("$n lies down and instantly falls asleep.",FALSE,mayor,0,0,TO_ROOM);
    break;

  case 'a' :
    act("$n says 'Hello Honey!'",FALSE,mayor,0,0,TO_ROOM);
    act("$n smirks.",FALSE,mayor,0,0,TO_ROOM);
    break;

  case 'b' :
    act("$n says 'What a view! I must get something done about that dump!'",
        FALSE,mayor,0,0,TO_ROOM);
    break;

  case 'c' :
    act("$n says 'Vandals! Youngsters nowadays have no respect for anything!'",
        FALSE,mayor,0,0,TO_ROOM);
    break;

  case 'd' :
    act("$n says 'Good day, citizens!'", FALSE, mayor, 0,0,TO_ROOM);
    break;

  case 'e' :
    act("$n says 'I hereby declare the bazaar open!'",FALSE,mayor,0,0,TO_ROOM);
    break;

  case 'E' :
    act("$n says 'I hereby declare Midgaard closed!'",FALSE,mayor,0,0,TO_ROOM);
    break;

  case 'O' :
    do_unlock(mayor, "gate", 0);
    do_open(mayor, "gate", 0);
    break;

  case 'C' :
    do_close(mayor, "gate", 0);
    do_lock(mayor, "gate", 0);
    break;

  case '.' :
    move = FALSE;
    break;

  }

  index++;
  return FALSE;
}

/* ********************************************************************
 *  General special procedures for mobiles                                      *
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

int membership(OBJ *obj,CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH];

  if ((cmd != CMD_BUY) && (cmd != CMD_LIST)) return(FALSE);

  arg = one_argument(arg, buf);


  if (cmd==CMD_LIST) {    /* List */
    send_to_char("The membership fee are 500000 coins.\n\r", ch);
    send_to_char("The fee to CANCEL the membership are 100000 coins.\n\r", ch);
    return(TRUE);
  }

  else {

    if (!*buf)
      {
  send_to_char("Buy what?\n\r",ch);
  return(TRUE);
      }

    if (!strcmp(buf,"membership"))
      {  if (GET_GOLD(ch) < 500000) {
  send_to_char("You don't have enough coins!", ch);
  return(TRUE);
      }
  if (IS_SET(ch->specials.pflag, PLR_SANES_VOCAL_CLUB) ||
      IS_SET(ch->specials.pflag, PLR_LEMS_LIQOUR_LOUNGE) ||
      IS_SET(ch->specials.pflag, PLR_LINERS_LOUNGE) ||
      IS_SET(ch->specials.pflag, PLR_RANGERS_RELIQUARY)) {
    send_to_char("You can only join one club!\n\r", ch);
    return(TRUE);
  }

  if (CHAR_REAL_ROOM(ch) == real_room(3075)) {
    send_to_char("You are now a member of Sane's Vocal Club.\n\r", ch);
    /*    if (ch->abilities.str == 18) {
      ch->abilities.str_add += 20;
      ch->tmpabilities.str_add += 20;
      } else {
      ch->abilities.str += 1;
      ch->tmpabilities.str += 1;
      } */
    SET_BIT(ch->specials.pflag, PLR_SANES_VOCAL_CLUB);
  }

  if (CHAR_REAL_ROOM(ch) == real_room(3078)) {
    send_to_char("You are now a member of Liner's Lounge.\n\r", ch);
    /*    ch->abilities.intel += 1;
      ch->tmpabilities.intel += 1; */
    SET_BIT(ch->specials.pflag, PLR_LINERS_LOUNGE);
  }

  if (CHAR_REAL_ROOM(ch) == real_room(3080)) {
    send_to_char("You are now a member of Lem's Liqour Lounge.\n\r", ch);
    /*    ch->abilities.con += 1;
      ch->tmpabilities.con += 1; */
    SET_BIT(ch->specials.pflag, PLR_LEMS_LIQOUR_LOUNGE);
  }

  if (CHAR_REAL_ROOM(ch) == real_room(3082)) {
    send_to_char("You are now a member of Ranger's Reliquary.\n\r", ch);
    /*    ch->abilities.wis += 2;
      ch->tmpabilities.wis += 2; */
    SET_BIT(ch->specials.pflag, PLR_RANGERS_RELIQUARY);
  }

  GET_GOLD(ch) -= 500000;
  act("$n is now a member of the club.", FALSE, ch, 0, 0, TO_ROOM);
        return(TRUE); /* Added by Ranger - April 96 */
      }
    else if (!strcmp(buf,"cancel")) {
      if (GET_GOLD(ch) < 100000) {
        send_to_char("You don't have enough coins!\n\r", ch);
        return(TRUE);
      }
      if (IS_SET(ch->specials.pflag, PLR_RANGERS_RELIQUARY))
        REMOVE_BIT(ch->specials.pflag, PLR_RANGERS_RELIQUARY);
      else if (IS_SET(ch->specials.pflag, PLR_LINERS_LOUNGE))
        REMOVE_BIT(ch->specials.pflag, PLR_LINERS_LOUNGE);
      else if (IS_SET(ch->specials.pflag, PLR_SANES_VOCAL_CLUB))
        REMOVE_BIT(ch->specials.pflag, PLR_SANES_VOCAL_CLUB);
      else if (IS_SET(ch->specials.pflag, PLR_LEMS_LIQOUR_LOUNGE))
        REMOVE_BIT(ch->specials.pflag, PLR_LEMS_LIQOUR_LOUNGE);
      else {
        send_to_char("But you are not a member of any club!!\n\r", ch);
        return(TRUE);
      }
      GET_GOLD(ch) -= 100000;
      send_to_char("Done. You are not a member of any club now!\n\r", ch);
      return(TRUE); /* Added by Ranger - April 96 */
    } else
      send_to_char("Buy what?\n\r", ch);
      return(TRUE); /* Added by Ranger - April 96 */
  }
  return FALSE;
}

int club_guard(CHAR *mob,CHAR *ch, int cmd, char *arg)
{
  char buf[256], buf2[256];

  if (cmd>6 || cmd<1)
    return FALSE;

  strcpy(buf,  "The guard humiliates you and says 'You are not a member!!'.\n\r");
  strcpy(buf2, "The guard humiliates $n and says 'You are not a member!!'.");

  if ((GET_POS(ch) == POSITION_RIDING) &&
      (((CHAR_REAL_ROOM(ch) == real_room(3075)) && (cmd ==CMD_WEST)) ||
       ((CHAR_REAL_ROOM(ch) == real_room(3078)) && (cmd ==CMD_NORTH)) ||
       ((CHAR_REAL_ROOM(ch) == real_room(3080)) && (cmd ==CMD_EAST)) ||
       ((CHAR_REAL_ROOM(ch) == real_room(3082)) && (cmd ==CMD_SOUTH))))
    {  send_to_char("Dismount please!\n\r", ch);
  return(TRUE);
      }

  if ((CHAR_REAL_ROOM(ch) == real_room(3075)) && (cmd == CMD_WEST) &&
      !IS_SET(ch->specials.pflag, PLR_SANES_VOCAL_CLUB)) {
    act(buf2, FALSE, ch, 0, 0, TO_ROOM);
    send_to_char(buf, ch);
    return(TRUE);
  } else if ((CHAR_REAL_ROOM(ch) == real_room(3078)) && (cmd == CMD_NORTH)
       && !IS_SET(ch->specials.pflag, PLR_LINERS_LOUNGE)) {
    act(buf2, FALSE, ch, 0, 0, TO_ROOM);
    send_to_char(buf, ch);
    return(TRUE);
  } else if ((CHAR_REAL_ROOM(ch) == real_room(3080)) && (cmd == CMD_EAST)
       && !IS_SET(ch->specials.pflag, PLR_LEMS_LIQOUR_LOUNGE)) {
    act(buf2, FALSE, ch, 0, 0, TO_ROOM);
    send_to_char(buf, ch);
    return(TRUE);
  } else if ((CHAR_REAL_ROOM(ch) == real_room(3082)) && (cmd ==CMD_SOUTH)
       && !IS_SET(ch->specials.pflag, PLR_RANGERS_RELIQUARY)) {
    act(buf2, FALSE, ch, 0, 0, TO_ROOM);
    send_to_char(buf, ch);
    return(TRUE);
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
    if (GET_ITEM_TYPE(i)==ITEM_CONTAINER && i->obj_flags.value[3] &&
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
  char buf[MAX_STRING_LENGTH], pet_name[MAX_STRING_LENGTH],tmp_pwd[11];
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

int teller(OBJ *obj,CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH], number[MAX_STRING_LENGTH];
  int num = 0;

  if ((cmd != CMD_BALANCE) && (cmd != CMD_WITHDRAW) && (cmd != CMD_DEPOSIT)) return(FALSE);


  if (IS_NPC(ch))
    return (FALSE);

  if (cmd == CMD_BALANCE)
    {
      sprintf(buf,"Your balance is %d coins.\n\r",ch->points.bank);
      send_to_char(buf,ch);
      return(TRUE);
    }

  if (!*arg)
    { send_to_char("Please specify an amount.\n\r", ch);
      return(FALSE);
    }

  one_argument(arg, number);
  if (!*number) return FALSE;
  if(!strcmp(number,"all")) {
    if(cmd==CMD_DEPOSIT) num=GET_GOLD(ch);
    if(cmd==CMD_WITHDRAW) num=GET_BANK(ch);
  }
  else {
    if(!isdigit(*number)) return FALSE;
    if (!(num = atoi(number)) ) return FALSE;
  }

  if (num <= 0)
    {  send_to_char("Amount must be positive.\n\r", ch);
  return(FALSE);
      }

  if (cmd == CMD_DEPOSIT)
    {
      if (GET_GOLD(ch) < num) {
  send_to_char("You don't have that many coins!\n\r",ch);
  return TRUE;
      }
      GET_GOLD(ch) += -num;
      ch->points.bank += num;
      send_to_char("Ok.\n\r",ch);
      return TRUE;
    }
  else
    {
      if (ch->points.bank < num) {
  send_to_char("You don't have that many coins in the Bank!\n\r",ch);
  return TRUE;
      }
      GET_GOLD(ch) += num;
      ch->points.bank -= num;
      send_to_char("Ok.\n\r",ch);
      return TRUE;
    }
  return FALSE;
}

/*int insurance(OBJ *obj,CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_STRING_LENGTH];
  int num;

  if ((cmd != CMD_LIST) && (cmd != CMD_BUY)) return(FALSE);
  arg = one_argument(arg, buf);

  if (cmd==CMD_LIST) {
    sprintf(buf, "Your insurance fee are %d coins.", (GET_LEVEL(ch) * 5000));
    send_to_char(buf, ch);
    return(TRUE);
  }
  else {
    if (!*buf) {
      send_to_char("Buy what?\n\r",ch);
      return(TRUE);
    }

    if (!strcmp(buf,"insurance")) {
      num = (GET_LEVEL(ch) * 5000);
      if (GET_GOLD(ch) < num) {
  send_to_char("You don't have enough coins!", ch);
  return(TRUE);
      }

      GET_GOLD(ch) -= num;
      send_to_char("You have bought an insurance.\n\r", ch);
      act("$n buys an insurance.", FALSE, ch, 0, 0, TO_ROOM);
      SET_BIT(ch->specials.pflag, PLR_INSURANCE);
    }
    else
      send_to_char("Buy what?\n\r", ch);
  }
  return FALSE;
}
*/

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

  if(GET_LEVEL(ch)>15 && !CHAOSMODE) {
    send_to_char("Doctor Naikrovek says 'This hospital is for level 15 and under only.'\n\r",ch);
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
  char buf[MAX_STRING_LENGTH];

  if(cmd!=CMD_LIST) return FALSE;
  if(!ch) return FALSE;
  one_argument(arg,buf);
  if(!*buf) {
    send_to_char("\
list all/keyword or :\n\r\
     light, finger, neck, body, head, legs, feet, hands,\n\r\
     arms, shield, about, waist, wrist, wield, hold, throw,\n\r\
     wizard, cleric, thief, warrior, ninja, nomad, paladin,\n\r\
     anti-paladin, avatar, bard, commando, evil, neutral, good.\n\r",ch);
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

                    if (GET_ITEM_TYPE(obj) == ITEM_RECIPE)
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
                            af.location = APPLY_AC;
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
}
