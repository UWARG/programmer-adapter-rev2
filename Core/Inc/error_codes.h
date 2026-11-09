/*
 * error_codes.h
 *
 *  Created on: Nov 7, 2026
 *      Author: Tejas
 */

#ifndef INC_ERROR_CODES_H_
#define INC_ERROR_CODES_H_

enum {
    ERROR_USB_BUFFER_FULL     = 1,
    // USB SW buffer overflowed when pushing from HW buffer
    ERROR_CAN_BUFFER_FULL     = 2,
    // CAN SW buffer overflowed when pushing from HW buffer
    ERROR_CAN_FILTER_CONFIG   = 3,
    // CAN filter configuration failed
    ERROR_CAN_START           = 4,
    // CAN start failed
    ERROR_CAN_RX_NOTIFICATION = 5,
    // CAN RX notification setup failed
    ERROR_CAN_READ_FAILED = 6,
    // CAN read from HW buffer failed
    ERROR_CAN_HW_OVERFLOW = 7,
    // CAN RX FIFO0 overflowed before frames could enter the SW queue
    ERROR_USB_PMA_OVERRUN = 8,
    // USB packet-memory RX overrun or TX underrun
    ERROR_CAN_TX_RETRIES_EXCEEDED = 9,
    // CAN transmission failed after the configured retries
    ERROR_USB_TX_RETRIES_EXCEEDED = 10,
    // USB transmission failed after the configured retries
};

#endif /* INC_ERROR_CODES_H_ */
