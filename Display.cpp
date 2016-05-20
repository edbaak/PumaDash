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

#if (ARDUINO >= 100)
#include "Arduino.h" // for Arduino 1.0
#else
#include "WProgram.h" // for Arduino 23
#endif

#define rpm_radius 110

// ******************************************************************************************************
//                                              PumaDisplay
// ******************************************************************************************************

PumaDisplay::PumaDisplay(Stream * virtualPort, Direction *pos, Tpms *tpms, CruiseCtrl *speed, PumaOBD *obd) : Diablo_Serial_4DLib(virtualPort)
{
  m_position = pos;
  m_tpms = tpms;
  m_speed = speed;
  m_obd = obd;
  g_init_display = true;
  g_active_screen = 1; // Define the default screen. We can change this by tapping the touchscreen

  // Set D4 on Arduino to Output (4D Arduino Adaptor V2 - Display Reset)
  pinMode(PIN_DISPLAY_RESET, OUTPUT);
}

void PumaDisplay::setup()
{
  reset();

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
  if (g_active_screen == 0) {
    return &m_screen0;
  } else { 
    if (g_active_screen == 1)
      return &m_screen1;
  }
  
  return &m_screen2;    
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
    activeScreen()->redrawLabels();
  }

  activeScreen()->update();
}

void PumaDisplay::reset()
{
  digitalWrite(PIN_DISPLAY_RESET, 1);  // Reset the Display via D4
  delay(100);
  digitalWrite(PIN_DISPLAY_RESET, 0);  // unReset the Display via D4
  delay(4000);
}

// ******************************************************************************************************
//                                              BASE SCREEN
// ******************************************************************************************************

BaseScreen::BaseScreen()
{
  display_max_y = 0;
  display_max_x = 0;
}

void BaseScreen::setup(PumaDisplay *disp)
{
  Display_ = disp;
}

void BaseScreen::init()
{  
  Display_->gfx_Cls();
  Display_->gfx_ScreenMode(displayOrientation());

  display_max_x = Display_->gfx_Get(X_MAX);
  display_max_y = Display_->gfx_Get(Y_MAX);
  top_separator_line = display_max_y / 3;
  mid_separator_line = top_separator_line + (top_separator_line * 2 / 3);
  bottom_separator_line = top_separator_line + (top_separator_line * 4 / 3);
  left_border = 10;
  right_border = display_max_x - left_border;
  display_y_mid = display_max_y / 2;
  display_x_mid = display_max_x / 2;
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

bool BaseScreen::updateNeeded(unsigned int &lastUpdate, word updateInterval)
{
  unsigned int tmp = millis();
  if (tmp - lastUpdate > updateInterval) {
    lastUpdate = tmp;
    return true;
  }
  return false;
}

void BaseScreen::updateStatusBar()
{
  static unsigned int last_update = millis();
  unsigned int tmp = millis();

  Display_->txt_FGcolour(LIGHTGREEN);
  Display_->txt_Width(1);
  Display_->txt_Height(1);

  static unsigned int last_tick_update = last_update;
  static String tick_string = " ";

  // Only update the static display items once every second
  if (tmp - last_tick_update > 1000) {
    last_tick_update = tmp;
    tick_string += ".";

    if (tick_string.length() > 29) {
      tick_string = " ";
      Display_->gfx_MoveTo(0, 470);
      Display_->print("                                 ");
    }
  }

  Display_->gfx_MoveTo(0, 470);
  int z = tmp - last_update;
  if (z < 1000)
    Display_->print(" ");
  Display_->print(z);
  Display_->print(tick_string);
  last_update = tmp;
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

void BaseScreen::printValue(int value, word x, word y, int color, int textSize)
{
  printPrepare(x, y, color, textSize);
  Display_->print(value);
}

void BaseScreen::printValue(word value, word x, word y, int color, int textSize)
{
  printPrepare(x, y, color, textSize);
  Display_->print(value);
}

void BaseScreen::printValue(long value, word x, word y, int color, int textSize)
{
  printPrepare(x, y, color, textSize);
  Display_->print(value);
}

void BaseScreen::printValue(String value, word x, word y, int color, int textSize)
{
  printPrepare(x, y, color, textSize);
  Display_->print(value);
}

String BaseScreen::intToString(int value)
{
  String s(value);
  return s;
}

// ******************************************************************************************************
//                                              LEFT DISPLAY
// ******************************************************************************************************

Screen0::Screen0() : BaseScreen()
{
}

byte Screen0::displayOrientation()
{
  return PORTRAIT;
}

void Screen0::update()
{
  updateCompass(Display_->m_position->compass());
  updatePitch(10, 8, 7, Display_->m_position->pitch());
  updateRoll(46, 149, 10, Display_->m_position->roll());

  static unsigned int last_update = 0;
  if (updateNeeded(last_update, 3000)) {
    updateTPMSvalue(FRONT_LEFT);
    updateTPMSvalue(FRONT_RIGHT);
    updateTPMSvalue(REAR_LEFT);
    updateTPMSvalue(REAR_RIGHT);
    updateTPMSvalue(TRAILER_LEFT);
    updateTPMSvalue(TRAILER_RIGHT);
  }
  updateStatusBar();
}

void Screen0::redrawLabels()
{
  printLabel("TPMS", 120, top_separator_line + 3, WHITE);

  Display_->gfx_LinePattern(0x00aa);
  byte border = 30;
  Display_->gfx_Line(border, mid_separator_line, display_max_x - border, mid_separator_line, WHITE);
  Display_->gfx_Line(border, bottom_separator_line, display_max_x - border, bottom_separator_line, WHITE);
  Display_->gfx_Line(display_max_x / 2, top_separator_line + border, display_max_x / 2, display_max_x - border, WHITE);

  printLabel("Position", 100, 3, WHITE);

  Display_->gfx_LinePattern(0);
  Display_->gfx_Line(left_border, top_separator_line, right_border, top_separator_line, WHITE);
}

void Screen0::updatePitch(byte x, byte y, byte interleave, int angle)
{
  static unsigned int last_update = 0;
  unsigned int tmp = millis();

  // Only update the static display items once every second
  if (tmp - last_update > 1000) {
    last_update = tmp;

    Display_->gfx_Line(x, y, x, y + interleave * 18, WHITE);

    byte y_ = y;
    for (int i = -9; i <= 9; i++) {
      int width = 2;
      if (i == 0  || abs(i) == 9)
        width = 8;
      else if (abs(i) == 2 || abs(i) == 4 || abs(i) == 6 || abs(i) == 8)
        width = 5;
      Display_->gfx_Line(x, y_, x + width, y_, WHITE);
      y_ += interleave;
    }
  }

  int color = LIGHTGREEN;
  if (abs(angle) > 35)
    color = RED;
  else if (abs(angle) > 15)
    color = YELLOW;

  printValue(abs(angle), x - 1, y + interleave * 18 + 1, color, 2);

  if (angle > 45)
    angle = 45;
  else if (angle < -45)
    angle = -45;

  float f = angle;
  f = f * interleave / 5.0;
  static int x_ = 0;
  static int y_ = 0;

  // Reset display area
  if (x_ != 0)
    Display_->gfx_TriangleFilled(x_, y_, x_ + 20, y_ - 5, x_ + 20, y_ + 5, BLACK);

  x_ = x + 10;
  y_ = y + interleave * 9 - round(f);
  Display_->gfx_TriangleFilled(x_, y_, x_ + 20, y_ - 5, x_ + 20, y_ + 5, color);
}

void Screen0::updateRoll(byte x, byte y, byte interleave, int angle)
{
  static unsigned int last_update = 0;
  unsigned int tmp = millis();

  // Only update the static display items once every second
  if (tmp - last_update > 1000) {
    last_update = tmp;

    Display_->gfx_Line(x, y, x + interleave * 18, y, WHITE);
    byte x_ = x;
    for (int i = -9; i <= 9; i++) {
      int heigth = 2;
      if (i == 0  || abs(i) == 9)
        heigth = 8;
      else if (abs(i) == 2 || abs(i) == 4 || abs(i) == 6 || abs(i) == 8)
        heigth = 5;
      Display_->gfx_Line(x_, y - heigth, x_, y, WHITE);
      x_ += interleave;
    }
  }

  int color = LIGHTGREEN;
  if (abs(angle) > 35)
    color = RED;
  else if (abs(angle) > 15)
    color = YELLOW;

  printValue(abs(angle), x + interleave * 18 + 4, y - 13, color, 2);

  if (angle > 45)
    angle = 45;
  else if (angle < -45)
    angle = -45;

  float f = angle;
  f = f * interleave / 5.0;

  static int x_ = 0;
  static int y_ = 0;
  if (x_ != 0)
    Display_->gfx_TriangleFilled(x_, y_, x_ - 5, y_ - 20, x_ + 5, y_ - 20, BLACK);

  x_ = x + interleave * 9 + round(f);
  y_ = y - 10;
  Display_->gfx_TriangleFilled(x_, y_, x_ - 5, y_ - 20, x_ + 5, y_ - 20, color);
}

void Screen0::updateCompass(word heading)
{
  static long int old_heading = -1;
  static unsigned int last_update = 0;
  unsigned int tmp = millis();

  // Only update the static display items once every x seconds
  if (tmp - last_update > 2000) {
    if (heading != old_heading) {
      old_heading = heading;
      last_update = tmp;

      Display_->txt_Xgap(2);
      char hd[5];
      sprintf(hd, "%0d3", heading);
      
      Display_->txt_FGcolour(LIGHTGREEN);
      printValue(hd, 65, 40, LIGHTGREEN, 6);
      Display_->txt_Xgap(0);

      heading += 270;
      if (heading > 360)
        heading -= 360;

      int x_ = 245;
      int y_ = 25;
      int radius_ = 25;
      Display_->gfx_CircleFilled(x_, y_, radius_ - 1, BLACK);
      Display_->gfx_Circle(x_, y_, radius_, WHITE);
      Display_->gfx_MoveTo(x_, y_);
      word orbitX1, orbitY1;
      Display_->gfx_Orbit(heading, radius_, &orbitX1, &orbitY1);
      word orbitX2, orbitY2;
      Display_->gfx_Orbit(heading + 150, radius_ / 2, &orbitX2, &orbitY2);
      word orbitX3, orbitY3;
      Display_->gfx_Orbit(heading + 210, radius_ / 2, &orbitX3, &orbitY3);
      Display_->gfx_TriangleFilled(orbitX1, orbitY1, orbitX2, orbitY2, orbitX3, orbitY3, LIGHTGREEN);
    }
  }
}

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

  Display_->gfx_Line(x + TPMS_X1_OFFSET, y + TPMS_Y1_OFFSET , x + TPMS_X2_OFFSET, y + TPMS_Y2_OFFSET, WHITE);

  word x1, y1, x2, y2;
  x1 = x + TPMS_X1_OFFSET / 2;
  y2 = y + TPMS_Y2_OFFSET;
  int color = LIGHTGREEN;
  if (Display_->m_tpms->tirePressureAlarm(tireLocation))
    color = RED;
  else if (Display_->m_tpms->tirePressureWarning(tireLocation))
    color = YELLOW;
  printValue(uint16_t(Display_->m_tpms->tirePressure(tireLocation)), x1, y2, color, 2);

  color = LIGHTGREEN;
  x2 = x + TPMS_X2_OFFSET;
  y1 = y + TPMS_Y1_OFFSET - 20;
  if (Display_->m_tpms->tireTemperatureAlarm(tireLocation))
    color = RED;
  else if (Display_->m_tpms->tireTemperatureWarning(tireLocation))
    color = YELLOW;
  printValue(int32_t(Display_->m_tpms->tireTemperature(tireLocation)), x2, y1, color, 2);
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

  left_vertical = 160;
  right_vertical = maxWidth() - left_vertical;
  bottom_divider = maxHeight() - 130;
  label_x_offset = 13;
  big_border = 20;
  word z = (display_max_y - big_border) / 3;
  label1_y_offset = big_border;
  label2_y_offset = label1_y_offset + z;
  label3_y_offset = label2_y_offset + z;
}

void Screen1::update()
{
  updateSpeed(Display_->m_obd->dataObject(PID_SPEED)->byteValue());
  updateRpm(Display_->m_obd->dataObject(PID_RPM)->wordValue());
  updateTemperatures(Display_->m_obd->dataObject(PID_AMBIENT_AIR_TEMP)->longValue(), 
                    Display_->m_obd->dataObject(PID_COOLANT_TEMP)->longValue(), 
                    Display_->m_obd->dataObject(PID_INTAKE_AIR_TEMP)->longValue());
  updateStatusBar();
}

byte Screen1::displayOrientation()
{
  return LANDSCAPE;
}

void Screen1::redrawLabels()
{
  Display_->txt_FGcolour(WHITE);
  Display_->gfx_LinePattern(0);

  //  Display_->gfx_MoveTo(display_display_x_mid - 120, 3);
  //  Display_->print("Speed");

  Display_->gfx_MoveTo(display_x_mid, maxHeight() - 100);

  #define MAX_BUF 10
  word x_pos[MAX_BUF];
  word y_pos[MAX_BUF];
  word i = 0;
  for (word heading = 150; heading <= 390; heading += 40) {
    Display_->gfx_Orbit(heading, rpm_radius, &x_pos[i], &y_pos[i]);
    Display_->gfx_CircleFilled(x_pos[i], y_pos[i], 4, WHITE);
    i++;    
  }

  for (word heading = 170; heading < 390; heading += 40) {
    word x_start, y_start, x_end, y_end;
    Display_->gfx_Orbit(heading, rpm_radius, &x_start, &y_start);
    Display_->gfx_Orbit(heading, rpm_radius+10, &x_end, &y_end);
    Display_->gfx_Line(x_start, y_start, x_end, y_end, WHITE);
  }

  for (word heading = 150; heading < 390; heading += 40) {
    word x_start, y_start, x_end, y_end;
    for (word offset = 10; offset < 40; offset += 20) {
      Display_->gfx_Orbit(heading+offset, rpm_radius, &x_start, &y_start);
      Display_->gfx_Orbit(heading+offset, rpm_radius+5, &x_end, &y_end);
      Display_->gfx_Line(x_start, y_start, x_end, y_end, WHITE);
    }
  }

  printLabel("0", x_pos[0] - 23, y_pos[0] - 5, WHITE, 2);
  printLabel("1", x_pos[1] - 20, y_pos[1] - 10, WHITE, 2);
  printLabel("2", x_pos[2] - 25, y_pos[2] - 18, WHITE, 2);
  printLabel("3", x_pos[3] - 6, y_pos[3] - 30, WHITE, 2);
  printLabel("4", x_pos[4] + 10, y_pos[4] - 18, WHITE, 2);
  printLabel("5", x_pos[5] + 10, y_pos[5] - 10, WHITE, 2);
  printLabel("6", x_pos[6] + 9, y_pos[6] - 5, WHITE, 2);
  printLabel("rpm", display_x_mid + 50, 120, WHITE);
  printLabel("Km/h", display_x_mid + 60, 230, WHITE);

  /*
    Display_->gfx_Line(left_vertical, 0, left_vertical, maxHeight()-1, WHITE);
    Display_->gfx_Line(right_vertical, 0, right_vertical, maxHeight()-1, WHITE);
    Display_->gfx_Line(0, display_y_mid, left_vertical, display_y_mid, WHITE);
    Display_->gfx_Line(right_vertical, display_y_mid, maxWidth() - 1, display_y_mid, WHITE);
    Display_->gfx_Line(left_vertical, bottom_divider, right_vertical, bottom_divider, WHITE);

    printLabel("Speed", display_x_mid - 30, 3, WHITE);
    printLabel("Temp", left_vertical / 2 - 20, 3, WHITE);
  */  
    printLabel("Air", label_x_offset, label1_y_offset, WHITE);
    printLabel("Engine", label_x_offset, label2_y_offset, WHITE);
    printLabel("Turbo", label_x_offset, label3_y_offset, WHITE);

/*
    printLabel("Pressure", left_vertical / 2 - 40, display_y_mid + 3, WHITE);
    printLabel("Fuel", label_x_offset, label1_y_offset + display_y_mid, WHITE);
    printLabel("Oil", label_x_offset, label2_y_offset + display_y_mid, WHITE);
    printLabel("Air", label_x_offset, label3_y_offset + display_y_mid, WHITE);

    printLabel("Drivetrain", display_y_mid - 50, bottom_divider + 3, WHITE);
    printLabel("Torque", left_vertical + 15, bottom_divider + 15, WHITE);
    printLabel("Power", left_vertical + 15, bottom_divider + 50, WHITE);
    printLabel("Lockers", right_vertical - 120, bottom_divider + 15, WHITE);
    printLabel("F:", right_vertical - 120, bottom_divider + 50, WHITE);
    printLabel("C:", right_vertical - 120, bottom_divider + 85, WHITE);
    printLabel("R:", right_vertical - 120, bottom_divider + 120, WHITE);
    printLabel("Fuel", right_vertical + left_vertical / 2 - 20, 3, WHITE);
*/
    printLabel("Tank", right_vertical + label_x_offset, label1_y_offset, WHITE);
    printLabel("Range", right_vertical + label_x_offset, label2_y_offset, WHITE);
    printLabel("Economy", right_vertical + label_x_offset, label3_y_offset, WHITE);

/*
    printLabel("Distance", right_vertical + left_vertical / 2 - 40, display_y_mid + 3, WHITE);
    printLabel("Odo", right_vertical + label_x_offset, label1_y_offset + display_y_mid, WHITE);
    printLabel("Trip", right_vertical + label_x_offset, label2_y_offset + display_y_mid, WHITE);
    printLabel("Last Service", right_vertical + label_x_offset, label3_y_offset + display_y_mid, WHITE);
*/
}

void Screen1::updateTemperatures(long air, long engine, long turbo)
{
    word x_value_offset = 20;
    word y_value_offset = 20;
    printValue(air, label_x_offset + x_value_offset, label1_y_offset + y_value_offset, LIGHTGREEN, 2);
    printValue(engine, label_x_offset + x_value_offset, label2_y_offset + y_value_offset, LIGHTGREEN, 2);
    printValue(turbo, label_x_offset + x_value_offset, label3_y_offset + y_value_offset, LIGHTGREEN, 2);
}

void Screen1::updateSpeed(word speed)
{
  word color = LIGHTGREEN;
  if (speed > 110)
    color = RED;

  word y = 145;

  static word last_speed;
  static word last_x;
  word new_x;

  if (last_speed != speed) {
    printValue(last_speed, last_x, y, BLACK, 7);
    last_speed = speed;
  }

  if (speed < 10)
    new_x = display_x_mid - 25;
  else if (speed < 100)
    new_x = display_x_mid - 50;
  else
    new_x = display_x_mid - 80;
  last_x = new_x;

  printValue(speed, new_x, y, color, 7);
}

void Screen1::updateRpm(word rpm)
{
  word color = LIGHTGREEN;
  if (rpm >= 4000)
    color = RED;
  else if (rpm < 1200)
    color = YELLOW;

  static word last_rpm = 0;
  static word last_x;
  static word x_pos[3] = {0, 0, 0};
  static word y_pos[3] = {0, 0, 0};

  if (x_pos[0] != 0 && last_rpm != rpm)
    Display_->gfx_TriangleFilled(x_pos[0], y_pos[0], x_pos[1], y_pos[1], x_pos[2], y_pos[2], BLACK);

  word rpm_heading = 150 + round(rpm * 40.0 / 1000.0);
  Display_->gfx_MoveTo(display_x_mid, maxHeight() - 100);
  Display_->gfx_Orbit(rpm_heading, rpm_radius - 7, &x_pos[0], &y_pos[0]);
  Display_->gfx_Orbit(rpm_heading - 4, rpm_radius - 30, &x_pos[1], &y_pos[1]);
  Display_->gfx_Orbit(rpm_heading + 4, rpm_radius - 30, &x_pos[2], &y_pos[2]);
  Display_->gfx_TriangleFilled(x_pos[0], y_pos[0], x_pos[1], y_pos[1], x_pos[2], y_pos[2], color);

  word new_x;
  if (rpm >= 1000)
    new_x = display_x_mid - 58;
  else
    new_x = display_x_mid - 40;

  if (last_rpm != rpm) {
    printValue(last_rpm, last_x, 95, BLACK, 3);
    last_rpm = rpm;
  }

  last_x = new_x;
  printValue(rpm, new_x, 95, color, 3);
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
  static unsigned int last_update = 0;
  if (updateNeeded(last_update, 3000)) {
    updateCruiseControl();
    updateOBD2Status();
  }
  updateStatusBar();
}

void Screen2::redrawLabels()
{
  Display_->txt_FGcolour(WHITE);

  Display_->gfx_MoveTo(80, 3);
  Display_->print("Cruise Control");
  Display_->gfx_MoveTo(180, 70);
  Display_->print("Km/h");
  Display_->gfx_MoveTo(20, 100);
  Display_->print("Mode");
  Display_->gfx_MoveTo(170, 100);
  Display_->print("Throttle");

  Display_->gfx_LinePattern(0);
  Display_->gfx_Line(left_border, top_separator_line, right_border, top_separator_line, WHITE);
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
    Display_->txt_FGcolour(LIGHTGREEN);
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
    Display_->txt_FGcolour(YELLOW);
    Display_->gfx_MoveTo(10, top_separator_line + 20);
    Display_->txt_Width(1);
    Display_->txt_Height(1);
    Display_->print("F1B3 Fuel Injector 1 Dirty");
  }
}

