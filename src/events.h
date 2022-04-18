/* -----------------------------------------------------------------------------
 * @file   events.h
 * @brief  custom events, not associated with BLE stack events 
 * @author Jake Michael, jami1063@colorado.edu
 * ---------------------------------------------------------------------------*/

#ifndef _EVENTS_H_
#define _EVENTS_H_

typedef enum uint32_t {
  evt_none              = 0,
  evt_letimer0_UF       = 1,
  evt_letimer0_COMP1    = 2
} event_t;

#endif // _EVENTS_H_
