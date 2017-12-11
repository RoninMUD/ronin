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

/* Prototypes */
void weather_and_time(int mode);
void another_hour(int mode);
void weather_change(void);

/* Code */

void weather_and_time(int mode)
{
  another_hour(mode);

  if (mode)
    weather_change();
}

void another_hour(int mode)
{
  time_info.hours++;

  if (mode)
  {
    switch (time_info.hours)
    {
      case 5:
          send_to_outdoor("The sun rises over the horizon, bathing the land in gleaming orange light.\n\r");
          weather_info.sunlight = SUN_RISE;
          break;
      case 6:
          send_to_outdoor("The sun's light spills onto the world.\n\r");
          weather_info.sunlight = SUN_LIGHT;
          break;
      case 21:
          send_to_outdoor("The sun sets on the horizon, cascading in a brilliant purple-red glow.\n\r");
          weather_info.sunlight = SUN_SET;
          break;
      case 22:
          send_to_outdoor("Night casts its endless shadow across the land.\n\r");
          weather_info.sunlight = SUN_DARK;
          break;
      default:
        break;
    }
  }

  if (time_info.hours > 23)
  {
    time_info.hours -= 24;
    time_info.day++;

    if (time_info.day > 27)
    {
      time_info.day = 0;
      time_info.month++;

      if (time_info.month > 16)
      {
        time_info.month = 0;
        time_info.year++;
      }
    }
  }
}

void weather_change(void)
{
  int diff = 0, change = 0;
  char buf[MSL];

  if (time_info.month >= 7 && time_info.month <= 11)
    diff = weather_info.pressure > 985 ? -2 : 2;
  else
    diff = weather_info.pressure > 1015 ? -2 : 2;

  weather_info.change += (dice(1, 4) * diff) + dice(2, 6) - dice(2, 6);

  weather_info.change = MIN(weather_info.change, 12);
  weather_info.change = MAX(weather_info.change, -12);

  weather_info.pressure += weather_info.change;

  weather_info.pressure = MIN(weather_info.pressure, 1040);
  weather_info.pressure = MAX(weather_info.pressure, 960);

  switch (weather_info.sky)
  {
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
        if(dice(1, 4) == 1)
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

  switch(change)
  {
    case 0:
      break;

    case 1:
      sprintf(buf, "A brisk squall blows in from the %s, signalling a storm in the distance.\n\r", dirs[number(0, 3)]);
      send_to_outdoor(buf);
      weather_info.sky = SKY_CLOUDY;
      break;

    case 2:
      send_to_outdoor("A heavy rain begins to fall.\n\r");
      weather_info.sky = SKY_RAINING;
      break;

    case 3:
      send_to_outdoor("You can see the sky once more as the clouds roll off the horizon.\n\r");
      weather_info.sky = SKY_CLOUDLESS;
      break;

    case 4:
      send_to_outdoor("Grim flashes of lightning indicate that you may be in for a storm.\n\r");
      weather_info.sky = SKY_LIGHTNING;
      break;

    case 5:
      send_to_outdoor("The rain eases off into a light drizzle, and then stops.\n\r");
      weather_info.sky = SKY_CLOUDY;
      break;

    case 6:
      send_to_outdoor("The lightning seems to have stopped.\n\r");
      weather_info.sky = SKY_RAINING;
      break;

    default:
      break;
  }
}
