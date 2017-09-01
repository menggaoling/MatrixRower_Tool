#include "Midware_RPM_MCB_01.h"


__no_init static UINT16 RPMPulseCounter;
__no_init static UINT32 RPMDataBuff[BUFFER_SIZE];
__no_init static struct
{
    unsigned DetectPulse    : 1;	            //1 == Pulse has been Detected, 0 = Detected No Pulse
    unsigned Lock           : 1;
} RPM;
void Midware_RPM_Int(void);
void Midware_RPM_Initial_HW(void)
{
    Hal_RPM_Initial();
}

void Midware_RPM_Initial_Data(void)
{
    RPMPulseCounter = 0;
    memset(&RPM, 0, sizeof(RPM));
    memset(RPMDataBuff, 0, sizeof(RPMDataBuff));
    Hal_RPM_Set_Intfuct(Midware_RPM_Int);
}

UCHAR Midware_RPM_Get_PulseData(UINT32 *pData)
{
    if(pData == NULL) return 0;
    
    UCHAR by_Out = FALSE;
    
    if(RPM.DetectPulse)
    {
        RPM.Lock = 1;
        memcpy(pData, RPMDataBuff, sizeof(RPMDataBuff));
        RPM.Lock = 0;
        
        if(RPMPulseCounter >= RPM_MIN_INTERVAL)
        {
            Midware_RPM_Initial_Data();
            by_Out = FALSE;
        }
        else
        {
            by_Out = TRUE;
        }
    }
    
    return by_Out;
}

void Midware_RPM_Int(void)
{
	RPMPulseCounter = 0;
	UINT32 RPMData = Hal_RPM_Get_Capture();
    
    
    if(RPM.DetectPulse)                                                         //每次运动开始时获得的第一个数据丢弃
    {
        if(!RPM.Lock)
        {
            for(UCHAR i=(BUFFER_SIZE-1); i>0; i--)
            {
                RPMDataBuff[i] = RPMDataBuff[i-1];
            }
            RPMDataBuff[0] = RPMData;
            
            if(RPMDataBuff[0] < MIN_PULSE) RPMDataBuff[0] = RPMDataBuff[1];
            if(RPMDataBuff[0] > MAX_PULSE) RPMDataBuff[0] = MAX_PULSE;
        }
    }
    
    RPM.DetectPulse = 1;
}

void Midware_RPM_1ms_Int(void)
{
    if(RPM.DetectPulse)
    {
        if(RPMPulseCounter < RPM_MIN_INTERVAL) RPMPulseCounter++;
    }
    else
    {
        RPMPulseCounter = 0;
    }
}
UCHAR Midware_RPM_IsRunning(void)
{
    return RPM.DetectPulse;
}


void Midware_RPM_ERP(void)
{
    Hal_RPM_ERP();
}


