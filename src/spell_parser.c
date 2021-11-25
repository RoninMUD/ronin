/* ************************************************************************
*  file: spell_parser.c , Basic routines and parsing      Part of DIKUMUD *
*  Usage : Interpreter of spells                                          *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "utility.h"
#include "mob.spells.h"
#include "subclass.h"
#include "enchant.h"
#include "fight.h"
#include "cmd.h"
#include "limits.h"
#include "aff_ench.h"

struct spell_info_t spell_info[MAX_SPL_LIST + 1];
struct spell_text_t spell_text[MAX_SPL_LIST + 1];
char *spells[MAX_SPL_LIST];

#define \
SPELLO(_spell, _beats, _min_pos, _mu_lvl, _cl_lvl, _ni_lvl, _pa_lvl, _ap_lvl, _av_lvl, _ba_lvl, _co_lvl, _mana, _tar, _func) { \
  if (_spell < MAX_SPL_LIST) { \
    spell_info[_spell].beats                  = (_beats);   \
    spell_info[_spell].minimum_position       = (_min_pos); \
    spell_info[_spell].min_level_magic        = (_mu_lvl);  \
    spell_info[_spell].min_level_cleric       = (_cl_lvl);  \
    spell_info[_spell].min_level_ninja        = (_ni_lvl);  \
    spell_info[_spell].min_level_paladin      = (_pa_lvl);  \
    spell_info[_spell].min_level_anti_paladin = (_ap_lvl);  \
    spell_info[_spell].min_level_avatar       = (_av_lvl);  \
    spell_info[_spell].min_level_bard         = (_ba_lvl);  \
    spell_info[_spell].min_level_commando     = (_co_lvl);  \
    spell_info[_spell].min_usesmana           = (_mana);    \
    spell_info[_spell].targets                = (_tar);     \
    spell_info[_spell].spell_pointer          = (_func);    \
  } \
  else { \
    log_f("WARNING: Spell %d out of range.", _spell); \
  } \
}

#define \
SPLTXTO(_spell, _name, _to_char_msg, _to_room_msg, _wear_off_msg) { \
  if (_spell < MAX_SPL_LIST) { \
    spell_text[_spell].name         = (_name);         \
    spell_text[_spell].to_char_msg  = (_to_char_msg);  \
    spell_text[_spell].to_room_msg  = (_to_room_msg);  \
    spell_text[_spell].wear_off_msg = (_wear_off_msg); \
  } \
  else { \
    log_f("WARNING: Spell message %d out of range.", _spell); \
  } \
}

void affect_update() {
  for (CHAR *ch = character_list; ch; ch = ch->next) {
    for (AFF *aff = ch->affected, *next_aff; aff; aff = next_aff) {
      next_aff = aff->next;

      if ((aff->type <= 0) || (aff->type >= MAX_SPL_LIST) || (aff->duration < 0)) continue;

      if (aff->duration == 0) {
        if (!aff->next || (aff->next->type != aff->type) || (aff->next->duration > 0)) {
          print_spell_wear_off_message(ch, aff->type);
        }

        aff_remove(ch, aff);
      }
      else {
        aff->duration--;
      }
    }
  }
}

int SPELL_LEVEL(CHAR *ch, int spell_nr) {
  int level = LEVEL_IMP;

  if (ch && spell_nr) {
    switch (GET_CLASS(ch)) {
      case CLASS_MAGIC_USER:
        level = spell_info[spell_nr].min_level_magic;
        break;

      case CLASS_CLERIC:
        level = spell_info[spell_nr].min_level_cleric;
        break;

      case CLASS_NINJA:
        level = spell_info[spell_nr].min_level_ninja;
        break;

      case CLASS_PALADIN:
        level = spell_info[spell_nr].min_level_paladin;
        break;

      case CLASS_ANTI_PALADIN:
        level = spell_info[spell_nr].min_level_anti_paladin;
        break;

      case CLASS_AVATAR:
        level = spell_info[spell_nr].min_level_avatar;
        break;

      case CLASS_BARD:
        level = spell_info[spell_nr].min_level_bard;
        break;

      case CLASS_COMMANDO:
        level = spell_info[spell_nr].min_level_commando;
        break;
    }
  }

  return level;
}

void say_spell(CHAR *ch, int spell) {
  if (!ch || !spell || !(*spell_text[spell].name)) return;

  struct say_spell_msg_t {
    char to_same_class[MSL];
    char to_other_class[MSL];
  };

  struct spell_syllable_t {
    char *org;
    char *new;
  };

  struct spell_syllable_t spell_syllable_table[] = {
    { " ", " " },
    { "ar", "abra" },
    { "au", "kada" },
    { "bless", "fido" },
    { "blind", "nose" },
    { "bur", "mosa" },
    { "cu", "judi" },
    { "de", "oculo" },
    { "en", "unso" },
    { "id", "dhfj" },
    { "light", "dies" },
    { "lo", "hi" },
    { "mor", "zak" },
    { "move", "sido" },
    { "ness", "lacri" },
    { "ning", "illa" },
    { "per", "duda" },
    { "ra", "gru" },
    { "re", "candus" },
    { "son", "sabru" },
    { "tect", "infra" },
    { "tri", "cula" },
    { "ven", "nofo" },
    { "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" }, { "e", "z" },
    { "f", "y" }, { "g", "o" }, { "h", "p" }, { "i", "u" }, { "j", "y" },
    { "k", "t" }, { "l", "r" }, { "m", "w" }, { "n", "i" }, { "o", "a" },
    { "p", "s" }, { "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
    { "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" }, { "y", "l" },
    { "z", "k" }
  };

  char mystic_words[MIL * 4] = "";

  for (size_t offset = 0, syl_len = 1; syl_len && (offset < strlen(spell_text[spell].name)); offset += syl_len) {
    for (int syl = 0; syl < NUMELEMS(spell_syllable_table); syl++) {
      syl_len = strlen(spell_syllable_table[syl].org);

      if (strncmp(spell_syllable_table[syl].org, (spell_text[spell].name + offset), syl_len) == 0) {
        str_cat(mystic_words, sizeof(mystic_words), spell_syllable_table[syl].new);
      }
    }
  }

  struct say_spell_msg_t say_spell_msg = { 0 };

  if (GET_CLASS(ch) == CLASS_NINJA) {
    snprintf(say_spell_msg.to_same_class, sizeof(say_spell_msg.to_same_class),
      "$n makes a mystic hand position and utters the words, '%s'.", spell_text[spell].name);
    snprintf(say_spell_msg.to_other_class, sizeof(say_spell_msg.to_other_class),
      "$n makes a mystic hand position and utters the words, '%s'.", mystic_words);
  }
  else {
    snprintf(say_spell_msg.to_same_class, sizeof(say_spell_msg.to_same_class),
      "$n utters the words, '%s'.", spell_text[spell].name);
    snprintf(say_spell_msg.to_other_class, sizeof(say_spell_msg.to_other_class),
      "$n utters the words, '%s'.", mystic_words);
  }

  for (CHAR *temp_ch = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)); temp_ch; temp_ch = temp_ch->next_in_room) {
    if (temp_ch != ch) {
      if (((GET_CLASS(temp_ch) == GET_CLASS(ch)) && !IS_IMMORTAL(ch)) || IS_IMMORTAL(temp_ch)) {
        act(say_spell_msg.to_same_class, FALSE, ch, 0, temp_ch, TO_VICT);
      }
      else {
        act(say_spell_msg.to_other_class, FALSE, ch, 0, temp_ch, TO_VICT);
      }
    }
  }
}

const char save[11][5][2] = {
  /* Para     Rods     Petri    Breath   Spells */
  { { 16, 4 },{ 13, 1 },{ 15, 3 },{ 17, 4 },{ 14, 1 } }, /* Mage */
  { { 11, 1 },{ 16, 3 },{ 14, 2 },{ 18, 5 },{ 17, 3 } }, /* Cleric */
  { { 15, 6 },{ 16, 2 },{ 14, 3 },{ 18, 4 },{ 17, 4 } }, /* Thief */
  { { 16, 1 },{ 18, 3 },{ 17, 1 },{ 20, 2 },{ 19, 3 } }, /* Warrior */
  { { 15, 6 },{ 16, 2 },{ 14, 3 },{ 18, 5 },{ 17, 1 } }, /* Ninja */
  { { 15, 6 },{ 16, 2 },{ 14, 3 },{ 18, 5 },{ 17, 1 } }, /* Nomad */
  { { 15, 6 },{ 16, 2 },{ 14, 3 },{ 18, 5 },{ 17, 1 } }, /* Paladin */
  { { 15, 6 },{ 16, 2 },{ 14, 3 },{ 18, 5 },{ 17, 1 } }, /* Antipaladin */
  { { 15, 6 },{ 16, 2 },{ 14, 3 },{ 18, 5 },{ 17, 1 } }, /* Avatar */
  { { 15, 6 },{ 16, 2 },{ 14, 3 },{ 18, 5 },{ 17, 1 } }, /* Bard */
  { { 15, 6 },{ 16, 2 },{ 14, 3 },{ 18, 5 },{ 17, 1 } }  /* Commando */
};

bool saves_spell(struct char_data *ch, sh_int type, int level)
{
  int saving_throw = 0;
  int ch_class = 0;
  double ch_level = 0.0;
  double diff = 0.0;
  double total = 0.0;

  /* For PCs, a negative apply_saving_throw makes the saving throw better! */
  /* For mobs, the saving throw is the mob's level; the higher the better. */
  saving_throw = ch->specials.apply_saving_throw[type];

  if (!IS_NPC(ch)) {
    if (GET_LEVEL(ch) >= LEVEL_IMM) return TRUE;

    ch_class = GET_CLASS(ch) - 1;
    ch_level = ((double)GET_LEVEL(ch)) / ((double)LEVEL_IMM - 1);
    diff = ch_level * save[ch_class][type][0] - save[ch_class][type][1];
    total = save[ch_class][type][0] - diff;
    saving_throw += total;
  }
  else {
    saving_throw = level - saving_throw;
  }

  return (MAX(2, saving_throw) < number(1, 20));
}

int USE_MANA(CHAR *ch, int spell_number) {
  int mana = 0;

  switch (spell_number) {
    case SPELL_METEOR:
      mana = spell_info[spell_number].min_usesmana;
      break;

    case SPELL_VAMPIRIC:
      mana = 40 + (GET_LEVEL(ch) * 2);
      break;

    case SPELL_FURY:
      mana = ((GET_CLASS(ch) == CLASS_PALADIN) && (GET_LEVEL(ch) == 50) && !CHAOSMODE) ? 120 : spell_info[spell_number].min_usesmana;
      break;

    default:
      mana = MAX(spell_info[spell_number].min_usesmana, 100 / (2 + GET_LEVEL(ch) - SPELL_LEVEL(ch, spell_number)));
      break;
  }

  /* Druid SC4: Elemental Form - Spells cost 10% less mana to cast. */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_DRUID, 4) && ench_enchanted_by(ch, ENCH_NAME_ELEMENTAL_FORM, 0)) {
    mana = MAX(mana * 0.9, 1);
  }

  return mana;
}

/* Assumes that *argument does start with first letter of chopped string */
void do_cast(struct char_data *ch, char *argument, int cmd) {
  void set_fighting(CHAR *ch, CHAR *vict);

  struct obj_data *tar_obj;
  struct char_data *tar_char;
  char name[MAX_STRING_LENGTH],buf[MAX_INPUT_LENGTH];
  int qend, spl, percent = 0,con_amt = 0,mult,conc,mana_cost;
  static int castlog=0;
  bool target_ok;
  FILE *fl;

  if (IS_NPC(ch) || !ch->skills) return;

  if (GET_LEVEL(ch) < LEVEL_IMM) {
    if (GET_CLASS(ch) == CLASS_WARRIOR) {
      send_to_char("Think you had better stick to fighting...\n\r", ch);
      return;
    }
    if (GET_CLASS(ch) == CLASS_THIEF) {
      send_to_char("Think you should stick to robbing and killing...\n\r", ch);
      return;
    }
    if (GET_CLASS(ch) == CLASS_NOMAD) {
      send_to_char("Think you should stick to trapping and disarming...\n\r",ch);
      return;
    }
    if ((GET_CLASS(ch) == CLASS_PALADIN) && (GET_ALIGNMENT(ch) < 350)) {
      send_to_char("Your alignment does not allow you to cast spells.\n\r",ch);
      return;
    }
    if ((GET_CLASS(ch) == CLASS_ANTI_PALADIN) && (GET_ALIGNMENT(ch) > -350)) {
      send_to_char("Your alignment does not allow you to cast spells.\n\r",ch);
      return;
    }
  }

  argument = skip_spaces(argument);

  /* If there is no chars in argument */
  if (!(*argument)) {
    send_to_char("Cast which what where?\n\r", ch);
    return;
  }

  if(GET_LEVEL(ch)==LEVEL_IMP && !strcmp(argument,"log")) {
    if(castlog) {
      send_to_char("Cast log off.\n\r",ch);
      castlog=0;
    }
    else {
      send_to_char("Cast log on.\n\r",ch);
      castlog=1;
    }
    return;
  }

  if (*argument != '\'') {
    send_to_char("Magic must always be enclosed by the magic symbols:'\n\r", ch);
    return;
  }

  for (qend = 1; *(argument + qend) && (*(argument + qend) != '\''); qend++) {
    *(argument + qend) = LOWER(*(argument + qend));
  }

  if (*(argument + qend) != '\'') {
    send_to_char("Magic must always be enclosed by the magic symbols: '\n\r", ch);
    return;
  }

  /* Look for the spell */
  if ((*argument + 1) == '!') {
    spl = MAX_SPL_LIST + 1;
  }
  else {
    spl = old_search_block(argument, 1, qend - 1, (const char * const * const)spells, 0);
  }

  if (!spl) {
    send_to_char("Your lips do not move; no magic appears.\n\r",ch);

    return;
  }

  if ((spl <= 0) || (spl > MAX_SPL_LIST) || !(spell_info[spl].spell_pointer) || !check_sc_access(ch, spl)) {
    switch (number(1, 5)) {
      case 1: send_to_char("Bylle Grylle Grop Gryf???\n\r", ch); break;
      case 2: send_to_char("Olle Bolle Snop Snyf?\n\r", ch); break;
      case 3: send_to_char("Olle Grylle Bolle Bylle?!?\n\r", ch); break;
      case 4: send_to_char("Gryffe Olle Gnyffe Snop???\n\r", ch); break;
      default: send_to_char("Bolle Snylle Gryf Bylle?!!?\n\r", ch); break;
    }

    return;
  }


  /* Check Position */
  if (GET_POS(ch) < spell_info[spl].minimum_position) {
    switch(GET_POS(ch)) {
      case POSITION_SLEEPING :
           send_to_char("You dream about great magical powers.\n\r", ch);
           break;
      case POSITION_RESTING :
           send_to_char("You can't concentrate enough while resting.\n\r",ch);
           break;
      case POSITION_SITTING :
           send_to_char("You can't do this sitting!\n\r", ch);
           break;
      case POSITION_FIGHTING :
           send_to_char("Impossible! You can't concentrate enough!\n\r", ch);
           break;
      default:
           send_to_char("It seems like you're in a pretty bad shape!\n\r",ch);
           break;
      }
    return;
  }

  /* Check Level */
  if(GET_LEVEL(ch) < LEVEL_IMM) {
       if ((GET_CLASS(ch) == CLASS_MAGIC_USER) &&
         (spell_info[spl].min_level_magic > GET_LEVEL(ch))) {
         send_to_char("Sorry, you can't do that.\n\r", ch);
         return;
       }
       if ((GET_CLASS(ch) == CLASS_CLERIC) &&
         (spell_info[spl].min_level_cleric > GET_LEVEL(ch))) {
         send_to_char("Sorry, you can't do that.\n\r", ch);
         return;
       }
       if ((GET_CLASS(ch) == CLASS_NINJA) &&
         (spell_info[spl].min_level_ninja > GET_LEVEL(ch))) {
         send_to_char("Sorry, you can't do that.\n\r", ch);
         return;
       }
       if ((GET_CLASS(ch) == CLASS_PALADIN) &&
         (spell_info[spl].min_level_paladin > GET_LEVEL(ch))) {
         send_to_char("Sorry, you can't do that.\n\r", ch);
         return;
       }
       if ((GET_CLASS(ch) == CLASS_ANTI_PALADIN) &&
         (spell_info[spl].min_level_anti_paladin > GET_LEVEL(ch))) {
         send_to_char("Sorry, you can't do that.\n\r", ch);
         return;
       }
       if ((GET_CLASS(ch) == CLASS_AVATAR) &&
         (spell_info[spl].min_level_avatar > GET_LEVEL(ch))) {
         send_to_char("Sorry, you can't do that.\n\r", ch);
         return;
       }
       if ((GET_CLASS(ch) == CLASS_BARD) &&
         (spell_info[spl].min_level_bard > GET_LEVEL(ch))) {
         send_to_char("Sorry, you can't do that.\n\r", ch);
         return;
       }
       if ((GET_CLASS(ch) == CLASS_COMMANDO) &&
         (spell_info[spl].min_level_commando > GET_LEVEL(ch))) {
         send_to_char("Sorry, you can't do that.\n\r", ch);
         return;
       }
  } else {
    sprintf(buf,"WIZINFO: %s casts %s.",GET_NAME(ch),argument);
    log_s(buf);
    wizlog(buf,GET_LEVEL(ch)+1,5);
  }

  argument+=qend+1;  /* Point to the last ' */
  for(;*argument == ' '; argument++);

  /* Locate targets */

  target_ok = FALSE;
  tar_char = 0;
  tar_obj = 0;

  if (!IS_SET(spell_info[spl].targets, TAR_IGNORE)) {

    argument = one_argument(argument, name);

     if (*name) {
       if (spl== SPELL_LOCATE_OBJECT && strstr(name,"token")) {
         send_to_char("Tokens cannot be located.\n\r",ch);
         return;
       }

       if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
        if ((tar_char = get_char_room_vis(ch, name)))
          target_ok = TRUE;

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
        if ((tar_char = get_char_vis(ch, name)))
          target_ok = TRUE;

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
        if ((tar_obj = get_obj_in_list_vis(ch, name, ch->carrying)))
          target_ok = TRUE;

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
        if ((tar_obj = get_obj_in_list_vis(ch, name, world[CHAR_REAL_ROOM(ch)].contents)))
          target_ok = TRUE;

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
        if ((tar_obj = get_obj_vis(ch, name)))
          target_ok = TRUE;

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP))
      {
        if ((tar_obj = get_obj_equipped_by_name(ch, name)))
        {
            target_ok = TRUE;
        }
      }

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY))
        if (str_cmp(GET_NAME(ch), name) == 0) {
          tar_char = ch;
          target_ok = TRUE;
        }

      /* Addition to allow IMM+ to cast any spell on a target - Ranger Feb 99 */
      if(!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY) && GET_LEVEL(ch)>=LEVEL_IMM)
        if((tar_char = get_char_room_vis(ch, name)))
          target_ok = TRUE;

      /* to prevent knowing no mob or obj in game and taking 0 mana */
      if(spl==SPELL_LOCATE_CHARACTER || spl==SPELL_LOCATE_OBJECT || spl==SPELL_CLAIRVOYANCE)
        target_ok=TRUE;

    } else { /* No argument was typed */
      if (IS_SET(spell_info[spl].targets, TAR_FIGHT_SELF))
        if (ch->specials.fighting) {
          tar_char = ch;
          target_ok = TRUE;
      }

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_FIGHT_VICT))
        if (ch->specials.fighting) {
          tar_char = ch->specials.fighting;
          target_ok = TRUE;
        }

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
        tar_char = ch;
        target_ok = TRUE;
      }

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM) &&
           !IS_SET(spell_info[spl].targets, TAR_FIGHT_VICT)) {
        tar_char = ch;
        target_ok = TRUE;
      }
    }

  } else {
    target_ok = TRUE; /* No target, is a good target */
  }

  if (!target_ok) {
    if (*name) {
      if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
        send_to_char("Nobody here by that name.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
        send_to_char("Nobody playing by that name.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
        send_to_char("You are not carrying anything like that.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
        send_to_char("Nothing here by that name.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
        send_to_char("Nothing at all by that name.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP))
        send_to_char("You are not wearing anything like that.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
        send_to_char("Nothing at all by that name.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_SELF_ONLY))
        send_to_char("You may only cast this spell on yourself.\n\r", ch);
    }
    else { /* Nothing was given as argument */
      if (spell_info[spl].targets < TAR_OBJ_INV)
        send_to_char("Who should the spell be cast upon?\n\r", ch);
      else
        send_to_char("What should the spell be cast upon?\n\r", ch);
    }
    return;
  }

  /* TARGET IS OK */
  if ((tar_char == ch) && IS_SET(spell_info[spl].targets, TAR_SELF_NONO)){
    send_to_char("You can not cast this spell upon yourself.\n\r", ch);
    return;
  }
  /* Addition to allow IMM+ to cast any spell on a target - Ranger Feb 99 */
  if ((tar_char != ch) && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY) && GET_LEVEL(ch)<LEVEL_IMM) {
    send_to_char("You can only cast this spell upon yourself.\n\r", ch);
    return;
  }
  if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tar_char)) {
    send_to_char("You are afraid that it could harm your master.\n\r", ch);
    return;
  }

  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,DOUBLE_MANA)) mult=20;
  else mult=10;

  if (GET_LEVEL(ch) < LEVEL_IMM) {
    if (GET_MANA(ch) < USE_MANA(ch, spl)*mult/10) {
      send_to_char("You can't summon enough energy to cast the spell.\n\r",ch);
      return;
    }
  }

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_MAGIC) && GET_LEVEL(ch)<LEVEL_IMM) 
  {
    send_to_char("Your magic has been absorbed by the surroundings.\n\r",ch);
    if(GET_LEVEL(ch) < LEVEL_IMM) GET_MANA(ch) -= (USE_MANA(ch, spl)*mult/10);
    else GET_MANA(ch) -= spell_info[spl].min_usesmana*mult/10;
    return;
  }

  if (spl != SPELL_VENTRILOQUATE) say_spell(ch, spl);
/*  WAIT_STATE(ch, spell_info[spl].beats);*/
  if (affected_by_spell(ch,SPELL_QUICK) && spl != SPELL_WRATH_ANCIENTS)
    WAIT_STATE(ch, PULSE_VIOLENCE/2); /* If affected by Quick cast twice in 1 round - wrath cannot be quickened */
  else WAIT_STATE(ch, PULSE_VIOLENCE);

  /* Druid SC4: Elemental Form - Bypasses concentration check. */
  if (!(IS_MORTAL(ch) && check_subclass(ch, SC_DRUID, 4) && ench_enchanted_by(ch, ENCH_NAME_ELEMENTAL_FORM, 0))) {
    con_amt = 0;
    if (GET_CLASS(ch) == CLASS_PALADIN ||
      GET_CLASS(ch) == CLASS_NINJA ||
      GET_CLASS(ch) == CLASS_COMMANDO ||
      GET_CLASS(ch) == CLASS_ANTI_PALADIN ||
      GET_CLASS(ch) == CLASS_BARD) {
      if (enchanted_by_type(ch, ENCHANT_SHOGUN) ||
        enchanted_by_type(ch, ENCHANT_COMMANDER) ||
        enchanted_by_type(ch, ENCHANT_DARKLORDLADY) ||
        enchanted_by_type(ch, ENCHANT_LORDLADY) ||
        enchanted_by_type(ch, ENCHANT_CONDUCTOR)) {
        con_amt += 50;
      }
    }

    /* Inner Peace */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_MYSTIC, 2)) con_amt += 50;

    /* Focus */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_CRUSADER, 2)) con_amt += 50;

    percent = number(1, 1001);
    conc = 1;
    if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, HALF_CONC)) conc = 2;
    if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, QRTR_CONC)) conc = 4;

    con_amt += (int)(int_app[GET_INT(ch)].conc + wis_app[GET_WIS(ch)].conc);
    con_amt += (int)ch->skills[spl].learned * 10;
    con_amt /= conc;


    if (percent > con_amt) { /* 100.1% is failure */
      if (castlog) {
        fl = fopen("castlog", "a+");
        fprintf(fl, "%s,%d,%d,%d,%d,0\n\r", GET_NAME(ch), percent, con_amt, GET_WIS(ch), GET_INT(ch));
        fclose(fl);
      }
      send_to_char("You lost your concentration!\n\r", ch);
      if (GET_LEVEL(ch) < LEVEL_IMM) GET_MANA(ch) -= ((USE_MANA(ch, spl)) >> 1) * mult / 10;
      else GET_MANA(ch) -= ((spell_info[spl].min_usesmana) >> 1) * mult / 10;
      return;
    }
  }

  if(castlog) {
    fl=fopen("castlog","a+");
    fprintf(fl,"%s,%d,%d,%d,%d,1\n\r",GET_NAME(ch),percent,con_amt,GET_WIS(ch),GET_INT(ch));
    fclose(fl);
  }
  send_to_char("Ok.\n\r",ch);

  /* ACT_SHIELD check Added by Ranger - May 96 */
  if(tar_char && IS_NPC(tar_char)) {
    if(IS_SET(tar_char->specials.act, ACT_SHIELD)) {
      act("Your magic doesn't seem to affect $N.",FALSE, ch, 0, tar_char, TO_CHAR);
      act("$N's magic doesn't seem to affect you.",FALSE, tar_char, 0, ch, TO_CHAR);
      act("$n's magic doesn't seem to affect $N.",FALSE, ch, 0, tar_char, TO_NOTVICT);

      if(GET_LEVEL(ch) < LEVEL_IMM) GET_MANA(ch) -= (USE_MANA(ch, spl))*mult/10;
      else GET_MANA(ch) -= spell_info[spl].min_usesmana*mult/10;

      if(spl!=SPELL_CLAIRVOYANCE &&
         spl!=SPELL_LOCATE_CHARACTER &&
         spl!=SPELL_VENTRILOQUATE) {
        /* Shun: Don't set fighting at all unless the two chars are
           in the same room */

        if (CHAR_REAL_ROOM(ch) == CHAR_REAL_ROOM(tar_char)) {
          if(GET_POS(tar_char) > POSITION_STUNNED) {
            if(!(tar_char->specials.fighting)) {
               set_fighting(tar_char, ch);
               GET_POS(tar_char) = POSITION_FIGHTING;
            }
          }

          if(GET_POS(ch) > POSITION_STUNNED) {
            if(!(ch->specials.fighting)) {
              set_fighting(ch, tar_char);
              GET_POS(ch) = POSITION_FIGHTING;
            }
          }
        }
      }
      return;
    }
  }

  int cost_reduction_chance = 0;

  /* Focus */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_CRUSADER, 2)) {
    cost_reduction_chance += 10;
  }

  if (IS_MORTAL(ch)) {
    mana_cost = (USE_MANA(ch, spl)) * mult / 10;

    switch (GET_LEVEL(ch)) {
    case 41:
      cost_reduction_chance += 10;
      if (chance(cost_reduction_chance)) mana_cost = mana_cost * 99 / 100;
      break;
    case 42:
      cost_reduction_chance += 11;
      if (chance(cost_reduction_chance)) mana_cost = mana_cost * 98 / 100;
      break;
    case 43:
      cost_reduction_chance += 12;
      if (chance(cost_reduction_chance)) mana_cost = mana_cost * 97 / 100;
      break;
    case 44:
      cost_reduction_chance += 13;
      if (chance(cost_reduction_chance)) mana_cost = mana_cost * 96 / 100;
      break;
    case 45:
      cost_reduction_chance += 15;
      if (chance(cost_reduction_chance)) mana_cost = mana_cost * 95 / 100;
      break;
    case 46:
      cost_reduction_chance += 17;
      if (chance(cost_reduction_chance)) mana_cost = mana_cost * 94 / 100;
      break;
    case 47:
      cost_reduction_chance += 19;
      if (chance(cost_reduction_chance)) mana_cost = mana_cost * 93 / 100;
      break;
    case 48:
      cost_reduction_chance += 21;
      if (chance(cost_reduction_chance)) mana_cost = mana_cost * 92 / 100;
      break;
    case 49:
      cost_reduction_chance += 23;
      if (chance(cost_reduction_chance)) mana_cost = mana_cost * 91 / 100;
      break;
    case 50:
      cost_reduction_chance += 25;
      if (chance(cost_reduction_chance)) mana_cost = mana_cost * 90 / 100;
      break;
    default:
      break;
    }

    GET_MANA(ch) -= mana_cost;
/*
    if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, MANABURN)) {
      send_to_char("Your magic burns into your soul!\n\r", ch);
      mana_cost=MAX(mana_cost,0);
      damage(ch,ch,mana_cost,TYPE_UNDEFINED,DAM_NO_BLOCK);
    }
*/
  }
  else {
    GET_MANA(ch) -= spell_info[spl].min_usesmana*mult/10;
  }

  if(tar_char)
    sprintf(buf,"%d %s",spl,GET_NAME(tar_char));
  else
    sprintf(buf,"%d",spl);

  if(signal_char(ch,ch,MSG_SPELL_CAST,buf)) return; /* Linerfix, changed ..(ch,0,MSG_.. to ..(ch,ch,MSG_.. */

  if(spl != SPELL_LOCATE_OBJECT)
    ((*spell_info[spl].spell_pointer) (GET_LEVEL(ch), ch, argument, SPELL_TYPE_SPELL, tar_char, tar_obj));
  else
    spell_locate_object (GET_LEVEL(ch), ch, 0, (struct obj_data*)name);

}

/* Mob casting routine for mob spell attacks - called from
   mob_attack in fight.c

   Proc expects the same thing as the regular do_cast routine.
   i.e.  ch is the mob casting
         arg is 'spell' <target>  (spell in single quotes)

   Added by Ranger - Jan 97
*/

void do_mob_cast(CHAR *ch, char *argument, int spell) {

  struct obj_data *tar_obj;
  struct char_data *tar_char;
  char name[MAX_STRING_LENGTH];
  int qend, spl, i;
  bool target_ok;

  if (!IS_NPC(ch)) return;

  argument = skip_spaces(argument);

  /* If there is no chars in argument */
  if (!(*argument)) {
    send_to_char("Cast which what where?\n\r", ch);
    return;
  }

  if (*argument != '\'') {
    send_to_char("Magic must always be enclosed by the magic symbols:'\n\r",ch);
    return;
  }

  /* Locate the last quote && lowercase the magic words (if any) */

  for (qend=1; *(argument+qend) && (*(argument+qend) != '\'') ; qend++)
    *(argument+qend) = LOWER(*(argument+qend));

  if (*(argument+qend) != '\'') {
    send_to_char("Magic must always be enclosed by the magic symbols: '\n\r",ch);
    return;
  }

  spl = old_search_block(argument, 1, qend-1, (const char * const * const)spells, 0);

  if (!spl) {
    send_to_char("Your lips do not move, no magic appears.\n\r",ch);
    return;
  }

  if ((spl < 0) || (spl > MAX_SPL_LIST) || !spell_info[spl].spell_pointer) {
    switch (number(1,5)) {
      case 1: send_to_char("Bylle Grylle Grop Gryf???\n\r", ch); break;
      case 2: send_to_char("Olle Bolle Snop Snyf?\n\r",ch); break;
      case 3: send_to_char("Olle Grylle Bolle Bylle?!?\n\r",ch); break;
      case 4: send_to_char("Gryffe Olle Gnyffe Snop???\n\r",ch); break;
      default: send_to_char("Bolle Snylle Gryf Bylle?!!?\n\r",ch); break;
    }
    return;
  }

  if (GET_POS(ch) < spell_info[spl].minimum_position) {
    switch(GET_POS(ch)) {
      case POSITION_SLEEPING :
     send_to_char("You dream about great magical powers.\n\r", ch);
        return;
     break;
      case POSITION_RESTING :
     send_to_char("You can't concentrate enough while resting.\n\r",ch);
        return;
     break;
      case POSITION_SITTING :
     send_to_char("You can't do this sitting!\n\r", ch);
        return;
     break;
      case POSITION_FIGHTING :
      case POSITION_STUNNED :
     break;
      default:
     send_to_char("It seems like you're in a pretty bad shape!\n\r",ch);
        return;
     break;
    } /* Switch */
  }

  argument+=qend+1;     /* Point to the last ' */
  for(;*argument == ' '; argument++);

  /* Locate targets */

  target_ok = FALSE;
  tar_char = 0;
  tar_obj = 0;

  if (!IS_SET(spell_info[spl].targets, TAR_IGNORE)) {

    argument = one_argument(argument, name);

    if (*name) {
      if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
        if ((tar_char = get_char_room_vis(ch, name)))
          target_ok = TRUE;

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
        if ((tar_char = get_char_vis(ch, name)))
          target_ok = TRUE;

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
        if ((tar_obj = get_obj_in_list_vis(ch, name, ch->carrying)))
          target_ok = TRUE;

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
        if ((tar_obj = get_obj_in_list_vis(ch, name, world[CHAR_REAL_ROOM(ch)].contents)))
          target_ok = TRUE;

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
        if ((tar_obj = get_obj_vis(ch, name)))
          target_ok = TRUE;

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP)) {
        for(i=0; i<MAX_WEAR && !target_ok; i++)
          if (ch->equipment[i] && str_cmp(name, ch->equipment[i]->name) == 0) {
            tar_obj = ch->equipment[i];
         target_ok = TRUE;
       }
      }

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY))
        if (str_cmp(GET_NAME(ch), name) == 0) {
          tar_char = ch;
          target_ok = TRUE;
        }

    } else { /* No argument was typed */

      if (IS_SET(spell_info[spl].targets, TAR_FIGHT_SELF))
        if (ch->specials.fighting) {
          tar_char = ch;
          target_ok = TRUE;
        }

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_FIGHT_VICT))
        if (ch->specials.fighting) {
          /* WARNING, MAKE INTO POINTER */
          tar_char = ch->specials.fighting;
          target_ok = TRUE;
        }

      if (!target_ok && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
        tar_char = ch;
        target_ok = TRUE;
      }

      if (!target_ok &&
          IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM) &&
          !IS_SET(spell_info[spl].targets, TAR_FIGHT_VICT)) {
        tar_char = ch;
        target_ok = TRUE;
      }

    }

  } else {
    target_ok = TRUE; /* No target, is a good target */
  }

  if (!target_ok) {
    if (*name) {
      if (IS_SET(spell_info[spl].targets, TAR_CHAR_ROOM))
        send_to_char("Nobody here by that name.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_CHAR_WORLD))
        send_to_char("Nobody playing by that name.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_OBJ_INV))
        send_to_char("You are not carrying anything like that.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_OBJ_ROOM))
        send_to_char("Nothing here by that name.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
        send_to_char("Nothing at all by that name.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_OBJ_EQUIP))
        send_to_char("You are not wearing anything like that.\n\r", ch);
      else if (IS_SET(spell_info[spl].targets, TAR_OBJ_WORLD))
        send_to_char("Nothing at all by that name.\n\r", ch);

    } else { /* Nothing was given as argument */
      if (spell_info[spl].targets < TAR_OBJ_INV)
        send_to_char("Who should the spell be cast upon?\n\r", ch);
      else
        send_to_char("What should the spell be cast upon?\n\r", ch);
    }
    return;
  }

  /* TARGET IS OK */
/*
    if ((tar_char == ch) && IS_SET(spell_info[spl].targets, TAR_SELF_NONO)){
      send_to_char("You can not cast this spell upon yourself.\n\r", ch);
      return;
    }
  if ((tar_char != ch) && IS_SET(spell_info[spl].targets, TAR_SELF_ONLY)) {
      send_to_char("You can only cast this spell upon yourself.\n\r", ch);
      return;
    }
 */
 if (IS_AFFECTED(ch, AFF_CHARM) && (ch->master == tar_char)) {
    send_to_char("You are afraid that it could harm your master.\n\r", ch);
    return;
  }

  if (spell) { /* Otherwise its a skill */
    if (GET_MANA(ch) < spell_info[spl].min_usesmana) {
      send_to_char("You can't summon enough energy to cast the spell.\n\r",ch);
      return;
    }

    if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_MAGIC)) {
      send_to_char("Your magic has been absorbed by the surrounding.\n\r", ch);
      GET_MANA(ch) -= spell_info[spl].min_usesmana;
      return;
    }

    if (spl != SPELL_VENTRILOQUATE) say_spell(ch, spl);
  }

  send_to_char("Ok.\n\r",ch);

  ch->specials.att_timer=2; /* 2 Rounds between casts */

  if (spl != SPELL_LOCATE_OBJECT)
    ((*spell_info[spl].spell_pointer) (GET_LEVEL(ch), ch, argument, SPELL_TYPE_SPELL, tar_char, tar_obj));

  if(spell) GET_MANA(ch) -= spell_info[spl].min_usesmana;

  return;
}

void assign_spell_pointers(void) {
  for (int i = 0; i < MAX_SPL_LIST + 1; i++) {
    spell_info[i].spell_pointer = NULL;
  }

  //                                                        Minimum Class Level Required
  /*     Spell #,                   BT, Minimum Position,  MU, CL, NI, PA, AP, AV, BA, CO, Mana, Target Types,                                                                 Spell Function */
  SPELLO(SPELL_ANIMATE_DEAD,        30, POSITION_STANDING, 22, 24, 57, 57, 57, 20, 57, 57,   50, TAR_OBJ_ROOM,                                                                 cast_animate_dead);
  SPELLO(SPELL_ARMOR,               30, POSITION_STANDING,  5,  1, 57,  2, 57,  1, 57, 57,    5, TAR_CHAR_ROOM,                                                                cast_armor);
  SPELLO(SPELL_BLESS,               30, POSITION_FIGHTING, 57,  5, 57,  6, 57,  3, 57, 57,    5, TAR_CHAR_ROOM,                                                                cast_bless);
  SPELLO(SPELL_BLINDNESS,           30, POSITION_STANDING,  8,  6,  6, 57,  7,  4, 57, 57,   15, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_blindness);
  SPELLO(SPELL_BLINDNESS_DUST,      30, POSITION_FIGHTING, 57, 57, 16, 57, 17, 19, 57, 57,   20, TAR_IGNORE,                                                                   cast_blindness_dust);
  SPELLO(SPELL_BLOOD_LUST,          30, POSITION_FIGHTING, 57, 57, 57, 57, 45, 57, 57, 57,   70, TAR_SELF_ONLY,                                                                cast_blood_lust);
  SPELLO(SPELL_BURNING_HANDS,       30, POSITION_FIGHTING,  5, 57, 57, 57, 57,  1, 57,  3,   17, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_burning_hands);
  SPELLO(SPELL_CALL_LIGHTNING,      30, POSITION_FIGHTING, 57, 12, 57, 57, 57, 57, 57, 12,   20, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_call_lightning);
  SPELLO(SPELL_CHARM_PERSON,        30, POSITION_STANDING, 14, 57, 57, 57, 57, 12, 57, 57,   50, TAR_CHAR_ROOM | TAR_SELF_NONO,                                                cast_charm_person);
  SPELLO(SPELL_CHILL_TOUCH,         30, POSITION_FIGHTING,  3, 57, 57, 57,  3, 57, 57,  2,   15, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_chill_touch);
  SPELLO(SPELL_CLAIRVOYANCE,        30, POSITION_STANDING, 18, 16, 21, 57, 57, 57, 57, 57,   35, TAR_CHAR_WORLD,                                                               cast_clairvoyance);
  SPELLO(SPELL_CLONE,               30, POSITION_STANDING, 27, 57, 57, 57, 57, 27, 57, 57,  100, TAR_OBJ_INV,                                                                  cast_clone);
  SPELLO(SPELL_COLOR_SPRAY,         30, POSITION_FIGHTING, 11, 57, 57, 57, 57, 57, 57, 10,   23, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_color_spray);
  SPELLO(SPELL_CONFLAGRATION,       30, POSITION_FIGHTING, 30, 57, 57, 57, 57, 57, 57, 57,  200, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_conflagration);
  SPELLO(SPELL_CONTROL_WEATHER,     30, POSITION_STANDING, 10, 13, 11, 57, 57, 57, 57, 57,   25, TAR_IGNORE,                                                                   cast_control_weather);
  SPELLO(SPELL_CONVERGENCE,         30, POSITION_FIGHTING, 21, 57, 27, 57, 57, 57, 57, 57,   20, TAR_CHAR_ROOM,                                                                cast_convergence);
  SPELLO(SPELL_CREATE_FOOD,         30, POSITION_STANDING, 57,  3, 57,  3, 57, 57, 57, 57,    5, TAR_IGNORE,                                                                   cast_create_food);
  SPELLO(SPELL_CREATE_WATER,        30, POSITION_STANDING, 57,  2, 57,  4, 57, 57, 57, 57,    5, TAR_OBJ_INV | TAR_OBJ_EQUIP,                                                  cast_create_water);
  SPELLO(SPELL_CURE_BLIND,          30, POSITION_STANDING, 57,  4,  9,  7, 57,  5, 57, 57,   15, TAR_CHAR_ROOM,                                                                cast_cure_blind);
  SPELLO(SPELL_CURE_CRITIC,         30, POSITION_FIGHTING, 57,  9, 57, 10, 57,  7, 57, 57,   25, TAR_CHAR_ROOM,                                                                cast_cure_critic);
  SPELLO(SPELL_CURE_CRITIC_SPRAY,   30, POSITION_FIGHTING, 57, 13, 57, 57, 57, 16, 57, 57,   38, TAR_IGNORE,                                                                   cast_cure_critic_spray);
  SPELLO(SPELL_CURE_LIGHT,          30, POSITION_FIGHTING, 57,  1,  4,  1, 57,  1, 57, 57,   15, TAR_CHAR_ROOM,                                                                cast_cure_light);
  SPELLO(SPELL_CURE_LIGHT_SPRAY,    30, POSITION_FIGHTING, 57,  4, 57, 57, 57, 57, 57, 57,   23, TAR_IGNORE,                                                                   cast_cure_light_spray);
  SPELLO(SPELL_CURE_SERIOUS,        30, POSITION_FIGHTING, 57,  6, 16,  5, 57,  4, 57, 57,   18, TAR_CHAR_ROOM,                                                                cast_cure_serious);
  SPELLO(SPELL_CURE_SERIOUS_SPRAY,  30, POSITION_FIGHTING, 57, 10, 57, 57, 57, 57, 57, 57,   27, TAR_IGNORE,                                                                   cast_cure_serious_spray);
  SPELLO(SPELL_CURSE,               30, POSITION_STANDING, 12, 57, 57, 57,  8, 57, 57, 57,   20, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO | TAR_OBJ_INV | TAR_OBJ_ROOM,  cast_curse);
  SPELLO(SPELL_DEATH_SPRAY,         30, POSITION_FIGHTING, 23, 57, 57, 57, 57, 17, 57, 24,   80, TAR_IGNORE,                                                                   cast_death_spray);
  SPELLO(SPELL_DETECT_ALIGNMENT,    30, POSITION_STANDING,  4,  4, 57,  4,  4,  3, 57, 57,   25, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_detect_alignment);
  SPELLO(SPELL_DETECT_INVISIBLE,    30, POSITION_STANDING,  2,  5,  1,  5,  5,  3, 57, 57,   25, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_detect_invisibility);
  SPELLO(SPELL_DETECT_MAGIC,        30, POSITION_STANDING,  2,  3, 57, 57, 57, 57, 57, 57,    5, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_detect_magic);
  SPELLO(SPELL_DETECT_POISON,       30, POSITION_STANDING,  4,  2, 57, 57,  2, 57,  4, 57,    5, TAR_CHAR_ROOM,                                                                cast_detect_poison);
  SPELLO(SPELL_DISINTEGRATE,        30, POSITION_FIGHTING, 27, 57, 57, 57, 57, 57, 57, 28,  150, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO | TAR_OBJ_INV | TAR_OBJ_ROOM,  cast_disintegrate);
  SPELLO(SPELL_DISPEL_EVIL,         30, POSITION_FIGHTING, 57, 10, 57, 19, 57, 57, 57, 57,   25, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_dispel_evil);
  SPELLO(SPELL_DISPEL_GOOD,         30, POSITION_FIGHTING, 57, 11, 57, 57, 22, 57, 57, 57,   25, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_dispel_good);
  SPELLO(SPELL_DISPEL_MAGIC,        30, POSITION_STANDING, 19, 17, 57, 57, 57, 57, 57, 57,   50, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM,                                   cast_dispel_magic);
  SPELLO(SPELL_DISPEL_SANCT,        30, POSITION_STANDING, 57, 57, 57, 57, 57, 57, 57, 57,   10, TAR_CHAR_ROOM,                                                                cast_dispel_sanct);
  SPELLO(SPELL_DIVINE_INTERVENTION, 30, POSITION_STANDING, 57, 50, 57, 57, 57, 57, 57, 57,  500, TAR_CHAR_ROOM,                                                                cast_divine_intervention);
  SPELLO(SPELL_DIVINE_WIND,         30, POSITION_FIGHTING, 57, 57, 40, 57, 57, 57, 57, 57,   65, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_divine_wind);
  SPELLO(SPELL_EARTHQUAKE,          30, POSITION_FIGHTING, 57,  7, 57, 57, 57, 57, 57,  6,   15, TAR_IGNORE,                                                                   cast_earthquake);
  SPELLO(SPELL_ELECTRIC_SHOCK,      30, POSITION_FIGHTING, 26, 57, 57, 57, 57, 25, 57, 29,  100, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_electric_shock);
  SPELLO(SPELL_ENCHANT_ARMOR,       30, POSITION_STANDING, 21, 57, 57, 57, 57, 57, 57, 57,  100, TAR_OBJ_INV,                                                                  cast_enchant_armor);
  SPELLO(SPELL_ENCHANT_WEAPON,      30, POSITION_STANDING, 12, 57, 57, 57, 57, 18, 57, 57,   50, TAR_OBJ_INV | TAR_OBJ_EQUIP,                                                  cast_enchant_weapon);
  SPELLO(SPELL_ENDURE,              30, POSITION_STANDING, 57, 57,  3, 57, 57,  3, 57, 57,   10, TAR_CHAR_ROOM,                                                                cast_endure);
  SPELLO(SPELL_ENERGY_DRAIN,        30, POSITION_FIGHTING, 13, 57, 57, 57, 15, 14, 57, 57,   35, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_energy_drain);
  SPELLO(SPELL_EVIL_BLESS,          30, POSITION_STANDING, 57, 20, 57, 57, 57, 18, 57, 57,   30, TAR_CHAR_ROOM,                                                                cast_evil_bless);
  SPELLO(SPELL_EVIL_WORD,           30, POSITION_FIGHTING, 57, 23, 57, 57, 26, 57, 57, 57,   80, TAR_IGNORE,                                                                   cast_evil_word);
  SPELLO(SPELL_FEAR,                30, POSITION_FIGHTING,  9, 57, 57, 57, 10,  9, 57, 57,   15, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_fear);
  SPELLO(SPELL_FIREBALL,            30, POSITION_FIGHTING, 15, 57, 57, 57, 20, 15, 57, 15,   25, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_fireball);
  SPELLO(SPELL_FIREBREATH,          30, POSITION_FIGHTING, 28, 57, 57, 57, 57, 27, 57, 57,  100, TAR_CHAR_ROOM,                                                                cast_firebreath);
  SPELLO(SPELL_FLAMESTRIKE,         30, POSITION_FIGHTING, 57, 11, 57, 10, 57,  6, 57, 57,   25, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_flamestrike);
  SPELLO(SPELL_FLY,                 30, POSITION_STANDING, 10, 57, 57, 57, 57,  7, 57, 57,   25, TAR_CHAR_ROOM,                                                                cast_fly);
  SPELLO(SPELL_FORGET,              30, POSITION_FIGHTING, 11, 57, 57, 57, 14, 10, 57, 57,   15, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_forget);
  SPELLO(SPELL_FURY,                30, POSITION_FIGHTING, 57, 57, 57, 21, 57, 23, 57, 57,   60, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_fury);
  SPELLO(SPELL_GREAT_MIRACLE,       30, POSITION_STANDING, 57, 30, 57, 57, 57, 30, 57, 57,  200, TAR_IGNORE,                                                                   cast_great_miracle);
  SPELLO(SPELL_HARM,                30, POSITION_FIGHTING, 57, 15, 57, 57, 57, 57, 57, 14,   35, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_harm);
  SPELLO(SPELL_HASTE,               30, POSITION_FIGHTING, 57, 45, 57, 57, 57, 26, 57, 57,  120, TAR_SELF_ONLY,                                                                cast_haste);
  SPELLO(SPELL_HEAL,                30, POSITION_FIGHTING, 57, 14, 29, 21, 57, 15, 57, 57,   50, TAR_CHAR_ROOM,                                                                cast_heal);
  SPELLO(SPELL_HEAL_SPRAY,          30, POSITION_FIGHTING, 57, 22, 57, 57, 57, 27, 57, 57,  100, TAR_IGNORE,                                                                   cast_heal_spray);
  SPELLO(SPELL_HELL_FIRE,           30, POSITION_FIGHTING, 57, 57, 57, 57, 28, 24, 57, 57,  100, TAR_IGNORE,                                                                   cast_hell_fire);
  SPELLO(SPELL_HOLD,                30, POSITION_STANDING, 21, 57, 24, 26, 57, 19, 57, 57,   20, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_hold);
  SPELLO(SPELL_HOLY_BLESS,          30, POSITION_STANDING, 57, 19, 57, 57, 57, 17, 57, 57,   30, TAR_CHAR_ROOM,                                                                cast_holy_bless);
  SPELLO(SPELL_HOLY_WORD,           30, POSITION_FIGHTING, 57, 23, 57, 25, 57, 57, 57, 57,   80, TAR_IGNORE,                                                                   cast_holy_word);
  SPELLO(SPELL_HYPNOTIZE,           30, POSITION_STANDING, 57, 57, 22, 57, 27, 57, 57, 57,   40, TAR_CHAR_ROOM | TAR_SELF_NONO,                                                cast_hypnotize);
  SPELLO(SPELL_ICEBALL,             30, POSITION_FIGHTING, 19, 57, 57, 57, 23, 18, 57, 20,   30, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_iceball);
  SPELLO(SPELL_IDENTIFY,            30, POSITION_STANDING,  7, 57, 57, 57, 57, 10,  9, 57,   25, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP,                   cast_identify);
  SPELLO(SPELL_IMP_INVISIBLE,       30, POSITION_STANDING, 25, 57, 57, 57, 29, 22, 57, 57,   30, TAR_SELF_ONLY,                                                                cast_imp_invisibility);
  SPELLO(SPELL_INFRAVISION,         30, POSITION_STANDING,  5, 57,  5, 57, 11,  4, 57, 57,   20, TAR_CHAR_ROOM,                                                                cast_infravision);
  SPELLO(SPELL_INVISIBLE,           30, POSITION_STANDING,  4, 57,  7, 57,  9,  6, 57, 57,   25, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP,                   cast_invisibility);
  SPELLO(SPELL_INVUL,               30, POSITION_STANDING, 29, 28, 57, 57, 57, 25, 57, 57,   50, TAR_SELF_ONLY,                                                                cast_invulnerability);
  SPELLO(SPELL_IRON_SKIN,           30, POSITION_STANDING, 57, 57, 57, 57, 57, 57, 57, 40,   50, TAR_CHAR_ROOM,                                                                cast_iron_skin);
  SPELLO(SPELL_LAY_HANDS,           30, POSITION_FIGHTING, 57, 57, 57, 15, 57, 57, 57, 57,  100, TAR_CHAR_ROOM,                                                                cast_lay_hands);
  SPELLO(SPELL_LETHAL_FIRE,         30, POSITION_FIGHTING, 24, 57, 57, 57, 57, 20, 57, 27,   50, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_lethal_fire);
  SPELLO(SPELL_LIGHTNING_BOLT,      30, POSITION_FIGHTING,  9, 57, 57, 57, 12, 57, 57,  7,   20, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_lightning_bolt);
  SPELLO(SPELL_LOCATE_CHARACTER,    30, POSITION_STANDING, 11, 14, 57, 57, 57, 11, 13, 57,   20, TAR_CHAR_WORLD,                                                               cast_locate_character);
  SPELLO(SPELL_LOCATE_OBJECT,       30, POSITION_STANDING,  6, 10, 14, 17, 57,  8, 10, 57,   50, TAR_OBJ_WORLD,                                                                cast_locate_object);
  SPELLO(SPELL_LUCK,                30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 57, 57, 57,   100, TAR_CHAR_ROOM,                                                                cast_luck);
  SPELLO(SPELL_MAGIC_MISSILE,       30, POSITION_FIGHTING,  1, 57, 57, 57,  1, 57, 57,  1,    5, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_magic_missile);
  SPELLO(SPELL_MANA_TRANSFER,       30, POSITION_FIGHTING, 18, 18, 57, 57, 57, 57, 57, 57,   50, TAR_CHAR_ROOM | TAR_SELF_NONO,                                                cast_mana_transfer);
  SPELLO(SPELL_MIRACLE,             30, POSITION_FIGHTING, 57, 25, 57, 57, 57, 22, 57, 57,  100, TAR_CHAR_ROOM,                                                                cast_miracle);
  SPELLO(SPELL_MYSTIC_SWIFTNESS,    30, POSITION_FIGHTING, 57, 57, 50, 57, 57, 42, 57, 57,  100, TAR_SELF_ONLY,                                                                cast_mystic_swiftness);
  SPELLO(SPELL_PARALYSIS,           30, POSITION_STANDING, 29, 57, 57, 57, 21, 28, 57, 57,   50, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_paralyze);
  SPELLO(SPELL_PERCEIVE,            30, POSITION_STANDING, 40, 57, 57, 57, 40, 40, 57, 57,   40, TAR_SELF_ONLY,                                                                cast_perceive);
  SPELLO(SPELL_POISON,              30, POSITION_STANDING, 57,  8, 12, 57,  6, 57, 57, 57,   10, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO | TAR_OBJ_INV,                 cast_poison);
  SPELLO(SPELL_POISON_SMOKE,        30, POSITION_FIGHTING, 57, 57, 17, 57, 16, 19, 57, 57,   20, TAR_IGNORE,                                                                   cast_poison_smoke);
  SPELLO(SPELL_PROTECT_FROM_EVIL,   30, POSITION_STANDING, 57,  6, 57, 24, 57, 20, 57, 57,   30, TAR_SELF_ONLY,                                                                cast_protection_from_evil);
  SPELLO(SPELL_PROTECT_FROM_GOOD,   30, POSITION_STANDING, 57,  6, 57, 57, 18, 57, 57, 57,   30, TAR_SELF_ONLY,                                                                cast_protection_from_good);
  SPELLO(SPELL_PW_KILL,             30, POSITION_FIGHTING, 26, 57, 57, 57, 30, 57, 57, 57,  100, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_power_word_kill);
  SPELLO(SPELL_QUICK,               30, POSITION_FIGHTING, 50, 57, 57, 57, 57, 57, 57, 57,  100, TAR_SELF_ONLY,                                                                cast_quick);
  SPELLO(SPELL_RAGE,                30, POSITION_FIGHTING, 57, 57, 57, 57, 50, 57, 57, 57,  130, TAR_SELF_ONLY,                                                                cast_rage);
  SPELLO(SPELL_REAPPEAR,            30, POSITION_STANDING, 10, 57, 57, 57, 57,  9, 57, 57,   20, TAR_OBJ_INV | TAR_OBJ_EQUIP,                                                  cast_reappear);
  SPELLO(SPELL_RECHARGE,            30, POSITION_STANDING, 17, 20, 57, 57, 57, 16, 57, 57,   30, TAR_OBJ_INV | TAR_OBJ_EQUIP,                                                  cast_recharge);
  SPELLO(SPELL_REGENERATION,        30, POSITION_FIGHTING, 28, 57, 57, 57, 57, 57, 57, 57,  100, TAR_CHAR_ROOM,                                                                cast_regeneration);
  SPELLO(SPELL_REJUVENATION,        30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 57, 57, 57,   80, TAR_CHAR_ROOM,                                                                cast_rejuvenation);
  SPELLO(SPELL_RELOCATION,          30, POSITION_STANDING, 27, 57, 57, 57, 57, 16, 57, 57,   50, TAR_CHAR_WORLD,                                                               cast_relocation);
  SPELLO(SPELL_REMOVE_CURSE,        30, POSITION_STANDING, 13, 12, 57, 57, 57, 21, 57, 57,   25, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP,                   cast_remove_curse);
  SPELLO(SPELL_REMOVE_IMP_INVIS,    30, POSITION_STANDING, 25, 57, 57, 57, 29, 22, 57, 57,   20, TAR_SELF_ONLY,                                                                cast_remove_improved_invis);
  SPELLO(SPELL_REMOVE_PARALYSIS,    30, POSITION_STANDING, 57, 14, 57, 57, 57,  9, 57, 57,   35, TAR_CHAR_ROOM,                                                                cast_remove_paralysis);
  SPELLO(SPELL_REMOVE_POISON,       30, POSITION_STANDING, 57,  9, 12, 14, 57,  5, 57, 57,   25, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP,                   cast_remove_poison);
  SPELLO(SPELL_REVEAL,              30, POSITION_STANDING,  6, 57, 57, 57, 57,  5, 57, 57,   30, TAR_CHAR_ROOM,                                                                cast_reveal);
  SPELLO(SPELL_RUSH,                30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 40, 57, 50,  100, TAR_SELF_ONLY,                                                                cast_rush);
  SPELLO(SPELL_SANCTUARY,           30, POSITION_STANDING, 57, 13, 57, 20, 57, 12, 57, 57,   50, TAR_CHAR_ROOM,                                                                cast_sanctuary);
  SPELLO(SPELL_SATIATE,             30, POSITION_STANDING, 57, 15, 57, 57, 57, 57, 57, 57,   10, TAR_CHAR_ROOM,                                                                cast_satiate);
  SPELLO(SPELL_SEARING_ORB,         30, POSITION_FIGHTING, 57, 16, 57, 57, 57, 57, 57, 57,   60, TAR_IGNORE,                                                                   cast_searing_orb);
  SPELLO(SPELL_SENSE_LIFE,          30, POSITION_STANDING, 57,  7, 10,  6, 13,  8, 57, 57,   20, TAR_SELF_ONLY,                                                                cast_sense_life);
  SPELLO(SPELL_SHOCKING_GRASP,      30, POSITION_FIGHTING,  7, 57, 57, 57, 57, 57, 57,  5,   19, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_shocking_grasp);
  SPELLO(SPELL_SLEEP,               30, POSITION_STANDING, 14, 57, 57, 57, 19, 11, 57, 57,   25, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_sleep);
  SPELLO(SPELL_SPHERE,              30, POSITION_STANDING, 18, 57, 57, 57, 57, 17, 57, 57,   50, TAR_SELF_ONLY,                                                                cast_sphere);
  SPELLO(SPELL_SPIRIT_LEVY,         30, POSITION_FIGHTING, 20, 57, 57, 57, 25, 10, 57, 57,   15, TAR_OBJ_ROOM,                                                                 cast_spirit_levy);
  SPELLO(SPELL_STRENGTH,            30, POSITION_STANDING,  7, 57, 57, 57, 57, 57, 57, 57,   10, TAR_SELF_ONLY,                                                                cast_strength);
  SPELLO(SPELL_SUMMON,              30, POSITION_STANDING, 57,  8, 57, 18, 57, 12, 57, 57,   50, TAR_CHAR_WORLD,                                                               cast_summon);
  SPELLO(SPELL_SUPER_HARM,          30, POSITION_FIGHTING, 57, 27, 57, 32, 57, 57, 57, 57,  100, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_super_harm);
  SPELLO(SPELL_TELEPORT,            30, POSITION_FIGHTING,  8, 57, 57, 57, 57,  7, 57, 57,   25, TAR_SELF_ONLY,                                                                cast_teleport);
  SPELLO(SPELL_THUNDERBALL,         30, POSITION_FIGHTING, 28, 57, 57, 57, 57, 27, 57, 57,  200, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_thunderball);
  SPELLO(SPELL_TOTAL_RECALL,        30, POSITION_STANDING, 57, 18, 57, 57, 57, 13, 57, 57,   30, TAR_IGNORE,                                                                   cast_total_recall);
  SPELLO(SPELL_VAMPIRIC,            30, POSITION_FIGHTING, 18, 57, 57, 57, 24, 57, 57, 32,   40, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_vampiric_touch);
  SPELLO(SPELL_VENTRILOQUATE,       30, POSITION_STANDING,  1, 57, 57, 57, 57, 57, 5,  57,    5, TAR_CHAR_ROOM | TAR_SELF_NONO | TAR_OBJ_ROOM,                                 cast_ventriloquate);
  SPELLO(SPELL_VITALITY,            30, POSITION_FIGHTING,  8, 16, 57, 14, 57, 10, 57, 57,   25, TAR_CHAR_ROOM,                                                                cast_vitality);
  SPELLO(SPELL_WIND_SLASH,          30, POSITION_FIGHTING, 57, 57, 30, 57, 57, 57, 57, 57,   85, TAR_IGNORE,                                                                   cast_wind_slash);
  SPELLO(SPELL_WORD_OF_RECALL,      30, POSITION_STANDING, 13, 11, 19, 57, 57,  6, 57, 19,   25, TAR_SELF_ONLY,                                                                cast_word_of_recall);
  // Enchanter
  SPELLO(SPELL_BLADE_BARRIER,       30, POSITION_STANDING, 30, 57, 57, 57, 57, 57, 57, 57,   50, TAR_SELF_ONLY,                                                                cast_blade_barrier);
  SPELLO(SPELL_PASSDOOR,            30, POSITION_STANDING, 30, 57, 57, 57, 57, 57, 57, 57,   50, TAR_IGNORE,                                                                   cast_passdoor);
  SPELLO(SPELL_ENGAGE,              30, POSITION_STANDING, 30, 57, 57, 57, 57, 57, 57, 57,   40, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_engage);
  SPELLO(SPELL_ETHEREAL_NATURE,     30, POSITION_STANDING, 30, 57, 57, 57, 57, 57, 57, 57,  100, TAR_SELF_ONLY,                                                                cast_ethereal_nature);
  SPELLO(SPELL_DISRUPT_SANCT,       30, POSITION_STANDING, 30, 57, 57, 57, 57, 57, 57, 57,  250, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_disrupt_sanct);
  // Archmage
  SPELLO(SPELL_METEOR,              30, POSITION_FIGHTING, 30, 57, 57, 57, 57, 57, 57, 57,    5, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_meteor);
  SPELLO(SPELL_ORB_PROTECTION,      30, POSITION_STANDING, 30, 30, 57, 57, 57, 57, 57, 57,   50, TAR_SELF_ONLY,                                                                cast_orb_protection);
  SPELLO(SPELL_FROSTBOLT,           30, POSITION_FIGHTING, 30, 57, 57, 57, 57, 57, 57, 30,   50, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_frostbolt);
  SPELLO(SPELL_WRATH_ANCIENTS,      30, POSITION_FIGHTING, 30, 57, 57, 57, 57, 57, 57, 57,  250, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_wrath_ancients);
  SPELLO(SPELL_DISTORTION,          30, POSITION_FIGHTING, 30, 57, 57, 57, 57, 57, 57, 57,  100, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_distortion);
  // Druid
  SPELLO(SPELL_WALL_THORNS,         30, POSITION_FIGHTING, 57, 30, 57, 57, 57, 57, 57, 57,  100, TAR_IGNORE,                                                                   cast_wall_thorns);
  // Templar
  SPELLO(SPELL_MAGIC_ARMAMENT,      30, POSITION_FIGHTING, 57, 30, 57, 57, 57, 57, 57, 57,   75, TAR_SELF_ONLY,                                                                cast_magic_armament);
  SPELLO(SPELL_FORTIFICATION,       30, POSITION_STANDING, 57, 30, 57, 57, 57, 57, 57, 57,  100, TAR_CHAR_ROOM,                                                                cast_fortification);
  SPELLO(SPELL_DIVINE_HAMMER,       30, POSITION_FIGHTING, 57, 30, 57, 57, 57, 57, 57, 57,  200, TAR_SELF_ONLY,                                                                cast_divine_hammer);
  // Ronin
  SPELLO(SPELL_BLUR,                30, POSITION_FIGHTING, 57, 57, 30, 57, 57, 57, 57, 57,   80, TAR_SELF_ONLY,                                                                cast_blur);
  // Mystic
  SPELLO(SPELL_DEBILITATE,          30, POSITION_FIGHTING, 57, 57, 30, 57, 57, 57, 57, 57,   20, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_debilitate);
  SPELLO(SPELL_TRANQUILITY,         30, POSITION_FIGHTING, 57, 57, 30, 57, 57, 57, 57, 57,  150, TAR_IGNORE,                                                                   cast_tranquility);
  // Cavalier
  SPELLO(SPELL_MIGHT,               30, POSITION_FIGHTING, 57, 30, 57, 30, 57, 57, 57, 57,   50, TAR_SELF_ONLY,                                                                cast_might);
  SPELLO(SPELL_WRATH_OF_GOD,        30, POSITION_FIGHTING, 57, 57, 57, 30, 57, 57, 57, 57,  100, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_wrath_of_god);
  SPELLO(SPELL_POWER_OF_DEVOTION,   30, POSITION_STANDING, 57, 57, 57, 30, 57, 57, 57, 57,  100, TAR_CHAR_ROOM,                                                                cast_power_of_devotion);
  // Crusader
  SPELLO(SPELL_RIGHTEOUSNESS,       30, POSITION_FIGHTING, 57, 57, 57, 30, 57, 57, 57, 57,   30, TAR_SELF_ONLY,                                                                cast_righteousness);
  SPELLO(SPELL_POWER_OF_FAITH,      30, POSITION_FIGHTING, 57, 57, 57, 30, 57, 57, 57, 57,   50, TAR_CHAR_ROOM,                                                                cast_power_of_faith);
  // Defiler
  SPELLO(SPELL_DESECRATE,           30, POSITION_STANDING, 57, 57, 57, 57, 30, 57, 57, 57,   70, TAR_OBJ_ROOM,                                                                 cast_desecrate);
  SPELLO(SPELL_BLACKMANTLE,         30, POSITION_FIGHTING, 57, 57, 57, 57, 30, 57, 57, 57,   70, TAR_SELF_ONLY,                                                                cast_blackmantle);
  // Infidel
  SPELLO(SPELL_SHADOW_WRAITH,       30, POSITION_FIGHTING, 57, 57, 57, 57, 30, 57, 57, 57,  100, TAR_SELF_ONLY,                                                                cast_shadow_wraith);
  SPELLO(SPELL_DUSK_REQUIEM,        30, POSITION_FIGHTING, 57, 57, 57, 57, 30, 57, 57, 57,   20, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_dusk_requiem);
  SPELLO(SPELL_WITHER,              30, POSITION_FIGHTING, 57, 57, 57, 57, 30, 57, 57, 57,   70, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_wither);
  // Mercenary
  SPELLO(SPELL_TREMOR,              30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 57, 57, 30,  130, TAR_IGNORE,                                                                   cast_tremor);
  SPELLO(SPELL_CLOUD_CONFUSION,     30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 57, 57, 30,  100, TAR_IGNORE,                                                                   cast_cloud_confusion);
  SPELLO(SPELL_INCENDIARY_CLOUD,    30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 57, 57, 30,  200, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_incendiary_cloud);
  // Legionnaire
  SPELLO(SPELL_RIMEFANG,            30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 57, 57, 30,  115, TAR_IGNORE,                                                                   cast_rimefang);
  SPELLO(SPELL_DEVASTATION,         30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 57, 57, 30,  150, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_devastation);

  // Other
  SPELLO(SPELL_DISENCHANT,          30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 57, 57, 57,   50, TAR_CHAR_ROOM,                                                                cast_disenchant);
  SPELLO(SPELL_GREAT_MANA,          30, POSITION_STANDING, 57, 57, 57, 57, 57, 57, 57, 57,   10, TAR_CHAR_ROOM,                                                                cast_great_mana);
  SPELLO(SPELL_MANA_HEAL,           30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 57, 57, 57,   10, TAR_SELF_ONLY,                                                                cast_mana_heal);
  SPELLO(SPELL_PETRIFY,             30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 57, 57, 57,  200, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_petrify);
  SPELLO(SPELL_RECOVER_MANA,        30, POSITION_STANDING, 57, 57, 57, 57, 57, 57, 57, 57,   10, TAR_CHAR_ROOM,                                                                cast_recover_mana);
}

/**
 * @brief Assigns text to the spell_text[] array, which contains the spell's
 *   name, message printed to a character when the spell is cast upon them,
 *   message printed to the room when a spell is cast upon another character,
 *   and the spell's wear-off message. The spell names are copied into the
 *   standard spells[] array, for legacy code purposes.
 */
void assign_spell_text(void) {
  for (int spell_nr = 0; spell_nr < NUMELEMS(spell_text); spell_nr++) {
    spell_text[spell_nr].name = "";
    spell_text[spell_nr].to_char_msg = "";
    spell_text[spell_nr].to_room_msg = "";
    spell_text[spell_nr].wear_off_msg = "";
  }

  /* spell_text[].to_char_msg and spell_text[].to_room_msg should be specified
   * only for spells which apply an affect (e.g. sanctuary), or that have some
   * kind of instantaneous effect that does not cause damage (e.g. cure blind).
   * These messages are also intended only for 'simple' affects where what you
   * see is what you get. Implement more complex messages directly in the spell
   * function(s). Damage spells and most skills (including toggles) should have
   * their messages coded in their functions, or sent via lib/messages as usual.
   */

  /*      Spell #                     name                            to_char_msg                                                            to_room_msg                                                                  wear_off_msg */
  SPLTXTO(SPELL_ANIMATE_DEAD,         "animate dead",                 "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_ARMOR,                "armor",                        "You are protected by magical armor.",                                 "$n is protected by magical armor.",                                         "You feel less protected.");
  SPLTXTO(SPELL_BLESS,                "bless",                        "You feel blessed.",                                                   "$n glows faintly.",                                                         "Your divine favor fades.");
  SPLTXTO(SPELL_BLINDNESS,            "blindness",                    "You have been blinded!",                                              "$n has been blinded!",                                                      "Your vision returns.");
  SPLTXTO(SPELL_BLINDNESS_DUST,       "blindness dust",               "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_BLOOD_LUST,           "blood lust",                   "Your body writhes with a gnawing hunger for blood!",                  "$n's body writhes with a gnawing hunger for blood!",                        "The gnawing hunger for blood slowly fades.");
  SPLTXTO(SPELL_BURNING_HANDS,        "burning hands",                "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_CALL_LIGHTNING,       "call lightning",               "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_CHARM_PERSON,         "charm person",                 "",                                                                    "",                                                                          "You come to your senses.");
  SPLTXTO(SPELL_CHILL_TOUCH,          "chill touch",                  "You are chilled to the bone.",                                        "$n is chilled to the bone.",                                                "You feel warm again.");
  SPLTXTO(SPELL_CLAIRVOYANCE,         "clairvoyance",                 "You close your eyes and concentrate...",                              "$n closes $s eyes and concentrates...",                                     "");
  SPLTXTO(SPELL_CLONE,                "clone",                        "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_COLOR_SPRAY,          "color spray",                  "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_CONFLAGRATION,        "conflagration",                "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_CONTROL_WEATHER,      "control weather",              "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_CONVERGENCE,          "convergence",                  "You feel energized.",                                                 "",                                                                          "");
  SPLTXTO(SPELL_CREATE_FOOD,          "create food",                  "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_CREATE_WATER,         "create water",                 "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_CURE_BLIND,           "cure blind",                   "Your vision returns!",                                                "$n's eyes become unclouded.",                                               "");
  SPLTXTO(SPELL_CURE_CRITIC,          "cure critic",                  "You feel better!",                                                    "",                                                                          "");
  SPLTXTO(SPELL_CURE_CRITIC_SPRAY,    "cure critic spray",            "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_CURE_LIGHT,           "cure light",                   "You feel better!",                                                    "",                                                                          "");
  SPLTXTO(SPELL_CURE_LIGHT_SPRAY,     "cure light spray",             "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_CURE_SERIOUS,         "cure serious",                 "You feel better!",                                                    "",                                                                          "");
  SPLTXTO(SPELL_CURE_SERIOUS_SPRAY,   "cure serious spray",           "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_CURSE,                "curse",                        "You feel very uncomfortable.",                                        "$n looks very uncomfortable.",                                              "Your discomfort subsides.");
  SPLTXTO(SPELL_DEATH_SPRAY,          "death spray",                  "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_DETECT_ALIGNMENT,     "detect alignment",             "Your eyes glow white.",                                               "$n's eyes glow white.",                                                     "The white glow in your eyes fades.");
  SPLTXTO(SPELL_DETECT_INVISIBLE,     "detect invisibility",          "Your eyes glow violet.",                                              "$n's eyes glow violet.",                                                    "The violet glow in your eyes fades.");
  SPLTXTO(SPELL_DETECT_MAGIC,         "detect magic",                 "Your eyes glow blue.",                                                "$n's eyes glow blue.",                                                      "The blue glow in your eyes fades.");
  SPLTXTO(SPELL_DETECT_POISON,        "detect poison",                "Your eyes glow green.",                                               "$n's eyes glow green.",                                                     "The green glow in your eyes fades.");
  SPLTXTO(SPELL_DISINTEGRATE,         "disintegrate",                 "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_DISPEL_EVIL,          "dispel evil",                  "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_DISPEL_GOOD,          "dispel good",                  "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_DISPEL_MAGIC,         "dispel magic",                 "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_DISPEL_SANCT,         "dispel sanctuary",             "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_DIVINE_INTERVENTION,  "divine intervention",          "You are protected by the gods.",                                      "$n is protected by the gods.",                                              "You feel as if the gods have forsaken you!");
  SPLTXTO(SPELL_DIVINE_WIND,          "divine wind",                  "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_EARTHQUAKE,           "earthquake",                   "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_ELECTRIC_SHOCK,       "electric shock",               "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_ENCHANT_ARMOR,        "enchant armor",                "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_ENCHANT_WEAPON,       "enchant weapon",               "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_ENDURE,               "endure",                       "You are shielded by protective magic.",                               "$n is shielded by protective magic.",                                       "You feel less shielded.");
  SPLTXTO(SPELL_ENERGY_DRAIN,         "energy drain",                 "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_EVIL_BLESS,           "evil bless",                   "You feel evil!",                                                      "",                                                                          "");
  SPLTXTO(SPELL_EVIL_WORD,            "evil word",                    "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_FEAR,                 "fear",                         "You are scared!",                                                     "$n is scared!",                                                             "");
  SPLTXTO(SPELL_FIREBALL,             "fireball",                     "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_FIREBREATH,           "firebreath",                   "Your eyes burn red and smoke rises from the ground.",                 "$n's eyes burn red and smoke rises from the ground.",                       "");
  SPLTXTO(SPELL_FLAMESTRIKE,          "flamestrike",                  "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_FLY,                  "fly",                          "You start to fly.",                                                   "$n starts to fly.",                                                         "You landed down on the ground successfully.");
  SPLTXTO(SPELL_FORGET,               "forget",                       "You seem to forget something.",                                       "$n seems to forget something.",                                             "");
  SPLTXTO(SPELL_FURY,                 "fury",                         "You feel very angry.",                                                "$n starts snarling and fuming with fury.",                                  "You calm down.");
  SPLTXTO(SPELL_GREAT_MIRACLE,        "great miracle",                "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_HARM,                 "harm",                         "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_HASTE,                "haste",                        "Suddenly everything around you seems to slow down to a crawl.",       "$n starts moving with blinding speed.",                                     "The speed of your movements slows down to normal.");
  SPLTXTO(SPELL_HEAL,                 "heal",                         "A warm feeling fills your body.",                                     "",                                                                          "");
  SPLTXTO(SPELL_HEAL_SPRAY,           "heal spray",                   "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_HELL_FIRE,            "hell fire",                    "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_HOLD,                 "hold",                         "You are rooted to the ground.",                                       "$n is rooted to the ground.",                                               "You can move again.");
  SPLTXTO(SPELL_HOLY_BLESS,           "holy bless",                   "You feel holy!",                                                      "",                                                                          "");
  SPLTXTO(SPELL_HOLY_WORD,            "holy word",                    "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_HYPNOTIZE,            "hypnotize",                    "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_ICEBALL,              "iceball",                      "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_IDENTIFY,             "identify",                     "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_IMP_INVISIBLE,        "improved invisibility",        "You vanish.",                                                         "$n vanishes into thin air.",                                                "You appear out of thin air.");
  SPLTXTO(SPELL_INFRAVISION,          "infravision",                  "Your eyes glow crimson.",                                             "$n's eyes glow crimson.",                                                   "The crimson glow in your eyes fades.");
  SPLTXTO(SPELL_INVISIBLE,            "invisibility",                 "You slowly fade out of existence.",                                   "$n slowly fades out of existence.",                                         "You slowly fade into existence.");
  SPLTXTO(SPELL_INVUL,                "invulnerability",              "You are surrounded by a powerful sphere.",                            "$n is surrounded by a powerful sphere.",                                    "Your powerful sphere disappears.");
  SPLTXTO(SPELL_IRON_SKIN,            "iron skin",                    "You feel your skin harden.",                                          "$n's skin hardens and turns dark iron in color.",                           "Your skin turns softer.");
  SPLTXTO(SPELL_LAY_HANDS,            "lay hands",                    "A healing power flows into your body.",                               "",                                                                          "");
  SPLTXTO(SPELL_LETHAL_FIRE,          "lethal fire",                  "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_LIGHTNING_BOLT,       "lightning bolt",               "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_LOCATE_CHARACTER,     "locate character",             "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_LOCATE_OBJECT,        "locate object",                "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_MAGIC_MISSILE,        "magic missile",                "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_MANA_TRANSFER,        "mana transfer",                "You feel energy surging into you!",                                   "",                                                                          "");
  SPLTXTO(SPELL_MIRACLE,              "miracle",                      "Your life has been restored.",                                        "",                                                                          "");
  SPLTXTO(SPELL_MYSTIC_SWIFTNESS,     "mystic swiftness",             "Your hands blur with mystical speed!",                                "$n's hands blur with mystical speed!",                                      "Your hands slow down.");
  SPLTXTO(SPELL_PARALYSIS,            "paralyze",                     "You are paralyzed!",                                                  "$n is paralyzed!",                                                          "Your paralysis wears off and you can move again.");
  SPLTXTO(SPELL_PERCEIVE,             "perceive",                     "Your eyes glow with unearthly light.",                                "$n's eyes glow with unearthly light.",                                      "Your depth of perception diminishes.");
  SPLTXTO(SPELL_POISON,               "poison",                       "You feel very sick.",                                                 "$n looks very sick.",                                                       "You feel better.");
  SPLTXTO(SPELL_POISON_SMOKE,         "poison smoke",                 "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_PROTECT_FROM_EVIL,    "protection from evil",         "A cloak of radiant energy surrounds you.",                            "A cloak of radiant energy surrounds $n.",                                   "The radiant aura around your body begins to weaken.");
  SPLTXTO(SPELL_PROTECT_FROM_GOOD,    "protection from good",         "A cloak of chaotic energy surrounds you.",                            "A cloak of chaotic energy surrounds $n.",                                   "The chaotic aura around your body begins to weaken.");
  SPLTXTO(SPELL_PW_KILL,              "power word kill",              "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_QUICK,                "quick",                        "Your mind begins to race.",                                           "$n's mind begins to race.",                                                 "Your thoughts slow.");
  SPLTXTO(SPELL_RAGE,                 "rage",                         "Rage courses through your body!",                                     "Rage courses through $n's body!",                                           "The rage coursing through your body subsides.");
  SPLTXTO(SPELL_REAPPEAR,             "reappear",                     "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_RECHARGE,             "recharge",                     "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_REGENERATION,         "regeneration",                 "Your skin turns green and you feel an affinity for the shining sun.", "$n's skin turns green and $e seems to bend toward the sun.",                "Your regenerative power fades.");
  SPLTXTO(SPELL_REJUVENATION,         "rejuvenation",                 "You feel much better!",                                               "",                                                                          "");
  SPLTXTO(SPELL_RELOCATION,           "relocation",                   "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_REMOVE_CURSE,         "remove curse",                 "You feel a great sense of relief.",                                   "$n looks less uncomfortable.",                                              "");
  SPLTXTO(SPELL_REMOVE_IMP_INVIS,     "remove improved invisibility", "You become visible to the world.",                                    "$n slowly fades into existence.",                                           "");
  SPLTXTO(SPELL_REMOVE_PARALYSIS,     "remove paralysis",             "You can move again!",                                                 "$n's body grows less rigid.",                                               "");
  SPLTXTO(SPELL_REMOVE_POISON,        "remove poison",                "A warm feeling runs through your body.",                              "$n looks less pale and sickly.",                                            "");
  SPLTXTO(SPELL_REVEAL,               "reveal",                       "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_RUSH,                 "rush",                         "Your pulse begins to race!",                                          "$n's veins bulge and twist as $s movement speeds up!",                      "Your pulse slows.");
  SPLTXTO(SPELL_SANCTUARY,            "sanctuary",                    "You start glowing.",                                                  "$n is surrounded by a white aura.",                                         "The white aura around your body fades.");
  SPLTXTO(SPELL_SATIATE,              "satiate",                      "You are full.",                                                       "",                                                                          "");
  SPLTXTO(SPELL_SEARING_ORB,          "searing orb",                  "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_SENSE_LIFE,           "sense life",                   "Your feel your awareness improve.",                                   "$n looks more aware of $s surroundings.",                                   "You feel less aware of your surroundings.");
  SPLTXTO(SPELL_SHOCKING_GRASP,       "shocking grasp",               "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_SLEEP,                "sleep",                        "You feel very sleepy... ZZZzzz...",                                   "$n falls asleep.",                                                          "You feel less tired.");
  SPLTXTO(SPELL_SPHERE,               "sphere",                       "You are surrounded by a golden sphere.",                              "$n is surrounded by a golden sphere.",                                      "Your golden sphere disappears.");
  SPLTXTO(SPELL_SPIRIT_LEVY,          "spirit levy",                  "You absorb life energy from the dead.",                               "$n absorbs life energy from the dead.",                                     "The dark energy within you fades, diminishing your power.");
  SPLTXTO(SPELL_STRENGTH,             "strength",                     "You feel stronger.",                                                  "",                                                                          "You feel weaker.");
  SPLTXTO(SPELL_SUMMON,               "summon",                       "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_SUPER_HARM,           "super harm",                   "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_TELEPORT,             "teleport",                     "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_THUNDERBALL,          "thunderball",                  "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_TOTAL_RECALL,         "total recall",                 "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_VAMPIRIC,             "vampiric touch",               "You feel the drained energy flowing into you.",                       "",                                                                          "");
  SPLTXTO(SPELL_VENTRILOQUATE,        "ventriloquate",                "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_VITALITY,             "vitality",                     "You feel refreshed!",                                                 "",                                                                          "");
  SPLTXTO(SPELL_WIND_SLASH,           "wind slash",                   "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_WORD_OF_RECALL,       "word of recall",               "",                                                                    "",                                                                          "");
  // Enchanter
  SPLTXTO(SPELL_BLADE_BARRIER,        "blade barrier",                "You summon thousands of tiny whirling blades to envelope you!",       "The air hums as thousands of tiny whirling blades appear and envelope $n!", "One by one, the blades in your barrier fall to the ground.");
  SPLTXTO(SPELL_PASSDOOR,             "passdoor",                     "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_ENGAGE,               "engage",                       "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_ETHEREAL_NATURE,      "ethereal nature",              "You slowly fade into an alternate plane of existence.",               "$n slowly fades into an alternate plane of existence.",                     "You feel strong enough to leave the physical plane.");
  SPLTXTO(SPELL_DISRUPT_SANCT,        "disrupt sanctuary",            "Your white aura seems to fade a little.",                             "$n's white aura seems to fade a little.",                                   "The force disrupting your white aura disappears.");
  // Archmage
  SPLTXTO(SPELL_METEOR,               "meteor",                       "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_ORB_PROTECTION,       "orb of protection",            "A golden orb appears over your head.",                                "A golden orb appears over $n's head.",                                      "Your golden orb disappears.");
  SPLTXTO(SPELL_FROSTBOLT,            "frost bolt",                   "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_WRATH_ANCIENTS,       "wrath of ancients",            "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_DISTORTION,           "distortion",                   "Your sphere appears to blur and distort.",                            "$n's sphere appears to blur and distort.",                                  "Your sphere returns to normal.");
  // Druid
  SPLTXTO(SPELL_WALL_THORNS,          "wall of thorns",               "",                                                                    "",                                                                          "");
  // Templar
  SPLTXTO(SPELL_MAGIC_ARMAMENT,       "magic armament",               "You channel magic energy around yourself to augment your attacks.",   "$n channels magic energy around $mself to augment $s attacks.",             "The magic energy surrounding you dissipates.");
  SPLTXTO(SPELL_FORTIFICATION,        "fortification",                "You feel able to withstand any attack.",                              "$n seems able to withstand any attack.",                                    "Your feeling of fortification diminishes.");
  SPLTXTO(SPELL_DIVINE_HAMMER,        "divine hammer",                "You reach to the heavens and call down a hammer of divine power!",    "$n reaches to the heavens and calls down a hammer of divine power!",        "");
  // Ronin
  SPLTXTO(SPELL_BLUR,                 "blur",                         "Your movements become a blur.",                                       "$n's movements become a blur.",                                             "Your movements become less of a blur.");
  // Mystic
  SPLTXTO(SPELL_DEBILITATE,           "debilitate",                   "You are enveloped by a greenish smoke - you feel weaker.",            "$n is enveloped by a greenish smoke.",                                      "You feel a little less weak.");
  SPLTXTO(SPELL_TRANQUILITY,          "tranquility",                  "You suddenly feel awash in a sense of tranquility.",                  "$n is suddenly awash in a sense of tranquility.",                           "Your sense of tranquility fades away.");
  // Cavalier
  SPLTXTO(SPELL_MIGHT,                "might",                        "You feel more powerful.",                                             "$n looks more powerful.",                                                   "You feel less powerful.");
  SPLTXTO(SPELL_WRATH_OF_GOD,         "wrath of god",                 "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_POWER_OF_DEVOTION,    "power of devotion",            "You are enveloped in a bright white aura.",                           "$n is enveloped in a bright white aura.",                                   "The bright white aura around your body fades.");
  // Crusader
  SPLTXTO(SPELL_RIGHTEOUSNESS,        "righteousness",                "You feel righteous!",                                                 "$n exerts a sense of righteousness.",                                       "Your sense of righteousness disappears.");
  SPLTXTO(SPELL_POWER_OF_FAITH,       "power of faith",               "", /* message in cast_power_of_faith() */                             "",                                                                          "");
  // Defiler
  SPLTXTO(SPELL_DESECRATE,            "desecrate",                    "",                                                                    "",                                                                          "You feel the insatiable urge to desecrate another corpse.");
  SPLTXTO(SPELL_BLACKMANTLE,          "blackmantle",                  "You are surrounded by an eerie mantle of darkness.",                  "$n is surrounded by an eerie mantle of darkness.",                          "Your mantle of darkness fades away.");
  // Infidel
  SPLTXTO(SPELL_SHADOW_WRAITH,        "shadow wraith",                "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_DUSK_REQUIEM,         "dusk requiem",                 "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_WITHER,               "wither",                       "",                                                                    "",                                                                          "The pain coursing through your withered body recedes.");
  // Bladesinger
  SPLTXTO(SPELL_RALLY,                "rally",                        "You feel rallied!",                                                   "",                                                                          "Your sense of unity disappears.");
  // Chanter
  SPLTXTO(SPELL_WAR_CHANT,            "war chant",                    "You feel safer!",                                                     "",                                                                          "You don't feel as safe.");
  SPLTXTO(SPELL_WAR_CHANT_DEBUFF,     "!war chant!",                  "You grow weak with panic!",                                           "$n grows weak with panic!",                                                 "You stop panicking.");
  SPLTXTO(SPELL_LUCK,                 "luck",                         "You feel lucky!",                                                     "",                                                                          "You don't feel as lucky.");
  SPLTXTO(SPELL_AID,                  "aid",                          "", /* message in song_parser.c */                                     "",                                                                          "");
  SPLTXTO(SPELL_CAMARADERIE,          "camaraderie",                  "You feel safety in numbers.",                                         "",                                                                          "Your kinship fades.");
  // Legionnaire
  SPLTXTO(SPELL_RIMEFANG,             "rimefang",                     "",                                                                    "",                                                                          "You begin to thaw.");
  SPLTXTO(SPELL_DEVASTATION,          "devastation",                  "",                                                                    "",                                                                          "");
  // Mercenary
  SPLTXTO(SPELL_TREMOR,               "tremor",                       "",                                                                    "",                                                                          "Your tremors subside.");
  SPLTXTO(SPELL_CLOUD_CONFUSION,      "cloud of confusion",           "You feel disoriented.",                                               "$n looks disoriented.",                                                     "You feel less disoriented.");
  SPLTXTO(SPELL_INCENDIARY_CLOUD,     "incendiary cloud",             "",                                                                    "",                                                                          "The cloud of fire surrounding you dies out.");

  /*      Skill #                     name                            to_char_msg                                                            to_room_msg                                                                  wear_off_msg */
  SPLTXTO(SKILL_AMBUSH,               "ambush",                       "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_ASSASSINATE,          "assassinate",                  "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_ASSAULT,              "assault",                      "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_BACKFIST,             "backfist",                     "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_BACKSTAB,             "backstab",                     "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_BASH,                 "bash",                         "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_BLOCK,                "block",                        "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_BUTCHER,              "butcher",                      "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_CAMP,                 "camp",                         "You quickly set up a camp here, then sit down and rest.",             "$n quickly sets up a camp here, then sits down and rests.",                 "You break camp.");
  SPLTXTO(SKILL_CIRCLE,               "circle",                       "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_COIN_TOSS,            "coin-toss",                    "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_CUNNING,              "cunning",                      "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_DISARM,               "disarm",                       "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_DISEMBOWEL,           "disembowel",                   "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_DODGE,                "dodge",                        "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_DUAL,                 "dual",                         "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_HIDDEN_BLADE,         "hidden-blade",                 "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_HIDE,                 "hide",                         "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_KICK,                 "kick",                         "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_KNOCK,                "knock",                        "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_MEDITATE,             "meditate",                     "You gaze inward and focus on healing.",                               "$n enters a deep trance.",                                                  "You feel less focused.");
  SPLTXTO(SKILL_PARRY,                "parry",                        "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_PEEK,                 "peek",                         "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_PICK_LOCK,            "pick",                         "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_PRAY,                 "pray",                         "You bow your head and begin your prayer.",                            "$n bows $s head and begins praying.",                                       "You finish your prayers.");
  SPLTXTO(SKILL_PUMMEL,               "pummel",                       "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_PUNCH,                "punch",                        "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_QUAD,                 "quad",                         "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_RESCUE,               "rescue",                       "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_SCAN,                 "scan",                         "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_SNEAK,                "sneak",                        "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_SPIN_KICK,            "spin",                         "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_STEAL,                "steal",                        "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_SUBDUE,               "subdue",                       "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_TAUNT,                "taunt",                        "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_THROW,                "throw",                        "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_TRAP,                 "trap",                         "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_TRIPLE,               "triple",                       "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_TWIST,                "twist",                        "",                                                                    "",                                                                          "");
  // Druid
  SPLTXTO(SKILL_DEGENERATE,           "degenerate",                   "You eat away at your life force for a few precious mana points.",     "$n eats away at $s life force for a few precious mana points.",             "");
  SPLTXTO(SKILL_SHAPESHIFT,           "shapeshift",                   "",                                                                    "",                                                                          "");
  // Rogue
  SPLTXTO(SKILL_DIRTY_TRICKS,         "dirty-tricks",                 "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_TROPHY,               "trophy",                       "",                                                                    "",                                                                          "You feel less inspired.");
  SPLTXTO(SKILL_VEHEMENCE,            "vehemence",                    "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_TRIP,                 "trip",                         "",                                                                    "",                                                                          "");
  // Bandit
  SPLTXTO(SKILL_EVASION,              "evasion",                      "",                                                                    "",                                                                          "");
  // Warlord
  SPLTXTO(SKILL_AWARENESS,            "awareness",                    "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_PROTECT,              "protect",                      "",                                                                    "",                                                                          "");
  // Gladiator
  SPLTXTO(SKILL_FLANK,                "flank",                        "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_MAIM,                 "maim",                         "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_HEADBUTT,             "headbutt",                     "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_HOSTILE,              "hostile",                      "",                                                                    "",                                                                          "");
  // Ronin
  SPLTXTO(SKILL_BANZAI,               "banzai",                       "",                                                                    "",                                                                          "");
  // Mystic
  SPLTXTO(SKILL_TIGERKICK,            "tigerkick",                    "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_MANTRA,               "mantra",                       "",                                                                    "",                                                                          "");
  // Ranger
  SPLTXTO(SKILL_BERSERK,              "berserk",                      "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_DEFEND,               "defend",                       "",                                                                    "",                                                                          "");
  // Trapper
  SPLTXTO(SKILL_BATTER,               "batter",                       "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_FRENZY,               "frenzy",                       "",                                                                    "",                                                                          "");
  // Cavalier
  SPLTXTO(SKILL_TRUSTY_STEED,         "trusty-steed",                 "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_SMITE,                "smite",                        "",                                                                    "",                                                                          "");
  // Crusader
  SPLTXTO(SKILL_ZEAL,                 "zeal",                         "",                                                                    "",                                                                          "");
  // Defiler
  SPLTXTO(SKILL_FEINT,                "feint",                        "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_SHADOWSTEP,           "shadowstep",                   "",                                                                    "",                                                                          "");
  // Infidel
  SPLTXTO(SKILL_VICTIMIZE,            "victimize",                    "",                                                                    "",                                                                          "");
  // Legionnaire
  SPLTXTO(SKILL_LUNGE,                "lunge",                        "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_CLOBBER,              "clobber",                      "",                                                                    "",                                                                          "");
  SPLTXTO(SKILL_SNIPE,                "snipe",                        "",                                                                    "",                                                                          "");
  // Mercenary
  SPLTXTO(SKILL_RIPOSTE,              "riposte",                      "",                                                                    "",                                                                          "");

  /*      Other #                     name                            to_char_msg                                                            to_room_msg                                                                  wear_off_msg */
  SPLTXTO(SPELL_DISENCHANT,           "disenchant",                   "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_GREAT_MANA,           "great mana",                   "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_MANA_HEAL,            "mana heal",                    "You feel slightly regenerated.",                                      "",                                                                          "");
  SPLTXTO(SPELL_PETRIFY,              "petrify",                      "",                                                                    "",                                                                          "");
  SPLTXTO(SPELL_RECOVER_MANA,         "recover mana",                 "Your mana has been restored.",                                        "",                                                                          "");
  SPLTXTO(SMELL_FARTMOUTH,            "fartmouth",                    "You have a poopy mouth.",                                             "$n has a poopy mouth.",                                                     "You finally get your hands on some mouthwash!");

  /* Copy spell_text[spell_nr].name to spells[spell_nr - 1] for legacy code purposes. */
  for (int spell_nr = 1; (spell_nr - 1 < NUMELEMS(spells)) && (spell_nr < NUMELEMS(spell_text)); spell_nr++) {
    spells[spell_nr - 1] = spell_text[spell_nr].name;
  }

  /* This is required; don't remove it. */
  spells[NUMELEMS(spells) - 1] = "\n";
}
