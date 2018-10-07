#include <stdint.h>
#include <support.h>

/** @brief swap the endian byte order of a unsigned 16-bit integer
 *  @param val unsigned 16-bit integer to be swapped
 *  @return swapped endian result
*/
uint16_t swap_uint16(uint16_t val)
{
	return (val << 8) | (val >> 8);
}

/** @brief swap the endian byte order of a signed 16-bit integer
 *  @param val signed 16-bit integer to be swapped
 *  @return swapped endian result
*/
int16_t swap_int16(int16_t val)
{
	return (val << 8) | ((val >> 8) & 0xFF);
}

/** @brief swap the endian byte order of a unsigned 32-bit integer
 *  @param val unsigned 16-bit integer to be swapped
 *  @return swapped endian result
*/
uint32_t swap_uint32(uint32_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | (val >> 16);
}

/** @brief swap the endian byte order of a signed 32-bit integer
 *  @param val signed 32-bit integer to be swapped
 *  @return swapped endian result
*/
int32_t swap_int32(int32_t val)
{
	val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
	return (val << 16) | ((val >> 16) & 0xFFFF);
}

/** @brief wraps sleep operations on the host application
 *  @param sleep_msec number of milliseconds to sleep
 *  @return Void.
*/
void short_sleep(int sleep_msec)
{
#ifdef WIN32
	Sleep(sleep_msec);
#else
	usleep(sleep_msec * 1000);
#endif
}
