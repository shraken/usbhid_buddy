/**
 * @author Nicholas L Shrake
 * @date 12-21-2018
 * @brief Buddy codec encode, decode and support routines.  Both firmware
 *  and software use this component.  
 * 
 */

#include "codec.h"

/// encode and decode byte offset into the frame buffer.  This offset points
/// to the location of where we are encoding or decoding.  
static uint8_t m_offset = BUDDY_APP_VALUE_OFFSET;

/// encode and decode count are stored in the frame and set by the encode
/// and used by decode for number of packet structures embedded in the frame
static uint8_t m_encode_count = 0;
static uint8_t m_decode_count = 0;

/// bitmask describing active channels
static uint8_t m_chan_mask = 0;

/// resolution describes the bit resolution of the packed values in the frame
static uint8_t m_resolution = 0;

/// number of active channels as calculated from the m_chan_mask.  We store
/// this for quick access by encode and decode
static uint8_t m_chan_number = 0;

/// the size of each packed channel value in the frame buffer
static uint8_t m_data_size = CODEC_FIXED_SIZE;

/// codec has been initialized, checked by the encode and decode routines
/// an error code is returned letting application know codec init must be run first
static bool m_initialized = false;

/// channel enable bit array.  There are N positions for each of the N channels
/// with 1 = active and 0 = inactive indication
bool m_chan_enable[BUDDY_CHAN_LENGTH];

/*
 * @brief put the codec encoder and decoder in an initial state where
 *  the offset and decode and encode counts are set to zero.
 */
void codec_reset(void) {
	m_offset = 0;
	m_encode_count = 0;
	m_decode_count = 0;
}

/**
 * @brief set the initial state for the codec.  We set the resolution, 
 *  mode, and other parameters here.
 * @param chan_mask channel bit mask sent during configuration step
 * @param resolution bit resolution used when packing
 * @return 0 for success, otherwise error
 * 
 */
int codec_init(uint8_t chan_mask, uint8_t resolution) {
    codec_reset();

    m_chan_mask = chan_mask;
    m_resolution = resolution;

    switch (resolution) {
    	case RESOLUTION_CTRL_SUPER:
    		m_data_size = BUDDY_DATA_SIZE_SUPER;
    		break;

    	case RESOLUTION_CTRL_HIGH:
    		m_data_size = BUDDY_DATA_SIZE_HIGH;
    		break;

    	case RESOLUTION_CTRL_LOW:
    		m_data_size = BUDDY_DATA_SIZE_LOW;
    		break;

    	default:
    		return -1;
    }

    // pre-compute number of channels for future encode/decode ops
    codec_count_channels();

    m_initialized = true;
    return 0;
}

/**
 * @brief set the data size that is used to advance in the frame buffer
 *  on each invocation of the encode/decode functions.  
 * 
 * @param data_size integer number of bytes that packed channel value
 */
void codec_set_data_size(const uint8_t data_size) {
    m_data_size = data_size;
}

/**
 * @brief get the data size used in advancing the frame buffer on each
 *  invocation of the encode/decode functions.
 * 
 * @return uint8_t data_size stored integer number of bytes of the packed
 *  channel values.  
 */
uint8_t codec_get_data_size(void) {
    return m_data_size;
}

/**
 * @brief set the codec offset count variable.  This variable tracks the
 *  encode (write) or decode (read) offset in the frame buffer.
 * @param count the counter to set in encode_count variable
 */
void codec_set_offset_count(const uint8_t offset) {
    m_offset = offset;
}

/**
 * @brief get the codec offset count variable.
 * 
 * @return uint8_t the codec offset variable
 */
uint8_t codec_get_offset_count(void) {
    return m_offset;
}

/** @brief returns the an integer count for the number of channels active
 *  as specified by the channel mask.
*/
void codec_count_channels(void)
{
	int i;

    m_chan_number = 0;
	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (m_chan_mask & (1 << i)) {
			m_chan_number++;
		}
	}
}

/**
 * @brief public API for getting pre-computed number of channels.  The
 *  application should call the codec_count_channels method to compute
 *  number of channels from the provided channel bitmask.
 * 
 * @return uint8_t the number of channels
 */
uint8_t codec_get_channel_count(void) {
    return m_chan_number;
}

/**
 * @brief get the codec m_encode_count.  This count is incremented
 *  on each invocation of the encode method.  
 * @return uint8_t the encode counter variable
 */
uint8_t codec_get_encode_count(void) {
    return m_encode_count;
}

/**
 * @brief set the codec m_encode_count used by the encoder when
 *  the frame is full.  
 * 
 * @param count value to set the m_encode_count variable with
 */
void codec_set_encode_count(const uint8_t count) {
    m_encode_count = count;
}

/**
 * @brief get the codec m_decode_count.  This count is decremented on
 *  each invocation of the decode method.
 * @return uint8_t the encode counter variable
 */
uint8_t codec_get_decode_count(void) {
    return m_decode_count;
}

/**
 * @brief set the codec m_decode_count used by the decoder when
 *  the frame is full.  
 * 
 * @param count value to set the m_decode_count variable with
 */
void codec_set_decode_count(const uint8_t count) {
    m_decode_count = count;
}

/**
 * @brief check if the channel is active in the codec
 * 
 * @param channel integer of the channel to chekc
 * @return true channel is active
 * @return false channel is not active
 */
bool codec_is_channel_active(const uint8_t channel) {
    return m_chan_enable[channel];
}

void codec_set_channel_active(const uint8_t channel, bool state) {
    m_chan_enable[channel] = state;
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

	if (!m_initialized) {
		return CODEC_STATUS_UNINITIALIZED;
	}

	if ((!frame) || (!packet)) {
		return CODEC_STATUS_ERROR;
	}

	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (m_chan_enable[i]) {
			if (m_resolution == RESOLUTION_CTRL_SUPER) {
				*(frame + BUDDY_APP_VALUE_OFFSET + m_offset) = ((packet->channels[i] & 0xFF000000) >> 24);
				*(frame + BUDDY_APP_VALUE_OFFSET + m_offset + 1) = ((packet->channels[i] & 0xFF0000) >> 16);
				*(frame + BUDDY_APP_VALUE_OFFSET + m_offset + 2) = ((packet->channels[i] & 0xFF00) >> 8);
				*(frame + BUDDY_APP_VALUE_OFFSET + m_offset + 3) = (packet->channels[i] & 0xFF);
			} else if (m_resolution == RESOLUTION_CTRL_HIGH) {
				*(frame + BUDDY_APP_VALUE_OFFSET + m_offset) = ((packet->channels[i] & 0xFF00) >> 8);
				*(frame + BUDDY_APP_VALUE_OFFSET + m_offset + 1) = (packet->channels[i] & 0xFF);
			} else if (m_resolution == RESOLUTION_CTRL_LOW) {
				*(frame + BUDDY_APP_VALUE_OFFSET + m_offset) = (packet->channels[i] & 0xFF);
			} else {
				// resolution not defined, return error
				return CODEC_STATUS_ERROR;
			}
			
			m_offset += codec_get_data_size();
		}
	}

	// check if subsequent packet will overflow buffer
	uint8_t encode_max_offset = (m_offset + (codec_get_data_size() * m_chan_number));

	if (encode_max_offset >= (MAX_REPORT_SIZE - BUDDY_APP_VALUE_OFFSET)) {
		m_offset = 0;
		return CODEC_STATUS_FULL;
	} else {
        m_encode_count++;
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

	if (!m_initialized) {
		return CODEC_STATUS_UNINITIALIZED;
	}

	if ((!frame) || (!packet)) {
		return CODEC_STATUS_ERROR;
	}

	count = *(frame + BUDDY_APP_INDIC_OFFSET);

	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (m_chan_enable[i]) {
			if (m_resolution == RESOLUTION_CTRL_SUPER) {
				packet->channels[i] = (*(frame + BUDDY_APP_VALUE_OFFSET + m_offset) << 24);
				packet->channels[i] |= (*(frame + BUDDY_APP_VALUE_OFFSET + m_offset + 1) << 16);
				packet->channels[i] |= (*(frame + BUDDY_APP_VALUE_OFFSET + m_offset + 2) << 8);
				packet->channels[i] |= *(frame + BUDDY_APP_VALUE_OFFSET + m_offset + 3);
			} else if (m_resolution == RESOLUTION_CTRL_HIGH) {
				if ((*(frame + BUDDY_APP_VALUE_OFFSET + m_offset)) & 0x80) {
					packet->channels[i] = ((*(frame + BUDDY_APP_VALUE_OFFSET + m_offset)) << 8);
					packet->channels[i] |= *(frame + BUDDY_APP_VALUE_OFFSET + m_offset + 1);
					
					// sign extension
					packet->channels[i] = packet->channels[i] | 0xFFFF0000;
				} else {
					packet->channels[i] = (*(frame + BUDDY_APP_VALUE_OFFSET + m_offset) << 8);
					packet->channels[i] |= *(frame + BUDDY_APP_VALUE_OFFSET + m_offset + 1);
				}
			} else if (m_resolution == RESOLUTION_CTRL_LOW) {
				packet->channels[i] = (*(frame + BUDDY_APP_VALUE_OFFSET + m_offset));
			} else {
				// resolution not defined, return error
				return CODEC_STATUS_ERROR;
			}

			m_offset += codec_get_data_size();
		}
	}

	m_decode_count++;

	// check if the next packet can fit in the packed array
	uint8_t decode_max_offset = (m_offset + (codec_get_data_size() * m_chan_number));

	// @todo: changed decode_count >= count to decode_count > count check if works
	if ((decode_max_offset >= (MAX_REPORT_SIZE - BUDDY_APP_VALUE_OFFSET)) || 
		(m_decode_count > count)) {
		m_offset = 0;
		m_decode_count = 0;
		return CODEC_STATUS_FULL;
	} else {
		return CODEC_STATUS_CONTINUE;
	}
}