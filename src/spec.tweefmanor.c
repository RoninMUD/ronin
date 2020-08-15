/*   spec.tweef.c - Specs for Tweef Manor by Indigo/Feroz/Kith

     Written by Kith for RoninMUD
     Last Modification Date: August 07, 2017

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
#include "enchant.h"
#include "spec_assign.h"

/* Zone */
#define TWEEF_ZONE              209

/* Rooms */

#define WELL_ROOM             20977
#define DESERT_DOOR_ROOM      20962
#define MANOR_ENTRANCE_ROOM   20961
#define CAGE_ROOM             20913
#define TEINFORT_ROOM         20911
#define TEIN_TUNNEL_A_ROOM    20965
#define TEIN_TUNNEL_B_ROOM    20966
#define TEIN_TUNNEL_C_ROOM    20967
#define PRIESTESS_ROOM        20960
#define TEMPLESTAIRS_G_ROOM   20956
#define TEMPLESTAIRS_W_ROOM   20957
#define TEMPLESTAIRS_E_ROOM   20958
#define KING_QUEEN_ROOM       20952

/* Mobs */
#define TEINFORT              20903
#define PRIESTESS             20921
#define KING                  20923
#define QUEEN                 20924
#define PRINCESS              20900
#define ASTRONOMER            20906
#define SCIENTIST             20912
#define HERO                  20901
#define SMITTER               20904

/* Objects */
#define BUCKET_EMPTY          20908
#define BUCKET_FULL           20912

/* ======================================================================== */
/* ===============================ENCHANTMENTS============================= */
/* ======================================================================== */

/* ======================================================================== */
/* ===============================OBJECT SPECS============================= */
/* ======================================================================== */

/* "fill bucket well" empty is replaced with full                           */
/* they can be holding or carrying the bucket                               */
int tweef_bucket_empty(OBJ *bucket, CHAR *ch, int cmd, char *arg) {
  OBJ *new_bucket;
  char buf[MAX_INPUT_LENGTH];
  bool bReturn = FALSE;

  if(ch && cmd==CMD_FILL && (bucket->carried_by || bucket->equipped_by)) //
  {
    arg = one_argument(arg,buf);
    if(*buf && !strncmp(buf, "bucket", MAX_INPUT_LENGTH)) // fill bucket ...
    {
      if(V_ROOM(ch)==WELL_ROOM) {
        one_argument(arg,buf);
        if(!*buf) send_to_char("Fill the bucket from where?\n\r",ch); // only entered: fill bucket
        else if(!strncmp(buf, "well", MAX_INPUT_LENGTH))
        { // fill bucket well
          act("$n fills a bucket with magic water from the well.",0,ch,0,0,TO_ROOM);
          act("You fill the bucket with magic water from the well.",0,ch,0,0,TO_CHAR);
          new_bucket = read_object(BUCKET_FULL, VIRTUAL);
          extract_obj(bucket);
          obj_to_char(new_bucket,ch);
        } else { // fill bucket <not well>
          send_to_char("You feel there is a better place to fill this bucket.\n\r",ch);
        }
        bReturn = TRUE;
    } else { //not WELL_ROOM
        send_to_char("You feel there is a better place to fill this bucket.\n\r",ch);
        bReturn = TRUE;
      }
    }
  }
  return bReturn;
}


/* "pour bucket sand" to reveal entrance                                    */
/* they can be holding or carrying the bucket                               */
int tweef_bucket_full(OBJ *bucket, CHAR *ch, int cmd, char *arg) {
  char buf[MAX_INPUT_LENGTH];
  bool bReturn = FALSE;

  //if(ch && cmd==CMD_POUR && (bucket==EQ(ch,HOLD)))
  if(ch && cmd==CMD_POUR && (bucket->carried_by || bucket->equipped_by))
  {
    arg = one_argument(arg,buf);
    if(*buf && !strncmp(buf, "bucket", MAX_INPUT_LENGTH)) // pour bucket
    {
      if(V_ROOM(ch)==DESERT_DOOR_ROOM)
      {
        one_argument(arg,buf);
        if(!*buf) send_to_char("Pour the bucket where?\n\r",ch); // only entered: pour bucket
        else if(!strncmp(buf, "sand", MAX_INPUT_LENGTH))
        { // pour bucket sand
          if(world[real_room(DESERT_DOOR_ROOM)].dir_option[DOWN]->to_room_r == -1) //portal not open
          {
            if(IS_SET(EXIT(ch, DOWN)->exit_info, EX_CLOSED)) //we'll be nice and open the door for them
            {
              act("You lift the door open, revealing sand.",0,ch,0,0,TO_CHAR);
              act("$n lifts the door open, revealing sand.",0,ch,0,0,TO_ROOM);
              REMOVE_BIT(world[real_room(DESERT_DOOR_ROOM)].dir_option[DOWN]->exit_info, EX_CLOSED);
            }
            world[real_room(DESERT_DOOR_ROOM)].dir_option[DOWN]->to_room_r = real_room(MANOR_ENTRANCE_ROOM);
            act("You pour the water into the center of the sand filled doorway.",0,ch,0,0,TO_CHAR);
            act("The sand begins to settle as the water soaks in, then solidifies to form a staircase.",0,ch,0,0,TO_CHAR);
            act("$n pours some water into the center of the sand filled doorway.",0,ch,0,0,TO_ROOM);
            act("The sand begins to settle as the water soaks in, then solidifies to form a staircase.",0,ch,0,0,TO_ROOM);
            extract_obj(bucket);
          }
        } else { // pour bucket <not sand>
          send_to_char("You feel there is a better place to pour this magical water.\n\r",ch);
        }
        bReturn = TRUE;
      } else { //not DESERT_DOOR_ROOM
        send_to_char("You feel there is a better place to pour this magical water.\n\r",ch);
        bReturn = TRUE;
      }
    }
  }
  return bReturn;
}

/* ======================================================================== */
/* ================================ROOM SPECS============================== */
/* ======================================================================== */

int tweef_desert_door(int room, CHAR *ch, int cmd, char *arg)
{
  char buf[MAX_INPUT_LENGTH];

  if(world[room].number == DESERT_DOOR_ROOM)
  {
    switch(cmd)
    {
    case MSG_ZONE_RESET: //close door and entrance
      if(world[real_room(DESERT_DOOR_ROOM)].dir_option[DOWN]->to_room_r >= 0)
      {
        send_to_room("One by one, the steps of the staircase rise from below to seal the entrance.\n\r", real_room(DESERT_DOOR_ROOM));
        world[real_room(DESERT_DOOR_ROOM)].dir_option[DOWN]->to_room_r = -1;
        }
      if(!IS_SET(world[real_room(DESERT_DOOR_ROOM)].dir_option[DOWN]->exit_info, EX_CLOSED))
      {
        SET_BIT(world[real_room(DESERT_DOOR_ROOM)].dir_option[DOWN]->exit_info, EX_CLOSED);
        send_to_room("The desert wind picks and slams the door slams.\n\r", real_room(DESERT_DOOR_ROOM));
      }
      break;
    case CMD_LOOK: //"look down" descriptions change by entrance/door status
      arg = one_argument(arg, buf);
      if(V_ROOM(ch)==DESERT_DOOR_ROOM && is_abbrev(buf,"down"))
      {
        if(IS_SET(world[real_room(DESERT_DOOR_ROOM)].dir_option[DOWN]->exit_info, EX_CLOSED))
        {
          world[real_room(DESERT_DOOR_ROOM)].dir_option[DOWN]->general_description = "The door is made of a dark colored wood, it absorbs the light from the blinding sun.\n\r";
          break;
        } else {
          if(world[real_room(DESERT_DOOR_ROOM)].dir_option[DOWN]->to_room_r >= 0) world[real_room(DESERT_DOOR_ROOM)].dir_option[DOWN]->general_description = "A staircase leads down to some sort of entrance. You hear the gurgle of a fountain.\n\r";
          else world[real_room(DESERT_DOOR_ROOM)].dir_option[DOWN]->general_description = "Sand. All you see is sand, it's everywhere and getting into everything.\n\r";
        }
      break;
      }
      break;
    default:
      break;
    }
  }
  return FALSE;
}

/* throw PCs in cage on e/w through circular tunnel rooms*/
int tweef_teinfort_tunnel(int room, CHAR *ch, int cmd, char *arg)
{
  bool bReturn = FALSE;

  if((cmd==CMD_WEST || cmd==CMD_EAST) && IS_MORTAL(ch))
  {
    if(V_ROOM(ch)>=TEIN_TUNNEL_A_ROOM && V_ROOM(ch)<=TEIN_TUNNEL_C_ROOM  && chance(50))
    {
      send_to_char("As you step forward into the darkness, you feel something click under your right foot.\n\r",ch);
      send_to_char("You hear the whirl of ropes and jingle of chains from above and suddenly your consciousness fades...\n\r",ch);
      char_from_room(ch);
      char_to_room(ch, real_room(CAGE_ROOM)); //no do_look() by design
      WAIT_STATE(ch,PULSE_VIOLENCE*1);
      bReturn = TRUE;
    }
  }
  return bReturn;
}

/* ======================================================================== */
/* only one boss kill per repop                                             */
/* teinfort killable during day if priestess alive                          */
/* priestess killable during night if teinfort alive                        */
/* both total recall (mobact) if their room becomes safe during a fight >:) */
/* ======================================================================== */
int tweef_boss_room(int room, CHAR *ch, int cmd, char *arg)
{
  CHAR *temp, *next;
  bool teinfort_life = 0, priestess_life = 0;

  if(world[room].number == TEINFORT_ROOM || world[room].number == PRIESTESS_ROOM)
  {
    if(cmd == MSG_TICK)
    {
      //search for teinfort and priestess
      for(temp = world[real_room(TEINFORT_ROOM)].people; temp; temp = next)
      {
        next = temp->next_in_room;
        if(!IS_NPC(temp) || (V_MOB(temp)!=TEINFORT)) continue;
        teinfort_life = 1;
        break;
      }
      for(temp = world[real_room(PRIESTESS_ROOM)].people; temp; temp = next)
      {
        next = temp->next_in_room;
        if(!IS_NPC(temp) || (V_MOB(temp)!=PRIESTESS)) continue;
        priestess_life = 1;
        break;
      }
      if(IS_NIGHT)
      {
        //night and teinfort alive: priestess killable
        if(teinfort_life) {
          REMOVE_BIT(world[real_room(PRIESTESS_ROOM)].room_flags, SAFE);
          SET_BIT(world[real_room(TEINFORT_ROOM)].room_flags, SAFE); //just incase something got screwy
        }
        //night and teinfort dead, priestess not killable
        if(!teinfort_life) {
          SET_BIT(world[real_room(PRIESTESS_ROOM)].room_flags, SAFE);
          SET_BIT(world[real_room(TEINFORT_ROOM)].room_flags, SAFE); //just incase something got screwy
        }
      } else { //day
        //day and priestess alive: teinfort killable
        if(priestess_life) {
          REMOVE_BIT(world[real_room(TEINFORT_ROOM)].room_flags, SAFE);
          SET_BIT(world[real_room(PRIESTESS_ROOM)].room_flags, SAFE);
        }
        //day and priestess dead, teinfort not killable
        if(!priestess_life) {
          SET_BIT(world[real_room(TEINFORT_ROOM)].room_flags, SAFE);
          SET_BIT(world[real_room(PRIESTESS_ROOM)].room_flags, SAFE);
        }
      }
    }
    if(cmd == MSG_ZONE_RESET) //default boss rooms to safe
    {
      SET_BIT(world[real_room(TEINFORT_ROOM)].room_flags, SAFE);
      SET_BIT(world[real_room(PRIESTESS_ROOM)].room_flags, SAFE);
    }
  }
  return FALSE;
}

/* ======================================================================== */
/* ===============================MOBILE SPECS============================= */
/* ======================================================================== */

/* king heals queen with his LOVE <3 <3 <3 */
int tweef_king(CHAR *king, CHAR *ch, int cmd, char *arg)
{
  CHAR *temp, *next;
  if(cmd==MSG_MOBACT)
  {
    for(temp = world[real_room(KING_QUEEN_ROOM)].people; temp; temp = next)
    {
      next = temp->next_in_room;
      if(!IS_NPC(temp) || (V_MOB(temp)!=QUEEN)) continue;
      //queen found!
      if (temp->specials.fighting && !king->specials.fighting)
      { // join queen's fight
        do_say(king,"It looks like our playtime must wait..",CMD_SAY);
        hit(king,temp->specials.fighting,TYPE_HIT);
      }
      if(affected_by_spell(temp,SPELL_DEBILITATE) && chance(33))
      {
        act("The king tries to heal Queen Larienne with an obscene gesture.",0,king,0,0,TO_ROOM);
        do_say(king,"Only I may make you shudder and convulse!",CMD_SAY);
        do_say(temp,"Yes! Yes!",CMD_SAY);
        act("The queen licks her lips and flashes a knowing glance towards the king.",0,temp,0,0,TO_ROOM);
        affect_from_char(temp,SPELL_DEBILITATE);
      }
      if(affected_by_spell(temp,SPELL_BLINDNESS) && chance(33))
      {
        act("The king tries to heal Queen Larienne with an obscene gesture.",0,king,0,0,TO_ROOM);
        do_say(king,"A day without a knowing glance from you would be unbearable...",CMD_SAY);
        do_say(temp,"Oh, my love! I shall repay you later!",CMD_SAY);
        act("The queen moans softly as she flashes a knowing glance towards the king.",0,temp,0,0,TO_ROOM);
        affect_from_char(temp,SPELL_BLINDNESS);
      }
      if(affected_by_spell(temp,SPELL_WAR_CHANT_DEBUFF) && chance(33))
      {
        act("The king tries to heal Queen Larienne with an obscene gesture.",0,king,0,0,TO_ROOM);
        do_say(king,"We can speak of unspeakable acts later..",CMD_SAY);
        act("The queen cackles with glee as she flashes a knowing glance towards the king.",0,temp,0,0,TO_ROOM);
        affect_from_char(temp, SPELL_WAR_CHANT_DEBUFF);
      }
      if(affected_by_spell(temp,SPELL_CLOUD_CONFUSION) && chance(33))
      {
        act("The king tries to heal Queen Larienne with an obscene gesture.",0,king,0,0,TO_ROOM);
        do_say(king,"You look flustered, darling.",CMD_SAY);
        act("The queen smiles as she blows a kiss towards the king.",0,temp,0,0,TO_ROOM);
        affect_from_char(temp,SPELL_CLOUD_CONFUSION);
      }
      break;
    }
  }
  return FALSE;
}

int tweef_queen(CHAR *queen, CHAR *ch, int cmd, char *arg)
{
  CHAR *temp, *next;
  if(cmd==MSG_MOBACT)
  {
    for(temp = world[real_room(KING_QUEEN_ROOM)].people; temp; temp = next)
    {
      next = temp->next_in_room;
      if(!IS_NPC(temp) || (V_MOB(temp)!=KING)) continue;
      //king found!
      if(temp->specials.fighting && !queen->specials.fighting)
      { //join king's fight
        act("The queen frowns.",0,queen,0,0,TO_ROOM);
        do_say(queen,"Playtime must wait, my pet.",CMD_SAY);
        hit(queen,temp->specials.fighting,TYPE_HIT);
      }
      if(temp->specials.fighting && !IS_SET(temp->specials.affected_by,AFF_FURY))
      {
        act("The queen swats at the king with her riding crop.",0,queen,0,0,TO_ROOM);
        do_say(queen,"Show them your fury, my pet!",CMD_SAY);
        do_say(temp,"Yes, yes, my love!",CMD_SAY);
        act("The king looks full of rage as he comes towards you.",0,temp,0,0,TO_ROOM);
        SET_BIT(temp->specials.affected_by,AFF_FURY);
      }
      break;
    }
  }
  return FALSE;
}

int tweef_teinfort(CHAR *teinfort, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;
  char buf[MAX_INPUT_LENGTH];

  switch(cmd)
  {
    case MSG_MOBACT:
      if(teinfort->specials.fighting)
      {
        if(IS_SET(world[teinfort->in_room_r].room_flags, SAFE))
        {//fighting && room is SAFE
          do_say(teinfort,"You bore me.",CMD_SAY);
          do_say(teinfort,"Come back when Natiana's grace shines bright.",CMD_SAY);
          spell_total_recall(LEVEL_IMM-1,teinfort,ch,SPELL_TYPE_SPELL);
        } else { //fighting && room is !SAFE
          vict = get_random_victim_fighting(teinfort);
          if(vict && chance(15))
          { //toss to cage, will open door if closed, chance for door to be closed after toss
            sprintf(buf,"You bore me %s!",vict->player.name);
            do_say(teinfort,buf,CMD_SAY);
            if(IS_SET(world[teinfort->in_room_r].dir_option[WEST]->exit_info, EX_CLOSED))
            {
              REMOVE_BIT(world[real_room(TEINFORT_ROOM)].dir_option[WEST]->exit_info, EX_CLOSED);
              REMOVE_BIT(world[real_room(CAGE_ROOM)].dir_option[EAST]->exit_info, EX_CLOSED);
              act("$n opens the cage door with a magical gesture.",0,teinfort,0,0,TO_ROOM);
            }
            act("$n grabs $N and tosses them into the cage.",0,teinfort,0,vict,TO_NOTVICT);
            act("$n grabs you and tosses you into the cage.",0,teinfort,0,vict,TO_VICT);
            if(chance(50))
            {
              SET_BIT(world[real_room(TEINFORT_ROOM)].dir_option[WEST]->exit_info, EX_CLOSED);
              SET_BIT(world[real_room(CAGE_ROOM)].dir_option[EAST]->exit_info, EX_CLOSED);
              act("The cage door slams shut.",0,teinfort,0,0,TO_ROOM);
            }
            char_from_room(vict);
            char_to_room(vict,real_room(CAGE_ROOM));
            do_look(vict,"",CMD_LOOK);
          }
        }
      }
      break;
    case MSG_ENTER:
      if(IS_SET(world[teinfort->in_room_r].room_flags, SAFE) && !teinfort->specials.fighting)
      {
        act("\n\rTeinfort laughs loudly at you.",0,ch,0,0,TO_CHAR);
        act("Teinfort says, 'Now is NOT the time mortal,",0,ch,0,0,TO_CHAR);
        act("Teinfort says, 'Natiana's grace is gone... and I grow powerful!'",0,ch,0,0,TO_CHAR);
      } else { //killable
        act("\n\rTeinfort scowls and glares at you.",0,ch,0,0,TO_CHAR);
        act("Teinfort quietly says, 'Natiana's grace may be with you now...'",0,ch,0,0,TO_CHAR);
        act("Teinfort quietly says, 'But, I am confident you will not survive this battle.'",0,ch,0,0,TO_CHAR);
      }
      break;
    default:
      break;
  }
  return FALSE;
}

int tweef_priestess(CHAR *priestess, CHAR *ch, int cmd, char *arg)
{
  CHAR *vict;

  switch(cmd)
  {
    case MSG_MOBACT:
      if(priestess->specials.fighting)
      {
        if(IS_SET(world[priestess->in_room_r].room_flags, SAFE))
        {//fighting && room is SAFE
          do_say(priestess,"Natiana's time is now!",CMD_SAY);
          do_say(priestess,"Natiana will not stand for this violence!",CMD_SAY);
          spell_total_recall(LEVEL_IMM-1,priestess,ch,SPELL_TYPE_SPELL);
        } else { //fighting && room is !SAFE
          vict = get_random_victim_fighting(priestess);
          if (vict && chance(50))
          {
            do_say(priestess,"Natiana will not stand for this violence!",CMD_SAY);
            act("$n sends out a massive wave of energy and $N is knocked out of the temple!",0,priestess,0,vict,TO_NOTVICT);
            act("$n sends out a massive wave of energy and you are knocked out of the temple!",0,priestess,0,vict,TO_VICT);
            char_from_room(vict);
            char_to_room(vict,real_room(number(TEMPLESTAIRS_G_ROOM,TEMPLESTAIRS_E_ROOM)));
            do_look(vict,"",CMD_LOOK);
          }
        }
      }
      break;
    case MSG_ENTER:
      if(IS_SET(world[priestess->in_room_r].room_flags, SAFE) && !priestess->specials.fighting)
      {
        act("\n\rThe Priestess calmly greets you and speaks in a soothing tune.",0,ch,0,0,TO_CHAR);
        act("Natiana's Priestess peacefully says, 'Natiana will not tolerate violence in her temple.",0,ch,0,0,TO_CHAR);
      } else { //killable
        act("\n\rNatiana's Priestess notices you enter the temple and scowls.",0,ch,0,0,TO_CHAR);
        act("The priestess quietly says, 'Teinfort has fooled you, this is not a battle you want to fight.'",0,ch,0,0,TO_CHAR);
      }
      break;
    default:
      break;
  }
  return FALSE;
}

/* the remaining are only to tell the story */
int tweef_princess(CHAR *princess, CHAR *ch, int cmd, char *arg)
{
  if(cmd==MSG_MOBACT && chance(50))
  {
    if(princess->specials.fighting)
    { //fighting
      do_shout(princess,"Smitter! Hero! Save me! Save me!",CMD_SHOUT);
    }
  }
  return FALSE;
}

int tweef_scientist(CHAR *scientist, CHAR *ch, int cmd, char *arg)
{
  static int state = 0;

  if(ch && cmd==MSG_ENTER)
  {
    if(IS_NPC(ch)) return FALSE;
    if(!state)
      {
        state++;
        return FALSE;
      }
  }

  if(cmd==MSG_MOBACT && !scientist->specials.fighting && state) {
    switch (state)
    {
      case 1:
        do_say(scientist,"The blarfs have grown complacent. There must be a cause.",CMD_SAY);
        do_say(scientist,"There must! There must!",CMD_SAY);
        state++;
        break;
      case 2:
        do_say(scientist,"Leinard blames the stars. Maybe he is right...",CMD_SAY);
        state++;
        break;
      case 3:
        do_say(scientist,"My experiments point to an evil demon. Leinard won't listen!",CMD_SAY);
        state++;
        break;
      case 4:
        state = 0;
        break;
      default:
        break;
    }
  }
  if(cmd==MSG_MOBACT && scientist->specials.fighting && chance(50))
  {
    switch (number(0,1))
    {
      case 0:
        do_say(scientist,"PLEASE STOP! I have important work to complete for the King and Queen.",CMD_SAY);
        break;
      case 1:
        do_say(scientist,"I beg you to stop this violence, Leinard and I must save the manor!",CMD_SAY);
        break;
      default:
        break;
    }
  }
  return FALSE;
}

int tweef_astronomer(CHAR *astronomer, CHAR *ch, int cmd, char *arg)
{
  static int state = 0;

  if(ch && cmd==MSG_ENTER)
  {
    if(IS_NPC(ch)) return FALSE;
    if(!state)
      {
        act("The Astronomer nervously glances from the orrey to the sky and back again.'",0,astronomer,0,0,TO_ROOM);
        state++;
        return FALSE;
      }
  }
  if(cmd==MSG_MOBACT && !astronomer->specials.fighting && state)
  {
    switch (state)
    {
    case 1:
      act("Blushing slightly, the Astronomer stares at his feet as he speaks.",0,astronomer,0,0,TO_ROOM);
      do_say(astronomer,"The King and Queen are always.. occupied.",CMD_SAY);
      do_say(astronomer,"I must solve this for them. ",CMD_SAY);
      state++;
      break;
    case 2:
      do_say(astronomer,"Natiana was always with us, but lately her grace waxes and wanes... something has corrupted her.",CMD_SAY);
      state++;
      break;
    case 3:
      do_say(astronomer,"Diminea plays with potions and powders..",CMD_SAY);
      state++;
      break;
    case 4:
      do_say(astronomer,"But, I know, the answers must lie in the sky.",CMD_SAY);
      state++;
      break;
    case 5:
      state = 0;
      break;
    default:
      break;
    }
  }
  return FALSE;
}

int tweef_hero(CHAR *hero, CHAR *ch, int cmd, char *arg)
{
  if(cmd==MSG_MOBACT && hero->specials.fighting && chance(25))
  {
    switch (number(0,2))
    {
    case 0:
      do_say(hero,"Leave us alone.. Us blarfs just want to dance!",CMD_SAY);
      break;
    case 1:
      do_say(hero,"Just dance with me! Natiana will protect the manor.",CMD_SAY);
      break;
    case 2:
      act("The cheery, fat blarf tries to dance away and shouts, 'I love this tune!'",0,hero,0,0,TO_ROOM);
      break;
    default:
      break;
    }
  }
  return FALSE;
}

int tweef_smitter(CHAR *smitter, CHAR *ch, int cmd, char *arg)
{
  if(cmd==MSG_MOBACT && smitter->specials.fighting && chance(25))
  {
    switch (number(0,2))
    {
    case 0:
      act("Belches loudly and stumbles toward you.",0,smitter,0,0,TO_ROOM);
      break;
    case 1:
      do_say(smitter,"These qdays, i pruotect thye beerg!",CMD_SAY);
      break;
    case 2:
      act("An inebriated blarf takes a large gulp from his drink and throws the stein down.",0,smitter,0,0,TO_ROOM);
      break;
    default:
      break;
    }
  }
  return FALSE;
}

/* assignments */
void assign_tweefmanor(void) {
  /* Objects */
  assign_obj(BUCKET_EMPTY,          tweef_bucket_empty);
  assign_obj(BUCKET_FULL,           tweef_bucket_full);
  /* Rooms */
  assign_room(DESERT_DOOR_ROOM,     tweef_desert_door);
  assign_room(TEINFORT_ROOM,        tweef_boss_room);
  assign_room(PRIESTESS_ROOM,       tweef_boss_room);
  assign_room(TEIN_TUNNEL_A_ROOM,   tweef_teinfort_tunnel);
  assign_room(TEIN_TUNNEL_B_ROOM,   tweef_teinfort_tunnel);
  assign_room(TEIN_TUNNEL_C_ROOM,   tweef_teinfort_tunnel);
  /* Mobs */
  assign_mob(SCIENTIST,             tweef_scientist);
  assign_mob(ASTRONOMER,            tweef_astronomer);
  assign_mob(PRIESTESS,             tweef_priestess);
  assign_mob(TEINFORT,              tweef_teinfort);
  assign_mob(KING,                  tweef_king);
  assign_mob(QUEEN,                 tweef_queen);
  assign_mob(PRINCESS,              tweef_princess);
  assign_mob(SMITTER,               tweef_smitter);
  assign_mob(HERO,                  tweef_hero);
}
