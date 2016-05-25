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
// Gearbox ratios
// MT82, 6 speed gearbox
// First: 5.443:1
// Second: 2.839:1
// Third: 1.721:1
// Fourth: 1.223:1
// Fifth: 1:1
// Sixth: 0.742:1
// LT230 transfer box
// Hi: 1.211
// Lo: 3.32
// Diff Ratio: 3.54  

*/
 
#ifndef CruiseCtrl_h
#define CruiseCtrl_h
 
#if (ARDUINO >= 100)
	#include "Arduino.h" // for Arduino 1.0
#else
	#include "WProgram.h" // for Arduino 23
#endif

#include "Utils.h"
#include <string.h>
#include <../Bounce2/Bounce2.h>

#define bounceInterval 1
#define brakePinId 1
#define clutchPinId 2
#define increaseSpeedPinId 3
#define decreaseSpeedPinId 4
#define enableCruiseControlPinId 5
#define lowRangeGearSelectedPinId 6
#define actualAccelleratorPinId 7
#define simulatedAccelleratorPinId 8

class CruiseCtrl
{
  public:
  CruiseCtrl();
				
void switchOnCruiseControlRelay(bool on);
void setAccelleration(int accelleration);
void update();
				
  private:
// this info comes from the accelerator pedal
  int actualAccelleratorPosition;

// this info comes from ODB2 and is the actual vehicle speed
  float actualSpeed;

// these two values determine min and max values for the actual accelerator position of the pedal, and can be used to calibrate the pedal
  int maxAcceleratorPosition;
  int minAcceleratorPosition;

// this is the simulated accelerator pedal
  int cruiseAcceleratorPosition;

// this is the speed the vehicle had when engagePressed was pressed
  float cruiseSpeed;
  float minAllowedSpeed;
  float maxAllowedSpeed;

  bool cruiseControlOn = false;

  // Inputs
  Bounce brakePressed;
  Bounce clutchPressed;
  Bounce enableCruiseControl;
  Bounce increaseSpeed;
  Bounce decreaseSpeed;
  Bounce lowRangeGearSelected;

};

#endif



