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
#include <Diablo_Const4D.h>
#include <Diablo_Serial_4DLib.h>

// Puma code
#include "Utils.h"
#include "CAN.h"
#include "OBD.h"
#include "Tpms.h"
#include "Position.h"
#include "Speed.h"
#include "Display.h"

bool led1_on = true;

PumaDisplay g_display(&DISPLAY_SERIAL1);  // Puma display driver. We need to create this beast first, so we can use a global reference to it throughout the code.
Position g_position;                      // GPS position & pitch and roll of vehicle
Tpms g_tpms;                              // Tire Pressure Monitoring
CruiseCtrl g_speed;                       // Speed Control and deals with gearbox ratios etc to calculate gear shifts
PumaOBD g_obd;                            // On Board Diagnostics for the Vehicle

void setup() {
  // Start with resetting the Display. Then do activities that don't require the display so that we use the recommended 5 seconds delay more usefully.
  g_display.reset(0); 

  // Initiate Serial comms so we can send debug info to the Serial Monitor (when connected)
  Serial.begin(DISPLAY_SPEED);  

  // Setup LED's in the Can board, which will indicate the life status of the app
  pinMode(PIN_CAN_BOARD_LED1, OUTPUT);
  pinMode(PIN_CAN_BOARD_LED2, OUTPUT);

// Setup logging to the SD (USB) medium
  initLogging();

  // Run a comprehensive self-test
  selfTest();

  // Setup crucial components in the system
  g_obd.setup();
  g_display.setup(); // In here we'll finalize the 5 second display reset delay.

  // Enable the interrupt handler that will process RX data coming from the CAN/OBD2 bus 
  attachInterrupt(digitalPinToInterrupt(PIN_MP2515_RX_INTERRUPT), canRxHandler, FALLING);
}

// Run the infinite loop. On average, one cycle takes 35-40 ms.
void loop() {
  // LED1 indicates that the main loop is running. A flashing led is 'good'. It means we are not stuck in a dead-lock somewhere.
  // The Led will appear to be more 'on' than 'off' but definitely needs to be flashing. Intensity will be much brighter than LED2 below.
  digitalWrite(PIN_CAN_BOARD_LED1, led1_on);
  led1_on = !led1_on;
  
  g_display.processTouchEvents();
  g_tpms.update();
  g_position.update();
  g_obd.requestObdUpdates();
  g_obd.readMessages();
  g_speed.update();
}

// Interrupt handler for fetching messages from MCP2515 RX buffer
void canRxHandler() {
  // LED2 indicates CAN bus RX Interrupt activity. A flashing led is 'good'. It means we have incoming data, and are not stuck in a dead-lock somewhere.
  // The Led should be on for only a very short period, so visually this will be a fast flashing led with a low light intensity. 
  digitalWrite(PIN_CAN_BOARD_LED2, HIGH);
  g_obd.readRxBuffers();
  digitalWrite(PIN_CAN_BOARD_LED2, LOW);
}
