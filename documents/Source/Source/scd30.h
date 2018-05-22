//=============================================================================
//    S E N S I R I O N   AG,  Laubisruetistr. 50, CH-8712 Staefa, Switzerland
//=============================================================================
// Project   :  SCD30 Sample Code (V1.1)
// File      :  SCD30.h (V1.1)
// Author    :  PFR
// Date      :  29-August-2015
// Controller:  STM32F100RB
// IDE       :  µVision V5.12.0.0
// Compiler  :  Armcc
// Brief     :  Sensor Layer: Definitions of commands and functions for sensor
//                            access.
//=============================================================================

#ifndef SCD30_H
#define SCD30_H

//-- Includes -----------------------------------------------------------------
#include "system.h"
#include "i2c_hal.h"

//-- Enumerations -------------------------------------------------------------
// Sensor Commands
typedef enum{
  CMD_READ_SERIALNBR    = 0xD033, // read serial number
  CMD_READ_ARTICLECODE  = 0xD025, // read article code
  CMD_START_SINGLE_MEAS = 0x0006, // starts a single shot measurement
  CMD_START_CONT_MEAS   = 0x0010,	// starts a continuous measurement
  CMD_STOP_MEAS         = 0x0104, // stops continuous measurement
  CMD_DATA_READY        = 0x0202, // reads the data ready register
  CMD_READ_MEAS_BUFFER  = 0x0300, // reads the measurement buffer
  CMD_SET_MEAS_INTVAL   = 0x4600, // sets the measurement interval
  
}etCommands;


//=============================================================================
// Initializes the I2C bus for communication with the sensor.
//-----------------------------------------------------------------------------
// input: i2cAddress    I2C address, 0x44 ADDR pin low / 0x45 ADDR pin high
// input: timeoutI2c    Timeout in miliseconds for I2C clockstretching
//-----------------------------------------------------------------------------
void SCD30_Init(u8t i2cAddress, u32t timeoutI2c);


//=============================================================================
// Sets the I2C address.
//-----------------------------------------------------------------------------
// input: i2cAddress    I2C address, 0x44 ADDR pin low / 0x45 ADDR pin high
//-----------------------------------------------------------------------------
void SCD30_SetI2cAdr(u8t i2cAddress);

//=============================================================================
// Sets I2C clockstretching timeout
//-----------------------------------------------------------------------------
// input: timeoutI2c    Timeout in miliseconds for I2C clockstretching
//-----------------------------------------------------------------------------
void SCD30_SetI2cTimeout(u32t timeout);

//=============================================================================
// Reads the serial number from sensor.
//-----------------------------------------------------------------------------
// input: serialNumber  pointer to serialNumber
//
// return: error:       ACK_ERROR      = no acknowledgment from sensor
//                      CHECKSUM_ERROR = checksum mismatch
//                      TIMEOUT_ERROR  = timeout
//                      NO_ERROR       = no error
//-----------------------------------------------------------------------------
etError SCD30_ReadSerialNumber(char* serialNumber);


//=============================================================================
// Reads the article from sensor.
//-----------------------------------------------------------------------------
// input: articleCode   pointer to articleCode
//
// return: error:       ACK_ERROR      = no acknowledgment from sensor
//                      CHECKSUM_ERROR = checksum mismatch
//                      TIMEOUT_ERROR  = timeout
//                      NO_ERROR       = no error
//-----------------------------------------------------------------------------
etError SCD30_ReadArticleCode(char* articleCode);


//=============================================================================
// Reads the number of ready measurements available in the measurement buffer.
//-----------------------------------------------------------------------------
// input: dataReady     pointer to dataReady
//                      1 -> Measurement available in buffer
//                      0 -> No Measurement available in buffer        
//
// return: error:       ACK_ERROR      = no acknowledgment from sensor
//                      CHECKSUM_ERROR = checksum mismatch
//                      TIMEOUT_ERROR  = timeout
//                      NO_ERROR       = no error
//-----------------------------------------------------------------------------
etError SCD30_GetDataReady(u16t* dataReady);



//=============================================================================
// Starts periodic measurement.
//-----------------------------------------------------------------------------
//
// return: error:       ACK_ERROR      = no acknowledgment from sensor
//                      CHECKSUM_ERROR = checksum mismatch
//                      TIMEOUT_ERROR  = timeout
//                      PARM_ERROR     = parameter out of range
//                      NO_ERROR       = no error
//-----------------------------------------------------------------------------
etError SCD30_StartPeriodicMeasurment(void);

//=============================================================================
// Starts single measurement.
//-----------------------------------------------------------------------------
//
// return: error:       ACK_ERROR      = no acknowledgment from sensor
//                      CHECKSUM_ERROR = checksum mismatch
//                      TIMEOUT_ERROR  = timeout
//                      PARM_ERROR     = parameter out of range
//                      NO_ERROR       = no error
//-----------------------------------------------------------------------------
etError SCD30_StartSingleMeasurment(void);

//=============================================================================
// Stops any ongoing measurement and disables the periodic mode
//-----------------------------------------------------------------------------
//
// return: error:       ACK_ERROR      = no acknowledgment from sensor
//                      CHECKSUM_ERROR = checksum mismatch
//                      TIMEOUT_ERROR  = timeout
//                      PARM_ERROR     = parameter out of range
//                      NO_ERROR       = no error
//-----------------------------------------------------------------------------
etError SCD30_StopMeasurement(void);

//=============================================================================
// Configure measurement interval in continuous mode
//-----------------------------------------------------------------------------
// input: intervalSec:  Measurement interval in seconds
// 
// return: error:       ACK_ERROR      = no acknowledgment from sensor
//                      CHECKSUM_ERROR = checksum mismatch
//                      TIMEOUT_ERROR  = timeout
//                      PARM_ERROR     = parameter out of range
//                      NO_ERROR       = no error
//-----------------------------------------------------------------------------
etError SCD30_SetMeasurementInterval(u16t intervalSec);


//=============================================================================
// Reads last measurement from the sensor buffer
//-----------------------------------------------------------------------------
// input: co2Concentration  pointer to co2Concentration [PPM]
// input: temperature       pointer to temperature [°C]
// input: humidity          pointer to humidity [%]
//
// return: error:       ACK_ERROR      = no acknowledgment from sensor
//                      CHECKSUM_ERROR = checksum mismatch
//                      TIMEOUT_ERROR  = timeout
//                      NO_ERROR       = no error
//-----------------------------------------------------------------------------
etError SCD30_ReadMeasurementBuffer(ft* co2Concentration, ft* temperature, ft* humidity);



#endif
