/*
 * error_codes.h
 *
 *  Created on: Nov 7, 2026
 *      Author: Tejas
 */

#ifndef INC_ERROR_CODES_H_
#define INC_ERROR_CODES_H_

typedef enum {
    // --- TIER 2: RUNTIME CATASTROPHES (Triggers Purge-and-Sync & \BEL) ---
    // Includes both software bottlenecks and fatal hardware blocks
    ERROR_USB_TX_BUFFER_FULL      = 1,  // USB software TX buffer overflowed (Host stall)
    ERROR_USB_RX_BUFFER_FULL      = 2,  // USB software RX buffer overflowed (STM32 bottleneck)
    ERROR_CAN_TX_BUFFER_FULL      = 3,  // CAN software TX queue overflowed
    ERROR_CAN_RX_BUFFER_FULL      = 4,  // CAN software RX queue overflowed
    ERROR_CAN_TX_RETRIES_EXCEEDED = 5,  // CAN frame failed completely / Bus-Off (Fatal during flash)
    ERROR_USB_TX_RETRIES_EXCEEDED = 6,  // USB endpoint write failed completely

    // --- HARDWARE WARNINGS / NON-FATAL EXCEPTIONS (Logged to stats, no reset) ---
    ERROR_CAN_HW_OVERFLOW         = 7,  // CAN hardware RX FIFO overwritten (dropped a periodic frame)
    ERROR_USB_PMA_OVERRUN         = 8,  // USB packet memory hiccup handled by driver
    ERROR_CAN_READ_FAILED         = 9,  // Low-level read error from mailbox

    // --- TIER 1: INITIALIZATION FATAL FAULTS (Pre-Run / Boot-up) ---
    ERROR_CAN_FILTER_CONFIG       = 10, // CAN filter configuration failed during init
    ERROR_CAN_START               = 11, // CAN peripheral start failed
    ERROR_CAN_RX_NOTIFICATION     = 12  // CAN RX interrupt registration failed
} ErrorCode_t;

#endif /* INC_ERROR_CODES_H_ */