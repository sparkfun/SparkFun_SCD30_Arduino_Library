//=============================================================================
//    S E N S I R I O N   AG,  Laubisruetistr. 50, CH-8712 Staefa, Switzerland
//=============================================================================
// Project   :  SCD30 Sample Code (V1.1)
// File      :  SCD30.c (V1.1)
// Author    :  PFR
// Date      :  30-August-2017
// Controller:  STM32F100RB
// IDE       :  µVision V5.12.0.0
// Compiler  :  Armcc
// Brief     :  Sensor Layer: Implementation of functions for sensor access.
//=============================================================================

//-- Includes -----------------------------------------------------------------
#include "SCD30.h"
#include "i2c_hal.h"

//-- Defines ------------------------------------------------------------------
// Generator polynomial for CRC
#define POLYNOMIAL  0x131 // P(x) = x^8 + x^5 + x^4 + 1 = 100110001

//=============================================================================
// IO-Pins                            /* -- adapt the defines for your uC -- */
//-----------------------------------------------------------------------------
//=============================================================================

//-- Global variables ---------------------------------------------------------
static u8t _i2cAddress; // I2C Address
static u32t _timeoutI2c; // clockstretching timeout

//-- Static function prototypes -----------------------------------------------
static etError SCD30_StartWriteAccess(void);
static etError SCD30_StartReadAccess(void);
static void SCD30_StopAccess(void);
static etError SCD30_WriteCommand(etCommands command);
static etError SCD30_Read2BytesAndCrc(u16t* data, etI2cAck finaleAckNack,
                                      u8t timeout);
static etError SCD30_Write2BytesAndCrc(u16t data);
static u8t SCD30_CalcCrc(u8t data[], u8t nbrOfBytes);
static etError SCD30_CheckCrc(u8t data[], u8t nbrOfBytes, u8t checksum);

static etError SCD30_ReadAndRemoveCrc(u8t data[], u32t noOfBytesToRead);

//-----------------------------------------------------------------------------
void SCD30_Init(u8t i2cAddress, u32t timeoutI2c)          /* -- adapt the init for your uC -- */
{
  // init I/O-pins
  RCC->APB2ENR |= 0x00000008;  // I/O port B clock enabled
   
  I2c_Init(); // init I2C
  SCD30_SetI2cAdr(i2cAddress);
  SCD30_SetI2cTimeout(timeoutI2c);
}

//-----------------------------------------------------------------------------
void SCD30_SetI2cAdr(u8t i2cAddress)
{
  _i2cAddress = i2cAddress;
}

//-----------------------------------------------------------------------------
void SCD30_SetI2cTimeout(u32t timeout)
{
  _timeoutI2c = timeout;
}



//-----------------------------------------------------------------------------
static etError SCD30_StartWriteAccess(void)
{
  etError error; // error code
  
  I2c_StartCondition();
  error  = I2c_WriteByte(_i2cAddress<<1, _timeoutI2c);
  
  return error;
}


static etError SCD30_StartReadAccess(void)
{
  etError error; // error code
  
  I2c_StartCondition();
  error  = I2c_WriteByte(_i2cAddress<<1 | 0x01, _timeoutI2c);
  
  return error;
}


static void SCD30_StopAccess(void)
{
  I2c_StopCondition();
}




//-----------------------------------------------------------------------------
etError SCD30_ReadSerialNumber(char* serialNumber)
{
  etError error; // error code
  
  error = SCD30_StartWriteAccess();
  // write "read serial number" command
  if(error == NO_ERROR) error = SCD30_WriteCommand(CMD_READ_SERIALNBR);
  SCD30_StopAccess();

  if(error == NO_ERROR) error = SCD30_StartReadAccess();
  // serial number is stored as char string. Two bytes are always followed by a checksum
  SCD30_ReadAndRemoveCrc((u8t*)serialNumber, 32);
  SCD30_StopAccess();
  
  // make sure last byte is '\0'
  serialNumber[31] = '\0';
  
  return error;
}


//-----------------------------------------------------------------------------
etError SCD30_ReadArticleCode(char* articleCode)
{
  etError error; // error code
  
  error = SCD30_StartWriteAccess();
  // write "read article code" command
  if(error == NO_ERROR) error = SCD30_WriteCommand(CMD_READ_ARTICLECODE);
  SCD30_StopAccess();

  if(error == NO_ERROR) error = SCD30_StartReadAccess();
  // article code is stored as char string. Two bytes are always followed by a checksum
  if(error == NO_ERROR)error = SCD30_ReadAndRemoveCrc((u8t*)articleCode, 32);
  SCD30_StopAccess();
  
  // make sure last byte is '\0'
  articleCode[31] = '\0';
  
  return error;
}


//-----------------------------------------------------------------------------
etError SCD30_GetDataReady(u16t* dataReady)
{
  etError error; // error code
  u8t buffer[2];
  
  error = SCD30_StartWriteAccess();
  // write "get data ready" command
  if(error == NO_ERROR) error = SCD30_WriteCommand(CMD_DATA_READY);
  SCD30_StopAccess();
  
  if(error == NO_ERROR) error = SCD30_StartReadAccess();
  // read 2 bytes plus CRC
  if(error == NO_ERROR) error = SCD30_ReadAndRemoveCrc(buffer, 2);
  SCD30_StopAccess();
  
  *dataReady = (((u16t)buffer[0])<<8) | buffer[1];

  return error;
}



//-----------------------------------------------------------------------------
etError SCD30_StartPeriodicMeasurment(void)
{
  etError error; // error code
  
  error = SCD30_StartWriteAccess();
  // write "start continuous measurement" command
  if(error == NO_ERROR) error = SCD30_WriteCommand(CMD_START_CONT_MEAS);
  
  // write payload 0x0000
  if(error == NO_ERROR) error = SCD30_Write2BytesAndCrc(0x0000);
  
  SCD30_StopAccess();

  return error;
}


//-----------------------------------------------------------------------------
etError SCD30_StartSingleMeasurment(void)
{
  etError error; // error code
  
  error = SCD30_StartWriteAccess();
  // write "start single measurement" command
  if(error == NO_ERROR) error = SCD30_WriteCommand(CMD_START_SINGLE_MEAS);
  
  // write payload 0x0000
  if(error == NO_ERROR) error = SCD30_Write2BytesAndCrc(0x0000);
  
  SCD30_StopAccess();

  return error;
}


//-----------------------------------------------------------------------------
etError SCD30_StopMeasurement(void)
{
  etError error; // error code
  
  error = SCD30_StartWriteAccess();
  // write "stop measurement" command
  if(error == NO_ERROR) error = SCD30_WriteCommand(CMD_STOP_MEAS);
  SCD30_StopAccess();

  return error;
}


//-----------------------------------------------------------------------------
etError SCD30_SetMeasurementInterval(u16t intervalSec)
{
  etError error; // error code

  error = SCD30_StartWriteAccess();
  // write "get data ready" command
  if(error == NO_ERROR) error = SCD30_WriteCommand(CMD_SET_MEAS_INTVAL);

  // write interval with CRC
  SCD30_Write2BytesAndCrc(intervalSec);
   
  
  SCD30_StopAccess();
  
  return error;
}

//-----------------------------------------------------------------------------
etError SCD30_ReadMeasurementBuffer(ft* co2Concentration, ft* temperature, ft* humidity)
{
  etError error; // error code
  u8t buffer[18];
  u32t tempU32;
  
  error = SCD30_StartWriteAccess();
  // write "get data ready" command
  if(error == NO_ERROR) error = SCD30_WriteCommand(CMD_READ_MEAS_BUFFER);
  SCD30_StopAccess();
  
  SCD30_StartReadAccess();
  // read 3*4 bytes plus 6*CRC
  if(error == NO_ERROR) error = SCD30_ReadAndRemoveCrc(buffer, 12);
  SCD30_StopAccess();
  
  // cast to floats
  tempU32 = (u32t)((((u32t)buffer[0]) << 24) | (((u32t)buffer[1]) << 16) | (((u32t)buffer[2]) << 8) | ((u32t)buffer[3]));
  *co2Concentration = *(float*)&tempU32;
  tempU32 = (u32t)((((u32t)buffer[4]) << 24) | (((u32t)buffer[5]) << 16) | (((u32t)buffer[6]) << 8) | ((u32t)buffer[7]));
  *temperature = *(float*)&tempU32;
  tempU32 = (u32t)((((u32t)buffer[8]) << 24) | (((u32t)buffer[9]) << 16) | (((u32t)buffer[10]) << 8) | ((u32t)buffer[11]));
  *humidity = *(float*)&tempU32;
  return error;
  
}

//-----------------------------------------------------------------------------
static etError SCD30_WriteCommand(etCommands command)
{
  etError error; // error code
  
  // write the upper 8 bits of the command to the sensor
  if(error == NO_ERROR) error  = I2c_WriteByte(command >> 8, _timeoutI2c);

  // write the lower 8 bits of the command to the sensor
  if(error == NO_ERROR) error |= I2c_WriteByte(command & 0xFF, _timeoutI2c);
 
  return error;
}

//-----------------------------------------------------------------------------
static etError SCD30_Read2BytesAndCrc(u16t* data, etI2cAck finaleAckNack,
                                      u8t timeout)
{
  etError error;    // error code
  u8t     bytes[2]; // read data array
  u8t     checksum; // checksum byte
 
  // read two data bytes and one checksum byte
                        error = I2c_ReadByte(&bytes[0], ACK, timeout);
  if(error == NO_ERROR) error = I2c_ReadByte(&bytes[1], ACK, timeout);
  if(error == NO_ERROR) error = I2c_ReadByte(&checksum, finaleAckNack, timeout);
  
  // verify checksum
  if(error == NO_ERROR) error = SCD30_CheckCrc(bytes, 2, checksum);
  
  // combine the two bytes to a 16-bit value
  *data = (bytes[0] << 8) | bytes[1];
  
  return error;
}

//-----------------------------------------------------------------------------
static etError SCD30_Write2BytesAndCrc(u16t data)
{
  etError error;    // error code
  u8t     bytes[2]; // read data array
  u8t     checksum; // checksum byte
  
  bytes[0] = data >> 8;
  bytes[1] = data & 0xFF;
  checksum = SCD30_CalcCrc(bytes, 2);
 
  // write two data bytes and one checksum byte
                        error = I2c_WriteByte(bytes[0], _timeoutI2c); // write data MSB
  if(error == NO_ERROR) error = I2c_WriteByte(bytes[1], _timeoutI2c); // write data LSB
  if(error == NO_ERROR) error = I2c_WriteByte(checksum, _timeoutI2c); // write checksum
  
  return error;
}

//-----------------------------------------------------------------------------
etError SCD30_ReadAndRemoveCrc(u8t data[], u32t noOfBytesToRead)
{
  etError error = NO_ERROR; // error code
  u16t word;
  u32t byteCounter = 0;
  u32t writeIndex = 0;
  noOfBytesToRead = (noOfBytesToRead+1)/2*3; // must be multiple of two plus one byte crc per word
  
  
  while(byteCounter<noOfBytesToRead)
  {
    if (byteCounter <noOfBytesToRead-3) 
      error = SCD30_Read2BytesAndCrc(&word, ACK, _timeoutI2c);
    else
      error = SCD30_Read2BytesAndCrc(&word,NACK, _timeoutI2c); // send nack for last byte
    
    data[writeIndex++] = (u8t)((word >> 8) & 0x00FF);
    data[writeIndex++] = (u8t)(word & 0x00FF);
   
    byteCounter += 3;
  }
   
  return error;
}


//-----------------------------------------------------------------------------
static u8t SCD30_CalcCrc(u8t data[], u8t nbrOfBytes)
{
  u8t bit;        // bit mask
  u8t crc = 0xFF; // calculated checksum
  u8t byteCtr;    // byte counter
  
  // calculates 8-Bit checksum with given polynomial
  for(byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++)
  {
    crc ^= (data[byteCtr]);
    for(bit = 8; bit > 0; --bit)
    {
      if(crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
      else           crc = (crc << 1);
    }
  }
  
  return crc;
}

//-----------------------------------------------------------------------------
static etError SCD30_CheckCrc(u8t data[], u8t nbrOfBytes, u8t checksum)
{
  u8t crc;     // calculated checksum
  
  // calculates 8-Bit checksum
  crc = SCD30_CalcCrc(data, nbrOfBytes);
  
  // verify checksum
  if(crc != checksum) return CHECKSUM_ERROR;
  else                return NO_ERROR;
}
