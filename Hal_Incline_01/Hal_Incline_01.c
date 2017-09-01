#include "Hal_Incline_01.h"

#define INC_FILTER_NUM              5   
#define INCLINE_ZERO_PORT           GPIOA
#define INCLINE_ZERO_PIN_MASK       GPIO_PTA7_MASK
#define INCLINE_UP_PORT             GPIOB
#define INCLINE_UP_PIN_MASK         GPIO_PTE1_MASK
#define INCLINE_UP_PIN              GPIO_PTE1
#define INCLINE_DOWN_PORT           GPIOB
#define INCLINE_DOWN_PIN_MASK       GPIO_PTE0_MASK
#define INCLINE_DOWN_PIN            GPIO_PTE0
#define INCLINE_COUNT_PORT          GPIOA
#define INCLINE_COUNT_PIN_MASK      GPIO_PTA6_MASK
#define INCLINE_COUNT_ACMP_PORT     ACMP1
#define INCLINE_COUNT_ACMP_CHANNEL  0

#ifdef __LCB_SYS__
#define INCLINE_CONTROL_PORT        GPIOA
#define INCLINE_CONTROL_PIN_MASK    GPIO_PTD3_MASK
#define INCLINE_CONTROL_PIN         GPIO_PTD3
#endif

__no_init static FNCT_VOID FNCT_IncInt;

void Hal_Incline_Initial(void)  
{     
  GPIO_Init(INCLINE_ZERO_PORT, INCLINE_ZERO_PIN_MASK, GPIO_PinInput);        //INCLINE_ZERO 
  GPIO_Init(INCLINE_UP_PORT, INCLINE_UP_PIN_MASK, GPIO_PinOutput);           //Incline_UP
  GPIO_Init(INCLINE_DOWN_PORT, INCLINE_DOWN_PIN_MASK, GPIO_PinOutput);       //Incline_DOWN
#ifdef __LCB_SYS__
  GPIO_Init(INCLINE_CONTROL_PORT, INCLINE_CONTROL_PIN_MASK, GPIO_PinOutput); //Incline_CONTROL  
#endif
  
  /***************************Initial Count, ACMP INT*************************/
  ACMP_ConfigType ACMP_Config;
  /* init ACMP config parameter */
  ACMP_Config.sCtrlStatus.bits.bIntEn = TRUE; /* enable interrupt */
  ACMP_Config.sCtrlStatus.bits.bOutEn = 0;    /* diable output */
  ACMP_Config.sPinSelect.bits.bNegPin = 0x3;  /* negative pin:DAC */
  ACMP_Config.sPinSelect.bits.bPosPin = INCLINE_COUNT_ACMP_CHANNEL;    /* positive pin:ex0 */
  ACMP_Config.sDacSet.bits.bEn        = TRUE; /* enable DAC */    
  ACMP_Config.sDacSet.bits.bRef       = DAC_REF_VDDA;    /* reference:Vdda */
  ACMP_Config.sDacSet.bits.bVal       = 0x3F; /* DAC out */
  ACMP_Config.sPinEnable.bits.bEn     = TRUE; /* enable ex0 */
  
  ACMP_Init(INCLINE_COUNT_ACMP_PORT, &ACMP_Config);             /* init ACMP moudle */  
  ACMP_Enable(INCLINE_COUNT_ACMP_PORT);                         /* enable ACMP */
  /**************************************************************************/
  #ifdef __LCB_SYS__
  GPIO_PinClear(INCLINE_CONTROL_PIN); 
#endif
}

void Hal_Incline_Set_Control(UCHAR level)
{
#ifdef __LCB_SYS__
  if(level == 0)
    GPIO_PinClear(INCLINE_CONTROL_PIN);
  else
    GPIO_PinSet(INCLINE_CONTROL_PIN);
#endif
}

void Hal_Incline_Set_Stop(void)
{
  GPIO_PinClear(INCLINE_UP_PIN);
  GPIO_PinClear(INCLINE_DOWN_PIN); 
}

void Hal_Incline_Set_UpDir(void)
{ 
  GPIO_PinClear(INCLINE_DOWN_PIN);
  GPIO_PinSet(INCLINE_UP_PIN); 
}

void Hal_Incline_Set_DownDir(void)
{
  GPIO_PinClear(INCLINE_UP_PIN);
  GPIO_PinSet(INCLINE_DOWN_PIN); 
}

UCHAR Hal_Incline_Check_Zero(void)  //1 = No Zero position, 0 = Zero Position
{
  UCHAR loop = 0;
  UINT32 ret = 0;
  ret = GPIO_Read(INCLINE_ZERO_PORT) & INCLINE_ZERO_PIN_MASK;  
  do
  {
    loop = 0;
    for(UCHAR i = 0; i < INC_FILTER_NUM; i++)
    {
      if(ret != (GPIO_Read(INCLINE_ZERO_PORT) & INCLINE_ZERO_PIN_MASK))
      {
        loop = 1;	
        ret = GPIO_Read(INCLINE_ZERO_PORT) & INCLINE_ZERO_PIN_MASK;
        break;
      }
    }
  }while(loop);
  
  return (ret ? 1 : 0);     
}

void Hal_Incline_ERP(void)
{
  GPIO_Init(INCLINE_UP_PORT, INCLINE_UP_PIN_MASK, GPIO_PinInput); 
  GPIO_Init(INCLINE_DOWN_PORT, INCLINE_DOWN_PIN_MASK, GPIO_PinInput);
}

void Hal_Incline_Set_Intfuct(FNCT_VOID FnctX)
{
  FNCT_IncInt = FnctX;
}

void Hal_Incline_Int(void)
{
  if(ACMP_GetFlag(INCLINE_COUNT_ACMP_PORT))
  {
    if(FNCT_IncInt != NULL) (*FNCT_IncInt)();
    ACMP_ClrFlag(INCLINE_COUNT_ACMP_PORT);
  }
}