#ifndef  _DATA_H
#define  _DATA_H

#include <stdint.h>
#include <buddy.h>

#define BUDDY_QUEUE_INTERNAL_SIZE 5

/**
 * \enum BUDDY_QUEUE_TYPE
 * \brief specifies if the type of the queue data structure.  If internal
 *			then a local memory copy (memcpy) is performed to store and
 *			retrieve the queue contents.  But, if external then the store
 *			and retrieve is performed from an external SPI SRAM memory device.
 * @see buddy_queue
 */
typedef enum _BUDDY_QUEUE_TYPE {
	BUDDY_QUEUE_TYPE_INTERNAL = 0,
	BUDDY_QUEUE_TYPE_EXTERNAL,
} BUDDY_QUEUE_TYPE;

typedef struct _queue {
	int16_t head;
	int16_t tail;
	uint16_t max_size;
	uint8_t type;
	buddy_frame_t *buffer;
} buddy_queue;

void queue_init();
void queue_clear(buddy_queue *queue);
int16_t queue_number_items(buddy_queue *queue);
int16_t queue_remain_items(buddy_queue *queue);
int8_t queue_enqueue(buddy_queue *queue, buddy_frame_t *p);
buddy_frame_t *queue_dequeue(buddy_queue *queue);
int8_t queue_is_empty(buddy_queue *queue);
int8_t queue_is_full(buddy_queue *queue);

#endif