/*
   Displays.cpp - Library for ...
 */

#include "VD-Const.h"
#include "Display.h"

#if (ARDUINO >= 100)
	#include "Arduino.h" // for Arduino 1.0
#else
	#include "WProgram.h" // for Arduino 23
#endif

BaseDisplay::BaseDisplay(Diablo_Serial_4DLib *Display)
{
  Display_ = Display;

  // Set D4 on Arduino to Output (4D Arduino Adaptor V2 - Display Reset)
  pinMode(uLCD_DISPLAY_RESETLINE, OUTPUT);
}

void BaseDisplay::reset()
{  
  digitalWrite(uLCD_DISPLAY_RESETLINE, 1);  // Reset the Display via D4
  delay(100);
  digitalWrite(uLCD_DISPLAY_RESETLINE, 0);  // unReset the Display via D4
  delay(5000);
}

void BaseDisplay::init()
{  
// Initialize the Display
  DisplaySerial.begin(DISPLAY_SPEED) ;

  Display_->TimeLimit4D = 5000 ; // 5 second timeout on all commands
  Display_->Callback4D = NULL ;
//  Display_->Callback4D = mycallback ;

  Display_->gfx_ScreenMode(displayOrientation());
  Display_->txt_FontID(FONT_3);
  display_max_width = Display_->gfx_Get(X_MAX) + 1;
  display_max_height = Display_->gfx_Get(Y_MAX) + 1;
  top_separator_line = display_max_height / 3;
  mid_separator_line = top_separator_line + (top_separator_line * 2 / 3);
  bottom_separator_line = top_separator_line + (top_separator_line * 4 / 3);
  start_x = 10;
  end_x = display_max_width - 11;
  mid_screen = display_max_width / 2;

  Display_->touch_Set(TOUCH_ENABLE);
}

bool BaseDisplay::touchPressed()
{
  int j = Display_->touch_Get(TOUCH_STATUS) ;
  return (j == TOUCH_PRESSED);
}

word BaseDisplay::maxWidth()
{
  return display_max_width;
}

word BaseDisplay::maxHeight()
{
  return display_max_height;  
}

bool BaseDisplay::updateNeeded(unsigned int &lastUpdate, word updateInterval)
{
  unsigned int tmp = millis();
  if (tmp - lastUpdate > updateInterval) {
    lastUpdate = tmp;
    return true;
  }
  return false;     
}

void BaseDisplay::updateStatusBar()
{  
  static unsigned int last_update = millis();
  unsigned int tmp = millis();
   
  Display_->txt_FGcolour(GREEN);
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
      Display_->gfx_MoveTo(0,470);
      Display_->print("                                 ");
    }
  }
  
  Display_->gfx_MoveTo(0,470);
  int z = tmp - last_update;
  if (z < 1000)
    Display_->print(" ");
  Display_->print(z);
  Display_->print(tick_string);
  last_update = tmp;
}

// ******************************************************************************************************
//                                              LEFT DISPLAY
// ******************************************************************************************************

LeftDisplay::LeftDisplay(Diablo_Serial_4DLib *Display, Direction *directionControl, Tpms *tpms) : BaseDisplay(Display)
{
  m_DirectionControl = directionControl;
  m_Tpms = tpms;
}

byte LeftDisplay::displayOrientation() 
{
  return PORTRAIT; 
}

void LeftDisplay::update()
{              
  updateCompass(m_DirectionControl->compass());
  updatePitch(10, 8, 7, m_DirectionControl->pitch());
  updateRoll(46, 149, 10, m_DirectionControl->roll());
  
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

void LeftDisplay::redrawLabels()
{
  Display_->txt_FGcolour(WHITE);
  Display_->gfx_MoveTo(120, top_separator_line + 3);
  Display_->print("TPMS");
  Display_->gfx_LinePattern(0x00aa);
  Display_->gfx_Line(start_x+20, mid_separator_line, end_x-20, mid_separator_line, WHITE);
  Display_->gfx_Line(start_x+20, bottom_separator_line, end_x-20, bottom_separator_line, WHITE);
  Display_->gfx_Line(mid_screen, top_separator_line + 32, mid_screen, display_max_height - 30, WHITE);
  
  Display_->gfx_MoveTo(100, 3);
  Display_->print("Direction");
  Display_->gfx_LinePattern(0);
  Display_->gfx_Line(start_x, top_separator_line, end_x, top_separator_line, WHITE);
}

void LeftDisplay::updatePitch(byte x, byte y, byte interleave, int angle)
{  
  static unsigned int last_update = 0;
  unsigned int tmp = millis();

  // Only update the static display items once every second
  if (tmp - last_update > 1000) {
    last_update = tmp;

    Display_->gfx_Line(x, y, x, y + interleave*18, WHITE);
  
    byte y_ = y;
    for (int i=-9; i<=9; i++) {
      int width = 2;
      if (i == 0  || abs(i) == 9)
        width = 8;
      else if (abs(i) == 2 || abs(i) == 4 || abs(i) == 6 || abs(i) == 8)
        width = 5; 
      Display_->gfx_Line(x, y_, x+width, y_, WHITE);
      y_ += interleave;
    }
  }

  int color = GREEN;
  if (abs(angle) > 35)
    color = RED;
  else if (abs(angle) > 15)
    color = YELLOW;

  Display_->gfx_MoveTo(x - 1, y + interleave*18 + 1);

  Display_->txt_Width(2);
  Display_->txt_Height(2);
  Display_->txt_FGcolour(color);
  Display_->print(abs(angle));    

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
  y_ = y + interleave*9 - round(f); 
  Display_->gfx_TriangleFilled(x_, y_, x_ + 20, y_ - 5, x_ + 20, y_ + 5, color); 
}

void LeftDisplay::updateRoll(byte x, byte y, byte interleave, int angle)
{
  static unsigned int last_update = 0;
  unsigned int tmp = millis();

  // Only update the static display items once every second
  if (tmp - last_update > 1000) {
    last_update = tmp;

    Display_->gfx_Line(x, y, x + interleave*18, y, WHITE);
    byte x_ = x;
    for (int i=-9; i<=9; i++) {
      int heigth = 2;
      if (i == 0  || abs(i) == 9)
        heigth = 8;
      else if (abs(i) == 2 || abs(i) == 4 || abs(i) == 6 || abs(i) == 8)
        heigth = 5; 
      Display_->gfx_Line(x_, y-heigth, x_, y, WHITE);
      x_ += interleave;
    }
  }
  
  int color = GREEN;
  if (abs(angle) > 35)
    color = RED;
  else if (abs(angle) > 15)
    color = YELLOW;

  Display_->gfx_MoveTo(x + interleave*18 + 4, y - 13);

  Display_->txt_Width(2);
  Display_->txt_Height(2);
  Display_->txt_FGcolour(color);
  Display_->print(abs(angle));    

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

  x_ = x + interleave*9 + round(f);
  y_ = y - 10;
  Display_->gfx_TriangleFilled(x_, y_, x_ - 5, y_ - 20, x_ + 5, y_ - 20, color); 
}

void LeftDisplay::updateCompass(word heading)
{  
  static long int old_heading = -1;
  static unsigned int last_update = 0;
  unsigned int tmp = millis();

  // Only update the static display items once every x seconds
  if (tmp - last_update > 2000) {
    if (heading != old_heading) {
      old_heading = heading;
      last_update = tmp;
      
      Display_->txt_FGcolour(GREEN);
      Display_->txt_Width(6);
      Display_->txt_Height(6);
      Display_->txt_Xgap(2);
      Display_->gfx_MoveTo(65,40);
      if (heading < 100)
        Display_->print("0");
      if (heading < 10)
        Display_->print("0");  
      Display_->print(heading);
      Display_->txt_Xgap(0);
    
      heading += 270;
      if (heading > 360)
        heading -= 360;
        
      int x_ = 245;
      int y_ = 25;
      int radius_ = 25;
      Display_->gfx_CircleFilled(x_, y_, radius_-1, BLACK);
      Display_->gfx_Circle(x_, y_, radius_, WHITE);
      Display_->gfx_MoveTo(x_, y_);
      word orbitX1, orbitY1;
      Display_->gfx_Orbit(heading, radius_, &orbitX1, &orbitY1);
      word orbitX2, orbitY2;
      Display_->gfx_Orbit(heading+150, radius_/2, &orbitX2, &orbitY2);
      word orbitX3, orbitY3;
      Display_->gfx_Orbit(heading+210, radius_/2, &orbitX3, &orbitY3);
      Display_->gfx_TriangleFilled(orbitX1, orbitY1, orbitX2, orbitY2, orbitX3, orbitY3, GREEN); 
    }
  }
}

void LeftDisplay::updateTPMSvalue(byte tireLocation)
{
  #define TPMS_X1_OFFSET 55
  #define TPMS_Y1_OFFSET 75
  #define TPMS_X2_OFFSET 85
  #define TPMS_Y2_OFFSET 30
  
  word x = 0;
  if (tireLocation == FRONT_RIGHT || tireLocation == REAR_RIGHT || tireLocation == TRAILER_RIGHT) {
    x = mid_screen;
  }

  word y = 0;
  if (tireLocation == FRONT_LEFT || tireLocation == FRONT_RIGHT) {
    y = top_separator_line;
  } else if (tireLocation == REAR_LEFT || tireLocation == REAR_RIGHT) {
    y = top_separator_line+106;
  } else if (tireLocation == TRAILER_LEFT || tireLocation == TRAILER_RIGHT) {
    y = top_separator_line+212;
  }

  Display_->gfx_Line(x + TPMS_X1_OFFSET, y + TPMS_Y1_OFFSET , x + TPMS_X2_OFFSET, y + TPMS_Y2_OFFSET, WHITE);
  
  Display_->txt_Width(2);
  Display_->txt_Height(2);

  int x1, y1, x2, y2;
  x1 = x + TPMS_X1_OFFSET / 2;
  y2 = y + TPMS_Y2_OFFSET;
  Display_->gfx_MoveTo(x1, y2);
  if (m_Tpms->tirePressureAlarm(tireLocation))
    Display_->txt_FGcolour(RED);
  else if (m_Tpms->tirePressureWarning(tireLocation))
    Display_->txt_FGcolour(YELLOW);
  else
    Display_->txt_FGcolour(GREEN);
  Display_->print(m_Tpms->tirePressure(tireLocation)); 
     
  x2 = x + TPMS_X2_OFFSET;
  y1 = y + TPMS_Y1_OFFSET - 20;
  Display_->gfx_MoveTo(x2, y1);
  if (m_Tpms->tireTemperatureAlarm(tireLocation))
    Display_->txt_FGcolour(RED);
  else if (m_Tpms->tireTemperatureWarning(tireLocation))
    Display_->txt_FGcolour(YELLOW);
  else
    Display_->txt_FGcolour(GREEN);
  Display_->print(m_Tpms->tireTemperature(tireLocation));    
}

// ******************************************************************************************************
//                                              CENTER DISPLAY
// ******************************************************************************************************

CenterDisplay::CenterDisplay(Diablo_Serial_4DLib *Display, OBD *obd) : BaseDisplay(Display)
{
  m_obd = obd;
}

void CenterDisplay::init()
{
  BaseDisplay::init();
  
  y_mid = maxHeight() / 2;
  x_mid = maxWidth() / 2;
  left_vertical = 160;
  right_vertical = maxWidth() - left_vertical;
  bottom_divider = maxHeight() - 130;
  label_x_offset = 13;
  label1_y_offset = 90;
  label2_y_offset = 160;
  label3_y_offset = 230;
}

#define rpm_radius 130

void CenterDisplay::update()
{                
  static unsigned int last_update = 0;
//  if (updateNeeded(last_update, 500))
    updateSpeed(m_obd->m_speed.value());
  updateRpm(m_obd->m_rpm.value());    
  updateStatusBar();
}

byte CenterDisplay::displayOrientation() 
{
  return LANDSCAPE; 
}

void CenterDisplay::redrawLabels()
{
  Display_->txt_FGcolour(WHITE);
  Display_->gfx_LinePattern(0);
  
//  Display_->gfx_MoveTo(x_mid - 120, 3);
//  Display_->print("Speed");

  Display_->gfx_MoveTo(x_mid, maxHeight() - 100);
  #define MAX_BUF 93
  word x_pos[MAX_BUF+2];
  word y_pos[MAX_BUF+2];
  word heading = 150;
  int c = 5;
  word R;
  for (int i=0; i<MAX_BUF; i++) {
      Display_->gfx_Orbit(heading, rpm_radius, &x_pos[i], &y_pos[i]);
      i++;
      R = rpm_radius + 4;
      if (c == 5) {
        R = rpm_radius + 10;
        c = 0;
      }
      Display_->gfx_Orbit(heading, R, &x_pos[i], &y_pos[i]);
      i++;
      x_pos[i] = x_pos[i-2];
      y_pos[i] = y_pos[i-2];
      heading += 8;
      c++;
  }
  
 #define _0000_pos 1
 #define _1000_pos 16
 #define _2000_pos 31
 #define _3000_pos 46
 #define _4000_pos 61
 #define _5000_pos 76
 #define _6000_pos 91

  Display_->gfx_Polyline(_4000_pos, &x_pos[0], &y_pos[0], WHITE);
  Display_->gfx_Polyline(MAX_BUF - _4000_pos, &x_pos[_4000_pos], &y_pos[_4000_pos], RED);

  Display_->gfx_MoveTo(x_pos[_0000_pos] - 15, y_pos[_0000_pos] - 5);
  Display_->txt_FGcolour(WHITE);
  Display_->print("0");
  Display_->gfx_MoveTo(x_pos[_1000_pos] - 35, y_pos[_1000_pos] - 7);
  Display_->print("1000");
  Display_->gfx_MoveTo(x_pos[_2000_pos] - 35, y_pos[_2000_pos] - 10);
  Display_->print("2000");
  Display_->gfx_MoveTo(x_pos[_3000_pos] - 15, y_pos[_3000_pos] - 14);
  Display_->print("3000");
  Display_->txt_FGcolour(RED);
  Display_->gfx_MoveTo(x_pos[_4000_pos] + 5, y_pos[_4000_pos] - 10);
  Display_->print("4000");
  Display_->gfx_MoveTo(x_pos[_5000_pos] + 5, y_pos[_5000_pos] - 7);
  Display_->print("5000");
  Display_->gfx_MoveTo(x_pos[_6000_pos] + 8, y_pos[_6000_pos] - 5);
  Display_->print("6000");

  Display_->txt_FGcolour(WHITE);
  Display_->gfx_MoveTo(x_mid + 50, 120);
  Display_->print("rpm");
  Display_->gfx_MoveTo(x_mid + 60, 230);
  Display_->print("Km/h");
  
/*  
  Display_->gfx_Line(left_vertical, 0, left_vertical, maxHeight()-1, WHITE);
  Display_->gfx_Line(right_vertical, 0, right_vertical, maxHeight()-1, WHITE);
  Display_->gfx_Line(0, y_mid, left_vertical, y_mid, WHITE);
  Display_->gfx_Line(right_vertical, y_mid, maxWidth() - 1, y_mid, WHITE);
  Display_->gfx_Line(left_vertical, bottom_divider, right_vertical, bottom_divider, WHITE);
  
  Display_->gfx_MoveTo(x_mid - 30, 3);
  Display_->print("Speed");
  Display_->gfx_MoveTo(left_vertical / 2 - 20, 3);
  Display_->print("Temp");
  Display_->gfx_MoveTo(label_x_offset, label1_y_offset);
  Display_->print("Air");
  Display_->gfx_MoveTo(label_x_offset, label2_y_offset);
  Display_->print("Engine");
  Display_->gfx_MoveTo(label_x_offset, label3_y_offset);
  Display_->print("Turbo");
  
  Display_->gfx_MoveTo(left_vertical / 2 - 40, y_mid + 3);
  Display_->print("Pressure");
  Display_->gfx_MoveTo(label_x_offset, label1_y_offset + y_mid);
  Display_->print("Fuel");
  Display_->gfx_MoveTo(label_x_offset, label2_y_offset + y_mid);
  Display_->print("Oil");
  Display_->gfx_MoveTo(label_x_offset, label3_y_offset + y_mid);
  Display_->print("Air");
  
  Display_->gfx_MoveTo(y_mid - 50, bottom_divider + 3);
  Display_->print("Drivetrain");
  Display_->gfx_MoveTo(left_vertical + 15, bottom_divider + 15);
  Display_->print("Torque");
  Display_->gfx_MoveTo(left_vertical + 15, bottom_divider + 50);
  Display_->print("Power");
  Display_->gfx_MoveTo(right_vertical - 120, bottom_divider + 15);
  Display_->print("Lockers");
  Display_->gfx_MoveTo(right_vertical - 120, bottom_divider + 50);
  Display_->print("F:");
  Display_->gfx_MoveTo(right_vertical - 120, bottom_divider + 85);
  Display_->print("C:");
  Display_->gfx_MoveTo(right_vertical - 120, bottom_divider + 120);
  Display_->print("R:");
  
  Display_->gfx_MoveTo(right_vertical + left_vertical / 2 - 20, 3);
  Display_->print("Fuel");
  Display_->gfx_MoveTo(right_vertical + label_x_offset, label1_y_offset);
  Display_->print("Tank");
  Display_->gfx_MoveTo(right_vertical + label_x_offset, label2_y_offset);
  Display_->print("Range");
  Display_->gfx_MoveTo(right_vertical + label_x_offset, label3_y_offset);
  Display_->print("Economy");
  
  Display_->gfx_MoveTo(right_vertical + left_vertical / 2 - 40, y_mid + 3);
  Display_->print("Distance");
  Display_->gfx_MoveTo(right_vertical + label_x_offset, label1_y_offset + y_mid);
  Display_->print("Odo");
  Display_->gfx_MoveTo(right_vertical + label_x_offset, label2_y_offset + y_mid);
  Display_->print("Trip");
  Display_->gfx_MoveTo(right_vertical + label_x_offset, label3_y_offset + y_mid);
  Display_->print("Last Service");
*/
}

void CenterDisplay::updateSpeed(word speed)
{
  word color = GREEN;
  if (speed > 110)
    color = RED;

  word y = 145;  

  static word last_speed;
  static word last_x;
  word new_x;
    
  Display_->txt_Width(7);
  Display_->txt_Height(7);

  if (last_speed != speed) {
    Display_->gfx_MoveTo(last_x, y);    
    Display_->txt_FGcolour(BLACK);
    Display_->print(last_speed);
    last_speed = speed;
  }
  
  if (speed < 10)
    new_x = x_mid - 25;
  else if (speed < 100)
    new_x = x_mid - 50;
  else
    new_x = x_mid - 80;
  last_x = new_x;
    
  Display_->gfx_MoveTo(new_x, y);
      Display_->txt_FGcolour(color);
  Display_->print(speed);  
}

void CenterDisplay::updateRpm(word rpm)
{
  word color = GREEN;
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
  Display_->gfx_MoveTo(x_mid, maxHeight() - 100);
  Display_->gfx_Orbit(rpm_heading, rpm_radius - 7, &x_pos[0], &y_pos[0]);
  Display_->gfx_Orbit(rpm_heading - 4, rpm_radius - 30, &x_pos[1], &y_pos[1]);
  Display_->gfx_Orbit(rpm_heading + 4, rpm_radius - 30, &x_pos[2], &y_pos[2]);
  Display_->gfx_TriangleFilled(x_pos[0], y_pos[0], x_pos[1], y_pos[1], x_pos[2], y_pos[2], color);

  Display_->txt_Width(3);
  Display_->txt_Height(3);
  
  word new_x;
  if (rpm >= 1000)
    new_x = x_mid - 58;
  else
    new_x = x_mid - 40;
  
  if (last_rpm != rpm) {
    Display_->gfx_MoveTo(last_x, 95);    
    Display_->txt_FGcolour(BLACK);
    Display_->print(last_rpm);
    last_rpm = rpm;
  }
  
  last_x = new_x;
  Display_->gfx_MoveTo(new_x, 95);    
  Display_->txt_FGcolour(color);
  Display_->print(rpm);
}

// ******************************************************************************************************
//                                             RIGHT DISPLAY
// ******************************************************************************************************

RightDisplay::RightDisplay(Diablo_Serial_4DLib *Display, CruiseCtrl *control) : BaseDisplay(Display)
{
  m_CruiseControl = control;
}

byte RightDisplay::displayOrientation() 
{
  return PORTRAIT; 
}

void RightDisplay::update()
{                
  static unsigned int last_update = 0;
  if (updateNeeded(last_update, 3000)) {
    updateCruiseControl();
    updateOBD2Status();
  }
  updateStatusBar();
}

void RightDisplay::redrawLabels()
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
  Display_->gfx_Line(start_x, top_separator_line, end_x, top_separator_line, WHITE);
  Display_->gfx_MoveTo(60, top_separator_line + 3);
  Display_->print("On-Board Diagnostics");
}

void RightDisplay::updateCruiseControl()
{  
  static unsigned int last_update = 0;
  unsigned int tmp = millis();

  // Only update the static display items once every second
  if (tmp - last_update > 1000) {
    last_update = tmp;
    Display_->txt_FGcolour(GREEN);
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

void RightDisplay::updateOBD2Status()
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

