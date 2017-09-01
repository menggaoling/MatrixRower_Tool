#include "APID.h"
#include "Midware_Trace_01.h"
#include "Midware_RPM_MCB_01.h"
#include "Midware_Nvflash_01.h"
#include "math.h"

//��С RPM 170 * 80 = 13600 / 60 = 226.6/s ��С����ʱ��Ϊ 1000ms / 226.6 = 4.4ms
//ÿһ��countʱ��Ϊ  1,000,000(1s) / (31250/2) = 64us
#define TIME_BASE_RPM           937500             //60000000(ʱ��60��) / (64)            

#define DEFAULT_OUTPUT_PWM_MAX  4000                //4300
#define DEFAULT_RPM_MAX         2130 

#define DEFAULT_KI              0.005                //Ĭ��Kiֵ


#define DEFAULT_STEP_START      1
#define DEFAULT_STEP_MID        1
#define DEFAULT_STEP_ADD        2                   //1000/20 = 50, 50*6 = 300, 4300/300 = 14.3��, ��0�����ٵ�4300�����Ҫʱ��
#define DEFAULT_STEP_DEC        4                   //Ĭ�����Step  1000/20 = 50*7 = 350RPM/s�ļ��ٶ�, 4300/350 = 12.2��.��4300���ٵ�0�����Ҫʱ��

#define DEFAULT_TIME_MS         20                  //���ε���ʱ�� ms
#define DEFAULT_MILE_03         60                  //<=>0.3mil/h��������ϵ��ٶ��ж�ֵ
#define DEFAULT_MILE_05         91                  //<=>0.5mil/h��������ϵ��ٶ��ж�ֵ
#define DEFAULT_MILE_1          186                 //1mile

#define DEFAULT_AUTO_CAL_TIMER  6000                //ms ÿһ��У����,�ȶ�����ʱ��
#define DEFAULT_CAL_NUM         11                  //10��У��


typedef struct
{
    UINT16 Rpm;         //Ŀ��RPMֵ
    UINT16 Pwm;         //RPM��Ӧ��PWMֵ
    UINT16 Offset;      //У��ʱ��ƫ����
} CAL_TABLE;


//������ʾʵ��ֵ����Ŀ��ֵ
static CAL_TABLE CalTableDefault[DEFAULT_CAL_NUM] = 
{
    {0,     0,      10},   
    {60,    175,    5}, 
    {100,   210,    5}, 
    {200,   303,    10}, 
    {400,   504,    10}, 
    {700,   829,    15}, 
    {1000,  1168,   15},
    {1300,  1513,   20},
    {1600,  1865,   20},
    {1900,  2217,   25},
    {DEFAULT_RPM_MAX,  2481, 25}, //����1   mile
};

typedef enum
{
    CALIBRATE_NONE   = 0,
    CALIBRATE_START,
    CALIBRATE_OK,
    CALIBRATE_FAIL,
}CALIBRATE_STATE;

__no_init static UCHAR  bFlagCalc;                  //�ɼ����־
__no_init static CALIBRATE_STATE CalibrateState;    //�Զ�У���־
__no_init static UCHAR  by_AutoCalibrateStep;       //�Զ�У�鲽��
__no_init static UINT32 w_RPM_Buff[BUFFER_SIZE];   //RPM����Buff
__no_init static UINT16 w_ActualRPM;                //ʵ��RPM
__no_init static UINT16 w_CalCounter;
__no_init static UINT16 w_TargetCurrent;            //��ǰĿ��ֵ
__no_init static UINT16 w_Output;                   //���ֵ
__no_init static UCHAR TargetStable;                 //Ŀ��ֵ�Ƿ��ڸı�,
__no_init static struct
{
    float Offset;                                   //����ƫ��ֵ
    float Ki;                                       //�������������
    float Output;                                   //�������ֵ
} PID;


static UINT16 APID_Calibrate_Tune(UINT16 Target, UINT16 Actual);
static UINT16 APID_Calc_Target(UINT16 Target, UINT16 Actual);
static UINT16 APID_Cal_RPM(void);
static void   APID_Initial_RPM(void);
static UINT32 APID_Filter_RPM_Data(UINT32 *pData32);
static UINT16 APID_Filter_RPM(UINT16 wNowRpm);


void APID_Initial_Data(void)
{
    APID_Initial_RPM();
    memset(&PID, 0, sizeof(PID));
    PID.Ki = DEFAULT_KI;
    
    TargetStable = 0;
    w_TargetCurrent = 0;
    w_Output = 0;
    bFlagCalc = FALSE;
    by_AutoCalibrateStep = 10;
    w_CalCounter = 0;
    CalibrateState=CALIBRATE_NONE;
    if(Midware_Nvflash_Get_OneCalibratedData( AUTO_CAL_FLAG) == C_ATUTO_CAL_FLAG)
    {
      for(UINT8 i=0;i<DEFAULT_CAL_NUM;i++)
      {
        CalTableDefault[i].Rpm = Midware_Nvflash_Get_OneCalibratedData( AUTO_CAL_RPM_0+2*i); 
        CalTableDefault[i].Pwm = Midware_Nvflash_Get_OneCalibratedData( AUTO_CAL_PWM_0+2*i);
      }     
      CalibrateState = CALIBRATE_OK;
    }
    else
    {
        APID_SetAutoCalibrate();
    }
}

void APID_SetAutoCalibrate(void)
{
    CalibrateState = CALIBRATE_START;
}

UCHAR APID_Calibrating(void)
{                                      
    return CalibrateState == CALIBRATE_START;
}

//�̶���ʱ�����һ��
UCHAR APID_Process(UINT16 wTargetRPM, UINT16 *pw_Out)
{
    if(bFlagCalc)
    {
        bFlagCalc = FALSE;
        w_ActualRPM = APID_Cal_RPM();                                           //���㵱ǰʵ��RPM
        
        if(CalibrateState == CALIBRATE_START)
        {
            wTargetRPM = CalTableDefault[by_AutoCalibrateStep].Rpm;
            UINT16 w_Target = APID_Calc_Target(wTargetRPM, w_ActualRPM);        //Ŀ��ֵ����,�Ӽ��ٶ�ʱ,Ŀ��ֵһ��һ��ƽ�
            
            //Trace("T:%d, A:%d, O:%d\n", w_Target, w_ActualRPM, w_Output);
            if(fabs((float)w_ActualRPM - (float)wTargetRPM) <= CalTableDefault[by_AutoCalibrateStep].Offset)
            {
                if(w_CalCounter >= DEFAULT_AUTO_CAL_TIMER)
                {
                    w_CalCounter = 0;
                    CalTableDefault[by_AutoCalibrateStep].Pwm = w_Output;       //save one
                    Trace("Now:%d, Output:%d,\n", by_AutoCalibrateStep, w_Output);
                    
                    if(by_AutoCalibrateStep >1)
                    {
                        by_AutoCalibrateStep--;
                    }
                    else
                    {
                        for(UINT8 i=0;i<DEFAULT_CAL_NUM;i++)
                        {
                          Midware_Nvflash_Prepare_OneCalibratedData( AUTO_CAL_RPM_0+2*i, CalTableDefault[i].Rpm); 
                          Midware_Nvflash_Prepare_OneCalibratedData( AUTO_CAL_PWM_0+2*i, CalTableDefault[i].Pwm);
                        }                       
                        Midware_Nvflash_Prepare_OneCalibratedData( AUTO_CAL_FLAG, C_ATUTO_CAL_FLAG);
                        Midware_Nvflash_Save_CalibratedDatas();
                        //�Զ�У��ɹ�,����У��ֵ
                        CalibrateState = CALIBRATE_OK;
                        by_AutoCalibrateStep = 10;
                        for(UCHAR i=0; i<DEFAULT_CAL_NUM; i++)
                        {
                            TraceW("Data %d:RPM %d, PWM %d,\n", i, CalTableDefault[i].Rpm, CalTableDefault[i].Pwm);
                        }
                        
                        TraceW("Calibrate Save OK\n");
                    }
                }
            }
            else
            {
                w_CalCounter = 0;
            }
            
            w_Output = APID_Calibrate_Tune(w_Target, w_ActualRPM);
        }
        else
        {
            float fTemp = 0;
            UINT16 w_Target = APID_Calc_Target(wTargetRPM, w_ActualRPM);        //Ŀ��ֵ����,�Ӽ��ٶ�ʱ,Ŀ��ֵһ��һ��ƽ�
            
            for(UCHAR i=1; i<DEFAULT_CAL_NUM; i++)
            {
                if(w_Target >= CalTableDefault[i-1].Rpm && w_Target <= CalTableDefault[i].Rpm)
                {
                    fTemp  = CalTableDefault[i].Pwm - CalTableDefault[i-1].Pwm;
                    fTemp /= (float)(CalTableDefault[i].Rpm - CalTableDefault[i-1].Rpm);
                    fTemp *= w_Target - CalTableDefault[i-1].Rpm;
                    fTemp += CalTableDefault[i-1].Pwm;
                    break;
                }
            }
            
            w_Output = (UINT16)fTemp;
            
            //Trace("A:%d, B:%d,\n", w_ActualRPM, w_Target);
        }
        
        if(w_Output > DEFAULT_OUTPUT_PWM_MAX) w_Output = DEFAULT_OUTPUT_PWM_MAX;
        *pw_Out = w_Output;
        
        return 1;
    }
    else
    {
        return 0;
    }
}

UINT16 APID_Calibrate_Tune(UINT16 Target, UINT16 Actual)
{
    PID.Offset = (float)Target - (float)Actual;
    
    if(PID.Offset < -3 || PID.Offset > 3)
    {
        //���㹫ʽһ: ������
        float Inc = PID.Ki * PID.Offset;
        PID.Output += Inc;
    }
    
    //�������С�����ֵ
    if(PID.Output < 0) PID.Output = 0;
    if(PID.Output > DEFAULT_OUTPUT_PWM_MAX) PID.Output = DEFAULT_OUTPUT_PWM_MAX;

    return (UINT16)PID.Output;
}

//�Ӽ��ٶ�ʱ,Ŀ��ֵһ��һ��ƽ�
UINT16 APID_Calc_Target(UINT16 Target, UINT16 Actual)
{
    UINT16 wTarget = Target;
    UINT16 wOut = wTarget;
    UCHAR  by_Step = 0;
    
    if(wTarget > DEFAULT_RPM_MAX) wTarget = DEFAULT_RPM_MAX;
    
    if(wTarget > w_TargetCurrent)                                               //����Ŀ���ٶ�
    {
        if(w_TargetCurrent < DEFAULT_MILE_05)
        {
            by_Step = DEFAULT_STEP_START;
        }
        else if(w_TargetCurrent < DEFAULT_MILE_1)
        {
            by_Step = DEFAULT_STEP_MID;
        }
        else
        {
            by_Step = DEFAULT_STEP_ADD;
        }
        
        if(wTarget > (w_TargetCurrent + by_Step))
        {
            wOut = w_TargetCurrent + by_Step;
        }
        else
        {
            wOut = wTarget;
        }
        
        TargetStable = 0;
    }
    else if(wTarget < w_TargetCurrent)                                          //��СĿ���ٶ�
    {
        if(w_TargetCurrent < DEFAULT_MILE_03)
        {
            by_Step = DEFAULT_STEP_START;
        }
        else if(w_TargetCurrent < DEFAULT_MILE_1)
        {
            by_Step = DEFAULT_STEP_MID;
        }
        else
        {
            by_Step = DEFAULT_STEP_DEC;
        }
        
        if(w_TargetCurrent - wTarget > by_Step)
        {
            wOut = w_TargetCurrent - by_Step;
        }
        else
        {
            wOut = wTarget;
        }
        
        TargetStable = 0;
    }
    else
    {
        TargetStable = 1;
        wOut = w_TargetCurrent;
    }
    
    w_TargetCurrent = wOut;
    
    return w_TargetCurrent;
}

UCHAR APID_Get_TargetStable(void)
{
    return TargetStable;
}

UINT16 APID_Get_Current_Target(void)
{
    return w_TargetCurrent;
}

void APID_1ms_Int(void)
{
    static UINT16 w_TickMs_Counter = 0;
    if(++w_TickMs_Counter >= DEFAULT_TIME_MS)
    {
        w_TickMs_Counter = 0;
        bFlagCalc = TRUE;
    }
    
    if(w_CalCounter < DEFAULT_AUTO_CAL_TIMER) w_CalCounter++;
}


/***********************************RPM****************************************/
void APID_Initial_RPM(void)
{
    w_ActualRPM = 0;
    memset(w_RPM_Buff, 0, sizeof(w_RPM_Buff));
}

UINT16 APID_Get_RPM(void)
{
    return w_ActualRPM;
}

UINT16 APID_Cal_RPM(void)
{
    UINT16 wOut = 0;
    
    if(Midware_RPM_Get_PulseData(w_RPM_Buff))
    {
        UINT32 wRPM = APID_Filter_RPM_Data(w_RPM_Buff);
        
        if(wRPM > 0)
        {
            wOut = (UINT16)(TIME_BASE_RPM / wRPM);
        }
        
        wOut = APID_Filter_RPM(wOut);
    }
    else
    {
        APID_Initial_RPM();
    }
    
    return wOut;
}

UINT32 APID_Filter_RPM_Data(UINT32 *pData32)
{
    //����OSP�ź� MAX_OSP_BUFF
    UINT32 wBuff[3] = {0};
    UCHAR  by_Index = 0;
    UCHAR  by_MaxRef = BUFFER_SIZE;
    
    if(w_TargetCurrent >= 1500)
    {
        by_MaxRef = BUFFER_SIZE;
    }
    else if(w_TargetCurrent >= 1000)
    {
        by_MaxRef = 5;
    }
    else
    {
        by_MaxRef = 3;
    }
    
    
    if(by_MaxRef >= 5)
    {
        //�ҳ�3�����ֵ,���ֵ����RPM��С
        for(UCHAR i=0; i<3; i++)
        {
            wBuff[i] = pData32[0];
            by_Index = 0;
            
            for(UCHAR k=1; k<by_MaxRef; k++)
            {
                if(pData32[k] > wBuff[i])
                {
                    wBuff[i] = pData32[k];
                    by_Index = k;
                }
            }
            
            pData32[by_Index] = 0;
        }
        
        //3��ֵ�����ƽ��
        UINT32 wCount = 0;
        for(UCHAR i=0; i<3; i++)
        {
            wCount += wBuff[i];
        }
        
        return (wCount / 3);
    }
    else
    {
        return pData32[0];
    }
}

UINT16 APID_Filter_RPM(UINT16 wNowRpm)
{
    static UINT16 w_LastActualRPM = 0;            //��һ��ʵ��RPM
    
    UINT16 wOut = wNowRpm;
    
    if(w_TargetCurrent >= DEFAULT_MILE_03)
    {
        if(wNowRpm > w_LastActualRPM)                                           //���Ӳ�������
        {
            if(wNowRpm - w_LastActualRPM >= 150) 
            {
                wOut = w_LastActualRPM+1;
            }
            else if(wNowRpm - w_LastActualRPM >= 80)
            {
                wOut = w_LastActualRPM + 80;                                    //ʵ��RPM���Ӳ���
            }
        }
        else                                                                    //������������
        {
            if(w_LastActualRPM - wNowRpm >= 80)                                 //ʵ��RPM��������
            {
                wOut = w_LastActualRPM - 80;
            }
        }
        
        w_LastActualRPM = wOut;                                                 //������һ�ε�RPM
    }
    else
    {
        w_LastActualRPM = 0;
    }
    
    return wOut;
}




