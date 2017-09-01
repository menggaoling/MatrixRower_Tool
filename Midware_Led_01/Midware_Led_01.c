#include "Midware_Led_01.h"

#define LED_INTERVAL_1000MS   1000
#define LED_INTERVAL_500MS    500

__no_init UINT16 timerCnt;
__no_init UCHAR flashCurrentCnt, flashTargetCnt;
__no_init UCHAR ERPmode;
__no_init UINT16 LEDflashTime, LEDintervalTime;

void Midware_Led_Set_FlashTime(UINT16 flashTime, UCHAR flashCnt)
{
  UCHAR countTemp = flashCnt * 2 - 1;
  if( LEDflashTime != flashTime || flashTargetCnt != countTemp )
  {
    LEDflashTime = flashTime;       
    flashTargetCnt = countTemp;
    flashCurrentCnt = 0;
    LEDintervalTime = LED_INTERVAL_1000MS;
    if(LEDflashTime == 500) LEDintervalTime = LED_INTERVAL_500MS;
    Hal_Led_Set_High();
  }
}

void Midware_Led_Init_HW(void)
{
  Hal_Led_Initial();
}

void Midware_Led_Init_Data(void)
{
  timerCnt = 0;
  ERPmode = 0;
  Midware_Led_Mode(LED_FLASH_ERR);
}

void Midware_Led_Mode(LED_TYPE Mode)
{
  UINT16 flashTime = 300;
  if(Mode == LED_NORMAL)
    flashTime = 500;
  Midware_Led_Set_FlashTime(flashTime, Mode);
}

void Midware_Led_1ms_Int(void)
{
  if(ERPmode) return;
  if(flashCurrentCnt < flashTargetCnt)
  {
    if(++timerCnt >= LEDflashTime)
    {
      timerCnt = 0;
      ++flashCurrentCnt;
      Hal_Led_Toggle();
    }
  }
  else if(flashCurrentCnt >= flashTargetCnt)
  {
    if(++timerCnt >= LEDintervalTime)                                      
    {
      timerCnt = 0;
      flashCurrentCnt = 0;
      Hal_Led_Set_High();
    }
    else
    {
      Hal_Led_Set_Low();
    }
  }
}

void Midware_Led_ERP(void)
{
  ERPmode = 1;
  Hal_Led_ERP();
}
