/*
 * can_queue.h
 *
 *  Created on: Nov 2, 2026
 *      Author: arthi
 */

#ifndef INC_CAN_QUEUE_H_
#define INC_CAN_QUEUE_H_

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>


typedef struct {
	uint32_t id;
	uint8_t dlc;
	uint8_t payload[8];
	bool is_extended;
	uint8_t retries;
}CANMessage_t;

typedef struct {
	CANMessage_t* buffer;
	uint32_t capacity;
	volatile uint32_t head;
	volatile uint32_t tail;
}CANQueue_t;


void CANQinit(CANQueue_t* canq, CANMessage_t* storage_queue, uint32_t size);

bool CANQpush(CANQueue_t* canq, CANMessage_t data);

bool CANQpop(CANQueue_t* canq, CANMessage_t* data);

bool CANQpop_upto(CANQueue_t* canq, uint32_t limit_head, CANMessage_t* data);


#endif /* INC_CAN_QUEUE_H_ */
