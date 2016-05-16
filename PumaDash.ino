/*
  Copyright (c) 2016 by Ed Baak
*/
#include "Utils.h"
#include <SPI.h>
#ifdef USE_CAN2
  #include "CAN.h"
#else
  #include <CAN.h>
  #include <CAN_AT90CAN.h>
  #include <CAN_K2X.h>
  #include <CAN_MCP2515.h>
  #include <CAN_SAM3X.h>
  #include <CAN_SJA1000.h>
  #include <sn65hvd234.h>
#endif
#include "OBD.h"
#include <SD.h>
#include <Bounce2.h>
#include "Tpms.h"
#include "Position.h"
#include "Speed.h"
#include <Diablo_Const4D.h>
#include <Diablo_Serial_4DLib.h>
#include "Display.h"

Tpms g_Tpms; // Tpms class deals with Tire Pressure Monitoring
Direction g_Position; // Position class deals with GPS position & pitch and roll of vehicle
CruiseCtrl g_Speed; // Speed class implements a Cruise Control, and deals with gearbox ratios etc to calculate gear shifts
#ifdef USE_CAN2
  MCP_CAN g_Can(PIN_MEGA_SPI_CS); // CAN class implements the CAN messaging protocol
#else
  CAN_MCP2515 g_Can(PIN_MEGA_SPI_CS);
#endif
OBD g_Obd(&g_Can); // OBS class implements the On Board Diagnostics for the Vehicle
Diablo_Serial_4DLib Display(&DisplaySerial); // Basic display driver
LeftDisplay g_LeftDisplay(&Display, &g_Position, &g_Tpms); // Implements all things to do with the 'Left' display
CenterDisplay g_CenterDisplay(&Display, &g_Obd); // Implements all things to do with the 'Center' display
RightDisplay g_RightDisplay(&Display, &g_Speed); // Implements all things to do with the 'Right' display

byte g_active_display = 1; // Define the default display. We can change this by tapping the touchscreen

// the setup function runs once when you press reset or power the board
void setup() {  
  Serial.begin(115200);
  
  initLogging();

#ifdef USE_CAN2
  g_Obd.begin(CAN_500KBPS, MCP_LOOPBACK);
#else
  g_Obd.begin(CAN_BPS_500K, MCP2515_MODE_LOOPBACK);
#endif  

  // we're assuming we're connecting to the left display, but will be switching to center or right display as needed in the code.
  // in practice it doesn't really matter as there really only one display connected and all three displays use the same
  // connection to communicate with the display. In other words, what changes is *what* information we present on the 
  // display, and not *how* we communicate with the display.
  g_LeftDisplay.reset();
}

// the loop function runs over and over again forever
void loop() {
  static bool init_display = true;
  static bool filters_on = true;
  
  g_Tpms.update();
  g_Position.update();
  g_Obd.refresh(g_logFileName);
  g_Speed.update();

  if (!init_display && g_LeftDisplay.touchPressed()) {
    Display.gfx_Cls();
/*    
    g_active_display++;
    if (g_active_display > 2)
      g_active_display = 0;
*/
    init_display = true;  
    if (filters_on) {
      g_Obd.setCanFilters(0x7E8,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF);
    } else {
      g_Obd.setCanFilters(0x00);
    }
    filters_on = !filters_on;    
  }

  if (init_display) {
    init_display = false;
    switch (g_active_display) {
      case 0: 
        g_LeftDisplay.init();
        g_LeftDisplay.redrawLabels(); 
        break;
      case 1: 
        g_CenterDisplay.init();
        g_CenterDisplay.redrawLabels(); 
        break;
      case 2: 
        g_RightDisplay.init();
        g_RightDisplay.redrawLabels(); 
        break;
    }
  }

  switch (g_active_display) {
    case 0: g_LeftDisplay.update(); break;
    case 1: g_CenterDisplay.update(); break;
    case 2: g_RightDisplay.update(); break;
  }
}

