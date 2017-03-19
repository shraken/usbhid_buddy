#ifndef  _DATA_H
#define  _DATA_H

#include <stdint.h>
#include <buddy.h>

#define BUDDY_QUEUE_SIZE 40

typedef struct _queue {
		int32_t head;
		int32_t tail;
		int32_t max_size;
		buddy_frame_t *buffer;
} buddy_queue;

void queue_init();
void queue_clear();
int32_t queue_remain_items();
void queue_enqueue(buddy_frame_t *p);
buddy_frame_t *queue_dequeue();
int8_t queue_is_empty();
int8_t queue_is_full();

#endif