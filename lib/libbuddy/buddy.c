#include <buddy.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#if defined(C8051)
#include <gpio.h>
#endif

#if defined(C8051)
#define XDATA_TYPE xdata
#define DATA_TYPE data
#else
#define XDATA_TYPE
#define DATA_TYPE
#endif

static XDATA_TYPE buddy_frame_t codec_frame;
static uint16_t DATA_TYPE codec_bit_offset;
static uint8_t DATA_TYPE codec_byte_offset = 0;

static bool XDATA_TYPE m_codec_initialized;
static uint8_t XDATA_TYPE m_resolution_byte_size;
static uint8_t DATA_TYPE m_resolution;
static uint8_t XDATA_TYPE m_codec;
static uint16_t XDATA_TYPE m_mask_resolution;
static uint8_t XDATA_TYPE m_channels_mask;
static uint8_t DATA_TYPE m_channels_count;
static bool XDATA_TYPE m_codec_stream = false;

uint8_t DATA_TYPE m_codec_chan_enable[BUDDY_CHAN_LENGTH];

int codec_init(bool streaming, uint8_t enabled, uint8_t channels_mask, uint8_t resolution)
{
	int XDATA_TYPE i;

	m_codec_initialized = true;

	m_codec_stream = streaming;
	m_codec = enabled;
	m_resolution = resolution;
	m_channels_mask = channels_mask;

	m_mask_resolution = 0;
	for (i = 0; i < resolution; i++) {
		m_mask_resolution |= (1 << i);
	}

	m_channels_count = 0;
	for (i = BUDDY_CHAN_0; i <= BUDDY_CHAN_7; i++) {
		if (m_channels_mask & (1 << i)) {
			m_codec_chan_enable[i] = 1;
			m_channels_count++;
    } else {
			m_codec_chan_enable[i] = 0;
		}
 	}

	//codec_reset();
	//codec_clear();

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

buddy_frame_t *codec_get_buffer(void)
{
	return &codec_frame;
}

void codec_reset(void)
{
	codec_bit_offset = 0;
	codec_byte_offset = 0;
}

void codec_clear(void)
{
	memset(&codec_frame, 0, sizeof(buddy_frame_t));
	codec_frame.count = 0;
}

void codec_dump(void)
{
	int XDATA_TYPE i;

	printf("codec_frame.payload = ");
	for (i = 0; i < (MAX_REPORT_SIZE - 1); i++) {
		printf("%02x:", codec_frame.payload[i]);
	}
	printf("\r\n");
}

bool codec_buffer_empty(void)
{
	if (m_codec == CODEC_CTRL_ENABLED) {
		return (codec_bit_offset == 0);
	} else {
		return (codec_byte_offset == 0);
	}
}

bool is_codec_initialized(void)
{
	return m_codec_initialized;
}