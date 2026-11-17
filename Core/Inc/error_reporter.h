/*
 * error_reporter.h
 *
 *  Created on: Nov 9, 2026
 *      Author: Tejas
 */

#ifndef INC_ERROR_REPORTER_H_
#define INC_ERROR_REPORTER_H_

#include "error_codes.h"
#include <string.h>
#include <circular_buffer.h>
#include <can_queue.h>
#include "usbd_cdc_if.h"

extern void CAN_SafeShutdown(void);
extern void usb_tx_push_error(CircularBuffer_t* cb, const char* error_mssg, uint16_t max_length);
extern USBD_CDC_HandleTypeDef *hcdc;
extern CircularBuffer_t usb_rx_buffer;
extern CircularBuffer_t usb_tx_buffer;
extern CANQueue_t can_tx_queue;
extern CANQueue_t can_rx_queue;

const char* GetErrorMessage(ErrorCode_t code);

void report_tier1_fatal(ErrorCode_t error_code);

void report_tier2_runtime(ErrorCode_t error_code);

void report_warning(ErrorCode_t warning_code);

#endif /* INC_ERROR_REPORTER_H_ */
