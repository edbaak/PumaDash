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

/*
  // Land Rover Defender MY12 Puma Dashboard
  // Ford 2.2L TDCI uses CAN 11bit (500Kb)
  // Protocol ISO 15765-4 11Bit (500Kb)
*/

#ifndef OBD_h
#define OBD_h

#include "Utils.h"
#include <Arduino.h>
#include <SPI.h>
#include "CAN.h"

// ***************************************************************
// Mode 01h = Show Current data
// Mode 02h = Show Freeze frame data
// ***************************************************************

#define PID_SUPPORTED_PID_01_20         0x00 // Returns a bit pattern for which PID's are supported
#define PID_MON_STATUS_SINCE_DTC_CLR 	  0x01 // Monitor status since DTC cleared
#define PID_FREEZE_DTC					        0x02
#define PID_FUEL_SYSTEM_STATUS			    0x03
#define PID_CALCULATED_ENGINE_LOAD		  0x04
#define PID_COOLANT_TEMP 				        0x05
#define PID_SHORT_TERM_FUEL_TRIM_1 		  0x06
#define PID_LONG_TERM_FUEL_TRIM_1 		  0x07
#define PID_SHORT_TERM_FUEL_TRIM_2 		  0x08
#define PID_LONG_TERM_FUEL_TRIM_2 		  0x09
#define PID_FUEL_PRESSURE 				      0x0A
#define PID_INTAKE_MANIFOLD_PRESSURE	  0x0B // INTAKE_MAP
#define PID_RPM 						            0x0C
#define PID_SPEED 						          0x0D
#define PID_TIMING_ADVANCE 				      0x0E
#define PID_INTAKE_AIR_TEMP 			      0x0F
#define PID_MAF_AIR_FLOW_RATE 			    0x10
#define PID_THROTTLE_POSITION 			    0x11
#define PID_COMM_SECONDARY_AIR_STATUS 	0x12 // Commanded secondary air status
#define PID_O2_SENSORS_PRESENT_2_BANKS	0x13 // [A0..A3] == Bank 1, Sensors 1-4. [A4..A7] == Bank 2, Sensors 1-4
#define PID_OXYGEN_SENSOR_1A			      0x14
#define PID_OXYGEN_SENSOR_2A			      0x15
#define PID_OXYGEN_SENSOR_3A			      0x16
#define PID_OXYGEN_SENSOR_4A			      0x17
#define PID_OXYGEN_SENSOR_5A			      0x18
#define PID_OXYGEN_SENSOR_6A			      0x19
#define PID_OXYGEN_SENSOR_7A			      0x1A
#define PID_OXYGEN_SENSOR_8A			      0x1B
#define PID_OBD_STANDARD				        0x1C // OBD standard this vehicle conforms to
#define PID_O2_SENSORS_PRESENT_4_BANKS	0x1D // O2 Sensors present in four banks: [A0..A7] == [B1S1, B1S2, B2S1, B2S2, B3S1, B3S2, B4S1, B4S2]
#define PID_AUX_INPUT_STATUS 			      0x1E
#define PID_RUNTIME_SINCE_ENG_START		  0x1F
#define PID_SUPPORTED_PID_21_40         0x20 // Returns a bit pattern for which PID's are supported
#define PID_DISTANCE_WITH_MIL_ON 		    0x21
#define PID_FUEL_RAIL_PRESSURE			    0x22
#define PID_FUEL_RAIL_GAUGE_PRESSURE	  0x23
#define PID_OXYGEN_SENSOR_1B			      0x24
#define PID_OXYGEN_SENSOR_2B			      0x25
#define PID_OXYGEN_SENSOR_3B			      0x26
#define PID_OXYGEN_SENSOR_4B			      0x27
#define PID_OXYGEN_SENSOR_5B			      0x28
#define PID_OXYGEN_SENSOR_6B			      0x29
#define PID_OXYGEN_SENSOR_7B			      0x2A
#define PID_OXYGEN_SENSOR_8B			      0x2B
#define PID_COMMANDED_EGR 				      0x2C
#define PID_EGR_ERROR 					        0x2D
#define PID_COMMANDED_EVAPORATIVE_PURGE 0x2E
#define PID_FUEL_LEVEL 					        0x2F
#define PID_WARMS_UPS_SINCE_DTC_CLEARED 0x30
#define PID_DISTANCE_SINCE_DTC_CLEARED  0x31
#define PID_EVAP_SYS_VAPOR_PRESSURE 	  0x32
#define PID_BAROMETRIC_PRESSURE			    0x33
#define PID_OXYGEN_SENSOR_1C			      0x34
#define PID_OXYGEN_SENSOR_2C			      0x35
#define PID_OXYGEN_SENSOR_3C			      0x36
#define PID_OXYGEN_SENSOR_4C			      0x37
#define PID_OXYGEN_SENSOR_5C			      0x38
#define PID_OXYGEN_SENSOR_6C			      0x39
#define PID_OXYGEN_SENSOR_7C			      0x3A
#define PID_OXYGEN_SENSOR_8C			      0x3B
#define PID_CATALYST_TEMP_B1S1 			    0x3C
#define PID_CATALYST_TEMP_B2S1 			    0x3D
#define PID_CATALYST_TEMP_B1S2 			    0x3E
#define PID_CATALYST_TEMP_B2S2 			    0x3F
#define PID_SUPPORTED_PID_41_60         0x40 // Returns a bit pattern for which PID's are supported
#define PID_MONITOR_STATUS_THIS_DRIVE	  0x41
#define PID_CONTROL_MODULE_VOLTAGE 		  0x42
#define PID_ABSOLUTE_ENGINE_LOAD 		    0x43
#define PID_AIR_FUEL_EQUIV_RATIO 		    0x44
#define PID_RELATIVE_THROTTLE_POS 		  0x45
#define PID_AMBIENT_AIR_TEMP 			      0x46
#define PID_ABSOLUTE_THROTTLE_POS_B 	  0x47
#define PID_ABSOLUTE_THROTTLE_POS_C 	  0x48
#define PID_ACC_PEDAL_POS_D 			      0x49
#define PID_ACC_PEDAL_POS_E 			      0x4A
#define PID_ACC_PEDAL_POS_F 			      0x4B
#define PID_COMMANDED_THROTTLE_ACTUATOR 0x4C
#define PID_RUN_TIME_WITH_MIL_ON		    0x4D
#define PID_TIME_SINCE_DTC_CLEARED 		  0x4E
#define PID_MAX_VALUE_FUEL_AIR_EQ		    0x4F
#define PID_MAX_VALUE_FOR_AIR_FLOW_RATE 0x50
#define PID_FUEL_TYPE					          0x51
#define PID_ETHANOL_FUEL 				        0x52
#define PID_ABSOLUTE_EVAP_VAPOR_PRESS	  0x53
#define PID_EVAP_SYSTEM_VAPOR_PRESS		  0x54
#define PID_ST_SEC_O2_SENSOR_TRIM_A1B3	0x55
#define PID_LT_SEC_O2_SENSOR_TRIM_A1B3	0x56
#define PID_ST_SEC_O2_SENSOR_TRIM_A2B4	0x57
#define PID_LT_SEC_O2_SENSOR_TRIM_A2B4	0x58
#define PID_FUEL_RAIL_ABS_PRESSURE 		  0x59
#define PID_RELATIVE_ACC_PEDAL_POS		  0x5A
#define PID_HYBRID_BATT_REMAINING_LIFE	0x5B
#define PID_ENGINE_OIL_TEMP 			      0x5C
#define PID_FUEL_INJECTION_TIMING 		  0x5D
#define PID_ENGINE_FUEL_RATE 			      0x5E
#define PID_EMISSION_REQ				        0x5F // Emission req to which engine is designed
#define PID_SUPPORTED_PID_61_80         0x60 // Returns a bit pattern for which PID's are supported
#define PID_ENGINE_TORQUE_DEMANDED 		  0x61 // Driver's demand engine - percent torque
#define PID_ENGINE_TORQUE_PERCENTAGE 	  0x62 // Actual engine - percent torque
#define PID_ENGINE_REF_TORQUE 			    0x63 // Engine reference torque
#define PID_ENGINE_PERC_TORQUE_DATA		  0x64 // Engine percent torque data
#define PID_AUX_INPUT_OUTPUT_SUPPORTED	0x65
#define PID_MASS_AIR_FLOW_SENSOR		    0x66
#define PID_ENGINE_COOLANT_TEMPERATURE	0x67
#define PID_INTAKE_AIR_TEMP_SENSOR		  0x68
#define PID_COMMANDED_EGR_AND_EGR_ERR	  0x69
#define PID_DIESEL_INTAKE_AIR_FLOW_CTRL	0x6A // Commanded Diesel intake air flow control
#define PID_EGR_TEMPERATURE				      0x6B
#define PID_COMM_THROTTLE_ACT_CTRL_POS	0x6C // Commanded throttle actuator control and relative throttle position
#define PID_FUEL_PRESS_CTRL_SYSTEM		  0x6D
#define PID_INJECTION_PRESS_CTRL_SYSTEM	0x6E
#define PID_TURBO_INLET_PRESSURE		    0x6F
#define PID_BOOST_PRESSURE_CTRL			    0x70
#define PID_VAR_GEOMETRY_CTRL			      0x71
#define PID_WASTEGATE_CTRL				      0x72
#define PID_EXHAUST_PRESSURE			      0x73
#define PID_TURBO_RPM					          0x74
#define PID_TURBO_TEMP1					        0x75
#define PID_TURBO_TEMP2					        0x76
#define PID_CHARGE_AIR_COOLER_TEMP		  0x77
#define PID_EGT_BANK1					          0x78 // Exhaust Gas Temperature Bank 1
#define PID_EGT_BANK2					          0x79 // Exhaust Gas Temperature Bank 2
#define PID_DIESEL_PART_FILTER1			    0x7A // Diesel Particulate Filter
#define PID_DIESEL_PART_FILTER2			    0x7B // Diesel Particulate Filter
#define PID_DIESEL_PART_FILTER_TEMP		  0x7C // Diesel Particulate Filter
#define PID_NOX_NTE_CONTROL_STATUS		  0x7D
#define PID_PM_NTE_CONTROL_STATUS		    0x7E
#define PID_ENGINE_RUN_TIME				      0x7F
#define PID_SUPPORTED_PID_81_A0         0x80 // Returns a bit pattern for which PID's are supported
#define PID_ENG_RUN_TME_AUX_EMISS_CTRL1 0x81 // Engine Run Time Auxiliary Emission Control
#define PID_ENG_RUN_TME_AUX_EMISS_CTRL2 0x82 // Engine Run Time Auxiliary Emission Control
#define PID_NOX_SENSOR					        0x83
#define PID_MANIFOLD_SURFACE_TEMP		    0x84
#define PID_NOX_REAGENT_SYSTEM			    0x85
#define PID_PARTICULATE_MATTER_SENSOR	  0x86
#define PID_INTAKE_MANIFOLD_ABS_PRESS	  0x87


//***************************************************************
// Mode 03h = Show stored diagnostic trouble codes DTC
// ***************************************************************

// Currently NOT supported

// ***************************************************************
// Mode 04h = Clear diagnostic trouble codes and stored values
// ***************************************************************

// Clears all DTC codes and stored values, and switches off the MIL light
// Currently NOT supported

// ***************************************************************
// Mode 05h = Test results, oxygen sensor monitoring (non CAN only)
// ***************************************************************

// Currently NOT supported

// ***************************************************************
// Mode 06h = Test results, other component/system monitoring (Test results, oxygen sensor
// monitoring for CAN only)
// ***************************************************************

// Currently NOT supported

// ***************************************************************
// Mode 07h = Show pending Diagnostic Trouble Codes (detected during current or last driving cycle)
// ***************************************************************

// Currently NOT supported

// ***************************************************************
// Mode 08h = Control operation of on-board component/system
// ***************************************************************

// Currently NOT supported

// ***************************************************************
// Mode 09h = Request vehicle information
// ***************************************************************

// Currently NOT supported
#define PID_MODE9_VIN					    0x03 // Returns VIN number (20 ascii char max)
#define PID_MODE9_ECU_NAME				0x0A // Returns ECU name (20 ascii char max)

// ***************************************************************
// Mode 0Ah = Permanent Diagnostic Trouble Codes (DTCs) (Cleared DTCs)
// ***************************************************************

// Currently NOT supported


// ***************************************************************
// The following PID values are more interesting for internal usage, and not expected to
// be used in apps directly
// ***************************************************************

// To request the value of a specific PID, a PID_REQUEST is done to the CAN bus
#define PID_REQUEST         0x7DF

// When an ECU responds to a PID_REQUEST it will reply with a PID_REPLY code.
// A 0x7E8 will come from the main ECU in the system. Other ECU's will have higher
// numbers, i.e. 0x7E9, 0x7EA, etc
#define PID_REPLY			0x7E8


// The following undocumented 0xID's are emitted regularly by the Puma ECU
//  9A, 
//  DC, 
//  DD, 
//  DF, 
//  E0, 
//  E1, 
// 16E, 
// 193, 
// 226, 
// 1B8, 
// 2B8, 
// 2DD, 
// 326
// 34B, 
// 394, 
// 400, 
// 405, 
// 4C0, 
// 5C0, 

#define MAX_RX_FIFO 12

typedef enum OBD_DATA_CONVERSION {
  BYTE_NO_CONVERSION,
  BYTE_PERCENTAGE,
  WORD_NO_CONVERSION,
  WORD_DIV4,
  WORD_DIV20,
  INT_MINUS40,
  INT_NO_CONVERSION,
  BYTE_TIMES10,
  BYTE_TIMES3,
  ULONG_NO_CONVERSION
} OBD_DATA_CONVERSION;

class OBDData
{
  public:
    OBDData();
    OBDData(uint8_t pid, String label, String format, String subLabel, uint16_t updateInterval, OBD_DATA_CONVERSION conversion, long min, long max, long step);
    virtual ~OBDData();

    bool needsUpdate();

    uint8_t pid();
    String label();
    String subLabel();
    word color();
    OBD_DATA_CONVERSION dataConversion();
    void setValue(uint32_t timeStamp, uint8_t *data);

    String toString();
    byte stringLength();
    byte toByte();
    word toWord();
    int toInt();
    long toLong();

#ifdef LOOPBACK_MODE
    void simulateData(CAN_Frame *message);
#endif

  protected:
    friend class PumaOBD;
    OBDData *m_next;
    uint32_t m_timeStamp;
    uint32_t m_updateRequested;
    uint16_t m_updateInterval;
    uint8_t m_pid;
    String m_label;
    String m_subLabel;
    String m_format;
    uint32_t m_simulator;
    OBD_DATA_CONVERSION m_conversion;

  private:
    long m_value;
#ifdef LOOPBACK_MODE
    long m_simValue;
    bool m_simIncrease;
    long m_simMinValue;
    long m_simMaxValue;
    long m_simStepValue;
#endif
};

class PumaOBD
{
  public:
    PumaOBD();
    void setup();
    void readMessages();
    void requestObdUpdates();

    void readRxBuffers(); // Interupt driven

  protected:
    void addDataObject(OBDData *obj);
    OBDData *dataObject(uint8_t PID);

    void processMessage(CAN_Frame message);
    void requestPID(uint16_t pid);
    void requestSensorData(OBDData *sensor);

    OBDData m_invalidPID; // This object is used to return a valid pointer in case we don't support the PID
    OBDData *m_first;
    OBDData *m_last;
    OBDData *m_current;
    OBDData *iterateDataObject(bool needsUpdate);

  private:
    PumaCAN m_CAN;
    CAN_Frame m_rxFIFO[MAX_RX_FIFO];
    byte m_rxFIFO_head;
    byte m_rxFIFO_tail;
    byte m_rxFIFO_count;

  private:
#ifdef PID_DISCOVERY_MODE
    void addUnhandledPID(uint16_t pid);
    void printUnhandledPIDS();
    uint16_t m_unknownPIDS[MAX_UNKNOWN_PIDS];
#endif
};

#endif // OBD_h



