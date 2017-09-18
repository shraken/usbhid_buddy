#include <data.h>
#include <string.h>
#include <c8051f3xx.h>

static buddy_frame_t xdata queue_internal_buffer[BUDDY_QUEUE_INTERNAL_SIZE];

buddy_queue data queue_internal;

void queue_init()
{
	queue_clear(&queue_internal);

	queue_internal.buffer = &queue_internal_buffer[0];
	queue_internal.type = BUDDY_QUEUE_TYPE_INTERNAL;
	queue_internal.max_size = BUDDY_QUEUE_INTERNAL_SIZE;
}

void queue_clear(buddy_queue *queue)
{
	queue->head = -1;
	queue->tail = -1;
}

int16_t queue_number_items(buddy_queue *queue)
{
	return ((queue->tail - queue->head));
}

int16_t queue_remain_items(buddy_queue *queue)
{
	return (queue->max_size - queue_number_items(queue) - 1);
}

int8_t queue_enqueue(buddy_queue *queue, buddy_frame_t *p)
{
	if (!queue_is_full(queue)) {
		queue->tail++;
		
		//P3 = P3 & ~0x40;
    memcpy(&queue->buffer[queue->tail % queue->max_size], p, sizeof(buddy_frame_t));
    //P3 = P3 | 0x40;
	} else {
		return -1;
	}
	
	return 0;
}

buddy_frame_t *queue_dequeue(buddy_queue *queue)
{
	if (!queue_is_empty(queue)) {
		queue->head++;
		
		if (queue->type == BUDDY_QUEUE_TYPE_INTERNAL) {
			return (buddy_frame_t *) &queue->buffer[queue->head % queue->max_size];	
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}

int8_t queue_is_empty(buddy_queue *queue)
{
	return (queue->head == queue->tail);
}

int8_t queue_is_full(buddy_queue *queue)
{
	return ((queue->tail - (queue->max_size - 1)) == queue->head);
}