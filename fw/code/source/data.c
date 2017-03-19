#include <data.h>
#include <string.h>

static uint8_t queue_lock_flag = 0;
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

int32_t queue_remain_items()
{
		return ((queue.tail - queue.head) + 1);
}

void queue_enqueue(buddy_frame_t *p)
{
		queue.tail++;
		memcpy(&queue.buffer[queue.tail % queue.max_size], p, sizeof(buddy_frame_t));
}

buddy_frame_t *queue_dequeue()
{
		queue.head++;
		return (buddy_frame_t *) &queue.buffer[queue.head % queue.max_size];
}

int8_t queue_is_empty()
{
		return (queue.head == queue.tail);
}

int8_t queue_is_full()
{
		return ((queue.tail - (queue.max_size - 1)) == queue.head);
}