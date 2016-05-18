/*
  2012 Copyright (c) Seeed Technology Inc.  All right reserved.
  2014 Copyright (c) Cory J. Fowler  All Rights Reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-
  1301  USA
*/
/*
  2016 Copyright (c) Ed Baak  All Rights Reserved.

  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License 
  as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.

  In cases where the GPL 3 is conflicting the the LGPL mentioned above, 
  LGPL needs to be followed.

  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this code; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-
  1301  USA
*/

#ifndef _MCP2515_H_
#define _MCP2515_H_

// Constants

#include "Utils.h"
#include <Arduino.h>
#include <SPI.h>
#include <inttypes.h>

#ifndef uint32_t
#define uint32_t unsigned long
#endif

#ifndef uint8_t
#define uint8_t byte
#endif

//#define CAN_DEBUG 1                                                   /* print debug information */

/*
 *   Begin mt
 */
#define TIMEOUTVALUE    50
#define MCP_SIDH        0
#define MCP_SIDL        1
#define MCP_EID8        2
#define MCP_EID0        3

#define MCP_TXB_EXIDE_M     0x08                                        /* In TXBnSIDL                  */
#define MCP_DLC_MASK        0x0F                                        /* 4 LSBits                     */
#define MCP_RTR_MASK        0x40                                        /* (1<<6) Bit 6                 */

#define MCP_RXB_RX_ANY      0x60
#define MCP_RXB_RX_EXT      0x40
#define MCP_RXB_RX_STD      0x20
#define MCP_RXB_RX_STDEXT   0x00
#define MCP_RXB_RX_MASK     0x60
#define MCP_RXB_BUKT_MASK   (1<<2)

/*
** Bits in the TXBnCTRL registers.
*/
#define MCP_TXB_TXBUFE_M    0x80
#define MCP_TXB_ABTF_M      0x40
#define MCP_TXB_MLOA_M      0x20
#define MCP_TXB_TXERR_M     0x10
#define MCP_TXB_TXREQ_M     0x08
#define MCP_TXB_TXIE_M      0x04
#define MCP_TXB_TXP10_M     0x03

#define MCP_TXB_RTR_M       0x40                                        /* In TXBnDLC                   */
#define MCP_RXB_IDE_M       0x08                                        /* In RXBnSIDL                  */
#define MCP_RXB_RTR_M       0x40                                        /* In RXBnDLC                   */

#define MCP_STAT_RXIF_MASK   (0x03)
#define MCP_STAT_RX0IF       (1<<0)
#define MCP_STAT_RX1IF       (1<<1)

#define MCP_EFLG_RX1OVR     (1<<7)
#define MCP_EFLG_RX0OVR     (1<<6)
#define MCP_EFLG_TXBO       (1<<5)
#define MCP_EFLG_TXEP       (1<<4)
#define MCP_EFLG_RXEP       (1<<3)
#define MCP_EFLG_TXWAR      (1<<2)
#define MCP_EFLG_RXWAR      (1<<1)
#define MCP_EFLG_EWARN      (1<<0)
#define MCP_EFLG_ERRORMASK  (0xF8)                                      /* 5 MS-Bits                    */

/*
 *   Define MCP2515 register addresses
 */
#define MCP_RXF0SIDH    0x00
#define MCP_RXF0SIDL    0x01
#define MCP_RXF0EID8    0x02
#define MCP_RXF0EID0    0x03
#define MCP_RXF1SIDH    0x04
#define MCP_RXF1SIDL    0x05
#define MCP_RXF1EID8    0x06
#define MCP_RXF1EID0    0x07
#define MCP_RXF2SIDH    0x08
#define MCP_RXF2SIDL    0x09
#define MCP_RXF2EID8    0x0A
#define MCP_RXF2EID0    0x0B
#define MCP_CANSTAT     0x0E
#define MCP_CANCTRL     0x0F
#define MCP_RXF3SIDH    0x10
#define MCP_RXF3SIDL    0x11
#define MCP_RXF3EID8    0x12
#define MCP_RXF3EID0    0x13
#define MCP_RXF4SIDH    0x14
#define MCP_RXF4SIDL    0x15
#define MCP_RXF4EID8    0x16
#define MCP_RXF4EID0    0x17
#define MCP_RXF5SIDH    0x18
#define MCP_RXF5SIDL    0x19
#define MCP_RXF5EID8    0x1A
#define MCP_RXF5EID0    0x1B
#define MCP_TEC         0x1C
#define MCP_REC         0x1D
#define MCP_RXM0SIDH    0x20
#define MCP_RXM0SIDL    0x21
#define MCP_RXM0EID8    0x22
#define MCP_RXM0EID0    0x23
#define MCP_RXM1SIDH    0x24
#define MCP_RXM1SIDL    0x25
#define MCP_RXM1EID8    0x26
#define MCP_RXM1EID0    0x27
#define MCP_CNF3        0x28
#define MCP_CNF2        0x29
#define MCP_CNF1        0x2A
#define MCP_CANINTE     0x2B
#define MCP_CANINTF     0x2C
#define MCP_EFLG        0x2D
#define MCP_TXB0CTRL    0x30
#define MCP_TXB1CTRL    0x40
#define MCP_TXB2CTRL    0x50
#define MCP_RXB0CTRL    0x60
#define MCP_RXB0SIDH    0x61
#define MCP_RXB1CTRL    0x70
#define MCP_RXB1SIDH    0x71

#define MCP_TX_INT          0x1C                                    // Enable all transmit interrup ts
#define MCP_TX01_INT        0x0C                                    // Enable TXB0 and TXB1 interru pts
#define MCP_RX_INT          0x03                                    // Enable receive interrupts
#define MCP_NO_INT          0x00                                    // Disable all interrupts

#define MCP_TX01_MASK       0x14
#define MCP_TX_MASK         0x54

/*
 *   Define SPI Instruction Set
 */
#define MCP2515_SPI_WRITE           0x02
#define MCP2515_SPI_READ            0x03
#define MCP2515_SPI_BITMOD          0x05
#define MCP2515_SPI_READ_STATUS     0xA0
#define MCP2515_SPI_RX_STATUS       0xB0
#define MCP2515_SPI_RESET           0xC0

#define MCP_LOAD_TX0        0x40
#define MCP_LOAD_TX1        0x42
#define MCP_LOAD_TX2        0x44

#define MCP_RTS_TX0         0x81
#define MCP_RTS_TX1         0x82
#define MCP_RTS_TX2         0x84
#define MCP_RTS_ALL         0x87

#define MCP_READ_RX0        0x90
#define MCP_READ_RX1        0x94

/*
 *   CANCTRL Register Values
 */
#define MODE_POWERUP    0xE0
#define MODE_MASK       0xE0
#define ABORT_TX        0x10
#define MODE_ONESHOT    0x08

#define CLKOUT_ENABLE   0x04
#define CLKOUT_DISABLE  0x00
#define CLKOUT_PS1      0x00
#define CLKOUT_PS2      0x01
#define CLKOUT_PS4      0x02
#define CLKOUT_PS8      0x03

/*
 *   CNF1 Register Values
 */
#define SJW1            0x00
#define SJW2            0x40
#define SJW3            0x80
#define SJW4            0xC0

/*
 *   CNF2 Register Values
 */
#define BTLMODE         0x80
#define SAMPLE_1X       0x00
#define SAMPLE_3X       0x40

/*
 *   CNF3 Register Values
 */
#define SOF_ENABLE      0x80
#define SOF_DISABLE     0x00
#define WAKFIL_ENABLE   0x40
#define WAKFIL_DISABLE  0x00

/*
 *   CANINTF Register Bits
 */
#define MCP_RX0IF       0x01
#define MCP_RX1IF       0x02
#define MCP_TX0IF       0x04
#define MCP_TX1IF       0x08
#define MCP_TX2IF       0x10
#define MCP_ERRIF       0x20
#define MCP_WAKIF       0x40
#define MCP_MERRF       0x80


#define MCPDEBUG        (0)
#define MCPDEBUG_TXBUF  (0)
#define MCP_N_TXBUFFERS (3)

#define MCP_RXBUF_0 (MCP_RXB0SIDH)
#define MCP_RXBUF_1 (MCP_RXB1SIDH)

//#define CANSENDTIMEOUT (200)                                            /* milliseconds                 */
//#define CANAUTOPROCESS (1)
//#define CANAUTOON  (1)
//#define CANAUTOOFF (0)
//#define CAN_STDID (0)
//#define CAN_EXTID (1)
//#define CANDEFAULTIDENT    (0x55CC)
//#define CANDEFAULTIDENTEXT (CAN_EXTID)

#define MAX_CHAR_IN_MESSAGE 8

// End of constants


class CAN_Frame
{
  public:
    CAN_Frame();
    void init(uint32_t id, uint8_t len, uint8_t *buf);
    void clear();

    uint32_t m_id;            // if (extended == CAN_RECESSIVE) { extended ID } else { standard ID }
    uint8_t m_rtr;            // Remote Transmission Request Bit (RTR)
    uint8_t m_extended;       // Identifier Extension Bit (IDE)
    uint8_t m_length;         // Data Length
    uint8_t m_data[8];        // Message data
};

class PumaCAN
{
  public:
    typedef enum CAN_CLOCK {
      MCP_20MHZ = 0,
      MCP_16MHZ = 1,
      MCP_8MHZ  = 2
    } CAN_CLOCK;

    typedef enum CAN_SPEED {
      CAN_4K096BPS = 0,
      CAN_5KBPS    = 1,
      CAN_10KBPS   = 2,
      CAN_20KBPS   = 3,
      CAN_31K25BPS = 4,
      CAN_40KBPS   = 5,
      CAN_50KBPS   = 6,
      CAN_80KBPS   = 7,
      CAN_100KBPS  = 8,
      CAN_125KBPS  = 9,
      CAN_200KBPS  = 10,
      CAN_250KBPS  = 11,
      CAN_500KBPS  = 12,
      CAN_1000KBPS = 13
    } CAN_SPEED;

    typedef enum ID_MODE_SET {
      MCP_STDEXT = 0,                                                  /* Standard and Extended        */
      MCP_STD    = 1,                                                  /* Standard IDs ONLY            */
      MCP_EXT    = 2,                                                  /* Extended IDs ONLY            */
      MCP_ANY    = 3                                                   /* Disables Masks and Filters   */
    } ID_MODE_SET;

    typedef enum CAN_MASK {
      MASK0,
      MASK1
    } MASK;

    typedef enum CAN_FILTER {
      FILT0,
      FILT1,
      FILT2,
      FILT3,
      FILT4,
      FILT5
    } CAN_FILTER;

    typedef enum CAN_MODE {
      MCP_NORMAL     = 0x00,
      MCP_SLEEP      = 0x20,
      MCP_LOOPBACK   = 0x40,
      MCP_LISTENONLY = 0x60,
      MCP_CONFIG     = 0x80
    } CAN_MODE;
    
    PumaCAN();

    bool begin(uint8_t _CS, ID_MODE_SET idmodeset, CAN_SPEED speedset, CAN_CLOCK clockset);       // Initilize controller prameters
    bool setMode(CAN_MODE opMode);                                        // Set operational mode
    bool hasError();                                             // Check for errors
    void softReset();                                           // Soft Reset MCP2515

    bool write(CAN_Frame message);                                     // Write a message into a TX buffer

    CAN_Frame read();                                                     // Read message from a RX buffer
    bool available();                                           // Check for received data
    bool setMask(CAN_MASK maskNo, uint8_t ext, uint32_t ulData);               // Initilize Mask(s)
    bool setFilter(CAN_FILTER filterNo, uint8_t ext, uint32_t ulData);               // initilize Filter(s)

  private:
    uint8_t   MCPCS;
    CAN_MODE  m_curMode;

  private:
    uint8_t mcp2515_readRegister(const uint8_t address);                    // Read MCP2515 register

    void mcp2515_readRegisterS(const uint8_t address,                     // Read MCP2515 successive registers
                               uint8_t values[],
                               const uint8_t n);

    void mcp2515_setRegister(const uint8_t address,                       // Set MCP2515 register
                             const uint8_t value);

    void mcp2515_setRegisterS(const uint8_t address,                      // Set MCP2515 successive registers
                              const uint8_t values[],
                              const uint8_t n);

    void mcp2515_initCANBuffers(void);

    void mcp2515_modifyRegister(const uint8_t address,                    // Set specific bit(s) of a register
                                const uint8_t mask,
                                const uint8_t data);

    uint8_t mcp2515_readStatus(void);                                     // Read MCP2515 Status
    bool mcp2515_setCANCTRL_Mode(CAN_MODE newmode);                 // Set mode
    bool mcp2515_configRate(const CAN_SPEED canSpeed,                      // Set baudrate
                            const CAN_CLOCK canClock);

    bool mcp2515_init(const uint8_t canIDMode,                           // Initialize Controller
                      const CAN_SPEED canSpeed,
                      const CAN_CLOCK canClock);

    void mcp2515_write_mf( const uint8_t mcp_addr,                        // Write CAN Mask or Filter
                           const uint8_t ext,
                           const uint32_t id );

    void mcp2515_write_id( const uint8_t mcp_addr,                        // Write CAN ID
                           const uint8_t ext,
                           const uint32_t id );

    void mcp2515_read_id( const uint8_t mcp_addr,                         // Read CAN ID
                          uint8_t* ext,
                          uint32_t* id );

    void mcp2515_write_canMsg(const uint8_t buffer_sidh_addr, CAN_Frame message);          // Write CAN message
    void mcp2515_read_canMsg(const uint8_t buffer_sidh_addr, CAN_Frame *message);            // Read CAN message
    uint8_t mcp2515_getNextFreeTXBuf(uint8_t *txbuf_n);                     // Find empty transmit buffer
};

#endif

