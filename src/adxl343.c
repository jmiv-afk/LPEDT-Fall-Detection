/* -----------------------------------------------------------------------------
 * @file adxl343.c 
 * @brief Peripheral driver for the ADXL343 accelerometer
 * @author Jake Michael, jami1063@colorado.edu
 * ---------------------------------------------------------------------------*/

#include <stdbool.h>
#include "spidrv.h"
#include "em_gpio.h"
//#include "spidrv_config.h"
#include "ecode.h"
#include "log.h"
#include "adxl343.h"

static void accel_cs_set(bool setpt);
static uint32_t accel_read_multi(uint8_t reg, uint8_t *rx, uint32_t nbytes);
static uint32_t accel_read_register(uint8_t reg, uint8_t *rx);
static uint32_t accel_write_register(uint8_t reg, uint8_t tx);

SPIDRV_HandleData_t accel_spi_handle;

#define ACCEL_CS_PORT (gpioPortA)
#define ACCEL_CS_PIN  (5)

int accel_init()
{
  // The init structure - see datasheet pg. 169 for multiplexed location numbers
  // corresponding to the proper pin/port
  SPIDRV_Init_t spi_init = {
    .bitOrder = spidrvBitOrderMsbFirst,
    .bitRate = 1000000,
    .clockMode = spidrvClockMode3,    // Clk_Polarity=1, Clk_Phase=1
    .csControl = spidrvCsControlApplication,
    .dummyTxValue = 0x0,
    .frameLength = 8,
    .port = USART1,
    .portLocationClk = 0,  // PA2
    .portLocationCs = 2,   // PA5
    .portLocationRx = 3,   // PA4
    .portLocationTx = 3,   // PA3
    .slaveStartMode = spidrvSlaveStartImmediate,
    .type = spidrvMaster
  };

  GPIO_PinModeSet(ACCEL_CS_PORT, ACCEL_CS_PIN, gpioModePushPull, 1);
  LOG(" \n\n");
  Ecode_t status = SPIDRV_Init(&accel_spi_handle, &spi_init);
  if (status != ECODE_OK) {
    return -1;
  }
  return 0;
}

int accel_device_id_test()
{
  uint8_t ret = 0;
  accel_read_register(ADXL343_DEVID, &ret);
  LOG("Device ID returned 0x%x", ret);
  return 0;
}

int accel_set_measurement_mode() 
{
  // Power control register map:
  // D7 | D6 | D5   | D4         | D3      | D2    | D1 | D0 |
  // 0  | 0  | Link | AUTO_SLEEP | Measure | Sleep | Wakeup  |
  // set measure bit to 1 to put part in measurement mode
  LOG("Setting measurement mode to 0x8\n");
  accel_write_register(ADXL343_POWER_CTL, 0x8);
  uint8_t ret = 0xff;
  accel_read_register(ADXL343_POWER_CTL, &ret);
  LOG(" Measurement mode set to 0x%x", ret);
  return 0;
}

int accel_get()
{ 
  uint8_t rx_buf[6];
  int16_t accel[3];
  accel_read_multi(ADXL343_DATAX0, rx_buf, 6);
  for (int i=0; i<3; i++)
  {
    accel[i] = (int16_t)( (rx_buf[2*i+1] << 0x8) | rx_buf[2*i] );
    printf("%d ", accel[i]);
  }
  printf("\n");
  return 0;
}

static uint32_t accel_read_multi(uint8_t reg, uint8_t *rx, uint32_t nbytes)
{
  Ecode_t status;
  uint8_t cmd[nbytes+1];
  uint8_t recv[nbytes+1];
  
  cmd[0] = MULTI_READ_CMD(reg);
  accel_cs_set(0);
  status = SPIDRV_MTransferB(&accel_spi_handle, cmd, recv, nbytes+1);
  accel_cs_set(1);
  if (status != ECODE_OK) 
  {
    return status;
  }
  for (uint32_t i=0; i<nbytes; i++)
  {
    *(rx + i) = recv[i+1];
  }
  return 0;
}

static uint32_t accel_read_register(uint8_t reg, uint8_t *rx) 
{
  Ecode_t status;
  uint8_t cmd[2];
  uint8_t recv[2];
  cmd[0] = SINGLE_READ_CMD(reg);
  accel_cs_set(0);
  status = SPIDRV_MTransferB(&accel_spi_handle, cmd, recv, 2);
  accel_cs_set(1);
  if (status != ECODE_OK) 
  {
    return status;
  }
  *rx = recv[1];
  return 0;
}

static uint32_t accel_write_register(uint8_t reg, uint8_t tx) 
{
  uint8_t cmd[2] = {0,0};
  cmd[0] = SINGLE_WRITE_CMD(reg);
  cmd[1] = tx;
  Ecode_t status;
  accel_cs_set(0);
  status = SPIDRV_MTransmitB(&accel_spi_handle, &cmd, sizeof(cmd));
  accel_cs_set(1);
  if (status != ECODE_OK) 
  {
    return status;
  }
  return 0;
}

static void accel_cs_set(bool setpt) 
{
  if (setpt) 
  {
    GPIO_PinOutSet(ACCEL_CS_PORT, ACCEL_CS_PIN);
  }
  else 
  {
    GPIO_PinOutClear(ACCEL_CS_PORT, ACCEL_CS_PIN);
  }
}
