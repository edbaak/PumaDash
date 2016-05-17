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
#include "Tpms.h"

#if (ARDUINO >= 100)
	#include "Arduino.h" // for Arduino 1.0
#else
	#include "WProgram.h" // for Arduino 23
#endif

Tpms::Tpms()
{
  for (int i=0; i< MAX_TIRES; i++) {
    m_tirePressures[i] = 40;
    m_tireTemperatures[i] = 27;
  }
  m_tirePressureWarningLevel = 30;
  m_tirePressureAlarmLevel = 24;  
  m_tireTemperatureWarningLevel = 35;
  m_tireTemperatureAlarmLevel = 45;  
}

void Tpms::update()
{
  #ifdef VEHICLEDASH_DEBUG
  static byte slowdown_counter = 0;
  slowdown_counter++;
  if (slowdown_counter == 20) {
    slowdown_counter = 0;
    if (m_tirePressures[0] > 28) {
      for (int i = 0; i<4; i++)
        m_tirePressures[i]--;
    } else if (m_tirePressures[1] > 18)
      m_tirePressures[1] --;

    if (m_tirePressures[0] == 28) {
      if (m_tireTemperatures[0] < 35) {
        for (int i = 0; i<4; i++)
          m_tireTemperatures[i]++;
      } else if (m_tireTemperatures[1] < 50)
          m_tireTemperatures[1]++;
    }
  }
  #endif
}

byte Tpms::tirePressure(byte tirePosition)
{
  if (tirePosition < MAX_TIRES)
    return m_tirePressures[tirePosition];
  return 0;
}

bool Tpms::tirePressureWarning(byte tirePosition)
{
  if (tirePosition < MAX_TIRES &&
      m_tirePressures[tirePosition] <= m_tirePressureWarningLevel &&
      m_tirePressures[tirePosition] > m_tirePressureAlarmLevel)
      return true;
  return false;
}

bool Tpms::tirePressureAlarm(byte tirePosition)
{
  if (tirePosition < MAX_TIRES &&
      m_tirePressures[tirePosition] <= m_tirePressureAlarmLevel)
      return true;
  return false;
}

byte Tpms::tireTemperature(byte tirePosition)
{
  if (tirePosition < MAX_TIRES)
    return m_tireTemperatures[tirePosition];
  return 0;
}

bool Tpms::tireTemperatureWarning(byte tirePosition)
{
  if (tirePosition < MAX_TIRES &&
      m_tireTemperatures[tirePosition] >= m_tireTemperatureWarningLevel &&
      m_tireTemperatures[tirePosition] < m_tireTemperatureAlarmLevel)
      return true;
  return false;
}

bool Tpms::tireTemperatureAlarm(byte tirePosition)
{
  if (tirePosition < MAX_TIRES &&
      m_tireTemperatures[tirePosition] >= m_tireTemperatureAlarmLevel)
      return true;
  return false;
}


