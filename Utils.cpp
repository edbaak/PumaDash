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
bool g_SD_card_available = false;

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

//***************************************************************************
//                                StopWatch
//***************************************************************************

StopWatch::StopWatch()
{
  m_start = 0;
}

void StopWatch::start()
{
  m_start = millis();
}

unsigned long StopWatch::elapsed()
{
  unsigned long tmp = millis();
  return tmp - m_start;
}

bool StopWatch::notStarted()
{
  return m_start == 0;
}

//******************************************************************************************************
//                                      PumaFile
//******************************************************************************************************

/*
  word file_Mount();  // Initializes SD card
  void file_Unmount(); // Shuts down SD card
  word file_Error(); // Returns last error code.
  word file_FindFirstRet(char *  Filename, char *  StringIn);  // StringIn returns the name of file that matches search criteria. Retvalue is string length. Return string is NOT null terminated.
  word file_FindNextRet(char *  StringIn); // StringIn returns the next filename that matches the search criteria
*/

PumaFile::PumaFile()
{
  m_fileName = "";
  m_handle = 0;
  m_bytesAvailable = 0;
}

PumaFile::~PumaFile()
{
  close();
}

bool PumaFile::open(String fileName, PUMA_FILE_MODE mode, bool eraseIfExists)
{
  if (fileName != "") m_fileName = fileName;
  if (m_fileName == "") return false;

  char fname[50];
  strcpy(fname, fileName.c_str());

  m_handle = Display()->file_Open(fname, mode);
  if (m_handle == 0 && mode == WRITE && eraseIfExists && exists(fileName)) {
    erase(fileName);
    m_handle = Display()->file_Open(fname, mode);
  }

  if (m_handle && mode == READ)
    m_bytesAvailable = size();
  else
    m_bytesAvailable = 0;

  return m_handle != 0;
}

void PumaFile::close()
{
  if (m_handle == 0) return;
  Display()->file_Close(m_handle);
  m_handle = 0;
  m_bytesAvailable = 0;
}

unsigned long PumaFile::bytesAvailable()
{
  return m_bytesAvailable;
}

#define MAX_STRING_BUF 255
String PumaFile::readString()
{
  if (m_handle == 0) return "";
  char buf[MAX_STRING_BUF + 1];
  //  Display()->file_GetS(buf, MAX_STRING_BUF, m_handle);
  word i = 0;
  char c = 0;
  while (i < MAX_STRING_BUF && c != '\r' && m_bytesAvailable > 0) {
    c = Display()->file_GetC(m_handle);
    buf[i] = c;
    i++;
  }
  buf[i - 1] = 0;

  return String(buf);
}

bool PumaFile::print(String s)
{
  char buf[100];
  strcpy(buf, s.c_str()); // TODO: This is ugly
  Display()->file_PutS(buf, m_handle);
  return true; // TODO: error checking?
}

bool PumaFile::println(String s)
{
  return print(s + "\r");
}

byte PumaFile::readByte()
{
  if (m_handle == 0) return 0;
  m_bytesAvailable--;
  return (byte)Display()->file_GetC(m_handle);
}

bool PumaFile::writeByte(byte b)
{
  Display()->file_PutC(b, m_handle);
  return true; // TODO: error checking?
}

word PumaFile::readWord()
{
  if (m_handle == 0) return 0;
  m_bytesAvailable -= 2;
  return (word)Display()->file_GetW(m_handle);
}

bool PumaFile::writeWord(word w)
{
  Display()->file_PutW(w, m_handle);
  return true; // TODO: error checking?
}

unsigned long PumaFile::size()
{
  if (m_handle == 0) return 0;
  word lo, hi;
  unsigned long ret;
  if (Display()->file_Size(m_handle, &hi, &lo)) {
    ret = hi;
    ret = ret << 16;
    ret = ret + lo;
  }
  return ret;
}

unsigned long PumaFile::pointer()
{
  if (m_handle == 0)
    return 0;
  word lo, hi;
  unsigned long ret = 0;
  if (Display()->file_Tell(m_handle, &hi, &lo)) {
    ret = hi;
    ret = ret << 16;
    ret = ret + lo;
  }
  return ret;
}

#define MAX_FILE_NAME_LENGTH 100
bool PumaFile::erase(String fileName)
{
  char buf[MAX_FILE_NAME_LENGTH];
  strcpy(buf, fileName.c_str()); // TODO: can I get away without this uglyness?
  return Display()->file_Erase(buf);
}

bool PumaFile::exists(String fileName)
{
  char buf[MAX_FILE_NAME_LENGTH];
  strcpy(buf, fileName.c_str()); // TODO: can I get away without this uglyness?
  return Display()->file_Exists(buf);
}

//***************************************************************************
//                                Logging
//***************************************************************************

void initLogging()
{
  // Start with an empty string so we always start on a new line.
  Serial.println("");

  // Initialize SD card
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  //  pinMode(PIN_CAN_BOARD_SD_CS, OUTPUT);

  // see if the card is present and can be initialized:
  g_SD_card_available = false;
  if (!Display()->file_Mount()) {
    Serial.println("SD Card not found: Logging disabled");
    Serial.println(Display()->file_Error());
  } else {
    g_SD_card_available = true;
    uniqueLogFileName();
    Serial.println("Logging to " + g_logFileName);
  }
}

String uniqueLogFileName()
{
  if (!g_SD_card_available) return "";
  if (g_logFileName != "") return g_logFileName;

  word num = 0;
  word prefix = LOGFILE_PREFIX;
  byte version = 0;

  // Try to read the last number used to create an incremental numbered filename
  PumaFile cfgFile;
  if (cfgFile.open("/PUMADASH.PDC", PumaFile::READ)) {
    version = cfgFile.readByte();
    num = cfgFile.readWord();
    if (version >= 2)
      prefix = cfgFile.readWord();
    cfgFile.close();
  }
  g_logFileName = v2s("/%4d", LOGFILE_PREFIX) + v2s("%04d", num);

  // If the file already exists i.e. we have indeed done some logging, we want to increment
  // the number and save it for next round
  if (PumaFile::exists(g_logFileName + ".PDR") || PumaFile::exists(g_logFileName + ".PDO")) {
    if (num < 9999) num++; // if reaching max we'll keep pumping into that file. This should never happen.
    g_logFileName = v2s("/%4d", LOGFILE_PREFIX) + v2s("%04d", num);
  }

  if (cfgFile.open("/PUMADASH.PDC", PumaFile::WRITE)) {
    byte newVersion = 2; // bump the version when changing the file layout
    cfgFile.writeByte(newVersion);
    cfgFile.writeWord(num);
    cfgFile.writeWord(prefix);
    cfgFile.close();
  }
  return g_logFileName;
}

void logRawData(CAN_Frame *message)
{
  if (message == 0)
    return;

  char buf1[150];
  sprintf(buf1, "%08lu, %04X, ",
          message->m_timeStamp,
          message->m_id);

  char buf2[150];
  sprintf(buf2, "%u, %02X%02X%02X%02X%02X%02X%02X%02X",
          message->m_length,
          message->m_data[0],
          message->m_data[1],
          message->m_data[2],
          message->m_data[3],
          message->m_data[4],
          message->m_data[5],
          message->m_data[6],
          message->m_data[7]);

#ifdef OBD_VERBOSE_DEBUG
  Serial.print(buf1);
  Serial.println(buf2);
#endif

  if (g_logFileName == "")
    return;

  PumaFile log;
  if (log.open(g_logFileName + ".PDR", PumaFile::APPEND)) {
    log.print(buf1);
    log.println(buf2);
    log.close();
  } else {
    Serial.println("Error opening SD file for RAW logging");
  }
}

byte hexToByte(char c)
{
  switch (c) {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a': return 10;
    case 'b': return 11;
    case 'c': return 12;
    case 'd': return 13;
    case 'e': return 14;
    case 'f': return 15;
    case 'A': return 10;
    case 'B': return 11;
    case 'C': return 12;
    case 'D': return 13;
    case 'E': return 14;
    case 'F': return 15;
  }
  return 0;
}

unsigned long strToUlong(String s)
{
  unsigned long tmp = 0;
  int i = 0;
  while (i < s.length() && isDigit(s[i])) {
    tmp *= 10;
    tmp += hexToByte(s[i]);
    i++;
  }
  return tmp;
}

void readRawData(String fileName)
{
  PumaFile logFile;
  if (logFile.open(fileName, PumaFile::READ)) {
    if (logFile.bytesAvailable()) {
      //      String tmp = logFile.readString();
      String tmp = "00030135, 07E8, 8, 03410D05000000AB";
      String timeStamp = "";
      CAN_Frame message;
      if (tmp.length() == 35 && tmp[8] == ',' && tmp[14] == ',' && tmp[17] == ',') {
        for (int i = 0; i < 8; i++) timeStamp += tmp[i];
        message.m_timeStamp = strToUlong(timeStamp);

        word id = 0;
        for (int i = 10; i < 14; i++) {
          id = id << 4;
          id += hexToByte(tmp[i]);
        }
        message.m_id = id;
        message.m_length = hexToByte(tmp[16]);

        int i = 19;
        for (int j = 0; j < 8; j++) {
          message.m_data[j] = hexToByte(tmp[i]);
          message.m_data[j] = (message.m_data[j] << 4) + hexToByte(tmp[i + 1]);
          i += 2;
        }
      }
      Serial.println("--------------------");
      Serial.println(message.m_timeStamp);
      Serial.println(message.m_id, HEX);
      Serial.println(message.m_length);
      Serial.println(message.m_data[0], HEX);
      Serial.println(message.m_data[1], HEX);
      Serial.println(message.m_data[2], HEX);
      Serial.println(message.m_data[3], HEX);
      Serial.println(message.m_data[4], HEX);
      Serial.println(message.m_data[5], HEX);
      Serial.println(message.m_data[6], HEX);
      Serial.println(message.m_data[7], HEX);
      Serial.println("Done");
    }
  }
}

void logObdData(String s)
{
#ifdef OBD_VERBOSE_DEBUG
  Serial.println(s);
#endif
  if (g_logFileName == "")
    return;

  PumaFile log;
  if (log.open(g_logFileName + ".PDO", PumaFile::APPEND)) {
    log.println(s);
    log.close();
  } else {
    Serial.println("Error opening SD file for OBD logging");
  }
}

// *************************************************************************************************
//                                  SELF TEST
// *************************************************************************************************

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

void sdCardTest()
{
  Serial.print("SD card test   : ");
  FAIL_IF_FALSE(g_SD_card_available, "No SD card found");

  // This should not be needed, but in case a previous test round failed we may have to clean up
  // some test artifacts.
  PumaFile::erase("/SELFTEST.XYZ");

  PumaFile test;
  FAIL_IF_FALSE(test.open("/SELFTEST.XYZ", PumaFile::WRITE), "SD open failed");

  test.print("line ");
  test.println("1");
  test.println("line 2");
  test.println("The Quick Brown Fox Jumps Over");
  test.writeByte(int(-1));
  test.writeByte(255);
  test.writeWord(0xFFFF);
  test.close();

#define FILE_SIZE 49
  byte ref_file[FILE_SIZE] =
  { 0x6C, 0x69, 0x6E, 0x65, 0x20, 0x31, 0x0D, 0x6C, 0x69, 0x6E,
    0x65, 0x20, 0x32, 0x0D, 0x54, 0x68, 0x65, 0x20, 0x51, 0x75,
    0x69, 0x63, 0x6B, 0x20, 0x42, 0x72, 0x6F, 0x77, 0x6E, 0x20,
    0x46, 0x6F, 0x78, 0x20, 0x4A, 0x75, 0x6D, 0x70, 0x73, 0x20,
    0x4F, 0x76, 0x65, 0x72, 0x0D, 0xFF, 0xFF, 0xFF, 0xFF
  };

  if (test.open("/SELFTEST.XYZ", PumaFile::READ)) {
    FAIL_IF_TRUE(test.size() != FILE_SIZE, "File::size() failed");

    int i = 0;
    while (test.bytesAvailable()) {
      FAIL_IF_TRUE(i >= FILE_SIZE, "Reading past file size");
      FAIL_IF_TRUE(ref_file[i++] != test.readByte(), "File::read() failed");
    }
  }
  FAIL_IF_FALSE(PumaFile::erase("/SELFTEST.XYZ"), "SD remove failed");
  Serial.println("PASS");
}

void OBDDataTest()
{
  Serial.print("OBD data test  : ");
  uint8_t t_pid = 0x0C;
  String t_label = "ObdLabel";
  String t_subLabel = "ObdSubLabel";
  OBD_DATA_CONVERSION t_conversion = BYTE_NO_CONVERSION;
  long t_min = 0;
  long t_max = 100;
  OBDData tester(t_pid, t_label, t_subLabel, t_conversion, 3, OBD_D, t_min, t_max, 0);

  FAIL_IF_FALSE(tester.pid() == t_pid, "1.1");
  FAIL_IF_FALSE(tester.label() == t_label, "1.2");
  FAIL_IF_FALSE(tester.subLabel() == t_subLabel, "1.3");
  FAIL_IF_FALSE(tester.dataConversion() == BYTE_NO_CONVERSION, "1.4");

  // TODO: need to test color()

  // Format & stringlen tests
  tester.setFormat(4, OBD_D);
  FAIL_IF_FALSE(tester.stringLength() == 4, "2.1");
  tester.setFormat(8, OBD_D);
  FAIL_IF_FALSE(tester.stringLength() == 8, "2.2");
  tester.setFormat(3, OBD_D);
  FAIL_IF_FALSE(tester.stringLength() == 3, "2.3");
  tester.setFormat(5, OBD_F1);
  FAIL_IF_FALSE(tester.stringLength() == 7, "2.4");
  tester.setFormat(8, OBD_F2);
  FAIL_IF_FALSE(tester.stringLength() == 11, "2.5");
  tester.setFormat(4, OBD_F1);
  FAIL_IF_FALSE(tester.stringLength() == 6, "2.6");
  tester.setFormat(3, OBD_F1);
  FAIL_IF_FALSE(tester.stringLength() == 5, "2.7");

  // Test data conversions. I'm splitting that up into two steps first. 1) from a long value to int, byte, word and string and then 2) from CAN_Frame to long.
  tester.setValue(0);
  FAIL_IF_FALSE(tester.toInt() == 0, "3.1");
  FAIL_IF_FALSE(tester.toWord() == 0, "3.2");
  FAIL_IF_FALSE(tester.toLong() == 0, "3.3");
  tester.setFormat(1, OBD_D);
  FAIL_IF_FALSE(tester.toString() == "0", "3.4");
  tester.setFormat(2, OBD_D);
  FAIL_IF_FALSE(tester.toString() == " 0", "3.5");
  tester.setFormat(3, OBD_D);
  FAIL_IF_FALSE(tester.toString() == "  0", "3.6");

  tester.setValue(1);
  FAIL_IF_FALSE(tester.toInt() == 1, "4.1");
  FAIL_IF_FALSE(tester.toWord() == 1, "4.2");
  FAIL_IF_FALSE(tester.toLong() == 1, "4.3");
  tester.setFormat(3, OBD_D);
  FAIL_IF_FALSE(tester.toString() == "  1", "4.5");

  tester.setValue(-1);
  FAIL_IF_FALSE(tester.toInt() == -1, "5.1");
  FAIL_IF_FALSE(tester.toWord() == 65535, "5.2");
  FAIL_IF_FALSE(tester.toLong() == -1, "5.3");
  tester.setFormat(3, OBD_D);
  FAIL_IF_FALSE(tester.toString() == " -1", "5.5");

  tester.setValue(255);
  FAIL_IF_FALSE(tester.toInt() == 255, "6.1");
  FAIL_IF_FALSE(tester.toWord() == 255, "6.2");
  FAIL_IF_FALSE(tester.toLong() == 255, "6.3");
  tester.setFormat(3, OBD_D);
  FAIL_IF_FALSE(tester.toString() == "255", "6.5");

  tester.setValue(256);
  FAIL_IF_FALSE(tester.toInt() == 256, "7.1");
  FAIL_IF_FALSE(tester.toWord() == 256, "7.2");
  FAIL_IF_FALSE(tester.toLong() == 256, "7.3");
  tester.setFormat(3, OBD_D);
  FAIL_IF_FALSE(tester.toString() == "256", "7.5");

  tester.setValue(65535);
  FAIL_IF_FALSE(tester.toInt() == -1, "8.1");
  FAIL_IF_FALSE(tester.toWord() == 65535, "8.2");
  FAIL_IF_FALSE(tester.toLong() == 65535, "8.3");
  tester.setFormat(3, OBD_D);
  FAIL_IF_FALSE(tester.toString() == "65535", "8.5");

  tester.setValue(0XFFFF);
  FAIL_IF_FALSE(tester.toWord() == 65535, "8.6");
  FAIL_IF_FALSE(tester.toWord() == 0XFFFF, "8.7");

  tester.setValue(-65535);
  FAIL_IF_FALSE(tester.toInt() == 1, "8.8");
  FAIL_IF_FALSE(tester.toWord() == 1, "8.9");
  FAIL_IF_FALSE(tester.toLong() == -65535, "8.10");
  tester.setFormat(6, OBD_D);
  FAIL_IF_FALSE(tester.toString() == "-65535", "8.12");

  tester.setValue(0XFFFFFFFF);
  FAIL_IF_FALSE(tester.toInt() == -1, "9.1");
  FAIL_IF_FALSE(tester.toWord() == 0XFFFF, "9.2");
  FAIL_IF_FALSE(tester.toLong() == long(0XFFFFFFFF), "9.3");
  tester.setFormat(3, OBD_D);
  FAIL_IF_FALSE(tester.toString() == " -1", "9.5");
  tester.setFormat(3, OBD_H);
  FAIL_IF_FALSE(tester.toString() == "0XFFFFFFFF", "9.6");

  tester.setValue(0XFFFFFFFE);
  tester.setFormat(3, OBD_D);
  FAIL_IF_FALSE(tester.toString() == " -2", "9.7");

  tester.setValue(200);
  tester.setFormat(1, OBD_F1);
  tester.setDataConversion(WORD_DIV100);
  FAIL_IF_FALSE(tester.toString() == "2.0", "10.1");

  tester.setFormat(2, OBD_F1);
  tester.setDataConversion(WORD_DIV1000);
  FAIL_IF_FALSE(tester.toString() == " 0.2", "10.2");

  unsigned long l = 5000000;
  FAIL_IF_FALSE(String(l) == "5000000", "10.3");
  l = 50000000;
  FAIL_IF_FALSE(String(l) == "50000000", "10.4");

  Serial.println("PASS");
}

void selfTest()
{

  Serial.println("**************************************");
  Serial.println("Start Self Test");
  Serial.flush();
  stringListTest();
  sdCardTest();
  OBDDataTest();
  Serial.println("Self Test Done ");
  Serial.println("**************************************");

//  readRawData("/16060185.PDR");
}

#endif // SELF_TEST

