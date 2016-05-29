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

#include <Arduino.h>
#include "Utils.h"
#include "OBD.h"
#include "CAN.h"
#include <Diablo_Const4D.h>
#include "Display.h"


#ifdef LOOPBACK_MODE
#define LOOPBACK_OR_NORMAL PumaCAN::MCP_LOOPBACK
#else
#define LOOPBACK_OR_NORMAL PumaCAN::MCP_NORMAL
#endif

PumaOBD::PumaOBD()
{
  m_firstData[0] = 0;
  m_firstData[1] = 0;
  m_lastData[0] = 0;
  m_lastData[1] = 0;
  m_curData[0] = 0;
  m_curData[1] = 0;

  m_rxFIFO_write = 0;
  m_rxFIFO_read = 0;
  m_rxFIFO_count = 0;

#ifdef PID_DISCOVERY_MODE
  addDataObject(false, new OBDData(PID_SUPPORTED_PID_01_20, "", "", "", ULONG_NO_CONVERSION, 0, 0));
  addDataObject(false, new OBDData(PID_SUPPORTED_PID_21_40, "", "", "", ULONG_NO_CONVERSION, 0, 0));
  //  addDataObject(false, new OBDData(PID_SUPPORTED_PID_41_60, "", "", "", ULONG_NO_CONVERSION, 0, 0));
  //  addDataObject(false, new OBDData(PID_SUPPORTED_PID_61_80, "", "", "", ULONG_NO_CONVERSION, 0, 0));
  //  addDataObject(false, new OBDData(PID_SUPPORTED_PID_81_A0, "", "", "", ULONG_NO_CONVERSION, 0, 0));
#endif
  addDataObject(true, new OBDData(PID_RPM, "", "%4d", "Rpm", WORD_DIV4, 0, 6000));
  addDataObject(true, new OBDData(PID_SPEED, "", "%3d", "Km/h", BYTE_NO_CONVERSION, 0, 115));

  addDataObject(false, new OBDData(PID_COOLANT_TEMP, "Coolant", "%3d", "C", INT_MINUS40, -25, 130));
  addDataObject(false, new OBDData(PID_INTAKE_AIR_TEMP, "Intake Air", "%3d", "C", INT_MINUS40, 10, 50));
  addDataObject(false, new OBDData(PID_AMBIENT_AIR_TEMP, "Ambient Air", "%3d", "C", INT_MINUS40, 10, 50));
  //  addDataObject(false, new OBDData(PID_ENGINE_OIL_TEMP, "Engine Oil", "%3d", "C", INT_MINUS40, 0, 150));

  addDataObject(false, new OBDData(PID_BAROMETRIC_PRESSURE, "Air", "%4d", "mBar", BYTE_TIMES10, 950, 1150));
  addDataObject(false, new OBDData(PID_FUEL_PRESSURE, "Fuel rail", "%4d", "kPa", BYTE_TIMES3, 0, 765));

  addDataObject(false, new OBDData(PID_FUEL_LEVEL, "Tank Level", "%3d", "%", BYTE_PERCENTAGE, 0, 100));
  addDataObject(false, new OBDData(PID_ENGINE_FUEL_RATE, "Fuel Rate", "%3.1f", "L/hr", WORD_DIV20, 0, 30)); // L/h

  //  addDataObject(false, new OBDFloatValue(PID_CONTROL_MODULE_VOLTAGE, "Battery Voltage", WORD_DIV1000 // V
  //  addDataObject(false, new OBDByteValue(PID_CALCULATED_ENGINE_LOAD, "Engine Load", BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(false, new OBDByteValue(PID_ABSOLUTE_ENGINE_LOAD, "Abs Engine Load", BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(false, new OBDData(PID_ENGINE_TORQUE_DEMANDED, "Torque Demanded", INT_MINUS125 // %
  //  addDataObject(false, new OBDData(PID_ENGINE_TORQUE_PERCENTAGE, "Torque Percentage", INT_MINUS125 // %
  //  addDataObject(false, new OBDWordValue(PID_EVAP_SYS_VAPOR_PRESSURE, SHIFT_RIGHT_2 // kPa
  //  addDataObject(false, new OBDByteValue(PID_THROTTLE_POSITION, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(false, new OBDByteValue(PID_COMMANDED_EGR, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(false, new OBDByteValue(PID_COMMANDED_EVAPORATIVE_PURGE, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(false, new OBDByteValue(PID_RELATIVE_THROTTLE_POS, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(false, new OBDByteValue(PID_ABSOLUTE_THROTTLE_POS_B, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(false, new OBDByteValue(PID_ABSOLUTE_THROTTLE_POS_C, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(false, new OBDByteValue(PID_ACC_PEDAL_POS_D, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(false, new OBDByteValue(PID_ACC_PEDAL_POS_E, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(false, new OBDByteValue(PID_ACC_PEDAL_POS_F, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(false, new OBDByteValue(PID_COMMANDED_THROTTLE_ACTUATOR, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(false, new OBDByteValue(PID_ETHANOL_FUEL, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(false, new OBDByteValue(PID_HYBRID_BATT_REMAINING_LIFE, BYTE_PERCENTAGE (word * 100 / 255)
  //  addDataObject(false, new OBDWordValue(PID_MAF_AIR_FLOW_RATE, WORD_DIV100 // grams/sec
  //  addDataObject(false, new OBDData(PID_TIMING_ADVANCE, BYTE_DIV2_MINUS64
  //  addDataObject(false, new OBDWordValue(PID_DISTANCE_SINCE_DTC_CLEARED, WORD_NO_CONVERSION: // km
  //  addDataObject(false, new OBDWordValue(PID_DISTANCE_WITH_MIL_ON, WORD_NO_CONVERSION: // km
  //  addDataObject(false, new OBDWordValue(PID_RUN_TIME_WITH_MIL_ON, WORD_NO_CONVERSION: // minute
  //  addDataObject(false, new OBDWordValue(PID_TIME_SINCE_DTC_CLEARED, WORD_NO_CONVERSION: // minute
  //  addDataObject(false, new OBDWordValue(PID_RUNTIME_SINCE_ENG_START, WORD_NO_CONVERSION: // second
  //  addDataObject(false, new OBDWordValue(PID_FUEL_RAIL_PRESSURE, WORD_NO_CONVERSION: // kPa
  //  addDataObject(false, new OBDWordValue(PID_ENGINE_REF_TORQUE, WORD_NO_CONVERSION: // Nm
  //  addDataObject(false, new OBDData(PID_SHORT_TERM_FUEL_TRIM_1, INT_MINUS128_TIMES100_DIV128
  //  addDataObject(false, new OBDData(PID_LONG_TERM_FUEL_TRIM_1, INT_MINUS128_TIMES100_DIV128
  //  addDataObject(false, new OBDData(PID_SHORT_TERM_FUEL_TRIM_2, INT_MINUS128_TIMES100_DIV128
  //  addDataObject(false, new OBDData(PID_LONG_TERM_FUEL_TRIM_2, INT_MINUS128_TIMES100_DIV128
  //  addDataObject(false, new OBDData(PID_EGR_ERROR, INT_MINUS128_TIMES100_DIV128
  //  addDataObject(false, new OBDData(PID_FUEL_INJECTION_TIMING, LONG_MINUS26880_DIV128
  //  addDataObject(false, new OBDData(PID_CATALYST_TEMP_B1S1, LONG_DIV10_MINUS40
  //  addDataObject(false, new OBDData(PID_CATALYST_TEMP_B2S1, LONG_DIV10_MINUS40
  //  addDataObject(false, new OBDData(PID_CATALYST_TEMP_B1S2, LONG_DIV10_MINUS40
  //  addDataObject(false, new OBDData(PID_CATALYST_TEMP_B2S2, LONG_DIV10_MINUS40
  //  addDataObject(false, new OBDData(PID_AIR_FUEL_EQUIV_RATIO, LONG_TIMES200_DIV65536: // 0~200

#ifdef PID_DISCOVERY_MODE
  for (word i = 0; i < MAX_UNKNOWN_PIDS; i++)
    m_unknownPIDS[i] = 0;
#endif
}

void PumaOBD::addDataObject(bool highPrio, OBDData *object)
{
  if (m_firstData[highPrio] == 0) {
    m_firstData[highPrio] = object;
  } else {
    m_lastData[highPrio]->m_next = object;
  }
  m_lastData[highPrio] = object;
  m_curData[highPrio] = object;
}

OBDData *PumaOBD::dataObject(uint8_t PID)
{
  for (byte prio = 1; prio >= 0; prio--) {
    OBDData *tmp = m_firstData[prio];
    while (tmp) {
      if (tmp->pid() == PID)
        return tmp;
      tmp = tmp->m_next;
    }
  }
  return &m_invalidPID;
}

// DANGER: This function can return a NULL pointer!
OBDData *PumaOBD::nextDataObject(bool highPrio)
{
  OBDData *tmp = m_curData[highPrio];
  if (tmp) {
    tmp = tmp->m_next;
    if (tmp == 0)
      tmp = m_firstData[highPrio];
    m_curData[highPrio] = tmp;
    return tmp;
  }

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
  pinMode(PIN_MP2515_RX_INTERRUPT, INPUT);

#ifdef PID_DISCOVERY_MODE
  Serial.println("WARNING: Running in PID Discovery mode!");
#endif

  m_CAN.begin(PIN_MEGA_SPI_CS, PumaCAN::MCP_STD, PumaCAN::CAN_500KBPS, PumaCAN::MCP_16MHZ);
  m_CAN.setMode(LOOPBACK_OR_NORMAL);

  // TODO: Add to PID_DISCOVERY_MODE?
  m_CAN.setMask(PumaCAN::MASK0, false, 0x07FF0000);
  m_CAN.setMask(PumaCAN::MASK1, false, 0x07FF0000);
  m_CAN.setFilter(PumaCAN::FILT0, false, 0x07E80000);
  m_CAN.setFilter(PumaCAN::FILT2, false, 0x07E80000);
}

void PumaOBD::requestObdUpdates()
{
  // Now check if there is any data that needs an update.
  // We only ask for MAX 2 elements at a time, because the MCP2515 can only handle two sets of data.
  // To ensure that every data element gets a chance we rotate through the list in a round robin fashion

  // First we take an element from the high priority list, which should only contain a few elements, and hence these will be updated with a higher frequency
  OBDData *tmp = nextDataObject(true);
  if (tmp) requestSensorData(tmp);

  // Then take one element from the low priority list
  tmp = nextDataObject(false);
  if (tmp) requestSensorData(tmp);
}

void PumaOBD::requestSensorData(OBDData * sensor)
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

    // We send the simulated data onto the CAN bus. Since it is in loopback mode, the data will echo back to the RX and processed as if it was send by someone else.
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

void PumaOBD::readRxBuffers()
{
  while (m_CAN.available() && (m_rxFIFO_count < MAX_RX_FIFO)) { // One or more messages available, and FIFO buffer not full?
    m_rxFIFO[m_rxFIFO_write] = m_CAN.read();
    m_rxFIFO[m_rxFIFO_write].m_timeStamp = millis();
    m_rxFIFO_write++;
    m_rxFIFO_count++;
    if (m_rxFIFO_write >= MAX_RX_FIFO)
      m_rxFIFO_write = 0;
  }
}

void PumaOBD::readMessages()
{
  // Process messages in the RX FIFO buffer
  while (m_rxFIFO_read != m_rxFIFO_write) {

    if (m_rxFIFO_count > 10) {
      // TODO: Add this as a warning on the display
      Serial.println("WARNING: RX FIFO is getting full: " + String(m_rxFIFO_count));
    }

#ifdef RAW_LOGGING
    logRawData(&m_rxFIFO[m_rxFIFO_read]);
#endif

    if (m_rxFIFO[m_rxFIFO_read].m_id == 0x7E8) {
      processMessage(m_rxFIFO[m_rxFIFO_read]);
    } else {
#ifdef PID_DISCOVERY_MODE
      addUnhandledPID(m_rxFIFO[m_rxFIFO_read].m_id);
#endif
    }

    if (m_rxFIFO_count > 0) m_rxFIFO_count--;
    m_rxFIFO_read++;
    if (m_rxFIFO_read >= MAX_RX_FIFO) m_rxFIFO_read = 0;
  }
}

void PumaOBD::processMessage(CAN_Frame message)
{
  uint16_t pid = message.m_id;
  uint8_t *data = &message.m_data[0];

  if (pid == PID_REPLY) {
    pid = message.m_data[2];
    data = &message.m_data[3];

    OBDData *object = dataObject(pid);
    if (object) {
      object->setValue(message.m_timeStamp, data);
      logObdData(v2s("%6d", message.m_timeStamp) +
                 v2s(" %04X ", pid) +
                 object->toString());
    }
  }
}

#ifdef PID_DISCOVERY_MODE
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

#ifdef PID_DISCOVERY_MODE
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
  m_timeStamp = 0;
  m_label = "";
  m_subLabel = "";
  m_conversion = BYTE_NO_CONVERSION;
  m_next = 0;
}

OBDData::~OBDData()
{
}

OBDData::OBDData(uint8_t pid, String label, String format, String subLabel, OBD_DATA_CONVERSION conversion, long min, long max)
{
  m_pid = pid;
  m_label = label;
  m_format = format;
  m_subLabel = subLabel;
  m_conversion = conversion;
  m_timeStamp = 0;
  m_next = 0;
  m_value = 0;
  m_minValue = min;
  m_maxValue = max;
#ifdef LOOPBACK_MODE
  m_simValue = 0;
  m_simIncrease = true;
#else
  min = max;
#endif
}

uint8_t OBDData::pid()
{
  return m_pid;
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

OBD_DATA_CONVERSION OBDData::dataConversion()
{
  return m_conversion;
}

// This returns the expected string length returned by toString, if the specified format could be applied correctly.
byte OBDData::stringLength()
{
  byte decimals = 0;
  byte fraction = 0;
  byte dot_pos = 0;
  if (m_format.length() < 2 || m_format[0] != '%')
    return 3;

  char l = m_format[1];
  dot_pos = 2;

  if (l == '0' && m_format.length() > 2) {
    l = m_format[2];
    dot_pos = 3;
  }

  switch (l) {
    case '1': decimals = 1; break;
    case '2': decimals = 2; break;
    case '3': decimals = 3; break;
    case '4': decimals = 4; break;
    case '5': decimals = 5; break;
    case '6': decimals = 6; break;
    case '7': decimals = 7; break;
    case '8': decimals = 8; break;
    case '9': decimals = 9; break;
    default: decimals = 3; break;
  }

  if (m_format.length() > 4 && m_format[dot_pos] == '.') {
    l = m_format[dot_pos + 1];

    switch (l) {
      case '1': fraction = 2; break;
      case '2': fraction = 3; break;
      case '3': fraction = 4; break;
      case '4': fraction = 5; break;
      case '5': fraction = 6; break;
      case '6': fraction = 7; break;
      case '7': fraction = 8; break;
      case '8': fraction = 9; break;
      case '9': fraction = 10; break;
      default: fraction = 1; break;
    }
  }

  return decimals + fraction;
}

String OBDData::toString()
{
  char buf[30];
  if (m_conversion == WORD_DIV20) {
    float tmp = m_value;
    tmp /= 20;
    sprintf(buf, m_format.c_str(), tmp);
  } else if (m_conversion == ULONG_NO_CONVERSION) {
    unsigned long tmp = m_value;
    sprintf(buf, "0X%08X", tmp);
  } else {
    sprintf(buf, m_format.c_str(), m_value);
  }

  return buf;
}

byte OBDData::toByte()
{
  byte b = m_value & 0xFF;
  return b;
}

word OBDData::toWord()
{
  word w = m_value & 0xFFFF;
  return w;
}

int OBDData::toInt()
{
  int i = m_value & 0xFFFF;
  return i;
}

long OBDData::toLong()
{
  return m_value;
}

#ifdef SELF_TEST
void OBDData::setValue(long value)
{
  m_value = value;
}
#endif

void OBDData::setValue(uint32_t timeStamp, uint8_t *data)
{
  m_timeStamp = timeStamp;

  long old_value = m_value;
  m_value = *data;

  byte repeat = 0;
  if (m_conversion == WORD_NO_CONVERSION ||
      m_conversion == WORD_DIV4 ||
      m_conversion == WORD_DIV20) {
    repeat = 1;
  } else if (m_conversion == ULONG_NO_CONVERSION) {
    repeat = 3;
  } else if (m_conversion == INT_NO_CONVERSION) {
    m_value = int8_t(*data);
  }

  for (byte r = 0; r < repeat; r++) {
    m_value = m_value << 8;
    data++;
    m_value += *data;
  }

  switch (m_conversion) {
    case INT_NO_CONVERSION:
      break;
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
    case ULONG_NO_CONVERSION:
      break;
  }

  if (old_value != m_value)
    Display()->updateSensor(this);
}

void OBDData::setFormat(String format)
{
  m_format = format;
}

#ifdef LOOPBACK_MODE
void OBDData::simulateData(CAN_Frame * message)
{
  if (m_simIncrease) {
    m_simValue += PUMA_SIM_STEP_VALUE;
    if (m_simValue >= m_maxValue) m_simIncrease = false;
  } else {
    m_simValue -= PUMA_SIM_STEP_VALUE;
    if (m_simValue <= m_minValue) m_simIncrease = true;
  }

  // Special case: 'message == 0', which is used to simulate data for non OBD originating data, i.e. heading, roll, pitch, tpms, etc.
  CAN_Frame _tmp_message;
  if (message == 0) {
    message = &_tmp_message;
    message->m_data[1] = 0x41;
    message->m_data[2] = m_pid;
  }

  long tmp = 0;
  switch (m_conversion) {
    case INT_NO_CONVERSION:
      message->m_data[0] = 3;
      message->m_data[3] = int8_t(m_simValue);
      break;
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
      message->m_data[3] = uint8_t(m_simValue * 255 / 100);
      break;
    case BYTE_NO_CONVERSION:
      message->m_data[0] = 3;
      message->m_data[3] = uint8_t(m_simValue);
      break;
    case WORD_NO_CONVERSION:
      message->m_data[0] = 4;
      message->m_data[3] = uint8_t(m_simValue >> 8 & 0xFF);
      message->m_data[4] = uint8_t(m_simValue & 0xFF);
      break;
    case WORD_DIV4:
      message->m_data[0] = 4;
      tmp = m_simValue * 4;
      message->m_data[3] = uint8_t(tmp >> 8);
      message->m_data[4] = uint8_t(tmp & 0xFF);
      break;
    case WORD_DIV20:
      message->m_data[0] = 4;
      tmp = m_simValue * 20;
      message->m_data[3] = uint8_t(tmp >> 8);
      message->m_data[4] = uint8_t(tmp & 0xFF);
      break;
    case ULONG_NO_CONVERSION:
      message->m_data[0] = 6;
      message->m_data[3] = uint8_t(m_simValue >> 24 & 0xFF);
      message->m_data[4] = uint8_t(m_simValue >> 16 & 0xFF);
      message->m_data[5] = uint8_t(m_simValue >> 8 & 0xFF);
      message->m_data[6] = uint8_t(m_simValue & 0xFF);
      break;
  }

  // In case the sensor data is not coming from the CAN bus, we set the value directly, which then will cause the display to be updated.
  if (message == &_tmp_message) {
    setValue(millis(), &message->m_data[3]);
  }
}
#endif





