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

void uniqueLogFileName()
{
  word num = 0;
  word version = 1;
  // Try to read the last number used to create an incremental numbered filename "MYY_xxxx.DAT"
  File dataFile = SD.open("/OBD.CFG", FILE_READ);
  if (dataFile) {
    version = dataFile.read();
    version = version * 256 + dataFile.read();
    num = dataFile.read();
    num = num * 256 + dataFile.read();
    dataFile.close();
  }

  // Convert into a string
  char s[15];
  sprintf(s, "/%d_%04d.TXT", LOGFILE_PREFIX, num);

  // If the file already exists i.e. we have indeed done some logging, we want to increment
  // the number and save it for next round
  if (SD.exists(s)) {
    num++; // if reaching max it'll go down to zero again.
    sprintf(s, "/%d_%04d.TXT", LOGFILE_PREFIX, num);
    // If the next sequential file also exists, we may have looped through all 99999 files, and we start
    // again at 0. In which case we need to delete the old file first.
    if (SD.exists(s)) {
      SD.remove(s);
    }

    File dataFile = SD.open("/OBD.CFG", FILE_READ);
    if (dataFile) {
      dataFile.write(version);
      dataFile.write(num);
      dataFile.close();
    }
  }
  g_logFileName = String(s);
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
    Serial.println("SD Card failed, or not present: Logging disabled");
  } else {
    uniqueLogFileName();
    Serial.print("Logging to ");
    Serial.println(g_logFileName);
  }
}

// Simple helper function to write a string to a logging file
//void logData(String s)
void logData(char *s)
{
  if (g_logFileName == "")
    return;

  File dataFile = SD.open(g_logFileName, FILE_WRITE);
  if (dataFile) {
    dataFile.print(millis());
    dataFile.print(" ");
    dataFile.println(s);
    dataFile.close();   //close file
  } else {
    Serial.println("Error opening SD file for logging");
  }
}

// ************************************************************************

Table::Table(PumaDisplay *display, String title, word borderLines, byte columns, byte rows, word minX, word maxX, word minY, word maxY)
{
  m_display = display;
  m_title = title;
  m_border_lines = borderLines;
  if (columns < 1) columns = 1;
  m_columns = columns;
  m_min_x = minX;
  m_max_x = maxX;
  if (rows < 1) rows = 1;
  m_rows = rows;
  m_min_y = minY;
  m_max_y = maxY;
  m_title_height = 0;
  if (title.length() > 0) {
    display->txt_FGcolour(WHITE);
    display->txt_Width(1);
    display->txt_Height(1);
    word w = display->charwidth('A');
    word x = m_min_x + (m_max_x - m_min_x) / 2;
    x -= w * title.length() / 2;
    m_title_height = display->charheight('A') + 4;

    display->gfx_MoveTo(x, m_min_y + 2);
    display->print(title);
  }

  m_cell_width = (m_max_x - m_min_x) / m_columns;
  m_cell_height = ((m_max_y - m_min_y) - m_title_height) / m_rows;

  if ((m_border_lines & LEFT_BORDER) > 0) {
    display->gfx_Line(m_min_x, m_min_y, m_min_x, m_max_y, WHITE);
  }
  if ((m_border_lines & RIGHT_BORDER) > 0) {
    display->gfx_Line(m_max_x, m_min_y, m_max_x, m_max_y, WHITE);
  }
  if ((m_border_lines & TOP_BORDER) > 0) {
    display->gfx_Line(m_min_x, m_min_y, m_max_x, m_min_y, WHITE);
  }
  if ((m_border_lines & BOTTOM_BORDER) > 0) {
    display->gfx_Line(m_min_x, m_max_y, m_max_x, m_max_y, WHITE);
  }
  if ((m_border_lines & SHOW_GRID) > 0) {
    display->gfx_LinePattern(0x00aa);
    byte border = 30;
    if (m_columns > 1)
      for (byte column = 1; column <= m_columns; column++)
        display->gfx_Line(cellX(column), m_min_y + border, cellX(column), m_max_y - border, WHITE);
    if (m_rows > 1)
      for (byte row = 1; row <= m_rows; row++)
        display->gfx_Line(m_min_x + border, cellY(row), m_max_x - border, cellY(row), WHITE);
    display->gfx_LinePattern(0);
  }
}

word Table::cellX(byte column)
{
  return m_min_x + (column * m_cell_width);
}

word Table::cellY(byte row)
{
  return m_min_y + m_title_height + (row * m_cell_height);
}



