#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

// $[ACMP0]
// [ACMP0]$

// $[ACMP1]
// [ACMP1]$

// $[ADC0]
// [ADC0]$

// $[CMU]
// [CMU]$

// $[DBG]
// DBG SWCLKTCK on PF0
#define DBG_SWCLKTCK_PORT                        gpioPortF
#define DBG_SWCLKTCK_PIN                         0
#define DBG_ROUTE_LOC                            0

// DBG SWDIOTMS on PF1
#define DBG_SWDIOTMS_PORT                        gpioPortF
#define DBG_SWDIOTMS_PIN                         1

// DBG SWV on PF2
#define DBG_SWV_PORT                             gpioPortF
#define DBG_SWV_PIN                              2
#define DBG_SWV_LOC                              0

// [DBG]$

// $[ETM]
// [ETM]$

// $[PTI]
// PTI DFRAME on PB13
#define PTI_DFRAME_PORT                          gpioPortB
#define PTI_DFRAME_PIN                           13
#define PTI_DFRAME_LOC                           6

// PTI DOUT on PB12
#define PTI_DOUT_PORT                            gpioPortB
#define PTI_DOUT_PIN                             12
#define PTI_DOUT_LOC                             6

// [PTI]$

// $[GPIO]
// [GPIO]$

// $[I2C0]
// [I2C0]$

// $[I2C1]
// [I2C1]$

// $[LESENSE]
// [LESENSE]$

// $[LETIMER0]
// [LETIMER0]$

// $[LEUART0]
// [LEUART0]$

// $[LFXO]
// [LFXO]$

// $[MODEM]
// [MODEM]$

// $[PCNT0]
// [PCNT0]$

// $[PRS.CH0]
// [PRS.CH0]$

// $[PRS.CH1]
// [PRS.CH1]$

// $[PRS.CH2]
// [PRS.CH2]$

// $[PRS.CH3]
// [PRS.CH3]$

// $[PRS.CH4]
// [PRS.CH4]$

// $[PRS.CH5]
// [PRS.CH5]$

// $[PRS.CH6]
// [PRS.CH6]$

// $[PRS.CH7]
// [PRS.CH7]$

// $[PRS.CH8]
// [PRS.CH8]$

// $[PRS.CH9]
// [PRS.CH9]$

// $[PRS.CH10]
// [PRS.CH10]$

// $[PRS.CH11]
// [PRS.CH11]$

// $[TIMER0]
// [TIMER0]$

// $[TIMER1]
// [TIMER1]$

// $[USART0]
// USART0 RX on PF4
#define USART0_RX_PORT                           gpioPortF
#define USART0_RX_PIN                            4
#define USART0_RX_LOC                            27

// USART0 TX on PF3
#define USART0_TX_PORT                           gpioPortF
#define USART0_TX_PIN                            3
#define USART0_TX_LOC                            27

// [USART0]$

// $[USART1]
// [USART1]$

// $[USART2]
// [USART2]$

// $[VDAC0]
// [VDAC0]$

// $[WTIMER0]
// [WTIMER0]$

#endif // PIN_CONFIG_H

