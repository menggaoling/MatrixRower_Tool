#ifndef __MIDWARE_TRACE_01_H__
#define __MIDWARE_TRACE_01_H__


#include "jistypes.h"

void Midware_Trace_Initial_HW(void);
void Midware_Trace_Initial_Data(void);
void Trace(CHAR *p_fmt, ...);
void TraceW(CHAR *p_fmt, ...);
void Midware_Trace_Tx_String(UCHAR *pString, UCHAR byLength);
void Midware_Trace_Tx_Int(void);
void Midware_Trace_Rx_Int(UCHAR by_Data);
void Midware_Trace_1ms_Int(void);
void Midware_Trace_Erp(void);

#endif

