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
  
  if(channel < TIMER_MAX_NUM)                                              //С�����TIMER_MAX_NUM����ʱ��
  {
    mask = 0x01 << channel;
    
    if(flag & mask)                                                     //ÿ��100msִ��һ�ε�ǰ��ʱ��.
    {
      flag &= ~mask;                                                  //��Ӧ��־λ������
      if(timer[channel] < 0xFFFF) timer[channel]++;           //ÿִ��һ����Ӧ�Ķ�ʱ���ͼ�1
      if(timer[channel] == w_Dat)                                   //����Ŀ��ֵ����1,��֤ÿ����һ�ζ�ʱ����ֻ�ᴥ��һ��
      {
        out = 1;                                                     //ֻ����ȲŻ������
        if(index & T_LOOP) timer[channel] = 0;                 //ѭ������
      }
    }
  }
  
  return(out);
}




