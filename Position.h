#ifndef Direction_h
#define Direction_h

#if (ARDUINO >= 100)
  #include "Arduino.h" // for Arduino 1.0
#else
  #include "WProgram.h" // for Arduino 23
#endif

class Direction
{
  public:
    Direction();
    void update();
    
    word compass();
    word pitch();
    word roll();
    
  private:
    word gps_Compass;
    int gps_Pitch;
    int gps_Roll;
};

#endif
