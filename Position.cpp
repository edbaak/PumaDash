#include "Utils.h"
#include "Position.h"

Direction::Direction()
{
  gps_Compass = 0;
  gps_Pitch = 0;
  gps_Roll = 0;
}

void Direction::update()
{
#ifdef VEHICLEDASH_DEBUG
  static byte slowdown_counter = 0;
  slowdown_counter++;
  if (slowdown_counter == 10) {
    slowdown_counter = 0;
    gps_Compass++;
    if (gps_Compass > 360)
      gps_Compass = 0;
  }

  static int debug_pitch_incrementer = 1;
  gps_Pitch+= debug_pitch_incrementer;
  if (gps_Pitch > 44)
    debug_pitch_incrementer = -1;
  else if (gps_Pitch < -44)
    debug_pitch_incrementer = 1;

  static int debug_roll_incrementer = 1;
  gps_Roll+= debug_roll_incrementer;
  if (gps_Roll > 44)
    debug_roll_incrementer = -1;
  else if (gps_Roll < -44)
    debug_roll_incrementer = 1;
#endif // VEHICLEDASH_DEBUG
}

word Direction::compass()
{
  return gps_Compass;
}

word Direction::pitch()
{
  return gps_Pitch;
}

word Direction::roll()
{
  return gps_Roll;  
}

