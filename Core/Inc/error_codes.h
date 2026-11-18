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

    // --- TIER 1: INITIALIZATION FATAL FAULTS (Pre-Run / Boot-up) ---
    ERROR_CAN_FILTER_CONFIG       = 7, // CAN filter configuration failed during init
    ERROR_CAN_START               = 8, // CAN peripheral start failed
    ERROR_CAN_RX_NOTIFICATION     = 9  // CAN RX interrupt registration failed
} ErrorCode_t;

typedef enum {
    // --- HARDWARE WARNINGS / NON-FATAL EXCEPTIONS (Logged to stats, no reset) ---
    WARN_CAN_HW_OVERFLOW         = 0,  // CAN hardware RX FIFO overwritten (dropped a periodic frame)
    WARN_USB_PMA_OVERRUN         = 1,  // USB packet memory hiccup handled by driver
    WARN_CAN_READ_FAILED         = 2,  // Low-level read error from mailbox
} WarningCode_t;

#endif /* INC_ERROR_CODES_H_ */