#include "Hal_RPM_MCB_01.h"
#include "gpio.h"
#include "ftm.h"
#include "sim.h"

#ifdef DIGITAL_MCB

#define RPM_PORT                GPIOA
#define PRM_PIN                 GPIO_PTC0

#define RPM_CAP_VALUE			FTM2->CONTROLS[0].CnV
#define RPM_SET_VALUE       FTM2->CNT

#define REMAP_CH_TO_PTC  SIM_RemapFTM2CH0ToPTC0()

#define FTM_CHANNEL FTM_CHANNEL_CHANNEL0
#define FTM_CLOCK  FTM_CLOCK_SYSTEMCLOCK
#define FTM_CLOCK_DIV  FTM_CLOCK_PS_DIV32


#else

#define RPM_PORT                GPIOA
#define PRM_PIN                 GPIO_PTC1

#define RPM_CAP_VALUE			FTM2->CONTROLS[1].CnV
#define RPM_SET_VALUE           FTM2->CNT

#define REMAP_CH_TO_PTC  SIM_RemapFTM2CH1ToPTC1()

#define FTM_CHANNEL FTM_CHANNEL_CHANNEL1
#define FTM_CLOCK  FTM_CLOCK_FIXEDFREQCLOCK
#define FTM_CLOCK_DIV  FTM_CLOCK_PS_DIV2

#endif

__no_init static FNCT_VOID FNCT_RPMint;

static UCHAR Hal_RPM_Get_IO_Filter(GPIO_Type *Port, GPIO_PinType Pin, UINT16 Times)
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

//每一个count时间为  1,000,000(1s) / 40,000,000 * 32 = 0.8us
void Hal_RPM_Initial(void)
{
#ifdef DIGITAL_MCB
GPIO_Init(RPM_PORT, 1 << (PRM_PIN % 32), GPIO_PinInput);
    REMAP_CH_TO_PTC;
    FTM_InputCaptureInit(FTM2, FTM_CHANNEL, FTM_INPUTCAPTURE_FALLINGEDGE);   //enable interrupt , Capture on rising edge
    FTM_ClockSet(FTM2, FTM_CLOCK, FTM_CLOCK_DIV);              // 40M / 32 = 1250000
    FTM_InputCaptureFilterSet(FTM2, FTM_CHANNEL, 15);
    FTM_EnableChannelInt(FTM2, FTM_CHANNEL);
    FTM_DisableOverflowInt(FTM2);
#else
GPIO_Init(RPM_PORT, 1 << (PRM_PIN % 32), GPIO_PinInput);
    REMAP_CH_TO_PTC;
    FTM_InputCaptureInit(FTM2, FTM_CHANNEL, FTM_INPUTCAPTURE_RISINGEDGE);   //enable interrupt , Capture on rising edge
    FTM_ClockSet(FTM2, FTM_CLOCK, FTM_CLOCK_DIV);            // 31.25k / 2 = 15625
    FTM_InputCaptureFilterSet(FTM2, FTM_CHANNEL, 15);
    FTM_EnableChannelInt(FTM2, FTM_CHANNEL);
    FTM_DisableOverflowInt(FTM2);	
#endif
}

UINT32 Hal_RPM_Get_Capture(void)
{
#ifdef DIGITAL_MCB
	UINT32 w_Captured = RPM_CAP_VALUE;
	RPM_CAP_VALUE = 0;
	RPM_SET_VALUE = 0;
	return w_Captured;
#else
    static UINT32 w_CapturedLast = 0;
    UINT32 w_Captured = RPM_CAP_VALUE;
    
    if(Hal_RPM_Get_IO_Filter(RPM_PORT, PRM_PIN, 50))                                 //230 //中断后再读几次,看是否电平稳定
    {
        RPM_CAP_VALUE = 0;
        RPM_SET_VALUE = 0;
    }
    else
    {
        w_Captured = w_CapturedLast;
    }

    w_CapturedLast = w_Captured;
    return w_Captured;  
#endif
}

void Hal_RPM_ERP(void)
{
#ifdef DIGITAL_MCB
    FTM_DisableOverflowInt(FTM2);
#endif
}

void Hal_RPM_Set_Intfuct(FNCT_VOID FnctX)
{
  FNCT_RPMint = FnctX;
}

void Hal_RPM_INT(void)
{
    DisableInterrupts;
    if(FTM_GetOverFlowFlag(FTM2))
    {
        FTM_ClrOverFlowFlag(FTM2);
    }
    
#ifdef DIGITAL_MCB
    if(FTM_GetChannelFlag(FTM2, FTM_CHANNEL_CHANNEL0))                          //OSP
    {
        FTM_ClrChannelFlag(FTM2, FTM_CHANNEL_CHANNEL0);
        if(FNCT_RPMint != NULL) (*FNCT_RPMint)();
    }
#elif defined(ANALOG_APID_MCB) 
    
    if(FTM_GetChannelFlag(FTM2, FTM_CHANNEL_CHANNEL1))                          //OSP
    {
        FTM_ClrChannelFlag(FTM2, FTM_CHANNEL_CHANNEL1);
        if(FNCT_RPMint != NULL) (*FNCT_RPMint)();
    }
#endif
   EnableInterrupts;
}

