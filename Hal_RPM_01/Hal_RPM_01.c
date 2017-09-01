#include "Hal_RPM_01.h"

#define RPM_KBI_CHANNEL         26
#define RPM_PORT                GPIOB
#define RPM_PIN_MASK            GPIO_PTH2_MASK
#define RPM_PIN                 GPIO_PTH2

#define RPM_FTM_TYPE        FTM1
#define RPM_FTM_CHANNEL     FTM_CHANNEL_CHANNEL0
#define RPM_FTM_CLOCK       FTM_CLOCK_SYSTEMCLOCK//FTM_CLOCK_SYSTEMCLOCK//40M Hz
#define RPM_FTM_CLOCK_DIV   FTM_CLOCK_PS_DIV64//FTM_CLOCK_PS_DIV128
#define RPM_FTM_CAPTURE_MODE FTM_INPUTCAPTURE_FALLINGEDGE

#define RPM_REMAP_CH_TO_PORT() SIM_RemapFTM1CH0ToPTH2()

__no_init static FNCT_VOID FNCT_RPMint;
static UINT32 Capture_Buf[RPM_BUF_SIZE];
static UINT32 Last_Capture_Value = 0;
static UCHAR Capture_Over_Flag = 0;
static UCHAR Capture_Flag = 0;
static UCHAR CntNum = 0;
static UINT16 RotatePulseNum = 0;
static UINT16 LastPulseNum = 0;
static UINT16 CurrentPulseNum = 0;
static UINT32 Last_Value = 0;

void Hal_RPM_Initial(void) 
{
//  GPIO_Init(RPM_PORT, RPM_PIN_MASK, GPIO_PinInput); 
  //FTM init

  RPM_REMAP_CH_TO_PORT();  //select PTH2 to FTM1 CH0
  SIM->SCGC |= SIM_SCGC_FTM1_MASK;
  FTM_ClockSet(RPM_FTM_TYPE,RPM_FTM_CLOCK,RPM_FTM_CLOCK_DIV);//Set FTM1 clock to system clock divide by 128
  FTM_EnableOverflowInt(RPM_FTM_TYPE);//Enable timer overflow interrupt
  FTM_InputCaptureInit(RPM_FTM_TYPE,RPM_FTM_CHANNEL,RPM_FTM_CAPTURE_MODE);
    
//  FTM_SetModValue(RPM_FTM_TYPE,0xFFFE);
  
  
  
}

void Hal_RPM_ERP(void)
{
  SIM->SCGC |= SIM_SCGC_KBI1_MASK;             /* enable clock to KBI1 */    
  /* mask keyboard interrupts first */
  KBI1->SC = KBI_MODE_EDGE_ONLY;
  /* configure KBI pin polarity and others */
  KBI1->PE |= (1<<RPM_KBI_CHANNEL);                      /* enable this KBI pin */
  KBI1->ES = (KBI1->ES & ~(1<<RPM_KBI_CHANNEL)) | (KBI_FALLING_EDGE_LOW_LEVEL << RPM_KBI_CHANNEL);     
  FGPIOA->PIDR &= ~(1<<RPM_KBI_CHANNEL);              /* enable GPIO input */    
  FGPIOA->PDDR &= ~(1<<RPM_KBI_CHANNEL);              /* configure pin as input */ 
  PORT->PUE0 |= (1<<RPM_KBI_CHANNEL);                 /*enable pullup for the pin */   
  /*Reset KBI_SP register*/
  KBI1->SC |= 1<<KBI_SC_RSTKBSP_SHIFT;	
  /*Real KBI_SP register enable*/
  KBI1->SC |= 1<<KBI_SC_KBSPEN_SHIFT;  
  /* write to KBACK to clear any false interrupts */
  KBI1->SC = 1<<KBI_SC_KBACK_SHIFT;   
  /* enable interrupt if needed */
  KBI1->SC |=  KBI_SC_KBIE_MASK;
  NVIC_EnableIRQ(KBI1_IRQn);
}


void Swap(UINT32 *data1,UINT32 *data2)
{
  UINT32 temp = *data1;
  *data1 = *data2;
  *data2 = temp;
}
//鸡尾酒排序法(冒泡法排序改进版)
void Cocktail_Sort(UINT32 *data,unsigned char length )
{
  UINT8 left = 0;                           // 初始化边界
  UINT8 right = length - 1;
  while (left < right)
  {
    for (UINT8 i = left; i < right; i++)  // 前半轮,将最大元素放到后面
      if (data[i] > data[i + 1]) 
      {
        Swap(data+i, data+(i + 1));
      }
    right--;
    for (UINT8 i = right; i > left; i--)  // 后半轮,将最小元素放到前面
      if (data[i - 1] > data[i]) 
      {
        Swap(data+(i - 1),data+i);
      }
    left++;
  }
}

UINT32 Average(UINT32 *data,UCHAR length )
{
  UINT32 sum = 0;
  for(UCHAR i = 0;i < length; i++)
    sum += data[i];
   sum /= length;
  return sum;
}
UINT32 Hal_RPM_Get_Level(UCHAR level) 
{	 
  UINT32 data ;
  UINT32 temp_buffer[RPM_SAMPLE_NUM];
  UINT16 temp_num;

  data = 0;

  
  if(Capture_Flag == 1 )
  {    

    if(CurrentPulseNum > LastPulseNum)
      RotatePulseNum = CurrentPulseNum - LastPulseNum;
    else
      RotatePulseNum = CurrentPulseNum + (0x10000 - LastPulseNum);
    LastPulseNum = CurrentPulseNum;
    Capture_Flag = 0;
    temp_num = CntNum;
    if(temp_num == 0)
      data = Capture_Buf[RPM_BUF_SIZE - 1];
    else
      data = Capture_Buf[temp_num - 1];
//    if(Last_Value != 0)
//    {
//      if(data > ((Last_Value*3) >> 1))// || data < (Last_Value >> 1))
//      {
//        data = 0;
//        LastPulseNum = CurrentPulseNum;
//      }
//    }
//    Last_Value = data;
      
//    if(data < 18510)
//    {
//      if(temp_num >= RPM_SAMPLE_NUM)
//      {
//        memcpy(temp_buffer,Capture_Buf + (temp_num-RPM_SAMPLE_NUM),RPM_SAMPLE_NUM*4);//INT32 include 4 bytes
//      }
//      else
//      {
//        memcpy(temp_buffer,Capture_Buf,temp_num*4);
//        memcpy(temp_buffer + temp_num,Capture_Buf + (RPM_BUF_SIZE - RPM_SAMPLE_NUM + temp_num),(RPM_SAMPLE_NUM - temp_num)*4);
//      }
//      Cocktail_Sort(temp_buffer,RPM_SAMPLE_NUM);
//      data = Average(temp_buffer + 1,RPM_SAMPLE_NUM - 2);//throw away MAX & MIN value
//    }
  }
  return data;
}

void Hal_RPM_Set_KBIfuct(FNCT_VOID FnctX)
{
  FNCT_RPMint = FnctX;
}

void Hal_RPM_INT(void)
{
  KBI_ClrFlags(KBI1);
  if(KBI_GetSP(KBI1) == (1 << RPM_KBI_CHANNEL))
  {
    if(FNCT_RPMint != NULL) (*FNCT_RPMint)();
  }
  KBI_RstSP(KBI1);
}
void Hal_RPM_INT0(void)
{
  if(FTM_GetOverFlowFlag(RPM_FTM_TYPE))
  {
    Capture_Over_Flag++;
    FTM_ClrOverFlowFlag(RPM_FTM_TYPE);
  }
  if(FTM_GetChannelFlag(RPM_FTM_TYPE, RPM_FTM_CHANNEL))
  {
    FTM_ClrChannelFlag(RPM_FTM_TYPE, RPM_FTM_CHANNEL);
    if(Capture_Over_Flag != 0)
    {
      Capture_Buf[CntNum++] = (RPM_FTM_TYPE->CONTROLS[RPM_FTM_CHANNEL].CnV + 0x10000*Capture_Over_Flag) - Last_Capture_Value ;
      Capture_Over_Flag = 0;
    }
    else
      Capture_Buf[CntNum++] = RPM_FTM_TYPE->CONTROLS[RPM_FTM_CHANNEL].CnV - Last_Capture_Value;
    if(Capture_Buf[CntNum - 1] > 700 && Capture_Buf[CntNum - 1] < 630000) // pulse > 1Hz or pulse <  65535 Hz 
    {
      Last_Capture_Value = RPM_FTM_TYPE->CONTROLS[RPM_FTM_CHANNEL].CnV;
      Capture_Flag = 1;
      CurrentPulseNum ++;
    }
    else
    {
      CntNum -= 1;
    }
    if(CntNum >= RPM_BUF_SIZE)
    {
      CntNum = 0;      
    }
  }
}
UCHAR Hal_RPM_Rotate_Pulse_Num(void)
{
  UCHAR rtn;
  rtn = (RotatePulseNum == 0) ? 1:RotatePulseNum;
  return (rtn);
}
void Hal_RPM_Switch(void)
{
  if(Capture_Over_Flag > 10)
  {
    Capture_Over_Flag = 0;
    Capture_Flag = 0;
    CntNum = 0;
    RotatePulseNum = 0;
    LastPulseNum = 0;
    CurrentPulseNum = 0;
    Last_Capture_Value = 0;
    Last_Value = 0;
  } 
}

UCHAR Hal_RPM_Get_Stop_Status(void)
{
  UCHAR rtn;
  rtn = (CurrentPulseNum > 0) ? 0:1;
  return (rtn);
}
