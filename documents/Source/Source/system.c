//=============================================================================
//    S E N S I R I O N   AG,  Laubisruetistr. 50, CH-8712 Staefa, Switzerland
//=============================================================================
// Project   :  SCD30 Sample Code (V1.1)
// File      :  system.c (V1.1)
// Author    :  PFR
// Date      :  29-August-2015
// Controller:  STM32F100RB
// IDE       :  µVision V5.12.0.0
// Compiler  :  Armcc
// Brief     :  System functions
//=============================================================================

//-- Includes -----------------------------------------------------------------
#include "system.h"

//-----------------------------------------------------------------------------
void SystemInit(void) 
{
  // no initialization required
}

//-----------------------------------------------------------------------------
void DelayMicroSeconds(u32t nbrOfUs)   /* -- adapt this delay for your uC -- */
{
  u32t i;
  for(i = 0; i < nbrOfUs; i++)
  {  
    __nop();  // nop's may be added or removed for timing adjustment
    __nop();
    __nop();
    __nop();
  }
}
