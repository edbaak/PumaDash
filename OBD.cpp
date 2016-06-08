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
#include "Display.h"
#include "OBD.h"
#include "CAN.h"
#include "Speed.h"


#ifdef LOOPBACK_MODE
#define LOOPBACK_OR_NORMAL PumaCAN::MCP_LOOPBACK
#else
#define LOOPBACK_OR_NORMAL PumaCAN::MCP_NORMAL
#endif

#ifdef OBD_DEBUG
#define OBD_PRINTLN(s) Serial.println(s); Serial.flush();
#define OBD_PRINT(s) Serial.print(s); Serial.flush();
#else
#define OBD_PRINTLN(s) {}
#define OBD_PRINT(s) {}
#endif


PumaOBD::PumaOBD(SpeedControl *speedControl)
{
  m_firstData = 0;
  m_lastData = 0;
  m_curData = 0;

  m_rxFIFO_write = 0;
  m_rxFIFO_read = 0;
  m_rxFIFO_count = 0;

  m_speedControl = speedControl;

  OBDColorRange *_speedRange = new OBDColorRange(LESS, 100, PUMA_NORMAL_COLOR);
  OBDColorRange *_engineTempRange = new OBDColorRange(LESS, 60, PUMA_WARNING_COLOR, new OBDColorRange(LESS, 105, PUMA_NORMAL_COLOR));
  OBDColorRange *_torquePowerRange = new OBDColorRange(LESS, 10, PUMA_WARNING_COLOR, new OBDColorRange(LESS, 80, PUMA_NORMAL_COLOR));
  OBDColorRange *_rpmRange = new OBDColorRange(LESS, 1200, PUMA_WARNING_COLOR, new OBDColorRange(LESS, 3500, PUMA_NORMAL_COLOR));
  OBDColorRange *_tankRange = new OBDColorRange(LESS, 25, RED, new OBDColorRange(LESS, 50, PUMA_WARNING_COLOR, new OBDColorRange(MORE, 0, PUMA_NORMAL_COLOR)));
  OBDColorRange *_airPressureRange = new OBDColorRange(LESS, 970, PUMA_WARNING_COLOR, new OBDColorRange(LESS, 1010, PUMA_NORMAL_COLOR));
  OBDColorRange *_airTemperatureRange = new OBDColorRange(LESS, 35, PUMA_NORMAL_COLOR, new OBDColorRange(LESS, 45, PUMA_WARNING_COLOR));

  // Speed and Rpm are handled separately since I want to update them at a higher frequency
  // All other sensor data is thrown in a linked list and updates happen in a round robin
  m_rpm = new OBDData(PID_RPM, "", "Rpm", WORD_DIV4, 4, OBD_D, 0, 4500, _rpmRange); // 0x0C
  m_speed = new OBDData(PID_SPEED, "", "Km/h", BYTE_NO_CONVERSION, 3, OBD_D, 0, 115, _speedRange); // 0x0D

  // Center Screen -> Temperatures
  addDataObject(new OBDData(PID_COOLANT_TEMP, "Coolant", "C", INT_MINUS40, 3, OBD_D, -0, 98, _engineTempRange)); // 0x05
  //addDataObject(new OBDData(PID_INTAKE_AIR_TEMP, "Intake Air", "C", INT_MINUS40, 3, OBD_D, 10, 40, _airTemperatureRange)); // 0x0F
  addDataObject(new OBDData(PID_AMBIENT_AIR_TEMP, "Ambient Air", "C", INT_MINUS40, 3, OBD_D, 10, 40, _airTemperatureRange)); //0x46
  addDataObject(new OBDData(PID_ENGINE_OIL_TEMP, "Engine Oil", "C", INT_MINUS40, 3, OBD_D, 5, 100, _engineTempRange)); // 0x5C

  // Center Screen -> Pressure
  addDataObject(new OBDData(PID_BAROMETRIC_PRESSURE, "Air", "mBar", BYTE_TIMES10, 4, OBD_D, 950, 1150, _airPressureRange)); // 0x33
  addDataObject(new OBDData(PID_FUEL_RAIL_GAUGE_PRESSURE, "Fuel Rail", "MPa", WORD_DIV100, 4, OBD_D, 0, 5000, 0)); // 0x23
  addDataObject(new OBDData(PID_INTAKE_MANIFOLD_PRESSURE, "Intake Manifold", "kPa", BYTE_NO_CONVERSION, 3, OBD_D, 0, 255, 0)); // 0x0B

  // Center Screen -> Engine & Drivetrain
  addDataObject(new OBDData(PID_CALCULATED_ENGINE_LOAD, "Engine Load", "%", BYTE_PERCENTAGE, 3, OBD_D, 0, 100, _torquePowerRange)); // 0x04
  addDataObject(new OBDData(PID_MAF_AIR_FLOW_RATE, "MAF Air Flow", "grams/sec", WORD_DIV100, 3, OBD_F1, 0, 655, 0));  // 0x10

  // Center Screen -> Fuel
  addDataObject(new OBDData(PID_FUEL_LEVEL, "Tank Level", "%", BYTE_PERCENTAGE, 3, OBD_D, 10, 25, _tankRange)); // 0x2F

  // Center Screen -> Distance

  // Right Screen -> Speed

  // Right Screen -> Diagnostics
  // Start capturing the MIL data only when we have a MIL
  addDataObject(new OBDData(PID_DISTANCE_WITH_MIL_ON, "Dist since MIL", "Km", WORD_NO_CONVERSION, 5, OBD_D, 0, 65535, 0)); // 0x21
  addDataObject(new OBDData(PID_RUN_TIME_WITH_MIL_ON, "Time since MIL", "Min", WORD_NO_CONVERSION, 5, OBD_D, 0, 65535, 0)); // 0x4D

  // Start capturing the DTC cleared counters only when we actually have cleared the DTC codes
  // PID_MON_STATUS_SINCE_DTC_CLEARED    // 0x01 --- Bit Encoded
  addDataObject(new OBDData(PID_WARMS_UPS_SINCE_DTC_CLEARED, "Warm ups", "", BYTE_NO_CONVERSION, 3, OBD_D, 0, 255, 0)); // 0x30
  addDataObject(new OBDData(PID_DISTANCE_SINCE_DTC_CLEARED, "Dist since DTC", "Km", WORD_NO_CONVERSION, 5, OBD_D, 0, 65535, 0)); //0x31
  addDataObject(new OBDData(PID_TIME_SINCE_DTC_CLEARED, "Time since DTC", "Min", WORD_NO_CONVERSION, 5, OBD_D, 0, 65535, 0)); // 0x4E

  // PID_MONITOR_STATUS_THIS_DRIVE   0x41 -- Bit encoded
  //addDataObject(new OBDFloatValue(PID_CONTROL_MODULE_VOLTAGE, "Battery Voltage", "V", WORD_DIV1000, 2, OBD_F1, 0, 65.535)); // 0x42

  // -------- PID's that are supported by the PUMA, but that I'm not using ----------

  // I've tried PID_THROTTLE_POSITION and it seems fixed to 93% for me. So quite useless.
  // For the purposes of a cruise control I'd also tend towards using a A/D conversion and directly measure the
  // pedal output rather than relying on the ECU.
  // PID_THROTTLE_POSITION           0x11
  // PID_ACC_PEDAL_POS_D             0x49
  // PID_ACC_PEDAL_POS_E             0x4A
  // PID_COMMANDED_THROTTLE_ACTUATOR 0x4C

  // PID_COMMANDED_EGR               0x2C
  // PID_EGR_ERROR                   0x2D
  // PID_COMMANDED_EGR_AND_EGR_ERR   0x69
  // PID_CATALYST_TEMP_B1S1          0x3C
  // PID_CATALYST_TEMP_B1S2          0x3E
  // PID_RUNTIME_SINCE_ENG_START     0x1F
  // PID_O2_SENSORS_PRESENT_2_BANKS  0x13
  // PID_OBD_STANDARD                0x1C
  // PID_AUX_INPUT_STATUS            0x1E
  // PID_OXYGEN_SENSOR_1B            0x24
  // PID_MAX_VALUE_FUEL_AIR_EQ       0x4F

  // ---------------------------------------------------------------------------------

#ifdef PID_DISCOVERY_MODE
  for (word i = 0; i < MAX_UNKNOWN_PIDS; i++)
    m_unknownPIDS[i] = 0;
#endif
}

void PumaOBD::addDataObject(OBDData *object)
{
  if (object == 0) return;

  if (m_firstData == 0) {
    m_firstData = object;
  } else {
    m_lastData->m_nextDataObject = object;
  }
  m_lastData = object;
  if (m_curData == 0) m_curData = object;
}

OBDData *PumaOBD::dataObject(uint8_t PID)
{
  if (PID == PID_RPM) return m_rpm;
  if (PID == PID_SPEED) return m_speed;

  OBDData *tmp = m_firstData;
  while (tmp) {
    if (tmp->pid() == PID)
      return tmp;
    tmp = (OBDData*)tmp->m_nextDataObject;
  }

  return &m_invalidPID;
}

// DANGER: This function can return a NULL pointer!
OBDData *PumaOBD::nextDataObject()
{
  OBDBaseData *tmp = m_curData;
  if (tmp) {
    tmp = tmp->m_nextDataObject;
    if (tmp == 0)
      tmp = m_firstData;
    m_curData = (OBDData*)tmp;
    return m_curData;
  }

  return 0;
}

void PumaOBD::setup()
{
  m_rpm_timer.start();
  m_speed_timer.start();
  m_slow_timer.start();

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

  m_CAN.begin(PIN_MEGA_SPI_CS, PumaCAN::MCP_STD, PumaCAN::CAN_500KBPS, PumaCAN::MCP_16MHZ);
  m_CAN.setMode(LOOPBACK_OR_NORMAL);

#ifdef PID_DISCOVERY_MODE
  OBD_PRINTLN("WARNING: Running in PID Discovery mode!");
#else
  m_CAN.setMask(PumaCAN::MASK0, false, 0x07FF0000);
  m_CAN.setMask(PumaCAN::MASK1, false, 0x07FF0000);
  m_CAN.setFilter(PumaCAN::FILT0, false, 0x07E80000);
  m_CAN.setFilter(PumaCAN::FILT2, false, 0x07E80000);
#endif
}

void PumaOBD::requestObdUpdates()
{
  OBD_PRINTLN(">> requestObdUpdates");

  // We only ask for MAX 2 elements at a time, because the MCP2515 can only handle two sets of data.
  // To ensure that every data element gets a chance we rotate through the list in a round robin fashion
  // Speed and Rpm are handled separately, with a higher frequency

  if (m_speed_timer.elapsed() > 333) { // Update SPEED 3x per second
    m_speed_timer.start();
    requestSensorData(m_speed);
  } else if (m_rpm_timer.elapsed() > 200) { // Update RPM's 5x per second
    m_rpm_timer.start();
    requestSensorData(m_rpm);
  }

  if (m_slow_timer.elapsed() > 500) { // Update *one* sensor every 500 ms, i.e. if we have 10 sensors then every sensor gets an update every 5 seconds
    m_slow_timer.start();
    // Query one element from the low priority list
    requestSensorData(nextDataObject());
  }

  OBD_PRINTLN("<< requestObdUpdates");
}

void PumaOBD::requestSupportedPIDRange(byte pidRange)
{
  CAN_Frame message;
  message.m_length = 8;     // eight data bytes follow (but not all have to be valid)
  message.m_id = PID_REQUEST;
  message.m_data[0] = 0x02; // two valid bytes with data following
  message.m_data[1] = 0x01; // mode 1 = show current data, mode 2 = show freeze frame
  message.m_data[2] = pidRange;  // PID_SUPPORTED_PID_01_20, PID_SUPPORTED_PID_21_40, PID_SUPPORTED_PID_41_60, etc.
  m_CAN.write(message);
}

void PumaOBD::requestSensorData(OBDData * sensor)
{
  if (sensor) {
    OBD_PRINTLN(">>>> requestSensorData")
    CAN_Frame message;
    message.m_length = 8;     // eight data bytes follow (but not all have to be valid)

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
    OBD_PRINTLN("<<<< requestSensorData");
  }
}

void PumaOBD::readRxBuffers()
{
  OBD_PRINTLN(">> readRXBuffers");
  while (m_CAN.available() && (m_rxFIFO_count < MAX_RX_FIFO)) { // One or more messages available, and FIFO buffer not full?
    // LED2 indicates CAN bus RX Interrupt activity. A flashing led is 'good'. It means we have incoming data, and are not stuck in a dead-lock somewhere.
    // The Led should be on for only a very short period, so visually this will be a fast flashing led with a low light intensity.
    digitalWrite(PIN_CAN_BOARD_LED2, HIGH);
    m_rxFIFO[m_rxFIFO_write] = m_CAN.read();
    m_rxFIFO[m_rxFIFO_write].m_timeStamp = millis();
    m_rxFIFO_write++;
    m_rxFIFO_count++;
    if (m_rxFIFO_write >= MAX_RX_FIFO)
      m_rxFIFO_write = 0;
  }
  digitalWrite(PIN_CAN_BOARD_LED2, LOW);
  OBD_PRINTLN("<< readRxBuffers");
}

void PumaOBD::update()
{
  OBD_PRINTLN(">> update");

  byte safety_fuse = 0;
  // Process messages in the RX FIFO buffer
  while (m_rxFIFO_read != m_rxFIFO_write) {
    safety_fuse++;
    if (safety_fuse > 20) {
      Serial.println(">>>>>>>>>>>> SAFETY FUSE BLOWN <<<<<<<<<<<<<<");
      return;
    }

    if (m_rxFIFO_count > 6) {
      // TODO: Add this as a warning on the display
      Serial.println("WARNING: RX FIFO is getting full: " + String(m_rxFIFO_count));
    }

    logRawData(&m_rxFIFO[m_rxFIFO_read]);

    if (m_rxFIFO[m_rxFIFO_read].m_id == 0x7E8) {
      processMessage(&m_rxFIFO[m_rxFIFO_read]);
    } else {
#ifdef PID_DISCOVERY_MODE
      addUnhandledPID(m_rxFIFO[m_rxFIFO_read].m_id);
#endif
    }

    if (m_rxFIFO_count > 0) m_rxFIFO_count--;
    m_rxFIFO_read++;
    if (m_rxFIFO_read >= MAX_RX_FIFO) m_rxFIFO_read = 0;
  }
  OBD_PRINTLN("<< update");
}

void PumaOBD::processMessage(CAN_Frame *message)
{
  if (message == 0) return;

  if (message->m_id == PID_REPLY) {
    uint16_t pid = message->m_data[2];
    uint8_t *data = &message->m_data[3];

#ifdef OBD_DEBUG
    Serial.println(pid, HEX);
    Serial.flush();
#endif
    OBDData *object = dataObject(pid);
    if (object != 0 && message != 0) {
      object->setValue(message->m_timeStamp, data);
      String logString = v2s("%6d", message->m_timeStamp) +
                         v2s(" %04X ", pid) +
                         object->toString();
      if (CONFIG()->logFileName() != "") {
        PumaFile log;
        log.appendLogData(CONFIG()->logFileName() + ".PDO",
                          logString);
      }
#ifdef OBD_VERBOSE_DEBUG
      Serial.println(logString);
#endif
      if (pid == PID_SPEED)
        m_speedControl->updateSpeed(m_speed->toWord());
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
//                                      OBDBaseData
//*************************************************************************************

OBDBaseData::OBDBaseData()
{
  m_nextDataObject = 0;
  m_pid = 0;
  m_timeStamp = 0;
  m_label = "";
  m_subLabel = "";
  m_conversion = BYTE_NO_CONVERSION;
}

OBDBaseData::~OBDBaseData()
{
}

OBDBaseData::OBDBaseData(uint8_t pid, String label, String subLabel, OBD_DATA_CONVERSION conversion, byte stringWidth, OBD_PRECISION stringPrecision, OBDColorRange *colorRange)
{
  m_nextDataObject = 0;
  m_pid = pid;
  m_label = label;
  m_stringWidth = stringWidth;
  m_stringPrecision = stringPrecision;
  m_subLabel = subLabel;
  m_conversion = conversion;
  m_timeStamp = 0;
  m_colorRange = colorRange;
}

uint8_t OBDBaseData::pid()
{
  return m_pid;
}

String OBDBaseData::label()
{
  return m_label;
}

String OBDBaseData::subLabel()
{
  return m_subLabel;
}

OBD_DATA_CONVERSION OBDBaseData::dataConversion()
{
  return m_conversion;
}

void OBDBaseData::setDataConversion(OBD_DATA_CONVERSION conversion)
{
  m_conversion = conversion;
}

void OBDBaseData::setFormat(byte width, OBD_PRECISION stringPrecision)
{
  m_stringWidth = width;
  m_stringPrecision = stringPrecision;
}

// This returns the expected string length returned by toString, if the specified format could be applied correctly.
byte OBDBaseData::stringLength()
{
  byte ret = m_stringWidth;
  if (m_stringPrecision < OBD_D)
    ret += (int)m_stringPrecision + 1; // one extra for the 'dot'
  return ret;
}

String OBDBaseData::toString()
{
  return "";
}

byte OBDBaseData::toByte()
{
  return 0;
}

word OBDBaseData::toWord()
{
  return 0;
}

int OBDBaseData::toInt()
{
  return 0;
}

long OBDBaseData::toLong()
{
  return 0;
}

word OBDBaseData::color()
{
  return PUMA_ALARM_COLOR;
}

//*************************************************************************************
//                                      OBDData
//*************************************************************************************

OBDData::OBDData() : OBDBaseData()
{
}

OBDData::~OBDData()
{
}

OBDData::OBDData(uint8_t pid, String label, String subLabel, OBD_DATA_CONVERSION conversion, byte stringWidth, OBD_PRECISION stringPrecision, long min, long max, OBDColorRange *colorRange) :
  OBDBaseData(pid, label, subLabel, conversion, stringWidth, stringPrecision, colorRange)
{
  m_value = -200; // set an arbitrary negative value so that we update the display even if the initial value is '0'.
  m_minValue = min;
  m_maxValue = max;
#ifdef LOOPBACK_MODE
  m_simValue = min - (min * 0.1);
  m_simIncrease = true;
#endif
}

word OBDData::color()
{
  if (m_colorRange != 0)
    return m_colorRange->color(m_value);
  return PUMA_NORMAL_COLOR;
}

String OBDData::toString()
{
  String ret;
  byte len = stringLength();

  if (m_stringPrecision < OBD_D) {
    float f = 0.0;
    if (m_conversion == WORD_DIV20) {
      f = m_value;
      f /= 20;
    } else if (m_conversion == WORD_DIV100) {
      f = m_value;
      f /= 100;
    } else if (m_conversion == WORD_DIV1000) {
      f = m_value;
      f /= 1000;
    } else {
      f = m_value;
    }

    ret = String(f, int(m_stringPrecision));
  } else if (m_stringPrecision == OBD_H) {
    ret = String(m_value, HEX);
    while (ret.length() < len)
      ret = "0" + ret;
    ret = "0X" + ret;
    ret.toUpperCase();
  } else {
    ret = String(m_value);
  }

  while (ret.length() < len)
    ret = " " + ret;
  return ret;
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
  OBD_PRINTLN(">>>>>> entering setValue");

  m_timeStamp = timeStamp;

  long old_value = m_value;
  m_value = *data;

  byte repeat = 0;
  if (m_conversion == WORD_NO_CONVERSION ||
      m_conversion == WORD_DIV4 ||
      m_conversion == WORD_DIV20 ||
      m_conversion == WORD_DIV100 ||
      m_conversion == WORD_DIV1000 ||
      m_conversion == WORD_TIMES10) {
    repeat = 1;
  } else if (m_conversion == ULONG_NO_CONVERSION) {
    repeat = 3;
  } else if (m_conversion == INT_NO_CONVERSION ||
             m_conversion == INT_TIMES100_DIV128_MINUS100) {
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
    case INT_TIMES100_DIV128_MINUS100:
      m_value = ((m_value * 100) / 128) - 100;
      break;
    case BYTE_TIMES10:
      m_value *= 10;
      break;
    case BYTE_TIMES3:
      m_value *= 3;
      break;
    case BYTE_PERCENTAGE:
      m_value = (m_value * 100) / 255;
      break;
    case BYTE_NO_CONVERSION:
      break;
    case WORD_NO_CONVERSION:
      break;
    case WORD_TIMES10:
      m_value *= 10;
      break;
    case WORD_DIV4:
      m_value /= 4;
      break;
    case WORD_DIV20:
    case WORD_DIV100:
    case WORD_DIV1000:
      // Do the conversion later when converting to a string
      break;
    case ULONG_NO_CONVERSION:
      break;
  }

  if (old_value != m_value) {
    OBD_PRINTLN("------- calling updateSensorWidget()");
    Display()->updateSensorWidget(this);
  }
  OBD_PRINTLN("<<<<<< setValue");
}

#ifdef LOOPBACK_MODE
void OBDData::simulateData(CAN_Frame *message)
{
  long step;
  if (m_maxValue > m_minValue)
    step = (m_maxValue - m_minValue) / 20;
  else
    step = (m_minValue = m_maxValue) / 20;
  if (step == 0)
    step = 2;

  if (m_simIncrease) {
    m_simValue += step;
    if (m_simValue >= m_maxValue + (m_maxValue * 0.4)) m_simIncrease = false;
  } else {
    m_simValue -= step;
    if (m_simValue <= m_minValue - (m_minValue * 0.3)) m_simIncrease = true;
  }

  // Special case: 'message == 0', which is used to simulate data for non OBD originating data, i.e. heading, roll, pitch, tpms, etc.
  CAN_Frame _tmp_message;
  if (message == 0) {
    message = &_tmp_message;
    message->m_data[1] = 0x41;
    message->m_data[2] = m_pid;
  }
  message->m_length = 8;
  message->m_rtr = 0;
  message->m_extended = 0;

  long tmp = 0;
  switch (m_conversion) {
    case INT_NO_CONVERSION:
      message->m_data[0] = 3;
      message->m_data[3] = int8_t(m_simValue);
      break;
    case INT_TIMES100_DIV128_MINUS100:
      message->m_data[0] = 3;
      message->m_data[3] = uint8_t(((m_simValue + 100) * 128) / 100);
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
    case WORD_TIMES10:
      message->m_data[0] = 4;
      tmp = m_simValue / 10;
      message->m_data[3] = uint8_t(tmp >> 8);
      message->m_data[4] = uint8_t(tmp & 0xFF);
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
    case WORD_DIV100:
      message->m_data[0] = 4;
      tmp = m_simValue * 100;
      message->m_data[3] = uint8_t(tmp >> 8);
      message->m_data[4] = uint8_t(tmp & 0xFF);
      break;
    case WORD_DIV1000:
      message->m_data[0] = 4;
      tmp = m_simValue * 1000;
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

//********************************************************************************
//                              OBDColorRange
//********************************************************************************

OBDColorRange::OBDColorRange(OBDCOLOR_RANGE range, long threshold, word color, void *next)
{
  m_lessThan = (range == LESS);
  m_threshold = threshold;
  m_color = color;
  m_next = (OBDColorRange*)next;
}

word OBDColorRange::color(long value)
{
  if (m_lessThan) {
    if (value <= m_threshold)
      return m_color;
    else if (m_next)
      return m_next->color(value);
    else
      return PUMA_ALARM_COLOR;
  }

  // For the 'Else' case we just return the specified color.
  return m_color;
}





