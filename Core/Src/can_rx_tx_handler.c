#include "can_rx_tx_handler.h"

#include "can_queue.h"
#include "error_reporter.h"
#include "main.h"
#include <string.h>

#define CAN_TX_MAX_ATTEMPTS 2U

extern CAN_HandleTypeDef hcan1;

void CAN_SafeShutdown(void) {
  HAL_CAN_Stop(&hcan1);
  HAL_CAN_DeInit(&hcan1);
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
  int pending = HAL_CAN_GetRxFifoFillLevel(hcan, CAN_RX_FIFO0);
  bool pushed = true;

  for (int i = 0; i < pending; i++){
    CAN_RxHeaderTypeDef header;
    CANMessage_t message = {0};
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &header, message.payload) != HAL_OK) {
        report_warning(WARN_CAN_READ_FAILED);
    }
    message.is_extended = (header.IDE == CAN_ID_EXT);
    message.id = message.is_extended ? header.ExtId : header.StdId;
    message.dlc = header.DLC;

    if (header.RTR == CAN_RTR_DATA)
      pushed = CANQpush(&can_rx_queue, message);

    if (!pushed) {
        report_warning(WARN_CAN_RX_BUFFER_FULL);
    }
  }
}

// HAL has already decoded the controller's last error code when this runs.
// Controller observations have no mailbox ID and never consume a TX retry.
static void report_can_status_warnings(uint32_t hal_errors, uint32_t esr) {
  static const struct {
    uint32_t hal_error;
    uint32_t state_flag;
    WarningCode_t warning;
  } causes[] = {
    {HAL_CAN_ERROR_RX_FOV0, 0U,           WARN_CAN_HW_OVERFLOW},
    {HAL_CAN_ERROR_ACK,     0U,           WARN_CAN_ACK},
    {HAL_CAN_ERROR_CRC,     0U,           WARN_CAN_CRC},
    {HAL_CAN_ERROR_STF,     0U,           WARN_CAN_STUFF},
    {HAL_CAN_ERROR_FOR,     0U,           WARN_CAN_FORM},
    {HAL_CAN_ERROR_BR,      0U,           WARN_CAN_BIT_RECESSIVE},
    {HAL_CAN_ERROR_BD,      0U,           WARN_CAN_BIT_DOMINANT},
    {HAL_CAN_ERROR_EWG,     CAN_ESR_EWGF, WARN_CAN_ERROR_WARNING},
    {HAL_CAN_ERROR_EPV,     CAN_ESR_EPVF, WARN_CAN_ERROR_PASSIVE},
    {HAL_CAN_ERROR_BOF,     CAN_ESR_BOFF, WARN_CAN_BUS_OFF}
  };
  const uint32_t state = esr & (CAN_ESR_EWGF | CAN_ESR_EPVF | CAN_ESR_BOFF);
  if (hal_errors == HAL_CAN_ERROR_NONE && state == 0U) {
    return;
  }

  uint32_t consumed = 0U;
  for (unsigned i = 0; i < sizeof(causes) / sizeof(causes[0]); ++i) {
    const uint32_t cause = hal_errors & causes[i].hal_error;
    if (cause != 0U || (state & causes[i].state_flag) != 0U) {
      report_warning(causes[i].warning);
    }
    consumed |= cause;
  }
  CLEAR_BIT(hcan1.ErrorCode, consumed);
}

// Call after HAL_CAN_IRQHandler in CAN1 TX, RX0 and SCE, at equal preemption priority.
// Pend CAN1_TX_IRQn after enqueueing to start transmission on an idle bus.
void can_tx_to_peripheral(void) {
  static uint8_t attempts[3];
  static const uint32_t mailbox_errors[3] = {
    HAL_CAN_ERROR_TX_TERR0 | HAL_CAN_ERROR_TX_ALST0,
    HAL_CAN_ERROR_TX_TERR1 | HAL_CAN_ERROR_TX_ALST1,
    HAL_CAN_ERROR_TX_TERR2 | HAL_CAN_ERROR_TX_ALST2
  };
  const uint32_t hal_errors = HAL_CAN_GetError(&hcan1);
  const uint32_t esr = CAN1->ESR;
  report_can_status_warnings(hal_errors, esr);
  const uint32_t tx_errors = hal_errors &
                             (mailbox_errors[0] | mailbox_errors[1] | mailbox_errors[2]);
  CLEAR_BIT(hcan1.ErrorCode, tx_errors);
  const bool tx_stopped = can_tx_error != ERROR_NONE;
  for (unsigned i = 0; i < 3; ++i) {
    const uint32_t cause = tx_errors & mailbox_errors[i];
    if (cause != 0U) {
      const uint32_t arbitration_lost = HAL_CAN_ERROR_TX_ALST0 | HAL_CAN_ERROR_TX_ALST1 |
                                        HAL_CAN_ERROR_TX_ALST2;
      const WarningCode_t warning = (cause & arbitration_lost) ? WARN_CAN_ARBITRATION_LOST : WARN_CAN_TX_FAILED;
      report_warning(warning);
      if (!tx_stopped && attempts[i] >= CAN_TX_MAX_ATTEMPTS) {
        can_tx_error = ERROR_CAN_TX_RETRIES_EXCEEDED;
      }
    }
  }
  if (tx_stopped) {
    return;
  }
  if (esr & CAN_ESR_BOFF) {
    can_tx_error = ERROR_CAN_BUS_OFF;
  }
  if (can_tx_error != ERROR_NONE || current_bridge_state == BRIDGE_STATE_WAITING_FOR_SYNC) {
    const uint32_t all_mailboxes = CAN_TX_MAILBOX0 | CAN_TX_MAILBOX1 | CAN_TX_MAILBOX2;
    if (HAL_CAN_IsTxMessagePending(&hcan1, all_mailboxes)) {
      HAL_CAN_AbortTxRequest(&hcan1, all_mailboxes);
    }
    memset(attempts, 0, sizeof(attempts));
    return;
  }

  const uint32_t status = CAN1->TSR;
  for (unsigned i = 0; i < 3; ++i) {
    // A mailbox can finish after HAL's snapshot. Leave it for the next IRQ.
    if (!(status & (CAN_TSR_TME0 << i)) || (status & (CAN_TSR_RQCP0 << (8U * i)))) {
      continue;
    }

    CAN_TxMailBox_TypeDef *mailbox = &CAN1->sTxMailBox[i];
    bool retry = attempts[i] != 0 && (tx_errors & mailbox_errors[i]) != 0;
    if (!retry) {
      attempts[i] = 0;
      CANMessage_t frame;
      if (!CANQpop(&can_tx_queue, &frame)) {
        continue;
      }
      // Address this specific empty mailbox so an unhandled completion in
      // another mailbox can never be overwritten by the new frame.
      mailbox->TIR = frame.is_extended ? (frame.id << CAN_TI0R_EXID_Pos) | CAN_TI0R_IDE
                                       : frame.id << CAN_TI0R_STID_Pos;
      mailbox->TDTR = frame.dlc;
      uint32_t data[2];
      memcpy(data, frame.payload, sizeof(data));
      mailbox->TDLR = data[0];
      mailbox->TDHR = data[1];
    }
    // A retry uses the identifier, DLC and payload still in this mailbox.
    ++attempts[i];
    SET_BIT(mailbox->TIR, CAN_TI0R_TXRQ);
  }
}
