#ifndef __HAL_TRACE_01_H__
#define __HAL_TRACE_01_H__

#include "jistypes.h"

void Hal_Trace_Initial(void);
void Hal_Trace_Enalbe_RxInt(void);
void Hal_Trace_Occur_TxInt(void);
void Hal_Trace_SendByte(UCHAR Data);
UCHAR Hal_Trace_GetByte(void);
void Hal_Trace_Int(void);
void Hal_Trace_ERP(void);
void Hal_Trace_Register_RxTxFun(FNCT_UCHAR RxFun, FNCT_VOID TxFun);

#endif

