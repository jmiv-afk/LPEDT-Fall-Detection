/* -----------------------------------------------------------------------------
 * @file   timers.c
 * @brief  Timing delays / interrupt functionalities
 * @author Jake Michael, jami1063@colorado.edu
 *
 * @resources Originally written for IoT Embedded Firmware course Spring 2021 
 * @copyright All rights reserved. Distribution allowed only for the use of
 * assignment grading. Use of code excerpts allowed at the discretion of author.
 * Contact for permission.
 * ---------------------------------------------------------------------------*/

#include "timers.h"

// initialize the UF cycle counter to -1 in order to start at time 0
static volatile uint32_t UF_cycles = -1;

static inline void letimer0_increment_UF_cycles()
{
  UF_cycles++;
}

uint32_t letimer0_get_uptime_msec() 
{
  // elapsed ticks since previous UF is COMP0-LETIMER_CounterGet()
  // so we return UF_cycles*period_in_msec + elapsed_time_in_msec
  return ((uint32_t)UF_cycles*LETIMER_PERIOD_MS +
         ((LETIMER_COMP0 - LETIMER_CounterGet(LETIMER0))*MSEC_PER_SEC)/LETIMER_FREQ_HZ);
}

void LETIMER0_IRQHandler() 
{
  // determine IRQ source
  uint32_t flags = LETIMER_IntGetEnabled(LETIMER0);

  // clear IRQ source
  LETIMER_IntClear(LETIMER0, flags);

  if (flags == LETIMER_IF_UF) 
  {
    CORE_CRITICAL_SECTION(
    letimer0_increment_UF_cycles();
    );
  } 
  
  else if (flags == LETIMER_IF_COMP1) 
  {
    // indicates COMP1 interrupt from a timer started previously and
    // time has now elapsed   
    CORE_CRITICAL_SECTION(
    sl_bt_external_signal(evt_letimer0_COMP1);
    letimer0_end_timer_usec();
    );
  }

} // LETIMER0_IRQHandler


int letimer0_start_timer_usec(uint32_t usec) 
{
  
  uint32_t start_ticks;
  uint32_t comp1_setpt;
  uint32_t ticks_req = ((uint64_t) usec*LETIMER_FREQ_HZ)/USEC_PER_SEC;

  // constrain ticks between 1 and COMP0 (top value) for simplicity
  if (ticks_req <= 0 || ticks_req >= LETIMER_COMP0) { return -1; } //error

  start_ticks = LETIMER_CounterGet(LETIMER0);

  if (start_ticks < ticks_req) 
  {
    // handle case where timer count wraps from 0 to COMP0 during countdown 
    comp1_setpt = LETIMER_COMP0+start_ticks-ticks_req;
  }
  else
  {
    // handle simple case 
    comp1_setpt = start_ticks - ticks_req; 
  }  

  // set compare COMP1 register and enable COMP1 interrupt
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();
  LETIMER_CompareSet(LETIMER0, 1, comp1_setpt);

  // clear any outstanding COMP1 interrupts
  LETIMER_IntClear(LETIMER0, LETIMER_IEN_COMP1);

  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);
  CORE_EXIT_CRITICAL();

  // return OK
  return 0;
}

void letimer0_end_timer_usec()
{
  // turn off COMP1 interrupts
  LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);
  // clear any outstanding COMP1 interrupts
  LETIMER_IntClear(LETIMER0, LETIMER_IEN_COMP1);
}


int letimer0_delay_usec(uint32_t usec)
{
  
  uint32_t start_ticks;
  // handle large usec by casting to uint64_t
  uint32_t ticks_req = ((uint64_t) usec*LETIMER_FREQ_HZ)/USEC_PER_SEC; 
  
  // constrain ticks between 1 and COMP0 (top value) for simplicity
  if (ticks_req <= 0 || ticks_req >= LETIMER_COMP0) { return -1; } //error

  // get starting count
  start_ticks = LETIMER_CounterGet(LETIMER0);

  if (start_ticks < ticks_req) 
  {
    // handle case where timer count wraps from 0 to COMP0 during countdown 
    while(LETIMER_CounterGet(LETIMER0) != LETIMER_COMP0+start_ticks-ticks_req) {;}
  }
  else
  {
    // handle simple case 
    while(LETIMER_CounterGet(LETIMER0) != start_ticks - ticks_req) {;} 
  }  

  return 0;
}

void letimer0_init()
{
  // enable oscillator
  CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

  // configure oscillator to LFA clock branch
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
  
  // set clock prescale divider to 4
  CMU_ClockDivSet(cmuClock_LETIMER0, LETIMER_PRESCALER);

  // enable LFA clock tree
  // CMU_ClockEnable(cmuClock_LFA, true); // may not be needed

  // enable LETIMER0 
  CMU_ClockEnable(cmuClock_LETIMER0, true);

  // setup a basis for initializing LETIMER0
  const LETIMER_Init_TypeDef letimer_ctx = {
    .enable = false,   // enabled later
    .debugRun = true,
    .comp0Top = true,
    .bufTop = false,
    .out0Pol = 0,
    .out1Pol = 0,
    .ufoa0 = letimerUFOANone,
    .ufoa1 = letimerUFOANone,
    .repMode = letimerRepeatFree
  };

  // initialize LETIMER0 with basis struct
  LETIMER_Init(LETIMER0, &letimer_ctx);

  // set COMP0
  LETIMER_CompareSet(LETIMER0, 0, LETIMER_COMP0);

  // enable interrupts on UF match
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);

  // clear any outstanding UF/COMP1 interrupts
  LETIMER_IntClear(LETIMER0, LETIMER_IEN_COMP1 | LETIMER_IEN_UF);

  // allow interrupts from NVIC
  NVIC_EnableIRQ(LETIMER0_IRQn);

  // enable timer
  LETIMER_Enable(LETIMER0, true);
}

