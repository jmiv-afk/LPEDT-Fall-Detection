/* -----------------------------------------------------------------------------
 * @file adxl343.h
 * @brief Peripheral driver for the ADXL343 accelerometer via 4-wire SPI
 * @resources Relies on silabs SPIDRV (EMDRV) driver for SPI interaction
 * @author Jake Michael, jami1063@colorado.edu
 * ---------------------------------------------------------------------------*/

#ifndef _ADXL343_H_
#define  _ADXL343_H_

#include <stdbool.h>
#include <em_core.h>
#include <em_usart.h>
#include <em_gpio.h>
#include <em_cmu.h>
#include <sl_bluetooth.h>

#include "events.h"
#include "log.h"

// Register masks / shifts:
#define ADDRESS_MASK      (0x3F)
#define READ_SHIFT        (0x7)
#define READ_MASK         (0x1 << READ_SHIFT)
#define MULTI_BYTE_SHIFT  (0x6)
#define MULTI_BYTE_MASK   (0x1 << MULTI_BYTE_SHIFT)

// Write commands:
#define SINGLE_READ_CMD(reg) ((reg & ADDRESS_MASK) | READ_MASK ) 
#define MULTI_READ_CMD(reg)  ((reg & ADDRESS_MASK) | MULTI_BYTE_MASK | READ_MASK )

// Read commands:
#define SINGLE_WRITE_CMD(reg)  ((reg & ADDRESS_MASK))
#define MULTI_WRITE_CMD(reg)   ((reg & ADDRESS_MASK) | MULTI_BYTE_MASK)

/* ============================================================================
 *       REGISTER MAP
 * ===========================================================================*/
typedef enum 
{
  ADXL343_DEVID =           0x00, // device ID is 0xE5
  ADXL343_THRESH_TAP =      0x1D, // tap threshold
  ADXL343_OFSX =            0x1E, // offset x-axis
  ADXL343_OFSY =            0x1F, // offset y-axis
  ADXL343_OFSZ =            0x20, // offset z-axis
  ADXL343_DUR =             0x21, // tap duration
  ADXL343_LATENT =          0x22, // tap latency
  ADXL343_WINDOW =          0x23, // tap window
  ADXL343_THRESH_ACT =      0x24, // activity threshold
  ADXL343_THRESH_INACT =    0x25, // inactivity threshold
  ADXL343_TIME_INACT =      0x26, // inactivity time
  ADXL343_ACT_INACT_CTL =   0x27, // activity/inactivity control (axis settings)
  ADXL343_THRESH_FF =       0x28, // free-fall threshold
  ADXL343_TIME_FF =         0x29, // free-fall time
  ADXL343_TAP_AXES =        0x2A, // tap control (axis settings, r/w)
  ADXL343_ACT_TAP_STATUS =  0x2B, // tap status (read-only)
  ADXL343_BW_RATE =         0x2C, // data rate
  ADXL343_POWER_CTL =       0x2D, // power modes
  ADXL343_INT_ENABLE =      0x2E, // interrupt enable
  ADXL343_INT_MAP =         0x2F, // interrupt mapping
  ADXL343_INT_SOURCE =      0x30, // interrupt source (read-only)
  ADXL343_DATA_FORMAT =     0x31, // data formatting settings  
  ADXL343_DATAX0 =          0x32, // LSByte x-axis
  ADXL343_DATAX1 =          0x33, // MSByte x-axis
  ADXL343_DATAY0 =          0x34, // LSByte y-axis
  ADXL343_DATAY1 =          0x35, // MSByte y-axis
  ADXL343_DATAZ0 =          0x36, // LSByte z-axis
  ADXL343_DATAZ1 =          0x37, // MSByte z-axis
  ADXL343_FIFO_CTL =        0x38, // FIFO mode settings
  ADXL343_FIFO_STATUS =     0x39  // FIFO status (read-only)
} accel_register;


typedef enum 
{
  INT_DATA_READY = 0x80,
  INT_SINGLE_TAP = 0x40,
  INT_DOUBLE_TAP = 0x20,
  INT_ACTIVITY = 0x10,
  INT_INACTIVITY = 0x08,
  INT_FREE_FALL = 0x04,
  INT_WATERMARK = 0x02,
  INT_OVERRUN = 0x01
} accel_interrupts;


/* ============================================================================
 *       FUNCTION PROTOTYPES 
 * ===========================================================================*/
int accel_init();
int accel_get_acceleration();
void accel_determine_interrupt_source(uint8_t *reg);
void GPIO_EVEN_IRQHandler();

#endif // _ADXL343_H_
