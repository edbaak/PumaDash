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

#ifndef Displays_h
#define Displays_h

#if (ARDUINO >= 100)
#include "Arduino.h" // for Arduino 1.0
#else
#include "WProgram.h" // for Arduino 23
#endif

#include "Utils.h"
//#include <Diablo_Const4D.h>
#include <Diablo_Serial_4DLib.h>
#include <string.h>
#include "OBD.h"

class PumaDisplay;
class SensorWidget;

#define MAX_FONT_SIZE 10

// Returns a pointer to the one and only global display class instance. This global pointer prevents the need for every class/instance to store a local copy.
PumaDisplay* Display();

class BaseScreen
{
  public:
    BaseScreen();

    virtual byte displayOrientation() = 0;
    virtual void init();

    void addSensorWidget(SensorWidget *sensor);
    SensorWidget *findSensorWidget(word pid);
    void updateSensorWidget(OBDBaseData *sensor);
    void requestStaticRefresh();

    void printLabel(String label, word x, word y, int color, byte fontSize = 1);
    void printSubLabel(String subLabel, word x, word y, int color, byte fontSize = 1);
    void printValue(String value, byte textLength, word x, word y, int color, byte fontSize);

    word maxWidth();
    word maxHeight();
    String screenName();

    void processTouchEvents();
    bool displayTapped(word &x, word &y, word &tapTime);
    bool swipedUpOrDown(int &steps);
    bool swipedLeftOrRight(int &steps);
    void resetSwiped();

  protected:
    void printPrepare(word x, word y, int color, byte fontSize);

  protected:
    String m_screenName;
    SensorWidget *m_first_sensor_widget;
    word display_max_x;
    word display_max_y;
    word display_x_mid;
    word display_y_mid;

    word left_border;
    word right_border;
    word top_border;
    word bottom_border;

    word left_divider;
    word right_divider;

    word top_divider;
    word mid_divider;
    word bottom_divider;

  protected:
    typedef enum SWIPE_MODE {
      NO_SWIPE,
      SWIPE_DETECTED,
      SWIPE_LR,
      SWIPE_UD,
      SWIPE_IGNORE
    } SWIPE_MODE;

    SWIPE_MODE m_swipe_mode;
    word m_touch_x_start;
    word m_touch_x1;
    word m_touch_x2;
    word m_touch_y_start;
    word m_touch_y1;
    word m_touch_y2;
    word m_touch_duration;
    bool m_display_swiped;
    long m_touch_swipe_x;
    long m_touch_swipe_y;
    StopWatch m_touch_start_time;
    StopWatch m_last_touch_time;
};

class Screen0 : public BaseScreen
{
  public:
    Screen0();
    virtual byte displayOrientation();
    virtual void init();
};

class Screen1 : public BaseScreen
{
  public:
    Screen1();
    virtual byte displayOrientation();
    virtual void init();
};

class Screen2 : public BaseScreen
{
  public:
    Screen2();
    virtual byte displayOrientation();
    virtual void init();
};

class PumaDisplay : public Diablo_Serial_4DLib
{
  public:
    PumaDisplay(Stream * virtualPort);
    virtual ~PumaDisplay();

    void setup();
    void processTouchEvents();
    void updateStatusbar();
    void reset();

    void printLabel(String label, word x, word y, int color, byte fontSize = 1);
    void printSubLabel(String subLabel, word x, word y, int color, byte fontSize = 1);
    void printValue(String value, byte textLength, word x, word y, int color, byte fontSize);

    void updateSensorWidget(OBDBaseData *sensor);

    word fontWidth(byte fontSize);
    word fontHeight(byte fontSize);

  protected:
    bool g_init_display;
    byte g_active_screen; // The screen currently shown on the display, i.e. m_screen0, m_screen1 or m_screen2
    BaseScreen *activeScreen();
    int m_displayContrast;

    StopWatch m_static_refresh_timer;
    StopWatch m_loop_timer;

    Screen0 m_screen0;    // Visual elements of the 'Left' display
    Screen1 m_screen1;    // Visual elements of the 'Center' display
    Screen2 m_screen2;    // Visual elements of the 'Right' display

  private:
    word m_font_width[MAX_FONT_SIZE + 1];
    word m_font_height[MAX_FONT_SIZE + 1];
};

#endif



