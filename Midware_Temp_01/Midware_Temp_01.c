#include "Midware_Temp_01.h"
#include "gpio.h"
#include "adc.h"

#define ADC_PORT                GPIOA
#define ADC_PIN                 GPIO_PTA6

#define DEFAULT_TRANS_INTERVAL          20          //unit 10ms 
#define DEFAULT_BUFF_SIZE               5           //�������

#define DEFAULT_PROTECT_VOLTAGE_RESTORE 148         //1.48v ��Ӧ55�� ��ѹֵ,���ֵ500(5.00V)- ��С0V(0.00V)
#define DEFAULT_PROTECT_VOLTAGE_MIN     28          //0.28V 120�ȵ�ѹֵ
#define DEFAULT_PROTECT_VOLTAGE_MAX     22          //0.22V 130�ȵ�ѹֵ

#define DEFAULT_ERROR_VOLTAGE_MIN       478         //4.78V -40������, �쳣����С�ȶ�
#define DEFAULT_ERROR_VOLTAGE_MAX       15          //0.15V 150������, �쳣������ȶ�

#define DEFAULT_PROTECT_TIME_MIN        1800        //3���� ���³�������ʱ�� unit 100ms 
#define DEFAULT_PROTECT_TIME_MAX        50          //5�� ���³�������ʱ�� unit 100ms

#define DEFAULT_ADC_BASE                1221        //(5 * 1000000) / 0x0FFF


__no_init static UCHAR  by_RoundIndex;
__no_init static UINT16 w_Temp;
__no_init static UINT16 aw_TempADC[DEFAULT_BUFF_SIZE];
__no_init static TEMP_ERROR TempError;
__no_init static struct
{
    unsigned Flag100ms                   : 1;    //100ms��־
} Temp;


UINT16 Midware_Temp_Cal_ADC(UINT16 *address);

void Midware_Temp_Initial_HW(void)
{
        GPIO_Init(ADC_PORT,  1 << (ADC_PIN % 32), GPIO_PinInput); 
}

void Midware_Temp_Initial_Data(void)
{
    memset(&TempError, 0, sizeof(TempError));
    memset(aw_TempADC, 0, sizeof(aw_TempADC));
    memset(&Temp,      0, sizeof(Temp));
    
    w_Temp = 0;
    by_RoundIndex = 0;
}

void Midware_Temp_Process(void)
{
    if(Temp.Flag100ms)
    {
        Temp.Flag100ms = 0;
            ADC_SingleConversion(ADC);
             aw_TempADC[by_RoundIndex] = ADC_PollRead(ADC, ADC_CHANNEL_AD2);
        if(++by_RoundIndex >= DEFAULT_BUFF_SIZE) by_RoundIndex = 0;
        
//        float fAdc = DEFAULT_ADC_BASE;
//        fAdc *= Midware_Temp_Cal_ADC(aw_TempADC);
//        fAdc /= 10000;
//        w_Temp = (UINT16)fAdc;
        UINT32 AdcValue=DEFAULT_ADC_BASE;
        AdcValue *=Midware_Temp_Cal_ADC(aw_TempADC);
        AdcValue /=10000;
        w_Temp = (UINT16)AdcValue;
        
        
        static UINT16 w_TempMcbTimerMin = 0;
        static UINT16 w_TempMcbTimerMax = 0;
        static UINT16 w_TempMcbErrorTimer = 0;
        static UINT16 w_TempMcbTimerRestore = 0;
        
        if(w_Temp <= DEFAULT_ERROR_VOLTAGE_MAX || w_Temp >= DEFAULT_ERROR_VOLTAGE_MIN)  //�ȶ��쳣����
        {
            if(!TempError.TempMcbUnnormal && w_TempMcbErrorTimer++ >= DEFAULT_PROTECT_TIME_MAX) //����5���¶��쳣����
            {
                w_TempMcbErrorTimer = 0;
                TempError.TempMcbUnnormal = 1;                                  //�쳣��־��1
            }
        }
        else 
        {
            w_TempMcbErrorTimer = 0;
            TempError.TempMcbUnnormal = 0;                                      //�쳣��־����
            
            if(w_Temp <= DEFAULT_PROTECT_VOLTAGE_MAX)
            {
                if(!TempError.TempMcb && w_TempMcbTimerMax++ >= DEFAULT_PROTECT_TIME_MAX)
                {
                    w_TempMcbTimerMax = 0;
                    TempError.TempMcb = 1;                                      //�����־��1
                }
                
                w_TempMcbTimerRestore = 0;
            }
            else if(w_Temp <= DEFAULT_PROTECT_VOLTAGE_MIN)
            {
                if(!TempError.TempMcb && w_TempMcbTimerMin++ >= DEFAULT_PROTECT_TIME_MIN)
                {
                    w_TempMcbTimerMin = 0;
                    TempError.TempMcb = 1;                                      //�����־��1
                }
                
                w_TempMcbTimerMax = 0;
                w_TempMcbTimerRestore = 0;
            }
            else
            {
                if(TempError.TempMcb && w_Temp >= DEFAULT_PROTECT_VOLTAGE_RESTORE)  //����ȶ������Ѵ���,�����ȶ��ѻָ���ָ���¶�ʱ,���㱣��
                {
                    if(w_TempMcbTimerRestore++ >= DEFAULT_PROTECT_TIME_MIN)
                    {
                        TempError.TempMcb = 0;
                    }
                }
                else
                {
                    w_TempMcbTimerRestore = 0;
                }
                
                w_TempMcbTimerMin = 0;
                w_TempMcbTimerMax = 0;
                w_TempMcbErrorTimer = 0;
            }
        }
    }
}

UINT16 Midware_Temp_Cal_ADC(UINT16 *address)
{
    UINT32 sum = 0;
    UINT16 max = address[0], min = address[0];
    
    for(UCHAR i = 0; i < DEFAULT_BUFF_SIZE; i++)
    {
        sum += address[i];
        if(max < address[i]) max = address[i];
        if(min > address[i]) min = address[i];
    }
    
    sum -= (max + min);
    return (sum / (DEFAULT_BUFF_SIZE - 2));
}

void Midware_Temp_1ms_Int(void)
{
    static UCHAR i = 0;
    if(++i >= 100)
    {
    i = 0;
    Temp.Flag100ms = 1;
    }
}

UINT16 Midware_Temp_Get_Type(TEMP_TYPE Type)
{
    return w_Temp;
}

void Midware_Temp_Get_Error(TEMP_ERROR *stError)
{
    memcpy(stError, &TempError, sizeof(TempError));
}





