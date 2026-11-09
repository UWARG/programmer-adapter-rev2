/*
 * circular_buffer.c
 *
 *  Created on: Oct 31, 2026
 *      Author: arthi
 */

#include <circular_buffer.h>

void CBinit(CircularBuffer_t* cb, uint8_t* storage_array, uint32_t size) {
	assert(size != 0 && (size & (size - 1)) == 0);

	cb->buffer = storage_array;
	cb->capacity = size;
	cb->head = 0;
	cb->tail = 0;
}

bool CBpush(CircularBuffer_t* cb, uint8_t data) {
	if (((cb->head + 1) & (cb->capacity - 1)) == cb->tail) {
		return false;
	}

	cb->buffer[cb->head] = data;
	cb->head = (cb->head + 1) & (cb->capacity - 1);
	return true;
}

bool CBpop(CircularBuffer_t* cb, uint8_t* data) {
	if (cb->head == cb->tail) {
		return false;
	}

	*data = cb->buffer[cb->tail];
	cb->tail = (cb->tail + 1) & (cb->capacity - 1);
	return true;
}

bool CBpop_upto(CircularBuffer_t* cb, uint32_t limit_head, uint8_t* data) {
    if (limit_head == cb->tail) {
        return false;
    }

    *data = cb->buffer[cb->tail];
    cb->tail = (cb->tail + 1) & (cb->capacity - 1);
    return true;
}
