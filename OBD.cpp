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

  addDataObject(new OBDWordValue(PID_RPM, "Rpm", 50, DIV4_WORD_CONVERSION, 0, 6000, 300));
  addDataObject(new OBDByteValue(PID_SPEED, "Speed", 250, PLAIN_BYTE_CONVERSION, 0, 115, 3));
  addDataObject(new OBDLongValue(PID_COOLANT_TEMP, "Coolant Temperature", 3000, INT_MINUS40, -25, 130, 4));
  //  addDataObject(new OBDByteValue(PID_BAROMETRIC_PRESSURE, "Barometric Pressure", 30000,
  addDataObject(new OBDLongValue(PID_INTAKE_AIR_TEMP, "Intake Air Temperature", 10000, INT_MINUS40, 10, 50, 5));
  addDataObject(new OBDLongValue(PID_AMBIENT_AIR_TEMP, "Ambient Air Temperature", 10000, INT_MINUS40, 10, 50, 5));
  //  addDataObject(new OBDFloatValue(PID_CONTROL_MODULE_VOLTAGE, "Battery Voltage", 5000, WORD_DIV1000 // V
  //  addDataObject(new OBDFloatValue(PID_ENGINE_FUEL_RATE, "Fuel Rate", 3000, WORD_DIV20 // L/h
  //  addDataObject(new OBDByteValue(PID_FUEL_LEVEL, "Fuel Level", 3000, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(new OBDLongValue(PID_ENGINE_OIL_TEMP, "Engine Oil Temperature", 2000, BYTE_MINUS40
  //  addDataObject(new OBDByteValue(PID_CALCULATED_ENGINE_LOAD, "Engine Load", 1000, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(new OBDByteValue(PID_ABSOLUTE_ENGINE_LOAD, "Abs Engine Load", 1000, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(new OBDLongValue(PID_ENGINE_TORQUE_DEMANDED, "Torque Demanded", 1000, INT_MINUS125 // %
  //  addDataObject(new OBDLongValue(PID_ENGINE_TORQUE_PERCENTAGE, "Torque Percentage", 1000, INT_MINUS125 // %

  //  addDataObject(new OBDWordValue(PID_EVAP_SYS_VAPOR_PRESSURE, SHIFT_RIGHT_2 // kPa
  //  addDataObject(new OBDByteValue(PID_FUEL_PRESSURE, TIMES_3 // kPa
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
  //  addDataObject(new OBDLongValue(PID_TIMING_ADVANCE, BYTE_DIV2_MINUS64
  //  addDataObject(new OBDWordValue(PID_DISTANCE_SINCE_DTC_CLEARED, PLAIN_WORD_CONVERSION: // km
  //  addDataObject(new OBDWordValue(PID_DISTANCE_WITH_MIL_ON, PLAIN_WORD_CONVERSION: // km
  //  addDataObject(new OBDWordValue(PID_RUN_TIME_WITH_MIL_ON, PLAIN_WORD_CONVERSION: // minute
  //  addDataObject(new OBDWordValue(PID_TIME_SINCE_DTC_CLEARED, PLAIN_WORD_CONVERSION: // minute
  //  addDataObject(new OBDWordValue(PID_RUNTIME_SINCE_ENG_START, PLAIN_WORD_CONVERSION: // second
  //  addDataObject(new OBDWordValue(PID_FUEL_RAIL_PRESSURE, PLAIN_WORD_CONVERSION: // kPa
  //  addDataObject(new OBDWordValue(PID_ENGINE_REF_TORQUE, PLAIN_WORD_CONVERSION: // Nm
  //  addDataObject(new OBDLongValue(PID_SHORT_TERM_FUEL_TRIM_1, INT_MINUS128_TIMES100_DIV128
  //  addDataObject(new OBDLongValue(PID_LONG_TERM_FUEL_TRIM_1, INT_MINUS128_TIMES100_DIV128
  //  addDataObject(new OBDLongValue(PID_SHORT_TERM_FUEL_TRIM_2, INT_MINUS128_TIMES100_DIV128
  //  addDataObject(new OBDLongValue(PID_LONG_TERM_FUEL_TRIM_2, INT_MINUS128_TIMES100_DIV128
  //  addDataObject(new OBDLongValue(PID_EGR_ERROR, INT_MINUS128_TIMES100_DIV128
  //  addDataObject(new OBDLongValue(PID_FUEL_INJECTION_TIMING, LONG_MINUS26880_DIV128
  //  addDataObject(new OBDLongValue(PID_CATALYST_TEMP_B1S1, LONG_DIV10_MINUS40
  //  addDataObject(new OBDLongValue(PID_CATALYST_TEMP_B2S1, LONG_DIV10_MINUS40
  //  addDataObject(new OBDLongValue(PID_CATALYST_TEMP_B1S2, LONG_DIV10_MINUS40
  //  addDataObject(new OBDLongValue(PID_CATALYST_TEMP_B2S2, LONG_DIV10_MINUS40
  //  addDataObject(new OBDLongValue(PID_AIR_FUEL_EQUIV_RATIO, LONG_TIMES200_DIV65536: // 0~200

#ifdef RECORD_UNKNOWN_PIDS
  for (word i = 0; i < MAX_UNKNOWN_PIDS; i++)
    m_unknownPIDS[i] = 0;
#endif
}

void PumaOBD::addDataObject(OBDDataValue *object)
{
  if (m_first == 0)
    m_first = object;
  else
    m_last->m_next = object;
  m_last = object;
}

OBDDataValue *PumaOBD::dataObject(uint8_t PID)
{
  // We start searching possibly somewhere in the middle of the list, at the current object with which we've had the last interaction
  OBDDataValue *tmp = m_current;
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
OBDDataValue *PumaOBD::iterateDataObject(bool needsUpdate)
{
  OBDDataValue *tmp = m_current;
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

void PumaOBD::setup()
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

  m_CAN.begin(PIN_MEGA_SPI_CS, PumaCAN::MCP_STD, PumaCAN::CAN_500KBPS, PumaCAN::MCP_16MHZ);
  m_CAN.setMode(LOOPBACK_OR_NORMAL);
  m_CAN.setMask(PumaCAN::MASK0, false, 0x07FF0000);
  m_CAN.setMask(PumaCAN::MASK1, false, 0x07FF0000);
  m_CAN.setFilter(PumaCAN::FILT0, false, 0x07E80000);
  m_CAN.setFilter(PumaCAN::FILT2, false, 0x07E80000);
}

void PumaOBD::update()
{
  // First we process messages in the RX buffer, so that we clear it and don't ask for updates that we just received
  byte i = 0; // add a safety net against infinite loop
  while (readMessage() && i++ < 4) {
  }  // Read and process OBD messages

  // Now check if there is any data that needs an update.
  // We only ask for MAX 2 elements at a time, because the MCP2515 can only handle two sets of data.
  // To ensure that every data element gets a chance we rotate through the list in a round robin fashion
  OBDDataValue *tmp = iterateDataObject(true);
  if (tmp) {
    updateSensor(tmp);
    m_current = tmp;
    updateSensor(iterateDataObject(true));
  }
}

void PumaOBD::updateSensor(OBDDataValue *sensor)
{
  if (sensor) {
    CAN_Frame message;
    message.m_length = 8;     // eight data bytes follow
#ifndef LOOPBACK_MODE
    // Ask for a sensor value from the OBD bus, but don't wait for a reply, i.e. some time in the future the ECU is hopefully
    // going to respond and give a reply. This way, we don't have to go into blocking loops waiting for ECU responses and can
    // keep the system running as fast as possible. The staleness of data is dealt with separately (by updateRequired()).
    message.m_id = PID_REQUEST;
    message.m_data[0] = 0x02; // two valid bytes with data following
    message.m_data[1] = 0x01; // mode 1 = show current data, mode 2 = show freeze frame
    message.m_data[2] = sensor->pid();  // the requested pid
    m_CAN.write(message);
#else
    // Simulate a sensor value and push it onto the OBD bus
    uint8_t dl = sensor->dataBytes();
    message.m_id = PID_REPLY;
    message.m_data[0] = dl + 2; // extra byte for mode and pid before actual data starts
    message.m_data[1] = 0x41; // mode 1 = show current data, mode 2 = show freeze frame
    message.m_data[2] = sensor->pid(); // The pid to which the simulated data applies
    if (dl > 0) {
      if (dl == 1) {
        if (sensor->dataConversion() == INT_MINUS40) {
          message.m_data[3] = sensor->simulateLong();
        } else {
          message.m_data[3] = sensor->simulateByte();
        }
      } if (dl == 2) {
        uint16_t value = sensor->simulateWord();
        message.m_data[3] = value >> 8;
        message.m_data[4] = value && 0x00FF;
      } else if (dl == 3) {
        /*
                  int8_t value = tmp->simulateInt();
                  message.m_data[3] = value >> 16;
                  message.m_data[4] = value >> 8 && 0x0000FF;
                  message.m_data[5] = value && 0x0000FF;
        */
      }
    }
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

      Serial.print(message.m_id, HEX);
      Serial.print(", ");
      Serial.println(buf);
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

    OBDDataValue *object = dataObject(pid);
    if (object != &m_invalidPID) {
      object->setValue(data);
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

OBDDataValue::OBDDataValue()
{
  m_pid = 0;
  m_updateInterval = 0;
  m_lastUpdate = 0;
  m_updateRequested = 0;
  m_label = "UNDEF";
  m_conversion = PLAIN_BYTE_CONVERSION;
  m_next = 0;
}

OBDDataValue::OBDDataValue(uint8_t pid, String label, uint16_t updateInterval, OBD_DATA_CONVERSION conversion)
{
  m_pid = pid;
  m_updateInterval = updateInterval;
  m_lastUpdate = 0;
  m_updateRequested = 0;
  m_label = label;
  m_conversion = conversion;
  m_next = 0;
}

OBDDataValue::~OBDDataValue()
{
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

String OBDDataValue::label()
{
  return m_label;
}

String OBDDataValue::toString()
{
  return 'UNDEF";
}

OBDByteValue::OBDByteValue(uint8_t pid, String label, uint16_t updateInterval, OBD_DATA_CONVERSION conversion, byte min, byte max, byte step) :
  OBDDataValue(pid, label, updateInterval, conversion)
{
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

void OBDByteValue::setValue(uint8_t *data)
{
  uint8_t tmp = (uint8_t) * data;
  if (m_conversion == PLAIN_BYTE_CONVERSION)
    setValue(tmp);
  //TODO: add more conversions?
}

byte OBDByteValue::byteValue()
{
  return m_value;
}

#ifdef LOOPBACK_MODE
uint8_t OBDByteValue::simulateByte()
{
  if (m_simIncrease) {
    m_simValue += m_simStepValue;
    if (m_simValue >= m_simMaxValue) m_simIncrease = false;
  } else {
    m_simValue -= m_simStepValue;
    if (m_simValue <= m_simMinValue) m_simIncrease = true;
  }
  if (m_conversion == PLAIN_BYTE_CONVERSION)
    return m_simValue;
  return 0;
}
#endif

OBDLongValue::OBDLongValue(uint8_t pid, String label, uint16_t updateInterval, OBD_DATA_CONVERSION conversion, long min, long max, long step) :
  OBDDataValue(pid, label, updateInterval, conversion)
{
  m_value = 0;
#ifdef LOOPBACK_MODE
  m_simValue = min;
  m_simIncrease = true;
  m_simMinValue = min;
  m_simMaxValue = max;
  m_simStepValue = step;
#endif
}

String OBDLongValue::toString()
{
  char buf[50];
  sprintf(buf, "%li", m_value);
  return buf;
}

void OBDLongValue::setValue(long newValue)
{
  resetUpdateTimer();
  m_value = newValue;
}

void OBDLongValue::setValue(uint8_t *data)
{
  int32_t tmp = *data;
  if (m_conversion == INT_MINUS40)
    setValue(tmp - 40);
  //TODO: add more conversions?
}

long OBDLongValue::longValue()
{
  return m_value;
}

#ifdef LOOPBACK_MODE
uint8_t OBDLongValue::simulateLong()
{
  if (m_simIncrease) {
    m_simValue += m_simStepValue;
    if (m_simValue >= m_simMaxValue) m_simIncrease = false;
  } else {
    m_simValue -= m_simStepValue;
    if (m_simValue <= m_simMinValue) m_simIncrease = true;
  }
  if (m_conversion == INT_MINUS40) {
    return uint8_t(m_simValue + 40);
  }
  //TODO: add more conversions?
  return uint8_t(m_simValue);
}
#endif


OBDWordValue::OBDWordValue(uint8_t pid, String label, uint16_t updateInterval, OBD_DATA_CONVERSION conversion, word min, word max, word step) :
  OBDDataValue(pid, label, updateInterval, conversion)
{
  m_value = 0;
#ifdef LOOPBACK_MODE
  m_simValue = min;
  m_simIncrease = true;
  m_simMinValue = min;
  m_simMaxValue = max;
  m_simStepValue = step;
#endif
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

void OBDWordValue::setValue(uint8_t *data)
{
  uint16_t tmp = *data * 256;
  data++;
  tmp += *data;

  if (m_conversion == PLAIN_WORD_CONVERSION)
    setValue(tmp);
  else if (m_conversion == DIV4_WORD_CONVERSION)
    setValue(tmp / 4);
  //TODO: add more conversions?
}

word OBDWordValue::wordValue()
{
  return m_value;
}

#ifdef LOOPBACK_MODE
uint16_t OBDWordValue::simulateWord()
{
  if (m_simIncrease) {
    m_simValue += m_simStepValue;
    if (m_simValue >= m_simMaxValue) m_simIncrease = false;
  } else {
    m_simValue -= m_simStepValue;
    if (m_simValue <= m_simMinValue) m_simIncrease = true;
  }

  if (m_conversion == PLAIN_WORD_CONVERSION)
    return m_simValue;
  else if (m_conversion == DIV4_WORD_CONVERSION)
    return m_simValue * 4;
  //TODO: add more conversions?

  return 0;
}
#endif



