/******************************************************************************
 * Special Character Functions                     (struct char_data -> func) *
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "structs.h"
#include "utils.h"

#include "act.h"
#include "aff_ench.h"
#include "db.h"
#include "cmd.h"
#include "comm.h"
#include "constants.h"
#include "enchant.h"
#include "fight.h"
#include "handler.h"
#include "subclass.h"
#include "utility.h"
#include "weather.h"

#include "char_spec.h"

 /* Bandit SC 3: Adrenaline Rush */
void adrenaline_rush_spec(CHAR *ch, CHAR *signaler, int cmd, const char *arg) {
  if (!ch) return;

  if (IS_MORTAL(ch) && check_subclass(ch, SC_BANDIT, 3) && GET_OPPONENT(ch)) {
    /* If less than max hit points, regenerate hit points. */
    if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
      GET_HIT(ch) = MIN((GET_HIT(ch) + (GET_LEVEL(ch) / 5)), GET_MAX_HIT(ch));
    }
    /* If at max hit points and less than max mana, regenerate mana. */
    else if (GET_MANA(ch) < GET_MAX_MANA(ch)) {
      GET_MANA(ch) = MIN((GET_MANA(ch) + (GET_LEVEL(ch) / 15)), GET_MAX_HIT(ch));
    }
  }
}

int dirty_tricks_enchantment(ENCH *ench, CHAR *ch, CHAR *signaler, int cmd, char *arg) {
  if (!ench || !ch) return FALSE;

  if (cmd == MSG_MOBACT) {
    act("Blood oozes from your gaping wound.", FALSE, ch, 0, 0, TO_CHAR);
    act("Blood oozes from $n's gaping wound.", TRUE, ch, 0, 0, TO_ROOM);

    int dmg = dice(3, 12);

    if (dmg >= GET_HIT(ch)) {
      dmg = GET_HIT(ch) - 1;
    }

    int set_pos = GET_POS(ch);

    damage(ch, ch, dmg, SKILL_DIRTY_TRICKS, DAM_PHYSICAL);

    GET_POS(ch) = MIN(GET_POS(ch), set_pos);

    return FALSE;
  }

  return FALSE;
}

void dirty_tricks_spec(CHAR *ch, CHAR *victim, int cmd, const char *arg) {
  if (!ch || !victim || !SAME_ROOM(ch, victim)) return;

  const int DT_STAB = 1, DT_BLIND = 2, DT_KICK = 3;

  /* Check access to Dirty Tricks and that the skill is toggled. */
  if (IS_MORTAL(ch) && IS_SET(GET_TOGGLES(ch), TOG_DIRTY_TRICKS) && check_sc_access(ch, SKILL_DIRTY_TRICKS)) {
    /* Calculate the skill check. */
    int check = number(1, 850) - (GET_DEX_APP(ch) * 5);

    /* Check skill success/failure. */
    if (check <= GET_LEARNED(ch, SKILL_DIRTY_TRICKS)) {
      /* Create a list to store possible tricks. */
      int trick_list[3], trick_idx = 0;

      /* Can we stab? */
      if (EQ(ch, WIELD) && !enchanted_by(victim, "Gaping Wound (Dirty Tricks)")) {
        trick_list[trick_idx] = DT_STAB;
      }
      else {
        trick_list[trick_idx] = -1;
      }

      trick_idx++;

      /* Can we blind? */
      if (!IS_IMMUNE(victim, IMMUNE_BLINDNESS) && !IS_AFFECTED(victim, AFF_BLIND)) {
        trick_list[trick_idx] = DT_BLIND;
      }
      else {
        trick_list[trick_idx] = -1;
      }

      trick_idx++;

      /* Kick is always possible. */
      trick_list[trick_idx] = DT_KICK;

      /* Quick sort the trick list descending, so that any FALSE values are at the end of the list. */
      qsort(trick_list, NUMELEMS(trick_list), sizeof(int), qcmp_int_desc);

      /* Count the number of tricks from the trick list that are not -1 (unavailable). */
      int num_tricks = 0;

      while ((num_tricks < NUMELEMS(trick_list)) && (trick_list[num_tricks] != -1)) {
        num_tricks++;
      }

      /* Check if there's something to choose. */
      if (num_tricks) {
        /* Shuffle the num_trick elements of the trick list. */
        shuffle_int_array(trick_list, num_tricks);

        /* Choose the trick to use (first index of trick_list). */
        int trick = trick_list[0];

        if (trick == DT_STAB) {
          act("You stab your weapon deeply into $N, opening a gruesome gaping wound.", FALSE, ch, 0, victim, TO_CHAR);
          act("$n stabs $s weapon deeply into you, opening a gruesome gaping wound.", FALSE, ch, 0, victim, TO_VICT);
          act("$n stabs $s weapon deeply into $N, opening a gruesome gaping wound.", FALSE, ch, 0, victim, TO_NOTVICT);

          enchantment_apply(victim, FALSE, "Gaping Wounds (Dirty Tricks)", SKILL_DIRTY_TRICKS, 20, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, dirty_tricks_enchantment);
        }
        else if (trick == DT_BLIND) {
          act("You throw some blinding dust into $N's eyes.", FALSE, ch, 0, victim, TO_CHAR);
          act("$n throws some blinding dust into your eyes.", FALSE, ch, 0, victim, TO_VICT);
          act("$n throws blinding dust into $N's eyes.", FALSE, ch, 0, victim, TO_NOTVICT);

          act("$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM);
          send_to_char("You have been blinded!\n\r", victim);

          affect_apply(victim, SPELL_BLINDNESS, 0, 40, APPLY_AC, AFF_BLIND, 0);
          affect_apply(victim, SPELL_BLINDNESS, 0, -4, APPLY_HITROLL, AFF_BLIND, 0);
        }
        else if (trick == DT_KICK) {
          if (AWAKE(victim) && IS_AFFECTED(victim, AFF_INVUL) && !breakthrough(ch, victim, SKILL_DIRTY_TRICKS, BT_INVUL)) {
            act("You kick $N savagely, but $E seems unfazed.", FALSE, ch, 0, victim, TO_CHAR);
            act("$n kicks you savagely,  but you feel unfazed.", FALSE, ch, 0, victim, TO_VICT);
            act("$n kicks $N savagely,  but $E seems unfazed.", FALSE, ch, 0, victim, TO_NOTVICT);

            damage(ch, victim, 0, SKILL_DIRTY_TRICKS, DAM_NO_BLOCK);
          }
          else {
            act("You kick $N savagely, causing $M to double over in pain!", FALSE, ch, 0, victim, TO_CHAR);
            act("$n kicks you savagely, causing you to double over in pain!", FALSE, ch, 0, victim, TO_VICT);
            act("$n kicks $N savagely, causing $M to double over in pain!", FALSE, ch, 0, victim, TO_NOTVICT);

            int set_pos = stack_position(victim, POSITION_SITTING);

            damage(ch, victim, calc_position_damage(GET_POS(victim), 10), SKILL_DIRTY_TRICKS, DAM_PHYSICAL);

            if (CHAR_REAL_ROOM(victim) != NOWHERE && !IS_IMPLEMENTOR(victim)) {
              GET_POS(victim) = set_pos;

              WAIT_STATE(victim, PULSE_VIOLENCE);
            }
          }
        }
      }
    }
  }
}

void blood_lust_spec(CHAR *ch, CHAR *victim, int cmd, const char *arg) {
  if (!ch || !victim || !SAME_ROOM(ch, victim)) return;

  if (affected_by_spell(ch, SPELL_BLOOD_LUST)) {
    int check = 20;

    /* Bathed in Blood */
    if (IS_MORTAL(ch) && check_subclass(ch, SC_DEFILER, 5)) {
      check += 5 + ((CHAR_REAL_ROOM(ch) != NOWHERE) ? ROOM_BLOOD(CHAR_REAL_ROOM(ch)) : 0);
    }

    if (chance(check)) {
      int dmg = 0;

      switch (number(1, 4)) {
        case 1:
        case 2:
          act("$n bites viciously at $N with $s fangs!", TRUE, ch, 0, victim, TO_NOTVICT);
          act("$n viciously bites at you with $s fangs!", FALSE, ch, 0, victim, TO_VICT);
          act("You bite at $N viciously with your fangs!", FALSE, ch, 0, victim, TO_CHAR);

          dmg = 60;

          if (affected_by_spell(ch, SPELL_DESECRATE)) {
            dmg *= 1.1;
          }

          damage(ch, victim, dmg, TYPE_UNDEFINED, DAM_PHYSICAL);
          break;

        case 3:
          act("$n sinks $s fangs into $N's neck, draining $S life!", TRUE, ch, 0, victim, TO_NOTVICT);
          act("$n sinks $s fangs into your neck, draining your life!", FALSE, ch, 0, victim, TO_VICT);
          act("You sink your fangs into $N's neck, draining $S life!", FALSE, ch, 0, victim, TO_CHAR);

          dmg = 40;

          if (affected_by_spell(ch, SPELL_DESECRATE)) {
            dmg *= 1.1;
          }

          damage(ch, victim, dmg, TYPE_UNDEFINED, DAM_MAGICAL);

          magic_heal(ch, ch, SPELL_BLOOD_LUST, dmg, TRUE);

          GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) - dmg);
          break;

        case 4:
          act("$n bites savagely at $N, draining $S magical essence!", TRUE, ch, 0, victim, TO_NOTVICT);
          act("$n bites at you savagely, draining your magical essence!", FALSE, ch, 0, victim, TO_VICT);
          act("You bite savagely at $N, draining $S magical essence!", FALSE, ch, 0, victim, TO_CHAR);

          dmg = 20;

          if (affected_by_spell(ch, SPELL_DESECRATE)) {
            dmg *= 1.1;
          }

          GET_MANA(victim) = MAX(GET_MANA(victim) - dmg, 0);

          GET_MANA(ch) = MIN(GET_MANA(ch) + dmg, GET_MAX_MANA(ch));
          break;
      }

      if (IS_MORTAL(ch) && chance(20)) {
        send_to_room("Blood sprays across the room, staining the surroundings dark crimson.\n\r", CHAR_REAL_ROOM(ch));

        ROOM_BLOOD(CHAR_REAL_ROOM(ch)) = MIN(ROOM_BLOOD(CHAR_REAL_ROOM(ch)) + 1, 10);
      }
    }
  }
}

void shadowstep_spec(CHAR *ch, CHAR *victim, int cmd, const char *arg) {
  if (!ch || !victim || !SAME_ROOM(ch, victim)) return;

  if (IS_MORTAL(ch) && IS_SET(GET_TOGGLES(ch), TOG_SHADOWSTEP) && check_sc_access(ch, SKILL_SHADOWSTEP)) {
    int check = number(1, 450) - (GET_DEX_APP(ch) * 5);

    double multi = 1.5;

    if (!IS_SET(CHAR_ROOM_FLAGS(ch), DARK) && ((IS_DAY && IS_OUTSIDE(ch)) || IS_SET(CHAR_ROOM_FLAGS(ch), LIT))) {
      check += 50;
      multi -= 0.5;
    }

    if (IS_NIGHT) {
      check -= 15;
      multi += 0.3;

      if (IS_EVIL(ch)) {
        check -= 10;
        multi += 0.2;
      }
    }

    if (IS_SET(CHAR_ROOM_FLAGS(ch), DARK)) {
      check -= 15;
      multi += 0.3;

      if (IS_EVIL(ch)) {
        check -= 10;
        multi += 0.2;
      }
    }

    if (!CAN_SEE(victim, ch) || affected_by_spell(ch, SPELL_IMP_INVISIBLE) || affected_by_spell(ch, SPELL_BLACKMANTLE)) {
      check -= 25;
      multi += 0.5;
    }

    if (multi < 1.0)
      multi = 1.0;

    if (check_subclass(ch, SC_DEFILER, 5)) {
      multi += 0.5;
    }

    if (multi > 3.0)
      multi = 3.0;

    if (check <= GET_LEARNED(ch, SKILL_SHADOWSTEP)) {
      act("You step into the shadows and attack $N by surprise!", FALSE, ch, 0, victim, TO_CHAR);
      act("$n steps into the shadows and attacks you by surprise!", FALSE, ch, 0, victim, TO_VICT);
      act("$n steps into the shadows and attacks $N by surprise!", FALSE, ch, 0, victim, TO_NOTVICT);

      damage(ch, victim, lround(calc_hit_damage(ch, victim, EQ(ch, WIELD), 0, RND_RND) * multi), get_attack_type(ch, EQ(ch, WIELD)), DAM_PHYSICAL);
    }
  }
}

void victimize_spec(CHAR *ch, CHAR *victim, int cmd, const char *arg) {
  if (!ch || !victim || !SAME_ROOM(ch, victim)) return;

  struct victimize_data_t {
    char *name;
    int location;
    int modifier;
  };

  struct victimize_data_t victimize_data[] = {
    {
      .name = "Victimized (AC Penalty)",
      .modifier = (GET_LEVEL(ch) / 4),
      .location = APPLY_AC,
    },
    {
      .name = "Victimized (Hitroll Penalty)",
      .modifier = -(number(3, 5)),
      .location = APPLY_HITROLL,
    },
    {
      .name = "Victimized (Damroll Penalty)",
      .modifier = -((GET_DAMROLL(victim) * number(5, 10)) / 100),
      .location = APPLY_DAMROLL,
    },
  };

  /* Check access to Victimize, the skill is toggled, and that the character is evil. */
  if (IS_MORTAL(ch) && IS_SET(GET_TOGGLES(ch), TOG_VICTIMIZE) && check_sc_access(ch, SKILL_VICTIMIZE) && IS_EVIL(ch)) {
    /* Calculate the skill check. */
    int check = number(1, 171) - GET_DEX_APP(ch);

    /* Check skill success/failure. */
    if (check <= GET_LEARNED(ch, SKILL_VICTIMIZE)) {
      /* Create a list to store possible affect locations for the Victimize affect. */
      int loc_list[NUMELEMS(victimize_data)];

      /* Fill the list by checking if the victim is already enchanted by .name from victimize_data above.
         If they aren't enchanted, put the .location number in the list index.
         If they are enchanted, put -1 in the list index. */
      for (int i = 0; (i < NUMELEMS(loc_list)) && (i < NUMELEMS(victimize_data)); i++) {
        loc_list[i] = (!enchanted_by(victim, victimize_data[i].name) ? victimize_data[i].location : -1);
      }

      /* Quick sort the location list descending, so that any -1 values are at the end of the list. */
      qsort(loc_list, NUMELEMS(loc_list), sizeof(int), qcmp_int_desc);

      /* Count the number of locations from the location list that are not -1 (already applied). */
      int num_locs = 0;

      while ((num_locs < NUMELEMS(loc_list)) && (loc_list[num_locs] != -1)) {
        num_locs++;
      }

      /* Check if there's nothing to choose. */
      if (num_locs) {
        /* Shuffle the num_loc elements of the location list. */
        shuffle_int_array(loc_list, num_locs);

        /* Create a victimize_data_t structure, then populate it with the data from the
           chosen location (first index of loc_list) by searching for it in victimize_data. */
        struct victimize_data_t *vic_spec = NULL;
        for (int i = 0; !vic_spec && (i < NUMELEMS(victimize_data)); i++) {
          if (victimize_data[i].location == loc_list[0]) {
            vic_spec = &victimize_data[i];
          }
        }

        /* Sanity check. */
        if (vic_spec) {
          act("You victimize $N, inflicting physical and mental torment.", FALSE, ch, 0, victim, TO_CHAR);
          act("$n victimizes you, inflicting physical and mental torment.", FALSE, ch, 0, victim, TO_VICT);
          act("$n victimizes $N, inflicting physical and mental torment.", FALSE, ch, 0, victim, TO_NOTVICT);

          enchantment_apply(victim, FALSE, vic_spec->name, SKILL_VICTIMIZE, 20, ENCH_INTERVAL_ROUND, vic_spec->modifier, vic_spec->location, 0, 0, 0);
        }
      }
    }
  }
}

void shadow_wraith_spec(CHAR *ch, CHAR *signaler, int cmd, const char *arg) {
  if (!ch) return;

  if (!affected_by_spell(ch, SPELL_SHADOW_WRAITH)) return;

  if ((duration_of_spell(ch, SPELL_SHADOW_WRAITH) == 1) ||
      ((duration_of_spell(ch, SPELL_SHADOW_WRAITH) > 11) && !((duration_of_spell(ch, SPELL_SHADOW_WRAITH) - 2) % 10))) {
    send_to_char("One of your shadows flickers and begins to fade from reality.\n\r", ch);
    act("One of $n's shadows flickers and begins to fade from reality.", TRUE, ch, 0, 0, TO_ROOM);

    return;
  }

  if ((duration_of_spell(ch, SPELL_SHADOW_WRAITH) == 0) ||
      ((duration_of_spell(ch, SPELL_SHADOW_WRAITH) > 10) && !((duration_of_spell(ch, SPELL_SHADOW_WRAITH) - 1) % 10))) {
    /* Dusk Requiem */
    if (!IS_NPC(ch) && check_subclass(ch, SC_INFIDEL, 4)) {
      if (GET_OPPONENT(ch)) {
        /* Cast spell at level 0 to inflict double damage. */
        spell_dusk_requiem(0, ch, GET_OPPONENT(ch), 0);
      }
      else {
        int num_shadows = (MAX(1, (duration_of_spell(ch, SPELL_SHADOW_WRAITH) - 1)) / 10) + 1;

        if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
          GET_HIT(ch) = MIN(GET_MAX_HIT(ch), GET_HIT(ch) + ((10 * (100 + (20 * num_shadows))) / 2));
        }

        if (GET_MANA(ch) < GET_MAX_MANA(ch)) {
          GET_MANA(ch) = MIN(GET_MAX_MANA(ch), GET_MANA(ch) + ((10 * (100 + (20 * num_shadows))) / 2));
        }

        send_to_char("One of your shadows fades from reality and recedes back into the void.\n\r", ch);
        act("One of $n's shadows fades from reality and recedes back into the void.", TRUE, ch, 0, 0, TO_ROOM);
      }
    }
  }
}

int taunt_enchantment(ENCH *ench, CHAR *ch, CHAR *signaler, int cmd, char *arg) {
  if (!ench || !ch || !signaler) return FALSE;

  if (cmd == MSG_SHOW_AFFECT_TEXT) {
    act("......$n is distracted.", FALSE, ch, 0, signaler, TO_VICT);

    return FALSE;
  }

  return FALSE;
}

void taunt_spec(CHAR *ch, CHAR *victim, int cmd, const char *arg) {
  if (!ch || !victim || !SAME_ROOM(ch, victim)) return;

  /* Taunt Messages
  {
    TO_NOTVICT,
    TO_VICT,
    TO_CHAR
  }
  */
  char *taunt_messages[][3] = {
    {
      "With a mighty leap, $n springs over $N while simultaneously dropping trou in mid-air exposing $M to a full moon!",
      "With a mighty leap, $n springs over you while simultaneously dropping trou in mid-air exposing you to a full moon!",
      "With a mighty leap, you spring over $N while simultaneously dropping trou in mid-air exposing $M to a full moon!"
    },
    {
      "$n recounts all the storied tales of past adventurers slaughtering $N, how embarrassing.",
      "$n recounts all the storied tales of past adventurers slaughtering you, how embarrassing.",
      "You recount all the storied tales of past adventurers slaughtering $N, how embarrassing."
    },
    {
      "$n regales the room with a story about the time $e caught $N kissing Rashgugh.",
      "$n regales the room with a story about the time $e caught you kissing Rashgugh.",
      "You regale the room with a story about the time you caught $N kissing Rashgugh."
    },
    {
      "$n begins reciting Vogon poetry loudly in $N's general direction.",
      "$n begins reciting Vogon poetry loudly in your general direction, it's unbearable!",
      "You begin reciting Vogon poetry loudly in $N's general direction.",
    },
    {
      "$n captivates the room with a retelling of $s raunchy encounter with $N's mother.",
      "$n captivates the room with a retelling of $s raunchy encounter with your mother!",
      "You captivate the room with a retelling of your raunchy encounter with $N's mother.",
    },
    {
      "$n seems unperturbed battling $N, and takes the opportunity to earn a few coin by busking.",
      "$n seems unperturbed battling you, and takes the opportunity to earn a few coin by busking.",
      "You are unperturbed battling $N, and take the opportunity to earn a few coin by busking.",
    },
    {
      "$n picks apart $N's character flaws one-by-one, revealing them to the world.",
      "$n picks apart your character flaws one-by-one, revealing them to the world.",
      "You pick apart $N's character flaws one-by-one, revealing them to the world.",
    },
    {
      "$n peeks at $N's soul, disclosing $S secrets for all to hear.",
      "$n peeks at your soul, disclosing your secrets for all to hear.",
      "You peek at $N's soul, disclosing $S secrets for all to hear.",
    }
  };

  if ((GET_CLASS(ch) == CLASS_BARD) && (GET_LEVEL(ch) >= 20)) {
    if ((number(1, 1500) <= GET_LEARNED(ch, SKILL_TAUNT) + (GET_CON_REGEN(ch) * 2)) || (arg && !strcasecmp(arg, "NO_SKILL_CHECK"))) {
      int taunt_message_idx = number(0, NUMELEMS(taunt_messages) - 1);

      act(taunt_messages[taunt_message_idx][0], FALSE, ch, 0, victim, TO_NOTVICT);
      act(taunt_messages[taunt_message_idx][1], FALSE, ch, 0, victim, TO_VICT);
      act(taunt_messages[taunt_message_idx][2], FALSE, ch, 0, victim, TO_CHAR);

      enchantment_apply(victim, FALSE, "Taunted", SKILL_TAUNT, 3, ENCH_INTERVAL_ROUND, -(GET_DAMROLL(victim) / 10), APPLY_DAMROLL, 0, 0, taunt_enchantment);
    }
  }
}

void mimicry_spec(CHAR *ch, CHAR *victim, int cmd, const char *arg) {
  if (!ch || !victim || !SAME_ROOM(ch, victim)) return;

  if (!IS_MORTAL(ch) || !check_subclass(ch, SC_BLADESINGER, 5))  return;

  if ((number(1, 1500) - (GET_DEX_APP(ch) * 5)) > 100) return;

  int mimicee_count = 0;

  /* Count eligible mimicee's in the room. */
  for (CHAR *temp_ch = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)); temp_ch; temp_ch = temp_ch->next_in_room) {
    if ((temp_ch != ch) && IS_MORTAL(temp_ch) && SAME_GROUP(temp_ch, ch)) {
      mimicee_count++;
    }
  }

  if (!mimicee_count) return;

  /* Choose a random mimicee. */
  int mimicee_num = number(1, mimicee_count);

  CHAR *mimicee = NULL;

  /* Get the chosen mimicee from the room's character list. */
  for (CHAR *temp_ch = ROOM_PEOPLE(CHAR_REAL_ROOM(ch)); !mimicee && temp_ch; temp_ch = temp_ch->next_in_room) {
    if ((temp_ch != ch) && IS_MORTAL(temp_ch) && SAME_GROUP(temp_ch, ch) && (--mimicee_num == 0)) {
      mimicee = temp_ch;
    }
  }

  if (!mimicee) return;

  act("$n does $s best impression of $N.", FALSE, ch, 0, mimicee, TO_NOTVICT);
  act("$n does $s best impression of you.", FALSE, ch, 0, mimicee, TO_VICT);
  act("You do your best impression of $N.", FALSE, ch, 0, mimicee, TO_CHAR);

  int dam = 0, set_pos = 0;

  switch (GET_CLASS(mimicee)) {
    case CLASS_MAGIC_USER: /* thunderball + self perceive */
      act("$n sings 'You've been... Thunderstruck!'", FALSE, ch, NULL, NULL, TO_ROOM);
      act("You sing 'You've been... Thunderstruck!'", FALSE, ch, NULL, NULL, TO_CHAR);

      /* thunderball: we want this to ignore sphere/shield so spoof the message of the spell */
      act("$N screams loudly as a ball of thunder and lightning hits $S body.", FALSE, ch, 0, victim, TO_NOTVICT);
      act("You scream loudly when a thunderball hits your body.", FALSE, ch, 0, victim, TO_VICT);
      act("You conjure up a ball of thunder and throw it at $N.", FALSE, ch, 0, victim, TO_CHAR);

      damage(ch, victim, number(800, 1100), TYPE_UNDEFINED, DAM_SOUND);

      if (!affected_by_spell(ch, SPELL_PERCEIVE))
        spell_perceive(GET_LEVEL(ch), ch, ch, NULL);
      break;

    case CLASS_CLERIC: /* miracle victimim's opponent (best approximation of tank) */
      act("$n sings 'All I need is a miracle, all I need is you...'", FALSE, ch, NULL, NULL, TO_ROOM);
      act("You sing 'All I need is a miracle, all I need is you...'", FALSE, ch, NULL, NULL, TO_CHAR);

      if (!GET_OPPONENT(victim) || IS_NPC(GET_OPPONENT(victim)) || GET_OPPONENT(victim) == ch) {
        act("$n sumptuous vocals heal $mself.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("Your sumptuous vocals heal yourself.", FALSE, ch, NULL, NULL, TO_CHAR);

        spell_miracle(GET_LEVEL(ch), ch, ch, NULL);
      }
      else {
        act("$n miraculous vocals heal $N.", FALSE, ch, NULL, GET_OPPONENT(victim), TO_NOTVICT);
        act("$n miraculous vocals heal you.", FALSE, ch, NULL, GET_OPPONENT(victim), TO_VICT);
        act("Your miraculous vocals heal $N.", FALSE, ch, NULL, GET_OPPONENT(victim), TO_CHAR);

        spell_miracle(GET_LEVEL(ch), ch, GET_OPPONENT(victim), NULL);
      }
      break;

    case CLASS_THIEF: /* circle + twist */
      act("$n sings 'It's the circle of life, it's the wheel of fortune...'", FALSE, ch, NULL, NULL, TO_ROOM);
      act("You sing 'It's the circle of life, it's the wheel of fortune...'", FALSE, ch, NULL, NULL, TO_CHAR);

      /* circle */
      act("$n pirouettes into the light, noisily dancing $s way behind $N.", FALSE, ch, 0, victim, TO_NOTVICT);
      act("$n pirouettes into the light, you are transfixed by the shameless dance.", FALSE, ch, 0, victim, TO_VICT);
      act("You pirouette wildly into the light, noisily dancing your way behind $N.", FALSE, ch, 0, victim, TO_CHAR);

      if (EQ(ch, WIELD)) {
        act("$n plunges $p deep into $N's back.", FALSE, ch, EQ(ch, WIELD), victim, TO_NOTVICT);
        act("$n plunges $p deep into your back.", FALSE, ch, EQ(ch, WIELD), victim, TO_VICT);
        act("You plunge $p deep into $N's back.", FALSE, ch, EQ(ch, WIELD), victim, TO_CHAR);
      }
      else if (EQ(ch, HOLD)) { // instrument of death!
        act("$n plunges $p deep into $N's back.", FALSE, ch, EQ(ch, HOLD), victim, TO_NOTVICT);
        act("$n plunges $p deep into your back.", FALSE, ch, EQ(ch, HOLD), victim, TO_VICT);
        act("You plunge $p deep into $N's back.", FALSE, ch, EQ(ch, HOLD), victim, TO_CHAR);
      }
      else { // no hold
        act("$n plunges $s tiny fist deep into $N's back!", FALSE, ch, NULL, victim, TO_NOTVICT);
        act("$n plunges $s tiny fist deep into your back!", FALSE, ch, NULL, victim, TO_VICT);
        act("You plunge your tiny fist deep into $N's back!", FALSE, ch, NULL, victim, TO_CHAR);
      }

      dam = circle_mult[GET_LEVEL(ch)] * calc_hit_damage(ch, victim, GET_WEAPON(ch), 0, RND_RND);

      damage(ch, victim, dam, TYPE_UNDEFINED, DAM_PHYSICAL);

      /* twist */
      if (CHAR_REAL_ROOM(victim) != NOWHERE) {
        if (EQ(ch, WIELD)) {
          act("$n gruesomely twists $s weapon in the flesh of $N.", TRUE, ch, NULL, victim, TO_NOTVICT);
          act("You writhe in pain as $n twists his weapon in your back.", FALSE, ch, NULL, victim, TO_VICT);
          act("You twist your weapon in the flesh of $N.", FALSE, ch, NULL, victim, TO_CHAR);
        }
        else if (EQ(ch, HOLD)) {
          act("$n gruesomely twists $s $p in the flesh of $N.", TRUE, ch, EQ(ch, HOLD), victim, TO_NOTVICT);
          act("You writhe in pain as $n twists $s $p in your back.", FALSE, ch, EQ(ch, HOLD), victim, TO_VICT);
          act("You twist $p in the flesh of $N.", FALSE, ch, EQ(ch, HOLD), victim, TO_CHAR);
        }
        else {
          act("$n gruesomely twists $s tiny fist in the flesh of $N, shouting 'Kalima!'", TRUE, ch, NULL, victim, TO_NOTVICT);
          act("You writhe in pain as $n twists $s tiny fist in your back.", FALSE, ch, NULL, victim, TO_VICT);
          act("You twist your tiny fist in the flesh of $N, shouting 'Kalima!'", FALSE, ch, NULL, victim, TO_CHAR);
        }

        damage(ch, victim, 250, SKILL_TWIST, DAM_PHYSICAL);
      }
      break;

    case CLASS_WARRIOR: /* punch + quad OR disembowel (victimim hp dependent) */
      if (GET_HIT(victim) > lround(GET_MAX_HIT(victim) * 0.3)) {
        act("$n sings 'I'm gonna knock you out, Mama said knock you out...'", FALSE, ch, NULL, NULL, TO_ROOM);
        act("You sing 'I'm gonna knock you out, Mama said knock you out...'", FALSE, ch, NULL, NULL, TO_CHAR);

        /* punch */
        act("$n strikes $N with the feeble fist of a musician.", FALSE, ch, 0, victim, TO_NOTVICT);
        act("$n strikes you with the feeble fist of a musician.", FALSE, ch, 0, victim, TO_VICT);
        act("You strike $N with your feeble musician's fist.", FALSE, ch, 0, victim, TO_CHAR);

        set_pos = stack_position(victim, POSITION_SITTING);

        damage(ch, victim, 2 * GET_LEVEL(ch), TYPE_UNDEFINED, DAM_PHYSICAL);

        if (CHAR_REAL_ROOM(victim) != NOWHERE)
          GET_POS(victim) = MIN(GET_POS(ch), set_pos);

        /* quad - no more no less */
          perform_hit(ch, victim, TYPE_UNDEFINED, 0);
          perform_hit(ch, victim, TYPE_UNDEFINED, 0);
          perform_hit(ch, victim, TYPE_UNDEFINED, 0);
          perform_hit(ch, victim, TYPE_UNDEFINED, 0);
      }
      else {
        /* disembowel */
        act("$n sings 'You better call me a doctor, feelin' no pain...'", FALSE, ch, NULL, NULL, TO_ROOM);
        act("You sing 'You better call me a doctor, feelin' no pain...'", FALSE, ch, NULL, NULL, TO_CHAR);

        dam = number(GET_LEVEL(ch) / 10, GET_LEVEL(ch) / 5) * calc_hit_damage(ch, victim, GET_WEAPON(ch), 0, RND_RND);

        act("$n's one-in-a-million attack causes $N's guts to spill out onto $s feet.", FALSE, ch, 0, victim, TO_NOTVICT);
        act("$n's one-in-a-million attack causes your guts to spill out onto $s feet.", FALSE, ch, 0, victim, TO_VICT);
        act("Your one-in-a-million attack causes $N's guts to spill out onto your feet.", FALSE, ch, 0, victim, TO_CHAR);

        damage(ch, victim, dam, SKILL_DISEMBOWEL, DAM_PHYSICAL);
      }
      break;

    case CLASS_NINJA: /* pummel, dual hit, divine wind, spoofed kick */
      act("$n sings 'Everybody is Kung Fu fighting!'", FALSE, ch, NULL, NULL, TO_ROOM);
      act("You sing 'Everybody is Kung Fu fighting!'", FALSE, ch, NULL, NULL, TO_CHAR);

      /* spoof pummel: smack with hold (instrument) */
      if (EQ(ch, HOLD)) {
        act("$n pummels $N with $p, and $N is stunned now!", FALSE, ch, EQ(ch, HOLD), victim, TO_NOTVICT);
        act("$n pummels you with $p, and you are stunned now!", FALSE, ch, EQ(ch, HOLD), victim, TO_VICT);
        act("You pummel $N with $p, and $N is stunned now!", FALSE, ch, EQ(ch, HOLD), victim, TO_CHAR);
      }
      else {
        act("$n pummels $N, and $N is stunned now!", FALSE, ch, NULL, victim, TO_NOTVICT);
        act("$n pummels you, and you are stunned now!", FALSE, ch, NULL, victim, TO_VICT);
        act("You pummel $N, and $N is stunned now!", FALSE, ch, NULL, victim, TO_CHAR);
      }

      set_pos = stack_position(victim, POSITION_STUNNED);

      damage(ch, victim, calc_position_damage(GET_POS(victim), 10), SKILL_PUMMEL, DAM_PHYSICAL);

      if (CHAR_REAL_ROOM(victim) != NOWHERE)
        GET_POS(victim) = MIN(GET_POS(ch), set_pos);

      /* dual hit */
        perform_hit(ch, victim, TYPE_UNDEFINED, 0);
        perform_hit(ch, victim, TYPE_UNDEFINED, 0);
      /* haste (in lieu mystic swiftness) */
      if (affected_by_spell(ch, SPELL_HASTE) && chance(30 + GET_DEX_APP(ch)))
        perform_hit(ch, victim, TYPE_UNDEFINED, 0);

      /* divine wind: we want this to ignore sphere/shield so spoof the message of the spell */
      if (SAME_ROOM(ch, victim)) {
        act("A wispy spirit wind blows by, almost knocking you over.", FALSE, ch, NULL, victim, TO_NOTVICT);
        act("Malicious spirits materialize in front of you, assaulting you viciously!", FALSE, ch, NULL, victim, TO_VICT);
        act("Malicious spirits materialize in front of $N, assaulting $M viciously!", FALSE, ch, NULL, victim, TO_CHAR);

        set_pos = stack_position(victim, POSITION_RESTING);

        damage(ch, victim, 300, TYPE_UNDEFINED, DAM_MAGICAL);

        if (CHAR_REAL_ROOM(victim) != NOWHERE)
          GET_POS(victim) = MIN(GET_POS(ch), set_pos);
      }

      /* spoof kick */
      if (SAME_ROOM(ch, victim)) {
        act("$n's graceful, balletic kick catches $N by surprise.", FALSE, ch, NULL, victim, TO_NOTVICT);
        act("$n's graceful, balletic kick catches you by surprise.", FALSE, ch, NULL, victim, TO_VICT);
        act("Your graceful, balletic kick catches $N by surprise.", FALSE, ch, NULL, victim, TO_CHAR);

        damage(ch, victim, (GET_LEVEL(ch) * 2), TYPE_UNDEFINED, DAM_PHYSICAL);
      }
      break;

    case CLASS_PALADIN: /* self fury for 0 ticks, spoofed pummel */
      act("$n sings 'But what makes a man decide, take the wrong or righteous road...'", FALSE, ch, NULL, NULL, TO_ROOM);
      act("You sing 'But what makes a man decide, take the wrong or righteous road...'", FALSE, ch, NULL, NULL, TO_CHAR);

      /* fury */
      if (!affected_by_spell(ch, SPELL_FURY)) {
        affect_apply(ch, SPELL_FURY, 0, 0, 0, AFF_FURY, 0);

        send_to_char("You feel very angry.\n\r", ch);
        act("$n starts snarling and fuming with fury.", FALSE, ch, 0, 0, TO_ROOM);
      }

      /* spoof pummel: smack with hold (instrument) */
      if (EQ(ch, HOLD)) {
        act("$n pummels $N with $p, and $N is stunned now!", FALSE, ch, EQ(ch, HOLD), victim, TO_NOTVICT);
        act("$n pummels you with $p, and you are stunned now!", FALSE, ch, EQ(ch, HOLD), victim, TO_VICT);
        act("You pummel $N with $p, and $N is stunned now!", FALSE, ch, EQ(ch, HOLD), victim, TO_CHAR);
      }
      else {
        act("$n pummels $N, and $N is stunned now!", FALSE, ch, NULL, victim, TO_NOTVICT);
        act("$n pummels you, and you are stunned now!", FALSE, ch, NULL, victim, TO_VICT);
        act("You pummel $N, and $N is stunned now!", FALSE, ch, NULL, victim, TO_CHAR);
      }

      set_pos = stack_position(victim, POSITION_STUNNED);

      damage(ch, victim, calc_position_damage(GET_POS(victim), 10), SKILL_PUMMEL, DAM_PHYSICAL);

      if (CHAR_REAL_ROOM(victim) != NOWHERE)
        GET_POS(victim) = MIN(GET_POS(ch), set_pos);
      break;

    case CLASS_ANTI_PALADIN: /* self blood lust & rage for 0 ticks, spoofed pummel + hidden-blade */
      act("$n sings 'Well, I'm hot blooded, check it and see...'", FALSE, ch, NULL, NULL, TO_ROOM);
      act("You sing 'Well, I'm hot blooded, check it and see...'", FALSE, ch, NULL, NULL, TO_CHAR);

      if (!affected_by_spell(ch, SPELL_BLOOD_LUST)) {
        affect_apply(ch, SPELL_BLOOD_LUST, 0, 0, 0, 0, 0);

        send_to_char("Your body writhes with a gnawing hunger for blood!\n\r", ch);
        act("$n's body writhes with a gnawing hunger for blood!", FALSE, ch, 0, 0, TO_ROOM);
      }

      if (!affected_by_spell(ch, SPELL_RAGE)) {
        affect_apply(ch, SPELL_RAGE, 0, 0, 0, 0, AFF2_RAGE);

        send_to_char("Rage courses through your body!\n\r", ch);
        act("Rage courses through $n's body!", FALSE, ch, 0, 0, TO_ROOM);
      }

      /* spoof pummel + hidden-blade: smack with hold (instrument) */
      if (EQ(ch, HOLD)) {
        act("$n pummels $N with $p, and $N is stunned now!", FALSE, ch, EQ(ch, HOLD), victim, TO_NOTVICT);
        act("$n pummels you with $p, and you are stunned now!", FALSE, ch, EQ(ch, HOLD), victim, TO_VICT);
        act("You pummel $N with $p, and $N is stunned now!", FALSE, ch, EQ(ch, HOLD), victim, TO_CHAR);
      }
      else {
        act("$n pummels $N, and $N is stunned now!", FALSE, ch, NULL, victim, TO_NOTVICT);
        act("$n pummels you, and you are stunned now!", FALSE, ch, NULL, victim, TO_VICT);
        act("You pummel $N, and $N is stunned now!", FALSE, ch, NULL, victim, TO_CHAR);
      }

      set_pos = stack_position(victim, POSITION_STUNNED);

      damage(ch, victim, calc_position_damage(GET_POS(victim), 10), SKILL_PUMMEL, DAM_PHYSICAL);

      /* spoofed hidden-blade */
      if (CHAR_REAL_ROOM(victim) != NOWHERE) {
        act("$n drives a hidden blade deep into $N's gut!", FALSE, ch, NULL, victim, TO_NOTVICT);
        act("$n drives a hidden blade deep into your gut!", FALSE, ch, NULL, victim, TO_VICT);
        act("You drive a hidden blade deep into $N's gut!", FALSE, ch, NULL, victim, TO_CHAR);

        damage(ch, victim, calc_position_damage(GET_POS(victim), GET_LEVEL(ch) * 2), SKILL_HIDDEN_BLADE, DAM_PHYSICAL);
      }

      if (CHAR_REAL_ROOM(victim) != NOWHERE)
        GET_POS(victim) = MIN(GET_POS(ch), set_pos);
      break;

    case CLASS_BARD: /* taunt + dual + heal song */
      if (chance(50)) {
        act("$n sings 'I'm a loser baby so why don't you kill me?' mockingly off-key and too loud.", FALSE, ch, NULL, NULL, TO_ROOM);
        act("You sing 'I'm a loser baby so why don't you kill me?' intentionally off-key and too loud.", FALSE, ch, NULL, NULL, TO_CHAR);
      }
      else {
        act("$n liltingly sings 'Anything you can do I can do better, anything you can do I can do better than you.'", FALSE, ch, NULL, NULL, TO_ROOM);
        act("You liltingly sing 'Anything you can do I can do better, anything you can do I can do better than you.'", FALSE, ch, NULL, NULL, TO_CHAR);
      }

      taunt_spec(ch, victim, 0, "NO_SKILL_CHECK"); // auto success

      /* dual + haste */
        perform_hit(ch, victim, TYPE_UNDEFINED, 0);
        /* haste */
        if (affected_by_spell(ch, SPELL_HASTE) && chance(30 + GET_DEX_APP(ch)))
          perform_hit(ch, victim, TYPE_UNDEFINED, 1);
        perform_hit(ch, victim, TYPE_UNDEFINED, 0);

      act("$n sings 'There is no pain you are receding...', a much better rendition than $N ever managed.", FALSE, ch, NULL, mimicee, TO_NOTVICT);
      act("$n sings 'There is no pain you are receding...', a much better rendition than you ever could.", FALSE, ch, NULL, mimicee, TO_VICT);
      act("You sing 'There is no pain you are receding...', a much better rendition than $N ever managed.", FALSE, ch, NULL, mimicee, TO_CHAR);

      for (CHAR *temp_target = world[CHAR_REAL_ROOM(ch)].people, *next_target; temp_target; temp_target = next_target) {
        next_target = temp_target->next_in_room;

        if (ch != temp_target && (!IS_NPC(temp_target) || SAME_GROUP(ch, temp_target))) {
          spell_heal(GET_LEVEL(ch), ch, temp_target, NULL);
        }
      }

      spell_heal(GET_LEVEL(ch), ch, ch, NULL);
      break;

    case CLASS_COMMANDO: /* triple + eshock + disarm */
      act("$n sings 'Disarm you with a smile, and cut you like you want me to...'", FALSE, ch, NULL, NULL, TO_ROOM);
      act("You sing 'Disarm you with a smile, and cut you like you want me to...'", FALSE, ch, NULL, NULL, TO_CHAR);

      /* disarm victim if wielding */
      if (EQ(victim, WIELD) && !(IS_NPC(victim) && IS_SET(GET_ACT(victim), ACT_ARM))) {
        const int WYVERN_TAIL = 11523;
        const int UBER_KATANA = 20102;

        OBJ *weapon = EQ(victim, WIELD);

        if (V_OBJ(weapon) != WYVERN_TAIL && (V_OBJ(weapon) != UBER_KATANA || !IS_NPC(victim))) {
          act("$n backflips into a scorpion kick, knocking $N's weapon loose!", FALSE, ch, 0, victim, TO_NOTVICT);
          act("$N backflips into a beautiful scorpion kick, knocking your weapon loose!", FALSE, ch, 0, victim, TO_VICT);
          act("Your elegant backflip into scorpion kick has knocked $N's weapon loose!", FALSE, ch, 0, victim, TO_CHAR);

          unequip_char(victim, WIELD);

          if (IS_SET(ROOM_FLAGS(CHAR_REAL_ROOM(ch)), CHAOTIC)) {
            obj_to_char(weapon, victim);
          }
          else {
            log_f("WIZLOG: %s disarms %s's %s (Room %d).", GET_DISP_NAME(ch), GET_DISP_NAME(victim), OBJ_SHORT(weapon), ROOM_VNUM(CHAR_REAL_ROOM(ch)));
            OBJ_LOG(weapon) = TRUE;

            obj_to_room(weapon, CHAR_REAL_ROOM(victim));
          }

          save_char(victim, NOWHERE);
        }
      }

      /* triple - no more no less */
        perform_hit(ch, victim, TYPE_UNDEFINED, 0);
        perform_hit(ch, victim, TYPE_UNDEFINED, 0);
        perform_hit(ch, victim, TYPE_UNDEFINED, 0);

      /* eshock: we want this to ignore sphere/shield so spoof the message of the spell */
      if (SAME_ROOM(ch, victim)) {
        act("$N screams loudly when $n touches $S body.", FALSE, ch, 0, victim, TO_NOTVICT);
        act("You scream loudly when 50.000 volts surge through your body.", FALSE, ch, 0, victim, TO_VICT);
        act("Your hands are full of electricity and you touch $N.", FALSE, ch, 0, victim, TO_CHAR);

        damage(ch, victim, number(450, 500), TYPE_UNDEFINED, DAM_ELECTRIC);
      }
      break;
  }
}

void sidearm_spec(CHAR *ch, CHAR *victim, int cmd, const char *arg) {
  if (!ch || !victim || !SAME_ROOM(ch, victim)) return;

  if (IS_MORTAL(ch) && check_subclass(ch, SC_MERCENARY, 5) && GET_WEAPON2(ch) && !enchanted_by(ch, "Readying Sidearm...")) {
    act("You draw your sidearm and attack $N!", FALSE, ch, 0, victim, TO_CHAR);
    act("$n draws $s sidearm and attacks $N!", FALSE, ch, 0, victim, TO_ROOM);

    hit(ch, victim, TYPE_WEAPON2);

    enchantment_apply(ch, FALSE, "Readying Sidearm...", 0, number(4, 6), ENCH_INTERVAL_ROUND, 0, 0, 0, 0, 0);
  }
}

void snipe_spec(CHAR *ch, CHAR *victim, int cmd, const char *arg) {
  if (!ch || !victim || !SAME_ROOM(ch, victim)) return;

  if ((IS_MORTAL(ch) && IS_SET(GET_TOGGLES(ch), TOG_SNIPE) && check_sc_access(ch, SKILL_SNIPE)) || (arg && !strcasecmp(arg, "NO_SKILL_CHECK"))) {
    char sniped_by_str[MIL];

    snprintf(sniped_by_str, sizeof(sniped_by_str), "Sniped by %s", GET_DISP_NAME(ch));

    if (!ench_enchanted_by(victim, sniped_by_str, 0)) {
      int dmg = GET_LEVEL(ch) * number(15, 25);

      double multi = 1.0;
      if (IS_AFFECTED(victim, AFF_SANCTUARY) && !IS_AFFECTED2(victim, AFF2_FORTIFICATION))
        multi = 0.5;
      else if (!IS_AFFECTED(victim, AFF_SANCTUARY) && IS_AFFECTED2(victim, AFF2_FORTIFICATION))
        multi = 0.85;
      else if (IS_AFFECTED(victim, AFF_SANCTUARY) && IS_AFFECTED2(victim, AFF2_FORTIFICATION))
        multi = 0.35;

      int check = 0;
      double percent = (double)GET_HIT(victim) / (double)GET_MAX_HIT(victim);
      if (GET_HIT(victim) > lround(dmg * multi))
        check = (int)(100 * (0.5 - percent));
      else
        check = MIN(100, (int)(10 * (1.0 / percent)));

      if (chance(check)) {
        act("You take advantage of $N's weakness and snipe $M with a deadly attack!", FALSE, ch, 0, victim, TO_CHAR);
        act("$n takes advantage of your weakness and snipes you with a deadly attack!", FALSE, ch, 0, victim, TO_VICT);
        act("$n takes advantage of $N's weakness and snipes $m with a deadly attack!", FALSE, ch, 0, victim, TO_NOTVICT);

        damage(ch, victim, dmg, SKILL_SNIPE, DAM_PHYSICAL);

        ench_apply(victim, FALSE, sniped_by_str, 0, 7, ENCH_INTERVAL_ROUND, 0, 0, 0, 0, 0);
      }
    }
  }
}

/* Called in signal_char() in comm.c and passed the command that is being
   signaled. Use this function to process command signals sent to player
   characters that have no other method of acting on the signal (such as an
   enchantment).

   Do not change ch->func to point to any other function, unless you are
   absolutely certain you know what you're doing. */
int char_spec(CHAR *ch, CHAR *signaler, int cmd, const char *arg) {
  if (!ch) return FALSE;

  switch (cmd) {
    case MSG_MOBACT:
      adrenaline_rush_spec(ch, signaler, cmd, arg);
      break;

    case MSG_TICK:
      shadow_wraith_spec(ch, signaler, cmd, arg);
      break;

    case MSG_VIOLENCE:
      shadowstep_spec(ch, signaler, cmd, arg);
      break;

    case MSG_VIOLENCE_POST_HIT:
      dirty_tricks_spec(ch, signaler, cmd, arg);

      blood_lust_spec(ch, signaler, cmd, arg);
      victimize_spec(ch, signaler, cmd, arg);

      taunt_spec(ch, signaler, cmd, arg);
      mimicry_spec(ch, signaler, cmd, arg);

      sidearm_spec(ch, signaler, cmd, arg);
      snipe_spec(ch, signaler, cmd, arg);
      break;
  }

  return FALSE;
}
