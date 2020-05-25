/* ************************************************************************
*  file: weather.c , Weather and time module              Part of DIKUMUD *
*  Usage: Performing the clock and the weather                            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#include <stdio.h>
#include <string.h>

#include "structs.h"
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "weather.h"
#include "utility.h"

/* Structs */

const struct season_info_t season_info[] = {
  {
    .season = SEASON_WINTER,
    .season_string = "Winter",
    .months = { 0, 1, 2, 15, 16 },
    .dawn = 8,
    .day = 9,
    .dusk = 17,
    .night = 18
  },
  {
    .season = SEASON_SPRING,
    .season_string = "Spring",
    .months = { 3, 4, 5, 6, -1 },
    .dawn = 7,
    .day = 8,
    .dusk = 19,
    .night = 20
  },
  {
    .season = SEASON_SUMMER,
    .season_string = "Summer",
    .months = { 7, 8, 9, 10, -1 },
    .dawn = 5,
    .day = 6,
    .dusk = 20,
    .night = 21
  },
  {
    .season = SEASON_AUTUMN,
    .season_string = "Autumn",
    .months = { 11, 12, 13, 14, -1 },
    .dawn = 7,
    .day = 8,
    .dusk = 19,
    .night = 20
  }
};

/*
const struct biome_info_t biome_info[] = {
  {
    .biome = BIOME_UNKNOWN,
    .biome_string = "Unknown",
    .temp_min_avg = 0,
    .temp_max_avg = 0,
    .precip_min_avg = 0,
    .precip_max_avg = 0,
    .use_default_weather_info = TRUE
  },
  {
    .biome = BIOME_GRASSLANDS,
    .biome_string = "Grasslands",
    .temp_min_avg = -25,
    .temp_max_avg = 40,
    .precip_min_avg = 50,
    .precip_max_avg = 90,
    .use_default_weather_info = FALSE
  },
  {
    .biome = BIOME_TEMPERATE,
    .biome_string = "Temperate Forest",
    .temp_min_avg = -30,
    .temp_max_avg = 30,
    .precip_min_avg = 30,
    .precip_max_avg = 60,
    .use_default_weather_info = FALSE
  },
  {
    .biome = BIOME_CONIFEROUS,
    .biome_string = "Coniferous Forest",
    .temp_min_avg = -40,
    .temp_max_avg = 25,
    .precip_min_avg = 30,
    .precip_max_avg = 90,
    .use_default_weather_info = FALSE
  },
  {
    .biome = BIOME_BOREAL,
    .biome_string = "Boreal Forest",
    .temp_min_avg = -50,
    .temp_max_avg = 20,
    .precip_min_avg = 30,
    .precip_max_avg = 75,
    .use_default_weather_info = FALSE
  },
  {
    .biome = BIOME_HILLS,
    .biome_string = "Hills",
    .temp_min_avg = -30,
    .temp_max_avg = 30,
    .precip_min_avg = 30,
    .precip_max_avg = 60,
    .use_default_weather_info = FALSE
  },
  {
    .biome = BIOME_MOUNTAINS,
    .biome_string = "Mountains",
    .temp_min_avg = -50,
    .temp_max_avg = 20,
    .precip_min_avg = 30,
    .precip_max_avg = 75,
    .use_default_weather_info = FALSE
  },
  {
    .biome = BIOME_TUNDRA,
    .biome_string = "Tundra",
    .temp_min_avg = -35,
    .temp_max_avg = -5,
    .precip_min_avg = 15,
    .precip_max_avg = 25,
    .use_default_weather_info = FALSE
  },
  {
    .biome = BIOME_DESERT,
    .biome_string = "Desert",
    .temp_min_avg = -20,
    .temp_max_avg = 50,
    .precip_min_avg = 0,
    .precip_max_avg = 20,
    .use_default_weather_info = FALSE
  },
  {
    .biome = BIOME_AQUATIC,
    .biome_string = "Aquatic",
    .temp_min_avg = 0,
    .temp_max_avg = 0,
    .precip_min_avg = 0,
    .precip_max_avg = 0,
    .use_default_weather_info = TRUE
  }
};
*/

/* Code */

void weather_and_time(void) {
  another_hour();

  weather_change();
}

void another_hour(void) {
  time_info.hours++;

  update_sunlight();

  if (time_info.hours == season_info[get_season()].dawn) {
    send_to_outdoor("The sun rises over the horizon, bathing the land in gleaming orange light.\n\r");
  }
  else if (time_info.hours == season_info[get_season()].day) {
    send_to_outdoor("The sun's light spills onto the world.\n\r");
  }
  else if (time_info.hours == season_info[get_season()].dusk) {
    send_to_outdoor("The sun sets on the horizon, cascading in a brilliant purple-red glow.\n\r");
  }
  else if (time_info.hours == season_info[get_season()].night) {
    send_to_outdoor("Night casts its endless shadow across the land.\n\r");
  }

  if (time_info.hours > 23) {
    time_info.hours = 0;
    time_info.day++;

    if (time_info.day > 27) {
      time_info.day = 0;
      time_info.month++;

      if (time_info.month > 16) {
        time_info.month = 0;
        time_info.year++;
      }
    }
  }
}

void update_sunlight(void) {
  if (time_info.hours == season_info[get_season()].dawn) {
    weather_info.sunlight = SUN_RISE;
  }
  else if ((time_info.hours >= season_info[get_season()].day) && (time_info.hours < season_info[get_season()].dusk)) {
    weather_info.sunlight = SUN_LIGHT;
  }
  else if (time_info.hours == season_info[get_season()].dusk) {
    weather_info.sunlight = SUN_SET;
  }
  else {
    weather_info.sunlight = SUN_DARK;
  }
}

void reset_weather(void) {
  weather_info.pressure = 960;

  if (get_season() == SEASON_SUMMER)
    weather_info.pressure += dice(1, 50);
  else
    weather_info.pressure += dice(1, 80);

  weather_info.change = 0;

  if (weather_info.pressure <= 980)
    weather_info.sky = SKY_LIGHTNING;
  else if (weather_info.pressure <= 1000)
    weather_info.sky = SKY_RAINING;
  else if (weather_info.pressure <= 1020)
    weather_info.sky = SKY_CLOUDY;
  else
    weather_info.sky = SKY_CLOUDLESS;
}

void weather_change(void) {
  int diff = 0, change = 0;

  if (get_season() == SEASON_SUMMER)
    diff = weather_info.pressure > 985 ? -2 : 2;
  else
    diff = weather_info.pressure > 1015 ? -2 : 2;

  weather_info.change += (dice(1, 4) * diff) + dice(2, 6) - dice(2, 6);

  weather_info.change = MIN(weather_info.change, 12);
  weather_info.change = MAX(weather_info.change, -12);

  weather_info.pressure += weather_info.change;

  weather_info.pressure = MIN(weather_info.pressure, 1040);
  weather_info.pressure = MAX(weather_info.pressure, 960);

  switch (weather_info.sky) {
    case SKY_CLOUDLESS:
      if (weather_info.pressure < 990)
        change = 1;
      else if (weather_info.pressure < 1010)
        if (dice(1, 4) == 1)
          change = 1;
      break;

    case SKY_CLOUDY:
      if (weather_info.pressure < 970)
        change = 2;
      else if (weather_info.pressure < 990)
        if (dice(1, 4) == 1)
          change = 2;
        else
          change = 0;
      else if (weather_info.pressure > 1030)
        if (dice(1, 4) == 1)
          change = 3;
      break;

    case SKY_RAINING:
      if (weather_info.pressure < 970)
        if (dice(1, 4) == 1)
          change = 4;
        else
          change = 0;
      else if (weather_info.pressure > 1030)
        change = 5;
      else if (weather_info.pressure > 1010)
        if (dice(1, 4) == 1)
          change = 5;
      break;

    case SKY_LIGHTNING:
      if (weather_info.pressure > 1010)
        change = 6;
      else if (weather_info.pressure > 990)
        if (dice(1, 4) == 1)
          change = 6;
      break;

    default:
      change = 0;
      weather_info.sky = SKY_CLOUDLESS;
      break;
  }

  switch (change) {
    case 1:
      printf_to_outdoor("A brisk squall blows in from the %s, signalling a storm in the distance.\n\r", dirs[number(NORTH, WEST)]);

      weather_info.sky = SKY_CLOUDY;
      break;

    case 2:
      printf_to_outdoor("A heavy rain begins to fall.\n\r");

      weather_info.sky = SKY_RAINING;
      break;

    case 3:
      printf_to_outdoor("You can see the sky once more as the clouds roll off the horizon.\n\r");

      weather_info.sky = SKY_CLOUDLESS;
      break;

    case 4:
      printf_to_outdoor("Grim flashes of lightning indicate that you may be in for a storm.\n\r");

      weather_info.sky = SKY_LIGHTNING;
      break;

    case 5:
      printf_to_outdoor("The rain eases off into a light drizzle, and then stops.\n\r");

      weather_info.sky = SKY_CLOUDY;
      break;

    case 6:
      printf_to_outdoor("The lightning seems to have stopped.\n\r");

      weather_info.sky = SKY_RAINING;
      break;
  }
}

int get_season(void) {
  int season = -1;

  for (int i = 0; i < NUMELEMS(season_info); i++) {
    for (int j = 0; (j < NUMELEMS(season_info[i].months)) && (season_info[i].months[j] >= 0); j++) {
      if (time_info.month == season_info[i].months[j]) {
        season = season_info[i].season;
      }
    }
  }

  if (season < 0) {
    log_f("ERROR :: get_season() :: Unable to find season info for month %d.", time_info.month);

    produce_core();
  }

  return season;
}

/*
void reset_zone_weather(int zone_vnum) {
  if (!zone_vnum) return;

  int zone_rnum = real_zone(zone_vnum);

  if (zone_rnum < 0) return;

  zone_table[zone_rnum].weather.pressure = 1013.25;

  double initial_pressure = 0.0;

  switch (zone_table[zone_rnum].terrain_type) {
    case SECT_FIELD:
    case SECT_ARCTIC:
      initial_pressure = (double)number(-1200, 3700) / 100.0;
      break;

    case SECT_FOREST:
    case SECT_HILLS:
      initial_pressure = (double)number(-2000, 2000) / 100.0;
      break;

    case SECT_MOUNTAIN:
    case SECT_DESERT:
      initial_pressure = (double)number(-3700, 1200) / 100.0;
      break;

    default:
      initial_pressure = (double)number(-3700, 3700) / 100.0;
      break;
  }

  switch (get_season()) {
    case SEASON_WINTER:
      initial_pressure += (double)number(0, 1000) / 100.0;
      break;

    case SEASON_SUMMER:
      initial_pressure -= (double)number(0, 1000) / 100.0;
      break;
  }

  zone_table[zone_rnum].weather.pressure += initial_pressure;

  zone_table[zone_rnum].weather.change = 0.0;

  if (zone_table[zone_rnum].weather.pressure <= 980)
    zone_table[zone_rnum].weather.weather = WEATHER_STORM;
  else if (zone_table[zone_rnum].weather.pressure <= 1000)
    zone_table[zone_rnum].weather.weather = WEATHER_RAIN;
  else if (zone_table[zone_rnum].weather.pressure <= 1020)
    zone_table[zone_rnum].weather.weather = WEATHER_OVERCAST;
  else
    zone_table[zone_rnum].weather.weather = WEATHER_CLEAR;

  if (((get_season() == SEASON_WINTER) ||
       (zone_table[zone_rnum].terrain_type == SECT_MOUNTAIN) ||
       (zone_table[zone_rnum].terrain_type == SECT_ARCTIC)) &&
      chance(50)) {
    if (zone_table[zone_rnum].weather.weather == WEATHER_STORM) {
      zone_table[zone_rnum].weather.weather = WEATHER_BLIZZARD;
    }
    else if (zone_table[zone_rnum].weather.weather == WEATHER_RAIN) {
      zone_table[zone_rnum].weather.weather = WEATHER_SNOW;
    }
  }
}
*/

/*
int get_biome(int room) {
  int biome = BIOME_UNKNOWN;

  switch (ROOM_SECTOR_TYPE(room)) {
    case SECT_FIELD:
      biome = BIOME_GRASSLANDS;
      break;

    case SECT_FOREST:
    {
      int zone = real_zone(inzone(ROOM_VNUM(room)));

      if (zone && ((zone_table[zone].terrain_type == SECT_HILLS) || (zone_table[zone].terrain_type == SECT_MOUNTAIN))) {
        biome = BIOME_CONIFEROUS;
      }
      else if (zone && (zone_table[zone].terrain_type == SECT_ARCTIC)) {
        biome = BIOME_BOREAL;
      }
      else {
        biome = BIOME_TEMPERATE;
      }
      break;
    }

    case SECT_HILLS:
      biome = BIOME_HILLS;
      break;

    case SECT_MOUNTAIN:
      biome = BIOME_MOUNTAINS;
      break;

    case SECT_DESERT:
      biome = BIOME_DESERT;
      break;

    case SECT_ARCTIC:
      biome = BIOME_TUNDRA;
      break;

    case SECT_WATER_SWIM:
    case SECT_WATER_NOSWIM:
      biome = BIOME_AQUATIC;
      break;

    default:
      biome = BIOME_UNKNOWN;
      break;
  }

  return biome;
}

void update_zone_terrain_type(int zone_vnum) {
  if (!zone_vnum) return;

  int zone_rnum = real_zone(zone_vnum);

  if (zone_rnum < 0) return;

  int bottom = zone_table[zone_rnum].bottom;
  int top = zone_table[zone_rnum].top;

  int sector_count[SECT_LAST + 1] = { 0 };
  int largest_sector_count = 0;

  for (int i = bottom; i <= top; i++) {
    int sector_type = world[real_room(i)].sector_type;
    int room_flags = world[real_room(i)].room_flags;

    if ((sector_type == SECT_INSIDE) ||
        (sector_type == SECT_CITY) ||
        (sector_type == SECT_WATER_SWIM) ||
        (sector_type == SECT_WATER_NOSWIM) ||
        IS_SET(room_flags, INDOORS)) continue;

    sector_count[sector_type]++;

    if (sector_count[sector_type] > largest_sector_count) {
      largest_sector_count = sector_count[sector_type];
    }
  }

  zone_table[zone_rnum].terrain_type = -1;

  for (int i = 0; (i < NUMELEMS(sector_count)) && (zone_table[zone_rnum].terrain_type < 0); i++) {
    if (sector_count[i] == largest_sector_count) {
      zone_table[zone_rnum].terrain_type = i;
    }
  }
}
*/
