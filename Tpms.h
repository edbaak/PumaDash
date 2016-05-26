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

  WARNING: Modifying a vehicle's dashboard and instrument panel 
  may require vehicle engineering and re-certification according 
  to local laws and regulations, such as the Australian Design
  Rules (ADR) and Vehicle Standards. This code does not make any 
  claim to meet any such standard. 

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
#include "OBD.h"
#include <string.h>
 
class Tpms
{
  public:
    Tpms();
    void update();
        
  private:
    OBDData *m_FL_Pressure;
    OBDData *m_FL_Temperature;
    
    OBDData *m_FR_Pressure;
    OBDData *m_FR_Temperature;
    
    OBDData *m_RL_Pressure;
    OBDData *m_RL_Temperature;
    
    OBDData *m_RR_Pressure;
    OBDData *m_RR_Temperature;
    
    OBDData *m_TL_Pressure;
    OBDData *m_TL_Temperature;
    
    OBDData *m_TR_Pressure;
    OBDData *m_TR_Temperature;
  
    word m_tirePressureWarningLevel;
    word m_tirePressureAlarmLevel;  
    word m_tireTemperatureWarningLevel;
    word m_tireTemperatureAlarmLevel;  
};

#endif



