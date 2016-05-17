#include <Arduino.h>
#include "Utils.h" 
#include "OBD.h"
#include "CAN.h"
#include <SD.h>

/*
  Constructor. Only copies the can pointer for internal usage.
*/  
OBD::OBD(MCP_CAN *can) 
{
  m_CAN = can;
  for (word i=0; i < MAX_UNKNOWN_PIDS; i++)
    m_unknownPIDS[i] = 0;

  m_speed.init(PID_SPEED, 250);
  m_rpm.init(PID_RPM, 100);
  m_coolantTemp.init(PID_COOLANT_TEMP, 5000);  
}

/*
  The usual 'begin' function.
*/  
void OBD::begin(MCP_CAN::CAN_SPEED bitRate, CAN_MODE mode)
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

  m_CAN->begin(MCP_CAN::MCP_STD, bitRate, MCP_CAN::MCP_16MHZ);
  m_CAN->setMode(mode);
}

/*
  end closes the CAN connection.
*/  
void OBD::end()
{
}

void OBD::refresh(String logFileName)
{
  if (m_speed.needsUpdate()) {
#ifndef LOOPBACK_MODE
    requestPID(PID_SPEED);
#else
    static uint8_t speed = 0;
    static bool increment_speed = true;
    if (increment_speed)
      speed += 3;
    else
      speed -= 3;  
    if (speed >= 115 || speed == 0)
      increment_speed = !increment_speed;
    simulateReply(PID_REPLY, PID_SPEED, speed);
    simulateReply(PID_REPLY+1, 0x1F, (uint8_t)50);   // Generate noise that should be filtered out
#endif    
  }
  
  if (m_rpm.needsUpdate()) {  
#ifndef LOOPBACK_MODE
    requestPID(PID_RPM);
#else
    static uint16_t rpm = 0;
    static bool increment_rpm = true;
    if (increment_rpm)
      rpm += 300;
    else
      rpm -= 300;  
    if (rpm >= 24000 || rpm == 0)
      increment_rpm = !increment_rpm;
    simulateReply(PID_REPLY, PID_RPM, rpm);  
#endif
  }
  
  if (m_coolantTemp.needsUpdate()) {
#ifndef LOOPBACK_MODE
    requestPID(PID_COOLANT_TEMP);
#endif
  }
  
  static bool ledD8_toggle = false;
  String logString;
  while (readMessage(logString)) {
    ledD8_toggle = !ledD8_toggle;
    if (ledD8_toggle) digitalWrite(PIN_CAN_BOARD_LED2, HIGH); 
    else digitalWrite(PIN_CAN_BOARD_LED2, LOW);
    Serial.println(logString);
    
    if (logFileName != "")
      writeToSDFile(logFileName, logString);
  }  
}

void OBD::simulateReply(uint16_t id, uint16_t pid, uint8_t value)
{
  CAN_Frame message;
    
  // Prepare message
  message.m_id = id;
  message.m_valid = true;   // todo: assuming this is the right value
  message.m_rtr = 0;        // Must be dominant (0) for data frames and recessive (1) for remote request frames
  message.m_extended = 0;   // Must be dominant (0) for base frame format with 11-bit identifiers
  message.m_length = 8;     // eight data bytes follow
  message.m_data[0] = 0x03; // this means there are x valid bytes with data
  message.m_data[1] = 0x41; // mode 1 = show current data, mode 2 = show freeze frame
  message.m_data[2] = pid;  // the requested pid
  message.m_data[3] = value;
  message.m_data[4] = 0x00; // unused byte
  message.m_data[5] = 0x00; // unused byte
  message.m_data[6] = 0x00; // unused byte
  message.m_data[7] = 0x00; // unused byte    
    
  m_CAN->write(message);
}

void OBD::simulateReply(uint16_t id, uint16_t pid, uint16_t value)
{
  CAN_Frame message;
    
  // Prepare message
  message.m_id = id;
  message.m_valid = true;   // todo: assuming this is the right value
  message.m_rtr = 0;        // Must be dominant (0) for data frames and recessive (1) for remote request frames
  message.m_extended = 0;   // Must be dominant (0) for base frame format with 11-bit identifiers
  message.m_length = 8;     // eight data bytes follow
  message.m_data[0] = 0x03; // this means there are x valid bytes with data
  message.m_data[1] = 0x41; // mode 1 = show current data, mode 2 = show freeze frame
  message.m_data[2] = pid;  // the requested pid
  message.m_data[3] = value >> 8;
  message.m_data[4] = value && 0x00FF;
  message.m_data[5] = 0x00; // unused byte
  message.m_data[6] = 0x00; // unused byte
  message.m_data[7] = 0x00; // unused byte    
    
  m_CAN->write(message);
}

// Simple helper function to write a string to a logging file
void OBD::writeToSDFile(String fileName, String s)
{
  File dataFile = SD.open(fileName, FILE_WRITE);
  if (dataFile) {
    dataFile.print(millis());
    dataFile.print(" ");
    dataFile.println(s);
    dataFile.close();   //close file
  } else {
    Serial.println("Error opening SD file for logging");
  }
}

/*
  requestPID requests the ECU for a specific pid, but it's not waiting for a reply, i.e. some time in the future the ECU is hopefully 
  going to respond and give a reply. This way, we don't have to go into blocking loops waiting for ECU responses and can keep the 
  system running as fast as possible.
  
  The staleness of data is dealt with separately.
*/  
void OBD::requestPID(uint16_t pid)
{
/*
  uint32_t id      : 29;  // if (extended == CAN_RECESSIVE) { extended ID } else { standard ID }
  uint8_t valid    : 1;   // To avoid passing garbage frames around
  uint8_t rtr      : 1;   // Remote Transmission Request Bit (RTR)
  uint8_t extended : 1;   // Identifier Extension Bit (IDE)
  uint32_t fid;           // family ID
  uint8_t priority : 4;   // Priority but only important for TX frames and then only for special uses.
  uint8_t length   : 4;   // Data Length
  uint16_t timeout;       // milliseconds, zero will disable waiting
  uint8_t data[8];        // Message data
*/
 
  CAN_Frame message;
    
	// Prepare message
	message.m_id = PID_REQUEST;
	message.m_valid = true;   // todo: assuming this is the right value
	message.m_rtr = 0;        // Must be dominant (0) for data frames and recessive (1) for remote request frames
	message.m_extended = 0;   // Must be dominant (0) for base frame format with 11-bit identifiers
	message.m_length = 8;     // eight data bytes follow
	message.m_data[0] = 0x02; // this means there are two valid bytes with data
	message.m_data[1] = 0x01; // mode 1 = show current data, mode 2 = show freeze frame
	message.m_data[2] = pid;  // the requested pid
	message.m_data[3] = 0x00; // unused byte
	message.m_data[4] = 0x00; // unused byte
	message.m_data[5] = 0x00; // unused byte
	message.m_data[6] = 0x00; // unused byte
	message.m_data[7] = 0x00;	// unused byte		
    
  m_CAN->write(message);
}

void OBD::addUnhandledPID(uint16_t pid)
{
  for (word i=0; i < MAX_UNKNOWN_PIDS; i++) {
    // If we have reached the end of the list, we apparently don't know the PID yet, so we add it. 
  	if (m_unknownPIDS[i] == 0) {
  		m_unknownPIDS[i] = pid;
  		printUnhandledPIDS();
  		return;
    }
    
    // Do we know the PID already?
    if (m_unknownPIDS[i] == pid)
    	return;
  }  	
}

void OBD::printUnhandledPIDS()
{
  Serial.print("UNHANDLED PIDS: ");
  for (word i=0; i < MAX_UNKNOWN_PIDS; i++) {
  	if (m_unknownPIDS[i] == 0) {
        Serial.println(" ");
  		return;
  	}
  		
    Serial.print(m_unknownPIDS[i], HEX);
    Serial.print(", ");
  }	
  Serial.println(" ");
}

void OBD::setCanFilters(uint32_t filter0, uint32_t filter2, uint32_t filter1, uint32_t filter3, uint32_t filter4, uint32_t filter5)
{
  if (filter0 > 0)
    m_CAN->setMask(MCP_CAN::MASK0, false, 0x07FF0000);
  else
    m_CAN->setMask(MCP_CAN::MASK0, false, 0);
  if (filter2 > 0)    
    m_CAN->setMask(MCP_CAN::MASK1, false, 0x07FF0000);
  else
    m_CAN->setMask(MCP_CAN::MASK1, false, 0);
  m_CAN->setFilter(MCP_CAN::FILT0, false, filter0);
  m_CAN->setFilter(MCP_CAN::FILT1, false, filter1);
  m_CAN->setFilter(MCP_CAN::FILT2, false, filter2);
  m_CAN->setFilter(MCP_CAN::FILT3, false, filter3);
  m_CAN->setFilter(MCP_CAN::FILT4, false, filter4);
  m_CAN->setFilter(MCP_CAN::FILT5, false, filter5);  
}

bool OBD::readMessage(String &logString)
{
  if (m_CAN->available()) // One or more messages available?
  {
    // message will follow the CAN structure of ID, RTR, length, data. Allows both Extended & Standard
    CAN_Frame message = m_CAN->read(); 
    Serial.print(message.m_id, HEX);
    
    if (message.m_id == 0x7E8) {
    	processMessage(message);
        
  		char buf[150]; 
  		sprintf(buf, " ,PID %02X, LEN %02X, MODE %02X, DATA %02X, %02X, %02X, %02X, %02X", 
  			message.m_data[2], message.m_data[0], message.m_data[1],
  			message.m_data[3], message.m_data[4], message.m_data[5],
  			message.m_data[6], message.m_data[7]);
  		logString = buf;
  		return true;
  	} else {
  	  addUnhandledPID(message.m_id);
  	}
  }
  return false;
}
         
bool OBD::processMessage(CAN_Frame message)
{
	uint16_t pid = message.m_id;
	uint8_t *data = &message.m_data[0];
		
	if (pid == PID_REPLY) {
	  pid = message.m_data[2];
	  data = &message.m_data[3];
	}
//	Serial.print("Process message for 0x");
//	Serial.println(pid, HEX);
		
	switch (pid) {
		case PID_SPEED: 
		{
		  m_speed.setValue((uint8_t)*data);
		  return true;
		}
		case PID_RPM: 
		{
			uint16_t tmp = *data * 256;
			data++;
			tmp += *data;
			m_rpm.setValue(tmp / 4);
//			m_rpm.setValue(uint16_t(*data) / 4);
			return true;
		}	
		case PID_COOLANT_TEMP: 
		{
			m_coolantTemp.setValue((int8_t)*data - 40);
			return true;
		}
	}
/*
	case PID_BAROMETRIC_PRESSURE:
		OBDByteValue.setValue(data);
		return true;
	case PID_EVAP_SYS_VAPOR_PRESSURE: // kPa
		result = getLargeValue(data) >> 2;
		break;
	case PID_FUEL_PRESSURE: // kPa
		result = getSmallValue(data) * 3;
		break;
	case PID_INTAKE_AIR_TEMP:
	case PID_AMBIENT_AIR_TEMP:
	case PID_ENGINE_OIL_TEMP:
		result = getTemperatureValue(data);
		break;
	case PID_THROTTLE_POSITION:
	case PID_COMMANDED_EGR:
	case PID_COMMANDED_EVAPORATIVE_PURGE:
	case PID_FUEL_LEVEL:
	case PID_RELATIVE_THROTTLE_POS:
	case PID_ABSOLUTE_THROTTLE_POS_B:
	case PID_ABSOLUTE_THROTTLE_POS_C:
	case PID_ACC_PEDAL_POS_D:
	case PID_ACC_PEDAL_POS_E:
	case PID_ACC_PEDAL_POS_F:
	case PID_COMMANDED_THROTTLE_ACTUATOR:
	case PID_CALCULATED_ENGINE_LOAD:
	case PID_ABSOLUTE_ENGINE_LOAD:
	case PID_ETHANOL_FUEL:
	case PID_HYBRID_BATT_REMAINING_LIFE:
		result = getPercentageValue(data);
		break;
	case PID_MAF_AIR_FLOW_RATE: // grams/sec
		result = getLargeValue(data) / 100;
		break;
	case PID_TIMING_ADVANCE:
		result = (int)(getSmallValue(data) / 2) - 64;
		break;
	case PID_DISTANCE_SINCE_DTC_CLEARED: // km
	case PID_DISTANCE_WITH_MIL_ON: // km
	case PID_RUN_TIME_WITH_MIL_ON: // minute
	case PID_TIME_SINCE_DTC_CLEARED: // minute
	case PID_RUNTIME_SINCE_ENG_START: // second
	case PID_FUEL_RAIL_PRESSURE: // kPa
	case PID_ENGINE_REF_TORQUE: // Nm
		result = getLargeValue(data);
		break;
	case PID_CONTROL_MODULE_VOLTAGE: // V
		result = getLargeValue(data) / 1000;
		break;
	case PID_ENGINE_FUEL_RATE: // L/h
		result = getLargeValue(data) / 20;
		break;
	case PID_ENGINE_TORQUE_DEMANDED: // %
	case PID_ENGINE_TORQUE_PERCENTAGE: // %
		result = (int)getSmallValue(data) - 125;
		break;
	case PID_SHORT_TERM_FUEL_TRIM_1:
	case PID_LONG_TERM_FUEL_TRIM_1:
	case PID_SHORT_TERM_FUEL_TRIM_2:
	case PID_LONG_TERM_FUEL_TRIM_2:
	case PID_EGR_ERROR:
		result = ((int)getSmallValue(data) - 128) * 100 / 128;
		break;
	case PID_FUEL_INJECTION_TIMING:
		result = ((int32_t)getLargeValue(data) - 26880) / 128;
		break;
	case PID_CATALYST_TEMP_B1S1:
	case PID_CATALYST_TEMP_B2S1:
	case PID_CATALYST_TEMP_B1S2:
	case PID_CATALYST_TEMP_B2S2:
		result = getLargeValue(data) / 10 - 40;
		break;
	case PID_AIR_FUEL_EQUIV_RATIO: // 0~200
		result = (long)getLargeValue(data) * 200 / 65536;
		break;
	default:
		result = getSmallValue(data);
	}
*/
	return false;
}

OBDDataValue::OBDDataValue()
{
	m_updateInterval = 1000;
	m_lastUpdate = 0;
	m_updateRequested = 0;
	m_pid = PID_RPM; // this is the wrong PID, but if we forget to call init, we at least will be using a valid PID
}

void OBDDataValue::init(uint8_t pid, uint16_t updateInterval)
{
  m_pid = pid;
  m_updateInterval = updateInterval;
}

uint8_t OBDDataValue::pid()
{
  return m_pid;
}

bool OBDDataValue::needsUpdate()
{
  unsigned long cur_time = millis();

  if (m_updateRequested + m_updateInterval > cur_time)
    return false;
    
  if (m_lastUpdate + m_updateInterval < cur_time) {
    updateRequested();
    return true;
  }
  
  return false;    
}

void OBDDataValue::updateRequested()
{
  m_updateRequested = millis();
}

void OBDDataValue::resetUpdateTimer()
{
  m_updateRequested = 0;
  m_lastUpdate = millis();
}
  
OBDByteValue::OBDByteValue() : OBDDataValue()
{
  m_value = 0;
}

String OBDByteValue::toString()
{
	char buf[50];
	sprintf(buf, "%d", m_value);
	return buf;
}

void OBDByteValue::setValue(byte newValue)
{
	resetUpdateTimer();
	m_value = newValue;
}

byte OBDByteValue::value()
{
	return m_value;
}
  
OBDWordValue::OBDWordValue() : OBDDataValue()
{
  m_value = 0;
}

String OBDWordValue::toString()
{
	char buf[50];
	sprintf(buf, "%d", m_value);
	return buf;
}

void OBDWordValue::setValue(word newValue)
{
	resetUpdateTimer();
	m_value = newValue;
}

word OBDWordValue::value()
{
	return m_value;
}
  
