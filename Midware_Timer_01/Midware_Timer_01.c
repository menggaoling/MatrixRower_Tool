#include "Midware_Timer_01.h"


#define TIMER_MAX_NUM       32


__no_init static UINT32 flag;
__no_init static UINT16 timer[TIMER_MAX_NUM];


void Midware_Timer_Initial_Data(void)
{
  flag = 0;
  memset(timer, 0, sizeof(timer));
}

void Midware_Timer_1ms_Int(void)
{
  static UCHAR i = 0;
  if(++i >= 10)
  {
    i = 0;
    flag = 0xFFFFFFFF;
  }
}

void Midware_Timer_Clear(UCHAR index)
{
  if(index < TIMER_MAX_NUM) timer[index] = 0;
}

UCHAR Midware_Timer_Counter(UCHAR index, UINT16 w_Dat)
{
  UINT32 mask = 0;
  UCHAR  out = 0;
  UCHAR  channel = 0;
  
  channel = index & 0x7F;
  
  if(channel < TIMER_MAX_NUM)                                              //小于最大TIMER_MAX_NUM个定时器
  {
    mask = 0x01 << channel;
    
    if(flag & mask)                                                     //每过100ms执行一次当前定时器.
    {
      flag &= ~mask;                                                  //相应标志位被清零
      if(timer[channel] < 0xFFFF) timer[channel]++;           //每执行一次相应的定时器就加1
      if(timer[channel] == w_Dat)                                   //等于目标值返回1,保证每计数一次定时器，只会触发一次
      {
        out = 1;                                                     //只有相等才会输出真
        if(index & T_LOOP) timer[channel] = 0;                 //循环计数
      }
    }
  }
  
  return(out);
}




