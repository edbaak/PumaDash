/*
   CruiseCtrl.cpp - Library for ...
 */

#include "Utils.h"
#include "Speed.h"

#if (ARDUINO >= 100)
	#include "Arduino.h" // for Arduino 1.0
#else
	#include "WProgram.h" // for Arduino 23
#endif

CruiseCtrl::CruiseCtrl() { 
}

void CruiseCtrl::switchOnCruiseControlRelay(bool on) {

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
