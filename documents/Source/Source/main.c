//=============================================================================
//    S E N S I R I O N   AG,  Laubisruetistr. 50, CH-8712 Staefa, Switzerland
//=============================================================================
// Project   :  SCD30 Sample Code (V1.1)
// File      :  main.c (V1.1)
// Author    :  PFR
// Date      :  19-March-2018
// Controller:  STM32F100RB
// IDE       :  µVision V5.12.0.0
// Compiler  :  Armcc
// Brief     :  This code shows in a minimal manner how to communicate with 
//              a SCD30 sensor.
//              Due to compatibility reasons the I2C interface is implemented
//              as "bit-banging" on normal I/O's. This code is written for an
//              easy understanding and is neither optimized for speed nor code
//              size.
//
// Porting to a different microcontroller (uC):
//   - the definitions of basic types may have to be changed  in typedefs.h
//   - adapt the led functions for your platform   in main.c
//   - adapt the port functions / definitions for your uC     in i2c_hal.h/.c
//   - adapt the timing of the delay function for your uC     in system.c
//   - adapt the SystemInit()                                 in system.c
//   - change the uC register definition file <stm32f10x.h>   in system.h
//=============================================================================

//-- Includes -----------------------------------------------------------------
#include "system.h"
#include "SCD30.h"

//-- Static function prototypes -----------------------------------------------
static void EvalBoardPower_Init(void);
static void Led_Init(void);
static void LedBlueOn(void);
static void LedBlueOff(void);
static void LedGreenOn(void);
static void LedGreenOff(void);



//-----------------------------------------------------------------------------
int main(void)
{
  etError   error = NO_ERROR;   // error code
  ft        co2Concentration;   // CO2 concentration [PPM]
  ft        temperature;        // temperature [°C]
  ft        humidity;           // relative humidty [%]
  u16t      dataReady;          // 0 -> no data in measurement buffer
                                // 1 -> data available in measurement buffer
  char serialNumber[32];        // string containing the serial number of SCD30
  char articleCode[32];         // string containing the artice code of SCD30
  u32t counter = 0;             // local variable
  
  SystemInit();
  Led_Init();
  EvalBoardPower_Init();
  
  
  SCD30_Init(0x61, 180); // SCD30 listens on address 0x61 with 180ms I2c timeout
 
    
  // wait 50ms after power on
  DelayMicroSeconds(50000);   

  // read serial number
  error = SCD30_ReadSerialNumber(serialNumber);  
  // read article code
  error = SCD30_ReadArticleCode(articleCode);
  // set periodic measurement interval to 2 seconds
  error = SCD30_SetMeasurementInterval(2);
  
  // start periodic measuments
  error = SCD30_StartPeriodicMeasurment();
  
 
  // loop forever
  while(1)
  {
    error = NO_ERROR;
    
    // loop while no error
    while(error == NO_ERROR)
    {
     
      // check if we have a measurement ready
      error = SCD30_GetDataReady(&dataReady);
      if (error != NO_ERROR)
        break;
 
      if (dataReady> 0) 
      {
        // Get measurement
        error = SCD30_ReadMeasurementBuffer(&co2Concentration, &temperature, &humidity);
        
        // flash blue led to indicate that we have a new measurement
        LedBlueOn();
      }
      
      DelayMicroSeconds(100000);
      LedBlueOff();
    }
    
    if (error != ACK_ERROR)
      error = NO_ERROR;
    
    // in case of an error ... 
    for (counter = 0; counter < 5; counter++)
    {
      LedGreenOn();
      DelayMicroSeconds(50000);
      LedGreenOff();
      DelayMicroSeconds(50000);
    }
    
  }
}



//-----------------------------------------------------------------------------
static void EvalBoardPower_Init(void)    /* -- adapt this code for your platform -- */
{
  RCC->APB2ENR |= 0x00000008;  // I/O port B clock enabled
  
  GPIOB->CRH   &= 0x0FFF0FFF;  // set push-pull output for Vdd & GND pins
  GPIOB->CRH   |= 0x10001000;  //
  
  GPIOB->BSRR = 0x08008000;    // set Vdd to High, set GND to Low
}

//-----------------------------------------------------------------------------
static void Led_Init(void)               /* -- adapt this code for your platform -- */
{
  RCC->APB2ENR |= 0x00000010;  // I/O port C clock enabled
  GPIOC->CRH   &= 0xFFFFFF00;  // set general purpose output mode for LEDs
  GPIOC->CRH   |= 0x00000011;  //
  GPIOC->BSRR   = 0x03000000;  // LEDs off
}

//-----------------------------------------------------------------------------
static void LedBlueOn(void)              /* -- adapt this code for your platform -- */
{
  GPIOC->BSRR = 0x00000100;
}

//-----------------------------------------------------------------------------
static void LedBlueOff(void)             /* -- adapt this code for your platform -- */
{
  GPIOC->BSRR = 0x01000000;
}

//-----------------------------------------------------------------------------
static void LedGreenOn(void)             /* -- adapt this code for your platform -- */
{
  GPIOC->BSRR = 0x00000200;
}

//-----------------------------------------------------------------------------
static void LedGreenOff(void)            /* -- adapt this code for your platform -- */
{
  GPIOC->BSRR = 0x02000000;
}
