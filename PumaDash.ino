/*
  Copyright (c) 2016 by Ed Baak
  
// Land Rover Defender MY12 Puma Dashboard
// Ford 2.2L TDCI uses CAN 11bit (500Kb)
// Protocol ISO 15765-4 11Bit (500Kb)
*/
#include "VD-Const.h"
#include <SPI.h>
#ifdef USE_CAN2
  #include "MCP2515.h"
#else
  #include <CAN.h>
  #include <CAN_AT90CAN.h>
  #include <CAN_K2X.h>
  #include <CAN_MCP2515.h>
  #include <CAN_SAM3X.h>
  #include <CAN_SJA1000.h>
  #include <sn65hvd234.h>
#endif
#include "OBD.h"
#include <SD.h>
#include <Bounce2.h>
#include "Tpms.h"
#include "Position.h"
#include "Speed.h"
#include <Diablo_Const4D.h>
#include <Diablo_Serial_4DLib.h>
#include "Display.h"

// Mega board PIN definitions
#define PIN_CAN_BOARD_LED1 7
#define PIN_CAN_BOARD_LED2 8
#define PIN_CAN_BOARD_SD_chipSelect 9 // SparkFun CAN-Bus Shield SPI chip select pin for SD card
#define PIN_LEGACY_SPI_CS 10
#define PIN_LEGACY_SPI_MOSI 11
#define PIN_LEGACY_SPI_MISO 12
#define PIN_LEGACY_SPI_SCK 13
#define PIN_MEGA_SPI_MISO 50 // Connect Pin 50 to 12 to use with CAN BOARD
#define PIN_MEGA_SPI_MOSI 51 // Connect Pin 51 to 11 to use with CAN BOARD
#define PIN_MEGA_SPI_SCK 52 // Connect Pin 52 to 13 to use with CAN BOARD
#define PIN_MEGA_SPI_CS 53 // Connect pin 53 to 10 to use with CAN BOARD

Diablo_Serial_4DLib Display(&DisplaySerial);
Tpms g_Tpms;
Direction g_DirectionControl;
CruiseCtrl g_CruiseControl;
#ifdef USE_CAN2
  MCP2515 g_Can(53); // Use Pin 53 for Chip Select on a Mega
#else
  CAN_MCP2515 g_Can(53); // Use Pin 53 for Chip Select on a Mega
#endif  
OBD g_Obd(&g_Can);

// Define which display is shown as the default. 
// We can change this by tapping the touchscreen
byte g_active_display = 1;
LeftDisplay g_LeftDisplay(&Display, &g_DirectionControl, &g_Tpms);
CenterDisplay g_CenterDisplay(&Display, &g_Obd);
RightDisplay g_RightDisplay(&Display, &g_CruiseControl);

#define LOGFILE_PREFIX 165
String g_logFileName;

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

void initSDCard()
{
  Serial.println("Initializing SD card");
  // Init pins for the two leds on the CAN board
  pinMode(PIN_CAN_BOARD_LED1, OUTPUT);
  pinMode(PIN_CAN_BOARD_LED2, OUTPUT);

  // Initialize SD card
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(PIN_CAN_BOARD_SD_chipSelect, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(PIN_CAN_BOARD_SD_chipSelect)) {
    Serial.println("SD Card failed, or not present");
  } else {
    uniqueLogFileName();
    Serial.print("Logging to ");
    Serial.println(g_logFileName);
  }
}

void initOBD()
{
  // Switch Pin 10-13 to INPUT mode so they are high impedance, floating. That way we can hardwire Pins 50-53 onto them, so that we can use the CAN-Board on a Mega.
  // Connect pin 53 to 10 == CS (Chip Select)
  // Connect Pin 52 to 13 == SCK (Clock)
  // Connect Pin 51 to 11 == MOSI (Master Out Slave In)
  // Connect Pin 50 to 12 == MISO (Master In Slave Out)
  pinMode(PIN_LEGACY_SPI_CS, INPUT); // set to high impedance, we're not actually using this pin
  pinMode(PIN_LEGACY_SPI_MOSI, INPUT); // set to high impedance, we're not actually using this pin
  pinMode(PIN_LEGACY_SPI_MISO, INPUT); // set to high impedance, we're not actually using this pin
  pinMode(PIN_LEGACY_SPI_SCK, INPUT); // set to high impedance, we're not actually using this pin
  pinMode(PIN_MEGA_SPI_MISO, INPUT);
  pinMode(PIN_MEGA_SPI_MOSI, OUTPUT);
  pinMode(PIN_MEGA_SPI_SCK, OUTPUT);
  pinMode(PIN_MEGA_SPI_CS, OUTPUT);

//  g_Obd.begin(CAN_BPS_500K, MCP2515_MODE_NORMAL);
  g_Obd.begin(CAN_BPS_500K, MCP2515_MODE_LOOPBACK);
}

// the setup function runs once when you press reset or power the board
void setup() {  
  Serial.begin(115200);
  
  initSDCard();
  initOBD();

  // we're assuming we're connecting to the left display, but will be switching to center or right display as needed in the code.
  // in practice it doesn't really matter as there really only one display connected and all three displays use the same
  // connection to communicate with the display. In other words, what changes is *what* information we present on the 
  // display, and not *how* we communicate with the display.
  g_LeftDisplay.reset();
}

// the loop function runs over and over again forever
void loop() {
  CAN_Frame message;
  message.id = PID_REPLY;
  message.length = 8;
  message.data[0] = 3;
  message.data[1] = 0x41;
  message.data[2] = PID_RPM;
  message.data[3] = 200;
  message.data[4] = 0;
  message.data[5] = 0;
  message.data[6] = 0;
  message.data[7] = 0;
  //g_Can.write(message);
  
  static bool init_display = true;
  static bool filters_on = true;
  
  g_Tpms.update();
  g_DirectionControl.update();
  g_Obd.refresh(g_logFileName);
  g_CruiseControl.update();

  if (!init_display && g_LeftDisplay.touchPressed()) {
    Display.gfx_Cls();
/*    
    g_active_display++;
    if (g_active_display > 2)
      g_active_display = 0;
*/
    init_display = true;  
    if (filters_on) {
      g_Obd.setCanFilters(0x7E8,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF);
//      g_Obd.setCanFilters(0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF,0xFFFF);
    } else {
      g_Obd.setCanFilters(0x00);
    }
    filters_on = !filters_on;    
  }

  if (init_display) {
    init_display = false;
    switch (g_active_display) {
      case 0: 
        g_LeftDisplay.init();
        g_LeftDisplay.redrawLabels(); 
        break;
      case 1: 
        g_CenterDisplay.init();
        g_CenterDisplay.redrawLabels(); 
        break;
      case 2: 
        g_RightDisplay.init();
        g_RightDisplay.redrawLabels(); 
        break;
    }
  }

  switch (g_active_display) {
    case 0: g_LeftDisplay.update(); break;
    case 1: g_CenterDisplay.update(); break;
    case 2: g_RightDisplay.update(); break;
  }
}

