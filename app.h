/***************************************************************************//**
 * @file
 * @brief Application interface provided to main().
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

#ifndef APP_H
#define APP_H

#include "sl_power_manager.h"
#include "src/ble.h"
#include "src/log.h"
#include "src/gpio.h"
#include "src/adxl343.h"
#include "src/timers.h"

#define LOWEST_ENERGY_MODE 2
#if (LOWEST_ENERGY_MODE == 0)
  #define APP_IS_OK_TO_SLEEP      (false)
#else
  #define APP_IS_OK_TO_SLEEP      (true)
#endif
#define APP_SLEEP_ON_ISR_EXIT   (SL_POWER_MANAGER_SLEEP)

void app_init(void);
void app_process_action(void);
bool app_is_ok_to_sleep(void);

#endif // APP_H
