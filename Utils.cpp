#include "Utils.h"
#include <SD.h>

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

void initLogging()
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

