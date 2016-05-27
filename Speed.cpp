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
#include "Speed.h"

#if (ARDUINO >= 100)
#include "Arduino.h" // for Arduino 1.0
#else
#include "WProgram.h" // for Arduino 23
#endif

CruiseCtrl::CruiseCtrl()
{
  maxAcceleratorPosition = 100;
  minAcceleratorPosition = 0;
  cruiseSpeed = 0.0;
  minAllowedSpeed = 5.0;
  maxAllowedSpeed = 100.0;

  m_CC_Speed = new OBDData(PID_PUMA_CC_SPEED, "Target Speed", "%3d", "Km/h", 1000, BYTE_NO_CONVERSION, 0, 110, 5);
  m_CC_Mode = new OBDData(PID_PUMA_CC_MODE, "Mode", "%3d", "", 1000, BYTE_NO_CONVERSION, 0, 2, 1);
  m_CC_Accelerator = new OBDData(PID_PUMA_CC_ACCELERATOR, "Accelerator", "%3d", "%", 1000, BYTE_NO_CONVERSION, 0, 100, 1);
}

void CruiseCtrl::switchOnCruiseControlRelay(bool on)
{
  on = on;
}

void CruiseCtrl::setAccelleration(int accelleration) {
  // set cur value to an invalid value so that we update the analog output pin to 0 in the setup function
  static int current_accelleration = -1;
  if (accelleration != current_accelleration) {
    current_accelleration = accelleration;
    analogWrite(simulatedAccelleratorPinId, accelleration);
  }
}

void CruiseCtrl::update()
{
#ifdef LOOPBACK_MODE
  m_CC_Speed->simulateData(0);
  m_CC_Mode->simulateData(0);
  m_CC_Accelerator->simulateData(0);
#endif

  /*
    if (!odbValid) {
      // Without a valid ODB2 signal we're not going to enable cruise control
      // switch off the relay so that we're always connecting the accelerator pedal directly to the ECU

      switchOnCruiseControlRelay(false);
    } else {
      // Check status of input pins
      increaseSpeed.update();
      decreaseSpeed.update();
      brakePressed.update();
      clutchPressed.update();
      enableCruiseControl.update();
      lowRangeGearSelected.update();
      actualAccelleratorPosition = analogRead(actualAccelleratorPinId) / 4;

      if (!cruiseControlOn) {
        setAccelleration(actualAccelleratorPosition);

        // only allow cruise control to become active if we've enabled it.
        if (enableCruiseControl.read()) {
          // Cruise Control is switched on by tapping the increase toggle switch
          if (increaseSpeed.read()) {
            // Only switch on Cruise Control if we drive >= minAllowedSpeed
            if (actualSpeed >= minAllowedSpeed) {
              cruiseControlOn = true;

              // the current speed will be the target speed that is kept by the cruise control
              cruiseSpeed = actualSpeed;
              // start with the current acceleratorposition and then adjust in small steps
              cruiseAcceleratorPosition = actualAccelleratorPosition;
            }
          } else if (!clutchPressed.read() && cruiseSpeed > 0.0) {
            cruiseControlOn = true;
            // we'll try to get back to the previously set target speed
            // start with the current acceleratorposition and then adjust in small steps
            cruiseAcceleratorPosition = actualAccelleratorPosition;
          }
        } // enableCruiseControl
      } else { // Cruise Control is ON

        if (brakePressed.read() || !enableCruiseControl.read()) {
          cruiseControlOn = false;
          cruiseSpeed = 0.0;
        } else if (clutchPressed.read()) {
          cruiseControlOn = false;
          // remember the target speed
        }

        if (cruiseControlOn) {
          if (increaseSpeed.read() && cruiseSpeed < maxAllowedSpeed)
            cruiseSpeed += 1.0;
          else if (decreaseSpeed.read() && cruiseSpeed > minAllowedSpeed)
            cruiseSpeed -= 1.0;

          if (actualSpeed + 0.1 < cruiseSpeed) { // driving too slow?
            if (cruiseAcceleratorPosition < maxAcceleratorPosition) {
              cruiseAcceleratorPosition++;
            }
          } else if (actualSpeed - 0.1 > cruiseSpeed) { // driving too fast?
            if (cruiseAcceleratorPosition > minAcceleratorPosition) {
              cruiseAcceleratorPosition--;
            }
          }

          setAccelleration(cruiseAcceleratorPosition);
        }
      } // odbValid
      switchOnCruiseControlRelay(cruiseControlOn);
    }
   */

  /*
   // -----------------------------
   brakePressed.attach(brakePinId);
   brakePressed.interval(bounceInterval);
   clutchPressed.attach(clutchPinId);
   clutchPressed.interval(bounceInterval);
   increaseSpeed.attach(increaseSpeedPinId);
   increaseSpeed.interval(bounceInterval);
   decreaseSpeed.attach(decreaseSpeedPinId);
   decreaseSpeed.interval(bounceInterval);
   enableCruiseControl.attach(enableCruiseControlPinId);
   enableCruiseControl.interval(bounceInterval);
   lowRangeGearSelected.attach(lowRangeGearSelectedPinId);
   lowRangeGearSelected.interval(bounceInterval);

   // ensure we start with the CruiseControl switched off when we start the vehicle
   cruiseControlOn = false;
   cruiseSpeed = 0.0;
   actualAccelleratorPosition = 0;

   setAccelleration(0);
  */
}



