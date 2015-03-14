/* ************************************************************************
*  file: spell_parser.c , Basic routines and parsing      Part of DIKUMUD *
*  Usage : Interpreter of spells                                          *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

/*
$Author: ronin $
$Date: 2005/01/21 14:55:29 $
$Header: /home/ronin/cvs/ronin/spell_parser.c,v 2.22 2005/01/21 14:55:29 ronin Exp $
$Id: spell_parser.c,v 2.22 2005/01/21 14:55:29 ronin Exp $
$Name:  $
$Log: spell_parser.c,v $
Revision 2.22  2005/01/21 14:55:29  ronin
Update to pfile version 5 and obj file version 3.  Additions include
bitvector2 for affected_by and enchanted_by, bitvector2 addition to
objects, increase in possible # of spells/skills to 500, addition
of space for object spells.

Revision 2.21  2004/11/19 17:20:33  void
Changed name of lvl 50 ninja spell to mystic swiftness

Revision 2.20  2004/11/19 14:47:21  void
New Level 50 Ninja Spell (adds chance of 3rd hit)

Revision 2.19  2004/11/17 19:21:52  void
Added Nomad Skill Cover (1/2 Damage -10 Hitroll)

Revision 2.18  2004/11/16 05:05:03  ronin
Chaos 2004 Update.

Revision 2.17  2004/10/21 17:08:22  void
Added level 50 AP spell Blood Lust

Revision 2.16  2004/09/30 15:59:00  ronin
Fixed mana reduction calc at level 50.

Revision 2.15  2004/09/29 22:56:13  void
Spell Rush for Commando

Revision 2.14  2004/09/28 20:07:03  void
Some more fixes to Divine Intervention

Revision 2.13  2004/09/28 14:28:01  void
Added Level 50 Cleric Spell Divine Intervention

Revision 2.12  2004/09/22 21:58:26  void
Added level 50 Mage spell Quick (Allows for 2 casts in a single combat
round)

Revision 2.11  2004/09/20 23:08:14  void
Increased Fury mana cost from 60 to 150 @ level 50

Revision 2.10  2004/09/20 15:01:13  void
Added chance of 10% spell cost reduction @ 50

Revision 2.9  2004/09/13 21:35:37  void
Bard - Haste, Paladin - Improved Fury

Revision 2.8  2004/08/16 18:13:36  void
Added Warrior Skill Quad

Revision 2.7  2004/06/30 19:57:28  ronin
Update of saving throw routine.

Revision 2.6  2004/02/26 18:02:10  void
Gave Mages Shroud of Existence Spell

Revision 2.5  2004/02/19 19:20:22  void
Added Spell Shroud of Existence (Good Align Commando Spell)

Revision 2.4  2004/02/14 13:12:50  void
Fixed a couple errors I made

Revision 2.2  2004/02/12 21:48:42  Void
Removed Detect Magic From Bard Spells

Revision 2.1  2004/02/11 22:21:42  Void
Changed Paladin spells
removed forget
removed cure serious spray
changed super harm to level 32 spell

Revision 2.0.0.1  2004/02/05 16:11:43  ronin
Reinitialization of cvs archives


Revision - changed mana cost of disrupt_sanct from 200 to 350 and increased
success of cast in subclass.spells.c

Revision - change of rejuvenation song words to There Is No Heart That Won't Heal

Revision 1.6  2002/06/05 02:56:07  ronin
Fix for power of faith to be castable in battle.

Revision 1.5  2002/04/27 05:47:53  ronin
Fix for signal MSG_SPELL_CAST.

Revision 1.4  2002/04/23 13:50:28  ronin
Addition of MSG_SPELL_CAST signal to caster.

Revision 1.3  2002/04/18 18:44:14  ronin
Fix to have mana deducted before the spell is cast.

Revision 1.2  2002/03/31 07:42:16  ronin
Addition of header lines.

$State: Exp $
*/


#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"
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

#define MANA_MU 1
#define MANA_CL 1
#define MANA_NI 1
#define MANA_PA 1
#define MANA_AN 1
#define MANA_AV 1
#define MANA_BA 1
#define MANA_CO 1

#define \
SPELLO(nr,beat,pos,mlev,clev,nlev,plev,aplev,avlev,blev,colev,mana,tar,func) \
{ spell_info[nr].spell_pointer = (func);    \
    spell_info[nr].beats = (beat);            \
    spell_info[nr].minimum_position = (pos);  \
    spell_info[nr].min_usesmana = (mana);     \
    spell_info[nr].min_level_cleric = (clev); \
    spell_info[nr].min_level_magic = (mlev);  \
    spell_info[nr].min_level_ninja = (nlev);  \
    spell_info[nr].min_level_paladin = (plev);  \
    spell_info[nr].min_level_anti_paladin = (aplev);  \
    spell_info[nr].min_level_avatar = (avlev);  \
    spell_info[nr].min_level_bard = (blev);  \
    spell_info[nr].min_level_commando = (colev);  \
    spell_info[nr].targets = (tar);           \
}

/* 100 is the MAX_MANA for a character */

/* old use_mana
#define USE_MANA(ch, sn)                            \
  MAX(spell_info[sn].min_usesmana, 100/(2+GET_LEVEL(ch)-SPELL_LEVEL(ch,sn)))
  */

/* Global data */
extern int CHAOSMODE;

extern struct room_data *world;
extern struct char_data *character_list;
extern char *spell_wear_off_msg[];
extern struct wis_app_type wis_app[];
extern struct int_app_type int_app[];


/* Extern procedures */

void set_fighting(CHAR *ch, CHAR *vict);
struct spell_info_type spell_info[MAX_SPL_LIST+1];

char *spells[]= {
  "armor",               /* 1 */
  "teleport",
  "bless",
  "blindness",
  "burning hands",
  "call lightning",
  "charm person",
  "chill touch",
  "clone",
  "colour spray",
  "control weather",     /* 11 */
  "create food",
  "create water",
  "cure blind",
  "cure critic",
  "cure light",
  "curse",
  "detect alignment",
  "detect invisibility",
  "detect magic",
  "detect poison",       /* 21 */
  "dispel evil",
  "earthquake",
  "enchant weapon",
  "energy drain",
  "fireball",
  "harm",
  "heal",
  "invisibility",
  "lightning bolt",
  "locate object",      /* 31 */
  "magic missile",
  "poison",
  "protection from evil",
  "remove curse",
  "sanctuary",
  "shocking grasp",
  "sleep",
  "strength",
  "summon",
  "ventriloquate",      /* 41 */
  "word of recall",
  "remove poison",
  "sense life",         /* 44 */
  "sneak",        /* 45 */
  "hide",
  "steal",
  "backstab",
  "pick",
  "kick",         /* 50 */
  "bash",
  "rescue",
  "block",
  "knock",
  "punch",
  "parry",
  "dual",
  "throw",
  "dodge",
  "peek",
  "butcher",      /* 61 */
  "trap",
  "disarm",
  "subdue",
  "circle",
  "triple",
  "ambush",
  "spin",
  "assault",
  "disembowel",
  "identify",           /* 71 */
  "cure serious",
  "infravision",
  "regeneration",
  "vitality",
  "cure light spray",
  "cure serious spray",
  "cure critic spray",
  "heal spray",
  "death spray",
  "holy word",            /*81*/
  "iceball",
  "total recall",
  "recharge",
  "miracle",
  "fly",
  "mana transfer",
  "holy bless",
  "evil bless",
  "satiate",
  "animate dead",          /*91*/
  "great miracle",
  "flamestrike",
  "spirit levy",
  "lethal fire",
  "hold",
  "sphere",
  "improved invisibility",
  "invulnerability",
  "fear",
  "forget",           /*101*/
  "fury",
  "endure",
  "blindness dust",
  "poison smoke",
  "hell fire",
  "hypnotize",
  "recover mana",
  "thunderball",
  "electric shock",
  "paralyze",        /*111*/
  "remove paralysis",
  "dispel good",
  "evil word",
  "reappear",
  "reveal",
  "relocation",
  "locate character",
  "super harm",
  "pummel", /* 120 */
  "legend lore",
  "great mana", /*god only*/
  "fartmouth",
  "perceive",
  "",
  "increase mana",
  "haste",
  "power word kill",
  "dispel magic",
  "conflagration",
  "mass invisibility",     /*131*/
  "convergence",
  "enchant armour",
  "disintegrate",
  "confusion",
  "vampiric touch",
  "searing orb",
  "clairvoyance",
  "firebreath",
  "lay hands",
  "dispel sanctuary", /* Ranger - Sept 96 */     /*141*/
  "disenchant", /* Quack - Oct 96 */
  "petrify", /* Quack - Oct 96 */
  "backflip", /* Ranger - July 97 */
  "protection from good", /* 145 -  Sumo Dec 97 */
  "remove improved invisibility",
  "shroud of existence", /* 147 - Void Feb 04 */
  "quad", /*148 - Void Aug 04 */
  "quick", /*149 - Void Sept 04 */
  "divine intervention", /*150 - Void Sept 04 */
  "rush", /*151 - Void Sept 04 */
  "blood lust", /*152 - Void Oct 04 */
  "scan", /* 153 Used to be Cover */
  "mystic swiftness", /* 154 - Void Nov 04 */
  "",
  "",
  "",
  "",
  "", /*155-159*/
  "",
  "",                      /*161*/
  "",
  "twist",/*163 - Void Oct 04 */
  "cunning", /* 164 */
  "wind slash", /* 165 */ 
  "","","","","","",/* 166-171 */
  "debilitate", /* 172 */ 
  "mana heal", /* 173 */
  "clobber", /* 174 - Hemp Nov 10 */
  "blur", /* 175 */
  "",/* 176 */
  "tranquility", /* 177 */
  "vehemence", /* 178 Used to be Execute */
  "tremor", /*179*/
  "shadow wraith",/*180*/
  "devastation",/*181*/
  "incendiary cloud",/*182*/
  "",/*183*/
  "riposte",/*184*/
  "trophy", /* 185 Used to be Cover */
  "frenzy",/*186*/
  "power of faith",/*187*/
  "focus",/*188*/
  "power of devotion",/*189*/
  "wrath of god",/*190*/
  "disrupt sanctuary",/*191*/
  "fortification",/*192*/
  "degenerate",/*193*/
  "magic armament",/*194*/
  "ethereal nature",/*195*/
  "engage",/*196*/
  "mantra",/*197*/
  "banzai",/*198*/
  "headbutt", /*199*/
  "",/*200 - skipped*/
  "charge",/*201*/
  "aid",/*202*/
  "demonic thunder",/*203*/
  "shadow-walk",/*204*/
  "batter",/*205*/
  "desecrate",/*206*/
  "defend",/*207*/
  "hostile",/*208*/
  "rimefang",/*209*/
  "dark ritual",/*210*/
  "blackmantle",/*211*/
  "divine wind",/*212*/
  "sweep",/*213*/
  "impair", /* 214 No longer a command; can be removed. */
  "flank",/*215*/
  "rejuvenation",/*216*/
  "wall of thorns",/*217*/
  "meteor",/*218*/
  "berserk",/*219*/
  "awareness",/*220*/
  "feint",/*221*/
  "evade",/*222*/
  "camp",/*223*/
  "luck",/*224*/
  "warchant",/*225*/
  "rally",/*226*/
  "evasion",/*227 Used to be Scan */
  "tigerkick",/*228*/
  "trip",/*229*/
  "dirty tricks",/*230*/
  "switch",/*231*/
  "pray",/*232*/
  "backfist",/*233*/
  "retreat",/*234*/
  "fade",/*235*/
  "cloud of confusion",/*236*/
  "lunge",/*237*/
  "rage",/*238*/
  "righteousness",/*239*/
  "protect", /*240*/
  "wrath of ancients",/*241*/
  "victimize", /*242*/
  "meditate",/*243*/
  "sanctify",/*244*/
  "blitz",/*245*/
  "orb of protection",/*246*/
  "assassinate",/*247*/
  "frost bolt",/*248*/
  "iron skin",/*249*/
  "distortion",/*250*/
  "passdoor",/*251*/
  "blade barrier",/*252*/
  "might", /*253*/
  "clarity",/*254*/

/*
  "quest champion",
  "spiritwrack",
  "spectral blade",
  "doppelganger",
  "shapeshift",
  "silence",
  "tranquility",
  "leech",
  "armageddon",
*/
  "\n"
};

void affect_update()
{
  static CHAR *ch;
  static AFF *tmp_af;
  static AFF *af_next;
  char buf[MSL];

  for (ch = character_list; ch; ch = ch->next)
  {
    for (tmp_af = ch->affected; tmp_af; tmp_af = af_next)
    {
      af_next = tmp_af->next;

      /* Mantra is handled in signal_char() in comm.c */
      if (tmp_af->type == SKILL_MANTRA) continue;

      if (tmp_af->duration > 0)
      {
        tmp_af->duration--;
      }
      else
      {
        if (tmp_af->duration > -1)
        {
          /* It must be a spell. */
          if (((tmp_af->type > 0) && (tmp_af->type <= 44)) || ((tmp_af->type > 70) && (tmp_af->type <= 254)))
          {
            if (!tmp_af->next || (tmp_af->next->type != tmp_af->type) || (tmp_af->next->duration > 0))
            {
              if (*spell_wear_off_msg[tmp_af->type])
              {
                sprintf(buf, "%s\n\r", spell_wear_off_msg[tmp_af->type]);
                send_to_char(buf, ch);
              }
            }
          }

          affect_remove(ch, tmp_af);
        }
      }
    }
  }
}

int SPELL_LEVEL(struct char_data *ch, int sn)
{
  if (GET_CLASS(ch) == CLASS_NINJA)
    {  return (spell_info[sn].min_level_ninja);
      }
  else if (GET_CLASS(ch) == CLASS_PALADIN)
    {  return (spell_info[sn].min_level_paladin);
      }
  else if (GET_CLASS(ch) == CLASS_ANTI_PALADIN)
    {  return (spell_info[sn].min_level_anti_paladin);
      }
  else if (GET_CLASS(ch) == CLASS_AVATAR)
    {  return (spell_info[sn].min_level_avatar);
      }
  else if (GET_CLASS(ch) == CLASS_BARD)
    {  return (spell_info[sn].min_level_bard);
      }
  else if (GET_CLASS(ch) == CLASS_MAGIC_USER)
    {  return (spell_info[sn].min_level_magic);
      }
  else if (GET_CLASS(ch) == CLASS_COMMANDO)
    {  return (spell_info[sn].min_level_commando);
      }
  else {
    return (spell_info[sn]. min_level_cleric);
  }
}

char *songs[] =
{
  "somebody's watching me",           /* detect invis */
  "seal with a kiss",                 /* armor */
  "like a prayer",                    /* bless */
  "one day I will fly away",          /* fly */
  "we don't need another hero",       /* cure critical */
  "smoke get in your eyes",           /* blindness */
  "best of me",                       /* statiate */
  "i'm taking a walk",                /* vitality */
  "dangerous",                        /* color spray */
  "we are the world",                 /* total recall */
  "moonlight shadow",                 /* infra */
  "nothing compares to you",          /* santuary */
  "better than the devil you know",   /* evil word */
  "toy soldiers",                     /* animate dead */
  "everything i do i do it for you",  /* heal */
  "a view to a kill",                 /* lethal fire */
  "spirits in the material world",    /* mass invis */
  "lifting shadows of a dream",       /* remove poison */
  "brightest flame burns quickest",   /* rally */
  "something takes a part of me",     /* warchant */
  "if i could change the world",      /* luck */
  "there is no heart that won't heal",/* rejuvenation */
  "hero of the day",                  /* aid */
  "heaven beside you",                /* protection from evil/good */
  "you've been thunderstruck",        /* thunderball */
  "ray of light",                     /* sphere */
  "\n"
};

int song_level[] =
{
  18,5,7,9,11,13,15,16,17,19,21,23,24,25,26,28,24,30,30,30,30,50,30,30,30,30
};

int song_mana[] =
{
  30,20,20,20,25,25,25,30,30,30,30,50,60,60,70,80,50,40,75,60,100,100,150,100,400,200
};

char *skip_spaces(char *string);
void do_song(CHAR *ch, char *arg, int cmd)
{
  char buf[MIL];
  int qend = 0;
  int song = 0;
  int mana = 0;
  int level = 0;
  int tmp = 0;
  CHAR *tmp_victim = NULL;
  CHAR *next_victim = NULL;
  OBJ *tmp_obj = NULL;
  OBJ *next_obj = NULL;

  if (IS_NPC(ch)) return;

  if (GET_CLASS(ch) != CLASS_BARD &&
      IS_MORTAL(ch))
  {
    send_to_char("Please leave singing songs to the bards.\n\r", ch);
    return;
  }

  if (IS_MORTAL(ch))
  {
    if (!ch->equipment[HOLD] ||
      ch->equipment[HOLD]->obj_flags.type_flag != ITEM_MUSICAL)
    {
      send_to_char("You need to hold a musical instrument to sing a song.\n\r", ch);
      return;
    }
  }

  arg = skip_spaces(arg);

  if (!(*arg))
  {
    send_to_char("What song?\n\r", ch);
    return;
  }

  for (qend = 1; *(arg + qend) && (*(arg + qend) != '\''); qend++)
  {
    *(arg + qend) = LOWER(*(arg + qend));
  }

  if (*(arg) != '\'' ||
    *(arg + qend) != '\'')
  {
    send_to_char("Songs must always be enclosed by the symbols: '\n\r", ch);
    return;
  }

  song = old_search_block(arg, 1, (qend - 1), songs, 0);

  if (!song ||
      !check_sc_song_access(ch, song))
  {
    send_to_char("You do not know of such a song.\n\r", ch);
    return;
  }

  if (GET_LEVEL(ch) < song_level[song - 1])
  {
    send_to_char("You can't quite remember the words to that song!\n\r", ch);
    return;
  }

  mana = song_mana[song - 1];

  if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, DOUBLE_MANA))
  {
    mana *= 2;
  }

  if (IS_MORTAL(ch))
  {
    if (GET_MANA(ch) < mana)
    {
      send_to_char("You can't summon enough energy to sing the song.\n\r", ch);
      return;
    }
  }
  else
  {
    sprintf(buf, "WIZINFO: %s sings %s.", GET_NAME(ch), arg);
    log_f("%s", buf);
    wizlog(buf, GET_LEVEL(ch) + 1, 5);
  }

  GET_MANA(ch) -= mana;

  send_to_char("You sing the song.\n\r", ch);

  if (IS_MORTAL(ch))
  {
    if (IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, NO_SONG))
    {
      send_to_char("The song has been absorbed by your surroundings.\n\r", ch);
      return;
    }
  }

  WAIT_STATE(ch, PULSE_VIOLENCE);

  switch (song) {
    case 1:
      act("$n sings 'I always feel like somebody's watching me...'", FALSE, ch, NULL, NULL, TO_ROOM);

      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim) {
        next_victim = tmp_victim->next_in_room;

        if (ch != tmp_victim) {
          spell_detect_invisibility(GET_LEVEL(ch), ch, tmp_victim, NULL);
        }
      }

      spell_detect_invisibility(GET_LEVEL(ch), ch, ch, NULL);
      break;

    case 2:
      act("$n sings 'In a letter, sealed with a kiss...'", FALSE, ch, NULL, NULL, TO_ROOM);

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

    case 3:
      act("$n sings 'Like a prayer...'", FALSE, ch, NULL, NULL, TO_ROOM);

      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
      {
        next_victim = tmp_victim->next_in_room;

        if (ch != tmp_victim)
        {
          spell_bless(GET_LEVEL(ch), ch, tmp_victim, NULL);
        }
      }

      spell_bless(GET_LEVEL(ch), ch, ch, NULL);
      break;

    case 4:
      act("$n sings 'One day I will fly away...'", FALSE, ch, NULL, NULL, TO_ROOM);

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

    case 5:
      act("$n sings 'We don't need another hero...'", FALSE, ch, NULL, NULL, TO_ROOM);

      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
      {
        next_victim = tmp_victim->next_in_room;

        if (ch != tmp_victim)
        {
          spell_cure_critic(GET_LEVEL(ch), ch, tmp_victim, NULL);
        }
      }

      spell_cure_critic(GET_LEVEL(ch), ch, ch, NULL);
      break;

    case 6:
      act("$n sings 'Smoke gets in your eyes...'", FALSE, ch, NULL, NULL, TO_ROOM);

      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
      {
        next_victim = tmp_victim->next_in_room;

        if (ch != tmp_victim)
        {
          spell_blindness(GET_LEVEL(ch), ch, tmp_victim, NULL);
        }
      }
      break;

    case 7:
      act("$n sings 'Now I save the best of me...'", FALSE, ch, NULL, NULL, TO_ROOM);

      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
      {
        next_victim = tmp_victim->next_in_room;

        if (ch != tmp_victim)
        {
          spell_satiate(GET_LEVEL(ch), ch, tmp_victim, NULL);
        }
      }

      spell_satiate(GET_LEVEL(ch), ch, ch, NULL);
      break;

    case 8:
      act("$n sings 'I'm taking a ride with my best friend...'", FALSE, ch, NULL, NULL, TO_ROOM);

      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
      {
        next_victim = tmp_victim->next_in_room;

        if (ch != tmp_victim)
        {
          spell_vitality(GET_LEVEL(ch), ch, tmp_victim, NULL);
        }
      }

      spell_vitality(GET_LEVEL(ch), ch, ch, NULL);
      break;

    case 9:
      act("$n sings 'Dangerous...'", FALSE, ch, NULL, NULL, TO_ROOM);

      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
      {
        next_victim = tmp_victim->next_in_room;

        if (ch != tmp_victim)
        {
          spell_colour_spray(GET_LEVEL(ch), ch, tmp_victim, NULL);
        }
      }
      break;

    case 10:
      act("$n sings 'We are the world, we are the children...'", FALSE, ch, NULL, NULL, TO_ROOM);

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

    case 11:
      act("$n sings 'Carried away by moonlight shadow...'", FALSE, ch, NULL, NULL, TO_ROOM);

      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
      {
        next_victim = tmp_victim->next_in_room;

        if (ch != tmp_victim)
        {
          spell_infravision(GET_LEVEL(ch), ch, tmp_victim, NULL);
        }
      }

      spell_infravision(GET_LEVEL(ch), ch, ch, NULL);
      break;

    case 12:
      act("$n sings 'It's been seven hours and fifteen days...'", FALSE, ch, NULL, NULL, TO_ROOM);

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

    case 13:
      act("$n sings 'Better the devil you know...'", FALSE, ch, NULL, NULL, TO_ROOM);

      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
      {
        next_victim = tmp_victim->next_in_room;

        if (ch != tmp_victim)
        {
          spell_dispel_good(GET_LEVEL(ch), ch, tmp_victim, NULL);
        }
      }
      break;

    case 14:
      act("$n sings 'Like toy soldiers...'", FALSE, ch, NULL, NULL, TO_ROOM);

      for (tmp_obj = world[CHAR_REAL_ROOM(ch)].contents; tmp_obj; tmp_obj = next_obj)
      {
        next_obj = tmp_obj->next_content;
        spell_animate_dead(GET_LEVEL(ch), ch, NULL, tmp_obj);
      }
      break;

    case 15:
      act("$n sings 'Everything I do, I do it for you...'", FALSE, ch, NULL, NULL, TO_ROOM);

      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
      {
        next_victim = tmp_victim->next_in_room;

        if (ch != tmp_victim)
        {
          spell_heal(GET_LEVEL(ch), ch, tmp_victim, NULL);
        }
      }

      spell_heal(GET_LEVEL(ch), ch, ch, NULL);
      break;

    case 16:
      act("$n sings 'Dance into the fire...'", FALSE, ch, NULL, NULL, TO_ROOM);

      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
      {
        next_victim = tmp_victim->next_in_room;

        if (ch != tmp_victim)
        {
          spell_lethal_fire(GET_LEVEL(ch), ch, tmp_victim, NULL);
        }
      }
      break;

    case 17:
      act("$n sings 'We are spirits in the material world...'", FALSE, ch, NULL, NULL, TO_ROOM);
      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
      {
        next_victim = tmp_victim->next_in_room;

        if (ch != tmp_victim)
        {
          spell_invisibility(GET_LEVEL(ch), ch, tmp_victim, NULL);
        }
      }

      spell_invisibility(GET_LEVEL(ch), ch, ch, NULL);
      break;

    case 18:
      act("$n sings 'Lifting shadows, off a dream...'", FALSE, ch, NULL, NULL, TO_ROOM);

      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
      {
        next_victim = tmp_victim->next_in_room;

        if (ch != tmp_victim)
        {
          spell_remove_poison(GET_LEVEL(ch), ch, tmp_victim, NULL);
        }
      }

      spell_remove_poison(GET_LEVEL(ch), ch, ch, NULL);
      break;

    case 19:
      act("$n sings 'The brightest flame burns quickest...'", FALSE, ch, NULL, NULL, TO_ROOM);

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

    case 20:
      act("$n sings 'Something takes a part of me...'", FALSE, ch, NULL, NULL, TO_ROOM);

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

    case 21:
      act("$n sings 'If I could change the world...'", FALSE, ch, NULL, NULL, TO_ROOM);

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

    case 22:
      act("$n sings 'There is no heart that won't heal...'", FALSE, ch, NULL, NULL, TO_ROOM);

      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
      {
        next_victim = tmp_victim->next_in_room;

        if (ch != tmp_victim)
        {
          spell_rejuvenation(GET_LEVEL(ch), ch, tmp_victim, NULL);
        }
      }

      spell_rejuvenation(GET_LEVEL(ch), ch, ch, NULL);
      break;

    case 23:
      act("$n sings 'Hero of the day...'", FALSE, ch, NULL, NULL, TO_ROOM);

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

    case 24:
      act("$n sings 'Heaven beside you...'", FALSE, ch, NULL, NULL, TO_ROOM);

      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
      {
        next_victim = tmp_victim->next_in_room;

        if (ch != tmp_victim)
        {
          if (IS_EVIL(ch))
          {
            spell_protection_from_good(GET_LEVEL(ch), ch, tmp_victim, NULL);
          }
          else if (IS_GOOD(ch))
          {
            spell_protection_from_evil(GET_LEVEL(ch), ch, tmp_victim, NULL);
          }
        }
      }

      if (IS_EVIL(ch))
      {
        spell_protection_from_good(GET_LEVEL(ch), ch, ch, NULL);
      }
      else if (IS_GOOD(ch))
      {
        spell_protection_from_evil(GET_LEVEL(ch), ch, ch, NULL);
      }
      break;

    case 25:
      act("$n sings 'You've been... Thunderstruck!'", FALSE, ch, NULL, NULL, TO_ROOM);

      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
      {
        next_victim = tmp_victim->next_in_room;

        if (ch != tmp_victim)
        {
          spell_thunderball(GET_LEVEL(ch), ch, tmp_victim, NULL);
        }
      }
      break;

    case 26:
      act("$n sings 'Quicker than a ray of light...'", FALSE, ch, NULL, NULL, TO_ROOM);

      for (tmp = 0, tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp++, tmp_victim = tmp_victim->next_in_room);

      tmp = MAX(MIN(tmp * 30, 200), 50);
      level = GET_LEVEL(ch);

      GET_MANA(ch) += (200 - tmp);

      for (tmp_victim = world[CHAR_REAL_ROOM(ch)].people; tmp_victim; tmp_victim = next_victim)
      {
        next_victim = tmp_victim->next_in_room;

        if (ch != tmp_victim)
        {
          spell_sphere(level, ch, tmp_victim, NULL);
        }
      }

      spell_sphere(level, ch, ch, NULL);
      break;

    default:
      act("$n hums the words of a song $e doesn't know.", FALSE, ch, NULL, NULL, TO_ROOM);
      act("You hum the words of a song you don't know.", FALSE, ch, NULL, NULL, TO_CHAR);
      break;
  }
}

void clone_char(struct char_data *ch)
{
  /* buggy - crashes the game */
  extern struct mob_proto *mob_proto_table;
  struct char_data *clone;
  struct affected_type_5 *af;
  int i;

  CREATE(clone, struct char_data, 1);


  clear_char(clone);       /* Clear EVERYTHING! (ASSUMES CORRECT) */

  clone->player    = ch->player;
  clone->abilities = ch->abilities;

  for (i=0; i<5; i++)
    clone->specials.apply_saving_throw[i] = ch->specials.apply_saving_throw[i];

  for (af=ch->affected; af; af = af->next)
    affect_to_char(clone, af);

  for (i=0; i<3; i++)
    GET_COND(clone,i) = GET_COND(ch, i);

  clone->points = ch->points;

  /*  for (i=0; i<MAX_SKILLS; i++)
  clone->skills[i] = ch->skills[i]; */

  clone->skills = 0;

  clone->specials = ch->specials;
  clone->specials.fighting = 0;

  clone->player.name = str_dup(GET_NAME(ch));

  clone->player.short_descr =  str_dup(MOB_SHORT(ch));

  clone->player.long_descr = str_dup(MOB_LONG(ch));

  clone->player.description = 0;
  /* REMEMBER EXTRA DESCRIPTIONS */

  GET_TITLE(clone) = str_dup(GET_TITLE(ch));

  clone->nr = ch->nr;

  if (IS_NPC(clone))
    mob_proto_table[clone->nr].number++;
  else { /* Make PC's into NPC's */
    clone->nr = -1;
    SET_BIT(clone->specials.act, ACT_ISNPC);
  }

  clone->desc = 0;
  clone->followers = 0;
  clone->master = 0;

  clone->next = character_list;
  character_list = clone;

  char_to_room(clone, CHAR_REAL_ROOM(ch));
}

void say_spell( struct char_data *ch, int si )
{
  char buf[MAX_STRING_LENGTH], splwd[MAX_BUF_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  int j, offs;
  struct char_data *temp_char;


  struct syllable {
    char org[10];
    char new[10];
  };

  struct syllable syls[] = {
    { " ", " " },
    { "ar", "abra"   },
    { "au", "kada"    },
    { "bless", "fido" },
    { "blind", "nose" },
    { "bur", "mosa" },
    { "cu", "judi" },
    { "de", "oculo"},
    { "en", "unso" },
    { "id", "dhfj" },
    { "light", "dies" },
    { "lo", "hi" },
    { "mor", "zak" },
    { "move", "sido" },
    { "ness", "lacri" },
    { "ning", "illa" },
    { "per", "duda" },
    { "ra", "gru"   },
    { "re", "candus" },
    { "son", "sabru" },
    { "tect", "infra" },
    { "tri", "cula" },
    { "ven", "nofo" },
    {"a", "a"},{"b","b"},{"c","q"},{"d","e"},{"e","z"},{"f","y"},{"g","o"},
    {"h", "p"},{"i","u"},{"j","y"},{"k","t"},{"l","r"},{"m","w"},{"n","i"},
    {"o", "a"},{"p","s"},{"q","d"},{"r","f"},{"s","g"},{"t","h"},{"u","j"},
    {"v", "z"},{"w","x"},{"x","n"},{"y","l"},{"z","k"}, {"",""}
  };



  strcpy(buf, "");
  strcpy(splwd, spells[si-1]);

  offs = 0;

  while(*(splwd+offs)) {
    for(j=0; *(syls[j].org); j++)
      if (strncmp(syls[j].org, splwd+offs, strlen(syls[j].org))==0) {
     strcat(buf, syls[j].new);
     if (strlen(syls[j].org))
       offs+=strlen(syls[j].org);
     else
       ++offs;
      }
  }

  if (GET_CLASS(ch) != CLASS_NINJA)
    { sprintf(buf2,"$n utters the words, '%s'", buf);
      sprintf(buf, "$n utters the words, '%s'", spells[si-1]);
    }
  else
    { sprintf(buf2,"$n makes a mystic hand position and utters the words, '%s'", buf);
      sprintf(buf,"$n makes a mystic hand position and utters the words, '%s'", spells[si-1]);
    }

  for(temp_char = world[CHAR_REAL_ROOM(ch)].people;
      temp_char;
      temp_char = temp_char->next_in_room)
    if(temp_char != ch) {
      if (GET_CLASS(ch) == GET_CLASS(temp_char))
     act(buf, FALSE, ch, 0, temp_char, TO_VICT);
      else
     act(buf2, FALSE, ch, 0, temp_char, TO_VICT);

    }

}


const char save[11][5][2] = {
  /* Para     Rods     Petri    Breath   Spells */
  { {16, 4}, {13, 1}, {15, 3}, {17, 4}, {14, 1} },    /* Mage */
  { {11, 1}, {16, 3}, {14, 2}, {18, 5}, {17, 3} },    /* Cleric */
  { {15, 6}, {16, 2}, {14, 3}, {18, 4}, {17, 4} },    /* Thief */
  { {16, 1}, {18, 3}, {17, 1}, {20, 2}, {19, 3} },    /* Warrior */
  { {15, 6}, {16, 2}, {14, 3}, {18, 5}, {17, 1} },    /* Ninja */
  { {15, 6}, {16, 2}, {14, 3}, {18, 5}, {17, 1} },    /* Nomad */
  { {15, 6}, {16, 2}, {14, 3}, {18, 5}, {17, 1} },    /* Paladin */
  { {15, 6}, {16, 2}, {14, 3}, {18, 5}, {17, 1} },    /* Antipaladin */
  { {15, 6}, {16, 2}, {14, 3}, {18, 5}, {17, 1} },    /* Avatar */
  { {15, 6}, {16, 2}, {14, 3}, {18, 5}, {17, 1} },    /* Bard */
  { {15, 6}, {16, 2}, {14, 3}, {18, 5}, {17, 1} }     /* Commando */
};

bool saves_spell(struct char_data *ch, sh_int type, int level)
{
  int throw, class;
  double chlevel, diff, total;
  /* For PCs Negative apply_saving_throw makes saving throw better! */
  /* For mobs, the saving throw is the mobs level - the higher the better */

  throw = ch->specials.apply_saving_throw[type];

  if (!IS_NPC(ch)) {
    if (GET_LEVEL(ch) >= LEVEL_IMM)
      return(TRUE);

    class = GET_CLASS(ch)-1;
    chlevel = ((double)GET_LEVEL(ch)) / ((double)LEVEL_IMM-1);
    diff = chlevel*save[class][type][0]-save[class][type][1];
    total = save[class][type][0] - diff;
    throw += total;
  }
  else {
    throw=level-throw;
  }
  return(MAX(2,throw) < number(1,20));
}

char *skip_spaces(char *string)
{
  while (*string && *string == ' ') string++;

  return string;
}

int USE_MANA(CHAR *ch, int sn)
{
  int mana = 0;

  switch (sn)
  {
    case SPELL_METEOR:
      /* Shouldn't be subject to caster level calculation. */
      mana = spell_info[sn].min_usesmana;
      break;
    case SPELL_VAMPIRIC:
    case SPELL_EXISTENCE:
      mana = 40 + (GET_LEVEL(ch) * 2);
      break;
    case SPELL_FURY:
      mana = (GET_CLASS(ch) == CLASS_PALADIN && GET_LEVEL(ch) == 50 && !CHAOSMODE) ? 120 : spell_info[sn].min_usesmana;
      break;
    case SPELL_DEGENERATE:
      mana = 0;
      break;
    default:
      mana = MAX(spell_info[sn].min_usesmana, 100 / (2 + GET_LEVEL(ch) - SPELL_LEVEL(ch, sn)));
      break;
  }

  return mana;
}

/* Assumes that *argument does start with first letter of chopped string */
void do_cast(struct char_data *ch, char *argument, int cmd) {
  struct obj_data *tar_obj;
  struct char_data *tar_char;
  char name[MAX_STRING_LENGTH],buf[MAX_INPUT_LENGTH];
  int qend, spl, i,percent,con_amt,mult,conc,mana_cost;
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

  /* Look for the spell */
  spl = old_search_block(argument, 1, qend-1,spells, 0);

  if (!spl) {
    send_to_char("Your lips do not move, no magic appears.\n\r",ch);
    return;
  }

  if((spl<=0) || (spl>MAX_SPL_LIST) || !(spell_info[spl].spell_pointer) ||
     !check_sc_access(ch,spl) ) {
    switch (number(1,5)) {
      case 1: send_to_char("Bylle Grylle Grop Gryf???\n\r", ch); break;
      case 2: send_to_char("Olle Bolle Snop Snyf?\n\r",ch); break;
      case 3: send_to_char("Olle Grylle Bolle Bylle?!?\n\r",ch); break;
      case 4: send_to_char("Gryffe Olle Gnyffe Snop???\n\r",ch); break;
      default: send_to_char("Bolle Snylle Gryf Bylle?!!?\n\r",ch); break;
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
           send_to_char("Impossible! You can't concentrate enough!.\n\r", ch);
           break;
      default:
           send_to_char("It seems like you're in a pretty bad shape!\n\r",ch);
           break;
      }
    return;
  }

  if(spell_info[spl].minimum_position>POSITION_FIGHTING &&
     ch->specials.fighting) {
     send_to_char("Impossible! You can't concentrate enought!.\n\r",ch);
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

    } else { /* Nothing was given as argument */
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

  if(affected_by_spell(ch, SPELL_CLARITY)) mult=mult*9/10;

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

  con_amt=0;
  if(GET_CLASS(ch)==CLASS_PALADIN || GET_CLASS(ch)==CLASS_NINJA ||
     GET_CLASS(ch)==CLASS_COMMANDO || GET_CLASS(ch)==CLASS_ANTI_PALADIN) {
     if(enchanted_by(ch,"The title of Shogun") ||
     enchanted_by(ch,"The rank of Commander") ||
     enchanted_by(ch,"The title of Dark Lord/Lady") ||
     enchanted_by(ch,"The title of Lord/Lady")) con_amt+=50;
  }

  if(affected_by_spell(ch, SPELL_FOCUS)) con_amt+=100;

  percent=number(1,1001);
  conc=1;
  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,HALF_CONC)) conc=2;
  if(IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,QRTR_CONC)) conc=4;

  con_amt+=(int)(int_app[GET_INT(ch)].conc+wis_app[GET_WIS(ch)].conc);
  con_amt+=(int)ch->skills[spl].learned*10;
  con_amt/=conc;
  if(con_amt>100 && affected_by_spell(ch, SPELL_CLARITY)) {
    con_amt=1002;
  }
  if (percent > con_amt) { /* 100.1% is failure */
    if(castlog) {
      fl=fopen("castlog","a+");
      fprintf(fl,"%s,%d,%d,%d,%d,0\n\r",GET_NAME(ch),percent,con_amt,GET_WIS(ch),GET_INT(ch));
      fclose(fl);
    }
    send_to_char("You lost your concentration!\n\r", ch);
    if(GET_LEVEL(ch) < LEVEL_IMM) GET_MANA(ch) -= ((USE_MANA(ch, spl))>>1)*mult/10;
    else GET_MANA(ch) -= ((spell_info[spl].min_usesmana)>>1)*mult/10;
    return;
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

  if(GET_LEVEL(ch) < LEVEL_IMM) {
    mana_cost=(USE_MANA(ch, spl))*mult/10;
    switch(GET_LEVEL(ch))
    {
	  case 41:
		if(chance(10)) mana_cost = mana_cost*99/100;
		break;
	  case 42:
		if(chance(11)) mana_cost = mana_cost*98/100;
		break;
	  case 43:
		if(chance(12)) mana_cost = mana_cost*97/100;
		break;
	  case 44:
		if(chance(13)) mana_cost = mana_cost*96/100;
		break;
	  case 45:
		if(chance(15)) mana_cost = mana_cost*95/100;
		break;
	  case 46:
		if(chance(17)) mana_cost = mana_cost*94/100;
		break;
	  case 47:
		if(chance(19)) mana_cost = mana_cost*93/100;
		break;
	  case 48:
		if(chance(21)) mana_cost = mana_cost*92/100;
		break;
	  case 49:
		if(chance(23)) mana_cost = mana_cost*91/100;
		break;
	  case 50:
		if(chance(25)) mana_cost = mana_cost*90/100;
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

  spl = old_search_block(argument, 1, qend-1,spells, 0);

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

void assign_spell_pointers(void)
{
  int i = 0;

  for (i = 0; i < MAX_SPL_LIST + 1; i++)
    spell_info[i].spell_pointer = 0;

  /*                                 Minimum Class Level Required
   * Spell #, BT, Minimum Position,  MU, CL, NI, PA, AP, AV, BA, CO, MANA, Target Types,                                                                 Cast Function
   */
  SPELLO(  1, 30, POSITION_STANDING,  5,  1, 51,  2, 51,  1,  5, 51,    5, TAR_CHAR_ROOM,                                                                cast_armor);
  SPELLO(  2, 30, POSITION_FIGHTING,  8, 51, 51, 51,  7,  7, 51, 51,   25, TAR_SELF_ONLY,                                                                cast_teleport);
  SPELLO(  3, 30, POSITION_STANDING, 51,  5, 51,  6, 51,  3,  7, 51,    5, TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_CHAR_ROOM,                                  cast_bless);
  SPELLO(  4, 30, POSITION_STANDING,  8,  6,  6, 51,  4,  4, 13, 51,   15, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_blindness);
  SPELLO(  5, 30, POSITION_FIGHTING,  5, 51, 51, 51, 51,  1, 51,  3,   17, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_burning_hands);
  SPELLO(  6, 30, POSITION_FIGHTING, 51, 12, 51, 51, 11, 51, 51, 12,   20, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_call_lightning);
  SPELLO(  7, 30, POSITION_STANDING, 14, 51, 51, 51, 51, 12, 18, 51,   50, TAR_CHAR_ROOM | TAR_SELF_NONO,                                                cast_charm_person);
  SPELLO(  8, 30, POSITION_FIGHTING,  3, 51, 51, 51,  3, 51, 51,  2,   15, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_chill_touch);
  SPELLO(  9, 30, POSITION_STANDING, 27, 51, 51, 51, 51, 27, 51, 51,  100, TAR_OBJ_INV,                                                                  cast_clone);
  SPELLO( 10, 30, POSITION_FIGHTING, 11, 51, 51, 51, 51, 51, 17, 10,   23, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_colour_spray);
  SPELLO( 11, 30, POSITION_STANDING, 10, 13, 11, 51, 51, 51, 51, 51,   25, TAR_IGNORE,                                                                   cast_control_weather);
  SPELLO( 12, 30, POSITION_STANDING, 51,  3, 51,  3, 51, 51, 51, 51,    5, TAR_IGNORE,                                                                   cast_create_food);
  SPELLO( 13, 30, POSITION_STANDING, 51,  2, 51,  4, 51, 51, 51, 51,    5, TAR_OBJ_INV | TAR_OBJ_EQUIP,                                                  cast_create_water);
  SPELLO( 14, 30, POSITION_STANDING, 51,  4,  9,  7, 51,  5, 51, 51,   15, TAR_CHAR_ROOM,                                                                cast_cure_blind);
  SPELLO( 15, 30, POSITION_FIGHTING, 51,  9, 51, 10, 51,  7, 11, 51,   25, TAR_CHAR_ROOM,                                                                cast_cure_critic);
  SPELLO( 16, 30, POSITION_FIGHTING, 51,  1,  4,  1,  6,  1, 51, 51,   15, TAR_CHAR_ROOM,                                                                cast_cure_light);
  SPELLO( 17, 30, POSITION_STANDING, 12, 51, 51, 51,  8, 51, 51, 51,   20, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_OBJ_ROOM | TAR_OBJ_INV,                  cast_curse);
  SPELLO( 18, 30, POSITION_STANDING,  4,  4, 51,  4,  5,  3, 51, 51,   25, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_detect_alignment);
  SPELLO( 19, 30, POSITION_STANDING,  2,  5,  1,  5, 51,  3,  3, 51,   25, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_detect_invisibility);
  SPELLO( 20, 30, POSITION_STANDING,  2,  3, 51, 51, 51, 51, 51, 51,    5, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_detect_magic);
  SPELLO( 21, 30, POSITION_STANDING,  4,  2, 51, 51, 51, 51, 51, 51,    5, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP,                                  cast_detect_poison);
  SPELLO( 22, 30, POSITION_FIGHTING, 51, 10, 51, 19, 51, 51, 51, 51,   25, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_dispel_evil);
  SPELLO( 23, 30, POSITION_FIGHTING, 51,  7, 51, 51, 51, 51, 51,  6,   15, TAR_IGNORE,                                                                   cast_earthquake);
  SPELLO( 24, 30, POSITION_STANDING, 12, 51, 51, 51, 51, 18, 51, 51,   50, TAR_OBJ_INV | TAR_OBJ_EQUIP,                                                  cast_enchant_weapon);
  SPELLO( 25, 30, POSITION_FIGHTING, 13, 51, 51, 51, 15, 14, 51, 51,   35, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_energy_drain);
  SPELLO( 26, 30, POSITION_FIGHTING, 15, 51, 51, 51, 20, 15, 51, 15,   25, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_fireball);
  SPELLO( 27, 30, POSITION_FIGHTING, 51, 15, 51, 51, 51, 51, 51, 14,   35, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_harm);
  SPELLO( 28, 30, POSITION_FIGHTING, 51, 14, 29, 21, 51, 15, 26, 51,   50, TAR_CHAR_ROOM,                                                                cast_heal);
  SPELLO( 29, 30, POSITION_STANDING,  4, 51,  7, 51,  9,  6, 8,  51,   25, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM | TAR_OBJ_EQUIP,                   cast_invisibility);
  SPELLO( 30, 30, POSITION_FIGHTING,  9, 51, 51, 51, 12, 51, 51,  7,   20, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_lightning_bolt);
  SPELLO( 31, 30, POSITION_STANDING,  6, 10, 14, 17, 51,  8, 10, 51,   20, TAR_OBJ_WORLD,                                                                cast_locate_object);
  SPELLO( 32, 30, POSITION_FIGHTING,  1, 51, 51, 51,  1, 51, 51,  1,    5, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_magic_missile);
  SPELLO( 33, 30, POSITION_STANDING, 51,  8, 12, 51,  6, 51, 51, 51,   10, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO | TAR_OBJ_INV | TAR_OBJ_EQUIP, cast_poison);
  SPELLO( 34, 30, POSITION_STANDING, 51,  6, 51, 24, 51, 20, 51, 51,   30, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_protection_from_evil);
  SPELLO( 35, 30, POSITION_STANDING, 13, 12, 51, 51, 51, 21, 51, 51,   25, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_EQUIP | TAR_OBJ_ROOM,                   cast_remove_curse);
  SPELLO( 36, 30, POSITION_STANDING, 51, 13, 51, 20, 51, 12, 23, 51,   50, TAR_CHAR_ROOM,                                                                cast_sanctuary);
  SPELLO( 37, 30, POSITION_FIGHTING,  7, 51, 51, 51, 51, 51, 51,  5,   19, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_shocking_grasp);
  SPELLO( 38, 30, POSITION_STANDING, 14, 51, 51, 51, 19, 11, 17, 51,   25, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_sleep);
  SPELLO( 39, 30, POSITION_STANDING,  7, 51, 51, 51, 51, 51, 51, 51,   10, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_strength);
  SPELLO( 40, 30, POSITION_STANDING, 51,  8, 51, 18, 51, 12, 51, 51,   50, TAR_CHAR_WORLD,                                                               cast_summon);
  SPELLO( 41, 30, POSITION_STANDING,  1, 51, 51, 51, 51, 51, 51, 51,    5, TAR_CHAR_ROOM | TAR_OBJ_ROOM | TAR_SELF_NONO,                                 cast_ventriloquate);
  SPELLO( 42, 30, POSITION_STANDING, 13, 11, 19, 51, 51,  6, 19, 19,   25, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_word_of_recall);
  SPELLO( 43, 30, POSITION_STANDING, 51,  9, 12, 14, 51,  5, 51, 51,   25, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM,                                   cast_remove_poison);
  SPELLO( 44, 30, POSITION_STANDING, 51,  7, 10,  6, 51,  8, 51, 51,   20, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_sense_life);
  SPELLO( 71, 30, POSITION_STANDING,  7, 51, 51, 51, 51, 10,  9, 51,   25, TAR_OBJ_INV,                                                                  cast_identify);
  SPELLO( 72, 30, POSITION_FIGHTING, 51,  6, 16,  5, 51,  4, 51, 51,   18, TAR_CHAR_ROOM,                                                                cast_cure_serious);
  SPELLO( 73, 30, POSITION_STANDING,  5, 51,  5, 51, 11,  4, 21, 51,   20, TAR_CHAR_ROOM,                                                                cast_infravision);
  SPELLO( 74, 30, POSITION_FIGHTING, 28, 51, 51, 51, 51, 51, 51, 51,  100, TAR_CHAR_ROOM,                                                                cast_regeneration);
  SPELLO( 75, 30, POSITION_FIGHTING,  8, 16, 51, 14, 51, 10,  7, 51,   25, TAR_CHAR_ROOM,                                                                cast_vitality);
  SPELLO( 76, 30, POSITION_FIGHTING, 51,  4, 51, 51, 51, 51, 51, 51,   23, TAR_IGNORE,                                                                   cast_cure_light_spray);
  SPELLO( 77, 30, POSITION_FIGHTING, 51, 10, 51, 51, 51, 51, 51, 51,   27, TAR_IGNORE,                                                                   cast_cure_serious_spray);
  SPELLO( 78, 30, POSITION_FIGHTING, 51, 13, 51, 51, 51, 16, 51, 51,   38, TAR_IGNORE,                                                                   cast_cure_critic_spray);
  SPELLO( 79, 30, POSITION_FIGHTING, 51, 22, 51, 51, 51, 27, 51, 51,  100, TAR_IGNORE,                                                                   cast_heal_spray);
  SPELLO( 80, 30, POSITION_FIGHTING, 23, 51, 51, 51, 51, 17, 51, 24,   80, TAR_IGNORE,                                                                   cast_death_spray);
  SPELLO( 81, 30, POSITION_FIGHTING, 51, 23, 51, 25, 51, 51, 51, 51,   80, TAR_IGNORE,                                                                   cast_holy_word);
  SPELLO( 82, 30, POSITION_FIGHTING, 19, 51, 51, 51, 23, 18, 51, 20,   30, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_iceball);
  SPELLO( 83, 30, POSITION_STANDING, 51, 18, 51, 51, 51, 13, 51, 51,   30, TAR_IGNORE,                                                                   cast_total_recall);
  SPELLO( 84, 30, POSITION_STANDING, 17, 20, 51, 51, 51, 16, 51, 51,   30, TAR_OBJ_INV,                                                                  cast_recharge);
  SPELLO( 85, 30, POSITION_FIGHTING, 51, 25, 51, 51, 51, 22, 51, 51,  100, TAR_CHAR_ROOM,                                                                cast_miracle);
  SPELLO( 86, 30, POSITION_STANDING, 10, 51, 51, 51, 51,  7, 51, 51,   25, TAR_CHAR_ROOM,                                                                cast_fly);
  SPELLO( 87, 30, POSITION_FIGHTING, 18, 18, 51, 51, 51, 51, 51, 51,   50, TAR_CHAR_ROOM,                                                                cast_mana_transfer);
  SPELLO( 88, 30, POSITION_STANDING, 51, 19, 51, 51, 51, 17, 51, 51,   30, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_holy_bless);
  SPELLO( 89, 30, POSITION_STANDING, 51, 20, 51, 51, 51, 18, 51, 51,   30, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_evil_bless);
  SPELLO( 90, 30, POSITION_STANDING, 51, 15, 51, 51, 51, 51, 15, 51,   10, TAR_CHAR_ROOM,                                                                cast_satiate);
  SPELLO( 91, 30, POSITION_STANDING, 22, 24, 51, 51, 51, 20, 25, 51,   50, TAR_OBJ_ROOM,                                                                 cast_animate_dead);
  SPELLO( 92, 30, POSITION_STANDING, 51, 30, 51, 51, 51, 30, 51, 51,  200, TAR_IGNORE,                                                                   cast_great_miracle);
  SPELLO( 93, 30, POSITION_FIGHTING, 51, 11, 51, 10, 51,  6, 51, 51,   25, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_flamestrike);
  SPELLO( 94, 30, POSITION_FIGHTING, 20, 51, 51, 51, 25, 10, 51, 51,   15, TAR_OBJ_ROOM,                                                                 cast_spirit_levy);
  SPELLO( 95, 30, POSITION_FIGHTING, 24, 51, 51, 51, 51, 20, 28, 27,   50, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_lethal_fire);
  SPELLO( 96, 30, POSITION_STANDING, 21, 51, 24, 26, 51, 19, 18, 51,   20, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_hold);
  SPELLO( 97, 30, POSITION_STANDING, 18, 51, 51, 51, 51, 17, 51, 51,   50, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_sphere);
  SPELLO( 98, 30, POSITION_STANDING, 25, 51, 51, 51, 29, 22, 51, 51,   30, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_imp_invisibility);
  SPELLO( 99, 30, POSITION_STANDING, 29, 28, 51, 51, 51, 25, 51, 51,   50, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_invulnerability);
  SPELLO(100, 30, POSITION_FIGHTING,  9, 51, 51, 51, 10,  9, 12, 51,   15, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_fear);
  SPELLO(101, 30, POSITION_FIGHTING, 11, 51, 51, 51, 14, 10, 15, 51,   15, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_forget);
  SPELLO(102, 30, POSITION_FIGHTING, 51, 51, 51, 21, 51, 23, 51, 51,   60, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_fury);
  SPELLO(103, 30, POSITION_STANDING, 51, 51,  3, 51, 51,  3, 51, 51,   10, TAR_CHAR_ROOM,                                                                cast_endure);
  SPELLO(104, 30, POSITION_FIGHTING, 51, 51, 16, 51, 17, 19, 51, 51,   20, TAR_IGNORE,                                                                   cast_blindness_dust);
  SPELLO(105, 30, POSITION_FIGHTING, 51, 51, 17, 51, 16, 19, 51, 51,   20, TAR_IGNORE,                                                                   cast_poison_smoke);
  SPELLO(106, 30, POSITION_FIGHTING, 51, 51, 51, 51, 28, 24, 51, 51,  130, TAR_IGNORE,                                                                   cast_hell_fire);
  SPELLO(107, 30, POSITION_STANDING, 51, 51, 22, 51, 27, 51, 51, 51,   40, TAR_CHAR_ROOM,                                                                cast_hypnotize);
  SPELLO(108, 30, POSITION_STANDING, 51, 51, 51, 51, 51, 51, 51, 51,   10, TAR_CHAR_ROOM,                                                                cast_recover_mana);
  SPELLO(109, 30, POSITION_FIGHTING, 28, 51, 51, 51, 51, 27, 51, 51,  200, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_thunderball);
  SPELLO(110, 30, POSITION_FIGHTING, 26, 51, 51, 51, 51, 25, 51, 29,  100, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_electric_shock);
  SPELLO(111, 30, POSITION_STANDING, 29, 51, 51, 51, 21, 28, 51, 51,   50, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_paralyze);
  SPELLO(112, 30, POSITION_STANDING, 51, 14, 51, 51, 51,  9, 51, 51,   35, TAR_CHAR_ROOM,                                                                cast_remove_paralysis);
  SPELLO(113, 30, POSITION_FIGHTING, 51, 11, 51, 51, 22, 51, 51, 51,   25, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_dispel_good);
  SPELLO(114, 30, POSITION_FIGHTING, 51, 23, 51, 51, 26, 51, 24, 51,   80, TAR_IGNORE,                                                                   cast_evil_word);
  SPELLO(115, 30, POSITION_STANDING, 10, 51, 51, 51, 51,  9, 51, 51,   20, TAR_OBJ_INV,                                                                  cast_reappear);
  SPELLO(116, 30, POSITION_STANDING,  6, 51, 51, 51, 51,  5, 51, 51,   30, TAR_CHAR_ROOM,                                                                cast_reveal);
  SPELLO(117, 30, POSITION_STANDING, 27, 51, 51, 51, 51, 16, 51, 51,   50, TAR_CHAR_WORLD,                                                               cast_relocation);
  SPELLO(118, 30, POSITION_STANDING, 11, 14, 51, 51, 51, 11, 13, 51,   20, TAR_CHAR_WORLD,                                                               cast_locate_character);
  SPELLO(119, 30, POSITION_FIGHTING, 51, 27, 51, 32, 51, 51, 51, 51,  100, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_super_harm);
  SPELLO(121, 30, POSITION_STANDING, 55, 55, 55, 55, 55, 26, 55, 55,   25, TAR_OBJ_INV,                                                                  cast_legend_lore);
  SPELLO(122, 30, POSITION_STANDING, 51, 51, 51, 51, 51, 51, 51, 51,   10, TAR_CHAR_ROOM,                                                                cast_great_mana);
  SPELLO(124, 30, POSITION_STANDING, 40, 57, 57, 57, 40, 40, 57, 57,   40, TAR_SELF_ONLY,                                                                cast_perceive);
  SPELLO(127, 30, POSITION_FIGHTING, 55, 40, 55, 55, 55, 26, 45, 55,  120, TAR_SELF_ONLY,                                                                cast_haste);
  SPELLO(128, 30, POSITION_FIGHTING, 26, 51, 51, 51, 51, 51, 51, 51,  100, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_power_word_kill);
  SPELLO(129, 30, POSITION_STANDING, 19, 17, 51, 51, 51, 51, 51, 51,   50, TAR_CHAR_ROOM | TAR_OBJ_INV | TAR_OBJ_ROOM,                                   cast_dispel_magic);
  SPELLO(130, 30, POSITION_FIGHTING, 30, 51, 51, 51, 51, 51, 51, 51,  200, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_conflagration);
  SPELLO(132, 30, POSITION_FIGHTING, 21, 51, 27, 51, 51, 51, 51, 51,   20, TAR_CHAR_ROOM | TAR_FIGHT_SELF,                                               cast_convergence);
  SPELLO(133, 30, POSITION_STANDING, 21, 51, 51, 51, 51, 51, 51, 51,  100, TAR_OBJ_INV,                                                                  cast_enchant_armour);
  SPELLO(134, 30, POSITION_FIGHTING, 27, 51, 51, 51, 51, 51, 51, 28,  150, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_OBJ_INV | TAR_OBJ_ROOM,                  cast_disintegrate);
  SPELLO(135, 30, POSITION_STANDING, 55, 55, 55, 55, 55, 55, 55, 55,   50, TAR_CHAR_ROOM | TAR_SELF_NONO,                                                cast_confusion);
  SPELLO(136, 30, POSITION_FIGHTING, 18, 51, 51, 51, 24, 51, 51, 32,   40, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_vampiric_touch);
  SPELLO(137, 30, POSITION_FIGHTING, 51, 16, 51, 51, 51, 51, 51, 51,   60, TAR_IGNORE,                                                                   cast_searing_orb);
  SPELLO(138, 30, POSITION_STANDING, 18, 16, 21, 51, 51, 51, 51, 51,   35, TAR_CHAR_WORLD,                                                               cast_clairvoyance);
  SPELLO(139, 30, POSITION_FIGHTING, 28, 51, 51, 51, 51, 27, 51, 51,  100, TAR_CHAR_ROOM,                                                                cast_firebreath);
  SPELLO(140, 30, POSITION_FIGHTING, 51, 51, 51, 15, 51, 51, 51, 51,  100, TAR_CHAR_ROOM,                                                                cast_layhands);
  SPELLO(141, 30, POSITION_STANDING, 51, 51, 51, 51, 51, 51, 51, 51,   10, TAR_CHAR_ROOM,                                                                cast_dispel_sanct);
  SPELLO(142, 30, POSITION_FIGHTING, 52, 52, 52, 52, 52, 52, 52, 52,   50, TAR_CHAR_WORLD,                                                               cast_disenchant);
  SPELLO(143, 30, POSITION_FIGHTING, 55, 55, 55, 55, 55, 55, 55, 55,  200, TAR_CHAR_ROOM | TAR_FIGHT_VICT | TAR_SELF_NONO,                               cast_petrify);
  SPELLO(145, 30, POSITION_STANDING, 51,  6, 51, 51, 18, 51, 51, 51,   30, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_protection_from_good);
  SPELLO(146, 30, POSITION_STANDING, 25, 51, 51, 51, 29, 22, 51, 51,   20, TAR_CHAR_ROOM | TAR_SELF_ONLY,                                                cast_remove_improved_invis);
  SPELLO(147, 30, POSITION_FIGHTING, 32, 51, 51, 51, 51, 51, 51, 34,   40, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_shroud_existence);
  SPELLO(149, 30, POSITION_FIGHTING, 50, 51, 51, 51, 51, 51, 51, 51,  100, TAR_SELF_ONLY,                                                                cast_quick);
  SPELLO(150, 30, POSITION_STANDING, 51, 50, 51, 51, 51, 51, 51, 51,  500, TAR_CHAR_ROOM,                                                                cast_divine_intervention);
  SPELLO(151, 30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 40, 57, 50,  100, TAR_SELF_ONLY,                                                                cast_rush);
  SPELLO(152, 30, POSITION_FIGHTING, 57, 57, 57, 57, 50, 41, 57, 57,  100, TAR_SELF_ONLY,                                                                cast_blood_lust);
  SPELLO(154, 30, POSITION_FIGHTING, 57, 57, 50, 57, 57, 42, 57, 57,  100, TAR_SELF_ONLY,                                                                cast_mystic_swiftness);
  SPELLO(254, 30, POSITION_STANDING, 57, 30, 57, 57, 57, 57, 57, 57,   50, TAR_SELF_ONLY,                                                                cast_clarity);
  SPELLO(253, 30, POSITION_STANDING, 57, 30, 57, 30, 57, 57, 57, 57,   50, TAR_SELF_ONLY,                                                                cast_might);
  SPELLO(252, 30, POSITION_STANDING, 30, 57, 57, 57, 57, 57, 57, 57,   50, TAR_SELF_ONLY,                                                                cast_blade_barrier);
  SPELLO(251, 30, POSITION_STANDING, 30, 57, 57, 57, 57, 57, 57, 57,   50, TAR_IGNORE,                                                                   cast_passdoor);
  SPELLO(250, 30, POSITION_FIGHTING, 30, 57, 57, 57, 57, 57, 57, 57,  100, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_distortion);
  SPELLO(249, 30, POSITION_STANDING, 57, 57, 57, 57, 57, 57, 57, 30,   50, TAR_SELF_ONLY,                                                                cast_ironskin);
  SPELLO(248, 30, POSITION_FIGHTING, 30, 57, 57, 57, 57, 57, 57, 30,   50, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_frostbolt);
  SPELLO(246, 30, POSITION_STANDING, 30, 30, 57, 57, 57, 57, 57, 57,   50, TAR_SELF_ONLY,                                                                cast_orb_protection);
  SPELLO(244, 30, POSITION_STANDING, 57, 30, 57, 57, 57, 57, 57, 57,  100, TAR_IGNORE,                                                                   cast_sanctify);
  SPELLO(241, 30, POSITION_FIGHTING, 30, 57, 57, 57, 57, 57, 57, 57,  250, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_wrath_ancients);
  SPELLO(239, 30, POSITION_FIGHTING, 57, 57, 57, 30, 57, 57, 57, 57,   30, TAR_SELF_ONLY,                                                                cast_righteousness);
  SPELLO(238, 30, POSITION_FIGHTING, 57, 57, 57, 57, 30, 57, 57, 57,   60, TAR_SELF_ONLY,                                                                cast_rage);
  SPELLO(236, 30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 57, 57, 30,  100, TAR_IGNORE,                                                                   cast_cloud_confusion);
  SPELLO(224, 30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 57, 57, 57,  100, TAR_CHAR_ROOM,                                                                cast_luck);
  SPELLO(218, 30, POSITION_FIGHTING, 30, 57, 57, 57, 57, 57, 57, 57,    5, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_meteor);
  SPELLO(217, 30, POSITION_FIGHTING, 57, 30, 57, 57, 57, 57, 57, 57,   80, TAR_IGNORE,                                                                   cast_wall_thorns);
  SPELLO(216, 30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 57, 50, 57,   80, TAR_CHAR_ROOM,                                                                cast_rejuvenation);
  SPELLO(212, 30, POSITION_FIGHTING, 57, 57, 30, 57, 57, 57, 57, 57,   70, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_divine_wind);
  SPELLO(211, 30, POSITION_FIGHTING, 57, 57, 57, 57, 30, 57, 57, 57,   50, TAR_SELF_ONLY,                                                                cast_blackmantle);
  SPELLO(210, 30, POSITION_STANDING, 57, 57, 57, 57, 30, 57, 57, 57,   25, TAR_OBJ_ROOM,                                                                 cast_dark_ritual);
  SPELLO(209, 30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 57, 57, 30,  150, TAR_IGNORE,                                                                   cast_rimefang);
  SPELLO(206, 30, POSITION_STANDING, 57, 57, 57, 57, 30, 57, 57, 57,   50, TAR_OBJ_ROOM,                                                                 cast_desecrate);
  SPELLO(203, 30, POSITION_FIGHTING, 57, 57, 57, 57, 30, 57, 57, 57,  100, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_demonic_thunder);
  SPELLO(196, 30, POSITION_STANDING, 30, 57, 57, 57, 57, 57, 57, 57,   40, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_engage);
  SPELLO(195, 30, POSITION_STANDING, 30, 57, 57, 57, 57, 57, 57, 57,  100, TAR_SELF_ONLY,                                                                cast_ethereal_nature);
  SPELLO(194, 30, POSITION_FIGHTING, 57, 30, 57, 57, 57, 57, 57, 57,   60, TAR_SELF_ONLY,                                                                cast_magic_armament);
  SPELLO(193, 30, POSITION_STANDING, 57, 30, 57, 57, 57, 57, 57, 57,    0, TAR_SELF_ONLY,                                                                cast_degenerate);
  SPELLO(192, 30, POSITION_STANDING, 57, 30, 57, 57, 57, 57, 57, 57,  100, TAR_CHAR_ROOM,                                                                cast_fortification);
  SPELLO(191, 30, POSITION_STANDING, 30, 57, 57, 57, 57, 57, 57, 57,  250, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_disrupt_sanct);
  SPELLO(190, 30, POSITION_FIGHTING, 57, 57, 57, 30, 57, 57, 57, 57,  200, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_wrath_of_god);
  SPELLO(189, 30, POSITION_STANDING, 57, 57, 57, 30, 57, 57, 57, 57,  100, TAR_CHAR_ROOM,                                                                cast_power_of_devotion);
  SPELLO(188, 30, POSITION_STANDING, 57, 57, 57, 30, 57, 57, 57, 57,   50, TAR_SELF_ONLY,                                                                cast_focus);
  SPELLO(187, 30, POSITION_FIGHTING, 57, 57, 57, 30, 57, 57, 57, 57,   50, TAR_CHAR_ROOM,                                                                cast_power_of_faith);
  SPELLO(182, 30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 57, 57, 30,  200, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_incendiary_cloud);
  SPELLO(181, 30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 57, 57, 30,  200, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_devastation);
  SPELLO(180, 30, POSITION_FIGHTING, 57, 57, 57, 57, 30, 57, 57, 57,  100, TAR_SELF_ONLY,                                                                cast_shadow_wraith);
  SPELLO(179, 30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 57, 57, 30,  150, TAR_IGNORE,                                                                   cast_tremor);
  SPELLO(177, 30, POSITION_FIGHTING, 57, 57, 30, 57, 57, 57, 57, 57,   35, TAR_CHAR_ROOM,                                                                cast_tranquility);
  SPELLO(175, 30, POSITION_FIGHTING, 57, 57, 30, 57, 57, 57, 57, 57,  100, TAR_SELF_ONLY,                                                                cast_blur);
  SPELLO(173, 30, POSITION_FIGHTING, 57, 57, 57, 57, 57, 57, 57, 30,   10, TAR_SELF_ONLY,                                                                cast_mana_heal);
  SPELLO(172, 30, POSITION_FIGHTING, 57, 57, 30, 57, 57, 57, 57, 57,   20, TAR_CHAR_ROOM | TAR_FIGHT_VICT,                                               cast_debilitate);
  SPELLO(165, 30, POSITION_FIGHTING, 51, 51, 30, 51, 51, 51, 51, 51,   85, TAR_IGNORE,                                                                   cast_wind_slash);
}
