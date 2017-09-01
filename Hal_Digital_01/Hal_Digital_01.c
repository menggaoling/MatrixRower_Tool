#include "Hal_Digital_01.h"

#define UART_485DE_PORT		GPIOA
#define UART_485DE_PIN		GPIO_PTC5

#define UART_RX_PORT		GPIOA
#define UART_RX_PIN		GPIO_PTC6

#define UART_TX_PORT		GPIOA
#define UART_TX_PIN		GPIO_PTC7

#define UART_PORT               UART1
#define UART_NORMAL_BAUD        9600
#define UART_UPDATE_BAUD        115200
#define UART_CLK                BUS_CLK_HZ
#define UART_IRQN               UART1_IRQn

__no_init static UCHAR by_Erp;
__no_init static FNCT_UCHAR FNCT_UartRx;
__no_init static FNCT_VOID FNCT_UartTx;
__no_init static UCHAR RxWakeupFlag;

static UCHAR Hal_Digital_Get_IO_Filter(GPIO_Type *Port, GPIO_PinType Pin, UINT16 Times)
{
  UCHAR  by_Loop = 0;
  UCHAR  by_CounterTimes = 5;                                                 //总的循环次数不超过5次
  UINT32 w_Out = GPIO_Read(Port) | (1 << Pin % 32);
  
  do
  {
    by_Loop = 0;
    if(by_CounterTimes) by_CounterTimes--;
    for(UCHAR i = 0; i < Times; i++)
    {
      if(w_Out != GPIO_Read(Port) | (1 << Pin % 32))
      {
        by_Loop = 1;
        w_Out = GPIO_Read(Port) | (1 << Pin % 32);
        break;
      }
    }
  }while(by_Loop && by_CounterTimes);
  
  return (w_Out ? 1 : 0);
}

static void Hal_Digital_WakeUp_Init(void)
{
  UART_PORT->C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK);
  NVIC_DisableIRQ(UART_IRQN); 
  GPIO_Init(UART_RX_PORT, 1 << UART_RX_PIN, GPIO_PinInput);  
  GPIO_PinClear(UART_485DE_PIN);
  RxWakeupFlag = 1;
}

void Hal_Digital_Initial(UCHAR mode)
{
  UART_ConfigType sConfig;
  
  GPIO_Init(UART_485DE_PORT, 1 << UART_485DE_PIN, GPIO_PinOutput);
  
  sConfig.u32SysClkHz = BUS_CLK_HZ;
  
  if(mode > 0)
    sConfig.u32Baudrate = UART_UPDATE_BAUD;
  else
    sConfig.u32Baudrate = UART_NORMAL_BAUD;
  UART_Init(UART_PORT, &sConfig);
  UART_PORT->BDH &= ~(1 << 6);
  
  NVIC_EnableIRQ(UART_IRQN);
  Hal_Digital_Enable_RxInt();
  
  by_Erp = 0;
  FNCT_UartRx = NULL;
  FNCT_UartTx = NULL;
}

void Hal_Digital_Send_Byte(UCHAR Data)
{
  UART_PORT->D = Data;
}

void Hal_Digital_Set_Direction(UCHAR dir)
{
  if(dir == 0)
    GPIO_PinClear(UART_485DE_PIN);
  else
    GPIO_PinSet(UART_485DE_PIN);
}


void Hal_Digital_Enable_RxInt(void)
{
  GPIO_PinClear(UART_485DE_PIN);
  UART_PORT->C2 &= ~UART_C2_TCIE_MASK;
  UART_PORT->C2 |= UART_C2_RIE_MASK;
}

void Hal_Digital_Occur_TxInt(void)
{
  GPIO_PinSet(UART_485DE_PIN);
  UART_PORT->C2 &= ~UART_C2_RIE_MASK;
  UART_PORT->C2 |= UART_C2_TCIE_MASK;
}

void Hal_Digital_Int(void)
{
  if(UART_PORT->S2 & UART_S2_RXEDGIF_MASK)
  {
    UART_PORT->S2 |= UART_S2_RXEDGIF_MASK;
    if(by_Erp) 
    {
      Hal_Digital_WakeUp_Init();
      return;
    }
  }
  
  if(UART_IsRxBuffFull(UART_PORT))
  {
    if(FNCT_UartRx != NULL) (*FNCT_UartRx)(UART_ReadDataReg(UART_PORT));
  }
  else if(UART_IsTxBuffEmpty(UART_PORT))
  {
    if(FNCT_UartTx != NULL) (*FNCT_UartTx)();
  }
}

void Hal_Digital_Set_RxTxFuct(FNCT_UCHAR FnctRx, FNCT_VOID FnctTx)
{
  FNCT_UartRx = FnctRx;
  FNCT_UartTx = FnctTx;
}

void Hal_Digital_Uart_Disable(void)
{
  UART_PORT->C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK);
  NVIC_DisableIRQ(UART_IRQN); 
  GPIO_Init(UART_TX_PORT, 1 << UART_TX_PIN, GPIO_PinOutput); 
}

void Hal_Digital_ReKeep_UartFunction(void)
{
  UART_PORT->C2 |= (UART_C2_TE_MASK | UART_C2_RE_MASK);
  NVIC_EnableIRQ(UART1_IRQn); 
  SIM_RemapUART1ToPTC_6_7();
}

void Hal_Digital_Set_TxLevel(UCHAR level)
{
  if(level == 0)
    GPIO_PinClear(UART_TX_PIN);
  else 
    GPIO_PinSet(UART_TX_PIN);
}

UCHAR Hal_Digital_Get_Rx(void)
{
  return Hal_Digital_Get_IO_Filter(UART_RX_PORT, UART_RX_PIN, 5);
}

void Hal_Digital_Erp(void)
{
  Hal_Digital_Enable_RxInt();
  GPIO_Init(UART_TX_PORT, 1 << UART_TX_PIN, GPIO_PinInput);
  UART_PORT->BDH |= 1 << 6 ;
  by_Erp = 1;
  RxWakeupFlag = 0;
}

UCHAR Hal_Digital_Get_WakeupFlag(void)
{
  UCHAR flag = RxWakeupFlag;
  RxWakeupFlag = 0;
  return flag;
}


