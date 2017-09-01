#include "Hal_Induction_01.h"

#define CURRENT_CHANNEL            ADC_CHANNEL_AD4
#define LOAD_PORT                  GPIOA
#define LOAD_PIN_MASK              GPIO_PTD2_MASK
#define LOAD_PIN                   GPIO_PTD2

void Hal_Induction_Init(void)  
{   
  //******************PWM init*************//
  SIM_RemapFTM2CH5ToPTB5(); //Select FTM2Ch5 on PTB5
  
  // FTM2 is set as edge aligned pwm mode, high true pulse 
  //FTM_PWMInit(FTM2, FTM_PWMMODE_EDGEALLIGNED, FTM_PWM_HIGHTRUEPULSE);
  SIM->SCGC  |= SIM_SCGC_FTM2_MASK;
  FTM2->CONTROLS[5].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;   //HIGHTRUEPULSE
  //FTM2->CONTROLS[5].CnSC = FTM_CnSC_MSB_MASK | FTM_CnSC_ELSA_MASK;   //LOWTRUEPULSE    
  // FTMEN enable 
  FTM_SetFTMEnhanced(FTM2);
  // update MOD value 
  FTM_SetModValue(FTM2, PWM_MAX);
  FTM2->CONTROLS[5].CnV = 0;  //FTM2->CONTROLS[5].CnV  = FTM_C0V_INIT; 
  // set clock source, start counter 
  FTM_ClockSet(FTM2, FTM_CLOCK_SYSTEMCLOCK, FTM_CLOCK_PS_DIV1);
  FTM_SetChannelValue(FTM2, FTM_CHANNEL_CHANNEL5, PWM_MAX);
  
  //******************ADC init*************//
  ADC_ConfigType  sADC_Config = {0};
  sADC_Config.u8ClockDiv = ADC_ADIV_DIVIDE_4;
  sADC_Config.u8ClockSource = CLOCK_SOURCE_BUS_CLOCK;
  sADC_Config.u8Mode = ADC_MODE_12BIT;
  //sADC_Config.sSetting.bIntEn = 1;
  ADC_Init(ADC, &sADC_Config);
  
  GPIO_Init(LOAD_PORT, LOAD_PIN_MASK, GPIO_PinOutput);
  //GPIO_PinSet(LOAD_PIN); 
}

void Hal_Induction_Set_PWM(UINT16 wDat)      //w_Dat	  0 ~ 2500 = 0% ~ 100%
{
  if(wDat <= PWM_MAX)
  {
    FTM_SetChannelValue(FTM2, FTM_CHANNEL_CHANNEL5, wDat);
  }
}

UINT16 Hal_Induction_Collect_Current(void)
{
  return ADC_PollRead(ADC,CURRENT_CHANNEL);
}

void Hal_Induction_ERP(void)
{
  ADC_DeInit(ADC);
  FTM_DeInit(FTM2); 
  GPIO_Init(GPIOA, GPIO_PTB3_MASK, GPIO_PinInput); 
  GPIO_Init(LOAD_PORT, LOAD_PIN_MASK, GPIO_PinOutput);
  GPIO_PinClear(LOAD_PIN); 
}

