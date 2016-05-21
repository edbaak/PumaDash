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
#include <SPI.h>
#include "CAN.h"
#include "OBD.h"
#include <SD.h>
#include <Bounce2.h>
#include "Tpms.h"
#include "Position.h"
#include "Speed.h"
#include <Diablo_Const4D.h>
#include <Diablo_Serial_4DLib.h>
#include "Display.h"

Direction g_position;                                                         // GPS position & pitch and roll of vehicle
Tpms g_tpms;                                                                  // Tire Pressure Monitoring
CruiseCtrl g_speed;                                                           // Speed Control and deals with gearbox ratios etc to calculate gear shifts
PumaOBD g_obd;                                                                // On Board Diagnostics for the Vehicle
PumaDisplay g_display(&DISPLAY_SERIAL1, &g_position, &g_tpms, &g_speed, &g_obd);  // Basic display driver

void setup() {
  g_obd.setDisplay(&g_display);
  Serial.begin(DISPLAY_SPEED);
  initLogging();
  g_obd.setup();
  g_display.setup();
}

void loop() {
  g_tpms.update();
  g_position.update();
  g_obd.update();
  g_speed.update();
  g_display.update();
}

