/*
 * slcan.h
 *
 *  Created on: Nov 1, 2026
 *      Author: arthi
 */

#ifndef INC_CAN_TRANSLATOR_H_
#define INC_CAN_TRANSLATOR_H_

#include <error_reporter.h>
#include <circular_buffer.h>
#include <can_queue.h>


static const char hexToCharLUT[] = "0123456789ABCDE";


bool charToHex(char c, uint8_t* out_val);

bool SLCANToCAN(CircularBuffer_t* cb, CANQueue_t* canq, uint16_t* frames_queued);

void hexToChar(uint8_t hex, char* out_val);

bool CANToSLCAN(CircularBuffer_t* cb, CANQueue_t* canq);


#endif /* INC_CAN_TRANSLATOR_H_ */
