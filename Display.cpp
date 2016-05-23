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


// ******************************************************************************************************
//                                              PumaDisplay
// ******************************************************************************************************

PumaDisplay::PumaDisplay(Stream *virtualPort) : Diablo_Serial_4DLib(virtualPort)
{
  g_init_display = true;
  g_active_screen = 1; // Define the default screen. We can change this by tapping the touchscreen

  // Set D4 on Arduino to Output (4D Arduino Adaptor V2 - Display Reset)
  pinMode(PIN_DISPLAY_RESET, OUTPUT);
}

void PumaDisplay::setup(Direction *pos, Tpms *tpms, CruiseCtrl *speed, PumaOBD *obd)
{
  reset();

  m_position = pos;
  m_tpms = tpms;
  m_speed = speed;
  m_obd = obd;

  m_screen0.setup(this);
  m_screen1.setup(this);
  m_screen2.setup(this);

  // Initialize the Display
  DISPLAY_SERIAL1.begin(DISPLAY_SPEED) ;

  TimeLimit4D = 5000 ; // 5 second timeout on all commands
  Callback4D = NULL ;
  //  Display_->Callback4D = mycallback ;

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
}

void PumaDisplay::update()
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

  // TODO: activescreen update should be removed completely, and instead replace with update events driven by refreshed obd data
  activeScreen()->update();
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
  delay(3500);
}

// ******************************************************************************************************
//                                              BASE SCREEN
// ******************************************************************************************************

BaseScreen::BaseScreen()
{
  display_max_y = 0;
  display_max_x = 0;
  m_first = 0;
}

void BaseScreen::setup(PumaDisplay *disp)
{
  Display_ = disp;
}

void BaseScreen::init()
{
  // TODO: Make some of these into #defines
  Display_->gfx_Cls();
  Display_->gfx_ScreenMode(displayOrientation());

  display_max_x = Display_->gfx_Get(X_MAX);
  display_max_y = Display_->gfx_Get(Y_MAX);
  top_separator_line = display_max_y / 3;
  mid_separator_line = top_separator_line + (top_separator_line * 2 / 3);
  bottom_divider = top_separator_line + (top_separator_line * 4 / 3);
  left_border = 10;
  right_border = display_max_x - left_border;
  display_y_mid = display_max_y / 2;
  display_x_mid = display_max_x / 2;
  top_border = 10;
  bottom_border = display_max_y - top_border;

  for (byte i = 1; i <= MAX_CHAR_SIZE; i++) {
    Display_->txt_Width(i);
    Display_->txt_Height(i);
    char_width[i] = Display_->charwidth('0');
    char_height[i] = Display_->charheight('0');
  }
}

void BaseScreen::addSensor(SensorWidget *sensor)
{
  //TODO: this might benefit from more automation, i.e. calculate the position of the sensor based on some standard definitions such as TOP_LEFT, MID_LEFT, etc,
  if (m_first == 0) {
    m_first = sensor;
    return;
  }

  SensorWidget *tmp = m_first;
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
  SensorWidget *tmp = m_first;
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
  byte size = tmp->m_fontSize;
  word x1 = tmp->m_x;
  word y1 = tmp->m_y;
  if (tmp) {
    // TODO: optimize by only repainting label and sublabel when needed.
    if (sensor->label() != "") {
      printLabel(sensor->label(), x1, y1, PUMA_LABEL_COLOR, 1);
      x1 += char_width[1] * 2;
      y1 += char_height[1];
    }

    printValue(sensor->toString(), x1, y1, sensor->color(), size);

    if (sensor->subLabel() != "") {
      x1 = x1 + (char_width[1] / 2) + (char_width[size] * sensor->valueLength());
      y1 = y1 + char_height[size] - char_height[1];
      printLabel(sensor->subLabel(), x1, y1, PUMA_LABEL_COLOR, 1);
    }
  }
}

bool BaseScreen::touchPressed()
{
  int j = Display_->touch_Get(TOUCH_STATUS) ;
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

void BaseScreen::printPrepare(word x, word y, int color, int textSize)
{
  Display_->gfx_MoveTo(x, y);
  Display_->txt_Width(textSize);
  Display_->txt_Height(textSize);
  Display_->txt_FGcolour(color);
}

void BaseScreen::printLabel(String label, word x, word y, int color, int textSize)
{
  printPrepare(x, y, color, textSize);
  Display_->print(label);
}

void BaseScreen::printSubLabel(String subLabel, word x, word y, int color, int textSize)
{
  printPrepare(x, y, color, textSize);
  Display_->print(subLabel);
}

void BaseScreen::printValue(String value, word x, word y, int color, int textSize)
{
  // TODO: This can be done better
  byte fixedStringLength = 3;

  while (value.length() < fixedStringLength)
    value = " " + value;
  printPrepare(x, y, color, textSize);
  Display_->print(value);
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
  if (m_first == 0) {
    addSensor(new PitchAndRollWidget(Display_, PID_PITCH, PUMA_LABEL_SIZE, 10, 8, true, 7 ));
    addSensor(new PitchAndRollWidget(Display_, PID_ROLL, PUMA_LABEL_SIZE, 46, 149, true, 10 ));
    addSensor(new CompassWidget(Display_, PID_HEADING, PUMA_LABEL_SIZE, 100, 100)); // TODO: set at correct x,y position
  }
}

byte Screen0::displayOrientation()
{
  return PORTRAIT;
}

void Screen0::update()
{
  updateTPMSvalue(FRONT_LEFT);
  updateTPMSvalue(FRONT_RIGHT);
  updateTPMSvalue(REAR_LEFT);
  updateTPMSvalue(REAR_RIGHT);
  updateTPMSvalue(TRAILER_LEFT);
  updateTPMSvalue(TRAILER_RIGHT);
}

/*
  void Screen0::redrawLabels()
  {
  printLabel("TPMS", 120, top_separator_line + 3, PUMA_LABEL_COLOR);
  printLabel("Position", 100, 3, PUMA_LABEL_COLOR);
  }
*/

void Screen0::updateTPMSvalue(byte tireLocation)
{
#define TPMS_X1_OFFSET 55
#define TPMS_Y1_OFFSET 75
#define TPMS_X2_OFFSET 85
#define TPMS_Y2_OFFSET 30

  word x = 0;
  if (tireLocation == FRONT_RIGHT || tireLocation == REAR_RIGHT || tireLocation == TRAILER_RIGHT) {
    x = display_max_x / 2;
  }

  word y = 0;
  if (tireLocation == FRONT_LEFT || tireLocation == FRONT_RIGHT) {
    y = top_separator_line;
  } else if (tireLocation == REAR_LEFT || tireLocation == REAR_RIGHT) {
    y = top_separator_line + 106;
  } else if (tireLocation == TRAILER_LEFT || tireLocation == TRAILER_RIGHT) {
    y = top_separator_line + 212;
  }

  Display_->gfx_Line(x + TPMS_X1_OFFSET, y + TPMS_Y1_OFFSET , x + TPMS_X2_OFFSET, y + TPMS_Y2_OFFSET, PUMA_LABEL_COLOR);

  word x1, y1, x2, y2;
  x1 = x + TPMS_X1_OFFSET / 2;
  y2 = y + TPMS_Y2_OFFSET;
  int color = PUMA_NORMAL_COLOR;
  if (Display_->m_tpms->tirePressureAlarm(tireLocation))
    color = PUMA_ALARM_COLOR;
  else if (Display_->m_tpms->tirePressureWarning(tireLocation))
    color = PUMA_WARNING_COLOR;
  printValue(String(Display_->m_tpms->tirePressure(tireLocation)), x1, y2, color, 2);

  color = PUMA_NORMAL_COLOR;
  x2 = x + TPMS_X2_OFFSET;
  y1 = y + TPMS_Y1_OFFSET - 20;
  if (Display_->m_tpms->tireTemperatureAlarm(tireLocation))
    color = PUMA_ALARM_COLOR;
  else if (Display_->m_tpms->tireTemperatureWarning(tireLocation))
    color = PUMA_WARNING_COLOR;
  printValue(String(Display_->m_tpms->tireTemperature(tireLocation)), x2, y1, color, 2);
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
  label_x_offset = 13;

  // Only create and add sensor objects the first time we call init.
  if (m_first == 0) {
    byte speed_size = 5;
    addSensor(new SensorWidget(Display_, PID_SPEED, speed_size, display_x_mid - (char_width[speed_size] * 1.5), 145));
    addSensor(new RpmDialWidget(Display_, PID_RPM, 3, display_x_mid - (char_width[3] * 2), 95, RPM_RADIUS));

    Table t1(Display_, "Temperature", Table::RIGHT_BORDER | Table::BOTTOM_BORDER, 1, 3,
             left_border, left_divider_line,
             top_border, display_max_y / 2);
    addSensor(new SensorWidget(Display_, PID_AMBIENT_AIR_TEMP, 2, label_x_offset, t1.cellY(0)));
    addSensor(new SensorWidget(Display_, PID_COOLANT_TEMP, 2, label_x_offset, t1.cellY(1)));
    addSensor(new SensorWidget(Display_, PID_INTAKE_AIR_TEMP, 2, label_x_offset, t1.cellY(2)));

    Table t2(Display_, "Pressure", Table::TOP_BORDER | Table::RIGHT_BORDER, 1, 3,
             left_border, left_divider_line,
             display_max_y / 2, bottom_border);
    addSensor(new SensorWidget(Display_, PID_FUEL_PRESSURE, 2, label_x_offset, t2.cellY(0)));
    addSensor(new SensorWidget(Display_, PID_BAROMETRIC_PRESSURE, 2, label_x_offset, t2.cellY(1)));
    //    addSensor(new SensorWidget(PID_OIL, 2, label_x_offset, t2.cellY(2)));

    Table t3(Display_, "Fuel", Table::LEFT_BORDER | Table::BOTTOM_BORDER, 1, 3,
             right_divider_line, right_border,
             top_border, display_max_y / 2);
    addSensor(new SensorWidget(Display_, PID_FUEL_LEVEL, 2, right_divider_line + label_x_offset, t3.cellY(0))); // Tank
    addSensor(new SensorWidget(Display_, PID_ENGINE_FUEL_RATE, 2, right_divider_line + label_x_offset, t3.cellY(1)));  // Economy
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

byte Screen2::displayOrientation()
{
  return PORTRAIT;
}

void Screen2::update()
{
  updateCruiseControl();
  updateOBD2Status();
}

void Screen2::redrawLabels()
{
  Display_->txt_FGcolour(PUMA_LABEL_COLOR);

  Display_->gfx_MoveTo(80, 3);
  Display_->print("Cruise Control");
  Display_->gfx_MoveTo(180, 70);
  Display_->print("Km/h");
  Display_->gfx_MoveTo(20, 100);
  Display_->print("Mode");
  Display_->gfx_MoveTo(170, 100);
  Display_->print("Throttle");

  Display_->gfx_LinePattern(0);
  Display_->gfx_Line(left_border, top_separator_line, right_border, top_separator_line, PUMA_LABEL_COLOR);
  Display_->gfx_MoveTo(60, top_separator_line + 3);
  Display_->print("On-Board Diagnostics");
}

void Screen2::updateCruiseControl()
{
  static unsigned int last_update = 0;
  unsigned int tmp = millis();

  // Only update the static display items once every second
  if (tmp - last_update > 1000) {
    last_update = tmp;
    Display_->txt_FGcolour(PUMA_NORMAL_COLOR);
    Display_->gfx_MoveTo(80, 40);
    Display_->txt_Width(4);
    Display_->txt_Height(4);
    Display_->print("100");

    Display_->txt_Width(2);
    Display_->txt_Height(2);
    Display_->gfx_MoveTo(25, 115);
    Display_->print("ON");

    Display_->gfx_MoveTo(175, 115);
    Display_->print("50%");
  }
}

void Screen2::updateOBD2Status()
{
  static unsigned int last_update = 0;
  unsigned int tmp = millis();

  // Only update the static display items once every second
  if (tmp - last_update > 1000) {
    last_update = tmp;
    Display_->txt_FGcolour(PUMA_WARNING_COLOR);
    Display_->gfx_MoveTo(10, top_separator_line + 20);
    Display_->txt_Width(PUMA_LABEL_SIZE);
    Display_->txt_Height(PUMA_LABEL_SIZE);
    Display_->print("F1B3 Fuel Injector 1 Dirty");
  }
}

