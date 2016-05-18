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

#ifndef Displays_h
#define Displays_h
 
#if (ARDUINO >= 100)
	#include "Arduino.h" // for Arduino 1.0
#else
	#include "WProgram.h" // for Arduino 23
#endif

#include "Utils.h"
#include <Diablo_Const4D.h>
#include <Diablo_Serial_4DLib.h>
#include <string.h>
#include "Tpms.h"
#include "Position.h"
#include "Speed.h"
#include "OBD.h"

class PumaDisplay;

class BaseScreen
{
public:
  BaseScreen();
  
  virtual void setup(PumaDisplay *disp); 
  virtual void redrawLabels() {};
  virtual byte displayOrientation() { return 0; };
  virtual void update() {};
  virtual void init();
  
  word maxWidth();
  word maxHeight();  
  bool updateNeeded(unsigned int &lastUpdate, word updateInterval);
  void updateStatusBar();
  bool touchPressed();

protected:
  PumaDisplay *Display_;
  word display_max_width;
  word display_max_height;
  int top_separator_line;
  int mid_separator_line;
  int bottom_separator_line;
  int start_x;
  int end_x;
  int mid_screen;   
};

class Screen0 : public BaseScreen
{
public:
  Screen0();
  void update();
  void redrawLabels(); 
  virtual byte displayOrientation();
  
protected:  
  void updatePitch(byte x, byte y, byte interleave, int angle);
  void updateRoll(byte x, byte y, byte interleave, int angle);
  void updateCompass(word heading);
  void updateTPMSvalue(byte tireLocation);
};

class Screen1 : public BaseScreen
{
public:
  Screen1();
  void update();
  void redrawLabels(); 
  virtual byte displayOrientation();
  virtual void init();

protected:
  void updateSpeed(word speed);
  void updateRpm(word rpm);
      
private:
  word y_mid;
  word x_mid;
  word left_vertical;
  word right_vertical;
  word bottom_divider;
  word label_x_offset;
  word label1_y_offset;
  word label2_y_offset;
  word label3_y_offset;
};

class Screen2 : public BaseScreen
{
public:
  Screen2();
  void update();
  void redrawLabels(); 
  virtual byte displayOrientation();
  
protected:  
  void updateCruiseControl();
  void updateOBD2Status();
};

class PumaDisplay : public Diablo_Serial_4DLib
{
  public:
    PumaDisplay(Stream * virtualPort, Direction *pos, Tpms *tpms, CruiseCtrl *speed, PumaOBD *obd);
    void update();
    void reset();
    void setup();
    BaseScreen *activeScreen();
    
  protected:
    bool g_init_display;
    byte g_active_screen; // The screen currently shown on the display

    friend class Screen0;
    Screen0 m_screen0;    // Visual elements of the 'Left' display
    
    friend class Screen1;
    Screen1 m_screen1;    // Visual elements of the 'Center' display

    friend class Screen2;
    Screen2 m_screen2;    // Visual elements of the 'Right' display
  
    Direction *m_position;
    Tpms *m_tpms;
    CruiseCtrl *m_speed;
    PumaOBD *m_obd;   
};

#endif
