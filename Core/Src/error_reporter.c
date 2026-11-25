#include "error_reporter.h"

static volatile uint32_t warning_counts[WARNING_MAX_COUNT] = {0};

const char* GetErrorMessage(ErrorCode_t code) {
    switch (code) {
        // --- Tier 2: Runtime Error ---
        case ERROR_USB_TX_BUFFER_FULL:
            return "\a[ERR: USB_TX_FULL]\r\n";
        case ERROR_USB_RX_BUFFER_FULL:
            return "\a[ERR: USB_RX_FULL]\r\n";
        case ERROR_CAN_TX_BUFFER_FULL:
            return "\a[ERR: CAN_TX_FULL]\r\n";
        case ERROR_CAN_TX_RETRIES_EXCEEDED:
            return "\a[ERR: CAN_BUS_OFF]\r\n";
            
        // --- SLCAN Protocol Strings ---
        case ERROR_SLCAN_INVALID_CMD:
            return "\a[ERR: SLCAN_BAD_CMD]\r\n";
        case ERROR_SLCAN_INVALID_DLC:
            return "\a[ERR: SLCAN_BAD_DLC]\r\n";
        case ERROR_SLCAN_LINE_BUFFER_FULL:
            return "\a[ERR: SLCAN_BUF_OVF]\r\n";
        case ERROR_SLCAN_INVALID_HEX:
            return "\a[ERR: SLCAN_BAD_HEX]\r\n";
        case ERROR_SLCAN_ENCODE_FAILED:
            return "\a[ERR: SLCAN_ENCODE_FAIL]\r\n";

        // --- Tier 1: Initialization Fatal Faults ---
        case ERROR_CAN_FILTER_CONFIG:
            return "\a[FATAL: CAN_FILTER_FAIL]\r\n";
        case ERROR_CAN_START:
            return "\a[FATAL: CAN_START_FAIL]\r\n";
        case ERROR_CAN_RX_NOTIFICATION:
            return "\a[FATAL: CAN_NOTIF_FAIL]\r\n";

        // Fallback for safety
        default:
            return "\a[ERR: UNKNOWN]\r\n";
    }
}

const char* GetWarningMessage(WarningCode_t code) {
    switch (code) {
        case WARN_CAN_HW_OVERFLOW:
            return "[WARN: CAN_HW_OVERFLOW]";
        case WARN_USB_PMA_OVERRUN:
            return "[WARN: USB_PMA_OVERRUN]";
        case WARN_CAN_READ_FAILED:
            return "[WARN: CAN_READ_FAIL]";
        case WARN_CAN_INVALID_DLC:
            return "[WARN: CAN_INVALID_DLC]";
        case WARN_CAN_RX_BUFFER_FULL:
            return "[WARN: CAN_RX_FULL]";
        case WARN_CAN_TX_FAILED:
            return "[WARN: CAN_TX_FAILED]";
        case WARN_CAN_ARBITRATION_LOST:
            return "[WARN: CAN_ARBITRATION_LOST]";
        case WARN_CAN_ACK:
            return "[WARN: CAN_ACK]";
        case WARN_CAN_CRC:
            return "[WARN: CAN_CRC]";
        case WARN_CAN_STUFF:
            return "[WARN: CAN_STUFF]";
        case WARN_CAN_FORM:
            return "[WARN: CAN_FORM]";
        case WARN_CAN_BIT_RECESSIVE:
            return "[WARN: CAN_BIT_RECESSIVE]";
        case WARN_CAN_BIT_DOMINANT:
            return "[WARN: CAN_BIT_DOMINANT]";
        case WARN_CAN_ERROR_WARNING:
            return "[WARN: CAN_ERROR_WARNING]";
        case WARN_CAN_ERROR_PASSIVE:
            return "[WARN: CAN_ERROR_PASSIVE]";
        case WARN_CAN_BUS_OFF:
            return "[WARN: CAN_BUS_OFF]";
        default:
            return "[WARN: UNKNOWN]";
    }
}

void handle_diagnostic_request(void) {
    char report[256];
    
    int len = snprintf(report, sizeof(report), 
        "--- BRIDGE WARNING STATS ---\r\n"
        "%s - Count: %lu\r\n"
        "%s - Count: %lu\r\n"
        "%s - Count: %lu\r\n",
        GetWarningMessage(WARN_CAN_HW_OVERFLOW), warning_counts[WARN_CAN_HW_OVERFLOW],
        GetWarningMessage(WARN_USB_PMA_OVERRUN), warning_counts[WARN_USB_PMA_OVERRUN],
        GetWarningMessage(WARN_CAN_READ_FAILED), warning_counts[WARN_CAN_READ_FAILED]
    );

    if (len > 0 && len < sizeof(report)) {
        usb_tx_push(&usb_tx_buffer, report, (uint16_t)len);
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

void report_tier2_runtime(ErrorCode_t error_code) {
    const char* error_mssg = GetErrorMessage(error_code);
    usb_tx_push_error(&usb_tx_buffer, error_mssg, strlen(error_mssg));

    CBreset(&usb_rx_buffer);
    CANQreset(&can_tx_queue);
    CANQreset(&can_rx_queue);

    current_bridge_state = BRIDGE_STATE_WAITING_FOR_SYNC;
}

void report_warning(WarningCode_t warning_code) {
    if (warning_code < WARNING_MAX_COUNT) {
        uint32_t primask = __get_PRIMASK();
        __disable_irq();
        
        warning_counts[warning_code]++;
        
        __set_PRIMASK(primask);
    }
}