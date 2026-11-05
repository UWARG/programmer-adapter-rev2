/*
 * can_queue.c
 *
 *  Created on: Nov 2, 2026
 *      Author: arthi
 */

#include <can_queue.h>

void CANQinit(CANQueue_t* canq, CANMessage_t* storage_queue, uint32_t size) {
	assert(size != 0 && (size & (size - 1)) == 0);

	canq->buffer = storage_queue;
	canq->capacity = size;
	canq->head = 0;
	canq->tail = 0;
}

bool CANQpush(CANQueue_t* canq, CANMessage_t data){
	if (((canq->head + 1) & (canq->capacity - 1)) == canq->tail) {
		return false;
	}

	canq->buffer[canq->head] = data;
	canq->head = (canq->head + 1) & (canq->capacity - 1);
	return true;
}

bool CANQpop(CANQueue_t* canq, CANMessage_t* data){
	if (canq->head == canq->tail) {
		return false;
	}

	*data = canq->buffer[canq->tail];
	canq->tail = (canq->tail + 1) & (canq->capacity - 1);
	return true;
}
