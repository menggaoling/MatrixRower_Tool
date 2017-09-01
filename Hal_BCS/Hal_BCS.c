#include "Hal_BCS.h"
#include "gpio.h"
#include "ftm.h"
#include "adc.h"

#define ADC_OUT_PORT            GPIOA
#define ADC_OUT_PIN             GPIO_PTB5

#define ADC_IN_PORT             GPIOA
#define ADC_IN_PIN              GPIO_PTC2

#define BCS_PORT                GPIOA
#define BCS_PIN                 GPIO_PTA0

#define BCS_PWM_MAX	            5555	                //225


void Hal_BCS_PWM_Initial(void)
{
GPIO_Init(ADC_OUT_PORT,  1 << (ADC_OUT_PIN % 32), GPIO_PinOutput); 
	GPIO_PinSet(ADC_OUT_PIN);
    GPIO_Init(ADC_IN_PORT,  1 << (ADC_IN_PIN % 32), GPIO_PinInput); 
GPIO_Init(BCS_PORT,  1 << (BCS_PIN % 32), GPIO_PinOutput); 
    
   
    SIM->SCGC |= SIM_SCGC_FTM0_MASK;
    FTM0->CONTROLS[0].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
    FTM_SetModValue(FTM0, BCS_PWM_MAX);
    FTM0->CONTROLS[0].CnV = 0;
    FTM_ClockSet(FTM0, FTM_CLOCK_SYSTEMCLOCK, FTM_CLOCK_PS_DIV32);

    Hal_BCS_Set_PWM_Percent(0);
}

void Hal_BCS_Set_PWM_Percent(UINT16 w_Dat)
{
//    UINT16 wData = 0;
//    wData = BCS_PWM_MAX / 1000;
//    wData *= w_Dat;
    
    UINT16 wData = (BCS_PWM_MAX * w_Dat) / 1000;
    if(wData >= BCS_PWM_MAX)
    {
        wData = BCS_PWM_MAX - 1;
    }
    
    FTM_SetChannelValue(FTM0, FTM_CHANNEL_CHANNEL0, wData);
}

UINT16 Hal_BCS_ReadAdc(void)
{
  ADC_SingleConversion(ADC);
    return ADC_PollRead(ADC, ADC_CHANNEL_AD10);
}

void Hal_BCS_ERP(void)
{
	GPIO_Init(ADC_OUT_PORT,  1 << (ADC_OUT_PIN % 32), GPIO_PinInput); 
	GPIO_Init(ADC_IN_PORT,  1 << (ADC_IN_PIN % 32), GPIO_PinInput); 
	GPIO_Init(BCS_PORT,  1 << (BCS_PIN % 32), GPIO_PinOutput); 
	FTM_DeInit(FTM0);
}






