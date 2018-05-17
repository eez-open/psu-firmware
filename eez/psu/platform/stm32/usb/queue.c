#include "queue.h"

void queue_init(Queue *q) {
	q->tail = 0;
	q->head = 0;
	q->overflow = 0;
}

void queue_push(Queue *q, uint8_t data) {
    uint16_t head = q->head + 1;
    if (head >= BUFFER_SIZE) {
    	head = 0;
    }
    q->buffer[head] = data;
    q->head = head;
    if (q->head == q->tail) {
        q->overflow = 1;
    }

}

uint8_t queue_pop(Queue *q, uint8_t *p_data) {
    if(q->tail != q->head) {
    	uint16_t tail = q->tail + 1;
        if (tail >= BUFFER_SIZE) {
        	tail = 0;
        }
        q->tail = tail;
        *p_data = q->buffer[tail];
        return 1;
    }
    return 0;
}

uint16_t queue_available(Queue *q) {
	return q->head - q->tail;
}
