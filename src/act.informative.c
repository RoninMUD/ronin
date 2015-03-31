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

#include "structs.h"
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

/* extern variables */

struct scoreboard_data
{
  char killer[80];
  char killed[80];
  char location[80];
  char time_txt[80];
};

extern struct scoreboard_data scores[101];
extern int number_of_kills;
extern char *subclass_name[];

extern struct clan_data clan_list[MAX_CLANS];
extern int CHAOSMODE;
extern int CHAOSDEATH;
extern int top_of_p_table;
extern int max_connects;
extern int total_connects;
extern struct str_app_type str_app[];
extern int movement_loss[];
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern int file_to_string (char *name, char *buf);
extern int signal_char(CHAR *ch, CHAR *signaler, int cmd, char *arg) ;
extern char credits[MSL];
extern char heroes[MSL];
extern char motd[MSL];
extern char newbiemotd[MSL];
extern char godmotd[MSL];
extern char info[MSL];
extern char wizlist[MSL];
extern char wizlist_inactive[MAX_STRING_LENGTH];
extern char *dirs[];
extern char *where[];
extern char *color_liquid[];
extern char *fullness[];
extern const struct title_type titles[11][58];
extern char *Color[];
extern char *BKColor[];
extern int exp_table[58];
extern char *pc_class_types[];
extern struct dex_app_type dex_app[];
extern char *spells[];

/* extern functions */

struct time_info_data age(struct char_data *ch);
void page_string(struct descriptor_data *d, char *str, int keep_internal);
void make_chaos_corpse(struct char_data *ch);
void create_wizlist (FILE *wizlist);
void create_inactive_wizlist (FILE *wizlist);
void give_prompt(struct descriptor_data *point);

/* intern functions */

void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode,
          bool show);
void look_in_room(CHAR *ch, int vnum);

void show_char_to_char(struct char_data *i, struct char_data *ch, int mode);

/* Procedures related to 'look' */

void argument_split_2(char *argument, char *first_arg, char *second_arg) {
  int look_at, begin = 0;

  /* Find first non blank */

  for ( ;*(argument + begin ) == ' ' ; begin++);

  /* Find length of first word */
  for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)

    /* Make all letters lower case, AND copy them to first_arg */

    *(first_arg + look_at) = LOWER(*(argument + begin + look_at));
  *(first_arg + look_at) = '\0';
  begin += look_at;

  /* Find first non blank */
  for ( ;*(argument + begin ) == ' ' ; begin++);

  /* Find length of second word */
  for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

    /* Make all letters lower case, AND copy them to second_arg */
    *(second_arg + look_at) = LOWER(*(argument + begin + look_at));
  *(second_arg + look_at)='\0';
  begin += look_at;
}

struct obj_data *get_object_in_equip_vis(struct char_data *ch,
           char *arg,
           struct obj_data *equipment[],
           int *j) {

  for ((*j) = 0; (*j) < MAX_WEAR ; (*j)++)
    if (equipment[(*j)])
      if (CAN_SEE_OBJ(ch,equipment[(*j)]))
  if (isname(arg, OBJ_NAME(equipment[(*j)])))
    return(equipment[(*j)]);

  return (0);
}

char *find_ex_description(char *word, struct extra_descr_data *list)
{
  struct extra_descr_data *i;

  for (i = list; i; i = i->next)
    if (isname(word,i->keyword))
      return(i->description);

  return(0);
}

void death_list(CHAR *ch);
void make_statue(struct char_data *ch)
{
  struct obj_data *statue, *o;
  struct obj_data *money;
  char buf[MSL];
  int i;

  char *str_dup(char *source);
  struct obj_data *create_money( int amount );

  ch->new.been_killed += 1;
  death_list(ch);
  CREATE(statue, struct obj_data, 1);
  clear_object(statue);

  statue->item_number = NOWHERE;
  statue->in_room = NOWHERE;
  if(!IS_NPC(ch)) {
    sprintf(buf,"statue %s",GET_NAME(ch));
    string_to_lower(buf);
    statue->name=str_dup(buf);
  }
  else
    statue->name=str_dup("statue");

  sprintf(buf, "Statue of %s is standing here.",
    (IS_NPC(ch) ? GET_SHORT(ch) : GET_NAME(ch)));
  statue->description = str_dup(buf);

  sprintf(buf, "Statue of %s",
    (IS_NPC(ch) ? GET_SHORT(ch) : GET_NAME(ch)));
  statue->short_description = str_dup(buf);

  statue->contains = ch->carrying;
  if(GET_GOLD(ch)>0)
    {
      money = create_money(GET_GOLD(ch));
      GET_GOLD(ch)=0;
      obj_to_obj(money,statue);
    }

  statue->obj_flags.type_flag = ITEM_CONTAINER;
  statue->obj_flags.wear_flags = ITEM_TAKE;
  statue->obj_flags.value[0] = 0; /* You can't store stuff in a statue */
  statue->obj_flags.value[2] = GET_LEVEL(ch);
  statue->obj_flags.value[3] = 1; /* statue identifyer */
  statue->obj_flags.weight = 10000;
  statue->obj_flags.cost_per_day = 100000;
  if (IS_NPC(ch)) {
    statue->obj_flags.cost=NPC_STATUE;
    statue->obj_flags.timer = MAX_NPC_STATUE_TIME;
  }
  else {
    statue->obj_flags.cost=PC_STATUE;
    statue->obj_flags.timer = MAX_PC_STATUE_TIME;
  }

  for (i=0; i<MAX_WEAR; i++)
    if (ch->equipment[i])
      obj_to_obj(unequip_char(ch, i), statue);

  ch->carrying = 0;

  statue->next = object_list;
  object_list = statue;

  for(o = statue->contains; o; o->in_obj = statue,
      o = o->next_content);
  object_list_new_owner(statue, 0);

  if(GET_LEVEL(ch)<10 && statue->obj_flags.cost==PC_STATUE &&
     !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,DEATH) &&
     !IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags,HAZARD) ) {
    send_to_char("\n\rYour statue is in the Midgaard Morgue 2 west from the Temple.\n\rWhen you reach level 10, your statue will be whereever you died.\n\r",ch);
    obj_to_room(statue,real_room(3088));
  }
  else {
    obj_to_room(statue, CHAR_REAL_ROOM(ch));
  }
  GET_HIT(ch)  = 1;
  GET_MANA(ch) = 1;
  remove_all_affects(ch);
}


void show_obj_to_char(struct obj_data *object, struct char_data *ch,
          int mode,int num)
{
  char buffer[4*MSL] = "\0";
  char bb[MSL]="\0";
  char buf1[MSL*4]="\0";
  char *Dcorpse[10]=
  {"The fresh ","The ","The slightly decayed ","The mildly decayed ",
   "The badly decayed ","The awfully decayed ","The rotting ",
   "The putrid rotten ","The barely recognizable ",
   "Flies and maggots surround this gruesomely decayed corpse."};
  char *Dstatue[10]=
  {"The new ","The ","The slightly chipped ","The chipped  ",
   "The badly chipped ","The awfully chipped ","The cracked and chipped ",
   "The awfully cracked ","The barely recognizable crumbling ",
   "A crumbling statue of unknown origin stands here."};
  char *decay_text[10]=
  {"like new","almost new","fairly new","slightly worn","worn",
   "fairly worn","very worn","slightly cracked","cracked",
   "about to crumble"};
  int val3,max_time;
  bool isstatue=FALSE;

  if ((mode == 0) && OBJ_DESCRIPTION(object))
    strcpy(buffer,OBJ_DESCRIPTION(object));
  else if (OBJ_SHORT(object) && ((mode == 1)
           || (mode == 2) || (mode==3) || (mode == 4))) {
      sprintf(buffer,"%s",OBJ_SHORT(object));
  }
  else if (mode == 5) {
    if (object->obj_flags.type_flag == ITEM_NOTE) {
      if (OBJ_ACTION(object)) {
  strcpy(buffer, "There is something written upon it:\n\r\n\r");
  strcat(buffer, OBJ_ACTION(object));
  page_string(ch->desc, buffer, 1);
      }
      else act("It's blank.", FALSE, ch,0,0,TO_CHAR);
      return;
    }
    else if (object->obj_flags.type_flag == ITEM_TICKET) {
      if(object->obj_flags.value[1]==object->obj_flags.value[2] || object->obj_flags.value[2]==0)
        sprintf(buffer,"This is ticket # %d from lot %d.\n\r",object->obj_flags.value[1],object->obj_flags.value[0]);
      else
        sprintf(buffer,"This is ticket # %d to %d from lot %d.\n\r",object->obj_flags.value[1],object->obj_flags.value[2],object->obj_flags.value[0]);
      page_string(ch->desc, buffer, 1);
      return;
    }
    else if(IS_SET(object->obj_flags.extra_flags2, ITEM_ALL_DECAY) || IS_SET(object->obj_flags.extra_flags2, ITEM_EQ_DECAY)) {
      max_time=obj_proto_table[object->item_number].obj_flags.timer;
      if(max_time)
        val3=9-10*object->obj_flags.timer/max_time;
      else val3=9;
      if(val3<0) val3=0;
      val3=MIN(val3,9);
      sprintf(buffer,"This item decays and seems %s.\n\r",decay_text[val3]);
    }
    else if((object->obj_flags.type_flag != ITEM_DRINKCON)) {
      strcpy(buffer,"You see nothing special..");
    }
    else /* ITEM_TYPE == ITEM_DRINKCON */
      {
  strcpy(buffer, "It looks like a drink container.");
      }
  }

  if (mode != 3) {
    if (mode!=5 && GET_ITEM_TYPE(object)==ITEM_CONTAINER && object->obj_flags.value[3]) {
      strncpy(buf1, buffer, sizeof(buf1));
      switch(object->obj_flags.cost) {
        case PC_CORPSE:
          max_time=MAX_PC_CORPSE_TIME; break;
        case NPC_CORPSE:
          max_time=MAX_NPC_CORPSE_TIME; break;
        case PC_STATUE:
          max_time=MAX_PC_STATUE_TIME;
          isstatue=TRUE; break;
        case NPC_STATUE:
          max_time=MAX_NPC_STATUE_TIME;
          isstatue=TRUE; break;
        case CHAOS_CORPSE:
          max_time=MAX_CHAOS_CORPSE_TIME; break;
        default:
          max_time=0;
      }

      if(max_time) val3=9-10*object->obj_flags.timer/max_time;
      else val3=1;
      if(val3<0) val3=0;
      val3=MIN(val3,9);

      if(isstatue)
        strncpy(buffer, Dstatue[val3], sizeof(buffer));
       else
        strncpy(buffer, Dcorpse[val3], sizeof(buffer));
     if(val3!=9)   strcat(buffer, LOW(buf1));

    }

    if (GET_LEVEL(ch)>LEVEL_WIZ && diff_obj_stats(object)) {
      strcat(buffer,"(`iQUESTED`q)");
    }
    if (IS_OBJ_STAT(object,ITEM_INVISIBLE)) {
      strcat(buffer,"(invisible)");
    }
    if (IS_OBJ_STAT(object,ITEM_MAGIC) && IS_AFFECTED(ch,AFF_DETECT_MAGIC)) {
      strcat(buffer,"(blue)");
    }
    if (IS_OBJ_STAT(object,ITEM_GLOW)) {
      strcat(buffer,"(glowing)");
    }
    if (IS_OBJ_STAT(object,ITEM_HUM)) {
      strcat(buffer,"(humming)");
    }
    if (IS_OBJ_STAT(object,ITEM_CLONE)) {
      strcat(buffer,"(cloned)");
    }
  }

  if(num)  {
    sprintf(bb,"[%d] ",num);
    strcat(bb,buffer);
    strcpy(buffer, bb);
  }
  strcat(buffer, "\n\r");
  page_string(ch->desc, buffer, 1);
}

void list_obj_to_char(struct obj_data *list,
          struct char_data *ch, int mode, bool show)
{
  struct obj_data *i;
  struct obj_data *back;
  bool c,f,found=FALSE;
  int x=0;
  c=TRUE;
  i=list;
  if(i)  {
    found=TRUE;
    while(c) {
      if (CAN_SEE_OBJ(ch,i)) {
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
          else    f=FALSE;
        }
        show_obj_to_char(back,ch,mode,x);
      }
      else   {
        i=i->next_content;
        if(!i) c=FALSE;
      }
    }
  }
  if ((! found) && (show)) send_to_char("Nothing\n\r", ch);
}

 /* Project Dagger */
void show_char_to_char(struct char_data *i, struct char_data *ch, int mode)
{
  char buffer[MSL];
  int j, found, percent;
  struct obj_data *tmp_obj;
  struct descriptor_data *d;
  struct tagline_data *tag;
  long ct;
  char *tmstr;
  AFF *aff = NULL;
  bool stop = FALSE;

  if (mode == 0)
  {
    if (i->new.wizinv > GET_LEVEL(ch)) { return; }

    if (((IS_AFFECTED(i, AFF_HIDE)) && GET_LEVEL(ch) < LEVEL_IMM && !affected_by_spell(ch, SPELL_PERCEIVE)) || !CAN_SEE(ch, i))
    {
      if (i->specials.riding)
      {
        if (GET_POS(i->specials.riding) == POSITION_FLYING)
        {
          act("$n is here being flown by someone.", TRUE, i->specials.riding, 0, ch, TO_VICT);
        }
        else
        {
          act("$n is here being ridden by someone.", TRUE, i->specials.riding, 0, ch, TO_VICT);
        }

        return;
      }

      if (IS_AFFECTED(ch, AFF_SENSE_LIFE) && (GET_LEVEL(ch) < LEVEL_IMM))
      {
        send_to_char("You sense a hidden life form.\n\r", ch);
      }

      return;
    }

    if (GET_MOUNT(i))
    {
      if (ch == GET_MOUNT(i))
      {
        if (GET_POS(i) == POSITION_FLYING)
        {
          act("$n is here being flown by you.", FALSE, i, 0, ch, TO_VICT);
        }
        else
        {
          act("$n is here being ridden by you.", FALSE, i, 0, ch, TO_VICT);
        }
      }

      return;
    }

    buffer[0] = 0;

    if (!IS_NPC(i) || !(MOB_LONG(i)) || (GET_POS(i) != i->specials.default_pos))
    {
      /* A player char or a mobile without long descr, or not in default pos.*/
      if (!IS_NPC(i))
      {
        strcpy(buffer, PERS(i, ch));
        strcat(buffer, " ");

        if (GET_TITLE(i))
        {
          strcat(buffer, GET_TITLE(i));
        }
      }
      else
      {
        if (ch->questmob && ch->questmob == i)
        {
          strcpy(buffer, "[TARGET] ");
          strcat(buffer, CAP(GET_SHORT(i)));
        }
        else
        {
          strcpy(buffer, CAP(GET_SHORT(i)));
        }
      }

      strcat(buffer, "`q");

      if (IS_AFFECTED(i, AFF_INVISIBLE))
      {
        strcat(buffer, " (invisible)");
      }

      if (IS_AFFECTED(i, AFF_HIDE))
      {
        strcat(buffer, " (hiding)");
      }

      if (affected_by_spell(i, SKILL_FADE))
      {
        strcat(buffer, " (faded)");
      }

      if (IS_SET(i->specials.pflag, PLR_KILL) && !IS_NPC(i))
      {
        strcat(buffer, " (killer)");
      }

      if (IS_SET(i->specials.pflag, PLR_THIEF) && !IS_NPC(i))
      {
        strcat(buffer, " (thief)");
      }

      if (IS_SET(i->specials.affected_by2, AFF_SEVERED))
      {
        strcat(buffer, "'s upper torso is here... twitching.");
      }
      else if (i->specials.death_timer == 2)
      {
        strcat(buffer, " lies here... near death.");
      }
      else
      {
        switch (GET_POS(i))
        {
          case POSITION_STUNNED:
            strcat(buffer, " is lying here, stunned.");
            break;

          case POSITION_INCAP:
            strcat(buffer, " is lying here, incapacitated.");
            break;

          case POSITION_MORTALLYW:
            strcat(buffer, " is lying here, mortally wounded.");
            break;

          case POSITION_DEAD:
            strcat(buffer, " is lying here, dead.");
            break;

          case POSITION_STANDING:
            strcat(buffer, " is standing here.");
            break;

          case POSITION_SITTING:
            strcat(buffer, " is sitting here.");
            break;

          case POSITION_RESTING:
            strcat(buffer, " is resting here.");
            break;

          case POSITION_SLEEPING:
            strcat(buffer, " is sleeping here.");
            break;

          case POSITION_FLYING:
            strcat(buffer, " is flying here.");
            break;

          case POSITION_SWIMMING:
            strcat(buffer, " is swimming here.");
            break;

          case POSITION_RIDING:
            if (i->specials.riding)
            {
              if (GET_POS(i->specials.riding) == POSITION_FLYING)
              {
                strcat(buffer, " is here, flying on ");
              }
              else
              {
                strcat(buffer, " is here, riding ");
              }

              strcat(buffer, GET_SHORT(i->specials.riding));
              strcat(buffer, ".");
            }
            break;

          case POSITION_FIGHTING:
            if (i->specials.fighting)
            {
              strcat(buffer, " is here, fighting ");

              if (i->specials.fighting == ch)
              {
                strcat(buffer, " YOU!");
              }
              else
              {
                if (CHAR_REAL_ROOM(i) == CHAR_REAL_ROOM(i->specials.fighting))
                {
                  if (IS_NPC(i->specials.fighting))
                  {
                    strcat(buffer, GET_SHORT(i->specials.fighting));
                  }
                  else
                  {
                    strcat(buffer, "someone.");
                  }
                }
                else
                {
                  strcat(buffer, "someone who has already left.");
                }
              }
            }
            else   /* NIL fighting pointer */
            {
              strcat(buffer, " is here struggling with thin air.");
            }
            break;

          default:
            strcat(buffer, " is floating here.");
            break;
        }
      }

      if (IS_AFFECTED(ch, AFF_DETECT_ALIGNMENT))
      {
        if (IS_EVIL(i))
        {
          strcat(buffer, " (Red Aura)");
        }
        else if (IS_GOOD(i))
        {
          strcat(buffer, " (Yellow Aura)");
        }
      }

      strcat(buffer, "\n\r");
      send_to_char(buffer, ch);
    }
    else    /* npc with long */
    {
      if (IS_AFFECTED(i, AFF_INVISIBLE))
      {
        strcpy(buffer, "*");
      }
      else
      {
        *buffer = '\0';
      }

      if (IS_AFFECTED(ch, AFF_DETECT_ALIGNMENT))
      {
        if (IS_EVIL(i))
        {
          strcat(buffer, " (Red Aura)");
        }
        else if (IS_GOOD(i))
        {
          strcat(buffer, " (Yellow Aura)");
        }
      }

      if (ch->questmob && ch->questmob == i)
      {
        strcat(buffer, "[TARGET] ");
      }

      if (IS_SET(i->specials.affected_by2, AFF_SEVERED))
      {
        strcat(buffer, MOB_SHORT(i));
        strcat(buffer, "'s upper torso is here... twitching.");
      }
      else if (i->specials.death_timer == 2)
      {
        strcat(buffer, MOB_SHORT(i));
        strcat(buffer, " lies here... near death.");
      }
      else
      {
        strcat(buffer, MOB_LONG(i));
      }

      send_to_char(buffer, ch);
    }

    if (IS_SET(i->specials.pflag, PLR_WRITING))
    {
      act("......$n is writing a message.", FALSE, i, 0, ch, TO_VICT);
    }

    if (!IS_SET(ch->specials.pflag, PLR_TAGBRF))
    {
      if (affected_by_spell(i, SKILL_CAMP))
      {
        act("......$n is camping here.", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SMELL_FARTMOUTH))
      {
        act("......brown fumes waft from $n's mouth.", FALSE, i, 0, ch, TO_VICT);
      }

      signal_char(i, ch, MSG_SHOW_AFFECT_TEXT, "");

      if (affected_by_spell(i, SPELL_ORB_PROTECTION))
      {
        act("......a shield of power emanates from an orb above $n's head!", FALSE, i, 0, ch, TO_VICT);
      }
    }

    if (IS_AFFECTED(i, AFF_SANCTUARY) && !affected_by_spell(i, SPELL_DISRUPT_SANCT))
    {
      act("......$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
    }
    else if (IS_AFFECTED(i, AFF_SANCTUARY) && affected_by_spell(i, SPELL_DISRUPT_SANCT))
    {
      act("......$n's protective aura has been disrupted!", FALSE, i, 0, ch, TO_VICT);
    }

    if (IS_AFFECTED(i, AFF_SPHERE) && !affected_by_spell(i, SPELL_DISTORTION))
    {
      act("......$n is surrounded by a golden sphere!", FALSE, i, 0, ch, TO_VICT);
    }
    else if (IS_AFFECTED(i, AFF_SPHERE) && affected_by_spell(i, SPELL_DISTORTION))
    {
      act("......$n's golden sphere seems to shimmer and blur in weakness!", FALSE, i, 0, ch, TO_VICT);
    }

    if (IS_AFFECTED(i, AFF_INVUL))
    {
      act("......$n is surrounded by a powerful sphere!", FALSE, i, 0, ch, TO_VICT);
    }

    if (!IS_SET(ch->specials.pflag, PLR_TAGBRF))
    {
      for (stop = FALSE, aff = i->affected; stop == FALSE && aff; aff = aff->next)
      {
        if (aff->type == SPELL_WARCHANT && aff->location == APPLY_HITROLL && aff->modifier < 0)
        {
          act("......$n hears the sound of war!", FALSE, i, 0, ch, TO_VICT);

          stop = TRUE;
        }
      }

      for (stop = FALSE, aff = i->affected; stop == FALSE && aff; aff = aff->next) {
        if (aff->type == SPELL_WRATH_OF_GOD && aff->location == APPLY_DAMROLL) {
          if (aff->modifier <= -50) {
            act("......$n is being crushed by the wrath of gods!", FALSE, i, 0, ch, TO_VICT);
          }
          else if (aff->modifier <= -40) {
            act("......$n is tormented by a holy force of will!", FALSE, i, 0, ch, TO_VICT);
          }
          else if (aff->modifier <= -30) {
            act("......$n is persecuted by celestial powers!", FALSE, i, 0, ch, TO_VICT);
          }
          else if (aff->modifier <= -20) {
            act("......$n is oppressed by divine intervention.", FALSE, i, 0, ch, TO_VICT);
          }
          else {
            act("......$n suffers from righteous indignation.", FALSE, i, 0, ch, TO_VICT);
          }

          stop = TRUE;
        }
      }

      if (affected_by_spell(i, SPELL_DIVINE_INTERVENTION))
      {
        act("......$n is sheltered from death by a divine aura.", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_FORTIFICATION))
      {
        act("......$n is protected by a barrier of magical fortification.", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_BLADE_BARRIER))
      {
        act("......$n is surrounded by a barrier of whirling blades.", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_BLACKMANTLE))
      {
        act("......$n is surrounded by an eerie mantle of darkness.", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_MAGIC_ARMAMENT))
      {
        act("......$n is enveloped by a faint red glow.", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_FURY) || (IS_AFFECTED(i, AFF_FURY) && !IS_NPC(i)))
      {
        act("......$n is snarling and fuming with fury!", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_RAGE))
      {
        act("......$n is seething with hatred and rage!", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_BLOOD_LUST))
      {
        act("......$n thirsts for blood and lusts for carnage!", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_DESECRATE))
      {
        act("......$n's sinister presence desecrates the surroundings.", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_QUICK))
      {
        act("......$n's words and gestures are magically quickened.", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_HASTE))
      {
        act("......$n's hastened actions move faster than reality.", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_BLUR))
      {
        act("......$n's movements blur in and out of reality.", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_MYSTIC_SWIFTNESS))
      {
        act("......$n's hands move with mystical swiftness.", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_RUSH))
      {
        act("......$n's pulse rushes with supernatural speed.", FALSE, i, 0, ch, TO_VICT);
      }

      if (duration_of_spell(i, SPELL_SHADOW_WRAITH) > 0)
      {
        act("......$n's shadow stretches to the north.", FALSE, i, 0, ch, TO_VICT);
      }

      if (duration_of_spell(i, SPELL_SHADOW_WRAITH) > 20)
      {
        act("......$n's shadow stretches to the south.", FALSE, i, 0, ch, TO_VICT);
      }

      if (duration_of_spell(i, SPELL_SHADOW_WRAITH) > 40)
      {
        act("......$n's shadow stretches to the east.", FALSE, i, 0, ch, TO_VICT);
      }

      if (duration_of_spell(i, SPELL_SHADOW_WRAITH) > 60)
      {
        act("......$n's shadow stretches to the west.", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_BLINDNESS))
      {
        act("......$n stumbles about wildly!", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_PARALYSIS))
      {
        act("......$n is completely immobilized!", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_HOLD))
      {
        act("......$n is rooted to the ground!", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_DEBILITATE))
      {
        act("......$n is enveloped by a greenish smoke.", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_CLOUD_CONFUSION))
      {
        act("......$n drools absentmindedly.", FALSE, i, 0, ch, TO_VICT);
      }

      if (affected_by_spell(i, SPELL_INCENDIARY_CLOUD))
      {
        act("......$n is enveloped by a huge ball of flame!", FALSE, i, 0, ch, TO_VICT);
      }

      /*
      if (affected_by_spell(i, SPELL_IRONSKIN))
      {
      act("......$n's skin is as hard and impervious as iron.", FALSE, i, 0, ch, TO_VICT);
      }
      */

      if (IS_NPC(i))
      {
        if (mob_proto_table[i->nr].tagline)
        {
          for (tag = mob_proto_table[i->nr].tagline; tag; tag = tag->next)
          {
            act(tag->desc, FALSE, i, 0, ch, TO_VICT);
          }
        }
      }
    }
  }
  else if (mode == 1)
  {
    if (IS_NPC(i) && GET_LEVEL(ch) >= LEVEL_IMM)
    {
      sprintf(buffer, "WIZINFO: %s looks at %s", GET_NAME(ch), GET_SHORT(i));
      wizlog(buffer, GET_LEVEL(ch) + 1, 5);
      log_s(buffer);
    }

    if (IS_NPC(i) && (IS_AFFECTED(i, AFF_STATUE)) && (GET_LEVEL(ch) < LEVEL_IMM))
    {
      if (signal_char(ch, ch, MSG_STONE, "")) return;

      send_to_char("When you look into its eye, you slowly turn into a statue.\n\r", ch);

      if (GET_OPPONENT(i) && GET_OPPONENT(i) == ch)
      {
        stop_fighting(i);
      }

      if (GET_OPPONENT(i))
      {
        stop_fighting(ch);
      }

      act("$n screams in pain as $s slowly turns to stone.", FALSE, ch, 0, i, TO_ROOM);

      if ((CHAOSMODE || IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC)) &&
          !IS_NPC(ch) && GET_LEVEL(ch) < LEVEL_IMM)
      {
        sprintf(buffer, "%s stoned by %s at %s", GET_NAME(ch), GET_SHORT(i), world[CHAR_REAL_ROOM(ch)].name);

        for (d = descriptor_list; d; d = d->next)
        {
          if (!d->connected)
          {
            act(buffer, 0, d->character, 0, 0, TO_CHAR);
          }
        }

        number_of_kills++;

        if (number_of_kills < 100)
        {
          sprintf(scores[number_of_kills].killer, "%s", (IS_NPC(i) ? MOB_SHORT(i) : GET_NAME(i)));
          sprintf(scores[number_of_kills].killed, "%s", GET_NAME(ch));
          sprintf(scores[number_of_kills].location, "%s", world[CHAR_REAL_ROOM(ch)].name);
          ct = time(0);
          tmstr = asctime(localtime(&ct));
          *(tmstr + strlen(tmstr) - 1) = '\0';
          sprintf(scores[number_of_kills].time_txt, "%s", tmstr);

          if (number_of_kills == CHAOSDEATH)
          {
            sprintf(buffer,
                    "**** Kill number %d has been reached, we have a winner!!! ****\n\r\n\r",
                    CHAOSDEATH);
            send_to_all(buffer);
            send_to_all(buffer); /* yes, twice */
          }
        }
        else
        {
          number_of_kills = 99;
        }
      }
      else
      {
        sprintf(buffer, "%s stoned by %s at %s (%d)",
          GET_NAME(ch), GET_SHORT(i),
          world[CHAR_REAL_ROOM(ch)].name, world[CHAR_REAL_ROOM(ch)].number);
        wizlog(buffer, LEVEL_IMM, 3);
      }

      log_s(buffer);
      deathlog(buffer);
      death_cry(ch);

      if (GET_QUEST_STATUS(ch) || GET_QUEST_STATUS(ch) == QUEST_COMPLETED)
      {
        GET_QUEST_TIMER(ch) = 30;
      }

      ch->questgiver = 0;

      if (ch->questobj)
      {
        ch->questobj->owned_by = 0;
      }

      ch->questobj = 0;

      if (ch->questmob)
      {
        ch->questmob->questowner = 0;
      }

      ch->questmob = 0;
      ch->quest_status = QUEST_NONE;
      ch->quest_level = 0;
      make_statue(ch);
      save_char(ch, NOWHERE);
      extract_char(ch);

      return;
    }

    if (MOB_DESCRIPTION(i) || !IS_NPC(i))
    {
      send_to_char(MOB_DESCRIPTION(i), ch);
    }
    else
    {
      act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
    }

    /* Show a character to another */

    if (GET_MAX_HIT(i) > 0)
    {
      percent = (100 * GET_HIT(i)) / GET_MAX_HIT(i);
    }
    else
    {
      percent = -1;
    }

    if (IS_NPC(i))
    {
      strcpy(buffer, GET_SHORT(i));
    }
    else
    {
      strcpy(buffer, GET_NAME(i));
    }

    if (percent >= 100)
    {
      strcat(buffer, " is in an excellent condition.\n\r");
    }
    else if (percent >= 90)
    {
      strcat(buffer, " has a few scratches.\n\r");
    }
    else if (percent >= 75)
    {
      strcat(buffer, " has some small wounds and bruises.\n\r");
    }
    else if (percent >= 50)
    {
      strcat(buffer, " has quite a few wounds.\n\r");
    }
    else if (percent >= 30)
    {
      strcat(buffer, " has some big nasty wounds and scratches.\n\r");
    }
    else if (percent >= 15)
    {
      strcat(buffer, " looks pretty hurt.\n\r");
    }
    else if (percent >= 0)
    {
      strcat(buffer, " is in an awful condition.\n\r");
    }
    else
    {
      strcat(buffer, " is bleeding awfully from big wounds.\n\r");
    }

    send_to_char(buffer, ch);
    found = FALSE;

    for (j = 0; j < MAX_WEAR; j++)
    {
      if (EQ(i, j))
      {
        if (CAN_SEE_OBJ(ch, EQ(i, j)))
        {
          found = TRUE;
        }
      }
    }

    if (found)
    {
      act("\n\r$n is using:", FALSE, i, 0, ch, TO_VICT);

      for (j = 0; j < MAX_WEAR; j++)
      {
        if (EQ(i, j))
        {
          if (CAN_SEE_OBJ(ch, EQ(i, j)))
          {
            send_to_char(where[j], ch);
            show_obj_to_char(EQ(i, j), ch, 1, 0);
          }
        }
      }
    }

    if (GET_MOUNT(i))
    {
      sprintf(buffer, "<riding>             %s", GET_SHORT(i->specials.riding));
      send_to_char(buffer, ch);
    }

    if (((GET_CLASS(ch) == CLASS_THIEF) && (ch != i))
        || (GET_LEVEL(ch) > LEVEL_DEI))
    {
      found = FALSE;
      send_to_char("\n\rYou attempt to peek at the inventory:\n\r", ch);

      for (tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content)
      {
        if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, 40) < GET_LEVEL(ch)) &&
            tmp_obj->obj_flags.type_flag != ITEM_SC_TOKEN)
        {
          show_obj_to_char(tmp_obj, ch, 1, 0);
          found = TRUE;
        }
      }

      if (!found)
      {
        send_to_char("You can't see anything.\n\r", ch);
      }
    }
  }
  else if (mode == 2)
  {
    /* Lists inventory */
    act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
    list_obj_to_char(i->carrying, ch, 1, TRUE);
  }
}



void list_char_to_char(struct char_data *list,
           struct char_data *ch, int mode)
{
  struct char_data *i, *temp;

  for (i = list; i ; i = temp) {
    temp = i->next_in_room; /* Added temp - Ranger June 96 */
    if (ch!=i) show_char_to_char(i,ch,0);
  }
}


int check_extra_desc(CHAR *ch,char *tmp_desc);

void do_look(struct char_data *ch, char *argument, int cmd) {
  char buffer[MSL];
  char arg1[MSL];
  char arg2[MSL];
  int keyword_no;
  int j, bits, temp,window;
  bool found;
  struct obj_data *tmp_object, *found_object;
  struct char_data *tmp_char;
  char *tmp_desc;
  char *decay_text[10]=
  {"like new","almost new","fairly new","slightly worn","worn",
   "fairly worn","very worn","slightly cracked","cracked",
   "about to crumble"};
  int val3,max_time;

  char *keywords[]= {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "in",
    "at",
    "",  /* Look at '' case */
    "\n" };

  if (!ch->desc) return;

  if (GET_POS(ch) < POSITION_SLEEPING)
    send_to_char("You can't see anything but stars!\n\r", ch);
  else if (GET_POS(ch) == POSITION_SLEEPING)
    send_to_char("You can't see anything, you're sleeping!\n\r", ch);
  else if ( IS_AFFECTED(ch, AFF_BLIND) )
    send_to_char("You can't see anything, you're blind!\n\r", ch);
  else if (IS_DARK(CHAR_REAL_ROOM(ch)) &&
           !IS_AFFECTED(ch, AFF_INFRAVISION) &&
     (GET_LEVEL(ch) < LEVEL_IMM))
    send_to_char("It is pitch black...\n\r", ch);
  else {
    argument_split_2(argument,arg1,arg2);
    keyword_no = search_block(arg1, keywords, FALSE); /* Partial Match */

    if ((keyword_no == -1) && *arg1) {
      keyword_no = 7;
      strcpy(arg2, arg1); /* Let arg2 become the target object (arg1) */
    }

    found = FALSE;
    tmp_object = 0;
    tmp_desc   = 0;

    switch(keyword_no) {
      /* look <dir> */
    case 0 :
    case 1 :
    case 2 :
    case 3 :
    case 4 :
    case 5 : {
      if (EXIT(ch, keyword_no)) {
  if (EXIT(ch, keyword_no)->general_description) {
    send_to_char(EXIT(ch, keyword_no)->
           general_description, ch);
  } else {
    send_to_char("You see nothing special.\n\r", ch);
  }

  if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_CLOSED) &&
      (EXIT(ch, keyword_no)->keyword)) {
    sprintf(buffer, "The %s is closed.\n\r",
      fname(EXIT(ch, keyword_no)->keyword));
    send_to_char(buffer, ch);
  } else {
    if (IS_SET(EXIT(ch, keyword_no)->exit_info, EX_ISDOOR) &&
        EXIT(ch, keyword_no)->keyword) {
      sprintf(buffer, "The %s is open.\n\r",
        fname(EXIT(ch, keyword_no)->keyword));
      send_to_char(buffer, ch);
    }
  }
      } else {
  send_to_char("Nothing special there...\n\r", ch);
      }
    } break;

      /* look 'in'  */
    case 6: {
      if (*arg2) {
  /* Item carried */

  bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
          FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

  if (bits) { /* Found something */
    if (GET_ITEM_TYPE(tmp_object)== ITEM_DRINKCON) {
      if (tmp_object->obj_flags.value[1] <= 0) {
        act("It is empty.", FALSE, ch, 0, 0, TO_CHAR);
      } else {
        temp=((tmp_object->obj_flags.value[1]*3)/tmp_object->obj_flags.value[0]);
        sprintf(buffer,"It's %sfull of a %s liquid.\n\r",
          fullness[temp],color_liquid[tmp_object->obj_flags.value[2]]);
        send_to_char(buffer, ch);
      }
    } else if (GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER ||
          GET_ITEM_TYPE(tmp_object) == ITEM_AQ_ORDER) {
      if (!IS_SET(tmp_object->obj_flags.value[1],CONT_CLOSED) ||
          GET_ITEM_TYPE(tmp_object) == ITEM_AQ_ORDER) {
        send_to_char(fname(OBJ_NAME(tmp_object)), ch);
        switch (bits) {
        case FIND_OBJ_INV :
    send_to_char(" (carried) : \n\r", ch);
    break;
        case FIND_OBJ_ROOM :
    send_to_char(" (here) : \n\r", ch);
    break;
        case FIND_OBJ_EQUIP :
    send_to_char(" (used) : \n\r", ch);
    break;
        }
        list_obj_to_char(tmp_object->contains, ch, 2, TRUE);
      } else
        send_to_char("It is closed.\n\r", ch);
    } else {
      send_to_char("That is not a container.\n\r", ch);
    }
  } else { /* wrong argument */
    send_to_char("You do not see that item here.\n\r", ch);
  }
      } else { /* no argument */
  send_to_char("Look in what?!\n\r", ch);
      }
    } break;

      /* look 'at'  */
    case 7 : {

      if (*arg2) {
  bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
          FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &tmp_char, &found_object);
  if (tmp_char) {
    if (ch != tmp_char) {
      act("$n looks at you.", TRUE, ch, 0, tmp_char, TO_VICT);
      act("$n looks at $N.", TRUE, ch, 0, tmp_char, TO_NOTVICT);
    }
    show_char_to_char(tmp_char, ch, 1);
    return;
  }

  /* Search for Extra Descriptions in room and items */
  /* Extra description in room?? */
        /* Added window feature - Ranger August 96 */
  if (!found) {
    tmp_desc = find_ex_description(arg2, world[CHAR_REAL_ROOM(ch)].ex_description);
    if (tmp_desc) {
            sprintf(buffer,"$n looks at the %s.",arg2);
            act(buffer,1,ch,0,0,TO_ROOM);
            if(isdigit(*tmp_desc)) {
              window=atoi(tmp_desc);
              look_in_room(ch,window);
              return;
            }
            else page_string(ch->desc, tmp_desc, 0);
      return; /* RETURN SINCE IT WAS A ROOM DESCRIPTION */
      /* Old system was: found = TRUE; */
    }
  }

  /* Search for extra descriptions in items */
  /* Equipment Used */

  if (!found) {
    for (j = 0; j< MAX_WEAR && !found; j++) {
      if (ch->equipment[j]) {
        if (CAN_SEE_OBJ(ch,ch->equipment[j])) {
    tmp_desc = find_ex_description(arg2,
                 ch->equipment[j]->ex_description);
    if (tmp_desc) {
                  if(isdigit(*tmp_desc)) {
                    window=atoi(tmp_desc);
                    look_in_room(ch,window);
                    return;
                  }
      else {
       page_string(ch->desc, tmp_desc, 1);
       if(IS_SET(ch->equipment[j]->obj_flags.extra_flags2, ITEM_ALL_DECAY) || IS_SET(ch->equipment[j]->obj_flags.extra_flags2, ITEM_EQ_DECAY)) {
         max_time=obj_proto_table[ch->equipment[j]->item_number].obj_flags.timer;
         if(max_time)
           val3=9-10*ch->equipment[j]->obj_flags.timer/max_time;
         else val3=9;
         if(val3<0) val3=0;
         val3=MIN(val3,9);
         printf_to_char(ch,"This item decays and seems %s.\n\r",decay_text[val3]);
       }
      }
      found = TRUE;
    }
        }
      }
    }
  }

  /* In inventory */

  if (!found) {
    for(tmp_object = ch->carrying;
        tmp_object && !found;
        tmp_object = tmp_object->next_content) {
      if CAN_SEE_OBJ(ch, tmp_object) {
        tmp_desc = find_ex_description(arg2,
               tmp_object->ex_description);
        if (tmp_desc) {
                if(isdigit(*tmp_desc)) {
                  window=atoi(tmp_desc);
                  look_in_room(ch,window);
                  return;
                }
    else {
      page_string(ch->desc, tmp_desc, 1);
       if(IS_SET(tmp_object->obj_flags.extra_flags2, ITEM_ALL_DECAY) || IS_SET(tmp_object->obj_flags.extra_flags2, ITEM_EQ_DECAY)) {
         max_time=obj_proto_table[tmp_object->item_number].obj_flags.timer;
         if(max_time)
           val3=9-10*tmp_object->obj_flags.timer/max_time;
         else val3=9;
         if(val3<0) val3=0;
         val3=MIN(val3,9);
         printf_to_char(ch,"This item decays and seems %s.\n\r",decay_text[val3]);
       }
    }
    found = TRUE;
        }
      }
    }
  }

  /* Object In room */

  if (!found) {
    for(tmp_object = world[CHAR_REAL_ROOM(ch)].contents;
        tmp_object && !found;
        tmp_object = tmp_object->next_content) {
      if CAN_SEE_OBJ(ch, tmp_object) {
        tmp_desc = find_ex_description(arg2,
               tmp_object->ex_description);
        if (tmp_desc) {
                if(isdigit(*tmp_desc)) {
                  window=atoi(tmp_desc);
                  look_in_room(ch,window);
                  return;
                }
    else {
      page_string(ch->desc, tmp_desc, 1);
       if(IS_SET(tmp_object->obj_flags.extra_flags2, ITEM_ALL_DECAY) || IS_SET(tmp_object->obj_flags.extra_flags2, ITEM_EQ_DECAY)) {
         max_time=obj_proto_table[tmp_object->item_number].obj_flags.timer;
         if(max_time)
           val3=9-10*tmp_object->obj_flags.timer/max_time;
         else val3=9;
         if(val3<0) val3=0;
         val3=MIN(val3,9);
         printf_to_char(ch,"This item decays and seems %s.\n\r",decay_text[val3]);
       }

    }
    found = TRUE;
        }
      }
    }
  }

  /* wrong argument */

  if (bits) { /* If an object was found */
    if (!found) {
      show_obj_to_char(found_object, ch, 5,0); /* Show no-description */
          }
    else
      show_obj_to_char(found_object, ch, 6,0); /* Find hum, glow etc */
  } else if (!found) {
    send_to_char("You do not see that here.\n\r", ch);
  }
      } else {
  /* no argument */
  send_to_char("Look at what?\n\r", ch);
      }
    } break;

      /* look ''    */
    case 8 : {

      look_in_room(ch,world[CHAR_REAL_ROOM(ch)].number);

    } break;

      /* wrong arg  */
    case -1 :
      send_to_char("Sorry, I didn't understand that!\n\r", ch);
      break;
    }
  }

}

/* end of look */

/* looking in room pulled from case 8 in do_look and added here to
simplify the addition of the window feature - Ranger Aug 96 */

void look_in_room(CHAR *ch, int vnum) {
  char buffer[MSL];
  char ebuf[MSL]="\0";
  char dbuf[MSL]="\0";
  int room,d,dd,i;
  OBJ *obj;
  char *keywords[]= {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down"};
  char *blood_messages[] = {
    "Should never see this.\n\r",
    "There's a little blood here.\n\r",
    "You're standing in some blood.\n\r",
    "The blood here is flowing.\n\r",
    "There's so much blood here it's intoxicating!\n\r",
    "How much more blood can there be in any one place?\n\r",
    "Such carnage. The God of Death is feasting tonight!\n\r",
    "You are splashing around in the blood of the slain!\n\r",
    "Even the spirits are revolted by the death and destruction!\n\r",
    "The Gods should show some mercy and cleanse this horrid place!\n\r",
    "So much blood has been shed here, you are drowning in it!\n\r",
  "\n"
  };

  room = real_room(vnum);
  if (room==-1) {
    send_to_char("All you see is an endless void.\n\r",ch);
    return;
  }

  COLOR(ch,3);
  send_to_char(world[room].name, ch);
  ENDCOLOR(ch);
  if IS_SET(world[room].room_flags, ARENA) send_to_char(" [*]", ch);

  if (GET_LEVEL(ch) >= LEVEL_IMM) {
    sprintf(buffer, " [%d] ",world[room].number);
    send_to_char(buffer, ch);

    if (IS_SET(world[room].room_flags, LIT)) send_to_char("(LIT) ", ch);
    if (IS_SET(world[room].room_flags, DARK)) send_to_char("(DARK) ", ch);
    if (IS_SET(world[room].room_flags, DEATH)) send_to_char("(DEATH) ", ch);
    if (IS_SET(world[room].room_flags, NO_MAGIC)) send_to_char("(NO_MAGIC) ", ch);
    if (IS_SET(world[room].room_flags, NO_MOB)) send_to_char("(NOMOB) ", ch);
    if (IS_SET(world[room].room_flags, SAFE)) send_to_char("(SAFE) ", ch);
    if (IS_SET(world[room].room_flags, PRIVATE)) send_to_char("(PRIVATE) ", ch);
    if (IS_SET(world[room].room_flags, LOCK)) send_to_char("(LOCK) ", ch);
    if (IS_SET(world[room].room_flags, TRAP)) send_to_char("(TRAP) ", ch);
    if (IS_SET(world[room].room_flags, ARENA)) send_to_char("(ARENA) ", ch);
    if (IS_SET(world[room].room_flags, CLUB)) send_to_char("(CLUB) ", ch);
    if (IS_SET(world[room].room_flags, LAWFULL)) send_to_char("(LAWFULL) ", ch);
    if (IS_SET(world[room].room_flags, QUIET)) send_to_char("(QUIET) ", ch);
    if (IS_SET(world[room].room_flags, TUNNEL)) send_to_char("(TUNNEL) ", ch);
    if (IS_SET(world[room].room_flags, INDOORS)) send_to_char("(INDOORS) ", ch);
    if (IS_SET(world[room].room_flags, CHAOTIC)) send_to_char("(CHAOTIC) ", ch);
    if (IS_SET(world[room].room_flags, HAZARD)) send_to_char("(HAZARD) ", ch);
    if (IS_SET(world[room].room_flags, MOVETRAP)) send_to_char("(MOVETRAP) ", ch);
    if (IS_SET(world[room].room_flags, NO_BEAM)) send_to_char("(NO_BEAM) ", ch);
    if (IS_SET(world[room].room_flags, FLYING)) send_to_char("(FLYING) ", ch);
    if (IS_SET(world[room].room_flags, NO_PEEK)) send_to_char("(NO_PEEK) ", ch);
    if (IS_SET(world[room].room_flags, NO_SONG)) send_to_char("(NO_SONG) ", ch);
    if (IS_SET(world[room].room_flags, NO_REGEN)) send_to_char("(NO_REGEN) ", ch);
    if (IS_SET(world[room].room_flags, NO_QUAFF)) send_to_char("(NO_QUAFF) ", ch);
    if (IS_SET(world[room].room_flags, REV_REGEN)) send_to_char("(REV_REGEN) ", ch);
    if (IS_SET(world[room].room_flags, DOUBLE_MANA)) send_to_char("(DOUBLE_MANA) ", ch);
    if (IS_SET(world[room].room_flags, HALF_CONC)) send_to_char("(HALF_CONC) ", ch);
    if (IS_SET(world[room].room_flags, QRTR_CONC)) send_to_char("(QRTR_CONC) ", ch);
    if (IS_SET(world[room].room_flags, NO_ORB)) send_to_char("(NO_ORB) ", ch);
    if (IS_SET(world[room].room_flags, MANADRAIN)) send_to_char("(MANADRAIN) ", ch);
    if (IS_SET(world[room].room_flags, NO_SUM)) send_to_char("(NO_SUM) ", ch);
/*
    if (IS_SET(world[room].room_flags, MANABURN)) send_to_char("(MANABURN) ", ch);
*/
  }
  send_to_char("\n\r", ch);

  if (!IS_SET(ch->specials.pflag, PLR_BRIEF)) {
    send_to_char(world[room].description, ch);
    if (RM_BLOOD(room) > 0)
      send_to_char(blood_messages[(int)RM_BLOOD(room)],ch);
  }

  send_to_char("   Obvious Exits:",ch);
  dd=0;
  COLOR(ch,4);
  for(d=0;d<=5;d++) {
    if(world[room].dir_option[d])  {
      if(world[room].dir_option[d]->to_room_r!=NOWHERE &&
         world[room].dir_option[d]->to_room_r!=real_room(0)
          && !IS_SET(world[room].dir_option[d]->exit_info,EX_CLOSED)
          && !IS_SET(world[room].dir_option[d]->exit_info,EX_CRAWL)
          && !IS_SET(world[room].dir_option[d]->exit_info,EX_JUMP)
          && !IS_SET(world[room].dir_option[d]->exit_info,EX_ENTER)
          && !IS_SET(world[room].dir_option[d]->exit_info,EX_CLIMB)) {
        dd++;
        strcat(ebuf," [");
        if(ch->colors[0]&& ch->colors[2]) {
          strcat(ebuf,Color[(((ch->colors[2])*2)-2)]);
          strcat(ebuf,BKColor[ch->colors[13]]);
        }
        if(ch->colors[0]&& ch->colors[4]) {
          strcat(ebuf,Color[(((ch->colors[4])*2)-2)]);
          strcat(ebuf,BKColor[ch->colors[13]]);
        }
        strcat(ebuf,keywords[d]);
        strcat(ebuf,"]");
      }
    }
  }
  if(!dd) strcat(ebuf,"  None.\n\r");
  else strcat(ebuf,"\n\r");
  send_to_char(ebuf,ch);
  ENDCOLOR(ch);

  if(GET_LEVEL(ch)>LEVEL_WIZ) {
    send_to_char("           Doors:",ch);
    dd=0;
    COLOR(ch,4);
    for(d=0;d<=5;d++) {
      if(world[room].dir_option[d])  {
  if(world[room].dir_option[d]->to_room_r!=NOWHERE
            && IS_SET(world[room].dir_option[d]->exit_info,EX_CLOSED)) {
          dd++;
    strcat(dbuf," [");
    strcat(dbuf,keywords[d]);
    strcat(dbuf,"]");
  }
      }
    }

    if(!dd) strcat(dbuf,"  None.\n\r");
    else strcat(dbuf,"\n\r");
    send_to_char(dbuf,ch);
    ENDCOLOR(ch);
  }

   if(GET_LEVEL(ch)>LEVEL_WIZ) {
    sprintf(buffer,"     Sector Type:  %s (%d)\n\r",
     sector_types[world[CHAR_REAL_ROOM(ch)].sector_type],
     movement_loss[world[CHAR_REAL_ROOM(ch)].sector_type]);
    send_to_char(buffer,ch);
   }

   if(GET_LEVEL(ch)>LEVEL_WIZ) {
    send_to_char("     Extra Exits:",ch);
    dd=0;
    COLOR(ch,4);
    strcpy(ebuf,"");
    for(d=0;d<=5;d++) {
     if(world[room].dir_option[d])  {
      if((world[room].dir_option[d]->to_room_r!=NOWHERE &&
         world[room].dir_option[d]->to_room_r!=real_room(0)) &&
          (IS_SET(world[room].dir_option[d]->exit_info,EX_CRAWL) ||
           IS_SET(world[room].dir_option[d]->exit_info,EX_JUMP) ||
           IS_SET(world[room].dir_option[d]->exit_info,EX_ENTER) ||
           IS_SET(world[room].dir_option[d]->exit_info,EX_CLIMB)) ) {
  dd++;
  strcat(ebuf," [");
  strcat(ebuf,keywords[d]);
  strcat(ebuf,"]");
      }
     }
    }
    if(!dd) strcat(ebuf,"  None.\n\r");
    else strcat(ebuf,"\n\r");
    send_to_char(ebuf,ch);
    ENDCOLOR(ch);
  }

  if (!IS_SET(ch->specials.pflag, PLR_SUPERBRF)) {
    if(world[room].number==3084) {
      i=0;
      for(obj=world[room].contents;obj;obj=obj->next_content) {
        if(CAN_SEE_OBJ(ch,obj) && obj->obj_flags.type_flag!=ITEM_BOARD) i++;
      }
      if(i)
        printf_to_char(ch,"There are `i[%d]`q donated items here, use `i'LIST'`q to see them.\n\r",i);
      send_to_char("[1] An Adventure bulletin board is mounted on a wall here.(glowing)\n\r",ch);
    }
    else {
      list_obj_to_char(world[room].contents, ch, 0,FALSE);
    }
    if(world[room].number!=1203) list_char_to_char(world[room].people, ch, 0);
  }
 return;
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
    if ((GET_ITEM_TYPE(tmp_object)==ITEM_DRINKCON) ||
        (GET_ITEM_TYPE(tmp_object)==ITEM_CONTAINER) ||
        (GET_ITEM_TYPE(tmp_object)==ITEM_AQ_ORDER)) {
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

void increase_blood(int rm) {
  RM_BLOOD(rm) = MIN(RM_BLOOD(rm) + 1, 10);
}

/* Remove the Immortalis' Grace enchant. */
void imm_grace_remove_enchant(CHAR *ch)
{
  ENCH *ench = NULL;

  for (ench = ch->enchantments; ench; ench = ench->next)
  {
    if (ench->type == ENCHANT_IMM_GRACE)
    {
      enchantment_remove(ch, ench, FALSE);
    }
  }
}

/* Add the Immortali's Grace enchant. */
void imm_grace_add_enchant(CHAR *ch)
{
  char buf[MSL];
  ENCH ench;

  sprintf(buf, "Immortalis' Grace");
  ench.name = buf;
  enchantment_to_char(ch, &ench, TRUE);
}

/* Function used to maniplulate death experience. Don't use this unless you know what you're doing. */
int adjust_death_exp(CHAR *ch, int exp)
{
  GET_DEATH_EXP(ch) += exp;

  /* Did the player exhaust their death xp pool? */
  if (!GET_DEATH_EXP(ch))
  {
    GET_DEATH_EXP(ch) = 0;

    send_to_char("You are no longer affected by Immortalis' Grace.\n\r", ch);

    imm_grace_remove_enchant(ch);
  }

  return exp;
}

#define DEATH_EXP_MULTIPLIER 2
/* Give the player death experience (if they deserve it), and return how much experience was given. */
int gain_death_exp(CHAR *ch, int exp)
{
  if (!GET_DEATH_EXP(ch)) return 0;

  exp *= DEATH_EXP_MULTIPLIER;

  if (exp > GET_DEATH_EXP(ch)) exp = GET_DEATH_EXP(ch);

  adjust_death_exp(ch, -1 * exp); /* Deduct the experience from the death experience pool. */
  gain_exp(ch, exp); /* Give the death experience to the player. */

  return exp;
}

#define DEATH_EXP_PERCENT .90
void die(CHAR *ch)
{
                            /* mu   cl   th   wa   ni   no   pa   ap   av   ba   co */
  const int hit_limit[11]  = {250, 250, 450, 500, 350, 600, 350, 350,   0, 300, 400};
  const int mana_limit[11] = {500, 500, 100, 100, 350, 100, 350, 350, 100, 400, 350};

  char buf[MAX_INPUT_LENGTH];
  int i = 0, factor = 1, percent = 0, mana_chk = 0, hit_chk = 0, death_chk = 0, mana_diff = 0, hit_diff = 0;
  unsigned long death_exp = 0;

  if (signal_room(CHAR_REAL_ROOM(ch), ch, MSG_CORPSE, "")) return;

  increase_blood(CHAR_REAL_ROOM(ch));

  if (!IS_NPC(ch) && ch->specials.death_timer == 1 && GET_HIT(ch) >= 0)
  {
    sprintf(buf, "%s killed by Imminent Death.", GET_NAME(ch));
      wizlog(buf, LEVEL_IMM, 3);
    log_s(buf);
    deathlog(buf);

    death_chk = 1;
    factor = 2;
  }

  if (!IS_NPC(ch) && IS_SET(ch->specials.affected_by2, AFF_SEVERED) && GET_HIT(ch) >= 0)
  {
    sprintf(buf, "%s killed by massive body trama.", GET_NAME(ch));
    log_s(buf);
    deathlog(buf);
    wizlog(buf, LEVEL_IMM, 3);
  }

  if (!IS_NPC(ch) && IS_SET(world[CHAR_REAL_ROOM(ch)].room_flags, CHAOTIC) && !ch->specials.death_timer && !IS_SET(ch->specials.affected_by2, AFF_SEVERED))
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

  if (!IS_NPC(ch) && (ch->points.max_mana + 100) > mana_limit[GET_CLASS(ch) - 1])
  {
    mana_chk = TRUE;
  }

  if (!IS_NPC(ch) && ch->points.max_hit > hit_limit[GET_CLASS(ch) - 1])
  {
    hit_chk = TRUE;
  }

  if (!IS_NPC(ch) && GET_DEATH_LIMIT(ch) && GET_BEEN_KILLED(ch) > GET_DEATH_LIMIT(ch))
  {
    death_chk = TRUE;
    factor = number(2, 5);
  }

  if (mana_chk || hit_chk  || death_chk)
  {
    send_to_char("\n\r\n\rThe Reaper demands his toll for your passage through the underworld.\n\r", ch);
    send_to_char("Your soul burns as he tears some lifeforce from you.\n\r", ch);

    mana_diff = ch->points.max_mana + 100 - mana_limit[GET_CLASS(ch) - 1];
    hit_diff = ch->points.max_hit - hit_limit[GET_CLASS(ch) - 1];
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
            hit_diff, ch->points.max_hit, mana_diff, ch->points.max_mana + 100);
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

  if (IS_SET(ch->specials.affected_by2, AFF_SEVERED))
  {
    REMOVE_BIT(ch->specials.affected_by2, AFF_SEVERED);
  }

  if (IS_SET(ch->specials.affected_by2, AFF_IMMINENT_DEATH))
  {
    REMOVE_BIT(ch->specials.affected_by2, AFF_IMMINENT_DEATH);
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
      send_to_char("\n\rThe Reaper intones 'You may keep your current experience, but\n\r\after level six you will begin to lose half of it at death.'\n\r", ch);
    }

    if (death_exp)
    {
      death_exp = lround((double)death_exp * DEATH_EXP_PERCENT);

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
  else if (IS_SET(ch->specials.pflag, PLR_LEMS_LIQOUR_LOUNGE))
    return "Lem's Liqour Lounge";
  else if (IS_SET(ch->specials.pflag, PLR_RANGERS_RELIQUARY))
    return "Ranger's Reliquary";
  else
    return "None";
}

/* Prints skill/spell affects from worn equipment, applied skills and
   spells, and enchantments. */
void do_affect(CHAR *ch, char *arg, int cmd) {
  int i = 0;
  OBJ *obj = NULL;
  AFF *tmp_af = NULL;
  ENCH *tmp_ench = NULL;
  char buf[MIL];
  char buf2[MIL];
  bool equipment[MAX_SPL_LIST] = { FALSE };
  bool affects[MAX_SPL_LIST] = { FALSE };
  bool eq_af = FALSE;

  /* Get affects applied by worn equipment. */
  for (i = 0; i < MAX_WEAR; i++) {
    if (!(obj = EQ(ch, i))) continue;

    /* Set a simple flag for use later that shows that there was
       some equpment worn that applied an affect. */
    if (!eq_af &&
        (obj->obj_flags.bitvector || obj->obj_flags.bitvector2)) {
      eq_af = TRUE;
    }

    /* affected_by */
    if (IS_SET(obj->obj_flags.bitvector, AFF_BLIND) &&
        IS_SET(ch->specials.affected_by, AFF_BLIND)) {
      equipment[SPELL_BLINDNESS] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_INVISIBLE) &&
        IS_SET(ch->specials.affected_by, AFF_INVISIBLE)) {
      equipment[SPELL_INVISIBLE] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_DETECT_ALIGNMENT) &&
        IS_SET(ch->specials.affected_by, AFF_DETECT_ALIGNMENT)) {
      equipment[SPELL_DETECT_ALIGNMENT] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_DETECT_INVISIBLE) &&
        IS_SET(ch->specials.affected_by, AFF_DETECT_INVISIBLE)) {
      equipment[SPELL_DETECT_INVISIBLE] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_DETECT_MAGIC) &&
        IS_SET(ch->specials.affected_by, AFF_DETECT_MAGIC)) {
      equipment[SPELL_DETECT_MAGIC] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_SENSE_LIFE) &&
        IS_SET(ch->specials.affected_by, AFF_SENSE_LIFE)) {
      equipment[SPELL_SENSE_LIFE] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_HOLD) &&
        IS_SET(ch->specials.affected_by, AFF_HOLD)) {
      equipment[SPELL_HOLD] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_SANCTUARY) &&
        IS_SET(ch->specials.affected_by, AFF_SANCTUARY)) {
      equipment[SPELL_SANCTUARY] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_CONFUSION) &&
        IS_SET(ch->specials.affected_by, AFF_CONFUSION)) {
      equipment[SPELL_CONFUSION] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_CURSE) &&
        IS_SET(ch->specials.affected_by, AFF_CURSE)) {
      equipment[SPELL_CURSE] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_SPHERE) &&
        IS_SET(ch->specials.affected_by, AFF_SPHERE)) {
      equipment[SPELL_SPHERE] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_POISON) &&
        IS_SET(ch->specials.affected_by, AFF_POISON)) {
      equipment[SPELL_POISON] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_PROTECT_EVIL) &&
        IS_SET(ch->specials.affected_by, AFF_PROTECT_EVIL)) {
      equipment[SPELL_PROTECT_FROM_EVIL] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_PARALYSIS) &&
        IS_SET(ch->specials.affected_by, AFF_PARALYSIS)) {
      equipment[SPELL_PARALYSIS] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_INFRAVISION) &&
        IS_SET(ch->specials.affected_by, AFF_INFRAVISION)) {
      equipment[SPELL_INFRAVISION] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_SLEEP) &&
        IS_SET(ch->specials.affected_by, AFF_SLEEP)) {
      equipment[SPELL_SLEEP] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_DODGE) &&
        IS_SET(ch->specials.affected_by, AFF_DODGE)) {
      equipment[SKILL_DODGE] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_SNEAK) &&
        IS_SET(ch->specials.affected_by, AFF_SNEAK)) {
      equipment[SKILL_SNEAK] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_HIDE) &&
        IS_SET(ch->specials.affected_by, AFF_HIDE)) {
      equipment[SKILL_HIDE] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_ANIMATE) &&
        IS_SET(ch->specials.affected_by, AFF_ANIMATE)) {
      equipment[SPELL_ANIMATE_DEAD] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_CHARM) &&
        IS_SET(ch->specials.affected_by, AFF_CHARM)) {
      equipment[SPELL_CHARM_PERSON] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_PROTECT_GOOD) &&
        IS_SET(ch->specials.affected_by, AFF_PROTECT_GOOD)) {
      equipment[SPELL_PROTECT_FROM_GOOD] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_FLY) &&
        IS_SET(ch->specials.affected_by, AFF_FLY)) {
      equipment[SPELL_FLY] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_IMINV) &&
        IS_SET(ch->specials.affected_by, AFF_IMINV)) {
      equipment[SPELL_IMP_INVISIBLE] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_INVUL) &&
        IS_SET(ch->specials.affected_by, AFF_INVUL)) {
      equipment[SPELL_INVUL] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_DUAL) &&
        IS_SET(ch->specials.affected_by, AFF_DUAL)) {
      equipment[SKILL_DUAL] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector, AFF_FURY) &&
        IS_SET(ch->specials.affected_by, AFF_FURY)) {
      equipment[SPELL_FURY] = TRUE;
    }

    /* affected_by2 */
    if (IS_SET(obj->obj_flags.bitvector2, AFF_TRIPLE) &&
        IS_SET(ch->specials.affected_by2, AFF_TRIPLE)) {
      equipment[SKILL_TRIPLE] = TRUE;
    }

    if (IS_SET(obj->obj_flags.bitvector2, AFF_QUAD) &&
        IS_SET(ch->specials.affected_by2, AFF_QUAD)) {
      equipment[SKILL_QUAD] = TRUE;
    }
  }

  /* Check if we need to actually print anything. */
  if (eq_af ||
      ch->affected ||
      ch->enchantments) {
    send_to_char("\n\rAffected by:\n\r-----------", ch);

    /* Print skill/spell affects first. */
    if (ch->affected) {
      send_to_char("\n\r", ch);

      for (tmp_af = ch->affected; tmp_af; tmp_af = tmp_af->next) {
        if (!equipment[tmp_af->type] && !affects[tmp_af->type]) {
          affects[tmp_af->type] = TRUE;

          snprintf(buf, sizeof(buf), "'%s'", spells[tmp_af->type - 1]);

          if (tmp_af->duration == -1) {
            snprintf(buf2, sizeof(buf2), "Never Expires");
          }
          else {
            if (tmp_af->type == SKILL_MANTRA) {
              snprintf(buf2, sizeof(buf2), "Expires in: ~%3d Secs.", tmp_af->duration * 10);
            }
            else {
              snprintf(buf2, sizeof(buf2), "Expires in: %4d Tick%s", tmp_af->duration, tmp_af->duration > 1 ? "s" : "");
            }
          }

          printf_to_char(ch, "Skill/Spell: %-34s %s\n\r", buf, buf2);
        }
      }
    }

    /* Next, print enchantments. They are unique by name, so this is
       a bit more simple than skill/spell affects. */
    if (ch->enchantments) {
      send_to_char("\n\r", ch);

      for (tmp_ench = ch->enchantments; tmp_ench; tmp_ench = tmp_ench->next) {
        snprintf(buf, sizeof(buf), "'%s'", tmp_ench->name);

        if (tmp_ench->duration == -1) {
          snprintf(buf2, sizeof(buf2), "Never Expires");
        }
        else {
          snprintf(buf2, sizeof(buf2), "Expires in: %4d Tick%s", tmp_ench->duration, tmp_ench->duration > 1 ? "s" : " ");
        }

        printf_to_char(ch, "Enchantment: %-34s %s\n\r", buf, buf2);
      }
    }

    /* Finally, print any affects from worn equipment. */
    if (eq_af) {
      send_to_char("\n\r", ch);

      for (i = 0; i < MAX_SPL_LIST; i++) {
        if (equipment[i]) {
          snprintf(buf, sizeof(buf), "'%s'", spells[i - 1]);

          printf_to_char(ch, "  Equipment: %-34s Never Expires\n\r", buf);
        }
      }
    }
  }
  else {
    send_to_char("You are not affected by any spell, skill or enchantment.\n\r", ch);
  }
}

void do_time(struct char_data *ch, char *argument, int cmd) {
  long ct;
  char buf[100], *suf, *tmstr;
  int weekday, day;
  extern int REBOOT_AT;
  extern struct time_info_data time_info;
  extern const char *weekdays[];
  extern const char *month_name[];

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
  extern struct weather_data weather_info;
  char buf[100];
  char *sky_look[4]= {
    "cloudless",
    "cloudy",
    "rainy",
    "lit by flashes of lightning"};

  if (OUTSIDE(ch)) {
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
  extern int top_of_helpt;
  extern struct help_index_element *help_index;
  extern char helpcontents[3*MSL];
  extern FILE *help_fl;
  FILE *fl;
  extern char help[MSL];
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
  extern int top_of_wizhelpt;
  extern struct help_index_element *wizhelp_index;
  extern char wizhelpcontents[3*MSL];
  extern FILE *wizhelp_fl;

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
    for (;;) {
      mid = (bot + top) / 2;
      minlen = strlen(argument);
      if (!(chk = strn_cmp(argument, wizhelp_index[mid].keyword, minlen))) {
  fseek(wizhelp_fl, wizhelp_index[mid].pos, 0);
  *buffer = '\0';
  for (;;) {
    if (!fgets(buf, 80, wizhelp_fl)) break;
    if (*buf == '#')
      break;
    strcat(buffer, buf);
    strcat(buffer, "\r");
  }
  page_string(ch->desc, buffer, 1);
  return;
      } else if (bot >= top) {
        send_to_char("There is no help on that word.\n\r", ch);
  return;
      } else if (chk > 0)
  bot = ++mid;
      else
  top = --mid;
    }
    return;
  }

  send_to_char("No help on that command!\n\r", ch);

}

void do_wizhelp(struct char_data *ch, char *argument, int cmd) {
  char buf[MSL];
  int no, i,level,to_level,breakdown=0;
  extern struct command_info cmd_info[];

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
  extern struct command_info cmd_info[];
  extern int top_of_olchelpt;
  extern char olchelpcontents[3*MSL];
  extern struct help_index_element *olchelp_index;
  extern FILE *olchelp_fl;
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

  strcat(buf,"`iOBJS:`q (OLCHELP OBJECTS)\n\r");
  for(no = 1, i = 1;cmd_info[i].cmd_text; i++) {
    if ( GET_LEVEL(ch)>=cmd_info[i].minimum_level &&
         cmd_info[i].minimum_level >= LEVEL_IMM &&
         cmd_info[i].num==CMD_NEWOLCO &&
         IS_SET(ch->new.imm_flags, WIZ_CREATE)) {
      sprintf(buf + strlen(buf), "%-10s", cmd_info[i].cmd_text);
      if (!(no % 7)) strcat(buf, "\n\r");
      no++;
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

char *header_types[] = {
  "--",
  "Mu",
  "Cl",
  "Th",
  "Wa",
  "Ni",
  "No",
  "Pa",
  "Ap",
  "Av",
  "Ba",
  "Co",
};

/* who class not working atm - Ranger Nov 97 */
void do_who(struct char_data *ch, char *argument, int cmd) {
  struct descriptor_data *d;
  char buf[256], name[256], *p;
  char   f = 1,m = 1;
  char  s = 0;
  int  dep = 1,quest=1;
  int   cl = 0;
  int sc=0;
  char  cl_type[3] = "--";
  int count = 0, count2 = 0, pos, level = 1, level2 = LEVEL_IMP;
  char ok;
  extern char *subclass_name[];

  const char *god[] = {
    "IMM",   /* 0 == 51 */
    "DEI",
    "TEM",
    "WIZ",
    "ETE",
    "SUP",
    "IMP"    /* 6 == 57 */
  };

  char *header_types[] = {
    "--",
    "Mu",
    "Cl",
    "Th",
    "Wa",
    "Ni",
    "No",
    "Pa",
    "Ap",
    "Av",
    "Ba",
    "Co",
  };

  for(argument = one_argument(argument, name);*name;
      argument = one_argument(argument, name)) {
    if(*name == '-') {
      for( pos = 1, ok = TRUE; *(name + pos) != '\0' && ok; pos++) {
        switch(*(name + pos)) {
          case 'm':
            f = 0;
            m = 1;
            break;
          case 'f':
            f = 1;
            m = 0;
            break;
          case 'w':
            s = 1;
            break;
          case 's':
            sc = 1;
            break;
          case 'd':
            dep = 0;
            break;
          case 'q':
            quest=0;
            break;
/*          case 'mu':
          cl = 1;
            sprintf(cl_type,"Mu");
          break;
          case 'cl':
            cl = 2;
            sprintf(cl_type,"Cl");
            break;
          case 'th':
        cl = 3;
          sprintf(cl_type,"Th");
        break;
          case 'wa':
        cl = 4;
        sprintf(cl_type,"Wa");
        break;
          case 'ni':
        cl = 5;
        sprintf(cl_type,"Ni");
        break;
          case 'no':
        cl = 6;
        sprintf(cl_type,"No");
        break;
          case 'pa':
        cl = 7;
        sprintf(cl_type,"Pa");
       break;
          case 'ap':
        cl = 8;
        sprintf(cl_type,"Ap");
        break;
          case 'ba':
        cl = 10;
        sprintf(cl_type,"Ba");
        break;
          case 'co':
        cl = 11;
      sprintf(cl_type,"Co");
        break;
*/
          default:
            if(isdigit(*(name + pos))) {
              level2 = MIN(LEVEL_IMP, atoi(name + pos));
            } else {
/*              send_to_char("Usage: who [-m] [-f] [-s] [-d] [-(class)] [number][][-number]\n\r" \
                           "    or who [-[m][f][s][d][(class)][number]] [number][][-number]\n\r" \
                           "Example: who -sf 30-32\n\r" \
                           "         who -m33\n\r" \
                           "         who -th\n\r", ch);*/
              send_to_char("Usage: who [-m] [-f] [-w] [-d] [-q] [-s] [number][][-number]\n\r" \
                           "    or who [-[m][f][w][d][q][s][number]] [number][][-number]\n\r" \
                           "Example: who -wf 30-32\n\r" \
                           "         who -m33\n\r", ch);
              return;
            }
            ok = FALSE;
            break;
        }
      }
    } else {
      if(isdigit(*name)) {
        p = strchr(name, '-');
        if(p) {
          if(isdigit(*(p + 1))) level2 = MIN( LEVEL_IMP, atoi(p + 1));
          *p = '\0';
        }
        level = MAX(1, atoi(name));
      }
    }
  }

  if( level > level2) {
    sprintf(buf, "How you think anybody could be %d-%d level!?\n\r",level,level2);
    send_to_char(buf, ch);
    return;
  }

  send_to_char("Players\n\r-------\n\r",ch);
  if(!s) {
    for (d = descriptor_list; d; d = d->next) {
      if (!d->connected && CAN_SEE(ch, d->character) &&
         (GET_LEVEL(d->character) >= level) && (GET_LEVEL(d->character) <=level2) &&

         ( ((GET_SEX(d->character) == SEX_MALE) && m) ||
           ((GET_SEX(d->character) == SEX_FEMALE) && f) ||
           (GET_SEX(d->character) == SEX_NEUTRAL)   ) &&
          (IS_SET(d->character->specials.pflag, PLR_DEPUTY) || dep) &&
          (IS_SET(d->character->specials.pflag, PLR_QUEST) || quest) &&
          (!strcmp(header_types[cl], cl_type) || !cl) ) {

        if(sc) {
          if(GET_LEVEL(d->character) >= LEVEL_IMM) {
            if(d->character->ver3.subclass)
              sprintf(buf, "[ %s ] %s - %s %d", god[GET_LEVEL(d->character) - LEVEL_IMM],
                    PERS(d->character,ch),subclass_name[d->character->ver3.subclass-1],d->character->ver3.subclass_level);
            else
              sprintf(buf, "[ %s ] %s - None", god[GET_LEVEL(d->character) - LEVEL_IMM],
                    PERS(d->character,ch));
          }
          else {
            if(d->character->ver3.subclass)
              sprintf(buf, "[%2d %2s] %s - %s %d",GET_LEVEL(d->character),
                    header_types[d->character->player.class],
                    PERS(d->character,ch),subclass_name[d->character->ver3.subclass-1],d->character->ver3.subclass_level);
            else
              sprintf(buf, "[%2d %2s] %s - None",GET_LEVEL(d->character),
                    header_types[d->character->player.class],PERS(d->character,ch));
          }
        }
        else {
          if(d->original) { /* If switched */
            if(!CAN_SEE(ch, d->original)) continue;
            sprintf(buf, "[%2d %2s] %s %s", GET_LEVEL(d->original),
                    header_types[d->original->player.class],
                    PERS(d->original,ch),d->original->player.title);
          }
          else if(GET_LEVEL(d->character) >= LEVEL_IMM) {
            sprintf(buf, "[ %s ] %s %s", god[GET_LEVEL(d->character) - LEVEL_IMM],
                    PERS(d->character,ch),d->character->player.title);
          }
          else {
            sprintf(buf, "[%2d %2s] %s %s",GET_LEVEL(d->character),
                    header_types[d->character->player.class],
                    PERS(d->character,ch),d->character->player.title);
          }
        }

        if(IS_SET(d->character->specials.pflag, PLR_DEPUTY))
            strcat(buf, " (Deputy)");

        if(IS_SET(d->character->specials.pflag, PLR_NOMESSAGE))
            strcat(buf, " (deaf)");

        if(IS_AFFECTED(d->character, AFF_INVISIBLE))
            strcat(buf, " (invis)");

        if(IS_AFFECTED(d->character, AFF_IMINV))
            strcat(buf, " (impy)");

        if(IS_SET(d->character->specials.pflag, PLR_KILL))
            strcat(buf, " (killer)");

        if(IS_SET(d->character->specials.pflag, PLR_THIEF))
            strcat(buf, " (thief)");

        if(IS_SET(d->character->specials.pflag, PLR_QUEST) &&
             (GET_LEVEL(d->character)<LEVEL_IMM) )
            strcat(buf, " (quest)");

        if(IS_SET(d->character->new.imm_flags, WIZ_QUEST) &&
             GET_LEVEL(ch) > GET_LEVEL(d->character))
            strcat(buf, " (wq)");

        if (d->character->new.wizinv > 1) {
            send_to_char(buf,ch);
            sprintf(buf, " (i%d)`q\n\r", d->character->new.wizinv);
        }
        else {
          strcat(buf, "`q\n\r");
        }
        send_to_char(buf,ch);
        ++count;
      }
    }
  }
  else {
    for(d = descriptor_list; d; d = d->next) {
      if(!d->connected && CAN_SEE(ch, d->character) &&
         (GET_LEVEL(d->character) >= level) &&
         (GET_LEVEL(d->character) <= level2) &&
         (((GET_SEX(d->character) == SEX_MALE) && m) ||
          ((GET_SEX(d->character) == SEX_FEMALE) && f) ||
           (GET_SEX(d->character) == SEX_NEUTRAL)) &&
          (IS_SET(d->character->specials.pflag, PLR_DEPUTY) || dep) &&
          (IS_SET(d->character->specials.pflag, PLR_QUEST) || quest) &&
          (!strcmp(header_types[cl], cl_type) || !cl)) {

        if(d->original) { /* If switched */
          if(!CAN_SEE(ch, d->original)) continue;
          sprintf(buf, "[%2d %2s] %-25s", GET_LEVEL(d->original),
                  header_types[d->original->player.class],
                  PERS(d->original,ch));
        }
        else if(GET_LEVEL(d->character) >= LEVEL_IMM) {
          sprintf(buf, "[ %s ] %-25s", god[GET_LEVEL(d->character) - LEVEL_IMM],
          PERS(d->character,ch));
        }
        else {
        sprintf(buf, "[%2d %2s] %-25s",GET_LEVEL(d->character),
                header_types[d->character->player.class],
                PERS(d->character,ch));
        }
        ++count2;
        if (count2 == 2) {
          strcat(buf, "\n\r");
          count2 = 0;
        }
        ++count;
        send_to_char(buf,ch);
      }
    }
  }
  sprintf(buf,"\n\rThere are %d visible players connected.\n\r",count);
  send_to_char(buf,ch);
  sprintf(buf,"With a boot time high of %d players.\n\r",max_connects);
  send_to_char(buf,ch);
  if(GET_LEVEL(ch)>=LEVEL_IMM) {
    sprintf(buf,"%d players have connected since boot.\n\r",total_connects);
    send_to_char(buf,ch);
  }
}

extern char *connected_types[];
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
        CAN_SEE(ch, d->character)){

     if(d->connected==CON_PLYNG) timer=d->character->specials.timer;
     else timer=d->timer;
          if(d->original)
      sprintf(line, "#%2d [-- --] %-12s %-20s %2d ", d->descriptor,
              GET_NAME(d->original),connected_types[d->connected],
              timer);
     else
      sprintf(line, "#%2d [%2d %2s] %-12s %-12s %2d ",
              d->descriptor, d->character->player.level,
              header_types[d->character->player.class],
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
  int j;
  bool found=FALSE;

  send_to_char("You are using:\n\r", ch);
  for (j=0; j< MAX_WEAR; j++) {
    if (ch->equipment[j]) {
      found=TRUE;
      if (CAN_SEE_OBJ(ch,ch->equipment[j])) {
        send_to_char(where[j],ch);
        show_obj_to_char(ch->equipment[j],ch,1,0);
      } else {
        send_to_char(where[j],ch);
        send_to_char("Something.\n\r",ch);
        found = TRUE;
      }
      if(j==WIELD && ch->equipment[j]->obj_flags.type_flag==ITEM_2HWEAPON) {
        send_to_char(where[HOLD],ch);
        send_to_char("********\n\r",ch);
        j++;
      }
    }
    else {
      if(GET_LEVEL(ch)<LEVEL_IMM) {
        send_to_char(where[j],ch);
        send_to_char("Nothing.\n\r",ch);
      }
    }
  }
  if(!found && GET_LEVEL(ch)>=LEVEL_IMM) send_to_char("Nothing.\n\r",ch);
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
  char name[MSL],host[50];
  int days, hours, mins, secs;
  int version,class,level,subclass=0,subclass_level;
  struct char_file_u_5 char_info_5;
  struct char_file_u_4 char_info_4;
  struct char_file_u_2 char_info_2;
  extern char *immortal_types[];
  extern char *subclass_name[];

  one_argument( argument, name);

  if(!*name) {
    send_to_char("Whois who ?\n\r", ch);
    return;
  }

  for (d = descriptor_list; d; d = d->next) {
    if(d->character && d->character->player.name &&
       isname(name, d->character->player.name) && CAN_SEE(ch, d->character)) {
      if(GET_LEVEL(d->character) >= LEVEL_IMM && !IS_NPC(d->character)) {
        sprintf(buf, "%s is %s.\n\r",GET_NAME(d->character),
                immortal_types[GET_LEVEL(d->character) - LEVEL_IMM]);
        send_to_char(buf, ch);
      }
      else {
        sprintf(buf, "%s is level %d %s.\n\r", GET_NAME(d->character),
                GET_LEVEL(d->character), pc_class_types[(int)GET_CLASS(d->character)]);
        send_to_char(buf, ch);
      }
      if(d->character->ver3.subclass) {
        sprintf(buf, "Subclass: %s Level: %d.\n\r",subclass_name[d->character->ver3.subclass-1],d->character->ver3.subclass_level);
        send_to_char(buf,ch);
      }
      if(d->host) {
        buf2[0]=0;
        sscanf(&d->host[strlen(d->host)-5],"%*[^.].%s",buf2);
        if(is_number(buf2)) {
          send_to_char("Last logged from numeric address.\n\r", ch);
        } else {
          sprintf(buf, "Last logged from *.%s.\n\r", buf2);
          send_to_char(buf, ch);
        }
      }
      else send_to_char("Last logged from Unknown place.\n\r", ch);
      if(d->character->specials.timer && GET_LEVEL(d->character)<LEVEL_IMM) {
        sprintf(buf, "Idle: %d minute(s).\n\r",d->character->specials.timer);
        send_to_char(buf,ch);
      }
      return;
    }
  }

  string_to_lower(name);
  sprintf(buf,"rent/%c/%s.dat",UPPER(name[0]),name);
  if(!(fl=fopen(buf,"rb"))) {
    send_to_char("You didn't find anyone by that name.\n\r", ch);
    return;
  }

  version=char_version(fl);

  switch(version) {
    case 2:
      memset(&char_info_2,0,sizeof(char_info_2));
      if(fread(&char_info_2, sizeof(struct char_file_u_2), 1, fl) < 1) {
        log_s("Error reading rent file (do_whois).");
        fclose(fl);
        return;
      }
      fclose(fl);
      class=(int)char_info_2.class;
      level=char_info_2.level;
      secs=time(0)-char_info_2.last_update;
      sprintf(host,"%s",char_info_2.new.host);
      break;
    case 3:
      memset(&char_info_4,0,sizeof(char_info_4));
      if(fread(&char_info_4, sizeof(struct char_file_u_4), 1, fl) < 1) {
        log_s("Error reading rent file (do_whois).");
        fclose(fl);
        return;
      }
      fclose(fl);
      class=(int)char_info_4.class;
      subclass=char_info_4.ver3.subclass;
      subclass_level=char_info_4.ver3.subclass_level;
      level=char_info_4.level;
      if(level>=31) level=LEVEL_IMM;
      secs=time(0)-char_info_4.last_update;
      sprintf(host,"%s",char_info_4.new.host);
      break;
    case 4:
      memset(&char_info_4,0,sizeof(char_info_4));
      if(fread(&char_info_4, sizeof(struct char_file_u_4), 1, fl) < 1) {
        log_s("Error reading rent file (do_whois).");
        fclose(fl);
        return;
      }
      fclose(fl);
      class=(int)char_info_4.class;
      subclass=char_info_4.ver3.subclass;
      subclass_level=char_info_4.ver3.subclass_level;
      level=char_info_4.level;
      secs=time(0)-char_info_4.last_update;
      sprintf(host,"%s",char_info_4.new.host);
      break;
    case 5:
      memset(&char_info_5,0,sizeof(char_info_5));
      if(fread(&char_info_5, sizeof(struct char_file_u_5), 1, fl) < 1) {
        log_s("Error reading rent file (do_whois).");
        fclose(fl);
        return;
      }
      fclose(fl);
      class=(int)char_info_5.class;
      subclass=char_info_5.ver3.subclass;
      subclass_level=char_info_5.ver3.subclass_level;
      level=char_info_5.level;
      secs=time(0)-char_info_5.last_update;
      sprintf(host,"%s",char_info_5.new.host);
      break;
    default:
      log_s("Error getting pfile version (do_whois)");
      return;
  }

  sprintf(buf, "%s is a level %d %s.\n\r", name,level,pc_class_types[class]);
  send_to_char (buf, ch);
  if(subclass) {
    sprintf(buf, "Subclass: %s Level: %d.\n\r",subclass_name[subclass-1],subclass_level);
    send_to_char(buf,ch);
  }
  sprintf(buf, "%s isn't in now.\n\r", CAP(name));
  send_to_char(buf, ch);
  days = secs/SECS_PER_REAL_DAY;
  secs -= days*SECS_PER_REAL_DAY;
  hours = secs/SECS_PER_REAL_HOUR;
  secs -= hours*SECS_PER_REAL_HOUR;
  mins = secs/SECS_PER_REAL_MIN;
  secs -= (mins * SECS_PER_REAL_MIN);

  buf2[0]=0;
  sscanf(host+strlen(host)-5,"%*[^.].%s",buf2);
  if(is_number(buf2))
    send_to_char("Originally logged from numeric address.\n\r", ch);
  else sprintf(buf, "Originally logged from *.%s.\n\r", buf2);
  send_to_char(buf, ch);
  sprintf(buf,"Last on %d days, %d hours, %d minute, and %d seconds ago.\n\r",
          days,hours,mins,secs);
  send_to_char(buf, ch);
}
