#include "error_reporter.h"

const char* GetErrorMessage(ErrorCode_t code) {
    switch (code) {
        // --- Tier 2: Runtime Error ---
        case ERROR_USB_TX_BUFFER_FULL:
            return "[ERR: USB_TX_FULL]\r\n";
        case ERROR_USB_RX_BUFFER_FULL:
            return "[ERR: USB_RX_FULL]\r\n";
        case ERROR_CAN_TX_BUFFER_FULL:
            return "[ERR: CAN_TX_FULL]\r\n";
        case ERROR_CAN_RX_BUFFER_FULL:
            return "[ERR: CAN_RX_FULL]\r\n";
        case ERROR_CAN_TX_RETRIES_EXCEEDED:
            return "[ERR: CAN_BUS_OFF]\r\n";
        case ERROR_USB_TX_RETRIES_EXCEEDED:
            return "[ERR: USB_TX_RETRY]\r\n";

        // --- Hardware Warnings / Non-Fatal Exceptions ---
        case ERROR_CAN_HW_OVERFLOW:
            return "[WARN: CAN_HW_OVERFLOW]\r\n";
        case ERROR_USB_PMA_OVERRUN:
            return "[WARN: USB_PMA_OVERRUN]\r\n";
        case ERROR_CAN_READ_FAILED:
            return "[ERR: CAN_READ_FAIL]\r\n";

        // --- Tier 1: Initialization Fatal Faults ---
        case ERROR_CAN_FILTER_CONFIG:
            return "[FATAL: CAN_FILTER_FAIL]\r\n";
        case ERROR_CAN_START:
            return "[FATAL: CAN_START_FAIL]\r\n";
        case ERROR_CAN_RX_NOTIFICATION:
            return "[FATAL: CAN_NOTIF_FAIL]\r\n";

        // Fallback for safety
        default:
            return "[ERR: UNKNOWN_FAULT]\r\n";
    }
}

void report_tier1_fatal(ErrorCode_t error_code) {
    CAN_SafeShutdown();

    const char* error_mssg = GetErrorMessage(error_code);
    usb_tx_push_error(&usb_tx_buffer, error_mssg, strlen(error_mssg));

    uint32_t safety_timeout = 2000000; // Generous fallback counter (e.g., ~100-200ms)
    
    while (safety_timeout > 0) {
        bool sw_buffer_empty = CBIsEmpty(&usb_tx_buffer);
        bool hw_idle = (hcdc->TxState == 0);

        if (sw_buffer_empty && hw_idle) {
            break;
        }

        safety_timeout--;
    }

    __disable_irq();

    CBreset(&usb_rx_buffer);
    CANQreset(&can_tx_queue);
    CANQreset(&can_rx_queue);

    while (1) {
        __NOP();
    }
}