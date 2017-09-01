#include "Hal_Safekey_01.h"
#include "gpio.h"



#define SAFETY_KEY_PORT                 GPIOA
#define SAFETY_KEY_PIN                    GPIO_PTC3
#define SAFETY_KEY_PIN_MASK          GPIO_PTC3_MASK

#ifdef DIGITAL_MCB

#define SAFETY_SHUT_PORT                GPIOB
#define SAFETY_SHUT_PIN                 GPIO_PTE7
#define SAFETY_SHUT_PIN_MASK       GPIO_PTE7_MASK

#else

#define SAFETY_SHUT_PORT                GPIOA
#define SAFETY_SHUT_PIN                 GPIO_PTC0
#define SAFETY_SHUT_PIN_MASK       GPIO_PTC0_MASK

#endif

static UCHAR Hal_Safekey_Get_IO_Filter(GPIO_Type *Port, GPIO_PinType Pin, UINT16 Times)
{
    UCHAR  by_Loop = 0;
    UCHAR  by_CounterTimes = 5;                                                 //总的循环次数不超过5次
    UINT32 w_Out = GPIO_Read(Port) & (1 << Pin % 32);
    
    do
    {
        by_Loop = 0;
        if(by_CounterTimes) by_CounterTimes--;
        for(UCHAR i = 0; i < Times; i++)
        {
            if(w_Out != GPIO_Read(Port) & (1 << Pin % 32))
            {
                by_Loop = 1;
                w_Out = GPIO_Read(Port) & (1 << Pin % 32);
                break;
            }
        }
    }while(by_Loop && by_CounterTimes);
    
    return (w_Out ? 1 : 0);
}


void Hal_Safekey_Initial(void)
{
   GPIO_Init(SAFETY_KEY_PORT, SAFETY_KEY_PIN_MASK, GPIO_PinInput);
   GPIO_Init(SAFETY_SHUT_PORT, SAFETY_SHUT_PIN_MASK,GPIO_PinInput);
}

//safekey Remove: false, safkey plug: true
UCHAR Hal_Safekey_Get_Safekey(void)
{
    return Hal_Safekey_Get_IO_Filter(SAFETY_KEY_PORT, SAFETY_KEY_PIN, 5);
}

//safeShut Remove: true, safeShut plug: false
UCHAR Hal_Safekey_Get_Shut(void)
{
    return Hal_Safekey_Get_IO_Filter(SAFETY_SHUT_PORT, SAFETY_SHUT_PIN, 5);
}

void Hal_Safekey_ERP(void)
{
   GPIO_Init(SAFETY_KEY_PORT, SAFETY_KEY_PIN_MASK, GPIO_PinInput);
   GPIO_Init(SAFETY_SHUT_PORT, SAFETY_SHUT_PIN_MASK,GPIO_PinInput);
}










