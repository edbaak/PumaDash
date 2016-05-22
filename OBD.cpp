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

#include <Arduino.h>
#include "Utils.h"
#include "OBD.h"
#include "CAN.h"
#include <SD.h>
#include <Diablo_Const4D.h>
#include "Display.h"


#ifdef LOOPBACK_MODE
#define LOOPBACK_OR_NORMAL PumaCAN::MCP_LOOPBACK
#else
#define LOOPBACK_OR_NORMAL PumaCAN::MCP_NORMAL
#endif

PumaOBD::PumaOBD()
{
  m_first = 0;
  m_last = 0;
  m_current = 0;
  m_display = 0;

  addDataObject(new OBDData(PID_RPM, "", "%4d", "Rpm", 50, WORD_DIV4, 0, 6000, 300));
  addDataObject(new OBDData(PID_SPEED, "", "%3d", "Km/h", 250, BYTE_NO_CONVERSION, 0, 115, 3));

  addDataObject(new OBDData(PID_COOLANT_TEMP, "Coolant", "%3d", "C", 3000, INT_MINUS40, -25, 130, 4));
  addDataObject(new OBDData(PID_INTAKE_AIR_TEMP, "Intake Air", "%3d", "C", 10000, INT_MINUS40, 10, 50, 5));
  addDataObject(new OBDData(PID_AMBIENT_AIR_TEMP, "Ambient Air", "%3d", "C", 10000, INT_MINUS40, 10, 50, 5));
  //  addDataObject(new OBDData(PID_ENGINE_OIL_TEMP, "Engine Oil", "%3d", "C", 2000, INT_MINUS40, 0, 150, 4));

  addDataObject(new OBDData(PID_BAROMETRIC_PRESSURE, "Air", "%4d", "mBar", 10000, BYTE_TIMES10, 950, 1150, 10));
  addDataObject(new OBDData(PID_FUEL_PRESSURE, "Fuel rail", "%4d", "kPa", 5000, BYTE_TIMES3, 0, 765, 30));

  addDataObject(new OBDData(PID_FUEL_LEVEL, "Tank Level", "%3d", "%", 3000, BYTE_PERCENTAGE, 0, 100, 2)); 
  addDataObject(new OBDData(PID_ENGINE_FUEL_RATE, "Fuel Rate", "%3.1f", "L/hr", 3000, WORD_DIV20, 0, 30, 1)); // L/h

  //  addDataObject(new OBDFloatValue(PID_CONTROL_MODULE_VOLTAGE, "Battery Voltage", 5000, WORD_DIV1000 // V
  //  addDataObject(new OBDByteValue(PID_CALCULATED_ENGINE_LOAD, "Engine Load", 1000, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(new OBDByteValue(PID_ABSOLUTE_ENGINE_LOAD, "Abs Engine Load", 1000, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(new OBDData(PID_ENGINE_TORQUE_DEMANDED, "Torque Demanded", 1000, INT_MINUS125 // %
  //  addDataObject(new OBDData(PID_ENGINE_TORQUE_PERCENTAGE, "Torque Percentage", 1000, INT_MINUS125 // %
  //  addDataObject(new OBDWordValue(PID_EVAP_SYS_VAPOR_PRESSURE, SHIFT_RIGHT_2 // kPa
  //  addDataObject(new OBDByteValue(PID_THROTTLE_POSITION, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(new OBDByteValue(PID_COMMANDED_EGR, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(new OBDByteValue(PID_COMMANDED_EVAPORATIVE_PURGE, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(new OBDByteValue(PID_RELATIVE_THROTTLE_POS, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(new OBDByteValue(PID_ABSOLUTE_THROTTLE_POS_B, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(new OBDByteValue(PID_ABSOLUTE_THROTTLE_POS_C, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(new OBDByteValue(PID_ACC_PEDAL_POS_D, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(new OBDByteValue(PID_ACC_PEDAL_POS_E, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(new OBDByteValue(PID_ACC_PEDAL_POS_F, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(new OBDByteValue(PID_COMMANDED_THROTTLE_ACTUATOR, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(new OBDByteValue(PID_ETHANOL_FUEL, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(new OBDByteValue(PID_HYBRID_BATT_REMAINING_LIFE, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(new OBDWordValue(PID_MAF_AIR_FLOW_RATE, WORD_DIV100 // grams/sec
  //  addDataObject(new OBDData(PID_TIMING_ADVANCE, BYTE_DIV2_MINUS64
  //  addDataObject(new OBDWordValue(PID_DISTANCE_SINCE_DTC_CLEARED, WORD_NO_CONVERSION: // km
  //  addDataObject(new OBDWordValue(PID_DISTANCE_WITH_MIL_ON, WORD_NO_CONVERSION: // km
  //  addDataObject(new OBDWordValue(PID_RUN_TIME_WITH_MIL_ON, WORD_NO_CONVERSION: // minute
  //  addDataObject(new OBDWordValue(PID_TIME_SINCE_DTC_CLEARED, WORD_NO_CONVERSION: // minute
  //  addDataObject(new OBDWordValue(PID_RUNTIME_SINCE_ENG_START, WORD_NO_CONVERSION: // second
  //  addDataObject(new OBDWordValue(PID_FUEL_RAIL_PRESSURE, WORD_NO_CONVERSION: // kPa
  //  addDataObject(new OBDWordValue(PID_ENGINE_REF_TORQUE, WORD_NO_CONVERSION: // Nm
  //  addDataObject(new OBDData(PID_SHORT_TERM_FUEL_TRIM_1, INT_MINUS128_TIMES100_DIV128
  //  addDataObject(new OBDData(PID_LONG_TERM_FUEL_TRIM_1, INT_MINUS128_TIMES100_DIV128
  //  addDataObject(new OBDData(PID_SHORT_TERM_FUEL_TRIM_2, INT_MINUS128_TIMES100_DIV128
  //  addDataObject(new OBDData(PID_LONG_TERM_FUEL_TRIM_2, INT_MINUS128_TIMES100_DIV128
  //  addDataObject(new OBDData(PID_EGR_ERROR, INT_MINUS128_TIMES100_DIV128
  //  addDataObject(new OBDData(PID_FUEL_INJECTION_TIMING, LONG_MINUS26880_DIV128
  //  addDataObject(new OBDData(PID_CATALYST_TEMP_B1S1, LONG_DIV10_MINUS40
  //  addDataObject(new OBDData(PID_CATALYST_TEMP_B2S1, LONG_DIV10_MINUS40
  //  addDataObject(new OBDData(PID_CATALYST_TEMP_B1S2, LONG_DIV10_MINUS40
  //  addDataObject(new OBDData(PID_CATALYST_TEMP_B2S2, LONG_DIV10_MINUS40
  //  addDataObject(new OBDData(PID_AIR_FUEL_EQUIV_RATIO, LONG_TIMES200_DIV65536: // 0~200

#ifdef RECORD_UNKNOWN_PIDS
  for (word i = 0; i < MAX_UNKNOWN_PIDS; i++)
    m_unknownPIDS[i] = 0;
#endif
}

void PumaOBD::addDataObject(OBDData *object)
{
  if (m_first == 0)
    m_first = object;
  else
    m_last->m_next = object;
  m_last = object;
}

OBDData *PumaOBD::dataObject(uint8_t PID)
{
  // We start searching possibly somewhere in the middle of the list, at the current object with which we've had the last interaction
  OBDData *tmp = m_current;
  while (tmp) {
    if (tmp->pid() == PID)
      return tmp;
    tmp = tmp->m_next;
  }

  // If we haven't found the PID, start at the beginning of the list until we reach the current object.
  if (m_current != m_first) {
    tmp = m_first;
    while (tmp && tmp != m_current) {
      if (tmp->pid() == PID)
        return tmp;
      tmp = tmp->m_next;
    }
  }
  return &m_invalidPID;
}

// DANGER: This function can return a NULL pointer!
OBDData *PumaOBD::iterateDataObject(bool needsUpdate)
{
  OBDData *tmp = m_current;
  if (tmp) {
    tmp = tmp->m_next;
    while (tmp && tmp != m_current) {
      if (!needsUpdate || tmp->needsUpdate())
        return tmp;
      tmp = tmp->m_next;
    }
  }

  // If we haven't found the PID, start at the beginning of the list until we reach the current object.
  if (m_current != m_first) {
    tmp = m_first;
    while (tmp && tmp != m_current) {
      if (!needsUpdate || tmp->needsUpdate())
        return tmp;
      tmp = tmp->m_next;
    }
  }

  if (m_first == m_last && m_first != 0 && m_first->needsUpdate())
    return m_first;

  return 0;
}

void PumaOBD::setup(PumaDisplay *display)
{
  m_display = display;

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

  m_CAN.begin(PIN_MEGA_SPI_CS, PumaCAN::MCP_STD, PumaCAN::CAN_500KBPS, PumaCAN::MCP_16MHZ);
  m_CAN.setMode(LOOPBACK_OR_NORMAL);
  m_CAN.setMask(PumaCAN::MASK0, false, 0x07FF0000);
  m_CAN.setMask(PumaCAN::MASK1, false, 0x07FF0000);
  m_CAN.setFilter(PumaCAN::FILT0, false, 0x07E80000);
  m_CAN.setFilter(PumaCAN::FILT2, false, 0x07E80000);
}

void PumaOBD::readRxBuffers()
{
  // First we process messages in the RX buffer, so that we clear it and don't ask for updates that we just received
  byte i = 0; // add a safety net against infinite loop
  while (readMessage() && i++ < 4) {
  }  // Read and process OBD messages
}

void PumaOBD::requestObdUpdates()
{
  // Now check if there is any data that needs an update.
  // We only ask for MAX 2 elements at a time, because the MCP2515 can only handle two sets of data.
  // To ensure that every data element gets a chance we rotate through the list in a round robin fashion
  OBDData *tmp = iterateDataObject(true);
  if (tmp) {
    updateSensor(tmp);
    m_current = tmp;
    updateSensor(iterateDataObject(true));
  }
}

void PumaOBD::updateSensor(OBDData *sensor)
{
  if (sensor) {
    CAN_Frame message;
    message.m_length = 8;     // eight data bytes follow
    
#ifdef LOOPBACK_MODE
    // Simulate a sensor value and push it onto the OBD bus
    message.m_id = PID_REPLY;
    message.m_data[1] = 0x41; // mode 1 = show current data, mode 2 = show freeze frame
    message.m_data[2] = sensor->pid(); // The pid to which the simulated data applies
    sensor->simulateData(&message);
    m_CAN.write(message);
#else
    // Ask for a sensor value from the OBD bus, but don't wait for a reply, i.e. some time in the future the ECU is hopefully
    // going to respond and give a reply. This way, we don't have to go into blocking loops waiting for ECU responses and can
    // keep the system running as fast as possible. The staleness of data is dealt with separately (by updateRequired()).
    message.m_id = PID_REQUEST;
    message.m_data[0] = 0x02; // two valid bytes with data following
    message.m_data[1] = 0x01; // mode 1 = show current data, mode 2 = show freeze frame
    message.m_data[2] = sensor->pid();  // the requested pid
    m_CAN.write(message);
#endif
  }
}

bool PumaOBD::readMessage()
{
  if (m_CAN.available()) { // One or more messages available?
    // message will follow the CAN structure of ID, RTR, length, data. Allows both Extended & Standard
    CAN_Frame message = m_CAN.read();

    if (message.m_id == 0x7E8) {
      processMessage(message);

      char buf[150];
      if (message.m_data[0] == 3) {
        sprintf(buf, "P %02X, M %02X, D %02X",
                message.m_data[2], message.m_data[1],
                message.m_data[3]);
      } else if (message.m_data[0] == 4) {
        sprintf(buf, "P %02X, M %02X, D %02X, %02X",
                message.m_data[2], message.m_data[1],
                message.m_data[3], message.m_data[4]);

      } else {
        sprintf(buf, "P %02X, L %02X, M %02X, D %02X, %02X, %02X, %02X, %02X",
                message.m_data[2], message.m_data[0], message.m_data[1],
                message.m_data[3], message.m_data[4], message.m_data[5],
                message.m_data[6], message.m_data[7]);
      }

      logData(buf);
#ifdef OBD_DEBUG
      Serial.print(message.m_id, HEX);
      Serial.print(", ");
      Serial.println(buf);
#endif
    } else {
#ifdef RECORD_UNKNOWN_PIDS
      addUnhandledPID(message.m_id);
#endif
    }
    return true;
  }
  return false;
}

bool PumaOBD::processMessage(CAN_Frame message)
{
  uint16_t pid = message.m_id;
  uint8_t *data = &message.m_data[0];

  if (pid == PID_REPLY) {
    pid = message.m_data[2];
    data = &message.m_data[3];

    OBDData *object = dataObject(pid);
    if (object != &m_invalidPID) {
      object->setValue(data);
      // TODO: If need be I can postpone the updateSensor until later, in which case I need to create a FIFO buffer that I cache updated sensors in
      m_display->updateSensor(object);
      return true;
    }
  }

  return false;
}

#ifdef RECORD_UNKNOWN_PIDS
void PumaOBD::addUnhandledPID(uint16_t pid)
{
  for (word i = 0; i < MAX_UNKNOWN_PIDS; i++) {
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
#endif

#ifdef RECORD_UNKNOWN_PIDS
void PumaOBD::printUnhandledPIDS()
{
  Serial.print("UNHANDLED PIDS: ");
  for (word i = 0; i < MAX_UNKNOWN_PIDS; i++) {
    if (m_unknownPIDS[i] == 0) {
      Serial.println(" ");
      return;
    }

    Serial.print(m_unknownPIDS[i], HEX);
    Serial.print(", ");
  }
  Serial.println(" ");
}
#endif

//*************************************************************************************

OBDData::OBDData()
{
  m_pid = 0;
  m_updateInterval = 0;
  m_lastUpdate = 0;
  m_updateRequested = 0;
  m_label = "";
  m_subLabel = "";
  m_conversion = BYTE_NO_CONVERSION;
  m_next = 0;
}

OBDData::~OBDData()
{
}

OBDData::OBDData(uint8_t pid, String label, String format, String subLabel, uint16_t updateInterval, OBD_DATA_CONVERSION conversion, long min, long max, long step)
{
  m_pid = pid;
  m_label = label;
  m_format = format;
  m_subLabel = subLabel;
  m_updateInterval = updateInterval;
  m_conversion = conversion;
  m_lastUpdate = 0;
  m_updateRequested = 0;
  m_next = 0;
  m_value = 0;
#ifdef LOOPBACK_MODE
  m_simValue = min;
  m_simIncrease = true;
  m_simMinValue = min;
  m_simMaxValue = max;
  m_simStepValue = step;
#else
  min = max = step;
#endif
}

uint8_t OBDData::pid()
{
  return m_pid;
}

bool OBDData::needsUpdate()
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

void OBDData::updateRequested()
{
  m_updateRequested = millis();
}

void OBDData::resetUpdateTimer()
{
  m_updateRequested = 0;
  m_lastUpdate = millis();
}

String OBDData::label()
{
  return m_label;
}

String OBDData::subLabel()
{
  return m_subLabel;
}

word OBDData::color()
{
  return LIGHTGREEN;
}

byte OBDData::valueLength()
{
  if (m_format.length() > 0 && m_format[0] == '%') {
    char l = m_format[1];
    switch (l) {
      case '1': return 1;
      case '2': return 2;
      case '3': return 3;
      case '4': return 4;
      case '5': return 5;
      case '6': return 6;
      case '7': return 7;
      case '8': return 8;
      case '9': return 9;
      default: return 4;
    }
  }
  // TODO: Not sure if this works well in all cases.
  return 4;
}

String OBDData::toString()
{
  char buf[20];
  if (m_conversion == WORD_DIV20) {
    float tmp = m_value;
    tmp /= 20;
    tmp = 3.1;
    sprintf(buf, m_format.c_str(), tmp);
  } else {
    sprintf(buf, m_format.c_str(), m_value);
  }
  return buf;
}

void OBDData::setValue(uint8_t *data)
{
  resetUpdateTimer();
  m_value = *data;

  if (m_conversion == WORD_NO_CONVERSION ||
      m_conversion == WORD_DIV4 ||
      m_conversion == WORD_DIV20) {
        m_value *= 256;
        data++;
        m_value += data && 0x00FF; 
      }
    
  switch (m_conversion) {
    case INT_MINUS40:
      m_value -= 40;
      break;
    case BYTE_TIMES10:
      m_value *= 10;
      break;
    case BYTE_TIMES3:
      m_value *= 3;
      break;
    case BYTE_PERCENTAGE:
      m_value = m_value * 100 / 255;
      break;
    case BYTE_NO_CONVERSION:
      break;
    case WORD_NO_CONVERSION:
      break;
    case WORD_DIV4:
      m_value /= 4;
      break;
    case WORD_DIV20:
//      m_value /= 20;
      break;
  }
}

#ifdef LOOPBACK_MODE
void OBDData::simulateData(CAN_Frame *message)
{
  if (m_simIncrease) {
    m_simValue += m_simStepValue;
    if (m_simValue >= m_simMaxValue) m_simIncrease = false;
  } else {
    m_simValue -= m_simStepValue;
    if (m_simValue <= m_simMinValue) m_simIncrease = true;
  }

  long tmp = 0;
  switch (m_conversion) {
    case INT_MINUS40:
      message->m_data[0] = 3;
      message->m_data[3] = uint8_t(m_simValue + 40);
      break;
    case BYTE_TIMES10:
      message->m_data[0] = 3;
      message->m_data[3] = uint8_t(m_simValue / 10);
      break;
    case BYTE_TIMES3:
      message->m_data[0] = 3;
      message->m_data[3] = uint8_t(m_simValue / 3);
      break;
    case BYTE_PERCENTAGE:
      message->m_data[0] = 3;
      message->m_data[3] = uint8_t(m_simValue *255 / 100);
      break;      
    case BYTE_NO_CONVERSION:
      message->m_data[0] = 3;
      message->m_data[3] = uint8_t(m_simValue);
      break;
    case WORD_NO_CONVERSION:
      message->m_data[0] = 4;
      message->m_data[3] = m_simValue >> 8;
      message->m_data[4] = m_simValue && 0x00FF;
      break;
    case WORD_DIV4:
      message->m_data[0] = 4;
      tmp = m_simValue * 4;
      message->m_data[3] = tmp >> 8;
      message->m_data[4] = tmp && 0x00FF;
      break;
    case WORD_DIV20:
      message->m_data[0] = 4;
      tmp = m_simValue * 20;
      message->m_data[3] = tmp >> 8;
      message->m_data[4] = tmp && 0x00FF;
      break;
  }
}
#endif


