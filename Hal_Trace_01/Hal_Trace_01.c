#include "Hal_Trace_01.h"
#include "gpio.h"
#include "uart.h"


__no_init static FNCT_UCHAR FNCT_UartRx;
__no_init static FNCT_VOID FNCT_UartTx;

#define TRACE_PORT              UART2
#define TRACE_BAUD              256000
#define TRACE_CLK               BUS_CLK_HZ

#define TRACE_RX_PORT		    GPIOA
#define TRACE_RX_PIN		    GPIO_PTD6
#define TRACE_RX_PIN_MASK	    GPIO_PTD6_MASK

#define TRACE_TX_PORT		    GPIOA
#define TRACE_TX_PIN		    GPIO_PTD7
#define TRACE_TX_PIN_MASK	    GPIO_PTD7_MASK

void Hal_Trace_Initial(void)
{
    GPIO_Init(TRACE_RX_PORT, TRACE_RX_PIN_MASK, GPIO_PinInput);
    GPIO_Init(TRACE_TX_PORT, (uint32_t)TRACE_TX_PIN_MASK, GPIO_PinOutput);

    UART_ConfigType sConfig;
    sConfig.u32SysClkHz = TRACE_CLK;
    sConfig.u32Baudrate = TRACE_BAUD;
    UART_Init(TRACE_PORT, &sConfig);

    NVIC_EnableIRQ(UART2_IRQn);
    Hal_Trace_Enalbe_RxInt();
}

UCHAR Hal_Trace_GetByte(void)
{
    return TRACE_PORT->D;
}

void Hal_Trace_SendByte(UCHAR Data)
{
    TRACE_PORT->D = Data;
}

void Hal_Trace_Enalbe_RxInt(void)
{
    TRACE_PORT->C2 &= ~UART_C2_TCIE_MASK;
    TRACE_PORT->C2 |= UART_C2_RIE_MASK;
}

void Hal_Trace_Occur_TxInt(void)
{
    TRACE_PORT->C2 &= ~UART_C2_RIE_MASK;
    TRACE_PORT->C2 |= UART_C2_TCIE_MASK;
}

void Hal_Trace_ERP(void)
{
    GPIO_Init(TRACE_RX_PORT, TRACE_RX_PIN_MASK, GPIO_PinInput);
    GPIO_Init(TRACE_TX_PORT, (uint32_t)TRACE_TX_PIN_MASK, GPIO_PinInput);
    NVIC_DisableIRQ(UART2_IRQn);
}

void Hal_Trace_Register_RxTxFun(FNCT_UCHAR RxFun, FNCT_VOID TxFun)
{
    FNCT_UartRx = RxFun;
    FNCT_UartTx = TxFun;
}

void Hal_Trace_Int(void)
{
    if(UART_IsRxBuffFull(TRACE_PORT))
    {
        if(FNCT_UartRx != NULL) (*FNCT_UartRx)(UART_ReadDataReg(TRACE_PORT));
    }
    else if(UART_IsTxBuffEmpty(TRACE_PORT))
    {
        if(FNCT_UartTx != NULL) (*FNCT_UartTx)();
    }
}
