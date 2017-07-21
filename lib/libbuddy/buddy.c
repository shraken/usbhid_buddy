#include <buddy.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#if defined(C8051)
#include <gpio.h>
#endif

static buddy_frame_t codec_frame;
static uint16_t codec_bit_offset;

static bool m_codec_initialized;
static uint8_t m_resolution_byte_size;
static uint8_t m_resolution;
static uint16_t m_mask_resolution;
static uint8_t m_channels_mask;
static uint8_t m_channels_count;
static bool m_codec_stream = false;

int codec_init(bool streaming, uint8_t channels_mask, uint8_t resolution)
{
	int i;

	m_codec_initialized = true;

	m_codec_stream = streaming;
	m_resolution = resolution;
	m_channels_mask = channels_mask;

	m_mask_resolution = 0;
	for (i = 0; i < resolution; i++) {
		m_mask_resolution |= (1 << i);
	}

	m_channels_count = 0;
	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (m_channels_mask & (1 << i)) {
			m_channels_count++;
		}
 	}

	codec_reset();
	codec_clear();

	#if defined(C8051)
	//printf("m_codec_initialized = %bd\r\n", m_codec_initialized);
	//printf("m_codec_stream = %bd\r\n", m_codec_stream);
	//printf("m_resolution = %bd (0x%bx)\r\n", m_resolution, m_resolution);
	//printf("m_channels_mask = %bd (0x%bx)\r\n", m_channels_mask, m_channels_mask);
	//printf("m_mask_resolution = %bd (0x%bx)\r\n", m_mask_resolution, m_mask_resolution);
	//printf("m_channels_count = %bd (0x%bx)\r\n", m_channels_count, m_channels_count);
	#endif
	
	return CODEC_STATUS_NOERR;
}

int encode_packet(general_packet_t *packet)
{
	uint16_t channel_temp;
	uint8_t future_div_base_index;
	uint8_t norm_div_base_index;
	uint8_t mod_base_index;
	uint8_t lo_byte;
	uint8_t hi_byte;
	int status_code;
	int i;
	int overflow_index;
	
	//printf("encode_packet() entered\n");
	//printf("m_channels_mask = %02x\n", m_channels_mask);

	//printf("codec_frame.count = %d\n", codec_frame.count);
	overflow_index = m_resolution - BUDDY_BIT_SIZE;

	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		// check if channel value passed is active
		if (m_channels_mask & (1 << i)) {
			norm_div_base_index = (codec_bit_offset / BUDDY_BIT_SIZE);
			future_div_base_index = ((codec_bit_offset + BUDDY_BIT_SIZE) / BUDDY_BIT_SIZE);
			
			mod_base_index = (codec_bit_offset % BUDDY_BIT_SIZE);
			
			channel_temp = (packet->channels[i] << mod_base_index);
			
			//printf("packet->channels[%d] = %d\r\n", i, packet->channels[i]);
			//printf("channel_temp = %d\r\n", channel_temp);
			//printf("mod_base_index = %d\r\n", mod_base_index);
			
			lo_byte = (channel_temp & 0x00FF);	
			hi_byte = ((channel_temp & 0xFF00) >> 8);
			
			/*
			#if defined(C8051)
			printf("lo_byte = %02bx, hi_byte = %02bx\r\n", lo_byte, hi_byte);
			#else
			printf("lo_byte = %02x, hi_byte = %02x\r\n", lo_byte, hi_byte);
			#endif
			*/

			//printf("overflow_index = %ld\r\n", overflow_index);
			if (overflow_index < 0) {
				// trying to pack a bit width smaller then 
				// 8bit into an 8bit aligned

				if (norm_div_base_index != future_div_base_index) {
					//printf("norm_div_base_index = %bd\r\n", norm_div_base_index);
					//printf("norm_div_base_index + 1 = %bd\r\n", norm_div_base_index + 1);

					// bit packing overflows onto two bytes, must mask and append
					codec_frame.payload[norm_div_base_index] |= lo_byte;
					codec_frame.payload[norm_div_base_index + 1] |= hi_byte;
				} else {
					//printf("norm_div_base_index = %bd\r\n", norm_div_base_index);

					// bit packing does not overflow.  mask single byte
					codec_frame.payload[norm_div_base_index] |= lo_byte;
				}
			} else {
				// pack a bit width greater than 8bit into an 8bit
				// aligned
				//printf("norm_div_base_index = %bd\r\n", norm_div_base_index);
				//printf("norm_div_base_index + 1 = %bd\r\n", norm_div_base_index + 1);

				codec_frame.payload[norm_div_base_index] |= lo_byte;
				codec_frame.payload[norm_div_base_index + 1] |= hi_byte;
			}
			
			codec_bit_offset += m_resolution;
		}
	}
	
	//printf("encode codec_bit_offset = %d\r\n", codec_bit_offset);

	// check if the next packet can fit in the packed array
	if ((codec_bit_offset + (m_resolution * m_channels_count)) > ((MAX_REPORT_SIZE - 3) * BUDDY_BIT_SIZE)) {
		status_code = CODEC_STATUS_FULL;
	} else {
		codec_frame.count++;
		status_code = CODEC_STATUS_CONTINUE;
	}
	
	return status_code;
}

int decode_packet(buddy_frame_t *frame, general_packet_t *packet)
{
	uint16_t channel_temp;
	uint8_t future_div_base_index;
	uint8_t norm_div_base_index;
	uint8_t mod_base_index;
	uint8_t lo_byte;
	uint8_t hi_byte;
	int i;
	int overflow_index;
	
	//printf("frame->count = %bd\r\n", frame->count);
	overflow_index = m_resolution - BUDDY_BIT_SIZE;
	//printf("overflow_index = %d\r\n", overflow_index);
	
	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		// check if channel value passed is active
		if (!(m_channels_mask & (1 << i))) {
				continue;
		}
		
		norm_div_base_index = (codec_bit_offset / BUDDY_BIT_SIZE);
		future_div_base_index = ((codec_bit_offset + BUDDY_BIT_SIZE) / BUDDY_BIT_SIZE);
		mod_base_index = (codec_bit_offset % BUDDY_BIT_SIZE);

		//printf("norm_div_base_index = %bd\r\n", norm_div_base_index);
		//printf("future_div_base_index = %bd\r\n", future_div_base_index);
		lo_byte = frame->payload[norm_div_base_index];
		hi_byte = frame->payload[norm_div_base_index + 1];

		//printf("mod_base_index = %bd\r\n", mod_base_index);
		//printf("lo_byte = %02bx, hi_byte = %02bx\r\n", lo_byte, hi_byte);
		if (overflow_index < 0) {
			//printf("path A\r\n");
			if (norm_div_base_index != future_div_base_index) {
				//printf("path B\r\n");
				channel_temp = ((hi_byte << 8) | lo_byte);
				//printf("channel_temp = %x\r\n", channel_temp);
				channel_temp = channel_temp >> mod_base_index;
				//printf("channel_temp = %x\r\n", channel_temp);
				channel_temp = channel_temp & m_mask_resolution;
				//printf("channel_temp = %x\r\n", channel_temp);
			}
			else {
				//printf("path C\r\n");
				channel_temp = lo_byte;
				channel_temp = channel_temp & m_mask_resolution;
			}
		}
		else {
			//printf("path D\r\n");
			channel_temp = ((hi_byte << 8) | lo_byte);
			//printf("channel_temp = %x\r\n", channel_temp);
			channel_temp = channel_temp >> mod_base_index;
			//printf("channel_temp = %x\r\n", channel_temp);
			channel_temp = channel_temp & m_mask_resolution;
			//printf("channel_temp = %x\r\n", channel_temp);
		}

		packet->channels[i] = channel_temp;
		codec_bit_offset += m_resolution;
	}

	// check if the next packet can fit in the packed array
	if ( (codec_bit_offset + (m_resolution * m_channels_count)) > 
			 (m_channels_count * m_resolution * frame->count)) {
		return CODEC_STATUS_FULL;
	} else {
		return CODEC_STATUS_CONTINUE;
	}
}

buddy_frame_t *codec_get_buffer(void)
{
	return &codec_frame;
}

void codec_reset(void)
{
	codec_bit_offset = 0;
}

void codec_clear(void)
{
	memset(&codec_frame, 0, sizeof(buddy_frame_t));
	codec_frame.count = 0;
}

void codec_dump(void)
{
	int i;

	printf("codec_frame.payload = ");
	for (i = 0; i < (MAX_REPORT_SIZE - 1); i++) {
		printf("%02x:", codec_frame.payload[i]);
	}
	printf("\r\n");
}

bool codec_buffer_empty(void)
{
	return (codec_bit_offset == 0);
}

bool is_codec_initialized(void)
{
	return m_codec_initialized;
}