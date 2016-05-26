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
#include <SD.h>
#include "Display.h"
#include "CAN.h"

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

String v2s(char* format, unsigned long value)
{
  // TODO: should use format :-)
  String s(value);
  return s;
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

  g_logFileName = v2s("/%4d", LOGFILE_PREFIX) + v2s("%04d", num);

  // If the file already exists i.e. we have indeed done some logging, we want to increment
  // the number and save it for next round
  if (SD.exists(g_logFileName + ".RAW") || SD.exists(g_logFileName + ".OBD")) {
    if (num < 9999) num++; // if reaching max we'll keep pumping into that file. This should never happen.
    g_logFileName = v2s("/%4d", LOGFILE_PREFIX) + v2s("%04d", num);
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
  // Start with an empty string so we always start on a new line.
  Serial.println("");

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

void logRawData(CAN_Frame *message)
{
  if (message == 0)
    return;

  char buf[150];
  sprintf(buf, "%06d, %04x, %d, %d, %d, %02X, %02X, %02X, %02X, %02X, %02X, %02X, %02X",
          message->m_timeStamp,
          message->m_id,
          message->m_rtr,
          message->m_extended,
          message->m_length,
          message->m_data[0],
          message->m_data[1],
          message->m_data[2],
          message->m_data[3],
          message->m_data[4],
          message->m_data[5],
          message->m_data[6],
          message->m_data[7]);
  logRawData(buf);
}

// Simple helper function to write a string to a logging file
void logRawData(char *s)
{
  if (g_logFileName == "")
    return;

  File dataFile = SD.open(g_logFileName + ".RAW", FILE_WRITE);
  if (dataFile) {
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


// *************************************************************************************************
//                                  SELF TEST

void selfTest()
{
  Serial.println("**************************************");
  Serial.println("Start Self Test");
  // Do actual tests here...
  Serial.println("Self Test Done ");
  Serial.println("**************************************");
}



