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
#include <can_rx_tx_handler.h>
#include "usbd_cdc_if.h"
#include "usb_device.h"

typedef enum {
    BRIDGE_STATE_NORMAL = 0,
    BRIDGE_STATE_WAITING_FOR_SYNC
} BridgeState_t;

extern void CAN_SafeShutdown(void);

extern USBD_HandleTypeDef hUsbDeviceFS;
extern volatile BridgeState_t current_bridge_state;
extern USBD_CDC_HandleTypeDef *hcdc;
extern CircularBuffer_t usb_rx_buffer;
extern CircularBuffer_t usb_tx_buffer;
extern CANQueue_t can_tx_queue;
extern CANQueue_t can_rx_queue;

const char* getErrorMessage(ErrorCode_t code);

const char* getWarningMessage(WarningCode_t code);

void handle_diagnostic_request(void);

void report_tier1_fatal(ErrorCode_t error_code);

void report_tier2_runtime(ErrorCode_t error_code);

void report_warning(WarningCode_t warning_code);

#endif /* INC_ERROR_REPORTER_H_ */
