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

class BaseDisplay
{
public:
  BaseDisplay(Diablo_Serial_4DLib *Display);

  virtual void redrawLabels() {};
  virtual byte displayOrientation() { return 0; };
  
  void reset();
  virtual void init();
  word maxWidth();
  word maxHeight();  
  bool updateNeeded(unsigned int &lastUpdate, word updateInterval);
  void updateStatusBar();
  bool touchPressed();

protected:
  Diablo_Serial_4DLib *Display_;
  word display_max_width = 0;
  word display_max_height = 0;
  int top_separator_line;
  int mid_separator_line;
  int bottom_separator_line;
  int start_x;
  int end_x;
  int mid_screen;   
};

class LeftDisplay : public BaseDisplay
{
public:
  LeftDisplay(Diablo_Serial_4DLib *Display, Direction *directionControl, Tpms *tpms);
  void update();
  void redrawLabels(); 
  virtual byte displayOrientation();
  
protected:  
  void updatePitch(byte x, byte y, byte interleave, int angle);
  void updateRoll(byte x, byte y, byte interleave, int angle);
  void updateCompass(word heading);
  void updateTPMSvalue(byte tireLocation);
  
private:
  Direction *m_DirectionControl;
  Tpms *m_Tpms;  
};

class CenterDisplay : public BaseDisplay
{
public:
  CenterDisplay(Diablo_Serial_4DLib *Display, OBD *obd);
  void update();
  void redrawLabels(); 
  virtual byte displayOrientation();
  virtual void init();

protected:
  void updateSpeed(word speed);
  void updateRpm(word rpm);
      
private:
  OBD *m_obd;
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

class RightDisplay : public BaseDisplay
{
public:
  RightDisplay(Diablo_Serial_4DLib *Display, CruiseCtrl *control);
  void update();
  void redrawLabels(); 
  virtual byte displayOrientation();
  
protected:  
  void updateCruiseControl();
  void updateOBD2Status();
  
private:
  CruiseCtrl *m_CruiseControl;
};

#endif
