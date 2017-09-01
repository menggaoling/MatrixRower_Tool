#include "Midware_Safekey_01.h"


#define SAFEKEY_REMOVE_MAX_TIME     100     //ms


__no_init UCHAR by_SafeKeyIsRemove;
__no_init UCHAR by_RemoveCounter;
void Midware_Safekey_Initial_HW(void)
{
  Hal_Safekey_Initial();
}

void Midware_Safekey_Initial_Data(void)
{
    
    by_SafeKeyIsRemove = 0;
    by_RemoveCounter = 0;
}

UCHAR Midware_Safekey_IsRemove(void)
{
    return by_SafeKeyIsRemove;
}
UCHAR by_Test ;
    UCHAR by_Test2;
void Midware_Safekey_1ms_Int(void)
{
#ifdef ANALOG_DPID_MCB
    if(Hal_Safekey_Get_Safekey())
#else
     by_Test = Hal_Safekey_Get_Safekey();
     by_Test2 = !Hal_Safekey_Get_Shut();
    if(Hal_Safekey_Get_Safekey() && !Hal_Safekey_Get_Shut())
#endif
    {
        by_SafeKeyIsRemove = 0;
        by_RemoveCounter = 0;
    }
    else
    {
        if(by_RemoveCounter > SAFEKEY_REMOVE_MAX_TIME)                          //100ms
        {
            by_SafeKeyIsRemove = 1;
        }
        else
        {
            by_RemoveCounter++;
        }
    }
}

void Midware_Safekey_ERP(void)
{
    Hal_Safekey_ERP();
}










