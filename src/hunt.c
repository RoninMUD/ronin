/*
** hunt.c
**   Routines for hunting
**
** Adapted for RoninMUD by Ranger
**
** Do not distribute without permission.
*/

/* Revision history

Revision 0.00 22-May-03 Ranger
Initial creation - IMP only and non-mob command for testing
the system. Also, limited to only one zone for testing.

*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
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
#include "act.h"
#include "fight.h"
#include "spec_assign.h"
#include "mob.spells.h"
#include "subclass.h"

extern const char *dirs[];

struct hash_link
{
  int			key;
  struct hash_link	*next;
  void			*data;
};

struct hash_header
{
  int			rec_size;
  int			table_size;
  int			*keylist, klistsize, klistlen;
  struct hash_link	**buckets;
};

#define	HASH_KEY(ht,key)((((unsigned int)(key))*17)%(ht)->table_size)

struct hunting_data
{
  char			*name;
  struct char_data	**victim;
};

struct room_q
{
  int		room_nr;
  struct room_q	*next_q;
};

struct nodes
{
  int	visited;
  int	ancestor;
};

void init_hash_table(struct hash_header	*ht,int rec_size,int table_size)
{
  ht->rec_size	= rec_size;
  ht->table_size= table_size;
  ht->buckets	= (void*)calloc(sizeof(struct hash_link**),table_size);
  ht->keylist	= (void*)malloc(sizeof(ht->keylist)*(ht->klistsize=128));
  ht->klistlen	= 0;
}

void destroy_hash_table(struct hash_header *ht,void (*gman)())
{
  int			i;
  struct hash_link	*scan,*temp;

  for(i=0;i<ht->table_size;i++)
    for(scan=ht->buckets[i];scan;)
      {
	temp = scan->next;
	(*gman)(scan->data);
	free(scan);
	scan = temp;
      }
  free(ht->buckets);
  free(ht->keylist);
}

void _hash_enter(struct hash_header *ht,int key,void *data)
{
  /* precondition: there is no entry for <key> yet */
  struct hash_link	*temp;
  int			i;

  temp		= (struct hash_link *)malloc(sizeof(struct hash_link));
  temp->key	= key;
  temp->next	= ht->buckets[HASH_KEY(ht,key)];
  temp->data	= data;
  ht->buckets[HASH_KEY(ht,key)] = temp;
  if(ht->klistlen>=ht->klistsize)
    {
      ht->keylist = (void*)realloc(ht->keylist,sizeof(*ht->keylist)*
				   (ht->klistsize*=2));
    }
  for(i=ht->klistlen;i>=0;i--)
    {
      if(ht->keylist[i-1]<key)
	{
	  ht->keylist[i] = key;
	  break;
	}
      ht->keylist[i] = ht->keylist[i-1];
    }
  ht->klistlen++;
}

void *hash_find(struct hash_header *ht,int key)
{
  struct hash_link *scan;

  scan = ht->buckets[HASH_KEY(ht,key)];

  while(scan && scan->key!=key)
    scan = scan->next;

  return scan ? scan->data : NULL;
}

int hash_enter(struct hash_header *ht,int key,void *data)
{
  void *temp;

  temp = hash_find(ht,key);
  if(temp) return 0;

  _hash_enter(ht,key,data);
  return 1;
}

void donothing()
{
  return;
}

int find_path( int in_room_vnum, int out_room_vnum, CHAR *ch,
	       int depth, int in_zone )
{
  struct room_q		*tmp_q, *q_head, *q_tail;
  struct hash_header	x_room;
  int	i, tmp_room, count=0, thru_doors;
  int herep;
  int startp;

  if ( depth <0 )
    {
      thru_doors = TRUE;
      depth = -depth;
    }
  else
    {
      thru_doors = FALSE;
    }

  startp = real_room(in_room_vnum);

  init_hash_table( &x_room, sizeof(int), 2048 );
  hash_enter( &x_room, in_room_vnum, (void *) - 1 );

  /* initialize queue */
  q_head = (struct room_q *) malloc(sizeof(struct room_q));
  q_tail = q_head;
  q_tail->room_nr = in_room_vnum;
  q_tail->next_q = 0;

  while(q_head)
    {
      herep=real_room( q_head->room_nr );
      /* for each room test all directions */
      if( world[herep].zone == world[startp].zone || !in_zone )
	  {
	  /* only look in this zone */
	  for( i = 0; i <= 5; i++ )
	    {
	      if(world[herep].dir_option[i] &&
	         world[herep].dir_option[i]->to_room_r != NOWHERE &&
	         world[herep].dir_option[i]->to_room_v != 0 &&
	         world[herep].dir_option[i]->to_room_r != herep &&
               !IS_SET(world[herep].dir_option[i]->exit_info, EX_CRAWL) &&
               !IS_SET(world[herep].dir_option[i]->exit_info, EX_JUMP) &&
               !IS_SET(world[herep].dir_option[i]->exit_info, EX_ENTER) &&
               !IS_SET(world[herep].dir_option[i]->exit_info, EX_CLIMB) &&
               (thru_doors || !IS_SET(world[herep].dir_option[i]->exit_info, EX_CLOSED)))
		{
		  /* next room */
		  tmp_room = world[herep].dir_option[i]->to_room_v;
		  if( tmp_room != out_room_vnum )
		    {
		      /* shall we add room to queue ?
			 count determines total breadth and depth */
		      if( !hash_find( &x_room, tmp_room )
			 && ( count < depth ) )
			{
			  count++;
			  /* mark room as visted and put on queue */

			  tmp_q = (struct room_q *)
			    malloc(sizeof(struct room_q));
			  tmp_q->room_nr = tmp_room;
			  tmp_q->next_q = 0;
			  q_tail->next_q = tmp_q;
			  q_tail = tmp_q;

			  /* ancestor for first layer is the direction */
			  hash_enter( &x_room, tmp_room,
				     ((int)hash_find(&x_room,q_head->room_nr)
				      == -1) ? (void*)(i+1)
				     : hash_find(&x_room,q_head->room_nr));
			}
		    }
		  else
		    {
		      /* have reached our goal so free queue */
		      tmp_room = q_head->room_nr;
		      for(;q_head;q_head = tmp_q)
			{
			  tmp_q = q_head->next_q;
			  free(q_head);
			}
		      /* return direction if first layer */
		      if ((int)hash_find(&x_room,tmp_room)==-1)
			{
			  if (x_room.buckets)
			    {
			      /* junk left over from a previous track */
			      destroy_hash_table(&x_room, donothing);
			    }
			  return(i);
			}
		      else
			{
			  /* else return the ancestor */
			  int i;

			  i = (int)hash_find(&x_room,tmp_room);
			  if (x_room.buckets)
			    {
			      /* junk left over from a previous track */
			      destroy_hash_table(&x_room, donothing);
			    }
			  return( -1+i);
			}
		    }
		}
	    }
	}

      /* free queue head and point to next entry */
      tmp_q = q_head->next_q;
      free(q_head);
      q_head = tmp_q;
    }

  /* couldn't find path */
  if( x_room.buckets )
    {
      /* junk left over from a previous track */
      destroy_hash_table( &x_room, donothing );
    }
  return -1;
}


void do_hunt(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];
  CHAR *victim;
  int direction;
  int fArea=0;

  one_argument( argument, arg );

  if(!*arg)
    {
      send_to_char( "Whom are you trying to hunt?\n\r", ch );
      return;
    }

  /* only imps can hunt to different areas - will be = 1 if expand beyond current zone */
  if(!IS_NPC(ch) && GET_LEVEL(ch)==LEVEL_IMP)
    fArea = 0;

  if( fArea )
    victim = get_char_vis( ch, arg );
  else

    victim = get_char_vis_zone( ch, arg );

  if(!victim)
    {
      send_to_char("No-one around by that name.\n\r", ch );
      return;
    }

  if(CHAR_REAL_ROOM(ch) == CHAR_REAL_ROOM(victim))
    {
      act("$N is here!",0, ch, NULL, victim, TO_CHAR );
      return;
    }

  /*
   * Deduct some movement.
   */
  if( GET_MOVE(ch) > 2 )
    GET_MOVE(ch) -= 3;
  else
    {
      send_to_char( "You're too exhausted to hunt anyone!\n\r", ch );
      return;
    }

  act( "$n carefully sniffs the air.",0, ch, NULL, NULL, TO_ROOM );
  WAIT_STATE(ch, PULSE_VIOLENCE);
  direction = find_path( CHAR_VIRTUAL_ROOM(ch), CHAR_VIRTUAL_ROOM(victim),
			ch, -40000, fArea );

  if( direction == -1 )
    {
      act( "You couldn't find a path to $N from here.",0,
	  ch, NULL, victim, TO_CHAR );
      return;
    }

  if( direction < 0 || direction > 5 )
    {
      send_to_char( "Hmm... Something seems to be wrong.\n\r", ch );
      return;
    }

  /*
   * Give a random direction if the player misses the die roll.
   */

/* This left in rough form for the time being.  No skill for HUNT
has been added yet and if added the missed roll would give
a random direction that exists.

The NPC check is immaterial at this point as it is only IMP
useable.  Left in just incase it becomes mob useable without
anyone looking at the code to see what it does.

*/

  if(( IS_NPC (ch) && chance(25)))        /* NPC @ 25% */
    {

	  direction = number(0,5);
/*      do
	{
	  direction = number(0,5);
	}
	tmproom=CHAR_REAL_ROOM(ch);
      while(world[tmproom].dir_option[i]==NULL &&
            world[tmproom].dir_option[i]->to_room_r != NOWHERE &&
	      world[tmproom].dir_option[i]->to_room_v != 0 &&
            !IS_SET(world[tmproom].dir_option[i]->exit_info, EX_CRAWL) &&
            !IS_SET(world[tmproom].dir_option[i]->exit_info, EX_JUMP) &&
            !IS_SET(world[tmproom].dir_option[i]->exit_info, EX_ENTER) &&
            !IS_SET(world[tmproom].dir_option[i]->exit_info, EX_CLIMB))

       ( ch->in_room->exit[direction] == NULL )
	    || ( ch->in_room->exit[direction]->to_room == NULL) );
*/
    }

  /*
   * Display the results of the search.
   */
  sprintf( buf, "$N is %s from here.", dirs[direction] );
  act( buf,0, ch, NULL, victim, TO_CHAR );

  return;
}

/*
This code is meant for a mob to hunt either another mob or
a PC.  It has not been modified from its orignal version and
will not compile under Ronin's code base.

No additions to the main code have been added for a mob to
use hunt.

void hunt_victim( CHAR *ch )
{
  int		dir;
  bool		found;
  CHAR	*tmp;

  if( ch == NULL || ch->hunting == NULL || !IS_NPC(ch) )
    return;

  for( found = 0, tmp = char_list; tmp && !found; tmp = tmp->next )
    if( ch->hunting == tmp )
      found = 1;

  if( !found || !can_see( ch, ch->hunting ) )
    {
      do_say( ch, "Damn!  My prey is gone!!" );
      ch->hunting = NULL;
      return;
    }

  if( ch->in_room == ch->hunting->in_room )
    {
      act( "$n glares at $N and says, 'Ye shall DIE!'",0,
	  ch, NULL, ch->hunting, TO_NOTVICT );
      act( "$n glares at you and says, 'Ye shall DIE!'",0,
	  ch, NULL, ch->hunting, TO_VICT );
      act( "You glare at $N and say, 'Ye shall DIE!",0,
	  ch, NULL, ch->hunting, TO_CHAR);
      multi_hit( ch, ch->hunting, TYPE_UNDEFINED );
      ch->hunting = NULL;
      return;
    }

  WAIT_STATE( ch, skill_table[gsn_hunt].beats );
  dir = find_path( ch->in_room->vnum, ch->hunting->in_room->vnum,
		  ch, -40000, TRUE );

  if( dir < 0 || dir > 5 )
    {
      act( "$n says 'Damn!  Lost $M!'",0, ch, NULL, ch->hunting, TO_ROOM );
      ch->hunting = NULL;
      return;
    }

  if( number_percent () > 75 )
    {
      do
        {
	  dir = number_door();
        }
      while( ( ch->in_room->exit[dir] == NULL )
	    || ( ch->in_room->exit[dir]->to_room == NULL ) );
    }


  if( IS_SET( ch->in_room->exit[dir]->exit_info, EX_CLOSED ) )
    {
      do_open( ch, (char *) dirs[dir] );
      return;
    }

  move_char( ch, dir );
  return;
}
*/
