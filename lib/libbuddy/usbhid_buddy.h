#ifndef _USBHID_BUDDY
#define _USBHID_BUDDY

#include <stdint.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <buddy.h>
#include "hidapi.h"

// Defines
#define BUDDY_USB_VID 0x10C4
#define BUDDY_USB_PID 0x82CD

#define MAX_STR 255

#define TBL_BASE_INDEX 0
#define TBL_BASE_VALUE 1

// Peripheral Control and Value mappings
#define INIT_ADC_ROW_MAX 3
#define INIT_ADC_COL_MAX 2

#define INIT_DAC_ROW_MAX 2
#define INIT_DAC_COL_MAX 2

#define FREQUENCY_TO_NSEC(freq) ((1.0 / freq) * 1e9)

typedef enum _BUDDY_ERROR {
	BUDDY_ERROR_OK = 1,
	BUDDY_ERROR_GENERAL = 0,
	BUDDY_ERROR_INVALID = -1,
	BUDDY_ERROR_MEMORY = -2,
} BUDDY_ERROR;

/** @brief prints a hex dump of the internal HID IN packet buffer
 *  @param buffer pointer to IN USBHID packet
 *  @param length number of the bytes specified by the pointer buffer
 *  @return Void.
*/
void print_buffer(uint8_t *buffer, uint8_t length);

/** @brief prints a dump of ADC values for incoming HID IN packet buffer
*   @param buffer pointer to IN USBHID packet
*   @return Void.
*/
void print_buffer_simple(uint16_t *buffer);

/** @brief use the hidapi library to write a USBHID packet
 *  @param handle hidapi internal handle returned from buddy_init
 *  @param buffer pointer to OUT USBHID packet
 *  @param length number of the bytes specified by the pointer buffer
 *  @return -1 on failure, 0 on success. 
 */
int buddy_write_packet(hid_device *handle, unsigned char *buffer, int length);

/** @brief use the hidapi library to read a USBHID packet
 *  @param handle hidapi internal handle returned from buddy_init
 *  @param buffer pointer to location to store the IN USBHID packet
 *  @param length number of bytes to read in
 *  @return -1 on failure, 0 on success. 
 */
int buddy_read_packet(hid_device *handle, unsigned char *buffer, int length);

/** @brief open hidapi handle for Buddy VID/PID and set to non-blocking mode
 *  @return NULL on failure, hidapi handle pointer on success.
 */
hid_device* hidapi_init();

/** @brief initializes the Buddy subsystem and returns a hidapi handle pointer
 *  @param mode enumeration to set device in DAC, PWM, or ADC mode
 *  @Param pointer to channel and resolution bitmask structure
 *  @return NULL on failure, hidapi handle pointer on success.
 */
hid_device* buddy_init(ctrl_general_t *general, ctrl_runtime_t *runtime, ctrl_timing_t *timing);

/** @brief cleanup routine for closing hidapi file handle
 *  @param hidapi handle pointer
 *  @return BUDDY_ERROR_OK on success, BUDDY_ERROR_GENERAL on failure.
 */
int buddy_cleanup(hid_device *handle);

/** @brief sends a USB OUT request with the specified binary data
*   @param hidapi handle pointer
*   @param BUDDY_APP_CODE_OFFSET enum offset value
*   @param BUDDY_APP_INDIC_OFFSET enum offset value
*   @param binary data to be written in the USB HID OUT request
*   @param length of the binary data to written
*   @return BUDDY_ERROR_OK on success, BUDDY_ERROR_GENERAL on failure.
*/
int buddy_write_raw(hid_device *handle, uint8_t code, uint8_t indic, uint8_t *raw, uint8_t length);

/** @brief encodes the packet using codec and sends either immediately or if using
*			streaming then waits for codec buffer to be full before sending.
*   @param hidapi handle pointer
*   @param pointer to general_packet_t structure with DAC values to be sent
*	@param boolean indicating if stream mode is MODE_CTRL_STREAM or MODE_CTRL_IMMEDIATE
*   @return BUDDY_ERROR_OK on success, BUDDY_ERROR_GENERAL on failure.
*/
int buddy_send_dac(hid_device *handle, general_packet_t *packet, bool streaming);

/** @brief if streaming mode is on then a packet is decoded from the current frame, if
*			the frame buffer is empty then a new HID IN packet is received and decoded.
*   @param hidapi handle pointer
*   @param pointer to general_packet_t structure with ADC values to be received
*	@param boolean indicating if stream mode is MODE_CTRL_STREAM or MODE_CTRL_IMMEDIATE
*   @return BUDDY_ERROR_OK on success, BUDDY_ERROR_GENERAL on failure.
*/
int buddy_read_adc(hid_device *handle, general_packet_t *packet, bool streaming);

/** @brief writes the bytes that remain in the codec buffer.  This needs to be performed
*		    on the last write to prevent stagnant data remaining in the codec buffer.
*   @param BUDDY_ERROR_OK on success, BUDDY_ERROR_GENERAL on failure.
*/
int buddy_flush(hid_device *handle);

/** @brief returns the number of active channels by looking at the channel_mask
*			and counting them.
*   @param number of channels activated in the current request
*/
int buddy_count_channels(uint8_t chan_mask);

#endif