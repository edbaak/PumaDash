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

void PumaDisplay::setup()
{
  reset();

  // Initialize the Displays. At the moment I'm only running on one display, but want to ensure I have sufficient memory for all three.
  DISPLAY_SERIAL1.begin(DISPLAY_SPEED) ;
//  DISPLAY_SERIAL2.begin(DISPLAY_SPEED) ;
//  DISPLAY_SERIAL3.begin(DISPLAY_SPEED) ;

  TimeLimit4D = 5000 ; // 5 second timeout on all commands
  Callback4D = NULL ;
  //  Display()->Callback4D = mycallback ;

  txt_FontID(FONT_3);
  touch_Set(TOUCH_ENABLE);

  for (byte i = 0; i < MAX_CHAR_SIZE; i++) {
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
  if (!g_init_display && m_screen0.touchPressed()) {
    g_active_screen++;
    if (g_active_screen > 2)
      g_active_screen = 0;
    g_init_display = true;
  }

  if (g_init_display) {
    g_init_display = false;
    activeScreen()->init();
  }
}

void PumaDisplay::updateSensor(OBDData *sensor)
{
  activeScreen()->updateSensor(sensor);
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
      Serial.println("Display reset. Waiting " + v2s("%d", word(new_ms)) + " ms");
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
  fontSize -= 1;
  if (fontSize < MAX_CHAR_SIZE)
    return m_font_width[fontSize];
  return 10;
}

word PumaDisplay::fontHeight(byte fontSize)
{
  fontSize -= 1;
  if (fontSize < MAX_CHAR_SIZE)
    return m_font_height[fontSize];
  return 10;
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
  m_first_sensor = 0;
}

void BaseScreen::init()
{
  display_max_y = 0;
  display_max_x = 0;
  m_first_sensor = 0;

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

}

void BaseScreen::addSensor(SensorWidget *sensor)
{
  //TODO: this might benefit from more automation, i.e. calculate the position of the sensor based on some standard definitions such as TOP_LEFT, MID_LEFT, etc,
  if (m_first_sensor == 0) {
    m_first_sensor = sensor;
    return;
  }

  SensorWidget *tmp = m_first_sensor;
  while (tmp) {
    if (tmp->m_next == 0) {
      tmp->m_next = sensor;
      return;
    }
    tmp = tmp->m_next;
  }
}

SensorWidget *BaseScreen::findSensor(word pid)
{
  SensorWidget *tmp = m_first_sensor;
  while (tmp) {
    if (tmp->m_pid == pid)
      return tmp;
    tmp = tmp->m_next;
  }
  return 0;
}

void BaseScreen::updateSensor(OBDData *sensor)
{
  SensorWidget *tmp = findSensor(sensor->pid());
  if (tmp)
    tmp->update(sensor);
}

bool BaseScreen::touchPressed()
{
  int j = Display()->touch_Get(TOUCH_STATUS) ;
  return (j == TOUCH_PRESSED);
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
  Display()->gfx_MoveTo(x, y);
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
  if (m_first_sensor == 0) {
    printLabel("Position", 100, 4, PUMA_LABEL_COLOR);
    addSensor(new PitchAndRollWidget(PID_PUMA_PITCH, PUMA_SENSOR_DATA_FONT_SIZE, 10, 8, true, 7 ));
    addSensor(new PitchAndRollWidget(PID_PUMA_ROLL, PUMA_SENSOR_DATA_FONT_SIZE, 55, 149, false, 10 ));
    addSensor(new CompassWidget(PID_PUMA_HEADING, PUMA_HEADING_FONT_SIZE, display_x_mid, top_divider / 2));

    TableWidget t1("TPMS", TableWidget::TOP_BORDER | TableWidget::SHOW_GRID, 2, 3,
                 left_border, 
                 top_divider, 
                 right_border,
                 bottom_border);

    addSensor(new TpmsWidget(PID_PUMA_TPMS_FL_PRESS, TPMS_PRESSURE, PUMA_SENSOR_DATA_FONT_SIZE, t1.cellX(0), t1.cellY(0)));
    addSensor(new TpmsWidget(PID_PUMA_TPMS_FL_TEMP, TPMS_TEMPERATURE, PUMA_SENSOR_DATA_FONT_SIZE, t1.cellX(0), t1.cellY(0)));

    addSensor(new TpmsWidget(PID_PUMA_TPMS_FR_PRESS, TPMS_PRESSURE, PUMA_SENSOR_DATA_FONT_SIZE, t1.cellX(1), t1.cellY(0)));
    addSensor(new TpmsWidget(PID_PUMA_TPMS_FR_TEMP, TPMS_TEMPERATURE, PUMA_SENSOR_DATA_FONT_SIZE, t1.cellX(1), t1.cellY(0)));

    addSensor(new TpmsWidget(PID_PUMA_TPMS_RL_PRESS, TPMS_PRESSURE, PUMA_SENSOR_DATA_FONT_SIZE, t1.cellX(0), t1.cellY(1)));
    addSensor(new TpmsWidget(PID_PUMA_TPMS_RL_TEMP, TPMS_TEMPERATURE, PUMA_SENSOR_DATA_FONT_SIZE, t1.cellX(0), t1.cellY(1)));

    addSensor(new TpmsWidget(PID_PUMA_TPMS_RR_PRESS, TPMS_PRESSURE, PUMA_SENSOR_DATA_FONT_SIZE, t1.cellX(1), t1.cellY(1)));
    addSensor(new TpmsWidget(PID_PUMA_TPMS_RR_TEMP, TPMS_TEMPERATURE, PUMA_SENSOR_DATA_FONT_SIZE, t1.cellX(1), t1.cellY(1)));

    addSensor(new TpmsWidget(PID_PUMA_TPMS_TL_PRESS, TPMS_PRESSURE, PUMA_SENSOR_DATA_FONT_SIZE, t1.cellX(0), t1.cellY(2)));
    addSensor(new TpmsWidget(PID_PUMA_TPMS_TL_TEMP, TPMS_TEMPERATURE, PUMA_SENSOR_DATA_FONT_SIZE, t1.cellX(0), t1.cellY(2)));

    addSensor(new TpmsWidget(PID_PUMA_TPMS_TR_PRESS, TPMS_PRESSURE, PUMA_SENSOR_DATA_FONT_SIZE, t1.cellX(1), t1.cellY(2)));
    addSensor(new TpmsWidget(PID_PUMA_TPMS_TR_TEMP, TPMS_TEMPERATURE, PUMA_SENSOR_DATA_FONT_SIZE, t1.cellX(1), t1.cellY(2)));
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
  if (m_first_sensor == 0) {
    addSensor(new SensorWidget(PID_SPEED, PUMA_SPEED_FONT_SIZE, display_x_mid - (Display()->fontWidth(PUMA_SPEED_FONT_SIZE) * 1.7), 145));
    addSensor(new RpmDialWidget(PID_RPM, PUMA_RPM_FONT_SIZE, display_x_mid, RPM_RADIUS + 40, RPM_RADIUS));

    TableWidget t1("Temperature", TableWidget::RIGHT_BORDER | TableWidget::BOTTOM_BORDER, 1, 3,
                   left_border, 
                   top_border, 
                   left_divider,
                   display_max_y / 2);
    addSensor(new SensorWidget(PID_AMBIENT_AIR_TEMP, 2, label_x_offset, t1.cellY(0)));
    addSensor(new SensorWidget(PID_COOLANT_TEMP, 2, label_x_offset, t1.cellY(1)));
    addSensor(new SensorWidget(PID_INTAKE_AIR_TEMP, 2, label_x_offset, t1.cellY(2)));

    TableWidget t2("Pressure", TableWidget::TOP_BORDER | TableWidget::RIGHT_BORDER, 1, 3,
                   left_border, 
                   display_max_y / 2, 
                   left_divider,
                   bottom_border);
    addSensor(new SensorWidget(PID_FUEL_PRESSURE, 2, label_x_offset, t2.cellY(0)));
    addSensor(new SensorWidget(PID_BAROMETRIC_PRESSURE, 2, label_x_offset, t2.cellY(1)));
    //    addSensor(new SensorWidget(PID_OIL, 2, label_x_offset, t2.cellY(2)));

    TableWidget t3("Fuel", TableWidget::LEFT_BORDER | TableWidget::BOTTOM_BORDER, 1, 3,
                   right_divider, 
                   top_border, 
                   right_border,
                   display_max_y / 2);
    addSensor(new SensorWidget(PID_FUEL_LEVEL, 2, right_divider + label_x_offset, t3.cellY(0))); // Tank
    addSensor(new SensorWidget(PID_ENGINE_FUEL_RATE, 2, right_divider + label_x_offset, t3.cellY(1)));  // Economy
    //    addSensor(new SensorWidget(PID_RANGE, 2, right_divider + label_x_offset, t3.cellY(2)));     // Range

    TableWidget t4("Distance", TableWidget::LEFT_BORDER, 1, 3,
                   right_divider, 
                   display_max_y / 2, 
                   right_border,
                   bottom_border);
    //    printLabel("Odo", right_divider + label_x_offset, label1_y_offset + display_y_mid, PUMA_LABEL_COLOR);
    //    printLabel("Trip", right_divider + label_x_offset, label2_y_offset + display_y_mid, PUMA_LABEL_COLOR);
    //    printLabel("Last Service", right_divider + label_x_offset, label3_y_offset + display_y_mid, PUMA_LABEL_COLOR);

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

  byte speed_size = PUMA_SENSOR_DATA_FONT_SIZE + 2;

  // Only create and add sensor objects the first time we call init.
  if (m_first_sensor == 0) {
    printLabel("Speed Control", 80, 4, PUMA_LABEL_COLOR);
    addSensor(new SensorWidget(PID_PUMA_CC_SPEED,
                               speed_size,
                               display_max_x / 2 - Display()->fontWidth(speed_size),
                               45));        // Km/h
    addSensor(new SensorWidget(PID_PUMA_CC_MODE, 
                               PUMA_SENSOR_DATA_FONT_SIZE, 
                               left_border, 
                               100));               // Mode: OFF, ARMED, ON
    addSensor(new SensorWidget(PID_PUMA_CC_ACCELERATOR, 
                               PUMA_SENSOR_DATA_FONT_SIZE, 
                               left_border + 150, 
                               100));  // Throttle: 50%
    TableWidget t1("On-Board Diagnostics", TableWidget::TOP_BORDER, 1, 3,
                   left_border, 
                   top_divider, 
                   right_border,
                   bottom_border);

    //    addSensor(new ListWidget("On-Board Diagnostics", PID_PUMA_DTC, 3, left_border, 120, display_max_x, display_max_y));
  }
}

byte Screen2::displayOrientation()
{
  return PORTRAIT;
}


