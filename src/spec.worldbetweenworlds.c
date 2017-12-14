/*   spec.worldbetweenworlds.c - Specs for World Between Worlds

     Written by Jeffrey Shattuck for RoninMUD
     Last Modification Date: Not finished yet!

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

/* ROOMS */
#define DECILON_1 27231
#define QUEEN_ROOM 27244
#define MIDGAARD_TEMPLE 3001

#define WBW_1 27221
#define INKY 27230
#define EXP_1 27245

#define CLEARING 27250
#define FINAL_ROOM 27298

#define BOTTOM_CLIFF_1 27258
#define BOTTOM_CLIFF_2 27273
#define TOP_CLIFF_1 27287
#define TOP_CLIFF_2 27289

#define GARDEN_EAST 27290
#define GARDEN_WEST 27294
#define GARDEN_SOUTH 27292
#define GARDEN_NORTH 27296

#define FOREST_ROOM_1 27251
#define FOREST_ROOM_2 27252
#define FOREST_ROOM_3 27253
#define FOREST_ROOM_4 27259
#define FOREST_ROOM_5 27260
#define FOREST_ROOM_6 27261
#define FOREST_ROOM_7 27262
#define FOREST_ROOM_8 27272
#define FOREST_ROOM_9 27276
#define FOREST_ROOM_10 27277
#define FOREST_ROOM_11 27278

#define CANYON_ROOM_1 27254
#define CANYON_ROOM_2 27255
#define CANYON_ROOM_3 27256
#define CANYON_ROOM_4 27257
#define CANYON_ROOM_5 27258
#define CANYON_ROOM_6 27273
#define CANYON_ROOM_7 27275
#define CANYON_ROOM_8 27279
#define CANYON_ROOM_9 27280
#define CANYON_ROOM_10 27281

#define RIVER_ROOM_1 27263
#define RIVER_ROOM_2 27264
#define RIVER_ROOM_3 27265
#define RIVER_ROOM_4 27266
#define RIVER_ROOM_5 27267
#define RIVER_ROOM_6 27268
#define RIVER_ROOM_7 27269
#define RIVER_ROOM_8 27270
#define RIVER_ROOM_9 27271
#define RIVER_ROOM_10 27272

#define HILL_ROOM_1 27282
#define HILL_ROOM_2 27283
#define HILL_ROOM_3 27284
#define HILL_ROOM_4 27285
#define HILL_ROOM_5 27286

#define MESA_ROOM_1 27287
#define MESA_ROOM_2 27288
#define MESA_ROOM_3 27289
#define MESA_ROOM_4 27290

#define MOUNTAIN_ROOM_1 27291
#define MOUNTAIN_ROOM_2 27292
#define MOUNTAIN_ROOM_3 27293
#define MOUNTAIN_ROOM_4 27294
#define MOUNTAIN_ROOM_5 27295
#define MOUNTAIN_ROOM_6 27296
#define MOUNTAIN_ROOM_7 27297

/* MOBS */
#define ICE_QUEEN 27239
#define GUARDIAN 27240

#define TALKING_ELK 27202
#define DUMB_ELK 27203
#define FAUN 27204
#define SATYR 27205
#define TALKING_BEAR 27206
#define DUMB_BEAR 27207
#define WALKING_OAK 27208

#define TALKING_ELEPHANT 27209
#define DUMB_ELEPHANT 27210
#define TALKING_TAPIR 27211
#define DUMB_TAPIR 27212
#define TALKING_PANTHER 27213
#define DUMB_PANTHER 27214
#define TALKING_HORSE 27215
#define DUMB_HORSE 27216

#define TALKING_FROG 27217
#define DUMB_FROG 27218
#define RIVER_GOD 27219
#define NAIAD 27220
#define TALKING_BEAVER 27221
#define DUMB_BEAVER 27222
#define TALKING_DONKEY 27223
#define DUMB_DONKEY 27224

#define TALKING_RABBIT 27225
#define DUMB_RABBIT 27226
#define TALKING_MOLE 27227
#define DUMB_MOLE 27228

#define TALKING_JACKDAW 27229
#define TALKING_HE-OWL 27230
#define TALKING_WARTHOG 27231
#define DUMB_WARTHOG 27232

#define COMMON_DWARF 27233
#define CHIEF_DWARF 27234

/* OBJECTS */
#define GREEN_RING 27210
#define YELLOW_RING 27211
#defube APPLE 27217
#define POOL_1 27214
#define POOL_2 27215
#define POOL_3 27216

int mobs_loaded = 0;
int max_mobs = 20;
CHAR *talking_elk, *dumb_elk, *faun, *satyr, *talking_bear, *dumb_bear, *walking_oak, *talking_rabbit, *dumb_rabbit, *dumb_mole, *talking_mole, *talking_frog, *dumb_frog, *river_god, *naiad, *talking_beaver, *dumb_beaver, *talking_donkey, *dumb_donkey, *talking_elephant, *dumb_elephant, *talking_tapir, *dumb_tapir, *talking_panther, *dumb_panther, *talking_horse, *dumb_horse, *talking_jackdaw, *talking_he-owl, *talking_warthog, *dumb_warthog, *common_dwarf, *chief_dwarf, *ice_queen;

/* ROOM SPECS */
/* Heading south in the Queen's Room Loads Queen */
int queen_room_spec(int room, CHAR *ch, int cmd, char *arg) {
	static int triggered = 0;
	if(!ch) return FALSE;
	if(!IS_MORTAL(ch)) return FALSE;
	if(V_ROOM(ch)!=QUEEN_ROOM) return FALSE;
	
	if(cmd == MSG_ZONE_RESET) {
		triggered = 0;
		return FALSE;
	}
	
	if(triggered == 1) return FALSE;
	if(cmd == CMD_SOUTH) {
		triggered = 1;
		act("Your entrance into this room has broken some magical spell! As you turn the\n\rleave the statue of the queen cracks mightily and you are blinded\n\rtemporarily by a bright flash of light!",FALSE,ch,0,0,TO_ROOM);
		ice_queen = read_mobile(ICE_QUEEN, VIRTUAL);
		char_to_room(ice_queen, real_room(V_ROOM(ch)));
		act("The Ice Queen of Decilon rises from her chair looking about.\n\rShe shouts, \"WHO HAS AWAKENED ME?\"",FALSE,ch,0,0,TO_ROOM);
		return TRUE;
	}
	
	return FALSE;
}
		


/* Prevent chars from getting up to the top of the cliff unless they are flying */
int bottom_cliff_spec(int room, CHAR *ch, int cmd, char *arg) {
	
	if(!ch) return FALSE;
	if(GET_LEVEL(ch)>LEVEL_IMM) return FALSE;
	if(V_ROOM(ch)!=BOTTOM_CLIFF_1 || V_ROOM(ch)!=BOTTOM_CLIFF_2) return FALSE;
	
	if (cmd == CMD_EAST) {
		if (GET_POS(ch) == POSITION_FLYING) {
			act("You fly to the top of the cliff and hover above the ground",FALSE,ch,0,0,TO_CHAR);
			act("$n flies up to the top of the cliff",FALSE,ch,0,0,TO_ROOM);
			return FALSE;
		} else {
			act("You can't find any way to scale this cliff.",FALSE,ch,0,0,TO_CHAR);
			return TRUE;
		}
	}
	
	return FALSE;
}

/* If a non flying char goes from top of cliff to bottom of cliff without flying set hps to -1 */
int top_cliff_spec(int room, CHAR *ch, int cmd, char *arg) {
	
	if (!ch) return FALSE;
	if(GET_LEVEL(ch)>LEVEL_IMM) return FALSE;
	if(V_ROOM(ch)!=TOP_CLIFF_1 || V_ROOM(ch)!=TOP_CLIFF_2) return FALSE:
	
	if (cmd == CMD_WEST) {
		if (GET_POS(ch) == POSITION_FLYING) {
			act("You gently soar down over the side of the cliff.",FALSE,ch,0,0,TO_CHAR);
			act("$n gets a running start and glides down over the side of the cliff.",FALSE,ch,0,0,TO_ROOM);
			return FALSE;
		} else {
			act("You approach the edge of the cliff and lose your balance! You fall all the way to the ground below, crippling yourself in the process.",FALSE,ch,0,0,TO_CHAR);
			act("Balance fails $n as $e approaches the edge of the cliff. $n falls to the ground below in a mangled heap.",FALSE,ch,0,0,TO_ROOM);
			if(!IS_NPC(ch)) GET_HP(ch) = -1;
			return FALSE;
		}
	}
	return FALSE;
}

/* Send people to temple with -1hps 1mana 1mv if they try to climb the bushes of the garden */
int garden_room_spec(int room, CHAR *ch, int cmd, char *arg) {
	if (!ch) return FALSE;
	if(!IS_MORTAL(ch) || IS_NPC(ch)) return FALSE;
	char arg[MAX_INPUT_LENGTH];
	one_argument(arg, arg1);
	if(!*arg1) return FALSE;
	if(cmd == CMD_CLIMB && !str_cmp(arg1,"hedge")) {
		act("The ethics of the Old Magic require punishment of those who would steal from the garden!\n\rYou are transported in a flash of light.",FALSE,ch,0,0,TO_CHAR);
		act("As $n attempts to climb the hedge they disappear suddenly in a flash of light!",FALSE,ch,0,0,TO_ROOM);
		GET_HP(ch) = -1;
		GET_MANA(ch) = 1;
		GET_MOVE(ch) = 1;
		char_from_room(ch);
		char_to_room(ch, MIDGAARD_TEMPLE);
		return TRUE;
	}
	
	return FALSE;
}

/* Count unkilled mobs and reset mob counter on MSG_ZONE_RESET */
int clearing_upkeep_spec(int room, CHAR *ch, int cmd, char *arg) {
	if(cmd!=MSG_ZONE_RESET) return FALSE;
	static int counter = 0;
	for(i=27251; i<27299; i++) {
		counter = counter + count_mobs_real_room(i);
	}
	mobs_loaded = mobs_loaded - (20 - counter);
	return FALSE;
}

/* The following specs load the exp mobs in their rooms upon character entry */
int forest_room_spec(int room, CHAR *ch, int cmd, char *arg) {

	static int room_activated_tick = 0;
	
	/* reset this room's counter on tick */
	if(cmd==MSG_TICK && room_activated_tick == 1) {
		room_activated_tick = 0;
		return FALSE;
	}
		
	/* load a mob on character entry */
	if(cmd != MSG_ENTER) return FALSE;
	if(!ch) return FALSE;
	if(GET_LEVEL(ch)>LEVEL_IMM) return FALSE;
	if(IS_NPC(ch)) return FALSE;
	
	if(room_activated_tick==1 || mobs_loaded >= max_mobs) return FALSE;
	room_activated_tick = 1;
	if(number(0,2)==0) { /* 33% chance to load a mob */
		load_mob = number(0,6);
		switch(load_mob) {
			case 0:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Talking Elk!",FALSE,ch,0,0,TO_ROOM);
				talking_elk = read_mobile(TALKING_ELK, VIRTUAL);
				char_to_room(talking_elk, real_room(V_ROOM(ch)));
				break;
			case 1:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Dumb Elk!",FALSE,ch,0,0,TO_ROOM);
				dumb_elk = read_mobile(DUMB_ELK, VIRTUAL);
				char_to_room(dumb_elk, real_room(V_ROOM(ch)));
				break;
			case 2:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Faun!",FALSE,ch,0,0,TO_ROOM);
				faun = read_mobile(FAUN, VIRTUAL);
				char_to_room(faun, real_room(V_ROOM(ch)));
				break;
			case 3:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Satyr!",FALSE,ch,0,0,TO_ROOM);
				satyr = read_mobile(SATYR, VIRTUAL);
				char_to_room(satyr, real_room(V_ROOM(ch)));
				break;
			case 4:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Talking Bear!",FALSE,ch,0,0,TO_ROOM);
				talking_bear = read_mobile(TALKING_BEAR, VIRTUAL);
				char_to_room(talking_bear, real_room(V_ROOM(ch)));
				break;
			case 5:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Dumb Bear!",FALSE,ch,0,0,TO_ROOM);
				dumb_bear = read_mobile(DUMB_BEAR, VIRTUAL);
				char_to_room(talking_bear, real_room(V_ROOM(ch)));
				break;
			case 6:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Walking Oak!",FALSE,ch,0,0,TO_ROOM);
				walking_oak = read_mobile(WALKING_OAK, VIRTUAL);
				char_to_room(walking_oak, real_room(V_ROOM(ch)));
				break;
		}
		mobs_loaded++;
		return FALSE;
	}
	return FALSE;
}

int canyon_room_spec(int room, CHAR *ch, int cmd, char *arg) {
	
	static int room_activated_tick = 0;
	
	/* reset this room's counter on tick */
	if(cmd==MSG_TICK && room_activated_tick == 1) {
		room_activated_tick = 0;
		return FALSE;
	}
	
	/* load a mob on character entry */
	if(cmd != MSG_ENTER) return FALSE;
	if(!ch) return FALSE;
	if(GET_LEVEL(ch)>LEVEL_IMM) return FALSE;
	if(IS_NPC(ch)) return FALSE;
	
	if(room_activated_tick==1 || mobs_loaded >= max_mobs) return FALSE;
	room_activated_tick = 1;
	if(number(0,2)==0) { /* 33% chance to load a mob */
		load_mob = number(0,7);
		switch(load_mob) {
			case 0:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Talking Elephant!",FALSE,ch,0,0,TO_ROOM);
				talking_elephant = read_mobile(TALKING_ELEPHANT, VIRTUAL);
				char_to_room(talking_elephant, real_room(V_ROOM(ch)));
				break;
			case 1:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Dumb Elephant!",FALSE,ch,0,0,TO_ROOM);
				dumb_elephant = read_mobile(DUMB_ELEPHANT, VIRTUAL);
				char_to_room(dumb_elephant, real_room(V_ROOM(ch)));
				break;
			case 2:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Talking Tapir!",FALSE,ch,0,0,TO_ROOM);
				talking_tapir = read_mobile(TALKING_TAPIR, VIRTUAL);
				char_to_room(talking_tapir, real_room(V_ROOM(ch)));
				break;
			case 3:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Dumb Tapir!",FALSE,ch,0,0,TO_ROOM);
				dumb_tapir = read_mobile(DUMB_TAPIR, VIRTUAL);
				char_to_room(dumb_tapir, real_room(V_ROOM(ch)));
				break;
			case 4:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Talking Panther!",FALSE,ch,0,0,TO_ROOM);
				talking_panther = read_mobile(TALKING_PANTHER, VIRTUAL);
				char_to_room(talking_panther, real_room(V_ROOM(ch)));
				break;
			case 5:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Dumb Panther!",FALSE,ch,0,0,TO_ROOM);
				dumb_panther = read_mobile(DUMB_PANTHER, VIRTUAL);
				char_to_room(talking_panther, real_room(V_ROOM(ch)));
				break;
			case 6:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Talking Horse!",FALSE,ch,0,0,TO_ROOM);
				talking_horse = read_mobile(TALKING_HORSE, VIRTUAL);
				char_to_room(talking_horse, real_room(V_ROOM(ch)));
				break;
			case 7:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Dumb Horse!",FALSE,ch,0,0,TO_ROOM);
				dumb_horse = read_mobile(DUMB_HORSE, VIRTUAL);
				char_to_room(dumb_horse, real_room(V_ROOM(ch)));
				break;
		}
		mobs_loaded++;
		return FALSE;
	}
	return FALSE;
}

int river_room_spec(int room, CHAR *ch, int cmd, char *arg) {
	
	static int room_activated_tick = 0;
	
	/* reset this room's counter on tick */
	if(cmd==MSG_TICK && room_activated_tick == 1) {
		room_activated_tick = 0;
		return FALSE;
	}
	
	/* load a mob on character entry */
	if(cmd != MSG_ENTER) return FALSE;
	if(!ch) return FALSE;
	if(GET_LEVEL(ch)>LEVEL_IMM) return FALSE;
	if(IS_NPC(ch)) return FALSE;
	
	if(room_activated_tick==1 || mobs_loaded >= max_mobs) return FALSE;
	room_activated_tick = 1;
	if(number(0,2)==0) { /* 33% chance to load a mob */
		load_mob = number(0,7);
		switch(load_mob) {
			case 0:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Talking Frog!",FALSE,ch,0,0,TO_ROOM);
				talking_frog = read_mobile(TALKING_FROG, VIRTUAL);
				char_to_room(talking_frog, real_room(V_ROOM(ch)));
				break;
			case 1:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Dumb Frog!",FALSE,ch,0,0,TO_ROOM);
				dumb_frog = read_mobile(DUMB_FROG, VIRTUAL);
				char_to_room(dumb_frog, real_room(V_ROOM(ch)));
				break;
			case 2:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles the River God!",FALSE,ch,0,0,TO_ROOM);
				river_god = read_mobile(RIVER_GOD, VIRTUAL);
				char_to_room(river_god, real_room(V_ROOM(ch)));
				break;
			case 3:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Naiad!",FALSE,ch,0,0,TO_ROOM);
				naiad = read_mobile(NAIAD, VIRTUAL);
				char_to_room(naiad, real_room(V_ROOM(ch)));
				break;
			case 4:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Talking Beaver!",FALSE,ch,0,0,TO_ROOM);
				talking_beaver = read_mobile(TALKING_BEAVER, VIRTUAL);
				char_to_room(talking_beaver, real_room(V_ROOM(ch)));
				break;
			case 5:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Dumb Beaver!",FALSE,ch,0,0,TO_ROOM);
				dumb_beaver = read_mobile(DUMB_BEAVER, VIRTUAL);
				char_to_room(talking_beaver, real_room(V_ROOM(ch)));
				break;
			case 6:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Talking Donkey!",FALSE,ch,0,0,TO_ROOM);
				talking_donkey = read_mobile(TALKING_DONKEY, VIRTUAL);
				char_to_room(talking_donkey, real_room(V_ROOM(ch)));
				break;
			case 7:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Dumb Donkey!",FALSE,ch,0,0,TO_ROOM);
				dumb_donkey = read_mobile(DUMB_DONKEY, VIRTUAL);
				char_to_room(dumb_donkey, real_room(V_ROOM(ch)));
				break;
		}
		mobs_loaded++;
		return FALSE;
	}
	return FALSE;
}

int hill_room_spec(int room, CHAR *ch, int cmd, char *arg) {
	
	static int room_activated_tick = 0;
	
	/* reset this room's counter on tick */
	if(cmd==MSG_TICK && room_activated_tick == 1) {
		room_activated_tick = 0;
		return FALSE;
	}
	
	/* load a mob on character entry */
	if(cmd != MSG_ENTER) return FALSE;
	if(!ch) return FALSE;
	if(GET_LEVEL(ch)>LEVEL_IMM) return FALSE;
	if(IS_NPC(ch)) return FALSE;
	
	if(room_activated_tick==1 || mobs_loaded >= max_mobs) return FALSE;
	room_activated_tick = 1;
	if(number(0,2)==0) { /* 33% chance to load a mob */
		load_mob = number(0,5);
		switch(load_mob) {
			case 0:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Talking Rabbit!",FALSE,ch,0,0,TO_ROOM);
				talking_rabbit = read_mobile(TALKING_RABBIT, VIRTUAL);
				char_to_room(talking_rabbit, real_room(V_ROOM(ch)));
				break;
			case 1:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Dumb Rabbit!",FALSE,ch,0,0,TO_ROOM);
				dumb_rabbit = read_mobile(DUMB_RABBIT, VIRTUAL);
				char_to_room(dumb_rabbit, real_room(V_ROOM(ch)));
				break;
			case 2:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Talking Mole!",FALSE,ch,0,0,TO_ROOM);
				talking_mole = read_mobile(TALKING_MOLE, VIRTUAL);
				char_to_room(talking_mole, real_room(V_ROOM(ch)));
				break;
			case 3:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Dumb Mole!",FALSE,ch,0,0,TO_ROOM);
				dumb_mole = read_mobile(DUMB_MOLE, VIRTUAL);
				char_to_room(dumb_mole, real_room(V_ROOM(ch)));
				break;
			case 4:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Dumb Horse!",FALSE,ch,0,0,TO_ROOM);
				dumb_horse = read_mobile(DUMB_HORSE, VIRTUAL);
				char_to_room(dumb_horse, real_room(V_ROOM(ch)));
				break;
			case 5:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Talking Horse!",FALSE,ch,0,0,TO_ROOM);
				talking_horse = read_mobile(TALKING_HORSE, VIRTUAL);
				char_to_room(talking_horse, real_room(V_ROOM(ch)));
				break;
		}
		mobs_loaded++;
		return FALSE;
	}
	return FALSE;
}

int mesa_room_spec(int room, CHAR *ch, int cmd, char *arg) {
	
	static int room_activated_tick = 0;
	
	/* reset this room's counter on tick */
	if(cmd==MSG_TICK && room_activated_tick == 1) {
		room_activated_tick = 0;
		return FALSE;
	}
	
	/* load a mob on character entry */
	if(cmd != MSG_ENTER) return FALSE;
	if(!ch) return FALSE;
	if(GET_LEVEL(ch)>LEVEL_IMM) return FALSE;
	if(IS_NPC(ch)) return FALSE;
	
	if(room_activated_tick==1 || mobs_loaded >= max_mobs) return FALSE;
	room_activated_tick = 1;
	if(number(0,2)==0) { /* 33% chance to load a mob */
		load_mob = number(0,3);
		switch(load_mob) {
			case 0:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Talking Jackdaw!",FALSE,ch,0,0,TO_ROOM);
				talking_jackdaw = read_mobile(TALKING_JACKDAW, VIRTUAL);
				char_to_room(talking_jackdaw, real_room(V_ROOM(ch)));
				break;
			case 1:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Talking He-Owl!",FALSE,ch,0,0,TO_ROOM);
				talking_he-owl = read_mobile(TALKING_HE-OWL, VIRTUAL);
				char_to_room(talking_he-owl, real_room(V_ROOM(ch)));
				break;
			case 2:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Talking Warthog!",FALSE,ch,0,0,TO_ROOM);
				talking_warthog = read_mobile(TALKING_WARTHOG, VIRTUAL);
				char_to_room(talking_warthog, real_room(V_ROOM(ch)));
				break;
			case 3:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Dumb Warthog!",FALSE,ch,0,0,TO_ROOM);
				dumb_warthog = read_mobile(DUMB_WARTHOG, VIRTUAL);
				char_to_room(dumb_warthog, real_room(V_ROOM(ch)));
				break;
		}
		mobs_loaded++;
		return FALSE;
	}
	return FALSE;
}

int mountain_room_spec(int room, CHAR *ch, int cmd, char *arg) {
	
	static int room_activated_tick = 0;
	
	/* reset this room's counter on tick */
	if(cmd==MSG_TICK && room_activated_tick == 1) {
		room_activated_tick = 0;
		return FALSE;
	}
	
	/* load a mob on character entry */
	if(cmd != MSG_ENTER) return FALSE;
	if(!ch) return FALSE;
	if(GET_LEVEL(ch)>LEVEL_IMM) return FALSE;
	if(IS_NPC(ch)) return FALSE;
	
	if(room_activated_tick==1 || mobs_loaded >= max_mobs) return FALSE;
	room_activated_tick = 1;
	if(number(0,2)==0) { /* 33% chance to load a mob */
		load_mob = number(0,5);
		switch(load_mob) {
			case 0:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Common Dwarf!",FALSE,ch,0,0,TO_ROOM);
				common_dwarf = read_mobile(COMMON_DWARF, VIRTUAL);
				char_to_room(common_dwarf, real_room(V_ROOM(ch)));
				break;
			case 1:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles the Chief Dwarf!",FALSE,ch,0,0,TO_ROOM);
				chief_dwarf = read_mobile(CHIEF_DWARF, VIRTUAL);
				char_to_room(chief_dwarf, real_room(V_ROOM(ch)));
				break;
			case 2:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles the Talking Panther!",FALSE,ch,0,0,TO_ROOM);
				talking_panther = read_mobile(TALKING_PANTHER, VIRTUAL);
				char_to_room(talking_panther, real_room(V_ROOM(ch)));
				break;
			case 3:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Dumb Panther!",FALSE,ch,0,0,TO_ROOM);
				dumb_panther = read_mobile(DUMB_PANTHER, VIRTUAL);
				char_to_room(dumb_panther, real_room(V_ROOM(ch)));
				break;
			case 4:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Talking Rabbit!",FALSE,ch,0,0,TO_ROOM);
				talking_rabbit = read_mobile(TALKING_RABBIT, VIRTUAL);
				char_to_room(talking_rabbit, real_room(V_ROOM(ch)));
				break;
			case 5:
				act("The ground before you bubbles like a pot of boiling water. Soon a large hump in the earth forms, the hump breaks and out scambles a Dumb Rabbit!",FALSE,ch,0,0,TO_ROOM);
				dumb_rabbit = read_mobile(DUMB_RABBIT, VIRTUAL);
				char_to_room(talking_rabbit, real_room(V_ROOM(ch)));
				break;
		}
		mobs_loaded++;
		return FALSE;
	}
	return FALSE;
}

/* MOB SPECS */

/* The guardian throws characters out of the garden to either the north, east, south or west. Tank only gets thrown to the east. He also blocks anyone try to 'get apple' */
int guardian_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
	char arg1[MAX_STRING_LENGTH];
	one_argument(arg,arg1);
	if(cmd==CMD_GET && !str_cmp(arg1,"apple")) {
	   do_say(mob, "Not so fast!",CMD_SAY);
	}
	
	if (cmd!=MSG_MOBACT) return FALSE;
	if (number(0,5)==0) {
		tank=mob->specials.fighting;
		vict = get_random_victom_fighting(tank);
		if(vict==tank) {
			act("$n swings an enormous limb and knocks $N out of the garden through the gate!",0,mob,0,vict,TO_NOTVICT);
			act("$n flicks you out of the garden with a mighty swing to the east!",0,mob,0,vict,TO_VICT);
			char_from_room(vict);
			char_to_room(vict,real_room(GARDEN_EAST));
			damage(mob,vict,150,TYPE_UNDEFINED, DAM_NO_BLOCK);
			return FALSE;
		} else {
			rnum = number(0,3);
			switch(rnum) {
				case 0:
					act("The Guardian Tree of Right swings an enormous limb and knocks $n out of the garden through the gate!",0,mob,0,vict,TO_NOTVICT);
					act("The Guardian Tree of Right flicks you out of the garden with a mighty swing to the east!",0,mob,0,vict,TO_VICT);
					char_from_room(vict);
					char_to_room(vict,real_room(GARDEN_EAST));
					damage(mob,vict,150,TYPE_UNDEFINED, DAM_NO_BLOCK);
					return FALSE;
					break;
				case 1:
					act("The Guardian Tree of Right swings an enormous limb and knocks $n out of the garden over the bushes to the north!",0,mob,0,vict,TO_NOTVICT);
					act("The Guardian Tree of Right flicks you out of the garden with a mighty swing to the north!",0,mob,0,vict,TO_VICT);
					char_from_room(vict);
					char_to_room(vict,real_room(GARDEN_NORTH));
					damage(mob,vict,150,TYPE_UNDEFINED, DAM_NO_BLOCK);
					return FALSE;
					break;
				case 2:
					act("The Guardian Tree of Right swings an enormous limb and knocks $n out of the garden over the bushes to the west!",0,mob,0,vict,TO_NOTVICT);
					act("The Guardian Tree of Right flicks you out of the garden with a mighty swing to the west!",0,mob,0,vict,TO_VICT);
					char_from_room(vict);
					char_to_room(vict,real_room(GARDEN_WEST));
					damage(mob,vict,150,TYPE_UNDEFINED, DAM_NO_BLOCK);
					return FALSE;
					break;
				case 3:
					act("The Guardian Tree of Right swings an enormous limb and knocks $n out of the garden over the bushes to the south!",0,mob,0,vict,TO_NOTVICT);
					act("The Guardian Tree of Right flicks you out of the garden with a mighty swing to the south!",0,mob,0,vict,TO_VICT);
					char_from_room(vict);
					char_to_room(vict,real_room(GARDEN_SOUTH));
					damage(mob,vict,150,TYPE_UNDEFINED, DAM_NO_BLOCK);
					return FALSE;
					break;
			}
			return FALSE;
		}
	}
	return FALSE;
}

/* Lion does a bunch of junk:
 Before he teleports, on entrance he says to engage him in combat.
 A few rounds into combat he'll teleport and give quest.
 After teleportation:
 On entrance he checks to see if you have the apple.
 He tells you to throw the apple.
 He blocks throw apple, upon throw his immunities disappear and he tells you to attack him.
 On death he makes sure you know he didn't "die"
 */
int lion_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
	static int triggered=0;
	char arg1[MAX_STRING_LENGTH];
	
	if (cmd==MSG_MOBACT && mob->specials.fighting) {
		if(triggered==0 && number(0,1)==0) {
			do_say(mob,"Good. You have the courage to complete this task. Find a garden in a valley between great mountain ranges past a towering waterfall. There you will find a tree whose fruit will help save this land from the evil you have brought to it. When you have found it, meet me at the river to the north of the forests.",CMD_SAY);
			char_to_room(mob,FINAL_ROOM);
		}
		return FALSE;
	}
	
	if (cmd==MSG_DIE) {
		act("Even though the lion appears dead you hear his voice in your head, \"Well done, your valor is great. Return in the future and I may have a more difficult task worthy of your courage.\"",FALSE,ch,0,0,TO_ROOM);
		return FALSE;
	}
	
	if(GET_LEVEL(ch)>LEVEL_IMM || IS_NPC(ch)) return FALSE;
	if(!ch) return FALSE;
	one_argument(arg,arg1)
	
	if(cmd==MSG_ENTER) {
		if(triggered==0) {
			act("The Lion Sorcerer says, \"Welcome adventurer. Your uninformed use of powerful magic has brought you to this new world and has already damaged the delicate balance of good and evil. Now you must fix this problem. This task will be difficult, prove your courage by engaging me in combat!\"",FALSE,ch,0,0,TO_CHAR);
		} else {
			if(is_carrying_obj(ch,APPLE) || is_wearing_obj(ch,APPLE, HOLD)) {
				do_say(mob,"Well done adventurer. You have found the apple of protection! Now, to protect this world from further evil throw it over into the soft earth near the river.", CMD_SAY);
			}
		}
		return FALSE;
	}
	
	if(cmd==CMD_THROW && triggered==1 && !str_cmp(arg1, " apple")) {
		if(is_wearing_obj(ch,APPLE)) {
			act("You take the apple and toss it gently into the mud near the river.",FALSE,ch,0,0,TO_CHAR);
			act("$n tosses the apple with a nice arc towards the river. The apple settles into the mud quickly.",FALSE,ch,0,0,TO_ROOM);
			apple = get_obj_in_list_num(APPLE, ch->equipped);
			extract_obj(apple);
			act("Before you have time to blink the apple begins to sprout. A few moments later it is a sapling, and in not too long a towering apple tree has grown in the very same spot.",FALSE,ch,0,0,TO_ROOM);
			do_say(mob,"You have completed this task adventurer. In the future I may have more difficult tasks for you. Let me help advance your training by showing you some combat tricks!", CMD_SAY);
			if(IS_SET(mob->specials.immune, IMMUNE_HIT)) REMOVE_BIT(lion->specials.immune, IMMUNE_HIT);
			if(IS_SET(mob->specials.act, ACT_SHIELD)) REMOVE_BIT(lion->specials.act, ACT_SHIELD);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}

/* The following specs simply make the talking animals talk */
int talking_elk_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
	
	if(cmd==MSG_MOBACT && count_mortals_room(mob,TRUE) > 0 && !mob->specials.fighting) {
		if(number(0,5)==0) {
			variable = number(0,2);
			switch(variable) {
				case 0:
					do_say(mob,"I do say, my antlers look rather ravishing in this light!", CMD_SAY);
					break;
				case 1:
					do_say(mob,"Are you feeling like rubbing up against trees as much as I am?", CMD_SAY);
					break;
				case 2:
					do_say(mob,"I hope you can help us free the world of evil like the lion said!", CMD_SAY);
					break;
			}
		}
		
		return FALSE
	}
}

int talking_bear_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
	
	if(cmd==MSG_MOBACT && count_mortals_room(mob,TRUE) > 0 && !mob->specials.fighting) {
		if(number(0,5)==0) {
			variable = number(0,2);
			switch(variable) {
				case 0:
					do_say(mob,"There's a delicious beehive just over yonder, would you like me to save it for you?", CMD_SAY);
					break;
				case 1:
					do_say(mob,"One magic apple can save this whole world? Cool!", CMD_SAY);
					break;
				case 2:
					do_say(mob,"Please help us!", CMD_SAY);
					break;
			}
		}
		
		return FALSE
	}
}

int talking_elephant_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
	
	if(cmd==MSG_MOBACT && count_mortals_room(mob,TRUE) > 0 && !mob->specials.fighting) {
		if(number(0,5)==0) {
			variable = number(0,2);
			switch(variable) {
				case 0:
					do_say(mob,"Us talking elephants have been chosen to help protect this world.", CMD_SAY);
					break;
				case 1:
					do_say(mob,"You! Are you friend or foe?", CMD_SAY);
					break;
				case 2:
					do_say(mob,"Up over the cliff and in the valley the lion said!", CMD_SAY);
					break;
			}
		}
		
		return FALSE
	}
}

int talking_tapir_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
	
	if(cmd==MSG_MOBACT && count_mortals_room(mob,TRUE) > 0 && !mob->specials.fighting) {
		if(number(0,5)==0) {
			variable = number(0,2);
			switch(variable) {
				case 0:
					do_say(mob,"I'm a tapir. I think...", CMD_SAY);
					break;
				case 1:
					do_say(mob,"Be careful, I might eat your dreams!", CMD_SAY);
					break;
				case 2:
					do_say(mob,"I like mud!", CMD_SAY);
					break;
			}
		}
		
		return FALSE
	}
}

int talking_panther_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
	
	if(cmd==MSG_MOBACT && count_mortals_room(mob,TRUE) > 0 && !mob->specials.fighting) {
		if(number(0,5)==0) {
			variable = number(0,2);
			switch(variable) {
				case 0:
					do_say(mob,"While some of us panthers have been given the ability to talk, we don't like to say much.", CMD_SAY);
					break;
				case 1:
					do_say(mob,"Purrr.", CMD_SAY);
					break;
				case 2:
					do_say(mob,"Growl?", CMD_SAY);
					break;
			}
		}
		
		return FALSE
	}
}

int talking_horse_spec(CHAR *mob, CHAR *ch, int cmd, char *arg) {
	
	if(cmd==MSG_MOBACT && count_mortals_room(mob,TRUE) > 0 && !mob->specials.fighting) {
		if(number(0,5)==0) {
			variable = number(0,2);
			switch(variable) {
				case 0:
					do_say(mob,"Do you happen to have a block of sugar for me?", CMD_SAY);
					break;
				case 1:
					do_say(mob,"Wanna race?", CMD_SAY);
					break;
				case 2:
					do_say(mob,"The great lion turned one of our brothers into a winged horse. He flew up over the great cliff!", CMD_SAY);
					break;
			}
		}
		
		return FALSE
	}
}


			
			

/* OBJECT SPECS */

/* Wearing the green ring teleports you to the World Between Worlds */
int green_ring_spec(OBJ *obj,CHAR *ch, int cmd, char *arg) {
	CHAR *vict, *next_vict;
	if(!ch) return FALSE;
	if(!IS_MORTAL(ch) || IS_NPC(ch)) return FALSE;
	
	one_argument(arg,arg1);
	if (!isname(arg1,OBJ_NAME(obj))) return FALSE;
	if (cmd == CMD_WEAR) {
		act("As you slip the ring on you suddenly find the ground beneath you slip away and you are transported to a far off world!",FALSE,ch,0,0,TO_CHAR);
		act("$n puts on a luminescent green ring and you are suddenly transported to a different world!",FALSE,ch,0,0,TO_ROOM);
		extract_obj(obj);
		
		for(vict = world[CHAR_REAL_ROOM(ch)].people; vict; vict = next_vict) {
			next_vict = vict->next_in_room;
			if(IS_MORTAL(vict)) {
				char_from_room(vict);
				char_to_room(vict,real_room(WBW_1));
				do_look(vict,"",0);
				act("$n gently alights on the ground.",0,vict,0,0,TO_ROOM);
			}
		}
		
		return TRUE;
	}
	return FALSE;
}

/* apple suggests you eat it.
 eating the apple sends you to temple with -1hps 1mana 1mv */
int apple_spec(OBJ *obj,CHAR *ch, int cmd, char *arg) {
	if(!(ch = obj->carried_by)) return FALSE;
	
	if(cmd == MSG_TICK) {
		act("The smell of the apple is intoxicating. Surely it can't hurt to just take one bite?",FALSE,ch,0,0,TO_CHAR);
		return FALSE;
	}
	
	if(cmd == CMD_EAT && str_cmp(arg," apple")) {
	   act("The ethics of the Old Magic require those who use the garden's bounty for their own benefit be punished! You are transported in a flash of light!",FALSE,ch,0,0,TO_CHAR);
		act("$n succumbs to the smell of the apple they are carrying and takes a bite! In a flash of light $n disappears.",FALSE,ch,0,0,TO_ROOM);
	   GET_HP(ch) = -1;
	   GET_MANA(ch) = 1;
	   GET_MOVE(ch) = 1;
	   char_from_room(ch);
	   char_to_room(ch, 3001);
	   extract_obj(obj);
	   return TRUE;
	}
	return FALSE;
}
	

/* entering the pools _while wearing yellow ring_ teleports you to various places */
int pool_1_spec(OBJ *obj,CHAR *ch, int cmd, char *arg) {
	CHAR *vict, *next_vict;
	if(!ch) return FALSE;
	if(GET_LEVEL(ch)>LEVEL_IMM || IS_NPC(ch)) return FALSE;
	
	one_argument(arg,arg1)
	if (cmd == CMD_ENTER && !(str_cmp(arg1, "pool"))) {
		if (is_wearing_obj(ch,YELLOW_RING)) {
			for(vict = world[CHAR_REAL_ROOM(ch)].people; vict;vict = next_vict) {
				next_vict = vict->next_in_room;
				if(IS_MORTAL(vict)) {
					act("You step into the pool and are transported to a different world!",FALSE,vict,0,0,TO_CHAR);
					char_from_room(vict);
					char_to_room(vict,real_room(DECILON_1));
					do_look(vict,"",0);
					act("$n gently alights on the ground.",0,vict,0,0,TO_ROOM);
				}
			}
			return TRUE;
		} else {
			act("You wade into the middle of the pool. The water comes up to your ankles, but nothing happens.",FALSE,ch,0,0,TO_CHAR);
			act("$n wades into the middle of the pool. The shallow water only comes up to $s ankles.",FALSE,ch,0,0,TO_CHAR);
			return TRUE;
		}
	}
	return FALSE;
}

int pool_2_spec(OBJ *obj,CHAR *ch, int cmd, char *arg) {
	CHAR *vict=0, *next_vict;
	if(!ch) return FALSE;
	if(GET_LEVEL(ch)>LEVEL_IMM || IS_NPC(ch)) return FALSE;
	
	one_argument(arg,arg1);
	if (cmd == CMD_ENTER && !(str_cmp(arg, "pool"))) {
		if (is_wearing_obj(ch,YELLOW_RING)) {
			for(vict = world[CHAR_REAL_ROOM(ch)].people; vict;vict = next_vict) {
				next_vict = vict->next_in_room;
				if(IS_MORTAL(vict)) {
					act("You step into the pool and are transported to a different world!",FALSE,vict,0,0,TO_CHAR);
					char_from_room(vict);
					char_to_room(vict,real_room(INKY));
					do_look(vict,"",0);
					act("$n gently alights on the ground.",0,vict,0,0,TO_ROOM);
				}
			}
			return TRUE;
		} else {
			act("You wade into the middle of the pool. The water comes up to your ankles, but nothing happens.",FALSE,ch,0,0,TO_CHAR);
			act("$n wades into the middle of the pool. The shallow water only comes up to $s ankles.",FALSE,ch,0,0,TO_CHAR);
			return TRUE;
		}
	}
	return FALSE;
}

int pool_3_spec(OBJ *obj,CHAR *ch, int cmd, char *arg) {
	CHAR *vict=0, *next_vict;
	if(!ch) return FALSE;
	if(GET_LEVEL(ch)>LEVEL_IMM || IS_NPC(ch)) return FALSE;
	
	one_argument(arg,arg1)
	if (cmd == CMD_ENTER && !(str_cmp(arg, "pool"))) {
		if (is_wearing_obj(ch,YELLOW_RING)) {
			for(vict = world[CHAR_REAL_ROOM(ch)].people; vict;vict = next_vict) {
				next_vict = vict->next_in_room;
				if(IS_MORTAL(vict)) {
					act("You step into the pool and are transported to a different world!",FALSE,vict,0,0,TO_CHAR);
					char_from_room(vict);
					char_to_room(vict,real_room(EXP_1));
					do_look(vict,"",0);
					act("$n gently alights on the ground.",0,vict,0,0,TO_ROOM);
				}
			}
			return TRUE;
		} else {
			act("You wade into the middle of the pool. The water comes up to your ankles, but nothing happens.",FALSE,ch,0,0,TO_CHAR);
			act("$n wades into the middle of the pool. The shallow water only comes up to $s ankles.",FALSE,ch,0,0,TO_CHAR);
			return TRUE;
		}
	}
	return FALSE;
}
		
			
void assign_wbw(void) {
	assign_room(QUEEN_ROOM, queen_room_spec);
	
	
	assign_room(BOTTOM_CLIFF_1, bottom_cliff_spec);
	assign_room(BOTTOM_CLIFF_2, bottom_cliff_spec);
	assign_room(TOP_CLIFF_1, top_cliff_spec);
	assign_room(TOP_CLIFF_2, top_cliff_spec);
	assign_room(GARDEN_SOUTH, garden_room_spec);
	assign_room(GARDEN_WEST, garden_room_spec);
	assign_room(GARDEN_NORTH, garden_room_spec);
	assign_room(CLEARING, clearing_upkeep_spec);
	
	assign_room(FOREST_ROOM_1, forest_room_spec);
	assign_room(FOREST_ROOM_2, forest_room_spec);
	assign_room(FOREST_ROOM_3, forest_room_spec);
	assign_room(FOREST_ROOM_4, forest_room_spec);
	assign_room(FOREST_ROOM_5, forest_room_spec);
	assign_room(FOREST_ROOM_6, forest_room_spec);
	assign_room(FOREST_ROOM_7, forest_room_spec);
	assign_room(FOREST_ROOM_8, forest_room_spec);
	assign_room(FOREST_ROOM_9, forest_room_spec);
	assign_room(FOREST_ROOM_10, forest_room_spec);
	assign_room(FOREST_ROOM_11, forest_room_spec);
	
	assign_room(CANYON_ROOM_1, canyon_room_spec);
	assign_room(CANYON_ROOM_2, canyon_room_spec);
	assign_room(CANYON_ROOM_3, canyon_room_spec);
	assign_room(CANYON_ROOM_4, canyon_room_spec);
	assign_room(CANYON_ROOM_5, canyon_room_spec);
	assign_room(CANYON_ROOM_6, canyon_room_spec);
	assign_room(CANYON_ROOM_7, canyon_room_spec);
	assign_room(CANYON_ROOM_8, canyon_room_spec);
	assign_room(CANYON_ROOM_9, canyon_room_spec);
	assign_room(CANYON_ROOM_10, canyon_room_spec);
	
	assign_room(RIVER_ROOM_1, river_room_spec);
	assign_room(RIVER_ROOM_2, river_room_spec);
	assign_room(RIVER_ROOM_3, river_room_spec);
	assign_room(RIVER_ROOM_4, river_room_spec);
	assign_room(RIVER_ROOM_5, river_room_spec);
	assign_room(RIVER_ROOM_6, river_room_spec);
	assign_room(RIVER_ROOM_7, river_room_spec);
	assign_room(RIVER_ROOM_8, river_room_spec);
	assign_room(RIVER_ROOM_9, river_room_spec);
	assign_room(RIVER_ROOM_10, river_room_spec);
	
	assign_room(HILL_ROOM_1, hill_room_spec);
	assign_room(HILL_ROOM_2, hill_room_spec);
	assign_room(HILL_ROOM_3, hill_room_spec);
	assign_room(HILL_ROOM_4, hill_room_spec);
	assign_room(HILL_ROOM_5, hill_room_spec);
	
	assign_room(MESA_ROOM_1, mesa_room_spec);
	assign_room(MESA_ROOM_2, mesa_room_spec);
	assign_room(MESA_ROOM_3, mesa_room_spec);
	assign_room(MESA_ROOM_4, mesa_room_spec);
	
	assign_room(MOUNTAIN_ROOM_1, mountain_room_spec);
	assign_room(MOUNTAIN_ROOM_2, mountain_room_spec);
	assign_room(MOUNTAIN_ROOM_3, mountain_room_spec);
	assign_room(MOUNTAIN_ROOM_4, mountain_room_spec);
	assign_room(MOUNTAIN_ROOM_5, mountain_room_spec);
	assign_room(MOUNTAIN_ROOM_6, mountain_room_spec);
	assign_room(MOUNTAIN_ROOM_7, mountain_room_spec);
	
	assign_mob(GUARDIAN, guardian_spec);
	assign_mob(LION, lion_spec);
	assign_mob(TALKING_ELK, talking_elk_spec);
	assign_mob(TALKING_BEAR, talking_bear_spec);
	assign_mob(TALKING_ELEPHANT, talking_elephant_spec);
	assign_mob(TALKING_TAPIR, talking_tapir_spec);
	assign_mob(TALKING_HORSE, talking_horse_spec);
	
	assign_obj(POOL_1, pool_1_spec);
	assign_obj(POOL_2, pool_2_spec);
	assign_obj(POOL_3, pool_3_spec);
	assign_obj(GREEN_RING, green_ring_spec);
	assign_obj(APPLE, apple_spec);
	
}
