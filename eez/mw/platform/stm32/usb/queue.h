#pragma once

#include <stdint.h>

#define BUFFER_SIZE 2048

typedef struct {
	uint8_t buffer[BUFFER_SIZE];
    volatile uint16_t tail;
    volatile uint16_t head;
    volatile uint8_t overflow;
} Queue;

void queue_init(Queue *q);
void queue_push(Queue *q, uint8_t data);
uint8_t queue_pop(Queue *q, uint8_t *p_data);
uint16_t queue_available(Queue *q);
