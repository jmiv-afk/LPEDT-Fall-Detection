/* -----------------------------------------------------------------------------
 * @file log.h 
 * @brief Provide logging via printf
 * @author Jake Michael, jami1063@colorado.edu
 * ---------------------------------------------------------------------------*/
#ifndef _LOG_H_
#define _LOG_H_

#include <stdio.h>
#include "app_log.h"

#define DEBUG (1)

#if (DEBUG == 1)
  #define LOG(msg, ...) \
    printf(msg "\n", ##__VA_ARGS__)
#else
  #define LOG(msg, ...) // do nothing 
#endif 

#endif // _LOG_H_
