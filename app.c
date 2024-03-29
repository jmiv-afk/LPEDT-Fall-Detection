/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#include "app.h"

#if defined(SL_CATALOG_POWER_MANAGER_PRESENT)

// power manager callback
bool app_is_ok_to_sleep(void)
{
  return APP_IS_OK_TO_SLEEP;
} // app_is_ok_to_sleep()

// power manager callback
sl_power_manager_on_isr_exit_t app_sleep_on_isr_exit(void)
{
  return APP_SLEEP_ON_ISR_EXIT;
} // app_sleep_on_isr_exit()

#endif // defined(SL_CATALOG_POWER_MANAGER_PRESENT)

// application init
SL_WEAK void app_init(void)
{
  letimer0_init(); // initialize the timers
  gpio_init();     // initialize the gpio
  int status = accel_init();
  LOG("accel_init() returned %d", status);
}

// process application actions
SL_WEAK void app_process_action(void)
{
  // do nothing
  //while(1)
  //{
  //  accel_get_acceleration();
  //}
}

// Bluetooth stack event handler.
// This overrides the dummy weak implementation.
// @param[in] evt Event coming from the Bluetooth stack.
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  handle_ble_event(evt);
}
