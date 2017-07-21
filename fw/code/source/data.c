#include <data.h>
#include <string.h>

static buddy_frame_t queue_buffer[BUDDY_QUEUE_SIZE];
buddy_queue queue;

void queue_init()
{
	queue_clear();
	queue.buffer = &queue_buffer[0];
	queue.max_size = BUDDY_QUEUE_SIZE;
}

void queue_clear()
{
	queue.head = -1;
	queue.tail = -1;
}

int16_t queue_number_items(void)
{
	return ((queue.tail - queue.head));
}

int16_t queue_remain_items(void)
{
	return (queue.max_size - queue_number_items() - 1);
}

int8_t queue_enqueue(buddy_frame_t *p)
{
	if (!queue_is_full()) {
		queue.tail++;
		memcpy(&queue.buffer[queue.tail % queue.max_size], p, sizeof(buddy_frame_t));
	} else {
		return -1;
	}
}

buddy_frame_t *queue_dequeue()
{
	if (!queue_is_empty()) {
		queue.head++;
		return (buddy_frame_t *) &queue.buffer[queue.head % queue.max_size];	
	} else {
		return NULL;
	}
}

int8_t queue_is_empty()
{
	return (queue.head == queue.tail);
}

int8_t queue_is_full()
{
	return ((queue.tail - (queue.max_size - 1)) == queue.head);
}