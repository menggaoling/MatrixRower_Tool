#include "Midware_MachineType_01.h"

__no_init static UCHAR LCBtype;
__no_init static UINT16 typeCheck;

#define TYPE_FINISH_CHECK     0X55AA

void Midware_MachineType_Init_HW(void)  
{   
    Hal_MachineType_Init();
}

void Midware_MachineType_Init_Data(void)  
{   
    LCBtype = 0;
    typeCheck = 0;
}

void Midware_MachineType_Process(void) 
{
	UCHAR ADCcnt = 0;
    UINT16 avgADC = 0;
    UINT16 typeADC[10] = {0};
    
    if(typeCheck != TYPE_FINISH_CHECK)
    {
        LCBtype = 0;
        while(1)
        {
            typeADC[ADCcnt] = Hal_MachineType_Collect_ADC();
            if(++ADCcnt >= 10)  
            {
                UINT32 dwSum = 0; 
                UINT16 wMax = typeADC[0], wMin = typeADC[0];
                
                for(int i = 0; i < 10; i++) 
                {
                    dwSum += typeADC[i];
                    if(wMax < typeADC[i]) wMax = typeADC[i];
                    if(wMin > typeADC[i]) wMin = typeADC[i];
                }
                
                dwSum -= ( wMax + wMin );
                dwSum /= 8;
                avgADC = dwSum;
                Hal_MachineType_End_Collect();
                break;
            }
        }
        
        if(avgADC >= 3140 && avgADC <= 4096)  
        {
            LCBtype = 0xB4;
        }
        else if(avgADC >= 2048 && avgADC <= 2868) 
        {
            LCBtype = 0xB5;
        }
        else if(avgADC >= 200 && avgADC <= 800) 
        {
            LCBtype = 0xB3;
        }
        else if(avgADC <= 100) 
        {
            LCBtype = 0xB6;
        } 
        typeCheck = TYPE_FINISH_CHECK;
    }
}

UINT16 Midware_MachineType_Get_Type(void) 
{
    return LCBtype;
}


