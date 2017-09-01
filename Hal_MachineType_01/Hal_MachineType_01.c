#include "Hal_Induction_01.h"

#define TYPE_CHANNEL              ADC_CHANNEL_AD10
#define TYPE_POWER_PORT           GPIOA
#define TYPE_POWER_PIN_MASK       GPIO_PTC3_MASK
#define TYPE_POWER_PIN            GPIO_PTC3

void Hal_MachineType_Init(void)  
{   
    //******************ADC init*************//
	ADC_ConfigType  sADC_Config = {0};
	sADC_Config.u8ClockDiv = ADC_ADIV_DIVIDE_4;
	sADC_Config.u8ClockSource = CLOCK_SOURCE_BUS_CLOCK;
	sADC_Config.u8Mode = ADC_MODE_12BIT;
	//sADC_Config.sSetting.bIntEn = 1;
	ADC_Init(ADC, &sADC_Config);
    
    GPIO_Init(TYPE_POWER_PORT, TYPE_POWER_PIN_MASK, GPIO_PinOutput);
    GPIO_PinSet(TYPE_POWER_PIN); 
}

UINT16 Hal_MachineType_Collect_ADC(void)
{
	return ADC_PollRead(ADC, TYPE_CHANNEL);
}

void Hal_MachineType_End_Collect(void)
{
    GPIO_PinClear(TYPE_POWER_PIN); 
}

