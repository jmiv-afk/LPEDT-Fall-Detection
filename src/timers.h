/* -----------------------------------------------------------------------------
 * @file   timers.h
 * @brief  Timing delays / interrupt functionalities
 * @author Jake Michael, jami1063@colorado.edu
 *
 * @resources Originally written for IoT Embedded Firmware course Spring 2021
 * @copyright All rights reserved. Distribution allowed only for the use of
 * assignment grading. Use of code excerpts allowed at the discretion of author.
 * Contact for permission.
 * ---------------------------------------------------------------------------*/

#ifndef _TIMERS_H_
#define _TIMERS_H_

#include "em_core.h"
#include "em_cmu.h"
#include "em_letimer.h"
#include "sl_bt_api.h"

#include "events.h"

#define USEC_PER_SEC       (1000000)
#define MSEC_PER_SEC       (1000)
#define LETIMER_PERIOD_MS  (3000)
#define OSC_FREQ_HZ        (32768) 
#define LETIMER_PRESCALER  (4)
#define LETIMER_COMP0      ((LETIMER_PERIOD_MS*OSC_FREQ_HZ)/(1000*LETIMER_PRESCALER))
#define LETIMER_FREQ_HZ    (OSC_FREQ_HZ/LETIMER_PRESCALER)


/* @brief  LETIMER0 interrupt service routine  
 *
 * Sets events to control state machine in scheduler module.
 *
 * @param  None
 * @return None
 */
void LETIMER0_IRQHandler();


/* @brief  Returns approximate uptime measured in msec.
 *
 * The approximation is based on the current ticks in the counter and the 
 * number of UF interrupts that have occurred at the UF interrupt freq. 
 *
 * @param  None
 * @return uint64_t, msec elapsed since startup
 */
uint32_t letimer0_get_uptime_msec();


/* @brief  Starts a timer for usec microseconds  
 *
 * Note that the possible timer time (in usec) is constrained to the inclusive
 * range [1, LETIMER_PERIOD_MS*1000-1]
 *
 * @param  uint32_t, time in usec for the timer to run
 * @return -1 upon error, 0 upon success (timer started) 
 */
int letimer0_start_timer_usec(uint32_t usec) ;


/* @brief  Ends (or cancels) the previously started timer using COMP1
 *
 * @param  None
 * @return None
 */
void letimer0_end_timer_usec();


/* @brief  Runs BLOCKING delay for usec microseconds
 *
 * Note that the possible delay time (in usec) is constrained to the inclusive
 * range [1, LETIMER_PERIOD_MS*1000-1]
 *
 * @param  uint32_t, the time for the delay in microseconds
 * @return -1 upon error, 0 upon success
 */
int letimer0_delay_usec(uint32_t usec);


/* @brief  Initializes LETIMER0
 *
 * Sets top reload to COMP0 with free (continuous) repeat and reload. Enables
 * interrupts on COMP0 (top) and COMP1 (adjustable) 16-bit compare registers 
 * then starts the timer.
 *
 * @param  None
 * @return None
 */
void letimer0_init();

#endif // _TIMERS_H_

