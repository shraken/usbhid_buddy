#ifndef _BUDDY_CODEC
#define _BUDDY_CODEC

/**
 * @author Nicholas L Shrake
 * @date 12-21-2018
 * @brief Buddy codec encode, decode and support routines.  Both firmware
 *  and software use this component.  
 * 
 */

#include <stdint.h>
#include "buddy_common.h"

#define CODEC_FIXED_SIZE 2

int codec_init(uint8_t chan_mask, uint8_t resolution);
void codec_reset(void);

void codec_set_data_size(const uint8_t data_size);
uint8_t codec_get_data_size(void);

void codec_set_offset_count(const uint8_t offset);
uint8_t codec_get_offset_count(void);

void codec_count_channels(void);
uint8_t codec_get_channel_count(void);

uint8_t codec_get_encode_count(void);
void codec_set_encode_count(const uint8_t count);

uint8_t codec_get_decode_count(void);
void codec_set_decode_count(const uint8_t count);

void codec_set_channel_active(const uint8_t channel, bool state);
bool codec_is_channel_active(const uint8_t channel);

int codec_encode(uint8_t *frame, general_packet_t *packet);
int codec_encode_imm(void);
int codec_decode(uint8_t *frame, general_packet_t *packet);
int codec_decode_imm(void);

#endif