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
#undef DEC
#undef HEX
#undef BIN
#include <Diablo_Const4D.h>
#include <Diablo_Serial_4DLib.h>
#include "Display.h"

Tpms g_Tpms; // Tpms class deals with Tire Pressure Monitoring
Direction g_Position; // Position class deals with GPS position & pitch and roll of vehicle
CruiseCtrl g_Speed; // Speed class implements a Cruise Control, and deals with gearbox ratios etc to calculate gear shifts
PumaCAN g_Can(PIN_MEGA_SPI_CS); // CAN class implements the CAN messaging protocol
PumaOBD g_Obd(&g_Can); // OBS class implements the On Board Diagnostics for the Vehicle
Diablo_Serial_4DLib Display(&DisplaySerial); // Basic display driver
Screen0 g_screen0(&Display, &g_Position, &g_Tpms); // Implements all things to do with the 'Left' display
Screen1 g_screen1(&Display, &g_Obd); // Implements all things to do with the 'Center' display
Screen2 g_screen2(&Display, &g_Speed); // Implements all things to do with the 'Right' display

byte g_active_screen = 1; // Define the default screen. We can change this by tapping the touchscreen

// the setup function runs once when you press reset or power the board
void setup() {  
  Serial.begin(115200);
  
  initLogging();

#ifdef LOOPBACK_MODE
  g_Obd.begin(PumaCAN::CAN_500KBPS, PumaCAN::MCP_LOOPBACK);
#else
  g_Obd.begin(PumaCAN::CAN_500KBPS, PumaCAN::MCP_NORMAL);
#endif
  g_Obd.setCanFilters(0x07E80000, 0x07E80000);

  // Reset the display. It doesn't really matter which screen we used: it's all the 
  // same physical display.
  g_screen0.reset();
}

// the loop function runs over and over again forever
bool g_init_display = true;
void loop() {
  g_Tpms.update();
  g_Position.update();
  g_Obd.update();
  g_Speed.update();

  if (!g_init_display && g_screen0.touchPressed()) {
    Display.gfx_Cls();
    g_active_screen++;
    if (g_active_screen > 2)
      g_active_screen = 0;
    g_init_display = true;      
  }

  if (g_init_display) {
    g_init_display = false;
    switch (g_active_screen) {
      case 0: 
        g_screen0.init();
        g_screen0.redrawLabels(); 
        break;
      case 1: 
        g_screen1.init();
        g_screen1.redrawLabels(); 
        break;
      case 2: 
        g_screen2.init();
        g_screen2.redrawLabels(); 
        break;
    }
  }

  switch (g_active_screen) {
    case 0: g_screen0.update(); break;
    case 1: g_screen1.update(); break;
    case 2: g_screen2.update(); break;
  }
}

