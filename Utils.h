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

#ifndef Utils_h
#define Utils_h
 
#if (ARDUINO >= 100)
	#include "Arduino.h" // for Arduino 1.0
#else
	#include "WProgram.h" // for Arduino 23
#endif

#define DISPLAY_SPEED 115200   // The baudrate at which we're running the 4D display
#define LOGFILE_PREFIX 165     // Prefix for SD card logging file names, i.e. 165_0001.txt
//#define LOOPBACK_MODE        // CAN loopback mode. Messages transmitted are looped back to the CAN receiver, which helps with debugging.
//#define VEHICLEDASH_DEBUG
//#define OBD_DEBUG

// Mega board PIN definitions
#define PIN_DISPLAY_RESET 4
#define PIN_CAN_BOARD_LED1 7
#define PIN_CAN_BOARD_LED2 8
#define PIN_CAN_BOARD_SD_chipSelect 9 // SparkFun CAN-Bus Shield SPI chip select pin for SD card
#define PIN_LEGACY_SPI_CS 10
#define PIN_LEGACY_SPI_MOSI 11
#define PIN_LEGACY_SPI_MISO 12
#define PIN_LEGACY_SPI_SCK 13
#define DISPLAY_SERIAL1 Serial1 // We're running the 4D Display on USART 2, using pin 18 & 19
#define PIN_MEGA_SPI_MISO 50 // Connect Pin 50 to 12 to use with CAN BOARD
#define PIN_MEGA_SPI_MOSI 51 // Connect Pin 51 to 11 to use with CAN BOARD
#define PIN_MEGA_SPI_SCK 52 // Connect Pin 52 to 13 to use with CAN BOARD
#define PIN_MEGA_SPI_CS 53 // Connect pin 53 to 10 to use with CAN BOARD

// Helper functions that don't belong in a class and aren't that important.
void uniqueLogFileName();
void initLogging();
void logData(char *s);

class PumaDisplay;

class Table
{
  public:
typedef enum BORDER_LINES {
    NONE = 0,
    LEFT_BORDER = 0x01,
    TOP_BORDER = 0x02,
    RIGHT_BORDER = 0x04,
    BOTTOM_BORDER = 0x08,
    ALL_BORDER = 0x0F,
    GRID = 0x10
} BORDER_LINES;

    Table(PumaDisplay *display, String title, word borderLines, byte columns, byte rows, word minX, word maxX, word minY, word maxY);
    word cellX(byte column);
    word cellY(byte row);
    
  private:
    PumaDisplay *m_display; 
    String m_title;
    word m_title_height;
    word m_border_lines;
    word m_columns;
    word m_min_x;
    word m_max_x;
    word m_rows;
    word m_min_y;
    word m_max_y;
    word m_cell_width;
    word m_cell_height;
};


#endif
