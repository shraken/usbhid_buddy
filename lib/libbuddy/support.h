#ifndef _SUPPORT
#define _SUPPORT

#include <stdint.h>

// Headers needed for sleeping.
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

/** @brief wraps sleep operations on the host application
 *  @param sleep_msec number of milliseconds to sleep
 *  @return Void.
*/
void short_sleep(int sleep_msec);

/** @brief swap the endian byte order of a unsigned 16-bit integer
 *  @param val unsigned 16-bit integer to be swapped
 *  @return swapped endian result
*/
uint16_t swap_uint16(uint16_t val);

/** @brief swap the endian byte order of a signed 16-bit integer
 *  @param val signed 16-bit integer to be swapped
 *  @return swapped endian result
*/
int16_t swap_int16(int16_t val);

/** @brief swap the endian byte order of a unsigned 32-bit integer
 *  @param val unsigned 16-bit integer to be swapped
 *  @return swapped endian result
*/
uint32_t swap_uint32(uint32_t val);

/** @brief swap the endian byte order of a signed 32-bit integer
 *  @param val signed 32-bit integer to be swapped
 *  @return swapped endian result
*/
int32_t swap_int32(int32_t val);

#endif