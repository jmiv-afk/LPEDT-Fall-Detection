/* -----------------------------------------------------------------------------
 * @file   events.h
 * @brief  custom events, not associated with BLE stack events 
 * @author Jake Michael, jami1063@colorado.edu
 * ---------------------------------------------------------------------------*/

#ifndef _EVENTS_H_
#define _EVENTS_H_

typedef enum uint32_t {
  evt_none                 = 0x0,
  evt_accel_GPIO_INT1      = 0x1,
  evt_letimer0_UF          = 0x10,
  evt_letimer0_COMP1       = 0x20
} event_t;

#endif // _EVENTS_H_
