/* -----------------------------------------------------------------------------
 * @file   gpio.c
 * @brief  GPIO interfacing  
 * @author Jake Michael, jami1063@colorado.edu
 * ---------------------------------------------------------------------------*/

#include "em_gpio.h"
#include "gpio.h"

/* Useful routines: 
GPIO_PinOutSet
GPIO_DriveStrengthSet
GPIO_PinModeSet
*/

void gpio_init()
{
  GPIO_PinModeSet(TP1_PORT, TP1_PIN, gpioModePushPull, false);
  GPIO_PinModeSet(TP2_PORT, TP2_PIN, gpioModePushPull, false);
} // gpio_init

void gpio_TP1_toggle()
{
  GPIO_PinOutToggle(TP1_PORT, TP1_PIN);
} // gpio_TP1_toggle()
