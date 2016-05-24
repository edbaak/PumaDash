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

/*
	A TPMS system for a Land Rover Defender 110 Puma MY 12.
	No guearantees whatsoever that it works. Other vehicles are even 
	more doubtfull, you're on your own.
*/
 
#ifndef Tpms_h
#define Tpms_h
 
#if (ARDUINO >= 100)
	#include "Arduino.h" // for Arduino 1.0
#else
	#include "WProgram.h" // for Arduino 23
#endif

#include "Utils.h"
#include <string.h>

#define FRONT_LEFT 0
#define FRONT_RIGHT 1
#define REAR_LEFT 2
#define REAR_RIGHT 3
#define TRAILER_LEFT 4
#define TRAILER_RIGHT 5
#define VEHICLE_SPARE 6
#define TRAILER_SPARE 7

#define MAX_TIRES 6
 
class Tpms
{
  public:
    Tpms();
    void update();
    
    byte tirePressure(byte tirePosition);
    bool tirePressureWarning(byte tirePosition);
    bool tirePressureAlarm(byte tirePosition);
    
    byte tireTemperature(byte tirePosition);
    bool tireTemperatureWarning(byte tirePosition);
    bool tireTemperatureAlarm(byte tirePosition);
  
  private:
    word m_tirePressures[6];
    word m_tireTemperatures[6];
    word m_tirePressureWarningLevel;
    word m_tirePressureAlarmLevel;  
    word m_tireTemperatureWarningLevel;
    word m_tireTemperatureAlarmLevel;  
    
};

#endif



