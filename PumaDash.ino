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

#if (ARDUINO >= 100)
#include "Arduino.h" // for Arduino 1.0
#else
#include "WProgram.h" // for Arduino 23
#endif

// Third party libraries
#include <SPI.h>
#include <SD.h>
#include <Bounce2.h>
#include <Diablo_Serial_4DLib.h>

/*
 * DONE List
 * -- Optimize display speed by redrawing static elements only when needed
 * -- Run final two OBD range support queries
 * -- Add all supported PID's and show data somewhere
 * -- Show logfilename in Red if no logging
 * -- Improve min/max with color mechanism
 * 
 * TODO List
 * -- Make CAN processing super stable
 * ---- Re-enable interrupt
 * ---- Try interrupts with enable/disable interrupts
 * -- Try reducing the reset delay time
 * -- Try running the self-test sooner
 * -- Show logfilename in Red if logging errors
 * -- Drive Defender without a console, to see that it works
 * -- Take console measurements
 * -- One or two resistors in accelerator pedal?
 * -- Measure voltages on accelerator pedal
 * -- Call Engineer
 * -- Order two displays and extension board (plus electronic components) 
 * -- Order USB port
 * -- Order GPS plus motion sensors
 */
 
// Puma code
#include "Utils.h"
#include "CAN.h"
#include "OBD.h"
#include "Tpms.h"
#include "Position.h"
#include "Speed.h"
#include "Display.h"

bool led1_on = true;

PumaDisplay g_display1(&DISPLAY_SERIAL1);  // Puma display driver. We need to create this beast first, so we can use a global reference to it throughout the code.
//PumaDisplay g_display2(&DISPLAY_SERIAL2);  // Same here. This one is for the second display.
//PumaDisplay g_display3(&DISPLAY_SERIAL3);  // And again for the third display.
Position g_position;                      // GPS position & pitch and roll of vehicle
Tpms g_tpms;                              // Tire Pressure Monitoring
CruiseCtrl g_speed;                       // Speed Control and deals with gearbox ratios etc to calculate gear shifts
PumaOBD g_obd;                            // On Board Diagnostics for the Vehicle

void setup() {
  // Start with resetting the Display. Then do activities that don't require the display so that we use the recommended 5 seconds delay more usefully.
  g_display1.reset(); 

  // Initiate Serial comms so we can send debug info to the Serial Monitor (when connected)
  Serial.begin(DISPLAY_SPEED);  

  // Setup LED's in the Can board, which will indicate the life status of the app
  pinMode(PIN_CAN_BOARD_LED1, OUTPUT);
  pinMode(PIN_CAN_BOARD_LED2, OUTPUT);

  // Setup crucial components in the system
  g_obd.setup();
  g_display1.setup(); // In here we'll finalize the 5 second display reset delay.

// Setup logging to the SD (USB) medium. Since this depends on the display we can only start logging once we have initialized the display.
  initLogging();

#ifdef SELF_TEST
  // Run a comprehensive self-test
  selfTest();
#endif

  // Enable the interrupt handler that will process RX data coming from the CAN/OBD2 bus 
  //attachInterrupt(digitalPinToInterrupt(PIN_MP2515_RX_INTERRUPT), canRxHandler, FALLING);
}

// Run the infinite loop. On average, one cycle takes 35-40 ms.
void loop() {
  // LED1 indicates that the main loop is running. A flashing led is 'good'.
  // The Led will appear to be more 'on' than 'off' but definitely needs to be flashing. 
  // Intensity will be much brighter than LED2 which is triggered by incoming OBD data.
  digitalWrite(PIN_CAN_BOARD_LED1, led1_on);
  led1_on = !led1_on;

  g_obd.readRxBuffers();
  g_obd.requestObdUpdates();
  
  g_tpms.update();
  g_position.update();
  g_obd.update();
  g_speed.update();

  g_display1.processTouchEvents();
  g_display1.updateStatusbar();
}

/*
// Interrupt handler for fetching messages from MCP2515 RX buffer
void canRxHandler() {
  noInterrupts();
  g_obd.readRxBuffers();
  interrupts();
}
*/
