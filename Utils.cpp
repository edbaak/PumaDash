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
#include <SD.h>
#include "Display.h"

String g_logFileName = "";

String v2s(char* format, int value)
{
  char s[10];
  sprintf(s, format, value);
  String ret(s);
  return ret;   
}

String v2s(char* format, byte value)
{
  char s[10];
  sprintf(s, format, value);
  String ret(s);
  return ret;   
}

String v2s(char* format, word value)
{
  char s[10];
  sprintf(s, format, value);
  String ret(s);
  return ret;   
}

void uniqueLogFileName()
{
  word num = 0;
  byte version = 1;
  // Try to read the last number used to create an incremental numbered filename "MMYYxxxx.DAT"
  File dataFile = SD.open("/OBD.CFG", FILE_READ);
  if (dataFile) {
    version = dataFile.read();
    num = dataFile.read();
    num = (num * 256) + dataFile.read();
    dataFile.close();
  }

  // Convert into a string
  char s[10];
//  sprintf(s, "/%d_%04d", LOGFILE_PREFIX, num);
  g_logFileName = v2s("/%4d", LOGFILE_PREFIX) + v2s("%04d", num);

  // If the file already exists i.e. we have indeed done some logging, we want to increment
  // the number and save it for next round
  if (SD.exists(g_logFileName + ".TXT")) {
    if (num < 9999) num++; // if reaching max we'll keep pumping into that file. This should never happen.
    
    g_logFileName = v2s("/%4d", LOGFILE_PREFIX) + v2s("%04d", num);
//    sprintf(s, "/%d_%04d", LOGFILE_PREFIX, num);
    // If the next sequential file also exists, we may have looped through all 9999 files, and we start
    // again at 0. In which case we need to delete the old file first.
    if (SD.exists(g_logFileName + ".TXT")) {
      SD.remove(g_logFileName + ".TXT");
    }
  } else {
    Serial.println("File doesn't exist yet");
  }
  
  dataFile = SD.open("/OBD.CFG", FILE_WRITE);
  if (dataFile) {
    dataFile.write(version);
    dataFile.write(num);
    dataFile.close();
  }
}

void initLogging()
{
  // Init pins for the two leds on the CAN board
  pinMode(PIN_CAN_BOARD_LED1, OUTPUT);
  pinMode(PIN_CAN_BOARD_LED2, OUTPUT);

  // Initialize SD card
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(PIN_CAN_BOARD_SD_chipSelect, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(PIN_CAN_BOARD_SD_chipSelect)) {
    Serial.println("SD Card not found: Logging disabled");
  } else {
    Serial.print("SD Card found: ");
    uniqueLogFileName();
    Serial.println("Logging to " + g_logFileName);
  }
}

// Simple helper function to write a string to a logging file
void logRawData(char *s)
{
  if (g_logFileName == "")
    return;

  File dataFile = SD.open(g_logFileName + ".RAW", FILE_WRITE);
  if (dataFile) {
    dataFile.print(millis());
    dataFile.print(" ");
    dataFile.println(s);
    dataFile.close();   //close file
  } else {
    Serial.println("Error opening SD file for RAW logging");
  }
}

void logObdData(String s)
{
  if (g_logFileName == "")
    return;

  File dataFile = SD.open(g_logFileName + ".OBD", FILE_WRITE);
  if (dataFile) {
    dataFile.println(s);
    dataFile.close();   //close file
  } else {
    Serial.println("Error opening SD file for OBD logging");
  }
}





