/*
 * slcan.h
 *
 *  Created on: Nov 1, 2026
 *      Author: arthi
 */

#ifndef INC_CAN_TRANSLATOR_H_
#define INC_CAN_TRANSLATOR_H_

#include <circular_buffer.h>
#include <can_queue.h>


typedef struct {
    uint32_t buffer_overflows;
    uint32_t invalid_command_headers;
    uint32_t invalid_dlc_errors;
} CANTranslatorStats_t;

static const char hextoCharLUT[] = "0123456789ABCDE";


bool charToHex(char c, uint8_t* out_val);

bool SLCANToCAN(CircularBuffer_t* cb, CANQueue_t* canq);

bool hextoChar(uint8_t hex, char* out_val);

bool CANToSLCAN(CircularBuffer_t* cb, CANQueue_t* canq);


#endif /* INC_CAN_TRANSLATOR_H_ */
