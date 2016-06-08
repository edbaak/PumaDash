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

#include "Utils.h"
#include "Display.h"
#include "Widget.h"

#if (ARDUINO >= 100)
#include "Arduino.h" // for Arduino 1.0
#else
#include "WProgram.h" // for Arduino 23
#endif

#ifdef DISPLAY_DEBUG
#define DISPLAY_PRINTLN(s) Serial.println(s); Serial.flush();
#define DISPLAY_PRINT(s) Serial.print(s); Serial.flush();
#else
#define DISPLAY_PRINTLN(s) {}
#define DISPLAY_PRINT(s) {}
#endif

#ifdef TOUCH_DEBUG
#define TOUCH_PRINTLN(s) Serial.println(s); Serial.flush();
#define TOUCH_PRINT(s) Serial.print(s); Serial.flush();
#else
#define TOUCH_PRINTLN(s) {}
#define TOUCH_PRINT(s) {}
#endif

PumaDisplay *global_Puma_Display = 0;

PumaDisplay* Display()
{
  return global_Puma_Display;
}

// ******************************************************************************************************
//                                              PumaDisplay
// ******************************************************************************************************

PumaDisplay::PumaDisplay(Stream *virtualPort) : Diablo_Serial_4DLib(virtualPort)
{
  global_Puma_Display = this;
  g_init_display = true;
  g_active_screen = PUMA_DEFAULT_SCREEN; // Define the default screen. We can change this by tapping the touchscreen

  // Set D4 on Arduino to Output (4D Arduino Adaptor V2 - Display Reset)
  pinMode(PIN_DISPLAY_RESET, OUTPUT);
}

PumaDisplay::~PumaDisplay()
{
  file_Unmount();
}

void mycallback(int ErrCode, unsigned char Errorbyte)
{
  const char *Error4DText[] = {"OK\0", "Timeout\0", "NAK\0", "Length\0", "Invalid\0"} ;
  Serial.print(F("Serial 4D Library reports error ")) ;
  Serial.print(Error4DText[ErrCode]);

  if (ErrCode == Err4D_NAK) {
    Serial.print(F(" returned data= ")) ;
    Serial.println(Errorbyte) ;
  } else {
    Serial.println(F("")) ;
  }
}

void PumaDisplay::setup()
{
  reset();

  // Initialize the Displays. At the moment I'm only running on one display, but want to ensure I have sufficient memory for all three.
  DISPLAY_SERIAL1.begin(DISPLAY_SPEED) ;
  //  DISPLAY_SERIAL2.begin(DISPLAY_SPEED) ;
  //  DISPLAY_SERIAL3.begin(DISPLAY_SPEED) ;

  TimeLimit4D = 5000 ; // 5 second timeout on all commands
  Callback4D = mycallback ;
  m_displayContrast = 13;
  gfx_Contrast(m_displayContrast);

  txt_FontID(FONT_3);
  touch_Set(TOUCH_ENABLE);

  for (byte i = 0; i < MAX_FONT_SIZE; i++) {
    txt_Width(i + 1);
    txt_Height(i + 1);
    m_font_width[i] = charwidth('0');
    m_font_height[i] = charheight('0');
  }
}

BaseScreen *PumaDisplay::activeScreen()
{
  switch (g_active_screen) {
    case 0: return &m_screen0;
    case 1: return &m_screen1;
    default: return &m_screen2;
  }
  return &m_screen2;
}

void PumaDisplay::processTouchEvents()
{
  DISPLAY_PRINTLN(">> PumaDisplay::processTouchEvents()");
  m_screen0.processTouchEvents();

  int steps = -1;
  //  if (!g_init_display && m_screen0.swipedLeftOrRight(steps)) {
  //    m_screen0.resetSwiped();
  //  }

  word x, y, tapTime;
  if (m_screen0.displayTapped(x, y, tapTime)) {
    if (steps < 0)
      g_active_screen++;
    else if (g_active_screen > 0)
      g_active_screen--;
    else
      g_active_screen = 2;

    if (g_active_screen > 2)
      g_active_screen = 0;
    g_init_display = true;
  }

  if (g_init_display) {
    g_init_display = false;
    DISPLAY_PRINTLN(">>> initializing active screen");
    activeScreen()->init();
    DISPLAY_PRINTLN(activeScreen()->screenName());
    activeScreen()->requestStaticRefresh();
    DISPLAY_PRINTLN("<<< initializing active screen");
  }

  if (m_screen0.swipedUpOrDown(steps)) {
    m_displayContrast -= steps;
    if (m_displayContrast > 15) m_displayContrast = 15;
    if (m_displayContrast < 1) m_displayContrast = 1;
    gfx_Contrast(m_displayContrast);
  }
  DISPLAY_PRINTLN("<< PumaDisplay::processTouchEvents()");
}

void PumaDisplay::updateStatusbar()
{
  DISPLAY_PRINTLN(">> PumaDisplay::updateStatusBar()");

  static word y_ = 0;
  if (y_ == 0)
    y_ = activeScreen()->maxHeight() - fontHeight(1);

  word char_width = fontWidth(1);
  static word _logfile_x = 0;
  if (_logfile_x == 0) 
    _logfile_x = activeScreen()->maxWidth() - (9 * char_width);

  String out = String(m_loop_timer.elapsed());
  while (out.length() < 4) out = " " + out;
  printLabel(out, 0, y_, WHITE, 1);
  m_loop_timer.start();

  static word dot_count = 0;
  static bool _print_dots = true;
  word dot_x = (char_width * 5) + (dot_count++ * char_width);
  if (dot_x >= _logfile_x - (2 * char_width)) {
    dot_count = 0;
    dot_x = (char_width * 5);
    _print_dots = !_print_dots;
  }
  Display()->gfx_MoveTo(dot_x, y_);
  if (_print_dots) Display()->print(".");
  else Display()->print(" ");

  if (m_static_refresh_timer.elapsed() > 10000) {
    m_static_refresh_timer.start();
    activeScreen()->requestStaticRefresh();

    // While are are at it, refresh the log file name in the display status bar as well
    if (CONFIG()->logFileName().length() > 0)
      printLabel(CONFIG()->logFileName(), _logfile_x, y_, PUMA_LABEL_COLOR);
    else
      printLabel("NO LOG", _logfile_x, y_, PUMA_ALARM_COLOR);
  }

  DISPLAY_PRINTLN("<< PumaDisplay::updateStatusBar()");
}

void PumaDisplay::updateSensorWidget(OBDBaseData *sensor)
{
  activeScreen()->updateSensorWidget(sensor);
}

void PumaDisplay::reset()
{
  digitalWrite(PIN_DISPLAY_RESET, 1);  // Reset the Display via D4
  delay(100);
  digitalWrite(PIN_DISPLAY_RESET, 0);  // unReset the Display via D4
  delay(DISPLAY_RESET_MS);
}

word PumaDisplay::fontWidth(byte fontSize)
{
  if (fontSize > 0) fontSize -= 1;
  if (fontSize < MAX_FONT_SIZE)
    return m_font_width[fontSize];
  return 12;
}

word PumaDisplay::fontHeight(byte fontSize)
{
  if (fontSize > 0) fontSize -= 1;
  if (fontSize < MAX_FONT_SIZE)
    return m_font_height[fontSize];
  return 18;
}

void PumaDisplay::printLabel(String label, word x, word y, int color, byte fontSize)
{
  activeScreen()->printLabel(label, x, y, color, fontSize);
}

void PumaDisplay::printSubLabel(String subLabel, word x, word y, int color, byte fontSize)
{
  activeScreen()->printSubLabel(subLabel, x, y, color, fontSize);
}

void PumaDisplay::printValue(String value, byte textLength, word x, word y, int color, byte fontSize)
{
  activeScreen()->printValue(value, textLength, x, y, color, fontSize);
}

// ******************************************************************************************************
//                                              BASE SCREEN
// ******************************************************************************************************

BaseScreen::BaseScreen()
{
  display_max_y = 0;
  display_max_x = 0;
  m_first_sensor_widget = 0;
}

void BaseScreen::init()
{
  DISPLAY_PRINTLN(">>> BaseScreen_init");
  display_max_y = 0;
  display_max_x = 0;
  m_first_sensor_widget = 0;

  // TODO: Make some of these into #defines
  Display()->gfx_Cls();
  Display()->gfx_ScreenMode(displayOrientation());
  delay(50);

  display_max_x = Display()->gfx_Get(X_MAX);
  display_max_y = Display()->gfx_Get(Y_MAX);
  top_divider = display_max_y / 3;
  mid_divider = top_divider + (top_divider * 2 / 3);
  bottom_divider = top_divider + (top_divider * 4 / 3);
  left_border = 10;
  right_border = display_max_x - left_border;
  display_y_mid = display_max_y / 2;
  display_x_mid = display_max_x / 2;
  top_border = 10;
  bottom_border = display_max_y - top_border;

  left_divider = 110; //135;
  right_divider = maxWidth() - left_divider;
  bottom_divider = maxHeight() - 130;

  m_touch_x1 = 0;
  m_touch_x2 = 0;
  m_touch_y1 = 0;
  m_touch_y2 = 0;
  m_touch_duration = 0;
  m_display_swiped = false;
  m_touch_swipe_x = 0;
  m_touch_swipe_y = 0;
  m_swipe_mode = NO_SWIPE;
  DISPLAY_PRINTLN("<<< BaseScreen_init");
}

void BaseScreen::addSensorWidget(SensorWidget * sensor)
{
  //TODO: this might benefit from more automation, i.e. calculate the position of the sensor based on some standard definitions such as TOP_LEFT, MID_LEFT, etc,
  if (m_first_sensor_widget == 0) {
    m_first_sensor_widget = sensor;
    return;
  }

  SensorWidget *tmp = m_first_sensor_widget;
  while (tmp) {
    if (tmp->m_next == 0) {
      tmp->m_next = sensor;
      return;
    }
    tmp = tmp->m_next;
  }
}

SensorWidget *BaseScreen::findSensorWidget(word pid)
{
  SensorWidget *tmp = m_first_sensor_widget;
  while (tmp) {
    if (tmp->m_pid == pid)
      return tmp;
    tmp = tmp->m_next;
  }
  return 0;
}

void BaseScreen::updateSensorWidget(OBDBaseData * sensor)
{
  SensorWidget *tmp = findSensorWidget(sensor->pid());
  if (tmp) {
    DISPLAY_PRINTLN(String(">>> updateSensorWidget: ") + String(sensor->pid(), HEX));
    tmp->update(sensor);
  }
}

void BaseScreen::requestStaticRefresh()
{
  SensorWidget *tmp = m_first_sensor_widget;
  while (tmp) {
    tmp->requestStaticRefresh();
    tmp = tmp->m_next;
  }
}

void BaseScreen::processTouchEvents()
{
  int state = Display()->touch_Get(TOUCH_STATUS);
  if (state == NOTOUCH)
    return;

  DISPLAY_PRINTLN(">> BaseScreen::processTouchEvents()");

  if (state == TOUCH_PRESSED) {

    // Touch events have a tendency to 'bounce a bit'. We de-bounce by only starting a *new* touch series after a delay
    if (m_last_touch_time.elapsed() > TOUCH_DEBOUNCE_TIME) {
      m_touch_x1 = m_touch_x_start = Display()->touch_Get(TOUCH_GETX);
      m_touch_y1 = m_touch_y_start = Display()->touch_Get(TOUCH_GETY);
      m_swipe_mode = NO_SWIPE;

      TOUCH_PRINTLN("TOUCH_PRESSED");

      // save the start time of a touch event, so we can calculate the duration
      m_touch_start_time.start();
      m_touch_duration = 0;
    }

  } else if (m_touch_start_time.notStarted()) {

    TOUCH_PRINTLN("SWIPE_IGNORE_1");

  } else {

    // grab the new x and the y coordinates of the touch
    m_touch_x2 = Display()->touch_Get(TOUCH_GETX);
    m_touch_y2 = Display()->touch_Get(TOUCH_GETY);

    if (state == TOUCH_RELEASED) {
      TOUCH_PRINTLN("TOUCH_RELEASED");
      if ((m_touch_start_time.elapsed() > MINIMUM_TOUCH_DURATION) &&
          (m_swipe_mode != SWIPE_UD) &&
          (m_swipe_mode != SWIPE_LR) &&
          (m_swipe_mode != SWIPE_IGNORE)) {

        m_touch_duration = m_touch_start_time.elapsed();
      }

    } else if (state == TOUCH_MOVING) {
      if (m_swipe_mode == SWIPE_IGNORE) {
        // Ignore all swipe events
        TOUCH_PRINTLN("SWIPE_IGNORE_2");
        //       return;
      } else {
        // calculate in which direction we're making the biggest move: up/down or left/right?
        long swipe_x = m_touch_x2;
        swipe_x -= m_touch_x_start;
        long swipe_y = m_touch_y2;
        swipe_y -= m_touch_y_start;

        if (swipe_x != 0 || swipe_y != 0) {
          if (m_swipe_mode == NO_SWIPE) {

            TOUCH_PRINTLN("SWIPE_DETECTED");
            m_swipe_mode = SWIPE_DETECTED; // Do another loop to get more movement so we can better assess with direction we're swiping in

          } else if (m_swipe_mode == SWIPE_DETECTED) {

            long diff = abs(swipe_x) - abs(swipe_y);
            if (abs(diff) >= SWIPE_THRESHOLD) {
              if (abs(swipe_x) > abs(swipe_y)) {
                TOUCH_PRINTLN("SWIPE_LR");
                m_swipe_mode = SWIPE_LR;
              } else {
                TOUCH_PRINTLN("SWIPE_UD");
                m_swipe_mode = SWIPE_UD;
              }
            } else {
              TOUCH_PRINTLN("SWIPE BELOW THRESHOLD");
            }

          } else if (m_swipe_mode == SWIPE_LR) {

            m_touch_swipe_y = 0;
            long swipe_x = m_touch_x2;
            swipe_x -= m_touch_x1;
            m_touch_swipe_x = swipe_x / SWIPE_DIVIDER;
            m_touch_x1 += m_touch_swipe_x * SWIPE_DIVIDER;
            m_display_swiped = true;

          } else if (m_swipe_mode == SWIPE_UD) {

            m_touch_swipe_x = 0;
            long swipe_y = m_touch_y2;
            swipe_y -= m_touch_y1;
            m_touch_swipe_y = swipe_y / SWIPE_DIVIDER;
            m_touch_y1 += m_touch_swipe_y * SWIPE_DIVIDER;
            m_display_swiped = true;

          }
        }
      }
    }
  }

  m_last_touch_time.start();
  DISPLAY_PRINTLN("<< BaseScreen::processTouchEvents()");
}

bool BaseScreen::displayTapped(word &x, word &y, word &tapTime)
{
  if (m_touch_duration > 0) {
    x = m_touch_x1;
    y = m_touch_y1;
    tapTime = m_touch_duration;
    TOUCH_PRINT("Display tapped: ");
    TOUCH_PRINTLN(m_touch_duration);
    m_touch_duration = 0;
    return true;
  }
  return false;
}

bool BaseScreen::swipedUpOrDown(int &steps)
{
  if (m_swipe_mode == SWIPE_UD && m_display_swiped) {
    steps = m_touch_swipe_y;
    m_display_swiped = false;
#ifdef TOUCH_DEBUG
    if (m_touch_swipe_y < 0) {
      TOUCH_PRINT("Swiped up: ");
      TOUCH_PRINTLN(m_touch_swipe_y);
    } else if (m_touch_swipe_y > 0) {
      TOUCH_PRINT("Swiped down: ");
      TOUCH_PRINTLN(m_touch_swipe_y);
    }
#endif
    return true;
  }
  return false;
}

bool BaseScreen::swipedLeftOrRight(int &steps)
{
  if (m_swipe_mode == SWIPE_LR && m_display_swiped) {
    steps = m_touch_swipe_x;
    m_display_swiped = false;
#ifdef TOUCH_DEBUG
    if (m_touch_swipe_x < 0) {
      TOUCH_PRINT("Swiped left: ");
      TOUCH_PRINTLN(m_touch_swipe_x);
    } else if (m_touch_swipe_x > 0) {
      TOUCH_PRINT("Swiped right: ");
      TOUCH_PRINTLN(m_touch_swipe_x);
    }
#endif
    return true;
  }
  return false;
}

void BaseScreen::resetSwiped()
{
  TOUCH_PRINTLN("resetSwiped ...");
  //  m_last_touch_time = 0;
  //  m_touch_start_time = 0;
  m_swipe_mode = SWIPE_IGNORE;
  TOUCH_PRINTLN("resetSwiped ... done");
}

word BaseScreen::maxWidth()
{
  return display_max_x + 1;
}

word BaseScreen::maxHeight()
{
  return display_max_y + 1;
}

String BaseScreen::screenName()
{
  return m_screenName;
}

void BaseScreen::printPrepare(word x, word y, int color, byte fontSize)
{
  // TODO: don't change the settings if they are already the correct value
  Display()->gfx_MoveTo(x, y);
  if (fontSize >= MAX_FONT_SIZE) fontSize = MAX_FONT_SIZE - 1;
  Display()->txt_Width(fontSize);
  Display()->txt_Height(fontSize);
  Display()->txt_FGcolour(color);
}

void BaseScreen::printLabel(String label, word x, word y, int color, byte fontSize)
{
  printPrepare(x, y, color, fontSize);
  Display()->print(label);
}

void BaseScreen::printSubLabel(String subLabel, word x, word y, int color, byte fontSize)
{
  printPrepare(x, y, color, fontSize);
  Display()->print(subLabel);
}

void BaseScreen::printValue(String value, byte textLength, word x, word y, int color, byte fontSize)
{
  while (value.length() < textLength)
    value = " " + value;
  printPrepare(x, y, color, fontSize);
  Display()->print(value);
}

// ******************************************************************************************************
//                                              LEFT DISPLAY
// ******************************************************************************************************

Screen0::Screen0() : BaseScreen()
{
  m_screenName = "Left Screen";
}

void Screen0::init()
{
  BaseScreen::init();

  // Only create and add sensor objects the first time we call init.
  if (m_first_sensor_widget == 0) {
    DISPLAY_PRINTLN(">>> Screen0_init");
    printLabel("Position",
               100,
               4,
               PUMA_LABEL_COLOR);
    addSensorWidget(new PitchAndRollWidget(PID_PUMA_PITCH,
                                           PUMA_SENSOR_DATA_FONT_SIZE,
                                           10,
                                           8,
                                           true,
                                           7 ));
    addSensorWidget(new PitchAndRollWidget(PID_PUMA_ROLL,
                                           PUMA_SENSOR_DATA_FONT_SIZE,
                                           55,
                                           149,
                                           false,
                                           10 ));
    addSensorWidget(new CompassWidget(PID_PUMA_HEADING,
                                      PUMA_HEADING_FONT_SIZE,
                                      display_x_mid,
                                      top_divider / 2));

    TableWidget t1("TPMS", TableWidget::TOP_BORDER | TableWidget::SHOW_GRID, 2, 3,
                   left_border - 3,
                   top_divider,
                   right_border + 3,
                   bottom_border);

    addSensorWidget(new TpmsWidget(PID_PUMA_TPMS_FL_PRESS,
                                   TPMS_PRESSURE,
                                   PUMA_SENSOR_DATA_FONT_SIZE,
                                   t1.X(0),
                                   t1.Y(0)));
    addSensorWidget(new TpmsWidget(PID_PUMA_TPMS_FL_TEMP,
                                   TPMS_TEMPERATURE,
                                   PUMA_SENSOR_DATA_FONT_SIZE,
                                   t1.X(0),
                                   t1.Y(0)));

    addSensorWidget(new TpmsWidget(PID_PUMA_TPMS_FR_PRESS,
                                   TPMS_PRESSURE,
                                   PUMA_SENSOR_DATA_FONT_SIZE,
                                   t1.X(1),
                                   t1.Y(0)));
    addSensorWidget(new TpmsWidget(PID_PUMA_TPMS_FR_TEMP,
                                   TPMS_TEMPERATURE,
                                   PUMA_SENSOR_DATA_FONT_SIZE,
                                   t1.X(1),
                                   t1.Y(0)));

    addSensorWidget(new TpmsWidget(PID_PUMA_TPMS_RL_PRESS,
                                   TPMS_PRESSURE,
                                   PUMA_SENSOR_DATA_FONT_SIZE,
                                   t1.X(0),
                                   t1.Y(1)));
    addSensorWidget(new TpmsWidget(PID_PUMA_TPMS_RL_TEMP,
                                   TPMS_TEMPERATURE,
                                   PUMA_SENSOR_DATA_FONT_SIZE,
                                   t1.X(0),
                                   t1.Y(1)));

    addSensorWidget(new TpmsWidget(PID_PUMA_TPMS_RR_PRESS,
                                   TPMS_PRESSURE,
                                   PUMA_SENSOR_DATA_FONT_SIZE,
                                   t1.X(1),
                                   t1.Y(1)));
    addSensorWidget(new TpmsWidget(PID_PUMA_TPMS_RR_TEMP,
                                   TPMS_TEMPERATURE,
                                   PUMA_SENSOR_DATA_FONT_SIZE,
                                   t1.X(1),
                                   t1.Y(1)));

    addSensorWidget(new TpmsWidget(PID_PUMA_TPMS_TL_PRESS,
                                   TPMS_PRESSURE,
                                   PUMA_SENSOR_DATA_FONT_SIZE,
                                   t1.X(0),
                                   t1.Y(2)));
    addSensorWidget(new TpmsWidget(PID_PUMA_TPMS_TL_TEMP,
                                   TPMS_TEMPERATURE,
                                   PUMA_SENSOR_DATA_FONT_SIZE,
                                   t1.X(0),
                                   t1.Y(2)));

    addSensorWidget(new TpmsWidget(PID_PUMA_TPMS_TR_PRESS,
                                   TPMS_PRESSURE,
                                   PUMA_SENSOR_DATA_FONT_SIZE,
                                   t1.X(1),
                                   t1.Y(2)));
    addSensorWidget(new TpmsWidget(PID_PUMA_TPMS_TR_TEMP,
                                   TPMS_TEMPERATURE,
                                   PUMA_SENSOR_DATA_FONT_SIZE,
                                   t1.X(1),
                                   t1.Y(2)));
    DISPLAY_PRINTLN("<<< Screen0_init");
  }
}

byte Screen0::displayOrientation()
{
  return PORTRAIT;
}

// ******************************************************************************************************
//                                              CENTER DISPLAY
// ******************************************************************************************************

Screen1::Screen1() : BaseScreen()
{
  m_screenName = "Center Screen";
}

void Screen1::init()
{
  BaseScreen::init();

  word label_x_offset = 13;

  // Only create and add sensor objects the first time we call init.
  if (m_first_sensor_widget == 0) {
    DISPLAY_PRINTLN(">>> Screen1_init");
    addSensorWidget(new SensorWidget(PID_SPEED,
                                     PUMA_SPEED_FONT_SIZE,
                                     display_x_mid - (Display()->fontWidth(PUMA_SPEED_FONT_SIZE) * 1.7),
                                     145));
    addSensorWidget(new RpmDialWidget(PID_RPM,
                                      PUMA_RPM_FONT_SIZE,
                                      display_x_mid, RPM_RADIUS + 40,
                                      RPM_RADIUS));

    TableWidget t1("Temperature", TableWidget::RIGHT_BORDER | TableWidget::BOTTOM_BORDER, 1, 3,
                   left_border,
                   top_border,
                   left_divider,
                   display_max_y / 2);

    addSensorWidget(new SensorWidget(PID_AMBIENT_AIR_TEMP,
                                     2,
                                     label_x_offset,
                                     t1.Y(0)));
    addSensorWidget(new SensorWidget(PID_COOLANT_TEMP,
                                     2,
                                     label_x_offset,
                                     t1.Y(1)));
    addSensorWidget(new SensorWidget(PID_ENGINE_OIL_TEMP,
                                     2,
                                     label_x_offset,
                                     t1.Y(2)));
    //    addSensorWidget(new SensorWidget(PID_INTAKE_AIR_TEMP,
    //                                     2,
    //                                     label_x_offset,
    //                                     t1.Y(2)));

    TableWidget t2("Pressure", TableWidget::TOP_BORDER | TableWidget::RIGHT_BORDER, 1, 3,
                   left_border,
                   display_max_y / 2,
                   left_divider,
                   bottom_border);

    addSensorWidget(new SensorWidget(PID_FUEL_RAIL_GAUGE_PRESSURE,
                                     2,
                                     label_x_offset,
                                     t2.Y(0)));
    addSensorWidget(new SensorWidget(PID_INTAKE_MANIFOLD_PRESSURE,
                                     2,
                                     label_x_offset,
                                     t2.Y(1)));
    addSensorWidget(new SensorWidget(PID_BAROMETRIC_PRESSURE,
                                     2,
                                     label_x_offset,
                                     t2.Y(2)));

    TableWidget t3("Fuel", TableWidget::LEFT_BORDER | TableWidget::BOTTOM_BORDER, 1, 3,
                   right_divider,
                   top_border,
                   right_border,
                   display_max_y / 2);

    addSensorWidget(new SensorWidget(PID_FUEL_LEVEL,
                                     2,
                                     right_divider + label_x_offset,
                                     t3.Y(0)));  // Tank
    addSensorWidget(new SensorWidget(PID_CALCULATED_ENGINE_LOAD,        //PID_ENGINE_FUEL_RATE,
                                     2,
                                     right_divider + label_x_offset,
                                     t3.Y(1)));  // Economy
    addSensorWidget(new SensorWidget(PID_MAF_AIR_FLOW_RATE,             //PID_RANGE,
                                     2,
                                     right_divider + label_x_offset,
                                     t3.Y(2)));  // Range

    TableWidget t4("Distance", TableWidget::LEFT_BORDER, 1, 3,
                   right_divider,
                   display_max_y / 2,
                   right_border,
                   bottom_border);

    addSensorWidget(new SensorWidget(PID_PUMA_ODO,
                                     2,
                                     right_divider + label_x_offset,
                                     t4.Y(0)));
    addSensorWidget(new SensorWidget(PID_PUMA_TRIP,
                                     2,
                                     right_divider + label_x_offset,
                                     t4.Y(1)));
    addSensorWidget(new SensorWidget(PID_PUMA_LAST_SERVICE,
                                     2,
                                     right_divider + label_x_offset,
                                     t4.Y(2)));


    //    printLabel("Drivetrain", display_y_mid - 50, bottom_divider + 3, PUMA_LABEL_COLOR);
    //    printLabel("Torque", left_divider + 15, bottom_divider + 15, PUMA_LABEL_COLOR);
    //    printLabel("Power", left_divider + 15, bottom_divider + 50, PUMA_LABEL_COLOR);

    //    printLabel("Lockers", right_divider - 120, bottom_divider + 15, PUMA_LABEL_COLOR);
    //    printLabel("Front", right_divider - 120, bottom_divider + 50, PUMA_LABEL_COLOR);
    //    printLabel("Center", right_divider - 120, bottom_divider + 85, PUMA_LABEL_COLOR);
    //    printLabel("Rear", right_divider - 120, bottom_divider + 120, PUMA_LABEL_COLOR);

    addSensorWidget(new SensorWidget(PID_WARMS_UPS_SINCE_DTC_CLEARED,
                                     2,
                                     left_divider + 180,
                                     220));

    DISPLAY_PRINTLN("<<< Screen1_init");
  }
}

byte Screen1::displayOrientation()
{
  return LANDSCAPE;
}

// ******************************************************************************************************
//                                             RIGHT DISPLAY
// ******************************************************************************************************

Screen2::Screen2() : BaseScreen()
{
  m_screenName = "Right Screen";
}

void Screen2::init()
{
  BaseScreen::init();

  byte speed_size = PUMA_SENSOR_DATA_FONT_SIZE + 1;

  // Only create and add sensor objects the first time we call init.
  if (m_first_sensor_widget == 0) {
    DISPLAY_PRINTLN(">>> Screen2_init");
    printLabel("Speed Control", 80, 4, PUMA_LABEL_COLOR);
    addSensorWidget(new SensorWidget(PID_PUMA_CC_SPEED,
                                     speed_size,
                                     display_max_x / 2 - Display()->fontWidth(speed_size),
                                     45));        // Km/h
    addSensorWidget(new SensorWidget(PID_PUMA_CC_MODE,
                                     PUMA_SENSOR_DATA_FONT_SIZE,
                                     left_border,
                                     100));               // Mode: OFF, ARMED, ON
    addSensorWidget(new SensorWidget(PID_PUMA_CC_ACCELERATOR,
                                     PUMA_SENSOR_DATA_FONT_SIZE,
                                     left_border + 150,
                                     100));  // Throttle: 50%

    /*
       TableWidget t1("On-Board Diagnostics", TableWidget::TOP_BORDER, 1, 3,
                       left_border,
                       top_divider,
                       right_border,
                       bottom_border);

        //    addSensorWidget(new ListWidget("On-Board Diagnostics", PID_PUMA_DTC, 3, left_border, 120, display_max_x, display_max_y));
    */

    TableWidget t1("On-Board Diagnostics", TableWidget::TOP_BORDER, 2, 4,
                   left_border,
                   top_divider,
                   right_border,
                   bottom_border);

    addSensorWidget(new SensorWidget(PID_DISTANCE_WITH_MIL_ON,
                                     2,
                                     t1.X(0),
                                     t1.Y(0)));

    addSensorWidget(new SensorWidget(PID_RUN_TIME_WITH_MIL_ON,
                                     2,
                                     t1.X(1),
                                     t1.Y(0)));

    addSensorWidget(new SensorWidget(PID_DISTANCE_SINCE_DTC_CLEARED,
                                     2,
                                     t1.X(0),
                                     t1.Y(1)));

    addSensorWidget(new SensorWidget(PID_TIME_SINCE_DTC_CLEARED,
                                     2,
                                     t1.X(1),
                                     t1.Y(1)));

    addSensorWidget(new SensorWidget(PID_COMMANDED_EGR,
                                     2,
                                     t1.X(0),
                                     t1.Y(2)));

    addSensorWidget(new SensorWidget(PID_EGR_ERROR,
                                     2,
                                     t1.X(1),
                                     t1.Y(2)));

    addSensorWidget(new SensorWidget(PID_WARMS_UPS_SINCE_DTC_CLEARED,
                                     2,
                                     t1.X(0),
                                     t1.Y(3)));

    //    addSensorWidget(new SensorWidget(PID_,
    //                                     2,
    //                                     t1.X(1),
    //                                     t1.Y(3)));


    DISPLAY_PRINTLN("<<< Screen2_init");
  }
}

byte Screen2::displayOrientation()
{
  return PORTRAIT;
}


