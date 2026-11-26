#ifndef INC_CAN_RX_TX_HANDLER_H_
#define INC_CAN_RX_TX_HANDLER_H_

#include "stm32l4xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void CAN_SafeShutdown(void);
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan);
void can_tx_to_peripheral(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_CAN_RX_TX_HANDLER_H_ */
