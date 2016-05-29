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
#include "Widget.h"

String g_logFileName = "";

String v2s(String format, int value)
{
  char s[10];
  sprintf(s, format.c_str(), value);
  return String (s);
}

String v2s(String format, byte value)
{
  char s[10];
  sprintf(s, format.c_str(), value);
  return String(s);
}

String v2s(String format, word value)
{
  char s[10];
  sprintf(s, format.c_str(), value);
  return String(s);
}

String v2s(String format, unsigned long value)
{
  // TODO: should use format :-)
  return String(value);
}

void uniqueLogFileName()
{
  g_logFileName = "16050002";
  return;
  
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

#ifdef SELF_TEST

#define FAIL_IF_FALSE(x, fail_string) if (x) {} else {Serial.println(String("FAIL: ") + String(fail_string)); return;}
#define FAIL_IF_TRUE(x, fail_string) if (x) {Serial.println(String("FAIL: ") + String(fail_string)); return;}

void stringListTest()
{
  Serial.print("StringList test: ");
  StringList test;
  FAIL_IF_FALSE(test.count() == 0, "1");
  test.addString("one");
  FAIL_IF_FALSE(test.count() == 1, "2");
  FAIL_IF_FALSE(test.stringAt(0) == "one", "3");
  test.addString("two");
  test.addString("three");
  test.addString("four");
  FAIL_IF_FALSE(test.count() == 4, "4");
  FAIL_IF_FALSE(test.stringAt(2) == "three", "5");
  test.deleteAt(2);
  FAIL_IF_FALSE(test.count() == 3, "6");
  FAIL_IF_FALSE(test.stringAt(0) == "one", "7");
  FAIL_IF_FALSE(test.stringAt(1) == "two", "8");
  FAIL_IF_FALSE(test.stringAt(2) == "four", "9");
  test.deleteAll();
  FAIL_IF_FALSE(test.count() == 0, "10");
  Serial.println("PASS");
}

/*
    Not tested at the moment (because I am lazy and don't need these):
    void dataFile.flush()
    byte dataFile.peek()
    unsigned long dataFile.position()
    bool dataFile.seek(position)
    File dataFile.rewindDirectory()
*/
void sdCardTest()
{
  Serial.print("SD card test   : ");
//  FAIL_IF_FALSE(SD.begin(PIN_CAN_BOARD_SD_chipSelect), "No SD card found");

  // This should not be needed, but in case a previous test round failed we may have to clean up
  // some test artifacts.
  SD.remove("/SELF_TST.XYZ/SelfTest.log");
  SD.rmdir("/SELF_TST.XYZ");

  FAIL_IF_TRUE(SD.exists("/SELF_TST.XYZ"), "Test dir already exists");

  // Use various SD and File functions to check that we can create a file, write date, read data,
  // and delete everything again
  FAIL_IF_FALSE(SD.mkdir("/SELF_TST.XYZ"), "SD.mkdir() failed");

  File dataFile = SD.open("/SELF_TST.XYZ/SelfTest.log", FILE_WRITE);
  FAIL_IF_FALSE(dataFile, "SD open failed");

  dataFile.print("line ");
  dataFile.println("1");
  dataFile.println("line 2");
  dataFile.write("The Quick Brown Fox Jumps Over\r\n");
  dataFile.write(int(-1));
  dataFile.write(byte(255));
  dataFile.close();

  // Remember: All file names are converted to upper case
  File dir = SD.open("/SELF_TST.XYZ");
  FAIL_IF_FALSE(dir, "SD.open() failed");

  File tst_file = dir.openNextFile();
  FAIL_IF_FALSE(tst_file, "openNextFile failed");

  FAIL_IF_FALSE(strcmp(tst_file.name(), "SELFTEST.LOG") == 0, "file.name() failed");

  int len = tst_file.size();
  FAIL_IF_TRUE(len != 50, "File::size() failed");

  byte buf[300];
  int count = 0;
  while (tst_file.available() > 0) {
    buf[count++] = tst_file.read();
    FAIL_IF_TRUE(count > len, "Too much data read.");
  }
  FAIL_IF_TRUE(count != 50, "Not all data read.");

  byte ref_file[50] =
  { 0x6C, 0x69, 0x6E, 0x65, 0x20, 0x31, 0xD, 0xA, 0x6C, 0x69,
    0x6E, 0x65, 0x20, 0x32, 0xD, 0xA, 0x54, 0x68, 0x65, 0x20,
    0x51, 0x75, 0x69, 0x63, 0x6B, 0x20, 0x42, 0x72, 0x6F, 0x77,
    0x6E, 0x20, 0x46, 0x6F, 0x78, 0x20, 0x4A, 0x75, 0x6D, 0x70,
    0x73, 0x20, 0x4F, 0x76, 0x65, 0x72, 0xD, 0xA, 0xFF, 0xFF
  };

  for (int i = 0; i < len; i++) {
    FAIL_IF_TRUE(buf[i] != ref_file[i], "File::read() failed");
  }

  FAIL_IF_TRUE(!dir.isDirectory(), "isDirectory() failed");

  if (!SD.rmdir("/SELF_TST.XYZ")) {
    FAIL_IF_FALSE(SD.remove("/SELF_TST.XYZ/SelfTest.log"), "SD remove failed");
    FAIL_IF_TRUE(!SD.rmdir("/SELF_TST.XYZ"), "rmdir() failed");
  }
  Serial.println("PASS");
}

void OBDDataTest()
{
  Serial.print("OBD data test  : ");
  uint8_t t_pid = 0x0C;
  String t_label = "ObdLabel";
  char *t_format = "%d";
  String t_subLabel = "ObdSubLabel";
  uint16_t t_interval = 0;
  OBD_DATA_CONVERSION t_conversion = BYTE_NO_CONVERSION;
  long t_min = 0;
  long t_max = 100;
  long t_step = 5;
  OBDData tester(t_pid, t_label, t_format, t_subLabel, t_interval, t_conversion, t_min, t_max, t_step);

  FAIL_IF_FALSE(tester.pid() == t_pid, "1.1");
  FAIL_IF_FALSE(tester.label() == t_label, "1.2");
  FAIL_IF_FALSE(tester.subLabel() == t_subLabel, "1.3");
  FAIL_IF_FALSE(tester.dataConversion() == BYTE_NO_CONVERSION, "1.4");

  // TODO: need to test color()

  // Format & stringlen tests
  tester.setFormat("%4d");
  FAIL_IF_FALSE(tester.stringLength() == 4, "2.1");
  tester.setFormat("%8d");
  FAIL_IF_FALSE(tester.stringLength() == 8, "2.2");
  tester.setFormat("%03d");
  FAIL_IF_FALSE(tester.stringLength() == 3, "2.3");
  tester.setFormat("%05.1d");
  FAIL_IF_FALSE(tester.stringLength() == 7, "2.4");
  tester.setFormat("%08.2x");
  FAIL_IF_FALSE(tester.stringLength() == 11, "2.5");
  tester.setFormat("%4.1x");
  FAIL_IF_FALSE(tester.stringLength() == 6, "2.6");

  // Test data conversions. I'm splitting that up into two steps first. 1) from a long value to int, byte, word and string and then 2) from CAN_Frame to long.
//  tester.setFormat("%d");
  tester.setFormat("%li");
  tester.setValue(0);
  FAIL_IF_FALSE(tester.toInt() == 0, "3.1");
  FAIL_IF_FALSE(tester.toWord() == 0, "3.2");
  FAIL_IF_FALSE(tester.toLong() == 0, "3.3");
//  FAIL_IF_FALSE(tester.toFloat() == 0.0, "3.4");
  FAIL_IF_FALSE(tester.toString() == "0", "3.5");

  tester.setValue(1);
  FAIL_IF_FALSE(tester.toInt() == 1, "4.1");
  FAIL_IF_FALSE(tester.toWord() == 1, "4.2");
  FAIL_IF_FALSE(tester.toLong() == 1, "4.3");
//  FAIL_IF_FALSE(tester.toFloat() == 1.0, "4.4");
  FAIL_IF_FALSE(tester.toString() == "1", "4.5");

  tester.setValue(-1);
  FAIL_IF_FALSE(tester.toInt() == -1, "5.1");
  FAIL_IF_FALSE(tester.toWord() == 65535, "5.2");
  FAIL_IF_FALSE(tester.toLong() == -1, "5.3");
//  FAIL_IF_FALSE(tester.toFloat() == -1.0, "5.4");
  FAIL_IF_FALSE(tester.toString() == "-1", "5.5");

  tester.setValue(255);
  FAIL_IF_FALSE(tester.toInt() == 255, "6.1");
  FAIL_IF_FALSE(tester.toWord() == 255, "6.2");
  FAIL_IF_FALSE(tester.toLong() == 255, "6.3");
//  FAIL_IF_FALSE(tester.toFloat() == 255.0, "6.4");
  FAIL_IF_FALSE(tester.toString() == "255", "6.5");

  tester.setValue(256);
  FAIL_IF_FALSE(tester.toInt() == 256, "7.1");
  FAIL_IF_FALSE(tester.toWord() == 256, "7.2");
  FAIL_IF_FALSE(tester.toLong() == 256, "7.3");
//  FAIL_IF_FALSE(tester.toFloat() == 256.0, "7.4");
  FAIL_IF_FALSE(tester.toString() == "256", "7.5");

//  tester.setFormat("%u");
  tester.setValue(65535);
  FAIL_IF_FALSE(tester.toInt() == -1, "8.1");
  FAIL_IF_FALSE(tester.toWord() == 65535, "8.2");
  FAIL_IF_FALSE(tester.toLong() == 65535, "8.3");
//  FAIL_IF_FALSE(tester.toFloat() == 65535.0, "8.4");
  FAIL_IF_FALSE(tester.toString() == "65535", "8.5");

  tester.setValue(0XFFFF);
  FAIL_IF_FALSE(tester.toWord() == 65535, "8.6");
  FAIL_IF_FALSE(tester.toWord() == 0XFFFF, "8.7");

  tester.setValue(-65535);
  FAIL_IF_FALSE(tester.toInt() == 1, "8.8");
  FAIL_IF_FALSE(tester.toWord() == 1, "8.9");
  FAIL_IF_FALSE(tester.toLong() == -65535, "8.10");
//  FAIL_IF_FALSE(tester.toFloat() == -65535.0, "8.11");
//  tester.setFormat("%li");
  FAIL_IF_FALSE(tester.toString() == "-65535", "8.12");

  tester.setValue(0XFFFFFFFF);
  FAIL_IF_FALSE(tester.toInt() == -1, "9.1");
  FAIL_IF_FALSE(tester.toWord() == 0XFFFF, "9.2");
  FAIL_IF_FALSE(tester.toLong() == long(0XFFFFFFFF), "9.3");
//  FAIL_IF_FALSE(tester.toFloat() == 65535.0, "9.4");
  tester.setFormat("%lu");
  FAIL_IF_FALSE(tester.toString() == "4294967295", "9.5");
  tester.setFormat("0X%lX");
  FAIL_IF_FALSE(tester.toString() == "0XFFFFFFFF", "9.6");

  Serial.println("PASS");
}

void selfTest()
{
  Serial.println("**************************************");
  Serial.println("Start Self Test");
  stringListTest();
  //sdCardTest();
  OBDDataTest();
  Serial.println("Self Test Done ");
  Serial.println("**************************************");
}

#endif // SELF_TEST

