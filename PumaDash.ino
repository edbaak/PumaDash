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

PumaDisplay g_display(&DISPLAY_SERIAL1);  // Puma display driver. We need to create this beast first, so we can use a global reference to it throughout the code.
Position g_position;                      // GPS position & pitch and roll of vehicle
Tpms g_tpms;                              // Tire Pressure Monitoring
CruiseCtrl g_speed;                       // Speed Control and deals with gearbox ratios etc to calculate gear shifts
PumaOBD g_obd;                            // On Board Diagnostics for the Vehicle

void setup() {
  pinMode(PIN_CAN_BOARD_LED1, OUTPUT);
  pinMode(PIN_CAN_BOARD_LED2, OUTPUT);
  
  Serial.begin(DISPLAY_SPEED);
//  Serial.println("---------------------");
//  long m_simValue = 0x112233FF;
//  Serial.println(m_simValue, HEX);
//  Serial.println(m_simValue & 0xFF, HEX);
//  Serial.println(m_simValue >> 8 & 0xFF, HEX);
//  Serial.println(m_simValue >> 16 & 0xFF, HEX);
//  Serial.println(uint8_t(m_simValue >> 24 & 0xFF), HEX);
//  Serial.println("---------------------");

  initLogging();
  g_obd.setup();
  g_display.setup();
  attachInterrupt(digitalPinToInterrupt(PIN_MP2515_RX_INTERRUPT), canRxHandler, FALLING);
}

bool show_led1 = true;
void loop() {
  // Use LED1 to indicate that the main loop is running.
  // The Led will appear to be more 'on' than 'off' but definitely needs to be flashing. Intensity will be much brighter than LED2 below.
  // A flashing led is 'good'. It means we are not stuck in a dead-lock somewhere.
  digitalWrite(PIN_CAN_BOARD_LED1, show_led1);
  show_led1 = !show_led1;
  
  g_display.processTouchEvents();
  g_tpms.update();
  g_position.update();
  g_obd.requestObdUpdates();
  g_obd.readMessages();
  g_speed.update();
}

// Interrupt handler for fetching messages from MCP2515 RX buffer
void canRxHandler() {
  // Use LED2 to indicate Interrupt activity.
  // The Led should be on for only a very short period, so visually this will be a fast flashing led with a low light intensity. 
  // A flashing led is 'good'. It means we have incoming data, and are not stuck in a dead-lock somewhere.
  digitalWrite(PIN_CAN_BOARD_LED2, HIGH);
  g_obd.readRxBuffers();
  digitalWrite(PIN_CAN_BOARD_LED2, LOW);
}
