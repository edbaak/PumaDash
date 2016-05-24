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

#ifndef Direction_h
#define Direction_h

#if (ARDUINO >= 100)
  #include "Arduino.h" // for Arduino 1.0
#else
  #include "WProgram.h" // for Arduino 23
#endif

#include "Utils.h"

class Direction
{
  public:
    Direction();
    void update();
    
    word compass();
    word pitch();
    word roll();
    
  private:
    word gps_Compass;
    int gps_Pitch;
    int gps_Roll;
};

#endif



