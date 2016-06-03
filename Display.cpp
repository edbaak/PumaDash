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

#ifdef DISPLAY_DEBUG1
#define DISPLAY_PRINTLN1(s) Serial.println(s); Serial.flush();
#else
#define DISPLAY_PRINTLN1(s) {}
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
  m_static_refresh_timer = 0;

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
  DISPLAY_PRINTLN1(">> PumaDisplay::processTouchEvents()");
  m_screen0.processTouchEvents();

  word x, y;
  if (m_screen0.displayTapped(x, y)) {
  }

  int steps;
  if (!g_init_display && m_screen0.swipedLeftOrRight(steps)) {
    m_screen0.resetSwiped();
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
    activeScreen()->init();
    activeScreen()->requestStaticRefresh();
  }

  if (m_screen0.swipedUpOrDown(steps)) {
    m_displayContrast -= steps;
    if (m_displayContrast > 15) m_displayContrast = 15;
    if (m_displayContrast < 1) m_displayContrast = 1;
    gfx_Contrast(m_displayContrast);
  }
  DISPLAY_PRINTLN1("<< PumaDisplay::processTouchEvents()");
}

void PumaDisplay::updateStatusbar()
{
  unsigned long cur_time = millis();

  static unsigned long timer = 0;
  String out = String(cur_time - timer);
  while (out.length() < 4) out = out + " ";

  word y_ = activeScreen()->maxHeight() - fontHeight(1);
  printLabel(out, 0, y_, WHITE, 1);

  word char_width = fontWidth(1);
  word _logfile_x = activeScreen()->maxWidth() - (9 * char_width);

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

  if (cur_time - m_static_refresh_timer > 10000) {
    m_static_refresh_timer = cur_time;
    activeScreen()->requestStaticRefresh();

    // While are are at it, refresh the log file name in the display status bar as well
    Display()->gfx_MoveTo(_logfile_x, y_);
    Display()->print(uniqueLogFileName());
  }

  timer = cur_time;
}

void PumaDisplay::updateSensorWidget(OBDData * sensor)
{
  activeScreen()->updateSensorWidget(sensor);
}

void PumaDisplay::reset(word ms)
{
  static unsigned long display_startup_delay_timer = 0;
  if (display_startup_delay_timer != 0) {
    // If we get in here we've called reset before with a ms of 0, indicating that we want a reset but but wanted to postpone a delay whilst doing other - more usefull - stuff.
    // Now that we're back, we can calculate how much of the recommended 5000 ms is left to delay.
    long new_ms = ms - (millis() - display_startup_delay_timer);
    display_startup_delay_timer = 0; // Make sure the next reset is handled normally
    if (new_ms > 0) {
      Serial.println(String("Display reset. Waiting ") + v2s("%d", word(new_ms)) + String(" ms"));
      delay(word(new_ms));
    }
    return;
  }

  digitalWrite(PIN_DISPLAY_RESET, 1);  // Reset the Display via D4
  delay(100);
  digitalWrite(PIN_DISPLAY_RESET, 0);  // unReset the Display via D4

  if (ms == 0)
    display_startup_delay_timer = millis();
  else
    delay(ms);
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
  display_max_y = 0;
  display_max_x = 0;
  m_first_sensor_widget = 0;

  // TODO: Make some of these into #defines
  Display()->gfx_Cls();
  Display()->gfx_ScreenMode(displayOrientation());

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
  m_display_tapped = false;
  m_display_swiped = false;
  m_touch_move_x = 0;
  m_touch_move_y = 0;
  m_touch_start = 0;
  m_touch_end = 0;
  m_swipe_mode = SWIPE_UNKNOWN;
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

void BaseScreen::updateSensorWidget(OBDData * sensor)
{
  SensorWidget *tmp = findSensorWidget(sensor->pid());
  if (tmp)
    tmp->update(sensor);
}

void BaseScreen::requestStaticRefresh()
{
  SensorWidget *tmp = m_first_sensor_widget;
  while (tmp) {
    tmp->requestStaticRefresh();
    tmp = tmp->m_next;
  }
}

#define TOUCH_DIVIDER 6
void BaseScreen::processTouchEvents()
{
  static unsigned long touch_timer = 0;
  unsigned long cur_time = millis();
//  if (cur_time - touch_timer < 40) return;
  if (cur_time - m_touch_end > 100) m_touch_end = 0;
  touch_timer = cur_time;
  DISPLAY_PRINTLN1(">> BaseScreen::processTouchEvents()");

  int state = Display()->touch_Get(TOUCH_STATUS);
  if (state == TOUCH_PRESSED) {
    if (m_touch_end == 0) {
      Serial.println("TOUCH_PRESSED");
      m_touch_x1 = Display()->touch_Get(TOUCH_GETX);
      m_touch_x_start = m_touch_x1;
      m_touch_y1 = Display()->touch_Get(TOUCH_GETY);
      m_touch_y_start = m_touch_y1;
      m_touch_start = cur_time;
      m_display_tapped = false;
      m_swipe_mode = SWIPE_UNKNOWN;
    } else {
      Serial.println("TOUCH PRESSED, continuing previous trace");
    }

  } else if (m_touch_start > 0) {

    // grab the new x and the y coordinates of the touch
    m_touch_x2 = Display()->touch_Get(TOUCH_GETX);
    m_touch_y2 = Display()->touch_Get(TOUCH_GETY);

    if (state == TOUCH_RELEASED) {
      Serial.println("TOUCH_RELEASED");

      if (cur_time - m_touch_start > 40 && cur_time - m_touch_start < 400 && m_swipe_mode != SWIPE_UD & m_swipe_mode != SWIPE_LR) {
        Serial.println("TAPPED");
        m_display_tapped = true;
      } else {
        Serial.println(cur_time - m_touch_start);
      }
      m_touch_end = cur_time;

    } else if (state == TOUCH_MOVING) {
      // calculate in which direction we're making the biggest move: up/down or left/right?
      long move_x = m_touch_x2;
      move_x -= m_touch_x_start;
      long move_y = m_touch_y2;
      move_y -= m_touch_y_start;

      if (move_x != 0 || move_y != 0) {
        if (m_swipe_mode == SWIPE_UNKNOWN) {
          Serial.println("SWIPE_UNKNOWN");
          m_swipe_mode = SWIPE_DETECTED; // Do another loop to get more movement so we can better assess with direction we're swiping in
        } else if (m_swipe_mode == SWIPE_DETECTED) {
          Serial.println("SWIPE_DETECTED");
          long diff = abs(move_x) - abs(move_y);
          Serial.print(">>>>>>>>>>>> ");
          Serial.println(diff);
          if (abs(diff) >= 10) {
            if (abs(move_x) > abs(move_y))
              m_swipe_mode = SWIPE_LR;
            else
              m_swipe_mode = SWIPE_UD;
          }
        } else if (m_swipe_mode == SWIPE_LR) {
//          Serial.println("SWIPE_LR");
          m_touch_move_y = 0;
          long move_x = m_touch_x2;
          move_x -= m_touch_x1;
          m_touch_move_x = move_x / TOUCH_DIVIDER;
          m_touch_x1 += m_touch_move_x * TOUCH_DIVIDER;
          m_display_swiped = true;
        } else if (m_swipe_mode == SWIPE_UD) {
//          Serial.println("SWIPE_UD");
          m_touch_move_x = 0;
          long move_y = m_touch_y2;
          move_y -= m_touch_y1;
          m_touch_move_y = move_y / TOUCH_DIVIDER;
          m_touch_y1 += m_touch_move_y * TOUCH_DIVIDER;
          m_display_swiped = true;
        }

#ifdef TOUCH_DEBUG
//        if (m_display_swiped) {
//          Serial.print("x move: ");
//          Serial.print(m_touch_move_x);
//          Serial.print(",  y move: ");
//          Serial.print(m_touch_move_y);
//          if (m_touch_move_y != 0)
//            Serial.println(" UP/DOWN");
//          else if (m_touch_move_x != 0)
//            Serial.println(" LEFT/RIGHT");
//          else
//            Serial.println("");
//        }
#endif
      } else {
        Serial.println("TOUCH_MOVING: Not enough movement");
      }
    }
  }
  DISPLAY_PRINTLN1("<< BaseScreen::processTouchEvents()");
}

bool BaseScreen::displayTapped(word &x, word &y)
{
  if (m_display_tapped) {
    m_display_tapped = false;
    x = m_touch_x1;
    y = m_touch_y1;
#ifdef TOUCH_DEBUG
    Serial.println("Display tapped");
#endif
    return true;
  }
  return false;
}

bool BaseScreen::swipedUpOrDown(int &steps)
{
  if (m_swipe_mode == SWIPE_UD && m_display_swiped) {
    steps = m_touch_move_y;
    m_display_swiped = false;
#ifdef TOUCH_DEBUG
    if (m_touch_move_y < 0) {
      Serial.print("Swiped up: ");
      Serial.println(m_touch_move_y);
    } else if (m_touch_move_y > 0) {
      Serial.print("Swiped down: ");
      Serial.println(m_touch_move_y);
    }
#endif
    return true;
  }
  return false;
}

bool BaseScreen::swipedLeftOrRight(int &steps)
{
  if (m_swipe_mode == SWIPE_LR && m_display_swiped) {
    steps = m_touch_move_x;
    m_display_swiped = false;
#ifdef TOUCH_DEBUG
    if (m_touch_move_x < 0) {
      Serial.print("Swiped left: ");
      Serial.println(m_touch_move_x);
    } else if (m_touch_move_x > 0) {
      Serial.print("Swiped right: ");
      Serial.println(m_touch_move_x);
    }
#endif
    return true;
  }
  return false;
}

void BaseScreen::resetSwiped()
{
  m_touch_start = 0;
}

word BaseScreen::maxWidth()
{
  return display_max_x + 1;
}

word BaseScreen::maxHeight()
{
  return display_max_y + 1;
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
}

void Screen0::init()
{
  BaseScreen::init();

  // Only create and add sensor objects the first time we call init.
  if (m_first_sensor_widget == 0) {
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
                   left_border,
                   top_divider,
                   right_border,
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
}

void Screen1::init()
{
  BaseScreen::init();

  word label_x_offset = 13;

  // Only create and add sensor objects the first time we call init.
  if (m_first_sensor_widget == 0) {
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
    addSensorWidget(new SensorWidget(PID_INTAKE_AIR_TEMP,
                                     2,
                                     label_x_offset,
                                     t1.Y(2)));

    TableWidget t2("Pressure", TableWidget::TOP_BORDER | TableWidget::RIGHT_BORDER, 1, 3,
                   left_border,
                   display_max_y / 2,
                   left_divider,
                   bottom_border);

    addSensorWidget(new SensorWidget(PID_FUEL_PRESSURE,
                                     2,
                                     label_x_offset,
                                     t2.Y(0)));
    addSensorWidget(new SensorWidget(PID_BAROMETRIC_PRESSURE,
                                     2,
                                     label_x_offset,
                                     t2.Y(1)));
    //    addSensorWidget(new SensorWidget(PID_OIL, 2, label_x_offset, t2.Y(2)));

    TableWidget t3("Fuel", TableWidget::LEFT_BORDER | TableWidget::BOTTOM_BORDER, 1, 3,
                   right_divider,
                   top_border,
                   right_border,
                   display_max_y / 2);

    addSensorWidget(new SensorWidget(PID_FUEL_LEVEL,
                                     2,
                                     right_divider + label_x_offset,
                                     t3.Y(0))); // Tank
    addSensorWidget(new SensorWidget(PID_ENGINE_FUEL_RATE,
                                     2,
                                     right_divider + label_x_offset,
                                     t3.Y(1)));  // Economy
    //    addSensorWidget(new SensorWidget(PID_RANGE,
    //                            2,
    //                            right_divider + label_x_offset,
    //                            t3.Y(2)));     // Range

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
}

void Screen2::init()
{
  BaseScreen::init();

  byte speed_size = PUMA_SENSOR_DATA_FONT_SIZE + 1;

  // Only create and add sensor objects the first time we call init.
  if (m_first_sensor_widget == 0) {
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

    TableWidget t1("On-Board Diagnostics", TableWidget::TOP_BORDER, 1, 3,
                   left_border,
                   top_divider,
                   right_border,
                   bottom_border);

    //    addSensorWidget(new ListWidget("On-Board Diagnostics", PID_PUMA_DTC, 3, left_border, 120, display_max_x, display_max_y));
  }
}

byte Screen2::displayOrientation()
{
  return PORTRAIT;
}


