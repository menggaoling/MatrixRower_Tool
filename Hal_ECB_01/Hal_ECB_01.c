#include "Hal_ECB_01.h"
#include "acmp.h"

#define ECB_FILTER_NUM          5   

#define ECB_ZERO_PORT           GPIOA
#define ECB_ZERO_PIN_MASK       GPIO_PTA1_MASK

#define ECB_UP_PORT             GPIOA
#define ECB_UP_PIN_MASK         GPIO_PTD0_MASK
#define ECB_UP_PIN              GPIO_PTD0

#define ECB_DOWN_PORT           GPIOA
#define ECB_DOWN_PIN_MASK       GPIO_PTD1_MASK
#define ECB_DOWN_PIN            GPIO_PTD1

#define ECB_COUNT_PORT          GPIOA
#define ECB_COUNT_PIN_MASK      GPIO_PTA0_MASK

#define ECB_COUNT               0
#define ECB_KBI                 KBI0

__no_init static FNCT_VOID FNCT_ECBint;

void Hal_ECB_Initial(void)
{  
  GPIO_Init(ECB_ZERO_PORT, ECB_ZERO_PIN_MASK, GPIO_PinInput); //ECB_ZERO
  GPIO_Init(ECB_UP_PORT, ECB_UP_PIN_MASK, GPIO_PinOutput);    //ECB_UP
  GPIO_Init(ECB_DOWN_PORT, ECB_DOWN_PIN_MASK, GPIO_PinOutput);//ECB_DOWN   
  
  /******************Initial Count ,KBI INT**************/   
  GPIO_Init(ECB_COUNT_PORT, ECB_COUNT_PIN_MASK, GPIO_PinInput); 
  SIM->SCGC |= SIM_SCGC_KBI0_MASK;             /* enable clock to KBI0 */    
  /* mask keyboard interrupts first */
  ECB_KBI->SC = KBI_MODE_EDGE_ONLY;
  /* configure KBI pin polarity and others */
  ECB_KBI->PE |= (1<<ECB_COUNT);                      /* enable this KBI pin */
  ECB_KBI->ES = (ECB_KBI->ES & ~(1<<ECB_COUNT)) | (KBI_FALLING_EDGE_LOW_LEVEL << ECB_COUNT);     
  FGPIOA->PIDR &= ~(1<<ECB_COUNT);              /* enable GPIO input */    
  FGPIOA->PDDR &= ~(1<<ECB_COUNT);              /* configure pin as input */ 
  PORT->PUE0 |= (1<<ECB_COUNT);                 /*enable pullup for the pin */  
  /*Reset KBI_SP register*/
  ECB_KBI->SC |= 1<<KBI_SC_RSTKBSP_SHIFT;	
  /*Real KBI_SP register enable*/
  ECB_KBI->SC |= 1<<KBI_SC_KBSPEN_SHIFT;
  /* write to KBACK to clear any false interrupts */
  ECB_KBI->SC = 1<<KBI_SC_KBACK_SHIFT;   
  /* enable interrupt if needed */
  ECB_KBI->SC |= KBI_SC_KBIE_MASK;
  NVIC_EnableIRQ(KBI0_IRQn); 
  /**********************************************************/     
  
  Hal_ECB_Set_Stop(); 
}

void Hal_ECB_Set_UpDir(void)
{
  GPIO_PinClear(ECB_UP_PIN);
  GPIO_PinSet(ECB_DOWN_PIN);  
}

void Hal_ECB_Set_DownDir(void)
{
  GPIO_PinClear(ECB_DOWN_PIN);
  GPIO_PinSet(ECB_UP_PIN);  
}

void Hal_ECB_Set_Stop(void)
{
  GPIO_PinClear(ECB_UP_PIN);
  GPIO_PinClear(ECB_DOWN_PIN);  
}

UCHAR Hal_ECB_Check_Zero(void)  //0 = No Zero position, 1 = Zero Position
{
  UCHAR loop = 0;
  UINT32 ret = 0;
  ret = GPIO_Read(ECB_ZERO_PORT) & ECB_ZERO_PIN_MASK;  
  do
  {
    loop = 0;
    for(UCHAR i = 0; i < ECB_FILTER_NUM; i++)
    {
      if(ret != (GPIO_Read(ECB_ZERO_PORT) & ECB_ZERO_PIN_MASK))
      {
        loop = 1;	
        ret = GPIO_Read(ECB_ZERO_PORT) & ECB_ZERO_PIN_MASK;
        break;
      }
    }
  }while(loop);
  return (ret > 0 ? 1 : 0);     
}

void Hal_ECB_ERP(void)
{
  GPIO_Init(ECB_UP_PORT, ECB_UP_PIN_MASK, GPIO_PinInput);    
  GPIO_Init(ECB_DOWN_PORT, ECB_DOWN_PIN_MASK, GPIO_PinInput);
}

void Hal_ECB_Set_KBIfuct(FNCT_VOID FnctX)
{
  FNCT_ECBint = FnctX;
}

void Hal_ECB_INT(void)
{
  KBI_ClrFlags(KBI0);
  if(KBI_GetSP(KBI0) == (1 << ECB_COUNT))
  {
    if(FNCT_ECBint != NULL) (*FNCT_ECBint)();
  }
  KBI_RstSP(KBI0);
}