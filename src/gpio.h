/* -----------------------------------------------------------------------------
 * @file   gpio.h
 * @brief  GPIO interfacing  
 * @author Jake Michael, jami1063@colorado.edu
 * ---------------------------------------------------------------------------*/

#ifndef _GPIO_H_
#define _GPIO_H_

#include "em_gpio.h"
#include "gpio.h"
#include "log.h"

#define TP1_PORT        (gpioPortD)
#define TP1_PIN         (10)
#define TP2_PORT        (gpioPortD)
#define TP2_PIN         (11)
#define ACCEL_INT1_PORT (gpioPortA)
#define ACCEL_INT1_PIN  (4)
#define ACCEL_INT2_PORT (gpioPortA)
#define ACCEL_INT2_PIN  (3)

void gpio_init();
void gpio_TP1_toggle();

#endif // _GPIO_H_
