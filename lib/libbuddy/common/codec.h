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

void codec_reset(void);
void codec_set_config(void);
int codec_count_channels(void);
int codec_encode(uint8_t *frame, general_packet_t *packet);
int codec_decode(uint8_t *frame, general_packet_t *packet);

uint8_t codec_get_encode_count(void);
void codec_set_encode_count(const uint8_t count);

#endif