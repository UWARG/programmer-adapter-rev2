#include "error_reporter.h"
#include "stm32l4xx.h"

// File-local storage: every caller uses these same two variables.
// Static storage is automatically zero-initialized at boot.
static volatile int error;
static volatile int warnings[16];

void error_reporter_report_error(int error_code)
{
    // Prevent main and other maskable interrupts from changing the stored data
    // or interrupting this fatal-error workflow. This path never restores them.
    __disable_irq();
    error = error_code;

    // TODO: Format error and all nonzero warnings into a USB message (TBD).
    // USB CDC send (include usbd_cdc_if.h when implemented):
    // CDC_Transmit_FS(message_buffer, message_length);
    // TODO: Interrupt-driven USB cannot complete with PRIMASK set. Before
    // implementing the send, arrange interrupt priorities/masking to let USB
    // run while blocking competing work, then wait for completion with a
    // timeout. Entry priority must also be configured in platform setup.
    // TODO: Stop DMA/peripherals and handle watchdogs as needed to stay halted.

    while (1) {
        __NOP();
    }
}

void error_reporter_report_warning(int warning_code)
{
    if (warning_code == 0) {
        return; // Zero marks an unused entry.
    }

    // Protect the entire find-and-store operation against nested interrupts.
    // Restore the caller's mask so an already-disabled context stays disabled.
    const uint32_t interrupt_mask = __get_PRIMASK();
    __disable_irq();

    for (unsigned int i = 0; i < sizeof warnings / sizeof warnings[0]; ++i) {
        if (warnings[i] == 0) {
            warnings[i] = warning_code;
            break;
        }
    }

    __set_PRIMASK(interrupt_mask);
}
