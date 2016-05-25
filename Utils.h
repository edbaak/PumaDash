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

// Operational settings
#define LOGFILE_PREFIX 1605     // Prefix for SD card logging file names, i.e. 16050001.OBD
#define DISPLAY_SPEED 115200    // The baudrate at which we're running the 4D display
//#define RAW_LOGGING           // Saves RAW OBD data in a file on SD card
//#define OBD_LOGGING           // Saves Processed OBD data in a file on SD card

// Debugging modes
#define LOOPBACK_MODE           // CAN loopback mode. Messages transmitted are looped back to the CAN receiver, which helps with debugging.
//#define OBD_DEBUG             // Prints out raw OBD RX data to Serial
//#define CAN_DEBUG 1           // Low level CAN debugging
//#define PID_DISCOVERY_MODE    // To discover new unknown PIDS, enable RECORD_UNKNOWN_PIDS. NOTE: This will only work if RX Masking/Filtering is switched off
#define MAX_UNKNOWN_PIDS 50     // Max number of unhandled PID's that we keep track of

// Puma dashboard specific UI defines
#define PUMA_LABEL_SIZE 1             // Font size for labels and subLabels
#define PUMA_SENSOR_DATA_SIZE 2       // Font size for 'normal' sensor data (RPM, Speed, TPMS and a few more are shown bigger)
#define PUMA_LABEL_COLOR WHITE        // FG color for label text
#define PUMA_ALARM_COLOR RED          // FX color for data that is in the alarm operating zone. Usually this means a too high value, but for Fuel Tank it means 'too low'
#define PUMA_WARNING_COLOR YELLOW     // FG color for data that is in a warning operating zone
#define PUMA_NORMAL_COLOR LIGHTGREEN  // FG color for data that is in the normal/safe operating zone
#define RPM_RADIUS 110                // Size of the Rpm dial

// PUMA Dash specific PID's, i.e. not part of the OBD2 standard and unknown to the vehicle ECU.
// These PID's are 'extensions' to the OBD standard and are NOT transmitted on the CAN bus
#define PID_PUMA_PITCH            0xFF01
#define PID_PUMA_ROLL             0xFF02
#define PID_PUMA_HEADING          0xFF03
#define PID_PUMA_TPMS_FL_PRESS    0xFF04
#define PID_PUMA_TPMS_FL_TEMP     0xFF05
#define PID_PUMA_TPMS_FR_PRESS    0xFF06
#define PID_PUMA_TPMS_FR_TEMP     0xFF07
#define PID_PUMA_TPMS_RL_PRESS    0xFF08
#define PID_PUMA_TPMS_RL_TEMP     0xFF09
#define PID_PUMA_TPMS_RR_PRESS    0xFF0A
#define PID_PUMA_TPMS_RR_TEMP     0xFF0B
#define PID_PUMA_TPMS_TL_PRESS    0xFF0C
#define PID_PUMA_TPMS_TL_TEMP     0xFF0D
#define PID_PUMA_TPMS_TR_PRESS    0xFF0E
#define PID_PUMA_TPMS_TR_TEMP     0xFF0F
#define PID_PUMA_CC_SPEED         0xFF10
#define PID_PUMA_CC_MODE          0xFF11
#define PID_PUMA_CC_ACCELERATOR   0xFF12
#define PID_PUMA_DTC              0xFF13

// Mega board PIN definitions
#define PIN_MP2515_RX_INTERRUPT 2
#define PIN_DISPLAY_RESET 4
#define PIN_CAN_BOARD_LED1 7
#define PIN_CAN_BOARD_LED2 8
#define PIN_CAN_BOARD_SD_chipSelect 9 // SparkFun CAN-Bus Shield SPI chip select pin for SD card
#define PIN_LEGACY_SPI_CS 10
#define PIN_LEGACY_SPI_MOSI 11
#define PIN_LEGACY_SPI_MISO 12
#define PIN_LEGACY_SPI_SCK 13
#define DISPLAY_SERIAL1 Serial1  // We're running the 4D Display on USART 2, using pin 18 & 19
#define PIN_MEGA_SPI_MISO 50     // Connect Pin 50 to 12 to use with CAN BOARD
#define PIN_MEGA_SPI_MOSI 51     // Connect Pin 51 to 11 to use with CAN BOARD
#define PIN_MEGA_SPI_SCK 52      // Connect Pin 52 to 13 to use with CAN BOARD
#define PIN_MEGA_SPI_CS 53       // Connect pin 53 to 10 to use with CAN BOARD

// Helper functions that don't belong in a class and aren't that important.
String v2s(char* format, int value);
String v2s(char* format, byte value);
String v2s(char* format, word value);

void uniqueLogFileName();
void initLogging();
void logRawData(char *s);
void logObdData(String s);

#endif


