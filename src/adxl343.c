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

#define ACCEL_INT1_PORT (gpioPortA)
#define ACCEL_INT1_PIN  (4)

static int accel_read(uint8_t start_register, uint8_t *rx, uint32_t nbytes);
static int accel_write(uint8_t start_register, uint8_t *tx, uint32_t nbytes);
static inline void accel_cs_high() { GPIO_PinOutSet(ACCEL_CS_PORT, ACCEL_CS_PIN); }
static inline void accel_cs_low() { GPIO_PinOutClear(ACCEL_CS_PORT, ACCEL_CS_PIN); }

void GPIO_EVEN_IRQHandler()
{
  CORE_CRITICAL_SECTION(
    uint32_t flags = GPIO_IntGetEnabled() & 0x55555555; // pickoff even bits
    GPIO_IntClear(flags);
    sl_bt_external_signal(evt_accel_GPIO_INT1);
  );
}

int accel_init()
{
  // note: SPI is a part of the USART peripheral on the blue gecko
  // Universal Synchronous Asynchronous Receiver Transmitter
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

  // enable I/O and set tx,rx,clk locations - ref. manual pg 646
  USART1->ROUTELOC0 = (USART1->ROUTELOC0 & ~(_USART_ROUTELOC0_TXLOC_MASK
                        | _USART_ROUTELOC0_RXLOC_MASK | _USART_ROUTELOC0_CLKLOC_MASK))
                        | (ACCEL_TX_LOC << _USART_ROUTELOC0_TXLOC_SHIFT)
                        | (ACCEL_RX_LOC << _USART_ROUTELOC0_RXLOC_SHIFT)
                        | (ACCEL_CLK_LOC << _USART_ROUTELOC0_CLKLOC_SHIFT);
  // enable routes
  USART1->ROUTEPEN |=   USART_ROUTEPEN_CLKPEN 
                      | USART_ROUTEPEN_RXPEN 
                      | USART_ROUTEPEN_TXPEN;

  USART_Enable(USART1, usartEnable);

  // TODO: setup accelerometer interrupt driven settings here
  // read the device ID 
  uint8_t val = 0;
  accel_read(ADXL343_DEVID, &val, 1);
  if (val != 0xE5)
  {
    LOG("Error: device ID != 0xe5, returned %d", val);
  }

  int settings_len = 29;
  uint8_t settings[settings_len]; 
  accel_read(ADXL343_THRESH_TAP, settings, settings_len);
  for (int i=0; i<settings_len; i++)
  {
    LOG("Reg: 0x%x = 0x%x", i+0x1d, settings[i]);
  }

  // disable interrupts during configuration
  val = 0x0;
  accel_write(ADXL343_INT_ENABLE, &val, 1);

  // set bandwidth register:
  // D7 | D6 | D5 | D4        | D3 | D2 | D1 | D0 |
  // 0  | 0  | 0  | LOW_POWER |      Rate         |
  val = (1 << 4) | (0b0000010);  // use 100 Hz
  accel_write(ADXL343_BW_RATE, &val, 1); 

  // set inactivity threshold (scale factor = 62.5 mg/LSB)
  val = 8; // = 1/2 g
  accel_write(ADXL343_THRESH_INACT, &val, 1);
  // set inactivity time (scale factor = 1 sec/LSB)
  val = 4; // 4 seconds
  accel_write(ADXL343_TIME_INACT, &val, 1);

  // set activity threshold (scale factor = 62.5 mg/LSB)
  val = 4; // = 1/4 g
  accel_write(ADXL343_THRESH_ACT, &val, 1);

  // activity/inactivity control
  // D7          | D6              | D5              | D4           
  // ACT AC/DC   | ACT_X enable    | ACT_Y enable    | ACT_Z enable   | 
  // ------------+-----------------+-----------------+----------------+
  // D3          | D2              | D1              | D0             |
  // INACT AC/DC | INACT_X enable  | INACT_Y enable  | INACT_Z enable |
  val = 0b11111111; // all axis, ac coupled operation
  accel_write(ADXL343_ACT_INACT_CTL, &val, 1);

  // set free-fall threshold (scale factor = 62.5 mg/LSB)
  val = 9;
  accel_write(ADXL343_THRESH_FF, &val, 1);
  // set free-fall time (scale factor = 5 ms/LSB)
  val = 40; // 200 msec
  accel_write(ADXL343_TIME_FF, &val, 1);

  // setup interrupts
  // Below register map applies to INT_ENABLE, INT_MAP, and INT_SOURCE
  // D7         | D6         | D5         | D4       |
  // DATA_READY | SINGLE_TAP | DOUBLE_TAP | ACTIVITY | 
  // -----------+------------+------------+----------+
  // D3         | D2         | D1         | D0       |
  // INACTIVITY | FREE_FALL  | WATERMARK  | OVERRUN  |
  val = 0b00011100; // enable free fall, activity, inactivity
  accel_write(ADXL343_INT_ENABLE, &val, 1);

  // Power control register map:
  // D7 | D6 | D5   | D4         | D3      | D2    | D1 | D0 |
  // 0  | 0  | Link | AUTO_SLEEP | Measure | Sleep | Wakeup  |
  val = 0b00111000; // enable link, auto sleep, measurement mode
  accel_write(ADXL343_POWER_CTL, &val, 1);

  // clear interrupt sources
  accel_determine_interrupt_source(&val);
  // setup and configure interrupt GPIO's
  GPIO_PinModeSet(ACCEL_INT1_PORT, ACCEL_INT1_PIN, gpioModeInput, 0);
  // disable interrupts
  uint32_t flags = GPIO_IntGetEnabled();
  GPIO_IntDisable(flags);
  GPIO_IntClear(flags);
  GPIO_ExtIntConfig( ACCEL_INT1_PORT, // port
                     ACCEL_INT1_PIN,  // pin
                     ACCEL_INT1_PIN,  // interrupt number
                     true,            // rising edge enable
                     false,           // falling edge enable
                     true             // enable upon return
                    );
  // don't forget the NVIC!!!
  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);

  // read out final register settings
  memset(settings, 0x0, settings_len);
  accel_read(ADXL343_THRESH_TAP, settings, settings_len);
  for (int i=0; i<settings_len; i++)
  {
    LOG("Reg: 0x%x = 0x%x", i+0x1d, settings[i]);
  }

  return 0;
}

void accel_determine_interrupt_source(uint8_t *reg) 
{
  accel_read(ADXL343_INT_SOURCE, reg, 1);
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

static int accel_read(uint8_t start_register, uint8_t *rx, uint32_t nbytes)
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

static int accel_write(uint8_t start_register, uint8_t *tx, uint32_t nbytes) 
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

