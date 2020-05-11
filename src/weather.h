/* ************************************************************************
*  file: weather.h , Weather and time module              Part of DIKUMUD *
*  Usage: Performing the clock and the weather                            *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#ifndef __WEATHER_H__
#define __WEATHER_H__

#include <stdint.h>

/*
struct weather_info_t {
  double pressure;
  double change;
  uint8_t weather;
};

#define WEATHER_CLEAR    0
#define WEATHER_OVERCAST 1
#define WEATHER_RAIN     2
#define WEATHER_STORM    3
#define WEATHER_SNOW     4
#define WEATHER_BLIZZARD 5
*/

#define SEASON_WINTER 0
#define SEASON_SPRING 1
#define SEASON_SUMMER 2
#define SEASON_AUTUMN 3

struct season_info_t {
  uint8_t season;
  char *season_string;
  uint8_t months[5];
  uint8_t dawn, day, dusk, night;
};

void weather_and_time(void);
void another_hour(void);
void update_sunlight(void);
void reset_weather(void);
void weather_change(void);

int get_season(void);

//void reset_zone_weather(int zone_vnum);

#define BIOME_UNKNOWN    0
#define BIOME_GRASSLANDS 1
#define BIOME_TEMPERATE  2
#define BIOME_CONIFEROUS 3
#define BIOME_BOREAL     4
#define BIOME_HILLS      5
#define BIOME_MOUNTAINS  6
#define BIOME_TUNDRA     7
#define BIOME_DESERT     8
#define BIOME_AQUATIC    9

struct biome_info_t {
  uint8_t biome;
  char *biome_string;
  int8_t temp_min_avg, temp_max_avg;      /* Celcius */
  int16_t precip_min_avg, precip_max_avg; /* Centimeters */
  bool use_default_weather_info;
};

int get_biome(int room);
void update_zone_terrain_type(int zone_vnum);

#endif /* __WEATHER_H__ */
