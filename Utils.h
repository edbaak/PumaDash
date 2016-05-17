#ifndef Utils_h
#define Utils_h
 
#if (ARDUINO >= 100)
	#include "Arduino.h" // for Arduino 1.0
#else
	#include "WProgram.h" // for Arduino 23
#endif

// Define the baudrate at which we're running the 4D display
#define DISPLAY_SPEED 115200

#define VEHICLEDASH_DEBUG

#define LOOPBACK_MODE

// Mega board PIN definitions
#define uLCD_DISPLAY_RESETLINE 4
#define PIN_CAN_BOARD_LED1 7
#define PIN_CAN_BOARD_LED2 8
#define PIN_CAN_BOARD_SD_chipSelect 9 // SparkFun CAN-Bus Shield SPI chip select pin for SD card
#define PIN_LEGACY_SPI_CS 10
#define PIN_LEGACY_SPI_MOSI 11
#define PIN_LEGACY_SPI_MISO 12
#define PIN_LEGACY_SPI_SCK 13
#define DisplaySerial Serial1 // We're running the 4D Display on USART 2, pin 18 & 19
#define PIN_MEGA_SPI_MISO 50 // Connect Pin 50 to 12 to use with CAN BOARD
#define PIN_MEGA_SPI_MOSI 51 // Connect Pin 51 to 11 to use with CAN BOARD
#define PIN_MEGA_SPI_SCK 52 // Connect Pin 52 to 13 to use with CAN BOARD
#define PIN_MEGA_SPI_CS 53 // Connect pin 53 to 10 to use with CAN BOARD


#define LOGFILE_PREFIX 165
extern String g_logFileName;

void uniqueLogFileName();
void initLogging();

#endif
