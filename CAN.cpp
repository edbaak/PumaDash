/*
  2012 Copyright (c) Seeed Technology Inc.  All right reserved.
  2014 Copyright (c) Cory J. Fowler  All Rights Reserved.

  Author: Loovee
  Contributor: Cory J. Fowler
  2014-9-16
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

#include "CAN.h"

/*
 *  Speed 8M
 */
#define MCP_8MHz_1000kBPS_CFG1 (0x00)
#define MCP_8MHz_1000kBPS_CFG2 (0x80)
#define MCP_8MHz_1000kBPS_CFG3 (0x80)

#define MCP_8MHz_500kBPS_CFG1 (0x00)
#define MCP_8MHz_500kBPS_CFG2 (0x90)
#define MCP_8MHz_500kBPS_CFG3 (0x82)

#define MCP_8MHz_250kBPS_CFG1 (0x00)
#define MCP_8MHz_250kBPS_CFG2 (0xB1)
#define MCP_8MHz_250kBPS_CFG3 (0x85)

#define MCP_8MHz_200kBPS_CFG1 (0x00)
#define MCP_8MHz_200kBPS_CFG2 (0xB4)
#define MCP_8MHz_200kBPS_CFG3 (0x86)

#define MCP_8MHz_125kBPS_CFG1 (0x01)
#define MCP_8MHz_125kBPS_CFG2 (0xB1)
#define MCP_8MHz_125kBPS_CFG3 (0x85)

#define MCP_8MHz_100kBPS_CFG1 (0x01)
#define MCP_8MHz_100kBPS_CFG2 (0xB4)
#define MCP_8MHz_100kBPS_CFG3 (0x86)

#define MCP_8MHz_80kBPS_CFG1 (0x01)
#define MCP_8MHz_80kBPS_CFG2 (0xBF)
#define MCP_8MHz_80kBPS_CFG3 (0x87)

#define MCP_8MHz_50kBPS_CFG1 (0x03)
#define MCP_8MHz_50kBPS_CFG2 (0xB4)
#define MCP_8MHz_50kBPS_CFG3 (0x86)

#define MCP_8MHz_40kBPS_CFG1 (0x03)
#define MCP_8MHz_40kBPS_CFG2 (0xBF)
#define MCP_8MHz_40kBPS_CFG3 (0x87)

#define MCP_8MHz_31k25BPS_CFG1 (0x07)
#define MCP_8MHz_31k25BPS_CFG2 (0xA4)
#define MCP_8MHz_31k25BPS_CFG3 (0x84)

#define MCP_8MHz_20kBPS_CFG1 (0x07)
#define MCP_8MHz_20kBPS_CFG2 (0xBF)
#define MCP_8MHz_20kBPS_CFG3 (0x87)

#define MCP_8MHz_10kBPS_CFG1 (0x0F)
#define MCP_8MHz_10kBPS_CFG2 (0xBF)
#define MCP_8MHz_10kBPS_CFG3 (0x87)

#define MCP_8MHz_5kBPS_CFG1 (0x1F)
#define MCP_8MHz_5kBPS_CFG2 (0xBF)
#define MCP_8MHz_5kBPS_CFG3 (0x87)

/*
 *  speed 16M
 */
#define MCP_16MHz_1000kBPS_CFG1 (0x00)
#define MCP_16MHz_1000kBPS_CFG2 (0xD0)
#define MCP_16MHz_1000kBPS_CFG3 (0x82)

#define MCP_16MHz_500kBPS_CFG1 (0x00)
#define MCP_16MHz_500kBPS_CFG2 (0xF0)
#define MCP_16MHz_500kBPS_CFG3 (0x86)

#define MCP_16MHz_250kBPS_CFG1 (0x41)
#define MCP_16MHz_250kBPS_CFG2 (0xF1)
#define MCP_16MHz_250kBPS_CFG3 (0x85)

#define MCP_16MHz_200kBPS_CFG1 (0x01)
#define MCP_16MHz_200kBPS_CFG2 (0xFA)
#define MCP_16MHz_200kBPS_CFG3 (0x87)

#define MCP_16MHz_125kBPS_CFG1 (0x03)
#define MCP_16MHz_125kBPS_CFG2 (0xF0)
#define MCP_16MHz_125kBPS_CFG3 (0x86)

#define MCP_16MHz_100kBPS_CFG1 (0x03)
#define MCP_16MHz_100kBPS_CFG2 (0xFA)
#define MCP_16MHz_100kBPS_CFG3 (0x87)

#define MCP_16MHz_80kBPS_CFG1 (0x03)
#define MCP_16MHz_80kBPS_CFG2 (0xFF)
#define MCP_16MHz_80kBPS_CFG3 (0x87)

#define MCP_16MHz_50kBPS_CFG1 (0x07)
#define MCP_16MHz_50kBPS_CFG2 (0xFA)
#define MCP_16MHz_50kBPS_CFG3 (0x87)

#define MCP_16MHz_40kBPS_CFG1 (0x07)
#define MCP_16MHz_40kBPS_CFG2 (0xFF)
#define MCP_16MHz_40kBPS_CFG3 (0x87)

#define MCP_16MHz_20kBPS_CFG1 (0x0F)
#define MCP_16MHz_20kBPS_CFG2 (0xFF)
#define MCP_16MHz_20kBPS_CFG3 (0x87)

#define MCP_16MHz_10kBPS_CFG1 (0x1F)
#define MCP_16MHz_10kBPS_CFG2 (0xFF)
#define MCP_16MHz_10kBPS_CFG3 (0x87)

#define MCP_16MHz_5kBPS_CFG1 (0x3F)
#define MCP_16MHz_5kBPS_CFG2 (0xFF)
#define MCP_16MHz_5kBPS_CFG3 (0x87)

/*
 *  speed 20M
 */
#define MCP_20MHz_1000kBPS_CFG1 (0x00)
#define MCP_20MHz_1000kBPS_CFG2 (0xD9)
#define MCP_20MHz_1000kBPS_CFG3 (0x82)

#define MCP_20MHz_500kBPS_CFG1 (0x00)
#define MCP_20MHz_500kBPS_CFG2 (0xFA)
#define MCP_20MHz_500kBPS_CFG3 (0x87)

#define MCP_20MHz_250kBPS_CFG1 (0x41)
#define MCP_20MHz_250kBPS_CFG2 (0xFB)
#define MCP_20MHz_250kBPS_CFG3 (0x86)

#define MCP_20MHz_200kBPS_CFG1 (0x01)
#define MCP_20MHz_200kBPS_CFG2 (0xFF)
#define MCP_20MHz_200kBPS_CFG3 (0x87)

#define MCP_20MHz_125kBPS_CFG1 (0x03)
#define MCP_20MHz_125kBPS_CFG2 (0xFA)
#define MCP_20MHz_125kBPS_CFG3 (0x87)

#define MCP_20MHz_100kBPS_CFG1 (0x04)
#define MCP_20MHz_100kBPS_CFG2 (0xFA)
#define MCP_20MHz_100kBPS_CFG3 (0x87)

#define MCP_20MHz_80kBPS_CFG1 (0x04)
#define MCP_20MHz_80kBPS_CFG2 (0xFF)
#define MCP_20MHz_80kBPS_CFG3 (0x87)

#define MCP_20MHz_50kBPS_CFG1 (0x09)
#define MCP_20MHz_50kBPS_CFG2 (0xFA)
#define MCP_20MHz_50kBPS_CFG3 (0x87)

#define MCP_20MHz_40kBPS_CFG1 (0x09)
#define MCP_20MHz_40kBPS_CFG2 (0xFF)
#define MCP_20MHz_40kBPS_CFG3 (0x87)

#define MCP2515_SELECT()   digitalWrite(MCPCS, LOW)
#define MCP2515_UNSELECT() digitalWrite(MCPCS, HIGH)


#define spi_readwrite SPI.transfer
#define spi_read() spi_readwrite(0x00)

CAN_Frame::CAN_Frame()
{
  clear();
}

void CAN_Frame::init(uint32_t id, uint8_t len, uint8_t *buf)
{
  m_extended = 0;
  m_id     = id;
  m_length    = len;
  for (int i = 0; i < MAX_CHAR_IN_MESSAGE; i++)
    m_data[i] = *(buf + i);  
}

void CAN_Frame::clear()
{
  m_id       = 0;
  m_length      = 0;
  m_extended   = 0;
  m_rtr      = 0;
  for (int i = 0; i < MAX_CHAR_IN_MESSAGE; i++ )
    m_data[i] = 0x00;
}

/*********************************************************************************************************
** Function name:           PumaCAN
** Descriptions:            Public function to declare CAN class and the /CS pin.
*********************************************************************************************************/
PumaCAN::PumaCAN()
{
}

/*********************************************************************************************************
** Function name:           begin
** Descriptions:            Public function to declare controller initialization parameters.
*********************************************************************************************************/
bool PumaCAN::begin(uint8_t _CS, ID_MODE_SET idmodeset, CAN_SPEED speedset, CAN_CLOCK clockset)
{
  MCPCS = _CS;
  pinMode(MCPCS, OUTPUT);
  MCP2515_UNSELECT();

  SPI.begin();
  return mcp2515_init(idmodeset, speedset, clockset);
}

/*********************************************************************************************************
** Function name:           init_Mask
** Descriptions:            Public function to set mask(s).
*********************************************************************************************************/
bool PumaCAN::setMask(CAN_MASK maskNo, uint8_t ext, uint32_t ulData)
{
#if CAN_DEBUG
  Serial.println("Starting to Set Mask");
#endif
  if (!mcp2515_setCANCTRL_Mode(MCP_CONFIG)) {
#if CAN_DEBUG
    Serial.println("Entering Configuration Mode Failure");
#endif
    return false;
  }

  if (maskNo == MASK0) {
    mcp2515_write_mf(MCP_RXM0SIDH, ext, ulData);
  } else {
    mcp2515_write_mf(MCP_RXM1SIDH, ext, ulData);
  }

  if (!mcp2515_setCANCTRL_Mode(m_curMode)) {
#if CAN_DEBUG
    Serial.println("Entering Previous Mode Failure...\r\nSetting Mask Failure");
#endif
    return false;
  }
  
#if CAN_DEBUG
  Serial.println("Setting Mask Successful");
#endif
  return true;
}

/*********************************************************************************************************
** Function name:           init_Filt
** Descriptions:            Public function to set filter(s).
*********************************************************************************************************/
bool PumaCAN::setFilter(CAN_FILTER filterNo, uint8_t ext, uint32_t ulData)
{
#if CAN_DEBUG
  Serial.println("Starting to Set Filter");
#endif
  if (!mcp2515_setCANCTRL_Mode(MCP_CONFIG)) {
#if CAN_DEBUG
    Serial.println("Enter Configuration Mode Failure");
#endif
    return false;
  }

  switch ( filterNo )
  {
    case FILT0:
      mcp2515_write_mf(MCP_RXF0SIDH, ext, ulData);
      break;

    case FILT1:
      mcp2515_write_mf(MCP_RXF1SIDH, ext, ulData);
      break;

    case FILT2:
      mcp2515_write_mf(MCP_RXF2SIDH, ext, ulData);
      break;

    case FILT3:
      mcp2515_write_mf(MCP_RXF3SIDH, ext, ulData);
      break;

    case FILT4:
      mcp2515_write_mf(MCP_RXF4SIDH, ext, ulData);
      break;

    case FILT5:
      mcp2515_write_mf(MCP_RXF5SIDH, ext, ulData);
      break;

    default:
      return false;
  }

  if (!mcp2515_setCANCTRL_Mode(m_curMode)) {
#if CAN_DEBUG
    Serial.println("Entering Previous Mode Failure...\r\nSetting Filter Failure");
#endif
    return false;
  }
#if CAN_DEBUG
  Serial.println("Setting Filter Successfull");
#endif

  return true;
}


/*********************************************************************************************************
** Function name:           sendMsg
** Descriptions:            Send message
*********************************************************************************************************/
bool PumaCAN::write(CAN_Frame message)
{
  uint8_t res, res1, txbuf_n;
  uint16_t uiTimeOut = 0;

  do {
    res = mcp2515_getNextFreeTXBuf(&txbuf_n);                       /* info = addr.                 */
    uiTimeOut++;
  } while (res == 2 && (uiTimeOut < TIMEOUTVALUE));

  if (uiTimeOut == TIMEOUTVALUE)
  {
    return false;                                      /* get tx buff time out         */
  }
  uiTimeOut = 0;
  mcp2515_write_canMsg(txbuf_n, message);
  mcp2515_modifyRegister( txbuf_n - 1 , MCP_TXB_TXREQ_M, MCP_TXB_TXREQ_M );
  do
  {
    uiTimeOut++;
    res1 = mcp2515_readRegister(txbuf_n - 1);                       /* read send buff ctrl reg   */
    res1 = res1 & 0x08;
  } while (res1 && (uiTimeOut < TIMEOUTVALUE));
  if (uiTimeOut == TIMEOUTVALUE)                                      /* send msg timeout             */
  {
    return false;
  }
  
  return true;
}

CAN_Frame PumaCAN::read()
{
  CAN_Frame message;

  uint8_t stat = mcp2515_readStatus();

  if ( stat & MCP_STAT_RX0IF )                                        /* Msg in Buffer 0              */
  {
    mcp2515_read_canMsg(MCP_RXBUF_0, &message);
    mcp2515_modifyRegister(MCP_CANINTF, MCP_RX0IF, 0);
  }
  else if ( stat & MCP_STAT_RX1IF )                                   /* Msg in Buffer 1              */
  {
    mcp2515_read_canMsg(MCP_RXBUF_1, &message);
    mcp2515_modifyRegister(MCP_CANINTF, MCP_RX1IF, 0);
  }
  
  return message;
}

/*********************************************************************************************************
** Function name:           checkReceive
** Descriptions:            Public function, Checks for received data.  (Used if not using the interrupt output)
*********************************************************************************************************/
bool PumaCAN::available(void)
{
  uint8_t res = mcp2515_readStatus();                                         /* RXnIF in Bit 1 and 0         */
  return ( res & MCP_STAT_RXIF_MASK );
}

/*********************************************************************************************************
** Function name:           checkError
** Descriptions:            Public function, Returns error register data.
*********************************************************************************************************/
bool PumaCAN::hasError(void)
{
  uint8_t eflg = mcp2515_readRegister(MCP_EFLG);
  return ( eflg & MCP_EFLG_ERRORMASK );
}

/*********************************************************************************************************
** Function name:           mcp2515_reset
** Descriptions:            Performs a software reset
*********************************************************************************************************/
void PumaCAN::softReset(void)
{
  MCP2515_SELECT();
  spi_readwrite(MCP2515_SPI_RESET);
  MCP2515_UNSELECT();
  delay(10);
}

/*********************************************************************************************************
** Function name:           mcp2515_readRegister
** Descriptions:            Read data register
*********************************************************************************************************/
uint8_t PumaCAN::mcp2515_readRegister(const uint8_t address)
{
  MCP2515_SELECT();
  spi_readwrite(MCP2515_SPI_READ);
  spi_readwrite(address);
  uint8_t ret = spi_read();
  MCP2515_UNSELECT();
  return ret;
}

/*********************************************************************************************************
** Function name:           mcp2515_readRegisterS
** Descriptions:            Reads sucessive data registers
*********************************************************************************************************/
void PumaCAN::mcp2515_readRegisterS(const uint8_t address, uint8_t values[], const uint8_t n)
{
  MCP2515_SELECT();
  spi_readwrite(MCP2515_SPI_READ);
  spi_readwrite(address);
  // mcp2515 has auto-increment of address-pointer
  for (uint8_t i = 0; i < n; i++)
    values[i] = spi_read();
  MCP2515_UNSELECT();
}

/*********************************************************************************************************
** Function name:           mcp2515_setRegister
** Descriptions:            Sets data register
*********************************************************************************************************/
void PumaCAN::mcp2515_setRegister(const uint8_t address, const uint8_t value)
{
  MCP2515_SELECT();
  spi_readwrite(MCP2515_SPI_WRITE);
  spi_readwrite(address);
  spi_readwrite(value);
  MCP2515_UNSELECT();
  delayMicroseconds(250);
}

/*********************************************************************************************************
** Function name:           mcp2515_setRegisterS
** Descriptions:            Sets sucessive data registers
*********************************************************************************************************/
void PumaCAN::mcp2515_setRegisterS(const uint8_t address, const uint8_t values[], const uint8_t n)
{
  MCP2515_SELECT();
  spi_readwrite(MCP2515_SPI_WRITE);
  spi_readwrite(address);
  for (uint8_t i = 0; i < n; i++)
    spi_readwrite(values[i]);
  MCP2515_UNSELECT();
  delayMicroseconds(250);
}

/*********************************************************************************************************
** Function name:           mcp2515_modifyRegister
** Descriptions:            Sets specific bits of a register
*********************************************************************************************************/
void PumaCAN::mcp2515_modifyRegister(const uint8_t address, const uint8_t mask, const uint8_t data)
{
  MCP2515_SELECT();
  spi_readwrite(MCP2515_SPI_BITMOD);
  spi_readwrite(address);
  spi_readwrite(mask);
  spi_readwrite(data);
  MCP2515_UNSELECT();
  delayMicroseconds(250);
}

/*********************************************************************************************************
** Function name:           mcp2515_readStatus
** Descriptions:            Reads status register
*********************************************************************************************************/
uint8_t PumaCAN::mcp2515_readStatus(void)
{
  MCP2515_SELECT();
  spi_readwrite(MCP2515_SPI_READ_STATUS);
  uint8_t i = spi_read();
  MCP2515_UNSELECT();
  return i;
}

/*********************************************************************************************************
** Function name:           setMode
** Descriptions:            Sets control mode
*********************************************************************************************************/
bool PumaCAN::setMode(CAN_MODE opMode)
{
  m_curMode = opMode;
  return mcp2515_setCANCTRL_Mode(m_curMode);
}

/*********************************************************************************************************
** Function name:           mcp2515_setCANCTRL_Mode
** Descriptions:            Set control mode
*********************************************************************************************************/
bool PumaCAN::mcp2515_setCANCTRL_Mode(CAN_MODE newmode)
{
  mcp2515_modifyRegister(MCP_CANCTRL, MODE_MASK, newmode);
  uint8_t i = mcp2515_readRegister(MCP_CANCTRL);
  i &= MODE_MASK;
  return ((CAN_MODE)i == newmode);
}

/*********************************************************************************************************
** Function name:           mcp2515_configRate
** Descriptions:            Set baudrate
*********************************************************************************************************/
bool PumaCAN::mcp2515_configRate(const CAN_SPEED canSpeed, const CAN_CLOCK canClock)
{
  uint8_t set, cfg1, cfg2, cfg3;
  set = 1;
  switch (canClock)
  {
    case (MCP_8MHZ):
      switch (canSpeed)
      {
        case (CAN_5KBPS):                                               //   5KBPS
          cfg1 = MCP_8MHz_5kBPS_CFG1;
          cfg2 = MCP_8MHz_5kBPS_CFG2;
          cfg3 = MCP_8MHz_5kBPS_CFG3;
          break;

        case (CAN_10KBPS):                                              //  10KBPS
          cfg1 = MCP_8MHz_10kBPS_CFG1;
          cfg2 = MCP_8MHz_10kBPS_CFG2;
          cfg3 = MCP_8MHz_10kBPS_CFG3;
          break;

        case (CAN_20KBPS):                                              //  20KBPS
          cfg1 = MCP_8MHz_20kBPS_CFG1;
          cfg2 = MCP_8MHz_20kBPS_CFG2;
          cfg3 = MCP_8MHz_20kBPS_CFG3;
          break;

        case (CAN_31K25BPS):                                            //  31.25KBPS
          cfg1 = MCP_8MHz_31k25BPS_CFG1;
          cfg2 = MCP_8MHz_31k25BPS_CFG2;
          cfg3 = MCP_8MHz_31k25BPS_CFG3;
          break;

        case (CAN_40KBPS):                                              //  40Kbps
          cfg1 = MCP_8MHz_40kBPS_CFG1;
          cfg2 = MCP_8MHz_40kBPS_CFG2;
          cfg3 = MCP_8MHz_40kBPS_CFG3;
          break;

        case (CAN_50KBPS):                                              //  50Kbps
          cfg1 = MCP_8MHz_50kBPS_CFG1;
          cfg2 = MCP_8MHz_50kBPS_CFG2;
          cfg3 = MCP_8MHz_50kBPS_CFG3;
          break;

        case (CAN_80KBPS):                                              //  80Kbps
          cfg1 = MCP_8MHz_80kBPS_CFG1;
          cfg2 = MCP_8MHz_80kBPS_CFG2;
          cfg3 = MCP_8MHz_80kBPS_CFG3;
          break;

        case (CAN_100KBPS):                                             // 100Kbps
          cfg1 = MCP_8MHz_100kBPS_CFG1;
          cfg2 = MCP_8MHz_100kBPS_CFG2;
          cfg3 = MCP_8MHz_100kBPS_CFG3;
          break;

        case (CAN_125KBPS):                                             // 125Kbps
          cfg1 = MCP_8MHz_125kBPS_CFG1;
          cfg2 = MCP_8MHz_125kBPS_CFG2;
          cfg3 = MCP_8MHz_125kBPS_CFG3;
          break;

        case (CAN_200KBPS):                                             // 200Kbps
          cfg1 = MCP_8MHz_200kBPS_CFG1;
          cfg2 = MCP_8MHz_200kBPS_CFG2;
          cfg3 = MCP_8MHz_200kBPS_CFG3;
          break;

        case (CAN_250KBPS):                                             // 250Kbps
          cfg1 = MCP_8MHz_250kBPS_CFG1;
          cfg2 = MCP_8MHz_250kBPS_CFG2;
          cfg3 = MCP_8MHz_250kBPS_CFG3;
          break;

        case (CAN_500KBPS):                                             // 500Kbps
          cfg1 = MCP_8MHz_500kBPS_CFG1;
          cfg2 = MCP_8MHz_500kBPS_CFG2;
          cfg3 = MCP_8MHz_500kBPS_CFG3;
          break;

        case (CAN_1000KBPS):                                            //   1Mbps
          cfg1 = MCP_8MHz_1000kBPS_CFG1;
          cfg2 = MCP_8MHz_1000kBPS_CFG2;
          cfg3 = MCP_8MHz_1000kBPS_CFG3;
          break;

        default:
          set = 0;
          break;
      }
      break;

    case (MCP_16MHZ):
      switch (canSpeed)
      {
        case (CAN_5KBPS):                                               //   5Kbps
          cfg1 = MCP_16MHz_5kBPS_CFG1;
          cfg2 = MCP_16MHz_5kBPS_CFG2;
          cfg3 = MCP_16MHz_5kBPS_CFG3;
          break;

        case (CAN_10KBPS):                                              //  10Kbps
          cfg1 = MCP_16MHz_10kBPS_CFG1;
          cfg2 = MCP_16MHz_10kBPS_CFG2;
          cfg3 = MCP_16MHz_10kBPS_CFG3;
          break;

        case (CAN_20KBPS):                                              //  20Kbps
          cfg1 = MCP_16MHz_20kBPS_CFG1;
          cfg2 = MCP_16MHz_20kBPS_CFG2;
          cfg3 = MCP_16MHz_20kBPS_CFG3;
          break;

        case (CAN_40KBPS):                                              //  40Kbps
          cfg1 = MCP_16MHz_40kBPS_CFG1;
          cfg2 = MCP_16MHz_40kBPS_CFG2;
          cfg3 = MCP_16MHz_40kBPS_CFG3;
          break;

        case (CAN_50KBPS):                                              //  50Kbps
          cfg2 = MCP_16MHz_50kBPS_CFG2;
          cfg3 = MCP_16MHz_50kBPS_CFG3;
          break;

        case (CAN_80KBPS):                                              //  80Kbps
          cfg1 = MCP_16MHz_80kBPS_CFG1;
          cfg2 = MCP_16MHz_80kBPS_CFG2;
          cfg3 = MCP_16MHz_80kBPS_CFG3;
          break;

        case (CAN_100KBPS):                                             // 100Kbps
          cfg1 = MCP_16MHz_100kBPS_CFG1;
          cfg2 = MCP_16MHz_100kBPS_CFG2;
          cfg3 = MCP_16MHz_100kBPS_CFG3;
          break;

        case (CAN_125KBPS):                                             // 125Kbps
          cfg1 = MCP_16MHz_125kBPS_CFG1;
          cfg2 = MCP_16MHz_125kBPS_CFG2;
          cfg3 = MCP_16MHz_125kBPS_CFG3;
          break;

        case (CAN_200KBPS):                                             // 200Kbps
          cfg1 = MCP_16MHz_200kBPS_CFG1;
          cfg2 = MCP_16MHz_200kBPS_CFG2;
          cfg3 = MCP_16MHz_200kBPS_CFG3;
          break;

        case (CAN_250KBPS):                                             // 250Kbps
          cfg1 = MCP_16MHz_250kBPS_CFG1;
          cfg2 = MCP_16MHz_250kBPS_CFG2;
          cfg3 = MCP_16MHz_250kBPS_CFG3;
          break;

        case (CAN_500KBPS):                                             // 500Kbps
          cfg1 = MCP_16MHz_500kBPS_CFG1;
          cfg2 = MCP_16MHz_500kBPS_CFG2;
          cfg3 = MCP_16MHz_500kBPS_CFG3;
          break;

        case (CAN_1000KBPS):                                            //   1Mbps
          cfg1 = MCP_16MHz_1000kBPS_CFG1;
          cfg2 = MCP_16MHz_1000kBPS_CFG2;
          cfg3 = MCP_16MHz_1000kBPS_CFG3;
          break;

        default:
          set = 0;
          break;
      }
      break;

    case (MCP_20MHZ):
      switch (canSpeed)
      {
        case (CAN_40KBPS):                                              //  40Kbps
          cfg1 = MCP_20MHz_40kBPS_CFG1;
          cfg2 = MCP_20MHz_40kBPS_CFG2;
          cfg3 = MCP_20MHz_40kBPS_CFG3;
          break;

        case (CAN_50KBPS):                                              //  50Kbps
          cfg1 = MCP_20MHz_50kBPS_CFG1;
          cfg2 = MCP_20MHz_50kBPS_CFG2;
          cfg3 = MCP_20MHz_50kBPS_CFG3;
          break;

        case (CAN_80KBPS):                                              //  80Kbps
          cfg1 = MCP_20MHz_80kBPS_CFG1;
          cfg2 = MCP_20MHz_80kBPS_CFG2;
          cfg3 = MCP_20MHz_80kBPS_CFG3;
          break;

        case (CAN_100KBPS):                                             // 100Kbps
          cfg1 = MCP_20MHz_100kBPS_CFG1;
          cfg2 = MCP_20MHz_100kBPS_CFG2;
          cfg3 = MCP_20MHz_100kBPS_CFG3;
          break;

        case (CAN_125KBPS):                                             // 125Kbps
          cfg1 = MCP_20MHz_125kBPS_CFG1;
          cfg2 = MCP_20MHz_125kBPS_CFG2;
          cfg3 = MCP_20MHz_125kBPS_CFG3;
          break;

        case (CAN_200KBPS):                                             // 200Kbps
          cfg1 = MCP_20MHz_200kBPS_CFG1;
          cfg2 = MCP_20MHz_200kBPS_CFG2;
          cfg3 = MCP_20MHz_200kBPS_CFG3;
          break;

        case (CAN_250KBPS):                                             // 250Kbps
          cfg1 = MCP_20MHz_250kBPS_CFG1;
          cfg2 = MCP_20MHz_250kBPS_CFG2;
          cfg3 = MCP_20MHz_250kBPS_CFG3;
          break;

        case (CAN_500KBPS):                                             // 500Kbps
          cfg1 = MCP_20MHz_500kBPS_CFG1;
          cfg2 = MCP_20MHz_500kBPS_CFG2;
          cfg3 = MCP_20MHz_500kBPS_CFG3;
          break;

        case (CAN_1000KBPS):                                            //   1Mbps
          cfg1 = MCP_20MHz_1000kBPS_CFG1;
          cfg2 = MCP_20MHz_1000kBPS_CFG2;
          cfg3 = MCP_20MHz_1000kBPS_CFG3;
          break;

        default:
          set = 0;
          break;
      }
      break;

    default:
      set = 0;
      break;
  }

  if (set) {
    mcp2515_setRegister(MCP_CNF1, cfg1);
    mcp2515_setRegister(MCP_CNF2, cfg2);
    mcp2515_setRegister(MCP_CNF3, cfg3);
    return true;
  }

  return false;
}

/*********************************************************************************************************
** Function name:           mcp2515_initCANBuffers
** Descriptions:            Initialize Buffers, Masks, and Filters
*********************************************************************************************************/
void PumaCAN::mcp2515_initCANBuffers(void)
{
  uint8_t i, a1, a2, a3;

  uint8_t std = 0;
  uint8_t ext = 1;
  uint32_t ulMask = 0x00, ulFilt = 0x00;

  mcp2515_write_mf(MCP_RXM0SIDH, ext, ulMask);			/*Set both masks to 0           */
  mcp2515_write_mf(MCP_RXM1SIDH, ext, ulMask);			/*Mask register ignores ext bit */

  /* Set all filters to 0         */
  mcp2515_write_mf(MCP_RXF0SIDH, ext, ulFilt);			/* RXB0: extended               */
  mcp2515_write_mf(MCP_RXF1SIDH, std, ulFilt);			/* RXB1: standard               */
  mcp2515_write_mf(MCP_RXF2SIDH, ext, ulFilt);			/* RXB2: extended               */
  mcp2515_write_mf(MCP_RXF3SIDH, std, ulFilt);			/* RXB3: standard               */
  mcp2515_write_mf(MCP_RXF4SIDH, ext, ulFilt);
  mcp2515_write_mf(MCP_RXF5SIDH, std, ulFilt);

  /* Clear, deactivate the three  */
  /* transmit buffers             */
  /* TXBnCTRL -> TXBnD7           */
  a1 = MCP_TXB0CTRL;
  a2 = MCP_TXB1CTRL;
  a3 = MCP_TXB2CTRL;
  for (i = 0; i < 14; i++) {                                          /* in-buffer loop               */
    mcp2515_setRegister(a1, 0);
    mcp2515_setRegister(a2, 0);
    mcp2515_setRegister(a3, 0);
    a1++;
    a2++;
    a3++;
  }
  mcp2515_setRegister(MCP_RXB0CTRL, 0);
  mcp2515_setRegister(MCP_RXB1CTRL, 0);
}

/*********************************************************************************************************
** Function name:           mcp2515_init
** Descriptions:            Initialize the controller
*********************************************************************************************************/
bool PumaCAN::mcp2515_init(const uint8_t canIDMode, const CAN_SPEED canSpeed, const CAN_CLOCK canClock)
{
  softReset();

  m_curMode = MCP_LOOPBACK;

  if (!mcp2515_setCANCTRL_Mode(MCP_CONFIG)) {
#if CAN_DEBUG
    Serial.print("Entering Configuration Mode Failure...\r\n");
#endif
    return false;
  }
  
#if CAN_DEBUG
  Serial.print("Entering Configuration Mode Successful!!!\r\n");
#endif

  /* set baudrate                 */
  if (!mcp2515_configRate(canSpeed, canClock)) {
#if CAN_DEBUG
    Serial.print("Setting Baudrate Failure...\r\n");
#endif
    return false;
  }
  
#if CAN_DEBUG
  Serial.print("Setting Baudrate Successful!!!\r\n");
#endif

    /* init canbuffers              */
    mcp2515_initCANBuffers();

    /* interrupt mode               */
    mcp2515_setRegister(MCP_CANINTE, MCP_RX0IF | MCP_RX1IF);

    switch (canIDMode)
    {
      case (MCP_ANY):
        mcp2515_modifyRegister(MCP_RXB0CTRL,
                               MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
                               MCP_RXB_RX_ANY | MCP_RXB_BUKT_MASK);
        mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK,
                               MCP_RXB_RX_ANY);
        break;

      case (MCP_STD):
        mcp2515_modifyRegister(MCP_RXB0CTRL,
                               MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
                               MCP_RXB_RX_STD | MCP_RXB_BUKT_MASK );
        mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK,
                               MCP_RXB_RX_STD);
        break;

      case (MCP_EXT):
        mcp2515_modifyRegister(MCP_RXB0CTRL,
                               MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
                               MCP_RXB_RX_EXT | MCP_RXB_BUKT_MASK );
        mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK,
                               MCP_RXB_RX_EXT);
        break;

      case (MCP_STDEXT):
        mcp2515_modifyRegister(MCP_RXB0CTRL,
                               MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
                               MCP_RXB_RX_STDEXT | MCP_RXB_BUKT_MASK );
        mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK,
                               MCP_RXB_RX_STDEXT);
        break;

      default:
#if CAN_DEBUG
        Serial.print("`Setting ID Mode Failure...\r\n");
#endif
        return false;
        break;
    }


    if (!mcp2515_setCANCTRL_Mode(m_curMode)) {
#if CAN_DEBUG
      Serial.print("Returning to Previous Mode Failure...\r\n");
#endif
      return false;

  }
  return true;
}

/*********************************************************************************************************
** Function name:           mcp2515_write_id
** Descriptions:            Write CAN ID
*********************************************************************************************************/
void PumaCAN::mcp2515_write_id( const uint8_t mcp_addr, const uint8_t ext, const uint32_t id )
{
  uint16_t canid;
  uint8_t tbufdata[4];

  canid = (uint16_t)(id & 0x0FFFF);

  if ( ext == 1)
  {
    tbufdata[MCP_EID0] = (uint8_t) (canid & 0xFF);
    tbufdata[MCP_EID8] = (uint8_t) (canid >> 8);
    canid = (uint16_t)(id >> 16);
    tbufdata[MCP_SIDL] = (uint8_t) (canid & 0x03);
    tbufdata[MCP_SIDL] += (uint8_t) ((canid & 0x1C) << 3);
    tbufdata[MCP_SIDL] |= MCP_TXB_EXIDE_M;
    tbufdata[MCP_SIDH] = (uint8_t) (canid >> 5 );
  }
  else
  {
    tbufdata[MCP_SIDH] = (uint8_t) (canid >> 3 );
    tbufdata[MCP_SIDL] = (uint8_t) ((canid & 0x07 ) << 5);
    tbufdata[MCP_EID0] = 0;
    tbufdata[MCP_EID8] = 0;
  }

  mcp2515_setRegisterS( mcp_addr, tbufdata, 4 );
}

/*********************************************************************************************************
** Function name:           mcp2515_write_mf
** Descriptions:            Write Masks and Filters
*********************************************************************************************************/
void PumaCAN::mcp2515_write_mf( const uint8_t mcp_addr, const uint8_t ext, const uint32_t id )
{
  uint16_t canid;
  uint8_t tbufdata[4];
  canid = (uint16_t)(id & 0x0FFFF);
  
  if ( ext == 1)
  {
    tbufdata[MCP_EID0] = (uint8_t) (canid & 0xFF);
    tbufdata[MCP_EID8] = (uint8_t) (canid >> 8);
    canid = (uint16_t)(id >> 16);
    tbufdata[MCP_SIDL] = (uint8_t) (canid & 0x03);
    tbufdata[MCP_SIDL] += (uint8_t) ((canid & 0x1C) << 3);
    tbufdata[MCP_SIDL] |= MCP_TXB_EXIDE_M;
    tbufdata[MCP_SIDH] = (uint8_t) (canid >> 5 );
  }
  else
  {
    tbufdata[MCP_EID0] = (uint8_t) (canid & 0xFF);
    tbufdata[MCP_EID8] = (uint8_t) (canid >> 8);
    canid = (uint16_t)(id >> 16);
    tbufdata[MCP_SIDL] = (uint8_t) ((canid & 0x07) << 5);
    tbufdata[MCP_SIDH] = (uint8_t) (canid >> 3 );
  }

  mcp2515_setRegisterS( mcp_addr, tbufdata, 4 );
}

/*********************************************************************************************************
** Function name:           mcp2515_read_id
** Descriptions:            Read CAN ID
*********************************************************************************************************/
void PumaCAN::mcp2515_read_id( const uint8_t mcp_addr, uint8_t* ext, uint32_t* id )
{
  uint8_t tbufdata[4];

  *ext = 0;
  *id = 0;

  mcp2515_readRegisterS( mcp_addr, tbufdata, 4 );

  *id = (tbufdata[MCP_SIDH] << 3) + (tbufdata[MCP_SIDL] >> 5);

  if ( (tbufdata[MCP_SIDL] & MCP_TXB_EXIDE_M) ==  MCP_TXB_EXIDE_M )
  {
    /* extended id                  */
    *id = (*id << 2) + (tbufdata[MCP_SIDL] & 0x03);
    *id = (*id << 8) + tbufdata[MCP_EID8];
    *id = (*id << 8) + tbufdata[MCP_EID0];
    *ext = 1;
  }
}

/*********************************************************************************************************
** Function name:           mcp2515_write_canMsg
** Descriptions:            Write message
*********************************************************************************************************/
void PumaCAN::mcp2515_write_canMsg(const uint8_t buffer_sidh_addr, CAN_Frame message)
{
  uint8_t mcp_addr;
  
  mcp_addr = buffer_sidh_addr;
  mcp2515_setRegisterS(mcp_addr + 5, message.m_data, message.m_length );                /* write data bytes             */
  if ( message.m_rtr == 1)                                                   /* if RTR set bit in byte       */
  {
    message.m_length |= MCP_RTR_MASK;
  }
  mcp2515_setRegister((mcp_addr + 4), message.m_length );                       /* write the RTR and DLC        */
  mcp2515_write_id(mcp_addr, message.m_extended, message.m_id );                      /* write CAN id                 */

}

/*********************************************************************************************************
** Function name:           mcp2515_read_canMsg
** Descriptions:            Read message
*********************************************************************************************************/
void PumaCAN::mcp2515_read_canMsg( const uint8_t buffer_sidh_addr, CAN_Frame *message)        /* read can msg                 */
{
  uint8_t mcp_addr, ctrl;

  mcp_addr = buffer_sidh_addr;

  mcp2515_read_id( mcp_addr, &message->m_extended, &message->m_id );

  ctrl = mcp2515_readRegister( mcp_addr - 1 );
  message->m_length = mcp2515_readRegister( mcp_addr + 4 );

  if ((ctrl & 0x08)) {
    message->m_rtr = 1;
  }
  else {
    message->m_rtr = 0;
  }

  message->m_length &= MCP_DLC_MASK;
  mcp2515_readRegisterS( mcp_addr + 5, &(message->m_data[0]), message->m_length );
}

/*********************************************************************************************************
** Function name:           mcp2515_getNextFreeTXBuf
** Descriptions:            Send message
*********************************************************************************************************/
uint8_t PumaCAN::mcp2515_getNextFreeTXBuf(uint8_t *txbuf_n)                 /* get Next free txbuf          */
{
  uint8_t i, ctrlval;
  uint8_t ctrlregs[MCP_N_TXBUFFERS] = { MCP_TXB0CTRL, MCP_TXB1CTRL, MCP_TXB2CTRL };

  *txbuf_n = 0x00;

  /* check all 3 TX-Buffers       */
  for (i = 0; i < MCP_N_TXBUFFERS; i++) {
    ctrlval = mcp2515_readRegister( ctrlregs[i] );
    if ( (ctrlval & MCP_TXB_TXREQ_M) == 0 ) {
      *txbuf_n = ctrlregs[i] + 1;                                 /* return SIDH-address of Buffer*/
      return true;
    }
  }
  return false;
}





