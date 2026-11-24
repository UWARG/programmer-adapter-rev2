/*
 * error_codes.h
 *
 *  Created on: Nov 7, 2026
 *      Author: Tejas
 */

#ifndef INC_ERROR_CODES_H_
#define INC_ERROR_CODES_H_

typedef enum {
    ERROR_NONE                    = 0,
    // --- TIER 2: RUNTIME CATASTROPHES (Triggers Purge-and-Sync & \BEL) ---
    // Includes software bottlenecks, queue overflows, and protocol/syntax violations
    ERROR_USB_TX_BUFFER_FULL      = 1,  // USB software TX buffer overflowed (Host stall)
    ERROR_USB_RX_BUFFER_FULL      = 2,  // USB software RX buffer overflowed (STM32 bottleneck)
    ERROR_CAN_TX_BUFFER_FULL      = 3,  // CAN software TX queue overflowed
    ERROR_CAN_TX_RETRIES_EXCEEDED = 4,  // CAN frame failed completely / Bus-Off (Fatal during flash)
    
    // --- SLCAN Protocol & Formatting Errors (Tier 2) ---
    ERROR_SLCAN_INVALID_CMD      = 5,  // Unknown ASCII command character received from host
    ERROR_SLCAN_INVALID_DLC      = 6,  // CAN frame DLC > 8 or malformed length nibble
    ERROR_SLCAN_LINE_BUFFER_FULL = 7,  // Line buffer overflowed without receiving '\r'
    ERROR_SLCAN_INVALID_HEX      = 8,  // Non-hex character found in incoming host payload/ID
    ERROR_SLCAN_ENCODE_FAILED    = 9, // Internal firmware error: failed to convert nibble to ASCII for transmission

    // --- TIER 1: INITIALIZATION FATAL FAULTS (Pre-Run / Boot-up) ---
    ERROR_CAN_FILTER_CONFIG       = 10, // CAN filter configuration failed during init
    ERROR_CAN_START               = 11, // CAN peripheral start failed
    ERROR_CAN_RX_NOTIFICATION     = 12, // CAN RX interrupt registration failed
    ERROR_CAN_BUS_OFF             = 13  // CAN controller entered bus-off during TX
} ErrorCode_t;

typedef enum {
    // --- HARDWARE WARNINGS / NON-FATAL EXCEPTIONS (Logged to stats, no reset) ---
    WARN_CAN_HW_OVERFLOW         = 0,  // CAN hardware RX FIFO overwritten (dropped a periodic frame)
    WARN_USB_PMA_OVERRUN         = 1,  // USB packet memory hiccup handled by driver
    WARN_CAN_READ_FAILED         = 2,  // Low-level read error from mailbox
    WARN_CAN_INVALID_DLC         = 3,  // Corrupted CAN frame received from bus with DLC > 8 (dropped)
    WARN_CAN_RX_BUFFER_FULL      = 4,  // CAN software RX queue overflowed
    WARN_CAN_TX_FAILED,
    WARN_CAN_ARBITRATION_LOST,
    WARN_CAN_ACK,
    WARN_CAN_CRC,
    WARN_CAN_STUFF,
    WARN_CAN_FORM,
    WARN_CAN_BIT_RECESSIVE,
    WARN_CAN_BIT_DOMINANT,
    WARN_CAN_ERROR_WARNING,
    WARN_CAN_ERROR_PASSIVE,
    WARN_CAN_BUS_OFF
} WarningCode_t;

#endif /* INC_ERROR_CODES_H_ */
