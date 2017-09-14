#ifndef _USBHID_BUDDY
#define _USBHID_BUDDY

#include <stdint.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include "buddy.h"
#include "hidapi.h"

//#define LABVIEW_BUILD _USRDLL

// Defined by VStudio project if a DLL is building
// to allow easy exports
#if defined(LABVIEW_BUILD)
#define BUDDY_EXPORT __declspec(dllexport)
#else
#define BUDDY_EXPORT /**< API export macro */
#endif

// Defines
#define BUDDY_USB_VID 0x10C4
#define BUDDY_USB_PID 0x4002

#define FREQUENCY_TO_NSEC(freq) ((1.0 / freq) * 1e9)
#define MAX_CHAR_LENGTH 255

#define BUDDY_MAX_IO_ATTEMPTS 5

typedef struct _buddy_hid_info_t {
	char *str_mfr;
	char *str_product;
	char *str_serial;
	char *str_index_1;
} buddy_hid_info_t;

typedef enum _BUDDY_ERROR {
	BUDDY_ERROR_OK = 1,
	BUDDY_ERROR_GENERAL = 0,
	BUDDY_ERROR_INVALID = -1,
	BUDDY_ERROR_MEMORY = -2,
} BUDDY_ERROR;

extern char *fw_info_dac_type_names[FIRMWARE_INFO_DAC_TYPE_LENGTH];

/** @brief prints a dump of the fw_info_dac_type_names ASCIIz
 *			strings to the console.
 *  @return Void.
*/
void print_fw_info_dac_types(void);

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

/** @brief open hidapi handle for Buddy VID/PID, set to non-blocking mode and
 *			return info on USB device and firmware.
 *	@param hid_info pointer to structure to store USB device information
 *  @return NULL on failure, hidapi handle pointer on success.
 */
hid_device* hidapi_init(buddy_hid_info_t *hid_info);

/** @brief configure the Buddy device
 *	@param general pointer to ctrl_general_t structure describing the
 *			operation (ADC/DAC), channels, resolution, etc.
 *	@param runtime pointer to ctrl_runtime_t structure describing the
 *			register settings for the ADC and DAC device.
 *  @param timing pointer to ctrl_timing_t structure desribing the
 *			sample period and averaging.
 *  @return BUDDY_ERROR_OK on success, BUDDY_ERROR_GENERAL on failure.
 */
// EXPORT
BUDDY_EXPORT int buddy_configure(hid_device *handle, ctrl_general_t *general, ctrl_runtime_t *runtime, ctrl_timing_t *timing);

/** @brief configure the Buddy device
 *  @param handle hidapi internal handle returned from buddy_init
 *  @param fw_info pointer firmware_info_t structure that will be filled with
 *			firmware device info
 *  @return BUDDY_ERROR_OK on success, BUDDY_ERROR_GENERAL on failure.
 */
int buddy_get_firmware_info(hid_device *handle, firmware_info_t *fw_info);

/** @brief initialize the USB HID connection and get the remote firmware device
 *			info and capabilities.
 *  @param hid_info pointer to buddy_hid_info_t structure that will be filled
 *			with USB HID info (mfr name, serial #, etc.)
 *	@param fw_info pointer firmware_info_t structure that will be filled with
 *			firmware device info
 *  @return BUDDY_ERROR_OK on success, BUDDY_ERROR_GENERAL on failure.
 */
BUDDY_EXPORT hid_device* buddy_init(buddy_hid_info_t *hid_info, firmware_info_t *fw_info);

/** @brief cleanup routine for closing hidapi file handle
 *  @param hidapi handle pointer
 *	@param hid_info pointer to structure to store USB device information
 *  @return BUDDY_ERROR_OK on success, BUDDY_ERROR_GENERAL on failure.
 */
BUDDY_EXPORT int buddy_cleanup(hid_device *handle, buddy_hid_info_t *hid_info);

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
BUDDY_EXPORT int buddy_send_dac(hid_device *handle, general_packet_t *packet, bool streaming);

/** @brief if streaming mode is on then a packet is decoded from the current frame, if
*			the frame buffer is empty then a new HID IN packet is received and decoded.
*   @param hidapi handle pointer
*   @param pointer to general_packet_t structure with ADC values to be received
*	@param boolean indicating if stream mode is MODE_CTRL_STREAM or MODE_CTRL_IMMEDIATE
*   @return BUDDY_ERROR_OK on success, BUDDY_ERROR_GENERAL on failure.
*/
BUDDY_EXPORT int buddy_read_adc(hid_device *handle, general_packet_t *packet, bool streaming);

/** @brief writes the bytes that remain in the codec buffer.  This needs to be performed
*		    on the last write to prevent stagnant data remaining in the codec buffer.
*   @param BUDDY_ERROR_OK on success, BUDDY_ERROR_GENERAL on failure.
*/
BUDDY_EXPORT int buddy_flush(hid_device *handle);

/** @brief returns the number of active channels by looking at the channel_mask
*			and counting them.
*   @param number of channels activated in the current request
*/
int buddy_count_channels(uint8_t chan_mask);

/** @brief trigger a start of conversion for either DAC or ADC
*   @param handle pointer
*/
BUDDY_EXPORT int buddy_trigger(hid_device *handle);

#endif