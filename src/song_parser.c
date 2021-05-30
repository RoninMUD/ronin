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

struct song_data_t {
  int song_id;
  char* name;
  char* lyrics;
  int level;
  int mana;
  int subclass;
  int subclass_level;
};

const struct song_data_t song_data[] = {
//  song_id             name                                 lyrics                                                     level  mana  subclass        subclass_level
  { SONG_DETECT_POISON, "toxic",                             "With a taste of poison paradise...",                      3,     5,    0,              0 },
  { SONG_ARMOR,         "seal with a kiss",                  "In a letter, sealed with a kiss...",                      5,     5,    0,              0 },
  { SONG_ARMOR,         "love is a battlefield",             "Why do you hurt me so bad...",                            5,     5,    0,              0 },
  { SONG_CURE_BLIND,    "i can see clearly now",             "Gone are the dark clouds that had me blind...",           7,     15,   0,              0 },
  { SONG_FLY,           "one day i will fly away",           "One day I will fly away...",                              9,     25,   0,              0 },
  { SONG_FLY,           "come fly with me",                  "Come fly with me, let's fly, let's fly away...",          9,     25,   0,              0 },
  { SONG_CURE_CRITIC,   "we don't need another hero",        "We don't need another hero...",                           11,    25,   0,              0 },
  { SONG_FORGET,        "forget you",                        "Said, if I was richer, I'd still be with ya...",          12,    15,   0,              0 },
  { SONG_BLINDNESS,     "smoke gets in your eyes",           "Smoke gets in your eyes...",                              13,    15,   0,              0 },
  { SONG_BLINDNESS,     "blinded by the light",              "Revved up like a deuce, another runner in the night...",  13,    15,   0,              0 },
  { SONG_SLEEP,         "enter sandman",                     "I tuck you in, warm within, keep you free from sin...",   14,    25,   0,              0 },
  { SONG_SATIATE,       "best of me",                        "Now I save the best of me...",                            15,    10,   0,              0 },
  { SONG_SATIATE,       "hungry like the wolf",              "Mouth is alive with juices like wine...",                 15,    10,   0,              0 },
  { SONG_VITALITY,      "i'm taking a walk",                 "I'm taking a ride with my best friend...",                16,    25,   0,              0 },
  { SONG_COLOR_SPRAY,   "dangerous",                         "Dangerous...",                                            17,    25,   0,              0 },
  { SONG_COLOR_SPRAY,   "purple rain",                       "I never meant to cause you any pain...",                  17,    25,   0,              0 },
  { SONG_DETECT_INVIS,  "somebody's watching me",            "I always feel like somebody's watching me...",            18,    25,   0,              0 },
  { SONG_TOTAL_RECALL,  "we are the world",                  "We are the world, we are the children...",                19,    30,   0,              0 },
  { SONG_TOTAL_RECALL,  "mama, i'm coming home",             "Lost and found and turned around...",                     19,    30,   0,              0 },
  { SONG_FEAR,          "don't fear the reaper",             "Come on baby, don't fear the reaper...",                  20,    15,   0,              0 },
  { SONG_INFRAVISION,   "moonlight shadow",                  "Carried away by moonlight shadow...",                     21,    20,   0,              0 },
  { SONG_INFRAVISION,   "i wear my sunglasses at night",     "Don't masquerade with the guy in shades, oh no...",       21,    20,   0,              0 },
  { SONG_CHARM_PERSON,  "this charming man",                 "Will nature make a man of me yet...",                     22,    50,   0,              0 },
  { SONG_SANCTUARY,     "nothing compares to you",           "It's been seven hours and fifteen days...",               23,    50,   0,              0 },
  { SONG_SANCTUARY,     "safety dance",                      "And we can act like we come from out of this world...",   23,    50,   0,              0 },
  { SONG_ANIMATE_DEAD,  "toy soldier",                       "Like toy soldiers...",                                    25,    50,   0,              0 },
  { SONG_ANIMATE_DEAD,  "thriller",                          "Something evil's lurking in the dark...",                 25,    50,   0,              0 },
  { SONG_HEAL,          "everything i do i do it for you",   "Everything I do, I do it for you...",                     26,    50,   0,              0 },
  { SONG_HEAL,          "comfortably numb",                  "There is no pain you are receding...",                    26,    50,   0,              0 },
  { SONG_HOLD,          "hold on",                           "Why do you lock yourself up in these chains...",          27,    15,   0,              0 },
  { SONG_LETHAL_FIRE,   "a view to a kill",                  "Dance into the fire...",                                  28,    50,   0,              0 },
  { SONG_LETHAL_FIRE,   "ring of fire",                      "I went down down down the flames went higher...",         28,    50,   0,              0 },
  { SONG_REMOVE_POISON, "lifting shadows of a dream",        "Lifting shadows, off a dream...",                         36,    25,   0,              0 },
  { SONG_REMOVE_POISON, "every rose has its thorn",          "Was it something I said or something I did...",           36,    25,   0,              0 },
  { SONG_HASTE,         "canned heat",                       "You know this boogie is for real...",                     42,    120,  0,              0 },
  { SONG_REJUVENATION,  "there is no heart that won't heal", "There is no heart that won't heal...",                    46,    80,   0,              0 },
  { SONG_REJUVENATION,  "forever young",                     "Let us die young or let us live forever...'",             46,    80,   0,              0 },
  { SONG_GROUP_SUMMON,  "come together",                     "He got joo joo eyeballs, he one holy roller...",          50,    100,  0,              0 },
  { SONG_RESPITE,       "bridge over troubled water",        "When you're weary, feeling small...",                     30,    40,   SC_CHANTER,     1 },
  { SONG_WARCHANT,      "something takes a part of me",      "Something takes a part of me...",                         30,    60,   SC_CHANTER,     2 },
  { SONG_WARCHANT,      "war",                               "Oh, war, has shattered many a young mans dreams...",      30,    60,   SC_CHANTER,     2 },
  { SONG_LUCK,          "if i could change the world",       "If I could change the world...",                          40,    100,  SC_CHANTER,     3 },
  { SONG_LUCK,          "lucky",                             "She's so lucky, she's a star...",                         40,    100,  SC_CHANTER,     3 },
  { SONG_AID,           "hero of the day",                   "Hero of the day...",                                      40,    150,  SC_CHANTER,     4 },
  { SONG_AID,           "power of love",                     "Tougher than diamonds, rich like cream...",               40,    150,  SC_CHANTER,     4 },
  { SONG_CAMARADERIE,   "we are family",                     "Everyone can see we're together...",                      45,    150,  SC_CHANTER,     5 },
  { SONG_RALLY,         "brightest flame burns quickest",    "The brightest flame burns quickest...",                   30,    50,   SC_BLADESINGER, 1 },
  { SONG_RALLY,         "bulls on parade",                   "Rally 'round the family with a pocket full of shells...", 30,    50,   SC_BLADESINGER, 1 },
  { SONG_BLADE_DANCE,   "ballroom blitz",                    "And the man in the back said everyone attack...",         40,    80,   SC_BLADESINGER, 4 }
};

int bladesinging_enchantment(ENCH *ench, CHAR *enchanted_ch, CHAR *char_in_room, int cmd, char *arg) {
  if (cmd == MSG_REMOVE_ENCH) {
    send_to_char("The music enchanting your blade decrescendos to silence.\n\r", enchanted_ch);

    return FALSE;
  }

  return FALSE;
}

struct song_data_t get_song_info(CHAR* ch, char* song_name) {
  struct song_data_t song_info = { 0 };

  if (ch && song_name && *song_name) {
    for (int i = 0; i < NUMELEMS(song_data); i++) {
      if (is_abbrev(song_name, song_data[i].name)) {
        return song_data[i];
      }
    }
  }

  return song_info;
}

bool check_song_access(CHAR* ch, struct song_data_t song_info) {
  if (!ch || (song_info.song_id <= 0)) return FALSE;

  if (IS_IMMORTAL(ch)) {
    return TRUE;
  }

  if (GET_LEVEL(ch) < song_info.level) {
    return FALSE;
  }

  if (song_info.subclass > 0) {
    if (GET_SC(ch) != song_info.subclass) {
      return FALSE;
    }

    if (GET_SC_LEVEL(ch) < song_info.subclass_level) {
      return FALSE;
    }
  }

  return TRUE;
}

void do_song(CHAR* ch, char* arg, int cmd)
{
  char buf[MIL];

  if (!ch || IS_NPC(ch)) return;

  if ((GET_CLASS(ch) != CLASS_BARD) && IS_MORTAL(ch))
  {
    send_to_char("Please leave singing songs to the bards.\n\r", ch);
    return;
  }

  if (IS_MORTAL(ch) && (!EQ(ch, HOLD) || OBJ_TYPE(EQ(ch, HOLD)) != ITEM_MUSICAL))
  {
    send_to_char("You need to hold a musical instrument to sing a song.\n\r", ch);
    return;
  }

  arg = skip_spaces(arg);

  if (!(*arg))
  {
    send_to_char("What song?\n\r", ch);
    return;
  }

  char song_name[MIL];

  if (!str_sub_delim(song_name, sizeof(song_name), arg, '\'', '\''))
  {
    send_to_char("Songs must always be enclosed by the symbols: '\n\r", ch);
    return;
  }

  struct song_data_t song_info = get_song_info(ch, song_name);

  if ((song_info.song_id <= 0) || !check_song_access(ch, song_info))
  {
    send_to_char("You do not know of such a song.\n\r", ch);
    return;
  }

  int mana = song_info.mana;

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, DOUBLE_MANA))
  {
    mana *= 2;
  }

  if (IS_MORTAL(ch) && (GET_MANA(ch) < mana))
  {
    send_to_char("You can't summon enough energy to sing the song.\n\r", ch);
    return;
  }
  else if (IS_IMMORTAL(ch))
  {
    sprintf(buf, "WIZINFO: %s sings %s.", GET_NAME(ch), arg);
    wizlog(buf, GET_LEVEL(ch) + 1, 5);
    log_f("%s", buf);
  }

  GET_MANA(ch) -= mana;

  send_to_char("You sing the song.\n\r", ch);

  if (IS_MORTAL(ch) && IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_SONG))
  {
    send_to_char("The song has been absorbed by your surroundings.\n\r", ch);
    return;
  }

  WAIT_STATE(ch, PULSE_VIOLENCE);

  CHAR* tmp_victim = NULL;
  CHAR* next_victim = NULL;
  OBJ* tmp_obj = NULL;
  OBJ* next_obj = NULL;

  switch (song_info.song_id) {
  case SONG_DETECT_POISON:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim) {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && !IS_NPC(tmp_victim)) {
        spell_detect_poison(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_detect_poison(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_ARMOR:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim)
      {
        spell_armor(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_armor(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_CURE_BLIND:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && (!IS_NPC(tmp_victim) || SAME_GROUP(ch, tmp_victim)))
      {
        spell_cure_blind(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_cure_blind(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_FLY:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim)
      {
        spell_fly(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_fly(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_CURE_CRITIC:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && (!IS_NPC(tmp_victim) || SAME_GROUP(ch, tmp_victim)))
      {
        spell_cure_critic(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_cure_critic(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_FORGET:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && IS_NPC(tmp_victim))
      {
        spell_forget(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }
    break;

  case SONG_BLINDNESS:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && !SAME_GROUP(ch, tmp_victim))
      {
        spell_blindness(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }
    break;


  case SONG_SLEEP:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && IS_NPC(tmp_victim))
      {
        spell_sleep(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }
    break;

  case SONG_SATIATE:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && !IS_NPC(tmp_victim))
      {
        spell_satiate(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_satiate(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_VITALITY:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && !IS_NPC(tmp_victim))
      {
        spell_vitality(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_vitality(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_COLOR_SPRAY:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && !SAME_GROUP(ch, tmp_victim))
      {
        spell_color_spray(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }
    break;

  case SONG_DETECT_INVIS:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim) {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && !IS_NPC(tmp_victim)) {
        spell_detect_invisibility(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_detect_invisibility(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_TOTAL_RECALL:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim)
      {
        spell_word_of_recall(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_word_of_recall(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_FEAR:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && !SAME_GROUP(ch, tmp_victim))
      {
        spell_fear(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }
    break;

  case SONG_INFRAVISION:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && !IS_NPC(tmp_victim))
      {
        spell_infravision(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_infravision(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_CHARM_PERSON:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && IS_NPC(tmp_victim))
      {
        spell_charm_person(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }
    break;

  case SONG_SANCTUARY:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim)
      {
        spell_sanctuary(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_sanctuary(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_ANIMATE_DEAD:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_obj = world[CHAR_REAL_ROOM(ch)].contents; tmp_obj; tmp_obj = next_obj)
    {
      next_obj = tmp_obj->next_content;
      spell_animate_dead(GET_LEVEL(ch), ch, NULL, tmp_obj);
    }
    break;

  case SONG_HEAL:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && (!IS_NPC(tmp_victim) || SAME_GROUP(ch, tmp_victim)))
      {
        spell_heal(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_heal(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_HOLD:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && IS_NPC(tmp_victim))
      {
        spell_hold(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    break;

  case SONG_LETHAL_FIRE:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && !SAME_GROUP(ch, tmp_victim))
      {
        spell_lethal_fire(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }
    break;

  case SONG_REMOVE_POISON:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && (!IS_NPC(tmp_victim) || SAME_GROUP(ch, tmp_victim)))
      {
        spell_remove_poison(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_remove_poison(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_HASTE:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim)
      {
        if ((GET_CLASS(tmp_victim) == CLASS_BARD) || (GET_CLASS(tmp_victim) == CLASS_CLERIC) || chance(5))
        {
          spell_haste(GET_LEVEL(ch), ch, tmp_victim, NULL);
        }
      }
    }
    spell_haste(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_REJUVENATION:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && (!IS_NPC(tmp_victim) || SAME_GROUP(ch, tmp_victim)))
      {
        spell_rejuvenation(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_rejuvenation(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_GROUP_SUMMON:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    CHAR* group_leader = (GET_MASTER(ch) ? GET_MASTER(ch) : ch);

    if (ch != group_leader) spell_summon(GET_LEVEL(ch), ch, group_leader, NULL);

    for (FOL* follower = group_leader->followers; follower; follower = follower->next) {
      CHAR* temp_ch = follower->follower;

      if (temp_ch &&
          SAME_GROUP(group_leader, temp_ch) &&
          SAME_GROUP(ch, temp_ch) &&
          ch != temp_ch)
      {
        spell_summon(GET_LEVEL(ch), ch, temp_ch, NULL);
      }
    }
    break;

  case SONG_RESPITE:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    const int respite_dispel_types[] = {
      SPELL_CHARM_PERSON,
      SPELL_INCENDIARY_CLOUD,
      SPELL_CLOUD_CONFUSION,
      SPELL_CURSE,
      SPELL_CHILL_TOUCH,
      SPELL_WITHER,
      SPELL_RIMEFANG,
      SPELL_BLINDNESS,
      SPELL_POISON,
      SPELL_PARALYSIS,
      SPELL_SLEEP
    };
    int respite_dispel = 0;

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (!IS_NPC(tmp_victim))
      {
        respite_dispel = get_random_set_affect(tmp_victim, respite_dispel_types, NUMELEMS(respite_dispel_types));
        switch (respite_dispel) {
        case SPELL_CHARM_PERSON:
          if (affected_by_spell(tmp_victim, SPELL_CHARM_PERSON))
          {
            send_to_char("You feel more self-confident.\n\r", tmp_victim);
            affect_from_char(tmp_victim, SPELL_CHARM_PERSON);
          }
          break;
        case SPELL_INCENDIARY_CLOUD:
          if (affected_by_spell(tmp_victim, SPELL_INCENDIARY_CLOUD))
          {
            send_to_char("The blistering waves of heat subside.\n\r", tmp_victim);
            affect_from_char(tmp_victim, SPELL_INCENDIARY_CLOUD);
          }
          break;
        case SPELL_CLOUD_CONFUSION:
          if (affected_by_spell(tmp_victim, SPELL_CLOUD_CONFUSION))
          {
            send_to_char("You feel less disoriented.\n\r", tmp_victim);
            affect_from_char(tmp_victim, SPELL_CLOUD_CONFUSION);
          }
          break;
        case SPELL_CURSE:
          if (affected_by_spell(tmp_victim, SPELL_CURSE))
          {
            send_to_char("You feel better.\n\r", tmp_victim);
            affect_from_char(tmp_victim, SPELL_CURSE);
          }
          break;
        case SPELL_CHILL_TOUCH:
          if (affected_by_spell(tmp_victim, SPELL_CHILL_TOUCH))
          {
            send_to_char("You feel warm again.\n\r", tmp_victim);
            affect_from_char(tmp_victim, SPELL_CHILL_TOUCH);
          }
          break;
        case SPELL_WITHER:
          if (affected_by_spell(tmp_victim, SPELL_WITHER))
          {
            send_to_char("The pain coursing through your withered body begins to recede.\n\r", tmp_victim);
            affect_from_char(tmp_victim, SPELL_WITHER);
          }
          break;
        case SPELL_RIMEFANG:
          if (affected_by_spell(tmp_victim, SPELL_RIMEFANG))
          {
            send_to_char("The paralyzation wears off, you can move again.\n\r", tmp_victim);
            affect_from_char(tmp_victim, SPELL_RIMEFANG);
          }
          break;
        case SPELL_BLINDNESS:
          if (affected_by_spell(tmp_victim, SPELL_BLINDNESS))
          {
            send_to_char("You can see again.\n\r", tmp_victim);
            affect_from_char(tmp_victim, SPELL_BLINDNESS);
          }
          break;
        case SPELL_POISON:
          if (affected_by_spell(tmp_victim, SPELL_POISON))
          {
            send_to_char("You feel better.\n\r", tmp_victim);
            affect_from_char(tmp_victim, SPELL_POISON);
          }
          break;
        case SPELL_PARALYSIS:
          if (affected_by_spell(tmp_victim, SPELL_PARALYSIS))
          {
            send_to_char("You can move again.\n\r", tmp_victim);
            affect_from_char(tmp_victim, SPELL_PARALYSIS);
          }
          break;
        case SPELL_SLEEP:
          if (affected_by_spell(tmp_victim, SPELL_SLEEP))
          {
            send_to_char("You feel less tired.\n\r", tmp_victim);
            affect_from_char(tmp_victim, SPELL_SLEEP);
          }
          break;
        case TYPE_UNDEFINED:
          if ((ch != tmp_victim) && GET_DESCRIPTOR(tmp_victim)) {
              send_to_char("You feel ready!\n\r", tmp_victim);
              GET_WAIT(tmp_victim) = 0;
          }
          break;
        }
      }
    }
    break;

  case SONG_WARCHANT:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim)
      {
        spell_warchant(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_warchant(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_LUCK:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim)
      {
        spell_luck(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_luck(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_AID:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim)
      {
        spell_aid(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_aid(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_CAMARADERIE:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim)
      {
        spell_camaraderie(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_camaraderie(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_RALLY:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim)
      {
        spell_rally(GET_LEVEL(ch), ch, tmp_victim, NULL);
      }
    }

    spell_rally(GET_LEVEL(ch), ch, ch, NULL);
    break;

  case SONG_BLADE_DANCE:
    snprintf(buf, sizeof(buf), "$n sings '%s'", song_info.lyrics);
    act(buf, FALSE, ch, NULL, NULL, TO_ROOM);

    send_to_char("You leap and twirl and dodge and dive around in a flurry of sound and fury.\n\r", ch);
    act("$n leaps and twirls and dodges and dives around in a flurry of sound and fury.", FALSE, ch, NULL, NULL, TO_ROOM);

    for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
    {
      next_victim = tmp_victim->next_in_room;

      if (ch != tmp_victim && (IS_NPC(tmp_victim) || ROOM_CHAOTIC(CHAR_REAL_ROOM(tmp_victim))) && !SAME_GROUP(ch, tmp_victim))
      {
        send_to_char("The torrent of music overwhelms your senses and pierces your soul.\n\r", tmp_victim);

        damage(ch, tmp_victim, 350, TYPE_UNDEFINED, DAM_SOUND);

        if (SAME_ROOM(ch, tmp_victim)) {
          hit(ch, tmp_victim, TYPE_UNDEFINED);
        }
      }
    }
    break;

  default:
    act("$n hums the words of a song $e doesn't know.", FALSE, ch, NULL, NULL, TO_ROOM);
    act("You hum the words of a song you don't know.", FALSE, ch, NULL, NULL, TO_CHAR);
    break;
  }

  /* Bladesinging */
  if (IS_MORTAL(ch) && check_subclass(ch, SC_BLADESINGER, 3))
  {
    if (song_info.song_id && (mana > 0))
    {
      enchantment_apply(ch, TRUE, "Bladesinging", TYPE_UNDEFINED, 10, ENCH_INTERVAL_ROUND, MAX(1, (mana / 2)), APPLY_DAMROLL, 0, 0, bladesinging_enchantment);

      send_to_char("Your powerful lyrics lend strength to your blade!\n\r", ch);
    }
  }
}
