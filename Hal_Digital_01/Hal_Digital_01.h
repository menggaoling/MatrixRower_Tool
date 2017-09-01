#ifndef __HAL_DIGITAL_01_H__
#define __HAL_DIGITAL_01_H__

#include "JIStypes.h"

void Hal_Digital_Initial(UCHAR mode);
UCHAR Hal_Digital_Get_Rx(void);
void Hal_Digital_Occur_TxInt(void);
void Hal_Digital_Enable_RxInt(void);
void Hal_Digital_Set_RxTxFuct(FNCT_UCHAR FnctRx, FNCT_VOID FnctTx);
void Hal_Digital_Send_Byte(UCHAR Data);
void Hal_Digital_Int(void);
void Hal_Digital_Erp(void);
void Hal_Digital_Set_Direction(UCHAR dir);
void Hal_Digital_Uart_Disable(void);
void Hal_Digital_Set_TxLevel(UCHAR level);
UCHAR Hal_Digital_Get_WakeupFlag(void);
void Hal_Digital_ReKeep_UartFunction(void);
#endif

