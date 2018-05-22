//=============================================================================
//    S E N S I R I O N   AG,  Laubisruetistr. 50, CH-8712 Staefa, Switzerland
//=============================================================================
// Project   :  SCD30 Sample Code (V1.1)
// File      :  i2c_hal.c (V1.1)
// Author    :  PFR
// Date      :  29-August-2015
// Controller:  STM32F100RB
// IDE       :  µVision V5.12.0.0
// Compiler  :  Armcc
// Brief     :  I2C hardware abstraction layer
//=============================================================================

//-- Includes -----------------------------------------------------------------
#include "i2c_hal.h"

//-- Defines ------------------------------------------------------------------
// I2C IO-Pins                        /* -- adapt the defines for your uC -- */

// SDA on port B, bit 14
#define SDA_LOW()  (GPIOB->BSRR = 0x40000000) // set SDA to low
#define SDA_OPEN() (GPIOB->BSRR = 0x00004000) // set SDA to open-drain
#define SDA_READ   (GPIOB->IDR  & 0x4000)     // read SDA

// SCL on port B, bit 13              /* -- adapt the defines for your uC -- */
#define SCL_LOW()  (GPIOB->BSRR = 0x20000000) // set SCL to low
#define SCL_OPEN() (GPIOB->BSRR = 0x00002000) // set SCL to open-drain
#define SCL_READ   (GPIOB->IDR  & 0x2000)     // read SCL

//-- Static function prototypes -----------------------------------------------
static etError I2c_WaitWhileClockStreching(u32t timeout);

//-----------------------------------------------------------------------------
void I2c_Init(void)                      /* -- adapt the init for your uC -- */
{
  RCC->APB2ENR |= 0x00000008;  // I/O port B clock enabled
  
  SDA_OPEN();                  // I2C-bus idle mode SDA released
  SCL_OPEN();                  // I2C-bus idle mode SCL released
  
  // SDA on port B, bit 14
  // SCL on port B, bit 13
  GPIOB->CRH   &= 0xF00FFFFF;  // set open-drain output for SDA and SCL
  GPIOB->CRH   |= 0x05500000;  // 
}

//-----------------------------------------------------------------------------
void I2c_StartCondition(void)
{
  SDA_OPEN();
  DelayMicroSeconds(1);
  SCL_OPEN();
  DelayMicroSeconds(1);
  SDA_LOW();
  DelayMicroSeconds(10);  // hold time start condition (t_HD;STA)
  SCL_LOW();
  DelayMicroSeconds(10);
}

//-----------------------------------------------------------------------------
void I2c_StopCondition(void)
{
  SCL_LOW();
  DelayMicroSeconds(1);
  SDA_LOW();
  DelayMicroSeconds(1);
  SCL_READ;
  SCL_OPEN();
  DelayMicroSeconds(10);  // set-up time stop condition (t_SU;STO)
  SDA_OPEN();
  SDA_READ;
  DelayMicroSeconds(10);
}


//-----------------------------------------------------------------------------
static etError I2c_WaitWhileClockStreching(u32t timeout)
{
  etError error = NO_ERROR;
  timeout = timeout*100; // convert to 10 microsecond steps
  SCL_OPEN();
  while(SCL_READ == 0)
  {
    if(timeout-- == 0) 
      return TIMEOUT_ERROR;
    DelayMicroSeconds(10);
  }
  
  return error;
}




etError I2c_WriteBit(u8t bit, u32t timeout) 
{
  etError error = NO_ERROR;
 
  
  if (bit != 0) {
    SDA_OPEN();
    SDA_READ;
  } else {
    SDA_LOW();
  }

  error = I2c_WaitWhileClockStreching(timeout);
  
  DelayMicroSeconds(5);
  SCL_LOW();

  return error;
}

 etError I2c_ReadBit(u8t* bit, u32t timeout) 
 {
  etError error = NO_ERROR;
  SDA_OPEN();
  SDA_READ;
   
  error = I2c_WaitWhileClockStreching(timeout);
  DelayMicroSeconds(5);
  
  // SCL is high, now data is valid
  *bit = SDA_READ > 0;
  DelayMicroSeconds(5);
  SCL_LOW();
  return error;
}
 

etError I2c_WriteByte(u8t byte, u32t timeout) 
{
  etError error = NO_ERROR;
  u8t bit;
  
  for (bit = 0; bit < 8; bit++) 
  {
    error = I2c_WriteBit((byte & 0x80), timeout);
    if (error != NO_ERROR)
      break;
    byte <<= 1;
  }
  
  error = I2c_ReadBit(&bit, timeout); 
  if (bit != 0)
    error = ACK_ERROR;
  return error;
}
 

etError I2c_ReadByte(u8t* byte, etI2cAck ack, u32t timeout)
{
  etError error = NO_ERROR;
  u8t mask;
  u8t bit;
  *byte = 0;
  
  for (mask = 0; mask < 8; mask++) 
  {
    error = I2c_ReadBit(&bit, timeout);
    if (error != NO_ERROR)
      break;
    
    *byte =  *byte | (bit << (7-mask));
  }
  error = I2c_WriteBit(ack == NACK, timeout);

  return error;
}



