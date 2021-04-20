/*spec.HappyBob.c - Specs for Happy Bob's BBQ by Arodtanjoe/Fisher

     Written by Arodtanjoe/Fisher for RoninMUD
     Last Modification Date: 4/19/2021

     Basic Specs for the mobs and rooms in the zone.
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
#include "spec_assign.h"
#include "mob.spells.h"

/*Rooms */
#define HAPPY_BOB_OFFICE 14937
#define BATTERED_PSYCHE 14950

/*Mobs */
#define HAPPY_BOB 14900
#define MEAN_HOSTESS 14901
#define ROWDY_PATRON 14902
#define OVERWORKED_WAITER 14903
#define SWEATY_PITBOSS 14906

//Have the room close the new area off on reset.
int hb_happybob_office(int room, CHAR *ch, int cmd, char *arg)
{
  if (cmd == MSG_ZONE_RESET)
  {
    if (world[real_room(HAPPY_BOB_OFFICE)].dir_option[DOWN]->to_room_r != -1)
    {
      world[real_room(HAPPY_BOB_OFFICE)].dir_option[DOWN]->to_room_r = -1;
      world[real_room(BATTERED_PSYCHE)].dir_option[UP]->to_room_r = -1;
      send_to_room("That psychiatrist really worked her magic.", real_room(HAPPY_BOB_OFFICE));
      send_to_room("Wow, Happy Bob's mind heals itself.", real_room(BATTERED_PSYCHE));
    }
  }

  return FALSE;
}

int hb_happybob(CHAR *happybob, CHAR *ch, int cmd, char *arg)
{
  //These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(happybob, TRUE) < 1) return FALSE;

  char *happybob_speak[5] = { "Welcome to Happy Bob's. The Best BBQ in the world.", "The sauce is finger licking good.", "Have you tried our ribs yet? The pigs are super fresh.", "BBQ Sauce is good for the soul", "Please go back to the front and wait for your table. Its worth the wait here." };
  char *happybob_death_speak[4] = { "No...dont eat me mashed potatoes", "Cornbread!  What did i ever do to you?", "Baked Beans are scary! Run if you can.", "Stupid chickens, i saved you. Dont peck my eyes out." };

  switch (cmd)
  {
    case MSG_MOBACT:

      //Have him to chat to you.
      if (chance(40))
      {
        sprintf(buf, "%s", happybob_speak[number(0, NUMELEMS(happybob_speak) - 1)]);

        do_say(happybob, buf, CMD_SAY);
      }

      break;

    case MSG_DIE:  // on boss death reward AQP
      sprintf(buf, "%s", happybob_death_speak[number(0, NUMELEMS(happybob_death_speak) - 1)]);
      do_say(happybob, buf, CMD_SAY);

      send_to_room("Happy Bob's death has opened a door into his mind. \n\r", real_room(HAPPY_BOB_OFFICE));
      send_to_room("Happy Bob is alive once again.\n\r", real_room(BATTERED_PSYCHE));
      world[real_room(HAPPY_BOB_OFFICE)].dir_option[DOWN]->to_room_r = real_room(BATTERED_PSYCHE);
      world[real_room(BATTERED_PSYCHE)].dir_option[UP]->to_room_r = real_room(HAPPY_BOB_OFFICE);

      break;
  }

  return FALSE;
}

int hb_hostess(CHAR *hostess, CHAR *ch, int cmd, char *arg)
{
  //These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];

  //Define any other variables

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(hostess, TRUE) < 1) return FALSE;

  char *hostess_speak[4] = { "Please sit back down. We will call you when your table is ready.", "Sorry, your wait time is 60 minutes.", "Please enjoy this coloring book while you wait.", "The pitboss is super gross." };
  char *hostess_death_speak[3] = { "Fine, you will never get a table.", "Enjoy your crap food!", "BBQ is the worst." };
  switch (cmd)
  {
    case MSG_MOBACT:

      //Have him to chat to you.
      if (chance(40))
      {
        sprintf(buf, "%s", hostess_speak[number(0, NUMELEMS(hostess_speak) - 1)]);

        do_say(hostess, buf, CMD_SAY);
      }

      break;

    case MSG_DIE:  // on boss death reward AQP
      sprintf(buf, "%s", hostess_death_speak[number(0, NUMELEMS(hostess_death_speak) - 1)]);

      do_say(hostess, buf, CMD_SAY);

      break;
  }

  return FALSE;
}

int hb_patron(CHAR *patron, CHAR *ch, int cmd, char *arg)
{
  //These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(patron, TRUE) < 1) return FALSE;

  char *patron_speak[4] = { "Waiter..more beer please.", "Where is my food! I am hungry now.", "Oh yes, i love this sauce.", "Bah, waiting is so boring. " };
  char *patron_death_speak[3] = { "Just...one..more..rib...", "Bury me with the pigs!", "I'll see you in BBQ heaven." };

  switch (cmd)
  {
    case MSG_MOBACT:

      //Have him to chat to you.
      if (chance(40))
      {
        sprintf(buf, "%s", patron_speak[number(0, NUMELEMS(patron_speak) - 1)]);

        do_say(patron, buf, CMD_SAY);
      }

      break;

    case MSG_DIE:  // on boss death reward AQP
      sprintf(buf, "%s", patron_death_speak[number(0, NUMELEMS(patron_death_speak) - 1)]);

      do_say(patron, buf, CMD_SAY);

      break;
  }

  return FALSE;
}

int hb_waiter(CHAR *waiter, CHAR *ch, int cmd, char *arg)
{
  //These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(waiter, TRUE) < 1) return FALSE;

  char *waiter_speak[4] = { "Alright Table 5 needs Ribs and Table 3 needs...what?", "Yes, I'll be right there.", "I can't take this abuse anymore", "Man, I really need to quit." };
  char *waiter_death_speak[3] = { "Finally, I am free.", "Thank you. See you again soon.", "I'm glad you had a wonderful evening." };

  switch (cmd)
  {
    case MSG_MOBACT:

      //Have him to chat to you.
      if (chance(40))
      {
        sprintf(buf, "%s", waiter_speak[number(0, NUMELEMS(waiter_speak) - 1)]);

        do_say(waiter, buf, CMD_SAY);
      }

      break;

    case MSG_DIE:  // on boss death reward AQP
      sprintf(buf, "%s", waiter_death_speak[number(0, NUMELEMS(waiter_death_speak) - 1)]);

      do_say(waiter, buf, CMD_SAY);

      break;
  }

  return FALSE;
}

int hb_pitboss(CHAR *pitboss, CHAR *ch, int cmd, char *arg)
{
  //These are default declarations to give variables to characters.
  char buf[MAX_STRING_LENGTH];

  /*Don't waste any more CPU time if no one is in the room. */
  if (count_mortals_room(pitboss, TRUE) < 1) return FALSE;

  char *pitboss_speak[4] = { "Smoke em if you got em.", "Here piggy piggy piggy.", "Man, my sweat is delicious.", "I love Happy Bob. Best job I ever had." };
  char *pitboss_death_speak[3] = { "But who will cook the meat!", "Please enjoy the beef at my funeral.", "Bury me in the sauce. It's that good." };

  switch (cmd)
  {
    case MSG_MOBACT:

      //Have him to chat to you.
      if (chance(40))
      {
        sprintf(buf, "%s", pitboss_speak[number(0, NUMELEMS(pitboss_speak) - 1)]);

        do_say(pitboss, buf, CMD_SAY);
      }

      break;

    case MSG_DIE:  // on boss death reward AQP
      sprintf(buf, "%s", pitboss_death_speak[number(0, NUMELEMS(pitboss_death_speak) - 1)]);

      do_say(pitboss, buf, CMD_SAY);

      break;
  }

  return FALSE;
}

//Assign Spec for the zone. Sets all other specs.

void assign_happybob(void)
{
  /*Objects */
  //assign_obj(BUCKET_EMPTY,          tweef_bucket_empty);

  /*Rooms */
  assign_room(HAPPY_BOB_OFFICE, hb_happybob_office);

  /*Mobs */

  assign_mob(HAPPY_BOB, hb_happybob);
  assign_mob(MEAN_HOSTESS, hb_hostess);
  assign_mob(ROWDY_PATRON, hb_patron);
  assign_mob(OVERWORKED_WAITER, hb_waiter);
  assign_mob(SWEATY_PITBOSS, hb_pitboss);

}