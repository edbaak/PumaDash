#include <Arduino.h>
#include "OBD.h"
#include <CAN.h>
#include <SD.h>
#include "VehicleDash-AR-Const.h" 

#define PIN_CAN_BOARD_LED2 8

/*
  Constructor. Only copies the can pointer for internal usage.
*/  
OBD::OBD(CANClass *can) 
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
void OBD::begin(uint32_t CAN_BitRate, uint8_t mode)
{
	m_CAN->begin(CAN_BitRate, mode);
}

/*
  end closes the CAN connection.
*/  
void OBD::end()
{
	m_CAN->end();
}

void OBD::refresh(String logFileName)
{
  if (m_speed.needsUpdate()) {
#ifndef LOOPBACK_MODE
    requestPID(PID_SPEED);
#else
    static word speed = 0;
    static bool increment_speed = true;
    if (increment_speed)
      speed += 3;
    else
      speed -= 3;  
    if (speed >= 115 || speed == 0)
      increment_speed = !increment_speed;
    simulateReply(PID_REPLY, PID_SPEED, speed);
#endif    
  }
  
  if (m_rpm.needsUpdate()) {  
#ifndef LOOPBACK_MODE
    requestPID(PID_RPM);
#else
    static word rpm = 0;
    static bool increment_rpm = true;
    if (increment_rpm)
      rpm += 300;
    else
      rpm -= 300;  
    if (rpm >= 6000 || rpm == 0)
      increment_rpm = !increment_rpm;
    simulateReply(PID_REPLY, PID_RPM, rpm);  
#endif
  }

  simulateReply(PID_REPLY+1, 0x1F, 0xFEFE);  
  
#ifndef LOOPBACK_MODE
  if (m_coolantTemp.needsUpdate())
    requestPID(PID_COOLANT_TEMP);
#endif
  
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

void OBD::simulateReply(uint16_t id, uint16_t pid, uint16_t value)
{
  CAN_Frame message;
    
  // Prepare message
  message.id = id;
  message.valid = true;   // todo: assuming this is the right value
  message.rtr = 0;        // Must be dominant (0) for data frames and recessive (1) for remote request frames
  message.extended = 0;   // Must be dominant (0) for base frame format with 11-bit identifiers
  message.fid = 0;        // todo: meaning unclear.
  message.priority = 0;   // todo: not sure about this value. I'm assuming prio 0 is highest.
  message.length = 8;     // eight data bytes follow
  message.timeout = 0;    // zero will disable waiting
  message.data[0] = 0x03; // this means there are x valid bytes with data
  message.data[1] = 0x41; // mode 1 = show current data, mode 2 = show freeze frame
  message.data[2] = pid;  // the requested pid
  message.data[3] = value;
  message.data[4] = 0x00; // unused byte
  message.data[5] = 0x00; // unused byte
  message.data[6] = 0x00; // unused byte
  message.data[7] = 0x00; // unused byte    
    
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
	message.id = PID_REQUEST;
	message.valid = true;   // todo: assuming this is the right value
	message.rtr = 0;        // Must be dominant (0) for data frames and recessive (1) for remote request frames
	message.extended = 0;   // Must be dominant (0) for base frame format with 11-bit identifiers
	message.fid = 0;        // todo: meaning unclear.
	message.priority = 0;   // todo: not sure about this value. I'm assuming prio 0 is highest.
	message.length = 8;     // eight data bytes follow
	message.timeout = 10;    // zero will disable waiting
	message.data[0] = 0x02; // this means there are two valid bytes with data
	message.data[1] = 0x01; // mode 1 = show current data, mode 2 = show freeze frame
	message.data[2] = pid;  // the requested pid
	message.data[3] = 0x00; // unused byte
	message.data[4] = 0x00; // unused byte
	message.data[5] = 0x00; // unused byte
	message.data[6] = 0x00; // unused byte
	message.data[7] = 0x00;	// unused byte		
    
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

void OBD::setCanFilters(long filter0, long filter2, long filter1, long filter3, long filter4, long filter5)
{
  m_CAN->clearFilters();
  if (filter0 > 0) {
    Serial.println("Can filters ON");
    setCanRxMask(RXM0, 0x7FF); 
    setCanRxFilter(RXF0, filter0);
    setCanRxFilter(RXF1, filter1);
    setCanRxMask(RXM1, 0x7FF);
    setCanRxFilter(RXF2, filter2);
    setCanRxFilter(RXF3, filter3);
    setCanRxFilter(RXF4, filter4);
    setCanRxFilter(RXF5, filter5);      
  } else {
    Serial.println("Can filters OFF");    
  }
}

/*
mask = either RXM0 or RXM1
MaskValue is either an 11 or 29 bit mask value to set
ext is true if the mask is supposed to be extended (29 bit)
zero bits for the mask means the related filter bit needs to match exactly, i.e. $7FF for the mask will accept any message ID regardless of the filter value.
*/
void OBD::setCanRxMask(CAN_RX_Mask mask, long MaskValue, bool ext) 
{
  byte buf[4];  
  if (ext) { //fill out all 29 bits
    buf[0] = byte((MaskValue << 3) >> 24);
    buf[1] = byte((MaskValue << 11) >> 24) & B11100000;
    buf[1] |= byte((MaskValue << 14) >> 30);
    buf[2] = byte((MaskValue << 16)>>24);
    buf[3] = byte((MaskValue << 24)>>24);
  }
  else { //make sure to set mask as 11 bit standard mask
    buf[0] = byte((MaskValue << 21)>>24);
    buf[1] = byte((MaskValue << 29) >> 24) & B11100000;
    buf[2] = 0;
    buf[3] = 0;
  }

  Serial.print("MASK: ");
  Serial.print(buf[0], BIN);
  Serial.print(" - ");
  Serial.print(buf[1], BIN);
  Serial.print(" - ");
  Serial.print(buf[2], BIN);
  Serial.print(" - ");
  Serial.println(buf[3], BIN);
  m_CAN->setMask(mask, buf[0], buf[1], buf[2], buf[3]); 
}

/*
filter = RXF0, RXF1, RXF2, RXF3, RXF4, RXF5 (pick one)
FilterValue = 11 or 29 bit filter to use
ext is true if this filter should apply to extended frames or false (default) if it should apply to standard frames.
*/

void OBD::setCanRxFilter(CAN_RX_Filter filter, long FilterValue, bool ext) 
{
  byte buf[4];
  
  if (ext) { //fill out all 29 bits
    buf[0] = byte((FilterValue << 3) >> 24);
    buf[1] = byte((FilterValue << 11) >> 24) & B11100000;
    buf[1] |= byte((FilterValue << 14) >> 30);
    buf[1] |= B00001000; //set EXIDE
    buf[2] = byte((FilterValue << 16)>>24);
    buf[3] = byte((FilterValue << 24)>>24);
  }
  else { //make sure to set mask as 11 bit standard mask
    buf[0] = byte((FilterValue << 21)>>24);
    buf[1] = byte((FilterValue << 29) >> 24) & B11100000;
    buf[2] = 0;
    buf[3] = 0;
  }

  Serial.print("FILT: ");
  Serial.print(buf[0], BIN);
  Serial.print(" - ");
  Serial.print(buf[1], BIN);
  Serial.print(" - ");
  Serial.print(buf[2], BIN);
  Serial.print(" - ");
  Serial.println(buf[3], BIN);

  m_CAN->setFilter(filter, buf[0], buf[1], buf[2], buf[3]); 
}

bool OBD::readMessage(String &logString)
{
  if (m_CAN->available() != 0) // One or more messages available?
  {
    // message will follow the CAN structure of ID, RTR, length, data. Allows both Extended & Standard
    CAN_Frame message = m_CAN->read(); 
    Serial.print(message.id, HEX);
    
    if (message.id == 0x7E8) {
    	processMessage(message);
        
  		char buf[150]; 
  		sprintf(buf, "PID %02X, LEN %02X, MODE %02X, DATA %02X, %02X, %02X, %02X, %02X", 
  			message.data[2], message.data[0], message.data[1],
  			message.data[3], message.data[4], message.data[5],
  			message.data[6], message.data[7]);
  		logString = buf;
  		return true; //message.id == 0x7E8;
  	} else {
  	  addUnhandledPID(message.id);
  	}
  }
  return false;
}
         
bool OBD::processMessage(CAN_Frame message)
{
	uint16_t pid = message.id;
	uint8_t *data = &message.data[0];
		
	if (pid == PID_REPLY) {
	  pid = message.data[2];
	  data = &message.data[3];
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
  
