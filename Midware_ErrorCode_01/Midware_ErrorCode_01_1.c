#include "Midware_ErrorCode_01.h"

#define ERROR_LAST_SEND_NONE	0xFF

typedef struct
{
  UINT16      code;         //错误代码
  UCHAR       status;       //错误代码状态
  UCHAR       skipped;      //是否已Skip掉
  LED_TYPE    led;          //Led指示灯
}ERROR_INFOR;

typedef struct
{
  LED_TYPE    led;          //Led指示灯
  UINT16      code;         //错误代码
}ERROR_BRIEF;


static const ERROR_BRIEF errorBrief[NUM_OF_ERROR] = 
{
#ifdef __LCB_SYS__
  LED_NO_RPM,             0x02C8,    //NO_RPM
  LED_IND_ERR,            0x01AF,    //Induction open
  LED_INC_NO_CNT,         0x0140,    //incline no count
  LED_IND_ERR,            0x02C9,    //induction temperature over
  LED_TEMP_ABNORMAL,      0x024C,    //induction temperature abnormal
  LED_COMM_FAIL,          0x0440,    //connect fail
  LED_COMM_FAIL,          0x0443,    //RX or Tx fail one 
  LED_ECB_ERR,            0x0244,    //ECB no count
  LED_ECB_ERR,            0x0245,    //ECB zero open or short 
  LED_FLASH_ERR,          0x02B9,    //data write error
  LED_FLASH_ERR,          0x02BA,    //data read error
  LED_INC_SHORT,          0x01B5,    //incine zero short
  LED_IND_ERR,            0x02A8,    //induction mos short
  LED_LCB_UNKNOWN,        0x02AB,    //type err
#else
  LED_MCB_TYPE_ERROR,     0x02AB,	//Mcb Type Error,C
  LED_SAFEKEY,	          0x02B2,	//Safekey,C
  LED_NO_RPM,	          0x00A1,	//No RPM,C
  LED_OVER_CURRENT,	  0x01A8,	//Over Current,C
  LED_MOS_FAULT,	  0x02A8,	//Mos Fault,C
  LED_SPEED_LOW,	  0x0041,	//Speed Low,B
  LED_INC_NO_CNT,	  0x0140,	//Incline No count,B
  LED_INC_SHORT,	  0x01B5,	//Inc Zero Short,B
  LED_MCB_TEMP_OVER,	  0x0242,	//Mcb Temp Over,B
  LED_MCB_TEMP_UNNORMAL,  0x024C,	//Mcb TempSensor Unnormal,B
  LED_COMM_FAIL,	  0x0440,	//Digtial Timeout,B
  LED_COMM_FAIL,	  0x0443,	//Digtial Rx Error,B
  LED_FLASH_ERR,	  0x02B9,	//Falsh Write,A
  LED_FLASH_ERR,	  0x02BA,	//Falsh Read,A
#endif
};

__no_init static NVFLASH_ERROR      NvFlashError;
__no_init static DIGITAL_ERROR      DigitalError;
__no_init static INCLINE_ERROR      IncError;
__no_init static ERROR_INFOR        errorCode[NUM_OF_ERROR];
__no_init static UCHAR              lastIndex;

#ifdef __LCB_SYS__
__no_init static INDUCTION_ERROR    IndError;
__no_init static ECB_ERROR          ECBerror;
#else
__no_init static MOTOR_ERROR        MotorError;
__no_init static TEMP_ERROR         TempError;
__no_init static UCHAR              by_SafekeyError;
#endif

void Midware_ErrorCode_Initial_Data(void)
{
  memset(&DigitalError, 0, sizeof(DIGITAL_ERROR));
  memset(&IncError, 0, sizeof(INCLINE_ERROR));
  memset(&NvFlashError, 0, sizeof(NVFLASH_ERROR));
  memset(errorCode, 0, sizeof(errorCode));
  
#ifdef __LCB_SYS__
  memset(&IndError, 0, sizeof(INDUCTION_ERROR));
  memset(&ECBerror, 0, sizeof(ECB_ERROR));
#else
  memset(&MotorError,   0, sizeof(MotorError));
  memset(&TempError,    0, sizeof(TempError));
  by_SafekeyError = 0;
#endif    
  
  for(UCHAR i = 0; i < NUM_OF_ERROR; i++)
  {
    errorCode[i].code = errorBrief[i].code;
    errorCode[i].led = errorBrief[i].led;
  }
  lastIndex = ERROR_LAST_SEND_NONE;
}

void Midware_ErrorCode_Process(void)
{   
  Midware_Nvflash_GetError(&NvFlashError);  
  Midware_Digital_Get_Error(&DigitalError);                                     
  Midware_Incline_Get_Error(&IncError); 
#ifdef __LCB_SYS__    
  Midware_ECB_Get_Error(&ECBerror);                                     
  Midware_Induction_Get_Error(&IndError);
  errorCode[NO_RPM].status              = Midware_RPM_Get_RPM() ? 0 : 1;
  errorCode[IND_OPEN].status            = IndError.openCircuit ? 1 : 0; 
  errorCode[INC_NO_CNT_LCB].status        = IncError.noCount ? 1 : 0;
  errorCode[RX_TX_ERROR].status         = DigitalError.Digital_Rx_Timeout ? 1 : 0;
  errorCode[COMM_TIMEOUT].status        = DigitalError.Digital_TimeOut ? 1 : 0; 
  errorCode[ECB_NO_COUNT].status        = ECBerror.noCount ? 1 : 0;
  errorCode[ECB_ZERO_ERR].status        = (ECBerror.zeroOpen | ECBerror.zeroShort) ? 1 : 0;
  errorCode[NVFLASH_W].status           = NvFlashError.write ? 1 : 0;
  errorCode[NVFLASH_R].status           = NvFlashError.read ? 1 : 0;
  errorCode[INC_ZERO_SHORT].status      = IncError.zeroShort ? 1 : 0;
  errorCode[IND_MOS_SHORT].status       = IndError.shortCircuit ? 1 : 0; 
  errorCode[TYPE_FAIL].status           = Midware_MachineType_Get_Type() ? 0 : 1;
#else    
  Midware_Motor_Get_Error(&MotorError);                                         //获取motor 状态
  Temp_Get_Error(&TempError);                                                 //获取Temp 状态
  by_SafekeyError = Midware_Safekey_IsRemove();                                 //获取Safekey 状态
  if(!errorCode[SAFEKEY].status)                                              //安全开关处理,只认移除安全开关,插上安全开关需要上控发初始化命令
  {
    errorCode[SAFEKEY].status        = by_SafekeyError ? 1 : 0;
  }
  errorCode[MCB_TYPE_ERROR].status   = MotorError.Motor_HP ? 1 : 0;
  errorCode[NO_INCODE].status        = MotorError.Motor_NoRpm ? 1 : 0;
  errorCode[OVER_CURRENT].status     = MotorError.Motor_OverCurrent ? 1 : 0;
  errorCode[MOS_FAULT].status        = MotorError.Motor_MOS ? 1 : 0;
  errorCode[SPEED_LOW].status        = MotorError.Motor_SlowSpeed ? 1 : 0;
  errorCode[INC_NO_COUNT].status     = IncError.noCount ? 1 : 0;
  errorCode[INC_ZEROSHORT].status    = IncError.zeroShort? 1 : 0;
  errorCode[MCB_TEMP_OVER].status    = TempError.TempMcb ? 1 : 0;
  errorCode[MCB_TEMP_UNNORMAL].status= TempError.TempMcbUnnormal ? 1 : 0;
  errorCode[DIGTIAL_TIMEOUT].status  = DigitalError.Digital_TimeOut ? 1 : 0;
  errorCode[DIGTIAL_RX_ERROR].status = DigitalError.Digital_CRC ? 1 : 0;
  errorCode[FLASH_W].status          = NvFlashError.write ? 1 : 0;
  errorCode[FLASH_R].status          = NvFlashError.read ? 1 : 0;
#endif   
  UCHAR by_Led = 0;
  for(UCHAR i = 0; i < NUM_OF_ERROR; i++)                                     //每次只找最优先极的处理
  {
    if(errorCode[i].status)
    {
      if(errorCode[i].led != LED_NORMAL)
      {
        by_Led = 1;
        Midware_Led_Mode(errorCode[i].led);
        break;
      }
    }
  }     
  if(!by_Led) Midware_Led_Mode(LED_NORMAL);
}

UCHAR Midware_ErrorCode_Have_Error(void)
{
  UCHAR by_Out = 0;
  
  for(UCHAR i = 0; i < NUM_OF_ERROR; i++)   
  {
    if(errorCode[i].status && !errorCode[i].skipped)
    {
      by_Out = 1;
    }
  }
  return by_Out;
}

UCHAR Midware_ErrorCode_GetStatus(ERROR_BEHAVIOR ErrorType)
{
  UCHAR by_Out = 0;
  
  if(ErrorType < NUM_OF_ERROR)
  {
    by_Out = errorCode[ErrorType].status;
  }
  
  return by_Out;
}

//优先级,按从0至0xff,上面的最高优先级
UINT16 Midware_ErrorCode_Get_ErrorCode()
{
  UINT16 w_Out = 0;
  
  if(lastIndex != ERROR_LAST_SEND_NONE && lastIndex < NUM_OF_ERROR) 
  {
    w_Out = errorCode[lastIndex].code;
  }
  else
  {
    lastIndex = ERROR_LAST_SEND_NONE;
    
    for(UCHAR i = 0; i < NUM_OF_ERROR; i++)  
    {
      if(errorCode[i].status && !errorCode[i].skipped)
      {
        w_Out = errorCode[i].code;
        lastIndex = i;
        break;
      }
    }
  }
  return w_Out;
}

//屏蔽最近发送的错误代码
void Midware_ErrorCode_Skip_Last(void)
{
  if((lastIndex != ERROR_LAST_SEND_NONE) && (lastIndex <  NUM_OF_ERROR))                                      
  {
    errorCode[lastIndex].skipped = 1;
    lastIndex = ERROR_LAST_SEND_NONE;
  }
}

void Midware_ErrorCode_ResetSafekey(void)
{
#ifdef __MCB_SYS__ 
  errorCode[SAFEKEY].status = 0;
  errorCode[SAFEKEY].skipped = 0;
#endif
}






