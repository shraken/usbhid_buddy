#ifndef _SUPPORT
#define _SUPPORT

#include <stdint.h>

// Headers needed for sleeping.
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/** @brief sleep wrapper
*  @param sleep_msec number of milliseconds to sleep
*  @return Void.
*/
void short_sleep(int sleep_msec);

uint16_t swap_uint16(uint16_t val);
int16_t swap_int16(int16_t val);
uint32_t swap_uint32(uint32_t val);
int32_t swap_int32(int32_t val);

#endif