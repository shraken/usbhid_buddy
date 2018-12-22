/**
 * @author Nicholas L Shrake
 * @date 12-21-2018
 * @brief Buddy codec encode, decode and support routines.  Both firmware
 *  and software use this component.  
 * 
 */

#include "buddy.h"

uint8_t codec_byte_offset = 0;
uint8_t _chan_enable[BUDDY_CHAN_LENGTH];

uint8_t _data_size = CODEC_FIXED_SIZE;

/// encode and decode count are stored in the frame and set by the encode
/// and used by decode for number of packet structures embedded in the frame
static uint8_t encode_count = 0;
static uint8_t decode_count = 0;

/// bitmask describing active channels
static uint8_t m_chan_mask = 0;

/// resolution describes the bit resolution of the packed values in the frame
static uint8_t m_resolution = 0;

/*
 * @brief put the codec encoder and decoder in an initial state where
 *  the offset and decode and encode counts are set to zero.
 */
void codec_reset(void) {
	codec_byte_offset = 0;
	encode_count = 0;
	decode_count = 0;
}

/**
 * @brief get the encode_count variable
 * @return uint8_t the encode counter variable
 */
uint8_t codec_get_encode_count(void) {
    return encode_count;
}

/**
 * @brief set the encode_count variable
 * @param count the counter to set in encode_count variable
 */
void codec_set_encode_count(const uint8_t count) {
    encode_count = count;
}

/**
 * @brief set the initial state for the codec.  We set the resolution, 
 *  mode, and other parameters here.
 * @param 
 * @param 
 * 
 */
void codec_set_config(uint8_t chan_mask, uint8_t resolution) {
    m_chan_mask = chan_mask;
    m_resolution = resolution;
}

/** @brief returns the an integer count for the number of channels active
 *  as specified by the channel mask.
*/
int codec_count_channels(void)
{
	int i;
	int channel_count = 0;

	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (chan_mask & (1 << i)) {
			channel_count++;
		}
	}

	return channel_count;
}

/**
 * @brief encode the channel packet values into the buddy frame buffer.  This function
 *  bit packs the channel values as specified by the requested bit resolution.  The bit
 *  packed values are copied into the buffer.  If the frame is full, a return code indicates
 *  this and the frame buffer should be sent.
 * 
 * @param pointer temporary location where multiple packed packet channel values are stored
 *  before being sent over the USB HID interface
 * @param packet pointer specifies channel values in a structure
 * @return int CODEC_STATUS_ERROR for error, CODEC_STATUS_FULL if the frame buffer is full
 *  and CODEC_STATUS_CONTINUE if the operation was successfully and frame buffer not full.
 */
int codec_encode(uint8_t *frame, general_packet_t *packet)
{
	uint8_t i;

	if ((!frame) || (!packet)) {
		return CODEC_STATUS_ERROR;
	}

	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (_chan_enable[i]) {
			if (m_resolution == RESOLUTION_CTRL_SUPER) {
				*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset) = ((packet->channels[i] & 0xFF000000) >> 24);
				*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 1) = ((packet->channels[i] & 0xFF0000) >> 16);
				*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 2) = ((packet->channels[i] & 0xFF00) >> 8);
				*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 3) = (packet->channels[i] & 0xFF);
			} else if (m_resolution == RESOLUTION_CTRL_HIGH) {
				*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset) = ((packet->channels[i] & 0xFF00) >> 8);
				*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 1) = (packet->channels[i] & 0xFF);
			} else if (m_resolution == RESOLUTION_CTRL_LOW) {
				*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset) = (packet->channels[i] & 0xFF);
			} else {
				// resolution not defined, return error
				return CODEC_STATUS_ERROR;
			}
			
			codec_byte_offset += _data_size;
		}
	}

	// check if subsequent packet will overflow buffer
	uint8_t encode_max_offset = (codec_byte_offset + (_data_size * _chan_number));
	if (encode_max_offset >= (MAX_REPORT_SIZE - 3)) {
		codec_byte_offset = 0;
		return CODEC_STATUS_FULL;
	} else {
        encode_count++;
		return CODEC_STATUS_CONTINUE;
	}
}

/**
 * @brief decode the channel values from a frame buffer into a packet structure.  This
 *  function is called repeatedly on the frame buffer to decode all channel result packets.
 *  The packed channel values are bit shifted back to the resolution mode specified in
 *  initial codec configuration.  
 * 
 * @param frame pointer location where multiple packed packet channel values are stored
 *  before being unpacked and copied into packet pointer
 * @param packet pointer destination where unpacked values are stored from the frame buffer
 * @return int CODEC_STATUS_ERROR for error, CODEC_STATUS_FULL if the frame buffer has been
 *  decoded and CODEC_STATUS_CONTINUE if the operation was successfully but the frame buffer
 *  has not been fully decoded.
 */
int codec_decode(uint8_t *frame, general_packet_t *packet)
{
	uint8_t count;
	int i;

	if ((!frame) || (!packet)) {
		return CODEC_STATUS_ERROR;
	}

	count = *(frame + BUDDY_APP_INDIC_OFFSET);
	
	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (_chan_enable[i]) {
			if (m_resolution == RESOLUTION_CTRL_SUPER) {
				packet->channels[i] = (*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset) << 24);
				packet->channels[i] |= (*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 1) << 16);
				packet->channels[i] |= (*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 2) << 8);
				packet->channels[i] |= *(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 3);
			} else if (m_resolution == RESOLUTION_CTRL_HIGH) {
				if ((*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset)) & 0x80) {
					packet->channels[i] = ((*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset)) << 8);
					packet->channels[i] |= *(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 1);
					
					// sign extension
					packet->channels[i] = packet->channels[i] | 0xFFFF0000;
				} else {
					packet->channels[i] = (*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset) << 8);
					packet->channels[i] |= *(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset + 1);
				}
			} else if (m_resolution == RESOLUTION_CTRL_LOW) {
				packet->channels[i] = (*(frame + BUDDY_APP_VALUE_OFFSET + codec_byte_offset));
			} else {
				// resolution not defined, return error
				return CODEC_STATUS_ERROR;
			}

			codec_byte_offset += _data_size;
		}
	}

	decode_count++;

	// check if the next packet can fit in the packed array
	uint8_t decode_max_offset = (codec_byte_offset + (_data_size * _chan_number));
	
	// @todo: changed decode_count >= count to decode_count > count check if works
	if ((decode_max_offset >= (MAX_REPORT_SIZE - 3)) || (decode_count > count)) {
		codec_byte_offset = 0;
		decode_count = 0;
		return CODEC_STATUS_FULL;
	} else {
		return CODEC_STATUS_CONTINUE;
	}
}