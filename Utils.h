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

#ifndef Utils_h
#define Utils_h

#if (ARDUINO >= 100)
#include "Arduino.h" // for Arduino 1.0
#else
#include "WProgram.h" // for Arduino 23
#endif

class CAN_Frame;

// Operational settings
#define LOGFILE_PREFIX 1606         // Prefix for SD card logging file names, i.e. 16050001.OBD
#define DISPLAY_SPEED 115200        // The baudrate at which we're running the 4D display
#define DISPLAY_RESET_MS 5000       // Wait time after a display reset
#define MAX_RX_FIFO 12              // FIFO buffer that stores received CAN messages, so that the MCP2515 can be emptied and available for the next message
#define PUMA_DEFAULT_SCREEN 1       // Define the default screen. We can change this by tapping the touchscreen
#define SELF_TEST                   // Runs a self-test at start-up

// Debugging modes
//#define LOOPBACK_MODE               // CAN loopback mode. Messages transmitted are looped back to the CAN receiver, which helps with debugging.
//#define OBD_VERBOSE_DEBUG           // Shows processed OBD data on the Serial Monitor
//#define OBD_DEBUG                   // Creates debugging stacktrace for OBD functions
//#define DISPLAY_DEBUG               // Creates debugging stacktrace for important functions in Display class
//#define TOUCH_DEBUG                 // Creates debugging stacktrace for touch functions
#define SPEED_DEBUG                   // Debugging stacktrace for Speed functions

//#define PID_DISCOVERY_MODE          // To discover new unknown PIDS, enable RECORD_UNKNOWN_PIDS.
#ifdef PID_DISCOVERY_MODE
#define MAX_UNKNOWN_PIDS 25       // Max number of unhandled PID's that we keep track of
#endif

// Puma dashboard specific UI defines
#define PUMA_LABEL_SIZE 1             // Font size for labels and subLabels
#define PUMA_HEADING_FONT_SIZE 4      // Font size for heading
#define PUMA_SPEED_FONT_SIZE 5
#define PUMA_RPM_FONT_SIZE 3
#define PUMA_SENSOR_DATA_FONT_SIZE 2  // Font size for 'normal' sensor data (RPM, Speed, TPMS and a few more are shown bigger)
#define PUMA_LABEL_COLOR WHITE        // FG color for label text
#define PUMA_ALARM_COLOR RED          // FX color for data that is in the alarm operating zone. Usually this means a too high value, but for Fuel Tank it means 'too low'
#define PUMA_WARNING_COLOR YELLOW     // FG color for data that is in a warning operating zone
#define PUMA_NORMAL_COLOR LIGHTGREEN  // FG color for data that is in the normal/safe operating zone
#define RPM_RADIUS 100                // Size of the Rpm dial
#define PUMA_DEBOUNCE_INTERVAL 1      // Delay (in ms) used to debounce buttons

// Puma Dashboard touch event processing parameters
#define TOUCH_DEBOUNCE_TIME 100       // Number of ms delay before a consider a 'pressed' a new touch instead of a bouncing finger
#define MINIMUM_TOUCH_DURATION 40     // Minimum time in ms before a touch press is considered a 'tap'.
#define SWIPE_DIVIDER 5               // Scales back a swipe
#define SWIPE_THRESHOLD 5             // Minimum of displacement in x or y direction before a touch is considered a swipe


// PUMA Dash specific PID's, i.e. not part of the OBD2 standard and unknown to the vehicle ECU.
// These PID's are 'extensions' to the OBD standard and are NOT transmitted on the CAN bus
#define PID_PUMA_PITCH            0xFF
#define PID_PUMA_ROLL             0xFE
#define PID_PUMA_HEADING          0xFD
#define PID_PUMA_TPMS_FL_PRESS    0xFC
#define PID_PUMA_TPMS_FL_TEMP     0xFB
#define PID_PUMA_TPMS_FR_PRESS    0xFA
#define PID_PUMA_TPMS_FR_TEMP     0xF9
#define PID_PUMA_TPMS_RL_PRESS    0xF8
#define PID_PUMA_TPMS_RL_TEMP     0xF7
#define PID_PUMA_TPMS_RR_PRESS    0xF6
#define PID_PUMA_TPMS_RR_TEMP     0xF5
#define PID_PUMA_TPMS_TL_PRESS    0xF4
#define PID_PUMA_TPMS_TL_TEMP     0xF3
#define PID_PUMA_TPMS_TR_PRESS    0xF2
#define PID_PUMA_TPMS_TR_TEMP     0xF1
#define PID_PUMA_CC_SPEED         0xF0
#define PID_PUMA_CC_MODE          0xEF
#define PID_PUMA_CC_ACCELERATOR   0xEE
#define PID_PUMA_DTC              0xED
#define PID_PUMA_ODO              0xEC
#define PID_PUMA_TRIP             0xEB
#define PID_PUMA_LAST_SERVICE     0xEA

// Mega board PIN definitions
#define PIN_MP2515_RX_INTERRUPT 2
#define PIN_DISPLAY_RESET 4
#define PIN_CAN_BOARD_LED1 7
#define PIN_CAN_BOARD_LED2 8
//  #define PIN_CAN_BOARD_SD_CS 9 // SparkFun CAN-Bus Shield SPI chip select pin for SD card
#define PIN_LEGACY_SPI_CS 10
#define PIN_LEGACY_SPI_MOSI 11
#define PIN_LEGACY_SPI_MISO 12
#define PIN_LEGACY_SPI_SCK 13    // Also the on-board LED
#define DISPLAY_SERIAL1 Serial1  // We're running the Left 4D Display on USART 2, using pin 18 & 19
#define DISPLAY_SERIAL2 Serial2  // We're running the Center 4D Display on USART 3, using pin ?? & ??
#define DISPLAY_SERIAL3 Serial3  // We're running the Right 4D Display on USART 4, using pin ?? & ??
#define PIN_MEGA_SPI_MISO 50     // Connect Pin 50 to 12 to use with CAN BOARD
#define PIN_MEGA_SPI_MOSI 51     // Connect Pin 51 to 11 to use with CAN BOARD
#define PIN_MEGA_SPI_SCK 52      // Connect Pin 52 to 13 to use with CAN BOARD
#define PIN_MEGA_SPI_CS 53       // Connect pin 53 to 10 to use with CAN BOARD

#define PIN_BRAKE_PEDAL 1
#define PIN_CLUTCH_PEDAL 2
#define PIN_INCREASE_SPEED_SWITCH 3
#define PIN_DECREASE_SPEED_SWITCH 4
#define PIN_ENABLE_SPEED_CONTROL_SWITCH 5
#define PIN_LOW_RANGE_GEAR_SELECTED_SWITCH 6
#define PIN_ACTUAL_ACCELARATOR_SENSOR 7
#define PIN_SIMULATED_ACCELARATOR_SENSOR 8


// Helper functions that don't belong in a class and aren't that important.
String v2s(String format, int value);
String v2s(String format, byte value);
String v2s(String format, word value);
String v2s(String format, unsigned long value);

class StopWatch
{
  public:
    StopWatch();

    void start();
    unsigned long elapsed();
    bool notStarted();

  private:
    unsigned long m_start;
};

class PumaFile
{
  public:
    PumaFile();
    ~PumaFile();

    static bool erase(String fileName);
    static bool exists(String fileName);

    typedef enum PUMA_FILE_MODE {
      READ = 'r',
      WRITE = 'w',
      APPEND = 'a'
    } PUMA_FILE_MODE;

    bool open(String fileName, PUMA_FILE_MODE mode, bool eraseIfExists = true);
    void close();

    unsigned long bytesAvailable();
    unsigned long size();

    String readString();
    bool print(String s);
    bool println(String s);

    byte readByte();
    bool writeByte(byte b);

    word readWord();
    bool writeWord(word w);

    unsigned long pointer();

  private:
    unsigned long m_bytesAvailable;
    String m_fileName;
    word m_handle;
};

String uniqueLogFileName();
void initLogging();
void logRawData(CAN_Frame *message);
void logObdData(String s);
void selfTest();

#endif


