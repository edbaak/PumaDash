/*
	Tpms - Library for a TPMS system for a Land Rover Defender 110 Puma MY 12.
	It *may* work on other vehicles as well, but you're on your own.
	Released into the public domain.
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
