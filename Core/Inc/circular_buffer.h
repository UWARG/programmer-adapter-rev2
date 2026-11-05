/*
 * circular_buffer.h
 *
 *  Created on: Oct 31, 2026
 *      Author: arthi
 */

#ifndef INC_CIRCULAR_BUFFER_H_
#define INC_CIRCULAR_BUFFER_H_

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>


typedef struct {
	uint8_t* buffer;
	uint32_t capacity;
	volatile uint32_t head;
	volatile uint32_t tail;
}CircularBuffer_t;


void CBinit(CircularBuffer_t* cb, uint8_t* storage_array, uint32_t size);

bool CBpush(CircularBuffer_t* cb, uint8_t data);

bool CBpop(CircularBuffer_t* cb, uint8_t* data);

bool CBpop_upto(CircularBuffer_t* cb, uint32_t limit_head, uint8_t* data);

#endif /* INC_CIRCULAR_BUFFER_H_ */
