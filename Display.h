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

class SensorWidget
{
  public:
    SensorWidget(word pid, byte fontSize, word x, word y);

  protected:
    friend class BaseScreen;  
    word m_pid;
    byte m_fontSize;
    word m_x;
    word m_y;

    SensorWidget *m_next;
};

class BaseScreen
{
public:
  BaseScreen();
  
  virtual void setup(PumaDisplay *disp); 
  virtual byte displayOrientation() { return 0; };
  virtual void update() {};
  virtual void init();

  void addSensor(SensorWidget *sensor);
  SensorWidget *findSensor(word pid);
  void updateSensor(OBDDataValue *sensor);

  void printLabel(String label, word x, word y, int color, int textSize = 1);
  void printSubLabel(String subLabel, word x, word y, int color, int textSize = 1);
  void printValue(String value, word x, word y, int color, int textSize = 1);
  
  word maxWidth();
  word maxHeight();  
  bool touchPressed();

protected:
  void printPrepare(word x, word y, int color, int textSize);

protected:
  PumaDisplay *Display_;
  SensorWidget *m_first;
  word display_max_x;
  word display_max_y;
  word display_x_mid;
  word display_y_mid;
  word left_border;
  word right_border;
  byte big_border;
  word left_divider_line;
  word right_divider_line;
  word top_separator_line;
  word mid_separator_line;
  word bottom_divider;
};

class Screen0 : public BaseScreen
{
public:
  Screen0();
  void update();
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
  virtual byte displayOrientation();
  virtual void init();

protected:
  void updateRpm(word rpm);
      
private:
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
    PumaDisplay(Stream * virtualPort);
    void setup(Direction *pos, Tpms *tpms, CruiseCtrl *speed, PumaOBD *obd);
    void update();
    void reset();
    BaseScreen *activeScreen();
    void updateSensor(OBDDataValue *sensor);
    
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
