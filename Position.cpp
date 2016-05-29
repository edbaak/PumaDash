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
#include "Position.h"
#include "Display.h"

Position::Position()
{
  gps_Compass = new OBDData(PID_PUMA_HEADING, "Heading", "%03d", "", WORD_NO_CONVERSION, 0, 360);
  gps_Pitch = new OBDData(PID_PUMA_PITCH, "Pitch", "%2i", "", INT_NO_CONVERSION, -40, 40);
  gps_Roll = new OBDData(PID_PUMA_ROLL, "Roll", "%2i", "", INT_NO_CONVERSION, -40, 40);
}

void Position::update()
{
#ifdef LOOPBACK_MODE
  gps_Compass->simulateData(0);
  gps_Pitch->simulateData(0);
  gps_Roll->simulateData(0);
#else
// TODO: Get data from HW sensor
#endif // LOOPBACK_MODE
}



