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
  g_active_screen = 0; // Define the default screen. We can change this by tapping the touchscreen

  // Set D4 on Arduino to Output (4D Arduino Adaptor V2 - Display Reset)
  pinMode(PIN_DISPLAY_RESET, OUTPUT);
}

void PumaDisplay::setup()
{
  reset();

  // Initialize the Display
  DISPLAY_SERIAL1.begin(DISPLAY_SPEED) ;

  m_screen0.setup_(this);
  m_screen1.setup_(this);
  m_screen2.setup_(this);

  TimeLimit4D = 5000 ; // 5 second timeout on all commands
  Callback4D = NULL ;
  //  m_display->Callback4D = mycallback ;

  txt_FontID(FONT_3);
  touch_Set(TOUCH_ENABLE);
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

void PumaDisplay::reset()
{
  digitalWrite(PIN_DISPLAY_RESET, 1);  // Reset the Display via D4
  delay(100);
  digitalWrite(PIN_DISPLAY_RESET, 0);  // unReset the Display via D4
  delay(5000);
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

void BaseScreen::setup_(PumaDisplay *disp)
{
  m_display = disp;
}

void BaseScreen::init()
{
  display_max_y = 0;
  display_max_x = 0;
  m_first_sensor = 0;

  // TODO: Make some of these into #defines
  m_display->gfx_Cls();
  m_display->gfx_ScreenMode(displayOrientation());

  display_max_x = m_display->gfx_Get(X_MAX);
  display_max_y = m_display->gfx_Get(Y_MAX);
  top_separator_line = display_max_y / 3;
  mid_separator_line = top_separator_line + (top_separator_line * 2 / 3);
  bottom_divider = top_separator_line + (top_separator_line * 4 / 3);
  left_border = 10;
  right_border = display_max_x - left_border;
  display_y_mid = display_max_y / 2;
  display_x_mid = display_max_x / 2;
  top_border = 10;
  bottom_border = display_max_y - top_border;

  for (byte i = 0; i < MAX_CHAR_SIZE; i++) {
    m_display->txt_Width(i+1);
    m_display->txt_Height(i+1);
    m_char_width[i] = m_display->charwidth('0');
    m_char_height[i] = m_display->charheight('0');
  }
}

word BaseScreen::charWidth(byte fontSize)
{
  fontSize-=1;
  if (fontSize < MAX_CHAR_SIZE)
    return m_char_width[fontSize];
  return 10;  
}

word BaseScreen::charHeight(byte fontSize)
{
  fontSize-=1;
  if (fontSize < MAX_CHAR_SIZE)
    return m_char_height[fontSize];
  return 10;  
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
  int j = m_display->touch_Get(TOUCH_STATUS) ;
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
  m_display->gfx_MoveTo(x, y);
  m_display->txt_Width(fontSize);
  m_display->txt_Height(fontSize);
  m_display->txt_FGcolour(color);
}

void BaseScreen::printLabel(String label, word x, word y, int color, byte fontSize)
{
  printPrepare(x, y, color, fontSize);
  m_display->print(label);
}

void BaseScreen::printSubLabel(String subLabel, word x, word y, int color, byte fontSize)
{
  printPrepare(x, y, color, fontSize);
  m_display->print(subLabel);
}

void BaseScreen::printValue(String value, byte textLength, word x, word y, int color, byte fontSize)
{
  // TODO: This can be done better
  byte fixedStringLength = 1;

  while (value.length() < fixedStringLength)
    value = " " + value;
  printPrepare(x, y, color, fontSize);
  m_display->print(value);
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
    printLabel("Position", 100, 3, PUMA_LABEL_COLOR);
    addSensor(new PitchAndRollWidget(PID_PUMA_PITCH, PUMA_SENSOR_DATA_FONT_SIZE, 10, 8, true, 7 ));
    addSensor(new PitchAndRollWidget(PID_PUMA_ROLL, PUMA_SENSOR_DATA_FONT_SIZE, 50, 149, false, 10 ));
    addSensor(new CompassWidget(PID_PUMA_HEADING, PUMA_HEADING_FONT_SIZE, 100, 100)); // TODO: set at correct x,y position
    
    Table t1("TPMS", Table::TOP_BORDER | Table::SHOW_GRID, 2, 3,
             left_border, left_divider_line,
             top_border, display_max_y / 2);
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
  
  left_divider_line = 135;
  right_divider_line = maxWidth() - left_divider_line;
  bottom_divider = maxHeight() - 130;
  word label_x_offset = 13;

  // Only create and add sensor objects the first time we call init.
  if (m_first_sensor == 0) {
    byte speed_size = 5;
    addSensor(new SensorWidget(PID_SPEED, speed_size, display_x_mid - (charWidth(speed_size) * 1.5), 145));
    addSensor(new RpmDialWidget(PID_RPM, 3, display_x_mid - (charWidth(3) * 2), 95, RPM_RADIUS));

    Table t1("Temperature", Table::RIGHT_BORDER | Table::BOTTOM_BORDER, 1, 3,
             left_border, left_divider_line,
             top_border, display_max_y / 2);
    addSensor(new SensorWidget(PID_AMBIENT_AIR_TEMP, 2, label_x_offset, t1.cellY(0)));
    addSensor(new SensorWidget(PID_COOLANT_TEMP, 2, label_x_offset, t1.cellY(1)));
    addSensor(new SensorWidget(PID_INTAKE_AIR_TEMP, 2, label_x_offset, t1.cellY(2)));

    Table t2("Pressure", Table::TOP_BORDER | Table::RIGHT_BORDER, 1, 3,
             left_border, left_divider_line,
             display_max_y / 2, bottom_border);
    addSensor(new SensorWidget(PID_FUEL_PRESSURE, 2, label_x_offset, t2.cellY(0)));
    addSensor(new SensorWidget(PID_BAROMETRIC_PRESSURE, 2, label_x_offset, t2.cellY(1)));
    //    addSensor(new SensorWidget(PID_OIL, 2, label_x_offset, t2.cellY(2)));

    Table t3("Fuel", Table::LEFT_BORDER | Table::BOTTOM_BORDER, 1, 3,
             right_divider_line, right_border,
             top_border, display_max_y / 2);
    addSensor(new SensorWidget(PID_FUEL_LEVEL, 2, right_divider_line + label_x_offset, t3.cellY(0))); // Tank
    addSensor(new SensorWidget(PID_ENGINE_FUEL_RATE, 2, right_divider_line + label_x_offset, t3.cellY(1)));  // Economy
    //    addSensor(new SensorWidget(PID_RANGE, 2, right_divider_line + label_x_offset, t3.cellY(2)));     // Range

    //    printLabel("Distance", right_divider_line + left_divider_line / 2 - 40, display_y_mid + 3, PUMA_LABEL_COLOR);
    //    printLabel("Odo", right_divider_line + label_x_offset, label1_y_offset + display_y_mid, PUMA_LABEL_COLOR);
    //    printLabel("Trip", right_divider_line + label_x_offset, label2_y_offset + display_y_mid, PUMA_LABEL_COLOR);
    //    printLabel("Last Service", right_divider_line + label_x_offset, label3_y_offset + display_y_mid, PUMA_LABEL_COLOR);


    //    printLabel("Drivetrain", display_y_mid - 50, bottom_divider + 3, PUMA_LABEL_COLOR);
    //    printLabel("Torque", left_divider_line + 15, bottom_divider + 15, PUMA_LABEL_COLOR);
    //    printLabel("Power", left_divider_line + 15, bottom_divider + 50, PUMA_LABEL_COLOR);

    //    printLabel("Lockers", right_divider_line - 120, bottom_divider + 15, PUMA_LABEL_COLOR);
    //    printLabel("Front", right_divider_line - 120, bottom_divider + 50, PUMA_LABEL_COLOR);
    //    printLabel("Center", right_divider_line - 120, bottom_divider + 85, PUMA_LABEL_COLOR);
    //    printLabel("Rear", right_divider_line - 120, bottom_divider + 120, PUMA_LABEL_COLOR);
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

  // Only create and add sensor objects the first time we call init.
  if (m_first_sensor == 0) {
 //   printLabel("Speed Control", 80, 30, PUMA_LABEL_COLOR);
//    addSensor(new SensorWidget(PID_CC_SPEED, 4, mid_separator_line, 50));        // Km/h
//    addSensor(new SensorWidget(PID_CC_MODE, 2, left_border, 100));         // Mode: OFF, ARMED, ON
//    addSensor(new SensorWidget(PID_CC_ACCELERATOR, 2, left_border + 150, 100));  // Throttle: 50%
//    addSensor(new ListWidget("On-Board Diagnostics", PID_DTC, 3, left_border, 120, display_max_x, display_max_y));
  }
}

byte Screen2::displayOrientation()
{
  return PORTRAIT;
}


