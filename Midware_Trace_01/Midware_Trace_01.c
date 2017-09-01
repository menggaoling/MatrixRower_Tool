#include "Midware_Trace_01.h"

#ifdef __TRACE_FUNCATION__
#define TRACE_BUFFER_MAX	    200
#define TRACE_TX_TIMEOUT        50     //unit ms


__no_init static UCHAR  by_TxPointer;
__no_init static UCHAR  by_TxLength;
__no_init static UCHAR  aby_TxBuffer[TRACE_BUFFER_MAX];
__no_init static UINT16 w_CountMsTx;
__no_init static struct
{
  unsigned CmdTxBusy        : 1;
} Uart;

void Midware_Trace_Initial_HW(void)
{
  Hal_Trace_Initial();
  Hal_Trace_Register_RxTxFun(Midware_Trace_Rx_Int, Midware_Trace_Tx_Int);
}

void Midware_Trace_Initial_Data(void)
{
  
  memset(aby_TxBuffer, 0, sizeof(aby_TxBuffer));
  memset(&Uart       , 0, sizeof(Uart));
  
  by_TxPointer = 0;
  by_TxLength = 0;
  w_CountMsTx = 0;
}

//非阻塞发送
void Trace(CHAR *p_fmt, ...)
{
  if(Uart.CmdTxBusy) return;
  Uart.CmdTxBusy = 1;
  
  CHAR    str[TRACE_BUFFER_MAX + 1];
  UCHAR   len = 0;
  memset(str, 0, sizeof(str));
  va_list vArgs;
  
  va_start(vArgs, p_fmt);
  vsprintf((char *)str, (char const *)p_fmt, vArgs);
  va_end(vArgs);
  
  len = strlen((char const *)str);
  Midware_Trace_Tx_String((UCHAR *)str, len);
}

//阻塞发送
void TraceW(CHAR *p_fmt, ...)
{
  while(Uart.CmdTxBusy);                                                      //等待上个数据发送完毕再开始
  Uart.CmdTxBusy = 1;
  
  CHAR    str[TRACE_BUFFER_MAX + 1];
  UCHAR   len = 0;
  memset(str, 0, sizeof(str));
  va_list vArgs;
  
  va_start(vArgs, p_fmt);
  vsprintf((char *)str, (char const *)p_fmt, vArgs);
  va_end(vArgs);
  
  len = strlen((char const *)str);
  Midware_Trace_Tx_String((UCHAR *)str, len);
  while(Uart.CmdTxBusy);                                                      //等待发送完毕再出函数
}

void Midware_Trace_Tx_String(UCHAR *pString, UCHAR byLength)
{
  memcpy(aby_TxBuffer, pString, byLength);
  by_TxPointer = 0;
  by_TxLength = byLength;
  Hal_Trace_Occur_TxInt();
}

void Midware_Trace_Rx_Int(UCHAR by_Data)
{
  Hal_Trace_GetByte();                                                        //丢失不用
}

void Midware_Trace_Tx_Int(void)
{
  if(by_TxPointer >= by_TxLength)
  {
    Hal_Trace_Enalbe_RxInt();
    Uart.CmdTxBusy = 0;
    by_TxPointer = 0;
    by_TxLength = 0;
    w_CountMsTx = 0;
  }
  else
  {
    Hal_Trace_SendByte(aby_TxBuffer[by_TxPointer++]);
  }
}

void Midware_Trace_1ms_Int(void)
{
  static UCHAR i = 0;
  if(++i >= 10)
  {
    i = 0;
    if(Uart.CmdTxBusy)
    {
      if(++w_CountMsTx > TRACE_TX_TIMEOUT)
      {
        Hal_Trace_Enalbe_RxInt();
        Uart.CmdTxBusy = 0;
        by_TxPointer = 0;
        by_TxLength = 0;
        w_CountMsTx = 0;
      }
    }
    else
    {
      w_CountMsTx = 0;
    }
  }
}

void Midware_Trace_Erp(void)
{
  Hal_Trace_ERP();
}

#else
void Midware_Trace_Initial_HW(void){}
void Midware_Trace_Initial_Data(void){}
void Trace(CHAR *p_fmt, ...){}
void TraceW(CHAR *p_fmt, ...){}
void Midware_Trace_Tx_String(UCHAR *pString, UCHAR byLength){}
void Midware_Trace_Tx_Int(void){}
void Midware_Trace_Rx_Int(UCHAR by_Data){}
void Midware_Trace_1ms_Int(void){}
void Midware_Trace_Erp(void){}
#endif

