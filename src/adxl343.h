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

#include "log.h"

// Register masks / shifts:
#define ADDRESS_MASK      (0x3F)
#define READ_SHIFT        (0x7)
#define READ_MASK         (0x1 << READ_SHIFT)
#define MULTI_BYTE_SHIFT  (0x6)
#define MULTI_BYTE_MASK   (0x1 << MULTI_BIT_SHIFT)

// Write commands:
#define SINGLE_READ_CMD(reg) ((reg & ADDRESS_MASK) | READ_MASK ) 
#define MULTI_READ_CMD(reg)  ((reg & ADDRESS_MASK) | MULTI_BIT_MASK | READ_MASK )

// Read commands:
#define SINGLE_WRITE_CMD(reg)  ((reg & ADDRESS_MASK))
#define MULTI_WRITE_CMD(reg)   ((reg & ADDRESS_MASK) | MULTI_BIT_MASK)

/* ============================================================================
 *       REGISTER MAP
 * ===========================================================================*/
#define ADXL343_DEVID            (0x00)
#define ADXL343_THRESH_TAP       (0x1D)
#define ADXL343_OFSX             (0x1E)
#define ADXL343_OFSY             (0x1F)
#define ADXL343_OFSZ             (0x20)
#define ADXL343_DUR              (0x21)
#define ADXL343_LATENT           (0x22)
#define ADXL343_WINDOW           (0x23)
#define ADXL343_THRESH_ACT       (0x24)
#define ADXL343_THRESH_INACT     (0x25)
#define ADXL343_TIME_INACT       (0x26)
#define ADXL343_ACT_INACT_CTL    (0x27)
#define ADXL343_THRESH_FF        (0x28)
#define ADXL343_TIME_FF          (0x29)
#define ADXL343_TAP_AXES         (0x2A)
#define ADXL343_ACT_TAP_STATUS   (0x2B)
#define ADXL343_BW_RATE          (0x2C)
#define ADXL343_POWER_CTL        (0x2D)
#define ADXL343_INT_ENABLE       (0x2E)
#define ADXL343_INT_MAP          (0x2F)
#define ADXL343_INT_SOURCE       (0x30)
#define ADXL343_DATA_FORMAT      (0x31)
#define ADXL343_DATAX0           (0x32)
#define ADXL343_DATAX1           (0x33)
#define ADXL343_DATAY0           (0x34)
#define ADXL343_DATAY1           (0x35)
#define ADXL343_DATAZ0           (0x36)
#define ADXL343_DATAZ1           (0x37)
#define ADXL343_FIFO_CTL         (0x38)
#define ADXL343_FIFO_STATUS      (0x39)


/* ============================================================================
 *       FUNCTION PROTOTYPES 
 * ===========================================================================*/
int accel_init();
int accel_device_id_test();
int accel_set_measurement_mode();
int accel_get_acceleration();

#endif // _ADXL343_H_
