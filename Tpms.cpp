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

#include "Utils.h"
#include "Display.h"
#include "Tpms.h"

#if (ARDUINO >= 100)
#include "Arduino.h" // for Arduino 1.0
#else
#include "WProgram.h" // for Arduino 23
#endif

Tpms::Tpms()
{
#define MIN_PRESS 20
#define MAX_PRESS 45
#define MIN_TEMP 10
#define MAX_TEMP 60

  m_FL_Pressure = new OBDData(PID_PUMA_TPMS_FL_PRESS, "FL Pressure", "%2d", "PSI", WORD_NO_CONVERSION, MIN_PRESS, MAX_PRESS);
  m_FL_Temperature = new OBDData(PID_PUMA_TPMS_FL_TEMP, "Temperature", "%2d", "C", WORD_NO_CONVERSION, MIN_TEMP, MAX_TEMP);

  m_FR_Pressure = new OBDData(PID_PUMA_TPMS_FR_PRESS, "FR Pressure", "%2d", "PSI", WORD_NO_CONVERSION, MIN_PRESS, MAX_PRESS);
  m_FR_Temperature = new OBDData(PID_PUMA_TPMS_FR_TEMP, "Temperature", "%2d", "C", WORD_NO_CONVERSION, MIN_TEMP, MAX_TEMP);

  m_RL_Pressure = new OBDData(PID_PUMA_TPMS_RL_PRESS, "RL Pressure", "%2d", "PSI", WORD_NO_CONVERSION, MIN_PRESS, MAX_PRESS);
  m_RL_Temperature = new OBDData(PID_PUMA_TPMS_RL_TEMP, "Temperature", "%2d", "C", WORD_NO_CONVERSION, MIN_TEMP, MAX_TEMP);

  m_RR_Pressure = new OBDData(PID_PUMA_TPMS_RR_PRESS, "RR Pressure", "%2d", "PSI", WORD_NO_CONVERSION, MIN_PRESS, MAX_PRESS);
  m_RR_Temperature = new OBDData(PID_PUMA_TPMS_RR_TEMP, "Temperature", "%2d", "C", WORD_NO_CONVERSION, MIN_TEMP, MAX_TEMP);

  m_TL_Pressure = new OBDData(PID_PUMA_TPMS_TL_PRESS, "TL Pressure", "%2d", "PSI", WORD_NO_CONVERSION, MIN_PRESS, MAX_PRESS);
  m_TL_Temperature = new OBDData(PID_PUMA_TPMS_TL_TEMP, "Temperature", "%2d", "C", WORD_NO_CONVERSION, MIN_TEMP, MAX_TEMP);

  m_TR_Pressure = new OBDData(PID_PUMA_TPMS_TR_PRESS, "TR Pressure", "%2d", "PSI", WORD_NO_CONVERSION, MIN_PRESS, MAX_PRESS);
  m_TR_Temperature = new OBDData(PID_PUMA_TPMS_TR_TEMP, "Temperature", "%2d", "C", WORD_NO_CONVERSION, MIN_TEMP, MAX_TEMP);

  m_tirePressureWarningLevel = 30;
  m_tirePressureAlarmLevel = 24;
  m_tireTemperatureWarningLevel = 35;
  m_tireTemperatureAlarmLevel = 45;
}

void Tpms::update()
{
#ifdef LOOPBACK_MODE
  m_FL_Pressure->simulateData(0);
  m_FL_Temperature->simulateData(0);

  m_FR_Pressure->simulateData(0);
  m_FR_Temperature->simulateData(0);

  m_RL_Pressure->simulateData(0);
  m_RL_Temperature->simulateData(0);

  m_RR_Pressure->simulateData(0);
  m_RR_Temperature->simulateData(0);

  m_TL_Pressure->simulateData(0);
  m_TL_Temperature->simulateData(0);

  m_TR_Pressure->simulateData(0);
  m_TR_Temperature->simulateData(0);
#else
  // TODO: Get data from HW sensor
#endif // LOOPBACK_MODE
}

/*
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
*/




