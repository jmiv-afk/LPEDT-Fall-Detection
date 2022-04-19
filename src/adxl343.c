/* -----------------------------------------------------------------------------
 * @file adxl343.c 
 * @brief Peripheral driver for the ADXL343 accelerometer
 * @author Jake Michael, jami1063@colorado.edu
 * ---------------------------------------------------------------------------*/

#include "adxl343.h"

#define ACCEL_CLK_PORT (gpioPortA)
#define ACCEL_CLK_PIN  (0)
#define ACCEL_CLK_LOC  (30)

#define ACCEL_TX_PORT (gpioPortA)
#define ACCEL_TX_PIN  (1)
#define ACCEL_TX_LOC  (1)

#define ACCEL_RX_PORT (gpioPortA)
#define ACCEL_RX_PIN  (2)
#define ACCEL_RX_LOC  (1)

#define ACCEL_CS_PORT (gpioPortA)
#define ACCEL_CS_PIN  (5)
#define ACCEL_CS_LOC  (2)

static uint32_t accel_read(uint8_t start_register, uint8_t *rx, uint32_t nbytes);
static uint32_t accel_write(uint8_t start_register, uint8_t *tx, uint32_t nbytes);
static inline void accel_cs_high() { GPIO_PinOutSet(ACCEL_CS_PORT, ACCEL_CS_PIN); }
static inline void accel_cs_low() { GPIO_PinOutClear(ACCEL_CS_PORT, ACCEL_CS_PIN); }

int accel_init()
{
  // note: SPI is a part of the USART peripheral on the blue gecko
  // Universal Synchronous Asynchronous Reciever Transmitter
  // enable clocks for USART
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_USART1, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
  
  // setup and configure GPIO's, note that CS pin will be under application control
  GPIO_PinModeSet(ACCEL_CLK_PORT, ACCEL_CLK_PIN, gpioModePushPull, 0);
  GPIO_PinModeSet(ACCEL_TX_PORT, ACCEL_TX_PIN, gpioModePushPull, 0);
  GPIO_PinModeSet(ACCEL_RX_PORT, ACCEL_RX_PIN, gpioModeInput, 0);
  GPIO_PinModeSet(ACCEL_CS_PORT, ACCEL_CS_PIN, gpioModePushPull, 1);

  // initialize USART1 for synchronous master mode, MSB first, CPOL=1, CPHA=1
  USART_Reset(USART1);
  USART_InitSync_TypeDef usart_init = USART_INITSYNC_DEFAULT;
  usart_init.master = true;
  usart_init.clockMode = usartClockMode3; // CPOL=1, CPHA=1
  usart_init.msbf = true; // MSB first
  USART_InitSync(USART1, &usart_init);
  USART_BaudrateSyncSet(USART1, 0, 4000000);

  // Enable I/O and set location - ref. manual pg 646
  USART1->ROUTELOC0 = (USART1->ROUTELOC0 & ~(_USART_ROUTELOC0_TXLOC_MASK
                        | _USART_ROUTELOC0_RXLOC_MASK | _USART_ROUTELOC0_CLKLOC_MASK))
                        | (ACCEL_TX_LOC << _USART_ROUTELOC0_TXLOC_SHIFT)
                        | (ACCEL_RX_LOC << _USART_ROUTELOC0_RXLOC_SHIFT)
                        | (ACCEL_CLK_LOC << _USART_ROUTELOC0_CLKLOC_SHIFT);

  USART1->ROUTEPEN  |=  USART_ROUTEPEN_CLKPEN  //| USART_ROUTEPEN_CSPEN
                      | USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_TXPEN;

  USART_Enable(USART1, usartEnable);

  accel_device_id_test();
  accel_set_measurement_mode();


  
  // TODO: setup accelerometer interrupt driven settings here

  return 0;
}

int accel_device_id_test()
{
  uint8_t ret = 0;
  accel_read(ADXL343_DEVID, &ret, 1);
  LOG("Device ID returned 0x%x\n", ret);
  return 0;
}

int accel_set_measurement_mode() 
{
  // Power control register map:
  // D7 | D6 | D5   | D4         | D3      | D2    | D1 | D0 |
  // 0  | 0  | Link | AUTO_SLEEP | Measure | Sleep | Wakeup  |
  // set measure bit to 1 to put part in measurement mode
  LOG("Setting measurement mode to 0x8\n");
  uint8_t tx = 0x08;
  accel_write(ADXL343_POWER_CTL, &tx, 1);
  uint8_t ret = 0x00;
  accel_read(ADXL343_POWER_CTL, &ret, 1);
  LOG("Measurement mode set to 0x%x\n", ret);
  return 0;
}

int accel_get_acceleration()
{ 
  uint8_t rx_buf[6];
  int16_t accel[3];
  accel_read(ADXL343_DATAX0, &rx_buf[0], 6);
  for (int i=0; i<3; i++)
  {
    accel[i] = (int16_t)( (rx_buf[2*i+1] << 0x8) | rx_buf[2*i] );
    printf("%d ", accel[i]);
  }
  printf("\n");
  return 0;
}

static uint32_t accel_read(uint8_t start_register, uint8_t *rx, uint32_t nbytes)
{
  if (nbytes <= 0 || rx == NULL) return -1;
  USART1->CMD |= USART_CMD_CLEARTX | USART_CMD_CLEARRX;
  uint8_t cmd;
  if (nbytes > 1) 
  {
    cmd = MULTI_READ_CMD(start_register);
  }
  else if (nbytes == 1)
  {
    cmd = SINGLE_READ_CMD(start_register);
  }
  accel_cs_low();
  USART_SpiTransfer(USART1, cmd);
  for (uint32_t i=0; i<nbytes; i++)
  {
    rx[i] = USART_SpiTransfer(USART1, 0xFF);
  }
  accel_cs_high();
  return 0;
}

static uint32_t accel_write(uint8_t start_register, uint8_t *tx, uint32_t nbytes) 
{
  if (nbytes <= 0 || tx == NULL) return -1;
  USART1->CMD |= USART_CMD_CLEARTX | USART_CMD_CLEARRX;
  uint8_t cmd;
  if (nbytes > 1) 
  {
    cmd = MULTI_WRITE_CMD(start_register);
  }
  else if (nbytes == 1)
  {
    cmd = SINGLE_WRITE_CMD(start_register);
  }
  accel_cs_low();
  USART_SpiTransfer(USART1, cmd);
  for (uint32_t i=0; i<nbytes; i++) 
  {
    USART_SpiTransfer(USART1, tx[i]); 
  }
  accel_cs_high();
  return 0;
}

