/*
  2016 Copyright (c) Ed Baak  All Rights Reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License 
  as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this code; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-
  1301  USA
*/

#include "Utils.h"
#include "Position.h"

Direction::Direction()
{
  gps_Compass = 0;
  gps_Pitch = 0;
  gps_Roll = 0;
}

void Direction::update()
{
#ifdef VEHICLEDASH_DEBUG
  static byte slowdown_counter = 0;
  slowdown_counter++;
  if (slowdown_counter == 10) {
    slowdown_counter = 0;
    gps_Compass++;
    if (gps_Compass > 360)
      gps_Compass = 0;
  }

  static int debug_pitch_incrementer = 1;
  gps_Pitch+= debug_pitch_incrementer;
  if (gps_Pitch > 44)
    debug_pitch_incrementer = -1;
  else if (gps_Pitch < -44)
    debug_pitch_incrementer = 1;

  static int debug_roll_incrementer = 1;
  gps_Roll+= debug_roll_incrementer;
  if (gps_Roll > 44)
    debug_roll_incrementer = -1;
  else if (gps_Roll < -44)
    debug_roll_incrementer = 1;
#endif // VEHICLEDASH_DEBUG
}

word Direction::compass()
{
  return gps_Compass;
}

word Direction::pitch()
{
  return gps_Pitch;
}

word Direction::roll()
{
  return gps_Roll;  
}




